/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <asm/system.h>
#include <common.h>

static unsigned long timestamp;

/* how many counter cycles in a jiffy */
#define TIMER_COUNTER_CYCLES  (CONFIG_SYS_CLK_FREQ/CONFIG_SYS_HZ)

extern void _exception_handler_add(int,unsigned long);

void timer_isr(void)
{
	timestamp++;
	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT |
	      (TIMER_COUNTER_CYCLES & SPR_TTMR_TP));
}

int timer_init(void)
{
	/* Install timer exception handler */
	_exception_handler_add(5,(unsigned long)timer_isr);

	/* Set up the timer for the first expiration. */
	timestamp = 0;

	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT |
	      (TIMER_COUNTER_CYCLES & SPR_TTMR_TP));

	/* Enable tick timer exception in supervisor register */
	mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_TEE);

	return 0;
}

void reset_timer(void)
{
	timestamp = 0;

	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT |
	      (TIMER_COUNTER_CYCLES & SPR_TTMR_TP));
}

ulong get_timer(ulong base)
{
	return (timestamp - base);
}

void set_timer(ulong t)
{
	reset_timer();
	timestamp = t;
}

// TODO - check this!
void __udelay(unsigned long usec)
{

	int i;
	i = get_timer (0);
	while ((get_timer (0) - i) < (usec / 1000)) ;

}
