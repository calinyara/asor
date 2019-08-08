#ifndef _ASMPOWERPC_SMP_H_
#define _ASMPOWERPC_SMP_H_

#include <libcflat.h>

extern int nr_threads;

struct start_threads {
	int nr_threads;
	int nr_started;
};

typedef void (*secondary_entry_fn)(void);

extern void halt(void);

extern int start_thread(int cpu_id, secondary_entry_fn entry, uint32_t r3);
extern struct start_threads start_cpu(int cpu_node, secondary_entry_fn entry,
				      uint32_t r3);
extern bool start_all_cpus(secondary_entry_fn entry, uint32_t r3);

#endif /* _ASMPOWERPC_SMP_H_ */
