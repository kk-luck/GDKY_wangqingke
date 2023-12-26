/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     owi.c
  * @author   WangQk
  * @version  V1.00
  * @date     2023-09-15
  * @brief    OWI network maintenance.
  ******************************************************************************
  * @history
  */
	
	
/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "owi.h"
#include "combus.h"
#include "queue.h"

#define OWI_POWER_ON()    						FL_GPIO_ResetOutputPin(GPIOC, FL_GPIO_PIN_12)
#define OWI_POWER_OFF()  		    			FL_GPIO_SetOutputPin(GPIOC, FL_GPIO_PIN_12)


#define OWI_DOWN()    						FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_3)
#define OWI_UP()  		    				FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_3)

#define OWI_GET_SCAN_STATE()    	FL_GPIO_GetInputPin(GPIOB , FL_GPIO_PIN_2)
#define OWI_EXTI_INPUT_GROUP    	FL_GPIO_EXTI_INPUT_GROUP2
#define OWI_EXTI_LINE           	FL_GPIO_EXTI_LINE_4



QueueHandle_t 						owiRxQueue;           //数据接收队列
QueueHandle_t 						owiTxQueue;           //数据接收队列
QueueHandle_t 						owiRxTickQueue;       //数据接收队列

int ss = 0;
uint32_t 									owiTxDoTick;          //发送位数据时间戳
uint32_t                  txDoTick;							//发送数据时间戳（间隔发送整条数据）
uint32_t 									owiIRQDoTick;          //发送位数据时间戳

static owiRxTickBuff_t 		owiRxTickBuff;        //接收数据结构体（bit数据）
owiBitDataBuff_t          owiBitDataBuff;       //接收位结构体
owiDataBuff_t             owiRxDataBuff;

const uint8_t  					  OWI_MODE_CMD[3] = {0XB5,0XA6,0XC9};
uint8_t OWIMODEFLAG = 0;

/**
* @brief : owi发送初始化
 *
 * @param : None
 *
 * @retval: 无
 */
 void owiInit(void)
{	
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin         = FL_GPIO_PIN_3;
  GPIO_InitStruct.mode        = FL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.outputType  = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull        = FL_ENABLE;
	GPIO_InitStruct.remapPin    = FL_DISABLE;
	FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.pin         = FL_GPIO_PIN_12;
	FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.pin         = FL_GPIO_PIN_2;
  GPIO_InitStruct.mode        = FL_GPIO_MODE_INPUT;
	FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_2);
	
  FL_EXTI_CommonInitTypeDef  extiCommonInitStruct = {0};
  extiCommonInitStruct.clockSource = FL_CMU_EXTI_CLK_SOURCE_HCLK;    //EXTI中断采样时钟选择
  (void)FL_EXTI_CommonInit(&extiCommonInitStruct);
	
	FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_4);
  FL_GPIO_SetExtiLine4(GPIO, FL_GPIO_EXTI_LINE_4_PB2);
	
	FL_EXTI_InitTypeDef extiInitStruct = {0};
  extiInitStruct.input       = OWI_EXTI_INPUT_GROUP;             //中断线上对应的IO口
  extiInitStruct.filter      = FL_ENABLE;                        //使能数字滤波
  extiInitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_BOTH;   //设置触发边沿  FL_GPIO_EXTI_TRIGGER_EDGE_FALLING
  (void)FL_EXTI_Init(OWI_EXTI_LINE, &extiInitStruct);
	
	
  /*NVIC中断配置*/
  NVIC_DisableIRQ(GPIO_IRQn);
  NVIC_SetPriority(GPIO_IRQn, 2); //中断优先级
  NVIC_EnableIRQ(GPIO_IRQn);
	
	OWI_POWER_ON();
	
	if (NULL == owiRxTickQueue) owiRxTickQueue = xQueueCreate(30, sizeof(owiRxTickBuff_t));//创建接收队列
	if (NULL == owiRxQueue) owiRxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//创建接收队列
	if (NULL == owiTxQueue) owiTxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//创建写入队列
	//创建需要的队列
	
	
}
/**
* @brief : owi发送1
 *
 * @param : None
 *
 * @retval: 无
 */
 void owiSendOne(void)
{	
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,72)){}//延时3/4毫秒
	
	OWI_DOWN();
	
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,24)){}//延时1/4毫秒
	
	OWI_UP();
}
/**
* @brief : owi发送0
 *
 * @param : None
 *
 * @retval: 无
 */
 void owiSendZero(void)
{	
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,24)){}//延时1/4毫秒
	OWI_DOWN();
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,72)){}//延时3/4毫秒
	OWI_UP();
}
/**
 * @brief : owi起始位
 *
 * @param : None
 *
 * @retval: 无
 */
 void owiStart(void)
{	
	owiTxDoTick = SYSGetRunUs();
	OWI_DOWN();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,120)){}//延时1/2毫秒
	OWI_UP();
}
/**
 * @brief : owi终止为
 *
 * @param : None
 *
 * @retval: 无
 */
 void owiStop(void)
{	
	OWI_UP();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,96)){}//延时1毫秒
}
/**
* @brief : owi发送byte
 *
* @param : 需要发送的8位数据
 *
 * @retval: 无
 */
void owiSendByte(uint8_t owiSendB)
{
	if(owiSendB&0x80)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x40)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x20)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x10)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x08)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x04)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x02)
	{
		owiSendOne();
	}
	else
	{
		owiSendZero();
	}
	if(owiSendB&0x01)
	{
		owiSendOne();
	}
	else
	{
	owiSendZero();
	}
}

/**
 * @brief : OWI发送数据前缀
 *
 * @param : 发送数据结构体
 *
 * @retval: 无
 */
void owiSendDataPrefix(owiDataBuff_t owiTxDataBuff)
{	
	if(owiTxDataBuff.len <= 4)
	{
		owiStart();//发送起始位
		owiSendByte(owiTxDataBuff.address);//发送寄存器地址
		if((owiTxDataBuff.len-1)&0x02)//发送数据长度位
		{
			owiSendOne();
		}
		else
		{
			owiSendZero();
		}
		if((owiTxDataBuff.len-1)&0x01)//发送数据长度位
		{
			owiSendOne();
		}
		else
		{
			owiSendZero();
		}
		if(owiTxDataBuff.readOrWrite&0x01)//发送读写控制位
		{
			owiSendOne();
		}
		else
		{
			owiSendZero();
		}
	}
}
/**
 * @brief : OWI写入数据
 *
 * @param : 发送数据结构体
 *
 * @retval: 无
 */
uint8_t owiWriteData(owiDataBuff_t owiTxDataBuff)
{
	OWIMODEFLAG = 1;
	if (NULL == owiTxQueue) owiTxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//创建写入队列
	return xQueueSend(owiTxQueue, &owiTxDataBuff, 0);
}

/**
 * @brief : 进入 OWI 通讯模式
 *
 * @param : None
 *
 * @retval: 无
 */
void owiMode(void)
{
	OWI_POWER_OFF();
	uint32_t owiModeDoTick;
	owiModeDoTick = SYSGetRunMs();
	while(!JudgmentTimeout(SYSGetRunMs(),owiModeDoTick,5000)){}//延时5秒
		
	OWI_POWER_ON();
	
	for(int k = 0;k<10;k++)
	{
		owiStart();
		for(int i = 0;i<3;i++)
		{
			owiSendByte(OWI_MODE_CMD[i]);
		}
		owiStop();
		for(int j = 0;j<300;j++)
		{
			__NOP();
		}
	}
	owiModeDoTick = SYSGetRunMs();
	while(!JudgmentTimeout(SYSGetRunMs(),owiModeDoTick,200)){}//延时5秒
	while(xQueueReceive(owiRxTickQueue,&owiRxTickBuff, 0)!=NULL);
}

/**
 * @brief  owi接收外部中断
 * @param  无
 * @retval 无
 */
void OWI_IRQHandler(void)
{
		if(OWI_GET_SCAN_STATE() == 1)   //如果接收状态为低电平
		{
			owiRxTickBuff.levelUpTick = SYSGetRunUs();   //获取动作时间戳
		}
		else if(OWI_GET_SCAN_STATE() == 0&&owiRxTickBuff.levelUpTick!=0)   //如果接收状态为高电平且下降沿动作时间不为0
		{
			owiRxTickBuff.levelDownTick = SYSGetRunUs();   //获取动作时间戳
			xQueueSend(owiRxTickQueue, &owiRxTickBuff, 0);
			owiRxTickBuff.levelUpTick = 0;
			owiRxTickBuff.levelDownTick = 0;
		}
}

/**
 * @brief : owi接收轮询
 *
 * @param : None
 *
 * @retval: 无
 */
void owiRxPoll(void)
{
	if(xQueuePeek(owiRxTickQueue,&owiRxTickBuff,0))
	{
		for(int i = 0;i<10;i++)
		{
			xQueueReceive(owiRxTickQueue,&owiRxTickBuff,0);
			owiBitDataBuff.pbuf[i] = owiRxTickBuff.levelDownTick-owiRxTickBuff.levelUpTick>58?0:1;
		}
		if((owiBitDataBuff.pbuf[0]^owiBitDataBuff.pbuf[2]^owiBitDataBuff.pbuf[4]^owiBitDataBuff.pbuf[6]==owiBitDataBuff.pbuf[8])&&
			(owiBitDataBuff.pbuf[1]^owiBitDataBuff.pbuf[3]^owiBitDataBuff.pbuf[5]^owiBitDataBuff.pbuf[7]==owiBitDataBuff.pbuf[9]))//奇偶校验判断有效帧
		{
			for(int k = 0;k<8;k++)
			{
				owiRxDataBuff.rxData = (owiRxDataBuff.rxData<<1)|owiBitDataBuff.pbuf[k];
			}
			
		}
		if (NULL == owiRxQueue) owiRxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//创建接收队列
		xQueueSend(owiRxQueue, &owiRxDataBuff, 0);
	}
}
/**
 * @brief : owi控制轮询
 *
 * @param : None
 *
 * @retval: 无
 */
void owiEventPoll(void)
{
	if (NULL == owiRxTickQueue) owiRxTickQueue = xQueueCreate(30, sizeof(owiRxTickBuff_t));//创建接收队列
	
	owiDataBuff_t owiTxDataBuff;
	if(xQueueReceive(owiTxQueue, &owiTxDataBuff, 0))
	{
		if(owiTxDataBuff.readOrWrite == 1)
		{
			owiRxDataBuff.address = owiTxDataBuff.address;
			owiRxDataBuff.readOrWrite = owiTxDataBuff.readOrWrite;
			owiSendDataPrefix(owiTxDataBuff);//发送前缀
		}
		else
		{
			owiSendDataPrefix(owiTxDataBuff);//发送前缀
			for(int i = 0;i<owiTxDataBuff.len;i++)//发送数据内容
			{
				owiSendByte(owiTxDataBuff.pbuf);
			}
			owiStop();//发送停止位
		}
	}
}

/**
* @brief  获取读取数据队列
 * @param  uint32_t类型数据
 * @retval float类型数据
 */
QueueHandle_t getOwiRxQueue(void)
{
	if (NULL == owiRxQueue) owiRxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//创建接收队列
	return owiRxQueue;
}

/**
 * @brief : OWI写入
 *
* @param : address:寄存器地址  Data：寄存器值
 *
 * @retval: 无
 */
void owiSend(uint8_t  address,uint8_t Data)
{
	uint32_t owiSendDoTick;
	owiDataBuff_t owiTxDataBuff;
	owiTxDataBuff.address = address;
	owiTxDataBuff.pbuf = Data;
	owiTxDataBuff.len = 0X01;
	owiTxDataBuff.readOrWrite = 0X00;
	owiWriteData(owiTxDataBuff);
	owiEventPoll();
	owiSendDoTick = SYSGetRunMs();
	while(!JudgmentTimeout(SYSGetRunMs(),owiSendDoTick,100)){}//延时100毫秒
}

/**
 * @brief : OWI修改EE
 *
 * @param : 发送数据结构体
 *
 * @retval: 无
 */
void owiReviseEEPROM(uint8_t  address,uint8_t Data)
{
	uint32_t owiSendDoTick;
	owiMode();
	owiSend(0X30,0X00);
	owiSend(address,Data);
	owiSend(0X30,0X33);
	owiSend(0X6A,0X1E);
	owiSendDoTick = SYSGetRunMs();
	while(!JudgmentTimeout(SYSGetRunMs(),owiSendDoTick,7500)){}//延时100毫秒
	owiSend(0X62,0X00);
}




