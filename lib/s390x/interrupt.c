/*
 * s390x interrupt handling
 *
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#include <libcflat.h>
#include <asm/interrupt.h>
#include <asm/barrier.h>

static bool pgm_int_expected;
static struct lowcore *lc;

void expect_pgm_int(void)
{
	pgm_int_expected = true;
	lc->pgm_int_code = 0;
	mb();
}

uint16_t clear_pgm_int(void)
{
	uint16_t code;

	mb();
	code = lc->pgm_int_code;
	lc->pgm_int_code = 0;
	pgm_int_expected = false;
	return code;
}

void check_pgm_int_code(uint16_t code)
{
	mb();
	report("Program interrupt: expected(%d) == received(%d)",
	       code == lc->pgm_int_code, code, lc->pgm_int_code);
}

static void fixup_pgm_int(void)
{
	switch (lc->pgm_int_code) {
	case PGM_INT_CODE_PRIVILEGED_OPERATION:
		/* Normal operation is in supervisor state, so this exception
		 * was produced intentionally and we should return to the
		 * supervisor state.
		 */
		lc->pgm_old_psw.mask &= ~PSW_MASK_PSTATE;
		break;
	case PGM_INT_CODE_PROTECTION:
		/* Handling for iep.c test case. */
		if (lc->trans_exc_id & 0x80UL && lc->trans_exc_id & 0x04UL &&
		    !(lc->trans_exc_id & 0x08UL))
			lc->pgm_old_psw.addr = lc->sw_int_grs[14];
		break;
	case PGM_INT_CODE_SEGMENT_TRANSLATION:
	case PGM_INT_CODE_PAGE_TRANSLATION:
	case PGM_INT_CODE_TRACE_TABLE:
	case PGM_INT_CODE_AFX_TRANSLATION:
	case PGM_INT_CODE_ASX_TRANSLATION:
	case PGM_INT_CODE_LX_TRANSLATION:
	case PGM_INT_CODE_EX_TRANSLATION:
	case PGM_INT_CODE_PRIMARY_AUTHORITY:
	case PGM_INT_CODE_SECONDARY_AUTHORITY:
	case PGM_INT_CODE_LFX_TRANSLATION:
	case PGM_INT_CODE_LSX_TRANSLATION:
	case PGM_INT_CODE_ALEN_TRANSLATION:
	case PGM_INT_CODE_ALE_SEQUENCE:
	case PGM_INT_CODE_ASTE_VALIDITY:
	case PGM_INT_CODE_ASTE_SEQUENCE:
	case PGM_INT_CODE_EXTENDED_AUTHORITY:
	case PGM_INT_CODE_LSTE_SEQUENCE:
	case PGM_INT_CODE_ASTE_INSTANCE:
	case PGM_INT_CODE_STACK_FULL:
	case PGM_INT_CODE_STACK_EMPTY:
	case PGM_INT_CODE_STACK_SPECIFICATION:
	case PGM_INT_CODE_STACK_TYPE:
	case PGM_INT_CODE_STACK_OPERATION:
	case PGM_INT_CODE_ASCE_TYPE:
	case PGM_INT_CODE_REGION_FIRST_TRANS:
	case PGM_INT_CODE_REGION_SECOND_TRANS:
	case PGM_INT_CODE_REGION_THIRD_TRANS:
	case PGM_INT_CODE_PER:
	case PGM_INT_CODE_CRYPTO_OPERATION:
		/* The interrupt was nullified, the old PSW points at the
		 * responsible instruction. Forward the PSW so we don't loop.
		 */
		lc->pgm_old_psw.addr += lc->pgm_int_id;
	}
	/* suppressed/terminated/completed point already at the next address */
}

void handle_pgm_int(void)
{
	if (!pgm_int_expected)
		report_abort("Unexpected program interrupt: %d at %#lx, ilen %d\n",
			     lc->pgm_int_code, lc->pgm_old_psw.addr,
			     lc->pgm_int_id);

	pgm_int_expected = false;
	fixup_pgm_int();
}

void handle_ext_int(void)
{
	report_abort("Unexpected external call interrupt: at %#lx",
		     lc->ext_old_psw.addr);
}

void handle_mcck_int(void)
{
	report_abort("Unexpected machine check interrupt: at %#lx",
		     lc->mcck_old_psw.addr);
}

void handle_io_int(void)
{
	report_abort("Unexpected io interrupt: at %#lx",
		     lc->io_old_psw.addr);
}

void handle_svc_int(void)
{
	report_abort("Unexpected supervisor call interrupt: at %#lx",
		     lc->svc_old_psw.addr);
}
