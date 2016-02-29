/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef sdr_H_
#include "sdr.h"
#endif
#define sdr_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef tcpb_H_
#include "tcpb.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif





/* rtl_tcp iq fm demodulator by OE5DXL */
/*FROM Select IMPORT Usleep; */
#define sdr_IQBUF 65536

#define sdr_DDSMAXLEN 2048

#define sdr_AFCSPEED 400000000
/* slower afc reaction */

#define sdr_AFCRECENTER 1024
/* pull afc to middle */

struct Complex;


struct Complex {
   float Re;
   float Im;
};

static long fd;

static unsigned long audiohz;

static unsigned long rtlhz;

static char reconnect;

static unsigned char iqbuf[65536];

static short DDS[2048];

static char url[1001];

static char port[11];

static unsigned long sampsum;

static unsigned long sampsize;

static unsigned long reduce;

static unsigned long ddslen;

static unsigned long ddslen4;


static void initdds(unsigned long size)
{
   unsigned long i;
   float d;
   unsigned long tmp;
   if (size>2047UL) size = 2048UL;
   d = X2C_DIVR(6.2831853071796f,(float)size);
   tmp = size-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      DDS[i] = (short)(long)X2C_TRUNCI(32767.5f*RealMath_sin((float)i*d),
                X2C_min_longint,X2C_max_longint);
      if (i==tmp) break;
   } /* end for */
   ddslen = (unsigned long)(size-1UL);
   ddslen4 = size/4UL;
} /* end initdds() */

#define sdr_FG 128


static void iir128(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += (((xr*ddsr-xi*ddsi)-r1)-r3)/128L; /* mixer + lowpass i */
      r2 += (r3-r2)/128L;
      r3 += (r1-r2)/64L;
      i1 += (((xr*ddsi+xi*ddsr)-i1)-i3)/128L; /* mixer + lowpass q */
      i2 += (i3-i2)/128L;
      i3 += (i1-i2)/64L;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
   }
   rx->phase = ph;
} /* end iir128() */

#define sdr_FG0 64


static void iir64(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += (((xr*ddsr-xi*ddsi)-r1)-r3)/64L; /* mixer + lowpass i */
      r2 += (r3-r2)/64L;
      r3 += (r1-r2)/32L;
      i1 += (((xr*ddsi+xi*ddsr)-i1)-i3)/64L; /* mixer + lowpass q */
      i2 += (i3-i2)/64L;
      i3 += (i1-i2)/32L;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
   }
   rx->phase = ph;
} /* end iir64() */

#define sdr_FG1 32


static void iir32(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += (((xr*ddsr-xi*ddsi)-r1)-r3)/32L; /* mixer + lowpass i */
      r2 += (r3-r2)/32L;
      r3 += (r1-r2)/16L;
      i1 += (((xr*ddsi+xi*ddsr)-i1)-i3)/32L; /* mixer + lowpass q */
      i2 += (i3-i2)/32L;
      i3 += (i1-i2)/16L;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
   }
   rx->phase = ph;
} /* end iir32() */

#define sdr_FG2 16


static void iir16(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += (((xr*ddsr-xi*ddsi)-r1)-r3)/16L; /* mixer + lowpass i */
      r2 += (r3-r2)/16L;
      r3 += (r1-r2)/8L;
      i1 += (((xr*ddsi+xi*ddsr)-i1)-i3)/16L; /* mixer + lowpass q */
      i2 += (i3-i2)/16L;
      i3 += (i1-i2)/8L;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
   }
   rx->phase = ph;
} /* end iir16() */

#define sdr_FG3 8


static void iir8(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = (unsigned long)((unsigned long)(rx->df+(unsigned long)rx->afckhz)
                &ddslen);
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += (((xr*ddsr-xi*ddsi)-r1)-r3)/8L; /* mixer + lowpass i */
      r2 += (r3-r2)/8L;
      r3 += (r1-r2)/4L;
      i1 += (((xr*ddsi+xi*ddsr)-i1)-i3)/8L; /* mixer + lowpass q */
      i2 += (i3-i2)/8L;
      i3 += (i1-i2)/4L;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
   }
   rx->phase = ph;
} /* end iir8() */


static short getsamp(sdr_pRX rx)
{
   struct Complex abs0;
   struct Complex u;
   float af;
   float w;
   float l;
   float lev;
   u.Re = (float)rx->tapre.uc2;
   u.Im = (float)rx->tapim.uc2;
   /* complex to phase */
   abs0.Re = (float)fabs(u.Re);
   abs0.Im = (float)fabs(u.Im);
   if (abs0.Im>abs0.Re) {
      if (abs0.Im>0.0f) w = X2C_DIVR(abs0.Re,abs0.Im);
      else w = 0.0f;
      w = 1.5707963267949f-(w*1.055f-w*w*0.267f); /* arctan */
   }
   else {
      if (abs0.Re>0.0f) w = X2C_DIVR(abs0.Im,abs0.Re);
      else w = 0.0f;
      w = w*1.055f-w*w*0.267f;
   }
   if (u.Re<0.0f) w = 3.1415926535898f-w;
   if (u.Im<0.0f) w = -w;
   /* complex to phase */
   /* rssi */
   lev = 1.0f+abs0.Re*abs0.Re+abs0.Im*abs0.Im;
   rx->rssi = rx->rssi+(lev-rx->rssi)*0.001f;
   /* rssi */
   /* squelch */
   if (rx->squelch) {
      l = X2C_DIVR((float)fabs(rx->lastlev-lev),rx->lastlev+lev);
      rx->sqmed = rx->sqmed+(l-rx->sqmed)*0.002f;
      rx->lastlev = lev;
   }
   /* squelch */
   /* phase highpass make FM */
   af = w-rx->w1;
   rx->w1 = w;
   if (af>3.1415926535898f) af = af-6.2831853071796f;
   if (af<(-3.1415926535898f)) af = af+6.2831853071796f;
   return (short)X2C_TRUNCI(af*7.9577471545948E+3f,-32768,32767);
} /* end getsamp() */

#define sdr_FINESTEP 1024


extern long sdr_getsdr(unsigned long samps, sdr_pRX rx[],
                unsigned long rx_len)
{
   unsigned long bs;
   unsigned long as;
   unsigned long b;
   unsigned long a;
   unsigned long r;
   unsigned long s;
   long u;
   struct sdr_RX * anonym;
   unsigned long tmp;
   if (reconnect && fd<0L) {
      /*    Usleep(1000000); */
      fd = connecttob(url, port);
   }
   if (fd>=0L) {
      sampsum = sampsum&1023UL; /* partial sample reminder of last block */
      if (samps*(sampsize+1UL)>32768UL) samps = 32768UL/(sampsize+1UL);
      if (readsockb(fd, (char *)iqbuf,
                (long)(((samps*reduce+sampsum)/1024UL)*2UL))<0L) {
         /* connect lost */
         osic_Close(fd);
         fd = -1L;
         return -1L;
      }
      a = sampsum; /* continue from last partial step */
      tmp = samps-1UL;
      s = 0UL;
      if (s<=tmp) for (;; s++) {
         r = 0UL;
         b = a+reduce;
         as = (a/1024UL)*2UL;
         bs = (b/1024UL)*2UL-2UL;
         while (rx[r]) {
            { /* with */
               struct sdr_RX * anonym = rx[r];
               if (anonym->width==128UL) iir128(rx[r], as, bs);
               else if (anonym->width==64UL) iir64(rx[r], as, bs);
               else if (anonym->width==32UL) iir32(rx[r], as, bs);
               else if (anonym->width==16UL) iir16(rx[r], as, bs);
               else if (anonym->width==8UL) iir8(rx[r], as, bs);
               else return -2L;
               u = (long)getsamp(rx[r]);
               anonym->samples[s] = (short)u;
               /* AFC */
               if (anonym->maxafc>0UL) {
                  anonym->median = (anonym->median+u)-(anonym->afckhz*1024L)
                /(long)anonym->maxafc;
                  if (anonym->median>400000000L) {
                     if (anonym->afckhz<(long)anonym->maxafc) {
                        ++anonym->afckhz;
                     }
                     anonym->median = 0L;
                  }
                  else if (anonym->median<-400000000L) {
                     if (anonym->afckhz>-(long)anonym->maxafc) {
                        --anonym->afckhz;
                     }
                     anonym->median = 0L;
                  }
               }
            }
            /* AFC */
            ++r;
         }
         a = b;
         if (s==tmp) break;
      } /* end for */
      sampsum = a;
      return (long)samps;
   }
   else return -1L;
   return 0;
} /* end getsdr() */


extern void sdr_setparm(unsigned long num, unsigned long value)
{
   long res;
   char tbuf[5];
   tbuf[0U] = (char)num;
   tbuf[1U] = (char)(value/16777216UL);
   tbuf[2U] = (char)(value/65536UL&255UL);
   tbuf[3U] = (char)(value/256UL&255UL);
   tbuf[4U] = (char)(value&255UL);
   res = sendsock(fd, tbuf, 5L);
} /* end setparm() */


extern char sdr_startsdr(char ip[], unsigned long ip_len, char tport[],
                unsigned long tport_len, unsigned long inhz,
                unsigned long outhz, char reconn)
{
   aprsstr_Assign(url, 1001ul, ip, ip_len);
   aprsstr_Assign(port, 11ul, tport, tport_len);
   reconnect = reconn;
   if (inhz>0UL) rtlhz = inhz;
   if (rtlhz!=1024000UL && rtlhz!=2048000UL) return 0;
   if (outhz>0UL) audiohz = outhz;
   reduce = (1024UL*rtlhz+audiohz/2UL)/audiohz; /* sample reduction * 1024 */
   sampsize = reduce/1024UL; /* input samples per output sample, trunc */
   if (fd<0L) fd = connecttob(url, port);
   if (fd>=0L) {
      sdr_setparm(2UL, rtlhz);
      initdds(inhz/1000UL);
   }
   return fd>=0L;
} /* end startsdr() */


extern void sdr_BEGIN(void)
{
   static int sdr_init = 0;
   if (sdr_init) return;
   sdr_init = 1;
   if (sizeof(sdr_AUDIOSAMPLE)!=131072) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   fd = -1L;
   reconnect = 0;
   rtlhz = 2048000UL;
   audiohz = 16000UL;
}

