dirs += dirent \
		errno \
		stdio \
		stdlib \
		string \
		unistd \
		signal \
		sys \
		stdint \
		time

ifeq ($(ARCH),x86)
dirs += arch/x86
else ifeq ($(ARCH),x86_64)
dirs += arch/x86_64
else ifeq ($(ARCH),arm)
dirs += arch/arm
endif
