/*
 *	interrupts.S. OpenRISC interrupt functions
 *
 *	Contributor Julius Baxter  <julius@opencores.org>
 */

/*
 *	This function is to be called from the exception handler ONLY! It
 *	relies on the ability to trash all registers due to them having been
 *	saved on entry to the interrupt exception.
 *
 *	This function should  be installed in the exception handler's
 *	user interrupt exception function slot at reset.
 */

#include <asm/spr-defs.h>

#define INTERRUPT_HANDLER_NOT_SET -1

	.data
	.align 4
	.global _interrupt_handler_table
_interrupt_handler_table:
_interrupt_handler0:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler1:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler2:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler3:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler4:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler5:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler6:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler7:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler8:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler9:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler10:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler11:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler12:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler13:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler14:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler15:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler16:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler17:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler18:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler19:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler20:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler21:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler22:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler23:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler24:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler25:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler26:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler27:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler28:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler29:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler30:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler31:	.long	INTERRUPT_HANDLER_NOT_SET

	.global _interrupt_handler_data_ptr_table
_interrupt_handler_data_ptr_table:
_interrupt_handler_data_ptr0:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr1:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr2:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr3:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr4:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr5:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr6:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr7:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr8:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr9:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr10:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr11:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr12:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr13:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr14:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr15:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr16:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr17:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr18:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr19:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr20:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr21:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr22:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr23:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr24:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr25:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr26:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr27:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr28:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr29:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr30:	.long	INTERRUPT_HANDLER_NOT_SET
_interrupt_handler_data_ptr31:	.long	INTERRUPT_HANDLER_NOT_SET



/*
 * Function to call appropriate interrupt handler
 */

	.section .text
	.global	_interrupt_handler
        .type	_interrupt_handler,@function

_interrupt_handler:
	/* Make room on stack, save link register */
	l.addi	r1,r1,-12
	l.sw	0(r1),r9

	/* Read PICSR */
	l.mfspr	r3,r0,SPR_PICSR

	/* Load handler table base address */
	l.movhi r7,hi(_interrupt_handler_table)
	l.ori	r7,r7,lo(_interrupt_handler_table)
	/* Check to see if this handler has been set yet */
	l.movhi r8,hi(INTERRUPT_HANDLER_NOT_SET)
	l.ori 	r8,r8,lo(INTERRUPT_HANDLER_NOT_SET)
	/* Load data pointer table base address */
	l.movhi r12,hi(_interrupt_handler_data_ptr_table)
	l.ori	r12,r12,lo(_interrupt_handler_data_ptr_table)

.L0:
	/* Find first set bit in PICSR */
	l.ff1	r4,r3
	/* Any bits set? */
	l.sfne	r4,r0
	/* If none, finish */
	l.bnf	.L2
	l.nop
	/* What is IRQ function table offset? */
	l.addi	r5,r4,-1
	l.slli	r6,r5,2
	/* Add this to table bases */
	l.add	r6,r6,r7
	l.add	r13,r6,r12

	/* Fetch handler function address */
	l.lwz	r6,0(r6)

	/* Double check it's valid, compare against INTERRUPT_HANDLER_NOT_SET */
	l.sfne	r6,r8
	/* Skip if no handler: TODO: Indicate interrupt fired but no handler*/
	l.bnf .L1
	l.nop

	/* Pull out data pointer from table, save r3, we'll write over it */
	l.sw	4(r1),r3
	l.lwz	r3,0(r13)
	/* Call handler, save r5 in delay slot */
	l.jalr	r5
	l.sw	8(r1),r5

	/* Reload r3,r5 */
	l.lwz	r3,4(r1)
	l.lwz	r5,8(r1)
.L1:
	/* Clear bit from PICSR, return to start of checking loop */
	l.ori	r6,r0,1
	l.sll	r6,r6,r5
	l.j	.L0
	l.xor	r3,r3,r6

.L2:
	/* Finish up - write PICSR back, restore r9*/
	l.lwz	r9,0(r1)
	l.mtspr	r0,r3,SPR_PICSR
	l.jr	r9
	l.addi	r1,r1,12



/*
 * Function to add handler to table
 */
	.global	irq_install_handler
        .type	irq_install_handler,@function

	/* r3 should have IRQ line for peripheral */
	/* r4 should have handler function address */
	/* r5 should have data pointer */
irq_install_handler:
	l.addi 	r1,r1,-4
	l.sw	0(r1),r6
	/* Convert interrupt number into word address */
	l.slli	r3,r3,2
	l.addi	r3,r3,-4
	/* Get address of interrupt handler table */
	l.movhi r6,hi(_interrupt_handler_table)
	l.ori	r6,r6,lo(_interrupt_handler_table)
	/* Add handler offset to table base */
	l.add	r6,r6,r3
	/* Store handler function address */
	l.sw 	0(r6),r4
	/* Get address of interrupt handler data ptr table */
	l.movhi r6,hi(_interrupt_handler_data_ptr_table)
	l.ori	r6,r6,lo(_interrupt_handler_data_ptr_table)
	/* Add handler offset to table base */
	l.add	r6,r6,r3
	/* Store handler data pointer address */
	l.sw 	0(r6),r5
	/* Restore r6 */
	l.lwz	r6,0(r1)
	/* Return via link register */
	l.jr	r9
	/* Restore stack value */
	l.addi	r1,r1,4

	.global	irq_free_handler
        .type	irq_free_handler,@function

	/* r3 should have IRQ line for peripheral */
irq_free_handler:
	l.addi 	r1,r1,-8
	l.sw	0(r1),r6
	l.sw	4(r1),r4
	/* Convert interrupt number into word address */
	l.slli	r3,r3,2
	l.addi	r3,r3,-4
	/* Get address of interrupt handler table */
	l.movhi r6,hi(_interrupt_handler_table)
	l.ori	r6,r6,lo(_interrupt_handler_table)
	/* Add handler offset to table base */
	l.add	r6,r6,r3
	/* Prepare reg with not-set value */
	l.movhi	r4,hi(INTERRUPT_HANDLER_NOT_SET)
	l.ori	r4,r4,lo(INTERRUPT_HANDLER_NOT_SET)
	/* Store at handler function address */
	l.sw 	0(r6),r4
	/* Restore r4,r6 */
	l.lwz	r6,0(r1)
	l.lwz	r4,4(r1)
	/* Return via link register */
	l.jr	r9
	/* Restore stack value */
	l.addi	r1,r1,8
