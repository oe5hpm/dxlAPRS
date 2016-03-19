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
#define sdrtest_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef sdr_H_
#include "sdr.h"
#endif

/* test rtl_tcp iq fm demodulator by OE5DXL */
#define sdrtest_MAXCHANNELS 32

#define sdrtest_DEFAULTPOWERSAVE 0

#define sdrtest_SAMPSIZE 32
/* samples per sdr call */

struct FREQTAB;


struct FREQTAB {
   unsigned long khz;
   unsigned long hz;
   unsigned long width;
   unsigned long agc;
   long afc;
   char modulation;
};

struct STICKPARM;


struct STICKPARM {
   unsigned long val;
   char ok0;
   char changed;
};

struct SQUELCH;


struct SQUELCH {
   float lev;
   float lp;
   float u1;
   float u2;
   float il;
   float medmed;
   float mutlev;
   long sqsave;
   long wakeness;
   long pcmc;
   unsigned long nexttick;
};

static long fd;

static unsigned long sndw;

static unsigned long freqc;

static unsigned long midfreq;

static unsigned long lastmidfreq;

static unsigned long downsamp;

static unsigned long mixto;

static unsigned long powersave;

static short sndbuf[1024];

static unsigned char sndbuf8[1024];

static struct sdr_RX rxx[32];

static sdr_pRX prx[33];

static short sampx[32][32];

static struct STICKPARM stickparm[256];

static struct SQUELCH squelchs[33];

static unsigned long iqrate;

static unsigned long samphz;

static unsigned long maxrx;

static char url[1001];

static char port[1001];

static char soundfn[1001];

static char parmfn[1001];

static char verb;

static char reconn;

static char pcm8;

static char nosquelch;

static double offset;


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void card(const char s[], unsigned long s_len, unsigned long * p,
                unsigned long * n, char * ok0)
{
   *ok0 = 0;
   *n = 0UL;
   while ((unsigned char)s[*p]>='0' && (unsigned char)s[*p]<='9') {
      *n = ( *n*10UL+(unsigned long)(unsigned char)s[*p])-48UL;
      *ok0 = 1;
      ++*p;
   }
} /* end card() */


static void int0(const char s[], unsigned long s_len, unsigned long * p,
                long * n, char * ok0)
{
   char sgn;
   unsigned long c;
   if (s[*p]=='-') {
      sgn = 1;
      ++*p;
   }
   else sgn = 0;
   card(s, s_len, p, &c, ok0);
   if (sgn) *n = -(long)c;
   else *n = (long)c;
} /* end int() */


static void fix(const char s[], unsigned long s_len, unsigned long * p,
                double * x, char * ok0)
{
   float m;
   char sgn;
   if (s[*p]=='-') {
      sgn = 1;
      ++*p;
   }
   else sgn = 0;
   m = 1.0f;
   *ok0 = 0;
   *x = 0.0;
   while ((unsigned char)s[*p]>='0' && (unsigned char)
                s[*p]<='9' || s[*p]=='.') {
      if (s[*p]=='.') m = 0.1f;
      else if (m==1.0f) {
         *x =  *x*10.0+(double)(float)((unsigned long)(unsigned char)
                s[*p]-48UL);
      }
      else {
         *x = *x+(double)((float)((unsigned long)(unsigned char)s[*p]-48UL)
                *m);
         m = m*0.1f;
      }
      *ok0 = 1;
      ++*p;
   }
   if (sgn) *x = -*x;
} /* end fix() */


static void Parms(void)
{
   char s[1001];
   unsigned long n;
   unsigned long m;
   long ni;
   char ok0;
   reconn = 0;
   verb = 0;
   pcm8 = 0;
   downsamp = 0UL;
   mixto = 0UL;
   strncpy(url,"127.0.0.1",1001u);
   strncpy(port,"1234",1001u);
   soundfn[0] = 0;
   iqrate = 2048000UL;
   samphz = 16000UL;
   offset = 0.0;
   strncpy(parmfn,"sdrcfg.txt",1001u);
   powersave = 0UL;
   maxrx = 31UL;
   nosquelch = 0;
   for (;;) {
      osi_NextArg(s, 1001ul);
      if (s[0U]==0) break;
      if ((s[0U]=='-' && s[1U]) && s[2U]==0) {
         if (s[1U]=='s') {
            osi_NextArg(soundfn, 1001ul);
            pcm8 = 0;
         }
         else if (s[1U]=='S') {
            osi_NextArg(soundfn, 1001ul);
            pcm8 = 1;
         }
         else if (s[1U]=='c') osi_NextArg(parmfn, 1001ul);
         else if (s[1U]=='o') {
            /* offset */
            osi_NextArg(s, 1001ul);
            n = 0UL;
            fix(s, 1001ul, &n, &offset, &ok0);
            if (!ok0) Error(" -o <MHz>", 10ul);
         }
         else if (s[1U]=='d') {
            /* sampelrate to output divide */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &downsamp) || downsamp<1UL) {
               Error(" -p <ratio>", 12ul);
            }
            --downsamp;
         }
         else if (s[1U]=='m') {
            /* downmix to */
            osi_NextArg(s, 1001ul);
            if ((!aprsstr_StrToCard(s, 1001ul,
                &mixto) || mixto<1UL) || mixto>2UL) {
               Error(" -m <1..2>", 11ul);
            }
         }
         else if (s[1U]=='t') {
            osi_NextArg(s, 1001ul); /* url */
            n = 0UL;
            while ((n<1000UL && s[n]) && s[n]!=':') {
               if (n<1000UL) url[n] = s[n];
               ++n;
            }
            if (n>1000UL) n = 1000UL;
            url[n] = 0;
            if (s[n]==':') {
               m = 0UL;
               ++n;
               while ((n<1000UL && s[n]) && m<1000UL) {
                  port[m] = s[n];
                  ++n;
                  ++m;
               }
               if (m>1000UL) m = 1000UL;
               port[m] = 0;
            }
         }
         else if (s[1U]=='r') {
            /* sampelrate */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &samphz) || samphz>192000UL) {
               Error(" -r <Hz>", 9ul);
            }
         }
         else if (s[1U]=='i') {
            /* iq sampelrate */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul,
                &iqrate) || iqrate!=2048000UL && iqrate!=1024000UL) {
               Error(" -i <Hz> 2048000 or 1024000", 28ul);
            }
         }
         else if (s[1U]=='v') verb = 1;
         else if (s[1U]=='k') reconn = 1;
         else if (s[1U]=='z') {
            /* powersave time */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &powersave)) {
               Error(" -z <ms>", 9ul);
            }
            nosquelch = 0;
         }
         else if (s[1U]=='Z') {
            /* powersave time */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &powersave)) {
               Error(" -Z <ms>", 9ul);
            }
            nosquelch = 1;
         }
         else {
            if (s[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("FM Multirx from rtl_tcp (8 bit IQ via tcpip to 2 \
channel 16 bit pcm file/pipe", 78ul);
               osi_WrStrLn(" -c <configfilename> read channels config from fi\
le (sdrcfg.txt)", 65ul);
               osi_WrStrLn(" -d <ratio>          downsample output (1)",
                43ul);
               osi_WrStrLn(" -h                  help", 26ul);
               osi_WrStrLn(" -i <Hz>             input sampelrate Hz (2048000\
) or 1024000", 62ul);
               osi_WrStrLn(" -k                  keep connection", 37ul);
               osi_WrStrLn(" -m <audiochannels>  mix all rx channels to audio\
channels (1..2)", 65ul);
               osi_WrStrLn(" -o <mhz>            offset for entered frequenci\
es if Converters are used", 75ul);
               osi_WrStrLn(" -p <cmd> <value>    send rtl_tcp parameter, ppm,\
 tunergain ...", 64ul);
               osi_WrStrLn(" -r <Hz>             output sampelrate Hz for all\
 channels (16000)", 67ul);
               osi_WrStrLn(" -s <soundfilename>  16bit signed n-channel sound\
 stream/pipe", 62ul);
               osi_WrStrLn(" -S <soundfilename>  8bit unsigned n-channel soun\
d stream/pipe", 63ul);
               osi_WrStrLn(" -t <url:port>       connect rtl_tcp server (127.\
0.0.1:1234)", 61ul);
               osi_WrStrLn(" -v                  show rssi (dB) and afc (khz)\
", 50ul);
               osi_WrStrLn(" -z <ms>             sleep time inactive channel \
if squelch closed", 67ul);
               osi_WrStrLn(" -Z <ms>             same but no audio quieting f\
or sending to a decoder", 73ul);
               osi_WrStrLn("example: -s /tmp/sound.pcm -t 192.168.1.1:1234 -p\
 5 72 -p 8 1 -v", 65ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("config file: (re-read every some seconds and may \
be modified any time)", 71ul);
               osi_WrStrLn("  # comment", 12ul);
               osi_WrStrLn("  p <cmd> <value>  rtl_tcp parameter like \'p 5 5\
0\' ppm, \'p 8 1\' autogain", 73ul);
               osi_WrStrLn("  f <mhz> <AFC-range> <squelch%> <lowpass%>  <if-\
width>  FM Demodulator", 72ul);
               osi_WrStrLn("  a <mhz>  0           0         <lowpass%>  <if-\
width>  AM Demodulator", 72ul);
               osi_WrStrLn("  u <mhz> <if-shift>   0         <agc speed> <if-\
width>  USB Demodulator", 73ul);
               osi_WrStrLn("  l same for LSB", 17ul);
               osi_WrStrLn("    AFC-range in +-kHz, Squelch 0 off, 100 open, \
95 near open", 62ul);
               osi_WrStrLn("    audio lowpass in % Nyquist frequ. of output s\
ampelrate, 0 is off", 69ul);
               osi_WrStrLn("    IF-width 3000 6000 12000 24000 48000 96000 19\
2000Hz for low CPU usage", 74ul);
               osi_WrStrLn("    (192000 only with 2048khz iq-rate), (4th orde\
r IIR) SSB always low CPU", 75ul);
               osi_WrStrLn("    (+ 8th order IF-IIR), other values with more \
CPU-load (6000 default)", 73ul);
               osi_WrStrLn("  a <mhz> 0 <squelch> <lowpass%> <if-width>   AM \
Demodulator", 61ul);
               osi_WrStrLn("  u <mhz> 0 0 <lowpass%> <if-width>           USB\
 Demodulator", 62ul);
               osi_WrStrLn("  l <mhz> 0 0 <lowpass%> <if-width>           LSB\
 Demodulator", 62ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("  example:", 11ul);
               osi_WrStrLn("    p 5 50", 11ul);
               osi_WrStrLn("    p 8 1", 10ul);
               osi_WrStrLn("    f 438.825 5 95 70           (afc, quelch, aud\
io lowpass, 12khz IF)", 71ul);
               osi_WrStrLn("    f 439.275 0 0 0 20000       (20khz IF)",
                43ul);
               osi_WrStrLn("  will generate 2 channel 16bit PCM stream (up to\
 32 channels with future cpu...)", 82ul);
               osi_WrStrLn("  max 2MHz span, automatic rtl-tuned to center of\
 the span", 59ul);
               osi_WrStrLn("  excluded +-10khz of rtl-band-center to avoid AD\
C-DC offset pseudo carriers", 77ul);
               X2C_ABORT();
            }
            if (s[1U]=='p') {
               osi_NextArg(s, 1001ul);
               if (aprsstr_StrToCard(s, 1001ul, &m) && m<256UL) {
                  osi_NextArg(s, 1001ul);
                  if (aprsstr_StrToInt(s, 1001ul, &ni)) {
                     stickparm[m].val = (unsigned long)ni;
                /* stick parameter */
                     stickparm[m].ok0 = 1;
                     stickparm[m].changed = 1;
                  }
                  else Error(" -p <cmd> <value>", 18ul);
               }
               else Error(" -p <cmd> <value>", 18ul);
            }
            else Error("-h", 3ul);
         }
      }
      else Error("-h", 3ul);
   }
   powersave = (powersave*samphz)/32000UL;
} /* end Parms() */


static void setparms(char all)
{
   unsigned long i;
   char nl;
   char s[31];
   nl = 1;
   for (i = 0UL; i<=255UL; i++) {
      if (stickparm[i].ok0 && (all || stickparm[i].changed)) {
         sdr_setparm(i, stickparm[i].val);
         if (verb) {
            if (nl) {
               osi_WerrLn("", 1ul);
               nl = 0;
            }
            osi_Werr("parm:", 6ul);
            aprsstr_IntToStr((long)i, 0UL, s, 31ul);
            osi_Werr(s, 31ul);
            osi_Werr(" ", 2ul);
            aprsstr_IntToStr((long)stickparm[i].val, 0UL, s, 31ul);
            osi_WerrLn(s, 31ul);
         }
      }
      stickparm[i].changed = 0;
   } /* end for */
} /* end setparms() */


static void setstickparm(unsigned long cmd, unsigned long value)
{
   if (!stickparm[cmd].ok0 || stickparm[cmd].val!=value) {
      stickparm[cmd].val = value;
      stickparm[cmd].changed = 1;
   }
   stickparm[cmd].ok0 = 1;
} /* end setstickparm() */

#define sdrtest_OFFSET 10


static void centerfreq(const struct FREQTAB freq[], unsigned long freq_len)
{
   unsigned long max0;
   unsigned long min0;
   unsigned long i;
   long nomid;
   midfreq = 0UL;
   i = 0UL;
   max0 = 0UL;
   min0 = X2C_max_longcard;
   while (i<freqc) {
      /*WrStr("rx"); WrInt(i+1, 0); WrInt(freq[i].khz, 0); WrStrLn("kHz"); */
      if (freq[i].khz>max0) max0 = freq[i].khz;
      if (freq[i].khz<min0) min0 = freq[i].khz;
      ++i;
   }
   if (max0>=min0) {
      if (max0-min0>2000UL) osi_WerrLn("freq span > 2MHz", 17ul);
      midfreq = (max0+min0)/2UL;
      nomid = X2C_max_longint;
      i = 0UL;
      while (i<freqc) {
         if (labs((long)(freq[i].khz-midfreq))<labs(nomid)) {
            nomid = (long)(freq[i].khz-midfreq);
         }
         ++i;
      }
      if (labs(nomid)>10L) nomid = 0L;
      else if (nomid<0L) nomid = 10L+nomid;
      else nomid -= 10L;
      midfreq += (unsigned long)nomid;
      i = 0UL;
      while (i<freqc) {
         /*     FILL(ADR(rxx[i]), 0C, SIZE(rxx[0])); */
         prx[i] = &rxx[i];
         rxx[i].df = freq[i].khz-midfreq;
         rxx[i].dffrac = freq[i].hz;
         rxx[i].maxafc = freq[i].afc;
         /*WrInt(rxx[i].maxafc, 0); WrStrLn(" maxafc"); */
         rxx[i].squelch = squelchs[i].lev!=0.0f;
         rxx[i].width = freq[i].width;
         rxx[i].agc = freq[i].agc;
         rxx[i].modulation = freq[i].modulation;
         ++i;
      }
      prx[i] = 0;
   }
   if (midfreq<40000UL) osi_WerrLn("no valid frequency", 19ul);
   else if (midfreq!=lastmidfreq) {
      setstickparm(1UL, midfreq*1000UL);
      /*WrStr("set ");WrInt(midfreq, 0); WrStrLn("kHz"); */
      lastmidfreq = midfreq;
   }
} /* end centerfreq() */


static void skip(const char s[], unsigned long s_len, unsigned long * p)
{
   while (s[*p] && s[*p]==' ') ++*p;
} /* end skip() */


static void rdconfig(void)
{
   long len;
   long fd0;
   unsigned long sq;
   unsigned long wid;
   unsigned long lpp;
   unsigned long p;
   unsigned long lino;
   unsigned long n;
   unsigned long i;
   long m;
   double x;
   char ok0;
   char b[10001];
   char li[256];
   struct FREQTAB freq[32];
   char mo;
   fd0 = osi_OpenRead(parmfn, 1001ul);
   if (fd0>=0L) {
      len = osi_RdBin(fd0, (char *)b, 10001u/1u, 10001UL);
      if ((len>0L && len<10000L) && (unsigned char)b[len-1L]>=' ') {
         b[len-1L] = 0;
      }
      p = 0UL;
      lino = 1UL;
      freqc = 0UL;
      i = 0UL;
      while ((long)p<len) {
         if ((unsigned char)b[p]>=' ') {
            if (i<255UL) {
               li[i] = b[p];
               ++i;
            }
         }
         else if (i>0UL) {
            li[i] = 0;
            i = 0UL;
            skip(li, 256ul, &i);
            mo = X2C_CAP(li[i]);
            if (mo=='P') {
               ++i;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &n, &ok0);
               if (n>255UL || !ok0) {
                  osi_WerrLn("wrong parameter number", 23ul);
               }
               else {
                  skip(li, 256ul, &i);
                  int0(li, 256ul, &i, &m, &ok0);
                  if (!ok0) osi_WerrLn("wrong value", 12ul);
                  else setstickparm(n, (unsigned long)m);
               }
               i = 0UL;
            }
            else if (((mo=='F' || mo=='A') || mo=='U') || mo=='L') {
               if (mo=='A') freq[freqc].modulation = 'a';
               else if (mo=='F') freq[freqc].modulation = 'f';
               else if (mo=='U') freq[freqc].modulation = 's';
               else if (mo=='L') freq[freqc].modulation = 's';
               else freq[freqc].modulation = 'f';
               ++i;
               skip(li, 256ul, &i);
               fix(li, 256ul, &i, &x, &ok0);
               if (!ok0) osi_WerrLn("wrong MHz", 10ul);
               skip(li, 256ul, &i);
               int0(li, 256ul, &i, &m, &ok0);
               if (!ok0) m = 0L;
               if (mo=='U') m += 1500L;
               else if (mo=='L') m -= 1500L;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &sq, &ok0);
               if (!ok0) sq = 0UL;
               if (sq>200UL) sq = 200UL;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &lpp, &ok0);
               if (!ok0) lpp = 0UL;
               if (lpp>100UL) lpp = 100UL;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &wid, &ok0);
               if (!ok0) {
                  if (freq[freqc].modulation=='s') wid = 2800UL;
                  else if (freq[freqc].modulation=='a') wid = 6000UL;
                  else wid = 12000UL;
               }
               if (wid>1000000UL) wid = 1000000UL;
               if (freqc>31UL) osi_WerrLn("freq table full", 16ul);
               else {
                  x = x+offset;
                  if (x<=0.0 || x>=2.147483E+6) {
                     osi_WerrLn("freq out of range", 18ul);
                     x = 0.0;
                  }
                  x = x*1.E+6+(double)m;
                  freq[freqc].khz = (unsigned long)X2C_TRUNCC(x,0UL,
                X2C_max_longcard);
                  freq[freqc].hz = freq[freqc].khz%1000UL;
                  freq[freqc].khz = freq[freqc].khz/1000UL;
                  freq[freqc].afc = m;
                  freq[freqc].width = wid;
                  freq[freqc].agc = lpp;
                  squelchs[freqc].lev = X2C_DIVR((float)sq*32.0f,200.0f);
                  if (freq[freqc].modulation=='s') squelchs[freqc].lp = 0.0f;
                  else squelchs[freqc].lp = X2C_DIVR((float)lpp,200.0f);
                  ++freqc;
               }
               i = 0UL;
            }
            else if (mo=='#' || mo==' ') i = 0UL;
            else osi_WerrLn("unkown command", 15ul);
            ++lino;
         }
         ++p;
      }
      osic_Close(fd0);
   }
   else Error("config file not readable", 25ul);
   centerfreq(freq, 32ul);
} /* end rdconfig() */


static void showrssi(void)
{
   unsigned long j;
   unsigned long i;
   char s[31];
   i = 0UL;
   while (prx[i]) {
      j = prx[i]->idx;
      aprsstr_FixToStr(osic_ln((rxx[j].rssi+1.0f)*3.0517578125E-5f)
                *4.342944819f, 2UL, s, 31ul);
      osi_Werr(s, 31ul);
      if (squelchs[j].sqsave<=0L) osi_Werr("db", 3ul);
      else osi_Werr("dB", 3ul);
      if (!nosquelch && rxx[j].squelch) {
         osi_Werr(" ", 2ul);
         aprsstr_FixToStr(squelchs[j].medmed*0.03125f, 3UL, s, 31ul);
         osi_Werr(s, 31ul);
      }
      if (rxx[j].modulation=='f') {
         osi_Werr(" ", 2ul);
         aprsstr_IntToStr(rxx[j].afckhz, 0UL, s, 31ul);
         osi_Werr(s, 31ul);
         osi_Werr(" ", 2ul);
      }
      /*Werr(" "); IntToStr(squelchs[j].wakeness, 0, s); Werr(s); Werr(" ");
                */
      ++i;
   }
   osi_Werr("    \015", 6ul);
   osic_flush();
} /* end showrssi() */

static long sp;

static long sn;

static unsigned long rp;

static unsigned long actch;

static unsigned long ticker;

static unsigned long ix;

static unsigned long tshow;

static unsigned long dsamp;

static char recon;

static long pcm;

static long mixleft;

static long mixright;

static long levdiv2;


static void sendaudio(long pcm0, char pcm80, char chan0)
{
   if (pcm0>32767L) pcm0 = 32767L;
   else if (pcm0<-32767L) pcm0 = -32767L;
   if (pcm80) {
      sndbuf8[sndw] = (unsigned char)((unsigned long)(pcm0+32768L)/256UL);
   }
   else {
      pcm0 = (long)((unsigned long)pcm0&0xFFFFFFFEUL);
      if (chan0) ++pcm0;
      sndbuf[sndw] = (short)pcm0;
   }
   ++sndw;
   if (sndw>1023UL) {
      if (pcm80) osi_WrBin(fd, (char *)sndbuf8, 1024u/1u, sndw);
      else osi_WrBin(fd, (char *)sndbuf, 2048u/1u, sndw*2UL);
      sndw = 0UL;
   }
} /* end sendaudio() */


static void userio(void)
{
   rdconfig();
   setparms(recon);
   recon = 0;
   if (verb) showrssi();
} /* end userio() */


static void schedule(void)
{
   unsigned long j;
   unsigned long i;
   i = 0UL;
   if (freqc>0UL) {
      j = actch;
      do {
         if ((long)(ticker-squelchs[j].nexttick)>=0L) {
            squelchs[j].nexttick = ticker;
            prx[i] = &rxx[j];
            ++i;
         }
         ++j;
         if (j>=freqc) j = 0UL;
      } while (!(i>maxrx || j==actch));
   }
   prx[i] = 0;
   actch = j;
} /* end schedule() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   long tmp;
   unsigned long tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   sdr_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   midfreq = 0UL;
   lastmidfreq = 0UL;
   tshow = 0UL;
   levdiv2 = 256L;
   memset((char *)rxx,(char)0,sizeof(struct sdr_RX [32]));
   memset((char *)stickparm,(char)0,sizeof(struct STICKPARM [256]));
   memset((char *)squelchs,(char)0,sizeof(struct SQUELCH [33]));
   Parms();
   actch = 0UL;
   ticker = 0UL;
   prx[0U] = 0;
   for (freqc = 0UL; freqc<=31UL; freqc++) {
      rxx[freqc].samples = (sdr_pAUDIOSAMPLE)sampx[freqc];
      rxx[freqc].idx = freqc;
   } /* end for */
   if (sdr_startsdr(url, 1001ul, port, 1001ul, iqrate, samphz, reconn)) {
      rdconfig();
      sndw = 0UL;
      fd = osi_OpenWrite(soundfn, 1001ul);
      if (fd>=0L) {
         recon = 1;
         for (;;) {
            if (tshow>500UL) {
               userio();
               tshow = 0UL;
            }
            else ++tshow;
            schedule();
            sn = sdr_getsdr(32UL, prx, 33ul);
            if (sn<0L) {
               if (verb) {
                  if (sn==-2L) {
                     osi_WerrLn("impossible sampelrate conversion", 33ul);
                  }
                  else osi_WerrLn("connection lost", 16ul);
               }
               recon = 1;
               if (!reconn) break;
            }
            else {
               rp = 0UL;
               while (prx[rp]) {
                  ix = prx[rp]->idx;
                  { /* with */
                     struct SQUELCH * anonym = &squelchs[ix];
                     if (anonym->lev==0.0f || prx[rp]->modulation=='s') {
                        squelchs[ix].mutlev = 1.0f;
                     }
                     else {
                        if (anonym->lev<rxx[ix].sqsum) {
                           /* noise */
                           if (anonym->sqsave>0L) {
                              --anonym->sqsave;
                           }
                           else {
                              if (anonym->wakeness<(long)powersave) {
                                 ++anonym->wakeness;
                              }
                              anonym->nexttick = ticker+(unsigned long)
                anonym->wakeness;
                           }
                        }
                        else {
                           if (anonym->sqsave<2000L) {
                              ++anonym->sqsave;
                           }
                           if (anonym->wakeness>-100L) {
                              --anonym->wakeness;
                           }
                        }
                        if ((nosquelch || rxx[ix].modulation=='u')
                || rxx[ix].modulation=='l') anonym->mutlev = 1.0f;
                        else {
                           anonym->medmed = anonym->medmed+(rxx[ix]
                .sqsum-anonym->medmed)*0.1f;
                           anonym->mutlev = (anonym->lev-anonym->medmed)
                *0.3125f;
                           if (anonym->mutlev<0.0f) {
                              anonym->mutlev = 0.0f;
                           }
                           else if (anonym->mutlev>1.0f) {
                              anonym->mutlev = 1.0f;
                           }
                        }
                     }
                  }
                  rxx[ix].sqsum = 0.0f;
                  ++rp;
               }
               tmp = sn-1L;
               sp = 0L;
               if (sp<=tmp) for (;; sp++) {
                  rp = 0UL;
                  while (prx[rp]) {
                     ix = prx[rp]->idx;
                     { /* with */
                        struct SQUELCH * anonym0 = &squelchs[ix];
                        anonym0->pcmc = (long)rxx[ix].samples[sp];
                        if (!nosquelch) {
                           anonym0->pcmc = (long)(short)X2C_TRUNCI((float)
                anonym0->pcmc*anonym0->mutlev,-32768,32767);
                        }
                        /* lowpass */
                        if (anonym0->lp!=0.0f) {
                           anonym0->u1 = anonym0->u1+(((float)
                (anonym0->pcmc*2L)-anonym0->u1)-anonym0->il)*anonym0->lp;
                           anonym0->u2 = anonym0->u2+(anonym0->il-anonym0->u2)
                *anonym0->lp;
                           anonym0->il = anonym0->il+(anonym0->u1-anonym0->u2)
                *anonym0->lp*2.0f;
                           anonym0->pcmc = (long)X2C_TRUNCI(anonym0->u2,
                X2C_min_longint,X2C_max_longint);
                        }
                     }
                     /* lowpass */
                     ++rp;
                  }
                  if (dsamp==0UL) {
                     tmp0 = freqc-1UL;
                     rp = 0UL;
                     if (rp<=tmp0) for (;; rp++) {
                        pcm = squelchs[rp].pcmc;
                        squelchs[rp].pcmc = 0L;
                        /* channel mixer */
                        if (mixto==1UL) mixleft += pcm;
                        else if (mixto==2UL) {
                           if (freqc<=1UL) {
                              mixleft = pcm;
                              mixright = pcm;
                           }
                           else {
                              mixleft += pcm*(long)rp;
                              mixright += pcm*(long)((freqc-rp)-1UL);
                           }
                        }
                        /* channel mixer */
                        if (mixto==0UL || rp==0UL) {
                           if (mixto==0UL || freqc==0UL) {
                              sendaudio(pcm, pcm8, rp==0UL);
                           }
                           else {
                              sendaudio(mixleft*levdiv2>>8, pcm8, rp==0UL);
                              if (mixto==2UL) {
                                 sendaudio(mixright*levdiv2>>8, pcm8,
                rp==0UL);
                              }
                           }
                           if (labs(mixleft)>32767L || labs(mixright)>32767L)
                 {
                              if (levdiv2>20L) {
                                 --levdiv2;
                              }
                           }
                           else if (levdiv2<256L) ++levdiv2;
                           mixleft = 0L;
                           mixright = 0L;
                        }
                        if (rp==tmp0) break;
                     } /* end for */
                  }
                  if (dsamp==0UL) dsamp = downsamp;
                  else --dsamp;
                  if (sp==tmp) break;
               } /* end for */
            }
            ++ticker;
         }
      }
      else {
         osi_Werr(soundfn, 1001ul);
         osi_WerrLn(" sound file open error", 23ul);
      }
      if (verb) osi_WerrLn("connection lost", 16ul);
   }
   else osi_WerrLn("not connected", 14ul);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
