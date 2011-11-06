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

#include <asm/types.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/openrisc_exc.h>
#include <common.h>

extern void _interrupt_handler(void);

extern unsigned long _interrupt_handler_table;
extern unsigned long _interrupt_handler_data_ptr_table;

int interrupt_init(void)
{
	/* install handler for external interrupt exception */
	exception_install_handler(8, _interrupt_handler);
	/* Enable interrupts in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);

	return 0;
}

void enable_interrupts(void)
{
	/* Set interrupt enable bit in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);
	/* Enable timer exception */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);
}

int disable_interrupts(void)
{
	/* Clear interrupt enable bit in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_IEE);
	/* Disable timer exception */
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
	return 0;
}

void irq_install_handler(int irq, interrupt_handler_t *handler, void *arg)
{
	ulong *handler_table = &_interrupt_handler_table;
	ulong *arg_table = &_interrupt_handler_data_ptr_table;

	if (irq < 0 || irq > 31)
		return;

	handler_table[irq] = (ulong)handler;
	arg_table[irq] = (ulong)arg;
}

void irq_free_handler(int irq)
{
	ulong *handler_table = &_interrupt_handler_table;
	ulong *arg_table = &_interrupt_handler_data_ptr_table;

	if (irq < 0 || irq > 31)
		return;

	handler_table[irq] = 0;
	arg_table[irq] = 0;
}

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	ulong *handler, *arg;

	handler = &_interrupt_handler_table;
	arg = &_interrupt_handler_data_ptr_table;

	printf("\nInterrupt-Information:\n\n");
	printf("Nr  Routine   Arg\n");
	printf("-----------------\n");

	for (i=0; i<32; i++) {
		if (handler[i])
			printf("%02d  %08lx  %08lx\n", i,
				(ulong)handler[i],
				(ulong)arg[i]);
		else
			printf("%02d  Not set   Not set\n", i);

	}
	printf("\n");

	return (0);
}
#endif
