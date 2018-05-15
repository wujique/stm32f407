#ifndef __WJQ_CMD_ENV_H

#define __WJQ_CMD_ENV_H 1


unsigned char  env_get_char_memory (int index);
unsigned char  env_get_char (int index);
unsigned char  *env_get_addr (int index);
void env_relocate (void);
int get_default_env_size(void);

#endif

