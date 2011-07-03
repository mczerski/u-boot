/*
	exception_handler.S. OR1K exception handling function.

	Contributor Julius Baxter  <julius@opencores.org>

*/


#include <asm/spr-defs.h>

// Warning - this must be the same as specified in start.S	
#define EXCEPTION_STACK_SIZE (128+128)

// Value handler addresses are initialised to
#define EXCEPTION_HANDLER_NOT_SET -1
	
	.data
	.align 4
	.global _exception_handler_table
_exception_handler_table:
_exception_handler_buserr:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_dpfault:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_ipfault:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_tick:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_align:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_illegal:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_interrupt:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_dtlbmiss:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_itlbmiss:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_range:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_systemcall:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_float:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_trap:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved0:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved1:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved2:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved3:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved4:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved5:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved6:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved7:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved8:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved9:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved10:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved11:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved12:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved13:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved14:	.long	EXCEPTION_HANDLER_NOT_SET
_exception_handler_reserved15:	.long	EXCEPTION_HANDLER_NOT_SET
	
/* -------------------------------------------------------------------------- */
/*!Function to call appropriate exception handler            
                                                                              */
/* -------------------------------------------------------------------------- */
	
	.section .text
	.global	_exception_handler
	.extern	exception_hang
        .type	_exception_handler,@function

	/*
	r3 = address of exception vector
	r4 = address where exception occurred
	*/

_exception_handler:
	/* Store remainder of state (r3,r4 stored in vector entry)*/
	l.sw    0x00(r1), r2
	l.sw    0x0c(r1), r5
	l.sw    0x10(r1), r6
	l.sw    0x14(r1), r7
	l.sw    0x18(r1), r8
	l.sw    0x1c(r1), r9
	l.sw    0x20(r1), r10
	l.sw    0x24(r1), r11
	l.sw    0x28(r1), r12
	l.sw    0x2c(r1), r13
	l.sw    0x30(r1), r14
	l.sw    0x34(r1), r15
#ifndef __OR32_16GPRS__	
	l.sw    0x38(r1), r16
	l.sw    0x3c(r1), r17
	l.sw    0x40(r1), r18
	l.sw    0x44(r1), r19
	l.sw    0x48(r1), r20
	l.sw    0x4c(r1), r21
	l.sw    0x50(r1), r22
	l.sw    0x54(r1), r23
	l.sw    0x58(r1), r24
	l.sw    0x5c(r1), r25
	l.sw    0x60(r1), r26
	l.sw    0x64(r1), r27
	l.sw    0x68(r1), r28
	l.sw    0x6c(r1), r29
	l.sw    0x70(r1), r30
	l.sw    0x74(r1), r31
#endif	
	/* Determine offset in table of exception handler using r3*/
	l.andi	r13,r3,0xffff
	l.srli	r13,r13,6
	/* Substract 2 words, as we have no vector at 0 and no reset handler */
	l.addi	r13,r13,-8
	/* r13 now contains offset in _exception_handler_table for function */
	/* Get _exception_handler_table address */
	l.movhi r14,hi(_exception_handler_table)
	l.ori	r14,r14,lo(_exception_handler_table)
	/* r14 now contains base of exception handler table */
	l.add	r14,r14,r13
	l.lwz	r13, 0(r14)

	/* Check to see if this handler has been set yet */
	l.movhi r15,hi(EXCEPTION_HANDLER_NOT_SET)
	l.ori 	r15,r15,lo(EXCEPTION_HANDLER_NOT_SET)
	l.sfne	r13,r15
	l.bnf	.L1
	l.nop
	
	/* Call exception handler, copy EPCR to r3 */
	l.jalr	r13
	l.or	r3,r4,r4

	/* Restore state */
	l.lwz    r2, 0x00(r1)
	l.lwz    r3, 0x04(r1)
	l.lwz    r4, 0x08(r1)
	l.lwz    r5, 0x0c(r1)
	l.lwz    r6, 0x10(r1)
	l.lwz    r7, 0x14(r1)
	l.lwz    r8, 0x18(r1)
	l.lwz    r9, 0x1c(r1)
	l.lwz    r10, 0x20(r1)
	l.lwz    r11, 0x24(r1)
	l.lwz    r12, 0x28(r1)
	l.lwz    r13, 0x2c(r1)
	l.lwz    r14, 0x30(r1)
	l.lwz    r15, 0x34(r1)
#ifndef __OR32_16GPRS__	
	l.lwz    r16, 0x38(r1)
	l.lwz    r17, 0x3c(r1)
	l.lwz    r18, 0x40(r1)
	l.lwz    r19, 0x44(r1)
	l.lwz    r20, 0x48(r1)
	l.lwz    r21, 0x4c(r1)
	l.lwz    r22, 0x50(r1)
	l.lwz    r23, 0x54(r1)
	l.lwz    r24, 0x58(r1)
	l.lwz    r25, 0x5c(r1)
	l.lwz    r26, 0x60(r1)
	l.lwz    r27, 0x64(r1)
	l.lwz    r28, 0x68(r1)
	l.lwz    r29, 0x6c(r1)
	l.lwz    r30, 0x70(r1)
	l.lwz    r31, 0x74(r1)
#endif
	l.addi  r1, r1, EXCEPTION_STACK_SIZE

	l.rfe
	l.nop

.L1:
	/* Exception handler not set, loop */
	l.jal	exception_hang
	l.nop	0x0

	
/* -------------------------------------------------------------------------- */
/*!Function to add handler to table
                                                                              */
/* -------------------------------------------------------------------------- */
	.global	_exception_handler_add
        .type	_exception_handler_add,@function

	/* r3 should have exception number (2 for buserr, 5 for tick, etc.) */
	/* r4 should have handler function address */
_exception_handler_add:
	l.addi 	r1,r1,-4
	l.sw	0(r1),r5
	/* Convert exception number into word address */
	l.slli	r3,r3,2
	l.addi	r3,r3,-8
	/* Get address of exception handler table */
	l.movhi r5,hi(_exception_handler_table)
	l.ori	r5,r5,lo(_exception_handler_table)
	/* Add handler offset to table base */
	l.add	r5,r5,r3
	/* Store handler function address */
	l.sw 	0(r5),r4
	/* Restore r5 */
	l.lwz	r5,0(r1)
	/* Return via link register */
	l.jr	r9
	/* Restore stack value */
	l.addi	r1,r1,4
	