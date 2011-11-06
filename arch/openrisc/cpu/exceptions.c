/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
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
#include <stdio_dev.h>

extern void hang(void);
extern unsigned long _exception_handler_table;

void exception_install_handler(int exception, void (*handler)(void))
{
	ulong *handler_table = &_exception_handler_table;

	if (exception < 0 || exception > 31)
		return;

	handler_table[exception] = (ulong)handler;
}

void exception_free_handler(int exception)
{
	ulong *handler_table = &_exception_handler_table;

	if (exception < 0 || exception > 31)
		return;

	handler_table[exception] = 0;
}

void exception_hang(int vect, unsigned long addr)
{
	printf("Unhandled exception at 0x%x ", vect&0xff00);
	switch (vect&0xff00) {
	case 0x100:
		puts("(Reset)\n");
		break;
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
