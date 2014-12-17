/* Copyright (c) 1991,96 xTech Ltd, Russia. All Rights Reserved. */

#include "X2C.h"

#if defined(_unix)

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define TTY_LINEMODE_AND_ECHO ( ICANON | ECHO )


static int tty=-1;


/*
PROCEDURE X2C_ttyReadNE(buf: SYSTEM.ADDRESS; bsz: SYSTEM.CARD32; VAR rd: SYSTEM.CARD32): SYSTEM.INT32;
*/

#ifndef X2C_KRC
X2C_INT32 X2C_ttyReadNE(X2C_ADDRESS buf, X2C_CARD32 bsz, X2C_CARD32 * rd)
#else
X2C_INT32 X2C_ttyReadNE(buf, bsz, rd)
  X2C_ADDRESS buf;
  X2C_CARD32  bsz;
  X2C_CARD32  *rd;
#endif
{
  struct termios tty_attr, tty_attr_save;
  if ( tcgetattr(tty,&tty_attr_save) || tcgetattr(tty,&tty_attr) ) return errno;
  tty_attr.c_lflag &= ~ TTY_LINEMODE_AND_ECHO;
  if ( tcsetattr(tty,TCSANOW,&tty_attr) || ( -1 == ((*rd)=read(tty,buf,bsz-1)) ) ) return errno;
  buf[(*rd)]=0;
  return tcsetattr(tty,TCSANOW,&tty_attr_save) == -1 ? errno : 0;
}


/*
PROCEDURE X2C_ttyReadLE(buf: SYSTEM.ADDRESS; bsz: SYSTEM.CARD32; VAR rd: SYSTEM.CARD32): SYSTEM.INT32;
*/

#ifndef X2C_KRC
X2C_INT32 X2C_ttyReadLE(X2C_ADDRESS buf, X2C_CARD32 bsz, X2C_CARD32 * rd)
#else
X2C_INT32 X2C_ttyReadLE(buf, bsz, rd)
  X2C_ADDRESS buf;
  X2C_CARD32  bsz;
  X2C_CARD32  *rd;
#endif
{
  struct termios tty_attr, tty_attr_save;
  if ( tcgetattr(tty,&tty_attr_save) || tcgetattr(tty,&tty_attr) ) return errno;
  tty_attr.c_lflag |= TTY_LINEMODE_AND_ECHO;
  if ( tcsetattr(tty,TCSANOW,&tty_attr) || ( -1 == ((*rd)=read(tty,buf,bsz-1)) ) ) return errno;
  buf[(*rd)]=0;
  return tcsetattr(tty,TCSANOW,&tty_attr_save) == -1 ? errno : 0;
}


/*
PROCEDURE X2C_ttyWrite(buf: SYSTEM.ADDRESS; cc: SYSTEM.CARD32): SYSTEM.INT32;
*/

#ifndef X2C_KRC
X2C_INT32 X2C_ttyWrite(X2C_ADDRESS buf, X2C_CARD32 cc)
#else
X2C_INT32 X2C_ttyWrite(buf, cc)
  X2C_ADDRESS buf;
  X2C_CARD32  cc;
#endif
{
  return ( -1 == write(tty,buf,cc) ) ? errno : 0 ;
}


/*
  PROCEDURE X2C_InitTTY(): SYSTEM.INT32;
*/

#ifndef X2C_KRC
X2C_INT32 X2C_InitTTY(void)
#else
X2C_INT32 X2C_InitTTY()
#endif
{
  return ( -1 == (tty=open("/dev/tty",O_RDWR|O_NOCTTY)) ) ? errno : 0;
}

#else
#if defined(_WIN32) || defined(__NT__)

#include <windows.h>

static HANDLE hInp, hOut;
static int init = 0;

#define inpBits (ENABLE_LINE_INPUT+ENABLE_ECHO_INPUT)

static UINT setCharMode(DWORD * old)
{
        if (GetConsoleMode(hInp,old) && SetConsoleMode(hInp,*old-inpBits)) return 0;
        return GetLastError();
}

static UINT setLineMode(DWORD * old)
{
        if (GetConsoleMode(hInp,old) && SetConsoleMode(hInp,*old+inpBits)) return 0;
        return GetLastError();
}

static UINT restoreMode(DWORD old)
{
        if (SetConsoleMode(hInp,old)) return 0;
        return GetLastError();
}

#define CR '\015'
#define LF '\012'

static void ExpandCR(char *p, X2C_CARD32 *len)
{
        long i,n;

        if (*len==0) {
                if (p[0] == CR) { p[1] = LF; *len = (*len)+1; }
                return;
        }
        n = 0;
        for ( i=0; i<(*len); i++) {
                if (p[i] == CR) n++;
        }
        if (n > 0) {
                i = (*len)-1;
                *len = (*len) + n;
                n = (*len)-1;
                while (i>=0) {
                        if (p[i] == CR) {
                                p[n] = LF;
                                n--;
                        }
                        p[n] = p[i];
                        i--; n--;
                }
                if ( (i != -1) || (n != -1)) {
                        X2C_ASSERT(1);
                }
        }
} /* END ExpandCR */


/*
PROCEDURE X2C_ttyReadNE(buf: SYSTEM.ADDRESS; bsz: SYSTEM.CARD32; VAR rd: SYSTEM.CARD32): SYSTEM.INT32;
*/
X2C_INT32 X2C_ttyReadNE(X2C_ADDRESS buf, X2C_CARD32 bsz, X2C_CARD32 * rd)
{
        DWORD save;
        UINT res;

        res = setCharMode(&save);
        if (res) return res;
        if (ReadFile(hInp,buf,bsz / 2,rd,NULL)) {
                ExpandCR(buf,rd);
                return restoreMode(save);
        }
        res = GetLastError();
        restoreMode(save);
        return res;
}


/*
PROCEDURE X2C_ttyReadLE(buf: SYSTEM.ADDRESS; bsz: SYSTEM.CARD32; VAR rd: SYSTEM.CARD32): SYSTEM.INT32;
*/

X2C_INT32 X2C_ttyReadLE(X2C_ADDRESS buf, X2C_CARD32 bsz, X2C_CARD32 * rd)
{
        DWORD save;
        UINT res;

        res = setLineMode(&save);
        if (res) return res;
        if (ReadFile(hInp,buf,bsz,rd,NULL)) return restoreMode(save);
        res = GetLastError();
        restoreMode(save);
        return res;
}

/*
PROCEDURE X2C_ttyWrite(buf: SYSTEM.ADDRESS; cc: SYSTEM.CARD32): SYSTEM.INT32;
*/
X2C_INT32 X2C_ttyWrite(X2C_ADDRESS buf, X2C_CARD32 cc)
{
        DWORD wr;

        if (WriteFile(hOut,buf,cc,&wr,NULL)) return 0;
        return GetLastError();
}


/*
  PROCEDURE X2C_InitTTY(): SYSTEM.INT32;
*/

X2C_INT32 X2C_InitTTY(void)
{
        if (init) return 0;
        init = 1;

        hInp = CreateFile("CONIN$",GENERIC_READ+GENERIC_WRITE,
                          FILE_SHARE_READ+FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,0);
        if (hInp == INVALID_HANDLE_VALUE) return GetLastError();

        hOut = CreateFileA("CONOUT$",GENERIC_READ+GENERIC_WRITE,
                          FILE_SHARE_READ+FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,0);
        if (hOut == INVALID_HANDLE_VALUE) return GetLastError();
        return 0;
}

#else

/*
PROCEDURE X2C_ttyReadNE(buf: SYSTEM.ADDRESS; bsz: SYSTEM.CARD32; VAR rd: SYSTEM.CARD32): SYSTEM.INT32;
*/
X2C_INT32 X2C_ttyReadNE(X2C_ADDRESS buf, X2C_CARD32 bsz, X2C_CARD32 * rd)
{
        return 1;
}


/*
PROCEDURE X2C_ttyReadLE(buf: SYSTEM.ADDRESS; bsz: SYSTEM.CARD32; VAR rd: SYSTEM.CARD32): SYSTEM.INT32;
*/

X2C_INT32 X2C_ttyReadLE(X2C_ADDRESS buf, X2C_CARD32 bsz, X2C_CARD32 * rd)
{
        return 1;
}


/*
PROCEDURE X2C_ttyWrite(buf: SYSTEM.ADDRESS; cc: SYSTEM.CARD32): SYSTEM.INT32;
*/

X2C_INT32 X2C_ttyWrite(X2C_ADDRESS buf, X2C_CARD32 cc)
{
        return 1;
}


/*
  PROCEDURE X2C_InitTTY(): SYSTEM.INT32;
*/

X2C_INT32 X2C_InitTTY(void)
{
        return 1;
}
#endif /* defined(_WIN32) || defined(__NT__) */
#endif /* defined(_unix) */
