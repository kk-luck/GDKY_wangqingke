/**
  ******************************************************************************
  *               Copyright(C) 2022-2032 GDKY All Rights Reserved
  *
  * @file     fm33lg0xx_init.h
  * @author   TianRuidong
  * @version  V1.00
  * @date     
  * @brief    FM33LG0XX power on default status.
  ******************************************************************************
  * @history
  */

/* INCLUDES ----------------------------------------------------------------- */
#include "includes.h"
#include "sysClock.h"

/* TYPEDEFS ----------------------------------------------------------------- */

/* MACROS  ------------------------------------------------------------------ */

/* CONSTANTS  --------------------------------------------------------------- */

/* GLOBAL VARIABLES --------------------------------------------------------- */

/* GLOBAL FUNCTIONS --------------------------------------------------------- */

/* LOCAL VARIABLES ---------------------------------------------------------- */

/* LOCAL FUNCTIONS ---------------------------------------------------------- */



/**
  * @brief  SWD_IO Initialization function
  * @param  void
  * @retval None
  */
void MF_SWD_IO_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    /* PD7 SWD_SWCLK */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* PD8 SWD_SWDIO */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}


/**
  * @brief  CMU_XTLF Initialization function
  * @param  void
  * @retval None
  */
void MF_CMU_XTLF_Init(void)
{
    FL_VAO_XTLF_InitTypeDef    XTLF_InitStruct;

    XTLF_InitStruct.driveMode = FL_VAO_XTLF_DRIVE_LEVEL_1;
    XTLF_InitStruct.workingCurrentMode = FL_VAO_XTLF_WORK_CURRENT_450NA;

    FL_VAO_XTLF_Init(VAO, &XTLF_InitStruct);    
}

/**
 * @brief 系统时钟配置
 * @param
 * @retval
 */
void sysClockInit(void)
{
		FL_DelayInit();
	
    /* Initial CDIF */
    FL_CDIF_EnableCPUToVAO(CDIF);
    FL_CDIF_EnableVAOToCPU(CDIF);

    /* Initial RTCA Adjust Value */
    FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);
    FL_RTCA_WriteAdjustValue(RTCA, 0);
    FL_CMU_DisableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);

    /* Initial RTCB Adjust Value */
    FL_RTCB_WriteAdjustValue(RTCB, 0);

    /* Initial XTLF */
    MF_CMU_XTLF_Init();
    FL_VAO_XTLF_Enable(VAO);
    FL_DelayMs(1000);

    /* Initial Low Speed Clock */
    FL_CMU_SetLSCLKClockSource(FL_CMU_LSCLK_CLK_SOURCE_RCLP);

    /* Initial RCHF */
    FL_CMU_RCHF_WriteTrimValue(RCHF8M_TRIM);
    FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_8MHZ);
    FL_CMU_RCHF_Enable();

    /* Initial System Clock */
    FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);    /* 设置FLASH读等待为 0 个周期 */
    FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCHF);    /* 设置系统主时钟为 RCHF */
    FL_CMU_SetAHBPrescaler(FL_CMU_AHBCLK_PSC_DIV1);
    FL_CMU_SetAPBPrescaler(FL_CMU_APBCLK_PSC_DIV1);

    /* Initial CDIF Clock Prescaler */
    FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV1);

    SystemCoreClockUpdate();   
		
		MF_SWD_IO_Init();     //SWD口配置
		
}























