/**
 * @file            dev_ILI9341.c
 * @brief           TFT LCD 驱动芯片ST7789驱动程序
 * @author          wujique
 * @date            2017年11月8日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月8日 星期三
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

/* 
	1.3寸，IPS，中景园，只有SCL&SDA的SPI接口LCD
*/	

#include "stdlib.h"
#include "string.h"

#include "stm32f4xx.h"

#include "wujique_sysconf.h"

#include "stm324xg_eval_fsmc_sram.h"
#include "alloc.h"
#include "wujique_log.h"
#include "dev_lcdbus.h"
#include "dev_lcd.h"

extern void Delay(__IO uint32_t nTime);


#ifdef TFT_LCD_DRIVER_7789
#define ST7789_CMD_WRAM 0x2c
#define ST7789_CMD_SETX 0x2a
#define ST7789_CMD_SETY 0x2b

s32 drv_ST7789_init(DevLcdNode *lcd);
static s32 drv_ST7789_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ST7789_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_ST7789_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_ST7789_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_ST7789_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_ST7789_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_ST7789_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_ST7789_flush(DevLcdNode *lcd, u16 *color, u32 len);

_lcd_drv TftLcdST7789_Drv = {
							.id = 0X7789,

							.init = drv_ST7789_init,
							.draw_point = drv_ST7789_drawpoint,
							.color_fill = drv_ST7789_color_fill,
							.fill = drv_ST7789_fill,
							.onoff = drv_ST7789_display_onoff,
							.prepare_display = drv_ST7789_prepare_display,
							.flush = drv_ST7789_flush,
							.set_dir = drv_ST7789_scan_dir,
							.backlight = drv_ST7789_lcd_bl
							};

void drv_ST7789_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}		
/**
 *@brief:      drv_ST7789_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ST7789_scan_dir(DevLcdNode *lcd, u8 dir)
{
	u16 regval=0;
#if 0
	/*设置从左边到右边还是右边到左边*/
	switch(dir)
	{
		case R2L_U2D:
		case R2L_D2U:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<6); 
			break;	 
	}

	/*设置从上到下还是从下到上*/
	switch(dir)
	{
		case L2R_D2U:
		case R2L_D2U:
		case D2U_L2R:
		case D2U_R2L:
			regval|=(1<<7); 
			break;	 
	}

	/*
		设置先左右还是先上下 Reverse Mode
		如果设置为1，LCD控制器已经将行跟列对调了，
		因此需要在显示中进行调整
	*/
	switch(dir)
	{
		case U2D_L2R:
		case D2U_L2R:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<5);
			break;	 
	}
	/*
		还可以设置RGB还是GBR
		还可以设置调转上下
	*/	
	regval|=(1<<3);//0:GBR,1:RGB  跟R61408相反

	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);
	
	bus_lcd_write_cmd(node, (0x36));
	u16 tmp[2];
	tmp[0] = regval;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	bus_lcd_close(node);

#endif

}

/**
 *@brief:      drv_ST7789_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7789_set_cp_addr(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{

	DevLcdBusNode * node;
	u8 tmp[4];

	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_write_cmd(node, ST7789_CMD_SETX);
	tmp[0] = (0x00);
	tmp[1] = (sc);
	tmp[2] = (0X00);
	tmp[3] = (ec);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ST7789_CMD_SETY));
	tmp[0] = (0);
	tmp[1] = (sp);
	tmp[2] = (0);
	tmp[3] = (ep);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ST7789_CMD_WRAM));
	
	bus_lcd_close(node);
	
	return 0;
}

/**
 *@brief:      drv_ST7789_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7789_display_onoff(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);
	
	if(sta == 1)
		bus_lcd_write_cmd(node, (0x29));
	else
		bus_lcd_write_cmd(node, (0x28));

	bus_lcd_close(node);
	
	return 0;
}


/**
 *@brief:      drv_ST7789_init
 *@details:    初始化FSMC，并且读取ILI9341的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7789_init(DevLcdNode *lcd)
{
	u16 data;
	DevLcdBusNode * node;
	u8 tmp[16];
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(100);
	bus_lcd_rst(node, 1);
	Delay(50);

	bus_lcd_write_cmd(node, (0x36));
	tmp[0] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0x3A));
	tmp[0] = 0x05;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xB2));
	tmp[0] = 0x0C;
	tmp[1] = 0x0C;
	tmp[2] = 0x00;
	tmp[3] = 0x33;
	tmp[4] = 0x33;
	bus_lcd_write_data(node, (u8*)tmp, 5);
	
	bus_lcd_write_cmd(node, (0xB7));
	tmp[0] = 0x35;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xBB));
	tmp[0] = 0x19;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xC0));
	tmp[0] = 0x2C;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xC2));
	tmp[0] = 0x01;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	 
	bus_lcd_write_cmd(node, (0xC3));
	tmp[0] = 0x12;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xC4));
	tmp[0] = 0x20;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	  
	bus_lcd_write_cmd(node, (0xC6));
	tmp[0] = 0x0F;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xD0));
	tmp[0] = 0xA4;
	tmp[1] = 0xA1;
	bus_lcd_write_data(node, (u8*)tmp, 2);
	
	bus_lcd_write_cmd(node, (0xE0));
	tmp[0] = 0xD0;
	tmp[1] = 0x04;
	tmp[2] = 0x0D;
	tmp[3] = 0x11;

	tmp[4] = 0x13;
	tmp[5] = 0x2B;
	tmp[6] = 0x3F;
	tmp[7] = 0x54;

	tmp[8] = 0x4C;
	tmp[9] = 0x18;
	tmp[10] = 0x0D;
	tmp[11] = 0x0B;

	tmp[12] = 0x1F;
	tmp[13] = 0x23;
	
	bus_lcd_write_data(node, (u8*)tmp, 14);	
	
	bus_lcd_write_cmd(node, (0xE1));
	tmp[0] = 0xD0;
	tmp[1] = 0x04;
	tmp[2] = 0x0C;
	tmp[3] = 0x11;

	tmp[4] = 0x13;
	tmp[5] = 0x2C;
	tmp[6] = 0x3F;
	tmp[7] = 0x44;

	tmp[8] = 0x51;
	tmp[9] = 0x2F;
	tmp[10] = 0x1F;
	tmp[11] = 0x1F;

	tmp[12] = 0x20;
	tmp[13] = 0x23;
	bus_lcd_write_data(node, (u8*)tmp, 14);	
	bus_lcd_write_cmd(node, (0x21)); 
	
	bus_lcd_write_cmd(node, (0x11)); 
	Delay (50); 
	//Display on	 
	bus_lcd_write_cmd(node, (0x29)); 

	bus_lcd_close(node);
	
	Delay(50);
	
	return 0;
}
/**
 *@brief:      drv_ST7789_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7789_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{
	/*
		显示XY轴范围
	*/
	if(sx >= lcd->width)
		sx = lcd->width-1;
	
	if(ex >= lcd->width)
		ex = lcd->width-1;
	
	if(sy >= lcd->height)
		sy = lcd->height-1;
	
	if(ey >= lcd->height)
		ey = lcd->height-1;
	/*
		XY轴，实物角度来看，方向取决于横屏还是竖屏
		CP轴，是控制器显存角度，
		XY轴的映射关系取决于扫描方向
	*/
	if(
		(((lcd->scandir&LRUD_BIT_MASK) == LRUD_BIT_MASK)
		&&(lcd->dir == H_LCD))
		||
		(((lcd->scandir&LRUD_BIT_MASK) == 0)
		&&(lcd->dir == W_LCD))
		)
		{
			*sc = sy;
			*ec = ey;
			*sp = sx;
			*ep = ex;
		}
	else
	{
		*sc = sx;
		*ec = ex;
		*sp = sy;
		*ep = ey;
	}
	
	return 0;
}

/**
 *@brief:      drv_ST7789_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7789_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ST7789_xy2cp(lcd, x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ST7789_set_cp_addr(lcd, sc, ec, sp, ep);

	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);
	
	u8 tmp[2];
	tmp[0] = color>>8;
	tmp[1] = color&0xff;
	bus_lcd_write_data(node, (u8*)tmp, 2);
	bus_lcd_close(node);
 
	return 0;
}
/**
 *@brief:      drv_ST7789_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7789_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i;
	u16 sc,ec,sp,ep;

	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", sx, ex, sy, ey);

	drv_ST7789_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", sc, ec, sp, ep);
	
	drv_ST7789_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;//得到填充的宽度 +1是因为坐标从0开始
	height=(ep+1)-sp;//高度
	
	wjq_log(LOG_DEBUG, "ST7789 width:%d, height:%d\r\n", width, height);

	u8 *tmp;

	tmp = (u8 *)wjq_malloc(width*2);

	for(i = 0; i < width*2;)
	{
		tmp[i++] = color>>8;
		tmp[i++] = color&0xff;
	}
	
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);

	for(i = 0; i < height;i++)
	{
			bus_lcd_write_data(lcd->busnode, tmp, width*2);
	}
	
	bus_lcd_close(lcd->busnode);

	wjq_free(tmp);
	
	return 0;

}

/**
 *@brief:      drv_ST7789_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7789_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;
	u16 *pcc;
	
	drv_ST7789_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ST7789_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;
	height=(ep+1)-sp;

	wjq_log(LOG_DEBUG, "fill width:%d, height:%d\r\n", width, height);
	
	u8 *tmp;

	tmp = (u8 *)wjq_malloc(width*2);

	
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);
	
	pcc = color;
	
	for(i = 0; i < height;i++)
	{
		for(j = 0; j < width*2;)
		{
			tmp[j++] = (*pcc) >> 8;
			tmp[j++] = (*pcc) & 0xff;
			pcc++;
		}
		bus_lcd_write_data(lcd->busnode, tmp, width*2);
	}
	
	bus_lcd_close(lcd->busnode);

	wjq_free(tmp);	 
	return 0;
} 

s32 drv_ST7789_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;
	wjq_log(LOG_DEBUG, "XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);
	drv_ST7789_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "cp:-%d-%d-%d-%d-\r\n", sc, ec, sp, ep);
	drv_ST7789_set_cp_addr(lcd, sc, ec, sp, ep);	
	return 0;
}

s32 drv_ST7789_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	u8 *tmp;
	u32 i;
	
	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);

	tmp = (u8 *)wjq_malloc(len*2);
	i = 0;
	while(1)
	{
		tmp[i*2] = (*(color+i))>>8;
		tmp[i*2+1] = (*(color+i))&0xff;
		i++;
		if(i>=len)
			break;
	}

	bus_lcd_flush_data(lcd->busnode, tmp,  len*2);	
	bus_lcd_close(node);
	
	wjq_free(tmp);
	
	return 0;
} 

#endif

