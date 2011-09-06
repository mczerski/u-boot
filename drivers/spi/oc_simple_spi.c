/*
 * OpenCores simple_spi driver
 *
 * http://opencores.org/project,simple_spi
 *
 * based on oc_tiny_spi.c which in turn was based on bfin_spi.c
 * Copyright (c) 2005-2008 Analog Devices Inc.
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (C) 2011 Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
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
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>
#include <asm/gpio.h>

#define SIMPLE_SPI_SPCR 0x00
#define SIMPLE_SPI_SPSR 0x01
#define SIMPLE_SPI_SPDR 0x02
#define SIMPLE_SPI_SPER 0x03
#define SIMPLE_SPI_SSEL 0x04

#define SIMPLE_SPI_SPCR_SPIE  (1 << 7)
#define SIMPLE_SPI_SPCR_SPE   (1 << 6)
#define SIMPLE_SPI_SPCR_MSTR  (1 << 4)
#define SIMPLE_SPI_SPCR_CPOL  (1 << 3)
#define SIMPLE_SPI_SPCR_CPHA  (1 << 2)
#define SIMPLE_SPI_SPCR_SPR   0x03

#define SIMPLE_SPI_SPSR_SPIF	(1 << 7)
#define SIMPLE_SPI_SPSR_WCOL	(1 << 6)
#define SIMPLE_SPI_SPSR_WFFULL	(1 << 3)
#define SIMPLE_SPI_SPSR_WFEMPTY	(1 << 2)
#define SIMPLE_SPI_SPSR_RFFULL	(1 << 1)
#define SIMPLE_SPI_SPSR_RFEMPTY	(1 << 0)

#define SIMPLE_SPI_SPER_ICNT  0xc0
#define SIMPLE_SPI_SPER_ESPR  0x03

#ifndef CONFIG_OC_SIMPLE_SPI_DUMMY_BYTE
	#define CONFIG_OC_SIMPLE_SPI_DUMMY_BYTE 0x00
#endif

struct simple_spi_host {
	uint base;
	uint freq;
	uint baudwidth;
};
static const struct simple_spi_host simple_spi_host_list[] =
	CONFIG_SYS_SIMPLE_SPI_LIST;

struct simple_spi_slave {
	struct spi_slave slave;
	const struct simple_spi_host *host;
	uint mode;
	uint baud;
	uint flg;
};
#define to_simple_spi_slave(s) container_of(s, struct simple_spi_slave, slave)

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	#ifdef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	return bus < ARRAY_SIZE(simple_spi_host_list);
	# else
	return bus < ARRAY_SIZE(simple_spi_host_list) && gpio_is_valid(cs);
	#endif
}

void spi_cs_activate(struct spi_slave *slave)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
#ifdef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	uint base  = simple_spi->host->base;
	char flags = readb(base + SIMPLE_SPI_SSEL) | (1<<slave->cs);

	writeb(flags, base + SIMPLE_SPI_SSEL);
#else
	unsigned int cs = slave->cs;

	gpio_set_value(cs, simple_spi->flg);
	debug("%s: SPI_CS_GPIO:%x\n", __func__, gpio_get_value(cs));
#endif
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
#ifdef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	uint base  = simple_spi->host->base;
	char flags = readb(base + SIMPLE_SPI_SSEL) & ~(1<<slave->cs);

	writeb(flags, base + SIMPLE_SPI_SSEL);
#else
	unsigned int cs = slave->cs;

	gpio_set_value(cs, !simple_spi->flg);
	debug("%s: SPI_CS_GPIO:%x\n", __func__, gpio_get_value(cs));
#endif
}

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
	const struct simple_spi_host *host = simple_spi->host;
	int i;
	/* (ESPR << 8 | SPR) max allowed value is 11 */
	for (i = 0; i <= 11; i++) {
		if ((host->freq >> (1+i)) <= hz)
			break;
	}

	/* 
	 * The order of the values of clkdivider in 
	 * simple_spi might seem a bit strange, 
	 * but the reason is to keep SPR compatible with M68HC11
	 */
	switch (i) {
	case 2:
		simple_spi->baud = 4;
		break;
	case 3:
		simple_spi->baud = 2;
		break;
	case 4:
		simple_spi->baud = 3;
		break;
	default:
		simple_spi->baud = i;
		break;
	}

	debug("%s: speed %u actual %u\n", __func__, hz,
	      host->freq >> (i + 1));
}

void spi_init(void)
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int hz, unsigned int mode)
{
	struct simple_spi_slave *simple_spi;

#ifdef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	if (!spi_cs_is_valid(bus, cs))
		return NULL;
#else
	if (!spi_cs_is_valid(bus, cs) || gpio_request(cs, "simple_spi"))
		return NULL;
#endif

	simple_spi = malloc(sizeof(*simple_spi));
	if (!simple_spi)
		return NULL;
	memset(simple_spi, 0, sizeof(*simple_spi));

	simple_spi->slave.bus = bus;
	simple_spi->slave.cs = cs;
	simple_spi->host = &simple_spi_host_list[bus];
	simple_spi->mode = mode & (SPI_CPOL | SPI_CPHA);
	simple_spi->flg = mode & SPI_CS_HIGH ? 1 : 0;
	spi_set_speed(&simple_spi->slave, hz);

	debug("%s: bus:%i cs:%i base:%lx\n", __func__,
		bus, cs, simple_spi->host->base);
	return &simple_spi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
#ifndef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	gpio_free(slave->cs);
#endif
	free(simple_spi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
	uint base  = simple_spi->host->base;
	u8 spcr = 0;
	u8 sper = 0;

#ifndef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
	gpio_direction_output(slave->cs, !simple_spi->flg);
#endif

	/* SPI Enable */
	spcr |= SIMPLE_SPI_SPCR_SPE;
	/* Controller only supports Master mode, but set it explicitly */
	spcr |= SIMPLE_SPI_SPCR_MSTR;

	if (simple_spi->mode & SPI_CPOL)
		spcr |= SIMPLE_SPI_SPCR_CPOL;
	if (simple_spi->mode & SPI_CPHA)
		spcr = SIMPLE_SPI_SPCR_CPHA;

	spcr |= (simple_spi->baud & SIMPLE_SPI_SPCR_SPR);
	sper |= (simple_spi->baud & SIMPLE_SPI_SPER_ESPR);
	writeb(spcr, base + SIMPLE_SPI_SPCR);
	writeb(sper, base + SIMPLE_SPI_SPER);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
	uint base  = simple_spi->host->base;
	u8 spcr = readb(base + SIMPLE_SPI_SPCR);

	/* Disable SPI */
	spcr &= ~SIMPLE_SPI_SPCR_SPE;
	writeb(spcr, base + SIMPLE_SPI_SPCR);
#ifndef CONFIG_OC_SIMPLE_SPI_BUILTIN_SS
	gpio_direction_input(slave->cs);
	debug("%s: bus:%i cs:%i\n", __func__, slave->bus, slave->cs);
#endif
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct simple_spi_slave *simple_spi = to_simple_spi_slave(slave);
	uint base = simple_spi->host->base;
	const u8 *txp = dout;
	u8 *rxp = din;
	u8 spsr;
	uint bytes = bitlen / 8;
	uint rxbytes = 0;
	uint txbytes = 0;
	int ret = 0;
	debug("%s: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
		slave->bus, slave->cs, bitlen, bytes, flags);
	if (bitlen == 0)
		goto done;

	/* assume to do 8 bits transfers */
	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	/* empty read fifo */
	while(!(readb(base + SIMPLE_SPI_SPSR) & SIMPLE_SPI_SPSR_RFEMPTY)) {
		if (ctrlc())
			return -1;
		readb(base + SIMPLE_SPI_SPDR);
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	while(rxbytes < bytes) {
		spsr = readb(base + SIMPLE_SPI_SPSR);
		if (!(spsr &  SIMPLE_SPI_SPSR_RFEMPTY)) {
			if (rxp)
				*rxp++ = readb(base + SIMPLE_SPI_SPDR);
			else
				readb(base + SIMPLE_SPI_SPDR);
			rxbytes++;
		}
		if (!(spsr & SIMPLE_SPI_SPSR_WFFULL) && txbytes < bytes) {
			if (txp)
				writeb(*txp++, base + SIMPLE_SPI_SPDR);
			else
				writeb(CONFIG_OC_SIMPLE_SPI_DUMMY_BYTE, base + SIMPLE_SPI_SPDR);
			txbytes++;
		}

		if (ctrlc()) {
			ret = -1;
			goto done;
		}
	}

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return ret;
}
