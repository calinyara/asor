/*
 * processor control and status function
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */

#include <libcflat.h>
#include <asm/processor.h>
#include <asm/ptrace.h>
#include <asm/setup.h>
#include <asm/barrier.h>

static struct {
	void (*func)(struct pt_regs *, void *data);
	void *data;
} handlers[16];

void handle_exception(int trap, void (*func)(struct pt_regs *, void *),
		      void * data)
{
	trap >>= 8;

	if (trap < 16) {
		handlers[trap].func = func;
		handlers[trap].data = data;
	}
}

void do_handle_exception(struct pt_regs *regs)
{
	unsigned char v;

	v = regs->trap >> 8;

	if (v < 16 && handlers[v].func) {
		handlers[v].func(regs, handlers[v].data);
		return;
	}

	printf("unhandled cpu exception %#lx\n", regs->trap);
	abort();
}

void delay(uint64_t cycles)
{
	uint64_t start = get_tb();

	while ((get_tb() - start) < cycles)
		cpu_relax();
}

void udelay(uint64_t us)
{
	delay((us * tb_hz) / 1000000);
}
