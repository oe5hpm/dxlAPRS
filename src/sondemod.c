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
#define sondemod_C_
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#include <fcntl.h>
#ifndef osi_H_
#include "osic.h"
#endif
#include <math.h>
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef gpspos_H_
#include "gpspos.h"
#endif
#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif

/* decode RS92, RS41, SRS-C34 and DFM06 Radiosonde by OE5DXL */
/*FROM reedsolomon IMPORT decode, encode, CRC; */
/*FROM rs IMPORT initialize_ecc, encode_data; */
/*IMPORT IO; */
#define sondemod_DAYSEC 86400

#define sondemod_GPSTIMECORR 15

#define sondemod_ADCBUFLEN 4096

#define sondemod_BAUDSAMP 65536

#define sondemod_PLLSHIFT 1024

#define sondemod_AFIRLEN 512

#define sondemod_AOVERSAMP 16
/*16*/

#define sondemod_ASYNBITS 10

static char sondemod_CALIBFRAME = 'e';

static char sondemod_GPSFRAME = 'g';

static char sondemod_AUXILLARY = 'h';

static char sondemod_DATAFRAME = 'i';

static char sondemod_EMPTYAUX = '\003';

#define sondemod_PI 3.1415926535898

#define sondemod_RAD 1.7453292519943E-2

#define sondemod_MYLAT 8.4214719496019E-1
/* only for show sat elevations if no pos decode */

#define sondemod_MYLONG 2.2755602787502E-1

#define sondemod_NEWALMAGE 30
/* every s reread almanach */

#define sondemod_FASTALM 4
/* reread almanach if old */

typedef char FILENAME[1024];

typedef char OBJNAME[9];

typedef char CALLSSID[11];

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
   char rxbuf[320];
   AFIRTAB afirtab;
   long asynst[10];
};

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
   double ozontemp;
   double ozon;
   unsigned long goodsats;
   unsigned long timems;
   unsigned long framenum;
};

struct CONTEXTC34;

typedef struct CONTEXTC34 * pCONTEXTC34;


struct CONTEXTC34 {
   pCONTEXTC34 next;
   OBJNAME name;
   double clmb;
   double lat;
   double lon;
   double lat1;
   double lon1;
   double alt;
   double speed;
   double dir;
   double temp;
   double dewp;
   unsigned long lastsent;
   unsigned long gpstime;
   unsigned long tgpstime;
   unsigned long tlat;
   unsigned long tlon;
   unsigned long tlat1;
   unsigned long tlon1;
   unsigned long talt;
   unsigned long tspeed;
   unsigned long tdir;
   unsigned long ttemp;
   unsigned long tdewp;
};

struct CONTEXTDFM6;

typedef struct CONTEXTDFM6 * pCONTEXTDFM6;


struct CONTEXTDFM6 {
   pCONTEXTDFM6 next;
   OBJNAME name;
   double clmb;
   double lat;
   double lon;
   double lat1;
   double lon1;
   double alt;
   double speed;
   double dir;
   unsigned long lastsent;
   unsigned long tlat;
   unsigned long tlon;
   unsigned long tlat1;
   unsigned long tlon1;
   unsigned long talt;
   unsigned long tspeed;
   unsigned long tdir;
   unsigned long actrt;
   char posok;
};

struct CONTEXTR4;

typedef struct CONTEXTR4 * pCONTEXTR4;


struct CONTEXTR4 {
   pCONTEXTR4 next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   unsigned long gpssecond;
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

static unsigned long sendquick;

/* 0 send if full calibrated, 1 with mhz, 2 always */
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

static char mycall[100];

static pCONTEXTC34 pcontextc;

static pCONTEXTDFM6 pcontextdfm6;

static pCONTEXTR4 pcontextr4;


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osic_WrStr(text, text_len);
   osic_WrStrLn(" error abort", 13ul);
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
      f[i0] = (float)((double)f[i0]*(0.54+0.46*cos((double)
                (3.141592f*(X2C_DIVR((float)i0,(float)(1UL+(f_len-1))))))));
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
            t[i0] = (float)((double)t[i0]+(double)e*cos((double)
                (X2C_DIVR(3.141592f*(float)(i0*f),4096.0f))));
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
   soundfd = osic_OpenRW(soundfn, 1024ul);
   if (soundfd>=0L) {
      i0 = samplesize(soundfd, 16UL); /* 8, 16 */
      i0 = channels(soundfd, (unsigned long)maxchannels+1UL); /* 1, 2  */
      i0 = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
      if (i0) {
         osic_WrStr("sound setfragment returns ", 27ul);
         osic_WrUINT32(i0, 1UL);
         osic_WrLn();
      }
      i0 = sampelrate(soundfd, adcrate); /* 8000..48000 */
      s = (long)getsampelrate(soundfd);
      if (s!=(long)adcrate) {
         osic_WrStr("sound device returns ", 22ul);
         osic_WrUINT32(s, 1UL);
         osic_WrStrLn("Hz!", 4ul);
      }
   }
   else if (abortonsounderr) {
      osic_WrStr(soundfn, 1024ul);
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
   mycall[0U] = 0;
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
   sendquick = 1UL;
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
      SIN[cnum] = (float)cos((double)(X2C_DIVR((float)cnum*2.0f*3.141592f,
                32768.0f)));
   } /* end for */
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      osic_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='a') abortonsounderr = 1;
         else if (h[1U]=='c') {
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<1UL || cnum>2UL) Error("maxchannels 1..2", 17ul);
            maxchannels = (unsigned char)(cnum-1UL);
         }
         else if (h[1U]=='D') {
            osic_NextArg(h1, 1024ul);
            debfd = creat(h1, 420L);
         }
         else if (h[1U]=='d') sondeaprs_dao = 1;
         else if (h[1U]=='e') {
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
            if (labs(inum)>999L) Error("equalizer -999..999", 20ul);
            chan[channel].configequalizer = inum;
         }
         else if (h[1U]=='f') {
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<8000UL || cnum>96000UL) {
               Error("sampelrate 8000..96000", 23ul);
            }
            adcrate = cnum;
         }
         else if (h[1U]=='F') sondeaprs_nofilter = 1;
         else if (h[1U]=='l') {
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='o') {
            osic_NextArg(soundfn, 1024ul);
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
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            maxalmage = cnum*60UL;
         }
         else if (h[1U]=='R') {
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            almrequest = cnum*60UL;
         }
         else if (h[1U]=='p') {
            osic_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            sendquick = cnum;
         }
         else if (h[1U]=='t') osic_NextArg(sondeaprs_commentfn, 1025ul);
         else if (h[1U]=='m' || h[1U]=='r') {
            sondeaprs_sendmon = h[1U]!='r';
            osic_NextArg(h, 1024ul);
            i = 0UL;
            if (sondeaprs_GetIp(h, 1024ul, &i, &sondeaprs_ipnum,
                &sondeaprs_toport)<0L) {
               Error("-m or -r ip:port number", 24ul);
            }
            sondeaprs_udpsock = openudp();
            if (sondeaprs_udpsock<0L) Error("cannot open udp socket", 23ul);
         }
         else if (h[1U]=='w') {
            osic_NextArg(sondeaprs_via, 100ul);
            if ((unsigned char)sondeaprs_via[0UL]<=' ') {
               Error("-m vias like RELAY,WIDE1-1", 27ul);
            }
         }
         else if (h[1U]=='b') {
            osic_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_beacontime)) {
               Error("-b <s>", 7ul);
            }
         }
         else if (h[1U]=='B') {
            osic_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_lowaltbeacontime)) {
               Error("-B <s>", 7ul);
            }
            lowbeacon = 1;
         }
         else if (h[1U]=='A') {
            osic_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_lowalt)) {
               Error("-A <s>", 7ul);
            }
         }
         else if (h[1U]=='I') {
            osic_NextArg(mycall, 100ul);
            if ((unsigned char)mycall[0U]<' ') Error("-I <mycall>", 12ul);
         }
         else if (h[1U]=='s') {
            osic_NextArg(semfile, 1024ul);
            if ((unsigned char)semfile[0U]<' ') Error("-s <filename>", 14ul);
         }
         else if (h[1U]=='x') {
            osic_NextArg(rinexfile, 1024ul);
            if ((unsigned char)rinexfile[0U]<' ') {
               Error("-x <filename>", 14ul);
            }
         }
         else if (h[1U]=='y') {
            osic_NextArg(yumafile, 1024ul);
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
               osic_WrStrLn("sondemod(c) 0.5", 16ul);
               osic_WrStrLn(" multichannel decoder RS92 and SRS-C34 Radiosonde\
s", 51ul);
               osic_WrStrLn(" -A <meter>     at lower altitude use -B beacon t\
ime (meter) -A 1000", 69ul);
               osic_WrStrLn(" -a             (internal demod only) abort on so\
und error", 59ul);
               osic_WrStrLn(" -B <seconds>   low altitude send intervall -B 10\
", 50ul);
               osic_WrStrLn(" -b <seconds>   high altitude minimum send interv\
all -b 20", 59ul);
               osic_WrStrLn(" -c <num>       (internal demod only) maxchannels\
 (1) (1=mono, 2=stereo)", 73ul);
               osic_WrStrLn(" -D <filename>  (internal demod only) copy raw so\
und data to file or pipe", 74ul);
               osic_WrStrLn(" -d             dao extension for 20cm APRS resol\
ution instead of 18m", 70ul);
               osic_WrStrLn(" -e <num>       (internal demod only) demod equal\
izer (0) 100=6db/oct highpass)", 80ul);
               osic_WrStrLn(" -F             trackfilter off, DO NOT USE THIS \
SENDING TO THE WORLD!", 71ul);
               osic_WrStrLn(" -h             help", 21ul);
               osic_WrStrLn(" -I <mycall>    Sender of Object Callsign -I OE0A\
AA if not sent by \'sondeudp\'", 78ul);
               osic_WrStrLn(" -o <UDPport>   receive demodulated data via UDP \
port from \'sondeudp -u ...\'", 77ul);
               osic_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or wav/\
raw file, prefer external", 75ul);
               osic_WrStrLn("                demodulator with \'sondeudp\' bec\
ause GPS decode time may cause", 78ul);
               osic_WrStrLn("                bad decoding on sound overruns",
                47ul);
               osic_WrStrLn(" -p <num>       0 send if weather data ready, 1 i\
f MHz known, 2 send immediatly (1)", 84ul);
               osic_WrStrLn(" -R <minutes>   request new rinex almanach after \
minutes if receiving gps (-R 240)", 83ul);
               osic_WrStrLn("                use somewhat like \'getalmd\'-scr\
ipt to download", 63ul);
               osic_WrStrLn(" -r <ip>:<port> send AXUDP -r 127.0.0.1:9001 use \
udpgate4 or aprsmap as receiver", 81ul);
               osic_WrStrLn(" -s <filename>  gps almanach sem format (DO NOT U\
SE, not exact)", 64ul);
               osic_WrStrLn(" -T <minutes>   stop sending data after almanach \
age (-T 360)", 62ul);
               osic_WrStrLn(" -t <filename>  append comment lines from this fi\
le", 52ul);
               osic_WrStrLn(" -V             more verbous", 29ul);
               osic_WrStrLn(" -v             verbous", 24ul);
               osic_WrStrLn(" -x <filename>  gps almanach rinexnavigation form\
at (prefered)", 63ul);
               osic_WrStrLn(" -y <filename>  gps almanach yuma format (DO NOT \
USE, not exact)", 65ul);
               osic_WrStrLn("example: sondemod -o 18000 -x almanach.txt -d -A \
1500 -B 10 -I OE0AAA -r 127.0.0.1:9001", 88ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osic_WrStr(">", 2ul);
      osic_WrStr(h, 1024ul);
      osic_WrStrLn("< use -h", 9ul);
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
      osic_WrStr("time ms day: ", 14ul);
      osic_WrStr(h, 100ul);
      osic_WrUINT32((long)( *timems%1000UL), 4UL);
      osic_WrUINT32((long)( *timems/86400000UL), 2UL);
      osic_WrStrLn("", 1ul);
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
      osic_WrStr("prn:", 5ul);
      for (i0 = 0UL; i0<=11UL; i0++) {
         osic_WrUINT32((long)sats[i0].prn, 3UL);
      } /* end for */
      osic_WrStrLn("", 1ul);
      osic_WrStr("sig: ", 6ul);
      for (i0 = 0UL; i0<=11UL; i0++) {
         osic_WrHex((unsigned long)(unsigned char)sf[i0+14UL], 3UL);
      } /* end for */
      osic_WrStrLn("", 1ul);
      osic_WrStrLn("rang:", 6ul);
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
            osic_WrUINT32((long)sats[i0].prn, 3UL);
            osic_WrUINT32(sats[i0].rang, 12UL);
            osic_WrUINT32(sats[i0].rang1, 12UL);
            osic_WrUINT32(sats[i0].rang3, 5UL);
            osic_WrUINT32(d, 12UL);
            osic_WrUINT32(d-lastsat[i0].lastd, 12UL);
            osic_WrStrLn("", 1ul);
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
                &anonym->dir, &anonym->climb, &anonym->hrmsc, &anonym->vrmsc,
                 &anonym->goodsats);
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
      osic_WrStr(h, 100ul);
      osic_WrStr(" ", 2ul);
      degtostr((float)cont->long0, 0, '3', h, 100ul);
      osic_WrStr(h, 100ul);
      /*    WrStr("pos: "); WrFixed(lat/RAD, 5, 12);
                WrFixed(long/RAD, 5, 12); */
      osic_WrFixed((float)cont->heig, 0L, 10UL);
      osic_WrStr("m", 2ul);
      osic_WrFixed((float)(cont->speed*3.6), 1L, 6UL);
      osic_WrStr("km/h", 5ul);
      osic_WrFixed((float)cont->dir, 0L, 5UL);
      osic_WrStr("deg", 4ul);
      osic_WrFixed((float)cont->climb, 1L, 7UL);
      osic_WrStr("m/s", 4ul);
      osic_WrStr(" h/vrms:", 9ul);
      osic_WrFixed(cont->hrmsc, 1L, 0UL);
      osic_WrStr(" ", 2ul);
      osic_WrFixed(cont->vrmsc, 1L, 0UL);
      osic_WrStrLn("", 1ul);
   }
} /* end dogps() */


static void decodecalib(const char cd[], unsigned long cd_len)
{
   unsigned long n;
   unsigned long i0;
   unsigned long cr;
   unsigned long tmp;
   memset((char *)coeff,(char)0,sizeof(float [256]));
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
      osic_WrStr("mes:", 5ul);
      if (sondeaprs_verb2) {
         for (i0 = 0UL; i0<=7UL; i0++) {
            osic_WrUINT32(m[i0], 7UL);
            osic_WrStr(" ", 2ul);
         } /* end for */
         osic_WrStrLn("", 1ul);
      }
      osic_WrFixed((float)*temp, 3L, 7UL);
      osic_WrStr(" ", 2ul);
      osic_WrFixed(hr1, 3L, 7UL);
      /*WrStr(" ");WrFixed(hr2, 3,7); */
      osic_WrStr(" ", 2ul);
      osic_WrFixed(p, 2L, 8UL);
   }
/*WrStr(" ");WrFixed(x2, 2,8); */
/*WrStrLn(""); */
} /* end domes() */

/*
03 03 00 00 00 00 00 00 00 00 B2 7D  no aux
00 03 21 02 5C 5F 00 00 78 1C D4 C9  open input
00 03 00 00 54 5F 00 00 20 1C D8 1F
ozon zero 544
0.35nA/step  
volt 0.13mV/step
220cm3/min
mPa=0.043085*i*flow*temp(kelvin)
flow=time(s)/100cm3  (27.27)
ground 1..7mPa, stratosphere <25mPa
*/
#define sondemod_T20 25000.0
/* adc 20C */

#define sondemod_TM7 65535.0
/* adc fullrange - temp */

#define sondemod_OZON0 550.0
/* adc zero level ozon */

#define sondemod_OZONADC 0.31
/* nA per step */

#define sondemod_MPAUA 4
/* mPa per uA */


static void doozon(const char s[], unsigned long s_len, double * otemp,
                double * ozon)
{
   *otemp = (double)(float)((unsigned long)(unsigned char)
                s[4UL]+(unsigned long)(unsigned char)s[5UL]*256UL);
   *ozon = (double)(float)((unsigned long)(unsigned char)
                s[2UL]+(unsigned long)(unsigned char)s[3UL]*256UL);
   *otemp = (65535.0-*otemp)*1.3568521031208E-3-35.0;
   *ozon = (*ozon-550.0)*0.00124;
   if (*ozon<=0.0) *ozon = 0.0;
   if (sondeaprs_verb) {
      osic_WrStr("ozon:", 6ul);
      osic_WrFixed((float)*ozon, 1L, 5UL);
      osic_WrStr("mPa temp:", 10ul);
      osic_WrFixed((float)*otemp, 1L, 5UL);
      osic_WrStrLn("C", 2ul);
   }
/*WrStr(" ");WrFixed(FLOAT(ORD(s[8])+ORD(s[9])*256), 0,8); */
} /* end doozon() */


static void calibfn(char obj[], unsigned long obj_len, char fn[],
                unsigned long fn_len)
{
   unsigned long i0;
   X2C_PCOPY((void **)&obj,obj_len);
   aprsstr_Assign(fn, fn_len, obj, obj_len);
   i0 = 0UL;
   while (i0<=fn_len-1 && fn[i0]) {
      if (((unsigned char)fn[i0]<'0' || (unsigned char)fn[i0]>'9')
                && ((unsigned char)fn[i0]<'A' || (unsigned char)fn[i0]>'Z')) {
         fn[0UL] = 0;
         goto label;
      }
      ++i0;
   }
   aprsstr_Append(fn, fn_len, ".cal", 5ul);
   label:;
   X2C_PFREE(obj);
} /* end calibfn() */


static void readcontext(struct CONTEXT * cont, char objname0[],
                unsigned long objname_len)
{
   char fn[1024];
   long fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   initcontext(cont);
   calibfn(objname0, objname_len, fn, 1024ul);
   fd = osic_OpenRead(fn, 1024ul);
   if (fd>=0L) {
      if (osic_RdBin(fd, (char *)cont, sizeof(struct CONTEXT)/1u,
                sizeof(struct CONTEXT))!=(long)sizeof(struct CONTEXT)) {
         initcontext(cont);
      }
      osic_Close(fd);
   }
   X2C_PFREE(objname0);
} /* end readcontext() */


static void wrcontext(struct CONTEXT * cont, char objname0[],
                unsigned long objname_len)
{
   char fn[1024];
   long fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   calibfn(objname0, objname_len, fn, 1024ul);
   if (fn[0U]) {
      fd = osic_OpenWrite(fn, 1024ul);
      if (fd>=0L) {
         osic_WrBin(fd, (char *)cont, sizeof(struct CONTEXT)/1u,
                sizeof(struct CONTEXT));
         osic_Close(fd);
      }
   }
   X2C_PFREE(objname0);
} /* end wrcontext() */


static void docalib(const char sf[], unsigned long sf_len, char objname0[],
                unsigned long objname_len, struct CONTEXT * cont,
                float * mhz0, unsigned long * frameno)
{
   unsigned long idx;
   unsigned long j;
   unsigned long i0;
   char new0;
   *mhz0 = 0.0f;
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
   if (new0) readcontext(cont, objname0, objname_len);
   *frameno = (unsigned long)(unsigned char)sf[0UL]+(unsigned long)
                (unsigned char)sf[1UL]*256UL;
   if (sondeaprs_verb) {
      if (new0) osic_WrStr("new ", 5ul);
      osic_WrUINT32((long)*frameno, 1UL); /* frame no */
      osic_WrStr(" ", 2ul);
      osic_WrStr(objname0, objname_len); /*WrStr(" bat:");
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
         wrcontext(cont, objname0, objname_len);
      }
      /*    INCL(cont.calibok, idx); */
      if ((0x1UL & cont->calibok)) {
         *mhz0 = (float)(400000UL+((unsigned long)(unsigned char)
                cont->calibdata[2U]+(unsigned long)(unsigned char)
                cont->calibdata[3U]*256UL)*10UL)*0.001f;
         if (sondeaprs_verb) {
            osic_WrStr(" ", 2ul);
            osic_WrFixed(*mhz0, 2L, 6UL);
            osic_WrStr("MHz ", 5ul);
         }
      }
      if (sondeaprs_verb) {
         osic_WrStr(" calib: ", 9ul);
         for (i0 = 0UL; i0<=31UL; i0++) {
            if (i0==idx) osic_WrStr("!", 2ul);
            else if (X2C_IN(i0,32,cont->calibok)) {
               osic_WrStr("+", 2ul);
            }
            else osic_WrStr("-", 2ul);
         } /* end for */
      }
      if (cont->calibok==0xFFFFFFFFUL) {
         /* calibration ready now */
         decodecalib(cont->calibdata, 512ul);
      }
   }
} /* end docalib() */


static unsigned long calperc(unsigned long cs)
{
   unsigned long n;
   unsigned long i0;
   n = 0UL;
   for (i0 = 0UL; i0<=31UL; i0++) {
      if (X2C_IN(i0,32,cs)) ++n;
   } /* end for */
   return (n*100UL)/32UL;
} /* end calperc() */


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
   if (sondeaprs_verb) osic_WrStrLn(fn, 31ul);
   f = osic_OpenWrite("getalmanach", 12ul);
   if (f>=0L) {
      osic_WrBin(f, (char *)fn, 31u/1u, aprsstr_Length(fn, 31ul));
      osic_Close(f);
   }
   else osic_WrStrLn("can not write getalmanach file", 31ul);
} /* end WrRinexfn() */


static void getcall(const char b[], unsigned long b_len, char call[],
                unsigned long call_len)
{
   unsigned long c;
   unsigned long n;
   unsigned long i0;
   char tmp;
   call[0UL] = 0;
   n = (unsigned long)(unsigned char)b[0UL]*16777216UL+(unsigned long)
                (unsigned char)b[1UL]*65536UL+(unsigned long)(unsigned char)
                b[2UL]*256UL+(unsigned long)(unsigned char)b[3UL];
   if (n>0UL && (unsigned long)(unsigned char)b[4UL]<=15UL) {
      for (i0 = 5UL;; i0--) {
         c = n%37UL;
         if (c==0UL) call[i0] = 0;
         else if (c<27UL) call[i0] = (char)((c+65UL)-1UL);
         else call[i0] = (char)((c+48UL)-27UL);
         n = n/37UL;
         if (i0==0UL) break;
      } /* end for */
      call[6UL] = 0;
      c = (unsigned long)(unsigned char)b[4UL];
      if (c>0UL) {
         aprsstr_Append(call, call_len, "-", 2ul);
         if (c>=10UL) {
            aprsstr_Append(call, call_len, "1", 2ul);
            c = c%10UL;
         }
         aprsstr_Append(call, call_len, (char *)(tmp = (char)(c+48UL),&tmp),
                1u/1u);
      }
   }
/*WrStr("usercall:");WrStrLn(call); */
} /* end getcall() */

static unsigned short sondemod_POLYNOM = 0x1021U;


static void decodeframe(unsigned char m, unsigned long ip,
                unsigned long fromport)
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
   char bb[256];
   char b[256];
   char crdone;
   char calok;
   CALLSSID usercall;
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
   getcall(b, 256ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osic_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, bb, 256ul);
      osic_WrStr(bb, 256ul);
      osic_WrStr(":", 2ul);
      osic_WrUINT32((long)fromport, 1UL);
      if (usercall[0U]) {
         osic_WrStr(" (", 3ul);
         osic_WrStr(usercall, 11ul);
         osic_WrStr(")", 2ul);
      }
      osic_WrStrLn("", 1ul);
   }
   p = 6UL;
   crdone = 1;
   context.ozontemp = 0.0;
   context.ozon = 0.0;
   mhz = 0.0f;
   for (;;) {
      typ = b[p];
      if (typ=='e') {
         if (sondeaprs_verb) {
            osic_WrStr("cal  ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='g') {
         if (sondeaprs_verb) {
            osic_WrStr("gps  ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='h') {
         if (b[p+2UL]!='\003' && sondeaprs_verb) {
            osic_WrStr("aux ", 5ul);
            crdone = 0;
         }
      }
      else if (typ=='i') {
         if (sondeaprs_verb) {
            osic_WrStr("data ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='\377') {
         /*IF verb THEN WrStr("end ") END;*/
         break;
      }
      else {
         osic_WrStr("end  ", 6ul);
         if (sondeaprs_verb) {
            osic_WrHex((unsigned long)(unsigned char)typ, 4UL);
            crdone = 0;
         }
         break;
      }
      ++p;
      len = (unsigned long)(unsigned char)b[p]*2UL+2UL; /* +crc */
      ++p;
      j = 0UL;
      /*WrInt(len, 3);WrStrLn("=len"); */
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
            osic_WrStr("eof", 4ul); /* error */
            crdone = 0;
            goto loop_exit;
         }
      }
      crdone = 0;
      if ((char)crc!=sf[len-2UL] || (char)X2C_LSH(crc,16,-8)!=sf[len-1UL]) {
         if (sondeaprs_verb) osic_WrStrLn("********* crc error", 20ul);
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
               osic_WrStrLn("", 1ul);
               osic_WrStr("got out of order frame number ", 31ul);
               osic_WrUINT32((long)frameno, 1UL);
               osic_WrStr(" expecting ", 12ul);
               osic_WrUINT32((long)context.framenum, 1UL);
               crdone = 0;
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
                        osic_WrStr("almanach read error", 20ul);
                     }
                     if ((rinexfile[0U] && gpstime>almage)
                && gpstime-almage>almrequest) {
                        WrRinexfn(gpstime); /* request a new almanach */
                     }
                     crdone = 0;
                  }
                  if (gpstime>0UL && gpstime>=almage) {
                     almanachage = gpstime-almage;
                  }
                  else almanachage = 0UL;
                  if (almage+maxalmage>gpstime) anonym0->posok = 1;
                  else if (almanachage>0UL) {
                     osic_WrUINT32((long)(almanachage/60UL), 10UL);
                     osic_WrStrLn(" Min (almanach too old)", 24ul);
                     if (almread+4UL<=systime) {
                        almread = systime-30UL;
                /* look often for new almanach */
                     }
                  }
               }
               crdone = 1;
            }
         }
         else if (typ=='h') {
            if (sf[0U]!='\003') {
               if (sondeaprs_verb2) {
                  tmp = len-1UL;
                  j = 0UL;
                  if (j<=tmp) for (;; j++) {
                     osic_WrHex((unsigned long)(unsigned char)sf[j], 3UL);
                     if (j==tmp) break;
                  } /* end for */
                  osic_WrStrLn("", 1ul);
                  crdone = 1;
               }
               if (sf[0U]==0) {
                  doozon(sf, 256ul, &context.ozontemp, &context.ozon);
                  crdone = 1;
               }
            }
         }
         else {
            tmp = len-1UL;
            j = 0UL;
            if (j<=tmp) for (;; j++) {
               osic_WrHex((unsigned long)(unsigned char)sf[j], 3UL);
               if (j==tmp) break;
            } /* end for */
            crdone = 0;
         }
         if (sondeaprs_verb && !crdone) {
            osic_WrStrLn("", 1ul);
            crdone = 1;
         }
      }
   }
   loop_exit:;
   if (((((context.posok && calok) && almread+60UL>systime)
                && (((sendquick==2UL || sondeaprs_nofilter)
                || context.calibok==0xFFFFFFFFUL) || (context.calibok&0x1UL)
                !=0UL && sendquick==1UL)) && context.lat!=0.0)
                && context.long0!=0.0) {
      { /* with */
         struct CONTEXT * anonym1 = &context;
         if (!context.mesok || anonym1->calibok!=0xFFFFFFFFUL) {
            anonym1->hp = 0.0;
            anonym1->hyg = 0.0;
            anonym1->temp = (double)X2C_max_real;
         }
         sondeaprs_senddata(anonym1->lat, anonym1->long0, anonym1->heig,
                anonym1->speed, anonym1->dir, anonym1->climb, anonym1->hp,
                anonym1->hyg, anonym1->temp, anonym1->ozon,
                anonym1->ozontemp, (double)X2C_max_real, (double)mhz,
                (double)anonym1->hrmsc, (double)anonym1->vrmsc,
                (anonym1->timems/1000UL+86385UL)%86400UL, frameno, objname,
                9ul, almanachage, anonym1->goodsats, usercall, 11ul,
                calperc(context.calibok));
         anonym1->framesent = 1;
      }
      crdone = 1;
   }
   if (sondeaprs_verb) {
      if (!crdone) osic_WrStrLn("", 1ul);
      osic_WrStrLn("------------", 13ul);
   }
} /* end decodeframe() */

/*------------------------------ C34 */

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

#define sondemod_MAXEXTEND 4
/* limit extrapolation range */

#define sondemod_MAXTIMESPAN 10

#define sondemod_MAXRANGE 4.712388E-4
/* max jump in rad */


static double extrapolate(double yold, double y, unsigned long told,
                unsigned long t, unsigned long systime0, char * good)
{
   double dy;
   double k;
   *good = 1;
   if (t>=systime0) return y;
   if (told<t) {
      k = (double)(X2C_DIVR((float)(systime0-told),(float)(t-told)));
      if (k>4.0 || told+10UL<systime0) *good = 0;
      dy = y-yold;
      if (fabs(dy)>4.712388E-4) *good = 0;
      return yold+dy*k;
   }
   *good = 0;
   return y;
} /* end extrapolate() */


static void decodec34(const char rxb[], unsigned long rxb_len,
                unsigned long ip, unsigned long fromport)
{
   OBJNAME nam;
   char cb[10];
   char s[1001];
   CALLSSID usercall;
   unsigned long val;
   unsigned long sum2;
   unsigned long sum1;
   unsigned long j;
   unsigned long i0;
   double exlat;
   double exlon;
   double hr;
   pCONTEXTC34 pc;
   double stemp;
   char latok;
   char lonok;
   char posok;
   struct CONTEXTC34 * anonym;
   if (rxb[0UL]!='S' || rxb[1UL]!='C') return;
   /* no srsc34 frame */
   i0 = 0UL;
   do {
      nam[i0] = rxb[i0];
      ++i0;
   } while (i0<=8UL);
   if (nam[0U]==0) return;
   /* wait for id */
   ++i0;
   j = 0UL;
   do {
      cb[j] = rxb[i0];
      ++i0;
      ++j;
   } while (j<=4UL);
   getcall(cb, 10ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   j = 0UL;
   do {
      cb[j] = rxb[i0];
      ++i0;
      ++j;
   } while (j<=9UL);
   sum1 = 0UL;
   sum2 = 65791UL;
   for (i0 = 0UL; i0<=4UL; i0++) {
      sum1 += (unsigned long)(unsigned char)cb[i0];
      sum2 -= (unsigned long)(unsigned char)cb[i0]*(5UL-i0);
   } /* end for */
   sum1 = sum1&255UL;
   sum2 = sum2&255UL;
   if (sum1!=(unsigned long)(unsigned char)cb[5U] || sum2!=(unsigned long)
                (unsigned char)cb[6U]) return;
   /* chesum error */
   systime = TimeConv_time();
   if (sondeaprs_verb && fromport>0UL) {
      osic_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osic_WrStr(s, 1001ul);
      osic_WrStr(":", 2ul);
      osic_WrUINT32((long)fromport, 1UL);
      if (usercall[0U]) {
         osic_WrStr(" (", 3ul);
         osic_WrStr(usercall, 11ul);
         osic_WrStr(")", 2ul);
      }
      osic_WrStr(" ", 2ul);
   }
   osic_WrStr(nam, 9ul);
   osic_WrStr(" ", 2ul);
   pc = pcontextc;
   while (pc && !aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) pc = pc->next;
   if (pc==0) {
      Storage_ALLOCATE((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTC34));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((X2C_ADDRESS)pc,(char)0,sizeof(struct CONTEXTC34));
      pc->next = pcontextc;
      pcontextc = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osic_WrStrLn("is new ", 8ul);
   }
   val = (unsigned long)(unsigned char)cb[4U]+(unsigned long)(unsigned char)
                cb[3U]*256UL+(unsigned long)(unsigned char)
                cb[2U]*65536UL+(unsigned long)(unsigned char)
                cb[1U]*16777216UL;
   hr = (double)*X2C_CAST(&val,unsigned long,float,float *);
   posok = 0;
   switch ((unsigned)cb[0U]) {
   case '\003':
      /*
          CHR(01H): IF (hr<99.9) & (hr>-99.9)
                THEN       (* something magic with this value *)
                      IF verb THEN WrStr("pres"); (* WrFixed(hr, 2, 0);
                WrStr("hPa");*) END;
                    END;
      */
      if (hr<99.9 && hr>(-99.9)) {
         if (sondeaprs_verb) {
            osic_WrStr("temp", 5ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osic_WrStr("oC", 3ul);
         }
         pc->temp = hr;
         pc->ttemp = systime;
      }
      break;
   case '\007':
      if (hr<99.9 && hr>(-99.9)) {
         if (sondeaprs_verb) {
            osic_WrStr("dewp", 5ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osic_WrStr("oC", 3ul);
         }
         pc->dewp = hr;
         pc->tdewp = systime;
      }
      break;
   case '\024':
      if (sondeaprs_verb) {
         osic_WrStr("date", 5ul);
         aprsstr_IntToStr((long)(val%1000000UL+1000000UL), 1UL, s, 1001ul);
         s[0U] = ' ';
         osic_WrStr(s, 1001ul);
      }
      break;
   case '\025':
      pc->gpstime = (val/10000UL)*3600UL+((val%10000UL)/100UL)
                *60UL+val%100UL;
      pc->tgpstime = systime;
      if (sondeaprs_verb) {
         aprsstr_TimeToStr(pc->gpstime, s, 1001ul);
         osic_WrStr("time ", 6ul);
         osic_WrStr(s, 1001ul);
      }
      break;
   case '\026':
      hr = latlong(val);
      if (hr<89.9 && hr>(-89.9)) {
         if (sondeaprs_verb) {
            osic_WrStr("lati", 5ul);
            osic_WrFixed((float)hr, 5L, 0UL);
         }
         if (pc->tlat!=systime) {
            pc->lat1 = pc->lat;
            pc->tlat1 = pc->tlat;
            pc->lat = hr*1.7453292519943E-2;
            pc->tlat = systime;
            posok = 1;
         }
      }
      break;
   case '\027':
      hr = latlong(val);
      if (hr<180.0 && hr>(-180.0)) {
         if (sondeaprs_verb) {
            osic_WrStr("long", 5ul);
            osic_WrFixed((float)hr, 5L, 0UL);
         }
         if (pc->tlon!=systime) {
            pc->lon1 = pc->lon; /* save 2 values for extrapolating */
            pc->tlon1 = pc->tlon;
            pc->lon = hr*1.7453292519943E-2;
            pc->tlon = systime;
            posok = 1;
         }
      }
      break;
   case '\030':
      hr = (double)((float)val*0.1f);
      if (hr<50000.0) {
         if (sondeaprs_verb) {
            osic_WrStr("alti", 5ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osic_WrStr("m", 2ul);
         }
         if (pc->talt<systime) {
            pc->clmb = pc->clmb+(X2C_DIVL(hr-pc->alt,
                (double)(float)(systime-pc->talt))-pc->clmb)*0.25;
         }
         pc->alt = hr;
         pc->talt = systime;
      }
      break;
   case '\031':
      hr = (double)((float)val*1.851984E-1f);
                /*1.609*/ /*1.852*/ /* guess knots or miles */
      if (hr>=0.0 && hr<1000.0) {
         if (sondeaprs_verb) {
            osic_WrStr("wind", 5ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osic_WrStr("km/h", 5ul);
         }
         pc->speed = hr*2.7777777777778E-1;
         pc->tspeed = systime;
      }
      break;
   case '\032':
      hr = (double)((float)val*0.1f);
      if (hr>=0.0 && hr<=360.0) {
         if (sondeaprs_verb) {
            osic_WrStr("wdir", 5ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osic_WrStr("deg", 4ul);
         }
         pc->dir = hr;
         pc->tdir = systime;
      }
      break;
   default:;
      if (sondeaprs_verb) {
         osic_WrHex((unsigned long)(unsigned char)cb[0U], 0UL);
         osic_WrStr(" ", 2ul);
         osic_WrHex((unsigned long)(unsigned char)cb[1U], 0UL);
         osic_WrHex((unsigned long)(unsigned char)cb[2U], 0UL);
         osic_WrHex((unsigned long)(unsigned char)cb[3U], 0UL);
         osic_WrHex((unsigned long)(unsigned char)cb[4U], 0UL);
      }
      break;
   } /* end switch */
   { /* with */
      struct CONTEXTC34 * anonym = pc;
      if (((((((posok && anonym->lastsent!=systime)
                && anonym->tlon+8UL>systime) && anonym->tlat+8UL>systime)
                && anonym->talt+15UL>systime) && anonym->tspeed+15UL>systime)
                 && anonym->tdir+15UL>systime)
                && anonym->tgpstime+120UL>systime) {
         if (anonym->ttemp+30UL>systime) stemp = anonym->temp;
         else stemp = (double)X2C_max_real;
         exlon = extrapolate(anonym->lon1, anonym->lon, anonym->tlon1,
                anonym->tlon, systime, &lonok);
         exlat = extrapolate(anonym->lat1, anonym->lat, anonym->tlat1,
                anonym->tlat, systime, &latok);
         /*
         IF lonok THEN WrStrLn("--good ") ELSE WrStrLn("--bad  ") END;
         WrInt(systime-tlon1, 10); WrInt(systime-tlon, 10);
         WrFixed(lon1/RAD, 5,0); WrStr(" ");WrFixed(lon/RAD, 5,0);
                WrStr(" ");
         WrFixed(exlon/RAD, 5,0); WrStrLn("t1 t x1 x xext");
         */
         if (lonok && latok) {
            sondeaprs_senddata(exlat, exlon, anonym->alt, anonym->speed,
                anonym->dir, anonym->clmb, 0.0, 0.0, stemp, 0.0, 0.0,
                anonym->dewp, 0.0, 0.0, 0.0,
                ((systime-anonym->tgpstime)+anonym->gpstime)%86400UL, 0UL,
                anonym->name, 9ul, 0UL, 0UL, usercall, 11ul, 0UL);
            anonym->lastsent = systime;
         }
      }
   }
   if (sondeaprs_verb) osic_WrStrLn("", 1ul);
} /* end decodec34() */

/*------------------------------ DFM06 */

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

static unsigned long sondemod_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};


static void decodesub(const char b[], unsigned long b_len, pCONTEXTDFM6 pc,
                unsigned long subnum)
{
   unsigned long u;
   unsigned long v;
   long vi;
   char ok0;
   double vr;
   ok0 = 0;
   switch (bits2val(b, b_len, 48UL, 4UL)) {
   case 2UL:
      vi = (long)bits2val(b, b_len, 0UL, 32UL);
      u = bits2val(b, b_len, 32UL, 16UL);
      vr = (double)vi*1.E-7;
      if (vr<89.9 && vr>(-89.9)) {
         pc->lat1 = pc->lat;
         pc->tlat1 = pc->tlat;
         pc->lat = vr*1.7453292519943E-2;
         pc->tlat = systime;
         pc->posok = 1;
      }
      vr = (double)u*0.01;
      if (vr<999.0) {
         pc->speed = vr;
         pc->tspeed = systime;
      }
      if (sondeaprs_verb) {
         osic_WrStr(" lat: ", 7ul);
         osic_WrFixed((float)(X2C_DIVL(pc->lat,1.7453292519943E-2)), 5L, 0UL);
         osic_WrStr(" ", 2ul);
         osic_WrFixed((float)u*0.036f, 1L, 0UL);
         osic_WrStr("km/h", 5ul);
      }
      ok0 = 1;
      break;
   case 3UL:
      vi = (long)bits2val(b, b_len, 0UL, 32UL);
      u = bits2val(b, b_len, 32UL, 16UL);
      vr = (double)vi*1.E-7;
      if (vr<180.0 && vr>(-180.0)) {
         pc->lon1 = pc->lon; /* save 2 values for extrapolating */
         pc->tlon1 = pc->tlon;
         pc->lon = vr*1.7453292519943E-2;
         pc->tlon = systime;
         pc->posok = 1;
      }
      vr = (double)u*0.01;
      if (vr<=360.0) {
         pc->dir = vr;
         pc->tdir = systime;
      }
      if (sondeaprs_verb) {
         osic_WrStr(" long:", 7ul);
         osic_WrFixed((float)(X2C_DIVL(pc->lon,1.7453292519943E-2)), 5L, 0UL);
         osic_WrStr(" ", 2ul);
         osic_WrFixed((float)u*0.01f, 1L, 0UL);
         osic_WrStr(" deg", 5ul);
      }
      ok0 = 1;
      break;
   case 4UL:
      v = bits2val(b, b_len, 0UL, 32UL);
      vi = (long)bits2val(b, b_len, 32UL, 16UL);
      vr = (double)v*0.01;
      if (vr<50000.0) {
         pc->alt = vr;
         pc->talt = systime;
      }
      if (vi>=32768L) vi -= 65536L;
      vr = (double)vi*0.01;
      if (vr<50.0 && vr>(-500.0)) pc->clmb = vr;
      if (sondeaprs_verb) {
         osic_WrStr(" alti:", 7ul);
         osic_WrFixed((float)pc->alt, 1L, 0UL);
         osic_WrStr("m ", 3ul);
         osic_WrFixed((float)pc->clmb, 1L, 0UL);
         osic_WrStr(" m/s", 5ul);
      }
      ok0 = 1;
      break;
   } /* end switch */
} /* end decodesub() */


static void decodedfm6(const char rxb[], unsigned long rxb_len,
                unsigned long ip, unsigned long fromport)
{
   unsigned long rt;
   char db[56];
   pCONTEXTDFM6 pc;
   OBJNAME nam;
   char cb[10];
   char s[1001];
   CALLSSID usercall;
   unsigned long ib;
   unsigned long j;
   unsigned long i0;
   double exlat;
   double exlon;
   char latok;
   char lonok;
   struct CONTEXTDFM6 * anonym;
   if (rxb[0UL]!='D' || rxb[1UL]!='F') return;
   /* no dfm06 frame */
   i0 = 0UL;
   do {
      nam[i0] = rxb[i0];
      ++i0;
   } while (i0<=8UL);
   if (nam[0U]==0) return;
   /* wait for id */
   ++i0;
   j = 0UL;
   do {
      cb[j] = rxb[i0]; /* call */
      ++i0;
      ++j;
   } while (j<=4UL);
   getcall(cb, 10ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   rt = 0UL;
   for (j = 0UL; j<=3UL; j++) {
      rt = rt*256UL+(unsigned long)(unsigned char)rxb[i0]; /* realtime */
      ++i0;
   } /* end for */
   systime = TimeConv_time();
   if (sondeaprs_verb && fromport>0UL) {
      osic_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osic_WrStr(s, 1001ul);
      osic_WrStr(":", 2ul);
      osic_WrUINT32((long)fromport, 1UL);
      if (usercall[0U]) {
         osic_WrStr(" (", 3ul);
         osic_WrStr(usercall, 11ul);
         osic_WrStr(")", 2ul);
      }
      osic_WrStr(" ", 2ul);
   }
   osic_WrStr(nam, 9ul);
   osic_WrStr(" ", 2ul);
   pc = pcontextdfm6;
   while (pc && !aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) pc = pc->next;
   if (pc==0) {
      Storage_ALLOCATE((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTDFM6));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((X2C_ADDRESS)pc,(char)0,sizeof(struct CONTEXTDFM6));
      pc->next = pcontextdfm6;
      pcontextdfm6 = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osic_WrStrLn("is new ", 8ul);
   }
   if (rt+1UL>=pc->actrt) {
      /* not an older frame */
      pc->actrt = rt;
      pc->posok = 0;
      i0 += 4UL;
      for (j = 0UL; j<=7UL; j++) {
         for (ib = 0UL; ib<=7UL; ib++) {
            db[ib+8UL*j] = X2C_IN(7UL-ib,8,
                (unsigned char)(unsigned char)rxb[i0]);
         } /* end for */
         ++i0;
      } /* end for */
      decodesub(db, 56ul, pc, 0UL);
      for (j = 0UL; j<=7UL; j++) {
         for (ib = 0UL; ib<=7UL; ib++) {
            db[ib+8UL*j] = X2C_IN(7UL-ib,8,
                (unsigned char)(unsigned char)rxb[i0]);
         } /* end for */
         ++i0;
      } /* end for */
      decodesub(db, 56ul, pc, 1UL);
      { /* with */
         struct CONTEXTDFM6 * anonym = pc;
         if ((((((anonym->posok && anonym->lastsent!=systime)
                && anonym->tlon+8UL>systime) && anonym->tlat+8UL>systime)
                && anonym->talt+15UL>systime) && anonym->tspeed+15UL>systime)
                 && anonym->tdir+15UL>systime) {
            exlon = extrapolate(anonym->lon1, anonym->lon, anonym->tlon1,
                anonym->tlon, systime, &lonok);
            exlat = extrapolate(anonym->lat1, anonym->lat, anonym->tlat1,
                anonym->tlat, systime, &latok);
            /*
            IF lonok THEN WrStrLn("--lon good ") ELSE WrStrLn("--lon bad  ")
                END;
            IF latok THEN WrStrLn("--lat good ") ELSE WrStrLn("--lat bad  ")
                END;
            WrInt(systime-tlon1, 10); WrInt(systime-tlon, 10);
            WrFixed(lon1/RAD, 5,0); WrStr(" ");WrFixed(lon/RAD, 5,0);
                WrStr(" ");
            WrFixed(exlon/RAD, 5,0); WrStrLn("t1 t x1 x xext");
            */
            if (lonok && latok) {
               sondeaprs_senddata(exlat, exlon, anonym->alt, anonym->speed,
                anonym->dir, anonym->clmb, 0.0, 0.0, (double)X2C_max_real,
                0.0, 0.0, (double)X2C_max_real, 0.0, 0.0, 0.0,
                anonym->actrt%86400UL, 0UL, anonym->name, 9ul, 0UL, 0UL,
                usercall, 11ul, 0UL);
               anonym->lastsent = systime;
            }
         }
      }
   }
   else if (sondeaprs_verb) osic_WrStr(" got old frame ", 16ul);
   if (sondeaprs_verb) osic_WrStrLn("", 1ul);
} /* end decodedfm6() */

/*------------------------------ RS41 */

static double atan20(double x, double y)
{
   double w;
   if (fabs(x)>fabs(y)) {
      w = atan(X2C_DIVL(y,x));
      if (x<0.0) {
         if (y>0.0) w = 3.141592+w;
         else w = w-3.141592;
      }
   }
   else if (y!=0.0) {
      w = 1.570796-atan(X2C_DIVL(x,y));
      if (y<0.0) w = w-3.141592;
   }
   else w = 0.0;
   return w;
} /* end atan2() */

#define sondemod_EARTHA 6.378137E+6

#define sondemod_EARTHB 6.3567523142452E+6

#define sondemod_E2 6.6943799901413E-3

#define sondemod_EARTHAB 4.2841311513312E+4


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
      rh = sqrt(h);
      xh = x+rh;
      *long0 = atan20(xh, y)*2.0;
      if (*long0>3.141592) *long0 = *long0-6.283184;
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
} /* end wgs84r() */


static long getint32(const char frame[], unsigned long frame_len,
                unsigned long p)
{
   unsigned long n;
   unsigned long i0;
   n = 0UL;
   for (i0 = 3UL;; i0--) {
      n = n*256UL+(unsigned long)(unsigned char)frame[p+i0];
      if (i0==0UL) break;
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


static void posrs41(const char b[], unsigned long b_len, unsigned long p,
                double * lat, double * long0, double * heig, double * speed,
                double * dir, double * clmb)
{
   double vu;
   double ve;
   double vn;
   double vz;
   double vy;
   double vx;
   double z;
   double y;
   double x;
   x = (double)getint32(b, b_len, p)*0.01;
   y = (double)getint32(b, b_len, p+4UL)*0.01;
   z = (double)getint32(b, b_len, p+8UL)*0.01;
   wgs84r(x, y, z, lat, long0, heig);
   if (sondeaprs_verb) {
      osic_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(*lat,1.7453292519943E-2)), 5L, 1UL);
      osic_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(*long0,1.7453292519943E-2)), 5L, 1UL);
      osic_WrStr(" ", 2ul);
      osic_WrFixed((float)*heig, 1L, 1UL);
      osic_WrStr("m ", 3ul);
   }
   if (*heig<(-500.0) || *heig>50000.0) {
      *lat = 0.0;
      *long0 = 0.0;
      *heig = 0.0;
   }
   /*speed */
   vx = (double)getint16(b, b_len, p+12UL)*0.01;
   vy = (double)getint16(b, b_len, p+14UL)*0.01;
   vz = (double)getint16(b, b_len, p+16UL)*0.01;
   vn = (-(vx*sin(*lat)*cos(*long0))-vy*sin(*lat)*sin(*long0))+vz*cos(*lat);
   ve = -(vx*sin(*long0))+vy*cos(*long0);
   vu = vx*cos(*lat)*cos(*long0)+vy*cos(*lat)*sin(*long0)+vz*sin(*lat);
   *dir = X2C_DIVL(atan20(vn, ve),1.7453292519943E-2);
   if (*dir<0.0) *dir = 360.0+*dir;
   *speed = sqrt(vn*vn+ve*ve);
   *clmb = vu;
   if (sondeaprs_verb) {
      osic_WrStr(" ", 2ul);
      osic_WrFixed((float)( *speed*3.6), 2L, 1UL);
      osic_WrStr("km/h ", 6ul);
      osic_WrFixed((float)*dir, 1L, 1UL);
      osic_WrStr("deg ", 5ul);
      osic_WrFixed((float)vu, 1L, 1UL);
      osic_WrStr("m/s", 4ul);
   }
} /* end posrs41() */

static unsigned short sondemod_POLYNOM0 = 0x1021U;


static void decoders41(const char rxb[], unsigned long rxb_len,
                unsigned long ip, unsigned long fromport)
{
   OBJNAME nam;
   char s[1001];
   CALLSSID usercall;
   unsigned long frameno;
   unsigned long len;
   unsigned long p;
   unsigned long ic;
   unsigned long j;
   unsigned long i0;
   char nameok;
   char calok;
   unsigned short crc;
   char typ;
   pCONTEXTR4 pc;
   double climb;
   double dir;
   double speed;
   double heig;
   double long0;
   double lat;
   calok = 0;
   nameok = 0;
   nam[0U] = 0;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   getcall(rxb, rxb_len, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osic_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osic_WrStr(s, 1001ul);
      osic_WrStr(":", 2ul);
      osic_WrUINT32((long)fromport, 1UL);
      if (usercall[0U]) {
         osic_WrStr(" (", 3ul);
         osic_WrStr(usercall, 11ul);
         osic_WrStr(")", 2ul);
      }
      osic_WrStrLn("", 1ul);
   }
   p = 57UL;
   if (sondeaprs_verb) osic_WrStr("R41 ", 5ul);
   for (;;) {
      if (p+4UL>=rxb_len-1) break;
      typ = rxb[p];
      ++p;
      len = (unsigned long)(unsigned char)rxb[p]+2UL;
      ++p;
      if (p+len>=rxb_len-1) break;
      /*
      WrStrLn("");
      FOR i:=0 TO len-1 DO WrHex(ORD(rxb[p+i]),3) ;
                IF i MOD 16=15 THEN WrStrLn(""); END; END;
      WrStrLn("");
      */
      j = 0UL;
      crc = 0xFFFFU;
      while (j<len && p+j<rxb_len-1) {
         if (j+2UL<len) {
            for (ic = 0UL; ic<=7UL; ic++) {
               if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (unsigned char)(unsigned char)rxb[p+j])) {
                  crc = X2C_LSH(crc,16,1)^0x1021U;
               }
               else crc = X2C_LSH(crc,16,1);
            } /* end for */
         }
         ++j;
      }
      if ((char)crc!=rxb[(p+len)-2UL] || (char)X2C_LSH(crc,16,
                -8)!=rxb[(p+len)-1UL]) {
         if (sondeaprs_verb) osic_WrStr(" ----  crc err ", 16ul);
         break;
      }
      if (typ=='y') {
         nameok = 1;
         for (i0 = 0UL; i0<=7UL; i0++) {
            nam[i0] = rxb[p+2UL+i0];
            if ((unsigned char)nam[i0]<=' ' || (unsigned char)nam[i0]>'Z') {
               nameok = 0;
            }
         } /* end for */
         nam[8U] = 0;
         pc = pcontextr4;
         while (pc && !aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) {
            pc = pc->next;
         }
         if (pc==0) {
            Storage_ALLOCATE((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTR4));
            if (pc==0) Error("allocate context out im memory", 31ul);
            memset((X2C_ADDRESS)pc,(char)0,sizeof(struct CONTEXTR4));
            pc->next = pcontextr4;
            pcontextr4 = pc;
            aprsstr_Assign(pc->name, 9ul, nam, 9ul);
            if (sondeaprs_verb) osic_WrStrLn("is new ", 8ul);
         }
         frameno = (unsigned long)getint16(rxb, rxb_len, p);
         if (frameno>pc->framenum) {
            /* new frame number */
            pc->framesent = 0;
            calok = 1;
            pc->framenum = frameno;
         }
         else if (pc->framenum==frameno && !pc->framesent) calok = 1;
         else if (frameno<pc->framenum && sondeaprs_verb) {
            osic_WrStrLn("", 1ul);
            osic_WrStr("got out of order frame number ", 31ul);
            osic_WrUINT32((long)frameno, 1UL);
            osic_WrStr(" expecting ", 12ul);
            osic_WrUINT32((long)pc->framenum, 1UL);
         }
         if (rxb[p+23UL]==0) {
            pc->mhz0 = (float)(X2C_DIV(getint16(rxb, rxb_len, p+26UL),
                64L)+40000L)*0.01f+0.0005f;
         }
         if (sondeaprs_verb) {
            osic_WrStr(objname, 9ul);
            osic_WrUINT32((long)context.framenum, 0UL);
         }
      }
      else if (typ=='z') {
      }
      else if (typ=='|') {
         /*             WrStrLn("7A frame"); */
         /*             WrStrLn("7C frame"); */
         if (pc) {
            pc->gpssecond = (unsigned long)X2C_MOD(X2C_DIV(getint32(rxb,
                rxb_len, p+2UL),1000L)+86385L,86400L); /* gps TOW */
         }
      }
      else if (typ=='}') {
      }
      else if (typ=='{') {
         /*             WrStrLn("7D frame"); */
         /*             WrStrLn("7B frame"); */
         if (pc) {
            posrs41(rxb, rxb_len, p, &lat, &long0, &heig, &speed, &dir,
                &climb);
         }
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
      if (typ=='v') break;
      p += len;
   }
   if (sondeaprs_verb) osic_WrStrLn("", 1ul);
   if ((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) {
      sondeaprs_senddata(lat, long0, heig, speed, dir, climb, 0.0, 0.0,
                (double)X2C_max_real, 0.0, 0.0, (double)X2C_max_real,
                (double)pc->mhz0, 0.0, 0.0, pc->gpssecond, frameno, pc->name,
                 9ul, 0UL, 0UL, usercall, 11ul, 0UL);
      pc->framesent = 1;
   }
/*  IF verb THEN WrStrLn("") END;   */
} /* end decoders41() */

/*---------------------- local demodulator  */

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
         decodeframe(m, 0UL, 0UL);
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
   l = osic_RdBin(soundfd, (char *)buf, 8192u/1u, adcbuflen*adcbytes);
   if (l<0L) {
      if (abortonsounderr) Error("Sounddevice Failure", 20ul);
      else {
         osic_Close(soundfd);
         Usleep(100000UL);
         OpenSound();
         return;
      }
   }
   if (debfd>=0L) osic_WrBin(debfd, (char *)buf, 8192u/1u, (unsigned long)l);
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
   len = udpreceive(rxsock, chan[sondemod_LEFT].rxbuf, 320L, &fromport, &ip);
   if (len==240L) {
      /*
          IF verb & ((ip<>lastip) OR (fromport<>lastport)) THEN
            ipv4tostr(ip, s);
            WrStr(s); WrStr(":"); WrInt(fromport, 1); WrStrLn("");
            lastip:=ip;
            lastport:=fromport; 
          END; 
      */
      decodeframe(sondemod_LEFT, ip, fromport);
   }
   else if (len==22L) {
      decodec34(chan[sondemod_LEFT].rxbuf, 320ul, ip, fromport);
   }
   else if (len==37L) {
      decodedfm6(chan[sondemod_LEFT].rxbuf, 320ul, ip, fromport);
   }
   else if (len==320L) {
      decoders41(chan[sondemod_LEFT].rxbuf, 320ul, ip, fromport);
   }
   else Usleep(50000UL);
} /* end udprx() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(OBJNAME)!=9) X2C_ASSERT(0);
   if (sizeof(CALLSSID)!=11) X2C_ASSERT(0);
   sondeaprs_BEGIN();
   gpspos_BEGIN();
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   Parms();
   getst = 0UL;
   afin = 0UL;
   soundbufs = 0UL;
   initcontext(&context);
   pcontextc = 0;
   pcontextdfm6 = 0;
   pcontextr4 = 0;
   objname[0] = 0;
   almread = 0UL;
   almage = 0UL;
   lastip = 0UL;
   lastport = 0UL;
   systime = TimeConv_time();
   /*testalm; */
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
