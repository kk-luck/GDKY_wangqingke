/**
  ******************************************************************************
  *               Copyright(C) 2016-2026 GDKY All Rights Reserved
  *
  * @file     record_app.c
  * @author   ZouZH
  * @version  V1.00
  * @date     16-June-2016
  * @brief    数据存储应用程序
  ******************************************************************************
  * @history
  */
#define LOG_TAG    "REC"


/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
//#include "valve.h"
//#include "lora_app.h"


/**
 * @addtogroup Record
 * @{
 */

/**
 * @defgroup RecordApp
 * @brief 数据存储应用程序
 * @{
 */


/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/**
 * 系统参数,网络参数,历史参数
 */
SysParam_t  g_tSysParam;
HistParam_t g_tHistParam;

Valve_Param_t g_tValveParam;
Valve_RTime_t g_tValveRTime;
Valve_Cycle_t g_tValveCycle;


/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* 临时变量 */
static SysParam_t  s_tSysParam;
static HistParam_t s_tHistParam;

static Valve_Param_t s_tValveParam;
static Valve_RTime_t s_tValveRTime;
static Valve_Cycle_t s_tValveCycle;

/* 数据保存标识(空闲时保存)
 * BIT0 系统参数
 * BIT1 阀门参数
 * BIT2 阀门实时
 * BIT3 阀门周期
 */
static uint16_t s_usSaveFlag;

/* LOCAL FUNCTIONS ------------------------------------------------------------ */

/**
 * @brief  存储初始化
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_Init(void)
{
  RecordErr_t retErr;
  RecordErr_t retRet = REC_OK;
  uint32_t counter;

  // 存储初始化
  retErr = Record_Init();

  if (REC_OK != retErr)
    retRet = retErr;

  // 读系统参数
  retErr = RecordApp_ReadSysParam();

  if ((retErr != REC_OK) || (g_tSysParam.version != RECORD_SYSPARAM_VER))
  {
    log_d("Record SysParam Reset! ret=%d", retErr);
    RecordApp_SysParamDeInit();
    retErr = RecordApp_SaveSysParam();
  }
  else
  {
    if (lib_memcmp(g_tSysParam.softVer, APP_SOFT_VER, sizeof(APP_SOFT_VER)))
    {
      lib_memcpy(g_tSysParam.softVer, APP_SOFT_VER, sizeof(APP_SOFT_VER));
      retErr = RecordApp_SaveSysParam();
    }
  }

  // 设备号
  if (isbcds(APP_DEV_ADDR, sizeof(APP_DEV_ADDR)))
    lib_memcpy(REC_DEV_SN, APP_DEV_ADDR, sizeof(APP_DEV_ADDR));

  if (REC_OK != retErr)
    retRet = retErr;

  // 读阀门参数
  counter = 0;

  for (uint8_t i = 0; i < RECORD_DATA_BAK_NBR; i++)
  {
    if (REC_OK != (retErr = Record_Read(REC_DT_Valve, REC_DI_Param, i, &g_tValveParam)))
      continue;

    if (g_tValveParam.counter > counter)
      counter = g_tValveParam.counter;
  }

  retErr = Record_Read(REC_DT_Valve, REC_DI_Param, counter % RECORD_DATA_BAK_NBR, &g_tValveParam);
  log_d("Record ValveParam Read! ret=%d, counter=%d", retErr, counter);

  if ((retErr != REC_OK) || (g_tValveParam.version != RECORD_VALVEPARAM_VER))
  {
    if ((g_tSysParam.eeErrorCnt < UINT8_MAX) && (retErr != REC_OK) && (retErr != REC_ERR_CHECK))
      g_tSysParam.eeErrorCnt += 1;

    RecordApp_ValveParamDeInit();

    g_tValveParam.counter = counter;
    retErr = RecordApp_SaveValve(REC_DI_Param);
  }

  if (REC_OK != retErr)
    retRet = retErr;

  // 读阀门实时
  counter = 0;

  for (uint8_t i = 0; i < RECORD_DATA_BAK_NBR; i++)
  {
    if (REC_OK != (retErr = Record_Read(REC_DT_Valve, REC_DI_RTime, i, &g_tValveRTime)))
      continue;

    if (g_tValveRTime.counter > counter)
      counter = g_tValveRTime.counter;
  }

  retErr = Record_Read(REC_DT_Valve, REC_DI_RTime, counter % RECORD_DATA_BAK_NBR, &g_tValveRTime);
  log_d("Record ValveRTime Read! ret=%d, counter=%d", retErr, counter);

  if ((retErr != REC_OK) || (g_tValveRTime.version != RECORD_VALVERTIME_VER))
  {
    if ((g_tSysParam.eeErrorCnt < UINT8_MAX) && (retErr != REC_OK) && (retErr != REC_ERR_CHECK))
      g_tSysParam.eeErrorCnt += 1;

    RecordApp_ValveRTimeDeInit();
    g_tValveRTime.counter = counter;
    retErr = RecordApp_SaveValve(REC_DI_RTime);
  }

  if (REC_OK != retErr)
    retRet = retErr;

  // 读阀门周期
  counter = 0;

  for (uint8_t i = 0; i < RECORD_DATA_BAK_NBR; i++)
  {
    if (REC_OK != (retErr = Record_Read(REC_DT_Valve, REC_DI_Cycle, i, &g_tValveCycle)))
      continue;

    if (g_tValveCycle.counter > counter)
      counter = g_tValveCycle.counter;
  }

  retErr = Record_Read(REC_DT_Valve, REC_DI_Cycle, counter % RECORD_DATA_BAK_NBR, &g_tValveCycle);
  log_d("Record ValveCycle Read! ret=%d, counter=%d", retErr, counter);

  if ((retErr != REC_OK) || (g_tValveCycle.version != RECORD_VALVECYCLE_VER))
  {
    if ((g_tSysParam.eeErrorCnt < UINT8_MAX) && (retErr != REC_OK) && (retErr != REC_ERR_CHECK))
      g_tSysParam.eeErrorCnt += 1;

    RecordApp_ValveCycleDeInit();

    g_tValveCycle.counter = counter;
    retErr = RecordApp_SaveValve(REC_DI_Cycle);
  }

  if (REC_OK != retErr)
    retRet = retErr;

  log_d("Record Init=%d", retRet);

  return retRet;
}

/**
 * @brief  存储轮询
 * @param  无
 * @retval 无
 */
void RecordApp_Poll(void)
{
  // Lora
#if (APP_EXMOD_TYPE_LORA == APP_EXMOD_TYPE)

  if (loraAppGetState() == LORA_STATE_Sending)
    return;

  if (!loraAppIsTxEmpty(LORA_TX_DN_ACK))
    return;

#endif /* APP_EXMOD_TYPE */

  if (BIT_READ(s_usSaveFlag, BIT(0)))
  {
    BIT_CLEAR(s_usSaveFlag, BIT(0));
    RecordApp_SaveSysParam();
  }

  if (BIT_READ(s_usSaveFlag, BIT(1)))
  {
    BIT_CLEAR(s_usSaveFlag, BIT(1));
    RecordApp_SaveValve(REC_DI_Param);
  }

  if (BIT_READ(s_usSaveFlag, BIT(2)))
  {
    BIT_CLEAR(s_usSaveFlag, BIT(2));
    RecordApp_SaveValve(REC_DI_RTime);
  }

  if (BIT_READ(s_usSaveFlag, BIT(3)))
  {
    BIT_CLEAR(s_usSaveFlag, BIT(3));
    RecordApp_SaveValve(REC_DI_Cycle);
  }
}


/**
 * @brief  存储全部擦除
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_EraseAll(void)
{
  RecordErr_t retErr = REC_OK;
  RecordErr_t retRet = REC_OK;

  retErr = Record_Erase(REC_DT_SysParam, REC_DI_NONE);

  if (REC_OK != retErr)
    retRet = retErr;

  retErr = Record_Erase(REC_DT_Valve, REC_DI_NONE);

  if (REC_OK != retErr)
    retRet = retErr;

  return retRet;
}


/**
 * @brief  保存系统参数
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_SaveSysParam(void)
{
  RecordErr_t err = REC_ERR_PARAM;

  SYSEnterCritical();
  {
    // 设备号
    if (isbcds(APP_DEV_ADDR, sizeof(APP_DEV_ADDR)))
      lib_memcpy(REC_DEV_SN, APP_DEV_ADDR, sizeof(APP_DEV_ADDR));

    if (!lib_memcmp(&s_tSysParam, &g_tSysParam, sizeof(SysParam_t)))
      err = REC_OK;
    else
      lib_memcpy(&s_tSysParam, &g_tSysParam, sizeof(SysParam_t));
  }
  SYSExitCritical();

  if (err)
  {
    err = Record_Write(REC_DT_SysParam, REC_DI_NONE, 0, &s_tSysParam);
    err = Record_Write(REC_DT_SysParam, REC_DI_NONE, 1, &s_tSysParam);
  }

  return err;
}

/**
 * @brief  读取系统参数
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_ReadSysParam(void)
{
  RecordErr_t err = REC_OK;

  err = Record_Read(REC_DT_SysParam, REC_DI_NONE, 0, &s_tSysParam);

  if (err != REC_OK)
    err = Record_Read(REC_DT_SysParam, REC_DI_NONE, 1, &s_tSysParam);

  SYSEnterCritical();
  {
    if (err == REC_OK)
      lib_memcpy(&g_tSysParam, &s_tSysParam, sizeof(SysParam_t));

    lib_memcpy(REC_DEV_SN, APP_DEV_ADDR, sizeof(APP_DEV_ADDR));
  }
  SYSExitCritical();

  return err;
}

/**
 * @brief  保存历史参数
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_SaveHistParam(void)
{
  RecordErr_t err = REC_ERR_PARAM;

  SYSEnterCritical();
  {
    if (!lib_memcmp(&s_tHistParam, &g_tHistParam, sizeof(HistParam_t)))
      err = REC_OK;
    else
      lib_memcpy(&s_tHistParam, &g_tHistParam, sizeof(HistParam_t));
  }
  SYSExitCritical();

  if (err)
  {
    err = Record_Write(REC_DT_HistParam, REC_DI_NONE, 0, &s_tHistParam);
    err = Record_Write(REC_DT_HistParam, REC_DI_NONE, 1, &s_tHistParam);
  }

  return err;
}

/**
 * @brief  读取历史参数
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_ReadHistParam(void)
{
  RecordErr_t err = REC_OK;

  err = Record_Read(REC_DT_HistParam, REC_DI_NONE, 0, &s_tHistParam);

  if (err != REC_OK)
    err = Record_Read(REC_DT_HistParam, REC_DI_NONE, 1, &s_tHistParam);

  SYSEnterCritical();
  {
    if (err == REC_OK)
      lib_memcpy(&g_tHistParam, &s_tHistParam, sizeof(HistParam_t));
  }
  SYSExitCritical();

  return err;
}

/**
 * @brief  设置保存系统参数标识
 * @param  无
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_SetSaveSysParam(void)
{
  SYSEnterCritical();
  {
    BIT_SET(s_usSaveFlag, BIT(0));
  }
  SYSExitCritical();

  return REC_OK;
}


/**
 * @brief  设置保存阀门数据项标识
 * @param  REC_DI_x 数据条项(0-REC_DI_MAX)
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_SetSaveValve(RecordDataItem_t REC_DI_x)
{
  if ((REC_DI_Param != REC_DI_x) && (REC_DI_RTime != REC_DI_x) && (REC_DI_Cycle != REC_DI_x))
    return REC_ERR_PARAM;

  SYSEnterCritical();
  {
    if (REC_DI_Param == REC_DI_x)
      BIT_SET(s_usSaveFlag, BIT(1));
    else if (REC_DI_RTime == REC_DI_x)
      BIT_SET(s_usSaveFlag, BIT(2));
    else
      BIT_SET(s_usSaveFlag, BIT(3));
  }
  SYSExitCritical();

  return REC_OK;
}


/**
 * @brief  保存阀门数据项
 * @param  REC_DI_x 数据条项(0-REC_DI_MAX)
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_SaveValve(RecordDataItem_t REC_DI_x)
{
  RecordErr_t err = REC_OK;
  void *pv = NULL;
  uint8_t need_save = 0;
  uint32_t counter = 0;

  if ((REC_DI_Param != REC_DI_x) && (REC_DI_RTime != REC_DI_x) && (REC_DI_Cycle != REC_DI_x))
    return REC_ERR_PARAM;

  SYSEnterCritical();
  {
    if (REC_DI_Param == REC_DI_x)
    {
      pv = &s_tValveParam;

      if (lib_memcmp(&s_tValveParam, &g_tValveParam, sizeof(Valve_Param_t)))
      {
        g_tValveParam.counter += 1;
        lib_memcpy(&s_tValveParam, &g_tValveParam, sizeof(Valve_Param_t));

        need_save = 1;
        counter = g_tValveParam.counter;
      }
    }
    else if (REC_DI_RTime == REC_DI_x)
    {
      pv = &s_tValveRTime;

      if (lib_memcmp(&s_tValveRTime, &g_tValveRTime, sizeof(Valve_RTime_t)))
      {
        g_tValveRTime.counter += 1;
        lib_memcpy(&s_tValveRTime, &g_tValveRTime, sizeof(Valve_RTime_t));

        need_save = 1;
        counter = g_tValveRTime.counter;
      }
    }
    else
    {
      pv = &s_tValveCycle;

      if (lib_memcmp(&s_tValveCycle, &g_tValveCycle, sizeof(Valve_Cycle_t)))
      {
        g_tValveCycle.counter += 1;
        lib_memcpy(&s_tValveCycle, &g_tValveCycle, sizeof(Valve_Cycle_t));

        need_save = 1;
        counter = s_tValveCycle.counter;
      }
    }
  }
  SYSExitCritical();

  if (need_save)
  {
    err = Record_Write(REC_DT_Valve, REC_DI_x, counter % RECORD_DATA_BAK_NBR, pv);

    if (err && g_tSysParam.eeErrorCnt < UINT8_MAX)
      g_tSysParam.eeErrorCnt += 1;
  }


  return err;
}


/**
 * @brief  读取阀门数据项指针
 * @param  REC_DI_x 数据条项(0-REC_DI_MAX)
 * @param  pvBuf    数据项缓存指针
 * @retval 0 成功，其他失败
 */
RecordErr_t RecordApp_ReadValvePtr(RecordDataItem_t REC_DI_x, void **ppvBuf)
{
  void *pv = NULL;

  if ((REC_DI_Param != REC_DI_x) && (REC_DI_RTime != REC_DI_x) && (REC_DI_Cycle != REC_DI_x))
    return REC_ERR_PARAM;

  if (REC_DI_Param == REC_DI_x)
    pv = &g_tValveParam;
  else if (REC_DI_RTime == REC_DI_x)
    pv = &g_tValveRTime;
  else if (REC_DI_Cycle == REC_DI_x)
    pv = &g_tValveCycle;
  else
    pv = NULL;

  if (ppvBuf)
    *ppvBuf = pv;

  return REC_OK;
}

/**
 * @brief      Write data to history
 * @param[in]  ptData: Pointer to data to copy memory from
 * @return     0 Success
 */
RecordErr_t RecordApp_SaveHistData(const HistData_t *ptData)
{
  if (NULL == ptData)
    return REC_ERR_PARAM;

  if (g_tHistParam.saveIndex >= RECORD_HIST_MAX_NUM)
  {
    RecordApp_HistParamDeInit();
    RecordApp_SaveHistParam();
  }

  if (Record_Write(REC_DT_HistValve, REC_DI_Cycle, g_tHistParam.saveIndex, ptData))
    return REC_ERR_DRIVE;

  g_tHistParam.readIndex = g_tHistParam.saveIndex;

  g_tHistParam.saveIndex += 1;

  if (g_tHistParam.saveIndex >= RECORD_HIST_MAX_NUM)
    g_tHistParam.saveIndex = 0;

  if (g_tHistParam.saveNum < RECORD_HIST_MAX_NUM)
    g_tHistParam.saveNum += 1;

  RecordApp_SaveHistParam();

  return REC_OK;
}

/**
 * @brief      Read data to history
 * @param[in]  ptData: Pointer to data to copy memory to
 * @return     0 Success
 */
RecordErr_t RecordApp_ReadHistData(HistData_t *ptData)
{
  RecordErr_t ret = REC_OK;

  if ((NULL == ptData) || !g_tHistParam.saveNum)
    return REC_ERR_PARAM;

  if (g_tHistParam.readIndex >= RECORD_HIST_MAX_NUM)
  {
    RecordApp_HistParamDeInit();
    RecordApp_SaveHistParam();
    return REC_ERR_OVER;
  }

  do
  {
    ret = Record_Read(REC_DT_HistValve, REC_DI_Cycle, g_tHistParam.readIndex, ptData);

    if (REC_OK != ret)
    {
      g_tHistParam.saveNum -= 1;

      if (g_tHistParam.saveNum)
      {
        if (g_tHistParam.readIndex > 0)
          g_tHistParam.readIndex -= 1;
        else
          g_tHistParam.readIndex = RECORD_HIST_MAX_NUM - 1;
      }
      else
      {
        g_tHistParam.readIndex = g_tHistParam.saveIndex;
      }
    }
  } while (REC_OK != ret && g_tHistParam.saveNum);

  return ret;
}


/**
 * @brief      Read data to history
 * @param[in]  ptData: Pointer to data to copy memory to
 * @return     0 Success
 */
RecordErr_t RecordApp_SendHistDataOk(const HistData_t *ptData)
{
  if ((NULL == ptData) || !g_tHistParam.saveNum)
    return REC_ERR_PARAM;

  if (g_tHistParam.readIndex >= RECORD_HIST_MAX_NUM)
  {
    RecordApp_HistParamDeInit();
    RecordApp_SaveHistParam();
    return REC_ERR_OVER;
  }

  g_tHistParam.saveNum -= 1;

  if (g_tHistParam.saveNum)
  {
    if (g_tHistParam.readIndex > 0)
      g_tHistParam.readIndex -= 1;
    else
      g_tHistParam.readIndex = RECORD_HIST_MAX_NUM - 1;
  }
  else
  {
    g_tHistParam.readIndex = g_tHistParam.saveIndex;
  }

  RecordApp_SaveHistParam();

  return REC_OK;
}


/**
 * @brief  设置系统参数默认值
 * @param  None
 * @retval None
 */
void RecordApp_SysParamDeInit(void)
{
  SYSEnterCritical();

  lib_memset(&g_tSysParam, 0x00, sizeof(SysParam_t));

  g_tSysParam.version = RECORD_SYSPARAM_VER;

  g_tSysParam.comAddr = 1;
  lib_memcpy(g_tSysParam.devSn, APP_DEV_ADDR, 4);
  lib_memcpy(g_tSysParam.devSnFlash, APP_DEV_ADDR, 4);
  lib_memcpy(g_tSysParam.softVer, APP_SOFT_VER, 4);

  g_tSysParam.hardVer[0]  = 0x00;
  g_tSysParam.hardVer[1]  = 0x02;
  g_tSysParam.hardVer[2]  = 0x02;
  g_tSysParam.hardVer[3]  = 0x00;
  g_tSysParam.hardVer[4]  = VALVE_DN_DEF;

  g_tSysParam.protVer[0]  = 0x00;
  g_tSysParam.protVer[1]  = 0x00;
  g_tSysParam.protVer[2]  = 0x00;
  g_tSysParam.protVer[3]  = 0x01;

  g_tSysParam.dnCycleHour = APP_DN_CYCLE_DEF;
  g_tSysParam.upCycleHour = APP_UP_CYCLE_DEF;
  g_tSysParam.upRealMinute = APP_UP_REAL_DEF;

  g_tSysParam.heatSeason[0]   = bcd2dec(UINT32_BREAK(APP_HEATSEASON_DEF, 3));
  g_tSysParam.heatSeason[1]   = bcd2dec(UINT32_BREAK(APP_HEATSEASON_DEF, 2));
  g_tSysParam.heatSeason[2]   = bcd2dec(UINT32_BREAK(APP_HEATSEASON_DEF, 1));
  g_tSysParam.heatSeason[3]   = bcd2dec(UINT32_BREAK(APP_HEATSEASON_DEF, 0));

  switch (REC_VALVE_TYPE)
  {
    case REC_VALVE_TYPE_OnOff1:
    case REC_VALVE_TYPE_OnOff2:
      if (REC_VALVE_FUNC_GFP)
        g_tSysParam.devType = REC_DEV_TYPE_OnOffBall;
      else
        g_tSysParam.devType = REC_DEV_TYPE_ZnfOnOff;
      break;
    case REC_VALVE_TYPE_OnOffHeat:
      g_tSysParam.devType = REC_DEV_TYPE_OnOffHeat;
      break;

    case REC_VALVE_TYPE_TimeAdj:
    case REC_VALVE_TYPE_PhyAdj6:
      g_tSysParam.devType = REC_DEV_TYPE_Znf6;
      break;

    case REC_VALVE_TYPE_PhyAdj9:
      g_tSysParam.devType = REC_DEV_TYPE_Znf9;
      break;

    case REC_VALVE_TYPE_PctAdj:
    case REC_VALVE_TYPE_StepAdj:
    case REC_VALVE_TYPE_DcAdj:
      g_tSysParam.devType = REC_DEV_TYPE_DEF;
      break;

    default:
      g_tSysParam.devType = REC_DEV_TYPE_Reserved;
      break;
  }

  if (REC_VALVE_FUNC_IS_ACT)
    g_tSysParam.devType = REC_DEV_TYPE_Znf200;

  if (APP_TEMP_TYPE == APP_TEMP_TYPE_RCD)
  {
    g_tSysParam.tempCal[0] = 0;
    g_tSysParam.tempCal[1] = 10130;
  }
  else
  {
    g_tSysParam.tempCal[0] = 0;
    g_tSysParam.tempCal[1] = 13047;
  }

  g_tSysParam.ptFactor[0] = 10000;
  g_tSysParam.ptFactor[1] = 10000;

  g_tSysParam.tempMinute = APP_TEMP_CYCLE_DEF;

  g_tSysParam.rfFreq = APP_LORA_FREQ_DEF;
  g_tSysParam.rfSpeed = APP_LORA_SPEED_DEF;

  g_tSysParam.baud = REC_BAUD_DEF / 100;

  SYSExitCritical();
}


/**
 * @brief  设置历史参数默认值
 * @param  None
 * @retval None
 */
void RecordApp_HistParamDeInit(void)
{
  SYSEnterCritical();

  lib_memset(&g_tHistParam, 0x00, sizeof(HistParam_t));
  g_tHistParam.version = RECORD_HISTPARAM_VER;
  SYSExitCritical();
}



/**
 * @brief  设置阀门参数默认值
 * @param  None
 * @retval None
 */
void RecordApp_ValveParamDeInit(void)
{
  SYSEnterCritical();

  lib_memset(&g_tValveParam, 0x00, sizeof(Valve_Param_t));

  g_tValveParam.version = RECORD_VALVEPARAM_VER;

  g_tValveParam.valveMode.bit.mode = 0;
  g_tValveParam.valveMode.bit.tc_off = 3;
  g_tValveParam.valveMode.bit.com_off = 4;
  g_tValveParam.valveMode.bit.run_tm = 0;

  g_tValveParam.freezeMode.bit.onTime = 2;
  g_tValveParam.freezeMode.bit.onCycle = 1;

  switch (REC_VALVE_TYPE)
  {
    case REC_VALVE_TYPE_OnOff1:
    case REC_VALVE_TYPE_OnOff2:
    case REC_VALVE_TYPE_OnOffHeat:
      g_tValveParam.valveAdjNum = 9;
      break;

    case REC_VALVE_TYPE_TimeAdj:
    case REC_VALVE_TYPE_PhyAdj6:
      g_tValveParam.valveAdjNum = 6;
      break;

    case REC_VALVE_TYPE_PhyAdj9:
      g_tValveParam.valveAdjNum = 9;
      break;

    case REC_VALVE_TYPE_PctAdj:
    case REC_VALVE_TYPE_StepAdj:
    case REC_VALVE_TYPE_DcAdj:
      if (REC_DEV_TYPE_Znf200 == REC_DEV_TYPE)
        g_tValveParam.valveAdjNum = 200;
      else if (REC_DEV_TYPE_Znf100 == REC_DEV_TYPE)
        g_tValveParam.valveAdjNum = 100;
      else if (REC_DEV_TYPE_Znf20 == REC_DEV_TYPE)
        g_tValveParam.valveAdjNum = 20;
      else if (REC_DEV_TYPE_Znf10 == REC_DEV_TYPE)
        g_tValveParam.valveAdjNum = 10;
      else
        g_tValveParam.valveAdjNum = 9;
      break;

    default:
      g_tValveParam.valveAdjNum = 9;
      break;
  }

  g_tValveParam.valveUpLimit = g_tValveParam.valveAdjNum;
  g_tValveParam.valveDnLimit = 0;
  g_tValveParam.valveSet = g_tValveParam.valveAdjNum;

  g_tValveParam.tcFlag.bit.uninstall = 1;
  g_tValveParam.tcFlag.bit.invalid   = 1;
  g_tValveParam.tcFlag.bit.remote  = 0;
  g_tValveParam.tcCom.bit.mode  = 0;
  g_tValveParam.tcCom.bit.floor = 0;

  g_tValveParam.roomSetUpLimit  = 2600;
  g_tValveParam.roomSetDnLimit  = 1200;
  g_tValveParam.roomSet = 3000;
  g_tValveParam.roomSetRemoteUp = 2600;
  g_tValveParam.roomSetRemoteDn = 1200;
  g_tValveParam.roomSetRemote   = g_tValveParam.roomSetRemoteUp;
  g_tValveParam.usrRoomBase     = 2200;
  g_tValveParam.sysRoomBase     = 2200;
  g_tValveParam.roomCorrect     = 0;

  lib_memset(g_tValveParam.hourValveSet, g_tValveParam.valveUpLimit, sizeof(g_tValveParam.hourValveSet));

  g_tValveParam.hmInTemp    = 70;
  g_tValveParam.hmOutTemp   = 55;

  g_tValveParam.valveRotateDay = 15;

  g_tValveParam.stationIdx = 0;
  g_tValveParam.stationOutTemp = 0;

  g_tValveParam.usrSetTempMins = 1;
  g_tValveParam.sysSetTempMins = 1;
  g_tValveParam.usrCloseMins = 1;
  g_tValveParam.sysCloseMins = 0;
  g_tValveParam.energyCalcDay = 1;
  g_tValveParam.energyMinTemp = 1800;
  g_tValveParam.energyEnable = 0;

  SYSExitCritical();
}


/**
 * @brief  设置阀门实时数据默认值
 * @param  None
 * @retval None
 */
void RecordApp_ValveRTimeDeInit(void)
{
  SYSEnterCritical();
  lib_memset(&g_tValveRTime, 0x00, sizeof(Valve_RTime_t));

  g_tValveRTime.version = RECORD_VALVERTIME_VER;

  SYSExitCritical();
}

/**
 * @brief  设置阀门周期数据默认值
 * @param  None
 * @retval None
 */
void RecordApp_ValveCycleDeInit(void)
{
  DateTime_t tm =
  {
    .year = 2019,
    .month = 1,
    .day = 1,
    .hour = 0,
    .minute = 0,
    .second = 0,
    .ms = 0
  };

  SYSEnterCritical();
  lib_memset(&g_tValveCycle, 0, sizeof(Valve_Cycle_t));

  g_tValveCycle.version = RECORD_VALVECYCLE_VER;
  g_tValveCycle.time = TimeDateToUnixSecond(&tm);

  g_tValveCycle.valve1Fb = g_tValveParam.valveUpLimit;
  g_tValveCycle.correctOnRatio = 1;
  g_tValveCycle.totalOnRatio = 1;

  g_tValveCycle.inTemp = 9999;
  g_tValveCycle.outTemp = 9999;
  g_tValveCycle.inTempAvgCycle = 9999;
  g_tValveCycle.outTempAvgCycle = 9999;

  g_tValveCycle.roomTemp[0] = 1000;
  g_tValveCycle.roomTemp[1] = 1000;
  g_tValveCycle.roomTemp[2] = 1000;
  g_tValveCycle.roomTempAvgCycle = 1000;
  g_tValveCycle.roomTempAvgTotal = 1000;

  if (g_tValveParam.tcFlag.bit.remote)
    g_tValveCycle.roomSetTempAvg = g_tValveParam.roomSetRemote;
  else
    g_tValveCycle.roomSetTempAvg = g_tValveParam.roomSet;

  SYSExitCritical();
}


/**
 * @brief  设置设备类型
 * @param  devType 设备类型
 * @retval 0成功,1失败
 */
uint8_t RecordApp_SetDevType(uint8_t devType)
{
  uint8_t ret = 0;

  switch (devType)
  {
    // 11-100档
    case REC_DEV_TYPE_Znf100:
      g_tSysParam.devType = devType;
      g_tValveParam.valveAdjNum = 100;
      g_tValveParam.valveSet = 100;
      g_tValveParam.valveUpLimit = 100;
      g_tValveParam.valveDnLimit = 0;
      break;

    // 12-200档
    case REC_DEV_TYPE_Znf200:
      g_tSysParam.devType = devType;
      g_tValveParam.valveAdjNum = 200;
      g_tValveParam.valveSet = 200;
      g_tValveParam.valveUpLimit = 200;
      g_tValveParam.valveDnLimit = 0;
      break;

    default:
      ret = 1;
      break;
  }

  if (!ret)
  {
    if (RecordApp_SaveValve(REC_DI_Param) || RecordApp_SaveSysParam())
      ret = 2;
  }

  return ret;
}


/**
 * @}
 */
