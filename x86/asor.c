// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "libcflat.h"
#include "virt.h"
#include "vmx.h"

int asor_entry(void);
int asor_entry(void)
{
	setup_vm();

	printf("Hello ASOR.\n");

	register_virt_ops(get_x86_virt_arch_ops());
	enable_hw_virt();

	while (1) {
		asm volatile ("hlt");
	}

	return 0;
}
