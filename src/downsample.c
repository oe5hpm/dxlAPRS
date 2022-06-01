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
#define downsample_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <math.h>

/* shift frequency and change sampelrate of iq file */
#define downsample_DDSSIZE 65536

#define downsample_SUBSAMPBITS 8

#define downsample_FIRMAX 4096

#define downsample_PI 3.1415926535

#define downsample_SUBSAMP 256

#define downsample_LF "\012"

struct Complex;


struct Complex {
   float Im;
   float Re;
};

static char as[4096];

static char fni[4096];

static char fno[4096];

static char bb[32768];

static struct Complex ib[32768];

static uint8_t ob[32768];

static int32_t prefill;

static int32_t fi;

static int32_t fo;

static char eof;

static char shiftin;

static char shiftout;

static float fg;

static float re;

static float im;

static float f;

static float val;

static float sr;

static float srin;

static float gain;

static float outpeak;

static uint32_t pb;

static uint32_t lenb;

static uint32_t wp;

static uint32_t rp;

static uint32_t inform;

static uint32_t outform;

static uint32_t firlen;

static uint32_t rl;

static uint32_t fpend;

static uint32_t phasereg;

static uint32_t shiftstep;

static uint32_t sampc;

static uint32_t i;

static uint32_t fp;

static uint32_t samprate;

static uint32_t statistic;

static uint32_t limcnt;

static uint32_t outsamps;

static struct Complex sum;

struct _0;


struct _0 {
   float si;
   float co;
};

static struct _0 DDS[65536];

static float firraw[1048576];

static float firtab[1048576];


static void Err(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Err() */


static void mkfir(float fg0, float gain0, uint32_t subsamp,
                uint32_t flen, float fir[], uint32_t fir_len)
{
   uint32_t m;
   uint32_t i0;
   float l;
   float w;
   float u;
   uint32_t tmp;
   flen = flen*subsamp;
   w = X2C_DIVR(6.283185307f*fg0,(float)subsamp);
   m = flen/2UL;
   fir[m] = 1.0f;
   tmp = flen/2UL;
   i0 = 1UL;
   if (i0<=tmp) for (;; i0++) {
      u = (float)i0*w;
      u = (float)(X2C_DIVL(sin((double)u),(double)u));
      if (m+i0<flen) fir[m+i0] = u;
      fir[m-i0] = u;
      if (i0==tmp) break;
   } /* end for */
   l = fg0*2.0f*gain0; /* gain compensation */
   u = X2C_DIVR(2.0f,(float)flen);
   tmp = flen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      /* hamming */
      fir[i0] = (float)((double)(l*fir[i0])
                *(0.54+0.46*cos((double)(3.1415926535f*((float)
                i0*u-1.0f)))));
      if (i0==tmp) break;
   } /* end for */
} /* end mkfir() */


static void mkdds(void)
{
   uint32_t q;
   uint32_t i0;
   float r;
   r = 9.5873799240112E-5f;
   q = 16384UL;
   for (i0 = 0UL; i0<=65535UL; i0++) {
      DDS[i0].si = (float)sin((double)((float)i0*r));
      DDS[i0+q&65535UL].co = DDS[i0].si;
   } /* end for */
} /* end mkdds() */


static void WrStat(void)
{
   char hh[101];
   char h[101];
   float v;
   strncpy(h,"Peak:",101u);
   v = outpeak;
   if (outform==1UL) v = v*256.0f;
   else if (outform>1UL) v = v*7.8125E-3f;
   aprsstr_FixToStr(v, 3UL, hh, 101ul);
   aprsstr_Append(h, 101ul, hh, 101ul);
   if (outform==0UL) v = v*7.843137254902E-1f;
   else if (outform==1UL) v = v*3.0518043793393E-3f;
   if (outform<=1UL) {
      aprsstr_Append(h, 101ul, "(", 2ul);
      aprsstr_FixToStr(v, 2UL, hh, 101ul);
      aprsstr_Append(h, 101ul, hh, 101ul);
      aprsstr_Append(h, 101ul, "%) Limited:", 12ul);
      aprsstr_FixToStr(X2C_DIVR((float)limcnt*100.0f,(float)outsamps),
                4UL, hh, 101ul);
      aprsstr_Append(h, 101ul, hh, 101ul);
      aprsstr_Append(h, 101ul, "%", 2ul);
   }
   aprsstr_Append(h, 101ul, "\012", 2ul);
   osi_Werr(h, 101ul);
   outpeak = 0.0f;
   limcnt = 0UL;
   outsamps -= statistic;
} /* end WrStat() */


static void WrSamp(float s)
{
   int32_t n;
   if (statistic) {
      ++outsamps;
      if ((float)fabs(s)>outpeak) outpeak = (float)fabs(s);
      if (outsamps>=statistic) WrStat();
   }
   if (outform==0UL) {
      if (s>127.99f) {
         s = 127.99f;
         ++limcnt;
      }
      else if (s<(-127.99f)) {
         s = (-127.99f);
         ++limcnt;
      }
      ob[wp] = (uint8_t)(uint32_t)X2C_TRUNCC(s+128.0f,0UL,
                X2C_max_longcard);
      ++wp;
   }
   else if (outform==1UL) {
      s = s*256.0f;
      if (s>32766.9f) {
         s = 32767.0f;
         ++limcnt;
      }
      else if (s<(-3.2767E+4f)) {
         s = (-3.2767E+4f);
         ++limcnt;
      }
      n = (int32_t)X2C_TRUNCI(s,X2C_min_longint,X2C_max_longint);
      ob[wp] = (uint8_t)n;
      ++wp;
      ob[wp] = (uint8_t)(n>>8);
      ++wp;
   }
   else {
      s = s*7.8125E-3f; /* float is -1 to 1 */
      n = (int32_t)*X2C_CAST(&s,float,uint32_t,uint32_t *);
      ob[wp] = (uint8_t)n;
      ++wp;
      ob[wp] = (uint8_t)(n>>8);
      ++wp;
      ob[wp] = (uint8_t)(n>>16);
      ++wp;
      ob[wp] = (uint8_t)(n>>24);
      ++wp;
   }
   if (wp>32767UL) {
      osi_WrBin(fo, (char *)ob, 32768u/1u, wp);
      wp = 0UL;
   }
} /* end WrSamp() */


static float inword(void)
{
   int32_t i0;
   uint32_t e;
   uint32_t c;
   float r;
   if (prefill>0L) {
      --prefill;
      return 0.0f;
   }
   if (pb>=lenb) {
      i0 = osi_RdBin(fi, (char *)bb, 32768u/1u, 32768UL);
      if (i0<=0L) {
         if (prefill>=-(int32_t)(firlen-1UL)) --prefill;
         else eof = 1;
         return 0.0f;
      }
      lenb = (uint32_t)i0;
      pb = 0UL;
   }
   if (inform==0UL) {
      ++pb;
      return (float)(uint32_t)(uint8_t)bb[pb-1UL]-127.5f;
   }
   else if (inform==1UL) {
      pb += 2UL;
      return (float)(short)(((uint32_t)(uint8_t)bb[pb-1UL]<<8)
                +(uint32_t)(uint8_t)bb[pb-2UL])*3.90625E-3f;
   }
   else {
      pb += 4UL;
      c = ((uint32_t)(uint8_t)bb[pb-1UL]<<24)+((uint32_t)(uint8_t)
                bb[pb-2UL]<<16)+((uint32_t)(uint8_t)bb[pb-3UL]<<8)
                +(uint32_t)(uint8_t)bb[pb-4UL];
      e = c/16777216UL;
      if (e==127UL) return 127.0f;
      if (e==255UL) return (-127.0f);
      r = *X2C_CAST(&c,uint32_t,float,float *);
      if (r>1.0f) r = 1.0f;
      else if (r<(-1.0f)) r = (-1.0f);
      return r*127.0f;
   }
   return 0;
} /* end inword() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   uint32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   /*  verb:=TRUE; */
   statistic = 0UL;
   outpeak = 0.0f;
   outsamps = 0UL;
   limcnt = 0UL;
   inform = 0UL;
   outform = 0UL;
   firlen = 64UL;
   fg = 0.0f;
   gain = 1.0f;
   shiftstep = 0UL;
   sr = 0.0f;
   samprate = 65536UL;
   for (;;) {
      osi_NextArg(as, 4096ul);
      if (as[0U]==0) break;
      if ((as[0U]=='-' && as[1U]) && as[2U]==0) {
         if (as[0U]=='-') {
            if (as[1U]=='i') osi_NextArg(fni, 4096ul);
            else if (as[1U]=='o') osi_NextArg(fno, 4096ul);
            else if (as[1U]=='s') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) {
                  Err("-s <shift>", 11ul);
               }
               if (val>0.5f || val<(-0.5f)) {
                  Err("shift range +-0.5 (of sampelrate)", 34ul);
               }
               shiftstep = (uint32_t)(int32_t)
                X2C_TRUNCI(val*4.29496715E+9f,X2C_min_longint,
                X2C_max_longint);
            }
            else if (as[1U]=='r') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) {
                  Err("-r <ratio>", 11ul);
               }
               if (val<0.02f || val>5.0f) {
                  Err("ratio range 0.001..5.0 (of input sampelrate)", 45ul);
               }
               samprate = (uint32_t)(int32_t)
                X2C_TRUNCI(X2C_DIVR(65536.0f,val),X2C_min_longint,
                X2C_max_longint);
               sr = val;
            }
            else if (as[1U]=='a') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) {
                  Err("-a <aliasingfilter>", 20ul);
               }
               if (val<=0.0f || val>1.0f) {
                  Err("filter range 0.0..1.0 (of input sampelrate)", 44ul);
               }
               fg = val*0.5f;
            }
            else if (as[1U]=='l') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &firlen)) {
                  Err("-l <fir length>", 16ul);
               }
               if (firlen>4096UL || firlen<4UL) {
                  Err("max fir length 4096", 20ul);
               }
            }
            else if (as[1U]=='F') {
               osi_NextArg(as, 4096ul);
               if (as[0U]=='u' && as[1U]=='8') inform = 0UL;
               else if ((as[0U]=='i' && as[1U]=='1') && as[2U]=='6') {
                  inform = 1UL;
               }
               else if ((as[0U]=='f' && as[1U]=='3') && as[2U]=='2') {
                  inform = 2UL;
               }
               else Err("input formats u8 i16 f32", 25ul);
            }
            else if (as[1U]=='f') {
               osi_NextArg(as, 4096ul);
               if (as[0U]=='u' && as[1U]=='8') outform = 0UL;
               else if ((as[0U]=='i' && as[1U]=='1') && as[2U]=='6') {
                  outform = 1UL;
               }
               else if ((as[0U]=='f' && as[1U]=='3') && as[2U]=='2') {
                  outform = 2UL;
               }
               else Err("output formats u8 i16 f32", 26ul);
            }
            else if (as[1U]=='g') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&gain, as, 4096ul)) {
                  Err("-g <factor>", 12ul);
               }
            }
            else if (as[1U]=='v') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &statistic)) {
                  Err("-v <cnt>", 9ul);
               }
            }
            else if (as[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn(" shift frequency and up/downsample iq-file",
                43ul);
               osi_WrStrLn(" -a <aliasingfilter> manual set antialiasing filt\
er 0.0..1.0 else automatic", 76ul);
               osi_WrStrLn(" -F <input format>   u8 i16 f32 (u8)", 37ul);
               osi_WrStrLn(" -f <output format>  u8 i16 f32 (u8)", 37ul);
               osi_WrStrLn(" -g <factor>         gain (1.0)", 32ul);
               osi_WrStrLn(" -h", 4ul);
               osi_WrStrLn(" -i <infile>         iq file", 29ul);
               osi_WrStrLn(" -l <fir length>     fir length 4..4096 with line\
ar more cpu usage (64)", 72ul);
               osi_WrStrLn(" -o <outfile>        iq file", 29ul);
               osi_WrStrLn(" -r <ratio>          output to input sampelrate 0\
.02..5.0 (1.0)", 64ul);
               osi_WrStrLn(" -s <shift>          shift frequency before resam\
ple -0.5..0.5 of iq rate (0.0)", 80ul);
               osi_WrStrLn(" -v <cnt>            show output level, overdrive\
s on stderr every <cnt> output sample (0)", 91ul);
               X2C_ABORT();
            }
            else Err(as, 4096ul);
         }
         else Err("use -h", 7ul);
      }
      else Err("use -h", 7ul);
   }
   srin = sr;
   if (srin>1.0f) srin = 1.0f;
   if ((fg==0.0f && srin!=0.0f) && firlen>=8UL) {
      fg = srin*0.5f-X2C_DIVR(1.2f,(float)firlen);
                /* auto aliasingfilter */
   }
   if (fg<=0.005f) {
      if (sr!=0.0f) {
         Err("(auto) aliasing fail, set filter manual (-a) and/or use longer \
filter (-l) ", 76ul);
      }
      fg = 0.5f; /* like pass thru */
   }
   fi = osi_OpenRead(fni, 4096ul);
   if (fi<0L) Err("iq file open", 13ul);
   fo = osi_OpenWrite(fno, 4096ul);
   if (fo<0L) Err("iq file write", 14ul);
   shiftout = 0;
   shiftin = 0;
   if (shiftstep>0UL) {
      mkdds();
      if (sr>1.0f) shiftout = 1;
      else shiftin = 1;
   }
   mkfir(fg, gain, 256UL, firlen, firraw, 1048576ul);
   tmp = firlen*256UL-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      firtab[i] = firraw[(((i%firlen)*256UL+256UL)-1UL)-i/firlen];
      if (i==tmp) break;
   } /* end for */
   prefill = (int32_t)firlen;
   lenb = 0UL;
   pb = 0UL;
   eof = 0;
   wp = 0UL;
   rp = 0UL;
   phasereg = 0UL;
   wp = 0UL;
   sampc = 0UL;
   rl = 0UL;
   do {
      do {
         { /* with */
            struct Complex * anonym = &ib[rl];
            re = inword();
            im = inword();
            /*shift */
            if (shiftin) {
               { /* with */
                  struct _0 * anonym0 = &DDS[phasereg/65536UL];
                  anonym->Re = re*anonym0->si-im*anonym0->co;
                  anonym->Im = re*anonym0->co+im*anonym0->si;
               }
               phasereg += shiftstep;
            }
            else {
               /*shift */
               anonym->Re = re;
               anonym->Im = im;
            }
         }
         ++rl;
      } while (!(eof || rl>32767UL));
      /*fir */
      sampc = sampc&65535UL;
      for (;;) {
         rp = sampc>>16;
         fpend = rp+firlen;
         if (fpend>=rl) break;
         sum.Re = 0.0f;
         sum.Im = 0.0f;
         fp = (sampc>>8&255UL)*firlen; /* intepolate */
         do {
            f = firtab[fp];
            sum.Re = sum.Re+ib[rp].Re*f;
            sum.Im = sum.Im+ib[rp].Im*f;
            ++fp;
            ++rp;
         } while (rp<fpend);
         /*shift */
         if (shiftout) {
            { /* with */
               struct _0 * anonym1 = &DDS[phasereg/65536UL];
               WrSamp(sum.Re*anonym1->si-sum.Im*anonym1->co);
               WrSamp(sum.Re*anonym1->co+sum.Im*anonym1->si);
            }
            phasereg += shiftstep;
         }
         else {
            /*shift */
            WrSamp(sum.Re);
            WrSamp(sum.Im);
         }
         sampc += samprate;
      }
      /*fir */
      if (rl>rp) {
         X2C_MOVE((char *) &ib[rp],(char *)ib,
                (rl-rp)*sizeof(struct Complex));
      }
      rl -= rp;
   } while (!eof);
   if (wp>0UL) osi_WrBin(fo, (char *)ob, 32768u/1u, wp);
   if (statistic && outsamps>0UL) WrStat();
   osic_Close(fi);
   osic_Close(fo);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
