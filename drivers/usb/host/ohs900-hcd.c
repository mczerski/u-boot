/*
 * OpenCores "OHS900" USB Host driver, http://opencores.org/project,usbhostslave
 *
 * Copyright (C) 2011 Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 *
 * Based on sl811-hcd.c
 *
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * and the out of tree Linux driver ohs900-hcd.c:
 *
 * Copyright (C) 2005 Steve Fielding
 * Copyright (C) 2004 Psion Teklogix (for NetBook PRO)
 * Copyright (C) 2004-2005 David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include "ohs900-hcd.h"

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS		0x0080
#define RH_CLEAR_FEATURE	0x0100
#define RH_SET_FEATURE		0x0300
#define RH_SET_ADDRESS		0x0500
#define RH_GET_DESCRIPTOR	0x0680
#define RH_SET_DESCRIPTOR	0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE		0x0280
#define RH_GET_INTERFACE	0x0A80
#define RH_SET_INTERFACE	0x0B00
#define RH_SYNC_FRAME		0x0C80

#define OHS900_RETRY_COUNT 3

static int root_hub_devnum = 0;
static struct usb_port_status rh_status = { 0 };/* root hub port status */
static struct ohs900_regs *ohs900;

static int ohs900_rh_submit_urb(struct usb_device *usb_dev, ulong pipe,
				void *data, int buf_len,
				struct devrequest *cmd);

static int ohs900_reset(void)
{
	u8 connstate;

	writeb(OHS900_HSCTLREG_RESET_CORE, &ohs900->hostslavectlreg);
	wait_ms(20);
	/* Disable SOF generation and interrupts */
	writeb(0, &ohs900->irq_enable);
	writeb(0, &ohs900->sofenreg);
	writeb(OHS900_HS_CTL_INIT, &ohs900->hostslavectlreg);
	connstate = readb(&ohs900->rxconnstatereg);
	switch (connstate) {
	case OHS900_DISCONNECT_STATE:
		debug("No device connected\n");
		rh_status.wPortStatus &= ~USB_PORT_STAT_CONNECTION;
		rh_status.wPortStatus &= ~USB_PORT_STAT_ENABLE;
		rh_status.wPortChange |= USB_PORT_STAT_C_CONNECTION;
		return 0;

	case OHS900_LS_CONN_STATE:
		debug("Low speed device connected\n");
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION;
		rh_status.wPortStatus |= USB_PORT_STAT_LOW_SPEED;
		break;

	case OHS900_FS_CONN_STATE:
		debug("Full speed device connected\n");
		rh_status.wPortStatus |= USB_PORT_STAT_CONNECTION;
		rh_status.wPortStatus &= ~USB_PORT_STAT_LOW_SPEED;
		break;
	}
	rh_status.wPortChange |= USB_PORT_STAT_C_CONNECTION;

	return 1;
}
/*
 * runs a 50 ms bus reset by taking direct control over the D-/D+ lines
 */
void ohs900_bus_reset(void)
{
	debug("Bus reset\n");
	writeb(0, &ohs900->sofenreg);
	writeb(OHS900_TXLCTL_MASK_SE0, &ohs900->txlinectlreg);
	wait_ms(50);
	if (readb(&ohs900->rxconnstatereg) == OHS900_LS_CONN_STATE)
		writeb(OHS900_TXLCTL_MASK_LSPD, &ohs900->txlinectlreg);
	else
		writeb(OHS900_TXLCTL_MASK_FSPD, &ohs900->txlinectlreg);
	writeb(OHS900_MASK_SOF_ENA, &ohs900->sofenreg);
}

static inline void ohs900_write_txfifo(u8 *buffer, int len)
{
	while (len--)
		writeb(*buffer++, &ohs900->host_txfifo_data);
}

static inline void ohs900_read_rxfifo(u8 *buffer, int len)
{
	while (len--)
		*buffer++ = readb(&ohs900->host_rxfifo_data);
}

static inline void ohs900_wait_transreq(void)
{
	while (readb(&ohs900->host_tx_ctlreg) & OHS900_HCTLMASK_TRANS_REQ)
		udelay(1);
}

static inline int ohs900_check_error(u8 status)
{
		if (status & OHS900_STATMASK_STALL_RXED)
			return USB_ST_STALLED;
		if (status & OHS900_STATMASK_RX_OVF)
			return USB_ST_BUF_ERR;
		if (status & OHS900_STATMASK_BS_ERROR)
			return USB_ST_BIT_ERR;
		return USB_ST_CRC_ERR;
}

/* Check if there is enough time in this frame left before SOF */
static int ohs900_need_sofsync(ulong pipe, int len)
{
	u16 ticks_left;
	u16 ticks_needed;
	int preamble = !(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
		usb_pipeslow(pipe);
	/*
	 * SOF generation happens every 1 ms,
	 * with a 48 MHz clock that gives 48000 ticks per SOF.
	 */
	ticks_left = 48000 - (readb(&ohs900->softmrreg) << 8);

	/* account for overhead */
	if (preamble)
		len += 24;
	else
		len += 18;

	if (preamble || (rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED))
		ticks_needed = (48000000 / ((1536 * 1024) / 8)) * len;
	else
		ticks_needed = (48000000 / ((12288 * 1024) / 8)) * len;

	return (ticks_needed >= ticks_left) ? 1 : 0;
}

static int send_setup_packet(struct usb_device *dev, ulong pipe,
				struct devrequest *setup)
{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int len = sizeof(*setup);
	u8 *data = (u8 *)setup;
	int retry = OHS900_RETRY_COUNT;
	u8 control = OHS900_HCTLMASK_TRANS_REQ;
	u8 status;

	debug("%s: ep = %d, len = %d, devnum = %d\n",
		__func__, ep, len, devnum);

	while (retry) {
		writeb(OHS900_FIFO_FORCE_EMPTY, &ohs900->txfifocontrolreg);
		ohs900_write_txfifo(data, len);
		writeb(OHS900_SETUP, &ohs900->txtranstypereg);
		writeb(ep, &ohs900->txendpreg);
		writeb(devnum, &ohs900->txaddrreg);

		if (ohs900_need_sofsync(pipe, len))
			control |= OHS900_HCTLMASK_SOF_SYNC;

		if (!(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
			usb_pipeslow(pipe))
			control |= OHS900_HCTLMASK_PREAMBLE_EN;

		writeb(control, &ohs900->host_tx_ctlreg);
		ohs900_wait_transreq();

		status = readb(&ohs900->hrxstatreg);
		if (status &  OHS900_STATMASK_NAK_RXED)
			continue;

		if (status & OHS900_STATMASK_ACK_RXED)
			break;

		retry--;
		udelay(1);
	}

	if (!retry) {
		debug("Setup packet failed! (status = %02x)\n", status);
		return -ohs900_check_error(status);
	}

	return 0;
}

static int send_status_packet(ulong pipe)
{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int retry = OHS900_RETRY_COUNT;
	u8 control = OHS900_HCTLMASK_TRANS_REQ;
	u8 status;
	int do_out;

	debug("send_status_packet, ep = %d, devnum = %d\n", ep, devnum);
	while (retry) {
		do_out = usb_pipein(pipe);
		writeb((do_out ? OHS900_OUT_DATA1 : OHS900_IN),
			&ohs900->txtranstypereg);

		writeb(ep, &ohs900->txendpreg);
		writeb(devnum, &ohs900->txaddrreg);

		if (ohs900_need_sofsync(pipe, 0))
			control |= OHS900_HCTLMASK_SOF_SYNC;

		if (!(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
			usb_pipeslow(pipe))
			control |= OHS900_HCTLMASK_PREAMBLE_EN;

		writeb(control, &ohs900->host_tx_ctlreg);
		ohs900_wait_transreq();

		status = readb(&ohs900->hrxstatreg);
		if (status &  OHS900_STATMASK_NAK_RXED)
			continue;

		if (status & OHS900_STATMASK_ACK_RXED)
			break;

		retry--;
		udelay(1);
	}

	if (!retry) {
		debug("Status packet failed! (status = %02x)\n", status);
		return -ohs900_check_error(status);
	}

	return 0;
}

static int send_out_packet(struct usb_device *dev, ulong pipe,
			   u8 *buffer, int len)
{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int act_len = len;
	int retry = OHS900_RETRY_COUNT;
	u8 control = OHS900_HCTLMASK_TRANS_REQ;
	u8 status;

	debug("%s: ep = %d, len = %d, devnum = %d\n",
		__func__, ep, len, devnum);

	while (retry) {
		writeb(OHS900_FIFO_FORCE_EMPTY, &ohs900->txfifocontrolreg);
		if (usb_gettoggle(dev, ep, 1))
			writeb(OHS900_OUT_DATA1, &ohs900->txtranstypereg);
		else
			writeb(OHS900_OUT_DATA0, &ohs900->txtranstypereg);
		ohs900_write_txfifo(buffer, len);

		writeb(ep, &ohs900->txendpreg);
		writeb(devnum, &ohs900->txaddrreg);

		if (ohs900_need_sofsync(pipe, len))
			control |= OHS900_HCTLMASK_SOF_SYNC;

		if (!(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
			usb_pipeslow(pipe))
			control |= OHS900_HCTLMASK_PREAMBLE_EN;

		writeb(control, &ohs900->host_tx_ctlreg);
		ohs900_wait_transreq();

		status = readb(&ohs900->hrxstatreg);
		if (status &  OHS900_STATMASK_NAK_RXED)
			continue;

		if (status & OHS900_STATMASK_ACK_RXED)
			break;

		retry--;
		udelay(1);
	}

	if (!retry) {
		debug("Out packet failed (status = %02x)\n", status);
		return -ohs900_check_error(status);
	}
	return act_len;
}

static int send_in_packet(struct usb_device *dev, ulong pipe,
			  u8 *buffer, int len)
{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	u16 act_len;
	u8 control = OHS900_HCTLMASK_TRANS_REQ;
	u8 status;
	int retry = OHS900_RETRY_COUNT;

	debug("%s: ep = %d, len = %d, devnum = %d\n",
		__func__, ep, len, devnum);

	while (retry) {
		writeb(OHS900_FIFO_FORCE_EMPTY, &ohs900->rxfifocontrolreg);
		writeb(OHS900_IN, &ohs900->txtranstypereg);
		writeb(ep, &ohs900->txendpreg);
		writeb(devnum, &ohs900->txaddrreg);

		if (ohs900_need_sofsync(pipe, len))
			control |= OHS900_HCTLMASK_SOF_SYNC;

		if (!(rh_status.wPortStatus & USB_PORT_STAT_LOW_SPEED) &&
			usb_pipeslow(pipe))
			control |= OHS900_HCTLMASK_PREAMBLE_EN;

		writeb(control, &ohs900->host_tx_ctlreg);
		ohs900_wait_transreq();

		status = readb(&ohs900->hrxstatreg);
		act_len = readb(&ohs900->rxfifocntlsbreg)
			| (readb(&ohs900->rxfifocntmsbreg) << 8);

		if (status &  OHS900_STATMASK_NAK_RXED)
			continue;

		if ((status & OHS900_STATMASK_ACK_RXED) ||
			((status & ~OHS900_STATMASK_DATA_SEQ) == 0)) {
			ohs900_read_rxfifo(buffer, act_len);
			break;
		}
		retry--;
		udelay(1);
	}

	if (!retry) {
		debug("In packet failed! (status = %02x)\n", status);
		return -ohs900_check_error(status);
	}

	return (int)act_len;
}

int usb_lowlevel_init(void)
{
	ohs900 = (struct ohs900_regs *)CONFIG_OHS900_HCD_BASE;
	ohs900_reset();
	ohs900_bus_reset();
	debug("ohs900 controller version 0x%02x found\n",
		readb(&ohs900->hwrevreg));

	return 0;
}

int usb_lowlevel_stop(void)
{
	ohs900_reset();

	return 0;
}

int submit_control_msg(struct usb_device *dev, ulong pipe, void *buffer,
			int len, struct devrequest *setup)

{
	int devnum = usb_pipedevice(pipe);
	int ep = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);
	int res;
	int act_len = 0;

	dev->status = 0;
	dev->act_len = 0;

	if (devnum == root_hub_devnum)
		return ohs900_rh_submit_urb(dev, pipe, buffer, len, setup);

	usb_settoggle(dev, ep, 1, 0);
	res = send_setup_packet(dev, pipe, setup);
	if (res < 0) {
		dev->status = -res;
		return 0;
	}

	usb_settoggle(dev, ep, usb_pipeout(pipe), 1);

	while (act_len < len) {
		if (usb_pipein(pipe))
			res = send_in_packet(dev, pipe,
					(u8 *)(buffer + act_len),
					max > (len - act_len) ?
					(len - act_len) : max);
		else
			res = send_out_packet(dev, pipe,
					(u8 *)(buffer + act_len),
					max > (len - act_len) ?
					(len - act_len) : max);

		if (res < 0) {
			dev->status = -res;
			return 0;
		}
		act_len += res;
		usb_dotoggle(dev, ep, usb_pipeout(pipe));

		if (usb_pipein(pipe) && res < max)
			break;
	}

	send_status_packet(pipe);
	usb_settoggle(dev, ep, !usb_pipeout(pipe), 1);

	dev->act_len = act_len;

	return act_len;
}

int submit_int_msg(struct usb_device *dev, ulong pipe, void *buffer,
		   int len, int interval)
{
	debug("dev = %p pipe = %#lx buf = %p size = %d int = %d\n",
		dev, pipe, buffer, len, interval);
	return -1;
}

int submit_bulk_msg(struct usb_device *dev, ulong pipe, void *buffer, int len)
{
	int act_len = 0;
	int res;
	int ep = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);

	debug("submit_bulk_msg: len = %d, dir = %s, max = %d\n",
		len, usb_pipein(pipe) ? "in" : "out", max);
	dev->status = 0;

	while (act_len < len) {
		if (usb_pipein(pipe))
			res = send_in_packet(dev, pipe,
					(u8 *)(buffer + act_len),
					max > (len - act_len) ?
					(len - act_len) : max);

		else
			res = send_out_packet(dev, pipe,
					(u8 *)(buffer + act_len),
					max > (len - act_len) ?
					(len - act_len) : max);
		if (res < 0) {
			dev->status = -res;
			return res;
		}
		act_len += res;
		usb_dotoggle(dev, ep, usb_pipeout(pipe));
		if (usb_pipein(pipe) && res < max)
			break;
	}
	dev->act_len = act_len;

	debug("submit_bulk_msg: act_len = %d\n", act_len);

	return 0;
}

/*
 * Virtual Root Hub
 */

/* Device descriptor */
static u8 root_hub_dev_des[] = {
	0x12,	/* __u8  bLength; */
	0x01,	/* __u8  bDescriptorType; Device */
	0x10,	/* __u16 bcdUSB; v1.1 */
	0x01,
	0x09,	/* __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	/* __u8  bDeviceSubClass; */
	0x00,	/* __u8  bDeviceProtocol; */
	0x08,	/* __u8  bMaxPacketSize0; 8 Bytes */
	0x00,	/* __u16 idVendor; */
	0x00,
	0x00,	/* __u16 idProduct; */
	0x00,
	0x00,	/* __u16 bcdDevice; */
	0x00,
	0x00,	/* __u8  iManufacturer; */
	0x01,	/* __u8  iProduct; */
	0x00,	/* __u8  iSerialNumber; */
	0x01	/* __u8  bNumConfigurations; */
};

/* Configuration descriptor */
static u8 root_hub_config_des[] = {
	0x09,	/* __u8  bLength; */
	0x02,	/* __u8  bDescriptorType; Configuration */
	0x19,	/* __u16 wTotalLength; */
	0x00,
	0x01,	/* __u8  bNumInterfaces; */
	0x01,	/* __u8  bConfigurationValue; */
	0x00,	/* __u8  iConfiguration; */
	0x40,	/*
		 *__u8  bmAttributes;
		 * Bit 7: Bus-powered, 6: Self-powered,
		 * 5 Remote-wakwup, 4..0: resvd
		 */
	0x00,	/* __u8  MaxPower; */

	/* interface */
	0x09,	/* __u8  if_bLength; */
	0x04,	/* __u8  if_bDescriptorType; Interface */
	0x00,	/* __u8  if_bInterfaceNumber; */
	0x00,	/* __u8  if_bAlternateSetting; */
	0x01,	/* __u8  if_bNumEndpoints; */
	0x09,	/* __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,	/* __u8  if_bInterfaceSubClass; */
	0x00,	/* __u8  if_bInterfaceProtocol; */
	0x00,	/* __u8  if_iInterface; */

	/* endpoint */
	0x07,	/* __u8  ep_bLength; */
	0x05,	/* __u8  ep_bDescriptorType; Endpoint */
	0x81,	/* __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,	/* __u8  ep_bmAttributes; Interrupt */
	0x02,	/* __u16 ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
	0x00,
	0xff	/* __u8  ep_bInterval; 255 ms */
};

/* root hub class descriptor*/
static u8 root_hub_hub_des[] = {
	0x09,	/*  __u8  bLength; */
	0x29,	/*  __u8  bDescriptorType; Hub-descriptor */
	0x01,	/*  __u8  bNbrPorts; */
	0x00,	/* __u16  wHubCharacteristics; */
	0x00,
	0x50,	/*  __u8  bPwrOn2pwrGood; 2ms */
	0x00,	/*  __u8  bHubContrCurrent; 0 mA */
	0xfc,	/*  __u8  DeviceRemovable; ** 7 Ports max ** */
	0xff	/*  __u8  PortPwrCtrlMask; ** 7 ports max ** */
};

/*
 * helper routine for returning string descriptors in UTF-16LE
 * input can actually be ISO-8859-1; ASCII is its 7-bit subset
 */
static int ascii2utf(char *s, u8 *utf, int utfmax)
{
	int retval;

	for (retval = 0; *s && utfmax > 1; utfmax -= 2, retval += 2) {
		*utf++ = *s++;
		*utf++ = 0;
	}
	return retval;
}

/*
 * root_hub_string is used by each host controller's root hub code,
 * so that they're identified consistently throughout the system.
 */
static int usb_root_hub_string(int id, int serial, char *type, u8 *data,
				int len)
{
	char buf[30];

	/* language ids */
	if (id == 0) {
		*data++ = 4; *data++ = 3;	/* 4 bytes data */
		*data++ = 0; *data++ = 0;	/* some language id */
		return 4;

	/* serial number */
	} else if (id == 1) {
		sprintf(buf, "%#x", serial);

	/* product description */
	} else if (id == 2) {
		sprintf(buf, "USB %s Root Hub", type);

	/* id 3 == vendor description */

	/* unsupported IDs --> "stall" */
	} else
	    return 0;

	ascii2utf(buf, data + 2, len - 2);
	data[0] = 2 + strlen(buf) * 2;
	data[1] = 3;
	return data[0];
}

static int ohs900_rh_submit_urb(struct usb_device *usb_dev, unsigned long pipe,
				void *data, int buf_len, struct devrequest *cmd)
{
	u8 data_buf[16];
	u8 *bufp = data_buf;
	int len = 0;
	int status = 0;

	u16 bmRType_bReq;
	u16 value;
	u16 index;
	u16 length;

	if (usb_pipeint(pipe)) {
		debug("interrupt transfer unimplemented!\n");
		return 0;
	}

	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);
	value	      = le16_to_cpu(cmd->value);
	index	      = le16_to_cpu(cmd->index);
	length	      = le16_to_cpu(cmd->length);

	debug("submit rh urb, req = %d(%x) val = %#x index = %#x len=%d\n",
	      bmRType_bReq, bmRType_bReq, value, index, length);

	/*
	 * Request Destination:
	 * without flags: Device,
	 * USB_RECIP_INTERFACE: interface,
	 * USB_RECIP_ENDPOINT: endpoint,
	 * USB_TYPE_CLASS means HUB here,
	 * USB_RECIP_OTHER | USB_TYPE_CLASS  almost ever means HUB_PORT here
	 */
	switch (bmRType_bReq) {
	case RH_GET_STATUS:
		*(u16 *)bufp = cpu_to_le16(1);
		len = 2;
		break;

	case RH_GET_STATUS | USB_RECIP_INTERFACE:
		*(u16 *)bufp = cpu_to_le16(0);
		len = 2;
		break;

	case RH_GET_STATUS | USB_RECIP_ENDPOINT:
		*(u16 *)bufp = cpu_to_le16(0);
		len = 2;
		break;

	case RH_GET_STATUS | USB_TYPE_CLASS:
		*(u32 *)bufp = cpu_to_le32(0);
		len = 4;
		break;

	case RH_GET_STATUS | USB_RECIP_OTHER | USB_TYPE_CLASS:
		*(u32 *)bufp = cpu_to_le32(rh_status.wPortChange<<16 |
						rh_status.wPortStatus);
		len = 4;
		break;

	case RH_CLEAR_FEATURE | USB_RECIP_ENDPOINT:
		switch (value) {
		case 1:
			len = 0;
			break;
		}
		break;

	case RH_CLEAR_FEATURE | USB_TYPE_CLASS:
		switch (value) {
		case C_HUB_LOCAL_POWER:
			len = 0;
			break;

		case C_HUB_OVER_CURRENT:
			len = 0;
			break;
		}
		break;

	case RH_CLEAR_FEATURE | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (value) {
		case USB_PORT_FEAT_ENABLE:
			rh_status.wPortStatus &= ~USB_PORT_STAT_ENABLE;
			len = 0;
			break;

		case USB_PORT_FEAT_SUSPEND:
			rh_status.wPortStatus &= ~USB_PORT_STAT_SUSPEND;
			len = 0;
			break;

		case USB_PORT_FEAT_POWER:
			rh_status.wPortStatus &= ~USB_PORT_STAT_POWER;
			len = 0;
			break;

		case USB_PORT_FEAT_C_CONNECTION:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_CONNECTION;
			len = 0;
			break;

		case USB_PORT_FEAT_C_ENABLE:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_ENABLE;
			len = 0;
			break;

		case USB_PORT_FEAT_C_SUSPEND:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_SUSPEND;
			len = 0;
			break;

		case USB_PORT_FEAT_C_OVER_CURRENT:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_OVERCURRENT;
			len = 0;
			break;

		case USB_PORT_FEAT_C_RESET:
			rh_status.wPortChange &= ~USB_PORT_STAT_C_RESET;
			len = 0;
			break;
		}
		break;

	case RH_SET_FEATURE | USB_RECIP_OTHER | USB_TYPE_CLASS:
		switch (value) {
		case USB_PORT_FEAT_SUSPEND:
			rh_status.wPortStatus |= USB_PORT_STAT_SUSPEND;
			len = 0;
			break;

		case USB_PORT_FEAT_RESET:
			rh_status.wPortStatus |= USB_PORT_STAT_RESET;
			ohs900_bus_reset();
			rh_status.wPortChange = 0;
			rh_status.wPortChange |= USB_PORT_STAT_C_RESET;
			rh_status.wPortStatus &= ~USB_PORT_STAT_RESET;
			rh_status.wPortStatus |= USB_PORT_STAT_ENABLE;
			len = 0;
			break;

		case USB_PORT_FEAT_POWER:
			rh_status.wPortStatus |= USB_PORT_STAT_POWER;
			len = 0;
			break;

		case USB_PORT_FEAT_ENABLE:
			rh_status.wPortStatus |= USB_PORT_STAT_ENABLE;
			len = 0;
			break;
		}
		break;

	case RH_SET_ADDRESS:
		root_hub_devnum = value;
		len = 0;
		break;

	case RH_GET_DESCRIPTOR:
		switch ((value & 0xff00) >> 8) {
		case USB_DT_DEVICE:
			len = sizeof(root_hub_dev_des);
			bufp = root_hub_dev_des;
			break;

		case USB_DT_CONFIG:
			len = sizeof(root_hub_config_des);
			bufp = root_hub_config_des;
			break;

		case USB_DT_STRING:
			len = usb_root_hub_string(value & 0xff, (int)(long)0,
							"OHS900", data, length);
			if (len > 0) {
				bufp = data;
				break;
			}

		default:
			status = -32;
		}
		break;

	case RH_GET_DESCRIPTOR | USB_TYPE_CLASS:
		len = sizeof(root_hub_hub_des);
		bufp = root_hub_hub_des;
		break;

	case RH_GET_CONFIGURATION:
		bufp[0] = 0x01;
		len = 1;
		break;

	case RH_SET_CONFIGURATION:
		len = 0;
		break;

	default:
		debug("unsupported root hub command\n");
		status = -32;
	}

	len = min(len, buf_len);
	if (data != bufp)
		memcpy(data, bufp, len);

	debug("len = %d, status = %d\n", len, status);

	usb_dev->status = status;
	usb_dev->act_len = len;

	return status == 0 ? len : status;
}
