/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* "@(#)sondemod.c Apr 19 13:36:16 2015" */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define sondemod_C_
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
#ifndef TimeConv_H_
#include "TimeConv.h"
#endif
#ifndef gpspos_H_
#include "gpspos.h"
#endif
#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif

/* demodulate RS92 sonde (2400bit/s manchester) by OE5DXL */
/*FROM reedsolomon IMPORT decode, encode, CRC; */
/*FROM rs IMPORT initialize_ecc, encode_data; */
/*IMPORT IO; */
#define sondemod_ADCBUFLEN 4096

#define sondemod_BAUDSAMP 65536

#define sondemod_PLLSHIFT 1024

#define sondemod_AFIRLEN 512

#define sondemod_AOVERSAMP 16
/*16*/

#define sondemod_ASYNBITS 10

static char sondemod_CALIBFRAME = 'e';

static char sondemod_GPSFRAME = 'g';

static char sondemod_UNKNOWN = 'h';

static char sondemod_DATAFRAME = 'i';

#define sondemod_PI 3.1415926535898

#define sondemod_RAD 1.7453292519943E-2

#define sondemod_MYLAT 8.4214719496019E-1
/* only for show sat elevations if no pos decode */

#define sondemod_MYLONG 2.2755602787502E-1

#define sondemod_NEWALMAGE 30

typedef char FILENAME[1024];

enum CHANNELS {sondemod_LEFT, sondemod_RIGHT};


typedef float AFIRTAB[8192];

struct CHAN;


struct CHAN {
   long dds;
   long adcmax;
   long actmodem;
   float afir[512];
   long configequalizer;
   long baudfine;
   long pllshift;
   long manchestd;
   float tcnt;
   float noise;
   float sqmed[2];
   unsigned long fin;
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

typedef char OBJNAME[9];

struct CONTEXT;


struct CONTEXT {
   char calibdata[512];
   unsigned long calibok;
   char mesok;
   char posok;
   char framesent;
   double lat;
   double long0;
   double heig;
   double speed;
   double dir;
   double climb;
   double lastlat;
   double laslong;
   double lastalt;
   double lastspeed;
   double lastdir;
   double lastclb;
   float hrmsc;
   float vrmsc;
   double hp;
   double hyg;
   double temp;
   unsigned long timems;
   unsigned long framenum;
};

static long soundfd;

static long debfd;

static char abortonsounderr;

static FILENAME semfile;

static FILENAME yumafile;

static FILENAME rinexfile;

static unsigned long getst;

static unsigned long i;

static unsigned long afin;

static unsigned long soundbufs;

static unsigned long adcrate;

static unsigned long adcbuflen;

static unsigned long adcbytes;

static unsigned long fragmentsize;

static unsigned long clock0;

static unsigned long almread;

/* time last almanach read */
static unsigned long almrequest;

/* seconds rinex age to request new */
static unsigned long almage;

static unsigned long systime;

static float SIN[32768];

static FILENAME soundfn;

static struct CHAN chan[2];

static unsigned char maxchannels;

static unsigned long demodbaud;

static unsigned long configbaud;

static struct CONTEXT context;

static gpspos_SATS lastsat;

static float coeff[256];

static float mhz;

static OBJNAME objname;

static long rxsock;

static unsigned long maxalmage;

static unsigned long lastip;

static unsigned long lastport;


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
   unsigned long i0;
   unsigned long tmp;
   tmp = f_len-1;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      f[i0] = f[i0]*(0.54f+0.46f*RealMath_cos(3.141592f*(X2C_DIVR((float)i0,
                (float)(1UL+(f_len-1))))));
      if (i0==tmp) break;
   } /* end for */
} /* end Hamming() */


static float pow0(float x, unsigned long y)
{
   float z;
   z = x;
   while (y>1UL) {
      z = z*x;
      --y;
   }
   return z;
} /* end pow() */


static void initafir(AFIRTAB atab, unsigned long F0, unsigned long F1,
                float eq)
{
   unsigned long f;
   unsigned long i0;
   float t[4096];
   float f10;
   float f00;
   float e;
   unsigned long tmp;
   f00 = X2C_DIVR((float)(F0*512UL),(float)adcrate);
   f10 = X2C_DIVR((float)(F1*512UL),(float)adcrate);
   for (i0 = 0UL; i0<=4095UL; i0++) {
      t[i0] = 0.0f;
   } /* end for */
   tmp = (unsigned long)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL;
   f = (unsigned long)X2C_TRUNCC(f00,0UL,X2C_max_longcard);
   if (f<=tmp) for (;; f++) {
      e = 1.0f+eq*((X2C_DIVR((float)f,X2C_DIVR((float)((F0+F1)*512UL),
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
         for (i0 = 0UL; i0<=4095UL; i0++) {
            t[i0] = t[i0]+e*0.5f;
         } /* end for */
      }
      else {
         for (i0 = 0UL; i0<=4095UL; i0++) {
            t[i0] = t[i0]+e*RealMath_cos(X2C_DIVR(3.141592f*(float)(i0*f),
                4096.0f));
         } /* end for */
      }
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 4096ul);
   for (i0 = 0UL; i0<=4095UL; i0++) {
      atab[4095UL+i0] = t[i0];
      atab[4095UL-i0] = t[i0];
   } /* end for */
   if (F0>0UL) {
      /* make dc level zero */
      e = 0.0f;
      for (i0 = 0UL; i0<=8191UL; i0++) {
         e = e+atab[i0];
      } /* end for */
      e = X2C_DIVR(e,8192.0f);
      for (i0 = 0UL; i0<=8191UL; i0++) {
         atab[i0] = atab[i0]-e;
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
   long i0;
   soundfd = osi_OpenRW(soundfn, 1024ul);
   if (soundfd>=0L) {
      i0 = samplesize(soundfd, 16UL); /* 8, 16 */
      i0 = channels(soundfd, (unsigned long)maxchannels+1UL); /* 1, 2  */
      i0 = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
      if (i0) {
         InOut_WriteString("sound setfragment returns ", 27ul);
         InOut_WriteInt(i0, 1UL);
         osi_WrLn();
      }
      i0 = sampelrate(soundfd, adcrate); /* 8000..48000 */
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


static char GetNum(const char h[], unsigned long h_len, char eot,
                unsigned long * p, unsigned long * n)
{
   *n = 0UL;
   while ((unsigned char)h[*p]>='0' && (unsigned char)h[*p]<='9') {
      *n = ( *n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static void Config(void)
{
   unsigned long i0;
   unsigned char c;
   struct CHAN * anonym;
   for (c = sondemod_LEFT;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         configbaud = 4800UL;
         demodbaud = (2UL*configbaud*65536UL)/adcrate;
         initafir(anonym->afirtab, 300UL, 3600UL,
                X2C_DIVR((float)anonym->configequalizer,100.0f));
         anonym->baudfine = 0L;
         anonym->tcnt = 0.0f;
         anonym->cbit = 0;
         anonym->rxp = 0UL;
         anonym->rxbitc = 0UL;
         anonym->manchestd = 0L;
         anonym->lastmanch = 0;
         anonym->rxbyte = 0UL;
         for (i0 = 0UL; i0<=9UL; i0++) {
            anonym->asynst[i0] = 0L;
         } /* end for */
      }
      if (c==sondemod_RIGHT) break;
   } /* end for */
} /* end Config() */


static void Parms(void)
{
   char err;
   char lowbeacon;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   unsigned long cnum;
   long inum;
   unsigned char channel;
   struct CHAN * anonym;
   semfile[0] = 0;
   yumafile[0] = 0;
   rinexfile[0] = 0;
   err = 0;
   abortonsounderr = 0;
   adcrate = 16000UL;
   adcbytes = 2UL;
   adcbuflen = 1024UL;
   fragmentsize = 12UL;
   maxchannels = sondemod_LEFT;
   debfd = -1L;
   rxsock = -1L;
   sondeaprs_dao = 0;
   lowbeacon = 0;
   maxalmage = 21600UL;
   almrequest = 14400UL;
   sondeaprs_verb = 0;
   sondeaprs_verb2 = 0;
   for (channel = sondemod_LEFT;; channel++) {
      { /* with */
         struct CHAN * anonym = &chan[channel];
         anonym->configequalizer = 0L;
         anonym->pllshift = 1024L;
      }
      if (channel==sondemod_RIGHT) break;
   } /* end for */
   channel = sondemod_LEFT;
   for (cnum = 0UL; cnum<=32767UL; cnum++) {
      SIN[cnum] = RealMath_cos(X2C_DIVR((float)cnum*2.0f*3.141592f,
                32768.0f));
   } /* end for */
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      Lib_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='a') abortonsounderr = 1;
         else if (h[1U]=='c') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<1UL || cnum>2UL) Error("maxchannels 1..2", 17ul);
            maxchannels = (unsigned char)(cnum-1UL);
         }
         else if (h[1U]=='D') {
            Lib_NextArg(h1, 1024ul);
            debfd = creat(h1, 420L);
         }
         else if (h[1U]=='d') sondeaprs_dao = 1;
         else if (h[1U]=='e') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
            if (labs(inum)>999L) Error("equalizer -999..999", 20ul);
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
         else if (h[1U]=='F') sondeaprs_nofilter = 1;
         else if (h[1U]=='l') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='o') {
            Lib_NextArg(soundfn, 1024ul);
            if (aprsstr_StrToCard(soundfn, 1024ul, &cnum)) {
               /* listen on UDP instead of soundcard */
               soundfn[0] = 0;
               rxsock = openudp();
               if (rxsock<0L) Error("cannot open rx udp socket", 26ul);
               if (bindudp(rxsock, cnum)<0L) {
                  Error("cannot bind inport", 19ul);
               }
            }
         }
         else if (h[1U]=='T') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            maxalmage = cnum*60UL;
         }
         else if (h[1U]=='R') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            almrequest = cnum*60UL;
         }
         else if (h[1U]=='t') Lib_NextArg(sondeaprs_commentfn, 1025ul);
         else if (h[1U]=='m' || h[1U]=='r') {
            sondeaprs_sendmon = h[1U]!='r';
            Lib_NextArg(h, 1024ul);
            i = 0UL;
            if (sondeaprs_GetIp(h, 1024ul, &i, &sondeaprs_ipnum,
                &sondeaprs_toport)<0L) {
               Error("-m or -r ip:port number", 24ul);
            }
            sondeaprs_udpsock = openudp();
            if (sondeaprs_udpsock<0L) Error("cannot open udp socket", 23ul);
         }
         else if (h[1U]=='w') {
            Lib_NextArg(sondeaprs_via, 100ul);
            if ((unsigned char)sondeaprs_via[0UL]<=' ') {
               Error("-m vias like RELAY,WIDE1-1", 27ul);
            }
         }
         else if (h[1U]=='b') {
            Lib_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_beacontime)) {
               Error("-b <s>", 7ul);
            }
         }
         else if (h[1U]=='B') {
            Lib_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_lowaltbeacontime)) {
               Error("-B <s>", 7ul);
            }
            lowbeacon = 1;
         }
         else if (h[1U]=='A') {
            Lib_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_lowalt)) {
               Error("-A <s>", 7ul);
            }
         }
         else if (h[1U]=='I') {
            Lib_NextArg(sondeaprs_mycall, 100ul);
            if ((unsigned char)sondeaprs_mycall[0UL]<' ') {
               Error("-I <mycall>", 12ul);
            }
         }
         else if (h[1U]=='s') {
            Lib_NextArg(semfile, 1024ul);
            if ((unsigned char)semfile[0U]<' ') Error("-s <filename>", 14ul);
         }
         else if (h[1U]=='x') {
            Lib_NextArg(rinexfile, 1024ul);
            if ((unsigned char)rinexfile[0U]<' ') {
               Error("-x <filename>", 14ul);
            }
         }
         else if (h[1U]=='y') {
            Lib_NextArg(yumafile, 1024ul);
            if ((unsigned char)yumafile[0U]<' ') {
               Error("-y <filename>", 14ul);
            }
         }
         else if (h[1U]=='v') sondeaprs_verb = 1;
         else if (h[1U]=='V') {
            sondeaprs_verb = 1;
            sondeaprs_verb2 = 1;
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("sondemod(c) 0.2", 16ul);
               osi_WrStrLn(" -A <meter>     low altitude (useing -B beacon ti\
me) limit meter -A 1000", 73ul);
               osi_WrStrLn(" -a             abort on sound error", 37ul);
               osi_WrStrLn(" -B <seconds>   low altitude send intervall -B 10\
", 50ul);
               osi_WrStrLn(" -b <seconds>   minimum send intervall -b 20",
                45ul);
               osi_WrStrLn(" -c <num>       maxchannels (1) (1=mono, 2=stereo\
)", 51ul);
               osi_WrStrLn(" -D <filename>  copy raw sound data to file or pi\
pe", 52ul);
               osi_WrStrLn(" -d             dao extension for 20cm APRS resol\
ution instead of 18m", 70ul);
               osi_WrStrLn(" -e <num>       demod equalizer (0) 100=6db/oct h\
ighpass   (-999..999)", 71ul);
               osi_WrStrLn(" -F             trackfilter off", 32ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <mycall>    Sender of Object Callsign -I OE0A\
AA", 52ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or wav/\
raw file", 58ul);
               osi_WrStrLn(" -o <UDPport>   receive demodulated data via UDP",
                 49ul);
               osi_WrStrLn(" -R <minutes>   request new rinex almanach after \
minutes if receiving gps (-T 240)", 83ul);
               osi_WrStrLn(" -r <ip>:<port> send AXUDP  -r 127.0.0.1:4000",
                46ul);
               osi_WrStrLn(" -s <filename>  gps almanach sem format (not exac\
t)", 52ul);
               osi_WrStrLn(" -T <minutes>   stop sending data after almanach \
age (-T 360)", 62ul);
               osi_WrStrLn(" -t <filename>  append comment lines from this fi\
le", 52ul);
               osi_WrStrLn(" -V             more verbous", 29ul);
               osi_WrStrLn(" -v             verbous", 24ul);
               osi_WrStrLn(" -x <filename>  gps almanach rinexnavigation form\
at (prefered)", 63ul);
               osi_WrStrLn(" -y <filename>  gps almanach yuma format (not exa\
ct)", 53ul);
               osi_WrStrLn("example: sondemod -f 16000 -o /dev/dsp -s current\
.txt -I OE0AAA -r 127.0.0.1:4000", 82ul);
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
   if (!lowbeacon) sondeaprs_lowaltbeacontime = sondeaprs_beacontime;
   Config();
   if (soundfn[0U]) OpenSound();
} /* end Parms() */

/*
PROCEDURE WrQuali(q:REAL);
BEGIN
  IF q>0.0 THEN
    q:=100.5-q*200.0;
    IF q<0.0 THEN q:=0.0 END;
    WrStr(" q:"); WrInt(TRUNC(q), 1);
  END;
END WrQuali;
*/

static float Fir(unsigned long in, unsigned long sub, unsigned long step,
                float fir[], unsigned long fir_len, float firtab[],
                unsigned long firtab_len)
{
   float s;
   unsigned long i0;
   s = 0.0f;
   i0 = sub;
   do {
      s = s+fir[in]*firtab[i0];
      ++in;
      if (in>fir_len-1) in = 0UL;
      i0 += step;
   } while (i0<=firtab_len-1);
   return s;
} /* end Fir() */

/*  WrStr(" "); WrHex(n DIV 01000000H MOD 256, 2);
                WrHex(n DIV 010000H MOD 256, 2);
                WrHex(n DIV 0100H MOD 256, 2); WrHex(n MOD 256, 2);  */
#define sondemod_Z 48


static void degtostr(float d, char lat, char form, char s[],
                unsigned long s_len)
{
   unsigned long i0;
   unsigned long n;
   if (s_len-1<11UL) {
      s[0UL] = 0;
      return;
   }
   if (form=='2') i0 = 7UL;
   else if (form=='3') i0 = 8UL;
   else i0 = 9UL;
   if (d<0.0f) {
      d = -d;
      if (lat) s[i0] = 'S';
      else s[i0+1UL] = 'W';
   }
   else if (lat) s[i0] = 'N';
   else s[i0+1UL] = 'E';
   if (form=='2') {
      /* DDMM.MMNDDMM.MME */
      n = (unsigned long)X2C_TRUNCC(d*3.4377467707849E+5f+0.5f,0UL,
                X2C_max_longcard);
      s[0UL] = (char)((n/600000UL)%10UL+48UL);
      i0 = (unsigned long)!lat;
      s[i0] = (char)((n/60000UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)((n/6000UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)((n/1000UL)%6UL+48UL);
      ++i0;
      s[i0] = (char)((n/100UL)%10UL+48UL);
      ++i0;
      s[i0] = '.';
      ++i0;
      s[i0] = (char)((n/10UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)(n%10UL+48UL);
      ++i0;
   }
   else if (form=='3') {
      /* DDMM.MMMNDDMM.MMME */
      n = (unsigned long)X2C_TRUNCC(d*3.4377467707849E+6f+0.5f,0UL,
                X2C_max_longcard);
      s[0UL] = (char)((n/6000000UL)%10UL+48UL);
      i0 = (unsigned long)!lat;
      s[i0] = (char)((n/600000UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)((n/60000UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)((n/10000UL)%6UL+48UL);
      ++i0;
      s[i0] = (char)((n/1000UL)%10UL+48UL);
      ++i0;
      s[i0] = '.';
      ++i0;
      s[i0] = (char)((n/100UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)((n/10UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)(n%10UL+48UL);
      ++i0;
   }
   else {
      /* DDMMSS */
      n = (unsigned long)X2C_TRUNCC(d*2.062648062471E+5f+0.5f,0UL,
                X2C_max_longcard);
      s[0UL] = (char)((n/360000UL)%10UL+48UL);
      i0 = (unsigned long)!lat;
      s[i0] = (char)((n/36000UL)%10UL+48UL);
      ++i0;
      s[i0] = (char)((n/3600UL)%10UL+48UL);
      ++i0;
      s[i0] = 'd';
      ++i0;
      s[i0] = (char)((n/600UL)%6UL+48UL);
      ++i0;
      s[i0] = (char)((n/60UL)%10UL+48UL);
      ++i0;
      s[i0] = '\'';
      ++i0;
      s[i0] = (char)((n/10UL)%6UL+48UL);
      ++i0;
      s[i0] = (char)(n%10UL+48UL);
      ++i0;
      s[i0] = '\"';
      ++i0;
   }
   ++i0;
   s[i0] = 0;
} /* end degtostr() */


static void initcontext(struct CONTEXT * cont)
{
   memset((char *)cont,(char)0,sizeof(struct CONTEXT));
   cont->lastlat = 8.4214719496019E-1;
   cont->laslong = 2.2755602787502E-1;
} /* end initcontext() */


static void dogps(const char sf[], unsigned long sf_len,
                struct CONTEXT * cont, unsigned long * timems,
                unsigned long * gpstime)
{
   unsigned long i0;
   gpspos_SATS sats;
   long res;
   long d1;
   long d;
   char h[100];
   struct CONTEXT * anonym;
   cont->lat = 0.0;
   cont->long0 = 0.0;
   cont->heig = 0.0;
   cont->speed = 0.0;
   cont->dir = 0.0;
   /*WrStrLn("gps:"); */
   /*FOR i:=0 TO 121 DO WrHex(ORD(sf[i]), 3) END; WrStrLn(""); */
   *timems = (unsigned long)(unsigned char)sf[0UL]+(unsigned long)
                (unsigned char)sf[1UL]*256UL+(unsigned long)(unsigned char)
                sf[2UL]*65536UL+(unsigned long)(unsigned char)
                sf[3UL]*16777216UL;
   if (sondeaprs_verb2) {
      aprsstr_TimeToStr(( *timems/1000UL)%86400UL, h, 100ul);
      InOut_WriteString("time ms day: ", 14ul);
      InOut_WriteString(h, 100ul);
      InOut_WriteInt((long)( *timems%1000UL), 4UL);
      InOut_WriteInt((long)( *timems/86400000UL), 2UL);
      osi_WrStrLn("", 1ul);
   }
   /*  WrInt(ORD(sf[4]), 4); WrInt(ORD(sf[5]), 4); WrStrLn(""); */
   /*  FILL(ADR(sats), 0C, SIZE(sats)); */
   for (i0 = 0UL; i0<=3UL; i0++) {
      sats[i0*3UL].prn = (unsigned long)(unsigned char)sf[i0*2UL+6UL]&31UL;
      sats[i0*3UL+1UL].prn = (unsigned long)(unsigned char)
                sf[i0*2UL+6UL]/32UL+(unsigned long)(unsigned char)
                sf[i0*2UL+7UL]*8UL&31UL;
      sats[i0*3UL+2UL].prn = (unsigned long)(unsigned char)
                sf[i0*2UL+7UL]/4UL&31UL;
   } /* end for */
   if (sondeaprs_verb2) {
      InOut_WriteString("prn:", 5ul);
      for (i0 = 0UL; i0<=11UL; i0++) {
         InOut_WriteInt((long)sats[i0].prn, 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      InOut_WriteString("sig: ", 6ul);
      for (i0 = 0UL; i0<=11UL; i0++) {
         osi_WrHex((unsigned long)(unsigned char)sf[i0+14UL], 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStrLn("rang:", 6ul);
   }
   for (i0 = 0UL; i0<=11UL; i0++) {
      if (sats[i0].prn>0UL) {
         sats[i0].rang = (long)((unsigned long)(unsigned char)
                sf[i0*8UL+26UL]+(unsigned long)(unsigned char)
                sf[i0*8UL+27UL]*256UL+(unsigned long)(unsigned char)
                sf[i0*8UL+28UL]*65536UL+(unsigned long)(unsigned char)
                sf[i0*8UL+29UL]*16777216UL);
         sats[i0].rang1 = (long)((unsigned long)(unsigned char)
                sf[i0*8UL+30UL]+(unsigned long)(unsigned char)
                sf[i0*8UL+31UL]*256UL+(unsigned long)(unsigned char)
                sf[i0*8UL+32UL]*65536UL);
         sats[i0].rang1 = sats[i0].rang1&8388607L;
         sats[i0].rang3 = (long)(signed char)(unsigned char)sf[i0*8UL+33UL];
         d = sats[i0].rang-lastsat[i0].rang;
         d1 = sats[i0].rang1-lastsat[i0].rang1;
         if (sondeaprs_verb2) {
            InOut_WriteInt((long)sats[i0].prn, 3UL);
            InOut_WriteInt(sats[i0].rang, 12UL);
            InOut_WriteInt(sats[i0].rang1, 12UL);
            InOut_WriteInt(sats[i0].rang3, 5UL);
            InOut_WriteInt(d, 12UL);
            InOut_WriteInt(d-lastsat[i0].lastd, 12UL);
            osi_WrStrLn("", 1ul);
         }
         sats[i0].lastd = d;
         sats[i0].lastd1 = d1;
      }
   } /* end for */
   memcpy(lastsat,sats,sizeof(gpspos_SATS));
   { /* with */
      struct CONTEXT * anonym = cont;
      systime = TimeConv_time();
      if (almread>systime) almread = 0UL;
      if (almread+60UL>systime) {
         *gpstime = systime;
         res = gpspos_getposit(anonym->timems, gpstime, sats,
                anonym->lastlat, anonym->laslong, anonym->lastalt,
                &anonym->lat, &anonym->long0, &anonym->heig, &anonym->speed,
                &anonym->dir, &anonym->climb, &anonym->hrmsc,
                &anonym->vrmsc);
      }
      else res = -2L;
      if (res>=0L) {
         anonym->lastlat = anonym->lat;
         anonym->laslong = anonym->long0;
         anonym->lastalt = anonym->heig;
         anonym->lastspeed = anonym->speed;
         anonym->lastdir = anonym->dir;
         anonym->lastclb = anonym->climb;
      }
      else *gpstime = 0UL;
   }
   if (sondeaprs_verb && res>=0L) {
      degtostr((float)cont->lat, 1, '3', h, 100ul);
      InOut_WriteString(h, 100ul);
      InOut_WriteString(" ", 2ul);
      degtostr((float)cont->long0, 0, '3', h, 100ul);
      InOut_WriteString(h, 100ul);
      /*    WrStr("pos: "); WrFixed(lat/RAD, 5, 12);
                WrFixed(long/RAD, 5, 12); */
      osi_WrFixed((float)cont->heig, 1L, 10UL);
      osi_WrFixed((float)(cont->speed*3.6), 1L, 6UL);
      InOut_WriteString("km/h", 5ul);
      osi_WrFixed((float)cont->dir, 0L, 5UL);
      InOut_WriteString("deg", 4ul);
      osi_WrFixed((float)cont->climb, 1L, 7UL);
      InOut_WriteString("m/s", 4ul);
      InOut_WriteString(" h/vrms:", 9ul);
      osi_WrFixed(cont->hrmsc, 1L, 0UL);
      InOut_WriteString(" ", 2ul);
      osi_WrFixed(cont->vrmsc, 1L, 0UL);
   }
/*WrStrLn(""); */
} /* end dogps() */


static void decodecalib(const char cd[], unsigned long cd_len)
{
   unsigned long n;
   unsigned long i0;
   unsigned long cr;
   unsigned long tmp;
   memset((char *)coeff,(char)0,sizeof(float [256]));
   /*FOR i:=0 TO HIGH(coeff) DO coeff[i]:=0.0 END; */
   /*WrStrLn("calib:"); */
   i0 = 64UL;
   for (tmp = 88UL;;) {
      n = (unsigned long)(unsigned char)cd[i0];
      cr = (unsigned long)(unsigned char)cd[i0+1UL]+(unsigned long)
                (unsigned char)cd[i0+2UL]*256UL+(unsigned long)
                (unsigned char)cd[i0+3UL]*65536UL+(unsigned long)
                (unsigned char)cd[i0+4UL]*16777216UL;
      coeff[n] = *X2C_CAST(&cr,unsigned long,float,float *);
      if (!tmp) break;
      --tmp;
      i0 += 5UL;
   } /* end for */
/*WrInt(n, 3); WrInt(n DIV 10, 3);  WrInt(n MOD 10, 3);
                WrFixed(coeff[n], 15, 30); WrStrLn("");  */
/*
FOR i:=0 TO 255 DO
--  IF coeff[i]<>0.0 THEN 
  (*  WrInt(i DIV 10, 1);  WrInt(i MOD 10, 2);*)
   WrInt(i, 1); WrStr(" ");
   IF coeff[i]<>0.0 THEN WrFixed(coeff[i], 12, 15) ELSE WrFixed(coeff[i], 0,
                1) END; WrStr(" ");
--  END;
  IF i MOD 10=9 THEN WrStrLn("") END;
END;

WrStrLn("");    
*/
/*
FOR i:=0 TO 511 DO
 WrHex(ORD(cd[i]), 3);
 IF i MOD 16=15 THEN WrStrLn("") END;
END;
WrStrLn("");
*/
} /* end decodecalib() */


static float coef(float ref, float u, float c)
{
   float x;
   float v;
   v = X2C_DIVR(ref,u);
   x = 1.0f-v*(1.0f-c);
   if (x!=0.0f) return X2C_DIVR(v,x);
   return 0.0f;
} /* end coef() */


static float extr(unsigned long hi, unsigned long lo, unsigned long u,
                unsigned long idx)
{
   float f;
   float x;
   float v;
   unsigned long i0;
   unsigned long tmp;
   if (hi<=lo || u<=lo) return 0.0f;
   v = coef((float)(hi-lo), (float)(u-lo), coeff[idx+7UL]);
   x = 0.0f;
   f = 1.0f;
   tmp = idx+5UL;
   i0 = idx;
   if (i0<=tmp) for (;; i0++) {
      /* sum(x^n * k[n] */
      x = x+coeff[i0]*f;
      f = f*v;
      if (i0==tmp) break;
   } /* end for */
   return x;
} /* end extr() */


static void domes(const char md[], unsigned long md_len, double * hp,
                double * hyg, double * temp)
{
   unsigned long i0;
   long m[8];
   float d5;
   float d4;
   float d3;
   float p;
   float hr2;
   float hr1;
   for (i0 = 0UL; i0<=7UL; i0++) {
      m[i0] = (long)((unsigned long)(unsigned char)md[i0*3UL]+(unsigned long)
                (unsigned char)md[i0*3UL+1UL]*256UL+(unsigned long)
                (unsigned char)md[i0*3UL+2UL]*65536UL);
   } /* end for */
   /* hygro 1 */
   hr1 = extr((unsigned long)m[3U], (unsigned long)m[7U],
                (unsigned long)m[1U], 40UL);
   hr2 = extr((unsigned long)m[3U], (unsigned long)m[7U],
                (unsigned long)m[2U], 50UL);
   if (hr2>hr1) hr1 = hr2;
   if (hr1<2.0f) hr1 = 0.0f;
   else if (hr1>100.0f) hr1 = 100.0f;
   *hyg = (double)hr1;
   /* temp */
   *temp = (double)extr((unsigned long)m[3U], (unsigned long)m[7U],
                (unsigned long)m[0U], 30UL);
   /* baro */
   d3 = (float)(m[3U]-m[7U]);
   d4 = (float)(m[4U]-m[7U]);
   d5 = (float)(m[5U]-m[7U]);
   p = extr((unsigned long)m[3U], (unsigned long)m[7U], (unsigned long)m[5U],
                 10UL)+coeff[60U]*extr((unsigned long)m[3U],
                (unsigned long)m[7U], (unsigned long)m[4U],
                20UL)+X2C_DIVR(coeff[61U]*coeff[20U]*d3,
                d5)+X2C_DIVR(coeff[61U]*coeff[21U]*coef(d3, d4,
                coeff[27U])*d3,d5)+X2C_DIVR(coeff[61U]*coeff[22U]*pow0(coef(d3,
                 d4, coeff[27U]), 2UL)*d3,
                d5)+X2C_DIVR(coeff[61U]*coeff[23U]*pow0(coef(d3, d4,
                coeff[27U]), 3UL)*d3,
                d5)+X2C_DIVR(coeff[62U]*coeff[20U]*d3*d3,
                d5*d5)+X2C_DIVR(coeff[62U]*coeff[21U]*coef(d3, d4,
                coeff[27U])*d3*d3,
                d5*d5)+X2C_DIVR(coeff[62U]*coeff[22U]*pow0(coef(d3, d4,
                coeff[27U]), 2UL)*d3*d3,
                d5*d5)+X2C_DIVR(coeff[62U]*coeff[23U]*pow0(coef(d3, d4,
                coeff[27U]), 3UL)*d3*d3,
                d5*d5)+X2C_DIVR(coeff[63U]*coeff[20U]*pow0(d3, 3UL),pow0(d5,
                3UL))+X2C_DIVR(coeff[63U]*coeff[21U]*coef(d3, d4,
                coeff[27U])*pow0(d3, 3UL),pow0(d5,
                3UL))+X2C_DIVR(coeff[63U]*coeff[22U]*pow0(coef(d3, d4,
                coeff[27U]), 2UL)*pow0(d3, 3UL),pow0(d5,
                3UL))+X2C_DIVR(coeff[63U]*coeff[23U]*pow0(coef(d3, d4,
                coeff[27U]), 3UL)*pow0(d3, 3UL),pow0(d5, 3UL));
   /*
          + coeff[70]*pow(coeff[20],2)
          + coeff[70]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)
          + coeff[70]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)
          + coeff[70]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)
   
          + coeff[71]*pow(coeff[20],2)*d3/d5
          + coeff[71]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)*d3/d5
          + coeff[71]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)*d3/d5
          + coeff[71]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)*d3/d5
   
          + coeff[72]*pow(coeff[20],2)*pow(d3,2)/pow(d5,2)
          + coeff[72]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)*pow(d3,
                2)/pow(d5,2)
          + coeff[72]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)*pow(d3,
                2)/pow(d5,2)
          + coeff[72]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)*pow(d3,
                2)/pow(d5,2)
   
          + coeff[73]*pow(coeff[20],2)*pow(d3,3)/pow(d5,3)
          + coeff[73]*pow(coeff[21],2)*pow(coef(d3, d4, coeff[27]),2)*pow(d3,
                3)/pow(d5,3)
          + coeff[73]*pow(coeff[22],2)*pow(coef(d3, d4, coeff[27]),4)*pow(d3,
                3)/pow(d5,3)
          + coeff[73]*pow(coeff[23],2)*pow(coef(d3, d4, coeff[27]),6)*pow(d3,
                3)/pow(d5,3)
   
          + coeff[80]*pow(coeff[20],3)
          + coeff[80]*pow(coeff[21],3)*pow(coef(d3, d4, coeff[27]),3)
          + coeff[80]*pow(coeff[22],3)*pow(coef(d3, d4, coeff[27]),6)
          + coeff[80]*pow(coeff[23],3)*pow(coef(d3, d4, coeff[27]),9)
   
          + coeff[81]*pow(coeff[20],3)*d3/d5
          + coeff[81]*pow(coeff[21],3)*pow(coef(d3, d4, coeff[27]),3)*d3/d5
          + coeff[81]*pow(coeff[22],3)*pow(coef(d3, d4, coeff[27]),6)*d3/d5
          + coeff[81]*pow(coeff[23],3)*pow(coef(d3, d4, coeff[27]),9)*d3/d5
   
          + coeff[82]*pow(coeff[20],3)*pow(d3,2)/pow(d5,2)
          + coeff[82]*pow(coeff[21],3)*pow(coef(d3, d4, coeff[27]),3)*pow(d3,
                2)/pow(d5,2)
          + coeff[82]*pow(coeff[22],3)*pow(coef(d3, d4, coeff[27]),6)*pow(d3,
                2)/pow(d5,2)
          + coeff[82]*pow(coeff[23],3)*pow(coef(d3, d4, coeff[27]),9)*pow(d3,
                2)/pow(d5,2)
   */
   *hp = (double)p;
   /*  
   x10:=c[10] + ....
   x20:=c[20] + c[21]*v(m4) + c[22]*v(m4)^2 ...
   x60:=-c[60]*x20 + 10*c[61]*x20 - 100*c[62]*x20 + 1000*c[63]*x20
   x70:=c[70]*x20^2 - 10*c[71]*x20^2 + 100*c[72]*x20^2 - 1000*c[72]*x20^2
   x80:=-c[80]*x20^3 + 10*c[81]*x20^3 - 100*c[82]*x20^3
   p:=x10 + x20 + x60 + x70 + x80
   */
   if (sondeaprs_verb) {
      InOut_WriteString("mes:", 5ul);
      if (sondeaprs_verb2) {
         for (i0 = 0UL; i0<=7UL; i0++) {
            InOut_WriteInt(m[i0], 7UL);
            InOut_WriteString(" ", 2ul);
         } /* end for */
      }
      osi_WrFixed((float)*temp, 3L, 7UL);
      InOut_WriteString(" ", 2ul);
      osi_WrFixed(hr1, 3L, 7UL);
      /*WrStr(" ");WrFixed(hr2, 3,7); */
      InOut_WriteString(" ", 2ul);
      osi_WrFixed(p, 2L, 8UL);
      /*WrStr(" ");WrFixed(x2, 2,8); */
      osi_WrStrLn("", 1ul);
   }
} /* end domes() */


static void calibfn(char obj[], unsigned long obj_len, char fn[],
                unsigned long fn_len)
{
   X2C_PCOPY((void **)&obj,obj_len);
   aprsstr_Assign(fn, fn_len, obj, obj_len);
   aprsstr_Append(fn, fn_len, ".cal", 5ul);
   X2C_PFREE(obj);
} /* end calibfn() */


static void docalib(const char sf[], unsigned long sf_len, char objname0[],
                unsigned long objname_len, struct CONTEXT * cont,
                float * mhz0, unsigned long * frameno)
/*; VAR sameid:BOOLEAN*/
{
   unsigned long idx;
   unsigned long j;
   unsigned long i0;
   char fn[1024];
   long fd;
   char new0;
   new0 = 0;
   i0 = 0UL;
   for (j = 2UL; j<=11UL; j++) {
      /* object name */
      /*    IF (1 IN cont.calibok) & (sf[j]<>cont.calibdata[j+20])
                THEN cont.calibok:=SET32{} END; */
      if (i0<=objname_len-1 && (unsigned char)sf[j]>' ') {
         if (objname0[i0]!=sf[j]) new0 = 1;
         objname0[i0] = sf[j];
         ++i0;
      }
   } /* end for */
   if (i0<=objname_len-1) objname0[i0] = 0;
   if (new0) {
      /*    sameid:=FALSE; */
      initcontext(cont);
      calibfn(objname0, objname_len, fn, 1024ul);
      fd = osi_OpenRead(fn, 1024ul);
      if (fd>=0L) {
         if (osi_RdBin(fd, (char *)cont, sizeof(struct CONTEXT)/1u,
                sizeof(struct CONTEXT))!=(long)sizeof(struct CONTEXT)) {
            initcontext(cont);
         }
         osi_Close(fd);
      }
   }
   *frameno = (unsigned long)(unsigned char)sf[0UL]+(unsigned long)
                (unsigned char)sf[1UL]*256UL;
   if (sondeaprs_verb) {
      if (new0) InOut_WriteString("new ", 5ul);
      InOut_WriteInt((long)*frameno, 1UL); /* frame no */
      InOut_WriteString(" ", 2ul);
      InOut_WriteString(objname0, objname_len); /*WrStr(" bat:");
                WrHex(ORD(sf[12]), 2);*/
   }
   idx = (unsigned long)(unsigned char)sf[15UL];
   if (idx<32UL) {
      j = idx*16UL;
      for (i0 = 16UL; i0<=31UL; i0++) {
         if (j<=511UL) {
            /*      IF (idx IN cont.calibok) & (cont.calibdata[j]<>sf[i])
                THEN cont.calibok:=SET32{} END; */
            cont->calibdata[j] = sf[i0];
         }
         ++j;
      } /* end for */
      if (!X2C_IN(idx,32,cont->calibok)) {
         /* got more new info */
         cont->calibok |= (1UL<<idx);
         calibfn(objname0, objname_len, fn, 1024ul);
         fd = osi_OpenWrite(fn, 1024ul);
         if (fd>=0L) {
            osi_WrBin(fd, (char *)cont, sizeof(struct CONTEXT)/1u,
                sizeof(struct CONTEXT));
            osi_Close(fd);
         }
      }
      /*    INCL(cont.calibok, idx); */
      if ((0x1UL & cont->calibok)) {
         *mhz0 = (float)(400000UL+((unsigned long)(unsigned char)
                cont->calibdata[2U]+(unsigned long)(unsigned char)
                cont->calibdata[3U]*256UL)*10UL)*0.001f;
         if (sondeaprs_verb) {
            InOut_WriteString(" ", 2ul);
            osi_WrFixed(*mhz0, 2L, 6UL);
            InOut_WriteString("MHz ", 5ul);
         }
      }
      if (sondeaprs_verb) {
         InOut_WriteString(" calibration: ", 15ul);
         for (i0 = 0UL; i0<=31UL; i0++) {
            if (i0==idx) InOut_WriteString("!", 2ul);
            else if (X2C_IN(i0,32,cont->calibok)) {
               InOut_WriteString("+", 2ul);
            }
            else InOut_WriteString("-", 2ul);
         } /* end for */
      }
      if (cont->calibok==0xFFFFFFFFUL) {
         /* calibration ready now */
         decodecalib(cont->calibdata, 512ul);
      }
   }
} /* end docalib() */


static void WrRinexfn(unsigned long t)
{
   char fn[31];
   unsigned long y;
   unsigned long d;
   long f;
   /*DateToStr(t, fn); WrStrLn(fn); */
   d = 25568UL+t/86400UL;
   y = (d*4UL)/1461UL;
   d = 1UL+((d*4UL)%1461UL)/4UL;
   strncpy(fn,"brdc0000.00n",31u);
   fn[4U] = (char)(d/100UL+48UL);
   fn[5U] = (char)((d/10UL)%10UL+48UL);
   fn[6U] = (char)(d%10UL+48UL);
   fn[9U] = (char)((y/10UL)%10UL+48UL);
   fn[10U] = (char)(y%10UL+48UL);
   if (sondeaprs_verb) osi_WrStrLn(fn, 31ul);
   f = osi_OpenWrite("getalmanach", 12ul);
   if (f>=0L) {
      osi_WrBin(f, (char *)fn, 31u/1u, aprsstr_Length(fn, 31ul));
      osi_Close(f);
   }
   else osi_WrStrLn("can not write getalmanach file", 31ul);
} /* end WrRinexfn() */

static unsigned short sondemod_POLYNOM = 0x1021U;


static void decodeframe(unsigned char m)
{
   unsigned long gpstime;
   unsigned long frameno;
   unsigned long len;
   unsigned long ic;
   unsigned long p;
   unsigned long j;
   unsigned long i0;
   unsigned long almanachage;
   unsigned short crc;
   char typ;
   char sf[256];
   char b[256];
   char calok;
   struct CONTEXT * anonym;
   struct CONTEXT * anonym0;
   struct CONTEXT * anonym1;
   unsigned long tmp;
   /*
   FOR try:=0 TO 21*0 DO
     FOR i:=0 TO HIGH(b) DO b[i]:=CHR(0) END;
   IF ODD(try DIV 4) THEN
     FOR i:=6 TO 215 DO b[try DIV 16+(i-0)]:=chan[m].rxbuf[i] END;
                (*209- 230- 21+*)
   ELSE
     FOR i:=6 TO 215 DO b[try DIV 16+(215-i)]:=chan[m].rxbuf[i] END;
                (*209- 230- 21+*)
   END;
   IF ODD(try DIV 2) THEN
     FOR i:=216 TO 239 DO b[231+(i-216)]:=CHR(ORD(chan[m].rxbuf[i])) END;
                (*254- 231+*)
   ELSE
     FOR i:=216 TO 239 DO b[254-(i-216)]:=CHR(ORD(chan[m].rxbuf[i])) END;
                (*254- 231+*)
   END;
   
   IF ODD(try DIV 8) THEN
     FOR i:=0 TO 254 DO b[i]:=CHR(255-ORD(b[i])) END;
   END;
   --  WrStrLn(" ecco: ");
   --  FOR i:=216 TO 239 DO WrHex(ORD(chan[m].rxbuf[i]), 4) END; WrStrLn("");
                 WrStrLn("");
   IF ODD(try) THEN
     FOR i:=0 TO HIGH(b) DO
       j:=0;
       FOR ic:=0 TO 7 DO
         INC(j,j);
         IF ic IN CAST(SET8, b[i]) THEN INC(j) END;
       END;
       b[i]:=CHR(j);
     END;
   END;
   
   
   
   --  encode(b, ecc);
   --WrStrLn(" eccm: ");
   --FOR i:=0 TO 23 DO WrHex(ORD(ecc[i]), 4) END; WrStrLn(""); WrStrLn("");
   
   --initialize_ecc();
   --encode_data(b, 231, ecc);
   --WrStrLn(" eccc: ");
   --FOR i:=231 TO 231+23 DO WrHex(ORD(ecc[i]), 4) END; WrStrLn("");
                WrStrLn("");
   --FOR i:=210 TO 233 DO WrHex(ORD(ecc[i]), 4) END; WrStrLn("");
                WrStrLn("");
   
   
   --FOR i:=216 TO 239 DO b[254+216-i]:=chan[m].rxbuf[i] END;
   --FOR i:=216 TO 239 DO b[231+i-216]:=chan[m].rxbuf[i] END;
   --FOR i:=0 TO 23 DO b[231+i]:=ecc[i] END;
   
   
   --FOR i:=0 TO HIGH(b) DO WrHex(ORD(b[i]), 4) END; WrStrLn(" before"); 
   FOR i:=0 TO 23 DO eraspos[i]:=231+i END;
     bb:=b;
     res:=decode(b, eraspos, 12);
     IF res<>-1 THEN
       WrInt(try, 4); WrInt(res, 4); WrStrLn("=rs!!!!!!!!!!!!!!!!!!!!!!");
   --    FOR i:=0 TO HIGH(b) DO WrHex(ORD(b[i]), 4) END; WrStrLn(" eraspos");
       FOR i:=0 TO 254 DO
         IF b[i]<>bb[i] THEN
           WrInt(i, 4); WrStr(":");WrHex(ORD(bb[i]), 2); WrStr("-");
                WrHex(ORD(b[i]), 2);
         END;
       END;
       WrStrLn(" diffs");
   
     END;
   END;
   --IF res<>-1 THEN
   --  b[32]:=0C;
   --  b[26]:=1C;
   
   --  FOR i:=0 TO 255 DO WrHex(ORD(b[i]), 4) END; WrStrLn("");
   --  WrInt(decode(b, eraspos, 0), 4); WrStrLn("");
   --  FOR i:=0 TO 255 DO WrHex(ORD(b[i]), 4) END; WrStrLn("");
   
   --END;
   */
   /*
   FOR i:=0 TO 239 DO
    WrHex(ORD(chan[m].rxbuf[i]), 3);
    IF i MOD 16=15 THEN WrStrLn("") END;
   END;
   WrStrLn("");
   */
   for (i0 = 0UL; i0<=255UL; i0++) {
      b[i0] = chan[m].rxbuf[i0];
   } /* end for */
   calok = 0;
   p = 6UL;
   for (;;) {
      typ = b[p];
      if (typ=='e') {
         if (sondeaprs_verb) InOut_WriteString("cal  ", 6ul);
      }
      else if (typ=='g') {
         if (sondeaprs_verb) InOut_WriteString("gps  ", 6ul);
      }
      else if (typ=='h') {
      }
      else if (typ=='i') {
         /*IF verb THEN WrStr("unkn ") END;*/
         if (sondeaprs_verb) {
            InOut_WriteString("data ", 6ul);
         }
      }
      else if (typ=='\377') {
         /*IF verb THEN WrStr("end ") END;*/
         break;
      }
      else {
         InOut_WriteString("end  ", 6ul);
         if (sondeaprs_verb) {
            osi_WrHex((unsigned long)(unsigned char)typ, 4UL);
         }
         break;
      }
      ++p;
      len = (unsigned long)(unsigned char)b[p]*2UL+2UL; /* +crc */
      ++p;
      j = 0UL;
      crc = 0xFFFFU;
      while (j<len) {
         sf[j] = b[p];
         if (j+2UL<len) {
            for (ic = 0UL; ic<=7UL; ic++) {
               if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (unsigned char)(unsigned char)b[p])) {
                  crc = X2C_LSH(crc,16,1)^0x1021U;
               }
               else crc = X2C_LSH(crc,16,1);
            } /* end for */
         }
         ++p;
         ++j;
         if (p>240UL) {
            InOut_WriteString("eof", 4ul); /* error */
            goto loop_exit;
         }
      }
      if ((char)crc!=sf[len-2UL] || (char)X2C_LSH(crc,16,-8)!=sf[len-1UL]) {
         if (sondeaprs_verb) osi_WrStrLn("********* crc error", 20ul);
      }
      else {
         if (typ=='e') {
            docalib(sf, 256ul, objname, 9ul, &context, &mhz, &frameno);
            if (frameno>context.framenum) {
               /* new frame number */
               context.mesok = 0;
               context.posok = 0;
               context.framesent = 0;
               calok = 1;
               context.framenum = frameno;
            }
            else if (context.framenum==frameno && !context.framesent) {
               calok = 1;
            }
            else if (frameno<context.framenum && sondeaprs_verb) {
               osi_WrStrLn("", 1ul);
               InOut_WriteString("got out of order frame number ", 31ul);
               InOut_WriteInt((long)frameno, 1UL);
               InOut_WriteString(" expecting ", 12ul);
               InOut_WriteInt((long)context.framenum, 1UL);
            }
         }
         else if (typ=='i') {
            { /* with */
               struct CONTEXT * anonym = &context;
               if (calok && anonym->calibok==0xFFFFFFFFUL) {
                  domes(sf, 256ul, &anonym->hp, &anonym->hyg, &anonym->temp);
                  anonym->mesok = 1;
               }
            }
         }
         else if (typ=='g') {
            if (calok) {
               { /* with */
                  struct CONTEXT * anonym0 = &context;
                  dogps(sf, 256ul, &context, &anonym0->timems, &gpstime);
                  if (almread+30UL<=systime) {
                     if (gpspos_readalmanach(semfile, 1024ul, yumafile,
                1024ul, rinexfile, 1024ul, anonym0->timems/1000UL, &almage,
                sondeaprs_verb)) {
                        if (almread+60UL<=systime) {
                           dogps(sf, 256ul, &context, &anonym0->timems,
                &gpstime);
                        }
                        almread = systime;
                     }
                     else {
                        almread = 0UL;
                        almage = 0UL;
                        osi_WrStrLn("almanach read error", 20ul);
                     }
                     if ((rinexfile[0U] && gpstime>almage)
                && gpstime-almage>almrequest) {
                        WrRinexfn(gpstime); /* request a new almanach */
                     }
                  }
                  /*WrInt(gpstime-systime, 15); WrInt(systime, 15);
                WrStrLn(" almage"); */
                  if (gpstime>0UL && gpstime>=almage) {
                     almanachage = gpstime-almage;
                  }
                  else almanachage = 0UL;
                  if (almage+maxalmage>gpstime) anonym0->posok = 1;
                  else if (almanachage>0UL) {
                     InOut_WriteInt((long)(almanachage/60UL), 10UL);
                     osi_WrStrLn(" Min (almanach too old)", 24ul);
                  }
               }
            }
         }
         else if (typ=='h') {
         }
         else {
            tmp = len-1UL;
            j = 0UL;
            if (j<=tmp) for (;; j++) {
               osi_WrHex((unsigned long)(unsigned char)sf[j], 3UL);
               if (j==tmp) break;
            } /* end for */
         }
         if ((sondeaprs_verb && typ!='h') && typ!='\377') {
            osi_WrStrLn("", 1ul);
         }
      }
   }
   loop_exit:;
   if (((((context.posok && calok) && almread+60UL>systime)
                && (context.mesok && context.calibok==0xFFFFFFFFUL || sondeaprs_nofilter)
                ) && context.lat!=0.0) && context.long0!=0.0) {
      { /* with */
         struct CONTEXT * anonym1 = &context;
         if (anonym1->calibok!=0xFFFFFFFFUL) {
            anonym1->hp = 0.0;
            anonym1->hyg = 0.0;
            anonym1->temp = 0.0;
            mhz = 0.0f;
         }
         sondeaprs_senddata(anonym1->lat, anonym1->long0, anonym1->heig,
                anonym1->speed, anonym1->dir, anonym1->climb, anonym1->hp,
                anonym1->hyg, anonym1->temp, (double)mhz,
                (double)anonym1->hrmsc, (double)anonym1->vrmsc,
                (anonym1->timems/1000UL)%86400UL, frameno, objname, 9ul,
                almanachage);
         anonym1->framesent = 1;
      }
   }
   if (sondeaprs_verb) {
      osi_WrStrLn("", 1ul);
      osi_WrStrLn("---------------", 16ul);
   }
} /* end decodeframe() */


static void stobyte(unsigned char m, char b)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      anonym->rxbuf[anonym->rxp] = b;
      if (anonym->rxp>=5UL || b=='*') ++anonym->rxp;
      else anonym->rxp = 0UL;
      if (anonym->rxp>=240UL) {
         anonym->rxp = 0UL;
         decodeframe(m);
      }
   }
/*      FOR i:=0 TO 239 DO WrHex(ORD(rxbuf[i]), 4) END; WrStrLn("");
                WrInt(maxitest, 10); WrStrLn("");     */
} /* end stobyte() */


static void demodbyte(unsigned char m, char d)
{
   unsigned long maxi;
   unsigned long i0;
   long max0;
   long n;
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      anonym->rxbyte = anonym->rxbyte/2UL+256UL*(unsigned long)d;
      max0 = 0L;
      maxi = 0UL;
      for (i0 = 0UL; i0<=9UL; i0++) {
         n = anonym->asynst[i0]-anonym->asynst[(i0+1UL)%10UL];
         if (labs(n)>labs(max0)) {
            max0 = n;
            maxi = i0;
         }
      } /* end for */
      /*maxitest:=maxi; */
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


static void demodbit(unsigned char m, char d)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->lastmanch==d) {
         anonym->manchestd += X2C_DIV(32767L-anonym->manchestd,16L);
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd>=0L) demodbyte(m, d);
   }
} /* end demodbit() */


static void demod(float u, unsigned char m)
{
   char d;
   struct CHAN * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   d = u>=0.0f;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->cbit) {
         demodbit(m, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
      }
      else {
         /*squelch
               sqmed[d]:=sqmed[d] + (u-sqmed[d])*0.05;
               noise:=noise + (ABS(u-sqmed[d])-noise)*0.05; 
         squelch*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
} /* end demod() */


static void Fsk(unsigned char m)
{
   float ff;
   long lim;
   struct CHAN * anonym;
   /*
     IF debfd>=0 THEN
       lim:=-1;
       WrBin(debfd, lim, 2);
     END;
   */
   { /* with */
      struct CHAN * anonym = &chan[m];
      lim = (long)demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (unsigned long)X2C_DIV(anonym->baudfine&65535L,
                4096L), 16UL, anonym->afir, 512ul, anonym->afirtab, 8192ul);
            demod(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */


static void getadc(void)
{
   short buf[4096];
   long minr;
   long maxr;
   long minl;
   long maxl;
   long sl;
   long i0;
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
   for (c = sondemod_LEFT;; c++) {
      chan[c].adcmax = chan[c].adcmax*15L>>4;
      if (c==sondemod_RIGHT) break;
   } /* end for */
   maxl = -32768L;
   maxr = -32768L;
   minl = 32767L;
   minr = 32767L;
   i0 = 0L;
   while (i0<l) {
      sl = (long)buf[i0];
      chan[sondemod_LEFT].afir[afin] = (float)sl;
      if (sl>maxl) maxl = sl;
      if (sl<minl) minl = sl;
      if (maxl-minl>chan[sondemod_LEFT].adcmax) {
         chan[sondemod_LEFT].adcmax = maxl-minl;
      }
      if (maxchannels>sondemod_LEFT) {
         sl = (long)buf[i0+1L];
         chan[sondemod_RIGHT].afir[afin] = (float)sl;
         if (sl>maxr) maxr = sl;
         if (sl<minr) minr = sl;
         if (maxr-minr>chan[sondemod_RIGHT].adcmax) {
            chan[sondemod_RIGHT].adcmax = maxr-minr;
         }
      }
      afin = afin+1UL&511UL;
      Fsk(sondemod_LEFT);
      if (maxchannels>sondemod_LEFT) Fsk(sondemod_RIGHT);
      i0 += (long)((unsigned long)maxchannels+1UL);
   }
} /* end getadc() */


static void udprx(void)
{
   unsigned long fromport;
   unsigned long ip;
   long len;
   char s[100];
   len = udpreceive(rxsock, chan[sondemod_LEFT].rxbuf, 256L, &fromport, &ip);
   if (len==240L) {
      if (sondeaprs_verb && (ip!=lastip || fromport!=lastport)) {
         aprsstr_ipv4tostr(ip, s, 100ul);
         InOut_WriteString(s, 100ul);
         InOut_WriteString(":", 2ul);
         InOut_WriteInt((long)fromport, 1UL);
         osi_WrStrLn("", 1ul);
         lastip = ip;
         lastport = fromport;
      }
      decodeframe(sondemod_LEFT);
   }
   else Usleep(100000UL);
} /* end udprx() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(OBJNAME)!=9) X2C_ASSERT(0);
   sondeaprs_BEGIN();
   gpspos_BEGIN();
   TimeConv_BEGIN();
   Lib_BEGIN();
   RealMath_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   Parms();
   getst = 0UL;
   afin = 0UL;
   soundbufs = 0UL;
   initcontext(&context);
   objname[0] = 0;
   almread = 0UL;
   almage = 0UL;
   lastip = 0UL;
   lastport = 0UL;
   systime = TimeConv_time();
   for (;;) {
      if (soundfn[0U]) getadc();
      else if (rxsock>=0L) udprx();
      ++clock0;
      if ((clock0&63UL)==0UL) systime = TimeConv_time();
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
