/* Startup code for OpenRISC u-boot

   Copyright (C) 2010, Embecosm Limited <info@embecosm.com>

   Contributor Jeremy Bennett       <jeremy.bennett@embecosm.com>
   Contributor Julius Baxter        <julius@opencores.org>
   Contributor Stefan Kristiansson  <stefan.kristiansson@saunalahti.fi>

   Currently for in-ram only (we jump to board_init_r, no flash option yet)	

*/
/* -------------------------------------------------------------------------- */
/* This program is commented throughout in a fashion suitable for processing
   with Doxygen.                                                              */
/* -------------------------------------------------------------------------- */

#include <asm/spr-defs.h>
#include <config.h>	

/* -------------------------------------------------------------------------- */
/*!Macro to load a symbol's address into a register.         
  
  @param[in] gpr  General purpose register to load address into.       
  @param[in] symbol Name of symbol to load.                                   */
/* -------------------------------------------------------------------------- */
#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
	.global symbol ;               \
	l.movhi gpr, hi(symbol) ;      \
	l.ori   gpr, gpr, lo(symbol)

/* -------------------------------------------------------------------------- */
/*!Macro to handle exceptions.                               

  Load NPC into r3, EPCR into r4
                                                                              */
/* -------------------------------------------------------------------------- */
// Size of redzone + size of space required to store state
// This value must match that in _exception_handler.S
#define EXCEPTION_STACK_SIZE (128+128)
	
#define CALL_EXCEPTION_HANDLER            		\
	l.addi  r1, r1, -EXCEPTION_STACK_SIZE;          \
	l.sw    4(r1), r3;             			\
	l.sw    8(r1), r4;             			\
	l.mfspr r3,r0,SPR_NPC;          		\
	l.j 	_exception_handler; 			\
	l.mfspr r4,r0,SPR_EPCR_BASE

/* -------------------------------------------------------------------------- */
/*!Exception vectors                                                          */
/* -------------------------------------------------------------------------- */
	.section .vectors,"ax"
	.org	0x0
	l.nop 0x1
	/* 0x100: RESET exception */
        .org	0x100
	
	/* declare it like so we can call from reset_cpu() */
	.global	__reset
__reset:
	l.movhi r0, 0
	l.movhi r1, 0
	l.movhi r2, 0
	l.movhi r3, 0
	l.movhi r4, 0
	l.movhi r5, 0
	l.movhi r6, 0
	l.movhi r7, 0
	l.movhi r8, 0
	l.movhi r9, 0
	l.movhi r10, 0
	l.movhi r11, 0
	l.movhi r12, 0
	l.movhi r13, 0
	l.movhi r14, 0
	l.movhi r15, 0
#ifndef __OR32_16GPRS__
	l.movhi r16, 0
	l.movhi r17, 0
	l.movhi r18, 0
	l.movhi r19, 0
	l.movhi r20, 0
	l.movhi r21, 0
	l.movhi r22, 0
	l.movhi r23, 0
	l.movhi r24, 0
	l.movhi r25, 0
	l.movhi r26, 0
	l.movhi r27, 0
	l.movhi r28, 0
	l.movhi r29, 0
	l.movhi r30, 0
	l.movhi r31, 0
#endif
	/* Clear status register, set supervisor mode */
	l.ori r1, r0, SPR_SR_SM
	l.mtspr r0, r1, SPR_SR
	/* Clear timer mode register*/
	l.mtspr r0, r0, SPR_TTMR

	/* Relocate u-boot */
	l.movhi	r3,hi(__start)		/* source start address */
	l.ori	r3,r3,lo(__start)
	l.movhi	r4,hi(_stext)		/* dest start address */
	l.ori	r4,r4,lo(_stext)
	l.movhi	r5,hi(__end)		/* dest end address */
	l.ori	r5,r5,lo(__end)
.L_reloc:
	l.lwz	r6,0(r3)
	l.sw	0(r4),r6
	l.addi	r3,r3,4
	l.sfltu	r4,r5
	l.bf	.L_reloc
	l.addi	r4,r4,4			/* delay slot */
	
	/* Jump to board initialisation code */
	LOAD_SYMBOL_2_GPR(r4, _start)
	l.jr    r4
	l.nop

	.org	0x200
	CALL_EXCEPTION_HANDLER

	/* 0x300: Data Page Fault exception */
	.org	0x300
	CALL_EXCEPTION_HANDLER
	
	/* 0x400: Insn Page Fault exception */
	.org	0x400
	CALL_EXCEPTION_HANDLER

	/* 0x500: Timer exception */
	.org	0x500
	CALL_EXCEPTION_HANDLER

	/* 0x600: Aligment exception */
	.org	0x600
	CALL_EXCEPTION_HANDLER
	
	/* 0x700: Illegal insn exception */
	.org	0x700
	CALL_EXCEPTION_HANDLER

	/* 0x800: External interrupt exception */
	.org	0x800
	CALL_EXCEPTION_HANDLER

	/* 0x900: DTLB miss exception */
	.org	0x900
	CALL_EXCEPTION_HANDLER
	
	/* 0xa00: ITLB miss exception */
	.org	0xa00
	CALL_EXCEPTION_HANDLER
	
	/* 0xb00: Range exception */
	.org	0xb00
	CALL_EXCEPTION_HANDLER
	
	/* 0xc00: Syscall exception */
	.org	0xc00
	CALL_EXCEPTION_HANDLER
	
	/* 0xd00: floating point exception */
	.org	0xd00
	CALL_EXCEPTION_HANDLER
	
	/* 0xe00: Trap exception */
	.org	0xe00
	CALL_EXCEPTION_HANDLER
	
	/* 0xf00: Reserved exceptions */
	.org	0xf00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1000
	CALL_EXCEPTION_HANDLER
	
	.org	0x1100
	CALL_EXCEPTION_HANDLER
	
	.org	0x1200
	CALL_EXCEPTION_HANDLER
	
	.org	0x1300
	CALL_EXCEPTION_HANDLER
	
	.org	0x1400
	CALL_EXCEPTION_HANDLER
	
	.org	0x1500
	CALL_EXCEPTION_HANDLER
	
	.org	0x1600
	CALL_EXCEPTION_HANDLER
	
	.org	0x1700
	CALL_EXCEPTION_HANDLER
	
	.org	0x1800
	CALL_EXCEPTION_HANDLER
	
	.org	0x1900
	CALL_EXCEPTION_HANDLER
	
	.org	0x1a00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1b00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1c00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1d00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1e00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1f00
	CALL_EXCEPTION_HANDLER

	/* Pad to the end */
	.org	0x1ffc
	l.nop

 /* Do this in board-specific code init */
/* -------------------------------------------------------------------------- */
/*!Main entry point

   The remaining tasks are then:
   - optionally set up instruction and/or data caches
   - clear BSS
   - jump to the main function
   - loop forever (should never get here)                                     */
/* -------------------------------------------------------------------------- */
	/* The stack grows down from the top of writable memory. */
	.section .data
	.global stack
stack:	.space	4,0	

	.section .text
	.global	_start
        .type	_start,@function


	/* Following externs from board-specific object passed at link time */
	
_start:
	/* Initialise stack and frame pointer (set to same value) */
	l.movhi	r1,hi(CONFIG_SYS_INIT_SP_ADDR)
	l.ori	r1,r1,lo(CONFIG_SYS_INIT_SP_ADDR)
	l.or	r2,r1,r1
	
	/* Initialise cache */
	/* TODO - potentially make this optional for simulation targets to save
	time during startup */
	l.jal 	_cache_init
	l.nop
	
	/* Clear BSS */
.L10:	l.movhi r3,hi(_bss_start)
	l.ori   r3,r3,lo(_bss_start)
	l.movhi r4,hi(_bss_end)
	l.ori   r4,r4,lo(_bss_end)

.L1:    l.sw    (0)(r3),r0
        l.sfltu r3,r4
        l.bf    .L1
        l.addi  r3,r3,4		/* Delay slot */

	/* Jump to main program entry point (argc = argv = envp = 0) */
	l.or	r3,r0,r0
	l.or	r4,r0,r0
	l.jal   board_init
	l.or	r5,r0,r0		/* Delay slot */


	/* Loop forever */
.L3:	l.j	.L3
	l.nop	0x1
	
	.size	_start, .-_start
