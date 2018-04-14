/**
 * @file            mcu_i2c.c
 * @brief           IO模拟I2C
 * @author          test
 * @date            2017年10月30日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:      2017年10月30日 星期一
 *   作    者:      屋脊雀工作室
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


#define MCU_I2C_PORT GPIOD
#define MCU_I2C_SCL GPIO_Pin_6
#define MCU_I2C_SDA GPIO_Pin_7

#define MCU_I2C_TIMEOUT 250

/**
 *@brief:      mcu_i2c_delay
 *@details:    I2C信号延时函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_delay(void)
{
    //Delay(1);//延时，I2C时钟
    u32 i = 70;

    for(;i>0;i--);
}

/**
 *@brief:      mcu_i2c_sda_input
 *@details:    将I2C sda IO设置为输入
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_i2c_sda_input(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = MCU_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(MCU_I2C_PORT, &GPIO_InitStructure);//初始化   
}
/**
 *@brief:      mcu_i2c_sda_output
 *@details:       将I2C sda IO设置为输出
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */
void mcu_i2c_sda_output(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = MCU_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(MCU_I2C_PORT, &GPIO_InitStructure);//初始化

}
/**
 *@brief:      mcu_i2c_readsda
 *@details:    读SDA数据
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_readsda(void)
{
    if(Bit_SET == GPIO_ReadInputDataBit(MCU_I2C_PORT, MCU_I2C_SDA))
        return 1;
    else
        return 0;
}
/**
 *@brief:      mcu_i2c_sda
 *@details:       SDA输出高低电平
 *@param[in]  u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_sda(u8 sta)
{
    if(sta == 1)
    {
        GPIO_SetBits(MCU_I2C_PORT,MCU_I2C_SDA);    
    }
    else if(sta == 0)
    {
        GPIO_ResetBits(MCU_I2C_PORT, MCU_I2C_SDA);
    }
    else
    {
    
    }
}

/**
 *@brief:      mcu_i2c_scl
 *@details:    时钟SCL输出高低电平
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_scl(u8 sta)
{
    if(sta == 1)
    {
        GPIO_SetBits(MCU_I2C_PORT, MCU_I2C_SCL);    
    }
    else if(sta == 0)
    {
        GPIO_ResetBits(MCU_I2C_PORT, MCU_I2C_SCL);
    }
    else
    {
    
    }    
}
/**
 *@brief:      mcu_i2c_start
 *@details:    发送start时序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_start(void)
{
    mcu_i2c_sda_output();
    
    mcu_i2c_sda(1);  
    mcu_i2c_scl(1);

    mcu_i2c_delay();
    mcu_i2c_sda(0);

    mcu_i2c_delay();
    mcu_i2c_scl(0);
}
/**
 *@brief:      mcu_i2c_stop
 *@details:    发送I2C STOP时序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_stop(void)
{
    mcu_i2c_sda_output();

    mcu_i2c_scl(0);
    mcu_i2c_sda(0);   
    mcu_i2c_delay();
    
    mcu_i2c_scl(1);
    mcu_i2c_delay();
    
    mcu_i2c_sda(1);
    mcu_i2c_delay();
}

/**
 *@brief:      mcu_i2c_wait_ack
 *@details:       等待ACK信号
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_wait_ack(void)
{
    u8 time_out = 0;
    
    //sda转输入
    mcu_i2c_sda_input();
    mcu_i2c_sda(1);
    mcu_i2c_delay();
    
    mcu_i2c_scl(1);
    mcu_i2c_delay();
    
    while(1)
    {
        time_out++;
        if(time_out > MCU_I2C_TIMEOUT)
        {
            mcu_i2c_stop();
            wjq_log(LOG_INFO, "i2c:wait ack time out!\r\n");
            return 1;
        }

        if(0 == mcu_i2c_readsda())
        {   
            break;
        }
    }
    
    mcu_i2c_scl(0);
    
    return 0;
}
/**
 *@brief:      mcu_i2c_ack
 *@details:       发送ACK信号
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_ack(void)
{
    mcu_i2c_scl(0);
    mcu_i2c_sda_output();
    
    mcu_i2c_sda(0);
    mcu_i2c_delay();
    
    mcu_i2c_scl(1);
    mcu_i2c_delay();
    
    mcu_i2c_scl(0);
}
/**
 *@brief:      mcu_i2c_writebyte
 *@details:       I2C总线写一个字节数据
 *@param[in]  u8 data  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_writebyte(u8 data)
{
    u8 i = 0;

    mcu_i2c_sda_output();

    mcu_i2c_scl(0);
    
    while(i<8)
    {
    
        if((0x80 & data) == 0x80)
        {
            mcu_i2c_sda(1);   
        }
        else
        {
            mcu_i2c_sda(0);
        }
        
        mcu_i2c_delay();

        mcu_i2c_scl(1);
        mcu_i2c_delay();
        
        mcu_i2c_scl(0);
        mcu_i2c_delay();
        
        data = data <<1;
        i++;
    }
		return 0;
}
/**
 *@brief:      mcu_i2c_readbyte
 *@details:       I2C总线读一个字节数据
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static u8 mcu_i2c_readbyte(void)
{
    u8 i = 0;
    u8 data = 0;

    mcu_i2c_sda_input();
    
    while(i<8)
    {
        mcu_i2c_scl(0);
        mcu_i2c_delay();
        
        mcu_i2c_scl(1);

        data = (data <<1);

        if(1 == mcu_i2c_readsda())
        {
            data =data|0x01;    
        }
        else
        {
            data = data & 0xfe;
        }

        mcu_i2c_delay();
        
        i++;
    }

    return data;
}

/**
 *@brief:      mcu_i2c_transfer
 *@details:    中间无重新开始位的传输流程
 *@param[in]   u8 addr   
               u8 rw    0 写，1 读    
               u8* data  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_i2c_transfer(u8 addr, u8 rw, u8* data, s32 datalen)
{
    s32 i;
    u8 ch;

    #if 0//测试IO口是否连通
    while(1)
    {
        uart_printf("test \r\n");
        mcu_i2c_scl(1);
        mcu_i2c_sda(1); 
        Delay(5);
        mcu_i2c_scl(0);
        mcu_i2c_sda(0); 
        Delay(5);
    }   
    #endif
    
    //发送起始
    mcu_i2c_start();
    //发送地址+读写标志
    //处理ADDR
    if(rw == MCU_I2C_MODE_W)
    {
        addr = ((addr<<1)&0xfe);
        //uart_printf("write\r\n");
    }
    else
    {
        addr = ((addr<<1)|0x01);
        //uart_printf("read\r\n");
    }
    
    mcu_i2c_writebyte(addr);
    mcu_i2c_wait_ack();

    i = 0;
    while(i < datalen)
    {
        //数据传输
        if(rw == MCU_I2C_MODE_W)//写
        {
            ch = *(data+i);
            mcu_i2c_writebyte(ch);
            mcu_i2c_wait_ack();
            
        }
        else if(rw == MCU_I2C_MODE_R)//读
        {
            ch = mcu_i2c_readbyte();  
            mcu_i2c_ack();
            *(data+i) = ch;
        }
        i++;
    }

    //发送结束
    mcu_i2c_stop();
    return 0;
}

/**
 *@brief:      mcu_i2c_init
 *@details:    初始化I2C接口
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_i2c_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = MCU_I2C_SCL | MCU_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(MCU_I2C_PORT, &GPIO_InitStructure);//初始化

    //初始化IO口状态
    mcu_i2c_scl(1);
    mcu_i2c_sda(1);
	
	return 0;
}


