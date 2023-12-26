/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     fm24clxx.c
  * @author   ZouZH
  * @version  V1.09
  * @date     12-June-2019
  * @brief    RAMTRON FRAM Serial Memory Driver for FM33G0xx.
  ******************************************************************************
  * @history :
  * --2014-12-01 V1.01 Add FM24V10, Change of address space
  * --2014-12-07 V1.02 Add Erase chip function.
  * --2014-12-08 V1.03 Modify the I2C timing parameters
  * --2015-01-22 V1.04 Add FM24V10_2
  * --2015-01-28 V1.05 Add FreeRTOS Mutex Semaphore
  * --2015-02-16 V1.06 Add FM24V10_3 FM24V10_4
  * --2015-06-25 V1.07 Add FM24CL64_2, FM24W256_1, FM24W256_2
  *                    Modify FM24CL64 to FM24CL64_1
  * --2016-03-17 V1.08 Modify FRAM_EraseChip() to FRAM_Erase()
  * --2019-06-12 V1.09 Reformatting the code
  */


/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "fm24clxx.h"

/* TYPEDEFS ------------------------------------------------------------------- */

/* MACROS  -------------------------------------------------------------------- */

#define GPIO_PORT_SDA GPIOD
#define GPIO_PORT_CLK GPIOB
#define GPIO_PORT_WP  GPIOE

#define GPIO_PIN_SDA  FL_GPIO_PIN_12
#define GPIO_PIN_CLK  FL_GPIO_PIN_15
#define GPIO_PIN_WP   FL_GPIO_PIN_5

#define SDA_H()       FL_GPIO_SetOutputPin(GPIO_PORT_SDA, GPIO_PIN_SDA)
#define SDA_L()       FL_GPIO_ResetOutputPin(GPIO_PORT_SDA, GPIO_PIN_SDA)
#define SDA_READ()    FL_GPIO_GetInputPin(GPIO_PORT_SDA, GPIO_PIN_SDA)

#define SCL_H()       FL_GPIO_SetOutputPin(GPIO_PORT_CLK, GPIO_PIN_CLK)
#define SCL_L()       FL_GPIO_ResetOutputPin(GPIO_PORT_CLK, GPIO_PIN_CLK)

#define WP_H()        FL_GPIO_SetOutputPin(GPIO_PORT_WP, GPIO_PIN_WP)
#define WP_L()        FL_GPIO_ResetOutputPin(GPIO_PORT_WP, GPIO_PIN_WP)


/**
  * @brief  memory address computation
  */
#define ADDR_1st_CYCLE(ADDR)       (uint8_t)((ADDR)& 0xFFUL)               /* 1st addressing cycle */
#define ADDR_2nd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF00UL) >> 8)      /* 2nd addressing cycle */
#define ADDR_3rd_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF0000UL) >> 16)   /* 3rd addressing cycle */
#define ADDR_4th_CYCLE(ADDR)       (uint8_t)(((ADDR)& 0xFF000000UL) >> 24) /* 4th addressing cycle */



/* CONSTANTS  ----------------------------------------------------------------- */

#define I2C_DELAY       5
#define I2C_FALSE       0
#define I2C_TRUE        1

/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */

#if configUSE_MUTEXES
  static SemaphoreHandle_t hMutexFRAM = NULL;
#endif  /* INC_FREERTOS_H */


/* LOCAL FUNCTIONS ------------------------------------------------------------ */
static uint8_t i2c_start(void);
static uint8_t i2c_restart(void);
static void    i2c_stop(void);
static void    i2c_ack(void);
static void    i2c_noack(void);
static uint8_t i2c_waitack(void);
static void    i2c_writebyte(uint8_t byte);
static uint8_t i2c_readbyte(uint8_t *pbyte);


/**
 * @brief  FRAM init
 * @param  None
 * @retval None
 */
void FRAM_Init(void)
{
  FL_GPIO_InitTypeDef GPIO_InitStruct;
  FL_I2C_MasterMode_InitTypeDef I2C_InitStruct;

  FL_GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.pull = FL_DISABLE;

  GPIO_InitStruct.pin = GPIO_PIN_SDA;
  FL_GPIO_Init(GPIO_PORT_SDA, &GPIO_InitStruct);

  GPIO_InitStruct.pin = GPIO_PIN_CLK;
  FL_GPIO_Init(GPIO_PORT_CLK, &GPIO_InitStruct);

  GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.pull = FL_DISABLE;

	WP_H();
  GPIO_InitStruct.pin =  GPIO_PIN_WP;
  FL_GPIO_Init(GPIO_PORT_WP, &GPIO_InitStruct);
  WP_H();


  FL_I2C_DeInit(I2C);

  I2C_InitStruct.clockSource = FL_CMU_I2C_CLK_SOURCE_APBCLK;  //FL_CMU_I2C_CLK_SOURCE_APBCLK
  I2C_InitStruct.baudRate = 300000;
  FL_I2C_MasterMode_Init(I2C, &I2C_InitStruct);

//#if 0
//  // I2C bus deadlock reset
//  if (i2c_start() != I2C_TRUE)
//  {
//    for (uint8_t i = 0; i < 9; i++)
//    {
//      i2c_start();
//      i2c_stop();
//    }
//  }

//  i2c_stop();
//#else
//  i2c_restart();
//#endif
	
	i2c_stop();
	

#if configUSE_MUTEXES

  if (hMutexFRAM == NULL)
    hMutexFRAM = xSemaphoreCreateMutex();

#endif /* INC_FREERTOS_H*/
}

/**
 * @brief  FRAM Write
 *
 * @param[in] FM24xx: IC type @ref FRAMDev_t
 * @param[in] addr: Data start address
 * @param[in] pvbuf: Data buffer
 * @param[in] size: Data bytes
 *
 * @retval I2C_TRUE(1): Success
 * @retval I2C_FALSE(0): Fail
 */
uint8_t FRAM_Write(FRAMDev_t FM24xx, uint16_t addr, const void *pvbuf, uint16_t size)
{
  uint8_t err = I2C_TRUE;
  uint16_t addrtmp = 0;
  const uint8_t *p = pvbuf;

	SYSEnterCritical();
	
  do
  {
#if configUSE_MUTEXES
    xSemaphoreTake(hMutexFRAM, portMAX_DELAY);
#endif /* INC_FREERTOS_H */

    WP_L();

    addrtmp = addr & FRAM_MAX_ADDR_x256;

    for (uint16_t i = 0; i < size; i++)
    {
      // check bus busy (write cycle 5ms)
      for (uint8_t cnt = 0; cnt < 10; cnt++)
      {
        if (I2C_FALSE == err)
        {
          i2c_stop();
          FL_DelayMs(1);
        }

        if (I2C_FALSE == (err = i2c_start()))
          continue;

        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx) << 1));

        if (I2C_FALSE == (err = i2c_waitack()))
          continue;

        // data addr
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp + i));

        if (I2C_FALSE == (err = i2c_waitack()))
          continue;

        i2c_writebyte(ADDR_1st_CYCLE(addrtmp + i));

        if (I2C_FALSE == (err = i2c_waitack()))
          continue;

        break;
      }

      if (I2C_FALSE == err)
        break;

      do
      {
        // write data
        i2c_writebyte(*(p + i));

        if (I2C_FALSE == (err = i2c_waitack()))
          break;

        if (0 == ((addrtmp + i + 1) % 32))
          break;

        i += 1;
      } while (i < size);

      if (I2C_FALSE == err)
        break;

      i2c_stop();
      FL_DelayMs(10);
    }
  } while (0);

  if (I2C_TRUE != err)
    i2c_stop();

  WP_H();

#if configUSE_MUTEXES
  xSemaphoreGive(hMutexFRAM);
#endif  /* INC_FREERTOS_H */

	SYSExitCritical();
	
  return err;
}


/**
 * @brief  FRAM Read
 *
 * @param[in] FM24xx: IC type @ref FRAMDev_t
 * @param[in] addr: Data start address
 * @param[out] pvbuf: Data buffer
 * @param[in] size: Data bytes
 *
 * @retval I2C_TRUE(1): Success
 * @retval I2C_FALSE(0): Fail
 */
uint8_t FRAM_Read(FRAMDev_t FM24xx, uint16_t addr, void *pvbuf, uint16_t size)
{
  uint8_t err = I2C_TRUE;
  uint16_t i = 0;
  uint16_t addrtmp = 0;
  uint8_t *p = (uint8_t *)pvbuf;

	SYSEnterCritical();
	
  do
  {
#if configUSE_MUTEXES
    xSemaphoreTake(hMutexFRAM, portMAX_DELAY);
#endif /* INC_FREERTOS_H */

    addrtmp = addr & FRAM_MAX_ADDR_x256;

    // check bus busy (write cycle 5ms)
    for (uint8_t cnt = 0; cnt < 10; cnt++)
    {
      if (I2C_FALSE == err)
      {
        i2c_stop();
        FL_DelayMs(1);
      }

      if (I2C_FALSE == (err = i2c_start()))
        continue;

      i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx) << 1));

      if (I2C_FALSE == (err = i2c_waitack()))
        continue;

      // data addr
      i2c_writebyte(ADDR_2nd_CYCLE(addrtmp + i));

      if (I2C_FALSE == (err = i2c_waitack()))
        continue;

      i2c_writebyte(ADDR_1st_CYCLE(addrtmp + i));

      if (I2C_FALSE == (err = i2c_waitack()))
        continue;

      break;
    }

    if (I2C_FALSE == err)
      break;

    // read data
    if (I2C_FALSE == (err = i2c_restart()))
      break;

    i2c_writebyte(FRAM_READ_ADDR | ((FM24xx) << 1));

    if (I2C_FALSE == (err = i2c_waitack()))
      break;

    for (i = 0; i < size - 1; i++)
    {
      i2c_ack();
      i2c_readbyte(p + i);
    }

    i2c_readbyte(p + i);
    i2c_noack();
  } while (0);

  i2c_stop();

	if(FL_GPIO_GetInputPin(GPIO_PORT_SDA, GPIO_PIN_SDA) == 0)
	{
		i2c_start();
		i2c_writebyte(0xFF);
		i2c_stop();
	}
	
#if configUSE_MUTEXES
  xSemaphoreGive(hMutexFRAM);
#endif  /* INC_FREERTOS_H */

	SYSExitCritical();
	
  return err;
}


/**
 * @brief  FRAM Erase
 *
 * @param[in] FM24xx: IC type @ref FRAMDev_t
 * @param[in] addr: Erase start address
 * @param[in] size: Erase bytes
 *
 * @retval I2C_TRUE(1): Success
 * @retval I2C_FALSE(0): Fail
 */
uint8_t FRAM_Erase(FRAMDev_t FM24xx, uint16_t addr, uint16_t size)
{
  uint8_t err = I2C_TRUE;
  uint16_t addrtmp = 0;

	SYSEnterCritical();
	
  do
  {
#if configUSE_MUTEXES
    xSemaphoreTake(hMutexFRAM, portMAX_DELAY);
#endif /* INC_FREERTOS_H */

    WP_L();

    addrtmp = addr & FRAM_MAX_ADDR_x256;

    for (uint16_t i = 0; i < size; i++)
    {
      // check bus busy (write cycle 5ms)
      for (uint8_t cnt = 0; cnt < 10; cnt++)
      {
        if (I2C_FALSE == err)
        {
          i2c_stop();
          FL_DelayMs(1);
        }

        if (I2C_FALSE == (err = i2c_start()))
          continue;

        i2c_writebyte(FRAM_WRITE_ADDR | ((FM24xx) << 1));

        if (I2C_FALSE == (err = i2c_waitack()))
          continue;

        // data addr
        i2c_writebyte(ADDR_2nd_CYCLE(addrtmp + i));

        if (I2C_FALSE == (err = i2c_waitack()))
          continue;

        i2c_writebyte(ADDR_1st_CYCLE(addrtmp + i));

        if (I2C_FALSE == (err = i2c_waitack()))
          continue;

        break;
      }

      if (I2C_FALSE == err)
        break;

      do
      {
        // write data
        i2c_writebyte(0xFF);

        if (I2C_FALSE == (err = i2c_waitack()))
          break;

        if (0 == ((addrtmp + i + 1) % 32))
          break;

        i += 1;
      } while (i < size);

      if (I2C_FALSE == err)
        break;

      i2c_stop();
      FL_DelayMs(10);
    }
  } while (0);

  if (I2C_TRUE != err)
    i2c_stop();

  WP_H();

#if configUSE_MUTEXES
  xSemaphoreGive(hMutexFRAM);
#endif  /* INC_FREERTOS_H */

	SYSExitCritical();
	
  return err;
}



// START signal
static uint8_t i2c_start(void)
{
  uint8_t i;

  FL_I2C_Master_EnableI2CStart(I2C);

  for (i = 0; i < I2C_DELAY; i++)
  {
    if (FL_I2C_Master_IsActiveFlag_Start(I2C))
      break;

    FL_DelayMs(1);
  }

  if (i < I2C_DELAY)
    return I2C_TRUE;

  return I2C_FALSE;
}

// RESTART signal
static uint8_t i2c_restart(void)
{
  uint8_t i;

  FL_I2C_Master_EnableI2CRestart(I2C);

  for (i = 0; i < I2C_DELAY; i++)
  {
    if (FL_I2C_Master_IsActiveFlag_Start(I2C))
      break;

    FL_DelayMs(1);
  }

  if (i < I2C_DELAY)
    return I2C_TRUE;

  return I2C_FALSE;
}


// STOP signal
static void i2c_stop(void)
{
  uint8_t i;

  FL_I2C_Master_EnableI2CStop(I2C);

  for (i = 0; i < I2C_DELAY; i++)
  {
    if (FL_I2C_Master_IsActiveFlag_Stop(I2C))
      break;

    FL_DelayMs(2);
  }

  if (i < I2C_DELAY)
    return;

//  i2c_restart();
}

// ACK signal
static void i2c_ack(void)
{
  FL_I2C_Master_SetRespond(I2C, FL_I2C_MASTER_RESPOND_ACK);
}

// NOACK signal
static void i2c_noack(void)
{
  FL_I2C_Master_SetRespond(I2C, FL_I2C_MASTER_RESPOND_NACK);
}


// WAIT ACK signal
static uint8_t i2c_waitack(void)
{
  uint16_t i;

  for (i = 0; i < I2C_DELAY; i++)
  {
    if (!FL_I2C_Master_IsActiveFlag_NACK(I2C))
      break;

    FL_DelayMs(1);
  }

  if (i < I2C_DELAY)
    return I2C_TRUE;

  FL_I2C_Master_ClearFlag_NACK(I2C);

  return I2C_FALSE;
}

// Write signal,MSB First
static void i2c_writebyte(uint8_t byte)
{
  if (FL_I2C_Master_IsEnabledRX(I2C))
    FL_I2C_Master_DisableRX(I2C);
  
  FL_I2C_Master_WriteTXBuff(I2C, byte);

  while(!FL_I2C_Master_IsActiveFlag_TXComplete(I2C));

  FL_I2C_Master_ClearFlag_TXComplete(I2C);
}

// Read signal,MSB First
static uint8_t i2c_readbyte(uint8_t *pbyte)
{
  uint16_t i;

  if (!FL_I2C_Master_IsEnabledRX(I2C))
    FL_I2C_Master_EnableRX(I2C);

  for (i = 0; i < 1000; i++)
  {
    if (FL_I2C_Master_IsActiveFlag_RXComplete(I2C))
      break;
  }

  FL_I2C_Master_ClearFlag_RXComplete(I2C);

  *pbyte = FL_I2C_Master_ReadRXBuff(I2C);

  if (i < 1000)
    return I2C_TRUE;

  return I2C_FALSE;
}

