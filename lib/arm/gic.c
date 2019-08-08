/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <devicetree.h>
#include <asm/gic.h>
#include <asm/io.h>

struct gicv2_data gicv2_data;
struct gicv3_data gicv3_data;

struct gic_common_ops {
	void (*enable_defaults)(void);
	u32 (*read_iar)(void);
	u32 (*iar_irqnr)(u32 iar);
	void (*write_eoir)(u32 irqstat);
	void (*ipi_send_single)(int irq, int cpu);
	void (*ipi_send_mask)(int irq, const cpumask_t *dest);
};

static const struct gic_common_ops *gic_common_ops;

static const struct gic_common_ops gicv2_common_ops = {
	.enable_defaults = gicv2_enable_defaults,
	.read_iar = gicv2_read_iar,
	.iar_irqnr = gicv2_iar_irqnr,
	.write_eoir = gicv2_write_eoir,
	.ipi_send_single = gicv2_ipi_send_single,
	.ipi_send_mask = gicv2_ipi_send_mask,
};

static const struct gic_common_ops gicv3_common_ops = {
	.enable_defaults = gicv3_enable_defaults,
	.read_iar = gicv3_read_iar,
	.iar_irqnr = gicv3_iar_irqnr,
	.write_eoir = gicv3_write_eoir,
	.ipi_send_single = gicv3_ipi_send_single,
	.ipi_send_mask = gicv3_ipi_send_mask,
};

/*
 * Documentation/devicetree/bindings/interrupt-controller/arm,gic.txt
 * Documentation/devicetree/bindings/interrupt-controller/arm,gic-v3.txt
 */
static bool
gic_get_dt_bases(const char *compatible, void **base1, void **base2)
{
	struct dt_pbus_reg reg;
	struct dt_device gic;
	struct dt_bus bus;
	int node, ret, i;

	dt_bus_init_defaults(&bus);
	dt_device_init(&gic, &bus, NULL);

	node = dt_device_find_compatible(&gic, compatible);
	assert(node >= 0 || node == -FDT_ERR_NOTFOUND);

	if (node == -FDT_ERR_NOTFOUND)
		return false;

	dt_device_bind_node(&gic, node);

	ret = dt_pbus_translate(&gic, 0, &reg);
	assert(ret == 0);
	*base1 = ioremap(reg.addr, reg.size);

	for (i = 0; i < GICV3_NR_REDISTS; ++i) {
		ret = dt_pbus_translate(&gic, i + 1, &reg);
		if (ret == -FDT_ERR_NOTFOUND)
			break;
		assert(ret == 0);
		base2[i] = ioremap(reg.addr, reg.size);
	}

	return true;
}

int gicv2_init(void)
{
	return gic_get_dt_bases("arm,cortex-a15-gic",
			&gicv2_data.dist_base, &gicv2_data.cpu_base);
}

int gicv3_init(void)
{
	return gic_get_dt_bases("arm,gic-v3", &gicv3_data.dist_base,
			&gicv3_data.redist_bases[0]);
}

int gic_version(void)
{
	if (gic_common_ops == &gicv2_common_ops)
		return 2;
	else if (gic_common_ops == &gicv3_common_ops)
		return 3;
	return 0;
}

int gic_init(void)
{
	if (gicv2_init())
		gic_common_ops = &gicv2_common_ops;
	else if (gicv3_init())
		gic_common_ops = &gicv3_common_ops;
	return gic_version();
}

void gic_enable_defaults(void)
{
	if (!gic_common_ops) {
		int ret = gic_init();
		assert(ret != 0);
	} else
		assert(gic_common_ops->enable_defaults);
	gic_common_ops->enable_defaults();
}

u32 gic_read_iar(void)
{
	assert(gic_common_ops && gic_common_ops->read_iar);
	return gic_common_ops->read_iar();
}

u32 gic_iar_irqnr(u32 iar)
{
	assert(gic_common_ops && gic_common_ops->iar_irqnr);
	return gic_common_ops->iar_irqnr(iar);
}

void gic_write_eoir(u32 irqstat)
{
	assert(gic_common_ops && gic_common_ops->write_eoir);
	gic_common_ops->write_eoir(irqstat);
}

void gic_ipi_send_single(int irq, int cpu)
{
	assert(gic_common_ops && gic_common_ops->ipi_send_single);
	gic_common_ops->ipi_send_single(irq, cpu);
}

void gic_ipi_send_mask(int irq, const cpumask_t *dest)
{
	assert(gic_common_ops && gic_common_ops->ipi_send_mask);
	gic_common_ops->ipi_send_mask(irq, dest);
}
