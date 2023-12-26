/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     owi.h
  * @author   WangQk
  * @version  V1.00
  * @date     2023-09-15
  * @brief    OWI network maintenance.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __OWI_H
#define __OWI_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>
#include "queue.h"


/**
 * 命令缓存,发送命令队列使用
 */
typedef struct
{
	uint8_t rxData;
  uint8_t readOrWrite;                 /*!< 读写控制位  写:0  读:1*/
	uint8_t address;                      /*!< 地址位 */
	uint8_t len;                        /*!< 数据长度 */
  uint8_t pbuf;                       /*!< 数据指针 */
} owiDataBuff_t;

/**
 * 安位接收结构体
 */
typedef struct
{
	uint8_t len;                         /*!< 数据长度 */
  uint8_t pbuf[10];                       /*!< 数据指针 */
} owiBitDataBuff_t;
/**
 * 接收命令结构体
 */
typedef struct
{

	uint32_t  levelUpTick;           //第二次上升沿时间戳
	uint32_t  levelDownTick;           //下降沿时间戳
} owiRxTickBuff_t;


/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

void owiInit(void);
uint8_t owiWriteData(owiDataBuff_t owiTxDataBuff);			  	// OWI写入数据
void owiEventPoll(void);																//owi控制轮询
QueueHandle_t getOwiRxQueue(void);                      //获取读取数据队列
void owiMode(void);
void owiRxPoll(void);                                   //接收轮询
void owiReviseEEPROM(uint8_t  address,uint8_t Data);
void owiSend(uint8_t  address,uint8_t Data);
/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __OWI_H */

/***************************** END OF FILE ************************************/

