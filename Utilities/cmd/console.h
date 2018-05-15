#ifndef _CONSOLE_H_
#define _CONSOLE_H_

int serial_getc (void);
void serial_putc (const char c);
int serial_tstc(void);
int getc(void);
int tstc(void);
void putc(const char c);
int ctrlc(void);
int disable_ctrlc(int disable);
int had_ctrlc (void);
void clear_ctrlc(void);

#endif
