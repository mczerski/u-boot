/*
 * Driver for AAC Microtec NAND flash controller
 *
 * (C) Copyright 2011 Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <malloc.h>

#include <asm/errno.h>
#include <asm/io.h>

#include <nand.h>

#define AAC_NAND_DATW  0x00000000
#define AAC_NAND_CR    0x00000001
#define AAC_NAND_ADR0  0x00000002
#define AAC_NAND_ADR1  0x00000003
#define AAC_NAND_ADR2  0x00000004
#define AAC_NAND_ADR3  0x00000005
#define AAC_NAND_SR    0x00000006
#define AAC_NAND_DATR  0x00000007
#define AAC_NAND_RB    0x00000015

#define AAC_NAND_CR_PROGRAM      (1<<0)
#define AAC_NAND_CR_READSTATUS   (1<<1)
#define AAC_NAND_CR_READ         (1<<2)
#define AAC_NAND_CR_ERASE        (1<<3)
#define AAC_NAND_CR_READID       (1<<4)
#define AAC_NAND_CR_RESET        (1<<5)
#define AAC_NAND_CR_PROG_DONE    (1<<6)

#define AAC_NAND_SR_BUSY         (1<<0)
#define AAC_NAND_SR_BYTEWRITEREQ (1<<1)
#define AAC_NAND_SR_BYTEREADREQ  (1<<2)

#define AAC_NAND_TIMEOUT         5000

struct aac_nand_priv {
	struct mtd_info       mtd;
	struct nand_chip      chip;
	void __iomem         *regs;
	int                   cmd;
	int                   column;
	int                   page;
};

static inline u8 aac_nand_readreg(void __iomem *base, loff_t offset)
{
	return ioread8(base + offset);
}

static inline void aac_nand_writereg(void __iomem *base, loff_t offset, u8 data)
{
	iowrite8(data, base + offset);
}

static inline int aac_nand_nfc_ready(struct mtd_info *mtd)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;

	return !(aac_nand_readreg(priv->regs, AAC_NAND_SR) &
		 AAC_NAND_SR_BUSY);
}

/* Waits until the nand flash controller is not busy */
static void aac_nand_wait_nfc_ready(struct mtd_info *mtd)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;
	int timeout = AAC_NAND_TIMEOUT;
	do {
		udelay(1);
	} while(!aac_nand_nfc_ready(mtd) && --timeout);

	if (!timeout) {
		aac_nand_writereg(priv->regs,AAC_NAND_CR, AAC_NAND_CR_RESET);
		printk(KERN_WARNING "aac_nand: wait ready fc timeout!\n");
	}
}

static u8 aac_nand_read_status(struct mtd_info *mtd)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;

	aac_nand_wait_nfc_ready(mtd);
	aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_READSTATUS);
	
	while(!aac_nand_nfc_ready(mtd)) {
		if(aac_nand_readreg(priv->regs, AAC_NAND_SR) &
		   AAC_NAND_SR_BYTEREADREQ) {
			aac_nand_writereg(priv->regs, AAC_NAND_CR,
					  AAC_NAND_CR_READ);
			return aac_nand_readreg(priv->regs, AAC_NAND_DATR);
		}
	}
	/* never reached, but removes warning */
	return 0;
}

static inline int aac_nand_ready(struct mtd_info *mtd)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;

	return aac_nand_readreg(priv->regs, AAC_NAND_RB);
}

/* Waits until the flash ready/busy is ready */
static void aac_nand_wait_ready(struct mtd_info *mtd)
{
	int timeout = AAC_NAND_TIMEOUT;

	do {
		udelay(1);
	} while(!aac_nand_ready(mtd) && --timeout);

	if (!timeout)
		printk(KERN_WARNING "aac_nand: wait ready timeout!\n");
}

static void aac_nand_addr_cycle(struct mtd_info *mtd, int column, int page)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;
	unsigned long calc_addr;

	calc_addr = page * (mtd->writesize*2) + column;

	aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_RESET);
	aac_nand_wait_nfc_ready(mtd);

	aac_nand_writereg(priv->regs, AAC_NAND_ADR0, (calc_addr >>  0) & 0xff);
	aac_nand_writereg(priv->regs, AAC_NAND_ADR1, (calc_addr >>  8) & 0xff);
	aac_nand_writereg(priv->regs, AAC_NAND_ADR2, (calc_addr >> 16) & 0xff);
	aac_nand_writereg(priv->regs, AAC_NAND_ADR3, (calc_addr >> 24) & 0xff);
}

static u8 aac_nand_read_id(struct mtd_info *mtd, int column)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;
	u8 idbuf[5];
	int i=0;
 
	aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_READID);
	while(!aac_nand_nfc_ready(mtd)) {		
		if(aac_nand_readreg(priv->regs, AAC_NAND_SR) &
		   AAC_NAND_SR_BYTEREADREQ) {
			idbuf[i] = aac_nand_readreg(priv->regs, AAC_NAND_DATR);
			aac_nand_writereg(priv->regs, AAC_NAND_CR,
					  AAC_NAND_CR_READ);
			if (i++ == 4)
				break;
		}
	}

	if (column < 5)
		return idbuf[column];

	return 0;
}

static void aac_nand_write_buf(struct mtd_info *mtd,
			       const uint8_t *buf, int len)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;
	int i = 0, timeout = AAC_NAND_TIMEOUT;

	if (len == 0)
		return;

	aac_nand_addr_cycle(mtd, priv->column, priv->page);
	aac_nand_writereg(priv->regs, AAC_NAND_DATW, buf[i++]);
	aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_PROGRAM);
	priv->column++;

	while(--timeout && (i < len))
	{
		if(aac_nand_readreg(priv->regs, AAC_NAND_SR) &
		   AAC_NAND_SR_BYTEWRITEREQ) {
			aac_nand_writereg(priv->regs, AAC_NAND_DATW, buf[i++]);
			aac_nand_writereg(priv->regs, AAC_NAND_CR,
					  AAC_NAND_CR_PROGRAM);
			priv->column++;
			timeout = AAC_NAND_TIMEOUT;
		} else {
			udelay(1);
		}
	}
	/* 
	 * nfc always expects a whole page to be written, 
	 * to write out a smaller amount of data a PROG_DONE
	 * command has to be issued.
	 */
	if (len < mtd->writesize)
		aac_nand_writereg(priv->regs, AAC_NAND_CR, 
		AAC_NAND_CR_PROG_DONE);

	aac_nand_wait_nfc_ready(mtd);
	
	if (i != len)
		printk(KERN_WARNING "aac_nand: write buf error! "
		       "req bytes = %d, written = %d "
		       "page = %d, column = %d\n",
		       len, i, priv->page, priv->column);
}

static void aac_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;
	int i = 0, timeout = AAC_NAND_TIMEOUT;

	switch (priv->cmd) {

	case NAND_CMD_READOOB:
	case NAND_CMD_READ0:
	case NAND_CMD_RNDOUT:
		aac_nand_addr_cycle(mtd, priv->column, priv->page);
		aac_nand_wait_nfc_ready(mtd);
		break;

	default:
		break;
	}

	aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_READ);

	while(--timeout) {
		if(aac_nand_readreg(priv->regs, AAC_NAND_SR) &
		   AAC_NAND_SR_BYTEREADREQ) {
			buf[i++] = aac_nand_readreg(priv->regs, AAC_NAND_DATR);
			priv->column++;
			timeout = AAC_NAND_TIMEOUT;
			if (i < len)
				aac_nand_writereg(priv->regs, AAC_NAND_CR,
						  AAC_NAND_CR_READ);
			else
				break;
		} else {
			udelay(1);
		}
	}

	/* 
	 * nfc always expects a whole page to be read, 
	 * so reset it to get it out of waiting for read requests if less
	 * than the page size has been requested (i.e. oob)
	 */
	if (len < mtd->writesize) {
		aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_RESET);
		aac_nand_wait_nfc_ready(mtd);
	}

	if (i != len)
		printk(KERN_WARNING "aac_nand: read buf error! "
		       "req bytes = %d, read = %d, "
		       "page = %d, column = %d\n",
		       len, i, priv->page, priv->column);
}

static u8 aac_nand_read_byte(struct mtd_info *mtd)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;
	u8 tmp;

	if (priv->cmd == NAND_CMD_STATUS)
		return aac_nand_read_status(mtd);

	if (priv->cmd == NAND_CMD_READID)
		return aac_nand_read_id(mtd, priv->column++);

	aac_nand_read_buf(mtd, &tmp, sizeof(tmp));

	return tmp;
}
static void aac_nand_erase_block(struct mtd_info *mtd, int page)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;

	aac_nand_addr_cycle(mtd, 0, page);
	aac_nand_writereg(priv->regs, AAC_NAND_CR, AAC_NAND_CR_ERASE);
	aac_nand_wait_nfc_ready(mtd);
}

static void aac_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	/* empty */
}

static void aac_nand_command(struct mtd_info *mtd, unsigned cmd,
			     int column, int page)
{
	struct aac_nand_priv *priv = ((struct nand_chip *)mtd->priv)->priv;

	if (cmd == NAND_CMD_NONE)
		return;

	priv->cmd  = cmd;
	if (column != -1)
		priv->column = column;
	if (page != -1)
		priv->page = page;

	switch (cmd) {
	case NAND_CMD_READOOB:
		priv->column += mtd->writesize;
		break;

	case NAND_CMD_ERASE1:
		aac_nand_erase_block(mtd, page);
		break;

	case NAND_CMD_RESET:
		aac_nand_writereg(priv->regs,AAC_NAND_CR,
				  AAC_NAND_CR_RESET);
		break;

	default:
		break;
	}
}

int board_nand_init(struct nand_chip *nand)
{
	struct aac_nand_priv *priv;

	priv = malloc(sizeof(*priv));
	if (!priv) {
		printk(KERN_ERR "aac_nand: Out of mem!\n");
		return -ENOMEM;
	}

	priv->regs = (void __iomem *)CONFIG_SYS_NAND_BASE;
	nand->ecc.mode  = NAND_ECC_SOFT;
	nand->cmd_ctrl  = aac_nand_cmd_ctrl;
	nand->cmdfunc   = aac_nand_command;
	nand->read_buf  = aac_nand_read_buf;
	nand->dev_ready = aac_nand_ready;
	nand->read_byte = aac_nand_read_byte;
	nand->write_buf = aac_nand_write_buf;

	nand->priv      = priv;

	return 0;
}
