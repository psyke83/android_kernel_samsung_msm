/* linux/include/asm-arm/arch-msm/vreg.h
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ARCH_ARM_MACH_MSM_VREG_H
#define __ARCH_ARM_MACH_MSM_VREG_H

struct vreg;

struct vreg *vreg_get(struct device *dev, const char *id);
void vreg_put(struct vreg *vreg);

int vreg_enable(struct vreg *vreg);
int vreg_disable(struct vreg *vreg);
int vreg_set_level(struct vreg *vreg, unsigned mv);

#ifdef CONFIG_MAX8899_CHARGER
/* OUT voltage setting */
typedef enum {
	OUT1200mV = 0,
	OUT1500mV,
	OUT1600mV,
	OUT1800mV,
	OUT2100mV,
	OUT2500mV,
	OUT2600mV,
	OUT2650mV,
	OUT2700mV,
	OUT2800mV,
	OUT2850mV,
	OUT2900mV,
	OUT3000mV,
	OUT3100mV,
	OUT3300mV,
	FAIL_VOLT,
	OUT1200uV = 0,
	OUT1500uV = 1000,
	OUT1600uV = 2000,
	OUT1800uV = 3000,
	OUT2100uV = 4000,
	OUT2500uV = 5000,
	OUT2600uV = 6000,
	OUT2650uV = 7000,
	OUT2700uV = 8000,
	OUT2800uV = 9000,
	OUT2850uV = 10000,
	OUT2900uV = 11000,
	OUT3000uV = 12000,
	OUT3100uV = 13000,
	OUT3300uV = 14000
} out_voltage_type;
#endif

#endif
