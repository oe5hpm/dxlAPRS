/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* "@(#)IOc.c Jun 10 18:17:15 2013" */


#define X2C_int32
#define X2C_index32
#ifndef IOc_H_
#include "IOc.h"
#endif
#define IOc_C_
#ifndef printc_H_
#include "printc.h"
#endif


extern void IOc_WrChar(char c)
{
   WrCharc(c);
} /* end WrChar() */


extern void IOc_WrStr(char s[], unsigned long s_len)
{
   unsigned long i;
   i = 0UL;
   while (i<=s_len-1 && s[i]) {
      IOc_WrChar(s[i]);
      ++i;
   }
} /* end WrStr() */


extern void IOc_WrLn(void)
{
   IOc_WrChar('\012');
} /* end WrLn() */


extern void IOc_WrStrLn(char s[], unsigned long s_len)
{
   X2C_PCOPY((void **)&s,s_len);
   IOc_WrStr(s, s_len);
   IOc_WrLn();
   X2C_PFREE(s);
} /* end WrStrLn() */


extern void IOc_WrInt(long x, unsigned long f)
{
   unsigned long j;
   unsigned long i;
   char neg;
   char s[256];
   s[255U] = 0;
   i = 255UL;
   if (x<0L) {
      x = -x;
      neg = 1;
   }
   else neg = 0;
   do {
      --i;
      s[i] = (char)(X2C_MOD(x,10L)+48L);
      x = (long)((unsigned long)x/10UL);
   } while (!(x==0L || i==0UL));
   if (neg && i>0UL) {
      --i;
      s[i] = '-';
   }
   if (f<255UL) f = 255UL-f;
   else f = 1UL;
   while (i>f) {
      --i;
      s[i] = ' ';
   }
   if (i>0UL) {
      j = 0UL;
      while (i<=255UL) {
         s[j] = s[i];
         ++j;
         ++i;
      }
   }
   IOc_WrStr(s, 256ul);
} /* end WrInt() */


extern void IOc_WrCard(unsigned long x, unsigned long f)
{
   unsigned long j;
   unsigned long i;
   char s[256];
   s[255U] = 0;
   i = 255UL;
   do {
      --i;
      s[i] = (char)(x%10UL+48UL);
      x = x/10UL;
   } while (!(x==0UL || i==0UL));
   if (f<255UL) f = 255UL-f;
   else f = 1UL;
   while (i>f) {
      --i;
      s[i] = ' ';
   }
   if (i>0UL) {
      j = 0UL;
      while (i<=255UL) {
         s[j] = s[i];
         ++j;
         ++i;
      }
   }
   IOc_WrStr(s, 256ul);
} /* end WrCard() */


extern void IOc_WrHex(unsigned long x, unsigned long f)
{
   unsigned long j;
   unsigned long i;
   unsigned long n;
   char s[256];
   s[255U] = 0;
   i = 255UL;
   do {
      --i;
      n = x&15UL;
      if (n<=9UL) n += 48UL;
      else n += 55UL;
      s[i] = (char)n;
      x = x/16UL;
   } while (!(x==0UL || i==0UL));
   if (f<255UL) f = 255UL-f;
   else f = 1UL;
   while (i>f) {
      --i;
      s[i] = ' ';
   }
   if (i>0UL) {
      j = 0UL;
      while (i<=255UL) {
         s[j] = s[i];
         ++j;
         ++i;
      }
   }
   IOc_WrStr(s, 256ul);
} /* end WrHex() */


extern void IOc_BEGIN(void)
{
   static int IOc_init = 0;
   if (IOc_init) return;
   IOc_init = 1;
}

