/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     fm24clxx.h
  * @author   ZouZH
  * @version  V1.09
  * @date     12-June-2019
  * @brief    RAMTRON FRAM Serial Memory Driver.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __FM24CLXX_H
#define __FM24CLXX_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>

/* TYPEDEFS ------------------------------------------------------------------- */

typedef enum
{
//  FM24CL64_1    = 2,    /* 64Kb(8KB)  */
//  FM24CL64_2    = 3,    /* 64Kb(8KB)  */
	FM24W256_1    = 0,    /* 256Kb(32KB)  */
} FRAMDev_t;

typedef enum
{
  FRAM_OK = 0,
  FRAM_WRITE,
  FRAM_READ,
  FRAM_TMOUT,
  FRAM_MUTEX,

} FRAMErr_t;

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

#define FM24CL64_MAX_ADDR    ((uint32_t)0x1FFFul)
#define FRAM_MAX_ADDR_x256   ((uint32_t)0x7FFFul)

#define FRAM_WRITE_ADDR      ((uint8_t)0xA0u)
#define FRAM_READ_ADDR       ((uint8_t)0xA1u)


/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

void    FRAM_Init(void);
uint8_t FRAM_Write(FRAMDev_t FM24xx, uint16_t addr, const void *pvbuf, uint16_t size);
uint8_t FRAM_Read(FRAMDev_t FM24xx, uint16_t addr, void *pvbuf, uint16_t size);
uint8_t FRAM_Erase(FRAMDev_t FM24xx, uint16_t addr, uint16_t size);

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __FM24CLXX_H */

/***************************** END OF FILE *************************************/


