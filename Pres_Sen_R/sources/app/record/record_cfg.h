/**
  ******************************************************************************
  *               Copyright(C) 2015-2025 GDKY  All Rights Reserved
  *
  * @file     record_cfg.h
  * @author   ZouZH
  * @version  V1.00
  * @date     27-Nov-2015
  * @brief    数据存储配置和定义文件.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __RECORD_CFG_H
#define __RECORD_CFG_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>

/**
 * @addtogroup Record
 * @{
 */

/**
 * @defgroup RecordApp
 * @brief 数据存储配置项
 * @{
 */


/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * 存储错误代码
 */
typedef enum
{
  REC_OK            = 0,         /*!< 执行正常 */
  REC_ERR_PARAM     = 1,         /*!< 参数错误 */
  REC_ERR_CHECK     = 2,         /*!< 校验错误 */
  REC_ERR_DRIVE     = 3,         /*!< 底层驱动错误 */
  REC_ERR_PASSWD    = 4,         /*!< 系统密码错误 */
  REC_ERR_CFG       = 5,         /*!< 配置参数错误 */
  REC_ERR_OVER      = 6,         /*!< 地址溢出 */

} RecordErr_t;


/*
*****************************************************************************
* 以下定义需要与实际使用的 RecordTBL 内存分配表对应
*****************************************************************************
*/

/**
 * 数据类型(DataType)定义
 */
typedef enum
{
  REC_DT_NONE = 0,               /*!< 无效类型 */
  REC_DT_SysParam,               /*!< 系统参数      - System Parameter */
  REC_DT_NetParam,               /*!< 网络参数      - Network Parameter */
  REC_DT_SysPasswd,              /*!< 系统密码      - System Password */
  REC_DT_Valve,                  /*!< 阀门        - Valve */

  REC_DT_HistParam,              /*!< 历史数据参数  - History Parameter */
  REC_DT_HistValve,              /*!< 阀门历史数据  - History Data of Valve */
  REC_DT_AppUpdate,              /*!< IAP升级参数   - In Application Program */

  REC_DT_MAX,                    /*!< 最大数据类型 */
} RecordDataType_t;

/**
 * 数据条项(DataItem)定义
 */
typedef enum
{
  REC_DI_NONE              = 0,  /*!< 无效条项 */
  REC_DI_Init              = 1,  /*!< 初始化信息     - Initial Information */
  REC_DI_RTime             = 2,  /*!< 实时数据       - Real Time Data */
  REC_DI_Cycle             = 3,  /*!< 周期数据       - Cycle Data */
  REC_DI_CycleBak          = 4,  /*!< 周期数据备份   - Cycle Data backup */
  REC_DI_History           = 5,  /*!< 历史数据       - History Data */
  REC_DI_Event             = 6,  /*!< 事件记录       - Event */
  REC_DI_Alarm             = 7,  /*!< 报警记录       - Alarm */
  REC_DI_Param             = 8,  /*!< 参数信息       - Parameter Information */
  REC_DI_MAX,                    /*!< 最大数据项 */

} RecordDataItem_t;

/**
 * 存储设备(Device)定义
 */
typedef enum
{
  REC_DEV_NONE        = 0,       /*!< 未指定设备 */
  REC_DEV_FM24CL64    = 1,       /*!< FM24CL64，8KB */
} RecordDevType_t;


/*
*****************************************************************************
* 以下为用户存储的数据条项定义,必须按照以下标准存储结构(即最后3字节必须为校验)
* __packed typedef struct
* {
*   uint8_t  xxx;
*   uint16_t xxx;
*   uint32_t xxx;
*
*   uint8_t  bcc;              // BCC校验
*   uint16_t crc;              // CRC校验
*
* }UserDataItem_t
*****************************************************************************
*/

/**
 *  系统参数 64Bytes
 */
typedef struct
{
  uint16_t version;              /*!< 版本号: xxxx */

  uint8_t  comAddr;              /*!< 通信地址: 1-247 */
  uint8_t  devSn[4];             /*!< 设备编号(8位BCD码) */
  uint8_t  devSnFlash[4];        /*!< 设备编号(8位BCD码),Flash内存储 */
  uint8_t  hardVer[5];           /*!< 硬件版本(10位BCD码) */
  uint8_t  softVer[4];           /*!< 软件版本(8位BCD码) */
  uint8_t  protVer[4];           /*!< 协议版本(8位BCD码) */

  uint8_t  dnCycleHour;          /*!< 下传周期(小时): 0~24 */
  uint8_t  upCycleHour;          /*!< 上传周期(小时): 0~24 */
  uint8_t  upRealMinute;         /*!< 上传实时(10*分钟): 1~144 */
  uint8_t  isHeatSeason;         /*!< 0 非采暖季 1 采暖季 */
  uint8_t  heatSeason[4];        /*!< 采暖季(开始月日-结束月日)(默认11.15~3.15) */

  uint8_t  eeErrorCnt;           /*!< EEPROM错误计数 */
  uint8_t  devType;              /*!< 设备类型: 0保留,1球阀,2电热阀,3-6档,4-通断阀,7-9档,8-10档,10-20档,11-100档,12-200档 */
  int16_t  tempCal[2];           /*!< 内部参考标准温度:1K,1.5K xx.xx℃*/
  uint16_t ptFactor[2];          /*!< PT1000系数:PT1,PT2 x.xxxx(1±0.01) */
  int16_t  tempLine[2];          /*!< 线阻补偿温度:PT1,PT2 x.xxxx(±3) */
  uint8_t  tempMinute;           /*!< 温度采集间隔(分钟): 1~240 */
  uint8_t  rfFreq;               /*!< 无线通讯频段,默认0 */
  uint8_t  rfSpeed;              /*!< 无线通讯速率,默认3 */
  uint8_t  sigThr;               /*!< 无线信号阀值: 0~31(默认20) */

  uint16_t fb1AdOpen;            /*!< 阀1反馈AD值(全开) */
  uint16_t fb1AdClose;           /*!< 阀1反馈AD值(全关) */
  uint8_t  baud;                 /*!< 通信波特率:12~96(1200~9600bps) */
  uint8_t  reserved1[6];         /*!< 保留字节 */

  uint8_t  bcc;                  /*!< BCC校验 */
  uint16_t crc;                  /*!< CRC校验 */
} SysParam_t;


/**
 *  历史参数 8Bytes
 */
typedef struct
{
  uint16_t version;              /*!< 版本号: xxxx */

  uint8_t  saveNum;              /*!< 保存数量 */
  uint8_t  saveIndex;            /*!< 写索引 */
  uint8_t  readIndex;            /*!< 读索引 */

  uint8_t  bcc;                  /*!< BCC校验 */
  uint16_t crc;                  /*!< CRC校验 */
} HistParam_t;


/**
 *  历史数据 164Bytes
 */
typedef struct
{
  uint8_t  flag;
  uint8_t  len;
  uint8_t  buff[156];

  uint8_t  reserved[3];

  uint8_t  bcc;                 /*!< BCC校验 */
  uint16_t crc;                 /*!< CRC校验 */
} HistData_t;



/**
 *  BootLoader 参数 32Bytes
 */
typedef struct
{
  uint32_t bootVer;             /*!< Boot版本号: xx.xx.xx.xx(BCD Code ) */
  uint32_t runAppVer;           /*!< 运行App版本号 */
  uint32_t updateAppVer;        /*!< 升级App版本号 */
  uint16_t updateFlag;          /*!< 升级标志: 升级成功高字节置0xAA, 低字节为0x55时
                                     由应用程序进入IAP升级,0x56为正在写入CPU Flash */
  uint8_t  reserved[15];        /*!< 保留字节 */

  uint8_t  bcc;                 /*!< BCC校验 */
  uint16_t crc;                 /*!< CRC校验 */
} BootParam_t;


/* MACROS  ------------------------------------------------------------------ */

// 设备编号
#define REC_DEV_SN                g_tSysParam.devSn

// 设备类型:0保留,1球阀,2电热阀,3智能阀6档,4智能通断阀,7智能阀9档,8智能阀10档,10智能阀20档,11智能阀100档,12智能阀200档
#define REC_DEV_TYPE_Reserved     0
#define REC_DEV_TYPE_OnOffBall    1
#define REC_DEV_TYPE_OnOffHeat    2
#define REC_DEV_TYPE_Znf6         3
#define REC_DEV_TYPE_ZnfOnOff     4
#define REC_DEV_TYPE_Znf9         7
#define REC_DEV_TYPE_Znf10        8
#define REC_DEV_TYPE_Ldf9         9
#define REC_DEV_TYPE_Znf20        10
#define REC_DEV_TYPE_Znf100       11
#define REC_DEV_TYPE_Znf200       12
#define REC_DEV_TYPE              g_tSysParam.devType


// 阀体类型:[0] 通断单向阀  [1] 通断双向阀  [2] 通断电热阀 [3] 时间调节型
//          [4] 6开度调节阀 [5] 9开度调节阀 [6] 模拟调节阀 [7]步进电机 [8]直流电机

#define REC_VALVE_TYPE_OnOff1     0x00
#define REC_VALVE_TYPE_OnOff2     0x01
#define REC_VALVE_TYPE_OnOffHeat  0x02
#define REC_VALVE_TYPE_TimeAdj    0x03
#define REC_VALVE_TYPE_PhyAdj6    0x04
#define REC_VALVE_TYPE_PhyAdj9    0x05
#define REC_VALVE_TYPE_PctAdj     0x06
#define REC_VALVE_TYPE_StepAdj    0x07
#define REC_VALVE_TYPE_DcAdj      0x08
#define REC_VALVE_TYPE_NONE       0x99
#define REC_VALVE_TYPE            g_tSysParam.softVer[1]


// 设备功能:  Bit[0]是否通断控制器(0智能阀,1 通断控制器) Bit[1]是否有锁闭阀(0无,1 有)
#define REC_VALVE_FUNC            g_tSysParam.softVer[2]
#define REC_VALVE_FUNC_GFP        BIT_READ(REC_VALVE_FUNC, BIT(0))
#define REC_VALVE_FUNC_VALVE2     BIT_READ(REC_VALVE_FUNC, BIT(1))
#define REC_VALVE_FUNC_IS_ACT     BIT_READ(REC_VALVE_FUNC, BIT(2))

// 通信波特率
#define REC_BAUD                  (g_tSysParam.baud * 100)

// 是否采暖期
#define REC_IsHeat                g_tSysParam.isHeatSeason


// 周期上传间隔(小时)
// 集抄间隔
#define REC_DnCycle               g_tSysParam.dnCycleHour

// 0 关闭周期上传
#define REC_UpCycle               g_tSysParam.upCycleHour

/// 实时上传间隔(10分钟)
// 0 关闭实时上传
#define REC_UpReal                g_tSysParam.upRealMinute


/* CONSTANTS  --------------------------------------------------------------- */

/**
 * 密码定义
 */
#define APP_PWD_0                 0x13  // 0x0FEE
#define APP_PWD_1                 0x93  // 0x0FEF

/**
 * 参数版本
 */
#define RECORD_SYSPARAM_VER       0x2323
#define RECORD_NETPARAM_VER       0x2323
#define RECORD_HISTPARAM_VER      0x2323

#define RECORD_VALVEPARAM_VER     0x2323
#define RECORD_VALVERTIME_VER     0x2323
#define RECORD_VALVECYCLE_VER     0x2323


/**
 * 存储运行配置参数项
 */
#define RECORD_DATA_BAK_NBR       20 /* 数据备份数量 */

#define RECORD_HW_MAX_NBR         3  /* 最多设备数, 0-255 */

#define RECORD_ERR_RETRY_CNT      2  /* 存储发生错误重试次数 */

#define RECORD_HIST_MAX_NUM       24 /* 历史数据存储最大数量 */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __RECORD_CFG_H */

/**
 * @}
 */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

