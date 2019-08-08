/*
 * PSCI API
 * From arch/arm[64]/kernel/psci.c
 *
 * Copyright (C) 2015, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <asm/psci.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/smp.h>

__attribute__((noinline))
int psci_invoke(unsigned long function_id, unsigned long arg0,
		unsigned long arg1, unsigned long arg2)
{
	asm volatile(
		"hvc #0"
	: "+r" (function_id)
	: "r" (arg0), "r" (arg1), "r" (arg2));
	return function_id;
}

int psci_cpu_on(unsigned long cpuid, unsigned long entry_point)
{
#ifdef __arm__
	return psci_invoke(PSCI_0_2_FN_CPU_ON, cpuid, entry_point, 0);
#else
	return psci_invoke(PSCI_0_2_FN64_CPU_ON, cpuid, entry_point, 0);
#endif
}

extern void secondary_entry(void);
int cpu_psci_cpu_boot(unsigned int cpu)
{
	int err = psci_cpu_on(cpus[cpu], __pa(secondary_entry));
	if (err)
		printf("failed to boot CPU%d (%d)\n", cpu, err);
	return err;
}

#define PSCI_POWER_STATE_TYPE_POWER_DOWN (1U << 16)
void cpu_psci_cpu_die(void)
{
	int err = psci_invoke(PSCI_0_2_FN_CPU_OFF,
			PSCI_POWER_STATE_TYPE_POWER_DOWN, 0, 0);
	printf("CPU%d unable to power off (error = %d)\n", smp_processor_id(), err);
}

void psci_system_reset(void)
{
	psci_invoke(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
}

void psci_system_off(void)
{
	int err = psci_invoke(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
	printf("CPU%d unable to do system off (error = %d)\n", smp_processor_id(), err);
}
