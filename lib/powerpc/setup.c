/*
 * Initialize machine setup information and I/O.
 *
 * After running setup() unit tests may query how many cpus they have
 * (nr_cpus), how much memory they have (PHYSICAL_END - PHYSICAL_START),
 * may use dynamic memory allocation (malloc, etc.), printf, and exit.
 * Finally, argc and argv are also ready to be passed to main().
 *
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <libfdt/libfdt.h>
#include <devicetree.h>
#include <alloc.h>
#include <alloc_phys.h>
#include <argv.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/hcall.h>
#include "io.h"

extern unsigned long stacktop;

char *initrd;
u32 initrd_size;

u32 cpus[NR_CPUS] = { [0 ... NR_CPUS-1] = (~0U) };
int nr_cpus;
uint64_t tb_hz;

struct mem_region mem_regions[NR_MEM_REGIONS];
phys_addr_t __physical_start, __physical_end;
unsigned __icache_bytes, __dcache_bytes;

struct cpu_set_params {
	unsigned icache_bytes;
	unsigned dcache_bytes;
	uint64_t tb_hz;
};

#define EXCEPTION_STACK_SIZE	(32*1024) /* 32kB */

static char exception_stack[NR_CPUS][EXCEPTION_STACK_SIZE];

static void cpu_set(int fdtnode, u64 regval, void *info)
{
	static bool read_common_info = false;
	struct cpu_set_params *params = info;
	int cpu = nr_cpus++;

	assert_msg(cpu < NR_CPUS, "Number cpus exceeds maximum supported (%d).", NR_CPUS);

	cpus[cpu] = regval;

	/* set exception stack address for this CPU (in SPGR0) */
	asm volatile ("mtsprg0 %[addr]" ::
		      [addr] "r" (exception_stack[cpu + 1]));

	if (!read_common_info) {
		const struct fdt_property *prop;
		u32 *data;

		prop = fdt_get_property(dt_fdt(), fdtnode,
					"i-cache-line-size", NULL);
		assert(prop != NULL);
		data = (u32 *)prop->data;
		params->icache_bytes = fdt32_to_cpu(*data);

		prop = fdt_get_property(dt_fdt(), fdtnode,
					"d-cache-line-size", NULL);
		assert(prop != NULL);
		data = (u32 *)prop->data;
		params->dcache_bytes = fdt32_to_cpu(*data);

		prop = fdt_get_property(dt_fdt(), fdtnode,
					"timebase-frequency", NULL);
		assert(prop != NULL);
		data = (u32 *)prop->data;
		params->tb_hz = fdt32_to_cpu(*data);

		read_common_info = true;
	}
}

static void cpu_init(void)
{
	struct cpu_set_params params;
	int ret;

	nr_cpus = 0;
	ret = dt_for_each_cpu_node(cpu_set, &params);
	assert(ret == 0);
	__icache_bytes = params.icache_bytes;
	__dcache_bytes = params.dcache_bytes;
	tb_hz = params.tb_hz;

	/* Interrupt Endianness */

#if  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        hcall(H_SET_MODE, 1, 4, 0, 0);
#else
        hcall(H_SET_MODE, 0, 4, 0, 0);
#endif
}

static void mem_init(phys_addr_t freemem_start)
{
	struct dt_pbus_reg regs[NR_MEM_REGIONS];
	struct mem_region primary, mem = {
		.start = (phys_addr_t)-1,
	};
	int nr_regs, i;

	nr_regs = dt_get_memory_params(regs, NR_MEM_REGIONS);
	assert(nr_regs > 0);

	primary.end = 0;

	for (i = 0; i < nr_regs; ++i) {
		mem_regions[i].start = regs[i].addr;
		mem_regions[i].end = regs[i].addr + regs[i].size;

		/*
		 * pick the region we're in for our primary region
		 */
		if (freemem_start >= mem_regions[i].start
				&& freemem_start < mem_regions[i].end) {
			mem_regions[i].flags |= MR_F_PRIMARY;
			primary = mem_regions[i];
		}

		/*
		 * set the lowest and highest addresses found,
		 * ignoring potential gaps
		 */
		if (mem_regions[i].start < mem.start)
			mem.start = mem_regions[i].start;
		if (mem_regions[i].end > mem.end)
			mem.end = mem_regions[i].end;
	}
	assert(primary.end != 0);
//	assert(!(mem.start & ~PHYS_MASK) && !((mem.end - 1) & ~PHYS_MASK));

	__physical_start = mem.start;	/* PHYSICAL_START */
	__physical_end = mem.end;	/* PHYSICAL_END */

	phys_alloc_init(freemem_start, primary.end - freemem_start);
	phys_alloc_set_minimum_alignment(__icache_bytes > __dcache_bytes
					 ? __icache_bytes : __dcache_bytes);
}

void setup(const void *fdt)
{
	void *freemem = &stacktop;
	const char *bootargs, *tmp;
	u32 fdt_size;
	int ret;

	/*
	 * Before calling mem_init we need to move the fdt and initrd
	 * to safe locations. We move them to construct the memory
	 * map illustrated below:
	 *
	 * +----------------------+   <-- top of physical memory
	 * |                      |
	 * ~                      ~
	 * |                      |
	 * +----------------------+   <-- top of initrd
	 * |                      |
	 * +----------------------+   <-- top of FDT
	 * |                      |
	 * +----------------------+   <-- top of cpu0's stack
	 * |                      |
	 * +----------------------+   <-- top of text/data/bss/toc sections,
	 * |                      |       see powerpc/flat.lds
	 * |                      |
	 * +----------------------+   <-- load address
	 * |                      |
	 * +----------------------+
	 */
	fdt_size = fdt_totalsize(fdt);
	ret = fdt_move(fdt, freemem, fdt_size);
	assert(ret == 0);
	ret = dt_init(freemem);
	assert(ret == 0);
	freemem += fdt_size;

	ret = dt_get_initrd(&tmp, &initrd_size);
	assert(ret == 0 || ret == -FDT_ERR_NOTFOUND);
	if (ret == 0) {
		initrd = freemem;
		memmove(initrd, tmp, initrd_size);
		freemem += initrd_size;
	}

	/* call init functions */
	cpu_init();

	/* cpu_init must be called before mem_init */
	mem_init(PAGE_ALIGN((unsigned long)freemem));

	/* mem_init must be called before io_init */
	io_init();

	/* finish setup */
	ret = dt_get_bootargs(&bootargs);
	assert(ret == 0 || ret == -FDT_ERR_NOTFOUND);
	setup_args_progname(bootargs);

	if (initrd) {
		/* environ is currently the only file in the initrd */
		char *env = malloc(initrd_size);
		memcpy(env, initrd, initrd_size);
		setup_env(env, initrd_size);
	}
}
