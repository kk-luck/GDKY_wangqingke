/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     uart1.c
  * @author   ZouZH
  * @version  V3.00
  * @date     17-Jan-2022
  * @brief    fm33lg0xx uart1 driver.
  ******************************************************************************
  */

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "uart1.h"
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
} uart1_obj_t;


/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

// ms
#define UART1_FRAME_INTERVAL  50

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

static uart1_obj_t uart1Obj;

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


/**
 * @brief  init
 * @param  none
 * @retval 0 success
 */
void uart1_init(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStruct;

  // PB13 UART1 RX
  // PB14 UART1 TX
  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.pin        = FL_GPIO_PIN_13 | FL_GPIO_PIN_14;
  GPIO_InitStruct.mode       = FL_GPIO_MODE_DIGITAL;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull       = FL_ENABLE;
  GPIO_InitStruct.remapPin   = FL_DISABLE;
  FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  uart1_set_param(115200, 0);

  lib_memset(&uart1Obj, 0, sizeof(uart1Obj));
}



/**
 * @brief  set comunication parameter
 * @param  baud: baudrate
 * @param  parity: 0 none, 1 odd, 2 even
 * @retval 0 success
 */
void uart1_set_param(uint32_t baud, uint8_t parity)
{
  FL_UART_InitTypeDef UART_InitStruct;

  FL_UART_DeInit(UART1);

  FL_UART_StructInit(&UART_InitStruct);
  UART_InitStruct.clockSrc = FL_CMU_UART1_CLK_SOURCE_RCHF;
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

//  FL_UART_EnableRXTimeout(UART1);        //使能接收超时功能
//  FL_UART_WriteRXTimeout(UART1, 5);    //设置超时时间
  FL_UART_ClearFlag_RXBuffTimeout(UART1);  //清接收超时标志  
//  FL_UART_EnableIT_RXTimeout(UART1);     //打开串口接收超时中断
  FL_UART_ClearFlag_FallingEdgeWakeup(UART1); //清串口下降沿唤醒标志
//  FL_UART_EnableFallingEdgeWakeup(UART1);    //使能串口下降沿唤醒
  
  // 打开接收中断
  FL_UART_EnableIT_RXBuffFull(UART1);

  NVIC_EnableIRQ(UART1_IRQn);

}

/**
 * @brief  send data buffer
 * @param  pbuf: data buffer
 * @param  len: data length
 * @retval actual sent data length
 */
uint32_t uart1_send(const uint8_t *pbuf, uint32_t len)
{
  volatile uint16_t len_left;

  if ((NULL == pbuf) || !len)
    return 0;

  // Receive Mode
  uart1_set_rx_stat(1);

  len_left = len;
  while (len_left)
  {
    FL_UART_WriteTXBuff(UART1, *pbuf);

    while (FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(UART1))
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
uint32_t uart1_is_rx_valid(void)
{
  uint32_t ret = 0;
//  uint32_t cur_ms;

  do
  {
    if (!uart1Obj.rx_len)
      break;

    if (uart1Obj.rx_valid)
    {
      ret = uart1Obj.rx_valid;
      break;
    }

//    cur_ms = SYSGetRunMs();

//    if (cur_ms < uart1Obj.rx_tick)
//    {
//      uart1Obj.rx_tick = cur_ms;
//      break;
//    }

    if (!uart1Obj.rx_valid && uart1Obj.rx_len && (SYSGetTick() - uart1Obj.rx_tick > SYSMsToTick(UART1_FRAME_INTERVAL)))
    {
      // Stop rx
      uart1_set_rx_stat(0);

      uart1Obj.rx_valid = 2;
      ret = uart1Obj.rx_valid;
    }
  } while (0);

  return ret;
}

/**
 * @brief  get receive data max length.
 * @param  none
 * @return max receive data length
 */
uint32_t uart1_get_rx_len_max(void)
{
  return sizeof(uart1Obj.rx_buff);
}

/**
 * @brief  get receive length
 * @param  none
 * @retval receive length
 */
uint32_t uart1_get_rx_len(void)
{
  uint16_t len = 0;

  SYSEnterCritical();
  len = uart1Obj.rx_len;
  SYSExitCritical();

  return len;
}

/**
 * @brief  get receive data buffer point.
 * @param  none
 * @return receive data buffer point
 */
uint8_t *uart1_get_rx_buff(void)
{
  return (void *)uart1Obj.rx_buff;
}


/**
 * @brief  get last receive tick
 * @param  none
 * @retval last receive tick
 */
uint32_t uart1_get_last_rx_ms(void)
{
  uint16_t tick = 0;

  SYSEnterCritical();
  tick = uart1Obj.rx_tick;
  SYSExitCritical();

  return tick;
}



/**
 * @brief  Enable receive
 * @param  stat: 0-Stop rx, 1-Restart rx
 * @retval None
 */
void uart1_set_rx_stat(uint32_t stat)
{
  if (stat)
  {
	#ifdef USE_FULL_ASSERT
		lib_memset(uart1Obj.rx_buff, 0, sizeof(uart1Obj.rx_buff));
	#endif

		uart1Obj.rx_len = 0;
		uart1Obj.rx_valid = 0;

		FL_UART_ReadRXBuff(UART1);
		FL_UART_EnableIT_RXBuffFull(UART1);
  }
  else
  {
		FL_UART_DisableIT_RXBuffFull(UART1);
  }
}




/**
 * @brief  Set sleep status
 * @param  stat: 1-Enter sleep, 0-Exit sleep
 * @retval None
 */
void uart1_set_sleep_stat(uint32_t stat)
{
  if (stat)
  {
		
  }
  else
  {
		uart1_init();
  }
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
  uint16_t u16temp;
  
//  uint32_t counter,RXFlag;


  //下降沿唤醒中断处理
  if((FL_UART_IsEnabledFallingEdgeWakeup(UART1) == 0x01UL) && (FL_UART_IsActiveFlag_FallingEdgeWakeup(UART1) == 0x01UL))
  {
      FL_UART_ClearFlag_FallingEdgeWakeup(UART1);
    
//      wakeUpCtrlTaskFromISR();   //中断唤醒控制任务
    
//      do
//      {
//          RXFlag = FL_UART_IsActiveFlag_RXBuffFull(UART1);
//          counter++;
//      } while((counter <= 1000000U) && (RXFlag == 0U));           //等待转换完成
  }
  
  
  if (FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART1) && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART1)))
  {
    rx_char = FL_UART_ReadRXBuff(UART1);

    if (uart1Obj.rx_valid)
      return;

    cur_tick = SYSGetTickISR();

//    if ((cur_tick > uart1Obj.rx_tick) && ((cur_tick - uart1Obj.rx_tick) > SYSMsToTick(UART1_FRAME_INTERVAL)))     //由于接收前几个字节的时候已经获取或时间戳，但接收完前几个字节后，低功耗唤醒程序补偿了嘀嗒时间戳，导致接收后几个自己时超时，所以去掉时间戳判断。
//      uart1Obj.rx_len = 0;

    uart1Obj.rx_tick = cur_tick;

    //if (0xFE != uart1Obj.rx_buff[0])
    //  uart1Obj.rx_len = 0;

    if (uart1Obj.rx_len >= sizeof(uart1Obj.rx_buff))
      uart1Obj.rx_len = 0;

    uart1Obj.rx_buff[uart1Obj.rx_len++] = rx_char;
    
    if(rx_char == 0x16)
    {
//      if((uart1Obj.rx_len+1) == uart1Obj.rx_buff[1])
//      {
//        u16temp = uart1Obj.rx_len;
//        for(;u16temp>0;u16temp--)
//        {
//          uart1Obj.rx_buff[u16temp] = uart1Obj.rx_buff[u16temp-1];
//        }
//        uart1Obj.rx_buff[0] = 0x68;
//        uart1Obj.rx_len++;
//      }
//      else if((uart1Obj.rx_len+2) == uart1Obj.rx_buff[0])
//      {
//        u16temp = uart1Obj.rx_len+1;
//        for(;u16temp>0;u16temp--)
//        {
//          uart1Obj.rx_buff[u16temp] = uart1Obj.rx_buff[u16temp-1];
//        }
//        uart1Obj.rx_buff[0] = 0x68;
//        uart1Obj.rx_buff[1] = 0x00;
//        uart1Obj.rx_len+=2;
//      }
//      wakeUpCtrlTaskFromISR();   //中断唤醒控制任务
      
      if(uart1Obj.rx_buff[0] == 0x68)
      {
        u16temp = uart1Obj.rx_buff[1];
        u16temp <<= 8;
        u16temp |= uart1Obj.rx_buff[2];
        if(u16temp == uart1Obj.rx_len)
        {
          uart1Obj.rx_valid = 2;

//          wakeUpCtrlTaskFromISR();   //中断唤醒控制任务
//          wakeUpAllTaskFromISR();  //唤醒所有任务
        }        
      }
    }
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
  
//  if(FL_SET == FL_UART_IsActiveFlag_RXBuffTimeout(UART1))     //接收超时中断
//  {
//    FL_UART_ClearFlag_RXBuffTimeout(UART1);  //清接收超时标志
//    
//    if(uart1Obj.rx_len > 3)
//    {
//      wakeUpAllTaskFromISR();  //唤醒所有任务
////      wakeUpAllTaskFromISR();  //唤醒所有任务
////    wakeUpAllTaskFromISR();  //唤醒所有任务      
//    }
//  }
}

