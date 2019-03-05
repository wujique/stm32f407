
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"

#include "lfs.h"


#define LFS_FLASH_NAME "board_spiflash"
/*
	flash 接口,
	所有函数默认返回0，也就是说，默认操作FLASH成功。
*/

// Read a region in a block. Negative error codes are propogated
// to the user.
int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block,
		lfs_off_t off, void *buffer, lfs_size_t size)
{
	DevSpiFlashNode *node;
	
	node = dev_spiflash_open(LFS_FLASH_NAME);
	if(node == NULL)
	{
			return -1;
	}
	
	dev_spiflash_read(node, block* c->block_size + off, size, buffer);
	
	dev_spiflash_close(node);

	return 0;	
}

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block,
		lfs_off_t off, const void *buffer, lfs_size_t size)
{		
	DevSpiFlashNode *node;
	
	node = dev_spiflash_open(LFS_FLASH_NAME);
	if(node == NULL)
	{
			return -1;
	}
	
	dev_spiflash_write(node, block* c->block_size + off, size, buffer);
	
	dev_spiflash_close(node);

	return 0;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{		
	DevSpiFlashNode *node;
	
	node = dev_spiflash_open(LFS_FLASH_NAME);
	if(node == NULL)
	{
			return -1;
	}
	
	dev_spiflash_erase(node, block* c->block_size);
	
	dev_spiflash_close(node);

	return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propogated to the user.
int user_provided_block_device_sync(const struct lfs_config *c)
{		
	return 0;
}


// variables used by the filesystem

lfs_t lfs;
lfs_file_t lfs_file;


// configuration of the filesystem is provided by this struct
const struct lfs_config lfs_cfg = {
    // block device operations
    .read  = user_provided_block_device_read,
    .prog  = user_provided_block_device_prog,
    .erase = user_provided_block_device_erase,
    .sync  = user_provided_block_device_sync,

    // block device configuration
    .read_size = 256,
    .prog_size = 256,
    .block_size = 4096,
    .block_count = 128,
    .lookahead = 256,
};

void sys_lfs_mount(void)
{
	// mount the filesystem
    int err = lfs_mount(&lfs, &lfs_cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &lfs_cfg);
        lfs_mount(&lfs, &lfs_cfg);
    }	
}

// entry point
/*
	测试
*/
int lfs_test(void) 
{
    
    // read current count
    uint32_t boot_count = 0;
	
    lfs_file_open(&lfs, &lfs_file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &lfs_file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &lfs_file);
    lfs_file_write(&lfs, &lfs_file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &lfs_file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    wjq_log(3, "boot_count: %d\r\n", boot_count);
}


