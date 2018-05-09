/**
  ******************************************************************************
  * @file    dual_func_demo.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    09-November-2015
  * @brief   This file contain the demo implementation
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
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
#include <string.h>
#include "dual_func_demo.h"
#include "usbh_core.h"
#include "usbh_msc_usr.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbh_msc_core.h"
#include "usbd_msc_core.h"

#include "usb_conf.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"


/** @addtogroup USBH_USER
* @{
*/

/** @addtogroup USBH_DUAL_FUNCT_DEMO
* @{
*/

/** @defgroup USBH_DUAL_FUNCT_DEMO 
* @brief    This file includes the usb host stack user callbacks
* @{
*/ 

/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Defines
* @{
*/ 
#define IMAGE_BUFFER_SIZE  512

#define BF_TYPE 0x4D42             /* "MB" */

#define BI_RGB       0             /* No compression - straight BGR data */
#define BI_RLE8      1             /* 8-bit run-length compression */
#define BI_RLE4      2             /* 4-bit run-length compression */
#define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */

/**
* @}
*/ 


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Macros
* @{
*/ 

/**
* @}
*/ 


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Variables
* @{
*/ 


uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;
uint8_t Image_Buf[IMAGE_BUFFER_SIZE];

uint8_t filenameString[15]  = {0};
FATFS fatfs;
FIL file;
DEMO_StateMachine             demo;
uint8_t line_idx = 0; 
__IO uint8_t wait_user_input = 0; 
uint8_t Enum_Done      = 0;   
uint8_t * DEMO_main_menu[] = 
{
	(uint8_t*)"      1 - Host Demo",
	(uint8_t*)"      2 - Device Demo",
	(uint8_t*)"      3 - Credits",
};

uint8_t * DEMO_HOST_menu[] = 
{
	(uint8_t*)"      1 - Explore Flash content",
	(uint8_t*)"      2 - Write File to disk                                                 ",
	(uint8_t*)"      3 - Show BMP file                                                      ",
	(uint8_t*)"      4 - Return                                                             ",
};

uint8_t * DEMO_DEVICE_menu[] = 
{
	(uint8_t*)"      1 - Return                                                             ",
	(uint8_t*)"                                                                             ",
	(uint8_t*)"                                                                             ",
};

uint8_t writeTextBuff[] = "STM32 Connectivity line Host Demo application using FAT_FS   ";  

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_Core __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST               USB_Host __ALIGN_END ;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_FS_Core __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST                     USB_FS_Host __ALIGN_END ;

/**
* @}
*/

/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Constants
* @{
*/ 
static void Demo_Application (void);
static void Demo_SelectItem (uint8_t **menu , uint8_t item);
static uint8_t Explore_Disk (char* path , uint8_t recu_level);
static void Toggle_Leds(void);
#if !defined(USE_STM324x9I_EVAL)
//static uint8_t Check_BMP_file(uint8_t *buf);
//static uint8_t Image_Browser (char* path);
//static void     Show_Image(void);
#endif
/**
* @}
*/



/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/ 


/** @defgroup USBH_DUAL_FUNCT_DEMO_Private_Functions
* @{
*/ 

/**
* @brief  Demo_Init 
*         Demo initialization
* @param  None
* @retval None
*/

void usb_Demo_Init (void)
{
#if 0
  Demo_SelectItem (DEMO_main_menu, 0); 
  uart_printf("> Demo Initialized\n");
  uart_printf("> Use Joystick to Select demo.\n");


  USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS
            USB_OTG_FS_CORE_ID,
#elif defined USE_USB_OTG_HS
            USB_OTG_HS_CORE_ID,
#endif                
            &USB_Host,
            &USBH_MSC_cb, 
            &USR_USBH_MSC_cb);
  
  USB_OTG_BSP_mDelay(500);
 #endif
  DEMO_UNLOCK();
}

/**
* @brief  Demo_Application 
*         Demo background task
* @param  None
* @retval None
*/

void Demo_Process (void)
{
	/*只有在HOST状态才有轮询？在DEVICE模式，都是中断回调？*/
  if(demo.state == DEMO_HOST)
  {
    if(HCD_IsDeviceConnected(&USB_OTG_Core))
    {
      	USBH_Process(&USB_OTG_Core, &USB_Host); 
    }
  }
  
  Demo_Application();
  
}

/**
* @brief  Demo_SelectItem 
*         manage the menu on the screen
* @param  menu : menu table
*         item : selected item to be highlighted
	      界面，高亮选中的那行菜单
* @retval None
*/
static void Demo_SelectItem (uint8_t **menu , uint8_t item)
{
}

/**
* @brief  Demo_Application 
*         Demo state machine
* @param  None
* @retval None
*/

static void Demo_Application (void)
{
  static uint8_t prev_select = 0;
  uint16_t bytesWritten, bytesToWrite;
  FRESULT res;
  
  switch (demo.state)
  {
  case  DEMO_IDLE:
    //__disable_irq();
    //Demo_SelectItem (DEMO_main_menu, 0); 
   //__enable_irq();
    demo.state = DEMO_WAIT;
    demo.select = 0;
    break;    

  case  DEMO_WAIT:
  	//wjq_log(LOG_DEBUG, "DEMO_WAIT");
	/*
		在这里进行OTG是HOST还是DEVICE的检测
		如果都不是，就一直检测
  	*/
	if(1 == 1)
	{
		demo.state = DEMO_HOST;  
    	demo.Host_state = DEMO_HOST_IDLE;	
	}
	else
	{
		//demo.state = DEMO_DEVICE;            
    	//demo.Device_state = DEMO_DEVICE_IDLE;   
	}
    break;
	
  case  DEMO_HOST:  
    switch (demo.Host_state)
    {
    case  DEMO_HOST_IDLE:
      DEMO_LOCK();
      /* Init HS Core  : Demo start in host mode*/
#ifdef USE_USB_OTG_HS
      wjq_log(LOG_DEBUG, "> Initializing USB Host High speed...\n");    
#else
      //wjq_log(LOG_DEBUG, "> Initializing USB Host Full speed...\n");         
#endif

      USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS
                USB_OTG_FS_CORE_ID,
#elif defined USE_USB_OTG_HS
                USB_OTG_HS_CORE_ID,
#endif                
                &USB_Host,
                &USBH_MSC_cb, 
                &USR_USBH_MSC_cb);  

      demo.Host_state = DEMO_HOST_WAIT;
      DEMO_UNLOCK();      
      break;

    case  DEMO_HOST_WAIT:

	  /*
	  	HOST 模式断开
		流程会返回DEMO_IDLE
		重新开始检测模式
		*/	
      if (!HCD_IsDeviceConnected(&USB_OTG_Core))
      {
        Demo_HandleDisconnect();
        //wjq_log(LOG_DEBUG, "Please, connect a device and try again.\r\n");
      }

	  /*
	  	1 Enum_Done只会在
		USBH_USR_MSC_Application函数中赋值1
		这个函数是一个回调
		
		2 在USBH_MSC_Handle函数状态
		USBH_MSC_DEFAULT_APPLI_STATE
		的时候回调

		赋值为1的时候说明USB HOST已经枚举成功了，
		下一步就是挂载文件系统了
		如果要做HOST跟DEVICE自动切换，肯定在初始化之前
		USBH_Init
		USBD_Init
	  */

	  
      if(Enum_Done == 1)
      {
#ifdef USE_USB_OTG_HS
        wjq_log(LOG_DEBUG, ("> USB Host High speed initialized.\r\n");
#else
        wjq_log(LOG_DEBUG, "> USB Host Full speed initialized.\r\n");        
#endif        
        
        /* Initialises the File System*/
        if ( f_mount(&fatfs, "2:/", 0) != FR_OK ) 
        {
          /* efs initialisation fails*/
          wjq_log(LOG_DEBUG, "> Cannot initialize File System.\r\n");
        }
        
        wjq_log(LOG_DEBUG, "> File System initialized.\r\n");
        wjq_log(LOG_DEBUG, "> Disk capacity : %d Bytes\r\n", 
                   USBH_MSC_Param.MSCapacity * USBH_MSC_Param.MSPageLength);       
        Demo_SelectItem (DEMO_HOST_menu, 0);
        demo.select = 0; 

		demo.select = 0x80; 
		
        Enum_Done = 2;
      }
      
      if(Enum_Done == 2)
      {
        if(demo.select != prev_select)
        {
          prev_select = demo.select ;
          USB_OTG_DisableGlobalInt(&USB_OTG_Core);
          Demo_SelectItem (DEMO_HOST_menu, demo.select & 0x7F);
          USB_OTG_EnableGlobalInt(&USB_OTG_Core);
          
          /* Handle select item */
          if(demo.select & 0x80)
          {
            demo.select &= 0x7F;
            switch (demo.select)
            {
            case  0:
              DEMO_LOCK();
              Explore_Disk("2:/", 1);
              line_idx = 0;  
              DEMO_UNLOCK();
              break;
			  
            case 1:
              /* Writes a text file, STM32.TXT in the disk*/
              wjq_log(LOG_DEBUG, "> Writing File to disk flash ...\n");
              if(USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
              {
                
                wjq_log(LOG_DEBUG,  "> Disk flash is write protected \n");
                USBH_USR_ApplicationState = USH_USR_FS_DRAW;
                break;
              }
              DEMO_LOCK();
              /* Register work area for logical drives */
              f_mount(&fatfs, "2:/", 0);
              
              if(f_open(&file, "0:STM32.TXT",FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
              { 
                /* Write buffer to file */
                bytesToWrite = sizeof(writeTextBuff); 
                res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);   
                
                if((bytesWritten == 0) || (res != FR_OK)) /*EOF or Error*/
                {
                  wjq_log(LOG_DEBUG, "> STM32.TXT CANNOT be writen.\n");
                }
                else
                {
                  wjq_log(LOG_DEBUG, "> 'STM32.TXT' file created\n");
                }
                
                /*close file and filesystem*/
                f_close(&file);
              }
              DEMO_UNLOCK();
              break;
            case  2:
              if (f_mount( &fatfs, "2:/", 0) != FR_OK ) 
              {
                /* fat_fs initialisation fails*/
                break;
              }
#if !defined(USE_STM324x9I_EVAL)
              //Image_Browser("0:/");
#endif
              /*Clear windows */
              wjq_log(LOG_DEBUG, "> Slide show application closed.\n");
              break;
              
            case 3:
              Demo_HandleDisconnect();
              f_mount(NULL, "", 0); 
              USB_OTG_StopHost(&USB_OTG_Core);
              /* Manage User disconnect operations*/
              USB_Host.usr_cb->DeviceDisconnected();
              
              /* Re-Initilaize Host for new Enumeration */
              USBH_DeInit(&USB_OTG_Core, &USB_Host);
              USB_Host.usr_cb->DeInit();
              USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);
			  
              wjq_log(LOG_DEBUG, "> usb host disconnect.\r\n");
              break;
              
            default:
              break;
            }
          }
        }
      }
      break;        
    default:
      break;
    }    
    break; 
	
  case  DEMO_DEVICE:
    switch (demo.Device_state)
    {
    case  DEMO_DEVICE_IDLE:
      DEMO_LOCK();
	  
      USBD_Init(&USB_OTG_Core,
#ifdef USE_USB_OTG_FS
                USB_OTG_FS_CORE_ID,
#elif defined USE_USB_OTG_HS
                USB_OTG_HS_CORE_ID,
#endif 
                &USR_desc,
                &USBD_MSC_cb, 
                &USR_cb);
				
      demo.Device_state = DEMO_DEVICE_WAIT;  
      demo.select = 0;
      DEMO_UNLOCK();
      break;
      
    case  DEMO_DEVICE_WAIT:
      if(demo.select != prev_select)
      {
        prev_select = demo.select ;
        __disable_irq();
        Demo_SelectItem (DEMO_DEVICE_menu, demo.select & 0x7F);
        __enable_irq();
        
        /* Handle select item */
        if(demo.select & 0x80)
        {
          demo.select &= 0x7F;
          switch (demo.select)
          {
          case  0:
             __disable_irq();
            demo.state = DEMO_IDLE;
            demo.select = 0;
            wjq_log(LOG_DEBUG, "> Device application closed.\n");
            __enable_irq();
			/*断开设备*/
            DCD_DevDisconnect (&USB_OTG_Core);
            USB_OTG_StopDevice(&USB_OTG_Core);               
            break;
          default:
            break;
          }
        }
      } 
      break;      
      
    default:
      break;
    }     
    break;     
	
  default:
    break;
  }
  
}


/**
* @brief  Explore_Disk 
*         Displays disk content
* @param  path: pointer to root path
* @retval None
*/
static uint8_t Explore_Disk (char* path , uint8_t recu_level)
{
  
  FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
  char tmp[14];
  
  
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    while(HCD_IsDeviceConnected(&USB_OTG_Core)) 
    {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) 
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }
      fn = fno.fname;      
      strcpy(tmp, fn);
      line_idx++;
      if(line_idx > 12)
      {
        line_idx = 0;
        wait_user_input = 1;
        wjq_log(LOG_INFO, "Press any key to continue...");
        while((HCD_IsDeviceConnected(&USB_OTG_Core)) && (wait_user_input != 2))
        {
          Toggle_Leds();
        }
      } 
      wait_user_input = 0; 
      
      if(recu_level == 1)
      {
        wjq_log(LOG_INFO, "   |__");
      }
      else if(recu_level == 2)
      {
        wjq_log(LOG_INFO, "   |   |__");
      }
      if((fno.fattrib & AM_MASK) == AM_DIR)
      {
        strcat(tmp, "\n");
        wjq_log(LOG_INFO, (void *)tmp);
      }
      else
      {
        strcat(tmp, "\n");        
        wjq_log(LOG_INFO, (void *)tmp);
      }
      
      if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 1))
      {
        Explore_Disk(fn, 2);
      }
    }
  }
  return res;
}

/**
* @brief  Demo_HandleDisconnect
*         deinit demo and astart again the enumeration
* @param  None
* @retval None
*/
void Demo_HandleDisconnect (void)
{
  demo.state = DEMO_IDLE;
  USBH_DeInit(&USB_OTG_Core, &USB_Host);
  Enum_Done = 0;
  DEMO_UNLOCK();
}
/**
* @brief  Toggle_Leds
*         Toggle leds to shows user input state
* @param  None
* @retval None
*/
static void Toggle_Leds(void)
{

}

#if 0//!defined(USE_STM324x9I_EVAL)
/**
* @brief  Show_Image 
*         Displays BMP image
* @param  None
* @retval None
*/
static void Show_Image(void)
{

  uint16_t i = 0;
  uint16_t numOfReadBytes = 0;
  FRESULT res; 
  
  LCD_SetDisplayWindow(239, 319, 240, 320);
  LCD_WriteReg(R3, 0x1008);
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  
  
  if(Check_BMP_file(Image_Buf) > 0)
  {
    LCD_LOG_ClearTextZone();            
    LCD_LOG_SetHeader((uint8_t*)" USB Manual DRD demo");
    LCD_ErrLog("Bad BMP Format.\n");
    return;
  }
  
  while (HCD_IsDeviceConnected(&USB_OTG_Core))
  {
    res = f_read(&file, Image_Buf, IMAGE_BUFFER_SIZE, (void *)&numOfReadBytes);
    if((numOfReadBytes == 0) || (res != FR_OK)) /*EOF or Error*/
    {
      break; 
    }
    for(i = 0 ; i < IMAGE_BUFFER_SIZE; i+= 2)
    {
      LCD_WriteRAM(Image_Buf[i+1] << 8 | Image_Buf[i]); 
    } 
  }
}


/**
* @brief  Show_Image 
*         launch the Image browser
* @param  path string
* @retval status
*/
static uint8_t Image_Browser (char* path)
{
  FRESULT res;
  uint8_t ret = 1;
  FILINFO fno;
  DIR dir;
  char *fn;
  
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) 
      {
        wait_user_input = 0;
        break;
      }  
      
      if (fno.fname[0] == '.') continue;

      fn = fno.fname;

      if (fno.fattrib & AM_DIR) 
      {
        continue;
      } 
      else 
      {
        if((strstr(fn, "bmp")) || (strstr(fn, "BMP")))
        {
          
          res = f_open(&file, fn, FA_OPEN_EXISTING | FA_READ);
          Show_Image();
          wait_user_input = 1;
          LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 12, "Press any key to continue...             ");
          LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR); 
          
          while((HCD_IsDeviceConnected(&USB_OTG_Core)) && (wait_user_input != 2))
          {
            Toggle_Leds();
          }
          STM_EVAL_LEDOff(LED1);
          STM_EVAL_LEDOff(LED2);
          STM_EVAL_LEDOff(LED3);
          STM_EVAL_LEDOff(LED4);
          LCD_ClearLine( LCD_PIXEL_HEIGHT - 12);
          f_close(&file);
          wait_user_input = 0; 
          
        }
      }
    }  
  }
  return ret;
}

/**
* @brief  Check_BMP_file 
*         Displays BMP image
* @param  None
* @retval None
*/
static uint8_t Check_BMP_file(uint8_t *buf)
{
  uint16_t             numOfReadBytes = 0;
  uint16_t  Type;
  uint32_t  Width;  
  uint32_t  Height;
  uint16_t  Bpp;
  uint32_t  Compression ; 
  
  if(f_read(&file, buf, 54, (void *)&numOfReadBytes) != FR_OK) /*Error*/
  {
    return 1;
  }
  
  Type        = *(__packed uint16_t *)(buf + 0 );
  Width       = *(__packed uint32_t *)(buf + 18 );
  Height      = *(__packed uint32_t *)(buf + 22 );
  Bpp         = *(__packed uint16_t *)(buf + 28 );
  Compression = *(__packed uint32_t *)(buf + 30 );
  
  if(( Type != BF_TYPE)||
     ( Width != 320)||
       ( Height != 240)||
         ( Bpp != 16)||
           ( Compression != BI_BITFIELDS))
  {
    return 1;    
  }
  
  return 0;
}
#endif
/**
* @}
*/ 

/**
* @}
*/ 

/*

	测试USB_ID中断

*/
void USB_ID_EXTIConfig(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the INT () Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Connect EXTI Line to INT Pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource10);

  /* Configure EXTI line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set the EXTI interrupt to priority 1*/
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
}

s32 UsbGd = -2;

s32 usb_app_init(void)
{
	#ifdef SYS_USE_USB
  usb_Demo_Init();
  wjq_log(LOG_INFO, "\nSystem Information :\r\n");  
  wjq_log(LOG_INFO, "_________________________\r\n\r\n");  
  wjq_log(LOG_INFO, "Board : wujique stm32f407.\r\n");  
  wjq_log(LOG_INFO, "Device: STM32F407.\r\n");  
  wjq_log(LOG_INFO, "USB Host Library v2.2.0.\r\n");  
  wjq_log(LOG_INFO, "USB Device Library v1.2.0.\r\n"); 
  wjq_log(LOG_INFO, "USB OTG Driver v2.2.0\r\n");  
  wjq_log(LOG_INFO, "STM32 Std Library v1.5.0.\r\n");
  UsbGd = 0;
  #else
  wjq_log(LOG_INFO, ">-------USB APP NO INIT\r\n");
  #endif 
	return 0;
}
/**/
s32 usb_loop_task(void)
{
	if(UsbGd != 0)
		return -1;
	
	Demo_Process(); 
	return 0;
}

/*
	创建USB任务
*/
#include "FreeRtos.h"
#define USB_TASK_STK_SIZE 1024
#define USB_TASK_PRIO	2
TaskHandle_t  UsbTaskHandle;

void usb_main(void)
{
		   
  usb_app_init();	

  while(1)
  {
  	//wjq_log(LOG_DEBUG, "USB TASK ");
  	vTaskDelay(5);
    usb_loop_task();   
  }
}



s32 usb_task_create(void)
{
	xTaskCreate(	(TaskFunction_t) usb_main,
					(const char *)"usb task",		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
					(const configSTACK_DEPTH_TYPE) USB_TASK_STK_SIZE,
					(void *) NULL,
					(UBaseType_t) USB_TASK_PRIO,
					(TaskHandle_t *) &UsbTaskHandle );	
					
					return 0;
}

/**
* @}
*/


/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
