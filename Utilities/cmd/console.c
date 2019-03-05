

#include <command.h>
#include "console.h"
#include "mcu_uart.h"

int serial_getc (void)
{
	u8 ch,ret;

	while(1)
	{
		ret = mcu_uart_read(PC_PORT, &ch, 1);
		if(0 == ret)
		{
			Delay(1);
		}
		else
			return (int)ch;
	}
	//return getchar();

}


/*
 * Output a single byte to the serial port.
 */
void serial_putc (const char c)
{
	mcu_uart_write(PC_PORT, (u8 *)&c, 1);
	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
}
/*

	检测串口是否有数据

*/
int serial_tstc(void)
{
	return 1;//public_soc_uart_get_present(TERM_PORT);
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

int getc(void)
{
	/* Send directly to the handler */
	return serial_getc();
}

int tstc(void)
{
	/* Send directly to the handler */
	return serial_tstc();
}

void putc(const char c)
{
/* Send directly to the handler */
    serial_putc(c);
}


/* test if ctrl-c was pressed */
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;
int ctrlc(void)
{
	if (!ctrlc_disabled ) {
		if (tstc()) {
			switch (getc()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				return 1;
			default:
				break;
			}
		}
	}
	return 0;
}

/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
int disable_ctrlc(int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}


