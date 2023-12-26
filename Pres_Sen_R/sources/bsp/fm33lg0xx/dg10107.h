/**
  ******************************************************************************
  *               Copyright(C) 2020-2030 GDKY All Rights Reserved
  *
  * @file     dg10107.h
  * @author   ZouZH
  * @version  V1.00
  * @date     15-June-2020
  * @brief    lcd dg10107 driver for fm33g0xx.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __DG10107_H
#define __DG10107_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
#include "lcd.h"
/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

#define DG10107_SEG  20
#define DG10107_COM  4

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
uint8_t DG10107_Init(void);
void    DG10107_Clear(uint8_t dat);
void    DG10107_Backup(void);
void    DG10107_Restore(void);
void    DG10107_Refresh(void);
void    DG10107_RefreshNoSave(void);

void    DG10107_DisplayOff(void);
void    DG10107_DisplayOn(void);
uint8_t DG10107_IsEnable(void);   //“∫æß∆¡ «∑Ò πƒ‹

void    DG10107_DispSign(LCD_SIGN_t LCD_SIGN_x, uint8_t on);
void    DG10107_DispChar(uint8_t pos, char ch, uint8_t dot);
void    DG10107_DispString(const char *string);

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __DG10107_H */

/***************************** END OF FILE *************************************/

