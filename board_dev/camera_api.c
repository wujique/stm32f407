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

const uint8_t *ImageForematArray[] =
{
  (uint8_t*)"BMP QQVGA Format    ",
  (uint8_t*)"BMP QVGA Format     ",
  (uint8_t*)"JPEG Image 160x120 Size    ",
  (uint8_t*)"JPEG Image 176x144 Size     ",
  (uint8_t*)"JPEG Image 320x240 Size     ",
  (uint8_t*)" JPEG Image 352x288 Size    ",
};


Camera_TypeDef Camera;
ImageFormat_TypeDef ImageFormat;
OV9655_IDTypeDef  OV9655_Camera_ID;
OV2640_IDTypeDef  OV2640_Camera_ID;
s32 CameraGd = -2;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 *@brief:	   dev_camera_config
 *@details:    配置摄像头
 *@param[in]   Format 图像格式
			   Dst0 数据目标地址0
			   Dst1 数据目标地址1，如果是直接发送到LCD，地址1为null
		       size buff长度，单位word(u32)
 *@param[out]  无
 *@retval:	   
 */

void dev_camera_config(ImageFormat_TypeDef Format, u32 Dst0, u32 Dst1, u32 size)
{	
	if(Camera == OV9655_CAMERA)
	{
		OV9655_Init(Format);	
	}
	else if(Camera == OV2640_CAMERA)
	{
		OV2640_Init(Format);	
	}

	BUS_DCMI_DMA_Init(Dst0, Dst1, size);

}
/**
 *@brief:	   dev_camera_fresh
 *@details:    启动摄像头数据传输
 *@param[in]   
 *@param[out]  无
 *@retval:	   
 */
s32 dev_camera_fresh(void)
{
	/* Enable DMA2 stream 1 and DCMI interface then start image capture */
	DMA_Cmd(DMA2_Stream1, ENABLE); 
	DCMI_Cmd(ENABLE); 
	DCMI_CaptureCmd(ENABLE);	
}
/**
 *@brief:	   dev_camera_stop
 *@details:    停止摄像头数据传输
 *@param[in]   
 *@param[out]  无
 *@retval:	   
 */
s32 dev_camera_stop(void)
{
	DMA_Cmd(DMA2_Stream1, DISABLE); 
	DCMI_Cmd(DISABLE); 
	DCMI_CaptureCmd(DISABLE); 	
}
/**
 *@brief:	   dev_camera_init
 *@details:    初始化摄像头
 *@param[in]   
 *@param[out]  无
 *@retval:	   
 */
s32 dev_camera_init(void)
{
	#ifdef SYS_USE_CAMERA
	/* camera xclk use the MCO1 */
	MCO1_Init();
	DCMI_PWDN_RESET_Init();
	
	/* Initializes the DCMI interface (I2C and GPIO) used to configure the camera */
	BUS_DCMI_HW_Init();
	SCCB_GPIO_Config();

	/* Read the OV9655/OV2640 Manufacturer identifier */
	OV9655_ReadID(&OV9655_Camera_ID);
	OV2640_ReadID(&OV2640_Camera_ID);

	if(OV9655_Camera_ID.PID  == 0x96)
	{
		Camera = OV9655_CAMERA;
		wjq_log(LOG_DEBUG, "OV9655 Camera ID 0x%x\r\n", OV9655_Camera_ID.PID);
	}
	else if(OV2640_Camera_ID.PIDH  == 0x26)
	{
		Camera = OV2640_CAMERA;
		wjq_log(LOG_DEBUG, "OV2640 Camera ID 0x%x\r\n", OV2640_Camera_ID.PIDH);
	}
	else
	{
		wjq_log(LOG_DEBUG, "Check the Camera HW and try again\r\n");
		return -1;  
	}
	
	CameraGd = -1;
	#else
	wjq_log(LOG_INFO, ">---------sys no use camera\r\n");
	#endif
	return 0;
}
/**
 *@brief:	   dev_camera_open
 *@details:    打开摄像头
 *@param[in]   
 *@param[out]  无
 *@retval:	   
 */
s32 dev_camera_open(void)
{
	if(CameraGd!= -1)
		return -1;

	CameraGd = 0;
	return 0;	
}
/**
 *@brief:	   dev_camera_close
 *@details:    关闭摄像头
 *@param[in]   
 *@param[out]  无
 *@retval:	   
 */
s32 dev_camera_close(void)
{
	if(CameraGd!= 0)
		return -1;

	CameraGd = -1;
	dev_camera_stop(); 
	return 0;
}

/**
  * @}

	ST官方例程的main函数，移植作为一个测试函数。
  
  */ 
u16 *camera_buf0;
u16 *camera_buf1;

#define LCD_BUF_SIZE 320*2*2//一次传输2行数据，一个像素2个字节
u16 DmaCnt;

#define CAMERA_USE_RAM2LCD	1

s32 dev_camera_show(DevLcdNode *lcd)
{
	uint8_t abuffer[40];
  	s32 ret;
	volatile u8 sta;

	/* 选择图片格式 */
	ImageFormat = (ImageFormat_TypeDef)BMP_QVGA;
	wjq_log(LOG_DEBUG, " Image selected: %s", ImageForematArray[ImageFormat]);

	#ifndef CAMERA_USE_RAM2LCD
	/*直接投到LCD*/
	dev_camera_config(ImageFormat, FSMC_LCD_ADDRESS, NULL, 1);
	#else
	/*放到buf*/
	camera_buf0 = wjq_malloc(LCD_BUF_SIZE);
	camera_buf1 = wjq_malloc(LCD_BUF_SIZE);
	/*除4的原因：LCD_BUF_SIZE是u8，DMA数据源是u32，目标数据是u16*/
	dev_camera_config(ImageFormat, (u32)camera_buf0, (u32)camera_buf1, LCD_BUF_SIZE/4);
	#endif

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
	DmaCnt = 0;
	 dev_camera_fresh();

	while(1)
	{
		
		mcu_dcmi_get_sta(&sta);

		#ifdef CAMERA_USE_RAM2LCD
		if(DCMI_FLAG_BUF0 == (sta&DCMI_FLAG_BUF0))
		{
			dev_lcd_flush(lcd, camera_buf0, LCD_BUF_SIZE/2);
			DmaCnt++;
		}
		if(DCMI_FLAG_BUF1 == (sta&DCMI_FLAG_BUF1))
		{	
			dev_lcd_flush(lcd, camera_buf1, LCD_BUF_SIZE/2);
			DmaCnt++;
		}
		#endif
		/*一定要先检测数据再检测帧完成，最有一次两个中断差不多同时来*/
		if(DCMI_FLAG_FRAME == (sta&DCMI_FLAG_FRAME))
		{
			wjq_log(LOG_DEBUG, "-f-%d- ", DmaCnt);
			DmaCnt = 0;
			dev_camera_fresh();
		}
	}
	return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

