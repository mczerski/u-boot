/*
 * Copyright (C) 2011 Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * Copyright (C) 2011 Fredrik Bruhn <fredrik.bruhn@aacmicrotec.com>
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
#include <command.h>
#include "sensors.h"
#include "id.h"
#include "power.h"
#include "systeminfo.h"

#define SHORT_HELP\
	"aac    - aac test programs\n"

#define LONG_HELP\
	"\n"\
	"aac sensors\n"\
	"    - display device housekeeping information\n"\
	"aac id\n"\
	"    - display device hardware product id\n"\
	"aac systeminfo\n"\
	"    - display device system information\n"


int do_aac (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return 0;

	if (strncmp ("sensors", argv[1], strlen(argv[1])) == 0) {
		do_sensors();
	}

	if (strncmp ("id", argv[1], strlen(argv[1])) == 0) {
		do_id();
	}

	if (strncmp ("pmin", argv[1], strlen(argv[1])) == 0) {
		do_pmin();
	}

	if (strncmp ("pmax", argv[1], strlen(argv[1])) == 0) {
		do_pmax();
	}

	if (strncmp ("systeminfo", argv[1], strlen(argv[1])) == 0) {
		do_systeminfo();
	}


	return 0;
}
U_BOOT_CMD( aac, 2, 0, do_aac, SHORT_HELP, LONG_HELP );
