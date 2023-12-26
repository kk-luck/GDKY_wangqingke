/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     combus.h
  * @author   ZouZH
  * @version  V1.02
  * @date     30-Nov-2019
  * @brief    Multi-communication serial bus management.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __COMBUS_H
#define __COMBUS_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>

/**
 * @addtogroup ComBus
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * Bus error type
 */
typedef enum
{
  BUS_OK = 0x00,
  BUS_ERR_Param,
  BUS_ERR_Busy,
  BUS_ERR_Timeout,
} BusError_t;

/**
 * bus type
 */
typedef enum
{
  BUS_RS485 = 0,
  BUS_MBUS,
  BUS_UART,
  BUS_BLE,
	BUS_LORA,
  BUS_NUM,
} BusType_t;

/**
 * bus wait (Flag Bit) type
 */
typedef enum
{
  BUS_FB_CONFIG = 0,      /* 参数配置 */
  BUS_FB_CMDPROC,         /* 命令处理 */
  BUS_FB_HM_CYCLE,
  BUS_FB_HM_RT
} BusFlagBit_t;

/* bus wait flag type */
typedef uint16_t BUS_FLAG_TYPE_t;

/**
 * bus manager object
 */
typedef struct
{
  BUS_FLAG_TYPE_t pendingFlag;  /* Waiting to use the bus bit flag, Bit0 highest priority    总线挂起标志（哪个任务正在请求使用总线） */
  uint8_t         runBit;       /* The bit index currently in use(1-sizeof(BUS_FLAG_TYPE_t)), 0-Bus Idle  总线空闲标志（或哪个任务正在使用总线） */   
} BusObj_t;


/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */
BusError_t BusInit(BusType_t BUS_x);
void       BusPoll(void);

BusError_t BusSetWaitFlag(BusType_t BUS_x, uint8_t bitIndex);
BusError_t BusClearWaitFlag(BusType_t BUS_x, uint8_t bitIndex);
BusError_t BusCheckWaitFlag(BusType_t BUS_x, uint8_t bitIndex);
uint8_t    BusIsIdle(BusType_t BUS_x);
BusError_t BusWaitIdle(BusType_t BUS_x, uint8_t bitIndex, uint32_t tmout);


/* Bus hardware interface */
uint8_t    BusSetParam(BusType_t BUS_x, uint32_t baud, uint8_t parity);
uint16_t   BusSend(BusType_t BUS_x, const uint8_t *pbuf, uint16_t len);
uint8_t    BusIsRxValidFrame(BusType_t BUS_x);
uint16_t   BusGetRxLenMax(BusType_t BUS_x);
uint16_t   BusGetRxLen(BusType_t BUS_x);
void      *BusGetRxBuffer(BusType_t BUS_x);
uint32_t   BusLastRxMs(BusType_t BUS_x);
void       BusEnableRx(BusType_t BUS_x);
void       BusDisableRx(BusType_t BUS_x);


/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __COMBUS_H */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

