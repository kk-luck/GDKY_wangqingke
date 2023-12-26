/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     lcd.c
  * @author   ZouZH
  * @version  V1.00
  * @date     20-June-2019
  * @brief    lcd common interface.
  ******************************************************************************
  * @history
  */


/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "lcd.h"
#include "dg10107.h"


/**
 * @defgroup LCD
 * @brief lcd common interface
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

#define BACKLIGHT_PIN           FL_GPIO_PIN_0
#define BACKLIGHT_PORT          GPIOE

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */



/**
 * @brief  Init
 * @param  None
 * @retval 0 success
 */
uint8_t LCD_Init(void)
{
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull = FL_DISABLE;
  
  GPIO_InitStruct.pin = BACKLIGHT_PIN;
	
	FL_GPIO_ResetOutputPin(GPIOE, FL_GPIO_PIN_0);
  FL_GPIO_Init(BACKLIGHT_PORT, &GPIO_InitStruct);
	
	
  return DG10107_Init();
}


/**
 * @brief  清屏（清空显示缓冲区）
 * @param  dat: 0 all off, 1 all on
 * @retval None
 */
void LCD_Clear(uint8_t dat)
{
  DG10107_Clear(dat);
}

/**
 * @brief  恢复备份寄存器里的显示
 * @param  None
 * @retval None
 */
void LCD_Restore(void)
{
  DG10107_Restore();
}

/**
 * @brief  刷新显示区域
 * @param  all: 0 只刷新变化的, 1 刷新所有
 * @retval None
 */
void LCD_Refresh(uint8_t all)
{
  if (all)
  {
    DG10107_RefreshNoSave();
    DG10107_Backup();
  }
  else
    DG10107_Refresh();
}



/**
 * @brief  背光控制
 * @param  stat 0-Off, 1-On
 * @retval None
 */
void LCD_Backlight(uint8_t stat)
{
  if (stat && !FL_GPIO_GetOutputPin(GPIOE, FL_GPIO_PIN_0))
    FL_GPIO_SetOutputPin(GPIOE, FL_GPIO_PIN_0);

  if (!stat && FL_GPIO_GetOutputPin(GPIOE, FL_GPIO_PIN_0))
    FL_GPIO_ResetOutputPin(GPIOE, FL_GPIO_PIN_0);
}


/**
 * @brief  关闭LCD
 * @param  None
 * @retval None
 */
void LCD_DisplayOff(void)
{
  DG10107_DisplayOff();
}


/**
 * @brief  打开LCD
 * @param  None
 * @retval None
 */
void LCD_DisplayOn(void)
{
  DG10107_DisplayOn();
}


/**
 * @brief  液晶是否使能
 * @param  None
 * @retval None
 */
uint8_t LCD_IsEnable(void)
{
  return DG10107_IsEnable();
}


/**
 * @brief  显示字符
 * @param  pos: Display position (0-7)
 * @param  ch: Char to display (0-9, A-Z)
 * @param  dot: Is display dot
 * @retval None
 */
void LCD_DispChar(uint8_t pos, char ch, uint8_t dot)
{
  DG10107_DispChar(pos, ch, dot);
}


/**
 * @brief  显示字符串
 * @param  string: String to display (0-9, A-Z), length <= 8
 * @retval None
 */
void LCD_DispString(const char *string)
{
  DG10107_DispString(string);
}

/**
 * @brief  显示 符号
 * @param  string: String to display (0-9, A-Z), length <= 8
 * @param  on: 1 Dispaly, 0 Clear
 * @retval None
 */
void LCD_DispSign(LCD_SIGN_t LCD_SIGN_x, uint8_t on)
{
  DG10107_DispSign(LCD_SIGN_x, on);
}



/**
 * @}
 */

