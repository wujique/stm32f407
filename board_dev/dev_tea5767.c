/**
 * @file            dev_tea5767.c
 * @brief           收音机芯片 tea5765 驱动
 * @author          test
 * @date            2017年10月31日 星期二
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:      2017年10月31日 星期二
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

extern void Delay(__IO uint32_t nTime);



#define DEV_TEA5767_DEBUG

#ifdef DEV_TEA5767_DEBUG
#define TEA5767_DEBUG	wjq_log 
#else
#define TEA5767_DEBUG(a, ...)
#endif

#define DEV_TEA5767_I2CBUS "VI2C1"
#define DEV_TEA5767_I2CC_ADDR 0x60//110 0000b, 7位地址模式，在I2C驱动中会进行左移

#define TEA5767_MAX_FREQ 108000
#define TEA5767_MIN_FREQ 87500

unsigned int Tea5767MaxPll=0x339b;    //108MHz时的pll,
unsigned int Tea5767MinPll=0x299d;    //87.5MHz时的pll.

unsigned int Tea5767Fre;
unsigned int Tea5767Pll;

//tea5767进行I2C通信的时候，每次都是将5个字节一起读出来。
u8 tea5767_readbuf[5];
//初始化要写入TEA5767的数据，见<TEA5767HN低功耗立体声收音机接收器>
u8 tea5767_writebuf[5]={0x2a,0xb6,0x51,0x11,0x40};
u8 tea5767_initbuf[5]={0xaa,0xb6,0x51,0x11,0x40};//静音，上电初始化


static s32 dev_tea5767_readreg(u8* data)
{
	DevI2cNode *dev;
	dev = mcu_i2c_open(DEV_TEA5767_I2CBUS);
	mcu_i2c_transfer(dev, DEV_TEA5767_I2CC_ADDR, MCU_I2C_MODE_R, tea5767_readbuf, 5);
	mcu_i2c_close(dev);	
	return 0;
}
static s32 dev_tea5767_writereg(u8* data)
{
	DevI2cNode *dev;

	dev = mcu_i2c_open(DEV_TEA5767_I2CBUS);
    mcu_i2c_transfer(dev, DEV_TEA5767_I2CC_ADDR, MCU_I2C_MODE_W, tea5767_writebuf, 5);
	mcu_i2c_close(dev);
	return 0;	
}

/**
 *@brief:      dev_tea5767_fre2pll
 *@details:    将频率值转为PLL
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static u32 dev_tea5767_fre2pll(u8 hlsi, u32 fre)
{
    if (hlsi)
    {
        //return (((fre+225)*4000)/32768);
		return (u32)((float)((fre+225))/(float)8.192);
    }
	else
	{
        //return (((fre-225)*4000)/32768);
		return (u32)((float)((fre-225))/(float)8.192);
	}
}
/**
 *@brief:      dev_tea5767_pll2fre
 *@details:    将PLL转换为频率
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static u32 dev_tea5767_pll2fre(u8 hlsi, u32 pll)
{  
    if (hlsi)
        return (pll*8192/1000 - 225);
    else
        return (pll*8192/1000 + 225);

}
/**
 *@brief:      dev_tea5767_getfre
 *@details:    从读取到的状态值中计算FM频率
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static void dev_tea5767_getfre(void)
{
    unsigned char temp_l,temp_h;
    
    Tea5767Pll=0;
	
    temp_l =tea5767_readbuf[1];
    temp_h =tea5767_readbuf[0]&0x3f;
	
    Tea5767Pll=temp_h*256 + temp_l;
    
    Tea5767Fre = dev_tea5767_pll2fre(1, Tea5767Pll);
}

/**
 *@brief:      dev_tea5767_setfre
 *@details:    直接设置tea5767到指定频率
 *@param[in]   unsigned long fre  
 *@param[out]  无
 *@retval:     
 */
void dev_tea5767_setfre(unsigned long fre)
{

    if(fre>TEA5767_MAX_FREQ)
        return;
    if(fre<TEA5767_MIN_FREQ)  
        return;

    Tea5767Fre = fre;            
    Tea5767Pll = dev_tea5767_fre2pll(tea5767_writebuf[2]&0x10, Tea5767Fre);
    
    tea5767_writebuf[0]=Tea5767Pll/256;
    tea5767_writebuf[1]=Tea5767Pll%256;

	dev_tea5767_writereg(tea5767_writebuf);
}

/**
 *@brief:      dev_tea5767_set_ssl
 *@details:    设置搜索停止级别
 *@param[in]   u8 level  0 允许搜索，1 低级别，2 中级别，3 高级别
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_set_ssl(u8 level)
{
	u8 tmp;
	
	if(level > 3)
		return -1;
	
	tmp = tea5767_writebuf[2];
	tea5767_writebuf[2] = ((tmp&0x9f)|(level<<5));
	TEA5767_DEBUG(LOG_DEBUG, "---%02x\r\n", tea5767_writebuf[2]);
	
	return 0;
}
/**
 *@brief:      dev_tea5767_auto_search
 *@details:    芯片自动搜台
 *@param[in]   u8 mode  mode=1,频率增加搜台; mode=0:频率减小搜台
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_auto_search(u8 mode)
{
	u8 if_counter = 0;
	u32 flag = 0;
	u8 adc;

	dev_tea5767_readreg(tea5767_readbuf);
	dev_tea5767_getfre();

    if(mode)
    {
        tea5767_writebuf[2] |=0x80;

    }
    else
    {
        tea5767_writebuf[2] &=0x7f; 
    }          
	
	while(1)
	{
		flag = 0;
		
		if(mode)
    	{
        	Tea5767Fre += 20;

    	}
    	else
    	{
        	Tea5767Fre -= 20;
    	} 
		TEA5767_DEBUG(LOG_DEBUG, "Tea5767Fre:%d\r\n", Tea5767Fre);
	    Tea5767Pll = dev_tea5767_fre2pll(1, Tea5767Fre);

		tea5767_writebuf[0]=Tea5767Pll/256+0x40;//加0x40是将SM置为1 为自动搜索模式
		tea5767_writebuf[1]=Tea5767Pll%256;   

		dev_tea5767_writereg(tea5767_writebuf);
	    while(1)     
	    {
	        dev_tea5767_readreg(tea5767_readbuf);
	        if(0x80 == (tea5767_readbuf[0]&0x80))//搜台成功标志
	        {
				dev_tea5767_getfre();
				if_counter = tea5767_readbuf[2]&0x7f;
				if((0x31 < if_counter) && (0x3e > if_counter))
				{         
					adc = (tea5767_readbuf[3]>>4);
					TEA5767_DEBUG(LOG_DEBUG, "fre:%d, adc:%d\r\n", Tea5767Fre, adc);
					//比较leve 电平确认是否收到台 实际测试使用此数 不会漏台
					if( adc >= 8)
					{
						TEA5767_DEBUG(LOG_DEBUG, "fre: \r\n", Tea5767Fre);
						return 0;
					}
				}
	            flag = 1;
	        }

			if(0x60 == (tea5767_readbuf[0]&0x60))//到达频率极限
			{
				TEA5767_DEBUG(LOG_DEBUG, "tea5767  search fail!\r\n");
				
				if(mode)
					Tea5767Fre = TEA5767_MIN_FREQ;
				else
					Tea5767Fre = TEA5767_MAX_FREQ;
				
				flag = 1;
			}

			if(flag == 1)
			{
				break;
			}
	    }  

	}	
} 
/**
 *@brief:      dev_tea5767_search
 *@details:    程序搜台 步进100KHZ
 *@param[in]   u8 mode  
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_search(u8 mode)
{
	u8 adc;

	dev_tea5767_readreg(tea5767_readbuf);

	dev_tea5767_getfre();
    TEA5767_DEBUG(LOG_DEBUG, "Tea5767Fre:%d\r\n", Tea5767Fre);

	if(mode)
	{
	    Tea5767Fre += 100;
		if(Tea5767Fre > TEA5767_MAX_FREQ)
			Tea5767Fre = TEA5767_MIN_FREQ;
	}
	else
	{
	    Tea5767Fre -= 100;
		if(Tea5767Fre < TEA5767_MIN_FREQ)
			Tea5767Fre = TEA5767_MAX_FREQ;
	}
	
	while(1)
	{
		
		dev_tea5767_setfre(Tea5767Fre);
		
		Delay(2);
		dev_tea5767_readreg(tea5767_readbuf);
		adc = (tea5767_readbuf[3]>>4);
		
		if( adc >=7)
		{
			TEA5767_DEBUG(LOG_DEBUG, "fre:%d, adc:%d\r\n", Tea5767Fre, adc);
			return 0;
		}

		if(mode)
		{
		    Tea5767Fre += 20;
			if(Tea5767Fre > TEA5767_MAX_FREQ)
				Tea5767Fre = TEA5767_MIN_FREQ;
		}
		else
		{
		    Tea5767Fre -= 20;
			if(Tea5767Fre < TEA5767_MIN_FREQ)
				Tea5767Fre = TEA5767_MAX_FREQ;
		}
	}

}
/**
 *@brief:      dev_tea5767_init
 *@details:    初始化TEA5767设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_init(void)
{

	dev_tea5767_writereg(tea5767_initbuf);
	return 0;
}
/**
 *@brief:      dev_tea5767_open
 *@details:    打开5767设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_open(void)
{

	dev_tea5767_writereg(tea5767_writebuf);
	return 0;
}
/**
 *@brief:      dev_tea5767_close
 *@details:    关闭5767设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_close(void)
{
	dev_tea5767_writereg(tea5767_initbuf);
	return 0;
}

/**
 *@brief:      dev_tea5767_test
 *@details:       测试TEA5767设备
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_tea5767_test(void)
{
    wjq_log(LOG_INFO, "\r\n------tea5767 test--------\r\n");

    dev_tea5767_open();
	dev_tea5767_setfre(89800);
	while(1)
	{
		Delay(5000);
		dev_tea5767_auto_search(1);
	}
    //dev_tea5767_close();
    
    //return 0;
}


