/**
 * @file            dev_rf24l01.c
 * @brief           RF24L01驱动
 * @author          wujique
 * @date            2018年3月12日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年3月12日 星期一
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#include "stm32f4xx.h"
#include "mcu_spi.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"

/*
	接法：
	RF24L01接在外扩的SPI3上，也可以接到左边用IO口模拟SPI
	接口跟OLED一样
	接口：
	除SPI口外，还有IRQ跟CE管脚
	IRQ，低电平中断，具体中断原因需要查询寄存器
	CE，控制RF是否工作，高电平工作，发送接收都需要拉高，配置芯片时拉低关掉RF。
	功能：
	0 只有一个发送通道
	1 有6个接收通道，6个通道频率一样，地址可以设置不同。通道0/通道1地址40BIT，其他通道前32BIT跟通道1相同，低8bit不一样。
	2 发送模式，通道0用于确认应答，因此通道0地址要设置跟发送目的地址一样。
	3 芯片会自动应答、重发，自动应答的延时和重发次数是可编程的。
	4 每次开始SPI通信，芯片都会返回状态。

	本驱动对NRF24L01的使用：
	1 接收通道0默认做为ACK接收通道，不提供给APP使用。
	2 提供1-5通道做为接收通道，APP可设置地址。
	3 提供读写接口给APP，读接口读的是缓冲，写则直接操作发送。
*/

#define RF24L01_SPI "SPI3_CH3"

#define RF24L01_CE_PORT	GPIOG
#define RF24L01_CE_PIN	GPIO_Pin_7
#define RF24L01_IRQ_PORT GPIO_Pin_4
#define RF24L01_IRQ_PIN GPIOG


#define NRF_READ_REG 0x00 	//读配置寄存器,低5位为寄存器地址
#define NRF_WRITE_REG 0x20 	//写配置寄存器,低5位为寄存器地址
#define NRF_RD_RX_PLOAD 0x61 //读RX有效数据,1~32字节
#define NRF_WR_TX_PLOAD 0xA0 //写TX有效数据,1~32字节
#define NRF_FLUSH_TX 0xE1 	//清除TX FIFO寄存器.发射模式下用
#define NRF_FLUSH_RX 0xE2 	//清除RX FIFO寄存器.接收模式下用
#define NRF_REUSE_TX_PL 0xE3 //重新使用上一包数据,CE为高,数据包被不断发送.
#define NRF_NOP 0xFF //空操作,可以用来读状态寄存器
/*
	寄存器
*/
#define NRF_REG_CONFIG 0x00 //配置寄存器地址
#define NRF_REG_EN_AA 0x01 //使能自动应答功能
#define NRF_REG_EN_RXADDR 0x02 //接收地址允许
#define NRF_REG_SETUP_AW 0x03 //设置地址宽度(所有数据通道)
#define NRF_REG_SETUP_RETR 0x04 //建立自动重发
#define NRF_REG_RF_CH 0x05 //RF通道, 设置通信频率
#define NRF_REG_RF_SETUP 0x06 //RF寄存器
#define NRF_REG_STATUS 0x07 //状态寄存器
#define NRF_REG_OBSERVE_TX 0x08 // 发送检测寄存器
#define NRF_REG_CD 0x09 // 载波检测寄存器

#define NRF_REG_RX_ADDR_P0 0x0A // 数据通道0接收地址 5字节
#define NRF_REG_RX_ADDR_P1 0x0B // 数据通道1接收地址 5字节
#define NRF_REG_RX_ADDR_P2 0x0C // 数据通道2接收地址 1字节，高四字节与P1一样
#define NRF_REG_RX_ADDR_P3 0x0D // 数据通道3接收地址 1字节，高四字节与P1一样
#define NRF_REG_RX_ADDR_P4 0x0E // 数据通道4接收地址 1字节，高四字节与P1一样
#define NRF_REG_RX_ADDR_P5 0x0F // 数据通道5接收地址 1字节，高四字节与P1一样

#define NRF_REG_TX_ADDR 0x10 // 发送地址寄存器
/*
   通过查询这5个寄存器，可以知道本次接收到多少数据
*/
#define NRF_REG_RX_PW_P0 0x11 // 接收数据通道0有效数据宽度(1~32字节)
#define NRF_REG_RX_PW_P1 0x12 // 接收数据通道1有效数据宽度(1~32字节)
#define NRF_REG_RX_PW_P2 0x13 // 接收数据通道2有效数据宽度(1~32字节)
#define NRF_REG_RX_PW_P3 0x14 // 接收数据通道3有效数据宽度(1~32字节)
#define NRF_REG_RX_PW_P4 0x15 // 接收数据通道4有效数据宽度(1~32字节)
#define NRF_REG_RX_PW_P5 0x16 // 接收数据通道5有效数据宽度(1~32字节)

#define NRF_REG_FIFO_STATUS 0x17 // FIFO状态寄存器

/*
	状态标志位
*/
#define NRF_STA_BIT_TX_FULL 0x01//发送缓冲满
#define NRF_STA_BIT_RX_P_NO 0x0e//收到数据通道 000-101: Data Pipe Number；110: Not Used；111: RX FIFO Empty
#define NRF_STA_BIT_MAX_RT  0x10//达到最大重发次数，需写1清，否则无法继续通信
#define NRF_STA_BIT_TX_DS   0x20//发送成功，如果使能自动应答，收到应答算成功
#define NRF_STA_BIT_RX_DR   0x40//收到数据

#define TX_PLOAD_WIDTH 32//每次最多发送32个字节
#define RX_PLOAD_WIDTH 32
#define CHANAL	40

#define TX_ADR_WIDTH 5

#define RX_ADR_WIDTH 5

u8 TX_ADDRESS[TX_ADR_WIDTH] = {0x02,0x02,0x02,0x02,0x02};

u8 RX_ADDRESS_0[RX_ADR_WIDTH] = {0x02,0x02,0x02,0x02,0x02};

u8 RX_ADDRESS_1[RX_ADR_WIDTH] = {0x02,0xc2,0xc2,0xc2,0xc2};
u8 RX_ADDRESS_2[1] = {0x03};
u8 RX_ADDRESS_3[1] = {0x02};
u8 RX_ADDRESS_4[1] = {0x02};
u8 RX_ADDRESS_5[1] = {0x02};


/**
 *@brief:      dev_nrf24l01_init
 *@details:    初始化NRF24
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_nrf24l01_init(void)
{
    
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
 
	/* ce */
	GPIO_InitStructure.GPIO_Pin = RF24L01_CE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(RF24L01_CE_PORT, &GPIO_InitStructure);

	/* IRQ */
	GPIO_InitStructure.GPIO_Pin = RF24L01_IRQ_PORT;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(RF24L01_IRQ_PIN, &GPIO_InitStructure);

	GPIO_ResetBits(RF24L01_CE_PORT, RF24L01_CE_PIN);
	return 0;
}

/**
 *@brief:      dev_nrf24l01_setce
 *@details:    设置CE电平，高电平RF工作
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 dev_nrf24l01_setce(u8 sta)
{
	if(sta == 1)
	{
		GPIO_SetBits(RF24L01_CE_PORT, RF24L01_CE_PIN);	
	}
	else
	{
		GPIO_ResetBits(RF24L01_CE_PORT, RF24L01_CE_PIN);	
	}
	
	return 0;
}
/**
 *@brief:      dev_nrf24l01_irqsta
 *@details:    读IRQ管脚中断状态
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static s32 dev_nrf24l01_irqsta(void)
{
	u8 sta;
	sta = GPIO_ReadInputDataBit(RF24L01_IRQ_PIN, RF24L01_IRQ_PORT);	
	if(sta == Bit_SET)
		return 1;
	else
		return 0;
}


static u8 dev_nrf24l01_write_reg(u8 reg,u8 data)
{
	u8 cmd[2];
	s32 res;
	
	cmd[0] = reg;
	cmd[1] = data;
	
	//mcu_spi_cs(RF24L01_SPI, 0);
	//res = mcu_spi_transfer(RF24L01_SPI, &cmd[0], NULL, 2);
	//mcu_spi_cs(RF24L01_SPI, 1);
	
    return 0;
}

static s32 dev_nrf24l01_read_reg(u8 reg, u8 *data)
{
  	s32 res;

	
	//mcu_spi_cs(RF24L01_SPI, 0);
    //res = mcu_spi_transfer(RF24L01_SPI, &reg, NULL, 1);
    if(res != -1)
    {
	//	res = mcu_spi_transfer(RF24L01_SPI, NULL, data, 1);

    }
	//mcu_spi_cs(RF24L01_SPI, 1);
	
    return res;
}

static s32 dev_nrf24l01_write_buf(u8 reg, u8 *pBuf, u8 len)
{
	s32 res;
	u8 tmp = reg;

	//mcu_spi_cs(RF24L01_SPI, 0);
	//res = mcu_spi_transfer(RF24L01_SPI, &tmp, NULL, 1);
    if(res != -1)
    {
	//	res = mcu_spi_transfer(RF24L01_SPI, pBuf, NULL, len);
    }
	
	//mcu_spi_cs(RF24L01_SPI, 1);
	return res;
}

static u8 dev_nrf24l01_read_buf(u8 reg,u8 *pBuf, u8 len)
{

	s32 res;
	u8 tmp;
	tmp = reg;

	//mcu_spi_cs(RF24L01_SPI, 0);
    //res = mcu_spi_transfer(RF24L01_SPI, &tmp, NULL, 1);
    if(res != -1)
    {
    //	res = mcu_spi_transfer(RF24L01_SPI, NULL, pBuf, len);
    }
	//mcu_spi_cs(RF24L01_SPI, 1);
	
    return 0;   
}

static s32 dev_nrf24l01_openspi(void)
{
	 s32 res;

	//res = mcu_spi_open(RF24L01_SPI, SPI_MODE_0, SPI_BaudRatePrescaler_32);
	if(res == -1)
	{
		wjq_log(LOG_FUN, "rf open spi err\r\n");
		return -1;
	}	
	
	return 0;
}

static s32 dev_nrf24l01_closespi(void)
{
	//mcu_spi_close(RF24L01_SPI);	
	return 0;
}
/**
 *@brief:      NRF_Tx_Dat
 *@details:       发送数据，阻塞模式
 *@param[in]  u8 *txbuf  
               u16 len    
 *@param[out]  无
 *@retval:     
 */
u8 NRF_Tx_Dat(u8 *txbuf, u16 len)  
{  
    u8 state;    

	
	wjq_log(LOG_FUN, "%s\r\n", __FUNCTION__);

	dev_nrf24l01_openspi();

	/*ce为低，进入待机模式1*/
	 dev_nrf24l01_setce(0);

	/*写数据到TX BUF 最大 32个字节*/ 
	dev_nrf24l01_write_buf(NRF_WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); 

		 /*CE为高，txb非空，发送数据包 */
	    dev_nrf24l01_setce(1);
		 
		 /*等待发送完成中断 */ 
	    while(0 != dev_nrf24l01_irqsta()); 
		
		wjq_log(LOG_FUN, "NRF send data ok\r\n");
	    dev_nrf24l01_read_reg(NRF_REG_STATUS, &state); /*读取状态寄存器的值 */   
	    dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_STATUS, state); /*清除TX_DS或MAX_RT中断标志*/      
	    dev_nrf24l01_write_reg(NRF_FLUSH_TX, NRF_NOP);    //清除TX FIFO寄存器   

	wjq_log(LOG_FUN, "NRF send data finish\r\n");
	
	dev_nrf24l01_closespi();
	/*判断中断类型*/      
    if(state&NRF_STA_BIT_MAX_RT)   //达到最大重发次数  
        return NRF_STA_BIT_MAX_RT;   
  
    else if(state&NRF_STA_BIT_TX_DS)//发送完成  
        return NRF_STA_BIT_TX_DS;  
    else  
        return ERROR;                 //其他原因发送失败  
}   

s32 NRF_Rx_Dat(u8 *rxb)  
{  
    u8 state;  
	s32 res;
	
	dev_nrf24l01_openspi();
    dev_nrf24l01_setce(1);   //进入接收状态  
     /*等待接收中断*/  
    while(dev_nrf24l01_irqsta()!=0);  
	
    dev_nrf24l01_setce(0);    //进入待机状态                  
    dev_nrf24l01_read_reg(NRF_REG_STATUS, &state);    /*读取status寄存器的值*/    
    if(state&NRF_STA_BIT_RX_DR)  /*判断是否接收到数据*/  //接收到数据  
    {
    	wjq_log(LOG_FUN, "nrf receive data\r\n");
		
        dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_STATUS,state);/* 清除中断标志*/   
        dev_nrf24l01_read_buf(NRF_RD_RX_PLOAD, rxb, RX_PLOAD_WIDTH);//读取数据  
        
        PrintFormat(rxb, RX_PLOAD_WIDTH);
		
//      SPI_NRF_WriteReg(FLUSH_RX,NOP);          //清除RX FIFO寄存器  
        res = NRF_STA_BIT_RX_DR;   
    }else      
        res =  ERROR;                    //没收到任何数据  

	dev_nrf24l01_closespi();

	return res;
}  

extern void Delay(__IO uint32_t nTime);


void NRF_TX_Mode(u8 *addr, u8 rfch)  
{    
   dev_nrf24l01_openspi();
	
   dev_nrf24l01_setce(0);  
   /*写TX节点地址;  */
   dev_nrf24l01_write_buf(NRF_WRITE_REG+NRF_REG_TX_ADDR, addr, TX_ADR_WIDTH); 
   /*//设置TX节点0地址, 跟发送地址一样，主要为了使能ACK;   */
   dev_nrf24l01_write_buf(NRF_WRITE_REG+NRF_REG_RX_ADDR_P0, addr, RX_ADR_WIDTH);
   
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_EN_AA,0x01);     //使能通道0的自动应答;      
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_EN_RXADDR,0x01); //使能通道0的接收地址;    
   /*//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次;  */
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_SETUP_RETR,0x1a);
   /**/
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_RF_CH,  rfch);   //设置RF通道为CHANAL;  
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_RF_SETUP, 0x07);  //设置TX发射参数,0db增益,1Mbps,低噪声增益开启;     
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,发射模式,开启所有中断;  

	dev_nrf24l01_closespi();
/*CE拉高，进入发送模式*/   
   dev_nrf24l01_setce(1);  
   Delay(100); //CE要拉高一段时间才进入发送模式  
}  

void NRF_RX_Mode(u8 rfch)  
{  
   dev_nrf24l01_openspi();
   dev_nrf24l01_setce(0);      
   dev_nrf24l01_write_buf(NRF_WRITE_REG+NRF_REG_RX_ADDR_P0, RX_ADDRESS_0, RX_ADR_WIDTH);//写RX节点地址  
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_EN_AA,0x01);    //使能通道0的自动应答      
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_EN_RXADDR,0x01);//使能通道0的接收地址      
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_RF_CH, rfch);      //设置RF通信频率      
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_RX_PW_P0, RX_PLOAD_WIDTH);//选择通道0的有效数据宽度        
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_RF_SETUP,0x07); //设置TX发射参数,0db增益,1Mbps,低噪声增益开启     
   dev_nrf24l01_write_reg(NRF_WRITE_REG+NRF_REG_CONFIG, 0x0f);  //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式  

   dev_nrf24l01_closespi();

   dev_nrf24l01_setce(1); /*CE拉高，进入接收模式*/   
   Delay(100); //CE拉高一段时间  
}     

/**
 *@brief:      dev_nrf24l01_check
 *@details:    检测模块是否连接
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_nrf24l01_check(void)  
{  
    u8 buf[5]={0xC2,0xC2,0xC2,0xC2,0xC2};  
    u8 buf1[5];  
 
	
	dev_nrf24l01_openspi();
    dev_nrf24l01_write_buf(NRF_WRITE_REG + NRF_REG_TX_ADDR, buf, 5); /*写入5个字节的地址.  */    
    dev_nrf24l01_read_buf(NRF_REG_TX_ADDR, buf1, 5); /*读出写入的地址 */ 
	dev_nrf24l01_closespi();
	
	wjq_log(LOG_FUN, "\r\n-------------\r\n");
	PrintFormat(buf, sizeof(buf));
	wjq_log(LOG_FUN, "\r\n-------------\r\n");
	PrintFormat(buf1, sizeof(buf1));
	wjq_log(LOG_FUN, "\r\n-------------\r\n");
	
	if(0 == memcmp(buf,buf1, sizeof(buf)))  
    {
    	wjq_log(LOG_FUN, "nrf24l01 ok\r\n");
    	return 0;
    }
	else
	{
		wjq_log(LOG_FUN, "nrf24l01 err\r\n");
		return -1;
	}
}  
/**
 *@brief:      dev_nrf24l01_task
 *@details:    常驻任务，可以看做线程
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_nrf24l01_task(void)
{
	return 0;
}
  

s32 dev_nrf24l01_open(void)
{
	return 0;
}
s32 dev_nrf24l01_close(void)
{
	/*进入待机空闲模式*/
	dev_nrf24l01_setce(0);
	return 0;
}
/**
 *@brief:      dev_nrf24l01_read
 *@details:    读数据
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 dev_nrf24l01_read(u8 *buf, u16 len)
{
		return 0;
}
/**
 *@brief:      dev_nrf24l01_write
 *@details:       写数据，发送数据
 *@param[in]       无
 *@param[out]  无
 *@retval:     
 */
s32 dev_nrf24l01_write(u8* addr, u8 *buf, u16 len)
{
	
	/* 进入发送模式 */
	NRF_TX_Mode(addr, CHANAL);
	
	/* 发送数据 */
	NRF_Tx_Dat(buf,len);
	/* 返回接收模式 */
	NRF_RX_Mode(CHANAL);
	
	return 0;
}


s32 dev_nrf24l01_test(void)
{
	s32 res;
	u8 buf[32];
	u8 src[256];
	u16 i;
	
	for(i = 0; i<sizeof(src);i++)
	{
		src[i] = i;
	}

	
	dev_nrf24l01_init();
	res = dev_nrf24l01_check();
	if(res != 0)
	{
		wjq_log(LOG_FUN, "nrf 24l01 init err!\r\n");
		while(1);
	}
	/*进入接收模式*/
	NRF_RX_Mode(CHANAL);
	while(1)
	{
		#if 1
		wjq_log(LOG_FUN, "nrf write test\r\n");
		dev_nrf24l01_write(TX_ADDRESS ,src, sizeof(src));	
		Delay(1000);
		#else
		NRF_Rx_Dat(buf);
		#endif
	}
}

