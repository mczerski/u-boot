/*
 * Misc on-board ADC measurements (temperature, voltage and current)
 *
 * Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * Adapted to u-boot from "Housekeeping test software" by
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
#include <spi.h>
#include <asm/gpio.h>

#define ADC_CHANNELS 8

#define ADC_SPI_BUS 0

#define ADC_CS     0
#define ADC_EOC    1
#define ADC_FS     2
#define ADC_PWDN   3
#define ADC_CSTART 4

#define READ_CFR 0x90 	/* 0x9000 read Configuration register command */
/*
 * Configuration register settings, ref. page 7 in data sheet.
 * D11 	-  1 - Internal reference
 * D10 	-  1 - Internal reference = 2 V
 * D9 	-  0 - Short sample time = 12 SCLK clocks
 * D8,7	- 00 - Conversion clock = Internal OSC
 * D6,5	- 00 - Single shot mode
 * D4,3	- 00 - N/A in single shot mode
 * D2	-  1 - EOC operation
 * D1,0	- 00 - FIFO trigger level (not used in single shot mode).
 *
 * Register configuration = 0xC04
 */

/* 0xAC84 internal reference 2V ,short sampling,SCLK,single shot,EOC */
#define  WRITE_CFR_H  	0xAC
#define  WRITE_CFR_L	0x04
/* Set test voltage to Analog GND (Channel 0-3) command */
#define	 TEST_VOLTAGE_L 0xC0
/* Set test voltage to internal reference 2 V (Channel 0-3) command.*/
#define	 TEST_VOLTAGE_H 0xD0
#define  ADC_CHANNEL1	1 	/* 0x1000 */
#define  ADC_CHANNEL2	2 	/* 0x2000 */
#define  ADC_CHANNEL3	3 	/* 0x3000 */
#define  ADC_TEMP	4 	/* 0x4000 */
#define  ADC_ICC	5 	/* 0x5000 */
#define  ADC_VCC	6 	/* 0x6000 */
#define  ADC_V33	7 	/* 0x7000 */

static int tlv2548_init(struct spi_slave *slave)
{
 	ushort result = 0;
	u8 data_in[2];
	u8 data_out[2];

	/* Wake up the ADC with a dummy write of something. */
	data_out[0] = WRITE_CFR_H;
	data_out[1] = WRITE_CFR_L;
	spi_xfer(slave, 16, data_out, NULL, SPI_XFER_BEGIN | SPI_XFER_END);

	/* Write iniatialization configuration */
	spi_xfer(slave, 16, data_out, NULL, SPI_XFER_BEGIN | SPI_XFER_END);

	/* Initiate a read of the configuration */
	data_out[0] = READ_CFR;
	data_out[1] = 0;
	spi_xfer(slave, 16, data_out, data_in, SPI_XFER_BEGIN | SPI_XFER_END);

	/* According to Datasheet, page 11. Only safe bits 11..0 (12). */
	result = (((ushort)data_in[0] & 0x0f) << 8) | (data_in[1] & 0xff);
	if (result == (((WRITE_CFR_H << 8) | WRITE_CFR_L) & 0x0fff))
		return 1;
	else
		return 0;
}

static ushort tlv2548_read(struct spi_slave *slave, ushort channel)
{
	ushort result = 0;
	u8 data_in[2];
	u8 data_out[2];

	/* According to data sheet, page 6, channel values are D15-D14 */
	channel = (channel << 4) & 0xF0;

	data_out[0] = channel;
	data_out[1] = 0;
	spi_xfer(slave, 16, data_out, data_in, SPI_XFER_BEGIN | SPI_XFER_END);

	while (!gpio_get_value(ADC_EOC)) {
		if (ctrlc())
			return 0;
	}

	spi_xfer(slave, 16, data_out, data_in, SPI_XFER_BEGIN | SPI_XFER_END);
	/*
	 * According to Datasheet page 14, data is clocked out MSB (D15..D4).
	 * so, shift result down >> 4 to get the real value
	 */
	result = (0x0fff & ((ushort)data_in[0] << 4)) |
		 (0x000f & ((ushort)data_in[1] >> 4));

	/* LSB is noise according to datasheet, throw away */
	return (result >> 1);
}

/* Returns temperature in C */
static float calc_board_temp(ushort raw_data)
{
	return (((float)raw_data * 0.0009766) - 1.4081) / 0.0038;
}

/* Returns current in mA */
static float calc_board_icc(ushort raw_data)
{
        return ((float)raw_data * 5000.0) / 2048;
}

/* Returns voltage in V */
static float calc_board_vcc(ushort raw_data)
{
        return (5 / 1.5986 * (float)raw_data * 0.000976563);
}

/* Returns voltage in V */
static float calc_board_v33(ushort raw_data)
{
        return (3.3 / 1.055 * (float) raw_data * 0.000976563);
}

void do_sensors()
{
	struct spi_slave *slave;
	int i;
	ushort raw_data;
	int icc;
	int vcc;
	gpio_direction_output(ADC_CS, 1);
	gpio_direction_input(ADC_EOC);
	gpio_direction_output(ADC_FS, 1);
	gpio_direction_output(ADC_PWDN, 1);
	gpio_direction_output(ADC_CSTART, 1);
	gpio_direction_output(5, 1);
	gpio_direction_output(6, 1);
	gpio_direction_output(7, 0);

	slave = spi_setup_slave(ADC_SPI_BUS, ADC_CS, 1000000, SPI_MODE_0);
	if (!slave) {
		printf("SPI setup failed\n");
		return;
	}
	spi_claim_bus(slave);
	if (!tlv2548_init(slave)) {
		printf("ERROR! Didn't detect any Analog to Digital converter "
		       "on SPI bus %d\n", slave->bus);
		goto out;
	}
	raw_data = tlv2548_read(slave, ADC_TEMP);
	printf("Device temperature \t \t%d (deg C)\n", (int) calc_board_temp(raw_data));

	raw_data = tlv2548_read(slave, ADC_ICC);
	icc = (int) calc_board_icc(raw_data);
	printf("Device current \t \t \t%d (mA)\n",icc);

	raw_data = tlv2548_read(slave, ADC_VCC);
	vcc = (int) (calc_board_vcc(raw_data) * 1000);
	printf("Device 5 V level \t \t%d (mV)\n", vcc);

	raw_data = tlv2548_read(slave, ADC_V33);
	printf("Device 3.3 V level \t \t%d (mV)\n",(int) (calc_board_v33(raw_data) * 1000));

	printf("Device power consumption\t%01d.%02d (W)\n",
	       (vcc*icc)/1000000, (vcc*icc)%1000000);

out:
	spi_release_bus(slave);
	spi_free_slave(slave);
}
