/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef osi_H_
#include "osi.h"
#endif
#define osi_C_
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif

/* os interface linux/win32 */


static void hFd(long fd,unsigned long n)
{
   char tmp;
   if (n<10UL) osic_WrBin(fd,(char *)(tmp = (char)(n+48UL),&tmp), 1ul,1ul);
   else osic_WrBin(fd,(char *)(tmp = (char)(n+55UL),&tmp), 1ul,1ul);
} /* end h() */


extern void osi_WrHexFd(long fd,unsigned long n, unsigned long f)
{
   hFd(fd,n/16UL&15UL);
   hFd(fd,n&15UL);
   while (f>=3UL) {
      osic_WrBin(fd," ", 2ul,1ul);
      --f;
   }
} /* end WrHex() */


static void h(unsigned long n)
{
   char tmp;
   if (n<10UL) osi_WrStr((char *)(tmp = (char)(n+48UL),&tmp), 1ul);
   else osi_WrStr((char *)(tmp = (char)(n+55UL),&tmp), 1ul);
} /* end h() */


extern void osi_WrHex(unsigned long n, unsigned long f)
{
   h(n/16UL&15UL);
   h(n&15UL);
   while (f>=3UL) {
      osi_WrStr(" ", 2ul);
      --f;
   }
} /* end WrHex() */


extern long osi_realint(float x)
{
   if (x>=2.E+9f) return 2000000000L;
   if (x<=(-2.E+9f)) return -2000000000L;
   return (long)X2C_TRUNCI(x,X2C_min_longint,X2C_max_longint);
} /* end realint() */


extern unsigned long osi_realcard(float x)
{
   if (x>=4.E+9f) return 0x0EE6B2800UL;
   if (x<=0.0f) return 0UL;
   return (unsigned long)X2C_TRUNCC(x,0UL,X2C_max_longcard);
} /* end realcard() */


extern long osi_OpenAppendLong(char fn[], unsigned long fn_len)
{
   return osic_OpenAppendLong(fn, fn_len);
} /* end OpenAppendLong() */


extern long osi_OpenAppend(char fn[], unsigned long fn_len)
{
   return osic_OpenAppend(fn, fn_len);
} /* end OpenAppend() */


extern long osi_OpenWrite(char fn[], unsigned long fn_len)
{
   return osic_OpenWrite(fn, fn_len);
} /* end OpenWrite() */


extern long osi_OpenReadLong(char fn[], unsigned long fn_len)
{
   return osic_OpenReadLong(fn, fn_len);
} /* end OpenReadLong() */


extern long osi_OpenRead(char fn[], unsigned long fn_len)
{
   return osic_OpenRead(fn, fn_len);
} /* end OpenRead() */


extern long osi_OpenRW(char fn[], unsigned long fn_len)
{
   return osic_OpenRW(fn, fn_len);
} /* end OpenRW() */


extern long osi_OpenNONBLOCK(char fn[], unsigned long fn_len)
{
   return osic_OpenNONBLOCK(fn, fn_len);
} /* end OpenNONBLOCK() */


extern void osi_NextArg(char s[], unsigned long s_len)
{
   osic_NextArg(s, s_len);
} /* end NextArg() */


extern long osi_RdBin(long fd, char buf[], unsigned long buf_len,
                unsigned long size)
{
   return osic_RdBin(fd, buf, buf_len, size);
} /* end RdBin() */


extern void osi_WrBin(long fd, char buf[], unsigned long buf_len,
                unsigned long size)
{
   osic_WrBin(fd, buf, buf_len, size);
} /* end WrBin() */


extern void osi_Rename(char fname[], unsigned long fname_len, char newname[],
                 unsigned long newname_len)
{
   osic_Rename(fname, fname_len, newname, newname_len);
} /* end Rename() */


extern void osi_Werr(char text[], unsigned long text_len)
{
   osic_WerrStr(text, text_len);
} /* end Werr() */


extern void osi_WerrLn(char text[], unsigned long text_len)
{
   osic_WerrStrLn(text, text_len);
} /* end WerrLn() */


extern void osi_WrStrLn(char s[], unsigned long s_len)
{
   osic_WrStrLn(s, s_len);
} /* end WrStrLn() */


extern void osi_WrStr(char s[], unsigned long s_len)
{
   osic_WrStr(s, s_len);
} /* end WrStr() */


extern void osi_Erase(char fn[], unsigned long fn_len, char * done)
{
   osic_Remove(fn, fn_len, done);
} /* end Erase() */


extern char osi_Exists(char fn[], unsigned long fn_len)
{
   return osic_Exists(fn, fn_len);
} /* end Exists() */


extern long osi_getptsname(long fd, X2C_ADDRESS s, unsigned long size)
{
   return osic_getptsname(fd, s, size);
} /* end getptsname() */

typedef char * pSTR;


extern long osi_symblink(X2C_ADDRESS fname, X2C_ADDRESS newname)
{
   return osic_symblink((pSTR)fname, (pSTR)newname);
} /* end symblink() */


extern void osi_BEGIN(void)
{
   static int osi_init = 0;
   if (osi_init) return;
   osi_init = 1;
}

