/**
 * @file            wujique_sysconf.c
 * @brief           系统管理
 * @author          wujique
 * @date            2018年4月9日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月9日 星期一
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#include "stm32f4xx.h"


/*

	尝试做一套设备树方案

*/
#if 0
{
	"DEV_SPI",	//spi控制器
	"SPI3"
	DevVspi1IO,

		"BUS_SPI", //spi总线，基于SPI控制器
		"SPI3_1"
		data,

		"BUS_SPI", //spi总线，基于SPI控制器
		"SPI3_2"
		data,

		"BUS_SPI", //spi总线，基于SPI控制器
		"SPI3_13"
		data,
	
	"DEV_VSPI",	//vspi控制器
	"VSPI1"
	DevVspi1IO,
	
		"BUS_SPI", //spi总线，基于SPI控制器
		"VSPI1_1"
		data,
		
	"DEV_VSPI",	//vspi控制器
	"VSPI2"
	DevVspi1IO,

		"BUS_SPI", //spi总线，基于SPI控制器
		"VSPI2_1"
		data,
			"LCD"
			"COGLCD"
			DATA

	"LCD",
	
}

/* 设备列表*/

/*驱动列表*/


/*
	根据设备列表查找驱动并初始化设备

	暂时只支持SPI I2C LCD SPI FLASH

	IO级别的不定义设备

	设备也可以不挂在设备树
*/
s32 dev_init(void)
{
	
}

s32 dev_open()
#endif

