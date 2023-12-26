/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     mbus.c
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx uart1 driver.
  ******************************************************************************
  */


/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "mbus.h"
#include "uart4.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/**
 * @brief usart run object
 */
typedef struct
{
  volatile uint8_t  rx_valid;
  volatile uint16_t rx_len;
  volatile uint32_t rx_tick;

  uint8_t  rx_buff[128];
} mbus_obj_t;


/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

// ms
#define MBUS_FRAME_INTERVAL  50

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

//static mbus_obj_t uart1Obj;

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#define MBUS_Init                         uart4_init
#define MBUS_SetParam(baudrate, parity)   uart4_set_param(baudrate,parity)
#define MBUS_Send(pbuf, len)              uart4_send(pbuf, len)
#define MBUS_LastRxTick                   uart4_get_last_rx_ms


/**
 * @brief  query for valid packets received
 * @param  none
 * @retval 1 have one valid frame
 */
uint8_t MBUS_CheckRxPoll(void)
{
  return uart4_is_rx_valid();
}

/**
 * @brief  get receive data max length.
 * @param  none
 * @return max receive data length
 */
uint16_t MBUS_MaxRxLen(void)
{
  return (uint16_t)uart4_get_rx_len_max();
}

/**
 * @brief  get receive length
 * @param  none
 * @retval receive length
 */
uint16_t MBUS_GetRxLen(void)
{
	return uart4_get_rx_len();
}

/**
 * @brief  get receive data buffer point.
 * @param  none
 * @return receive data buffer point
 */
void *MBUS_GetRxBuffer(void)
{
  return uart4_get_rx_buff();
}




/**
 * @brief  enable receive
 * @param  none
 * @retval none
 */
void MBUS_EnableRx(void)
{
	uart4_set_rx_stat(1);
}


/**
 * @brief  disable receive
 * @param  none
 * @retval none
 */
void MBUS_DisableRx(void)
{
  uart4_set_rx_stat(0);
}

/**
 * @brief  set enter sleep
 * @param  none
 * @retval none
 */
void MBUS_EnterSleep(void)
{
	uart4_set_sleep_stat(1);
}


/**
 * @brief  exit sleep
 * @param  none
 * @retval none
 */
void MBUS_ExitSleep(void)
{
  uart4_set_sleep_stat(0);
}



