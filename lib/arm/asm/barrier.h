#ifndef _ASMARM_BARRIER_H_
#define _ASMARM_BARRIER_H_
/*
 * Adapted from arch/arm/include/asm/barrier.h
 *
 * Copyright (C) 2017, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 */

#define sev()		asm volatile("sev" : : : "memory")
#define wfe()		asm volatile("wfe" : : : "memory")
#define wfi()		asm volatile("wfi" : : : "memory")
#define yield()		asm volatile("yield" : : : "memory")
#define cpu_relax()	yield()

#define isb(option) __asm__ __volatile__ ("isb " #option : : : "memory")
#define dsb(option) __asm__ __volatile__ ("dsb " #option : : : "memory")
#define dmb(option) __asm__ __volatile__ ("dmb " #option : : : "memory")

#define mb()		dsb()
#define rmb()		dsb()
#define wmb()		dsb(st)
#define smp_mb()	dmb(ish)
#define smp_rmb()	smp_mb()
#define smp_wmb()	dmb(ishst)

#endif /* _ASMARM_BARRIER_H_ */
