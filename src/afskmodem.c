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
#define afskmodem_C_
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
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <unistd.h>
#ifndef afskmodemptt_H_
#include "afskmodemptt.h"
#endif
#include <signal.h>

/* (a)fsk -  kiss/axudp stereo soundcard afsk/fsk multimodem by OE5DXL */
/*FROM setsighandler IMPORT SIGTERM, SIGINT, setsignalproc, SIGPROC; */
#define afskmodem_MAXLEN 339

#define afskmodem_MAXMODEMS 8

#define afskmodem_TXBUFLEN 60
/* default kiss buffers */

#define afskmodem_ADCBUFLEN 4096

#define afskmodem_BAUDSAMP 65536

#define afskmodem_PLLSHIFT 4096

#define afskmodem_DEFTIMEOUT 60
/* then unsent frames are discarded */

#define afskmodem_AFIRLEN 32
/*32, 64*/

#define afskmodem_AOVERSAMP 16
/*16*/

#define afskmodem_DFIRLEN 64

#define afskmodem_DOVERSAMP 16

#define afskmodem_TFIRLEN 64
/* 2^ */
/*32/5*/

#define afskmodem_SHIFTBITS 6

#define afskmodem_TFIRFINE 128

#define afskmodem_MAXVOL 25000.0
/* max dac / overswing */

static uint32_t afskmodem_POLYNOM = 0x8408UL;

static uint32_t afskmodem_CRCINIT = 0xFFFFUL;

static uint32_t afskmodem_CRCRESULT = 0x9F0BUL;

#define afskmodem_MINFLEN 9

#define afskmodem_STUFFLEN 5

#define afskmodem_FEND 192

#define afskmodem_FESC 219

#define afskmodem_TFEND 220

#define afskmodem_TFESC 221

struct KISSBUF;

typedef struct KISSBUF * pKISSNEXT;


struct KISSBUF {
   pKISSNEXT next;
   char port;
   uint32_t time0;
   uint32_t len;
   char data[341]; /* +2 byte crc */
};

typedef float TXFIRTAB[128][64];

typedef char FILENAME[1024];

enum MONITOR {afskmodem_off, afskmodem_noinfo, afskmodem_normal,
                afskmodem_passall};


enum CHANNELS {afskmodem_LEFT, afskmodem_RIGHT};


typedef float DFIRTAB[1024];

typedef float DFIR[64];

typedef float AFIRTAB[512];

struct MPAR;


struct MPAR {
   uint8_t ch;
   int32_t baudfine;
   int32_t pllshift;
   float left;
   float tcnt;
   float freq;
   float afskmidfreq;
   float clamp;
   float squelchdcd;
   float noise;
   float afshift;
   float afmid;
   float txvolum;
   float afskhighpass;
   uint8_t monitor;
   float sqmed[2];
   uint32_t dfin;
   DFIRTAB dfirtab;
   DFIR dfir;
   AFIRTAB afirtab;
   char afsk;
   char cbit;
   char oldd;
   char plld;
   char scramb;
   char data1;
   char axudp2;
   char dcdmsgs;
   char haddcd;
   char hadtxdata;
   char haddcdrand;
   char flagbeg;
   uint32_t flagc; /* for statistic */
   uint32_t flags;
   uint32_t flage;
   uint32_t rxstuffc;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   uint32_t demodbaud;
   uint32_t txbaud;
   uint32_t port16;
   uint32_t dcdclockm;
   uint32_t bertc;
   uint32_t berterr;
   uint32_t txdel;
   uint32_t txtail;
   uint32_t rxcrc;
   uint32_t scrambler;
   char rxbuf[339];
   pKISSNEXT txbufin;
   pKISSNEXT txbufout;
   uint32_t timeout;
   uint32_t configbaud;
   uint32_t configtxdel;
   uint32_t configtxtail;
   uint32_t txdelpattern;
   uint32_t configafskshift;
   uint32_t configafskmid;
   uint32_t confignyquist;
   int32_t configequalizer;
   int32_t leveldcd;
   int32_t bert;
   char configured;
   char kissignore;
   int32_t udpsocket;
   uint32_t udpport;
   uint32_t udpbind;
   uint32_t udpip;
   char checkip;
};

enum TXSTATE {afskmodem_receiv, afskmodem_slotwait, afskmodem_sendtxdel,
                afskmodem_senddata, afskmodem_sendtxtail};


enum DUPLEX {afskmodem_simplex, afskmodem_shiftdigi, afskmodem_fullduplex,
                afskmodem_onetx};


struct CHAN;


struct CHAN {
   uint32_t gmqtime;
   uint32_t gmcnt; /* gm900 quiet tx time after ptt */
   uint32_t dcdclock;
   uint32_t pttsoundbufs;
   uint32_t dds;
   uint32_t txbaudgen;
   uint32_t addrandom;
   uint32_t tbyte;
   uint32_t tbytec;
   int32_t txstuffc;
   int32_t tbitc;
   char * hptt;
   char pttstate;
   char tnrzi;
   char echoflags;
   uint8_t state;
   uint8_t duplex;
   float persist;
   float hipasscap;
   uint32_t tscramb;
   int32_t adcmax;
   int32_t actmodem;
   uint32_t configpersist;
   float afir[32];
};

static int32_t pipefd;

static int32_t soundfd;

static int32_t debfd;

static char esc;

static char abortonsounderr;

static char badsounddriver;

static char deb01;

static uint32_t debp;

static uint32_t getst;

static uint32_t afin;

static uint32_t soundbufs;

static uint32_t maxsoundbufs;

static uint32_t adcrate;

static uint32_t adcbuflen;

static uint32_t adcbytes;

static uint32_t fragmentsize;

static uint32_t extraaudiodelay;

static uint32_t clock0;

static uint32_t systime;

static uint8_t maxchannels;

static struct CHAN chan[2];

static pKISSNEXT pTxFree;

static pKISSNEXT pGetKiss;

static TXFIRTAB TFIR;

static struct MPAR modpar[8];

static float SIN[32768];

static uint8_t CRCL[256];

static uint8_t CRCH[256];

static FILENAME soundfn;

static char debb[81];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */

/*
PROCEDURE SetStatLine(fd:INTEGER; rts, dtr:BOOLEAN):BOOLEAN;
VAR arg : ARRAY[0..255] OF CARDINAL;
    r:INTEGER;
BEGIN
  arg[0]:=TIOCM_RTS;
  IF rts THEN r:=ioctl(fd, TIOCMBIS, ADR(arg)) ELSE r:=ioctl(fd, TIOCMBIC,
                ADR(arg)) END;
  IF r>=0 THEN
    arg[0]:=TIOCM_DTR;
    IF dtr THEN r:=ioctl(fd, TIOCMBIS, ADR(arg)) ELSE r:=ioctl(fd, TIOCMBIC,
                ADR(arg)) END;
  END;
  RETURN r>=0
END SetStatLine;
*/

static void Hamming(float f[], uint32_t f_len)
{
   uint32_t i;
   uint32_t tmp;
   tmp = f_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      f[i] = f[i]*(0.54f+0.46f*osic_cos(3.1415926535898f*(X2C_DIVR((float)
                i,(float)(1UL+(f_len-1))))));
      if (i==tmp) break;
   } /* end for */
} /* end Hamming() */


static void initdfir(DFIRTAB dfirtab, uint32_t fg)
{
   uint32_t f;
   uint32_t i;
   float t[512];
   float e;
   float f1;
   uint32_t tmp;
   for (i = 0UL; i<=511UL; i++) {
      t[i] = 0.5f;
   } /* end for */
   f1 = X2C_DIVR((float)(fg*64UL),(float)adcrate);
   tmp = (uint32_t)X2C_TRUNCC(f1,0UL,X2C_max_longcard)+1UL;
   f = 1UL;
   if (f<=tmp) for (;; f++) {
      e = 1.0f;
      if (f==(uint32_t)X2C_TRUNCC(f1,0UL,X2C_max_longcard)+1UL) {
         e = f1-(float)(uint32_t)X2C_TRUNCC(f1,0UL,X2C_max_longcard);
      }
      for (i = 0UL; i<=511UL; i++) {
         t[i] = t[i]+e*osic_cos(X2C_DIVR(3.1415926535898f*(float)(i*f),
                512.0f));
      } /* end for */
      if (f==tmp) break;
   } /* end for */
   /*
     FOR f:=1 TO fg*DFIRLEN DIV adcrate DO
       FOR i:=0 TO HIGH(t) DO t[i]:=t[i]+cos(pi*FLOAT(i*f)/FLOAT(HIGH(t)+1))
                END;
     END;
   */
   Hamming(t, 512ul);
   /*  FOR i:=0 TO HIGH(t) DO t[i]:=t[i]*(0.54+0.46*cos(pi*(FLOAT(i)
                /FLOAT(1+HIGH(t))))) END; */
   for (i = 0UL; i<=511UL; i++) {
      dfirtab[511UL+i] = t[i];
      dfirtab[511UL-i] = t[i];
   } /* end for */
/*
IO.WrLn;
FOR i:=0 TO HIGH(dfirtab) DO IO.WrFixed(dfirtab[i], 2,8) END;
IO.WrLn;
*/
/*
fd:=FIO.Create("/tmp/td.raw");
FOR i:=0 TO HIGH(dfirtab) DO n:=VAL(INTEGER, dfirtab[i]*2000.0);
                FIO.WrBin(fd,n,2) END;
FIO.Close(fd);
*/
} /* end initdfir() */


static void initafir(AFIRTAB atab, uint32_t F0, uint32_t F1,
                float eq)
{
   uint32_t f;
   uint32_t i;
   float t[256];
   float f10;
   float f00;
   float e;
   uint32_t tmp;
   f00 = X2C_DIVR((float)(F0*32UL),(float)adcrate);
   f10 = X2C_DIVR((float)(F1*32UL),(float)adcrate);
   for (i = 0UL; i<=255UL; i++) {
      t[i] = 0.0f;
   } /* end for */
   tmp = (uint32_t)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL;
   f = (uint32_t)X2C_TRUNCC(f00,0UL,X2C_max_longcard);
   if (f<=tmp) for (;; f++) {
      e = 1.0f+eq*((X2C_DIVR((float)f,X2C_DIVR((float)((F0+F1)*32UL),
                (float)adcrate)))*2.0f-1.0f);
      /*
          e:=1.0 + eq*(FLOAT(f)/FLOAT((F0+F1)*AFIRLEN DIV adcrate)*2.0-1.0);
      */
      if (e<0.0f) e = 0.0f;
      if (f==(uint32_t)X2C_TRUNCC(f00,0UL,X2C_max_longcard)) {
         e = e*(1.0f-(f00-(float)(uint32_t)X2C_TRUNCC(f00,0UL,
                X2C_max_longcard)));
      }
      if (f==(uint32_t)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL) {
         e = e*(f10-(float)(uint32_t)X2C_TRUNCC(f10,0UL,
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


static void initTFIR(void)
{
   uint32_t b;
   uint32_t f;
   uint32_t j;
   uint32_t i;
   float sym[384];
   float sum[128];
   float s[774];
   float max0;
   float k;
   float fr;
   for (i = 0UL; i<=383UL; i++) {
      sym[i] = 0.5f;
   } /* end for */
   fr = 2.88f;
   for (f = 1UL; f<=3UL; f++) {
      if (f==(uint32_t)X2C_TRUNCC(fr,0UL,X2C_max_longcard)+1UL) {
         k = fr-(float)(uint32_t)X2C_TRUNCC(fr,0UL,X2C_max_longcard);
      }
      else k = 1.0f;
      for (i = 0UL; i<=383UL; i++) {
         sym[i] = sym[i]+k*osic_cos(X2C_DIVR(3.1415926535898f*(float)(i*f)
                ,384.0f));
      } /* end for */
   } /* end for */
   Hamming(sym, 384ul);
   for (i = 0UL; i<=773UL; i++) {
      s[i] = 0.0f;
   } /* end for */
   max0 = 0.0f;
   for (i = 0UL; i<=383UL; i++) {
      s[383UL+i+3UL] = sym[i];
      s[(383UL-i)+3UL] = sym[i];
      if ((float)fabs(sym[i])>max0) max0 = (float)fabs(sym[i]);
   } /* end for */
   max0 = X2C_DIVR(2.0f,max0);
   for (j = 0UL; j<=63UL; j++) {
      for (i = 0UL; i<=127UL; i++) {
         sum[i] = 0.0f;
      } /* end for */
      for (b = 0UL; b<=5UL; b++) {
         for (i = 0UL; i<=127UL; i++) {
            sum[i] = sum[i]+s[i+b*128UL]*((float)(uint32_t)X2C_IN(b,32,
                (uint32_t)j)-0.5f);
         } /* end for */
      } /* end for */
      for (i = 0UL; i<=127UL; i++) {
         TFIR[i][j] = sum[i]*max0;
      } /* end for */
   } /* end for */
} /* end initTFIR() */


static void SetMixer(char mixfn[], uint32_t mixfn_len,
                uint32_t chan0, uint32_t left, uint32_t right)
{
   int32_t fd;
   X2C_PCOPY((void **)&mixfn,mixfn_len);
   fd = osi_OpenRW(mixfn, mixfn_len);
   if (fd>=0L) {
      if (chan0==255UL) chan0 = recnum();
      setmixer(fd, chan0, (right<<8)+left);
   }
   else {
      osi_WrStr(mixfn, mixfn_len);
      Error(" open", 6ul);
   }
   X2C_PFREE(mixfn);
} /* end SetMixer() */


static void OpenSound(void)
{
   int32_t s;
   int32_t i;
   soundfd = osi_OpenRW(soundfn, 1024ul);
   if (soundfd>=0L) {
      i = samplesize(soundfd, 16UL); /* 8, 16 */
      i = channels(soundfd, (uint32_t)maxchannels+1UL); /* 1, 2  */
      i = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
      if (i) {
         osi_WrStr("sound setfragment returns ", 27ul);
         osic_WrINT32((uint32_t)i, 1UL);
         osic_WrLn();
      }
      i = sampelrate(soundfd, adcrate); /* 8000..48000 */
      s = (int32_t)getsampelrate(soundfd);
      if (s!=(int32_t)adcrate) {
         osi_WrStr("sound device returns ", 22ul);
         osic_WrINT32((uint32_t)s, 1UL);
         osi_WrStrLn("Hz!", 4ul);
      }
   }
   else if (abortonsounderr) {
      /*
            IF s>=0 THEN Error("") END;
      */
      osi_WrStr(soundfn, 1024ul);
      Error(" open", 6ul);
   }
} /* end OpenSound() */


static void ttypar(char fn[], uint32_t fn_len)
{
   struct termios term;
   int32_t fd;
   int32_t res;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenNONBLOCK(fn, fn_len);
   if (fd>=0L) {
      res = tcgetattr(fd, &term);
      /*
          term.c_lflag:=CAST(CARDINAL, CAST(BITSET,
                term.c_lflag) - CAST(BITSET, ECHO));
      */
      term.c_lflag = 0UL;
      res = tcsetattr(fd, 0L, &term);
   }
   /*
       res:=tcsetattr (fd, 0, term);
   */
   osic_Close(fd);
   X2C_PFREE(fn);
} /* end ttypar() */


static int32_t Opentty(char linkname[], uint32_t linkname_len)
{
   int32_t fd;
   char voidok;
   char ptsname[4096];
   int32_t Opentty_ret;
   X2C_PCOPY((void **)&linkname,linkname_len);
   fd = osi_OpenNONBLOCK("/dev/ptmx", 10ul);
   if (fd<0L) Error("/dev/ptmx open", 15ul);
   if (osi_getptsname(fd, (char *)ptsname, 4096UL)) {
      Error("no ttyname", 11ul);
   }
   /*
     IO.WrStr(ptsname); IO.WrLn;
   */
   if (osic_grantpts(fd)) Error("ptty grant", 11ul);
   if (osic_unlockpts(fd)) Error("ptty unlock", 12ul);
   ttypar(ptsname, 4096ul);
   /*make link*/
   osi_Erase(linkname, linkname_len, &voidok);
   if (osi_symblink((char *)ptsname, (char *)linkname)) {
      osi_WrStr("cannot create link <", 21ul);
      osi_WrStr(linkname, linkname_len);
      osi_WrStrLn(">, starting without kiss interface", 35ul);
      osic_Close(fd);
      fd = -1L;
   }
   Opentty_ret = fd;
   X2C_PFREE(linkname);
   return Opentty_ret;
} /* end Opentty() */


static void Makekissbufs(uint32_t n)
{
   pKISSNEXT pt;
   uint32_t i;
   pTxFree = 0;
   pGetKiss = 0;
   i = 0UL;
   for (;;) {
      if (i>=n) break;
      osic_alloc((char * *) &pt, sizeof(struct KISSBUF));
      ++i;
      if (pt==0) break;
      pt->next = pTxFree;
      pTxFree = pt;
   }
} /* end Makekissbufs() */


static void StoBuf(int32_t m, pKISSNEXT p)
{
   struct MPAR * anonym;
   { /* with */
      struct MPAR * anonym = &modpar[m];
      if (anonym->configured && pTxFree) {
         p->next = 0;
         if (anonym->txbufin==0) anonym->txbufin = p;
         else anonym->txbufout->next = p;
         anonym->txbufout = p;
      }
      else {
         p->next = pTxFree; /* discard frame, out of memory */
         pTxFree = p;
      }
   }
} /* end StoBuf() */


static void ExtractWord(char w[], uint32_t w_len, char s[],
                uint32_t s_len)
{
   uint32_t j;
   uint32_t i;
   w[0UL] = 0;
   i = 0UL;
   while ((i<=s_len-1 && s[i]) && s[i]!=':') {
      if (i<=w_len-1) w[i] = s[i];
      ++i;
   }
   if (i<=w_len-1) w[i] = 0;
   j = 0UL;
   if (i<=s_len-1 && s[i]) {
      ++i;
      while (i<=s_len-1 && s[i]) {
         s[j] = s[i];
         ++i;
         ++j;
      }
   }
   s[j] = 0;
} /* end ExtractWord() */


static void Config(void)
{
   int32_t i;
   uint8_t c;
   struct CHAN * anonym;
   struct MPAR * anonym0;
   for (c = afskmodem_LEFT;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         anonym->persist = (float)
                ((anonym->configpersist*adcrate*((uint32_t)maxchannels+1UL)
                )/adcbuflen)*0.001f; /* ms */
      }
      if (c==afskmodem_RIGHT) break;
   } /* end for */
   for (i = 0L; i<=7L; i++) {
      { /* with */
         struct MPAR * anonym0 = &modpar[i];
         anonym0->txbaud = (anonym0->configbaud*65536UL)/adcrate;
         anonym0->demodbaud = anonym0->txbaud*2UL;
         anonym0->txdel = (anonym0->configbaud*anonym0->configtxdel)/8000UL;
         anonym0->txtail = 2UL+2UL*(uint32_t)
                anonym0->scramb+(anonym0->configbaud*anonym0->configtxtail)
                /8000UL;
         anonym0->afskmidfreq = X2C_DIVR((float)
                anonym0->configafskmid*2.0f,(float)adcrate);
         anonym0->afshift = X2C_DIVR(0.5f*(float)
                (anonym0->configafskshift*32768UL),(float)adcrate);
         anonym0->afmid = X2C_DIVR((float)(anonym0->configafskmid*32768UL)
                ,(float)adcrate);
         if (anonym0->afsk) {
            initafir(anonym0->afirtab,
                (anonym0->configafskmid-anonym0->configafskshift/2UL)
                -anonym0->configbaud/4UL,
                anonym0->configafskmid+anonym0->configafskshift/2UL+anonym0->configbaud/4UL,
                 X2C_DIVR((float)anonym0->configequalizer,100.0f));
            initdfir(anonym0->dfirtab,
                (anonym0->configbaud*anonym0->confignyquist)/100UL);
         }
         else {
            initafir(anonym0->afirtab, 0UL,
                (anonym0->configbaud*anonym0->confignyquist)/100UL,
                X2C_DIVR((float)anonym0->configequalizer,100.0f));
         }
         anonym0->baudfine = 0L;
         anonym0->left = 0.0f;
         anonym0->tcnt = 0.0f;
         anonym0->freq = 0.0f;
         anonym0->dfin = 0UL;
         anonym0->cbit = 0;
         anonym0->rxp = 0UL;
         anonym0->rxbitc = 0UL;
         anonym0->rxcrc = 0xFFFFUL;
      }
   } /* end for */
} /* end Config() */

/*
PROCEDURE GetIp(h:ARRAY OF CHAR; VAR ip:IPNUM; VAR dp, lp:UDPPORT;
                VAR fd:INTEGER; VAR check:BOOLEAN):INTEGER;
VAR i, p, n:CARDINAL;
    ok:BOOLEAN;
BEGIN
  p:=0;
  h[HIGH(h)]:=0C;
  ip:=0;
  n:=0;
  FOR i:=0 TO 5 DO
    n:=0;
    ok:=FALSE;
    WHILE (h[p]>="0") & (h[p]<="9") DO
      ok:=TRUE;
      n:=n*10+ORD(h[p])-ORD("0");
      INC(p);
    END;
    IF NOT ok THEN RETURN -1 END;
    IF i<3 THEN
      IF (h[p]<>".") OR (n>255) THEN RETURN -1 END;
      ip:=ip*256+n;
    ELSIF i=3 THEN
      ip:=ip*256+n;
      IF (h[p]<>":")OR (n>255) THEN RETURN -1 END;
    ELSIF i=4 THEN
      check:=h[p]<>"/";
      IF (h[p]<>":") & (h[p]<>"/") OR (n>65535) THEN RETURN -1 END;
      dp:=n;
    ELSIF n>65535 THEN RETURN -1 END;
    lp:=n;
    INC(p);
  END;  

  fd:=udp.openudp();
  IF (fd<0) OR (udp.bindudp(fd, lp)<0) (*OR (udp.udpnonblock(fd)<0)*) THEN RETURN -1 END;
  RETURN 0

END GetIp;
*/

static int32_t GetIp(char h[], uint32_t h_len, uint32_t * ip,
                uint32_t * dp, uint32_t * lp, int32_t * fd,
                char * check)
{
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   if (aprsstr_GetIp2(h, h_len, ip, dp, lp, check)<0L) {
      GetIp_ret = -1L;
      goto label;
   }
   *fd = openudp();
   if (*fd<0L || bindudp(*fd, *lp)<0L) {
      /*OR (udp.udpnonblock(fd)<0)*/
      GetIp_ret = -1L;
      goto label;
   }
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static void bertstart(void)
{
   pKISSNEXT p;
   uint32_t i;
   struct MPAR * anonym;
   for (i = 0UL; i<=7UL; i++) {
      { /* with */
         struct MPAR * anonym = &modpar[i];
         if (anonym->bert>0L && pTxFree) {
            p = pTxFree;
            pTxFree = pTxFree->next;
            p->port = (char)i;
            p->len = 0UL;
            p->time0 = X2C_max_longcard;
            StoBuf((int32_t)i, p);
         }
      }
   } /* end for */
} /* end bertstart() */


static void Parms(void)
{
   char err;
   FILENAME pipefn;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   char ptth[4096];
   uint32_t right;
   uint32_t left;
   uint32_t cnum;
   uint32_t kissbufs;
   int32_t inum;
   int32_t modem;
   int32_t channel;
   uint8_t c;
   struct CHAN * anonym;
   struct MPAR * anonym0;
   struct MPAR * anonym1;
   err = 0;
   abortonsounderr = 0;
   adcrate = 16000UL;
   adcbytes = 2UL;
   adcbuflen = 256UL;
   fragmentsize = 9UL;
   maxchannels = afskmodem_LEFT;
   extraaudiodelay = 1UL;
   debfd = -1L;
   deb01 = 0;
   debp = 0UL;
   badsounddriver = 0;
   for (cnum = 0UL; cnum<=32767UL; cnum++) {
      SIN[cnum] = osic_cos(X2C_DIVR((float)cnum*2.0f*3.1415926535898f,
                32768.0f));
   } /* end for */
   for (c = afskmodem_LEFT;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         anonym->state = afskmodem_receiv;
         anonym->pttsoundbufs = 0UL;
         anonym->dds = 0UL;
         anonym->duplex = afskmodem_simplex;
         anonym->persist = 800.0f;
         anonym->actmodem = -1L;
         anonym->hptt = 0;
         anonym->configpersist = 800UL;
         anonym->tscramb = 0x1UL; /* for bert */
         anonym->gmqtime = 0UL;
         anonym->echoflags = 0;
      }
      if (c==afskmodem_RIGHT) break;
   } /* end for */
   for (cnum = 0UL; cnum<=7UL; cnum++) {
      { /* with */
         struct MPAR * anonym0 = &modpar[cnum];
         anonym0->configured = cnum==0UL;
         anonym0->configbaud = 1200UL;
         anonym0->configtxdel = 300UL;
         anonym0->configtxtail = 20UL;
         anonym0->configafskshift = 1000UL;
         anonym0->configafskmid = 1700UL;
         anonym0->confignyquist = 75UL;
         anonym0->configequalizer = 0L;
         anonym0->kissignore = 0;
         anonym0->leveldcd = 10L;
         anonym0->pllshift = 4096L;
         anonym0->ch = afskmodem_LEFT;
         anonym0->afsk = 1;
         anonym0->monitor = afskmodem_normal;
         anonym0->scramb = 0;
         anonym0->clamp = 0.996f;
         anonym0->port16 = cnum*16UL;
         anonym0->squelchdcd = 0.14f;
         anonym0->txbufin = 0;
         anonym0->bert = 0L;
         anonym0->txdelpattern = 126UL;
         anonym0->txvolum = 25000.0f;
         anonym0->afskhighpass = 0.0f;
         anonym0->timeout = 60UL;
         anonym0->udpsocket = -1L;
      }
   } /* end for */
   kissbufs = 60UL;
   channel = -1L;
   modem = -1L;
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   pipefn[0U] = 0;
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   maxsoundbufs = 10UL;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='a') {
            if (modem>=0L) modpar[modem].afsk = 0;
            else if (channel<=0L) abortonsounderr = 1;
            else Error("need modem number -M before -a", 31ul);
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (modem>=0L) {
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               if (cnum>=adcrate) {
                  Error("sampelrate -f too low for this baud", 36ul);
               }
               modpar[modem].configbaud = cnum;
            }
            else {
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               maxsoundbufs = cnum;
            }
         }
         else if (h[1U]=='B') {
            if (modem<0L && channel<0L) badsounddriver = 1;
            else {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
               if (modem>=0L) modpar[modem].bert = inum*8000L;
               else Error("need modem number -M before -B", 31ul);
            }
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>1UL) Error("channel 0 to 1", 15ul);
            channel = (int32_t)cnum;
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) {
               if (cnum>(uint32_t)maxchannels) {
                  Error("channel too high", 17ul);
               }
               modpar[modem].ch = (uint8_t)cnum;
            }
            else {
               if (cnum<1UL || cnum>2UL) Error("maxchannels 1..2", 17ul);
               maxchannels = (uint8_t)(cnum-1UL);
            }
         }
         else if (h[1U]=='D') {
            osi_NextArg(h1, 1024ul);
            debfd = creat(h1, 420L);
            inum = 0L;
            while (inum<1023L && h1[inum]) ++inum;
            deb01 = (((inum>=4L && h1[inum-4L]=='.') && h1[inum-3L]=='t')
                && h1[inum-2L]=='x') && h1[inum-1L]=='t';
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) {
               modpar[modem].squelchdcd = X2C_DIVR((float)cnum,400.0f);
            }
            else Error("need modem number -M before -d", 31ul);
         }
         else if (h[1U]=='e') {
            osi_NextArg(h, 1024ul);
            if (modem>=0L) {
               if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
               if (labs(inum)>999L) Error("equalizer -999..999", 20ul);
               modpar[modem].configequalizer = inum;
            }
            else {
               if (channel>=0L) {
                  Error("need modem number -M before -e", 31ul);
               }
               if (!aprsstr_StrToCard(h, 1024ul, &extraaudiodelay)) err = 1;
            }
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) modpar[modem].configafskmid = cnum;
            else if (channel>=0L) {
               if (cnum>3UL) Error("duplex 0..3", 12ul);
               chan[channel].duplex = (uint8_t)cnum;
            }
            else {
               if (cnum<8000UL || cnum>96000UL) {
                  Error("sampelrate 8000..96000", 23ul);
               }
               adcrate = cnum;
            }
         }
         else if (h[1U]=='g') {
            if (modem>=0L) modpar[modem].scramb = 1;
            else if (channel>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum) || cnum>999UL) {
                  Error("-g <ms>", 8ul);
               }
               chan[channel].gmqtime = (cnum*adcrate)/1000UL;
            }
            else Error("need modem number -M before -g", 31ul);
         }
         else if (h[1U]=='H') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) modpar[modem].afskhighpass = (float)cnum*0.01f;
            else Error("need modem number -M before -H", 31ul);
         }
         else if (h[1U]=='i') {
            if (modem>=0L) modpar[modem].kissignore = 1;
            else osi_NextArg(pipefn, 1024ul);
         }
         else if (h[1U]=='k') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) {
               if (cnum>7UL) Error("kissports 0..7", 15ul);
               modpar[modem].port16 = cnum*16UL;
            }
            else kissbufs = cnum;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>7UL) Error(">maxmodems", 11ul);
            modem = (int32_t)cnum;
            modpar[modem].configured = 1;
         }
         else if (h[1U]=='m') {
            osi_NextArg(h, 1024ul);
            if (modem<0L && channel<0L) {
               while (h[0U]) {
                  ExtractWord(h1, 1024ul, h, 1024ul);
                  if (h1[0U]=='/') X2C_COPY(h1,1024u,mixfn,1024u);
                  else {
                     if (!aprsstr_StrToCard(h1, 1024ul, &cnum)) err = 1;
                     ExtractWord(h1, 1024ul, h, 1024ul);
                     if (!aprsstr_StrToCard(h1, 1024ul, &left)) err = 1;
                     ExtractWord(h1, 1024ul, h, 1024ul);
                     if (!aprsstr_StrToCard(h1, 1024ul, &right)) err = 1;
                     if (!err) SetMixer(mixfn, 1024ul, cnum, left, right);
                  }
               }
            }
            else {
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               if (cnum>3UL) Error("monitor 0..3", 13ul);
               if (modem>=0L) modpar[modem].monitor = (uint8_t)cnum;
               else Error("need modem number -M before -m", 31ul);
            }
         }
         else if (h[1U]=='n') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) modpar[modem].confignyquist = cnum;
            else Error("need modem number -M before -n", 31ul);
         }
         else if (h[1U]=='o') osi_NextArg(soundfn, 1024ul);
         else if (h[1U]=='p') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               modpar[modem].pllshift = (int32_t)(cnum*256UL);
            }
            else if (channel>=0L) {
               osi_NextArg(h1, 1024ul);
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
               inum = labs(inum)+1L;
               if (h[0U]=='-') inum = -inum;
               chan[channel].hptt = pttinit((char *)h1, inum);
            }
         }
         else if (h[1U]=='q') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               modpar[modem].leveldcd = (int32_t)cnum;
            }
            else Error("need modem number -M before -q", 31ul);
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (channel>=0L) {
               if (!aprsstr_StrToCard(h, 1024ul,
                &chan[channel].configpersist)) err = 1;
            }
            else Error("need channel number -C before -r", 33ul);
         }
         else if (h[1U]=='s') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) modpar[modem].configafskshift = cnum;
            else fragmentsize = cnum;
         }
         else if (h[1U]=='t') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               modpar[modem].configtxdel = cnum;
            }
         }
         else if (h[1U]=='u') {
            if (channel>=0L) pttSetclaim(chan[channel].hptt, 1L);
         }
         else if ((h[1U]=='U' || h[1U]=='L') || h[1U]=='P') {
            if (modem>=0L) {
               modpar[modem].axudp2 = h[1U]!='U'; /* switch on axudp2 */
               modpar[modem].dcdmsgs = h[1U]=='P';
                /* send dcd change & txbuffer empty messages*/
               osi_NextArg(h, 1024ul);
               { /* with */
                  struct MPAR * anonym1 = &modpar[modem];
                  if (GetIp(h, 1024ul, &anonym1->udpip, &anonym1->udpport,
                &anonym1->udpbind, &anonym1->udpsocket,
                &anonym1->checkip)<0L) Error("cannot open udp socket", 23ul);
               }
            }
            else Error("need modem number -M before -U", 31ul);
         }
         else if (h[1U]=='v') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               if (cnum>100UL) cnum = 100UL;
               modpar[modem].txvolum = (float)cnum*250.0f;
            }
            else Error("need modem number -M before -w", 31ul);
         }
         else if (h[1U]=='T') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               modpar[modem].timeout = cnum;
            }
            else Error("need modem number -M before -T", 31ul);
         }
         else if (h[1U]=='w') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
               modpar[modem].txdelpattern = cnum;
            }
            else Error("need modem number -M before -w", 31ul);
         }
         else if (h[1U]=='x') {
            if (modem>=0L) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
               modpar[modem].configtxtail = (uint32_t)labs(inum);
               chan[channel].echoflags = inum<0L;
            }
            else Error("need modem number -M before -x", 31ul);
         }
         else if (h[1U]=='z') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (modem>=0L) {
               modpar[modem].clamp = X2C_DIVR((float)cnum,1000.0f);
            }
            else Error("need modem number -M before -z", 31ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("first for all modems", 21ul);
               osi_WrStrLn(" -a             abort on sounddevice error else r\
etry to open", 62ul);
               osi_WrStrLn(" -b <num>       tx dacbuffers (10) (more to avoid\
 underruns)", 61ul);
               osi_WrStrLn(" -B             bad sound driver repair, sending \
continuous quietness to avoid receive", 87ul);
               osi_WrStrLn("                sample loss on start/stop of soun\
d output. Use for stereo or fullduplex)", 89ul);
               osi_WrStrLn(" -c <num>       maxchannels (1) (1=mono, 2=stereo\
)", 51ul);
               osi_WrStrLn(" -D <filename>  (debug) write raw soundcard input\
 data to file or pipe", 71ul);
               osi_WrStrLn(" -D <filename>.txt  (debug) write demodulated bit\
s as \"01..\" to file or pipe", 77ul);
               osi_WrStrLn(" -e <num>       additional ptt hold time (if soun\
dsystem has delay) unit=adcbuffers (1)", 88ul);
               osi_WrStrLn(" -f <num>       adcrate (16000) (8000..96000)",
                46ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -i <filename>  kiss pipename (/dev/kiss/soundmod\
em)", 53ul);
               osi_WrStrLn(" -k <num>       tx kiss bufs (60)", 34ul);
               osi_WrStrLn(" -l <num>       adcbuffer length (256) more: lowe\
r system load but slower reaction", 83ul);
               osi_WrStrLn(" -m [<mixername>:]<channel>:<left>:<right> (0..25\
5) ossmixer (/dev/mixer)", 74ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp)",
                42ul);
               osi_WrStrLn(" -s <num>       fragment size in 2^n (9)", 41ul);
               osic_WrLn();
               osi_WrStrLn("repeat for each channel -C ... :", 33ul);
               osi_WrStrLn("  -C <num>              (0..1) channel parameters\
 follow (repeat for each channel)", 83ul);
               pttHelp((char *)ptth, 4096UL);
               osi_WrStrLn(ptth, 4096ul);
               osi_WrStrLn("  -f <num>              (0) (0=halfduplex, 1=mast\
er fullduplex, 2=all fullduplex,", 82ul);
               osi_WrStrLn("                        3=simplex \'stereo never \
both tx same time\')", 68ul);
               osi_WrStrLn("  -g <ms>               GM900 audio quiet time af\
ter ptt on (0)", 64ul);
               osi_WrStrLn("  -r <num>              max random wait time afte\
r dcd before start tx (ms) (800)", 82ul);
               osic_WrLn();
               osi_WrStrLn("repeat for each modem -M ... :", 31ul);
               osi_WrStrLn("   -M <num> (0..7) modem parameters follow (repea\
t for each modem)", 67ul);
               osi_WrStrLn("   -a       afsk off, fsk on (on)", 34ul);
               osi_WrStrLn("   -b <num> baud (1200) (1..32000)", 35ul);
               osi_WrStrLn("   -B <kbyte> send BERT, (negative bytes receive \
only)", 55ul);
               osi_WrStrLn("   -c <num> use stereo channel 0=left (or mono), \
1=right", 57ul);
               osi_WrStrLn("   -d <num> dcdlevel, 0 no dcd (56) (0..100)",
                45ul);
               osi_WrStrLn("   -e <num> demod equalizer (0) 100=6db/oct highp\
ass   (-999..999)", 67ul);
               osi_WrStrLn("   -f <num> afsk mid frequency, tx and rx (hz) (1\
700)", 54ul);
               osi_WrStrLn("   -g       g3ruh scrambler on (off)", 37ul);
               osi_WrStrLn("   -H <num> afsk tx highpass (0) (0..100)",
                42ul);
               osi_WrStrLn("   -i       ignore modem parameters from kiss (of\
f)", 52ul);
               osi_WrStrLn("   -k <num> received data send to this kiss port \
(0) (0..7)", 60ul);
               /*
                       WrStrLn("#  -l <num> filterlow (hz)");
               */
               osi_WrStrLn("   -m <num> monitor (2) (0=off, 1=header, 2=all, \
3=passall)", 60ul);
               osi_WrStrLn("   -n <num> nyquist filter baseband -6db point in\
 % of baudrate (65) (0..100)", 78ul);
               osi_WrStrLn("   -P same as -U but use AXUDP v2, send dcd and t\
xbuffer empty messages", 72ul);
               osi_WrStrLn("   -p <num> receive clock pll aquisition speed (1\
6) (num/256 of bit time)", 74ul);
               osi_WrStrLn("   -q <num> quiet adc level to save cpu or avoid \
reciption of channel crosstalk (0)", 84ul);
               /*
                       WrStrLn("#  -r       rzi -1 1");
               */
               osi_WrStrLn("   -s <num> afsk shift tx (for rx bandfilter widt\
h) (1000) (hz)", 64ul);
               osi_WrStrLn("   -t <num> txdelay (300) (ms)", 31ul);
               osi_WrStrLn("   -T <seconds> timeout for tx buffered frame (60\
) (s)", 55ul);
               /*
                       WrStrLn("#  -u       filterup  (hz)");
               */
               osi_WrStrLn("   -U <[x.x.x.x]:destport:listenport> use axudp i\
nstead of kiss /listenport check ip", 85ul);
               osi_WrStrLn("   -L same as -U but use AXUDP v2 (no dcd and txb\
uffer empty messages)", 71ul);
               osi_WrStrLn("   -v <num> tx loudness (100)", 30ul);
               osi_WrStrLn("   -w <num> txdelay pattern before 1 flag (126) (\
0..255)", 57ul);
               osi_WrStrLn("   -x <num> txtail (20) (ms), ptt hold for fulldu\
plex", 54ul);
               osi_WrStrLn("   -z <num> fsk rx baseband dc regeneration clamp\
 speed (996) (0=off, 1=fast, 999=slow)", 88ul);
               osi_WrStrLn("example: ./afskmodem -f 22050 -c 2 -C 0 -p /dev/t\
tyS0 0 -M 0 -c 0 -b 1200 -M 1 -c 1 -b 9600 -a -g -U 127.0.0.1:6001:1093",
                121ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else {
         /*
               h[0]:=0C;
         */
         err = 1;
      }
      if (err) break;
   }
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
   Config();
   OpenSound();
   pipefd = -1L;
   if (pipefn[0U]) pipefd = Opentty(pipefn, 1024ul);
   Makekissbufs(kissbufs);
   bertstart();
} /* end Parms() */


static void AppCRC(char frame[], uint32_t frame_len, int32_t size)
{
   uint8_t h;
   uint8_t l;
   uint8_t b;
   int32_t i;
   int32_t tmp;
   l = 0U;
   h = 0U;
   tmp = size-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (uint8_t)((uint8_t)(uint8_t)frame[i]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i==tmp) break;
   } /* end for */
   frame[size] = (char)l;
   frame[size+1L] = (char)h;
} /* end AppCRC() */


static void Kisscmd(void)
{
   uint32_t x;
   uint32_t cmd;
   int32_t modem;
   struct MPAR * anonym;
   cmd = (uint32_t)(uint8_t)pGetKiss->port;
   modem = (int32_t)(cmd>>4&7UL);
   cmd = cmd&15UL;
   osi_WrStr("p=", 3ul);
   osic_WrINT32((uint32_t)modem, 1UL);
   osic_WrLn();
   { /* with */
      struct MPAR * anonym = &modpar[modem];
      if (anonym->configured && !anonym->kissignore) {
         x = (uint32_t)(uint8_t)pGetKiss->data[0U];
         if (cmd==1UL) {
            anonym->configtxdel = x*10UL;
            osi_WrStr("txdel=", 7ul);
            osic_WrINT32(anonym->configtxdel, 1UL);
            osic_WrLn();
         }
         else if (cmd==2UL) {
            chan[modpar[modem].ch].configpersist = 10UL*(255UL-x);
            osi_WrStr("persist=", 9ul);
            osic_WrINT32(chan[modpar[modem].ch].configpersist, 1UL);
            osic_WrLn();
         }
         else if (cmd==4UL) {
            /*3 SlotTime*/
            anonym->configtxtail = x*10UL;
            osi_WrStr("txtail=", 8ul);
            osic_WrINT32(anonym->configtxtail, 1UL);
            osic_WrLn();
         }
         else if (cmd==5UL) {
            if (x<=2UL) chan[modpar[modem].ch].duplex = (uint8_t)x;
            osi_WrStr("duplex=", 8ul);
            osic_WrINT32(x, 1UL);
            osic_WrLn();
         }
         Config();
      }
   }
} /* end Kisscmd() */


static void getkiss(void)
{
   char b[1024];
   int32_t i;
   int32_t l;
   int32_t m;
   int32_t tmp;
   for (;;) {
      if (pipefd<0L) break;
      l = read(pipefd, (char *)b, 1024UL);
      if (l<=0L) break;
      tmp = l-1L;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         /*allocate buffer*/
         if (pGetKiss==0) {
            if (pTxFree==0) goto loop_exit;
            pGetKiss = pTxFree;
            pTxFree = pTxFree->next;
         }
         /*allocate buffer*/
         if (b[i]=='\300') {
            esc = 0;
            if (getst>2UL) {
               pGetKiss->len = getst-2UL;
               getst = 0UL;
               /*store frame*/
               m = (int32_t)((uint32_t)(uint8_t)pGetKiss->port>>4&7UL);
               if (m<=7L) {
                  if (((uint32_t)(uint8_t)pGetKiss->port&15UL)==0UL) {
                     /* data frame */
                     pGetKiss->port = (char)m;
                     AppCRC(pGetKiss->data, 341ul, (int32_t)pGetKiss->len);
                     pGetKiss->time0 = systime+modpar[m].timeout;
                     StoBuf(m, pGetKiss);
                     pGetKiss = 0;
                  }
                  else Kisscmd();
               }
            }
            else {
               /*store frame*/
               getst = 1UL;
            }
         }
         else if (b[i]=='\333' && getst>0UL) esc = 1;
         else {
            if (esc) {
               if (b[i]=='\335') b[i] = '\333';
               else if (b[i]=='\334') b[i] = '\300';
               esc = 0;
            }
            if (getst==1UL) {
               pGetKiss->port = b[i];
               getst = 2UL;
            }
            else if (getst>=2UL && getst-2UL<339UL) {
               pGetKiss->data[getst-2UL] = b[i];
               ++getst;
            }
         }
         if (i==tmp) break;
      } /* end for */
   }
   loop_exit:;
} /* end getkiss() */


static float dB(uint32_t v)
{
   if (v>0UL) return osic_ln((float)v)*8.685889638f-96.5f;
   else return 0.0f;
   return 0;
} /* end dB() */


static float noiselevel(uint32_t m)
/* 0.0 perfect, ~0.25 noise only*/
{
   struct MPAR * anonym;
   { /* with */
      struct MPAR * anonym = &modpar[m];
      if (anonym->sqmed[1]==anonym->sqmed[0]) return 0.0f;
      else return X2C_DIVR(anonym->noise,anonym->sqmed[1]-anonym->sqmed[0]);
   }
   return 0;
} /* end noiselevel() */


static void app(uint32_t * i, uint32_t * p, char b[501], char c,
                int32_t v)
{
   char s[51];
   b[*p] = c;
   ++*p;
   aprsstr_IntToStr(v, 0UL, s, 51ul);
   *i = 0UL;
   while (s[*i]) {
      b[*p] = s[*i];
      ++*p;
      ++*i;
   }
   b[*p] = ' ';
   ++*p;
} /* end app() */


static void sendaxudp2(uint32_t modem, uint32_t datalen,
                char parms, char data[], uint32_t data_len)
{
   char b[501];
   int32_t ret;
   uint32_t ff;
   uint32_t i;
   uint32_t p;
   float q;
   struct MPAR * anonym;
   X2C_PCOPY((void **)&data,data_len);
   { /* with */
      struct MPAR * anonym = &modpar[modem];
      if (anonym->udpsocket>=0L) {
         b[0U] = '\001';
         b[1U] = (char)(48UL+(uint32_t)
                anonym->haddcdrand*2UL+(uint32_t)
                anonym->hadtxdata*4UL+(uint32_t)chan[anonym->ch].pttstate);
         p = 2UL;
         if (parms) {
            app(&i, &p, b, 'D', (int32_t)(uint32_t)(chan[anonym->ch].duplex==afskmodem_fullduplex));
            app(&i, &p, b, 'B', (int32_t)anonym->configbaud);
            app(&i, &p, b, 't', (int32_t)anonym->configtxdel);
         }
         if (datalen>0UL) {
            /* with data */
            ff = (anonym->flags*1000UL)/anonym->configbaud;
            if (ff>0UL) app(&i, &p, b, 'T', (int32_t)ff);
            app(&i, &p, b, 'V', (int32_t)X2C_TRUNCI(dB((uint32_t)chan[anonym->ch].adcmax),
                X2C_min_longint,X2C_max_longint)); /* volume in dB */
            q = noiselevel(modem);
            if (q>0.0f) {
               q = 100.5f-q*200.0f;
               if (q<1.0f) q = 1.0f;
               app(&i, &p, b, 'Q', (int32_t)X2C_TRUNCI(q,X2C_min_longint,
                X2C_max_longint)); /* quality in % */
            }
            b[p] = 0; /* end of axudp2 header */
            ++p;
            i = 0UL;
            do {
               b[p] = data[i];
               ++p;
               ++i;
            } while (i<datalen);
         }
         else {
            b[p] = 0;
            ++p;
         }
         AppCRC(b, 501ul, (int32_t)p);
         ret = udpsend(anonym->udpsocket, b, (int32_t)(p+2UL),
                anonym->udpport, anonym->udpip);
      }
   }
   X2C_PFREE(data);
} /* end sendaxudp2() */


static void getudp(void)
{
   pKISSNEXT p;
   uint32_t i;
   int32_t ulen;
   uint32_t fromport;
   uint32_t fromip;
   char crc2;
   char crc1;
   char udp2[100];
   struct MPAR * anonym;
   for (i = 0UL; i<=7UL; i++) {
      { /* with */
         struct MPAR * anonym = &modpar[i];
         if (anonym->udpsocket>=0L && pTxFree) {
            ulen = udpreceive(anonym->udpsocket, pTxFree->data, 341L,
                &fromport, &fromip);
            if ((ulen>2L && ulen<341L)
                && (!anonym->checkip || fromip==anonym->udpip)) {
               crc1 = pTxFree->data[ulen-2L];
               crc2 = pTxFree->data[ulen-1L];
               AppCRC(pTxFree->data, 341ul, ulen-2L);
               if (crc1==pTxFree->data[ulen-2L]
                && crc2==pTxFree->data[ulen-1L]) {
                  if (pTxFree->data[0U]=='\001') {
                     aprsstr_extrudp2(pTxFree->data, 341ul, udp2, 100ul,
                &ulen);
                     AppCRC(pTxFree->data, 341ul, ulen-2L);
                  }
                  /* axudp2 */
                  if (ulen>2L) {
                     p = pTxFree;
                     pTxFree = pTxFree->next;
                     p->port = (char)i;
                     p->len = (uint32_t)(ulen-2L);
                     p->time0 = systime+anonym->timeout;
                     StoBuf((int32_t)i, p);
                  }
                  else if (udp2[1U]=='?' && udp2[2U]==0) {
                     sendaxudp2(i, 0UL, 1, udp2, 100ul);
                /* on axudp2 header only send dcd & txbuf status */
                  }
               }
            }
         }
      }
   } /* end for */
/* else crc error */
/* else wrong len or source ip */
} /* end getudp() */


static void sendkiss(char data[], uint32_t data_len, int32_t len,
                uint32_t port16)
{
   char b[683];
   int32_t i;
   int32_t l;
   char d;
   uint32_t po;
   struct MPAR * anonym;
   int32_t tmp;
   /*
   FOR i:=0 TO len-1 DO IO.WrHex(ORD(data[i]),3) END; IO.WrLn; 
   */
   po = port16>>4&7UL;
   if (po<=7UL && modpar[po].udpsocket>=0L) {
      { /* with */
         struct MPAR * anonym = &modpar[po];
         if (anonym->axudp2) {
            sendaxudp2(po, (uint32_t)len, 0, data, data_len);
                /* makes new crc */
         }
         else {
            i = udpsend(anonym->udpsocket, data, len+2L, anonym->udpport,
                anonym->udpip);
         }
      }
   }
   else if (pipefd>=0L) {
      b[0U] = '\300';
      b[1U] = (char)port16;
      l = 2L;
      tmp = len-1L;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         d = data[i];
         if (d=='\300') {
            b[l] = '\333';
            ++l;
            b[l] = '\334';
         }
         else if (d=='\333') {
            b[l] = '\333';
            ++l;
            b[l] = '\335';
         }
         else b[l] = d;
         ++l;
         if (i==tmp) break;
      } /* end for */
      b[l] = '\300';
      ++l;
      i = write(pipefd, (char *)b, (uint32_t)l);
   }
} /* end sendkiss() */


static void WrQuali(float q)
{
   if (q>0.0f) {
      q = 100.5f-q*200.0f;
      if (q<0.0f) q = 0.0f;
      osi_WrStr(" q:", 4ul);
      osic_WrINT32((uint32_t)X2C_TRUNCC(q,0UL,X2C_max_longcard), 1UL);
   }
} /* end WrQuali() */


static void WrdB(int32_t volt)
{
   if (volt>0L) {
      osic_WrFixed(dB((uint32_t)volt), 1L, 6UL);
      osi_WrStr("dB", 3ul);
   }
} /* end WrdB() */


static void WrTXD(uint32_t ms)
{
   osi_WrStr(" txd:", 6ul);
   osic_WrINT32(ms, 1UL);
   osi_WrStr("ms", 3ul);
} /* end WrTXD() */


static void WCh(char c)
{
   if (c!='\015') {
      if ((uint8_t)c<' ' || (uint8_t)c>='\177') osi_WrStr(".", 2ul);
      else osi_WrStr((char *) &c, 1u/1u);
   }
} /* end WCh() */


static void ShowCall(char f[], uint32_t f_len, uint32_t pos)
{
   uint32_t e;
   uint32_t i;
   uint32_t tmp;
   e = pos;
   tmp = pos+5UL;
   i = pos;
   if (i<=tmp) for (;; i++) {
      if (f[i]!='@') e = i;
      if (i==tmp) break;
   } /* end for */
   tmp = e;
   i = pos;
   if (i<=tmp) for (;; i++) {
      WCh((char)((uint32_t)(uint8_t)f[i]>>1));
      if (i==tmp) break;
   } /* end for */
   if ((uint32_t)(uint8_t)f[pos+6UL]>>1&15UL) {
      osi_WrStr("-", 2ul);
      osic_WrINT32((uint32_t)(uint8_t)f[pos+6UL]>>1&15UL, 1UL);
   }
} /* end ShowCall() */

static uint32_t afskmodem_UA = 0x63UL;

static uint32_t afskmodem_DM = 0xFUL;

static uint32_t afskmodem_SABM = 0x2FUL;

static uint32_t afskmodem_DISC = 0x43UL;

static uint32_t afskmodem_FRMR = 0x87UL;

static uint32_t afskmodem_UI = 0x3UL;

static uint32_t afskmodem_RR = 0x1UL;

static uint32_t afskmodem_REJ = 0x9UL;

static uint32_t afskmodem_RNR = 0x5UL;


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
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>1&7UL)),&tmp),
                1u/1u);
   }
   else if (cm==0x3UL) osi_WrStr("UI", 3ul);
   else if (cm==0xFUL) osi_WrStr("DM", 3ul);
   else if (cm==0x2FUL) osi_WrStr("SABM", 5ul);
   else if (cm==0x43UL) osi_WrStr("DISC", 5ul);
   else if (cm==0x63UL) osi_WrStr("UA", 3ul);
   else if (cm==0x87UL) osi_WrStr("FRMR", 5ul);
   else osi_WrHex(cmd, 1UL);
   strncpy(PF,"v^-+",4u);
   if (com==0UL || com==3UL) osi_WrStr("v1", 3ul);
   else {
      osi_WrStr((char *) &PF[(com&1UL)+2UL*(uint32_t)
                ((0x10UL & (uint32_t)cmd)!=0)], 1u/1u);
   }
} /* end Showctl() */


static void ShowFrame(char f[], uint32_t f_len, uint32_t len,
                int32_t modem, int32_t volt, char noinfo)
{
   uint32_t ff;
   uint32_t i;
   char d;
   char v;
   char tmp;
   X2C_PCOPY((void **)&f,f_len);
   i = 0UL;
   while (!((uint32_t)(uint8_t)f[i]&1)) {
      ++i;
      if (i>len) goto label;
   }
   /* no address end mark found */
   /*
     IF i=1 THEN
       flexmon(f, len);
       i:=13;
     END;
   */
   if (i%7UL!=6UL) goto label;
   /* address end not modulo 7 error */
   osi_WrStr((char *)(tmp = (char)((modem&7L)+48L),&tmp), 1u/1u);
   osi_WrStr(":fm ", 5ul);
   ShowCall(f, f_len, 7UL);
   osi_WrStr(" to ", 5ul);
   ShowCall(f, f_len, 0UL);
   i = 14UL;
   v = 1;
   while (i+6UL<len && !((uint32_t)(uint8_t)f[i-1UL]&1)) {
      if (v) {
         osi_WrStr(" via", 5ul);
         v = 0;
      }
      osi_WrStr(" ", 2ul);
      ShowCall(f, f_len, i);
      if ((uint32_t)(uint8_t)f[i+6UL]>=128UL && (((uint32_t)(uint8_t)
                f[i+6UL]&1) || (uint32_t)(uint8_t)f[i+13UL]<128UL)) {
         osi_WrStr("*", 2ul);
      }
      i += 7UL;
   }
   /*
     IO.WrStr(" ctl "); IO.WrHex(ORD(f[i]),1);
   */
   Showctl((uint32_t)((0x80U & (uint8_t)(uint8_t)f[6UL])!=0)
                +2UL*(uint32_t)((0x80U & (uint8_t)(uint8_t)f[13UL])!=0)
                , (uint32_t)(uint8_t)f[i]);
   ++i;
   if (i<len) {
      osi_WrStr(" pid ", 6ul);
      osi_WrHex((uint32_t)(uint8_t)f[i], 1UL);
   }
   ++i;
   if (volt>0L) {
      WrQuali(noiselevel((uint32_t)modem));
      WrdB(volt);
      ff = (modpar[modem].flags*1000UL)/modpar[modem].configbaud;
      if (ff>0UL) WrTXD(ff);
   }
   /*
   IO.WrCard(bufree(), 3);
   */
   osic_WrLn();
   if (!noinfo) {
      d = 0;
      while (i<len) {
         if (f[i]!='\015') {
            WCh(f[i]);
            d = 1;
         }
         else if (d) {
            osic_WrLn();
            d = 0;
         }
         ++i;
      }
      if (d) osic_WrLn();
   }
   label:;
   X2C_PFREE(f);
/*
FOR i:=0 TO len-1 DO WrStr("\"); WrInt(ASH(ORD(f[i]), -6),1);
 IO.WrCard(ORD(f[i]) DIV 8 MOD 8,1);IO.WrCard(ORD(f[i]) MOD 8,1);
END;
IO.WrLn;
*/
} /* end ShowFrame() */

#define afskmodem_POLINOM 0x8408 


static void Gencrctab(void)
{
   uint32_t c;
   uint32_t crc;
   uint32_t i;
   for (c = 0UL; c<=255UL; c++) {
      crc = 255UL-c;
      for (i = 0UL; i<=7UL; i++) {
         if ((crc&1)) crc = (uint32_t)((uint32_t)(crc>>1)^0x8408UL);
         else crc = crc>>1;
      } /* end for */
      CRCL[c] = (uint8_t)crc;
      CRCH[c] = (uint8_t)(255UL-(crc>>8));
   } /* end for */
} /* end Gencrctab() */


static float Fir(uint32_t in, uint32_t sub, uint32_t step,
                float fir[], uint32_t fir_len, float firtab[],
                uint32_t firtab_len)
{
   float s;
   uint32_t i;
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


static void demodbit(int32_t m, char d)
{
   char xor;
   struct MPAR * anonym;
   { /* with */
      struct MPAR * anonym = &modpar[m];
      /*descrambler*/
      if (anonym->scramb) {
         /*
               scrambler:=CAST(BITSET, CAST(CARDINAL, scrambler)*2 + ORD(d));
                 
         */
         anonym->scrambler = X2C_LSH(anonym->scrambler,32,1);
         if (d) anonym->scrambler |= 0x1UL;
         d = ((0x1UL & anonym->scrambler)!=0)
                ==(((0x1000UL & anonym->scrambler)!=0)
                ==((0x20000UL & anonym->scrambler)!=0));
                /*result is xor bit 0 12 17*/
      }
      /*nrzi*/
      xor = d;
      d = d==anonym->data1;
      anonym->data1 = xor;
      if (deb01) {
         debb[debp] = (char)(48UL+(uint32_t)d);
         ++debp;
         if (debp>=80UL) {
            debb[80U] = '\012';
            debp = (uint32_t)write(debfd, (char *)debb, debp+1UL);
            debp = 0UL;
         }
      }
      /*bert*/
      if (anonym->bert) {
         if (!d) ++anonym->berterr;
         ++anonym->bertc;
         if (anonym->bert<0L) {
            /* receive only */
            ++anonym->bert;
            if (anonym->bert==0L) osi_WrStrLn("---- end BERT", 14ul);
         }
         if (anonym->bertc>2000UL) {
            osic_WrINT32(anonym->berterr, 4UL);
            osic_WrINT32((uint32_t)m, 2UL);
            WrQuali(noiselevel((uint32_t)m));
            WrdB(chan[anonym->ch].adcmax);
            osic_WrLn();
            anonym->bertc = 0UL;
            anonym->berterr = 0UL;
         }
      }
      if (anonym->rxstuffc<5UL) {
         /*bit to byte*/
         anonym->rxbyte = anonym->rxbyte/2UL+((uint32_t)d<<7);
         ++anonym->rxbitc;
         /*crc*/
         xor = d!=((0x1UL & anonym->rxcrc)!=0); /*databit xor crcbit0*/
         /* 
               rxcrc:=CAST(BITSET, CAST(CARDINAL, rxcrc) DIV 2);
                (*shift right crc register*)
         */
         anonym->rxcrc = X2C_LSH(anonym->rxcrc,32,-1);
         if (xor) anonym->rxcrc = anonym->rxcrc^0x8408UL;
         /*byte to frame*/
         if (anonym->rxbitc>=8UL) {
            if (anonym->rxp<339UL) {
               anonym->rxbuf[anonym->rxp] = (char)anonym->rxbyte;
               ++anonym->rxp;
            }
            /*else frame too long error*/
            anonym->rxbitc = 0UL;
            if (anonym->flagbeg) {
               /* start of data */
               if (anonym->flage>2UL) anonym->flagc = 0UL;
               anonym->flags = anonym->flagc;
               anonym->flagbeg = 0;
            }
            else if (anonym->rxbyte && anonym->flagc>64UL) {
               ++anonym->flage; /* looks like data not txdel patterns */
            }
         }
      }
      else if (anonym->rxstuffc>5UL) {
         /*flag*/
         /*flag*/
         if (((!d && anonym->rxbitc==6UL) && anonym->rxp>=9UL)
                && anonym->rxp<339UL) {
            /*0111111x 0 is flag else abort*/
            /*bits modulo 8 ?*/
            /*frame long enough ?*/
            if (anonym->rxcrc==0x9F0BUL) {
               sendkiss(anonym->rxbuf, 339ul, (int32_t)(anonym->rxp-2UL),
                anonym->port16);
            }
            if (anonym->monitor==afskmodem_passall || anonym->rxcrc==0x9F0BUL && anonym->monitor)
                 {
               ShowFrame(anonym->rxbuf, 339ul, anonym->rxp-2UL, m,
                chan[anonym->ch].adcmax, anonym->monitor==afskmodem_noinfo);
               anonym->flagc = 0UL;
               anonym->flage = 0UL;
            }
         }
         anonym->rxp = 0UL;
         anonym->rxbitc = 0UL;
         anonym->rxcrc = 0xFFFFUL; /*init crc register*/
         anonym->flagbeg = 1;
      }
      if (d) ++anonym->rxstuffc;
      else anonym->rxstuffc = 0UL;
   }
/*destuffing*/
} /* end demodbit() */


static void demod(float u, int32_t m)
{
   char d;
   struct MPAR * anonym;
   d = u>=0.0f;
   { /* with */
      struct MPAR * anonym = &modpar[m];
      if (anonym->cbit) {
         demodbit(m, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         else if (d!=anonym->plld) {
            anonym->flagc = 0UL;
            anonym->flage = 0UL;
            anonym->flagbeg = 0;
         }
         /*squelch*/
         anonym->sqmed[d] = anonym->sqmed[d]+(u-anonym->sqmed[d])*0.05f;
         anonym->noise = anonym->noise+((float)fabs(u-anonym->sqmed[d])
                -anonym->noise)*0.05f;
         /*squelch*/
         ++anonym->flagc;
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod() */


static void Afsk(int32_t m)
{
   float ff;
   float b;
   float a;
   float d;
   float mid;
   float right;
   struct MPAR * anonym;
   { /* with */
      struct MPAR * anonym = &modpar[m];
      right = Fir(afin, 0UL, 16UL, chan[anonym->ch].afir, 32ul,
                anonym->afirtab, 512ul);
      if (anonym->left<0.0f!=right<0.0f) {
         d = X2C_DIVR(anonym->left,anonym->left-right);
         a = (float)(uint32_t)X2C_TRUNCC(d*16.0f+0.5f,0UL,
                X2C_max_longcard);
         b = a*0.0625f;
         if ((uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)>0UL && (uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)<16UL) {
            mid = Fir(afin, 16UL-(uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard), 16UL, chan[anonym->ch].afir, 32ul,
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
      anonym->baudfine += (int32_t)anonym->demodbaud;
      if (anonym->baudfine>=65536L) {
         anonym->baudfine -= 65536L;
         if (anonym->baudfine<65536L) {
            /* normal alway true */
            ff = Fir(anonym->dfin, (uint32_t)(16L-anonym->baudfine/4096L),
                16UL, anonym->dfir, 64ul, anonym->dfirtab, 1024ul);
            demod(ff, m);
         }
      }
   }
} /* end Afsk() */


static void Fsk(int32_t m)
{
   float ff;
   int32_t lim;
   struct MPAR * anonym;
   { /* with */
      struct MPAR * anonym = &modpar[m];
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),
                16UL, chan[anonym->ch].afir, 32ul, anonym->afirtab, 512ul);
            demod(ff-(anonym->left+anonym->freq)*0.5f, m);
            /*clamp*/
            if (!anonym->cbit) {
               if (ff>anonym->left) {
                  anonym->left = anonym->left+(ff-anonym->left)*1.0f;
               }
               if (ff<anonym->freq) {
                  anonym->freq = anonym->freq+(ff-anonym->freq)*1.0f;
               }
               anonym->left = anonym->left*anonym->clamp;
               anonym->freq = anonym->freq*anonym->clamp;
            }
         }
         /*clamp*/
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */

/*
(*pp065*)
PROCEDURE setppout(fd:INTEGER; mask:SET32):BOOLEAN;
CONST PORTCFG=40026206H;

VAR pc:SET32;
    r:CARDINAL;
BEGIN
  r:=100;
  LOOP
    pc:=SET32{30};
    IF ioctl(fd, CAST(INTEGER, PORTCFG), ADR(pc))=0 THEN EXIT END;
    DEC(r);
    IF r=0 THEN pc:=SET32{}; EXIT END;
  END;
  pc:=pc*SET32{0..15}+SET32{31}+mask+SHIFT(mask,1);
  RETURN ioctl(fd, CAST(INTEGER, PORTCFG), ADR(pc))=0;
END setppout;

PROCEDURE ppbitset(fd:INTEGER; port:CARDINAL; on:BOOLEAN);
CONST PARWR=40026207H;
VAR res:INTEGER;
BEGIN
--IO.WrInt(port, 3); IO.WrInt(ORD(on) , 3); IO.WrStrLn("ppout");
  res:=ioctl(fd, CAST(INTEGER, PARWR), CAST(ADDRESS,SET32{port+ORD(NOT on)*8}
                ));
END ppbitset;
(*pp065*)



PROCEDURE Ptt;
VAR lptfd, b:INTEGER;
    map, mask:SET8;
    c:CHANNELS;
    ppmask:SET32;
BEGIN
--WrInt(ORD(chan[LEFT].pttstate),2); WrInt(ORD(chan[RIGHT].pttstate),1);
                WrStrLn("=ptts");
(*tty*)
  IF ttyfn[0]<>0C THEN
    IF ttyfd<0 THEN ttyfd:=open(ttyfn, 0) END;
    IF ttyfd>=0 THEN

      IF NOT SetStatLine(ttyfd, chan[LEFT].pttstate, chan[RIGHT].pttstate)
      OR closetty THEN
        Close(ttyfd);
        ttyfd:=-1;
      END;                                      (* else keep open for alzheimer usb devices *)
    ELSE WrStrLn("tty port open error") END;
  END;
(*tty*)

  IF parporttyp=1 THEN
(*parport*)
    mask:=SET8{};
    map:=SET8{};
    FOR c:=LEFT TO RIGHT DO 
      b:=chan[c].lptpttbit;
      IF b<>0 THEN
        IF chan[c].pttstate=(b>=0) THEN INCL(map, ABS(b)-1) END;
        INCL(mask, ABS(b)-1);
      END;
    END; 
      IF mask<>SET8{} THEN
      lptfd:=OpenRW(lptfn);
      IF (lptfd<0) OR (lpt.ppclaim(lptfd)<0)
                THEN WrStrLn("lpt port open error");
      ELSE 
        map:=(CAST(SET8, lpt.ppdevrdata(lptfd))-mask) + map;
        lpt.ppdevdata(lptfd, CAST(CHAR, map));
      END;
      IF (lptfd>=0) THEN Close(lptfd) END;
    END;
(*parport*)

  ELSIF parporttyp=2 THEN
(*pp065*)
    lptfd:=OpenRW(lptfn);
    IF lptfd>=0 THEN 
      ppmask:=SET32{};
      FOR c:=LEFT TO RIGHT DO
        b:=chan[c].lptpttbit;
        IF b<>0 THEN INCL(ppmask, (ABS(b)-1)*2) END;
      END;
      IF setppout(lptfd, ppmask) THEN
        FOR c:=LEFT TO RIGHT DO
          b:=chan[c].lptpttbit;
          IF b<>0 THEN
            ppbitset(lptfd, ABS(b)-1, chan[c].pttstate=(b>=0));
          END;
        END;
      END;
      Close(lptfd);
    ELSE WrStr(lptfn); WrStrLn(" port open error") END;
  END; 
(*pp065*)


END Ptt;
*/

static void startrandom(uint8_t ch)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[ch];
      anonym->addrandom = 2UL+(uint32_t)X2C_TRUNCC(osic_Random()
                *(double)anonym->persist,0UL,X2C_max_longcard);
                /* store ramdom wait */
      anonym->dcdclock = clock0; /* start txwait after we sent */
   }
} /* end startrandom() */


static char CheckRandom(int32_t modem, uint8_t pttch)
{
   uint32_t clk;
   struct MPAR * anonym;
   struct CHAN * anonym0;
   { /* with */
      struct MPAR * anonym = &modpar[modem];
      { /* with */
         struct CHAN * anonym0 = &chan[anonym->ch];
         if (anonym0->duplex==afskmodem_shiftdigi) {
            clk = modpar[modem].dcdclockm; /* use dcd of this modulation */
         }
         else clk = anonym0->dcdclock;
         return anonym0->duplex!=afskmodem_fullduplex && (clock0-clk<=anonym0->addrandom || anonym0->duplex==afskmodem_onetx && chan[(uint32_t)
                maxchannels-(uint32_t)pttch].pttstate);
      }
   }
/* onetx: tx locks tx of other channel */
} /* end CheckRandom() */

/* (usb) soundcard died */

static void repairsound(void)
{
   /*  pttok:=FALSE; */
   ptt(chan[afskmodem_LEFT].hptt, -1L);
   ptt(chan[afskmodem_RIGHT].hptt, -1L);
   if (abortonsounderr) Error("Sounddevice Failure", 20ul);
   osic_Close(soundfd);
   usleep(100000UL);
   /*WrStrLn("openA"); */
   OpenSound();
} /* end repairsound() */


static void getadc(void)
{
   short buf[4096];
   int32_t minr;
   int32_t maxr;
   int32_t minl;
   int32_t maxl;
   int32_t sl;
   int32_t m;
   int32_t i;
   int32_t l;
   uint8_t c;
   char ndcd;
   struct MPAR * anonym;
   struct MPAR * anonym0;
   l = read(soundfd, (char *)buf, adcbuflen*adcbytes);
   for (m = 0L; m<=7L; m++) {
      modpar[m].noise = modpar[m].noise*0.99f; /* clear dcd on silence */
   } /* end for */
   if (l<=0L) {
      repairsound();
      return;
   }
   if (debfd>=0L && !deb01) {
      i = write(debfd, (char *)buf, (uint32_t)l);
   }
   l = (int32_t)((uint32_t)l/adcbytes);
   for (c = afskmodem_LEFT;; c++) {
      chan[c].adcmax = chan[c].adcmax*15L>>4;
      if (c==afskmodem_RIGHT) break;
   } /* end for */
   maxl = -32768L;
   maxr = -32768L;
   minl = 32767L;
   minr = 32767L;
   i = 0L;
   while (i<l) {
      sl = (int32_t)buf[i];
      chan[afskmodem_LEFT].afir[afin] = (float)sl;
      if (sl>maxl) maxl = sl;
      if (sl<minl) minl = sl;
      if (maxl-minl>chan[afskmodem_LEFT].adcmax) {
         chan[afskmodem_LEFT].adcmax = maxl-minl;
      }
      if (maxchannels>afskmodem_LEFT) {
         sl = (int32_t)buf[i+1L];
         chan[afskmodem_RIGHT].afir[afin] = (float)sl;
         if (sl>maxr) maxr = sl;
         if (sl<minr) minr = sl;
         if (maxr-minr>chan[afskmodem_RIGHT].adcmax) {
            chan[afskmodem_RIGHT].adcmax = maxr-minr;
         }
      }
      afin = afin+1UL&31UL;
      for (m = 0L; m<=7L; m++) {
         { /* with */
            struct MPAR * anonym = &modpar[m];
            if ((anonym->configured && chan[anonym->ch]
                .adcmax>=anonym->leveldcd)
                && (!chan[anonym->ch].pttstate || chan[anonym->ch]
                .duplex && chan[anonym->ch].duplex!=afskmodem_onetx)) {
               /* save cpu and echo reception */
               if (anonym->afsk) Afsk(m);
               else Fsk(m);
               /*
               IF (m=0) & (clock MOD 64=0)
                THEN IO.WrInt(chan[modpar[m].ch].adcmax,6); IO.WrLn; END;
               */
               if (noiselevel((uint32_t)m)<anonym->squelchdcd) {
                  chan[anonym->ch].dcdclock = clock0; /* tx wise dcd */
                  anonym->dcdclockm = clock0;
                /* modem wise dcd for shift digi*/
               }
            }
            else anonym->rxp = 0UL;
         }
      } /* end for */
      i += (int32_t)((uint32_t)maxchannels+1UL);
   }
   for (m = 0L; m<=7L; m++) {
      { /* with */
         struct MPAR * anonym0 = &modpar[m];
         if (anonym0->configured) {
            ndcd = chan[anonym0->ch]
                .adcmax>=anonym0->leveldcd && noiselevel((uint32_t)m)
                <anonym0->squelchdcd;
            if (ndcd) {
               chan[anonym0->ch].dcdclock = clock0; /* tx wise dcd */
               anonym0->dcdclockm = clock0;
                /* modem wise dcd for shift digi*/
            }
            if (ndcd!=anonym0->haddcd) {
               if (ndcd) startrandom(anonym0->ch);
               anonym0->haddcd = ndcd;
            }
            ndcd = CheckRandom(m, anonym0->ch);
            if (ndcd!=anonym0->haddcdrand) {
               anonym0->haddcdrand = ndcd;
               if (anonym0->dcdmsgs) {
                  sendaxudp2((uint32_t)m, 0UL, 0, "", 1ul);
               }
            }
         }
      }
   } /* end for */
} /* end getadc() */


static void txmon(pKISSNEXT pf)
{
   if (pf && modpar[(uint8_t)pf->port].monitor) {
      ShowFrame(pf->data, 341ul, pf->len,
                (int32_t)(uint32_t)(uint8_t)pf->port, 0L,
                modpar[(uint8_t)pf->port].monitor==afskmodem_noinfo);
   }
} /* end txmon() */


static float Phasemod(float * uc, float u, float hp)
{
   float ud;
   ud = u-*uc;
   *uc = *uc+ud*hp;
   return ud;
} /* end Phasemod() */


static void Free(pKISSNEXT * tb)
{
   pKISSNEXT ph;
   if (*tb) {
      ph = (*tb)->next;
      (*tb)->next = pTxFree;
      pTxFree = *tb;
      *tb = ph;
   }
} /* end Free() */


static char frames2tx(int32_t modem)
{
   char txo;
   char tx;
   tx = 0;
   if (modpar[modem].txbufin) {
      if (modpar[modem].txbufin->time0<=systime) {
         Free(&modpar[modem].txbufin); /* frame too old */
      }
      else tx = 1;
   }
   if (modpar[modem].dcdmsgs) {
      /* axudp2 check if txbuffer run empty*/
      txo = modpar[modem].hadtxdata;
      modpar[modem].hadtxdata = tx;
      if (txo && !tx) {
         sendaxudp2((uint32_t)modem, 0UL, 0, "", 1ul);
                /* send tx ready msg */
      }
   }
   return tx;
} /* end frames2tx() */

#define afskmodem_MINENDFLAGS 3
/* txtail between different modulation frames*/

#define afskmodem_MINMIDDLFLAGS 6
/* txtdel between different modulation frames*/


static void sendmodem(void)
{
   short buf[4096];
   int32_t i;
   float samp;
   uint8_t c;
   struct CHAN * anonym;
   struct CHAN * anonym0;
   struct MPAR * anonym1;
   struct CHAN * anonym2;
   uint8_t tmp;
   int32_t tmp0;
   if (soundbufs>0UL) --soundbufs;
   tmp = maxchannels;
   c = afskmodem_LEFT;
   if (c<=tmp) for (;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         if (anonym->pttsoundbufs>0UL) --anonym->pttsoundbufs;
         if (anonym->state==afskmodem_receiv || anonym->state==afskmodem_sendtxtail && anonym->tbytec>3UL)
                 {
            /* minimum flags at end if baud change */
            for (i = 0L; i<=7L; i++) {
               /* has any modem data? */
               /*
                         IF modpar[i].txbufin<>NIL THEN
                           IF c=modpar[i].ch THEN actmodem:=i END;  
                         END;
               */
               if (frames2tx(i) && c==modpar[i].ch) anonym->actmodem = i;
            } /* end for */
            /*
                    IF (actmodem<0) OR (modpar[actmodem].txbufin=NIL)
                THEN     (* no data to send *)
            */
            if (anonym->actmodem<0L || !frames2tx(anonym->actmodem)) {
               /* no data to send */
               /*
               IF soundctl.getoutfilled(soundfd)
                <=0 THEN soundctl.pcmsync(soundfd) END;
               IF soundctl.getoutfilled(soundfd)
                >0 THEN IO.WrInt(soundctl.getoutfilled(soundfd),1); IO.WrLn;
                END;
               */
               if (anonym->pttstate && anonym->pttsoundbufs==0UL) {
                  anonym->pttstate = 0;
                  ptt(anonym->hptt, 0L);
                /* guess all sound buffers are sent*/
                  if (modpar[anonym->actmodem].dcdmsgs) {
                     sendaxudp2((uint32_t)anonym->actmodem, 0UL, 0, "",
                1ul);
                  }
               }
            }
            else if (anonym->pttstate) {
               /* more data, ptt is on*/
               anonym->state = afskmodem_sendtxdel;
               anonym->tbitc = 0L;
               /*          tbytec:=0; */
               if (6UL>modpar[anonym->actmodem].txdel) anonym->tbytec = 6UL;
               else {
                  anonym->tbytec = modpar[anonym->actmodem].txdel-6UL;
                /* start flags after modulation change */
               }
               txmon(modpar[anonym->actmodem].txbufin);
            }
            else {
               /* data ptt off */
               anonym->tbytec = 0UL;
               anonym->state = afskmodem_slotwait;
               startrandom(c);
            }
         }
         if (anonym->state==afskmodem_slotwait) {
            if (!CheckRandom(anonym->actmodem, c)) {
               /*
                       IF duplex=shiftdigi THEN 
                         clk:=modpar[actmodem].dcdclockm;
                (* use dcd of this modulation *)
                       ELSE clk:=dcdclock END;
                (* use dcd of latest heard modulation *)
               
                       IF (duplex=fullduplex) OR (clock-clk > addrandom)
                       & ((duplex<>onetx) OR NOT chan[VAL(CHANNELS,
                ORD(maxchannels)-ORD(c))].pttstate)
                THEN  (* onetx: tx locks tx of other channel *)
               */
               if (frames2tx(anonym->actmodem)) {
                  txmon(modpar[anonym->actmodem].txbufin);
                  chan[c].pttstate = 1; /*WrInt(ORD(c),1); WrStrLn(" ptton");
                */
                  ptt(chan[c].hptt, 1L);
                  if (modpar[anonym->actmodem].dcdmsgs) {
                     sendaxudp2((uint32_t)anonym->actmodem, 0UL, 0, "",
                1ul);
                  }
                  chan[c].gmcnt = chan[c].gmqtime;
                  anonym->pttsoundbufs = soundbufs+extraaudiodelay;
                  anonym->state = afskmodem_sendtxdel;
                  anonym->tbitc = 0L;
                  anonym->tbytec = 0UL;
               }
               else anonym->state = afskmodem_receiv;
            }
         }
      }
      if (c==tmp) break;
   } /* end for */
   /*  WHILE (soundbufs<maxsoundbufs) & ((chan[LEFT].state>=sendtxdel)
                OR (chan[RIGHT].state>=sendtxdel)) DO */
   while (soundbufs<maxsoundbufs && ((badsounddriver || chan[afskmodem_LEFT]
                .state>=afskmodem_sendtxdel)
                || chan[afskmodem_RIGHT].state>=afskmodem_sendtxdel)) {
      tmp0 = (int32_t)(adcbuflen-1UL);
      i = 0L;
      if (i<=tmp0) for (;; i++) {
         if (maxchannels==afskmodem_RIGHT) c = (uint8_t)(i&1L);
         else c = afskmodem_LEFT;
         { /* with */
            struct CHAN * anonym0 = &chan[c];
            if (anonym0->state>=afskmodem_sendtxdel) {
               samp = TFIR[anonym0->txbaudgen/512UL][(uint32_t)
                anonym0->tscramb&63UL];
               if (modpar[anonym0->actmodem].afsk) {
                  anonym0->dds = anonym0->dds+(uint32_t)
                X2C_TRUNCC(samp*modpar[anonym0->actmodem]
                .afshift+modpar[anonym0->actmodem].afmid,0UL,
                X2C_max_longcard)&32767UL;
                  buf[i] = (short)(int32_t)
                X2C_TRUNCI(Phasemod(&anonym0->hipasscap,
                SIN[anonym0->dds]*modpar[anonym0->actmodem].txvolum,
                modpar[anonym0->actmodem].afskhighpass),X2C_min_longint,
                X2C_max_longint);
               }
               else {
                  buf[i] = (short)(int32_t)
                X2C_TRUNCI(samp*modpar[anonym0->actmodem].txvolum,
                X2C_min_longint,X2C_max_longint);
               }
               if (anonym0->gmcnt>0UL) {
                  buf[i] = 0;
                  --anonym0->gmcnt;
               }
               anonym0->txbaudgen = anonym0->txbaudgen+modpar[anonym0->actmodem]
                .txbaud;
               if (anonym0->txbaudgen>=65536UL) {
                  anonym0->txbaudgen -= 65536UL;
                  /*send next bit */
                  if (anonym0->tbitc<=0L) {
                     anonym0->tbitc = 8L;
                     if (anonym0->state==afskmodem_sendtxdel) {
                        if (modpar[anonym0->actmodem].bert>0L) {
                           anonym0->tbyte = 255UL;
                           modpar[anonym0->actmodem].bert -= 8L;
                        }
                        else {
                           anonym0->tbyte = modpar[anonym0->actmodem]
                .txdelpattern;
                           ++anonym0->tbytec;
                        }
                        if (anonym0->tbytec>=modpar[anonym0->actmodem].txdel)
                 anonym0->tbyte = 126UL;
                        if (anonym0->tbytec>modpar[anonym0->actmodem].txdel) {
                           anonym0->state = afskmodem_senddata;
                           anonym0->tbytec = 0UL;
                           anonym0->txstuffc = 0L;
                        }
                     }
                     if (anonym0->state==afskmodem_sendtxtail) {
                        anonym0->tbyte = 126UL;
                        ++anonym0->tbytec;
                        /*next frame*/
                        if (modpar[anonym0->actmodem].txbufin) {
                           /* same modem */
                           anonym0->tbytec = 0UL;
                           anonym0->state = afskmodem_senddata;
                           anonym0->txstuffc = 0L;
                           txmon(modpar[anonym0->actmodem].txbufin);
                        }
                        else if (anonym0->tbytec>modpar[anonym0->actmodem]
                .txtail) {
                           if (anonym0->echoflags) {
                              ptt(anonym0->hptt,
                (int32_t)(uint32_t)modpar[anonym0->actmodem].haddcdrand);
                 /* echo flags if dcd */
                              anonym0->tbytec = modpar[anonym0->actmodem]
                .txtail; /* stay in this mode until new data send */
                           }
                           else {
                              anonym0->state = afskmodem_receiv;
                /* no data for this modem */
                           }
                        }
                     }
                     if (anonym0->state==afskmodem_senddata) {
                        { /* with */
                           struct MPAR * anonym1 = &modpar[anonym0->actmodem]
                ;
                           if (anonym1->txbufin==0 || anonym0->tbytec>=anonym1->txbufin->len+2UL)
                 {
                              /*frame sent*/
                              Free(&anonym1->txbufin);
                              anonym0->state = afskmodem_sendtxtail;
                              anonym0->tbyte = 126UL;
                              anonym0->tbytec = 0UL;
                           }
                           else {
                              anonym0->tbyte = (uint32_t)(uint8_t)
                anonym1->txbufin->data[anonym0->tbytec];
                              ++anonym0->tbytec;
                           }
                        }
                     }
                  }
                  /*stuff*/
                  if ((anonym0->tbyte&1)
                && anonym0->state==afskmodem_senddata) {
                     ++anonym0->txstuffc;
                     if (anonym0->txstuffc>=5L) {
                        anonym0->tbyte += anonym0->tbyte-1UL;
                        ++anonym0->tbitc;
                        anonym0->txstuffc = 0L;
                     }
                  }
                  else anonym0->txstuffc = 0L;
                  /*stuff*/
                  /* nrzi */
                  if (!(anonym0->tbyte&1)) anonym0->tnrzi = !anonym0->tnrzi;
                  /*
                              IF modpar[actmodem].scramb THEN
                                tscramb:=CAST(BITSET, CAST(CARDINAL,
                tscramb)*2+ORD(tnrzi <> ((11 IN tscramb) <> (16 IN tscramb)))
                );
                              ELSE tscramb:=CAST(BITSET, CAST(CARDINAL,
                tscramb)*2+ORD(tnrzi)) END;
                  */
                  anonym0->tscramb = X2C_LSH(anonym0->tscramb,32,1);
                  if (modpar[anonym0->actmodem].scramb) {
                     if (anonym0->tnrzi!=(((0x1000UL & anonym0->tscramb)!=0)
                !=((0x20000UL & anonym0->tscramb)!=0))) {
                        anonym0->tscramb |= 0x1UL;
                     }
                  }
                  else if (anonym0->tnrzi) anonym0->tscramb |= 0x1UL;
                  anonym0->tbyte = anonym0->tbyte>>1;
                  --anonym0->tbitc;
               }
            }
            else buf[i] = 0;
         }
         if (i==tmp0) break;
      } /* end for */
      /*    WrBin(soundfd, buf, adcbuflen*adcbytes); */
      i = write(soundfd, (char *)buf, adcbuflen*adcbytes);
      for (c = afskmodem_LEFT;; c++) {
         { /* with */
            struct CHAN * anonym2 = &chan[c];
            if (anonym2->state>=afskmodem_sendtxdel) ++anonym2->pttsoundbufs;
         }
         if (c==afskmodem_RIGHT) break;
      } /* end for */
      ++soundbufs;
   }
} /* end sendmodem() */

static void afskmodemcleanup(int32_t);


static void afskmodemcleanup(int32_t signum)
{
   pttDestroy(chan[afskmodem_LEFT].hptt);
   pttDestroy(chan[afskmodem_RIGHT].hptt);
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   X2C_HALT((uint32_t)signum);
} /* end afskmodemcleanup() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGTERM, afskmodemcleanup);
   signal(SIGINT, afskmodemcleanup);
   memset((char *)modpar,(char)0,sizeof(struct MPAR [8]));
   Parms();
   Gencrctab();
   initTFIR();
   /*
   tfd:=FIO.Create("/tmp/t.raw");
   */
   getst = 0UL;
   esc = 0;
   afin = 0UL;
   soundbufs = 0UL;
   ptt(chan[afskmodem_LEFT].hptt, -1L);
   ptt(chan[afskmodem_RIGHT].hptt, -1L);
   for (;;) {
      getadc();
      ++clock0;
      if ((clock0&63UL)==0UL) {
         ptt(chan[afskmodem_LEFT].hptt, -1L);
                /* sync ptt to hardware sometime */
         ptt(chan[afskmodem_RIGHT].hptt, -1L);
         systime = osic_time();
      }
      getkiss();
      getudp();
      sendmodem();
   }
/*
IO.WrStr("len="); IO.WrCard(txbuf[txp].len, 1); IO.WrStr("cmd=");
                IO.WrHex(ORD(txbuf[txp].cmd), 1);

FOR i:=1 TO txbuf[txp].len DO IO.WrHex(ORD(txbuf[txp].data[i-1]), 3) END;
IO.WrLn;   
*/
/*
  FIO.Close(pipefd);
*/
/*
FIO.Close(tfd);
*/
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
