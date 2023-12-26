/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file    main.c
  * @author  TianRuidong
  * @version V1.03
  * @date    2022-11-30
  * @brief   main
  ******************************************************************************
  */

/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "sysClock.h"
#include "watdog.h"
#include "sys_tick.h"
#include "owi.h"
#include "SEGGER_RTT.h"
#include "cmdproc_at.h"


/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */

static rs485_obj_t rs485Obj;


//字符串转为字符指针
char* str_to_c(char* s,int n)
{
	char *result = &s[0];
	for (int i = 1; i < n + 1; i++)
	{
		*(result + i) = s[i];
	}
	return result;
}

uint32_t owiModeDoTick;
uint32_t owiDoTick;
const uint8_t  REGISTER_ADDRESS[1] = {0XFF};
owiDataBuff_t owiDataBuff;

int main(void)
{
	sysClockInit();
	
	SysMicroSecondTickInit();   //系统微秒时钟初始化
	SysSecondTickInit();        //秒嘀嗒定时器初始化
	owiInit();
	RS485_Init();
	// watdogInit();   //看门狗初始化
	
	
	SEGGER_RTT_Init();
	SEGGER_RTT_printf(0,"RS485 Receive data:\n");
	while(1)
	{
		
		if (RS485_CheckRxPoll())
		{
			xQueueReceive(RS485_GetRxQueue(), &rs485Obj, 0);
			RS485_memset();
			CmdParseAT((char *)rs485Obj.rx_buff,rs485Obj.rx_len);	
		}
		owiEventPoll();
		
		//feedDog();     //喂狗
	}
}


