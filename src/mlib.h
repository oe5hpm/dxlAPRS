/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef mlib_H_
#define mlib_H_
#include "X2C.h"

typedef unsigned char cc_t;

typedef unsigned long speed_t;

typedef unsigned long tcflag_t;

struct termios;


struct termios {
   unsigned long c_iflag;
   unsigned long c_oflag;
   unsigned long c_cflag;
   unsigned long c_lflag;
   unsigned char c_line;
   unsigned char c_cc[32];
   unsigned long c_ispeed;
   unsigned long c_ospeed;
};

typedef char * tExecArg[256];

#define NCCS 19

#define VINTR 0

#define VQUIT 1

#define VERASE 2

#define VKILL 3

#define VEOF 4

#define VTIME 5

#define VMIN 6

#define VSTART 8

#define VSTOP 9

#define VSUSP 10

#define VEOL 11

#define IGNLCR 384

#define B0 0

#define B50 1

#define B75 2

#define B110 3

#define B134 4

#define B150 5

#define B200 6

#define B300 7

#define B600 8

#define B1200 9

#define B1800 10

#define B2400 11

#define B4800 12

#define B9600 13

#define B19200 14

#define B38400 15

#define B57600 4097

#define B115200 4098

#define B230400 4099

#define B460800 4100

#define B500000 4101

#define B576000 4102

#define B921600 4103

#define B1000000 4104

#define B1152000 4105

#define B1500000 4106

#define B2000000 4107

#define B2500000 4108

#define B3000000 4109

#define B3500000 4110

#define B4000000 4111

#define CSIZE 48

#define CS5 0

#define CS6 16

#define CS7 32

#define CS8 48

#define CSTOPB 64

#define CREAD 128

#define PARENB 256

#define PARODD 512

#define HUPCL 1024

#define CLOCAL 2048

#define ISIG 1

#define ICANON 2

#define ECHO 8

#define ECHOE 16

#define ECHOK 32

#define ECHONL 64

#define NOFLASH 128

#define TOSTOP 256

#define IEXTEN 32768

#define TIOCNOTTY 0x5422 

#define TIOCSCTTY 0x540E 

#define IGNBRK 1

#define BRKINT 2

#define IGNPAR 4

#define PARMRK 8

#define INPCK 16

#define ISTRIP 32

#define INLCR 64

#define IGNCR 128

#define ICRNL 256

#define IUCLC 512

#define IXON 1024

#define IXANY 2048

#define IXOFF 4096

#define IMAXBEL 8192

#define OPOST 1

#define OLCUC 2

#define ONLCR 4

#define OCRNL 8

#define ONOCR 16

#define ONLRET 32

#define OFILL 64

#define OFDEL 128

#define NLDLY 256

#define NL0 0

#define NL1 256

#define CRDLY 1536

#define CR0 0

#define CR1 512

#define CR2 1024

#define CR3 1536

#define TABDLY 6144

#define TAB0 0

#define TAB1 2048

#define TAB2 4096

#define TAB3 6144

#define XTABS 6144

#define BSDLY 8192

#define BS0 0

#define BS1 8192

#define VTDLY 16384

#define VT0 0

#define VT1 16384

#define FFDLY 32768

#define FF0 0

#define FF1 32768

#define CRTSCTS 0x080000000

#define TCOOFF 0

#define TCOON 1

#define TCIOFF 2

#define TCION 3

#define TCIFLUSH 0

#define TCOFLUSH 1

#define TCIOFLUSH 2

#define TCSANOW 0

#define TCSADRAIN 1

#define TCSAFLUSH 2

#define TIOCMBIS 0x5416 

#define TIOCMBIC 0x5417 

#define TIOCMSET 0x5418 

#define TIOCMGET 0x5415 

#define TIOCMIWAIT 0x545C 

#define TIOCM_LE 0x1 

#define TIOCM_DTR 0x2 

#define TIOCM_RTS 0x4 

#define TIOCM_ST 0x8 

#define TIOCM_SR 0x10 

#define TIOCM_CTS 0x20 

#define TIOCM_CAR 0x40 

#define TIOCM_RNG 0x80 

#define TIOCM_DSR 0x100 

#define TIOCM_CD 64

#define TIOCM_RI 128

#define TIOCM_OUT1 0x2000 

#define TIOCM_OUT2 0x4000 

extern unsigned long cfgetispeed(struct termios *);

extern unsigned long cfgetospeed(struct termios *);

extern long cfsetispeed(struct termios *, unsigned long);

extern long cfsetospeed(struct termios *, unsigned long);

extern long tcdrain(long);

extern long tcflow(long, long);

extern long tcflush(long, long);

extern long tcgetattr(long, struct termios *);

extern long tcdsendbreak(long, long);

extern long tcsetattr(long, long, struct termios *);

extern long ioctl(long, long, X2C_ADDRESS);


#endif /* mlib_H_ */
