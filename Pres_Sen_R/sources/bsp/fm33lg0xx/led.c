/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file    led.c
  * @author  ZouZH
  * @version V1.03
  * @date    24-Feb-2022
  * @brief   This file contains the interface to the LED Service.
  ******************************************************************************
  * 2013-12-26 First create this file
  * 2014-12-22 Modify Led_GetSystemClock() unit is ms
  * 2018-01-24 Modify code organization
  * 2022-02-24 Fixed LED_MAX_LEDS less than actual quantity
  */

/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "led.h"

/* TYPEDEFS ----------------------------------------------------------------- */

/*
 * LED control structure
 */
typedef struct
{
  uint8_t mode;    /* Operation mode */

#ifdef LED_BLINK
  uint8_t todo;    /* Blink cycles left */
  uint8_t onPct;   /* On cycle percentage */
  uint16_t time;   /* On/Off cycle time(msec) */
  uint32_t next;   /* Time for next change */
#endif /* LED_BLINK */

} LedCtrl_t;

/*
 * LED status structure
 */
typedef struct
{
  LedCtrl_t ledCtrlTbl[LED_MAX_LEDS];
  uint8_t   sleepAct;
} LedObj_t;

/* MACROS  -------------------------------------------------------------------- */
#define LED_BOARD_TYPE 1

#if (LED_BOARD_TYPE == 1)

  #define PORT_LED1  GPIOA
  #define PORT_LED2  GPIOA
  #define PORT_LED3	 GPIOA
  #define PORT_LED4	 GPIOA

  #define PIN_LED1   FL_GPIO_PIN_0
  #define PIN_LED2   FL_GPIO_PIN_1
  #define PIN_LED3   FL_GPIO_PIN_2
  #define PIN_LED4   FL_GPIO_PIN_3
#endif /* LED_BOARD_TYPE */

/*
 * Turn on or off leds
 */
#define ON_LED1()         FL_GPIO_ResetOutputPin(PORT_LED1, PIN_LED1)
#define ON_LED2()         FL_GPIO_ResetOutputPin(PORT_LED2, PIN_LED2)
#define ON_LED3()         FL_GPIO_ResetOutputPin(PORT_LED3, PIN_LED3)
#define ON_LED4()         FL_GPIO_ResetOutputPin(PORT_LED4, PIN_LED4)

#define OFF_LED1()        FL_GPIO_SetOutputPin(PORT_LED1, PIN_LED1)
#define OFF_LED2()        FL_GPIO_SetOutputPin(PORT_LED2, PIN_LED2)
#define OFF_LED3()        FL_GPIO_SetOutputPin(PORT_LED3, PIN_LED3)
#define OFF_LED4()        FL_GPIO_SetOutputPin(PORT_LED4, PIN_LED4)

/*
 * Toggle led state
 */
#define TOGGLE_LED1()     FL_GPIO_ToggleOutputPin(PORT_LED1, PIN_LED1)
#define TOGGLE_LED2()     FL_GPIO_ToggleOutputPin(PORT_LED2, PIN_LED2)
#define TOGGLE_LED3()     FL_GPIO_ToggleOutputPin(PORT_LED3, PIN_LED3)
#define TOGGLE_LED4()     FL_GPIO_ToggleOutputPin(PORT_LED4, PIN_LED4)
/*
 * Get current led state
 */
#define STATE_LED1()      !FL_GPIO_GetOutputPin(PORT_LED1, PIN_LED1)
#define STATE_LED2()      !FL_GPIO_GetOutputPin(PORT_LED2, PIN_LED2)
#define STATE_LED3()      !FL_GPIO_GetOutputPin(PORT_LED3, PIN_LED3)
#define STATE_LED4()      !FL_GPIO_GetOutputPin(PORT_LED4, PIN_LED4)

/*
 * Get current system tick count
 */
#ifdef LED_BLINK
  #define Led_GetSystemClock() SYSGetRunMs()
#endif /* LED_BLINK && FreeRTOS */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

static LedObj_t sxLedObj;

/* LED current state */
static uint8_t  sucLedCurState;

/* LED state before going to sleep */
static uint8_t  sucLedSleepState;

#ifdef LED_BLINK
  /* LED state before going to blink mode */
  static uint8_t sucLedPreBlinkState;
  static TimerHandle_t shTmrLed;
#endif /* LED_BLINK */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */
static void LedLowLevelInit(FL_FunState NewState);
static void LedOnOff(uint8_t leds, uint8_t mode);

#ifdef LED_BLINK
  static void LedUpdate(void);
  static void vLedTmrCB(TimerHandle_t pxTimer);
#endif /* LED_BLINK */


/**
 * @brief  Initialize LED Service
 * @param  None
 * @retval 0 SUCCESS.
 */
uint8_t led_init(void)
{
  uint8_t ret = 0;

  LedLowLevelInit(FL_ENABLE);

  /* Set all LEDs to OFF */
  led_set(LED_ALL, LED_MODE_OFF);

#ifdef LED_BLINK
  /* Initialize sleepActive to FLASE */
  sxLedObj.sleepAct = 0;

  /* Create led blink update timer */
  shTmrLed = xTimerCreate("LedTmr", SYSMsToTick(LED_FLASH_TIME), pdFALSE, (void *)1U, vLedTmrCB);

  if (NULL == shTmrLed)
  {
    xprintf("Led blink timer create failure!\r\n");
    ret = 1;
  }

#endif /* LED_BLINK */

  return ret;
}


/**
 * @brief  Tun ON/OFF/TOGGLE given LEDs
 * @param  leds - bit mask value of leds to be turned ON/OFF/TOGGLE
 *         mode - BLINK, FLASH, TOGGLE, ON, OFF
 * @retval LED state at last set/clr/blink/ update
 */
uint8_t led_set(uint8_t leds, uint8_t mode)
{
  uint8_t led;
  uint8_t ledcnt;
  LedCtrl_t *pledctrl;

  switch (mode)
  {
#ifdef LED_BLINK

    case LED_MODE_BLINK:
    {
      /* Default blink 1 time, D% duty cycle */
      led_blink(leds, 1, LED_DUTY_CYCLE, LED_FLASH_TIME);
    }
    break;

    case LED_MODE_FLASH:
    {
      led_blink(leds, LED_FLASH_COUNT, LED_DUTY_CYCLE, LED_FLASH_TIME);
    }
    break;
#endif /* LED_BLINK */

    case LED_MODE_ON:
    case LED_MODE_OFF:
    case LED_MODE_TOGGLE:
    {
      led = 1;
      ledcnt = 0;
      leds &= LED_ALL;
      pledctrl = sxLedObj.ledCtrlTbl;

      while (leds && (ledcnt < LED_MAX_LEDS))
      {
        if (leds & led)
        {
          if (mode != LED_MODE_TOGGLE)
            pledctrl->mode = mode;
          else
            pledctrl->mode ^= LED_MODE_ON;

          LedOnOff(led, pledctrl->mode);
          leds ^= led;
        }

        led <<= 1;
        pledctrl++;
        ledcnt++;  
      }
    }
    break;

    default:
      break;
  }

  return sucLedCurState;
}


/**
 * @brief  Turns specified LED ON or OFF
 * @param  leds - LED bit mask
 *         mode - LED_ON,LED_OFF,
 * @retval None
 */
static void LedOnOff(uint8_t leds, uint8_t mode)
{
  if (leds & LED_1)
  {
    if (LED_MODE_ON & mode)
      ON_LED1();
    else
      OFF_LED1();
  }

  if (leds & LED_2)
  {
    if (LED_MODE_ON & mode)
      ON_LED2();
    else
      OFF_LED2();
  }

  if (leds & LED_3)
  {
    if (LED_MODE_ON & mode)
      ON_LED3();
    else
      OFF_LED3();
  }

  if (leds & LED_4)
  {
    if (LED_MODE_ON & mode)
      ON_LED4();
    else
      OFF_LED4();
  }

  /* Remember current led state */
  if (LED_MODE_ON & mode)
    sucLedCurState |= leds;
  else
    sucLedCurState &= (leds ^ 0xFF);
}


/**
 * @brief  Initialize the led low level hardware
 * @param  None
 * @retval None
 */
static void LedLowLevelInit(FL_FunState NewState)
{
  FL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (FL_DISABLE == NewState)
    return;

  OFF_LED1();
  OFF_LED2();
  OFF_LED3();
  OFF_LED4();

  // Configure pin as PP output
  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull = FL_DISABLE;
  
  GPIO_InitStruct.pin = PIN_LED1;
  FL_GPIO_Init(PORT_LED1, &GPIO_InitStruct);

  GPIO_InitStruct.pin = PIN_LED2;
  FL_GPIO_Init(PORT_LED2, &GPIO_InitStruct);

  GPIO_InitStruct.pin = PIN_LED3;
  FL_GPIO_Init(PORT_LED3, &GPIO_InitStruct);

  GPIO_InitStruct.pin = PIN_LED4;
  FL_GPIO_Init(PORT_LED4, &GPIO_InitStruct);
}


/**
 * @brief  Get current led state
 * @param  None
 * @retval None
 */
uint8_t LedGetState(void)
{
  return sucLedCurState;
}


/**
 * @brief  Store current LEDs state before sleep
 * @param  None
 * @retval None
 */
void LedEnterSleep(void)
{
#ifdef LED_BLINK
  /* Sleep ON */
  sxLedObj.sleepAct = 1;

  if (xTimerStop(shTmrLed, SYSMsToTick(100)) != pdPASS)
    xprintf("Led blink timer stop failure!\r\n");

#endif /* LED_BLINK */

  /* Save the state of each led */
  sucLedSleepState = 0;
  sucLedSleepState |= STATE_LED1();
  sucLedSleepState |= STATE_LED2() << 1;
  sucLedSleepState |= STATE_LED3() << 2;
  sucLedSleepState |= STATE_LED4() << 3;

  /* TURN OFF all LEDS to save power */
  LedOnOff(LED_ALL, LED_MODE_OFF);

  LedLowLevelInit(FL_DISABLE);
}


/**
 * @brief  Restore current LEDs state after sleep
 *
 * @param  None
 *
 * @retval None
 */
void LedExitSleep(void)
{
  LedLowLevelInit(FL_ENABLE);

  /* Load back the saved state */
  LedOnOff(sucLedSleepState, LED_MODE_ON);

#ifdef LED_BLINK
  /* Restart - This takes case BLINKING LEDS */
  LedUpdate();

  /* Sleep OFF */
  sxLedObj.sleepAct = 0;
#endif /* LED_BLINK */
}


#ifdef LED_BLINK

/**
 * @brief  Blink the leds
 * @param  leds       - bit mask value of leds to be blinked
 *         numBlinks  - number of blinks
 *         percent    - the percentage in each period where the led will be on
 *         period     - length of each cycle in milliseconds
 * @retval None
 */
void led_blink(uint8_t leds, uint8_t numBlinks, uint8_t onPct, uint16_t period)
{
  uint8_t led;
  uint8_t ledcnt;
  LedCtrl_t *pledctrl;

  if (!leds || !onPct || !period)
  {
    led_set(leds, LED_MODE_OFF);
    return;
  }

  if (onPct >= 100)
  {
    led_set(leds, LED_MODE_ON);
    return;
  }

  led = 1;
  ledcnt = 0;
  leds &= LED_ALL;
  pledctrl = sxLedObj.ledCtrlTbl;

  while (leds && (ledcnt < LED_MAX_LEDS))
  {
    if (leds & led)
    {
      /* Store the current state of the led before going to blinking if not already blinking */
      if (pledctrl->mode < LED_MODE_BLINK)
        sucLedPreBlinkState |= (led & sucLedCurState);

      /* Stop previous blink */
      pledctrl->mode = LED_MODE_OFF;

      /* Time for one on/off cycle */
      pledctrl->time = period;

      /* % of cycle LED is on */
      pledctrl->onPct = onPct;

      /* Number of blink cycles */
      pledctrl->todo  = numBlinks;

      /* Continuous */
      if (!numBlinks)
        pledctrl->mode |= LED_MODE_FLASH;

      /* Start now */
      pledctrl->next = Led_GetSystemClock();

      /* Enable blinking */
      pledctrl->mode |= LED_MODE_BLINK;
      leds ^= led;
    }

    led <<= 1;
    pledctrl++;
    ledcnt++;
  }

  /* Cancel any overlapping timer for blink events */
  if (xTimerStop(shTmrLed, SYSMsToTick(100)) != pdPASS)
    xprintf("Led blink timer stop failure!\r\n");
  else
    LedUpdate();
}


/**
 * @brief  Update leds to work with blink
 * @param  None
 * @retval None
 */
void LedUpdate(void)
{
  uint8_t led;
  uint8_t pct = 10;
  uint8_t leds;
  LedCtrl_t *pledctrl;
  uint32_t time;
  uint16_t next;
  uint16_t wait;

  /* Check if sleep is active or not */
  if (sxLedObj.sleepAct)
    return;

  next = 0;
  led = 1;
  leds = LED_ALL;
  pledctrl = sxLedObj.ledCtrlTbl;

  while (leds)
  {
    if (!(leds & led))
    {
      led <<= 1;
      pledctrl++;
      continue;
    }

    if (!(pledctrl->mode & LED_MODE_BLINK))
    {
      leds ^= led;
      led <<= 1;
      pledctrl++;
      continue;
    }

    /* Get current system tick count(ms) */
    time = Led_GetSystemClock();

    if (time >= pledctrl->next)
    {
      if (pledctrl->mode & LED_MODE_ON)
      {
        /* Percentage of cycle for off */
        pct = 100U - pledctrl->onPct;

        /* Say it's not on */
        pledctrl->mode &= ~LED_MODE_ON;
        LedOnOff(led, LED_MODE_OFF);

        /* Not continuous, reduce count */
        if (!(pledctrl->mode & LED_MODE_FLASH))
          pledctrl->todo--;
      }
      else if ((!pledctrl->todo) && !(pledctrl->mode & LED_MODE_FLASH))
      {
        /* No more blinks */
        pledctrl->mode ^= LED_MODE_BLINK;
      }
      else
      {
        /* Percentage of cycle for on */
        pct = pledctrl->onPct;
        pledctrl->mode |= LED_MODE_ON;
        LedOnOff(led, LED_MODE_ON);
      }

      if (pledctrl->mode & LED_MODE_BLINK)
      {
        wait = (((uint32_t)pct * (uint32_t)pledctrl->time) / 100U);
        pledctrl->next = time + wait;
      }
      else
      {
        /* No more blink, No more wait */
        wait = 0;

        /* After blinking, set the LED back to the state before it blinks */
//        led_blink(led, ((sucLedPreBlinkState & led) != 0U) ? LED_MODE_ON : LED_MODE_OFF);

        /* Clear the saved bit */
        sucLedPreBlinkState &= led ^ 0xFF;
      }
    }
    else
    {
      /* Time left */
      wait = pledctrl->next - time;
    }

    if (!next || (wait && (wait < next)))
    {
      next = wait;
    }

    leds ^= led;
    led <<= 1;
    pledctrl++;
  }

  if (next)
  {
    /* Start timer to schedule event */
//    if (xTimerChangePeriod(shTmrLed,  SYS_MS_TO_TICKS(next), 0) != pdPASS)
//    {
//      xprintf("Led blink timer start failure!\r\n");
//    }
  }
}


static void vLedTmrCB(TimerHandle_t pxTimer)
{
  LedUpdate();
}
#endif /* LED_BLINK */

