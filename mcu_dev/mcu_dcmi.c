

#include "stm32f4xx.h"
#include "mcu_dcmi.h"



/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

#define DCMI_DR_ADDRESS       0x50050028

extern void Delay(__IO uint32_t nTime);

/**
  * @brief  Initializes the hardware resources (I2C and GPIO) used to configure 
  *         the OV9655 camera.
  * @param  None
  * @retval None
  */
void BUS_DCMI_HW_Init(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

	/*** Configures the DCMI GPIOs to interface with the OV9655 camera module ***/

	/*
	HSYNC -------PA4;	 
	PCLK -------- PA6; 
	D0 -----------PA9;	
	
	D5 -----------PB6;	
	VSYNC -------PB7; 
	
	D1 -----------PC7;
	
	D2 -----------PE0;	
	D3 -----------PE1;	
	D4 -----------PE4
	D6 -----------PE5;	 
	D7 -----------PE6

	*/
	
	/* Enable DCMI GPIOs clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
						 RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);

  /* Connect DCMI pins to AF13 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI);//HSYNC
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI);//PCLK
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_DCMI);//D0
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_DCMI);//D5
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI);//VSYNC
  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI);//D1
  
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_DCMI);//D2
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_DCMI);//D3
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_DCMI);//D4
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_DCMI);//D6
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_DCMI);//D7

  
  /* DCMI GPIO configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_4 |GPIO_Pin_5 | GPIO_Pin_6;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

}


void BUS_DCMI_Config(u16 pck, u16 vs, u16 hs)
{
	DCMI_InitTypeDef DCMI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable DCMI clock */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);

	/* DCMI configuration */ 
	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;
	DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
	DCMI_InitStructure.DCMI_PCKPolarity = pck;
	DCMI_InitStructure.DCMI_VSPolarity = vs;
	DCMI_InitStructure.DCMI_HSPolarity = hs;
	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
	DCMI_Init(&DCMI_InitStructure);
	
	/* DCMI 帧中断 */
	DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
}

void BUS_DCMI_DMA_Init(u32 Memory0BaseAddr, u32 Memory1BaseAddr, u32 BufferSize)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Configures the DMA2 to transfer Data from DCMI */
	/* Enable DMA2 clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	/* DMA2 Stream1 Configuration */
	DMA_DeInit(DMA2_Stream1);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;	
	DMA_InitStructure.DMA_PeripheralBaseAddr = DCMI_DR_ADDRESS;   
	DMA_InitStructure.DMA_Memory0BaseAddr = Memory0BaseAddr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	
	if(BufferSize == 1)
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;	
	else
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	//	存储器增量模式
		
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	/* DMA2 IRQ channel Configuration */
    DMA_Init(DMA2_Stream1, &DMA_InitStructure);

	if(Memory1BaseAddr != NULL)
  	{	
	    DMA_DoubleBufferModeConfig(DMA2_Stream1,(uint32_t)Memory0BaseAddr, DMA_Memory_0);	//	配置缓冲区0
		DMA_DoubleBufferModeConfig(DMA2_Stream1,(uint32_t)Memory1BaseAddr, DMA_Memory_1);	//	配置缓冲区1
		DMA_DoubleBufferModeCmd(DMA2_Stream1,ENABLE);										//	开启双缓冲模式
		DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);	//开启传输完成中断
		
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

	}
}

void DCMI_PWDN_RESET_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/*PWDN*/
	GPIO_ResetBits(GPIOE, GPIO_Pin_2);
	Delay(10);
	/* reset */
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	Delay(10);
	GPIO_SetBits(GPIOE, GPIO_Pin_3);	
	
}

/**
  * @brief  Set PA8 Output SYSCLK/2.
  * @param  None
  * @retval None
  */
void MCO1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_ClockSecuritySystemCmd(ENABLE);

	/* Enable GPIOs clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);

	/* Configure MCO (PA8) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC_MCO1Config(RCC_MCO1Source_HSI, RCC_MCO1Div_1);
}

static u8 DcmiFlag = 0;

/*
	dcmi帧中断服务函数
*/
s32 mcu_dcmi_frame_process(void)
{
	DcmiFlag |= DCMI_FLAG_FRAME;
	return 0;
}
/*
	dcmi dma传输完成中断（一个buf满）
*/
s32 mcu_dcmi_dma_process(void)
{
	if(DMA2_Stream1->CR&(1<<19))//DMA使用buf1,读取buf0
	{ 
 		DcmiFlag |= DCMI_FLAG_BUF0;
	}
	else 					//DMA使用buf0,读取buf1
	{
		DcmiFlag |= DCMI_FLAG_BUF1;
	}
	
	return 0;
}
/*
	获取dcmi传输状态
*/
s32 mcu_dcmi_get_sta(u8 *sta)
{
	*sta = DcmiFlag;
	if(DcmiFlag != 0x00)
		DcmiFlag = 0x00;
	return 0;
}

