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
#define sondeudp_C_
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#include <fcntl.h>
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef Storage_H_
#include "Storage.h"
#endif
#ifndef TimeConv_H_
#include "TimeConv.h"
#endif

/* demodulate RS92 sonde (2400bit/s manchester)
                and send as AXUDP by OE5DXL */
#define sondeudp_MAXLEN 9
/* data frame size c34 */

#define sondeudp_ADCBUFLEN 4096

#define sondeudp_BAUDSAMP 65536

#define sondeudp_PLLSHIFT 1024

#define sondeudp_DFIRLEN 64

#define sondeudp_AFIRLEN 32

#define sondeudp_AOVERSAMP 16
/*16*/

#define sondeudp_ASYNBITS 10

static char sondeudp_CALIBFRAME = 'e';

static char sondeudp_GPSFRAME = 'g';

static char sondeudp_UNKNOWN = 'h';

static char sondeudp_DATAFRAME = 'i';

#define sondeudp_DOVERSAMP 16

#define sondeudp_CIDTIMEOUT 3600
/* s to delete c34 sonde id */

typedef char FILENAME[1024];

enum CHANNELS {sondeudp_LEFT, sondeudp_RIGHT};


typedef float AFIRTAB[512];

typedef float DFIRTAB[1024];

typedef float DFIR[64];

struct UDPTX;

typedef struct UDPTX * pUDPTX;


struct UDPTX {
   pUDPTX next;
   unsigned long ip;
   unsigned long destport;
   long udpfd;
};

struct R92;


struct R92 {
   char enabled;
   long pllshift;
   long baudfine;
   long manchestd;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char data1;
   char lastmanch;
   unsigned long rxbyte;
   unsigned long rxbitc;
   unsigned long rxp;
   char rxbuf[256];
   AFIRTAB afirtab;
   long asynst[10];
};

typedef char CNAMESTR[9];

struct C34;


struct C34 {
   char enabled;
   CNAMESTR id;
   CNAMESTR idcheck;
   unsigned long idtime;
   unsigned long idcnt;
   long pllshift;
   long baudfine;
   long leveldcd;
   float sqmed[2];
   float afskhighpass;
   float freq;
   float left;
   float tcnt;
   float afskmidfreq;
   float afmid;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char data1;
   char lastmanch;
   unsigned long rxbyte;
   unsigned long rxbitc;
   unsigned long rxp;
   char rxbuf[9];
   AFIRTAB afirtab;
   long asynst[10];
   DFIRTAB dfirtab;
   DFIR dfir;
   unsigned long dfin;
   unsigned long confignyquist;
   unsigned long configafskshift;
   unsigned long configafskmid;
   unsigned long demodbaud0;
   unsigned long configbaud0;
   unsigned long txbaud;
   unsigned long dcdclock;
   float hipasscap;
};

struct CHAN;


struct CHAN {
   long adcmax;
   float afir[32];
   long configequalizer;
   pUDPTX udptx;
   unsigned long mycallc;
   char myssid;
   struct R92 r92;
   struct C34 c34;
};

static long soundfd;

static long debfd;

static char abortonsounderr;

static char verb;

static unsigned long getst;

static unsigned long afin;

static unsigned long soundbufs;

static unsigned long adcrate;

static unsigned long adcbuflen;

static unsigned long adcbytes;

static unsigned long fragmentsize;

static FILENAME soundfn;

static struct CHAN chan[2];

static unsigned char maxchannels;

static unsigned long demodbaud;

static unsigned long configbaud;


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   InOut_WriteString(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void Hamming(float f[], unsigned long f_len)
{
   unsigned long i;
   unsigned long tmp;
   tmp = f_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      f[i] = f[i]*(0.54f+0.46f*RealMath_cos(3.141592f*(X2C_DIVR((float)i,
                (float)(1UL+(f_len-1))))));
      if (i==tmp) break;
   } /* end for */
} /* end Hamming() */


static void initdfir(DFIRTAB dfirtab, unsigned long fg)
{
   unsigned long f;
   unsigned long i;
   float t[512];
   float e;
   float f1;
   unsigned long tmp;
   for (i = 0UL; i<=511UL; i++) {
      t[i] = 0.5f;
   } /* end for */
   f1 = X2C_DIVR((float)(fg*64UL),(float)adcrate);
   tmp = (unsigned long)X2C_TRUNCC(f1,0UL,X2C_max_longcard)+1UL;
   f = 1UL;
   if (f<=tmp) for (;; f++) {
      e = 1.0f;
      if (f==(unsigned long)X2C_TRUNCC(f1,0UL,X2C_max_longcard)+1UL) {
         e = f1-(float)(unsigned long)X2C_TRUNCC(f1,0UL,X2C_max_longcard);
      }
      for (i = 0UL; i<=511UL; i++) {
         t[i] = t[i]+e*RealMath_cos(X2C_DIVR(3.141592f*(float)(i*f),512.0f));
      } /* end for */
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 512ul);
   for (i = 0UL; i<=511UL; i++) {
      t[i] = t[i]*(0.54f+0.46f*RealMath_cos(3.141592f*(X2C_DIVR((float)i,
                512.0f))));
   } /* end for */
   for (i = 0UL; i<=511UL; i++) {
      dfirtab[511UL+i] = t[i];
      dfirtab[511UL-i] = t[i];
   } /* end for */
} /* end initdfir() */


static void initafir(AFIRTAB atab, unsigned long F0, unsigned long F1,
                float eq)
{
   unsigned long f;
   unsigned long i;
   float t[256];
   float f10;
   float f00;
   float e;
   unsigned long tmp;
   f00 = X2C_DIVR((float)(F0*32UL),(float)adcrate);
   f10 = X2C_DIVR((float)(F1*32UL),(float)adcrate);
   for (i = 0UL; i<=255UL; i++) {
      t[i] = 0.0f;
   } /* end for */
   tmp = (unsigned long)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL;
   f = (unsigned long)X2C_TRUNCC(f00,0UL,X2C_max_longcard);
   if (f<=tmp) for (;; f++) {
      e = 1.0f+eq*((X2C_DIVR((float)f,X2C_DIVR((float)((F0+F1)*32UL),
                (float)adcrate)))*2.0f-1.0f);
      /*
          e:=1.0 + eq*(FLOAT(f)/FLOAT((F0+F1)*AFIRLEN DIV adcrate)*2.0-1.0);
      */
      if (e<0.0f) e = 0.0f;
      if (f==(unsigned long)X2C_TRUNCC(f00,0UL,X2C_max_longcard)) {
         e = e*(1.0f-(f00-(float)(unsigned long)X2C_TRUNCC(f00,0UL,
                X2C_max_longcard)));
      }
      if (f==(unsigned long)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL) {
         e = e*(f10-(float)(unsigned long)X2C_TRUNCC(f10,0UL,
                X2C_max_longcard));
      }
      /*
      IF eq<>0 THEN IO.WrFixed(e,2,2);IO.WrLn; END;
      */
      if (f==0UL) {
         for (i = 0UL; i<=255UL; i++) {
            t[i] = t[i]+e*0.5f;
         } /* end for */
      }
      else {
         for (i = 0UL; i<=255UL; i++) {
            t[i] = t[i]+e*RealMath_cos(X2C_DIVR(3.141592f*(float)(i*f),
                256.0f));
         } /* end for */
      }
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 256ul);
   for (i = 0UL; i<=255UL; i++) {
      atab[255UL+i] = t[i];
      atab[255UL-i] = t[i];
   } /* end for */
   if (F0>0UL) {
      /* make dc level zero */
      e = 0.0f;
      for (i = 0UL; i<=511UL; i++) {
         e = e+atab[i];
      } /* end for */
      e = X2C_DIVR(e,512.0f);
      for (i = 0UL; i<=511UL; i++) {
         atab[i] = atab[i]-e;
      } /* end for */
   }
/*
IO.WrLn;
FOR i:=0 TO HIGH(atab) DO IO.WrFixed(atab[i], 2,8) END;
IO.WrLn;
*/
/*
IF eq<>0.0 THEN
debfd:=FIO.Create("/tmp/ta.raw");
FOR i:=0 TO HIGH(atab) DO f:=VAL(INTEGER, atab[i]*1000.0);
                FIO.WrBin(debfd,f,2) END;
FIO.Close(debfd);
END;
*/
} /* end initafir() */


static void OpenSound(void)
{
   long s;
   long i;
   soundfd = osi_OpenRW(soundfn, 1024ul);
   if (soundfd>=0L) {
      i = samplesize(soundfd, 16UL); /* 8, 16 */
      i = channels(soundfd, (unsigned long)maxchannels+1UL); /* 1, 2  */
      i = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
      if (i) {
         InOut_WriteString("sound setfragment returns ", 27ul);
         InOut_WriteInt(i, 1UL);
         osi_WrLn();
      }
      i = sampelrate(soundfd, adcrate); /* 8000..48000 */
      s = (long)getsampelrate(soundfd);
      if (s!=(long)adcrate) {
         InOut_WriteString("sound device returns ", 22ul);
         InOut_WriteInt(s, 1UL);
         osi_WrStrLn("Hz!", 4ul);
      }
   }
   else if (abortonsounderr) {
      InOut_WriteString(soundfn, 1024ul);
      Error(" open", 6ul);
   }
} /* end OpenSound() */


static char packcall(char cs[], unsigned long cs_len, unsigned long * cc,
                char * ssid)
{
   unsigned long s;
   unsigned long j;
   unsigned long i;
   char c;
   char packcall_ret;
   X2C_PCOPY((void **)&cs,cs_len);
   cs[cs_len-1] = 0;
   *cc = 0UL;
   s = 0UL;
   i = 0UL;
   for (j = 0UL; j<=5UL; j++) {
      *cc =  *cc*37UL;
      c = cs[i];
      if ((unsigned char)c>='A' && (unsigned char)c<='Z') {
         *cc += ((unsigned long)(unsigned char)c-65UL)+1UL;
         ++i;
      }
      else if ((unsigned char)c>='0' && (unsigned char)c<='9') {
         *cc += ((unsigned long)(unsigned char)c-48UL)+27UL;
         ++i;
      }
      else if (c && c!='-') {
         packcall_ret = 0;
         goto label;
      }
   } /* end for */
   if (cs[i]=='-') {
      /* ssid */
      ++i;
      c = cs[i];
      if ((unsigned char)c>='0' && (unsigned char)c<='9') {
         s += (unsigned long)(unsigned char)c-48UL;
         ++i;
         c = cs[i];
         if ((unsigned char)c>='0' && (unsigned char)c<='9') {
            s = (s*10UL+(unsigned long)(unsigned char)c)-48UL;
         }
      }
      if (s>15UL) {
         packcall_ret = 0;
         goto label;
      }
   }
   else if (cs[i]) {
      packcall_ret = 0;
      goto label;
   }
   *ssid = (char)s;
   packcall_ret = *cc>0UL;
   label:;
   X2C_PFREE(cs);
   return packcall_ret;
} /* end packcall() */


static long GetIp(char h[], unsigned long h_len, unsigned long * ip,
                unsigned long * port)
{
   unsigned long p;
   unsigned long n;
   unsigned long i;
   char ok0;
   long GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((unsigned char)h[p]>='0' && (unsigned char)h[p]<='9') {
         ok0 = 1;
         n = (n*10UL+(unsigned long)(unsigned char)h[p])-48UL;
         ++p;
      }
      if (!ok0) {
         GetIp_ret = -1L;
         goto label;
      }
      if (i<3UL) {
         if (h[p]!='.' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[p]!=':' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++p;
   } /* end for */
   GetIp_ret = openudp();
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static void Config(void)
{
   unsigned long i;
   unsigned char c;
   struct R92 * anonym;
   struct C34 * anonym0;
   for (c = sondeudp_LEFT;; c++) {
      { /* with */
         struct R92 * anonym = &chan[c].r92;
         configbaud = 4800UL;
         demodbaud = (2UL*configbaud*65536UL)/adcrate;
         initafir(anonym->afirtab, 300UL, 3600UL,
                X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym->baudfine = 0L;
         anonym->noise = 0.0f;
         anonym->bitlev = 0.0f;
         anonym->cbit = 0;
         anonym->rxp = 0UL;
         anonym->rxbitc = 0UL;
         anonym->manchestd = 0L;
         anonym->lastmanch = 0;
         anonym->rxbyte = 0UL;
         for (i = 0UL; i<=9UL; i++) {
            anonym->asynst[i] = 0L;
         } /* end for */
      }
      { /* with */
         struct C34 * anonym0 = &chan[c].c34;
         anonym0->txbaud = (anonym0->configbaud0*65536UL)/adcrate;
         anonym0->demodbaud0 = anonym0->txbaud*2UL;
         anonym0->afskmidfreq = X2C_DIVR((float)anonym0->configafskmid*2.0f,
                (float)adcrate);
         initafir(anonym0->afirtab,
                (anonym0->configafskmid-anonym0->configafskshift/2UL)
                -anonym0->configbaud0/4UL,
                anonym0->configafskmid+anonym0->configafskshift/2UL+anonym0->configbaud0/4UL,
                 X2C_DIVR((float)chan[c].configequalizer,100.0f));
         initdfir(anonym0->dfirtab,
                (anonym0->configbaud0*anonym0->confignyquist)/100UL);
         anonym0->baudfine = 0L;
         anonym0->left = 0.0f;
         anonym0->tcnt = 0.0f;
         anonym0->freq = 0.0f;
         anonym0->dfin = 0UL;
         anonym0->cbit = 0;
         anonym0->rxp = 0UL;
         anonym0->rxbitc = 0UL;
      }
      if (c==sondeudp_RIGHT) break;
   } /* end for */
} /* end Config() */


static void Parms(void)
{
   char err;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   unsigned long cnum;
   long inum;
   unsigned char channel;
   pUDPTX utx;
   char chanset;
   char mycall[11];
   unsigned long myc;
   char mys;
   struct R92 * anonym;
   struct C34 * anonym0;
   struct CHAN * anonym1;
   /* set only 1 chan */
   struct CHAN * anonym2;
   struct CHAN * anonym3;
   err = 0;
   abortonsounderr = 0;
   adcrate = 22050UL;
   adcbytes = 2UL;
   adcbuflen = 1024UL;
   fragmentsize = 11UL;
   maxchannels = sondeudp_LEFT;
   debfd = -1L;
   chanset = 0;
   for (channel = sondeudp_LEFT;; channel++) {
      { /* with */
         struct R92 * anonym = &chan[channel].r92;
         anonym->enabled = 1;
         anonym->pllshift = 1024L;
      }
      { /* with */
         struct C34 * anonym0 = &chan[channel].c34;
         anonym0->enabled = 1;
         anonym0->pllshift = 1024L;
         anonym0->confignyquist = 75UL;
         anonym0->pllshift = 1024L;
         anonym0->afskhighpass = 0.0f;
         anonym0->configbaud0 = 2400UL;
         anonym0->configafskshift = 1800UL;
         anonym0->configafskmid = 3800UL;
         anonym0->id[0] = 0;
         anonym0->idcheck[0] = 0;
         anonym0->idtime = 0UL;
         anonym0->idcnt = 0UL;
         anonym0->idcheck[0] = 0;
         anonym0->idtime = 0UL;
         anonym0->idcnt = 0UL;
      }
      { /* with */
         struct CHAN * anonym1 = &chan[channel];
         anonym1->configequalizer = 0L;
         anonym1->udptx = 0;
         anonym1->mycallc = 0UL;
      }
      if (channel==sondeudp_RIGHT) break;
   } /* end for */
   channel = sondeudp_LEFT;
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      Lib_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='a') abortonsounderr = 1;
         else if (h[1U]=='3') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].c34.enabled = 0;
            }
            else {
               /* use before -C set both */
               chan[sondeudp_LEFT].c34.enabled = 0;
               chan[sondeudp_RIGHT].c34.enabled = 0;
            }
         }
         else if (h[1U]=='9') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r92.enabled = 0;
            }
            else {
               /* use before -C set both */
               chan[sondeudp_LEFT].r92.enabled = 0;
               chan[sondeudp_RIGHT].r92.enabled = 0;
            }
         }
         else if (h[1U]=='c') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<1UL || cnum>2UL) Error("maxchannels 1..2", 17ul);
            maxchannels = (unsigned char)(cnum-1UL);
         }
         else if (h[1U]=='C') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>1UL) Error("channel 0 to 1", 15ul);
            channel = (unsigned char)cnum;
            chanset = 1;
         }
         else if (h[1U]=='D') {
            Lib_NextArg(h1, 1024ul);
            debfd = creat(h1, 420L);
         }
         else if (h[1U]=='e') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
            if (labs(inum)>999L) {
               Error("equalizer -999..999", 20ul);
            }
            chan[channel].configequalizer = inum;
         }
         else if (h[1U]=='f') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<8000UL || cnum>96000UL) {
               Error("sampelrate 8000..96000", 23ul);
            }
            adcrate = cnum;
         }
         else if (h[1U]=='F') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            chan[channel].c34.configafskmid = cnum;
         }
         else if (h[1U]=='l') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='o') Lib_NextArg(soundfn, 1024ul);
         else if (h[1U]=='I') {
            Lib_NextArg(mycall, 11ul);
            if (!packcall(mycall, 11ul, &myc, &mys)) {
               Error("-I illegall Callsign + ssid", 28ul);
            }
            if (chanset) {
               { /* with */
                  struct CHAN * anonym2 = &chan[channel];
                  anonym2->mycallc = myc;
                  anonym2->myssid = mys;
               }
            }
            else {
               /* use before -C set both */
               chan[sondeudp_LEFT].mycallc = myc;
               chan[sondeudp_LEFT].myssid = mys;
               chan[sondeudp_RIGHT].mycallc = myc;
               chan[sondeudp_RIGHT].myssid = mys;
            }
         }
         else if (h[1U]=='u') {
            Lib_NextArg(h, 1024ul);
            { /* with */
               struct CHAN * anonym3 = &chan[channel];
               Storage_ALLOCATE((X2C_ADDRESS *) &utx, sizeof(struct UDPTX));
               if (utx==0) Error("udp socket out of memory", 25ul);
               utx->udpfd = GetIp(h, 1024ul, &utx->ip, &utx->destport);
               if (utx->udpfd<0L) Error("cannot open udp socket", 23ul);
               utx->next = anonym3->udptx;
               anonym3->udptx = utx;
            }
         }
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("oss Mono/Stereo RS92-SGPA and SRSC34 Sonde Demodu\
lator to raw Frames", 69ul);
               osi_WrStrLn("sent via UDP to \'sondemod\' decoder, more demodu\
lators may send to same decoder", 79ul);
               osi_WrStrLn("Stereo used for 2 Rx for 2 Sondes or 1 Sonde with\
 Antenna-Diversity", 68ul);
               osi_WrStrLn(" -3             disable SRSC34 decoding (use -C b\
efore to select 1 channel)", 76ul);
               osi_WrStrLn(" -9             disable RG92 decoding (use -C bef\
ore to select 1 channel)", 74ul);
               osi_WrStrLn(" -a             abort on sounddevice error else r\
etry to open (USB audio...)", 77ul);
               osi_WrStrLn(" -c <num>       maxchannels (1) (1=mono, 2=stereo\
)", 51ul);
               osi_WrStrLn(" -C <num>       (0..1) channel parameters follow \
(repeat for each channel)", 75ul);
               osi_WrStrLn(" -D <filename>  write raw soundcard input data to\
 file or pipe", 63ul);
               osi_WrStrLn("                for debug or chaining demodulator\
s (equalizer diversity)", 73ul);
               osi_WrStrLn(" -e <num>       demod equalizer (0) 100=6db/oct h\
ighpass (-999..999)", 69ul);
               osi_WrStrLn("                -C <n> before -e sets channel num\
ber", 53ul);
               osi_WrStrLn(" -f <num>       adcrate (22050) (8000..96000)",
                46ul);
               osi_WrStrLn(" -I <call>      mycall + ssid (use -C before to s\
elect 1 channel)", 66ul);
               osi_WrStrLn(" -l <num>       adcbuflen (256)", 32ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or raw/\
wav audio file or pipe /dev/stdin", 83ul);
               osi_WrStrLn(" -u <x.x.x.x:destport> send rx data in udp (to so\
ndemod), -C <n> before sets", 77ul);
               osi_WrStrLn("                channel number, maybe repeated fo\
r more destinations", 69ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -v             verbous, (CRC-checked Sondename)",
                 49ul);
               osi_WrStrLn("example: sondeudp -f 16000 -o /dev/dsp -c 2 -C 0 \
-e 50 -u 127.0.0.1:4000", 73ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      InOut_WriteString(">", 2ul);
      InOut_WriteString(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
   Config();
   OpenSound();
} /* end Parms() */


static void sendudp(char data[], unsigned long data_len, long len,
                unsigned long ip, unsigned long destport, long udpfd)
{
   long i;
   X2C_PCOPY((void **)&data,data_len);
   i = udpsend(udpfd, data, len, destport, ip);
   X2C_PFREE(data);
} /* end sendudp() */


static void WrdB(long volt)
{
   if (volt>0L) {
      osi_WrFixed(RealMath_ln((float)volt)*8.685889638f-96.4f, 1L, 6UL);
      InOut_WriteString("dB", 3ul);
   }
} /* end WrdB() */


static void WrQ(float lev, float noise)
{
   if (lev>0.0f) {
      noise = X2C_DIVR(noise*200.0f,lev);
      if (noise>100.0f) noise = 100.0f;
      else if (noise<=0.0f) noise = 0.0f;
      InOut_WriteInt(100L-(long)X2C_TRUNCI(noise,X2C_min_longint,
                X2C_max_longint), 4UL);
      InOut_WriteString("%", 2ul);
   }
} /* end WrQ() */


static void WrQuali(float q)
{
   if (q>0.0f) {
      q = 100.5f-q*200.0f;
      if (q<0.0f) q = 0.0f;
      InOut_WriteString(" q:", 4ul);
      InOut_WriteInt((long)(unsigned long)X2C_TRUNCC(q,0UL,X2C_max_longcard),
                 1UL);
   }
} /* end WrQuali() */


static float noiselevel(unsigned char channel)
/* 0.0 perfect, ~0.25 noise only*/
{
   struct C34 * anonym;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      if (anonym->sqmed[1]==anonym->sqmed[0]) return 0.0f;
      else {
         return X2C_DIVR(anonym->noise,
                (float)fabs(anonym->sqmed[1]-anonym->sqmed[0]));
      }
   }
   return 0;
} /* end noiselevel() */


static float Fir(unsigned long in, unsigned long sub, unsigned long step,
                float fir[], unsigned long fir_len, float firtab[],
                unsigned long firtab_len)
{
   float s;
   unsigned long i;
   s = 0.0f;
   i = sub;
   do {
      s = s+fir[in]*firtab[i];
      ++in;
      if (in>fir_len-1) in = 0UL;
      i += step;
   } while (i<=firtab_len-1);
   return s;
} /* end Fir() */


static void alludp(pUDPTX utx, unsigned long len, char buf[],
                unsigned long buf_len)
{
   X2C_PCOPY((void **)&buf,buf_len);
   while (utx) {
      if (X2C_CHKNIL(pUDPTX,utx)->udpfd>=0L) {
         sendudp(buf, buf_len, (long)len, utx->ip, utx->destport,
                utx->udpfd);
      }
      utx = utx->next;
   }
   X2C_PFREE(buf);
} /* end alludp() */

static unsigned short sondeudp_POLYNOM = 0x1021U;


static void decodeframe92(unsigned char m)
{
   unsigned long ic;
   unsigned long len;
   unsigned long p;
   unsigned long j;
   unsigned short crc;
   unsigned char channel;
   struct CHAN * anonym;
   channel = m;
   if (chan[channel].udptx==0) {
      if (channel==sondeudp_LEFT) channel = sondeudp_RIGHT;
      else channel = sondeudp_LEFT;
   }
   { /* with */
      struct CHAN * anonym = &chan[channel];
      if (anonym->mycallc>0UL) {
         chan[m].r92.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         chan[m].r92.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].r92.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].r92.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         chan[m].r92.rxbuf[4U] = anonym->myssid;
      }
      alludp(anonym->udptx, 240UL, chan[m].r92.rxbuf, 256ul);
   }
   if (verb) {
      p = 6UL;
      if (chan[m].r92.rxbuf[6U]=='e') {
         ++p;
         len = (unsigned long)(unsigned char)chan[m].r92.rxbuf[7U]*2UL+2UL;
                /* +crc */
         ++p;
         j = 0UL;
         crc = 0xFFFFU;
         while (j<len && 8UL+j<240UL) {
            if (j+2UL<len) {
               for (ic = 0UL; ic<=7UL; ic++) {
                  if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (unsigned char)(unsigned char)chan[m].r92.rxbuf[p+j])) {
                     crc = X2C_LSH(crc,16,1)^0x1021U;
                  }
                  else crc = X2C_LSH(crc,16,1);
               } /* end for */
            }
            ++j;
         }
         if ((8UL+len>240UL || (char)crc!=chan[m].r92.rxbuf[(8UL+len)-2UL])
                || (char)X2C_LSH(crc,16,
                -8)!=chan[m].r92.rxbuf[(8UL+len)-1UL]) {
            if (verb) InOut_WriteString("----  crc err ", 15ul);
         }
         else {
            InOut_WriteInt((long)((unsigned long)(unsigned char)
                chan[m].r92.rxbuf[8U]+(unsigned long)(unsigned char)
                chan[m].r92.rxbuf[9U]*256UL), 4UL);
            j = 2UL;
            while ((unsigned char)chan[m].r92.rxbuf[8UL+j]>=' ') {
               InOut_WriteString((char *) &chan[m].r92.rxbuf[8UL+j], 1u/1u);
               ++j;
            }
         }
         if (m>sondeudp_LEFT) InOut_WriteString("             ", 14ul);
         WrdB(chan[m].adcmax);
         WrQ(chan[m].r92.bitlev, chan[m].r92.noise);
         osi_WrStrLn("", 1ul);
      }
   }
} /* end decodeframe92() */


static double latlong(unsigned long val)
{
   double hf;
   double hr;
   hr = (double)(X2C_DIVR((float)(val%0x080000000UL),1.E+6f));
   hf = (double)(float)(unsigned long)X2C_TRUNCC(hr,0UL,X2C_max_longcard);
   hr = hf+X2C_DIVL(hr-hf,0.6);
   if (val>=0x080000000UL) hr = -hr;
   return hr;
} /* end latlong() */


static char hex(unsigned long n)
{
   n = n&15UL;
   if (n<10UL) return (char)(n+48UL);
   else return (char)(n+55UL);
   return 0;
} /* end hex() */


static void demodframe34(unsigned char channel)
{
   unsigned long val;
   unsigned long sum2;
   unsigned long sum1;
   unsigned long i;
   double hr;
   char s[101];
   char good;
   char ok0;
   struct C34 * anonym;
   struct CHAN * anonym0; /* call if set */
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      sum1 = 0UL;
      sum2 = 65791UL;
      for (i = 2UL; i<=6UL; i++) {
         sum1 += (unsigned long)(unsigned char)anonym->rxbuf[i];
         sum2 -= (unsigned long)(unsigned char)anonym->rxbuf[i]*(7UL-i);
      } /* end for */
      sum1 = sum1&255UL;
      sum2 = sum2&255UL;
      ok0 = sum1==(unsigned long)(unsigned char)
                anonym->rxbuf[7U] && sum2==(unsigned long)(unsigned char)
                anonym->rxbuf[8U];
      good = 0;
      if (verb) {
         if ((unsigned long)maxchannels>0UL) {
            InOut_WriteInt((long)((unsigned long)channel+1UL), 1UL);
            InOut_WriteString(":", 2ul);
         }
         InOut_WriteString(anonym->id, 9ul);
         WrdB(chan[channel].adcmax);
         WrQuali(noiselevel(channel));
         InOut_WriteString(" [", 3ul);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[2U], 2UL);
         InOut_WriteString(" ", 2ul);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[3U], 2UL);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[4U], 2UL);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[5U], 2UL);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[6U], 2UL);
         InOut_WriteString(" ", 2ul);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[7U], 2UL);
         osi_WrHex((unsigned long)(unsigned char)anonym->rxbuf[8U], 2UL);
         InOut_WriteString("] ", 3ul);
      }
      if (ok0) {
         /* chksum ok */
         if (anonym->idtime+3600UL<TimeConv_time()) {
            anonym->id[0U] = 0;
            anonym->idcheck[0U] = 0;
            anonym->idcnt = 0UL;
         }
         /* remove old id */
         val = (unsigned long)(unsigned char)
                anonym->rxbuf[6U]+(unsigned long)(unsigned char)
                anonym->rxbuf[5U]*256UL+(unsigned long)(unsigned char)
                anonym->rxbuf[4U]*65536UL+(unsigned long)(unsigned char)
                anonym->rxbuf[3U]*16777216UL;
         hr = (double)*X2C_CAST(&val,unsigned long,float,float *);
         switch ((unsigned)anonym->rxbuf[2U]) {
         case '\001': /* something magic with this value */
            if (hr<99.9 && hr>(-99.9)) {
               if (verb) {
                  InOut_WriteString("pres", 5ul); /* WrFixed(hr, 2, 0);
                WrStr("hPa");*/
               }
            }
            break;
         case '\033':
            if (verb) {
               InOut_WriteString("hygr", 5ul);
               osi_WrFixed((float)hr, 2L, 0UL);
               InOut_WriteString("%", 2ul);
            }
            break;
         case '\003':
            if (hr<99.9 && hr>(-99.9)) {
               if (verb) {
                  InOut_WriteString("temp", 5ul);
                  osi_WrFixed((float)hr, 1L, 0UL);
                  InOut_WriteString("oC", 3ul);
               }
               good = 1;
            }
            break;
         case '\007':
            if (hr<99.9 && hr>(-99.9)) {
               if (verb) {
                  InOut_WriteString("dewp", 5ul);
                  osi_WrFixed((float)hr, 1L, 0UL);
                  InOut_WriteString("oC", 3ul);
               }
               good = 1;
            }
            break;
         case '\024':
            if (verb) {
               InOut_WriteString("date", 5ul);
               aprsstr_IntToStr((long)(val%1000000UL+1000000UL), 1UL, s,
                101ul);
               s[0U] = ' ';
               InOut_WriteString(s, 101ul);
            }
            good = 1;
            break;
         case '\025':
            if (verb) {
               aprsstr_TimeToStr((val/10000UL)*3600UL+((val%10000UL)/100UL)
                *60UL+val%100UL, s, 101ul);
               InOut_WriteString("time ", 6ul);
               InOut_WriteString(s, 101ul);
            }
            good = 1;
            break;
         case '\026':
            hr = latlong(val);
            if (hr<89.9 && hr>(-89.9)) {
               if (verb) {
                  InOut_WriteString("lati", 5ul);
                  osi_WrFixed((float)hr, 5L, 0UL);
               }
               good = 1;
            }
            break;
         case '\027':
            hr = latlong(val);
            if (hr<180.0 && hr>(-180.0)) {
               if (verb) {
                  InOut_WriteString("long", 5ul);
                  osi_WrFixed((float)hr, 5L, 0UL);
               }
               good = 1;
            }
            break;
         case '\030':
            hr = (double)((float)val*0.1f);
            if (hr<50000.0) {
               if (verb) {
                  InOut_WriteString("alti", 5ul);
                  osi_WrFixed((float)hr, 1L, 0UL);
                  InOut_WriteString("m", 2ul);
               }
               good = 1;
            }
            break;
         case '\031':
            hr = (double)((float)val*0.1609f);
                /*1.852*/ /* guess knots or miles */
            if (hr<1000.0) {
               if (verb) {
                  InOut_WriteString("wind", 5ul);
                  osi_WrFixed((float)hr, 1L, 0UL);
                  InOut_WriteString("km/h", 5ul);
               }
               good = 1;
            }
            break;
         case '\032':
            hr = (double)((float)val*0.1f);
            if (hr>=0.0 && hr<=360.0) {
               if (verb) {
                  InOut_WriteString("wdir", 5ul);
                  osi_WrFixed((float)hr, 1L, 0UL);
                  InOut_WriteString("deg", 4ul);
               }
               good = 1;
            }
            break;
         case 'd':
            strncpy(s,"SC34",101u);
                /* build a name from seems like serial number */
            s[4U] = hex(val/65536UL);
            s[5U] = hex(val/4096UL);
            s[6U] = hex(val/256UL);
            s[7U] = hex(val/16UL);
            s[8U] = hex(val);
            s[9U] = 0;
            if (verb) {
               InOut_WriteString("numb ", 6ul);
               InOut_WriteString((char *) &s[4U], 1u/1u);
               InOut_WriteString((char *) &s[5U], 1u/1u);
               InOut_WriteString((char *) &s[6U], 1u/1u);
               InOut_WriteString((char *) &s[7U], 1u/1u);
               InOut_WriteString((char *) &s[8U], 1u/1u);
            }
            /* check name, if changed may be checksum error or 2 sondes on same frequency */
            if (aprsstr_StrCmp(anonym->idcheck, 9ul, s, 101ul)) {
               ++anonym->idcnt; /* got same name again */
            }
            else {
               /* new name so check if wrong */
               aprsstr_Assign(anonym->idcheck, 9ul, s, 101ul);
               anonym->idcnt = 1UL;
            }
            if (anonym->idcnt>3UL || anonym->id[0U]==0) {
               /* first name or safe new name */
               memcpy(anonym->id,anonym->idcheck,9u);
               anonym->idtime = TimeConv_time();
            }
            good = 1;
            break;
         default:;
            if (verb) InOut_WriteString("????", 5ul);
            break;
         } /* end switch */
         /* build tx frame */
         if (anonym->id[0U] && aprsstr_StrCmp(anonym->idcheck, 9ul,
                anonym->id, 9ul)) {
            /* stop sending if ambigous id */
            for (i = 0UL; i<=8UL; i++) {
               s[i] = anonym->id[i];
            } /* end for */
            s[9U] = 0;
            { /* with */
               struct CHAN * anonym0 = &chan[channel];
               s[10U] = (char)(anonym0->mycallc/16777216UL);
               s[11U] = (char)(anonym0->mycallc/65536UL&255UL);
               s[12U] = (char)(anonym0->mycallc/256UL&255UL);
               s[13U] = (char)(anonym0->mycallc&255UL);
               if (anonym0->mycallc>0UL) s[14U] = anonym0->myssid;
               else s[14U] = '\020';
            }
            for (i = 0UL; i<=6UL; i++) {
               s[i+15UL] = anonym->rxbuf[i+2UL]; /* payload */
            } /* end for */
            alludp(chan[channel].udptx, 22UL, s, 101ul);
         }
         else if (verb && anonym->id[0U]) {
            InOut_WriteString(" changing name ", 16ul);
            InOut_WriteString(anonym->id, 9ul);
            InOut_WriteString("<->", 4ul);
            InOut_WriteString(anonym->idcheck, 9ul);
         }
      }
      else if (verb) {
         /*build tx frame */
         InOut_WriteString("---- chksum ", 13ul);
         osi_WrHex(sum1, 2UL);
         osi_WrHex(sum2, 2UL);
      }
      if (verb) osi_WrStrLn("", 1ul);
   }
/* name(9) 0C call(5) playload(7) */
} /* end demodframe34() */


static void stobyte(unsigned char m, char b)
{
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      anonym->rxbuf[anonym->rxp] = b;
      if (anonym->rxp>=5UL || b=='*') ++anonym->rxp;
      else anonym->rxp = 0UL;
      if (anonym->rxp>=240UL) {
         anonym->rxp = 0UL;
         decodeframe92(m);
      }
   }
} /* end stobyte() */


static void demodbyte(unsigned char m, char d)
{
   unsigned long maxi;
   unsigned long i;
   long max0;
   long n;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      anonym->rxbyte = anonym->rxbyte/2UL+256UL*(unsigned long)d;
      max0 = 0L;
      maxi = 0UL;
      for (i = 0UL; i<=9UL; i++) {
         n = anonym->asynst[i]-anonym->asynst[(i+1UL)%10UL];
         if (labs(n)>labs(max0)) {
            max0 = n;
            maxi = i;
         }
      } /* end for */
      if (anonym->rxbitc==maxi) {
         if (max0<0L) {
            anonym->rxbyte = (unsigned long)((unsigned long)
                anonym->rxbyte^0xFFUL);
         }
         stobyte(m, (char)(anonym->rxbyte&255UL));
      }
      if (d) {
         anonym->asynst[anonym->rxbitc]
                += X2C_DIV(32767L-anonym->asynst[anonym->rxbitc],16L);
      }
      else {
         anonym->asynst[anonym->rxbitc]
                -= X2C_DIV(32767L+anonym->asynst[anonym->rxbitc],16L);
      }
      anonym->rxbitc = (anonym->rxbitc+1UL)%10UL;
   }
/*FOR i:=0 TO HIGH(asynst) DO WrInt(asynst[i], 8) END; WrStrLn(""); */
} /* end demodbyte() */


static void demodbit92(unsigned char m, char d)
{
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      if (anonym->lastmanch==d) {
         anonym->manchestd += X2C_DIV(32767L-anonym->manchestd,16L);
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd>=0L) demodbyte(m, d);
   }
} /* end demodbit92() */


static void demod92(float u, unsigned char m)
{
   char d;
   struct R92 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   d = u>=0.0f;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      if (anonym->cbit) {
         demodbit92(m, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         /*quality*/
         anonym->bitlev = anonym->bitlev+((float)fabs(u)-anonym->bitlev)
                *0.05f;
         anonym->noise = anonym->noise+((float)fabs((float)fabs(u)
                -anonym->bitlev)-anonym->noise)*0.05f;
      }
      else {
         /*quality*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
} /* end demod92() */


static void Fsk(unsigned char m)
{
   float ff;
   long lim;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      lim = (long)demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (unsigned long)X2C_DIV(anonym->baudfine&65535L,
                4096L), 16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod92(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */


static void demodbit34(unsigned char channel, char d)
{
   struct C34 * anonym;
   d = !d;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      if (anonym->rxbitc==0UL) {
         if (!d) anonym->rxbitc = 1UL;
      }
      else if (anonym->rxbitc<=8UL) {
         /* databits */
         anonym->rxbyte = anonym->rxbyte/2UL;
         if (d) anonym->rxbyte += 128UL;
         ++anonym->rxbitc;
      }
      else {
         /* byte ready */
         anonym->rxbitc = 0UL;
         anonym->rxbuf[anonym->rxp] = (char)anonym->rxbyte;
         if (anonym->rxp==0UL) {
            if (anonym->rxbyte==0UL) anonym->rxp = 1UL;
         }
         else if (anonym->rxp==1UL) {
            if (anonym->rxbyte==255UL) anonym->rxp = 2UL;
            else anonym->rxp = 0UL;
         }
         else {
            ++anonym->rxp;
            if (anonym->rxp>8UL) {
               demodframe34(channel);
               anonym->rxp = 0UL;
            }
         }
      }
   }
} /* end demodbit34() */


static void demod34(float u, unsigned char channel)
{
   char d;
   struct C34 * anonym;
   d = u>=0.0f;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      if (anonym->cbit) {
         demodbit34(channel, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         /*squelch*/
         anonym->sqmed[d] = anonym->sqmed[d]+(u-anonym->sqmed[d])*0.05f;
         anonym->noise = anonym->noise+((float)fabs(u-anonym->sqmed[d])
                -anonym->noise)*0.05f;
      }
      else {
         /*squelch*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
} /* end demod34() */


static void Afsk(unsigned char channel)
{
   float ff;
   float b;
   float a;
   float d;
   float mid;
   float right;
   struct C34 * anonym;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      right = Fir(afin, 0UL, 16UL, chan[channel].afir, 32ul, anonym->afirtab,
                 512ul);
      if (anonym->left<0.0f!=right<0.0f) {
         d = X2C_DIVR(anonym->left,anonym->left-right);
         a = (float)(unsigned long)X2C_TRUNCC(d*16.0f+0.5f,0UL,
                X2C_max_longcard);
         b = a*0.0625f;
         if ((unsigned long)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)>0UL && (unsigned long)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)<16UL) {
            mid = Fir(afin, 16UL-(unsigned long)X2C_TRUNCC(a,0UL,
                X2C_max_longcard), 16UL, chan[channel].afir, 32ul,
                anonym->afirtab, 512ul);
            if (anonym->left<0.0f!=mid<0.0f) {
               d = (X2C_DIVR(anonym->left,anonym->left-mid))*b;
            }
            else d = b+(X2C_DIVR(mid,mid-right))*(1.0f-b);
         }
         if (anonym->tcnt+d!=0.0f) {
            anonym->freq = X2C_DIVR(1.0f,anonym->tcnt+d);
         }
         anonym->tcnt = 0.0f-d;
      }
      anonym->tcnt = anonym->tcnt+1.0f;
      anonym->left = right;
      anonym->dfir[anonym->dfin] = anonym->freq-anonym->afskmidfreq;
      anonym->dfin = anonym->dfin+1UL&63UL;
      anonym->baudfine += (long)anonym->demodbaud0;
      if (anonym->baudfine>=65536L) {
         anonym->baudfine -= 65536L;
         if (anonym->baudfine<65536L) {
            /* normal alway true */
            ff = Fir(anonym->dfin,
                (unsigned long)(16L-X2C_DIV(anonym->baudfine,4096L)), 16UL,
                anonym->dfir, 64ul, anonym->dfirtab, 1024ul);
            demod34(ff, channel);
         }
      }
   }
} /* end Afsk() */


static void getadc(void)
{
   short buf[4096];
   long minr;
   long maxr;
   long minl;
   long maxl;
   long sl;
   long i;
   long l;
   unsigned char c;
   l = osi_RdBin(soundfd, (char *)buf, 8192u/1u, adcbuflen*adcbytes);
   if (l<0L) {
      if (abortonsounderr) Error("Sounddevice Failure", 20ul);
      else {
         osi_Close(soundfd);
         Usleep(100000UL);
         OpenSound();
         return;
      }
   }
   if (debfd>=0L) osi_WrBin(debfd, (char *)buf, 8192u/1u, (unsigned long)l);
   l = (long)((unsigned long)l/adcbytes);
   for (c = sondeudp_LEFT;; c++) {
      chan[c].adcmax = chan[c].adcmax*15L>>4;
      if (c==sondeudp_RIGHT) break;
   } /* end for */
   maxl = -32768L;
   maxr = -32768L;
   minl = 32767L;
   minr = 32767L;
   i = 0L;
   while (i<l) {
      sl = (long)buf[i];
      chan[sondeudp_LEFT].afir[afin] = (float)sl;
      if (sl>maxl) maxl = sl;
      if (sl<minl) minl = sl;
      if (maxl-minl>chan[sondeudp_LEFT].adcmax) {
         chan[sondeudp_LEFT].adcmax = maxl-minl;
      }
      if (maxchannels>sondeudp_LEFT) {
         sl = (long)buf[i+1L];
         chan[sondeudp_RIGHT].afir[afin] = (float)sl;
         if (sl>maxr) maxr = sl;
         if (sl<minr) minr = sl;
         if (maxr-minr>chan[sondeudp_RIGHT].adcmax) {
            chan[sondeudp_RIGHT].adcmax = maxr-minr;
         }
      }
      afin = afin+1UL&31UL;
      if (chan[sondeudp_LEFT].r92.enabled) Fsk(sondeudp_LEFT);
      if (chan[sondeudp_LEFT].c34.enabled) Afsk(sondeudp_LEFT);
      if (maxchannels>sondeudp_LEFT) {
         if (chan[sondeudp_RIGHT].r92.enabled) Fsk(sondeudp_RIGHT);
         if (chan[sondeudp_RIGHT].c34.enabled) Afsk(sondeudp_RIGHT);
      }
      i += (long)((unsigned long)maxchannels+1UL);
   }
} /* end getadc() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(CNAMESTR)!=9) X2C_ASSERT(0);
   TimeConv_BEGIN();
   Storage_BEGIN();
   Lib_BEGIN();
   RealMath_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   /*  Gencrctab; */
   Parms();
   getst = 0UL;
   afin = 0UL;
   soundbufs = 0UL;
   if (verb) osi_WrStrLn("Frame ID       level-L qual level-R qual", 41ul);
   for (;;) getadc();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
