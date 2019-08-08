/*
 * Each architecture must implement puts() and exit().
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <asm/spinlock.h>
#include <asm/rtas.h>
#include <asm/setup.h>
#include "io.h"

static struct spinlock print_lock;

void io_init(void)
{
	rtas_init();
}

void puts(const char *s)
{
	spin_lock(&print_lock);
	while (*s)
		putchar(*s++);
	spin_unlock(&print_lock);
}

/*
 * Defining halt to take 'code' as an argument guarantees that it will
 * be in r3 when we halt. That gives us a final chance to see the exit
 * status while inspecting the halted unit test state.
 */
extern void halt(int code);

void exit(int code)
{
// FIXME: change this print-exit/rtas-poweroff to chr_testdev_exit(),
//        maybe by plugging chr-testdev into a spapr-vty.
	printf("\nEXIT: STATUS=%d\n", ((code) << 1) | 1);
	rtas_power_off();
	halt(code);
	__builtin_unreachable();
}
