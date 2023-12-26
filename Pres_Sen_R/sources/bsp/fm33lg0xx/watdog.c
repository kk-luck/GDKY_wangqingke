/**
  ******************************************************************************
  *               Copyright(C) 2017-2027 GDKY All Rights Reserved
  *
  * @file    watdog.c
  * @author  TianRuidong
  * @version V1.00
  * @date    
  * @brief   board support packet.
  ******************************************************************************
  */
#define LOG_TAG "watdog"

/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "watdog.h"

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


/**
  * @brief  看门狗初始化
  * @param  void
  * @retval None
  */
void watdogInit(void)
{
    FL_WWDT_InitTypeDef    WWDT_InitStruct;

    WWDT_InitStruct.overflowPeriod = FL_WWDT_PERIOD_1024CNT;

    FL_WWDT_Init(WWDT, &WWDT_InitStruct);    
}




/**
  * @brief  喂狗
  * @param  void
  * @retval None
  */
void feedDog(void)
{
	FL_IWDT_ReloadCounter(IWDT);
}





