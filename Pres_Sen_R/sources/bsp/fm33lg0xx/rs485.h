/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     rs485.h
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx uart4 driver.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __RS485_H
#define __RS485_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
#include "includes.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */
/**
 * @brief usart run object
 */
typedef struct
{
  volatile uint8_t  rx_valid;
  volatile int rx_len;
  volatile uint32_t rx_tick;

  uint8_t  rx_buff[128];
} rs485_obj_t;
/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
uint8_t  RS485_Init(void);
uint8_t  RS485_SetParam(uint32_t baud, uint8_t parity);

uint16_t RS485_Send(const uint8_t *pbuf, uint16_t len);
uint8_t  RS485_CheckRxPoll(void);
uint16_t RS485_MaxRxLen(void);
uint16_t RS485_GetRxLen(void);
void    *RS485_GetRxBuffer(void);
uint32_t RS485_LastRxTick(void);

void     RS485_EnableRx(void);
void     RS485_DisableRx(void);

void     RS485_SetPower(uint8_t stat);
QueueHandle_t RS485_GetRxQueue(void);
void     RS485_EnterSleep(void);
void     RS485_ExitSleep(void);
void     UART1_IRQHandler(void);
void RS485_memset(void);


/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __RS485_H */

/***************************** END OF FILE *************************************/


