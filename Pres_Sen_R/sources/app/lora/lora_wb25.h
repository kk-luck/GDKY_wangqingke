/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     lora_app.h
  * @author   ZouZH
  * @version  V1.00
  * @date     07-08-2018
  * @brief    NB-IoT network maintenance.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __LORA_WB25_H
#define __LORA_WB25_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>

#if LORA_CFG_OS
#include "FreeRTOS.h"
#include "queue.h"
#endif

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */


uint16_t wb25txCont(void);			//LORA_wb25发送次数
uint16_t wb25rxCont(void);			//LORA_wb25接收次数
void wb25Init(void);       //LORA_wb25 初始化
uint8_t loraSendData(const void *pbuf, uint16_t len ,uint8_t SendMode);    //lora发送数据
void wb25EventPoll(void);    //LORA_wb25 控制轮询处理

void wb25RxCmdHandPoll(void);    //wb25接收命令轮询处理


void wb25Reconnection(void);   //LORA_wb25 重新入网

uint16_t getWb25OnlineState(void);    //获取wb25在线状态  0无效  0x01-设备入网成功，0x02-设备入网失败，0x03-设备已离网
uint16_t getWb25ErrCode(void);     //获取wb25故障代码
uint16_t getWb25IdleState(void);      //获取wb25空闲状态
void resetWb25IdleDoTick(void);       //重置wb25唤醒时钟，唤醒一段时间

uint8_t wb25IsAckTx(void);     //wb25是否有应答数据要发送  0 无     1有


/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __LORA_APP_H */

/**
 * @}
 */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

