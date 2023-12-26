/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     lora_app.c
  * @author   tianruidong
  * @version  V1.00
  * @date     2022-08-15
  * @brief    LORA network maintenance.
  ******************************************************************************
  * @history
  */
#define LOG_TAG    "LORA"

/* INCLUDES ----------------------------------------------------------------- */

#include "includes.h"
#include "lora_wb25.h"
// #include "cmdproc.h"
#include "combus.h"
// #include "record_app.h"
#include "queue.h"

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * 属性
 */
__packed typedef struct 	
{
	uint8_t actionCommand;   //动作指令
	
	uint8_t actionStep;      //动作步骤
	uint32_t actionDoTick;   //门动作时间戳
	uint8_t OneErrTimes;     //
	
	uint8_t cmdAck;    //0 没有应答    非0，收到应答指令的命令字
	uint8_t ackResult;       //应答结果   
	uint8_t networkResult;   //入网结果  0x01-设备入网成功，0x02-设备入网失败，0x03-设备已离网
	uint32_t netKeepDoTick;    //网络维护时间戳（记录最后一次收到数据时的 秒 时间戳）
	
	uint8_t errCode;      //故障代码   0、没故障   1、故障
}wb25Obj_t;


/**
 * 命令缓存,发送命令队列使用
 */
typedef struct
{
  uint16_t len;                        /*!< 数据长度 */
  uint8_t *pbuf;                       /*!< 数据指针 */
  uint8_t sendMode;                    /*!< 发送模式0应答数据    1主动上传数据 */
} loraDataTxBuff_t;

/**
 * 命令缓存,接收命令队列使用
 */
typedef struct
{
  uint16_t len;                        /*!< 数据长度 */
  uint8_t *pbuf;                       /*!< 数据指针 */
} loraDataRxBuff_t;

/* MACROS  ------------------------------------------------------------------ */

#define GPIO_PORT_RST   GPIOE
#define GPIO_PORT_PWR   GPIOD
#define GPIO_PORT_WKP   GPIOD

#define GPIO_PIN_RST    FL_GPIO_PIN_9
#define GPIO_PIN_PWR    FL_GPIO_PIN_6
#define GPIO_PIN_WKP    FL_GPIO_PIN_10


#define LoraRst()         FL_GPIO_ResetOutputPin(GPIO_PORT_RST, GPIO_PIN_RST)
#define LoraNrst()          FL_GPIO_SetOutputPin(GPIO_PORT_RST, GPIO_PIN_RST)

#define loraPowOff()         FL_GPIO_SetOutputPin(GPIO_PORT_PWR, GPIO_PIN_PWR)
#define loraPowOn()          FL_GPIO_ResetOutputPin(GPIO_PORT_PWR, GPIO_PIN_PWR)

#define LoraSleep()         FL_GPIO_ResetOutputPin(GPIO_PORT_WKP, GPIO_PIN_WKP)
#define LoraWakeUp()        FL_GPIO_SetOutputPin(GPIO_PORT_WKP, GPIO_PIN_WKP)
#define LoraWakeUpPoll()        FL_GPIO_ToggleOutputPin(GPIO_PORT_WKP, GPIO_PIN_WKP)


#define  pdTRUE  1

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

wb25Obj_t wb25Obj;    //
QueueHandle_t loraTxQueue;    //数据发送队列
QueueHandle_t loraRxQueue;    //数据接收队列

static uint32_t s_ulWb25IdelDoTick;      //LoRa空闲时间戳

uint8_t  tx_buff[300];   //发送数据缓冲区
uint8_t  tx_len;
uint16_t  tx_cont = 0; //发送数据次数
uint16_t  rx_cont = 0; //接受数据次数


/* LOCAL FUNCTIONS ---------------------------------------------------------- */


void wb25NetKeepPoll(void);     //wb25网络维护（发现一段时间没有接收数据后重新初始化）
	
void resetWb25IdleDoTick(void);   //重置wb25唤醒时钟，唤醒一段时间

/**
 * @brief : LORA_wb25 发送数据次数
 *
 * @param : None
 *
 * @retval: 无
 */
uint16_t wb25txCont(void)
{
  return tx_cont = tx_cont == 999?0:tx_cont;
}
/**
 * @brief : LORA_wb25 接受数据次数
 *
 * @param : None
 *
 * @retval: 无
 */
uint16_t wb25rxCont(void)
{
  return rx_cont = rx_cont == 99?0:rx_cont;
}
/**
 * @brief : LORA_wb25 初始化
 *
 * @param : None
 *
 * @retval: 无
 */
void wb25Init(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStructure;

  // Capture pin(PA4 GPT1_CH3)
  FL_GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.mode         = FL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.pull            = FL_DISABLE;
//  GPIO_InitStruct.remapPin        = FL_DISABLE;
//  GPIO_InitStruct.analogSwitch    = FL_DISABLE;

  GPIO_InitStructure.pin =  GPIO_PIN_RST;
  FL_GPIO_Init(GPIO_PORT_RST, &GPIO_InitStructure);
	
  GPIO_InitStructure.pin =  GPIO_PIN_PWR;
  FL_GPIO_Init(GPIO_PORT_PWR, &GPIO_InitStructure);
	
  GPIO_InitStructure.pin =  GPIO_PIN_WKP;
  FL_GPIO_Init(GPIO_PORT_WKP, &GPIO_InitStructure);
	
	loraPowOff();    //LORA模组断电
	LoraRst();    //复位状态
	LoraSleep();   //模组睡眠
	
	BusInit(BUS_LORA);   //串口初始化
	
  if (NULL == loraTxQueue)
    loraTxQueue = xQueueCreate(5, sizeof(loraDataTxBuff_t));
	
  	s_ulWb25IdelDoTick = SYSGetTick();       //空闲时间戳
	wb25Obj.netKeepDoTick = SYSGetRunSec();    //更新网络维护时间戳
  	wb25Obj.actionStep = 1;
	wb25Obj.actionCommand = 1;    //初始化LORA模组
}




/**
 * @brief : LORA_wb25 重新入网
 *
* @param : 无
 *
 * @retval: 无
 */
void wb25Reconnection(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStructure;

  if(wb25Obj.actionStep == 1)
  {
    // Capture pin(PA4 GPT1_CH3)
    FL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.mode         = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
  //  GPIO_InitStruct.pull            = FL_DISABLE;
  //  GPIO_InitStruct.remapPin        = FL_DISABLE;
  //  GPIO_InitStruct.analogSwitch    = FL_DISABLE;

    GPIO_InitStructure.pin =  GPIO_PIN_RST;
    FL_GPIO_Init(GPIO_PORT_RST, &GPIO_InitStructure);
    
    GPIO_InitStructure.pin =  GPIO_PIN_PWR;
    FL_GPIO_Init(GPIO_PORT_PWR, &GPIO_InitStructure);
    
    GPIO_InitStructure.pin =  GPIO_PIN_WKP;
    FL_GPIO_Init(GPIO_PORT_WKP, &GPIO_InitStructure);
    
    loraPowOff();    //LORA模组断电
    LoraRst();    //复位状态
    LoraSleep();   //模组睡眠
    
    BusInit(BUS_LORA);   //串口初始化
    
    if (NULL == loraTxQueue)
      loraTxQueue = xQueueCreate(5, sizeof(loraDataTxBuff_t));
    
    s_ulWb25IdelDoTick = SYSGetTick();       //空闲时间戳
    wb25Obj.netKeepDoTick = SYSGetRunSec();    //更新网络维护时间戳
    wb25Obj.actionStep = 1;
    wb25Obj.actionCommand = 1;    //初始化LORA模组    
  }
}





/**
 * @brief : LORA_wb25 发送数据
 *
* @param : 发送BUF   数据长度   发送模式:  0应答数据   1主动上传数据
 *
 * @retval: 无
 */
uint8_t loraSendData(const void *pbuf, uint16_t len ,uint8_t SendMode)
{
	uint8_t err;
	loraDataTxBuff_t loraDataTxBuff;
	
	err = 0;
	if (NULL == loraTxQueue)
	{
		loraTxQueue = xQueueCreate(5, sizeof(loraDataTxBuff_t));
		err = 1;
		goto loraSendDataEnd;
	}
	// if (!uxQueueSpacesAvailable(loraTxQueue))
	// {
	// 	// log_w("Receive queue is full!!!");
	// 	err = 2;
	// 	goto loraSendDataEnd;
	// }
	
//	loraDataTxBuff.pbuf = (uint8_t*)pbuf;
	loraDataTxBuff.len = len;
  loraDataTxBuff.sendMode = SendMode;
  loraDataTxBuff.pbuf = SYSMalloc(loraDataTxBuff.len);

	if (NULL == loraDataTxBuff.pbuf)
	{
		err = 3;
		goto loraSendDataEnd;
	}

	lib_memcpy(loraDataTxBuff.pbuf, pbuf, len);
  
	if (pdTRUE != xQueueSend(loraTxQueue, &loraDataTxBuff, 0))    //把要发送的数据放入队列
	{
		err = 4;
		SYSFree(loraDataTxBuff.pbuf);		
		goto loraSendDataEnd;
	}
	
	loraSendDataEnd:
	
	if(err == 0)
	{
		return len;
	}
	else
	{
		return 0;	
	}
}
/**
 * @brief : LORA_wb25 控制轮询处理  
 *
 * @param : None
 *
 * @retval: 无
 */
void wb25EventPoll(void)
{
	static uint32_t wkupPollDoTick;
	
	loraDataTxBuff_t loraDataTxBuff;
	wb25RxCmdHandPoll();     //wb25接收命令轮询处理
	//wb25NetKeepPoll();     //wb25网络维护（发现一段时间没有接收数据后重新初始化）
	
	//如果模组保持唤醒，最少毎5S在唤醒脚发送一个上升沿。
	
	if(JudgmentTimeout(SYSGetRunMs(),wkupPollDoTick,200))    //唤醒信号刷新频率500ms
	{
		wkupPollDoTick = SYSGetRunMs();    //获取ms时间戳
		
		if(wb25Obj.actionStep >= 6)
		{
			LoraWakeUpPoll();    //唤醒轮询
		}
		else
		{
			LoraSleep();    //睡眠状态
		}
	}
	
	switch(wb25Obj.actionStep)
	{
		case 1:    //
		{
			if(wb25Obj.actionCommand == 1)    //如果收到初始化命令
			{
				wb25Obj.actionCommand = 0;
				
				wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 2;   //
			}
			else if (NULL != loraTxQueue)
			{
				if((uxQueueMessagesWaiting(loraTxQueue) != 0) && (wb25Obj.networkResult == 1))   //如果发送队列中有数据需要发送
				{
          LoraWakeUpPoll();    //唤醒轮询
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.OneErrTimes = 0;
					wb25Obj.actionStep = 20;   //
				}
			}
			else
			{
				loraTxQueue = xQueueCreate(5, sizeof(loraDataTxBuff_t));
			}

		}break;
		case 2:    //掉电拉低复位
		{
			loraPowOff();    //LORA模组断电
			LoraRst();    //复位状态
			LoraSleep();   //睡眠状态
			
			wb25Obj.networkResult = 0;    //掉线
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.OneErrTimes = 0;
			wb25Obj.actionStep = 3;   //
		}break;
		case 3:    //等待3秒钟，模组上电
		{
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,3000))    //复位3S钟
			{
				loraPowOn();    //LORA模组上电
				
				wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 4;   //
			}
		}break;
		case 4:    //模组解除复位
		{
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //复位3S钟
			{
				loraPowOn();    //LORA模组上电
				LoraNrst();    //解除复位状态
				
				wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 5;   //
			}
		}break;
		case 5:    //等待模组上电完成
		{
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,5000))    //等待复位完成
			{
				wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep =58;   //
			}
		}break;

		case 58:    //发送进入测试模式命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x07;   //数据长度
			tx_buff[tx_len++] = 0xFB;   //命令字
			tx_buff[tx_len++] = 0x01;   //配置项目
			tx_buff[tx_len++] = 0x03;   //68 00 07 FB 01 03 16    
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 59;   //

		}break;
		case 59:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 60;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 30)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 7;    //进入测试模式错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
//					BusInit(BUS_LORA);   //串口初始化
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 58;   //重新配置
				}
			}
		}break;
		case 60:    //发送写入负载电容值命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x07;   //数据长度
			tx_buff[tx_len++] = 0xFD;   //命令字
			tx_buff[tx_len++] = 0x1C;   //配置项目
			tx_buff[tx_len++] = 0x20;   //68 00 07 FD 1C 20 16      
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 61;   //

		}break;
		case 61:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 62;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 30)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 7;    //写入负载电容值错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
//					BusInit(BUS_LORA);   //串口初始化
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 60;   //重新配置
				}
			}
		}break;
		case 62:    //发送退出测试模式命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x07;   //数据长度
			tx_buff[tx_len++] = 0xFB;   //命令字
			tx_buff[tx_len++] = 0x00;   //配置项目
			tx_buff[tx_len++] = 0x02;   //68 00 07 FB 00 02 16            
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 63;   //

		}break;
		case 63:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 6;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 30)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 7;    //配置MAC地址错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
//					BusInit(BUS_LORA);   //串口初始化
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 62;   //重新配置
				}
			}
		}break;
		case 6:    //发送配置MAC地址命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x0D;   //数据长度
			tx_buff[tx_len++] = 0x90;   //命令字
			tx_buff[tx_len++] = 0x01;   //配置项目
			tx_buff[tx_len++] = 0x30;   //配置MAC地址为30 36 00 37 00 47      
			tx_buff[tx_len++] = 0x36;      
//			tx_buff[tx_len++] = g_tSysParam.devSn[0];
//			tx_buff[tx_len++] = g_tSysParam.devSn[1];
//			tx_buff[tx_len++] = g_tSysParam.devSn[2];
//			tx_buff[tx_len++] = g_tSysParam.devSn[3];
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x37;
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x47;
			
			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
			tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 7;   //

		}break;
		case 7:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 8;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 30)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 7;    //配置MAC地址错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
//					BusInit(BUS_LORA);   //串口初始化
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 6;   //重新配置
				}
			}
		}break;
		case 8:    //发送配置netID地址命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x08;   //数据长度
			tx_buff[tx_len++] = 0x90;   //命令字
			tx_buff[tx_len++] = 0x02;   //配置项目
			// tx_buff[tx_len++] = g_tSysParam.loraNetId;   //netID
			tx_buff[tx_len++] = 0x01;//netID
			
			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
			tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 9;   //

		}break;
		case 9:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 50;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 3)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 9;    //配置MAC地址错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 8;   //重新配置
				}
			}
		}break;
		case 50:    //发送配置通信速率命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x08;   //数据长度
			tx_buff[tx_len++] = 0x90;   //命令字
			tx_buff[tx_len++] = 0x09;   //配置项目
			tx_buff[tx_len++] = 0x05;   //通信速率

			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
			tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();   //获取ms时间戳
			wb25Obj.actionStep = 51;   //

		}break;
		case 51:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 52;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 3)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 51;    //配置错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 50;   //重新配置
				}
			}
		}break;
		case 52:    //发送配置最大字节长度命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x08;   //数据长度
			tx_buff[tx_len++] = 0x90;   //命令字
			tx_buff[tx_len++] = 0x0B;   //配置项目
			tx_buff[tx_len++] = 0x03;   //最大字节长度

			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
			tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 53;   //

		}break;
		case 53:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 10;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,500))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 3)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 53;    //配置错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 52;   //重新配置
				}
			}
		}break;
		case 10:    //发送通信频段命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x08;   //数据长度
			tx_buff[tx_len++] = 0x90;   //命令字
			tx_buff[tx_len++] = 0x03;   //配置项目
		  //tx_buff[tx_len++] = g_tSysParam.band&0xff;   //通信频率用主端口号代替
			tx_buff[tx_len++] = 0x09;
			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
			tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 11;   //

		}break;
		case 11:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 100;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,2000))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 3)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 11;    //配置MAC地址错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 10;   //重新配置
				}
			}
		}break;
		case 100:    //等待模组配置完成，不能配置太快，配置太快不上线
		{
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,5000))    //等待复位完成
			{
				wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 12;   //
			}
		}break;
		case 12:    //发送关闭低功耗命令
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x08;   //数据长度
			tx_buff[tx_len++] = 0x90;   //命令字
			tx_buff[tx_len++] = 0x06;   //配置项目
			tx_buff[tx_len++] = 0x00;
      tx_buff[tx_len++] = 0x9E;
//			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
//			tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 13;   //

		}break;
		case 13:    //等待命令应答
		{
			if(wb25Obj.cmdAck == 0x20&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 14;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,3000))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 3)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 13;    //配置MAC地址错误
					wb25Obj.actionStep = 0;   //					
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 12;   //重新配置
				}
			}
		}break;
		case 14:    //主动入网
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x08;   //数据长度
			tx_buff[tx_len++] = 0x81;   //命令字
			tx_buff[tx_len++] = 0x01;   //配置项目
			tx_buff[tx_len++] = 0x01;   
			tx_buff[tx_len++] = 0x8B;   //校验和
			//tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);
			//tx_len++;
			tx_buff[tx_len++] = 0x16;

			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			wb25Obj.actionStep = 15;   //

		}break;
		case 15:    //等待命令应答
		{
			if((wb25Obj.cmdAck == 0x02) && (wb25Obj.networkResult == 1))  //如果收到应答 且 上线
			{
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 16;   //
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,5000))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 3)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 15;    //错误
					wb25Obj.actionStep = 0;   //	
					wb25Obj.actionCommand = 1; //初始化命令			
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 14;   //重新配置
				}
			}
		}break;
		case 16:    //初始化完成
		{
        wb25Obj.errCode = 0;
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 0;   //
		}break;
		
		case 20:    //开始发送数据
		{
			if(wb25Obj.networkResult == 1)    //如果在线
			{
        wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
        
        if (xQueuePeek(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)
        {
          if(loraDataTxBuff.sendMode == 0)
          {
            wb25Obj.actionStep = 21;   //
          }
          else
          {
            wb25Obj.actionStep = 101;   //主动发送数据时需要等待
          }		          
        }
        else
        {
          wb25Obj.actionStep = 21;   //疑问？？？？？
        }
			}
			else
			{
				wb25Obj.errCode = 20;
				wb25Obj.actionStep = 0;   //		
			}
		}break;
		case 101:    //等待模组唤醒完成，不能太快，太快发送数据失败
		{
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,50))    //等待唤醒完成
			{
				wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 21;   //
			}
		}break;
		case 21:    //发送数据
		{
			wb25Obj.cmdAck = 0;    //清应答标志
			wb25Obj.ackResult = 0;
			
			tx_len = 0;
			tx_buff[tx_len++] = 0x68;   //帧头
			tx_buff[tx_len++] = 0x00;
			tx_buff[tx_len++] = 0x00;   //数据长度
      
      tx_len++;
      
			if (xQueuePeek(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)
			{
				lib_memcpy(&tx_buff[tx_len], loraDataTxBuff.pbuf, loraDataTxBuff.len);
				tx_len += loraDataTxBuff.len;
			}
      
      if(loraDataTxBuff.sendMode == 0)
      {
        tx_buff[3] = 0x84;   //命令字(数据下发应答)
      }
      else
      {
        tx_buff[3] = 0x85;   //命令字(主动上报)
      }

			tx_buff[2] = tx_len+2;  //数据长度
			tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);   //校验
			tx_len++;
			tx_buff[tx_len++] = 0x16;
			
			BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据
			
			wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
			
			if(tx_buff[3] == 0x84)
			{
				wb25Obj.actionStep = 0;   //数据下发应答没有ACK，默认成功。
        wb25Obj.errCode = 0;    //清错误代码
				wb25Obj.OneErrTimes = 0;

				if (xQueueReceive(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)   //取出发送完成的数据
				{
					SYSFree(loraDataTxBuff.pbuf);
				}
				else
				{
					if (xQueueReceive(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)   //取出发送完成的数据
					{
						SYSFree(loraDataTxBuff.pbuf);
					}
				}
			}
			else
			{
				wb25Obj.actionStep = 22;   //
			}

		}break;
		case 22:    //等待发送数据完成
		{
			if(wb25Obj.cmdAck == 0x06&&wb25Obj.ackResult == 0x00)   //如果收到应答
			{
				tx_cont++;
				wb25Obj.networkResult = 1;  //网络状态置1
        wb25Obj.errCode = 0;    //清错误代码
				wb25Obj.OneErrTimes = 0;
				wb25Obj.actionStep = 0;   //

				
        //AppCalUpDelay(0);    //计算上传数据延时(清总线失联计数)
        
				if (xQueueReceive(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)   //取出发送完成的数据
				{
					SYSFree(loraDataTxBuff.pbuf);
				}
				else
				{
					if (xQueueReceive(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)   //取出发送完成的数据
					{
						SYSFree(loraDataTxBuff.pbuf);
					}
				}
			}
			if(JudgmentTimeout(SYSGetRunMs(),wb25Obj.actionDoTick,200))    //等待设置完成
			{
				wb25Obj.OneErrTimes++;
				if(wb25Obj.OneErrTimes >= 100)
				{
					wb25Obj.networkResult = 0;
					wb25Obj.errCode = 22;    //发送数据错误
					wb25Obj.actionStep = 14; 
					
					if (xQueueReceive(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)   //取出发送完成的数据
					{
						SYSFree(loraDataTxBuff.pbuf);
					}
					else
					{
						if (xQueueReceive(loraTxQueue, &loraDataTxBuff, 0) == pdTRUE)   //取出发送完成的数据
						{
							SYSFree(loraDataTxBuff.pbuf);
						}
					}
				}
				else
				{
					wb25Obj.actionDoTick = SYSGetRunMs();    //获取ms时间戳
					wb25Obj.actionStep = 21;   //重新发送
				}
			}
		}break;
		
		default:
		{
			wb25Obj.actionStep = 1;    //
		}break;
	}
}
/**
 * @brief  获取wb25空闲状态
 * @param  None
 * @retval 还需要忙的时间   0 空闲     1还需要运行的时间ms
 */
uint16_t getWb25IdleState(void)
{
  uint16_t runTime = 0;
  
  if((wb25Obj.actionStep >= 6) || (JudgmentTimeout(SYSGetTick(),s_ulWb25IdelDoTick,SYSMsToTick(500)) == 0))
  {
    runTime = 1;
  }
  else
  {
    s_ulWb25IdelDoTick = SYSGetTick() - SYSMsToTick(500) - 1;
    runTime = 0;
  }
  
  return runTime;
}



/**
 * @brief  wb25是否有应答数据要发送
 * @param  None
 * @retval 是否有应答数据要发送   0 无     1有
 */
uint8_t wb25IsAckTx(void)
{
  loraDataTxBuff_t TxBuff1;
  uint8_t state;

  state = 0;

	if (NULL == loraTxQueue)
	{
    if((xQueuePeek(loraTxQueue, &TxBuff1, 0) == pdTRUE) && (wb25Obj.networkResult == 1))   //如果有应答主机的数据要发送
    {
      if(TxBuff1.sendMode == 0)
      {
        state = 1;
      }	          
    }
  }

  return state;
}




/**
 * @brief  获取wb25在线状态
 * @param  None
 * @retval 0无效  0x01-设备入网成功，0x02-设备入网失败，0x03-设备已离网
 */
uint16_t getWb25OnlineState(void)
{
  return wb25Obj.networkResult;
}
/**
 * @brief  获取wb25故障代码
 * @param  None
 * @retval 
 */
uint16_t getWb25ErrCode(void)
{
  return wb25Obj.errCode;
}
/**
 * @brief  重置wb25唤醒时钟，唤醒一段时间
 * @param  None
 * @retval 
 */
void resetWb25IdleDoTick(void)
{
  s_ulWb25IdelDoTick = SYSGetTick();    //收到数据包置休眠时间戳（唤醒一段时间）
}
/**
 * @brief  wb25接收命令轮询处理
 * @param  无
 * @retval 无
 */
void wb25RxCmdHandPoll(void)
{
//  CmdQBuff_t cmdBuff;

	uint16_t i,j;
	
  uint16_t dataLen = 0;   //串口收到的数据总长度
  uint8_t *pbuf = NULL;
	
	uint16_t frameLen = 0;    //协议中的帧长度
	
  if (BusIsRxValidFrame(BUS_LORA) != 0)    //如果LORA串口收到数据
	{
		dataLen  = BusGetRxLen(BUS_LORA);
		pbuf = BusGetRxBuffer(BUS_LORA);
    
//    log_e("Lora Receive Data:");
//    for (uint16_t i = 0; i < dataLen; i++)
//    {
//      xprintf("%02X ", pbuf[i]);
//      if (!((i + 1) % 32))
//        xprintf("\r\n");
//    }   
    
//    BusSend(BUS_MBUS, pbuf, dataLen);     //调试用
		
		if(dataLen >= 6)      //数据包最短也要6个字节
		{
      s_ulWb25IdelDoTick = SYSGetTick();    //收到数据包置休眠时间戳（唤醒一段时间）
      
			j = dataLen - 6;   
			for(i=0;i<j;i++)   //判断数据包是否合法
			{
				if(pbuf[i] == 0x68)   //寻找数据头
				{
					frameLen = UINT16_BUILD(pbuf[i+2],pbuf[i+1]);   //解析出帧长度
					
					if((i+frameLen-1) <= dataLen)   //如果剩余数据够一个完整帧
					{
						if(pbuf[i+frameLen-1] == 0x16)   //数据尾正确
						{
							if(chk_sum8_hex(&pbuf[i+1], frameLen-3) == pbuf[i+frameLen-2])   //校验正确
							{
								
								
								for(int m = 0;m<frameLen;m++)//终端打印
								{
									SEGGER_RTT_printf(0,"%02X ",pbuf[i+m]);
								}
								SEGGER_RTT_printf(0,"\n");
								
								
								
                s_ulWb25IdelDoTick = SYSGetTick();    //收到正确的数据包置休眠时间戳（唤醒一段时间）
                
								wb25Obj.cmdAck = pbuf[i+3];    //置收到命令标志
								wb25Obj.ackResult = pbuf[i+4]; //应答是否成功结果
								
								if(pbuf[i+3] == 0x02) 	//如果是入网信息指令
								{
									wb25Obj.networkResult = pbuf[i+4];     //保存上线标志
								}
								
								if(pbuf[i+3] == 0x03)   //如果收到主机下发数据
								{
 									wb25Obj.netKeepDoTick = SYSGetRunSec();    //更新网络维护时间戳
									
									//loraDataRxBuff_t loraDataRxBuff;
									//if (NULL == loraRxQueue)
									//{
									//loraTxQueue = xQueueCreate(5, sizeof(loraDataRxBuff_t));//创建队列
									//}
									//loraDataRxBuff.len = frameLen-6;//数据长度
									//loraDataRxBuff.pbuf = SYSMalloc(loraDataRxBuff.len);//申请数据空间
									
									//for(int n = 0;n<loraDataRxBuff.len;n++){
									//loraDataRxBuff.pbuf[n] = pbuf[i+4+n];
									//}
									//xQueueSend(loraRxQueue, &loraDataRxBuff, 0);//把收到数据存入队列
									

									
									
									tx_len = 0;
									tx_buff[tx_len++] = 0x68;   //帧头
									tx_buff[tx_len++] = 0x00;
									tx_buff[tx_len++] = 0x00;   //数据长度
									tx_buff[tx_len++] = 0x84;   //命令字
									
									//lib_memcpy(&tx_buff[tx_len],&pbuf[i+4], frameLen-6);
									
									for(int m = 0;m<(frameLen-6)/2;m++){
										tx_buff[4+2*m] = (pbuf[i+4+m]/16)*16+pbuf[i+4+(frameLen-6)/2+m]/16;
										tx_buff[4+2*m+1] = (pbuf[i+4+m]%16)*16+pbuf[i+4+(frameLen-6)/2+m]%16;
									}
									tx_len += frameLen-6;
									tx_buff[2] = tx_len+2;  //数据长度
									tx_buff[tx_len] = chk_sum8_hex(&tx_buff[1],tx_len-1);   //校验
									tx_len++;
									tx_buff[tx_len++] = 0x16;
									rx_cont++;
									BusSend(BUS_LORA, tx_buff, tx_len);    //发送数据  应答
								}
							}
						}
					}
				}
			}	
		}
		BusEnableRx(BUS_LORA);    //串口重新开始接收数据	
	}
}
/**
 * @brief  wb25网络维护（发现一段时间没有接收数据后重新初始化）
 * @param  无
 * @retval 无
 */
void wb25NetKeepPoll(void)
{
	if(JudgmentTimeout(SYSGetRunSec(),wb25Obj.netKeepDoTick,SYSMakeSec(50, 0, 0)))    //如果一天没有接收到数据
	{
		wb25Obj.netKeepDoTick = SYSGetRunSec();    //更新网络维护时间戳
		
		wb25Reconnection();       //LORA_wb25 重新入网
	}
}











