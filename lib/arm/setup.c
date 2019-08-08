/*
 * Initialize machine setup information and I/O.
 *
 * After running setup() unit tests may query how many cpus they have
 * (nr_cpus), how much memory they have (PHYS_END - PHYS_OFFSET), may
 * use dynamic memory allocation (malloc, etc.), printf, and exit.
 * Finally, argc and argv are also ready to be passed to main().
 *
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <libfdt/libfdt.h>
#include <devicetree.h>
#include <alloc.h>
#include <alloc_phys.h>
#include <alloc_page.h>
#include <argv.h>
#include <asm/thread_info.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/smp.h>

#include "io.h"

extern unsigned long stacktop;

char *initrd;
u32 initrd_size;

u64 cpus[NR_CPUS] = { [0 ... NR_CPUS-1] = (u64)~0 };
int nr_cpus;

struct mem_region mem_regions[NR_MEM_REGIONS];
phys_addr_t __phys_offset, __phys_end;

int mpidr_to_cpu(uint64_t mpidr)
{
	int i;

	for (i = 0; i < nr_cpus; ++i)
		if (cpus[i] == (mpidr & MPIDR_HWID_BITMASK))
			return i;
	return -1;
}

static void cpu_set(int fdtnode __unused, u64 regval, void *info __unused)
{
	int cpu = nr_cpus++;

	assert_msg(cpu < NR_CPUS, "Number cpus exceeds maximum supported (%d).", NR_CPUS);

	cpus[cpu] = regval;
	set_cpu_present(cpu, true);
}

static void cpu_init(void)
{
	int ret;

	nr_cpus = 0;
	ret = dt_for_each_cpu_node(cpu_set, NULL);
	assert(ret == 0);
	set_cpu_online(0, true);
}

static void mem_init(phys_addr_t freemem_start)
{
	struct dt_pbus_reg regs[NR_MEM_REGIONS];
	struct mem_region primary, mem = {
		.start = (phys_addr_t)-1,
	};
	phys_addr_t base, top;
	int nr_regs, i;

	nr_regs = dt_get_memory_params(regs, NR_MEM_REGIONS);
	assert(nr_regs > 0);

	primary = (struct mem_region){ 0 };

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
	assert(!(mem.start & ~PHYS_MASK) && !((mem.end - 1) & ~PHYS_MASK));

	__phys_offset = primary.start;	/* PHYS_OFFSET */
	__phys_end = primary.end;	/* PHYS_END */

	phys_alloc_init(freemem_start, primary.end - freemem_start);
	phys_alloc_set_minimum_alignment(SMP_CACHE_BYTES);

	phys_alloc_get_unused(&base, &top);
	base = PAGE_ALIGN(base);
	top = top & PAGE_MASK;
	assert(sizeof(long) == 8 || !(base >> 32));
	if (sizeof(long) != 8 && (top >> 32) != 0)
		top = ((uint64_t)1 << 32);
	free_pages((void *)(unsigned long)base, top - base);
	page_alloc_ops_enable();
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
	 *    +----------------------+   <-- top of physical memory
	 *    |                      |
	 *    ~                      ~
	 *    |                      |
	 *    +----------------------+   <-- top of initrd
	 *    |                      |
	 *    +----------------------+   <-- top of FDT
	 *    |                      |
	 *    +----------------------+   <-- top of cpu0's stack
	 *    |                      |
	 *    +----------------------+   <-- top of text/data/bss sections,
	 *    |                      |       see arm/flat.lds
	 *    |                      |
	 *    +----------------------+   <-- load address
	 *    |                      |
	 *    +----------------------+
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
	mem_init(PAGE_ALIGN((unsigned long)freemem));
	cpu_init();

	/* cpu_init must be called before thread_info_init */
	thread_info_init(current_thread_info(), 0);

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
