/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "fm33lg0xx_it.h"


/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/


/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void SysTick_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
//		return;
  }
}




//
void NMI_Handler(void)
{
    //NWKUP
    if(FL_SET == FL_PMU_IsActiveFlag_WakeupPIN(PMU, FL_PMU_WAKEUP0_PIN))     //
    {
        FL_PMU_ClearFlag_WakeupPIN(PMU, FL_PMU_WAKEUP0_PIN);
			
//				wakeUpAllTaskFromISR();    //
    }
	
    if(FL_SET == FL_PMU_IsActiveFlag_WakeupPIN(PMU, FL_PMU_WAKEUP9_PIN))    //唤醒owi
    { 	

			
//			extern void OWI_IRQHandler(void);    //
//			
//			OWI_IRQHandler();
			
			FL_PMU_ClearFlag_WakeupPIN(PMU, FL_PMU_WAKEUP9_PIN);
			
    }
}



void GPIO_IRQHandler(void)
{
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_4))
    {
			
			FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_4);
			extern void OWI_IRQHandler(void);   //
			OWI_IRQHandler();   //按键中断
			
		
    }
}






/**
  * @brief This function handles RTC interrupt.
  */
void RTC_IRQHandler(void)
{
  if (FL_RTCA_IsActiveFlag_Second(RTCA))
  {
    FL_RTCA_ClearFlag_Second(RTCA);
  }
}

