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
#define fmrepeater_C_
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
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef afskmodemptt_H_
#include "afskmodemptt.h"
#endif
#include <signal.h>
#include <unistd.h>

/* subaudio detector, subaudio generator, audio filter, ptt */
static uint32_t fmrepeater_TONES[8] = {1209UL,1336UL,1475UL,1633UL,941UL,
                852UL,770UL,697UL};

#define fmrepeater_CODES " *0#D789C456B123A"

#define fmrepeater_NL "\012"

typedef char FILENAME[1024];

struct _0;


struct _0 {
   float L;
   float R;
   float il;
   float uc;
   float lev;
};

typedef struct _0 DTMF[8];

struct WAV;


struct WAV {
   int32_t wavfd;
   int32_t wavlen;
   int32_t wavp;
   char wavbuf[4096];
   uint32_t bytepersamp;
   uint32_t block;
   uint32_t channels0;
   float sampmul;
   float frac;
   float nextsamp;
   float lastsamp;
};

static int32_t i;

static int32_t cnt;

static uint32_t adcrate;

static uint32_t filloss;

static uint32_t soundretry;

static float subtonrx;

static float subtontx;

static float subtonlev;

static float clipplev;

static float beepton;

static float exitsq;

static float exitnosq;

static float beepwait;

static float beepend;

static float ptthold;

static float beeplev;

static float highpass;

static float lowpass;

static float limlev;

static float volume;

static float thsqopen;

static float thsqclose;

static float wavlev;

static float wavstart;

static float beaconopen;

static float beaconclosed;

static float beaconlonesome;

static char notch;

static char verb;

static char pttstate;

static char * hptt;

static struct WAV wav;

static FILENAME soundfn;

static FILENAME soundfnout;

static FILENAME dtmffn;

static FILENAME wavfn;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static float sqr(float x)
{
   return x*x;
} /* end sqr() */


static float tone(float f, float * w)
{
   *w = *w+f;
   while (*w>6.2831853071796f) *w = *w-6.2831853071796f;
   return osic_sin(*w);
} /* end tone() */


static void Parms(void)
{
   char err;
   FILENAME h1;
   FILENAME h;
   char ptth[4096];
   uint32_t vc;
   int32_t vi;
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &vc)) err = 1;
            if (vc<8000UL || vc>96000UL) {
               Error("sampelrate 8000..96000", 23ul);
            }
            adcrate = vc;
         }
         else if (h[1U]=='a') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &soundretry)) err = 1;
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &filloss)) err = 1;
         }
         else if (h[1U]=='o') {
            osi_NextArg(soundfn, 1024ul);
            vc = 0UL;
            for (;;) {
               if (vc>1023UL) break;
               if (soundfn[vc]==0) break;
               if (soundfn[vc]==',') {
                  soundfn[vc] = 0;
                  vi = 0L;
                  ++vc;
                  while (vc<=1023UL && soundfn[vc]) {
                     soundfnout[vi] = soundfn[vc];
                     ++vc;
                     ++vi;
                  }
                  soundfnout[vi] = 0;
                  break;
               }
               ++vc;
            }
         }
         else if (h[1U]=='p') {
            osi_NextArg(h1, 1024ul);
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &vi)) err = 1;
            vi = labs(vi)+1L;
            if (h[0U]=='-') vi = -vi;
            hptt = pttinit((char *)h1, vi);
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&subtonrx, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='T') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&subtontx, h, 1024ul)) err = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&subtonlev, h, 1024ul)) err = 1;
            subtonlev = subtonlev*32767.0f;
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beepwait, h, 1024ul)) err = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beepend, h, 1024ul)) err = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beepton, h, 1024ul)) err = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beeplev, h, 1024ul)) err = 1;
            beeplev = beeplev*32767.0f;
         }
         else if (h[1U]=='P') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&ptthold, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='e') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&exitnosq, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='E') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&exitsq, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='W') {
            osi_NextArg(wavfn, 1024ul);
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&wavstart, h, 1024ul)) err = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&wavlev, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='B') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beaconopen, h, 1024ul)) err = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beaconclosed, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='I') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&beaconlonesome, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='N') notch = 1;
         else if (h[1U]=='d') osi_NextArg(dtmffn, 1024ul);
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='H') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&highpass, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='L') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&lowpass, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='G') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&volume, h, 1024ul)) err = 1;
         }
         else if (h[1U]=='S') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&thsqopen, h, 1024ul)) err = 1;
            thsqopen = thsqopen+1.0f;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&thsqclose, h, 1024ul)) err = 1;
            thsqclose = thsqclose+1.0f;
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&clipplev, h, 1024ul)) err = 1;
            clipplev = clipplev*32767.0f;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&limlev, h, 1024ul)) err = 1;
            limlev = limlev*32767.0f;
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn(" Soundcard (oss) FM-Repeater with ctcss squelch, \
ctcss generator, filters,", 75ul);
               osi_WrStrLn(" rogerbeep, beacon (wav file) and PTT (LPT, RS232\
, parport pins). Needs rx", 75ul);
               osi_WrStrLn(" audio from 60Hz up (fsk out), same on tx if subt\
one needed.", 61ul);
               osi_WrStrLn(" Use audio pipe from/to alsa, sdr ...", 38ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("  -a <n>             retry <n> times to open soun\
d input (every 50ms)", 70ul);
               osi_WrStrLn("  -B <s> <s>         send wav every <s> if contin\
ous open else every <s>", 73ul);
               osi_WrStrLn("  -b <s> <s> <Hz> <level>  rogerbeep <delay> <dur\
ation> <tone> <level>", 71ul);
               osi_WrStrLn("  -C <level>         Clipp audio before lowpass (\
0..1)", 55ul);
               osi_WrStrLn("  -d <filename>      dtmf on and send to file or \
pipe", 54ul);
               osi_WrStrLn("  -E <s>             program exit after <s> squel\
ch open (0)", 61ul);
               osi_WrStrLn("  -e <s>             program exit after <s> squel\
ch close (0)", 62ul);
               osi_WrStrLn("  -f <n>             fill oss with quietness to r\
epair underrun bug (0)", 72ul);
               osi_WrStrLn("  -G <level>         input gain before audio proc\
essing (1.0) (use with limiter)", 81ul);
               osi_WrStrLn("  -H <Hz>            highpass (filter out all sub\
audio band", 60ul);
               osi_WrStrLn("  -h                 help", 26ul);
               osi_WrStrLn("  -I <s>             send wav every <s> if contin\
ous closed", 60ul);
               osi_WrStrLn("  -L <Hz>            lowpass (beware modulator fr\
om high audio frequencies", 75ul);
               osi_WrStrLn("  -l <level>         limit audio level", 39ul);
               osi_WrStrLn("  -N                 notch out received subtone t\
o tx", 54ul);
               osi_WrStrLn("  -o <filename>      oss devicename (/dev/dsp)",
                47ul);
               osi_WrStrLn("  -o <infilename>,<outfilename>       /dev/stdin,\
outpipe  (mknod outpipe p)", 76ul);
               osi_WrStrLn("                       or in/out-pipe from SDR, a\
fskmodem -D or alsa interface", 79ul);
               osi_WrStrLn("  -o <infilename>,-  detect subtone, dtmf with no\
 sound out", 60ul);
               osi_WrStrLn("  -P <s>             PTT hold time (1)", 39ul);
               pttHelp((char *)ptth, 4096UL);
               osi_WrStrLn(ptth, 4096ul);
               osi_WrStrLn("  -r <Hz>            adcrate (22050) (8000..96000\
) (filters tested with 22050)", 79ul);
               osi_WrStrLn("  -S <level> <level> sqelch threshold <open> <clo\
se> (0.3 0.5)", 63ul);
               osi_WrStrLn("                       more open: save stay close\
 but needs exacter tone", 73ul);
               osi_WrStrLn("                       more close: stable open bu\
t longer noise tail", 69ul);
               osi_WrStrLn("  -T <Hz> <level>    tx subtone (67..254) (0.0..1\
.0) (0)", 57ul);
               osi_WrStrLn("  -t <Hz>            rx subtone (67..254) (88.5)",
                 49ul);
               osi_WrStrLn("  -v                 Verbous", 29ul);
               osi_WrStrLn("  -W <soundfile> <s> <level>  play wav after <s> \
squelch close with <level> 0..1", 81ul);
               osi_WrStrLn("                       wav type pcm 8/16bit 1/2ch\
an any sampelrate, best as -r", 79ul);
               osi_WrStrLn("repeater: ./repeater -r 22050 -o /dev/dsp -p /dev\
/ttyS0 0 -L 4000 -N -l 0.5 -t 88.5 -b 0.5 0.1 1500 0.2", 104ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */


static void buildresonance(float f, float adcf, float bw,
                float * r, float * l)
/* L R (C=1) for resonance iir */
{
   *l = X2C_DIVR(39.488f,sqr(X2C_DIVR(adcf,f-sqr(f)*1.1E-5f)));
   *r = X2C_DIVR(3.7f*bw,(float)adcrate);
} /* end buildresonance() */


static void dtmfeval(const DTMF dtmfs, float * dtmfmaxl,
                float * dtmfmaxh, uint32_t * key)
{
   uint32_t i0;
   float maxh;
   float maxl;
   uint32_t ch;
   uint32_t cl;
   uint32_t dh;
   uint32_t dl;
   maxl = 0.0f;
   maxh = 0.0f;
   for (i0 = 0UL; i0<=3UL; i0++) {
      if (dtmfs[i0].lev>maxl) {
         maxl = dtmfs[i0].lev;
         dl = i0;
      }
      if (dtmfs[i0+4UL].lev>maxh) {
         maxh = dtmfs[i0+4UL].lev;
         dh = i0;
      }
   } /* end for */
   *dtmfmaxl =  *dtmfmaxl*0.99f;
   *dtmfmaxh =  *dtmfmaxh*0.99f;
   if (maxl>*dtmfmaxl) *dtmfmaxl = maxl;
   if (maxh>*dtmfmaxh) *dtmfmaxh = maxh;
   if ( *dtmfmaxl*0.5f<maxl &&  *dtmfmaxh*0.5f<maxh) {
      maxl = maxl*0.5f;
      maxh = maxh*0.5f;
      cl = 0UL;
      ch = 0UL;
      for (i0 = 0UL; i0<=3UL; i0++) {
         if (dtmfs[i0].lev>=maxl) ++cl;
         if (dtmfs[i0+4UL].lev>=maxh) ++ch;
      } /* end for */
      if ((*key==0UL && cl==1UL) && ch==1UL) *key = 1UL+dl+dh*4UL;
   }
   else *key = 0UL;
} /* end dtmfeval() */


static void setptt(char on)
{
   if (pttstate!=on) {
      /* change */
      /*WrInt(ORD(on),1); WrStrLn(""); */
      ptt(hptt, (int32_t)(uint32_t)on);
      ptt(hptt, -1L);
      pttstate = on;
   }
} /* end setptt() */

/*------------ wav */

static void wavopen(const char fn[], uint32_t fn_len)
/* get wave header */
{
   char h[4096];
   uint32_t l;
   struct WAV * anonym;
   { /* with */
      struct WAV * anonym = &wav;
      if (anonym->wavfd>=0L) osic_Close(anonym->wavfd);
      anonym->wavfd = osi_OpenRead(fn, fn_len);
      anonym->wavlen = 0L;
      if (anonym->wavfd>=0L) {
         anonym->sampmul = 1.0f;
         anonym->block = 2UL;
         anonym->channels0 = 1UL;
         anonym->bytepersamp = 2UL;
         anonym->lastsamp = 0.0f;
         anonym->nextsamp = 0.0f;
         if ((((read(anonym->wavfd, (char *)h,
                20UL)==20L && h[0U]=='R') && h[1U]=='I') && h[2U]=='F')
                && h[3U]=='F') {
            l = 8UL+(uint32_t)(uint8_t)h[16U]+(uint32_t)(uint8_t)
                h[17U]*256UL+(uint32_t)(uint8_t)
                h[18U]*65536UL+(uint32_t)(uint8_t)h[19U]*16777216UL;
                /* + data chunk */
            if (l<=4096UL) {
               i = read(anonym->wavfd, (char *)h, l);
               if ((uint32_t)(uint8_t)h[0U]+(uint32_t)(uint8_t)
                h[1U]*256UL==1UL) {
                  /* PCM */
                  anonym->channels0 = (uint32_t)(uint8_t)h[2U];
                /* channels */
                  anonym->sampmul = (float)((uint32_t)(uint8_t)
                h[4U]+(uint32_t)(uint8_t)h[5U]*256UL+(uint32_t)
                (uint8_t)h[6U]*65536UL+(uint32_t)(uint8_t)
                h[7U]*16777216UL);
                  anonym->sampmul = X2C_DIVR(anonym->sampmul,
                (float)adcrate); /* relation wav to output samplerate */
                  anonym->block = (uint32_t)(uint8_t)h[12U]+(uint32_t)
                (uint8_t)h[13U]*256UL;
                  anonym->bytepersamp = ((uint32_t)(uint8_t)
                h[14U]+(uint32_t)(uint8_t)h[15U]*256UL+7UL)/8UL;
               }
            }
         }
      }
      else {
         /*WrInt(channels, 4); WrInt(TRUNC(samphz), 6); WrInt(block, 4);
                WrInt(bytepersamp, 4); WrStrLn(""); */
         osi_Werr("beacon file open error\012", 24ul);
      }
   }
} /* end wavopen() */


static short wavsamp(void)
/* get 1 wav sample */
{
   short o;
   struct WAV * anonym;
   o = 0;
   { /* with */
      struct WAV * anonym = &wav;
      if (anonym->wavfd>=0L) {
         if (anonym->wavlen==0L || anonym->wavp>=anonym->wavlen) {
            anonym->wavlen = read(anonym->wavfd, (char *)anonym->wavbuf,
                 4096UL);
            anonym->wavp = 0L;
         }
         if (anonym->wavlen<=0L) {
            osic_Close(anonym->wavfd);
            anonym->wavfd = -1L;
         }
         else {
            if (anonym->bytepersamp==1UL) {
               o = (short)((int32_t)(uint32_t)(uint8_t)
                anonym->wavbuf[anonym->wavp]*256L-32768L);
            }
            else {
               o = (short)((uint32_t)(uint8_t)
                anonym->wavbuf[anonym->wavp]+(uint32_t)(uint8_t)
                anonym->wavbuf[anonym->wavp+1L]*256UL);
            }
            anonym->wavp += (int32_t)anonym->block;
         }
      }
   }
   return o;
} /* end wavsamp() */


static float wavsample(void)
/* change samplerate with linear interpolation */
{
   float o;
   struct WAV * anonym;
   o = 0.0f;
   { /* with */
      struct WAV * anonym = &wav;
      if (anonym->wavfd>=0L) {
         while (anonym->frac>1.0f) {
            anonym->lastsamp = anonym->nextsamp;
            anonym->nextsamp = (float)wavsamp();
            anonym->frac = anonym->frac-1.0f;
         }
         o = anonym->lastsamp*(1.0f-anonym->frac)
                +anonym->nextsamp*anonym->frac;
         anonym->frac = anonym->frac+anonym->sampmul;
      }
   }
   return o;
} /* end wavsample() */

/*-------------------- wav */
static uint32_t _cnst[8] = {1209UL,1336UL,1475UL,1633UL,941UL,852UL,770UL,
                697UL};

static void audio(void)
{
   float dtmfmaxh;
   float dtmfmaxl;
   float blmedq;
   float blmed;
   float dmif;
   float sqi;
   float olpAR;
   float lpAL;
   float lpAR;
   float olpR;
   float lpL;
   float lpR;
   float limmul;
   float ll;
   float rahlah;
   float rah;
   float rhlh;
   float rh;
   float ilah2;
   float ilah1;
   float ucah;
   float uch;
   float ilh2;
   float ilh1;
   float ila;
   float uca2;
   float uca1;
   float ilr;
   float ucr;
   float il;
   float uc2;
   float uc1;
   float ui;
   float uin;
   float ilrn;
   float ucrn;
   float tonem;
   float noisem;
   float iyi;
   float ymi;
   float dmi;
   float vb;
   float uo;
   float dtmffilt;
   float dfilt;
   float rfilt;
   float tfilt;
   float nfilt;
   float lfilt;
   float beepf;
   float subtonf;
   float peepw;
   float subw;
   float subtonAR;
   float subtonR;
   float subtonL;
   double timestep;
   double opentime;
   double closedtime;
   char evertx;
   char wavdone;
   char wastx;
   char sqold;
   int32_t i0;
   uint32_t dcnt;
   uint32_t j;
   uint32_t dtmfkey;
   uint32_t oldkey;
   int32_t dtmffd;
   int32_t len;
   int32_t adcw;
   int32_t adc;
   DTMF dtmfs;
   char ch;
   short bw[512];
   short b[512];
   struct _0 * anonym;
   uint32_t tmp;
   int32_t tmp0;
   dtmffd = -1L;
   if (dtmffn[0U]) {
      dtmffd = osi_OpenWrite(dtmffn, 1024ul);
      if (dtmffd<0L) osi_Werr("dtmf write file/pipe open error\012", 33ul);
   }
   adcw = -1L;
   j = 0UL;
   for (;;) {
      if (soundfnout[0U]) adc = osi_OpenRead(soundfn, 1024ul);
      else adc = osi_OpenRW(soundfn, 1024ul);
      if (adc>=0L || j>=soundretry) break;
      usleep(50000UL);
      ++j;
   }
   if (adc>=0L) {
      i0 = samplesize(adc, 16UL); /* 8, 16 */
      i0 = channels(adc, 1UL); /* 1, 2  */
      i0 = sampelrate(adc, adcrate); /* 8000..48000 */
      if (soundfnout[0U] && soundfnout[0U]!='-') {
         adcw = osi_OpenWrite(soundfnout, 1024ul);
         if (adcw>=0L) {
            i0 = samplesize(adcw, 16UL); /* 8, 16 */
            i0 = channels(adcw, 1UL); /* 1, 2  */
            i0 = sampelrate(adcw, adcrate); /* 8000..48000 */
         }
         else osi_Werr("sound output open error\012", 25ul);
      }
      else adcw = adc;
      sqi = 0.0f;
      sqold = 0;
      wastx = 0;
      wavdone = 0;
      evertx = 0;
      ymi = 0.0f;
      dmi = 1000.0f;
      dmif = 0.0f;
      noisem = 0.0f;
      tonem = 0.0f;
      ucrn = 0.0f;
      uch = 0.0f;
      ilrn = 0.0f;
      limmul = 0.0f;
      blmed = 0.0f;
      blmedq = 0.0f;
      uca1 = 0.0f;
      uca2 = 0.0f;
      ila = 0.0f;
      oldkey = 0UL;
      dtmfkey = 0UL;
      uo = X2C_DIVR(2822.4f,(float)adcrate);
      rh = X2C_DIVR(uo,6.0f);
      rhlh = uo*2.8571428571429E-1f;
      if (highpass!=0.0f) {
         uo = (X2C_DIVR(44.1f,(float)adcrate))*highpass;
         rah = X2C_DIVR(uo,6.0f);
         rahlah = uo*2.8571428571429E-1f; /* 6/21/uo) */
      }
      lpR = X2C_DIVR(5.8026315789474E+2f,(float)adcrate);
      lpL = lpR*lpR*2.888f;
      olpR = 1.0f-lpR;
      lpAR = X2C_DIVR(X2C_DIVR(lowpass,(float)adcrate),
                4.3773242630385E-1f);
      lpAL = lpAR*lpAR*2.888f;
      olpAR = 1.0f-lpAR;
      if (clipplev>32000.0f) clipplev = 32000.0f;
      /*WrFixed(1.0/lpAR, 3, 10); WrFixed(1.0/lpAL, 3, 10); WrStrLn(""); */
      subtonf = X2C_DIVR(6.2831853071796f*subtontx,(float)adcrate);
      beepf = X2C_DIVR(6.2831853071796f*beepton,(float)adcrate);
      buildresonance(subtonrx, (float)adcrate, X2C_DIVR(subtonrx,18.0f),
                &subtonR, &subtonL);
      subtonAR = subtonR*2.0f; /* wider filter for notch */
      closedtime = 0.0;
      opentime = 0.0;
      rfilt = X2C_DIVR(50.0f,(float)adcrate); /* resonance level */
      lfilt = X2C_DIVR(32.0f,(float)adcrate);
                /* subtonband-resonance level */
      nfilt = X2C_DIVR(2.0f,(float)adcrate);
      tfilt = X2C_DIVR(1.0f,(float)adcrate);
      dfilt = 1.0f-X2C_DIVR(40.0f,(float)adcrate);
      dtmffilt = X2C_DIVR(64.0f,(float)adcrate);
      setptt(0);
      memset((char *)dtmfs,(char)0,sizeof(DTMF));
      for (j = 0UL; j<=7UL; j++) {
         buildresonance((float)_cnst[j], (float)adcrate,
                (float)_cnst[j]*0.1f, &dtmfs[j].R, &dtmfs[j].L);
      } /* end for */
      dtmfmaxl = 0.0f;
      dtmfmaxh = 0.0f;
      dcnt = 0UL;
      if (adcw>0L) {
         memset((char *)bw,(char)0,1024UL);
         tmp = filloss;
         j = 1UL;
         if (j<=tmp) for (;; j++) {
            write(adcw, (char *)bw, 1024UL);
            if (j==tmp) break;
         } /* end for */
      }
      timestep = X2C_DIVL(1.0,(double)adcrate);
      for (;;) {
         len = read(adc, (char *)b, 1024UL);
         if (len<=0L) break;
         tmp0 = len/2L-1L;
         i0 = 0L;
         if (i0<=tmp0) for (;; i0++) {
            /*audio path */
            ui = (float)b[i0]*volume;
            if (sqi>0.0f) {
               if (!sqold) {
                  if (closedtime>(double)beaconclosed) {
                     wavdone = 0; /* send beacon after sq close*/
                  }
                  sqold = 1;
                  setptt(1);
                  wastx = 1;
                  evertx = 1;
               }
               else opentime = opentime+timestep;
               if ((beaconopen!=0.0f && adcw>0L) && opentime>(double)
                beaconopen) {
                  /* send beacon on continuous open */
                  wavopen(wavfn, 1024ul);
                  opentime = 0.0;
               }
               if (exitsq!=0.0f && closedtime>(double)exitsq) {
                  goto loop_exit; /* terminate */
               }
               /* notch */
               if (notch) {
                  ucrn = ucrn+(ui-ucrn)*subtonAR+ilrn*subtonL;
                  ilrn = ilrn-ucrn;
                  uin = ui-ucrn;
               }
               else uin = ui;
               /* hipass 24db/oct */
               if (highpass!=0.0f) {
                  uo = uin-(ilah1+ilah2+ucah)*0.5f;
                  ucah = ucah+(ilah2+uo)*rah;
                  ilah2 = ilah2+uo*rahlah;
                  ilah1 = ilah1+(uo+ucah)*rahlah;
               }
               else uo = uin;
               if (wav.wavfd>=0L) uo = uo+wavsample()*wavlev;
               /* limiter */
               if (limlev!=0.0f) {
                  uo = uo*(1.0f-limmul);
                  ll = (float)fabs(uo)-limlev;
                  if (ll>0.0f) {
                     limmul = limmul+(1.0f-limmul)*ll*0.00001f;
                /* too loud */
                  }
                  else limmul = limmul*0.9999f;
               }
               /* clipper */
               if (uo>clipplev) uo = clipplev;
               else if (uo<-clipplev) uo = -clipplev;
               /* lowpass 24db/oct (6db loss) */
               if (lowpass!=0.0f) {
                  uca1 = (uca1+(uo-uca1)*lpAR)-ila;
                  uca2 = uca2*olpAR+ila;
                  ila = ila+(uca1-uca2)*lpAL;
                  uo = uca2*2.0f;
               }
               uo = uo*sqi;
            }
            else {
               /* squelch closed */
               if (sqold) {
                  closedtime = 0.0;
                  sqold = 0;
               }
               else closedtime = closedtime+timestep;
               uo = 0.0f;
               if (wastx && closedtime>(double)beepwait) {
                  /* beep */
                  vb = (float)((closedtime-(double)beepwait)*100.0);
                  if (vb>1.0f) {
                     vb = (float)(((double)(beepwait+beepend)
                -closedtime)*100.0);
                  }
                  if (vb>1.0f) vb = 1.0f;
                  uo = 0.0f+tone(beepf, &peepw)*beeplev*vb;
                  if (closedtime>(double)(beepwait+beepend)) {
                     wastx = 0; /* beep done */
                  }
               }
               if (beaconlonesome!=0.0f && closedtime>(double)
                beaconlonesome) {
                  /* send beacon if long closed */
                  wavdone = 0;
                  evertx = 1;
                  closedtime = 0.0;
               }
               if ((((wavstart!=0.0f && evertx) && !wavdone) && adcw>0L)
                && closedtime>(double)wavstart) {
                  wavopen(wavfn, 1024ul);
                  wavdone = 1;
                  opentime = 0.0;
               }
               if (wav.wavfd>=0L) {
                  setptt(1);
                  uo = uo+wavsample()*wavlev; /* play wav */
               }
               else if (closedtime>(double)ptthold) setptt(0);
               if (exitnosq!=0.0f && closedtime>(double)exitnosq) {
                  goto loop_exit; /* terminate */
               }
            }
            if (subtonf!=0.0f) uo = uo+tone(subtonf, &subw)*subtonlev;
            if (uo>32767.0f) uo = 32767.0f;
            else if (uo<(-3.2767E+4f)) uo = (-3.2767E+4f);
            bw[i0] = (short)X2C_TRUNCI(uo,-32768,32767);
            /*audio path */
            /*ctcss path */
            /* lowpass 254hz 24db/oct */
            uc1 = (uc1+(ui-uc1)*lpR)-il;
            uc2 = uc2*olpR+il;
            il = il+(uc1-uc2)*lpL;
            /* hipass 65hz 24db/oct */
            uo = uc2-(ilh1+ilh2+uch)*0.5f;
            uch = uch+(ilh2+uo)*rh;
            ilh2 = ilh2+uo*rhlh;
            ilh1 = ilh1+(uo+uch)*rhlh;
            /* resonance 88.5hz */
            ucr = ucr+(uo-ucr)*subtonR+ilr*subtonL;
            ilr = ilr-ucr;
            /* ctcss logic */
            iyi = uo-ucr; /* bandpass - resonance */
            ymi = ymi+((float)fabs(ucr)-ymi)*rfilt; /* level resonance */
            dmi = dmi+((float)fabs(iyi)-dmi)*lfilt;
                /* level bandpass - resonance */
            noisem = noisem+(dmif-noisem)*nfilt;
                /* median bandpass - resonance */
            if ((float)fabs(iyi)>dmif) {
               dmif = (float)fabs(iyi); /* peak bandpass - resonance */
            }
            else dmif = dmif*dfilt;
            if (ymi>dmi*thsqopen) {
               tonem = ymi;
               sqi = sqi+0.001f;
               if (sqi>1.0f) sqi = 1.0f;
            }
            else {
               tonem = tonem+(dmi-tonem)*tfilt;
               if (ymi*thsqclose<tonem && sqi>0.0f) sqi = sqi-0.01f;
            }
            /* dtmf */
            if (dtmffn[0U]) {
               for (j = 0UL; j<=7UL; j++) {
                  { /* with */
                     struct _0 * anonym = &dtmfs[j];
                     anonym->uc = anonym->uc+(ui-anonym->uc)
                *anonym->R+anonym->il*anonym->L;
                     anonym->il = anonym->il-anonym->uc;
                     anonym->lev = anonym->lev+((float)fabs(anonym->uc)
                -anonym->lev)*dtmffilt;
                  }
               } /* end for */
               dcnt += 200UL;
               if (dcnt>adcrate) {
                  /* process dtmf levels */
                  dcnt = 0UL;
                  dtmfeval(dtmfs, &dtmfmaxl, &dtmfmaxh, &dtmfkey);
                  if (dtmfkey>0UL && oldkey==0UL) {
                     ch = " *0#D789C456B123A"[dtmfkey];
                     if (verb) {
                        osi_WrStrLn("", 1ul);
                        osi_WrStr("dtmf:", 6ul);
                        osi_WrStrLn((char *) &ch, 1u/1u);
                     }
                     if (dtmffd>=0L) write(dtmffd, (char *) &ch, 1UL);
                  }
                  oldkey = dtmfkey;
               }
            }
            if (verb) {
               if (cnt>=(int32_t)(adcrate/20UL)) {
                  cnt = 0L;
                  osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(ymi,
                X2C_min_longint,X2C_max_longint), 7UL);
                  osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(dmi,
                X2C_min_longint,X2C_max_longint), 7UL);
                  osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(tonem,
                X2C_min_longint,X2C_max_longint), 7UL);
                  osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(noisem,
                X2C_min_longint,X2C_max_longint), 7UL);
                  osic_WrINT32((uint32_t)(int32_t)
                X2C_TRUNCI(limmul*1000.0f,X2C_min_longint,X2C_max_longint),
                7UL);
                  for (j = 0UL; j<=7UL; j++) {
                     osic_WrINT32((uint32_t)X2C_TRUNCC(dtmfs[j].lev,0UL,
                X2C_max_longcard)/100UL, 4UL);
                  } /* end for */
                  /*
                            IO.WrInt(j1, 2); IO.WrInt(j2, 2);
                            IF (j1=3) & (j2=3) THEN IO.WrCard(dtmfx+dtmfy*4,
                2) END;
                  
                  --        IF sq THEN IO.WrStr(" *") ELSE IO.WrStr("  ")
                END;  
                  
                  */
                  if (sqi>0.0f) osi_WrStr(" *", 3ul);
                  else osi_WrStr("  ", 3ul);
                  if (dmi>noisem*2.0f) osi_WrStr("F", 2ul);
                  else osi_WrStr(" ", 2ul);
                  osic_WrLn();
               }
               ++cnt;
            }
            if (i0==tmp0) break;
         } /* end for */
         if (adcw>0L) write(adcw, (char *)bw, (uint32_t)len);
      }
      loop_exit:;
      osic_Close(adc);
      if (adcw>=0L) osic_Close(adcw);
   }
   else osi_Werr("sound input open error\012", 24ul);
} /* end audio() */

static void pttcleanup(int32_t);


static void pttcleanup(int32_t signum)
{
   pttDestroy(hptt);
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   X2C_HALT((uint32_t)signum);
} /* end pttcleanup() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGTERM, pttcleanup);
   signal(SIGINT, pttcleanup);
   signal(SIGPIPE, pttcleanup);
   pttstate = 0;
   hptt = 0;
   adcrate = 22050UL;
   subtonrx = 88.5f;
   subtontx = 0.0f;
   subtonlev = 3276.7f;
   clipplev = 0.0f;
   beepton = 0.0f;
   beepwait = 0.0f;
   beepend = 0.0f;
   beeplev = 3276.7f;
   ptthold = 1.0f;
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   soundfnout[0] = 0;
   exitsq = 0.0f;
   exitnosq = 0.0f;
   notch = 0;
   highpass = 0.0f;
   lowpass = 0.0f;
   clipplev = 32000.0f;
   limlev = 0.0f;
   volume = 1.0f;
   thsqclose = 1.5f;
   thsqopen = 1.3f;
   dtmffn[0] = 0;
   filloss = 0UL;
   soundretry = 0UL;
   verb = 0;
   wavlev = 0.5f;
   wavstart = 0.0f;
   wavfn[0] = 0;
   wav.wavfd = -1L;
   beaconopen = 0.0f;
   beaconclosed = 0.0f;
   beaconlonesome = 0.0f;
   Parms();
   audio();
   pttDestroy(hptt);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
