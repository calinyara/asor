#ifndef __FAULT_TEST__
#define __FAULT_TEST__

#include "x86/msr.h"
#include "x86/processor.h"
#include "x86/apic-defs.h"
#include "x86/apic.h"
#include "x86/desc.h"
#include "x86/isr.h"
#include "alloc.h"
#include "setjmp.h"
#include "usermode.h"

#include "libcflat.h"
#include <stdint.h>

#define FAULT_TEST(nm, a) { .name = nm, .arg = a}

struct fault_test_arg;

typedef uint64_t (*test_fault_func)(uint64_t arg1, uint64_t arg2,
		uint64_t arg3, uint64_t arg4);
typedef bool (*test_fault_callback)(struct fault_test_arg *arg);

struct fault_test_arg {
	bool usermode;
	unsigned int fault_vector;
	bool should_fault;
	uint64_t arg[4];
	uint64_t retval;
	test_fault_func func;
	test_fault_callback callback;
};

struct fault_test {
	const char *name;
	struct fault_test_arg arg;
};

void test_run(struct fault_test *test);

#endif
