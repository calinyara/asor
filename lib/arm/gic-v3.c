/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/gic.h>
#include <asm/io.h>

void gicv3_set_redist_base(size_t stride)
{
	u32 aff = mpidr_compress(get_mpidr());
	u64 typer;
	int i = 0;

	while (gicv3_data.redist_bases[i]) {
		void *ptr = gicv3_data.redist_bases[i];
		do {
			typer = gicv3_read_typer(ptr + GICR_TYPER);
			if ((typer >> 32) == aff) {
				gicv3_redist_base() = ptr;
				return;
			}
			ptr += stride; /* skip RD_base, SGI_base, etc. */
		} while (!(typer & GICR_TYPER_LAST));
		++i;
	}

	/* should never reach here */
	assert(0);
}

void gicv3_enable_defaults(void)
{
	void *dist = gicv3_dist_base();
	void *sgi_base;
	unsigned int i;

	gicv3_data.irq_nr = GICD_TYPER_IRQS(readl(dist + GICD_TYPER));
	if (gicv3_data.irq_nr > 1020)
		gicv3_data.irq_nr = 1020;

	writel(0, dist + GICD_CTLR);
	gicv3_dist_wait_for_rwp();

	writel(GICD_CTLR_ARE_NS | GICD_CTLR_ENABLE_G1A | GICD_CTLR_ENABLE_G1,
	       dist + GICD_CTLR);
	gicv3_dist_wait_for_rwp();

	for (i = 0; i < gicv3_data.irq_nr; i += 4)
		writel(~0, dist + GICD_IGROUPR + i);

	if (!gicv3_redist_base())
		gicv3_set_redist_base(SZ_64K * 2);
	sgi_base = gicv3_sgi_base();

	writel(~0, sgi_base + GICR_IGROUPR0);

	for (i = 0; i < 16; i += 4)
		writel(GICD_INT_DEF_PRI_X4, sgi_base + GICR_IPRIORITYR0 + i);

	writel(GICD_INT_EN_SET_SGI, sgi_base + GICR_ISENABLER0);

	gicv3_write_pmr(GICC_INT_PRI_THRESHOLD);
	gicv3_write_grpen1(1);
}

u32 gicv3_iar_irqnr(u32 iar)
{
	return iar & ((1 << 24) - 1);
}

void gicv3_ipi_send_mask(int irq, const cpumask_t *dest)
{
	u16 tlist;
	int cpu;

	assert(irq < 16);

	/*
	 * For each cpu in the mask collect its peers, which are also in
	 * the mask, in order to form target lists.
	 */
	for_each_cpu(cpu, dest) {
		u64 mpidr = cpus[cpu], sgi1r;
		u64 cluster_id;

		/*
		 * GICv3 can send IPIs to up 16 peer cpus with a single
		 * write to ICC_SGI1R_EL1 (using the target list). Peers
		 * are cpus that have nearly identical MPIDRs, the only
		 * difference being Aff0. The matching upper affinity
		 * levels form the cluster ID.
		 */
		cluster_id = mpidr & ~0xffUL;
		tlist = 0;

		/*
		 * Sort of open code for_each_cpu in order to have a
		 * nested for_each_cpu loop.
		 */
		while (cpu < nr_cpus) {
			if ((mpidr & 0xff) >= 16) {
				printf("cpu%d MPIDR:aff0 is %d (>= 16)!\n",
					cpu, (int)(mpidr & 0xff));
				break;
			}

			tlist |= 1 << (mpidr & 0xf);

			cpu = cpumask_next(cpu, dest);
			if (cpu >= nr_cpus)
				break;

			mpidr = cpus[cpu];

			if (cluster_id != (mpidr & ~0xffUL)) {
				/*
				 * The next cpu isn't in our cluster. Roll
				 * back the cpu index allowing the outer
				 * for_each_cpu to find it again with
				 * cpumask_next
				 */
				--cpu;
				break;
			}
		}

		/* Send the IPIs for the target list of this cluster */
		sgi1r = (MPIDR_TO_SGI_AFFINITY(cluster_id, 3)	|
			 MPIDR_TO_SGI_AFFINITY(cluster_id, 2)	|
			 irq << 24				|
			 MPIDR_TO_SGI_AFFINITY(cluster_id, 1)	|
			 tlist);

		gicv3_write_sgi1r(sgi1r);
	}

	/* Force the above writes to ICC_SGI1R_EL1 to be executed */
	isb();
}

void gicv3_ipi_send_single(int irq, int cpu)
{
	cpumask_t dest;

	cpumask_clear(&dest);
	cpumask_set_cpu(cpu, &dest);
	gicv3_ipi_send_mask(irq, &dest);
}
