/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     cmdproc_at.c
  * @author   ZouZH
  * @version  V1.00
  * @date     18-July-2019
  * @brief    command process routine for AT config.
  ******************************************************************************
  */
#define LOG_TAG "CMD_AT"

/* INCLUDES ------------------------------------------------------------------- */
#include <stdio.h>
#include "includes.h"
#include "rs485.h"
#include "queue.h"
#include "owi.h"
#include "cmdproc_at.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

// General response
const char RSP_OK[]         =    "OK\r\n";
const char RSP_ERROR[]      =    "ERROR\r\n";
const char RSP_OWISUCCESS[] = 	 "SET WOI MODE SUCCESS\r\n";
const char RSP_OWIFAIL[]    =    "SET WOI MODE FAIL\r\n";
const char RSP_CRLF[]		    = 	 "\r\n";

#define WRITE       0X00
#define READ        0X01
uint32_t ATTxDoTick;
owiDataBuff_t owiDataRxBuff;


const uint8_t  OWI_REGISTER_ADDRESS[65] = {0X00,0X02,0X06,0X07,0X08,0X09,0X0A,0X0B,0X12,0X13,0X14,0X30,0X61,0X62,0X6A,0X70,
																					 0XA1,0XA2,0XA3,0XA4,0XA5,0XA6,0XA7,0XA8,0XA9,0XAA,0XAB,0XAC,0XAD,0XAE,0XAF,0XB0,
																					 0XB1,0XB2,0XB3,0XB4,0XB5,0XB6,0XB7,0XB8,0XB9,0XBA,0XBB,0XBC,0XBD,0XBE,0XBF,0XC0,
																					 0XC1,0XC2,0XC3,0XC4,0XC5,0XC6,0XC7,0XC8,0XC9,0XCA,0XCB,0XCC,0XCD,0XCE,0XD7,0XD8,
																					 0XD9};
const uint8_t  OWI_REGISTER_INIT[46]   =  {0XA1,0X08,0XA2,0X02,0XA3,0XF0,0XA4,0X92,0XA5,0X00,0XA6,0X52,0XA9,0X15,0XAA,0X00,
																					 0XAF,0XFF,0XB0,0XFF,0XB5,0XE6,0XB6,0X11,0XB7,0X00,0XBC,0X01,0XC5,0X40,0XC6,0XA7,
																					 0XC7,0X3C,0XC8,0XBF,0XC9,0XFF,0XCA,0XFF,0XCB,0X6D,0XCE,0X40,0XD7,0X40};
																					 
typedef struct
{
#define CMD_BUF_LEN   256
  uint8_t       txBuff[CMD_BUF_LEN];   /*!< 发送缓存 */
} CmdRunObj_t;

uint8_t pRspStr[256];
// AT+LIST
const char RSP_AT_LIST[]  = "\r\n\
AT+LIST                 List all AT Command\r\n\
AT+SHOW                 Show all param\r\n\
AT+SENINIT              Sensor initialization\r\n\
AT+OWIMODE              Set owi mode\r\n\
AT+WRITEPARAM           Write registers\r\n\
AT+READPARAM            Read registers\r\n\
AT+QUITOWI              Quit OWI mode\r\n\
AT+WRITEEPROM           Write EEPROM\r\n\
AT+READDATA             Read temperature and pressure\r\n\
";

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */
CmdRunObj_t s_tCmd;

/* LOCAL FUNCTIONS ------------------------------------------------------------ */
uint8_t CmdSendStr(const char *str);
uint8_t CmdParseAT(char *str, uint16_t len);

uint8_t CmdPrintOwi(uint8_t regAdd);



/**
 * @brief  8bit to dec
 * @param  8bit1
 * @param  8bit2
 * @param  8bit2
 * @retval dec
 */
float bit2dec(uint8_t p1,uint8_t p2,uint8_t p3)
{
	uint8_t symbol = p1&0x80;
	float ret;
	if(symbol == 0)
	{
		ret = ((float)p1)*pow(2,16)+((float)p2)*pow(2,8)+((float)p3);
	}
	else
	{
		ret = 0 -((float)(~p1&0x7f)*pow(2,16)+((float)(~p2&0XFF))*pow(2,8)+((float)(~p3&0XFF))+1);
	}
	
	return ret;
}

/**
 * @brief  Parse AT command string
 * @param  str strings
 * @param  len length
 * @retval 0 Success, Other is error
 */
uint8_t CmdParseAT(char *str, uint16_t len)
{
	owiDataBuff_t owiDataTxBuff;
	uint32_t ATINITDoTick;
	uint32_t QUITOWIDoTick;
	uint8_t value[2];
	
  // COMMIT CONFIG
  if (!strncasecmp(str, "COMMIT CONFIG", 13))
  {
    CmdSendStr("+Enter Config: OK!\r\n");
    return 0;
  }
  // 调试指令AT
  if (!strncasecmp(str, "AT\r", 3))
	{
		CmdSendStr(RSP_OK);
	}
  // AT+LIST
  else if (!strncasecmp(str, "AT+LIST", 7))
  {
    CmdSendStr(RSP_AT_LIST);
    CmdSendStr(RSP_OK);
  }
	// AT+OWIMODE
	else if (!strncasecmp(str, "AT+OWIMODE", 10))
  {
    owiMode();
			
		if(CmdPrintOwi(0XD9) == 0X01)CmdSendStr(RSP_OWISUCCESS);
		else
		{
			CmdSendStr(RSP_OWIFAIL);
		}
		
		CmdSendStr(RSP_CRLF);
  }
  // AT+SHOW
  else if (!strncasecmp(str, "AT+SHOW", 7))
  {
		for(int i = 0;i<65;i++)
		{
			if(CmdPrintOwi(OWI_REGISTER_ADDRESS[i]) == 0X01)
			{
				sprintf((char *)pRspStr, "Register address:%02X    Register value:%02X \r\n", owiDataRxBuff.address,owiDataRxBuff.rxData);
				CmdSendStr((char *)pRspStr);
			}
			else
			{
				sprintf((char *)pRspStr, "DATA IS NULL\r\n");
				CmdSendStr((char *)pRspStr);
			}
		}
  }
	// AT+SENINIT
  else if (!strncasecmp(str, "AT+SENINIT", 10))
  {
		sprintf((char *)pRspStr, "Initialize is begin.............................................................\r\n");
		CmdSendStr((char *)pRspStr);
		for(int i = 0;i<23;i++)
		{
			owiReviseEEPROM(OWI_REGISTER_INIT[2*i],OWI_REGISTER_INIT[2*i+1]);
			
			sprintf((char *)pRspStr, "Initialization register address:%02X\r\n", OWI_REGISTER_INIT[2*i]);
			CmdSendStr((char *)pRspStr);
			
			ATINITDoTick = SYSGetRunMs();
			while(!JudgmentTimeout(SYSGetRunMs(),ATINITDoTick,2000)){}//延时1/2毫秒
			owiMode();
		}
		sprintf((char *)pRspStr, "Initialize is finish.............................................................\r\n");
		CmdSendStr((char *)pRspStr);
  }	
	// AT+WRITEPARAM
  else if (!strncasecmp(str, "AT+WRITEPARAM", 13))
  {
    str += 13;

    if (('=' == *str) && strncasecmp(str, "=?", 2))
    {
			str += 1;
			lib_atoint8_t(str,value);
			owiDataTxBuff.address = value[0];
			owiDataTxBuff.len = 0X01;
			owiDataTxBuff.pbuf = value[1];
			owiDataTxBuff.readOrWrite = WRITE;
			
			owiWriteData(owiDataTxBuff);
			owiEventPoll();
    }
		if(CmdPrintOwi(value[0]) == 0X01)
		{
			sprintf((char *)pRspStr, "Register address:%02X    Register value:%02X \r\n", owiDataRxBuff.address,owiDataRxBuff.rxData);
			CmdSendStr((char *)pRspStr);
		}
		else
		{
			sprintf((char *)pRspStr, "DATA IS NULL\r\n");
			CmdSendStr((char *)pRspStr);
		}
  }
	// AT+READPARAM
	else if (!strncasecmp(str, "AT+READPARAM", 12))
	{
		str += 12;
		
		if (('=' == *str) && strncasecmp(str, "=?", 2))
    {
			lib_atoint8_t(str + 1,value);
			if(CmdPrintOwi(value[0]) == 0X01)
			{
				sprintf((char *)pRspStr, "Register address:%02X    Register value:%02X \r\n", owiDataRxBuff.address,owiDataRxBuff.rxData);
				CmdSendStr((char *)pRspStr);
			}
			else
			{
				sprintf((char *)pRspStr, "DATA IS NULL\r\n");
				CmdSendStr((char *)pRspStr);
			}
    }
	}
	// AT+QUITOWI
	else if (!strncasecmp(str, "AT+QUITOWI", 10))
	{
		str += 10;
		
		if (('=' == *str) && strncasecmp(str, "=?", 2))
    {
			str += 1;
			lib_atoint8_t(str,value);
			owiDataTxBuff.address = 0X62;
			owiDataTxBuff.len = 0X01;
			owiDataTxBuff.pbuf = value[0];
			owiDataTxBuff.readOrWrite = WRITE;
			
			owiWriteData(owiDataTxBuff);
			owiEventPoll();
			
			QUITOWIDoTick = SYSGetRunMs();
			while(!JudgmentTimeout(SYSGetRunMs(),QUITOWIDoTick,100)){}//延时100毫秒
			if(CmdPrintOwi(0X62) == 0X01)
			{
				sprintf((char *)pRspStr, "Quit owi : %2f s\r\n", ((float)owiDataRxBuff.rxData)/20);
				CmdSendStr((char *)pRspStr);
			}
			owiDataTxBuff.address = 0X61;
			owiDataTxBuff.pbuf = 0X5D;
			
			owiWriteData(owiDataTxBuff);
			owiEventPoll();
    }
	}
	//AT+WRITEEPROM
	else if (!strncasecmp(str, "AT+WRITEEPROM", 13))
  {
    str += 13;

    if (('=' == *str) && strncasecmp(str, "=?", 2))
    {
			str += 1;
			lib_atoint8_t(str,value);
			owiReviseEEPROM(value[0],value[1]);
			
			
    }
		if(CmdPrintOwi(value[0]) == 0X01)
		{
			sprintf((char *)pRspStr, "Register address:%02X    Register value:%02X \r\n", owiDataRxBuff.address,owiDataRxBuff.rxData);
			CmdSendStr((char *)pRspStr);
			owiSend(0X62,0X00);
			owiSend(0X61,0X5D);
		}
		else
		{
			sprintf((char *)pRspStr, "DATA IS NULL\r\n");
			CmdSendStr((char *)pRspStr);
		}
  }
	// AT+READDATA
	else if(!strncasecmp(str, "AT+READDATA", 11))
	{
		str += 11;
		uint8_t P1 , P2 , P3 , T1 , T2;

		if(CmdPrintOwi(0X06) == 0X01)   P1 = owiDataRxBuff.rxData;
		
		
		if(CmdPrintOwi(0X07) == 0X01)   P2 = owiDataRxBuff.rxData;
		
		
		if(CmdPrintOwi(0X08) == 0X01)   P3 = owiDataRxBuff.rxData;
		
		
		if(CmdPrintOwi(0X12) == 0X01)   T1 = owiDataRxBuff.rxData;
		
		
		if(CmdPrintOwi(0X13) == 0X01)   T2 = owiDataRxBuff.rxData;
		
		
		float press = bit2dec(P1,P2,P3)/(pow(10,5));
		
		int temp = ((int)T1)*pow(2,8)+(int)T2;
	
		sprintf((char *)pRspStr, "Pressure:%5f bar    Analog pressure:%d P\r\n",press , temp);
		CmdSendStr((char *)pRspStr);
		
	}
	else 
	{
		CmdSendStr("Invalid commands");
	}
	return 0;
}
/**
 * @brief  Print lora info
 * @param  regAdd:Register address
 * @retval None
 */
uint8_t  CmdPrintOwi(uint8_t regAdd)
{
	uint8_t ret = 0X00;
	uint32_t ATRxDoTick;
	owiDataRxBuff.address = regAdd;
	owiDataRxBuff.len = 0x01;
	owiDataRxBuff.readOrWrite = READ;
	owiWriteData(owiDataRxBuff);
			
	owiEventPoll();
			
	ATRxDoTick = SYSGetRunMs();
	while(!JudgmentTimeout(SYSGetRunMs(),ATRxDoTick,100)){}//延时100毫秒
				
	owiRxPoll();
				
	if(xQueueReceive(getOwiRxQueue(),&owiDataRxBuff,0))
	{
		ret = 0X01;
	}
	return ret;
}

/**
 * @brief  Send String
 * @param  str strings
 * @retval 0Success
 */
uint8_t CmdSendStr(const char *str)
{
  uint16_t len;
  uint16_t sndlen;

  if (NULL == str)
    return 0;

  if (0 == (len = strlen(str)))
    return 0;
  sndlen = RS485_Send((const uint8_t *)str, len);


  return (len == sndlen) ? 0 : 1;
}

