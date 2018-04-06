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

#ifndef _BEEOS_LIBC_TERMIOS_H_
#define _BEEOS_LIBC_TERMIOS_H_

#include <stdint.h>

/*
 * Subscript names for c_cc[] array inside the termios structure.
 */
#define VEOF    0   /**< EOF character */
#define VEOL    1   /**< EOL character */
#define VERASE  2   /**< ERASE character */
#define VINTR   3   /**< INTR character */
#define VKILL   4   /**< KILL character */
#define VMIN    5   /**< MIN value */
#define VQUIT   6   /**< QUIT character */
#define VSTART  7   /**< START character */
#define VSTOP   8   /**< STOP character */
#define VSUSP   9   /**< SUSP character */
#define VTIME   10  /**< TIME value */

/*
 * Input flags, for 'c_iflag' inside the termios structure.
 */
#define BRKINT  0x0001      /**< Signal interrupt or break */
#define ICRNL   0x0002      /**< Map CR to NL on input */
#define IGNBRK  0x0004      /**< Ignore break condition */
#define IGNCR   0x0008      /**< Ignore CR */
#define IGNPAR  0x0010      /**< Ignore characters with parity errors */
#define INLCR   0x0020      /**< Map NL to CR on input */
#define INPCK   0x0040      /**< Enable input parity check */
#define ISTRIP  0x0080      /**< Strip off eight bit */
#define IXOFF   0x0100      /**< Enable start/stop intpu control */
#define IXON    0x0200      /**< Enable start/stop output control */
#define PARMRK  0x0400      /**< Mark parity errors */

/*
 * Local flags, for 'c_lflag' inside the termios structure.
 */
#define ECHO    0x0001      /**< Enable echo */
#define ECHOE   0x0002      /**< Echo erase character as backspace */
#define ECHOK   0x0004      /**< Echo KILL */
#define ECHONL  0x0008      /**< Echo NL */
#define ICANON  0x0010      /**< Canonical input mode */
#define IEXTEN  0x0020      /**< Extended input mode */
#define ISIG    0x0040      /**< Enable signals */
#define NOFLSH  0x0080      /**< Disable flush after interrupt or quit */
#define TOSTOP  0x0100      /**< Send SIGTTOU for background output */


typedef uint16_t tcflag_t;
typedef unsigned char cc_t;

#define NCCS    11
struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
};


int tcgetattr(int fd, struct termios *termptr);
int tcsetattr(int fd, int action, struct termios *termptr);

#endif /* _BEEOS_LIBC_TERMIOS_H_ */
