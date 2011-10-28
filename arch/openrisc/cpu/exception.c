#include <asm/system.h>
#include <common.h>
#include <stdio_dev.h>

extern void hang(void);

void exception_hang(int vect, unsigned long addr)
{
	printf("Unhandled exception at 0x%x ", vect&0xff00);
	switch (vect&0xff00) {
	case 0x200:
		puts("(Bus Error)\n");
		break;
	case 0x300:
		puts("(Data Page Fault)\n");
		break;
	case 0x400:
		puts("(Instruction Page Fault)\n");
		break;
	case 0x500:
		puts("(Tick Timer)\n");
		break;
	case 0x600:
		puts("(Alignment)\n");
		break;
	case 0x700:
		puts("(Illegal Instruction)\n");
		break;
	case 0x800:
		puts("(External Interrupt)\n");
		break;
	case 0x900:
		puts("(D-TLB Miss)\n");
		break;
	case 0xA00:
		puts("(I-TLB Miss)\n");
		break;
	case 0xB00:
		puts("(Range)\n");
		break;
	case 0xC00:
		puts("(System Call)\n");
		break;
	case 0xD00:
		puts("(Floating Point)\n");
		break;
	case 0xE00:
		puts("(Trap)\n");
		break;
	default:
		puts("(Unknown exception)\n");
		break;
	}
	printf("EPCR: 0x%08lx\n", addr);
	printf("EEAR: 0x%08lx\n", mfspr (SPR_EEAR_BASE));
	printf("ESR:  0x%08lx\n", mfspr (SPR_ESR_BASE));
	hang();
}
