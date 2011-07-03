#include <common.h>
#include <stdio_dev.h>

extern void hang(void);

void
exception_hang(int vect, unsigned long addr)
{
	puts("### ERROR ### Unhandled exception ###\n");
	printf("### VECTOR### 0x%x                  ###\n",vect&0xff00);
	printf("### ECPR  ### 0x%08x              ###\n",addr);
	hang();

}
