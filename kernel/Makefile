include ../common.mk

CPPFLAGS += -Isrc -Iinclude
# -nostdlib avoids any dependency on the host 32-bit shared libraries
# (libc, libgcc_s). Only the static libgcc is needed (e.g. for __divdi3).
LDFLAGS  += -nostartfiles -nostdlib

kernel := $(BINARY_DIR)/kernel

all: $(kernel)

$(kernel): $(objects) $(libc)
	$(LINK.o) $^ -lgcc -o $@
	#$(OBJCOPY) --only-keep-debug $@ $@.sym
	cp $@ $@.sym
	$(STRIP) $@

tags: $(kernel)
	ctags -R --c++-kinds=+p --fields=+S .

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(depends)),)
-include $(depends)
endif
endif
