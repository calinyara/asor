#include "x86/msr.h"
#include "x86/processor.h"
#include "x86/apic-defs.h"
#include "x86/apic.h"
#include "x86/desc.h"
#include "x86/isr.h"
#include "alloc.h"
#include "setjmp.h"
#include "usermode.h"

#include "libcflat.h"
#include <stdint.h>

#define USERMODE_STACK_SIZE	0x2000
#define RET_TO_KERNEL_IRQ	0x20

jmp_buf jmpbuf;

static void restore_exec_to_jmpbuf(void)
{
	longjmp(jmpbuf, 1);
}

static void restore_exec_to_jmpbuf_exception_handler(struct ex_regs *regs)
{
	/* longjmp must happen after iret, so do not do it now.  */
	regs->rip = (unsigned long)&restore_exec_to_jmpbuf;
	regs->cs = KERNEL_CS;
}

uint64_t run_in_user(usermode_func func, unsigned int fault_vector,
		uint64_t arg1, uint64_t arg2, uint64_t arg3,
		uint64_t arg4, bool *raised_vector)
{
	extern char ret_to_kernel;
	uint64_t rax = 0;
	static unsigned char user_stack[USERMODE_STACK_SIZE];

	*raised_vector = 0;
	set_idt_entry(RET_TO_KERNEL_IRQ, &ret_to_kernel, 3);
	handle_exception(fault_vector,
			restore_exec_to_jmpbuf_exception_handler);

	if (setjmp(jmpbuf) != 0) {
		*raised_vector = 1;
		return 0;
	}

	asm volatile (
			/* Backing Up Stack in rdi */
			"mov %%rsp, %%rdi\n\t"
			/* Load user_ds to DS and ES */
			"mov %[user_ds], %%ax\n\t"
			"mov %%ax, %%ds\n\t"
			"mov %%ax, %%es\n\t"
			/* IRET into user mode */
			"pushq %[user_ds]\n\t"
			"pushq %[user_stack_top]\n\t"
			"pushfq\n\t"
			"pushq %[user_cs]\n\t"
			"pushq $user_mode\n\t"
			"iretq\n"

			"user_mode:\n\t"
			/* Back up registers before invoking func */
			"push %%rbx\n\t"
			"push %%rcx\n\t"
			"push %%rdx\n\t"
			"push %%r8\n\t"
			"push %%r9\n\t"
			"push %%r10\n\t"
			"push %%r11\n\t"
			"push %%rdi\n\t"
			"push %%rsi\n\t"
			/* Call user mode function */
			"mov %[arg1], %%rdi\n\t"
			"mov %[arg2], %%rsi\n\t"
			"mov %[arg3], %%rdx\n\t"
			"mov %[arg4], %%rcx\n\t"
			"call *%[func]\n\t"
			/* Restore registers */
			"pop %%rsi\n\t"
			"pop %%rdi\n\t"
			"pop %%r11\n\t"
			"pop %%r10\n\t"
			"pop %%r9\n\t"
			"pop %%r8\n\t"
			"pop %%rdx\n\t"
			"pop %%rcx\n\t"
			"pop %%rbx\n\t"
			/* Return to kernel via system call */
			"int %[kernel_entry_vector]\n\t"
			/* Kernel Mode */
			"ret_to_kernel:\n\t"
			"mov %%rdi, %%rsp\n\t"
			:
			"+a"(rax)
			:
			[arg1]"m"(arg1),
			[arg2]"m"(arg2),
			[arg3]"m"(arg3),
			[arg4]"m"(arg4),
			[func]"m"(func),
			[user_ds]"i"(USER_DS),
			[user_cs]"i"(USER_CS),
			[user_stack_top]"r"(user_stack +
					sizeof(user_stack)),
			[kernel_entry_vector]"i"(RET_TO_KERNEL_IRQ)
			:
			"rsi", "rdi", "rcx", "rdx");

	return rax;
}
