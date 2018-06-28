/**
 * @file            dev_ILI9341.c
 * @brief           TFT LCD 驱动芯片ILI6341驱动程序
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
#include "stdlib.h"
#include "string.h"

#include "stm32f4xx.h"

#include "wujique_sysconf.h"

#include "stm324xg_eval_fsmc_sram.h"
#include "alloc.h"
#include "wujique_log.h"
#include "dev_lcdbus.h"
#include "dev_lcd.h"
#include "dev_ILI9341.h"

#define DEV_ILI9341_DEBUG

#ifdef DEV_ILI9341_DEBUG
#define ILI9341_DEBUG	wjq_log 
#else
#define ILI9341_DEBUG(a, ...)
#endif

extern void Delay(__IO uint32_t nTime);

/*

	本文件名字虽然叫9341，但是内容暂时存放所有并口总线LCD驱动

*/

/* ----------下面为不同LCD 驱动--------*/

/*

	9341驱动

*/
#ifdef TFT_LCD_DRIVER_9341
/*9341命令定义*/
#define ILI9341_CMD_WRAM 0x2c
#define ILI9341_CMD_SETX 0x2a
#define ILI9341_CMD_SETY 0x2b

s32 drv_ILI9341_init(DevLcdNode *lcd);
static s32 drv_ILI9341_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ILI9341_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_ILI9341_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_ILI9341_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_ILI9341_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_ILI9341_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_ILI9341_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_ILI9341_flush(DevLcdNode *lcd, u16 *color, u32 len);


/*

	定义一个TFT LCD，使用ILI9341驱动IC的设备

*/
_lcd_drv TftLcdILI9341Drv = {
							.id = 0X9341,

							.init = drv_ILI9341_init,
							.draw_point = drv_ILI9341_drawpoint,
							.color_fill = drv_ILI9341_color_fill,
							.fill = drv_ILI9341_fill,
							.onoff = drv_ILI9341_display_onoff,
							.prepare_display = drv_ILI9341_prepare_display,
							.flush = drv_ILI9341_flush,
							.set_dir = drv_ILI9341_scan_dir,
							.backlight = drv_ILI9341_lcd_bl
							};

void drv_ILI9341_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}
	
/**
 *@brief:      drv_ILI9341_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ILI9341_scan_dir(DevLcdNode *lcd, u8 dir)
{
	u16 regval=0;

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

}

/**
 *@brief:      drv_ILI9341_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_set_cp_addr(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{

	DevLcdBusNode * node;
	u16 tmp[4];

	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_write_cmd(node, ILI9341_CMD_SETX);
	tmp[0] = (sc>>8);
	tmp[1] = (sc&0XFF);
	tmp[2] = (ec>>8);
	tmp[3] = (ec&0XFF);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ILI9341_CMD_SETY));
	tmp[0] = (sp>>8);
	tmp[1] = (sp&0XFF);
	tmp[2] = (ep>>8);
	tmp[3] = (ep&0XFF);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ILI9341_CMD_WRAM));
	
	bus_lcd_close(node);
	
	return 0;
}

/**
 *@brief:      drv_ILI9341_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9341_display_onoff(DevLcdNode *lcd, u8 sta)
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
 *@brief:      drv_ILI9341_init
 *@details:    初始化FSMC，并且读取ILI9341的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_init(DevLcdNode *lcd)
{
	u16 data;
	DevLcdBusNode * node;
	u16 tmp[16];
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(50);
	bus_lcd_rst(node, 1);
	Delay(50);

	bus_lcd_write_cmd(node, (0x00d3));
	/*读4个字节，第一个字节是dummy read， 第二字节是0x00， 第三字节是93，第四字节是41*/
	bus_lcd_read_data(node, (u8*)tmp, 4);
	
	data = tmp[2]; 
	data<<=8;
	data |= tmp[3];

	ILI9341_DEBUG(LOG_DEBUG, "read reg:%04x\r\n", data);

	if(data != TftLcdILI9341Drv.id)
	{
		ILI9341_DEBUG(LOG_DEBUG, "lcd drive no 9341\r\n");	
		bus_lcd_close(node);
		return -1;
	}

	bus_lcd_write_cmd(node, (0xCF));//Power control B
	tmp[0] = 0x00;
	tmp[1] = 0xC1;
	tmp[2] = 0x30;
	bus_lcd_write_data(node, (u8*)tmp, 3);
	

	bus_lcd_write_cmd(node, (0xED));//Power on sequence control 
	tmp[0] = 0x64;
	tmp[1] = 0x03;
	tmp[2] = 0x12;
	tmp[3] = 0x81;
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (0xE8));//Driver timing control A
	tmp[0] = 0x85;
	//tmp[1] = 0x01;
	tmp[1] = 0x10;
	tmp[2] = 0x7A;
	bus_lcd_write_data(node, (u8*)tmp, 3);

	bus_lcd_write_cmd(node, (0xCB));//Power control 
	tmp[0] = 0x39;
	tmp[1] = 0x2C;
	tmp[2] = 0x00;
	tmp[3] = 0x34;
	tmp[4] = 0x02;
	bus_lcd_write_data(node, (u8*)tmp, 5);

	bus_lcd_write_cmd(node, (0xF7));//Pump ratio control
	tmp[0] = 0x20;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xEA));//Driver timing control
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xC0));
	tmp[0] = 0x1B;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xC1));
	tmp[0] = 0x01;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xC5));
	tmp[0] = 0x30;
	tmp[1] = 0x30;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xC7));
	tmp[0] = 0xB7;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x36));
	tmp[0] = 0x48;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x3A));
	tmp[0] = 0x55;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xB1));
	tmp[0] = 0x00;
	tmp[1] = 0x1A;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xB6));
	tmp[0] = 0x0A;
	tmp[1] = 0xA2;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xF2));
	tmp[0] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x26));
	tmp[0] = 0x01;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xE0));
	tmp[0] = 0x0F; tmp[1] = 0x2A; tmp[2] = 0x28; tmp[3] = 0x08;
	tmp[4] = 0x0E; tmp[5] = 0x08; tmp[6] = 0x54; tmp[7] = 0xa9;
	tmp[8] = 0x43; tmp[9] = 0x0a; tmp[10] = 0x0F; tmp[11] = 0x00;
	tmp[12] = 0x00; tmp[13] = 0x00; tmp[14] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 15);

	bus_lcd_write_cmd(node, (0XE1));
	tmp[0] = 0x00; tmp[1] = 0x15; tmp[2] = 0x17; tmp[3] = 0x07;
	tmp[4] = 0x11; tmp[5] = 0x06; tmp[6] = 0x2B; tmp[7] = 0x56;
	tmp[8] = 0x3C; tmp[9] = 0x05; tmp[10] = 0x10; tmp[11] = 0x0F;
	tmp[12] = 0x3F; tmp[13] = 0x3F; tmp[14] = 0x0F;
	bus_lcd_write_data(node, (u8*)tmp, 15);	

	bus_lcd_write_cmd(node, (0x2B));
	tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x01; tmp[3] = 0x3f;
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (0x2A));
	tmp[0] = 0x00; tmp[1] = 0x00; tmp[2] = 0x00; tmp[3] = 0xef;
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (0x11));
	Delay(120);
	bus_lcd_write_cmd(node, (0x29));

	bus_lcd_close(node);
	
	Delay(50);
	
	return 0;
}
/**
 *@brief:      drv_ILI9341_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
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
 *@brief:      drv_ILI9341_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9341_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(lcd, x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(lcd, sc, ec, sp, ep);

	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);
	
	u16 tmp[2];
	tmp[0] = color;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	bus_lcd_close(node);
 
	return 0;
}
/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	drv_ILI9341_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;//得到填充的宽度 +1是因为坐标从0开始
	height=(ep+1)-sp;//高度
	
	//uart_printf("ili9341 width:%d, height:%d\r\n", width, height);

	DevLcdBusNode * node;
	
#define TMP_BUF_SIZE 32
	u16 tmp[TMP_BUF_SIZE];
	u32 cnt;

	for(cnt = 0; cnt < TMP_BUF_SIZE; cnt ++)
	{
		tmp[cnt] = color;
	}
	
	cnt = height*width;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	while(1)
	{
		if(cnt < TMP_BUF_SIZE)
		{
			bus_lcd_write_data(node, (u8 *)tmp, cnt);
			cnt -= cnt;
		}
		else
		{
			bus_lcd_write_data(node, (u8 *)tmp, TMP_BUF_SIZE);
			cnt -= TMP_BUF_SIZE;
		}

		if(cnt <= 0)
			break;
	}
	
	bus_lcd_close(node);

	return 0;

}

/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;
	height=(ep+1)-sp;

	wjq_log(LOG_DEBUG, "fill width:%d, height:%d\r\n", width, height);
	
	DevLcdBusNode * node;

	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_write_data(node, (u8 *)color, height*width);	
	bus_lcd_close(node);	 
	
	return 0;

} 

s32 drv_ILI9341_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;
	
	wjq_log(LOG_DEBUG, "XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);
	drv_ILI9341_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "cp:-%d-%d-%d-%d-\r\n", sc, ec, sp, ep);
	drv_ILI9341_set_cp_addr(lcd, sc, ec, sp, ep);	
	return 0;
}

s32 drv_ILI9341_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_flush_data(lcd->busnode, (u8 *)color,  len);	
	bus_lcd_close(lcd->busnode);
	return 0;
} 
#endif

#ifdef TFT_LCD_DRIVER_9341_8BIT

/*

	定义一个TFT LCD，使用ILI9341驱动IC的设备
	但是这个驱动使用的是SPI或者I2C，8位接口，
	相对8080 16位接口，需要特殊处理

*/

s32 drv_ILI9341_8_init(DevLcdNode *lcd);
static s32 drv_ILI9341_8_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ILI9341_8_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_ILI9341_8_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);

s32 drv_ILI9341_8_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_ILI9341_8_scan_dir(DevLcdNode *lcd, u8 dir);
s32 drv_ILI9341_8_flush(DevLcdNode *lcd, u16 *color, u32 len);

_lcd_drv TftLcdILI9341_8_Drv = {
							.id = 0X9342,

							.init = drv_ILI9341_8_init,
							.draw_point = drv_ILI9341_8_drawpoint,
							.color_fill = drv_ILI9341_8_color_fill,
							.fill = drv_ILI9341_8_fill,
							.onoff = drv_ILI9341_display_onoff,
							.prepare_display = drv_ILI9341_8_prepare_display,
							.flush = drv_ILI9341_8_flush,
							.set_dir = drv_ILI9341_8_scan_dir,
							.backlight = drv_ILI9341_lcd_bl
							};

	
/**
 *@brief:      drv_ILI9341_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ILI9341_8_scan_dir(DevLcdNode *lcd, u8 dir)
{
	u16 regval=0;

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

}

/**
 *@brief:      drv_ILI9341_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_8_set_cp_addr(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{

	DevLcdBusNode * node;
	u8 tmp[4];

	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_write_cmd(node, ILI9341_CMD_SETX);
	tmp[0] = (sc>>8);
	tmp[1] = (sc&0XFF);
	tmp[2] = (ec>>8);
	tmp[3] = (ec&0XFF);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ILI9341_CMD_SETY));
	tmp[0] = (sp>>8);
	tmp[1] = (sp&0XFF);
	tmp[2] = (ep>>8);
	tmp[3] = (ep&0XFF);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ILI9341_CMD_WRAM));
	
	bus_lcd_close(node);
	
	return 0;
}



/**
 *@brief:      drv_ILI9341_init
 *@details:    初始化FSMC，并且读取ILI9341的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_8_init(DevLcdNode *lcd)
{
	u16 data;
	DevLcdBusNode * node;
	u8 tmp[16];
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(50);
	bus_lcd_rst(node, 1);
	Delay(50);

	bus_lcd_write_cmd(node, (0xd3));
	/*读4个字节，第一个字节是dummy read， 第二字节是0x00， 第三字节是93，第四字节是41*/
	bus_lcd_read_data(node, (u8*)tmp, 4);

	ILI9341_DEBUG(LOG_DEBUG, "read reg:%02x, %02x,%02x,%02x,\r\n", tmp[0], tmp[1], tmp[2], tmp[3]);
	
	data = tmp[2]; 
	data<<=8;
	data |= tmp[3];

	ILI9341_DEBUG(LOG_DEBUG, "read reg:%04x\r\n", data);

	if(data != TftLcdILI9341Drv.id)
	{
		ILI9341_DEBUG(LOG_DEBUG, "lcd drive no 9341\r\n");	
		//bus_lcd_close(node);
		//return -1;
	}

	bus_lcd_write_cmd(node, (0xCF));//Power control B
	tmp[0] = 0x00;
	tmp[1] = 0xC1;
	tmp[2] = 0x30;
	bus_lcd_write_data(node, (u8*)tmp, 3);
	

	bus_lcd_write_cmd(node, (0xED));//Power on sequence control 
	tmp[0] = 0x64;
	tmp[1] = 0x03;
	tmp[2] = 0x12;
	tmp[3] = 0x81;
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (0xE8));//Driver timing control A
	tmp[0] = 0x85;
	tmp[1] = 0x01;
	tmp[2] = 0x7A;
	bus_lcd_write_data(node, (u8*)tmp, 3);

	bus_lcd_write_cmd(node, (0xCB));//Power control 
	tmp[0] = 0x39;
	tmp[1] = 0x2C;
	tmp[2] = 0x00;
	tmp[3] = 0x34;
	tmp[4] = 0x02;
	bus_lcd_write_data(node, (u8*)tmp, 5);

	bus_lcd_write_cmd(node, (0xF7));//Pump ratio control
	tmp[0] = 0x20;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xEA));//Driver timing control
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xC0));
	tmp[0] = 0x21;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xC1));
	tmp[0] = 0x11;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xC5));
	tmp[0] = 0x31;
	tmp[1] = 0x3C;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xC7));
	tmp[0] = 0x9f;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x36));
	tmp[0] = 0x08;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x3A));
	tmp[0] = 0x55;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xB1));
	tmp[0] = 0x00;
	tmp[1] = 0x1B;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xB6));
	tmp[0] = 0x0A;
	tmp[1] = 0xA2;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xF2));
	tmp[0] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x26));
	tmp[0] = 0x01;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xE0));
	tmp[0] = 0x0F; tmp[1] = 0x20; tmp[2] = 0x1d; tmp[3] = 0x0b;
	tmp[4] = 0x10; tmp[5] = 0x0a; tmp[6] = 0x49; tmp[7] = 0xa9;
	tmp[8] = 0x3b; tmp[9] = 0x0a; tmp[10] = 0x15; tmp[11] = 0x06;
	tmp[12] = 0x0c; tmp[13] = 0x06; tmp[14] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 15);

	bus_lcd_write_cmd(node, (0XE1));
	tmp[0] = 0x00; tmp[1] = 0x1f; tmp[2] = 0x22; tmp[3] = 0x04;
	tmp[4] = 0x0f; tmp[5] = 0x05; tmp[6] = 0x36; tmp[7] = 0x46;
	tmp[8] = 0x46; tmp[9] = 0x05; tmp[10] = 0x0b; tmp[11] = 0x09;
	tmp[12] = 0x33; tmp[13] = 0x39; tmp[14] = 0x0F;
	bus_lcd_write_data(node, (u8*)tmp, 15);	
	
	bus_lcd_write_cmd(node, (0x11));

	bus_lcd_close(node);
	
	Delay(50);
	
	return 0;
}

/**
 *@brief:      drv_ILI9341_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9341_8_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(lcd, x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ILI9341_8_set_cp_addr(lcd, sc, ec, sp, ep);

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
 *@brief:      drv_ILI9341_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_8_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i;
	u16 sc,ec,sp,ep;

	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", sx, ex, sy, ey);

	drv_ILI9341_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", sc, ec, sp, ep);
	
	drv_ILI9341_8_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;//得到填充的宽度 +1是因为坐标从0开始
	height=(ep+1)-sp;//高度
	
	wjq_log(LOG_DEBUG, "ili9341 width:%d, height:%d\r\n", width, height);

	u8 *tmp;

	tmp = (u8 *)wjq_malloc(width*2);

	for(i = 0; i < width*2;)
	{
		tmp[i++] = color>>8;
		tmp[i++] = color&0xff;
	}
	
	GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);

	for(i = 0; i < height;i++)
	{
			bus_lcd_write_data(lcd->busnode, tmp, width*2);
	}
	
	bus_lcd_close(lcd->busnode);

	wjq_free(tmp);
	
	GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);

	return 0;

}

/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_8_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;
	u16 *pcc;
	
	drv_ILI9341_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ILI9341_8_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;
	height=(ep+1)-sp;

	wjq_log(LOG_DEBUG, "fill width:%d, height:%d\r\n", width, height);
	
	u8 *tmp;

	tmp = (u8 *)wjq_malloc(width*2);

	GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
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

s32 drv_ILI9341_8_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;
	wjq_log(LOG_DEBUG, "XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);
	drv_ILI9341_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "cp:-%d-%d-%d-%d-\r\n", sc, ec, sp, ep);
	drv_ILI9341_8_set_cp_addr(lcd, sc, ec, sp, ep);	
	return 0;
}

s32 drv_ILI9341_8_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	u8 *tmp;
	u32 i;
	
	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);

	/*显示320*240图片，本处数据转换大概需要10ms时间*/
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

//----------------------------------------------------------------------
/*

	9325

*/
#ifdef TFT_LCD_DRIVER_9325

/*
	9325特点：
	9325不使用page跟colum，使用Horizontal和Vertical，对于我们的模组，H是240短边，V是320长边
	1 设置扫描起始地址,  也就是 H, V地址， HV地址组成参数AD
	  AD低八位对应H，高八位对应V
	  
	2 扫描窗口地址，
	  9325叫做HSA/HEA/VSA/VEA
	  窗口H要大于等于4
	  “00”h ≤ HSA[7:0]< HEA[7:0] ≤ “EF”h. and “04”h≦HEA-HAS
	  “000”h ≤ VSA[8:0]< VEA[8:0] ≤ “13F”h.

	3 在设置扫描方向，先上下还是先左右时，H跟V不变。

	4 无论扫描方向怎么变，原点都是竖屏的左上角。其他屏幕是扫描方向的起点。
	例如，设置为横屏，左到右，上到下
	dev_tftlcd_setdir( W_LCD, L2R_U2D);
	然后刷新一个数据块
	TftLcd->color_fill(0, TftLcd->width/4, 0 , TftLcd->height/4, RED);
	其他屏幕刷新的是横屏的左上角，
	9325刷的是左下下角，
	而且是先刷最底下一行，也就是第0行，方向是从左到右。
	然后再刷最上一行，也是从左到右。
	因此需要将扫描起点设为XY轴的起点
	
	
*/
#define ILI9325_CMD_WRAM 0x22
#define ILI9325_CMD_SETV 0x21
#define ILI9325_CMD_SETH 0x20


s32 drv_ILI9325_init(DevLcdNode *lcd);
static s32 drv_ILI9325_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ILI9325_color_fill(DevLcdNode *lcd, u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
s32 drv_ILI9325_fill(DevLcdNode *lcd, u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);
static s32 drv_ILI9325_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_ILI9325_prepare_display(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep);
static void drv_ILI9325_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_ILI9325_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_ILI9325_flush(DevLcdNode *lcd, u16 *color, u32 len);

/*

	9325驱动

*/
_lcd_drv TftLcdILI9325Drv = {
							.id = 0X9325,

							.init = drv_ILI9325_init,
							.draw_point = drv_ILI9325_drawpoint,
							.color_fill = drv_ILI9325_color_fill,
							.fill = drv_ILI9325_fill,
							.prepare_display = drv_ILI9325_prepare_display,
							.flush = drv_ILI9325_flush,
							
							.onoff = drv_ILI9325_display_onoff,
							.set_dir = drv_ILI9325_scan_dir,
							.backlight = drv_ILI9325_lcd_bl
							};
void drv_ILI9325_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}	
/**
 *@brief:	   drv_ILI9325_scan_dir
 *@details:    设置显存扫描方向
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:	   static OK
 */
static void drv_ILI9325_scan_dir(DevLcdNode *lcd, u8 dir)
{
	u16 regval=0;
	u8 dirreg=0;
	
	switch(dir)
    {
        case L2R_U2D://从左到右,从上到下
            regval|=(1<<5)|(1<<4)|(0<<3); 
            break;
        case L2R_D2U://从左到右,从下到上
            regval|=(0<<5)|(1<<4)|(0<<3); 
            break;
        case R2L_U2D://从右到左,从上到下
            regval|=(1<<5)|(0<<4)|(0<<3);
            break;
        case R2L_D2U://从右到左,从下到上
            regval|=(0<<5)|(0<<4)|(0<<3); 
            break;   
        case U2D_L2R://从上到下,从左到右
            regval|=(1<<5)|(1<<4)|(1<<3); 
            break;
        case U2D_R2L://从上到下,从右到左
            regval|=(1<<5)|(0<<4)|(1<<3); 
            break;
        case D2U_L2R://从下到上,从左到右
            regval|=(0<<5)|(1<<4)|(1<<3); 
            break;
        case D2U_R2L://从下到上,从右到左
            regval|=(0<<5)|(0<<4)|(1<<3); 
            break;   
    }
	
    dirreg=0X03;
    regval|=1<<12;  

	u16 tmp[16];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	
	bus_lcd_write_cmd(node, (dirreg));
	tmp[0] = regval;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_close(node);

}

/**
 *@brief:      drv_ILI9325_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     9325设置扫描区域有一个限制，那就是窗口宽度不能小于四
 */
static s32 drv_ILI9325_set_cp_addr(DevLcdNode *lcd, u16 hsa, u16 hea, u16 vsa, u16 vea)
{
	u16 heatmp;
	u16 tmp[2];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	/* 设置扫描窗口 */
	if((hsa+4) > hea)
		heatmp = hsa+4;
	else
		heatmp = hea;
	
	bus_lcd_write_cmd(node, (0x0050));//HSA
	tmp[0] = hsa;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0051));//HEA
	tmp[0] = heatmp;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0x0052));//VSA
	tmp[0] = vsa;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0053));//VEA
	tmp[0] = vea;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	/*
		设置扫描起始地址。
	*/
	if((lcd->scandir&LR_BIT_MASK) == LR_BIT_MASK)
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETH));
		tmp[0] = hea&0XFF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	
	}
	else
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETH));
		tmp[0] = hsa&0XFF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
			  
	}

	if((lcd->scandir&UD_BIT_MASK) == UD_BIT_MASK)
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETV));
		tmp[0] = vea&0X1FF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
		
	}
	else
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETV));
		tmp[0] = vsa&0X1FF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	}
	bus_lcd_write_cmd(node, (ILI9325_CMD_WRAM));
	bus_lcd_close(node);
	return 0;
}

/**
 *@brief:	   drv_ILI9325_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:	   static OK
 */
static s32 drv_ILI9325_display_onoff(DevLcdNode *lcd, u8 sta)
{
	u16 tmp[2];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	if(sta == 1)
	{
		bus_lcd_write_cmd(node, (0X07));
		tmp[0] = 0x0173;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	}
	else
	{
		bus_lcd_write_cmd(node, (0X07));
		tmp[0] = 0x00;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	}
	bus_lcd_close(node);
	return 0;
}

/**
 *@brief:	   drv_ILI9325_init
 *@details:    初始化FSMC，并且读取ILI9325的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:	   
 */
s32 drv_ILI9325_init(DevLcdNode *lcd)
{
	u16 data;
	u16 tmp[16];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(50);
	bus_lcd_rst(node, 1);
	Delay(50);
	/*
		读9325的ID
		
	*/
	bus_lcd_write_cmd(node, (0x0000));
	tmp[0] = 0x0001;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0x0000));
	bus_lcd_read_data(node, (u8*)tmp, 1);
    data = tmp[0]; 

	ILI9341_DEBUG(LOG_DEBUG, "read reg:%04x\r\n", data);
	if(data != TftLcdILI9325Drv.id)
	{
		ILI9341_DEBUG(LOG_DEBUG, "lcd drive no 9325\r\n");	
		bus_lcd_close(node);
		return -1;
	}
	
	bus_lcd_write_cmd(node, (0x00E5));
	tmp[0] = 0x78F0;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0001));
	tmp[0] = 0x0100;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0002));
	tmp[0] = 0x0700;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0003));
	tmp[0] = 0x1030;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0004));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0008));
	tmp[0] = 0x0202;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0009));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x000A));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x000C));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x000D));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x000F));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0010));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0011));
	tmp[0] = 0x0007;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0012));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0013));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0007));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0010));
	tmp[0] = 0x1690;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0011));
	tmp[0] = 0x0227;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0012));
	tmp[0] = 0x009D;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0013));
	tmp[0] = 0x1900;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0029));
	tmp[0] = 0x0025;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x002B));
	tmp[0] = 0x000D;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0030));
	tmp[0] = 0x0007;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0031));
	tmp[0] = 0x0303;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0032));
	tmp[0] = 0x0003;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0035));
	tmp[0] = 0x0206;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0036));
	tmp[0] = 0x0008;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0037));
	tmp[0] = 0x0406;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0038));
	tmp[0] = 0x0304;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0039));
	tmp[0] = 0x0007;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x003C));
	tmp[0] = 0x0602;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x003D));
	tmp[0] = 0x0008;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	/*
	Horizontal and Vertical RAM Address Position 219*319
	设置扫描窗口

	*/
	bus_lcd_write_cmd(node, (0x0050));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0051));
	tmp[0] = 0x00EF;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0052));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0053));
	tmp[0] = 0x013F;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	//-------------------------------------
	bus_lcd_write_cmd(node, (0x0060));
	tmp[0] = 0xA700;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0061));
	tmp[0] = 0x0001;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x006A));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0080));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0081));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0082));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0083));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0084));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0085));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0090));
	tmp[0] = 0x0010;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0092));
	tmp[0] = 0x0600;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0007));
	tmp[0] = 0x0133;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x00));
	tmp[0] = 0x0022;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_close(node);
	return 0;
}
/**
 *@brief:      drv_ILI9325_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *hsa, u16 *hea, u16 *vsa, u16 *vea)
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
		XY轴，实物来看，方向取决于横屏还是竖屏
		CP轴，是控制器显存，
		映射关系取决于扫描方向
	*/
	/* 
		横屏，用户视角的XY坐标，跟LCD扫描的CP坐标要进行一个对调
		而且，9325在横竖屏也要进行映射
		
	*/
	if(lcd->dir == W_LCD)
	{
		*hsa = (lcd->height - ey) - 1;
		*hea = (lcd->height - sy) - 1;
		
		*vsa = sx;
		*vea = ex;
	}
	else
	{
		*hsa = sx;
		*hea = ex;
		*vsa = sy;
		*vea = ey;
	}
	
	return 0;
}
/**
 *@brief:      drv_ILI9325_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9325_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(lcd, x, x, y, y, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);
	
	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);
	
	u16 tmp[2];
	tmp[0] = color;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	bus_lcd_close(node);
	return 0;
}
/**
 *@brief:      dev_ILI9325_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(lcd, sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);

	width = hea - hsa + 1;//得到填充的宽度
	height = vea - vsa + 1;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	//GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	DevLcdBusNode * node;
	#define TMP_BUF_SIZE 32
	u16 tmp[TMP_BUF_SIZE];
	u32 cnt;

	for(cnt = 0; cnt < TMP_BUF_SIZE; cnt ++)
	{
		tmp[cnt] = color;
	}
	
	cnt = height*width;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	while(1)
	{
		if(cnt < TMP_BUF_SIZE)
		{
			bus_lcd_write_data(node, (u8 *)tmp, cnt);
			cnt -= cnt;
		}
		else
		{
			bus_lcd_write_data(node, (u8 *)tmp, TMP_BUF_SIZE);
			cnt -= TMP_BUF_SIZE;
		}

		if(cnt <= 0)
			break;
	}
	
	bus_lcd_close(node);

	//GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	return 0;

}

/**
 *@brief:      dev_ILI9325_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(lcd, sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);

	width=(hea +1) - hsa ;//得到填充的宽度 +1是因为坐标从0开始
	height=(vea +1) - vsa;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	//GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);

	DevLcdBusNode * node;

	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_write_data(node, (u8 *)color, height*width);	
	bus_lcd_close(node);

	//GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	return 0;

} 

s32 drv_ILI9325_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 hsa,hea,vsa,vea;

	wjq_log(LOG_DEBUG, "XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);

	drv_ILI9325_xy2cp(lcd, sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);

	wjq_log(LOG_DEBUG, "HV:-%d-%d-%d-%d-\r\n", hsa, hea, vsa, vea);
	
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);	
	return 0;
}
s32 drv_ILI9325_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_flush_data(lcd->busnode, (u8 *)color,  len);	
	bus_lcd_close(lcd->busnode);
	return 0;
} 

#endif


#ifdef TFT_LCD_DRIVER_7735
#define ST7735R_CMD_WRAM 0x2c
#define ST7735R_CMD_SETX 0x2a
#define ST7735R_CMD_SETY 0x2b

s32 drv_ST7735R_init(DevLcdNode *lcd);
static s32 drv_ST7735R_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ST7735R_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_ST7735R_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_ST7735R_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_ST7735R_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_ST7735R_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_ST7735R_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_ST7735R_flush(DevLcdNode *lcd, u16 *color, u32 len);

_lcd_drv TftLcdST7735R_Drv = {
							.id = 0X7735,

							.init = drv_ST7735R_init,
							.draw_point = drv_ST7735R_drawpoint,
							.color_fill = drv_ST7735R_color_fill,
							.fill = drv_ST7735R_fill,
							.onoff = drv_ST7735R_display_onoff,
							.prepare_display = drv_ST7735R_prepare_display,
							.flush = drv_ST7735R_flush,
							.set_dir = drv_ST7735R_scan_dir,
							.backlight = drv_ST7735R_lcd_bl
							};

void drv_ST7735R_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}		
/**
 *@brief:      drv_ST7735R_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ST7735R_scan_dir(DevLcdNode *lcd, u8 dir)
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
 *@brief:      drv_ST7735R_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7735R_set_cp_addr(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{

	DevLcdBusNode * node;
	u8 tmp[4];

	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_write_cmd(node, ST7735R_CMD_SETX);
	tmp[0] = (0x00);
	tmp[1] = (sc+2);
	tmp[2] = (0X00);
	tmp[3] = (ec+2);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ST7735R_CMD_SETY));
	tmp[0] = (0);
	tmp[1] = (sp+3);
	tmp[2] = (0);
	tmp[3] = (ep+3);
	bus_lcd_write_data(node, (u8*)tmp, 4);

	bus_lcd_write_cmd(node, (ST7735R_CMD_WRAM));
	
	bus_lcd_close(node);
	
	return 0;
}

/**
 *@brief:      drv_ST7735R_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7735R_display_onoff(DevLcdNode *lcd, u8 sta)
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
 *@brief:      drv_ST7735R_init
 *@details:    初始化FSMC，并且读取ILI9341的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7735R_init(DevLcdNode *lcd)
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

	bus_lcd_write_cmd(node, (0x11));
	Delay(50);

	bus_lcd_write_cmd(node, (0xB1));
	tmp[0] = 0x01;
	tmp[1] = 0x2C;
	tmp[2] = 0x2D;
	bus_lcd_write_data(node, (u8*)tmp, 3);



	bus_lcd_write_cmd(node, (0xB2));
	tmp[0] = 0x01;
	tmp[1] = 0x2C;
	tmp[2] = 0x2D;
	bus_lcd_write_data(node, (u8*)tmp, 3);

		
	bus_lcd_write_cmd(node, (0xB3));
	tmp[0] = 0x01;
	tmp[1] = 0x2C;
	tmp[2] = 0x2D;
	tmp[3] = 0x01;
	tmp[4] = 0x2C;
	tmp[5] = 0x2D;
	bus_lcd_write_data(node, (u8*)tmp, 6);

	bus_lcd_write_cmd(node, (0xB4));
	tmp[0] = 0x07;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	//ST7735R Power Sequence
	bus_lcd_write_cmd(node, (0xC0));
	tmp[0] = 0xA2;
	tmp[1] = 0x02;
	tmp[2] = 0x84;
	bus_lcd_write_data(node, (u8*)tmp, 3);
	
	bus_lcd_write_cmd(node, (0xC1));
	tmp[0] = 0xC5;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0xC2));
	tmp[0] = 0x0A;
	tmp[1] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xC3));
	tmp[0] = 0x8A;
	tmp[1] = 0x2A;
	bus_lcd_write_data(node, (u8*)tmp, 2);
	
	bus_lcd_write_cmd(node, (0xC4));
	tmp[0] = 0x8A;
	tmp[1] = 0xEE;
	bus_lcd_write_data(node, (u8*)tmp, 2);

	bus_lcd_write_cmd(node, (0xC5));
	tmp[0] = 0x0E;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0x36));
	tmp[0] = 0xC8;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	//ST7735R Gamma Sequence
	bus_lcd_write_cmd(node, (0xe0));
	tmp[0] = 0x0f;
	tmp[1] = 0x1a;
	tmp[2] = 0x0f;
	tmp[3] = 0x18;

	tmp[4] = 0x2f;
	tmp[5] = 0x28;
	tmp[6] = 0x20;
	tmp[7] = 0x22;

	tmp[8] = 0x1f;
	tmp[9] = 0x1b;
	tmp[10] = 0x23;
	tmp[11] = 0x37;

	tmp[12] = 0x00;
	tmp[13] = 0x07;
	tmp[12] = 0x02;
	tmp[15] = 0x10;
	bus_lcd_write_data(node, (u8*)tmp, 16);
	
	bus_lcd_write_cmd(node, (0xe1));
	tmp[0] = 0x0f;
	tmp[1] = 0x1b;
	tmp[2] = 0x0f;
	tmp[3] = 0x17;

	tmp[4] = 0x33;
	tmp[5] = 0x2c;
	tmp[6] = 0x29;
	tmp[7] = 0x2e;

	tmp[8] = 0x30;
	tmp[9] = 0x30;
	tmp[10] = 0x39;
	tmp[11] = 0x3f;

	tmp[12] = 0x00;
	tmp[13] = 0x07;
	tmp[12] = 0x03;
	tmp[15] = 0x10;
	bus_lcd_write_data(node, (u8*)tmp, 16);

	bus_lcd_write_cmd(node, (0x2a));
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	tmp[2] = 0x00;
	tmp[3] = 0x7f;
	bus_lcd_write_data(node, (u8*)tmp, 4);
	
	bus_lcd_write_cmd(node, (0x2b));
	tmp[0] = 0x00;
	tmp[1] = 0x00;
	tmp[2] = 0x00;
	tmp[3] = 0x9f;
	bus_lcd_write_data(node, (u8*)tmp, 4);
	
	bus_lcd_write_cmd(node, (0xF0));
	tmp[0] = 0x01;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0xF6));
	tmp[0] = 0x00;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	//65k mode 
	bus_lcd_write_cmd(node, (0x3A));
	tmp[0] = 0x05;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	//Display on	 
	bus_lcd_write_cmd(node, (0x29));

	bus_lcd_close(node);
	
	Delay(50);
	
	return 0;
}
/**
 *@brief:      drv_ST7735R_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7735R_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
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
 *@brief:      drv_ST7735R_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7735R_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ST7735R_xy2cp(lcd, x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ST7735R_set_cp_addr(lcd, sc, ec, sp, ep);

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
 *@brief:      drv_ST7735R_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7735R_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i;
	u16 sc,ec,sp,ep;

	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", sx, ex, sy, ey);

	drv_ST7735R_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", sc, ec, sp, ep);
	
	drv_ST7735R_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;//得到填充的宽度 +1是因为坐标从0开始
	height=(ep+1)-sp;//高度
	
	wjq_log(LOG_DEBUG, "ST7735R width:%d, height:%d\r\n", width, height);

	u8 *tmp;

	tmp = (u8 *)wjq_malloc(width*2);

	for(i = 0; i < width*2;)
	{
		tmp[i++] = color>>8;
		tmp[i++] = color&0xff;
	}
	
	GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);

	for(i = 0; i < height;i++)
	{
			bus_lcd_write_data(lcd->busnode, tmp, width*2);
	}
	
	bus_lcd_close(lcd->busnode);

	wjq_free(tmp);
	
	GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);

	return 0;

}

/**
 *@brief:      drv_ST7735R_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7735R_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;
	u16 *pcc;
	
	drv_ST7735R_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ST7735R_set_cp_addr(lcd, sc, ec, sp, ep);

	width=(ec+1)-sc;
	height=(ep+1)-sp;

	wjq_log(LOG_DEBUG, "fill width:%d, height:%d\r\n", width, height);
	
	u8 *tmp;

	tmp = (u8 *)wjq_malloc(width*2);

	GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
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

s32 drv_ST7735R_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;
	wjq_log(LOG_DEBUG, "XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);
	drv_ST7735R_xy2cp(lcd, sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	wjq_log(LOG_DEBUG, "cp:-%d-%d-%d-%d-\r\n", sc, ec, sp, ep);
	drv_ST7735R_set_cp_addr(lcd, sc, ec, sp, ep);	
	return 0;
}

s32 drv_ST7735R_flush(DevLcdNode *lcd, u16 *color, u32 len)
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


