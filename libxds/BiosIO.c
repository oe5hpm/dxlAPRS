/* Copyright (C) 2000 Excelsior. */

#include "X2C.h"
#include "BiosIO.h"

#undef UNIX_BIOSIO

#if (defined(_MSC_VER) && defined(_WIN32))
#include "conio.h"
#define GETCH _getch
#define GETCHE _getche
#define KBHIT _kbhit

#elif defined(__WATCOMC__)
#include "conio.h"
#define GETCH getch
#define GETCHE getche
#define KBHIT kbhit

#elif defined(__IBMC__)
#include "conio.h"
#define GETCH _getch
#define GETCHE _getche
#define KBHIT _kbhit

#elif defined(_unix)
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/poll.h>
#define UNIX_BIOSIO
#endif

X2C_CHAR     BiosIO_LastChar = 0xff;
X2C_CARD8    BiosIO_LastScan = 0;
X2C_BOOLEAN  BiosIO_EnhancedBIOS = 1;

#ifndef UNIX_BIOSIO
X2C_BOOLEAN BiosIO_KeyPressed(void) {
    return KBHIT();
}

X2C_CHAR BiosIO_RdKey(void) {
    return GETCH();
}

X2C_CHAR BiosIO_RdChar(void) {
    return GETCHE();
}
void BiosIO_BEGIN(void) { /* !!! */
}
#else
struct termios termios_save;
struct pollfd fd[1];
int stdoutn, stdinn;

X2C_BOOLEAN BiosIO_KeyPressed(void) {
    if(poll(fd, 1, 0)>0) {
        return 1; // true
    }
    return 0; // false
}
X2C_CHAR BiosIO_RdKey(void) {
/*
    int lastChar;
    // wait until some key is pressed
    lastChar = getchar();
    if(lastChar==0x1b) {
        if(poll(fd, 1, 0)>0) {
	    lastChar = getchar();
        }
    }
    BiosIO_LastChar = (X2C_CHAR)lastChar;
    BiosIO_LastScan = (X2C_CARD8)BiosIO_LastChar;
*/
    read(stdinn, &BiosIO_LastChar, 1);
    BiosIO_LastScan = (X2C_CARD8)BiosIO_LastChar;
    return BiosIO_LastChar;
}
X2C_CHAR BiosIO_RdChar(void) {
    putc(BiosIO_RdKey(), stdout);
    return BiosIO_LastChar;
}
static void BiosIO_FINALLY(void) {
    tcsetattr(stdoutn, TCSANOW, &termios_save);
}
void BiosIO_BEGIN(void) {
    struct termios buf;
    stdoutn = fileno(stdout);
    stdinn = fileno(stdin);
    
    X2C_FINALLY(BiosIO_FINALLY);

    tcgetattr(stdoutn, &termios_save);
    buf = termios_save;
    buf.c_iflag = 0;
    buf.c_oflag = OPOST|ONLCR;
    buf.c_cflag = CREAD;
    buf.c_lflag = TOSTOP|CS8;
    tcsetattr(stdoutn, TCSANOW, &buf);
    
    fd[0].fd = stdinn;
    fd[0].events = POLLIN;
}
#endif

BiosIO_KBFlagSet KBFlags (void) {
    return 0;
}
