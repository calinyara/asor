// SPDX-License-Identifier: GPL-2.0
/*
 * Defines functions for writing to the monitor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "asm/io.h"
#include "monitor.h"

/* The VGA framebuffer starts at 0xB8000. */
static uint16_t *video_memory = (uint16_t *)0xB8000;
/* Stores the cursor position. */
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

/* Updates the hardware cursor. */
static void move_cursor(void)
{
	/* The screen is 80 characters wide... */
	uint16_t cursorLocation = cursor_y * 80 + cursor_x;
	outb(14, 0x3D4);                  /* Tell the VGA board we are setting the high cursor byte. */
	outb(cursorLocation >> 8, 0x3D5); /* Send the high cursor byte. */
	outb(15, 0x3D4);                  /* Tell the VGA board we are setting the low cursor byte. */
	outb(cursorLocation, 0x3D5);      /* Send the low cursor byte. */
}

/* Scrolls the text on the screen up by one line. */
static void scroll(void)
{

	/* Get a space character with the black background and white font attributes. */
	uint8_t attribute_byte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	uint16_t blank = 0x20 /* space */ | (attribute_byte << 8);

	/* Row 25 is the end, this means we need to scroll up */
	if(cursor_y >= 25) {
		/* Move the current text chunk that makes up the screen
		 * back in the buffer by a line
		 */
		int i;
		for (i = 0 * 80; i < 24 * 80; i++)
			video_memory[i] = video_memory[i + 80];

		/* The last line should now be blank. Do this by writing
		 * 80 spaces to it.
		 */
		for (i = 24 * 80; i < 25 * 80; i++)
			video_memory[i] = blank;

		/* The cursor should now be on the last line. */
		cursor_y = 24;
	}
}

/* Write a single character out to the screen with color. */
void monitor_put(char c, mcolor_t bc, mcolor_t fc)
{
	/* The background color is black (0), the foreground is white (15). */
	uint8_t bcolor = (uint8_t)bc;
	uint8_t fcolor = (uint8_t)fc;

	/* The attribute byte is made up of two nibbles - the lower being the
	 * foreground color, and the upper the background colour.
	 *
	 * The attribute byte is the top 8 bits of the word we have to
	 * send to theVGA board.
	 */
	uint8_t attribute_byte = (bcolor << 4) | (fcolor & 0x0F);
	uint16_t attribute = attribute_byte << 8;

	/* Handle a backspace, by moving the cursor back one space */
	if (c == 0x08 && cursor_x) {
		cursor_x--;
	}
	/* Handle a tab by increasing the cursor's X, but only
	 * to a point where it is divisible by 8.
	 */
	else if (c == 0x09) {
		cursor_x = (cursor_x + 8) & ~(8 - 1);
	}
	/* Handle carriage return */
	else if (c == '\r') {
		cursor_x = 0;
	}
	/* Handle newline by moving cursor back to left
	 * and increasing the row
	 */
	else if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	}
	/* Handle any other printable character. */
	else if(c >= ' ') {
		video_memory[cursor_y * 80 + cursor_x] = c | attribute;
		cursor_x++;
	}

	/* Check if we need to insert a new line because we have
	 * reached the end of the screen.
	 */
	if (cursor_x >= 80) {
		cursor_x = 0;
		cursor_y ++;
	}

	/* Scroll the screen if needed. */
	scroll();
	/* Move the hardware cursor. */
	move_cursor();
}

/* Clears the screen, by copying lots of spaces to the framebuffer. */
void monitor_clear(void)
{
	/* Make an attribute byte for the default colors */
	uint8_t attribute_byte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	uint16_t blank = 0x20 /* space */ | (attribute_byte << 8);

	int i;
	for (i = 0; i < 80 * 25; i++)
		video_memory[i] = blank;

	/* Move the hardware cursor back to the start. */
	cursor_x = 0;
	cursor_y = 0;
	move_cursor();
}

/* Output a null-terminated ASCII string to the monitor with color. */
void monitor_write_color(const char *str, mcolor_t bc, mcolor_t fc)
{
	while (*str)
		monitor_put(*str++, bc, fc);
}

/* Output a null-terminated ASCII string to the monitor with
 * black backround and white foreground. 
 */
void monitor_write(const char *str)
{
	monitor_write_color(str, MC_BLACK, MC_WHITE);
}

void monitor_write_hex(uint32_t n, mcolor_t bc, mcolor_t fc)
{
	int tmp;
	char no_zeroes = 1;

	monitor_write_color("0x", bc, fc);

	int i;
	for (i = 28; i > 0; i -= 4) {
		tmp = (n >> i) & 0xF;
		if (tmp == 0 && no_zeroes != 0)
			continue;
		if (tmp >= 0xA) {
			no_zeroes = 0;
			monitor_put(tmp - 0xA + 'a', bc, fc);
		} else {
			no_zeroes = 0;
			monitor_put(tmp + '0', bc, fc);
		}
	}

	tmp = n & 0xF;
	if (tmp >= 0xA)
		monitor_put(tmp - 0xA + 'a', bc, fc);
	else
		monitor_put(tmp + '0', bc, fc);
}

void monitor_write_dec(uint32_t n, mcolor_t bc, mcolor_t fc)
{
	if (n == 0) {
		monitor_put('0', bc, fc);
		return;
	}

	uint32_t acc = n;
	char c[32];
	int i = 0;
	while (acc > 0) {
		c[i] = '0' + acc % 10;
		acc /= 10;
		i++;
	}

	c[i] = 0;
	char c2[32];
	c2[i--] = 0;
	int j = 0;
	while(i >= 0)
		c2[i--] = c[j++];

	monitor_write_color(c2, bc, fc);
}
