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
#define sdrtx_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#include <signal.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef soundctl_H_
#include "soundctl.h"
#endif

/* audio to iq modulator am, fm, usb, lsb, wfm-stereo by oe5dxl */
#define sdrtx_SAMPLES 2048

#define sdrtx_OSSSAMP 48000

#define sdrtx_LEVEL 2.925625E+4
/*      RADIOPREEM=0.88; */

#define sdrtx_RADIOFG 18000

#define sdrtx_RADIOFIRLEN 64

#define sdrtx_LF "\012"

#define sdrtx_PI 3.1415926535

#define sdrtx_INTPOLS 64

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

enum MODS {sdrtx_mFM, sdrtx_mUSB, sdrtx_mLSB, sdrtx_mAM, sdrtx_mSTEREO,
                sdrtx_mUPSAMP};


struct STEREOCONTEXT;


struct STEREOCONTEXT {
   float lastl;
   float lastr;
   float wpilot;
};

static short buf[4096];

static int32_t adc;

static int32_t i;

static int32_t tmpi;

static int32_t bi;

static int32_t bq;

static int32_t si;

static int32_t sq;

static int32_t wshift;

static int32_t shift;

static int32_t of;

static uint32_t is;

static uint32_t p;

static uint32_t adcrate;

static uint32_t oform;

static uint32_t pilotu;

static uint32_t wpilotu;

static uint32_t loops;

static uint32_t firsteps;

static uint32_t fs;

static uint32_t fh;

static float asamp;

static float limlev;

static float ohp;

static float ohd;

static float levmul;

static float wsub;

static float ssbwinc;

static float pilot;

static float clipplev;

static float upsamplef;

static float offset;

static float bbi;

static float bbq;

static float sampmul;

static float sampcnt;

static float fmdeviation;

static float devi;

static float miclowpass;

static float preemfilter;

static float bassfilter;

static float limlim;

static float sample;

static double limmul;

static struct LPCONTEXT12 upsamplelpi;

static struct LPCONTEXT12 upsamplelpq;

static struct LPCONTEXT24 miclp;

static struct LPCONTEXT24 ssblpi;

static struct LPCONTEXT24 ssblpq;

static char exit0;

static short abuf[1024];

static char ifn[1024];

static char ofn[1024];

static uint32_t pabuf;

static uint32_t abuflen;

static uint8_t mod;

static struct STEREOCONTEXT cstereo;

static short sintab[32768];

static float radiofirtab[64][64];

static float radiofirl[64];

static float radiofirr[64];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void Parms(void)
{
   char err;
   char h[1024];
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='m') {
            osi_NextArg(h, 1024ul);
            if (h[0U]=='u') mod = sdrtx_mUSB;
            else if (h[0U]=='l') mod = sdrtx_mLSB;
            else if (h[0U]=='a') mod = sdrtx_mAM;
            else if (h[0U]=='f') mod = sdrtx_mFM;
            else if (h[0U]=='s') mod = sdrtx_mSTEREO;
            else Error("-m a|f|u|l|s", 13ul);
         }
         else if (h[1U]=='o') {
            osi_NextArg(ofn, 1024ul);
            if (ofn[0U]==0 || ofn[0U]=='-') Error("-o <iq-filename>", 17ul);
         }
         else if (h[1U]=='i') {
            osi_NextArg(ifn, 1024ul);
            if (ifn[0U]==0 || ifn[0U]=='-') {
               Error("-i <sound-filename>", 20ul);
            }
         }
         else if (h[1U]=='s') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&offset, h, 1024ul)) {
               Error("-s <offset-Hz>", 15ul);
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&devi, h, 1024ul)) {
               Error("-d <devi-Hz>", 13ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&levmul, h, 1024ul)) {
               Error("-g <gainfactor>", 16ul);
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&clipplev, h, 1024ul)) {
               Error("-c <rfclipper>", 15ul);
            }
         }
         else if (h[1U]=='u') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&upsamplef, h, 1024ul)) {
               Error("-u <upsamplefiltHz>", 20ul);
            }
         }
         else if (h[1U]=='l') {
            /*
                 ELSIF h[1]="a" THEN
                    NextArg(h);
                    IF NOT StrToFix(limlim,
                h) OR (limlim<0.0) THEN Error("-u <autogainlimit>") END;
                    limlim:=1.0/(limlim+1.0);
            */
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&miclowpass, h, 1024ul)) {
               Error("-l <lowpass-Hz>", 16ul);
            }
         }
         else if (h[1U]=='p') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&preemfilter, h,
                1024ul) || preemfilter>1.0f) {
               Error("-p <preemphase 0..1>", 21ul);
            }
         }
         else if (h[1U]=='b') {
            /*        IF preemfilter<>0.0 THEN preemfilter:=1.0/preemfilter END;
                 */
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&bassfilter, h, 1024ul)) {
               Error("-b <bassfilter>", 16ul);
            }
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if ((h[0U]=='i' && h[1U]=='1') && h[2U]=='6') oform = 2UL;
            else if ((h[0U]=='f' && h[1U]=='3') && h[2U]=='2') {
               oform = 4UL;
            }
            else if (h[0U]=='u' && h[1U]=='8') oform = 1UL;
            else Error("-f u8|i16|f32", 14ul);
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&sampmul, h, 1024ul) || sampmul<2.0f) {
               Error("-r <samplemultiplier>", 22ul);
            }
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn(" Modulate audio to IQ-File", 27ul);
               osi_WrStrLn(" -b <0..1>           filter bass of modulation (n\
ot stereo) (0.05)", 67ul);
               osi_WrStrLn(" -c <v>              ssb rf-clipper level after A\
LC 1=off (1.0)", 64ul);
               osi_WrStrLn(" -d <Hz>             fm deviation (3000) (stereo \
40000)", 56ul);
               osi_WrStrLn(" -f <output format>  u8 i16 f32 (u8)", 37ul);
               osi_WrStrLn(" -g <v>              input loudness (not stereo) \
more for more ALC (50.0)", 74ul);
               osi_WrStrLn(" -h                  this", 26ul);
               osi_WrStrLn(" -i <filename>       input file/pipe/oss-device (\
stereo 2 channel 48kHz) (/dev/dsp)", 84ul);
               osi_WrStrLn(" -l <Hz>             modulation lowpass (not ster\
eo) 0=off (3000)", 66ul);
               osi_WrStrLn(" -m <modulation>     u(sb) l(sb) a(m) f(m) s(ster\
eo) (f)", 57ul);
               osi_WrStrLn(" -o <filename>       output iq file/pipe", 41ul);
               osi_WrStrLn(" -p <0..1>           preemphase of modulation (no\
t stereo) 0=off (0.8)", 71ul);
               osi_WrStrLn(" -r <n>              output/input samplerate (21.\
33333)", 56ul);
               osi_WrStrLn(" -s <Hz>             shift signal from iq band ce\
nter (0)", 58ul);
               osi_WrStrLn(" -u <Hz>             upsampler aliasing filter (6\
000) (stereo 22000)", 69ul);
               osi_WrStrLn(" all <Hz>-values based on 48kHz input samplerate \
and stereo pilot tone", 71ul);
               osi_WrStrLn(" filters 4th order IIR, stereo preemphase+17kHz l\
owpass 64 tap FIR", 67ul);
               osi_WrStrLn("", 1ul);
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
} /* end Parms() */


static void OpenSound(uint32_t adcrate0, const char fn[],
                uint32_t fn_len)
{
   int32_t i0;
   adc = osi_OpenRead(fn, fn_len);
   if (adc>=0L) {
      i0 = samplesize(adc, 16UL); /* 8, 16 */
      i0 = channels(adc, 1UL+(uint32_t)(mod==sdrtx_mSTEREO)); /* 1, 2  */
      i0 = sampelrate(adc, adcrate0); /* 8000..48000 */
   }
   else Error("sound device open", 18ul);
} /* end OpenSound() */


static float audiosamp(void)
{
   int32_t i0;
   if (pabuf>=abuflen) {
      i0 = osi_RdBin(adc, (char *)abuf, 2048u/1u, 2048UL);
      if (i0<=0L) {
         exit0 = 1;
         return 0.0f;
      }
      abuflen = (uint32_t)(i0/2L);
      pabuf = 0UL;
   }
   i0 = (int32_t)abuf[pabuf];
   ++pabuf;
   return (float)i0;
} /* end audiosamp() */

/*
PROCEDURE audioout(o:REAL);
VAR i:INTEGER;
BEGIN
  IF pobuf>HIGH(about) THEN
    WrBin(adc, about, SIZE(about));
    pobuf:=0; 
  END;
  i:=VAL(INTEGER, o);
  IF i>32767 THEN i:=32767 ELSIF i<-32767 THEN i:=-32767 END;
  about[pobuf]:=i;
  INC(pobuf);
END audioout;
*/

static void mkfir(float gain, float interp, uint32_t flen,
                float fir[], uint32_t fir_len)
{
   uint32_t m;
   uint32_t f;
   uint32_t i0;
   float g;
   float w;
   float u;
   uint32_t tmp;
   uint32_t tmp0;
   m = flen/2UL;
   tmp = flen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      fir[i0] = 0.0f;
      if (i0==tmp) break;
   } /* end for */
   tmp = (uint32_t)X2C_TRUNCC(0.75f*(float)m,0UL,X2C_max_longcard)-1UL;
   f = 1UL;
   if (f<=tmp) for (;; f++) {
      g = X2C_DIVR((float)f,(float)m)+0.25f; /* preemphase */
      w = X2C_DIVR((float)f*2.0f*3.1415926535f,(float)flen);
      tmp0 = flen-1UL;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         fir[i0] = fir[i0]+g*osic_cos((((float)i0-(float)m)+interp)*w);
         if (i0==tmp0) break;
      } /* end for */
      if (f==tmp) break;
   } /* end for */
   u = X2C_DIVR(2.0f,(float)flen);
   /*WrStrLn(""); */
   tmp = flen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      /* hamming */
      fir[i0] = gain*fir[i0]*(0.54f+0.46f*osic_cos(3.1415926535f*((float)
                i0*u-1.0f)));
      if (i0==tmp) break;
   } /* end for */
/*WrFixed(fir[i], 3,1);WrStr(" "); */
/*WrStrLn(""); */
} /* end mkfir() */


static float dofir(float u, float f[], uint32_t f_len,
                const float tab[], uint32_t tab_len, uint32_t flen)
{
   uint32_t i0;
   float s;
   uint32_t tmp;
   s = f[0UL]*tab[0UL];
   f[flen-1UL] = u;
   tmp = flen-2UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      f[i0] = f[i0+1UL];
      s = s+f[i0]*tab[i0];
      if (i0==tmp) break;
   } /* end for */
   return s;
} /* end dofir() */


static float dofiri(float f[], uint32_t f_len, const float tab[],
                uint32_t tab_len, uint32_t flen)
{
   uint32_t i0;
   float s;
   uint32_t tmp;
   s = 0.0f;
   tmp = flen-2UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      s = s+f[i0]*tab[i0];
      if (i0==tmp) break;
   } /* end for */
   return s;
} /* end dofiri() */


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
   o = in*levmul;
   if (miclowpass!=0.0f) o = lp(o, &miclp);
   if (bassfilter!=0.0f) {
      ohd = ohd+(o-ohd)*bassfilter; /* remove bass */
      o = o-ohd;
   }
   if (preemfilter!=0.0f) {
      ll = ohp; /* preemphase */
      ohp = o;
      /*    o:=o*preemfilter-ll;                          */
      o = o-ll*preemfilter;
   }
   o = (float)((double)o*(1.0-limmul));
   ll = (float)fabs(o)-limlev;
   if (ll>0.0f) {
      /* too loud */
      limmul = limmul+(1.0-limmul)*(double)ll*0.00002; /* 0.00002 */
      if (limmul>1.0) limmul = 1.0;
   }
   else limmul = limmul*9.99995E-1;
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


static float encstereo(float l, float r,
                struct STEREOCONTEXT * stereocontext)
{
   float s;
   struct STEREOCONTEXT * anonym;
   { /* with */
      struct STEREOCONTEXT * anonym = stereocontext;
      wpilotu += pilotu;
      s = 3.0517578125E-5f*(float)sintab[(uint32_t)X2C_LSH((uint32_t)
                wpilotu,32,-17)];
      return (r-l)*s*s+l+s*3276.8f;
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


static void upsample(float u, struct LPCONTEXT12 * c)
{
   struct LPCONTEXT12 * anonym;
   { /* with */
      struct LPCONTEXT12 * anonym = c;
      anonym->uc = anonym->uc+anonym->il;
      anonym->il = anonym->il*anonym->K2+(u-anonym->uc)*anonym->K1;
   }
} /* end upsample() */


static void SendSamps(short buf0[], uint32_t buf_len, uint32_t * p0)
{
   uint32_t i0;
   uint8_t b[4096];
   float f[4096];
   uint32_t tmp;
   if (oform==2UL) osi_WrBin(of, (char *)buf0, (buf_len*2u)/1u,  *p0*2UL);
   else if (oform==1UL) {
      tmp = *p0-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         b[i0] = (uint8_t)((buf0[i0]>>8)+128);
         if (i0==tmp) break;
      } /* end for */
      osi_WrBin(of, (char *)b, 4096u/1u, *p0);
   }
   else {
      tmp = *p0-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         f[i0] = (float)buf0[i0]*3.0517578125E-5f;
         if (i0==tmp) break;
      } /* end for */
      osi_WrBin(of, (char *)f, sizeof(float [4096])/1u,
                *p0*sizeof(float));
   }
   *p0 = 0UL;
} /* end SendSamps() */

static void exitprog(int32_t);


static void exitprog(int32_t signum)
{
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   exit0 = 1;
} /* end exitprog() */


X2C_STACK_LIMIT(100000l)
int main(int argc, char **argv)
{
   uint32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGTERM, exitprog);
   signal(SIGINT, exitprog);
   signal(SIGPIPE, exitprog);
   of = -1L;
   adcrate = 48000UL;
   sampmul = 2.1333333333333E+1f;
   mod = sdrtx_mFM;
   strncpy(ifn,"/dev/dsp",1024u);
   ofn[0] = 0;
   wshift = 0L;
   offset = 0.0f;
   devi = 0.0f;
   levmul = 50.0f;
   miclowpass = 3000.0f;
   bassfilter = 0.05f;
   preemfilter = 0.8f;
   clipplev = 1.0f;
   upsamplef = 0.0f;
   limlim = 0.0f;
   oform = 1UL;
   Parms();
   /*  ALLOCATE(buf, SIZE(buf^)); */
   /*  IF buf=NIL THEN Werr("out of memory") END; */
   if (devi==0.0f) {
      if (mod==sdrtx_mSTEREO) devi = 40000.0f;
      else devi = 3000.0f;
   }
   if (upsamplef==0.0f) {
      if (mod==sdrtx_mSTEREO) upsamplef = 22000.0f;
      else upsamplef = 6000.0f;
   }
   sample = (float)adcrate*sampmul;
   if (ofn[0U]==0) osi_Werr("output file?", 13ul);
   of = osi_OpenWrite(ofn, 1024ul);
   if (of<0L) osi_Werr("file write", 11ul);
   OpenSound(48000UL, ifn, 1024ul);
   makelp12(upsamplef, sample, &upsamplelpi);
   upsamplelpq = upsamplelpi;
   if (miclowpass!=0.0f) makelp24(miclowpass, (float)adcrate, &miclp);
   makelp24(1200.0f, (float)adcrate, &ssblpi);
   ssblpq = ssblpi;
   if (mod==sdrtx_mSTEREO) {
      for (i = 0L; i<=63L; i++) {
         mkfir(0.1f, X2C_DIVR((float)i+0.5f,64.0f), 64UL,
                radiofirtab[63L-i], 64ul);
      } /* end for */
   }
   for (i = 0L; i<=32767L; i++) {
      sintab[i] = (short)X2C_TRUNCI(32767.0f*osic_sin(X2C_DIVR((float)
                i*3.1415926535898f*2.0f,32768.0f)),-32768,32767);
   } /* end for */
   ssbwinc = X2C_DIVR(9.4247779607694E+3f,(float)adcrate);
   fmdeviation = X2C_DIVR(X2C_DIVR(devi*4.294967295E+9f,sample),
                2.925625E+4f);
   pilot = X2C_DIVR(1.1938052083641E+5f,sample);
   pilotu = (uint32_t)X2C_TRUNCC(X2C_DIVL(8.1604378624E+13,
                (double)sample),0UL,X2C_max_longcard);
   limlev = 2.6330625E+4f;
   shift = (int32_t)X2C_TRUNCI(-(X2C_DIVR(offset*4.294967295E+9f,sample)),
                X2C_min_longint,X2C_max_longint);
   pabuf = 2147483647UL;
   p = 0UL;
   sampcnt = 0.0f;
   firsteps = (uint32_t)X2C_TRUNCC(sampmul,0UL,X2C_max_longcard)/2UL;
                /* make fir interpolation steps */
   /*WrStrLn(""); */
   do {
      sampcnt = sampcnt+sampmul;
      loops = (uint32_t)X2C_TRUNCC(sampcnt,0UL,X2C_max_longcard);
      if (mod<sdrtx_mSTEREO) {
         asamp = audioproc(audiosamp());
         if (mod==sdrtx_mUSB) ssbtx(asamp*clipplev, &bbi, &bbq);
         else if (mod==sdrtx_mLSB) ssbtx(asamp*clipplev, &bbq, &bbi);
         else if (mod==sdrtx_mFM) bbi = asamp*fmdeviation;
         else if (mod==sdrtx_mAM) bbi = asamp;
      }
      else {
         bbi = dofir(audiosamp(), radiofirl, 64ul, radiofirtab[0U], 64ul,
                64UL);
         bbq = dofir(audiosamp(), radiofirr, 64ul, radiofirtab[0U], 64ul,
                64UL);
      }
      /*    bbi:=4.0*lp(preem(audiosamp(), cstereo.lastl), leftlp); */
      /*    bbq:=4.0*lp(preem(audiosamp(), cstereo.lastr), rightlp); */
      /*    bbi:=2.0*preem(audiosamp(), cstereo.lastl); */
      /*    bbq:=2.0*preem(audiosamp(), cstereo.lastr); */
      fs = 0UL;
      tmp = loops-1UL;
      is = 0UL;
      if (is<=tmp) for (;; is++) {
         if (mod==sdrtx_mFM) {
            upsample(bbi, &upsamplelpi);
            buf[p] = sintab[(uint32_t)X2C_LSH((uint32_t)wshift,32,-17)];
            ++p;
            buf[p] = sintab[(uint32_t)X2C_LSH((uint32_t)
                (wshift+1073741824L),32,-17)];
            ++p;
            wshift = wshift+shift+(int32_t)X2C_TRUNCI(upsamplelpi.uc,
                X2C_min_longint,X2C_max_longint);
         }
         else if (mod==sdrtx_mAM) {
            /*WrFixed(bbi, 3, 1); WrStr(" "); */
            upsample(bbi, &upsamplelpi);
            si = (int32_t)X2C_TRUNCI(upsamplelpi.uc,X2C_min_longint,
                X2C_max_longint)+32768L>>1;
            if (si<0L) si = 0L;
            else if (si>32767L) si = 32767L;
            buf[p] = (short)(si*(int32_t)sintab[(uint32_t)
                X2C_LSH((uint32_t)wshift,32,-17)]>>15);
            ++p;
            buf[p] = (short)(si*(int32_t)sintab[(uint32_t)
                X2C_LSH((uint32_t)(wshift+1073741824L),32,-17)]>>15);
            ++p;
            wshift += shift;
         }
         else if (mod==sdrtx_mSTEREO) {
            /*        IF is+is=TRUNC(sampmul)
                THEN                                   (* double audio samplerate *)
                 */
            /*          bbi:=dofiri(radiofirl, radiofirtab[INTPOLS DIV 2],
                RADIOFIRLEN); */
            /*          bbq:=dofiri(radiofirr, radiofirtab[INTPOLS DIV 2],
                RADIOFIRLEN); */
            /*        END; */
            if (fs>=firsteps) {
               fh = (is*64UL)/loops;
               /*WrInt(fh,1); WrStr(" "); */
               bbi = dofiri(radiofirl, 64ul, radiofirtab[fh], 64ul, 64UL);
               bbq = dofiri(radiofirr, 64ul, radiofirtab[fh], 64ul, 64UL);
               fs = 0UL;
            }
            ++fs;
            upsample(bbi, &upsamplelpi);
            upsample(bbq, &upsamplelpq);
            buf[p] = sintab[(uint32_t)X2C_LSH((uint32_t)wshift,32,-17)];
            ++p;
            buf[p] = sintab[(uint32_t)X2C_LSH((uint32_t)
                (wshift+1073741824L),32,-17)];
            ++p;
            wshift = wshift+shift+(int32_t)
                X2C_TRUNCI(fmdeviation*encstereo(upsamplelpi.uc,
                upsamplelpq.uc, &cstereo),X2C_min_longint,X2C_max_longint);
         }
         else {
            /* iq baseband upsample */
            upsample(bbi, &upsamplelpi);
            upsample(bbq, &upsamplelpq);
            si = (int32_t)sintab[(uint32_t)X2C_LSH((uint32_t)wshift,32,
                -17)];
            sq = (int32_t)sintab[(uint32_t)X2C_LSH((uint32_t)
                (wshift+1073741824L),32,-17)];
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
         if (p>4095UL) SendSamps(buf, 4096ul, &p);
         if (is==tmp) break;
      } /* end for */
      sampcnt = sampcnt-(float)loops;
   } while (!exit0);
   if (p>0UL) SendSamps(buf, 4096ul, &p);
   osic_Close(of);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
