#include "fault_test.h"

jmp_buf jmpbuf;

static void restore_exec_to_jmpbuf(void)
{
	longjmp(jmpbuf, 1);
}

static void fault_test_fault(struct ex_regs *regs)
{
	regs->rip = (unsigned long)&restore_exec_to_jmpbuf;
}

static bool fault_test(struct fault_test_arg *arg)
{
	volatile uint64_t val;
	bool raised_vector = false;
	test_fault_func func = (test_fault_func) arg->func;
	/* Init as success in case there isn't callback */
	bool callback_success = true;

	if (arg->usermode) {
		val = run_in_user((usermode_func) func, arg->fault_vector,
				arg->arg[0], arg->arg[1], arg->arg[2],
				arg->arg[3], &raised_vector);
	} else {
		handle_exception(arg->fault_vector, fault_test_fault);
		if (setjmp(jmpbuf) == 0)
			val = func(arg->arg[0], arg->arg[1], arg->arg[2],
					arg->arg[3]);
		else
			raised_vector = true;
	}

	if (!raised_vector) {
		arg->retval = val;
		if (arg->callback != NULL)
			callback_success = arg->callback(arg);
	}

	return arg->should_fault ?
		raised_vector : (!raised_vector && callback_success);
}

void test_run(struct fault_test *test)
{
	bool passed = fault_test(&(test->arg));

	report("%s", passed, test->name);
}

