/**
  ******************************************************************************
  * @file    DCMI/DCMI_CameraExample/dcmi_ov9655.c
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   This file includes the driver for OV9655 Camera module mounted on 
  *          STM324xG-EVAL and STM32437I-EVAL evaluation boards.
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
#include "dcmi_ov9655.h"
#include "mcu_dcmi.h"

#include "mcu_i2c.h"

extern void Delay(__IO uint32_t nTime);

/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup DCMI_CameraExample
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  TIMEOUT  2

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* QQVGA 160x120 */
const static unsigned char OV9655_QQVGA[][2]=
{
  0x00, 0x00,
  0x01, 0x80,
  0x02, 0x80,
  0x03, 0x02,
  0x04, 0x03,
  0x09, 0x01,
  0x0b, 0x57,
  0x0e, 0x61,
  0x0f, 0x40,
  0x11, 0x01,
  0x12, 0x62,
  0x13, 0xc7,
  0x14, 0x3a,
  0x16, 0x24,
  0x17, 0x18,
  0x18, 0x04,
  0x19, 0x01,
  0x1a, 0x81,
  0x1e, 0x00,//0x20
  0x24, 0x3c,
  0x25, 0x36,
  0x26, 0x72,
  0x27, 0x08,
  0x28, 0x08,
  0x29, 0x15,
  0x2a, 0x00,
  0x2b, 0x00,
  0x2c, 0x08,
  0x32, 0xa4,
  0x33, 0x00,
  0x34, 0x3f,
  0x35, 0x00,
  0x36, 0x3a,
  0x38, 0x72,
  0x39, 0x57,
  0x3a, 0xcc,
  0x3b, 0x04,
  0x3d, 0x99,
  0x3e, 0x0e,
  0x3f, 0xc1,
  0x40, 0xc0,
  0x41, 0x41,
  0x42, 0xc0,
  0x43, 0x0a,
  0x44, 0xf0,
  0x45, 0x46,
  0x46, 0x62,
  0x47, 0x2a,
  0x48, 0x3c,
  0x4a, 0xfc,
  0x4b, 0xfc,
  0x4c, 0x7f,
  0x4d, 0x7f,
  0x4e, 0x7f,
  0x4f, 0x98,
  0x50, 0x98,
  0x51, 0x00,
  0x52, 0x28,
  0x53, 0x70,
  0x54, 0x98,
  0x58, 0x1a,
  0x59, 0x85,
  0x5a, 0xa9,
  0x5b, 0x64,
  0x5c, 0x84,
  0x5d, 0x53,
  0x5e, 0x0e,
  0x5f, 0xf0,
  0x60, 0xf0,
  0x61, 0xf0,
  0x62, 0x00,
  0x63, 0x00,
  0x64, 0x02,
  0x65, 0x20,
  0x66, 0x00,
  0x69, 0x0a,
  0x6b, 0x5a,
  0x6c, 0x04,
  0x6d, 0x55,
  0x6e, 0x00,
  0x6f, 0x9d,
  0x70, 0x21,
  0x71, 0x78,
  0x72, 0x22,
  0x73, 0x02,
  0x74, 0x10,
  0x75, 0x10,
  0x76, 0x01,
  0x77, 0x02,
  0x7A, 0x12,
  0x7B, 0x08,
  0x7C, 0x16,
  0x7D, 0x30,
  0x7E, 0x5e,
  0x7F, 0x72,
  0x80, 0x82,
  0x81, 0x8e,
  0x82, 0x9a,
  0x83, 0xa4,
  0x84, 0xac,
  0x85, 0xb8,
  0x86, 0xc3,
  0x87, 0xd6,
  0x88, 0xe6,
  0x89, 0xf2,
  0x8a, 0x24,
  0x8c, 0x80,
  0x90, 0x7d,
  0x91, 0x7b,
  0x9d, 0x02,
  0x9e, 0x02,
  0x9f, 0x7a,
  0xa0, 0x79,
  0xa1, 0x40,
  0xa4, 0x50,
  0xa5, 0x68,
  0xa6, 0x4a,
  0xa8, 0xc1,
  0xa9, 0xef,
  0xaa, 0x92,
  0xab, 0x04,
  0xac, 0x80,
  0xad, 0x80,
  0xae, 0x80,
  0xaf, 0x80,
  0xb2, 0xf2,
  0xb3, 0x20,
  0xb4, 0x20,
  0xb5, 0x00,
  0xb6, 0xaf,
  0xb6, 0xaf,
  0xbb, 0xae,
  0xbc, 0x7f,
  0xbd, 0x7f,
  0xbe, 0x7f,
  0xbf, 0x7f,
  0xbf, 0x7f,
  0xc0, 0xaa,
  0xc1, 0xc0,
  0xc2, 0x01,
  0xc3, 0x4e,
  0xc6, 0x05,
  0xc7, 0x82,
  0xc9, 0xe0,
  0xca, 0xe8,
  0xcb, 0xf0,
  0xcc, 0xd8,
  0xcd, 0x93,

  0x12, 0x63,/* Set the RGB565 mode */
  0x40, 0x10,
  0x15, 0x08,/* Invert the HRef signal*/
};

/* QVGA 360x240 */
const static unsigned char OV9655_QVGA[][2]=
	{
	  0x00, 0x00,
	  0x01, 0x80,
	  0x02, 0x80,
	  0x03, 0x02,
	  0x04, 0x03,
	  0x09, 0x01,
	  0x0b, 0x57,
	  0x0e, 0x61,
	  0x0f, 0x40, 
	  0x11, 0x01,
	  0x12, 0x62,
	  0x13, 0xc7,
	  0x14, 0x3a,
	  0x16, 0x24,
	  0x17, 0x18,
	  0x18, 0x04,
	  0x19, 0x01,
	  0x1a, 0x81,
	  0x1e, 0x00,
	  0x24, 0x3c,
	  0x25, 0x36,
	  0x26, 0x72,
	  0x27, 0x08,
	  0x28, 0x08,
	  0x29, 0x15,
	  0x2a, 0x00,
	  0x2b, 0x00,
	  0x2c, 0x08,
	  0x32, 0x12,
	  0x33, 0x00,
	  0x34, 0x3f,
	  0x35, 0x00,
	  0x36, 0x3a,
	  0x38, 0x72,
	  0x39, 0x57,
	  0x3a, 0xcc,
	  0x3b, 0x04,
	  0x3d, 0x99,
	  0x3e, 0x02,
	  0x3f, 0xc1,
	  0x40, 0xc0,
	  0x41, 0x41,
	  0x42, 0xc0,
	  0x43, 0x0a,
	  0x44, 0xf0,
	  0x45, 0x46,
	  0x46, 0x62,
	  0x47, 0x2a,
	  0x48, 0x3c,
	  0x4a, 0xfc,
	  0x4b, 0xfc,
	  0x4c, 0x7f,
	  0x4d, 0x7f,
	  0x4e, 0x7f,
	  0x4f, 0x98,
	  0x50, 0x98,
	  0x51, 0x00,
	  0x52, 0x28,
	  0x53, 0x70,
	  0x54, 0x98,
	  0x58, 0x1a,
	  0x59, 0x85,
	  0x5a, 0xa9,
	  0x5b, 0x64,
	  0x5c, 0x84,
	  0x5d, 0x53,
	  0x5e, 0x0e,
	  0x5f, 0xf0,
	  0x60, 0xf0,
	  0x61, 0xf0,
	  0x62, 0x00,
	  0x63, 0x00,
	  0x64, 0x02,
	  0x65, 0x20,
	  0x66, 0x00,
	  0x69, 0x0a,
	  0x6b, 0x5a,
	  0x6c, 0x04,
	  0x6d, 0x55,
	  0x6e, 0x00,
	  0x6f, 0x9d,
	  0x70, 0x21,
	  0x71, 0x78,
	  0x72, 0x11,
	  0x73, 0x01,
	  0x74, 0x10,
	  0x75, 0x10,
	  0x76, 0x01,
	  0x77, 0x02,
	  0x7A, 0x12,
	  0x7B, 0x08,
	  0x7C, 0x16,
	  0x7D, 0x30,
	  0x7E, 0x5e,
	  0x7F, 0x72,
	  0x80, 0x82,
	  0x81, 0x8e,
	  0x82, 0x9a,
	  0x83, 0xa4,
	  0x84, 0xac,
	  0x85, 0xb8,
	  0x86, 0xc3,
	  0x87, 0xd6,
	  0x88, 0xe6,
	  0x89, 0xf2,
	  0x8a, 0x24,
	  0x8c, 0x80,
	  0x90, 0x7d,
	  0x91, 0x7b,
	  0x9d, 0x02,
	  0x9e, 0x02,
	  0x9f, 0x7a,
	  0xa0, 0x79,
	  0xa1, 0x40,
	  0xa4, 0x50,
	  0xa5, 0x68,
	  0xa6, 0x4a,
	  0xa8, 0xc1,
	  0xa9, 0xef,
	  0xaa, 0x92,
	  0xab, 0x04,
	  0xac, 0x80,
	  0xad, 0x80,
	  0xae, 0x80,
	  0xaf, 0x80,
	  0xb2, 0xf2,
	  0xb3, 0x20,
	  0xb4, 0x20,
	  0xb5, 0x00,
	  0xb6, 0xaf,
	  0xb6, 0xaf,
	  0xbb, 0xae,
	  0xbc, 0x7f,
	  0xbd, 0x7f,
	  0xbe, 0x7f,
	  0xbf, 0x7f,
	  0xbf, 0x7f,
	  0xc0, 0xaa,
	  0xc1, 0xc0,
	  0xc2, 0x01,
	  0xc3, 0x4e,
	  0xc6, 0x05,
	  0xc7, 0x81,
	  0xc9, 0xe0,
	  0xca, 0xe8,
	  0xcb, 0xf0,
	  0xcc, 0xd8,
	  0xcd, 0x93,
	
	  0x12, 0x63,/* Set the RGB565 mode */
	  0x40, 0x10,
	  0x15, 0x08,/* Invert the HRef signal*/
};

/**
  * @brief  Resets the OV9655 camera.
  * @param  None
  * @retval None
  */
void OV9655_Reset(void)
{
  OV9655_WriteReg(OV9655_COM7, 0x80);
}

/**
  * @brief  Reads the OV9655 Manufacturer identifier.
  * @param  OV9655ID: pointer to the OV9655 Manufacturer identifier.
  * @retval None
  */
void OV9655_ReadID(OV9655_IDTypeDef* OV9655ID)
{
  OV9655ID->Manufacturer_ID1 = OV9655_ReadReg(OV9655_MIDH);
  OV9655ID->Manufacturer_ID2 = OV9655_ReadReg(OV9655_MIDL);
  OV9655ID->Version = OV9655_ReadReg(OV9655_VER);
  OV9655ID->PID = OV9655_ReadReg(OV9655_PID);
}

/**
  * @brief  Configures the DCMI/DMA to capture image from the OV9655 camera.
  * @param  ImageFormat: Image format BMP or JPEG
  * @param  BMPImageSize: BMP Image size  
  * @retval None
  */
void OV9655_Init(ImageFormat_TypeDef ImageFormat)
{
  DCMI_InitTypeDef DCMI_InitStructure;

	 
  /*** Configures the DCMI to interface with the OV9655 camera module ***/
  /* Enable DCMI clock */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);

  /* DCMI configuration */ 
  DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_Continuous;
  DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
  DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Falling;
  DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_High;
  DCMI_InitStructure.DCMI_HSPolarity = DCMI_HSPolarity_High;
  DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
  DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
  
//用中断可以统计帧率
/* DCMI Interrupts config ***************************************************/
  //DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);
  //sys_NVIC_set(DCMI_IRQn, 1, 1);
  
  switch(ImageFormat)
  {
    case BMP_QQVGA:
    {
      /* DCMI configuration */
      DCMI_Init(&DCMI_InitStructure);

      BUS_DCMI_DMA_Init(FSMC_LCD_ADDRESS, 1, DMA_MemoryInc_Disable, DMA_MemoryDataSize_HalfWord);
      break;
    }
    case BMP_QVGA:
    {
      /* DCMI configuration */ 
      DCMI_Init(&DCMI_InitStructure);

      /* DMA2 IRQ channel Configuration */
      BUS_DCMI_DMA_Init(FSMC_LCD_ADDRESS, 1, DMA_MemoryInc_Disable, DMA_MemoryDataSize_HalfWord); 
      break;
    }
    default:
    {
      /* DCMI configuration */ 
      DCMI_Init(&DCMI_InitStructure);

      /* DMA2 IRQ channel Configuration */
      BUS_DCMI_DMA_Init(FSMC_LCD_ADDRESS, 1, DMA_MemoryInc_Disable, DMA_MemoryDataSize_HalfWord);
      break;
    }
  }    
}

/**
  * @brief  Configures the OV9655 camera in QQVGA mode.
  * @param  None
  * @retval None
  */
void OV9655_QQVGAConfig(void)
{
  uint32_t i;

  OV9655_Reset();
  Delay(200);

  /* Initialize OV9655 */
  for(i=0; i<(sizeof(OV9655_QQVGA)/2); i++)
  {
    OV9655_WriteReg(OV9655_QQVGA[i][0], OV9655_QQVGA[i][1]);
    Delay(2);
  }
}

/**
  * @brief  SConfigures the OV9655 camera in QVGA mode.
  * @param  None
  * @retval None
  */
void OV9655_QVGAConfig(void)
{
  uint32_t i;

  OV9655_Reset();
  Delay(200);

  /* Initialize OV9655 */
  for(i=0; i<(sizeof(OV9655_QVGA)/2); i++)
  {
    OV9655_WriteReg(OV9655_QVGA[i][0], OV9655_QVGA[i][1]);
    Delay(2);
  }
}

/**
  * @brief  Configures the OV9655 camera brightness.
  * @param  Brightness: Brightness value, where Brightness can be: 
  *         positively (0x01 ~ 0x7F) and negatively (0x80 ~ 0xFF)
  * @retval None
  */
void OV9655_BrightnessConfig(uint8_t Brightness)
{
  OV9655_WriteReg(OV9655_BRTN, Brightness);
}

/**
  * @brief  Writes a byte at a specific Camera register
  * @param  Addr: OV9655 register address
  * @param  Data: data to be written to the specific register
  * @retval 0x00 if write operation is OK
  *         0xFF if timeout condition occurred (device not connected or bus error).
  */
uint8_t OV9655_WriteReg(uint16_t Addr, uint8_t Data)
{

	return bus_sccb_writereg(OV9655_DEVICE_WRITE_ADDRESS, Addr, Data);

}

/**
  * @brief  Reads a byte from a specific Camera register
  * @param  Addr: OV9655 register address.
  * @retval data read from the specific register or 0xFF if timeout condition
  *         occurred.
  */
uint8_t OV9655_ReadReg(uint16_t Addr)
{
	return bus_sccb_readreg(OV9655_DEVICE_READ_ADDRESS, Addr);

}


/**
  * @brief  Set the Internal Clock Prescaler.
  * @param  OV9655_Prescaler: the new value of the prescaler. 
  *         This parameter can be a value between 0x0 and 0x1F
  * @retval None
  */
void OV9655_SetPrescaler(uint8_t OV9655_Prescaler)
{
  OV9655_WriteReg(OV9655_CLKRC, OV9655_Prescaler);
}

/**
  * @brief  Select the Output Format.
  * @param  OV9655_OuputFormat: the Format of the ouput Data.  
  *         This parameter can be one of the following values:
  *           @arg OUTPUT_FORMAT_RAWRGB_DATA 
  *           @arg OUTPUT_FORMAT_RAWRGB_INTERP    
  *           @arg OUTPUT_FORMAT_YUV              
  *           @arg OUTPUT_FORMAT_RGB    
  * @retval None
  */
void OV9655_SelectOutputFormat(uint8_t OV9655_OuputFormat)
{
  OV9655_WriteReg(OV9655_COM7, OV9655_OuputFormat);
}


/**
  * @brief  Select the Output Format Resolution.
  * @param  OV9655_FormatResolution: the Resolution of the ouput Data. 
  *         This parameter can be one of the following values:
  *           @arg FORMAT_CTRL_15fpsVGA 
  *           @arg FORMAT_CTRL_30fpsVGA_NoVArioPixel    
  *           @arg FORMAT_CTRL_30fpsVGA_VArioPixel     
  * @retval None
  */
void OV9655_SelectFormatResolution(uint8_t OV9655_FormatResolution)
{
  OV9655_WriteReg(OV9655_COM7, OV9655_FormatResolution);
}

/**
  * @brief  Select the HREF Control signal option
  * @param  OV9665_HREFControl: the HREF Control signal option.
  *         This parameter can be one of the following value:
  *           @arg OV9665_HREFControl_Opt1: HREF edge offset to data output. 
  *           @arg OV9665_HREFControl_Opt2: HREF end 3 LSB    
  *           @arg OV9665_HREFControl_Opt3: HREF start 3 LSB      
  * @retval None
  */
void OV9655_HREFControl(uint8_t OV9665_HREFControl)
{
  OV9655_WriteReg(OV9655_HREF, OV9665_HREFControl);
}

/**
  * @brief  Select the RGB format option
  * @param  OV9665_RGBOption: the RGB Format option.
  *         This parameter can be one of the following value:
  *           @arg RGB_NORMAL
  *           @arg RGB_565  
  *           @arg RGB_555    
  * @retval None
  */
void OV9655_SelectRGBOption(uint8_t OV9665_RGBOption)
{
  OV9655_WriteReg(OV9655_COM15, OV9665_RGBOption);
}


/**
  * @}
  */ 

/**
  * @}
  */ 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
