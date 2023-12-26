/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     rs485.c
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx RS485 driver.
  ******************************************************************************
  */


/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "rs485.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */
QueueHandle_t RS485RxQueue;    //数据接收队列
/**
 * RS485 port and pin
 */
#define PORT_NRE     GPIOB
#define PIN_NRE      FL_GPIO_PIN_12

#define RS485_POWER_H()
#define RS485_POWER_L()
#define RS485_POWER_Read()

#define RS485_NRE_H()     FL_GPIO_SetOutputPin(PORT_NRE, PIN_NRE)
#define RS485_NRE_L()     FL_GPIO_ResetOutputPin(PORT_NRE, PIN_NRE)
#define RS485_NRE_Read()  FL_GPIO_GetInputPin(PORT_NRE, PIN_NRE)


/* CONSTANTS  ----------------------------------------------------------------- */

// ms
#define RS485_FRAME_INTERVAL  50

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

static rs485_obj_t rs485Obj;
uint32_t EnableRxDoTick;


/* LOCAL FUNCTIONS ------------------------------------------------------------ */


/**
 * @brief  init
 * @param  none
 * @retval 0 success
 */
uint8_t RS485_Init(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStruct;

  // PC2 UART1 RX
  // PC3 UART1 TX
  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.pin        = FL_GPIO_PIN_2;
  GPIO_InitStruct.mode       = FL_GPIO_MODE_DIGITAL;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull       = FL_ENABLE;
  GPIO_InitStruct.remapPin   = FL_DISABLE;
  FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.pin        = FL_GPIO_PIN_3;
  GPIO_InitStruct.pull       = FL_DISABLE;
  FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // RS485-RE  0-Rx
  GPIO_InitStruct.pin        = PIN_NRE;
  GPIO_InitStruct.mode       = FL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull       = FL_ENABLE;
  GPIO_InitStruct.remapPin   = FL_DISABLE;
  FL_GPIO_Init(PORT_NRE, &GPIO_InitStruct);
  RS485_NRE_L();

  RS485_SetParam(115200, 0);

  lib_memset(&rs485Obj, 0, sizeof(rs485Obj));

  return 0;
}


/**
 * @brief  set comunication parameter
 * @param  baud: baudrate
 * @param  parity: 0 none, 1 odd, 2 even
 * @retval 0 success
 */
uint8_t RS485_SetParam(uint32_t baud, uint8_t parity)
{
  FL_UART_InitTypeDef UART_InitStruct;

  FL_UART_DeInit(UART1);             //复位UART 外设寄存器值为复位值

  FL_UART_StructInit(&UART_InitStruct);
  UART_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
  UART_InitStruct.baudRate = baud;
  UART_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;

  if (0 == parity)
  {
    UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART_InitStruct.parity = FL_UART_PARITY_NONE;
  }
  else if (1 == parity)
  {
    UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART_InitStruct.parity = FL_UART_PARITY_ODD;
  }
  else if (2 == parity)
  {
    UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART_InitStruct.parity = FL_UART_PARITY_EVEN;
  }
  else
  {
    UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_9B;
    UART_InitStruct.parity = FL_UART_PARITY_NONE;
  }

  FL_UART_Init(UART1, &UART_InitStruct);

  // 打开接收使能
  FL_UART_EnableRX(UART1);

  // 打开发送使能
  FL_UART_EnableTX(UART1);

  // 接收中断标志
  FL_UART_ReadRXBuff(UART1);

  // 打开接收中断
  FL_UART_EnableIT_RXBuffFull(UART1);
	
	NVIC_EnableIRQ(UART1_IRQn);

  return 0;
}



/**
 * @brief  send data buffer
 * @param  pbuf: data buffer
 * @param  len: data length
 * @retval actual sent data length
 */
uint16_t RS485_Send(const uint8_t *pbuf, uint16_t len)
{
  volatile int len_left;

  if ((NULL == pbuf) || !len)
    return 0;

  len_left = len;
  // Send Mode
  RS485_NRE_H();

  for (volatile uint8_t i = 0; i < 32; i++)
  {
    __NOP();
  }

  while (len_left)
  {
    FL_UART_WriteTXBuff(UART1, *pbuf);
    while (FL_SET != FL_UART_IsActiveFlag_TXShiftBuffEmpty(UART1))
    {
      __NOP();
    }

    pbuf++;
    len_left--;
  }
  // Receive Mode
  RS485_EnableRx();

  return (len - len_left);
}


/**
 * @brief  query for valid packets received
 * @param  none
 * @retval 1 have one valid frame
 */
uint8_t RS485_CheckRxPoll(void)
{
  uint8_t ret = 0;
//	while(!JudgmentTimeout(SYSGetRunMs(),IRQPollDoTick,500))
//	{
//		UART1_IRQHandler();
//	}
//	IRQPollDoTick = SYSGetRunMs();
  SYSEnterCritical();

  if (!rs485Obj.rx_valid && rs485Obj.rx_len && (SYSGetTick() - rs485Obj.rx_tick > SYSMsToTick(RS485_FRAME_INTERVAL)))
  {  
		rs485Obj.rx_valid = 1;
		if (NULL == RS485RxQueue)RS485RxQueue = xQueueCreate(5, sizeof(rs485_obj_t));
		xQueueSend(RS485RxQueue,&rs485Obj,0);
	}

  ret = rs485Obj.rx_valid;
  SYSExitCritical();

  return ret;
}


/**
 * @brief  get receive data Queue.
 * @param  none
 * @return receive data buffer point
 */
QueueHandle_t RS485_GetRxQueue(void)
{
  return RS485RxQueue;
}
/**
 * @brief  get receive data max length.
 * @param  none
 * @return max receive data length
 */

uint16_t RS485_MaxRxLen(void)
{
  return sizeof(rs485Obj.rx_buff);
}

/**
 * @brief  get receive length
 * @param  none
 * @retval receive length
 */
uint16_t RS485_GetRxLen(void)
{
  uint16_t len = 0;

  SYSEnterCritical();
  len = rs485Obj.rx_len;
  SYSExitCritical();

  return len;
}

/**
 * @brief  get receive data buffer point.
 * @param  none
 * @return receive data buffer point
 */
void *RS485_GetRxBuffer(void)
{
  return (void *)rs485Obj.rx_buff;
}


/**
 * @brief  get last receive tick
 * @param  none
 * @retval last receive tick
 */
uint32_t RS485_LastRxTick(void)
{
  uint16_t tick = 0;

  SYSEnterCritical();
  tick = rs485Obj.rx_tick;
  SYSExitCritical();

  return tick;
}


/**
 * @brief  enable receive
 * @param  none
 * @retval none
 */
void RS485_EnableRx(void)
{
#ifdef USE_FULL_ASSERT
  lib_memset(rs485Obj.rx_buff, 0, sizeof(rs485Obj.rx_buff));
#endif

  rs485Obj.rx_len = 0;
  rs485Obj.rx_valid = 0;

  RS485_NRE_L();
  FL_UART_ReadRXBuff(UART1); //UART 接收 1 字节的数据
  FL_UART_EnableIT_RXBuffFull(UART1);//启用 UART 接收缓冲区完全中断
}


/**
 * @brief  disable receive
 * @param  none
 * @retval none
 */
void RS485_DisableRx(void)
{
  FL_UART_DisableIT_RXBuffFull(UART1);
}


/**
 * @brief  set power on or off
 * @param  stat: 0 Off, 1 On
 * @retval none
 */
void RS485_SetPower(uint8_t stat)
{
  if (stat)
    RS485_POWER_L();
  else
    RS485_POWER_H();
}

/**
 * @brief  set enter sleep
 * @param  none
 * @retval none
 */
void RS485_EnterSleep(void)
{
}


/**
 * @brief  exit sleep
 * @param  none
 * @retval none
 */
void RS485_ExitSleep(void)
{
  RS485_Init();
}
/**
 * @brief  interrupt handler
 * @param  none
 * @retval none
 */
void UART1_IRQHandler(void)
{
  uint8_t rx_char;
  uint32_t cur_tick;

  if ((FL_UART_IsActiveFlag_RXBuffFull(UART1) == FL_SET)  &&  FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART1))
  {
    rx_char = FL_UART_ReadRXBuff(UART1);

    if (rs485Obj.rx_valid)
      return;

    cur_tick = SYSGetTickISR();

    if (rs485Obj.rx_len && (cur_tick > rs485Obj.rx_tick) && ((cur_tick - rs485Obj.rx_tick) > SYSMsToTick(RS485_FRAME_INTERVAL)))
      rs485Obj.rx_len = 0;

    rs485Obj.rx_tick = cur_tick;

    //if (0x68 != rs485Obj.rx_buff[0])
    //  rs485Obj.rx_len = 0;

    if (rs485Obj.rx_len >= sizeof(rs485Obj.rx_buff))
      rs485Obj.rx_len = 0;

    rs485Obj.rx_buff[rs485Obj.rx_len++] = rx_char;
  }
	else
  {
    rx_char = FL_UART_ReadRXBuff(UART1);
    FL_UART_ClearFlag_FrameError(UART1);
    FL_UART_ClearFlag_ParityError(UART1);
    FL_UART_ClearFlag_RXBuffOverflowError(UART1);
    FL_UART_ClearFlag_RXBuffTimeout(UART1);
    FL_UART_ClearFlag_RXBuffFull(UART1);
  }
}
/**
 * @brief  初始化结构体
 * @param  none
 * @retval none
 */
void RS485_memset(void)
{
  rs485Obj.rx_len = 0;
	rs485Obj.rx_tick = 0;
	rs485Obj.rx_valid = 0;
}

