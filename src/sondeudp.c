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
#include <math.h>
#ifndef rsc_H_
#include "rsc.h"
#endif

/* demodulate RS92 sonde (2400bit/s manchester)
   and SRS-C34-C50 (2400Bd AFSK 2000/3800Hz
   and DFM (1250bit/s manchester)
   and RS41 (2400Bd GFSK)
   and M10 M20 (4800 bit/s manchester)
   and iMET (1200Bd AFSK 2200/1200Hz)
   and MRZ (1200 manchester) and send as AXUDP by OE5DXL */
/*FROM stat IMPORT fstat, stat_t; */
/*FROM flush IMPORT Flush; */
/*IMPORT reedsolomon; */
/* link init_rs_char.o decode_rs_char.o */
/* gcc  -o sondeudp Lib.o aprsstr.o filesize.o flush.o osi.o ptty.o rsc.o sondeudp.o soundctl.o symlink.o tcp.o timec.o udp.o init_rs_char.o decode_rs_char.o /usr/local/xds/lib/x86/libts.a /usr/local/xds/lib/x86/libxds.a -lm -lrt */
#define sondeudp_VERSION "1.37"

#define sondeudp_GPSTIMECORR 18
/* leap seconds */

#define sondeudp_MAXCHAN 64

#define sondeudp_MONTIME 10
/* seconds till next monitor udp send per channel */

#define sondeudp_DAYSEC 86400

#define sondeudp_CONTEXTLIFE 1800
/* seconds till forget context after last heared */

#define sondeudp_MAXACTIVE 300
/* limit active time of demodulator */

#define sondeudp_ADCBYTES 2

#define sondeudp_MAXLEN 9
/* data frame size c34 */

#define sondeudp_ADCBUFLEN 4096

#define sondeudp_BAUDSAMP 65536

#define sondeudp_PLLSHIFT 1024

#define sondeudp_RAD 1.7453292519943E-2

#define sondeudp_FRAMELIFETIME 3
/* max seconds from frame sync to sending frame */

#define sondeudp_WATERLEN 5

#define sondeudp_cFREQ "f"
/* watermark type freq */

#define sondeudp_cRAPPORT "r"
/* watermark type dB */

#define sondeudp_cAFC "a"
/* watermark type afc */

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

#define sondeudp_FLEN20 88
/* M20 framelen */

#define sondeudp_FLENC34 22
/* c34 frame len */

#define sondeudp_FLENIMET 106
/* IMET frame length */

#define sondeudp_FLENMP3 49
/* MP3H frame length */

#define sondeudp_FLENMEISEI 27
/* MEISEI header + bytes after fec */

#define sondeudp_M10SYN 0x649F20 
/* M10 sync */

#define sondeudp_M10SYN1 0x644900 

#define sondeudp_M20ASYN 0x452000 
/* M20 sync */

#define sondeudp_MP3COPIES 6

#define sondeudp_FLENMEISEIRAW 276
/* MEISEI bits per frame */

/*11111000 01000101 00100000 0111xxxx syn m20 */
/*11111000 01100100 01001001 0000xxxx -syn1-  */
/*11111000 01100100 10011111 00100000 -syn-  */
/*11111000 01000101 00100000 11000000 */
/*11111000 01000101 00100000 01111001 */
/*11111000 01000101 00100000 10000100 */
/*11111000 01000101 00100000 10001011 */
/*11111000 01000101 00100000 11000000 */
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

typedef uint32_t SET256[8];

typedef char FILENAME[1024];

typedef char CNAMESTR[9];

typedef float AFIRTAB[512];

typedef float DFIRTAB[1024];

typedef float DFIR[64];

struct DFNAMES;


struct DFNAMES {
   uint8_t start;
   uint16_t dat[2];
   uint8_t cnt[2];
};

struct UDPTX;

typedef struct UDPTX * pUDPTX;


struct UDPTX {
   pUDPTX next;
   uint32_t ip;
   uint32_t destport;
   int32_t udpfd;
};

enum ENABLE {sondeudp_OFF, sondeudp_ALWAYS, sondeudp_SLEEP};


struct R92;


struct R92 {
   uint8_t enabled;
   int32_t savecnt;
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
   uint8_t enabled;
   int32_t savecnt;
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
   char frametimeok;
   CNAMESTR id9;
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
   /* new df serial */
   uint32_t lastfrid;
   uint32_t lastfrcnt;
   uint32_t nameregok;
   uint32_t nameregtop;
   struct DFNAMES namereg[50];
   /* new df serial */
   char wasdate;
   char txok;
   char d9;
   uint32_t synword;
   char cb[56];
   char ch[56];
   char db1[104];
   char db2[104];
   char dh1[104];
   char dh2[104];
};

struct DFM6A;


struct DFM6A {
   uint8_t enabled;
   int32_t savecnt;
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
   uint32_t rxp;
   char rxbuf[264];
   AFIRTAB afirtab;
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct IMET;


struct IMET {
   /* check name */
   uint32_t idcnt;
   uint32_t idnew;
   float lastlat;
   float lastlong;
   uint32_t idtime;
};
/* check name */

struct IMETA;


struct IMETA {
   uint8_t enabled;
   int32_t savecnt;
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
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   uint32_t stopc;
   char rxbuf[106];
   AFIRTAB afirtab;
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
};

struct M1020;


struct M1020 {
   uint8_t enabled;
   int32_t savecnt;
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
   char ism20;
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
   struct SCID id34;
   struct SCID id50;
   uint32_t idtime;
   uint32_t idcnt;
   uint32_t tused;
   char c50;
};

struct C34A;


struct C34A {
   uint8_t enabled;
   int32_t savecnt;
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
};

typedef char MP3BUF[50];

struct MP3FRAME;


struct MP3FRAME {
   MP3BUF dat;
   uint32_t time0;
};

struct MP3H;


struct MP3H {
   uint8_t enabled;
   int32_t savecnt;
   int32_t pllshift;
   int32_t baudfine;
   int32_t manchestd;
   float noise;
   float bitlev;
   float lastu;
   char dmeis;
   char id1ok;
   char id2ok;
   char dateok;
   char cbit;
   char oldd;
   char polarity;
   char plld;
   char lastmanch;
   uint32_t gpsdate;
   uint32_t timeatdate;
   uint32_t gpstime;
   uint32_t blocktime;
   uint32_t id1;
   uint32_t id2;
   uint32_t blocknum;
   uint32_t synword;
   uint32_t copycnt;
   uint32_t rxb;
   uint32_t rxp;
   struct MP3FRAME copybuf[6];
   AFIRTAB afirtab;
   uint32_t demodbaud;
   uint32_t configbaud;
};

struct MEISEI;

struct _0;


struct _0 {
   uint32_t d;
   uint32_t t;
};


struct MEISEI {
   uint8_t enabled;
   char timeok;
   uint32_t gpsdaytime;
   uint32_t subtype;
   uint32_t gpssum;
   uint32_t gpstime;
   uint32_t frametyp;
   uint32_t lostsamps;
   uint32_t synword;
   uint32_t lastser;
   uint32_t rxb;
   char rxbuf[276];
   char ser[11];
   struct _0 config[128];
};

struct NONAMES;

typedef struct NONAMES * pNONAMES;


struct NONAMES {
   pNONAMES next;
   uint32_t lastvalid;
   uint32_t chname;
   struct DFM6 dfm6;
   struct C34 c34;
   struct IMET imet;
};

struct CHAN;

struct _1;


struct _1 {
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
   struct _1 waterdata[5];
   struct _1 watertemp[5];
   uint32_t watersend;
   char waterok;
   uint32_t chlabel;
   uint32_t lastmon;
   struct R92 r92;
   struct R41 r41;
   struct M1020 m10;
   struct C34A c34a;
   struct DFM6A dfm6a;
   struct IMETA imeta;
   struct MP3H mp3h;
   struct MEISEI meisei;
   pNONAMES nonames;
};

static int32_t soundfd;

static int32_t debfd;

static char dfmswap;

static char dfmoldname;

static char abortonsounderr;

static char waterenabled;

static char nosendsdr;

static char verb;

static char verb2;

static uint32_t dfmidoldthreshold;

static uint32_t dfmidchgthreshold;

static uint32_t dfmidthreshold;

static uint32_t getst;

static uint32_t afin;

static uint32_t soundbufs;

static uint32_t adcrate;

static uint32_t adcbuflen;

static uint32_t adcbufrd;

static uint32_t adcbufsamps;

static uint32_t savelevel;

static uint32_t sampcount;

static uint32_t fragmentsize;

static FILENAME soundfn;

static struct CHAN chan[64];

static uint32_t adcbufsampx;

static uint32_t maxchannels;

static uint32_t cfgchannels;

static pUDPTX monitorudp;

static short adcbuf[4096];

static uint32_t dfmnametyp;

static char rxlabel[5];

static pNONAMES oldnonames;

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
      f[i] = (float)((double)f[i]*(0.54+0.46*cos((double)
                (3.1415926535898f*(X2C_DIVR((float)i,
                (float)(1UL+(f_len-1))))))));
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
         t[i] = (float)((double)t[i]+(double)
                e*cos((double)(X2C_DIVR(3.1415926535898f*(float)
                (i*f),512.0f))));
      } /* end for */
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 512ul);
   for (i = 0UL; i<=511UL; i++) {
      t[i] = (float)((double)t[i]*(0.54+0.46*cos((double)
                (3.1415926535898f*(X2C_DIVR((float)i,512.0f))))));
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
            t[i] = (float)((double)t[i]+(double)
                e*cos((double)(X2C_DIVR(3.1415926535898f*(float)
                (i*f),256.0f))));
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
         if (samplesize(soundfd, 16UL)<0L) {
            /* 8, 16 */
            if (!osi_IsFifo(soundfd)) {
               Error("sound must be pipe or oss", 26ul);
            }
         }
         else {
            i = channels(soundfd, maxchannels+1UL); /* 1, 2  */
            i = setfragment(soundfd, fragmentsize);
                /* 2^bufsize * 65536*bufs*/
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
   struct DFM6A * anonym1;
   struct M1020 * anonym2;
   struct C34A * anonym3;
   struct IMETA * anonym4;
   struct MP3H * anonym5;
   struct MEISEI * anonym6;
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
         struct DFM6A * anonym1 = &chan[c].dfm6a;
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
      }
      { /* with */
         struct M1020 * anonym2 = &chan[c].m10;
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
         struct C34A * anonym3 = &chan[c].c34a;
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
      { /* with */
         struct IMETA * anonym4 = &chan[c].imeta;
         anonym4->txbaud = (anonym4->configbaud*65536UL)/adcrate;
         anonym4->demodbaud = anonym4->txbaud*2UL;
         anonym4->afskmidfreq = X2C_DIVR((float)
                anonym4->configafskmid*2.0f,(float)adcrate);
         initafir(anonym4->afirtab,
                (anonym4->configafskmid-anonym4->configafskshift/2UL)
                -anonym4->configbaud/4UL,
                anonym4->configafskmid+anonym4->configafskshift/2UL+anonym4->configbaud/4UL,
                 X2C_DIVR((float)chan[c].configequalizer,100.0f));
         initdfir(anonym4->dfirtab,
                (anonym4->configbaud*anonym4->confignyquist)/100UL);
         anonym4->baudfine = 0L;
         anonym4->left = 0.0f;
         anonym4->tcnt = 0.0f;
         anonym4->freq = 0.0f;
         anonym4->dfin = 0UL;
         anonym4->cbit = 0;
         anonym4->rxp = 0UL;
         anonym4->rxbitc = 0UL;
      }
      { /* with */
         struct MP3H * anonym5 = &chan[c].mp3h;
         anonym5->configbaud = 2400UL;
         anonym5->demodbaud = (2UL*anonym5->configbaud*65536UL)/adcrate;
         initafir(anonym5->afirtab, 0UL, 1900UL,
                X2C_DIVR((float)chan[c].configequalizer,100.0f));
         anonym5->baudfine = 0L;
         anonym5->noise = 0.0f;
         anonym5->bitlev = 0.0f;
         anonym5->cbit = 0;
         anonym5->polarity = 0;
         anonym5->copycnt = 0UL;
         anonym5->rxp = 49UL; /* out of frame, wait for sync */
         anonym5->manchestd = 0L;
      }
      { /* with */
         struct MEISEI * anonym6 = &chan[c].meisei;
         anonym6->rxb = 276UL; /* out of frame, wait for sync */
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
   uint8_t enab;
   struct R92 * anonym;
   struct R41 * anonym0;
   struct DFM6A * anonym1;
   struct C34A * anonym2;
   struct IMETA * anonym3;
   struct M1020 * anonym4;
   struct MP3H * anonym5;
   struct MEISEI * anonym6;
   struct CHAN * anonym7;
   /* set only 1 chan */
   struct CHAN * anonym8;
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
   dfmnametyp = 512UL;
   dfmidoldthreshold = 1UL;
   dfmidchgthreshold = 3UL;
   dfmidthreshold = 1UL;
   rxlabel[0] = 0;
   dfmswap = 0;
   dfmoldname = 0;
   savelevel = 0UL;
   for (channel = 0UL; channel<=63UL; channel++) {
      { /* with */
         struct R92 * anonym = &chan[channel].r92;
         anonym->enabled = sondeudp_SLEEP;
         anonym->pllshift = 1024L;
      }
      { /* with */
         struct R41 * anonym0 = &chan[channel].r41;
         anonym0->enabled = sondeudp_SLEEP;
         anonym0->pllshift = 1024L;
      }
      { /* with */
         struct DFM6A * anonym1 = &chan[channel].dfm6a;
         anonym1->enabled = sondeudp_SLEEP;
         anonym1->pllshift = 1024L;
      }
      { /* with */
         struct C34A * anonym2 = &chan[channel].c34a;
         anonym2->enabled = sondeudp_SLEEP;
         anonym2->pllshift = 14336L;
         anonym2->confignyquist = 65UL;
         anonym2->afskhighpass = 0.0f;
         anonym2->configbaud = 2400UL;
         anonym2->configafskshift = 1800UL;
         anonym2->configafskmid = 3800UL;
      }
      { /* with */
         struct IMETA * anonym3 = &chan[channel].imeta;
         anonym3->enabled = sondeudp_SLEEP;
         anonym3->pllshift = 3072L;
         anonym3->confignyquist = 65UL;
         anonym3->afskhighpass = 0.0f;
         anonym3->configbaud = 1200UL;
         anonym3->configafskshift = 1000UL;
         anonym3->configafskmid = 1700UL;
      }
      { /* with */
         struct M1020 * anonym4 = &chan[channel].m10;
         anonym4->enabled = sondeudp_SLEEP;
         anonym4->pllshift = 4096L;
      }
      { /* with */
         struct MP3H * anonym5 = &chan[channel].mp3h;
         anonym5->enabled = sondeudp_SLEEP;
         anonym5->pllshift = 1024L;
      }
      { /* with */
         struct MEISEI * anonym6 = &chan[channel].meisei;
         anonym6->enabled = sondeudp_SLEEP;
      }
      { /* with */
         struct CHAN * anonym7 = &chan[channel];
         anonym7->configequalizer = 0L;
         anonym7->udptx = 0;
         anonym7->mycallc = 0UL;
      }
   } /* end for */
   channel = 0UL;
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if (((h[0U]=='-' || h[0U]=='+') && h[1U]) && h[2U]==0) {
         if (h[0U]=='+') enab = sondeudp_ALWAYS;
         else enab = sondeudp_OFF;
         if (h[1U]=='a') abortonsounderr = 1;
         else if (h[1U]=='2') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].mp3h.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].mp3h.enabled = enab;
               } /* end for */
            }
         }
         else if (h[1U]=='3') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].c34a.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].c34a.enabled = enab;
               } /* end for */
            }
         }
         else if (h[1U]=='9') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r92.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].r92.enabled = enab;
               } /* end for */
            }
         }
         else if (h[1U]=='4') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].r41.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].r41.enabled = enab;
               } /* end for */
            }
         }
         else if (h[1U]=='6') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].dfm6a.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].dfm6a.enabled = enab;
               } /* end for */
            }
         }
         else if (h[1U]=='1') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].m10.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].m10.enabled = enab;
               } /* end for */
            }
         }
         else if (h[1U]=='8') {
            if (chanset) {
               /* set only 1 chan */
               chan[channel].imeta.enabled = enab;
            }
            else {
               /* use before -C set both */
               for (ch = 0UL; ch<=63UL; ch++) {
                  chan[ch].imeta.enabled = enab;
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
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) {
               err = 1;
            }
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
            chan[channel].c34a.configafskmid = cnum;
         }
         else if (h[1U]=='G') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmidchgthreshold)) err = 1;
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmidoldthreshold)) err = 1;
         }
         else if (h[1U]=='S') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &dfmidthreshold)) err = 1;
         }
         else if (h[1U]=='W') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &savelevel)) err = 1;
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
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
                  struct CHAN * anonym8 = &chan[channel];
                  anonym8->mycallc = myc;
                  anonym8->myssid = mys;
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
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            osic_alloc((char * *) &utx, sizeof(struct UDPTX));
            if (utx==0) Error("udp socket out of memory", 25ul);
            utx->udpfd = GetIp(h, 1024ul, &utx->ip, &utx->destport);
            if (utx->udpfd<0L) Error("cannot open udp socket", 23ul);
            utx->next = monitorudp;
            monitorudp = utx;
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='s') nosendsdr = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h[1U]=='J') dfmswap = 1;
         else if (h[1U]=='O') dfmoldname = 1;
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("Mono/Stereo up to 64 Channel RS92, RS41, C34, C50\
, DFM, IMET, M10, M20, MRZ, MEISEI Sonde Demodulator", 102ul);
               osi_WrStrLn("to raw Frames sent via UDP to \'sondemod\' decode\
r or watch with -v V:1.37", 73ul);
               osi_WrStrLn("more demodulators may send to same \'sondemod\'",
                 46ul);
               osi_WrStrLn("Stereo used for 2 Rx for 2 Sondes or 1 Sonde with\
 Antenna-Diversity", 68ul);
               osi_WrStrLn(" Switch off not needed decoders to save CPU",
                44ul);
               osi_WrStrLn(" -1             disable M10,M20 decoding (use -C \
before to select a channel)", 77ul);
               osi_WrStrLn(" -2             disable MRZ,Meisei decoding (use \
-C before to select a channel)", 80ul);
               osi_WrStrLn(" -3             disable SRSC34/50 decoding (use -\
C before to select a channel)", 79ul);
               osi_WrStrLn(" -4             disable RS41 decoding (use -C bef\
ore to select a channel)", 74ul);
               osi_WrStrLn(" -6             disable DFM decoding (use -C befo\
re to select a channel)", 73ul);
               osi_WrStrLn(" -8             disable IMET decoding (use -C bef\
ore to select a channel)", 74ul);
               osi_WrStrLn(" -9             disable RS92 decoding (use -C bef\
ore to select a channel)", 74ul);
               osi_WrStrLn(" +<typenum>     exclude from sleep if -W <s> set \
eg. \"+4\" no sleep RS41", 72ul);
               osi_WrStrLn(" -a             abort on sounddevice error else r\
etry to open (USB audio, pipe)", 80ul);
               osi_WrStrLn(" -c <num>       maxchannels, 0 for automatic chan\
nel number recognition from sdrtst", 84ul);
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
               osi_WrStrLn(" -g <minutes>   DFMxx (with no serial number) sub\
stitute name stability check before tx (1)", 92ul);
               osi_WrStrLn("                default set to 3 on automatic ser\
ial number search \"-n 0\"", 74ul);
               osi_WrStrLn(" -G <minutes>   no tx if DFMxx substitute Name ch\
anges (3)", 59ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <call>      mycall + ssid (use -C before to s\
elect 1 channel) else sondemod sets call", 90ul);
               osi_WrStrLn(" -l <num>       sound buffer length (256)",
                42ul);
               osi_WrStrLn(" -L <name>      Label of device sent to sondemod,\
 max 4 char", 61ul);
               osi_WrStrLn(" -M <x.x.x.x:destport> Send (human readable) UDP \
info about decoded data to Scanner", 84ul);
               osi_WrStrLn("                  (or netcat) to weed out birdies\
, maybe repeated for more destinations", 88ul);
               osi_WrStrLn(" -N <num>       1..255 generate DFM-ID from seria\
l no. (see -V) (off)", 70ul);
               osi_WrStrLn("                num is start byte of frame with s\
erial no. in decimal \"AC00070\" -N 172", 87ul);
               osi_WrStrLn("                0 automatic search serial number \
(default), increase -S for more reliability", 93ul);
               osi_WrStrLn(" -n <num>       same as -N but send substitute na\
me if no serial number found in \"-g\" min", 90ul);
               osi_WrStrLn(" -O             DFM send \"DF6...\" with hex numb\
er else \"D...\" with decimal number", 82ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or raw/\
wav audio file or pipe /dev/stdin", 83ul);
               osi_WrStrLn(" -s             disable sending sdr-data (freq/af\
c/rssi/label) to (old version) sondemod", 89ul);
               osi_WrStrLn(" -S <retries>   check DFM serial number for stabi\
lity before fixing name (1)", 77ul);
               osi_WrStrLn("                on automatic search, minimum and \
default is 2", 62ul);
               osi_WrStrLn(" -u <x.x.x.x:destport> send rx data in udp (to so\
ndemod), -C <n> before sets", 77ul);
               osi_WrStrLn("                channel number, maybe repeated fo\
r more destinations", 69ul);
               osi_WrStrLn(" -V             very verbous, with some hex dumps\
", 50ul);
               osi_WrStrLn(" -v             verbous, (frames with Name looks \
ok)", 53ul);
               osi_WrStrLn(" -W <s>         cyclic sleep <seconds> inactive d\
emodulators to save CPU", 73ul);
               osi_WrStrLn("                  wakes 1..2s (dep. on type) and \
stay long awake if found fitting pattern", 90ul);
               osi_WrStrLn("                  do not use in frequency-hopping\
 environment", 62ul);
               osi_WrStrLn("example: sondeudp -f 16000 -o /dev/dsp -c 2 -C 0 \
-e 50 -u 127.0.0.1:4000 -v", 76ul);
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
   if (dfmnametyp==256UL && dfmidoldthreshold<2UL) dfmidoldthreshold = 2UL;
   if ((dfmnametyp&255UL)==0UL && dfmidthreshold<2UL) dfmidthreshold = 2UL;
   if (dfmidthreshold==0UL) dfmidthreshold = 1UL;
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


static void WrChName(uint32_t n)
{
   /*
     WrStr(CHR(ASH(n,-24)));
     WrStr(CHR(ASH(n,-16) MOD 256));
     WrStr(CHR(ASH(n,-8) MOD 256));
     WrStr(CHR(n MOD 256));
   */
   osic_WrINT32(n, 1UL);
} /* end WrChName() */


static void appendsdr(uint32_t c)
{
   uint32_t i;
   int32_t w;
   int32_t v;
   struct _1 * anonym;
   if (chan[c].waterok) {
      i = 0UL;
      while (i<=4UL && chan[c].waterdata[i].cmd) {
         { /* with */
            struct _1 * anonym = &chan[c].waterdata[i];
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
         /*      ELSIF cmd=cCHNAME THEN WrStr(" $"); WrChName(dat);
                WrStr(" "); */
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
   struct _1 * anonym;
   if (!nosendsdr) {
      p = *len;
      t = osic_time();
      if ((chan[ch].watersend<=t && chan[ch].watersend+2UL>=t)
                && b_len-1>*len+19UL) {
         i = 0UL;
         while (i<=4UL && chan[ch].waterdata[i].cmd) {
            { /* with */
               struct _1 * anonym = &chan[ch].waterdata[i];
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
      osic_WrFixed((float)(log((double)volt)*8.685889638-96.4), 1L,
                6UL);
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


static uint32_t getfreq(uint32_t ch)
/* in 10hz */
{
   uint32_t f;
   uint32_t i;
   struct _1 * anonym;
   i = 0UL;
   f = 0UL;
   while (i<=4UL && chan[ch].waterdata[i].cmd) {
      { /* with */
         struct _1 * anonym = &chan[ch].waterdata[i];
         if (anonym->cmd=='f') f += anonym->dat;
         else if (anonym->cmd=='a') {
            f += (uint32_t)((int32_t)(short)(anonym->dat&65535UL)
                *100L);
         }
      }
      ++i;
   }
   return f;
} /* end getfreq() */


static void monitor(uint32_t ch, const char typ[], uint32_t typ_len,
                const char id[], uint32_t id_len)
{
   pUDPTX u;
   uint32_t t;
   uint32_t hz10;
   char h[201];
   char s[201];
   if (monitorudp) {
      hz10 = getfreq(ch);
      if (hz10==0UL) hz10 = ch;
      /*  IF hz10<>0 THEN */
      t = osic_time();
      if (chan[ch].lastmon+10UL<t) {
         chan[ch].lastmon = t;
         strncpy(s,"RX",201u);
         aprsstr_CardToStr(hz10, 1UL, h, 201ul);
         aprsstr_Append(s, 201ul, h, 201ul);
         aprsstr_Append(s, 201ul, ",", 2ul);
         aprsstr_Append(s, 201ul, typ, typ_len);
         if (id[0UL]) {
            aprsstr_Append(s, 201ul, ",", 2ul);
            aprsstr_Append(s, 201ul, id, id_len);
         }
         aprsstr_Append(s, 201ul, "\012", 2ul);
         u = monitorudp;
         do {
            if (u->udpfd>=0L) {
               sendudp(s, 201ul, (int32_t)aprsstr_Length(s, 201ul), u->ip,
                u->destport, u->udpfd);
            }
            u = u->next;
         } while (u);
      }
   }
/*  END; */
} /* end monitor() */


static uint32_t cardmsb(const char b[], uint32_t b_len,
                uint32_t pos, uint32_t len)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = len-1UL;; i--) {
      n = n*256UL+(uint32_t)(uint8_t)b[pos+i];
      if (i==0UL) break;
   } /* end for */
   return n;
} /* end cardmsb() */


static double atang2(double x, double y)
{
   double w;
   if (fabs(x)>fabs(y)) {
      w = atan(X2C_DIVL(y,x));
      if (x<0.0) {
         if (y>0.0) w = 3.1415926535898+w;
         else w = w-3.1415926535898;
      }
   }
   else if (y!=0.0) {
      w = 1.5707963267949-atan(X2C_DIVL(x,y));
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
      rh = sqrt(h);
      xh = x+rh;
      *long0 = atang2(xh, y)*2.0;
      if (*long0>3.1415926535898) *long0 = *long0-6.2831853071796;
      t = atan(X2C_DIVL(z*1.003364089821,rh));
      st = sin(t);
      ct = cos(t);
      *lat = atan(X2C_DIVL(z+4.2841311513312E+4*st*st*st,
                rh-4.269767270718E+4*ct*ct*ct));
      sl = sin(*lat);
      *heig = X2C_DIVL(rh,cos(*lat))-X2C_DIVL(6.378137E+6,
                sqrt(1.0-6.6943799901413E-3*sl*sl));
   }
   else {
      *lat = 0.0;
      *long0 = 0.0;
      *heig = 0.0;
   }
/*  lat:=atan(z/(rh*(1.0 - E2))); */
/*  heig:=sqrt(h + z*z) - EARTHA; */
} /* end wgs84r() */


static void speeddir(double lat, double long0, double vx,
                double vy, double vz, double * kmh,
                double * dir, double * clb)
{
   double coslong;
   double sinlong;
   double coslat;
   double sinlat;
   double ve;
   double vn;
   sinlat = sin(lat);
   coslat = cos(lat);
   sinlong = sin(long0);
   coslong = cos(long0);
   vn = (-(vx*sinlat*coslong)-vy*sinlat*sinlong)+vz*coslat;
   ve = -(vx*sinlong)+vy*coslong;
   *clb = vx*coslat*coslong+vy*coslat*sinlong+vz*sinlat;
   *dir = X2C_DIVL(atang2(vn, ve),1.7453292519943E-2);
   if (*dir<0.0) *dir = 360.0+*dir;
   *kmh = sqrt(vn*vn+ve*ve)*3.6;
} /* end speeddir() */


static void setactiv(int32_t * savecnt, int32_t t)
{
   if (*savecnt<0L) *savecnt = 0L;
   if (*savecnt<300L) *savecnt += t;
} /* end setactiv() */


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
   char id[8];
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
   id[0U] = 0;
   p = 6UL;
   if (chan[m].r92.rxbuf[6U]=='e') {
      ++p;
      len = (uint32_t)(uint8_t)chan[m].r92.rxbuf[7U]*2UL+2UL; /* +crc */
      ++p;
      if (verb) {
         WrChan((int32_t)m);
         osi_WrStr("R92 ", 5ul);
      }
      if (8UL+len>240UL || !crcrs(chan[m].r92.rxbuf, 301ul, 8L,
                (int32_t)(8UL+len))) {
         if (verb) osi_WrStr("----  crc err ", 15ul);
      }
      else {
         j = 0UL;
         while ((uint8_t)chan[m].r92.rxbuf[8UL+j+4UL]>=' ' && j<=7UL) {
            id[j] = chan[m].r92.rxbuf[8UL+j+4UL];
            ++j;
         }
         /*      j:=4; */
         /*      WHILE chan[m].r92.rxbuf[p+j]>=" " DO WrStr(chan[m]
                .r92.rxbuf[p+j]); INC(j); END; */
         if (verb) {
            osi_WrStr(id, 8ul);
            osi_WrStr(" ", 2ul);
            osic_WrINT32((uint32_t)(uint8_t)
                chan[m].r92.rxbuf[8U]+(uint32_t)(uint8_t)
                chan[m].r92.rxbuf[9U]*256UL, 4UL);
         }
         setactiv(&chan[m].r92.savecnt, 60L);
      }
      /*      IF m>0 THEN WrStr("             ") END; */
      if (verb) {
         WrdB(chan[m].admax-chan[m].admin);
         WrQ(chan[m].r92.bitlev, chan[m].r92.noise);
         if (corr<0L) osi_WrStr(" -R", 4ul);
         else if (corr>0L && corr<=12L) {
            osi_WrStr(" +", 3ul);
            osic_WrINT32((uint32_t)corr, 1UL);
            osi_WrStr("R", 2ul);
         }
         /*      Wrtune(chan[m].admax+chan[m].admin,
                chan[m].admax-chan[m].admin); */
         appendsdr(m);
         osi_WrStrLn("", 1ul);
      }
      monitor(m, "RS92", 5ul, id, 8ul);
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
   double clb;
   double kmh;
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
   speeddir(lat, long0, vx, vy, vz, &kmh, &dir, &clb);
   osi_WrStr(" ", 2ul);
   osic_WrFixed((float)kmh, 1L, 1UL);
   osi_WrStr("km/h ", 6ul);
   osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(dir,X2C_min_longint,
                X2C_max_longint), 1UL);
   osi_WrStr("deg ", 5ul);
   osic_WrFixed((float)clb, 1L, 1UL);
   osi_WrStr("m/s ", 5ul);
   osic_WrINT32((uint32_t)(uint8_t)b[p+18UL], 1UL);
   osi_WrStr("Sats ", 6ul);
/*  WrFixed(FLOAT(ORD(b[p+20]))*0.1, 1, 1); WrStr("dop"); */
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

static uint8_t sondeudp_POWERTAB[8] = {1U,2U,5U,8U,11U,14U,17U,20U};

static uint8_t _cnst[8] = {1U,2U,5U,8U,11U,14U,17U,20U};

static void decode41(uint32_t m)
{
   uint32_t date;
   uint32_t try0;
   uint32_t posok;
   uint32_t nameok;
   uint32_t len;
   uint32_t p;
   uint32_t i;
   char ch;
   char typ;
   char encr;
   char allok;
   int32_t repl;
   int32_t corr;
   uint32_t aux;
   char s[24];
   char monid[16];
   char ha[16];
   char id[8];
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
         encr = 0;
         aux = 0UL;
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
                  ch = anonym->rxbuf[p+i];
                  osi_WrHex((uint32_t)(uint8_t)ch, 3UL);
                  if ((uint8_t)ch>=' ' && (uint8_t)ch<='~') {
                     ha[i&15UL] = ch;
                  }
                  else ha[i&15UL] = '.';
                  if ((i&15UL)!=15UL) ha[i+1UL&15UL] = 0;
                  if (i==len-1UL || (i&15UL)==15UL) osi_WrStrLn(ha, 16ul);
                  if (i==tmp) break;
               } /* end for */
               osi_WrStrLn("", 1ul);
            }
            tmp = (p+len)-1UL;
            i = p-2UL;
            if (i<=tmp) for (;; i++) {
               /* update fixbyte statistics */
               if (anonym->fixbytes[i]==anonym->rxbuf[i]) {
                  if (anonym->fixcnt[i]<255U) ++anonym->fixcnt[i];
               }
               else {
                  anonym->fixbytes[i] = anonym->rxbuf[i];
                  anonym->fixcnt[i] = 0U;
               }
               if (i==tmp) break;
            } /* end for */
            if (typ=='y') {
               /* ID with correct crc */
               nameok = p;
               for (i = 0UL; i<=7UL; i++) {
                  id[i] = anonym->rxbuf[nameok+2UL+i];
                  if ((uint8_t)id[i]<=' ' || (uint8_t)id[i]>='\177') {
                     id[i] = '.';
                  }
               } /* end for */
               setactiv(&anonym->savecnt, 60L);
            }
            else if (typ=='|') {
               /*        ELSIF typ=CHR(7AH) THEN */
               /*             WrStrLn("7A ptu frame"); */
               date = (uint32_t)getint32(anonym->rxbuf, 560ul,
                p+2UL)/1000UL+86382UL+getcard16(anonym->rxbuf, 560ul,
                p)*604800UL+315878400UL;
            }
            else if (typ=='{') {
               /*           
                           WrStrLn("");
                           FOR i:=0 TO 11 DO
                             WrInt(ORD(rxbuf[p+6+i*2]), 1);
                   (* sat n *)
                             WrStr(":");
                             WrFixed(FLOAT(ORD(rxbuf[p+7+i*2]))*0.2, 1, 1);
                   (* signal ? *)
                             WrStr(" ");
                           END;
               */
               /*        WrStrLn("7C frame"); */
               /*WrInt(getint32(rxbuf, p+2) DIV 1000 MOD 86400 , 10);
                WrStr("=gpstime "); */
               /*        ELSIF typ=CHR(7DH) THEN */
               /*             WrStrLn("7D gps raw"); */
               posok = p;
            }
            else if (typ=='~') ++aux;
            else if (typ=='x') {
            }
            else if (typ=='\200') {
               /*        ELSIF typ=CHR(76H) THEN */
               /*            WrStrLn("76 fill frame"); */
               /*            IF verb THEN WrStr(" encrypted gps ") END; */
               /*        ELSIF typ=CHR(7FH) THEN */
               /*            WrStrLn("7F short ptu frame"); */
               encr = 1;
               if (verb) osi_WrStr(" encrypted ", 12ul);
            }
            /*        WrInt(getint16(rxbuf, 3BH), 0); */
            /*        WrStr(" ");WrHex(ORD(typ), 0);WrStr(" ");
                WrHex(p DIV 256, 0); WrHex(p, 0); */
            /*        WrStr(" "); */
            if (typ=='v') break;
            p += len;
         }
         ++try0;
      } while (!(allok || try0>2UL));
      if (verb && nameok>0UL) {
         WrChan((int32_t)m);
         osi_WrStr("R41 ", 5ul);
         osi_WrStr(id, 8ul);
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
         if (allok) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)(uint32_t)(uint8_t)
                anonym->rxbuf[nameok+10UL]*0.1f, 1L, 1UL); /* vbatt */
            osi_WrStr("V", 2ul);
         }
         i = (uint32_t)(uint8_t)anonym->rxbuf[nameok+21UL];
                /* txpow 0..7 */
         if (i<7UL) {
            osi_WrStr(" ", 2ul);
            osic_WrINT32((uint32_t)_cnst[i], 1UL);
            osi_WrStr("dBm", 4ul);
         }
         if (aux>0UL) {
            osi_WrStr(" ", 2ul);
            osic_WrINT32(aux, 1UL);
            osi_WrStr("xAux", 5ul);
         }
         if (encr) osi_WrStr(" encrypted part", 16ul);
         if (!((allok || posok>0UL) || aux>0UL)) {
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
         /*      Wrtune(chan[m].admax+chan[m].admin,
                chan[m].admax-chan[m].admin); */
         appendsdr(m);
         osi_WrStrLn("", 1ul);
      }
   }
   if (nameok>0UL) {
      sendrs41(m);
      monid[0] = 0;
      if (encr) {
         strncpy(monid,"#",16u);
      }
      aprsstr_Append(monid, 16ul, id, 8ul);
      monitor(m, "RS41", 5ul, monid, 16ul);
   }
} /* end decode41() */

#define sondeudp_MAXHEADERR 4

static uint8_t _cnst0[64] = {150U,131U,62U,81U,177U,73U,8U,152U,50U,5U,89U,
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
                anonym->rxbyte^(uint8_t)_cnst0[anonym->rxp&63UL]);
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
      if (anonym->manchestd<0L) {
         demodbyte92(m, d);
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)
                *0.02f;
      }
   }
/*quality*/
} /* end demodbit92() */


static void demod9241(float u, uint32_t m)
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
         if (anonym->savecnt>0L) demodbit92(m, u, anonym->lastu);
         if (chan[m].r41.savecnt>0L) demodbit41(m, u);
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
} /* end demod9241() */


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
            demod9241(ff, m);
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


static void killdfid(uint32_t m, char all)
/* reset dfm06 name */
{
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].nonames->dfm6;
      if (all) anonym->idcnt = 0UL;
      anonym->idcnt0 = 0UL;
      anonym->idcnt1 = 0UL;
      anonym->txok = 0;
      anonym->id9[0U] = 0;
      anonym->idnew = 0UL;
      anonym->frametimeok = 0;
      anonym->nameregtop = 0UL;
      anonym->nameregok = 0UL;
      anonym->lastfrid = 0UL;
      anonym->lastfrcnt = 0UL;
   }
} /* end killdfid() */


static void checkdfpos(float deg, float odeg, uint32_t m)
/* reset dfm06 name on pos jump */
{
   if ((deg!=0.0f && odeg!=0.0f) && (float)fabs(deg-odeg)>0.25f) {
      if (verb && chan[m].nonames->dfm6.id9[0U]) {
         osi_WrStr(" POSITION JUMP! TX STOP ", 25ul);
      }
      killdfid(m, 1);
   }
} /* end checkdfpos() */


static void checkdf69(float long0, uint32_t m)
{
   if (dfmswap) chan[m].nonames->dfm6.d9 = long0<30.0f;
   else chan[m].nonames->dfm6.d9 = 0;
} /* end checkdf69() */

static uint32_t sondeudp_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};

static uint32_t _cnst1[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,181UL,
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
   struct DFM6 * anonym;
   struct DFM6 * anonym0;
   struct DFM6 * anonym1;
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
      { /* with */
         struct DFM6 * anonym = &chan[m].nonames->dfm6;
         numcntok = 0;
         if ((anonym->num+1UL&255UL)==v && anonym->wasdate) {
            ++anonym->numcnt; /* date/num+1/date/num+1 sequence ok */
         }
         else anonym->numcnt = 0UL;
         anonym->num = v;
         anonym->wasdate = 0; /* we have number, next must be date */
         if (anonym->lastdate+60UL==anonym->actdate) {
            /* minute change */
            if (anonym->numcnt>=3UL) {
               anonym->timediff = anonym->actdate-osic_time();
                /* sonde realtime - systime */
               anonym->numcnt = 0UL;
               numcntok = 1;
            }
         }
         if (dfmnametyp<512UL && (dfmnametyp==0UL || (anonym->idcnt0<dfmidthreshold || anonym->idcnt1<dfmidthreshold)
                 && anonym->lastfrcnt<dfmidthreshold)) {
            /* id with old methode */
            u = (((v+256UL)-(anonym->idnum&255UL)&255UL)+256UL)-(osic_time()
                +anonym->timediff&255UL)&255UL;
            if (u>=128UL) u = 256UL-u;
            anonym->txok = u<=1UL; /* max +-1s tolerance of num */
            if (verb) {
               osi_WrStr(" [", 3ul);
               aprsstr_IntToStr((int32_t)u, 0UL, s, 101ul);
               if (anonym->txok) aprsstr_Append(s, 101ul, "s seq]", 7ul);
               else aprsstr_Append(s, 101ul, "s OUT OF seq - NO TX]", 22ul);
               osi_WrStr(s, 101ul);
            }
            if (numcntok) {
               v = (anonym->num+256UL)-(anonym->actdate&255UL)&255UL;
               if (verb) {
                  osi_WrStr(" NEW ID", 8ul);
                  osic_WrINT32(v, 0UL);
               }
               anonym->idnum = v;
               strncpy(s,"DFM6",101u); /* build a name */
               s[4U] = (char)((v/100UL)%10UL+48UL);
               s[5U] = (char)((v/10UL)%10UL+48UL);
               s[6U] = (char)(v%10UL+48UL);
               s[7U] = 0;
               if (aprsstr_StrCmp(anonym->idcheck, 9ul, s, 101ul)) {
                  ++anonym->idcnt; /* got same name again */
               }
               else {
                  /* new name so check if wrong */
                  aprsstr_Assign(anonym->idcheck, 9ul, s, 101ul);
                  anonym->idcnt = 1UL;
               }
               if (verb) {
                  osi_WrStr(" MATCHES:", 10ul);
                  osic_WrINT32(anonym->idcnt, 1UL);
               }
               if (anonym->idcnt>dfmidchgthreshold || anonym->id9[0U]
                ==0 && anonym->idcnt>dfmidoldthreshold) {
                  /* first name or safe new name */
                  memcpy(anonym->id9,anonym->idcheck,9u);
                  anonym->idtime = osic_time();
               }
            }
         }
      }
      if (chan[m].nonames->dfm6.d9) {
         /* dfm09 speed */
         v = bits2val(b, b_len, 32UL, 16UL);
         if (verb) {
            osic_WrFixed((float)v*0.036f, 1L, 0UL);
            osi_WrStr(" km/h", 6ul);
         }
      }
      break;
   case 1UL:
      { /* with */
         struct DFM6 * anonym0 = &chan[m].nonames->dfm6;
         if (anonym0->d9) {
            /* dfm09 lat, dir */
            v = bits2val(b, b_len, 0UL, 32UL);
            u = bits2val(b, b_len, 32UL, 16UL);
            vr = (float)v*1.E-7f;
            checkdfpos(vr, anonym0->lastlat, m);
            anonym0->lastlat = vr;
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
      }
      break;
   case 2UL:
      v = bits2val(b, b_len, 0UL, 32UL);
      vr = (float)v*1.E-7f;
      checkdf69(vr, m); /* test if dfm6 or dfm9 */
      if (chan[m].nonames->dfm6.d9) {
         /* dfm09 long, clb */
         ui = (int32_t)bits2val(b, b_len, 32UL, 16UL);
         if (ui>=32768L) ui -= 65536L;
         checkdfpos(vr, chan[m].nonames->dfm6.lastlong, m);
         chan[m].nonames->dfm6.lastlong = vr;
         if (verb) {
            osi_WrStr(" long:", 7ul);
            osic_WrFixed(vr, 5L, 0UL);
            osic_WrFixed((float)ui*0.01f, 1L, 0UL);
            osi_WrStr("m/s", 4ul);
         }
      }
      else {
         /* dfm06 lat, speed */
         u = bits2val(b, b_len, 32UL, 16UL);
         checkdfpos(vr, chan[m].nonames->dfm6.lastlat, m);
         chan[m].nonames->dfm6.lastlat = vr;
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
      if (chan[m].nonames->dfm6.d9) {
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
         checkdfpos(vr, chan[m].nonames->dfm6.lastlong, m);
         chan[m].nonames->dfm6.lastlong = vr;
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
      if (chan[m].nonames->dfm6.d9) {
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
            tt += _cnst1[tmon];
            if ((tyear&3UL)==0UL && tmon>2UL) ++tt;
         }
         tt = ((tt+tday)-1UL)*86400UL+thour*3600UL+tmin*60UL;
      }
      else tt = 0UL;
      { /* with */
         struct DFM6 * anonym1 = &chan[m].nonames->dfm6;
         if (anonym1->wasdate) anonym1->numcnt = 0UL;
         anonym1->wasdate = 1;
         anonym1->lastdate = anonym1->actdate;
         anonym1->actdate = tt;
         th = osic_time();
         if (anonym1->lastdate+60UL==anonym1->actdate) {
            /* minute jump */
            if (anonym1->frametimeok) thh = 3UL;
            else thh = 4UL;
            if (anonym1->lastdatesystime+thh>=th) {
               /* in timespan */
               anonym1->frametime = anonym1->actdate-th;
                /* sonde realtime - systime */
               anonym1->frametimeok = 1;
               if (verb) osi_WrStr(" got SECOND", 12ul);
            }
         }
         anonym1->lastdatesystime = th;
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


static void dfmnumtostr(uint32_t v, char s[], uint32_t s_len)
{
   char h[21];
   if (dfmoldname) {
      aprsstr_Assign(s, s_len, "DF6", 4ul);
      s[3UL] = hex(v/65536UL);
      s[4UL] = hex(v/4096UL);
      s[5UL] = hex(v/256UL);
      s[6UL] = hex(v/16UL);
      s[7UL] = hex(v);
      if (s_len-1>=8UL) s[8UL] = 0;
   }
   else {
      aprsstr_CardToStr(v%100000000UL, 0UL, h, 21ul);
                /* strip to 8 digits no trailing 0 */
      aprsstr_Assign(s, s_len, "D", 2ul);
      aprsstr_Append(s, s_len, h, 21ul);
   }
} /* end dfmnumtostr() */


static void getdfname(const char b[], uint32_t b_len, uint32_t m,
                uint32_t startbyte)
{
   uint32_t n;
   uint32_t u;
   uint32_t v;
   char s[101];
   struct DFM6 * anonym;
   { /* with */
      struct DFM6 * anonym = &chan[m].nonames->dfm6;
      v = bits2val(b, b_len, 0UL, 8UL);
      if (v==startbyte) {
         u = bits2val(b, b_len, 24UL, 4UL);
         v = bits2val(b, b_len, 8UL, 16UL);
         if (u==1UL) {
            /* id low 16 bit*/
            n = anonym->idnew&65535UL;
            if (n==0UL || n==v) {
               ++anonym->idcnt0;
               anonym->idtime = osic_time();
               if (verb) {
                  osi_WrStr(" ID LOW(", 9ul);
                  osic_WrINT32(anonym->idcnt0, 1UL);
                  osi_WrStr(")", 2ul);
               }
            }
            else {
               killdfid(m, 0);
               if (verb) osi_WrStr(" NEW ID LOW", 12ul);
            }
            anonym->idnew = (uint32_t)((uint32_t)
                anonym->idnew&0xFFFF0000UL)+v;
         }
         else if (u==0UL) {
            /* id high 16 bit*/
            n = anonym->idnew/65536UL;
            if (n==0UL || n==v) {
               ++anonym->idcnt1;
               anonym->idtime = osic_time();
               if (verb) {
                  osi_WrStr(" ID HIGH(", 10ul);
                  osic_WrINT32(anonym->idcnt1, 1UL);
                  osi_WrStr(")", 2ul);
               }
            }
            else {
               killdfid(m, 0);
               if (verb) osi_WrStr(" NEW ID HIGH ", 14ul);
            }
            anonym->idnew = (uint32_t)((uint32_t)anonym->idnew&0xFFFFUL)
                +v*65536UL;
         }
         if (anonym->idcnt0>=dfmidthreshold && anonym->idcnt1>=dfmidthreshold)
                 {
            dfmnumtostr(anonym->idnew, s, 101ul);
            aprsstr_Assign(anonym->id9, 9ul, s, 101ul);
            memcpy(anonym->idcheck,anonym->id9,9u);
            anonym->txok = 1;
            if (verb) {
               osi_WrStr(" ID:", 5ul);
               osi_WrStr(s, 101ul);
            }
         }
      }
   }
} /* end getdfname() */


static char olddfmser(uint32_t v, char s[], uint32_t s_len)
{
   uint32_t n;
   uint32_t i;
   s[0UL] = 'D';
   for (i = 6UL; i>=1UL; i--) {
      n = v&15UL;
      if (n>9UL) return 0;
      s[i] = (char)(n+48UL);
      v = v>>4;
   } /* end for */
   s[7UL] = 0;
   return 1;
} /* end olddfmser() */


static void finddfname(const char b[], uint32_t b_len,
                uint32_t m)
{
   uint8_t st;
   uint32_t thres;
   uint32_t v;
   uint32_t i;
   uint32_t ix;
   uint16_t d;
   char s[101];
   struct DFM6 * anonym;
   /* start byte found */
   struct DFNAMES * anonym0;
   /* make new entry */
   struct DFNAMES * anonym1;
   char tmp;
   { /* with */
      struct DFM6 * anonym = &chan[m].nonames->dfm6;
      st = (uint8_t)bits2val(b, b_len, 0UL, 8UL); /* frame start byte */
      ix = bits2val(b, b_len, 24UL, 4UL); /* hi/lo part of ser */
      d = (uint16_t)bits2val(b, b_len, 8UL, 16UL); /* data bytes */
      i = 0UL;
      /*find highest channel number single frame serial,
                (2 frame serial will make a single serial too) */
      if (anonym->idcnt0<dfmidthreshold && anonym->idcnt1<dfmidthreshold) {
         /* 2 frame serial ok*/
         v = bits2val(b, b_len, 0UL, 28UL);
         if ((uint32_t)st>anonym->lastfrid>>20) {
            anonym->lastfrid = v;
            if (verb) osi_WrStr(" MAXCH", 7ul);
            anonym->lastfrcnt = 0UL;
         }
         else if ((uint32_t)st==anonym->lastfrid>>20) {
            /* same id found */
            if (v==anonym->lastfrid) {
               ++anonym->lastfrcnt;
               thres = dfmidthreshold*2UL;
               if (ix<=1UL) {
                  thres = thres*2UL;
                /* may be a 2 frame serial so increase safety level */
               }
               if (st/16U!=6U) {
                  thres = thres*2UL;
                /* may be not a dfm6 so increase safety level */
               }
               if (anonym->lastfrcnt>=thres) {
                  /* id found */
                  if (anonym->lastfrcnt==thres) {
                     /*            dfmnumtostr(v MOD 100000H, s);
                           (* which bits are ser? *)  */
                     if (olddfmser(v, s, 101ul)) {
                        aprsstr_Assign(anonym->id9, 9ul, s, 101ul);
                        memcpy(anonym->idcheck,anonym->id9,9u);
                        anonym->txok = 1;
                        if (verb) {
                           osi_WrStr(" NEW MAXCHID:", 14ul);
                           osi_WrStr(s, 101ul);
                        }
                     }
                     else {
                        anonym->lastfrcnt = 0UL;
                        if (verb) {
                           osi_WrStr(" NOT NUMERIC SERIAL", 20ul);
                        }
                     }
                  }
                  anonym->idtime = osic_time();
               }
               else if (verb) {
                  osi_WrStr(" MAXCHCNT/SECURITYLEVEL:", 25ul);
                  osic_WrINT32(anonym->lastfrcnt, 1UL);
                  osi_WrStr("/", 2ul);
                  osic_WrINT32(thres, 1UL);
               }
            }
            else {
               anonym->lastfrid = v; /* not stable ser */
               anonym->lastfrcnt = 0UL;
            }
         }
      }
      /*find highest channel number single frame serial */
      /*
          IF verb & (st=lastfrid) THEN
            CASE lastfrid DIV 16 OF
               7:WrStr(" PS-15");
              |0AH, 0BH:WrStr(" DFM-09");
              |0CH, 0DH:WrStr(" DFM-17");
            ELSE WrStr(" DFM-unknown");
            END;
          END;
      */
      while (i<anonym->nameregtop && anonym->namereg[i].start!=st) {
         ++i;
      }
      if (i<anonym->nameregtop) {
         { /* with */
            struct DFNAMES * anonym0 = &anonym->namereg[i];
            if (ix<=1UL && (anonym0->cnt[ix]==0U || anonym0->dat[ix]==d)) {
               anonym0->dat[ix] = d;
               if (anonym0->cnt[ix]<255U) ++anonym0->cnt[ix];
               if (verb2 || verb) {
                  /* & (idcnt0=0)*/
                  osi_WrStr(" ID:", 5ul);
                  osi_WrStr((char *)(tmp = hex((uint32_t)(st/16U)),
                &tmp), 1u/1u);
                  osi_WrStr((char *)(tmp = hex((uint32_t)st),&tmp),
                1u/1u);
                  osi_WrStr("[", 2ul);
                  osic_WrINT32(ix, 1UL);
                  osi_WrStr("] CNT:", 7ul);
                  osic_WrINT32((uint32_t)anonym0->cnt[0U], 1UL);
                  osi_WrStr(",", 2ul);
                  osic_WrINT32((uint32_t)anonym0->cnt[1U], 1UL);
               }
               if ((uint32_t)anonym0->cnt[0U]>=dfmidthreshold && (uint32_t)
                anonym0->cnt[1U]>=dfmidthreshold) {
                  if (anonym->idcnt0==0UL) {
                     anonym->idcnt0 = (uint32_t)anonym0->cnt[0U];
                     anonym->idcnt1 = (uint32_t)anonym0->cnt[1U];
                     anonym->nameregok = i;
                     dfmnumtostr((uint32_t)
                anonym0->dat[0U]*65536UL+(uint32_t)anonym0->dat[1U], s,
                101ul);
                     aprsstr_Assign(anonym->id9, 9ul, s, 101ul);
                     memcpy(anonym->idcheck,anonym->id9,9u);
                     anonym->txok = 1;
                     if (verb) {
                        osi_WrStr(" NEW AUTOID:", 13ul);
                        osi_WrStr(s, 101ul);
                     }
                  }
                  if (anonym->nameregok==i) {
                     if (verb2 || verb) osi_WrStr(" IDOK ", 7ul);
                     anonym->idtime = osic_time();
                  }
               }
            }
            else {
               /* data changed so not ser */
               anonym0->cnt[0U] = 0U;
               anonym0->cnt[1U] = 0U;
               if (anonym->nameregok==i) {
                  anonym->idcnt0 = 0UL; /* found id wrong */
                  anonym->idcnt1 = 0UL;
               }
            }
         }
      }
      else if (ix<=1UL) {
         { /* with */
            struct DFNAMES * anonym1 = &anonym->namereg[anonym->nameregtop];
            anonym1->start = st;
            anonym1->cnt[0U] = 0U;
            anonym1->cnt[1U] = 0U;
            anonym1->dat[ix] = d;
            anonym1->cnt[ix] = 1U;
         }
         if (anonym->nameregtop<49UL) ++anonym->nameregtop;
      }
   }
} /* end finddfname() */


static void decodeframe6(uint32_t m)
{
   uint32_t j;
   uint32_t i;
   uint32_t tnow;
   uint32_t rt;
   char s[201];
   char tx;
   struct DFM6 * anonym;
   struct CHAN * anonym0; /* my call if set */
   { /* with */
      struct DFM6 * anonym = &chan[m].nonames->dfm6;
      deinterleave(chan[m].dfm6a.rxbuf, 264ul, 0UL, 7UL, anonym->ch, 56ul);
      deinterleave(chan[m].dfm6a.rxbuf, 264ul, 56UL, 13UL, anonym->dh1,
                104ul);
      deinterleave(chan[m].dfm6a.rxbuf, 264ul, 160UL, 13UL, anonym->dh2,
                104ul);
      if (hamming(anonym->dh1, 104ul, 13UL, anonym->db1,
                104ul) && hamming(anonym->dh2, 104ul, 13UL, anonym->db2,
                104ul)) {
         tnow = osic_time();
         /*    IF tused+CONTEXTLIFE<tnow THEN id[0]:=0C; nameregtop:=0 END;
                (* timed out context *) */
         if (anonym->tused+1800UL<tnow) killdfid(m, 1);
         anonym->tused = tnow;
         chan[m].nonames->lastvalid = tnow;
         if (verb) {
            WrChan((int32_t)m);
            if (anonym->id9[0U]) osi_WrStr(anonym->id9, 9ul);
            else if (anonym->d9) osi_WrStr("DF9", 4ul);
            else osi_WrStr("DFM", 4ul);
            WrdB(chan[m].admax-chan[m].admin);
            WrQ(chan[m].dfm6a.bitlev, chan[m].dfm6a.noise);
         }
         /*        Wrtune(chan[m].admax+chan[m].admin,
                chan[m].admax-chan[m].admin); */
         /*
                 WrStr(" ");
                 IF frametimeok THEN wrdate(tnow()+chan[m].dfm6.frametime);
                 ELSE WrStr("[wait for GPS Time]") END;
         */
         if (hamming(anonym->ch, 56ul, 7UL, anonym->cb, 56ul)) {
            if (verb) {
               osi_WrStr(" ", 2ul);
               for (i = 0UL; i<=6UL; i++) {
                  wh(bits2val(anonym->cb, 56ul, i*4UL, 4UL));
               } /* end for */
            }
            if (dfmnametyp>=256UL) {
               if (dfmnametyp&255UL) {
                  getdfname(anonym->cb, 56ul, m, dfmnametyp&255UL);
               }
               else finddfname(anonym->cb, 56ul, m);
            }
         }
         decodesub(anonym->db1, 104ul, m, 0UL);
         decodesub(anonym->db2, 104ul, m, 1UL);
         /* build tx frame */
         /*    WITH chan[m].nonames^.dfm6 DO */
         tx = (anonym->id9[0U] && aprsstr_StrCmp(anonym->idcheck, 9ul,
                anonym->id9, 9ul)) && anonym->idtime+900UL>tnow;
         for (i = 0UL; i<=9UL; i++) {
            s[i] = 0;
         } /* end for */
         if (tx && anonym->txok) {
            /* else stop sending if ambigous id */
            for (i = 0UL; i<=8UL; i++) {
               s[i] = anonym->id9[i]; /* sonde id or zero string for no tx */
            } /* end for */
         }
         { /* with */
            struct CHAN * anonym0 = &chan[m];
            s[10U] = (char)(anonym0->mycallc/16777216UL);
            s[11U] = (char)(anonym0->mycallc/65536UL&255UL);
            s[12U] = (char)(anonym0->mycallc/256UL&255UL);
            s[13U] = (char)(anonym0->mycallc&255UL);
            if (anonym0->mycallc>0UL) s[14U] = anonym0->myssid;
            else s[14U] = '\020';
         }
         if (anonym->frametimeok) {
            rt = tnow+anonym->frametime; /* interpolated sonde realtime */
         }
         else rt = 0UL;
         s[15U] = (char)(rt/16777216UL);
         s[16U] = (char)(rt/65536UL&255UL);
         s[17U] = (char)(rt/256UL&255UL);
         s[18U] = (char)(rt&255UL);
         j = 19UL;
         for (i = 0UL; i<=3UL; i++) {
            s[j] = (char)bits2val(anonym->cb, 56ul, i*8UL, 8UL);
                /* payload */
            ++j;
         } /* end for */
         for (i = 0UL; i<=6UL; i++) {
            s[j] = (char)bits2val(anonym->db1, 104ul, i*8UL, 8UL);
                /* payload */
            ++j;
         } /* end for */
         for (i = 0UL; i<=6UL; i++) {
            s[j] = (char)bits2val(anonym->db2, 104ul, i*8UL, 8UL);
                /* payload */
            ++j;
         } /* end for */
         sdrparm(s, 201ul, &j, (int32_t)m);
         alludp(chan[m].udptx, j, s, 201ul);
         if ((verb && !tx) && anonym->id9[0U]) {
            if (!aprsstr_StrCmp(anonym->idcheck, 9ul, anonym->id9, 9ul)) {
               osi_WrStrLn("", 1ul);
               osi_WrStr(" changing id ", 14ul);
               osi_WrStr(anonym->id9, 9ul);
               osi_WrStr("<->", 4ul);
               osi_WrStr(anonym->idcheck, 9ul);
            }
            else {
               osi_WrStrLn("", 1ul);
               osi_WrStr(" no tx, id timeout ", 20ul);
            }
         }
         /*    END; */
         /*build tx frame */
         monitor(m, "DFM", 4ul, anonym->id9, 9ul);
         setactiv(&chan[m].dfm6a.savecnt, 30L);
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
   struct DFM6A * anonym;
   { /* with */
      struct DFM6A * anonym = &chan[m].dfm6a;
      chan[m].nonames->dfm6.synword = chan[m]
                .nonames->dfm6.synword*2UL+(uint32_t)d;
      if (anonym->rxp>=264UL) {
         if ((chan[m].nonames->dfm6.synword&65535UL)==17871UL) {
            anonym->rxp = 0UL;
         }
         else if ((chan[m].nonames->dfm6.synword&65535UL)==47664UL) {
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
   struct DFM6A * anonym;
   /*WrFixed(u,0,9); WrStr(" "); */
   d = u>=u0;
   { /* with */
      struct DFM6A * anonym = &chan[m].dfm6a;
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
   struct DFM6A * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct DFM6A * anonym = &chan[m].dfm6a;
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
   struct DFM6A * anonym;
   { /* with */
      struct DFM6A * anonym = &chan[m].dfm6a;
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
      struct C34 * anonym = &chan[channel].nonames->c34;
      sum1 = 0UL;
      sum2 = 65791UL;
      for (i = 2UL; i<=6UL; i++) {
         sum1 += (uint32_t)(uint8_t)chan[channel].c34a.rxbuf[i];
         sum2 -= (uint32_t)(uint8_t)chan[channel].c34a.rxbuf[i]*(7UL-i);
      } /* end for */
      sum1 = sum1&255UL;
      sum2 = sum2&255UL;
      ok0 = sum1==(uint32_t)(uint8_t)
                chan[channel].c34a.rxbuf[7U] && sum2==(uint32_t)(uint8_t)
                chan[channel].c34a.rxbuf[8U];
      if (anonym->tused+1800UL<osic_time()) {
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
         WrQuali(noiselevel(chan[channel].c34a.bitlev,
                chan[channel].c34a.noise));
         /*      Wrtune(chan[channel].admax+chan[channel].admin,
                chan[channel].admax-chan[channel].admin); */
         osi_WrStr(" [", 3ul);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[2U], 2UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[3U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[4U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[5U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[6U], 2UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[7U], 2UL);
         osi_WrHex((uint32_t)(uint8_t)chan[channel].c34a.rxbuf[8U], 2UL);
         osi_WrStr("] ", 3ul);
      }
      if (ok0) {
         /* chksum ok */
         /*INC(test1); WrInt(test1, 6); WrStrLn(" testcnt"); */
         chan[channel].nonames->lastvalid = osic_time();
         val = (uint32_t)(uint8_t)
                chan[channel].c34a.rxbuf[6U]+(uint32_t)(uint8_t)
                chan[channel].c34a.rxbuf[5U]*256UL+(uint32_t)(uint8_t)
                chan[channel].c34a.rxbuf[4U]*65536UL+(uint32_t)(uint8_t)
                chan[channel].c34a.rxbuf[3U]*16777216UL;
         hr = (double)SaveReal(val);
         /*      IF hr<0.0 THEN END; */
         /*WrFixed(hr, 10, 20); */
         if (anonym->c50) {
            if (anonym->id50.idtime+3600UL<osic_time()) {
               anonym->id50.id[0U] = 0;
               anonym->id50.idcheck[0U] = 0;
               anonym->id50.idcnt = 0UL;
            }
            /* remove old id */
            switch ((unsigned)chan[channel].c34a.rxbuf[2U]) {
            case '\003':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tair ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\004':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("thum ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\005':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("tcha ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\006':
               if (hr<99.9 && hr>(-99.9)) {
                  if (verb) {
                     osi_WrStr("to3  ", 6ul);
                     osic_WrFixed((float)hr, 1L, 0UL);
                     osi_WrStr("oC", 3ul);
                  }
               }
               break;
            case '\007':
               if (verb) {
                  osi_WrStr("io3  ", 6ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("uA", 3ul);
               }
               break;
            case '\020':
               if (hr<=100.0 && hr>=0.0) {
                  if (verb) {
                     osi_WrStr("hum  ", 6ul);
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
            case '\031':
               if (verb) {
                  osi_WrStr("sped ", 6ul);
                  osic_WrFixed((float)(hr*3.6), 1L, 0UL);
                  osi_WrStr("km/h", 5ul);
               }
               break;
            case '\032':
               if (verb) {
                  osi_WrStr("dir  ", 6ul);
                  osic_WrFixed((float)hr, 1L, 0UL);
                  osi_WrStr("deg", 4ul);
               }
               break;
            case 'd': /* 66H 67H 68H 89H 6BH seem to be fixed too */
               /*         |CHR(1BH): */
               /*                    IF verb THEN WrStr("1B  ");
                WrFixed(hr, 5, 0); WrStr("?") END; */
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
            switch ((unsigned)chan[channel].c34a.rxbuf[2U]) {
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
               s[i+15UL] = chan[channel].c34a.rxbuf[i+2UL]; /* payload */
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
         /*build tx frame */
         if (anonym->c50) monitor(channel, "C50", 4ul, anonym->id50.id, 9ul);
         else monitor(channel, "C34", 4ul, anonym->id34.id, 9ul);
      }
      else if (verb2) {
         osi_WrStr("---- chksum ", 13ul);
         osi_WrHex(sum1, 2UL);
         osi_WrHex(sum2, 2UL);
      }
      setactiv(&chan[channel].c34a.savecnt, 30L);
      if (verb2 || ok0 && verb) {
         appendsdr(channel);
         osi_WrStrLn("", 1ul);
      }
   }
/* name(9) 0C call(5) playload(7) */
} /* end demodframe34() */


static void demodbit34(uint32_t channel, char d)
{
   struct C34A * anonym;
   d = !d;
   { /* with */
      struct C34A * anonym = &chan[channel].c34a;
      anonym->rxbytec = anonym->rxbytec*2UL+(uint32_t)d;
      if ((anonym->rxbytec&268435455UL)==234942462UL) {
         /* c34 1110 0000 0000 1110 1111 1111 1110*/
         chan[channel].nonames->c34.c50 = 0;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
         chan[channel].framestarttime = osic_time();
      }
      else if ((anonym->rxbytec&2097151UL)==3070UL) {
         /* c50 0 0000 0000 1011 1111 1110 */
         chan[channel].nonames->c34.c50 = 1;
         anonym->rxp = 2UL;
         anonym->rxbitc = 0UL;
         chan[channel].framestarttime = osic_time();
      }
      if ((chan[channel].nonames->c34.c50 || anonym->rxbitc) || !d) {
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
   struct C34A * anonym;
   d = u>=0.0f;
   { /* with */
      struct C34A * anonym = &chan[channel].c34a;
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
   struct C34A * anonym;
   { /* with */
      struct C34A * anonym = &chan[channel].c34a;
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

/*---------------------- M10 M20 */

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

static SET256 _cnst5 = {0x03FFFFF7UL,0x00000003UL,0x00000000UL,0x00000018UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};
static float _cnst4[3] = {12100.0f,36500.0f,4.75E+5f};
static float _cnst3[3] = {1.21E-16f,1.1060606E-1f,1.5833333E-1f};
static SET256 _cnst2 = {0x03BBBBF0UL,0x80600000UL,0x06A001A0UL,0x0000001CUL,
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
   struct M1020 * anonym;
   struct CHAN * anonym0; /* call if set */
   { /* with */
      struct M1020 * anonym = &chan[m].m10;
      anonym->txok = 0;
      repairstep = 16U;
      repl = 0UL;
      for (;;) {
         crcok = (uint32_t)crcm10(99L, anonym->rxbuf,
                101ul)==m10card(anonym->rxbuf, 101ul, 99L, 2L);
         if ((anonym->alternativ || crcok) || repairstep==0U) break;
         repl = 0UL;
         for (i = 0UL; i<=98UL; i++) {
            if (!X2C_INL(i,256,_cnst2) && anonym->fixcnt[i]>=repairstep) {
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
         /*INC(test1); WrInt(test1, 6); WrStrLn(" testcnt");  */
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
            lat = (double)(int32_t)m10card(anonym->rxbuf, 101ul, 14L,
                 4L)*8.3819036711397E-8;
            lon = (double)(int32_t)m10card(anonym->rxbuf, 101ul, 18L,
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
            v = sqrt(ve*ve+vn*vn); /* hor speed */
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
            rt = X2C_DIVR(4095.0f-rt,rt)-_cnst3[sct];
            if (rt>0.0f && sct<3UL) {
               rt = X2C_DIVR(_cnst4[sct],rt);
               if (rt>0.0f) {
                  rt = (float)log((double)rt);
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
         /*build tx frame */
         monitor(m, "M10", 4ul, ids, 201ul);
      }
      else if (verb) {
         WrChan((int32_t)m);
         osi_WrStr("M10 crc error", 14ul);
      }
      setactiv(&anonym->savecnt, 30L);
      if (verb) {
         WrdB(chan[m].admax-chan[m].admin);
         WrQuali(noiselevel(anonym->bitlev, anonym->noise));
         if (repl>0UL) {
            osi_WrStr(" +r", 4ul);
            osic_WrINT32(repl, 1UL);
            osi_WrStr("/", 2ul);
            osic_WrINT32((uint32_t)repairstep, 1UL);
         }
         /*      Wrtune(chan[m].admax+chan[m].admin,
                chan[m].admax-chan[m].admin); */
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
            if (anonym->alternativ || !X2C_INL(i,256,_cnst5)) {
               if (tab%12UL==0UL) osi_WrStrLn("", 1ul);
               osic_WrINT32(i, 3UL);
               osi_WrStr(":", 2ul);
               osi_WrHex((uint32_t)(int32_t)m10card(anonym->rxbuf, 101ul,
                 (int32_t)i, 1L), 3UL);
               ++tab;
            }
         } /* end for */
      }
      if (verb) osi_WrStrLn("", 1ul);
   }
} /* end decodeframe10() */

#define sondeudp_DEGMUL0 1.E-6

#define sondeudp_VMUL0 0.01
/*    CRCPOS=68; */

#define sondeudp_CRCPOSB 22
/*    HSET  =SET256{0..2,4..25,32,33,93..97,99,100}
                ;                (* not hexlist known bytes *) */

static SET256 sondeudp_HSET0 = {0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL}; /* not hexlist known bytes */

static SET256 sondeudp_VARSET0 = {0xF3E27F54UL,0x0000000FUL,0x00000030UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL}; /* known as variable */

static SET256 _cnst7 = {0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};
static SET256 _cnst6 = {0xF3E27F54UL,0x0000000FUL,0x00000030UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};

static void decodeframe20(uint32_t m)
{
   uint32_t fnum;
   uint32_t gpstimecorr;
   uint32_t repl;
   uint32_t id;
   uint32_t flen;
   uint32_t tab;
   uint32_t week;
   uint32_t tow;
   uint32_t frl;
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
   char crcbok;
   char crcok;
   uint8_t repairstep;
   char ids[201];
   char s[201];
   struct M1020 * anonym;
   struct CHAN * anonym0; /* call if set */
   uint32_t tmp;
   { /* with */
      struct M1020 * anonym = &chan[m].m10;
      memset((char *)ids,(char)0,201UL);
      anonym->txok = 0;
      repairstep = 16U;
      repl = 0UL;
      /*check first block */
      for (i = 1UL; i<=21UL; i++) {
         s[i] = anonym->rxbuf[i+1UL];
      } /* end for */
      s[0U] = '\026';
      crcbok = (uint32_t)crcm10(21L, s, 201ul)==m10card(anonym->rxbuf,
                101ul, 22L, 2L); /* inner block crc may be removed */
      frl = m10card(anonym->rxbuf, 101ul, 0L, 1L)+1UL; /* frame length (?) */
      if (frl>100UL) frl = 100UL;
      if (frl>2UL) {
         for (;;) {
            /* repair bytes */
            crcok = (uint32_t)crcm10((int32_t)(frl-2UL), anonym->rxbuf,
                101ul)==m10card(anonym->rxbuf, 101ul, (int32_t)(frl-2UL),
                2L);
            if (crcok || repairstep==0U) {
               /*alternativ OR*/
               break;
            }
            repl = 0UL;
            if (crcbok) i = 24UL;
            else i = 0UL;
            do {
               if (!X2C_INL(i,256,_cnst6) && anonym->fixcnt[i]>=repairstep) {
                  /* replace stable bytes */
                  if (anonym->rxbuf[i]!=anonym->fixbytes[i]) {
                     ++repl;
                     anonym->rxbuf[i] = anonym->fixbytes[i];
                  }
               }
               ++i;
            } while (i<frl-2UL);
            repairstep = repairstep/2U; /* make next crc check */
         }
         if (crcok) {
            /* update fixbyte statistics */
            tmp = frl-2UL;
            i = 0UL;
            if (i<=tmp) for (;; i++) {
               /* save good bytes */
               if (anonym->fixbytes[i]==anonym->rxbuf[i]) {
                  if (anonym->fixcnt[i]<255U) {
                     ++anonym->fixcnt[i]; /* how long got same byte */
                  }
               }
               else {
                  anonym->fixbytes[i] = anonym->rxbuf[i];
                  anonym->fixcnt[i] = 0U;
               }
               if (i==tmp) break;
            } /* end for */
            /* update fixbyte statistics */
            /* get ID */
            strncpy(ids,"ME0000000",201u);
            id = m10rcard(anonym->rxbuf, 101ul, 19L, 2L)/4UL;
            ids[8U] = (char)(id%10UL+48UL);
            ids[7U] = (char)((id/10UL)%10UL+48UL);
            ids[6U] = (char)((id/100UL)%10UL+48UL);
            ids[5U] = (char)((id/1000UL)%10UL+48UL);
            ids[4U] = (char)((id/10000UL)%10UL+48UL);
            id = m10card(anonym->rxbuf, 101ul, 18L, 1L);
            ids[3U] = hex(id);
            ids[2U] = hex(id/16UL);
            /* get ID */
            /*
                    IF alternativ THEN
                      IF verb THEN
                        WrChan(m);
                        WrStr("M20 ");
                        WrStr(ids);
                        WrStr(" ");
                        wrtime(timefn MOD 86400);
                        WrStr(" typ="); WrInt(m10card(rxbuf, 2, 1),1);
                WrStr(" ");
                      END;
            
                    ELSE
            */
            /*
            0    framelen 45
            1    type 20
            2-3  adc
            4-5  adc
            6-7  ad temp
            22-23 block check or what else
            */
            tow = m10card(anonym->rxbuf, 101ul, 15L, 3L);
            week = m10card(anonym->rxbuf, 101ul, 26L, 2L);
            gpstimecorr = 18UL;
            anonym->timefn = (tow+week*604800UL+315964800UL)-18UL;
            lat = (double)(int32_t)m10card(anonym->rxbuf, 101ul, 28L,
                 4L)*1.E-6;
            lon = (double)(int32_t)m10card(anonym->rxbuf, 101ul, 32L,
                 4L)*1.E-6;
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 24L, 2L);
            if (ci>32767L) ci -= 65536L;
            vv = (double)ci*0.01;
            anonym->txok = 1;
            fnum = m10card(anonym->rxbuf, 101ul, 21L, 1L);
            /*      IF verb2 THEN WrStr(" ");DateToStr(time, s); WrStr(s);
                WrStr(" ") END; */
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 8L, 3L);
                /* is it signed? */
            if (ci>8388352L) ci -= 16777216L;
            alt = (double)ci*0.01;
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 11L, 2L);
            if (ci>32767L) ci -= 65536L;
            ve = (double)ci*0.01;
            ci = (int32_t)m10card(anonym->rxbuf, 101ul, 13L, 2L);
            if (ci>32767L) ci -= 65536L;
            vn = (double)ci*0.01;
            v = sqrt(ve*ve+vn*vn); /* hor speed */
            dir = atang2(vn, ve)*5.7295779513082E+1;
            if (dir<0.0) dir = 360.0+dir;
            if (verb) {
               WrChan((int32_t)m);
               osi_WrStr("M20 ", 5ul);
               osi_WrStr(ids, 201ul);
               osi_WrStr(" ", 2ul);
               osic_WrINT32(fnum, 1UL);
               osi_WrStr(" ", 2ul);
               aprsstr_DateToStr(anonym->timefn, s, 201ul);
               osi_WrStr(s, 201ul);
               /*        wrtime(timefn MOD 86400); */
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
               osic_WrFixed((float)vv, 1L, 1UL); /* climb */
               osi_WrStr("m/s ", 5ul);
               /*        IF tok THEN WrStr(" "); WrFixed(rt, 1, 1);
                WrStr("C") END; */
               osi_WrStr("a1=", 4ul);
               osic_WrINT32(m10rcard(anonym->rxbuf, 101ul, 2L, 2L), 1UL);
               osi_WrStr(" a2=", 5ul);
               osic_WrINT32(m10rcard(anonym->rxbuf, 101ul, 4L, 2L), 1UL);
               osi_WrStr(" a3=", 5ul);
               osic_WrINT32(m10rcard(anonym->rxbuf, 101ul, 6L, 2L), 1UL);
               osi_WrStr(" c2=", 5ul);
               osic_WrINT32(m10rcard(anonym->rxbuf, 101ul, 45L, 2L), 1UL);
               osi_WrStr(" c3=", 5ul);
               osic_WrINT32(m10rcard(anonym->rxbuf, 101ul, 47L, 2L), 1UL);
               osi_WrStr(" cr=", 5ul);
               osic_WrINT32(m10rcard(anonym->rxbuf, 101ul, 22L, 2L), 1UL);
            }
            /* verb */
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
            /*build tx frame */
            monitor(m, "M20", 4ul, ids, 201ul);
         }
         else if (verb) {
            WrChan((int32_t)m);
            osi_WrStr("M20 crc error", 14ul);
         }
         setactiv(&anonym->savecnt, 30L);
         if (verb) {
            WrdB(chan[m].admax-chan[m].admin);
            /*      WrQuali(noiselevel(bitlev, noise)); */
            if (repl>0UL) {
               osi_WrStr(" +r", 4ul);
               osic_WrINT32(repl, 1UL);
               osi_WrStr("/", 2ul);
               osic_WrINT32((uint32_t)repairstep, 1UL);
            }
            /*      Wrtune(chan[m].admax+chan[m].admin,
                chan[m].admax-chan[m].admin); */
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
            tmp = frl-1UL;
            i = 0UL;
            if (i<=tmp) for (;; i++) {
               if (anonym->alternativ || !X2C_INL(i,256,_cnst7)) {
                  /*&  (tset[i]<256)*/
                  if (tab%12UL==0UL) osi_WrStrLn("", 1ul);
                  osic_WrINT32(i, 3UL);
                  osi_WrStr(":", 2ul);
                  osi_WrHex(m10card(anonym->rxbuf, 101ul, (int32_t)i, 1L),
                3UL);
                  ++tab;
               }
               if (i==tmp) break;
            } /* end for */
         }
         if (verb) osi_WrStrLn("", 1ul);
      }
   }
/* min framelen */
} /* end decodeframe20() */


static void demodbyte10(uint32_t m, char d)
{
   struct M1020 * anonym;
   { /* with */
      struct M1020 * anonym = &chan[m].m10;
      /*WrInt(ORD(d),1);  */
      /*WrInt(rxp, 4); WrStrLn(""); */
      anonym->synword1 = anonym->synword1*2UL+(uint32_t)(((uint32_t)
                anonym->synword&0x800000UL)!=0UL);
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=101UL) {
         if (((uint32_t)anonym->synword1&0xFFC7UL)==0xC0UL) {
            /* unique frame sync */
            /*WrStr(" -fsyn- "); WrHex(synword>>12, 3); WrHex(synword>>8, 3);
                WrHex(synword, 3); WrStrLn(""); */
            if (((uint32_t)anonym->synword&0xFFFFFFUL)==0x649F20UL) {
               anonym->rxp = 3UL;
               anonym->alternativ = 0;
               anonym->ism20 = 0;
            }
            else if (((uint32_t)anonym->synword&0xFFFFF0UL)==0x644900UL) {
               /*WrStr(" -syn- "); WrHex(synword1 DIV 256, 10);
                WrHex(synword1, 15); */
               anonym->rxp = 3UL;
               anonym->alternativ = 1;
               anonym->ism20 = 0;
            }
            else if (((uint32_t)anonym->synword&0xFFFF00UL)==0x452000UL) {
               /*WrStr(" -syn1- "); */
               anonym->rxp = 3UL;
               anonym->alternativ = 0;
               anonym->ism20 = 1;
            }
            /*WrStr(" -syn20- "); */
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
            if (anonym->ism20) {
               if (anonym->rxp==88UL) {
                  if (chan[m].framestarttime+3UL>=osic_time()) {
                     decodeframe20(m);
                  }
                  anonym->rxp = 101UL; /* start new frame hunt */
               }
            }
            else if (anonym->rxp==101UL && chan[m]
                .framestarttime+3UL>=osic_time()) decodeframe10(m);
         }
      }
   }
} /* end demodbyte10() */


static void demodbit10(uint32_t m, float u)
{
   char bit;
   char d;
   float ua;
   struct M1020 * anonym;
   d = u>=0.0f;
   { /* with */
      struct M1020 * anonym = &chan[m].m10;
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
   struct M1020 * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   { /* with */
      struct M1020 * anonym = &chan[m].m10;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbit10(m, u);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
      }
      else anonym->plld = d;
      anonym->cbit = !anonym->cbit;
   }
} /* end demod10() */


static void Fsk10(uint32_t m)
{
   float ff;
   int32_t lim;
   struct M1020 * anonym;
   { /* with */
      struct M1020 * anonym = &chan[m].m10;
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

/*---------------------- IMET */
#define sondeudp_POLY0 0x1021 

#define sondeudp_NAMECHECKS 3


static void decodeframeimet(uint32_t m)
{
   uint32_t flen;
   uint32_t frnum;
   uint32_t ii;
   uint32_t gpstime;
   uint32_t len;
   uint32_t j;
   uint32_t i;
   uint32_t p;
   uint32_t st;
   uint16_t crc;
   char typ;
   float long0;
   float lat;
   float dir;
   float vy;
   float vx;
   char posok;
   char lfdone;
   char crcok;
   char name[9];
   char s[201];
   struct IMETA * anonym;
   struct IMET * anonym0;
   struct IMET * anonym1;
   /* gpstime - framenum */
   struct IMET * anonym2;
   struct CHAN * anonym3; /* call if set */
   char tmp;
   uint32_t tmp0;
   memset((char *)name,(char)0,9UL);
   posok = 0;
   lfdone = 1;
   p = 0UL;
   { /* with */
      struct IMETA * anonym = &chan[m].imeta;
      { /* with */
         struct IMET * anonym0 = &chan[m].nonames->imet;
         if (anonym0->idnew && anonym0->idcnt>3UL) {
            /* name ok */
            strncpy(name,"IMET",9u);
            aprsstr_IntToStr((int32_t)anonym0->idnew, 1UL, s, 201ul);
            aprsstr_Append(name, 9ul, s, 201ul);
         }
      }
      frnum = X2C_max_longcard;
      gpstime = X2C_max_longcard;
      for (;;) {
         if (p+2UL>105UL) break;
         typ = anonym->rxbuf[p+1UL]; /* frame type */
         switch ((unsigned)typ) {
         case '\001':
            len = 12UL;
            break;
         case '\002':
            len = 18UL;
            break;
         case '\003':
            len = 5UL+(uint32_t)(uint8_t)anonym->rxbuf[p+2UL];
            break;
         case '\004':
            len = 20UL;
            break;
         case '\005':
            len = 30UL;
            break;
         default:;
            len = 0UL;
            break;
         } /* end switch */
         if (len<=2UL || p+len>anonym->rxp) break;
         if (verb) {
            if (lfdone) {
               WrChan((int32_t)m);
               osi_WrStr("iMET ", 6ul);
               if (name[0U]==0) {
                  osi_WrStr("idchk:", 7ul);
                  osic_WrINT32(chan[m].nonames->imet.idcnt, 1UL);
               }
               else osi_WrStr(name, 9ul);
               /*          WrStr(" "); */
               WrdB(chan[m].admax-chan[m].admin);
               WrQuali(noiselevel(anonym->bitlev, anonym->noise));
               lfdone = 0;
            }
            osi_WrStr(" <", 3ul);
            osi_WrStr((char *)(tmp = (char)((uint32_t)(uint8_t)
                typ+48UL),&tmp), 1u/1u);
            osi_WrStr(">", 2ul);
         }
         setactiv(&anonym->savecnt, 30L);
         /* crc */
         crc = 0x1D0FU;
         tmp0 = len-3UL;
         i = 0UL;
         if (i<=tmp0) for (;; i++) {
            crc = crc^X2C_LSH((uint16_t)(uint32_t)(uint8_t)
                anonym->rxbuf[p+i],16,8);
            for (j = 0UL; j<=7UL; j++) {
               if ((0x8000U & crc)) crc = X2C_LSH(crc,16,1)^0x1021U;
               else crc = X2C_LSH(crc,16,1);
            } /* end for */
            if (i==tmp0) break;
         } /* end for */
         /* crc */
         crcok = crc==(uint16_t)((uint32_t)(uint8_t)
                anonym->rxbuf[(p+len)-2UL]*256UL+(uint32_t)(uint8_t)
                anonym->rxbuf[(p+len)-1UL]);
         if (crcok) {
            if (typ=='\002' || typ=='\005') {
               /* gps */
               /*INC(test1); */
               if (typ=='\005') ii = 25UL;
               else ii = 13UL;
               gpstime = cardmsb(anonym->rxbuf, 106ul, p+ii,
                1UL)*3600UL+cardmsb(anonym->rxbuf, 106ul, p+ii+1UL,
                1UL)*60UL+cardmsb(anonym->rxbuf, 106ul, p+ii+2UL, 1UL);
               /* raw gps time not utc */
               lat = SaveReal(cardmsb(anonym->rxbuf, 106ul, p+2UL, 4UL));
               long0 = SaveReal(cardmsb(anonym->rxbuf, 106ul, p+6UL, 4UL));
               posok = 1;
               /* check id */
               st = osic_time();
               { /* with */
                  struct IMET * anonym1 = &chan[m].nonames->imet;
                  if (st>=anonym1->idtime && st-anonym1->idtime>1800UL) {
                     /* a while not heard */
                     anonym1->idnew = 0UL;
                     anonym1->idcnt = 0UL;
                     name[0U] = 0;
                  }
                  if ((float)fabs(lat-anonym1->lastlat)+(float)
                fabs(long0-anonym1->lastlong)>1.0f) {
                     /* another region ? */
                     if (anonym1->idnew && verb) {
                        osi_WrStr(" pos jump! ", 12ul);
                     }
                     anonym1->idnew = 0UL;
                     anonym1->idcnt = 0UL;
                     name[0U] = 0;
                  }
                  anonym1->lastlat = lat;
                  anonym1->lastlong = long0;
                  anonym1->idtime = st;
               }
               /* check id */
               if (verb) {
                  osic_WrFixed(lat, 5L, 1UL);
                  osi_WrStr(" ", 2ul);
                  osic_WrFixed(long0, 5L, 1UL);
                  osi_WrStr(" ", 2ul);
                  osic_WrINT32((uint32_t)((int32_t)cardmsb(anonym->rxbuf,
                 106ul, p+10UL, 2UL)-5000L), 1UL);
                  osi_WrStr("m sats:", 8ul);
                  osic_WrINT32(cardmsb(anonym->rxbuf, 106ul, p+12UL, 1UL),
                1UL);
                  if (typ=='\005') {
                     /* gps with speeds */
                     vx = SaveReal(cardmsb(anonym->rxbuf, 106ul, p+13UL,
                4UL));
                     vy = SaveReal(cardmsb(anonym->rxbuf, 106ul, p+17UL,
                4UL));
                     dir = (float)(X2C_DIVL(atang2((double)vx,
                (double)vy),1.7453292519943E-2));
                     if (dir<0.0f) dir = 360.0f+dir;
                     osi_WrStr(" ", 2ul);
                     osic_WrFixed((float)(sqrt((double)(vx*vy+vy*vy)
                )*3.6), 2L, 1UL);
                     osi_WrStr("km/h ", 6ul);
                     osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(dir,
                X2C_min_longint,X2C_max_longint), 0UL);
                     osi_WrStr("deg clb:", 9ul);
                     osic_WrFixed(SaveReal(cardmsb(anonym->rxbuf, 106ul,
                p+21UL, 4UL)), 2L, 1UL);
                     osi_WrStr("m/s ", 5ul);
                  }
                  osi_WrStr(" ", 2ul); /* gps time not utc */
                  wrtime(((86400UL+gpstime)-18UL)%86400UL);
               }
            }
            else if (typ=='\001' || typ=='\004') {
               /* ptu */
               /*INC(test2); */
               frnum = cardmsb(anonym->rxbuf, 106ul, p+2UL, 2UL);
               if (verb) {
                  osi_WrStr("fn:", 4ul);
                  osic_WrINT32(frnum, 1UL);
                  osi_WrStr(" ", 2ul);
                  osic_WrFixed((float)cardmsb(anonym->rxbuf, 106ul, p+4UL,
                 3UL)*0.01f, 2L, 1UL);
                  osi_WrStr("hPa ", 5ul);
                  osic_WrFixed((float)(short)cardmsb(anonym->rxbuf,
                106ul, p+7UL, 2UL)*0.01f, 2L, 1UL);
                  osi_WrStr("C ", 3ul);
                  osic_WrFixed((float)cardmsb(anonym->rxbuf, 106ul, p+9UL,
                 2UL)*0.01f, 2L, 1UL);
                  osi_WrStr("% Vb:", 6ul);
                  osic_WrFixed((float)cardmsb(anonym->rxbuf, 106ul,
                p+11UL, 1UL)*0.1f, 1L, 1UL);
                  if (typ=='\004') {
                     /* extended ptu */
                     osi_WrStr(" ti:", 5ul);
                     osic_WrFixed((float)(short)cardmsb(anonym->rxbuf,
                 106ul, p+12UL, 2UL)*0.01f, 2L, 1UL);
                     osi_WrStr(" tp:", 5ul);
                     osic_WrFixed((float)(short)cardmsb(anonym->rxbuf,
                 106ul, p+14UL, 2UL)*0.01f, 2L, 1UL);
                     osi_WrStr(" tu:", 5ul);
                     osic_WrFixed((float)(short)cardmsb(anonym->rxbuf,
                 106ul, p+16UL, 2UL)*0.01f, 2L, 1UL);
                  }
               }
            }
            else if (typ=='\003') {
               /* extensions */
               if (len==13UL) {
                  /* may be ozone */
                  if (verb) {
                     osi_WrStr(" otyp:", 7ul);
                     osic_WrINT32(cardmsb(anonym->rxbuf, 106ul, p+3UL, 1UL),
                1UL);
                     osi_WrStr(" onum:", 7ul);
                     osic_WrINT32(cardmsb(anonym->rxbuf, 106ul, p+4UL, 1UL),
                1UL);
                     osi_WrStr(" oi:", 5ul);
                     osic_WrFixed((float)cardmsb(anonym->rxbuf, 106ul,
                p+5UL, 2UL)*0.001f, 3L, 1UL);
                     osi_WrStr("uA to:", 7ul);
                     osic_WrFixed((float)cardmsb(anonym->rxbuf, 106ul,
                p+7UL, 2UL)*0.01f, 2L, 1UL);
                     osi_WrStr("C Pump:", 8ul);
                     osic_WrINT32(cardmsb(anonym->rxbuf, 106ul, p+9UL, 1UL),
                1UL);
                     osi_WrStr("mA VP:", 7ul);
                     osic_WrFixed((float)cardmsb(anonym->rxbuf, 106ul,
                p+10UL, 1UL)*0.1f, 1L, 1UL);
                     osi_WrStr("V ", 3ul);
                  }
               }
               else if (verb) {
                  /* unknown extension */
                  osi_WrStr("unknown extension frame [", 26ul);
                  i = 0UL;
                  for (;;) {
                     if (i+3UL>len) {
                        osi_WrStr("] ", 3ul);
                        break;
                     }
                     osi_WrHex((uint32_t)(uint8_t)anonym->rxbuf[p+i],
                3UL);
                     ++i;
                     if ((i&15UL)==15UL && i+3UL<len) osi_WrStrLn("", 1ul);
                  }
               }
            }
         }
         else if (verb) {
            osi_WrStr("crc err ", 9ul);
         }
         p += len;
      }
      /*WrInt(test1, 6); WrInt(test2, 6); WrStrLn(" testcnt"); */
      if (frnum<65536UL && gpstime<86400UL) {
         /* try build serial no */
         i = 10000UL+((gpstime+86400UL)-frnum)%86400UL;
         { /* with */
            struct IMET * anonym2 = &chan[m].nonames->imet;
            if (i==anonym2->idnew) ++anonym2->idcnt;
            else anonym2->idcnt = 0UL;
            if (anonym2->idnew==0UL) anonym2->idnew = i;
         }
      }
      if (verb && !lfdone) osi_WrStrLn("", 1ul);
      if (posok && name[0U]) {
         memset((char *)s,(char)0,201UL); /* end of block is 0 */
         for (i = 0UL; i<=8UL; i++) {
            s[i+7UL] = name[i];
         } /* end for */
         { /* with */
            struct CHAN * anonym3 = &chan[m];
            s[0U] = (char)(anonym3->mycallc/16777216UL);
            s[1U] = (char)(anonym3->mycallc/65536UL&255UL);
            s[2U] = (char)(anonym3->mycallc/256UL&255UL);
            s[3U] = (char)(anonym3->mycallc&255UL);
            if (anonym3->mycallc>0UL) s[4U] = anonym3->myssid;
            else s[4U] = '\020';
            s[5U] = 0;
            s[6U] = 0;
         }
         for (i = 0UL; i<=105UL; i++) {
            s[i+16UL] = anonym->rxbuf[i]; /* payload */
         } /* end for */
         flen = 122UL;
         sdrparm(s, 201ul, &flen, (int32_t)m);
         alludp(chan[m].udptx, flen, s, 201ul);
      }
      monitor(m, "IMET", 5ul, name, 9ul);
   }
} /* end decodeframeimet() */


static void demodbitimet(uint32_t m, char d)
{
   struct IMETA * anonym;
   { /* with */
      struct IMETA * anonym = &chan[m].imeta;
      if (d) anonym->stopc = 0UL;
      else ++anonym->stopc;
      if (anonym->stopc>20UL) {
         /* a while stop steps is end of frame,
                more time avoids decode frames out of noise */
         if (anonym->rxp>=12UL && anonym->rxp<105UL) decodeframeimet(m);
         anonym->rxp = 0UL;
         anonym->rxbitc = 0UL;
      }
      else {
         ++anonym->rxbitc;
         anonym->rxbyte = anonym->rxbyte/2UL;
         if (!d) anonym->rxbyte += 256UL;
         if (anonym->rxbitc>=10UL) {
            if ((anonym->rxp || anonym->rxbyte==257UL) && anonym->rxp<=105UL)
                 {
               /* x01 is start of frame */
               anonym->rxbuf[anonym->rxp] = (char)(anonym->rxbyte&255UL);
               ++anonym->rxp;
            }
            anonym->rxbitc = 0UL;
         }
      }
   }
/* IF d THEN WrStr("0") ELSE WrStr("1") END; */
} /* end demodbitimet() */


static void demodimet(float u, uint32_t channel)
{
   char d;
   float ua;
   struct IMETA * anonym;
   d = u>=0.0f;
   { /* with */
      struct IMETA * anonym = &chan[channel].imeta;
      if (anonym->cbit) {
         demodbitimet(channel, d);
         if (d!=anonym->oldd) {
            if (anonym->stopc==0UL) {
               /* fast dpll lock on byte start,
                less than ideal solution for phase jump on asynchron bytes */
               if (d==anonym->plld) anonym->baudfine += anonym->pllshift*3L;
               else anonym->baudfine -= anonym->pllshift*3L;
            }
            else if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
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
} /* end demodimet() */


static void Afsk202(uint32_t channel)
{
   float ff;
   float b;
   float a;
   float d;
   float mid;
   float right;
   struct IMETA * anonym;
   { /* with */
      struct IMETA * anonym = &chan[channel].imeta;
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
            demodimet(ff, channel);
         }
      }
   }
} /* end Afsk202() */

/*---------------------- MEISEI (share demod with MRZ *) */
static uint32_t sondeudp_SYNWORDMEIS1 = 0x49DCEUL;

static uint32_t sondeudp_SYNWORDMEIS2 = 0xFB6230UL;

/*-bch */
static uint16_t sondeudp_P = 0x1539U;


static char checkbch(const char bb[], uint32_t bb_len,
                uint32_t start)
/* use bch as 12bit crc */
{
   uint32_t i;
   uint16_t s;
   s = 0U;
   for (i = 0UL; i<=33UL; i++) {
      s = X2C_LSH(s,16,1);
      if (((0x1000U & s)!=0)!=(i<34UL && bb[start+i])) s = s^0x1539U;
   } /* end for */
   for (i = 0UL; i<=11UL; i++) {
      if (bb[(start+45UL)-i]!=X2C_IN(i,16,s)) return 0;
   } /* end for */
   return 1;
} /* end checkbch() */

/* correct 0..1 bit */

static int32_t bchcorr2(char bb[], uint32_t bb_len,
                uint32_t start)
/* correct 0..2 bit */
{
   uint32_t j;
   uint32_t i;
   if (checkbch(bb, bb_len, start)) return 0L;
   for (i = 0UL; i<=33UL; i++) {
      bb[start+i] = !bb[start+i];
      if (checkbch(bb, bb_len, start)) return 1L;
      for (j = i+1UL; j<=33UL; j++) {
         bb[start+j] = !bb[start+j];
         if (checkbch(bb, bb_len, start)) return 2L;
         bb[start+j] = !bb[start+j];
      } /* end for */
      bb[start+i] = !bb[start+i];
   } /* end for */
   return -1L;
} /* end bchcorr2() */

/*-bch */

static void meisname(uint32_t n, char s[], uint32_t s_len)
{
   float sn;
   uint32_t i;
   uint32_t tmp;
   tmp = s_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      s[i] = 0;
      if (i==tmp) break;
   } /* end for */
   sn = SaveReal(n);
   if (sn>=1.0f && sn<=1.6777215E+7f) {
      n = (uint32_t)X2C_TRUNCC(sn+0.5f,0UL,X2C_max_longcard)&1048575UL;
      aprsstr_Assign(s, s_len, "IMS", 4ul);
      for (i = 8UL; i>=3UL; i--) {
         if (i<=s_len-1) s[i] = hex(n);
         n = n/16UL;
      } /* end for */
   }
} /* end meisname() */


static uint32_t meisdate(uint32_t d, uint32_t t)
{
   char s[21];
   uint32_t n;
   strncpy(s,"....-..-.. 00:00:00",21u);
   n = 2020UL+d%10UL; /* has to be updated every decade */
   s[0U] = (char)(n/1000UL+48UL);
   s[1U] = (char)((n/100UL)%10UL+48UL);
   s[2U] = (char)((n/10UL)%10UL+48UL);
   s[3U] = (char)(n%10UL+48UL);
   n = (d/10UL)%100UL;
   s[5U] = (char)(n/10UL+48UL);
   s[6U] = (char)(n%10UL+48UL);
   n = (d/1000UL)%100UL;
   s[8U] = (char)(n/10UL+48UL);
   s[9U] = (char)(n%10UL+48UL);
   if (aprsstr_StrToTime(s, 21ul, &n)) n += t;
   else {
      /*    t:=(t+(DAYSEC-GPSTIMECORR)) MOD DAYSEC; */
      n = 0UL;
   }
   return n;
} /* end meisdate() */

#define sondeudp_IMS 0xC1 

#define sondeudp_RS11 0xA2 

#define sondeudp_TYPDISC 0xC0 

#define sondeudp_IDTIME 180
/* stop send if not got actual serial */

#define sondeudp_NOJUNK 4
/* min blocks correct to show frame */

#define sondeudp_AKTIVJUNK 6
/* min blocks correct to stay awake */


static void decodemeisei(uint32_t m)
{
   uint32_t cf;
   uint32_t cfg;
   uint32_t fnum;
   uint32_t fcnt;
   uint32_t date;
   uint32_t bp;
   uint32_t bk;
   uint32_t c;
   uint32_t b;
   uint32_t j;
   uint32_t i;
   uint32_t t;
   double dg;
   uint8_t okbits;
   uint8_t bb[24];
   signed char blockok[6];
   char verb1;
   char nameok;
   char sumok;
   char s[201];
   char name[21];
   struct MEISEI * anonym;
   struct CHAN * anonym0; /* call if set */
   uint32_t tmp;
   { /* with */
      struct MEISEI * anonym = &chan[m].meisei;
      sumok = 0;
      nameok = 0;
      date = 0UL;
      bp = 0UL;
      for (bk = 0UL; bk<=5UL; bk++) {
         /* extract databytes */
         /*    IF checkbch(rxbuf, bk*46) THEN INCL(blockok, bk) END;
                (* as not found a crc use bch as a 12 bit crc,
                data correction will make too much junk *)  */
         blockok[bk] = (signed char)bchcorr2(anonym->rxbuf, 276ul, bk*46UL);
                /* try correct 0 to 2 bit */
         if (blockok[bk]>=0) {
            for (j = 0UL; j<=1UL; j++) {
               /* make 4 databytes out of 34 bits */
               c = 0UL;
               b = 0UL;
               for (i = 0UL; i<=16UL; i++) {
                  b += b; /* check 16+1 bit patrity */
                  if (anonym->rxbuf[i+j*17UL+bk*46UL]) {
                     ++b;
                     ++c;
                  }
               } /* end for */
               b = b/2UL;
               bb[bp] = (uint8_t)(b/256UL);
               ++bp;
               bb[bp] = (uint8_t)(b&255UL);
               ++bp;
               if (!(c&1)) blockok[bk] = -2;
            } /* end for */
         }
      } /* end for */
      okbits = 0U;
      j = 0UL;
      for (bk = 0UL; bk<=5UL; bk++) {
         if (blockok[bk]>=2) ++j;
         else if (blockok[bk]>=0) {
            j += 2UL;
            okbits |= (1U<<bk);
         }
      } /* end for */
      if (j>=6UL) setactiv(&chan[m].mp3h.savecnt, 30L);
      verb1 = verb && j>=4UL || verb2; /* not show too much from noise */
      if (anonym->lostsamps<100UL) okbits |= 0x80U;
      else anonym->timeok = 0;
      anonym->lostsamps = 0UL;
      if (verb1) {
         if ((uint8_t)anonym->ser[0U]>0 && anonym->lastser+120UL>=osic_time()
                ) osi_WrStr(anonym->ser, 11ul);
         else osi_WrStr("Meisei", 7ul);
         osi_WrStr(" ", 2ul);
         for (i = 0UL; i<=5UL; i++) {
            if (blockok[i]==0) osi_WrStr("+", 2ul);
            else if (blockok[i]==1) osi_WrStr("1", 2ul);
            else if (blockok[i]==2) osi_WrStr("2", 2ul);
            else if (blockok[i]==-1) osi_WrStr("-", 2ul);
            else osi_WrStr("!", 2ul);
         } /* end for */
         if (verb2) {
            osi_WrStrLn("", 1ul);
            osi_WrStr("[", 2ul);
            for (i = 0UL; i<=23UL; i++) {
               osi_WrStr(" ", 2ul);
               osi_WrHex((uint32_t)bb[i], 1UL);
            } /* end for */
            osi_WrStrLn("]", 2ul);
         }
      }
      if (anonym->frametyp==1UL) {
         if (blockok[0U]>=0) {
            date = (uint32_t)bb[0U]*256UL+(uint32_t)bb[1U];
            if (date==0UL) anonym->frametyp = 3UL;
            if (anonym->frametyp==1UL) {
               if (anonym->timeok) {
                  for (i = 0UL; i<=10UL; i++) {
                     anonym->gpssum += (uint32_t)
                bb[i*2UL]*256UL+(uint32_t)bb[i*2UL+1UL];
                  } /* end for */
                  sumok = (anonym->gpssum&65535UL)==(uint32_t)
                bb[22U]*256UL+(uint32_t)bb[23U];
                  if (sumok) {
                     anonym->gpstime = meisdate(date, anonym->gpsdaytime);
                  }
               }
               if (verb1) {
                  osi_WrStr("B", 2ul);
                  osic_WrINT32(anonym->subtype, 1UL);
                  if (!sumok) osi_WrStr(" chksum-err", 12ul);
                  osi_WrStr(" ", 2ul);
                  osic_WrINT32(2020UL+date%10UL, 1UL);
                  osi_WrStr("-", 2ul);
                  osic_WrINT32((date/10UL)%100UL, 1UL);
                  osi_WrStr("-", 2ul);
                  osic_WrINT32(date/1000UL, 1UL);
                  if (blockok[1U]>=0) {
                     j = 0UL;
                     for (i = 0UL; i<=3UL; i++) {
                        j = j*256UL+(uint32_t)bb[i+2UL];
                     } /* end for */
                     if (anonym->subtype==1UL) {
                        dg = (double)(j/1000000UL)+(double)
                (j%1000000UL)*1.6666666666667E-6;
                     }
                     else dg = (double)j*1.E-7;
                     osi_WrStr(" lat:", 6ul);
                     osic_WrFixed((float)dg, 5L, 1UL);
                  }
                  if (blockok[1U]>=0 && blockok[2U]>=0) {
                     j = 0UL;
                     for (i = 0UL; i<=3UL; i++) {
                        j = j*256UL+(uint32_t)bb[i+6UL];
                     } /* end for */
                     if (anonym->subtype==1UL) {
                        dg = (double)(j/1000000UL)+(double)
                (j%1000000UL)*1.6666666666667E-6;
                     }
                     else dg = (double)j*1.E-7;
                     osi_WrStr(" long:", 7ul);
                     osic_WrFixed((float)dg, 5L, 1UL);
                  }
                  if (blockok[2U]>=0 && blockok[3U]>=0) {
                     j = 0UL;
                     if (anonym->subtype==1UL) {
                        for (i = 0UL; i<=2UL; i++) {
                           j = j*256UL+(uint32_t)bb[i+10UL];
                        } /* end for */
                     }
                     else {
                        for (i = 0UL; i<=3UL; i++) {
                           j = j*256UL+(uint32_t)bb[i+10UL];
                        } /* end for */
                     }
                     dg = (double)j*0.01;
                     osi_WrStr(" alt:", 6ul);
                     osic_WrFixed((float)dg, 1L, 1UL);
                  }
                  if ((anonym->subtype==1UL && blockok[3U]>=0)
                && blockok[4U]>=0) {
                     if (verb2) {
                        osi_WrStr(" [", 3ul);
                        for (i = 13UL; i<=17UL; i++) {
                           osi_WrHex((uint32_t)bb[i], 1UL);
                        } /* end for */
                        osi_WrStr("]", 2ul);
                     }
                  }
                  if (blockok[4U]>=0) {
                     if (anonym->subtype==1UL) {
                        dg = (double)((uint32_t)
                bb[18U]*256UL+(uint32_t)bb[19U])*0.01;
                     }
                     else {
                        dg = (double)((uint32_t)
                bb[16U]*256UL+(uint32_t)bb[17U])*0.01;
                     }
                     osi_WrStr(" dir:", 6ul);
                     osic_WrFixed((float)dg, 1L, 1UL);
                  }
                  if (anonym->subtype==1UL) {
                     if (blockok[5U]>=0) {
                        dg = (double)((uint32_t)
                bb[20U]*256UL+(uint32_t)bb[21U])*1.851984E-2;
                        osi_WrStr(" kmh:", 6ul);
                        osic_WrFixed((float)dg, 2L, 1UL);
                     }
                  }
                  else if (blockok[4U]>=0) {
                     dg = (double)((uint32_t)
                bb[14U]*256UL+(uint32_t)bb[15U])*0.01;
                     osi_WrStr(" kmh:", 6ul);
                     osic_WrFixed((float)dg, 2L, 1UL);
                  }
                  if (anonym->subtype!=1UL && blockok[4U]>=0) {
                     dg = (double)(int32_t)((uint32_t)
                bb[18U]*256UL+(uint32_t)bb[19U])*0.01;
                     osi_WrStr(" clb:", 6ul);
                     osic_WrFixed((float)dg, 2L, 1UL);
                  }
               }
            }
            else {
               anonym->timeok = 0;
               if (verb1) {
                  osi_WrStr("D", 2ul);
                  osic_WrINT32(anonym->subtype, 1UL);
               }
            }
         }
         else {
            anonym->timeok = 0;
            if (verb1) osi_WrStr("  ", 3ul);
         }
      }
      else {
         /* frametyp 0,2 */
         anonym->timeok = 0;
         anonym->gpssum = (uint32_t)bb[20U]*256UL+(uint32_t)
                bb[21U]+(uint32_t)bb[22U]*256UL+(uint32_t)bb[23U];
         if (blockok[0U]>=0) {
            fcnt = (uint32_t)bb[0U]*256UL+(uint32_t)bb[1U];
            if ((fcnt&1)) anonym->frametyp = 2UL;
            fnum = fcnt/2UL;
            if (verb1) {
               if (anonym->frametyp==2UL) osi_WrStr("C", 2ul);
               else osi_WrStr("A", 2ul);
               osic_WrINT32(anonym->subtype, 1UL);
               osi_WrStr(" fn:", 5ul);
               osic_WrINT32(fnum, 1UL);
            }
            if (blockok[3U]>=0 && (uint32_t)bb[14U]==48UL+(uint32_t)
                (anonym->frametyp==2UL)) {
               anonym->subtype = 1UL+(uint32_t)(bb[15U]<=192U);
               if (verb1) {
                  if (anonym->subtype==1UL) osi_WrStr(" ims100", 8ul);
                  else {
                     osi_WrStr(" ", 2ul);
                     osi_WrHex((uint32_t)bb[15U], 1UL);
                     osi_WrStr("=type?", 7ul);
                  }
               }
            }
            if (anonym->subtype==1UL && blockok[1U]>=0) {
               cf = (uint32_t)bb[4U]*256UL+(uint32_t)bb[5U]+(uint32_t)
                bb[6U]*16777216UL+(uint32_t)bb[7U]*65536UL;
               cfg = (fcnt&63UL)+64UL*(uint32_t)(anonym->frametyp==2UL);
               anonym->config[cfg].d = cf;
               anonym->config[cfg].t = osic_time();
               if (verb1) {
                  osi_WrStr(" calib[", 8ul);
                  osic_WrINT32(cfg, 1UL);
                  osi_WrStr("]:", 3ul);
                  j = cf;
                  for (i = 0UL; i<=3UL; i++) {
                     osi_WrHex(j>>24, 0UL);
                     j = j<<8;
                  } /* end for */
               }
               if (((cfg==0UL || cfg==16UL) || cfg==32UL) || cfg==48UL) {
                  meisname(cf, anonym->ser, 11ul);
                  if (verb1) {
                     osi_WrStr(" ser:", 6ul);
                     osi_WrStr(anonym->ser, 11ul);
                  }
                  anonym->lastser = osic_time();
               }
               else if (cfg==79UL) {
                  if (verb1) {
                     osi_WrStr(" MHz:", 6ul);
                     osic_WrFixed(400.0f+SaveReal(cf)*0.1f, 2L, 1UL);
                  }
               }
            }
            if (blockok[5U]>=0 && anonym->frametyp==0UL) {
               anonym->gpsdaytime = ((uint32_t)bb[20U]*256UL+(uint32_t)
                bb[21U])/1000UL+(uint32_t)bb[22U]*3600UL+(uint32_t)
                bb[23U]*60UL;
               anonym->timeok = anonym->gpsdaytime<86400UL;
               if (verb1) {
                  osi_WrStr(" ", 2ul);
                  wrtime(anonym->gpsdaytime);
               }
            }
         }
         else if (verb1) osi_WrStr("  ", 3ul);
      }
      if (verb1) {
         WrdB(chan[m].admax-chan[m].admin);
         WrQuali(noiselevel(chan[m].mp3h.bitlev, chan[m].mp3h.noise));
         osi_WrStrLn("", 1ul);
      }
      /*-check name */
      if (anonym->subtype==1UL) {
         t = osic_time();
         j = 0UL;
         b = 0UL;
         i = 0UL;
         for (tmp = 3UL;;) {
            if (anonym->config[i].t+180UL>=t && anonym->config[i].d) {
               if (b==0UL) {
                  b = anonym->config[i].d; /* count same actual ser */
                  ++j;
               }
               else if (b==anonym->config[i].d) ++j;
            }
            if (!tmp) break;
            --tmp;
            i += 16UL;
         } /* end for */
         if (j>=2UL) {
            /* found 2 ident ser */
            meisname(b, name, 21ul);
            if (aprsstr_Length(name, 21ul)>=5UL) nameok = 1;
         }
      }
      /*-check name */
      if (nameok && chan[m].framestarttime+3UL>=osic_time()) {
         monitor(m, "MEIS", 5ul, name, 21ul);
         memset((char *)s,(char)0,201UL); /* end of block is 0 */
         for (i = 0UL; i<=8UL; i++) {
            s[i+7UL] = name[i];
         } /* end for */
         { /* with */
            struct CHAN * anonym0 = &chan[m];
            s[0U] = (char)(anonym0->mycallc/16777216UL);
            s[1U] = (char)(anonym0->mycallc/65536UL&255UL);
            s[2U] = (char)(anonym0->mycallc/256UL&255UL);
            s[3U] = (char)(anonym0->mycallc&255UL);
            if (anonym0->mycallc>0UL) {
               s[4U] = anonym0->myssid;
            }
            else s[4U] = '\020';
            s[5U] = 0;
            s[6U] = 0;
         }
         s[16U] = (char)(anonym->gpstime>>24);
         s[17U] = (char)(anonym->gpstime>>16);
         s[18U] = (char)(anonym->gpstime>>8);
         s[19U] = (char)anonym->gpstime;
         s[20U] = (char)okbits;
         if ((anonym->frametyp&1)) {
            s[21U] = (char)0xFBUL;
            s[22U] = (char)0xFB62UL;
            s[23U] = (char)0xFB6230UL;
         }
         else {
            s[21U] = (char)0x4UL;
            s[22U] = (char)0x49DUL;
            s[23U] = (char)0x49DCEUL;
         }
         for (i = 0UL; i<=23UL; i++) {
            s[24UL+i] = bb[i];
         } /* end for */
         i = 48UL;
         sdrparm(s, 201ul, &i, (int32_t)m);
         alludp(chan[m].udptx, i, s, 201ul);
      }
   }
} /* end decodemeisei() */

#define sondeudp_MAXERR 2
/* allow wrong bits in sync word */


static char cmpsyn(uint32_t w, uint32_t s)
{
   uint32_t j;
   uint32_t i;
   w = w&0xFFFFFFUL^s;
   j = 0UL;
   for (i = 0UL; i<=23UL; i++) {
      if (X2C_IN(i,32,w)) {
         /* wrong bit */
         ++j;
         if (j>2UL) return 0;
      }
   } /* end for */
   return 1;
} /* end cmpsyn() */


static void demodbytemeisei(uint32_t m, char d)
{
   struct MEISEI * anonym;
   /*  IF chan[m].mp3h.savecnt>0 THEN */
   { /* with */
      struct MEISEI * anonym = &chan[m].meisei;
      anonym->synword += anonym->synword+(uint32_t)d;
      if (anonym->rxb>=276UL) {
         /* out of frame */
         ++anonym->lostsamps; /* check for time frame fits to date */
         if (cmpsyn((uint32_t)anonym->synword, 0x49DCEUL)) {
            anonym->rxb = 0UL;
            chan[m].framestarttime = osic_time();
            chan[m].meisei.frametyp = 0UL;
         }
         else if (cmpsyn((uint32_t)anonym->synword, 0xFB6230UL)) {
            anonym->rxb = 0UL;
            chan[m].framestarttime = osic_time();
            chan[m].meisei.frametyp = 1UL;
         }
      }
      else {
         anonym->rxbuf[anonym->rxb] = d;
         ++anonym->rxb;
         if (anonym->rxb==276UL) decodemeisei(m);
      }
   }
/*  END; */
} /* end demodbytemeisei() */

/*---------------------- MRZ MP3-H1 (share demod with Meisei) */

static int32_t cint16(int32_t v)
{
   if (v>32767L) v -= 65536L;
   return v;
} /* end cint16() */


static void Whex(uint32_t n)
{
   char s[4];
   s[0U] = hex(n/4096UL);
   s[1U] = hex(n/256UL);
   s[2U] = hex(n/16UL);
   s[3U] = hex(n);
   osi_WrStr(s, 4ul);
} /* end Whex() */

#define sondeudp_POLY1 0xA001 


static char crcmp3(const char b[], uint32_t b_len,
                uint32_t from, uint32_t to)
{
   uint32_t j;
   uint32_t i;
   uint16_t c;
   uint32_t tmp;
   c = 0xFFFFU;
   tmp = to-1UL;
   i = from;
   if (i<=tmp) for (;; i++) {
      c = c^(uint16_t)(uint32_t)(uint8_t)b[i];
      for (j = 0UL; j<=7UL; j++) {
         if ((0x1U & c)) c = X2C_LSH(c,16,-1)^0xA001U;
         else c = X2C_LSH(c,16,-1);
      } /* end for */
      if (i==tmp) break;
   } /* end for */
   return c==(uint16_t)cardmsb(b, b_len, to, 2UL);
} /* end crcmp3() */


static void mp3id(uint32_t m, char ser, char id[],
                uint32_t id_len)
{
   char s[21];
   uint32_t n;
   struct MP3H * anonym;
   id[0UL] = 0;
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      if (anonym->id1ok && anonym->id2ok) {
         if (ser) {
            /*serial */
            aprsstr_Assign(id, id_len, "MRZ-", 5ul);
            aprsstr_CardToStr(anonym->id1, 1UL, s, 21ul);
            aprsstr_Append(id, id_len, s, 21ul);
            aprsstr_Append(id, id_len, "-", 2ul);
            aprsstr_CardToStr(anonym->id2, 1UL, s, 21ul);
         }
         else {
            /*aprs */
            aprsstr_Assign(id, id_len, "MRZ", 4ul);
            n = anonym->id1*100000UL+anonym->id2;
            s[0U] = hex(n/1048576UL);
            s[1U] = hex(n/65536UL);
            s[2U] = hex(n/4096UL);
            s[3U] = hex(n/256UL);
            s[4U] = hex(n/16UL);
            s[5U] = hex(n);
            s[6U] = 0;
         }
         aprsstr_Append(id, id_len, s, 21ul);
      }
   }
} /* end mp3id() */


static uint32_t mp3time(char b[], uint32_t b_len)
{
   uint32_t mp3time_ret;
   X2C_PCOPY((void **)&b,b_len);
   mp3time_ret = cardmsb(b, b_len, 1UL, 1UL)*3600UL+cardmsb(b, b_len, 2UL,
                1UL)*60UL+cardmsb(b, b_len, 3UL, 1UL);
   X2C_PFREE(b);
   return mp3time_ret;
} /* end mp3time() */


static uint32_t mp3date(uint32_t d, uint32_t timedate, uint32_t t)
{
   char s[21];
   uint32_t n;
   strncpy(s,"....-..-.. 00:00:00",21u);
   n = 2000UL+d%100UL;
   s[0U] = (char)(n/1000UL+48UL);
   s[1U] = (char)((n/100UL)%10UL+48UL);
   s[2U] = (char)((n/10UL)%10UL+48UL);
   s[3U] = (char)(n%10UL+48UL);
   n = (d/100UL)%100UL;
   s[5U] = (char)(n/10UL+48UL);
   s[6U] = (char)(n%10UL+48UL);
   n = (d/10000UL)%100UL;
   s[8U] = (char)(n/10UL+48UL);
   s[9U] = (char)(n%10UL+48UL);
   if (aprsstr_StrToTime(s, 21ul, &n)) {
      n += t; /* add daytime to date */
      if (t<timedate) n += 86400UL;
      t = (t+86382UL)%86400UL;
   }
   else n = 0UL;
   return n;
} /* end mp3date() */


static void showmp3(uint32_t m, uint32_t state, uint32_t repaired,
                const char rxbuf[], uint32_t rxbuf_len)
{
   uint32_t cnt;
   uint32_t cfg;
   uint32_t i;
   double clb;
   double dir;
   double kmh;
   double heig;
   double long0;
   double lat;
   double vz;
   double vy;
   double vx;
   double wz;
   double wy;
   double wx;
   char done;
   char s[21];
   done = 0;
   cnt = cardmsb(rxbuf, rxbuf_len, 0UL, 1UL)&15UL;
   if (state==1UL) {
      /* crc good */
      WrChan((int32_t)m);
      osi_WrStr("MRZ ", 5ul);
      osic_WrINT32(cnt, 2UL);
      osi_WrStr(" ", 2ul);
      mp3id(m, 0, s, 21ul);
      if (s[0U]) {
         osi_WrStr(s, 21ul);
         osi_WrStr(" ", 2ul);
      }
      if (chan[m].mp3h.gpstime>0UL) {
         aprsstr_DateToStr(chan[m].mp3h.gpstime, s, 21ul);
         osi_WrStr(s, 21ul);
         osi_WrStr(" ", 2ul);
      }
      wx = (double)(int32_t)cardmsb(rxbuf, rxbuf_len, 5UL, 4UL)*0.01;
      wy = (double)(int32_t)cardmsb(rxbuf, rxbuf_len, 9UL, 4UL)*0.01;
      wz = (double)(int32_t)cardmsb(rxbuf, rxbuf_len, 13UL,
                4UL)*0.01;
      vx = (double)cint16((int32_t)cardmsb(rxbuf, rxbuf_len, 17UL,
                2UL))*0.01;
      vy = (double)cint16((int32_t)cardmsb(rxbuf, rxbuf_len, 19UL,
                2UL))*0.01;
      vz = (double)cint16((int32_t)cardmsb(rxbuf, rxbuf_len, 21UL,
                2UL))*0.01;
      wgs84r(wx, wy, wz, &lat, &long0, &heig);
      osic_WrFixed((float)(X2C_DIVL(lat,1.7453292519943E-2)), 5L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(long0,1.7453292519943E-2)), 5L, 1UL);
      if (heig<1.E+5 && heig>(-1.E+5)) {
         osi_WrStr(" ", 2ul);
         osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(heig,X2C_min_longint,
                X2C_max_longint), 1UL);
         osi_WrStr("m", 2ul);
      }
      speeddir(lat, long0, vx, vy, vz, &kmh, &dir, &clb);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)kmh, 1L, 1UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(dir,X2C_min_longint,
                X2C_max_longint), 1UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)clb, 1L, 1UL);
      osi_WrStr("m/s ", 5ul);
      osic_WrINT32((uint32_t)(uint8_t)rxbuf[23UL], 1UL);
      osi_WrStr("sat", 4ul);
      if (verb2) {
         osi_WrStr(" c1=", 5ul);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[4UL], 2UL);
         osi_WrStr(" a1=", 5ul);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[24UL], 2UL);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[25UL], 2UL);
         osi_WrStr(" a2=", 5ul);
         osic_WrINT32(cardmsb(rxbuf, rxbuf_len, 26UL, 2UL), 1UL);
         osi_WrStr(" a3=", 5ul);
         osic_WrINT32(cardmsb(rxbuf, rxbuf_len, 28UL, 2UL), 1UL);
         osi_WrStr(" c2=", 5ul);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[30UL], 2UL);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[31UL], 2UL);
         osi_WrStr(" a4=", 5ul);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[32UL], 2UL);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[33UL], 2UL);
         osi_WrStr(" a5=", 5ul);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[34UL], 2UL);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[35UL], 2UL);
         osi_WrStr(" a6=", 5ul);
         osic_WrINT32(cardmsb(rxbuf, rxbuf_len, 36UL, 2UL), 1UL);
         osi_WrStr(" c3=", 5ul);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[38UL], 2UL);
         osi_WrHex((uint32_t)(uint8_t)rxbuf[39UL], 2UL);
         osi_WrStr(" n=", 4ul);
         osic_WrINT32((uint32_t)(uint8_t)rxbuf[40UL], 1UL);
      }
      cfg = cardmsb(rxbuf, rxbuf_len, 41UL, 4UL);
      if (cnt==15UL) {
         osi_WrStr(" date=", 7ul);
         osic_WrINT32(2000UL+cfg%100UL, 1UL);
         osi_WrStr("-", 2ul);
         osic_WrINT32((cfg/100UL)%100UL, 1UL);
         osi_WrStr("-", 2ul);
         osic_WrINT32((cfg/10000UL)%100UL, 1UL);
      }
      else if (verb2) {
         osi_WrStr(" cfg=", 6ul);
         Whex(cfg/65536UL);
         Whex(cfg);
      }
      if (repaired>0UL) {
         osi_WrStr(" rep=", 6ul);
         osic_WrINT32(repaired, 1UL);
      }
      if (verb2) {
         for (i = 0UL; i<=48UL; i++) {
            if (i%25UL==0UL) osi_WrStrLn("", 1ul);
            osi_WrHex((uint32_t)(uint8_t)rxbuf[i], 3UL);
         } /* end for */
      }
      done = 1;
   }
   else if (repaired>2UL || verb2) {
      /* do not show single noise frames as fills screen */
      WrChan((int32_t)m);
      osi_WrStr("MRZ ", 5ul);
      osic_WrINT32(cnt, 2UL);
      if (state==0UL) {
         osi_WrStr(" crc error copies=", 19ul);
         osic_WrINT32(repaired, 1UL);
      }
      else osi_WrStr(" dupe", 6ul);
      done = 1;
   }
   if (done) {
      WrdB(chan[m].admax-chan[m].admin);
      WrQuali(noiselevel(chan[m].mp3h.bitlev, chan[m].mp3h.noise));
      osi_WrStrLn("", 1ul);
   }
} /* end showmp3() */


static void decodeframemp3(uint32_t m, const char buf[],
                uint32_t buf_len)
{
   uint32_t cnt;
   uint32_t cfg;
   uint32_t flen;
   uint32_t i;
   char name[21];
   char s[201];
   struct MP3H * anonym;
   struct CHAN * anonym0; /* call if set */
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      cnt = cardmsb(buf, buf_len, 0UL, 1UL)&15UL;
      cfg = cardmsb(buf, buf_len, 41UL, 4UL);
      if (cnt==15UL) {
         anonym->dateok = 1;
         anonym->gpsdate = cfg;
         anonym->timeatdate = mp3time(buf, buf_len); /* to check day wrap */
      }
      else if (cnt==13UL) {
         anonym->id2 = cfg;
         anonym->id2ok = 1;
      }
      else if (cnt==12UL) {
         anonym->id1 = cfg;
         anonym->id1ok = 1;
      }
      if (anonym->dateok) {
         anonym->gpstime = mp3date(anonym->gpsdate, anonym->timeatdate,
                mp3time(buf, buf_len));
      }
      setactiv(&anonym->savecnt, 30L);
      mp3id(m, 0, name, 21ul);
      if (anonym->gpstime>0UL && name[0U]) {
         memset((char *)s,(char)0,201UL); /* end of block is 0 */
         for (i = 0UL; i<=8UL; i++) {
            s[i+7UL] = name[i];
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
         s[16U] = (char)(anonym->gpstime/16777216UL);
         s[17U] = (char)(anonym->gpstime/65536UL&255UL);
         s[18U] = (char)(anonym->gpstime/256UL&255UL);
         s[19U] = (char)(anonym->gpstime&255UL);
         for (i = 0UL; i<=48UL; i++) {
            s[i+20UL] = buf[i]; /* payload */
         } /* end for */
         flen = 69UL;
         sdrparm(s, 201ul, &flen, (int32_t)m);
         alludp(chan[m].udptx, flen, s, 201ul);
      }
      monitor(m, "MRZ", 4ul, name, 21ul);
   }
} /* end decodeframemp3() */


static void repairmp3(uint32_t m)
{
   uint32_t rprcnt;
   uint32_t done;
   uint32_t cnt;
   uint32_t max0;
   uint32_t k;
   uint32_t j;
   uint32_t i;
   uint32_t tlast;
   uint32_t t;
   char ch;
   MP3BUF rxbuf;
   struct MP3H * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   rprcnt = 0UL;
   t = osic_time();
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      done = 0UL;
      memcpy(rxbuf,anonym->copybuf[anonym->copycnt].dat,50u);
      tlast = anonym->copybuf[anonym->copycnt].time0;
      for (;;) {
         /*FOR i:=0 TO FLENMP3-1 DO WrHex(ORD(rxbuf[i]),3); END; WrStrLn("");
                 */
         if (crcmp3(rxbuf, 50ul, 0UL, 45UL)) done += 2UL;
         if (done>0UL) break;
         /*- got a crc error frame */
         j = 0UL;
         tmp = anonym->copycnt;
         i = 0UL;
         if (i<=tmp) for (;; i++) {
            if (anonym->copybuf[i].time0+4UL>=t) {
               anonym->copybuf[j] = anonym->copybuf[i];
               ++j;
            }
            if (i==tmp) break;
         } /* end for */
         rprcnt = j;
         anonym->copycnt = j;
         if (anonym->copycnt>=3UL) {
            /* enough noisy frames to find max count same bytes */
            for (i = 0UL; i<=46UL; i++) {
               /* find best byte */
               max0 = 0UL;
               tmp = anonym->copycnt-2UL;
               j = 0UL;
               if (j<=tmp) for (;; j++) {
                  cnt = 0UL;
                  ch = anonym->copybuf[j].dat[i];
                  tmp0 = anonym->copycnt-1UL;
                  k = j+1UL;
                  if (k<=tmp0) for (;; k++) {
                     if (ch==anonym->copybuf[k].dat[i]) ++cnt;
                     if (k==tmp0) break;
                  } /* end for */
                  if (cnt>=max0) {
                     max0 = cnt;
                     rxbuf[i] = anonym->copybuf[j].dat[i];
                  }
                  if (j==tmp) break;
               } /* end for */
            } /* end for */
            if (anonym->copycnt>5UL) {
               anonym->copycnt = 5UL;
               for (i = 1UL; i<=5UL; i++) {
                  anonym->copybuf[i-1UL] = anonym->copybuf[i];
               } /* end for */
            }
         }
         done = 1UL;
      }
      if (done>=2UL) {
         /* good or repaired frame */
         if (tlast+4UL>=t && (anonym->blocknum!=((uint32_t)(uint8_t)
                rxbuf[0U]&15UL) || anonym->blocktime+16UL<t)) {
            /* new block */
            anonym->blocknum = (uint32_t)(uint8_t)rxbuf[0U]&15UL;
            anonym->blocktime = t;
            decodeframemp3(m, rxbuf, 50ul);
            if (verb) showmp3(m, 1UL, rprcnt, rxbuf, 50ul);
         }
         else if (verb) showmp3(m, 2UL, rprcnt, rxbuf, 50ul);
         anonym->copycnt = 0UL;
      }
      if (verb && done==1UL) showmp3(m, 0UL, rprcnt, rxbuf, 50ul);
   }
} /* end repairmp3() */

static uint32_t sondeudp_SYNWORD = 0xBF35UL;

static uint32_t sondeudp_SYNWORDREV = 0x40CAUL;


static void demodbytemp3(uint32_t m, char d)
{
   struct MP3H * anonym;
   /*  IF savecnt>0 THEN */
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      anonym->synword = anonym->synword*2UL+(uint32_t)d;
      if (anonym->rxp>=49UL) {
         /* out of frame */
         if (((uint32_t)anonym->synword&0xFFFFUL)==0xBF35UL) {
            anonym->rxp = 0UL;
            anonym->copybuf[anonym->copycnt].time0 = osic_time();
         }
         else if (((uint32_t)anonym->synword&0xFFFFUL)==0x40CAUL) {
            anonym->polarity = !anonym->polarity;
            anonym->rxp = 0UL;
            anonym->copybuf[anonym->copycnt].time0 = osic_time();
         }
      }
      else {
         ++anonym->rxb;
         if (anonym->rxb>=8UL) {
            anonym->copybuf[anonym->copycnt].dat[anonym->rxp] = (char)
                (anonym->synword&255UL);
            anonym->rxb = 0UL;
            ++anonym->rxp;
            if (anonym->rxp==49UL) repairmp3(m);
         }
      }
   }
/*  END;  */
} /* end demodbytemp3() */


static void demodbitmp3(uint32_t m, float u, float u0)
{
   char dm;
   float ua;
   struct MP3H * anonym;
   dm = u0<0.0f;
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      if (anonym->lastmanch==u0<0.0f) {
         anonym->manchestd += (32767L-anonym->manchestd)/16L;
      }
      anonym->lastmanch = u0<0.0f;
      anonym->manchestd = -anonym->manchestd;
      if (anonym->manchestd<0L) {
         demodbytemp3(m, u>=u0==anonym->polarity);
         demodbytemeisei(m, dm==anonym->dmeis);
         /*quality*/
         ua = (float)fabs(u-u0)-anonym->bitlev;
         anonym->bitlev = anonym->bitlev+ua*0.005f;
         anonym->noise = anonym->noise+((float)fabs(ua)-anonym->noise)
                *0.02f;
      }
      /*quality*/
      anonym->dmeis = dm;
   }
} /* end demodbitmp3() */


static void demodmp3(float u, uint32_t m)
{
   char d;
   struct MP3H * anonym;
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      d = u>=0.0f;
      if (anonym->cbit) {
         demodbitmp3(m, u, anonym->lastu);
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
} /* end demodmp3() */


static void Fskmp3(uint32_t m)
{
   float ff;
   int32_t lim;
   struct MP3H * anonym;
   { /* with */
      struct MP3H * anonym = &chan[m].mp3h;
      lim = (int32_t)anonym->demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (uint32_t)((anonym->baudfine&65535L)/4096L),
                16UL, chan[m].afir, 32ul, anonym->afirtab, 512ul);
            demodmp3(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fskmp3() */

/*---------------------- */
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
               /* crc is end of frame */
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
               if (ww<=4UL) anonym->watertemp[ww].cmd = 0;
               anonym->waterbits = 0UL;
               if (crc==anonym->waterword) {
                  memcpy(anonym->waterdata,anonym->watertemp,
                sizeof(struct _1 [5]));
                  anonym->waterok = 1;
                  anonym->watersend = osic_time();
                  i = 0UL;
                  for (;;) {
                     if (i>4UL || anonym->waterdata[i].cmd==0) break;
                     if (anonym->waterdata[i].cmd=='f') {
                        anonym->chlabel = anonym->waterdata[i].dat;
                /* use freq as channel name */
                        break;
                     }
                     ++i;
                  }
               }
            }
            else if (ww<=4UL) {
               /* store byte + word */
               anonym->watertemp[ww].cmd = anonym->waterbyte;
               anonym->watertemp[ww].dat = anonym->waterword;
            }
         }
         ++anonym->waterbits;
      }
   }
} /* end DecWater() */


static pNONAMES findcontext(uint32_t label)
{
   uint32_t i;
   pNONAMES pn;
   pNONAMES po;
   pNONAMES pl;
   uint32_t t;
   char old;
   t = osic_time();
   i = 0UL;
   pl = 0;
   for (;;) {
      /* look at all channels */
      pl = chan[i].nonames;
      if (pl && pl->chname==label) {
         /* label is on another channel */
         chan[i].nonames = 0; /* remove it */
         /*IF verb THEN WrStr("found context "); WrChName(label);
                WrStr(" on channel "); WrInt(i+1,1); WrStrLn("") END; */
         break;
      }
      ++i;
      if (i>63UL) break;
   }
   if (pl==0) {
      /* not found on channels */
      pl = oldnonames; /* history of contexts */
      po = 0;
      for (;;) {
         /* look in history and purge */
         if (pl==0) break;
         pn = pl->next;
         old = pl->lastvalid && pl->lastvalid+1800UL<t;
         if (old || pl->chname==label) {
            /* remove from chain */
            if (po==0) oldnonames = pn;
            else po->next = pn;
            if (old) {
               osic_free((char * *) &pl, sizeof(struct NONAMES));
               pl = 0;
            }
            else {
               /*IF verb THEN WrStr("found context "); WrChName(label);
                WrStrLn(" on history") END; */
               break;
            }
         }
         else po = pl;
         pl = pn;
      }
   }
   if ((pl && pl->lastvalid) && pl->lastvalid+1800UL<t) {
      osic_free((char * *) &pl, sizeof(struct NONAMES));
      pl = 0;
      if (verb) {
         osi_WrStr("free timed out context ", 24ul);
         WrChName(label);
         osi_WrStrLn("", 1ul);
      }
   }
   return pl;
} /* end findcontext() */


static void sdec(int32_t savelev, uint8_t en, int32_t * cnt,
                int32_t dur)
/* set on/off intervall of inactiv demod */
{
   if (en) {
      if (en==sondeudp_SLEEP) --*cnt;
      if (*cnt<=-savelev) *cnt = dur;
   }
/*WrInt(cnt, 6); WrStrLn(" sc"); */
} /* end sdec() */


static void cpusave(int32_t savelev, uint32_t chans)
{
   uint32_t c;
   struct CHAN * anonym;
   uint32_t tmp;
   tmp = chans;
   c = 0UL;
   if (c<=tmp) for (;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         sdec(savelev, anonym->r92.enabled, &anonym->r92.savecnt, 2L);
         sdec(savelev, anonym->r41.enabled, &anonym->r41.savecnt, 2L);
         sdec(savelev, anonym->c34a.enabled, &anonym->c34a.savecnt, 1L);
         sdec(savelev, anonym->dfm6a.enabled, &anonym->dfm6a.savecnt, 1L);
         sdec(savelev, anonym->imeta.enabled, &anonym->imeta.savecnt, 2L);
         sdec(savelev, anonym->m10.enabled, &anonym->m10.savecnt, 1L);
         sdec(savelev, anonym->mp3h.enabled, &anonym->mp3h.savecnt, 1L);
      }
      if (c==tmp) break;
   } /* end for */
} /* end cpusave() */


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
      ++adcbufrd;
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
   ++sampcount;
   if (sampcount>adcrate && savelevel>0UL) {
      /* every 1 second */
      sampcount = 0UL;
      cpusave((int32_t)savelevel, maxchannels);
                /* lull inactive modems to sleep */
   }
   tmp = maxchannels;
   c = 0UL;
   if (c<=tmp) for (;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         if (anonym->squelch<64UL) {
            /* squelch open */
            if (anonym->r92.savecnt>0L || anonym->r41.savecnt>0L) Fsk(c);
            if ((anonym->c34a.savecnt>0L || anonym->dfm6a.savecnt>0L)
                || anonym->imeta.savecnt>0L) {
               /* variable contexts */
               if (anonym->chlabel) {
                  /* got label from sdr */
                  if (anonym->nonames && anonym->chlabel!=anonym->nonames->chname)
                 {
                     /* channel has a different label */
                     if (anonym->nonames->chname) {
                        /* channel has a label */
                        if (verb) {
                           osi_WrStr("save context ", 14ul);
                           WrChName(anonym->nonames->chname);
                           osi_WrStr(" from channel ", 15ul);
                           osic_WrINT32(c+1UL, 1UL);
                           osi_WrStrLn("", 1ul);
                        }
                        anonym->nonames->next = oldnonames;
                /* move context to storage */
                        oldnonames = anonym->nonames;
                        anonym->nonames = 0;
                     }
                     else {
                        osic_free((char * *) &anonym->nonames,
                sizeof(struct NONAMES));
                        anonym->nonames = 0;
                     }
                  }
                  if (anonym->nonames==0) {
                     anonym->nonames = findcontext(anonym->chlabel);
                /* fitting context or nil */
                  }
               }
               if (anonym->nonames==0) {
                  osic_alloc((char * *) &anonym->nonames,
                sizeof(struct NONAMES));
                  if (anonym->nonames) {
                     memset((char *)anonym->nonames,(char)0,
                sizeof(struct NONAMES));
                  }
               }
               if (anonym->nonames) {
                  /* not out of memory */
                  if (anonym->chlabel) {
                     anonym->nonames->chname = anonym->chlabel;
                /* give context a label */
                     anonym->chlabel = 0UL;
                  }
                  if (anonym->c34a.savecnt>0L) Afsk(c);
                  if (anonym->dfm6a.savecnt>0L) Fsk6(c);
                  if (anonym->imeta.savecnt>0L) Afsk202(c);
               }
            }
            if (anonym->m10.savecnt>0L) Fsk10(c);
            if (anonym->mp3h.savecnt>0L) Fskmp3(c);
         }
      }
      if (c==tmp) break;
   } /* end for */
} /* end getadc() */

static uint16_t sondeudp_POLY2 = 0x1021U;


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
   if (sizeof(SET256)!=32) X2C_ASSERT(0);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(CNAMESTR)!=9) X2C_ASSERT(0);
   if (sizeof(MP3BUF)!=50) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   Gencrctab();
   GenCRC32tab();
   monitorudp = 0;
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
   oldnonames = 0;
   cpusave(0L, 63UL);
   for (;;) getadc();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
