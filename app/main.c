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

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
 /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */

	  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* SysTick end of count event */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / (1000/SYSTEMTICK_PERIOD_MS));
  
  /* Add your application code here */
  /* Insert 5 ms delay */
  Delay(5);

	/*初始化LED IO口*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);   
	GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	/* Infinite loop */
	mcu_uart_init();
	mcu_uart_open(PC_PORT);
	wjq_log(LOG_INFO, "hello word!\r\n");
	mcu_rtc_init();
	mcu_i2c_init();
	mcu_spi_init();
	dev_lcdbus_init();
	
	dev_key_init();
	dev_keypad_init();
	//mcu_timer_init();
	dev_buzzer_init();
	dev_tea5767_init();
	dev_dacsound_init();
	dev_spiflash_init();
	dev_wm8978_init();
	dev_rs485_init();
	dev_lcd_init();
	//dev_touchscreen_init();
	//dev_camera_init();
	//eth_app_init();

	fun_mount_sd();
	//font_check_hzfont();
	
	//dev_dacsound_open();
	dev_key_open();
	dev_keypad_open();
	//dev_wm8978_open();
	//dev_tea5767_open();
	//dev_tea5767_setfre(105700);
	//dev_camera_open();
	#if 0
	mcu_adc_test();
	#endif
	
	#if 0
	dev_touchscreen_test();
	#endif

	#if 0
	dev_touchscreen_init();
	dev_touchscreen_open();
	ts_calibrate();
	ts_calibrate_test();
	#endif
	//camera_test();
	
	while (1)
	{
		/*驱动轮询*/
		dev_key_scan();
		dev_keypad_scan();
		eth_loop_task();
		fun_sound_task();
		fun_rec_task();

		/*应用*/
		dev_keypad_test();
		u8 key;
		s32 res;
		
		res = dev_key_read(&key, 1);
		if(res == 1)
		{
			if(key == DEV_KEY_PRESS)
			{
				//dev_buzzer_open();
				//dev_dacsound_play();
				//dev_spiflash_test();
				//dev_sdio_test();
				//dev_wm8978_test();
				//dev_lcd_test();
				GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);	
				//dev_tea5767_search(1);
				/*读时间*/
				mcu_rtc_get_date();
				mcu_rtc_get_time();

				dev_i2coledlcd_test();
				//fun_sound_test();
				/*设置时间*/
				//mcu_rtc_set_date(2018, 2, 4, 17);
				//mcu_rtc_set_time(2, 47, 0);
				
			}
			else if(key == DEV_KEY_REL)
			{
				//dev_buzzer_close();

				GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);	
			}
		}
		
		Delay(1);

		/*测试触摸按键*/
		//dev_touchkey_task();
		//dev_touchkey_test();
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
