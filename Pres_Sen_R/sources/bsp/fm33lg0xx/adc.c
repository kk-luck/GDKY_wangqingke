/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     adc.c
  * @author   TianRuidong
  * @version  V1.00
  * @date     2022-12-06
  * @brief    Analog input and output.
  ******************************************************************************
  * @history
  */

/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "adc.h"
#include "sys_tick.h"
#include "queue.h"


/**
 * @defgroup AnalogIO
 * @brief Analog input and output
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */


/**
 * ADC动作参数
 */
#define ADC_BUFF_SIZE 5   //ADC 转换平均次数
typedef struct 	
{
	uint8_t actionCommand;   //动作命令
	uint8_t actionStep;      //动作步骤
	uint32_t actionDoTick;   //时间戳
	
	uint16_t adcBuffer[ADC_BUFF_SIZE];
//	uint8_t OneErrTimes;     //错误次数
	uint8_t adcTimes;        //AD转换次数
	uint8_t   finish;         //是否转换完成
	uint8_t errCode;        //错误代码
	
	adcItem_t *adcItem;     //正在转换的通道
	
}adcAction_Obj_t;



/* MACROS  ------------------------------------------------------------------ */


#define ADC_QUEUE_SIZE    10     //ADC队列长度

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

//static uint8_t adcOccupySemaphore;       //ADC被占用信号量   0、没有占用     非0、占用

adcAction_Obj_t adcAction_Obj;     //ADC动作参数
QueueHandle_t QueueAdc;     //ADC转换队列


/* LOCAL FUNCTIONS ---------------------------------------------------------- */


//DMA基础初始化配置
void ADC_DMA_Common_Init(void)
{
    FL_DMA_InitTypeDef DMA_InitStruct = {0};
    FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

		FL_DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;                       /*! DMA外设映射地址 */
    DMA_InitStruct.direction = FL_DMA_DIR_PERIPHERAL_TO_RAM;                          /*! DMA传输方向 */
    DMA_InitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;            /*! RAM地址增长方向 */
    DMA_InitStruct.dataSize = FL_DMA_BANDWIDTH_16B;                                   /*! DAM传输通道数据位宽 */
    DMA_InitStruct.priority = FL_DMA_PRIORITY_HIGH;                                   /*! DMA通道优先级 */
    DMA_InitStruct.circMode = FL_ENABLE;                                              /*! 循环模式 */
    (void)FL_DMA_Init(DMA, &DMA_InitStruct, FL_DMA_CHANNEL_4);
    FL_DMA_Enable(DMA);                                                               //配置DMA全局开关
		
    FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4);                         //清标志
    FL_DMA_EnableIT_TransferComplete(DMA, FL_DMA_CHANNEL_4);                          //配置DMA全程中断

    InterruptConfigStruct.preemptPriority = 2U;                                       //配置DMA的优先级
    FL_NVIC_Init(&InterruptConfigStruct, DMA_IRQn);
}


//DMA地址配置
void ADC_DMA_ADD_Config(uint16_t *buffer, uint32_t length)
{
    FL_DMA_ConfigTypeDef DMA_ConfigStruct = {0};
    DMA_ConfigStruct.memoryAddress = (uint32_t)buffer;                                //配置DMA_RAM地址
    DMA_ConfigStruct.transmissionCount = length - 1;                                  //配置DMA传输长度
    (void)FL_DMA_StartTransmission(DMA, &DMA_ConfigStruct, FL_DMA_CHANNEL_4);
}



/**
 * @brief  Init
 * @param  None
 * @return None
 */
void ADC_Init(void)
{
//	FL_VREFP_InitTypeDef VREFP_InitStructure;     //基准源
  FL_ADC_CommonInitTypeDef ADC_CommonInitStruct;     //ADC时钟和位数设置
  FL_ADC_InitTypeDef ADC_InitStruct;      //ADC工作模式配置
  
//  FL_VREFP_StructInit(&VREFP_InitStructure);
//  VREFP_InitStructure.voltageTrim = FL_VREFP_OUTPUT_VOLTAGE_3P0V_TRIM;  /* 输出电压的TRIM值 */
//  VREFP_InitStructure.outputVoltage = FL_VREFP_OUTPUT_VOLTAGE_3P0V;  /* 输出电压值 */
//  VREFP_InitStructure.mode = FL_VREFP_WORK_MODE_CONTINUOUS;  /* VREFP输出模式 */
//  VREFP_InitStructure.timeOfDriving = FL_VREFP_ENABLE_PERIOD_1MS;  /* 间歇模式下单次驱动时间 */
//  VREFP_InitStructure.timeOfPeriod = FL_VREFP_DRIVING_TIME_4LSCLK;  /* 间歇模式下使能周期 */
//  FL_VREFP_Init(VREFP, &VREFP_InitStructure);
	
  // ADC共用寄存器设置以配置外设工作时钟
  FL_ADC_CommonDeInit();
  FL_ADC_CommonStructInit(&ADC_CommonInitStruct);

  ADC_CommonInitStruct.clockSource            = FL_CMU_ADC_CLK_SOURCE_RCHF;   /** ADC工作时钟源选择 */
  ADC_CommonInitStruct.clockPrescaler         = FL_CMU_ADC_PSC_DIV8;          /** ADCCLK预分频配置 */
  ADC_CommonInitStruct.referenceSource        = FL_ADC_REF_SOURCE_VDDA;       /** ADC基准源选择 */ //   FL_ADC_REF_SOURCE_VREFP  
  ADC_CommonInitStruct.bitWidth               = FL_ADC_BIT_WIDTH_12B;         /** ADC数据位选择 */
  FL_ADC_CommonInit(&ADC_CommonInitStruct);     //ADC共用寄存器设置以配置外设工作时钟
  
  // ADC
  FL_ADC_DeInit(ADC);
  FL_ADC_StructInit(&ADC_InitStruct);
  ADC_InitStruct.conversionMode               = FL_ADC_CONV_MODE_SINGLE;      //连续转换模式 FL_ADC_CONV_MODE_CONTINUOUS  
  ADC_InitStruct.autoMode                     = FL_ADC_SINGLE_CONV_MODE_AUTO;     //自动转换模式  配置ADC转换流程，仅对单次转换有效
  ADC_InitStruct.waitMode                     = FL_ENABLE;                    /*等待模式配置*/
  ADC_InitStruct.overrunMode                  = FL_ENABLE;                    /*覆盖模式配置*/
  ADC_InitStruct.scanDirection                = FL_ADC_SEQ_SCAN_DIR_FORWARD;  /*通道扫描顺序配置*/
  ADC_InitStruct.externalTrigConv             = FL_ADC_TRIGGER_EDGE_NONE;     /*触发信号使能配置*/
  ADC_InitStruct.triggerSource                = FL_ADC_TRGI_LUT0;             /*硬件触发源选择*/
  ADC_InitStruct.fastChannelTime              = FL_ADC_FAST_CH_SAMPLING_TIME_192_ADCCLK;  /*快速通道采样时间配置*/
  ADC_InitStruct.lowChannelTime               = FL_ADC_SLOW_CH_SAMPLING_TIME_192_ADCCLK;  /*慢速通道采样时间配置*/
  ADC_InitStruct.oversamplingMode             = FL_ENABLE;                      /*过采样使能配置*/
  ADC_InitStruct.overSampingMultiplier        = FL_ADC_OVERSAMPLING_MUL_8X;     /*过采样率配置*/
  ADC_InitStruct.oversamplingShift            = FL_ADC_OVERSAMPLING_SHIFT_3B;   /*过采样移位配置*/
  FL_ADC_Init(ADC, &ADC_InitStruct);

  // Channel enable
//  FL_ADC_EnableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH1);   //

	FL_ADC_EnableDMAReq(ADC);                                                           /*使能ADC_DMA*/


	ADC_DMA_Common_Init();   //DMA基础初始化配置

  // Start
//    FL_ADC_ClearFlag_EndOfConversion(ADC);                                             //清标志
//    FL_ADC_Enable(ADC);                                                                //启动ADC
//    FL_ADC_EnableSWConversion(ADC);                                                    //开始转换
	
//  FL_ADC_DisableSequencerChannel(ADC, 0xfffffU);      //清空打开的通道
//  FL_ADC_EnableSWConversion(ADC);    //使能ADC软件触发转换
  
	
  if (NULL == QueueAdc)
    QueueAdc = xQueueCreate(ADC_QUEUE_SIZE, sizeof(adcItem_t*));
  if (NULL == QueueAdc)
  {
//    log_e("ADC queue create failure!");
  }
	
	
//	adcOccupySemaphore = 0;
//	
//	FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV1);                  //配置ADC工作时钟分频
//	FL_ADC_DisableSequencerChannel(ADC, 0xfffffU);      //清空打开的通道
////	FL_ADC_EnableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH7);                  //通道选择

//	FL_ADC_ClearFlag_EndOfConversion(ADC);                        //清标志
////	FL_ADC_Enable(ADC);                                           //启动ADC
}


/**
 * @brief  开始一次ADC采集
 * @param  采集的通道
 * @return None
 */
void adcStart(uint32_t adcChannel)
{
	FL_ADC_DisableSequencerChannel(ADC, 0xfffffU);      //清空打开的通道
	FL_ADC_EnableSequencerChannel(ADC, adcChannel);     //通道选择
	FL_ADC_ClearFlag_EndOfConversion(ADC);              //清标志
	FL_ADC_Enable(ADC);                                 //启动ADC
	FL_ADC_EnableSWConversion(ADC);                     //开始转换
}




//DMA 转换完成中断
void DMA_IRQHandler(void)
{
	uint32_t IE_Flag, IF_Flag;
	IE_Flag = FL_DMA_IsEnabledIT_TransferComplete(DMA, FL_DMA_CHANNEL_4);             //获取中断使能以及中断标志状态
	IF_Flag = FL_DMA_IsActiveFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4);
	if((0x01U == IE_Flag) && (0x01U == IF_Flag))
	{
		FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4);                     //清标志
		
		adcAction_Obj.adcTimes++;   //转换次数 加 1
		if(adcAction_Obj.adcTimes < ADC_BUFF_SIZE)   //如果达到转换次数
		{
			adcStart(adcAction_Obj.adcItem->channel);   //开始AD转换
		}
		else
		{
			adcAction_Obj.finish = 1;    //置转换完成标志
		}
	}
}





/**
 * @brief  应用层ADC开始转换
 * @param  adc转换结构
 * @return 是否开始成功 0、失败    1、成功
 */
uint8_t collectOnceAd(adcItem_t *adcItem)
{
	return xQueueSend(QueueAdc, adcItem, 0);   //需要转换的ADC参数写入队列
}





/**
 * @brief  ADC转换轮询
 * @param  无
 * @return 无
 */
void adcPoll(void)
{
	switch(adcAction_Obj.actionStep)
	{
		case 0:
		{
			if(xQueueReceive(QueueAdc,adcAction_Obj.adcItem,0))   //如果有需要转换的通道
			{
				adcAction_Obj.adcItem->adcValue = 0;
				adcAction_Obj.adcItem->finish = 0;
				
				lib_memset(adcAction_Obj.adcBuffer, 0, sizeof(ADC_BUFF_SIZE));
				adcAction_Obj.adcTimes = 0;   //转换次数清0
				adcAction_Obj.finish = 0;   //清转换完成标志
				adcAction_Obj.actionStep = 1;		
			}
		}break;
		case 1:
		{
			ADC_DMA_ADD_Config(adcAction_Obj.adcBuffer, ADC_BUFF_SIZE);    //重新初始化DMA的BUF的起始地址
			adcStart(adcAction_Obj.adcItem->channel);   //开始AD转换
			adcAction_Obj.actionDoTick = SYSGetTick();    //获取时间戳
			adcAction_Obj.actionStep = 2;	
		}break;
		case 2:    //等待转换完成
		{
			if(adcAction_Obj.finish == 1)   //如果转换完成
			{
				if(ADC_BUFF_SIZE >= 3)
				{
					sort_bubble(adcAction_Obj.adcBuffer, ADC_BUFF_SIZE);    //冒泡法排序
					adcAction_Obj.adcItem->adcValue = temp_avg(&adcAction_Obj.adcBuffer[1], ADC_BUFF_SIZE-2);    //取平均值
				}
				else
				{
					adcAction_Obj.adcItem->adcValue = temp_avg(&adcAction_Obj.adcBuffer[0], ADC_BUFF_SIZE);    //取平均值
				}
				adcAction_Obj.adcItem->finish = 1;   //转换完成
				adcAction_Obj.adcItem = NULL;
				
				adcAction_Obj.actionStep = 0;
			}
			else if(JudgmentTimeout(SYSGetTick(),adcAction_Obj.actionDoTick,SYSMsToTick(500)))   //如果超时
			{
				adcAction_Obj.adcItem->finish = 0xff;    //转换错误
				adcAction_Obj.adcItem->adcValue = 0;
				adcAction_Obj.adcItem = NULL;
				
				adcAction_Obj.actionStep = 0;
			}
		}break;
		default:
		{
			adcAction_Obj.actionStep = 0;
			adcAction_Obj.adcItem = NULL;
		}
	}
}






///**
// * @brief  获取ADC使用权
// * @param  需要获取使用权的任务号
// * @return 0、没有获取到    1、获取到
// */
//uint8_t adcSemaphoreTake(uint8_t taskNum)
//{
//	if(adcOccupySemaphore == 0)
//	{
//		adcOccupySemaphore = taskNum;
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}
//}



///**
// * @brief  释放ADC使用权
// * @param  释放使用权的任务号
// * @return 无
// */
//void adcSemaphoreGive(uint8_t taskNum)
//{
//	if(adcOccupySemaphore == taskNum)
//	{
//		adcOccupySemaphore = 0;
//	}
//}





///**
// * @brief  ADC被占用轮询
// * @param  无
// * @return 无
// */
//void adcUsePoll(void)
//{
//	static uint8_t lastAdcOccupySemaphore;
//	static uint32_t adcUseDoTick;
//	
//	if((lastAdcOccupySemaphore == adcOccupySemaphore) && (adcOccupySemaphore != 0))    //如果ADC被一个任务占用
//	{
//		if(JudgmentTimeout(SYSGetRunSec(),adcUseDoTick,600) == 1)    //如果一个任务占用超过10分钟
//		{
//			adcOccupySemaphore = 0;    //强制解除占用
//		}
//	}
//	else
//	{
//		adcUseDoTick = SYSGetRunSec();   //重制ADC占用时间戳
//	}
//}








