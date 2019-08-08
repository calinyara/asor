#ifndef _ASMPOWERPC_PROCESSOR_H_
#define _ASMPOWERPC_PROCESSOR_H_

#include <libcflat.h>
#include <asm/ptrace.h>

#ifndef __ASSEMBLY__
void handle_exception(int trap, void (*func)(struct pt_regs *, void *), void *);
void do_handle_exception(struct pt_regs *regs);
#endif /* __ASSEMBLY__ */

static inline uint64_t get_tb(void)
{
	uint64_t tb;

	asm volatile ("mfspr %[tb],268" : [tb] "=r" (tb));

	return tb;
}

extern void delay(uint64_t cycles);
extern void udelay(uint64_t us);

static inline void mdelay(uint64_t ms)
{
	while (ms--)
		udelay(1000);
}

#endif /* _ASMPOWERPC_PROCESSOR_H_ */
