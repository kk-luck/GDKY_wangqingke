/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     menu.h
  * @author   ZouZH
  * @version  V1.00
  * @date     28-June-2019
  * @brief    menu display.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MENU_H
#define __MENU_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>
#include "lcd.h"

/**
 * @addtogroup MENU
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 *  @brief Menu fsm item
 */
typedef enum
{
  MENU_ITEM_Start = 0,            /*!< 开始 */
  MENU_ITEM_StartSn,              /*!< 上电显示设备号 */
  MENU_ITEM_StartVer,             /*!< 上电显示版本号 */

  MENU_ITEM_Top,                  /*!< 顶层菜单 */
  MENU_ITEM_Heat,                 /*!< 热量 */
  MENU_ITEM_WaterTemp,            /*!< 供回温 */
  MENU_ITEM_Press,                /*!< 供回压 */
  MENU_ITEM_Ammeter,              /*!< 电量 */
  MENU_ITEM_Date,                 /*!< 日期 */
  MENU_ITEM_Time,                 /*!< 时间 */
  MENU_ITEM_RoomTemp,             /*!< 室温-设温+上下限 */
  MENU_ITEM_RoomFloor,            /*!< 中继-楼层 */
  MENU_ITEM_OpenTime,             /*!< 接通时间 */
  MENU_ITEM_CloseTime,            /*!< 断开时间 */
  MENU_ITEM_LockTime,             /*!< 锁闭时间 */
  MENU_ITEM_AdjNum,               /*!< 开度设定-反馈+上下限 */
  MENU_ITEM_ValveDN,              /*!< 阀口径-档位-电压值 */
  MENU_ITEM_FIRST_END,            /*!< 一级菜单结束 */

  // 二级菜单
  MENU_ITEM_DevSN,                /*!< 设备编号+通讯地址 */
  MENU_ITEM_SoftVer,              /*!< 软件版本+硬件版本 */
  MENU_ITEM_ErrCode,              /*!< 错误代码 */
  MENU_ITEM_OpenRatio,            /*!< 接通比 */
  MENU_ITEM_HeatSeason,           /*!< 采暖季 */
  MENU_ITEM_ValveDelay,           /*!< 阀门延时时间 */
  MENU_ITEM_ValveCheck,           /*!< 阀门检测时间 */
  MENU_ITEM_ValveStatus,          /*!< 阀门当前状态 */
//  MENU_ITEM_NB_IMEI,              /*!< NB-IoT IMEI */
//  MENU_ITEM_NB_IMSI,              /*!< NB-IoT IMSI */
//  MENU_ITEM_NB_IP,                /*!< NB-IoT IP */
//  MENU_ITEM_NB_ERROR_STATE,       /*!< NB-IoT 错误状态-运行状态 */
//  MENU_ITEM_NB_RSSI,              /*!< NB-IoT 信号强度-连接状态-模组状态 */
  MENU_ITEM_NB_CYCLE,             /*!< NB-IoT、LORA 实时-周期上传间隔 */
  MENU_ITEM_LORA_ONLINE,          /*!< LORA 在线标志 */
  MENU_ITEM_LORA_NETID_BAND,      /*!< LORA 网络ID、频段 */
  MENU_ITEM_BAT_VOLTAGE,          /*!< 电池电压 */

  MENU_ITEM_SECOND_END,           /*!< 二级菜单结束 */

  // 三级菜单
  MENU_ITEM_HM_WaterTemp,         /*!< 楼表供回温(通断控制器) */
  MENU_ITEM_RoomCtrl,             /*!< 温控安装标志 */
  MENU_ITEM_RoomTempRemote,       /*!< 室温修正-远程设温+远程上下限 */
  MENU_ITEM_ValveFlag,            /*!< 阀门设定标志 */
  MENU_ITEM_ValveMode,            /*!< 阀门设定模式+24时段策略 */
  MENU_ITEM_ValveOffline,         /*!< 阀门失联模式 */
  MENU_ITEM_ValveAntifreeze,      /*!< 阀门防冻模式 */
  MENU_ITEM_THREE_END,            /*!< 三级菜单结束 */

  MENU_ITEM_Max                   /*!< 菜单结束 */
} MENU_ITEM_t;


/* MACROS  ------------------------------------------------------------------ */

/**
 * 毫秒转换为系统时钟滴答
 */
#define MENU_MS_TO_TICKS(xTimeInMs)   pdMS_TO_TICKS(xTimeInMs)

/* CONSTANTS  --------------------------------------------------------------- */

/**
 *  最大超时 ms
 */
#define MENU_MAX_TIMEOUT              MENU_MS_TO_TICKS(120000)

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */
void MenuInit(void);
void MenuPoll(void);
void MenuSignPoll(void);
uint16_t getMenuIdleState(void);    //获取菜单空闲状态 空闲返回0 ，非0：还需要运行的时间ms

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __MENU_H */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

