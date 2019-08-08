#ifndef _ASMARM_DELAY_H_
#define _ASMARM_DELAY_H_
/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>

extern void delay(u64 cycles);
extern void udelay(unsigned long usecs);
extern void mdelay(unsigned long msecs);

#endif /* _ASMARM_DELAY_H_ */
