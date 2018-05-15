/**
 * @file            cmd_sys.c
 * @brief           系统命令定义
 * @author          wujique
 * @date            2018年1月8日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月8日 星期一
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#include <command.h>
#include "console.h"

#define DEBUGF(fmt,args...)

#define __REG(x)	(*((volatile uint32 *)(x)))
#define __REG16(x)	(*((volatile uint16 *)(x)))
#define __REG8(x)	(*((volatile uint8 *)(x)))

int do_wtd_test ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int wtd_feed = 0;	

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}	
	if(strncmp(argv[1],"feedon",6) == 0)
	{
		printf ("watch dog init  feedon !\n");
		wtd_feed = 1;
		//wtd_init();
		while(1)
		{
			if (ctrlc()) {
				putc ('\n');
				return 1;
			}
			Delay(10);
		}
	}	
	return 0;
}


REGISTER_CMD(
	wtdtest,2,	1,do_wtd_test,
	"test watch_dog",
	"[feedon|feedoff|stopsysfd|stopplus] "
);


#define DAY_SECOND		(24*60*60)
#define HOUR_SECOND		(60*60)
#define MINI_SECOND		(60)

#define SYSTEM_TICK_HZ 100


typedef long long  u64;


int do_system_info( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uint32 ticks = Time_Get_LocalTime();//OS_CPU_SysUpTime();//get_system_ticks();
	uint32 temp;
	
	unsigned int day=0,hour=0,min = 0,tmp=0;
	printf("COMPILE DATE: %s - %s \r\n",__DATE__,__TIME__);
	printf("TICKS FREQ: %d \r\n",SYSTEM_TICK_HZ);
	printf("SYSTEM TCKS: %d  \r\n",ticks);
	ticks = ticks/1000;
	printf("SYSTEM UPTIME: %d seconds \r\n",ticks);

	day = (ticks/DAY_SECOND);
	ticks =(ticks%DAY_SECOND);

	tmp = (unsigned int)ticks;
	hour = tmp/HOUR_SECOND;
	tmp = tmp%HOUR_SECOND;

	min = tmp/MINI_SECOND;
	tmp = tmp%MINI_SECOND;

	printf("SYSTEM UPTIME:%d:DAYS %dH:%dM:%dS\r\n",(unsigned int)day,
			hour,min, tmp);

	printf("IMAGE VERSION: %s \r\n", "0.1.1");


	temp = mcu_tempreate_get_tempreate();
	printf("CPU TEMPREATE: %d \r\n", temp);
	
	return 0;
}



REGISTER_CMD(
	systeminfo,2,1,do_system_info,
	"systeminfo",
	"\t display system info "
);


int do_disable_irq( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

	return 0;
}

REGISTER_CMD(
	disableirq,2,1,do_disable_irq,
	"disableirq",
	"\t disable system interrupts  "
);


void read_register_by_addr(unsigned int addr,int data_wd,int n)
{
	int i = 0;
	for(i=0;i<n;i++)
	{
		switch(data_wd)
		{
			case 4:
				printf("ADDR: 0X%08X  VALUE: 0X%08X \n",(addr+data_wd*i),
					__REG(addr+data_wd*i ));
				break;
			case 2:
				printf("ADDR: 0X%08X  VALUE: 0X%04X \n",(addr+data_wd*i),
					__REG16(addr+data_wd*i ));
				break;
			case 1:
				printf("ADDR: 0X%08X  VALUE: 0X%04X \n",(addr+data_wd*i),
					__REG8(addr+data_wd*i ));
				break;
			default:
				printf("==== ERROR====\n");
		}
	}
}


void write_register_by_addr(unsigned int addr,int data_wd,unsigned int val)
{
		DEBUGF("WADDR: 0X%08X  RVALUE: 0X%08X \n",addr,val);
		switch(data_wd)
		{
			case 4:
				__REG(addr)=val;
				break;
			case 2:
				__REG16(addr)=val;
				break;
			case 1:
				__REG8(addr)=val;
				break;
			default:
				printf("==== ERROR====\n");
		}

}

int do_dump_regs ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *cmd,*s;
	int cnt,wd;
	unsigned int addr=0,val=0;;
	
	cmd = argv[0];
	DEBUGF("COMMAD %s \n",cmd);
	
	s = (char *)strchr(cmd, '.');
	DEBUGF("COMMAD %s \n",s);	
	
	if(!s || !strcmp(s, ".read"))
	{
		if (argc < 3) {
			cmd_usage(cmdtp);
			return 1;
			}
		DEBUGF("FILE:[%s] LINE: %d \n",__FILE__,__LINE__);
		addr = strtoul(argv[1], NULL, 16);
		wd = strtoul(argv[2], NULL, 16);
		if(addr%wd !=0)
		{
			printf("Address 0x%X not %d byte%s aligned\n",
				addr,wd, (wd>1)?"s":"");
			return -1;
		}
		read_register_by_addr(addr,wd,1);
	}
	else if(!strcmp(s, ".write"))
	{
		DEBUGF("FILE:[%s] LINE: %d \n",__FILE__,__LINE__);
		if (argc < 4) {
			cmd_usage(cmdtp);
			return 1;
			}
		addr = strtoul(argv[1], NULL, 16);
		wd = strtoul(argv[2], NULL, 16);
		val = strtoul(argv[3], NULL, 16);
		if(addr%wd !=0)
		{
			printf("Address 0x%X not %d byte%s aligned\n",
				addr,wd, (wd>1)?"s":"");
			return -1;
		}
		write_register_by_addr(addr,wd,val);
	}
	else if(!strcmp(s,".inc"))
	{

		if (argc < 4) {
			cmd_usage(cmdtp);
			return 1;
			}
		addr = strtoul(argv[1], NULL, 16);
		wd = strtoul(argv[2], NULL, 16);
		cnt = strtoul(argv[3], NULL, 16);
		if(addr%wd !=0)
		{
			printf("Address 0x%X not %d byte%s aligned\n",
				addr,wd, (wd>1)?"s":"");
			return -1;
		}
		read_register_by_addr(addr,wd,cnt);
	}
	else
	{
			cmd_usage(cmdtp);
			return 1;
	}
	
	return 0;
}


REGISTER_CMD(
	dumpreg,5,	1,do_dump_regs,
	"dump registers vale,HEX VALUE ONLY",
	".read|write|inc addr data_width [value|regs] "
);

