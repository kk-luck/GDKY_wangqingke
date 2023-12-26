/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     uart4.c
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx uart4 driver.
  ******************************************************************************
  */

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "uart4.h"
//#include "lora_wb25.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/**
 * @brief usart run object
 */
typedef struct
{
  volatile uint8_t  rx_valid;
  volatile uint16_t rx_len;
  volatile uint32_t rx_tick;

  uint8_t  rx_buff[128];
} uart4_obj_t;


/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

// ms
#define UART4_FRAME_INTERVAL  50

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

static uart4_obj_t uart4Obj;

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


/**
 * @brief  init
 * @param  none
 * @retval 0 success
 */
void uart4_init(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStruct;

  // PB2 UART4 RX
  // PB3 UART4 TX
  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.pin        = FL_GPIO_PIN_2 | FL_GPIO_PIN_3;
  GPIO_InitStruct.mode       = FL_GPIO_MODE_DIGITAL;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull       = FL_ENABLE;
  GPIO_InitStruct.remapPin   = FL_DISABLE;
  FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  uart4_set_param(4800, 0);

  lib_memset(&uart4Obj, 0, sizeof(uart4Obj));
}



/**
 * @brief  set comunication parameter
 * @param  baud: baudrate
 * @param  parity: 0 none, 1 odd, 2 even
 * @retval 0 success
 */
void uart4_set_param(uint32_t baud, uint8_t parity)
{
  FL_UART_InitTypeDef UART_InitStruct;

  FL_UART_DeInit(UART4);

  FL_UART_StructInit(&UART_InitStruct);
  // UART_InitStruct.clockSrc = FL_CMU_GROUP3_BUSCLK_UART4;
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

  FL_UART_Init(UART4, &UART_InitStruct);

  // 打开接收使能
  FL_UART_EnableRX(UART4);

  // 打开发送使能
  FL_UART_EnableTX(UART4);

  // 接收中断标志
  FL_UART_ReadRXBuff(UART4);

//  FL_UART_EnableRXTimeout(UART4);        //使能接收超时功能
//  FL_UART_WriteRXTimeout(UART4, 5);    //设置超时时间
  FL_UART_ClearFlag_RXBuffTimeout(UART4);  //清接收超时标志  
//  FL_UART_EnableIT_RXTimeout(UART4);     //打开串口接收超时中断
  FL_UART_ClearFlag_FallingEdgeWakeup(UART4); //清串口下降沿唤醒标志
//  FL_UART_EnableFallingEdgeWakeup(UART4);    //使能串口下降沿唤醒
  
  // 打开接收中断
  FL_UART_EnableIT_RXBuffFull(UART4);

  NVIC_EnableIRQ(UART4_IRQn);

}

/**
 * @brief  send data buffer
 * @param  pbuf: data buffer
 * @param  len: data length
 * @retval actual sent data length
 */
uint32_t uart4_send(const uint8_t *pbuf, uint32_t len)
{
  volatile uint16_t len_left;

  if ((NULL == pbuf) || !len)
    return 0;

  // Receive Mode
  uart4_set_rx_stat(1);

  len_left = len;
  while (len_left)
  {
    FL_UART_WriteTXBuff(UART4, *pbuf);

    while (FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(UART4))
    {
      __NOP();
    }

    pbuf++;
    len_left--;
  }

  return (len - len_left);
}


/**
 * @brief  Whether a valid frame was received
 * @param  None
 * @retval 0-Invalid frame, Other is a valid frame was received
 */
uint32_t uart4_is_rx_valid(void)
{
  uint32_t ret = 0;
//  uint32_t cur_ms;

  do
  {
    if (!uart4Obj.rx_len)
      break;

    if (uart4Obj.rx_valid)
    {
      ret = uart4Obj.rx_valid;
      break;
    }

//    cur_ms = SYSGetRunMs();

//    if (cur_ms < uart4Obj.rx_tick)
//    {
//      uart4Obj.rx_tick = cur_ms;
//      break;
//    }

    if (!uart4Obj.rx_valid && uart4Obj.rx_len && (SYSGetTick() - uart4Obj.rx_tick > SYSMsToTick(UART4_FRAME_INTERVAL)))
    {
      // Stop rx
      uart4_set_rx_stat(0);

      uart4Obj.rx_valid = 2;
      ret = uart4Obj.rx_valid;
    }
  } while (0);

  return ret;
}

/**
 * @brief  get receive data max length.
 * @param  none
 * @return max receive data length
 */
uint32_t uart4_get_rx_len_max(void)
{
  return sizeof(uart4Obj.rx_buff);
}

/**
 * @brief  get receive length
 * @param  none
 * @retval receive length
 */
uint32_t uart4_get_rx_len(void)
{
  uint16_t len = 0;

  SYSEnterCritical();
  len = uart4Obj.rx_len;
  SYSExitCritical();

  return len;
}

/**
 * @brief  get receive data buffer point.
 * @param  none
 * @return receive data buffer point
 */
uint8_t *uart4_get_rx_buff(void)
{
  return (void *)uart4Obj.rx_buff;
}


/**
 * @brief  get last receive tick
 * @param  none
 * @retval last receive tick
 */
uint32_t uart4_get_last_rx_ms(void)
{
  uint16_t tick = 0;

  SYSEnterCritical();
  tick = uart4Obj.rx_tick;
  SYSExitCritical();

  return tick;
}



/**
 * @brief  Enable receive
 * @param  stat: 0-Stop rx, 1-Restart rx
 * @retval None
 */
void uart4_set_rx_stat(uint32_t stat)
{
  if (stat)
  {
	#ifdef USE_FULL_ASSERT
		lib_memset(uart4Obj.rx_buff, 0, sizeof(uart4Obj.rx_buff));
	#endif

		uart4Obj.rx_len = 0;
		uart4Obj.rx_valid = 0;

		FL_UART_ReadRXBuff(UART4);
		FL_UART_EnableIT_RXBuffFull(UART4);
  }
  else
  {
		FL_UART_DisableIT_RXBuffFull(UART4);
  }
}




/**
 * @brief  Set sleep status
 * @param  stat: 1-Enter sleep, 0-Exit sleep
 * @retval None
 */
void uart4_set_sleep_stat(uint32_t stat)
{
  if (stat)
  {
		
  }
  else
  {
		uart4_init();
  }
}



/**
 * @brief  interrupt handler
 * @param  none
 * @retval none
 */
void UART4_IRQHandler(void)
{
  uint8_t rx_char;
  uint32_t cur_tick;
  uint16_t u16temp;
  
//  uint32_t counter,RXFlag;


  //下降沿唤醒中断处理
  if((FL_UART_IsEnabledFallingEdgeWakeup(UART4) == 0x01UL) && (FL_UART_IsActiveFlag_FallingEdgeWakeup(UART4) == 0x01UL))
  {
      FL_UART_ClearFlag_FallingEdgeWakeup(UART4);
  }
  
  
  if (FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART4) && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART4)))
  {
    rx_char = FL_UART_ReadRXBuff(UART4);

    if (uart4Obj.rx_valid)
      return;

    cur_tick = SYSGetTickISR();
    uart4Obj.rx_tick = cur_tick;



    if (uart4Obj.rx_len >= sizeof(uart4Obj.rx_buff))
      uart4Obj.rx_len = 0;

    uart4Obj.rx_buff[uart4Obj.rx_len++] = rx_char;
    
    if(rx_char == 0x16)
    {    
      if(uart4Obj.rx_buff[0] == 0x68)
      {
        u16temp = uart4Obj.rx_buff[1];
        u16temp <<= 8;
        u16temp |= uart4Obj.rx_buff[2];
        if(u16temp == uart4Obj.rx_len)
        {
          uart4Obj.rx_valid = 2;

        }        
      }
    }
  }
  else
  {
    rx_char = FL_UART_ReadRXBuff(UART4);
    FL_UART_ClearFlag_FrameError(UART4);
    FL_UART_ClearFlag_ParityError(UART4);
    FL_UART_ClearFlag_RXBuffOverflowError(UART4);
    FL_UART_ClearFlag_RXBuffTimeout(UART4);
    FL_UART_ClearFlag_RXBuffFull(UART4);
  }
}

