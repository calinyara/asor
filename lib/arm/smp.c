/*
 * Secondary cpu support
 *
 * Copyright (C) 2015, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include <libcflat.h>
#include <auxinfo.h>
#include <asm/thread_info.h>
#include <asm/spinlock.h>
#include <asm/cpumask.h>
#include <asm/barrier.h>
#include <asm/mmu.h>
#include <asm/psci.h>
#include <asm/smp.h>

bool cpu0_calls_idle;

cpumask_t cpu_present_mask;
cpumask_t cpu_online_mask;
cpumask_t cpu_idle_mask;

struct secondary_data {
	void *stack;            /* must be first member of struct */
	secondary_entry_fn entry;
};
struct secondary_data secondary_data;
static struct spinlock lock;

/* Needed to compile with -Wmissing-prototypes */
secondary_entry_fn secondary_cinit(void);

secondary_entry_fn secondary_cinit(void)
{
	struct thread_info *ti = current_thread_info();
	secondary_entry_fn entry;

	thread_info_init(ti, 0);

	if (!(auxinfo.flags & AUXINFO_MMU_OFF)) {
		ti->pgtable = mmu_idmap;
		mmu_mark_enabled(ti->cpu);
	}

	/*
	 * Save secondary_data.entry locally to avoid opening a race
	 * window between marking ourselves online and calling it.
	 */
	entry = secondary_data.entry;
	set_cpu_online(ti->cpu, true);
	sev();

	/*
	 * Return to the assembly stub, allowing entry to be called
	 * from there with an empty stack.
	 */
	return entry;
}

static void __smp_boot_secondary(int cpu, secondary_entry_fn entry)
{
	int ret;

	secondary_data.stack = thread_stack_alloc();
	secondary_data.entry = entry;
	mmu_mark_disabled(cpu);
	ret = cpu_psci_cpu_boot(cpu);
	assert(ret == 0);

	while (!cpu_online(cpu))
		wfe();
}

void smp_boot_secondary(int cpu, secondary_entry_fn entry)
{
	spin_lock(&lock);
	assert_msg(!cpu_online(cpu), "CPU%d already boot once", cpu);
	__smp_boot_secondary(cpu, entry);
	spin_unlock(&lock);
}

struct on_cpu_info {
	void (*func)(void *data);
	void *data;
	cpumask_t waiters;
};
static struct on_cpu_info on_cpu_info[NR_CPUS];

static void __deadlock_check(int cpu, const cpumask_t *waiters, bool *found)
{
	int i;

	for_each_cpu(i, waiters) {
		if (i == cpu) {
			printf("CPU%d", cpu);
			*found = true;
			return;
		}
		__deadlock_check(cpu, &on_cpu_info[i].waiters, found);
		if (*found) {
			printf(" <=> CPU%d", i);
			return;
		}
	}
}

static void deadlock_check(int me, int cpu)
{
	bool found = false;

	__deadlock_check(cpu, &on_cpu_info[me].waiters, &found);
	if (found) {
		printf(" <=> CPU%d deadlock detectd\n", me);
		assert(0);
	}
}

static void cpu_wait(int cpu)
{
	int me = smp_processor_id();

	if (cpu == me)
		return;

	cpumask_set_cpu(me, &on_cpu_info[cpu].waiters);
	deadlock_check(me, cpu);
	while (!cpu_idle(cpu))
		wfe();
	cpumask_clear_cpu(me, &on_cpu_info[cpu].waiters);
}

void do_idle(void)
{
	int cpu = smp_processor_id();

	if (cpu == 0)
		cpu0_calls_idle = true;

	set_cpu_idle(cpu, true);
	sev();

	for (;;) {
		while (cpu_idle(cpu))
			wfe();
		smp_rmb();
		on_cpu_info[cpu].func(on_cpu_info[cpu].data);
		on_cpu_info[cpu].func = NULL;
		smp_wmb();
		set_cpu_idle(cpu, true);
		sev();
	}
}

void on_cpu_async(int cpu, void (*func)(void *data), void *data)
{
	if (cpu == smp_processor_id()) {
		func(data);
		return;
	}

	assert_msg(cpu != 0 || cpu0_calls_idle, "Waiting on CPU0, which is unlikely to idle. "
						"If this is intended set cpu0_calls_idle=1");

	spin_lock(&lock);
	if (!cpu_online(cpu))
		__smp_boot_secondary(cpu, do_idle);
	spin_unlock(&lock);

	for (;;) {
		cpu_wait(cpu);
		spin_lock(&lock);
		if ((volatile void *)on_cpu_info[cpu].func == NULL)
			break;
		spin_unlock(&lock);
	}
	on_cpu_info[cpu].func = func;
	on_cpu_info[cpu].data = data;
	spin_unlock(&lock);
	set_cpu_idle(cpu, false);
	sev();
}

void on_cpu(int cpu, void (*func)(void *data), void *data)
{
	on_cpu_async(cpu, func, data);
	cpu_wait(cpu);
}

void on_cpus(void (*func)(void *data), void *data)
{
	int cpu, me = smp_processor_id();

	for_each_present_cpu(cpu) {
		if (cpu == me)
			continue;
		on_cpu_async(cpu, func, data);
	}
	func(data);

	for_each_present_cpu(cpu) {
		if (cpu == me)
			continue;
		cpumask_set_cpu(me, &on_cpu_info[cpu].waiters);
		deadlock_check(me, cpu);
	}
	while (cpumask_weight(&cpu_idle_mask) < nr_cpus - 1)
		wfe();
	for_each_present_cpu(cpu)
		cpumask_clear_cpu(me, &on_cpu_info[cpu].waiters);
}
