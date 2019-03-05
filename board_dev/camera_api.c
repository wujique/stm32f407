/**
  ******************************************************************************
  * @file    DCMI/DCMI_CameraExample/camera_api.c 
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   This file contains the routine needed to configure OV9655/OV2640 
  *          Camera modules.
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
#include "camera_api.h"
#include "dcmi_ov9655.h"
#include "dcmi_ov2640.h"
#include "mcu_dcmi.h"
#include "mcu_i2c.h"

#include "dev_lcd.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"


extern void Delay(__IO uint32_t nTime);

/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup DCMI_CameraExample
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Image Formats */

u8 CameraFlag = 0;

s32 dev_carema_dcmi_process(void)
{
	DMA_Cmd(DMA2_Stream1, DISABLE); 
	DCMI_Cmd(DISABLE); 
	DCMI_CaptureCmd(DISABLE); 
	CameraFlag = 1;
	return 0;
}



const uint8_t *ImageForematArray[] =
{
  (uint8_t*)"BMP QQVGA Format    ",
  (uint8_t*)"BMP QVGA Format     ",
  (uint8_t*)"JPEG Image 160x120 Size    ",
  (uint8_t*)"JPEG Image 176x144 Size     ",
  (uint8_t*)"JPEG Image 320x240 Size     ",
  (uint8_t*)" JPEG Image 352x288 Size    ",
};


uint8_t ValueMin = 0, ValueMax = 0;
Camera_TypeDef Camera;
ImageFormat_TypeDef ImageFormat;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Configures OV9655 or OV2640 Camera module mounted on STM324xG-EVAL board.
  * @param  ImageBuffer: Pointer to the camera configuration structure
  * @retval None
  */
void Camera_Config(void)
{
  if(Camera == OV9655_CAMERA)
  {
    switch (ImageFormat)
    {
      case BMP_QQVGA:
      {
        /* Configure the OV9655 camera and set the QQVGA mode */
        OV9655_Init(BMP_QQVGA);
        OV9655_QQVGAConfig();
        break;
      }
      case BMP_QVGA:
      {
        /* Configure the OV9655 camera and set set the QVGA mode */
        OV9655_Init(BMP_QVGA);
        OV9655_QVGAConfig();
        break;
      }
      default:
      {
        /* Configure the OV9655 camera and set the QQVGA mode */
        OV9655_Init(BMP_QQVGA);
        OV9655_QQVGAConfig();
        break;
      } 
    }
  }
  else if(Camera == OV2640_CAMERA)
  {
    switch (ImageFormat)
    {
      case BMP_QQVGA:
      {
        /* Configure the OV2640 camera and set the QQVGA mode */
        OV2640_Init(BMP_QQVGA);
        OV2640_QQVGAConfig();
        break;
      }
	  
      case BMP_QVGA:
      {
        /* Configure the OV2640 camera and set the QQVGA mode */
        OV2640_Init(BMP_QVGA);
        OV2640_QVGAConfig();
        break;
      }
	  
	  case JPEG_160x120:
	  case JPEG_176x144:
	  case JPEG_320x240:
	  case JPEG_352x288:
      {
        /* Configure the OV2640 camera and set the JPEG mode */
        OV2640_Init(ImageFormat);
        OV2640_JPEGConfig(ImageFormat);
        break;
      }
	  
      default:
      {
        /* Configure the OV2640 camera and set the QQVGA mode */
        OV2640_Init(BMP_QQVGA);
        OV2640_QQVGAConfig();
        break; 
      }
    }
  }
}
/**
  * @brief  OV2640 camera special effects.
* @param  index: 
  * @retval None
  */
void OV2640_SpecialEffects(uint8_t index)
{
  switch (index)
  {
    case 1:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Antique               ");
      OV2640_ColorEffectsConfig(0x40, 0xa6);/* Antique */ 
      break;
    }
    case 2:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Bluish                ");
      OV2640_ColorEffectsConfig(0xa0, 0x40);/* Bluish */
      break;
    }
    case 3:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Greenish              ");
      OV2640_ColorEffectsConfig(0x40, 0x40);/* Greenish */
      break;
    }
    case 4:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Reddish               ");
      OV2640_ColorEffectsConfig(0x40, 0xc0);/* Reddish */
      break;
    }
    case 5:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Black & White         ");
      OV2640_BandWConfig(0x18);/* Black & White */
      break;
    }
    case 6:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Negative              ");
      OV2640_BandWConfig(0x40);/* Negative */
      break;
    }
    case 7:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Black & White negative");
      OV2640_BandWConfig(0x58);/* B&W negative */
      break;
    }
    case 8:
    {
      //LCD_DisplayStringLine(LINE(16), (uint8_t*)" Normal                ");
      OV2640_BandWConfig(0x00);/* Normal */
      break;
    }
    default:
      break;
  }
}

s32 CameraGd = -2;

s32 dev_camera_init(void)
{
	#ifdef SYS_USE_CAMERA
	/* camera xclk use the MCO1 */
	MCO1_Init();
	DCMI_PWDN_RESET_Init();
	
	/* Initializes the DCMI interface (I2C and GPIO) used to configure the camera */
	BUS_DCMI_HW_Init();
	
	SCCB_GPIO_Config();
	CameraGd = -1;
	#else
	wjq_log(LOG_INFO, ">---------sys no use camera\r\n");
	#endif
	return 0;
}
s32 dev_camera_open(void)
{
	if(CameraGd!= -1)
		return -1;

	CameraGd = 0;
	return 0;	
}
s32 dev_camera_close(void)
{
	if(CameraGd!= 0)
		return -1;

	CameraGd = -1;
	DMA_Cmd(DMA2_Stream1, DISABLE); 
	DCMI_Cmd(DISABLE); 
	/* Insert 100ms delay: wait 100ms */ 
	DCMI_CaptureCmd(DISABLE); 
	return 0;
}
/**
  * @}

	ST官方例程的main函数，移植作为一个测试函数。
  
  */ 

OV9655_IDTypeDef  OV9655_Camera_ID;
OV2640_IDTypeDef  OV2640_Camera_ID;

u16 *ov2640_data;

s32 dev_camera_show(DevLcdNode *lcd)
{
	uint8_t abuffer[40];
  
	if(CameraGd!= 0)
		return -1;

	wjq_log(LOG_FUN, "camera test....\r\n");
	/* Read the OV9655/OV2640 Manufacturer identifier */
	OV9655_ReadID(&OV9655_Camera_ID);
	OV2640_ReadID(&OV2640_Camera_ID);

	if(OV9655_Camera_ID.PID  == 0x96)
	{
		Camera = OV9655_CAMERA;
		sprintf((char*)abuffer, "OV9655 Camera ID 0x%x", OV9655_Camera_ID.PID);
		ValueMax = 2;
	}
	else if(OV2640_Camera_ID.PIDH  == 0x26)
	{
		Camera = OV2640_CAMERA;
		sprintf((char*)abuffer, "OV2640 Camera ID 0x%x", OV2640_Camera_ID.PIDH);
		ValueMax = 2;
	}
	else
	{
		put_string_center(lcd, 160, 120, (char *)"Check the Camera HW and try again", 0xf880);
		wjq_log(LOG_FUN, "Check the Camera HW and try again\r\n");
		Delay(1000);
		return -1;  
	}

	put_string_center(lcd, 160, 120, (char *)abuffer, 0xf880);
	wjq_log(LOG_FUN, "%s\r\n", abuffer);
	Delay(200);
	
	/* Initialize demo */
	ImageFormat = (ImageFormat_TypeDef)BMP_QVGA;

	/* Configure the Camera module mounted on STM324xG-EVAL/STM324x7I-EVAL boards */
	wjq_log(LOG_FUN, "Camera_Config...\r\n");
	Camera_Config();
	
	sprintf((char*)abuffer, " Image selected: %s", ImageForematArray[ImageFormat]);
	put_string_center(lcd, 160, 160, (char *)abuffer, 0xf880);

	CameraFlag = 0;
	
	if(ImageFormat == BMP_QQVGA)
	{
		/*  
			设置显示区域，用的就是设置显示指针的指令，通常我们只是设置起始，
			没有设置结束
		*/
		/* LCD Display window */
		dev_lcd_setdir(lcd, W_LCD, L2R_U2D);
		dev_lcd_prepare_display(lcd, 1, 160, 1, 120);
	}
	else if(ImageFormat == BMP_QVGA)
	{
		/* LCD Display window */
		dev_lcd_setdir(lcd, W_LCD, L2R_U2D);
		dev_lcd_prepare_display(lcd, 1, 320, 1, 240);
	}	

	/* Enable DMA2 stream 1 and DCMI interface then start image capture */
	DMA_Cmd(DMA2_Stream1, ENABLE); 
	DCMI_Cmd(ENABLE); 
	/* Insert 100ms delay: wait 100ms */
	Delay(100); 
	DCMI_CaptureCmd(ENABLE); 

		return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
