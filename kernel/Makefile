include ../common.mk

CPPFLAGS += -Isrc -Iinclude
LDFLAGS  += -nostartfiles

kernel := $(BINARY_DIR)/kernel
 
all: $(kernel)
	
$(kernel): $(objects) $(libc)
	$(LINK.o) $^ -o $@
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
