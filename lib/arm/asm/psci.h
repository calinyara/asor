#ifndef _ASMARM_PSCI_H_
#define _ASMARM_PSCI_H_
#include <libcflat.h>
#include <linux/psci.h>

extern int psci_invoke(unsigned long function_id, unsigned long arg0,
		       unsigned long arg1, unsigned long arg2);
extern int psci_cpu_on(unsigned long cpuid, unsigned long entry_point);
extern void psci_system_reset(void);
extern int cpu_psci_cpu_boot(unsigned int cpu);
extern void cpu_psci_cpu_die(void);
extern void psci_system_off(void);

#endif /* _ASMARM_PSCI_H_ */
