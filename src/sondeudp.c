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
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef rsc_H_
#include "rsc.h"
#endif

/* demodulate RS92 sonde (2400bit/s manchester)
   and SRS-C34 (2400Bd AFSK 2000/3800Hz
   and DFM06 (2500bit/s manchester)
   and RS41 (4800Bd GFSK) and send as AXUDP by OE5DXL */
/*FROM fcntl IMPORT creat, open; */
/*FROM flush IMPORT Flush; */
/*IMPORT reedsolomon; */
/* link init_rs_char.o decode_rs_char.o */
/* gcc  -o sondeudp Lib.o aprsstr.o filesize.o flush.o osi.o ptty.o rsc.o sondeudp.o soundctl.o symlink.o tcp.o timec.o udp.o init_rs_char.o decode_rs_char.o /usr/local/xds/lib/x86/libts.a /usr/local/xds/lib/x86/libxds.a  -lm */
#define sondeudp_MAXCHAN 64

#define sondeudp_DAYSEC 86400

#define sondeudp_CONTEXTLIFE 3600
/* seconds till forget context after last heared */

#define sondeudp_ADCBYTES 2

#define sondeudp_MAXLEN 9
/* data frame size c34 */

#define sondeudp_ADCBUFLEN 4096

#define sondeudp_BAUDSAMP 65536

#define sondeudp_PLLSHIFT 1024

#define sondeudp_RAD 1.7453292519943E-2

#define sondeudp_FRAMELIFETIME 3
/* max seconds from frame sync to sending frame */

#define sondeudp_WATERLEN 4

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

#define sondeudp_DFIDTIMEOUT 900
/* s to delete dfm sonde id */

#define sondeudp_FLEN6 264
/* dfm06 frame len */

#define sondeudp_DFMSYN 0x45CF 
/* frame sync sequence */

#define sondeudp_FLENRS41 520
/* rs41  frame len */

#define sondeudp_FLENR92 240

#define sondeudp_FLEN10 101
/* M10 framelen */

#define sondeudp_FLENC34 22
/* c34 frame len */

#define sondeudp_M10SYN 0x649F20 
/* M10 sync */

#define sondeudp_M10SYN1 0x644900 
/*11111000 01100100 01001001 0000xxxx -syn1-  */

/*11111000 01100100 10011111 00100000 -syn-  */
/*
  rs41x 0x86, 0x35, 0xf4, 0x40, 0x93, 0xdf, 0x1a, 0x60
  rs41  0x10, 0xB6, 0xCA, 0x11, 0x22, 0x96, 0x12, 0xF8
*/
#define sondeudp_RHEAD41 "000010000110110101010011100010000100010001101001010\
0100000011111"

static uint8_t sondeudp_EXOR41[64] = {150U,131U,62U,81U,177U,73U,8U,152U,
                50U,5U,89U,14U,249U,68U,198U,38U,33U,96U,194U,234U,121U,93U,
                109U,161U,84U,105U,71U,12U,220U,232U,92U,241U,247U,118U,130U,
                127U,7U,153U,162U,44U,147U,124U,48U,99U,245U,16U,46U,97U,
                208U,188U,180U,182U,6U,170U,244U,35U,120U,110U,59U,174U,191U,
                123U,76U,193U};

typedef char FILENAME[1024];

typedef char CNAMESTR[9];

typedef float AFIRTAB[512];

typedef float DFIRTAB[1024];

typedef float DFIR[64];

struct UDPTX;

typedef struct UDPTX * pUDPTX;


struct UDPTX {
   pUDPTX next;
   uint32_t ip;
   uint32_t destport;
   int32_t udpfd;
};

struct R92;


struct R92 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float noise;
   float bitlev;
   float lastu;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   uint32_t headerrs;
   char rxbuf[301];
   AFIRTAB afirtab;
   int32_t asynst[10];
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct R41;


struct R41 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   float noise0;
   float bitlev0;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char rev;
   char headok;
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[560];
   char fixbytes[560];
   uint8_t fixcnt[560];
   AFIRTAB afirtab;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t synp;
   char synbuf[64];
};

struct DFM6;


struct DFM6 {
   char enabled;
   char frametimeok;
   CNAMESTR id;
   CNAMESTR idcheck;
   uint32_t idcnt;
   uint32_t lastdatesystime;
   uint32_t frametime;
   uint32_t idtime;
   uint32_t timediff;
   uint32_t actdate;
   uint32_t lastdate;
   uint32_t tused;
   uint32_t idnum;
   uint32_t num;
   uint32_t numcnt;
   uint32_t lastnum;
   /* check name */
   uint32_t idcnt0;
   uint32_t idcnt1;
   uint32_t idnew;
   float lastlat;
   float lastlong;
   /* check name */
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float noise;
   float bitlev;
   float lastu;
   char polarity;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   char wasdate;
   char txok;
   char d9;
   uint32_t rxp;
   char rxbuf[264];
   AFIRTAB afirtab;
   uint32_t synword;
   char cb[56];
   char ch[56];
   char db1[104];
   char db2[104];
   char dh1[104];
   char dh2[104];
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct M10;


struct M10 {
   char enabled;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float bitlev;
   float noise;
   float lastu;
   char cbit;
   char oldd;
   char plld;
   char lastmanch;
   char alternativ;
   char txok;
   uint32_t rxb;
   uint32_t rxp;
   char rxbuf[101];
   char fixbytes[101];
   uint8_t fixcnt[101];
   AFIRTAB afirtab;
   uint32_t timefn;
   uint32_t synword1;
   uint32_t synword;
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct SCID;


struct SCID {
   CNAMESTR id;
   CNAMESTR idcheck;
   uint32_t idtime;
   uint32_t idcnt;
};

struct C34;


struct C34 {
   char enabled;
   struct SCID id34;
   struct SCID id50;
   uint32_t idtime;
   uint32_t idcnt;
   int32_t pllshift;
   int32_t baudfine;
   int32_t leveldcd;
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
   char c50;
   uint32_t rxbyte;
   uint32_t rxbytec;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[9];
   AFIRTAB afirtab;
   int32_t asynst[10];
   DFIRTAB dfirtab;
   DFIR dfir;
   uint32_t dfin;
   uint32_t confignyquist;
   uint32_t configafskshift;
   uint32_t configafskmid;
   uint32_t demodbaud;
   uint32_t configbaud;
   uint32_t txbaud;
   float hipasscap;
   uint32_t tused;
};

struct CHAN;

struct _0;


struct _0 {
   char cmd;
   uint32_t dat;
};


struct CHAN {
   int32_t admax;
   int32_t admin;
   float afir[32];
   int32_t configequalizer;
   pUDPTX udptx;
   uint32_t squelch;
   uint32_t mycallc;
   uint32_t framestarttime;
   char myssid;
   uint8_t waterbyte;
   uint32_t waterword;
   uint32_t waterbits;
   struct _0 waterdata[4];
   struct _0 watertemp[4];
   uint32_t watersend;
   char waterok;
   struct R92 r92;
   struct R41 r41;
   struct DFM6 dfm6;
   struct C34 c34;
   struct M10 m10;
};

static int32_t soundfd;

static int32_t debfd;

static char abortonsounderr;

static char waterenabled;

static char nosendsdr;

static char verb;

static char verb2;

static uint32_t dfmidchg;

static uint32_t getst;

static uint32_t afin;

static uint32_t soundbufs;

static uint32_t adcrate;

static uint32_t adcbuflen;

static uint32_t adcbufrd;

static uint32_t adcbufsamps;

static uint32_t fragmentsize;

static FILENAME soundfn;

static struct CHAN chan[64];

static uint32_t adcbufsampx;

static uint32_t maxchannels;

static uint32_t cfgchannels;

static short adcbuf[4096];

static uint32_t dfmnametyp;

static char rxlabel[5];

static uint16_t CRCTAB[256];

static uint32_t CRC32TAB[256];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */

static uint32_t sondeudp_POLY = 0x4C11DB7UL;


static void GenCRC32tab(void)
{
   uint32_t j;
   uint32_t i;
   uint32_t crc;
   uint32_t revpoly;
   revpoly = 0UL;
   for (i = 0UL; i<=31UL; i++) {
      if (X2C_IN(i,32,0x4C11DB7UL)) revpoly |= (1UL<<31UL-i);
   } /* end for */
   for (i = 0UL; i<=255UL; i++) {
      crc = i;
      for (j = 0UL; j<=7UL; j++) {
         if ((crc&1)) crc = (uint32_t)((uint32_t)(crc/2UL)^revpoly);
         else crc = crc/2UL;
      } /* end for */
      CRC32TAB[255UL-i] = (uint32_t)crc^0xFF000000UL;
   } /* end for */
} /* end GenCRC32tab() */


static void crc32(uint32_t * crc, uint8_t byte)
{
   *crc = (uint32_t)(X2C_LSH((uint32_t)*crc,32,
                -8)^CRC32TAB[(uint32_t)(((uint32_t)*crc^(uint32_t)byte)
                &0xFFUL)]);
} /* end crc32() */


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
   Hamming(t, 512ul);
   for (i = 0UL; i<=511UL; i++) {
      t[i] = t[i]*(0.54f+0.46f*osic_cos(3.1415926535898f*(X2C_DIVR((float)
                i,512.0f))));
   } /* end for */
   for (i = 0UL; i<=511UL; i++) {
      dfirtab[511UL+i] = t[i];
      dfirtab[511UL-i] = t[i];
   } /* end for */
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


static void OpenSound(void)
{
   int32_t s;
   int32_t i;
   soundfd = osi_OpenRW(soundfn, 1024ul);
   if (soundfd>=0L) {
      if (maxchannels<2UL) {
         i = samplesize(soundfd, 16UL); /* 8, 16 */
         i = channels(soundfd, maxchannels+1UL); /* 1, 2  */
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
   }
   else if (abortonsounderr) {
      osi_WrStr(soundfn, 1024ul);
      Error(" open", 6ul);
   }
} /* end OpenSound() */


static char packcall(char cs[], uint32_t cs_len,
                uint32_t * cc, char * ssid)
{
   uint32_t s;
   uint32_t j;
   uint32_t i;
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
      if ((uint8_t)c>='A' && (uint8_t)c<='Z') {
         *cc += ((uint32_t)(uint8_t)c-65UL)+1UL;
         ++i;
      }
      else if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         *cc += ((uint32_t)(uint8_t)c-48UL)+27UL;
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
      if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         s += (uint32_t)(uint8_t)c-48UL;
         ++i;
         c = cs[i];
         if ((uint8_t)c>='0' && (uint8_t)c<='9') {
            s = (s*10UL+(uint32_t)(uint8_t)c)-48UL;
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


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * ip,
                uint32_t * port)
{
   uint32_t p;
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
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
   uint32_t c;
   uint32_t i;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6 * anonym1;
   struct M10 * anonym2;
   struct C34 * anonym3;
   for (c = 0UL; c<=63UL; c++) {
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
         struct M10 * anonym2 = &chan[c].m10;
         anonym2->configbaud = 9600UL;
         anonym2->demodbaud = (2UL*anonym2->configbaud*65536UL)/adcrate;
         initafir(anonym2->afirtab, 0UL, 5000UL,
                X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym2->baudfine = 0L;
         anonym2->noise = 0.0f;
         anonym2->bitlev = 0.0f;
         anonym2->cbit = 0;
         anonym2->rxp = 101UL; /* out of fram, wait for sync */
         anonym2->manchestd = 0L;
         anonym2->txok = 0;
      }
      { /* with */
         struct C34 * anonym3 = &chan[c].c34;
         anonym3->txbaud = (anonym3->configbaud*65536UL)/adcrate;
         anonym3->demodbaud = anonym3->txbaud*2UL;
         anonym3->afskmidfreq = X2C_DIVR((float)
                anonym3->configafskmid*2.0f,(float)adcrate);
         initafir(anonym3->afirtab,
                (anonym3->configafskmid-anonym3->configafskshift/2UL)
                -anonym3->configbaud/4UL,
                anonym3->configafskmid+anonym3->configafskshift/2UL+anonym3->configbaud/4UL,
                 X2C_DIVR((float)chan[c].configequalizer,100.0f));
         initdfir(anonym3->dfirtab,
                (anonym3->configbaud*anonym3->confignyquist)/100UL);
         anonym3->baudfine = 0L;
         anonym3->left = 0.0f;
         anonym3->tcnt = 0.0f;
         anonym3->freq = 0.0f;
         anonym3->dfin = 0UL;
         anonym3->cbit = 0;
         anonym3->rxp = 0UL;
         anonym3->rxbitc = 0UL;
      }
   } /* end for */
} /* end Config() */


static void Parms(void)
{
   char err;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   uint32_t ch;
   uint32_t cnum;
   int32_t inum;
   uint32_t channel;
   pUDPTX utx;
   char chanset;
   char mycall[11];
   uint32_t myc;
   char mys;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6 * anonym1;
   struct C34 * anonym2;
   struct M10 * anonym3;
   struct CHAN * anonym4;
   /* set only 1 chan */
   struct CHAN * anonym5;
   err = 0;
   abortonsounderr = 0;
   nosendsdr = 0;
   waterenabled = 1;
   adcrate = 22050UL;
   adcbuflen = 1024UL;
   fragmentsize = 11UL;
   maxchannels = 0UL;
   cfgchannels = 1UL; /* fix 1 channel */
   debfd = -1L;
   chanset = 0;
   dfmnametyp = 0UL;
   dfmidchg = 1UL; /* minutes no tx if dfm name change */
   rxlabel[0] = 0;
   for (channel = 0UL; channel<=63UL; channel++) {
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
         anonym1->idcnt0 = 0UL;
         anonym1->idcnt1 = 0UL;
      }
      { /* with */
         struct C34 * anonym2 = &chan[channel].c34;
         anonym2->enabled = 1;
         anonym2->pllshift = 4096L;
         anonym2->confignyquist = 65UL;
         anonym2->afskhighpass = 0.0f;
         anonym2->configbaud = 2400UL;
         anonym2->configafskshift = 1800UL;
         anonym2->configafskmid = 3800UL;
         anonym2->id34.id[0] = 0;
         anonym2->id34.idcheck[0] = 0;
         anonym2->id34.idtime = 0UL;
         anonym2->id34.idcnt = 0UL;
         anonym2->id50.id[0] = 0;
         anonym2->id50.idcheck[0] = 0;
         anonym2->id50.idtime = 0UL;
         anonym2->id50.idcnt = 0UL;
      }
      { /* with */
         struct M10 * anonym3 = &chan[channel].m10;
         anonym3->enabled = 1;
         anonym3->pllshift = 4096L;
      }
      { /* with */
         struct CHAN * anonym4 = &chan[channel];
         anonym4->configequalizer = 0L;
         anonym4->udptx = 0;
         anonym4->mycallc = 0UL;
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
               for (ch = 0UL; ch<=63UL; ch++) {
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
               for (ch = 0UL; ch<=63UL; ch++) {
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
               for (ch = 0UL; ch<=63UL; ch++) {
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
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].dfm6.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='1') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].m10.enabled = 0;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].m10.enabled = 0;
               } /* end for */
            }
         }
         else if (h[1U]=='N') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmnametyp)) err = 1;
            dfmnametyp += 512UL;
         }
         else if (h[1U]=='n') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmnametyp)) err = 1;
            dfmnametyp += 256UL;
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=63UL) Error("maxchannels 0..max", 19ul);
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
         else if (h[1U]=='G') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmidchg)) err = 1;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else {
               Error("sound buffer out of range", 26ul);
            }
         }
         else if (h[1U]=='o') osi_NextArg(soundfn, 1024ul);
         else if (h[1U]=='L') osi_NextArg(rxlabel, 5ul);
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 11ul);
            if (!packcall(mycall, 11ul, &myc, &mys)) {
               Error("-I illegall Callsign + ssid", 28ul);
            }
            if (chanset) {
               { /* with */
                  struct CHAN * anonym5 = &chan[channel];
                  anonym5->mycallc = myc;
                  anonym5->myssid = mys;
               }
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].mycallc = myc;
                  chan[ch].myssid = mys;
               } /* end for */
            }
         }
         else if (h[1U]=='u') {
            osi_NextArg(h, 1024ul);
            osic_alloc((char * *) &utx, sizeof(struct UDPTX));
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
               for (ch = 0UL; ch<=63UL; ch++) {
                  utx->next = chan[ch].udptx;
                  chan[ch].udptx = utx;
               } /* end for */
            }
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='s') nosendsdr = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else {
            /*      ELSIF h[1]="M" THEN m10newID:=TRUE; */
            if (h[1U]=='h') {
               osi_WrStrLn("oss Mono/Stereo up to 64 Channel RS92, RS41, C34,\
 C50 Sonde Demodulator to raw Frames", 86ul);
               osi_WrStrLn("sent via UDP to \'sondemod\' decoder, more demodu\
lators may send to same decoder", 79ul);
               osi_WrStrLn("Stereo used for 2 Rx for 2 Sondes or 1 Sonde with\
 Antenna-Diversity", 68ul);
               osi_WrStrLn(" -1             disable M10 decoding (use -C befo\
re to select 1 channel)", 73ul);
               osi_WrStrLn(" -3             disable SRSC34/50 decoding (use -\
C before to select 1 channel)", 79ul);
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
               osi_WrStrLn(" -G <minutes>   no tx if DFMxx Name changes (2)",
                 48ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <call>      mycall + ssid (use -C before to s\
elect 1 channel)", 66ul);
               osi_WrStrLn(" -l <num>       adcbuflen (256)", 32ul);
               osi_WrStrLn(" -L <name>      Label of device sent to sondemod,\
 max 4 char", 61ul);
               /*        WrStrLn("
                -M             make M10 object name out of all (known)
                bits of serial number"); */
               osi_WrStrLn(" -N <num>       0..255 generate DFM-ID from seria\
l no. in first frame (see -v)", 79ul);
               osi_WrStrLn("                enter first byte in decimal \"AC0\
0070\" -N 172", 61ul);
               osi_WrStrLn(" -n <num>       same as -N but try old methode to\
o", 51ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or raw/\
wav audio file or pipe /dev/stdin", 83ul);
               osi_WrStrLn(" -s             disable sending sdr-data (freq/af\
c/rssi/label) to (old version) sondemod", 89ul);
               osi_WrStrLn(" -u <x.x.x.x:destport> send rx data in udp (to so\
ndemod), -C <n> before sets", 77ul);
               osi_WrStrLn("                channel number, maybe repeated fo\
r more destinations", 69ul);
               osi_WrStrLn(" -V             very verbous, with some hex dumps\
", 50ul);
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


static void sendudp(char data[], uint32_t data_len, int32_t len,
                uint32_t ip, uint32_t destport, int32_t udpfd)
{
   int32_t i;
   X2C_PCOPY((void **)&data,data_len);
   i = udpsend(udpfd, data, len, destport, ip);
   X2C_PFREE(data);
} /* end sendudp() */


static void appendsdr(uint32_t c)
{
   uint32_t i;
   int32_t w;
   int32_t v;
   struct _0 * anonym;
   if (chan[c].waterok) {
      i = 0UL;
      while (i<=3UL && chan[c].waterdata[i].cmd) {
         { /* with */
            struct _0 * anonym = &chan[c].waterdata[i];
            if (anonym->cmd=='f') {
               osi_WrStr(" ", 2ul);
               osic_WrFixed((float)anonym->dat*0.00001f, 3L, 1UL);
            }
            else if (anonym->cmd=='a') {
               v = (int32_t)(short)((uint32_t)anonym->dat&0xFFFFUL);
               w = (int32_t)(short)X2C_LSH((uint32_t)anonym->dat,32,
                -16);
               if (w) {
                  osi_WrStr("(", 2ul);
                  if (v>=0L) osi_WrStr("+", 2ul);
                  osic_WrINT32((uint32_t)v, 1UL);
                  osi_WrStr("/", 2ul);
                  osic_WrINT32((uint32_t)w, 1UL);
                  osi_WrStr(")", 2ul);
               }
               osi_WrStr(" ", 2ul);
            }
            else if (anonym->cmd=='r') {
               osic_WrFixed((float)anonym->dat*0.1f, 1L, 1UL);
               osi_WrStr("dB", 3ul);
            }
         }
         ++i;
      }
      chan[c].waterok = 0;
   }
} /* end appendsdr() */

#define sondeudp_SDRBLEN 19

#define sondeudp_WATERTIME 2
/* seconds send unupdated waterdata */


static void sdrparm(char b[], uint32_t b_len, uint32_t * len,
                int32_t ch)
{
   uint32_t i;
   uint32_t p;
   uint32_t t;
   struct _0 * anonym;
   /*
   chan[ch].waterok:=TRUE;
   chan[ch].waterdata[0].cmd:="f";
   chan[ch].waterdata[0].dat:=40250000;
   chan[ch].waterdata[1].cmd:="a";
   chan[ch].waterdata[1].dat:=00007FFFEH;
   chan[ch].waterdata[2].cmd:="r";
   chan[ch].waterdata[2].dat:=670;
   */
   if (!nosendsdr) {
      p = *len;
      t = osic_time();
      if ((chan[ch].watersend<=t && chan[ch].watersend+2UL>=t)
                && b_len-1>*len+19UL) {
         i = 0UL;
         while (i<=3UL && chan[ch].waterdata[i].cmd) {
            { /* with */
               struct _0 * anonym = &chan[ch].waterdata[i];
               if ((anonym->cmd=='f' || anonym->cmd=='a')
                || anonym->cmd=='r') {
                  b[p] = anonym->cmd;
                  ++p;
                  b[p] = (char)(anonym->dat/16777216UL);
                  ++p;
                  b[p] = (char)(anonym->dat/65536UL&255UL);
                  ++p;
                  b[p] = (char)(anonym->dat/256UL&255UL);
                  ++p;
                  b[p] = (char)(anonym->dat&255UL);
                  ++p;
               }
            }
            ++i;
         }
      }
      if (rxlabel[0U]) {
         b[p] = 'n';
         ++p;
         b[p] = rxlabel[0U];
         ++p;
         b[p] = rxlabel[1U];
         ++p;
         b[p] = rxlabel[2U];
         ++p;
         b[p] = rxlabel[3U];
         ++p;
      }
      b[p] = (char)( *len/256UL&255UL);
                /* append original sonde data len */
      ++p;
      b[p] = (char)(*len&255UL);
      ++p;
      b[p] = (char)(255UL-( *len/256UL&255UL)); /* safety copy */
      ++p;
      b[p] = (char)(255UL-(*len&255UL));
      ++p;
      *len = p;
   }
} /* end sdrparm() */


static void WrdB(int32_t volt)
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
      osic_WrINT32((uint32_t)(100L-(int32_t)X2C_TRUNCI(noise,
                X2C_min_longint,X2C_max_longint)), 3UL);
      osi_WrStr("%", 2ul);
   }
} /* end WrQ() */


static void WrQuali(float q)
{
   if (q>0.0f) {
      q = 100.5f-q*100.0f;
      if (q<0.0f) q = 0.0f;
      osi_WrStr(" q:", 4ul);
      osic_WrINT32((uint32_t)osi_realint(q), 2UL);
   }
} /* end WrQuali() */


static void Wrtune(int32_t volt, int32_t max0)
{
   int32_t u;
   if (max0>0L && max0>labs(volt)) {
      u = (volt*50L)/max0;
      if (labs(u)>0L) {
         osi_WrStr(" f:", 4ul);
         osic_WrINT32((uint32_t)u, 2UL);
      }
      else osi_WrStr("     ", 6ul);
   }
} /* end Wrtune() */


static void wrtime(uint32_t t)
{
   char s[31];
   aprsstr_TimeToStr(t, s, 31ul);
   osi_WrStr(s, 31ul);
} /* end wrtime() */


static float noiselevel(float bitlev, float noise)
/* 0.0 perfect, ~0.25 noise only*/
{
   if (bitlev==0.0f) return 0.0f;
   else return X2C_DIVR(noise,bitlev);
   return 0;
} /* end noiselevel() */


static void WrChan(int32_t c)
{
   if (maxchannels>0UL) {
      osic_WrINT32((uint32_t)(c+1L), 1UL);
      osi_WrStr(":", 2ul);
   }
} /* end WrChan() */


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


static void alludp(pUDPTX utx, uint32_t len, char buf[],
                uint32_t buf_len)
{
   X2C_PCOPY((void **)&buf,buf_len);
   while (utx) {
      if (utx->udpfd>=0L) {
         sendudp(buf, buf_len, (int32_t)len, utx->ip, utx->destport,
                utx->udpfd);
      }
      utx = utx->next;
   }
   X2C_PFREE(buf);
} /* end alludp() */


static int32_t reedsolomon92(char buf[], uint32_t buf_len)
{
   uint32_t i;
   int32_t res;
   char b[256];
   uint32_t eraspos[24];
   for (i = 0UL; i<=255UL; i++) {
      b[i] = 0;
   } /* end for */
   for (i = 0UL; i<=209UL; i++) {
      b[230UL-i] = buf[i+6UL];
   } /* end for */
   for (i = 0UL; i<=23UL; i++) {
      b[254UL-i] = buf[i+216UL];
   } /* end for */
   res = decodersc(b, eraspos, 0L);
   if (res>0L && res<=12L) {
      for (i = 0UL; i<=209UL; i++) {
         buf[i+6UL] = b[230UL-i];
      } /* end for */
      for (i = 0UL; i<=23UL; i++) {
         buf[i+216UL] = b[254UL-i];
      } /* end for */
   }
   return res;
} /* end reedsolomon92() */


static char crcrs(const char frame[], uint32_t frame_len,
                int32_t from, int32_t to)
{
   uint16_t crc;
   int32_t i;
   int32_t tmp;
   crc = 0xFFFFU;
   tmp = to-3L;
   i = from;
   if (i<=tmp) for (;; i++) {
      crc = X2C_LSH(crc,16,-8)^CRCTAB[(uint32_t)((crc^(uint16_t)(uint8_t)frame[i])&0xFFU)];
      if (i==tmp) break;
   } /* end for */
   return frame[to-1L]==(char)crc && frame[to-2L]==(char)X2C_LSH(crc,
                16,-8);
} /* end crcrs() */

static uint16_t sondeudp_POLYNOM = 0x1021U;


static void decodeframe92(uint32_t m)
{
   uint32_t flen;
   uint32_t len;
   uint32_t p;
   uint32_t j;
   int32_t corr;
   struct CHAN * anonym;
   corr = reedsolomon92(chan[m].r92.rxbuf, 301ul);
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         chan[m].r92.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         chan[m].r92.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         chan[m].r92.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         chan[m].r92.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         chan[m].r92.rxbuf[4U] = anonym->myssid;
      }
      flen = 240UL;
      sdrparm(chan[m].r92.rxbuf, 301ul, &flen, (int32_t)m);
      alludp(anonym->udptx, flen, chan[m].r92.rxbuf, 301ul);
   }
   if (verb) {
      p = 6UL;
      if (chan[m].r92.rxbuf[6U]=='e') {
         ++p;
         len = (uint32_t)(uint8_t)chan[m].r92.rxbuf[7U]*2UL+2UL;
                /* +crc */
         ++p;
         WrChan((int32_t)m);
         osi_WrStr("R92 ", 5ul);
         if (8UL+len>240UL || !crcrs(chan[m].r92.rxbuf, 301ul, 8L,
                (int32_t)(8UL+len))) osi_WrStr("----  crc err ", 15ul);
         else {
            j = 4UL;
            while ((uint8_t)chan[m].r92.rxbuf[8UL+j]>=' ') {
               osi_WrStr((char *) &chan[m].r92.rxbuf[8UL+j], 1u/1u);
               ++j;
            }
            osi_WrStr(" ", 2ul);
            osic_WrINT32((uint32_t)(uint8_t)
                chan[m].r92.rxbuf[8U]+(uint32_t)(uint8_t)
                chan[m].r92.rxbuf[9U]*256UL, 4UL);
         }
         /*      IF m>0 THEN WrStr("             ") END; */
         WrdB(chan[m].admax-chan[m].admin);
         WrQ(chan[m].r92.bitlev, chan[m].r92.noise);
         if (corr<0L) osi_WrStr(" -R", 4ul);
         else if (corr>0L && corr<=12L) {
            osi_WrStr(" +", 3ul);
            osic_WrINT32((uint32_t)corr, 1UL);
            osi_WrStr("R", 2ul);
         }
         Wrtune(chan[m].admax+chan[m].admin, chan[m].admax-chan[m].admin);
         appendsdr(m);
         osi_WrStrLn("", 1ul);
      }
   }
} /* end decodeframe92() */


static double latlong(uint32_t val, char c50)
{
   double hf;
   double hr;
   hr = (double)(val%0x080000000UL);
   if (c50) hr = X2C_DIVL(hr,1.E+7);
   else hr = X2C_DIVL(hr,1.E+6);
   hf = (double)(uint32_t)X2C_TRUNCC(hr,0UL,X2C_max_longcard);
   hr = hf+X2C_DIVL(hr-hf,0.6);
   if (val>=0x080000000UL) hr = -hr;
   return hr;
} /* end latlong() */


static char hex(uint32_t n)
{
   n = n&15UL;
   if (n<10UL) return (char)(n+48UL);
   else return (char)(n+55UL);
   return 0;
} /* end hex() */

/*------------------------------ RS41 */

static void sendrs41(uint32_t m)
{
   uint32_t flen;
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->mycallc>0UL) {
         anonym->r41.rxbuf[0U] = (char)(anonym->mycallc/16777216UL);
         anonym->r41.rxbuf[1U] = (char)(anonym->mycallc/65536UL&255UL);
         anonym->r41.rxbuf[2U] = (char)(anonym->mycallc/256UL&255UL);
         anonym->r41.rxbuf[3U] = (char)(anonym->mycallc&255UL);
         anonym->r41.rxbuf[4U] = anonym->myssid;
         anonym->r41.rxbuf[5U] = 0;
         anonym->r41.rxbuf[6U] = 0;
      }
      flen = 520UL;
      sdrparm(anonym->r41.rxbuf, 560ul, &flen, (int32_t)m);
      alludp(anonym->udptx, flen, anonym->r41.rxbuf, 560ul);
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
      w = (double)(1.5707963267949f-osic_arctan((float)(X2C_DIVL(x,
                y))));
      if (y<0.0) w = w-3.1415926535898;
   }
   else w = 0.0;
   return w;
} /* end atang2() */

#define sondeudp_EARTHA 6.378137E+6

#define sondeudp_EARTHB 6.3567523142452E+6

#define sondeudp_E2 6.6943799901413E-3

#define sondeudp_EARTHAB 4.2841311513312E+4


static void wgs84r(double x, double y, double z,
                double * lat, double * long0,
                double * heig)
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
      t = (double)osic_arctan((float)(X2C_DIVL(z*1.003364089821,
                rh)));
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


static int32_t getint32(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = 3UL;; i--) {
      n = n*256UL+(uint32_t)(uint8_t)frame[p+i];
      if (i==0UL) break;
   } /* end for */
   return (int32_t)n;
} /* end getint32() */


static uint32_t getcard16(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   return (uint32_t)(uint8_t)frame[p]+256UL*(uint32_t)(uint8_t)
                frame[p+1UL];
} /* end getcard16() */


static int32_t getint16(const char frame[], uint32_t frame_len,
                uint32_t p)
{
   uint32_t n;
   n = (uint32_t)(uint8_t)frame[p]+256UL*(uint32_t)(uint8_t)
                frame[p+1UL];
   if (n>=32768UL) return (int32_t)(n-65536UL);
   return (int32_t)n;
} /* end getint16() */


static void posrs41(const char b[], uint32_t b_len, uint32_t p)
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
   if (heig<1.E+5 && heig>(-1.E+5)) {
      osi_WrStr(" ", 2ul);
      osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(heig,X2C_min_longint,
                X2C_max_longint), 1UL);
      osi_WrStr("m", 2ul);
   }
   /*speed */
   vx = (double)getint16(b, b_len, p+12UL)*0.01;
   vy = (double)getint16(b, b_len, p+14UL)*0.01;
   vz = (double)getint16(b, b_len, p+16UL)*0.01;
   vn = (-(vx*(double)osic_sin((float)lat)*(double)
                osic_cos((float)long0))-vy*(double)
                osic_sin((float)lat)*(double)osic_sin((float)
                long0))+vz*(double)osic_cos((float)lat);
   ve = -(vx*(double)osic_sin((float)long0))+vy*(double)
                osic_cos((float)long0);
   vu = vx*(double)osic_cos((float)lat)*(double)
                osic_cos((float)long0)+vy*(double)
                osic_cos((float)lat)*(double)osic_sin((float)
                long0)+vz*(double)osic_sin((float)lat);
   dir = X2C_DIVL(atang2(vn, ve),1.7453292519943E-2);
   if (dir<0.0) dir = 360.0+dir;
   osi_WrStr(" ", 2ul);
   osic_WrFixed(osic_sqrt((float)(vn*vn+ve*ve))*3.6f, 1L, 1UL);
   osi_WrStr("km/h ", 6ul);
   osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(dir,X2C_min_longint,
                X2C_max_longint), 1UL);
   osi_WrStr("deg ", 5ul);
   osic_WrFixed((float)vu, 1L, 1UL);
   osi_WrStr("m/s ", 5ul);
   osic_WrINT32(getcard16(b, b_len, p+18UL)&255UL, 1UL);
   osi_WrStr("Sats", 5ul);
} /* end posrs41() */


static int32_t reedsolomon41(char buf[], uint32_t buf_len,
                uint32_t len2)
{
   uint32_t i;
   int32_t res1;
   /*tb1, */
   int32_t res;
   char b1[256];
   char b[256];
   uint32_t eraspos[24];
   uint32_t tmp;
   for (i = 0UL; i<=255UL; i++) {
      b[i] = 0;
      b1[i] = 0;
   } /* end for */
   tmp = len2;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      b[230UL-i] = buf[i*2UL+56UL];
      b1[230UL-i] = buf[i*2UL+57UL];
      if (i==tmp) break;
   } /* end for */
   for (i = 0UL; i<=23UL; i++) {
      b[254UL-i] = buf[i+8UL];
      b1[254UL-i] = buf[i+32UL];
   } /* end for */
   /*tb1:=b; */
   res = decodersc(b, eraspos, 0L);
   /*FOR i:=0 TO HIGH(b) DO */
   /*IF tb1[i]<>b[i] THEN WrHex(ORD(tb1[i]),4);WrHex(ORD(b[i]),4);
                WrInt(i, 4); WrStr("=pos "); END; */
   /*END; */
   /*tb1:=b1; */
   res1 = decodersc(b1, eraspos, 0L);
   /*FOR i:=0 TO HIGH(b) DO */
   /*IF tb1[i]<>b1[i] THEN WrHex(ORD(tb1[i]),4);WrHex(ORD(b1[i]),4);
                WrInt(i, 4); WrStr("=pos1 "); END; */
   /*END; */
   if (res>0L && res<=12L) {
      tmp = len2;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         buf[i*2UL+56UL] = b[230UL-i];
         if (i==tmp) break;
      } /* end for */
      for (i = 0UL; i<=23UL; i++) {
         buf[i+8UL] = b[254UL-i];
      } /* end for */
   }
   if (res1>0L && res1<=12L) {
      tmp = len2;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         buf[i*2UL+57UL] = b1[230UL-i];
         if (i==tmp) break;
      } /* end for */
      for (i = 0UL; i<=23UL; i++) {
         buf[i+32UL] = b1[254UL-i];
      } /* end for */
   }
   if (res<0L || res1<0L) return -1L;
   else return res+res1;
   return 0;
} /* end reedsolomon41() */

static uint16_t sondeudp_POLYNOM0 = 0x1021U;


static void decode41(uint32_t m)
{
   uint32_t date;
   uint32_t try0;
   uint32_t posok;
   uint32_t nameok;
   uint32_t len;
   uint32_t p;
   uint32_t j;
   uint32_t i;
   char ch;
   char typ;
   char aux;
   char allok;
   int32_t repl;
   int32_t corr;
   char s[24];
   struct R41 * anonym;
   uint32_t tmp;
   { /* with */
      struct R41 * anonym = &chan[m].r41;
      try0 = 0UL;
      do {
         allok = 1;
         nameok = 0UL;
         posok = 0UL;
         corr = 0L;
         repl = 0L;
         date = 0UL;
         if (try0>0UL) {
            if (try0>1UL) {
               for (i = 0UL; i<=559UL; i++) {
                  if (anonym->fixcnt[i]>=10U) {
                     /* replace stable bytes */
                     anonym->rxbuf[i] = anonym->fixbytes[i];
                     ++repl;
                  }
               } /* end for */
            }
            corr = reedsolomon41(anonym->rxbuf, 560ul, 131UL);
                /* try short frame */
            if (corr<0L) {
               corr = reedsolomon41(anonym->rxbuf, 560ul, 230UL);
                /* may be long frame */
            }
         }
         p = 57UL;
         aux = 0;
         for (;;) {
            if (p+4UL>=559UL) break;
            typ = anonym->rxbuf[p];
            ++p;
            len = (uint32_t)(uint8_t)anonym->rxbuf[p]+2UL;
            ++p;
            if (p+len>=559UL) break;
            /*
            WrStrLn("");
            FOR i:=0 TO len+1 DO WrHex(ORD(rxbuf[p+i-2]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;
            WrStrLn("");
            */
            if (!crcrs(anonym->rxbuf, 560ul, (int32_t)p,
                (int32_t)(p+len))) {
               /* crc error */
               allok = 0;
               break;
            }
            if (verb2) {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("", 1ul);
               osi_WrStr("start ID length data... crc [", 30ul);
               osi_WrHex((p-2UL)/256UL, 0UL);
               osi_WrHex(p-2UL, 3UL);
               osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p-2UL], 3UL);
               osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p-1UL], 0UL);
               osi_WrStrLn("]", 2ul);
               tmp = len-1UL;
               i = 0UL;
               if (i<=tmp) for (;; i++) {
                  osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p+i], 3UL);
                  if (i==len-1UL || (i&15UL)==15UL) {
                     for (j = 0UL; j<=15UL; j++) {
                        if ((uint8_t)anonym->rxbuf[((p+i)-16UL)
                +j]>=' ' && (uint8_t)anonym->rxbuf[((p+i)-16UL)+j]<='~') {
                           osi_WrStr((char *) &anonym->rxbuf[((p+i)-16UL)
                +j], 1u/1u);
                        }
                        else osi_WrStr(".", 2ul);
                     } /* end for */
                     osi_WrStrLn("", 1ul);
                  }
                  if (i==tmp) break;
               } /* end for */
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("", 1ul);
            }
            tmp = (p+len)-1UL;
            i = p-2UL;
            if (i<=tmp) for (;; i++) {
               /* update fixbyte statistics */
               if (anonym->fixbytes[i]==anonym->rxbuf[i]) {
                  if (anonym->fixcnt[i]<255U) {
                     ++anonym->fixcnt[i];
                  }
               }
               else {
                  anonym->fixbytes[i] = anonym->rxbuf[i];
                  anonym->fixcnt[i] = 0U;
               }
               if (i==tmp) break;
            } /* end for */
            if (typ=='y') nameok = p;
            else if (typ=='|') {
               /*        ELSIF typ=CHR(7AH) THEN */
               /*             WrStrLn("7A frame"); */
               date = (uint32_t)getint32(anonym->rxbuf, 560ul,
                p+2UL)/1000UL+86382UL+getcard16(anonym->rxbuf, 560ul,
                p)*604800UL+315878400UL;
            }
            else if (typ=='{') {
               /*        WrStrLn("7C frame"); */
               /*WrInt(getint32(rxbuf, p+2) DIV 1000 MOD 86400 , 10);
                WrStr("=gpstime "); */
               /*        ELSIF typ=CHR(7DH) THEN */
               /*             WrStrLn("7D frame"); */
               posok = p;
            }
            else if (typ=='~') aux = 1;
            /*        ELSIF typ=CHR(76H) THEN */
            /*             WrStrLn("76 frame"); */
            /*        ELSE EXIT END; */
            /*        WrInt(getint16(rxbuf, 3BH), 0); */
            /*        WrStr(" ");WrHex(ORD(typ), 0);WrStr(" ");
                WrHex(p DIV 256, 0);WrHex(p, 0); */
            /*        WrStr(" "); */
            if (typ=='v') break;
            p += len;
         }
         ++try0;
      } while (!(allok || try0>2UL));
      if (verb && nameok>0UL) {
         WrChan((int32_t)m);
         osi_WrStr("R41 ", 5ul);
         for (i = 0UL; i<=7UL; i++) {
            ch = anonym->rxbuf[nameok+2UL+i];
            if ((uint8_t)ch>' ' && (uint8_t)ch<'\177') {
               osi_WrStr((char *) &ch, 1u/1u);
            }
         } /* end for */
         osi_WrStr(" ", 2ul);
         osic_WrUINT32(getcard16(anonym->rxbuf, 560ul, nameok), 1UL);
         if (date>0UL) {
            osi_WrStr(" ", 2ul);
            aprsstr_DateToStr(date, s, 24ul);
            osi_WrStr(s, 24ul);
         }
         if (posok>0UL) posrs41(anonym->rxbuf, 560ul, posok);
         if (anonym->rxbuf[nameok+23UL]==0) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)(getcard16(anonym->rxbuf, 560ul,
                nameok+26UL)/64UL+40000UL)*0.01f, 2L, 1UL);
            osi_WrStr("MHz", 4ul);
         }
         if (aux) osi_WrStr(" +Aux", 6ul);
         if (!((allok || posok>0UL) || aux)) {
            osi_WrStr(" ----  crc err ", 16ul);
         }
         WrdB(chan[m].admax-chan[m].admin);
         WrQ(anonym->bitlev0, anonym->noise0);
         /*WrStrLn(""); */
         /*FOR i:=0 TO HIGH(rxbuf) DO WrHex(ORD(rxbuf[i]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;  */
         if (repl>0L) {
            osi_WrStr(" x", 3ul);
            osic_WrINT32((uint32_t)repl, 1UL);
         }
         if (corr<0L) osi_WrStr(" -R", 4ul);
         else if (corr>0L) {
            osi_WrStr(" +", 3ul);
            osic_WrINT32((uint32_t)corr, 1UL);
            osi_WrStr("R", 2ul);
         }
         Wrtune(chan[m].admax+chan[m].admin, chan[m].admax-chan[m].admin);
         appendsdr(m);
         osi_WrStrLn("", 1ul);
      }
   }
   if (nameok>0UL) sendrs41(m);
} /* end decode41() */

#define sondeudp_MAXHEADERR 4

static uint8_t _cnst[64] = {150U,131U,62U,81U,177U,73U,8U,152U,50U,5U,89U,
                14U,249U,68U,198U,38U,33U,96U,194U,234U,121U,93U,109U,161U,
                84U,105U,71U,12U,220U,232U,92U,241U,247U,118U,130U,127U,7U,
                153U,162U,44U,147U,124U,48U,99U,245U,16U,46U,97U,208U,188U,
                180U,182U,6U,170U,244U,35U,120U,110U,59U,174U,191U,123U,76U,
                193U};

static void demodbyte41(uint32_t m, char d)
{
   uint32_t j;
   uint32_t i;
   uint32_t revc;
   uint32_t normc;
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
         normc = 0UL;
         revc = 0UL;
         do {
            --j;
            if (("00001000011011010101001110001000010001000110100101001000000\
11111"[j]=='1')==anonym->synbuf[i]) ++normc;
            else ++revc;
            if (i==0UL) i = 63UL;
            else --i;
         } while (!(j==24UL || normc>4UL && revc>4UL));
         anonym->headok = normc==0UL || revc==0UL;
         anonym->rev = normc<revc;
         if (j==24UL) {
            anonym->rxp = 7UL;
            chan[m].framestarttime = osic_time();
         }
         anonym->rxbitc = 0UL;
      }
      else {
         anonym->rxbyte = anonym->rxbyte/2UL+128UL*(uint32_t)
                (d!=anonym->rev);
         ++anonym->rxbitc;
         if (anonym->rxbitc>=8UL) {
            anonym->rxbuf[anonym->rxp] = (char)((uint8_t)
                anonym->rxbyte^(uint8_t)_cnst[anonym->rxp&63UL]);
            ++anonym->rxp;
            if (anonym->rxp>=559UL) {
               if (chan[m].framestarttime+3UL>=osic_time()) decode41(m);
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


static void demodbit41(uint32_t m, float u)
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

static void stobyte92(uint32_t m, char b)
{
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      anonym->rxbuf[anonym->rxp] = b;
      if (anonym->rxp>=5UL || b=='*') ++anonym->rxp;
      else anonym->rxp = 0UL;
      if (anonym->rxp==5UL) chan[m].framestarttime = osic_time();
      if (anonym->rxp>=240UL) {
         anonym->rxp = 0UL;
         if (chan[m].framestarttime+3UL>=osic_time()) decodeframe92(m);
      }
   }
} /* end stobyte92() */


static void demodbyte92(uint32_t m, char d)
{
   uint32_t maxi;
   uint32_t i;
   int32_t max0;
   int32_t n;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      anonym->rxbyte = anonym->rxbyte/2UL+256UL*(uint32_t)d;
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
            anonym->rxbyte = (uint32_t)((uint32_t)anonym->rxbyte^0xFFUL);
         }
         stobyte92(m, (char)(anonym->rxbyte&255UL));
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
} /* end demodbyte92() */


static void demodbit92(uint32_t m, float u, float u0)
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
         demodbyte92(m, d);
         /*WrInt(VAL(INTEGER, u0*0.001), 4); WrStr("/");
                WrInt(VAL(INTEGER, u*0.001), 0); */
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)
                *0.02f;
      }
   }
/*quality*/
} /* end demodbit92() */


static void demod92(float u, uint32_t m)
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


static void Fsk(uint32_t m)
{
   float ff;
   int32_t lim;
   struct R92 * anonym;
   { /* with */
      struct R92 * anonym = &chan[m].r92;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),
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

static void deinterleave(const char b[], uint32_t b_len,
                uint32_t base, uint32_t len, char db[],
                uint32_t db_len)
{
   uint32_t j;
   uint32_t i;
   uint32_t tmp;
   for (j = 0UL; j<=7UL; j++) {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         db[i*8UL+j] = b[base+len*j+i];
         if (i==tmp) break;
      } /* end for */
   } /* end for */
} /* end deinterleave() */


static char hamcorr(char b[], uint32_t b_len, uint32_t d,
                uint32_t h)
{
   uint32_t e;
   e = (uint32_t)((b[d]==b[d+2UL])!=(b[h]==b[h+2UL]))+2UL*(uint32_t)
                ((b[d+1UL]==b[d+2UL])!=(b[h+1UL]==b[h+2UL]))+4UL*(uint32_t)
                ((b[d+3UL]==b[h])!=(b[h+1UL]==b[h+2UL]));
   /* hamming matrix multiplication */
   if (e>4UL) b[(h+e)-4UL] = !b[(h+e)-4UL];
   else if (e>0UL) b[(d+e)-1UL] = !b[(d+e)-1UL];
   /*  IF e<>0 THEN WrStr("<");WrInt(e, 1);WrStr(">") END; */
   e = (uint32_t)b[d]+(uint32_t)b[d+1UL]+(uint32_t)
                b[d+2UL]+(uint32_t)b[d+3UL]+(uint32_t)b[h]+(uint32_t)
                b[h+1UL]+(uint32_t)b[h+2UL]+(uint32_t)b[h+3UL];
   return !(e&1);
/* 1 bit checksum */
} /* end hamcorr() */


static char hamming(const char b[], uint32_t b_len,
                uint32_t len, char db[], uint32_t db_len)
{
   uint32_t j;
   uint32_t i;
   uint32_t tmp;
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


static uint32_t bits2val(const char b[], uint32_t b_len,
                uint32_t from, uint32_t len)
{
   uint32_t n;
   n = 0UL;
   while (len>0UL) {
      n = n*2UL+(uint32_t)b[from];
      ++from;
      --len;
   }
   return n;
} /* end bits2val() */


static void wh(uint32_t x)
{
   char tmp;
   x = x&15UL;
   if (x<10UL) osi_WrStr((char *)(tmp = (char)(x+48UL),&tmp), 1u/1u);
   else osi_WrStr((char *)(tmp = (char)(x+55UL),&tmp), 1u/1u);
} /* end wh() */


static void killdfid(uint32_t m)
/* reset dfm06 name */
{
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      anonym->idcnt0 = 0UL;
      anonym->idcnt1 = 0UL;
      anonym->txok = 0;
      anonym->id[0U] = 0;
      anonym->idnew = 0UL;
      anonym->frametimeok = 0;
   }
} /* end killdfid() */


static char dfidok(uint32_t m)
{
   return chan[m].dfm6.idcnt0>=dfmidchg && chan[m].dfm6.idcnt1>=dfmidchg;
} /* end dfidok() */


static void checkdfpos(float deg, float odeg, uint32_t m)
/* reset dfm06 name on pos jump */
{
   if ((float)fabs(deg-odeg)>0.25f) {
      if (verb && dfidok(m)) osi_WrStr(" POS JUMP! TX STOP ", 20ul);
      killdfid(m);
   }
} /* end checkdfpos() */


static void checkdf69(float long0, uint32_t m)
{
   chan[m].dfm6.d9 = long0<30.0f;
                /* if long<30 it is df6 lat else is df9 long */
} /* end checkdf69() */

static uint32_t sondeudp_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};

static uint32_t _cnst0[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,181UL,
                212UL,243UL,273UL,304UL,334UL};

static void decodesub(const char b[], uint32_t b_len, uint32_t m,
                uint32_t subnum)
{
   uint32_t u;
   uint32_t v;
   float vr;
   int32_t ui;
   char s[101];
   uint32_t thh;
   uint32_t th;
   uint32_t tt;
   uint32_t tyear;
   uint32_t tmon;
   uint32_t tday;
   uint32_t thour;
   uint32_t tmin;
   char numcntok;
   /* check name, if changed may be checksum error or 2 sondes on same frequency */
   struct DFM6 * anonym;
   if (verb2) {
      if (subnum==0UL) osi_WrStr(" Dat", 5ul);
      else osi_WrStr(" dat", 5ul);
      wh(bits2val(b, b_len, 48UL, 4UL));
      osi_WrStr(":", 2ul);
      for (u = 0UL; u<=5UL; u++) {
         osi_WrHex(bits2val(b, b_len, u*8UL, 8UL), 0UL);
      } /* end for */
   }
   switch (bits2val(b, b_len, 48UL, 4UL)) {
   case 0UL: /* framecount */
      v = bits2val(b, b_len, 24UL, 8UL);
      if (verb) {
         osi_WrStr(" num: ", 7ul);
         osic_WrINT32(v, 0UL);
      }
      numcntok = 0;
      if ((chan[m].dfm6.num+1UL&255UL)==v && chan[m].dfm6.wasdate) {
         ++chan[m].dfm6.numcnt; /* date/num+1/date/num+1 sequence ok */
      }
      else chan[m].dfm6.numcnt = 0UL;
      chan[m].dfm6.num = v;
      chan[m].dfm6.wasdate = 0; /* we have number, next must be date */
      if (chan[m].dfm6.lastdate+60UL==chan[m].dfm6.actdate) {
         /* minute change */
         if (chan[m].dfm6.numcnt>=3UL) {
            chan[m].dfm6.timediff = chan[m].dfm6.actdate-osic_time();
                /* sonde realtime - systime */
            chan[m].dfm6.numcnt = 0UL;
            numcntok = 1;
         }
      }
      if (dfmnametyp<512UL && ((dfmnametyp==0UL || chan[m].dfm6.idcnt0==0UL)
                || chan[m].dfm6.idcnt1==0UL)) {
         /* id with old methode */
         /*
                  IF ((chan[m].dfm6.num+1) MOD 256=v) & chan[m].dfm6.wasdate
                  THEN INC(chan[m].dfm6.numcnt)
                (* date/num+1/date/num+1 sequence ok *)
                  ELSE chan[m].dfm6.numcnt:=0 END;
                  chan[m].dfm6.num:=v;
                  chan[m].dfm6.wasdate:=FALSE;
                (* we have number, next must be date *)
         */
         u = (((v+256UL)-(chan[m].dfm6.idnum&255UL)&255UL)+256UL)
                -(osic_time()+chan[m].dfm6.timediff&255UL)&255UL;
         if (u>=128UL) u = 256UL-u;
         chan[m].dfm6.txok = u<=1UL; /* max +-1s tolerance of num */
         if (verb) {
            osi_WrStr(" [", 3ul);
            aprsstr_IntToStr((int32_t)u, 0UL, s, 101ul);
            if (chan[m].dfm6.txok) aprsstr_Append(s, 101ul, "s seq]", 7ul);
            else aprsstr_Append(s, 101ul, "s OUT OF seq - NO TX]", 22ul);
            osi_WrStr(s, 101ul);
         }
         /*
                  IF chan[m].dfm6.lastdate+60=chan[m]
                .dfm6.actdate THEN              (* minute change *)
                    IF chan[m].dfm6.numcnt>=3 THEN
                      chan[m].dfm6.timediff:=chan[m].dfm6.actdate-time();
                (* sonde realtime - systime *)
         */
         if (numcntok) {
            v = (chan[m].dfm6.num+256UL)-(chan[m].dfm6.actdate&255UL)&255UL;
            if (verb) {
               osi_WrStr(" NEW ID", 8ul);
               osic_WrINT32(v, 0UL);
            }
            chan[m].dfm6.idnum = v;
            if (chan[m].dfm6.d9) strncpy(s,"DFM9",101u);
            else strncpy(s,"DFM6",101u);
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
         /*           END; */
         /*           chan[m].dfm6.numcnt:=0; */
         chan[m].dfm6.tused = osic_time();
      }
      if (chan[m].dfm6.d9) {
         /* dfm09 speed */
         v = bits2val(b, b_len, 32UL, 16UL);
         if (verb) {
            osic_WrFixed((float)v*0.036f, 1L, 0UL);
            osi_WrStr(" km/h", 6ul);
         }
      }
      break;
   case 1UL:
      if (chan[m].dfm6.d9) {
         /* dfm09 lat, dir */
         v = bits2val(b, b_len, 0UL, 32UL);
         u = bits2val(b, b_len, 32UL, 16UL);
         vr = (float)v*1.E-7f;
         checkdfpos(vr, chan[m].dfm6.lastlat, m);
         chan[m].dfm6.lastlat = vr;
         if (verb) {
            osi_WrStr(" lat: ", 7ul);
            osic_WrFixed(vr, 5L, 0UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.01f, 1L, 0UL);
            osi_WrStr(" deg", 5ul);
         }
      }
      else {
         u = bits2val(b, b_len, 24UL, 16UL);
         if (verb) {
            osi_WrStr(" ms: ", 6ul);
            osic_WrINT32(u, 0UL);
         }
      }
      break;
   case 2UL:
      v = bits2val(b, b_len, 0UL, 32UL);
      vr = (float)v*1.E-7f;
      checkdf69(vr, m); /* test if dfm6 or dfm9 */
      if (chan[m].dfm6.d9) {
         /* dfm09 long, clb */
         ui = (int32_t)bits2val(b, b_len, 32UL, 16UL);
         if (ui>=32768L) ui -= 65536L;
         checkdfpos(vr, chan[m].dfm6.lastlong, m);
         chan[m].dfm6.lastlong = vr;
         if (verb) {
            osi_WrStr(" long:", 7ul);
            osic_WrFixed(vr, 5L, 0UL);
            osic_WrFixed((float)ui*0.01f, 1L, 0UL);
            osi_WrStr(" m/s", 5ul);
         }
      }
      else {
         /* dfm06 lat, speed */
         u = bits2val(b, b_len, 32UL, 16UL);
         checkdfpos(vr, chan[m].dfm6.lastlat, m);
         chan[m].dfm6.lastlat = vr;
         if (verb) {
            osi_WrStr(" lat: ", 7ul);
            osic_WrFixed(vr, 5L, 0UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.036f, 1L, 0UL);
            osi_WrStr("km/h", 5ul);
         }
      }
      break;
   case 3UL: /* dfm09 alt */
      if (chan[m].dfm6.d9) {
         v = bits2val(b, b_len, 0UL, 32UL);
         if (verb) {
            osi_WrStr(" alti:", 7ul);
            osic_WrFixed((float)v*0.01f, 1L, 0UL);
            osi_WrStr("m ", 3ul);
         }
      }
      else {
         /* dfm06 long, dir */
         v = bits2val(b, b_len, 0UL, 32UL);
         u = bits2val(b, b_len, 32UL, 16UL);
         vr = (float)v*1.E-7f;
         checkdfpos(vr, chan[m].dfm6.lastlong, m);
         chan[m].dfm6.lastlong = vr;
         if (verb) {
            osi_WrStr(" long:", 7ul);
            osic_WrFixed(vr, 5L, 0UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.01f, 1L, 0UL);
            osi_WrStr(" deg", 5ul);
         }
      }
      break;
   case 4UL:
      if (chan[m].dfm6.d9) {
      }
      else {
         /* dfm06 alt, clb */
         v = bits2val(b, b_len, 0UL, 32UL);
         ui = (int32_t)bits2val(b, b_len, 32UL, 16UL);
         if (ui>=32768L) ui -= 65536L;
         if (verb) {
            osi_WrStr(" alti:", 7ul);
            osic_WrFixed((float)v*0.01f, 1L, 0UL);
            osi_WrStr("m ", 3ul);
            osic_WrFixed((float)ui*0.01f, 1L, 0UL);
            osi_WrStr(" m/s", 5ul);
         }
      }
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
      chan[m].dfm6.wasdate = 1;
      chan[m].dfm6.lastdate = chan[m].dfm6.actdate;
      chan[m].dfm6.actdate = tt;
      th = osic_time();
      if (chan[m].dfm6.lastdate+60UL==chan[m].dfm6.actdate) {
         /* minute jump */
         if (chan[m].dfm6.frametimeok) thh = 3UL;
         else thh = 4UL;
         if (chan[m].dfm6.lastdatesystime+thh>=th) {
            /* in timespan */
            chan[m].dfm6.frametime = chan[m].dfm6.actdate-th;
                /* sonde realtime - systime */
            chan[m].dfm6.frametimeok = 1;
            if (verb) osi_WrStr(" got SECOND", 12ul);
         }
      }
      chan[m].dfm6.lastdatesystime = th;
      if (verb) {
         osi_WrStr(" ", 2ul);
         aprsstr_IntToStr((int32_t)tyear, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, "-", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((int32_t)tmon, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, "-", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((int32_t)tday, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, " ", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((int32_t)thour, 0UL, s, 101ul);
         aprsstr_Append(s, 101ul, ":", 2ul);
         osi_WrStr(s, 101ul);
         aprsstr_IntToStr((int32_t)tmin, 0UL, s, 101ul);
         osi_WrStr(s, 101ul);
      }
      break;
   default:;
      if (verb && !verb2) {
         if (subnum==0UL) osi_WrStr(" Dat", 5ul);
         else osi_WrStr(" dat", 5ul);
         wh(bits2val(b, b_len, 48UL, 4UL));
         osi_WrStr(":", 2ul);
         for (u = 0UL; u<=5UL; u++) {
            osi_WrHex(bits2val(b, b_len, u*8UL, 8UL), 0UL);
         } /* end for */
      }
      break;
   } /* end switch */
} /* end decodesub() */


static void getdfname(const char b[], uint32_t b_len, uint32_t m,
                uint32_t startbyte)
{
   uint32_t n;
   uint32_t u;
   uint32_t v;
   char s[101];
   v = bits2val(b, b_len, 0UL, 8UL);
   if (v==startbyte) {
      u = bits2val(b, b_len, 24UL, 4UL);
      v = bits2val(b, b_len, 8UL, 16UL);
      if (u==1UL) {
         /* id low 16 bit*/
         n = chan[m].dfm6.idnew&65535UL;
         if (n==0UL || n==v) {
            ++chan[m].dfm6.idcnt0;
            chan[m].dfm6.idtime = osic_time();
            if (verb) osi_WrStr(" ID LOW ", 9ul);
         }
         else {
            killdfid(m);
            if (verb) osi_WrStr(" NEW ID LOW ", 13ul);
         }
         chan[m].dfm6.idnew = (uint32_t)((uint32_t)
                chan[m].dfm6.idnew&0xFFFF0000UL)+v;
      }
      else if (u==0UL) {
         /* id low 16 bit*/
         n = chan[m].dfm6.idnew/65536UL;
         if (n==0UL || n==v) {
            ++chan[m].dfm6.idcnt1;
            chan[m].dfm6.idtime = osic_time();
            if (verb) osi_WrStr(" ID HIGH ", 10ul);
         }
         else {
            killdfid(m);
            if (verb) osi_WrStr(" NEW ID HIGH ", 14ul);
         }
         chan[m].dfm6.idnew = (uint32_t)((uint32_t)
                chan[m].dfm6.idnew&0xFFFFUL)+v*65536UL;
      }
      if (dfidok(m)) {
         v = chan[m].dfm6.idnew;
         if (chan[m].dfm6.d9) strncpy(s,"DF9",101u);
         else strncpy(s,"DF6",101u);
         s[3U] = hex(v/65536UL);
         s[4U] = hex(v/4096UL);
         s[5U] = hex(v/256UL);
         s[6U] = hex(v/16UL);
         s[7U] = hex(v);
         s[8U] = 0;
         aprsstr_Assign(chan[m].dfm6.id, 9ul, s, 101ul);
         memcpy(chan[m].dfm6.idcheck,chan[m].dfm6.id,9u);
         chan[m].dfm6.txok = 1;
         chan[m].dfm6.tused = osic_time();
         if (verb) {
            osi_WrStr(" ID:", 5ul);
            osi_WrStr(s, 101ul);
         }
      }
   }
} /* end getdfname() */


static void decodeframe6(uint32_t m)
{
   uint32_t j;
   uint32_t i;
   uint32_t rt;
   char s[201];
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
      if (anonym->tused+3600UL<osic_time()) anonym->id[0U] = 0;
      if (hamming(anonym->dh1, 104ul, 13UL, anonym->db1,
                104ul) && hamming(anonym->dh2, 104ul, 13UL, anonym->db2,
                104ul)) {
         if (verb) {
            WrChan((int32_t)m);
            if (anonym->id[0U]) osi_WrStr(anonym->id, 9ul);
            else if (chan[m].dfm6.d9) osi_WrStr("DF9", 4ul);
            else osi_WrStr("DF6", 4ul);
            WrdB(chan[m].admax-chan[m].admin);
            WrQ(anonym->bitlev, chan[m].dfm6.noise);
            Wrtune(chan[m].admax+chan[m].admin, chan[m].admax-chan[m].admin);
         }
         /*
                 WrStr(" ");
                 IF frametimeok THEN wrdate(time()+chan[m].dfm6.frametime);
                 ELSE WrStr("[wait for GPS Time]") END;
         */
         if (hamming(anonym->ch, 56ul, 7UL, anonym->cb, 56ul)) {
            if (dfmnametyp>=256UL) {
               getdfname(anonym->cb, 56ul, m, dfmnametyp&255UL);
            }
            if (verb) {
               osi_WrStr(" ", 2ul);
               for (i = 0UL; i<=6UL; i++) {
                  wh(bits2val(anonym->cb, 56ul, i*4UL, 4UL));
               } /* end for */
            }
         }
         decodesub(anonym->db1, 104ul, m, 0UL);
         /*      IF verb THEN WrStr(" /") END; */
         decodesub(anonym->db2, 104ul, m, 1UL);
         { /* with */
            struct DFM6 * anonym0 = &chan[m].dfm6;
            tx = (anonym0->id[0U] && aprsstr_StrCmp(anonym0->idcheck, 9ul,
                anonym0->id, 9ul)) && anonym0->idtime+900UL>osic_time();
            for (i = 0UL; i<=9UL; i++) {
               s[i] = 0;
            } /* end for */
            if (tx && anonym0->txok) {
               /* else stop sending if ambigous id */
               for (i = 0UL; i<=7UL; i++) {
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
            if (anonym0->frametimeok) {
               rt = osic_time()+chan[m].dfm6.frametime;
                /* interpolated sonde realtime */
            }
            else rt = 0UL;
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
            sdrparm(s, 201ul, &j, (int32_t)m);
            alludp(chan[m].udptx, j, s, 201ul);
            if ((verb && !tx) && anonym0->id[0U]) {
               osi_WrStrLn("", 1ul);
               osi_WrStr(" changing name ", 16ul);
               osi_WrStr(anonym0->id, 9ul);
               osi_WrStr("<->", 4ul);
               osi_WrStr(anonym0->idcheck, 9ul);
            }
         }
         /*build tx frame */
         if (verb) {
            appendsdr(m);
            osi_WrStrLn("", 1ul);
         }
      }
   }
} /* end decodeframe6() */


static void demodbyte6(uint32_t m, char d)
{
   /*  WrInt(ORD(d),1); */
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=264UL) {
         if ((anonym->synword&65535UL)==17871UL) anonym->rxp = 0UL;
         else if ((anonym->synword&65535UL)==47664UL) {
            /* inverse start sequence found */
            anonym->polarity = !anonym->polarity;
            anonym->rxp = 0UL;
         }
         if (anonym->rxp==0UL) chan[m].framestarttime = osic_time();
      }
      else {
         anonym->rxbuf[anonym->rxp] = d;
         ++anonym->rxp;
         if (anonym->rxp==264UL && chan[m].framestarttime+3UL>=osic_time()) {
            decodeframe6(m);
         }
      }
   }
} /* end demodbyte6() */


static void demodbit6(uint32_t m, float u, float u0)
{
   char d;
   float ua;
   struct DFM6 * anonym;
   /*WrFixed(u,0,9); WrStr(" "); */
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
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)
                *0.02f;
      }
   }
/*quality*/
} /* end demodbit6() */


static void demod6(float u, uint32_t m)
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


static void Fsk6(uint32_t m)
{
   float ff;
   int32_t lim;
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].dfm6;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),
                16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod6(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk6() */

/*------------------------------ C34 C50 */

static float SaveReal(uint32_t c)
{
   uint32_t e;
   e = c/16777216UL;
   if (e==127UL) return X2C_max_real;
   if (e==255UL) return X2C_min_real;
   return *X2C_CAST(&c,uint32_t,float,float *);
} /* end SaveReal() */


static void demodframe34(uint32_t channel)
{
   uint32_t flen;
   uint32_t val;
   uint32_t sum2;
   uint32_t sum1;
   uint32_t i;
   double hr;
   char s[101];
   char ok0;
   struct C34 * anonym;
   struct CHAN * anonym0; /* call if set */
   char tmp;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      sum1 = 0UL;
      sum2 = 65791UL;
      for (i = 2UL; i<=6UL; i++) {
         sum1 += (uint32_t)(uint8_t)anonym->rxbuf[i];
         sum2 -= (uint32_t)(uint8_t)anonym->rxbuf[i]*(7UL-i);
      } /* end for */
      sum1 = sum1&255UL;
      sum2 = sum2&255UL;
      ok0 = sum1==(uint32_t)(uint8_t)
                anonym->rxbuf[7U] && sum2==(uint32_t)(uint8_t)
                anonym->rxbuf[8U];
      if (anonym->tused+3600UL<osic_time()) {
         anonym->id34.id[0U] = 0; /* timed out context */
         anonym->id50.id[0U] = 0;
      }
      if (verb && ok0 || verb2) {
         if (maxchannels>0UL) {
            osic_WrINT32(channel+1UL, 1UL);
            osi_WrStr(":", 2ul);
         }
         if (anonym->c50) {
            osi_WrStr("C50 ", 5ul);
            osi_WrStr(anonym->id50.id, 9ul);
         }
         else {
            osi_WrStr("C34 ", 5ul);
            osi_WrStr(anonym->id34.id, 9ul);
         }
         WrdB(chan[channel].admax-chan[channel].admin);
         WrQuali(noiselevel(anonym->bitlev, anonym->noise));
         Wrtune(chan[channel].admax+chan[channel].admin,
                chan[channel].admax-chan[channel].admin);
         osi_WrStr(" [", 3ul);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[2U], 2UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[3U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[4U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[5U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[6U], 2UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[7U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[8U], 2UL);
         osi_WrStr("] ", 3ul);
      }
      if (ok0) {
         /* chksum ok */
         val = (uint32_t)(uint8_t)anonym->rxbuf[6U]+(uint32_t)
                (uint8_t)anonym->rxbuf[5U]*256UL+(uint32_t)(uint8_t)
                anonym->rxbuf[4U]*65536UL+(uint32_t)(uint8_t)
                anonym->rxbuf[3U]*16777216UL;
         hr = (double)SaveReal(val);
         /*WrFixed(hr, 10, 20); */
         if (anonym->c50) {
            if (anonym->id50.idtime+3600UL<osic_time()) {
               anonym->id50.id[0U] = 0;
               anonym->id50.idcheck[0U] = 0;
               anonym->id50.idcnt = 0UL;
            }
            /* remove old id */
            switch ((unsigned)anonym->rxbuf[2U]) {
            case '\003':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp1 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\004':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp2 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\005':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp3 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\020':
               if (hr<=100.0 && hr>=0.0) {
                  if (verb) {
                     osi_WrStr("hum ", 5ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("%", 2ul);
                  }
               }
               break;
            case '\024':
               if (verb) {
                  osi_WrStr("date", 5ul);
                  aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL,
                 s, 101ul);
                  s[0U] = ' ';
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\025':
               if (verb) {
                  aprsstr_TimeToStr((val/10000UL)*3600UL+((val%10000UL)
                /100UL)*60UL+val%100UL, s, 101ul);
                  osi_WrStr("time ", 6ul);
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\026':
               hr = latlong(val, anonym->c50);
               if (hr<89.9 && hr>(-89.9)) {
                  if (verb) {
                     osi_WrStr("lat  ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\027':
               hr = latlong(val, anonym->c50);
               if (hr<180.0 && hr>(-180.0)) {
                  if (verb) {
                     osi_WrStr("long ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\030':
               hr = (double)((float)val*0.1f);
               if (hr<50000.0) {
                  if (verb) {
                     osi_WrStr("alti ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("m", 2ul);
                  }
               }
               break;
            case 'd': /* 66H 67H 68H 89H 6BH seem to be fixed too */
               strncpy(s,"SC50",101u);
               s[4U] = hex(val/4096UL&7UL);
               s[5U] = hex(val/256UL);
               s[6U] = hex(val/16UL);
               s[7U] = hex(val);
               s[8U] = 0;
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
               if (aprsstr_StrCmp(anonym->id50.idcheck, 9ul, s, 101ul)) {
                  ++anonym->id50.idcnt; /* got same name again */
               }
               else {
                  /* new name so check if wrong */
                  aprsstr_Assign(anonym->id50.idcheck, 9ul, s, 101ul);
                  anonym->idcnt = 1UL;
               }
               if (anonym->id50.idcnt>2UL || anonym->id50.id[0U]==0) {
                  /* first name or safe new name */
                  memcpy(anonym->id50.id,anonym->id50.idcheck,9u);
                  anonym->id50.idtime = osic_time();
               }
               anonym->tused = osic_time();
               break;
            default:;
               if (verb2) {
                  /*WrStr("????");*/
                  osic_WrINT32(val, 12UL);
                  osic_WrINT32(val/65536UL, 7UL);
                  osic_WrINT32(val&65535UL, 7UL);
                  osic_WrFixed((float)hr, 2L, 10UL);
                  osi_WrStr(" ", 2ul);
                  for (i = 31UL;; i--) {
                     osi_WrStr((char *)(tmp = (char)
                (48UL+(uint32_t)X2C_IN(i,32,(uint32_t)val)),&tmp),
                1u/1u);
                     if (i==0UL) break;
                  } /* end for */
               }
               break;
            } /* end switch */
         }
         else {
            /* SC34 */
            if (anonym->id34.idtime+3600UL<osic_time()) {
               anonym->id34.id[0U] = 0;
               anonym->id34.idcheck[0U] = 0;
               anonym->id34.idcnt = 0UL;
            }
            /* remove old id */
            switch ((unsigned)anonym->rxbuf[2U]) {
            case '\003':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tmp1 ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\007':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("dewp ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\024':
               if (verb) {
                  osi_WrStr("date", 5ul);
                  aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL,
                 s, 101ul);
                  s[0U] = ' ';
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\025':
               if (verb) {
                  aprsstr_TimeToStr((val/10000UL)*3600UL+((val%10000UL)
                /100UL)*60UL+val%100UL, s, 101ul);
                  osi_WrStr("time ", 6ul);
                  osi_WrStr(s, 101ul);
               }
               break;
            case '\026':
               hr = latlong(val, anonym->c50);
               if (hr<89.9 && hr>(-89.9)) {
                  if (verb) {
                     osi_WrStr("lati ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\027':
               hr = latlong(val, anonym->c50);
               if (hr<180.0 && hr>(-180.0)) {
                  if (verb) {
                     osi_WrStr("long ", 6ul);
                     osic_WrFixed((float)hr, 5L, 0UL);
                  }
               }
               break;
            case '\030':
               hr = (double)((float)val*0.1f);
               if (hr<50000.0) {
                  if (verb) {
                     osi_WrStr("alti ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("m", 2ul);
                  }
               }
               break;
            case '\031':
               hr = (double)((float)val*0.1852f);
                /*1.609*/ /*1.852*/ /* guess knots or miles */
               if (hr<1000.0) {
                  if (verb) {
                     osi_WrStr("wind ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("km/h", 5ul);
                  }
               }
               break;
            case '\032':
               hr = (double)((float)val*0.1f);
               if (hr>=0.0 && hr<=360.0) {
                  if (verb) {
                     osi_WrStr("wdir ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("deg", 4ul);
                  }
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
               if (aprsstr_StrCmp(anonym->id34.idcheck, 9ul, s, 101ul)) {
                  ++anonym->id34.idcnt; /* got same name again */
               }
               else {
                  /* new name so check if wrong */
                  aprsstr_Assign(anonym->id34.idcheck, 9ul, s, 101ul);
                  anonym->idcnt = 1UL;
               }
               if (anonym->id34.idcnt>3UL || anonym->id34.id[0U]==0) {
                  /* first name or safe new name */
                  memcpy(anonym->id34.id,anonym->id34.idcheck,9u);
                  anonym->id34.idtime = osic_time();
               }
               anonym->tused = osic_time();
               break;
            default:;
               if (verb2) {
                  osi_WrStr("????", 5ul);
                  osic_WrINT32(val, 12UL);
                  osic_WrFixed((float)hr, 2L, 10UL);
               }
               break;
            } /* end switch */
         }
         /* build tx frame */
         if ((anonym->c50 && anonym->id50.id[0U])
                && aprsstr_StrCmp(anonym->id50.idcheck, 9ul, anonym->id50.id,
                 9ul) || (!anonym->c50 && anonym->id34.id[0U])
                && aprsstr_StrCmp(anonym->id34.idcheck, 9ul, anonym->id34.id,
                 9ul)) {
            /* stop sending if ambigous id */
            if (anonym->c50) {
               for (i = 0UL; i<=8UL; i++) {
                  s[i] = anonym->id50.id[i];
               } /* end for */
            }
            else {
               for (i = 0UL; i<=8UL; i++) {
                  s[i] = anonym->id34.id[i];
               } /* end for */
            }
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
            flen = 22UL;
            sdrparm(s, 101ul, &flen, (int32_t)channel);
            alludp(chan[channel].udptx, flen, s, 101ul);
         }
         else if (verb) {
            if (anonym->c50) {
               if (anonym->id50.id[0U]) {
                  osi_WrStr(" changing name ", 16ul);
                  osi_WrStr(anonym->id50.id, 9ul);
                  osi_WrStr("<->", 4ul);
                  osi_WrStr(anonym->id50.idcheck, 9ul);
               }
            }
            else if (anonym->id34.id[0U]) {
               osi_WrStr(" changing name ", 16ul);
               osi_WrStr(anonym->id34.id, 9ul);
               osi_WrStr("<->", 4ul);
               osi_WrStr(anonym->id34.idcheck, 9ul);
            }
         }
      }
      else if (verb2) {
         /*build tx frame */
         osi_WrStr("---- chksum ", 13ul);
         osi_WrHex(sum1, 2UL);
         osi_WrHex(sum2, 2UL);
      }
      if (verb2 || ok0 && verb) {
         appendsdr(channel);
         osi_WrStrLn("", 1ul);
      }
   }
/* name(9) 0C call(5) playload(7) */
} /* end demodframe34() */


static void demodbit34(uint32_t channel, char d)
{
   struct C34 * anonym;
   d = !d;
   { /* with */
      struct C34 * anonym = &chan[channel].c34;
      anonym->rxbytec = anonym->rxbytec*2UL+(uint32_t)d;
      if ((anonym->rxbytec&268435455UL)==234942462UL) {
         /* c34 1110 0000 0000 1110 1111 1111 1110*/
         anonym->c50 = 0;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
         chan[channel].framestarttime = osic_time();
      }
      else if ((anonym->rxbytec&2097151UL)==3070UL) {
         /* c50 0 0000 0000 1011 1111 1110 */
         anonym->c50 = 1;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
         chan[channel].framestarttime = osic_time();
      }
      if ((anonym->c50 || anonym->rxbitc) || !d) {
         if (anonym->rxbitc<=8UL) {
            /* databits */
            anonym->rxbyte = (anonym->rxbyte&255UL)/2UL;
            if (d) anonym->rxbyte += 128UL;
            ++anonym->rxbitc;
         }
         else if (anonym->rxp>0UL) {
            /* byte ready */
            anonym->rxbitc = 0UL;
            anonym->rxbuf[anonym->rxp] = (char)anonym->rxbyte;
            ++anonym->rxp;
            if (anonym->rxp>8UL) {
               if (chan[channel].framestarttime+3UL>=osic_time()) {
                  demodframe34(channel);
               }
               anonym->rxp = 0UL;
            }
         }
      }
   }
} /* end demodbit34() */


static void demod34(float u, uint32_t channel)
{
   char d;
   float ua;
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
         /*quality*/
         ua = (float)fabs(u)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.02f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)
                *0.05f;
      }
      else {
         /*quality*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
} /* end demod34() */


static void Afsk(uint32_t channel)
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
         a = (float)(uint32_t)X2C_TRUNCC(d*16.0f+0.5f,0UL,
                X2C_max_longcard);
         b = a*0.0625f;
         if ((uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)>0UL && (uint32_t)X2C_TRUNCC(a,0UL,
                X2C_max_longcard)<16UL) {
            mid = Fir(afin, 16UL-(uint32_t)X2C_TRUNCC(a,0UL,
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
      anonym->baudfine += (int32_t)anonym->demodbaud;
      if (anonym->baudfine>=65536L) {
         anonym->baudfine -= 65536L;
         if (anonym->baudfine<65536L) {
            /* normal alway true */
            ff = Fir(anonym->dfin, (uint32_t)(16L-anonym->baudfine/4096L),
                16UL, anonym->dfir, 64ul, anonym->dfirtab, 1024ul);
            demod34(ff, channel);
         }
      }
   }
} /* end Afsk() */

/*---------------------- M10 */

static uint16_t crcm10(int32_t len, const char buf[],
                uint32_t buf_len)
{
   int32_t i;
   uint16_t s;
   uint16_t t;
   uint16_t b;
   uint16_t cs;
   int32_t tmp;
   cs = 0U;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (uint16_t)(uint32_t)(uint8_t)buf[i];
      b = X2C_LSH(b,16,-1)|X2C_LSH(b&0x1U,16,7);
      b = b^X2C_LSH(b,16,-2)&0xFFU;
      t = cs&0x3FU|X2C_LSH((cs^X2C_LSH(cs,16,-2)^X2C_LSH(cs,16,-4))&0x1U,16,
                6)|X2C_LSH((X2C_LSH(cs,16,-1)^X2C_LSH(cs,16,-3)^X2C_LSH(cs,
                16,-5))&0x1U,16,7);
      s = X2C_LSH(cs,16,-7)&0xFFU;
      s = (s^X2C_LSH(s,16,-2))&0xFFU;
      cs = X2C_LSH(cs&0xFFU,16,8)|b^t^s;
      if (i==tmp) break;
   } /* end for */
   return (uint16_t)cs;
} /* end crcm10() */


static uint32_t m10card(const char b[], uint32_t b_len,
                int32_t pos, int32_t len)
{
   int32_t i;
   uint32_t n;
   int32_t tmp;
   n = 0UL;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      n = n*256UL+(uint32_t)(uint8_t)b[pos+i];
      if (i==tmp) break;
   } /* end for */
   return n;
} /* end m10card() */


static uint32_t m10rcard(const char b[], uint32_t b_len,
                int32_t pos, int32_t len)
{
   int32_t i;
   uint32_t n;
   n = 0UL;
   for (i = len-1L; i>=0L; i--) {
      n = n*256UL+(uint32_t)(uint8_t)b[pos+i];
   } /* end for */
   return n;
} /* end m10rcard() */

typedef uint32_t SET256[8];

static float sondeudp_DEGMUL = 8.3819036711397E-8f;

#define sondeudp_VMUL 0.005

#define sondeudp_CRCPOS 99
/*      HSET  =SET256{0..2,4..25,32,33,93..97,99,100}
                ;                (* not hexlist known bytes *) */

static SET256 sondeudp_HSET = {0x03FFFFF7UL,0x00000003UL,0x00000000UL,
                0x00000018UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL}; /* not hexlist known bytes */

static SET256 sondeudp_VARSET = {0x03BBBBF0UL,0x80600000UL,0x06A001A0UL,
                0x0000001CUL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL}; /* known as variable */

static float sondeudp_Rs[3] = {12100.0f,36500.0f,4.75E+5f};

static float sondeudp_Rp[3] = {1.21E-16f,1.1060606E-1f,1.5833333E-1f};

static SET256 _cnst4 = {0x03FFFFF7UL,0x00000003UL,0x00000000UL,0x00000018UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};
static float _cnst3[3] = {12100.0f,36500.0f,4.75E+5f};
static float _cnst2[3] = {1.21E-16f,1.1060606E-1f,1.5833333E-1f};
static SET256 _cnst1 = {0x03BBBBF0UL,0x80600000UL,0x06A001A0UL,0x0000001CUL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};

static void decodeframe10(uint32_t m)
{
   uint32_t gpstimecorr;
   uint32_t repl;
   uint32_t id;
   uint32_t flen;
   uint32_t tab;
   uint32_t week;
   uint32_t tow;
   uint32_t i;
   int32_t ci;
   double dir;
   double v;
   double vv;
   double vn;
   double ve;
   double alt;
   double lon;
   double lat;
   uint32_t sct;
   float rt;
   char crcok;
   char tok;
   uint8_t repairstep;
   char ids[201];
   char s[201];
   struct M10 * anonym;
   struct CHAN * anonym0; /* call if set */
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      anonym->txok = 0;
      repairstep = 16U;
      repl = 0UL;
      for (;;) {
         crcok = (uint32_t)crcm10(99L, anonym->rxbuf,
                101ul)==m10card(anonym->rxbuf, 101ul, 99L, 2L);
         if ((anonym->alternativ || crcok) || repairstep==0U) break;
         repl = 0UL;
         for (i = 0UL; i<=98UL; i++) {
            if (!X2C_INL(i,256,_cnst1) && anonym->fixcnt[i]>=repairstep) {
               /* replace stable bytes */
               if (anonym->rxbuf[i]!=anonym->fixbytes[i]) {
                  ++repl;
                  anonym->rxbuf[i] = anonym->fixbytes[i];
               }
            }
         } /* end for */
         repairstep = repairstep/2U; /* make next crc check */
      }
      if (crcok) {
         /* update fixbyte statistics */
         for (i = 0UL; i<=98UL; i++) {
            if (anonym->fixbytes[i]==anonym->rxbuf[i]) {
               if (anonym->fixcnt[i]<255U) ++anonym->fixcnt[i];
            }
            else {
               anonym->fixbytes[i] = anonym->rxbuf[i];
               anonym->fixcnt[i] = 0U;
            }
         } /* end for */
         /* update fixbyte statistics */
         /* get ID    */
         /* IF m10newID THEN */
         ids[0U] = 'M';
         ids[1U] = 'E';
         ids[2U] = hex((uint32_t)(uint8_t)anonym->rxbuf[95U]/16UL);
                /* should be 0..9 */
         ids[3U] = hex((uint32_t)(uint8_t)anonym->rxbuf[95U]);
         ids[4U] = hex((uint32_t)(uint8_t)anonym->rxbuf[93U]);
         id = (uint32_t)(uint8_t)anonym->rxbuf[96U]+(uint32_t)
                (uint8_t)anonym->rxbuf[97U]*256UL;
         ids[5U] = hex(id/4096UL);
         ids[6U] = hex(id/256UL);
         ids[7U] = hex(id/16UL);
         ids[8U] = hex(id);
         ids[9U] = 0;
         /* 
            ELSE
              id:=CAST(CARDINAL,
                  (CAST(SET32, ORD(rxbuf[97]) + 100H*ORD(rxbuf[96]) + 10000H*ORD(rxbuf[95]))
                 /CAST(SET32, ORD(rxbuf[93]) DIV 10H + 10H*ORD(rxbuf[94]) + 1000H*ORD(rxbuf[95])))
                  *SET32{0..19});
              i:=8;
              ids[i]:=0C;
              DEC(i);
              REPEAT
                ids[i]:=CHR(id MOD 10 + ORD("0"));
                id:=id DIV 10;
                DEC(i);
              UNTIL i=1;
              ids[i]:="E";
              DEC(i);
              ids[i]:="M";
            END;
         */
         /* get ID */
         if (anonym->alternativ) {
            if (verb) {
               WrChan((int32_t)m);
               osi_WrStr("M10 ", 5ul);
               osi_WrStr(ids, 201ul);
               osi_WrStr(" ", 2ul);
               wrtime(anonym->timefn%86400UL);
               osi_WrStr(" typ=", 6ul);
               osic_WrINT32(m10card(anonym->rxbuf, 101ul, 2L, 1L), 1UL);
               osi_WrStr(" ", 2ul);
            }
         }
         else {
            tow = m10card(anonym->rxbuf, 101ul, 10L, 4L);
            week = m10card(anonym->rxbuf, 101ul, 32L, 2L);
            gpstimecorr = (uint32_t)(uint8_t)anonym->rxbuf[31U];
            anonym->timefn = (tow/1000UL+week*604800UL+315964800UL)
                -gpstimecorr;
            /*        IF verb2 THEN WrStr(" ");DateToStr(time, s); WrStr(s);
                WrStr(" ") END; */
            lat = (double)m10card(anonym->rxbuf, 101ul, 14L,
                4L)*8.3819036711397E-8;
            lon = (double)m10card(anonym->rxbuf, 101ul, 18L,
                4L)*8.3819036711397E-8;
            alt = (double)m10card(anonym->rxbuf, 101ul, 22L, 4L)*0.001;
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 4L, 2L);
            if (ci>32767L) ci -= 65536L;
            ve = (double)ci*0.005;
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 6L, 2L);
            if (ci>32767L) ci -= 65536L;
            vn = (double)ci*0.005;
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 8L, 2L);
            if (ci>32767L) ci -= 65536L;
            vv = (double)ci*0.005;
            v = (double)osic_sqrt((float)(ve*ve+vn*vn));
                /* hor speed */
            dir = atang2(vn, ve)*5.7295779513082E+1;
            if (dir<0.0) dir = 360.0+dir;
            anonym->txok = 1;
            /*- m10 temp */
            tok = 0;
            sct = m10rcard(anonym->rxbuf, 101ul, 62L, 1L);
            rt = (float)(m10rcard(anonym->rxbuf, 101ul, 63L, 2L)&4095UL);
            /*
                    tcal0:=m10rcard(rxbuf, 65, 2);
                    tcal1:=m10rcard(rxbuf, 67, 2);
            */
            rt = X2C_DIVR(4095.0f-rt,rt)-_cnst2[sct];
            if (rt>0.0f && sct<3UL) {
               rt = X2C_DIVR(_cnst3[sct],rt);
               if (rt>0.0f) {
                  rt = osic_ln(rt);
                  rt = X2C_DIVR(1.0f,
                1.07303516E-3f+2.41296733E-4f*rt+2.26744154E-6f*rt*rt+6.52855181E-8f*rt*rt*rt)
                -273.15f;
                  tok = rt>(-99.0f) && rt<50.0f;
               }
            }
            /*- m10 hum */
            if (verb) {
               WrChan((int32_t)m);
               osi_WrStr("M10 ", 5ul);
               osi_WrStr(ids, 201ul);
               osi_WrStr(" ", 2ul);
               aprsstr_DateToStr(anonym->timefn, s, 201ul);
               osi_WrStr(s, 201ul);
               /*          wrtime(timefn MOD 86400); */
               osi_WrStr(" ", 2ul);
               osic_WrFixed((float)lat, 5L, 1UL);
               osi_WrStr(" ", 2ul);
               osic_WrFixed((float)lon, 5L, 1UL);
               osi_WrStr(" ", 2ul);
               if (alt<1.E+5 && alt>(-1.E+5)) {
                  osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(alt,
                X2C_min_longint,X2C_max_longint), 1UL);
                  osi_WrStr("m ", 3ul);
               }
               osic_WrFixed((float)(v*3.6), 1L, 1UL);
               osi_WrStr("km/h ", 6ul);
               osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(dir,
                X2C_min_longint,X2C_max_longint), 1UL);
               osi_WrStr("deg ", 5ul);
               osic_WrFixed((float)vv, 1L, 1UL);
               osi_WrStr("m/s", 4ul);
               if (tok) {
                  osi_WrStr(" ", 2ul);
                  osic_WrFixed(rt, 1L, 1UL);
                  osi_WrStr("C", 2ul);
               }
            }
         }
         /* build tx frame */
         if (anonym->txok) {
            for (i = 0UL; i<=8UL; i++) {
               s[i+7UL] = ids[i];
            } /* end for */
            { /* with */
               struct CHAN * anonym0 = &chan[m];
               s[0U] = (char)(anonym0->mycallc/16777216UL);
               s[1U] = (char)(anonym0->mycallc/65536UL&255UL);
               s[2U] = (char)(anonym0->mycallc/256UL&255UL);
               s[3U] = (char)(anonym0->mycallc&255UL);
               if (anonym0->mycallc>0UL) s[4U] = anonym0->myssid;
               else s[4U] = '\020';
               s[5U] = 0;
               s[6U] = 0;
            }
            for (i = 0UL; i<=100UL; i++) {
               s[i+16UL] = anonym->rxbuf[i]; /* payload */
            } /* end for */
            flen = 117UL;
            sdrparm(s, 201ul, &flen, (int32_t)m);
            alludp(chan[m].udptx, flen, s, 201ul);
         }
      }
      else if (verb) {
         /*build tx frame */
         WrChan((int32_t)m);
         osi_WrStr("M10 crc error", 14ul);
      }
      if (verb) {
         WrdB(chan[m].admax-chan[m].admin);
         WrQuali(noiselevel(anonym->bitlev, anonym->noise));
         if (repl>0UL) {
            osi_WrStr(" +r", 4ul);
            osic_WrINT32(repl, 1UL);
            osi_WrStr("/", 2ul);
            osic_WrINT32((uint32_t)repairstep, 1UL);
         }
         Wrtune(chan[m].admax+chan[m].admin, chan[m].admax-chan[m].admin);
         appendsdr(m);
      }
      if (verb2) {
         /*
               FOR i:=0 TO 23 DO
                 IF i MOD 10=0 THEN WrStrLn("") END;
                 WrInt(m10card(rxbuf, 48+i*2, 2), 6); WrStr(" ");
               END;
         */
         tab = 0UL;
         for (i = 0UL; i<=100UL; i++) {
            if (anonym->alternativ || !X2C_INL(i,256,_cnst4)) {
               if (tab%12UL==0UL) osi_WrStrLn("", 1ul);
               osic_WrINT32(i, 3UL);
               osi_WrStr(":", 2ul);
               osi_WrHex((uint32_t)(int32_t)m10card(anonym->rxbuf, 101ul,
                 (int32_t)i, 1L), 3UL);
               ++tab;
            }
         } /* end for */
      }
      /*i:=ORD(rxbuf[89]) + ORD(rxbuf[88])*0100H + ORD(rxbuf[87])
                *010000H +ORD(rxbuf[86])*01000000H; */
      /*WrStr(" "); WrInt(i, 12); WrStr(" ");  WrFixed(SaveReal(i), 10, 10);
                */
      /*89 90  63 64 */
      /*WrStrLn(""); WrInt(CAST(INT16, m10rcard(rxbuf, 63, 4)), 6);
                WrInt(CAST(INT16, m10rcard(rxbuf, 89, 4)), 6);  WrStrLn("");
                 */
      /*
            FOR i:=0 TO HIGH(rxbuf) DO
              IF i MOD 24=0 THEN WrStrLn(""); WrInt(i, 2); WrStr(":"); END; 
              IF i IN HSET THEN WrStr(" . ") ELSE WrHex(ORD(rxbuf[i]),
                3) END;
      --        WrHex(ORD(rxbuf[i]), 3);
            END;
      */
      /*
            FOR i:=0 TO HIGH(rxbuf) DO
              IF i MOD 24=0 THEN WrStrLn(""); WrInt(i, 2); WrStr(":"); END;
              IF rxbuf[i]=diffb[i] THEN WrStr(" . ") ELSE WrHex(ORD(rxbuf[i])
                , 3) END;
            END;
      diffb:=rxbuf;
      */
      if (verb) osi_WrStrLn("", 1ul);
   }
} /* end decodeframe10() */


static void demodbyte10(uint32_t m, char d)
{
   struct M10 * anonym;
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      /*WrInt(ORD(d),1);  */
      /*WrInt(rxp, 4); WrStrLn(""); */
      anonym->synword1 = anonym->synword1*2UL+(uint32_t)(((uint32_t)
                anonym->synword&0x800000UL)!=0UL);
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=101UL) {
         if (((uint32_t)anonym->synword1&0xFFC7UL)==0xC0UL) {
            /* unique frame sync */
            /*WrStr(" -fsyn- ");WrHex(frsyn, 15); WrHex(synword, 15); */
            if (((uint32_t)anonym->synword&0xFFFFFFUL)==0x649F20UL) {
               anonym->rxp = 3UL;
               anonym->alternativ = 0;
            }
            else if (((uint32_t)anonym->synword&0xFFFFF0UL)==0x644900UL) {
               /*WrStr(" -syn- "); WrHex(synword1 DIV 256, 10);
                WrHex(synword1, 15); */
               anonym->rxp = 3UL;
               anonym->alternativ = 1;
            }
            /*WrStr(" -syn1- "); */
            if (anonym->rxp==3UL) {
               anonym->rxb = 0UL;
               anonym->rxbuf[0U] = (char)(anonym->synword/65536UL);
               anonym->rxbuf[1U] = (char)(anonym->synword/256UL&255UL);
               anonym->rxbuf[2U] = (char)(anonym->synword&255UL);
               chan[m].framestarttime = osic_time();
            }
         }
      }
      else {
         ++anonym->rxb;
         if (anonym->rxb>=8UL) {
            anonym->rxbuf[anonym->rxp] = (char)(anonym->synword&255UL);
            anonym->rxb = 0UL;
            ++anonym->rxp;
            if (anonym->rxp==101UL && chan[m].framestarttime+3UL>=osic_time()
                ) decodeframe10(m);
         }
      }
   }
} /* end demodbyte10() */


static void demodbit10(uint32_t m, float u, float u0)
{
   char bit;
   char d;
   float ua;
   struct M10 * anonym;
   /*IF manchestd>20000 THEN  */
   /*WrInt(VAL(INTEGER, u), 8); Flush; */
   d = u>=0.0f;
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      /*WrInt(manchestd DIV 256, 1); WrStr("("); WrInt(ORD(lastmanch),1);
                WrInt(ORD(d),1);WrStr(")");  Flush(); */
      /*WrInt(ORD(lastmanch<>d),1); Flush(); */
      /*END; */
      if (anonym->lastmanch==d) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      bit = d!=anonym->lastmanch;
      if (anonym->manchestd>0L) {
         demodbyte10(m, bit);
         /*quality*/
         ua = (float)fabs(u)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.02f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)
                *0.05f;
      }
      /*quality*/
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
   }
} /* end demodbit10() */


static void demod10(float u, uint32_t m)
{
   char d;
   struct M10 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbit10(m, u, anonym->lastu);
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
} /* end demod10() */


static void Fsk10(uint32_t m)
{
   float ff;
   int32_t lim;
   struct M10 * anonym;
   { /* with */
      struct M10 * anonym = &chan[m].m10;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),
                16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demod10(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk10() */

/*---------------------- M10 */
#define sondeudp_CRCINIT 0x0FFFFFFFF

#define sondeudp_WBSTART 0xA5 

#define sondeudp_WWSTART 0x0CA55F047

#define sondeudp_WBCRC 0x7F 


static void DecWater(uint32_t c, char bit)
{
   uint32_t crc;
   uint32_t ww;
   uint32_t i;
   /*WrStr("(");WrInt(ORD(bit), 1);WrStr(")"); */
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[c];
      anonym->waterbyte = anonym->waterbyte/2U; /* shift in 40 bits */
      if ((anonym->waterword&1)) anonym->waterbyte += 128U;
      anonym->waterword = anonym->waterword/2UL;
      if (bit) anonym->waterword += 0x080000000UL;
      if (anonym->waterbyte==165U && anonym->waterword==0x0CA55F047UL) {
         anonym->waterbits = 41UL; /* frame start found */
      }
      else if (anonym->waterbits>40UL) {
         /* in frame */
         if (anonym->waterbits%40UL==0UL) {
            /* byte + word ready */
            ww = anonym->waterbits/40UL-2UL;
            if (anonym->waterbyte==127U) {
               /* crc is end if frame */
               crc = X2C_max_longcard;
               i = 0UL;
               while (i<ww) {
                  crc32(&crc, anonym->watertemp[i].cmd);
                  crc32(&crc, (uint8_t)anonym->watertemp[i].dat);
                  crc32(&crc, (uint8_t)(anonym->watertemp[i].dat/256UL));
                  crc32(&crc, (uint8_t)(anonym->watertemp[i].dat/65536UL));
                  crc32(&crc, (uint8_t)(anonym->watertemp[i].dat/16777216UL));
                  ++i;
               }
               if (ww<=3UL) anonym->watertemp[ww].cmd = 0;
               anonym->waterbits = 0UL;
               if (crc==anonym->waterword) {
                  memcpy(anonym->waterdata,anonym->watertemp,
                sizeof(struct _0 [4]));
                  anonym->waterok = 1;
                  anonym->watersend = osic_time();
               }
            }
            else if (ww<=3UL) {
               /* store byte + word */
               anonym->watertemp[ww].cmd = anonym->waterbyte;
               anonym->watertemp[ww].dat = anonym->waterword;
            }
         }
         ++anonym->waterbits;
      }
   }
} /* end DecWater() */


static void getadc(void)
{
   int32_t pp;
   int32_t sl;
   int32_t l;
   uint32_t ch;
   uint32_t c;
   struct CHAN * anonym;
   uint32_t tmp;
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
               usleep(100000UL);
               OpenSound();
               return;
            }
         }
         if (l<2L) return;
         adcbufsamps = (uint32_t)(l/2L);
         if (debfd>=0L) {
            osi_WrBin(debfd, (char *)adcbuf, 8192u/1u, adcbufsamps*2UL);
         }
         /*      FOR ch:=0 TO maxchannels DO */
         /*        chan[ch].adcmax:=chan[ch].adcmax*15 DIV 16; */
         /*        chan[ch].adcmax:=chan[ch].adcmax*15 DIV 16; */
         /*        INC(chan[ch].adcdc,
                ASH((admax[ch]+admin[ch]) DIV 2-chan[ch].adcdc, -4)); */
         /*        INC(chan[ch].adcmax,
                ASH(admax[ch]-admin[ch]-chan[ch].adcmax, -4)); */
         /*        admax[ch]:=MIN(ADCWORD); */
         /*        admin[ch]:=MAX(ADCWORD); */
         /*      END; */
         adcbufsampx = X2C_max_longcard;
      }
      sl = (int32_t)adcbuf[adcbufrd];
      if (cfgchannels==0UL && (sl&1)) {
         /* auto channels channel 0 */
         /*WrInt(lastc, 1); WrStrLn(" ch1"); */
         if (adcbufsampx!=X2C_max_longcard) {
            ch = (adcbufrd-adcbufsampx)-1UL;
            /*WrInt(ch, 1); WrStrLn(" ch"); */
            if (ch<63UL) {
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
      sl = (int32_t)((uint32_t)sl&0xFFFFFFFEUL);
      if (sl==0L) ++chan[c].squelch;
      else chan[c].squelch = 0UL;
      /*    DEC(sl, chan[c].adcdc); */
      ++adcbufrd;
      /*    chan[c].afir[afin]:=VAL(REAL, sl-chan[c].adcdc); */
      /*    chan[c].afir[afin]:=VAL(REAL, sl); */
      chan[c].afir[afin] = (float)(sl-(chan[c].admax+chan[c].admin)/2L);
      if (waterenabled) DecWater(c, (0x2UL & (uint32_t)sl)!=0);
      if (sl>chan[c].admax) chan[c].admax = sl;
      if (sl<chan[c].admin) chan[c].admin = sl;
      pp = (chan[c].admax-chan[c].admin)/256L;
      chan[c].admax = chan[c].admax-pp;
      chan[c].admin = chan[c].admin+pp;
      ++c;
   } while (c<=maxchannels);
   afin = afin+1UL&31UL;
   tmp = maxchannels;
   c = 0UL;
   if (c<=tmp) for (;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         if (anonym->squelch<64UL) {
            /* squelch open */
            if (anonym->r92.enabled || anonym->r41.enabled) Fsk(c);
            if (anonym->c34.enabled) Afsk(c);
            if (anonym->dfm6.enabled) Fsk6(c);
            if (anonym->m10.enabled) {
               Fsk10(c);
            }
         }
      }
      if (c==tmp) break;
   } /* end for */
} /* end getadc() */

static uint16_t sondeudp_POLY0 = 0x1021U;


static void Gencrctab(void)
{
   uint16_t j;
   uint16_t i;
   uint16_t crc;
   for (i = 0U; i<=255U; i++) {
      crc = (uint16_t)(i*256U);
      for (j = 0U; j<=7U; j++) {
         if ((0x8000U & crc)) crc = X2C_LSH(crc,16,1)^0x1021U;
         else crc = X2C_LSH(crc,16,1);
      } /* end for */
      CRCTAB[i] = X2C_LSH(crc,16,-8)|X2C_LSH(crc,16,8);
   } /* end for */
} /* end Gencrctab() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(CNAMESTR)!=9) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   Gencrctab();
   GenCRC32tab();
   memset((char *)chan,(char)0,sizeof(struct CHAN [64]));
   Parms();
   initrsc();
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
