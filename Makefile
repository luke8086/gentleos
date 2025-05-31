CC 		:= clang
LD 		:= ld
NASM 	:= nasm

BASEDIR 	:= .
BUILDDIR 	:= $(BASEDIR)/build

EMPTY_DISK_IMAGE	:= $(BASEDIR)/misc/empty-disk.img
DISK_IMAGE 			:= $(BUILDDIR)/disk.img
DISK_FS_OFFSET 		:= 1048576

CFLAGS 	:=  -std=c11 -m32 -march=i386 \
			-ffreestanding -Wall -Wextra -pedantic \
			-I$(BASEDIR)/include

ASFLAGS :=

LDFLAGS := 	-m elf_i386 -nostdlib -z nodefaultlib \
			-z noexecstack --no-warn-rwx-segments \
			-T$(BASEDIR)/misc/kernel.ld

SUBDIRS := gui apps lib kernel data
CONFIG_H := $(BASEDIR)/include/config.h
C_SRCS  := $(foreach d,$(SUBDIRS),$(wildcard $(d)/*.c))
S_SRCS  := $(foreach d,$(SUBDIRS),$(wildcard $(d)/*.s))
SRCS    := $(C_SRCS) $(S_SRCS)
OBJS    := $(patsubst %.c,$(BUILDDIR)/%.o,$(C_SRCS)) \
           $(patsubst %.s,$(BUILDDIR)/%.o,$(S_SRCS))
DEPS    := $(OBJS:.o=.d)
OBJDIRS := $(addprefix $(BUILDDIR)/,$(SUBDIRS))

all: disk

clean:
	rm -rf $(BUILDDIR)

$(OBJDIRS):
	@mkdir -p $@

$(CONFIG_H):
	[ -f $@ ] || cp $(BASEDIR)/include/config.sample.h $@

$(BUILDDIR)/%.o: %.c | $(OBJDIRS) $(CONFIG_H)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR)/%.o: %.s | $(OBJDIRS)
	$(NASM) $(ASFLAGS) -f elf32 $< -o $@

kernel: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(BUILDDIR)/kernel.elf

disk: kernel
	zcat $(EMPTY_DISK_IMAGE) > $(DISK_IMAGE)
	mcopy -D o -i $(DISK_IMAGE)@@$(DISK_FS_OFFSET) $(BUILDDIR)/kernel.elf ::
	mcopy -D o -i $(DISK_IMAGE)@@$(DISK_FS_OFFSET) $(BASEDIR)/misc/grub.cfg ::boot/grub

print:
	@echo "SUBDIRS=$(SUBDIRS)"
	@echo "SRCS=$(SRCS)"
	@echo "OBJS=$(OBJS)"

.PHONY: all clean kernel disk print

# Include auto-generated dependency files if they exist
-include $(DEPS)
