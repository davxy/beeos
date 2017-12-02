include ../common.mk

CPPFLAGS += -I../libu/include
LDFLAGS  += -nostdlib
LDLIBS   := ../libu/build/$(ARCH)/libu.a ../libc/build/$(ARCH)/libc.a -lgcc 

programs = $(call src_to_bin_dir,$(basename $(sources)))

all: $(programs)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(depends)),)
-include $(depends)
endif
endif
