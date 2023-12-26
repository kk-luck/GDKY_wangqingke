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



QueueHandle_t 						owiRxQueue;           //���ݽ��ն���
QueueHandle_t 						owiTxQueue;           //���ݽ��ն���
QueueHandle_t 						owiRxTickQueue;       //���ݽ��ն���

int ss = 0;
uint32_t 									owiTxDoTick;          //����λ����ʱ���
uint32_t                  txDoTick;							//��������ʱ�������������������ݣ�
uint32_t 									owiIRQDoTick;          //����λ����ʱ���

static owiRxTickBuff_t 		owiRxTickBuff;        //�������ݽṹ�壨bit���ݣ�
owiBitDataBuff_t          owiBitDataBuff;       //����λ�ṹ��
owiDataBuff_t             owiRxDataBuff;

const uint8_t  					  OWI_MODE_CMD[3] = {0XB5,0XA6,0XC9};
uint8_t OWIMODEFLAG = 0;

/**
* @brief : owi���ͳ�ʼ��
 *
 * @param : None
 *
 * @retval: ��
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
  extiCommonInitStruct.clockSource = FL_CMU_EXTI_CLK_SOURCE_HCLK;    //EXTI�жϲ���ʱ��ѡ��
  (void)FL_EXTI_CommonInit(&extiCommonInitStruct);
	
	FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_4);
  FL_GPIO_SetExtiLine4(GPIO, FL_GPIO_EXTI_LINE_4_PB2);
	
	FL_EXTI_InitTypeDef extiInitStruct = {0};
  extiInitStruct.input       = OWI_EXTI_INPUT_GROUP;             //�ж����϶�Ӧ��IO��
  extiInitStruct.filter      = FL_ENABLE;                        //ʹ�������˲�
  extiInitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_BOTH;   //���ô�������  FL_GPIO_EXTI_TRIGGER_EDGE_FALLING
  (void)FL_EXTI_Init(OWI_EXTI_LINE, &extiInitStruct);
	
	
  /*NVIC�ж�����*/
  NVIC_DisableIRQ(GPIO_IRQn);
  NVIC_SetPriority(GPIO_IRQn, 2); //�ж����ȼ�
  NVIC_EnableIRQ(GPIO_IRQn);
	
	OWI_POWER_ON();
	
	if (NULL == owiRxTickQueue) owiRxTickQueue = xQueueCreate(30, sizeof(owiRxTickBuff_t));//�������ն���
	if (NULL == owiRxQueue) owiRxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//�������ն���
	if (NULL == owiTxQueue) owiTxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//����д�����
	//������Ҫ�Ķ���
	
	
}
/**
* @brief : owi����1
 *
 * @param : None
 *
 * @retval: ��
 */
 void owiSendOne(void)
{	
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,72)){}//��ʱ3/4����
	
	OWI_DOWN();
	
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,24)){}//��ʱ1/4����
	
	OWI_UP();
}
/**
* @brief : owi����0
 *
 * @param : None
 *
 * @retval: ��
 */
 void owiSendZero(void)
{	
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,24)){}//��ʱ1/4����
	OWI_DOWN();
	owiTxDoTick = SYSGetRunUs();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,72)){}//��ʱ3/4����
	OWI_UP();
}
/**
 * @brief : owi��ʼλ
 *
 * @param : None
 *
 * @retval: ��
 */
 void owiStart(void)
{	
	owiTxDoTick = SYSGetRunUs();
	OWI_DOWN();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,120)){}//��ʱ1/2����
	OWI_UP();
}
/**
 * @brief : owi��ֹΪ
 *
 * @param : None
 *
 * @retval: ��
 */
 void owiStop(void)
{	
	OWI_UP();
	while(!JudgmentTimeout(SYSGetRunUs(),owiTxDoTick,96)){}//��ʱ1����
}
/**
* @brief : owi����byte
 *
* @param : ��Ҫ���͵�8λ����
 *
 * @retval: ��
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
 * @brief : OWI��������ǰ׺
 *
 * @param : �������ݽṹ��
 *
 * @retval: ��
 */
void owiSendDataPrefix(owiDataBuff_t owiTxDataBuff)
{	
	if(owiTxDataBuff.len <= 4)
	{
		owiStart();//������ʼλ
		owiSendByte(owiTxDataBuff.address);//���ͼĴ�����ַ
		if((owiTxDataBuff.len-1)&0x02)//�������ݳ���λ
		{
			owiSendOne();
		}
		else
		{
			owiSendZero();
		}
		if((owiTxDataBuff.len-1)&0x01)//�������ݳ���λ
		{
			owiSendOne();
		}
		else
		{
			owiSendZero();
		}
		if(owiTxDataBuff.readOrWrite&0x01)//���Ͷ�д����λ
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
 * @brief : OWIд������
 *
 * @param : �������ݽṹ��
 *
 * @retval: ��
 */
uint8_t owiWriteData(owiDataBuff_t owiTxDataBuff)
{
	OWIMODEFLAG = 1;
	if (NULL == owiTxQueue) owiTxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//����д�����
	return xQueueSend(owiTxQueue, &owiTxDataBuff, 0);
}

/**
 * @brief : ���� OWI ͨѶģʽ
 *
 * @param : None
 *
 * @retval: ��
 */
void owiMode(void)
{
	OWI_POWER_OFF();
	uint32_t owiModeDoTick;
	owiModeDoTick = SYSGetRunMs();
	while(!JudgmentTimeout(SYSGetRunMs(),owiModeDoTick,5000)){}//��ʱ5��
		
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
	while(!JudgmentTimeout(SYSGetRunMs(),owiModeDoTick,200)){}//��ʱ5��
	while(xQueueReceive(owiRxTickQueue,&owiRxTickBuff, 0)!=NULL);
}

/**
 * @brief  owi�����ⲿ�ж�
 * @param  ��
 * @retval ��
 */
void OWI_IRQHandler(void)
{
		if(OWI_GET_SCAN_STATE() == 1)   //�������״̬Ϊ�͵�ƽ
		{
			owiRxTickBuff.levelUpTick = SYSGetRunUs();   //��ȡ����ʱ���
		}
		else if(OWI_GET_SCAN_STATE() == 0&&owiRxTickBuff.levelUpTick!=0)   //�������״̬Ϊ�ߵ�ƽ���½��ض���ʱ�䲻Ϊ0
		{
			owiRxTickBuff.levelDownTick = SYSGetRunUs();   //��ȡ����ʱ���
			xQueueSend(owiRxTickQueue, &owiRxTickBuff, 0);
			owiRxTickBuff.levelUpTick = 0;
			owiRxTickBuff.levelDownTick = 0;
		}
}

/**
 * @brief : owi������ѯ
 *
 * @param : None
 *
 * @retval: ��
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
			(owiBitDataBuff.pbuf[1]^owiBitDataBuff.pbuf[3]^owiBitDataBuff.pbuf[5]^owiBitDataBuff.pbuf[7]==owiBitDataBuff.pbuf[9]))//��żУ���ж���Ч֡
		{
			for(int k = 0;k<8;k++)
			{
				owiRxDataBuff.rxData = (owiRxDataBuff.rxData<<1)|owiBitDataBuff.pbuf[k];
			}
			
		}
		if (NULL == owiRxQueue) owiRxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//�������ն���
		xQueueSend(owiRxQueue, &owiRxDataBuff, 0);
	}
}
/**
 * @brief : owi������ѯ
 *
 * @param : None
 *
 * @retval: ��
 */
void owiEventPoll(void)
{
	if (NULL == owiRxTickQueue) owiRxTickQueue = xQueueCreate(30, sizeof(owiRxTickBuff_t));//�������ն���
	
	owiDataBuff_t owiTxDataBuff;
	if(xQueueReceive(owiTxQueue, &owiTxDataBuff, 0))
	{
		if(owiTxDataBuff.readOrWrite == 1)
		{
			owiRxDataBuff.address = owiTxDataBuff.address;
			owiRxDataBuff.readOrWrite = owiTxDataBuff.readOrWrite;
			owiSendDataPrefix(owiTxDataBuff);//����ǰ׺
		}
		else
		{
			owiSendDataPrefix(owiTxDataBuff);//����ǰ׺
			for(int i = 0;i<owiTxDataBuff.len;i++)//������������
			{
				owiSendByte(owiTxDataBuff.pbuf);
			}
			owiStop();//����ֹͣλ
		}
	}
}

/**
* @brief  ��ȡ��ȡ���ݶ���
 * @param  uint32_t��������
 * @retval float��������
 */
QueueHandle_t getOwiRxQueue(void)
{
	if (NULL == owiRxQueue) owiRxQueue = xQueueCreate(5, sizeof(owiDataBuff_t));//�������ն���
	return owiRxQueue;
}

/**
 * @brief : OWIд��
 *
* @param : address:�Ĵ�����ַ  Data���Ĵ���ֵ
 *
 * @retval: ��
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
	while(!JudgmentTimeout(SYSGetRunMs(),owiSendDoTick,100)){}//��ʱ100����
}

/**
 * @brief : OWI�޸�EE
 *
 * @param : �������ݽṹ��
 *
 * @retval: ��
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
	while(!JudgmentTimeout(SYSGetRunMs(),owiSendDoTick,7500)){}//��ʱ100����
	owiSend(0X62,0X00);
}




