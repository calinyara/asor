#ifndef _ASMARM_PROCESSOR_H_
#define _ASMARM_PROCESSOR_H_
/*
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/ptrace.h>
#include <asm/sysreg.h>
#include <asm/barrier.h>

enum vector {
	EXCPTN_RST,
	EXCPTN_UND,
	EXCPTN_SVC,
	EXCPTN_PABT,
	EXCPTN_DABT,
	EXCPTN_ADDREXCPTN,
	EXCPTN_IRQ,
	EXCPTN_FIQ,
	EXCPTN_MAX,
};

typedef void (*exception_fn)(struct pt_regs *);
extern void install_exception_handler(enum vector v, exception_fn fn);

extern void show_regs(struct pt_regs *regs);

static inline unsigned long current_cpsr(void)
{
	unsigned long cpsr;
	asm volatile("mrs %0, cpsr" : "=r" (cpsr));
	return cpsr;
}

#define current_mode() (current_cpsr() & MODE_MASK)

static inline void local_irq_enable(void)
{
	asm volatile("cpsie i" : : : "memory", "cc");
}

static inline void local_irq_disable(void)
{
	asm volatile("cpsid i" : : : "memory", "cc");
}

#define MPIDR __ACCESS_CP15(c0, 0, c0, 5)
static inline uint64_t get_mpidr(void)
{
	return read_sysreg(MPIDR);
}

#define MPIDR_HWID_BITMASK 0xffffff
extern int mpidr_to_cpu(uint64_t mpidr);

#define MPIDR_LEVEL_SHIFT(level) \
	(((1 << level) >> 1) << 3)
#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
	((mpidr >> MPIDR_LEVEL_SHIFT(level)) & 0xff)

extern void start_usr(void (*func)(void *arg), void *arg, unsigned long sp_usr);
extern bool is_user(void);

#define CNTVCT		__ACCESS_CP15_64(1, c14)
#define CNTFRQ		__ACCESS_CP15(c14, 0, c0, 0)

static inline u64 get_cntvct(void)
{
	isb();
	return read_sysreg(CNTVCT);
}

static inline u32 get_cntfrq(void)
{
	return read_sysreg(CNTFRQ);
}

#endif /* _ASMARM_PROCESSOR_H_ */
