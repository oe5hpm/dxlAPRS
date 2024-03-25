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
#define lorarx_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <signal.h>

/* iq downsample, shift, (auto)notch lora demodulator with axudp, json out and packet radio, aprs, lorawan,
                fanet viewer */
/*--------------------------------------------------------------------------------

Synchronisation simple:

Sampling preamble upchirps give independent of sample time constant fft frequency.

So sample at any time with normal chirprate and, if it looks like an upchrip,
jump as many samples as the found frequency differs from zero.
 
As not known exact frequency and not at all sample time but frequency
is proportional to time, we first say we are (+-25%) on frequency so we get
a 75% exact sample time.

This is good enough for decoding the 2 sync chirps and switch to reverse chirps.

After fft of the 2 reverse chirps we get another new zero frequency.
The median of upchirps and downchirps frequencies makes the true receive frequency.

As we know now how far we are away from real frequency, we jump in sampletime half of
the upchirp-downchirp frequency difference (only full samples) + the quarter downchirp
(thanks to the quarter chirp, we never need back jumps) and add the frequency change
from the jump to the median of the upchirps frequencys.

Now we are 1 sample exact on sampletime and (as exact as the median of the
preamble) on frequency and can decode data. Further drifts in frequency and time
(samplerate error) may be corrected with AFC. Sampletime needs no further correction.

----------------

Demodulation-test rtl-stick/sx127x (sf12,cr5,125khz)
stable signals not much multipath: till same s/n
colliding transmissions, stronger signal later: sx: zero frames, rtl 100% if +1.5dB
                                         first: sx: if +3dB, rtl +1.5dB
fading with massive multipath: sx better, rtl still in work 

----------------

frequency stability (short/longterm) of rtl is ok
rtl's fast AGC up/down on pulsing outband signals modulates carrier and spreads fft-bin
same as mobile fading

---------------------------------------------------------------------------------*/
#define lorarx_PI 3.1415926535

#define lorarx_PI2 6.283185307

#define lorarx_LF "\012"

#define lorarx_MAXSF 4096

#define lorarx_TIMEOUT 3
/* chips with no dcd is end of unkown lenght data */

#define lorarx_MAXBINS 3
/* look at weaker bins for fitting parity */

#define lorarx_FRAMELEN 256

#define lorarx_DATASQUELCH 15.0

#define lorarx_DATASQUELCHFACT (-1.1)
/* adapt squelch to sf */

#define lorarx_MAXNEST 4
/* start new demodulator if other(s) in frame */

#define lorarx_MAXINBUF 2048

#define lorarx_MINSF 5

#define lorarx_SHIFTSTEPS 65536

static uint8_t lorarx_WHITEN[255] = {255U,254U,252U,248U,240U,225U,194U,133U,11U,23U,47U,94U,188U,120U,241U,227U,198U,
                141U,26U,52U,104U,208U,160U,64U,128U,1U,2U,4U,8U,17U,35U,71U,142U,28U,56U,113U,226U,196U,137U,18U,37U,
                75U,151U,46U,92U,184U,112U,224U,192U,129U,3U,6U,12U,25U,50U,100U,201U,146U,36U,73U,147U,38U,77U,155U,
                55U,110U,220U,185U,114U,228U,200U,144U,32U,65U,130U,5U,10U,21U,43U,86U,173U,91U,182U,109U,218U,181U,
                107U,214U,172U,89U,178U,101U,203U,150U,44U,88U,176U,97U,195U,135U,15U,31U,62U,125U,251U,246U,237U,219U,
                183U,111U,222U,189U,122U,245U,235U,215U,174U,93U,186U,116U,232U,209U,162U,68U,136U,16U,33U,67U,134U,13U,
                27U,54U,108U,216U,177U,99U,199U,143U,30U,60U,121U,243U,231U,206U,156U,57U,115U,230U,204U,152U,49U,98U,
                197U,139U,22U,45U,90U,180U,105U,210U,164U,72U,145U,34U,69U,138U,20U,41U,82U,165U,74U,149U,42U,84U,169U,
                83U,167U,78U,157U,59U,119U,238U,221U,187U,118U,236U,217U,179U,103U,207U,158U,61U,123U,247U,239U,223U,
                191U,126U,253U,250U,244U,233U,211U,166U,76U,153U,51U,102U,205U,154U,53U,106U,212U,168U,81U,163U,70U,
                140U,24U,48U,96U,193U,131U,7U,14U,29U,58U,117U,234U,213U,170U,85U,171U,87U,175U,95U,190U,124U,249U,242U,
                229U,202U,148U,40U,80U,161U,66U,132U,9U,19U,39U,79U,159U,63U,127U};

#define lorarx_MAXBW 5.E+5

static uint32_t lorarx_BWTAB[10] = {64UL,48UL,32UL,24UL,16UL,12UL,8UL,4UL,2UL,1UL};

static uint8_t lorarx_HAMMTAB8[256] = {128U,192U,192U,3U,192U,5U,6U,135U,192U,9U,10U,139U,12U,141U,142U,15U,128U,1U,
                2U,199U,4U,199U,199U,135U,8U,137U,138U,11U,140U,13U,14U,199U,128U,1U,2U,131U,4U,133U,206U,7U,8U,137U,
                206U,11U,206U,13U,142U,206U,0U,201U,130U,3U,132U,5U,6U,135U,201U,137U,10U,201U,12U,201U,142U,15U,128U,
                1U,2U,203U,4U,133U,134U,7U,8U,203U,203U,139U,140U,13U,14U,203U,0U,129U,130U,3U,204U,5U,6U,135U,204U,9U,
                10U,139U,140U,204U,204U,15U,0U,197U,130U,3U,197U,133U,6U,197U,136U,9U,10U,139U,12U,197U,142U,15U,194U,
                1U,130U,194U,4U,133U,194U,7U,8U,137U,194U,11U,140U,13U,14U,143U,128U,1U,2U,131U,4U,205U,134U,7U,8U,205U,
                138U,11U,205U,141U,14U,205U,0U,129U,202U,3U,132U,5U,6U,135U,202U,9U,138U,202U,12U,141U,202U,15U,0U,195U,
                195U,131U,132U,5U,6U,195U,136U,9U,10U,195U,12U,141U,142U,15U,196U,1U,2U,131U,132U,196U,196U,7U,8U,137U,
                138U,11U,196U,13U,14U,143U,0U,129U,198U,3U,198U,5U,134U,198U,136U,9U,10U,139U,12U,141U,198U,15U,193U,
                129U,2U,193U,4U,193U,134U,7U,8U,193U,138U,11U,140U,13U,14U,143U,200U,1U,2U,131U,4U,133U,134U,7U,136U,
                200U,200U,11U,200U,13U,14U,143U,0U,129U,130U,3U,132U,5U,6U,207U,136U,9U,10U,207U,12U,207U,207U,143U};

static uint8_t lorarx_HAMMTAB7[128] = {128U,192U,192U,131U,192U,205U,134U,135U,192U,205U,138U,139U,205U,141U,142U,
                205U,128U,129U,202U,199U,132U,199U,199U,135U,202U,137U,138U,202U,140U,141U,202U,199U,128U,195U,195U,
                131U,132U,133U,206U,195U,136U,137U,206U,195U,206U,141U,142U,206U,196U,201U,130U,131U,132U,196U,196U,
                135U,201U,137U,138U,201U,196U,201U,142U,143U,128U,129U,198U,203U,198U,133U,134U,198U,136U,203U,203U,
                139U,140U,141U,198U,203U,193U,129U,130U,193U,204U,193U,134U,135U,204U,193U,138U,139U,140U,204U,204U,
                143U,200U,197U,130U,131U,197U,133U,134U,197U,136U,200U,200U,139U,200U,197U,142U,143U,194U,129U,130U,
                194U,132U,133U,194U,207U,136U,137U,194U,207U,140U,207U,207U,143U};

static uint8_t lorarx_HAMMTAB6[64] = {128U,1U,2U,3U,4U,5U,134U,7U,8U,9U,10U,139U,12U,141U,14U,15U,0U,129U,2U,3U,4U,5U,
                6U,135U,8U,9U,138U,11U,140U,13U,14U,15U,0U,1U,2U,131U,4U,133U,6U,7U,136U,9U,10U,11U,12U,13U,142U,15U,0U,
                1U,130U,3U,132U,5U,6U,7U,8U,137U,10U,11U,12U,13U,14U,143U};

struct Complex;


struct Complex {
   float Re;
   float Im;
};


struct CB {
   struct Complex * Adr;
   size_t Len0;
};

typedef struct CB * pCB;

struct BIN;


struct BIN {
   float sharpness;
   float lev;
   float freq;
   uint32_t bn;
};

struct BINS;


struct BINS {
   struct BIN b[4];
   float noise;
   float splt;
   uint32_t halfnoisc;
};

typedef uint8_t NIBBBLOCK[12];

typedef uint8_t NIBBS[523];

enum STATES {lorarx_sSLEEP, lorarx_sHUNT, lorarx_sSYNRAW, lorarx_sID, lorarx_sREV1, lorarx_sREV2, lorarx_sDATA};


typedef struct BINS BINTAB[8];

struct FFRAME;


struct FFRAME {
   BINTAB bintab;
   NIBBS nibbs;
   char fecinfo[96];
   NIBBBLOCK oneerr;
   uint32_t chirpc;
   uint32_t nibbc;
   uint32_t crfromhead;
   uint32_t dlen;
   uint32_t idfound;
   uint32_t cnt;
   uint32_t dcnt;
   uint32_t txdel;
   uint32_t jp;
   uint32_t cfgsf;
   uint32_t cfgcr;
   uint32_t synfilter;
   uint32_t timeout;
   uint32_t cfgdatalen;
   uint32_t iqread;
   uint32_t label;
   uint32_t fp;
   uint32_t fasecorrs;
   uint32_t oneerrs;
   float fc;
   float fci;
   float fcstart;
   float fcfix;
   float sigsum;
   float sigmin;
   float sigmax;
   float noissum;
   float noismax;
   float noismin;
   float eye;
   float lastbin;
   float lastsq;
   float afcspeed;
   float datasquelch;
   float dataratecorr;
   char optimize;
   char withhead;
   char withcrc;
   char implicitcrc;
   char nodcdlost;
   char udpcrcok;
   char invertiq;
   char ax25long;
   char done;
   int32_t df;
   uint8_t state;
   uint32_t ipnumraw;
   uint32_t udprawport;
   uint32_t ipnum;
   uint32_t udpport;
   char udp2;
   char axjoin;
   char axpart[255];
};

struct DEM;

typedef struct DEM * pDEM;


struct DEM {
   pDEM next;
   int32_t cfgopt;
   uint32_t coldet;
   char done;
   struct FFRAME frames[4];
};

static pDEM dems;

static char iqfn[1024];

static char jpipename[1024];

static int32_t jsonfd;

static int32_t iqfd;

static uint32_t firlen;

static uint32_t globfilt;

static uint32_t bwnum;

static uint32_t iqwrite;

static uint32_t binsview;

static uint32_t isize;

static char quietcrc;

static char newline;

static char verb;

static char verb2;

static char nomultipath;

static char allwaysascii;

static char nocrcfec;

static char nofec;

static int32_t udpsock;

static struct Complex iqbuf[8192];

static pCB fftbufs[13];

static float DDS[65536];

static float cfglevel;

static uint32_t jipnum;

static uint32_t judpport;

static float globdrate;

static float jmhz;

static float SINTAB[25];

#define lorarx_MAXINFIRLEN 4096

#define lorarx_FIRMAX 4096

#define lorarx_SUBSAMPBITS 4

#define lorarx_SUBSAMP 16


struct _0 {
   struct Complex * Adr;
   size_t Len0;
};

typedef struct _0 * pFIRTAB;


struct _1 {
   struct Complex * Adr;
   size_t Len0;
};

typedef struct _1 * pFIR;

struct NOTCH;

typedef struct NOTCH * pNOTCH;


struct NOTCH {
   pNOTCH next;
   float f1;
   float f2;
};


struct _2 {
   float * Adr;
   size_t Len0;
};

typedef struct _2 * pFREQMASK;


struct FIXFIR {
   float * Adr;
   size_t Len0;
};

typedef struct FIXFIR * pFIXFIR;

static pFIR pfir;

static pFIXFIR pfixfir;

static pNOTCH pnotches;

static float pknoisemed;

static float pknoiseup;

static float pknoisedown;

static float notchthres;

static float downsample;

static uint32_t notchcnt;

static uint32_t autonotch;

static uint32_t samprate;

static uint32_t firwp;

static uint32_t sampc;

static uint32_t shiftstep;

static uint32_t birdpos;

static uint32_t phasereg;

static char noisblanker;

static char complexfir;

static pFREQMASK pbirdhist;

static pFIRTAB ptmpfir;

static pFIRTAB pbirdbuf;

static pFIRTAB pfirtab;

static int32_t outdechirped;

static int32_t outfiltered;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static char hex(uint32_t n)
{
   n = n&15UL;
   if (n<=9UL) return (char)(n+48UL);
   return (char)(n+55UL);
} /* end hex() */


static void WrHex(uint32_t x, uint32_t digits, uint32_t len)
{
   uint32_t i0;
   char s[256];
   if (digits>255UL) digits = 255UL;
   i0 = digits;
   while (i0<len && i0<255UL) {
      s[i0] = ' ';
      ++i0;
   }
   s[i0] = 0;
   while (digits>0UL) {
      --digits;
      s[digits] = hex(x);
      x = x/16UL;
   }
   osi_WrStr(s, 256ul);
} /* end WrHex() */


static void WrChHex(char c)
{
   char s[11];
   if ((uint8_t)c>='\177' || (uint8_t)c<' ') {
      s[0U] = '[';
      s[1U] = hex((uint32_t)(uint8_t)c/16UL);
      s[2U] = hex((uint32_t)(uint8_t)c);
      s[3U] = ']';
      s[4U] = 0;
   }
   else {
      s[0U] = c;
      s[1U] = 0;
   }
   osi_WrStr(s, 11ul);
} /* end WrChHex() */


static float lim(float x, float max0)
{
   if (x>max0) return max0;
   if (x<-max0) return -max0;
   return x;
} /* end lim() */


static float flmin(float x, float min0)
{
   if (x<min0) return x;
   return min0;
} /* end flmin() */


static float flmax(float x, float max0)
{
   if (x<max0) return max0;
   return x;
} /* end flmax() */


static float CABS(struct Complex X)
{
   return osic_sqrt(X.Re*X.Re+X.Im*X.Im);
} /* end CABS() */


static float ln0(float x)
{
   if (x<=0.0f) return 0.0f;
   return osic_ln(x);
} /* end ln0() */


static float freqmod(float d, int32_t max0)
{
   float m;
   float h;
   m = (float)max0;
   h = m*0.5f;
   if (d>h) return d-m;
   if (d<-h) return d+m;
   return d;
} /* end freqmod() */

/* abs difference a-b in cycle */
/*------ fft */
/*-not level correct fft and dc part same level */

static void buttf(struct Complex * R, struct Complex * A, struct Complex * B)
{
   struct Complex h;
   h.Re = A->Re*B->Re-A->Im*B->Im;
   h.Im = A->Re*B->Im+A->Im*B->Re;
   B->Re = R->Re-h.Re;
   B->Im = R->Im-h.Im;
   R->Re = R->Re+h.Re;
   R->Im = R->Im+h.Im;
} /* end buttf() */


static void Transform(struct Complex feld[], uint32_t feld_len, uint32_t logsize, char INVERS)
{
   uint32_t n2;
   uint32_t ri;
   uint32_t N;
   uint32_t id;
   uint32_t idd2;
   uint32_t z;
   uint32_t j1;
   uint32_t i0;
   struct Complex wcpx;
   struct Complex h;
   float sko;
   float sn;
   float cs;
   float sk;
   float r;
   float ck;
   struct Complex * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   N = (uint32_t)(1UL<<logsize);
   /*bitrev*/
   ri = 1UL;
   n2 = N/2UL;
   tmp = N-1UL;
   i0 = 1UL;
   if (i0<=tmp) for (;; i0++) {
      if (i0<ri) {
         h = feld[ri-1UL];
         feld[ri-1UL] = feld[i0-1UL];
         feld[i0-1UL] = h;
      }
      j1 = n2;
      while (j1<ri) {
         ri -= j1;
         j1 = j1/2UL;
      }
      ri += j1;
      if (i0==tmp) break;
   } /* end for */
   /*bitrev*/
   idd2 = 1UL;
   sko = 0.0f;
   tmp = logsize;
   z = 1UL;
   if (z<=tmp) for (;; z++) {
      id = 2UL*idd2;
      sk = sko;
      /*    sko:=sin(wk); */
      sko = SINTAB[z];
      if (INVERS) sko = -sko;
      ck = 2.0f*sko*sko;
      r = -(2.0f*ck);
      cs = 1.0f;
      sn = 0.0f;
      i0 = 0UL;
      while (i0<N) {
         h = feld[i0+idd2];
         { /* with */
            struct Complex * anonym = &feld[i0];
            feld[i0+idd2].Re = anonym->Re-h.Re;
            feld[i0+idd2].Im = anonym->Im-h.Im;
            anonym->Re = anonym->Re+h.Re;
            anonym->Im = anonym->Im+h.Im;
         }
         i0 += id;
      }
      if (z!=1UL) {
         tmp0 = idd2-1UL;
         j1 = 1UL;
         if (j1<=tmp0) for (;; j1++) {
            ck = r*cs+ck;
            cs = cs+ck;
            sk = r*sn+sk;
            sn = sn+sk;
            wcpx.Re = cs;
            wcpx.Im = sn;
            i0 = j1;
            while (i0<N) {
               buttf(&feld[i0], &wcpx, &feld[i0+idd2]);
               i0 += id;
            }
            if (j1==tmp0) break;
         } /* end for */
      }
      idd2 = id;
      if (z==tmp) break;
   } /* end for */
/*------ fft */
} /* end Transform() */


static float smoothnotch(pFREQMASK p, uint32_t i0, float thres)
{
   float v2;
   float v1;
   float v;
   if (p==0) return (-2.0f);
   v = p->Adr[i0];
   if (v<thres) return (-2.0f);
   v1 = 0.0f;
   v2 = 0.0f;
   ++i0;
   if (i0<firlen) v2 = p->Adr[i0];
   if (i0>=2UL) v1 = p->Adr[i0-2UL];
   return lim(X2C_DIVR(v2-v1,v), 1.0f);
} /* end smoothnotch() */


static void shownotches(void)
{
   uint32_t m;
   uint32_t nc;
   int32_t ii;
   int32_t tmp;
   if (pbirdhist) {
      if (verb2) osi_WrStr(" notch at:", 11ul);
      else osi_WrStr(" notches:", 10ul);
      nc = 0UL;
      m = (uint32_t)X2C_TRUNCC((X2C_DIVR((float)firlen,downsample))*0.5f+0.5f,0UL,X2C_max_longcard);
      if (m>=firlen/2UL) m = firlen/2UL-1UL;
      tmp = (int32_t)m;
      ii = -(int32_t)m;
      if (ii<=tmp) for (;; ii++) {
         if (smoothnotch(pbirdhist, (uint32_t)((int32_t)(firlen/2UL)+ii), notchthres)>=(-1.0f)) {
            if (verb2) {
               if (nc) osi_WrStr(",", 2ul);
               osic_WrINT32((uint32_t)ii, 1UL);
            }
            ++nc;
         }
         if (ii==tmp) break;
      } /* end for */
      if (!verb2) osic_WrINT32(nc, 1UL);
   }
} /* end shownotches() */


static float manualnotch(int32_t fi)
{
   pNOTCH p;
   float v;
   float f;
   f = (float)fi;
   p = pnotches;
   v = 1.0f;
   while (p) {
      if (p->f1>=f) v = flmin(v, (p->f1-f)*0.5f);
      else if (p->f2<f) v = flmin(v, (f-p->f2)*0.5f);
      else v = 0.0f;
      p = p->next;
   }
   return flmax(0.0f, (v*v-0.25f)*1.33333333f);
} /* end manualnotch() */


static void joinfirs(int32_t i0, float w0)
{
   uint32_t j1;
   /*WrFixed(w,3,1); WrStrLn("=w"); */
   if ((float)fabs(w0)<=1.0f) {
      if (i0<0L) j1 = (uint32_t)((int32_t)(ptmpfir->Len0-1)+1L+i0);
      else j1 = (uint32_t)i0;
      ptmpfir->Adr[j1].Im = 0.0f;
      --i0;
      if (i0<0L) j1 = (uint32_t)((int32_t)(ptmpfir->Len0-1)+1L+i0);
      else j1 = (uint32_t)i0;
      ptmpfir->Adr[j1].Im = ptmpfir->Adr[j1].Im*(w0*0.25f+0.25f);
      i0 += 2L;
      if (i0<0L) j1 = (uint32_t)((int32_t)(ptmpfir->Len0-1)+1L+i0);
      else j1 = (uint32_t)i0;
      ptmpfir->Adr[j1].Im = ptmpfir->Adr[j1].Im*(0.25f-w0*0.25f);
   }
} /* end joinfirs() */

#define lorarx_DECNOTCH 10


static void wrnotch(uint32_t * j1, int32_t f)
{
   if (*j1>0UL) osi_WrStr(",", 2ul);
   else osi_WrStr("manual notches at:", 19ul);
   osic_WrINT32((uint32_t)f, 1UL);
   ++*j1;
} /* end wrnotch() */


static void makefir(float fg, pFIRTAB pfir0, pFREQMASK pbird, float thres)
{
   uint32_t subsamp;
   uint32_t flen;
   uint32_t logfl;
   uint32_t m;
   uint32_t j1;
   uint32_t i0;
   int32_t ii;
   struct Complex u;
   float l;
   float w0;
   uint32_t tmp;
   int32_t tmp0;
   flen = (pfir0->Len0-1)+1UL;
   subsamp = flen/firlen;
   if (pbird==0) {
      tmp = firlen/2UL-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         pfixfir->Adr[firlen/2UL+i0] = manualnotch((int32_t)i0);
         pfixfir->Adr[(firlen/2UL-i0)-1UL] = manualnotch(-(int32_t)(i0+1UL));
         if (i0==tmp) break;
      } /* end for */
      if (verb) {
         j1 = 0UL;
         for (i0 = firlen/2UL-1UL; i0>=1UL; i0--) {
            if (pfixfir->Adr[(firlen/2UL-i0)-1UL]<0.5f) wrnotch(&j1, -(int32_t)i0);
         } /* end for */
         tmp = firlen/2UL-1UL;
         i0 = 0UL;
         if (i0<=tmp) for (;; i0++) {
            if (pfixfir->Adr[firlen/2UL+i0]<0.5f) wrnotch(&j1, (int32_t)i0);
            if (i0==tmp) break;
         } /* end for */
         if (j1>0UL) osi_WrStrLn("", 1ul);
      }
   }
   tmp = ptmpfir->Len0-1;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      ptmpfir->Adr[i0].Re = 0.0f;
      ptmpfir->Adr[i0].Im = 0.0f;
      if (i0==tmp) break;
   } /* end for */
   w0 = (X2C_DIVR((float)firlen,fg))*0.5f+0.5f; /* downsample bandfilter */
   m = (uint32_t)X2C_TRUNCC(w0,0UL,X2C_max_longcard);
   w0 = w0-(float)m;
   if (m>=firlen/2UL) m = firlen/2UL-1UL;
   tmp0 = (int32_t)m;
   ii = -(int32_t)m;
   if (ii<=tmp0) for (;; ii++) {
      /* manual notches */
      if (ii<0L) j1 = (uint32_t)((int32_t)(ptmpfir->Len0-1)+1L+ii);
      else j1 = (uint32_t)ii;
      ptmpfir->Adr[j1].Im = pfixfir->Adr[(int32_t)(firlen/2UL)+ii];
      if (ii==tmp0) break;
   } /* end for */
   ptmpfir->Adr[m].Im = w0;
   ptmpfir->Adr[(ptmpfir->Len0-1)-m].Im = w0;
   if (pbird) {
      /* adative notches */
      tmp0 = (int32_t)m;
      ii = -(int32_t)m;
      if (ii<=tmp0) for (;; ii++) {
         joinfirs(ii, smoothnotch(pbird, (uint32_t)((int32_t)(firlen/2UL)+ii), thres));
         if (ii==tmp0) break;
      } /* end for */
   }
   /*
   FOR i:=0 TO firlen-1 DO WrFixed(ptmpfir^[i].Im,2,5) END; WrStrLn("");
   FOR i:=0 TO firlen-1 DO WrFixed(ptmpfir^[HIGH(ptmpfir^)-i].Im,2,5) END; WrStrLn("");
   */
   logfl = 1UL;
   do {
      ++logfl;
   } while (flen>(uint32_t)(1UL<<logfl));
   Transform(ptmpfir->Adr, ptmpfir->Len0, logfl, 1);
   m = flen/2UL;
   tmp = m-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      u = ptmpfir->Adr[i0];
      ptmpfir->Adr[i0] = ptmpfir->Adr[i0+m];
      ptmpfir->Adr[i0+m] = u;
      if (i0==tmp) break;
   } /* end for */
   l = 0.00001f;
   tmp = flen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      u = ptmpfir->Adr[(((i0%firlen)*subsamp+subsamp)-1UL)-i0/firlen]; /* rearange table for better memory cashing */
      pfir0->Adr[i0] = u;
      w0 = CABS(u);
      if (w0>l) l = w0;
      if (i0==tmp) break;
   } /* end for */
   l = X2C_DIVR(1.0f,l);
   tmp = flen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      pfir0->Adr[i0].Re = l*pfir0->Adr[i0].Re; /* make uniq gain */
      pfir0->Adr[i0].Im = l*pfir0->Adr[i0].Im;
      if (i0==tmp) break;
   } /* end for */
/*shownotches; */
} /* end makefir() */


static void MakeDDS(void)
{
   uint32_t i0;
   float r;
   r = 9.5873799240112E-5f;
   for (i0 = 0UL; i0<=65535UL; i0++) {
      DDS[i0] = osic_sin((float)i0*r);
   } /* end for */
} /* end MakeDDS() */

static uint32_t _cnst[10] = {64UL,48UL,32UL,24UL,16UL,12UL,8UL,4UL,2UL,1UL};

static float baud(uint32_t sf, uint32_t bwnum0)
{
   return X2C_DIVR(5.E+5f,(float)(_cnst[bwnum0]*(uint32_t)(1UL<<sf)));
} /* end baud() */

#define lorarx_DEFAULTIP 0x7F000001 

#define lorarx_PORTSEP ":"


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * ip, uint32_t * port)
{
   uint32_t p;
   uint32_t n;
   uint32_t i0;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i0 = 0UL; i0<=4UL; i0++) {
      if (i0>=3UL || h[0UL]!=':') {
         n = 0UL;
         ok0 = 0;
         while ((uint8_t)h[p]>='0' && (uint8_t)h[p]<='9') {
            ok0 = 1;
            n = (n*10UL+(uint32_t)(uint8_t)h[p])-48UL;
            ++p;
         }
         if (!ok0) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      if (i0<3UL) {
         if (h[0UL]!=':') {
            if (h[p]!='.' || n>255UL) {
               GetIp_ret = -1L;
               goto label;
            }
            *ip =  *ip*256UL+n;
         }
      }
      else if (i0==3UL) {
         if (h[0UL]!=':') {
            *ip =  *ip*256UL+n;
            if (h[p]!=':' || n>255UL) {
               GetIp_ret = -1L;
               goto label;
            }
         }
         else {
            p = 0UL;
            *ip = 2130706433UL;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++p;
   } /* end for */
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static char StrToHex(const char s[], uint32_t s_len, uint32_t * n)
{
   uint32_t i0;
   char c;
   i0 = 0UL;
   *n = 0UL;
   while (i0<=s_len-1 && s[i0]) {
      *n =  *n*16UL;
      c = X2C_CAP(s[i0]);
      if ((uint8_t)c>='0' && (uint8_t)c<='9') *n += (uint32_t)(uint8_t)c-48UL;
      else if ((uint8_t)c>='A' && (uint8_t)c<='F') *n += (uint32_t)(uint8_t)c-55UL;
      else return 0;
      ++i0;
   }
   return 1;
} /* end StrToHex() */


static void newdem(void)
{
   pDEM dem;
   struct FFRAME * anonym;
   osic_alloc((char * *) &dem, sizeof(struct DEM));
   if (dem==0) Error("out of memory", 14ul);
   memset((char *)dem,(char)0,sizeof(struct DEM));
   dem->cfgopt = -1L;
   dem->coldet = 3UL;
   { /* with */
      struct FFRAME * anonym = &dem->frames[0U];
      anonym->cfgsf = 0UL;
      anonym->cfgcr = 0UL;
      anonym->synfilter = globfilt;
      anonym->afcspeed = (-1.0f);
      anonym->dataratecorr = globdrate;
      anonym->datasquelch = (-1.0f);
   }
   dem->next = dems;
   dems = dem;
} /* end newdem() */


static void Parms(void)
{
   char hassf;
   char hasbw;
   char hasudp;
   char err;
   float afcspd;
   float rh;
   float insamplerate;
   int32_t ih;
   uint32_t globcr;
   uint32_t globsf;
   char h[1024];
   uint32_t i0;
   pDEM pd0;
   pNOTCH pnotch;
   float configoffsethz;
   float offs;
   struct NOTCH * anonym;
   pnotch = 0;
   complexfir = 0;
   shiftstep = 0UL;
   configoffsethz = 0.0f;
   autonotch = 0UL;
   noisblanker = 0;
   pknoisemed = 0.0f;
   outdechirped = -1L;
   outfiltered = -1L;
   allwaysascii = 0;
   binsview = 0UL;
   firlen = 0UL;
   jmhz = 0.0f;
   iqfn[0] = 0;
   isize = 1UL;
   verb = 0;
   verb2 = 0;
   quietcrc = 0;
   newline = 0;
   err = 0;
   nomultipath = 0;
   cfglevel = 0.0f;
   nofec = 0;
   nocrcfec = 0;
   hasudp = 0;
   udpsock = -1L;
   bwnum = 7UL;
   globfilt = 0UL;
   globsf = 12UL;
   globcr = 0UL;
   afcspd = (-1.0f);
   hasbw = 0;
   hassf = 0;
   jpipename[0] = 0;
   judpport = 0UL;
   jsonfd = -1L;
   globdrate = 0.0f;
   insamplerate = 0.0f;
   newdem();
   dems->frames[0U].synfilter = globfilt;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            osi_NextArg(iqfn, 1024ul);
            if (iqfn[0U]==0 || iqfn[0U]=='-') Error("-i <iqfilename>", 16ul);
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if ((h[0U]=='i' && h[1U]=='1') && h[2U]=='6') isize = 2UL;
            else if (h[0U]=='u' && h[1U]=='8') isize = 1UL;
            else if ((h[0U]=='f' && h[1U]=='3') && h[2U]=='2') isize = 4UL;
            else Error("-f u8|i16|f32", 14ul);
         }
         else if (h[1U]=='s') {
            if (hassf) newdem();
            hassf = 1;
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul, &ih) || labs(ih)>12L) || labs(ih)<5L) {
               Error("-s <sf> [-]5..[-]12", 20ul);
            }
            dems->frames[0U].cfgsf = (uint32_t)labs(ih);
            dems->frames[0U].invertiq = ih<0L;
            if (!hasudp) globsf = dems->frames[0U].cfgsf;
         }
         else if (h[1U]=='a') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dems->frames[0U].afcspeed, h, 1024ul)) {
               Error("-a <afcspeed> (0.5)", 20ul);
            }
            if (!hasudp) afcspd = dems->frames[0U].afcspeed;
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &dems->frames[0U].cfgcr) || dems->frames[0U].cfgcr>8UL) || dems->frames[0U].cfgcr<4UL) {
               Error("-c <cr> 4..8", 13ul);
            }
            if (!hasudp) globcr = dems->frames[0U].cfgcr;
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (hasbw) osi_WrStrLn("Warning: only one bw = input samplerate!", 41ul);
            if (!aprsstr_StrToCard(h, 1024ul, &bwnum) || bwnum>9UL) Error("-b <bw> 0..9", 13ul);
            hasbw = 1;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &dems->frames[0U].cfgdatalen) || dems->frames[0U].cfgdatalen>255UL) || dems->frames[0U].cfgdatalen<2UL) {
               Error("-l <datalen> 2..255", 20ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&cfglevel, h, 1024ul)) Error("-g <dB>", 8ul);
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&insamplerate, h, 1024ul)) Error("-r <sampelrate-Hz>", 19ul);
         }
         else if (h[1U]=='w') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul, &firlen) || firlen>4096UL) || firlen<2UL) {
               Error("-w <firlen> 2..4096", 20ul);
            }
         }
         else if (h[1U]=='o') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&configoffsethz, h, 1024ul)) Error("-o <offset-Hz>", 15ul);
         }
         else if (h[1U]=='Z') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&pknoiseup, h, 1024ul)) {
               Error("-Z <upspeed> <downspeed> (0.01 0.9999)", 39ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&pknoisedown, h, 1024ul)) {
               Error("-Z <upspeed> <downspeed> (0.01 0.9999)", 39ul);
            }
            noisblanker = 1;
         }
         else if (h[1U]=='S') {
            /* synsquelch sf10 31 sf7 33 sf12 34 */
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dems->frames[0U].datasquelch, h, 1024ul)) {
               Error("-S <level>", 11ul);
            }
         }
         else if (h[1U]=='X') {
            osi_NextArg(h, 1024ul);
            if (!StrToHex(h, 1024ul, &dems->frames[0U].synfilter)) Error("-X <netid>", 11ul);
            if (!hasudp) globfilt = dems->frames[0U].synfilter;
         }
         else if (h[1U]=='O') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul, &dems->cfgopt) || dems->cfgopt<0L) || dems->cfgopt>1L) {
               Error("-o <onoff> 0..1 (-1)", 21ul);
            }
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&jmhz, h, 1024ul)) Error("-M <MHz>", 9ul);
         }
         else if (h[1U]=='P') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dems->frames[0U].dataratecorr, h, 1024ul)) {
               Error("-P <MHz>", 9ul);
            }
            if (!hassf) globdrate = dems->frames[0U].dataratecorr;
         }
         else if (h[1U]=='n') {
            osic_alloc((char * *) &pnotch, sizeof(struct NOTCH));
            if (pnotch==0) Error("out of memory", 14ul);
            { /* with */
               struct NOTCH * anonym = pnotch;
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&anonym->f1, h, 1024ul)) Error("-n <Hz> <Hz>", 13ul);
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&anonym->f2, h, 1024ul)) Error("-n <Hz> <Hz>", 13ul);
               if (anonym->f1>anonym->f2) {
                  rh = anonym->f1;
                  anonym->f1 = anonym->f2;
                  anonym->f2 = rh;
               }
            }
            pnotch->next = pnotches;
            pnotches = pnotch;
            complexfir = 1;
         }
         else if (h[1U]=='W') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &autonotch)) {
               Error("-W <autonotchintervall> (0 = off)", 34ul);
            }
            complexfir = 1;
         }
         else if (h[1U]=='R') nomultipath = 1;
         else if (h[1U]=='d') dems->coldet = 0UL;
         else if (h[1U]=='E') nocrcfec = 1;
         else if (h[1U]=='F') nofec = 1;
         else if (h[1U]=='C') dems->frames[0U].implicitcrc = 1;
         else if (h[1U]=='D') dems->frames[0U].nodcdlost = 1;
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='Q') {
            quietcrc = 1;
            verb = 1;
         }
         else if (h[1U]=='N') newline = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h[1U]=='q') dems->frames[0U].invertiq = 1;
         else if (h[1U]=='A') dems->frames[0U].ax25long = 1;
         else if (h[1U]=='H') allwaysascii = 1;
         else if (h[1U]=='U' || h[1U]=='L') {
            hasudp = 1;
            dems->frames[0U].udp2 = h[1U]=='L'; /* switch on axudp2 */
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &dems->frames[0U].ipnum, &dems->frames[0U].udpport)<0L) {
               Error("-U or -L ip:port number", 24ul);
            }
         }
         else if (h[1U]=='t' || h[1U]=='u') {
            hasudp = 1;
            dems->frames[0U].udpcrcok = h[1U]=='t';
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &dems->frames[0U].ipnumraw, &dems->frames[0U].udprawport)<0L) {
               Error("-u ip:port number", 18ul);
            }
         }
         else if (h[1U]=='J') {
            hasudp = 1;
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &jipnum, &judpport)<0L) Error("-J ip:port number", 18ul);
         }
         else if (h[1U]=='Y') {
            osi_NextArg(h, 1024ul);
            if (h[0U]=='d') {
               osi_NextArg(h, 1024ul);
               outdechirped = osi_OpenWrite(h, 1024ul);
               if (outdechirped<0L) Error("-Y d|f iq-filename create", 26ul);
            }
            else if (h[0U]=='f') {
               osi_NextArg(h, 1024ul);
               outfiltered = osi_OpenWrite(h, 1024ul);
               if (outfiltered<0L) Error("-Y d|f iq-filename create", 26ul);
            }
            else if (h[0U]=='b') {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &binsview)) {
                  Error("-Y b <number-of-bins>", 22ul);
               }
            }
            else Error("-Y d|f|b iq-filename", 21ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn(" Decode lora out of IQ-File/Pipe (samplerate must be exact +/-0.00001)", 71ul);
               osi_WrStrLn(" output data in udp, axudp or json, view aprs, pr, lorawan, fanet", 66ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn(" -A                 (*) enable frame chaining for ax25 longframes", 66ul);
               osi_WrStrLn(" -a <afc-speed>     (*)follow frequency drift, 0 off (0.5), on sf<9 (0)", 72ul);
               osi_WrStrLn(" -b <bandwidth>     kHz 0:7.8 1:10.4 2:15.6 3:20.8 4:31.25 5:41.7 6:62.5 7:125 8:250 9:500 (\
7)", 95ul);
               osi_WrStrLn(" -C                 (*)use crc on implicit header", 50ul);
               osi_WrStrLn(" -c <cr>            (*)coding rate and enable implicit header (4..8) (else from header)",
                88ul);
               osi_WrStrLn(" -D                 (*)if dcd lost go on decoding until frame length limit (for external FEC\
 or monitoring)", 108ul);
               osi_WrStrLn(" -d                 (*)swith off collision detection (less cpu but loose stronger frames sta\
rting in weaker", 108ul);
               osi_WrStrLn(" -E                 switch off useing crc to repair 1 chirp (crc checks 4 variants) (on)",
                89ul);
               osi_WrStrLn(" -F                 max. block energy FEC off (hamming code fec remains on) (on)", 81ul);
               osi_WrStrLn(" -f u8|i16|f32      IQ data format", 35ul);
               osi_WrStrLn(" -g <dB>            add this to measured signal level (0.0)", 60ul);
               osi_WrStrLn(" -H                 show -v data in ascii if printable else in [hex]", 69ul);
               osi_WrStrLn(" -h                 this", 25ul);
               osi_WrStrLn(" -i <file>          IQ-filename or pipe", 40ul);
               osi_WrStrLn(" -J <x.x.x.x:destport>  send demodulated data(base64) with metadata in json", 76ul);
               osi_WrStrLn(" -j <file/pipe>     write demodulated data(base64) with metadata in json to file or (unbreak\
able) pipe", 103ul);
               osi_WrStrLn(" -L <x.x.x.x:destport>  (*)as -U but AXUDPv2 with metadata for igate", 69ul);
               osi_WrStrLn("                      may be repeated with same -b but different -s  after -L", 78ul);
               osi_WrStrLn(" -l <len>           (*)fixed datalen for implicit header else guess datalen from dcd",
                85ul);
               osi_WrStrLn(" -M <MHz>           pass through rx frequency to json metadata -M 433.775", 74ul);
               osi_WrStrLn(" -N                 if verbous add empty line after data line", 62ul);
               osi_WrStrLn(" -n <[-]Hz> <[-]Hz> notchfilter baseband from-to Hz (may be repeatet)", 70ul);
               osi_WrStrLn(" -O <0..1>          (*)optimize on off else automatic on sf/bw (-1)", 68ul);
               osi_WrStrLn(" -o <Hz>            shift input iq band +-Hz", 45ul);
               osi_WrStrLn(" -P <+/-ppm>        (*)tune datarate (chirp sampelrate) or preset for auto (-a) in ppm (0)",
                 91ul);
               osi_WrStrLn(" -Q                 verbous only for frames with crc and crc ok", 64ul);
               osi_WrStrLn(" -q                 (*)invers chirps or swapped I/Q (prefer negative spread factor)", 84ul);
               osi_WrStrLn(" -R                 switch off repairing multipath or if-filter fase distortion", 80ul);
               osi_WrStrLn(" -r <Hz>            downsample input, give exact input sampelrate in Hz (off)", 78ul);
               osi_WrStrLn(" -S <level>         (*)modifiy squelch level critical for implicit header frame length guess\
 (automatic)", 105ul);
               osi_WrStrLn(" -s [-]<sf>         (*)spread factor (5..12) (12) and start a new demodulator", 78ul);
               osi_WrStrLn("                      negativ value for invers chirps or swapped I/Q", 69ul);
               osi_WrStrLn(" -t <x.x.x.x:destport>  (*)send raw frame bytes in udp frame only if has crc and crc ok",
                88ul);
               osi_WrStrLn(" -U <x.x.x.x:destport>  (*)send frame in AXUDP", 47ul);
               osi_WrStrLn(" -u <x.x.x.x:destport>  (*)send raw frame bytes in udp frame", 61ul);
               osi_WrStrLn(" -V                 very verbous", 33ul);
               osi_WrStrLn(" -v                 verbous +:no hamming or ok, -:error, h:corrected, ~:weakest chirp replac\
ed, c:try until crc ok", 115ul);
               osi_WrStrLn("                      ^:bins with maximum power sum used until fitting hamming", 79ul);
               osi_WrStrLn(" -W <n>             every n*firlen samples update notch filter 0=off, ok:50  (0)", 81ul);
               osi_WrStrLn(" -w <len>           downsample fir length else automatic (8..4096) (0)", 71ul);
               osi_WrStrLn(" -X <netid>         (*)filter network-id (sync), 1xx stops decode on wrong id so fast ready \
for new frame if set -d", 116ul);
               osi_WrStrLn("                      0 is wildcard, 20 will pass 2*, 03 for *3, 00 pass all (00)", 82ul);
               osi_WrStrLn(" -Y d|f|b <filename>  iq debug output in float32-iq dechirped or filtered", 74ul);
               osi_WrStrLn(" -Z <upspeed> <downspeed>  add pulse noise filter (noise blanker) (0.05 0.999)", 79ul);
               osi_WrStrLn("(*) may be repeated for more demodulators, to start next demodulator apply -s <sf> before ot\
her pramaeters", 107ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("example1: aprs with autonotch for birdies: rtl_sdr -f 433.775m -s 1000000 - | ./lorarx -i /d\
ev/stdin -f u8 -v -N -b 7 -s 12 -w 64 -r 1000000 -W 50", 147ul);
               osi_WrStrLn("example2: lorawan all modulations: rtl_sdr -f 869.525m -s 1000000 - | ./lorarx -i /dev/stdin\
 -f u8 -v -N -b 7 -s 12 -s 11 -s 10 -s 9 -s 8 -s 7 -s -12 -s -11 -s -10 -s -9 -s -8 -s -7 -Q -w 64 -r 1000000", 202ul);
               osi_WrStrLn("example3: fanet: rtl_sdr -f 868.215m -s 1000000 - | ./lorarx -i /dev/stdin -f u8 -v -N -b 8 \
-s 7 -Q -w 128 -r 1000000", 118ul);
               osi_WrStrLn("example4: sdrtst IQ output with FIR 125kHz in sdrcfg.txt: q 433.775 0 0 0 192000+125000,32",
                 91ul);
               osi_WrStrLn("          sdrtst -t 127.0.0.1:1234 -c sdrcfg.txt -r 250000 -s /dev/stdout -k | ", 80ul);
               osi_WrStrLn("          lorarx -i /dev/stdin -f i16 -b 7 -v -s 12 -L 127.0.0.1:2300 -s 10 -L 127.0.0.1:230\
1", 94ul);
               osi_WrStrLn("example: decode payload in json with python3: -J 127.0.0.1:5100", 64ul);
               osi_WrStrLn("import json, base64, socket", 28ul);
               osi_WrStrLn("IP=(\"0.0.0.0\",5100)", 20ul);
               osi_WrStrLn("sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)", 53ul);
               osi_WrStrLn("sock.bind(IP)", 14ul);
               osi_WrStrLn("while True:", 12ul);
               osi_WrStrLn("  data, addr=sock.recvfrom(1500)", 33ul);
               osi_WrStrLn("  obj = json.loads(data.decode())", 34ul);
               osi_WrStrLn("  print(obj)", 13ul);
               osi_WrStrLn("  try:", 7ul);
               osi_WrStrLn("    payload=base64.b64decode(obj[\"payload\"])", 45ul);
               osi_WrStrLn("    print(\"payload=\",payload)", 30ul);
               osi_WrStrLn("  except: pass", 15ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_Werr(">", 2ul);
      osi_Werr(h, 1024ul);
      osi_Werr("< use -h\012", 10ul);
      X2C_ABORT();
   }
   if (hasudp) {
      if (udpsock<0L) udpsock = openudp();
      if (udpsock<0L) Error("cannot open udp socket", 23ul);
   }
   if (insamplerate==0.0f && (autonotch>0UL || firlen)) insamplerate = X2C_DIVR(5.E+5f,(float)_cnst[bwnum]);
   downsample = X2C_DIVR(insamplerate,X2C_DIVR(5.E+5f,(float)_cnst[bwnum]));
   if (insamplerate!=0.0f && firlen==0UL) {
      /* make default fir length */
      firlen = 32UL;
      if (downsample>2.0f) {
         firlen = 64UL;
         if (downsample>4.0f) {
            firlen = 128UL;
            if (downsample>8.0f) firlen = 256UL;
         }
      }
   }
   if (firlen==0UL && complexfir) Error("enable FIR (-r) for notches (-n)", 33ul);
   if (firlen>0UL) {
      /* make it 2^n */
      i0 = 1UL;
      do {
         ++i0;
      } while (firlen>(uint32_t)(1UL<<i0));
      firlen = (uint32_t)(1UL<<i0);
   }
   samprate = (uint32_t)X2C_TRUNCC(1.6777216E+7f*downsample,0UL,X2C_max_longcard);
   if (outfiltered>=0L && firlen==0UL) Error("no FIR (-w) set so no filtert output", 37ul);
   /*- offset */
   if (configoffsethz!=0.0f && insamplerate!=0.0f) {
      offs = (X2C_DIVR(configoffsethz,insamplerate))*65536.0f*65536.0f;
      if (offs<0.0f) offs = offs+4.294967295E+9f;
      if (offs<0.0f || offs>=4.294967295E+9f) Error("-o shift outside iq band", 25ul);
      shiftstep = (uint32_t)X2C_TRUNCC(offs,0UL,X2C_max_longcard);
   }
   /*- offset */
   if (verb) {
      if (firlen) {
         osi_WrStr("FIR length=", 12ul);
         osic_WrINT32(firlen, 1UL);
         osi_WrStr(" ratio=", 8ul);
         osic_WrFixed(downsample, 6L, 1UL);
         if (configoffsethz!=0.0f) {
            osi_WrStr(" offset=", 9ul);
            osic_WrFixed(configoffsethz, 2L, 1UL);
            osi_WrStr("Hz", 3ul);
         }
         if (pnotches) {
            osi_WrStr(" notches:", 10ul);
            pnotch = pnotches;
            while (pnotch) {
               osi_WrStr(" ", 2ul);
               osic_WrFixed(pnotch->f1, 0L, 1UL);
               osi_WrStr("..", 3ul);
               osic_WrFixed(pnotch->f2, 0L, 1UL);
               pnotch = pnotch->next;
            }
         }
         osi_WrStrLn("", 1ul);
      }
   }
   if (insamplerate!=0.0f) {
      pnotch = pnotches;
      while (pnotch) {
         pnotch->f1 = X2C_DIVR(pnotch->f1*(float)firlen,insamplerate);
         pnotch->f2 = X2C_DIVR(pnotch->f2*(float)firlen,insamplerate);
         pnotch = pnotch->next;
      }
   }
   pd0 = dems;
   while (pd0) {
      if (pd0->cfgopt==0L) pd0->frames[0U].optimize = 0;
      else if (pd0->cfgopt==1L || baud(pd0->frames[0U].cfgsf, bwnum)<62.5f) pd0->frames[0U].optimize = 1;
      if (pd0->frames[0U].afcspeed<0.0f) {
         /* automatic afc on */
         if (afcspd<0.0f) {
            if (pd0->frames[0U].cfgsf<=10UL) afcspd = 0.3f;
            else afcspd = 0.5f;
         }
         if (pd0->frames[0U].cfgsf>=7UL) pd0->frames[0U].afcspeed = afcspd;
         else pd0->frames[0U].afcspeed = 0.0f;
      }
      if (dems->frames[0U].cfgsf==0UL) dems->frames[0U].cfgsf = globsf;
      if (dems->frames[0U].cfgcr==0UL) dems->frames[0U].cfgcr = globcr;
      if (pd0->frames[0U].datasquelch<0.0f) {
         pd0->frames[0U].datasquelch = 15.0f+(float)pd0->frames[0U].cfgsf*(-1.1f);
      }
      pd0->frames[0U].dataratecorr = pd0->frames[0U].dataratecorr*1.E-6f*(float)(uint32_t)
                (1UL<<pd0->frames[0U].cfgsf);
      if (verb) {
         osi_WrStr("bw=", 4ul);
         osic_WrFixed(X2C_DIVR(5.E+5f,(float)_cnst[bwnum]), 1L, 1UL);
         osi_WrStr(" cr=", 5ul);
         if (pd0->frames[0U].cfgcr==0UL) osi_WrStr("from header", 12ul);
         else osic_WrINT32(pd0->frames[0U].cfgcr, 1UL);
         osi_WrStr(" sf=", 5ul);
         osic_WrINT32(pd0->frames[0U].cfgsf, 1UL);
         osi_WrStr(" id=", 5ul);
         if (pd0->frames[0U].synfilter==0UL) osi_WrStr("Off", 4ul);
         else WrHex(pd0->frames[0U].synfilter, 2UL, 0UL);
         osi_WrStr(" optimize=", 11ul);
         osic_WrINT32((uint32_t)pd0->frames[0U].optimize, 1UL);
         osi_WrStr(" drpll=", 8ul);
         osic_WrFixed(pd0->frames[0U].afcspeed, 2L, 1UL);
         osi_WrStr(" drc=", 6ul);
         osic_WrFixed(X2C_DIVR(pd0->frames[0U].dataratecorr*1.E+6f,(float)(uint32_t)(1UL<<pd0->frames[0U].cfgsf)),
                2L, 1UL);
         osi_WrStr("ppm", 4ul);
         if (pd0->frames[0U].datasquelch>0.0f) {
            osi_WrStr(" squelch=", 10ul);
            osic_WrFixed(pd0->frames[0U].datasquelch, 1L, 1UL);
         }
         if (pd0->frames[0U].invertiq) osi_WrStr(" inverted", 10ul);
         osi_WrStrLn("", 1ul);
         if (pd0->frames[0U].cfgcr==0UL && pd0->frames[0U].cfgsf<7UL) {
            osi_WrStrLn("warning: sf<7 needs implizit header", 36ul);
         }
      }
      for (i0 = 1UL; i0<=3UL; i0++) {
         pd0->frames[i0] = pd0->frames[0U]; /* copy parameters to all demodulators */
         pd0->frames[i0].label = i0;
      } /* end for */
      pd0 = pd0->next;
   }
} /* end Parms() */


static void cleantext(char text[], uint32_t text_len, uint32_t len)
{
   uint32_t j1;
   uint32_t i0;
   i0 = 0UL;
   while (((i0<len && i0<=4UL) && ((uint8_t)text[i0]<'A' || (uint8_t)text[i0]>'Z')) && ((uint8_t)
                text[i0]<'0' || (uint8_t)text[i0]>'9')) ++i0;
   if (i0<=3UL) {
      j1 = 0UL;
      while (j1<text_len-1 && i0<len) {
         text[j1] = text[i0];
         ++j1;
         ++i0;
      }
      text[j1] = 0;
      if (j1>0UL && (uint8_t)text[j1-1UL]<'\034') text[j1-1UL] = 0;
   }
   else if (verb) osi_WrStrLn("too much junk in frame", 23ul);
} /* end cleantext() */


static void app(uint32_t * i0, uint32_t * p, char b[501], char c, int32_t v)
{
   char s[51];
   b[*p] = c;
   ++*p;
   aprsstr_IntToStr(v, 0UL, s, 51ul);
   *i0 = 0UL;
   while (s[*i0]) {
      b[*p] = s[*i0];
      ++*p;
      ++*i0;
   }
   b[*p] = ' ';
   ++*p;
} /* end app() */


static void sendaxudp2(uint32_t ipnum, uint32_t udpport, char udp2, char mon[], uint32_t mon_len,
                uint32_t dlen, int32_t txd, float level, float snrr, int32_t afc, int32_t qual)
{
   char data[501];
   char b[501];
   int32_t lev;
   int32_t snr;
   int32_t datalen;
   int32_t ret;
   uint32_t i0;
   uint32_t p;
   X2C_PCOPY((void **)&mon,mon_len);
   snr = (int32_t)X2C_TRUNCI(snrr+0.5f,X2C_min_longint,X2C_max_longint);
   if (snr>127L) snr = 127L;
   else if (snr<-127L) snr = -127L;
   lev = (int32_t)X2C_TRUNCI(level+0.5f,X2C_min_longint,X2C_max_longint);
   if (lev>127L) lev = 127L;
   else if (lev<-255L) lev = -255L;
   cleantext(mon, mon_len, dlen);
   aprsstr_mon2raw(mon, mon_len, data, 501ul, &datalen);
   if (datalen>2L) {
      if (udp2) {
         datalen -= 2L; /* remove crc */
         b[0U] = '\001';
         b[1U] = '0';
         p = 2UL;
         app(&i0, &p, b, 'T', txd);
         app(&i0, &p, b, 'V', lev);
         app(&i0, &p, b, 'S', snr);
         app(&i0, &p, b, 'A', afc);
         app(&i0, &p, b, 'Q', qual);
         b[p] = 0; /* end of axudp2 header */
         ++p;
         i0 = 0UL;
         do {
            b[p] = data[i0];
            ++p;
            ++i0;
         } while ((int32_t)i0<datalen);
         aprsstr_AppCRC(b, 501ul, (int32_t)p);
      }
      if (udpport) ret = udpsend(udpsock, b, (int32_t)(p+2UL), udpport, ipnum);
   }
   else if (verb) osi_WrStrLn("beacon encode error", 20ul);
   X2C_PFREE(mon);
} /* end sendaxudp2() */


static char b64(uint32_t c)
{
   c = c&63UL;
   if (c<26UL) return (char)(c+65UL);
   else if (c<52UL) return (char)(c+71UL);
   else if (c<62UL) return (char)((int32_t)c+(-4L));
   else if (c==62UL) return '+';
   else return '/';
   return 0;
} /* end b64() */


static void enc64(uint32_t b, uint32_t n, char s[], uint32_t s_len)
{
   uint32_t i0;
   for (i0 = n; i0<=2UL; i0++) {
      b = b*256UL;
   } /* end for */
   s[2UL] = '=';
   s[3UL] = '=';
   s[4UL] = 0;
   s[0UL] = b64(b/262144UL);
   s[1UL] = b64(b/4096UL);
   if (n>=2UL) s[2UL] = b64(b/64UL);
   if (n==3UL) s[3UL] = b64(b);
} /* end enc64() */


static void sendjson(uint32_t jipnum0, uint32_t judpport0, uint32_t id, const char text[],
                uint32_t text_len, uint32_t dlen, char hascrc, char crc0, char invert,
                uint32_t sf, uint32_t cr, uint32_t txd, uint32_t frametime, float level, float n,
                float peakn, float snr, float dre, int32_t df, int32_t qual)
{
   char h[1000];
   char s[1000];
   int32_t ret;
   uint32_t b;
   uint32_t i0;
   strncpy(s,"{",1000u);
   aprsstr_Append(s, 1000ul, "\"net\":", 7ul);
   aprsstr_IntToStr((int32_t)id, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"crc\":", 8ul);
   aprsstr_IntToStr((int32_t)((uint32_t)hascrc+(uint32_t)crc0)-1L, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"invers\":", 11ul);
   aprsstr_IntToStr((int32_t)(uint32_t)invert, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"bw\":", 7ul);
   aprsstr_FixToStr(X2C_DIVR(5.E+5f,(float)_cnst[bwnum]), 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"sf\":", 7ul);
   aprsstr_IntToStr((int32_t)sf, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"cr\":", 7ul);
   aprsstr_IntToStr((int32_t)cr, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"preamb\":", 11ul);
   aprsstr_IntToStr((int32_t)txd, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"duration\":", 13ul);
   aprsstr_IntToStr((int32_t)frametime, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"level\":", 10ul);
   aprsstr_FixToStr(level, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"afc\":", 8ul);
   aprsstr_IntToStr(df, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"dre\":", 8ul);
   aprsstr_FixToStr(dre, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"eye\":", 8ul);
   aprsstr_IntToStr(qual, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"nfloor\":", 11ul);
   aprsstr_FixToStr(n, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"pknfloor\":", 13ul);
   aprsstr_FixToStr(peakn, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"snr\":", 8ul);
   aprsstr_FixToStr(snr, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   if (jmhz!=0.0f) {
      aprsstr_Append(s, 1000ul, ",\"rxmhz\":", 10ul);
      aprsstr_FixToStr(jmhz+0.0005f, 4UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",\"ver\":\"sdr-lorarx\"", 20ul);
   aprsstr_Append(s, 1000ul, ",\"payload\":\"", 13ul);
   b = 0UL;
   i0 = 0UL;
   while (i0<dlen) {
      /* base64 encode */
      b = b*256UL+(uint32_t)(uint8_t)text[i0];
      if (i0%3UL==2UL) {
         enc64(b, 3UL, h, 1000ul);
         aprsstr_Append(s, 1000ul, h, 1000ul);
         b = 0UL;
      }
      ++i0;
   }
   if (i0%3UL) {
      enc64(b, i0%3UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, "\"}\012", 4ul);
   if (jpipename[0U]) {
      if (jsonfd<0L) {
         jsonfd = osi_OpenNONBLOCK(jpipename, 1024ul);
         if (jsonfd<0L) {
            jsonfd = osi_OpenWrite(jpipename, 1024ul); /* no file and no pipe */
         }
         else osic_Seekend(jsonfd, 0L);
      }
      if (jsonfd>=0L) osi_WrBin(jsonfd, (char *)s, 1000u/1u, aprsstr_Length(s, 1000ul));
      else osi_WrStrLn("cannot write json-file", 23ul);
   }
   if (judpport0) ret = udpsend(udpsock, s, (int32_t)aprsstr_Length(s, 1000ul), judpport0, jipnum0);
} /* end sendjson() */


static void WCh(char c)
{
   if (c!='\015') {
      if ((uint8_t)c<' ' || (uint8_t)c>='\177') osi_WrStr(".", 2ul);
      else osi_WrStr((char *) &c, 1u/1u);
   }
} /* end WCh() */


static void ShowCall(char f[], uint32_t f_len, uint32_t pos0)
{
   uint32_t e;
   uint32_t i0;
   uint32_t tmp;
   char tmp0;
   e = pos0;
   tmp = pos0+5UL;
   i0 = pos0;
   if (i0<=tmp) for (;; i0++) {
      if (f[i0]!='@') e = i0;
      if (i0==tmp) break;
   } /* end for */
   tmp = e;
   i0 = pos0;
   if (i0<=tmp) for (;; i0++) {
      WCh((char)((uint32_t)(uint8_t)f[i0]>>1));
      if (i0==tmp) break;
   } /* end for */
   i0 = (uint32_t)(uint8_t)f[pos0+6UL]>>1&15UL;
   if (i0) {
      osi_WrStr("-", 2ul);
      if (i0>=10UL) osi_WrStr((char *)(tmp0 = (char)(i0/10UL+48UL),&tmp0), 1u/1u);
      osi_WrStr((char *)(tmp0 = (char)(i0%10UL+48UL),&tmp0), 1u/1u);
   }
} /* end ShowCall() */

static uint32_t lorarx_UA = 0x63UL;

static uint32_t lorarx_DM = 0xFUL;

static uint32_t lorarx_SABM = 0x2FUL;

static uint32_t lorarx_DISC = 0x43UL;

static uint32_t lorarx_FRMR = 0x87UL;

static uint32_t lorarx_UI = 0x3UL;

static uint32_t lorarx_RR = 0x1UL;

static uint32_t lorarx_REJ = 0x9UL;

static uint32_t lorarx_RNR = 0x5UL;


static void Showctl(uint32_t com, uint32_t cmd)
{
   uint32_t cm;
   char PF[4];
   char tmp;
   osi_WrStr(" ctl ", 6ul);
   cm = (uint32_t)cmd&~0x10UL;
   if ((cm&0xFUL)==0x1UL) {
      osi_WrStr("RR", 3ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0xFUL)==0x5UL) {
      osi_WrStr("RNR", 4ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0xFUL)==0x9UL) {
      osi_WrStr("REJ", 4ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0x1UL)==0UL) {
      osi_WrStr("I", 2ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>1&7UL)),&tmp), 1u/1u);
   }
   else if (cm==0x3UL) osi_WrStr("UI", 3ul);
   else if (cm==0xFUL) osi_WrStr("DM", 3ul);
   else if (cm==0x2FUL) osi_WrStr("SABM", 5ul);
   else if (cm==0x43UL) osi_WrStr("DISC", 5ul);
   else if (cm==0x63UL) osi_WrStr("UA", 3ul);
   else if (cm==0x87UL) osi_WrStr("FRMR", 5ul);
   else WrHex(cmd, 2UL, 0UL);
   strncpy(PF,"v^-+",4u);
   if (com==0UL || com==3UL) osi_WrStr("v1", 3ul);
   else osi_WrStr((char *) &PF[(com&1UL)+2UL*(uint32_t)((0x10UL & (uint32_t)cmd)!=0)], 1u/1u);
} /* end Showctl() */


static void ShowFrame(char f[], uint32_t f_len, uint32_t len, char port)
{
   uint32_t i0;
   char d;
   char v;
   X2C_PCOPY((void **)&f,f_len);
   osi_WrStr((char *) &port, 1u/1u);
   i0 = 0UL;
   while (!((uint32_t)(uint8_t)f[i0]&1)) {
      ++i0;
      if (i0>len) {
         osi_WrStrLn(" no ax.25 (no address end mark)", 32ul);
         goto label;
      }
   }
   /* no address end mark found */
   if (i0%7UL!=6UL) {
      osi_WrStrLn(" no ax.25 (address field size not multiples of 7)", 50ul);
      goto label;
   }
   /* address end not modulo 7 error */
   osi_WrStr(":fm ", 5ul);
   ShowCall(f, f_len, 7UL);
   osi_WrStr(" to ", 5ul);
   ShowCall(f, f_len, 0UL);
   i0 = 14UL;
   v = 1;
   while (i0+6UL<len && !((uint32_t)(uint8_t)f[i0-1UL]&1)) {
      if (v) {
         osi_WrStr(" via", 5ul);
         v = 0;
      }
      osi_WrStr(" ", 2ul);
      ShowCall(f, f_len, i0);
      if ((uint32_t)(uint8_t)f[i0+6UL]>=128UL && (((uint32_t)(uint8_t)f[i0+6UL]&1) || (uint32_t)(uint8_t)
                f[i0+13UL]<128UL)) osi_WrStr("*", 2ul);
      i0 += 7UL;
   }
   Showctl((uint32_t)((0x80U & (uint8_t)(uint8_t)f[6UL])!=0)+2UL*(uint32_t)((0x80U & (uint8_t)(uint8_t)
                f[13UL])!=0), (uint32_t)(uint8_t)f[i0]);
   ++i0;
   if (i0<len) {
      osi_WrStr(" pid ", 6ul);
      WrHex((uint32_t)(uint8_t)f[i0], 2UL, 0UL);
   }
   ++i0;
   /*
     DateToStr(time(), h);  (* 2019.12.31 23:59:59 *)
   
     h[0]:=h[8]; h[8]:=h[2]; h[2]:=h[0];
     h[0]:=h[9]; h[9]:=h[3]; h[3]:=h[0];
     h[0]:="-"; h[1]:=" ";
     WrStr(" "); WrStr(h);
   */
   osi_WrStrLn("", 1ul);
   /*  IF NOT noinfo THEN */
   d = 0;
   while (i0<len) {
      if (f[i0]!='\015') {
         WCh(f[i0]);
         d = 1;
      }
      else if (d) {
         osi_WrStrLn("", 1ul);
         d = 0;
      }
      ++i0;
   }
   if (d) osi_WrStrLn("", 1ul);
   label:;
   X2C_PFREE(f);
/*  END; */
} /* end ShowFrame() */


static void decodepr(char raw[], uint32_t raw_len, uint32_t rawlen, char port, char * join,
                char axpart[], uint32_t axpart_len, char fb[], uint32_t fb_len, uint32_t * fblen)
{
   uint32_t i0;
   uint32_t tmp;
   X2C_PCOPY((void **)&raw,raw_len);
   *fblen = 0UL;
   if (rawlen>0UL) {
      if (*join) {
         if (verb) osi_WrStrLn("ax25 part 2", 12ul);
         if (rawlen+2UL<=raw_len-1) {
            aprsstr_AppCRC(raw, raw_len, (int32_t)rawlen);
            if (raw[rawlen]==axpart[253UL] && raw[rawlen+1UL]==axpart[254UL]) {
               /* join hash fits */
               for (i0 = 0UL; i0<=252UL; i0++) {
                  fb[i0] = axpart[i0];
               } /* end for */
               tmp = rawlen-1UL;
               i0 = 0UL;
               if (i0<=tmp) for (;; i0++) {
                  fb[i0+253UL] = raw[i0];
                  if (i0==tmp) break;
               } /* end for */
               *fblen = rawlen+253UL;
            }
            else if (verb) osi_WrStrLn("ax25 chain hash missmatch", 26ul);
         }
      }
      *join = 0;
      if (*fblen==0UL) {
         i0 = 0UL;
         while (i0<rawlen && !((uint32_t)(uint8_t)raw[i0]&1)) ++i0;
         if ((i0%7UL==6UL && i0>=13UL) && i0<=69UL) {
            /* is pr,  not 2 to 10 shift up calls */
            if (rawlen==255UL) {
               tmp = axpart_len-1;
               i0 = 0UL;
               if (i0<=tmp) for (;; i0++) {
                  axpart[i0] = raw[i0];
                  if (i0==tmp) break;
               } /* end for */
               *join = 1;
               if (verb) osi_WrStrLn("ax25 part 1", 12ul);
            }
            else {
               tmp = rawlen-1UL;
               i0 = 0UL;
               if (i0<=tmp) for (;; i0++) {
                  fb[i0] = raw[i0];
                  if (i0==tmp) break;
               } /* end for */
               *fblen = rawlen;
            }
         }
      }
   }
   if (*fblen>=2UL) {
      if (verb) {
         osi_WrStr("AX25:", 6ul);
         ShowFrame(fb, fb_len, *fblen-2UL, port);
      }
   }
   X2C_PFREE(raw);
} /* end decodepr() */


static void WH(const char title[], uint32_t title_len, const char text[], uint32_t text_len,
                uint32_t from, uint32_t too)
{
   uint32_t i0;
   osi_WrStr(title, title_len);
   i0 = from;
   while (i0<=too && i0<=text_len-1) {
      WrHex((uint32_t)(uint8_t)text[i0], 2UL, 0UL);
      ++i0;
   }
} /* end WH() */


static void decodelorawan(const char text[], uint32_t text_len, uint32_t textlen)
{
   uint32_t p;
   uint32_t fctl;
   uint32_t mtype;
   if (textlen>0UL) {
      mtype = (uint32_t)(uint8_t)text[0UL]/32UL; /* MHDR */
      switch (mtype) {
      case 0UL:
         osi_WrStr("Join-request", 13ul);
         break;
      case 1UL:
         osi_WrStr("Join-accept", 12ul);
         break;
      case 2UL:
         osi_WrStr("Unconfirmed Data Up", 20ul);
         break;
      case 3UL:
         osi_WrStr("Unconfirmed Data Down", 22ul);
         break;
      case 4UL:
         osi_WrStr("Confirmed Data Up", 18ul);
         break;
      case 5UL:
         osi_WrStr("Confirmed Data Down", 20ul);
         break;
      case 6UL:
         osi_WrStr("Rejoin-request", 15ul);
         break;
      default:;
         osi_WrStr("Proprietary", 12ul);
         break;
      } /* end switch */
      osi_WrStr(" RFU:", 6ul);
      osic_WrINT32((uint32_t)(uint8_t)text[0UL]/4UL&7UL, 1UL);
      osi_WrStr(" Major:", 8ul);
      osic_WrINT32((uint32_t)(uint8_t)text[0UL]&3UL, 1UL);
      if (mtype==0UL && textlen>=18UL) {
         WH(" AppEUI:", 9ul, text, text_len, 1UL, 8UL);
         WH(" DevEUI:", 9ul, text, text_len, 9UL, 16UL);
         WH(" DevNonce:", 11ul, text, text_len, 17UL, 18UL);
      }
      else if (mtype==1UL && textlen>=12UL) {
         WH(" DevNonce:", 11ul, text, text_len, 1UL, 3UL);
         WH(" NetID:", 8ul, text, text_len, 4UL, 6UL);
         WH(" DevAddr:", 10ul, text, text_len, 7UL, 10UL);
         WH(" DLSettings:", 13ul, text, text_len, 11UL, 11UL);
         WH(" RxDelay:", 10ul, text, text_len, 12UL, 12UL);
         if (textlen>34UL) WH(" CFList:", 9ul, text, text_len, 13UL, 28UL);
      }
      else if (mtype>=2UL && mtype<=5UL) {
         /* data frame */
         fctl = (uint32_t)(uint8_t)text[5UL];
         WH(" DevAddr:", 10ul, text, text_len, 1UL, 4UL);
         osi_WrStr(" ADR:", 6ul);
         osic_WrINT32(fctl/128UL, 1UL);
         if ((mtype&1)) osi_WrStr(" RFU:", 6ul);
         else osi_WrStr(" ADRACKReq:", 12ul);
         osic_WrINT32(fctl/64UL&1UL, 1UL);
         osi_WrStr(" ACK:", 6ul);
         osic_WrINT32(fctl/32UL&1UL, 1UL);
         if ((mtype&1)) osi_WrStr(" FPending:", 11ul);
         else osi_WrStr(" ClassB:", 9ul);
         osic_WrINT32(fctl/16UL&1UL, 1UL);
         osi_WrStr(" FOptsLen:", 11ul);
         osic_WrINT32(fctl&15UL, 1UL);
         osi_WrStr(" FCnt:", 7ul);
         osic_WrINT32((uint32_t)(uint8_t)text[6UL]+(uint32_t)(uint8_t)text[7UL]*256UL, 1UL);
         p = 0UL;
         if ((fctl&15UL)>0UL) {
            osi_WrStr(" FOpts:", 8ul);
            while (p<(fctl&15UL) && p+8UL<textlen) {
               WH("", 1ul, text, text_len, p+8UL, p+8UL);
               ++p;
            }
         }
         if (textlen>=6UL) {
            osi_WrStr(" FPort:", 8ul);
            osic_WrINT32((uint32_t)(uint8_t)text[p+8UL], 1UL);
            if (p+9UL<=textlen-5UL) WH(" Payload:", 10ul, text, text_len, p+9UL, textlen-5UL);
            WH(" MIC:", 6ul, text, text_len, textlen-4UL, textlen-1UL);
         }
      }
      osi_WrStrLn("", 1ul);
   }
} /* end decodelorawan() */


static void decodemeshcom4(const char text[], uint32_t text_len, uint32_t textlen)
/* MESHCOM4 */
{
   uint32_t te;
   uint32_t i0;
   uint32_t tmp;
   if (verb) {
      /*-fcs */
      i0 = 0UL;
      te = 0UL;
      while (te+2UL<textlen && (te<6UL || text[te])) {
         i0 += (uint32_t)(uint8_t)text[te];
         ++te;
      }
      i0 += (uint32_t)(uint8_t)text[te+1UL];
      i0 += (uint32_t)(uint8_t)text[te+2UL];
      /*-fcs */
      if (te+4UL<textlen && i0==(uint32_t)(uint8_t)text[te+3UL]*256UL+(uint32_t)(uint8_t)text[te+4UL]) {
         osi_WrStr("Meshcom4: FCS:Ok MID:", 22ul);
         WrHex((uint32_t)(uint8_t)text[1UL]+(uint32_t)(uint8_t)text[2UL]*256UL+(uint32_t)(uint8_t)
                text[3UL]*65536UL+(uint32_t)(uint8_t)text[4UL]*16777216UL, 8UL, 0UL);
         osi_WrStr(" MAX-HOP=", 10ul);
         osic_WrUINT32((uint32_t)(uint8_t)text[5UL]&7UL, 1UL);
         if ((0x80U & (uint8_t)(uint8_t)text[5UL])) osi_WrStr(" viaMQTT", 9ul);
         if ((0x40U & (uint8_t)(uint8_t)text[5UL])) osi_WrStr(" +Traceroute", 13ul);
         if (te+2UL<=textlen) {
            osi_WrStr(" HW-ID=", 8ul);
            osic_WrUINT32((uint32_t)(uint8_t)text[te+1UL], 1UL);
            osi_WrStr(" MOD=", 6ul);
            osic_WrUINT32((uint32_t)(uint8_t)text[te+2UL], 1UL);
         }
         if (te+6UL<textlen) {
            osi_WrStr(" FW=", 5ul);
            osic_WrUINT32((uint32_t)(uint8_t)text[te+6UL], 1UL);
            osi_WrStr(".", 2ul);
            osic_WrUINT32((uint32_t)(uint8_t)text[te+5UL], 1UL);
         }
         osi_WrStrLn("", 1ul);
         osi_WrStr("[", 2ul);
         tmp = te-1UL;
         i0 = 6UL;
         if (i0<=tmp) for (;; i0++) {
            WrChHex(text[i0]);
            if (i0==tmp) break;
         } /* end for */
         osi_WrStrLn("]", 2ul);
      }
   }
} /* end decodemeshcom4() */


static void wcsv(uint32_t n, const char s[], uint32_t s_len)
{
   uint32_t i0;
   i0 = 0UL;
   while (n>0UL) {
      if (i0>=s_len-1) return;
      if (s[i0]==',') --n;
      ++i0;
   }
   while (i0<=s_len-1 && s[i0]!=',') {
      osi_WrStr((char *) &s[i0], 1u/1u);
      ++i0;
   }
} /* end wcsv() */


static void spd(uint32_t i0, float scale)
{
   if (i0>=128UL) i0 = (i0&127UL)*5UL;
   osic_WrFixed((float)i0*scale, 1L, 1UL); /* speed */
   osi_WrStr("km/h ", 6ul);
} /* end spd() */


static void clb(char text[], uint32_t text_len, uint32_t p)
{
   uint32_t i0;
   int32_t ii;
   i0 = (uint32_t)(uint8_t)text[p];
   ii = (int32_t)(i0&127UL);
   if (ii>=64L) ii = 64L-ii;
   if (i0>=128UL) ii = ii*5L;
   osic_WrFixed((float)ii*0.1f, 1L, 1UL); /* climb */
   osi_WrStr("m/s ", 5ul);
} /* end clb() */


static void latlong(char text[], uint32_t text_len, uint32_t p, float scale)
{
   int32_t ii;
   ii = (int32_t)((uint32_t)(uint8_t)text[p+2UL]*65536UL+(uint32_t)(uint8_t)text[p+1UL]*256UL+(uint32_t)
                (uint8_t)text[p]);
   if (ii>=8388608L) ii = 8388608L-ii;
   osic_WrFixed((float)ii*scale, 5L, 1UL);
} /* end latlong() */


static void pos(char text[], uint32_t text_len, uint32_t * p)
{
   latlong(text, text_len, *p, 1.0728923030706E-5f); /* lat */
   *p += 3UL;
   osi_WrStr(",", 2ul);
   latlong(text, text_len, *p, 2.1457846061412E-5f); /* long */
   *p += 3UL;
   osi_WrStr(" ", 2ul);
} /* end pos() */


static void manufact(uint32_t b)
{
   char s[100];
   s[0] = 0;
   if (b==1UL) strncpy(s,"Skytraxx",100u);
   else if (b==3UL) strncpy(s,"BitBroker.eu",100u);
   else if (b==4UL) strncpy(s,"AirWhere",100u);
   else if (b==5UL) strncpy(s,"Windline",100u);
   else if (b==6UL) strncpy(s,"Burnair.ch",100u);
   else if (b==7UL) strncpy(s,"SoftRF",100u);
   else if (b==8UL) strncpy(s,"GXAircom",100u);
   else if (b==9UL) strncpy(s,"Airtribune",100u);
   else if (b==16UL) strncpy(s,"alfapilot",100u);
   else if (b==17UL) strncpy(s,"FANET+ (incl FLARM. Currently Skytraxx, Naviter, and Skybean)",100u);
   else if (b==10UL) strncpy(s,"FLARM",100u);
   else if (b==32UL) strncpy(s,"XC Tracer",100u);
   else if (b==224UL) strncpy(s,"OGN Tracker",100u);
   else if (b==228UL) strncpy(s,"4aviation",100u);
   else if (b==250UL) strncpy(s,"Various",100u);
   else if (b==251UL) strncpy(s,"Espressif based base stations, address is last 2bytes of MAC",100u);
   else if (b==252UL) strncpy(s,"Unregistered Devices",100u);
   else if (b==253UL) strncpy(s,"Unregistered Devices",100u);
   else if (b==254UL) strncpy(s,"[Multicast]",100u);
   osi_WrStr(s, 100ul);
} /* end manufact() */


static void srcdest(char text[], uint32_t text_len, uint32_t b)
{
   osi_WrStr("FNT", 4ul);
   WrHex((uint32_t)(uint8_t)text[b]*65536UL+(uint32_t)(uint8_t)text[b+1UL]+(uint32_t)(uint8_t)
                text[b+2UL]*256UL, 6UL, 7UL);
   manufact(b);
   osi_WrStr(" ", 2ul);
} /* end srcdest() */


static void alt25(char text[], uint32_t text_len, uint32_t p)
{
   int32_t ii;
   ii = (int32_t)(uint32_t)(uint8_t)text[p];
   if (ii>=128L) ii = 128L-ii;
   ii = (ii+109L)*125L;
   osi_WrStr("alt:", 5ul);
   osic_WrINT32((uint32_t)ii, 1UL);
   osi_WrStr("m ", 3ul);
} /* end alt25() */

#define lorarx_ATY "Other,Paraglider,Hangglider,Balloon,Glider,Powered Aircraft,Helicopter,UAV"

#define lorarx_GNDVEHICLES "Other,Walking,Vehicle,Bike,Boot,Need a ride,Landed well,Need technical support,Need medical \
help,Distress call,Distress call automatically"

#define lorarx_LANDM "Text,Line,Arrow,Area,Area Filled,Circle,Circle Filled,3D Line,3D Area,3D Cylinder"

#define lorarx_LANDLAYER "Info,Warning,Keep out,Touch down,No airspace warn zone"


static void decodefanet(const char text[], uint32_t text_len, uint32_t textlen)
/* FANET */
{
   uint32_t i0;
   uint32_t p;
   int32_t ii;
   uint32_t ss;
   uint8_t s;
   uint8_t typ;
   char unicast;
   char signature;
   char exth;
   osi_WrStr("Fanet:", 7ul);
   typ = (uint8_t)(uint8_t)text[0UL];
   exth = (0x80U & typ)!=0;
   if (exth) osi_WrStr("Ext Header ", 12ul);
   if ((0x40U & typ)) osi_WrStr("Forward ", 9ul);
   srcdest(text, text_len, 1UL);
   p = 4UL;
   if (exth) {
      s = (uint8_t)(uint8_t)text[p];
      osi_WrStr("ACK ", 5ul);
      if ((s&0xC0U)==0U) osi_WrStr("none ", 6ul);
      else if ((s&0xC0U)==0x40U) osi_WrStr("requested ", 11ul);
      else if ((s&0xC0U)==0x80U) osi_WrStr("requested via forward ", 23ul);
      else osi_WrStr("unknown ", 9ul);
      unicast = (0x20U & s)!=0;
      if (unicast) osi_WrStr("unicast ", 9ul);
      else osi_WrStr("broadcast ", 11ul);
      signature = (0x10U & s)!=0;
      if ((0x8U & s)) osi_WrStr("geobased fwd ", 14ul);
      ++p;
      if (unicast) {
         osi_WrStr("Dst:", 5ul);
         srcdest(text, text_len, p);
         p += 3UL;
      }
      if (signature) {
         osi_WrStr("Signature:", 11ul);
         WrHex((uint32_t)(uint8_t)text[p+3UL]*16777216UL+(uint32_t)(uint8_t)text[p+2UL]*65536UL+(uint32_t)
                (uint8_t)text[p+1UL]*256UL+(uint32_t)(uint8_t)text[p], 8UL, 9UL);
         p += 4UL;
      }
   }
   typ = typ&0x3FU;
   if (typ==0U) osi_WrStr("no payload ", 12ul);
   else if (typ==0x1U) {
      /* tracking */
      osi_WrStr("tracking:", 10ul);
      pos(text, text_len, &p);
      ss = (uint32_t)((uint32_t)(uint8_t)text[p+1UL]*256UL+(uint32_t)(uint8_t)text[p]);
      p += 2UL;
      if ((0x8000UL & ss)) osi_WrStr("Online Tracking ", 17ul);
      wcsv((uint32_t)(X2C_LSH(ss,32,-12)&0x7UL), "Other,Paraglider,Hangglider,Balloon,Glider,Powered Aircraft,Helicopt\
er,UAV", 75ul);
      osi_WrStr(" ", 2ul);
      i0 = (uint32_t)(ss&0x7FFUL);
      if ((0x800UL & ss)) i0 = i0*4UL;
      osic_WrINT32(i0, 1UL); /* alt */
      osi_WrStr("m ", 3ul);
      spd((uint32_t)(uint8_t)text[p], 0.5f); /* speed */
      ++p;
      clb(text, text_len, p);
      ++p;
      osic_WrINT32(((uint32_t)(uint8_t)text[p]*360UL)/256UL, 1UL); /* heading */
      osi_WrStr("deg ", 5ul);
      ++p;
      if (p<textlen) {
         i0 = (uint32_t)(uint8_t)text[p];
         ++p;
         ii = (int32_t)(i0&127UL);
         if (ii>=64L) ii = -ii;
         if (i0>=128UL) ii = ii*4L;
         osic_WrFixed((float)ii*0.25f, 1L, 1UL); /* turn rate */
         osi_WrStr("deg/s ", 7ul);
      }
      if (p<textlen) {
         i0 = (uint32_t)(uint8_t)text[p];
         ++p;
         ii = (int32_t)(i0&127UL);
         if (ii>=64L) ii = -ii;
         if (i0>=128UL) ii = ii*4L;
         osi_WrStr("QNE:", 5ul); /* QNE */
         osic_WrFixed((float)ii, 1L, 1UL);
         osi_WrStr(" ", 2ul);
      }
   }
   else if (typ==0x2U) {
      /* text */
      osi_WrStr("Message:", 9ul);
      while (p<textlen) {
         WrChHex(text[p]);
         ++p;
      }
   }
   else if (typ==0x3U) {
      /* subheader text */
      osi_WrStr("Message subheader:", 19ul);
      WrHex((uint32_t)(uint8_t)text[p], 2UL, 3UL);
      ++p;
      while (p<textlen) {
         WrChHex(text[p]);
         ++p;
      }
   }
   else if (typ==0x4U) {
      /* Service */
      s = (uint8_t)(uint8_t)text[p];
      ++p;
      pos(text, text_len, &p);
      osi_WrStr("Service:", 9ul);
      if ((0x80U & s)) osi_WrStr("Internet Gateway ", 18ul);
      if ((0x40U & s)) {
         osi_WrStr("Temperature:", 13ul);
         i0 = (uint32_t)(uint8_t)text[p];
         ++p;
         ii = (int32_t)(i0&127UL);
         if (i0>=128UL) ii = -ii;
         osic_WrFixed((float)ii*0.5f, 1L, 1UL);
         osi_WrStr("C ", 3ul);
      }
      if ((0x20U & s)) {
         /* wind */
         osi_WrStr("Wind:", 6ul);
         osic_WrINT32(((uint32_t)(uint8_t)text[p]*360UL)/256UL, 1UL); /* heading */
         osi_WrStr("deg ", 5ul);
         ++p;
         spd((uint32_t)(uint8_t)text[p], 0.2f); /* speed */
         ++p;
         osi_WrStr("Gusts:", 7ul);
         spd((uint32_t)(uint8_t)text[p], 0.2f); /* gust */
         ++p;
      }
      if ((0x10U & s)) {
         osi_WrStr("Humidity:", 10ul);
         osic_WrFixed((float)(uint32_t)(uint8_t)text[p]*0.4f, 1L, 1UL);
         osi_WrStr("% ", 3ul);
         ++p;
      }
      if ((0x8U & s)) {
         osi_WrStr("Barometric pressure normailized ", 33ul);
         osic_WrFixed((float)((uint32_t)(uint8_t)text[p+1UL]*256UL+(uint32_t)(uint8_t)text[p])*0.1f+430.0f,
                1L, 1UL);
         osi_WrStr("hPa ", 5ul);
         p += 2UL;
      }
      if ((0x4U & s)) osi_WrStr("Support for Remote Configuration ", 34ul);
      if ((0x2U & s)) osi_WrStr("State of Charge ", 17ul);
      if ((0x1U & s)) osi_WrStr("Extended Header ", 17ul);
   }
   else if (typ==0x9U) {
      /* Thermal */
      osi_WrStr("Thermal:", 9ul);
      pos(text, text_len, &p);
      ss = (uint32_t)((uint32_t)(uint8_t)text[p+1UL]*256UL+(uint32_t)(uint8_t)text[p]);
      p += 2UL;
      i0 = (uint32_t)(ss&0x7FFUL);
      if ((0x800UL & ss)) i0 = i0*4UL;
      osic_WrINT32((uint32_t)(ss&0x7FFUL), 1UL); /* alt */
      osi_WrStr("m ", 3ul);
      clb(text, text_len, p);
      ++p;
      spd((uint32_t)(uint8_t)text[p], 0.5f); /* speed */
      ++p;
      osic_WrINT32(((uint32_t)(uint8_t)text[p]*360UL)/256UL, 1UL); /* heading */
      osi_WrStr("deg ", 5ul);
      ++p;
      i0 = (uint32_t)(X2C_LSH(ss,32,-12)&0x7UL);
      osi_WrStr("qual:", 6ul); /* quality */
      osic_WrINT32(i0*14UL+i0/3UL, 1UL);
      osi_WrStr("% ", 3ul);
   }
   else if (typ==0xAU) {
      /* hw-info */
      osi_WrStr("HW-Info ", 9ul);
   }
   else if (typ==0x7U) {
      /* */
      /* ground track */
      osi_WrStr("Ground Track:", 14ul);
      pos(text, text_len, &p);
      s = (uint8_t)(uint32_t)(uint8_t)text[p];
      ++p;
      if ((0x1U & s)) osi_WrStr("online ", 8ul);
      wcsv((uint32_t)(X2C_LSH(s,8,-4)&0xFU), "Other,Walking,Vehicle,Bike,Boot,Need a ride,Landed well,Need technical s\
upport,Need medical help,Distress call,Distress call automatically", 139ul);
   }
   else if (typ==0x6U) {
      /* remote config */
      osi_WrStr("Remote configuration:", 22ul);
      i0 = (uint32_t)(uint8_t)text[p];
      ++p;
      if (i0==0UL) {
         osi_WrStr("Acknowledge configuration ", 27ul);
         WrHex((uint32_t)(uint8_t)text[p], 2UL, 3UL);
         ++p;
      }
      else if (i0==1UL) {
         osi_WrStr("Request ", 9ul);
         WrHex((uint32_t)(uint8_t)text[p], 2UL, 3UL);
         ++p;
      }
      else if (i0==2UL) {
         pos(text, text_len, &p);
         alt25(text, text_len, p);
         ++p;
         osic_WrINT32(((uint32_t)(uint8_t)text[p]*360UL)/256UL, 1UL); /* heading */
         osi_WrStr("deg ", 5ul);
         ++p;
      }
      else if (i0>=4UL && i0<=8UL) {
         osi_WrStr("Geofence ", 10ul);
         alt25(text, text_len, p);
         ++p;
      }
      else if (i0>=9UL && i0<=33UL) {
         /*- */
         osi_WrStr("Broadcast Reply ", 17ul);
      }
   }
   else if (typ==0x5U) {
      /*- */
      /* remote config */
      osi_WrStr("Landmarks ", 11ul);
      s = (uint8_t)(uint32_t)(uint8_t)text[p];
      ++p;
      wcsv((uint32_t)(s&0xFU), "Text,Line,Arrow,Area,Area Filled,Circle,Circle Filled,3D Line,3D Area,3D Cylinder",
                82ul);
      osi_WrStr(" ", 2ul);
      s = (uint8_t)(uint32_t)(uint8_t)text[p];
      ++p;
      wcsv((uint32_t)(s&0xFU), "Info,Warning,Keep out,Touch down,No airspace warn zone", 55ul);
      osi_WrStr(" ", 2ul);
      if ((0x10U & s)) ++p;
   }
   /*- */
   /*- */
   osi_WrStrLn("", 1ul);
} /* end decodefanet() */


static float db(float r)
{
   return ln0(r)*4.342944819f;
} /* end db() */


static void frameout(struct FFRAME * frame, const char finf[], uint32_t finf_len, char hascrc,
                char crc0, char dcdlost, char opt, uint32_t cr, char text[],
                uint32_t text_len)
{
   uint32_t td;
   uint32_t blocksize;
   uint32_t axlen;
   uint32_t i0;
   int32_t truedf;
   int32_t ret;
   int32_t qual;
   int32_t frametime;
   int32_t txd;
   float minlev;
   float maxlev;
   float cor;
   float drift;
   float minn;
   float maxn;
   float n;
   float snrr;
   float level;
   char s[512];
   uint32_t tmp;
   X2C_PCOPY((void **)&text,text_len);
   if (frame->dlen>0UL && frame->dlen<text_len-1) {
      blocksize = (uint32_t)(1UL<<frame->cfgsf);
      snrr = 0.0f;
      maxn = 0.0f;
      minn = 0.0f;
      maxlev = 0.0f;
      minlev = 0.0f;
      n = 0.0f;
      if (frame->noissum!=0.0f) {
         snrr = db(X2C_DIVR(X2C_DIVR(frame->sigsum,frame->noissum)-1.0f,(float)blocksize));
      }
      qual = 100L;
      level = 0.0f;
      cor = 0.0f;
      if (isize==1UL) cor = (-42.0f);
      else if (isize==2UL) cor = (-90.3f);
      cor = (cfglevel+cor)-(float)frame->cfgsf*6.0206f; /* repair level from simplified fft */
      if (frame->cnt>1UL && frame->dcnt>1UL) {
         level = db(X2C_DIVR(frame->sigsum,(float)(frame->dcnt+1UL)))+cor;
         maxlev = db(frame->sigmax)+cor;
         minlev = db(frame->sigmin)+cor;
         n = db(X2C_DIVR(frame->noissum,(float)(frame->dcnt+1UL)))+cor;
         maxn = db(frame->noismax)+cor;
         minn = db(frame->noismin)+cor;
         qual -= (int32_t)X2C_TRUNCI((X2C_DIVR(frame->eye,(float)frame->cnt))*200.0f,X2C_min_longint,
                X2C_max_longint);
      }
      if (qual<0L) qual = 0L;
      if (frame->cfgcr) td = 0UL;
      else td = 56UL/frame->cfgsf;
      txd = (int32_t)X2C_TRUNCI(X2C_DIVR((float)(frame->txdel+td)*1000.0f,baud(frame->cfgsf, bwnum)),
                X2C_min_longint,X2C_max_longint);
      frametime = (int32_t)X2C_TRUNCI(X2C_DIVR((float)frame->cnt*1000.0f,baud(frame->cfgsf, bwnum)),
                X2C_min_longint,X2C_max_longint);
      drift = 0.0f;
      if (frame->cnt>3UL) {
         drift = (X2C_DIVR(frame->fc,(float)frame->cnt*(float)(uint32_t)(1UL<<frame->cfgsf)))*1.E+6f;
      }
      if (frame->invertiq) truedf = frame->df;
      else truedf = -frame->df;
      if (verb && (!quietcrc || hascrc && crc0)) {
         aprsstr_TimeToStr(osic_time()%86400UL, s, 512ul);
         osi_WrStr(s, 512ul);
         osi_WrStr(" ", 2ul);
         osic_WrINT32(frame->cfgsf, 1UL);
         if (frame->invertiq) osi_WrStr("-", 2ul);
         else osi_WrStr(":", 2ul);
         osic_WrINT32(frame->label, 1UL);
         osi_WrStr(":", 2ul);
         osi_WrStr("id:", 4ul);
         WrHex(frame->idfound, 2UL, 0UL);
         osi_WrStr(" cr:", 5ul);
         osic_WrINT32(cr, 1UL);
         osi_WrStr(" len:", 6ul);
         osic_WrINT32(frame->dlen, 1UL);
         if (dcdlost) osi_WrStr(" dcd-lost", 10ul);
         osi_WrStr(" crc:", 6ul);
         if (hascrc) {
            if (crc0) osi_WrStr("ok", 3ul);
            else osi_WrStr("err", 4ul);
         }
         else osi_WrStr("no", 3ul);
         osi_WrStr(" lev:", 6ul);
         osic_WrFixed(level, 1L, 1UL);
         osi_WrStr("(", 2ul);
         osic_WrFixed(minlev, 1L, 1UL);
         osi_WrStr("/", 2ul);
         osic_WrFixed(maxlev, 1L, 1UL);
         osi_WrStr(")dB snr:", 9ul);
         osic_WrFixed(snrr, 1L, 1UL);
         osi_WrStr("dB nf:", 7ul);
         osic_WrFixed(n, 1L, 1UL);
         osi_WrStr("(", 2ul);
         osic_WrFixed(minn, 1L, 1UL);
         osi_WrStr("/", 2ul);
         osic_WrFixed(maxn, 1L, 1UL);
         osi_WrStr(")dB txd:", 9ul);
         osic_WrINT32((uint32_t)txd, 1UL);
         osi_WrStr(" t:", 4ul);
         osic_WrINT32((uint32_t)frametime, 1UL);
         osi_WrStr(" q:", 4ul);
         osic_WrINT32((uint32_t)qual, 1UL);
         osi_WrStr("%", 2ul);
         if (frame->fasecorrs) {
            osi_WrStr(" fc:", 5ul);
            osic_WrINT32(frame->fasecorrs, 1UL);
         }
         osi_WrStr(" afc:", 6ul);
         osic_WrINT32((uint32_t)truedf, 1UL);
         osi_WrStr("Hz", 3ul);
         osi_WrStr(" dre:", 6ul);
         osic_WrFixed(drift, 1L, 1UL);
         osi_WrStr("ppm", 4ul);
         shownotches();
         osi_WrStrLn("", 1ul);
         osi_WrStr("[", 2ul);
         osi_WrStr(finf, finf_len);
         osi_WrStr("]", 2ul);
         tmp = frame->dlen-1UL;
         i0 = 0UL;
         if (i0<=tmp) for (;; i0++) {
            if (allwaysascii || frame->idfound==18UL) WrChHex(text[i0]);
            else WrHex((uint32_t)(uint8_t)text[i0], 2UL, 0UL);
            if (i0==tmp) break;
         } /* end for */
         osi_WrStrLn("", 1ul);
      }
      if (((frame->synfilter&255UL)/16UL==0UL || frame->idfound/16UL==(frame->synfilter&255UL)/16UL)
                && ((frame->synfilter&15UL)==0UL || (frame->idfound&15UL)==(frame->synfilter&15UL))) {
         if ((!frame->ax25long && frame->udprawport) && (!frame->udpcrcok || (hascrc && crc0) && !dcdlost)) {
            ret = udpsend(udpsock, text, (int32_t)frame->dlen, frame->udprawport, frame->ipnumraw);
         }
         if (((frame->udpport && hascrc) && crc0) && !dcdlost) {
            sendaxudp2(frame->ipnum, frame->udpport, frame->udp2, text, text_len, frame->dlen, txd, level, snrr, truedf,
                 qual);
         }
         if (judpport || jpipename[0U]) {
            sendjson(jipnum, judpport, frame->idfound, text, text_len, frame->dlen, hascrc, crc0, frame->invertiq,
                frame->cfgsf, cr, (uint32_t)txd, (uint32_t)frametime, level, n, maxn, snrr, drift, truedf, qual);
         }
      }
      else if (verb && (!quietcrc || hascrc && crc0)) {
         osi_WrStr("frame deleted, wrong sync word ", 32ul);
         WrHex(frame->idfound, 2UL, 0UL);
         osi_WrStrLn("", 1ul);
      }
      if (hascrc && crc0) {
         if (frame->idfound==52UL) decodelorawan(text, text_len, frame->dlen);
         if (frame->idfound==18UL) {
            decodepr(text, text_len, frame->dlen, '0', &frame->axjoin, frame->axpart, 255ul, s, 512ul, &axlen); /* PR */
            if (frame->ax25long && axlen>0UL) {
               ret = udpsend(udpsock, s, (int32_t)axlen, frame->udprawport, frame->ipnumraw);
            }
         }
         if (frame->idfound==241UL) decodefanet(text, text_len, frame->dlen);
         if (text[0UL]=='!' || text[0UL]==':') {
            /* & (text[frame.dlen-8]=0C) */
            decodemeshcom4(text, text_len, frame->dlen); /* MESHCOM4 */
         }
      }
      if ((newline && verb) && (!quietcrc || hascrc && crc0)) {
         osi_WrStrLn("", 1ul);
      }
   }
   X2C_PFREE(text);
} /* end frameout() */


static void deint(uint32_t sf, uint32_t cr, const uint16_t rb[], uint32_t rb_len, uint8_t hb[],
                uint32_t hb_len)
{
   uint32_t j1;
   uint32_t i0;
   uint32_t tmp;
   uint32_t tmp0;
   memset((char *)hb,(char)0,hb_len);
   tmp = sf-1UL;
   j1 = 0UL;
   if (j1<=tmp) for (;; j1++) {
      tmp0 = cr-1UL;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         if (X2C_IN(j1,16,rb[i0])) hb[(j1+i0)%sf] |= (1U<<i0);
         if (i0==tmp0) break;
      } /* end for */
      if (j1==tmp) break;
   } /* end for */
} /* end deint() */


static uint32_t crc(const char b[], uint32_t b_len, uint32_t from, uint32_t len, uint32_t ini)
{
   int32_t ic;
   int32_t i0;
   uint16_t poli;
   uint16_t crc0;
   uint8_t d;
   int32_t tmp;
   crc0 = (uint16_t)ini;
   poli = 0x1021U;
   tmp = (int32_t)len-1L;
   i0 = (int32_t)from;
   if (i0<=tmp) for (;; i0++) {
      d = (uint8_t)(uint8_t)b[i0];
      for (ic = 0L; ic<=7L; ic++) {
         if (((0x8000U & crc0)!=0)!=X2C_IN(7L-ic,8,d)) crc0 = X2C_LSH(crc0,16,1)^poli;
         else crc0 = X2C_LSH(crc0,16,1);
      } /* end for */
      if (i0==tmp) break;
   } /* end for */
   return (uint32_t)(uint16_t)crc0;
} /* end crc() */


static char headcrc(const uint8_t b[], uint32_t b_len)
{
   return (X2C_LSH(b[3UL],8,4)&0x10U|b[4UL]&0xFU)==((X2C_LSH(b[0UL],8,1)^X2C_LSH(b[0UL],8,2)^X2C_LSH(b[0UL],8,
                3)^X2C_LSH(b[0UL],8,4))&0x10U|(b[0UL]^b[1UL]^X2C_LSH(b[1UL],8,1)^X2C_LSH(b[1UL],8,2)^X2C_LSH(b[2UL],8,
                3))&0x8U|(b[0UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[1UL],8,2)^X2C_LSH(b[2UL],8,
                1))&0x4U|(b[0UL]^b[2UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[1UL]^b[2UL],8,
                1))&0x2U|(b[0UL]^b[2UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[2UL],8,-3)^X2C_LSH(b[2UL],8,-2))&0x1U);
} /* end headcrc() */


static void trycrc(NIBBS nibbs, const uint8_t e[], uint32_t e_len, uint32_t * ec, uint32_t * try0,
                uint32_t sf)
/* if only 1 block has false parity, modify 1 of 4 chirps to parity ok until good crc */
{
   uint32_t i0;
   uint32_t tmp;
   tmp = sf-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      nibbs[*ec+i0] = e[i0]&0xFU;
      if ((0x80U & e[i0])==0) nibbs[*ec+i0] = nibbs[*ec+i0]^(1U<<*try0);
      if (i0==tmp) break;
   } /* end for */
   ++*try0;
} /* end trycrc() */

static uint8_t _cnst2[256] = {128U,192U,192U,3U,192U,5U,6U,135U,192U,9U,10U,139U,12U,141U,142U,15U,128U,1U,2U,199U,4U,
                199U,199U,135U,8U,137U,138U,11U,140U,13U,14U,199U,128U,1U,2U,131U,4U,133U,206U,7U,8U,137U,206U,11U,206U,
                13U,142U,206U,0U,201U,130U,3U,132U,5U,6U,135U,201U,137U,10U,201U,12U,201U,142U,15U,128U,1U,2U,203U,4U,
                133U,134U,7U,8U,203U,203U,139U,140U,13U,14U,203U,0U,129U,130U,3U,204U,5U,6U,135U,204U,9U,10U,139U,140U,
                204U,204U,15U,0U,197U,130U,3U,197U,133U,6U,197U,136U,9U,10U,139U,12U,197U,142U,15U,194U,1U,130U,194U,4U,
                133U,194U,7U,8U,137U,194U,11U,140U,13U,14U,143U,128U,1U,2U,131U,4U,205U,134U,7U,8U,205U,138U,11U,205U,
                141U,14U,205U,0U,129U,202U,3U,132U,5U,6U,135U,202U,9U,138U,202U,12U,141U,202U,15U,0U,195U,195U,131U,
                132U,5U,6U,195U,136U,9U,10U,195U,12U,141U,142U,15U,196U,1U,2U,131U,132U,196U,196U,7U,8U,137U,138U,11U,
                196U,13U,14U,143U,0U,129U,198U,3U,198U,5U,134U,198U,136U,9U,10U,139U,12U,141U,198U,15U,193U,129U,2U,
                193U,4U,193U,134U,7U,8U,193U,138U,11U,140U,13U,14U,143U,200U,1U,2U,131U,4U,133U,134U,7U,136U,200U,200U,
                11U,200U,13U,14U,143U,0U,129U,130U,3U,132U,5U,6U,207U,136U,9U,10U,207U,12U,207U,207U,143U};
static uint8_t _cnst1[128] = {128U,192U,192U,131U,192U,205U,134U,135U,192U,205U,138U,139U,205U,141U,142U,205U,128U,
                129U,202U,199U,132U,199U,199U,135U,202U,137U,138U,202U,140U,141U,202U,199U,128U,195U,195U,131U,132U,
                133U,206U,195U,136U,137U,206U,195U,206U,141U,142U,206U,196U,201U,130U,131U,132U,196U,196U,135U,201U,
                137U,138U,201U,196U,201U,142U,143U,128U,129U,198U,203U,198U,133U,134U,198U,136U,203U,203U,139U,140U,
                141U,198U,203U,193U,129U,130U,193U,204U,193U,134U,135U,204U,193U,138U,139U,140U,204U,204U,143U,200U,
                197U,130U,131U,197U,133U,134U,197U,136U,200U,200U,139U,200U,197U,142U,143U,194U,129U,130U,194U,132U,
                133U,194U,207U,136U,137U,194U,207U,140U,207U,207U,143U};
static uint8_t _cnst0[64] = {128U,1U,2U,3U,4U,5U,134U,7U,8U,9U,10U,139U,12U,141U,14U,15U,0U,129U,2U,3U,4U,5U,6U,135U,
                8U,9U,138U,11U,140U,13U,14U,15U,0U,1U,2U,131U,4U,133U,6U,7U,136U,9U,10U,11U,12U,13U,142U,15U,0U,1U,130U,
                3U,132U,5U,6U,7U,8U,137U,10U,11U,12U,13U,14U,143U};

static void chkhamm(NIBBBLOCK hn, uint32_t cr, uint32_t sf, char * hamok, char * hamcorr)
{
   uint8_t s;
   uint8_t b;
   uint32_t i0;
   uint32_t tmp;
   *hamok = 1;
   *hamcorr = 0;
   if (cr>=5UL) {
      tmp = sf-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         /* repair bits */
         s = hn[i0];
         if (cr==5UL) {
            if ((0x1U & (s^X2C_LSH(s,8,-1)^X2C_LSH(s,8,-2)^X2C_LSH(s,8,-3)^X2C_LSH(s,8,-4)))==0) {
               hn[i0] |= 0x80U;
            }
         }
         else {
            b = (uint8_t)s;
            if (cr==6UL) b = _cnst0[b];
            else if (cr==7UL) b = _cnst1[b];
            else b = _cnst2[b];
            hn[i0] = (uint8_t)b;
            if ((0x40U & hn[i0])) *hamcorr = 1;
         }
         if ((0x80U & hn[i0])==0) *hamok = 0;
         if (i0==tmp) break;
      } /* end for */
   }
} /* end chkhamm() */


static uint16_t gray(uint16_t x)
{
   return x^X2C_LSH(x,16,-1);
} /* end gray() */

/*
PROCEDURE fecneighbour(VAR bintab:BINTAB; cr,sf:CARDINAL);
VAR i,j, j1, j2, m, fi, n1, n2:CARDINAL;
    c,c1,c2:SET16;
    f:REAL;
    d0,d1,d2:ARRAY[0..7] OF SET16;
    hn0,hn1,hn2,hnh:NIBBBLOCK;
    dir:ARRAY[0..7] OF INTEGER;
    hamok, hamcorr:BOOLEAN;
    binh:BINTAB;
BEGIN
  binh:=bintab;
  m:=CAST(CARDINAL,BITSET{sf});
  FOR i:=0 TO cr-1 DO
    f:=bintab[i].b[0].freq;
    fi:=TRUNC(f);
    bintab[i].b[0].bn:=fi;
    c1:=CAST(SET16,(fi+1) MOD m);
    c2:=CAST(SET16,(fi+m-1) MOD m);
    dir[i]:=1;
    IF f-FLOAT(fi)<0.5 THEN c:=c1; c1:=c2; c2:=c; dir[i]:=-1 END;
    c:=CAST(SET16, fi);
    d0[i]:=gray(c);  
    d1[i]:=gray(c)/gray(c1);
    d2[i]:=gray(c)/gray(c2);

WrInt(CAST(CARDINAL,c),4); WrStr(" ");
FOR j:=0 TO sf-1 DO WrInt(ORD(j IN d0[i]),1) END; WrStr(" ");
FOR j:=0 TO sf-1 DO WrInt(ORD(j IN d1[i]),1) END; WrStr(" ");
FOR j:=0 TO sf-1 DO WrInt(ORD(j IN d2[i]),1) END; WrStr(" ");
WrStrLn("");

  END; 
  deint(sf, cr, d0, hn0);
  deint(sf, cr, d1, hn1);
  deint(sf, cr, d2, hn2);

FOR i:=0 TO sf-1 DO
 FOR j:=0 TO cr-1 DO
   WrInt(ORD(j IN hn1[i]),1);
 END;
 WrStr("-");
END;
WrStrLn("");
FOR i:=0 TO sf-1 DO
 FOR j:=0 TO cr-1 DO
   WrInt(ORD(j IN hn2[i]),1);
 END;
 WrStr("+");
END;
WrStrLn("");

  hnh:=hn0;
  chkhamm(hnh, cr, sf, hamok, hamcorr);

FOR i:=0 TO sf-1 DO WrInt(ORD(7 IN hnh[i]),1) END; WrStrLn("=errs");
WrInt(ORD(hamok),1); WrStrLn("=ok1 ");
  FOR i:=0 TO sf-1 DO
    n1:=0;
    n2:=0;
    IF NOT (7 IN hnh[i]) THEN
      FOR j:=0 TO cr-1 DO
        IF j IN hn1[i] THEN INC(n1); j1:=j END;
        IF j IN hn2[i] THEN INC(n2); j2:=j END;
      END;
--      IF (n1=1) & (n2<=1) THEN 
      IF ((n1=1) OR (n1=2)) & (n2<=1) THEN 
        bintab[j1].b[0].bn:=VAL(CARDINAL, VAL(INTEGER, bintab[j1].b[0].bn+m)+dir[j1]) MOD m;
        bintab[j1].b[0].freq:=FLOAT(bintab[j1].b[0].bn)+0.5;
        hn0[i]:=hn0[i]/SET8{j1};
WrInt(i,3); WrInt(dir[j1],3);WrInt(j1,3);WrStr("b1 ");
--      ELSIF (n1=0) & (n2=1) THEN
      ELSIF (n1=0) & ((n2=1) OR (n2=1)) THEN
        bintab[j2].b[0].bn:=VAL(CARDINAL, VAL(INTEGER, bintab[j2].b[0].bn+m)+dir[j2]) MOD m;
        bintab[j2].b[0].freq:=FLOAT(bintab[j2].b[0].bn)+0.5;
        hn0[i]:=hn0[i]/SET8{j2};
WrInt(i,3); WrInt(dir[j2],3);WrInt(j2,3);WrStr("b2 ");
      END;
    END;
  END; 
  chkhamm(hn0, cr, sf, hamok, hamcorr);
  IF NOT hamok THEN bintab:=binh END;
WrInt(ORD(hamok),1); WrStrLn("=ok2 --------------------");

END fecneighbour;
*/

static void fecneighbour(BINTAB bintab, uint32_t cr, uint32_t sf)
{
   uint32_t n2;
   uint32_t n1;
   uint32_t fi;
   uint32_t m;
   uint32_t j2;
   uint32_t j1;
   uint32_t j3;
   uint32_t i0;
   uint16_t c2;
   uint16_t c1;
   uint16_t c;
   float f;
   uint16_t d2[8];
   uint16_t d1[8];
   uint16_t d0[8];
   NIBBBLOCK hnh;
   NIBBBLOCK hn2;
   NIBBBLOCK hn1;
   NIBBBLOCK hn0;
   char hamcorr;
   char hamok;
   BINTAB binh;
   uint32_t tmp;
   uint32_t tmp0;
   memcpy(binh,bintab,sizeof(BINTAB));
   m = (uint32_t)(1UL<<sf);
   tmp = cr-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      f = bintab[i0].b[0U].freq;
      fi = (uint32_t)X2C_TRUNCC(f,0UL,X2C_max_longcard);
      bintab[i0].b[0U].bn = fi;
      c1 = (uint16_t)((fi+1UL)%m);
      c2 = (uint16_t)(((fi+m)-1UL)%m);
      c = (uint16_t)fi;
      d0[i0] = gray(c);
      d1[i0] = gray(c)^gray(c1);
      d2[i0] = gray(c)^gray(c2);
      if (i0==tmp) break;
   } /* end for */
   /*
   WrInt(CAST(CARDINAL,c),4); WrStr(" ");
   FOR j:=0 TO sf-1 DO WrInt(ORD(j IN d0[i]),1) END; WrStr(" ");
   FOR j:=0 TO sf-1 DO WrInt(ORD(j IN d1[i]),1) END; WrStr(" ");
   FOR j:=0 TO sf-1 DO WrInt(ORD(j IN d2[i]),1) END; WrStr(" ");
   WrStrLn("");
   */
   deint(sf, cr, d0, 8ul, hn0, 12ul);
   deint(sf, cr, d1, 8ul, hn1, 12ul);
   deint(sf, cr, d2, 8ul, hn2, 12ul);
   /*
   FOR i:=0 TO sf-1 DO
    FOR j:=0 TO cr-1 DO
      WrInt(ORD(j IN hn0[i]),1);
    END;
    WrStr(".");
   END;
   WrStrLn("");
   FOR i:=0 TO sf-1 DO
    FOR j:=0 TO cr-1 DO
      WrInt(ORD(j IN hn1[i]),1);
    END;
    WrStr("-");
   END;
   WrStrLn("");
   FOR i:=0 TO sf-1 DO
    FOR j:=0 TO cr-1 DO
      WrInt(ORD(j IN hn2[i]),1);
    END;
    WrStr("+");
   END;
   WrStrLn("");
   */
   memcpy(hnh,hn0,12u);
   chkhamm(hnh, cr, sf, &hamok, &hamcorr);
   /*
   FOR i:=0 TO sf-1 DO WrInt(ORD(7 IN hnh[i]),1) END; WrStrLn("=errs");
   WrInt(ORD(hamok),1); WrStrLn("=ok1 ");
   */
   tmp = sf-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      n1 = 0UL;
      n2 = 0UL;
      if ((0x80U & hnh[i0])==0) {
         /* wrong hamm column */
         tmp0 = cr-1UL;
         j3 = 0UL;
         if (j3<=tmp0) for (;; j3++) {
            if (X2C_IN(j3,8,hn1[i0])) {
               ++n1;
               j1 = j3;
            }
            if (X2C_IN(j3,8,hn2[i0])) {
               ++n2;
               j2 = j3;
            }
            if (j3==tmp0) break;
         } /* end for */
         if (n1==1UL) {
            bintab[j1].b[0U].bn = (uint32_t)((int32_t)(bintab[j1].b[0U].bn+m)+1L)%m;
            bintab[j1].b[0U].freq = (float)bintab[j1].b[0U].bn+0.5f;
            hn0[i0] = hn0[i0]^(1U<<j1);
         }
         else if (n2==1UL) {
            /*WrInt(i,3); WrInt(j1,3);WrStr("b1 "); */
            /*      ELSIF (n1=0) & (n2=1) THEN */
            bintab[j2].b[0U].bn = (uint32_t)((int32_t)(bintab[j2].b[0U].bn+m)-1L)%m;
            bintab[j2].b[0U].freq = (float)bintab[j2].b[0U].bn+0.5f;
            hn0[i0] = hn0[i0]^(1U<<j2);
         }
      }
      if (i0==tmp) break;
   } /* end for */
   /*WrInt(i,3); WrInt(j2,3);WrStr("b2 "); */
   /*
   FOR i:=0 TO sf-1 DO
    FOR j:=0 TO cr-1 DO
      WrInt(ORD(j IN hn0[i]),1);
    END;
    WrStr(":");
   END;
   WrStrLn("");
   */
   chkhamm(hn0, cr, sf, &hamok, &hamcorr);
   if (!hamok) memcpy(bintab,binh,sizeof(BINTAB));
/*WrInt(ORD(hamok),1); WrStrLn("=ok2 --------------------"); */
} /* end fecneighbour() */

static uint8_t _cnst3[255] = {255U,254U,252U,248U,240U,225U,194U,133U,11U,23U,47U,94U,188U,120U,241U,227U,198U,141U,
                26U,52U,104U,208U,160U,64U,128U,1U,2U,4U,8U,17U,35U,71U,142U,28U,56U,113U,226U,196U,137U,18U,37U,75U,
                151U,46U,92U,184U,112U,224U,192U,129U,3U,6U,12U,25U,50U,100U,201U,146U,36U,73U,147U,38U,77U,155U,55U,
                110U,220U,185U,114U,228U,200U,144U,32U,65U,130U,5U,10U,21U,43U,86U,173U,91U,182U,109U,218U,181U,107U,
                214U,172U,89U,178U,101U,203U,150U,44U,88U,176U,97U,195U,135U,15U,31U,62U,125U,251U,246U,237U,219U,183U,
                111U,222U,189U,122U,245U,235U,215U,174U,93U,186U,116U,232U,209U,162U,68U,136U,16U,33U,67U,134U,13U,27U,
                54U,108U,216U,177U,99U,199U,143U,30U,60U,121U,243U,231U,206U,156U,57U,115U,230U,204U,152U,49U,98U,197U,
                139U,22U,45U,90U,180U,105U,210U,164U,72U,145U,34U,69U,138U,20U,41U,82U,165U,74U,149U,42U,84U,169U,83U,
                167U,78U,157U,59U,119U,238U,221U,187U,118U,236U,217U,179U,103U,207U,158U,61U,123U,247U,239U,223U,191U,
                126U,253U,250U,244U,233U,211U,166U,76U,153U,51U,102U,205U,154U,53U,106U,212U,168U,81U,163U,70U,140U,24U,
                48U,96U,193U,131U,7U,14U,29U,58U,117U,234U,213U,170U,85U,171U,87U,175U,95U,190U,124U,249U,242U,229U,
                202U,148U,40U,80U,161U,66U,132U,9U,19U,39U,79U,159U,63U,127U};

static char decodechirp(struct FFRAME * frame, const struct BINS bins, char opti)
{
   int32_t o;
   uint32_t burst;
   uint32_t crctry;
   uint32_t datalen;
   uint32_t cr;
   uint32_t sf;
   uint32_t st;
   uint32_t try0;
   uint32_t fulllen;
   uint32_t j1;
   uint32_t i0;
   int32_t bursttry;
   int32_t mintry;
   int32_t maxtry;
   float lv;
   float v;
   float minsnr;
   float maxlev;
   char bt;
   /*    c:SET16; */
   uint8_t s;
   /*    b:CARD8;  */
   char dcd;
   char explicit;
   char crcok;
   char burstcorr;
   char hamcorr;
   char hamok;
   char ishead;
   NIBBBLOCK hn;
   char text[261];
   uint16_t chirps[8];
   char br[2];
   struct FFRAME * anonym;
   uint32_t tmp;
   { /* with */
      struct FFRAME * anonym = frame;
      sf = anonym->cfgsf;
      cr = anonym->cfgcr;
      datalen = anonym->cfgdatalen;
      explicit = cr==0UL;
      ishead = explicit && anonym->dcnt<8UL; /* we are in header */
      opti = opti || ishead; /* header is always optimized */
      o = 0L;
      if (opti) o = 2L;
      sf -= (uint32_t)o;
      if (ishead) cr = 8UL;
      else if (explicit) cr = anonym->crfromhead;
      else anonym->crfromhead = cr;
      if (cr<4UL) cr = 4UL;
      else if (cr>8UL) cr = 8UL;
      burstcorr = 0;
      if (anonym->dcnt==0UL) {
         /* start new frame */
         anonym->chirpc = 0UL;
         anonym->nibbc = 0UL;
         anonym->dlen = datalen; /* for implicit header */
         anonym->withcrc = anonym->implicitcrc;
         anonym->fecinfo[0] = 0;
         anonym->oneerrs = 0UL;
      }
      anonym->bintab[anonym->chirpc] = bins;
      ++anonym->chirpc;
      if (anonym->chirpc>=cr) {
         /* block complete */
         dcd = anonym->timeout<cr;
         anonym->chirpc = 0UL;
         /*-fec */
         fecneighbour(anonym->bintab, cr, sf);
         try0 = 0UL;
         maxlev = 0.0f;
         maxtry = -1L;
         minsnr = X2C_max_real;
         mintry = -1L;
         bursttry = -1L;
         burst = 0UL;
         st = 0UL;
         for (;;) {
            lv = 0.0f;
            tmp = cr-1UL;
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               if (try0/4UL==i0) j1 = try0&3UL;
               else j1 = 0UL;
               chirps[i0] = gray((uint16_t)(uint32_t)X2C_TRUNCC(anonym->bintab[i0].b[j1].freq,0UL,
                X2C_max_longcard));
               if (try0==0UL) {
                  v = anonym->bintab[i0].noise; /* noise */
                  if (v!=0.0f) {
                     v = X2C_DIVR(anonym->bintab[i0].b[0U].lev,v); /* snr of best bin */
                  }
                  if (v<minsnr) {
                     minsnr = v; /* weakest chirp */
                     mintry = (int32_t)i0;
                  }
                  if (burst<anonym->bintab[i0].halfnoisc) {
                     burst = anonym->bintab[i0].halfnoisc;
                     bursttry = (int32_t)i0;
                  }
               }
               else lv = anonym->bintab[i0].b[j1].lev;
               if (i0==tmp) break;
            } /* end for */
            if (burst>=8UL) {
               mintry = bursttry;
               burstcorr = 1;
            }
            deint(sf, cr, chirps, 8ul, hn, 12ul); /* deint block */
            chkhamm(hn, cr, sf, &hamok, &hamcorr);
            if (st>0UL) {
               /* final try */
               if (st==2UL) {
                  /* repair weekest chirp */
                  if (!nocrcfec) {
                     if (anonym->oneerrs==0UL) {
                        /* save block for later tries to fit crc */
                        memcpy(anonym->oneerr,hn,12u);
                        anonym->oneerrs = anonym->nibbc;
                     }
                     else {
                        anonym->oneerrs = 1UL; /* do it only for 1 block */
                     }
                  }
                  tmp = sf-1UL;
                  i0 = 0UL;
                  if (i0<=tmp) for (;; i0++) {
                     /* invert wrong bits */
                     bt = X2C_IN(mintry,8,hn[i0]);
                     hn[i0] &= ~(1U<<mintry);
                     if (bt==((0x80U & hn[i0])!=0)) hn[i0] |= (1U<<mintry);
                     if (i0==tmp) break;
                  } /* end for */
               }
               break;
            }
            if (hamok || nofec) {
               /* checkbits ok */
               if (try0==0UL) break;
               if (lv>maxlev) {
                  /* best bin with check ok */
                  maxlev = lv;
                  maxtry = (int32_t)try0;
               }
            }
            ++try0;
            if (try0>=4UL*cr) {
               /* all combinations tested */
               try0 = 0UL;
               st = 1UL;
               if (maxtry<0L) {
                  /* no valid checkbits found */
                  if (mintry>=0L) {
                     try0 = 0UL;
                     st = 2UL;
                  }
               }
               else try0 = (uint32_t)maxtry;
            }
         }
         if (st==2UL) {
            if (burstcorr) aprsstr_Append(anonym->fecinfo, 96ul, "b", 2ul);
            else aprsstr_Append(anonym->fecinfo, 96ul, "~", 2ul);
         }
         else if (hamok) {
            if (st==1UL) aprsstr_Append(anonym->fecinfo, 96ul, "^", 2ul);
            else if (hamcorr) aprsstr_Append(anonym->fecinfo, 96ul, "h", 2ul);
            else aprsstr_Append(anonym->fecinfo, 96ul, "+", 2ul);
         }
         else aprsstr_Append(anonym->fecinfo, 96ul, "-", 2ul);
         /*-fec */
         if (verb2) {
            if (isize==1UL) v = 8.0f;
            else if (isize==2UL) v = 0.03125f;
            else v = 4.0f;
            tmp = cr-1UL;
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               for (j1 = 0UL; j1<=3UL; j1++) {
                  strncpy(br,"()",2u);
                  if (st==1UL) {
                     /* best bin sum */
                     if (try0/3UL!=i0 && j1==0UL || try0/3UL==i0 && try0%3UL==j1) {
                        strncpy(br,"[]",2u);
                     }
                  }
                  else if (st==2UL) {
                     /* weekest bin */
                     if (mintry!=(int32_t)i0 && j1==0UL) {
                        strncpy(br,"[]",2u);
                     }
                  }
                  else if (j1==0UL) strncpy(br,"[]",2u);
                  if (j1==0UL && anonym->bintab[i0].splt<0.0f) {
                     osic_WrFixed(-(osic_sqrt((float)fabs(anonym->bintab[i0].splt))*v), 1L, 5UL);
                  }
                  else {
                     osic_WrFixed(osic_sqrt(anonym->bintab[i0].b[j1].lev)*v, 1L, 5UL);
                  }
                  osi_WrStr((char *) &br[0U], 1u/1u);
                  osic_WrFixed(anonym->bintab[i0].b[j1].freq, (int32_t)(2UL*(uint32_t)(j1==0UL)), 5UL);
                  osi_WrStr((char *) &br[1U], 1u/1u);
               } /* end for */
               osic_WrFixed(osic_sqrt(anonym->bintab[i0].noise)*v, 1L, 5UL);
               osi_WrStr("%", 2ul);
               osic_WrFixed((X2C_DIVR((float)anonym->bintab[i0].halfnoisc,
                (float)(uint32_t)(1UL<<anonym->cfgsf)))*100.0f, 1L, 0UL);
               osi_WrStrLn("", 1ul);
               if (i0==tmp) break;
            } /* end for */
         }
         if (verb2) osi_WrStrLn("", 1ul);
         if (ishead) {
            /* head block */
            if (headcrc(hn, 12ul)) {
               anonym->dlen = (uint32_t)(uint8_t)(X2C_LSH(hn[0U]&0xFU,8,4)|hn[1U]&0xFU);
               anonym->withcrc = (0x1U & hn[2U])!=0;
               anonym->crfromhead = (uint32_t)(4U+(uint8_t)(X2C_LSH(hn[2U],8,-1)&0x7U));
               if (anonym->crfromhead>8UL) anonym->crfromhead = 8UL;
               tmp = sf-1UL;
               i0 = 5UL;
               if (i0<=tmp) for (;; i0++) {
                  /* 5 header nibbles copy rest to data */
                  anonym->nibbs[anonym->nibbc] = hn[i0]&0xFU;
                  ++anonym->nibbc;
                  if (i0==tmp) break;
               } /* end for */
            }
            else {
               if (verb2) osi_WrStrLn("head crc error", 15ul);
               anonym->dlen = 0UL; /* show metadata and stop frame soon */
               anonym->withcrc = 0;
            }
         }
         else if (dcd) {
            /* data block */
            tmp = sf-1UL;
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               anonym->nibbs[anonym->nibbc] = hn[i0]&0xFU;
               if (anonym->nibbc<522UL) ++anonym->nibbc;
               if (i0==tmp) break;
            } /* end for */
         }
      }
      if (anonym->chirpc==0UL) {
         /* block complete dcd-loss over 1 block */
         if (explicit) {
            /* len from header */
            if (datalen && anonym->dlen>datalen) anonym->dlen = datalen;
            fulllen = anonym->dlen;
            if (anonym->withcrc) fulllen += 2UL;
         }
         else if (datalen==0UL) {
            /* len from dcd */
            anonym->dlen = anonym->nibbc/2UL;
            if (dcd) fulllen = 255UL;
            else fulllen = anonym->dlen;
            /*          fulllen:=dlen; */
            if (anonym->withcrc && anonym->dlen>=2UL) anonym->dlen -= 2UL;
         }
         else {
            /* fixed len */
            anonym->dlen = datalen;
            fulllen = datalen;
            if (anonym->withcrc) fulllen += 2UL;
         }
         if (!dcd || anonym->nibbc>=fulllen*2UL) {
            /* frame complete or implicit variable len dcd out */
            if (anonym->nibbc>1UL) {
               crctry = 0UL;
               for (;;) {
                  tmp = anonym->nibbc/2UL-1UL;
                  i0 = 0UL;
                  if (i0<=tmp) for (;; i0++) {
                     s = anonym->nibbs[i0*2UL]&0xFU|X2C_LSH(anonym->nibbs[i0*2UL+1UL],8,4)&0xF0U;
                     if (i0<anonym->dlen) {
                        s = s^(uint8_t)_cnst3[i0];
                     }
                     text[i0] = (char)s;
                     if (i0==tmp) break;
                  } /* end for */
                  if (anonym->dlen>anonym->nibbc/2UL) {
                     anonym->dlen = anonym->nibbc/2UL; /* remove dcd lost junk */
                  }
                  crcok = 0;
                  if (!anonym->withcrc) break;
                  i0 = 0UL;
                  if (anonym->dlen>2UL) {
                     i0 = crc(text, 261ul, 0UL, anonym->dlen-2UL, 0UL);
                  }
                  if (anonym->dlen>1UL) {
                     i0 = (uint32_t)((uint16_t)i0^(uint16_t)((uint32_t)(uint8_t)text[anonym->dlen-2UL]*256UL))
                ;
                  }
                  if (anonym->dlen>0UL) {
                     i0 = (uint32_t)((uint16_t)i0^(uint16_t)(uint32_t)(uint8_t)text[anonym->dlen-1UL]);
                  }
                  crcok = (char)(i0&255UL)==text[anonym->dlen] && (char)(i0/256UL)==text[anonym->dlen+1UL];
                  if ((crcok || anonym->oneerrs<=1UL) || crctry>=4UL) break;
                  trycrc(anonym->nibbs, anonym->oneerr, 12ul, &anonym->oneerrs, &crctry, sf);
               }
               if (anonym->oneerrs>1UL && crctry>0UL) {
                  i0 = 0UL;
                  for (;;) {
                     if (anonym->fecinfo[i0]==0) break;
                     if (anonym->fecinfo[i0]=='~') {
                        anonym->fecinfo[i0] = 'c';
                        break;
                     }
                     if (anonym->fecinfo[i0]=='b') {
                        anonym->fecinfo[i0] = 'C';
                        break;
                     }
                     ++i0;
                  }
               }
               frameout(frame, anonym->fecinfo, 96ul, anonym->withcrc, crcok, !dcd, opti, cr, text, 261ul);
            }
            return 1;
         }
      }
   }
   return 0;
} /* end decodechirp() */

#define lorarx_BIRDSPEED 0.01
/* floating median speed */

#define lorarx_MINBIRDYLEV 0.7
/* minimal sqr level of a birdy */


static void findbirdies(uint32_t from, uint32_t to, const struct Complex fir[], uint32_t fir_len)
{
   uint32_t optbirds;
   uint32_t nc;
   uint32_t len;
   uint32_t f1;
   uint32_t f0;
   uint32_t hsize;
   uint32_t j1;
   uint32_t i0;
   int32_t n;
   float iv;
   float v;
   float med;
   struct Complex * anonym;
   uint32_t tmp;
   len = (pbirdbuf->Len0-1)+1UL;
   while (birdpos<len && from<to) {
      /* fill up fft buffer to size of fir */
      pbirdbuf->Adr[birdpos] = fir[from];
      ++from;
      ++birdpos;
   }
   if (birdpos>=len) {
      /* fft buffer full */
      ++notchcnt;
      if (notchcnt>autonotch) {
         notchcnt = 0UL;
         birdpos = 0UL;
         i0 = 1UL;
         do {
            ++i0;
         } while (len>(uint32_t)(1UL<<i0));
         Transform(pbirdbuf->Adr, pbirdbuf->Len0, i0, 0);
         hsize = len/2UL;
         tmp = len-1UL;
         i0 = 0UL;
         if (i0<=tmp) for (;; i0++) {
            { /* with */
               struct Complex * anonym = &pbirdbuf->Adr[i0];
               anonym->Re = anonym->Re*anonym->Re+anonym->Im*anonym->Im;
            }
            if (i0==tmp) break;
         } /* end for */
         tmp = len-1UL;
         i0 = 0UL;
         if (i0<=tmp) for (;; i0++) {
            /* reorder freq */
            if (i0<=hsize) j1 = hsize-i0;
            else j1 = hsize*3UL-i0;
            pbirdbuf->Adr[i0].Im = pbirdbuf->Adr[j1].Re;
            if (i0==tmp) break;
         } /* end for */
         i0 = (uint32_t)X2C_TRUNCC(X2C_DIVR((float)len*0.5f,downsample),0UL,X2C_max_longcard);
                /* use only passband area of filter */
         if (i0<hsize) f0 = hsize-i0;
         else f0 = 0UL;
         f1 = hsize+i0;
         if (f1>=len) f1 = len-1UL;
         med = 1.E-7f;
         tmp = f1;
         i0 = f0;
         if (i0<=tmp) for (;; i0++) {
            med = med+pbirdbuf->Adr[i0].Im; /* noise level */
            if (i0==tmp) break;
         } /* end for */
         med = X2C_DIVR(med,(float)((f1-f0)+1UL));
         iv = X2C_DIVR(1.0f,med);
         tmp = f1;
         i0 = f0;
         if (i0<=tmp) for (;; i0++) {
            v = pbirdhist->Adr[i0];
            pbirdhist->Adr[i0] = v+((pbirdbuf->Adr[i0].Im-med)*iv-v)*0.01f;
            if (i0==tmp) break;
         } /* end for */
         /*- find notch start level */
         optbirds = ((f1-f0)+5UL)/5UL; /* limit notches */
         n = 0L;
         for (;;) {
            if (notchthres<0.7f) {
               notchthres = 0.7f;
               break;
            }
            nc = 0UL;
            tmp = f1;
            i0 = f0;
            if (i0<=tmp) for (;; i0++) {
               if (pbirdhist->Adr[i0]>notchthres) ++nc;
               if (i0==tmp) break;
            } /* end for */
            /*WrStr("(");WrInt(nc,1);WrStr(")"); */
            if (nc>optbirds) {
               if (n<0L) break;
               notchthres = notchthres+0.05f;
               /*WrFixed(notchthres, 2,10); */
               n = 1L;
            }
            else {
               if (n>0L) break;
               notchthres = notchthres-0.05f;
               /*WrFixed(notchthres, 2,10); */
               n = -1L;
            }
         }
         /*WrFixed(notchthres, 2,12); WrStrLn("=tres");         */
         makefir(downsample, pfirtab, pbirdhist, notchthres);
      }
   }
} /* end findbirdies() */


/* split samples in left from peak and right and transform separate and add gains */
static float fasejumps(struct Complex b[], uint32_t b_len, uint32_t guess, uint32_t mid, uint32_t sf)
{
   uint32_t jj;
   uint32_t j1;
   uint32_t i0;
   struct Complex ff;
   struct Complex f;
   float co;
   float si;
   uint16_t wgi;
   struct Complex * anonym;
   uint32_t tmp;
   jj = (sf+1UL)-guess%sf;
   j1 = (jj*65536UL)/sf;
   f.Re = 0.0f;
   f.Im = 0.0f;
   wgi = 0U;
   tmp = sf-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      si = DDS[wgi];
      co = DDS[(uint16_t)(wgi+16384U)];
      { /* with */
         struct Complex * anonym = &b[i0];
         f.Re = (f.Re+anonym->Im*co)-anonym->Re*si;
         f.Im = f.Im+anonym->Re*co+anonym->Im*si;
      }
      wgi += (uint16_t)j1;
      if (i0==mid) {
         ff = f; /* store left part from peak */
         f.Re = 0.0f;
         f.Im = 0.0f;
      }
      if (i0==tmp) break;
   } /* end for */
   return ff.Re*ff.Re+ff.Im*ff.Im+f.Re*f.Re+f.Im*f.Im;
} /* end fasejumps() */


struct _3 {
   float lev;
   uint32_t bn;
};


static void getbin(struct Complex c[], uint32_t c_len, struct BINS * bins, uint32_t sf, float offset,
                char rev, char opt, char sort, char invers, uint32_t * corrcnt)
{
   uint32_t bufsize;
   uint32_t imax;
   uint32_t lastbin;
   uint32_t jj;
   uint32_t j1;
   uint32_t i0;
   uint16_t wgi;
   int32_t ii;
   float mnois;
   float vv;
   float v;
   float nois;
   float co;
   float si;
   float max0;
   struct BIN hb;
   float vf[3];
   float amp[4096];
   struct Complex tmp[4096];
   struct Complex * anonym;
   struct Complex * anonym0;
   uint32_t tmp0;
   bufsize = (uint32_t)(1UL<<sf);
   ii = (int32_t)(65536UL/bufsize);
   if (rev!=invers) ii = -ii;
   j1 = (uint32_t)(32768L-(int32_t)X2C_TRUNCI(offset*(float)ii,X2C_min_longint,X2C_max_longint));
                /* fine tune frequ */
   wgi = 0U;
   tmp0 = bufsize-1UL;
   i0 = 0UL;
   if (i0<=tmp0) for (;; i0++) {
      /* dechirp */
      si = DDS[wgi];
      co = DDS[(uint16_t)(wgi+16384U)];
      { /* with */
         struct Complex * anonym = &c[i0];
         v = anonym->Im*co-anonym->Re*si;
         anonym->Im = anonym->Re*co+anonym->Im*si;
         anonym->Re = v;
      }
      wgi += (uint16_t)j1;
      j1 += (uint32_t)ii;
      if (i0==tmp0) break;
   } /* end for */
   if (outdechirped>=0L) {
      osi_WrBin(outdechirped, (char *)c, (c_len*sizeof(struct Complex))/1u, c_len*sizeof(struct Complex));
   }
   X2C_MOVE((char *)c,(char *)tmp,c_len*sizeof(struct Complex));
   Transform(c, c_len, sf, 0);
   /*  c[0].Re:=c[0].Re*2.0;                                      (* dc part *) */
   /*  c[0].Im:=c[0].Im*2.0; */
   lastbin = 0UL;
   memset((char *)bins,(char)0,sizeof(struct BINS));
   max0 = (-1.0f);
   nois = 0.0f;
   j1 = 0UL;
   ii = 1L;
   if (invers) {
      j1 = bufsize-1UL;
      ii = -1L;
   }
   tmp0 = bufsize-1UL;
   i0 = 0UL;
   if (i0<=tmp0) for (;; i0++) {
      { /* with */
         struct Complex * anonym0 = &c[i0];
         v = anonym0->Re*anonym0->Re+anonym0->Im*anonym0->Im;
      }
      amp[j1] = v;
      nois = nois+v;
      if (v>max0) {
         max0 = v;
         imax = j1;
      }
      j1 += (uint32_t)ii;
      if (i0==tmp0) break;
   } /* end for */
   /*
     IF NOT opt THEN                               (* limit afc runaway in echo distorted signal *)
       i:=(bufsize+imax-1) MOD bufsize;
       v:=amp[imax]*0.25;
       amp[i]:=flmin(amp[i], v);
       i:=(imax+1) MOD bufsize;
       amp[i]:=flmin(amp[i], v);
     END;
   */
   if (binsview>0UL) {
      /* show bins around best */
      osic_WrINT32(imax, 4UL);
      if (rev) osi_WrStr("- ", 3ul);
      else if (sort) osi_WrStr("> ", 3ul);
      else osi_WrStr(": ", 3ul);
      v = X2C_DIVR(1.0f,amp[imax]+1.E-6f);
      tmp0 = binsview;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         osic_WrFixed(osic_sqrt(amp[((bufsize+i0+imax)-binsview/2UL)%bufsize]*v), 2L, 4UL);
         osi_WrStr(" ", 2ul);
         if (i0==tmp0) break;
      } /* end for */
      osi_WrStrLn("", 1ul);
   }
   /*- find fase jump */
   if (!nomultipath && sort) {
      if (imax>20UL && imax+20UL<bufsize) {
         /* no advantage with small wrap around part */
         v = 0.0f;
         j1 = 1UL;
         for (i0 = 0UL; i0<=2UL; i0++) {
            /* test freq below and above peak freq */
            jj = imax+i0;
            if (invers) jj = ((bufsize-imax)+i0)-1UL;
            vv = fasejumps(tmp, 4096ul, jj, bufsize-imax, bufsize); /* split samples in before and after wrap around */
            if (vv>v) {
               v = vv; /* winner has max. level sum */
               j1 = i0;
            }
         } /* end for */
         if (invers) j1 = (((bufsize+imax)-j1)+1UL)%bufsize;
         else j1 = ((imax+bufsize+j1)-1UL)%bufsize;
         if (j1!=imax) {
            /* swap best level with found real best */
            v = amp[imax];
            amp[imax] = v*0.51f; /* make original max bin to second best */
            amp[((bufsize+j1+j1)-imax)%bufsize] = v*0.49f; /* make 0.51/1/0.49 peak as not known freq offset */
            amp[j1] = v;
            bins->splt = -v; /* for -V view only */
            imax = j1;
            ++*corrcnt; /* for monitoring only */
         }
      }
   }
   for (i0 = 0UL; i0<=2UL; i0++) {
      v = amp[(((imax+bufsize)-1UL)+i0)%bufsize]; /* subtract data bins from noise sum */
      if (nois>v) nois = nois-v;
   } /* end for */
   nois = X2C_DIVR(nois,(float)bufsize);
   if (sort) {
      mnois = max0*0.8f;
      max0 = 0.0f;
      bins->halfnoisc = 0UL;
      tmp0 = bufsize-1UL;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         vv = amp[i0];
         if (vv>max0) {
            max0 = vv;
            imax = i0;
         }
         if (vv>mnois) ++bins->halfnoisc;
         if (max0>bins->b[lastbin].lev && (!opt || (i0&3UL)==1UL)) {
            bins->b[lastbin].lev = max0;
            bins->b[lastbin].bn = imax;
            v = X2C_max_real;
            for (j1 = 0UL; j1<=3UL; j1++) {
               /* find new weekest bin */
               if (bins->b[j1].lev<v) {
                  v = bins->b[j1].lev;
                  lastbin = j1;
               }
            } /* end for */
            max0 = bins->b[lastbin].lev;
         }
         if (i0==tmp0) break;
      } /* end for */
      j1 = 0UL;
      for (i0 = 1UL; i0<=3UL; i0++) {
         if (bins->b[i0].lev>bins->b[j1].lev) j1 = i0;
      } /* end for */
      if (j1>0UL) {
         /* swap best to front */
         hb = bins->b[0U];
         bins->b[0U] = bins->b[j1];
         bins->b[j1] = hb;
      }
   }
   else {
      bins->b[0U].lev = max0;
      bins->b[0U].bn = imax;
   }
   bins->noise = nois;
   for (i0 = 0UL; i0<=2UL; i0++) {
      vf[i0] = amp[(((bins->b[0U].bn+bufsize)-1UL)+i0)%bufsize];
   } /* end for */
   v = 0.0f;
   bins->b[0U].sharpness = 0.0f;
   if (vf[1U]!=0.0f) {
      v = X2C_DIVR((vf[2U]-vf[0U])*0.5f,vf[1U]);
      bins->b[0U].sharpness = X2C_DIVR(flmax(0.0f, vf[1U]-(vf[2U]+vf[0U])*0.5f),vf[1U]);
   }
   /*  IF vf[1]<>0.0 THEN v:=(sqrt(vf[2])-sqrt(vf[0]))*0.5/sqrt(vf[1]) END; */
   if (v>0.49f) v = 0.49f;
   else if (v<(-0.49f)) v = (-0.49f);
   for (i0 = 0UL; i0<=3UL; i0++) {
      v = v+0.5f+(float)bins->b[i0].bn;
      if (opt) v = v*0.25f+0.375f;
      bins->b[i0].freq = v;
      v = 0.0f;
   } /* end for */
} /* end getbin() */

union _4;


union _4 {
   struct Complex c[2048];
   short i0[8192];
   uint8_t b[16384];
};


static char inreform(struct Complex b[], uint32_t b_len, uint32_t * wp)
{
   uint32_t rs;
   uint32_t bs;
   uint32_t i0;
   int32_t res;
   union _4 ib;
   char * p;
   uint32_t tmp;
   bs = isize*4096UL;
   rs = 0UL;
   do {
      p = (char *) &ib.b[rs];
      res = osi_RdBin(iqfd, p, 65536ul, bs-rs);
      if (res<=0L) return 0;
      rs += (uint32_t)res;
   } while (rs<bs);
   if (isize==1UL) {
      tmp = rs-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (tmp = (uint32_t)(tmp-i0)/2UL;;) {
         b[*wp].Re = (float)((int32_t)ib.b[i0]-128L);
         b[*wp].Im = (float)((int32_t)ib.b[i0+1UL]-128L);
         *wp = (*wp+1UL)%((b_len-1)+1UL);
         if (!tmp) break;
         --tmp;
         i0 += 2UL;
      } /* end for */
   }
   else if (isize==2UL) {
      tmp = rs/2UL-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (tmp = (uint32_t)(tmp-i0)/2UL;;) {
         b[*wp].Re = (float)ib.i0[i0];
         b[*wp].Im = (float)ib.i0[i0+1UL];
         *wp = (*wp+1UL)%((b_len-1)+1UL);
         if (!tmp) break;
         --tmp;
         i0 += 2UL;
      } /* end for */
   }
   else {
      tmp = rs/8UL-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         b[*wp] = ib.c[i0];
         *wp = (*wp+1UL)%((b_len-1)+1UL);
         if (i0==tmp) break;
      } /* end for */
   }
   return 1;
} /* end inreform() */


static char readsampsfir(void)
{
   uint32_t frd;
   uint32_t fp;
   uint32_t w0;
   uint32_t i0;
   float f;
   float co;
   float si;
   struct Complex sum;
   struct Complex fc;
   char * ph;
   struct Complex * anonym;
   uint32_t tmp;
   if (firlen==0UL) return inreform(iqbuf, 8192ul, &iqwrite);
   if (!inreform(pfir->Adr, pfir->Len0, &firwp)) return 0;
   if (firwp>firlen) {
      if (shiftstep) {
         tmp = firwp-1UL;
         i0 = firlen;
         if (i0<=tmp) for (;; i0++) {
            { /* with */
               struct Complex * anonym = &pfir->Adr[i0];
               w0 = phasereg/65536UL;
               si = DDS[w0];
               co = DDS[(uint16_t)(w0+16384UL)];
               sum.Re = anonym->Re*si-anonym->Im*co;
               anonym->Im = anonym->Re*co+anonym->Im*si;
               anonym->Re = sum.Re;
            }
            phasereg += shiftstep;
            if (i0==tmp) break;
         } /* end for */
      }
      if (autonotch) findbirdies(firlen, firwp, pfir->Adr, pfir->Len0);
      /*fir resample, interpolate, notch */
      w0 = iqwrite;
      frd = 0UL;
      do {
         sum.Re = 0.0f;
         sum.Im = 0.0f;
         fp = (sampc>>20&15UL)*firlen; /* interpolate */
         if (complexfir) {
            tmp = (frd+firlen)-1UL;
            i0 = frd;
            if (i0<=tmp) for (;; i0++) {
               fc = pfirtab->Adr[fp];
               ++fp;
               sum.Re = (sum.Re+pfir->Adr[i0].Re*fc.Re)-pfir->Adr[i0].Im*fc.Im;
               sum.Im = sum.Im+pfir->Adr[i0].Re*fc.Im+pfir->Adr[i0].Im*fc.Re;
               if (i0==tmp) break;
            } /* end for */
         }
         else {
            /* symmetric fir, save cpu if no notches */
            tmp = (frd+firlen)-1UL;
            i0 = frd;
            if (i0<=tmp) for (;; i0++) {
               f = pfirtab->Adr[fp].Im;
               ++fp;
               sum.Re = sum.Re+pfir->Adr[i0].Re*f;
               sum.Im = sum.Im+pfir->Adr[i0].Im*f;
               if (i0==tmp) break;
            } /* end for */
         }
         if (noisblanker) {
            f = sum.Re*sum.Re+sum.Im*sum.Im;
            pknoisemed = pknoisemed*pknoisedown+(f-pknoisemed)*pknoiseup;
            if (f>pknoisemed*2.0f) {
               /* limit to median level */
               f = X2C_DIVR(pknoisemed*2.0f,f); /* raw sqrt */
               sum.Re = sum.Re*f;
               sum.Im = sum.Im*f;
            }
         }
         iqbuf[iqwrite] = sum;
         iqwrite = iqwrite+1UL&8191UL;
         sampc += samprate;
         frd += sampc>>24;
         sampc = (uint32_t)((uint32_t)sampc&0xFFFFFFUL);
      } while (frd+firlen<firwp);
      if (outfiltered>=0L) {
         /* debug out */
         if (iqwrite<w0) i0 = 8192UL-w0;
         else i0 = iqwrite-w0;
         ph = (char *) &iqbuf[w0];
         osi_WrBin(outfiltered, (char *)ph, 1000001u/1u, i0*sizeof(struct Complex));
         if (iqwrite<w0 && iqwrite>0UL) {
            osi_WrBin(outfiltered, (char *)iqbuf, sizeof(struct Complex [8192])/1u, iqwrite*sizeof(struct Complex));
         }
      }
      if (firwp>frd) {
         firwp -= frd;
         X2C_MOVE((char *) &pfir->Adr[frd],(char *) &pfir->Adr[0UL],firwp*sizeof(struct Complex));
      }
   }
   return 1;
} /* end readsampsfir() */

/*
PROCEDURE readsamps():BOOLEAN;
VAR i, bs, rs:CARDINAL;
    res:INTEGER;
    ib:RECORD CASE :CARDINAL OF
                0:c:ARRAY[0..MAXINBUF-1] OF Complex;
               |1:i:ARRAY[0..MAXINBUF*4-1] OF INT16;
               |2:b:ARRAY[0..MAXINBUF*8-1] OF CARD8;
              END;
       END;
    p:POINTER TO ARRAY[0..65535] OF BYTE;
BEGIN
  bs:=isize*(MAXINBUF*2);
  rs:=0;
  REPEAT
    p:=ADR(ib.b[rs]);
    res:=RdBin(iqfd, p^, bs-rs);
    IF res<=0 THEN RETURN FALSE END;

    INC(rs, res);
  UNTIL (rs>=bs) OR (rs MOD 8=0);

  IF isize=1 THEN
    FOR i:=0 TO rs-1 BY 2 DO
      iqbuf[iqwrite].Re:=VAL(REAL, VAL(INTEGER, ib.b[i  ])-127);
      iqbuf[iqwrite].Im:=VAL(REAL, VAL(INTEGER, ib.b[i+1])-127);
      iqwrite:=(iqwrite+1) MOD (HIGH(iqbuf)+1)
    END;
  ELSIF isize=2 THEN
    FOR i:=0 TO rs DIV 2 - 1 BY 2 DO
      iqbuf[iqwrite].Re:=VAL(REAL, ib.i[i  ]);
      iqbuf[iqwrite].Im:=VAL(REAL, ib.i[i+1]);
      iqwrite:=(iqwrite+1) MOD (HIGH(iqbuf)+1)
    END;
  ELSE
    FOR i:=0 TO rs DIV 8 - 1 DO
      iqbuf[iqwrite]:=ib.c[i];
      iqwrite:=(iqwrite+1) MOD (HIGH(iqbuf)+1)
    END;
  END;
  RETURN TRUE
END readsamps;
*/
#define lorarx_IQSIZE 8


static char getsamps(struct Complex c[], uint32_t c_len, uint32_t * iqread, uint32_t jump)
{
   uint32_t i0;
   uint32_t newsamps;
   newsamps = (iqwrite+8192UL)-*iqread&8191UL;
   if (newsamps<jump) return 0;
   *iqread = (*iqread+jump+8192UL)-((c_len-1)+1UL)&8191UL;
   if (*iqread+(c_len-1)+1UL>8192UL) {
      /* wraps around */
      i0 = 8192UL-*iqread;
      X2C_MOVE((char *) &iqbuf[*iqread],(char *)c,i0*8UL);
      if (i0<=c_len-1) X2C_MOVE((char *)iqbuf,(char *) &c[i0],(((c_len-1)+1UL)-i0)*8UL);
   }
   else X2C_MOVE((char *) &iqbuf[*iqread],(char *)c,((c_len-1)+1UL)*8UL);
   *iqread = *iqread+(c_len-1)+1UL&8191UL;
   return 1;
} /* end getsamps() */


static float squelch(const struct BINS bins)
{
   return X2C_DIVR(bins.b[0U].lev,bins.noise+1.E-9f);
} /* end squelch() */


static char nextchirp(struct FFRAME * frame)
{
   struct BINS bins;
   uint32_t blocksize;
   uint32_t i0;
   float sq;
   float fi;
   char opt;
   pCB anonym;
   pCB anonym0;
   blocksize = (uint32_t)(1UL<<frame->cfgsf);
   /*  LOOP */
   if (frame->state==lorarx_sSLEEP) {
      frame->jp = blocksize;
      frame->cnt = 0UL;
      frame->dcnt = 0UL;
      frame->iqread = iqwrite; /* start decode not from old data */
      frame->state = lorarx_sHUNT;
      frame->fcfix = 0.0f;
   }
   if (frame->jp==0UL) frame->jp = blocksize;
   if (!(anonym = fftbufs[frame->cfgsf],getsamps(anonym->Adr, anonym->Len0, &frame->iqread, frame->jp))) return 1;
   frame->jp = blocksize;
   opt = frame->state==lorarx_sDATA && (frame->optimize || frame->cfgcr==0UL && frame->dcnt<8UL);
   anonym0 = fftbufs[frame->cfgsf];
   getbin(anonym0->Adr, anonym0->Len0, &bins, frame->cfgsf, frame->fci+frame->fc+frame->fcfix,
                frame->state==lorarx_sREV1 || frame->state==lorarx_sREV2, opt, frame->state==lorarx_sDATA,
                frame->invertiq, &frame->fasecorrs);
   frame->fcfix = frame->fcfix+frame->dataratecorr; /* correct known samplerate error */
   /*- afc */
   if (frame->state>=lorarx_sSYNRAW) {
      fi = (bins.b[0U].freq-(float)(int32_t)X2C_TRUNCI(bins.b[0U].freq,X2C_min_longint,X2C_max_longint))-0.5f;
      frame->eye = frame->eye+(float)fabs(fi); /* for statistics */
      i0 = blocksize;
      if (opt) i0 = i0/4UL;
      fi = fi*(float)fabs(X2C_DIVR(bins.b[0U].freq,(float)i0)-0.5f)*2.0f; /* prefer not middle splitted sybols */
      /*    fi:=fi*ABS(fi);  */
      /*IF fi<0.0 THEN fi:=-sqrt(-fi) ELSE fi:=sqrt(fi) END; */
      if (opt) fi = fi*4.0f;
      /*      fi:=lim(fi*flmin(squelch(bins), 50.0)/50.0, 0.5); */
      fi = lim(X2C_DIVR(fi*flmin(squelch(bins), 100.0f),100.0f), X2C_DIVR(2.0f,(float)(frame->cnt+25UL)));
      fi = fi*bins.b[0U].sharpness*bins.b[0U].sharpness;
      fi = fi*frame->afcspeed;
      /*      fi:=fi*(0.1+0.2/FLOAT(frame.cnt+8)); */
      frame->fc = (frame->fc-frame->fci)-fi*12.0f; /* datarate lead-lag loop filter */
      /*      frame.fci:=frame.fci+fi*(0.1+0.2/FLOAT(frame.cnt+8)); */
      frame->fci = frame->fci+fi;
   }
   /*WrFixed(frame.fc, 3,1); WrStr(" "); WrFixed(frame.fci, 4,1); WrStr(" "); WrFixed(fi, 4,1); WrStr(" ");
                WrFixed(bins.b[0].sharpness, 2,1); WrStrLn("=fc, fci, f, sharp");  */
   /*- afc */
   if (frame->state==lorarx_sHUNT) {
      /*IF verb2 THEN WrInt(frame.cnt,1); WrStr(" "); WrFixed(freqmod(frame.lastbin-bins.b[0].freq, blocksize),2,1);
                WrStrLn(" hunt") END; */
      if (frame->cnt==1UL && (float)fabs(freqmod(frame->lastbin-bins.b[0U].freq,
                (int32_t)blocksize))<=(float)(1UL+(uint32_t)frame->optimize*3UL)) {
         /* 2 good peamble chirps */
         frame->jp = blocksize-(uint32_t)X2C_TRUNCC(bins.b[0U].freq,0UL,X2C_max_longcard);
         frame->state = lorarx_sSYNRAW;
         frame->idfound = 0UL;
         frame->eye = 0.0f;
         frame->fc = 0.0f;
         frame->fci = 0.0f;
         if (verb2) {
            osic_WrINT32(frame->label, 1UL);
            osi_WrStr(" ", 2ul);
            osic_WrINT32(frame->jp, 1UL);
            osi_WrStrLn("=jump", 6ul);
         }
      }
      frame->cnt = 0UL;
      frame->lastbin = bins.b[0U].freq;
   }
   else if (frame->state==lorarx_sSYNRAW) {
      fi = freqmod(bins.b[0U].freq, (int32_t)blocksize);
      if ((float)fabs(fi)>=4.0f) {
         i0 = (uint32_t)X2C_TRUNCC((bins.b[0U].freq+3.5f)*0.125f,0UL,X2C_max_longcard);
         if (i0>15UL || (frame->synfilter>=256UL && (frame->synfilter&255UL)>=16UL) && i0!=(frame->synfilter&255UL)
                /16UL) {
            if (verb2) {
               osic_WrINT32(frame->label, 1UL);
               osi_WrStr(" ", 2ul);
               osi_WrStrLn("wrong first syn nibble", 23ul);
            }
            if (frame->idfound) frame->state = lorarx_sHUNT;
            frame->idfound = 1UL; /* set is not first sync exception */
         }
         else {
            if (verb2) {
               osic_WrINT32(frame->label, 1UL);
               osi_WrStr(" ", 2ul);
               osic_WrFixed(fi-0.5f, 2L, 1UL);
               osi_WrStrLn(" state=SYN 1 OK", 16ul);
            }
            frame->idfound = i0;
            frame->state = lorarx_sID;
         }
      }
      else {
         /* good syn frame */
         frame->idfound = 0UL;
         if (frame->cnt==2UL) frame->fcfix = (frame->fcfix-fi)+0.5f;
         if (verb2) {
            osic_WrINT32(frame->label, 1UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed(fi-0.5f, 2L, 1UL);
            osi_WrStrLn(" state=SYN", 11ul);
         }
      }
   }
   else if (frame->state==lorarx_sID) {
      if (verb2) {
         osic_WrINT32(frame->label, 1UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed(bins.b[0U].freq-0.5f, 2L, 1UL);
         osi_WrStrLn(" state=NETID", 13ul);
      }
      i0 = (uint32_t)X2C_TRUNCC((bins.b[0U].freq+3.5f)*0.125f,0UL,X2C_max_longcard);
      if (i0==0UL) {
         frame->state = lorarx_sSYNRAW; /* syn sequence with 1 exception */
         frame->idfound = 0UL;
      }
      else {
         frame->state = lorarx_sREV1;
         fi = (float)(i0*8UL); /* use sync pattern for median zero freq */
         frame->idfound = frame->idfound*16UL+i0;
         if (i0>15UL || (frame->synfilter>=256UL && (frame->synfilter&15UL)>0UL) && i0!=(frame->synfilter&15UL)) {
            frame->state = lorarx_sHUNT;
            if (verb2) {
               osic_WrINT32(frame->label, 1UL);
               osi_WrStr(" ", 2ul);
               osi_WrStrLn(" wrong second syn nibble", 25ul);
            }
         }
      }
   }
   else if (frame->state==lorarx_sREV1) {
      if (verb2) {
         osic_WrINT32(frame->label, 1UL);
         osi_WrStrLn(" state=REVERS1", 15ul);
      }
      frame->lastbin = freqmod(bins.b[0U].freq, (int32_t)blocksize)-0.5f;
      frame->lastsq = squelch(bins);
      frame->state = lorarx_sREV2;
   }
   else if (frame->state==lorarx_sREV2) {
      if (verb2) {
         osic_WrINT32(frame->label, 1UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed(bins.b[0U].freq-0.5f, 2L, 1UL);
         osi_WrStrLn(" state=REVERS2", 15ul);
      }
      fi = freqmod(bins.b[0U].freq, (int32_t)blocksize)-0.5f;
      sq = squelch(bins);
      if ((float)fabs(freqmod(frame->lastbin-fi, (int32_t)blocksize))>1.0f) {
         /* 2 good rev frames */
         if (frame->lastsq>squelch(bins)) {
            /* else use best of the 2 */
            fi = frame->lastbin;
            sq = frame->lastsq;
         }
      }
      if (sq>frame->datasquelch) {
         /* usable reverse chirp */
         fi = fi*0.5f; /* real freq halfway between forward & rev frames */
         frame->df = (int32_t)X2C_TRUNCI(fi*baud(frame->cfgsf, bwnum),X2C_min_longint,X2C_max_longint);
         frame->jp = (uint32_t)((int32_t)(blocksize+(blocksize>>2))+(int32_t)X2C_TRUNCI(fi,X2C_min_longint,
                X2C_max_longint));
         if (verb2) {
            osic_WrINT32(frame->label, 1UL);
            osi_WrStr(" ", 2ul);
            osic_WrINT32(frame->jp, 1UL);
            osi_WrStrLn("=jump", 6ul);
         }
         frame->fcfix = (frame->fcfix-(float)(int32_t)X2C_TRUNCI(fi,X2C_min_longint,X2C_max_longint))-1.0f;
         frame->state = lorarx_sDATA;
         frame->dcnt = 0UL;
         frame->txdel = frame->cnt; /* store for show preamble time */
         frame->timeout = 0UL;
         frame->fp = frame->iqread;
         frame->fasecorrs = 0UL;
         frame->sigsum = 0.0f;
         frame->sigmin = X2C_max_real;
         frame->sigmax = 0.0f;
         frame->noissum = 0.0f;
         frame->noismax = 0.0f;
         frame->noismin = X2C_max_real;
      }
      else {
         frame->state = lorarx_sHUNT;
         if (verb2) osi_WrStrLn("rev chirp timeout", 18ul);
      }
   }
   else {
      /* data */
      if (verb2) {
         osic_WrINT32(frame->label, 1UL);
         osi_WrStr(" ", 2ul);
         osic_WrINT32(frame->cnt, 1UL);
         osi_WrStrLn(" state=DATA", 12ul);
      }
      sq = squelch(bins);
      if (!frame->nodcdlost && sq<=frame->datasquelch) {
         ++frame->timeout;
         if (verb2) {
            osic_WrINT32(frame->label, 1UL);
            osi_WrStr(" ", 2ul);
            osi_WrStr(" dcd-lost-count=", 17ul);
            osic_WrINT32(frame->timeout, 1UL);
            osi_WrStrLn("", 1ul);
         }
      }
      else frame->timeout = 0UL;
      frame->lastbin = bins.b[0U].freq;
      frame->sigsum = frame->sigsum+bins.b[0U].lev;
      if (bins.b[0U].lev>frame->sigmax) frame->sigmax = bins.b[0U].lev;
      if (bins.b[0U].lev<frame->sigmin) frame->sigmin = bins.b[0U].lev;
      frame->noissum = frame->noissum+bins.noise;
      if (bins.noise>frame->noismax) frame->noismax = bins.noise;
      if (bins.noise<frame->noismin) frame->noismin = bins.noise;
      if (decodechirp(frame, bins, opt)) frame->state = lorarx_sSLEEP;
      ++frame->dcnt;
   }
   ++frame->cnt;
   return 0;
} /* end nextchirp() */

static void jsonpipebroken(int32_t);


static void jsonpipebroken(int32_t signum)
{
   osi_WrStr("got signal ", 12ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
} /* end jsonpipebroken() */

static uint32_t i;

static uint32_t j;

static char alldone;

static char allcol;

static pDEM pd;

static float w;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   uint32_t tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(NIBBBLOCK)!=12) X2C_ASSERT(0);
   if (sizeof(NIBBS)!=523) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   Parms();
   if (judpport>0UL) signal(SIGPIPE, jsonpipebroken);
   w = 3.1415926535f;
   for (i = 0UL; i<=24UL; i++) {
      SINTAB[i] = osic_sin(w);
      w = w*0.5f;
   } /* end for */
   pbirdbuf = 0;
   notchthres = 0.0f;
   pbirdhist = 0;
   if (firlen) {
      sampc = 0UL;
      firwp = 0UL;
      birdpos = 0UL;
      X2C_DYNALLOCATE((char **) &pfir,sizeof(struct Complex),(tmp[0] = 4096UL+firlen,tmp),1u);
      if (pfir==0) Error(" out of memory", 15ul);
      if (autonotch) {
         X2C_DYNALLOCATE((char **) &pbirdbuf,sizeof(struct Complex),(tmp[0] = firlen,tmp),1u);
         if (pbirdbuf==0) Error(" out of memory", 15ul);
         X2C_DYNALLOCATE((char **) &pbirdhist,sizeof(float),(tmp[0] = firlen,tmp),1u);
         if (pbirdhist==0) Error(" out of memory", 15ul);
         tmp0 = pbirdhist->Len0-1;
         i = 0UL;
         if (i<=tmp0) for (;; i++) {
            pbirdhist->Adr[i] = 0.0f;
            if (i==tmp0) break;
         } /* end for */
      }
      X2C_DYNALLOCATE((char **) &pfirtab,sizeof(struct Complex),(tmp[0] = firlen*16UL,tmp),1u);
      if (pfirtab==0) Error(" out of memory", 15ul);
      X2C_DYNALLOCATE((char **) &ptmpfir,sizeof(struct Complex),(tmp[0] = firlen*16UL,tmp),1u);
      if (ptmpfir==0) Error(" out of memory", 15ul);
      X2C_DYNALLOCATE((char **) &pfixfir,sizeof(float),(tmp[0] = firlen,tmp),1u);
      if (pfixfir==0) Error(" out of memory", 15ul);
      makefir(downsample, pfirtab, 0, 0.0f);
   }
   MakeDDS();
   for (i = 5UL; i<=12UL; i++) {
      X2C_DYNALLOCATE((char **) &fftbufs[i],sizeof(struct Complex),(tmp[0] = (uint32_t)(1UL<<i),tmp),1u);
      if (fftbufs[i]==0) Error("out of memory", 14ul);
   } /* end for */
   iqfd = osi_OpenRead(iqfn, 1024ul);
   if (iqfd<0L) Error("open iq file", 13ul);
   do {
      i = 0UL;
      do {
         /* do all collision search steps */
         pd = dems;
         while (pd) {
            pd->done = 0;
            pd = pd->next;
         }
         allcol = 1;
         do {
            /* do till no demodulator can fill fft buffer */
            alldone = 1;
            pd = dems;
            while (pd) {
               /* all demodulators 1 chirp */
               if (i<=pd->coldet && ((i==0UL || pd->frames[i].state>=lorarx_sSYNRAW)
                || pd->frames[i-1UL].state>=lorarx_sID)) {
                  /* start next demod or keep on running */
                  if (i>0UL && pd->frames[i].state>=lorarx_sDATA) {
                     /* check if earlier demod works on same frame */
                     tmp0 = i-1UL;
                     j = 0UL;
                     if (j<=tmp0) for (;; j++) {
                        if (pd->frames[j].state>=lorarx_sDATA && labs((int32_t)pd->frames[i].fp-(int32_t)
                pd->frames[j].fp)<=1L) {
                           /* if both are on same sample */
                           pd->frames[i].state = lorarx_sSLEEP; /* discontinue working on this frame */
                        }
                        if (j==tmp0) break;
                     } /* end for */
                  }
                  allcol = 0;
                  pd->done = pd->done || nextchirp(&pd->frames[i]);
                  alldone = alldone && pd->done;
               }
               pd = pd->next;
            }
         } while (!alldone);
         ++i;
      } while (!allcol);
   } while (readsampsfir());
   if (outdechirped>=0L) osic_Close(outdechirped);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
