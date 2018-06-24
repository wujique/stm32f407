/**
 * @file                wujique_stm407.c
 * @brief           屋脊雀STM32F407开发板硬件测试程序
 * @author          wujique
 * @date            2018年1月29日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月29日 星期一
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"
#include "FreeRtos.h"
#include "font.h"
#include "emenu.h"

#include "main.h"

extern u16 PenColor;
extern u16 BackColor;


DevLcdNode * WJQTestLcd;

s32 wjq_wait_key(u8 key)
{
	while(1)
	{
		u8 keyvalue;
		s32 res;
		
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(key == 0)
				break;
			else if(keyvalue == key)
				break;	
		}
	}	
	return 0;
}

s32 wjq_test_showstr(char *s)
{
	wjq_log(LOG_DEBUG, "test:%s", s);
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, BackColor);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, s, PenColor);
	wjq_wait_key(0);
	
	return 0;
}	
/**
 *@brief:      test_tft_display
 *@details:    测试TFT LCD
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 test_tft_display(void)
{
	DevLcdNode *lcd;
	u8 step = 0;
	u8 dis = 1;
	
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open lcd err!");	
	}
	else
	{
		while(1)
		{
			if(dis == 1)
			{
				dis = 0;
				switch(step)
				{
					case 0:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, YELLOW);
						break;
					case 1:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, RED);
						break;
					case 2:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);

						dev_lcd_put_string(lcd, FONT_SONGTI_1616, 1, 120, "abc屋脊雀ADC123工作室12345678901234屋脊雀工作室", RED);

						break;
					default:
						break;
				}
				step++;
				if(step >= 3)
					step = 0;
			}
			u8 keyvalue;
			s32 res;
			
			res = dev_keypad_read(&keyvalue, 1);
			if(res == 1)
			{
				if(keyvalue == 16)
				{
					dis = 1;
				}
				else if(keyvalue == 12)
				{
					break;
				}
			}
		}
	
	}
		return 0;
}

s32 test_cogoled_lcd_display(char *name)
{
	DevLcdNode *lcd;
	u8 step = 0;
	u8 dis = 1;
	
	lcd = dev_lcd_open(name);
	if(lcd == NULL)
	{
		wjq_test_showstr("open cog lcd err!");	
	}
	else
	{
		while(1)
		{
			if(dis == 1)
			{
				dis = 0;
				switch(step)
				{
					case 0:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLACK);
						break;
					case 1:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, WHITE);
						break;
					case 2:
						dev_lcd_put_string(lcd, FONT_SONGTI_1616, 1, 56, "abc屋脊雀ADC123工作室", BLACK);
						break;
						
					default:
						break;
				}
				step++;
				if(step >= 3)
					step = 0;
			}
			u8 keyvalue;
			s32 res;
			
			res = dev_keypad_read(&keyvalue, 1);
			if(res == 1)
			{
				if(keyvalue == 16)
				{
					dis = 1;
				}
				else if(keyvalue == 12)
				{
					break;
				}
			}
		}

	}
	
	return 0;
}
s32 test_i2c_oled_display(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	return 	test_cogoled_lcd_display("i2coledlcd");
}
s32 test_vspi_oled_display(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	return 	test_cogoled_lcd_display("vspioledlcd");
}

s32 test_spi_cog_display(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	return 	test_cogoled_lcd_display("spicoglcd");
}


s32 test_lcd_pic(void)
{
	DevLcdNode *lcd;
	u8 step = 0;
	u8 dis = 1;
	
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open lcd err!");	
		return -1;
	}
	
	dev_lcd_setdir(lcd, W_LCD, L2R_D2U);
	
	wjq_test_showstr((char *)__FUNCTION__);
	dev_lcd_show_bmp(lcd, 1, 1, 320, 240, "1:/pic/女人单色.bmp");
	wjq_wait_key(16);
	dev_lcd_show_bmp(lcd, 1, 1, 320, 240, "1:/pic/女人16色.bmp");//调色板
	wjq_wait_key(16);
	dev_lcd_show_bmp(lcd, 1, 1, 320, 240, "1:/pic/女人256色.bmp");//调色板
	wjq_wait_key(16);
	dev_lcd_show_bmp(lcd, 1, 1, 320, 240, "1:/pic/女人24位.bmp");//真彩色
	return 0;
}

s32 test_lcd_font(void)
{
	wjq_test_showstr((char *)__FUNCTION__);
	return 0;
}


s32 test_sound_buzzer(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);

	while(1)
	{
		u8 keyvalue;
		s32 res;
		
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 16)
			{
				dev_buzzer_open();	
			}
			else if(keyvalue == (0x80+16))
			{
				dev_buzzer_close();
			}
			else if(keyvalue == 12)
			{
				break;
			}
		}
			
	}
	return 0;
}

s32 test_sound_fm(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);

	dev_wm8978_inout(WM8978_INPUT_DAC|WM8978_INPUT_AUX|WM8978_INPUT_ADC,
					WM8978_OUTPUT_PHONE|WM8978_OUTPUT_SPK);
	
	dev_tea5767_open();
	dev_tea5767_setfre(105700);
	wjq_wait_key(12);
	//dev_tea5767_close();
	return 0;
}

s32 test_sound_wm8978(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	fun_sound_play("2:/stereo_16bit_32k.wav", "wm8978");
	wjq_wait_key(16);
	fun_sound_stop();
	wjq_log(LOG_DEBUG,"wm8978 test out\r\n");
	return 0;
}

s32 test_sound_dac(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	fun_sound_play("1:/mono_16bit_8k.wav", "dacsound");
	wjq_wait_key(16);
	fun_sound_stop();
	wjq_log(LOG_DEBUG,"dac test out\r\n");
	return 0;
}
s32 test_sound_rec(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);

	fun_sound_rec("1:/rec8.wav");
	wjq_wait_key(16);
	fun_rec_stop();
	fun_sound_play("1:/rec8.wav", "wm8978");	
	while(1)
	{
		if(SOUND_IDLE == fun_sound_get_sta())
			break;

		u8 keyvalue;
		s32 res;
		
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
		}
	}
	wjq_log(LOG_DEBUG, "test_sound_rec OUT!\r\n");
	return 0;
}


#include "tslib-private.h"
extern struct tsdev *ts_open_module(void);
s32 test_tp_calibrate(void)
{
	DevLcdNode *lcd;

	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);

	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open lcd err!");	
	}
	else
	{
		dev_lcd_setdir(lcd, H_LCD, L2R_U2D);
		dev_touchscreen_open();
		ts_calibrate(lcd);
		dev_touchscreen_close();
	}
	
	dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);
	dev_lcd_close(lcd);
	
	return 0;
}


s32 test_tp_test(void)
{
	DevLcdNode *lcd;

	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open lcd err!");	
	}
	else
	{
		dev_lcd_setdir(lcd, H_LCD, L2R_U2D);
		dev_touchscreen_open();	
	
		struct tsdev *ts;
		ts = ts_open_module();

		struct ts_sample samp[10];
		int ret;
		u8 i =0;	
		while(1)
		{
			ret = ts_read(ts, samp, 10);
			if(ret != 0)
			{
				//uart_printf("pre:%d, x:%d, y:%d\r\n", samp[0].pressure, samp[0].x, samp[0].y);
						
				i = 0;
				
				while(1)
				{
					if(i>= ret)
						break;
					
					if(samp[i].pressure != 0 )
					{
						//uart_printf("pre:%d, x:%d, y:%d\r\n", samp.pressure, samp.x, samp.y);
						dev_lcd_drawpoint(lcd, samp[i].x, samp[i].y, RED); 
					}
					i++;
				}
			}

			u8 keyvalue;
			s32 res;
			
			res = dev_keypad_read(&keyvalue, 1);
			if(res == 1)
			{
				if(keyvalue == 8)
				{
					dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);
				}
				else if(keyvalue == 12)
				{
					break;
				}
			}
		}

		dev_touchscreen_close();
	}
	return 0;
}


s32 test_key(void)
{
	u8 tmp;
	s32 res;
	u8 keyvalue;
		
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	dev_key_open();

	while(1)
	{
		res = dev_key_read(&tmp, 1);
		if(1 == res)
		{
			if(tmp == DEV_KEY_PRESS)
			{
				dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 10, "	   press	", BLACK);
			}
			else if(tmp == DEV_KEY_REL)
			{
				dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 10, "	 release	  ", BLACK);
			}
		}

		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
		}
	}


	dev_key_close();
	return 0;
}


s32 test_camera(void)
{
	DevLcdNode *lcd;

	//dev_lcd_color_fill(emenulcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	//dev_lcd_put_string(emenulcd, FONT_SONGTI_1212, 1, 32, __FUNCTION__, BLACK);
	
	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open lcd err!");	
	}
	else
	{
		dev_camera_open();
		dev_camera_show(lcd);
			
	}
	wjq_wait_key(16);

	dev_camera_close();
	dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);
	
	return 0;
}
s32 test_rs485_rec(void)
{
	u8 keyvalue;
	u8 buf[20];
	u8 len;
	s32 res;
	
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);

	dev_rs485_open();

	while(1)
	{
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
		}

		len = dev_rs485_read(buf, sizeof(buf));
		if(len > 0)
		{
			buf[len] = 0;
			wjq_log(LOG_DEBUG,"%s", buf);
			memset(buf, 0, sizeof(buf));
		}

	}

	return 0;
}

s32 test_rs485_snd(void)
{
	u8 keyvalue;
	s32 res;
	
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	dev_rs485_open();

	while(1)
	{
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
			else if(keyvalue == 16)
			{
				res = dev_rs485_write("rs485 test\r\n", 14);
				wjq_log(LOG_DEBUG, "dev rs485 write:%d\r\n", res);
			}
		}
	}
	return 0;
}

s32 test_can_rec(void)
{
	wjq_test_showstr((char *)__FUNCTION__);
	return 0;
}
s32 test_can_snd(void)
{
	wjq_test_showstr((char *)__FUNCTION__);
	return 0;
}
s32 test_uart(void)
{
	u8 keyvalue;
	u8 buf[12];
    s32 len;
    s32 res;
	
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	
	while(1)
	{
		
	    len =  mcu_uart_read (MCU_UART_3, buf, 10);
		if(len > 0)
		{
	    	res = mcu_uart_write(MCU_UART_3, buf, len);
		}

		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
		}
	}
	return 0;
}
/*

	简单测试，串口发送AT命令，收到OK即可。

*/
s32 test_esp8266(void)
{
	u8 keyvalue;	
	u8 buf[32];
	u16 len;
	u32 timeout = 0;
	s32 res;
	
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
		
	dev_8266_open();

	while(1)
	{
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
		}
		
		Delay(500);
		dev_8266_write("at\r\n", 4);
		timeout = 0;
		while(1)
		{
			Delay(50);
			memset(buf, 0, sizeof(buf));
			len = dev_8266_read(buf, sizeof(buf));
			if(len != 0)
			{
				wjq_log(LOG_FUN, "%s", buf);
			}
			
			timeout++;
			if(timeout >= 100)
			{
				wjq_log(LOG_FUN, "timeout---\r\n");
				break;
			}
		}
	}

	return 0;
}



s32 wjq_test(void)
{
	wjq_test_showstr((char *)__FUNCTION__);
	return 0;
}

s32 test_spiflash_board(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	dev_spiflash_test_chipcheck("board_spiflash");
	wjq_wait_key(12);
	return 0;
}

s32 test_spiflash_core(void)
{
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	dev_spiflash_test_chiperase("core_spiflash");
	wjq_wait_key(12);
	return 0;
}

s32 test_touchkey(void)
{
	u8 tmp;
	s32 res;
	u8 keyvalue;
		
	dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
	dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 32, (char *)__FUNCTION__, BLACK);
	dev_touchkey_open();

	while(1)
	{
		res = dev_touchkey_read(&tmp, 1);
		if(1 == res)
		{
			if(tmp == DEV_TOUCHKEY_TOUCH)
			{
				wjq_log(LOG_FUN, "touch key test get a touch event!\r\n");
				dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 10, "      touch    ", BLACK);
			}
			else if(tmp == DEV_TOUCHKEY_RELEASE)
			{
				wjq_log(LOG_FUN, "touch key test get a release event!\r\n");
				dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 10, "      release    ", BLACK);
			}
		}

		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1)
		{
			if(keyvalue == 12)
			{
				break;
			}
		}
	}


	dev_touchkey_close();
	return 0;
}


s32 test_keypad(void)
{
	u8 dis_flag = 1;
	u8 keyvalue;
	s32 res;
	s32 esc_flag = 1;
	
	char testkeypad1[]="1 2 3 F1 ";
	char testkeypad2[]="4 5 6 DEL";
	char testkeypad3[]="7 8 9 ESC";
	char testkeypad4[]="* 0 # ENT";
	
	dev_keypad_open();

	while(1)
	{
		if(dis_flag == 1)
		{
			dev_lcd_color_fill(WJQTestLcd, 1, 1000, 1, 1000, WHITE);
			dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 1, (char *)__FUNCTION__, BLACK);
			dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 13, testkeypad1, BLACK);
			dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 26, testkeypad2, BLACK);
			dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 39, testkeypad3, BLACK);
			dev_lcd_put_string(WJQTestLcd, FONT_SONGTI_1212, 1, 52, testkeypad4, BLACK);
			dis_flag = 0;
		}
		
		res = dev_keypad_read(&keyvalue, 1);
		if(res == 1 && ((keyvalue & KEYPAD_PR_MASK) == 0))
		{
			dis_flag = 1;
			switch(keyvalue)
			{
				case 1:
					testkeypad1[0] = ' ';
					break;
				case 2:
					testkeypad1[2] = ' ';
					break;
				case 3:
					testkeypad1[4] = ' ';
					break;
				case 4:
					testkeypad1[6] = ' ';
					testkeypad1[7] = ' ';
					testkeypad1[8] = ' ';
					break;
				case 5:
					testkeypad2[0] = ' ';
					break;
				case 6:
					testkeypad2[2] = ' ';
					break;
				case 7:
					testkeypad2[4] = ' ';
					break;
				case 8:
					testkeypad2[6] = ' ';
					testkeypad2[7] = ' ';
					testkeypad2[8] = ' ';
					break;
				case 9:
					testkeypad3[0] = ' ';
					break;
				case 10:
					testkeypad3[2] = ' ';
					break;
				case 11:
					testkeypad3[4] = ' ';
					break;
				case 12:
					if(esc_flag == 1)
					{
						esc_flag = 0;
						testkeypad3[6] = ' ';
						testkeypad3[7] = ' ';
						testkeypad3[8] = ' ';
					}
					else
					{
						return 0;
					}
					break;

				case 13:
					testkeypad4[0] = ' ';
					break;
				case 14:
					testkeypad4[2] = ' ';
					break;
				case 15:
					testkeypad4[4] = ' ';
					break;
				case 16:
					testkeypad4[6] = ' ';
					testkeypad4[7] = ' ';
					testkeypad4[8] = ' ';
					break;
			} 
		}
	}

}


const MENU WJQTestList[]=
{
	MENU_L_0,//菜单等级
	"测试程序",//中文
	"test",	//英文
	MENU_TYPE_KEY_2COL,//菜单类型
	NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"LCD",//中文
		"LCD",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"VSPI OLED",//中文
			"VSPI OLED",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_vspi_oled_display,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"I2C OLED",//中文
			"I2C OLED",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_i2c_oled_display,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"SPI COG",//中文
			"SPI COG",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_spi_cog_display,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"tft",//中文
			"tft",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_tft_display,//菜单函数，功能菜单才会执行，有子菜单的不会执行
	
			MENU_L_2,//菜单等级
			"图片测试",//中文
			"test BMP",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_lcd_pic,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"字库测试",//中文
			"test Font",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_lcd_font,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"声音",//中文
		"sound",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"蜂鸣器",//中文
			"buzzer",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_sound_buzzer,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"DAC音乐",//中文
			"DAC music",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_sound_dac,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"收音",//中文
			"FM",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_sound_fm,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"I2S音乐",//中文
			"I2S Music",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_sound_wm8978,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"录音",//中文
			"rec",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_sound_rec,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"触摸屏",//中文
		"tp",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"校准",//中文
			"calibrate",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_tp_calibrate,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"测试",//中文
			"test",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_tp_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
		MENU_L_1,//菜单等级
		"按键",//中文
		"KEY",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"核心板按键",//中文
			"core KEY",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_key,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			/*触摸按键*/
			MENU_L_2,//菜单等级
			"触摸按键",//中文
			"touch key",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_touchkey,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			/*可以不测，本来测试程序就要使用矩阵按键*/
			MENU_L_2,//菜单等级
			"矩阵按键",//中文
			"keypad",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_keypad,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
		MENU_L_1,//菜单等级
		"摄像",//中文
		"camera",	//英文
		MENU_TYPE_FUN,//菜单类型
		test_camera,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
		MENU_L_1,//菜单等级
		"SPI FLASH",//中文
		"SPI FLASH",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"核心板FLASH",//中文
			"core FLASH",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_spiflash_core,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"底板 FLASH",//中文
			"board FLASH",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_spiflash_board,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
		MENU_L_1,//菜单等级
		"通信",//中文
		"通信",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"485",//中文
			"485",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
				MENU_L_3,//菜单等级
				"485 接收",//中文
				"485 rec",	//英文
				MENU_TYPE_FUN,//菜单类型
				test_rs485_rec,//菜单函数，功能菜单才会执行，有子菜单的不会执行
				MENU_L_3,//菜单等级
				"485 发送",//中文
				"485 snd",	//英文
				MENU_TYPE_FUN,//菜单类型
				test_rs485_snd,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"CAN",//中文
			"CAN",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
				MENU_L_3,//菜单等级
				"CAN 接收",//中文
				"CAN rec",	//英文
				MENU_TYPE_FUN,//菜单类型
				test_can_rec,//菜单函数，功能菜单才会执行，有子菜单的不会执行
				
				MENU_L_3,//菜单等级
				"CAN 发送",//中文
				"CAN snd",	//英文
				MENU_TYPE_FUN,//菜单类型
				test_can_snd,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
		
		
			MENU_L_2,//菜单等级
			"串口",//中文
			"uart",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_uart,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"网络",//中文
			"eth",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"OTG",//中文
			"OTG",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
				MENU_L_3,//菜单等级
				"HOST",//中文
				"HOST",	//英文
				MENU_TYPE_LIST,//菜单类型
				NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
				MENU_L_3,//菜单等级
				"Device",//中文
				"Device",	//英文
				MENU_TYPE_LIST,//菜单类型
				NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		MENU_L_1,//菜单等级
		"模块",//中文
		"模块",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
			/*外扩接口测试*/
			MENU_L_2,//菜单等级
			"RF24L01",//中文
			"RF24L01",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"MPU6050",//中文
			"MPU6050",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			/*用串口外扩8266模块*/
			MENU_L_2,//菜单等级
			"wifi",//中文
			"wifi",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_esp8266,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		

		MENU_L_1,//菜单等级
		"test",//中文
		"test",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
		MENU_L_1,//菜单等级
		"test1",//中文
		"test1",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test2",//中文
		"test2",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test3",//中文
		"test3",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test4",//中文
		"test4",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test5",//中文
		"test5",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test6",//中文
		"test6",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test7",//中文
		"test7",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test8",//中文
		"test8",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test9",//中文
		"test9",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"test10",//中文
		"test10",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
			MENU_L_2,//菜单等级
			"t10-1",//中文
			"t10-1",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-2",//中文
			"t10-2",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-3",//中文
			"t10-3",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-4",//中文
			"t10-4",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-5",//中文
			"t10-5",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-6",//中文
			"t10-6",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-7",//中文
			"t10-7",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-8",//中文
			"t10-8",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-9",//中文
			"t10-9",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-10",//中文
			"t10-10",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-11",//中文
			"t10-11",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"t10-12",//中文
			"t10-12",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
	
	/*最后的菜单是结束菜单，无意义*/			
	MENU_L_0,//菜单等级
	"END",//中文
	"END",	//英文
	MENU_TYPE_NULL,//菜单类型
	NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
};


void wujique_stm407_test(void)
{
	wjq_log(LOG_DEBUG,"run app\r\n");

	
	WJQTestLcd = dev_lcd_open("spitftlcd");
	if(WJQTestLcd == NULL)
	{
		wjq_log(LOG_DEBUG, "open oled lcd err\r\n");
	}

	dev_key_open();
	dev_keypad_open();

	emenu_run(WJQTestLcd, (MENU *)&WJQTestList[0], sizeof(WJQTestList)/sizeof(MENU), FONT_SONGTI_1616, 2);	
	while(1)
	{
	}
}

#define Wujique407_TASK_STK_SIZE (4096)
#define Wujique407_TASK_PRIO	1
TaskHandle_t  Wujique407TaskHandle;


s32 wujique_407test_init(void)
{
	xTaskCreate(	(TaskFunction_t) wujique_stm407_test,
					(const char *)"wujique 407 test task",		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
					(const configSTACK_DEPTH_TYPE) Wujique407_TASK_STK_SIZE,
					(void *) NULL,
					(UBaseType_t) Wujique407_TASK_PRIO,
					(TaskHandle_t *) &Wujique407TaskHandle );	
					
	return 0;
}

/*

生产测试

*/

s32 test_tft_lcd(void)
{
	DevLcdNode *lcd;
	u8 step = 0;
	u8 dis = 1;
	
	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open tft lcd err!");	
	}
	else
	{
		while(1)
		{
			if(dis == 1)
			{
				dis = 0;
				switch(step)
				{
					case 0:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, YELLOW);
						break;
					
					case 1:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, RED);
						break;
					
					case 2:
						dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);
						dev_lcd_put_string(lcd, FONT_SONGTI_1616, 20, 20, "abc屋脊雀工作室ADC", RED);
						break;

					case 3:
						dev_lcd_show_bmp(lcd, 1, 1, 320, 240, "1:/pic/女人单色.bmp");
						break;
					
					case 4:
						dev_lcd_show_bmp(lcd, 1, 1, 320, 240, "1:/pic/女人24位.bmp");//真彩色
						break;
					case 5:
						dev_lcd_backlight(lcd, 0);
						break;
					case 6:
						dev_lcd_backlight(lcd, 1);
						break;		
					default:
						break;
				}
				step++;
				if(step >= 7)
					step = 0;
			}
			u8 keyvalue;
			s32 res;
			
			res = dev_keypad_read(&keyvalue, 1);
			if(res == 1)
			{
				if(keyvalue == 16)
				{
					dis = 1;
				}
				else if(keyvalue == 12)
				{
					break;
				}
			}
		}

	}
	dev_lcd_close(lcd);
	return 0;
}

s32 test_cog_lcd(void)
{
	DevLcdNode *lcd;
	u8 step = 0;
	u8 dis = 1;
	
	lcd = dev_lcd_open("spicoglcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open cog lcd err!");	
		while(1);
	}

	while(1)
	{
		dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLACK);
		wjq_wait_key(16);
		dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, WHITE);
		wjq_wait_key(16);
		dev_lcd_put_string(lcd, FONT_SONGTI_1212, 1, 32, "cog LCD测试程序", BLACK);
		wjq_wait_key(16);
		dev_lcd_backlight(lcd, 0);
		wjq_wait_key(16);
		dev_lcd_backlight(lcd, 1);
		wjq_wait_key(16);
	}
	
	return 0;
}

s32 test_tft_tp(void)
{
	DevLcdNode *lcd;


	lcd = dev_lcd_open("tftlcd");
	if(lcd == NULL)
	{
		wjq_test_showstr("open lcd err!");	
	}
	else
	{
		dev_lcd_backlight(lcd, 1);
		
		dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);
		dev_lcd_setdir(lcd, H_LCD, L2R_U2D);
		dev_touchscreen_open();
		ts_calibrate(lcd);
		dev_touchscreen_close();
	}
	
	dev_lcd_color_fill(lcd, 1, 1000, 1, 1000, BLUE);

	{
		dev_touchscreen_open();	
	
		struct tsdev *ts;
		ts = ts_open_module();

		struct ts_sample samp[10];
		int ret;
		u8 i =0;	
		while(1)
		{
			ret = ts_read(ts, samp, 10);
			if(ret != 0)
			{
				//uart_printf("pre:%d, x:%d, y:%d\r\n", samp[0].pressure, samp[0].x, samp[0].y);
						
				i = 0;
				
				while(1)
				{
					if(i>= ret)
						break;
					
					if(samp[i].pressure != 0 )
					{
						//uart_printf("pre:%d, x:%d, y:%d\r\n", samp.pressure, samp.x, samp.y);
						dev_lcd_drawpoint(lcd, samp[i].x, samp[i].y, 0xF800); 
					}
					i++;
				}
			}

		}

		dev_touchscreen_close();
	}
	return 0;
}


