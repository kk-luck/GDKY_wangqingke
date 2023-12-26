/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     lcd.h
  * @author   ZouZH
  * @version  V1.00
  * @date     20-June-2019
  * @brief    lcd common interface.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>

/**
 * @addtogroup LCD
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 * @brief LCD symbol sign
 */
typedef enum
{
  LCD_SIGN_Valve = 0,   /*!< 阀关符号 */
  LCD_SIGN_Time,        /*!< 时间 */
  LCD_SIGN_Off,         /*!< 断开 */
  LCD_SIGN_Logo,        /*!< Logo */
  LCD_SIGN_Celsius,     /*!< ℃ */
  LCD_SIGN_Lock,        /*!< 锁闭符号 */
  LCD_SIGN_ComErr,      /*!< 通信异常 */
  LCD_SIGN_Bat,         /*!< 低电符号 */
  LCD_SIGN_Temp,        /*!< 温度 */
  LCD_SIGN_On,          /*!< 接通 */
  LCD_SIGN_Hour,        /*!< 小时符号(h) */
  LCD_SIGN_kW,          /*!< 千瓦符号(kW.) */
  LCD_SIGN_OnRatio,     /*!< 通断比 */
  LCD_SIGN_Hour1,       /*!< 小时符号(/h) */
  LCD_SIGN_M3,          /*!< 立方米符号(m3) */
} LCD_SIGN_t;

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */
uint8_t LCD_Init(void);
void    LCD_Clear(uint8_t dat);
void    LCD_Restore(void);
void    LCD_Refresh(uint8_t all);
void    LCD_Backlight(uint8_t stat);
void    LCD_DisplayOff(void);
void    LCD_DisplayOn(void);
uint8_t LCD_IsEnable(void);   //

void    LCD_DispChar(uint8_t pos, char ch, uint8_t dot);
void    LCD_DispString(const char *string);
void    LCD_DispSign(LCD_SIGN_t LCD_SIGN_x, uint8_t on);


/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __TEMPLATE_H */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

