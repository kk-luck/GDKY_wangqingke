/**
  ******************************************************************************
  *               Copyright(C) 2016-2026 GDKY All Rights Reserved
  *
  * @file     record_app.h
  * @author   ZouZH
  * @version  V1.00
  * @date     16-June-2016
  * @brief    数据存储应用程序
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __RECORD_APP_H
#define __RECORD_APP_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
#include "record.h"

/**
 * @addtogroup Record
 * @{
 */

/**
 * @addtogroup RecordApp
 * @{
 */

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

extern SysParam_t  g_tSysParam;
extern HistParam_t g_tHistParam;


/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
RecordErr_t RecordApp_Init(void);
void        RecordApp_Poll(void);

RecordErr_t RecordApp_EraseAll(void);

RecordErr_t RecordApp_SaveSysParam(void);
RecordErr_t RecordApp_ReadSysParam(void);
RecordErr_t RecordApp_SaveHistParam(void);
RecordErr_t RecordApp_ReadHistParam(void);

RecordErr_t RecordApp_SetSaveSysParam(void);
RecordErr_t RecordApp_SetSaveValve(RecordDataItem_t REC_DI_x);

RecordErr_t RecordApp_SaveValve(RecordDataItem_t REC_DI_x);
RecordErr_t RecordApp_ReadValvePtr(RecordDataItem_t REC_DI_x, void **ppvBuf);

RecordErr_t RecordApp_SaveHistData(const HistData_t *ptData);
RecordErr_t RecordApp_ReadHistData(HistData_t *ptData);
RecordErr_t RecordApp_SendHistDataOk(const HistData_t *ptData);


void RecordApp_SysParamDeInit(void);
void RecordApp_HistParamDeInit(void);

void RecordApp_ValveParamDeInit(void);
void RecordApp_ValveRTimeDeInit(void);
void RecordApp_ValveCycleDeInit(void);

uint8_t RecordApp_SetDevType(uint8_t devType);

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __RECORD_APP_H */

/**
 * @}
 */

/**
 * @}
 */

/***************************** END OF FILE *************************************/

