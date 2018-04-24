/**
 * @file            mcu_spi.c
 * @brief           片上SPI控制器驱动
 * @author          test
 * @date            2017年10月26日 星期四
 * @version         初稿
 * @par             
 * @par History:
 * 1.日    期:      2017年10月26日 星期四
 *   作    者:      test
 *   修改内容:      创建文件
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
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_spi.h"

#define MCU_SPI_DEBUG

#ifdef MCU_SPI_DEBUG
#define SPI_DEBUG	wjq_log 
#else
#define SPI_DEBUG(a, ...)
#endif


#define MCU_SPI_WAIT_TIMEOUT 0x40000
/*
	硬件SPI使用控制器SPI3
*/
#define SPI_DEVICE SPI3
/*
	相位配置，一共四种模式
*/
typedef struct
{
	u16 CPOL;
	u16 CPHA;	
}_strSpiModeSet;

const _strSpiModeSet SpiModeSet[SPI_MODE_MAX]=
	{
		{SPI_CPOL_Low, SPI_CPHA_1Edge},
		{SPI_CPOL_Low, SPI_CPHA_2Edge},
		{SPI_CPOL_High, SPI_CPHA_1Edge},
		{SPI_CPOL_High, SPI_CPHA_2Edge}
	};

/*SPI控制设备符，每个硬件控制器必须定义一个，此处用了SPI3一个硬件SPI控制器*/
s32 DevSpi3Gd = -2;

/**
 *@brief:      mcu_spi_init
 *@details:    初始化SPI控制器，并初始化所有CS脚
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStruct;

    //初始化片选，系统暂时设定为3个SPI，全部使用SPI3
    //DEV_SPI_3_1, 核心板上的SPI FLASH
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	
    //DEV_SPI_3_2, 底板的SPI FLASH
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_SetBits(GPIOG,GPIO_Pin_15);

	//DEV_SPI_3_3, 核心板外扩SPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_SetBits(GPIOG,GPIO_Pin_6);
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;//---PB3~5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//---复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//---推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//---100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//---上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//---初始化

    //配置引脚复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3); //PB3 复用为 SPI3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3); //PB4 复用为 SPI3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3); //PB5 复用为 SPI3

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);// ---使能 SPI3 时钟
    // 复位SPI模块
    SPI_I2S_DeInit(SPI_DEVICE);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//---双线双向全双工
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//---主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//---8bit帧结构
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;//----串行同步时钟的空闲状态为低电平
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;//---数据捕获于第1个时钟沿
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; //---SPI_NSS_Hard; 片选由硬件管理，SPI控制器不管理
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  //---预分频
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//---数据传输从 MSB 位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;//---CRC 值计算的多项式

    SPI_Init(SPI_DEVICE, &SPI_InitStruct);
    
    //SPI_SSOutputCmd(SPI_DEVICE, DISABLE); 
    DevSpi3Gd = -1;
    return 0;
}

/**
 *@brief:      mcu_spi_open
 *@details:       打开SPI
 *@param[in]   SPI_DEV dev  ：SPI号
               u8 mode      模式
               u16 pre      预分频
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_open(SPI_DEV dev, SPI_MODE mode, u16 pre)
{
	SPI_InitTypeDef SPI_InitStruct;

	if(DevSpi3Gd != -1)
		return -1;

	if(mode >= SPI_MODE_MAX)
		return -1;

	SPI_I2S_DeInit(SPI_DEVICE);
	
	SPI_Cmd(SPI1, DISABLE); 
	
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//---双线双向全双工
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//---主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//---8bit帧结构
    SPI_InitStruct.SPI_CPOL = SpiModeSet[mode].CPOL;
    SPI_InitStruct.SPI_CPHA = SpiModeSet[mode].CPHA;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; //---SPI_NSS_Hard; 片选由硬件管理，SPI控制器不管理
    SPI_InitStruct.SPI_BaudRatePrescaler = pre;  //---预分频
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//---数据传输从 MSB 位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;//---CRC 值计算的多项式

    SPI_Init(SPI_DEVICE, &SPI_InitStruct);
	
	/*
		要先使能SPI，再使能CS
	*/
	SPI_Cmd(SPI_DEVICE, ENABLE);
	
	if(dev == DEV_SPI_3_1)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);	
	}
	else if(dev == DEV_SPI_3_2)
	{
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);	
	}
	else if(dev == DEV_SPI_3_3)
	{
		GPIO_ResetBits(GPIOG,GPIO_Pin_6);
	}
	else
	{
		return -1;
	}

	DevSpi3Gd = dev;
		
    
    return 0;
}
/**
 *@brief:      mcu_spi_close
 *@details:    关闭SPI 控制器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_close(SPI_DEV dev)
{
	if(DevSpi3Gd != dev)
	{
		SPI_DEBUG(LOG_DEBUG, "spi dev err\r\n");
		return -1;
	}
	
	if(dev == DEV_SPI_3_1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);	
	}
	else if(dev == DEV_SPI_3_2)
	{
		GPIO_SetBits(GPIOG,GPIO_Pin_15);	
	}
	else if(dev == DEV_SPI_3_3)
	{
		GPIO_SetBits(GPIOG,GPIO_Pin_6);
	}
	else
	{
		return -1;
	}
	
    SPI_Cmd(SPI_DEVICE, DISABLE);
	DevSpi3Gd = -1;
    return 0;
}
/**
 *@brief:      mcu_spi_transfer
 *@details:    SPI 传输
 *@param[in]   u8 *snd  
               u8 *rsv  
               s32 len  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_transfer(SPI_DEV dev, u8 *snd, u8 *rsv, s32 len)
{
    s32 i = 0;
    s32 pos = 0;
    u32 time_out = 0;
    u16 ch;

	if(dev != DevSpi3Gd)
	{
		SPI_DEBUG(LOG_DEBUG, "spi dev err\r\n");
		return -1;
	}
	
    if( ((snd == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return -1;
    }
    
    /* 忙等待 */
    time_out = 0;
    while(SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_BSY) == SET)
    {
        if(time_out++ > MCU_SPI_WAIT_TIMEOUT)
        {
            return(-1);
        }
    }

    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    time_out = 0;
    while(SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == SET)
    {
        SPI_I2S_ReceiveData(SPI_DEVICE);
        if(time_out++ > 2)
        {
            return(-1);
        }
    }

    /* 开始传输 */
    for(i=0; i < len; )
    {
        // 写数据
        if(snd == NULL)/*发送指针为NULL，说明仅仅是读数据 */
        {
            SPI_I2S_SendData(SPI_DEVICE, 0xff);
        }
        else
        {
            ch = (u16)snd[i];
            SPI_I2S_SendData(SPI_DEVICE, ch);
        }
        i++;
        
        // 等待接收结束
        time_out = 0;
        while(SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == RESET)
        {
            time_out++;
            if(time_out > MCU_SPI_WAIT_TIMEOUT)
            {
                return -1;
            }    
        }
        // 读数据
        if(rsv == NULL)/* 接收指针为空，读数据后丢弃 */
        {
            SPI_I2S_ReceiveData(SPI_DEVICE);
        }
        else
        {
            ch = SPI_I2S_ReceiveData(SPI_DEVICE);
            rsv[pos] = (u8)ch;
        } 
        pos++;

    }

    return i;
}
/**
 *@brief:      mcu_spi_cs
 *@details:    操控对应SPI的CS
 *@param[in]   SPI_DEV dev  
               u8 sta       
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_cs(SPI_DEV dev, u8 sta)
{
	if(DevSpi3Gd != dev)
	{
		SPI_DEBUG(LOG_DEBUG, "spi dev err\r\n");
		return -1;
	}

	if(sta == 1)
	{
		switch(dev)
		{
			case DEV_SPI_3_1:
				GPIO_SetBits(GPIOB,GPIO_Pin_14);
				break;
			case DEV_SPI_3_2:
				GPIO_SetBits(GPIOG,GPIO_Pin_15);
				break;
			case DEV_SPI_3_3:
				GPIO_SetBits(GPIOG,GPIO_Pin_6);
				break;
			default:
				break;
		}
	}
	else
	{
		switch(dev)
		{
			case DEV_SPI_3_1:
				GPIO_ResetBits(GPIOB,GPIO_Pin_14);
				break;
			case DEV_SPI_3_2:
				GPIO_ResetBits(GPIOG,GPIO_Pin_15);
				break;
			case DEV_SPI_3_3:
				GPIO_ResetBits(GPIOG,GPIO_Pin_6);
				break;
			default:
				break;
		}
	}
	
	return 0;
}


/*
	VSPI1，使用触摸屏四线接口模拟SPI，用于XPT2046方案触摸处理，可读可写。
*/

/*

	对虚拟SPI的硬件操作抽象对象
	（注意，不是对SPI接口的抽象）
	*
	直接传入IO口还是传入函数？
	传入函数，1 不同的IC函数可能不一样。
			2 同一个IC，不同的IO，说不定IO口的配置也不一样。
	传入IO口配置：简单，容易改动
	*
*/
typedef struct
{
	char *name;
	SPI_DEV dev;
	s32 gd;
	
	u32 clkrcc;
	GPIO_TypeDef *clkport;
	u16 clkpin;

	u32 mosircc;
	GPIO_TypeDef *mosiport;
	u16 mosipin;

	u32 misorcc;
	GPIO_TypeDef *misoport;
	u16 misopin;

	u32 csrcc;
	GPIO_TypeDef *csport;
	u16 cspin;
}DevVspiIO;

/*

	外扩接口模拟SPI，可接OLED LCD或COG LCD

*/
#define VSPI1_CS_PORT GPIOB
#define VSPI1_CS_PIN GPIO_Pin_1
	
#define VSPI1_CLK_PORT GPIOB
#define VSPI1_CLK_PIN GPIO_Pin_0
	
#define VSPI1_MOSI_PORT GPIOD
#define VSPI1_MOSI_PIN GPIO_Pin_11
	
#define VSPI1_MISO_PORT GPIOD
#define VSPI1_MISO_PIN GPIO_Pin_12
	
#define VSPI1_RCC RCC_AHB1Periph_GPIOB
#define VSPI1_RCC2 RCC_AHB1Periph_GPIOD	

DevVspiIO DevVspi1IO={
		"VSPI1",
		DEV_VSPI_1,
		-2,//未初始化
		
		VSPI1_RCC,
		VSPI1_CLK_PORT,
		VSPI1_CLK_PIN,
		
		VSPI1_RCC2,
		VSPI1_MOSI_PORT,
		VSPI1_MOSI_PIN,

		VSPI1_RCC2,
		VSPI1_MISO_PORT,
		VSPI1_MISO_PIN,

		VSPI1_RCC,
		VSPI1_CS_PORT,
		VSPI1_CS_PIN,
	};
		
#define VSPI2_CS_PORT GPIOF
#define VSPI2_CS_PIN GPIO_Pin_12
		
#define VSPI2_CLK_PORT GPIOF
#define VSPI2_CLK_PIN GPIO_Pin_11
		
#define VSPI2_MOSI_PORT GPIOF
#define VSPI2_MOSI_PIN GPIO_Pin_10
		
#define VSPI2_MISO_PORT GPIOF
#define VSPI2_MISO_PIN GPIO_Pin_9
		
#define VSPI2_RCC RCC_AHB1Periph_GPIOF
		
DevVspiIO DevVspi2IO={
		"VSPI2",
		DEV_VSPI_2,
		-2,//未初始化
		
		VSPI2_RCC,
		VSPI2_CLK_PORT,
		VSPI2_CLK_PIN,
		
		VSPI2_RCC,
		VSPI2_MOSI_PORT,
		VSPI2_MOSI_PIN,

		VSPI2_RCC,
		VSPI2_MISO_PORT,
		VSPI2_MISO_PIN,

		VSPI2_RCC,
		VSPI2_CS_PORT,
		VSPI2_CS_PIN,
	};

/*无用的虚拟SPI设备，占位用*/		
DevVspiIO DevVspiNULL={
		"VSPI0",
		DEV_VSPI_0,
		-2,//未初始化;
		};

DevVspiIO *DevVspiIOList[]={
	&DevVspiNULL,
	
	#ifdef SYS_USE_VSPI1
	&DevVspi1IO,
	#endif

	#ifdef SYS_USE_VSPI2
	&DevVspi2IO,
	#endif
	};

/**
 *@brief:      mcu_vspi_init
 *@details:       初始化虚拟SPI
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	u8 i;
	/* 根据传入的dev找到IO口配置*/
	DevVspiIO *vspi;

	i = 1;
	while(1)
	{
		if(i >= sizeof(DevVspiIOList)/sizeof(DevVspiIO *))
			break;
		
		vspi = DevVspiIOList[i];
		
		wjq_log(LOG_INFO, "bus_vspi_init %s\r\n", vspi->name);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

		RCC_AHB1PeriphClockCmd(vspi->csrcc, ENABLE);
		GPIO_InitStructure.GPIO_Pin = vspi->cspin;
		GPIO_Init(vspi->csport, &GPIO_InitStructure);
		GPIO_SetBits(vspi->csport, vspi->cspin);

		RCC_AHB1PeriphClockCmd(vspi->clkrcc, ENABLE);
		GPIO_InitStructure.GPIO_Pin = vspi->clkpin;	
		GPIO_Init(vspi->clkport, &GPIO_InitStructure);
		GPIO_SetBits(vspi->clkport,vspi->clkpin);

		RCC_AHB1PeriphClockCmd(vspi->mosircc, ENABLE);
		GPIO_InitStructure.GPIO_Pin = vspi->mosipin;
		GPIO_Init(vspi->mosiport, &GPIO_InitStructure);
		GPIO_SetBits(vspi->mosiport, vspi->mosipin);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

		RCC_AHB1PeriphClockCmd(vspi->misorcc, ENABLE);
		GPIO_InitStructure.GPIO_Pin = vspi->misopin;
		GPIO_Init(vspi->misoport, &GPIO_InitStructure);
		GPIO_SetBits(vspi->misoport, vspi->misopin);

		vspi->gd = -1;

		i++;
			
	}
	wjq_log(LOG_INFO, "vspi init finish!\r\n");
	
	return 0;
}


static DevVspiIO *mcu_vspi_find_io(SPI_DEV dev)
{
	u8 i;
	/* 根据传入的dev找到IO口配置*/
	DevVspiIO *vspi;

	i = 0;
	while(1)
	{
		if(i >= sizeof(DevVspiIOList)/sizeof(DevVspiIO *))
			break;
		
		vspi = DevVspiIOList[i];
		if(vspi->dev == dev)
		{
			//uart_printf("vspi find io\r\n");
			return vspi;
		}
		i++;
	}
	
	return NULL;
	
}
/**
 *@brief:      vspi_delay
 *@details:    虚拟SPI时钟延时
 *@param[in]   u32 delay  
 *@param[out]  无
 *@retval:     
 */
void vspi_delay(u32 delay)
{
	volatile u32 i=delay;

	while(i>0)
	{
		i--;	
	}

}
/**
 *@brief:      mcu_vspi_open
 *@details:    打开虚拟SPI
 *@param[in]   SPI_DEV dev    
               SPI_MODE mode  
               u16 pre        
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_open(SPI_DEV dev, SPI_MODE mode, u16 pre)
{
	DevVspiIO *vspi;

	vspi = mcu_vspi_find_io(dev);

	if(vspi == NULL)
		return -1;
	
	if(vspi->gd != -1)
		return -1;

	GPIO_ResetBits(vspi->csport, vspi->cspin);	

	vspi->gd = dev;
		
    return 0;
}
/**
 *@brief:      mcu_vspi_close
 *@details:       关闭虚拟SPI
 *@param[in]  SPI_DEV dev  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_close(SPI_DEV dev)
{
	DevVspiIO *vspi;

	vspi = mcu_vspi_find_io(dev);

	if(vspi == NULL)
		return -1;
	
	if(vspi->gd != dev)
	{
		SPI_DEBUG(LOG_DEBUG, "vspi dev err\r\n");
		return -1;
	}
	
	GPIO_SetBits(vspi->csport, vspi->cspin);	

	vspi->gd = -1;
    return 0;
}
/**
 *@brief:      mcu_vspi_transfer
 *@details:       虚拟SPI通信
 *@param[in]   SPI_DEV dev  
               u8 *snd      
               u8 *rsv      
               s32 len      
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_transfer(SPI_DEV dev, u8 *snd, u8 *rsv, s32 len)
{
	u8 i;
	u8 data;
	s32 slen;
	u8 misosta;
	DevVspiIO *vspi;

	vspi = mcu_vspi_find_io(dev);

	if(vspi == NULL)
		return -1;

	if(dev != vspi->gd)
	{
		SPI_DEBUG(LOG_DEBUG, "vspi dev err\r\n");
		return -1;
	}
	
    if( ((snd == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return -1;
    }

	slen = 0;

	while(1)
	{
		if(slen >= len)
			break;

		if(snd == NULL)
			data = 0xff;
		else
			data = *(snd+slen);
		
		for(i=0; i<8; i++)
		{
			GPIO_ResetBits(vspi->clkport, vspi->clkpin);
			vspi_delay(10);
			
			if(data&0x80)
				GPIO_SetBits(vspi->mosiport, vspi->mosipin);
			else
				GPIO_ResetBits(vspi->mosiport, vspi->mosipin);
			
			vspi_delay(10);
			data<<=1;
			GPIO_SetBits(vspi->clkport, vspi->clkpin);
			
			misosta = GPIO_ReadInputDataBit(vspi->misoport, vspi->misopin);
			if(misosta == Bit_SET)
			{
				data |=0x01;
			}
			else
			{
				data &=0xfe;
			}
			vspi_delay(10);
			
		}

		if(rsv != NULL)
			*(rsv+slen) = data;
		
		slen++;
	}

	return slen;
}
/**
 *@brief:      mcu_vspi_cs
 *@details:    控制虚拟SPI的使能脚
 *@param[in]   SPI_DEV dev  
               u8 sta       
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_cs(SPI_DEV dev, u8 sta)
{
	DevVspiIO *vspi;

	vspi = mcu_vspi_find_io(dev);

	if(vspi == NULL)
		return -1;

	if(dev != vspi->gd)
	{
		SPI_DEBUG(LOG_DEBUG, "vspi dev err\r\n");
		return -1;
	}

	if(sta == 1)
	{
		GPIO_SetBits(vspi->csport, vspi->cspin);
	}
	else
	{
		GPIO_ResetBits(vspi->csport, vspi->cspin);
	}
	
	return 0;
}

/*


	所有SPI统一对外接口


*/
s32 mcu_spi_init(void)
{
	mcu_hspi_init();
	mcu_vspi_init();
	return 0;
}

/**
 *@brief:      mcu_spi_open
 *@details:       打开SPI
 *@param[in]   SPI_DEV dev  ：SPI号
               u8 mode      模式
               u16 pre      预分频
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_open(SPI_DEV dev, SPI_MODE mode, u16 pre)
{
	switch(dev)
	{
		case DEV_SPI_3_1:
		case DEV_SPI_3_2:
		case DEV_SPI_3_3:
			return mcu_hspi_open(dev, mode, pre);

		case DEV_VSPI_1:
		case DEV_VSPI_2:
			return mcu_vspi_open(dev, mode, pre);
		
		default:
			return -1;
		
	}

}
/**
 *@brief:      mcu_spi_close
 *@details:    关闭SPI 控制器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_close(SPI_DEV dev)
{
	switch(dev)
	{
		case DEV_SPI_3_1:
		case DEV_SPI_3_2:
		case DEV_SPI_3_3:
			return mcu_hspi_close(dev);

		case DEV_VSPI_1:
		case DEV_VSPI_2:
			return mcu_vspi_close(dev);

		default:
			return -1;
	}

}
/**
 *@brief:      mcu_spi_transfer
 *@details:    SPI 传输
 *@param[in]   u8 *snd  
               u8 *rsv  
               s32 len  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_transfer(SPI_DEV dev, u8 *snd, u8 *rsv, s32 len)
{
	switch(dev)
	{
		case DEV_SPI_3_1:
		case DEV_SPI_3_2:
		case DEV_SPI_3_3:
			return mcu_hspi_transfer(dev, snd, rsv, len);
		
		case DEV_VSPI_1:
		case DEV_VSPI_2:
			return mcu_vspi_transfer(dev, snd, rsv, len);

		default:
			return -1;
	}   
}
/**
 *@brief:      mcu_spi_cs
 *@details:    操控对应SPI的CS
 *@param[in]   SPI_DEV dev  
               u8 sta       
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_cs(SPI_DEV dev, u8 sta)
{
	switch(dev)
	{
		case DEV_SPI_3_1:
		case DEV_SPI_3_2:
		case DEV_SPI_3_3:
			return mcu_hspi_cs(dev, sta);

		case DEV_VSPI_1:
		case DEV_VSPI_2:
			return mcu_vspi_cs( dev, sta);
		
		default:
			return -1;

	}
	
}



