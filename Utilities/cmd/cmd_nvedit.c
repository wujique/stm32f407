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

/**************************************************************************
 *
 * Support for persistent environment data
 *
 * The "environment" is stored as a list of '\0' terminated
 * "name=value" strings. The end of the list is marked by a double
 * '\0'. New entries are always added at the end. Deleting an entry
 * shifts the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old value and adding the new one.
 *
 * The environment is preceeded by a 32 bit CRC over the data part.
 *
 **************************************************************************
 */
#include <command.h>
#include "console.h"
#include "cmd_env.h"


typedef unsigned char uchar;
typedef unsigned long ulong;

#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

/************************************************************************
************************************************************************/

int envmatch (uchar *s1, int i2);

/*
 * This variable is incremented on each do_setenv (), so it can
 * be used via get_env_id() as an indication, if the environment
 * has changed or not. So it is possible to reread an environment
 * variable only if the environment was changed ... done so for
 * example in NetInitLoop()
 */


static int env_id = 1;

int get_env_id (void)
{
	return env_id;
}



/************************************************************************
 * Set a new environment variable,
 * or replace or delete an existing one.
 *
 * This function will ONLY work with a in-RAM copy of the environment
 */

int _do_setenv (int flag, int argc, char *argv[])
{
	int   i, len, oldval;
	uchar *env, *nxt = NULL;
	char *name;


	uchar *env_data = (uchar *) env_get_addr(0);

	if (!env_data)	/* need copy in RAM */
		return 1;

	name = argv[1];

	if (strchr(name, '=')) {
		printf ("## Error: illegal character '=' in variable name \"%s\"\n", name);
		return 1;
	}

	env_id++;
	/*
	 * search if variable with this name already exists
	 */
	oldval = -1;
	for (env=env_data; *env; env=nxt+1) {
		for (nxt=env; *nxt; ++nxt)
			;
		if ((oldval = envmatch((uchar *)name, env-env_data)) >= 0)
			break;
	}

	/* Delete only ? */
	if ((argc < 3) || argv[2] == NULL) {
		//env_crc_update ();
		return 0;
	}

	/*
	 * Append new definition at the end
	 */
	 
	for (env=env_data; *env || *(env+1); ++env)	;
	
	if (env > env_data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > ENV_SIZE - (env-env_data)
	 */
	len = strlen(name) + 2;
	/* add '=' for first arg, ' ' for all others */
	for (i=2; i<argc; ++i) {
		len += strlen(argv[i]) + 1;
	}
	if (len > (&env_data[ENV_SIZE]-env)) {
		printf ("## Error: environment overflow, \"%s\" deleted\n", name);
		return 1;
	}
	
	while ((*env = *name++) != '\0')
		env++;
	for (i=2; i<argc; ++i) {
		char *val = argv[i];

		*env = (i==2) ? '=' : ' ';
		while ((*++env = *val++) != '\0')
			;
	}

	/* end is marked with double '\0' */
	*++env = '\0';

	/* Update CRC */
//	env_crc_update ();

	/*
	 * Some variables should be updated when the corresponding
	 * entry in the enviornment is changed
	 */

	if (strcmp(argv[1],"ethaddr") == 0)
		return 0;

	

#if defined(CONFIG_CMD_NET)
	if (strcmp(argv[1],"bootfile") == 0) {
		copy_filename (BootFile, argv[2], sizeof(BootFile));
		return 0;
	}
#endif

	return 0;
}








int setenv (char *varname, char *varvalue)
{
	char *argv[4] = { "setenv", varname, varvalue, NULL };
	if (varvalue == NULL)
		return _do_setenv (0, 2, argv);
	else
		return _do_setenv (0, 3, argv);
}


/************************************************************************
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */

char *cmd_getenv (char *name)
{
	int i, nxt;

//	WATCHDOG_RESET();

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CONFIG_ENV_SIZE) {
				return (NULL);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;
		return ((char *)env_get_addr(val));
	}

	return (NULL);
}

int getenv_r (char *name, char *buf, unsigned len)
{
	int i, nxt;

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val, n;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CONFIG_ENV_SIZE) {
				return (-1);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;
		/* found; copy out */
		n = 0;
		while ((len > n++) && (*buf++ = env_get_char(val++)) != '\0')
			;
		if (len == n)
			*buf = '\0';
		return (n);
	}
	return (-1);
}






/************************************************************************
 * Command interface: print one or all environment variables
 */

/*
 * state 0: finish printing this string and return (matched!)
 * state 1: no matching to be done; print everything
 * state 2: continue searching for matched name
 */
static int printenv(char *name, int state)
{
	int i, j;
	char c, buf[17];

	i = 0;
	buf[16] = '\0';

	while (state && env_get_char(i) != '\0') 
	{
		if (state == 2 && envmatch((uchar *)name, i) >= 0)
			state = 0;

		j = 0;
		do
		{
			buf[j++] = c = env_get_char(i++);
			if(j == sizeof(buf) - 1)
			{
				if (state <= 1)
					puts(buf);
				j = 0;
			}
		} while (c != '\0');

		if (state <= 1) 
		{
			if (j)
				puts(buf);
			putc('\n');
		}

		if (ctrlc())
			return -1;
	}

	if (state == 0)
		i = 0;
	return i;
}


/************************************************************************
 * Match a name / name=value pair
 *
 * s1 is either a simple 'name', or a 'name=value' pair.
 * i2 is the environment index for a 'name2=value2' pair.
 * If the names match, return the index for the value2, else NULL.
 */

int envmatch (uchar *s1, int i2)
{

	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=')
			return(i2);
	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return(i2);
	return(-1);
}


extern uchar default_environment[];
extern int get_default_env_size();
static int get_default_env_count(void)
{
    int cnt=0;
    int len = get_default_env_size();
    int i;

    //dprintf("default_environment size:%d\n",len);

    for(i = 0,cnt = 0;i < len;i ++)
    {
        if(default_environment[i] == '\0')
        {
            cnt ++;
        }
    }

    //dprintf_line("cnt=%d",cnt);
    return cnt;
}

static int get_default_env(int pos,char * name ,char * value)
{
    int cnt=0;
    int len = get_default_env_size();
    int i;
    char * p1,* p2;

    for(i = 0,cnt = 0;i < len;i ++)
    {
        if(default_environment[i] == '\0')
        {
            if(cnt == pos)
            {
                p1 = (char *)&default_environment[i + 1];
                p2 = (char *)strchr(p1, '=');
                if(p2 == NULL)
                {
                    //dprintf_line();
                    return -1;
                }
                else
                {
                    //dprintf_line("%p %p %d",p2,p1,p2-p1);
                    memcpy(name,p1,p2-p1);
                    strcpy(value,(char * )(p2+1));
                    //dprintf_line("%s %s",name,value);
                    return 0;
                }
            }
            cnt ++;
        }
    }

    //dprintf_line();
    return -2;
}



int recover_env()
{
    int default_env_cnt = 0;
    int i = 0;
    char name[512];
    char value[512];
    int ret;
    char * penv;
    int flag = 0;

    // get default env count
    default_env_cnt = get_default_env_count();

    i = 0;
    while(i < default_env_cnt)
    {
        // get default env name && value
        memset(name,0,sizeof(name));
        memset(value,0,sizeof(value));

        ret = get_default_env(i,name,value);
        //dprintf_line("%s %s",name,value);
        if(ret < 0)
        {
            break;
        }

        penv=cmd_getenv(name);
        //dprintf_line("%s",penv);
        if(penv == NULL)
        {
            //Set env
            setenv(name,value);
            flag ++;
        }
        i ++;
    }

    printf("recover %d environment variables \n",flag);
 //   if(flag)
 //   {
 //       saveenv();
 //   }
    return 0;
}



#ifdef ENABLE_K21_CLI

#include "cmd_env.h"

int do_printenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) 
	{
		/* print all env vars */
		rcode = printenv(NULL, 1);
		if (rcode < 0)
			return 1;
		printf("\nEnvironment size: %d/%ld bytes\n",rcode, (ulong)ENV_SIZE);
		return 0;
	}

	/* print selected env vars */
	for(i = 1; i < argc; ++i) 
	{
		char *name = argv[i];
		if(printenv(name, 2)) 
		{
			printf("## Error: \"%s\" not defined\n", name);
			++rcode;
		}
	}

	return rcode;
}











int do_setenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	return _do_setenv (flag, argc, argv);
}


int do_saveenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern char * env_name_spec;

	printf ("Saving Environment to %s...\n", env_name_spec);

	return 1;
	//return (saveenv() ? 1 : 0);
}

REGISTER_CMD(
	saveenv, 1, 0,	do_saveenv,
	"save environment variables to persistent storage",
	""
);



/**************************************************/

REGISTER_CMD(
	printenv, CONFIG_SYS_MAXARGS, 1,	do_printenv,
	"print environment variables",
	"\n    - print values of all environment variables\r\n"
	"printenv name ...\r\n"
	"    - print value of environment variable 'name'"
);

REGISTER_CMD(
	setenv, CONFIG_SYS_MAXARGS, 0,	do_setenv,
	"set environment variables",
	"name value ...\r\n"
	"    - set environment variable 'name' to 'value ...'\r\n"
	"setenv name\r\n"
	"    - delete environment variable 'name'"
);

#if defined(CONFIG_CMD_ASKENV)

REGISTER_CMD(
	askenv,	CONFIG_SYS_MAXARGS,	1,	do_askenv,
	"get environment variables from stdin",
	"name [message] [size]\r\n"
	"    - get environment variable 'name' from stdin (max 'size' chars)\r\n"
	"askenv name\r\n"
	"    - get environment variable 'name' from stdin\r\n"
	"askenv name size\r\n"
	"    - get environment variable 'name' from stdin (max 'size' chars)\r\n"
	"askenv name [message] size\r\n"
	"    - display 'message' string and get environment variable 'name'"
	"from stdin (max 'size' chars)"
);
#endif

#if defined(CONFIG_CMD_RUN)
int do_run (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
REGISTER_CMD(
	run,	CONFIG_SYS_MAXARGS,	1,	do_run,
	"run commands in an environment variable",
	"var [...]\r\n"
	"    - run the commands in the environment variable(s) 'var'"
);
#endif

//=========================================




int do_recover_env (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 1) {
		cmd_usage(cmdtp);
		return 1;
	}

	return recover_env();
}

REGISTER_CMD(
	recover_env, 1, 0,	do_recover_env,
	"recover environment variables",
	"    - recover environment variables\r\n"
);
#endif



