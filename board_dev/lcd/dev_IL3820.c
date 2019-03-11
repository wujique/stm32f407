/**
 * @file            dev_IL3820.c
 * @brief           TFT LCD 驱动芯片IL3820驱动程序
 * @author          wujique
 * @date            2017年11月8日 星期五
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年81月8日 星期五
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
	3820 大连佳显 GDEH154D27
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
#include "dev_IL3820.h"

#define DEV_IL3820_DEBUG

#ifdef DEV_IL3820_DEBUG
#define IL3820_DEBUG	wjq_log 
#else
#define IL3820_DEBUG(a, ...)
#endif

extern void Delay(__IO uint32_t nTime);

/*

	电子纸的显存扫描方向：
	gram[0], 是竖屏时左下角角横向8个点。
	这是做些color fill和fill函数要考虑的。

	例如大连佳显的1.54寸 200*200像素的
	一个page就是竖屏的一行X轴方向的点。
	第1个page是竖屏靠近排线的一行。
	也就是说，一个page有 200/8 个字节，
	LCD总共有200page。

	我们定义坐标(0,0)， 是page199的第1个字节第8个bit。

*/
#define IL3820_PAGE_SIZE ((lcd->dev.width+7)/8)

struct _epaper3820_drv_data
{
	/*显存，通过动态申请，根据屏幕大小申请*/
	u8 *bgram;

	/*刷新区域*/
	u16 sx;
	u16 ex;
	u16 sy;
	u16 ey;
	u16 disx;
	u16 disy;

};



#ifdef TFT_LCD_DRIVER_3820

s32 drv_IL3820_init(DevLcdNode *lcd);
static s32 drv_IL3820_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_IL3820_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_IL3820_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_IL3820_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_IL3820_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_IL3820_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_IL3820_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_IL3820_flush(DevLcdNode *lcd, u16 *color, u32 len);
s32 drv_IL3820_update(DevLcdNode *lcd);


/*

	定义一个TFT LCD，使用IL3820驱动IC的设备

*/
_lcd_drv TftLcdIL3820Drv = {
							.id = 0X3820,

							.init = drv_IL3820_init,
							.draw_point = drv_IL3820_drawpoint,
							.color_fill = drv_IL3820_color_fill,
							.fill = drv_IL3820_fill,
							.onoff = drv_IL3820_display_onoff,
							.prepare_display = drv_IL3820_prepare_display,
							.flush = drv_IL3820_flush,
							.set_dir = drv_IL3820_scan_dir,
							.backlight = drv_IL3820_lcd_bl,
							.update = drv_IL3820_update,
							};


/*
	IL3820, 每个字节的SPI通信都需要CS下降沿
	为了不影响LCD BUS，对CS的操作放在驱动中，
	属于一种特殊情况

	mcu_spi_cs
*/
s32 drv_il3820_write_cmd(DevLcdBusNode *node, u8 cmd)
{
	mcu_spi_cs((DevSpiChNode *)node->basenode, 0);
	bus_lcd_write_cmd(node, cmd);
	mcu_spi_cs((DevSpiChNode *)node->basenode, 1);
}

s32 drv_il3820_write_data(DevLcdBusNode *node, u8 *data, u32 len)
{
	mcu_spi_cs((DevSpiChNode *)node->basenode, 0);
	bus_lcd_write_data(node, data, len);
	mcu_spi_cs((DevSpiChNode *)node->basenode, 1);
}


/*
	墨水屏没有背光，有前置光源
*/
void drv_IL3820_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}
	
/**
 *@brief:      drv_IL3820_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_IL3820_scan_dir(DevLcdNode *lcd, u8 dir)
{

}

/**
 *@brief:      drv_IL3820_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL3820_set_cp_addr(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{

}
/**
 *@brief:      drv_IL3820_refresh_gram
 *@details:       刷新指定区域到屏幕上
                  坐标是横屏模式坐标
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_IL3820_refresh_gram(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{	
	struct _epaper3820_drv_data *drvdata; 

	DevLcdBusNode *node;
	u32 cnt;
	u32 gramsize;
	u16 i,j;
	u8 data;
	u16 w,l;
	
	drvdata = (struct _epaper3820_drv_data *)lcd->pri;
	node = bus_lcd_open(lcd->dev.buslcd);

	wjq_log(LOG_DEBUG, "drv_IL3820_refresh_gram: %d, %d, %d, %d\r\n ", sc, ec, sp, ep);

	/*注意，要用dev中的w和h，因为gram跟横屏竖屏调换没关系，
	只和定义一致*/
	gramsize = lcd->dev.height * IL3820_PAGE_SIZE;
	//wjq_log(LOG_DEBUG, "gram size: %d\r\n ", gramsize);

	drv_il91874_write_cmd(node, (0x24));
	for(cnt = 0; cnt <gramsize; cnt++)
		drv_il91874_write_data(node, (u8 *)&drvdata->bgram[cnt], 1);

	wjq_log(LOG_DEBUG, " DISPLAY REFRESH\r\n ");

	drv_il3820_write_cmd(node, 0x22); 
	data = 0xC7;
	drv_il3820_write_data(node, &data, 1);   
	drv_il3820_write_cmd(node, 0x20); 

	unsigned char busy;
	do
	{
		busy = mcu_io_input_readbit(node->dev.staport, node->dev.stapin);
		busy = (busy & 0x01);        
	}
	while(busy); 

	wjq_log(LOG_DEBUG, "IL3820 refresh finish\r\n ");
	bus_lcd_close(node);

	return 0;
}

/**
 *@brief:      drv_IL91874_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_IL3820_display_onoff(DevLcdNode *lcd, u8 sta)
{
	wjq_log(LOG_DEBUG, " drv_IL3820_display_onoff\r\n ");	
	return 0;
}


const unsigned char LUT_DATA[]= {    //30 bytes
0x66, 0x66, 0x44, 0x66, 0xAA, 0x11, 0x80, 0x08,
0x11, 0x18, 0x81, 0x18, 0x11, 0x88, 0x11, 0x88,
0x11, 0x88, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x5F, 0xAF, 0xFF, 0xFF, 0x2F, 0x00
};	

const unsigned char LUT_DATA_part[]={  //30 bytes
0x10, 0x18, 0x18, 0x28, 0x18, 0x18, 0x18, 0x18,
0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x13, 0x11, 0x22, 0x63,
0x11, 0x00, 0x00, 0x00, 0x00, 0x00
};				

void EPD_select_LUT(DevLcdBusNode *node, const unsigned char * wave_data)
{      
	u8 tmp[16];
    unsigned char count;

     drv_il3820_write_cmd(node, 0x32);
	 for(count=0;count<30;count++)
	 {
		drv_il3820_write_data(node, (u8 *)wave_data, 1);
		wave_data++;
	 }
}

/**
 *@brief:      drv_IL3820_init
 *@details:   
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL3820_init(DevLcdNode *lcd)
{
	u16 data;
	DevLcdBusNode * node;
	u8 tmp[16];
	u8 testbuf[2];

	wjq_log(LOG_DEBUG, "drv_IL3820_init\r\n ");
	
	node = bus_lcd_open(lcd->dev.buslcd);

	mcu_io_config_in(node->dev.staport, node->dev.stapin);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(1000);
	bus_lcd_rst(node, 1);
	Delay(1000);

	drv_il3820_write_cmd(node, 0x01); //Driver output control  
	tmp[0] = 0xC7;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0x00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0x00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	
	drv_il3820_write_cmd(node, 0x0C); //softstart  
	tmp[0] = 0xD7;
	drv_il3820_write_data(node, (u8*)tmp, 1); //is necessary
	tmp[0] = 0xD6;
	drv_il3820_write_data(node, (u8*)tmp, 1);//is necessary
	tmp[0] = 0x9D;
	drv_il3820_write_data(node, (u8*)tmp, 1);//is necessary

	drv_il3820_write_cmd(node, 0x2C);     //VCOM Voltage
	tmp[0] = 0x77;
	drv_il3820_write_data(node, (u8*)tmp, 1);   

	drv_il3820_write_cmd(node, 0x3A);     //Dummy Line 	
	tmp[0] = 0x1A;
	drv_il3820_write_data(node, (u8*)tmp, 1);   
	drv_il3820_write_cmd(node, 0x3B);     //Gate time 
	tmp[0] = 0X08;
	drv_il3820_write_data(node, (u8*)tmp, 1); 
	
	drv_il3820_write_cmd(node, 0x11); //data entry mode       
	tmp[0] = 0X01;
	drv_il3820_write_data(node, (u8*)tmp, 1);

	drv_il3820_write_cmd(node, 0x3C); //BorderWavefrom
	tmp[0] = 0X33;
	drv_il3820_write_data(node, (u8*)tmp, 1);

	drv_il3820_write_cmd(node, 0x44); //set Ram-X address start/end position   
	tmp[0] = 0X00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0X18;
	drv_il3820_write_data(node, (u8*)tmp, 1);   //0x18-->(24+1)*8=200

	drv_il3820_write_cmd(node, 0x45); //set Ram-Y address start/end position          
	tmp[0] = 0XC7;
	drv_il3820_write_data(node, (u8*)tmp, 1);//0xC7-->(199+1)=200
	tmp[0] = 0X00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0X00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0X00;
	drv_il3820_write_data(node, (u8*)tmp, 1);

	drv_il3820_write_cmd(node, 0x4E);   // set RAM x address count to 0;
	tmp[0] = 0X00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	drv_il3820_write_cmd(node, 0x4F);   // set RAM y address count to 0X127;    
	tmp[0] = 0XC7;
	drv_il3820_write_data(node, (u8*)tmp, 1); //Y
	tmp[0] = 0X00;
	drv_il3820_write_data(node, (u8*)tmp, 1);
	
	unsigned char busy;
	do
	{
		busy = mcu_io_input_readbit(node->dev.staport, node->dev.stapin);
		busy =(busy & 0x01);        
	}
	while(busy); 
	
	Delay(200); 

	/* LUT 设置 */
	EPD_select_LUT(node, LUT_DATA); 

	bus_lcd_close(node);
	
	/*申请显存，永不释放*/
	lcd->pri = (void *)wjq_malloc(sizeof(struct _epaper3820_drv_data));
	memset((char*)lcd->pri, 0x00, sizeof(struct _epaper3820_drv_data));

	struct _epaper3820_drv_data *p;
	u16 gramsize;

	/*三色电子纸，要两个缓冲*/
	p = (struct _epaper3820_drv_data *)lcd->pri;

	gramsize = lcd->dev.height * IL3820_PAGE_SIZE;
	wjq_log(LOG_DEBUG, "gram size: %d\r\n ", gramsize);

	p->bgram = (u8 *)wjq_malloc(gramsize);

	wjq_log(LOG_DEBUG, "drv_IL3820_init finish\r\n ");

	return 0;
}
/**
 *@brief:      drv_IL3820_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL3820_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{

	return 0;
}
/**
 *@brief:      drv_IL3820_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_IL3820_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	wjq_log(LOG_DEBUG, " drv_IL3820_drawpoint------\r\n ");
	return 0;
}
/**
 *@brief:      drv_IL3820_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL3820_color_fill(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 color)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 page, colum;
	u16 sp, ep, sc, ec;
	
	struct _epaper3820_drv_data *drvdata;

	wjq_log(LOG_DEBUG, " drv_IL3820_color_fill\r\n ");

	drvdata = (struct _epaper3820_drv_data *)lcd->pri;

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

	#if 0
	/*求出被改动的page和cloum*/
	if(lcd->dir == H_LCD)
	{
		sp = sy;
		ep = ey;

		sc = sx;
		ec = ex;
			
	}
	else//如果是竖屏，XY轴跟显存的映射要对调
	{
		sp = sx;
		ep = ex;

		ec = (lcd->height - sy);
		sc = (lcd->height - ey);
	}
	#endif
	wjq_log(LOG_DEBUG, " %d, %d, %d, %d\r\n ", sx, ex, sy, ey);
	for(j=sy;j<=ey;j++)
	{
		//wjq_log(LOG_DEBUG, "\r\n");
		
		for(i=sx;i<=ex;i++)
		{

			if(lcd->dir == H_LCD)
			{
				xtmp = i;
				ytmp = j;
			}
			else//如果是竖屏，XY轴跟显存的映射要对调
			{
				/* 不同的算法，相当于不同的扫描方式*/
				xtmp = lcd->dev.width- 1 - j;
				ytmp = i;
			}

			/*从竖屏的角度算出page和colum*/
			page =  lcd->dev.height - 1 - ytmp; //页地址
			colum = xtmp/8;//列地址
		
			
			if(color != BLACK)
			{
				drvdata->bgram[page*IL3820_PAGE_SIZE + colum] |= (0x80>>(xtmp%8));
				//uart_printf("*");
			}	
			else
			{
				drvdata->bgram[page*IL3820_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("-");
			}
		}
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	//drv_IL3820_refresh_gram(lcd, sc, ec, sp, ep);
	wjq_log(LOG_DEBUG, " drv_IL91874_color_fill finish\r\n ");
	return 0;
}

/**
 *@brief:      drv_IL3820_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL3820_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 xlen,ylen;
	u16 page, colum;
	u32 index;
	u16 cdata;
	u16 sp, ep, sc, ec;
	
	struct _epaper3820_drv_data *drvdata;

	wjq_log(LOG_DEBUG, " drv_IL91874_fill:%d,%d,%d,%d\r\n", sx,ex,sy,ey);
	
	drvdata = (struct _epaper3820_drv_data *)lcd->pri;

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

	#if 0
	/*求出被改动的page和cloum*/
	if(lcd->dir == H_LCD)
	{
		sp = sy;
		ep = ey;

		sc = sx;
		ec = ex;
			
	}
	else//如果是竖屏，XY轴跟显存的映射要对调
	{
		sp = sx;
		ep = ex;

		ec = (lcd->height - sy);
		sc = (lcd->height - ey);
	}
	#endif
	
	for(j=sy;j<=ey;j++)
	{
		//uart_printf("\r\n");

		index = (j-sy)*xlen;
		
		for(i=sx;i<=ex;i++)
		{
			/*如果是W横屏，就将坐标转为竖屏的坐标。*/
			if(lcd->dir == H_LCD)
			{
				xtmp = i;
				ytmp = j;
			}
			else
			{
				xtmp = lcd->dev.width - j;
				ytmp = i;
			}

			/*从竖屏的角度算出page和colum*/
			page =  lcd->dev.height - ytmp; //页地址
			colum = xtmp/8;//列地址
			
			cdata = *(color+index+i-sx);

			if(cdata != BLACK)
			{
				drvdata->bgram[page*IL3820_PAGE_SIZE + colum] |= (0x80>>(xtmp%8));
				//uart_printf("*");
			}	
			else
			{
				drvdata->bgram[page*IL3820_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("-");
			}
		}
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	//drv_IL3820_refresh_gram(lcd, sc,ec,sp,ep);
			
	return 0;
}


s32 drv_IL3820_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	wjq_log(LOG_DEBUG, " drv_IL91874_prepare_display\r\n ");
	return 0;
}

s32 drv_IL3820_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	wjq_log(LOG_DEBUG, " drv_IL91874_flush\r\n ");
	return 0;
} 

s32 drv_IL3820_update(DevLcdNode *lcd)
{
	drv_IL3820_refresh_gram(lcd, 0, 0, 0, 0);	
	return 0;
}

#endif


