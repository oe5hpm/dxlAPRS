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
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#include <math.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef gpspos_H_
#include "gpspos.h"
#endif
#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#include <signal.h>

/* decode RS92, RS41, SRS-C34, DFM, M10, iMET Radiosonde by OE5DXL */
#define sondemod_CONTEXTLIFE 3600
/* seconds till forget context after last heared */

#define sondemod_DAYSEC 86400

#define sondemod_GPSTIMECORR 18
/* leap seconds */

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

#define sondemod_EARTH 6370.0

#define sondemod_MYLAT 8.4214719496019E-1
/* only for show sat elevations if no pos decode */

#define sondemod_MYLONG 2.2755602787502E-1

#define sondemod_NEWALMAGE 30
/* every s reread almanach */

#define sondemod_FASTALM 4
/* reread almanach if old */

#define sondemod_DEFAULTSUBTYP 255
/* dfm subtype wild card */

typedef uint32_t SET51[2];

typedef char FILENAME[1024];

typedef char OBJNAME[9];

typedef char CALLSSID[11];

enum CHANNELS {sondemod_LEFT, sondemod_RIGHT};


struct CHAN;


struct CHAN {
   uint32_t rxbyte;
   uint32_t rxbitc;
   uint32_t rxp;
   char rxbuf[560];
};

struct DFNAMES;


struct DFNAMES {
   uint8_t start;
   uint16_t dat[2];
   uint8_t cnt[2];
   uint8_t errcnt;
};

struct CONTEXTR9;


struct CONTEXTR9 {
   char calibdata[512];
   uint32_t calibok;
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
   uint32_t goodsats;
   uint32_t timems;
   uint32_t framenum;
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
   double latv1;
   double lonv1;
   double alt;
   double vlon;
   double vlat;
   double speed;
   double dir;
   double temp;
   double hum;
   uint32_t lastsent;
   uint32_t gpsdate;
   uint32_t gpstime;
   uint32_t tgpstime;
   uint32_t tlat;
   uint32_t tlon;
   uint32_t tlat1;
   uint32_t tlon1;
   uint32_t tlatv1;
   uint32_t tlonv1;
   uint32_t talt;
   uint32_t tspeed;
   uint32_t tdir;
   uint32_t ttemp;
   uint32_t thum;
   uint32_t tused;
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
   uint32_t gpsdate;
   uint32_t lastsent;
   uint32_t tlat;
   uint32_t tlon;
   uint32_t tlat1;
   uint32_t tlon1;
   uint32_t talt;
   uint32_t tspeed;
   uint32_t tdir;
   uint32_t actrt;
   uint32_t tused;
   char d9;
   char posok;
   uint32_t poserr; /* count down after position jump */
   /* new df serial */
   uint32_t lastfrid;
   uint32_t nameregtop;
   struct DFNAMES namereg[50];
};
/* new df serial */

struct CONTEXTR4;

typedef struct CONTEXTR4 * pCONTEXTR4;


struct CONTEXTR4 {
   pCONTEXTR4 next;
   OBJNAME name;
   char flightstate;
   char bk;
   char posok;
   char framesent;
   float mhz0;
   uint32_t encrcnt;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t tused;
   double hp;
   uint32_t ozonInstType;
   uint32_t ozonInstNum;
   double ozonTemp;
   double ozonuA;
   double ozonBatVolt;
   double ozonPumpMA;
   double ozonExtVolt;
   int32_t txtime;
   char calibdata[816];
   SET51 calibok;
};
/*
       ozon_id_ser           : ARRAY[0..8] OF CHAR;
       ozon_id_diag          : INT16;
       ozon_id_version       : REAL;  
*/

struct CONTEXTM10;

typedef struct CONTEXTM10 * pCONTEXTM10;


struct CONTEXTM10 {
   pCONTEXTM10 next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t tused;
};

struct CONTEXTIMET;

typedef struct CONTEXTIMET * pCONTEXTIMET;


struct CONTEXTIMET {
   pCONTEXTIMET next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   uint32_t gpssecond;
   uint32_t framenum;
   uint32_t talt;
   uint32_t tused;
   uint32_t pumpmA;
   uint32_t frnum;
   double lat;
   double long0;
   float speed;
   float clb;
   float dir;
   float alt;
   float otemp;
   float ozoneuA;
   float pumpV;
   float hpa;
   float rtok;
   float hum;
   float vbatt;
};

struct CONTEXTMP3;

typedef struct CONTEXTMP3 * pCONTEXTMP3;


struct CONTEXTMP3 {
   pCONTEXTMP3 next;
   OBJNAME name;
   char ser[21];
   char id1ok;
   char id2ok;
   char posok;
   char framesent;
   uint32_t id1;
   uint32_t id2;
   uint32_t lastgpstime;
   uint32_t gpstime;
   uint32_t tused;
   double lat;
   double long0;
   float speed;
   float clb;
   float dir;
   float alt;
};

struct CONTEXTMEISEI;

typedef struct CONTEXTMEISEI * pCONTEXTMEISEI;


struct CONTEXTMEISEI {
   pCONTEXTMEISEI next;
   OBJNAME name;
   char ser[21];
   char posok;
   char framesent;
   uint32_t lastgpstime;
   uint32_t gpstime;
   uint32_t tused;
   double lat;
   double long0;
   float mhz0;
   float speed;
   float clb;
   float dir;
   float alt;
   uint32_t gpssum;
   uint32_t fnum;
   uint32_t subtype;
};

struct DFMTYPES;

typedef struct DFMTYPES * pDFMTYPES;


struct DFMTYPES {
   pDFMTYPES next;
   uint32_t n;
   char t[9];
};

static FILENAME semfile;

static FILENAME yumafile;

static FILENAME rinexfile;

static FILENAME encryptmsgfn;

static uint32_t sendquick; /* 0 send if full calibrated, 1 with mhz, 2 always */

static int32_t leapseconds;

static uint32_t almread;

/* time last almanach read */
static uint32_t almrequest;

/* seconds rinex age to request new */
static uint32_t almage;

static uint32_t systime;

static FILENAME soundfn;

static struct CHAN chan[2];

static gpspos_SATS lastsat;

static float coeff[256];

static float mhz;

static OBJNAME objname;

static int32_t rxsock;

static uint32_t maxalmage;

static uint32_t lastip;

static uint32_t lastport;

static char mycall[100];

static struct CONTEXTR9 contextr9;

static pCONTEXTC34 pcontextc;

static pCONTEXTDFM6 pcontextdfm6;

static pCONTEXTR4 pcontextr4;

static pCONTEXTM10 pcontextm10;

static pCONTEXTIMET pcontextimet;

static pCONTEXTMP3 pcontextmp3;

static pCONTEXTMEISEI pcontextmeisei;

static struct sondeaprs_SDRBLOCK sdrblock;

static pDFMTYPES dftypes;

static char sendmhzfromsdr;

static char dfmswap;

static struct sondeaprs_XDATA xdatablock;

static uint16_t CRCTAB[256];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static char hex(uint32_t n)
{
   n = n&15UL;
   if (n<10UL) return (char)(n+48UL);
   else return (char)(n+55UL);
   return 0;
} /* end hex() */


static float pow0(float x, uint32_t y)
{
   float z;
   z = x;
   while (y>1UL) {
      z = z*x;
      --y;
   }
   return z;
} /* end pow() */


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


static float SaveReal(uint32_t c)
{
   uint32_t e;
   e = c/16777216UL;
   if (e==127UL) return X2C_max_real;
   if (e==255UL) return X2C_min_real;
   return *X2C_CAST(&c,uint32_t,float,float *);
} /* end SaveReal() */


static void WrSemaFile(const char fn[], uint32_t fn_len,
                const char name[], uint32_t name_len,
                const struct sondeaprs_SDRBLOCK sdr)
{
   int32_t f;
   char h[1000];
   char s[1000];
   aprsstr_Assign(s, 1000ul, name, name_len);
   if (sdr.valid && sdr.freq) {
      aprsstr_Append(s, 1000ul, " ", 2ul);
      aprsstr_FixToStr((float)((int32_t)(sdr.freq/100UL)+sdr.afc)
                *0.001f, 4UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
      aprsstr_Append(s, 1000ul, "MHz", 4ul);
   }
   f = osi_OpenWrite(fn, fn_len);
   if (f>=0L) {
      osi_WrBin(f, (char *)s, 1000u/1u, aprsstr_Length(s, 1000ul));
   }
   osic_Close(f);
} /* end WrSemaFile() */


static char GetNum(const char h[], uint32_t h_len, char eot,
                 uint32_t * p, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==0 || h[*p]==eot;
} /* end GetNum() */


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


static uint32_t getdfhex(const char h[], uint32_t h_len,
                uint32_t * i)
{
   uint32_t n;
   char ok0;
   n = 0UL;
   ok0 = 0;
   for (;;) {
      if (*i>h_len-1) break;
      if ((uint8_t)h[*i]<'0') break;
      if ((uint8_t)h[*i]<='9') {
         n = (n*16UL+(uint32_t)(uint8_t)h[*i])-48UL;
         ok0 = 1;
      }
      else {
         if ((uint8_t)h[*i]<'A' || (uint8_t)h[*i]>'F') break;
         n = (n*16UL+(uint32_t)(uint8_t)h[*i])-55UL;
         ok0 = 1;
      }
      ++*i;
   }
   if (ok0) return n;
   return 256UL;
} /* end getdfhex() */


static void Parms(void)
{
   char err;
   FILENAME h;
   uint32_t cnum;
   uint32_t n;
   uint32_t i;
   pDFMTYPES dftyp;
   sondeaprs_pUDPDESTS udest;
   mycall[0U] = 0;
   semfile[0] = 0;
   yumafile[0] = 0;
   rinexfile[0] = 0;
   encryptmsgfn[0] = 0;
   err = 0;
   sondeaprs_rectfence.leftdown.lat = 0.0f;
   rxsock = -1L;
   sondeaprs_dao = 0;
   memset((char *)sondeaprs_beacontimes,(char)0,
                sizeof(struct sondeaprs__D2 [20]));
   sondeaprs_beacontimes[0UL].beacontime = 30UL;
   sondeaprs_beacontimes[0UL].below = X2C_max_longcard;
   sondeaprs_beacontimes[1UL].beacontime = 10UL;
   sondeaprs_beacontimes[1UL].below = 1000UL;
   maxalmage = 21600UL;
   almrequest = 14400UL;
   sondeaprs_verb = 0;
   sondeaprs_verb2 = 0;
   sondeaprs_json = 0;
   sendquick = 2UL;
   sondeaprs_myalt = (-5.E+5f);
   sondeaprs_mypos.lat = 0.0f;
   sondeaprs_mypos.long0 = 0.0f;
   sondeaprs_maxsenddistance = 0UL;
   sondeaprs_expire = 0UL;
   dfmswap = 0;
   sendmhzfromsdr = 0;
   leapseconds = 18L;
   sondeaprs_axudpdests = 0;
   sondeaprs_jsondests = 0;
   sondeaprs_udpsock = -1L;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='d') sondeaprs_dao = 1;
         else if (h[1U]=='a') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&sondeaprs_rectfence.leftdown, h, 1024ul);
            if (!aprspos_posvalid(sondeaprs_rectfence.leftdown)) {
               if (!aprsstr_StrToFix(&sondeaprs_rectfence.leftdown.lat, h,
                1024ul) || (float)fabs(sondeaprs_rectfence.leftdown.lat)
                >=90.0f) {
                  Error("-a <lat> <long> or <locator> <lat> <long> or <locato\
r>", 55ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&sondeaprs_rectfence.leftdown.long0, h,
                1024ul) || (float)fabs(sondeaprs_rectfence.leftdown.long0)
                >180.0f) {
                  Error("-a <lat> <long> or <locator> <lat> <long> or <locato\
r>", 55ul);
               }
            }
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&sondeaprs_rectfence.rightup, h, 1024ul);
            if (!aprspos_posvalid(sondeaprs_rectfence.rightup)) {
               if (!aprsstr_StrToFix(&sondeaprs_rectfence.rightup.lat, h,
                1024ul) || (float)fabs(sondeaprs_rectfence.rightup.lat)
                >=90.0f) {
                  Error("-a <lat> <long> or <locator> <lat> <long> or <locato\
r>", 55ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&sondeaprs_rectfence.rightup.long0, h,
                1024ul) || (float)fabs(sondeaprs_rectfence.rightup.long0)
                >180.0f) {
                  Error("-a <lat> <long> or <locator> <lat> <long> or <locato\
r>", 55ul);
               }
            }
         }
         else if (h[1U]=='F') sondeaprs_nofilter = 1;
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &leapseconds)) {
               Error("-g [-]seconds", 14ul);
            }
         }
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
         else if (h[1U]=='E') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &sondeaprs_expire)) err = 1;
         }
         else if (h[1U]=='t') osi_NextArg(sondeaprs_commentfn, 1025ul);
         else if (h[1U]=='S') osi_NextArg(libsrtm_srtmdir, 1024ul);
         else if (h[1U]=='C') osi_NextArg(sondeaprs_csvfilename, 1025ul);
         else if (h[1U]=='j') {
            osi_NextArg(sondeaprs_csvfilename, 1025ul);
            sondeaprs_json = 1;
         }
         else if (h[1U]=='J') {
            /* send json udp */
            sondeaprs_json = 1;
            osi_NextArg(h, 1024ul);
            osic_alloc((char * *) &udest,
                sizeof(struct sondeaprs_UDPDESTS));
            if (udest==0) Error("out of memory", 14ul);
            udest->next = sondeaprs_jsondests;
            sondeaprs_jsondests = udest;
            i = 0UL;
            if (sondeaprs_GetIp(h, 1024ul, &i, &udest->ipnum,
                &udest->port)<0L) Error("-J ip:port number", 18ul);
            if (sondeaprs_udpsock<0L) {
               sondeaprs_udpsock = openudp();
               if (sondeaprs_udpsock<0L) {
                  Error("cannot open udp socket", 23ul);
               }
            }
         }
         else if (h[1U]=='m' || h[1U]=='r') {
            sondeaprs_sendmon = h[1U]!='r';
            osi_NextArg(h, 1024ul);
            osic_alloc((char * *) &udest,
                sizeof(struct sondeaprs_UDPDESTS));
            if (udest==0) Error("out of memory", 14ul);
            udest->next = sondeaprs_axudpdests;
            sondeaprs_axudpdests = udest;
            i = 0UL;
            if (sondeaprs_GetIp(h, 1024ul, &i, &udest->ipnum,
                &udest->port)<0L) Error("-m or -r ip:port number", 24ul);
            if (sondeaprs_udpsock<0L) {
               sondeaprs_udpsock = openudp();
               if (sondeaprs_udpsock<0L) {
                  Error("cannot open udp socket", 23ul);
               }
            }
         }
         else if (h[1U]=='w') {
            osi_NextArg(sondeaprs_via, 100ul);
            if ((uint8_t)sondeaprs_via[0UL]<=' ') {
               Error("-m vias like RELAY,WIDE1-1", 27ul);
            }
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            n = 0UL;
            i = 0UL;
            for (;;) {
               if (n>19UL) Error("-b timetable full", 18ul);
               if (!GetNum(h, 1024ul, ':', &i,
                &sondeaprs_beacontimes[n].beacontime)) {
                  Error("-b <s>[:<s>]...", 16ul);
               }
               if (h[i]==0) break;
               if (h[i]!=':') Error("-b <s>[:<s>]...", 16ul);
               ++i;
               ++n;
            }
         }
         else if (h[1U]=='B') {
            /* obsolete */
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i,
                &sondeaprs_beacontimes[1UL].beacontime)) {
               Error("-B <s>", 7ul);
            }
         }
         else if (h[1U]=='G') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &sondeaprs_maxsenddistance)) {
               Error("G <m>", 6ul);
            }
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            n = 1UL;
            i = 0UL;
            for (;;) {
               if (n>19UL) Error("-A altitude table full", 23ul);
               if (!GetNum(h, 1024ul, ':', &i,
                &sondeaprs_beacontimes[n].below)) {
                  Error("-A <m>[:<m>]...", 16ul);
               }
               if (h[i]==0) break;
               if (h[i]!=':') Error("-A <m>[:<m>]...", 16ul);
               ++i;
               ++n;
            }
         }
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 100ul);
            if ((uint8_t)mycall[0U]<' ') Error("-I <mycall>", 12ul);
         }
         else if (h[1U]=='s') {
            osi_NextArg(semfile, 1024ul);
            if ((uint8_t)semfile[0U]<' ') Error("-s <filename>", 14ul);
         }
         else if (h[1U]=='x') {
            osi_NextArg(rinexfile, 1024ul);
            if ((uint8_t)rinexfile[0U]<' ') Error("-x <filename>", 14ul);
         }
         else if (h[1U]=='y') {
            osi_NextArg(yumafile, 1024ul);
            if ((uint8_t)yumafile[0U]<' ') Error("-y <filename>", 14ul);
         }
         else if (h[1U]=='X') {
            osi_NextArg(encryptmsgfn, 1024ul);
            if ((uint8_t)encryptmsgfn[0U]<' ') {
               Error("-X <filename>", 14ul);
            }
         }
         else if (h[1U]=='P') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&sondeaprs_mypos, h, 1024ul);
            if (!aprspos_posvalid(sondeaprs_mypos)) {
               if (!aprsstr_StrToFix(&sondeaprs_mypos.lat, h,
                1024ul) || (float)fabs(sondeaprs_mypos.lat)>=90.0f) {
                  Error("-P <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&sondeaprs_mypos.long0, h,
                1024ul) || (float)fabs(sondeaprs_mypos.long0)>180.0f) {
                  Error("-P <lat> <long> or <locator>", 29ul);
               }
               sondeaprs_mypos.lat = sondeaprs_mypos.lat*1.7453292519943E-2f;
               sondeaprs_mypos.long0 = sondeaprs_mypos.long0*1.7453292519943E-2f;
            }
         }
         else if (h[1U]=='N') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&sondeaprs_myalt, h, 1024ul)) {
               Error("-N <altitude>", 14ul);
            }
         }
         else if (h[1U]=='v') sondeaprs_verb = 1;
         else if (h[1U]=='V') {
            sondeaprs_verb = 1;
            sondeaprs_verb2 = 1;
         }
         else if (h[1U]=='M') sendmhzfromsdr = 1;
         else if (h[1U]=='L') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            for (;;) {
               osic_alloc((char * *) &dftyp, sizeof(struct DFMTYPES));
               if (dftyp==0) Error("out of memory", 14ul);
               dftyp->n = getdfhex(h, 1024ul, &i);
               if (dftyp->n>255UL || h[i]!='=') {
                  Error("-L <hex>=<typname>", 19ul);
               }
               dftyp->t[0U] = 0;
               for (;;) {
                  ++i;
                  if ((i>1023UL || h[i]==0) || h[i]==',') break;
                  if ((uint8_t)h[i]<' ' || (uint8_t)h[i]>='~') {
                     Error("-L <hex>=<typname>", 19ul);
                  }
                  aprsstr_Append(dftyp->t, 9ul, (char *) &h[i], 1u/1u);
               }
               dftyp->next = dftypes;
               dftypes = dftyp;
               if (h[i]!=',') break;
               ++i;
            }
         }
         else {
            /*    ELSIF h[1]="J" THEN dfmswap:=TRUE; */
            if (h[1U]=='h') {
               osi_WrStr("sondemod 1.37b", 15ul);
               osi_WrStrLn(" multichannel decoder RS92, RS41, SRS-C34/50, DFM\
, M10, iMET Radiosondes", 73ul);
               osi_WrStrLn(" -A <meter>[:<meter>]   at lower altitude use -b \
beacon time (meter) -A 3000:1000:200", 86ul);
               osi_WrStrLn("                  if SRTM/EGM-data available, Ove\
rground will be used", 70ul);
               osi_WrStrLn(" -a <lat> <long> <lat> <long> or <locator> <locat\
or> send beacons from inside leftdown", 87ul);
               osi_WrStrLn("                  to rightup, swap corners to sen\
d only outside the rectangle", 78ul);
               osi_WrStrLn(" -B <seconds>   obsolete, use -b <seconds>:<secon\
ds>", 53ul);
               osi_WrStrLn(" -b <seconds>[:<seconds>]...   minimum send inter\
vall or 0 for never send -b 30:20:6:2", 87ul);
               osi_WrStrLn("                  first for high altitude, next b\
elow (descending) altitudes as given in -A", 92ul);
               osi_WrStrLn(" -C <filename>  write decoded data in csv-format \
to this file", 62ul);
               osi_WrStrLn(" -d             dao extension for 20cm APRS resol\
ution instead of 18m", 70ul);
               osi_WrStrLn(" -E <seconds>   stop sending if more difference g\
ps-time to computer-clock (0=off)", 83ul);
               osi_WrStrLn("                  (-E 4) use to keep Tracks clean\
 if system time is set to UTC", 79ul);
               osi_WrStrLn(" -F             trackfilter off, DO NOT USE THIS \
SENDING TO THE WORLD!", 71ul);
               osi_WrStrLn(" -G <km>        send only if Gound-distance to So\
nde not more, 0=off (needs -P) (-G 15)", 88ul);
               osi_WrStrLn(" -g <seconds>   GPS to utc correction (leapsecond\
s) (if not from sonde) (18)", 77ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -I <mycall>    Sender of Object Callsign -I OE0A\
AA if not sent by \'sondeudp\'", 78ul);
               osi_WrStrLn(" -J <x.x.x.x:destport>  send decoded data in (ld)\
json-format via UDP", 69ul);
               osi_WrStrLn("                  may be repeated to send to more\
 destinations", 63ul);
               osi_WrStrLn(" -j <filename>  write decoded data in (ld)json-fo\
rmat to this file or (unbreakable) pipe", 89ul);
               osi_WrStrLn(" -L <hex>=<typname>[,<hex>=<typname>]...", 41ul);
               osi_WrStrLn("                 IF there is a dependency, assign\
 DFM-Subtype to highest first 4 bit in", 88ul);
               osi_WrStrLn("                 serial number frame (in hex), FF\
=wildcard if nothing else fits ", 81ul);
               osi_WrStrLn("                 eg. -L 6=DFM06,7=PS-15,A=DFM09,B\
=DFM17,C=DFM09P,D=DFM17,FF=DFMx", 81ul);
               osi_WrStrLn(" -M             Send \"MHz\" in APRS (if not rece\
ived in Data) from SDR-parameter +afc", 85ul);
               osi_WrStrLn("                  do only with calibrated SDR, ac\
cept wrong data from alias receptions", 87ul);
               osi_WrStrLn(" -m             same as -r but send as text inste\
ad of axudp", 61ul);
               osi_WrStrLn(" -N <meter>     my altitude over NN for Distance/\
Elevation to sonde output", 75ul);
               osi_WrStrLn(" -o <UDPport>   receive demodulated data via UDP \
port from \'sondeudp -u ...\'", 77ul);
               osi_WrStrLn(" -P <lat> <long> or <locator>  my Position for Di\
stance/Azimuth/Elevation", 74ul);
               osi_WrStrLn("                 eg. -P JQ50AB12CD or -P 70.0506 \
10.0092", 57ul);
               osi_WrStrLn(" -p <num>       0 send if weather data ready, 1 i\
f MHz known, 2 send immediatly (2)", 84ul);
               osi_WrStrLn(" -R <minutes>   request new rinex almanach after \
minutes if receiving gps (-R 240)", 83ul);
               osi_WrStrLn("                use somewhat like \'getalmd\'-scr\
ipt to download", 63ul);
               osi_WrStrLn(" -r <ip>:<port> send AXUDP -r 127.0.0.1:9001 use \
udpgate4 or aprsmap as receiver", 81ul);
               osi_WrStrLn("                  udp stream maybe duplicated wit\
h udpbox to more destinations", 79ul);
               osi_WrStrLn("                  may be repeated to send to more\
 destinations", 63ul);
               osi_WrStrLn(" -S <pathname>  directory with SRTM(1/3/30) Data \
and WW15MGH.DAC file (egm96-Geoid)", 84ul);
               osi_WrStrLn("                  for Overground Calculation belo\
w -A <altitude>", 65ul);
               osi_WrStrLn("                  example with: -S /home/pi",
                44ul);
               osi_WrStrLn("                  /home/pi/WW15MGH.DAC         (2\
076480Byte, covers whole World)", 81ul);
               osi_WrStrLn("                  /home/pi/srtm1/N48E014.hgt  (25\
934402Byte, not SRTM3!)", 73ul);
               osi_WrStrLn("                  /home/pi/srtm1/N48E015.hgt",
                45ul);
               osi_WrStrLn(" -s <filename>  gps almanach sem format (DO NOT U\
SE, not exact)", 64ul);
               osi_WrStrLn(" -T <minutes>   stop sending data after almanach \
age (-T 360)", 62ul);
               osi_WrStrLn(" -t <filename>  append comment lines from this fi\
le at start of line eg \"%f%d%v text...\"", 89ul);
               osi_WrStrLn("                  %A Azimuth from sonde-rx, (-P n\
eeded too)", 60ul);
               osi_WrStrLn("                  %d rssi if received with sdrtst\
 -e", 53ul);
               osi_WrStrLn("                  %D Distance to sonde-rx, (-P -S\
 needed too with EGM96)", 73ul);
               osi_WrStrLn("                  %E Elevation to sonde, (-P -S n\
eeded too with EGM96)", 71ul);
               osi_WrStrLn("                  %F same as \"f\" but send even \
if MHz got from sonde data", 74ul);
               osi_WrStrLn("                  %f sdr freq+AFC from sdrtst wit\
h -e and not (yet) got MHz from sonde", 87ul);
               osi_WrStrLn("                  %l label given in sondeudp -L e\
g. \"omni\" \"west\" \"rx1\"", 72ul);
               osi_WrStrLn("                  %n frame number if available",
                47ul);
               osi_WrStrLn("                  %r hdil if available, gps horiz\
ontal noise in meter", 70ul);
               osi_WrStrLn("                  %s gps sat count if available",
                 48ul);
               osi_WrStrLn("                  %t tx power dBm", 34ul);
               osi_WrStrLn("                  %u sonde uptime if available",
                47ul);
               osi_WrStrLn("                  %v sondemod version", 38ul);
               osi_WrStrLn("                  # or empty line(s) for comment-\
free beacons", 62ul);
               osi_WrStrLn(" -V             more verbous", 29ul);
               osi_WrStrLn(" -v             verbous", 24ul);
               osi_WrStrLn(" -X <filename>  if seen encrypted frame, write a \
file with sonde number", 72ul);
               osi_WrStrLn(" -x <filename>  gps almanach rinexnavigation form\
at (prefered)", 63ul);
               osi_WrStrLn(" -y <filename>  gps almanach yuma format (DO NOT \
USE, not exact)", 65ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("example: sondemod -o 18000 -d -A 1500 -b 30:15 -I\
 OE0AAA -r 127.0.0.1:9001", 75ul);
               osi_WrStrLn("example with srtm and WW15MGH.DAC and faster beac\
on near ground:", 65ul);
               osi_WrStrLn("sondemod -o 18000 -I OE0AAA -r 127.0.0.1:18100 -b\
 30:20:6:2 -A 2000:1000:200 -t sondecom.txt -x /tmp/alm.txt -T 360 -R 240 -d \
-p 2 -S osm -P 48.000 13.000 -N 365 -E 7 -M -L 6=DFM06,7=PS-15,A=DFM09,B=DFM1\
7,C=DFM09P,D=DFM17,FF=DFMx -v", 233ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("python3 json import example: (-J 127.0.0.1:18600",
                 49ul);
               osi_WrStrLn("----------------------------", 29ul);
               osi_WrStrLn("import json, socket", 20ul);
               osi_WrStrLn("PORT=18600", 11ul);
               osi_WrStrLn("IP=(\"0.0.0.0\",PORT)", 20ul);
               osi_WrStrLn("sock=socket.socket(socket.AF_INET,socket.SOCK_DGR\
AM)", 53ul);
               osi_WrStrLn("sock.bind(IP)", 14ul);
               osi_WrStrLn("while True:", 12ul);
               osi_WrStrLn("  data, addr=sock.recvfrom(1500)", 33ul);
               osi_WrStrLn("  try:", 7ul);
               osi_WrStrLn("    obj = json.loads(data.decode())", 36ul);
               osi_WrStrLn("    print(obj)", 15ul);
               osi_WrStrLn("    print(\"lat:\",obj[\"lat\"],\"long:\",obj[\"l\
ong\"])", 49ul);
               osi_WrStrLn("  except: print(\"json decode error\")", 37ul);
               osi_WrStrLn("", 1ul);
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
   if ((sondeaprs_maxsenddistance>0UL && sondeaprs_mypos.lat==0.0f)
                && sondeaprs_mypos.long0==0.0f) {
      osi_WrStrLn("Warning: -G needs Your -P <Position>", 37ul);
   }
} /* end Parms() */


static void wrdate(uint32_t t)
{
   char s[31];
   aprsstr_DateToStr(t, s, 31ul);
   osi_WrStr(s, 31ul);
} /* end wrdate() */


static void wrsdr(void)
{
   struct sondeaprs_SDRBLOCK * anonym;
   { /* with */
      struct sondeaprs_SDRBLOCK * anonym = &sdrblock;
      if (anonym->valid) {
         if (anonym->freq) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)anonym->freq*0.00001f, 3L, 1UL);
         }
         if (anonym->maxafc) {
            osi_WrStr("(", 2ul);
            if (anonym->afc>=0L) osi_WrStr("+", 2ul);
            osic_WrINT32((uint32_t)anonym->afc, 1UL);
            osi_WrStr("/", 2ul);
            osic_WrINT32(anonym->maxafc, 1UL);
            osi_WrStr(")", 2ul);
         }
         if (anonym->db) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)anonym->db*0.1f, 1L, 1UL);
            osi_WrStr("dB", 3ul);
         }
         if (anonym->name[0UL]) {
            osi_WrStr(" ", 2ul);
            osi_WrStr(anonym->name, 4ul);
         }
      }
   }
} /* end wrsdr() */

static uint32_t sondemod_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};

static uint32_t _cnst[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,181UL,
                212UL,243UL,273UL,304UL,334UL};

static uint32_t unixdate(uint32_t yyyy, uint32_t mm, uint32_t dd)
/* make unix time */
{
   uint32_t tt;
   if (yyyy>2000UL && yyyy<2100UL) {
      tt = (yyyy-1970UL)*365UL+(yyyy-1969UL)/4UL; /* days since 1970 */
      if (mm<=12UL) {
         tt += _cnst[mm];
         if ((yyyy&3UL)==0UL && mm>2UL) ++tt;
      }
      tt = ((tt+dd)-1UL)*86400UL;
   }
   else tt = 0UL;
   return tt;
} /* end unixdate() */

/*  WrStr(" "); WrHex(n DIV 01000000H MOD 256, 2);
                WrHex(n DIV 010000H MOD 256, 2);
                WrHex(n DIV 0100H MOD 256, 2); WrHex(n MOD 256, 2);  */
#define sondemod_Z 48


static void degtostr(float d, char lat, char form,
                char s[], uint32_t s_len)
{
   uint32_t i;
   uint32_t n;
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
      i = (uint32_t)!lat;
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
      i = (uint32_t)!lat;
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
      i = (uint32_t)!lat;
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


static void storexdata(struct sondeaprs_XDATA * x, char b[],
                uint32_t b_len, uint32_t from, uint32_t l)
{
   uint32_t i;
   X2C_PCOPY((void **)&b,b_len);
   if (x->cnt<=9UL) {
      i = 0UL;
      x->xdata[x->cnt].len = l;
      while (i<l && i<=255UL) {
         x->xdata[x->cnt].frame[i] = b[from+i];
         ++i;
      }
      ++x->cnt;
   }
   X2C_PFREE(b);
} /* end storexdata() */


static void initcontext(struct CONTEXTR9 * cont)
{
   memset((char *)cont,(char)0,sizeof(struct CONTEXTR9));
   cont->lastlat = 8.4214719496019E-1;
   cont->laslong = 2.2755602787502E-1;
} /* end initcontext() */

static float sondemod_P[13] = {1000.0f,150.0f,100.0f,70.0f,60.0f,50.0f,
                40.0f,30.0f,20.0f,15.0f,10.0f,8.0f,0.0f};

static float sondemod_C[13] = {1.0f,1.0f,1.01f,1.022f,1.025f,1.035f,
                1.047f,1.065f,1.092f,1.12f,1.17f,1.206f,1.3f};

static float _cnst1[13] = {1.0f,1.0f,1.01f,1.022f,1.025f,1.035f,1.047f,
                1.065f,1.092f,1.12f,1.17f,1.206f,1.3f};
static float _cnst0[13] = {1000.0f,150.0f,100.0f,70.0f,60.0f,50.0f,40.0f,
                30.0f,20.0f,15.0f,10.0f,8.0f,0.0f};

static double getOzoneCorr(double p)
/* From from ftp://ftp.cpc.ncep.noaa.gov/ndacc/meta/sonde/cv_payerne_snd.txt */
{
   uint32_t i;
   i = 12UL;
   while (i>0UL && (double)_cnst0[i]<p) --i;
   return (double)_cnst1[i];
} /* end getOzoneCorr() */


static float calcOzone(double uA, double temp,
                double airpres)
{
   return (float)(4.307E-4*uA*(temp+273.15)*28.57*getOzoneCorr(airpres));
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


static void dogps(const char sf[], uint32_t sf_len,
                struct CONTEXTR9 * cont, uint32_t * timems,
                uint32_t * gpstime)
{
   uint32_t i;
   gpspos_SATS sats;
   int32_t res;
   int32_t d1;
   int32_t d;
   char h[100];
   struct CONTEXTR9 * anonym;
   cont->lat = 0.0;
   cont->long0 = 0.0;
   cont->heig = 0.0;
   cont->speed = 0.0;
   cont->dir = 0.0;
   /*WrStrLn("gps:"); */
   /*FOR i:=0 TO 121 DO WrHex(ORD(sf[i]), 3) END; WrStrLn(""); */
   *timems = (uint32_t)(uint8_t)sf[0UL]+(uint32_t)(uint8_t)
                sf[1UL]*256UL+(uint32_t)(uint8_t)
                sf[2UL]*65536UL+(uint32_t)(uint8_t)sf[3UL]*16777216UL;
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
      sats[i*3UL].prn = (uint32_t)(uint8_t)sf[i*2UL+6UL]&31UL;
      sats[i*3UL+1UL].prn = (uint32_t)(uint8_t)
                sf[i*2UL+6UL]/32UL+(uint32_t)(uint8_t)
                sf[i*2UL+7UL]*8UL&31UL;
      sats[i*3UL+2UL].prn = (uint32_t)(uint8_t)sf[i*2UL+7UL]/4UL&31UL;
   } /* end for */
   if (sondeaprs_verb2) {
      osi_WrStr("prn:", 5ul);
      for (i = 0UL; i<=11UL; i++) {
         osic_WrINT32(sats[i].prn, 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStr("sig: ", 6ul);
      for (i = 0UL; i<=11UL; i++) {
         osi_WrHex((uint32_t)(uint8_t)sf[i+14UL], 3UL);
      } /* end for */
      osi_WrStrLn("", 1ul);
      osi_WrStrLn("rang:", 6ul);
   }
   for (i = 0UL; i<=11UL; i++) {
      if (sats[i].prn>0UL) {
         sats[i].rang = (int32_t)((uint32_t)(uint8_t)
                sf[i*8UL+26UL]+(uint32_t)(uint8_t)
                sf[i*8UL+27UL]*256UL+(uint32_t)(uint8_t)
                sf[i*8UL+28UL]*65536UL+(uint32_t)(uint8_t)
                sf[i*8UL+29UL]*16777216UL);
         sats[i].rang1 = (int32_t)((uint32_t)(uint8_t)
                sf[i*8UL+30UL]+(uint32_t)(uint8_t)
                sf[i*8UL+31UL]*256UL+(uint32_t)(uint8_t)
                sf[i*8UL+32UL]*65536UL);
         sats[i].rang1 = sats[i].rang1&8388607L;
         sats[i].rang3 = (int32_t)(signed char)(uint8_t)sf[i*8UL+33UL];
         d = sats[i].rang-lastsat[i].rang;
         d1 = sats[i].rang1-lastsat[i].rang1;
         if (sondeaprs_verb2) {
            osic_WrINT32(sats[i].prn, 3UL);
            osic_WrINT32((uint32_t)sats[i].rang, 12UL);
            osic_WrINT32((uint32_t)sats[i].rang1, 12UL);
            osic_WrINT32((uint32_t)sats[i].rang3, 5UL);
            osic_WrINT32((uint32_t)d, 12UL);
            osic_WrINT32((uint32_t)(d-lastsat[i].lastd), 12UL);
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


static void decodecalib(const char cd[], uint32_t cd_len)
{
   uint32_t n;
   uint32_t i;
   uint32_t cr;
   uint32_t tmp;
   memset((char *)coeff,(char)0,sizeof(float [256]));
   i = 64UL;
   for (tmp = 88UL;;) {
      n = (uint32_t)(uint8_t)cd[i];
      cr = (uint32_t)(uint8_t)cd[i+1UL]+(uint32_t)(uint8_t)
                cd[i+2UL]*256UL+(uint32_t)(uint8_t)
                cd[i+3UL]*65536UL+(uint32_t)(uint8_t)
                cd[i+4UL]*16777216UL;
      coeff[n] = SaveReal(cr);
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


static float extr(uint32_t hi, uint32_t lo, uint32_t u,
                uint32_t idx)
{
   float f;
   float x;
   float v;
   uint32_t i;
   uint32_t tmp;
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


static void domes(const char md[], uint32_t md_len, double * hp,
                double * hyg, double * temp)
{
   uint32_t i;
   int32_t m[8];
   float d5;
   float d4;
   float d3;
   float p;
   float hr2;
   float hr1;
   for (i = 0UL; i<=7UL; i++) {
      m[i] = (int32_t)((uint32_t)(uint8_t)md[i*3UL]+(uint32_t)
                (uint8_t)md[i*3UL+1UL]*256UL+(uint32_t)(uint8_t)
                md[i*3UL+2UL]*65536UL);
   } /* end for */
   /* hygro 1 */
   /*  IF verb THEN WrStr(" <h> ") END; */
   hr1 = extr((uint32_t)m[3U], (uint32_t)m[7U], (uint32_t)m[1U], 40UL);
   hr2 = extr((uint32_t)m[3U], (uint32_t)m[7U], (uint32_t)m[2U], 50UL);
   if (hr2>hr1) hr1 = hr2;
   if (hr1<2.0f) hr1 = 0.0f;
   else if (hr1>100.0f) hr1 = 100.0f;
   *hyg = (double)hr1;
   /* temp */
   *temp = (double)extr((uint32_t)m[3U], (uint32_t)m[7U],
                (uint32_t)m[0U], 30UL);
   /* baro */
   d3 = (float)(m[3U]-m[7U]);
   d4 = (float)(m[4U]-m[7U]);
   d5 = (float)(m[5U]-m[7U]);
   p = extr((uint32_t)m[3U], (uint32_t)m[7U], (uint32_t)m[5U],
                10UL)+coeff[60U]*extr((uint32_t)m[3U], (uint32_t)m[7U],
                (uint32_t)m[4U], 20UL)+X2C_DIVR(coeff[61U]*coeff[20U]*d3,
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
            osic_WrINT32((uint32_t)m[i], 7UL);
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


static void doozon(const char s[], uint32_t s_len,
                const double airpres, double * otemp,
                double * ozon)
{
   *otemp = (double)(float)((uint32_t)(uint8_t)
                s[4UL]+(uint32_t)(uint8_t)s[5UL]*256UL);
   *ozon = (double)(float)((uint32_t)(uint8_t)
                s[2UL]+(uint32_t)(uint8_t)s[3UL]*256UL);
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


static void calibfn(char obj[], uint32_t obj_len, char fn[],
                uint32_t fn_len)
{
   uint32_t i;
   X2C_PCOPY((void **)&obj,obj_len);
   aprsstr_Assign(fn, fn_len, obj, obj_len);
   i = 0UL;
   while (i<=fn_len-1 && fn[i]) {
      if (((uint8_t)fn[i]<'0' || (uint8_t)fn[i]>'9') && ((uint8_t)
                fn[i]<'A' || (uint8_t)fn[i]>'Z')) {
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
                uint32_t objname_len)
{
   char fn[1024];
   int32_t fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   initcontext(cont);
   calibfn(objname0, objname_len, fn, 1024ul);
   fd = osi_OpenRead(fn, 1024ul);
   if (fd>=0L) {
      if (osi_RdBin(fd, (char *)cont, sizeof(struct CONTEXTR9)/1u,
                sizeof(struct CONTEXTR9))!=(int32_t)
                sizeof(struct CONTEXTR9)) initcontext(cont);
      osic_Close(fd);
   }
   X2C_PFREE(objname0);
} /* end readcontext() */


static void wrcontext(struct CONTEXTR9 * cont, char objname0[],
                uint32_t objname_len)
{
   char fn[1024];
   int32_t fd;
   X2C_PCOPY((void **)&objname0,objname_len);
   calibfn(objname0, objname_len, fn, 1024ul);
   if (fn[0U]) {
      fd = osi_OpenWrite(fn, 1024ul);
      if (fd>=0L) {
         osi_WrBin(fd, (char *)cont, sizeof(struct CONTEXTR9)/1u,
                sizeof(struct CONTEXTR9));
         osic_Close(fd);
      }
      else {
         osic_WrLn();
         osi_WrStr("can not write ", 15ul);
         osi_WrStr(fn, 1024ul);
         osi_WrStrLn(" calibration file", 18ul);
      }
   }
   X2C_PFREE(objname0);
} /* end wrcontext() */


static void docalib(const char sf[], uint32_t sf_len,
                char objname0[], uint32_t objname_len,
                struct CONTEXTR9 * cont, float * mhz0,
                uint32_t * frameno)
{
   uint32_t idx;
   uint32_t j;
   uint32_t i;
   char new0;
   *mhz0 = 0.0f;
   new0 = 0;
   i = 0UL;
   for (j = 2UL; j<=11UL; j++) {
      /* object name */
      /*    IF (1 IN cont.calibok) & (sf[j]<>cont.calibdata[j+20])
                THEN cont.calibok:=SET32{} END; */
      if (i<=objname_len-1 && (uint8_t)sf[j]>' ') {
         if (objname0[i]!=sf[j]) new0 = 1;
         objname0[i] = sf[j];
         ++i;
      }
   } /* end for */
   if (i<=objname_len-1) objname0[i] = 0;
   if (new0) readcontext(cont, objname0, objname_len);
   *frameno = (uint32_t)(uint8_t)sf[0UL]+(uint32_t)(uint8_t)
                sf[1UL]*256UL;
   if (sondeaprs_verb) {
      if (new0) osi_WrStr("new ", 5ul);
      osic_WrINT32(*frameno, 1UL); /* frame no */
      osi_WrStr(" ", 2ul);
      osi_WrStr(objname0, objname_len); /*WrStr(" bat:");
                WrHex(ORD(sf[12]), 2);*/
   }
   idx = (uint32_t)(uint8_t)sf[15UL];
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
         *mhz0 = (float)(400000UL+((uint32_t)(uint8_t)
                cont->calibdata[2U]+(uint32_t)(uint8_t)
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


static uint32_t calperc91(uint32_t cs)
{
   uint32_t n;
   uint32_t i;
   n = 0UL;
   for (i = 0UL; i<=31UL; i++) {
      if (X2C_IN(i,32,cs)) ++n;
   } /* end for */
   return (n*100UL)/32UL;
} /* end calperc91() */


static void WrRinexfn(uint32_t t)
{
   char fn[31];
   uint32_t y;
   uint32_t d;
   int32_t f;
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


static void getcall(const char b[], uint32_t b_len, char call[],
                uint32_t call_len)
{
   uint32_t c;
   uint32_t n;
   uint32_t i;
   char tmp;
   call[0UL] = 0;
   n = (uint32_t)(uint8_t)b[0UL]*16777216UL+(uint32_t)(uint8_t)
                b[1UL]*65536UL+(uint32_t)(uint8_t)
                b[2UL]*256UL+(uint32_t)(uint8_t)b[3UL];
   if (n>0UL && (uint32_t)(uint8_t)b[4UL]<=15UL) {
      for (i = 5UL;; i--) {
         c = n%37UL;
         if (c==0UL) call[i] = 0;
         else if (c<27UL) call[i] = (char)((c+65UL)-1UL);
         else call[i] = (char)((c+48UL)-27UL);
         n = n/37UL;
         if (i==0UL) break;
      } /* end for */
      call[6UL] = 0;
      c = (uint32_t)(uint8_t)b[4UL];
      if (c>0UL) {
         aprsstr_Append(call, call_len, "-", 2ul);
         if (c>=10UL) {
            aprsstr_Append(call, call_len, "1", 2ul);
            c = c%10UL;
         }
         aprsstr_Append(call, call_len,
                (char *)(tmp = (char)(c+48UL),&tmp), 1u/1u);
      }
   }
/*WrStr("usercall:");WrStrLn(call); */
} /* end getcall() */


static void decodeframe(uint8_t m, uint32_t ip, uint32_t fromport)
{
   uint32_t gpstime;
   uint32_t frameno;
   uint32_t len;
   uint32_t p;
   uint32_t j;
   uint32_t i;
   uint32_t almanachage;
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
   uint32_t tmp;
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
   xdatablock.cnt = 0UL;
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
            osi_WrHex((uint32_t)(uint8_t)typ, 4UL);
            crdone = 0;
         }
         break;
      }
      ++p;
      len = (uint32_t)(uint8_t)b[p]*2UL+2UL; /* +crc */
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
      while (j<len) {
         sf[j] = b[p];
         ++p;
         ++j;
         if (p>240UL) {
            osi_WrStr("eof", 4ul); /* error */
            crdone = 0;
            goto loop_exit;
         }
      }
      crdone = 0;
      if (!crcrs(sf, 256ul, 0L, (int32_t)len)) {
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
               if (sondeaprs_verb) {
                  wrsdr();
                  osi_WrStrLn("", 1ul);
               }
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
               osi_WrStr(" ", 2ul);
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
                        osi_WrStrLn("almanach read error", 20ul);
                     }
                     if (rinexfile[0U]
                && (almage==0UL || gpstime>almage && gpstime-almage>almrequest)
                ) {
                        /* request a new almanach */
                        if (gpstime==0UL) WrRinexfn(systime);
                        else WrRinexfn(gpstime);
                     }
                     crdone = 0;
                  }
                  if (gpstime>0UL && gpstime>=almage) {
                     almanachage = gpstime-almage;
                  }
                  else almanachage = 0UL;
                  if (almage+maxalmage>gpstime) anonym0->posok = 1;
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
                     osi_WrHex((uint32_t)(uint8_t)sf[j], 3UL);
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
               storexdata(&xdatablock, sf, 256ul, 0UL, len-2UL);
            }
         }
         else if (sondeaprs_verb2) {
            tmp = len-1UL;
            j = 0UL;
            if (j<=tmp) for (;; j++) {
               osi_WrHex((uint32_t)(uint8_t)sf[j], 3UL);
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
                anonym1->speed, anonym1->dir, anonym1->climb, 0.0,
                anonym1->hyg, anonym1->temp, anonym1->ozon,
                anonym1->ozontemp, 0.0, 0.0, (double)mhz,
                (double)anonym1->hrmsc, (double)anonym1->vrmsc,
                (uint32_t)((int32_t)gpstime-leapseconds), frameno,
                objname, 9ul, almanachage, anonym1->goodsats, 0UL, 0.0,
                usercall, 11ul, calperc91(anonym1->calibok), anonym1->hp,
                sondeaprs_nofilter, 0, 0L, "RS92", 5ul, "", 1ul, 0, sdrblock,
                 leapseconds, xdatablock);
         anonym1->framesent = 1;
      }
      crdone = 1;
   }
   if (sondeaprs_verb) {
      if (!crdone) osi_WrStrLn("", 1ul);
      osi_WrStrLn("------------", 13ul);
   }
} /* end decodeframe() */

/*------------------------------ C34 C50 */

static double latlong(uint32_t val, char c50)
{
   double hf;
   double hr;
   hr = (double)(float)(val%0x080000000UL);
   if (c50) hr = X2C_DIVL(hr,1.E+7);
   else hr = X2C_DIVL(hr,1.E+6);
   hf = (double)(float)(uint32_t)X2C_TRUNCC(hr,0UL,
                X2C_max_longcard);
   hr = hf+X2C_DIVL(hr-hf,0.6);
   if (val>=0x080000000UL) hr = -hr;
   return hr;
} /* end latlong() */


static double dist(double a, double b)
{
   double d;
   d = a-b;
   if (d>3.1415926535898) d = d-6.2831853071796;
   else if (d<(-3.1415926535898)) d = d+6.2831853071796;
   return d;
} /* end dist() */

#define sondemod_MAXEXTEND 3.0
/* limit extrapolation range */

#define sondemod_MAXTIMESPAN 10

#define sondemod_MAXRANGE 4.7123889803847E-4
/* max jump in rad */


static double extrapolate(double yold, double y,
                uint32_t told, uint32_t t, uint32_t systime0,
                char * good)
{
   double maxex;
   double maxr;
   double dy;
   double k;
   uint32_t maxt;
   maxr = 4.7123889803847E-4;
   maxt = 10UL;
   maxex = 3.0;
   if (sondeaprs_nofilter) {
      maxr = 1.8849555921539E-3;
      maxt = 40UL;
      maxex = 12.0;
   }
   *good = 1;
   if (t>=systime0) return y;
   /* point is just in time */
   if (told<t) {
      k = (double)(X2C_DIVR((float)(systime0-told),
                (float)(t-told)));
      if (k>maxex || told+maxt<systime0) *good = 0;
      dy = y-yold;
      if (fabs(dy)>maxr) *good = 0;
      return yold+dy*k;
   }
   *good = 0;
   return y;
} /* end extrapolate() */

#define sondemod_MINTV 8
/* min seconds for speed out of positions */

#define sondemod_VLIM 2.6164311878598E-5
/* max speed */


static void decodec34(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   OBJNAME nam;
   char cb[10];
   char s[1001];
   char tstr[51];
   CALLSSID usercall;
   uint32_t val;
   uint32_t sum2;
   uint32_t sum1;
   uint32_t j;
   uint32_t i;
   double ve;
   double exlat;
   double exlon;
   double hr;
   pCONTEXTC34 pc0;
   pCONTEXTC34 pc1;
   pCONTEXTC34 pc;
   double shum;
   double stemp;
   char c50;
   char latok;
   char lonok;
   char posok;
   struct CONTEXTC34 * anonym;
   struct CONTEXTC34 * anonym0;
   struct CONTEXTC34 * anonym1;
   struct CONTEXTC34 * anonym2;
   if (rxb[0UL]!='S' || rxb[1UL]!='C') return;
   /* no srsc34 frame */
   c50 = rxb[2UL]=='5'; /* is a sc50 */
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
      sum1 += (uint32_t)(uint8_t)cb[i];
      sum2 -= (uint32_t)(uint8_t)cb[i]*(5UL-i);
   } /* end for */
   sum1 = sum1&255UL;
   sum2 = sum2&255UL;
   if (sum1!=(uint32_t)(uint8_t)cb[5U] || sum2!=(uint32_t)(uint8_t)
                cb[6U]) return;
   /* checksum error */
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
         osic_free((char * *) &pc, sizeof(struct CONTEXTC34));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTC34));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTC34));
      pc->next = pcontextc;
      pcontextc = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   pc->tused = systime;
   val = (uint32_t)(uint8_t)cb[4U]+(uint32_t)(uint8_t)
                cb[3U]*256UL+(uint32_t)(uint8_t)
                cb[2U]*65536UL+(uint32_t)(uint8_t)cb[1U]*16777216UL;
   hr = (double)SaveReal(val);
   posok = 0;
   if (c50) {
      switch ((unsigned)cb[0U]) {
      case '\003':
         if (hr<99.9 && hr>(-99.9)) {
            if (sondeaprs_verb) {
               osi_WrStr("tair ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("oC", 3ul);
            }
            pc->temp = hr;
            pc->ttemp = systime;
         }
         break;
      case '\020':
         if (hr<=100.0 && hr>0.1) {
            if (sondeaprs_verb) {
               osi_WrStr("hum  ", 6ul);
               osic_WrFixed((float)hr, 1L, 0UL);
               osi_WrStr("%", 2ul);
            }
            pc->hum = hr;
            pc->thum = systime;
         }
         break;
      case '\024':
         pc->gpsdate = unixdate(2000UL+val%100UL, (val/100UL)%100UL,
                (val/10000UL)%100UL);
         if (sondeaprs_verb) {
            osi_WrStr("date", 5ul);
            aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL, s,
                1001ul);
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
         hr = latlong(val, c50);
         if (hr<89.9 && hr>(-89.9)) {
            if (sondeaprs_verb) {
               osi_WrStr("lat  ", 6ul);
               osic_WrFixed((float)hr, 5L, 0UL);
            }
            if (pc->tlat!=systime) {
               { /* with */
                  struct CONTEXTC34 * anonym = pc;
                  anonym->lat1 = anonym->lat;
                  anonym->tlat1 = anonym->tlat;
                  anonym->lat = hr*1.7453292519943E-2;
                  anonym->tlat = systime;
                  if (anonym->tlat<anonym->tlatv1) {
                     anonym->tlatv1 = anonym->tlat;
                /* repair back jumped time */
                  }
                  if (anonym->tlat>anonym->tlatv1+8UL) {
                     /* south-north speed */
                     ve = X2C_DIVL(dist(anonym->lat, anonym->latv1),
                (double)(anonym->tlat-anonym->tlatv1));
                     /*WrStr(" ");WrFixed(ve*(EARTH*1000), 1, 9);
                WrStr("VTn"); */
                     if (fabs(ve)<=2.6164311878598E-5) {
                        anonym->vlat = anonym->vlat+(ve-anonym->vlat)*0.5;
                     }
                     anonym->latv1 = anonym->lat;
                     anonym->tlatv1 = anonym->tlat;
                  }
               }
               posok = 1;
            }
         }
         break;
      case '\027':
         hr = latlong(val, c50);
         if (hr<180.0 && hr>(-180.0)) {
            if (sondeaprs_verb) {
               osi_WrStr("long ", 6ul);
               osic_WrFixed((float)hr, 5L, 0UL);
            }
            if (pc->tlon!=systime) {
               { /* with */
                  struct CONTEXTC34 * anonym0 = pc;
                  anonym0->lon1 = anonym0->lon;
                /* save 2 values for extrapolating */
                  anonym0->tlon1 = anonym0->tlon;
                  anonym0->lon = hr*1.7453292519943E-2;
                  anonym0->tlon = systime;
                  if (anonym0->tlon<anonym0->tlonv1) {
                     anonym0->tlonv1 = anonym0->tlon;
                /* repair back jumped time */
                  }
                  if (anonym0->tlat>0UL && anonym0->tlon>anonym0->tlonv1+8UL)
                 {
                     /* east-west speed */
                     ve = X2C_DIVL(dist(anonym0->lon,
                anonym0->lonv1)*cos(anonym0->lat),
                (double)(anonym0->tlon-anonym0->tlonv1));
                     /*WrStr(" ");WrFixed(ve*(EARTH*1000), 1, 9);
                WrStr("VTe"); */
                     if (fabs(ve)<=2.6164311878598E-5) {
                        anonym0->vlon = anonym0->vlon+(ve-anonym0->vlon)*0.5;
                     }
                     anonym0->lonv1 = anonym0->lon;
                     anonym0->tlonv1 = anonym0->tlon;
                  }
               }
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
      default:;
         if (sondeaprs_verb) {
            osi_WrHex((uint32_t)(uint8_t)cb[0U], 0UL);
            osi_WrStr(" ", 2ul);
            osi_WrHex((uint32_t)(uint8_t)cb[1U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[2U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[3U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[4U], 0UL);
            osic_WrFixed((float)hr, 2L, 10UL);
         }
         break;
      } /* end switch */
      { /* with */
         struct CONTEXTC34 * anonym1 = pc;
         anonym1->tspeed = systime;
         anonym1->tdir = systime;
         anonym1->speed = sqrt(anonym1->vlon*anonym1->vlon+anonym1->vlat*anonym1->vlat)
                *6.37E+6; /* speed out of moved distance km/h */
         anonym1->dir = atang2(anonym1->vlat, anonym1->vlon);
         if (anonym1->dir<0.0) anonym1->dir = anonym1->dir+6.2831853071796;
         anonym1->dir = anonym1->dir*5.7295779513082E+1;
      }
   }
   else {
      /*WrStrLn(""); WrStr("vlat,vlon spd, dir:");
                WrFixed(vlat*(EARTH*3600), 1,7); */
      /*WrFixed(vlon*(EARTH*3600), 1,7); WrFixed(speed*3.6, 1,8);
                WrFixed(dir, 1,9) ; WrFixed(alt, 1,9); */
      /* SC34 */
      switch ((unsigned)cb[0U]) {
      case '\003':
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
      case '\024':
         /*
              |CHR(07H): IF (hr<99.9) & (hr>-99.9) THEN 
                           IF verb THEN WrStr("dewp "); WrFixed(hr, 1, 0);
                WrStr("oC"); END;
                           pc^.dewp:=hr;
                           pc^.tdewp:=systime;
                         END;
         */
         pc->gpsdate = unixdate(2000UL+val%100UL, (val/100UL)%100UL,
                (val/10000UL)%100UL);
         if (sondeaprs_verb) {
            osi_WrStr("date", 5ul);
            aprsstr_IntToStr((int32_t)(val%1000000UL+1000000UL), 1UL, s,
                1001ul);
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
         hr = latlong(val, c50);
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
         hr = latlong(val, c50);
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
            osi_WrHex((uint32_t)(uint8_t)cb[0U], 0UL);
            osi_WrStr(" ", 2ul);
            osi_WrHex((uint32_t)(uint8_t)cb[1U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[2U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[3U], 0UL);
            osi_WrHex((uint32_t)(uint8_t)cb[4U], 0UL);
            osic_WrFixed((float)hr, 2L, 10UL);
         }
         break;
      } /* end switch */
   }
   { /* with */
      struct CONTEXTC34 * anonym2 = pc;
      if (posok && (sondeaprs_nofilter || (((((anonym2->lastsent!=systime && anonym2->tlon+8UL>systime)
                 && anonym2->tlat+8UL>systime) && anonym2->talt+20UL>systime)
                 && anonym2->tspeed+120UL>systime)
                && anonym2->tdir+120UL>systime)
                && anonym2->tgpstime+120UL>systime)) {
         if (anonym2->ttemp+30UL>systime) stemp = anonym2->temp;
         else stemp = (double)X2C_max_real;
         if (anonym2->thum+30UL>systime) shum = anonym2->hum;
         else shum = (double)X2C_max_real;
         exlon = extrapolate(anonym2->lon1, anonym2->lon, anonym2->tlon1,
                anonym2->tlon, systime, &lonok);
         exlat = extrapolate(anonym2->lat1, anonym2->lat, anonym2->tlat1,
                anonym2->tlat, systime, &latok);
         /*
         IF lonok THEN WrStrLn("--good ") ELSE WrStrLn("--bad  ") END;
         WrInt(systime-tlon1, 10); WrInt(systime-tlon, 10);
         WrFixed(lon1/RAD, 5,0); WrStr(" ");WrFixed(lon/RAD, 5,0);
                WrStr(" ");
         WrFixed(exlon/RAD, 5,0); WrStrLn("t1 t x1 x xext");
         */
         if (lonok && latok) {
            if (c50) strncpy(tstr,"SRSC50",51u);
            else strncpy(tstr,"SRSC34",51u);
            xdatablock.cnt = 0UL;
            sondeaprs_senddata(exlat, exlon, anonym2->alt, anonym2->speed,
                anonym2->dir, anonym2->clmb, 0.0, shum, stemp, 0.0, 0.0, 0.0,
                 0.0, (double) -(float)(uint32_t)sendmhzfromsdr,
                0.0, 0.0, ((systime-anonym2->tgpstime)+anonym2->gpstime)%86400UL+anonym2->gpsdate,
                 0UL, anonym2->name, 9ul, 0UL, 0UL, 0UL, 0.0, usercall, 11ul,
                 0UL, 0.0, sondeaprs_nofilter, 0, 0L, tstr, 51ul, "", 1ul, 0,
                 sdrblock, 0L, xdatablock);
            anonym2->lastsent = systime;
         }
      }
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
} /* end decodec34() */

/*------------------------------ DFM */

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

#define sondemod_DIST 10
/*km*/


static void jumpcheck(float p1, float p2, uint32_t * cnt)
{
   if (p2!=0.0f && (float)fabs(p1-p2)>1.5707963267949E-3f) *cnt = 30UL;
   else if (*cnt>0UL) --*cnt;
} /* end jumpcheck() */


static void checkdf69(float long0, char * df9)
{
   if (dfmswap) *df9 = long0<30.0f;
   else *df9 = 0;
} /* end checkdf69() */

static uint32_t sondemod_MON0[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};


static void decodesub(const char b[], uint32_t b_len,
                pCONTEXTDFM6 pc, uint32_t subnum)
{
   uint32_t u;
   uint32_t v;
   int32_t vi;
   double vr;
   switch (bits2val(b, b_len, 48UL, 4UL)) {
   case 0UL:
      if (pc->d9) {
         /* dfm09 speed */
         u = bits2val(b, b_len, 32UL, 16UL);
         vr = (double)u*0.01;
         if (vr<999.0) {
            pc->speed = vr;
            pc->tspeed = systime;
         }
         if (sondeaprs_verb) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.036f, 1L, 0UL);
            osi_WrStr("km/h", 5ul);
         }
      }
      break;
   case 1UL:
      if (pc->d9) {
         /* dfm09 lat, dir */
         vi = (int32_t)bits2val(b, b_len, 0UL, 32UL);
         u = bits2val(b, b_len, 32UL, 16UL);
         vr = (double)vi*1.E-7;
         if ((vr!=0.0 && vr<89.9) && vr>(-89.9)) {
            pc->lat1 = pc->lat;
            pc->tlat1 = pc->tlat;
            pc->lat = vr*1.7453292519943E-2;
            pc->tlat = systime;
            pc->posok = 1;
            jumpcheck((float)pc->lat, (float)pc->lat1, &pc->poserr);
         }
         vr = (double)u*0.01;
         if (vr<=360.0) {
            pc->dir = vr;
            pc->tdir = systime;
         }
         if (sondeaprs_verb) {
            osi_WrStr(" Lat: ", 7ul);
            osic_WrFixed((float)(X2C_DIVL(pc->lat,1.7453292519943E-2)),
                5L, 0UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.01f, 1L, 0UL);
            osi_WrStr(" deg", 5ul);
         }
      }
      break;
   case 2UL:
      vi = (int32_t)bits2val(b, b_len, 0UL, 32UL);
      vr = (double)vi*1.E-7;
      checkdf69((float)vr, &pc->d9); /* test if dfm6 or dfm9 */
      if (pc->d9) {
         /* dfm09 long clb */
         if ((vr!=0.0 && vr<180.0) && vr>(-180.0)) {
            pc->lon1 = pc->lon; /* save 2 values for extrapolating */
            pc->tlon1 = pc->tlon;
            pc->lon = vr*1.7453292519943E-2;
            pc->tlon = systime;
            pc->posok = 1;
            jumpcheck((float)pc->lon, (float)pc->lon1, &pc->poserr);
         }
         vi = (int32_t)bits2val(b, b_len, 32UL, 16UL);
         if (vi>=32768L) vi -= 65536L;
         vr = (double)vi*0.01;
         if (vr<50.0 && vr>(-500.0)) pc->clmb = vr;
         if (sondeaprs_verb) {
            osi_WrStr(" Long:", 7ul);
            osic_WrFixed((float)(X2C_DIVL(pc->lon,1.7453292519943E-2)),
                5L, 0UL);
            osic_WrFixed((float)pc->clmb, 1L, 0UL);
            osi_WrStr(" m/s", 5ul);
         }
      }
      else {
         /* dfm06 lat speed */
         u = bits2val(b, b_len, 32UL, 16UL);
         if ((vr!=0.0 && vr<89.9) && vr>(-89.9)) {
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
            osic_WrFixed((float)(X2C_DIVL(pc->lat,1.7453292519943E-2)),
                5L, 0UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.036f, 1L, 0UL);
            osi_WrStr("km/h", 5ul);
         }
      }
      break;
   case 3UL: /* dfm09 alt */
      if (pc->d9) {
         v = bits2val(b, b_len, 0UL, 32UL);
         vr = (double)v*0.01;
         if (vr<50000.0) {
            pc->alt = vr;
            pc->talt = systime;
         }
         if (sondeaprs_verb) {
            osi_WrStr(" alti:", 7ul);
            osic_WrFixed((float)pc->alt, 1L, 0UL);
            osi_WrStr("m ", 3ul);
         }
      }
      else {
         /* dfm06 long, dir */
         vi = (int32_t)bits2val(b, b_len, 0UL, 32UL);
         u = bits2val(b, b_len, 32UL, 16UL);
         vr = (double)vi*1.E-7;
         if ((vr!=0.0 && vr<180.0) && vr>(-180.0)) {
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
            osic_WrFixed((float)(X2C_DIVL(pc->lon,1.7453292519943E-2)),
                5L, 0UL);
            osi_WrStr(" ", 2ul);
            osic_WrFixed((float)u*0.01f, 1L, 0UL);
            osi_WrStr(" deg", 5ul);
         }
      }
      break;
   case 4UL:
      if (pc->d9) {
      }
      else {
         /* dfm06 alt, speed */
         v = bits2val(b, b_len, 0UL, 32UL);
         vi = (int32_t)bits2val(b, b_len, 32UL, 16UL);
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
      }
      break;
   case 8UL:
      pc->gpsdate = unixdate(bits2val(b, b_len, 0UL, 12UL), bits2val(b,
                b_len, 12UL, 4UL), bits2val(b, b_len, 16UL, 5UL));
      break;
   } /* end switch */
} /* end decodesub() */


static void getdfserial(const char rxb[], uint32_t rxb_len,
                uint32_t p, pCONTEXTDFM6 pc, char ser[],
                uint32_t ser_len, char typstr[],
                uint32_t typstr_len)
{
   uint8_t max0;
   uint8_t min0;
   uint8_t st;
   uint32_t v;
   uint32_t i;
   uint32_t ix;
   uint16_t d;
   char s[101];
   pDFMTYPES defaulttyp;
   pDFMTYPES dftyp;
   struct CONTEXTDFM6 * anonym;
   /* start byte found */
   struct DFNAMES * anonym0;
   /* make new entry */
   struct DFNAMES * anonym1;
   /* find best lo/hi pair */
   struct DFNAMES * anonym2;
   ser[0UL] = 0;
   typstr[0UL] = 0;
   { /* with */
      struct CONTEXTDFM6 * anonym = pc;
      st = rxb[p]; /* frame start byte */
      ix = (uint32_t)(uint8_t)rxb[p+3UL]/16UL; /* hi/lo part of ser */
      d = (uint16_t)((uint32_t)(uint8_t)rxb[p+1UL]*256UL+(uint32_t)
                (uint8_t)rxb[p+2UL]); /* data bytes */
      if ((uint32_t)st>anonym->lastfrid) anonym->lastfrid = (uint32_t)st;
      i = 0UL;
      while (i<anonym->nameregtop && anonym->namereg[i].start!=st) ++i;
      if (i<anonym->nameregtop) {
         { /* with */
            struct DFNAMES * anonym0 = &anonym->namereg[i];
            if (ix<=1UL && (anonym0->cnt[ix]==0U || anonym0->dat[ix]==d)) {
               anonym0->dat[ix] = d;
               if (anonym0->cnt[ix]<255U) ++anonym0->cnt[ix];
               anonym0->errcnt = 0U;
            }
            else if ((anonym0->errcnt==0U && anonym0->cnt[0U]>10U)
                && anonym0->cnt[1U]>10U) {
               /* first checksum error on stable ser */
               ++anonym0->errcnt; /* break transmitting ser */
               if (sondeaprs_verb) osi_WrStr(" ser checksum ", 15ul);
            }
            else {
               /* data changed second time kill ser */
               anonym0->cnt[0U] = 0U;
               anonym0->cnt[1U] = 0U;
            }
         }
      }
      else if (ix<=1UL) {
         { /* with */
            struct DFNAMES * anonym1 = &anonym->namereg[anonym->nameregtop];
            anonym1->start = st;
            anonym1->cnt[0U] = 0U;
            anonym1->cnt[1U] = 0U;
            anonym1->errcnt = 0U;
            anonym1->dat[ix] = d;
            anonym1->cnt[ix] = 1U;
         }
         if (anonym->nameregtop<49UL) ++anonym->nameregtop;
      }
      i = 0UL;
      max0 = 0U;
      while (i<anonym->nameregtop) {
         { /* with */
            struct DFNAMES * anonym2 = &anonym->namereg[i];
            min0 = anonym2->cnt[0U];
            if (min0>anonym2->cnt[1U]) min0 = anonym2->cnt[1U];
            if (min0>max0) {
               max0 = min0;
               v = i;
            }
         }
         ++i;
      }
      if (max0>2U && anonym->namereg[v].errcnt==0U) {
         /* may be good enough */
         aprsstr_Assign(ser, ser_len, "[  ]", 5ul);
         ser[1UL] = hex((uint32_t)(anonym->namereg[v].start/16U));
         ser[2UL] = hex((uint32_t)anonym->namereg[v].start);
         aprsstr_CardToStr((uint32_t)
                anonym->namereg[v].dat[0U]*65536UL+(uint32_t)
                anonym->namereg[v].dat[1U], 1UL, s, 101ul);
         aprsstr_Append(ser, ser_len, s, 101ul);
         dftyp = dftypes;
         defaulttyp = 0;
         for (;;) {
            /* find type string */
            if (dftyp==0) break;
            /* safe version same es serial source */
            if ((uint32_t)(anonym->namereg[v].start/16U)==dftyp->n) {
               aprsstr_Assign(typstr, typstr_len, dftyp->t, 9ul);
               break;
            }
            /* propagated version simple highest number */
            /*      IF lastfrid DIV 16=dftyp^.n THEN Assign(typstr,
                dftyp^.t); EXIT END; */
            if (dftyp->n==255UL) defaulttyp = dftyp;
            dftyp = dftyp->next;
         }
         if (dftyp==0 && defaulttyp) {
            aprsstr_Assign(typstr, typstr_len, defaulttyp->t, 9ul);
         }
      }
   }
   if (typstr[0UL]==0) aprsstr_Assign(typstr, typstr_len, "DFM", 4ul);
} /* end getdfserial() */


static void decodedfm6(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   uint32_t rt;
   char db[56];
   pCONTEXTDFM6 pc0;
   pCONTEXTDFM6 pc1;
   pCONTEXTDFM6 pc;
   OBJNAME nam;
   char cb[10];
   char s[1001];
   char ser[16];
   char typstr[9];
   CALLSSID usercall;
   uint32_t ib;
   uint32_t j;
   uint32_t i;
   double exlat;
   double exlon;
   char latok;
   char lonok;
   struct CONTEXTDFM6 * anonym;
   if (rxb[0UL]!='D') return;
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
      rt = rt*256UL+(uint32_t)(uint8_t)rxb[i]; /* realtime */
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
      if (rt>0UL) {
         wrdate(rt);
         osi_WrStr(" ", 2ul);
      }
   }
   if (sondeaprs_verb) {
      osi_WrStr(nam, 9ul);
      osi_WrStr(" ", 2ul);
   }
   pc = pcontextdfm6;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextdfm6 = pc1;
         else pc0->next = pc1;
         osic_free((char * *) &pc, sizeof(struct CONTEXTDFM6));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTDFM6));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTDFM6));
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
      getdfserial(rxb, rxb_len, i, pc, ser, 16ul, typstr, 9ul);
      i += 4UL;
      for (j = 0UL; j<=6UL; j++) {
         for (ib = 0UL; ib<=7UL; ib++) {
            db[ib+8UL*j] = X2C_IN(7UL-ib,8,(uint8_t)(uint8_t)rxb[i]);
         } /* end for */
         ++i;
      } /* end for */
      decodesub(db, 56ul, pc, 0UL);
      for (j = 0UL; j<=6UL; j++) {
         for (ib = 0UL; ib<=7UL; ib++) {
            db[ib+8UL*j] = X2C_IN(7UL-ib,8,(uint8_t)(uint8_t)rxb[i]);
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
               xdatablock.cnt = 0UL;
               sondeaprs_senddata(exlat, exlon, anonym->alt, anonym->speed,
                anonym->dir, anonym->clmb, 0.0, 0.0,
                (double)X2C_max_real, 0.0, 0.0, 0.0, 0.0,
                (double) -(float)(uint32_t)sendmhzfromsdr, 0.0,
                0.0, anonym->actrt, 0UL, anonym->name, 9ul, 0UL, 0UL, 0UL,
                0.0, usercall, 11ul, 0UL, 0.0, sondeaprs_nofilter, 0, 0L,
                typstr, 9ul, ser, 16ul, 0, sdrblock, 0L, xdatablock);
               anonym->lastsent = systime;
            }
         }
      }
   }
   else if (sondeaprs_verb) {
      if (rt==0UL) osi_WrStr(" got no date", 13ul);
      else {
         osi_WrStr(" frame delayed ", 16ul);
         osic_WrINT32(pc->actrt-rt, 1UL);
         osi_WrStr("s", 2ul);
      }
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
} /* end decodedfm6() */

/*------------------------------ RS41 */
#define sondemod_FULLID 536


static void WrChChk(char ch)
{
   if ((uint8_t)ch>=' ' && (uint8_t)ch<'\177') {
      osi_WrStr((char *) &ch, 1u/1u);
   }
} /* end WrChChk() */

#define sondemod_EARTHA 6.378137E+6

#define sondemod_EARTHB 6.3567523142452E+6

#define sondemod_E2 6.6943799901413E-3

#define sondemod_EARTHAB 4.2841311513312E+4


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


static uint32_t gethex(const char frame[], uint32_t frame_len,
                uint32_t p, uint32_t nibb)
{
   uint32_t c;
   uint32_t n;
   n = 0UL;
   while (nibb>0UL) {
      n = n*16UL;
      c = (uint32_t)(uint8_t)frame[p];
      if (c>=48UL && c<=57UL) n += c-48UL;
      else if (c>=65UL && c<=70UL) n += c-55UL;
      else return 0UL;
      ++p;
      --nibb;
   }
   return n;
} /* end gethex() */


static void posrs41(const char b[], uint32_t b_len, uint32_t p,
                double * lat, double * long0,
                double * heig, double * speed,
                double * dir, double * clmb, uint32_t * sats)
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
   vn = (-(vx*sin(*lat)*cos(*long0))-vy*sin(*lat)*sin(*long0))+vz*cos(*lat);
   ve = -(vx*sin(*long0))+vy*cos(*long0);
   vu = vx*cos(*lat)*cos(*long0)+vy*cos(*lat)*sin(*long0)+vz*sin(*lat);
   *dir = X2C_DIVL(atang2(vn, ve),1.7453292519943E-2);
   if (*dir<0.0) *dir = 360.0+*dir;
   *speed = sqrt(vn*vn+ve*ve);
   *clmb = vu;
   *sats = (uint32_t)(uint8_t)b[p+18UL];
   /*  hdop:=ORD(b[p+20]); */
   if (sondeaprs_verb) {
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)( *speed*3.6), 2L, 1UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrFixed((float)*dir, 1L, 1UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)vu, 1L, 1UL);
      osi_WrStr("m/s Sats:", 10ul);
      osic_WrINT32(*sats, 1UL);
   }
} /* end posrs41() */


static double altToPres(double a)
/* meter to hPa */
{
   if (a<=0.0) return 1010.0;
   else if (a>40000.0) return 0.0;
   else if (a>15000.0) return exp(a*(-1.5873015873016E-4)+0.2629)*1000.0;
   else return 1010.0*exp(log((293.0-0.0065*a)*3.4129692832765E-3)*5.26);
   return 0;
} /* end altToPres() */


static uint32_t calperc41(SET51 cs)
{
   uint32_t n;
   uint32_t i;
   SET51 tmp;
   cs = (uint32_t *)memcpy(tmp,cs,8u);
   n = 0UL;
   for (i = 0UL; i<=50UL; i++) {
      if (X2C_INL(i,51,cs)) ++n;
   } /* end for */
   return (n*100UL)/51UL;
} /* end calperc41() */


static char setinset(const SET51 a, const SET51 b, uint32_t n)
/* c language workaround */
{
   uint32_t i;
   uint32_t tmp;
   tmp = n-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if (X2C_INL(i,51,b) && !X2C_INL(i,51,a)) return 0;
      if (i==tmp) break;
   } /* end for */
   return 1;
} /* end setinset() */


static float waterVaporSaturationPressure(float temp)
{
   float p;
   temp = temp+273.15f; /* kelvin */
   temp = (((-4.931358E-1f)+1.00461f*temp)-1.3746454E-5f*temp*temp)
                +1.2743214E-8f*temp*temp*temp;
   if (temp<=0.0f) return 0.0f;
   p = (float)exp(((((double)(-(X2C_DIVR(5.8002206E+3f,
                temp))+1.3914993f)+6.5459673*log((double)temp))
                -(double)(4.8640239E-2f*temp))+(double)
                (4.1764768E-5f*temp*temp))-(double)
                (1.4452093E-8f*temp*temp*temp));
   return p*0.01f;
/* hPa */
} /* end waterVaporSaturationPressure() */

#define sondemod_CALT10 77

#define sondemod_CALT11 81

#define sondemod_CALT12 85

#define sondemod_CALT13 89

#define sondemod_POLYT 93

#define sondemod_CALT20 293

#define sondemod_CALT21 297

#define sondemod_CALT22 301

#define sondemod_CALT23 305

#define sondemod_POLYTH 309

#define sondemod_CALRf1 61

#define sondemod_CALRf2 65

#define sondemod_CALC1 69

#define sondemod_CALC2 73

#define sondemod_CALHUM1 117

#define sondemod_CALHUM2 121

#define sondemod_HUMMATRIX 125

static SET51 sondemod_TEMP1CAL = {0x00000078UL,0x00000000UL};

static SET51 sondemod_TEMP2HUMCAL = {0x001FFF98UL,0x00000000UL};

#define sondemod_PRESSPOLY 606
/* 18 float */

static SET51 sondemod_PRESSCAL = {0x00000000UL,0x000007E2UL};


static float getcal(pCONTEXTR4 pc, uint32_t p)
{
   uint32_t i;
   uint32_t n;
   n = 0UL;
   for (i = 3UL;; i--) {
      n = n*256UL;
      n += (uint32_t)(uint8_t)pc->calibdata[p+i];
      if (i==0UL) break;
   } /* end for */
   return SaveReal(n);
} /* end getcal() */


static uint32_t getcard(char rxb[], uint32_t rxb_len, uint32_t p,
                uint32_t bytes)
{
   uint32_t i;
   uint32_t n;
   n = 0UL;
   for (i = bytes-1UL;; i--) {
      n = n*256UL;
      n += (uint32_t)(uint8_t)rxb[p+i];
      if (i==0UL) break;
   } /* end for */
   return n;
} /* end getcard() */

static SET51 _cnst4 = {0x001FFF98UL,0x00000000UL};
static SET51 _cnst3 = {0x00000000UL,0x000007E2UL};
static SET51 _cnst2 = {0x00000078UL,0x00000000UL};

static void ptu41(pCONTEXTR4 pc, uint32_t pb, uint32_t len,
                const char rxb[], uint32_t rxb_len, double * tmp,
                 double * pres, double * humid,
                double * tmphum)
{
   uint32_t j;
   uint32_t i;
   float hc;
   float th;
   float hum;
   float psum;
   float xx;
   float pt;
   float r;
   float f2;
   float f1;
   float f;
   float Rf2;
   float Rf1;
   float pw[6];
   uint32_t meas[12];
   /* data positions and calculations https://github.com/einergehtnochrein */
   for (i = 0UL; i<=11UL; i++) {
      meas[i] = getcard(rxb, rxb_len, pb+i*3UL, 3UL);
   } /* end for */
   /*- temp 1 */
   if (setinset(pc->calibok, _cnst2, 52UL)) {
      /* needed calibs valid temperature 1 */
      f = (float)meas[0U];
      f1 = (float)meas[1U];
      f2 = (float)meas[2U];
      Rf1 = getcal(pc, 61UL);
      Rf2 = getcal(pc, 65UL);
      r = f2-f1;
      if (r!=0.0f) {
         r = (Rf1+X2C_DIVR((Rf2-Rf1)*(f-f1),r))*getcal(pc, 89UL);
         r = getcal(pc, 77UL)+r*getcal(pc, 81UL)+r*r*getcal(pc, 85UL);
         f = r;
         f1 = 1.0f;
         for (i = 0UL; i<=5UL; i++) {
            r = r+f1*getcal(pc, 93UL+i*4UL);
            f1 = f1*f;
         } /* end for */
         if ((float)fabs(r)<99.0f) *tmp = (double)r;
      }
   }
   /*- pres */
   if ((setinset(pc->calibok, _cnst3,
                52UL) && pc->calibdata[543U]=='P') && len>39UL) {
      /* needed calibs valid pressure and type=SGP */
      pt = (float)(short)getcard(rxb, rxb_len, pb+38UL, 2UL)*0.01f;
      pw[0U] = getcal(pc, 606UL)+pt*getcal(pc, 634UL)+pt*pt*getcal(pc,
                650UL)+pt*pt*pt*getcal(pc, 666UL);
      pw[1U] = getcal(pc, 610UL)+pt*getcal(pc, 638UL)+pt*pt*getcal(pc,
                654UL)+pt*pt*pt*getcal(pc, 670UL);
      pw[2U] = getcal(pc, 614UL)+pt*getcal(pc, 642UL)+pt*pt*getcal(pc,
                658UL)+pt*pt*pt*getcal(pc, 674UL);
      pw[3U] = getcal(pc, 618UL)+pt*getcal(pc, 646UL)+pt*pt*getcal(pc,
                662UL);
      pw[4U] = getcal(pc, 622UL);
      pw[5U] = getcal(pc, 626UL);
      f = (float)meas[9U];
      f1 = (float)meas[10U];
      f2 = (float)meas[11U];
      r = f-f1;
      if (r!=0.0f) {
         r = X2C_DIVR((f2-f1)*getcal(pc, 630UL),r);
         xx = 1.0f;
         psum = 0.0f;
         for (i = 0UL; i<=5UL; i++) {
            psum = psum+pw[i]*xx;
            xx = xx*r;
         } /* end for */
         *pres = (double)psum;
      }
   }
   /*- temp 2 on hum sensor  */
   *tmphum = (double)X2C_max_real;
   if (setinset(pc->calibok, _cnst4, 52UL)) {
      /* needed calibs valid temperature 2 */
      f = (float)meas[6U];
      f1 = (float)meas[7U];
      f2 = (float)meas[8U];
      Rf1 = getcal(pc, 61UL);
      Rf2 = getcal(pc, 65UL);
      r = f2-f1;
      if (r!=0.0f) {
         r = (Rf1+X2C_DIVR((Rf2-Rf1)*(f-f1),r))*getcal(pc, 305UL);
         r = getcal(pc, 293UL)+r*getcal(pc, 297UL)+r*r*getcal(pc, 301UL);
         if ((float)fabs(r)<99.0f) *tmphum = (double)r;
      }
      else {
         /*WrStrLn("");WrFixed(r, 2, 10); WrStrLn(" tmphum");
                (* a bit too hot *) */
         *tmphum = (double)X2C_max_real;
      }
      /*- hum */
      if (fabs(*tmphum)<100.0 && fabs(*tmp)<100.0) {
         th = (float)*tmphum;
         f = th;
         f1 = 1.0f;
         for (i = 0UL; i<=5UL; i++) {
            th = th+f1*getcal(pc, 309UL+i*4UL); /* modify hum sensor tmp */
            f1 = f1*f;
         } /* end for */
         f = (float)meas[3U];
         f1 = (float)meas[4U];
         f2 = (float)meas[5U];
         r = f2-f1;
         if (r!=0.0f) {
            r = X2C_DIVR(f-f1,r);
            hc = getcal(pc, 69UL)+(getcal(pc, 73UL)-getcal(pc, 69UL))*r;
            r = getcal(pc, 117UL);
            if (r!=0.0f) {
               hc = (X2C_DIVR(hc,r)-1.0f)*getcal(pc, 121UL);
               /*WrStrLn("");WrFixed(hc, 4, 14); WrStrLn(" hc"); */
               hum = 0.0f;
               f1 = 1.0f;
               for (i = 0UL; i<=6UL; i++) {
                  f2 = 1.0f;
                  for (j = 0UL; j<=5UL; j++) {
                     hum = hum+f1*f2*getcal(pc, 125UL+(i*6UL+j)*4UL);
                     f2 = f2*(th-20.0f)*5.5555555555556E-3f;
                  } /* end for */
                  f1 = f1*hc;
               } /* end for */
               r = waterVaporSaturationPressure((float)*tmp);
               if (r>0.0f) {
                  *humid = (double)
                (X2C_DIVR(hum*waterVaporSaturationPressure((float)*tmphum)
                ,r));
               }
            }
         }
      }
   }
/*WrStrLn("");WrFixed(humid, 2, 10); WrStrLn(" hum"); */
} /* end ptu41() */

static uint8_t sondemod_POWERTAB[8] = {1U,2U,5U,8U,11U,14U,17U,20U};

#define sondemod_BURSTKILL 43

#define sondemod_TXTIME 800
/* remainin tx time position */

#define sondemod_BURSTTXTIME 790

#define sondemod_TXFRAMES 39
/* good calibration and ptu data description: https://github.com/einergehtnochrein */

static uint8_t _cnst5[8] = {1U,2U,5U,8U,11U,14U,17U,20U};

static void decoders41(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   OBJNAME nam;
   int32_t res;
   int32_t txoff;
   int32_t txtime;
   char s[1001];
   CALLSSID usercall;
   char fullid[12];
   uint32_t frameno;
   uint32_t len;
   uint32_t txpower;
   uint32_t sats;
   uint32_t p;
   uint32_t j;
   uint32_t i;
   char nameok;
   char calok;
   char typ;
   pCONTEXTR4 pc0;
   pCONTEXTR4 pc1;
   pCONTEXTR4 pc;
   double vBatt;
   double tmphum;
   double humidity;
   double pressure;
   double temperature;
   double ozonval;
   double climb;
   double dir;
   double speed;
   double heig;
   double long0;
   double lat;
   sondeaprs_SATSIG satsigs;
   sondeaprs_pSATSIG psatsigs;
   uint32_t tmp;
   calok = 0;
   nameok = 0;
   nam[0U] = 0;
   fullid[0U] = 0;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   ozonval = 0.0;
   temperature = (double)X2C_max_real;
   pressure = (double)X2C_max_real;
   humidity = (double)X2C_max_real;
   sats = 0UL;
   txpower = 0UL;
   vBatt = 0.0;
   frameno = 0UL;
   txoff = X2C_max_longint;
   getcall(rxb, rxb_len, usercall, 11ul);
   txtime = X2C_max_longint;
   psatsigs = 0;
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
   xdatablock.cnt = 0UL;
   p = 57UL;
   if (sondeaprs_verb) osi_WrStr("R41 ", 5ul);
   for (;;) {
      if (p+4UL>=rxb_len-1) break;
      typ = rxb[p];
      /*WrStr("TYP============="); WrHex(ORD(typ), 3); WrStr(" "); */
      ++p;
      len = (uint32_t)(uint8_t)rxb[p]+2UL;
      ++p;
      if (p+len>=rxb_len-1) break;
      if (!crcrs(rxb, rxb_len, (int32_t)p, (int32_t)(p+len))) {
         if (sondeaprs_verb) osi_WrStr(" ----  crc err ", 16ul);
         break;
      }
      if (typ=='y') {
         if (p>(rxb_len-1)-9UL) break;
         nameok = 1;
         for (i = 0UL; i<=7UL; i++) {
            nam[i] = rxb[p+2UL+i];
            if ((uint8_t)nam[i]<=' ' || (uint8_t)nam[i]>'Z') nameok = 0;
         } /* end for */
         nam[8U] = 0;
         j = (uint32_t)(uint8_t)rxb[p+21UL]; /* txpow 0..7 */
         if (j<=7UL) txpower = (uint32_t)_cnst5[j];
         vBatt = (double)((float)(uint32_t)(uint8_t)
                rxb[p+10UL]*0.1f); /* battery voltage */
         pc = pcontextr4;
         pc0 = 0;
         for (;;) {
            if (pc==0) break;
            pc1 = pc->next;
            if (pc->tused+3600UL<systime) {
               /* timed out */
               if (pc0==0) pcontextr4 = pc1;
               else pc0->next = pc1;
               osic_free((char * *) &pc, sizeof(struct CONTEXTR4));
            }
            else {
               if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
               pc0 = pc;
            }
            pc = pc1;
         }
         if (pc==0) {
            osic_alloc((char * *) &pc, sizeof(struct CONTEXTR4));
            if (pc==0) Error("allocate context out im memory", 31ul);
            memset((char *)pc,(char)0,sizeof(struct CONTEXTR4));
            pc->next = pcontextr4;
            pcontextr4 = pc;
            aprsstr_Assign(pc->name, 9ul, nam, 9ul);
            if (sondeaprs_verb) osi_WrStrLn(" is new ", 9ul);
         }
         frameno = getcard16(rxb, rxb_len, p);
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
            osi_WrStr(" ", 2ul);
         }
         pc->flightstate = rxb[p+13UL]; /* 0 start, 1 flight, 2 decent */
         j = (uint32_t)(uint8_t)rxb[p+23UL]; /* calib frame number */
         if (j<=50UL && p<((rxb_len-1)-24UL)-15UL) {
            if (!X2C_INL(j,51,pc->calibok)) {
               X2C_INCL(pc->calibok,j,51);
               for (i = 0UL; i<=15UL; i++) {
                  pc->calibdata[j*16UL+i] = rxb[p+24UL+i];
               } /* end for */
               if (sondeaprs_verb) {
                  osi_WrStr("Cal:[", 6ul);
                  for (i = 0UL; i<=50UL; i++) {
                     if (X2C_INL(i,51,pc->calibok)) osi_WrStr("+", 2ul);
                     else osi_WrStr("-", 2ul);
                  } /* end for */
                  osi_WrStr("] ", 3ul);
               }
            }
            if (j==0UL) {
               pc->mhz0 = (float)(getcard16(rxb, rxb_len,
                p+24UL+2UL)/64UL+40000UL)*0.01f+0.0005f;
            }
            else if (j==50UL) {
               txoff = getint16(rxb, rxb_len, p+24UL+0UL);
               pc->txtime = txoff;
               if (pc->txtime>=0L) {
                  pc->txtime += (int32_t)frameno;
                /* descending countdown */
               }
               else if (X2C_INL((int32_t)49,51,pc->calibok)) {
                  pc->txtime = 100000L+getint16(pc->calibdata, 816ul, 790UL);
                 /* show default time */
               }
            }
            else if (j==2UL) {
               pc->bk = rxb[p+24UL+11UL]=='\001';
               txtime = getint16(rxb, rxb_len, p+24UL+7UL);
            }
         }
         if (sondeaprs_verb) {
            osi_WrStr(pc->name, 9ul);
            osi_WrStr(" ", 2ul);
            osic_WrINT32(pc->framenum, 1UL);
            if (txoff<=32767L) {
               osi_WrStr(" TxOff=", 8ul);
               osic_WrINT32((uint32_t)txoff, 1UL);
            }
            if (txtime<=32767L) {
               if (pc->bk) osi_WrStr(" TxTime(BK)=", 13ul);
               else osi_WrStr(" TxTime=", 9ul);
               osic_WrINT32((uint32_t)txtime, 1UL);
            }
         }
         if (X2C_INL((int32_t)33,51,pc->calibok)) {
            for (i = 0UL; i<=7UL; i++) {
               fullid[i] = pc->calibdata[i+536UL];
            } /* end for */
            fullid[8U] = 0;
            if (sondeaprs_verb) {
               osi_WrStr(" [", 3ul);
               osi_WrStr(fullid, 12ul);
               osi_WrStr("]", 2ul);
            }
         }
      }
      else if (typ=='z') {
         if (pc && p<(rxb_len-1)-44UL) {
            ptu41(pc, p, len, rxb, rxb_len, &temperature, &pressure,
                &humidity, &tmphum);
            if (sondeaprs_verb) {
               if (fabs(temperature)<100.0) {
                  osi_WrStr(" t=", 4ul);
                  osic_WrFixed((float)temperature, 1L, 1UL);
                  osi_WrStr("C", 2ul);
               }
               if (pressure>0.1 && pressure<1500.0) {
                  osi_WrStr(" p=", 4ul);
                  osic_WrFixed((float)pressure, 2L, 1UL);
                  osi_WrStr("hPa", 4ul);
               }
               if (fabs(humidity)<1000.0) {
                  osi_WrStr(" h=", 4ul);
                  osic_WrFixed((float)humidity, 1L, 1UL);
                  osi_WrStr("%", 2ul);
                  if (fabs(humidity)<1000.0) {
                     osi_WrStr(" th=", 5ul);
                     osic_WrFixed((float)tmphum, 1L, 1UL);
                     osi_WrStr("C", 2ul);
                  }
               }
            }
         }
      }
      else if (typ=='|') {
         if (pc) {
            pc->gpssecond = (uint32_t)(getint32(rxb, rxb_len,
                p+2UL)/1000L)+(uint32_t)(86400L-leapseconds)
                %86400UL+getcard16(rxb, rxb_len, p)*604800UL+315878400UL;
            /* gps TOW */
            for (i = 0UL; i<=11UL; i++) {
               satsigs[i].num = (uint32_t)(uint8_t)rxb[p+6UL+i*2UL];
               satsigs[i].level = (uint32_t)(uint8_t)rxb[p+7UL+i*2UL];
            } /* end for */
            psatsigs = (sondeaprs_pSATSIG)satsigs;
         }
      }
      else if (typ=='}') {
      }
      else if (typ=='{') {
         /*             WrStrLn("7D frame"); */
         if (pc && p<(rxb_len-1)-18UL) {
            posrs41(rxb, rxb_len, p, &lat, &long0, &heig, &speed, &dir,
                &climb, &sats);
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
               res = (int32_t)gethex(rxb, rxb_len, p+5UL, 4UL);
               if (res>=32768L) res = 32768L-res;
               pc->ozonTemp = (double)res*0.01;
               pc->ozonuA = (double)gethex(rxb, rxb_len, p+9UL,
                5UL)*0.0001;
               pc->ozonBatVolt = (double)gethex(rxb, rxb_len, p+14UL,
                2UL)*0.1;
               pc->ozonPumpMA = (double)gethex(rxb, rxb_len, p+16UL,
                3UL);
               pc->ozonExtVolt = (double)gethex(rxb, rxb_len, p+19UL,
                2UL)*0.1;
               ozonval = (double)calcOzone(pc->ozonuA, pc->ozonTemp,
                pc->hp);
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
               if (((uint32_t)(uint8_t)rxb[17UL]&1)) {
                  osi_WrStr(" NotCal", 8ul);
               }
               osi_WrStr(" V:", 4ul);
               osic_WrFixed((float)gethex(rxb, rxb_len, p+18UL, 2UL)*0.1f,
                 1L, 1UL);
               osi_WrStr(")", 2ul);
            }
         }
         storexdata(&xdatablock, rxb, rxb_len, p, len-2UL);
      }
      else if (typ=='v') {
      }
      else if (typ=='\200') {
         /*             WrStrLn("76 frame"); */
         if (sondeaprs_verb) osi_WrStr(" Encryption frame", 18ul);
         if (encryptmsgfn[0U] && pc) {
            ++pc->encrcnt;
            if (pc->encrcnt==2UL) {
               WrSemaFile(encryptmsgfn, 1024ul, pc->name, 9ul, sdrblock);
                /* write semaphore file for alarm email */
            }
         }
      }
      /*    ELSE EXIT END; */
      if (typ=='v') break;
      p += len;
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
   if (((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) && sats>0UL) {
      res = pc->txtime;
      if (res<100000L) res -= (int32_t)frameno;
      sondeaprs_senddata(lat, long0, heig, speed, dir, climb, 0.0, humidity,
                temperature, ozonval, pc->ozonTemp, pc->ozonPumpMA,
                pc->ozonBatVolt, (double)pc->mhz0, (-1.0), 0.0,
                pc->gpssecond, frameno, pc->name, 9ul, 0UL, sats, txpower,
                vBatt, usercall, 11ul, calperc41(pc->calibok), pressure,
                sondeaprs_nofilter, 0, res, "RS41", 5ul, fullid, 12ul,
                psatsigs, sdrblock, leapseconds, xdatablock);
      pc->framesent = 1;
   }
/*  IF verb THEN WrStrLn("") END;   */
} /* end decoders41() */

/*------------------------------ M10 */

static uint16_t crcm10(int32_t from, int32_t len, const char buf[],
                 uint32_t buf_len)
{
   int32_t i;
   uint16_t s;
   uint16_t t;
   uint16_t b;
   uint16_t cs;
   int32_t tmp;
   cs = 0U;
   tmp = (from+len)-1L;
   i = from;
   if (i<=tmp) for (;; i++) {
      /*WrHex(ORD(buf[i]), 3); */
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

#define sondemod_FH 16
/* size of header before payload */

static float sondemod_DEGMUL10 = 8.3819036711397E-8f;

#define sondemod_VMUL10 0.005

#define sondemod_DEGMUL20 1.E-6

#define sondemod_VMUL20 0.01

static char sondemod_ISM20 = 'E';

#define sondemod_CRCPOS20 68

#define sondemod_CRCPOS10 99

static float sondemod_Rs[3] = {12100.0f,36500.0f,4.75E+5f};

static float sondemod_Rp[3] = {1.21E-16f,1.1060606E-1f,1.5833333E-1f};

static float _cnst7[3] = {12100.0f,36500.0f,4.75E+5f};
static float _cnst6[3] = {1.21E-16f,1.1060606E-1f,1.5833333E-1f};

static void decodem10(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   uint32_t gpstimecorr;
   uint32_t week;
   uint32_t tow;
   uint32_t cs;
   uint32_t ii;
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
   uint32_t time0;
   OBJNAME nam;
   char cb[10];
   char s[1001];
   CALLSSID usercall;
   uint32_t frameno;
   uint32_t fnum;
   uint32_t sct;
   uint32_t sats;
   float batt;
   float rtok;
   float rt;
   char nameok;
   char calok;
   char fullid[21];
   char wid[4];
   pCONTEXTM10 pc0;
   pCONTEXTM10 pc1;
   pCONTEXTM10 pc;
   calok = 0;
   nameok = 0;
   pc = 0;
   lat = 0.0;
   lon = 0.0;
   sats = 0UL;
   rtok = X2C_max_real;
   batt = 0.0f;
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
   if (rxb[16UL]=='E') {
      /* M20 */
      strncpy(wid,"M20",4u);
      i = 68UL;
   }
   else {
      strncpy(wid,"M10",4u);
      i = 99UL;
   }
   if (sondeaprs_verb) {
      osi_WrStr(wid, 4ul);
      osi_WrStr(" ", 2ul);
   }
   cs = (uint32_t)crcm10(16L, (int32_t)i, rxb, rxb_len);
   if (cs==m10card(rxb, rxb_len, (int32_t)(16UL+i), 2L)) {
      /* crc ok */
      nameok = 1;
      for (i = 0UL; i<=8UL; i++) {
         nam[i] = rxb[7UL+i];
         if ((uint8_t)nam[i]<=' ' || (uint8_t)nam[i]>'Z') nameok = 0;
      } /* end for */
      pc = pcontextm10;
      pc0 = 0;
      for (;;) {
         if (pc==0) break;
         pc1 = pc->next;
         if (pc->tused+3600UL<systime) {
            /* timed out */
            if (pc0==0) pcontextm10 = pc1;
            else pc0->next = pc1;
            osic_free((char * *) &pc, sizeof(struct CONTEXTM10));
         }
         else {
            if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
            pc0 = pc;
         }
         pc = pc1;
      }
      if (pc==0) {
         osic_alloc((char * *) &pc, sizeof(struct CONTEXTM10));
         if (pc==0) Error("allocate context out im memory", 31ul);
         memset((char *)pc,(char)0,sizeof(struct CONTEXTM10));
         pc->next = pcontextm10;
         pcontextm10 = pc;
         aprsstr_Assign(pc->name, 9ul, nam, 9ul);
         if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
      }
      if (rxb[16UL]=='E') {
         /* M20 */
         tow = m10card(rxb, rxb_len, 31L, 3L);
         week = m10card(rxb, rxb_len, 42L, 2L);
         gpstimecorr = (uint32_t)leapseconds;
         time0 = (tow+week*604800UL+315964800UL)-gpstimecorr;
         sats = 0UL;
      }
      else {
         tow = m10card(rxb, rxb_len, 26L, 4L);
         sats = m10card(rxb, rxb_len, 46L, 1L);
         week = m10card(rxb, rxb_len, 48L, 2L);
         gpstimecorr = (uint32_t)(uint8_t)rxb[47UL];
         time0 = tow/1000UL+week*604800UL+315878400UL+(86400UL-gpstimecorr)
                %86400UL;
      }
      frameno = time0;
      pc->gpssecond = time0;
      if (frameno>pc->framenum) {
         /* new frame number */
         pc->framesent = 0;
         calok = 1;
         pc->framenum = frameno;
         pc->tused = systime;
      }
      else if (pc->framenum==frameno) {
         if (!pc->framesent) calok = 1;
      }
      else if (sondeaprs_verb) {
         osi_WrStr(" got old frame ", 16ul);
         osic_WrINT32(frameno, 1UL);
         osi_WrStr(" expected> ", 12ul);
         osic_WrINT32(pc->framenum, 1UL);
         osi_WrStr(" ", 2ul);
      }
      if (rxb[16UL]=='E') {
         /* M20 */
         fnum = m10card(rxb, rxb_len, 37L, 1L);
         lat = (double)(int32_t)m10card(rxb, rxb_len, 44L, 4L)*1.E-6;
         lon = (double)(int32_t)m10card(rxb, rxb_len, 48L, 4L)*1.E-6;
         alt = (double)m10card(rxb, rxb_len, 24L, 3L)*0.01;
         ci = (int32_t)m10card(rxb, rxb_len, 27L, 2L);
         if (ci>32767L) ci -= 65536L;
         ve = (double)ci*0.01;
         ci = (int32_t)m10card(rxb, rxb_len, 29L, 2L);
         if (ci>32767L) ci -= 65536L;
         vn = (double)ci*0.01;
         ci = (int32_t)m10card(rxb, rxb_len, 40L, 2L);
         if (ci>32767L) ci -= 65536L;
         vv = (double)ci*0.01;
         v = sqrt(ve*ve+vn*vn); /* hor speed */
         dir = atang2(vn, ve)*5.7295779513082E+1;
         if (dir<0.0) dir = 360.0+dir;
         nameok = 1;
         memset((char *)fullid,(char)0,21UL);
         /*
               FOR i:=0 TO 8 DO
                 fullid[i]:=rxb[7+i];
                 IF ((i<8) OR (nam[i]<>0C)) & ((nam[i]<=" ") OR (nam[i]>"Z"))
                 THEN nameok:=FALSE END;
               END;
         */
         i = m10card(rxb, rxb_len, 34L, 1L); /* 002-2-xxxxx 911-2-xxxxx */
         ii = m10rcard(rxb, rxb_len, 35L, 2L);
         fullid[0U] = (char)((i&127UL)/12UL+48UL);
         fullid[1U] = (char)(((i&127UL)%12UL+1UL)/10UL+48UL);
         fullid[2U] = (char)(((i&127UL)%12UL+1UL)%10UL+48UL);
         fullid[3U] = '-';
         fullid[4U] = (char)(i/128UL+(ii&3UL)*2UL+49UL);
         fullid[5U] = '-';
         i = ii/4UL;
         fullid[6U] = (char)((i/10000UL)%10UL+48UL);
         fullid[7U] = (char)((i/1000UL)%10UL+48UL);
         fullid[8U] = (char)((i/100UL)%10UL+48UL);
         fullid[9U] = (char)((i/10UL)%10UL+48UL);
         fullid[10U] = (char)(i%10UL+48UL);
         fullid[11U] = 0;
         if ((fnum&7UL)==0UL) {
            /* time by time show all 3 id bytes for debug */
            cb[0U] = '[';
            cb[1U] = hex((uint32_t)(uint8_t)rxb[34UL]/16UL);
            cb[2U] = hex((uint32_t)(uint8_t)rxb[34UL]);
            cb[3U] = hex((uint32_t)(uint8_t)rxb[35UL]/16UL);
            cb[4U] = hex((uint32_t)(uint8_t)rxb[35UL]);
            cb[5U] = hex((uint32_t)(uint8_t)rxb[36UL]/16UL);
            cb[6U] = hex((uint32_t)(uint8_t)rxb[36UL]);
            cb[7U] = ']';
            cb[8U] = 0;
            aprsstr_Append(fullid, 21ul, cb, 10ul);
         }
      }
      else {
         lat = (double)(int32_t)m10card(rxb, rxb_len, 30L,
                4L)*8.3819036711397E-8;
         lon = (double)(int32_t)m10card(rxb, rxb_len, 34L,
                4L)*8.3819036711397E-8;
         alt = (double)(int32_t)m10card(rxb, rxb_len, 38L, 4L)*0.001;
         ci = (int32_t)m10card(rxb, rxb_len, 20L, 2L);
         if (ci>32767L) ci -= 65536L;
         ve = (double)ci*0.005;
         ci = (int32_t)m10card(rxb, rxb_len, 22L, 2L);
         if (ci>32767L) ci -= 65536L;
         vn = (double)ci*0.005;
         ci = (int32_t)m10card(rxb, rxb_len, 24L, 2L);
         if (ci>32767L) ci -= 65536L;
         vv = (double)ci*0.005;
         v = sqrt(ve*ve+vn*vn); /* hor speed */
         dir = atang2(vn, ve)*5.7295779513082E+1;
         if (dir<0.0) dir = 360.0+dir;
         /* full id */
         i = (uint32_t)(uint8_t)rxb[111UL];
         fullid[0U] = '0';
         i = i&15UL;
         fullid[1U] = (char)(i/10UL+48UL);
         fullid[2U] = (char)(i%10UL+48UL);
         fullid[3U] = '-';
         /*      fullid[0]:=CHR(ORD(rxb[FH+93+2]) DIV 16+ORD("0")); */
         /*      fullid[1]:="0"; */
         /*      fullid[2]:=hex(ORD(rxb[FH+93+2])); */
         fullid[4U] = hex((uint32_t)(uint8_t)rxb[109UL]);
         i = (uint32_t)(uint8_t)rxb[112UL]+(uint32_t)(uint8_t)
                rxb[113UL]*256UL;
         fullid[5U] = '-';
         fullid[6U] = (char)((i/8192UL&7UL)+48UL);
         i = i&8191UL;
         fullid[7U] = (char)((i/1000UL)%10UL+48UL);
         fullid[8U] = (char)((i/100UL)%10UL+48UL);
         fullid[9U] = (char)((i/10UL)%10UL+48UL);
         fullid[10U] = (char)(i%10UL+48UL);
         fullid[11U] = 0;
         /*- m10 temp */
         sct = m10rcard(rxb, rxb_len, 78L, 1L);
         rt = (float)(m10rcard(rxb, rxb_len, 79L, 2L)&4095UL);
         if (rt!=0.0f && sct<3UL) {
            rt = X2C_DIVR(4095.0f-rt,rt)-_cnst6[sct];
            if (rt>0.0f) {
               rt = X2C_DIVR(_cnst7[sct],rt);
               if (rt>0.0f) {
                  rt = (float)log((double)rt);
                  rt = X2C_DIVR(1.0f,
                1.07303516E-3f+2.41296733E-4f*rt+2.26744154E-6f*rt*rt+6.52855181E-8f*rt*rt*rt)
                -273.15f;
                  if (rt>(-99.0f) && rt<50.0f) rtok = rt;
               }
            }
         }
         /*- m10 temp */
         batt = (float)m10rcard(rxb, rxb_len, 85L, 2L)*0.00668f;
                /* not checkt */
      }
      if (sondeaprs_verb) {
         osi_WrStr(nam, 9ul);
         osi_WrStr(" ", 2ul);
         osic_WrINT32(frameno, 1UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)lat, 5L, 1UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)lon, 5L, 1UL);
         osi_WrStr(" ", 2ul);
         osic_WrFixed((float)alt, 1L, 1UL);
         osi_WrStr("m ", 3ul);
         osic_WrFixed((float)(v*3.6), 1L, 1UL);
         osi_WrStr("km/h ", 6ul);
         osic_WrFixed((float)dir, 0L, 1UL);
         osi_WrStr("deg ", 5ul);
         osic_WrFixed((float)vv, 1L, 1UL);
         osi_WrStr("m/s sats ", 10ul);
         osic_WrINT32(sats, 1UL);
         osi_WrStr(" batt ", 7ul);
         osic_WrFixed(batt, 2L, 1UL);
         if (rtok<100.0f) {
            osi_WrStr(" ", 2ul);
            osic_WrFixed(rt, 1L, 1UL);
            osi_WrStr("C", 2ul);
         }
         osi_WrStr(" ser=", 6ul);
         osi_WrStr(fullid, 21ul);
      }
   }
   else if (sondeaprs_verb) {
      osi_WrStr("crc error", 10ul);
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
   if ((((pc && nameok) && calok) && lat!=0.0) && lon!=0.0) {
      xdatablock.cnt = 0UL;
      sondeaprs_senddata(lat*1.7453292519943E-2, lon*1.7453292519943E-2, alt,
                 v, dir, vv, 0.0, 0.0, (double)rtok, 0.0, 0.0, 0.0,
                0.0, (double) -(float)(uint32_t)sendmhzfromsdr,
                0.0, 0.0, pc->gpssecond, 0UL, pc->name, 9ul, 0UL, sats, 0UL,
                (double)batt, usercall, 11ul, 0UL, 0.0,
                sondeaprs_nofilter, 0, 0L, wid, 4ul, fullid, 21ul, 0,
                sdrblock, (int32_t)gpstimecorr, xdatablock);
      pc->framesent = 1;
   }
} /* end decodem10() */

/*------------------------------ IMET */
#define sondemod_DT 86400


static void sysdatetotime(uint32_t * t)
/* add system date to daytime */
{
   uint32_t st;
   int32_t d;
   st = osic_time();
   d = (int32_t)(st%86400UL)-(int32_t)*t;
   if (d<-43200L) d += 86400L;
   else if (d>43200L) d -= 86400L;
   *t = st-(uint32_t)d;
} /* end sysdatetotime() */


static uint32_t imetcard(const char b[], uint32_t b_len,
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
} /* end imetcard() */

#define sondemod_FH0 16
/* size of header before payload */

#define sondemod_POLY 0x1021 


static void decodeimet(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   uint32_t ii;
   uint32_t j;
   uint32_t i;
   double ozonval;
   double alt;
   double long0;
   double lat;
   uint32_t dayt;
   uint32_t dt;
   OBJNAME nam;
   char s[1001];
   CALLSSID usercall;
   char typ;
   uint16_t crc;
   uint32_t gpstime;
   uint32_t p;
   uint32_t len;
   uint32_t sats;
   float vy;
   float vx;
   float oodt;
   char nameok;
   char calok;
   char crcok;
   pCONTEXTIMET pc0;
   pCONTEXTIMET pc1;
   pCONTEXTIMET pc;
   uint32_t tmp;
   calok = 0;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   /*  speed:=MAX(REAL); */
   /*  clb:=MAX(REAL); */
   getcall(rxb, rxb_len, usercall, 11ul);
   ozonval = 0.0;
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
   if (sondeaprs_verb) osi_WrStr("iMET ", 6ul);
   gpstime = 0UL;
   nameok = 1;
   for (i = 0UL; i<=8UL; i++) {
      nam[i] = rxb[7UL+i];
      if ((i<8UL || nam[i]) && ((uint8_t)nam[i]<=' ' || (uint8_t)
                nam[i]>'Z')) nameok = 0;
   } /* end for */
   pc = pcontextimet;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextimet = pc1;
         else pc0->next = pc1;
         osic_free((char * *) &pc, sizeof(struct CONTEXTIMET));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTIMET));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTIMET));
      pc->next = pcontextimet;
      pcontextimet = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      pc->rtok = X2C_max_real;
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   p = 16UL;
   for (;;) {
      if (p+2UL>rxb_len-1) break;
      typ = rxb[p+1UL];
      switch ((unsigned)typ) {
      case '\001':
         len = 12UL;
         break;
      case '\002':
         len = 18UL;
         break;
      case '\003':
         len = 5UL+(uint32_t)(uint8_t)rxb[p+2UL];
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
      if (len<=2UL) break;
      /*- crc */
      crc = 0x1D0FU;
      tmp = len-3UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         crc = crc^X2C_LSH((uint16_t)(uint32_t)(uint8_t)rxb[p+i],16,8);
         for (j = 0UL; j<=7UL; j++) {
            if ((0x8000U & crc)) crc = X2C_LSH(crc,16,1)^0x1021U;
            else crc = X2C_LSH(crc,16,1);
         } /* end for */
         if (i==tmp) break;
      } /* end for */
      /* crc */
      crcok = crc==(uint16_t)((uint32_t)(uint8_t)rxb[(p+len)
                -2UL]*256UL+(uint32_t)(uint8_t)rxb[(p+len)-1UL]);
      if (crcok) {
         if (typ=='\002' || typ=='\005') {
            /* gps */
            if (typ=='\005') ii = 25UL;
            else ii = 13UL;
            gpstime = imetcard(rxb, rxb_len, p+ii, 1UL)*3600UL+imetcard(rxb,
                rxb_len, p+ii+1UL, 1UL)*60UL+imetcard(rxb, rxb_len, p+ii+2UL,
                 1UL);
            gpstime = (uint32_t)(((int32_t)gpstime+86400L)-leapseconds)
                %86400UL; /* seems like raw gps time with leapseconds */
            dayt = gpstime;
            sysdatetotime(&gpstime);
            lat = (double)SaveReal(imetcard(rxb, rxb_len, p+2UL, 4UL));
            long0 = (double)SaveReal(imetcard(rxb, rxb_len, p+6UL,
                4UL));
            alt = (double)(float)((int32_t)imetcard(rxb, rxb_len,
                p+10UL, 2UL)-5000L);
            sats = imetcard(rxb, rxb_len, p+12UL, 1UL); /* always 16? */
            if (typ=='\005') {
               /* gps with speeds */
               vx = SaveReal(imetcard(rxb, rxb_len, p+13UL, 4UL));
               vy = SaveReal(imetcard(rxb, rxb_len, p+17UL, 4UL));
               pc->dir = (float)(X2C_DIVL(atang2((double)vy,
                (double)vx),1.7453292519943E-2));
               if (pc->dir<0.0f) pc->dir = 360.0f+pc->dir;
               pc->speed = (float)sqrt((double)(vx*vx+vy*vy));
               pc->clb = SaveReal(imetcard(rxb, rxb_len, p+21UL, 4UL));
            }
            else {
               /* speeds out of positions */
               dt = ((dayt+86400UL)-pc->talt)%86400UL;
                /* seconds since last frame */
               if (dt>0UL) {
                  /* clb out of altitudes */
                  oodt = X2C_DIVR(1.0f,(float)dt);
                  pc->clb = (float)((double)
                pc->clb+((alt-(double)pc->alt)*(double)
                oodt-(double)pc->clb)*0.25);
                  vx = (float)((long0-pc->long0)*(double)
                oodt*cos(lat*1.7453292519943E-2));
                  vy = (float)((lat-pc->lat)*(double)oodt);
                  pc->dir = (float)(X2C_DIVL(atang2((double)vy,
                (double)vx),1.7453292519943E-2));
                  if (pc->dir<0.0f) pc->dir = 360.0f+pc->dir;
                  pc->speed = (float)((double)
                pc->speed+(sqrt((double)(vx*vx+vy*vy))
                *1.11111E+5-(double)pc->speed)*0.25); /* km/h */
               }
            }
            pc->alt = (float)alt;
            pc->talt = dayt;
            pc->lat = lat;
            pc->long0 = long0;
         }
         else if (typ=='\001' || typ=='\004') {
            /* ptu */
            pc->frnum = imetcard(rxb, rxb_len, p+2UL, 2UL);
            pc->hpa = (float)imetcard(rxb, rxb_len, p+4UL, 3UL)*0.01f;
            pc->rtok = (float)(short)imetcard(rxb, rxb_len, p+7UL,
                2UL)*0.01f;
            pc->hum = (float)imetcard(rxb, rxb_len, p+9UL, 2UL)*0.01f;
            pc->vbatt = (float)imetcard(rxb, rxb_len, p+11UL, 1UL)*0.1f;
         }
         else if (typ=='\003') {
            /*
                    IF typ=4C THEN                                          (* extended ptu *)
                      WrStr(" ti:");
                WrFixed(FLOAT(imetcard(rxbuf, p+12, 2))*0.01, 2,1);
                      WrStr(" tp:");
                WrFixed(FLOAT(imetcard(rxbuf, p+14, 2))*0.01, 2,1);
                      WrStr(" tu:");
                WrFixed(FLOAT(imetcard(rxbuf, p+16, 2))*0.01, 2,1);
                    END;
            */
            /* extensions */
            if (len==13UL) {
               /* may be ozone */
               /*        WrStr(" otyp:"); WrInt(imetcard(rxbuf, p+3, 1), 1);
                */
               /*        WrStr(" onum:"); WrInt(imetcard(rxbuf, p+4, 1), 1);
                */
               /*        WrStr(" oi:");
                WrFixed(FLOAT(imetcard(rxbuf, p+5, 2))*0.001); */
               pc->ozoneuA = (float)imetcard(rxb, rxb_len, p+5UL,
                2UL)*0.001f;
               pc->otemp = (float)(short)imetcard(rxb, rxb_len, p+7UL,
                 2UL)*0.01f;
               pc->pumpmA = imetcard(rxb, rxb_len, p+9UL, 1UL);
               pc->pumpV = (float)imetcard(rxb, rxb_len, p+10UL,
                1UL)*0.1f;
               if (((pc->ozoneuA>0.0f && pc->hpa>0.0f) && pc->otemp>(-5.0f))
                && pc->otemp<50.0f) {
                  ozonval = (double)calcOzone((double)
                pc->ozoneuA, (double)pc->otemp, (double)pc->hpa);
                 /* very experimental,
                guess Mast/Keystone ozonsensor with unknown airflow */
               }
            }
            else if (sondeaprs_verb) osi_WrStr("unknown frame", 14ul);
         }
      }
      else if (sondeaprs_verb) osi_WrStr(" crc err", 9ul);
      p += len;
   }
   pc->gpssecond = gpstime;
   if (pc->frnum>pc->framenum) {
      /* new frame number */
      pc->framesent = 0;
      pc->framenum = pc->frnum;
      pc->tused = systime;
      calok = 1;
   }
   else if (pc->framenum==pc->frnum) {
      if (!pc->framesent) calok = 1;
   }
   else if (sondeaprs_verb) {
      osi_WrStr(" got old frame ", 16ul);
      osic_WrINT32(pc->frnum, 1UL);
      osi_WrStr(" expected> ", 12ul);
      osic_WrINT32(pc->framenum, 1UL);
      osi_WrStr(" ", 2ul);
   }
   if (sondeaprs_verb) {
      osi_WrStr(nam, 9ul);
      osi_WrStr(" ", 2ul);
      osic_WrINT32(pc->frnum, 1UL);
      osi_WrStr(" ", 2ul);
      if (ozonval>0.0) {
         osic_WrFixed((float)ozonval, 1L, 1UL);
         osi_WrStr("mPa ", 5ul);
      }
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
   if ((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) {
      xdatablock.cnt = 0UL;
      sondeaprs_senddata(lat*1.7453292519943E-2, long0*1.7453292519943E-2,
                alt, (double)pc->speed, (double)pc->dir,
                (double)pc->clb, 0.0, (double)pc->hum,
                (double)pc->rtok, ozonval, (double)pc->otemp,
                (double)pc->pumpmA, (double)pc->pumpV,
                (double) -(float)(uint32_t)sendmhzfromsdr, 0.0,
                0.0, pc->gpssecond, pc->frnum, pc->name, 9ul, 0UL, sats, 0UL,
                 (double)pc->vbatt, usercall, 11ul, 0UL,
                (double)pc->hpa, sondeaprs_nofilter, 1, 0L, "iMET",
                5ul, "", 1ul, 0, sdrblock, leapseconds, xdatablock);
      pc->framesent = 1;
   }
} /* end decodeimet() */

/*------------------------------ MP3-H1 */

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

#define sondemod_POLY0 0xA001 


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


static int32_t cint16(int32_t v)
{
   if (v>32767L) v -= 65536L;
   return v;
} /* end cint16() */

#define sondemod_FH1 20
/* size of header before payload */


static void decodemp3(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   uint32_t i;
   double clb;
   double dir;
   double kmh;
   double alt;
   double long0;
   double lat;
   double vz;
   double vy;
   double vx;
   double wz;
   double wy;
   double wx;
   char s[101];
   OBJNAME nam;
   CALLSSID usercall;
   uint32_t cfg;
   uint32_t cnt;
   uint32_t sats;
   char nameok;
   char calok;
   pCONTEXTMP3 pc0;
   pCONTEXTMP3 pc1;
   pCONTEXTMP3 pc;
   calok = 0;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   sats = 0UL;
   getcall(rxb, rxb_len, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 101ul);
      osi_WrStr(s, 101ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   nameok = 1;
   for (i = 0UL; i<=8UL; i++) {
      nam[i] = rxb[7UL+i];
      if ((i<8UL || nam[i]) && ((uint8_t)nam[i]<=' ' || (uint8_t)
                nam[i]>'Z')) nameok = 0;
   } /* end for */
   pc = pcontextmp3;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextmp3 = pc1;
         else pc0->next = pc1;
         osic_free((char * *) &pc, sizeof(struct CONTEXTMP3));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTMP3));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTMP3));
      pc->next = pcontextmp3;
      pcontextmp3 = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   if (sondeaprs_verb) {
      osi_WrStr("MRZ ", 5ul);
      osi_WrStr(nam, 9ul);
      osi_WrStr(" ", 2ul);
   }
   if (crcmp3(rxb, rxb_len, 20UL, 65UL)) {
      cnt = (uint32_t)(uint8_t)rxb[20UL]&15UL;
      cfg = cardmsb(rxb, rxb_len, 61UL, 4UL);
      if (cnt==13UL) {
         pc->id2 = cfg;
         pc->id2ok = 1;
      }
      else if (cnt==12UL) {
         pc->id1 = cfg;
         pc->id1ok = 1;
      }
      if (pc->id2ok && pc->id1ok) {
         /*      pc^.ser:="MRZ-"; */
         aprsstr_CardToStr(pc->id1, 1UL, pc->ser, 21ul);
         /*      Append(pc^.ser,s); */
         aprsstr_Append(pc->ser, 21ul, "-", 2ul);
         aprsstr_CardToStr(pc->id2, 1UL, s, 101ul);
         aprsstr_Append(pc->ser, 21ul, s, 101ul);
      }
      pc->gpstime = (uint32_t)(uint8_t)rxb[16UL]*16777216UL+(uint32_t)
                (uint8_t)rxb[17UL]*65536UL+(uint32_t)(uint8_t)
                rxb[18UL]*256UL+(uint32_t)(uint8_t)rxb[19UL];
      if (sondeaprs_verb) {
         osic_WrUINT32(pc->gpstime, 1UL);
         osi_WrStr(" c=", 4ul);
         osic_WrINT32(cnt, 1UL);
      }
      wx = (double)(int32_t)cardmsb(rxb, rxb_len, 25UL, 4UL)*0.01;
      wy = (double)(int32_t)cardmsb(rxb, rxb_len, 29UL, 4UL)*0.01;
      wz = (double)(int32_t)cardmsb(rxb, rxb_len, 33UL, 4UL)*0.01;
      vx = (double)cint16((int32_t)cardmsb(rxb, rxb_len, 37UL,
                2UL))*0.01;
      vy = (double)cint16((int32_t)cardmsb(rxb, rxb_len, 39UL,
                2UL))*0.01;
      vz = (double)cint16((int32_t)cardmsb(rxb, rxb_len, 41UL,
                2UL))*0.01;
      wgs84r(wx, wy, wz, &lat, &long0, &alt);
      speeddir(lat, long0, vx, vy, vz, &kmh, &dir, &clb);
      sats = (uint32_t)(uint8_t)rxb[43UL];
   }
   else if (sondeaprs_verb) {
      /*
          IF verb THEN 
            WrFixed(lat/RAD, 5,1);
            WrStr(" "); WrFixed(long/RAD, 5,1);
            IF (alt<100000.0) & (alt>-100000.0) THEN
              WrStr(" "); WrInt(VAL(INTEGER, alt), 1); WrStr("m");
            END;
            WrStr(" "); WrFixed(kmh, 1,1); WrStr("km/h ");
            WrInt(VAL(INTEGER, dir), 1); WrStr("deg ");
            WrFixed(clb, 1,1); WrStr("m/s");
          END;
      */
      osi_WrStr(" crc err", 9ul);
   }
   if (pc->gpstime>pc->lastgpstime) {
      /* new frame number */
      pc->framesent = 0;
      pc->lastgpstime = pc->gpstime;
      pc->tused = systime;
      calok = 1;
   }
   else if (pc->lastgpstime==pc->gpstime) {
      if (!pc->framesent) calok = 1;
   }
   else if (sondeaprs_verb) {
      osi_WrStr(" got old frame ", 16ul);
      osic_WrINT32(pc->gpstime, 1UL);
      osi_WrStr(" expected> ", 12ul);
      osic_WrINT32(pc->lastgpstime, 1UL);
      osi_WrStr(" ", 2ul);
   }
   if ((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) {
      xdatablock.cnt = 0UL;
      sondeaprs_senddata(lat, long0, alt, kmh*2.7777777777778E-1, dir, clb,
                0.0, (double)X2C_max_real, (double)X2C_max_real,
                0.0, 0.0, 0.0, 0.0,
                (double) -(float)(uint32_t)sendmhzfromsdr, 0.0,
                0.0, pc->gpstime, 0UL, pc->name, 9ul, 0UL, sats, 0UL, 0.0,
                usercall, 11ul, 0UL, (double)X2C_max_real,
                sondeaprs_nofilter, 1, 0L, "MRZ", 4ul, pc->ser, 21ul, 0,
                sdrblock, 0L, xdatablock);
      pc->framesent = 1;
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
} /* end decodemp3() */

/*------------------------------ MEISEI */
#define sondemod_FH2 24
/* size of header before payload */

#define sondemod_CHKSET 20
/* checksum ok bits for subframes and frames joined bit 7 */

#define sondemod_TYPDISC 0xC0 

#define sondemod_SYNWORDMEIS1 0x49DCE 

#define sondemod_SYNWORDMEIS2 0xFB6230 


static void decodemeisei(const char rxb[], uint32_t rxb_len,
                uint32_t ip, uint32_t fromport)
{
   uint32_t timedif;
   uint32_t fcnt;
   uint32_t frametyp;
   uint32_t j;
   uint32_t i;
   double dir;
   double kmh;
   double dalt;
   double oalt;
   double long0;
   double lat;
   char s[101];
   char typname[13];
   OBJNAME nam;
   CALLSSID usercall;
   CALLSSID ser;
   char nameok;
   char calok;
   pCONTEXTMEISEI pc0;
   pCONTEXTMEISEI pc1;
   pCONTEXTMEISEI pc;
   char tmp;
   pc = 0;
   lat = 0.0;
   long0 = 0.0;
   kmh = 0.0;
   dir = 0.0;
   getcall(rxb, rxb_len, usercall, 11ul);
   if (usercall[0U]==0) aprsstr_Assign(usercall, 11ul, mycall, 100ul);
   if (sondeaprs_verb && fromport>0UL) {
      osi_WrStr("UDP:", 5ul);
      aprsstr_ipv4tostr(ip, s, 101ul);
      osi_WrStr(s, 101ul);
      osi_WrStr(":", 2ul);
      osic_WrINT32(fromport, 1UL);
      if (usercall[0U]) {
         osi_WrStr(" (", 3ul);
         osi_WrStr(usercall, 11ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   nameok = 1;
   for (i = 0UL; i<=8UL; i++) {
      nam[i] = rxb[7UL+i];
      if ((uint8_t)nam[i]<=' ' || (uint8_t)nam[i]>'Z') nameok = 0;
   } /* end for */
   pc = pcontextmeisei;
   pc0 = 0;
   for (;;) {
      if (pc==0) break;
      pc1 = pc->next;
      if (pc->tused+3600UL<systime) {
         /* timed out */
         if (pc0==0) pcontextmeisei = pc1;
         else pc0->next = pc1;
         osic_free((char * *) &pc, sizeof(struct CONTEXTMEISEI));
      }
      else {
         if (aprsstr_StrCmp(nam, 9ul, pc->name, 9ul)) break;
         pc0 = pc;
      }
      pc = pc1;
   }
   if (pc==0) {
      osic_alloc((char * *) &pc, sizeof(struct CONTEXTMEISEI));
      if (pc==0) Error("allocate context out im memory", 31ul);
      memset((char *)pc,(char)0,sizeof(struct CONTEXTMEISEI));
      pc->next = pcontextmeisei;
      pcontextmeisei = pc;
      aprsstr_Assign(pc->name, 9ul, nam, 9ul);
      if (sondeaprs_verb) osi_WrStrLn("is new ", 8ul);
   }
   if (sondeaprs_verb) {
      osi_WrStr("Meisei ", 8ul);
      osi_WrStr(nam, 9ul);
      osi_WrStr(" ", 2ul);
   }
   frametyp = 4UL;
   i = (uint32_t)(uint8_t)rxb[21UL]*65536UL+(uint32_t)(uint8_t)
                rxb[22UL]*256UL+(uint32_t)(uint8_t)rxb[23UL];
   if (302542UL==i) frametyp = 0UL;
   else if (16474672UL==i) frametyp = 1UL;
   if ((0x1U & (uint8_t)(uint8_t)rxb[20UL]) && frametyp<=1UL) {
      /* header found */
      /*-frametype */
      fcnt = (uint32_t)(uint8_t)rxb[24UL]*256UL+(uint32_t)(uint8_t)
                rxb[25UL]; /* framenum or date */
      /*WrStr("<<");WrInt(i,1);WrStr(">>");WrInt(frametyp,1);WrStr(">>"); */
      if (frametyp==1UL) {
         if (fcnt==0UL) frametyp = 3UL;
      }
      else {
         pc->fnum = fcnt/2UL;
         if ((fcnt&1)) frametyp = 2UL;
      }
      if (sondeaprs_verb) {
         osi_WrStr((char *)(tmp = (char)(65UL+frametyp),&tmp),
                1u/1u);
         if ((0x80U & (uint8_t)(uint8_t)rxb[20UL])) osi_WrStr("*", 2ul);
         else osi_WrStr("/", 2ul);
         for (i = 0UL; i<=5UL; i++) {
            if (X2C_IN(i,8,(uint8_t)(uint8_t)rxb[20UL])) {
               osi_WrStr("+", 2ul);
            }
            else osi_WrStr("-", 2ul);
         } /* end for */
      }
      if (sondeaprs_verb) {
         osi_WrStr(" ", 2ul);
         osic_WrINT32(pc->fnum, 1UL);
      }
      j = (uint32_t)(uint8_t)rxb[16UL]*16777216UL+(uint32_t)(uint8_t)
                rxb[17UL]*65536UL+(uint32_t)(uint8_t)
                rxb[18UL]*256UL+(uint32_t)(uint8_t)rxb[19UL];
      if (j<=pc->gpstime) timedif = 1UL;
      else timedif = j-pc->gpstime;
      pc->gpstime = j;
      if (sondeaprs_verb) {
         osi_WrStr(" ", 2ul);
         wrdate(pc->gpstime);
      }
      /*-subtype */
      if (((uint8_t)(uint8_t)rxb[20UL]&0x9U)==0x9U && (uint32_t)
                (uint8_t)rxb[38UL]==48UL+(uint32_t)(frametyp==2UL)) {
         pc->subtype = 1UL+(uint32_t)((uint32_t)(uint8_t)
                rxb[39UL]<=192UL);
         if (sondeaprs_verb) {
            if (pc->subtype==1UL) osi_WrStr(" ims100", 8ul);
            else {
               osi_WrStr(" ", 2ul);
               osi_WrHex((uint32_t)(uint8_t)rxb[39UL], 1UL);
               osi_WrStr("=type?", 7ul);
            }
         }
      }
      if ((pc->subtype==1UL && frametyp==2UL) && (0x2U & (uint8_t)
                (uint8_t)rxb[20UL])) {
         /* calib */
         if ((fcnt&63UL)==15UL) {
            j = (uint32_t)(uint8_t)rxb[28UL]*256UL+(uint32_t)
                (uint8_t)rxb[29UL]+(uint32_t)(uint8_t)
                rxb[30UL]*16777216UL+(uint32_t)(uint8_t)
                rxb[31UL]*65536UL;
            pc->mhz0 = 400.0f+SaveReal(j)*0.1f;
         }
      }
      if (sondeaprs_verb && pc->mhz0!=0.0f) {
         osi_WrStr(" ", 2ul);
         osic_WrFixed(pc->mhz0, 1L, 1UL);
         osi_WrStr("MHz", 4ul);
      }
      if (frametyp==1UL && ((uint8_t)(uint8_t)rxb[20UL]&0xBFU)==0xBFU) {
         for (i = 0UL; i<=10UL; i++) {
            pc->gpssum += (uint32_t)(uint8_t)
                rxb[24UL+i*2UL]*256UL+(uint32_t)(uint8_t)rxb[25UL+i*2UL];
         } /* end for */
         if ((pc->gpssum&65535UL)==(uint32_t)(uint8_t)
                rxb[46UL]*256UL+(uint32_t)(uint8_t)rxb[47UL]) {
            j = 0UL;
            for (i = 0UL; i<=3UL; i++) {
               j = j*256UL+(uint32_t)(uint8_t)rxb[26UL+i];
            } /* end for */
            if (pc->subtype==1UL) {
               lat = ((double)(j/1000000UL)+(double)(j%1000000UL)
                *1.6666666666667E-6)*1.7453292519943E-2;
            }
            else lat = (double)j*1.7453292519943E-9;
            j = 0UL;
            for (i = 0UL; i<=3UL; i++) {
               j = j*256UL+(uint32_t)(uint8_t)rxb[30UL+i];
            } /* end for */
            if (pc->subtype==1UL) {
               long0 = ((double)(j/1000000UL)+(double)
                (j%1000000UL)*1.6666666666667E-6)*1.7453292519943E-2;
            }
            else long0 = (double)j*1.7453292519943E-9;
            j = 0UL;
            if (pc->subtype==1UL) {
               for (i = 0UL; i<=2UL; i++) {
                  j = j*256UL+(uint32_t)(uint8_t)rxb[34UL+i];
               } /* end for */
            }
            else {
               for (i = 0UL; i<=3UL; i++) {
                  j = j*256UL+(uint32_t)(uint8_t)rxb[34UL+i];
               } /* end for */
            }
            oalt = (double)j*0.01;
            dalt = X2C_DIVL(oalt-(double)pc->alt,
                (double)timedif);
            if (dalt<20.0 && dalt>(-200.0)) {
               pc->clb = (float)((double)
                pc->clb+(dalt-(double)pc->clb)*0.25);
            }
            pc->alt = (float)oalt;
            if (pc->subtype==1UL) {
               dir = (double)((uint32_t)(uint8_t)
                rxb[42UL]*256UL+(uint32_t)(uint8_t)rxb[43UL])*0.01;
            }
            else {
               dir = (double)((uint32_t)(uint8_t)
                rxb[40UL]*256UL+(uint32_t)(uint8_t)rxb[41UL])*0.01;
            }
            if (pc->subtype==1UL) {
               kmh = (double)((uint32_t)(uint8_t)
                rxb[44UL]*256UL+(uint32_t)(uint8_t)rxb[45UL])
                *1.851984E-2;
            }
            else {
               kmh = (double)((uint32_t)(uint8_t)
                rxb[38UL]*256UL+(uint32_t)(uint8_t)rxb[39UL])*0.01;
            }
         }
         else if (sondeaprs_verb) osi_WrStr(" checksum error", 16ul);
      }
      else if (frametyp==0UL) {
         pc->gpssum = (uint32_t)(uint8_t)rxb[44UL]*256UL+(uint32_t)
                (uint8_t)rxb[45UL]+(uint32_t)(uint8_t)
                rxb[46UL]*256UL+(uint32_t)(uint8_t)rxb[47UL];
      }
   }
   if (pc->gpstime>pc->lastgpstime) {
      /* new frame number */
      pc->framesent = 0;
      pc->lastgpstime = pc->gpstime;
      pc->tused = systime;
      calok = 1;
   }
   else if (pc->lastgpstime==pc->gpstime) {
      if (!pc->framesent) calok = 1;
   }
   else if (sondeaprs_verb) {
      osi_WrStr(" got old frame ", 16ul);
      osic_WrINT32(pc->gpstime, 1UL);
      osi_WrStr(" expected> ", 12ul);
      osic_WrINT32(pc->lastgpstime, 1UL);
      osi_WrStr(" ", 2ul);
   }
   if (pc->subtype==1UL) strncpy(typname,"IMS100",13u);
   else strncpy(typname,"RS11",13u);
   /*  Assign(ser, pc^.name); */
   /*  Delstr(ser, 0, 3); */
   aprsstr_CardToStr(gethex(pc->name, 9ul, 3UL, 6UL), 1UL, ser, 11ul);
                /* serial num in decimal */
   if ((((pc && nameok) && calok) && lat!=0.0) && long0!=0.0) {
      if (sondeaprs_verb) osi_WrStr(" ", 2ul);
      xdatablock.cnt = 0UL;
      sondeaprs_senddata(lat, long0, (double)pc->alt,
                kmh*2.7777777777778E-1, dir, (double)pc->clb, 0.0,
                (double)X2C_max_real, (double)X2C_max_real, 0.0,
                0.0, 0.0, 0.0, (double)pc->mhz0, 0.0, 0.0, pc->gpstime,
                 pc->fnum, pc->name, 9ul, 0UL, 0UL, 0UL, 0.0, usercall, 11ul,
                 0UL, (double)X2C_max_real, sondeaprs_nofilter, 1, 0L,
                typname, 13ul, ser, 11ul, 0, sdrblock, 0L, xdatablock);
      pc->framesent = 1;
   }
   if (sondeaprs_verb) {
      wrsdr();
      osi_WrStrLn("", 1ul);
   }
} /* end decodemeisei() */


static char readsdrdata(char b[], uint32_t b_len,
                int32_t * len, struct sondeaprs_SDRBLOCK * sdr)
{
   uint32_t p;
   uint32_t n;
   char readsdrdata_ret;
   X2C_PCOPY((void **)&b,b_len);
   memset((char *)sdr,(char)0,sizeof(struct sondeaprs_SDRBLOCK));
   if (*len<4L) {
      readsdrdata_ret = 0;
      goto label;
   }
   n = (uint32_t)(uint8_t)b[*len-4L]*256UL+(uint32_t)(uint8_t)
                b[*len-3L];
   if ((int32_t)n>=*len) {
      readsdrdata_ret = 0;
      goto label;
   }
   if (n!=(255UL-(uint32_t)(uint8_t)b[*len-2L])*256UL+(255UL-(uint32_t)
                (uint8_t)b[*len-1L])) {
      readsdrdata_ret = 0;
      goto label;
   }
   p = n;
   do {
      if (b[p]=='f') {
         sdr->freq = (uint32_t)(uint8_t)b[p+1UL]*16777216UL+(uint32_t)
                (uint8_t)b[p+2UL]*65536UL+(uint32_t)(uint8_t)
                b[p+3UL]*256UL+(uint32_t)(uint8_t)b[p+4UL];
         sdr->valid = 1;
      }
      else if (b[p]=='a') {
         sdr->afc = (int32_t)(short)((uint32_t)(uint8_t)
                b[p+3UL]*256UL+(uint32_t)(uint8_t)b[p+4UL]);
         sdr->maxafc = (uint32_t)(uint8_t)b[p+1UL]*256UL+(uint32_t)
                (uint8_t)b[p+2UL];
         sdr->valid = 1;
      }
      else if (b[p]=='r') {
         sdr->db = (uint32_t)(uint8_t)b[p+1UL]*16777216UL+(uint32_t)
                (uint8_t)b[p+2UL]*65536UL+(uint32_t)(uint8_t)
                b[p+3UL]*256UL+(uint32_t)(uint8_t)b[p+4UL];
         sdr->valid = 1;
      }
      else if (b[p]=='n') {
         sdr->name[0UL] = b[p+1UL];
         sdr->name[1UL] = b[p+2UL];
         sdr->name[2UL] = b[p+3UL];
         sdr->name[3UL] = b[p+4UL];
         sdr->valid = 1;
      }
      p += 5UL;
   } while ((int32_t)p<*len);
   *len = (int32_t)n;
   readsdrdata_ret = 1;
   label:;
   X2C_PFREE(b);
   return readsdrdata_ret;
} /* end readsdrdata() */


static void udprx(void)
{
   uint32_t fromport;
   uint32_t ip;
   int32_t len;
   char done;
   len = udpreceive(rxsock, chan[sondemod_LEFT].rxbuf, 560L, &fromport, &ip);
   systime = osic_time();
   sdrblock.valid = 0;
   done = 0;
   if (len>0L) {
      for (;;) {
         if (len==240L) {
            decodeframe(sondemod_LEFT, ip, fromport);
            break;
         }
         if (len==22L) {
            decodec34(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (len==37L) {
            decodedfm6(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (len==520L) {
            decoders41(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (len==117L) {
            decodem10(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (len==122L) {
            decodeimet(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (len==69L) {
            decodemp3(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (len==48L) {
            decodemeisei(chan[sondemod_LEFT].rxbuf, 560ul, ip, fromport);
            break;
         }
         if (done || !readsdrdata(chan[sondemod_LEFT].rxbuf, 560ul, &len,
                &sdrblock)) break;
         done = 1;
      }
   }
   else usleep(10000UL);
} /* end udprx() */

static uint16_t sondemod_POLY1 = 0x1021U;


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

static void jsonpipebroken(int32_t);


static void jsonpipebroken(int32_t signum)
{
   osi_WrStr("got signal ", 12ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
} /* end jsonpipebroken() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(SET51)!=8) X2C_ASSERT(0);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(OBJNAME)!=9) X2C_ASSERT(0);
   if (sizeof(CALLSSID)!=11) X2C_ASSERT(0);
   aprspos_BEGIN();
   libsrtm_BEGIN();
   sondeaprs_BEGIN();
   gpspos_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGPIPE, jsonpipebroken);
   dftypes = 0;
   Parms();
   /*  initrsc; */
   Gencrctab();
   initcontext(&contextr9);
   pcontextc = 0;
   pcontextdfm6 = 0;
   pcontextr4 = 0;
   pcontextm10 = 0;
   pcontextimet = 0;
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
