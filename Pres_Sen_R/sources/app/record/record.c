/**
  ******************************************************************************
  *               Copyright(C) 2016-2026 GDKY All Rights Reserved
  *
  * @file     record.c
  * @author   ZouZH
  * @version  V1.03
  * @date     17-March-2016
  * @brief    线性数据存储应用层接口.
  ******************************************************************************
  * @history
  * 2015-11-04 V1.00 ZouZH 初次创建
  * 2015-12-07 V1.01 ZouZH 修正读取和写入数据项地址计算错误
  * 2015-12-24 V1.02 ZouZH 修正读取和写入数据项地址计算错误
  * 2016-03-17 V1.03 ZouZH 增加Record_Erase()
  */
#define LOG_TAG "REC"


/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "record.h"
#include "record_hw.h"


/**
 * @defgroup Record
 * @brief 线性数据存储模块
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


/**
 * @brief  存储初始化
 *
 * @param  无
 *
 * @retval 0 成功，其他失败
 */
RecordErr_t Record_Init(void)
{
  RecordErr_t retErr = REC_OK;

  // 硬件初始化
  retErr = Record_HWInit(NULL);

  return retErr;
}


/**
 * @brief  读取数据
 *
 * @param  REC_DT_x 数据类型(1-REC_DT_MAX)
 * @param  REC_DI_x 数据条项(0-REC_DI_MAX)
 * @param  itemIdx  数据条项索引(0-65535)
 * @param  pvBuf    数据项缓存
 *
 * @retval 0 成功，其他失败
 */
RecordErr_t Record_Read(RecordDataType_t REC_DT_x, RecordDataItem_t REC_DI_x, uint16_t itemIdx, void *pvBuf)
{
  RecordErr_t recErr = REC_OK;

  uint8_t  i = 0;
  uint8_t  tbl_idx = 0;
  uint32_t addr = 0;

  ASSERT(REC_DT_x && (REC_DT_x < REC_DT_MAX) && (REC_DI_x < REC_DI_MAX) && pvBuf);

  // 搜索存储分配表中对应数据项
  while (RecordTBL[i].dataType && RecordTBL[i].devType)
  {
    if ((REC_DT_x == RecordTBL[i].dataType) && (REC_DI_x == RecordTBL[i].dataItem))
      break;
    else
      i += 1;
  }

  // 未搜索到有效条项
  if (!RecordTBL[i].dataType || !RecordTBL[i].devType)
  {
    log_w("Read no valid item was searched!,DT=%d DI=%d", REC_DT_x, REC_DI_x);
    return REC_ERR_PARAM;
  }

  // 计算存储地址
  tbl_idx = i;
  addr = 0;

  // 从同数据类型的最后一个数据项开始计算
  while (RecordTBL[tbl_idx].dataType == RecordTBL[i + 1].dataType)
    i += 1;

  do
  {
    addr += RecordTBL[i].dataItemSize;

    if (i > 0)
      i -= 1;
    else
      break;
  } while (RecordTBL[tbl_idx].dataType == RecordTBL[i].dataType);

  addr *= itemIdx;
  addr += RecordTBL[tbl_idx].dataStartAddr;

  // 判断地址范围
  if ((addr + RecordTBL[tbl_idx].dataItemSize > (RecordTBL[tbl_idx].dataStartAddr + RecordTBL[tbl_idx].dataTotalSize)) || \
      (addr + RecordTBL[tbl_idx].dataItemSize > RecordTBL[tbl_idx].devTotalSize))
  {
    log_w("Read address out of range!,DT=%d DI=%d IDX=%d", REC_DT_x, REC_DI_x, itemIdx);
    return REC_ERR_OVER;
  }

  for (i = 0; i < RECORD_ERR_RETRY_CNT; i++)
  {
    if (recErr != REC_OK)
      recErr = Record_HWInit(&RecordTBL[tbl_idx]);

    if (Record_HWRead(&RecordTBL[tbl_idx], addr, pvBuf, RecordTBL[tbl_idx].dataItemSize) != REC_OK)
    {
      log_w("Read driver error!,DT=%d DI=%d IDX=%d", REC_DT_x, REC_DI_x, itemIdx);
      recErr = REC_ERR_DRIVE;
      continue;
    }

    if (chk_crc16_MB((const uint8_t *)pvBuf,  RecordTBL[tbl_idx].dataItemSize))
    {
      log_w("Read CRC error!,DT=%d DI=%d IDX=%d", REC_DT_x, REC_DI_x, itemIdx);
      recErr = REC_ERR_CHECK;
      break;
    }

    if (chk_bcc((const uint8_t *)pvBuf, RecordTBL[tbl_idx].dataItemSize - 2))
    {
      log_w("Read BCC error!,DT=%d DI=%d IDX=%d", REC_DT_x, REC_DI_x, itemIdx);
      recErr = REC_ERR_CHECK;
      break;
    }
    else
      i = 0x55;
  }

  return recErr;
}


/**
 * @brief  写入数据
 *
 * @param  REC_DT_x 数据类型(1-REC_DT_MAX)
 * @param  REC_DI_x 数据条项(0-REC_DI_MAX)
 * @param  itemIdx  数据条项索引(0-65535)
 * @param  pvBuf    数据项缓存
 *
 * @retval 0 成功，其他失败
 */
RecordErr_t Record_Write(RecordDataType_t REC_DT_x, RecordDataItem_t REC_DI_x, uint16_t itemIdx, const void *pvBuf)
{
  RecordErr_t recErr = REC_OK;

  uint8_t  i = 0;
  uint8_t  tbl_idx = 0;
  uint32_t addr = 0;

  ASSERT(REC_DT_x && (REC_DT_x < REC_DT_MAX) && (REC_DI_x < REC_DI_MAX) && pvBuf);

  // 搜索存储分配表中对应数据项
  while (RecordTBL[i].dataType && RecordTBL[i].devType)
  {
    if ((REC_DT_x == RecordTBL[i].dataType) && (REC_DI_x == RecordTBL[i].dataItem))
      break;
    else
      i += 1;
  }

  // 未搜索到有效条项
  if (!RecordTBL[i].dataType || !RecordTBL[i].devType)
  {
    log_w("Write no valid item was searched!,DT=%d DI=%d", REC_DT_x, REC_DI_x);
    return REC_ERR_PARAM;
  }

  // 计算存储地址
  tbl_idx = i;
  addr = 0;

  // 从同数据类型的最后一个数据项开始计算
  while (RecordTBL[tbl_idx].dataType == RecordTBL[i + 1].dataType)
    i += 1;

  do
  {
    addr += RecordTBL[i].dataItemSize;

    if (i > 0)
      i -= 1;
    else
      break;
  } while (RecordTBL[tbl_idx].dataType == RecordTBL[i].dataType);

  addr *= itemIdx;
  addr += RecordTBL[tbl_idx].dataStartAddr;

  // 判断地址范围
  if ((addr + RecordTBL[tbl_idx].dataItemSize > (RecordTBL[tbl_idx].dataStartAddr + RecordTBL[tbl_idx].dataTotalSize)) || \
      (addr + RecordTBL[tbl_idx].dataItemSize > RecordTBL[tbl_idx].devTotalSize))
  {
    log_w("Write address out of range!,DT=%d DI=%d IDX=%d", REC_DT_x, REC_DI_x, itemIdx);
    return REC_ERR_OVER;
  }

  *(uint8_t *)(((uint8_t *)pvBuf) + RecordTBL[tbl_idx].dataItemSize - 3) =
    chk_bcc((const uint8_t *)pvBuf, RecordTBL[tbl_idx].dataItemSize - 3);

  *(uint16_t *)(((uint8_t *)pvBuf) + RecordTBL[tbl_idx].dataItemSize - 2) =
    chk_crc16_MB((const uint8_t *)pvBuf, RecordTBL[tbl_idx].dataItemSize - 2);

  for (i = 0; i < RECORD_ERR_RETRY_CNT; i++)
  {
    if (recErr != REC_OK)
      recErr = Record_HWInit(&RecordTBL[tbl_idx]);

    if (Record_HWWrite(&RecordTBL[tbl_idx], addr, pvBuf, RecordTBL[tbl_idx].dataItemSize) != REC_OK)
    {
      log_w("Write driver error!,DT=%d DI=%d IDX=%d", REC_DT_x, REC_DI_x, itemIdx);
      recErr = REC_ERR_DRIVE;
      continue;
    }
    else
      i = 0x55;
  }

  return recErr;
}

/**
 * @brief  擦除数据
 *
 * @param  REC_DT_x 数据类型(1-REC_DT_MAX)
 * @param  REC_DI_x 数据条项(0-REC_DI_MAX)
 *
 * @retval 0 成功，其他失败
 */
RecordErr_t Record_Erase(RecordDataType_t REC_DT_x, RecordDataItem_t REC_DI_x)
{
  RecordErr_t recErr = REC_OK;

  uint8_t  i = 0;
  uint8_t  tbl_idx = 0;
  uint32_t addr = 0;

  ASSERT(REC_DT_x && (REC_DT_x < REC_DT_MAX) && (REC_DI_x < REC_DI_MAX));

  // 搜索存储分配表中对应数据项
  while (RecordTBL[i].dataType && RecordTBL[i].devType)
  {
    if (((REC_DT_x == RecordTBL[i].dataType) && (REC_DI_x == RecordTBL[i].dataItem)) \
        || ((REC_DT_x == RecordTBL[i].dataType) && (REC_DI_x == REC_DI_NONE)))
      break;
    else
      i += 1;
  }

  // 未搜索到有效条项
  if (!RecordTBL[i].dataType || !RecordTBL[i].devType)
  {
    log_w("Erase no valid item was searched!,DT=%d DI=%d", REC_DT_x, REC_DI_x);
    return REC_ERR_PARAM;
  }

  // 计算存储地址
  tbl_idx = i;
  addr = RecordTBL[tbl_idx].dataStartAddr;

  // 判断地址范围
  if ((addr + RecordTBL[tbl_idx].dataItemSize > (RecordTBL[tbl_idx].dataStartAddr + RecordTBL[tbl_idx].dataTotalSize)) \
      || (addr + RecordTBL[tbl_idx].dataItemSize > RecordTBL[tbl_idx].devTotalSize))
  {
    log_w("Erase address out of range!,DT=%d DI=%d", REC_DT_x, REC_DI_x);
    return REC_ERR_OVER;
  }

  // 删除整个dataType
  if (REC_DI_x == REC_DI_NONE)
  {
    for (i = 0; i < RECORD_ERR_RETRY_CNT; i++)
    {
      if (recErr != REC_OK)
        recErr = Record_HWInit(&RecordTBL[tbl_idx]);

      if (Record_HWErase(&RecordTBL[tbl_idx], addr, RecordTBL[tbl_idx].dataTotalSize) != REC_OK)
      {
        log_w("Erase driver error!,DT=%d DI=%d", REC_DT_x, REC_DI_x);
        recErr = REC_ERR_DRIVE;
        continue;
      }
      else
      {
        i = 0x55;
      }
    }
  }
  else
  {
    while (REC_DT_x == RecordTBL[tbl_idx].dataType)
    {
      if (REC_DI_x == RecordTBL[tbl_idx].dataItem)
      {
        for (i = 0; i < RECORD_ERR_RETRY_CNT; i++)
        {
          if (recErr != REC_OK)
            recErr = Record_HWInit(&RecordTBL[tbl_idx]);

          if (Record_HWErase(&RecordTBL[tbl_idx], addr, RecordTBL[tbl_idx].dataItemSize) != REC_OK)
          {
            log_w("Erase driver error!,DT=%d DI=%d", REC_DT_x, REC_DI_x);
            recErr = REC_ERR_DRIVE;
            continue;
          }
          else
          {
            i = 0x55;
          }
        }
      }

      addr += RecordTBL[tbl_idx].dataItemSize;
      tbl_idx += 1;
    }
  }

  return recErr;
}


/* ============================ 用户函数 ===================================== */

/**
 * @brief  校验系统密码
 *
 * @param  无
 *
 * @retval 无
 */
RecordErr_t Record_VerifyPasswd(void)
{
  RecordErr_t recErr = REC_OK;

  struct SysPasswdType
  {
    uint8_t  pwd[5];
    uint8_t  bcc;
    uint16_t crc;
  } syspwd;

  ASSERT(sizeof(syspwd) == 8);

  recErr = Record_Read(REC_DT_SysPasswd, REC_DI_NONE, 0, &syspwd);
#ifdef USE_FULL_ASSERT

  if (recErr != REC_OK)
  {
    lib_memset(&syspwd, 0x00, sizeof(syspwd));
    syspwd.pwd[0] = APP_PWD_0;
    syspwd.pwd[1] = APP_PWD_1;
    recErr = Record_Write(REC_DT_SysPasswd, REC_DI_NONE, 0, &syspwd);
  }

#endif

  if (recErr != REC_OK)
    return recErr;

  if ((syspwd.pwd[0] != APP_PWD_0) || (syspwd.pwd[1] != APP_PWD_1))
    return REC_ERR_PASSWD;

  return recErr;
}

/**
 * @brief  保存应用程序在线升级标志
 *
 * @param  无
 *
 * @retval 0 成功, 其他失败
 */
RecordErr_t Record_SaveAppUpFlag(void)
{
  RecordErr_t err = REC_OK;
  BootParam_t bootParam;

  Record_Read(REC_DT_AppUpdate, REC_DI_NONE, 0, &bootParam);

  bootParam.runAppVer = UINT32_BUILD(APP_SOFT_VER[3], APP_SOFT_VER[2], \
                                     APP_SOFT_VER[1], APP_SOFT_VER[0]);
  bootParam.updateFlag = 0x0055;

  err = Record_Write(REC_DT_AppUpdate, REC_DI_NONE, 0, &bootParam);

  return err;
}

/**
 * @brief  检查应用程序是否更新
 *
 * @param  无
 *
 * @retval 1 更新过程序，0未更新过程序
 */
uint8_t Record_IsAppUpdated(void)
{
  BootParam_t bootParam;

  if (REC_OK == Record_Read(REC_DT_AppUpdate, REC_DI_NONE, 0, &bootParam))
  {
    if (UINT16_HI(bootParam.updateFlag) == 0xAA)
    {
      bootParam.runAppVer = UINT32_BUILD(APP_SOFT_VER[3], APP_SOFT_VER[2], \
                                         APP_SOFT_VER[1], APP_SOFT_VER[0]);
      bootParam.updateFlag = 0;

      if (REC_OK == Record_Write(REC_DT_AppUpdate, REC_DI_NONE, 0, &bootParam))
        return 1;
    }
  }

  return 0;
}

/**
 * @}
 */

