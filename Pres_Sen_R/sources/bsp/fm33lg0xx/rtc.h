/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 ZouZH  All Rights Reserved
  *
  * @file     rtc.h
  * @author   ZouZH
  * @version  V1.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx RTC Service.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
#include "datetime.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

uint8_t rtcInit(void);

uint8_t rtcSetTime(const DateTime_t *ptm);
uint8_t rtcGetTime(DateTime_t *ptime);
uint8_t rtcSetAlarm(uint8_t hour, uint8_t minute);
uint8_t rtcSetWakeup(uint16_t seconds);
uint8_t rtcIsXTLFRun(void);


/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __RTC_H */

/***************************** END OF FILE *************************************/

