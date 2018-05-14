/**
 * @file            dev_htu21d.c
 * @brief           
 * @author          test
 * @date            2017年10月31日 星期二
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:      2018年5月31日 星期二
 *   作    者:         屋脊雀工作室
 *   修改内容:      创建文件
     	1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_i2c.h"
#include "dev_htu21d.h"

#define DEV_HTU21D_I2CBUS "VI2C1"
/* HTU21D i2c地址，不包含最低位读写位 */
#define HTU21D_I2C_ADDR   0X40
/*
所谓的主机保持，就是在转换过程中，HTU21D控制住SCK，
那么这个I2C就不能控制其他设备了。
因此通常用不保持模式，通过查询获转换结果。
*/
#define HTU21D_CMD_TTM_H 	0XE3//测温度，主机保持模式
#define HTU21D_CMD_THM_H 	0XE5//测湿度，主机保持模式
#define HTU21D_CMD_TTM_NH 	0XF3//测温度，主机不保持
#define HTU21D_CMD_THM_NH 	0XF5//测湿度不保持
#define HTU21D_CMD_WREG 	0XE6//写用户寄存器
#define HTU21D_CMD_RREG 	0XE7//读用户寄存器
#define HTU21D_CMD_RESET 	0XFE//软复位

s32 dev_htu21d_open(void)
{
	
}

s32 dev_htu21d_init(void)
{
	DevI2cNode *dev;
	u8 tmp[2];

	tmp[0] = HTU21D_CMD_RESET;
	
	dev = mcu_i2c_open(DEV_HTU21D_I2CBUS);
    mcu_i2c_transfer(dev, HTU21D_I2C_ADDR, MCU_I2C_MODE_W, tmp, 1);
	mcu_i2c_close(dev);
	return 0;	
}

s32 dev_htu21d_colse(void)
{

}

s32 dev_htu21d_read(u8 type)
{
	DevI2cNode *dev;
	u8 tmp[3];
	s32 res;
	u32 data;
	s32 temp;

	if(type == HTU21D_READ_TEMP)
		tmp[0] = HTU21D_CMD_TTM_NH;
	else if(type == HTU21D_READ_HUMI)
		tmp[0] = HTU21D_CMD_THM_NH;
	else
		return -1;
	
	dev = mcu_i2c_open(DEV_HTU21D_I2CBUS);
	wjq_log(LOG_INFO, "HTU21D_CMD_TTM_NH\r\n");	
    mcu_i2c_transfer(dev, HTU21D_I2C_ADDR, MCU_I2C_MODE_W, tmp, 1);

	wjq_log(LOG_INFO, "read data \r\n");
	while(1)
	{
		Delay(5);
		res = mcu_i2c_transfer(dev, HTU21D_I2C_ADDR, MCU_I2C_MODE_R, tmp, 3);
		if(res == 0)
			break;		
	}
	mcu_i2c_close(dev);
	/**/
	wjq_log(LOG_INFO, "-%02x %02x %02x\r\n", tmp[0], tmp[1], tmp[2]);
	//计算CRC校验
	//计算温湿度
	data = tmp[0]*256+((tmp[1]>>2)&0x3f);
	if((tmp[1] & 0x02) == 0x00)//温度测量
	{
		temp = ((17572*data)>>16)-4685;
		wjq_log(LOG_INFO, "data: %d, temp:%d\r\n", data, temp);
	}
	else//湿度测量
	{
		temp = ((125*data)>>16)-6;
		wjq_log(LOG_INFO, "data: %d, humi:%d\r\n", data, temp);	
	}
	
	return temp;
}



