/**
 * @file            dev_cog12864.c
 * @brief           COG LCD驱动
 * @author          wujique
 * @date            2018年1月10日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月10日 星期三
 *   作    者:         屋脊雀工作室
 *   修改内容:   创建文件
		版权说明：
		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且保留版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
/*

	COG LCD 的驱动

*/


#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "main.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"
#include "alloc.h"
#include "dev_lcdbus.h"
#include "dev_lcd.h"
#include "dev_str7565.h"


/*
	驱动使用的数据结构，不对外
*/
struct _cog_drv_data
{
	u8 gram[8][128];

	/*刷新区域*/
	u16 sx;
	u16 ex;
	u16 sy;
	u16 ey;
	u16 disx;
	u16 disy;

};	


#ifdef TFT_LCD_DRIVER_COG12864

s32 drv_ST7565_init(DevLcdNode *lcd);
static s32 drv_ST7565_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ST7565_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_ST7565_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_ST7565_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_ST7565_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_ST7565_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_ST7565_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_ST7565_flush(DevLcdNode *lcd, u16 *color, u32 len);


/*

	定义一个TFT LCD，使用ST7565驱动IC的设备

*/
_lcd_drv CogLcdST7565Drv = {
							.id = 0X7565,

							.init = drv_ST7565_init,
							.draw_point = drv_ST7565_drawpoint,
							.color_fill = drv_ST7565_color_fill,
							.fill = drv_ST7565_fill,
							.onoff = drv_ST7565_display_onoff,
							.prepare_display = drv_ST7565_prepare_display,
							.set_dir = drv_ST7565_scan_dir,
							.backlight = drv_ST7565_lcd_bl,
							.flush = drv_ST7565_flush
							};

void drv_ST7565_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);
}
	
/**
 *@brief:      drv_ST7565_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ST7565_scan_dir(DevLcdNode *lcd, u8 dir)
{
	return;
}

/**
 *@brief:      drv_ST7565_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
#if 0
static s32 drv_ST7565_set_cp_addr(DevLcd *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{
	return 0;
}
#endif
/**
 *@brief:      drv_ST7565_refresh_gram
 *@details:       刷新指定区域到屏幕上
                  坐标是横屏模式坐标
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7565_refresh_gram(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{	
	struct _cog_drv_data *drvdata; 
	u8 i;
	DevLcdBusNode *node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	//uart_printf("drv_ST7565_refresh:%d,%d,%d,%d\r\n", sc,ec,sp,ep);
	drvdata = (struct _cog_drv_data *)lcd->pri;
	
    for(i=sp/8; i <= ep/8; i++)
    {
        bus_lcd_write_cmd (node, 0xb0+i);    //设置页地址（0~7）
        bus_lcd_write_cmd(node, ((sc>>4)&0x0f)+0x10);      //设置显示位置—列高地址
        bus_lcd_write_cmd(node, sc&0x0f);      //设置显示位置—列低地址

         bus_lcd_write_data(node, &(drvdata->gram[i][sc]), ec-sc+1);

	}
	bus_lcd_close(node);
	
	return 0;
}

/**
 *@brief:      drv_ST7565_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7565_display_onoff(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	if(sta == 1)
	{
		bus_lcd_write_cmd (node, 0XCF);  //DISPLAY ON
	}
	else
	{
		bus_lcd_write_cmd (node, 0XCE);  //DISPLAY OFF	
	}
	bus_lcd_close(node);
	return 0;
}

/**
 *@brief:      drv_ST7565_init
 *@details:    
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7565_init(DevLcdNode *lcd)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(50);
	bus_lcd_rst(node, 1);
	Delay(50);
	
	bus_lcd_write_cmd (node, 0xe2);//软复位
	Delay(50);
	bus_lcd_write_cmd (node, 0x2c);//升压步骤1
	Delay(50);
	bus_lcd_write_cmd (node, 0x2e);//升压步骤2
	Delay(50);
	bus_lcd_write_cmd (node, 0x2f);//升压步骤3
	Delay(50);
	
	bus_lcd_write_cmd (node, 0x24);//对比度粗调，范围0X20，0X27
	bus_lcd_write_cmd (node, 0x81);//对比度微调
	bus_lcd_write_cmd (node, 0x25);//对比度微调值 0x00-0x3f
	
	bus_lcd_write_cmd (node, 0xa2);// 偏压比
	bus_lcd_write_cmd (node, 0xc8);//行扫描，从上到下
	bus_lcd_write_cmd (node, 0xa0);//列扫描，从左到右
	bus_lcd_write_cmd (node, 0x40);//起始行，第一行
	bus_lcd_write_cmd (node, 0xaf);//开显示

	bus_lcd_close(node);
	
	wjq_log(LOG_INFO, "drv_ST7565_init finish\r\n");

	/*申请显存，永不释放*/
	lcd->pri = (void *)wjq_malloc(sizeof(struct _cog_drv_data));
	memset((char*)lcd->pri, 0x00, 128*8);//要改为动态判断显存大小
	
	//drv_ST7565_refresh_gram(lcd, 0,127,0,63);

	return 0;
}

/**
 *@brief:      drv_ST7565_xy2cp
 *@details:    将xy坐标转换为CP坐标
 			   对于COG黑白屏来说，CP坐标就是横屏坐标，
 			   转竖屏后，CP坐标还是横屏坐标，
 			   也就是说当竖屏模式，就需要将XY坐标转CP坐标
 			   横屏不需要转换
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7565_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{

	return 0;
}
/**
 *@brief:      drv_ST7565_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ST7565_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 xtmp,ytmp;
	u16 page, colum;

	struct _cog_drv_data *drvdata;
	
	DevLcdBusNode * node;
	
	drvdata = (struct _cog_drv_data *)lcd->pri;

	if(x > lcd->width)
		return -1;
	if(y > lcd->height)
		return -1;

	if(lcd->dir == W_LCD)
	{
		xtmp = x;
		ytmp = y;
	}
	else//如果是竖屏，XY轴跟显存的映射要对调
	{
		xtmp = y;
		ytmp = lcd->width-1-x;
	}
	
	page = ytmp/8; //页地址
	colum = xtmp;//列地址
	
	if(color == BLACK)
	{
		drvdata->gram[page][colum] |= (0x01<<(ytmp%8));
	}
	else
	{
		drvdata->gram[page][colum] &= ~(0x01<<(ytmp%8));
	}

	/*效率不高*/
	node = bus_lcd_open(lcd->dev.buslcd);
    bus_lcd_write_cmd (node, 0xb0 + page );   
    bus_lcd_write_cmd (node, ((colum>>4)&0x0f)+0x10); 
    bus_lcd_write_cmd (node, colum&0x0f);    
    bus_lcd_write_data (node, &(drvdata->gram[page][colum]), 1);
	bus_lcd_close (node);
	return 0;
}
/**
 *@brief:      drv_ST7565_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7565_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 page, colum;

	
	struct _cog_drv_data *drvdata;

	//uart_printf("drv_ST7565_fill:%d,%d,%d,%d\r\n", sx,ex,sy,ey);

	drvdata = (struct _cog_drv_data *)lcd->pri;

	/*防止坐标溢出*/
	if(sy >= lcd->height)
	{
		sy = lcd->height-1;
	}
	if(sx >= lcd->width)
	{
		sx = lcd->width-1;
	}
	
	if(ey >= lcd->height)
	{
		ey = lcd->height-1;
	}
	if(ex >= lcd->width)
	{
		ex = lcd->width-1;
	}
	
	for(j=sy;j<=ey;j++)
	{
		//uart_printf("\r\n");
		
		for(i=sx;i<=ex;i++)
		{

			if(lcd->dir == W_LCD)
			{
				xtmp = i;
				ytmp = j;
			}
			else//如果是竖屏，XY轴跟显存的映射要对调
			{
				xtmp = j;
				ytmp = lcd->width-1-i;
			}

			page = ytmp/8; //页地址
			colum = xtmp;//列地址
			
			if(color == BLACK)
			{
				drvdata->gram[page][colum] |= (0x01<<(ytmp%8));
				//uart_printf("*");
			}
			else
			{
				drvdata->gram[page][colum] &= ~(0x01<<(ytmp%8));
				//uart_printf("-");
			}
		}
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	if(lcd->dir == W_LCD)
	{
		drv_ST7565_refresh_gram(lcd, sx,ex,sy,ey);
	}
	else
	{
		drv_ST7565_refresh_gram(lcd, sy, ey, lcd->width-ex-1, lcd->width-sx-1); 	
	}
		
	return 0;
}


/**
 *@brief:      drv_ST7565_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ST7565_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 xlen,ylen;
	u16 page, colum;
	u32 index;
	
	struct _cog_drv_data *drvdata;

	//uart_printf("drv_ST7565_fill:%d,%d,%d,%d\r\n", sx,ex,sy,ey);

	drvdata = (struct _cog_drv_data *)lcd->pri;

	/*xlen跟ylen是用来取数据的，不是填LCD*/
	xlen = ex-sx+1;//全包含
	ylen = ey-sy+1;

	/*防止坐标溢出*/
	if(sy >= lcd->height)
	{
		sy = lcd->height-1;
	}
	if(sx >= lcd->width)
	{
		sx = lcd->width-1;
	}
	
	if(ey >= lcd->height)
	{
		ey = lcd->height-1;
	}
	if(ex >= lcd->width)
	{
		ex = lcd->width-1;
	}
	
	for(j=sy;j<=ey;j++)
	{
		//uart_printf("\r\n");

		index = (j-sy)*xlen;
		
		for(i=sx;i<=ex;i++)
		{

			if(lcd->dir == W_LCD)
			{
				xtmp = i;
				ytmp = j;
			}
			else//如果是竖屏，XY轴跟显存的映射要对调
			{
				xtmp = j;
				ytmp = lcd->width-1-i;
			}

			page = ytmp/8; //页地址
			colum = xtmp;//列地址
			
			if(*(color+index+i-sx) == BLACK)
			{
				drvdata->gram[page][colum] |= (0x01<<(ytmp%8));
				//uart_printf("*");
			}
			else
			{
				drvdata->gram[page][colum] &= ~(0x01<<(ytmp%8));
				//uart_printf("-");
			}
		}
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	if(lcd->dir == W_LCD)
	{
		drv_ST7565_refresh_gram(lcd, sx,ex,sy,ey);
	}
	else
	{

		drv_ST7565_refresh_gram(lcd, sy, ey, lcd->width-ex-1, lcd->width-sx-1); 	
	}
	//uart_printf("refresh ok\r\n");		
	return 0;
}

s32 drv_ST7565_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	struct _cog_drv_data *drvdata;
	drvdata = (struct _cog_drv_data *)lcd->pri;

	drvdata->ex = ex;
	drvdata->sx = sx;
	drvdata->sy = sy;
	drvdata->ey = ey;

	drvdata->disx = sx;
	drvdata->disy = sy;
	
	return 0;
}

s32 drv_ST7565_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 page, colum;
	u32 index;
	
	u16 sx,ex,sy,ey;
	
	struct _cog_drv_data *drvdata;

	//uart_printf("drv_ST7565_fill:%d,%d,%d,%d\r\n", sx,ex,sy,ey);

	drvdata = (struct _cog_drv_data *)lcd->pri;
	
	sx = lcd->width;
	ex = 0;
	sy = lcd->height;
	ey = 0;

	index = 0;
	while(1)
	{
		if(index >= len)
			break;

		if((drvdata->disx < lcd->width)&&(drvdata->disy < lcd->height))
		{			
			if(sx >drvdata->disx)
				sx = drvdata->disx;

		
			if(ex < drvdata->disx)
				ex = drvdata->disx;

			if(sy >drvdata->disy)
				sy = drvdata->disy;

			if(ey < drvdata->disy)
				ey = drvdata->disy;

			if(lcd->dir == W_LCD)
			{
				xtmp = drvdata->disx;
				ytmp = drvdata->disy;
			}
			else//如果是竖屏，XY轴跟显存的映射要对调
			{
				xtmp = drvdata->disy;
				ytmp = lcd->width-drvdata->disx;
			}

			//wjq_log(LOG_DEBUG, " %d, %d \r\n", xtmp,ytmp);
			
			page = ytmp/8; //页地址
			colum = xtmp;//列地址
				
			if(*(color+index) == BLACK)
			{
				drvdata->gram[page][colum] |= (0x01<<(ytmp%8));
				//uart_printf("*");
			}
			else
			{
				drvdata->gram[page][colum] &= ~(0x01<<(ytmp%8));
				//uart_printf("-");
			}
		}

		drvdata->disx++;
		if(drvdata->disx > drvdata->ex)
		{
			drvdata->disx = drvdata->sx;
			drvdata->disy++;
			
			if(drvdata->disy> drvdata->ey)
				drvdata->disy = drvdata->sy;
		}
		index++;
		
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	if(lcd->dir == W_LCD)
	{
		drv_ST7565_refresh_gram(lcd, sx,ex,sy,ey);
	}
	else
	{

		drv_ST7565_refresh_gram(lcd, sy, ey, lcd->width-ex-1, lcd->width-sx-1); 	
	}
	//uart_printf("refresh ok\r\n");		
	return 0;
} 

#endif


/*

	OLED 跟 COG LCD 操作类似
	仅仅初始化不一样
	OLED有两种驱动，SSD1315，SSD1615，一样的。
*/
#ifdef TFT_LCD_DRIVER_SSD1615

/**
 *@brief:	   drv_ssd1615_init
 *@details:    
 *@param[in]   void  
 *@param[out]  无
 *@retval:	   
 */
s32 drv_ssd1615_init(DevLcdNode *lcd)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(50);
	bus_lcd_rst(node, 1);
	Delay(50);
	
	bus_lcd_write_cmd (node, 0xAE);//--turn off oled panel
	bus_lcd_write_cmd (node, 0x00);//---set low column address
	bus_lcd_write_cmd (node, 0x10);//---set high column address
	bus_lcd_write_cmd (node, 0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	bus_lcd_write_cmd (node, 0x81);//--set contrast control register
	bus_lcd_write_cmd (node, 0xCF); // Set SEG Output Current Brightness
	bus_lcd_write_cmd (node, 0xA1);//--Set SEG/Column Mapping	  0xa0左右反置 0xa1正常
	bus_lcd_write_cmd (node, 0xC8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	bus_lcd_write_cmd (node, 0xA6);//--set normal display
	bus_lcd_write_cmd (node, 0xA8);//--set multiplex ratio(1 to 64)
	bus_lcd_write_cmd (node, 0x3f);//--1/64 duty
	bus_lcd_write_cmd (node, 0xD3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	bus_lcd_write_cmd (node, 0x00);//-not offset
	bus_lcd_write_cmd (node, 0xd5);//--set display clock divide ratio/oscillator frequency
	bus_lcd_write_cmd (node, 0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	bus_lcd_write_cmd (node, 0xD9);//--set pre-charge period
	bus_lcd_write_cmd (node, 0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	bus_lcd_write_cmd (node, 0xDA);//--set com pins hardware configuration
	bus_lcd_write_cmd (node, 0x12);
	bus_lcd_write_cmd (node, 0xDB);//--set vcomh
	bus_lcd_write_cmd (node, 0x40);//Set VCOM Deselect Level
	bus_lcd_write_cmd (node, 0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	bus_lcd_write_cmd (node, 0x02);//
	bus_lcd_write_cmd (node, 0x8D);//--set Charge Pump enable/disable
	bus_lcd_write_cmd (node, 0x14);//--set(0x10) disable
	bus_lcd_write_cmd (node, 0xA4);// Disable Entire Display On (0xa4/0xa5)
	bus_lcd_write_cmd (node, 0xA6);// Disable Inverse Display On (0xa6/a7) 
	bus_lcd_write_cmd (node, 0xAF);//--turn on oled panel

	bus_lcd_write_cmd (node, 0xAF);//--turn on oled panel 
	bus_lcd_close (node);
	

	lcd->pri = (void *)wjq_malloc(sizeof(struct _cog_drv_data));
	memset((char*)lcd->pri, 0x00, 128*8);//要改为动态判断显存大小
	
	//drv_ST7565_refresh_gram(lcd, 0,127,0,63);
	
	wjq_log(LOG_INFO, "dev_ssd1615_init finish\r\n");
	return 0;
}

/**
 *@brief:      drv_ssd1615_display_onoff
 *@details:    SSD1615打开或关闭显示
 *@param[in]   DevLcd *lcd  
               u8 sta       
 *@param[out]  无
 *@retval:     
 */
s32 drv_ssd1615_display_onoff(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	if(sta == 1)
	{
    	bus_lcd_write_cmd (node, 0X8D);  //SET DCDC命令
    	bus_lcd_write_cmd (node, 0X14);  //DCDC ON
    	bus_lcd_write_cmd (node, 0XAF);  //DISPLAY ON
	}
	else
	{
		bus_lcd_write_cmd (node, 0X8D);  //SET DCDC命令
    	bus_lcd_write_cmd (node, 0X10);  //DCDC OFF
    	bus_lcd_write_cmd (node, 0XAE);  //DISPLAY OFF	
	}
	bus_lcd_close (node);
	
	return 0;
}

_lcd_drv OledLcdSSD1615rv = {
							.id = 0X1315,

							.init = drv_ssd1615_init,
							.draw_point = drv_ST7565_drawpoint,
							.color_fill = drv_ST7565_color_fill,
							.fill = drv_ST7565_fill,
							.onoff = drv_ssd1615_display_onoff,
							.prepare_display = drv_ST7565_prepare_display,
							.set_dir = drv_ST7565_scan_dir,
							.backlight = drv_ST7565_lcd_bl,
							.flush = drv_ST7565_flush
							};
#endif

