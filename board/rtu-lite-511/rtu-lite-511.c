/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
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

#include <common.h>
#include <netdev.h>
#include <asm/gpio.h>

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

int get_aac_agc(void)
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

	return agc;
}

int board_early_init_f(void)
{
	return 0;
}

int checkboard(void)
{
	ushort agc = get_aac_agc();

	if ( agc == RTU_lite_511 )
	{
  		printf("BOARD: AAC Microtec RTU lite 511 (EM)\n");
	}

	if ( agc == RTU_lite_512 )
	{
  		printf("BOARD: AAC Microtec RTU lite 512 (FM)\n");
	}

	if ( agc == uRTU_311 )
	{
  		printf("BOARD: AAC Microtec uRTU 311 (EM)\n");
	}

	if ( agc == uRTU_312 )
	{
  		printf("BOARD: AAC Microtec uRTU 312 (FM)\n");
	}

	return 0;
}

phys_size_t initdram(int board_type)
{
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_ETHOC
	rc += ethoc_initialize(0, CONFIG_SYS_ETHOC_BASE);
#endif
	return rc;
}
#endif
