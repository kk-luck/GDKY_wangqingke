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

volatile uint32_t g_ulSecondCount;   //ϵͳ��δ�ʱ��
volatile uint32_t g_ulMilliSecondCount; //ϵͳ����δ�ʱ��
volatile uint32_t g_ulUsSecondCount; //ϵͳ΢��δ�ʱ��


static uint32_t g_ulUsTimerUseDoTick = 0;   //΢�붨ʱ��ʹ��ʱ���
static uint16_t g_unUsCountUseFlag = 0;     //΢�붨ʱ��ʹ�ñ�־
static uint16_t g_unLastUsCountUseFlag = 0;   //��һ�����ں��붨ʱ��ʹ�ñ�־


/* LOCAL FUNCTIONS ------------------------------------------------------------ */




/**
 * @brief : ϵͳ��ʱ�ӳ�ʼ��
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
	FL_NVIC_Init(&InterruptConfigStruct, LPTIMx_IRQn);    //�����ж����ȼ�
	
	FL_LPTIM32_Enable(LPTIM32);    //��ʱ��ʹ��
}




/**
 * @brief : ϵͳ���ж�
 *
 * @param : 
 *
 * @retval: 
 */
void SysSecondIrqHandler(void)
{
	g_ulSysSecondCount++;
}

//LPTIM32�жϷ������
void LPTIM_IRQHandler(void)
{
    if(FL_LPTIM32_IsEnabledIT_Update(LPTIM32) && FL_LPTIM32_IsActiveFlag_Update(LPTIM32))
    {
        FL_LPTIM32_ClearFlag_Update(LPTIM32);
				SysSecondIrqHandler();
    }
}




/**
 * @brief  ��ȡϵͳ�����
 *
 * @param  ��
 *
 * @retval ϵͳ�δ����
 */
uint32_t GetSysSecondCount(void)
{
  return g_ulSysSecondCount;
}







/**
 * @brief : ϵͳ����ʱ�ӳ�ʼ��
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

	FL_BSTIM32_Init(BSTIM32, &TimerBase_InitStruct);    //��ʱ����ʼ��
	FL_BSTIM32_ClearFlag_Update(BSTIM32);
	FL_BSTIM32_EnableIT_Update(BSTIM32);      //���ж�
	
	InterruptConfigStruct.preemptPriority = 0x02;
	FL_NVIC_Init(&InterruptConfigStruct, BSTIM_IRQn);    //�����ж����ȼ�
	
	FL_BSTIM32_Enable(BSTIM32);    //��ʱ��ʹ��
}



void SysMilliSecondTickEnable(void)
{
	FL_BSTIM32_Enable(BSTIM32);    //��ʱ��ʹ��
}

void SysMilliSecondTickDisenable(void)
{
	FL_BSTIM32_Disable(BSTIM32);    //��ʱ����ʹ��
}

uint8_t SysMilliSecondTickIsEnabled(void)
{
	return FL_BSTIM32_IsEnabled(BSTIM32);
}


/**
 * @brief  ���붨ʱ���ر���ѯ
 *
 * @param  ��
 *
 * @retval 
 */
void MilliTimerControlPoll(void)
{
	if(SysMilliSecondTickIsEnabled() != 0)   //������붨ʱ����ʹ��״̬
	{
		if(g_unLastMilliCountUseFlag == g_unMilliCountUseFlag)  //��������ں��붨ʱ��û��ˢ��
		{
			if(JudgmentTimeout(g_ulSysMilliSecondCount,g_ulMilliTimerUseDoTick,5000))  //�����ʱ
			{
				g_ulMilliTimerUseDoTick = GetMilliSecondCount();
				SysMilliSecondTickDisenable();    //�رպ��붨ʱ��
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
 * @brief  ��ȡ�������
 *
 * @param  ��
 *
 * @retval ϵͳ�δ����
 */
uint32_t GetMilliSecondCount(void)
{
	if(SysMilliSecondTickIsEnabled() == 0)    //���ms��ʱ���ǹر�״̬
	{
		SysMilliSecondTickEnable();   //�򿪺��붨ʱ��
	}
	g_unMilliCountUseFlag++;   //���붨ʱ��ʹ���ź�(�������ʹ��Ms��ʱ��������Ҫˢ�´˺������˺���5S���ϲ�ˢ�£�Ϊ���͹��ģ����Զ��ر�Ms��ʱ��)
	
  return g_ulSysMilliSecondCount;
}


//BSTIM32�жϷ������
void BSTIM_IRQHandler(void)
{
    if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);
        g_ulSysMilliSecondCount++;
    }
}






/**
 * @brief  �ж���ʱ�Ƿ�ʱ
 *
 * @param  1��ϵͳʱ��  2����ʼ��ʱʱ��  3����ʱʱ��
 *
 * @retval ��ʱ����1��û�г�ʱ����0
 */
uint8_t JudgmentTimeout(uint32_t sysTime,uint32_t StartTime,uint32_t TimeOutTime)
{
//	if(sysTime == g_ulSysMilliSecondCount)   //�������ʹ��ms��ʱ��
//	{
//		SysMilliSecondTickEnable();   //�򿪺��붨ʱ��
//		g_unMilliCountUseFlag++;   //���붨ʱ��ʹ���ź�(�������ʹ��Ms��ʱ��������Ҫˢ�´˺������˺���5S���ϲ�ˢ�£�Ϊ���͹��ģ����Զ��ر�Ms��ʱ��)
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


















