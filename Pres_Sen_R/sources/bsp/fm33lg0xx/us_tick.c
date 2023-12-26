/**
  ******************************************************************************
  *               Copyright(C)   All Rights Reserved
  *
  * @file    : us_tick.c
  * @author  : wqk
  * @version : V1.0
  * @date    : 2022.12.03
  * @brief   : 
  ******************************************************************************
  */


/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "us_tick.h"


/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

volatile uint32_t g_ulSecondCount;   //系统秒滴答时间
volatile uint32_t g_ulMilliSecondCount; //系统毫秒滴答时间
volatile uint32_t g_ulUsSecondCount; //系统微秒滴答时间


static uint32_t g_ulUsTimerUseDoTick = 0;   //微秒定时器使用时间戳
static uint16_t g_unUsCountUseFlag = 0;     //微秒定时器使用标志
static uint16_t g_unLastUsCountUseFlag = 0;   //上一个周期毫秒定时器使用标志


/* LOCAL FUNCTIONS ------------------------------------------------------------ */




/**
 * @brief : 系统秒时钟初始化
 *
 * @param : 
 *
 * @retval: 
 */
void SysSecondTickInit(void)
{
	FL_LPTIM16_InitTypeDef    TimerBase_InitStruct;
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

	TimerBase_InitStruct.clockSource = FL_CMU_LPTIM16_CLK_SOURCE_RCLP;
	TimerBase_InitStruct.prescalerClockSource = FL_LPTIM16_CLK_SOURCE_INTERNAL;
	TimerBase_InitStruct.prescaler = FL_LPTIM16_PSC_DIV1;
	TimerBase_InitStruct.autoReload = 32768;
	TimerBase_InitStruct.mode = FL_LPTIM32_OPERATION_MODE_NORMAL;
	TimerBase_InitStruct.onePulseMode = FL_LPTIM32_ONE_PULSE_MODE_CONTINUOUS;
	TimerBase_InitStruct.triggerEdge = FL_LPTIM32_ETR_TRIGGER_EDGE_RISING;
	TimerBase_InitStruct.countEdge = FL_LPTIM32_ETR_COUNT_EDGE_RISING;

	FL_LPTIM32_Init(LPTIM32, &TimerBase_InitStruct);
	FL_LPTIM32_ClearFlag_Update(LPTIM32);
	FL_LPTIM32_EnableIT_Update(LPTIM32);  
	
	InterruptConfigStruct.preemptPriority = 0x02;
	FL_NVIC_Init(&InterruptConfigStruct, LPTIMx_IRQn);    //设置中断优先级
	
	FL_LPTIM32_Enable(LPTIM32);    //定时器使能
}




/**
 * @brief : 系统秒中断
 *
 * @param : 
 *
 * @retval: 
 */
void SysSecondIrqHandler(void)
{
	g_ulSysSecondCount++;
}

//LPTIM32中断服务程序
void LPTIM_IRQHandler(void)
{
    if(FL_LPTIM32_IsEnabledIT_Update(LPTIM32) && FL_LPTIM32_IsActiveFlag_Update(LPTIM32))
    {
        FL_LPTIM32_ClearFlag_Update(LPTIM32);
				SysSecondIrqHandler();
    }
}




/**
 * @brief  获取系统秒计数
 *
 * @param  无
 *
 * @retval 系统滴答计数
 */
uint32_t GetSysSecondCount(void)
{
  return g_ulSysSecondCount;
}







/**
 * @brief : 系统毫秒时钟初始化
 *
 * @param : 
 *
 * @retval: 
 */
void SysMilliSecondTickInit(void)
{
	FL_BSTIM32_InitTypeDef    TimerBase_InitStruct;
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

	TimerBase_InitStruct.prescaler = 8;
	TimerBase_InitStruct.autoReload = 1000;
	TimerBase_InitStruct.autoReloadState = FL_ENABLE;
	TimerBase_InitStruct.clockSource = FL_CMU_BSTIM32_CLK_SOURCE_APBCLK;

	FL_BSTIM32_Init(BSTIM32, &TimerBase_InitStruct);    //定时器初始化
	FL_BSTIM32_ClearFlag_Update(BSTIM32);
	FL_BSTIM32_EnableIT_Update(BSTIM32);      //打开中断
	
	InterruptConfigStruct.preemptPriority = 0x02;
	FL_NVIC_Init(&InterruptConfigStruct, BSTIM_IRQn);    //设置中断优先级
	
	FL_BSTIM32_Enable(BSTIM32);    //定时器使能
}



void SysMilliSecondTickEnable(void)
{
	FL_BSTIM32_Enable(BSTIM32);    //定时器使能
}

void SysMilliSecondTickDisenable(void)
{
	FL_BSTIM32_Disable(BSTIM32);    //定时器不使能
}

uint8_t SysMilliSecondTickIsEnabled(void)
{
	return FL_BSTIM32_IsEnabled(BSTIM32);
}


/**
 * @brief  毫秒定时器关闭轮询
 *
 * @param  无
 *
 * @retval 
 */
void MilliTimerControlPoll(void)
{
	if(SysMilliSecondTickIsEnabled() != 0)   //如果毫秒定时器是使能状态
	{
		if(g_unLastMilliCountUseFlag == g_unMilliCountUseFlag)  //如果本周期毫秒定时器没有刷新
		{
			if(JudgmentTimeout(g_ulSysMilliSecondCount,g_ulMilliTimerUseDoTick,5000))  //如果超时
			{
				g_ulMilliTimerUseDoTick = GetMilliSecondCount();
				SysMilliSecondTickDisenable();    //关闭毫秒定时器
			}
		}
		else
		{
			g_unLastMilliCountUseFlag = g_unMilliCountUseFlag;
			g_ulMilliTimerUseDoTick = GetMilliSecondCount();
		}		
	}
}



/**
 * @brief  获取毫秒计数
 *
 * @param  无
 *
 * @retval 系统滴答计数
 */
uint32_t GetMilliSecondCount(void)
{
	if(SysMilliSecondTickIsEnabled() == 0)    //如果ms定时器是关闭状态
	{
		SysMilliSecondTickEnable();   //打开毫秒定时器
	}
	g_unMilliCountUseFlag++;   //毫秒定时器使用信号(如果正在使用Ms定时器，则需要刷新此函数，此函数5S以上不刷新，为降低功耗，则自动关闭Ms定时器)
	
  return g_ulSysMilliSecondCount;
}


//BSTIM32中断服务程序
void BSTIM_IRQHandler(void)
{
    if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);
        g_ulSysMilliSecondCount++;
    }
}






/**
 * @brief  判断延时是否超时
 *
 * @param  1、系统时钟  2、开始计时时间  3、超时时间
 *
 * @retval 超时返回1，没有超时返回0
 */
uint8_t JudgmentTimeout(uint32_t sysTime,uint32_t StartTime,uint32_t TimeOutTime)
{
//	if(sysTime == g_ulSysMilliSecondCount)   //如果正在使用ms定时器
//	{
//		SysMilliSecondTickEnable();   //打开毫秒定时器
//		g_unMilliCountUseFlag++;   //毫秒定时器使用信号(如果正在使用Ms定时器，则需要刷新此函数，此函数5S以上不刷新，为降低功耗，则自动关闭Ms定时器)
//	}
	
  if((uint32_t)(sysTime-StartTime)>=(uint32_t)TimeOutTime)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


















