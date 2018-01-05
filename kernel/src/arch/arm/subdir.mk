#TARGET_ARCH := -mcpu=arm926ej-s
#TARGET_MACH := -mcpu=arm926ej-s

LDFLAGS += -Tsrc/arch/arm/board/raspi2/linker.ld

#local_sources := cpuinfo.c \
				 idle.c \
				 init.c \
				 low_level_init.c \
				 startup.S \
				 timer.c \
				 task.c \
				 sync.c \
				 uart.c

local_sources := boot.S \
				 kernel_main.c
