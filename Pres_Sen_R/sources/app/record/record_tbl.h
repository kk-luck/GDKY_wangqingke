/**
  ******************************************************************************
  *               Copyright(C) 2015-2025 GDKY  All Rights Reserved
  *
  * @file     record_tbl.h
  * @author   ZouZH
  * @version  V1.01
  * @date     07-Dec-2015
  * @brief    存储分配表,包含数据类型，设备类型，地址范围等定义.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __RECORD_TBL_H
#define __RECORD_TBL_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include "record_cfg.h"

/**
 * @addtogroup Record
 * @{
 */

/**
 * @addtogroup RecordTable
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * Record Table
 */
typedef struct
{
  RecordDataType_t  dataType;
  RecordDataItem_t  dataItem;
  RecordDevType_t   devType;
  uint8_t           devAddr;
  uint32_t          dataStartAddr;
  uint32_t          dataItemSize;
  uint32_t          dataTotalSize;
  uint32_t          devTotalSize;

} RecordTBL_t;

/* MACROS  ------------------------------------------------------------------ */

/* 定义各个数据类型占用空间总大小 */

/* 系统参数 */
#define REC_Size_SysParam         (2 * sizeof(SysParam_t))

/* Bootloader参数 */
#define REC_Size_BootParam        (2 * sizeof(BootParam_t))

/* 阀门参数+实时+周期 */
#define REC_Size_Valve            (RECORD_DATA_BAK_NBR * (sizeof(Valve_Param_t) + sizeof(Valve_RTime_t) + sizeof(Valve_Cycle_t)))

/* 历史参数 */
#define REC_Size_HistParam        (2 * sizeof(HistParam_t))

/* 历史数据 */
#define REC_Size_HistData         (RECORD_HIST_MAX_NUM * sizeof(HistData_t))


/* 定义各个数据类型存储起始地址 */

/* FM24CL64 U11=000 */
#define REC_Addr_BootParam        0x00000000
#define REC_Addr_SysParam         (REC_Addr_BootParam + REC_Size_BootParam)
#define REC_Addr_Valve            (REC_Addr_SysParam + REC_Size_SysParam)
//#define REC_Addr_HistParam        (REC_Addr_Valve + REC_Size_Valve)
//#define REC_Addr_HistData         (REC_Addr_HistParam + REC_Size_HistParam)


/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

extern const RecordTBL_t RecordTBL[];

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __RECORD_TBL_H */

/**
 * @}
 */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

