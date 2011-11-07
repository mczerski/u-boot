/*
 * (C) Copyright 2011, Julius Baxter, julius@opencores.org
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * BOARD/CPU
 */

#define CONFIG_SYS_CLK_FREQ		66666666
#define CONFIG_SYS_RESET_ADDR		0x00000100

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		0x02000000

#define CONFIG_SYS_UART_BASE		0x90000000
#define CONFIG_SYS_UART_FREQ		CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_UART_BAUD		115200

#define CONFIG_BOARD_NAME		"ml501" /* custom board name */

#define CONFIG_SYS_FLASH_BASE		0xF0000000
#define CONFIG_SYS_FLASH_CFI			/* Flash is CFI conformant    */
#undef CONFIG_SYS_FLASH_CFI_DRIVER
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	280	/* 
						 * max number of sectors on one
                                                 * chip 
						 */
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000  /* Flash Erase Timeout (in ms)*/
#define CONFIG_SYS_FLASH_INCREMENT	0x01000000
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	/* 
						 * use buffered writes
						 * (20x faster)
						 */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)*/
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_PROTECTION

/* OpenRISC-specific boot settings */
#define CONFIG_SYS_RELOCATE_VECTORS		/* 
						 * Relocate the vectors code
                                                 * from flash to RAM at reset
						 */
#define CONFIG_SYS_VECTORS_LEN 		0x2000


/*
 * SERIAL
 */
# define CONFIG_SYS_NS16550
# define CONFIG_SYS_NS16550_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	1
# define CONFIG_CONS_INDEX		1
# define CONFIG_SYS_NS16550_COM1 	(0x90000000)
# define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_CLK_FREQ

#define CONFIG_BAUDRATE			CONFIG_SYS_UART_BAUD
#define CONFIG_SYS_BAUDRATE_TABLE	{CONFIG_BAUDRATE}
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* Suppress console info */
#define CONSOLE_ARG			"console=console=ttyS0,115200\0"

/*
 * Ethernet
 */
#define CONFIG_ETHOC
#define CONFIG_SYS_ETHOC_BASE		0x92000000

#define CONFIG_ETHADDR			00:12:34:56:78:9a
#define CONFIG_NETMASK			255.255.0.0
#define CONFIG_IPADDR			192.168.100.155 /* board's IP */
#define CONFIG_GATEWAYIP		192.168.100.1
#define CONFIG_SERVERIP			192.168.100.105
#define CONFIG_BOOTFILE			"boot.img"
#define CONFIG_LOADADDR			0x100000 /* 1MB mark */

/*
 * TIMER
 */
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_OPENRISC_TMR_HZ	100

/*
 * Memory organisation:
 *
 * RAM start ---------------------------
 *           | ...                     |
 *           ---------------------------
 *           | Stack                   |
 *           ---------------------------
 *           | Global data             |
 *           ---------------------------
 *           | Environment             |
 *           ---------------------------
 *           | Monitor                 |
 * RAM end   ---------------------------
 */
/* We're running in RAM */
#define CONFIG_MONITOR_IS_IN_RAM
#define CONFIG_SYS_MONITOR_LEN	0x40000	/* Reserve 256k */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_SDRAM_BASE + \
				CONFIG_SYS_SDRAM_SIZE - \
				CONFIG_SYS_MONITOR_LEN)

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET	0x80000 /*
					 * Flagrantly ignore the CONFIG_ENV_IS_IN_FLASH
                                         * instructions in the README - store it in the 
					 * open for now. 
					 */
#define CONFIG_ENV_SECT_SIZE	(128*1024)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

/*
 * Global data object and stack pointer
 */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_ENV_ADDR \
					- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_GBL_DATA_ADDR	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_STACK_LENGTH		0x10000 /* 64KB */
#define CONFIG_SYS_MALLOC_LEN		0x400000 /* 4MB */
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_INIT_SP_OFFSET \
					- CONFIG_SYS_STACK_LENGTH \
					- CONFIG_SYS_MALLOC_LEN)
/*
 * MISC
 */
#define CONFIG_SYS_LONGHELP		/* Provide extended help */
#define CONFIG_SYS_PROMPT		"==> "	/* Command prompt	*/
#define CONFIG_SYS_CBSIZE		256	/* Console I/O buf size */
#define CONFIG_SYS_MAXARGS		16	/* Max command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Bootarg buf size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + \
					 16)	/* Print buf size */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x2000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_INIT_SP_ADDR - 0x20000)


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>


#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ELF
#define CONFIG_CMD_BSP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_CMD_MII
#define CONFIG_NET_MULTI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FLASH


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#define CONFIG_OF_LIBFDT
#define CONFIG_LMB

#endif /* __CONFIG_H */
