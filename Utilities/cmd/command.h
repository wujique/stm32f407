
#ifndef __COMMAND_H_
#define __COMMAND_H_

/*包含系统平台的定义，主要是变量类型*/
#include "stm32f4xx.h"

#define printf cmd_uart_printf

#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_SYS_CBSIZE		256		      /* console I/O buffer  */

#define CONFIG_ENV_SIZE			512
#define	ENV_SIZE				CONFIG_ENV_SIZE

#define CMD_DATA_SIZE	1

#define CONFIG_CMD_BOOTD	1
#define CONFIG_CMD_RUN		1

#define	CONFIG_BOOTARGS		"This is Test String"
	
#define	CONFIG_BOOTCOMMAND	"Run your owen scripts"

#ifndef CONFIG_SYS_HELP_CMD_WIDTH
#define CONFIG_SYS_HELP_CMD_WIDTH	8
#endif

#define CONFIG_SYS_LONGHELP             1
#define CONFIG_SYS_MAXARGS	12

#define CONFIG_SYS_PROMPT		"cmd> "	      /* prompt string	     */
#define CONFIG_AUTO_COMPLETE	1

/*
	cmd定义
*/
struct cmd_tbl_s {
	char	*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
	/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *, int, int, char *[]);
	char	*usage;		/* Usage message	(short)	*/

#ifdef	CONFIG_SYS_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif

#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char *argv[], char last_char, int maxv, char *cmdv[]);
#endif
};

typedef struct cmd_tbl_s	cmd_tbl_t;

extern cmd_tbl_t  *__u_boot_cmd_start;
extern cmd_tbl_t  *__u_boot_cmd_end;
/*
	将命令结构体定义到指定FLASH位置，并且连续放置，
	不同的编译器定义不一样
*/ 
/*  IAR定义，要在link文件内定义.cmd段*/
#if 0
#define REGISTER_CMD(name,maxargs,rep,cmd,usage,help) \
      __root  const cmd_tbl_t cmd_##name @ ".cmd" =  {#name, maxargs, rep, cmd, usage,help}
#endif
	  
#define REGISTER_CMD(name,maxargs,rep,cmd,usage,help) \
			const cmd_tbl_t strcmd_##name __attribute__ ((section ("cmd"))) =  {#name, maxargs, rep, cmd, usage,help}
		
/*Try located Table to .cmd section  */
//#define Struct_Section  __attribute__ ((unused,section (".u_boot_cmd")))

//	#pragma location=".cmd"	

  /*
   * Command Flags:
  */ 
#define CMD_FLAG_REPEAT		0x0001	/* repeat last command		*/
#define CMD_FLAG_BOOTD		0x0002	/* command is from bootd	*/


int cmd_usage(cmd_tbl_t *cmdtp);
cmd_tbl_t *find_cmd(const char *cmd);
cmd_tbl_t *find_cmd_tbl (const char *cmd, cmd_tbl_t *table, int table_len);               
void install_auto_complete(void);
int env_complete(char *var, int maxv, char *cmdv[], int bufsz, char *buf);
int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
long simple_strtol(const char *cp,char **endp,unsigned int base);

#endif
