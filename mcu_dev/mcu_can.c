/**
 * @file                mcu_can.c
 * @brief           can总线控制器驱动
 * @author          wujique
 * @date            2017年12月8日 星期五
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年12月8日 星期五
 *   作    者:         wujique
 *   修改内容:   创建文件
*/

#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx.h"

#include "wujique_log.h"

#define MCU_CAN_DEBUG

#ifdef MCU_CAN_DEBUG
#define CAN_DEBUG	wjq_log 
#else
#define CAN_DEBUG(a, ...)
#endif


#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_8
#define CAN_TX_PIN                 GPIO_Pin_9
#define CAN_GPIO_PORT              GPIOB
#define CAN_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define CAN_AF_PORT                GPIO_AF_CAN1
#define CAN_RX_SOURCE              GPIO_PinSource8
#define CAN_TX_SOURCE              GPIO_PinSource9 


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
CAN_InitTypeDef        CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;

CanTxMsg TxMessage;
CanRxMsg RxMessage;
volatile uint8_t CanRxFlag = 0;
/**
  * @brief  Configures the CAN.
  * @param  None
  * @retval None
  */
void mcu_can_config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* CAN GPIOs configuration **************************************************/

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(CAN_GPIO_CLK, ENABLE);

  /* Connect CAN pins to AF9 */
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT); 
  
  /* Configure CAN RX and TX pins */
  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN | CAN_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);

  /* CAN configuration ********************************************************/  
  /* Enable CAN clock */
  RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
  
  /* CAN register init */
  CAN_DeInit(CANx);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = ENABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    
  /* CAN Baudrate = 1 MBps (CAN clocked at 30 MHz) */
  CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
  CAN_InitStructure.CAN_Prescaler = 5;
  CAN_Init(CANx, &CAN_InitStructure);

  /* CAN filter init */
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
   
  /* Enable FIFO 0 message pending Interrupt */
  CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
}

/**
  * @brief  Configures the NVIC for CAN.
  * @param  None
  * @retval None
  */
void NVIC_CAN_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  Initializes the Rx Message.
  * @param  RxMessage: pointer to the message to initialize
  * @retval None
  */
void Init_RxMes(CanRxMsg *RxMessage)
{
  uint8_t ubCounter = 0;

  RxMessage->StdId = 0x00;
  RxMessage->ExtId = 0x00;
  RxMessage->IDE = CAN_ID_STD;
  RxMessage->DLC = 0;
  RxMessage->FMI = 0;
  
  for (ubCounter = 0; ubCounter < 8; ubCounter++)
  {
    RxMessage->Data[ubCounter] = 0x00;
  }
}


/**
 *@brief:      mcu_can1_rx0_IRQ
 *@details:    can总线中断服务
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_can1_rx0_IRQ(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	CanRxFlag = 1;
	CAN_DEBUG(LOG_DEBUG, "can rx message\r\n");
}

extern void Delay(__IO uint32_t nTime);

/**
 *@brief:      mcu_can_test
 *@details:    can总线测试程序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
int mcu_can_test(u8 mode)
{
	uint8_t data = 0;	
  /* NVIC configuration */
  NVIC_CAN_Config();

  /* CAN configuration */
  mcu_can_config();
  
  while(1)
  {
	if(mode == 1)/* 测试发送端，发送后等待接收端响应 */
	{
		Delay(1000);
		data++;
		/* Transmit Structure preparation */
		TxMessage.StdId = 0x321;
		TxMessage.ExtId = 0x01;
		TxMessage.RTR = CAN_RTR_DATA;
		TxMessage.IDE = CAN_ID_STD;
		TxMessage.DLC = 1;
		
	    TxMessage.Data[0] = data;
	    CAN_Transmit(CANx, &TxMessage);
		
	    /* Wait until one of the mailboxes is empty */
	    while((CAN_GetFlagStatus(CANx, CAN_FLAG_RQCP0) !=RESET) || \
	          (CAN_GetFlagStatus(CANx, CAN_FLAG_RQCP1) !=RESET) || \
	          (CAN_GetFlagStatus(CANx, CAN_FLAG_RQCP2) !=RESET));
		
		wjq_log(LOG_FUN, "can transmit :%02x\r\n", data);
		
		while(1)
		{
			if(CanRxFlag == 1)
			{
				CanRxFlag = 0;
				
				if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1))
				{
					
					wjq_log(LOG_FUN, "can rep :%02x\r\n", RxMessage.Data[0]);
					
				}
				
				break;
			}
		}

	}
	else
	{
		/*  测试接收端，接收到数据后返回给发送端 */
		if(CanRxFlag == 1)
		{
			CanRxFlag = 0;
			
			if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1))
			{
				wjq_log(LOG_FUN, "can receive :%02x\r\n", RxMessage.Data[0]);
				/* Transmit Structure preparation */
				TxMessage.StdId = 0x321;
				TxMessage.ExtId = 0x01;
				TxMessage.RTR = CAN_RTR_DATA;
				TxMessage.IDE = CAN_ID_STD;
				TxMessage.DLC = 1;

				TxMessage.Data[0] = RxMessage.Data[0];
				CAN_Transmit(CANx, &TxMessage);

				/* Wait until one of the mailboxes is empty */
				while((CAN_GetFlagStatus(CANx, CAN_FLAG_RQCP0) !=RESET) || \
				      (CAN_GetFlagStatus(CANx, CAN_FLAG_RQCP1) !=RESET) || \
				      (CAN_GetFlagStatus(CANx, CAN_FLAG_RQCP2) !=RESET));
						
					}
			    wjq_log(LOG_FUN, "can send rep ok\r\n");
		}
	}
  }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


