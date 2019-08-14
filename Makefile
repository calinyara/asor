
SHELL := /usr/bin/env bash

ifeq ($(wildcard config.mak),)
$(error run ./configure first. See ./configure -h)
endif

include config.mak

# Set search path for all sources
VPATH = $(SRCDIR)

libdirs-get = $(shell [ -d "lib/$(1)" ] && echo "lib/$(1) lib/$(1)/asm")
ARCH_LIBDIRS := $(call libdirs-get,$(ARCH)) $(call libdirs-get,$(ASOR_DIR))
OBJDIRS := $(ARCH_LIBDIRS)

DESTDIR := $(PREFIX)/share/asor/

.PHONY: arch_clean clean distclean cscope

#make sure env CFLAGS variable is not used
CFLAGS =

libgcc := $(shell $(CC) --print-libgcc-file-name)

libasor := lib/libasor.a
asorobjs := \
	lib/argv.o \
	lib/printf.o \
	lib/string.o \
	lib/abort.o \
	lib/report.o \
	lib/stack.o \
	driver/monitor.o \
	core/virt.o

# libfdt paths
LIBFDT_objdir = lib/libfdt
LIBFDT_srcdir = $(SRCDIR)/lib/libfdt
LIBFDT_archive = $(LIBFDT_objdir)/libfdt.a
LIBFDT_include = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_INCLUDES))
LIBFDT_version = $(addprefix $(LIBFDT_srcdir)/,$(LIBFDT_VERSION))

OBJDIRS += $(LIBFDT_objdir)

#include architecture specific make rules
include $(SRCDIR)/$(ASOR_DIR)/Makefile

# cc-option
# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

COMMON_CFLAGS += -g $(autodepend-flags)
COMMON_CFLAGS += -Wall -Wwrite-strings -Wclobbered -Wempty-body -Wuninitialized
COMMON_CFLAGS += -Wignored-qualifiers -Wunused-but-set-parameter
COMMON_CFLAGS += -Wmissing-prototypes -Wstrict-prototypes
# COMMON_CFLAGS += -Werror -Wno-unused-result
COMMON_CFLAGS += -ggdb -gstabs+
frame-pointer-flag=-f$(if $(KEEP_FRAME_POINTER),no-,)omit-frame-pointer
fomit_frame_pointer := $(call cc-option, $(frame-pointer-flag), "")
fnostack_protector := $(call cc-option, -fno-stack-protector, "")
fnostack_protector_all := $(call cc-option, -fno-stack-protector-all, "")
wno_frame_address := $(call cc-option, -Wno-frame-address, "")
fno_pic := $(call cc-option, -fno-pic, "")
no_pie := $(call cc-option, -no-pie, "")
COMMON_CFLAGS += $(fomit_frame_pointer)
COMMON_CFLAGS += $(fno_stack_protector)
COMMON_CFLAGS += $(fno_stack_protector_all)
COMMON_CFLAGS += $(wno_frame_address)
COMMON_CFLAGS += $(if $(U32_LONG_FMT),-D__U32_LONG_FMT__,)
COMMON_CFLAGS += $(fno_pic) $(no_pie)

CFLAGS += $(COMMON_CFLAGS)
CFLAGS += -Wmissing-parameter-type -Wold-style-declaration -Woverride-init

CXXFLAGS += $(COMMON_CFLAGS)

autodepend-flags = -MMD -MF $(dir $*).$(notdir $*).d

LDFLAGS += $(CFLAGS)

$(libasor): $(asorobjs)
	$(AR) rcs $@ $^

include $(LIBFDT_srcdir)/Makefile.libfdt
$(LIBFDT_archive): CFLAGS += -ffreestanding -I $(SRCDIR)/lib -I $(SRCDIR)/lib/libfdt -Wno-sign-compare
$(LIBFDT_archive): $(addprefix $(LIBFDT_objdir)/,$(LIBFDT_OBJS))
	$(AR) rcs $@ $^


# Build directory target
.PHONY: directories
directories:
	@mkdir -p $(OBJDIRS)

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

-include */.*.d */*/.*.d

all: directories $(shell cd $(SRCDIR) && git rev-parse --verify --short=8 HEAD >$(PWD)/build-head 2>/dev/null)

standalone: all
	@scripts/mkstandalone.sh

install: standalone
	mkdir -p $(DESTDIR)
	install tests/* $(DESTDIR)

clean: arch_clean
	$(RM) lib/.*.d core/.*.d $(libasor) $(asorobjs) asor

libfdt_clean:
	$(RM) $(LIBFDT_archive) \
	$(addprefix $(LIBFDT_objdir)/,$(LIBFDT_OBJS)) \
	$(LIBFDT_objdir)/.*.d

distclean: clean libfdt_clean
	$(RM) lib/asm lib/config.h config.mak $(ASOR_DIR)-run msr.out cscope.* build-head
	$(RM) -r tests logs logs.old

cscope: cscope_dirs = lib lib/libfdt lib/linux $(ASOR_DIR) $(ARCH_LIBDIRS) lib/asm-generic
cscope:
	$(RM) ./cscope.*
	find -L $(cscope_dirs) -maxdepth 1 \
		-name '*.[chsS]' -exec realpath --relative-base=$(PWD) {} \; | sort -u > ./cscope.files
	cscope -bk

.PHONY:img
img: all
	[ -d /mnt/asor ] || sudo mkdir -p /mnt/asor
	sudo kpartx -av asor.img
	sleep 1  # sleep a sec, wait for kpartx to create the device nodes
	sudo mount /dev/mapper/loop0p1 /mnt/asor
	# asor.elf: ELF 64-bit LSB executable,for x86-64
	# asor.flat: ELF 32-bit LSB executable, for Intel 80386
	sudo cp x86/asor.elf /mnt/asor/asor
	sync
	sudo umount /mnt/asor
	sudo kpartx -d asor.img
	cp x86/asor.elf asor

.PHONY:qemu
qemu:
	qemu -hda asor.img -boot c -cpu host -enable-kvm -serial stdio

.PHONY:debug
debug:
	qemu -S -s -hda asor.img -boot c -cpu host -enable-kvm &
	sleep 1
	cgdb -x scripts/gdbinit
