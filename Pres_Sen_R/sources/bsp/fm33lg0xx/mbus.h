/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     mbus.h
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx uart1 driver.
  ******************************************************************************
  */


/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __MBUS_H
#define __MBUS_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
uint8_t  MBUS_Init(void);
uint8_t  MBUS_SetParam(uint32_t baud, uint8_t parity);

uint16_t MBUS_Send(const uint8_t *pbuf, uint16_t len);
uint8_t  MBUS_CheckRxPoll(void);
uint16_t MBUS_MaxRxLen(void);
uint16_t MBUS_GetRxLen(void);
void    *MBUS_GetRxBuffer(void);
uint32_t MBUS_LastRxTick(void);

void     MBUS_EnableRx(void);
void     MBUS_DisableRx(void);

void     MBUS_SetPower(uint8_t stat);

void     MBUS_EnterSleep(void);
void     MBUS_ExitSleep(void);


/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __MBUS_H */

/***************************** END OF FILE *************************************/


