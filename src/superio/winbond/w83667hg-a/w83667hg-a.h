/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2012 Advanced Micro Devices, Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SUPERIO_WINBOND_W83667HG_A_W83667HG_A_H
#define SUPERIO_WINBOND_W83667HG_A_W83667HG_A_H

#define W83667HG_A_FDC          0x00
#define W83667HG_A_PP           0x01
#define W83667HG_A_SP1          0x02 /* Com1 */
#define W83667HG_A_SP2          0x03 /* Com2 */
#define W83667HG_A_KBC          0x05
#define W83667HG_A_SPI          0x06
#define W83667HG_A_GPIO6789_V   0x07
#define W83667HG_A_WDT1         0x08
#define W83667HG_A_GPIO2345_V   0x09
#define W83667HG_A_ACPI         0x0A
#define W83667HG_A_HWM_TSI      0x0B /* HW monitor/SB-TSI/deep S5 */
#define W83667HG_A_PECI         0x0C
#define W83667HG_A_VID_BUSSEL   0x0D /* VID and BUSSEL */
#define W83667HG_A_GPIO_PP_OD   0x0F /* GPIO Push-Pull/Open drain select */

/* The following are handled using "virtual LDNs" (hence the _V suffix). */
#define W83667HG_A_SPI1                 ((1 << 8) | W83667HG_A_SPI)
#define W83667HG_A_GPIO1                ((1 << 8) | W83667HG_A_WDT1)
#define W83667HG_A_GPIO2                ((0 << 8) | W83667HG_A_GPIO2345_V)
#define W83667HG_A_GPIO3                ((1 << 8) | W83667HG_A_GPIO2345_V)
#define W83667HG_A_GPIO4                ((2 << 8) | W83667HG_A_GPIO2345_V)
#define W83667HG_A_GPIO5                ((3 << 8) | W83667HG_A_GPIO2345_V)
#define W83667HG_A_GPIO6                ((1 << 8) | W83667HG_A_GPIO6789_V)
#define W83667HG_A_GPIO7                ((2 << 8) | W83667HG_A_GPIO6789_V)
#define W83667HG_A_GPIO8                ((3 << 8) | W83667HG_A_GPIO6789_V)
#define W83667HG_A_GPIO9                ((4 << 8) | W83667HG_A_GPIO6789_V)

void pnp_enter_ext_func_mode(device_t dev);
void pnp_exit_ext_func_mode(device_t dev);
void w83667hg_a_enable_serial(device_t dev, u16 iobase);
void w83667hg_a_enable_i2c(device_t dev);
void w83667hg_a_set_clksel_48(device_t dev);

#endif
