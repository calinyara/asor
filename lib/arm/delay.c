/*
 * Delay loops
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <asm/processor.h>
#include <asm/barrier.h>
#include <asm/delay.h>

void delay(u64 cycles)
{
	u64 start = get_cntvct();

	while ((get_cntvct() - start) < cycles)
		cpu_relax();
}

void udelay(unsigned long usec)
{
	delay((u64)usec * get_cntfrq() / 1000000);
}

void mdelay(unsigned long msecs)
{
	while (msecs--)
		udelay(1000);
}
