struct ohs900_regs {
	u8 host_tx_ctlreg;	/* 0x00 */
	u8 txtranstypereg;	/* 0x01 */
	u8 txlinectlreg;	/* 0x02 */
	u8 sofenreg;		/* 0x03 */
	u8 txaddrreg;		/* 0x04 */
	u8 txendpreg;		/* 0x05 */
	u8 frame_num_msp_reg;	/* 0x06 */
	u8 frame_num_lsp_reg;	/* 0x07 */
	u8 irq_status;		/* 0x08 */
	u8 irq_enable;		/* 0x09 */
	u8 hrxstatreg;		/* 0x0a */
	u8 hrxpidreg;		/* 0x0b */
	u8 hrxaddrreg;		/* 0x0c */
	u8 hrxendpreg;		/* 0x0d */
	u8 rxconnstatereg;	/* 0x0e */
	u8 softmrreg;		/* 0x0f */
	u8 reserved0[16];	/* 0x10...0x1f */
	u8 host_rxfifo_data;	/* 0x20 */
	u8 reserved1;		/* 0x21 */
	u8 rxfifocntmsbreg;	/* 0x22 */
	u8 rxfifocntlsbreg;	/* 0x23 */
	u8 rxfifocontrolreg;	/* 0x24 */
	u8 reserved2[11];	/* 0x25...0x2f */
	u8 host_txfifo_data;	/* 0x30 */
	u8 reserved3[3];	/* 0x31...0x33 */
	u8 txfifocontrolreg;	/* 0x34 */
	u8 reserved4[171];	/* 0x35...0xdf */
	u8 hostslavectlreg;	/* 0xe0 */
	u8 hwrevreg;		/* 0xe1 */
};

#define OHS900_HCTLMASK_TRANS_REQ		(1<<0)
#define OHS900_HCTLMASK_SOF_SYNC		(1<<1)
#define OHS900_HCTLMASK_PREAMBLE_EN		(1<<2)
#define OHS900_HCTLMASK_ISO_EN			(1<<3)

#define OHS900_STATMASK_CRC_ERROR		(1<<0)
#define OHS900_STATMASK_BS_ERROR		(1<<1)
#define OHS900_STATMASK_RX_OVF			(1<<2)
#define OHS900_STATMASK_RX_TMOUT		(1<<3)
#define OHS900_STATMASK_NAK_RXED		(1<<4)
#define OHS900_STATMASK_STALL_RXED		(1<<5)
#define OHS900_STATMASK_ACK_RXED		(1<<6)
#define OHS900_STATMASK_DATA_SEQ		(1<<7)

#define OHS900_SETUP				0x00
#define OHS900_IN				0x01
#define OHS900_OUT_DATA0			0x02
#define OHS900_OUT_DATA1			0x03

#define OHS900_MASK_SOF_ENA			(1<<0)

#define OHS900_TXLCTL_MASK_FORCE		0x4
#define OHS900_TXLCTL_MASK_LINE_CTRL_BITS	0x7
#define OHS900_TXLCTL_MASK_NORMAL		0x00
#define OHS900_TXLCTL_MASK_SE0			0x04
#define OHS900_TXLCTL_MASK_FS_J			0x06
#define OHS900_TXLCTL_MASK_FS_K			0x05
#define OHS900_TXLCTL_MASK_LSPD			0x00
#define OHS900_TXLCTL_MASK_FSPD			0x18
#define OHS900_TXLCTL_MASK_FS_POL		0x08
#define OHS900_TXLCTL_MASK_FS_RATE		0x10

#define OHS900_INTMASK_TRANS_DONE		0x01
#define OHS900_INTMASK_SOFINTR			0x08
#define OHS900_INTMASK_INSRMV			0x04
#define OHS900_INTMASK_RESUME_DET		0x02

#define OHS900_DISCONNECT_STATE			0x00
#define OHS900_LS_CONN_STATE			0x01
#define OHS900_FS_CONN_STATE			0x02

#define OHS900_HSCTLREG_HOST_EN_MASK		(1<<0)
#define OHS900_HSCTLREG_RESET_CORE		(1<<1)

#define OHS900_HS_CTL_INIT OHS900_HSCTLREG_HOST_EN_MASK

#define OHS900_FIFO_FORCE_EMPTY			0x01
