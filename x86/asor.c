// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "libcflat.h"
#include "virt.h"
#include "vmx.h"
#include "smp.h"
#include "desc.h"
#include "isr.h"
#include "msr.h"
#include "atomic.h"

int asor_entry(void);
int asor_entry(void)
{
        u64 efer, cr0, cr4;

        cr0 = read_cr0();
        cr4 = read_cr4();
        printf("0.cr0 = %lx\n", cr0);
        printf("0.cr4 = %lx\n", cr4);
        printf("0.msr_efer=%lx\n", rdmsr(MSR_EFER));

//        setup_vm();
        setup_idt();

        cr0 &= (~X86_CR0_PG);
        printf("cr0 = %lx\n", cr0);
        write_cr0(cr0);

        cr0 = read_cr0();
        printf("1.cr0 = %lx\n", cr0);
        printf("1.cr4 = %lx\n", cr4);
        printf("1.msr_efer=%lx\n", rdmsr(MSR_EFER));

#if 0
	setup_vm();

	printf("Hello ASOR.\n");

	register_virt_ops(get_x86_virt_arch_ops());
	enable_hw_virt();

	while (1) {
		asm volatile ("hlt");
	}
#endif

	return 0;
}
