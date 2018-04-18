/**
 * @file            dev_lcd.c
 * @brief           LCD 中间层
 * @author          wujique
 * @date            2018年4月17日 星期二
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月17日 星期二
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
#include "wujique_log.h"
#include "dev_lcd.h"

//#define DEV_LCD_DEBUG

#ifdef DEV_LCD_DEBUG
#define LCD_DEBUG	wjq_log 
#else
#define LCD_DEBUG(a, ...)
#endif


struct _strlcd_obj LCD;

extern _lcd_drv TftLcdILI9341Drv;
extern _lcd_drv TftLcdILI9325Drv;
extern _lcd_drv CogLcdST7565Drv;
extern _lcd_drv OledLcdSSD1615rv;

/**
 *@brief:      dev_lcd_setdir
 *@details:    设置横屏或竖屏，扫描方向
 *@param[in]   u8 dir       0 竖屏1横屏
               u8 scan_dir  参考宏定义L2R_U2D       
 *@param[out]  无
 *@retval:     
 */
void dev_lcd_setdir(u8 dir, u8 scan_dir)
{
	struct _strlcd_obj *obj;
	obj = &LCD;

	u16 temp;
	u8 scan_dir_tmp;
	
	if(dir != obj->dir)//切换屏幕方向	
	{
		
		obj->dir = obj->dir^0x01;
		temp = obj->width;
		obj->width = obj->height;
		obj->height = temp;
		LCD_DEBUG(LOG_DEBUG, "set dir w:%d, h:%d\r\n", obj->width, obj->height);
	}
	
	
	if(obj->dir == W_LCD)//横屏，扫描方向映射转换
	{
		/*
			横屏	 竖屏
			LR----UD
			RL----DU
			UD----RL
			DU----LR
			UDLR----LRUD
		*/
		scan_dir_tmp = 0;
		if((scan_dir&LRUD_BIT_MASK) == 0)
		{
			scan_dir_tmp += LRUD_BIT_MASK;
		}

		if((scan_dir&LR_BIT_MASK) == LR_BIT_MASK)
		{
			scan_dir_tmp += UD_BIT_MASK;	
		}

		if((scan_dir&UD_BIT_MASK) == 0)
		{
			scan_dir_tmp += LR_BIT_MASK;
		}
	}
	else
	{
		scan_dir_tmp = scan_dir;
	}
	
	obj->scandir = scan_dir_tmp;
	
	obj->drv->set_dir(obj->scandir);
}


s32 dev_lcd_init(void)
{
	s32 ret = -1;

	#if 0
	/*初始化8080接口，包括背光信号*/
	bus_8080interface_init();

	if(ret != 0)
	{
		/*尝试初始化9341*/
		ret = drv_ILI9341_init();
		if(ret == 0)
		{
			LCD.drv = &TftLcdILI9341Drv;//将9341驱动赋值到LCD
			LCD.dir = H_LCD;//默认竖屏
			LCD.height = 320;
			LCD.width = 240;
		}
	}
	

	if(ret != 0)
	{
		/* 尝试初始化9325 */
		ret = drv_ILI9325_init();
		if(ret == 0)
		{
			LCD.drv = &TftLcdILI9325Drv;
			LCD.dir = H_LCD;
			LCD.height = 320;
			LCD.width = 240;
		}
	}

	#else
	#if 1
	if(ret != 0)
	{
		/* 初始化COG 12864 LCD */
		ret = drv_ST7565_init();
		if(ret == 0)
		{
			LCD.drv = &CogLcdST7565Drv;
			LCD.dir = W_LCD;
			LCD.height = 64;
			LCD.width = 128;
		}
	}
	#else
	if(ret != 0)
	{
		/* 初始化OLED LCD */
		ret = drv_ssd1615_init();
		if(ret == 0)
		{
			LCD.drv = &OledLcdSSD1615rv;
			LCD.dir = W_LCD;
			LCD.height = 64;
			LCD.width = 128;
		}
	}
	#endif
	
	#endif
	/*设置屏幕方向，扫描方向*/
	dev_lcd_setdir(W_LCD, U2D_L2R);
	LCD.drv->onoff(1);//打开显示
	bus_8080_lcd_bl(1);//打开背光	
	LCD.drv->color_fill(0, LCD.width, 0, LCD.height, YELLOW);
	
	return 0;
}


s32 dev_lcd_drawpoint(u16 x, u16 y, u16 color)
{
	return LCD.drv->draw_point(x, y, color);
}

s32 dev_lcd_prepare_display(u16 sx, u16 ex, u16 sy, u16 ey)
{
	return LCD.drv->prepare_display(sx, ex, sy, ey);
}

s32 dev_lcd_display_onoff(u8 sta)
{
	return LCD.drv->onoff(sta);
}
/* 

从tslib拷贝一些显示函数到这里
这些函数可以归为GUI


*/
#include "font.h"

/**
 *@brief:      line
 *@details:    画一条线
 *@param[in]   int x1           
               int y1           
               int x2           
               int y2           
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void line (int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs (dx) < abs (dy)) 
	{
		if (y1 > y2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2)
		{
			LCD.drv->draw_point (x1 >> 16, y1, colidx);
			x1 += dx;
			y1++;
		}
	} 
	else 
	{
		if (x1 > x2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) 
		{
			LCD.drv->draw_point (x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

/**
 *@brief:     put_cross
 *@details:   画十字
 *@param[in]  int x            
              int y            
              unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void put_cross(int x, int y, unsigned colidx)
{
	line (x - 10, y, x - 2, y, colidx);
	line (x + 2, y, x + 10, y, colidx);
	line (x, y - 10, x, y - 2, colidx);
	line (x, y + 2, x, y + 10, colidx);

	line (x - 6, y - 9, x - 9, y - 9, colidx + 1);
	line (x - 9, y - 8, x - 9, y - 6, colidx + 1);
	line (x - 9, y + 6, x - 9, y + 9, colidx + 1);
	line (x - 8, y + 9, x - 6, y + 9, colidx + 1);
	line (x + 6, y + 9, x + 9, y + 9, colidx + 1);
	line (x + 9, y + 8, x + 9, y + 6, colidx + 1);
	line (x + 9, y - 6, x + 9, y - 9, colidx + 1);
	line (x + 8, y - 9, x + 6, y - 9, colidx + 1);

}
/**
 *@brief:      put_char
 *@details:    显示一个英文
 *@param[in]   int x       
               int y       
               int c       
               int colidx  
 *@param[out]  无
 *@retval:     
 */
void put_char(int x, int y, int c, int colidx)
{
	int i,j,bits;

	for (i = 0; i < font_vga_8x8.height; i++) 
	{
		bits = font_vga_8x8.data [font_vga_8x8.height * c + i];
		for (j = 0; j < font_vga_8x8.width; j++, bits <<= 1)
		{
			if (bits & 0x80)
			{
				LCD.drv->draw_point(x + j, y + i, colidx);
			}
		}
	}
}
/**
 *@brief:      put_string
 *@details:    显示一个字符串
 *@param[in]   int x            
               int y            
               char *s          
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void put_string(int x, int y, char *s, unsigned colidx)
{
	int i;
	
	for (i = 0; *s; i++, x += font_vga_8x8.width, s++)
		put_char(x, y, *s, colidx);
}
/**
 *@brief:      put_string_center
 *@details:    居中显示一个字符串
 *@param[in]   int x            
               int y            
               char *s          
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void put_string_center(int x, int y, char *s, unsigned colidx)
{
	int sl = strlen (s);
	
    put_string (x - (sl / 2) * font_vga_8x8.width,
                y - font_vga_8x8.height / 2, s, colidx);
}

/**
 *@brief:      rect
 *@details:    画一个矩形框
 *@param[in]   int x1           
               int y1           
               int x2           
               int y2           
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void rect (int x1, int y1, int x2, int y2, unsigned colidx)
{
	line (x1, y1, x2, y1, colidx);
	line (x2, y1, x2, y2, colidx);
	line (x2, y2, x1, y2, colidx);
	line (x1, y2, x1, y1, colidx);
}


/**
 *@brief:      dev_lcd_test
 *@details:    LCD测试函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void dev_lcd_test(void)
{

	while(1)
	{		
		#if 0//测试彩屏
		LCD.drv->color_fill(0,LCD.width,0,LCD.height,BLUE);
		Delay(1000);
		LCD.drv->color_fill(0,LCD.width/2,0,LCD.height/2,RED);
		Delay(1000);
		LCD.drv->color_fill(0,LCD.width/4,0,LCD.height/4,GREEN);
		Delay(1000);
		
		put_string_center (LCD.width/2+50, LCD.height/2+50,
			   "ADCD WUJIQUE !", 0xF800);
		Delay(1000);
		#else//测试COG LCD跟OLED LCD
		put_string_center (20, 32,
			   "ADCD WUJIQUE !", BLACK);
		Delay(1000);
		LCD.drv->color_fill(0,LCD.width,0,LCD.height,WHITE);
		Delay(1000);
		LCD.drv->color_fill(0,LCD.width,0,LCD.height,BLACK);
		Delay(1000);
		LCD.drv->color_fill(0,LCD.width,0,LCD.height,WHITE);
		Delay(1000);
		#endif
	}

}

