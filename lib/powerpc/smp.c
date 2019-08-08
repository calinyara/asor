/*
 * Secondary cpu support
 *
 * Copyright 2016 Suraj Jitindar Singh, IBM.
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */

#include <devicetree.h>
#include <asm/setup.h>
#include <asm/rtas.h>
#include <asm/smp.h>

int nr_threads;

struct secondary_entry_data {
	secondary_entry_fn entry;
	uint64_t r3;
	int nr_started;
};

/*
 * Start stopped thread cpu_id at entry
 * Returns:	<0 on failure to start stopped cpu
 *		0  on success
 *		>0 on cpu not in stopped state
 */
int start_thread(int cpu_id, secondary_entry_fn entry, uint32_t r3)
{
	uint32_t query_token, start_token;
	int outputs[1], ret;

	ret = rtas_token("query-cpu-stopped-state", &query_token);
	assert(ret == 0);
	ret = rtas_token("start-cpu", &start_token);
	assert(ret == 0);

	ret = rtas_call(query_token, 1, 2, outputs, cpu_id);
	if (ret) {
		printf("query-cpu-stopped-state failed for cpu %d\n", cpu_id);
	} else if (!outputs[0]) { /* cpu in stopped state */
		ret = rtas_call(start_token, 3, 1, NULL, cpu_id, entry, r3);
		if (ret)
			printf("failed to start cpu %d\n", cpu_id);
	} else { /* cpu not in stopped state */
		ret = outputs[0];
	}

	return ret;
}

/*
 * Start all stopped threads (vcpus) on cpu_node
 * Returns: Number of stopped cpus which were successfully started
 */
struct start_threads start_cpu(int cpu_node, secondary_entry_fn entry,
			       uint32_t r3)
{
	int len, i, nr_threads, nr_started = 0;
	const struct fdt_property *prop;
	u32 *threads;

	/* Get the id array of threads on this cpu_node */
	prop = fdt_get_property(dt_fdt(), cpu_node,
				"ibm,ppc-interrupt-server#s", &len);
	assert(prop);

	nr_threads = len >> 2; /* Divide by 4 since 4 bytes per thread */
	threads = (u32 *)prop->data; /* Array of valid ids */

	for (i = 0; i < nr_threads; i++) {
		if (!start_thread(fdt32_to_cpu(threads[i]), entry, r3))
			nr_started++;
	}

	return (struct start_threads) { nr_threads, nr_started };
}

static void start_each_secondary(int fdtnode, u64 regval __unused, void *info)
{
	struct secondary_entry_data *datap = info;
	struct start_threads ret = start_cpu(fdtnode, datap->entry, datap->r3);

	nr_threads += ret.nr_threads;
	datap->nr_started += ret.nr_started;
}

/*
 * Start all stopped cpus on the guest at entry with register 3 set to r3
 * We expect that we come in with only one thread currently started
 * Returns:	TRUE on success
 *		FALSE on failure
 */
bool start_all_cpus(secondary_entry_fn entry, uint32_t r3)
{
	struct secondary_entry_data data = { entry, r3,	0 };
	int ret;

	ret = dt_for_each_cpu_node(start_each_secondary, &data);
	assert(ret == 0);

	/* We expect that we come in with one thread already started */
	return data.nr_started == nr_threads - 1;
}
