// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <stdint.h>

typedef enum mcolor {
	MC_BLACK = 0,
	MC_DARK_BLUE = 1,
	MC_DARK_GREEN = 2,
	MC_DARK_CYAN = 3,
	MC_DARK_RED = 4,
	MC_DARK_MAGENTA = 5,
	MC_DARK_YELLOW = 6,
	MC_LIGHT_GREY = 7,
	MC_DARK_GREY = 8,
	MC_LIGHT_BLUE = 9,
	MC_LIGHT_GREEN = 10,
	MC_LIGHT_CYAN = 11,
	MC_LIGHT_RED = 12,
	MC_LIGHT_MAGENTA = 13,
	MC_LIGHT_YELLOW  = 14,
	MC_WHITE = 15
} mcolor_t;

/* Write a single character out to the screen with color. */
void monitor_put(char c, mcolor_t bc, mcolor_t fc);

/* Clear the screen to all black. */
void monitor_clear(void);

/* Output a null-terminated ASCII string to the monitor with color. */
void monitor_write_color(const char *str, mcolor_t bc, mcolor_t fc);

/* Output a null-terminated ASCII string to the monitor with
 * black backround and white foreground. 
 */
void monitor_write(const char *str);

/* Output a hexadecimal to the monitor. */
void monitor_write_hex(uint32_t n, mcolor_t bc, mcolor_t fc);

/* Output a decimal to the monitor. */
void monitor_write_dec(uint32_t n, mcolor_t bc, mcolor_t fc);

#endif /* __MONITOR_H__ */
