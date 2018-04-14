#ifndef __DEV_SPIFLASH_H_
#define __DEV_SPIFLASH_H_

/*SPI FLASH 信息*/
typedef struct
{
	char *name;
	u32 JID;
	u32 MID;
	/*容量，块数，块大小等信息*/
	u32 sectornum;//总块数
	u32 sector;//块大小
	u32 structure;//总容量
	
}_strSpiFlash;


/*SPI FLASH设备定义*/
typedef struct
{
	char *name;//设备名称
	SPI_DEV spi;//挂载在哪条SPI总线
	_strSpiFlash *pra;//设备信息
}DevSpiFlash;


extern s32 dev_spiflash_readmorebyte(DevSpiFlash *dev, u32 addr, u8 *dst, u32 len);
extern s32 dev_spiflash_write(DevSpiFlash *dev, u8* pbuffer, u32 addr, u16 wlen);
extern s32 dev_spiflash_sector_erase(DevSpiFlash *dev, u32 sector_addr);
extern s32 dev_spiflash_sector_read(DevSpiFlash *dev, u32 sector, u8 *dst);	
extern s32 dev_spiflash_sector_write(DevSpiFlash *dev, u32 sector, u8 *src);
extern s32 dev_spiflash_init(void);
extern s32 dev_spiflash_open(DevSpiFlash *dev, char* name);
extern s32 dev_spiflash_test(void);

#endif

