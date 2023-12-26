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

/* INCLUDES ----------------------------------------------------------------- */

#include "queue.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "includes.h"

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */

#define assert_param(expr) do{if((expr) == 0)for(;;);}while(0)



/**
 * @brief  延时
 * @param  None
 * @retval None
 */
void qdelay_us(uint32_t us)
{
	uint32_t dd;
	while(us--)
	{
		dd = 2;
		while(dd--);
	}
}



/**
 * @brief  创建队列
 * @param  None
 * @retval None
 */
QueueHandle_t xQueueCreate( const uint16_t uxQueueLength, const uint16_t uxItemSize)
{
	QueueHandle_t pxNewQueue = NULL;
	
	pxNewQueue = (QueueHandle_t)malloc(sizeof(QueueDefinition) + (uxQueueLength * uxItemSize));   //分配内存
	if(pxNewQueue != NULL)
	{
		pxNewQueue->uxMessagesWaiting = 0;
		pxNewQueue->uxLength = uxQueueLength;
		pxNewQueue->uxItemSize = uxItemSize;
		pxNewQueue->pcHead = (int8_t*)(((uint8_t*)pxNewQueue) + sizeof(QueueDefinition));    //队列前几个字节存储QueueHandle_t（队列信息） 后面存储 队列中的数据
		pxNewQueue->pcTail = pxNewQueue->pcHead + (uxQueueLength * uxItemSize);
		pxNewQueue->pcWriteTo = pxNewQueue->pcHead;
		pxNewQueue->pcReadFrom = pxNewQueue->pcHead;
	}
	return pxNewQueue;
}


/**
 * @brief  删除队列
 * @param  None
 * @retval None
 */
void vQueueDelete(QueueHandle_t xQueue)
{
	free(xQueue);  //释放内存
	xQueue = NULL;
}


/**
 * @brief  队列写入
 * @param  队列句柄、需要写入的数据指针、等待时间
 * @retval 0、失败    1、成功
 */
int8_t xQueueSend(QueueHandle_t xQueue, const void * const pvItemToQueue, uint32_t xTicksToWait)
{
	uint8_t xReturn = 0;
	if (NULL == xQueue)return xReturn;
	assert_param(pvItemToQueue != NULL);
	assert_param(xQueue != NULL);
	
	do
	{
		if(xQueue->uxMessagesWaiting < xQueue->uxLength)
		{
			SYSEnterCritical();
			
			if(xQueue->pcWriteTo >= xQueue->pcTail)
			{
				xQueue->pcWriteTo = xQueue->pcHead;
			}			
			memcpy((int8_t*)xQueue->pcWriteTo, (int8_t*)pvItemToQueue, ( uint16_t ) xQueue->uxItemSize ); 
			xQueue->pcWriteTo += xQueue->uxItemSize;
			xQueue->uxMessagesWaiting++;
			
			//检查队列参数是否合法
			if(xQueue->pcWriteTo >= xQueue->pcReadFrom)
			{
				if(((xQueue->pcWriteTo - xQueue->pcReadFrom)/xQueue->uxItemSize) != xQueue->uxMessagesWaiting)
				{
					xQueue->uxMessagesWaiting = ((xQueue->pcWriteTo - xQueue->pcReadFrom)/xQueue->uxItemSize);
				}
			}
			else
			{
				if((xQueue->uxLength - ((xQueue->pcReadFrom - xQueue->pcWriteTo)/xQueue->uxItemSize)) != xQueue->uxMessagesWaiting)
				{
					xQueue->uxMessagesWaiting = (xQueue->uxLength - ((xQueue->pcReadFrom - xQueue->pcWriteTo)/xQueue->uxItemSize));
				}
			}
			
			SYSExitCritical();
			
			xReturn = 1;    //队列写入成功
			xTicksToWait = 0;    //队列写入成功，退出。
		}
		if(xTicksToWait!=0)
		{
			qdelay_us(1000);	
		}
		if(xTicksToWait == portMAX_DELAY)
		{
			xTicksToWait = portMAX_DELAY;
		}
	}while(xTicksToWait--);
	
	
	
	return xReturn;
}



/**
 * @brief  读取队列
 * @param  队列句柄、读出数据缓冲区、等待时间
 * @retval None
 */
int8_t xQueueReceive(QueueHandle_t xQueue, void * const pvBuffer, uint32_t xTicksToWait)
{
	uint8_t xReturn = 0;
	if (NULL == xQueue)return xReturn;
	assert_param(pvBuffer != NULL);
	assert_param(xQueue != NULL);
	
	do
	{
		if(xQueue->uxMessagesWaiting > 0)
		{
			SYSEnterCritical();
			
			memcpy((int8_t*)pvBuffer, (int8_t*)xQueue->pcReadFrom, ( uint16_t ) xQueue->uxItemSize ); 
			xQueue->pcReadFrom += xQueue->uxItemSize;
			if(xQueue->pcReadFrom >= xQueue->pcTail)
			{
				xQueue->pcReadFrom = xQueue->pcHead;
			}
			xQueue->uxMessagesWaiting--;
			
			if(xQueue->uxMessagesWaiting == 0)   //如果队列里面的成员为0，则重新初始化队列
			{
				xQueue->uxMessagesWaiting = 0;

				xQueue->pcWriteTo = xQueue->pcHead;
				xQueue->pcReadFrom = xQueue->pcHead;
			}
			else if(xQueue->pcWriteTo >= xQueue->pcReadFrom)//检查队列参数是否合法
			{
				if(((xQueue->pcWriteTo - xQueue->pcReadFrom)/xQueue->uxItemSize) != xQueue->uxMessagesWaiting)
				{
					xQueue->uxMessagesWaiting = ((xQueue->pcWriteTo - xQueue->pcReadFrom)/xQueue->uxItemSize);
				}
			}
			else
			{
				if((xQueue->uxLength - ((xQueue->pcReadFrom - xQueue->pcWriteTo)/xQueue->uxItemSize)) != xQueue->uxMessagesWaiting)
				{
					xQueue->uxMessagesWaiting = (xQueue->uxLength - ((xQueue->pcReadFrom - xQueue->pcWriteTo)/xQueue->uxItemSize));
				}
			}
			
			SYSExitCritical();

			xReturn = 1;    //队列读取成功
			xTicksToWait = 0;    //队列读取成功，退出。
		}
		
		if(xTicksToWait!=0)
		{
			qdelay_us(1000);	
		}
		if(xTicksToWait == portMAX_DELAY)
		{
			xTicksToWait = portMAX_DELAY;
		}
	}while(xTicksToWait--);
	
	return xReturn;
}


/**
* @brief  读取队列，不删除队列中的消息
 * @param  队列句柄、读出数据缓冲区、等待时间
 * @retval None
 */
int8_t xQueuePeek(QueueHandle_t xQueue, void * const pvBuffer, uint32_t xTicksToWait)
{
	uint8_t xReturn = 0;
	if (NULL == xQueue)return xReturn;
	assert_param(pvBuffer != NULL);
	assert_param(xQueue != NULL);
	
	
	do
	{
		if(xQueue->uxMessagesWaiting > 0)
		{
			SYSEnterCritical();
			
			memcpy((int8_t*)pvBuffer, (int8_t*)xQueue->pcReadFrom, ( uint16_t ) xQueue->uxItemSize );
			
			SYSExitCritical();

			xReturn = 1;    //队列读取成功
			xTicksToWait = 0;    //队列读取成功，退出。
		}
		qdelay_us(1000);
		if(xTicksToWait == portMAX_DELAY)
		{
			xTicksToWait = portMAX_DELAY;
		}		
	}while(xTicksToWait--);
	
	return xReturn;
}




/**
* @brief  读取队列中数据项的个数
 * @param  队列句柄
 * @retval None
 */
uint16_t uxQueueMessagesWaiting(QueueHandle_t xQueue)    //
{
	return xQueue->uxMessagesWaiting;
}





