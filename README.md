BeeOS
=====


README
------

  A simple and minimal "Unix-like" kernel trying to be POSIX compliant.
  (at least with respect to the implemented services).

  The kernel is **NOT STABLE** for production and is a discontinuous work
  mainly used as a didactical tool for operating systems and low level
  programming.

  Because of it's simplicity it can be used as a trampoline to start 
  digging with common operating system problems.

  Once you've mastered the BeeOS sources it will be a lot easier to start 
  hacking with more complex OS projects.


### Prerequisites

  - gcc
  - binutils
  - make
  - qemu (qemu-system)


### Quickstart

1. `make all`

    This command will build the kernel, standard library (libc), user utility
    library (libu) and user applications.

2. `(cd misc && sudo ./mkfs.sh)`

    Creates the root filesystem ramdisk with the user applications.

3. `(cd misc && ./qemu.sh)`

    Starts qemu and BeeOS.


### Implemented Milestones
  
**Supported Architectures**
  
  - x86

**Core**

  - Multitasking
  - Syscalls
  - Virtual Memory
  - Virtual File System: ext2, devfs
  - SLAB  allocator
  - Buddy allocator
  - Timer queues

**IPC**

  - Signals
  - Pipes
    
**Synchronization**

  - Spinlocks
  - Wait queues

**Drivers**

  - TTY
  - Screen
  - Ramdisk
  - Keyboard
  - PRNG

**Other**

  - Multi TTY console


CONTRIBUTIONS
-------------

  All contributions to the project will be listed in the AUTHORS file.
  

LICENSING
---------

  BeeOS is free software. See the file COPYING for licensing conditions.


CONTENTS
--------

    ChangeLog
      Version history

    COPYING
      Copying, distribution and modification conditions and terms.

    kernel/
      BeeOS kernel sources.

    libc/
      BeeOS standard C library.

    libu/
      Common user functions.

    user/
      Various user applications and test programs.

