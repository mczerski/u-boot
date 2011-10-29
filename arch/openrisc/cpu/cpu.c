/*
 * (C) Copyright 2011, Stefan Kristiansson  <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/system.h>
#include <common.h>

int checkcpu(void)
{
	puts("CPU:    OpenRISC\n");
	printf("DCACHE: %d bytes\n", checkdcache());
	printf("ICACHE: %d bytes\n", checkicache());
	/* TODO - add more info here */
	return 0;
}

int cpu_init(void)
{
	return 0;
}

int cleanup_before_linux(void)
{
	disable_interrupts();
	return 0;
}

extern void __reset(void);

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	__reset();
	return 0;
}

void flush_dcache(unsigned long addr, unsigned long size)
{
	unsigned long target = addr + size;

	while (addr < target)
	{
		mtspr(SPR_DCBFR, addr);
		addr +=4;
	}
}

void flush_dcache_range(unsigned long addr, unsigned long stop)
{
	while (addr < stop)
	{
		mtspr(SPR_DCBFR, addr);
		addr +=4;
	}
}

void invalidate_dcache_range(unsigned long addr, unsigned long stop)
{
	while (addr < stop)
	{
		mtspr(SPR_DCBIR, addr);
		addr +=4;
	}
}

void flush_icache(unsigned long addr, unsigned long size)
{
	unsigned long target = addr + size;

	while (addr < target)
	{
		mtspr(SPR_ICBIR, addr);
		addr +=4;
	}
}

void flush_cache(unsigned long addr, unsigned long size)
{
	flush_dcache(addr, size);
	flush_icache(addr, size);
}

int icache_status(void)
{
	return (mfspr(SPR_SR) & SPR_SR_ICE);
}

int checkicache(void)
{
	unsigned long iccfgr;
	unsigned long cache_set_size;
	unsigned long cache_ways;
	unsigned long cache_block_size;

	iccfgr = mfspr(SPR_ICCFGR);
	cache_ways = 1 << (iccfgr & SPR_ICCFGR_NCW);
	cache_set_size = 1 << ((iccfgr & SPR_ICCFGR_NCS) >> 3);
	cache_block_size = 16 << ((iccfgr & SPR_ICCFGR_CBS) >> 7);

	return (cache_set_size * cache_ways * cache_block_size);
}

int dcache_status(void)
{
	return (mfspr(SPR_SR) & SPR_SR_DCE);
}

int checkdcache(void)
{
	unsigned long dccfgr;
	unsigned long cache_set_size;
	unsigned long cache_ways;
	unsigned long cache_block_size;

	dccfgr = mfspr(SPR_DCCFGR);
	cache_ways = 1 << (dccfgr & SPR_DCCFGR_NCW);
	cache_set_size = 1 << ((dccfgr & SPR_DCCFGR_NCS) >> 3);
	cache_block_size = 16 << ((dccfgr & SPR_DCCFGR_CBS) >> 7);

	return cache_set_size * cache_ways * cache_block_size;
}

int cpu_eth_init(bd_t *bis)
{
	return 0;
}
