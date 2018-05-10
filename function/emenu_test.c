
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "emenu.h"
#include "dev_lcd.h"
#include "dev_keypad.h"
#include "font.h"
#include "mcu_uart.h"

DevLcdNode *emenulcd;

s32 test_wait_key(u8 key)
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

s32 emenu_test_fun(char *s)

{
	wjq_log(LOG_DEBUG, "test:%s", s);
	
	dev_lcd_color_fill(emenulcd, 1, 1000, 1, 1000, WHITE);
	/*顶行居中显示父菜单*/
	dev_lcd_put_string(emenulcd, FONT_SONGTI_1212, 1, 32, s, BLACK);
	
	test_wait_key(0);
	
	return 0;
}	

s32 test_test(void)
{
	emenu_test_fun((char *)__FUNCTION__);
	return 0;
}


const MENU EMenuListTest[]=
{
	MENU_L_0,//菜单等级
	"测试程序",//中文
	"test",	//英文
	MENU_TYPE_LIST,//菜单类型
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
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"I2C OLED",//中文
			"I2C OLED",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"SPI COG",//中文
			"SPI COG",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"tft",//中文
			"tft",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
	
			MENU_L_2,//菜单等级
			"图片测试",//中文
			"test BMP",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"字库测试",//中文
			"test Font",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"声音",//中文
		"sound",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"蜂鸣器",//中文
			"buzzer",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"DAC音乐",//中文
			"DAC music",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"收音",//中文
			"FM",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"I2S音乐",//中文
			"I2S Music",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"录音",//中文
			"rec",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"触摸屏",//中文
		"tp",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"校准",//中文
			"calibrate",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

			MENU_L_2,//菜单等级
			"测试",//中文
			"test",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
		MENU_L_1,//菜单等级
		"按键",//中文
		"KEY",	//英文
		MENU_TYPE_FUN,//菜单类型
		test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"摄像",//中文
		"camera",	//英文
		MENU_TYPE_FUN,//菜单类型
		test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"485",//中文
		"485",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"485 接收",//中文
			"485 rec",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"485 发送",//中文
			"485 snd",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
		MENU_L_1,//菜单等级
		"CAN",//中文
		"CAN",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"CAN 接收",//中文
			"CAN rec",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"CAN 发送",//中文
			"CAN snd",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
		MENU_L_1,//菜单等级
		"SPI FLASH",//中文
		"SPI FLASH",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"核心板FLASH",//中文
			"core FLASH",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			
			MENU_L_2,//菜单等级
			"底板 FLASH",//中文
			"board FLASH",	//英文
			MENU_TYPE_FUN,//菜单类型
			test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
		MENU_L_1,//菜单等级
		"串口",//中文
		"uart",	//英文
		MENU_TYPE_FUN,//菜单类型
		test_test,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"网络",//中文
		"eth",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"OTG",//中文
		"OTG",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"HOST",//中文
			"HOST",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
			MENU_L_2,//菜单等级
			"Device",//中文
			"Device",	//英文
			MENU_TYPE_LIST,//菜单类型
			NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		/*外扩接口测试*/
		MENU_L_1,//菜单等级
		"RF24L01",//中文
		"RF24L01",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		MENU_L_1,//菜单等级
		"MPU6050",//中文
		"MPU6050",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行

		/*可以不测，本来测试程序就要使用矩阵按键*/
		MENU_L_1,//菜单等级
		"矩阵按键",//中文
		"keypad",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
		/*用串口外扩8266模块*/
		MENU_L_1,//菜单等级
		"wifi",//中文
		"wifi",	//英文
		MENU_TYPE_LIST,//菜单类型
		NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
		
	/*最后的菜单是结束菜单，无意义*/			
	MENU_L_0,//菜单等级
	"END",//中文
	"END",	//英文
	MENU_TYPE_NULL,//菜单类型
	NULL,//菜单函数，功能菜单才会执行，有子菜单的不会执行
};
	



s32 emenu_test(DevLcdNode *lcd)
{
	emenulcd = lcd;
	if(emenulcd == NULL)
	{
		wjq_log(LOG_DEBUG, "open lcd err\r\n");
	}
	emenu_run(emenulcd, (MENU *)&EMenuListTest[0], sizeof(EMenuListTest)/sizeof(MENU));	
	
	return 0;
}


