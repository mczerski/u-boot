/*
 * Hardware power management
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

void do_pmin()
{

// Power off configuration
// GPIO Byte 0 = 0x73
// GPIO Byte 1 = 0x55
// GPIO Byte 2 = 0x03

	gpio_direction_output(0, 1);
	gpio_direction_output(1, 1);
	gpio_direction_output(2, 0);
	gpio_direction_output(3, 0);	// 0x73
	gpio_direction_output(4, 1);
	gpio_direction_output(5, 1);
	gpio_direction_output(6, 1);
	gpio_direction_output(7, 0);

	gpio_direction_output(8, 1);
	gpio_direction_output(9, 0);
	gpio_direction_output(10, 1);
	gpio_direction_output(11, 0);
	gpio_direction_output(12, 1);	// 0x55
	gpio_direction_output(13, 0);
	gpio_direction_output(14, 1);
	gpio_direction_output(15, 0);

	gpio_direction_output(16, 1);
	gpio_direction_output(17, 1);
	gpio_direction_output(18, 0);
	gpio_direction_output(19, 0);
	gpio_direction_output(20, 0);	// 0x03
	gpio_direction_output(21, 0);
	gpio_direction_output(22, 0);
	gpio_direction_output(23, 0);

  	printf("Device put in minimal power state\n");
		
}

void do_pmax()
{

  	printf("Device put in nominal power state\n");		
}
