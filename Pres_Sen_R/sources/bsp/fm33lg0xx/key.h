/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     key.h
  * @author   TianRuidong
  * @version  V1.02
  * @date     2022.12.03
  * @brief    Key detection service
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
#include "queue.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/**
 * @brief 按键状态
 */
typedef enum
{
  KEY_DEPRESS = 0,     /*!< 按下 */
  KEY_RELEASE,         /*!< 释放 */
  KEY_VALID,           /*!< Up and Down */
  KEY_LONG,            /*!< 长按 */
  KEY_DBLCLICK,        /*!< 双击 */
  KEY_REPEAT,          /*!< 重复发送 */
} KEY_STATUS_t;


/**
 * @brief 哪一个按键
 */
typedef enum
{
  KEY_V1,
	KEY_NUM
} KEY_VALUE_t;


/**
 * @brief Key event
 */
typedef struct
{
  KEY_VALUE_t  value;	   //键值（哪个按键被按下）
  KEY_STATUS_t state;    //按键状态
} key_event_t;



/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

extern QueueHandle_t shQueueKeyDetector;     //按键队列

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */


void KeyInit(void);    //按键初始化
uint16_t keyIsIdle(void);     //读取按键是否空闲
int8_t KeyPut(const key_event_t *ptEvent);     //放入一个按键
void KeyPoll(void);    //按键轮询
QueueHandle_t keyQueue(void);//输出按键队列


/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __KEY_H */

/***************************** END OF FILE *************************************/

