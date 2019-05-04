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
#ifndef tcpb_H_
#include "tcpb.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif







/* rtl_tcp iq fm demodulator by OE5DXL */
#define sdr_IQBUF 65536

#define sdr_DDSMAXLEN 2048

#define sdr_AFCSPEED 400000000
/* slower afc reaction */

#define sdr_AFCRECENTER 1024
/* pull afc to middle */

#define sdr_SSBDDSSIZE 2048

#define sdr_SSBDDSSIZE4 512

#define sdr_AFCRSSICLAMP 0.7
/* until rssi level below peak to do afc */

#define sdr_AFCRSSISPEED 0.9985
/* afc peak rssi fall speed */

struct Complex;


struct Complex {
   float Re;
   float Im;
};

static int32_t fd;

static char isfile;

static uint32_t audiohz;

static uint32_t rtlhz;

static char reconnect;

static uint8_t iqbuf[65536];

static short DDS[2048];

static float DDSR[2048];

static char url[1001];

static char port[11];

static uint32_t sampsum;

static uint32_t sampsize;

static uint32_t reduce;

static uint32_t ddslen;

static uint32_t ddslen4;

static float SSBDDS[2048];


static void initdds(uint32_t size)
{
   uint32_t i;
   float d;
   uint32_t tmp;
   if (size>2047UL) size = 2048UL;
   d = X2C_DIVR(6.2831853071796f,(float)size);
   tmp = size-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      DDSR[i] = 32767.5f*osic_sin((float)i*d);
      DDS[i] = (short)(int32_t)X2C_TRUNCI(DDSR[i],X2C_min_longint,
                X2C_max_longint);
      if (i==tmp) break;
   } /* end for */
   ddslen = (uint32_t)(size-1UL);
   ddslen4 = size/4UL;
} /* end initdds() */


static void initssbdds(float dds[], uint32_t dds_len)
{
   uint32_t i;
   float d;
   uint32_t tmp;
   d = X2C_DIVR(6.2831853071796f,(float)((dds_len-1)+1UL));
   tmp = dds_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      dds[i] = osic_sin((float)i*d);
      if (i==tmp) break;
   } /* end for */
} /* end initssbdds() */

#define sdr_FG (-9)


static void iir512(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>9; /* mixer + lowpass i */
      r2 += r3-r2>>9;
      r3 += r1-r2>>8;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>9; /* mixer + lowpass q */
      i2 += i3-i2>>9;
      i3 += i1-i2>>8;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir512() */

#define sdr_FG0 (-8)


static void iir256(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>8; /* mixer + lowpass i */
      r2 += r3-r2>>8;
      r3 += r1-r2>>7;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>8; /* mixer + lowpass q */
      i2 += i3-i2>>8;
      i3 += i1-i2>>7;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir256() */

#define sdr_FG1 (-7)


static void iir128(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>7; /* mixer + lowpass i */
      r2 += r3-r2>>7;
      r3 += r1-r2>>6;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>7; /* mixer + lowpass q */
      i2 += i3-i2>>7;
      i3 += i1-i2>>6;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir128() */

#define sdr_FG2 (-6)


static void iir64(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>6; /* mixer + lowpass i */
      r2 += r3-r2>>6;
      r3 += r1-r2>>5;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>6; /* mixer + lowpass q */
      i2 += i3-i2>>6;
      i3 += i1-i2>>5;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir64() */

#define sdr_FG3 (-5)


static void iir32(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>5; /* mixer + lowpass i */
      r2 += r3-r2>>5;
      r3 += r1-r2>>4;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>5; /* mixer + lowpass q */
      i2 += i3-i2>>5;
      i3 += i1-i2>>4;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir32() */

#define sdr_FG4 (-4)


static void iir16(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>4; /* mixer + lowpass i */
      r2 += r3-r2>>4;
      r3 += r1-r2>>3;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>4; /* mixer + lowpass q */
      i2 += i3-i2>>4;
      i3 += i1-i2>>3;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir16() */

#define sdr_FG5 (-3)


static void iir8(sdr_pRX rx, uint32_t a, uint32_t b)
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t r3;
   int32_t r2;
   int32_t r1;
   int32_t ddsi;
   int32_t ddsr;
   int32_t xi;
   int32_t xr;
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
   dfc = (uint32_t)((uint32_t)(rx->df+(uint32_t)rx->afckhz)&ddslen);
   i = a;
   do {
      xr = (int32_t)iqbuf[i]-127L;
      ++i;
      xi = (int32_t)iqbuf[i]-127L;
      ++i;
      ddsr = (int32_t)DDS[ph]; /* mix osz sin */
      ddsi = (int32_t)DDS[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>3; /* mixer + lowpass i */
      r2 += r3-r2>>3;
      r3 += r1-r2>>2;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>3; /* mixer + lowpass q */
      i2 += i3-i2>>3;
      i3 += i1-i2>>2;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir8() */

#define sdr_FG6 128


static void iirvar(sdr_pRX rx, uint32_t a, uint32_t b, float bw)
/* variable bandwidth */
{
   uint32_t dfc;
   uint32_t ph;
   uint32_t i;
   float ddsi;
   float ddsr;
   float bw2;
   float i3;
   float i2;
   float i1;
   float r3;
   float r2;
   float r1;
   float xi;
   float xr;
   /*WrFixed(bw, 5,1);WrStrLn(""); */
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->ucr1;
      r2 = anonym->ucr2;
      r3 = anonym->ilr;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->ucr1;
      i2 = anonym0->ucr2;
      i3 = anonym0->ilr;
   }
   ph = rx->phase;
   dfc = rx->df+(uint32_t)rx->afckhz;
   i = a;
   bw2 = bw*2.0f;
   do {
      xr = (float)iqbuf[i]-127.5f;
      ++i;
      xi = (float)iqbuf[i]-127.5f;
      ++i;
      ddsr = DDSR[ph]; /* mix osz sin */
      ddsi = DDSR[(uint32_t)((uint32_t)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (uint32_t)((uint32_t)(ph+dfc)&ddslen); /* drive mix osz */
      r1 = r1+(((xr*ddsr-xi*ddsi)-r1)-r3)*bw; /* mixer + lowpass i */
      r2 = r2+(r3-r2)*bw;
      r3 = r3+(r1-r2)*bw2;
      i1 = i1+(((xr*ddsi+xi*ddsr)-i1)-i3)*bw; /* mixer + lowpass q */
      i2 = i2+(i3-i2)*bw;
      i3 = i3+(i1-i2)*bw2;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->ucr1 = r1;
      anonym1->ucr2 = r2;
      anonym1->ilr = r3;
      anonym1->ucr2 = r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->ucr1 = i1;
      anonym2->ucr2 = i2;
      anonym2->ilr = i3;
      anonym2->ucr2 = i2;
   }
   rx->phase = ph;
} /* end iirvar() */

/*
PROCEDURE genfirtab(VAR t:ARRAY OF REAL; fg:REAL);
VAR i,f,h:CARDINAL;
    f1, e:REAL;
BEGIN
  h:=(HIGH(t)+1) DIV 2; 
  FOR i:=0 TO h-1 DO t[h+i]:=0.5 END;

  f1:=fg*FLOAT(h);
  FOR f:=1 TO TRUNC(f1)+1 DO
    e:=1.0;
    IF f=TRUNC(f1)+1 THEN e:=f1-FLOAT(TRUNC(f1)) END;
    FOR i:=0 TO h-1 DO t[i+h]:=t[i+h]+e*cos(pi*FLOAT(i*f)/FLOAT(h)) END;
  END;
  FOR i:=0 TO h-1 DO
    e:=0.54+0.46*cos(pi*(FLOAT(i)/FLOAT(h)));
    t[i+h]:=t[i+h]*e END;                                         (* hamming window *)
  FOR i:=0 TO h-1 DO t[h-i-1]:=t[i+h] END;
--FOR i:=0 TO HIGH(t) DO WrFixed(t[i], 2,0); WrStr(" ") END; WrStrLn(""); 
END genfirtab;



PROCEDURE fir(rx:pRX; VAR u:Complex);
VAR i,j:CARDINAL;
BEGIN
  WITH rx^ DO
    i:=ifp;
    ifre[i]:=u.Re;
    ifim[i]:=u.Im;
    u.Re:=0.0;
    u.Im:=0.0;
    FOR j:=0 TO HIGH(iftab) DO
      i:=(i+1) MOD (HIGH(iftab)+1);
      u.Re:=u.Re + ifre[i]*iftab[j];
      u.Im:=u.Im + ifim[i]*iftab[j];
    END;
    ifp:=(i+1) MOD (HIGH(iftab)+1);
  END;
END fir;
*/

static void ssbiir(struct sdr_SSBTAP * tap, float fg, float fgq,
                float * u)
{
   struct sdr_SSBTAP * anonym;
   { /* with */
      struct sdr_SSBTAP * anonym = tap;
      anonym->uc1 = (anonym->uc1+(*u-anonym->uc1)*fg)-anonym->il;
      anonym->uc2 = (anonym->uc2+anonym->il)-anonym->il2;
      anonym->uc3 = (anonym->uc3+anonym->il2)-anonym->uc3*fg;
      anonym->il = anonym->il+(anonym->uc1-anonym->uc2)*fgq;
      anonym->il2 = anonym->il2+(anonym->uc2-anonym->uc3)*fgq;
      *u = anonym->uc3;
   }
} /* end ssbiir() */


static void rotvector(struct Complex * v, float rr, float ri)
{
   float t;
   t = v->Re*rr-v->Im*ri;
   v->Im = v->Re*ri+v->Im*rr;
   v->Re = t;
} /* end rotvector() */


static short getsamp(sdr_pRX rx, char notfirst)
{
   struct Complex abs0;
   struct Complex u;
   float af;
   float w;
   float l;
   float lev;
   u.Re = rx->tapre.ucr2;
   u.Im = rx->tapim.ucr2;
   /* fine shift rest of full 1khz */
   if (rx->fine>0UL) {
      rx->fracphase = (uint32_t)((uint32_t)(rx->fracphase+rx->fine)
                &0x7FFUL);
      rotvector(&u, SSBDDS[rx->fracphase],
                SSBDDS[(uint32_t)((uint32_t)(rx->fracphase+512UL)
                &0x7FFUL)]);
   }
   /* fine shift rest of full 1khz */
   if (rx->modulation=='s') {
      /* ssb */
      /* additional IF fir */
      ssbiir(&rx->ssbre, rx->ssbfg, rx->ssbfgq, &u.Re);
      ssbiir(&rx->ssbim, rx->ssbfg, rx->ssbfgq, &u.Im);
      /* additional IF fir */
      /* rssi */
      lev = 1.0f+u.Re*u.Re+u.Im*u.Im;
      l = lev-rx->rssi;
      if (l>=0.0f) l = l*0.1f;
      else l = l*rx->agcspeed;
      rx->rssi = rx->rssi+l;
      /* rssi */
      /* ssb */
      rx->bfophase = (uint32_t)((uint32_t)(rx->bfophase+rx->bfo)&0x7FFUL)
                ;
      af = X2C_DIVR((u.Re*SSBDDS[rx->bfophase]-u.Im*SSBDDS[(uint32_t)
                ((uint32_t)(rx->bfophase+512UL)&0x7FFUL)])*25000.0f,
                osic_sqrt(rx->rssi));
   }
   else if (rx->modulation=='S') {
      /* ssb */
      /* AM FM */
      /* scan squelch */
      lev = u.Re*u.Re+u.Im*u.Im;
      if (notfirst) rx->sqsum = rx->sqsum+(float)fabs(rx->lastlev-lev);
      else {
         rx->sqsum = 0.0f;
         rx->rssi = 0.0f;
      }
      rx->rssi = rx->rssi+lev; /* sum levels */
      rx->lastlev = lev;
      /* scan squelch */
      af = osic_sqrt(lev)*0.01f;
   }
   else {
      /* rssi */
      lev = 1.0f+u.Re*u.Re+u.Im*u.Im;
      l = lev-rx->rssi;
      rx->rssi = rx->rssi+l*0.001f;
      /* rssi */
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
      /* phase highpass make FM */
      af = w-rx->w1;
      rx->w1 = w;
      if (af>3.1415926535898f) af = af-6.2831853071796f;
      if (af<(-3.1415926535898f)) af = af+6.2831853071796f;
      /* phase highpass make FM */
      if (rx->modulation=='a') {
         /* am squelch */
         if (rx->squelch) {
            if (notfirst) rx->sqsum = rx->sqsum+(float)fabs(rx->a1-af);
            rx->a1 = af;
         }
         /* am squelch */
         /* am demod */
         lev = osic_sqrt(lev); /* amplitude */
         af = lev-rx->lastlev; /* - median aplitude */
         rx->lastlev = rx->lastlev+af*0.001f;
         af = (X2C_DIVR(af,rx->lastlev))*32000.0f; /* agc */
      }
      else {
         /* am demod */
         /* fm squelch */
         if (rx->squelch) {
            if (notfirst) {
               rx->sqsum = rx->sqsum+X2C_DIVR((float)fabs(rx->lastlev-lev)
                ,rx->lastlev+lev);
            }
            rx->lastlev = lev;
         }
         /* fm squelch */
         af = af*7.9577471545948E+3f;
      }
   }
   if (af>32000.0f) af = 32000.0f;
   else if (af<(-3.2E+4f)) af = (-3.2E+4f);
   return (short)X2C_TRUNCI(af,-32768,32767);
} /* end getsamp() */

#define sdr_FINESTEP 1024


extern int32_t sdr_getsdr(uint32_t samps, sdr_pRX rx[],
                uint32_t rx_len)
{
   uint32_t wssb;
   uint32_t ws;
   uint32_t bs;
   uint32_t as;
   uint32_t b;
   uint32_t a;
   uint32_t r;
   uint32_t s;
   int32_t u;
   struct sdr_RX * anonym;
   struct sdr_RX * anonym0;
   uint32_t tmp;
   if (reconnect && fd<0L) {
      usleep(1000000UL);
      if (isfile) fd = osi_OpenRead(url, 1001ul);
      else fd = connecttob(url, port);
   }
   if (fd>=0L) {
      sampsum = sampsum&1023UL; /* partial sample reminder of last block */
      if (samps*(sampsize+1UL)>32768UL) samps = 32768UL/(sampsize+1UL);
      if (isfile) {
         if (osi_RdBin(fd, (char *)iqbuf, 65536u/1u,
                ((samps*reduce+sampsum)/1024UL)*2UL)<=0L) {
            osic_Close(fd);
            fd = -1L;
            return -1L;
         }
      }
      else if (readsockb(fd, (char *)iqbuf,
                (int32_t)(((samps*reduce+sampsum)/1024UL)*2UL))<0L) {
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
               ws = anonym->width;
               if (anonym->modulation=='s') {
                  wssb = (anonym->width*5UL)/4UL;
                  ws = 3000UL;
                  while (ws<wssb) ws = ws*2UL;
                  /*WrInt(ws, 0);WrStrLn(" ws"); */
                  anonym->ssbfg = X2C_DIVR((float)anonym->width,
                (float)audiohz);
                  anonym->ssbfgq = anonym->ssbfg*anonym->ssbfg*2.0f;
                  anonym->bfo = ((uint32_t)labs(anonym->maxafc)*2048UL)
                /audiohz;
                  if (anonym->maxafc>0L) anonym->bfo = 2048UL-anonym->bfo;
                  /*WrInt(bfo, 0);WrStrLn(" bfo"); */
                  anonym->fine = (anonym->dffrac*2048UL)/audiohz;
                  if (anonym->agc>0UL) {
                     anonym->agcspeed = X2C_DIVR(0.2f,(float)anonym->agc);
                  }
                  else anonym->agcspeed = 0.00025f;
               }
               else {
                  anonym->fine = 0UL;
                  anonym->agcspeed = 0.001f;
               }
               if (rtlhz<2048000UL) ws = ws*2UL;
               if (ws==3000UL) iir512(rx[r], as, bs);
               else if (ws==6000UL) iir256(rx[r], as, bs);
               else if (ws==12000UL) iir128(rx[r], as, bs);
               else if (ws==24000UL) iir64(rx[r], as, bs);
               else if (ws==48000UL) iir32(rx[r], as, bs);
               else if (ws==96000UL) iir16(rx[r], as, bs);
               else if (ws==192000UL) iir8(rx[r], as, bs);
               else iirvar(rx[r], as, bs, (float)ws*6.7934782608696E-7f);
               u = (int32_t)getsamp(rx[r], s>0UL);
               anonym->samples[s] = (short)u;
               if (anonym->afcrun) anonym->median = anonym->median+u;
            }
            ++r;
         }
         a = b;
         if (s==tmp) break;
      } /* end for */
      sampsum = a;
      /*AFC */
      r = 0UL;
      while (rx[r]) {
         { /* with */
            struct sdr_RX * anonym0 = rx[r];
            anonym0->afcrun = 0;
            if (anonym0->modulation=='f' && anonym0->maxafc>0L) {
               if (anonym0->afcrssi<anonym0->rssi) {
                  anonym0->afcrssi = anonym0->rssi; /* peak level rssi */
               }
               else anonym0->afcrssi = anonym0->afcrssi*0.9985f;
               if (anonym0->afcrssi*0.7f<anonym0->rssi) {
                  /* near peak rssi so let afc work */
                  anonym0->median = anonym0->median-(anonym0->afckhz*(int32_t)
                samps*1024L)/anonym0->maxafc; /* weak pull to middle */
                  if (anonym0->median>400000000L) {
                     if (anonym0->afckhz<anonym0->maxafc) ++anonym0->afckhz;
                     anonym0->median = 0L;
                  }
                  else if (anonym0->median<-400000000L) {
                     if (anonym0->afckhz>-anonym0->maxafc) --anonym0->afckhz;
                     anonym0->median = 0L;
                  }
                  anonym0->afcrun = 1;
               }
            }
         }
         /* low rssi so freeze afc */
         /*WrInt(ORD(afcrun),1); */
         ++r;
      }
      /*AFC */
      return (int32_t)samps;
   }
   else return -1L;
   return 0;
} /* end getsdr() */


extern void sdr_setparm(uint32_t num, uint32_t value)
{
   int32_t res;
   char tbuf[5];
   tbuf[0U] = (char)num;
   tbuf[1U] = (char)(value/16777216UL);
   tbuf[2U] = (char)(value/65536UL&255UL);
   tbuf[3U] = (char)(value/256UL&255UL);
   tbuf[4U] = (char)(value&255UL);
   res = sendsock(fd, tbuf, 5L);
} /* end setparm() */


extern char sdr_startsdr(char ip[], uint32_t ip_len,
                char tport[], uint32_t tport_len, uint32_t inhz,
                uint32_t outhz, char reconn)
{
   aprsstr_Assign(url, 1001ul, ip, ip_len);
   aprsstr_Assign(port, 11ul, tport, tport_len);
   reconnect = reconn;
   if (inhz>0UL) rtlhz = inhz;
   if (rtlhz!=1024000UL && (rtlhz<2048000UL || rtlhz>2500000UL)) return 0;
   if (outhz>0UL) audiohz = outhz;
   reduce = (1024UL*rtlhz+audiohz/2UL)/audiohz; /* sample reduction * 1024 */
   sampsize = reduce/1024UL; /* input samples per output sample, trunc */
   isfile = port[0U]=='0' && port[1U]==0;
   if (fd<0L) {
      if (isfile) fd = osi_OpenRead(url, 1001ul);
      else fd = connecttob(url, port);
   }
   if (fd>=0L) {
      if (!isfile) sdr_setparm(2UL, rtlhz);
      if (inhz>=2048000UL) initdds(2048UL);
      else initdds(1024UL);
      initssbdds(SSBDDS, 2048ul);
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

