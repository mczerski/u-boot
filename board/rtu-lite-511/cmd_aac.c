#include <common.h>
#include <command.h>

#define SHORT_HELP\
	"aac    - aac test programs\n"

#define LONG_HELP\
	"\n"\
	"aac test\n"\
	"    - Test\n"

int do_aac (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("DEBUG: do_aac stub\n");
	return 0;
}
U_BOOT_CMD( aac, 1, 0, do_aac, SHORT_HELP, LONG_HELP );
