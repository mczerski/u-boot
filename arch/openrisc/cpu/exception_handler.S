/*
 * OpenRISC exception handling function.
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

#include <asm/spr-defs.h>

	.data
	.align 4

	.global _exception_handler_table
_exception_handler_table:
	.rept 32
	.long 0
	.endr

/*
 * Function to call appropriate exception handler
 */
	.section .text
	.global	_exception_handler
	.extern	exception_hang
	.type	_exception_handler,@function

_exception_handler:
	/* Store state (r9 already saved)*/
	l.sw	0x00(r1), r2
	l.sw	0x04(r1), r3
	l.sw	0x08(r1), r4
	l.sw	0x0c(r1), r5
	l.sw	0x10(r1), r6
	l.sw	0x14(r1), r7
	l.sw	0x18(r1), r8
	l.sw	0x20(r1), r10
	l.sw	0x24(r1), r11
	l.sw	0x28(r1), r12
	l.sw	0x2c(r1), r13
	l.sw	0x30(r1), r14
	l.sw	0x34(r1), r15
	l.sw	0x38(r1), r16
	l.sw	0x3c(r1), r17
	l.sw	0x40(r1), r18
	l.sw	0x44(r1), r19
	l.sw	0x48(r1), r20
	l.sw	0x4c(r1), r21
	l.sw	0x50(r1), r22
	l.sw	0x54(r1), r23
	l.sw	0x58(r1), r24
	l.sw	0x5c(r1), r25
	l.sw	0x60(r1), r26
	l.sw	0x64(r1), r27
	l.sw	0x68(r1), r28
	l.sw	0x6c(r1), r29
	l.sw	0x70(r1), r30
	l.sw	0x74(r1), r31
	/* Load address from where the exception occurred into r4 */
	l.mfspr r4,r0,SPR_EPCR_BASE
	/* Load Link address into r3 (i.e. vector address) */
	l.or	r3, r0, r9
	/* Determine offset in table of exception handler using r3 */
	l.andi	r13, r3, 0xffff
	l.srli	r13, r13, 6
	/* r13 now contains offset in _exception_handler_table for function */
	/* Get _exception_handler_table address */
	l.movhi r14, hi(_exception_handler_table)
	l.ori	r14, r14, lo(_exception_handler_table)
	/* r14 now contains base of exception handler table */
	l.add	r14, r14, r13
	l.lwz	r13, 0(r14)

	/* Check to see if this handler has been set yet */
	l.sfnei	r13, 0x0
	l.bnf	.L1
	 l.nop
	/* Save return address */
	l.or	r14, r0, r9
	/* Call exception handler, copy EPCR to r3 */
	l.jalr	r13
	 l.or	r3, r4, r4
	/* Load return address */
	l.or	r9, r0, r14

	/* Restore state */
	l.lwz	r2, 0x00(r1)
	l.lwz	r3, 0x04(r1)
	l.lwz	r4, 0x08(r1)
	l.lwz	r5, 0x0c(r1)
	l.lwz	r6, 0x10(r1)
	l.lwz	r7, 0x14(r1)
	l.lwz	r8, 0x18(r1)
	l.lwz	r10, 0x20(r1)
	l.lwz	r11, 0x24(r1)
	l.lwz	r12, 0x28(r1)
	l.lwz	r13, 0x2c(r1)
	l.lwz	r14, 0x30(r1)
	l.lwz	r15, 0x34(r1)
	l.lwz	r16, 0x38(r1)
	l.lwz	r17, 0x3c(r1)
	l.lwz	r18, 0x40(r1)
	l.lwz	r19, 0x44(r1)
	l.lwz	r20, 0x48(r1)
	l.lwz	r21, 0x4c(r1)
	l.lwz	r22, 0x50(r1)
	l.lwz	r23, 0x54(r1)
	l.lwz	r24, 0x58(r1)
	l.lwz	r25, 0x5c(r1)
	l.lwz	r26, 0x60(r1)
	l.lwz	r27, 0x64(r1)
	l.lwz	r28, 0x68(r1)
	l.lwz	r29, 0x6c(r1)
	l.lwz	r30, 0x70(r1)
	l.lwz	r31, 0x74(r1)

	l.jr	r9
	 l.nop
.L1:
	/* Exception handler not set, loop */
	l.jal	exception_hang
	 l.nop	0x0
