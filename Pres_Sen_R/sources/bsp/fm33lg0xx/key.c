/**
  ******************************************************************************
  *               Copyright(C) 2019-2029 GDKY All Rights Reserved
  *
  * @file     key.c
  * @author   TianRuidong
  * @version  V1.02
  * @date     
  * @brief    Key detection service
  ******************************************************************************
  */
#define LOG_TAG    "KEY"

/* INCLUDES ------------------------------------------------------------------- */
#include "includes.h"
#include "key.h"
#include "queue.h"
#include "sys_tick.h"

/* TYPEDEFS ------------------------------------------------------------------- */


/**
 * @brief 按键瞬时状态（没有软件消抖前的状态）
 */
typedef struct
{
  KEY_VALUE_t  value;	   //键值（哪个按键被按下）
  KEY_STATUS_t state;    //按键状态
	uint8_t   newAction;     //按键有新动作，则置1
	uint8_t   KeyDownPress;    //按键按下有效，则置1（用来过滤由于抖动造成的 按键松开 确认-----既，如果按键按下是是抖动状态，则按键松开后也不会把 按键松开 认为成一次按键）
	uint32_t  KeyActionDoTick;        //按键新动作时间戳
	uint32_t  KeyLastPressDoTick;        //按键上次有效按下时间戳（用于判断双击）
} keyInstantState_t;


/* MACROS  -------------------------------------------------------------------- */

//#define KEY_NUM   1   //按键数量

#define KEY1_PIN           FL_GPIO_PIN_6
#define KEY1_PORT          GPIOE

#define KEY_GET_SCAN_STATE()  	FL_GPIO_GetInputPin(KEY1_PORT, KEY1_PIN)

#define KEY1_EXTI_INPUT_GROUP  FL_GPIO_EXTI_INPUT_GROUP3
#define KEY1_EXTI_LINE         FL_GPIO_EXTI_LINE_17
#define KEY1_WAKEUP            FL_GPIO_WAKEUP_9


/* CONSTANTS  ----------------------------------------------------------------- */


/*
 * Key drive parameter configuration
 */
#define KEY_PRESS_TIME    20     // 短按(20ms)
#define KEY_DOUBLE_TIME   500    // 双击时间(500ms)
#define KEY_LONG_TIME     5000   // 长按时间(5s)
#define KEY_REPEAT_TIME   500    // 长时间重复该按键(500ms)

#define KEY_QUEUE_SIZE    10     //按键队列长度

/* GLOBAL VARIABLES ----------------------------------------------------------- */

QueueHandle_t shQueueKeyDetector;     //按键队列

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

/* LOCAL VARIABLES ------------------------------------------------------------ */


static keyInstantState_t keyInstantState[KEY_NUM];   //按键瞬时状态（没有经过消抖前）

  

/* LOCAL FUNCTIONS ------------------------------------------------------------ */




/**
* @brief  初始化按键为低电平
 * @param  None
 * @retval None
 */
void Key_hwInit(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
		
    FL_EXTI_InitTypeDef extiInitStruct = {0};
    FL_EXTI_CommonInitTypeDef  extiCommonInitStruct = {0};

    extiCommonInitStruct.clockSource = FL_CMU_EXTI_CLK_SOURCE_HCLK;    //EXTI中断采样时钟选择
    (void)FL_EXTI_CommonInit(&extiCommonInitStruct);

    // Configure pin as input
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_ENABLE;
		GPIO_InitStruct.analogSwitch =  FL_ENABLE;
  
    GPIO_InitStruct.pin = KEY1_PIN;
    FL_GPIO_Init(KEY1_PORT, &GPIO_InitStruct);
		
		
    extiInitStruct.input       = KEY1_EXTI_INPUT_GROUP;        //中断线上对应的IO口
    extiInitStruct.filter      = FL_ENABLE;                        //使能数字滤波
    extiInitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_BOTH;   //设置触发边沿  FL_GPIO_EXTI_TRIGGER_EDGE_FALLING
    (void)FL_EXTI_Init(KEY1_EXTI_LINE, &extiInitStruct);

		//WKUP 使能
		FL_GPIO_SetWakeupInterruptEntry(GPIO, FL_GPIO_WAKEUP_INT_ENTRY_NMI);//NMI中断入口
		FL_GPIO_SetWakeupEdge(GPIO, KEY1_WAKEUP, FL_GPIO_WAKEUP_TRIGGER_BOTH);//双边沿唤醒
		FL_GPIO_EnableWakeup(GPIO, KEY1_WAKEUP);//使能NWKUP功能
	
    /*NVIC中断配置*/
    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2); //中断优先级
    NVIC_EnableIRQ(GPIO_IRQn);
}




/**
 * @brief  Initialize KEY Service
 * @param  None
 * @retval 0, Init success.
 */
void KeyInit(void)
{
	uint8_t keyValue;
	
  Key_hwInit();

  for(keyValue=0; keyValue<KEY_NUM; keyValue++)    //遍历所有按键
	{
		keyInstantState[keyValue].value = (KEY_VALUE_t)keyValue;
		keyInstantState[keyValue].state = KEY_RELEASE;
		keyInstantState[keyValue].newAction = 0;
		keyInstantState[keyValue].KeyDownPress = 0;
		keyInstantState[keyValue].KeyActionDoTick = 0;
		keyInstantState[keyValue].KeyLastPressDoTick = 0;
	}
	
  if (NULL == shQueueKeyDetector)
    shQueueKeyDetector = xQueueCreate(KEY_QUEUE_SIZE, sizeof(key_event_t));

  if (NULL == shQueueKeyDetector)
  {
//    log_e("Key queue create failure!");
  }
}


/**
 * @brief  读取按键空闲状态
 * @param  None
 * @retval 0、忙    非0、空闲
 */
uint16_t keyIsIdle(void)
{
	uint16_t idleState;
	uint8_t i;
	
	idleState = 1;
  for(i=0; i<KEY_NUM; i++)    //遍历所有按键
	{
		if(JudgmentTimeout(SYSGetTick(),keyInstantState[i].KeyActionDoTick,SYSMsToTick(30000)) == 0)    //
		{
			idleState = 0;
		}
	}
	return idleState;
}



/**
 * @brief  Put KEY Event
 * @param  Key event
 * @retval 0, Put a valid key value
 */
int8_t KeyPut(const key_event_t *ptEvent)
{
	return xQueueSend(shQueueKeyDetector, ptEvent, 0);   //按键写入队列
}
  /**
 * @brief  按键外部中断
 * @param  None
 * @retval None
 */
void keyIRQHandler(void)
{
//	if(FL_GPIO_IsActiveFlag_EXTI(GPIO, KEY1_EXTI_LINE) == 0x01UL)
	{
//		FL_GPIO_ClearFlag_EXTI(GPIO, KEY1_EXTI_LINE);
		
		if(KEY_GET_SCAN_STATE() == 1)   //如果按键状态为抬起
		{
			keyInstantState[KEY_V1].KeyActionDoTick = SYSGetTick();   //获取动作时间戳
			
			keyInstantState[KEY_V1].value = KEY_V1;
			keyInstantState[KEY_V1].state = KEY_RELEASE;    //按键抬起

			keyInstantState[KEY_V1].newAction = 1; //按键有新动作
		}
		if(KEY_GET_SCAN_STATE() == 0)   //如果按键状态为按下
		{
			keyInstantState[KEY_V1].KeyActionDoTick = SYSGetTick();   //获取动作时间戳
			
			keyInstantState[KEY_V1].value = KEY_V1;
			keyInstantState[KEY_V1].state = KEY_DEPRESS;    //按键按下
			
			keyInstantState[KEY_V1].newAction = 1; //按键有新动作
		}
	}
}



/**
 * @brief  轮询按键扫描服务
 * @param  None
 * @retval None
 */
void KeyPoll(void)
{
  uint8_t keyValue;
	key_event_t keyState;
	
///////////////如果按键没用中断，用查询方式，则用下面这段代码//////////////////////////////////
//	if(KEY_GET_SCAN_STATE() == 1)   //如果按键状态为抬起
//	{
//		if((keyInstantState[KEY1].state == KEY_DEPRESS) || (keyInstantState[KEY1].state == KEY_LONG) || (keyInstantState[KEY1].state == KEY_REPEAT))
//		{
//			keyInstantState[KEY1].KeyActionDoTick = SYSGetTick();   //获取动作时间戳
//			
//			keyInstantState[KEY1].value = KEY1;
//			keyInstantState[KEY1].state = KEY_RELEASE;    //按键抬起

//			keyInstantState[KEY1].newAction = 1; //按键有新动作
//		}
//	}
//	if(KEY_GET_SCAN_STATE() == 0)   //如果按键状态为按下
//	{
//		if(keyInstantState[KEY1].state == KEY_RELEASE)
//		{
//			keyInstantState[KEY1].KeyActionDoTick = SYSGetTick();   //获取动作时间戳
//			
//			keyInstantState[KEY1].value = KEY1;
//			keyInstantState[KEY1].state = KEY_DEPRESS;    //按键按下
//			
//			keyInstantState[KEY1].newAction = 1; //按键有新动作
//		}
//	}
/////////////////////////////////////////////////////////////////////////////////////////////
	
  for(keyValue=0; keyValue<KEY_NUM; keyValue++)    //遍历所有按键
	{
		if((keyInstantState[keyValue].newAction != 0) && (JudgmentTimeout(SYSGetTick(),keyInstantState[keyValue].KeyActionDoTick,SYSMsToTick(KEY_PRESS_TIME+100)) == 0))   //如果按键有新动作
		{
			if(JudgmentTimeout(SYSGetTick(),keyInstantState[keyValue].KeyActionDoTick,SYSMsToTick(KEY_PRESS_TIME)))    //如果超过消抖时间
			{
				keyInstantState[keyValue].newAction = 0;
				
				if(keyInstantState[keyValue].state == KEY_DEPRESS)
				{
					keyInstantState[keyValue].KeyDownPress = 1;   //按键按下有效
					
					keyState.value = (KEY_VALUE_t)keyValue;
					keyState.state = KEY_DEPRESS;   //按下
					xQueueSend(shQueueKeyDetector, &keyState, 0);       //按键写入队列						
					
					if(JudgmentTimeout(SYSGetTick(),keyInstantState[keyValue].KeyLastPressDoTick,SYSMsToTick(KEY_DOUBLE_TIME)) == 0)    //如果没有超过 双击 时间
					{
						keyState.value = (KEY_VALUE_t)keyValue;
						keyState.state = KEY_DBLCLICK;   //双击
						xQueueSend(shQueueKeyDetector, &keyState, 0);   //按键写入队列
						
						keyInstantState[keyValue].KeyLastPressDoTick = SYSGetTick() - SYSMsToTick(KEY_DOUBLE_TIME);   //避免连续按键时，按一下也认为是双击的问题
					}
					else
					{
						keyInstantState[keyValue].KeyLastPressDoTick = SYSGetTick();   //获取动作时间戳（用来判断双击）
					}
					
				}
				else if((keyInstantState[keyValue].state == KEY_RELEASE) && (keyInstantState[keyValue].KeyDownPress == 1))    //如果按键状态为 抬起
				{
					keyInstantState[keyValue].KeyDownPress = 0;   //按键松开有效
					
					keyState.value = (KEY_VALUE_t)keyValue;
					keyState.state = KEY_RELEASE;   //松开
					xQueueSend(shQueueKeyDetector, &keyState, 0);       //按键写入队列
				}
			}
		}

		if(keyInstantState[keyValue].state == KEY_DEPRESS)   //如果按键状态为按下状态 且 超过长按时间
		{
			if((JudgmentTimeout(SYSGetTick(),keyInstantState[keyValue].KeyActionDoTick,SYSMsToTick(KEY_LONG_TIME))))    //如果超过 长按 时间
			{
				keyInstantState[keyValue].state = KEY_LONG;   //按键状态更改为常按
				keyInstantState[keyValue].KeyActionDoTick = SYSGetTick();   //获取动作时间戳
				
				keyState.value = (KEY_VALUE_t)keyValue;
				keyState.state = KEY_LONG;    //长按
				xQueueSend(shQueueKeyDetector, &keyState, 0);   //按键写入队列
			}
		}
		else if(keyInstantState[keyValue].state == KEY_LONG)   //如果按键状态为按下状态 且 超过长按时间
		{
			if((JudgmentTimeout(SYSGetTick(),keyInstantState[keyValue].KeyActionDoTick,SYSMsToTick(KEY_REPEAT_TIME))))    //如果超过 长按重复 时间
			{
//				keyInstantState[keyValue].state = KEY_LONG;   //按键状态更改为常按
				keyInstantState[keyValue].KeyActionDoTick = SYSGetTick();   //获取动作时间戳
				
				keyState.value = (KEY_VALUE_t)keyValue;
				keyState.state = KEY_REPEAT;    //长按重复
				xQueueSend(shQueueKeyDetector, &keyState, 0);   //按键写入队列
			}
		}
	}
}

/**
 * 输出按键队列
 * @param  Key event
 * @retval 0, Put a valid key value
 */
QueueHandle_t keyQueue(void)
{
	return shQueueKeyDetector;   //按键写入队列
}





