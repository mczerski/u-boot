#include <asm/system.h>
#include <common.h>

int checkcpu(void)
{
	puts("CPU: OpenRISC\n");
	// TODO - all the info here
	return 0;
}

int cpu_init (void)
{
	return 0;
}

int cleanup_before_linux (void)
{
	disable_interrupts();
	return 0;
}

extern void __reset();

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	__reset();
	return 0;
}


void flush_cache (unsigned long addr, unsigned long size)
{
	unsigned long target = addr + size;
	while(addr < target)
	{
		mtspr(SPR_DCBIR,addr);
		addr +=4;
	}
}

void flush_dcache (unsigned long addr, unsigned long size)
{
	flush_cache(addr,size);
}

void flush_icache (unsigned long addr, unsigned long size)
{
	unsigned long target = addr + size;
	while(addr < target)
	{
		mtspr(SPR_ICBIR,addr);
		addr +=4;
	}
}

int icache_status (void)
{
	return 0;
}

int dcache_status (void)
{
	return 0;
}

int cpu_eth_init(bd_t *bis)
{

	return 0;
}
