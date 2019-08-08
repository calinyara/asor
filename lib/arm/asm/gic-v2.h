/*
 * All GIC* defines are lifted from include/linux/irqchip/arm-gic.h
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#ifndef _ASMARM_GIC_V2_H_
#define _ASMARM_GIC_V2_H_

#ifndef _ASMARM_GIC_H_
#error Do not directly include <asm/gic-v2.h>. Include <asm/gic.h>
#endif

#define GICD_ENABLE			0x1

#define GICC_ENABLE			0x1
#define GICC_IAR_INT_ID_MASK		0x3ff

#ifndef __ASSEMBLY__
#include <asm/cpumask.h>

struct gicv2_data {
	void *dist_base;
	void *cpu_base;
	unsigned int irq_nr;
};
extern struct gicv2_data gicv2_data;

#define gicv2_dist_base()		(gicv2_data.dist_base)
#define gicv2_cpu_base()		(gicv2_data.cpu_base)

extern int gicv2_init(void);
extern void gicv2_enable_defaults(void);
extern u32 gicv2_read_iar(void);
extern u32 gicv2_iar_irqnr(u32 iar);
extern void gicv2_write_eoir(u32 irqstat);
extern void gicv2_ipi_send_single(int irq, int cpu);
extern void gicv2_ipi_send_mask(int irq, const cpumask_t *dest);

#endif /* !__ASSEMBLY__ */
#endif /* _ASMARM_GIC_V2_H_ */
