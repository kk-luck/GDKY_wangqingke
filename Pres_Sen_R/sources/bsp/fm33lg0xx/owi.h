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
 * �����,�����������ʹ��
 */
typedef struct
{
	uint8_t rxData;
  uint8_t readOrWrite;                 /*!< ��д����λ  д:0  ��:1*/
	uint8_t address;                      /*!< ��ַλ */
	uint8_t len;                        /*!< ���ݳ��� */
  uint8_t pbuf;                       /*!< ����ָ�� */
} owiDataBuff_t;

/**
 * ��λ���սṹ��
 */
typedef struct
{
	uint8_t len;                         /*!< ���ݳ��� */
  uint8_t pbuf[10];                       /*!< ����ָ�� */
} owiBitDataBuff_t;
/**
 * ��������ṹ��
 */
typedef struct
{

	uint32_t  levelUpTick;           //�ڶ���������ʱ���
	uint32_t  levelDownTick;           //�½���ʱ���
} owiRxTickBuff_t;


/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

void owiInit(void);
uint8_t owiWriteData(owiDataBuff_t owiTxDataBuff);			  	// OWIд������
void owiEventPoll(void);																//owi������ѯ
QueueHandle_t getOwiRxQueue(void);                      //��ȡ��ȡ���ݶ���
void owiMode(void);
void owiRxPoll(void);                                   //������ѯ
void owiReviseEEPROM(uint8_t  address,uint8_t Data);
void owiSend(uint8_t  address,uint8_t Data);
/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __OWI_H */

/***************************** END OF FILE ************************************/

