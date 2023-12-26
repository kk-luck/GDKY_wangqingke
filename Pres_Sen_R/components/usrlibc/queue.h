/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY  All Rights Reserved
  *
  * @file     libstdc.c
  * @author   TianRuidong
  * @version  V1.01
  * @date     2022-12-1
  * @brief    self-defining standard c library function .
  ******************************************************************************
  */


/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __QUEUE_H
#define __QUEUE_H

#ifdef __cplusplus
 extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include "stdint.h"

/* TYPEDEFS ----------------------------------------------------------------- */

/* 队列结构  */
typedef struct 
{
	int8_t *pcHead;					/*< 指向队列存储区首地址 */
	int8_t *pcTail;			    	/*< 指向队列存储区最后一个地址 */
	int8_t *pcWriteTo;				/*< 下一个要写入的数据地址 */
	int8_t *pcReadFrom;				/*< 下一个要读取的数据地址 */
	
	uint16_t uxMessagesWaiting;     /*< 当前队列中队列项的数量 */
	
	uint16_t uxItemSize;			/*< 每个队列项的大小 */
	uint16_t uxLength;		    	/*< 队列总长度 */
}QueueDefinition;
//xQUEUE

typedef QueueDefinition*  QueueHandle_t;

	 
/* MACROS  ------------------------------------------------------------------ */

#define portMAX_DELAY 0xffffffff

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */



QueueHandle_t xQueueCreate( const uint16_t uxQueueLength, const uint16_t uxItemSize);   //创建队列
void vQueueDelete(QueueHandle_t xQueue);    //删除队列
int8_t xQueueSend(QueueHandle_t xQueue, const void * const pvItemToQueue, uint32_t xTicksToWait);   //队列写入
int8_t xQueueReceive(QueueHandle_t xQueue, void * const pvBuffer, uint32_t xTicksToWait);    //读取队列
int8_t xQueuePeek(QueueHandle_t xQueue, void * const pvBuffer, uint32_t xTicksToWait);       //读取队列，不删除队列中的消息
uint16_t uxQueueMessagesWaiting(QueueHandle_t xQueue);    //读取队列中数据项的个数



#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __LIBSTDC_H */

/***************************** END OF FILE ************************************/

