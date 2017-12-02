include ../common.mk

CFLAGS  += -nostdlib -nodefaultlibs
ASFLAGS += -nostdlib -nodefaultlibs

library := $(BINARY_DIR)/libc.a

all: $(library)

$(library): $(objects)
	$(AR) $(ARFLAGS) $@ $?

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(depends)),)
-include $(depends)
endif
endif
