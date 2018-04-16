/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32f4xx_it.h"
#include "main.h"

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern void mcu_uart3_IRQhandler(void);
extern void mcu_tim5_IRQhandler(void);
extern void mcu_tim3_IRQhandler(void);

#include "stm324xg_eval_sdio_sd.h"
extern SD_Error SD_ProcessIRQSrc(void);
extern void SD_ProcessDMAIRQ(void);

extern void mcu_i2s_dma_process(void);
extern void mcu_tim7_IRQhandler(void);
extern void mcu_adc_IRQhandler(void);

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 
void USART3_IRQHandler(void)
{
	mcu_uart3_IRQhandler();    
}

void TIM5_IRQHandler(void)
{
    mcu_tim5_IRQhandler();
}

void TIM3_IRQHandler(void)
{
    mcu_tim3_IRQhandler();
}

void TIM7_IRQHandler(void)
{
    mcu_tim7_IRQhandler();
}

/**
  * @brief  This function handles SDIO global interrupt request.
  * @param  None
  * @retval None
  */
void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}

/**
  * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
  *         requests.
  * @param  None
  * @retval None
  */
//void SD_SDIO_DMA_IRQHANDLER(void)
void DMA2_Stream3_IRQHandler(void)
{
  /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
  SD_ProcessDMAIRQ();
}

void DMA1_Stream4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream4,DMA_IT_TCIF4)==SET)		// 判断是否完成传输
	{ 
		DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_TCIF4);	//	清除相应的标志位
		mcu_i2s_dma_process();		
	} 

}

void ADC_IRQHandler(void)
{
	mcu_adc_IRQhandler();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
