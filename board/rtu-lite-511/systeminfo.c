/*
 * Hardware identification
 *
 * Fredrik Bruhn, fredrik.bruhn@aacmicrotec.com
 * Dan Ohlsson, dan.ohlsson@aacmicrotec.com
 * Copyright (C) 2011 AAC Microtec AB
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

#include <common.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <asm/spr-defs.h>
#include <errno.h>


// AAC AGC lookup table.

#define RTU_lite_511 	0x31
#define RTU_lite_512 	0x32
#define uRTU_311 	0x21
#define uRTU_312 	0x22

//Get the AGC which is located on GPIO 44 to GPIO 51

#define agc_bit0 	44
#define agc_bit1 	45
#define agc_bit2 	46
#define agc_bit3 	47
#define agc_bit4 	48
#define agc_bit5 	49
#define agc_bit6 	50
#define agc_bit7 	51

void aac_agc_info(void)
{
	ushort agc = 0x00;

	// Define AGC IO
	gpio_direction_input(agc_bit0);
	gpio_direction_input(agc_bit1);
	gpio_direction_input(agc_bit2);
	gpio_direction_input(agc_bit3);
	gpio_direction_input(agc_bit4);
	gpio_direction_input(agc_bit5);
	gpio_direction_input(agc_bit6);
	gpio_direction_input(agc_bit7);

	//Get the AGC which is located on GPIO 44 to GPIO 51
	agc = agc + gpio_get_value(agc_bit0) * 1;
	agc = agc + gpio_get_value(agc_bit1) * 2;
	agc = agc + gpio_get_value(agc_bit2) * 4;
	agc = agc + gpio_get_value(agc_bit3) * 8;
	agc = agc + gpio_get_value(agc_bit4) * 16;
	agc = agc + gpio_get_value(agc_bit5) * 32;
	agc = agc + gpio_get_value(agc_bit6) * 64;
	agc = agc + gpio_get_value(agc_bit7) * 128;
  	printf("Device hardware product id: 0x%x\n", agc);
	
	if ( agc == RTU_lite_511 )
	{
  		printf("Product identified as AAC Microtec RTU lite 511 (EM)\n");
	}

	if ( agc == RTU_lite_512 )
	{
  		printf("Product identified as AAC Microtec RTU lite 512 (FM)\n");
	}

	if ( agc == uRTU_311 )
	{
  		printf("Product identified as AAC Microtec uRTU 311 (EM)\n");
	}

	if ( agc == uRTU_312 )
	{
  		printf("Product identified as AAC Microtec uRTU 312 (FM)\n");
	}
}

void aac_cpu_info()
{
	unsigned long vr = mfspr(SPR_VR);
	
	printf("\nOpenRISC 1000 Architecture - CPU information (Version Register)\n");
	
	printf("Version:\t0x%2x",(vr & SPR_VR_VER) >> SPR_VR_VER_OFF);
	printf("\tConfig:\t%d",(vr & SPR_VR_CFG) >> SPR_VR_CFG_OFF);
	printf("\tRevision:\t%d",(vr & SPR_VR_REV));
	printf("\n");

	unsigned version = (vr & SPR_VR_VER) >> SPR_VR_VER_OFF;

	if ( (version >= 45) && (version <= 50) )
		puts("CPU: AAC certified OpenRISC Fault Tolerant\n");

	if ( (version >= 30) && (version <= 35) ) 
		puts("CPU: AAC certified OpenRISC\n");
	
	if (version < 30)
		puts("CPU: uncertified OpenRISC\n");

	printf("\nCPU Capabilities:\n");

	unsigned long cpucfgr = mfspr(SPR_CPUCFGR);
	
	printf("OpenRISC 1000 Architecture - ORBIS32: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OB32S)==SPR_CPUCFGR_OB32S) ?"":"not ");

	printf("OpenRISC 1000 Architecture - ORFPX32: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OF32S)==SPR_CPUCFGR_OF32S) ?"":"not ");

	printf("\nOptional instructions supported by this system\n");

	// Multiply

	errno = 0;

	volatile int a, b, c;
	a = b = 12;
        asm ("l.mul %0,%1,%2" : "=r" (c) : "r" (a), "r" (b));

	if (errno >= 0)
		printf("Multiply (l.mul)\n");

	// Divide

	errno = 0;

	a = b = 12;
        asm ("l.div %0,%1,%2" : "=r" (c) : "r" (a), "r" (b));

	if (errno >= 0)
		printf("Divide (l.div)\n");

	// FF1

	errno = 0;

	a = b = 12;
        asm ("l.ff1 %0,%1" : "=r" (a) : "r" (b));

	if (errno >= 0)
		printf("Find First 1 (l.ff1)\n");

	// FL1

	errno = 0;

	a = b = 12;
        asm ("l.fl1 %0,%1" : "=r" (a) : "r" (b));

	if (errno >= 0)
		printf("Find Last 1 (l.fl1)\n");

	// lfrem

	errno = 0;

	a = b = 12;
        asm ("lf.rem.s %0,%1,%2" : "=r" (c) : "r" (a), "r" (b));

	if (errno >= 0)
		printf("Floating remainder (lf.frem.s)\n");


	/* Unit present register */
	unsigned long upr;
	upr = mfspr(SPR_UPR);

	if (!(SPR_UPR_UP & upr))
	{
		printf("UPR not present. Cannot determine present units\n");
		return 1;
	}
	
	printf("\nOptional CPU near units present in this system:\n");
	

	if (upr & SPR_UPR_DCP)
		printf("Data Cache\n");

	if (upr & SPR_UPR_ICP)
		printf("Instruction Cache\n");

	if (upr & SPR_UPR_DMP)
		printf("Data MMU\n");

	if (upr & SPR_UPR_IMP)
		printf("Instruction MMU\n");

	if (upr & SPR_UPR_MP)
		printf("MAC Unit\n");

	if (upr & SPR_UPR_DUP)
		printf("Debug Unit\n");

	if (upr & SPR_UPR_PCUP)
		printf("Performance Counters Unit\n");

	if (upr & SPR_UPR_PMP)
		printf("Power Management Unit\n");

	if (upr & SPR_UPR_PICP)
		printf("Programmable Interrupt Controller\n");

	if (upr & SPR_UPR_TTP)
		printf("Tick Timer\n");

	if (upr & SPR_UPR_CUP)
		printf("Context units\n");
}

void do_systeminfo()
{
	// Present AGC related information
	aac_agc_info();
 
	// Present CPU related information
	aac_cpu_info();
}
