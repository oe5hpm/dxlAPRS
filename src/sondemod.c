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
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef gpspos_H_
#include "gpspos.h"
#endif
#ifndef RS41DATA_H_
#include "rs41data.h"
#endif

#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif

/* decode RS92, RS41, SRS-C34 and DFM06 Radiosonde by OE5DXL */
/*FROM rsc IMPORT initrsc, decodersc; */
#define sondemod_CONTEXTLIFE 3600
/* seconds till forget context after last heared */

#define sondemod_DAYSEC 86400

#define sondemod_GPSTIMECORR 18

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


#define sondemod_MIN_BURST_HEIGHT 20000
#define sondemod_AFTER_BURST_OFF_TIMER 30600
//37800(10.5h)-30600(8.5h)

typedef char FILENAME[1024];


typedef char CALLSSID[11];

enum CHANNELS {sondemod_LEFT, sondemod_RIGHT};


struct CHAN;


struct CHAN {
   unsigned long rxbyte;
   unsigned long rxbitc;
   unsigned long rxp;
   char rxbuf[520];
};

struct CONTEXTR9;


struct CONTEXTR9 {
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
   unsigned long tused;
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
   unsigned long tused;
   char posok;
   unsigned long poserr; /* count down after position jump */
};

/*SQ7BR moved to own header file*/

/*
       ozon_id_ser           : ARRAY[0..8] OF CHAR;
       ozon_id_diag          : INT16;
       ozon_id_version       : REAL;  
*/

static FILENAME semfile;

static FILENAME yumafile;

static FILENAME rinexfile;

static char sondeaprs_verbHex;

static FILENAME sondeaprs_verbHexDir ;

static unsigned long sendquick; /* 0 send if full calibrated, 1 with mhz, 2 always */

static unsigned long almread;

/* time last almanach read */
static unsigned long almrequest;

/* seconds rinex age to request new */
static unsigned long almage;

static unsigned long systime;

static FILENAME soundfn;

static struct CHAN chan[2];

static gpspos_SATS lastsat;

static float coeff[256];

static float mhz;

static OBJNAME objname;

static long rxsock;

static unsigned long maxalmage;

static unsigned long lastip;

static unsigned long lastport;

static char mycall[100];

static struct CONTEXTR9 contextr9;

static pCONTEXTC34 pcontextc;

static pCONTEXTDFM6 pcontextdfm6;

static pCONTEXTR41 pcontextr4;

static long testFHandle;


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


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


static void Parms(void)
{
   char err;
   char lowbeacon;
   FILENAME h;
   unsigned long cnum;
   unsigned long i;
   mycall[0U] = 0;
   semfile[0] = 0;
   yumafile[0] = 0;
   rinexfile[0] = 0;
   err = 0;
   rxsock = -1L;
   sondeaprs_dao = 0;
   lowbeacon = 0;
   maxalmage = 21600UL;
   almrequest = 14400UL;
   sondeaprs_verb = 0;
   sondeaprs_verb2 = 0;
   /*SQ7BR*/
   sondeaprs_verbHex = 0;

   sendquick = 1UL;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='d') sondeaprs_dao = 1;
         else if (h[1U]=='F') sondeaprs_nofilter = 1;
         else if (h[1U]=='o') {
            osi_NextArg(soundfn, 1024ul);
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
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            maxalmage = cnum*60UL;
         }
         else if (h[1U]=='R') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            almrequest = cnum*60UL;
         }
         else if (h[1U]=='p') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            sendquick = cnum;
         }
         else if (h[1U]=='t') osi_NextArg(sondeaprs_commentfn, 1025ul);
         else if (h[1U]=='m' || h[1U]=='r') {
            sondeaprs_sendmon = h[1U]!='r';
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (sondeaprs_GetIp(h, 1024ul, &i, &sondeaprs_ipnum,
                &sondeaprs_toport)<0L) {
               Error("-m or -r ip:port number", 24ul);
            }
            sondeaprs_udpsock = openudp();
            if (sondeaprs_udpsock<0L) Error("cannot open udp socket", 23ul);
         }
         else if (h[1U]=='w') {
            osi_NextArg(sondeaprs_via, 100ul);
            if ((unsigned char)sondeaprs_via[0UL]<=' ') {
               Error("-m vias like RELAY,WIDE1-1", 27ul);
            }
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_beacontime)) {
               Error("-b <s>", 7ul);
            }
         }
         else if (h[1U]=='B') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_lowaltbeacontime)) {
               Error("-B <s>", 7ul);
            }
            lowbeacon = 1;
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_lowalt)) {
               Error("-A <s>", 7ul);
            }
         }
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 100ul);
            if ((unsigned char)mycall[0U]<' ') {
               Error("-I <mycall>", 12ul);
            }
         }
         else if (h[1U]=='s') {
            osi_NextArg(semfile, 1024ul);
            if ((unsigned char)semfile[0U]<' ') Error("-s <filename>", 14ul);
         }
         else if (h[1U]=='x') {
            osi_NextArg(rinexfile, 1024ul);
            if ((unsigned char)rinexfile[0U]<' ') {
               Error("-x <filename>", 14ul);
            }
         }
         else if (h[1U]=='y') {
            osi_NextArg(yumafile, 1024ul);
            if ((unsigned char)yumafile[0U]<' ') {
               Error("-y <filename>", 14ul);
            }
         }
         else if (h[1U]=='v') sondeaprs_verb = 1;
         else if (h[1U]=='V') {
            sondeaprs_verb = 1;
            sondeaprs_verb2 = 1;
         }
         /*SQ7BR*/
         else if (h[1U]=='q') {
             osi_NextArg(sondeaprs_verbHexDir, 1024ul);
             if ((unsigned char)sondeaprs_verbHexDir[0U]<' ') {
                Error("-q <hexDirName>", 14ul);
             }
        	 sondeaprs_verbHex =1;
         }

         else {
            if (h[1U]=='h') {
               osi_WrStr("sondemod(c) 0.8", 16ul);
               osi_WrStrLn(" multichannel decoder RS92, RS41, SRS-C34 Radioso\
ndes", 54ul);
               osi_WrStrLn(" -A <meter>     at lower altitude use -B beacon t\
ime (meter) -A 1000", 69ul);
               osi_WrStrLn(" -B <seconds>   low altitude send intervall -B 10\
", 50ul);
               osi_WrStrLn(" -b <seconds>   high altitude minimum send interv\
all -b 20", 59ul);
               osi_WrStrLn(" -d             dao extension for 20cm APRS resol\
ution instead of 18m", 70ul);
               osi_WrStrLn(" -F             trackfilter off, DO NOT USE THIS \
SENDING TO THE WORLD!", 71ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <mycall>    Sender of Object Callsign -I OE0A\
AA if not sent by \'sondeudp\'", 78ul);
               osi_WrStrLn(" -o <UDPport>   receive demodulated data via UDP \
port from \'sondeudp -u ...\'", 77ul);
               osi_WrStrLn(" -p <num>       0 send if weather data ready, 1 i\
f MHz known, 2 send immediatly (1)", 84ul);
               osi_WrStrLn(" -R <minutes>   request new rinex almanach after \
minutes if receiving gps (-R 240)", 83ul);
               osi_WrStrLn("                use somewhat like \'getalmd\'-scr\
ipt to download", 63ul);
               osi_WrStrLn(" -r <ip>:<port> send AXUDP -r 127.0.0.1:9001 use \
udpgate4 or aprsmap as receiver", 81ul);
               osi_WrStrLn(" -s <filename>  gps almanach sem format (DO NOT U\
SE, not exact)", 64ul);
               osi_WrStrLn(" -T <minutes>   stop sending data after almanach \
age (-T 360)", 62ul);
               osi_WrStrLn(" -t <filename>  append comment lines from this fi\
le", 52ul);
               osi_WrStrLn(" -V             more verbous", 29ul);
               osi_WrStrLn(" -v             verbous", 24ul);
               osi_WrStrLn(" -x <filename>  gps almanach rinexnavigation form\
at (prefered)", 63ul);
               osi_WrStrLn(" -y <filename>  gps almanach yuma format (DO NOT \
USE, not exact)", 65ul);
               osi_WrStrLn("example: sondemod -o 18000 -x almanach.txt -d -A \
1500 -B 10 -I OE0AAA -r 127.0.0.1:9001", 88ul);

               /*SQ7BR*/
               osi_WrStrLn(" -q <hexDirName>   dump hex strings to file in dirna\
me", 52ul);

               X2C_ABORT();
            }
            err = 1;
         }
      }
      else {
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
   if (!lowbeacon) sondeaprs_lowaltbeacontime = sondeaprs_beacontime;
} /* end Parms() */

/*  WrStr(" "); WrHex(n DIV 01000000H MOD 256, 2);
                WrHex(n DIV 010000H MOD 256, 2);
                WrHex(n DIV 0100H MOD 256, 2); WrHex(n MOD 256, 2);  */
#define sondemod_Z 48


static void degtostr(float d, char lat, char form, char s[],
                unsigned long s_len)
{
   unsigned long i;
   unsigned long n;
   if (s_len-1<11UL) {
      s[0UL] = 0;
      return;
   }
   if (form=='2') i = 7UL;
   else if (form=='3') i = 8UL;
   else i = 9UL;
   if (d<0.0f) {
      d = -d;
      if (lat) s[i] = 'S';
      else s[i+1UL] = 'W';
   }
   else if (lat) s[i] = 'N';
   else s[i+1UL] = 'E';
   if (form=='2') {
      /* DDMM.MMNDDMM.MME */
      n = osi_realcard(d*3.4377467707849E+5f+0.5f);
      s[0UL] = (char)((n/600000UL)%10UL+48UL);
      i = (unsigned long)!lat;
      s[i] = (char)((n/60000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/6000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/1000UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/100UL)%10UL+48UL);
      ++i;
      s[i] = '.';
      ++i;
      s[i] = (char)((n/10UL)%10UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
   }
   else if (form=='3') {
      /* DDMM.MMMNDDMM.MMME */
      n = osi_realcard(d*3.4377467707849E+6f+0.5f);
      s[0UL] = (char)((n/6000000UL)%10UL+48UL);
      i = (unsigned long)!lat;
      s[i] = (char)((n/600000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/60000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/10000UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/1000UL)%10UL+48UL);
      ++i;
      s[i] = '.';
      ++i;
      s[i] = (char)((n/100UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/10UL)%10UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
   }
   else {
      /* DDMMSS */
      n = osi_realcard(d*2.062648062471E+5f+0.5f);
      s[0UL] = (char)((n/360000UL)%10UL+48UL);
      i = (unsigned long)!lat;
      s[i] = (char)((n/36000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/3600UL)%10UL+48UL);
      ++i;
      s[i] = 'd';
      ++i;
      s[i] = (char)((n/600UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/60UL)%10UL+48UL);
      ++i;
      s[i] = '\'';
      ++i;
      s[i] = (char)((n/10UL)%6UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
      s[i] = '\"';
      ++i;
   }
   ++i;
   s[i] = 0;
} /* end degtostr() */


static void initcontext(struct CONTEXTR9 * cont)
{
   memset((char *)cont,(char)0,sizeof(struct CONTEXTR9));
   cont->lastlat = 8.4214719496019E-1;
   cont->laslong = 2.2755602787502E-1;
} /* end initcontext() */


static void dogps(const char sf[], unsigned long sf_len,
                struct CONTEXTR9 * cont, unsigned long * timems,
                unsigned long * gpstime)
{
   unsigned long i;
   gpspos_SATS sats;
   long res;
   long d1;
   long d;
   char h[100];
   struct CONTEXTR9 * anonym;
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
      osi_WrStr("time ms day: ", 14ul);
      osi_WrStr(h, 100ul);
      osic_WrINT32( *timems%1000UL, 4UL);
      osic_WrINT32( *timems/86400000UL, 2UL);
      osi_WrStrLn("", 1ul);
   }
   /*  WrInt(ORD(sf[4]), 4); WrInt(ORD(sf[5]), 4); WrStrLn(""); */
   /*  FILL(ADR(sats), 0C, SIZE(sats)); */
   for (i = 0UL; i<=3UL; i++) {
      sats[i*3UL].prn = (unsigned long)(unsigned char)sf[i*2UL+6UL]&31UL;
      sats[i*3UL+1UL].prn = (unsigned long)(unsigned char)
                sf[i*2UL+6UL]/32UL+(unsigned long)(unsigned char)
                sf[i*2UL+7UL]*8UL&31UL;
      sats[i*3UL+2UL].prn = (unsigned long)(unsigned char)
                sf[i*2UL+7UL]/4UL&31UL;
   } /* end for */
   if (sondeaprs_verb2) {
      osi_WrStr("prn:", 5ul);
      for (i = 0UL; i<=11UL; i++) {
         osic_WrINT32(sats[i].prn, 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStr("sig: ", 6ul);
      for (i = 0UL; i<=11UL; i++) {
         osi_WrHex((unsigned long)(unsigned char)sf[i+14UL], 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStrLn("rang:", 6ul);
   }
   for (i = 0UL; i<=11UL; i++) {
      if (sats[i].prn>0UL) {
         sats[i].rang = (long)((unsigned long)(unsigned char)
                sf[i*8UL+26UL]+(unsigned long)(unsigned char)
                sf[i*8UL+27UL]*256UL+(unsigned long)(unsigned char)
                sf[i*8UL+28UL]*65536UL+(unsigned long)(unsigned char)
                sf[i*8UL+29UL]*16777216UL);
         sats[i].rang1 = (long)((unsigned long)(unsigned char)
                sf[i*8UL+30UL]+(unsigned long)(unsigned char)
                sf[i*8UL+31UL]*256UL+(unsigned long)(unsigned char)
                sf[i*8UL+32UL]*65536UL);
         sats[i].rang1 = sats[i].rang1&8388607L;
         sats[i].rang3 = (long)(signed char)(unsigned char)sf[i*8UL+33UL];
         d = sats[i].rang-lastsat[i].rang;
         d1 = sats[i].rang1-lastsat[i].rang1;
         if (sondeaprs_verb2) {
            osic_WrINT32(sats[i].prn, 3UL);
            osic_WrINT32((unsigned long)sats[i].rang, 12UL);
            osic_WrINT32((unsigned long)sats[i].rang1, 12UL);
            osic_WrINT32((unsigned long)sats[i].rang3, 5UL);
            osic_WrINT32((unsigned long)d, 12UL);
            osic_WrINT32((unsigned long)(d-lastsat[i].lastd), 12UL);
            osi_WrStrLn("", 1ul);
         }
         sats[i].lastd = d;
         sats[i].lastd1 = d1;
      }
   } /* end for */
   memcpy(lastsat,sats,sizeof(gpspos_SATS));
   { /* with */
      struct CONTEXTR9 * anonym = cont;
      systime = osic_time();
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
      osi_WrStr(h, 100ul);
      osi_WrStr(" ", 2ul);
      degtostr((float)cont->long0, 0, '3', h, 100ul);
      osi_WrStr(h, 100ul);
      /*    WrStr("pos: "); WrFixed(lat/RAD, 5, 12);
                WrFixed(long/RAD, 5, 12); */
      osic_WrFixed((float)cont->heig, 0L, 10UL);
      osi_WrStr("m ", 3ul);
      osic_WrFixed((float)(cont->speed*3.6), 1L, 6UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrFixed((float)cont->dir, 0L, 5UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)cont->climb, 1L, 7UL);
      osi_WrStr("m/s", 4ul);
      osi_WrStr(" h/vrms:", 9ul);
      osic_WrFixed(cont->hrmsc, 1L, 0UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(cont->vrmsc, 1L, 0UL);
      osi_WrStrLn("", 1ul);
   }
} /* end dogps() */


static void decodecalib(const char cd[], unsigned long cd_len)
{
   unsigned long n;
   unsigned long i;
   unsigned long cr;
   unsigned long tmp;
   memset((char *)coeff,(char)0,sizeof(float [256]));
   i = 64UL;
   for (tmp = 88UL;;) {
      n = (unsigned long)(unsigned char)cd[i];
      cr = (unsigned long)(unsigned char)cd[i+1UL]+(unsigned long)
                (unsigned char)cd[i+2UL]*256UL+(unsigned long)(unsigned char)
                cd[i+3UL]*65536UL+(unsigned long)(unsigned char)
                cd[i+4UL]*16777216UL;
      coeff[n] = *X2C_CAST(&cr,unsigned long,float,float *);
      if (!tmp) break;
      --tmp;
      i += 5UL;
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
   unsigned long i;
   unsigned long tmp;
   if (hi<=lo || u<=lo) return 0.0f;
   v = coef((float)(hi-lo), (float)(u-lo), coeff[idx+7UL]);
   x = 0.0f;
   f = 1.0f;
   tmp = idx+5UL;
   i = idx;
   if (i<=tmp) for (;; i++) {
      /* sum(x^n * k[n] */
      x = x+coeff[i]*f;
      f = f*v;
      if (i==tmp) break;
   } /* end for */
   return x;
} /* end extr() */


static void domes(const char md[], unsigned long md_len, double * hp,
                double * hyg, double * temp)
{
   unsigned long i;
   long m[8];
   float d5;
   float d4;
   float d3;
   float p;
   float hr2;
   float hr1;
   for (i = 0UL; i<=7UL; i++) {
      m[i] = (long)((unsigned long)(unsigned char)md[i*3UL]+(unsigned long)
                (unsigned char)md[i*3UL+1UL]*256UL+(unsigned long)
                (unsigned char)md[i*3UL+2UL]*65536UL);
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
      osi_WrStr("mes:", 5ul);
      if (sondeaprs_verb2) {
         for (i = 0UL; i<=7UL; i++) {
            osic_WrINT32((unsigned long)m[i], 7UL);
            osi_WrStr(" ", 2ul);
         } /* end for */
         osi_WrStrLn("", 1ul);
      }
      osic_WrFixed((float)*temp, 3L, 7UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(hr1, 3L, 7UL);
      /*WrStr(" ");WrFixed(hr2, 3,7); */
      osi_WrStr(" ", 2ul);
      osic_WrFixed(p, 2L, 8UL);
   }
/*WrStr(" ");WrFixed(x2, 2,8); */
/*WrStrLn(""); */
} /* end domes() */

static float sondemod_P[13] = {1000.0f,150.0f,100.0f,70.0f,60.0f,50.0f,40.0f,
                30.0f,20.0f,15.0f,10.0f,8.0f,0.0f};

static float sondemod_C[13] = {1.0f,1.0f,1.01f,1.022f,1.025f,1.035f,1.047f,
                1.065f,1.092f,1.12f,1.17f,1.206f,1.3f};

static float _cnst0[13] = {1.0f,1.0f,1.01f,1.022f,1.025f,1.035f,1.047f,
                1.065f,1.092f,1.12f,1.17f,1.206f,1.3f};
static float _cnst[13] = {1000.0f,150.0f,100.0f,70.0f,60.0f,50.0f,40.0f,
                30.0f,20.0f,15.0f,10.0f,8.0f,0.0f};

static double getOzoneCorr(double p)
/* From from ftp://ftp.cpc.ncep.noaa.gov/ndacc/meta/sonde/cv_payerne_snd.txt */
{
   unsigned long i;
   i = 12UL;
   while (i>0UL && (double)_cnst[i]<p) --i;
   return (double)_cnst0[i];
} /* end getOzoneCorr() */

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


static void doozon(const char s[], unsigned long s_len, const double airpres,
                 double * otemp, double * ozon)
{
   *otemp = (double)(float)((unsigned long)(unsigned char)
                s[4UL]+(unsigned long)(unsigned char)s[5UL]*256UL);
   *ozon = (double)(float)((unsigned long)(unsigned char)
                s[2UL]+(unsigned long)(unsigned char)s[3UL]*256UL);
   *otemp = (65535.0-*otemp)*1.3568521031208E-3-35.0;
   *ozon = (*ozon-550.0)*0.00124;
   *ozon =  *ozon*(*otemp+273.15)*3.0769230769231E-3*getOzoneCorr(airpres);
                /* temp and pressure correction */
   if (*ozon<=0.0) *ozon = 0.0;
   if (sondeaprs_verb) {
      osi_WrStr("ozon:", 6ul);
      osic_WrFixed((float)*ozon, 1L, 5UL);
      osi_WrStr("mPa temp:", 10ul);
      osic_WrFixed((float)*otemp, 1L, 5UL);
      osi_WrStrLn("C", 2ul);
   }
/*WrStr(" ");WrFixed(FLOAT(ORD(s[8])+ORD(s[9])*256), 0,8); */
} /* end doozon() */


static void calibfn(char obj[], unsigned long obj_len, char fn[],
                unsigned long fn_len)
{
   unsigned long i;
   X2C_PCOPY((void **)&obj,obj_len);
   aprsstr_Assign(fn, fn_len, obj, obj_len);
   i = 0UL;
   while (i<=fn_len-1 && fn[i]) {
      if (((unsigned char)fn[i]<'0' || (unsigned char)fn[i]>'9')
                && ((unsigned char)fn[i]<'A' || (unsigned char)fn[i]>'Z')) {
         fn[0UL] = 0;
         goto label;
      }
      ++i;
   }
   aprsstr_Append(fn, fn_len, ".cal", 5ul);
   label:;
   X2C_PFREE(obj);
} /* end calibfn() */


static void readcontext(struct CONTEXTR9 * cont, char objname0[],
                unsigned long objname_len)
{
   char fn[1024];
   long fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   initcontext(cont);
   calibfn(objname0, objname_len, fn, 1024ul);
   fd = osi_OpenRead(fn, 1024ul);
   if (fd>=0L) {
      if (osi_RdBin(fd, (char *)cont, sizeof(struct CONTEXTR9)/1u,
                sizeof(struct CONTEXTR9))!=(long)sizeof(struct CONTEXTR9)) {
         initcontext(cont);
      }
      osic_Close(fd);
   }
   X2C_PFREE(objname0);
} /* end readcontext() */


static void wrcontext(struct CONTEXTR9 * cont, char objname0[],
                unsigned long objname_len)
{
   char fn[1024];
   long fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   calibfn(objname0, objname_len, fn, 1024ul);
   if (fn[0U]) {
      fd = osi_OpenWrite(fn, 1024ul);
      if (fd>=0L) {
         osi_WrBin(fd, (char *)cont, sizeof(struct CONTEXTR9)/1u,
                sizeof(struct CONTEXTR9));
         osic_Close(fd);
      }
   }
   X2C_PFREE(objname0);
} /* end wrcontext() */


static void docalib(const char sf[], unsigned long sf_len, char objname0[],
                unsigned long objname_len, struct CONTEXTR9 * cont,
                float * mhz0, unsigned long * frameno)
{
   unsigned long idx;
   unsigned long j;
   unsigned long i;
   char new0;
   *mhz0 = 0.0f;
   new0 = 0;
   i = 0UL;
   for (j = 2UL; j<=11UL; j++) {
      /* object name */
      /*    IF (1 IN cont.calibok) & (sf[j]<>cont.calibdata[j+20])
                THEN cont.calibok:=SET32{} END; */
      if (i<=objname_len-1 && (unsigned char)sf[j]>' ') {
         if (objname0[i]!=sf[j]) new0 = 1;
         objname0[i] = sf[j];
         ++i;
      }
   } /* end for */
   if (i<=objname_len-1) objname0[i] = 0;
   if (new0) readcontext(cont, objname0, objname_len);
   *frameno = (unsigned long)(unsigned char)sf[0UL]+(unsigned long)
                (unsigned char)sf[1UL]*256UL;
   if (sondeaprs_verb) {
      if (new0) osi_WrStr("new ", 5ul);
      osic_WrINT32(*frameno, 1UL); /* frame no */
      osi_WrStr(" ", 2ul);
      osi_WrStr(objname0, objname_len); /*WrStr(" bat:");
                WrHex(ORD(sf[12]), 2);*/
   }
   idx = (unsigned long)(unsigned char)sf[15UL];
   if (idx<32UL) {
      j = idx*16UL;
      for (i = 16UL; i<=31UL; i++) {
         if (j<=511UL) {
            /*      IF (idx IN cont.calibok) & (cont.calibdata[j]<>sf[i])
                THEN cont.calibok:=SET32{} END; */
            cont->calibdata[j] = sf[i];
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
            osi_WrStr(" ", 2ul);
            osic_WrFixed(*mhz0, 2L, 6UL);
            osi_WrStr("MHz ", 5ul);
         }
      }
      if (sondeaprs_verb) {
         osi_WrStr(" calib: ", 9ul);
         for (i = 0UL; i<=31UL; i++) {
            if (i==idx) osi_WrStr("!", 2ul);
            else if (X2C_IN(i,32,cont->calibok)) osi_WrStr("+", 2ul);
            else osi_WrStr("-", 2ul);
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
   unsigned long i;
   n = 0UL;
   for (i = 0UL; i<=31UL; i++) {
      if (X2C_IN(i,32,cs)) ++n;
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
   if (sondeaprs_verb) osi_WrStrLn(fn, 31ul);
   f = osi_OpenWrite("getalmanach", 12ul);
   if (f>=0L) {
      osi_WrBin(f, (char *)fn, 31u/1u, aprsstr_Length(fn, 31ul));
      osic_Close(f);
   }
   else osi_WrStrLn("can not write getalmanach file", 31ul);
} /* end WrRinexfn() */




static void getcall(const char b[], unsigned long b_len, char call[],
                unsigned long call_len)
{
   unsigned long c;
   unsigned long n;
   unsigned long i;
   char tmp;
   call[0UL] = 0;
   n = (unsigned long)(unsigned char)b[0UL]*16777216UL+(unsigned long)
                (unsigned char)b[1UL]*65536UL+(unsigned long)(unsigned char)
                b[2UL]*256UL+(unsigned long)(unsigned char)b[3UL];
   if (n>0UL && (unsigned long)(unsigned char)b[4UL]<=15UL) {
      for (i = 5UL;; i--) {
         c = n%37UL;
         if (c==0UL) call[i] = 0;
         else if (c<27UL) call[i] = (char)((c+65UL)-1UL);
         else call[i] = (char)((c+48UL)-27UL);
         n = n/37UL;
         if (i==0UL) break;
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
   unsigned long i;
   unsigned long almanachage;
   unsigned short crc;
   char typ;
   char sf[256];
   char bb[256];
   char b[256];
   char crdone;
   char calok;
   CALLSSID usercall;
   struct CONTEXTR9 * anonym;
   struct CONTEXTR9 * anonym0;
   struct CONTEXTR9 * anonym1;
   unsigned long tmp;
   /*
   -- reedsolomon is done by sondeudp
     FOR i:=0 TO HIGH(b) DO b[i]:=0C END;
     FOR i:=0 TO 240-6-24-1 DO b[(255-24-1)-i]:=chan[m].rxbuf[i+6] END;
     FOR i:=0 TO 24-1 DO b[(255-1)-i]:=chan[m].rxbuf[i+(240-24)] END;
   --  WrStrLn(" ecco: ");
   --  FOR i:=216 TO 239 DO WrHex(ORD(chan[m].rxbuf[i]), 4) END; WrStrLn("");
                 WrStrLn("");
   --bb:=b;
     res:=decodersc(b, eraspos, 0);
     IF res>0 THEN
       FOR i:=0 TO 240-6-24-1 DO chan[m].rxbuf[i+6]:=b[(255-24-1)-i] END;
       FOR i:=0 TO 24-1 DO chan[m].rxbuf[i+(240-24)]:=b[(255-1)-i] END;
       IF verb THEN WrInt(res, 1); WrStr(" bytes corrected "); END;
     END;
   */
   /*
     WrInt(res, 1); WrStrLn("=rs");
     FOR i:=0 TO 254 DO
       IF b[i]<>bb[i] THEN
         WrInt(i, 4); WrStr(":");WrHex(ORD(bb[i]), 2); WrStr("-");
                WrHex(ORD(b[i]), 2);
       END;
     END;
     WrStrLn(" diffs");
   */
   for (i = 0UL; i<=255UL; i++) {
      b[i] = chan[m].rxbuf[i];
   } /* end for */
   calok = 0;
   getcall(b, 256ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, bb, 256ul);
      osi_WrStr(bb, 256ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   p = 6UL;
   crdone = 1;
   contextr9.posok = 0;
   contextr9.ozontemp = 0.0;
   contextr9.ozon = 0.0;
   mhz = 0.0f;
   for (;;) {
      typ = b[p];
      if (typ=='e') {
         if (sondeaprs_verb) {
            osi_WrStr("cal  ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='g') {
         if (sondeaprs_verb) {
            osi_WrStr("gps  ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='h') {
         if (b[p+2UL]!='\003' && sondeaprs_verb) {
            osi_WrStr("aux ", 5ul);
            crdone = 0;
         }
      }
      else if (typ=='i') {
         if (sondeaprs_verb) {
            osi_WrStr("data ", 6ul);
            crdone = 0;
         }
      }
      else if (typ=='\377') break;
      else {
         osi_WrStr("R92 end ", 9ul);
         if (sondeaprs_verb) {
            osi_WrHex((unsigned long)(unsigned char)typ, 4UL);
            crdone = 0;
         }
         break;
      }
      ++p;
      len = (unsigned long)(unsigned char)b[p]*2UL+2UL; /* +crc */
      if (len>=240UL) {
         if (sondeaprs_verb) {
            osi_WrStr("RS92 Frame too long ", 21ul);
            osic_WrINT32(len, 1UL);
            crdone = 0;
         }
         break;
      }
      ++p;
      j = 0UL;
      /*WrInt(len 3);WrStrLn("=len"); */
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
            osi_WrStr("eof", 4ul); /* error */
            crdone = 0;
            goto loop_exit;
         }
      }
      crdone = 0;
      if ((char)crc!=sf[len-2UL] || (char)X2C_LSH(crc,16,-8)!=sf[len-1UL]) {
         if (sondeaprs_verb) osi_WrStrLn("********* crc error", 20ul);
      }
      else {
         if (typ=='e') {
            docalib(sf, 256ul, objname, 9ul, &contextr9, &mhz, &frameno);
            if (frameno>contextr9.framenum) {
               /* new frame number */
               contextr9.mesok = 0;
               contextr9.posok = 0;
               contextr9.framesent = 0;
               calok = 1;
               contextr9.framenum = frameno;
            }
            else if (contextr9.framenum==frameno && !contextr9.framesent) {
               calok = 1;
            }
            else if (frameno<contextr9.framenum && sondeaprs_verb) {
               osi_WrStrLn("", 1ul);
               osi_WrStr("got out of order frame number ", 31ul);
               osic_WrINT32(frameno, 1UL);
               osi_WrStr(" expecting ", 12ul);
               osic_WrINT32(contextr9.framenum, 1UL);
               crdone = 0;
            }
         }
         else if (typ=='i') {
            { /* with */
               struct CONTEXTR9 * anonym = &contextr9;
               if (calok && anonym->calibok==0xFFFFFFFFUL) {
                  domes(sf, 256ul, &anonym->hp, &anonym->hyg, &anonym->temp);
                  anonym->mesok = 1;
               }
            }
         }
         else if (typ=='g') {
            if (calok) {
               { /* with */
                  struct CONTEXTR9 * anonym0 = &contextr9;
                  dogps(sf, 256ul, &contextr9, &anonym0->timems, &gpstime);
                  if (almread+30UL<=systime) {
                     if (gpspos_readalmanach(semfile, 1024ul, yumafile,
                1024ul, rinexfile, 1024ul, anonym0->timems/1000UL, &almage,
                sondeaprs_verb)) {
                        if (almread+60UL<=systime) {
                           dogps(sf, 256ul, &contextr9, &anonym0->timems,
                &gpstime);
                        }
                        almread = systime;
                     }
                     else {
                        almread = 0UL;
                        almage = 0UL;
                        osi_WrStr("almanach read error", 20ul);
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
                  if (almage+maxalmage>gpstime) {
                     anonym0->posok = 1;
                  }
                  else if (almanachage>0UL) {
                     osic_WrINT32(almanachage/60UL, 10UL);
                     osi_WrStrLn(" Min (almanach too old)", 24ul);
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
                     osi_WrHex((unsigned long)(unsigned char)sf[j], 3UL);
                     if (j==tmp) break;
                  } /* end for */
                  osi_WrStrLn("", 1ul);
                  crdone = 1;
               }
               if (sf[0U]==0) {
                  doozon(sf, 256ul, contextr9.hp, &contextr9.ozontemp,
                &contextr9.ozon);
                  crdone = 1;
               }
            }
         }
         else if (sondeaprs_verb2) {
            tmp = len-1UL;
            j = 0UL;
            if (j<=tmp) for (;; j++) {
               osi_WrHex((unsigned long)(unsigned char)sf[j], 3UL);
               if (j==tmp) break;
            } /* end for */
            crdone = 0;
         }
         if (sondeaprs_verb && !crdone) {
            osi_WrStrLn("", 1ul);
            crdone = 1;
         }
      }
   }
   loop_exit:;
   if (((((contextr9.posok && calok) && almread+60UL>systime)
                && (((sendquick==2UL || sondeaprs_nofilter)
                || contextr9.calibok==0xFFFFFFFFUL)
                || (contextr9.calibok&0x1UL)!=0UL && sendquick==1UL))
                && contextr9.lat!=0.0) && contextr9.long0!=0.0) {
      { /* with */
         struct CONTEXTR9 * anonym1 = &contextr9;
         if (!anonym1->mesok || anonym1->calibok!=0xFFFFFFFFUL) {
            anonym1->hp = 0.0;
            anonym1->hyg = 0.0;
            anonym1->temp = (double)X2C_max_real;
         }
         sondeaprs_senddata(anonym1->lat, anonym1->long0, anonym1->heig,
                anonym1->speed, anonym1->dir, anonym1->climb, anonym1->hp,
                anonym1->hyg, anonym1->temp, anonym1->ozon,
                anonym1->ozontemp, 0.0, 0.0, (double)X2C_max_real,
                (double)mhz, (double)anonym1->hrmsc, (double)anonym1->vrmsc,
                (anonym1->timems/1000UL+86382UL)%86400UL, frameno, objname,
                9ul, almanachage, anonym1->goodsats, usercall, 11ul,
                calperc(anonym1->calibok), 0UL);
         anonym1->framesent = 1;
      }
      crdone = 1;
   }
   if (sondeaprs_verb) {
      if (!crdone) osi_WrStrLn("", 1ul);
      osi_WrStrLn("------------", 13ul);
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

#define sondemod_MAXRANGE 4.7123889803847E-4
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
      if (fabs(dy)>4.7123889803847E-4) *good = 0;
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
   unsigned long i;
   double exlat;
   double exlon;
   double hr;
   pCONTEXTC34 pc0;
   pCONTEXTC34 pc1;
   pCONTEXTC34 pc;
   double stemp;
   char latok;
   char lonok;
   char posok;
   struct CONTEXTC34 * anonym;
   if (rxb[0UL]!='S' || rxb[1UL]!='C') return;
   /* no srsc34 frame */
   i = 0UL;
   do {
      nam[i] = rxb[i];
      ++i;
   } while (i<=8UL);
   if (nam[0U]==0) return;
   /* wait for id */
   ++i;
   j = 0UL;
   do {
      cb[j] = rxb[i];
      ++i;
      ++j;
   } while (j<=4UL);
   getcall(cb, 10ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   j = 0UL;
   do {
      cb[j] = rxb[i];
      ++i;
      ++j;
   } while (j<=9UL);
   sum1 = 0UL;
   sum2 = 65791UL;
   for (i = 0UL; i<=4UL; i++) {
      sum1 += (unsigned long)(unsigned char)cb[i];
      sum2 -= (unsigned long)(unsigned char)cb[i]*(5UL-i);
   } /* end for */
   sum1 = sum1&255UL;
   sum2 = sum2&255UL;
   if (sum1!=(unsigned long)(unsigned char)cb[5U] || sum2!=(unsigned long)
                (unsigned char)cb[6U]) return;
   /* chesum error */
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStr(" ", 2ul);
   }
   osi_WrStr(nam, 9ul);
   osi_WrStr(" ", 2ul);
   pc = pcontextc;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextc = pc1;
         else pc0->next = pc1;
         osic_free((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTC34));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTC34));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((X2C_ADDRESS)pc,(char)0,sizeof(struct CONTEXTC34));
      pc->next = pcontextc;
      pcontextc = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   pc->tused = systime;
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
                      IF verb THEN WrStr("pres "); (* WrFixed(hr, 2, 0);
                WrStr("hPa");*) END;
                    END;
      */
      if (hr<99.9 && hr>(-99.9)) {
         if (sondeaprs_verb) {
            osi_WrStr("temp ", 6ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osi_WrStr("oC", 3ul);
         }
         pc->temp = hr;
         pc->ttemp = systime;
      }
      break;
   case '\007':
      if (hr<99.9 && hr>(-99.9)) {
         if (sondeaprs_verb) {
            osi_WrStr("dewp ", 6ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osi_WrStr("oC", 3ul);
         }
         pc->dewp = hr;
         pc->tdewp = systime;
      }
      break;
   case '\024':
      if (sondeaprs_verb) {
         osi_WrStr("date", 5ul);
         aprsstr_IntToStr((long)(val%1000000UL+1000000UL), 1UL, s, 1001ul);
         s[0U] = ' ';
         osi_WrStr(s, 1001ul);
      }
      break;
   case '\025':
      pc->gpstime = (val/10000UL)*3600UL+((val%10000UL)/100UL)
                *60UL+val%100UL;
      pc->tgpstime = systime;
      if (sondeaprs_verb) {
         aprsstr_TimeToStr(pc->gpstime, s, 1001ul);
         osi_WrStr("time ", 6ul);
         osi_WrStr(s, 1001ul);
      }
      break;
   case '\026':
      hr = latlong(val);
      if (hr<89.9 && hr>(-89.9)) {
         if (sondeaprs_verb) {
            osi_WrStr("lati ", 6ul);
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
            osi_WrStr("long ", 6ul);
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
            osi_WrStr("alti ", 6ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osi_WrStr("m", 2ul);
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
            osi_WrStr("wind ", 6ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osi_WrStr("km/h", 5ul);
         }
         pc->speed = hr*2.7777777777778E-1;
         pc->tspeed = systime;
      }
      break;
   case '\032':
      hr = (double)((float)val*0.1f);
      if (hr>=0.0 && hr<=360.0) {
         if (sondeaprs_verb) {
            osi_WrStr("wdir ", 6ul);
            osic_WrFixed((float)hr, 1L, 0UL);
            osi_WrStr("deg", 4ul);
         }
         pc->dir = hr;
         pc->tdir = systime;
      }
      break;
   default:;
      if (sondeaprs_verb) {
         osi_WrHex((unsigned long)(unsigned char)cb[0U], 0UL);
         osi_WrStr(" ", 2ul);
         osi_WrHex((unsigned long)(unsigned char)cb[1U], 0UL);
         osi_WrHex((unsigned long)(unsigned char)cb[2U], 0UL);
         osi_WrHex((unsigned long)(unsigned char)cb[3U], 0UL);
         osi_WrHex((unsigned long)(unsigned char)cb[4U], 0UL);
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
                anonym->dir, anonym->clmb, 0.0, 0.0, stemp, 0.0, 0.0, 0.0,
                0.0, anonym->dewp, 0.0, 0.0, 0.0,
                ((systime-anonym->tgpstime)+anonym->gpstime)%86400UL, 0UL,
                anonym->name, 9ul, 0UL, 0UL, usercall, 11ul, 0UL, 0UL);
            anonym->lastsent = systime;
         }
      }
   }
   if (sondeaprs_verb) osi_WrStrLn("", 1ul);
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

#define sondemod_DIST 10
/*km*/


static void jumpcheck(float p1, float p2, unsigned long * cnt)
{
   if (p2!=0.0f && (float)fabs(p1-p2)>1.5707963267949E-3f) *cnt = 30UL;
   else if (*cnt>0UL) --*cnt;
} /* end jumpcheck() */

static unsigned long sondemod_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};


static void decodesub(const char b[], unsigned long b_len, pCONTEXTDFM6 pc,
                unsigned long subnum)
{
   unsigned long u;
   unsigned long v;
   long vi;
   double vr;
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
         jumpcheck((float)pc->lat, (float)pc->lat1, &pc->poserr);
      }
      vr = (double)u*0.01;
      if (vr<999.0) {
         pc->speed = vr;
         pc->tspeed = systime;
      }
      if (sondeaprs_verb) {
         osi_WrStr(" Lat: ", 7ul);
         osic_WrFixed((float)(X2C_DIVL(pc->lat,1.7453292519943E-2)), 5L,
                0UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)u*0.036f, 1L, 0UL);
         osi_WrStr("km/h", 5ul);
      }
      break;
   case 3UL:
      vi = (long)bits2val(b, b_len, 0UL, 32UL);
      u = bits2val(b, b_len, 32UL, 16UL);
      vr = (double)vi*1.E-7;
      if (vr<180.0 && vr>(-180.0)) {
         /*IF time() MOD 33=0 THEN vr:=vr+0.1 END; */
         pc->lon1 = pc->lon; /* save 2 values for extrapolating */
         pc->tlon1 = pc->tlon;
         pc->lon = vr*1.7453292519943E-2;
         pc->tlon = systime;
         pc->posok = 1;
         jumpcheck((float)pc->lon, (float)pc->lon1, &pc->poserr);
      }
      vr = (double)u*0.01;
      if (vr<=360.0) {
         pc->dir = vr;
         pc->tdir = systime;
      }
      if (sondeaprs_verb) {
         osi_WrStr(" Long:", 7ul);
         osic_WrFixed((float)(X2C_DIVL(pc->lon,1.7453292519943E-2)), 5L,
                0UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)u*0.01f, 1L, 0UL);
         osi_WrStr(" deg", 5ul);
      }
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
         osi_WrStr(" alti:", 7ul);
         osic_WrFixed((float)pc->alt, 1L, 0UL);
         osi_WrStr("m ", 3ul);
         osic_WrFixed((float)pc->clmb, 1L, 0UL);
         osi_WrStr(" m/s", 5ul);
      }
      break;
   } /* end switch */
} /* end decodesub() */


static void decodedfm6(const char rxb[], unsigned long rxb_len,
                unsigned long ip, unsigned long fromport)
{
   unsigned long rt;
   char db[56];
   pCONTEXTDFM6 pc0;
   pCONTEXTDFM6 pc1;
   pCONTEXTDFM6 pc;
   OBJNAME nam;
   char cb[10];
   char s[1001];
   CALLSSID usercall;
   unsigned long ib;
   unsigned long j;
   unsigned long i;
   double exlat;
   double exlon;
   char latok;
   char lonok;
   struct CONTEXTDFM6 * anonym;
   if (rxb[0UL]!='D' || rxb[1UL]!='F') return;
   /* no dfm06 frame */
   i = 0UL;
   do {
      nam[i] = rxb[i];
      ++i;
   } while (i<=8UL);
   if (nam[0U]==0) return;
   /* wait for id */
   ++i;
   j = 0UL;
   do {
      cb[j] = rxb[i]; /* call */
      ++i;
      ++j;
   } while (j<=4UL);
   getcall(cb, 10ul, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   rt = 0UL;
   for (j = 0UL; j<=3UL; j++) {
      rt = rt*256UL+(unsigned long)(unsigned char)rxb[i]; /* realtime */
      ++i;
   } /* end for */
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStr(" ", 2ul);
   }
   osi_WrStr(nam, 9ul);
   osi_WrStr(" ", 2ul);
   pc = pcontextdfm6;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextdfm6 = pc1;
         else pc0->next = pc1;
         osic_free((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTDFM6));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTDFM6));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((X2C_ADDRESS)pc,(char)0,sizeof(struct CONTEXTDFM6));
      pc->next = pcontextdfm6;
      pcontextdfm6 = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   if (rt+1UL>=pc->actrt) {
      /* not an older frame */
      pc->tused = systime;
      pc->actrt = rt;
      pc->posok = 0;
      i += 4UL;
      for (j = 0UL; j<=6UL; j++) {
         for (ib = 0UL; ib<=7UL; ib++) {
            db[ib+8UL*j] = X2C_IN(7UL-ib,8,
                (unsigned char)(unsigned char)rxb[i]);
         } /* end for */
         ++i;
      } /* end for */
      decodesub(db, 56ul, pc, 0UL);
      for (j = 0UL; j<=6UL; j++) {
         for (ib = 0UL; ib<=7UL; ib++) {
            db[ib+8UL*j] = X2C_IN(7UL-ib,8,
                (unsigned char)(unsigned char)rxb[i]);
         } /* end for */
         ++i;
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
            if ((sondeaprs_verb && !sondeaprs_nofilter) && pc->poserr>0UL) {
               osi_WrStr(" Pos jump (", 12ul);
               osic_WrINT32(pc->poserr, 1UL);
               osi_WrStrLn(")", 2ul);
            }
            if ((lonok && latok) && (pc->poserr==0UL || sondeaprs_nofilter)) {
               sondeaprs_senddata(exlat, exlon, anonym->alt, anonym->speed,
                anonym->dir, anonym->clmb, 0.0, 0.0, (double)X2C_max_real,
                0.0, 0.0, 0.0, 0.0, (double)X2C_max_real, 0.0, 0.0, 0.0,
                anonym->actrt%86400UL, 0UL, anonym->name, 9ul, 0UL, 0UL,
                usercall, 11ul, 0UL, 0UL);
               anonym->lastsent = systime;
            }
         }
      }
   }
   else if (sondeaprs_verb) osi_WrStr(" got old frame ", 16ul);
   if (sondeaprs_verb) osi_WrStrLn("", 1ul);
} /* end decodedfm6() */

/*------------------------------ RS41 */

static void WrChChk(char ch)
{
   if ((unsigned char)ch>=' ' && (unsigned char)ch<'\177') {
      osi_WrStr((char *) &ch, 1u/1u);
   }
} /* end WrChChk() */


static double atan20(double x, double y)
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
      rh = (double)osic_sqrt((float)h);
      xh = x+rh;
      *long0 = atan20(xh, y)*2.0;
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


static unsigned long getcard16(const char frame[], unsigned long frame_len,
                unsigned long p)
{
   return (unsigned long)(unsigned char)frame[p]+256UL*(unsigned long)
                (unsigned char)frame[p+1UL];
} /* end getcard16() */


static long getint16(const char frame[], unsigned long frame_len,
                unsigned long p)
{
   unsigned long n;
   n = (unsigned long)(unsigned char)frame[p]+256UL*(unsigned long)
                (unsigned char)frame[p+1UL];
   if (n>=32768UL) return (long)(n-65536UL);
   return (long)n;
} /* end getint16() */


static unsigned long gethex(const char frame[], unsigned long frame_len,
                unsigned long p, unsigned long nibb)
{
   unsigned long c;
   unsigned long n;
   n = 0UL;
   while (nibb>0UL) {
      n = n*16UL;
      /*WrStr("<<"); WrStr(frame[p]); WrStr(">>"); */
      c = (unsigned long)(unsigned char)frame[p];
      if (c>=48UL && c<=57UL) n += c-48UL;
      else if (c>=65UL && c<=70UL) n += c-55UL;
      else return 0UL;
      ++p;
      --nibb;
   }
   /*WrInt(n,5); */
   return n;
} /* end gethex() */


/* SQ7BR */
static void WrHexToFile(const long f,const char rxb[], unsigned long rxb_len)
{
   int i;
   if (osic_FdValid(f)) {
	   for (i=8;i<rxb_len;i++)
		   osi_WrHexFd(f,rxb[i],3ul);
	   osi_WrBin(f,"\r\n",3ul,2ul);
   }
   else osi_WrStrLn("can't write hex file", 31ul);
} /* end WrHexToFile() */





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
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(*lat,1.7453292519943E-2)), 5L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)(X2C_DIVL(*long0,1.7453292519943E-2)), 5L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)*heig, 1L, 1UL);
      osi_WrStr("m ", 3ul);
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
   vn = (-(vx*(double)osic_sin((float)*lat)*(double)osic_cos((float)*long0))
                -vy*(double)osic_sin((float)*lat)*(double)osic_sin((float)
                *long0))+vz*(double)osic_cos((float)*lat);
   ve = -(vx*(double)osic_sin((float)*long0))+vy*(double)osic_cos((float)
                *long0);
   vu = vx*(double)osic_cos((float)*lat)*(double)osic_cos((float)*long0)
                +vy*(double)osic_cos((float)*lat)*(double)osic_sin((float)
                *long0)+vz*(double)osic_sin((float)*lat);
   *dir = X2C_DIVL(atan20(vn, ve),1.7453292519943E-2);
   if (*dir<0.0) *dir = 360.0+*dir;
   *speed = (double)osic_sqrt((float)(vn*vn+ve*ve));
   *clmb = vu;
   if (sondeaprs_verb) {
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)( *speed*3.6), 2L, 1UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrFixed((float)*dir, 1L, 1UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)vu, 1L, 1UL);
      osi_WrStr("m/s", 4ul);
   }
} /* end posrs41() */


static double altToPres(double a)
/* meter to hPa */
{
   if (a<=0.0) return 1010.0;
   else if (a>40000.0) return 0.0;
   else if (a>15000.0) {
      return (double)(osic_exp((float)(a*(-1.5873015873016E-4)+0.2629))
                *1000.0f);
   }
   else {
      return (double)(1010.0f*osic_exp(osic_ln((float)((293.0-0.0065*a)
                *3.4129692832765E-3))*5.26f));
   }
   return 0;
} /* end altToPres() */


static double calcOzone(double uA, double temp, double airpres)
{
   return 4.307E-4*uA*(temp+273.15)*28.57*getOzoneCorr(airpres);
/*
        From Mast/Keystone ozonsensor 730-10 datasheet:
                1 uA per 50 umb Ozone (1 uA per 5 mPa)
                Airflow 190-230 ml/min (avg. 210 ml/min => 3.5 ml/s => 100 ml in 28,
                57 s

        Default ozone formula from ftp://ftp.cpc.ncep.noaa.gov/ndacc/meta/sonde/cv_payerne_snd.txt

                POZ(nb)  = 0.004307 * i * Tp * t * E(p)
                => POZ(mPa)  = 0.0004307 * i * Tp * t * E(p)

                where:  i is the current from the sensor in uA
                         t is the time in seconds to pump 0.100 liters of air through the pump
                         E(p) is the pump efficiency correction
                         Tp is the pump temperature
*/
} /* end calcOzone() */

static unsigned short sondemod_POLYNOM0 = 0x1021U;

static unsigned short sondemod_burstIndicatorBytes[12] = {2U,262U,276U,391U,
                306U,0U,0U,0U,255U,255U,0U,0U};

static unsigned short _cnst1[12] = {2U,262U,276U,391U,306U,0U,0U,0U,255U,
                255U,0U,0U};

static void decoders41(const char rxb[], unsigned long rxb_len,
                unsigned long ip, unsigned long fromport)
{
   OBJNAME nam;
   long res;
   char s[1001];
   CALLSSID usercall;
   unsigned long frameno;
   unsigned long len;
   unsigned long p;
   unsigned long ic;
   unsigned long j;
   unsigned long i;
   char nameok;
   char calok;
   unsigned short crc;
   char typ;
   pCONTEXTR41 pc0;
   pCONTEXTR41 pc1;
   pCONTEXTR41 pc;
   double ozonval;
   double climb;
   double dir;
   double speed;
   double heig;
   double long0;
   double lat;
   unsigned long tmp;
   /*SQ7BR*/
   unsigned long bkValue;
   char frameType;
   char hwType[11]; //with zero end byte
   char hwId[5];    //with zero end byte
   char sondeType[8]; //with zero end byte
   char serialNumber[6];
   unsigned long firmwareVersion;
   unsigned long version;
   unsigned long subversionMajor;
   unsigned long subversionMinor;

   long fHandle;
   char fName[1024]; //fileName
   unsigned long systime;
   char timeValue[11];


   calok = 0;
   nameok = 0;
   nam[0U] = 0;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   ozonval = 0.0;
   getcall(rxb, rxb_len, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 1001ul);
      osi_WrStr(s, 1001ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   p = 57UL;
   if (sondeaprs_verb) osi_WrStr("R41 ", 5ul);
   for (;;) {
	   //each block with own id, length, crc
      if (p+4UL>=rxb_len-1) break;
      typ = rxb[p++];
      //++p;
      len = (unsigned long)(unsigned char)rxb[p++]+2UL;
      //++p;
      if (p+len>=rxb_len-1) break;

      //crc start
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
         if (sondeaprs_verb) osi_WrStr(" ----  crc err ", 16ul);

      //crc end
         break;
      }
      if (typ=='y') { //config
         nameok = 1;
         for (i = 0UL; i<=7UL; i++) {
            nam[i] = rxb[p+2UL+i];
            if ((unsigned char)nam[i]<=' ' || (unsigned char)nam[i]>'Z') {
               nameok = 0;
            }
         } /* end for */
         nam[8U] = 0;
         pc = pcontextr4;
         pc0 = 0;
         for (;;) {
            if (pc==0) break;
            pc1 = pc->next;
            if (pc->tused+3600UL<systime) {
               /* timed out */
               if (pc0==0) pcontextr4 = pc1;
               else pc0->next = pc1;
               /*SQ7BR*/
               osic_Close(pc->fileHex);
               osic_free((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTR41));
            }
            else {
               if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
               pc0 = pc;
            }
            pc = pc1;
         }
         if (pc==0) {
            osic_alloc((X2C_ADDRESS *) &pc, sizeof(struct CONTEXTR41));
            if (pc==0) Error("allocate context out im memory", 31ul);
            memset((X2C_ADDRESS)pc,(char)0,sizeof(struct CONTEXTR41));
            pc->next = pcontextr4;
            pcontextr4 = pc;
            aprsstr_Assign(pc->name, 9ul, nam, 9ul);
            if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
            /*SQ7BR*/
            if (sondeaprs_verbHex) {
            	   systime=osic_time();
                fName[0ul]=0ul;
             	aprsstr_Append(fName,1024ul,sondeaprs_verbHexDir,1024ul);
             	aprsstr_Append(fName,1024ul,"/digitalSonde_",15ul);
             	aprsstr_DateToStr(systime,timeValue,21ul);
             	aprsstr_Append(fName,1024ul,timeValue,10ul);
             	aprsstr_Append(fName,1024ul,"_",2ul);
             	aprsstr_Append(fName,1024ul,pc->name,9ul);
             	aprsstr_Append(fName,1024ul,".txt",5ul);
                 osi_WrStr("create file: ", 14ul);
                 osi_WrStrLn(fName, 1024ul);
                 osic_WrLn();

             	//testFHandle=osi_OpenWrite("/home/pi/dxlAPRS/raw/L5120074.hex", 37ul);
                 testFHandle=osi_OpenWrite(fName,1024ul);
             	 if (osic_FdValid(testFHandle)){
             		 pc->fileHex=testFHandle;
             	 }else  		 osi_WrStrLn("couldn't create file", 21ul);


            }
         }

         frameno = (unsigned long)getint16(rxb, rxb_len, p);
         if (frameno>pc->framenum) {
            /* new frame number */
            pc->framesent = 0;
            calok = 1;
            pc->framenum = frameno;
            pc->tused = systime;
         }
         else if (pc->framenum==frameno && !pc->framesent) calok = 1;
         else if (frameno<pc->framenum && sondeaprs_verb) {
            osi_WrStrLn("", 1ul);
            osi_WrStr("got out of order frame number ", 31ul);
            osic_WrINT32(frameno, 1UL);
            osi_WrStr(" expecting ", 12ul);
            osic_WrINT32(pc->framenum, 1UL);
         }

         if (sondeaprs_verb) {
         	osi_WrStr(pc->name, 9ul);
         	osi_WrStr(" ", 2ul);
            osi_WrStr(objname, 9ul);
            osic_WrINT32(pc->framenum, 0UL);
         }

         /*SQ7BR*/

          frameType=(char)(rxb[p+23UL]);
          if (sondeaprs_verb) {
                      osi_WrStr(", FrTpe=", 13ul);
                      osic_WrUINT32(frameType, 0UL);
                   }

          switch (frameType){
          case 0UL:
              pc->mhz0 = (float)(getcard16(rxb, rxb_len,
                  p+26UL)/64UL+40000UL)*0.01f+0.0005f;
              if (sondeaprs_verb) {
                 osi_WrStr(", Freq=", 9ul);
                 osic_WrFixed(pc->mhz0, 3UL,7UL);
              }

        	                break;
          case 1UL:
        	  //serial number
        	  nameok=1;
			  for (i = 0UL; i<=4UL; i++) {
						  serialNumber[i] = rxb[p+24UL+i];
						  if ((unsigned char)serialNumber[i]<=' ' || (unsigned char)serialNumber[i]>'Z') {
							  break;
							  //nameok = 0;
						  }
					   } /* end for */
					   serialNumber[i] = 0;
			  if (nameok) {
				  aprsstr_Assign(pc->serialNumber, 6ul, serialNumber, 6ul);
			  }

			  //firmware wersion
               pc->swVersion=(unsigned long)(getcard16(rxb, rxb_len,p+29UL));

              if (sondeaprs_verb) {
            	 osi_WrStr(", SerNum=", 10ul);
            	 osic_WrStr(pc->serialNumber,6ul);


            	 version=pc->swVersion / 10000;
            	 subversionMajor=(pc->swVersion-(version*10000)) / 100;
            	 subversionMinor=pc->swVersion-(version*10000)-(subversionMajor*100);

            	 osi_WrStr(", SwVersion=", 13ul);
            	 if (version<10){
            	      osi_WrStr("0", 2ul);
            	      osic_WrUINT32(version,1ul);
            	 } else osic_WrUINT32(version,2ul);
            	 osi_WrStr(".", 2ul);

            	 if (subversionMajor<10){
            		 osi_WrStr("0", 2ul);
            		 osic_WrUINT32(subversionMajor,1ul);
            	 } else osic_WrUINT32(subversionMajor,2ul);
            	 osi_WrStr(".", 2ul);

            	 if (subversionMinor<10){
            		 osi_WrStr("0", 2ul);
            		 osic_WrUINT32(subversionMinor,1ul);
            	 } else osic_WrUINT32(subversionMinor,2ul);
              }
   	        break;
          case 2UL:
        	  //killTimer
        	  pc->killTimer = (unsigned long)(getcard16(rxb, rxb_len,
                      p+31UL));

        	  //burstKill
               	 bkValue= ((unsigned long)(unsigned char)rxb[p+35UL]&1UL);
                switch (bkValue) {
                case 0:
                	 pc->burstKill =bk_off;
                	 break;
                case 1:
                	 pc->burstKill =bk_on;
                	 break;
               // default:
                }




              if (sondeaprs_verb) {
            	  //killT
                  osi_WrStr(", KillTimer=", 13ul);
                  osic_WrHex(pc->killTimer, 0UL);

            	  switch (pc->burstKill) {
            	  case bk_off:
            		  osi_WrStr(", BK=Off", 9ul);
            		  break;
            	  case bk_on:
            	      osi_WrStr(", BK=On", 8ul);
            	      break;
            	  default:
            		  osi_WrStr(", BK=NaN", 9ul);
            		  break;
            	  }
              }
                  	        break;
          case 3UL:
                  	        break;
          case 4UL:
                 	        break;
          case 5UL:
                  	        break;
          case 6UL:
                  	        break;
          case 7UL:
                  	        break;
          case 8UL:
                  	        break;
          case 9UL:
                  	        break;
          case 10UL:
                  	        break;
          case 11UL:
                  	        break;
          case 12UL:
                  	        break;
          case 13UL:
                  	        break;
          case 14UL:
                  	        break;
          case 15UL:
                  	        break;
          case 16UL:
                  	        break;
          case 17UL:
                  	        break;
          case 18UL:
                  	        break;
          case 19UL:
                  	        break;
          case 20UL:
                  	        break;
          case 21UL:
                  	        break;
          case 22UL:
                  	        break;
          case 23UL:
                  	        break;
          case 24UL:
                  	        break;
          case 25UL:
                  	        break;
          case 26UL:
                  	        break;
          case 27UL:
                  	        break;
          case 28UL:
                  	        break;
          case 29UL:
                  	        break;
          case 30UL:
                  	        break;
          case 31UL:
                  	        break;
          case 32UL:
                  	        break;
          case 33UL:
        	  //sondeType
        	  nameok=1;
			  for (i = 0UL; i<=6UL; i++) {
						  sondeType[i] = rxb[p+32UL+i];
						  if ((unsigned char)sondeType[i]<=' ' || (unsigned char)sondeType[i]>'Z') {
							  break;
							  //nameok = 0;
						  }
					   } /* end for */
					   sondeType[i] = 0;
			  if (nameok) {
				  aprsstr_Assign(pc->sondeType, 8ul, sondeType, 7ul);
			  }

              if (sondeaprs_verb) {
                 osi_WrStr(", SondeTpe=", 12ul);
                 osic_WrStr(pc->sondeType, 8UL);
              }
   	          break;
          case 34UL:
        	  //HW Type , HW
        	  nameok=1;
        	  for (i = 0UL; i<=9UL; i++) {
        	              hwType[i] = rxb[p+26UL+i];
        	              if ((unsigned char)hwType[i]<=' ' || (unsigned char)hwType[i]>'Z') {
        	                  break;
        	            	  //nameok = 0;
        	              }
        	           } /* end for */
        	           hwType[i] = 0;
        	  if (nameok) {
        		  aprsstr_Assign(pc->hwType, 11ul, hwType, 11ul);
        	  }

        	  nameok=1;
			  for (i = 0UL; i<=3UL; i++) {
						  hwId[i] = rxb[p+36UL+i];
						  if ((unsigned char)hwId[i]<=' ' || (unsigned char)hwId[i]>'Z') {
							  break;
							  //nameok = 0;
						  }
					   } /* end for */
					   hwId[i] = 0;
			  if (nameok) {
				  aprsstr_Assign(pc->hwId, 5ul, hwId, 5ul);
			  }

              if (sondeaprs_verb) {
                 osi_WrStr(", HWTpe=", 9ul);
                 osic_WrStr(pc->hwType, 11UL);
                 osi_WrStr(", HWId=", 8ul);
                 osic_WrStr(pc->hwId, 5UL);
              }


   	        break;

          }

      }
      else if (typ=='z') {
      }
      else if (typ=='|') {
         if (pc) {
            pc->gpssecond = (unsigned long)((getint32(rxb, rxb_len,
                p+2UL)/1000L+86382L)%86400L); /* gps TOW */

            if (sondeaprs_verb) {
            	 osi_WrStr(", gpsSec=", 10ul);
            	 osic_WrFixed(pc->gpssecond, 0L, 8UL);
            }

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
            pc->hp = altToPres(heig); /* make hPa out of gps alt for ozone */
         }
      }
      else if (typ=='~') {
         /* external device */
         if (len==23UL) {
            /* ozon values */
            if (pc) {
               /*          pc^.ozonInstType:=gethex(rxb, p+1, 2); */
               /*          pc^.ozonInstNum:=gethex(rxb, p+3, 2); */
               res = (long)gethex(rxb, rxb_len, p+5UL, 4UL);
               if (res>=32768L) res = 32768L-res;
               pc->ozonTemp = (double)res*0.01;
               pc->ozonuA = (double)gethex(rxb, rxb_len, p+9UL, 5UL)*0.0001;
               pc->ozonBatVolt = (double)gethex(rxb, rxb_len, p+14UL,
                2UL)*0.1;
               pc->ozonPumpMA = (double)gethex(rxb, rxb_len, p+16UL, 3UL);
               pc->ozonExtVolt = (double)gethex(rxb, rxb_len, p+19UL,
                2UL)*0.1;
               ozonval = calcOzone(pc->ozonuA, pc->ozonTemp, pc->hp);
               if (sondeaprs_verb) {
                  osi_WrStr(" OZON:(", 8ul);
                  osic_WrFixed((float)pc->ozonTemp, 2L, 1UL);
                  osi_WrStr("oC ", 4ul);
                  osic_WrFixed((float)pc->ozonuA, 4L, 1UL);
                  osi_WrStr("uA ", 4ul);
                  osic_WrFixed((float)ozonval, 3L, 1UL);
                  osi_WrStr("mPa ", 5ul);
                  osic_WrFixed((float)pc->ozonBatVolt, 1L, 1UL);
                  osi_WrStr("BatV ", 6ul);
                  osic_WrFixed((float)pc->ozonPumpMA, 0L, 1UL);
                  osi_WrStr("mA ", 4ul);
                  osic_WrFixed((float)pc->ozonExtVolt, 1L, 1UL);
                  osi_WrStr("ExtV", 5ul);
                  osi_WrStr(")", 2ul);
               }
            }
         }
         else if (len==24UL) {
            /* Ozon id-data */
            if (sondeaprs_verb) {
               osi_WrStr(" OZONID:(", 10ul);
               tmp = p+12UL;
               i = p+5UL;
               if (i<=tmp) for (;; i++) {
                  WrChChk(rxb[i]);
                  if (i==tmp) break;
               } /* end for */
               if (((unsigned long)(unsigned char)rxb[17UL]&1)) {
                  osi_WrStr(" NotCal", 8ul);
               }
               osi_WrStr(" V:", 4ul);
               osic_WrFixed((float)gethex(rxb, rxb_len, p+18UL, 2UL)*0.1f,
                1L, 1UL);
               osi_WrStr(")", 2ul);
            }
         }
      }
      else if (typ=='v') {
      }
      else {
         /*             WrStrLn("76 frame"); */
         break;
      }
      if (typ=='v') break;
      p += len;
   }  //for

   if (
		      pc->hwvwersion>202013
		   && pc->burstKill==bk_on
		   && pc->frameBurst==0
		   && climb<0.0
		   && heig>sondemod_MIN_BURST_HEIGHT
      )  pc->frameBurst=pc->framenum;

   if (pc->frameBurst>0ul) {
	   //left=burstMoment+afterLimit-currentMoment
	   pc->afterBurstTimerLeft=(long)(pc->frameBurst+sondemod_AFTER_BURST_OFF_TIMER-pc->framenum)/60;
   }


   if (sondeaprs_verb) {

       if ( pc->frameBurst>0ul
          ) {
    	   osi_WrStr(" AfterBurstToOffLeft=", 22ul);
    	   osic_WrFixed((float)(pc->afterBurstTimerLeft), 0UL,6ul);
          }


	   osi_WrStrLn("", 1ul);

   }

   	   	   /*sq7br*/
   if (sondeaprs_verbHex) WrHexToFile(pc->fileHex,rxb,rxb_len);

   if ((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) {
      sondeaprs_senddataRS41(usercall,11ul,frameno,lat,long0,heig,speed,dir,climb,ozonval,pc);
      pc->framesent = 1;
   }
/*  IF verb THEN WrStrLn("") END;   */
} /* end decoders41() */


static void udprx(void)
{
   unsigned long fromport;
   unsigned long ip;
   long len;
   len = udpreceive(rxsock, chan[sondemod_LEFT].rxbuf, 520L, &fromport, &ip);
   systime = osic_time();
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
      decodec34(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport);
   }
   else if (len==37L) {
      decodedfm6(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport);
   }
   else if (len==520L) {
      decoders41(chan[sondemod_LEFT].rxbuf, 520ul, ip, fromport);
   }
   else usleep(10000UL);
} /* end udprx() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(OBJNAME)!=9) X2C_ASSERT(0);
   if (sizeof(CALLSSID)!=11) X2C_ASSERT(0);
   sondeaprs_BEGIN();
   gpspos_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   Parms();
   /*  initrsc; */
   initcontext(&contextr9);
   pcontextc = 0;
   pcontextdfm6 = 0;
   pcontextr4 = 0;
   objname[0] = 0;
   almread = 0UL;
   almage = 0UL;
   lastip = 0UL;
   lastport = 0UL;
   systime = osic_time();
   /*testalm; */
   for (;;) udprx();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
