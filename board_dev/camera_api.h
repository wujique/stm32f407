/**
  ******************************************************************************
  * @file    DCMI/DCMI_CameraExample/camera_api.h 
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   Header for camera_api.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAMERA_API_H
#define __CAMERA_API_H

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx.h"

/* Exported constants --------------------------------------------------------*/
#define DCMI_DR_ADDRESS       0x50050028
#define FSMC_LCD_ADDRESS      0x6C010000//0x68000002 wujique modify
#define NOKEY                 0
#define SEL                   1
#define UP                    2
#define DOWN                  3

/* Exported types ------------------------------------------------------------*/
/* Camera devices enumeration */
typedef enum   
{
  OV9655_CAMERA            =   0x00,	 /* Use OV9655 Camera */
  OV2640_CAMERA            =   0x01      /* Use OV2640 Camera */
}Camera_TypeDef;

/* Image Sizes enumeration */
typedef enum   
{
  BMP_QQVGA             =   0x00,	    /* BMP Image QQVGA 160x120 Size */
  BMP_QVGA              =   0x01,           /* BMP Image QVGA 320x240 Size */
  JPEG_160x120          =   0x02,	    /* JPEG Image 160x120 Size */
  JPEG_176x144          =   0x03,	    /* JPEG Image 176x144 Size */
  JPEG_320x240          =   0x04,	    /* JPEG Image 320x240 Size */
  JPEG_352x288          =   0x05	    /* JPEG Image 352x288 Size */
}ImageFormat_TypeDef;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern s32 dev_camera_init(void);
extern s32 dev_camera_open(void);
extern s32 dev_camera_close(void);
extern s32 camera_test(void);

#endif /* __CAMERA_API_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
