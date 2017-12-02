include ../common.mk

CPPFLAGS += -Iinclude -Isrc
CFLAGS  += -nostdlib -nodefaultlibs
ASFLAGS += -nostdlib -nodefaultlibs

library := $(BINARY_DIR)/libu.a

all: $(library)

$(library): $(objects)
	$(AR) $(ARFLAGS) $@ $?

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(depends)),)
-include $(depends)
endif
endif
