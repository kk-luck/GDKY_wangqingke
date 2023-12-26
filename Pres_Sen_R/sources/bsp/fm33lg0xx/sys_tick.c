/**
  ******************************************************************************
  *               Copyright(C)   All Rights Reserved
  *
  * @file    : sys_tick.c
  * @author  : TianRuidong
  * @version : V1.0
  * @date    : 2022.12.03
  * @brief   : 
  ******************************************************************************
  */


/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "sys_tick.h"


/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

volatile uint32_t g_ulSysSecondCount;   //系统秒滴答时间
volatile uint32_t g_ulSysMilliSecondCount; //系统毫秒滴答时间
volatile uint32_t g_ulSysMicroSecondCount; //系统微秒滴答时间


static uint32_t g_ulMilliTimerUseDoTick = 0;   //微秒定时器使用时间戳
static uint32_t g_ulMicroTimerDoTick = 0;      //毫秒定时器使用微秒时间戳
static uint16_t g_unMicroCountUseFlag = 0;     //微秒定时器使用标志
static uint16_t g_unLastMicroCountUseFlag = 0;   //上一个周期微秒定时器使用标志



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
	FL_LPTIM32_InitTypeDef    TimerBase_InitStruct;
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

	TimerBase_InitStruct.clockSource = FL_CMU_LPTIM32_CLK_SOURCE_RCLP;
	TimerBase_InitStruct.prescalerClockSource = FL_LPTIM32_CLK_SOURCE_INTERNAL;
	TimerBase_InitStruct.prescaler = FL_LPTIM32_PSC_DIV1;
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
 * @brief : 系统微秒时钟初始化
 *
 * @param : 
 *
 * @retval: 
 */
void SysMicroSecondTickInit(void)
{
	FL_BSTIM32_InitTypeDef    TimerBase32_InitStruct;
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

	TimerBase32_InitStruct.prescaler = 7;
	TimerBase32_InitStruct.autoReload = 4;
	TimerBase32_InitStruct.autoReloadState = FL_DISABLE;
	TimerBase32_InitStruct.clockSource = FL_CMU_BSTIM32_CLK_SOURCE_APBCLK;

	FL_BSTIM32_Init(BSTIM32, &TimerBase32_InitStruct);    //定时器初始化
	FL_BSTIM32_ClearFlag_Update(BSTIM32);
	FL_BSTIM32_EnableIT_Update(BSTIM32);      //打开中断
	
	FL_BSTIM32_Enable(BSTIM32);    //定时器使能
	
	InterruptConfigStruct.preemptPriority = 0x02;
	FL_NVIC_Init(&InterruptConfigStruct, BSTIM_IRQn);    //设置中断优先级
	
	FL_BSTIM16_Enable(BSTIM16);    //定时器使能
}
void SysMicroSecondTickEnable(void)
{
	FL_BSTIM32_Enable(BSTIM32);    //定时器使能
}

void SysMicroSecondTickDisenable(void)
{
	FL_BSTIM32_Disable(BSTIM32);    //定时器不使能
}

uint8_t SysMicroSecondTickIsEnabled(void)
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
void MicroTimerControlPoll(void)
{
	if(SysMicroSecondTickIsEnabled() != 0)   //如果毫秒定时器是使能状态
	{
		if(g_unLastMicroCountUseFlag == g_unMicroCountUseFlag)  //如果本周期毫秒定时器没有刷新
		{
			if(JudgmentTimeout(g_ulSysMilliSecondCount,g_ulMilliTimerUseDoTick,5000))  //如果超时
			{
				g_ulMilliTimerUseDoTick = GetMicroSecondCount();
				SysMicroSecondTickDisenable();    //关闭毫秒定时器
			}
		}
		else
		{
			g_unLastMicroCountUseFlag = g_unMicroCountUseFlag;
			g_ulMilliTimerUseDoTick = GetMicroSecondCount();
		}		
	}
}
/**
 * @brief  获取95分之1毫秒计数
 *
 * @param  无
 *
 * @retval 系统滴答计数
 */
uint32_t GetMicroSecondCount(void)
{
	if(SysMicroSecondTickIsEnabled() == 0)    //如果us定时器是关闭状态
	{
		SysMicroSecondTickEnable();   //打开微秒定时器
	}
	g_unMicroCountUseFlag++;   //微秒定时器使用信号(如果正在使用Us定时器，则需要刷新此函数，此函数5S以上不刷新，为降低功耗，则自动关闭Ms定时器)
	
  return g_ulSysMicroSecondCount;
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
	if(JudgmentTimeout(g_ulSysMicroSecondCount,g_ulMicroTimerDoTick,9))
	{
		g_ulMicroTimerDoTick = g_ulSysMicroSecondCount;
		g_ulSysMilliSecondCount++;
	}
	return g_ulSysMilliSecondCount;
}


//BSTIM32中断服务程序
void BSTIM_IRQHandler(void)
{
    if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);
        g_ulSysMicroSecondCount++;
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
//	if(sysTime == g_ulSysMicroSecondCount)   //如果正在使用ms定时器
//	{
//		SysMicroSecondTickEnable();   //打开毫秒定时器
//		g_unMicroCountUseFlag++;   //毫秒定时器使用信号(如果正在使用Ms定时器，则需要刷新此函数，此函数5S以上不刷新，为降低功耗，则自动关闭Ms定时器)
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







