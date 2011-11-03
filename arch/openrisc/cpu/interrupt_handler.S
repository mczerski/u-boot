/*
 * OpenRISC interrupt handling function
*
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

/*
 * This function is to be called from the exception handler ONLY! It
 * relies on the ability to trash all registers due to them having been
 * saved on entry to the interrupt exception.
 *
 * This function should  be installed in the exception handler's
 * user interrupt exception function slot at reset.
 */

#include <asm/spr-defs.h>

	.data
	.align 4

	.global _interrupt_handler_table
_interrupt_handler_table:
	.rept 32
	.long 0
	.endr

	.global _interrupt_handler_data_ptr_table
_interrupt_handler_data_ptr_table:
	.rept 32
	.long 0
	.endr

/*
 * Function to call appropriate interrupt handler
 */

	.section .text
	.global	_interrupt_handler
        .type	_interrupt_handler,@function

_interrupt_handler:
	/* Make room on stack, save link register */
	l.addi	r1, r1, -12
	l.sw	0(r1), r9

	/* Read PICSR */
	l.mfspr	r3, r0, SPR_PICSR

	/* Load handler table base address */
	l.movhi	r7, hi(_interrupt_handler_table)
	l.ori	r7, r7, lo(_interrupt_handler_table)
	/* Load data pointer table base address */
	l.movhi	r12, hi(_interrupt_handler_data_ptr_table)
	l.ori	r12, r12, lo(_interrupt_handler_data_ptr_table)

.L0:
	/* Find first set bit in PICSR */
	l.ff1	r4, r3
	/* Any bits set? */
	l.sfne	r4, r0
	/* If none, finish */
	l.bnf	.L2
	 l.nop
	/* What is IRQ function table offset? */
	l.addi	r5, r4, -1
	l.slli	r6, r5, 2
	/* Add this to table bases */
	l.add	r6, r6, r7
	l.add	r13, r6, r12

	/* Fetch handler function address */
	l.lwz	r6, 0(r6)

	/* Check that function address != null */
	l.sfnei	r6, 0x0
	/* Skip if no handler: TODO: Indicate interrupt fired but no handler*/
	l.bnf	.L1
	 l.nop

	/* Pull out data pointer from table, save r3, we'll write over it */
	l.sw	4(r1), r3
	l.lwz	r3, 0(r13)
	/* Call handler, save r5 in delay slot */
	l.jalr	r5
	 l.sw	8(r1), r5

	/* Reload r3,r5 */
	l.lwz	r3, 4(r1)
	l.lwz	r5, 8(r1)
.L1:
	/* Clear bit from PICSR, return to start of checking loop */
	l.ori	r6, r0, 1
	l.sll	r6, r6, r5
	l.j	.L0
	 l.xor	r3, r3, r6

.L2:
	/* Finish up - write PICSR back, restore r9*/
	l.lwz	r9, 0(r1)
	l.mtspr	r0, r3, SPR_PICSR
	l.jr	r9
	 l.addi	r1, r1, 12
