
#include <command.h>
#include "console.h"

#include <spiffs.h>

int cmd_spiffs_ls( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	sys_spiffs_ls();
	return 0;
}

REGISTER_CMD(
	spiffsls, 2 , 1, cmd_spiffs_ls,
	"spiffsls",
	"\t ls spiffs file "
);

int cmd_spiffs_format( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	sys_spiffs_format();
	return 0;
}

REGISTER_CMD(
	spiffsformat, 2 , 1, cmd_spiffs_format,
	"format spiffs",
	"\t ls spiffs file "
);


