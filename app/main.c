/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_timer.h"
#include "mcu_uart.h"
#include "mcu_i2c.h"
#include "mcu_spi.h"
#include "mcu_rtc.h"
#include "dev_lcdbus.h"

#include "wujique_log.h"

#include "dev_key.h"
#include "dev_buzzer.h"
#include "dev_touchkey.h"
#include "dev_tea5767.h"
#include "dev_dacsound.h"
#include "dev_spiflash.h"
#include "dev_wm8978.h"
#include "dev_touchscreen.h"
#include "camera_api.h"
#include "dev_rs485.h"
#include "eth_app.h"
#include "dev_lcd.h"
#include "dev_keypad.h"
#include "dev_htu21d.h"
#include "FreeRTos.h"
/** @addtogroup Template_Project
  * @{
  */ 
extern int dev_sdio_test(void);
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

/* Private function prototypes -----------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  1//滴答定时，单位ms


void Delay(__IO uint32_t nTime);
/* Private functions ---------------------------------------------------------*/


/*
	尽快启动RTOS任务
*/
#define START_TASK_STK_SIZE 4096
#define START_TASK_PRIO	3//中间优先级
TaskHandle_t  StartTaskHandle;
void start_task(void *pvParameters);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
 /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */


	/* Set the Vector Table base address at 0x08000000 */
  	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);
	/*
		中断优先级分组，是一个全局的设置，只能在上电初始化时设置一次
		如果中断优先级分组设置为1，会死机，应该跟RTOS有关系
 	*/	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 	
	#ifndef SYS_USE_RTOS
  	/* SysTick end of count event */
  	RCC_GetClocksFreq(&RCC_Clocks);
  	SysTick_Config(RCC_Clocks.HCLK_Frequency / (1000/SYSTEMTICK_PERIOD_MS));
	#endif
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	mcu_uart_init();
	mcu_uart_open(PC_PORT);
	wjq_log(LOG_INFO,"\r\n---hello world!--20180424 10:41---\r\n");

  /* Infinite loop */
	#ifdef SYS_USE_RTOS
	wjq_log(LOG_INFO,"create start task!\r\n");
	xTaskCreate(	(TaskFunction_t) start_task,
					(const char *)"StartTask",		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
					(const configSTACK_DEPTH_TYPE) START_TASK_STK_SIZE,
					(void *) NULL,
					(UBaseType_t) START_TASK_PRIO,
					(TaskHandle_t *) &StartTaskHandle );
	vTaskStartScheduler();
	#else
	void *p;
	wjq_log(LOG_INFO,"run start task(no rtos)\r\n");
	start_task(p);
	#endif
	while(1);
  
}
/**
 *@brief:      start_task
 *@details:    开始第一个任务，主要做初始化
 			   初始化完成后不删除本任务，做为一个最低优先级任务存在
 			   类似HOOK
 *@param[in]   void *pvParameters  
 *@param[out]  无
 *@retval:     
 */
void start_task(void *pvParameters)
{
	#if 1
	GPIO_InitTypeDef GPIO_InitStructure;
	/*初始化LED IO口*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);   
	GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	#endif
	wjq_log(LOG_INFO,"start task---\r\n");
	
	mcu_rtc_init();
	
	sys_dev_register();
	
	dev_key_init();
	dev_keypad_init();
	dev_buzzer_init();
	dev_tea5767_init();
	dev_dacsound_init();

	dev_wm8978_init();
	dev_rs485_init();
	dev_touchscreen_init();
	dev_touchkey_init();
	dev_camera_init();
	dev_8266_init();
	/* stm32 内部ADC 测量温度 */
	mcu_adc_temprate_init();
	dev_htu21d_init();
	
	fun_mount_sd();
	//sys_spiffs_mount_coreflash();
	
	//usb_task_create();

	wujique_407test_init();
	
	/* 默认开启网络测试*/
	//eth_app_init();

	fun_cmd_init();
	
	while (1)
	{
		/*驱动轮询*/
		//wjq_log(LOG_DEBUG, "CORE TASK ");
		dev_key_scan();
		dev_keypad_scan();
		eth_loop_task();
		fun_sound_task();
		fun_rec_task();
		vTaskDelay(2);
		dev_touchkey_task();
	}
}


__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  timingdelay = LocalTime + nCount;  

  /* wait until the desired delay finish */  
  while(timingdelay > LocalTime)
  {     
  }
}

uint32_t Time_Get_LocalTime(void)
{
	return LocalTime;
}

/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
