/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 - 2012 Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "Porting.h"
#include "AGESA.h"
#include "amdlib.h"
#include <arch/io.h>
#include <arch/romcc_io.h>
#include <device/pci_ids.h>

AGESA_STATUS AmdMemoryReadSPD (UINT32 unused1, UINT32 unused2, AGESA_READ_SPD_PARAMS *info);
#define DIMENSION(array)(sizeof (array)/ sizeof (array [0]))

/* SP5100 GPIO 53-56 contoled by SMBUS PCI_Reg 0x52 */
#define SP5100_GPIO53_56	0x54

/**
 * TODO not support all GPIO yet
 * @param reg -GPIO Cntrl Register
 * @param out -GPIO bitmap
 * @param out -GPIO enable bitmap
 * @return  old setting
 */

static u8 switch_spd_mux(u8 channel) {
	u8 value, ret;
	device_t sm_dev = PCI_DEV(0, 0x14, 0); //SMBUS
	
	value = pci_read_config8(sm_dev, SP5100_GPIO53_56);
	ret = value;
	value &= ~0xc;                   /* Clear SPD mux GPIOs */
	value &= ~0xc0;                  /* Enable SPD mux GPIO output drivers */
	value |= (channel << 2) & 0xc;   /* Set SPD mux GPIOs */
	pci_write_config8(sm_dev, SP5100_GPIO53_56, value);

	return ret;
}

static u8 sp5100_set_gpio(u8 reg, u8 out, u8 enable)
{
	u8 value, ret;
	device_t sm_dev = PCI_DEV(0, 0x14, 0); //SMBUS

	value = pci_read_config8(sm_dev, reg);
	ret = value;
	value &= ~(enable);
	value |= out;
	value &= ~(enable << 4);
	pci_write_config8(sm_dev, reg, value);

	return ret;
}

static void sp5100_restore_gpio(u8 reg, u8 value)
{
	device_t sm_dev = PCI_DEV(0, 0x14, 0);
	pci_write_config8(sm_dev, reg, value);
}

/*-----------------------------------------------------------------------------
 *
 * SPD address table - porting required
 */
static const UINT8 spdAddressLookup [4] [4] [2] = { // socket, channel, dimm
	/* socket 0 */
	{
		{0x50, 0x51},
		{0x52, 0x53},
		{0x54, 0x55},
		{0x56, 0x57},
	},
	/* socket 1 */
	{
		{0x50, 0x51},
		{0x52, 0x53},
		{0x54, 0x55},
		{0x56, 0x57},
	},
	/* socket 2 */
	{
		{0x50, 0x51},
		{0x52, 0x53},
		{0x54, 0x55},
		{0x56, 0x57},
	},
	/* socket 3 */
	{
		{0x50, 0x51},
		{0x52, 0x53},
		{0x54, 0x55},
		{0x56, 0x57},
	},
};

/*-----------------------------------------------------------------------------
 *
 * readSmbusByteData - read a single SPD byte from any offset
 */

static int readSmbusByteData (int iobase, int address, char *buffer, int offset)
{
	unsigned int status;
	UINT64 limit;

	address |= 1; // set read bit

	outb(0xFF, iobase + 0);                // clear error status
	outb(0x1F, iobase + 1);                // clear error status
	outb(offset, iobase + 3);              // offset in eeprom
	outb(address, iobase + 4);             // slave address and read bit
	outb(0x48, iobase + 2);                // read byte command

	// time limit to avoid hanging for unexpected error status (should never happen)
	limit = __rdtsc () + 2000000000 / 10;
	for (;;)
	{
		status = inb(iobase);
		if (__rdtsc () > limit) break;
		if ((status & 2) == 0) continue;               // SMBusInterrupt not set, keep waiting
		if ((status & 1) == 1) continue;               // HostBusy set, keep waiting
		break;
	}

	buffer [0] = inb(iobase + 5);
	if (status == 2) status = 0;                      // check for done with no errors
	return status;
}

/*-----------------------------------------------------------------------------
 *
 * readSmbusByte - read a single SPD byte from the default offset
 *                 this function is faster function readSmbusByteData
 */

static int readSmbusByte (int iobase, int address, char *buffer)
{
	unsigned int status;
	UINT64 limit;

	outb(0xFF, iobase + 0);                // clear error status
	outb(0x44, iobase + 2);                // read command

	// time limit to avoid hanging for unexpected error status
	limit = __rdtsc () + 2000000000 / 10;
	for (;;)
	{
		status = inb(iobase);
		if (__rdtsc () > limit) break;
		if ((status & 2) == 0) continue;               // SMBusInterrupt not set, keep waiting
		if ((status & 1) == 1) continue;               // HostBusy set, keep waiting
		break;
	}

	buffer [0] = inb(iobase + 5);
	if (status == 2) status = 0;                      // check for done with no errors
	return status;
}

/*---------------------------------------------------------------------------
 *
 * readspd - Read one or more SPD bytes from a DIMM.
 *           Start with offset zero and read sequentially.
 *           Optimization relies on autoincrement to avoid
 *           sending offset for every byte.
 *          Reads 128 bytes in 7-8 ms at 400 KHz.
 */

static int readspd (int iobase, int SmbusSlaveAddress, char *buffer, int count)
{
	int index;

	buffer[0] = do_smbus_read_byte(iobase, SmbusSlaveAddress, 0);
	if(((uint8_t)buffer[0]) == 0xFD) return AGESA_ERROR;
	
	for(index = 1; index < count; index++) {
		buffer[index] = do_smbus_read_byte(iobase, SmbusSlaveAddress, index);
		//printk(BIOS_DEBUG, "readspd: iobase = %08X, address = %08X, index = %08X, value = %02X\n", iobase, SmbusSlaveAddress, index, (uint8_t)buffer[index]);
	}

	return 0;
}

static void setupFch (int ioBase)
{
	outb(66000000 / 400000 / 4, ioBase + 0x0E); /* set SMBus clock to 400 KHz */
}

AGESA_STATUS AmdMemoryReadSPD (UINT32 unused1, UINT32 unused2, AGESA_READ_SPD_PARAMS *info)
{
	AGESA_STATUS status;
	int spdAddress, ioBase, i;
	u8 i2c_channel;
	u8 backup;
	device_t sm_dev;

	if (info->SocketId     >= DIMENSION (spdAddressLookup      )) return AGESA_ERROR;
	if (info->MemChannelId >= DIMENSION (spdAddressLookup[0]   )) return AGESA_ERROR;
	if (info->DimmId       >= DIMENSION (spdAddressLookup[0][0])) return AGESA_ERROR;
	i2c_channel = (UINT8) info->SocketId;

	printk(BIOS_DEBUG, "S=%x, c=%x, d=%x\n", info->SocketId, info->MemChannelId, info->DimmId);

	/* set ght i2c channel
	 * GPIO54,53 control the HC4052 S1,S0
	 *  S1 S0 true table
	 *   0  0   channel 1 (Socket1)
	 *   0  1   channel 2 (Socket2)
	 *   1  0   channel 3 (Socket3)
	 *   1  1   channel 4 (Socket4)
	 */
	if(i2c_channel == 0) {
		backup = switch_spd_mux(0x2);
	} else if(i2c_channel == 1) {
		backup = switch_spd_mux(0x3);
	} else {
		return AGESA_ERROR;
	}

	spdAddress = spdAddressLookup [info->SocketId] [info->MemChannelId] [info->DimmId];
	if (spdAddress == 0)
		return AGESA_ERROR;

	/*
	 * SMBus Base Address was set during southbridge early setup.
	 * e.g. sb700 IO mapped SMBUS_IO_BASE 0x6000, CIMX using 0xB00 as default
	 */
	//sm_dev = pci_locate_device(PCI_ID(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_SB700_SM), 0);
	ioBase = 0xb20;//pci_read_config32(sm_dev, 0x90) & (0xFFFFFFF0);
	//setupFch(ioBase);

	printk(BIOS_DEBUG, "readspd, ioBase = %08X, spdAddress = %02X\n", ioBase, spdAddress);
	status = readspd(ioBase, spdAddress, (void *)info->Buffer, 256);
	for (i=0; i<256; i++) {
		printk(BIOS_DEBUG, "%02X ", info->Buffer[i]);
		if ((i%16)==15)
			printk(BIOS_DEBUG, "\n");
	}
	printk(BIOS_DEBUG, "\n");
	sp5100_restore_gpio(SP5100_GPIO53_56, backup);

	return status;
}
