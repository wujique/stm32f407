/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

extern DSTATUS SD_disk_initialize (
                         BYTE drv		/* Physical drive number (0) */
                           );
extern DSTATUS SD_disk_status (
                     BYTE drv		/* Physical drive number (0) */
                       );
extern DRESULT SD_disk_read (
                   BYTE pdrv,			/* Physical drive number (0) */
                   BYTE *buff,			/* Pointer to the data buffer to store read data */
                   DWORD sector,		/* Start sector number (LBA) */
                   UINT count			/* Sector count (1..255) */
                     );
extern DRESULT SD_disk_write (
                    BYTE pdrv,			/* Physical drive number (0) */
                    const BYTE *buff,	/* Pointer to the data to be written */
                    DWORD sector,		/* Start sector number (LBA) */
                    UINT count			/* Sector count (1..255) */
                      );

extern DRESULT SD_disk_ioctl (
                    BYTE drv,		/* Physical drive number (0) */
                    BYTE ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
                      );


extern DSTATUS USB_disk_initialize (
					   BYTE drv 	  /* Physical drive number (0) */
						 );
extern DSTATUS USB_disk_status (
				   BYTE drv 	  /* Physical drive number (0) */
					 );
extern DRESULT USB_disk_read (
				 BYTE pdrv, 		  /* Physical drive number (0) */
				 BYTE *buff,		  /* Pointer to the data buffer to store read data */
				 DWORD sector,		  /* Start sector number (LBA) */
				 UINT count 		  /* Sector count (1..255) */
				   );
extern DRESULT USB_disk_write (
				  BYTE pdrv,		  /* Physical drive number (0) */
				  const BYTE *buff,   /* Pointer to the data to be written */
				  DWORD sector, 	  /* Start sector number (LBA) */
				  UINT count		  /* Sector count (1..255) */
					);
extern DRESULT USB_disk_ioctl (
				  BYTE drv, 	  /* Physical drive number (0) */
				  BYTE ctrl,	  /* Control code */
				  void *buff	  /* Buffer to send/receive control data */
					);


/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		//result = MMC_disk_status();

		stat = SD_disk_status(pdrv);
		//uart_printf("disk status SD:%d\r\n", result);
		// translate the reslut code here

		return stat;

	case DEV_USB :
		stat = USB_disk_status(pdrv);

		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		//result = MMC_disk_initialize();
		stat = SD_disk_initialize(pdrv);
		//uart_printf("disk initialize SD:%d\r\n", result);
		// translate the reslut code here

		return stat;

	case DEV_USB :
		stat = USB_disk_initialize(pdrv);
		//uart_printf("disk initialize usb:%d\r\n", result);
		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = ATA_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_read(buff, sector, count);
		res = SD_disk_read(pdrv, buff, sector, count);
		//uart_printf("disk read SD:%d\r\n", result);
		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here
		//uart_printf("disk read usb:%d\r\n", result);
		res = USB_disk_read(pdrv, buff, sector, count);
		//uart_printf("disk read usb:%d\r\n", result);
		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_write(buff, sector, count);
		//uart_printf("disk write SD:%d\r\n", result);
		res = SD_disk_write(pdrv, buff, sector, count);
		//uart_printf("disk write SD:%d\r\n", result);
		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		res = USB_disk_write(pdrv, buff, sector, count);
		//uart_printf("disk write usb:%d\r\n", result);
		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card
		res = SD_disk_ioctl(pdrv, cmd, buff);

		return res;

	case DEV_USB :

		res = USB_disk_ioctl(pdrv, cmd, buff);
		// Process of the command the USB drive

		return res;
	}

	return RES_PARERR;
}

