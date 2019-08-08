// SPDX-License-Identifier: GPL-2.0
/*
 * ASOR Hypervisor.
 *
 * Author: Jie Deng <mr.dengjie@gmail.com>
 */

#include "libcflat.h"
#include "processor.h"
#include "alloc_page.h"
#include "asm/page.h"
#include "desc.h"
#include "msr.h"
#include "ept.h"
#include "vmx.h"

u64 *vmxon_region;
u32 vpid_cnt;
void *guest_stack, *guest_syscall_stack;
u32 ctrl_pin, ctrl_enter, ctrl_exit, ctrl_cpu[2];
struct regs regs;
struct asor_guest *current;

union vmx_basic basic;
union vmx_ctrl_msr ctrl_pin_rev;
union vmx_ctrl_msr ctrl_cpu_rev[2];
union vmx_ctrl_msr ctrl_exit_rev;
union vmx_ctrl_msr ctrl_enter_rev;
union vmx_ept_vpid  ept_vpid;

extern struct descriptor_table_ptr gdt64_desc;
extern struct descriptor_table_ptr idt_descr;
extern struct descriptor_table_ptr tss_descr;
extern void *vmx_return;
extern void *entry_sysenter;
extern void *guest_entry;

u64 hypercall_field;
bool launched;
static int guest_finished;
static int in_guest;

static bool is_vmx_supported(void)
{
	u64 ia32_feature_control;

	if (!(cpuid(1).c & (1 << 5))) {
		error("Error: vmx not supported.\n");
		return false;
	}

	ia32_feature_control = rdmsr(MSR_IA32_FEATURE_CONTROL);
	if ((ia32_feature_control & 0x5) == 0x5) {
		printf("VMX enabled and locked by BIOS\n");
		return true;
	} else if (ia32_feature_control & 0x1) {
		printf("ERROR: VMX locked out by BIOS without enabled.\n");
		return false;
	}
	/* Enable VMX in MSR_IA32_FEATURE_CONTROL */
	wrmsr(MSR_IA32_FEATURE_CONTROL, 0x5);

	return true;
}

static void init_vmcs_ctrl(void)
{
	/* 26.2 CHECKS ON VMX CONTROLS AND HOST-STATE AREA */
	/* 26.2.1.1 */
	vmcs_write(PIN_CONTROLS, ctrl_pin);
	/* Disable VMEXIT of IO instruction */
	vmcs_write(CPU_EXEC_CTRL0, ctrl_cpu[0]);
	if (ctrl_cpu_rev[0].set & CPU_SECONDARY) {
		ctrl_cpu[1] = (ctrl_cpu[1] | ctrl_cpu_rev[1].set) &
			ctrl_cpu_rev[1].clr;
		vmcs_write(CPU_EXEC_CTRL1, ctrl_cpu[1]);
	}
	vmcs_write(CR3_TARGET_COUNT, 0);
	vmcs_write(VPID, ++vpid_cnt);
}

static void init_vmcs_host(void)
{
	/* 26.2 CHECKS ON VMX CONTROLS AND HOST-STATE AREA */
	/* 26.2.1.2 */
	vmcs_write(HOST_EFER, rdmsr(MSR_EFER));

	/* 26.2.1.3 */
	vmcs_write(ENT_CONTROLS, ctrl_enter);
	vmcs_write(EXI_CONTROLS, ctrl_exit);

	/* 26.2.2 */
	vmcs_write(HOST_CR0, read_cr0());
	vmcs_write(HOST_CR3, read_cr3());
	vmcs_write(HOST_CR4, read_cr4());
	vmcs_write(HOST_SYSENTER_EIP, (u64)(&entry_sysenter));
	vmcs_write(HOST_SYSENTER_CS,  KERNEL_CS);

	/* 26.2.3 */
	vmcs_write(HOST_SEL_CS, KERNEL_CS);
	vmcs_write(HOST_SEL_SS, KERNEL_DS);
	vmcs_write(HOST_SEL_DS, KERNEL_DS);
	vmcs_write(HOST_SEL_ES, KERNEL_DS);
	vmcs_write(HOST_SEL_FS, KERNEL_DS);
	vmcs_write(HOST_SEL_GS, KERNEL_DS);
	vmcs_write(HOST_SEL_TR, TSS_MAIN);
	vmcs_write(HOST_BASE_TR, tss_descr.base);
	vmcs_write(HOST_BASE_GDTR, gdt64_desc.base);
	vmcs_write(HOST_BASE_IDTR, idt_descr.base);
	vmcs_write(HOST_BASE_FS, 0);
	vmcs_write(HOST_BASE_GS, 0);

	/* Set other vmcs area */
	vmcs_write(PF_ERROR_MASK, 0);
	vmcs_write(PF_ERROR_MATCH, 0);
	vmcs_write(VMCS_LINK_PTR, ~0ul);
	vmcs_write(VMCS_LINK_PTR_HI, ~0ul);
	vmcs_write(HOST_RIP, (u64)(&vmx_return));
}

static void init_vmcs_guest(void)
{
	/* 26.3 CHECKING AND LOADING GUEST STATE */
	ulong guest_cr0, guest_cr4, guest_cr3;
	/* 26.3.1.1 */
	guest_cr0 = read_cr0();
	guest_cr4 = read_cr4();
	guest_cr3 = read_cr3();
	if (ctrl_enter & ENT_GUEST_64) {
		guest_cr0 |= X86_CR0_PG;
		guest_cr4 |= X86_CR4_PAE;
	}
	if ((ctrl_enter & ENT_GUEST_64) == 0)
		guest_cr4 &= (~X86_CR4_PCIDE);
	if (guest_cr0 & X86_CR0_PG)
		guest_cr0 |= X86_CR0_PE;
	vmcs_write(GUEST_CR0, guest_cr0);
	vmcs_write(GUEST_CR3, guest_cr3);
	vmcs_write(GUEST_CR4, guest_cr4);
	vmcs_write(GUEST_SYSENTER_CS,  KERNEL_CS);
	vmcs_write(GUEST_SYSENTER_ESP,
		(u64)(guest_syscall_stack + PAGE_SIZE - 1));
	vmcs_write(GUEST_SYSENTER_EIP, (u64)(&entry_sysenter));
	vmcs_write(GUEST_DR7, 0);
	vmcs_write(GUEST_EFER, rdmsr(MSR_EFER));

	/* 26.3.1.2 */
	vmcs_write(GUEST_SEL_CS, KERNEL_CS);
	vmcs_write(GUEST_SEL_SS, KERNEL_DS);
	vmcs_write(GUEST_SEL_DS, KERNEL_DS);
	vmcs_write(GUEST_SEL_ES, KERNEL_DS);
	vmcs_write(GUEST_SEL_FS, KERNEL_DS);
	vmcs_write(GUEST_SEL_GS, KERNEL_DS);
	vmcs_write(GUEST_SEL_TR, TSS_MAIN);
	vmcs_write(GUEST_SEL_LDTR, 0);

	vmcs_write(GUEST_BASE_CS, 0);
	vmcs_write(GUEST_BASE_ES, 0);
	vmcs_write(GUEST_BASE_SS, 0);
	vmcs_write(GUEST_BASE_DS, 0);
	vmcs_write(GUEST_BASE_FS, 0);
	vmcs_write(GUEST_BASE_GS, 0);
	vmcs_write(GUEST_BASE_TR, tss_descr.base);
	vmcs_write(GUEST_BASE_LDTR, 0);

	vmcs_write(GUEST_LIMIT_CS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_DS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_ES, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_SS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_FS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_GS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_LDTR, 0xffff);
	vmcs_write(GUEST_LIMIT_TR, tss_descr.limit);

	vmcs_write(GUEST_AR_CS, 0xa09b);
	vmcs_write(GUEST_AR_DS, 0xc093);
	vmcs_write(GUEST_AR_ES, 0xc093);
	vmcs_write(GUEST_AR_FS, 0xc093);
	vmcs_write(GUEST_AR_GS, 0xc093);
	vmcs_write(GUEST_AR_SS, 0xc093);
	vmcs_write(GUEST_AR_LDTR, 0x82);
	vmcs_write(GUEST_AR_TR, 0x8b);

	/* 26.3.1.3 */
	vmcs_write(GUEST_BASE_GDTR, gdt64_desc.base);
	vmcs_write(GUEST_BASE_IDTR, idt_descr.base);
	vmcs_write(GUEST_LIMIT_GDTR, gdt64_desc.limit);
	vmcs_write(GUEST_LIMIT_IDTR, idt_descr.limit);

	/* 26.3.1.4 */
	vmcs_write(GUEST_RIP, (u64)(&guest_entry));
	vmcs_write(GUEST_RSP, (u64)(guest_stack + PAGE_SIZE - 1));
	vmcs_write(GUEST_RFLAGS, 0x2);

	/* 26.3.1.5 */
	vmcs_write(GUEST_ACTV_STATE, ACTV_ACTIVE);
	vmcs_write(GUEST_INTR_STATE, 0);
}

static int init_vmcs(struct vmcs **vmcs)
{
	*vmcs = alloc_page();
	memset(*vmcs, 0, PAGE_SIZE);
	(*vmcs)->hdr.revision_id = basic.revision;
	/* vmclear first to init vmcs */
	if (vmcs_clear(*vmcs)) {
		printf("%s : vmcs_clear error\n", __func__);
		return 1;
	}

	if (make_vmcs_current(*vmcs)) {
		printf("%s : make_vmcs_current error\n", __func__);
		return 1;
	}

	/* All settings to pin/exit/enter/cpu
	   control fields should be placed here */
	ctrl_pin |= PIN_EXTINT | PIN_NMI | PIN_VIRT_NMI;
	ctrl_exit = EXI_LOAD_EFER | EXI_HOST_64;
	ctrl_enter = (ENT_LOAD_EFER | ENT_GUEST_64);
	/* DIsable IO instruction VMEXIT now */
	ctrl_cpu[0] &= (~(CPU_IO | CPU_IO_BITMAP));
	ctrl_cpu[1] = 0;

	ctrl_pin = (ctrl_pin | ctrl_pin_rev.set) & ctrl_pin_rev.clr;
	ctrl_enter = (ctrl_enter | ctrl_enter_rev.set) & ctrl_enter_rev.clr;
	ctrl_exit = (ctrl_exit | ctrl_exit_rev.set) & ctrl_exit_rev.clr;
	ctrl_cpu[0] = (ctrl_cpu[0] | ctrl_cpu_rev[0].set) & ctrl_cpu_rev[0].clr;

	init_vmcs_ctrl();
	init_vmcs_host();
	init_vmcs_guest();
	return 0;
}

static void init_vmx(void)
{
	ulong fix_cr0_set, fix_cr0_clr;
	ulong fix_cr4_set, fix_cr4_clr;

	vmxon_region = alloc_page();
	memset(vmxon_region, 0, PAGE_SIZE);

	fix_cr0_set = rdmsr(MSR_IA32_VMX_CR0_FIXED0);
	fix_cr0_clr = rdmsr(MSR_IA32_VMX_CR0_FIXED1);
	fix_cr4_set = rdmsr(MSR_IA32_VMX_CR4_FIXED0);
	fix_cr4_clr = rdmsr(MSR_IA32_VMX_CR4_FIXED1);
	basic.val = rdmsr(MSR_IA32_VMX_BASIC);
	ctrl_pin_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_PIN
			: MSR_IA32_VMX_PINBASED_CTLS);
	ctrl_exit_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_EXIT
			: MSR_IA32_VMX_EXIT_CTLS);
	ctrl_enter_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_ENTRY
			: MSR_IA32_VMX_ENTRY_CTLS);
	ctrl_cpu_rev[0].val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_PROC
			: MSR_IA32_VMX_PROCBASED_CTLS);
	if ((ctrl_cpu_rev[0].clr & CPU_SECONDARY) != 0)
		ctrl_cpu_rev[1].val = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
	else
		ctrl_cpu_rev[1].val = 0;
	if ((ctrl_cpu_rev[1].clr & (CPU_EPT | CPU_VPID)) != 0)
		ept_vpid.val = rdmsr(MSR_IA32_VMX_EPT_VPID_CAP);
	else
		ept_vpid.val = 0;

	write_cr0((read_cr0() & fix_cr0_clr) | fix_cr0_set);
	write_cr4((read_cr4() & fix_cr4_clr) | fix_cr4_set | X86_CR4_VMXE);

	*vmxon_region = basic.revision;

	guest_stack = alloc_page();
	memset(guest_stack, 0, PAGE_SIZE);
	guest_syscall_stack = alloc_page();
	memset(guest_syscall_stack, 0, PAGE_SIZE);
}

/* This function can only be called in guest */
static void __attribute__((__used__)) hypercall(u32 hypercall_no)
{
	u64 val = 0;
	val = (hypercall_no & HYPERCALL_MASK) | HYPERCALL_BIT;
	hypercall_field = val;
	asm volatile("vmcall\n\t");
}

static bool is_hypercall(void)
{
	ulong reason, hyper_bit;

	reason = vmcs_read(EXI_REASON) & 0xff;
	hyper_bit = hypercall_field & HYPERCALL_BIT;
	if (reason == VMX_VMCALL && hyper_bit)
		return true;
	return false;
}

static int handle_hypercall(void)
{
	ulong hypercall_no;

	hypercall_no = hypercall_field & HYPERCALL_MASK;
	hypercall_field = 0;
	switch (hypercall_no) {
	case HYPERCALL_VMEXIT:
		return VMX_VMEXIT;
	case HYPERCALL_VMABORT:
		return VMX_VMABORT;
	case HYPERCALL_VMSKIP:
		return VMX_VMSKIP;
	default:
		printf("ERROR : Invalid hypercall number : %ld\n", hypercall_no);
	}
	return VMX_EXIT;
}

static int exit_handler(void)
{
	int ret;

	if (current == NULL) {
		error("Error: current vm is NULL.\n");
		return -1;
	}

	current->exits++;
	regs.rflags = vmcs_read(GUEST_RFLAGS);
	if (is_hypercall())
		ret = handle_hypercall();
	else
		ret = current->exit_handler();
	vmcs_write(GUEST_RFLAGS, regs.rflags);

	return ret;
}

/*
 * Called if vmlaunch or vmresume fails.
 *	@early    - failure due to "VMX controls and host-state area" (26.2)
 *	@vmlaunch - was this a vmlaunch or vmresume
 *	@rflags   - host rflags
 */
static int
entry_failure_handler(struct vmentry_failure *failure)
{
	if (current == NULL) {
		error("Error: current vm is NULL.\n");
		return -1;
	}

	if (current->entry_failure_handler)
		return current->entry_failure_handler(failure);
	else
		return VMX_EXIT;
}

/*
 * Tries to enter the guest. Returns true iff entry succeeded. Otherwise,
 * populates @failure.
 */
static bool vmx_enter_guest(struct vmentry_failure *failure)
{
	failure->early = 0;

	in_guest = 1;
	asm volatile (
		"mov %[HOST_RSP], %%rdi\n\t"
		"vmwrite %%rsp, %%rdi\n\t"
		LOAD_GPR_C
		"cmpb $0, %[launched]\n\t"
		"jne 1f\n\t"
		"vmlaunch\n\t"
		"jmp 2f\n\t"
		"1: "
		"vmresume\n\t"
		"2: "
		SAVE_GPR_C
		"pushf\n\t"
		"pop %%rdi\n\t"
		"mov %%rdi, %[failure_flags]\n\t"
		"movl $1, %[failure_early]\n\t"
		"jmp 3f\n\t"
		"vmx_return:\n\t"
		SAVE_GPR_C
		"3: \n\t"
		: [failure_early]"+m"(failure->early),
		  [failure_flags]"=m"(failure->flags)
		: [launched]"m"(launched), [HOST_RSP]"i"(HOST_RSP)
		: "rdi", "memory", "cc"
	);
	in_guest = 0;

	failure->vmlaunch = !launched;
	failure->instr = launched ? "vmresume" : "vmlaunch";

	return !failure->early && !(vmcs_read(EXI_REASON) & VMX_ENTRY_FAILURE);
}

static void __attribute__((__used__)) guest_main(void)
{
	current->guest_main();
}

/* guest_entry */
asm(
	".align	4, 0x90\n\t"
	".globl	entry_guest\n\t"
	"guest_entry:\n\t"
	"	call guest_main\n\t"
	"	mov $1, %edi\n\t"
	"	call hypercall\n\t"
);

static void __attribute__((__used__)) syscall_handler(u64 syscall_no)
{
	if (current->syscall_handler)
		current->syscall_handler(syscall_no);
}

asm(
	".align	4, 0x90\n\t"
	".globl	entry_sysenter\n\t"
	"entry_sysenter:\n\t"
	SAVE_GPR
	"	and	$0xf, %rax\n\t"
	"	mov	%rax, %rdi\n\t"
	"	call	syscall_handler\n\t"
	LOAD_GPR
	"	vmresume\n\t"
);

void print_vmexit_info()
{
	u64 guest_rip, guest_rsp;
	ulong reason = vmcs_read(EXI_REASON) & 0xff;
	ulong exit_qual = vmcs_read(EXI_QUALIFICATION);
	guest_rip = vmcs_read(GUEST_RIP);
	guest_rsp = vmcs_read(GUEST_RSP);
	printf("VMEXIT info:\n");
	printf("\tvmexit reason = %ld\n", reason);
	printf("\texit qualification = %#lx\n", exit_qual);
	printf("\tBit 31 of reason = %lx\n", (vmcs_read(EXI_REASON) >> 31) & 1);
	printf("\tguest_rip = %#lx\n", guest_rip);
	printf("\tRAX=%#lx    RBX=%#lx    RCX=%#lx    RDX=%#lx\n",
		regs.rax, regs.rbx, regs.rcx, regs.rdx);
	printf("\tRSP=%#lx    RBP=%#lx    RSI=%#lx    RDI=%#lx\n",
		guest_rsp, regs.rbp, regs.rsi, regs.rdi);
	printf("\tR8 =%#lx    R9 =%#lx    R10=%#lx    R11=%#lx\n",
		regs.r8, regs.r9, regs.r10, regs.r11);
	printf("\tR12=%#lx    R13=%#lx    R14=%#lx    R15=%#lx\n",
		regs.r12, regs.r13, regs.r14, regs.r15);
}

void
print_vmentry_failure_info(struct vmentry_failure *failure) {
	if (failure->early) {
		printf("Early %s failure: ", failure->instr);
		switch (failure->flags & VMX_ENTRY_FLAGS) {
		case X86_EFLAGS_CF:
			printf("current-VMCS pointer is not valid.\n");
			break;
		case X86_EFLAGS_ZF:
			printf("error number is %ld. See Intel 30.4.\n",
			       vmcs_read(VMX_INST_ERROR));
			break;
		default:
			printf("unexpected flags %lx!\n", failure->flags);
		}
	} else {
		u64 reason = vmcs_read(EXI_REASON);
		u64 qual = vmcs_read(EXI_QUALIFICATION);

		printf("Non-early %s failure (reason=%#lx, qual=%#lx): ",
			failure->instr, reason, qual);

		switch (reason & 0xff) {
		case VMX_FAIL_STATE:
			printf("invalid guest state\n");
			break;
		case VMX_FAIL_MSR:
			printf("MSR loading\n");
			break;
		case VMX_FAIL_MCHECK:
			printf("machine-check event\n");
			break;
		default:
			printf("unexpected basic exit reason %ld\n",
			       reason & 0xff);
		}

		if (!(reason & VMX_ENTRY_FAILURE))
			printf("\tVMX_ENTRY_FAILURE BIT NOT SET!\n");

		if (reason & 0x7fff0000)
			printf("\tRESERVED BITS SET!\n");
	}
}

static int vmx_run(void)
{
	while (1) {
		u32 ret;
		bool entered;
		struct vmentry_failure failure;

		entered = vmx_enter_guest(&failure);

		if (entered) {
			/*
			 * VMCS isn't in "launched" state if there's been any
			 * entry failure (early or otherwise).
			 */
			launched = 1;
			ret = exit_handler();
		} else {
			ret = entry_failure_handler(&failure);
		}

		switch (ret) {
		case VMX_RESUME:
			continue;
		case VMX_VMEXIT:
			guest_finished = 1;
			return 0;
		case VMX_EXIT:
			break;
		default:
			printf("ERROR : Invalid %s_handler return val %d.\n",
			       entered ? "exit" : "entry_failure",
			       ret);
			break;
		}

		if (entered)
			print_vmexit_info();
		else
			print_vmentry_failure_info(&failure);
		abort();
	}
}

extern struct asor_guest asor_guests[];

static int x86_virt_enable(void)
{
	struct asor_guest *guest = &asor_guests[0];
	current = guest;

	if (!is_vmx_supported()) {
		error("Error: vmx not supported.\n");
		return 1;
	}

	init_vmx();

	if (vmx_on()) {
		error("Error: %s : vmxon failed.\n", __func__);
		return 1;
	}

	init_vmcs(&(guest->vmcs));

	setup_ept(true);

	vmx_run();

	if (vmx_off()) {
		error("Error: %s : vmxoff failed.\n", __func__);
		return 1;
	}

	return 0;
}

static const struct virt_arch_ops x86_virt_arch_ops = {
	.enable = x86_virt_enable,
};

const struct virt_arch_ops *get_x86_virt_arch_ops(void)
{
	return &x86_virt_arch_ops;
}
