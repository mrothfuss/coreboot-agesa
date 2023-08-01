/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <arch/io.h>
#include <device/device.h>
#include <device/pnp.h>
#include <uart8250.h>
#include <pc80/keyboard.h>
#include <stdlib.h>
#include "chip.h"
#include "w83667hg-a.h"

void pnp_enter_ext_func_mode(device_t dev)
{
	outb(0x87, dev->path.pnp.port);
	outb(0x87, dev->path.pnp.port);
}

void pnp_exit_ext_func_mode(device_t dev)
{
	outb(0xaa, dev->path.pnp.port);
}

static void w83667hg_a_init(device_t dev)
{
	struct superio_winbond_w83667hg_a_config *conf = dev->chip_info;

	if (!dev->enabled)
		return;

	/*
	switch(dev->path.pnp.device) {
	case W83667HG_A_KBC:
		pc_keyboard_init(&conf->keyboard);
		break;
	}
	*/
}

static void w83667hg_a_pnp_set_resources(device_t dev)
{
	pnp_enter_ext_func_mode(dev);
	pnp_set_resources(dev);
	pnp_exit_ext_func_mode(dev);
}

static void w83667hg_a_pnp_enable_resources(device_t dev)
{
	pnp_enter_ext_func_mode(dev);
	pnp_enable_resources(dev);
	pnp_exit_ext_func_mode(dev);
}

static void w83667hg_a_pnp_enable(device_t dev)
{
	pnp_enter_ext_func_mode(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, !!dev->enabled);
	pnp_exit_ext_func_mode(dev);
}

static struct device_operations ops = {
	.read_resources   = pnp_read_resources,
	.set_resources    = w83667hg_a_pnp_set_resources,
	.enable_resources = w83667hg_a_pnp_enable_resources,
	.enable           = w83667hg_a_pnp_enable,
	.init             = w83667hg_a_init,
};

static struct pnp_info pnp_dev_info[] = {
	{ &ops, W83667HG_A_FDC, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x0ff8, 0}, },
	{ &ops, W83667HG_A_PP, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x0ff8, 0}, },
	{ &ops, W83667HG_A_SP1, PNP_IO0 | PNP_IRQ0, {0x0ff8, 0}, },
	{ &ops, W83667HG_A_SP2, PNP_IO0 | PNP_IRQ0, {0x0ff8, 0}, },
	{ &ops, W83667HG_A_KBC, PNP_IO0 | PNP_IO1 | PNP_IRQ0 | PNP_IRQ1, {0x0fff, 0}, {0x0fff, 4}, },
	{ &ops, W83667HG_A_SPI1, PNP_IO1, {}, {0x0ff8, 0}},
	{ &ops, W83667HG_A_WDT1},
	{ &ops, W83667HG_A_ACPI},
	{ &ops, W83667HG_A_HWM_TSI, PNP_IO0 | PNP_IRQ0, {0x0ffe, 0}, },
	{ &ops, W83667HG_A_PECI},
	{ &ops, W83667HG_A_VID_BUSSEL},
	{ &ops, W83667HG_A_GPIO_PP_OD},
	{ &ops, W83667HG_A_GPIO1},
	{ &ops, W83667HG_A_GPIO2},
	{ &ops, W83667HG_A_GPIO3},
	{ &ops, W83667HG_A_GPIO4},
	{ &ops, W83667HG_A_GPIO5},
	{ &ops, W83667HG_A_GPIO6},
	{ &ops, W83667HG_A_GPIO7},
	{ &ops, W83667HG_A_GPIO8},
	{ &ops, W83667HG_A_GPIO9},
};

static void enable_dev(struct device *dev)
{
	pnp_enable_devices(dev, &ops, ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
}

struct chip_operations superio_winbond_w83667hg_a_ops = {
	CHIP_NAME("Winbond W83667HG-A Super I/O")
	.enable_dev = enable_dev,
};
