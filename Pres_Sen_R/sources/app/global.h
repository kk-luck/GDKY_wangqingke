/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     combus.h
  * @author   ZouZH
  * @version  V1.02
  * @date     30-Nov-2019
  * @brief    Multi-communication serial bus management.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>
#include "includes.h"
/**
 * @addtogroup ComBus
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/**
 *  系统参数 64Bytes
 */
typedef struct
{
	uint8_t devNum[4];  //设备号
	uint8_t heatingSeason[4];    //采暖季	
} SysParam_t;


	
/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __COMBUS_H */

/**
 * @}
 */

/***************************** END OF FILE ************************************/

