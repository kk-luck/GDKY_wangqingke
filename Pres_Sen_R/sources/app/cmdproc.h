/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     cmdproc.h
  * @author   ZouZH
  * @version  V1.00
  * @date     16-May-2019
  * @brief    command process routine for Valve.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __CMDPROC_H
#define __CMDPROC_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
//#include "valve.h"
#include "combus.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/**
 * @brief 运行错误代码
 */
typedef enum
{
  CMD_OK = 0x00,
  CMD_ERR_Param,
  CMD_ERR_Receive,
  CMD_ERR_Command,

  CMD_ERR_Malloc,
  CMD_ERR_Record,
  CMD_ERR_Send,
  CMD_ERR_Busy,
  CMD_ERR_Timeout,
  CMD_ERR_Run,
  CMD_ERR_QFull,

} CMD_ERROR_t;


/**
 * @brief 通信端口
 */
typedef enum
{
  CMD_PORT_RS485 = 0,
  CMD_PORT_BLE,
  CMD_PORT_DEBUG,
  CMD_PORT_LORA,
  CMD_PORT_NUM
} CMD_PORT_t;


/**
 * 功能码定义
 */
typedef enum
{
  // IAP升级指令
  CMD_IAP_UPDATE                          = (0x3C12),    /* !<IAP 升级指令 */
  CMD_IAP_UP_DEV_INFO                     = (0x3C15),    /* !<IAP后第一次启动时上传 */

  // 校时命令
  CMD_SET_TIME_B                          = (0xAAAD),    /*!< 广播校时 */
  CMD_SET_TIME                            = (0xBDBD),    /*!< 校时 */
  CMD_GET_TIME                            = (0xBCA3),    /*!< 读时间 */
  CMD_SET_HEATSEASON                      = (0xDBCD),    /*!< 设置采暖季 */

  // 读取数据命令
  CMD_GET_NB_INFO                         = (0x94A3),    /*!< 读取NB模块网络信息 */
  CMD_GET_DEV_INFO                        = (0x51AC),    /*!< 读取设备信息 */

  CMD_GET_CYCLE_DATA1                     = (0xAAAA),    /*!< 周期数据1(接通,断开时间) */
  CMD_GET_CYCLE_DATA2                     = (0x51CA),    /*!< 周期数据2(阀门开度+防冻+灯具) */
  CMD_GET_CYCLE_LORA                      = (0x5931),    /*!< 周期数据(Lora通讯,简化数据内容) */
  CMD_GET_CYCLE_LORA_ACK                  = (0x5917),    /*!< 周期数据(Lora通讯,简化数据内容),回复指令5917 */
  CMD_UP_LORA                             = (0x593A),    /*!< 上传数据(Lora通讯,主动上传) */

  CMD_GET_RTIME_DATA                      = (0xDBAD),    /*!< 实时数据 */

  // 设置命令
  CMD_GET_DEV_SN                          = (0xBBBB),    /*!< 读取表号 */
  CMD_SET_DEV_SN                          = (0xCCBC),    /*!< 设置表号 */
  CMD_SET_DEV_TYPE                        = (0x41BA),    /*!< 设置设备类型 */

  CMD_SET_HM_TEMP_B                       = (0xACCD),    /*!< 广播楼表供回水温度 */
  CMD_SET_HM_TEMP                         = (0xDCAB),    /*!< 单播楼表供回水温度 */
  CMD_SET_STATION_BASE_TEMP               = (0xAAA3),    /*!< 广播发送机组回水温度+基准温度 */


  CMD_SET_LOCK                            = (0xCCCC),    /*!< 锁闭阀门 */
  CMD_SET_OPEN                            = (0xCCBD),    /*!< 开启阀门 */
  CMD_SET_FORCE_VALVE_STATUS              = (0xCCAD),    /*!< 强制开启/关闭 */

  CMD_SET_OPEN_SETVALVE                   = (0x4325),    /*!< 设置阀门设定开度 */
  CMD_GET_OPEN_SETVALVE                   = (0x519A),    /*!< 读取阀门设定开度 */
  CMD_SET_OPEN_SETVALVE_LIMIT             = (0x41A3),    /*!< 设置阀门设定开度上下限 */
  CMD_GET_OPEN_SETVALVE_LIMIT             = (0x51A5),    /*!< 读取阀门设定开度上下限 */

  CMD_SET_ON_OFF_LOCK_TIME                = (0xBABC),    /*!< 设置开启,断开,锁闭时间 */
  CMD_SET_ALLOC_HEAT                      = (0xDBCC),    /*!< 下发分摊热量 */
  CMD_SET_USR_SYS_ENERGY                  = (0x9D3A),    /*!< 设置用户和系统行为节能量 */

  CMD_SET_FLOOR_REPEATER                  = (0xDBCB),    /*!< 设置楼层中继 */
  CMD_SET_ROOM_SETTEMP                    = (0xDBAB),    /*!< 设定室温控制器的设温 */
  CMD_SET_FORCE_ROOM_TEMP                 = (0xCCAA),    /*!< 强制设定室温(上限) */
  CMD_SET_ROOM_SETTEMP_LIMIT              = (0xDCAC),    /*!< 设定室温控制器的设温下、上限 */

  CMD_SET_ROOM_TEMP_CORRECT               = (0x41A5),    /*!< 设置室温偏移修正值 */
  CMD_GET_ROOM_TEMP_CORRECT               = (0x51B1),    /*!< 读取室温偏移修正值 */

  CMD_SET_ROOM_TEMP_BASE                  = (0x5273),    /*!< 设置室温基准值 */
  CMD_GET_ROOM_TEMP_BASE                  = (0x571A),    /*!< 读取室温基准值 */

  CMD_SET_REMOTETEMP_PARAM                = (0x5371),    /*!< 设定远程设温下、上限,设温   */
  CMD_GET_REMOTETEMP_PARAM                = (0x569B),    /*!< 读取远程设温下、上限,设温 */

  CMD_SET_24H_CTRL                        = (0x41AD),    /*!< 设置时段控制策略 */
  CMD_GET_24H_CTRL                        = (0x51BA),    /*!< 读取时段控制策略 */

  CMD_SET_FREEZE_MODE                     = (0x41BC),    /*!< 设置防冻策略 */
  CMD_GET_FREEZE_MODE                     = (0x51AD),    /*!< 读取防冻策略 */

  CMD_SET_PWROFF_VALVE_STATUS             = (0x5637),    /*!< 设置断电阀门状态 */
  CMD_GET_PWROFF_VALVE_STATUS             = (0x564A),    /*!< 读取断电阀门状态 */
} CMD_GDKY_t;



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
/**
 * 运行程序对象
 */
typedef struct
{
#define CMD_BUF_LEN   256

  CMD_ERROR_t   error;                 /*!< 最后错误 */
  QueueHandle_t dnQueue;               /*!< 下传队列 */
  QueueHandle_t upQueue;               /*!< 上传队列 */

  uint8_t       txBuff[CMD_BUF_LEN];   /*!< 发送缓存 */
} CmdRunObj_t;


/**
 * 命令缓存
 */
typedef struct
{
  uint8_t  port;                       /*!< 数据端口: 0-UART, 1-RS485, 2-MBUS, 3-NB-IoT */
  uint8_t  flag;                       /*!< 数据标志: 0下传回复,1实时数据缓存,2周期数据缓存发送 */
  uint16_t len;                        /*!< 数据长度 */
  uint8_t *pbuf;                       /*!< 数据指针 */
} CmdQBuff_t;


/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/**
 * 最大缓存命令条数
 */
#define CMD_RX_Q_MAX_NUM             (10)


/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
CMD_ERROR_t CmdInit(void);
CMD_ERROR_t CmdRxPoll(void);
uint8_t     CmdRxIsEmpty(void);

// 保存接收数据
CMD_ERROR_t CmdRxLora(void *pdat, uint8_t datlen);

// GDKY
CMD_ERROR_t CmdWrRxGdky(CMD_GDKY_t CMD_x, CMD_PORT_t CMD_PORT_x, uint8_t type);
CMD_ERROR_t CmdWrRxModbus_Up(CMD_PORT_t CMD_PORT_x, uint8_t type);


/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __CMDPROC_H */

/***************************** END OF FILE *************************************/


