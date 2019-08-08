/*
 * SIGP related definitions.
 *
 * Copied from the Linux kernel file arch/s390/include/asm/sigp.h
 *
 * This work is licensed under the terms of the GNU GPL, version
 * 2.
 */

#ifndef ASM_S390X_SIGP_H
#define ASM_S390X_SIGP_H

/* SIGP order codes */
#define SIGP_SENSE			1
#define SIGP_EXTERNAL_CALL		2
#define SIGP_EMERGENCY_SIGNAL		3
#define SIGP_START			4
#define SIGP_STOP			5
#define SIGP_RESTART			6
#define SIGP_STOP_AND_STORE_STATUS	9
#define SIGP_INITIAL_CPU_RESET		11
#define SIGP_CPU_RESET			12
#define SIGP_SET_PREFIX			13
#define SIGP_STORE_STATUS_AT_ADDRESS	14
#define SIGP_SET_ARCHITECTURE		18
#define SIGP_COND_EMERGENCY_SIGNAL	19
#define SIGP_SENSE_RUNNING		21
#define SIGP_SET_MULTI_THREADING	22
#define SIGP_STORE_ADDITIONAL_STATUS	23

/* SIGP condition codes */
#define SIGP_CC_ORDER_CODE_ACCEPTED 0
#define SIGP_CC_STATUS_STORED	    1
#define SIGP_CC_BUSY		    2
#define SIGP_CC_NOT_OPERATIONAL	    3

/* SIGP cpu status bits */

#define SIGP_STATUS_INVALID_ORDER	0x00000002UL
#define SIGP_STATUS_CHECK_STOP		0x00000010UL
#define SIGP_STATUS_STOPPED		0x00000040UL
#define SIGP_STATUS_EXT_CALL_PENDING	0x00000080UL
#define SIGP_STATUS_INVALID_PARAMETER	0x00000100UL
#define SIGP_STATUS_INCORRECT_STATE	0x00000200UL
#define SIGP_STATUS_NOT_RUNNING		0x00000400UL

#ifndef __ASSEMBLER__

static inline void sigp_stop(void)
{
	register unsigned long status asm ("1") = 0;
	register unsigned long cpu asm ("2") = 0;

	asm volatile(
		"	sigp %0,%1,0(%2)\n"
		: "+d" (status)  : "d" (cpu), "d" (SIGP_STOP) : "cc");
}

#endif /* __ASSEMBLER__ */
#endif /* ASM_S390X_SIGP_H */
