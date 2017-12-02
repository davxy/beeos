all: $(libc)
	$(MAKE) -C libc
	$(MAKE) -C kernel
	$(MAKE) -C libu
	$(MAKE) -C user

clean:
	$(MAKE) -C libc clean
	$(MAKE) -C kernel clean
	$(MAKE) -C libu clean
	$(MAKE) -C user clean

purge:
	$(MAKE) -C libc purge
	$(MAKE) -C kernel purge
	$(MAKE) -C libu purge
	$(MAKE) -C user purge
