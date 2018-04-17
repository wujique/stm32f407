/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
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
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "eth_app.h"
#include "tcp_echoserver.h"
#include <stdio.h>
#include "wujique_log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int eth_app(void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       168 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */

  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();


  /* Initilaize the LwIP stack */
  LwIP_Init();

  /* tcp echo server Init */
  tcp_echoserver_init();
 
  /* Infinite loop */
  while (1)
  {  
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet*/
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP*/
    LwIP_Periodic_Handle(Time_Get_LocalTime());
  } 

}

s32 EthGd = -2;
/**
 *@brief:      eth_app_init
 *@details:    ³õÊ¼»¯echoserver
 *@param[in]   void  
 *@param[out]  ÎÞ
 *@retval:     
 */
s32 eth_app_init(void)
{
	/*!< At this stage the microcontroller clock setting is already configured to 
		  168 MHz, this is done through SystemInit() function which is called from
		  startup file (startup_stm32f4xx.s) before to branch to application main.
		  To reconfigure the default setting of SystemInit() function, refer to
		  system_stm32f4xx.c file
		*/
	
	 //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	 
	 /* configure ethernet (GPIOs, clocks, MAC, DMA) */
	 ETH_BSP_Config();
	
	
	 /* Initilaize the LwIP stack */
	 LwIP_Init();
	
	 /* tcp echo server Init */
	 tcp_echoserver_init();

	 EthGd = 0;
	 wjq_log(LOG_INFO, "eth init finish!\r\n");
	 
	 return 0;
}

s32 eth_loop_task(void)
{
	if(EthGd != 0)
		return -1;
	
	/* check if any packet received */
	if (ETH_CheckFrameReceived())
	{ 
	  /* process received ethernet packet*/
	  LwIP_Pkt_Handle();
	}
	/* handle periodic timers for LwIP*/
	LwIP_Periodic_Handle(Time_Get_LocalTime());
	
	return 0;
}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
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
  {}
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
