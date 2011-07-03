/* _cache.S. OR1K cache initialisation function.

   Copyright (C) 2010, 2011, Authors, ORSoC AB

   Contributor Jonas Bonn	<jonas.bonn@gmail.com>
   Contributor Julius Baxter  	<julius@opencores.org>

   This file is part of Newlib.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <http://www.gnu.org/licenses/>.            */

	
#include <asm/spr-defs.h>

	.section .text
	.global	_cache_init
        .type	_cache_init,@function

_cache_init:
	/*
	This function is to be used ONLY during reset, before main() is called.
	TODO: Perhaps break into individual enable instruction/data cache
	      sections functions, and provide disable functions, also, all
	      callable from C
	
	*/
	
	/* Instruction cache enable */
	/* Check if IC present and skip enabling otherwise */
.L6:	l.mfspr r3,r0,SPR_UPR
	l.andi  r7,r3,SPR_UPR_ICP
	l.sfeq  r7,r0
	l.bf    .L8
	l.nop
	
	/* Disable IC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_ICE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR
	
	/* Establish cache block size
	If BS=0, 16;
	If BS=1, 32;
	r14 contain block size
	*/
	l.mfspr r3,r0,SPR_ICCFGR
	l.andi  r7,r3,SPR_ICCFGR_CBS
	l.srli  r8,r7,7
	l.ori   r4,r0,16
	l.sll   r14,r4,r8
	
	/* Establish number of cache sets
	r10 contains number of cache sets
	r8 contains log(# of cache sets)
	*/
	l.andi  r7,r3,SPR_ICCFGR_NCS
	l.srli  r8,r7,3
	l.ori   r4,r0,1
	l.sll   r10,r4,r8
	
	/* Invalidate IC */
	l.addi  r6,r0,0
	l.sll   r5,r14,r8
	
.L7:	l.mtspr r0,r6,SPR_ICBIR
	l.sfne  r6,r5
	l.bf    .L7
	l.add   r6,r6,r14
	
	/* Enable IC */
	l.mfspr r6,r0,SPR_SR
	l.ori   r6,r6,SPR_SR_ICE
	l.mtspr r0,r6,SPR_SR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop

	/* Data cache enable */
        /* Check if DC present and skip enabling otherwise */
.L8:    l.mfspr r3,r0,SPR_UPR
        l.andi  r7,r3,SPR_UPR_DCP
        l.sfeq  r7,r0
        l.bf    .L10
        l.nop
        /* Disable DC */
        l.mfspr r6,r0,SPR_SR
        l.addi  r5,r0,-1
        l.xori  r5,r5,SPR_SR_DCE
	l.and   r5,r6,r5
        l.mtspr r0,r5,SPR_SR
        /* Establish cache block size
           If BS=0, 16;
           If BS=1, 32;
           r14 contain block size
        */
        l.mfspr r3,r0,SPR_DCCFGR
        l.andi  r7,r3,SPR_DCCFGR_CBS
        l.srli  r8,r7,7
        l.ori   r4,r0,16
        l.sll   r14,r4,r8
        /* Establish number of cache sets
           r10 contains number of cache sets
           r8 contains log(# of cache sets)
        */
	l.andi  r7,r3,SPR_DCCFGR_NCS
	l.srli  r8,r7,3
        l.ori   r4,r0,1
        l.sll   r10,r4,r8
        /* Invalidate DC */
        l.addi  r6,r0,0
        l.sll   r5,r14,r8

.L9:    l.mtspr r0,r6,SPR_DCBIR
        l.sfne  r6,r5
        l.bf    .L9
	l.add   r6,r6,r14
        /* Enable DC */
        l.mfspr r6,r0,SPR_SR
        l.ori   r6,r6,SPR_SR_DCE
        l.mtspr r0,r6,SPR_SR

.L10:	
	/* Return */
	l.jr	r9
	l.nop


/* Enable and disable functions - callable from anywhere. */
	
	.global	icache_enable
        .type	icache_enable,@function

icache_enable:
	l.addi	r1,r1,-4
	l.sw	0(r1),r6
	/* Enable IC */
	l.mfspr r6,r0,SPR_SR
	l.ori   r6,r6,SPR_SR_ICE
	l.mtspr r0,r6,SPR_SR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.lwz	r6,0(r1)
	l.jr	r9
	l.addi	r1,r1,4


	.global	icache_disable
        .type	icache_disable,@function

icache_disable:
	l.addi	r1,r1,-8
	l.sw	0(r1),r5
	l.sw	4(r1),r6
	/* Disable IC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_ICE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR
	l.lwz	r5,0(r1)
	l.lwz	r6,4(r1)
	l.jr	r9
	l.addi	r1,r1,8


	.global	dcache_enable
        .type	dcache_enable,@function

dcache_enable:
	l.addi	r1,r1,-4
	l.sw	0(r1),r6
	/* Enable DC */
	l.mfspr r6,r0,SPR_SR
	l.ori   r6,r6,SPR_SR_DCE
	l.mtspr r0,r6,SPR_SR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.lwz	r6,0(r1)
	l.jr	r9
	l.addi	r1,r1,4


	.global	dcache_disable
        .type	dcache_disable,@function

dcache_disable:
	l.addi	r1,r1,-8
	l.sw	0(r1),r5
	l.sw	4(r1),r6
	/* Disable DC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_DCE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR
	l.lwz	r5,0(r1)
	l.lwz	r6,4(r1)
	l.jr	r9
	l.addi	r1,r1,8


