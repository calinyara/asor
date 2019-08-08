/*
 * powerpc RTAS
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <libfdt/libfdt.h>
#include <devicetree.h>
#include <asm/spinlock.h>
#include <asm/hcall.h>
#include <asm/io.h>
#include <asm/rtas.h>

extern void enter_rtas(unsigned long);

unsigned long rtas_entry;
static struct rtas_args rtas_args;
static struct spinlock rtas_lock;

static int rtas_node(void)
{
	int node = fdt_path_offset(dt_fdt(), "/rtas");

	if (node < 0) {
		printf("%s: /rtas: %s\n", __func__, fdt_strerror(node));
		abort();
	}

	return node;
}

void rtas_init(void)
{
	bool broken_sc1 = hcall_have_broken_sc1();
	int node = rtas_node(), len, words, i;
	const struct fdt_property *prop;
	u32 *data, *insns;

	if (!dt_available()) {
		printf("%s: No device tree!\n", __func__);
		abort();
	}

	prop = fdt_get_property(dt_fdt(), node,
				"linux,rtas-entry", &len);
	if (!prop) {
		printf("%s: /rtas/linux,rtas-entry: %s\n",
				__func__, fdt_strerror(len));
		abort();
	}
	data = (u32 *)prop->data;
	rtas_entry = (unsigned long)fdt32_to_cpu(*data);
	insns = (u32 *)rtas_entry;

	prop = fdt_get_property(dt_fdt(), node, "rtas-size", &len);
	if (!prop) {
		printf("%s: /rtas/rtas-size: %s\n",
				__func__, fdt_strerror(len));
		abort();
	}
	data = (u32 *)prop->data;
	words = (int)fdt32_to_cpu(*data)/4;

	for (i = 0; i < words; ++i) {
		if (broken_sc1 && insns[i] == cpu_to_be32(SC1))
			insns[i] = cpu_to_be32(SC1_REPLACEMENT);
	}
}

int rtas_token(const char *service, uint32_t *token)
{
	const struct fdt_property *prop;
	u32 *data;

	if (!dt_available())
		return RTAS_UNKNOWN_SERVICE;

	prop = fdt_get_property(dt_fdt(), rtas_node(), service, NULL);
	if (!prop)
		return RTAS_UNKNOWN_SERVICE;

	data = (u32 *)prop->data;
	*token = fdt32_to_cpu(*data);

	return 0;
}

int rtas_call(int token, int nargs, int nret, int *outputs, ...)
{
	va_list list;
	int ret, i;

	spin_lock(&rtas_lock);

	rtas_args.token = cpu_to_be32(token);
	rtas_args.nargs = cpu_to_be32(nargs);
	rtas_args.nret = cpu_to_be32(nret);
	rtas_args.rets = &rtas_args.args[nargs];

	va_start(list, outputs);
	for (i = 0; i < nargs; ++i)
		rtas_args.args[i] = cpu_to_be32(va_arg(list, u32));
	va_end(list);

	for (i = 0; i < nret; ++i)
		rtas_args.rets[i] = 0;

	enter_rtas(__pa(&rtas_args));

	if (nret > 1 && outputs != NULL)
		for (i = 0; i < nret - 1; ++i)
			outputs[i] = be32_to_cpu(rtas_args.rets[i + 1]);

	ret = nret > 0 ? be32_to_cpu(rtas_args.rets[0]) : 0;

	spin_unlock(&rtas_lock);
	return ret;
}

void rtas_power_off(void)
{
	uint32_t token;
	int ret;

	ret = rtas_token("power-off", &token);
	if (ret) {
		puts("RTAS power-off not available\n");
		return;
	}

	ret = rtas_call(token, 2, 1, NULL, -1, -1);
	printf("RTAS power-off returned %d\n", ret);
}
