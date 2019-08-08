/*
 * Copyright (c) 2017 Red Hat Inc
 *
 * Authors:
 *  David Hildenbrand <david@redhat.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License version 2.
 */
#include <libcflat.h>
#include <kbuild.h>
#include <asm/arch_def.h>

int main(void)
{
	OFFSET(GEN_LC_EXT_INT_PARAM, lowcore, ext_int_param);
	OFFSET(GEN_LC_CPU_ADDR, lowcore, cpu_addr);
	OFFSET(GEN_LC_EXT_INT_CODE, lowcore, ext_int_code);
	OFFSET(GEN_LC_SVC_INT_ID, lowcore, svc_int_id);
	OFFSET(GEN_LC_SVC_INT_CODE, lowcore, svc_int_code);
	OFFSET(GEN_LC_PGM_INT_ID, lowcore, pgm_int_id);
	OFFSET(GEN_LC_PGM_INT_CODE, lowcore, pgm_int_code);
	OFFSET(GEN_LC_DXC_VXC, lowcore, dxc_vxc);
	OFFSET(GEN_LC_MON_CLASS_NB, lowcore, mon_class_nb);
	OFFSET(GEN_LC_PER_CODE, lowcore, per_code);
	OFFSET(GEN_LC_PER_ATMID, lowcore, per_atmid);
	OFFSET(GEN_LC_PER_ADDR, lowcore, per_addr);
	OFFSET(GEN_LC_EXC_ACC_ID, lowcore, exc_acc_id);
	OFFSET(GEN_LC_PER_ACC_ID, lowcore, per_acc_id);
	OFFSET(GEN_LC_OP_ACC_ID, lowcore, op_acc_id);
	OFFSET(GEN_LC_ARCH_MODE_ID, lowcore, arch_mode_id);
	OFFSET(GEN_LC_TRANS_EXC_ID, lowcore, trans_exc_id);
	OFFSET(GEN_LC_MON_CODE, lowcore, mon_code);
	OFFSET(GEN_LC_SUBSYS_ID_WORD, lowcore, subsys_id_word);
	OFFSET(GEN_LC_IO_INT_PARAM, lowcore, io_int_param);
	OFFSET(GEN_LC_IO_INT_WORD, lowcore, io_int_word);
	OFFSET(GEN_LC_STFL, lowcore, stfl);
	OFFSET(GEN_LC_MCCK_INT_CODE, lowcore, mcck_int_code);
	OFFSET(GEN_LC_EXT_DAMAGE_CODE, lowcore, ext_damage_code);
	OFFSET(GEN_LC_FAILING_STORAGE_ADDR, lowcore, failing_storage_addr);
	OFFSET(GEN_LC_EMON_CA_ORIGIN, lowcore, emon_ca_origin);
	OFFSET(GEN_LC_EMON_CA_SIZE, lowcore, emon_ca_size);
	OFFSET(GEN_LC_EMON_EXC_COUNT, lowcore, emon_exc_count);
	OFFSET(GEN_LC_BREAKING_EVENT_ADDR, lowcore, breaking_event_addr);
	OFFSET(GEN_LC_RESTART_OLD_PSW, lowcore, restart_old_psw);
	OFFSET(GEN_LC_EXT_OLD_PSW, lowcore, ext_old_psw);
	OFFSET(GEN_LC_SVC_OLD_PSW, lowcore, svc_old_psw);
	OFFSET(GEN_LC_PGM_OLD_PSW, lowcore, pgm_old_psw);
	OFFSET(GEN_LC_MCCK_OLD_PSW, lowcore, mcck_old_psw);
	OFFSET(GEN_LC_IO_OLD_PSW, lowcore, io_old_psw);
	OFFSET(GEN_LC_RESTART_NEW_PSW, lowcore, restart_new_psw);
	OFFSET(GEN_LC_EXT_NEW_PSW, lowcore, ext_new_psw);
	OFFSET(GEN_LC_SVC_NEW_PSW, lowcore, svc_new_psw);
	OFFSET(GEN_LC_PGM_NEW_PSW, lowcore, pgm_new_psw);
	OFFSET(GEN_LC_MCCK_NEW_PSW, lowcore, mcck_new_psw);
	OFFSET(GEN_LC_IO_NEW_PSW, lowcore, io_new_psw);
	OFFSET(GEN_LC_SW_INT_GRS, lowcore, sw_int_grs);
	OFFSET(GEN_LC_SW_INT_FPRS, lowcore, sw_int_fprs);
	OFFSET(GEN_LC_SW_INT_FPC, lowcore, sw_int_fpc);
	OFFSET(GEN_LC_SW_INT_CR0, lowcore, sw_int_cr0);
	OFFSET(GEN_LC_MCCK_EXT_SA_ADDR, lowcore, mcck_ext_sa_addr);
	OFFSET(GEN_LC_FPRS_SA, lowcore, fprs_sa);
	OFFSET(GEN_LC_GRS_SA, lowcore, grs_sa);
	OFFSET(GEN_LC_PSW_SA, lowcore, psw_sa);
	OFFSET(GEN_LC_PREFIX_SA, lowcore, prefix_sa);
	OFFSET(GEN_LC_FPC_SA, lowcore, fpc_sa);
	OFFSET(GEN_LC_TOD_PR_SA, lowcore, tod_pr_sa);
	OFFSET(GEN_LC_CPUTM_SA, lowcore, cputm_sa);
	OFFSET(GEN_LC_CC_SA, lowcore, cc_sa);
	OFFSET(GEN_LC_ARS_SA, lowcore, ars_sa);
	OFFSET(GEN_LC_CRS_SA, lowcore, crs_sa);
	OFFSET(GEN_LC_PGM_INT_TDB, lowcore, pgm_int_tdb);

	return 0;
}
