/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <command.h>

/*

	UBOOT环境变量，在STM32中用来保存一些系统参数
	暂时不实现，

*/

#undef DEBUG_ENV


#ifdef DEBUG_ENV
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif

typedef unsigned char uchar;

static uchar env_get_char_init (int index);

/************************************************************************
 * Default settings to be used when no valid environment is found
 */
#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

/*

	默认环境变量

*/
const uchar default_environment[CONFIG_ENV_SIZE-8] = {
#ifdef	CONFIG_BOOTARGS
	"bootargs="	CONFIG_BOOTARGS			"\0"
#endif
#ifdef	CONFIG_BOOTCOMMAND
	"bootcmd="	CONFIG_BOOTCOMMAND		"\0"
#endif
#ifdef	CONFIG_RAMBOOTCOMMAND
	"ramboot="	CONFIG_RAMBOOTCOMMAND		"\0"
#endif
#ifdef	CONFIG_NFSBOOTCOMMAND
	"nfsboot="	CONFIG_NFSBOOTCOMMAND		"\0"
#endif
#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	"bootdelay="	MK_STR(CONFIG_BOOTDELAY)	"\0"
#endif
#if defined(CONFIG_BAUDRATE) && (CONFIG_BAUDRATE >= 0)
	"baudrate="	MK_STR(CONFIG_BAUDRATE)		"\0"
#endif
#ifdef	CONFIG_LOADS_ECHO
	"loads_echo="	MK_STR(CONFIG_LOADS_ECHO)	"\0"
#endif
#ifdef	CONFIG_IPADDR
	"ipaddr="	MK_STR(CONFIG_IPADDR)		"\0"
#endif
#ifdef	CONFIG_SERVERIP
	"serverip="	MK_STR(CONFIG_SERVERIP)		"\0"
#endif
#ifdef	CONFIG_SYS_AUTOLOAD
	"autoload="	CONFIG_SYS_AUTOLOAD			"\0"
#endif
#ifdef	CONFIG_PREBOOT
	"preboot="	CONFIG_PREBOOT			"\0"
#endif
#ifdef	CONFIG_ROOTPATH
	"rootpath="	MK_STR(CONFIG_ROOTPATH)		"\0"
#endif
#ifdef	CONFIG_GATEWAYIP
	"gatewayip="	MK_STR(CONFIG_GATEWAYIP)	"\0"
#endif
#ifdef	CONFIG_NETMASK
	"netmask="	MK_STR(CONFIG_NETMASK)		"\0"
#endif
#ifdef	CONFIG_HOSTNAME
	"hostname="	MK_STR(CONFIG_HOSTNAME)		"\0"
#endif
#ifdef	CONFIG_BOOTFILE
	"bootfile="	MK_STR(CONFIG_BOOTFILE)		"\0"
#endif
#ifdef	CONFIG_LOADADDR
	"loadaddr="	MK_STR(CONFIG_LOADADDR)		"\0"
#endif
#ifdef  CONFIG_CLOCKS_IN_MHZ
	"clocks_in_mhz=1\0"
#endif
#if defined(CONFIG_PCI_BOOTDELAY) && (CONFIG_PCI_BOOTDELAY > 0)
	"pcidelay="	MK_STR(CONFIG_PCI_BOOTDELAY)	"\0"
#endif
#ifdef  CONFIG_EXTRA_ENV_SETTINGS
	CONFIG_EXTRA_ENV_SETTINGS
#endif
	"\0"
};



uchar environment[CONFIG_ENV_SIZE]; //BSS



static uchar env_get_char_init (int index)
{
	uchar c;
	c = environment[index];
	return (c);
}


uchar env_get_char_memory (int index)
{

	return ( environment[index] );

}


uchar env_get_char (int index)
{
	uchar c;

	/* if relocated to RAM */
	c = env_get_char_memory(index);
	

	return (c);
}

uchar *env_get_addr (int index)
{
	return (&environment[index]);
}

void env_relocate (void)
{
	puts ("Using default environment\n\n");
}


int get_default_env_size()
{
	return sizeof(default_environment);
}


#define BOOTARGS_END_TAGS  0x90abcdef

/**
	Layout
+-----------------+---------------------+	
|___environment____|_BOOTARGS_END_TAGS_|

**/

/**
**/
int  env_init(void)
{
	uint32 addr = 0;
	uint32 *tagPtr = NULL;
	memset(environment,0,sizeof(environment));
	#if 0
	if(dev_spi_flash_open())		//open succ
	{
		for(addr =BOOT_CFG_ADDR;addr < (BOOT_CFG_ADDR+BOOT_CFG_SZIE);addr += CONFIG_ENV_SIZE )
		{
			if(0 == dev_spi_flash_read(addr,environment,CONFIG_ENV_SIZE))
			{
				tagPtr = (uint32 *)(&environment[CONFIG_ENV_SIZE-1-4]);

				if(BOOTARGS_END_TAGS == *tagPtr) //valid
				{
					return 0;
				}
				if(0xFFFFFFFF == *tagPtr)		//Empty
				{
					break;	//use default
				}
			}
		}
	}
	#endif
	//copy default env
	memcpy(environment, default_environment, sizeof(default_environment));
	return (0);
}



int saveenv(void)
{
	uint32 addr = 0;
	uint32 *tagPtr = NULL;
	uchar buffer[4] = {0};
	int rcode = 0;
	int rc = 0;

#if 0
	if(dev_spi_flash_open())		//open succ
	{
		for(addr =BOOT_CFG_ADDR;addr < (BOOT_CFG_ADDR+BOOT_CFG_SZIE);addr += CONFIG_ENV_SIZE )
		{
			memset(buffer,0,sizeof(buffer));
			if(0 == dev_spi_flash_read((addr+CONFIG_ENV_SIZE-4),buffer,4))	//read tag
			{
				tagPtr = (uint32 *)(&buffer[0]);

				if(BOOTARGS_END_TAGS == *tagPtr) //valid
				{
					memset(buffer,0,sizeof(buffer));
					dev_spi_flash_write(buffer,(addr+CONFIG_ENV_SIZE-4),4);	// clear to zero, 1->0
				}

				if(0xFFFFFFFF == *tagPtr) //
				{
					break;
				}
			}
		}
	}
	else 
	{
		return 1;	// faild
	}

	//check  and erase 
	if(addr >=(BOOT_CFG_ADDR+BOOT_CFG_SZIE)) // all space used
	{
		puts ("Erasing Flash...");
		for(addr =BOOT_CFG_ADDR;addr < (BOOT_CFG_ADDR+BOOT_CFG_SZIE);addr += W25X16_SEC_SIZE )
			dev_spi_flash_sector_erase(addr);
		addr = BOOT_CFG_ADDR;
	}


	//save now 
	puts ("Writing to Flash... ");
	
	tagPtr = (uint32 *)(&environment[CONFIG_ENV_SIZE-1-4]);
	*tagPtr = BOOTARGS_END_TAGS;


	
	rc =  dev_spi_flash_write(environment, addr, CONFIG_ENV_SIZE);

	if (rc != 0) {
		puts ("failed\n");
	} else {
		puts ("done\n");
	}

	return rc;
	#endif
	
	return -1;
}


