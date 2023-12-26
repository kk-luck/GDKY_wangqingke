/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     dg10107.c
  * @author   ZouZH
  * @version  V1.00
  * @date     17-Jan-2022
  * @brief    lcd dg10107 driver for fm33lg0xx.
  ******************************************************************************
  */

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "dg10107.h"
#include "dg10107_table.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

/**
 * display buffer
 */
static uint32_t s_u32DispBuf[10];    //显示缓冲区
static uint32_t s_ucDispBak[10];     //显示缓冲区备份


/* LOCAL FUNCTIONS ------------------------------------------------------------ */

static uint8_t dg10107_get_disp_code(char ch);

/**
 * @brief  Init
 * @param  None
 * @retval 0 success
 */
uint8_t DG10107_Init(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStruct;
  FL_LCD_InitTypeDef LCD_InitStruct;

  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull = FL_DISABLE;
  GPIO_InitStruct.analogSwitch = FL_DISABLE;

  // COM0~3
  GPIO_InitStruct.pin = FL_GPIO_PIN_0 | FL_GPIO_PIN_1 | FL_GPIO_PIN_2 | FL_GPIO_PIN_3;
  FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // LCD Init
  FL_LCD_DeInit(LCD);
  FL_LCD_StructInit(&LCD_InitStruct);
  LCD_InitStruct.biasCurrent = FL_LCD_BIAS_CURRENT_HIGH;
  LCD_InitStruct.biasMode = FL_LCD_BIAS_MODE_3BIAS;
  LCD_InitStruct.biasVoltage = FL_LCD_BIAS_VOLTAGE_LEVEL15;
  LCD_InitStruct.COMxNum = FL_LCD_COM_NUM_4COM;
  LCD_InitStruct.waveform = FL_LCD_WAVEFORM_TYPEA;
  LCD_InitStruct.displayFreq = 64;
  LCD_InitStruct.mode = FL_LCD_DRIVER_MODE_INNER_RESISTER;
  FL_LCD_Init(LCD, &LCD_InitStruct);

  // COM and SEG Init
  FL_LCD_EnableCOMEN(LCD, FL_LCD_COMEN_COM0);
  FL_LCD_EnableCOMEN(LCD, FL_LCD_COMEN_COM1);
  FL_LCD_EnableCOMEN(LCD, FL_LCD_COMEN_COM2);
  FL_LCD_EnableCOMEN(LCD, FL_LCD_COMEN_COM3);

  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG0);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG1);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG2);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG3);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG4);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG5);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG6);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG7);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG8);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG9);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG10);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG11);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG12);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG13);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG14);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG15);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG16);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG17);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG18);
  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG19);
//  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG20);
//  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG21);
//  FL_LCD_EnableSEGEN0(LCD, FL_LCD_SEGEN0_SEG22);

  lib_memset(s_u32DispBuf, 0, sizeof(s_u32DispBuf));
  lib_memset(s_ucDispBak, 0, sizeof(s_ucDispBak));

  FL_LCD_Enable(LCD);

  return 0;
}


/**
 * @brief  清空 显示缓冲区
 * @param  dat: 0 all off, 1 all on
 * @retval None
 */
void DG10107_Clear(uint8_t dat)
{
  if (!dat)
    lib_memset(s_u32DispBuf, 0x00, sizeof(s_u32DispBuf));
  else
    lib_memset(s_u32DispBuf, 0xFF, sizeof(s_u32DispBuf));
}

/**
 * @brief  备份显示缓冲区
 * @param  None
 * @retval None
 */
void DG10107_Backup(void)
{
  lib_memcpy(s_ucDispBak, s_u32DispBuf, sizeof(s_ucDispBak));
  
}


/**
 * @brief  恢复备份寄存器里的显示
 * @param  None
 * @retval None
 */
void DG10107_Restore(void)
{
  lib_memcpy(s_u32DispBuf, s_ucDispBak, sizeof(s_u32DispBuf));

  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG0, s_u32DispBuf[0]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG1, s_u32DispBuf[1]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG2, s_u32DispBuf[2]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG3, s_u32DispBuf[3]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG4, s_u32DispBuf[4]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG5, s_u32DispBuf[5]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG6, s_u32DispBuf[6]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG7, s_u32DispBuf[7]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG8, s_u32DispBuf[8]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG9, s_u32DispBuf[9]);
}

/**
 * @brief  显示缓冲区里的数据 放入 液晶寄存器  并 备份
 * @param  None
 * @retval None
 */
void DG10107_Refresh(void)
{
  if (!lib_memcmp(s_u32DispBuf, s_ucDispBak, sizeof(s_u32DispBuf)))
    return ;

  lib_memcpy(s_ucDispBak, s_u32DispBuf, sizeof(s_ucDispBak));

  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG0, s_u32DispBuf[0]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG1, s_u32DispBuf[1]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG2, s_u32DispBuf[2]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG3, s_u32DispBuf[3]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG4, s_u32DispBuf[4]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG5, s_u32DispBuf[5]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG6, s_u32DispBuf[6]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG7, s_u32DispBuf[7]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG8, s_u32DispBuf[8]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG9, s_u32DispBuf[9]);
}

/**
* @brief  显示缓冲区里的数据 放入 液晶寄存器  不 备份
 * @param  None
 * @retval None
 */
void DG10107_RefreshNoSave(void)
{
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG0, s_u32DispBuf[0]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG1, s_u32DispBuf[1]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG2, s_u32DispBuf[2]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG3, s_u32DispBuf[3]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG4, s_u32DispBuf[4]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG5, s_u32DispBuf[5]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG6, s_u32DispBuf[6]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG7, s_u32DispBuf[7]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG8, s_u32DispBuf[8]);
  FL_LCD_WriteData(LCD, FL_LCD_DATA_REG9, s_u32DispBuf[9]);
}


/**
 * @brief  Trun-off display
 * @param  None
 * @retval None
 */
void DG10107_DisplayOff(void)
{
  FL_LCD_Disable(LCD);
}


/**
 * @brief  Trun-on display
 * @param  None
 * @retval None
 */
void DG10107_DisplayOn(void)
{
  FL_LCD_Enable(LCD);
}


/**
 * @brief  液晶是否使能
 * @param  None
* @retval 0不使能   1使能
 */
uint8_t DG10107_IsEnable(void)
{
  return FL_LCD_IsEnabled(LCD);
}




/**
 * @brief  显示符号（只更改缓冲区数据）
 * @param  string: String to display (0-9, A-Z), length <= 8
 * @param  on: 1 Dispaly, 0 Clear
 * @retval None
 */
void DG10107_DispSign(LCD_SIGN_t LCD_SIGN_x, uint8_t on)
{
  switch (LCD_SIGN_x)
  {
    case LCD_SIGN_Valve:
      if (on)
        SET_T1;
      else
        CLR_T1;

      break;

    case LCD_SIGN_Time:
      if (on)
        SET_HZ3;
      else
        CLR_HZ3;

      break;

    case LCD_SIGN_Off:
      if (on)
        SET_HZ2;
      else
        CLR_HZ2;

      break;

    case LCD_SIGN_Logo:
      if (on)
        SET_BZ;
      else
        CLR_BZ;

      break;

    case LCD_SIGN_Celsius:
      if (on)
        SET_TEMP;
      else
        CLR_TEMP;

      break;

    case LCD_SIGN_Lock:
      if (on)
        SET_LOCK;
      else
        CLR_LOCK;

      break;

    case LCD_SIGN_ComErr:
      if (on)
        SET_HZ8;
      else
        CLR_HZ8;

      break;

    case LCD_SIGN_Bat:
      if (on)
        SET_HZ7;
      else
        CLR_HZ7;

      break;

    case LCD_SIGN_Temp:
      if (on)
        SET_HZ5;
      else
        CLR_HZ5;

      break;

    case LCD_SIGN_On:
      if (on)
        SET_HZ1;
      else
        CLR_HZ1;

      break;

    case LCD_SIGN_Hour:
      if (on)
        SET_H;
      else
        CLR_H;

      break;

    case LCD_SIGN_kW:
      if (on)
        SET_KW;
      else
        CLR_KW;

      break;

    case LCD_SIGN_OnRatio:
      if (on)
        SET_HZ4;
      else
        CLR_HZ4;

      break;

    case LCD_SIGN_Hour1:
      if (on)
        SET_H_SLASH;
      else
        SET_H_SLASH;

      break;

    case LCD_SIGN_M3:
      if (on)
        SET_M3;
      else
        SET_M3;

      break;

    default:
      break;
  }
}

/**
 * @brief  显示字符（只更改缓冲区数据）
 * @param  pos: Display position (0-7)
 * @param  ch: Char to display (0-9, A-Z)
 * @param  dot: Is display dot
 * @retval None
 */
void DG10107_DispChar(uint8_t pos, char ch, uint8_t dot)
{
  uint8_t code;

  if (pos >= 8)
    return;

  // bit:  7  6  5  4  3  2  1  0
  //      dp  c  b  a  d  e  g  f
  code = dg10107_get_disp_code(ch) | ((dot & 0x01) << 7);

  switch (pos)
  {
    case 0:
      if (BIT_READ(code, BIT(0)))
        SET_F1;
      else
        CLR_F1;

      if (BIT_READ(code, BIT(1)))
        SET_G1;
      else
        CLR_G1;

      if (BIT_READ(code, BIT(2)))
        SET_E1;
      else
        CLR_E1;

      if (BIT_READ(code, BIT(3)))
        SET_D1;
      else
        CLR_D1;

      if (BIT_READ(code, BIT(4)))
        SET_A1;
      else
        CLR_A1;

      if (BIT_READ(code, BIT(5)))
        SET_B1;
      else
        CLR_B1;

      if (BIT_READ(code, BIT(6)))
        SET_C1;
      else
        CLR_C1;

      if (BIT_READ(code, BIT(7)))
        SET_H1;
      else
        CLR_H1;

      break;

    case 1:
      if (BIT_READ(code, BIT(0)))
        SET_F2;
      else
        CLR_F2;

      if (BIT_READ(code, BIT(1)))
        SET_G2;
      else
        CLR_G2;

      if (BIT_READ(code, BIT(2)))
        SET_E2;
      else
        CLR_E2;

      if (BIT_READ(code, BIT(3)))
        SET_D2;
      else
        CLR_D2;

      if (BIT_READ(code, BIT(4)))
        SET_A2;
      else
        CLR_A2;

      if (BIT_READ(code, BIT(5)))
        SET_B2;
      else
        CLR_B2;

      if (BIT_READ(code, BIT(6)))
        SET_C2;
      else
        CLR_C2;

      if (BIT_READ(code, BIT(7)))
        SET_H2;
      else
        CLR_H2;

      break;

    case 2:
      if (BIT_READ(code, BIT(0)))
        SET_F3;
      else
        CLR_F3;

      if (BIT_READ(code, BIT(1)))
        SET_G3;
      else
        CLR_G3;

      if (BIT_READ(code, BIT(2)))
        SET_E3;
      else
        CLR_E3;

      if (BIT_READ(code, BIT(3)))
        SET_D3;
      else
        CLR_D3;

      if (BIT_READ(code, BIT(4)))
        SET_A3;
      else
        CLR_A3;

      if (BIT_READ(code, BIT(5)))
        SET_B3;
      else
        CLR_B3;

      if (BIT_READ(code, BIT(6)))
        SET_C3;
      else
        CLR_C3;

      if (BIT_READ(code, BIT(7)))
        SET_H3;
      else
        CLR_H3;

      break;

    case 3:
      if (BIT_READ(code, BIT(0)))
        SET_F4;
      else
        CLR_F4;

      if (BIT_READ(code, BIT(1)))
        SET_G4;
      else
        CLR_G4;

      if (BIT_READ(code, BIT(2)))
        SET_E4;
      else
        CLR_E4;

      if (BIT_READ(code, BIT(3)))
        SET_D4;
      else
        CLR_D4;

      if (BIT_READ(code, BIT(4)))
        SET_A4;
      else
        CLR_A4;

      if (BIT_READ(code, BIT(5)))
        SET_B4;
      else
        CLR_B4;

      if (BIT_READ(code, BIT(6)))
        SET_C4;
      else
        CLR_C4;

      if (BIT_READ(code, BIT(7)))
        SET_H4;
      else
        CLR_H4;

      break;

    case 4:
      if (BIT_READ(code, BIT(0)))
        SET_F5;
      else
        CLR_F5;

      if (BIT_READ(code, BIT(1)))
        SET_G5;
      else
        CLR_G5;

      if (BIT_READ(code, BIT(2)))
        SET_E5;
      else
        CLR_E5;

      if (BIT_READ(code, BIT(3)))
        SET_D5;
      else
        CLR_D5;

      if (BIT_READ(code, BIT(4)))
        SET_A5;
      else
        CLR_A5;

      if (BIT_READ(code, BIT(5)))
        SET_B5;
      else
        CLR_B5;

      if (BIT_READ(code, BIT(6)))
        SET_C5;
      else
        CLR_C5;

      if (BIT_READ(code, BIT(7)))
        SET_H5;
      else
        CLR_H5;

      break;

    case 5:
      if (BIT_READ(code, BIT(0)))
        SET_F6;
      else
        CLR_F6;

      if (BIT_READ(code, BIT(1)))
        SET_G6;
      else
        CLR_G6;

      if (BIT_READ(code, BIT(2)))
        SET_E6;
      else
        CLR_E6;

      if (BIT_READ(code, BIT(3)))
        SET_D6;
      else
        CLR_D6;

      if (BIT_READ(code, BIT(4)))
        SET_A6;
      else
        CLR_A6;

      if (BIT_READ(code, BIT(5)))
        SET_B6;
      else
        CLR_B6;

      if (BIT_READ(code, BIT(6)))
        SET_C6;
      else
        CLR_C6;

      if (BIT_READ(code, BIT(7)))
        SET_H6;
      else
        CLR_H6;

      break;

    case 6:
      if (BIT_READ(code, BIT(0)))
        SET_F7;
      else
        CLR_F7;

      if (BIT_READ(code, BIT(1)))
        SET_G7;
      else
        CLR_G7;

      if (BIT_READ(code, BIT(2)))
        SET_E7;
      else
        CLR_E7;

      if (BIT_READ(code, BIT(3)))
        SET_D7;
      else
        CLR_D7;

      if (BIT_READ(code, BIT(4)))
        SET_A7;
      else
        CLR_A7;

      if (BIT_READ(code, BIT(5)))
        SET_B7;
      else
        CLR_B7;

      if (BIT_READ(code, BIT(6)))
        SET_C7;
      else
        CLR_C7;

      if (BIT_READ(code, BIT(7)))
        SET_H7;
      else
        CLR_H7;

      break;

    case 7:
      if (BIT_READ(code, BIT(0)))
        SET_F8;
      else
        CLR_F8;

      if (BIT_READ(code, BIT(1)))
        SET_G8;
      else
        CLR_G8;

      if (BIT_READ(code, BIT(2)))
        SET_E8;
      else
        CLR_E8;

      if (BIT_READ(code, BIT(3)))
        SET_D8;
      else
        CLR_D8;

      if (BIT_READ(code, BIT(4)))
        SET_A8;
      else
        CLR_A8;

      if (BIT_READ(code, BIT(5)))
        SET_B8;
      else
        CLR_B8;

      if (BIT_READ(code, BIT(6)))
        SET_C8;
      else
        CLR_C8;

      break;

    default:
      break;
  }
}


/**
 * @brief  显示字符串（只更改缓冲区数据）
 * @param  string: String to display (0-9, A-Z), length <= 8
 * @retval None
 */
void DG10107_DispString(const char *string)
{
  char ch = ' ';
  uint8_t pos = 0;

  while ('\0' != *string)
  {
    if (pos >= 8)
      break;

    ch = *string++;

    if ('.' == *string)
    {
      DG10107_DispChar(pos, ch, 1);
      string++;
    }
    else
      DG10107_DispChar(pos, ch, 0);

    pos += 1;
  }
}


/**
 * @brief  Convert an alphanumeric ch to the 7-segment code.
 * @param  ch: Display  char
 * @retval segment code
 */
static uint8_t dg10107_get_disp_code(char ch)
{
  /**
   *  LED Segments:         a
   *                     ----
   *                   f|    |b
   *                    |  g |
   *                     ----
   *                   e|    |c
   *                    |    |
   *                     ----  o dp
   *                       d
   *      bit:  7  6  5  4  3  2  1  0
   *           dp  c  b  a  d  e  g  f
   */
  static const struct
  {
    char    ascii;
    uint8_t code;
  } CHAR_CODE[] =
  {
    {' ', 0x00},
    {'-', 0x02},
    {'0', 0x7D},
    {'1', 0x60},
    {'2', 0x3E},
    {'3', 0x7A},
    {'4', 0x63},
    {'5', 0x5B},
    {'6', 0x5F},
    {'7', 0x70},
    {'8', 0x7F},
    {'9', 0x7B},
    {'A', 0x77},
    {'B', 0x4F},
    {'C', 0x1D},
    {'D', 0x6E},
    {'E', 0x1F},
    {'F', 0x17},
    {'G', 0x5D},
    {'H', 0x67},
    {'I', 0x1C},
    {'J', 0x6C},
    {'K', 0x27},
    {'L', 0x0D},
    {'M', 0x56},
    {'N', 0x46},
    {'O', 0x5E},
    {'P', 0x37},
    {'Q', 0x73},
    {'R', 0x06},
    {'S', 0x53},
    {'T', 0x0F},
    {'U', 0x6D},
    {'V', 0x4C},
    {'W', 0x2B},
    {'X', 0x5C},
    {'Y', 0x6B},
    {'Z', 0x1A},
    {0x00, 0x00}
  };

  for (char i = 0; CHAR_CODE[i].ascii; i++)
  {
    if (ch == CHAR_CODE[i].ascii)
      return CHAR_CODE[i].code;
  }

  return 0;
}


