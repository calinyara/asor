/*
 * s390x io implementation
 *
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  Thomas Huth <thuth@redhat.com>
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#include <libcflat.h>
#include <argv.h>
#include <asm/spinlock.h>
#include <asm/facility.h>
#include <asm/sigp.h>
#include "sclp.h"

extern char ipl_args[];
uint8_t stfl_bytes[NR_STFL_BYTES] __attribute__((aligned(8)));

static struct spinlock lock;

void setup(void);

void puts(const char *s)
{
	spin_lock(&lock);
	sclp_print(s);
	spin_unlock(&lock);
}

void setup(void)
{
	setup_args_progname(ipl_args);
	setup_facilities();
	sclp_console_setup();
	sclp_memory_setup();
}

void exit(int code)
{
	printf("\nEXIT: STATUS=%d\n", ((code) << 1) | 1);
	while (1) {
		sigp_stop();
	}
}
