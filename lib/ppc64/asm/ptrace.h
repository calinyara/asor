#ifndef _ASMPPC64_PTRACE_H_
#define _ASMPPC64_PTRACE_H_

#define KERNEL_REDZONE_SIZE	288
#define STACK_FRAME_OVERHEAD    112     /* size of minimum stack frame */

#ifndef __ASSEMBLY__
struct pt_regs {
	unsigned long gpr[32];
	unsigned long nip;
	unsigned long msr;
	unsigned long ctr;
	unsigned long link;
	unsigned long xer;
	unsigned long ccr;
	unsigned long trap;
};

#define STACK_INT_FRAME_SIZE    (sizeof(struct pt_regs) + \
				 STACK_FRAME_OVERHEAD + KERNEL_REDZONE_SIZE)

#endif /* __ASSEMBLY__ */

#endif /* _ASMPPC64_PTRACE_H_ */
