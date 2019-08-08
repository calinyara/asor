#ifndef _ASMPOWERPC_HANDLERS_H_
#define _ASMPOWERPC_HANDLERS_H_

#include <asm/ptrace.h>

void dec_except_handler(struct pt_regs *regs, void *data);

#endif /* _ASMPOWERPC_HANDLERS_H_ */
