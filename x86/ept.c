// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "libcflat.h"
#include "alloc_page.h"
#include "fwcfg.h"
#include "vmx.h"
#include "ept.h"

unsigned long *pml4;
u64 eptp;

/* EPT paging structure related functions */
/* split_large_ept_entry: Split a 2M/1G large page into 512 smaller PTEs.
		@ptep : large page table entry to split
		@level : level of ptep (2 or 3)
 */
static void split_large_ept_entry(unsigned long *ptep, int level)
{
	unsigned long *new_pt;
	unsigned long gpa;
	unsigned long pte;
	unsigned long prototype;
	int i;

	pte = *ptep;
	assert(pte & EPT_PRESENT);
	assert(pte & EPT_LARGE_PAGE);
	assert(level == 2 || level == 3);

	new_pt = alloc_page();
	assert(new_pt);
	memset(new_pt, 0, PAGE_SIZE);

	prototype = pte & ~EPT_ADDR_MASK;
	if (level == 2)
		prototype &= ~EPT_LARGE_PAGE;

	gpa = pte & EPT_ADDR_MASK;
	for (i = 0; i < EPT_PGDIR_ENTRIES; i++) {
		new_pt[i] = prototype | gpa;
		gpa += 1ul << EPT_LEVEL_SHIFT(level - 1);
	}

	pte &= ~EPT_LARGE_PAGE;
	pte &= ~EPT_ADDR_MASK;
	pte |= virt_to_phys(new_pt);

	*ptep = pte;
}

/* install_ept_entry : Install a page to a given level in EPT
		@pml4 : addr of pml4 table
		@pte_level : level of PTE to set
		@guest_addr : physical address of guest
		@pte : pte value to set
		@pt_page : address of page table, NULL for a new page
 */
static void install_ept_entry(unsigned long *pml4,
		int pte_level,
		unsigned long guest_addr,
		unsigned long pte,
		unsigned long *pt_page)
{
	int level;
	unsigned long *pt = pml4;
	unsigned offset;

	/* EPT only uses 48 bits of GPA. */
	assert(guest_addr < (1ul << 48));

	for (level = EPT_PAGE_LEVEL; level > pte_level; --level) {
		offset = (guest_addr >> EPT_LEVEL_SHIFT(level))
				& EPT_PGDIR_MASK;
		if (!(pt[offset] & (EPT_PRESENT))) {
			unsigned long *new_pt = pt_page;
			if (!new_pt)
				new_pt = alloc_page();
			else
				pt_page = 0;
			memset(new_pt, 0, PAGE_SIZE);
			pt[offset] = virt_to_phys(new_pt)
					| EPT_RA | EPT_WA | EPT_EA;
		} else if (pt[offset] & EPT_LARGE_PAGE)
			split_large_ept_entry(&pt[offset], level);
		pt = phys_to_virt(pt[offset] & EPT_ADDR_MASK);
	}
	offset = (guest_addr >> EPT_LEVEL_SHIFT(level)) & EPT_PGDIR_MASK;
	pt[offset] = pte;
}

/* Map a page, @perm is the permission of the page */
static void install_ept(unsigned long *pml4,
		unsigned long phys,
		unsigned long guest_addr,
		u64 perm)
{
	install_ept_entry(pml4, 1, guest_addr, (phys & PAGE_MASK) | perm, 0);
}

/* Map a 1G-size page */
static void install_1g_ept(unsigned long *pml4,
		unsigned long phys,
		unsigned long guest_addr,
		u64 perm)
{
	install_ept_entry(pml4, 3, guest_addr,
			(phys & PAGE_MASK) | perm | EPT_LARGE_PAGE, 0);
}

/* Map a 2M-size page */
static void install_2m_ept(unsigned long *pml4,
		unsigned long phys,
		unsigned long guest_addr,
		u64 perm)
{
	install_ept_entry(pml4, 2, guest_addr,
			(phys & PAGE_MASK) | perm | EPT_LARGE_PAGE, 0);
}

/* setup_ept_range : Setup a range of 1:1 mapped page to EPT paging structure.
		@start : start address of guest page
		@len : length of address to be mapped
		@map_1g : whether 1G page map is used
		@map_2m : whether 2M page map is used
		@perm : permission for every page
 */
static void setup_ept_range(unsigned long *pml4, unsigned long start,
		     unsigned long len, int map_1g, int map_2m, u64 perm)
{
	u64 phys = start;
	u64 max = (u64)len + (u64)start;

	if (map_1g) {
		while (phys + PAGE_SIZE_1G <= max) {
			install_1g_ept(pml4, phys, phys, perm);
			phys += PAGE_SIZE_1G;
		}
	}
	if (map_2m) {
		while (phys + PAGE_SIZE_2M <= max) {
			install_2m_ept(pml4, phys, phys, perm);
			phys += PAGE_SIZE_2M;
		}
	}
	while (phys + PAGE_SIZE <= max) {
		install_ept(pml4, phys, phys, perm);
		phys += PAGE_SIZE;
	}
}

static bool ept_2m_supported(void)
{
	return ept_vpid.val & EPT_CAP_2M_PAGE;
}

/**
 * __setup_ept - Setup the VMCS fields to enable Extended Page Tables (EPT)
 * @hpa:	Host physical address of the top-level, a.k.a. root, EPT table
 * @enable_ad:	Whether or not to enable Access/Dirty bits for EPT entries
 *
 * Returns 0 on success, 1 on failure.
 *
 * Note that @hpa doesn't need to point at actual memory if VM-Launch is
 * expected to fail, e.g. setup_dummy_ept() arbitrarily passes '0' to satisfy
 * the various EPTP consistency checks, but doesn't ensure backing for HPA '0'.
 */
static int __setup_ept(u64 hpa, bool enable_ad)
{
	if (!(ctrl_cpu_rev[0].clr & CPU_SECONDARY) ||
	    !(ctrl_cpu_rev[1].clr & CPU_EPT)) {
		printf("\tEPT is not supported");
		return 1;
	}
	if (!(ept_vpid.val & EPT_CAP_WB)) {
		printf("WB memtype for EPT walks not supported\n");
		return 1;
	}
	if (!(ept_vpid.val & EPT_CAP_PWL4)) {
		printf("\tPWL4 is not supported\n");
		return 1;
	}

	eptp = EPT_MEM_TYPE_WB;
	eptp |= (3 << EPTP_PG_WALK_LEN_SHIFT);
	eptp |= hpa;
	if (enable_ad)
		eptp |= EPTP_AD_FLAG;

	vmcs_write(EPTP, eptp);
	vmcs_write(CPU_EXEC_CTRL0, vmcs_read(CPU_EXEC_CTRL0)| CPU_SECONDARY);
	vmcs_write(CPU_EXEC_CTRL1, vmcs_read(CPU_EXEC_CTRL1)| CPU_EPT);

	return 0;
}

/**
 * setup_ept - Enable Extended Page Tables (EPT) and setup an identity map
 * @enable_ad:	Whether or not to enable Access/Dirty bits for EPT entries
 *
 * Returns 0 on success, 1 on failure.
 *
 * This is the "real" function for setting up EPT tables, i.e. use this for
 * tests that need to run code in the guest with EPT enabled.
 */
int setup_ept(bool enable_ad)
{
	unsigned long end_of_memory;

	pml4 = alloc_page();

	if (__setup_ept(virt_to_phys(pml4), enable_ad))
		return 1;

	memset(pml4, 0, PAGE_SIZE);

	end_of_memory = fwcfg_get_u64(FW_CFG_RAM_SIZE);
	if (end_of_memory < (1ul << 32))
		end_of_memory = (1ul << 32);
	/* Cannot use large EPT pages if we need to track EPT
	 * accessed/dirty bits at 4K granularity.
	 */
	setup_ept_range(pml4, 0, end_of_memory, 0,
			!enable_ad && ept_2m_supported(),
			EPT_WA | EPT_RA | EPT_EA);
	return 0;
}

