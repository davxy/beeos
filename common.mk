#ARCH := $(shell uname -m)
ARCH := x86
#ARCH := arm

libc := ../libc/build/$(ARCH)/libc.a

################################################################################
# Common programs and flags
#
CC := gcc
#
AR := ar
#
OBJCOPY := objcopy
#
STRIP := strip
#
RM := rm -rf
#
CPPFLAGS := -I../libc/include
# 
CFLAGS := -O0 -g -Wall -MMD -MP -nostdinc -fno-builtin \
		  -fno-stack-protector -fno-pic -masm=intel
#
ASFLAGS := -g -Wall -MMD -MP -nostdinc -fno-builtin
#
LDFLAGS := -ffreestanding
#
# Standard C library relative path
LDLIBS := $(libc)
#
BINARY_DIR := build/$(ARCH)
SOURCE_DIR := src
###############################################################################

ifeq ($(ARCH),x86)

TARGET_ARCH := -m32
TARGET_MACH := -m32

else ifeq ($(ARCH),x86_64)

TARGET_ARCH := -m64
TARGET_MACH := -m64

else ifeq ($(ARCH),arm)

PREFIX	:= /home/dave/dev/toolchain/arm-none-eabi/bin/arm-none-eabi-
CC 		:= $(addprefix $(PREFIX),$(CC))
OBJCOPY := $(addprefix $(PREFIX),$(OBJCOPY))
STRIP 	:= $(addprefix $(PREFIX),$(STRIP))
AR		:= $(addprefix $(PREFIX),$(AR))

endif

################################################################################
# Common macro functions

# $(call src_to_bin_dir,list)
# SOURCE_DIR and BINARY_DIR defined by Makefiles
src_to_bin_dir = $(patsubst $(SOURCE_DIR)%,$(BINARY_DIR)%,$1)

# $(call include_dir,dir_name)
define include_dir 
$(shell mkdir -p $(call src_to_bin_dir,$1))
dirs :=
local_sources :=
include $1/subdir.mk
sources += $$(if $$(local_sources),$$(addprefix $1/,$$(local_sources)))
$$(foreach dir,$$(dirs),$$(eval $$(call include_dir,$1/$$(dir))))
endef

sources :=
objects  = $(call src_to_bin_dir,$(addsuffix .o,$(basename $(sources))))
depends  = $(patsubst %.o,%.d,$(objects))

$(eval $(call include_dir,$(SOURCE_DIR)))

.PHONY: all clean purge

all:

clean:
	$(RM) $(BINARY_DIR)

purge:
	$(RM) build

$(BINARY_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(COMPILE.c) -o $@ $<

$(BINARY_DIR)/%.o: $(SOURCE_DIR)/%.S
	$(COMPILE.S) -o $@ $<

$(BINARY_DIR)/%: $(SOURCE_DIR)/%.c
	$(LINK.c) -o $@ $^ $(LDLIBS)
	$(OBJCOPY) --only-keep-debug $@ $@.sym
	chmod -x $@.sym
	$(STRIP) $@
