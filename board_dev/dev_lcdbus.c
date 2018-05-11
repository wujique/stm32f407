/**
 * @file            dev_lcdbus.c
 * @brief           对各种LCD接口封装
 * @author          wujique
 * @date            2018年4月18日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月18日 星期三
 *   作    者:         wujique
 *   修改内容:   创建文件
	1 源码归屋脊雀工作室所有。
	2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
	3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
	4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
	5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
	6 使用本源码则相当于认同本版权说明。
	7 如侵犯你的权利，请联系：code@wujique.com
	8 一切解释权归屋脊雀工作室所有。
*/
#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "main.h"
#include "wujique_log.h"
#include "list.h"
#include "mcu_spi.h"
#include "mcu_i2c.h"
#include "dev_lcdbus.h"


/*
	写寄存器要两步
	*LcdReg = LCD_Reg; //写入要写的寄存器序号
	*LcdData = LCD_RegValue; //写入数据 
*/

volatile u16 *LcdReg = (u16*)0x6C000000;
volatile u16 *LcdData = (u16*)0x6C010000;

/*
	一个LCD接口
	除了通信的接口
	还有其他不属于通信接口的信号
*/

/*LCD 总线设备节点根节点*/
struct list_head DevBusLcdRoot = {&DevBusLcdRoot, &DevBusLcdRoot};	

static void bus_lcd_IO_init(DevLcdBus *dev) 
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	if(dev->type == LCD_BUS_I2C)
		return;

	/* 初始化管脚 */
	mcu_io_config_out(dev->A0port,dev->A0pin);
	mcu_io_output_setbit(dev->A0port,dev->A0pin);

	mcu_io_config_out(dev->rstport,dev->rstpin);
	mcu_io_output_setbit(dev->rstport,dev->rstpin);
	
	mcu_io_config_out(dev->blport,dev->blpin);
	mcu_io_output_setbit(dev->blport,dev->blpin);

}

s32 bus_lcd_bl(DevLcdBusNode *node, u8 sta)
{
	if(sta ==1)
	{
		mcu_io_output_setbit(node->dev.blport, node->dev.blpin);
	}
	else
	{
		mcu_io_output_resetbit(node->dev.blport, node->dev.blpin);	
	}
	return 0;
}

s32 bus_lcd_rst(DevLcdBusNode *node, u8 sta)
{
	if(sta ==1)
	{
		mcu_io_output_setbit(node->dev.rstport, node->dev.rstpin);
	}
	else
	{
		mcu_io_output_resetbit(node->dev.rstport, node->dev.rstpin);	
	}
	return 0;
}

static s32 bus_lcd_a0(DevLcdBusNode *node, u8 sta)
{
	if(node->dev.type == LCD_BUS_8080)
		return 0;
	
	if(sta ==1)
	{
		mcu_io_output_setbit(node->dev.A0port, node->dev.A0pin);
	}
	else
	{
		mcu_io_output_resetbit(node->dev.A0port, node->dev.A0pin);	
	}
	return 0;
}


DevLcdBusNode *bus_lcd_open(char *name)
{
	/*找设备*/
	DevLcdBusNode *node;
	struct list_head *listp;

	//wjq_log(LOG_INFO, "lcd bus name:%s!\r\n", name);
	
	listp = DevBusLcdRoot.next;
	node = NULL;
	
	while(1)
	{
		if(listp == &DevBusLcdRoot)
			break;

		node = list_entry(listp, DevLcdBusNode, list);
		//wjq_log(LOG_INFO, "lcd bus name:%s!\r\n", node->dev.name);
		
		if(strcmp(name, node->dev.name) == 0)
		{
			break;
		}
		else
		{
			node = NULL;
		}
		
		listp = listp->next;
	}

	if(node != NULL)
	{
		if(node->gd == 0)
		{
			wjq_log(LOG_INFO, "lcd bus open err:using!\r\n");
			node = NULL;
		}
		else
		{
			
			if(node->dev.type == LCD_BUS_SPI)
			{
				node->basenode = (void *)mcu_spi_open(node->dev.basebus, SPI_MODE_3, SPI_BaudRatePrescaler_4);

			}
			else if(node->dev.type == LCD_BUS_I2C)
			{
				node->basenode = mcu_i2c_open(node->dev.basebus);
			}
			else if(node->dev.type == LCD_BUS_8080)
			{
				/*8080特殊处理*/
				node->basenode = (void *)1;
			}
			
			if(node->basenode == NULL)
			{
				wjq_log(LOG_INFO, "lcd bus open base bus err!\r\n");	
				node =  NULL;
			}
			else
			{
				node->gd = 0;

			}
		}
	}
	else
	{
		wjq_log(LOG_INFO, "lcd bus open err:%s!\r\n", name);
	}

	return node;
}

s32 bus_lcd_close(DevLcdBusNode *node)
{
	if(node->gd != 0)
		return -1;
	
	if(node->dev.type == LCD_BUS_SPI)
	{
		mcu_spi_close((DevSpiChNode *)node->basenode);
		
	}
	else if(node->dev.type == LCD_BUS_I2C)
	{
		mcu_i2c_close((DevI2cNode *)node->basenode);	
	}
	else if(node->dev.type == LCD_BUS_8080)
	{
		/*8080特殊处理*/
		node->basenode = NULL;
	}
	
	node->gd = -1;
	
	return 0;
}

s32 bus_lcd_write_data(DevLcdBusNode *node, u8 *data, u16 len)
{
	/*可能有BUF，要根据len动态申请*/
	u8 tmp[256];
	u16 i;
	
	if(node->dev.type == LCD_BUS_SPI)
	{
		bus_lcd_a0(node, 1);	
		mcu_spi_cs((DevSpiChNode *)node->basenode, 0);
		mcu_spi_transfer((DevSpiChNode *)node->basenode,  data, NULL, len);
		mcu_spi_cs((DevSpiChNode *)node->basenode, 1);
		
	}
	else if(node->dev.type == LCD_BUS_I2C)
	{
		
		tmp[0] = 0x40;
		memcpy(&tmp[1], data, len);
		mcu_i2c_transfer((DevI2cNode *)node->basenode, 0x3C, MCU_I2C_MODE_W, tmp, len+1);
		

	}
	else if(node->dev.type == LCD_BUS_8080)
	{
		u16 *p;
		p = (u16 *)data;
		for(i=0; i<len; i++)
		{
			*LcdData = *(p+i);	
		}
	}
	return 0;
}

s32 bus_lcd_write_cmd(DevLcdBusNode *node, u8 cmd)
{
	u8 tmp[2];

	if(node->dev.type == LCD_BUS_SPI)
	{	
		bus_lcd_a0(node, 0);
		tmp[0] = cmd;
		mcu_spi_cs((DevSpiChNode *)node->basenode, 0);
		mcu_spi_transfer((DevSpiChNode *)node->basenode,  &tmp[0], NULL, 1);
		mcu_spi_cs((DevSpiChNode *)node->basenode, 1);
	}
	else if(node->dev.type == LCD_BUS_I2C)
	{	
		tmp[0] = 0x00;
		tmp[1] = cmd;
		
		mcu_i2c_transfer((DevI2cNode *)node->basenode, 0x3C, MCU_I2C_MODE_W, tmp, 2);
	}
	else if(node->dev.type == LCD_BUS_8080)
	{
		*LcdReg = cmd;	
	}
	return 0;
}


/**
 *@brief:      dev_lcdbus_init
 *@details:    初始化所有LCD总线
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_lcdbus_register(DevLcdBus *dev)
{
	struct list_head *listp;
	DevLcdBusNode *p;

	wjq_log(LOG_INFO, "[register] lcd bus :%s, base on:%s!\r\n", dev->name, dev->basebus);

	/*
		先要查询当前，防止重名
	*/
	listp = DevBusLcdRoot.next;
	while(1)
	{
		if(listp == &DevBusLcdRoot)
			break;

		p = list_entry(listp, DevLcdBusNode, list);

		if(strcmp(dev->name, p->dev.name) == 0)
		{
			wjq_log(LOG_INFO, "bus lcd dev name err!\r\n");
			return -1;
		}
		
		listp = listp->next;
	}

	/* 
		申请一个节点空间 
		
	*/
	p = (DevLcdBusNode *)wjq_malloc(sizeof(DevLcdBusNode));
	list_add(&(p->list), &DevBusLcdRoot);
	/*复制设备信息*/
	memcpy((u8 *)&p->dev, (u8 *)dev, sizeof(DevLcdBus));
	p->gd = -1;

	/*初始化*/
	bus_lcd_IO_init(dev);

	if(dev->type == LCD_BUS_8080)
	{
		//初始FSMC
		mcu_fsmc_lcd_Init();
	}
	return 0;
}





