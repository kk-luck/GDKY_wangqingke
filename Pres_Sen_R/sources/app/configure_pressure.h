/**
  ******************************************************************************
  *               Copyright(C) 2018-2028 GDKY All Rights Reserved
  *
  * @file     owi.h
  * @author   WangQk
  * @version  V1.00
  * @date     2023-09-15
  * @brief    OWI network maintenance.
  ******************************************************************************
  * @history
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __CONFIGURE_PRESSURE_H
#define __CONFIGURE_PRESSURE_H
#endif

#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus */


/* INCLUDES ----------------------------------------------------------------- */
#include <stdint.h>

typedef enum
{
	/*!<COMMAND*/
  CMD_MODE                  = (0x3000),              /*!<命令模式，所有EEPROM寄存器只能在命令模式下写入*/
	RESERVED                  = (0x3001),              /*!<0x01/0x02: 保留模式*/
	CONT_OPERA                = (0x3003),              /*!<连续工作模式*/
	EEPROM                    = (0x3033),              /*!<进入EEPROM烧写模式*/
	
	/*!<QUIT_OWI*/
  PERPETUAL_QUIT_OWI        = (0x6100),              /*!<永久退出OWI*/
	TEMPORARY_QUIT_OWI        = (0x615D),              /*!<临时退出OWI*/
	
	/*!<QUIT_OWI_CNT*/
	TEMPORARY_QUIT_OWI_50MS   = (0x6201),              /*!<临时退出OWI 50MS(0x01：50ms，0x02：100ms … 0xFF：12.8s)*/
	
	/*!<EE_PROG*/
	EEPROM_START              = (0x6A1E),              /*!<开始EEPROM烧录，烧录完成后，自动回到0*/
	
	/*!<EE_PROG*/
	VDD_CHECK                 = (0x7001),              /*!<TADC输入强制为VDD/2,默认值0x7000>*/
	
} COM_OWI_t;