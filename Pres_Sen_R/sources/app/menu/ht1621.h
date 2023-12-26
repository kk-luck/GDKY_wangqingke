/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     ht1621.h
  * @author   ZouZH
  * @version  V1.00
  * @date     17-June-2019
  * @brief    ht1621 lcd driver
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __HT1621_H
#define __HT1621_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

#define HT1621_SEG  20
#define HT1621_COM  4

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */
uint8_t HT1621_Init(void);
void    HT1621_Clear(uint8_t dat);
void    HT1621_Backup(void);
void    HT1621_Restore(void);
void    HT1621_Refresh(void);
void    HT1621_RefreshNoSave(void);
void    HT1621_DisplayOff(void);
void    HT1621_DisplayOn(void);
void    HT1621_WritePixel(uint8_t seg, uint8_t com, uint8_t dat);
uint8_t HT1621_ReadPixel(uint8_t seg, uint8_t com);
void    HT1621_DispChar(uint8_t pos, char ch, uint8_t dot);
void    HT1621_DispString(const char *string);

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __HT1621_H */

/***************************** END OF FILE *************************************/

