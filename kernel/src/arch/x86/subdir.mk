LDFLAGS += -Tsrc/arch/x86/linker.ld

local_sources := init.S \
				 startup.S \
				 pic.c \
				 isr_stub.S \
				 console.c \
				 cpuinfo.c \
				 gdt.c \
				 idle.c \
				 idt.c \
				 kbd.c \
				 arch_init.c \
				 paging.c \
				 stack_trace.c \
				 task.c \
				 timer.c \
				 uart.c
