// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "libcflat.h"
#include "virt.h"

static const struct virt_arch_ops *virt_arch_ops = NULL;

int register_virt_ops(const struct virt_arch_ops *ops)
{
	if (virt_arch_ops != NULL || ops == NULL)
		return -1;

	virt_arch_ops = ops;
	return 0;
}

void free_virt_ops(const struct virt_arch_ops *ops)
{
	virt_arch_ops = NULL;
}

const struct virt_arch_ops *get_virt_arch_ops(void)
{
	return virt_arch_ops;
}

int enable_hw_virt(void)
{
	const struct virt_arch_ops *ops = get_virt_arch_ops();

	if (ops == NULL) {
		error("Error: virt_arch_ops is NULL.\n");
		return -1;
	}

	return ops->enable();
}

