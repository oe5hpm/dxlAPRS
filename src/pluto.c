/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define pluto_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#include <signal.h>
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef plutoio_H_
#include "plutoio.h"
#endif

/* plutosdr test */
#define pluto_SAMPLES 16384

#define pluto_OSSSAMP 48000

#define pluto_LEVEL 2.925625E+4

#define pluto_RADIOPREEM 1.2

struct LPCONTEXT24;


struct LPCONTEXT24 {
   float uc1;
   float uc2;
   float il;
   float LPR;
   float OLPR;
   float LPL;
};

struct LPCONTEXT12;


struct LPCONTEXT12 {
   float uc;
   float il;
   float K1;
   float K2;
};

enum MODS {pluto_mFM, pluto_mUSB, pluto_mLSB, pluto_mAM, pluto_mSTEREO};


struct STEREOCONTEXT;


struct STEREOCONTEXT {
   float lastl;
   float lastr;
   float wpilot;
};

static pIQBUF buf;

static int32_t adc;

static int32_t i;

static int32_t len;

static int32_t tmpi;

static int32_t bi;

static int32_t bq;

static int32_t si;

static int32_t sq;

static int32_t wshift;

static int32_t shift;

static uint32_t p;

static uint32_t adcrate;

static uint32_t sample;

static uint32_t upsamp;

static uint32_t is;

static float asamp;

static float limmul;

static float limlev;

static float ohp;

static float ohd;

static float levmul;

static float wsub;

static float ssbwinc;

static float bbi;

static float bbq;

static float fmdeviation;

static float pk;

static struct LPCONTEXT12 upsamplelpi;

static struct LPCONTEXT12 upsamplelpq;

static struct LPCONTEXT24 miclp;

static struct LPCONTEXT24 ssblpi;

static struct LPCONTEXT24 ssblpq;

static char exit0;

static short abuf[512];

static uint32_t pabuf;

static uint32_t abuflen;

static uint8_t mod;

static struct STEREOCONTEXT cstereo;

static short sintab[32768];


static void OpenSound(void)
{
   int32_t i0;
   adc = osi_OpenNONBLOCK("/dev/dsp1", 10ul);
   if (adc>=0L) {
      i0 = samplesize(adc, 16UL); /* 8, 16 */
      i0 = channels(adc, 1UL+(uint32_t)(mod==pluto_mSTEREO)); /* 1, 2  */
      i0 = sampelrate(adc, adcrate); /* 8000..48000 */
   }
   else {
      osi_WrStrLn("/dev/dsp open error", 20ul);
      X2C_ABORT();
   }
} /* end OpenSound() */


static float audiosamp(void)
{
   int32_t i0;
   if (pabuf>=abuflen) {
      i0 = osi_RdBin(adc, (char *)abuf, 1024u/1u, 1024UL);
      if (i0<=0L) return 0.0f;
      abuflen = (uint32_t)(i0/2L);
      pabuf = 0UL;
   }
   i0 = (int32_t)abuf[pabuf];
   ++pabuf;
   return (float)i0;
} /* end audiosamp() */


static float lp(float in, struct LPCONTEXT24 * c)
/* lowpass 24db/oct 6dB loss */
{
   struct LPCONTEXT24 * anonym;
   { /* with */
      struct LPCONTEXT24 * anonym = c;
      anonym->uc1 = (anonym->uc1+(in-anonym->uc1)*anonym->LPR)-anonym->il;
      anonym->uc2 = anonym->uc2*anonym->OLPR+anonym->il;
      anonym->il = anonym->il+(anonym->uc1-anonym->uc2)*anonym->LPL;
      return anonym->uc2;
   }
} /* end lp() */


static float audioproc(float in)
/* lowpass, limiter, clipper ...  */
{
   float ll;
   float o;
   o = lp(in*levmul, &miclp); /* lowpass */
   ohd = ohd+(o-ohd)*0.05f; /* remove bass */
   o = o-ohd;
   ll = ohp; /* preemphase */
   ohp = o;
   o = o*1.1f-ll;
   o = o*(1.0f-limmul);
   ll = (float)fabs(o)-limlev;
   if (ll>0.0f) {
      /* too loud */
      limmul = limmul+(1.0f-limmul)*ll*0.00002f;
      if (limmul>1.0f) limmul = 1.0f;
   }
   else limmul = limmul*0.99999f;
   /*
     o:=o+LEVEL*0.1*sin(wtone);
     wtone:=wtone+100.0*(2.0*pi/FLOAT(OSSSAMP));
     IF wtone>2.0*pi THEN wtone:=wtone-2.0*pi END;
   */
   if (o>2.925625E+4f) o = 2.925625E+4f;
   else if (o<(-2.925625E+4f)) o = (-2.925625E+4f);
   return o;
} /* end audioproc() */


static void ssbtx(float u, float * i0, float * q)
{
   float v;
   u = u*4.0f; /* lp has 6db loss, mixer 50% for each sideband */
   *i0 = lp(u*osic_sin(wsub), &ssblpi);
   *q = lp(u*osic_cos(wsub), &ssblpq);
   v =  *i0* *i0+ *q* *q;
   if (v>8.559281640625E+8f) {
      /* rf clipper */
      v = X2C_DIVR(2.925625E+4f,osic_sqrt(v));
      *i0 =  *i0*v;
      *q =  *q*v;
   }
   wsub = wsub+ssbwinc;
   if (wsub>6.2831853071796f) wsub = wsub-6.2831853071796f;
} /* end ssbtx() */

/*
PROCEDURE fmtx(u:REAL; VAR i, q:REAL);
BEGIN
  i:=LEVEL*sin(wsub);
  q:=LEVEL*cos(wsub);
  wsub:=wsub + u*fmdeviation;
  IF wsub>2.0*pi THEN wsub:=wsub-2.0*pi ELSIF wsub<0.0 THEN wsub:=wsub+2.0*pi END;
END fmtx;
*/

static float preem(float u, float * last)
{
   float o;
   o = *last;
   *last = u;
   o = u+(u-o)*1.2f;
   if (o>32767.0f) o = 32767.0f;
   else if (o<(-3.2767E+4f)) o = (-3.2767E+4f);
   return o;
} /* end preem() */


static float encstereo(float l, float r,
                struct STEREOCONTEXT * stereocontext)
{
   float s;
   struct STEREOCONTEXT * anonym;
   { /* with */
      struct STEREOCONTEXT * anonym = stereocontext;
      anonym->wpilot = anonym->wpilot+X2C_DIVR(1.1938052083641E+5f,
                (float)sample);
      if (anonym->wpilot>6.2831853071796f) {
         anonym->wpilot = anonym->wpilot-6.2831853071796f;
      }
      /*    RETURN (r+l) + (r-l)*sin(wpilot*2.0) + sin(wpilot)*(0.4*32768.0);
                 */
      s = osic_sin(anonym->wpilot);
      /*    RETURN (r+l) + (r-l)*(s*s*2.0-1.0) + s*(0.4*32768.0); */
      return (r-l)*s*s+l+s*13107.2f;
   }
} /* end encstereo() */


static void makelp24(float fg, float samp, struct LPCONTEXT24 * c)
{
   struct LPCONTEXT24 * anonym;
   { /* with */
      struct LPCONTEXT24 * anonym = c;
      anonym->LPR = (X2C_DIVR(fg,samp))*2.33363f;
      anonym->LPL = anonym->LPR*anonym->LPR*2.888f*(1.0f-9.0f*osic_power(X2C_DIVR(fg,
                samp), 2.0f));
      anonym->OLPR = 1.0f-anonym->LPR;
   }
} /* end makelp24() */


static void makelp12(float fg, float samp, struct LPCONTEXT12 * c)
{
   struct LPCONTEXT12 * anonym;
   { /* with */
      struct LPCONTEXT12 * anonym = c;
      anonym->K1 = osic_power((X2C_DIVR(fg,samp))*5.2f,
                2.0f)-osic_power((X2C_DIVR(fg,samp))*5.2f, 3.0f)*0.5f;
      anonym->K2 = 1.0f-osic_power(anonym->K1*1.0f, 0.48f);
                /* 0.85 10% ripple, 1.0 11% overshoot */
   }
} /* end makelp12() */

static void exitprog(int32_t);


static void exitprog(int32_t signum)
{
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   exit0 = 1;
} /* end exitprog() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   uint32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   osi_BEGIN();
   signal(SIGTERM, exitprog);
   signal(SIGINT, exitprog);
   signal(SIGPIPE, exitprog);
   adcrate = 48000UL;
   sample = 1056000UL;
   upsamp = sample/adcrate;
   mod = pluto_mFM;
   /*
     fd:=OpenWrite("/tmp/iq");
     IF fd<0 THEN WrStrLn("file open error"); HALT END;
   
     WrInt(plutoio.Initiio("ip:192.168.2.1"), 1);
     WrStrLn(":init");
     WrInt(plutoio.setrxgainmode("manual"), 1);
     WrStrLn(":gainmode");
     WrInt(plutoio.setrxgain(55), 1);
     WrStrLn(":rxgain");
     WrInt(plutoio.setfreq(93.025, FALSE), 1);
     WrStrLn(":mhz");
     WrInt(plutoio.setbw(1000000), 1);
     WrStrLn(":bw");
     WrInt(plutoio.setsampel(1024000, FALSE), 1);
     WrStrLn(":samphz");
     WrInt(plutoio.setport("A_BALANCED", FALSE), 1);
     WrStrLn(":rxport");
   --  plutoio.PlutoRxOff(FALSE);
     WrInt(plutoio.startrx(SAMPLES), 1);
     WrStrLn(":startrx");
     FOR i:=0 TO 99 DO
       len:=plutoio.receive(buf);  
       WrInt(len, 8);
       WrBin(fd, buf, SAMPLES*4);
     END;
     Close(fd);
     plutoio.closeiio;
   */
   OpenSound();
   osic_WrINT32((uint32_t)Initiio("ip:192.168.2.1"), 1UL);
   osi_WrStrLn(":init", 6ul);
   osic_WrINT32((uint32_t)starttx(16384UL, &buf), 1UL);
   osi_WrStrLn(":starttx", 9ul);
   osic_WrINT32((uint32_t)settxbufs(8UL), 1UL);
   osi_WrStrLn(":settxbufs", 11ul);
   osic_WrINT32((uint32_t)settxgain(-5), 1UL);
   osi_WrStrLn(":txgain", 8ul);
   osic_WrINT32((uint32_t)setfreq(430.25, 1), 1UL);
   osi_WrStrLn(":mhz", 5ul);
   osic_WrINT32((uint32_t)firenable(1), 1UL);
   osi_WrStrLn(":fir", 5ul);
   osic_WrINT32((uint32_t)setsampel(sample, 1), 1UL);
   osi_WrStrLn(":samphz", 8ul);
   osic_WrINT32((uint32_t)setbw(sample, 1), 1UL);
   osi_WrStrLn(":bwhz", 6ul);
   osic_WrINT32((uint32_t)setport("A", 1), 1UL);
   osi_WrStrLn(":rxport", 8ul);
   makelp12(5000.0f, (float)sample, &upsamplelpi);
   upsamplelpq = upsamplelpi;
   makelp24(2700.0f, (float)adcrate, &miclp);
   makelp24(1200.0f, (float)adcrate, &ssblpi);
   ssblpq = ssblpi;
   for (i = 0L; i<=32767L; i++) {
      sintab[i] = (short)X2C_TRUNCI(32767.0f*osic_sin(X2C_DIVR((float)
                i*3.1415926535898f*2.0f,32768.0f)),-32768,32767);
   } /* end for */
   ssbwinc = X2C_DIVR(9.4247779607694E+3f,(float)adcrate);
   fmdeviation = X2C_DIVR(X2C_DIVR(2.1474836475E+13f,(float)sample),
                2.925625E+4f);
   limlev = 2.6330625E+4f;
   levmul = 500.0f;
   shift = (int32_t)X2C_TRUNCI(X2C_DIVR(1.07374182375E+14f,
                (float)sample),X2C_min_longint,X2C_max_longint);
   pabuf = 2147483647UL;
   p = 0UL;
   do {
      if (mod!=pluto_mSTEREO) asamp = audioproc(audiosamp());
      else {
         bbi = 2.0f*preem(audiosamp(), &cstereo.lastl);
         bbq = 2.0f*preem(audiosamp(), &cstereo.lastr);
      }
      /*audioout(asamp); */
      if (mod==pluto_mUSB) ssbtx(asamp, &bbi, &bbq);
      else if (mod==pluto_mLSB) ssbtx(asamp, &bbq, &bbi);
      else if (mod==pluto_mFM) {
         bbi = asamp*fmdeviation;
      }
      else if (mod==pluto_mAM) bbi = asamp;
      /*WrFixed(asamp, 2,8); */
      if (bbq>pk) pk = bbq;
      tmp = upsamp-1UL;
      is = 0UL;
      if (is<=tmp) for (;; is++) {
         upsamplelpi.uc = upsamplelpi.uc+upsamplelpi.il;
         upsamplelpi.il = upsamplelpi.il*upsamplelpi.K2+(bbi-upsamplelpi.uc)
                *upsamplelpi.K1;
         if (mod==pluto_mFM) {
            buf[p] = sintab[(uint32_t)wshift>>17];
            ++p;
            buf[p] = sintab[(uint32_t)(wshift+1073741824L)>>17];
            ++p;
            wshift = wshift+shift+(int32_t)X2C_TRUNCI(upsamplelpi.uc,
                X2C_min_longint,X2C_max_longint);
         }
         else if (mod==pluto_mAM) {
            si = (int32_t)X2C_TRUNCI(upsamplelpi.uc,X2C_min_longint,
                X2C_max_longint)+32768L>>1;
            if (si<0L) si = 0L;
            else if (si>32767L) si = 32767L;
            buf[p] = (short)(si*(int32_t)sintab[(uint32_t)
                wshift>>17]>>15);
            ++p;
            buf[p] = (short)(si*(int32_t)sintab[(uint32_t)
                (wshift+1073741824L)>>17]>>15);
            ++p;
            wshift += shift;
         }
         else if (mod==pluto_mSTEREO) {
            upsamplelpq.uc = upsamplelpq.uc+upsamplelpq.il;
            upsamplelpq.il = upsamplelpq.il*upsamplelpq.K2+(bbq-upsamplelpq.uc)
                *upsamplelpq.K1;
            buf[p] = sintab[(uint32_t)wshift>>17];
            ++p;
            buf[p] = sintab[(uint32_t)(wshift+1073741824L)>>17];
            ++p;
            wshift = wshift+shift+(int32_t)
                X2C_TRUNCI(fmdeviation*encstereo(upsamplelpi.uc,
                upsamplelpq.uc, &cstereo),X2C_min_longint,X2C_max_longint);
         }
         else {
            /* iq baseband upsample */
            upsamplelpq.uc = upsamplelpq.uc+upsamplelpq.il;
            upsamplelpq.il = upsamplelpq.il*upsamplelpq.K2+(bbq-upsamplelpq.uc)
                *upsamplelpq.K1;
            si = (int32_t)sintab[(uint32_t)wshift>>17];
            sq = (int32_t)sintab[(uint32_t)(wshift+1073741824L)>>17];
            bi = (int32_t)X2C_TRUNCI(upsamplelpi.uc,X2C_min_longint,
                X2C_max_longint);
            bq = (int32_t)X2C_TRUNCI(upsamplelpq.uc,X2C_min_longint,
                X2C_max_longint);
            tmpi = bi*si-bq*sq>>15;
            if (tmpi>32767L) tmpi = 32767L;
            else if (tmpi<-32767L) tmpi = -32767L;
            buf[p] = (short)tmpi;
            ++p;
            tmpi = bi*sq+bq*si>>15;
            if (tmpi>32767L) tmpi = 32767L;
            else if (tmpi<-32767L) tmpi = -32767L;
            buf[p] = (short)tmpi;
            ++p;
            wshift += shift;
         }
         if (p>=16384UL) {
            len = transmitbuf();
            osic_WrINT32((uint32_t)len, 8UL);
            /*WrFixed(pk, 0, 8); pk:=0.0;  */
            p = 0UL;
         }
         if (is==tmp) break;
      } /* end for */
   } while (!exit0);
   closeiio();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
