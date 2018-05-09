/**
 * @file            dev_spiflash.c
 * @brief           spi flash 驱动程序
 * @author          test
 * @date            2017年10月26日 星期四
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:      2017年10月26日 星期四
 *   作    者:      test
 *   修改内容:      创建文件
  版权说明：
		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
#include "string.h"

#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_spi.h"
#include "dev_spiflash.h"

#define DEV_SPIFLASH_DEBUG

#ifdef DEV_SPIFLASH_DEBUG
#define SPIFLASH_DEBUG	wjq_log 
#else
#define SPIFLASH_DEBUG(a, ...)
#endif

/*
	SPI FLASH，命令的开始是从CS下降沿开始，所以每次操作都需要进行CS操作
*/

	
/*
	常用的SPI FLASH 参数信息
*/
_strSpiFlash SpiFlashPraList[]=
{
	{"MX25L3206E", 0XC22016, 0XC215, 1024, 4096, 4194304},
	{"W25Q64JVSI", 0Xef4017, 0Xef16, 2048, 4096, 8388608}
};
	
/*
	设备树定义
*/
#define DEV_SPI_FLASH_C 2//总共有两片SPI FLASH

DevSpiFlash DevSpiFlashList[DEV_SPI_FLASH_C]=
{
	/*有一个叫做board_spiflash的SPI FLASH挂在DEV_SPI_3_2上，型号未知*/
	{"board_spiflash", DEV_SPI_3_2, NULL},
	/*有一个叫做board_spiflash的SPI FLASH挂在DEV_SPI_3_1上，型号未知*/
	{"core_spiflash",  DEV_SPI_3_1, NULL},
};


/* spi flash 命令*/
#define SPIFLASH_WRITE      0x02  /* Write to Memory instruction  Page Program */
#define SPIFLASH_WRSR       0x01  /* Write Status Register instruction */
#define SPIFLASH_WREN       0x06  /* Write enable instruction */
#define SPIFLASH_WRDIS      0x04  /* Write disable instruction */
#define SPIFLASH_READ       0x03  /* Read from Memory instruction */
#define SPIFLASH_FREAD      0x0B  /* Fast Read from Memory instruction */
#define SPIFLASH_RDSR       0x05  /* Read Status Register instruction  */
#define SPIFLASH_SE         0x20  /* Sector Erase instruction */
#define SPIFLASH_BE         0xD8  /* Bulk Erase instruction */
#define SPIFLASH_CE         0xC7  /* Chip Erase instruction */
#define SPIFLASH_PD         0xB9  /* Power down enable */
#define SPIFLASH_RPD        0xAB  /* Release from Power down mode */
#define SPIFLASH_RDMID      0x90  /* Read Device identification */
#define SPIFLASH_RDJID      0x9F  /* Read JEDEC identification */
#define SPIFLASH_DUMMY_BYTE 0xA5

/**
 *@brief:      dev_spiflash_writeen
 *@details:    FLASH 写使能:设置FLASH内的寄存器，
               允许写/擦除，每次写/擦除之前都需要发送
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 dev_spiflash_writeen(DevSpiFlash *dev)  
{
    s32 len = 1;
    u8 command = SPIFLASH_WREN;
	mcu_spi_cs(dev->spi, 0);
	mcu_spi_transfer(dev->spi, &command, NULL, len); //数据传输
	mcu_spi_cs(dev->spi, 1);
    return 0;
}
/**
 *@brief:      dev_spiflash_waitwriteend
 *@details:    查询FLASH状态，等待写操作结束
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 dev_spiflash_waitwriteend(DevSpiFlash *dev)
{
    u8 flash_status = 0;
    s32 len = 1;
    u8 command = SPIFLASH_RDSR;
	
	mcu_spi_cs(dev->spi, 0);
    mcu_spi_transfer(dev->spi, &command, NULL, len);
    do
    {
        mcu_spi_transfer(dev->spi, NULL, &flash_status, len);
    }
    while ((flash_status & 0x01) != 0); 
	mcu_spi_cs(dev->spi, 1);
		
		return 0;
}
/**
 *@brief:      dev_spiflash_readmorebyte
 *@details:    读随意长度FLASH数据
 *@param[in]   u32 addr  
               u8 *dst  
               u32 len      
 *@param[out]  无
 *@retval:     
 */
static s32 dev_spiflash_readmorebyte(DevSpiFlash *dev, u32 addr, u8 *dst, u32 rlen)
{
    
    s32 len = 4;
    u8 command[4];

    if(rlen == 0)return 0;

    command[0] = SPIFLASH_READ;
    command[1] = (u8)(addr>>16);
    command[2] = (u8)(addr>>8);
    command[3] = (u8)(addr);
	mcu_spi_cs(dev->spi, 0);
    mcu_spi_transfer(dev->spi, command, NULL, len); 
    mcu_spi_transfer(dev->spi, NULL, dst, rlen);
	mcu_spi_cs(dev->spi, 1);
    return(0);
}
/**
 *@brief:      dev_spiflash_write
 *@details:    写数据到FLASH
 *@param[in]   u8* pbuffer     
               u32 addr  
               u16 wlen        
 *@param[out]  无
 *@retval:     
 */

static s32 dev_spiflash_write(DevSpiFlash *dev, u8* pbuffer, u32 addr, u16 wlen)
{
    s32 len;
    u8 command[4];

    while (wlen)
    {    
		dev_spiflash_writeen(dev);
	
        command[0] = SPIFLASH_WRITE;
        command[1] = (u8)(addr>>16);
        command[2] = (u8)(addr>>8);
        command[3] = (u8)(addr);

        len = 4;
		mcu_spi_cs(dev->spi, 0);
        mcu_spi_transfer(dev->spi, command, NULL, len);
		
        len = 256 - (addr & 0xff);
        if(len < wlen)
        {
            mcu_spi_transfer(dev->spi, pbuffer, NULL, len);
            wlen -= len;
            pbuffer += len;
            addr += len;
        }
        else
        {
            mcu_spi_transfer(dev->spi, pbuffer, NULL, wlen);
            wlen = 0;
            addr += wlen;
            pbuffer += wlen;
        }
		
		mcu_spi_cs(dev->spi, 1);
		
        dev_spiflash_waitwriteend(dev);       
    }
		
	return 0;
}

/**
 *@brief:      dev_spiflash_sector_erase
 *@details:    擦除一个sector
 *@param[in]   u32 sector_addr  sector内地址
 *@param[out]  无
 *@retval:     
 */
s32 dev_spiflash_sector_erase(DevSpiFlash *dev, u32 sector)
{
    s32 len = 4;
    u8 command[4];
	u32 addr;

	if(sector >= dev->pra->sectornum)
		return -1;
	
	addr = sector*dev->pra->sectorsize;
	
    command[0] = SPIFLASH_SE;
    command[1] = (u8)(addr>>16);
    command[2] = (u8)(addr>>8);
    command[3] = (u8)(addr);
    
    dev_spiflash_writeen(dev);
	
	mcu_spi_cs(dev->spi, 0);
    mcu_spi_transfer(dev->spi, command, NULL, len);
	mcu_spi_cs(dev->spi, 1);

    dev_spiflash_waitwriteend(dev);   

return 0;	
}

/**
 *@brief:      dev_spiflash_sector_read
 *@details:    读取一个扇区
 *@param[in]   DevSpiFlash *dev  
               u32 sector  扇区号
               u8 *dst           
 *@param[out]  无
 *@retval:     
 */
s32 dev_spiflash_sector_read(DevSpiFlash *dev, u32 sector, u8 *dst)	
{
	if(sector >= dev->pra->sectornum)
		return -1;
	
	return dev_spiflash_readmorebyte(dev, sector*dev->pra->sectorsize, dst, dev->pra->sectorsize);
}
/**
 *@brief:      dev_spiflash_sector_write
 *@details:    写一个扇区
 *@param[in]   DevSpiFlash *dev  
               u32 sector        
               u8 *src           
 *@param[out]  无
 *@retval:     
 */
s32 dev_spiflash_sector_write(DevSpiFlash *dev, u32 sector, u8 *src)
{
	u16 sector_size;

	if(sector >= dev->pra->sectornum)
		return -1;
	
	sector_size = dev->pra->sectorsize;
	dev_spiflash_write(dev, src, sector*sector_size, sector_size);
	return 0;
}
/**
 *@brief:      dev_spiflash_readMTD
 *@details:    读FLASH MID号
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static u32 dev_spiflash_readMTD(DevSpiFlash *dev)
{
    u32 MID;
    s32 len = 4;
    u8 command[4];
    u8 data[2];

    command[0] = SPIFLASH_RDMID;
    command[1] = 0;
    command[2] = 0;
    command[3] = 0;
	mcu_spi_cs(dev->spi, 0);
    mcu_spi_transfer(dev->spi, command, NULL, len);
    len = 2;
    mcu_spi_transfer(dev->spi, NULL, data, len);
	mcu_spi_cs(dev->spi, 1);
    MID = data[0];
    MID = (MID<<8) + data[1];
         
    return MID;
}
/**
 *@brief:      dev_spiflash_readJTD
 *@details:    读FLASH JTD号
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static u32 dev_spiflash_readJTD(DevSpiFlash *dev)
{
    u32 JID;
    s32 len = 1;
    u8 command = SPIFLASH_RDJID;
    u8 data[3];

	mcu_spi_cs(dev->spi, 0);
    len = 1;
    mcu_spi_transfer(dev->spi, &command, NULL, len);
    len = 3;
    mcu_spi_transfer(dev->spi, NULL, data, len);
	mcu_spi_cs(dev->spi, 1);
	
    JID = data[0];
    JID = (JID<<8) + data[1];
    JID = (JID<<8) + data[2];
    
    return JID;
}

/**
 *@brief:      SpiFlashOpen
 *@details:    打开SPI FLASH
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_spiflash_open(DevSpiFlash *dev, char* name)
{
	s32 res;
	SPI_DEV spidev = DEV_SPI_NULL;
	u8 i=0;

	/*根据名字name查找设备*/
	while(1)
	{
		if(0 == strcmp(name, DevSpiFlashList[i].name))
		{
			spidev = DevSpiFlashList[i].spi;
			break;
		}
		
		i++;
		if(i>= DEV_SPI_FLASH_C)
		{
			SPIFLASH_DEBUG(LOG_DEBUG, "open spi flash err\r\n");
			res = -1;
			break;	
		}		
	}

	SPIFLASH_DEBUG(LOG_DEBUG, "spi flash type:%s\r\n", DevSpiFlashList[i].pra->name);
	
	if(res != -1)
	{
		/*根据查找到的信息打开SPI*/
    	res = mcu_spi_open(spidev, SPI_MODE_3, SPI_BaudRatePrescaler_4); //打开spi
		if(res == -1)
		{
			SPIFLASH_DEBUG(LOG_DEBUG, "open spi err\r\n");
		}
	}
	
	if(res!= -1)
	{
		/*SPI 打开成功，设备可用*/
		dev->name = DevSpiFlashList[i].name;
		dev->spi = DevSpiFlashList[i].spi;
		dev->pra = DevSpiFlashList[i].pra;
	}
	else
	{
		/*打开失败，清，防止乱搞*/
		dev->name = NULL;
		dev->spi = DEV_SPI_NULL;
		dev->pra = NULL;	
	}
	return 0;    
}
/**
 *@brief:      dev_spiflash_close
 *@details:       关闭SPI FLASH设备
 *@param[in]  DevSpiFlash *dev  
 *@param[out]  无
 *@retval:     
 */
s32 dev_spiflash_close(DevSpiFlash *dev)
{
	s32 res;
	res = mcu_spi_close(dev->spi); 
	if(res == 0)
	{
		dev->name = NULL;
		dev->pra = NULL;
		dev->spi = DEV_SPI_NULL;
		return 0;
	}
	return -1;
}
/**
 *@brief:      dev_spiflash_init
 *@details:    初始化spiflash
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_spiflash_init(void)
{
	/*检测SPI FALSH，并且初始化设备信息*/
	u8 i=0;
	u8 index = 0;
	SPI_DEV spidev;
	s32 res;
	u32 JID = 0;
	u32 MID = 0;
	
	while(1)
	{
		spidev = DevSpiFlashList[i].spi;			
		
		res = mcu_spi_open(spidev, SPI_MODE_3, SPI_BaudRatePrescaler_4); //打开spi
		if(res == 0)
		{
			JID = dev_spiflash_readJTD(&DevSpiFlashList[i]);
			SPIFLASH_DEBUG(LOG_DEBUG, "%s jid:0x%x\r\n", DevSpiFlashList[i].name, JID);
			
			MID  = dev_spiflash_readMTD(&DevSpiFlashList[i]);
			SPIFLASH_DEBUG(LOG_DEBUG, "%s mid:0x%x\r\n", DevSpiFlashList[i].name, MID);
			
			/*根据JID查找设备信息*/
			for(index = 0; index<(sizeof(SpiFlashPraList)/sizeof(_strSpiFlash));index++)
			{
				if((SpiFlashPraList[index].JID == JID)
					&&(SpiFlashPraList[index].MID == MID))
				{
					DevSpiFlashList[i].pra = &(SpiFlashPraList[index]);
					break;
				}
			}
			
			mcu_spi_close(spidev);
		}

		i++;
		if(i>= DEV_SPI_FLASH_C)
		{
			
			break;	
		}		
	}
	
    return 0;    
}


#include "alloc.h"

/**
 *@brief:      dev_spiflash_test_fun
 *@details:    测试FLASH,擦除写读，调试时使用，量产存数据后不要测试
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void dev_spiflash_test_fun(char *name)
{
    u32 addr;
    u16 tmp;
    u8 i = 1;
    u8 rbuf[4096];
    u8 wbuf[4096];
    u8 err_flag = 0;

	DevSpiFlash dev;
	
	s32 res;
	
    wjq_log(LOG_FUN, ">:-------dev_spiflash_test-------\r\n");
    res = dev_spiflash_open(&dev, name);
	wjq_log(LOG_FUN, ">:-------%s-------\r\n", dev.name);
	if(res == -1)
	{
		wjq_log(LOG_FUN, "open spi flash ERR\r\n");
		while(1);
	}
    i = 0;
    for(tmp = 0; tmp < 4096; tmp++)
    {
        wbuf[tmp] = i;
        i++;
    }
    //sector 1 进行擦除，然后写，校验。
    wjq_log(LOG_FUN, ">:-------test sector erase-------\r\n", addr);
    
    addr = 0;
    dev_spiflash_sector_erase(&dev, addr);
    wjq_log(LOG_FUN, "erase...");

    dev_spiflash_sector_read(&dev, addr, rbuf);;//读一页回来
    wjq_log(LOG_FUN, "read...");
    
    for(tmp = 0; tmp < dev.pra->sectorsize; tmp++)
    {
        if(rbuf[tmp] != 0xff)//擦除后全部都是0xff
        {
            wjq_log(LOG_FUN, "%x=%02X ", tmp, rbuf[tmp]);//擦除后不等于0XFF,坏块    
            err_flag = 1;
        }
    }

    dev_spiflash_sector_write(&dev, addr, wbuf);
    wjq_log(LOG_FUN, "write...");
    
    dev_spiflash_sector_read(&dev, addr, rbuf);
    wjq_log(LOG_FUN, "read...");
    
    wjq_log(LOG_FUN, "\r\n>:test wr..\r\n");
    
    for(tmp = 0; tmp < dev.pra->sectorsize; tmp++)
    {
        if(rbuf[tmp] != wbuf[tmp])
        {
            wjq_log(LOG_FUN, "%x ", tmp);//读出来的跟写进去的不相等 
            err_flag = 1;
        }
    }

    if(err_flag == 1)
        wjq_log(LOG_FUN, "bad sector\r\n");
    else
        wjq_log(LOG_FUN, "OK sector\r\n");

	dev_spiflash_close(&dev);
}
/*
	检测整片FLASH
*/
void dev_spiflash_test_chipcheck(char *name)
{
    u32 addr;
	u16 sector;
    u16 tmp;
    u8 i = 1;
    u8 *rbuf;
    u8 *wbuf;
    u8 err_flag = 0;

	DevSpiFlash dev;
	
	s32 res;
	
    wjq_log(LOG_FUN, ">:-------dev_spiflash_test-------\r\n");
    res = dev_spiflash_open(&dev, name);
	wjq_log(LOG_FUN, ">:-------%s-------\r\n", dev.name);
	if(res == -1)
	{
		wjq_log(LOG_FUN, "open spi flash ERR\r\n");
		while(1);
	}

	rbuf = (u8*)wjq_malloc(dev.pra->sectorsize);
	wbuf = (u8*)wjq_malloc(dev.pra->sectorsize);

	for(sector = 0; sector < dev.pra->sectornum; sector++)
	{
	    i = sector%0xff;
		
	    for(tmp = 0; tmp < dev.pra->sectorsize; tmp++)
	    {
	        wbuf[tmp] = i;
	        i++;
	    }
	    
	    addr = sector * (dev.pra->sectorsize);
		
		wjq_log(LOG_FUN, ">:sector:%d, addr:0x%08x,", sector, addr);
	    dev_spiflash_sector_erase(&dev, sector);
	    wjq_log(LOG_FUN, "erase...");

	    dev_spiflash_sector_read(&dev, sector, rbuf);;//读一页回来
	    wjq_log(LOG_FUN, "read...");
	    
	    for(tmp = 0; tmp < dev.pra->sectorsize; tmp++)
	    {
	        if(rbuf[tmp] != 0xff)//擦除后全部都是0xff
	        {
	            //wjq_log(LOG_FUN, "%x=%02X ", tmp, rbuf[tmp]);//擦除后不等于0XFF,坏块    
	            err_flag = 1;
	        }
	    }

	    dev_spiflash_sector_write(&dev, sector, wbuf);
	    wjq_log(LOG_FUN, "write...");
	    
	    dev_spiflash_sector_read(&dev, sector, rbuf);
	    wjq_log(LOG_FUN, "read...");
	    
	    for(tmp = 0; tmp < dev.pra->sectorsize; tmp++)
	    {
	        if(rbuf[tmp] != wbuf[tmp])
	        {
	            //wjq_log(LOG_FUN, "%x ", tmp);//读出来的跟写进去的不相等 
	            err_flag = 1;
	        }
	    }

	    if(err_flag == 1)
	        wjq_log(LOG_FUN, "bad sector\r\n");
	    else
	        wjq_log(LOG_FUN, "OK sector\r\n");
	}
	dev_spiflash_close(&dev);

	wjq_free(rbuf);
	wjq_free(wbuf);
}

s32 dev_spiflash_test(void)
{
	dev_spiflash_test_fun("board_spiflash");
	dev_spiflash_test_fun("core_spiflash");
	return 0;
}

