/**
  ******************************************************************************
  *
  * @file    : sys_tick.c
  * @author  : TianRuidong
  * @version : V1.00
  * @date    : 2022.12.03
  * @brief   : 
  ******************************************************************************
**/

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __US_TICK_H
#define __US_TICK_H

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

//#define SYSGetRunSec         GetSysSecondCount
//#define SYSGetTick           GetMilliSecondCount
//#define portTICK_PERIOD_MS   1
//#define SYSMsToTick(ms)      ms*portTICK_PERIOD_MS

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

void SysMilliSecondTickInit(void);    //ϵͳ����ʱ�ӳ�ʼ��
uint32_t GetMilliSecondCount(void);     //��ȡϵͳ�����ʱ
void MilliTimerControlPoll(void);    //���붨ʱ���ر���ѯ

void SysSecondTickInit(void);    //����શ�ʱ����ʼ��
uint32_t GetSysSecondCount(void);    //��ȡϵͳ���ʱ

uint8_t JudgmentTimeout(uint32_t sysTime,uint32_t StartTime,uint32_t TimeOutTime);     //�ж���ʱ�Ƿ�ʱ

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */






#endif
