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
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif

/* demodulate RS92 sonde (2400bit/s manchester)
   and SRS-C34 (2400Bd AFSK 2000/3800Hz
   and DFM06 (2500bit/s manchester)
   and RS41 (4800Bd GFSK) and send as AXUDP by OE5DXL */
/*FROM fcntl IMPORT creat, open; */
/*FROM RealMath IMPORT pi, ln; */
/*FROM Lib IMPORT NextArg; */
/*FROM TimeConv IMPORT time; */
/*FROM math IMPORT cos,sin,atan,tan,pow,sqrt; */
#define sondeudp_MAXCHAN 32

#define sondeudp_ADCBYTES 2

#define sondeudp_MAXLEN 9
/* data frame size c34 */

#define sondeudp_ADCBUFLEN 4096

#define sondeudp_BAUDSAMP 65536

#define sondeudp_PLLSHIFT 1024

#define sondeudp_RAD 1.7453292519943E-2

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

#define sondeudp_FLEN6 264
/* dfm06 frame len */

#define sondeudp_DFMSYN 0x45CF 
/* frame sync sequence */

#define sondeudp_FLENRS41 520
/* rs41  frame len */

#define sondeudp_RHEAD41 "000010000110110101010011100010000100010001101001010\
0100000011111"

static unsigned char sondeudp_EXOR41[64] = {150U,131U,62U,81U,177U,73U,8U,
                152U,50U,5U,89U,14U,249U,68U,198U,38U,33U,96U,194U,234U,121U,
                93U,109U,161U,84U,105U,71U,12U,220U,232U,92U,241U,247U,118U,
                130U,127U,7U,153U,162U,44U,147U,124U,48U,99U,245U,16U,46U,
                97U,208U,188U,180U,182U,6U,170U,244U,35U,120U,110U,59U,174U,
                191U,123U,76U,193U};

typedef char FILENAME[1024];

typedef char CNAMESTR[9];

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
   float lastu;
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
   unsigned long demodbaud;
   unsigned long configbaud;
};

struct R41;


struct R41 {
   char enabled;
   long pllshift;
   long baudfine;
   float noise0;
   float bitlev0;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char data1;
   char rev;
   unsigned long rxbyte;
   unsigned long rxbitc;
   unsigned long rxp;
   char rxbuf[520];
   AFIRTAB afirtab;
   unsigned long demodbaud;
   unsigned long configbaud;
   unsigned long synp;
   char synbuf[64];
};

struct DFM6;


struct DFM6 {
   char enabled;
   CNAMESTR id;
   CNAMESTR idcheck;
   unsigned long idtime;
   unsigned long idcnt;
   unsigned long timediff;
   unsigned long actdate;
   unsigned long lastdate;
   unsigned long idnum;
   unsigned long num;
   unsigned long numcnt;
   unsigned long lastnum;
   long pllshift;
   long baudfine;
   long manchestd;
   float noise;
   float bitlev;
   float lastu;
   char polarity;
   char cbit;
   char oldd;
   char plld;
   char data1;
   char lastmanch;
   char wasdate;
   char txok;
   unsigned long rxp;
   char rxbuf[264];
   AFIRTAB afirtab;
   unsigned long synword;
   char cb[56];
   char ch[56];
   char db1[104];
   char db2[104];
   char dh1[104];
   char dh2[104];
   unsigned long demodbaud;
   unsigned long configbaud;
};

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
   unsigned long demodbaud;
   unsigned long configbaud;
   unsigned long txbaud;
   unsigned long dcdclock;
   float hipasscap;
};

struct CHAN;


struct CHAN {
   long adcmax;
   long adcdc;
   float afir[32];
   long configequalizer;
   pUDPTX udptx;
   unsigned long mycallc;
   char myssid;
   struct R92 r92;
   struct R41 r41;
   struct DFM6 dfm6;
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

static unsigned long adcbufrd;

static unsigned long adcbufsamps;

static unsigned long fragmentsize;

static FILENAME soundfn;

static struct CHAN chan[32];

static unsigned long adcbufsampx;

static unsigned long maxchannels;

static unsigned long cfgchannels;

static short adcbuf[4096];


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
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
      f[i] = f[i]*(0.54f+0.46f*osic_cos(3.1415926535898f*(X2C_DIVR((float)i,
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
         t[i] = t[i]+e*osic_cos(X2C_DIVR(3.1415926535898f*(float)(i*f),
                512.0f));
      } /* end for */
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 512ul);
   for (i = 0UL; i<=511UL; i++) {
      t[i] = t[i]*(0.54f+0.46f*osic_cos(3.1415926535898f*(X2C_DIVR((float)i,
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
            t[i] = t[i]+e*osic_cos(X2C_DIVR(3.1415926535898f*(float)(i*f),
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
      if (maxchannels<2UL) {
         i = samplesize(soundfd, 16UL); /* 8, 16 */
         i = channels(soundfd, maxchannels+1UL); /* 1, 2  */
         i = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
         if (i) {
            osi_WrStr("sound setfragment returns ", 27ul);
            osic_WrINT32((unsigned long)i, 1UL);
            osic_WrLn();
         }
         i = sampelrate(soundfd, adcrate); /* 8000..48000 */
         s = (long)getsampelrate(soundfd);
         if (s!=(long)adcrate) {
            osi_WrStr("sound device returns ", 22ul);
            osic_WrINT32((unsigned long)s, 1UL);
            osi_WrStrLn("Hz!", 4ul);
         }
      }
   }
   else if (abortonsounderr) {
      osi_WrStr(soundfn, 1024ul);
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
   unsigned long c;
   unsigned long i;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6 * anonym1;
   struct C34 * anonym2;
   for (c = 0UL; c<=31UL; c++) {
      { /* with */
         struct R92 * anonym = &chan[c].r92;
         anonym->configbaud = 4800UL;
         anonym->demodbaud = (2UL*anonym->configbaud*65536UL)/adcrate;
         initafir(anonym->afirtab, 0UL, 2800UL,
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
         struct R41 * anonym0 = &chan[c].r41;
         anonym0->configbaud = 4800UL;
         anonym0->demodbaud = (2UL*anonym0->configbaud*65536UL)/adcrate;
         initafir(anonym0->afirtab, 0UL, 2800UL,
                X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym0->baudfine = 0L;
         anonym0->noise = 0.0f;
         anonym0->bitlev = 0.0f;
         anonym0->cbit = 0;
         anonym0->rxp = 0UL;
         anonym0->rxbitc = 0UL;
         anonym0->rxbyte = 0UL;
         anonym0->synp = 0UL;
      }
      { /* with */
         struct DFM6 * anonym1 = &chan[c].dfm6;
         anonym1->configbaud = 2500UL;
         anonym1->demodbaud = (2UL*anonym1->configbaud*65536UL)/adcrate;
         initafir(anonym1->afirtab, 0UL, 1900UL,
                X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym1->baudfine = 0L;
         anonym1->noise = 0.0f;
         anonym1->bitlev = 0.0f;
         anonym1->cbit = 0;
         anonym1->rxp = 264UL; /* out of fram, wait for sync */
         anonym1->manchestd = 0L;
         anonym1->polarity = 0;
         anonym1->numcnt = 0UL;
         anonym1->txok = 0;
      }
      { /* with */
         struct C34 * anonym2 = &chan[c].c34;
         anonym2->txbaud = (anonym2->configbaud*65536UL)/adcrate;
         anonym2->demodbaud = anonym2->txbaud*2UL;
         anonym2->afskmidfreq = X2C_DIVR((float)anonym2->configafskmid*2.0f,
                (float)adcrate);
         initafir(anonym2->afirtab,
                (anonym2->configafskmid-anonym2->configafskshift/2UL)
                -anonym2->configbaud/4UL,
                anonym2->configafskmid+anonym2->configafskshift/2UL+anonym2->configbaud/4UL,
                 X2C_DIVR((float)chan[c].configequalizer,100.0f));
         initdfir(anonym2->dfirtab,
                (anonym2->configbaud*anonym2->confignyquist)/100UL);
         anonym2->baudfine = 0L;
         anonym2->left = 0.0f;
         anonym2->tcnt = 0.0f;
         anonym2->freq = 0.0f;
         anonym2->dfin = 0UL;
         anonym2->cbit = 0;
         anonym2->rxp = 0UL;
         anonym2->rxbitc = 0UL;
      }
   } /* end for */
} /* end Config() */


static void Parms(void)
{
   char err;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   unsigned long ch;
   unsigned long cnum;
   long inum;
   unsigned long channel;
   pUDPTX utx;
   char chanset;
   char mycall[11];
   unsigned long myc;
   char mys;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6 * anonym1;
   struct C34 * anonym2;
   struct CHAN * anonym3;
   /* set only 1 chan */
   struct CHAN * anonym4;
   err = 0;
   abortonsounderr = 0;
   adcrate = 22050UL;
   adcbuflen = 1024UL;
   fragmentsize = 11UL;
   maxchannels = 0UL;
   cfgchannels = 1UL; /* fix 1 channel */
   debfd = -1L;
   chanset = 0;
   for (channel = 0UL; channel<=31UL; channel++) {
      { /* with */
         struct R92 * anonym = &chan[channel].r92;
         anonym->enabled = 1;
         anonym->pllshift = 1024L;
      }
      { /* with */
         struct R41 * anonym0 = &chan[channel].r41;
         anonym0->enabled = 1;
         anonym0->pllshift = 1024L;
      }
      { /* with */
         struct DFM6 * anonym1 = &chan[channel].dfm6;
         anonym1->enabled = 1;
         anonym1->pllshift = 1024L;
      }
      { /* with */
         struct C34 * anonym2 = &chan[channel].c34;
         anonym2->enabled = 1;
         anonym2->pllshift = 1024L;
         anonym2->confignyquist = 75UL;
         anonym2->pllshift = 1024L;
         anonym2->afskhighpass = 0.0f;
         anonym2->configbaud = 2400UL;
         anonym2->configafskshift = 1800UL;
         anonym2->configafskmid = 3800UL;
         anonym2->id[0] = 0;
         anonym2->idcheck[0] = 0;
         anonym2->idtime = 0UL;
         anonym2->idcnt = 0UL;
         anonym2->idcheck[0] = 0;
         anonym2->idtime = 0UL;
         anonym2->idcnt = 0UL;
      }
      { /* with */
         struct CHAN * anonym3 = &chan[channel];
         anonym3->configequalizer = 0L;
         anonym3->udptx = 0;
         anonym3->mycallc = 0UL;
      }
   } /* end for */
   channel = 0UL;
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      osi_NextArg(h, 1024ul);
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
               for (ch = 0UL; ch<=31UL; ch++) {
                  chan[ch].c34.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='9') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r92.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=31UL; ch++) {
                  chan[ch].r92.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='4') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r41.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=31UL; ch++) {
                  chan[ch].r41.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='6') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].dfm6.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=31UL; ch++) {
                  chan[ch].dfm6.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=31UL) Error("maxchannels 0..max", 19ul);
            cfgchannels = cnum;
            if (cfgchannels>0UL) maxchannels = cfgchannels-1UL;
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>1UL) Error("channel 0 to max", 17ul);
            channel = cnum;
            chanset = 1;
         }
         else if (h[1U]=='D') {
            osi_NextArg(h1, 1024ul);
            debfd = osi_OpenWrite(h1, 1024ul);
         }
         else if (h[1U]=='e') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
            if (labs(inum)>999L) Error("equalizer -999..999", 20ul);
            chan[channel].configequalizer = inum;
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<8000UL || cnum>96000UL) {
               Error("sampelrate 8000..96000", 23ul);
            }
            adcrate = cnum;
         }
         else if (h[1U]=='F') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            chan[channel].c34.configafskmid = cnum;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='o') osi_NextArg(soundfn, 1024ul);
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 11ul);
            if (!packcall(mycall, 11ul, &myc, &mys)) {
               Error("-I illegall Callsign + ssid", 28ul);
            }
            if (chanset) {
               { /* with */
                  struct CHAN * anonym4 = &chan[channel];
                  anonym4->mycallc = myc;
                  anonym4->myssid = mys;
               }
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=31UL; ch++) {
                  chan[ch].mycallc = myc;
                  chan[ch].myssid = mys;
               } /* end for */
            }
         }
         else if (h[1U]=='u') {
            osi_NextArg(h, 1024ul);
            osic_alloc((X2C_ADDRESS *) &utx, sizeof(struct UDPTX));
            if (utx==0) Error("udp socket out of memory", 25ul);
            utx->udpfd = GetIp(h, 1024ul, &utx->ip, &utx->destport);
            if (utx->udpfd<0L) Error("cannot open udp socket", 23ul);
            if (chanset) {
               /* set only 1 chan */
               utx->next = chan[channel].udptx;
               chan[channel].udptx = utx;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=31UL; ch++) {
                  utx->next = chan[ch].udptx;
                  chan[ch].udptx = utx;
               } /* end for */
            }
         }
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("oss Mono/Stereo up to 32 Channel RS92, RS41, SRSC\
34 Sonde Demodulator to raw Frames", 84ul);
               osi_WrStrLn("sent via UDP to \'sondemod\' decoder, more demodu\
lators may send to same decoder", 79ul);
               osi_WrStrLn("Stereo used for 2 Rx for 2 Sondes or 1 Sonde with\
 Antenna-Diversity", 68ul);
               osi_WrStrLn(" -3             disable SRSC34 decoding (use -C b\
efore to select 1 channel)", 76ul);
               osi_WrStrLn(" -4             disable RS41 decoding (use -C bef\
ore to select 1 channel)", 74ul);
               osi_WrStrLn(" -6             disable DFM06 decoding (use -C be\
fore to select 1 channel)", 75ul);
               osi_WrStrLn(" -9             disable RS92 decoding (use -C bef\
ore to select 1 channel)", 74ul);
               osi_WrStrLn(" -a             abort on sounddevice error else r\
etry to open (USB audio...)", 77ul);
               osi_WrStrLn(" -c <num>       maxchannels, 0 for automatic chan\
nel number recognition", 72ul);
               osi_WrStrLn(" -C <num>       channel parameters follow (repeat\
 for each channel)", 68ul);
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
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
   if (adcbuflen*(maxchannels+1UL)>4096UL) {
      adcbuflen = 4096UL/(maxchannels+1UL);
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
      osic_WrFixed(osic_ln((float)volt)*8.685889638f-96.4f, 1L, 6UL);
      osi_WrStr("dB", 3ul);
   }
} /* end WrdB() */


static void WrQ(float lev, float noise)
{
   if (lev>0.0f) {
      noise = X2C_DIVR(noise*200.0f,lev);
      if (noise>100.0f) noise = 100.0f;
      else if (noise<=0.0f) noise = 0.0f;
      osic_WrINT32((unsigned long)(100L-(long)X2C_TRUNCI(noise,
                X2C_min_longint,X2C_max_longint)), 4UL);
      osi_WrStr("%", 2ul);
   }
} /* end WrQ() */


static void WrQuali(float q)
{
   if (q>0.0f) {
      q = 100.5f-q*200.0f;
      if (q<0.0f) q = 0.0f;
      osi_WrStr(" q:", 4ul);
      osic_WrINT32((unsigned long)X2C_TRUNCC(q,0UL,X2C_max_longcard), 1UL);
   }
} /* end WrQuali() */


static void Wrtune(long volt, long max0)
{
   long u;
   if (max0>0L && max0>labs(volt)) {
      u = (volt*100L)/max0;
      if (labs(u)>0L) {
         osi_WrStr(" f:", 4ul);
         osic_WrINT32((unsigned long)u, 2UL);
      }
      else osi_WrStr("     ", 6ul);
   }
} /* end Wrtune() */


static float noiselevel(unsigned long channel)
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
      if (utx->udpfd>=0L) {
         sendudp(buf, buf_len, (long)len, utx->ip, utx->destport,
                utx->udpfd);
      }
      utx = utx->next;
   }
   X2C_PFREE(buf);
} /* end alludp() */

static unsigned short sondeudp_POLYNOM = 0x1021U;


static void decodeframe92(unsigned long m)
{
   unsigned long ic;
   unsigned long len;
   unsigned long p;
   unsigned long j;
   unsigned short crc;
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
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
         if (maxchannels>0UL) {
            osic_WrINT32(m+1UL, 1UL);
            osi_WrStr(":", 2ul);
         }
         osi_WrStr("R92 ", 5ul);
         if ((8UL+len>240UL || (char)crc!=chan[m].r92.rxbuf[(8UL+len)-2UL])
                || (char)X2C_LSH(crc,16,
                -8)!=chan[m].r92.rxbuf[(8UL+len)-1UL]) {
            osi_WrStr("----  crc err ", 15ul);
         }
         else {
            j = 4UL;
            while ((unsigned char)chan[m].r92.rxbuf[8UL+j]>=' ') {
               osi_WrStr((char *) &chan[m].r92.rxbuf[8UL+j], 1u/1u);
               ++j;
            }
            osi_WrStr(" ", 2ul);
            osic_WrINT32((unsigned long)(unsigned char)
                chan[m].r92.rxbuf[8U]+(unsigned long)(unsigned char)
                chan[m].r92.rxbuf[9U]*256UL, 4UL);
         }
         /*      IF m>0 THEN WrStr("             ") END; */
         WrdB(chan[m].adcmax);
         WrQ(chan[m].r92.bitlev, chan[m].r92.noise);
         Wrtune(chan[m].adcdc, chan[m].adcmax);
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

/*------------------------------ RS41 */

static void sendrs41(unsigned long m)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         chan[m].r41.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         chan[m].r41.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].r41.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].r41.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         chan[m].r41.rxbuf[4U] = anonym->myssid;
         chan[m].r41.rxbuf[5U] = 0;
         chan[m].r41.rxbuf[6U] = 0;
      }
      alludp(anonym->udptx, 520UL, chan[m].r41.rxbuf, 520ul);
   }
} /* end sendrs41() */


static double atang2(double x, double y)
{
   double w;
   if (fabs(x)>fabs(y)) {
      w = (double)osic_arctan((float)(X2C_DIVL(y,x)));
      if (x<0.0) {
         if (y>0.0) w = 3.1415926535898+w;
         else w = w-3.1415926535898;
      }
   }
   else if (y!=0.0) {
      w = (double)(1.5707963267949f-osic_arctan((float)(X2C_DIVL(x,y))));
      if (y<0.0) w = w-3.1415926535898;
   }
   else w = 0.0;
   return w;
} /* end atang2() */

#define sondeudp_EARTHA 6.378137E+6

#define sondeudp_EARTHB 6.3567523142452E+6

#define sondeudp_E2 6.6943799901413E-3

#define sondeudp_EARTHAB 4.2841311513312E+4


static void wgs84r(double x, double y, double z, double * lat,
                double * long0, double * heig)
{
   double sl;
   double ct;
   double st;
   double t;
   double rh;
   double xh;
   double h;
   h = x*x+y*y;
   if (h>0.0) {
      rh = (double)osic_sqrt((float)h);
      xh = x+rh;
      *long0 = atang2(xh, y)*2.0;
      if (*long0>3.1415926535898) *long0 = *long0-6.2831853071796;
      t = (double)osic_arctan((float)(X2C_DIVL(z*1.003364089821,rh)));
      st = (double)osic_sin((float)t);
      ct = (double)osic_cos((float)t);
      *lat = (double)osic_arctan((float)
                (X2C_DIVL(z+4.2841311513312E+4*st*st*st,
                rh-4.269767270718E+4*ct*ct*ct)));
      sl = (double)osic_sin((float)*lat);
      *heig = X2C_DIVL(rh,(double)osic_cos((float)*lat))-(double)(X2C_DIVR(6.378137E+6f,
                osic_sqrt((float)(1.0-6.6943799901413E-3*sl*sl))));
   }
   else {
      *lat = 0.0;
      *long0 = 0.0;
      *heig = 0.0;
   }
/*  lat:=atan(z/(rh*(1.0 - E2))); */
/*  heig:=sqrt(h + z*z) - EARTHA; */
} /* end wgs84r() */


static long getint32(const char frame[], unsigned long frame_len,
                unsigned long p)
{
   unsigned long n;
   unsigned long i;
   n = 0UL;
   for (i = 3UL;; i--) {
      n = n*256UL+(unsigned long)(unsigned char)frame[p+i];
      if (i==0UL) break;
   } /* end for */
   return (long)n;
} /* end getint32() */


static long getint16(const char frame[], unsigned long frame_len,
                unsigned long p)
{
   unsigned long n;
   n = (unsigned long)(unsigned char)frame[p]+256UL*(unsigned long)
                (unsigned char)frame[p+1UL];
   if (n>=32768UL) return (long)(n-65536UL);
   return (long)n;
} /* end getint16() */


static void posrs41(const char b[], unsigned long b_len, unsigned long p)
{
   double dir;
   double vu;
   double ve;
   double vn;
   double vz;
   double vy;
   double vx;
   double heig;
   double long0;
   double lat;
   double z;
   double y;
   double x;
   x = (double)getint32(b, b_len, p)*0.01;
   y = (double)getint32(b, b_len, p+4UL)*0.01;
   z = (double)getint32(b, b_len, p+8UL)*0.01;
   wgs84r(x, y, z, &lat, &long0, &heig);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)(X2C_DIVL(lat,1.7453292519943E-2)), 5L, 1UL);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)(X2C_DIVL(long0,1.7453292519943E-2)), 5L, 1UL);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)heig, 1L, 1UL);
   osi_WrStr("m ", 3ul);
   /*speed */
   vx = (double)getint16(b, b_len, p+12UL)*0.01;
   vy = (double)getint16(b, b_len, p+14UL)*0.01;
   vz = (double)getint16(b, b_len, p+16UL)*0.01;
   vn = (-(vx*(double)osic_sin((float)lat)*(double)osic_cos((float)long0))
                -vy*(double)osic_sin((float)lat)*(double)osic_sin((float)
                long0))+vz*(double)osic_cos((float)lat);
   ve = -(vx*(double)osic_sin((float)long0))+vy*(double)osic_cos((float)
                long0);
   vu = vx*(double)osic_cos((float)lat)*(double)osic_cos((float)long0)
                +vy*(double)osic_cos((float)lat)*(double)osic_sin((float)
                long0)+vz*(double)osic_sin((float)lat);
   dir = X2C_DIVL(atang2(vn, ve),1.7453292519943E-2);
   if (dir<0.0) dir = 360.0+dir;
   osi_WrStr(" ", 2ul);
   osic_WrFixed(osic_sqrt((float)(vn*vn+ve*ve))*3.6f, 2L, 1UL);
   osi_WrStr("km/h ", 6ul);
   osic_WrFixed((float)dir, 1L, 1UL);
   osi_WrStr("deg ", 5ul);
   osic_WrFixed((float)vu, 1L, 1UL);
   osi_WrStr("m/s", 4ul);
} /* end posrs41() */

static unsigned short sondeudp_POLYNOM0 = 0x1021U;


static void decode41(unsigned long m)
{
   unsigned long ic;
   unsigned long len;
   unsigned long p;
   unsigned long j;
   unsigned long i;
   char ch;
   char typ;
   unsigned short crc;
   char nameok;
   struct R41 * anonym;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      nameok = 0;
      p = 57UL;
      if (verb) {
         if (maxchannels>0UL) {
            osic_WrINT32(m+1UL, 1UL);
            osi_WrStr(":", 2ul);
         }
         osi_WrStr("R41 ", 5ul);
      }
      for (;;) {
         if (p+4UL>=519UL) break;
         typ = anonym->rxbuf[p];
         ++p;
         len = (unsigned long)(unsigned char)anonym->rxbuf[p]+2UL;
         ++p;
         if (p+len>=519UL) break;
         /*
         WrStrLn("");
         FOR i:=0 TO len-1 DO WrHex(ORD(rxbuf[p+i]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;
         WrStrLn("");
         */
         j = 0UL;
         crc = 0xFFFFU;
         while (j<len && p+j<519UL) {
            if (j+2UL<len) {
               for (ic = 0UL; ic<=7UL; ic++) {
                  if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (unsigned char)(unsigned char)anonym->rxbuf[p+j])) {
                     crc = X2C_LSH(crc,16,1)^0x1021U;
                  }
                  else crc = X2C_LSH(crc,16,1);
               } /* end for */
            }
            ++j;
         }
         if ((p+len>519UL || (char)crc!=anonym->rxbuf[(p+len)-2UL]) || (char)
                X2C_LSH(crc,16,-8)!=anonym->rxbuf[(p+len)-1UL]) {
            if (verb) osi_WrStr(" ----  crc err ", 16ul);
            break;
         }
         if (typ=='y') {
            if (verb) {
               for (i = 0UL; i<=7UL; i++) {
                  ch = anonym->rxbuf[p+2UL+i];
                  if ((unsigned char)ch>' ' && (unsigned char)ch<'\177') {
                     osi_WrStr((char *) &ch, 1u/1u);
                  }
               } /* end for */
               osic_WrINT32((unsigned long)getint16(anonym->rxbuf, 520ul, p),
                 0UL);
            }
            nameok = 1;
            if (anonym->rxbuf[p+23UL]==0) {
               osi_WrStr(" ", 2ul);
               osic_WrFixed((float)(getint16(anonym->rxbuf, 520ul,
                p+26UL)/64L+40000L)*0.01f, 2L, 1UL);
               osi_WrStr("MHz", 4ul);
            }
         }
         else if (typ=='z') {
         }
         else if (typ=='|') {
         }
         else if (typ=='}') {
         }
         else if (typ=='{') {
            /*             WrStrLn("7A frame"); */
            /*             WrStrLn("7C frame"); */
            /*WrInt(getint32(rxbuf, p+2) DIV 1000 MOD 86400 , 10);
                WrStr("=gpstime "); */
            /*             WrStrLn("7D frame"); */
            /*             WrStrLn("7B frame"); */
            if (verb && p+17UL<519UL) posrs41(anonym->rxbuf, 520ul, p);
         }
         else if (typ=='~') {
         }
         else if (typ=='v') {
         }
         else {
            /*             WrStrLn("7E frame"); */
            /*             WrStrLn("76 frame"); */
            break;
         }
         /*        WrInt(getint16(rxbuf, 3BH), 0); */
         /*        WrStr(" ");WrHex(ORD(typ), 0);WrStr(" ");
                WrHex(p DIV 256, 0);WrHex(p, 0); */
         /*        WrStr(" "); */
         if (typ=='v') break;
         p += len;
      }
      if (verb) {
         WrdB(chan[m].adcmax);
         WrQ(anonym->bitlev0, anonym->noise0);
         Wrtune(chan[m].adcdc, chan[m].adcmax);
         /*WrStrLn(""); */
         /*FOR i:=0 TO HIGH(rxbuf) DO WrHex(ORD(rxbuf[i]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;  */
         osi_WrStrLn("", 1ul);
      }
   }
   if (nameok) sendrs41(m);
} /* end decode41() */

static unsigned char _cnst[64] = {150U,131U,62U,81U,177U,73U,8U,152U,50U,5U,
                89U,14U,249U,68U,198U,38U,33U,96U,194U,234U,121U,93U,109U,
                161U,84U,105U,71U,12U,220U,232U,92U,241U,247U,118U,130U,127U,
                7U,153U,162U,44U,147U,124U,48U,99U,245U,16U,46U,97U,208U,
                188U,180U,182U,6U,170U,244U,35U,120U,110U,59U,174U,191U,123U,
                76U,193U};

static void demodbyte41(unsigned long m, char d)
{
   unsigned long j;
   unsigned long i;
   char norm;
   /*WrStr(CHR(ORD(d)+48)); */
   struct R41 * anonym;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      if (anonym->rxp==0UL) {
         anonym->synbuf[anonym->synp] = d;
         i = anonym->synp;
         ++anonym->synp;
         if (anonym->synp>63UL) anonym->synp = 0UL;
         j = 56UL;
         norm = 1;
         anonym->rev = 1;
         do {
            --j;
            if (("00001000011011010101001110001000010001000110100101001000000\
11111"[j]=='1')==anonym->synbuf[i]) anonym->rev = 0;
            else norm = 0;
            if (i==0UL) i = 63UL;
            else --i;
         } while (!(j==24UL || !(anonym->rev || norm)));
         if (j==24UL) anonym->rxp = 7UL;
         anonym->rxbitc = 0UL;
      }
      else {
         anonym->rxbyte = anonym->rxbyte/2UL+128UL*(unsigned long)
                (d!=anonym->rev);
         ++anonym->rxbitc;
         if (anonym->rxbitc>=8UL) {
            anonym->rxbuf[anonym->rxp] = (char)((unsigned char)
                anonym->rxbyte^(unsigned char)_cnst[anonym->rxp&63UL]);
            ++anonym->rxp;
            if (anonym->rxp>=519UL) {
               decode41(m);
               anonym->rxp = 0UL;
            }
            if (anonym->rxp==200UL) {
               anonym->bitlev0 = anonym->bitlev;
                /* save quality before end of shortst frame */
               anonym->noise0 = anonym->noise;
            }
            anonym->rxbitc = 0UL;
         }
      }
   }
} /* end demodbyte41() */


static void demodbit41(unsigned long m, float u)
{
   char d;
   float ua;
   struct R41 * anonym;
   d = u>=0.0f;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      demodbyte41(m, d);
      /*quality*/
      ua = (float)fabs(u)-anonym->bitlev;
      anonym->bitlev = anonym->bitlev+ua*0.005f;
      anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
   }
/*quality*/
} /* end demodbit41() */

/*------------------------------ RS92 */

static void stobyte(unsigned long m, char b)
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


static void demodbyte(unsigned long m, char d)
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
                += (32767L-anonym->asynst[anonym->rxbitc])/16L;
      }
      else {
         anonym->asynst[anonym->rxbitc]
                -= (32767L+anonym->asynst[anonym->rxbitc])/16L;
      }
      anonym->rxbitc = (anonym->rxbitc+1UL)%10UL;
   }
/*FOR i:=0 TO HIGH(asynst) DO WrInt(asynst[i], 8) END; WrStrLn(""); */
} /* end demodbyte() */


static void demodbit92(unsigned long m, float u, float u0)
{
   char d;
   float ua;
   struct R92 * anonym;
   d = u>=u0;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      if (anonym->lastmanch==u0<0.0f) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd<0L) {
         demodbyte(m, d);
         /*WrInt(VAL(INTEGER, u0*0.001), 4); WrStr("/");
                WrInt(VAL(INTEGER, u*0.001), 0); */
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
      }
   }
/*quality*/
} /* end demodbit92() */


static void demod92(float u, unsigned long m)
{
   char d;
   struct R92 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      d = u>=0.0f;
      if (anonym->cbit) {
         if (anonym->enabled) demodbit92(m, u, anonym->lastu);
         if (chan[m].r41.enabled) demodbit41(m, u);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         /*
         (*quality*)
               ua:=ABS(u)+ABS(lastu);
               bitlev:=bitlev + (ua-bitlev)*0.05;
               noise:=noise + ((ua-bitlev)-noise)*0.05; 
         (*quality*)
         */
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod92() */


static void Fsk(unsigned long m)
{
   float ff;
   long lim;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      lim = (long)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (unsigned long)((anonym->baudfine&65535L)/4096L),
                16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod92(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */

/*------------------------------ DFM06 */

static void deinterleave(const char b[], unsigned long b_len,
                unsigned long base, unsigned long len, char db[],
                unsigned long db_len)
{
   unsigned long j;
   unsigned long i;
   unsigned long tmp;
   for (j = 0UL; j<=7UL; j++) {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         db[i*8UL+j] = b[base+len*j+i];
         if (i==tmp) break;
      } /* end for */
   } /* end for */
} /* end deinterleave() */


static char hamcorr(char b[], unsigned long b_len, unsigned long d,
                unsigned long h)
{
   unsigned long e;
   e = (unsigned long)((b[d]==b[d+2UL])!=(b[h]==b[h+2UL]))
                +2UL*(unsigned long)((b[d+1UL]==b[d+2UL])
                !=(b[h+1UL]==b[h+2UL]))+4UL*(unsigned long)((b[d+3UL]==b[h])
                !=(b[h+1UL]==b[h+2UL]));
   /* hamming matrix multiplication */
   if (e>4UL) b[(h+e)-4UL] = !b[(h+e)-4UL];
   else if (e>0UL) b[(d+e)-1UL] = !b[(d+e)-1UL];
   /*  IF e<>0 THEN WrStr("<");WrInt(e, 1);WrStr(">") END; */
   e = (unsigned long)b[d]+(unsigned long)b[d+1UL]+(unsigned long)
                b[d+2UL]+(unsigned long)b[d+3UL]+(unsigned long)
                b[h]+(unsigned long)b[h+1UL]+(unsigned long)
                b[h+2UL]+(unsigned long)b[h+3UL];
   return !(e&1);
/* 1 bit checksum */
} /* end hamcorr() */


static char hamming(const char b[], unsigned long b_len, unsigned long len,
                char db[], unsigned long db_len)
{
   unsigned long j;
   unsigned long i;
   unsigned long tmp;
   tmp = db_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      db[i] = 0;
      if (i==tmp) break;
   } /* end for */
   tmp = len-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      for (j = 0UL; j<=3UL; j++) {
         db[i*4UL+j] = b[i*8UL+j];
      } /* end for */
      for (j = 0UL; j<=3UL; j++) {
         db[i*4UL+j+len*4UL] = b[i*8UL+j+4UL];
      } /* end for */
      if (!hamcorr(db, db_len, i*4UL, i*4UL+len*4UL)) return 0;
      if (i==tmp) break;
   } /* end for */
   return 1;
/*
0000 0000
0001 1110
0010 1101
0011 0011
0100 1011
0101 0101
0110 0110
0111 1000
1000 0111
1001 1001
1010 1010
1011 0100
1100 1100
1101 0010
1110 0001
1111 1111
*/
} /* end hamming() */


static unsigned long bits2val(const char b[], unsigned long b_len,
                unsigned long from, unsigned long len)
{
   unsigned long n;
   n = 0UL;
   while (len>0UL) {
      n = n*2UL+(unsigned long)b[from];
      ++from;
      --len;
   }
   return n;
} /* end bits2val() */


static void wh(unsigned long x)
{
   char tmp;
   x = x&15UL;
   if (x<10UL) osi_WrStr((char *)(tmp = (char)(x+48UL),&tmp), 1u/1u);
   else osi_WrStr((char *)(tmp = (char)(x+55UL),&tmp), 1u/1u);
} /* end wh() */

static unsigned long sondeudp_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};

static unsigned long _cnst0[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,181UL,
                212UL,243UL,273UL,304UL,334UL};

static void decodesub(const char b[], unsigned long b_len, unsigned long m,
                unsigned long subnum)
{
   unsigned long u;
   unsigned long v;
   long ui;
   char s[101];
   char ok0;
   unsigned long tt;
   unsigned long tyear;
   unsigned long tmon;
   unsigned long tday;
   unsigned long thour;
   unsigned long tmin;
   /* check name, if changed may be checksum error or 2 sondes on same frequency */
   struct DFM6 * anonym;
   ok0 = 0;
   switch (bits2val(b, b_len, 48UL, 4UL)) {
   case 0UL: /* framecount */
      v = bits2val(b, b_len, 24UL, 8UL);
      if (verb) {
         osi_WrStr(" num: ", 7ul);
         osic_WrINT32(v, 0UL);
      }
      if ((chan[m].dfm6.num+1UL&255UL)==v && chan[m].dfm6.wasdate) {
         ++chan[m].dfm6.numcnt; /* date/num+1/date/num+1 sequence ok */
      }
      else chan[m].dfm6.numcnt = 0UL;
      chan[m].dfm6.num = v;
      chan[m].dfm6.wasdate = 0; /* we have number, next must be date */
      u = (((v+256UL)-(chan[m].dfm6.idnum&255UL)&255UL)+256UL)-(osic_time()
                +chan[m].dfm6.timediff&255UL)&255UL;
      if (u>=128UL) u = 256UL-u;
      chan[m].dfm6.txok = u<=1UL; /* max +-1s tolerance of num */
      if (verb) {
         osi_WrStr(" [", 3ul);
         aprsstr_IntToStr((long)u, 0UL, s, 101ul);
         if (chan[m].dfm6.txok) aprsstr_Append(s, 101ul, "s seq]", 7ul);
         else aprsstr_Append(s, 101ul, "s OUT OF seq - NO TX]", 22ul);
         osi_WrStr(s, 101ul);
      }
      if (chan[m].dfm6.lastdate+60UL==chan[m].dfm6.actdate) {
         /* minute change */
         if (chan[m].dfm6.numcnt>=3UL) {
            chan[m].dfm6.timediff = chan[m].dfm6.actdate-osic_time();
                /* sonde realtime - systime */
            v = (chan[m].dfm6.num+256UL)-(chan[m].dfm6.actdate&255UL)&255UL;
            if (verb) {
               osi_WrStr(" NEW ID", 8ul);
               osic_WrINT32(v, 0UL);
            }
            chan[m].dfm6.idnum = v;
            strncpy(s,"DFM6",101u); /* build a name from 1/1000s */
            s[4U] = (char)((v/100UL)%10UL+48UL);
            s[5U] = (char)((v/10UL)%10UL+48UL);
            s[6U] = (char)(v%10UL+48UL);
            s[7U] = 0;
            /*           IF verb THEN WrStr(" id: "); WrStr(s) END; */
            { /* with */
               struct DFM6 * anonym = &chan[m].dfm6;
               if (aprsstr_StrCmp(anonym->idcheck, 9ul, s, 101ul)) {
                  ++anonym->idcnt; /* got same name again */
               }
               else {
                  /* new name so check if wrong */
                  aprsstr_Assign(anonym->idcheck, 9ul, s, 101ul);
                  anonym->idcnt = 1UL;
               }
               if (anonym->idcnt>2UL || anonym->id[0U]==0) {
                  /* first name or safe new name */
                  memcpy(anonym->id,anonym->idcheck,9u);
                  anonym->idtime = osic_time();
               }
            }
         }
         chan[m].dfm6.numcnt = 0UL;
      }
      ok0 = 1;
      break;
   case 2UL:
      /*
         |1: v:=bits2val(b, 16, 32);                        (* time ms *)
             u:=v MOD 1000;
             ok:=TRUE;
      */
      v = bits2val(b, b_len, 0UL, 32UL);
      u = bits2val(b, b_len, 32UL, 16UL);
      if (verb) {
         osi_WrStr(" lat: ", 7ul);
         osic_WrFixed((float)v*1.E-7f, 5L, 0UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)u*0.036f, 1L, 0UL);
         osi_WrStr("km/h", 5ul);
      }
      ok0 = 1;
      break;
   case 3UL:
      v = bits2val(b, b_len, 0UL, 32UL);
      u = bits2val(b, b_len, 32UL, 16UL);
      if (verb) {
         osi_WrStr(" long:", 7ul);
         osic_WrFixed((float)v*1.E-7f, 5L, 0UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)u*0.01f, 1L, 0UL);
         osi_WrStr(" deg", 5ul);
      }
      ok0 = 1;
      break;
   case 4UL:
      v = bits2val(b, b_len, 0UL, 32UL);
      ui = (long)bits2val(b, b_len, 32UL, 16UL);
      if (ui>=32768L) ui -= 65536L;
      if (verb) {
         osi_WrStr(" alti:", 7ul);
         osic_WrFixed((float)v*0.01f, 1L, 0UL);
         osi_WrStr("m ", 3ul);
         osic_WrFixed((float)ui*0.01f, 1L, 0UL);
         osi_WrStr(" m/s", 5ul);
      }
      ok0 = 1;
      break;
   case 8UL: /* date */
      /*
         |5: ui:=bits2val(b, 8, 16);
             IF verb THEN
               WrStr(" mes5:"); WrInt(ui,0);
             END;
         |6: ui:=bits2val(b, 16, 32);
             IF verb THEN
               WrStr(" mes6:"); WrInt(ui,0);
             END;
         |7: ui:=bits2val(b, 16, 32);
             IF verb THEN
               WrStr(" mes7:"); WrInt(ui,0);
             END;
      */
      tmin = bits2val(b, b_len, 26UL, 6UL);
      thour = bits2val(b, b_len, 21UL, 5UL);
      tday = bits2val(b, b_len, 16UL, 5UL);
      tmon = bits2val(b, b_len, 12UL, 4UL);
      tyear = bits2val(b, b_len, 0UL, 12UL);
      if (tyear>=1970UL && tyear<2100UL) {
         /* make unix time */
         tt = (tyear-1970UL)*365UL+(tyear-1969UL)/4UL; /* days since 1970 */
         if (tmon<=12UL) {
            tt += _cnst0[tmon];
            if ((tyear&3UL)==0UL && tmon>2UL) ++tt;
         }
         tt = ((tt+tday)-1UL)*86400UL+thour*3600UL+tmin*60UL;
      }
      else tt = 0UL;
      /*DateToStr(tt, s); WrStr(s); WrStr("<date"); */
      if (chan[m].dfm6.wasdate) chan[m].dfm6.numcnt = 0UL;
      else ++chan[m].dfm6.numcnt;
      chan[m].dfm6.wasdate = 1;
      chan[m].dfm6.lastdate = chan[m].dfm6.actdate;
      chan[m].dfm6.actdate = tt;
      if (verb) {
         osi_WrStr(" ", 2ul);
         aprsstr_IntToStr((long)tyear, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, "-", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((long)tmon, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, "-", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((long)tday, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, " ", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((long)thour, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, ":", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((long)tmin, 0UL, s, 101ul);
         osi_WrStr(s, 101ul);
      }
      ok0 = 1;
      break;
   default:;
      if (verb) {
         if (subnum==0UL) osi_WrStr(" Dat", 5ul);
         else osi_WrStr(" dat", 5ul);
         wh(bits2val(b, b_len, 48UL, 4UL));
         osi_WrStr(":", 2ul);
         for (u = 0UL; u<=5UL; u++) {
            osic_WrHex(bits2val(b, b_len, u*8UL, 8UL), 0UL);
         } /* end for */
      }
      break;
   } /* end switch */
/*
      okk:=TRUE;
      FOR u:=0 TO 5 DO
        v:=bits2val(b, u*8, 8);
        IF (v<32) OR (v>128) THEN okk:=FALSE END;
      END;
      IF okk THEN
        WrStr(" [");
        FOR u:=0 TO 5 DO
          v:=bits2val(b, u*8, 8);
          IF v>=32 THEN WrStr(CHR(v)) ELSE WrStr(".") END;
        END;
        WrStr("]");
      END;
*/
} /* end decodesub() */


static void decodeframe6(unsigned long m)
{
   unsigned long j;
   unsigned long i;
   unsigned long rt;
   char s[101];
   char tx;
   struct DFM6 * anonym;
   /* build tx frame */
   struct DFM6 * anonym0;
   struct CHAN * anonym1; /* my call if set */
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      deinterleave(anonym->rxbuf, 264ul, 0UL, 7UL, anonym->ch, 56ul);
      deinterleave(anonym->rxbuf, 264ul, 56UL, 13UL, anonym->dh1, 104ul);
      deinterleave(anonym->rxbuf, 264ul, 160UL, 13UL, anonym->dh2, 104ul);
      if (verb) {
         if (maxchannels>0UL) {
            osic_WrINT32(m+1UL, 1UL);
            osi_WrStr(":", 2ul);
         }
         if (anonym->id[0U]) osi_WrStr(anonym->id, 9ul);
         else osi_WrStr("DF6", 4ul);
         WrdB(chan[m].adcmax);
         WrQ(chan[m].dfm6.bitlev, chan[m].dfm6.noise);
         Wrtune(chan[m].adcdc, chan[m].adcmax);
      }
      if (hamming(anonym->dh1, 104ul, 13UL, anonym->db1,
                104ul) && hamming(anonym->dh2, 104ul, 13UL, anonym->db2,
                104ul)) {
         if (verb && hamming(anonym->ch, 56ul, 7UL, anonym->cb, 56ul)) {
            osi_WrStr(" ", 2ul);
            for (i = 0UL; i<=6UL; i++) {
               wh(bits2val(anonym->cb, 56ul, i*4UL, 4UL));
            } /* end for */
         }
         /*      WrInt(bits2val(cb, 4, 16),0); */
         /*      WrInt(bits2val(cb, 8, 16),0); */
         /*      WrInt(bits2val(cb, 12, 16),0); */
         /*      IF bits2val(cb, 0, 4)=6 THEN WrInt(bits2val(cb, 4, 20),
                0) END; */
         decodesub(anonym->db1, 104ul, m, 0UL);
         /*      IF verb THEN WrStr(" /") END; */
         decodesub(anonym->db2, 104ul, m, 1UL);
         { /* with */
            struct DFM6 * anonym0 = &chan[m].dfm6;
            tx = anonym0->id[0U] && aprsstr_StrCmp(anonym0->idcheck, 9ul,
                anonym0->id, 9ul);
            for (i = 0UL; i<=9UL; i++) {
               s[i] = 0;
            } /* end for */
            if (tx && anonym0->txok) {
               /* else stop sending if ambigous id */
               for (i = 0UL; i<=6UL; i++) {
                  s[i] = anonym0->id[i];
                /* sonde id or zero string for no tx */
               } /* end for */
            }
            { /* with */
               struct CHAN * anonym1 = &chan[m];
               s[10U] = (char)(anonym1->mycallc/16777216UL);
               s[11U] = (char)(anonym1->mycallc/65536UL&255UL);
               s[12U] = (char)(anonym1->mycallc/256UL&255UL);
               s[13U] = (char)(anonym1->mycallc&255UL);
               if (anonym1->mycallc>0UL) s[14U] = anonym1->myssid;
               else s[14U] = '\020';
            }
            rt = osic_time()+chan[m].dfm6.timediff;
                /* interpolated sonde realtime */
            s[15U] = (char)(rt/16777216UL);
            s[16U] = (char)(rt/65536UL&255UL);
            s[17U] = (char)(rt/256UL&255UL);
            s[18U] = (char)(rt&255UL);
            j = 19UL;
            for (i = 0UL; i<=3UL; i++) {
               s[j] = (char)bits2val(anonym0->cb, 56ul, i*8UL, 8UL);
                /* payload */
               ++j;
            } /* end for */
            for (i = 0UL; i<=6UL; i++) {
               s[j] = (char)bits2val(anonym0->db1, 104ul, i*8UL, 8UL);
                /* payload */
               ++j;
            } /* end for */
            for (i = 0UL; i<=6UL; i++) {
               s[j] = (char)bits2val(anonym0->db2, 104ul, i*8UL, 8UL);
                /* payload */
               ++j;
            } /* end for */
            alludp(chan[m].udptx, j, s, 101ul);
            if ((verb && !tx) && anonym0->id[0U]) {
               osi_WrStrLn("", 1ul);
               osi_WrStr(" changing name ", 16ul);
               osi_WrStr(anonym0->id, 9ul);
               osi_WrStr("<->", 4ul);
               osi_WrStr(anonym0->idcheck, 9ul);
            }
         }
      }
      else if (verb) {
         /*build tx frame */
         osi_WrStr(" parity error", 14ul);
      }
      if (verb) osi_WrStrLn("", 1ul);
   }
} /* end decodeframe6() */


static void demodbyte6(unsigned long m, char d)
{
   /*  WrInt(ORD(d),1); */
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      anonym->synword = anonym->synword*2UL+(unsigned long)d;
      if (anonym->rxp>=264UL) {
         if ((anonym->synword&65535UL)==17871UL) anonym->rxp = 0UL;
         else if ((anonym->synword&65535UL)==47664UL) {
            /* inverse start sequence found */
            anonym->polarity = !anonym->polarity;
            anonym->rxp = 0UL;
         }
      }
      else {
         anonym->rxbuf[anonym->rxp] = d;
         ++anonym->rxp;
         if (anonym->rxp==264UL) decodeframe6(m);
      }
   }
} /* end demodbyte6() */


static void demodbit6(unsigned long m, float u, float u0)
{
   char d;
   float ua;
   struct DFM6 * anonym;
   d = u>=u0;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      if (anonym->lastmanch==u0<0.0f) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd<0L) {
         /*=polarity*/
         demodbyte6(m, d!=anonym->polarity);
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)*0.02f;
      }
   }
/*quality*/
} /* end demodbit6() */


static void demod6(float u, unsigned long m)
{
   char d;
   struct DFM6 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbit6(m, u, anonym->lastu);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         anonym->lastu = u;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod6() */


static void Fsk6(unsigned long m)
{
   float ff;
   long lim;
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      lim = (long)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (unsigned long)((anonym->baudfine&65535L)/4096L),
                16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod6(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk6() */

/*------------------------------ C34 */

static void demodframe34(unsigned long channel)
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
         if (maxchannels>0UL) {
            osic_WrINT32(channel+1UL, 1UL);
            osi_WrStr(":", 2ul);
         }
         osi_WrStr("C34 ", 5ul);
         osi_WrStr(anonym->id, 9ul);
         WrdB(chan[channel].adcmax);
         WrQuali(noiselevel(channel));
         Wrtune(chan[channel].adcdc, chan[channel].adcmax);
         osi_WrStr(" [", 3ul);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[2U], 2UL);
         osi_WrStr(" ", 2ul);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[3U], 2UL);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[4U], 2UL);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[5U], 2UL);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[6U], 2UL);
         osi_WrStr(" ", 2ul);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[7U], 2UL);
         osic_WrHex((unsigned long)(unsigned char)anonym->rxbuf[8U], 2UL);
         osi_WrStr("] ", 3ul);
      }
      if (ok0) {
         /* chksum ok */
         if (anonym->idtime+3600UL<osic_time()) {
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
                  osi_WrStr("pres", 5ul); /* WrFixed(hr, 2, 0); WrStr("hPa");
                */
               }
            }
            break;
         case '\033':
            if (verb) {
               osi_WrStr("hygr", 5ul);
               osic_WrFixed((float)hr, 2L, 0UL);
               osi_WrStr("%", 2ul);
            }
            break;
         case '\003':
            if (hr<99.9 && hr>(-99.9)) {
               if (verb) {
                  osi_WrStr("temp", 5ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("oC", 3ul);
               }
               good = 1;
            }
            break;
         case '\007':
            if (hr<99.9 && hr>(-99.9)) {
               if (verb) {
                  osi_WrStr("dewp", 5ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("oC", 3ul);
               }
               good = 1;
            }
            break;
         case '\024':
            if (verb) {
               osi_WrStr("date", 5ul);
               aprsstr_IntToStr((long)(val%1000000UL+1000000UL), 1UL, s,
                101ul);
               s[0U] = ' ';
               osi_WrStr(s, 101ul);
            }
            good = 1;
            break;
         case '\025':
            if (verb) {
               aprsstr_TimeToStr((val/10000UL)*3600UL+((val%10000UL)/100UL)
                *60UL+val%100UL, s, 101ul);
               osi_WrStr("time ", 6ul);
               osi_WrStr(s, 101ul);
            }
            good = 1;
            break;
         case '\026':
            hr = latlong(val);
            if (hr<89.9 && hr>(-89.9)) {
               if (verb) {
                  osi_WrStr("lati", 5ul);
                  osic_WrFixed((float)hr, 5L, 0UL);
               }
               good = 1;
            }
            break;
         case '\027':
            hr = latlong(val);
            if (hr<180.0 && hr>(-180.0)) {
               if (verb) {
                  osi_WrStr("long", 5ul);
                  osic_WrFixed((float)hr, 5L, 0UL);
               }
               good = 1;
            }
            break;
         case '\030':
            hr = (double)((float)val*0.1f);
            if (hr<50000.0) {
               if (verb) {
                  osi_WrStr("alti", 5ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("m", 2ul);
               }
               good = 1;
            }
            break;
         case '\031':
            hr = (double)((float)val*0.1852f);
                /*1.609*/ /*1.852*/ /* guess knots or miles */
            if (hr<1000.0) {
               if (verb) {
                  osi_WrStr("wind", 5ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("km/h", 5ul);
               }
               good = 1;
            }
            break;
         case '\032':
            hr = (double)((float)val*0.1f);
            if (hr>=0.0 && hr<=360.0) {
               if (verb) {
                  osi_WrStr("wdir", 5ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("deg", 4ul);
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
               osi_WrStr("numb ", 6ul);
               osi_WrStr((char *) &s[4U], 1u/1u);
               osi_WrStr((char *) &s[5U], 1u/1u);
               osi_WrStr((char *) &s[6U], 1u/1u);
               osi_WrStr((char *) &s[7U], 1u/1u);
               osi_WrStr((char *) &s[8U], 1u/1u);
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
               anonym->idtime = osic_time();
            }
            good = 1;
            break;
         default:;
            if (verb) osi_WrStr("????", 5ul);
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
            osi_WrStr(" changing name ", 16ul);
            osi_WrStr(anonym->id, 9ul);
            osi_WrStr("<->", 4ul);
            osi_WrStr(anonym->idcheck, 9ul);
         }
      }
      else if (verb) {
         /*build tx frame */
         osi_WrStr("---- chksum ", 13ul);
         osic_WrHex(sum1, 2UL);
         osic_WrHex(sum2, 2UL);
      }
      if (verb) osi_WrStrLn("", 1ul);
   }
/* name(9) 0C call(5) playload(7) */
} /* end demodframe34() */


static void demodbit34(unsigned long channel, char d)
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


static void demod34(float u, unsigned long channel)
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


static void Afsk(unsigned long channel)
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
      anonym->baudfine += (long)anonym->demodbaud;
      if (anonym->baudfine>=65536L) {
         anonym->baudfine -= 65536L;
         if (anonym->baudfine<65536L) {
            /* normal alway true */
            ff = Fir(anonym->dfin,
                (unsigned long)(16L-anonym->baudfine/4096L), 16UL,
                anonym->dfir, 64ul, anonym->dfirtab, 1024ul);
            demod34(ff, channel);
         }
      }
   }
} /* end Afsk() */


static void getadc(void)
{
   long sl;
   long l;
   long max0[32];
   long min0[32];
   unsigned long ch;
   unsigned long c;
   unsigned long tmp;
   c = 0UL;
   do {
      if (adcbufrd>=adcbufsamps) {
         adcbufrd = 0UL;
         l = osi_RdBin(soundfd, (char *)adcbuf, 8192u/1u, adcbuflen*2UL);
         adcbufsamps = 0UL;
         if (l<0L) {
            if (abortonsounderr) Error("Sounddevice Failure", 20ul);
            else {
               osic_Close(soundfd);
               Usleep(100000UL);
               OpenSound();
               return;
            }
         }
         if (l<2L) return;
         adcbufsamps = (unsigned long)(l/2L);
         if (debfd>=0L) {
            osi_WrBin(debfd, (char *)adcbuf, 8192u/1u, adcbufsamps*2UL);
         }
         tmp = maxchannels;
         ch = 0UL;
         if (ch<=tmp) for (;; ch++) {
            chan[ch].adcmax = (chan[ch].adcmax*15L)/16L;
            max0[ch] = -32768L;
            min0[ch] = 32767L;
            chan[ch].adcdc += ((max0[ch]+min0[ch])/2L-chan[ch].adcdc)/16L;
            chan[ch].adcmax = max0[ch]-min0[ch];
            if (ch==tmp) break;
         } /* end for */
         adcbufsampx = X2C_max_longcard;
      }
      sl = (long)adcbuf[adcbufrd];
      if (cfgchannels==0UL && (sl&1)) {
         /* auto channels channel 0 */
         /*WrInt(lastc, 1); WrStrLn(" ch1"); */
         if (adcbufsampx!=X2C_max_longcard) {
            ch = (adcbufrd-adcbufsampx)-1UL;
            /*WrInt(ch, 1); WrStrLn(" ch"); */
            if (ch<31UL) {
               if (verb && maxchannels!=ch) {
                  osi_WrStr("channels changed from ", 23ul);
                  osic_WrINT32(maxchannels+1UL, 0UL);
                  osi_WrStr(" to ", 5ul);
                  osic_WrINT32(ch+1UL, 0UL);
                  osi_WrStrLn("", 1ul);
               }
               maxchannels = ch;
            }
         }
         adcbufsampx = adcbufrd;
         c = 0UL;
      }
      sl -= chan[c].adcdc;
      ++adcbufrd;
      chan[c].afir[afin] = (float)(sl-chan[c].adcdc);
      if (sl>max0[c]) max0[c] = sl;
      if (sl<min0[c]) min0[c] = sl;
      ++c;
   } while (c<=maxchannels);
   afin = afin+1UL&31UL;
   tmp = maxchannels;
   c = 0UL;
   if (c<=tmp) for (;; c++) {
      if (chan[c].r92.enabled || chan[c].r41.enabled) Fsk(c);
      if (chan[c].c34.enabled) Afsk(c);
      if (chan[c].dfm6.enabled) Fsk6(c);
      if (c==tmp) break;
   } /* end for */
} /* end getadc() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(CNAMESTR)!=9) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   /*  Gencrctab; */
   memset((char *)chan,(char)0,sizeof(struct CHAN [32]));
   Parms();
   getst = 0UL;
   afin = 0UL;
   soundbufs = 0UL;
   /*  IF verb THEN WrStrLn("Frame ID       level-L qual level-R qual") END;
                */
   adcbufrd = 0UL;
   adcbufsamps = 0UL;
   adcbufsampx = X2C_max_longcard;
   for (;;) getadc();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
