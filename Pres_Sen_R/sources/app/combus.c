/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     combus.c
  * @author   ZouZH
  * @version  V1.02
  * @date     30-Nov-2019
  * @brief    Multi-communication serial bus management.
  ******************************************************************************
  * @history
  */
#define LOG_TAG "BUS"

/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "combus.h"


#include "uart0.h"
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "uart4.h"
#include "uart5.h"
/**
 * @defgroup ComBus
 * @brief 总线通讯通用接口
 * @{
 */

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

#define BUS_FLAG_BIT_NUM  (8 * sizeof(BUS_FLAG_TYPE_t))

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */
static BusObj_t s_tBusObj[BUS_NUM];

/* LOCAL FUNCTIONS ---------------------------------------------------------- */


/**
 * @brief  Init bus
 * @param  BUS_x: Bus Type
 * @retval 0 Success
 */
BusError_t BusInit(BusType_t BUS_x)
{
  switch (BUS_x)
  {
    case BUS_UART:
      SYSEnterCritical();
      //lib_memset(&s_tBusObj[BUS_UART], 0, sizeof(BusObj_t));
      SYSExitCritical();

      //uart1_init();
      break;

    case BUS_RS485:
      SYSEnterCritical();
      lib_memset(&s_tBusObj[BUS_RS485], 0, sizeof(BusObj_t));
      SYSExitCritical();

      //uart1_init();
      break;

    case BUS_MBUS:
      SYSEnterCritical();
      //lib_memset(&s_tBusObj[BUS_MBUS], 0, sizeof(BusObj_t));
      SYSExitCritical();

      uart4_init();
      break;

    case BUS_BLE:
      #if(BLUETOOTH_IS_ENABLE == 1)
      {
        SYSEnterCritical();
        lib_memset(&s_tBusObj[BUS_BLE], 0, sizeof(BusObj_t));
        SYSExitCritical();

        uart3_init();
      }
      #endif 
      break;
		
    case BUS_LORA:
      SYSEnterCritical();
      //lib_memset(&s_tBusObj[BUS_UART], 0, sizeof(BusObj_t));
      SYSExitCritical();

      uart0_init();
      break;

    default:
      SYSEnterCritical();
      lib_memset(s_tBusObj, 0, sizeof(s_tBusObj));
      SYSExitCritical();

      uart0_init();
      #if(BLUETOOTH_IS_ENABLE == 1)
      {
        uart3_init();
      }
      #endif 
      uart4_init();
      //uart4_init();
      break;
  }

  return BUS_OK;
}

/**
 * @brief  Bus time division multiplex access poll   //总线抢占轮询（确定把总线使用权给哪个任务）
 * @param  None
 * @retval None
 */
void BusPoll(void)
{
  for (uint8_t i = 0; i < BUS_NUM; i++)
  {
    if (s_tBusObj[i].runBit)
      continue;

    for (uint8_t j = 0; j < BUS_FLAG_BIT_NUM; j++)
    {
      if (BIT_READ(s_tBusObj[i].pendingFlag, BIT(j)))
      {
        s_tBusObj[i].runBit = j + 1;
        BIT_CLEAR(s_tBusObj[i].pendingFlag, BIT(j));
      }
    }
  }
}

/**
 * @brief  Set bus wait flag bit
 * @param  BUS_x:    Bus Type(<BUS_NUM)
 * @param  bitIndex: Bus bit index(0-sizeof(BUS_FLAG_TYPE_t))
 * @retval 0 Success
 */
BusError_t BusSetWaitFlag(BusType_t BUS_x, uint8_t bitIndex)
{
  if ((BUS_x >= BUS_NUM) || (bitIndex >= BUS_FLAG_BIT_NUM))
    return BUS_ERR_Param;

  SYSEnterCritical();

  if (!(s_tBusObj[BUS_x].pendingFlag & ~((BUS_FLAG_TYPE_t)(1 << bitIndex))) && !s_tBusObj[BUS_x].runBit)
  {
    s_tBusObj[BUS_x].runBit = bitIndex + 1;
    s_tBusObj[BUS_x].pendingFlag &= ~((BUS_FLAG_TYPE_t)(1 << bitIndex));
  }

  if (s_tBusObj[BUS_x].runBit != (bitIndex + 1))
    s_tBusObj[BUS_x].pendingFlag |= ((BUS_FLAG_TYPE_t)(1 << bitIndex));

  SYSExitCritical();

  return BUS_OK;
}

/**
 * @brief  Clear bus wait flag bit
 * @param  BUS_x:    Bus Type(<BUS_NUM)
 * @param  bitIndex: Bus bit index(0-sizeof(BUS_FLAG_TYPE_t))
 * @retval 0 Success
 */
BusError_t BusClearWaitFlag(BusType_t BUS_x, uint8_t bitIndex)
{
  if ((BUS_x >= BUS_NUM) || (bitIndex >= BUS_FLAG_BIT_NUM))
    return BUS_ERR_Param;

  SYSEnterCritical();
  s_tBusObj[BUS_x].pendingFlag &= ~((BUS_FLAG_TYPE_t)(1 << bitIndex));

  if (s_tBusObj[BUS_x].runBit == (bitIndex + 1))
    s_tBusObj[BUS_x].runBit = 0;

  SYSExitCritical();

  return BUS_OK;
}

/**
 * @brief  Check bus wait flag bit are equal   //校验是否获得总线使用权
 * @param  BUS_x:    Bus Type(<BUS_NUM)
 * @param  bitIndex: Bus bit index(0-sizeof(BUS_FLAG_TYPE_t))
 * @retval 0 Success
 */
BusError_t BusCheckWaitFlag(BusType_t BUS_x, uint8_t bitIndex)
{
  BusError_t retErr = BUS_ERR_Busy;

  if ((BUS_x >= BUS_NUM) || (bitIndex >= BUS_FLAG_BIT_NUM))
    return BUS_ERR_Param;

  SYSEnterCritical();

  if (s_tBusObj[BUS_x].runBit == (bitIndex + 1))
    retErr = BUS_OK;

  SYSExitCritical();

  return retErr;
}

/**
 * @brief  Check whether the bus is idle  //检查总线是否空闲
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @retval 1 Idle, 0 Busy
 */
uint8_t BusIsIdle(BusType_t BUS_x)
{
  if (BUS_x >= BUS_NUM)
    return 0;

  if (s_tBusObj[BUS_x].pendingFlag || s_tBusObj[BUS_x].runBit)
    return 0;

  return 1;
}

/**
 * @brief  Wait bus idle    //等待总线空闲
 * @param  BUS_x:    Bus Type(<BUS_NUM)
 * @param  bitIndex: Bus bit index(0-sizeof(BUS_FLAG_TYPE_t))
 * @param  tmout:    Maxium wait time
 * @retval 0 Success
 */
BusError_t BusWaitIdle(BusType_t BUS_x, uint8_t bitIndex, uint32_t tmout)
{
  BusError_t retErr = BUS_ERR_Busy;

  uint32_t curtick = SYSGetTick() + SYSMsToTick(tmout);

  if ((BUS_x >= BUS_NUM) || (bitIndex >= BUS_FLAG_BIT_NUM))
    return BUS_ERR_Param;

  BusSetWaitFlag(BUS_x, bitIndex);

  while ((SYSGetTick() < curtick) && (BUS_OK != retErr))
  {
    retErr = BusCheckWaitFlag(BUS_x, bitIndex);
    FL_DelayMs(10);
  }

  if (BUS_OK != retErr)
    BusClearWaitFlag(BUS_x, bitIndex);

  return BusCheckWaitFlag(BUS_x, bitIndex);
}


/**
 * @brief  Set communication parameter
 * @param  BUS_x:  Bus Type(<=BUS_NUM)
 * @param  baud:   baudrate
 * @param  parity: parity bit(0-None 1-Odd 2-Even)
 * @retval 0 Success
 */
uint8_t BusSetParam(BusType_t BUS_x, uint32_t baud, uint8_t parity)
{
  uint8_t ret = 0;

  switch (BUS_x)
  {
    case BUS_UART:
      // uart1_set_param(baud, parity);
      break;

    case BUS_RS485:
      //uart1_set_param(baud, parity);
      break;

    case BUS_MBUS:
      uart4_set_param(baud, parity);
      break;

    case BUS_BLE:
      #if(BLUETOOTH_IS_ENABLE == 1)
      {
        uart3_set_param(baud, parity);
      }
      #endif
      break;
		
    case BUS_LORA:
      uart0_set_param(baud, parity);
      break;

    default:
      uart0_set_param(baud, parity);
      #if(BLUETOOTH_IS_ENABLE == 1)
      {
        uart3_set_param(baud, parity);
      }
      #endif  
      uart4_set_param(baud, parity);
      //uart4_set_param(baud, parity);
      break;
  }

  return ret;
}


/**
 * @brief  Send data
 * @param  BUS_x: Bus Type(<=BUS_NUM)
 * @param  pbuf:  data buffer
 * @param  len:   data length
 * @return Actual send length
 */
uint16_t BusSend(BusType_t BUS_x, const uint8_t *pbuf, uint16_t len)
{
  uint16_t ret;

#ifdef USE_FULL_ASSERT1

  if (BUS_x != BUS_UART)
  {
    log_i("[Bus Send]port=%d, len=%d", BUS_x, len);

    for (uint16_t i = 0; i < len; i++)
    {
      xprintf("%02X ", pbuf[i]);

      if (!((i + 1) % 32))
        xprintf("\r\n");
    }

    xprintf("\r\n");
  }

#endif

  switch (BUS_x)
  {
    case BUS_UART:
      //ret = uart1_send(pbuf, len);
      break;

    case BUS_RS485:
			//ret = uart1_send(pbuf, len);
      break;

    case BUS_MBUS:
      uart4_set_rx_stat(0);
      //ret = uart4_send(pbuf, len);
      uart4_set_rx_stat(1);
      break;

    case BUS_BLE:
      //ret = uart3_send(pbuf, len);
      break;
		
    case BUS_LORA:
      SYSEnterCritical();
      //ret = uart0_send(pbuf, len);
      SYSExitCritical();
      break;

    default:
      ret = uart0_send(pbuf, len);
      ret = uart3_send(pbuf, len);
      ret = uart4_send(pbuf, len);
      //ret = uart4_send(pbuf, len);
      break;
  }

  return ret;
}

/**
 * @brief  Checks whether a data frame has been received.
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @retval 1 Have one frame, 0 Nothing
 */
uint8_t BusIsRxValidFrame(BusType_t BUS_x)
{
  uint8_t ret;

  switch (BUS_x)
  {
    case BUS_UART:
      //ret = uart1_is_rx_valid();
      break;

    case BUS_RS485:
      //ret = uart1_is_rx_valid();
      break;

    case BUS_MBUS:
     // ret = uart4_is_rx_valid();
      break;

    case BUS_BLE:
      //ret = uart3_is_rx_valid();
      break;

    case BUS_LORA:
      //ret = uart0_is_rx_valid();
      break;
		
    default:
      ret = 0;
      break;
  }

#ifdef USE_FULL_ASSERT1

  if (ret && (BUS_UART != BUS_x))
  {
    uint16_t len = BusGetRxLen(BUS_x);
    const uint8_t *pbuf = BusGetRxBuffer(BUS_x);
    log_i("[Bus Recv]port=%d, len=%d", BUS_x, len);

    for (uint16_t i = 0; i < len; i++)
    {
      xprintf("%02X ", pbuf[i]);

      if (!((i + 1) % 32))
        xprintf("\r\n");
    }

    xprintf("\r\n");
  }

#endif

  return ret;
}

/**
 * @brief  Get receive data max length.
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @return Receive data length
 */
uint16_t BusGetRxLenMax(BusType_t BUS_x)
{
  uint16_t ret = 0;

  switch (BUS_x)
  {
    case BUS_UART:
      //ret = uart1_get_rx_len_max();
      break;

    case BUS_RS485:
      //ret = uart1_get_rx_len_max();
      break;

    case BUS_MBUS:
      //ret = uart4_get_rx_len_max();
      break;

    case BUS_BLE:
      //ret = uart3_get_rx_len_max();
      break;

    case BUS_LORA:
      //ret = uart0_get_rx_len_max();
      break;
		
    default:
      ret = 0;
      break;
  }

  return ret;
}


/**
 * @brief  Get receive data length.
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @return Receive data length
 */
uint16_t BusGetRxLen(BusType_t BUS_x)
{
  uint16_t ret;

  switch (BUS_x)
  {
    case BUS_UART:
      //ret = uart1_get_rx_len();
      break;

    case BUS_RS485:
      //ret = uart1_get_rx_len();
      break;

    case BUS_MBUS:
      //ret = uart4_get_rx_len();
      break;

    case BUS_BLE:
      //ret = uart3_get_rx_len();
      break;

    case BUS_LORA:
      //ret = uart0_get_rx_len();
      break;
		
    default:
      ret = 0;
      break;
  }

  return ret;
}

/**
 * @brief  Get receive data buffer point.
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @return Receive data buffer point
 */
void *BusGetRxBuffer(BusType_t BUS_x)
{
  switch (BUS_x)
  {
    case BUS_UART:
      return (void *)0;

    case BUS_RS485:
      return (void *)0;

    case BUS_MBUS:
      //return uart4_get_rx_buff();

    case BUS_BLE:
      return uart3_get_rx_buff();
		
   case BUS_LORA:
      return uart0_get_rx_buff();

    default:
      return (void *)0;
  }
}

/**
 * @brief  Get last receive char timestamp.
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @return Last receive char timestamp
 */
uint32_t BusLastRxMs(BusType_t BUS_x)
{
  uint32_t ret = 0;

  switch (BUS_x)
  {
    case BUS_UART:
      //ret = uart1_get_last_rx_ms();
      break;

    case BUS_RS485:
      //ret = uart1_get_last_rx_ms();
      break;

    case BUS_MBUS:
      //ret = uart4_get_last_rx_ms();
      break;

    case BUS_BLE:
      //ret = uart3_get_last_rx_ms();
      break;
		
    case BUS_LORA:
      //ret = uart0_get_last_rx_ms();
      break;

    default:
      ret = 0;
      break;
  }

  return ret;
}

/**
 * @brief  Clear rx buff and enable receive
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @return None
 */
void BusEnableRx(BusType_t BUS_x)
{
  switch (BUS_x)
  {
    case BUS_UART:
      //uart1_set_rx_stat(1);
      break;

    case BUS_RS485:
      //uart1_set_rx_stat(1);
      break;

    case BUS_MBUS:
      //uart4_set_rx_stat(1);
      break;

    case BUS_BLE:
      //uart3_set_rx_stat(1);
      break;

    case BUS_LORA:
      //uart0_set_rx_stat(1);
      break;
		
    default:
      //uart1_set_rx_stat(1);
      //uart3_set_rx_stat(1);
      //uart4_set_rx_stat(1);
      //uart4_set_rx_stat(1);
      break;
  }
}

/**
 * @brief  Stop receive
 * @param  BUS_x: Bus Type(<BUS_NUM)
 * @return None
 */
void BusDisableRx(BusType_t BUS_x)
{
  switch (BUS_x)
  {
    case BUS_UART:
      //uart1_set_rx_stat(0);
      break;

    case BUS_RS485:
      //uart1_set_rx_stat(0);
      break;

    case BUS_MBUS:
      //uart4_set_rx_stat(0);
      break;

    case BUS_BLE:
      //uart3_set_rx_stat(0);
      break;

    case BUS_LORA:
      //uart0_set_rx_stat(0);
      break;
		
    default:
      //uart1_set_rx_stat(0);
      //uart3_set_rx_stat(0);
      //uart4_set_rx_stat(0);
      //uart4_set_rx_stat(0);
      break;
  }
}

/**
 * @}
 */

