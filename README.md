README
------

  A simple and minimal "Unix-like" kernel trying to be POSIX compliant.
  (at least with regard to the implemented services).

  The kernel is **NOT STABLE** and is a discontinuous work mainly used as
  a didactical tool for operating systems and low level programming.

  Because of it's simplicity it can be used as a trampoline to start 
  digging with common operating system problems.
  Once you've mastered the BeeOS sources it will be a lot easier to start 
  hacking with more complex projects like Linux.

### Quickstart

1. `make all`

    This will build the kernel, standard library (libc), user utility
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
    - Virtual File System (ext2 and custom initrd)
    - SLAB  memory allocator
    - Buddy memory allocator
    - Timer queues

**IPC**

    - Signals
    - Pipes
    - System V messages
    
**Synchronization**

    - Spinlocks
    - Wait queues

**Drivers**

    - TTY
    - Ramdisk
    - Keyboard

CONTRIBUTIONS
-------------

  All contributions to the project will be listed in the AUTHORS document.


CONTACT
-------

    Davide Galassi <davxy@datawok.net>


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
      Common user level functionalities.

    user/
      Various user applications and test programs.

