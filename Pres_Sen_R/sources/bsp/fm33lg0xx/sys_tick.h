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
#ifndef __SYS_TICK_H
#define __SYS_TICK_H

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

void SysMilliSecondTickInit(void);    //系统毫秒时钟初始化
uint32_t GetMilliSecondCount(void);     //获取系统毫秒计时
void MilliTimerControlPoll(void);    //毫秒定时器关闭轮询

void SysSecondTickInit(void);    //秒嘀嗒定时器初始化
uint32_t GetSysSecondCount(void);    //获取系统秒计时

void SysMicroSecondTickInit(void);//系统微秒时钟初始化
uint32_t GetMicroSecondCount(void);//获取系统微秒计时
void MicroTimerControlPoll(void);//微秒定时器关闭轮询

uint8_t JudgmentTimeout(uint32_t sysTime,uint32_t StartTime,uint32_t TimeOutTime);     //判断延时是否超时

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */






#endif
