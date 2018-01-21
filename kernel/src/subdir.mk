local_sources := kmain.c \
				 kprintf.c \
				 kmalloc.c \
				 panic.c \
				 isr.c \
				 elf.c \
				 timer.c

dirs := driver fs mm proc sync sys ipc

ifeq ($(ARCH),x86)
dirs += arch/x86
else ifeq ($(ARCH),x86_64)
dirs += arch/x86_64
else ifeq ($(ARCH),arm)
dirs += arch/arm
endif
