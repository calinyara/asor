// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#ifndef __VIRT_H__
#define __VIRT_H__

struct virt_arch_ops {
	int (*enable)(void);
};

int enable_hw_virt(void);
int register_virt_ops(const struct virt_arch_ops *ops);
void free_virt_ops(const struct virt_arch_ops *ops);
const struct virt_arch_ops *get_virt_arch_ops(void);

#endif /* __VIRT_H__ */
