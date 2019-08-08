/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/gic.h>
#include <asm/io.h>

void gicv2_enable_defaults(void)
{
	void *dist = gicv2_dist_base();
	void *cpu_base = gicv2_cpu_base();
	unsigned int i;

	gicv2_data.irq_nr = GICD_TYPER_IRQS(readl(dist + GICD_TYPER));
	if (gicv2_data.irq_nr > 1020)
		gicv2_data.irq_nr = 1020;

	for (i = 0; i < gicv2_data.irq_nr; i += 4)
		writel(GICD_INT_DEF_PRI_X4, dist + GICD_IPRIORITYR + i);

	writel(GICD_INT_EN_SET_SGI, dist + GICD_ISENABLER + 0);
	writel(GICD_ENABLE, dist + GICD_CTLR);

	writel(GICC_INT_PRI_THRESHOLD, cpu_base + GICC_PMR);
	writel(GICC_ENABLE, cpu_base + GICC_CTLR);
}

u32 gicv2_read_iar(void)
{
	return readl(gicv2_cpu_base() + GICC_IAR);
}

u32 gicv2_iar_irqnr(u32 iar)
{
	return iar & GICC_IAR_INT_ID_MASK;
}

void gicv2_write_eoir(u32 irqstat)
{
	writel(irqstat, gicv2_cpu_base() + GICC_EOIR);
}

void gicv2_ipi_send_single(int irq, int cpu)
{
	assert(cpu < 8);
	assert(irq < 16);
	writel(1 << (cpu + 16) | irq, gicv2_dist_base() + GICD_SGIR);
}

void gicv2_ipi_send_mask(int irq, const cpumask_t *dest)
{
	u8 tlist = (u8)cpumask_bits(dest)[0];

	assert(irq < 16);
	writel(tlist << 16 | irq, gicv2_dist_base() + GICD_SGIR);
}
