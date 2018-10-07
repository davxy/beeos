/*
 * Copyright (c) 2015-2018, Davide Galassi. All rights reserved.
 *
 * This file is part of the BeeOS software.
 *
 * BeeOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BeeOS; if not, see <http://www.gnu/licenses/>.
 */

#include "kbd.h"
#include "isr.h"
#include "io.h"
#include "kprintf.h"
#include "driver/tty.h"
#include "proc.h"
#include "sys.h"

/* Keyboard base address port. */
#define KEYB_PORT               0x60
/* Keyboard acknowledge register. */
#define KEYB_ACK                0x61
/* Keyboard status register. */
#define KEYB_STATUS             0x64
/* Keyboard LED register. */
#define KEYB_LED_CODE           0xED

/* The keyboard controller is busy. */
#define KEYB_BUSY               0x02
/* Command to set the typematic delay and rate. */
#define KEYB_SET_TYPEMATIC      0xF3

/* Special keycodes */
#define KEY_NULL                0x00
#define KEY_ESCAPE              0x1B
#define KEY_HOME                0xE0
#define KEY_END                 0xE1
#define KEY_UP                  0xE2
#define KEY_DOWN                0xE3
#define KEY_LEFT                0xE4
#define KEY_RIGHT               0xE5
#define KEY_PAGE_UP             0xE6
#define KEY_PAGE_DOWN           0xE7
#define KEY_INSERT              0xE8
#define KEY_DELETE              0xE9

#define KBD_STATUS_SHIFT        0x01
#define KBD_STATUS_CTRL         0x02
#define KBD_STATUS_ALT          0x04
#define KBD_STATUS_CAPS_LCK     0x08

/*
 * Primary meaning of scancodes.
 */
static const char kbd_map1[] = {
    KEY_NULL,           /* 0x00 - Null */
    KEY_ESCAPE,         /* 0x01 - Escape  */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b',               /* 0x0e - Backspace */
    '\t',               /* 0x0f - Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n',               /* 0x1e - Enter */
    KEY_NULL,           /* 0x1d - Left Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_NULL,           /* 0x2a - Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    KEY_NULL,           /* 0x36 - Right Shift */
    KEY_NULL,           /* 0x37 - Print Screen */
    KEY_NULL,           /* 0x38 - Left Alt */
    ' ',
    KEY_NULL,           /* 0x3a - CapsLock */
    KEY_NULL,           /* 0x3b - F1 */
    KEY_NULL,           /* 0x3c - F2 */
    KEY_NULL,           /* 0x3d - F3 */
    KEY_NULL,           /* 0x3e - F4 */
    KEY_NULL,           /* 0x3f - F5 */
    KEY_NULL,           /* 0x40 - F6 */
    KEY_NULL,           /* 0x41 - F7 */
    KEY_NULL,           /* 0x42 - F8 */
    KEY_NULL,           /* 0x43 - F9 */
    KEY_NULL,           /* 0x44 - F10 */
    KEY_NULL,           /* 0x45 - NumLock */
    KEY_NULL,           /* 0x46 - ScrollLock */
    KEY_HOME,           /* 0x47 - Home */
    KEY_UP,             /* 0x48 - Up Arrow */
    KEY_PAGE_UP,        /* 0x49 - Page Up */
    '-',
    KEY_LEFT,           /* 0x4b - Left Arrow */
    '5',                /* 0x4c - Numpad Center */
    KEY_RIGHT,          /* 0x4d - Right Arrow */
    '+',
    KEY_END,            /* 0x4f - End */
    KEY_DOWN,           /* 0x50 - Down Arrow */
    KEY_PAGE_DOWN,      /* 0x51 - Page Down */
    KEY_INSERT,         /* 0x52 - Insert */
    KEY_DELETE,         /* 0x53 - Delete */
    KEY_NULL,           /* 0x54 - Alt-SysRq */
    KEY_NULL,           /* 0x55 - F11/F12/PF1/FN */
    KEY_NULL,           /* 0x56 - unlabelled key next to LAlt */
    KEY_NULL,           /* 0x57 - F11 */
    KEY_NULL,           /* 0x58 - F12 */
    KEY_NULL,           /* 0x59 */
    KEY_NULL,           /* 0x5a */
    KEY_NULL,           /* 0x5b */
    KEY_NULL,           /* 0x5c */
    KEY_NULL,           /* 0x5d */
    KEY_NULL,           /* 0x5e */
    KEY_NULL,           /* 0x5f */
    KEY_NULL,           /* 0x60 */
    KEY_NULL,           /* 0x61 */
    KEY_NULL,           /* 0x62 */
    KEY_NULL,           /* 0x63 */
    KEY_NULL,           /* 0x64 */
    KEY_NULL,           /* 0x65 */
    KEY_NULL,           /* 0x66 */
    KEY_NULL,           /* 0x67 */
    KEY_NULL,           /* 0x68 */
    KEY_NULL,           /* 0x69 */
    KEY_NULL,           /* 0x6a */
    KEY_NULL,           /* 0x6b */
    KEY_NULL,           /* 0x6c */
    KEY_NULL,           /* 0x6d */
    KEY_NULL,           /* 0x6e */
    KEY_NULL,           /* 0x6f */
    KEY_NULL,           /* 0x70 */
    KEY_NULL,           /* 0x71 */
    KEY_NULL,           /* 0x72 */
    KEY_NULL,           /* 0x73 */
    KEY_NULL,           /* 0x74 */
    KEY_NULL,           /* 0x75 */
    KEY_NULL,           /* 0x76 */
    KEY_NULL,           /* 0x77 */
    KEY_NULL,           /* 0x78 */
    KEY_NULL,           /* 0x79 */
    KEY_NULL,           /* 0x7a */
    KEY_NULL,           /* 0x7b */
    KEY_NULL,           /* 0x7c */
    KEY_NULL,           /* 0x7d */
    KEY_NULL,           /* 0x7e */
    KEY_NULL            /* 0x7f */
};

/*
 * Secondary meaning of scancodes.
 */
static const char kbd_map2[] = {
    KEY_NULL,           /* 0x00 - Undefined */
    KEY_ESCAPE,         /* 0x01 - Escape */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b',               /* 0x0e - Backspace */
    '\t',               /* 0x0f - Tab */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    '\n',               /* 0x1e - Enter */
    KEY_NULL,           /* 0x1d - Left Ctrl */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    KEY_NULL,           /* 0x2a - Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    KEY_NULL,           /* 0x36 - Right Shift */
    KEY_NULL,           /* 0x37 - Print Screen */
    KEY_NULL,           /* 0x38 - Left Alt */
    ' ',
    KEY_NULL,           /* 0x3a - CapsLock */
    KEY_NULL,           /* 0x3b - F1 */
    KEY_NULL,           /* 0x3c - F2 */
    KEY_NULL,           /* 0x3d - F3 */
    KEY_NULL,           /* 0x3e - F4 */
    KEY_NULL,           /* 0x3f - F5 */
    KEY_NULL,           /* 0x40 - F6 */
    KEY_NULL,           /* 0x41 - F7 */
    KEY_NULL,           /* 0x42 - F8 */
    KEY_NULL,           /* 0x43 - F9 */
    KEY_NULL,           /* 0x44 - F10 */
    KEY_NULL,           /* 0x45 - NumLock */
    KEY_NULL,           /* 0x46 - ScrollLock */
    KEY_HOME,           /* 0x47 - Home */
    KEY_UP,             /* 0x48 - Up Arrow */
    KEY_PAGE_UP,        /* 0x49 - Page Up */
    '-',
    KEY_LEFT,           /* 0x4b - Left Arrow */
    '5',                /* 0x4c - Numpad Center */
    KEY_RIGHT,          /* 0x4d - Right Arrow */
    '+',
    KEY_END,            /* 0x4f - Page End */
    KEY_DOWN,           /* 0x50 - Down Arrow */
    KEY_PAGE_DOWN,      /* 0x51 - Page Down */
    KEY_INSERT,         /* 0x52 - Insert */
    KEY_DELETE,         /* 0x53 - Delete */
    KEY_NULL,           /* 0x54 - Alt-SysRq */
    KEY_NULL,           /* 0x55 - F11/F12/PF1/FN */
    KEY_NULL,           /* 0x56 - unlabelled key next to LAlt */
    KEY_NULL,           /* 0x57 - F11 */
    KEY_NULL,           /* 0x58 - F12 */
    KEY_NULL,           /* 0x59 */
    KEY_NULL,           /* 0x5a */
    KEY_NULL,           /* 0x5b */
    KEY_NULL,           /* 0x5c */
    KEY_NULL,           /* 0x5d */
    KEY_NULL,           /* 0x5e */
    KEY_NULL,           /* 0x5f */
    KEY_NULL,           /* 0x60 */
    KEY_NULL,           /* 0x61 */
    KEY_NULL,           /* 0x62 */
    KEY_NULL,           /* 0x63 */
    KEY_NULL,           /* 0x64 */
    KEY_NULL,           /* 0x65 */
    KEY_NULL,           /* 0x66 */
    KEY_NULL,           /* 0x67 */
    KEY_NULL,           /* 0x68 */
    KEY_NULL,           /* 0x69 */
    KEY_NULL,           /* 0x6a */
    KEY_NULL,           /* 0x6b */
    KEY_NULL,           /* 0x6c */
    KEY_NULL,           /* 0x6d */
    KEY_NULL,           /* 0x6e */
    KEY_NULL,           /* 0x6f */
    KEY_NULL,           /* 0x70 */
    KEY_NULL,           /* 0x71 */
    KEY_NULL,           /* 0x72 */
    KEY_NULL,           /* 0x73 */
    KEY_NULL,           /* 0x74 */
    KEY_NULL,           /* 0x75 */
    KEY_NULL,           /* 0x76 */
    KEY_NULL,           /* 0x77 */
    KEY_NULL,           /* 0x78 */
    KEY_NULL,           /* 0x79 */
    KEY_NULL,           /* 0x7a */
    KEY_NULL,           /* 0x7b */
    KEY_NULL,           /* 0x7c */
    KEY_NULL,           /* 0x7d */
    KEY_NULL,           /* 0x7e */
    KEY_NULL            /* 0x7f */
};

/*
 * Get scan code and ack the controller.
 */
static unsigned int scan_key(void)
{
    unsigned int cod;
    unsigned int val;

    cod = inb(KEYB_PORT);
    val = inb(KEYB_ACK);
    outb(KEYB_ACK, val | 0x80);
    outb(KEYB_ACK, val);
    return cod;
}

static void kill_tty_group(void)
{
    struct task *t = current;
    pid_t pgid;

    pgid = sys_tcgetpgrp(0);
    do {
        if (t->pgid == pgid)
            task_signal(t, SIGINT);
        t = list_container(t->tasks.next, struct task, tasks);
    } while (t != current);
}

/*
 * Keyboard interrupt handler
 */
static void kbd_handler(void)
{
    static unsigned int kbd_status = 0; /* keeps track of CTRL, ALT, SHIFT */
    unsigned int c;

    c = scan_key();
    switch (c) {
    case 0x2A:  /* LShift down */
    case 0x36:  /* RShift down */
        kbd_status |= KBD_STATUS_SHIFT;
        break;
    case 0xAA:  /* LShift up */
    case 0xB6:  /* RShift up */
        kbd_status &= ~KBD_STATUS_SHIFT;
        break;
    case 0x1D:
        kbd_status |= KBD_STATUS_CTRL;
        break;
    case 0x9D:
        kbd_status &= ~KBD_STATUS_CTRL;
        break;
    case 0x38:
        kbd_status |= KBD_STATUS_ALT;
        break;
    case 0xB8:
        kbd_status &= ~KBD_STATUS_ALT;
        break;
    case 0x3A:
        kbd_status ^= KBD_STATUS_CAPS_LCK;
        break;
    default:
        if ((c & 0x80) == 0) {
            if ((kbd_status & KBD_STATUS_CTRL) != 0) {
                c = kbd_map1[c];
                if (c == 'c' || c == 'C') {
                    /* Kill all the process in the group */
                    kill_tty_group();
                }
                c = '\n';
            } else if ((kbd_status & KBD_STATUS_ALT) != 0) {
                /*kprintf("ALT + %c (0x%x)\n", c, c);*/
                tty_change(c - 0x3B);
                return;
            } else if ((((kbd_status & KBD_STATUS_SHIFT) != 0)
                    ^ ((kbd_status & KBD_STATUS_CAPS_LCK) != 0)) != 0) {
                c = kbd_map2[c];
            } else {
                c = kbd_map1[c];
            }
            tty_update(c); /* Send the char to the tty driver */
        }
        break;
    }
}

/*
 * Initialize the keyboard sybsystem.
 *
 * Registers the interrupt handler.
 */
void kbd_init(void)
{
    isr_register_handler(ISR_KEYBOARD, kbd_handler);
}
