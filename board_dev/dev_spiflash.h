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
	u32 sectorsize;//块大小
	u32 structure;//总容量
	
}_strSpiFlash;


/*SPI FLASH设备定义*/
typedef struct
{
	char *name;//设备名称
	char *spich;//挂载在哪条SPI通道
	_strSpiFlash *pra;//设备信息
}DevSpiFlash;

typedef struct
{
	/**/
	s32 gd;
	/*设备信息*/
	
	DevSpiFlash dev;
	
	/*spi 通道节点*/
	DevSpiChNode *spichnode;
	
	struct list_head list;
}DevSpiFlashNode;

extern s32 dev_spiflash_sector_erase(DevSpiFlashNode *node, u32 sector_addr);
extern s32 dev_spiflash_sector_read(DevSpiFlashNode *node, u32 sector, u8 *dst);	
extern s32 dev_spiflash_sector_write(DevSpiFlashNode *node, u32 sector, u8 *src);
extern DevSpiFlashNode *dev_spiflash_open(char* name);
extern s32 dev_spiflash_test(void);

#endif

