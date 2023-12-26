/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     menu.c
  * @author   ZouZH
  * @version  V1.00
  * @date     28-June-2019
  * @brief    menu display.
  ******************************************************************************
  * @history
  */


/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "menu.h"

#include "valve.h"
#include "meter.h"
#include "press.h"
#include "nb_app.h"
#include "ble_app.h"
#include "led.h"
#include "lcd.h"
#include "lora_wb25.h"

/**
 * @defgroup MENU
 * @brief menu display
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * @brief Menu fsm parameter
 */
typedef struct
{
  uint8_t       dispDev;                              /*!< 0-LCD,1-LED */
  uint8_t       depth;                                /*!< 当前菜单深度 */
  MENU_ITEM_t   item;                                 /*!< 当前显示项 */
  uint8_t       pageNum;                              /*!< 当前显示项页数 */
  uint8_t       page;                                 /*!< 当前显示页 */
  char          str[17];                              /*!< 显示字符串 */
  uint8_t       blink;                                /*!< 闪烁标记(0 or 1) */
  uint32_t      blinkTime;                            /*!< 闪烁时间戳(ms) */
  uint32_t      execTime;                             /*!< 执行时间戳(ms) */
  uint32_t      pageTime;                             /*!< 页切换时间戳(ms) */
  uint32_t      menuTime;                             /*!< 菜单切换时间戳(ms) */
  uint32_t      blSec;                                /*!< 背光点亮超时(sec) */

  // LED
  uint32_t      ledPageTime;                          /*!< LED页切换时间戳(ms) */
  uint8_t       ledPageNum;                           /*!< LED显示项页数 */
  uint8_t       ledPage;                              /*!< LED当前显示页 */
  uint8_t       ledDepth;                             /*!< LED当前菜单深度 */
  uint8_t       ledStart;                             /*!< LED启动指示 */
  uint32_t      ledLastMs;                            /*!< LED闪烁计数 */
  uint8_t       led1Blink;                            /*!< LED1闪烁计数 */
  uint8_t       led2Blink;                            /*!< LED2闪烁计数 */
  uint8_t       led3Blink;                            /*!< LED3闪烁计数 */
  uint8_t       led4Blink;                            /*!< LED4闪烁计数 */

  // 阀门
  Valve_Param_t *ptParam;
  Valve_RTime_t *ptRTime;
  Valve_Cycle_t *ptCycle;
  Valve_Fsm_t *ptFsmObj;

  const NB_t *ptNBObj;
  const NB_App_t *ptNBAppObj;
} Menu_Fsm_t;

/* MACROS  ------------------------------------------------------------------ */

/* 获取系统运行时间ms */
#define MENU_GetCurTimeMs() (SYSGetRunMs() + MENU_MAX_TIMEOUT)

/* 设置延时时间 */
#define MENU_DelayMs(ms) do { curTime = MENU_GetCurTimeMs(); \
    s_tMenu.execTime = curTime - MENU_MAX_TIMEOUT + MENU_MS_TO_TICKS(ms);} while (0)

/* 设置页切换时间 */
#define MENU_SwitchPageMs(ms) do { curTime = MENU_GetCurTimeMs(); \
    s_tMenu.pageTime = curTime - MENU_MAX_TIMEOUT + MENU_MS_TO_TICKS(ms);} while (0)

#define MENU_SwitchLedPageMs(ms) do { curTime = MENU_GetCurTimeMs(); \
    s_tMenu.ledPageTime = curTime - MENU_MAX_TIMEOUT + MENU_MS_TO_TICKS(ms);} while (0)


/* CONSTANTS  --------------------------------------------------------------- */

/* 最大菜单深度 */
#define MENU_MAX_DEPTH  3

/* 最大菜单停留时间(超时回复到默认菜单) */
#define MENU_MAX_STAY_TIME  SYSMakeMs(0, 120, 0)

/* 多页切换时间ms */
#define MENU_PAGE_TIME      SYSMakeMs(0, 3, 0)

/* 背光时间(秒) */
#define MENU_BL_TIME        SYSMakeSec(0, 0, 15)

/* LED第一屏(状态) */
#define MENU_PAGE0_TIME_LED SYSMakeMs(0, 11, 0)

/* LED第二屏(开度) */
#define MENU_PAGE1_TIME_LED SYSMakeMs(0, 4, 0)


/* 指令显示码 */
static const struct
{
  uint8_t  code;
  uint16_t cmd;
} MENU_CMD_CODE[47] =

{
  // gdky
  {0x01, CMD_IAP_UPDATE},
  {0x02, CMD_IAP_UP_DEV_INFO},
  {0x03, CMD_SET_TIME_B},
  {0x04, CMD_SET_TIME},
  {0x05, CMD_GET_TIME},
  {0x06, CMD_SET_HEATSEASON},
  {0x07, CMD_GET_NB_INFO},
  {0x08, CMD_GET_DEV_INFO},
  {0x09, CMD_GET_CYCLE_DATA1},
  {0x0A, CMD_GET_CYCLE_DATA2},
  {0x0B, CMD_GET_RTIME_DATA},
  {0x0C, CMD_GET_DEV_SN},
  {0x0D, CMD_SET_DEV_SN},
  {0x0E, CMD_SET_HM_TEMP_B},
  {0x0F, CMD_SET_HM_TEMP},
  {0x10, CMD_SET_LOCK},
  {0x11, CMD_SET_OPEN},
  {0x12, CMD_SET_FORCE_VALVE_STATUS},
  {0x13, CMD_SET_OPEN_SETVALVE},
  {0x14, CMD_GET_OPEN_SETVALVE},
  {0x15, CMD_SET_OPEN_SETVALVE_LIMIT},
  {0x16, CMD_GET_OPEN_SETVALVE_LIMIT},
  {0x17, CMD_SET_ON_OFF_LOCK_TIME},
  {0x18, CMD_SET_ALLOC_HEAT},
  {0x19, CMD_SET_FLOOR_REPEATER},
  {0x1A, CMD_SET_ROOM_SETTEMP},
  {0x1B, CMD_SET_FORCE_ROOM_TEMP},
  {0x1C, CMD_SET_ROOM_SETTEMP_LIMIT},
  {0x1D, CMD_SET_ROOM_TEMP_CORRECT},
  {0x1E, CMD_GET_ROOM_TEMP_CORRECT},
  {0x1F, CMD_SET_REMOTETEMP_PARAM},
  {0x20, CMD_GET_REMOTETEMP_PARAM},
  {0x21, CMD_SET_24H_CTRL},
  {0x22, CMD_GET_24H_CTRL},
  {0x23, CMD_SET_FREEZE_MODE},
  {0x24, CMD_GET_FREEZE_MODE},
  {0x25, CMD_SET_PWROFF_VALVE_STATUS},
  {0x26, CMD_GET_PWROFF_VALVE_STATUS},
  {0x27, CMD_SET_DEV_TYPE},
  {0x28, CMD_GET_LORA_DATA_IN},
  {0x29, CMD_UPLOAD_LORA_DATA},

  // Modbus
  {0x40, 0x03},
  {0x41, 0x06},
  {0x42, 0x10},

  // AT
  {0xFF, 0xFFFF},

  // 温控器
  {0xFE, 0xFFFE},

  {0x00, 0x0000},
};


/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* 状态机对象运行参数 */
static Menu_Fsm_t  s_tMenu;

/* LOCAL FUNCTIONS ---------------------------------------------------------- */
void MenuLedPoll(void);
void MenuLcdPoll(void);

/**
 * @brief  初始化
 * @param  None
 * @retval None
 */
void MenuInit(void)
{
  lib_memset(&s_tMenu, 0x00, sizeof(s_tMenu));

  RecordApp_ReadValvePtr(REC_DI_Param, (void **)&s_tMenu.ptParam);
  RecordApp_ReadValvePtr(REC_DI_RTime, (void **)&s_tMenu.ptRTime);
  RecordApp_ReadValvePtr(REC_DI_Cycle, (void **)&s_tMenu.ptCycle);
  ValveGetFsmObj(&s_tMenu.ptFsmObj);

  nbGetObj(&s_tMenu.ptNBObj);
  nbAppGetObj(&s_tMenu.ptNBAppObj);

//  if (LCD_Init())
//  {
//    s_tMenu.dispDev = 1;

//    led_init();
//    led_set(LED_ALL, LED_MODE_ON);
//  }
//  else
//  {
    s_tMenu.dispDev = 0;

    LCD_Init();
    LCD_Clear(1);

    // 背光
    s_tMenu.blSec = SYSGetRunSec() + MENU_BL_TIME;
    LCD_Backlight(0);
//  }
}

/**
 * @brief  液晶菜单显示轮询
 * @param  None
 * @retval None
 */
void MenuPoll(void)
{
  if (s_tMenu.dispDev)
    MenuLedPoll();
  else
    MenuLcdPoll();
}

/**
 * @brief  LED显示BCD码
 * @param  None
 * @retval None
 */
void MenuLedBcd(uint8_t bcd)
{
  led_set(LED_ALL, LED_MODE_OFF);

  // 1
  if (BIT_READ(bcd, BIT(0)))
    led_set(LED_1, LED_MODE_ON);

  // 2
  if (BIT_READ(bcd, BIT(1)))
    led_set(LED_2, LED_MODE_ON);

  // 4
  if (BIT_READ(bcd, BIT(2)))
    led_set(LED_3, LED_MODE_ON);

  // 8
  if (BIT_READ(bcd, BIT(3)))
    led_set(LED_4, LED_MODE_ON);
}


/**
 * @brief  LED显示轮询
 * @param  None
 * @retval None
 */
void MenuLedPoll(void)
{
/*  uint32_t curTime;
  key_event_t keyEvt = {KEY_RELEASE, KEY_NONE};

  curTime = MENU_GetCurTimeMs();

  // 读按键状态
  if (!KeyGet(&keyEvt) && ((KEY_LONG == keyEvt.state) || (KEY_VALID == keyEvt.state)))
  {
    if (KEY_LONG == keyEvt.state)
    {
      // 校正阀门
      s_tMenu.ptFsmObj->flag.bit.valve1_rotat_auto_pre = 1;
    }

    // 立即采集
    MeterSync();

    // 蓝牙唤醒
    if (bleAppGetRunState() >= BLE_STATE_Suspend)
      bleAppSetRunState(BLE_STATE_Idle);
  }

  // 翻页超时
  if ((curTime - s_tMenu.ledPageTime > MENU_MAX_TIMEOUT) && (s_tMenu.ledStart >= 99))
  {
    s_tMenu.led1Blink = 0;
    s_tMenu.led2Blink = 0;
    s_tMenu.led3Blink = 0;
    s_tMenu.led4Blink = 0;
    led_set(LED_ALL, LED_MODE_OFF);

    s_tMenu.ledPage = !s_tMenu.ledPage;
    s_tMenu.ledPageNum = 0;

    if (!s_tMenu.ledPage)
      MENU_SwitchLedPageMs(MENU_PAGE0_TIME_LED);
    else
      MENU_SwitchLedPageMs(MENU_PAGE1_TIME_LED);
  }

  do
  {
    uint8_t step100Ms = (SYSGetRunMs() - s_tMenu.ledLastMs) / 100;

    if (!step100Ms)
      break;

    if (step100Ms > 10)
      step100Ms = 10;

    s_tMenu.ledLastMs = SYSGetRunMs();

    s_tMenu.led1Blink += step100Ms;
    s_tMenu.led2Blink += step100Ms;
    s_tMenu.led3Blink += step100Ms;
    s_tMenu.led4Blink += step100Ms;

    if ((s_tMenu.ptFsmObj->comCmd == CMD_GET_DEV_SN) && (s_tMenu.ledStart < 99))
      s_tMenu.ledStart = 99;

    // 开机指示
    switch (s_tMenu.ledStart)
    {
      case 0:
        if (s_tMenu.led1Blink > 30)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_OFF);
        }

        return;

      case 1:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_OFF);
          led_set(LED_4, LED_MODE_ON);
        }

        return;

      case 2:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_OFF);
          led_set(LED_3, LED_MODE_ON);
        }

        return;

      case 3:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_OFF);
          led_set(LED_2, LED_MODE_ON);
        }

        return;

      case 4:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_OFF);
          led_set(LED_1, LED_MODE_ON);
        }

        return;

      case 5:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 6:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_HI(REC_DEV_SN[0]));
        }

        return;

      case 7:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 8:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_LO(REC_DEV_SN[0]));
        }

        return;

      case 9:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 10:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_HI(REC_DEV_SN[1]));
        }

        return;

      case 11:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 12:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_LO(REC_DEV_SN[1]));
        }

        return;

      case 13:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 14:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_HI(REC_DEV_SN[2]));
        }

        return;

      case 15:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 16:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_LO(REC_DEV_SN[2]));
        }

        return;

      case 17:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 18:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_HI(REC_DEV_SN[3]));
        }

        return;

      case 19:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 20:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_LO(REC_DEV_SN[3]));
        }

        return;

      case 21:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 22:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          MenuLedBcd(UINT8_LO(g_tSysParam.softVer[3]));
        }

        return;

      case 23:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;
          led_set(LED_ALL, LED_MODE_ON);
        }

        return;

      case 24:
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.ledStart += 1;

          led_set(LED_ALL, LED_MODE_OFF);

          if (BIT_READ(AppGetError(), BIT(0)))
            led_set(LED_1, LED_MODE_ON);

          if (BIT_READ(AppGetError(), BIT(1)))
            led_set(LED_2, LED_MODE_ON);

          if (BIT_READ(AppGetError(), BIT(2)))
            led_set(LED_3, LED_MODE_ON);

          if (BIT_READ(AppGetError(), BIT(3)))
            led_set(LED_4, LED_MODE_ON);
        }

        return;

      case 25:
        if (s_tMenu.led1Blink > 20)
        {
          s_tMenu.led1Blink = 0;
          s_tMenu.led2Blink = 0;
          s_tMenu.led3Blink = 0;
          s_tMenu.led4Blink = 0;
          led_set(LED_ALL, LED_MODE_OFF);

          s_tMenu.ledStart = 99;
          s_tMenu.ledPage = 0;
          s_tMenu.ledPageNum = 0;
          MENU_SwitchLedPageMs(MENU_PAGE0_TIME_LED);
        }

        return;

      case 99:
        break;

      default:
        s_tMenu.ledStart = 99;
        break;
    }

    // LED1,NB-IoT/运行状态
    // LED2,通信状态
    // LED3,阀门动作状态
    // LED4,阀门设定状态
    if (!s_tMenu.ledPage)
    {
      if (!REC_CycleUp || (REC_EXMOD_TYPE_NB != REC_EXMOD_TYPE))
      {
        if (s_tMenu.led1Blink > 10)
        {
          led_set(LED_1, LED_MODE_TOGGLE);
          s_tMenu.led1Blink = 0;
        }
      }
      else
      {
        // LED1,NB-IoT运行状态
        switch (nbAppGetRunState())
        {
          // 常灭
          case NB_State_Suspend:
            led_set(LED_1, LED_MODE_OFF);
            s_tMenu.led1Blink = 0;
            break;

          // 常亮
          case NB_State_Error:
            led_set(LED_1, LED_MODE_ON);
            s_tMenu.led1Blink = 0;
            break;

          case NB_State_Idle:
          case NB_State_Restart:
          case NB_State_Startup:
          case NB_State_WaitStart:
            break;

          // 4S 闪烁
          case NB_State_ModInit:
            if (s_tMenu.led1Blink > 40)
            {
              led_set(LED_1, LED_MODE_TOGGLE);
              s_tMenu.led1Blink = 0;
            }

            break;

          // 3S 闪烁
          case NB_State_NetConfig:
            if (s_tMenu.led1Blink > 30)
            {
              led_set(LED_1, LED_MODE_TOGGLE);
              s_tMenu.led1Blink = 0;
            }

            break;

          // 2S 闪烁
          case NB_State_NetAttach:
            if (s_tMenu.led1Blink > 20)
            {
              led_set(LED_1, LED_MODE_TOGGLE);
              s_tMenu.led1Blink = 0;
            }

            break;

          // 1S 闪烁
          case NB_State_WaitAttach:
            if (s_tMenu.led1Blink > 10)
            {
              led_set(LED_1, LED_MODE_TOGGLE);
              s_tMenu.led1Blink = 0;
            }

            break;

          // 500ms 闪烁
          case NB_State_Running:
            if (s_tMenu.led1Blink > 5)
            {
              led_set(LED_1, LED_MODE_TOGGLE);
              s_tMenu.led1Blink = 0;
            }

            break;

          // 100ms 闪烁
          case NB_State_SendHist:
          case NB_State_Sending:
            led_set(LED_1, LED_MODE_TOGGLE);
            s_tMenu.led1Blink = 0;
            break;

          default:
            led_set(LED_1, LED_MODE_OFF);
            s_tMenu.led1Blink = 0;
            break;
        }
      }

      // LED2,
      // 收到指令,闪一下
      if (s_tMenu.ptFsmObj->comCmd || s_tMenu.ptFsmObj->comCmdTc)
      {
        s_tMenu.ptFsmObj->comCmd = 0;
        s_tMenu.ptFsmObj->comCmdTc = 0;

        led_set(LED_2, LED_MODE_TOGGLE);
        s_tMenu.led2Blink = 0;
      }
      // 阀门失联, 常亮
      else if (s_tMenu.ptFsmObj->flag.bit.com_offline)
      {
        led_set(LED_2, LED_MODE_ON);
        s_tMenu.led2Blink = 0;
      }
      // NB运行错误, 500ms 闪烁
      else if (nbAppGetLastError() && REC_CycleUp && (REC_EXMOD_TYPE_NB == REC_EXMOD_TYPE))
      {
        if (s_tMenu.led2Blink > 5)
        {
          led_set(LED_2, LED_MODE_TOGGLE);
          s_tMenu.led2Blink = 0;
        }
      }
#if 0
      // 外部掉电且法拉低于3.6v, 1S闪烁
      else if (PM_IsExtPwrOff() && PM_GetBatVol() < 3600)
      {
        if (s_tMenu.led2Blink > 10)
        {
          LedSet(LED_1, LED_MODE_TOGGLE);
          s_tMenu.led2Blink = 0;
        }
      }
#endif
      // 温控失联, 2S 闪烁
      else if (s_tMenu.ptCycle->errCode.bit.tc_off)
      {
        if (s_tMenu.led2Blink > 20)
        {
          led_set(LED_2, LED_MODE_TOGGLE);
          s_tMenu.led2Blink = 0;
        }
      }
      else
      {
        led_set(LED_2, LED_MODE_OFF);
        s_tMenu.led2Blink = 0;
      }

      // LED3 阀门动作状态
      if (s_tMenu.ptCycle->valveStat.bit.running1)
      {
        // 开阀 500ms 闪烁
        if (1 == s_tMenu.ptFsmObj->flag.bit.valve1_running_dir)
        {
          if (s_tMenu.led3Blink > 5)
          {
            led_set(LED_3, LED_MODE_TOGGLE);
            s_tMenu.led3Blink = 0;
          }
        }
        // 关阀 100ms 闪烁
        else if (2 == s_tMenu.ptFsmObj->flag.bit.valve1_running_dir)
        {
          led_set(LED_3, LED_MODE_TOGGLE);
          s_tMenu.led3Blink = 0;
        }
        else
        {
          led_set(LED_3, LED_MODE_OFF);
        }
      }
      else
      {
        // 故障 常亮
        if (s_tMenu.ptCycle->valveStat.bit.blocking1 || s_tMenu.ptCycle->valveStat.bit.fault1 || \
            s_tMenu.ptCycle->valveStat.bit.blocking2 || s_tMenu.ptCycle->valveStat.bit.fault2)
        {
          led_set(LED_3, LED_MODE_ON);
        }
        else
        {
          led_set(LED_3, LED_MODE_OFF);
        }

        s_tMenu.led3Blink = 0;
      }

      // LED4, 阀门1设定状态
      if (s_tMenu.ptParam->valveFlag.bit.lock)
      {
        // 锁闭, 常亮
        led_set(LED_4, LED_MODE_ON);
        s_tMenu.led4Blink = 0;
      }
      else if (2 == s_tMenu.ptParam->valveFlag.bit.force1)
      {
        // 强关, 500ms闪烁
        if (s_tMenu.led4Blink > 5)
        {
          led_set(LED_4, LED_MODE_TOGGLE);
          s_tMenu.led4Blink = 0;
        }
      }
      else if (s_tMenu.ptCycle->valveStat.bit.closed1 || !s_tMenu.ptFsmObj->valve1SetNext)
      {
        // 关阀或开度0, 1S闪烁
        if (s_tMenu.led4Blink > 10)
        {
          led_set(LED_4, LED_MODE_TOGGLE);
          s_tMenu.led4Blink = 0;
        }
      }
      else
      {
        led_set(LED_4, LED_MODE_OFF);
        s_tMenu.led4Blink = 0;
      }

      // 每个灯亮100ms
      //if (s_tMenu.led1Blink >= 1)
      //  led_set(LED_1, LED_MODE_OFF);

      //if (s_tMenu.led2Blink >= 1)
      //  led_set(LED_2, LED_MODE_OFF);

      //if (s_tMenu.led3Blink >= 1)
      //  led_set(LED_3, LED_MODE_OFF);

      //if (s_tMenu.led4Blink >= 1)
      //  led_set(LED_4, LED_MODE_OFF);

      break;
    }
    else
    {
      if (!s_tMenu.ledPageNum)
      {
        led_set(LED_ALL, LED_MODE_ON);
        s_tMenu.led1Blink = 0;
        s_tMenu.ledPageNum += 1;
      }
      else
      {
        if (s_tMenu.led1Blink > 10)
        {
          s_tMenu.led1Blink = 0;

          if (s_tMenu.ptCycle->valveStat.bit.running1)
            s_tMenu.ledDepth = 11;
          else if (s_tMenu.ptCycle->valveStat.bit.fault1)
            s_tMenu.ledDepth = 12;
          else if (s_tMenu.ptCycle->valveStat.bit.blocking1)
            s_tMenu.ledDepth = 13;
          else if (s_tMenu.ptCycle->valveStat.bit.circuit1)
            s_tMenu.ledDepth = 14;
          else
          {
            if (s_tMenu.ptParam->valveAdjNum <= 10)
              s_tMenu.ledDepth = s_tMenu.ptCycle->valve1Fb;
            else if (s_tMenu.ptParam->valveAdjNum <= 100)
              s_tMenu.ledDepth = s_tMenu.ptCycle->valve1Fb / 10;
            else
              s_tMenu.ledDepth = s_tMenu.ptCycle->valve1Fb / 20;
          }

          MenuLedBcd(s_tMenu.ledDepth);
        }
      }
    }
  } while (0);*/
}

/**
 * @brief  液晶菜单显示轮询
 * @param  None
 * @retval None
 */
void MenuLcdPoll(void)
{
  uint32_t   u32temp1,u32temp2;
  uint32_t   curTime = 0;
  key_event_t keyEvt = {KEY_RELEASE, KEY_NONE};
  static uint8_t lcdEnableState;

  curTime = MENU_GetCurTimeMs();

  //如果液晶由关闭变为打开且lora在掉线状态   则重新入网
  if((lcdEnableState == 0) && (LCD_IsEnable() != 0) && (getWb25OnlineState() != 1))
  {
    wb25Reconnection();       //LORA_wb25 重新入网
  }
  lcdEnableState = LCD_IsEnable();   //


  // 读按键状态
  if (!KeyGet(&keyEvt) && ((KEY_LONG == keyEvt.state) || (KEY_VALID == keyEvt.state)))
  {
    // 背光

    s_tMenu.blSec = SYSGetRunSec() + MENU_BL_TIME;
    LCD_Backlight(1);

    // 切换菜单深度
    if (KEY_LONG == keyEvt.state)
    {
      // 校正阀门
      if (MENU_ITEM_ValveDN == s_tMenu.item)    //阀门口径界面   长按键 自动转阀一次
      {
        s_tMenu.ptFsmObj->flag.bit.valve1_rotat_auto_pre = 1;
        s_tMenu.ptParam->valveFlag.bit.run_now = 1;
      }
      else if (MENU_ITEM_LORA_ONLINE == s_tMenu.item)    //LORA在线标志界面  长按键 重新初始化LORA
      {
        wb25Reconnection();       //LORA_wb25 重新入网
      }
      else if (MENU_ITEM_OpenTime == s_tMenu.item)    //接通时间 界面长按键，立即发送一条数据
      {
        CmdUploadMsgGDKY(CMD_UPLOAD_LORA_DATA, CMD_PORT_LORA);  //LORA主动上传一次数据
      }
      else
      {
        s_tMenu.depth = ++s_tMenu.depth % MENU_MAX_DEPTH;

        if (1 == s_tMenu.depth)
          s_tMenu.item = MENU_ITEM_DevSN;
        else if ((2 == s_tMenu.depth) && (MENU_ITEM_HeatSeason == s_tMenu.item))
          s_tMenu.item = MENU_ITEM_HM_WaterTemp;
        else
        {
          s_tMenu.depth = 0;
          s_tMenu.item = MENU_ITEM_Heat;
        }
      }

      s_tMenu.pageNum = 0;
      s_tMenu.page = 0;
      MENU_SwitchPageMs(MENU_PAGE_TIME);

      // 立即采集
      MeterSync();

      // 蓝牙唤醒
      if (bleAppGetRunState() >= BLE_STATE_Suspend)
        bleAppSetRunState(BLE_STATE_Idle);
    }
    else
    // 切换菜单显示
    {
      s_tMenu.item += 1;
      s_tMenu.pageNum = 0;
      s_tMenu.page = 0;
      MENU_SwitchPageMs(MENU_PAGE_TIME);
    }

    s_tMenu.menuTime = curTime - MENU_MAX_TIMEOUT + MENU_MS_TO_TICKS(MENU_MAX_STAY_TIME);
  }
  else
  {
    // 背光控制
    if (s_tMenu.blSec != UINT32_MAX)
    {
      if (SYSGetRunSec() > s_tMenu.blSec)
      {
        LCD_Backlight(0);
        s_tMenu.blSec = UINT32_MAX;
      }
      else
      {
        if (s_tMenu.blSec - SYSGetRunSec() > MENU_BL_TIME)
          s_tMenu.blSec = SYSGetRunSec() + MENU_BL_TIME;
      }
    }

    // 接收到命令
    if (s_tMenu.ptFsmObj->comCmd || s_tMenu.ptFsmObj->comCmdTc)
    {
      for (uint8_t i = 0; i < ARR_ITEM_NUM(MENU_CMD_CODE); i++)
      {
        if (!MENU_CMD_CODE[i].cmd && !MENU_CMD_CODE[i].code)
          break;

        if (MENU_CMD_CODE[i].cmd == s_tMenu.ptFsmObj->comCmd || MENU_CMD_CODE[i].cmd == s_tMenu.ptFsmObj->comCmdTc)
        {
          sprintf(s_tMenu.str, "%03d", MENU_CMD_CODE[i].code);
          LCD_DispString(s_tMenu.str);
          LCD_Refresh(0);

          MENU_DelayMs(1000);
          break;
        }
      }

      s_tMenu.ptFsmObj->comCmd = 0;
      s_tMenu.ptFsmObj->comCmdTc = 0;
    }

    // 等待数据项更新超时
    if ((curTime - s_tMenu.execTime < MENU_MAX_TIMEOUT))
      goto EXIT_PROC;

    // 翻页超时
    if ((curTime - s_tMenu.pageTime > MENU_MAX_TIMEOUT))
    {
      s_tMenu.page += 1;
      MENU_SwitchPageMs(MENU_PAGE_TIME);
    }

    // 自动返回待机菜单超时
    if ((curTime - s_tMenu.menuTime > MENU_MAX_TIMEOUT) && (s_tMenu.item > MENU_ITEM_Heat))
    {
      s_tMenu.item = MENU_ITEM_Heat;
      s_tMenu.depth = 0;
      s_tMenu.pageNum = 0;
      s_tMenu.page = 0;
      MENU_SwitchPageMs(MENU_PAGE_TIME);

      s_tMenu.menuTime = curTime - MENU_MAX_TIMEOUT + MENU_MS_TO_TICKS(2000);
    }
  }

  // 菜单范围
  if ((0 == s_tMenu.depth) && (s_tMenu.item >= MENU_ITEM_FIRST_END))
    s_tMenu.item = MENU_ITEM_Heat;

  if ((1 == s_tMenu.depth) && (s_tMenu.item >= MENU_ITEM_SECOND_END))
    s_tMenu.item = MENU_ITEM_DevSN;

  if ((2 == s_tMenu.depth) && (s_tMenu.item >= MENU_ITEM_THREE_END))
    s_tMenu.item = MENU_ITEM_HM_WaterTemp;

  if (s_tMenu.item >= MENU_ITEM_Max)
  {
    s_tMenu.depth = 0;
    s_tMenu.item = MENU_ITEM_Heat;
  }

  s_tMenu.pageNum = 0;

  // 菜单显示
  switch (s_tMenu.item)
  {
    case MENU_ITEM_Start:
    {
      LCD_Clear(1);
      s_tMenu.item += 1;

      // 设置超时
      MENU_DelayMs(2000);
    }
    break;

    case MENU_ITEM_StartSn:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "%02X%02X%02X%02X", REC_DEV_SN[0], REC_DEV_SN[1], REC_DEV_SN[2], REC_DEV_SN[3]);
      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Logo, 1);

      s_tMenu.item += 1;

      // 设置超时
      MENU_DelayMs(2000);
    }
    break;

    case MENU_ITEM_StartVer:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "N%X.%X.%02X.%02X", g_tSysParam.softVer[0], g_tSysParam.softVer[1],
              g_tSysParam.softVer[2], g_tSysParam.softVer[3]);
      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Logo, 1);

      s_tMenu.item += 1;

      // 设置超时
      MENU_DelayMs(2000);
    }
    break;

    case MENU_ITEM_Top:
    {
      LCD_Clear(0);
      s_tMenu.item += 1;

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    //////////////////////////// 一级菜单 /////////////////////////////////////

    case MENU_ITEM_Heat:
    {
      // 不显示热量
      if (!s_tMenu.ptParam->tcFlag.bit.disppay)
      {
        s_tMenu.item += 1;
        break;
      }

      LCD_Clear(0);
      sprintf(s_tMenu.str, "%06d.%02d", s_tMenu.ptParam->allocHeat / 100, s_tMenu.ptParam->allocHeat % 100);
      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_kW, 1);
      LCD_DispSign(LCD_SIGN_Hour, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_WaterTemp:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "%02d.%01d  %02d.%01d", s_tMenu.ptCycle->inTemp / 100, s_tMenu.ptCycle->inTemp % 100 / 10,
                s_tMenu.ptCycle->outTemp / 100, s_tMenu.ptCycle->outTemp % 100 / 10);

      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Temp, 1);
      LCD_DispSign(LCD_SIGN_Celsius, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_Press:
    {
      if (bcd2dec(PM_TYPE_INVALID) == REC_PM_VENDOR)
      {
        s_tMenu.item += 1;
        break;
      }

      LCD_Clear(0);
      sprintf(s_tMenu.str, "%02d.%01d--%02d.%01d", s_tMenu.ptCycle->inPress / 100, s_tMenu.ptCycle->inPress % 100 / 10,
              s_tMenu.ptCycle->outPress / 100, s_tMenu.ptCycle->outPress % 100 / 10);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_Ammeter:
    {
      if (bcd2dec(AM_TYPE_INVALID) == REC_AM_VENDOR)
      {
        s_tMenu.item += 1;
        break;
      }

      LCD_Clear(0);
      sprintf(s_tMenu.str, "%06d.%02d", s_tMenu.ptCycle->amKwh / 100, s_tMenu.ptCycle->amKwh % 100);
      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_kW, 1);
      LCD_DispSign(LCD_SIGN_Hour, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_Date:
    {
      DateTime_t dateTime;

      TimeGet(&dateTime);

      LCD_Clear(0);
      s_tMenu.page %= 1;
      s_tMenu.pageNum = 1;

      sprintf(s_tMenu.str, "%02d-%02d-%02d", dateTime.year % 100, dateTime.month, dateTime.day);

      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_Time:
    {
      DateTime_t dateTime;

      TimeGet(&dateTime);

      LCD_Clear(0);
      s_tMenu.page %= 1;
      s_tMenu.pageNum = 1;

      sprintf(s_tMenu.str, "%02d %02d %02d", dateTime.hour, dateTime.minute, dateTime.second);

      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_RoomTemp:
    {
      // 温控未安装
      if (s_tMenu.ptParam->tcFlag.bit.uninstall)
      {
        s_tMenu.item += 1;
        break;
      }

      LCD_Clear(0);
      s_tMenu.page %= 2;
      s_tMenu.pageNum = 2;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "%02d.%01d  %02d.%01d", s_tMenu.ptParam->roomSet / 100, s_tMenu.ptParam->roomSet % 100 / 10,
                s_tMenu.ptCycle->roomTemp[0] / 100, s_tMenu.ptCycle->roomTemp[0] % 100 / 10);
      else
        sprintf(s_tMenu.str, "%02d.%01d--%02d.%01d", s_tMenu.ptParam->roomSetUpLimit / 100, s_tMenu.ptParam->roomSetUpLimit % 100 / 10,
                s_tMenu.ptParam->roomSetDnLimit / 100, s_tMenu.ptParam->roomSetDnLimit % 100 / 10);

      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Temp, s_tMenu.blink);
      LCD_DispSign(LCD_SIGN_Celsius, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_RoomFloor:
    {
      // 温控未安装
      if (s_tMenu.ptParam->tcFlag.bit.uninstall)
      {
        s_tMenu.item += 1;
        break;
      }

      LCD_Clear(0);
      s_tMenu.page %= 1;
      s_tMenu.pageNum = 1;

      sprintf(s_tMenu.str, "F%01d   L%02d", s_tMenu.ptParam->tcCom.bit.mode, s_tMenu.ptParam->tcCom.bit.floor);
      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_OpenTime:
    {
      uint16_t hour;
      uint8_t min;
      uint8_t sec;

      SYSEnterCritical();
      {
        hour = s_tMenu.ptCycle->openTime / 3600;
        min = (s_tMenu.ptCycle->openTime - (hour * 3600)) / 60;
        sec = s_tMenu.ptCycle->openTime % 60;
      }
      SYSExitCritical();

      LCD_Clear(0);
      s_tMenu.page %= 1;
      s_tMenu.pageNum = 1;

      sprintf(s_tMenu.str, "%04d.%02d.%02d", hour, min, sec);
      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_On, 1);
      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_CloseTime:
    {
      uint16_t hour;
      uint8_t min;
      uint8_t sec;

      SYSEnterCritical();
      {
        hour = s_tMenu.ptCycle->closeTime / 3600;
        min = (s_tMenu.ptCycle->closeTime - (hour * 3600)) / 60;
        sec = s_tMenu.ptCycle->closeTime % 60;
      }
      SYSExitCritical();

      LCD_Clear(0);
      s_tMenu.page %= 1;
      s_tMenu.pageNum = 1;

      sprintf(s_tMenu.str, "%04d.%02d.%02d", hour, min, sec);
      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_Off, 1);
      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_LockTime:
    {
      uint16_t hour;
      uint8_t min;
      uint8_t sec;

      SYSEnterCritical();
      {
        hour = s_tMenu.ptCycle->lockTime / 3600;
        min = (s_tMenu.ptCycle->lockTime - (hour * 3600)) / 60;
        sec = s_tMenu.ptCycle->lockTime % 60;
      }
      SYSExitCritical();

      LCD_Clear(0);
      s_tMenu.page %= 1;
      s_tMenu.pageNum = 1;

      sprintf(s_tMenu.str, "%04d.%02d.%02d", hour, min, sec);
      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_AdjNum:
    {
      uint8_t ofs = 0;

      LCD_Clear(0);
      s_tMenu.page %= 4;
      s_tMenu.pageNum = 4;

      if ((6 == s_tMenu.ptParam->valveAdjNum) || (9 == s_tMenu.ptParam->valveAdjNum))
        ofs = 1;
      else
        ofs = 0;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "%03d  %03d", s_tMenu.ptFsmObj->valve1SetNow + ofs, s_tMenu.ptCycle->valve1Fb + ofs);
      else if (1 == s_tMenu.page)
        sprintf(s_tMenu.str, "%03d--%03d", s_tMenu.ptParam->valveUpLimit + ofs, s_tMenu.ptParam->valveDnLimit + ofs);
      else if (2 == s_tMenu.page)
        sprintf(s_tMenu.str, "SET  %03d", s_tMenu.ptParam->valveSet + ofs);
      else
        sprintf(s_tMenu.str, "MAX  %03d", s_tMenu.ptParam->valveAdjNum + ofs);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ValveDN:
    {
      uint8_t dn;

      switch (VALVE_DNx)
      {
        case VALVE_DN32:
          dn = 32;
          break;

        case VALVE_DN40:
          dn = 40;
          break;

        case VALVE_DN50:
          dn = 50;
          break;

        case VALVE_DN65:
          dn = 65;
          break;

        case VALVE_DN80:
          dn = 80;
          break;

        case VALVE_DN100:
          dn = 100;
          break;

        case VALVE_DN125:
          dn = 125;
          break;

        case VALVE_DN150:
          dn = 150;
          break;

        case VALVE_DN200:
          dn = 200;
          break;

        default:
          dn = 0;
          break;
      }

      LCD_Clear(0);

      if (6 == s_tMenu.ptParam->valveAdjNum)
      {
        s_tMenu.page %= 8;
        s_tMenu.pageNum = 8;

        if (!s_tMenu.page)
          sprintf(s_tMenu.str, "DN%d", dn);
        else
          sprintf(s_tMenu.str, "D%d-%03d", s_tMenu.page, s_tMenu.ptParam->valveOpenTbl[s_tMenu.page - 1]);
      }
      else if (9 == s_tMenu.ptParam->valveAdjNum)
      {
        s_tMenu.page %= 11;
        s_tMenu.pageNum = 11;

        if (!s_tMenu.page)
          sprintf(s_tMenu.str, "DN%d", dn);
        else
          sprintf(s_tMenu.str, "D%d-%03d", s_tMenu.page, s_tMenu.ptParam->valveOpenTbl[s_tMenu.page - 1]);
      }
      else
      {
        s_tMenu.page %= 1;
        s_tMenu.pageNum = 1;
        sprintf(s_tMenu.str, "DN%d", dn);
      }

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;



    //////////////////////////// 二级菜单 /////////////////////////////////////

    case MENU_ITEM_DevSN:
    {
      LCD_Clear(0);
      s_tMenu.page %= 3;
      s_tMenu.pageNum = 3;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "%02X%02X%02X%02X", REC_DEV_SN[0], REC_DEV_SN[1], REC_DEV_SN[2], REC_DEV_SN[3]);
      else if (1 == s_tMenu.page)
        sprintf(s_tMenu.str, "ADDR-%03d", g_tSysParam.comAddr);
      else
        sprintf(s_tMenu.str, "TYPE-%02d", g_tSysParam.devType);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_SoftVer:
    {
      LCD_Clear(0);
      s_tMenu.page %= 2;
      s_tMenu.pageNum = 2;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "N%X.%X.%02X.%02X", g_tSysParam.softVer[0], g_tSysParam.softVer[1],
                g_tSysParam.softVer[2], g_tSysParam.softVer[3]);
      else
        sprintf(s_tMenu.str, "H%X.%X.%X.%X.%X", g_tSysParam.hardVer[0], g_tSysParam.hardVer[1],
                g_tSysParam.hardVer[2], g_tSysParam.hardVer[3], g_tSysParam.hardVer[4]);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ErrCode:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "ERR-%04X", s_tMenu.ptCycle->errCode.value);
      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_OpenRatio:
    {
      LCD_Clear(0);

      if (BIT_READ(REC_VALVE_FUNC, BIT(1)))
      {
        s_tMenu.page %= 5;
        s_tMenu.pageNum = 5;
      }
      else
      {
        s_tMenu.page %= 3;
        s_tMenu.pageNum = 3;
      }

      if (!s_tMenu.page)
      {
        sprintf(s_tMenu.str, "%01.05f", s_tMenu.ptCycle->totalOnRatio);
        LCD_DispSign(LCD_SIGN_OnRatio, 1);
      }
      else if (1 == s_tMenu.page)
      {
        sprintf(s_tMenu.str, "N1-%05d", s_tMenu.ptCycle->valve1RunCnt > 99999 ? 99999 : s_tMenu.ptCycle->valve1RunCnt);
        LCD_DispSign(LCD_SIGN_On, 1);
      }
      else if (2 == s_tMenu.page)
      {
        sprintf(s_tMenu.str, "T1-%03d", s_tMenu.ptCycle->valve1RunTime);
        LCD_DispSign(LCD_SIGN_Time, 1);
      }
      else if (3 == s_tMenu.page)
      {
        sprintf(s_tMenu.str, "N2-%05d", s_tMenu.ptCycle->valve2RunCnt > 99999 ? 99999 : s_tMenu.ptCycle->valve2RunCnt);
        LCD_DispSign(LCD_SIGN_On, 1);
      }
      else if (4 == s_tMenu.page)
      {
        sprintf(s_tMenu.str, "T2-%03d", s_tMenu.ptCycle->valve2RunTime);
        LCD_DispSign(LCD_SIGN_Time, 1);
      }

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_HeatSeason:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "%02d%02d%02d%02d", g_tSysParam.heatSeason[0], g_tSysParam.heatSeason[1],
              g_tSysParam.heatSeason[2], g_tSysParam.heatSeason[3]);
      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ValveDelay:
    {
      LCD_Clear(0);

      if (s_tMenu.ptCycle->valveStat.bit.running1)
        sprintf(s_tMenu.str, "R1 %03d", s_tMenu.ptFsmObj->valve1RunTime);
      else if (s_tMenu.ptCycle->valveStat.bit.running2)
        sprintf(s_tMenu.str, "R2 %03d", s_tMenu.ptFsmObj->valve2RunTime);
      else
        sprintf(s_tMenu.str, "RD %03d.%02d", s_tMenu.ptFsmObj->valveRunDelay / 60, s_tMenu.ptFsmObj->valveRunDelay % 60);

      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_ValveCheck:
    {
      uint8_t uc_tmp = 0;
      uint32_t ul_tmp = 0;

      LCD_Clear(0);
      s_tMenu.page %= 3;
      s_tMenu.pageNum = 3;

      if (!s_tMenu.page)
      {
        sprintf(s_tMenu.str, "T%d -----", s_tMenu.page + 1);
        uc_tmp = (VALVE_FB_CHECK_PERIOD && (s_tMenu.ptFsmObj->fbCheckTime <= VALVE_FB_CHECK_PERIOD));

        if (uc_tmp)
        {
          ul_tmp = VALVE_FB_CHECK_PERIOD - s_tMenu.ptFsmObj->fbCheckTime;

          if (ul_tmp < SYSMakeSec(0, 999, 60))
            sprintf(s_tMenu.str, "T%d %03d.%02d", s_tMenu.page + 1, ul_tmp / 60, ul_tmp % 60);
        }
      }
      else if (1 == s_tMenu.page)
      {
        sprintf(s_tMenu.str, "T%d -----", s_tMenu.page + 1);
        uc_tmp = ((s_tMenu.ptParam->valveRotateDay & 0xFE) &&
                  (s_tMenu.ptRTime->valve1UnchgedTime <= SYSMakeSec(24 * (s_tMenu.ptParam->valveRotateDay >> 1), 0, VALVE_EXEC_DELAY)));

        if (uc_tmp)
        {
          ul_tmp = SYSMakeSec(24 * (s_tMenu.ptParam->valveRotateDay >> 1), 0, VALVE_EXEC_DELAY) - s_tMenu.ptRTime->valve1UnchgedTime;

          if (ul_tmp < SYSMakeSec(24 * 99, 24 * 60, 0))
            sprintf(s_tMenu.str, "T%d %02d.%03X", s_tMenu.page + 1, ul_tmp / SYSMakeSec(24, 0, 0), ul_tmp % SYSMakeSec(24, 0, 0));
        }
      }
      else
      {
        sprintf(s_tMenu.str, "V1.%d%d.%d%d.%d%d",
                s_tMenu.ptFsmObj->flag.bit.valve1_dircorrecting, s_tMenu.ptFsmObj->flag.bit.valve1_dircorrect_step,
                s_tMenu.ptFsmObj->flag.bit.valve1_correcting, s_tMenu.ptFsmObj->flag.bit.valve1_correct_step,
                s_tMenu.ptFsmObj->flag.bit.valve1_rotating, s_tMenu.ptFsmObj->flag.bit.valve1_rotate_step);
      }

      LCD_DispString(s_tMenu.str);
      LCD_DispSign(LCD_SIGN_Time, 1);

      // 设置超时
      MENU_DelayMs(200);
    }
    break;

    case MENU_ITEM_ValveStatus:
    {
      LCD_Clear(0);

      // 有阀2
      if (BIT_READ(REC_VALVE_FUNC, BIT(1)))
      {
        s_tMenu.page %= 2;
        s_tMenu.pageNum = 2;
      }
      else
      {
        s_tMenu.page %= 1;
        s_tMenu.pageNum = 1;
      }

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "VS1 %d%d%d%d", s_tMenu.ptCycle->valveStat.bit.closed1, s_tMenu.ptCycle->valveStat.bit.running1,
                s_tMenu.ptCycle->valveStat.bit.blocking1, s_tMenu.ptCycle->valveStat.bit.fault1);
      else
        sprintf(s_tMenu.str, "VS2 %d%d%d%d", s_tMenu.ptCycle->valveStat.bit.closed2, s_tMenu.ptCycle->valveStat.bit.running2,
                s_tMenu.ptCycle->valveStat.bit.blocking2, s_tMenu.ptCycle->valveStat.bit.fault2);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

//    case MENU_ITEM_NB_IMEI:
//    {
//      // 上传数据无效
//      if (REC_EXMOD_TYPE_NB != REC_EXMOD_TYPE)
//      {
//        s_tMenu.item = MENU_ITEM_NB_CYCLE;
//        s_tMenu.item += 1;
//        break;
//      }

//      LCD_Clear(0);
//      s_tMenu.page %= 4;
//      s_tMenu.pageNum = 4;

//      if (!s_tMenu.page)
//        sprintf(s_tMenu.str, "IMEI-%d-%d", g_tNetParam.modType, g_tNetParam.comType);
//      else
//        sprintf(s_tMenu.str, "P%d-%c%c%c%c%c", s_tMenu.page,
//                s_tMenu.ptNBObj->imei[((s_tMenu.page - 1) * 5) + 0], s_tMenu.ptNBObj->imei[((s_tMenu.page - 1) * 5) + 1],
//                s_tMenu.ptNBObj->imei[((s_tMenu.page - 1) * 5) + 2], s_tMenu.ptNBObj->imei[((s_tMenu.page - 1) * 5) + 3],
//                s_tMenu.ptNBObj->imei[((s_tMenu.page - 1) * 5) + 4]);

//      LCD_DispString(s_tMenu.str);

//      // 设置超时
//      MENU_DelayMs(100);
//    }
//    break;

//    case MENU_ITEM_NB_IMSI:
//    {
//      //本程序没有NB，所以跳过NB参数的显示
//      s_tMenu.item = MENU_ITEM_NB_CYCLE;
//      s_tMenu.item += 1;
//      break;
//      
//      LCD_Clear(0);
//      s_tMenu.page %= 4;
//      s_tMenu.pageNum = 4;

//      if (!s_tMenu.page)
//        sprintf(s_tMenu.str, "IMSI");
//      else
//        sprintf(s_tMenu.str, "P%d-%c%c%c%c%c", s_tMenu.page,
//                s_tMenu.ptNBObj->imsi[((s_tMenu.page - 1) * 5) + 0], s_tMenu.ptNBObj->imsi[((s_tMenu.page - 1) * 5) + 1],
//                s_tMenu.ptNBObj->imsi[((s_tMenu.page - 1) * 5) + 2], s_tMenu.ptNBObj->imsi[((s_tMenu.page - 1) * 5) + 3],
//                s_tMenu.ptNBObj->imsi[((s_tMenu.page - 1) * 5) + 4]);

//      LCD_DispString(s_tMenu.str);

//      // 设置超时
//      MENU_DelayMs(100);
//    }
//    break;

//    case MENU_ITEM_NB_IP:
//    {
//      LCD_Clear(0);
//      s_tMenu.page %= 3;
//      s_tMenu.pageNum = 3;

//      if (!s_tMenu.page)
//        sprintf(s_tMenu.str, "IP");
//      else
//        sprintf(s_tMenu.str, "P%d-%d.%d", s_tMenu.page,
//                s_tMenu.ptNBObj->ipAddr[((s_tMenu.page - 1) * 2) + 0], s_tMenu.ptNBObj->ipAddr[((s_tMenu.page - 1) * 2) + 1]);

//      LCD_DispString(s_tMenu.str);

//      // 设置超时
//      MENU_DelayMs(100);
//    }
//    break;

//    case MENU_ITEM_NB_ERROR_STATE:
//    {
//      LCD_Clear(0);
//      s_tMenu.page %= 1;
//      s_tMenu.pageNum = 1;

//      sprintf(s_tMenu.str, "E%02d  R%02d", s_tMenu.ptNBAppObj->error, s_tMenu.ptNBAppObj->state);
//      LCD_DispString(s_tMenu.str);

//      // 设置超时
//      MENU_DelayMs(100);
//    }
//    break;

//    case MENU_ITEM_NB_RSSI:
//    {
//      LCD_Clear(0);
//      s_tMenu.pageNum = 3;

//      if (s_tMenu.ptNBAppObj->updateTime)
//        s_tMenu.pageNum += 1;

//      s_tMenu.page %= s_tMenu.pageNum;


//      if (!s_tMenu.page)
//        sprintf(s_tMenu.str, "S %02d-%d-%d", s_tMenu.ptNBObj->rssi, s_tMenu.ptNBObj->Flags.F.isActive, s_tMenu.ptNBObj->Flags.F.isSleep);
//      else if (1 == s_tMenu.page)
//        sprintf(s_tMenu.str, "NET%02d-%02d", s_tMenu.ptNBObj->netStat, s_tMenu.ptNBObj->modStat);
//      else if (2 == s_tMenu.page)
//        sprintf(s_tMenu.str, "ERR-%03d", s_tMenu.ptNBObj->cme);
//      else
//        sprintf(s_tMenu.str, "UP-%05d", s_tMenu.ptNBAppObj->updateTime);

//      LCD_DispString(s_tMenu.str);

//      // 设置超时
//      MENU_DelayMs(100);
//    }
//    break;

    case MENU_ITEM_NB_CYCLE:
    {
      LCD_Clear(0);
      s_tMenu.pageNum = 1;
      s_tMenu.page %= s_tMenu.pageNum;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "T %02d", REC_CycleUp);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_LORA_ONLINE:
    {
      LCD_Clear(0);
      s_tMenu.pageNum = 1;
      s_tMenu.page %= s_tMenu.pageNum;

      sprintf(s_tMenu.str, "OL%d %d", getWb25OnlineState(),getWb25ErrCode());

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }break;

    case MENU_ITEM_LORA_NETID_BAND:
    {
      LCD_Clear(0);
      s_tMenu.pageNum = 1;
      s_tMenu.page %= s_tMenu.pageNum;

      sprintf(s_tMenu.str, "D%03d B%02d", g_tSysParam.loraNetId,g_tSysParam.band);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }break;

    case MENU_ITEM_BAT_VOLTAGE:
    {
      LCD_Clear(0);
      s_tMenu.pageNum = 1;
      s_tMenu.page %= s_tMenu.pageNum;
      
      u32temp1 =  AIO_GetMcuVoltage();     //获取单片机电压
      u32temp2 =  AIO_GetValveVoltage();   //获取动力电池电压
      
      sprintf(s_tMenu.str, "%01d.%02d  %01d.%02d", u32temp1%10000/1000, u32temp1%1000/10,
                u32temp2%10000/1000, u32temp2%1000/10);
      
      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }break;
    
    
    
    //////////////////////////// 三级菜单 /////////////////////////////////////

    case MENU_ITEM_HM_WaterTemp:
    {
      // 不是通断控制器
      if (!BIT_READ(REC_VALVE_FUNC, BIT(1)))
      {
        s_tMenu.item += 1;
        break;
      }

      LCD_Clear(0);
      sprintf(s_tMenu.str, "RB %02d-%02d", s_tMenu.ptParam->hmInTemp, s_tMenu.ptParam->hmOutTemp);
      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_Temp, 1);
      LCD_DispSign(LCD_SIGN_Celsius, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_RoomCtrl:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "RC %d%d%d%d%d", s_tMenu.ptParam->tcFlag.bit.uninstall, s_tMenu.ptParam->tcFlag.bit.invalid,
              s_tMenu.ptParam->tcFlag.bit.remote, s_tMenu.ptParam->tcFlag.bit.disppay, s_tMenu.ptParam->tcFlag.bit.dispuse);
      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_RoomTempRemote:
    {
      LCD_Clear(0);
      s_tMenu.page %= 2;
      s_tMenu.pageNum = 2;

      if (!s_tMenu.page)
      {
        if (s_tMenu.ptParam->roomCorrect >= 0)
          sprintf(s_tMenu.str, "%02d.%01d   %d.%d", s_tMenu.ptParam->roomSetRemote / 100, s_tMenu.ptParam->roomSetRemote % 100 / 10,
                  s_tMenu.ptParam->roomCorrect / 10, s_tMenu.ptParam->roomCorrect % 10);
        else
          sprintf(s_tMenu.str, "%02d.%01d  -%d.%d", s_tMenu.ptParam->roomSetRemote / 100, s_tMenu.ptParam->roomSetRemote % 100 / 10,
                  ABS(s_tMenu.ptParam->roomCorrect) / 10, ABS(s_tMenu.ptParam->roomCorrect) % 10);
      }
      else
      {
        sprintf(s_tMenu.str, "%02d.%01d--%02d.%01d", s_tMenu.ptParam->roomSetRemoteUp / 100, s_tMenu.ptParam->roomSetRemoteUp % 100 / 10,
                s_tMenu.ptParam->roomSetRemoteDn / 100, s_tMenu.ptParam->roomSetRemoteDn % 100 / 10);
      }

      LCD_DispString(s_tMenu.str);

      LCD_DispSign(LCD_SIGN_Temp, 1);
      LCD_DispSign(LCD_SIGN_Celsius, 1);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ValveFlag:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "VF %d%d-%d%d", s_tMenu.ptParam->valveFlag.bit.force1, s_tMenu.ptParam->valveFlag.bit.force2,
              s_tMenu.ptParam->valveFlag.bit.pwroff1, s_tMenu.ptParam->valveFlag.bit.pwroff2);
      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ValveMode:
    {
      LCD_Clear(0);

      s_tMenu.page %= 25;
      s_tMenu.pageNum = 25;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "VM %d-%d-%d", s_tMenu.ptParam->valveMode.bit.mode, s_tMenu.ptParam->valveMode.bit.tc_off,
                s_tMenu.ptParam->valveMode.bit.com_off);
      else
        sprintf(s_tMenu.str, "H%d-%d", s_tMenu.page, s_tMenu.ptParam->hourValveSet[s_tMenu.page - 1]);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ValveOffline:
    {
      LCD_Clear(0);
      sprintf(s_tMenu.str, "VO %d-%d-%d", s_tMenu.ptParam->valveMode.bit.no_close, s_tMenu.ptParam->valveMode.bit.nc_open,
              s_tMenu.ptParam->valveMode.bit.run_tm + 1);
      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    case MENU_ITEM_ValveAntifreeze:
    {
      LCD_Clear(0);
      s_tMenu.page %= 2;
      s_tMenu.pageNum = 2;

      if (!s_tMenu.page)
        sprintf(s_tMenu.str, "VA %d-%d", s_tMenu.ptParam->freezeMode.bit.on1, s_tMenu.ptParam->freezeMode.bit.on2);
      else
        sprintf(s_tMenu.str, "%02d-%04d", s_tMenu.ptParam->freezeMode.bit.onTime, s_tMenu.ptParam->freezeMode.bit.onCycle);

      LCD_DispString(s_tMenu.str);

      // 设置超时
      MENU_DelayMs(100);
    }
    break;

    default:
    {
      s_tMenu.page = 0;
      s_tMenu.pageNum = 0;

      s_tMenu.depth = 0;
      s_tMenu.item = MENU_ITEM_Start;
    }
    break;
  }

EXIT_PROC:

  if (s_tMenu.item > MENU_ITEM_Top)
    MenuSignPoll();

  LCD_Refresh(0);
}





/**
 * @brief  获取菜单空闲状态
 * @param  None
 * @retval 空闲返回0 ，非0：还需要运行的时间ms
 */
uint16_t getMenuIdleState(void)
{
  if(s_tMenu.item <= MENU_ITEM_Top)
  {
    return 1;
  }
  else
  {
    return 0;
  }
	
}





/**
 * @brief  标志显示轮询
 * @param  None
 * @retval None
 */
void MenuSignPoll(void)
{
  uint32_t curTime;
  uint8_t  displog;

  curTime = MENU_GetCurTimeMs();

  if ((curTime - s_tMenu.blinkTime > MENU_MAX_TIMEOUT))
  {
    s_tMenu.blinkTime = curTime - MENU_MAX_TIMEOUT + MENU_MS_TO_TICKS(500);
    s_tMenu.blink = !s_tMenu.blink;
  }

  // Logo
  displog = !(s_tMenu.pageNum && s_tMenu.page < s_tMenu.pageNum - 1);
  LCD_DispSign(LCD_SIGN_Logo, displog || s_tMenu.blink);

  // 锁闭标志
  LCD_DispSign(LCD_SIGN_Lock, s_tMenu.ptParam->valveFlag.bit.lock);

  // 阀关状态,开度为0闪烁
  LCD_DispSign(LCD_SIGN_Valve, s_tMenu.ptFsmObj->flag.bit.valve1_close_next || (!s_tMenu.ptFsmObj->valve1SetNext && s_tMenu.blink));

  // 阀电量低常显,温控电量低闪烁
  LCD_DispSign(LCD_SIGN_Bat, s_tMenu.ptCycle->errCode.bit.mcu_bat || (s_tMenu.ptCycle->errCode.bit.valve_bat && s_tMenu.blink));   //s_tMenu.ptCycle->errCode.bit.tc_bat

  // 阀通信失联常显,Lora通信失败闪烁
  LCD_DispSign(LCD_SIGN_ComErr, s_tMenu.ptFsmObj->flag.bit.com_offline || ((getWb25ErrCode()== 22) && s_tMenu.blink));
}


/**
 * @}
 */

