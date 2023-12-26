/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     analogio.h
  * @author   ZouZH
  * @version  V1.00
  * @date     03-Nov-2018
  * @brief    Analog input and output.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>

/**
 * @addtogroup AnalogIO
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * @brief ADC 转换队列结构
 */
typedef struct
{
  uint32_t  channel;	   //ADC转换通道
  uint8_t   finish;         //是否转换完成
	uint16_t  adcValue;     //转换结果
} adcItem_t;


	
/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */


void ADC_Init(void);


uint8_t collectOnceAd(adcItem_t *adcItem);   //应用层ADC开始转换 0、失败    1、成功
void adcPoll(void);    //ADC转换轮询


//uint8_t adcSemaphoreTake(uint8_t taskNum);    //获取信号量
//void adcSemaphoreGive(uint8_t taskNum);       //释放信号量
//void adcUsePoll(void);
	
/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __ANALOGIO_H */

/**
 * @}
 */

/***************************** END OF FILE ************************************/


