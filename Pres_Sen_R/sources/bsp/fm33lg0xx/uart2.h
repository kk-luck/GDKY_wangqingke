/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     uart2.h
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx uart2 driver.
  ******************************************************************************
  */


/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __UART2_H
#define __UART2_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */
	
// Default receive frame timeout(ms)
#define UART2_RX_TIME_DEF   50

// Receive buffer size
#define UART2_RX_BUFF_SIZE  256

// Port default parameter
#define UART2_BAUDRATE_DEF  4800

// 0-None, 1-Odd, 2-Even
#define UART2_PARITY_DEF    0

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
void     uart2_init(void);
void     uart2_set_param(uint32_t baud, uint8_t parity);

uint32_t uart2_send(const uint8_t *pbuf, uint32_t len);
uint32_t uart2_is_rx_valid(void);
uint32_t uart2_get_rx_len(void);
uint8_t *uart2_get_rx_buff(void);
uint32_t uart2_is_rx_valid(void);
uint32_t uart2_get_rx_len_max(void);
uint32_t uart2_get_last_rx_ms(void);
void     uart2_set_rx_stat(uint32_t stat);
void     uart2_set_sleep_stat(uint32_t stat);
/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __UART2_H */

/***************************** END OF FILE *************************************/


