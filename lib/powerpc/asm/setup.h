#ifndef _ASMPOWERPC_SETUP_H_
#define _ASMPOWERPC_SETUP_H_
/*
 * Copyright (C) 2016, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>

#define NR_CPUS			8	/* arbitrarily set for now */
extern u32 cpus[NR_CPUS];
extern int nr_cpus;

extern uint64_t tb_hz;

#define NR_MEM_REGIONS		8
#define MR_F_PRIMARY		(1U << 0)
struct mem_region {
	phys_addr_t start;
	phys_addr_t end;
	unsigned int flags;
};
extern struct mem_region mem_regions[NR_MEM_REGIONS];
extern phys_addr_t __physical_start, __physical_end;
extern unsigned __icache_bytes, __dcache_bytes;

#define PHYSICAL_START		(__physical_start)
#define PHYSICAL_END		(__physical_end)

void setup(const void *fdt);

#endif /* _ASMPOWERPC_SETUP_H_ */
