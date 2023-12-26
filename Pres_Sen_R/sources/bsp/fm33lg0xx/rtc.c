/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 ZouZH  All Rights Reserved
  *
  * @file     rtc.h
  * @author   ZouZH
  * @version  V1.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx RTC Service.
  ******************************************************************************
  */
#define LOG_TAG    "rtc"

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "rtc.h"
#include "datetime.h"


/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

// Debug info output
#define RTC_TRACE log_i

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */

/**
 * @brief  Initialize RTC Service
 * @param  None
 * @retval 0 Success.
 */
uint8_t rtcInit(void)
{
  FL_RTCA_InitTypeDef FL_RTCA_InitStruct = {0};

  FL_CDIF_EnableVAOToCPU(CDIF);
  FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);
  FL_RTCA_DeInit(RTCA);
  FL_RTCA_Enable(RTCA);

  // BCD Time
  FL_RTCA_InitStruct.year   = 0x22;
  FL_RTCA_InitStruct.month  = 0x01;
  FL_RTCA_InitStruct.day    = 0x01;
  FL_RTCA_InitStruct.week   = 0x00;
  FL_RTCA_InitStruct.hour   = 0x00;
  FL_RTCA_InitStruct.minute = 0x00;
  FL_RTCA_InitStruct.second = 0x00;
  FL_RTCA_Init(RTCA, &FL_RTCA_InitStruct);

#ifdef RTC_FOUT_ENABLE
  {
    FL_GPIO_InitTypeDef GPIO_InitStruct;
		
		FL_GPIO_StructInit(&GPIO_InitStruct);

    FL_RTCA_SetTimeMarkOutput(RTCA, FL_RTCA_TIME_MARK_SEC);

    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    FL_GPIO_SetFOUT0(GPIO, FL_GPIO_FOUT0_SELECT_RTCTM);
  }
#endif /* RTC_FOUT_ENABLE */

  return 0;
}

/**
 * @brief  设置时间
 * @param[in] ptm: Data and Time struct(@ref DateTime_t)
 * @retval 0、成功
 */
uint8_t rtcSetTime(const DateTime_t *ptm)
{
  uint8_t i = 0;
  FL_RTCA_InitTypeDef RTC_TimeStructWr;
  FL_RTCA_InitTypeDef RTC_TimeStructRd;

  RTC_TimeStructWr.year = dec2bcd(ptm->year % 100);
  RTC_TimeStructWr.month = dec2bcd(ptm->month);
  RTC_TimeStructWr.day = dec2bcd(ptm->day);
  RTC_TimeStructWr.week = TimeGetWeekFromDate(ptm->year, ptm->month, ptm->day);

  RTC_TimeStructWr.hour = dec2bcd(ptm->hour);
  RTC_TimeStructWr.minute = dec2bcd(ptm->minute);
  RTC_TimeStructWr.second = dec2bcd(ptm->second);

  do
  {
    // 设置RTC
    FL_RTCA_ConfigTime(RTCA, &RTC_TimeStructWr);

    // 读时间
    FL_RTCA_GetTime(RTCA, &RTC_TimeStructRd);

    if (!lib_memcmp(&RTC_TimeStructRd, &RTC_TimeStructWr, sizeof(FL_RTCA_InitTypeDef)))
      break;
  } while (++i < 3);

  if (i < 3)
    return 0;
  else
    return 1;
}

/**
 * @brief  Get the RTC date and time.
 * @param  Time out
 * @retval 0 Success.
 */
uint8_t rtcGetTime(DateTime_t *ptime)
{
  uint8_t i = 0;
  FL_RTCA_InitTypeDef RTC_TimeStructRd1;
  FL_RTCA_InitTypeDef RTC_TimeStructRd2;

  if (NULL == ptime)
  {
//    RTC_TRACE("RTC get time param error!");
    return 1;
  }

  do
  {
    // 读时间
    FL_RTCA_GetTime(RTCA, &RTC_TimeStructRd1);
    FL_RTCA_GetTime(RTCA, &RTC_TimeStructRd2);

    if (!lib_memcmp(&RTC_TimeStructRd1, &RTC_TimeStructRd2, sizeof(FL_RTCA_InitTypeDef)))
      break;
  } while (++i < 3);


  if (i < 3)
  {
    ptime->year = 2000 + bcd2dec(RTC_TimeStructRd1.year);
    ptime->month = bcd2dec(RTC_TimeStructRd1.month);
    ptime->day = bcd2dec(RTC_TimeStructRd1.day);
    ptime->wday = bcd2dec(RTC_TimeStructRd1.week);

    ptime->hour = bcd2dec(RTC_TimeStructRd1.hour);
    ptime->minute = bcd2dec(RTC_TimeStructRd1.minute);
    ptime->second = bcd2dec(RTC_TimeStructRd1.second);
    ptime->ms = 0;
  }

  if (i < 3)
    return 0;
  else
    return 1;
}

/**
 * @brief  设置闹钟
 * @param  Time input hour(00-23) and minute(00-59)
 * @retval 0 Success.
 */
uint8_t rtcSetAlarm(uint8_t hour, uint8_t minute)
{
  return 0;
}


/**
 * @brief  设置 rtc 唤醒
 * @param  Time in sec >= 2, 0 Disable
 * @retval 0 Success.
 */
uint8_t rtcSetWakeup(uint16_t seconds)
{
  if (seconds && (seconds < 2))
    seconds = 2;

  return 0;
}

/**
 * @brief  Check XTLF is running
 * @param  None
 * @retval 0 Stop, 1 Running.
 */
uint8_t rtcIsXTLFRun(void)
{
  return 1;
}

