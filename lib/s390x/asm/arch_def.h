/*
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#ifndef _ASM_S390X_ARCH_DEF_H_
#define _ASM_S390X_ARCH_DEF_H_

struct psw {
	uint64_t	mask;
	uint64_t	addr;
};

#define PSW_MASK_DAT			0x0400000000000000UL
#define PSW_MASK_PSTATE			0x0001000000000000UL

struct lowcore {
	uint8_t		pad_0x0000[0x0080 - 0x0000];	/* 0x0000 */
	uint32_t	ext_int_param;			/* 0x0080 */
	uint16_t	cpu_addr;			/* 0x0084 */
	uint16_t	ext_int_code;			/* 0x0086 */
	uint16_t	svc_int_id;			/* 0x0088 */
	uint16_t	svc_int_code;			/* 0x008a */
	uint16_t	pgm_int_id;			/* 0x008c */
	uint16_t	pgm_int_code;			/* 0x008e */
	uint32_t	dxc_vxc;			/* 0x0090 */
	uint16_t	mon_class_nb;			/* 0x0094 */
	uint8_t		per_code;			/* 0x0096 */
	uint8_t		per_atmid;			/* 0x0097 */
	uint64_t	per_addr;			/* 0x0098 */
	uint8_t		exc_acc_id;			/* 0x00a0 */
	uint8_t		per_acc_id;			/* 0x00a1 */
	uint8_t		op_acc_id;			/* 0x00a2 */
	uint8_t		arch_mode_id;			/* 0x00a3 */
	uint8_t		pad_0x00a4[0x00a8 - 0x00a4];	/* 0x00a4 */
	uint64_t	trans_exc_id;			/* 0x00a8 */
	uint64_t	mon_code;			/* 0x00b0 */
	uint32_t	subsys_id_word;			/* 0x00b8 */
	uint32_t	io_int_param;			/* 0x00bc */
	uint32_t	io_int_word;			/* 0x00c0 */
	uint8_t		pad_0x00c4[0x00c8 - 0x00c4];	/* 0x00c4 */
	uint32_t	stfl;				/* 0x00c8 */
	uint8_t		pad_0x00cc[0x00e8 - 0x00cc];	/* 0x00cc */
	uint64_t	mcck_int_code;			/* 0x00e8 */
	uint8_t		pad_0x00f0[0x00f4 - 0x00f0];	/* 0x00f0 */
	uint32_t	ext_damage_code;		/* 0x00f4 */
	uint64_t	failing_storage_addr;		/* 0x00f8 */
	uint64_t	emon_ca_origin;			/* 0x0100 */
	uint32_t	emon_ca_size;			/* 0x0108 */
	uint32_t	emon_exc_count;			/* 0x010c */
	uint64_t	breaking_event_addr;		/* 0x0110 */
	uint8_t		pad_0x0118[0x0120 - 0x0118];	/* 0x0118 */
	struct psw	restart_old_psw;		/* 0x0120 */
	struct psw	ext_old_psw;			/* 0x0130 */
	struct psw	svc_old_psw;			/* 0x0140 */
	struct psw	pgm_old_psw;			/* 0x0150 */
	struct psw	mcck_old_psw;			/* 0x0160 */
	struct psw	io_old_psw;			/* 0x0170 */
	uint8_t		pad_0x0180[0x01a0 - 0x0180];	/* 0x0180 */
	struct psw	restart_new_psw;		/* 0x01a0 */
	struct psw	ext_new_psw;			/* 0x01b0 */
	struct psw	svc_new_psw;			/* 0x01c0 */
	struct psw	pgm_new_psw;			/* 0x01d0 */
	struct psw	mcck_new_psw;			/* 0x01e0 */
	struct psw	io_new_psw;			/* 0x01f0 */
	/* sw definition: save area for registers in interrupt handlers */
	uint64_t	sw_int_grs[16];			/* 0x0200 */
	uint64_t	sw_int_fprs[16];		/* 0x0280 */
	uint32_t	sw_int_fpc;			/* 0x0300 */
	uint8_t		pad_0x0304[0x0308 - 0x0304];	/* 0x0304 */
	uint64_t	sw_int_cr0;			/* 0x0308 */
	uint8_t		pad_0x0310[0x11b0 - 0x0310];	/* 0x0310 */
	uint64_t	mcck_ext_sa_addr;		/* 0x11b0 */
	uint8_t		pad_0x11b8[0x1200 - 0x11b8];	/* 0x11b8 */
	uint64_t	fprs_sa[16];			/* 0x1200 */
	uint64_t	grs_sa[16];			/* 0x1280 */
	struct psw	psw_sa;				/* 0x1300 */
	uint8_t		pad_0x1310[0x1318 - 0x1310];	/* 0x1310 */
	uint32_t	prefix_sa;			/* 0x1318 */
	uint32_t	fpc_sa;				/* 0x131c */
	uint8_t		pad_0x1320[0x1324 - 0x1320];	/* 0x1320 */
	uint32_t	tod_pr_sa;			/* 0x1324 */
	uint64_t	cputm_sa;			/* 0x1328 */
	uint64_t	cc_sa;				/* 0x1330 */
	uint8_t		pad_0x1338[0x1340 - 0x1338];	/* 0x1338 */
	uint32_t	ars_sa[16];			/* 0x1340 */
	uint64_t	crs_sa[16];			/* 0x1380 */
	uint8_t		pad_0x1400[0x1800 - 0x1400];	/* 0x1400 */
	uint8_t		pgm_int_tdb[0x1900 - 0x1800];	/* 0x1800 */
} __attribute__ ((__packed__));

#define PGM_INT_CODE_OPERATION			0x01
#define PGM_INT_CODE_PRIVILEGED_OPERATION	0x02
#define PGM_INT_CODE_EXECUTE			0x03
#define PGM_INT_CODE_PROTECTION			0x04
#define PGM_INT_CODE_ADDRESSING			0x05
#define PGM_INT_CODE_SPECIFICATION		0x06
#define PGM_INT_CODE_DATA			0x07
#define PGM_INT_CODE_FIXED_POINT_OVERFLOW	0x08
#define PGM_INT_CODE_FIXED_POINT_DIVIDE		0x09
#define PGM_INT_CODE_DECIMAL_OVERFLOW		0x0a
#define PGM_INT_CODE_DECIMAL_DIVIDE		0x0b
#define PGM_INT_CODE_HFP_EXPONENT_OVERFLOW	0x0c
#define PGM_INT_CODE_HFP_EXPONENT_UNDERFLOW	0x0d
#define PGM_INT_CODE_HFP_SIGNIFICANCE		0x0e
#define PGM_INT_CODE_HFP_DIVIDE			0x0f
#define PGM_INT_CODE_SEGMENT_TRANSLATION	0x10
#define PGM_INT_CODE_PAGE_TRANSLATION		0x11
#define PGM_INT_CODE_TRANSLATION_SPEC		0x12
#define PGM_INT_CODE_SPECIAL_OPERATION		0x13
#define PGM_INT_CODE_OPERAND			0x15
#define PGM_INT_CODE_TRACE_TABLE		0x16
#define PGM_INT_CODE_VECTOR_PROCESSING		0x1b
#define PGM_INT_CODE_SPACE_SWITCH_EVENT		0x1c
#define PGM_INT_CODE_HFP_SQUARE_ROOT		0x1d
#define PGM_INT_CODE_PC_TRANSLATION_SPEC	0x1f
#define PGM_INT_CODE_AFX_TRANSLATION		0x20
#define PGM_INT_CODE_ASX_TRANSLATION		0x21
#define PGM_INT_CODE_LX_TRANSLATION		0x22
#define PGM_INT_CODE_EX_TRANSLATION		0x23
#define PGM_INT_CODE_PRIMARY_AUTHORITY		0x24
#define PGM_INT_CODE_SECONDARY_AUTHORITY	0x25
#define PGM_INT_CODE_LFX_TRANSLATION		0x26
#define PGM_INT_CODE_LSX_TRANSLATION		0x27
#define PGM_INT_CODE_ALET_SPECIFICATION		0x28
#define PGM_INT_CODE_ALEN_TRANSLATION		0x29
#define PGM_INT_CODE_ALE_SEQUENCE		0x2a
#define PGM_INT_CODE_ASTE_VALIDITY		0x2b
#define PGM_INT_CODE_ASTE_SEQUENCE		0x2c
#define PGM_INT_CODE_EXTENDED_AUTHORITY		0x2d
#define PGM_INT_CODE_LSTE_SEQUENCE		0x2e
#define PGM_INT_CODE_ASTE_INSTANCE		0x2f
#define PGM_INT_CODE_STACK_FULL			0x30
#define PGM_INT_CODE_STACK_EMPTY		0x31
#define PGM_INT_CODE_STACK_SPECIFICATION	0x32
#define PGM_INT_CODE_STACK_TYPE			0x33
#define PGM_INT_CODE_STACK_OPERATION		0x34
#define PGM_INT_CODE_ASCE_TYPE			0x38
#define PGM_INT_CODE_REGION_FIRST_TRANS		0x39
#define PGM_INT_CODE_REGION_SECOND_TRANS	0x3a
#define PGM_INT_CODE_REGION_THIRD_TRANS		0x3b
#define PGM_INT_CODE_MONITOR_EVENT		0x40
#define PGM_INT_CODE_PER			0x80
#define PGM_INT_CODE_CRYPTO_OPERATION		0x119
#define PGM_INT_CODE_TX_ABORTED_EVENT		0x200

struct cpuid {
	uint64_t version : 8;
	uint64_t id : 24;
	uint64_t type : 16;
	uint64_t format : 1;
	uint64_t reserved : 15;
};

static inline int tprot(unsigned long addr)
{
	int cc;

	asm volatile(
		"	tprot	0(%1),0\n"
		"	ipm	%0\n"
		"	srl	%0,28\n"
		: "=d" (cc) : "a" (addr) : "cc");
	return cc;
}

static inline void lctlg(int cr, uint64_t value)
{
	asm volatile(
		"	lctlg	%1,%1,%0\n"
		: : "Q" (value), "i" (cr));
}

static inline uint64_t stctg(int cr)
{
	uint64_t value;

	asm volatile(
		"	stctg	%1,%1,%0\n"
		: "=Q" (value) : "i" (cr) : "memory");
	return value;
}

static inline void ctl_set_bit(int cr, unsigned int bit)
{
        uint64_t reg;

	reg = stctg(cr);
	reg |= 1UL << bit;
	lctlg(cr, reg);
}

static inline void ctl_clear_bit(int cr, unsigned int bit)
{
        uint64_t reg;

	reg = stctg(cr);
	reg &= ~(1UL << bit);
	lctlg(cr, reg);
}

static inline uint64_t extract_psw_mask(void)
{
	uint32_t mask_upper = 0, mask_lower = 0;

	asm volatile(
		"	epsw	%0,%1\n"
		: "+r" (mask_upper), "+r" (mask_lower) : : );

	return (uint64_t) mask_upper << 32 | mask_lower;
}

static inline void load_psw_mask(uint64_t mask)
{
	struct psw psw = {
		.mask = mask,
		.addr = 0,
	};
	uint64_t tmp = 0;

	asm volatile(
		"	larl	%0,0f\n"
		"	stg	%0,8(%1)\n"
		"	lpswe	0(%1)\n"
		"0:\n"
		: "+r" (tmp) :  "a" (&psw) : "memory", "cc" );
}

static inline void enter_pstate(void)
{
	uint64_t mask;

	mask = extract_psw_mask();
	mask |= PSW_MASK_PSTATE;
	load_psw_mask(mask);
}

#endif
