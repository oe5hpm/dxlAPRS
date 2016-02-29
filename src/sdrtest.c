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
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef sdr_H_
#include "sdr.h"
#endif
#include "osic.h"

/* test rtl_tcp iq fm demodulator by OE5DXL */
#define sdrtest_MAXCHANNELS 32

struct FREQTAB;


struct FREQTAB {
   unsigned long khz;
   unsigned long afc;
   unsigned long width;
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
};

static long fd;

static unsigned long sndw;

static unsigned long freqc;

static unsigned long midfreq;

static unsigned long lastmidfreq;

static short sndbuf[1024];

static struct sdr_RX rxx[32];

static sdr_pRX prx[33];

static short sampx[32][16];

static struct STICKPARM stickparm[256];

static struct SQUELCH squelchs[33];

static unsigned long iqrate;

static unsigned long samphz;

static char url[1001];

static char port[1001];

static char soundfn[1001];

static char parmfn[1001];

static char verb;

static char reconn;


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osic_WrStr(text, text_len);
   osic_WrStrLn(" error abort", 13ul);
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
                unsigned long * n, char * ok0)
{
   char sgn;
   if (s[*p]=='-') {
      sgn = 1;
      ++*p;
   }
   else sgn = 0;
   card(s, s_len, p, n, ok0);
   if (sgn) *n = (unsigned long) -(long)*n;
} /* end int() */


static void fix(const char s[], unsigned long s_len, unsigned long * p,
                float * x, char * ok0)
{
   float m;
   m = 1.0f;
   *ok0 = 0;
   *x = 0.0f;
   while ((unsigned char)s[*p]>='0' && (unsigned char)
                s[*p]<='9' || s[*p]=='.') {
      if (s[*p]=='.') m = 0.1f;
      else if (m==1.0f) {
         *x =  *x*10.0f+(float)((unsigned long)(unsigned char)s[*p]-48UL);
      }
      else {
         *x = *x+(float)((unsigned long)(unsigned char)s[*p]-48UL)*m;
         m = m*0.1f;
      }
      *ok0 = 1;
      ++*p;
   }
} /* end fix() */


static void Parms(void)
{
   char s[1001];
   unsigned long n;
   unsigned long m;
   long ni;
   reconn = 0;
   verb = 0;
   strncpy(url,"127.0.0.1",1001u);
   strncpy(port,"1234",1001u);
   soundfn[0] = 0;
   iqrate = 2048000UL;
   samphz = 16000UL;
   strncpy(parmfn,"sdrcfg.txt",1001u);
   for (;;) {
      osi_NextArg(s, 1001ul);
      if (s[0U]==0) break;
      if ((s[0U]=='-' && s[1U]) && s[2U]==0) {
         if (s[1U]=='s') osi_NextArg(soundfn, 1001ul);
         else if (s[1U]=='c') osi_NextArg(parmfn, 1001ul);
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
         else {
            if (s[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("FM Multirx from rtl_tcp (8 bit IQ via tcpip to 2 \
channel 16 bit pcm file/pipe", 78ul);
               osi_WrStrLn(" -c <configfilename> read channels config from fi\
le (sdrcfg.txt)", 65ul);
               osi_WrStrLn(" -h                  help", 26ul);
               osi_WrStrLn(" -i <Hz>             input sampelrate Hz (2048000\
) or 1024000", 62ul);
               osi_WrStrLn(" -k                  keep connection", 37ul);
               osi_WrStrLn(" -p <cmd> <value>    send rtl_tcp parameter, ppm,\
 tunergain ...", 64ul);
               osi_WrStrLn(" -r <Hz>             output sampelrate Hz for all\
 channels (16000)", 67ul);
               osi_WrStrLn(" -s <soundfilename>  16bit signed n-channel sound\
 stream/pipe", 62ul);
               osi_WrStrLn(" -t <url:port>       connect rtl_tcp server (127.\
0.0.1:1234)", 61ul);
               osi_WrStrLn(" -v                  show rssi (dB) and afc (khz)\
", 50ul);
               osi_WrStrLn("example: -s /tmp/sound.pcm -t 192.168.1.1:1234 -p\
 5 72 -p 8 1 -v", 65ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("config file: (re-read every some seconds and may \
be modified any time)", 71ul);
               osi_WrStrLn("  # comment", 12ul);
               osi_WrStrLn("  p <cmd> <value>  rtl_tcp parameter like \'p 5 5\
0\' ppm, \'p 8 1\' autogain", 73ul);
               osi_WrStrLn("  f <mhz> <afc-range> <squelch%> <lowpass%> <if-w\
idth>", 55ul);
               osi_WrStrLn("    AFC-range in +-kHz, Squelch 0 off, 100 open, \
95 near open", 62ul);
               osi_WrStrLn("    audio lowpass in % Nyquist frequ. of output s\
ampelrate, 0 is off", 69ul);
               osi_WrStrLn("    if-width 1=12kHz 2=24kHz .. 5=192kHz (5 only \
with 2048kHz input sample)", 76ul);
               osi_WrStrLn("  example:", 11ul);
               osi_WrStrLn("    p 5 50", 11ul);
               osi_WrStrLn("    p 8 1", 10ul);
               osi_WrStrLn("    f 438.825 5 95 70", 22ul);
               osi_WrStrLn("    f 439.275 5 90 70", 22ul);
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
         prx[i] = &rxx[i];
         rxx[i].df = freq[i].khz-midfreq;
         rxx[i].maxafc = freq[i].afc;
         rxx[i].squelch = squelchs[i].lev!=0.0f;
         rxx[i].width = (freq[i].width*iqrate)/2048000UL;
         ++i;
      }
      prx[i] = 0;
   }
   if (midfreq<40000UL) Error("no valid frequency", 19ul);
   if (midfreq!=lastmidfreq) {
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
   unsigned long wid;
   unsigned long lpp;
   unsigned long sq;
   unsigned long p;
   unsigned long lino;
   unsigned long m;
   unsigned long n;
   unsigned long i;
   float x;
   char ok0;
   char b[10001];
   char li[256];
   struct FREQTAB freq[32];
   fd0 = osic_OpenRead(parmfn, 1001ul);
   if (fd0>=0L) {
      len = osic_RdBin(fd0, (char *)b, 10001u/1u, 10001UL);
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
            if (X2C_CAP(li[i])=='P') {
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
                  else setstickparm(n, m);
               }
               i = 0UL;
            }
            else if (X2C_CAP(li[i])=='F') {
               ++i;
               skip(li, 256ul, &i);
               fix(li, 256ul, &i, &x, &ok0);
               if (!ok0) osi_WerrLn("wrong value", 12ul);
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &m, &ok0);
               if (!ok0) m = 0UL;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &sq, &ok0);
               if (!ok0) sq = 0UL;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &lpp, &ok0);
               if (!ok0) lpp = 0UL;
               skip(li, 256ul, &i);
               card(li, 256ul, &i, &wid, &ok0);
               if (!ok0) wid = 0UL;
               if (freqc>31UL) osi_WerrLn("freq table full", 16ul);
               else {
                  freq[freqc].khz = (unsigned long)X2C_TRUNCC(x*1000.0f+0.5f,
                0UL,X2C_max_longcard);
                  freq[freqc].afc = m;
                  if (wid==0UL) wid = 128UL;
                  else if (wid==1UL) wid = 64UL;
                  else if (wid==2UL) wid = 32UL;
                  else if (wid==3UL) wid = 16UL;
                  else wid = 8UL;
                  freq[freqc].width = wid;
                  squelchs[freqc].lev = X2C_DIVR((float)sq,200.0f);
                  squelchs[freqc].lp = X2C_DIVR((float)lpp,200.0f);
                  ++freqc;
               }
               i = 0UL;
            }
            else if (li[i]=='#') i = 0UL;
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
   unsigned long i;
   char s[31];
   i = 0UL;
   while (prx[i]) {
      aprsstr_FixToStr(RealMath_ln((rxx[i].rssi+1.0f)*3.0517578125E-5f)
                *4.342944819f, 2UL, s, 31ul);
      osi_Werr(s, 31ul);
      osi_Werr("dB", 3ul);
      if (rxx[i].squelch) {
         osi_Werr(" ", 2ul);
         aprsstr_FixToStr(rxx[i].sqmed, 3UL, s, 31ul);
         osi_Werr(s, 31ul);
      }
      osi_Werr(" ", 2ul);
      aprsstr_IntToStr(rxx[i].afckhz, 0UL, s, 31ul);
      osi_Werr(s, 31ul);
      osi_WerrLn(" ", 2ul);
      ++i;
   }
   osi_Werr("    \015", 6ul);
} /* end showrssi() */

static long sp;

static long sn;

static unsigned long rp;

static float sqv;

static unsigned long tshow;

static char recon;

static long pcm;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   long tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   sdr_BEGIN();
   aprsstr_BEGIN();
   midfreq = 0UL;
   lastmidfreq = 0UL;
   tshow = 0UL;
   memset((char *)rxx,(char)0,sizeof(struct sdr_RX [32]));
   memset((char *)stickparm,(char)0,sizeof(struct STICKPARM [256]));
   memset((char *)squelchs,(char)0,sizeof(struct SQUELCH [33]));
   Parms();
   for (freqc = 0UL; freqc<=31UL; freqc++) {
      rxx[freqc].samples = (sdr_pAUDIOSAMPLE)sampx[freqc];
   } /* end for */
   if (sdr_startsdr(url, 1001ul, port, 1001ul, iqrate, samphz, reconn)) {
      rdconfig();
      sndw = 0UL;
      fd = osic_OpenWrite(soundfn, 1001ul);
      if (fd>=0L) {
         recon = 1;
         for (;;) {
            sn = sdr_getsdr(16UL, prx, 33ul);
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
               tmp = sn-1L;
               sp = 0L;
               if (sp<=tmp) for (;; sp++) {
                  rp = 0UL;
                  while (prx[rp]) {
                     pcm = (long)rxx[rp].samples[sp];
                     { /* with */
                        struct SQUELCH * anonym = &squelchs[rp];
                        if (anonym->lev!=0.0f) {
                           sqv = (anonym->lev-rxx[rp].sqmed)*10.0f;
                           if (sqv<0.0f) sqv = 0.0f;
                           else if (sqv>1.0f) sqv = 1.0f;
                           pcm = (long)(short)X2C_TRUNCI((float)pcm*sqv,
                -32768,32767);
                        }
                        if (anonym->lp!=0.0f) {
                           anonym->u1 = anonym->u1+(((float)(pcm*2L)
                -anonym->u1)-anonym->il)*anonym->lp;
                           anonym->u2 = anonym->u2+(anonym->il-anonym->u2)
                *anonym->lp;
                           anonym->il = anonym->il+(anonym->u1-anonym->u2)
                *anonym->lp*2.0f;
                           pcm = (long)(short)X2C_TRUNCI(anonym->u2,-32768,
                32767);
                        }
                     }
                     pcm = (long)((unsigned long)pcm&0xFFFFFFFEUL);
                     if (rp==0UL) ++pcm;
                     sndbuf[sndw] = (short)pcm;
                     ++sndw;
                     if (sndw>1023UL) {
                        osic_WrBin(fd, (char *)sndbuf, 2048u/1u, sndw*2UL);
                        sndw = 0UL;
                        if (tshow>20UL) {
                           rdconfig();
                           setparms(recon);
                           recon = 0;
                           if (verb) showrssi();
                           tshow = 0UL;
                        }
                        else ++tshow;
                     }
                     ++rp;
                  }
                  if (sp==tmp) break;
               } /* end for */
            }
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
