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
  CMD_MODE                  = (0x3000),              /*!<����ģʽ������EEPROM�Ĵ���ֻ��������ģʽ��д��*/
	RESERVED                  = (0x3001),              /*!<0x01/0x02: ����ģʽ*/
	CONT_OPERA                = (0x3003),              /*!<��������ģʽ*/
	EEPROM                    = (0x3033),              /*!<����EEPROM��дģʽ*/
	
	/*!<QUIT_OWI*/
  PERPETUAL_QUIT_OWI        = (0x6100),              /*!<�����˳�OWI*/
	TEMPORARY_QUIT_OWI        = (0x615D),              /*!<��ʱ�˳�OWI*/
	
	/*!<QUIT_OWI_CNT*/
	TEMPORARY_QUIT_OWI_50MS   = (0x6201),              /*!<��ʱ�˳�OWI 50MS(0x01��50ms��0x02��100ms �� 0xFF��12.8s)*/
	
	/*!<EE_PROG*/
	EEPROM_START              = (0x6A1E),              /*!<��ʼEEPROM��¼����¼��ɺ��Զ��ص�0*/
	
	/*!<EE_PROG*/
	VDD_CHECK                 = (0x7001),              /*!<TADC����ǿ��ΪVDD/2,Ĭ��ֵ0x7000>*/
	
} COM_OWI_t;