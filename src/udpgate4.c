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
#define udpgate4_C_
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* axudp / tcpip  aprs-is gateway and message repeater by OE5DXL */
/*FROM Storage IMPORT ALLOCATE, DEALLOCATE; */
/*FROM TimeConv IMPORT time;  */
/*
FROM stat IMPORT fstat, stat_t;
*/
#define udpgate4_CALLLEN 7

#define udpgate4_HASHSIZE 65536

#define udpgate4_VERS "udpgate 0.68"

#define udpgate4_TOCALL "APNL51"

#define udpgate4_HTTPTIMEOUT 60

#define udpgate4_APRSLISTENSOCKS 4
/* max same time tcp connects */

#define udpgate4_WWWLISTENSOCKS 16
/* max same time www tcp connects */

#define udpgate4_UDPSHOWTIME 600
/* show udp in html */

#define udpgate4_GATECONNDELAY 30

#define udpgate4_MAXINTERNALDELAY 5
/* max seconds between udp ports read to not discard data */

#define udpgate4_DEFAULTPONGTIME 30
/* s after last #ping stop datatransfer */

#define udpgate4_OLDMSGRETRY 4

typedef char MONCALL[10];

typedef char FILENAME[1024];

typedef char FRAMEBUF[512];

typedef char FILTERST[256];

/*
      FRAMEBUF=ARRAY[0..256+3+11*10] OF CHAR;
*/
typedef uint32_t CHSET[4];


struct BEACON {
   uint32_t bintervall;
   FILENAME bfile;
   uint32_t btime;
};

struct POSCALL;


struct POSCALL {
   MONCALL call;
   char typ0;
   struct aprsstr_POSITION pos;
};

struct FILTERS;


struct FILTERS {
   char typ0;
   struct aprsstr_POSITION base;
   struct aprsstr_POSITION edge;
   float radius;
   MONCALL viacalls[8];
   char notvia;
   MONCALL entrycalls[8];
   char notentry;
   MONCALL prefixes[8];
   char notprefix;
   MONCALL bud[8];
   char notbud;
   MONCALL objects[8];
   char notobject;
   MONCALL destcalls[8];
   char notdestcall;
   char typs[13];
   char nottyps;
};

typedef char WWWB[1401];

struct WWWBUF;

typedef struct WWWBUF * pWWWBUF;


struct WWWBUF {
   WWWB buf;
   int32_t tlen;
   char push;
   pWWWBUF next;
};

struct QWatch;


struct QWatch {
   uint32_t lasttb;
   int32_t qsize;
   int32_t lastqb;
   int32_t txbyte[60];
};

struct TCPSOCK;

typedef struct TCPSOCK * pTCPSOCK;


struct TCPSOCK {
   pTCPSOCK next;
   int32_t fd;
   uint32_t beacont;
   uint32_t connt;
   uint32_t pongtime;
   char slowlink;
   char valid;
   char pingout; /* ping timeout state */
   char service;
   uint32_t gatepri;
   char ipnum[64];
   char port[6];
   struct POSCALL user;
   char vers[21];
   uint32_t wwwst;
   uint32_t reload;
   char sortby[2];
   struct QWatch qwatch;
   uint32_t txframes;
   uint32_t txbytes;
   uint32_t txbytesh;
   uint32_t rxframes;
   uint32_t rxbytes;
   uint32_t rxbytesh;
   uint32_t losttxframes;
   uint32_t lostrxframes;
   struct FILTERS filters;
   FILTERST outfilterst; /* for www show outconn filters */
   int32_t rpos;
   int32_t tlen;
   FRAMEBUF rbuf;
   char tbuf[1024];
   char get[256];
   pWWWBUF txbuf;
};

struct UDPSOCK;

typedef struct UDPSOCK * pUDPSOCK;

struct _0;


struct _0 {
   uint32_t uip;
   uint32_t uport;
   uint32_t rxframes;
   uint32_t rxbytes;
   uint32_t utime;
   uint32_t dtime;
};


struct UDPSOCK {
   pUDPSOCK next;
   int32_t fd;
   aprsstr_GHOSTSET ghosts;
   char rawread;
   char checkip;
   uint32_t ip;
   uint32_t bindport;
   uint32_t dport;
   uint32_t lasttxtime; /* for net to rf bit/s limit */
   uint32_t laststat;
   float torfradius; /* radius 0 no gateing */
   uint32_t maxbytes; /* byte/s tx */
   uint32_t lasttxbytes; /* for tx byte/s limit */
   uint32_t txframes;
   uint32_t txbytes;
   char allpathkey[10]; /* keyword on comment for allpath */
   char portname[10];
   struct _0 stat[16];
};

enum GATEFILT {udpgate4_gUNGATE, udpgate4_gRFONLY, udpgate4_gNOGATE,
                udpgate4_gTCPIP, udpgate4_gTCPXX, udpgate4_gQ};


#define udpgate4_MAXHEARD 500

#define udpgate4_cUSERMSG ":"

#define udpgate4_USERACK "{"

#define udpgate4_cTHIRDPARTY "}"

#define udpgate4_ICONDIR "icon"

#define udpgate4_CALLINKFN "calllink.txt"
/* build an URL for a klicked mh call */

#define udpgate4_SERVERLINKFN "serverlink.txt"
/* build an URL for a klicked server ip */

#define udpgate4_MHSTEPS 48

#define udpgate4_tCELSIUS "C"

#define udpgate4_tSPEED "S"

#define udpgate4_tJUNK "J"

#define udpgate4_tUNK "U"

#define udpgate4_tPOS ""

#define udpgate4_UNACKRET 1

#define udpgate4_MSGLEN 67

typedef char MSGTEXT[68];

typedef char ACKTEXT[5];

typedef char REPLYACK[2];

enum SOURCE {udpgate4_OBSOLET, udpgate4_NET, udpgate4_INDIR, udpgate4_DIR};


struct MESSAGE;

typedef struct MESSAGE * pMESSAGE;


struct MESSAGE {
   pMESSAGE next;
   uint32_t chkt;
   uint32_t txtime;
   uint32_t gentime;
   uint32_t txport;
   uint32_t msg2rf;
   uint32_t msg2net;
   uint32_t ack2rf;
   uint32_t ack2net;
   uint32_t retryc;
   uint8_t acksrc;
   uint8_t src;
   char reject;
   char acked;
   char ackackt;
   char queryrep;
   MONCALL from;
   MONCALL to;
   ACKTEXT ack;
   REPLYACK replyack;
   MSGTEXT text; /* no text is ack */
};

struct RAWTEXT;

typedef struct RAWTEXT * pRAWTEXT;


struct RAWTEXT {
   pRAWTEXT next;
   uint32_t htime;
   uint32_t txd;
   uint32_t len;
   char text[1024];
};

struct HEARD;

typedef struct HEARD * pHEARD;

struct _1;


struct _1 {
   uint16_t pack;
   uint16_t junk;
};


struct HEARD {
   pHEARD next;
   uint32_t fromrx;
   uint32_t mhtime;
   MONCALL call;
   uint32_t cntt;
   struct aprsstr_POSITION position;
   char sym;
   char symt;
   struct _1 cnt[49];
   char head[41];
   char datatyp;
   float data;
   double sortval; /* sort value inserted depending on sort by */
   char ungate; /* flag set by user to not igate this direct heard */
   uint16_t txd;
   uint8_t quali;
   signed char level;
   pRAWTEXT rawtext[2]; /* normal / junk raw frame storage */
};

static uint32_t msgsendtime;

static uint32_t udpgate4_POLYNOM = 0x8408UL;

static uint32_t udpgate4_CRCINIT = 0xFFFFUL;

static uint32_t udpgate4_CRCRESULT = 0x9F0BUL;

#define udpgate4_CR "\015"

#define udpgate4_LF "\012"

#define udpgate4_cUSERMESSAGE ":"

#define udpgate4_cISGATEWAY "G"

#define udpgate4_cISSERVER "S"

#define udpgate4_cISWWW "W"

#define udpgate4_cTELEMETRY "T"
/*      STYLE='<html><head><link href="style.css" rel="stylesheet" type="
                text/css"'; */

static char mhperport;

/* mh line for same call but different port */
static char datafilter;

static char verb;

/* send no data to user with no filter set */
static char callsrc;

static pTCPSOCK tcpsocks;

static uint32_t showip1;

static uint32_t showport1;

static uint32_t systime;

static uint32_t keeptime;

static char ungates[6][11];

static uint32_t timehash[65536];

static uint32_t realtime;

static uint32_t udpdonetime;

static uint32_t maxpongtime;

static uint32_t dupetime;

static uint32_t uptime;

static FILENAME rawlogname;

static FILENAME netbeaconfn;

static FILENAME logframename;

static uint32_t lastdnstime;

static uint32_t gateconndelay;

static uint32_t netbeaconintervall;

static uint32_t vcourse;

static uint32_t vspeed;

static uint32_t qas;

static uint32_t qasc;

static uint32_t maxusers;

static uint32_t httpcount;

static char vsym;

static char vsymt;

static int32_t valt;

static int32_t wwwsizelimit;

static int32_t logframes;

static MONCALL showid;

static MONCALL servercall;

static struct aprsstr_POSITION home; /* own position */

static char passwd[6];

static char serverrangefilter[256];

static char actfilter[256];

static char wwwdir[1024];

static char tcpbindport[6];

static char wwwbindport[6];

static char nettorfpath[81];

static pUDPSOCK udpsocks;

static uint32_t trygate;

static int32_t qmaxtime;

static FILENAME gatesfn; /* filename with gateway table */

struct _2;


struct _2 {
   char url[256];
   char port[6];
   uint32_t resolvtime;
   uint32_t connecttime;
   FILTERST filterst;
};

static struct _2 gateways[21];

static MONCALL rfdestcall;

static MONCALL netdestcall;

static MONCALL viacall;

static uint32_t netmhin;

static uint32_t netmhout;

static uint32_t msghttpcount;

static uint32_t mhhttpcount;

static uint32_t maxmsg;

static uint32_t maxatonce;

static uint32_t rawlines;

static uint32_t msgretries;

static uint32_t purgeunack;

static uint32_t purgemsg;

static uint32_t purgeacked;

static uint32_t purgeunacksent;

static uint32_t rfquiet;

static uint32_t heardtimevia;

/*  heardtime, */
static uint32_t heardtimetcp;

static uint32_t heardtimew;

static pMESSAGE messages;

static pHEARD heardvia;

static pHEARD hearddir;

static pHEARD heardtcp;

static char sendnetmsg;

static MONCALL netmh[100];

static uint32_t mhfilelines;

static FILENAME mhfilename;

static char udp2[100];


static void spintime(void)
{
   /* make monotonic systime out of jumping realtime */
   uint32_t dt;
   uint32_t t;
   t = osic_time();
   dt = t-realtime;
   realtime = t;
   if (dt<60UL) systime += dt;
} /* end spintime() */


static void Err(const char text[], uint32_t text_len)
{
   osi_WrStr("udpgate: ", 10ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */


static uint32_t Min(uint32_t a, uint32_t b)
{
   if (a<b) return a;
   else return b;
   return 0;
} /* end Min() */


static void skipblank(const char s[], uint32_t s_len, uint32_t * p)
{
   while (*p<s_len-1 && s[*p]==' ') ++*p;
} /* end skipblank() */


static void app(char s[], uint32_t s_len, float x, char rad)
{
   char h[30];
   aprsstr_Append(s, s_len, "/", 2ul);
   if (rad) aprsstr_FixToStr(X2C_DIVR(x,1.7453292519444E-2f), 5UL, h, 30ul);
   else aprsstr_FixToStr(x, 0UL, h, 30ul);
   aprsstr_Append(s, s_len, h, 30ul);
} /* end app() */


static void wrcalls(char s[], uint32_t s_len, const MONCALL tab[],
                uint32_t tab_len, char not, char sym)
{
   uint32_t j;
   if (tab[0UL][0U]) {
      if (s[0UL]) aprsstr_Append(s, s_len, " ", 2ul);
      if (not) aprsstr_Append(s, s_len, "-", 2ul);
      aprsstr_Append(s, s_len, (char *) &sym, 1u/1u);
      j = 0UL;
      while (tab[j][0U]) {
         aprsstr_Append(s, s_len, "/", 2ul);
         aprsstr_Append(s, s_len, tab[j], 10ul);
         ++j;
      }
   }
} /* end wrcalls() */


static void FiltToStr(struct FILTERS f, char s[], uint32_t s_len)
{
   struct FILTERS * anonym;
   s[0UL] = 0;
   { /* with */
      struct FILTERS * anonym = &f;
      if (anonym->typ0=='a') {
         aprsstr_Assign(s, s_len, "a", 2ul);
         app(s, s_len, anonym->base.lat, 1);
         app(s, s_len, anonym->base.long0, 1);
         app(s, s_len, anonym->edge.lat, 1);
         app(s, s_len, anonym->edge.long0, 1);
      }
      else if (anonym->typ0=='r') {
         aprsstr_Assign(s, s_len, "r", 2ul);
         app(s, s_len, anonym->base.lat, 1);
         app(s, s_len, anonym->base.long0, 1);
         app(s, s_len, anonym->radius, 0);
      }
      else if (anonym->typ0=='m') {
         aprsstr_Assign(s, s_len, "m", 2ul);
         app(s, s_len, anonym->radius, 0);
      }
      wrcalls(s, s_len, anonym->viacalls, 8ul, anonym->notvia, 'd');
      wrcalls(s, s_len, anonym->entrycalls, 8ul, anonym->notentry, 'e');
      wrcalls(s, s_len, anonym->prefixes, 8ul, anonym->notprefix, 'p');
      wrcalls(s, s_len, anonym->bud, 8ul, anonym->notbud, 'b');
      wrcalls(s, s_len, anonym->objects, 8ul, anonym->notobject, 'o');
      wrcalls(s, s_len, anonym->destcalls, 8ul, anonym->notdestcall, 'u');
      if (anonym->typs[0U]) {
         aprsstr_Append(s, s_len, " ", 2ul);
         if (anonym->nottyps) aprsstr_Append(s, s_len, "-", 2ul);
         aprsstr_Append(s, s_len, "t/", 3ul);
         aprsstr_Append(s, s_len, anonym->typs, 13ul);
      }
   }
} /* end FiltToStr() */


static char Watchclock(uint32_t * t, uint32_t intervall)
{
   uint32_t tn;
   if (intervall>0UL || *t==0UL) {
      /* send once */
      tn = systime;
      if (*t<=tn) {
         *t += intervall;
         if (*t<=tn) *t = tn+intervall;
         return 1;
      }
   }
   return 0;
} /* end Watchclock() */

static uint32_t udpgate4_ROOT = 0x73E2UL;


static uint32_t call2pass(const char c[], uint32_t c_len)
{
   uint32_t i0;
   uint32_t s;
   s = 0x73E2UL;
   i0 = 0UL;
   while ((i0<=c_len-1 && c[i0]) && c[i0]!='-') {
      s = s^X2C_LSH((uint32_t)(uint32_t)(uint8_t)X2C_CAP(c[i0]),32,
                (int32_t)!(i0&1)*8L);
      ++i0;
   }
   return (uint32_t)(s&0x7FFFUL);
} /* end call2pass() */


static int32_t GetSec(char h[], uint32_t h_len, uint32_t * p,
                uint32_t * n)
{
   char ok0;
   int32_t GetSec_ret;
   X2C_PCOPY((void **)&h,h_len);
   h[h_len-1] = 0;
   *n = 0UL;
   ok0 = 0;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      ok0 = 1;
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   if (!ok0) {
      GetSec_ret = -1L;
      goto label;
   }
   /*
   IO.WrCard(n,10); IO.WrLn;
   */
   GetSec_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetSec_ret;
} /* end GetSec() */


static char callok(const char h[], uint32_t h_len)
{
   uint32_t lit;
   uint32_t num;
   uint32_t i0;
   char c;
   num = 0UL;
   lit = 0UL;
   i0 = 0UL;
   for (;;) {
      c = h[i0];
      if ((uint8_t)c>='0' && (uint8_t)c<='9') ++num;
      else if ((uint8_t)c>='A' && (uint8_t)c<='Z') ++lit;
      else break;
      ++i0;
   }
   if ((lit<2UL || num==0UL) || num>2UL) return 0;
   if (h[i0]=='-') {
      ++i0;
      if (h[i0]=='1') {
         ++i0;
         if ((uint8_t)h[i0]>='0' && (uint8_t)h[i0]<='5') ++i0;
      }
      else {
         if ((uint8_t)h[i0]<'1' || (uint8_t)h[i0]>'9') return 0;
         ++i0;
      }
   }
   return h[i0]==0;
} /* end callok() */


static void readurlsfile(const char gatesfn0[], uint32_t gatesfn_len)
{
   uint32_t i0;
   uint32_t n;
   int32_t ii;
   int32_t len;
   int32_t fd;
   FILENAME h;
   memset((char *)gateways,(char)0,sizeof(struct _2 [21]));
   fd = osi_OpenRead(gatesfn0, gatesfn_len);
   if (fd<0L) {
      strncpy(h,"-g :file <",1024u);
      aprsstr_Append(h, 1024ul, gatesfn0, gatesfn_len);
      aprsstr_Append(h, 1024ul, "> not readable", 15ul);
      osi_WrStrLn(h, 1024ul);
      return;
   }
   n = 0UL;
   do {
      i0 = 0UL;
      for (;;) {
         len = osi_RdBin(fd, (char *) &h[i0], 1u/1u, 1UL);
         if (((len<=0L || i0>=1023UL) || h[i0]=='\015') || h[i0]=='\012') {
            h[i0] = 0;
            break;
         }
         ++i0;
      }
      if (h[0U] && h[0U]!='#') {
         if (h[0U]=='[') {
            ii = 1L;
            while (h[ii] && h[ii]!=']') ++ii;
            if (h[ii]!=']' || h[ii+1L]!=':') {
               osi_WrStrLn("urlfile: [url]:port", 20ul);
            }
            h[ii] = 0;
            i0 = 1UL;
            while (i0<=1023UL) {
               h[i0-1UL] = h[i0];
               ++i0;
            }
         }
         else ii = aprsstr_InStr(h, 1024ul, ":", 2ul);
         if (ii>=0L) h[ii] = 0;
         aprsstr_Assign(gateways[n].url, 256ul, h, 1024ul);
         if (ii>0L) {
            /* port number */
            ++ii;
            i0 = 0UL;
            while (ii<=1023L) {
               h[i0] = h[ii];
               ++i0;
               ++ii;
            }
         }
         else osi_WrStrLn("urlfile: [url]:port", 20ul);
         ii = aprsstr_InStr(h, 1024ul, "#", 2ul);
         if (ii>=0L) h[ii] = 0;
         if (h[0U]==0) {
            osi_WrStrLn("urlfile: [url]:port#filters", 28ul);
         }
         aprsstr_Assign(gateways[n].port, 6ul, h, 1024ul);
         if (ii>0L) {
            /* we have a filter string */
            ++ii;
            i0 = 0UL;
            while (ii<=1023L) {
               if (h[ii]==',') h[ii] = ' ';
               h[i0] = h[ii];
               ++i0;
               ++ii;
            }
            aprsstr_Assign(gateways[n].filterst, 256ul, h, 1024ul);
         }
         ++n;
      }
   } while (!(len<=0L || n>20UL));
   osic_Close(fd);
} /* end readurlsfile() */

static aprsstr_GHOSTSET _cnst = {0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL};

static void parms(void)
{
   char h[4096];
   aprsstr_GHOSTSET ghost;
   char err;
   char lasth;
   uint32_t n;
   uint32_t i0;
   uint32_t gatecnt;
   int32_t len;
   int32_t fd;
   pUDPSOCK ush;
   pUDPSOCK usock;
   int32_t ii;
   char allkey[10];
   struct UDPSOCK * anonym;
   uint32_t tmp;
   err = 0;
   verb = 0;
   datafilter = 0;
   gatecnt = 0UL;
   memcpy(ghost,_cnst,36u);
   keeptime = 0UL; /*600*/ /* default keep connected to gateway time */
   allkey[0U] = 0;
   aprsstr_showctrl = 1;
   for (;;) {
      osi_NextArg(h, 4096ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         lasth = h[1U];
         if (lasth=='R' || lasth=='M') {
            osi_NextArg(h, 4096ul);
            osic_alloc((char * *) &usock, sizeof(struct UDPSOCK));
            if (usock==0) Err("out of memory", 14ul);
            memset((char *)usock,(char)0,sizeof(struct UDPSOCK));
            { /* with */
               struct UDPSOCK * anonym = usock;
               if (aprsstr_GetIp2(h, 4096ul, &anonym->ip, &anonym->dport,
                &anonym->bindport, &anonym->checkip)<0L) {
                  Err("-R or -M need ip:port:port", 27ul);
               }
               anonym->fd = openudp();
               anonym->rawread = lasth=='R';
               if (anonym->fd<0L || bindudp(anonym->fd,
                anonym->bindport)<0L) {
                  Err("-R or -M cannot bind udpport", 29ul);
               }
               len = socknonblock(anonym->fd);
               ii = aprsstr_InStr(h, 4096ul, "+", 2ul);
               if (ii>0L) {
                  i0 = (uint32_t)(ii+1L);
                  if (GetSec(h, 4096ul, &i0, &n)>=0L) {
                     anonym->maxbytes = n;
                     ++i0;
                     if (h[i0-1UL]==':' && GetSec(h, 4096ul, &i0, &n)>=0L) {
                        anonym->torfradius = (float)n;
                     }
                  }
               }
               ii = aprsstr_InStr(h, 4096ul, "#", 2ul); /* get port name */
               if (ii>0L) {
                  ++ii;
                  i0 = 0UL;
                  while ((i0<=9UL && ii<=4095L) && h[ii]) {
                     anonym->portname[i0] = h[ii];
                     ++i0;
                     ++ii;
                  }
               }
               memcpy(anonym->ghosts,ghost,36u);
               aprsstr_Assign(anonym->allpathkey, 10ul, allkey, 10ul);
               anonym->next = 0;
            }
            if (udpsocks==0) udpsocks = usock;
            else {
               ush = udpsocks;
               while (ush->next) ush = ush->next;
               ush->next = usock;
            }
            memcpy(ghost,_cnst,36u);
         }
         else if (lasth=='c') callsrc = 1;
         else if (lasth=='s' || lasth=='S') {
            osi_NextArg(h, 4096ul);
            aprsstr_Assign(servercall, 10ul, h, 4096ul);
            if ((servercall[0U]==0 || servercall[0U]=='-')
                || lasth=='s' && !callok(h, 4096ul)) {
               Err("-s call-ssid", 13ul);
            }
         }
         else if (lasth=='p') {
            osi_NextArg(h, 4096ul);
            if ((uint8_t)h[0U]>='0' && (uint8_t)h[0U]<='9') {
               i0 = 0UL;
               while (i0<=5UL && (uint8_t)h[i0]>=' ') {
                  passwd[i0] = h[i0];
                  ++i0;
               }
            }
            else {
               fd = osi_OpenRead(h, 4096ul);
               if (fd<0L) Err("-p passcode or passwordfile", 28ul);
               len = osi_RdBin(fd, (char *)passwd, 6u/1u, 5UL);
               if (len>=0L) passwd[len] = 0;
               else Err("-p error with password file", 28ul);
               osic_Close(fd);
            }
            passwd[5U] = 0;
            i0 = 0UL;
            while ((uint8_t)passwd[i0]>='0' && (uint8_t)passwd[i0]<='9') {
               ++i0;
            }
            if (i0==0UL || passwd[i0]) Err("-p invalid passcode", 20ul);
         }
         else if (lasth=='m') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) maxusers = n;
            else Err("-m number", 10ul);
         }
         else if (lasth=='D') {
            osi_NextArg(h, 4096ul);
            aprsstr_Assign(wwwdir, 1024ul, h, 4096ul);
            if (h[0U]==0 || h[0U]=='-') Err("-D directory", 13ul);
            i0 = aprsstr_Length(wwwdir, 1024ul);
            if (i0>0UL && wwwdir[i0-1UL]!='/') {
               aprsstr_Append(wwwdir, 1024ul, "/", 2ul);
            }
         }
         else if (lasth=='d') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) dupetime = n;
            else Err("-d number", 10ul);
            if (dupetime<27UL) {
               osi_WrStrLn("-d do not set dupefilter less 27s!", 35ul);
            }
         }
         else if (lasth=='e') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) {
               if (n==0UL) n = 1UL;
               gateconndelay = n;
            }
            else Err("-e seconds", 11ul);
         }
         else if (lasth=='C') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) heardtimetcp = n*60UL;
            else Err("-C minutes", 11ul);
         }
         else if (lasth=='H') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) heardtimew = n*60UL;
            else Err("-H minutes", 11ul);
         }
         else if (lasth=='I') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) heardtimevia = n*60UL;
            else Err("-I minutes", 11ul);
         }
         else if (lasth=='i') {
            osi_NextArg(h, 4096ul);
            if (h[0U]=='-') h[0U] = 0;
            aprsstr_Assign(allkey, 10ul, h, 4096ul);
         }
         else if (lasth=='L') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L) maxmsg = n;
            else Err("-L number", 10ul);
         }
         else if (lasth=='l') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L && h[i0]==':') {
               logframes = (int32_t)n;
               ++i0;
               for (n = 0UL; n<=1023UL; n++) {
                  if (i0<=4095UL) {
                     logframename[n] = h[i0];
                     ++i0;
                  }
               } /* end for */
               if ((uint8_t)logframename[0U]<=' ') {
                  Err("-l loglevel:filename", 21ul);
               }
            }
            else Err("log format is level:file", 25ul);
         }
         else if (lasth=='r') {
            osi_NextArg(rawlogname, 1024ul);
            if ((uint8_t)rawlogname[0U]<=' ') {
               Err("-r rawlogfilename", 18ul);
            }
         }
         else if (lasth=='F') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L && h[i0]==':') {
               mhfilelines = n;
               ++i0;
               for (n = 0UL; n<=1023UL; n++) {
                  if (i0<=4095UL) {
                     mhfilename[n] = h[i0];
                     ++i0;
                  }
               } /* end for */
               if ((uint8_t)mhfilename[0U]<=' ') {
                  Err("-F MHfilename", 14ul);
               }
            }
            else Err("MH File lines:file", 19ul);
         }
         else if (lasth=='n') {
            osi_NextArg(h, 4096ul);
            i0 = 0UL;
            if (GetSec(h, 4096ul, &i0, &n)>=0L && h[i0]==':') {
               netbeaconintervall = n*60UL;
               ++i0;
               for (n = 0UL; n<=1023UL; n++) {
                  if (i0<=4095UL) {
                     netbeaconfn[n] = h[i0];
                     ++i0;
                  }
               } /* end for */
               if ((uint8_t)netbeaconfn[0U]<=' ') {
                  Err("-n netbeacon filename", 22ul);
               }
            }
            else Err("-n netbeacon format is minutes:file", 36ul);
         }
         else if (lasth=='N') sendnetmsg = 0;
         else if (lasth=='E') aprsstr_showctrl = 0;
         else if (lasth=='t') {
            osi_NextArg(h, 4096ul);
            aprsstr_Assign(tcpbindport, 6ul, h, 4096ul);
            if (h[0U]==0 || h[0U]=='-') Err("-t port", 8ul);
         }
         else if (lasth=='w') {
            osi_NextArg(h, 4096ul);
            aprsstr_Assign(wwwbindport, 6ul, h, 4096ul);
            if (h[0U]==0 || h[0U]=='-') Err("-w port", 8ul);
         }
         else if (lasth=='f') {
            osi_NextArg(h, 4096ul);
            aprsstr_Assign(serverrangefilter, 256ul, h, 4096ul);
            if (h[0U]==0 || h[0U]=='-') Err("-f rangefilter", 15ul);
            tmp = aprsstr_Length(serverrangefilter, 256ul);
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               if (serverrangefilter[i0]==',') serverrangefilter[i0] = ' ';
               if (i0==tmp) break;
            } /* end for */
         }
         else if (lasth=='g') {
            /* "url port" or "url:port" or "url:port#filter" */
            osi_NextArg(h, 4096ul);
            if (h[0U]==0) Err("-g url port", 12ul);
            if (h[0U]==':') {
               /* get urls later from file */
               for (i0 = 1UL; i0<=4095UL; i0++) {
                  h[i0-1UL] = h[i0];
               } /* end for */
               h[4095U] = 0;
               aprsstr_Assign(gatesfn, 1024ul, h, 4096ul);
            }
            else {
               if (gatecnt>20UL) Err("-g gateway table full", 22ul);
               h[4095U] = 0;
               if (h[0U]=='[') {
                  ii = 1L;
                  while (h[ii] && h[ii]!=']') ++ii;
                  if (h[ii]!=']' || h[ii+1L]!=':') {
                     Err("-g [url]:port", 14ul);
                  }
                  h[ii] = 0;
                  i0 = 1UL;
                  while (i0<=4095UL) {
                     h[i0-1UL] = h[i0];
                     ++i0;
                  }
               }
               else ii = aprsstr_InStr(h, 4096ul, ":", 2ul);
               if (ii>=0L) h[ii] = 0;
               aprsstr_Assign(gateways[gatecnt].url, 256ul, h, 4096ul);
               if (ii>0L) {
                  /* port number */
                  ++ii;
                  i0 = 0UL;
                  while (ii<=4095L) {
                     h[i0] = h[ii];
                     ++i0;
                     ++ii;
                  }
               }
               else osi_NextArg(h, 4096ul);
               h[4095U] = 0;
               ii = aprsstr_InStr(h, 4096ul, "#", 2ul);
               if (ii>=0L) h[ii] = 0;
               if (h[0U]==0) Err("-g url:port", 12ul);
               aprsstr_Assign(gateways[gatecnt].port, 6ul, h, 4096ul);
               if (ii>0L) {
                  /* we have a filter string */
                  ++ii;
                  i0 = 0UL;
                  while (ii<=4095L) {
                     if (h[ii]==',') h[ii] = ' ';
                     h[i0] = h[ii];
                     ++i0;
                     ++ii;
                  }
                  aprsstr_Assign(gateways[gatecnt].filterst, 256ul, h,
                4096ul);
               }
               ++gatecnt;
            }
         }
         else {
            if (lasth=='h') {
               osi_WrStrLn(" -0             send no Data (only Messages and a\
ck) to User with no Filter", 76ul);
               osi_WrStrLn(" -C <time>      connected (tcp) remember position\
 minutes (Min) (-C 1440)", 74ul);
               osi_WrStrLn(" -c             delete frames with no valid sourc\
e call in APRS-IS stream", 74ul);
               osi_WrStrLn(" -D <path>      www server root directory (-D /us\
r/www/)", 57ul);
               osi_WrStrLn(" -d <time>      dupe filter time in seconds, not \
below 27s! (default 60s)", 74ul);
               osi_WrStrLn(" -E             Erase axudp frames with ctrl-char\
s in calls else show \"^\" (off)", 80ul);
               osi_WrStrLn(" -e <time>      wait before (re)connect to (next)\
 gateway in seconds, (30s)", 76ul);
               osi_WrStrLn(" -F <lines>:<file> write direct heard file (call,\
sym,port,s,cnt,km,data,path)", 78ul);
               osi_WrStrLn(" -f <filters>   backstream filter text sent to ou\
t connected server -f m/50", 76ul);
               osi_WrStrLn("                if blanks dont pass parameter set\
tings use , (-f m/30,-d/CW)", 77ul);
               osi_WrStrLn(" -g <url>:<port>[#<filters>] connect to APRS-IS g\
ateway, repeat -g for a list", 78ul);
               osi_WrStrLn("                with favorites first and all urls\
 will be tried to connect", 75ul);
               osi_WrStrLn("                if the active connect is not the \
first in list, urls", 69ul);
               osi_WrStrLn("                before will be polled and if gets\
 connected, data transfer", 75ul);
               osi_WrStrLn("                is switched to this link and the \
old gets disconnected", 71ul);
               osi_WrStrLn("                if no filter setting, global -f f\
ilter is used", 63ul);
               osi_WrStrLn("                -g www.db0anf.de:14580#m/50 -g 12\
7.0.0.1:3000", 62ul);
               osi_WrStrLn("                ipv6 if enabled by kernel -g [::1\
]:14580#m/200", 63ul);
               osi_WrStrLn(" -g :<filename> read gateway urls from file url:p\
ort#filter,filter,...", 71ul);
               osi_WrStrLn(" -H <time>      direct heard keep time minutes (M\
in) (-H 1440)", 63ul);
               osi_WrStrLn(" -h             this", 21ul);
               osi_WrStrLn(" -I <time>      indirect heard keep time minutes \
(Min) (-I 30)", 63ul);
               osi_WrStrLn(" -i <word>      keyword in rf-frame-comment to en\
able multipath, * for all frames", 82ul);
               osi_WrStrLn(" -j <time>[:<count>]   maximum time/count to (re)\
send messages (s) (-j 21600:12)", 81ul);
               osi_WrStrLn("                count=0: message passed thru to d\
irect heard as ports tx limits allow)", 87ul);
               osi_WrStrLn(" -k <time>      0 always connect to gateway else \
connect on demand and hold (0)", 80ul);
               osi_WrStrLn("                (seconds) after last User gone or\
 valid UDP Data arrived", 73ul);
               osi_WrStrLn(" -L <number>    max messages stored else delete o\
ldest (-L 1000)", 65ul);
               osi_WrStrLn("                -L 0 and -x set and net-to-rf ena\
bled all msg to heard gatet", 77ul);
               osi_WrStrLn(" -l <level>:<file> logfile -l 6:/tmp/log.txt",
                45ul);
               osi_WrStrLn("                level: 1 logins, 2 +sent frames, \
6 +fitered frames, 7 +dups", 76ul);
               osi_WrStrLn(" -M             same as -R but tnc text format",
                47ul);
               osi_WrStrLn(" -m <maxconnects> max inbound connects -m 20 (def\
ault 50)", 58ul);
               osi_WrStrLn(" -N             send no stored messages to net ex\
cept query answers", 68ul);
               osi_WrStrLn(" -n <min>:<file> netbeacon minutes:filename -n 10\
:netbeacon.txt", 64ul);
               osi_WrStrLn("                \\\\z ddhhmm, \\\\h hhmmss, \\\\:\
filename: insert file, \\\\v insert", 77ul);
               osi_WrStrLn("                Version, \\\\\\ insert \\\\",
                39ul);
               osi_WrStrLn("                beacon file like: !8959.00N/17959\
.00E&igate mars", 65ul);
               osi_WrStrLn("                beacon file used by udpgate itsel\
f to find out own position", 76ul);
               osi_WrStrLn(" -O             make MH entry for same calls but \
different port", 64ul);
               osi_WrStrLn(" -o <seconds>   ping-pong: time to stop data forw\
arding after last ping", 72ul);
               osi_WrStrLn("                use double time of igate ping int\
ervall", 56ul);
               osi_WrStrLn(" -P <time[:time]> purge unacked (:acked) messages\
 after seconds (-P 86400:300)", 79ul);
               osi_WrStrLn(" -p <password>  login passwort for aprs-is server\
s -p 12345", 60ul);
               osi_WrStrLn("                to hide password in commandline u\
se file mode -p pass.txt", 74ul);
               osi_WrStrLn(" -Q <n>         send netbeacon with qAS if qAI do\
nt pass some servers", 70ul);
               osi_WrStrLn("                0=never, 1=always else every n be\
acons send 1 with qAI", 71ul);
               osi_WrStrLn(" -q <time>      minimum quiet time after rf tx se\
conds (-q 10)", 63ul);
               osi_WrStrLn(" -R <ip>:<dport>/<lport>[+<byte/s>[:<radius>]][#<\
portname>]", 60ul);
               osi_WrStrLn("                udp rf port (monitor frame format\
) for local (t)rx", 67ul);
               osi_WrStrLn("                <dport>/<lport> \"/\" only from t\
his ip, dport=0 no tx", 69ul);
               osi_WrStrLn("                +byte/s enable inet to rf for ser\
vices like WLNK, WHO-IS", 73ul);
               osi_WrStrLn("                :radius enable all inet to rf gat\
e (from km around digi)", 73ul);
               osi_WrStrLn("                messages to NOT direct heard user\
s are gated at any radius >0", 78ul);
               osi_WrStrLn("                if no <ip> given then \'127.0.0.1\\
' is used", 58ul);
               osi_WrStrLn("                #portname max 10 char like \'1448\
00\'", 52ul);
               osi_WrStrLn("                repeat -M for each radio port wit\
h a tx or different portname", 78ul);
               osi_WrStrLn(" -r <filename>  write a dated 1 day logfile with \
date+time+data lines", 70ul);
               osi_WrStrLn(" -S <call>      server call of this server -s MYC\
ALL-10, no check if valid call", 80ul);
               osi_WrStrLn(" -s <call>      server call of this server -s MYC\
ALL-10", 56ul);
               osi_WrStrLn(" -T <seconds>   kill link to server if unack tcp \
bytes are longer in tx queue", 78ul);
               osi_WrStrLn("                avoids delayed trackpoints (defau\
lt 15s, off 0, max 60)", 72ul);
               osi_WrStrLn(" -t <localport> local igate tcp port for in conne\
cts -t 14580", 62ul);
               osi_WrStrLn(" -U <time[:time]> purge unsent(:sent) unack messa\
ges after seconds (-P 600:60)", 79ul);
               osi_WrStrLn(" -u <maxlines>  raw frame listing by click to fra\
me counter in Heard list (20) (0 off)", 87ul);
               osi_WrStrLn(" -V <path>      Via Path for net to rf frames, \"\
-1\" for SSID on destination call", 81ul);
               osi_WrStrLn(" -v             show frames and analytics on stdo\
ut", 52ul);
               osi_WrStrLn(" -W <filesize>  limit www server file size in 102\
4byte, (-W 1000)", 66ul);
               osi_WrStrLn(" -w <port>      port of www server -w 14501",
                44ul);
               osi_WrStrLn(" -x <call>      via <call> send messages to rf (-\
x OE0AAA-10) tx off: -x -", 75ul);
               osi_WrStrLn("                default is server call", 39ul);
               osi_WrStrLn(" -Y [num][,num]... bad digis fingerprints to inse\
rt GHOST* in otherwise false", 78ul);
               osi_WrStrLn("                direct heard path", 34ul);
               osi_WrStrLn("udpgate -v -R 127.0.0.1:9200:9201 -s MYCALL-10 -l\
 7:aprs.log -n 10:beacon.txt -t 14580 -g www.server.org:14580#m/30 -p 12345",
                 125ul);
               X2C_ABORT();
            }
            if (lasth=='u') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) rawlines = n;
               else Err("-u <lines>", 11ul);
            }
            else if (lasth=='O') mhperport = 1;
            else if (lasth=='j') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) msgsendtime = n;
               else Err("-j seconds[:retries]", 21ul);
               if (h[i0]==':') {
                  ++i0;
                  if (GetSec(h, 4096ul, &i0, &n)>=0L) msgretries = n;
                  else Err("-j seconds[:retries]", 21ul);
               }
            }
            else if (lasth=='P') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) purgemsg = n;
               else Err("-P seconds", 11ul);
               if (h[i0]==':') {
                  ++i0;
                  if (GetSec(h, 4096ul, &i0, &n)>=0L) purgeacked = n;
                  else Err("-P seconds:seconds", 19ul);
               }
            }
            else if (lasth=='U') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) purgeunack = n;
               else Err("-U seconds", 11ul);
               if (h[i0]==':') {
                  ++i0;
                  if (GetSec(h, 4096ul, &i0, &n)>=0L) purgeunacksent = n;
                  else Err("-U seconds:seconds", 19ul);
               }
            }
            else if (lasth=='Q') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) qas = n;
               else Err("-Q number", 10ul);
            }
            else if (lasth=='q') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) rfquiet = n;
               else Err("-q seconds", 11ul);
            }
            else if (lasth=='v') verb = 1;
            else if (lasth=='V') {
               osi_NextArg(h, 4096ul);
               aprsstr_Assign(nettorfpath, 81ul, h, 4096ul);
               if (h[0U]==0) Err("-V net to rf via path", 22ul);
            }
            else if (lasth=='k') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) keeptime = n;
               else Err("-k seconds", 11ul);
            }
            else if (lasth=='T') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) qmaxtime = (int32_t)n;
               else Err("-T seconds", 11ul);
               if (qmaxtime>59L) qmaxtime = 59L;
            }
            else if (lasth=='o') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) maxpongtime = n;
               else Err("-o seconds", 11ul);
            }
            else if (lasth=='x') {
               osi_NextArg(h, 4096ul);
               aprsstr_Assign(viacall, 10ul, h, 4096ul);
               if (h[0U]==0 || h[0U]=='-' && h[1U]) Err("-x call", 8ul);
            }
            else if (lasth=='W') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               if (GetSec(h, 4096ul, &i0, &n)>=0L) {
                  wwwsizelimit = (int32_t)(n*1024UL);
               }
               else Err("-W kbytes", 10ul);
            }
            else if (lasth=='Y') {
               osi_NextArg(h, 4096ul);
               i0 = 0UL;
               while (i0<4095UL && h[i0]) {
                  if (GetSec(h, 4096ul, &i0, &n)>=0L) X2C_INCL(ghost,n,257);
                  else Err("-Y <num>", 9ul);
                  if (h[i0]==',') ++i0;
               }
            }
            else if (lasth=='0') datafilter = 1;
            else err = 1;
         }
      }
      else {
         /*h[0]:=0C;*/
         err = 1;
      }
      if (err) break;
   }
   if (err) {
      aprsstr_Append(h, 4096ul, " ? use -h", 10ul);
      Err(h, 4096ul);
   }
   if (servercall[0U]==0) {
      osic_WrLn();
      osi_WrStrLn("udpgate: NO SERVERCALL ?", 25ul);
      osic_WrLn();
   }
   if (wwwbindport[0U] && wwwdir[0]==0) {
      osic_WrLn();
      osi_WrStrLn("udpgate: -w www-port but no -D icon-dir-path ?", 47ul);
      osic_WrLn();
   }
} /* end parms() */


static pUDPSOCK porttosock(uint32_t p)
{
   pUDPSOCK s;
   if (p==0UL) return 0;
   s = udpsocks;
   while (s && p>1UL) {
      s = s->next;
      --p;
   }
   return s;
} /* end porttosock() */


static char Sendudp(const FRAMEBUF s, uint32_t totx,
                char unlimit)
{
   int32_t len;
   FRAMEBUF raw;
   pUDPSOCK us;
   char done;
   struct UDPSOCK * anonym;
   /*        udpstat(us, 0, dport, ip); */
   done = 0;
   us = porttosock(totx); /* send to which udp modem */
   if (us && ((unlimit || us->maxbytes==65535UL) || (systime-us->lasttxtime)
                *us->maxbytes>us->lasttxbytes)) {
      if (us->rawread) aprsstr_mon2raw(s, 512ul, raw, 512ul, &len);
      else {
         len = (int32_t)(aprsstr_Length(s, 512ul)+1UL);
         memcpy(raw,s,512u);
      }
      if (len>1L) {
         { /* with */
            struct UDPSOCK * anonym = us;
            anonym->lasttxbytes = (uint32_t)len;
            anonym->lasttxtime = systime;
            ++anonym->txframes;
            anonym->txbytes += (uint32_t)len;
         }
         len = udpsend(us->fd, raw, len, us->dport, us->ip);
         done = 1;
      }
      else if (verb) osi_WrStrLn("wrong inet to rf frame format", 30ul);
   }
   return done;
} /* end Sendudp() */


static char getudp(pUDPSOCK usock, FRAMEBUF buf,
                aprsstr_GHOSTSET ghost)
{
   uint32_t fromport;
   uint32_t ipn;
   char crc2;
   char crc1;
   int32_t oldi;
   int32_t i0;
   int32_t len;
   uint32_t mlen;
   FRAMEBUF mbuf0;
   uint32_t oldt;
   struct _0 * anonym;
   aprsstr_GHOSTSET tmp;
   int32_t tmp0;
   char tmp1;
   ghost = (uint32_t *)memcpy(tmp,ghost,36u);
   for (;;) {
      len = udpreceive(usock->fd, buf, 512L, &fromport, &ipn);
      if ((len<=2L || len>=512L) || usock->checkip && usock->ip!=ipn) {
         return 0;
      }
      buf[len] = 0;
      udp2[0U] = 0;
      if (usock->rawread) {
         crc1 = buf[len-2L];
         crc2 = buf[len-1L];
         aprsstr_AppCRC(buf, 512ul, len-2L);
         if (crc1!=buf[len-2L] || crc2!=buf[len-1L]) {
            if (verb) osi_WrStrLn(" axudp crc error", 17ul);
            buf[0U] = 0;
         }
         else {
            if (buf[0U]=='\001') {
               aprsstr_extrudp2(buf, 512ul, udp2, 100ul, &len); /* axudp2 */
            }
            if (len>2L) {
               aprsstr_raw2mon(buf, 512ul, mbuf0, 512ul,
                (uint32_t)(len-2L), &mlen, ghost);
               if (mbuf0[0U]==0 && verb) {
                  osi_WrStrLn(" axudp frame decode error", 26ul);
                  tmp0 = len-3L;
                  i0 = 0L;
                  if (i0<=tmp0) for (;; i0++) {
                     osi_WrHex((uint32_t)(uint8_t)buf[i0], 3UL);
                     if (i0==tmp0) break;
                  } /* end for */
                  osic_WrLn();
                  i0 = 0L;
                  for (;;) {
                     if (i0>=len-3L) break;
                     if ((uint32_t)(uint8_t)buf[i0]/2UL<32UL) {
                        osi_WrStr("<", 2ul);
                        osi_WrHex((uint32_t)(uint8_t)buf[i0]/2UL, 1UL);
                        osi_WrStr(">", 2ul);
                     }
                     else {
                        osi_WrStr((char *)(tmp1 = (char)((uint32_t)
                (uint8_t)buf[i0]/2UL),&tmp1), 1u/1u);
                     }
                     if (((uint32_t)(uint8_t)buf[i0]&1)) break;
                     if (i0%7L==6L) osi_WrStr(",", 2ul);
                     ++i0;
                  }
                  osic_WrLn();
               }
               memcpy(buf,mbuf0,512u);
            }
            else buf[0U] = 0;
            len = (int32_t)aprsstr_Length(buf, 512ul); /* for statistic */
         }
      }
      /*-    udpstat(usock, len, fromport, ipn); */
      if (buf[0U]) {
         /* statistic */
         i0 = 0L;
         oldt = systime;
         oldi = 0L;
         for (;;) {
            if (usock->stat[i0].uip==ipn && usock->stat[i0].uport==fromport) {
               oldi = i0;
               i0 = 15L;
            }
            if (i0>=15L) {
               { /* with */
                  struct _0 * anonym = &usock->stat[oldi];
                  anonym->uport = fromport;
                  anonym->uip = ipn;
                  if (anonym->dtime/2UL+anonym->utime/2UL>systime || anonym->utime+600UL<systime)
                 {
                     /* allow clock back step till 1/2 uptime */
                     /* reset statistic */
                     anonym->dtime = systime;
                     /*            usock^.txframes:=0; */
                     /*            usock^.txbytes:=0; */
                     anonym->rxframes = 0UL;
                     anonym->rxbytes = 0UL;
                  }
                  ++anonym->rxframes;
                  anonym->rxbytes += (uint32_t)len;
                  anonym->utime = systime;
                  usock->laststat = (uint32_t)oldi;
               }
               break;
            }
            if (usock->stat[i0].utime<oldt) {
               oldt = usock->stat[i0].utime;
               oldi = i0;
            }
            ++i0;
         }
         return 1;
      }
   }
   return 0;
} /* end getudp() */


static int32_t Gettcp(int32_t fd, FRAMEBUF line, FRAMEBUF buf,
                int32_t * pos)
{
   int32_t j;
   int32_t i0;
   int32_t len;
   int32_t tmp;
   i0 = *pos;
   if (i0>=512L) i0 = 0L;
   len = readsock(fd, line, 512L-i0);
   if (len<0L) return -1L;
   if (*pos>=512L) {
      /* hunt mode */
      i0 = 0L;
      while (i0<len && line[i0]!='\012') ++i0;
      if (i0>=len) return 0L;
      /* no line start found */
      *pos = 0L;
      ++i0;
      j = 0L;
      while (i0<len) {
         line[j] = line[i0];
         ++j;
         ++i0;
      }
      len = j;
   }
   if (*pos>0L) {
      for (i0 = len-1L; i0>=0L; i0--) {
         line[i0+*pos] = line[i0]; /* move up new */
      } /* end for */
      tmp = *pos-1L;
      i0 = 0L;
      if (i0<=tmp) for (;; i0++) {
         line[i0] = buf[i0]; /* ins buf before */
         if (i0==tmp) break;
      } /* end for */
      len += *pos;
      *pos = 0L;
   }
   i0 = 0L;
   while (i0<len && line[i0]!='\012') ++i0;
   if (i0>=len) {
      /* no line end found */
      while (*pos<len) {
         buf[*pos] = line[*pos]; /* move to buf */
         ++*pos;
      }
      len = 0L;
   }
   else {
      j = i0+1L;
      while (j<len) {
         buf[*pos] = line[j]; /* move rest to buf */
         ++*pos;
         ++j;
      }
      len = i0;
   }
   line[len] = 0;
   return len;
} /* end Gettcp() */


static void showpip(uint32_t ip, uint32_t port, char s[],
                uint32_t s_len)
{
   char h[21];
   aprsstr_IntToStr((int32_t)(ip/16777216UL), 1UL, s, s_len);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip/65536UL&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip/256UL&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   if (port) {
      aprsstr_Append(s, s_len, ":", 2ul);
      aprsstr_IntToStr((int32_t)port, 1UL, h, 21ul);
      aprsstr_Append(s, s_len, h, 21ul);
   }
} /* end showpip() */


static void logline(int32_t r, char s[], uint32_t s_len)
{
   char h[512];
   int32_t fd;
   uint32_t i0;
   X2C_PCOPY((void **)&s,s_len);
   if (logframename[0U] && logframes+r>1L) {
      fd = osi_OpenAppend(logframename, 1024ul);
      if (fd<0L) fd = osi_OpenWrite(logframename, 1024ul);
      if (fd>=0L) {
         aprsstr_DateToStr(osic_time(), h, 512ul);
         aprsstr_Append(h, 512ul, " ", 2ul);
         aprsstr_Append(h, 512ul, s, s_len);
         aprsstr_Append(h, 512ul, "\012", 2ul);
         osi_WrBin(fd, (char *)h, 512u/1u, aprsstr_Length(h, 512ul));
         osic_Close(fd);
      }
      else {
         osi_WrStr("cannot write", 13ul);
         osi_WrStr(logframename, 1024ul);
         osic_WrLn();
      }
   }
   if (verb) {
      /* & (r<>-5) */
      i0 = aprsstr_Length(s, s_len);
      if ((i0>=2UL && s[i0-1UL]=='\012') && s[i0-2UL]=='\015') s[i0-2UL] = 0;
      aprsstr_CtrlHex(s, s_len);
      osi_WrStrLn(s, s_len);
   }
   X2C_PFREE(s);
} /* end logline() */


static void writerawlog(const FRAMEBUF b)
{
   char h[512];
   FILENAME fn;
   uint32_t i0;
   uint32_t l;
   int32_t f;
   l = aprsstr_Length(b, 512ul);
   while (l>0UL && (uint8_t)b[l-1UL]<='\015') --l;
   if (l>0UL) {
      aprsstr_DateToStr(osic_time(), h, 512ul);
      h[4U] = h[5U];
      h[5U] = h[6U];
      h[6U] = h[8U];
      h[7U] = h[9U];
      h[8U] = 0;
      h[9U] = h[11U];
      h[10U] = h[12U];
      h[11U] = h[14U];
      h[12U] = h[15U];
      h[13U] = h[17U];
      h[14U] = h[18U];
      h[15U] = ' ';
      aprsstr_Assign(fn, 1024ul, rawlogname, 1024ul);
      aprsstr_Append(fn, 1024ul, h, 512ul);
      h[8U] = ':';
      f = osi_OpenAppend(fn, 1024ul);
      if (f<0L) f = osi_OpenWrite(fn, 1024ul);
      if (f>=0L) {
         if (l>=495UL) l = 495UL;
         i0 = 0UL;
         do {
            h[i0+16UL] = b[i0];
            ++i0;
         } while (i0<l);
         h[i0+16UL] = '\012';
         ++i0;
         osi_WrBin(f, (char *)h, 512u/1u, i0+16UL);
         osic_Close(f);
      }
      else if (verb) {
         osi_WrStr("cannot write ", 14ul);
         osi_WrStrLn(fn, 1024ul);
      }
   }
} /* end writerawlog() */


static void showframe(int32_t r, pTCPSOCK sp, pUDPSOCK su,
                const char buf[], uint32_t buf_len,
                struct aprsstr_POSITION pos)
{
   char s[512];
   char h[512];
   struct UDPSOCK * anonym;
   if (sp==0) {
      if (su) {
         strncpy(s,"U:",512u);
         { /* with */
            struct UDPSOCK * anonym = su;
            if (anonym->stat[anonym->laststat]
                .uip!=showip1 || anonym->stat[anonym->laststat]
                .uport!=showport1) {
               showip1 = anonym->stat[anonym->laststat].uip;
               showport1 = anonym->stat[anonym->laststat].uport;
               showpip(showip1, showport1, h, 512ul);
               aprsstr_Append(s, 512ul, h, 512ul);
               aprsstr_Append(s, 512ul, ":", 2ul);
            }
         }
      }
      else strncpy(s," :",512u);
   }
   else {
      s[0U] = sp->service;
      s[1U] = ':';
      s[2U] = 0;
      if (sp->user.call[0U] && !aprsstr_StrCmp(sp->user.call, 10ul, showid,
                10ul)) {
         aprsstr_Append(s, 512ul, sp->user.call, 10ul);
         aprsstr_Append(s, 512ul, ":", 2ul);
         aprsstr_Assign(showid, 10ul, sp->user.call, 10ul);
      }
   }
   /*
   IO.WrFixed(distance(home,pos), 1, 10); IO.WrStr("km"); IO.WrLn;
   */
   switch (r) {
   case 0L:
      if (!aprspos_posvalid(pos) || !aprspos_posvalid(home)) {
         strncpy(h,"--->",512u);
      }
      else {
         aprsstr_IntToStr((int32_t)(uint32_t)
                X2C_TRUNCC(0.5f+aprspos_distance(home, pos),0UL,
                X2C_max_longcard), 3UL, h, 512ul);
         aprsstr_Append(h, 512ul, ">", 2ul);
      }
      break;
   case -1L:
      strncpy(h,"syn:",512u);
      break;
   case -2L:
      strncpy(h,"len:",512u);
      break;
   case -3L:
      strncpy(h,"cal:",512u);
      break;
   case -4L:
      strncpy(h,"msg:",512u);
      break;
   case -5L:
      strncpy(h,"dup:",512u);
      break;
   case -6L:
      strncpy(h,"ung:",512u);
      break;
   default:;
      strncpy(h,"   :",512u);
      break;
   } /* end switch */
   aprsstr_Append(s, 512ul, h, 512ul);
   aprsstr_Append(s, 512ul, buf, buf_len);
   logline(r, s, 512ul);
} /* end showframe() */


static void appkib(char s[], uint32_t s_len, uint32_t h,
                uint32_t l)
{
   char b[41];
   if (h>0UL) {
      aprsstr_CardToStr(l/1024UL+h*4194304UL, 1UL, b, 41ul);
      aprsstr_Append(s, s_len, "kB=", 4ul);
   }
   else {
      aprsstr_CardToStr(l, 1UL, b, 41ul);
      aprsstr_Append(s, s_len, "B=", 3ul);
   }
   aprsstr_Append(s, s_len, b, 41ul);
} /* end appkib() */


static uint32_t bitsec64(uint32_t h, uint32_t l, uint32_t t)
{
   float r;
   r = X2C_DIVR(((float)h*4.294967296E+9f+(float)l)*8.0f,(float)t);
   if (r>2.E+9f) return 2000000000UL;
   return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
} /* end bitsec64() */


static char iscall(const char b[], uint32_t b_len,
                uint32_t p)
{
   uint32_t i0;
   uint32_t nu;
   uint32_t li;
   char c;
   li = 0UL;
   nu = 0UL;
   for (i0 = 0UL; i0<=2UL; i0++) {
      c = b[p+i0];
      if ((uint8_t)c>='A' && (uint8_t)c<='Z') ++li;
      else if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         if (i0==0UL) ++li;
         else ++nu;
      }
   } /* end for */
   /*
   IO.WrCard(li,9);IO.WrCard(nu,9); IO.WrStrLn("<3>");
   */
   return (nu+li==3UL && nu>=1UL) && nu<=2UL;
} /* end iscall() */


static void Statist(pTCPSOCK cp, uint32_t t, char s[],
                uint32_t s_len)
{
   char h[256];
   struct TCPSOCK * anonym;
   { /* with */
      struct TCPSOCK * anonym = cp;
      if (anonym->connt<t) t -= anonym->connt;
      else t = 0UL;
      aprsstr_Assign(s, s_len, " t=", 4ul);
      aprsstr_TimeToStr(t, h, 256ul);
      aprsstr_Append(s, s_len, h, 256ul);
      aprsstr_CardToStr(anonym->txframes, 1UL, h, 256ul);
      aprsstr_Append(s, s_len, " txf=", 6ul);
      aprsstr_Append(s, s_len, h, 256ul);
      aprsstr_CardToStr(anonym->rxframes, 1UL, h, 256ul);
      aprsstr_Append(s, s_len, " rxf=", 6ul);
      aprsstr_Append(s, s_len, h, 256ul);
      aprsstr_Append(s, s_len, " tx", 4ul);
      appkib(s, s_len, anonym->txbytesh, anonym->txbytes);
      aprsstr_Append(s, s_len, " rx", 4ul);
      appkib(s, s_len, anonym->rxbytesh, anonym->rxbytes);
      if (t>0UL) {
         aprsstr_Append(s, s_len, " ", 2ul);
         aprsstr_CardToStr(bitsec64(anonym->txbytesh, anonym->txbytes, t),
                1UL, h, 256ul);
         aprsstr_Append(s, s_len, h, 256ul);
         aprsstr_Append(s, s_len, "tbit/s ", 8ul);
         aprsstr_CardToStr(bitsec64(anonym->rxbytesh, anonym->rxbytes, t),
                1UL, h, 256ul);
         aprsstr_Append(s, s_len, h, 256ul);
         aprsstr_Append(s, s_len, "rbit/s", 7ul);
      }
   }
} /* end Statist() */


static void showlogout(pTCPSOCK sp)
{
   char t[256];
   char h[256];
   struct TCPSOCK * anonym;
   { /* with */
      struct TCPSOCK * anonym = sp;
      if (anonym->slowlink) strncpy(h,"T:slowlink closed ",256u);
      else strncpy(h,"T:connection closed ",256u);
      aprsstr_Append(h, 256ul, anonym->user.call, 10ul);
      aprsstr_Append(h, 256ul, " ", 2ul);
      aprsstr_Append(h, 256ul, anonym->ipnum, 64ul);
      if (anonym->connt) {
         Statist(sp, systime, t, 256ul);
         /*
               ti:=time();
               IF ti<connt THEN ti:=0 ELSE DEC(ti, connt) END;
               Append(h," conntime ");
               TimeToStr(ti, t); 
         */
         aprsstr_Append(h, 256ul, t, 256ul);
      }
      aprsstr_Append(h, 256ul, "\012", 2ul);
   }
   logline(1L, h, 256ul);
} /* end showlogout() */


static char vias(char dat[], uint32_t dat_len, pTCPSOCK to)
{
   uint32_t j;
   uint32_t i0;
   uint32_t b;
   uint32_t a;
   struct FILTERS * anonym;
   if (to->filters.viacalls[0U][0U]==0 || dat[0UL]==0) return 0;
   b = 0UL;
   do {
      a = b+1UL;
      do {
         ++b;
         if (b>dat_len-1 || dat[b]==0) return 0;
      } while (!(dat[b]==',' || dat[b]==':'));
      /* not normal data */
      if (a>1UL) {
         j = 0UL;
         { /* with */
            struct FILTERS * anonym = &to->filters;
            while (j<=7UL && anonym->viacalls[j][0U]) {
               i0 = 0UL;
               for (;;) {
                  if (a+i0>=b || dat[a+i0]=='*') {
                     /* end of word 1 ignore h-bit "*" */
                     if ((i0>9UL || anonym->viacalls[j][i0]==0)
                || anonym->viacalls[j][i0]
                =='*' && (i0+1UL>9UL || anonym->viacalls[j][i0+1UL]==0)) {
                        return 1;
                     }
                     else break;
                  }
                  if (i0>9UL || anonym->viacalls[j][i0]==0) {
                     break; /* end of word 2 */
                  }
                  if (anonym->viacalls[j][i0]
                =='*' && (i0+1UL>9UL || anonym->viacalls[j][i0+1UL]==0)) {
                     return 1;
                  }
                  /* word 2 "*" at end */
                  if (anonym->viacalls[j][i0]!='*' && anonym->viacalls[j][i0]
                !=dat[a+i0]) break;
                  ++i0;
               }
               ++j; /* next call in table */
            }
         }
      }
   } while (dat[b]!=':');
   return 0;
} /* end vias() */


static char entrypoint(char dat[], uint32_t dat_len,
                pTCPSOCK to)
{
   uint32_t j;
   uint32_t i0;
   uint32_t b;
   uint32_t a;
   struct FILTERS * anonym;
   if (to->filters.entrycalls[0U][0U]==0) return 0;
   i0 = 0UL;
   for (;;) {
      if ((i0>(dat_len-1)-5UL || dat[i0]==0) || dat[i0]==':') return 0;
      /* no qA */
      if (((dat[i0]==',' && dat[i0+1UL]=='q') && dat[i0+2UL]=='A')
                && dat[i0+4UL]==',') {
         /* ,qA., */
         a = i0+5UL;
         b = a;
         while (((b<dat_len-1 && dat[b]) && dat[b]!=':') && dat[b]!=',') ++b;
         break;
      }
      ++i0;
   }
   j = 0UL;
   { /* with */
      struct FILTERS * anonym = &to->filters;
      while (j<=7UL && anonym->entrycalls[j][0U]) {
         i0 = 0UL;
         for (;;) {
            if (a+i0>=b) {
               /* end of word 1 */
               if ((i0>9UL || anonym->entrycalls[j][i0]==0)
                || anonym->entrycalls[j][i0]
                =='*' && (i0+1UL>9UL || anonym->entrycalls[j][i0+1UL]==0)) {
                  return 1;
               }
               else break;
            }
            if (i0>9UL || anonym->entrycalls[j][i0]==0) break;
            if (anonym->entrycalls[j][i0]
                =='*' && (i0+1UL>9UL || anonym->entrycalls[j][i0+1UL]==0)) {
               return 1;
            }
            /* word 2 "*" at end */
            if (anonym->entrycalls[j][i0]!='*' && anonym->entrycalls[j][i0]
                !=dat[a+i0]) break;
            ++i0;
         }
         /*
                 WHILE (a+i<b) & ((entrycalls[j][i]="*")
                OR (entrycalls[j][i]=dat[a+i])) DO
                   INC(i);
                   IF (a+i>=b) & ((i>HIGH(entrycalls[0]))
                OR (entrycalls[j][i]=0C)) THEN RETURN TRUE END;
         
         */
         ++j;
      }
   }
   return 0;
} /* end entrypoint() */


static char prefix(char dat[], uint32_t dat_len,
                const MONCALL pa[], uint32_t pa_len,
                char fullmatch)
{
   uint32_t j;
   uint32_t i0;
   char w;
   char eos;
   if (pa[0UL][0U]==0) return 0;
   j = 0UL;
   while (j<=pa_len-1 && pa[j][0U]) {
      i0 = 0UL;
      for (;;) {
         w = pa[j][i0]=='*';
         if (pa[j][i0]!=dat[i0] && !w) break;
         ++i0;
         eos = dat[i0]=='>';
         if (i0>9UL || pa[j][i0]==0) {
            if ((!eos && fullmatch) && !w) break;
            else return 1;
         }
         if (eos) break;
      }
      ++j;
   }
   return 0;
} /* end prefix() */


static char destcallfilt(char dat[], uint32_t dat_len,
                pTCPSOCK to)
{
   uint32_t b;
   uint32_t a;
   uint32_t j;
   uint32_t i0;
   struct FILTERS * anonym;
   if (to->filters.destcalls[0U][0U]==0) return 0;
   a = 0UL;
   while (dat[a]!='>') {
      /* begin of dest call ">" */
      if (a>=dat_len-1 || dat[a]==0) return 0;
      /* not normal data */
      ++a;
   }
   ++a;
   b = a;
   for (;;) {
      if (b>dat_len-1 || dat[b]==0) return 0;
      /* not normal data */
      if (dat[b]==',' || dat[b]==':') break;
      ++b;
   }
   b -= a; /* len of dest call */
   j = 0UL;
   { /* with */
      struct FILTERS * anonym = &to->filters;
      while (j<=7UL && anonym->destcalls[j][0U]) {
         i0 = 0UL;
         while (i0<b && (anonym->destcalls[j][i0]
                =='*' || anonym->destcalls[j][i0]==dat[i0+a])) {
            ++i0;
            if (i0>9UL || anonym->destcalls[j][i0]==0) return 1;
         }
         ++j;
      }
   }
   return 0;
} /* end destcallfilt() */


static char objectfilt(char dat[], uint32_t dat_len,
                pTCPSOCK to)
{
   uint32_t k;
   uint32_t j;
   uint32_t i0;
   char eos;
   char w;
   struct FILTERS * anonym;
   if (to->filters.objects[0U][0U]==0) return 0;
   i0 = 0UL;
   while (dat[i0]!=':') {
      /* begin data */
      if (i0>=dat_len-1 || dat[i0]==0) return 0;
      /* not normal data */
      ++i0;
   }
   ++i0;
   if (dat[i0]!=';') return 0;
   /* not object */
   ++i0; /* object name */
   j = 0UL;
   { /* with */
      struct FILTERS * anonym = &to->filters;
      while (j<=7UL && anonym->objects[j][0U]) {
         k = 0UL;
         for (;;) {
            w = anonym->objects[j][k]=='*';
            if (!w && anonym->objects[j][k]!=dat[i0+k]) break;
            ++k;
            if (i0+k>dat_len-1) break;
            eos = (uint8_t)dat[i0+k]<=' ';
            if (k>9UL || anonym->objects[j][k]==0) {
               if (eos || w) return 1;
               else break;
            }
            if (eos) break;
         }
         ++j;
      }
   }
   return 0;
} /* end objectfilt() */


static char typ(struct POSCALL * posc, char dat[],
                uint32_t dat_len, pTCPSOCK to, char * t)
{
   uint32_t ii;
   uint32_t j;
   uint32_t i0;
   char c;
   if (to->filters.typs[0U]==0) return 0;
   i0 = 0UL;
   j = 0UL;
   c = 0;
   for (;;) {
      if (i0>(dat_len-1)-2UL || dat[i0]==0) return 0;
      /* no typ */
      if (dat[i0]==':') {
         /* outer typ */
         c = dat[i0+1UL];
         if (c!='}') {
            i0 += 2UL; /* inner typchar found*/
            break;
         }
         if (aprsstr_InStr(to->filters.typs, 13ul, "3", 2ul)>=0L) return 1;
         /* 3rd party frame */
         j = i0+2UL; /* start of inner frame */
      }
      ++i0;
   }
   if (c==':') {
      ii = i0;
      while (ii<dat_len-1 && dat[j]==dat[ii]) {
         ++ii; /* msg to itself */
         ++j;
      }
      if (dat[j]=='>') {
         c = 'T'; /* gets type telemetry */
         *t = 'T';
      }
   }
   if (aprspos_posvalid(posc->pos) && aprsstr_InStr(to->filters.typs, 13ul, "\
p", 2ul)>=0L) return 1;
   /* position frame */
   if (c==';' && aprsstr_InStr(to->filters.typs, 13ul, "o", 2ul)>=0L) {
      return 1;
   }
   /* object */
   if (c==')') return aprsstr_InStr(to->filters.typs, 13ul, "i", 2ul)>=0L;
   /* item */
   if (c==':') {
      if (dat[i0+10UL]=='?') {
         return aprsstr_InStr(to->filters.typs, 13ul, "q", 2ul)>=0L;
      }
      else if ((((i0<(dat_len-1)-6UL && dat[i0]=='N') && dat[i0+1UL]=='W')
                && dat[i0+2UL]=='S') && dat[i0+3UL]=='-') {
         /* query */
         return aprsstr_InStr(to->filters.typs, 13ul, "n", 2ul)>=0L;
      }
      else {
         /* NWS bulletin */
         return aprsstr_InStr(to->filters.typs, 13ul, "m", 2ul)>=0L;
      }
   }
   /* message */
   if (c=='>') return aprsstr_InStr(to->filters.typs, 13ul, "s", 2ul)>=0L;
   /* status */
   if (c=='T') return aprsstr_InStr(to->filters.typs, 13ul, "t", 2ul)>=0L;
   /* telemetry */
   if (c=='{') return aprsstr_InStr(to->filters.typs, 13ul, "u", 2ul)>=0L;
   /* user defined */
   if (aprsstr_InStr(to->filters.typs, 13ul, "w", 2ul)>=0L) {
      /* test if wx */
      j = 0UL;
      if (c=='_') return 1;
      /* positionless wx */
      if (c=='!' || c=='=') {
         if ((uint8_t)dat[i0]>='0' && (uint8_t)dat[i0]<='9') j = i0+18UL;
         else j = i0+9UL;
      }
      else if (c=='/' || c=='@') {
         if ((uint8_t)dat[i0+7UL]>='0' && (uint8_t)dat[i0+7UL]<='9') {
            j = i0+25UL;
         }
         else j = i0+16UL;
      }
      else if (c==';') j = i0+35UL;
      /*WrStr("<<<"); WrStr(dat[j]);WrStrLn(">>>");  */
      if (((j>0UL && j<dat_len-1) && dat[j]=='_')
                && aprspos_posvalid(posc->pos)) return 1;
   }
   return 0;
} /* end typ() */


static char Filter(pTCPSOCK to, struct POSCALL posc,
                const char dat[], uint32_t dat_len)
{
   char pass;
   pass = 0;
   if (vias(dat, dat_len, to)) {
      if (to->filters.notvia) return 0;
      pass = 1; /* exclusion overloads */
   }
   if (entrypoint(dat, dat_len, to)) {
      if (to->filters.notentry) return 0;
      pass = 1;
   }
   if (destcallfilt(dat, dat_len, to)) {
      if (to->filters.notdestcall) return 0;
      pass = 1;
   }
   if (prefix(dat, dat_len, to->filters.bud, 8ul, 1)) {
      if (to->filters.notbud) return 0;
      pass = 1;
   }
   if (prefix(dat, dat_len, to->filters.prefixes, 8ul, 0)) {
      if (to->filters.notprefix) return 0;
      pass = 1;
   }
   if (objectfilt(dat, dat_len, to)) {
      if (to->filters.notobject) return 0;
      pass = 1;
   }
   if (typ(&posc, dat, dat_len, to, &posc.typ0)) {
      if (to->filters.nottyps) return 0;
      pass = 1;
   }
   if (pass || posc.typ0==':') return 1;
   /* pass messages and ack */
   if (to->filters.typ0=='m') {
      return (aprspos_posvalid(to->user.pos) && aprspos_posvalid(posc.pos))
                && aprspos_distance(to->user.pos,
                posc.pos)<=to->filters.radius;
   }
   if (to->filters.typ0=='r') {
      /*
      IO.WrStr("l,b,l,b,d,r:"); IO.WrFixed(to^.filters.base.lat/RAD, 4,10);
                IO.WrFixed(to^.filters.base.long/RAD, 4,10);  
      IO.WrFixed(posc.pos.lat/RAD, 4,10);
                IO.WrFixed(posc.pos.long/RAD, 4,10);
      IO.WrFixed(distance(to^.filters.base, posc.pos), 4,10);
                IO.WrFixed(to^.filters.radius, 4,10);
      IO.WrLn;
      */
      return aprspos_posvalid(posc.pos) && aprspos_distance(to->filters.base,
                 posc.pos)<=to->filters.radius;
   }
   if (to->filters.typ0=='a') {
      /*
      IO.WrStr("l,b,l,b,l,b,d,r:"); 
      IO.WrFixed(to^.filters.base.lat/RAD, 4,10);
                IO.WrFixed(to^.filters.base.long/RAD, 4,10);  
      IO.WrFixed(to^.filters.edge.lat/RAD, 4,10);
                IO.WrFixed(to^.filters.edge.long/RAD, 4,10);
      IO.WrFixed(posc.pos.lat/RAD, 4,10);
                IO.WrFixed(posc.pos.long/RAD, 4,10);
      IO.WrLn;
      */
      return (((aprspos_posvalid(posc.pos)
                && to->filters.base.lat>=posc.pos.lat)
                && to->filters.base.long0<=posc.pos.long0)
                && to->filters.edge.lat<=posc.pos.lat)
                && to->filters.edge.long0>=posc.pos.long0;
   }
   return !datafilter;
} /* end Filter() */


static void sendtcpbuf(pTCPSOCK to)
{
   int32_t i0;
   int32_t res0;
   struct TCPSOCK * anonym;
   int32_t tmp;
   { /* with */
      struct TCPSOCK * anonym = to;
      res0 = sendsock(anonym->fd, anonym->tbuf, anonym->tlen);
      if (res0>0L) {
         tmp = anonym->tlen-1L;
         i0 = res0;
         if (i0<=tmp) for (;; i0++) {
            anonym->tbuf[i0-res0] = anonym->tbuf[i0];
            if (i0==tmp) break;
         } /* end for */
         anonym->tlen -= res0;
      }
   }
} /* end sendtcpbuf() */


static void Sendtcp(pTCPSOCK to, const FRAMEBUF buf)
{
   int32_t i0;
   int32_t len;
   struct TCPSOCK * anonym;
   int32_t tmp;
   len = (int32_t)aprsstr_Length(buf, 512ul);
   { /* with */
      struct TCPSOCK * anonym = to;
      if (anonym->tlen+len>=1024L) sendtcpbuf(to);
      if (anonym->tlen+len<1024L) {
         tmp = len-1L;
         i0 = 0L;
         if (i0<=tmp) for (;; i0++) {
            anonym->tbuf[anonym->tlen] = buf[i0];
            ++anonym->tlen;
            if (i0==tmp) break;
         } /* end for */
         ++anonym->txframes;
         anonym->txbytes += (uint32_t)len; /* 32 bit overflow */
         if (anonym->txbytes<(uint32_t)len) ++anonym->txbytesh;
      }
      else {
         ++anonym->losttxframes;
         if (verb) osi_WrStrLn("tx buf overflow", 16ul);
      }
   }
} /* end Sendtcp() */


static char filt(int32_t outer, char b[], uint32_t b_len,
                const char s[], uint32_t s_len)
{
   int32_t n;
   n = aprsstr_InStr(b, b_len, s, s_len);
   return n>=0L && n<outer;
} /* end filt() */


static void NetToRf(const char b[], uint32_t b_len, uint32_t rfport)
/* send as 3rd party frame to rf */
{
   FRAMEBUF tb;
   uint32_t viaused;
   uint32_t viaend;
   uint32_t len;
   uint32_t i0;
   int32_t ii;
   int32_t outer;
   char done;
   outer = aprsstr_InStr(b, b_len, ":", 2ul);
                /* test if gate to rf allowed */
   if (outer<0L || filt(outer, b, b_len, "qAX", 4ul)) return;
   viaend = (uint32_t)outer;
   while (outer<(int32_t)(b_len-1) && (b[outer]!=':' || b[outer+1L]=='}')) {
      ++outer;
   }
   if ((filt(outer, b, b_len, "TCPXX", 6ul) || filt(outer, b, b_len,
                "NOGATE", 7ul)) || filt(outer, b, b_len, "RFONLY", 7ul)) {
      return;
   }
   aprsstr_Assign(tb, 512ul, viacall, 10ul);
   aprsstr_Append(tb, 512ul, ">APNL51", 8ul);
   if (nettorfpath[0U]) {
      if (nettorfpath[0U]=='-') {
         /* append ssid to dest call*/
         do {
            aprsstr_Append(tb, 512ul, (char *) &nettorfpath[0U], 1u/1u);
            aprsstr_Delstr(nettorfpath, 81ul, 0UL, 1UL);
         } while (!((uint8_t)nettorfpath[0U]<'0' || (uint8_t)nettorfpath[0U]>'9'));
         if (nettorfpath[0U]==',') {
            aprsstr_Delstr(nettorfpath, 81ul, 0UL, 1UL);
         }
      }
      if (nettorfpath[0U]) {
         aprsstr_Append(tb, 512ul, ",", 2ul);
         aprsstr_Append(tb, 512ul, nettorfpath, 81ul);
      }
   }
   aprsstr_Append(tb, 512ul, ":}", 3ul);
   viaused = 0UL;
   while (viaused<viaend && b[viaused]!=',') ++viaused;
   ii = aprsstr_InStr(b, b_len, ",TCPIP", 7ul);
   if (ii<0L || ii>(int32_t)viaend) {
      /* remove all via if via TCPIP */
      i0 = viaused;
      while (i0<viaend && b[i0]!='*') ++i0;
      if (i0<viaend) viaused = i0;
   }
   len = aprsstr_Length(tb, 512ul);
   i0 = 0UL;
   /*  WHILE (b[i]<>",") & (i<viaend) & (len<=HIGH(tb)) DO           (* original head remove via*) */
   while (i0<viaused && len<=511UL) {
      /* original head with used via*/
      tb[len] = b[i0];
      ++i0;
      ++len;
   }
   if (len<=511UL) tb[len] = 0;
   aprsstr_Append(tb, 512ul, ",TCPIP,", 8ul);
   aprsstr_Append(tb, 512ul, viacall, 10ul);
   aprsstr_Append(tb, 512ul, "*", 2ul);
   len = aprsstr_Length(tb, 512ul);
   i0 = viaend;
   while ((i0<=b_len-1 && b[i0]) && len<=511UL) {
      /* original data part */
      tb[len] = b[i0];
      ++i0;
      ++len;
   }
   if (len<=511UL) tb[len] = 0;
   i0 = 0UL;
   while (((i0<511UL && tb[i0]) && tb[i0]!='\015') && tb[i0]!='\012') ++i0;
   tb[i0] = 0;
   done = Sendudp(tb, rfport, 0);
} /* end NetToRf() */


static void Sendall(const FRAMEBUF buf, int32_t fromfd,
                const struct POSCALL posc)
{
   pTCPSOCK t;
   pUDPSOCK u;
   uint32_t uport;
   /*
       len:INTEGER;
   */
   if (buf[0U]) {
      t = tcpsocks;
      while (t) {
         if (((t->fd!=fromfd && t->service!='W') && t->connt>0UL)
                && (t->service!='S' || Filter(t, posc, buf, 512ul))) {
            Sendtcp(t, buf);
         }
         t = t->next;
      }
      if (fromfd>0L && aprspos_posvalid(posc.pos)) {
         u = udpsocks;
         uport = 1UL;
         while (u) {
            if (u->torfradius>0.0f && (u->torfradius>=20000.0f || aprspos_posvalid(home)
                 && aprspos_distance(home, posc.pos)<u->torfradius)) {
               NetToRf(buf, 512ul, uport);
            }
            ++uport;
            u = u->next;
         }
      }
      /*
          IF rfgateport>0 THEN 
            len:=Length(buf);
            len:=udp.udpsend(udpsock.fd, buf, len, rfgateport, rfgateip);
          END;
      */
      if (rawlogname[0U]) writerawlog(buf);
   }
} /* end Sendall() */


static char cmpfrom(const char a[], uint32_t a_len,
                uint32_t from, const char b[], uint32_t b_len)
{
   uint32_t i0;
   i0 = 0UL;
   while (from<=a_len-1 && b[i0]) {
      if (a[from]!=b[i0]) return 0;
      ++i0;
      ++from;
   }
   return 1;
} /* end cmpfrom() */

#define udpgate4_MSYM "\\"


static void beaconmacros(char s[], uint32_t s_len)
{
   uint32_t i0;
   int32_t j;
   int32_t len;
   char ns[256];
   char ds[256];
   char fn[1024];
   int32_t f;
   i0 = 0UL;
   ns[0U] = 0;
   while (i0<s_len-1 && s[i0]) {
      if (s[i0]=='\\' && s[i0+1UL]=='\\') {
         i0 += 2UL;
         if (s[i0]=='z') {
            /* insert day, hour, min */
            aprsstr_DateToStr(osic_time(), ds, 256ul);
            ds[0U] = ds[8U];
            ds[1U] = ds[9U];
            ds[2U] = ds[11U];
            ds[3U] = ds[12U];
            ds[4U] = ds[14U];
            ds[5U] = ds[15U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i0]=='h') {
            /* insert hour, min, s */
            aprsstr_DateToStr(osic_time(), ds, 256ul);
            ds[0U] = ds[11U];
            ds[1U] = ds[12U];
            ds[2U] = ds[14U];
            ds[3U] = ds[15U];
            ds[4U] = ds[17U];
            ds[5U] = ds[18U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i0]=='v') {
            /* insert version */
            aprsstr_Append(ns, 256ul, "udpgate 0.68", 13ul);
         }
         else if (s[i0]==':') {
            /* insert file */
            fn[0U] = 0;
            ++i0;
            while ((i0<s_len-1 && s[i0]) && s[i0]!=':') {
               aprsstr_Append(fn, 1024ul, (char *) &s[i0], 1u/1u);
               ++i0;
            }
            f = osi_OpenRead(fn, 1024ul);
            if (f>=0L) {
               len = osi_RdBin(f, (char *)ds, 256u/1u, 255UL);
               osic_Close(f);
               j = 0L;
               while (((j<len && ds[j]!='\015') && ds[j]!='\012') && ds[j]) {
                  aprsstr_Append(ns, 256ul, (char *) &ds[j], 1u/1u);
                  ++j;
               }
            }
            else {
               if (verb) {
                  osic_WrLn();
                  osi_WrStrLn("beacon macro file not readable ", 32ul);
               }
               s[0UL] = 0;
               return;
            }
         }
         else if (s[i0]=='\\') aprsstr_Append(ns, 256ul, "\\\\", 3ul);
         else {
            if (verb) {
               osic_WrLn();
               osi_WrStrLn("bad beacon macro ", 18ul);
            }
            s[0UL] = 0;
            return;
         }
      }
      else aprsstr_Append(ns, 256ul, (char *) &s[i0], 1u/1u);
      ++i0;
   }
   aprsstr_Assign(s, s_len, ns, 256ul);
} /* end beaconmacros() */


static void AppQ(char h[], uint32_t h_len, const char q[],
                uint32_t q_len, char appcall)
{
   aprsstr_Append(h, h_len, ">APNL51,", 9ul);
   aprsstr_Append(h, h_len, q, q_len);
   if (appcall) aprsstr_Append(h, h_len, servercall, 10ul);
   aprsstr_Append(h, h_len, ":", 2ul);
} /* end AppQ() */


static void Netbeacon(char h[], uint32_t h_len, char qai,
                char withpath)
{
   int32_t valt0;
   int32_t i0;
   int32_t f;
   char h1[4096];
   uint32_t j;
   uint32_t vspeed0;
   uint32_t vcourse0;
   char vsymt0;
   char vsym0;
   char postyp;
   h[0UL] = 0;
   if (servercall[0U] && netbeaconfn[0U]) {
      f = osi_OpenRead(netbeaconfn, 1024ul);
      if (f>=0L) {
         i0 = osi_RdBin(f, (char *)h1, 4096u/1u, 4095UL);
         if (i0>=0L) {
            h1[i0] = 0;
            while (h1[0U]=='#') {
               /* comment */
               i0 = 0L;
               while ((uint8_t)h1[i0]>=' ') ++i0;
               while (h1[i0] && (uint8_t)h1[i0]<' ') ++i0;
               aprsstr_Delstr(h1, 4096ul, 0UL, (uint32_t)i0);
                /* del comment line */
            }
            i0 = 0L;
            while ((uint8_t)h1[i0]>=' ') ++i0;
            h1[i0] = 0;
            beaconmacros(h1, 4096ul);
            if (withpath) {
               aprsstr_Assign(h, h_len, servercall, 10ul);
               if (qai) AppQ(h, h_len, "TCPIP*,qAI,", 12ul, 1);
               else AppQ(h, h_len, "TCPIP*", 7ul, 0);
               aprsstr_Append(h, h_len, h1, 4096ul);
               aprsstr_Append(h, h_len, "\015\012", 3ul);
            }
            else aprsstr_Assign(h, h_len, h1, 4096ul);
         }
         osic_Close(f);
         j = 0UL;
         while (j<(h_len-1)-1UL && h[j]!=':') ++j;
         aprspos_GetPos(&home, &vspeed0, &vcourse0, &valt0, &vsym0, &vsymt0,
                h, h_len, 0UL, j+1UL, h1, 4096ul, &postyp);
                /* find server position */
         if (verb && !aprspos_posvalid(home)) {
            osi_WrStrLn("netbeacon has no valid position", 32ul);
         }
      }
      else if (verb) osi_WrStrLn("netbeacon file open error", 26ul);
   }
} /* end Netbeacon() */


static void Timebeacon(pTCPSOCK cp)
{
   FRAMEBUF h;
   char qai;
   if (cp->service!='W' && Watchclock(&cp->beacont, netbeaconintervall)) {
      qai = 1;
      if (cp->service=='G') {
         if (qas==1UL || qas>qasc+1UL) {
            qai = 0;
            ++qasc;
         }
         else qasc = 0UL;
      }
      Netbeacon(h, 512ul, qai, 1);
      if (h[0U]) Sendtcp(cp, h);
   }
} /* end Timebeacon() */


static void Showmsg(pMESSAGE m)
{
   struct MESSAGE * anonym;
   { /* with */
      struct MESSAGE * anonym = m;
      osic_WrINT32(anonym->retryc, 3UL);
      osic_WrINT32((uint32_t)anonym->acked, 1UL);
      if (anonym->txtime) osic_WrINT32(systime-anonym->txtime, 5UL);
      else osi_WrStr(" notx", 6ul);
      osi_WrStr(":", 2ul);
      osi_WrStr(anonym->from, 10ul);
      osi_WrStr(">", 2ul);
      osi_WrStr(anonym->to, 10ul);
      osi_WrStr("|", 2ul);
      osi_WrStr(anonym->ack, 5ul);
      osi_WrStr(anonym->replyack, 2ul);
      osi_WrStr("|", 2ul);
      osi_WrStr(anonym->text, 68ul);
      osi_WrStrLn("|", 2ul);
   }
} /* end Showmsg() */


static void cpcall(char s[], uint32_t s_len, const char h[],
                uint32_t h_len, uint32_t pr)
{
   char c;
   uint32_t i0;
   i0 = 0UL;
   for (;;) {
      c = h[pr];
      if ((((((i0>=s_len-1 || c==0) || c=='>') || c=='*') || c==':')
                || c==',') || c==' ') break;
      s[i0] = c;
      ++i0;
      ++pr;
   }
   s[i0] = 0;
} /* end cpcall() */


static char SendMsg(pMESSAGE mp, uint32_t torf)
{
   FRAMEBUF s;
   uint32_t i0;
   MONCALL h;
   char done;
   struct POSCALL poscall0;
   uint8_t srcpath;
   pUDPSOCK uds;
   struct MESSAGE * anonym;
   /* rf: OE2XZR-10>RFONLY,OE2XZR-10:}
                OE0AAA>TCPIP,OE2XZR-10::OE2WAO-12:hallo{123 */
   /* ip: OE0AAA>TCPIP,OE2XZR-10::OE2WAO-12:hallo{123 */
   if (verb) {
      if (viacall[0U]==0) osi_WrStr("<no via call, to msg tx>", 25ul);
      else if (torf) osi_WrStr("<send rf>", 10ul);
      else osi_WrStr("<send net>", 11ul);
      Showmsg(mp);
   }
   if (mp->ackackt || viacall[0U]==0) return 0;
   done = 0;
   /*
     tcp:=udpsock.fd<0;
     s[0]:=0C;
     IF (torf<>0) & tcp THEN
       Append(s, viacall);
       Append(s, ">RFONLY,");
       Append(s, servercall);
       Append(s, ":}");
     END;
     IF mp^.acked THEN Append(s, mp^.to) ELSE Append(s, mp^.from) END;
   */
   if (mp->acked) {
      aprsstr_Assign(s, 512ul, mp->to, 10ul);
      srcpath = mp->acksrc;
   }
   else {
      aprsstr_Assign(s, 512ul, mp->from, 10ul);
      srcpath = mp->src;
   }
   if (torf) {
      aprsstr_Append(s, 512ul, ">", 2ul);
      if (srcpath==udpgate4_NET) aprsstr_Append(s, 512ul, netdestcall, 10ul);
      else aprsstr_Append(s, 512ul, rfdestcall, 10ul);
      aprsstr_Append(s, 512ul, ",", 2ul);
      aprsstr_Append(s, 512ul, viacall, 10ul);
      aprsstr_Append(s, 512ul, "*::", 4ul);
   }
   else {
      if (mp->queryrep) AppQ(s, 512ul, "qAS,", 5ul, 1);
      else AppQ(s, 512ul, "NOGATE*,qAS,", 13ul, 1);
      aprsstr_Append(s, 512ul, ":", 2ul);
   }
   if (mp->acked) memcpy(h,mp->from,10u);
   else memcpy(h,mp->to,10u);
   for (i0 = aprsstr_Length(h, 10ul); i0<=8UL; i0++) {
      aprsstr_Append(h, 10ul, " ", 2ul);
   } /* end for */
   aprsstr_Append(s, 512ul, h, 10ul);
   if (mp->acked) {
      if (mp->reject) aprsstr_Append(s, 512ul, ":rej", 5ul);
      else aprsstr_Append(s, 512ul, ":ack", 5ul);
   }
   else {
      aprsstr_Append(s, 512ul, ":", 2ul);
      { /* with */
         struct MESSAGE * anonym = mp;
         i0 = 0UL;
         while (i0<=67UL && anonym->text[i0]) {
            /* blank not allowed chars */
            if (((anonym->text[i0]=='|' || anonym->text[i0]=='~')
                || anonym->text[i0]=='{') || (uint8_t)anonym->text[i0]<' ')
                 anonym->text[i0] = ' ';
            ++i0;
         }
         aprsstr_Append(s, 512ul, anonym->text, 68ul);
         if (anonym->ack[0U]) {
            aprsstr_Append(s, 512ul, "{", 2ul);
         }
      }
   }
   aprsstr_Append(s, 512ul, mp->ack, 5ul);
   aprsstr_Append(s, 512ul, mp->replyack, 2ul);
   ++mp->retryc;
   poscall0.typ0 = ':';
   if (torf) {
      /*NOT tcp &*/
      done = Sendudp(s, torf, 1);
   }
   else {
      aprsstr_Append(s, 512ul, "\015\012", 3ul);
      Sendall(s, 0L, poscall0);
   }
   mp->txport = torf;
   if (verb || logframes>1L) {
      uds = porttosock(torf);
      showframe(-4L, 0, uds, s, 512ul, poscall0.pos);
   }
   if (mp->acked) {
      if (torf) ++mp->ack2rf;
      else ++mp->ack2net;
   }
   else if (torf) ++mp->msg2rf;
   else ++mp->msg2net;
   mp->txtime = systime;
   return done;
} /* end SendMsg() */


static uint32_t FindUserHeard(pHEARD * ph, const MONCALL c,
                uint32_t * rfport)
/* never heard or too long = 0 else time+1 */
{
   while (*ph && (*ph)->mhtime+heardtimew>systime) {
      if (aprsstr_StrCmp((*ph)->call, 10ul, c, 10ul)) {
         if ((*ph)->mhtime<=systime) {
            *rfport = (*ph)->fromrx;
            return (1UL+systime)-(*ph)->mhtime;
         }
         else return 0UL;
      }
      *ph = (*ph)->next;
   }
   return 0UL;
} /* end FindUserHeard() */


static uint32_t FindHeard(pHEARD ph, const MONCALL c, uint32_t * rfport)
/* never heard or too long = 0 else time+1 */
{
   return FindUserHeard(&ph, c, rfport);
} /* end FindHeard() */


static char Heard(const char b[], uint32_t b_len, MONCALL from,
                char * trust)
{
   uint32_t len;
   uint32_t i0;
   uint32_t p;
   /* from>to,path,gate:...*/
   p = 0UL;
   while (p<9UL && b[p]!='>') {
      from[p] = b[p]; /* get user call */
      ++p;
   }
   from[p] = 0;
   i0 = aprsstr_Length(b, b_len);
   len = 0UL;
   while (len<i0 && b[len]!=':') ++len;
   *trust = b[len+1UL]!=':' && b[len+1UL]!='}';
                /* not from physical sender */
   /*
     IF NOT udp THEN                                               (* frame is from aprsis *)
       lp:=Length(rfpath);
       IF (lp=0) OR (lp>len) THEN RETURN END;
                (* rfpath not fitting *)
   
       DEC(len, lp);                                               (* cut rfpath *)
       FOR i:=0 TO lp-1 DO
         IF b[len+i]<>rfpath[i] THEN RETURN END;
                (* wrong rfpath *)
       END;
     END;
   */
   while (p<len && b[p]!=',') ++p;
   if (b[p]==',') {
      /* get first via */
      ++p;
      i0 = p;
      while (p<len && b[p]!=',') ++p;
      if (b[p-1UL]=='*') return 0;
      /* is repeated */
      if (((uint8_t)b[p-1UL]>='0' && (uint8_t)b[p-1UL]<='9')
                && b[p-1UL]!=b[p-3UL]) return 0;
      /* WIDEn-x with n<>x */
      /*
          ELSIF ((p-i<=3) OR (b[p-2]<>"-") OR (b[p-1]<>b[p-3])
                OR (b[p-1]<"1") OR (b[p-1]>"7"))
            & ((p-i<>4) OR (b[i]<>"E") OR (b[i+1]<>"C") OR (b[i+2]<>"H")
                OR (b[i+3]<>"O"))
            & ((p-i<>4) OR (b[i]<>"G") OR (b[i+1]<>"A") OR (b[i+2]<>"T")
                OR (b[i+3]<>"E"))
            & ((p-i<>5) OR (b[i]<>"R") OR (b[i+1]<>"E") OR (b[i+2]<>"L")
                OR (b[i+3]<>"A") OR (b[i+4]<>"Y"))
          THEN RETURN FALSE END; (* strict direct heard via n-M or GATE or ECHO or RELAY *)
      */
      while (p<len) {
         /* more vias */
         ++p;
         while (p<len && b[p]!=',') ++p;
         if (b[p-1UL]=='*') return 0;
      }
   }
   /* is repeated */
   return 1;
} /* end Heard() */


static void IncHeard(pHEARD ph, char pk, char jnk,
                uint32_t livetime)
{
   uint32_t t;
   struct HEARD * anonym;
   struct _1 * anonym0;
   if (livetime>0UL) {
      { /* with */
         struct HEARD * anonym = ph;
         t = systime/(livetime/48UL)+49UL;
         /*WrInt(t, 10); WrInt(cntt, 10); WrInt(ORD(up), 10); WrLn; */
         if (anonym->cntt+49UL<t) anonym->cntt = t-49UL;
         while (anonym->cntt<t) {
            ++anonym->cntt;
            { /* with */
               struct _1 * anonym0 = &anonym->cnt[anonym->cntt%49UL];
               anonym0->pack = 0U;
               anonym0->junk = 0U;
            }
         }
         t = anonym->cntt%49UL;
         if (pk && anonym->cnt[t].pack<65535U) ++anonym->cnt[t].pack;
         if (jnk && anonym->cnt[t].junk<65535U) ++anonym->cnt[t].junk;
      }
   }
/*FOR t:=0 TO HIGH(ph^.cnt) DO WrInt(ORD(ph^.cnt[t]), 2) END; WrLn; */
} /* end IncHeard() */


static void Wx(float * data, char * typ0, char b[],
                uint32_t b_len, uint32_t p)
{
   int32_t t;
   char sig;
   if (((b[p]!='/' && b[p]!='@') && b[p]!='!') && b[p]!='=') return;
   if (b[p]=='/' || b[p]=='@') p += 7UL;
   ++p;
   if ((uint8_t)b[p]>='0' && (uint8_t)b[p]<='9') p += 30UL;
   else if ((uint8_t)b[p]>='/') p += 17UL;
   if (b[p]!='t') return;
   t = 0L;
   sig = 0;
   if (b[p+1UL]=='-') sig = 1;
   else if ((uint8_t)b[p+1UL]>='0' && (uint8_t)b[p+1UL]<='9') {
      t = (int32_t)(((uint32_t)(uint8_t)b[p+1UL]-48UL)*100UL);
   }
   else return;
   if ((uint8_t)b[p+2UL]>='0' && (uint8_t)b[p+2UL]<='9') {
      t += (int32_t)(((uint32_t)(uint8_t)b[p+2UL]-48UL)*10UL);
   }
   else return;
   if ((uint8_t)b[p+3UL]>='0' && (uint8_t)b[p+3UL]<='9') {
      t += (int32_t)((uint32_t)(uint8_t)b[p+3UL]-48UL);
   }
   else return;
   if (sig) t = -t;
   *typ0 = 'C';
   *data = X2C_DIVR((float)(t-32L),1.8f);
} /* end Wx() */


static void DirectPos(const char b[], uint32_t b_len,
                struct aprsstr_POSITION * pos, char * sym,
                char * symt, char * typ0, float * data)
{
   uint32_t my;
   uint32_t m;
   uint32_t p;
   struct aprsstr_POSITION posn;
   char ch;
   char symt1;
   char sym1;
   char postyp;
   MONCALL dc;
   FRAMEBUF com;
   /* OE0AAA>ABCDEF-2,....:...*/
   /*        ^mice         ^payload*/
   *typ0 = 'U';
   p = 0UL;
   while (p<b_len-1 && b[p]!='>') ++p;
   m = p+1UL;
   while (p<b_len-1 && b[p]!=':') ++p;
   ++p;
   my = 0UL;
   while (b[p]=='}') {
      my = p+1UL; /* inner src call */
      while ((p<=b_len-1 && b[p]) && b[p]!=':') ++p;
      ++p;
   }
   if (p<b_len-1 && b[p]!=';') {
      /* objects say wrong pos */
      vspeed = X2C_max_longcard;
      aprspos_GetPos(&posn, &vspeed, &vcourse, &valt, &sym1, &symt1, b,
                b_len, m, p, com, 512ul, &postyp);
      dc[0U] = b[m];
      dc[1U] = b[m+1UL];
      dc[2U] = b[m+2UL];
      dc[3U] = b[m+3UL];
      dc[4U] = b[m+4UL];
      dc[5U] = b[m+5UL];
      dc[6U] = 0;
      aprspos_GetSym(dc, 10ul, &sym1, &symt1);
      if (aprspos_posvalid(posn)) {
         *typ0 = 0;
         *pos = posn;
         *sym = sym1;
         *symt = symt1;
         if (*sym=='_') Wx(data, typ0, b, b_len, p);
         else if (vspeed<X2C_max_longcard) {
            *typ0 = 'S';
            *data = (float)vspeed*1.852f;
         }
      }
      else if (b[p]==':') {
         /*    ELSIF NOT (b[p] IN CHSET{":","?"}
                ) THEN typ:=tJUNK END;
                (* message bulletin query have no pos but is ok *)  */
         /* msg bulletin query or telemetry */
         m = 0UL;
         ++p; /* msg dest call */
         for (;;) {
            ch = b[my];
            if (ch=='>') ch = ' ';
            else ++my;
            if (ch!=b[p+m]) break;
            ++m;
            if (m>8UL) {
               *typ0 = 'J'; /* msg dest = inner mycall is telemetry */
               break;
            }
         }
      }
      else *typ0 = 'J';
   }
} /* end DirectPos() */


static uint32_t MHcount(pHEARD ph, uint32_t maxtime, uint32_t * cj)
{
   uint32_t i0;
   uint32_t ci;
   /* clean count array */
   struct HEARD * anonym;
   IncHeard(ph, 0, 0, maxtime);
   { /* with */
      struct HEARD * anonym = ph;
      ci = 0UL;
      *cj = 0UL;
      for (i0 = 0UL; i0<=48UL; i0++) {
         ci += (uint32_t)anonym->cnt[i0].pack;
         *cj += (uint32_t)anonym->cnt[i0].junk;
      } /* end for */
   }
   return ci;
} /* end MHcount() */


static void getval(const char s[], uint32_t s_len, uint32_t * i0,
                int32_t * v)
{
   char m;
   ++*i0;
   *v = 0L;
   m = s[*i0]=='-';
   if (m) ++*i0;
   while ((*i0<s_len-1 && (uint8_t)s[*i0]>='0') && (uint8_t)s[*i0]<='9') {
      *v =  *v*10L+(int32_t)((uint32_t)(uint8_t)s[*i0]-48UL);
      ++*i0;
   }
   while (*i0<s_len-1 && (uint8_t)s[*i0]>' ') ++*i0;
   while (*i0<s_len-1 && s[*i0]==' ') ++*i0;
   if (m) *v = -*v;
} /* end getval() */


static void AddHeard(pHEARD * table, uint32_t maxtime, const MONCALL from,
                uint32_t fromport, const char buf[],
                uint32_t buf_len, char * ungat,
                char setungat)
{
   uint32_t ji;
   uint32_t si;
   uint32_t j;
   uint32_t i0;
   int32_t res0;
   pHEARD po;
   pHEARD ph;
   pRAWTEXT prh;
   pRAWTEXT pr;
   struct HEARD * anonym;
   po = 0;
   ph = *table;
   if (mhperport) {
      while (ph && (ph->fromrx!=fromport || !aprsstr_StrCmp(ph->call, 10ul,
                from, 10ul))) {
         po = ph;
         ph = ph->next;
      }
   }
   else {
      while (ph && !aprsstr_StrCmp(ph->call, 10ul, from, 10ul)) {
         po = ph;
         ph = ph->next;
      }
   }
   if (ph) {
      /* entry found */
      if (po) po->next = ph->next;
   }
   else {
      /* new entry */
      po = 0;
      ph = *table;
      if (ph) {
         i0 = 0UL;
         while (ph->next) {
            ++i0;
            po = ph;
            ph = ph->next;
         }
         if (i0>=500UL || ph->mhtime+maxtime<systime) {
            /* old or table full */
            if (po) po->next = ph->next;
         }
         else ph = 0;
      }
      if (ph==0) {
         osic_alloc((char * *) &ph, sizeof(struct HEARD));
         if (ph==0 && po) {
            ph = po->next;
            po->next = 0;
         }
      }
      else {
         /* free rawtext lines */
         for (i0 = 0UL; i0<=1UL; i0++) {
            prh = ph->rawtext[i0];
            while (prh) {
               pr = prh->next;
               osic_free((char * *) &prh, prh->len);
               prh = pr;
            }
         } /* end for */
      }
      if (ph) {
         memset((char *)ph,(char)0,sizeof(struct HEARD));
         memcpy(ph->call,from,10u);
      }
   }
   if (ph) {
      if (ph!=*table) {
         ph->next = *table;
         *table = ph;
      }
      { /* with */
         struct HEARD * anonym = ph;
         anonym->mhtime = systime;
         if (setungat) {
            anonym->ungate = *ungat;
            if (!*ungat && MHcount(ph, maxtime, &j)==0UL) {
               ph->mhtime = 0UL; /* delete 0 count entry */
            }
         }
         else {
            /* axudp2 */
            anonym->txd = 0U;
            anonym->level = 0;
            anonym->quali = 0U;
            if (udp2[0U] && udp2[1U]) {
               i0 = 2UL;
               while (i0<99UL && udp2[i0]) {
                  switch ((unsigned)udp2[i0]) {
                  case 'T':
                     getval(udp2, 100ul, &i0, &res0);
                     anonym->txd = (uint16_t)res0;
                     break;
                  case 'V':
                     getval(udp2, 100ul, &i0, &res0);
                     anonym->level = (signed char)res0;
                     break;
                  case 'Q':
                     getval(udp2, 100ul, &i0, &res0);
                     anonym->quali = (uint8_t)res0;
                     break;
                  default:;
                     getval(udp2, 100ul, &i0, &res0);
                     break;
                  } /* end switch */
               }
            }
            /* axudp2 */
            *ungat = ph->ungate;
            DirectPos(buf, buf_len, &anonym->position, &anonym->sym,
                &anonym->symt, &anonym->datatyp, &anonym->data);
            IncHeard(ph, 1, anonym->datatyp=='J', maxtime);
            i0 = 0UL;
            while ((i0<buf_len-1 && buf[i0]!='>') && buf[i0]) {
               ++i0;
            }
            ++i0;
            j = 0UL;
            while ((((j<40UL && i0<buf_len-1) && buf[i0]!=':') && buf[i0])
                && !(buf[i0]==',' && buf[i0+1UL]=='q')) {
               anonym->head[j] = buf[i0];
               ++j;
               ++i0;
            }
            anonym->head[j] = 0;
            anonym->fromrx = fromport;
            if (rawlines>0UL) {
               /* store raw frames in mh */
               j = aprsstr_Length(buf, buf_len);
               if (j>=1023UL) j = 1022UL;
               si = (sizeof(struct RAWTEXT)+j)-1023UL; /* unit size */
               osic_alloc((char * *) &pr, si);
               if (pr) {
                  memset((char *)pr,(char)0,si);
                  pr->htime = osic_time();
                  pr->txd = (uint32_t)anonym->txd;
                  pr->len = si;
                  i0 = 0UL;
                  while (i0<j) {
                     pr->text[i0] = buf[i0];
                     ++i0;
                  }
                  pr->text[i0] = 0;
                  ji = (uint32_t)(anonym->datatyp=='J');
                /* use text/junk table */
                  prh = anonym->rawtext[ji];
                  i0 = 1UL;
                  if (prh) {
                     /* not empty so find last line */
                     while (prh->next) {
                        prh = prh->next; /* and count lines */
                        ++i0;
                     }
                     prh->next = pr;
                     if (i0>=rawlines) {
                        /* lines limit */
                        prh = anonym->rawtext[ji];
                        anonym->rawtext[ji] = prh->next;
                        osic_free((char * *) &prh, prh->len);
                     }
                  }
                  else anonym->rawtext[ji] = pr;
               }
            }
         }
      }
   }
   else *ungat = 0;
} /* end AddHeard() */


static void delmsg(pMESSAGE md)
{
   pMESSAGE mp;
   if (md==messages) messages = md->next;
   else {
      mp = messages;
      for (;;) {
         if (mp==0) return;
         if (mp->next==md) {
            mp->next = md->next;
            break;
         }
         mp = mp->next;
      }
   }
   if (verb) {
      osi_WrStrLn("---delete:", 11ul);
      Showmsg(md);
   }
   osic_free((char * *) &md, sizeof(struct MESSAGE));
} /* end delmsg() */


static char ismsgactiv(pMESSAGE m)
/* time and retry limit */
{
   return m->gentime+msgsendtime>systime && m->retryc<msgretries;
} /* end ismsgactiv() */


static uint32_t exp0(uint32_t n)
{
   if (n>0UL) --n;
   if (n>9UL) n = 9UL;
   return (uint32_t)(1UL<<n);
} /* end exp() */


static void RfTimer(void)
{
   pMESSAGE mp;
   pMESSAGE mo;
   char try0;
   uint32_t ht;
   uint32_t oldest;
   uint32_t totx;
   uint32_t maxi;
   pUDPSOCK us;
   struct MESSAGE * anonym;
   maxi = 0UL;
   for (;;) {
      oldest = X2C_max_longcard;
      mp = 0;
      mo = messages;
      while (mo) {
         if (mo->chkt!=systime && mo->txtime<oldest) {
            mp = mo;
            oldest = mp->txtime;
         }
         mo = mo->next;
      }
      if (mp==0) break;
      { /* with */
         struct MESSAGE * anonym = mp;
         anonym->chkt = systime;
         try0 = ismsgactiv(mp) && anonym->txtime+rfquiet<systime;
         totx = 0UL;
         if (try0) {
            if (anonym->acked) {
               try0 = (anonym->acksrc && anonym->retryc==0UL)
                && FindHeard(hearddir, anonym->from, &totx)>0UL;
            }
            else {
               ht = FindHeard(hearddir, anonym->to, &totx);
                /* msg only to heard user */
               us = porttosock(totx); /* send to which udp modem */
               try0 = 0;
               if (ht>0UL && us->lasttxtime+rfquiet<systime) {
                  /* suer ever heard and limit frames on this port */
                  if (anonym->ack[0U]==0) {
                     try0 = anonym->retryc<1UL; /* msg with no ack */
                  }
                  else if (anonym->txtime+ht<systime) {
                     /* msg with ack */
                     try0 = 1; /* heard after last sent */
                  }
                  else {
                     try0 = anonym->txtime+rfquiet*exp0(anonym->retryc)
                <systime; /* else double time on every tx */
                  }
                  if (try0) {
                     if (anonym->ack[0U]) {
                        /* with no ack send out of order */
                        mo = messages;
                        for (;;) {
                           if (mo==mp) break;
                           if (((aprsstr_StrCmp(mo->to, 10ul, anonym->to,
                10ul) && aprsstr_StrCmp(mo->from, 10ul, anonym->from,
                10ul)) && ismsgactiv(mo))
                && (mo->ack[0U] && !mo->acked || mo->ack[0U]
                ==0 && mo->retryc<1UL)) {
                              /* another older active msg to same user */
                              try0 = 0;
                              break;
                           }
                           mo = mo->next;
                        }
                     }
                  }
               }
            }
         }
         if (try0) {
            if (SendMsg(mp, totx)) ++maxi;
         }
         /*
                   mo:=messages;
                   WHILE mo<>NIL DO
                     IF StrCmp(mo^.to,
                to) & NOT acked THEN mo^.chkt:=systime END;
                (* 1 msg to 1 call *)
                     mo:=mo^.next;
                   END;
         */
         /*
                 IF (ack[0]=0C) & (retryc>=UNACKRET)
                OR acked & (msg2rf+msg2net+ack2rf+ack2net=0) 
                 THEN ht:=purgeunack ELSE ht:=purgemsg END;
                (* fast if unack & retried out or acked & never sent *)
         */
         if (anonym->ack[0U]==0) {
            if (anonym->retryc>=1UL) {
               ht = purgeunacksent;
            }
            else ht = purgeunack;
         }
         else if (anonym->acked) ht = purgeacked;
         else ht = purgemsg;
         if (anonym->gentime+ht<systime) delmsg(mp);
      }
      if (maxi>=maxatonce) break;
   }
} /* end RfTimer() */


static void SendNet(void)
{
   pMESSAGE mp;
   char done;
   while (netmhout!=netmhin) {
      mp = messages;
      while (mp) {
         if (mp->retryc==0UL) {
            if (mp->acked) {
               if (mp->acksrc==udpgate4_DIR && aprsstr_StrCmp(mp->from, 10ul,
                 netmh[netmhout], 10ul)) {
                  done = SendMsg(mp, 0UL);
                  return;
               }
            }
            else if ((mp->src==udpgate4_DIR || aprsstr_StrCmp(servercall,
                10ul, mp->from, 10ul)) && aprsstr_StrCmp(mp->to, 10ul,
                netmh[netmhout], 10ul)) {
               /* send only 1 a time */
               done = SendMsg(mp, 0UL);
               if (mp->ack[0U]) {
                  /* no more to this user if unack */
                  netmhout = (netmhout+1UL)%100UL;
               }
               return;
            }
         }
         /* send only 1 a time */
         mp = mp->next;
      }
      netmhout = (netmhout+1UL)%100UL;
   }
} /* end SendNet() */


static void Stoack(pMESSAGE mp, char rej, uint8_t path)
{
   struct MESSAGE * anonym;
   { /* with */
      struct MESSAGE * anonym = mp;
      if (!anonym->acked) {
         /* ack first time */
         anonym->acked = 1;
         anonym->reject = rej;
         anonym->acksrc = path;
         anonym->retryc = 0UL;
         anonym->txtime = 0UL;
      }
      else if (path>anonym->acksrc) anonym->acksrc = path;
   }
} /* end Stoack() */


static void Ackback(pMESSAGE tomp)
/* mark all older messages before an now acked */
{
   pMESSAGE mp;
   struct MESSAGE * anonym;
   mp = messages;
   while (mp!=tomp) {
      { /* with */
         struct MESSAGE * anonym = mp;
         if ((aprsstr_StrCmp(tomp->from, 10ul, anonym->from,
                10ul) && aprsstr_StrCmp(tomp->to, 10ul, anonym->to,
                10ul)) && anonym->gentime+dupetime<systime) {
            if (!anonym->acked) {
               anonym->acked = 1;
               anonym->acksrc = udpgate4_OBSOLET;
            }
            anonym->ackackt = 1;
         }
         mp = anonym->next;
      }
   }
} /* end Ackback() */


static char cmpmsg(MSGTEXT a, MSGTEXT b)
/* compare visable characters */
{
   uint32_t j;
   uint32_t i0;
   MSGTEXT tmp;
   MSGTEXT tmp0;
   a = (char *)memcpy(tmp,a,68u);
   b = (char *)memcpy(tmp0,b,68u);
   i0 = 0UL;
   j = 0UL;
   while (i0<=67UL && a[i0]) {
      if ((uint8_t)a[i0]>' ') {
         a[j] = a[i0];
         ++j;
      }
      ++i0;
   }
   if (j<=67UL) a[j] = 0;
   i0 = 0UL;
   j = 0UL;
   while (i0<=67UL && b[i0]) {
      if ((uint8_t)b[i0]>' ') {
         b[j] = b[i0];
         ++j;
      }
      ++i0;
   }
   if (j<=67UL) b[j] = 0;
   return aprsstr_StrCmp(a, 68ul, b, 68ul);
} /* end cmpmsg() */

#define udpgate4_REPACK "}"


static void Stomsg(const MONCALL fromcall, const MONCALL tocall, MSGTEXT msg,
                 ACKTEXT ackt, char isak, char rej,
                char query, uint8_t path)
{
   pMESSAGE mi;
   pMESSAGE mp;
   uint32_t cnt;
   REPLYACK rack;
   struct MESSAGE * anonym;
   struct MESSAGE * anonym0;
   MSGTEXT tmp;
   ACKTEXT tmp0;
   msg = (char *)memcpy(tmp,msg,68u);
   ackt = (char *)memcpy(tmp0,ackt,5u);
   if (verb && isak) osi_WrStrLn("---isak", 8ul);
   if (aprsstr_StrCmp(fromcall, 10ul, tocall, 10ul)) return;
   /* msg to him self ... */
   msg[67U] = 0;
   rack[0U] = 0;
   if (aprsstr_Length(ackt, 5ul)>=3UL && ackt[2U]=='}') {
      /* replyack xx}xx */
      rack[0U] = ackt[3U]; /* split ack text xx}yy to xx} yy */
      rack[1U] = ackt[4U];
      ackt[3U] = 0;
   }
   mp = 0;
   cnt = 0UL;
   mi = messages;
   while (mi) {
      { /* with */
         struct MESSAGE * anonym = mi;
         if (((aprsstr_StrCmp(ackt, 5ul, anonym->ack,
                5ul) && aprsstr_StrCmp(fromcall, 10ul, anonym->from,
                10ul)) && aprsstr_StrCmp(tocall, 10ul, anonym->to,
                10ul)) && ((isak || anonym->text[0U]==0) || cmpmsg(msg,
                anonym->text))) {
            mp = mi;
            memcpy(anonym->replyack,rack,2u);
                /* may change in retransmissions */
            if (isak) {
               Stoack(mi, rej, path);
               Ackback(mi);
            }
            else {
               if (anonym->acked && path==udpgate4_DIR) {
                  /* is acked resend ack */
                  if (anonym->retryc>0UL) --anonym->retryc;
                  anonym->txtime = 0UL;
               }
               if (anonym->text[0U]==0) {
                  /* ack came before message */
                  memcpy(anonym->text,msg,68u);
                  if (anonym->acked) Ackback(mi);
               }
               if (path>anonym->src) anonym->src = path;
            }
         }
         if ((((rack[0U] && rack[0U]==anonym->ack[0U])
                && (rack[1U]==anonym->ack[1U] || rack[1U]
                ==0 && anonym->ack[1U]=='}')) && aprsstr_StrCmp(fromcall,
                10ul, anonym->to, 10ul)) && aprsstr_StrCmp(tocall, 10ul,
                anonym->from, 10ul)) Stoack(mi, rej, path);
         mi = anonym->next;
      }
      ++cnt;
   }
   if (verb && mp) {
      osi_WrStr("msg found ", 11ul);
      Showmsg(mp);
   }
   if (mp==0) {
      /* new msg */
      if (cnt<maxmsg) {
         osic_alloc((char * *) &mp, sizeof(struct MESSAGE));
      }
      if (mp==0 && messages) {
         mp = messages; /* recycle oldest */
         messages = messages->next;
      }
      if (mp) {
         /* append to end of list */
         memset((char *)mp,(char)0,sizeof(struct MESSAGE));
         mp->next = 0;
         if (messages==0) messages = mp;
         else {
            mi = messages;
            while (mi->next) mi = mi->next;
            mi->next = mp;
         }
         { /* with */
            struct MESSAGE * anonym0 = mp;
            memcpy(anonym0->from,fromcall,10u);
            memcpy(anonym0->to,tocall,10u);
            memcpy(anonym0->text,msg,68u);
            memcpy(anonym0->ack,ackt,5u);
            memcpy(anonym0->replyack,rack,2u);
            anonym0->gentime = systime;
            if (path==udpgate4_DIR) anonym0->txtime = systime;
            else {
               anonym0->txtime = 0UL;
            }
            anonym0->acked = isak;
            anonym0->reject = rej;
            anonym0->src = path;
            anonym0->acksrc = path;
            anonym0->queryrep = query;
         }
      }
      if (verb) {
         if (path==udpgate4_DIR) osi_WrStr("new direct", 11ul);
         else if (path==udpgate4_INDIR) osi_WrStr("new rf", 7ul);
         else osi_WrStr("new net", 8ul);
         osi_WrStr(" msg#", 6ul);
         osic_WrINT32(cnt, 1UL);
         osi_WrStr(" ", 2ul);
         Showmsg(mp);
      }
   }
/*
  IF mp<>NIL THEN Ackback(mp) END;
                (* all before is thru and ack is at home so mark as done *)
*/
} /* end Stomsg() */


static void postoloc(char loc[], uint32_t loc_len,
                struct aprsstr_POSITION pos)
{
   uint32_t bc;
   uint32_t lc;
   float br;
   float lr;
   lr = (pos.long0*5.7295779513082E+1f+180.0f)*2880.0f;
   if (lr<=0.0f) lr = 0.0f;
   lc = (uint32_t)X2C_TRUNCC(lr,0UL,X2C_max_longcard);
   br = (pos.lat*5.7295779513082E+1f+90.0f)*5760.0f;
   if (br<=0.0f) br = 0.0f;
   bc = (uint32_t)X2C_TRUNCC(br,0UL,X2C_max_longcard);
   loc[0UL] = (char)(65UL+lc/57600UL);
   loc[1UL] = (char)(65UL+bc/57600UL);
   loc[2UL] = (char)(48UL+(lc/5760UL)%10UL);
   loc[3UL] = (char)(48UL+(bc/5760UL)%10UL);
   loc[4UL] = (char)(65UL+(lc/240UL)%24UL);
   loc[5UL] = (char)(65UL+(bc/240UL)%24UL);
   loc[6UL] = (char)(48UL+(lc/24UL)%10UL);
   loc[7UL] = (char)(48UL+(bc/24UL)%10UL);
   loc[8UL] = (char)(65UL+lc%24UL);
   loc[9UL] = (char)(65UL+bc%24UL);
   loc[10UL] = 0;
} /* end postoloc() */


static void degtostr(char s[], uint32_t s_len, float d,
                char posc, char negc)
{
   uint32_t n;
   char c;
   char tmp;
   d = X2C_DIVR(d,1.7453292519444E-2f);
   if (d<0.0f) {
      d = -d;
      c = negc;
   }
   else c = posc;
   n = (uint32_t)X2C_TRUNCC(d,0UL,X2C_max_longcard);
   aprsstr_IntToStr((int32_t)n, 1UL, s, s_len);
   aprsstr_Append(s, s_len, ".", 2ul);
   n = (uint32_t)X2C_TRUNCC((d-(float)n)*3600.0f,0UL,X2C_max_longcard);
   aprsstr_Append(s, s_len, (char *)(tmp = (char)(48UL+n/600UL),
                &tmp), 1u/1u);
   aprsstr_Append(s, s_len, (char *)(tmp = (char)(48UL+(n%600UL)/60UL),
                &tmp), 1u/1u);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_Append(s, s_len, (char *)(tmp = (char)(48UL+(n%60UL)/10UL),
                &tmp), 1u/1u);
   aprsstr_Append(s, s_len, (char *)(tmp = (char)(48UL+n%10UL),&tmp),
                 1u/1u);
   aprsstr_Append(s, s_len, (char *) &c, 1u/1u);
} /* end degtostr() */

#define udpgate4_PARPOS 6


static void Ungate(const MONCALL mycall, const char msg[],
                uint32_t msg_len, uint8_t path)
{
   uint32_t p;
   uint32_t n;
   uint32_t i0;
   MONCALL hn;
   MONCALL h;
   MSGTEXT answ;
   char un;
   pHEARD ph;
   char tmp;
   ACKTEXT tmp0;
   /* ?APRS#, ?APRS# -9, ?APRS# 9, ?APRS# CALL*/
   memcpy(h,mycall,10u);
   i0 = 0UL;
   for (;;) {
      if (i0>9UL) break;
      if (h[i0]=='-') {
         h[i0] = 0;
         break;
      }
      ++i0;
   }
   memcpy(hn,h,10u);
   i0 = 6UL;
   skipblank(msg, msg_len, &i0);
   p = i0;
   un = msg[i0]=='-';
   if (un) ++i0;
   if (GetSec(msg, msg_len, &i0, &n)==0L && n<=15UL) {
      if (n>0UL) {
         aprsstr_Append(h, 10ul, "-", 2ul);
         if (n>=10UL) aprsstr_Append(h, 10ul, "1", 2ul);
         aprsstr_Append(h, 10ul, (char *)(tmp = (char)(n%10UL+48UL),
                &tmp), 1u/1u);
      }
      AddHeard(&hearddir, heardtimew, h, 0UL, "", 1ul, &un, 1);
   }
   else {
      n = aprsstr_Length(msg, msg_len);
      i0 = 0UL;
      while ((i0<=9UL && p<n) && (uint8_t)msg[p]>' ') {
         hn[i0] = msg[p];
         ++i0;
         ++p;
      }
      if (i0>0UL && i0<=9UL) hn[i0] = 0;
   }
   strncpy(answ,"Not Gated:",68u);
   un = 1;
   ph = hearddir;
   for (;;) {
      if (ph==0) break;
      if (ph->ungate && aprsstr_InStr(ph->call, 10ul, hn, 10ul)==0L) {
         if (aprsstr_Length(answ, 68ul)+aprsstr_Length(ph->call,
                10ul)>=64UL) {
            aprsstr_Append(answ, 68ul, "...", 4ul);
            break;
         }
         aprsstr_Append(answ, 68ul, " ", 2ul);
         aprsstr_Append(answ, 68ul, ph->call, 10ul);
         un = 0;
      }
      ph = ph->next;
   }
   if (un) {
      aprsstr_Append(answ, 68ul, " None", 6ul);
      if (n<=6UL) {
         aprsstr_Append(answ, 68ul, " (igate your ssid off/on: ?APRS# -5 / 5)\
", 41ul);
      }
   }
   Stomsg(servercall, mycall, answ, *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0, 1,
                 path);
} /* end Ungate() */


static void Query(MONCALL fromcall, char msg[], uint32_t msg_len,
                const char buf[], uint32_t buf_len, uint8_t path)
{
   pMESSAGE mp;
   char no;
   MSGTEXT s;
   MSGTEXT mh;
   MONCALL callh;
   uint32_t len;
   uint32_t i0;
   pHEARD hn;
   pHEARD hp;
   char cmd;
   float maxdist;
   float dist;
   MONCALL tmp;
   ACKTEXT tmp0;
   MSGTEXT tmp1;
   fromcall = (char *)memcpy(tmp,fromcall,10u);
   X2C_PCOPY((void **)&msg,msg_len);
   cmd = 0;
   len = aprsstr_Length(msg, msg_len);
   i0 = 1UL;
   while (i0<len) {
      msg[i0] = X2C_CAP(msg[i0]);
      ++i0;
   }
   if ((((msg[1UL]=='A' && msg[2UL]=='P') && msg[3UL]=='R') && msg[4UL]=='S')
                 && (msg[6UL]==0 || msg[6UL]==' ')) cmd = msg[5UL];
   else if ((msg[1UL]=='M' && msg[2UL]=='H')
                && (msg[3UL]==0 || msg[3UL]=='?')) cmd = 'D';
   else if (((((msg[1UL]=='P' && msg[2UL]=='I') && msg[3UL]=='N')
                && msg[4UL]=='G') && msg[5UL]=='?') && msg[6UL]==0) {
      cmd = 'T';
   }
   else if ((((msg[1UL]=='H' && msg[2UL]=='E') && msg[3UL]=='L')
                && msg[4UL]=='P') && msg[5UL]==0) cmd = 'h';
   else if (((msg[1UL]=='V' && msg[2UL]=='E') && msg[3UL]=='R')
                && (msg[4UL]==0 || msg[4UL]=='?')) cmd = 'v';
   else if ((msg[1UL]=='D' && msg[2UL]=='X') && msg[3UL]==0) cmd = 'x';
   if (cmd=='M') {
      /* activate messages for user */
      no = verb;
      mp = messages;
      while (mp) {
         if (!mp->acked && aprsstr_StrCmp(fromcall, 10ul, mp->to, 10ul)) {
            mp->retryc = 0UL;
            if (verb) {
               no = 0;
               osi_WrStr("query msg activated:", 21ul);
               Showmsg(mp);
            }
         }
         mp = mp->next;
      }
      if (no) osi_WrStrLn("query msg not found", 20ul);
   }
   else if (cmd=='T') {
      /* return message path */
      i0 = 0UL;
      strncpy(mh,"Path - ",68u);
      while ((i0+7UL<67UL && buf[i0]!=':') && buf[i0]) {
         mh[i0+7UL] = buf[i0];
         ++i0;
      }
      mh[i0+7UL] = 0;
      Stomsg(servercall, fromcall, mh, *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0,
                 1, path);
   }
   else if (cmd=='D') {
      /* return mh table */
      strncpy(mh,"Directs=",68u);
      hn = hearddir;
      while ((hn && hn->mhtime+heardtimew>systime) && aprsstr_Length(mh,
                68ul)+aprsstr_Length(hn->call, 10ul)+1UL<67UL) {
         if (!hn->ungate) {
            aprsstr_Append(mh, 68ul, " ", 2ul);
            aprsstr_Append(mh, 68ul, hn->call, 10ul);
         }
         hn = hn->next;
      }
      Stomsg(servercall, fromcall, mh, *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0,
                 1, path);
   }
   else if (cmd=='x' || cmd=='H') {
      if (cmd=='H') {
         for (i0 = 0UL; i0<=9UL; i0++) {
            callh[i0] = X2C_CAP(msg[i0+7UL]);
         } /* end for */
      }
      maxdist = 0.0f;
      hn = 0;
      hp = hearddir;
      while (hp) {
         if (((hp->mhtime+heardtimew>systime && !hp->ungate)
                && aprspos_posvalid(hp->position)) && aprspos_posvalid(home))
                 {
            dist = aprspos_distance(home, hp->position);
            if (cmd=='H') {
               if (aprsstr_StrCmp(callh, 10ul, hp->call, 10ul)) {
                  hn = hp;
                  maxdist = dist;
               }
            }
            else if (dist>maxdist) {
               maxdist = dist;
               hn = hp;
            }
         }
         hp = hp->next;
      }
      if (hn) {
         strncpy(mh,"DX: ",68u);
         aprsstr_Append(mh, 68ul, hn->call, 10ul);
         aprsstr_Append(mh, 68ul, " ", 2ul);
         degtostr(s, 68ul, hn->position.lat, 'N', 'S');
         aprsstr_Append(mh, 68ul, s, 68ul);
         aprsstr_Append(mh, 68ul, " ", 2ul);
         degtostr(s, 68ul, hn->position.long0, 'E', 'W');
         aprsstr_Append(mh, 68ul, s, 68ul);
         aprsstr_Append(mh, 68ul, " ", 2ul);
         aprsstr_IntToStr((int32_t)(uint32_t)X2C_TRUNCC(maxdist+0.5f,0UL,
                X2C_max_longcard), 1UL, s, 68ul);
         aprsstr_Append(mh, 68ul, s, 68ul);
         aprsstr_Append(mh, 68ul, "km ", 4ul);
         aprsstr_TimeToStr(hn->mhtime%86400UL, s, 68ul);
         aprsstr_Append(mh, 68ul, s, 68ul);
      }
      else strncpy(mh,"No position available",68u);
      Stomsg(servercall, fromcall, mh, *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0,
                 1, path);
   }
   else if (cmd=='S') {
      Stomsg(servercall, fromcall, *(MSGTEXT *)memcpy(&tmp1,"udpgate 0.68 Msg\
 S&F Relay",27u), *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0, 1, path);
   }
   else if (cmd=='v') {
      Stomsg(servercall, fromcall, *(MSGTEXT *)memcpy(&tmp1,"udpgate 0.68",
                13u), *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0, 1, path);
   }
   else if (cmd=='h') {
      Stomsg(servercall, fromcall, *(MSGTEXT *)memcpy(&tmp1,"Cmd: ?APRS[D/H/M\
/P/S/T/#] ?DX ?MH ?VER? ?PING?",47u), *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0,
                1, path);
   }
   else if (cmd=='P') {
      Netbeacon(mh, 68ul, 0, 0);
      Stomsg(servercall, fromcall, mh, *(ACKTEXT *)memcpy(&tmp0,"",1u), 0, 0,
                 1, path);
   }
   else if (cmd=='#') Ungate(fromcall, msg, msg_len, path);
   X2C_PFREE(msg);
} /* end Query() */


static void gatetoviaheard(const char b[], uint32_t b_len,
                const MONCALL fromcall, const MONCALL tocall)
{
   pUDPSOCK u;
   uint32_t void0;
   uint32_t po;
   if (!aprsstr_StrCmp(fromcall, 10ul, tocall, 10ul) && FindHeard(heardvia,
                tocall, &po)>0UL) {
      u = porttosock(po);
      if ((u && u->torfradius>0.0f) && FindHeard(hearddir, tocall,
                &void0)==0UL) NetToRf(b, b_len, po);
   }
} /* end gatetoviaheard() */


static void getack(const char s[], uint32_t s_len, uint32_t p,
                ACKTEXT a)
{
   uint32_t i0;
   i0 = 0UL;
   while ((p<=s_len-1 && (uint8_t)s[p]>' ') && (uint8_t)s[p]<(uint8_t)
                '\200') {
      a[i0] = s[p];
      ++p;
      ++i0;
      if (i0>4UL) return;
   }
   a[i0] = 0;
} /* end getack() */

#define udpgate4_HEADEREND ":"

#define udpgate4_FROMEND ">"

#define udpgate4_NOARCHIVE "!x!"


static void Getmsg(const char b[], uint32_t b_len, uint32_t rxport,
                char goodpath, char * ungate)
/* get user msg and ack out of aprsis stream */
{
   uint32_t po;
   uint32_t void0;
   uint32_t p;
   uint32_t len;
   uint32_t pf;
   uint32_t pm;
   MSGTEXT msg;
   ACKTEXT ack;
   MONCALL hfrom;
   MONCALL tocall;
   MONCALL fromcall;
   char c;
   char badcall;
   char tomh;
   char dir;
   char trust;
   char isrej;
   char isack;
   uint8_t path;
   *ungate = 0;
   if ((b[0UL]==0 || rxport && !iscall(b, b_len, 0UL)) || aprsstr_InStr(b,
                b_len, "!x!", 4ul)>=0L) return;
   dir = Heard(b, b_len, hfrom, &trust);
   if (hfrom[0U]) {
      if (rxport) {
         if ((heardtimew>48UL && dir) && trust) {
            AddHeard(&hearddir, heardtimew, hfrom, rxport, b, b_len, ungate,
                0);
         }
         else if (heardtimevia>48UL) {
            AddHeard(&heardvia, heardtimevia, hfrom, rxport, b, b_len,
                ungate, 0);
         }
      }
   }
   if (maxmsg==0UL && viacall[0U]==0) return;
   /* mh server only */
   len = aprsstr_Length(b, b_len);
   pm = 0UL;
   do {
      pf = pm;
      for (;;) {
         if (pm>=len) return;
         if (b[pm]==':') break;
         ++pm;
      }
      pm += 2UL;
   } while (!(pm>len || b[pm-1UL]!='}'));
   /* skip thirdparty headers */
   --pm;
   tomh = sendnetmsg;
   if (((pm+10UL<len && b[pm]==':') && b[pm+10UL]==':') && iscall(b, b_len,
                pm+1UL)) {
      /* user message */
      /* to */
      cpcall(tocall, 10ul, b, b_len, pm+1UL);
      /* from */
      cpcall(fromcall, 10ul, b, b_len, pf);
      if (aprsstr_StrCmp(fromcall, 10ul, tocall, 10ul)) return;
      /* msg to him self telemetry ... */
      badcall = 1;
      if (iscall(b, b_len, pf) && maxmsg) {
         /* mail storage on & not from a special (no)call */
         badcall = 0;
         /* text */
         p = 0UL;
         pm += 11UL;
         for (;;) {
            if (pm>=len) break;
            c = b[pm];
            if ((p>=67UL || (uint8_t)c<' ') || c=='{') break;
            msg[p] = c;
            ++p;
            ++pm;
         }
         msg[p] = 0;
         /* ack */
         isack = 0;
         ack[0U] = 0;
         isrej = (msg[0U]=='r' && msg[1U]=='e') && msg[2U]=='j';
         if ((msg[0U]=='a' && msg[1U]=='c') && msg[2U]=='k' || isrej) {
            /* is ack or rej */
            getack(msg, 68ul, 3UL, ack);
            msg[0U] = 0;
            isack = 1;
         }
         else if (pm<len && b[pm]=='{') getack(b, b_len, pm+1UL, ack);
         path = udpgate4_NET;
         if (rxport) {
            path = udpgate4_INDIR;
            if ((dir && goodpath) && FindHeard(hearddir, fromcall,
                &void0)>0UL) path = udpgate4_DIR;
         }
         if (msg[0U]=='?') {
            if (aprsstr_StrCmp(tocall, 10ul, viacall,
                10ul) || aprsstr_StrCmp(tocall, 10ul, servercall, 10ul)) {
               Query(fromcall, msg, 68ul, b, b_len, path);
               tomh = 1;
            }
         }
         else if (isack) {
            Stomsg(tocall, fromcall, msg, ack, 1, isrej, 0, path);
         }
         else if (rxport || goodpath) {
            Stomsg(fromcall, tocall, msg, ack, 0, isrej, 0, path);
         }
         gatetoviaheard(b, b_len, fromcall, tocall);
      }
      if (((badcall || msgretries==0UL) && goodpath) && FindHeard(hearddir,
                tocall, &po)>0UL) {
         NetToRf(b, b_len, po); /* from not-call (WINLINK) or msg sending off to local user */
      }
      if (verb) {
         osi_WrStr(" <", 3ul);
         p = 0UL;
         while (p<b_len-1 && b[p]) {
            if ((uint8_t)b[p]>=' ') osi_WrStr((char *) &b[p], 1u/1u);
            ++p;
         }
         osi_WrStrLn(">", 2ul);
      }
   }
   if (((tomh && hfrom[0U]) && rxport==0UL) && (netmhin+1UL)%100UL!=netmhout)
                 {
      memcpy(netmh[netmhin],hfrom,10u);
      netmhin = (netmhin+1UL)%100UL;
   }
} /* end Getmsg() */


static void getportname(uint32_t n, char s[], uint32_t s_len)
/* if not found return port number */
{
   pUDPSOCK u;
   uint32_t i0;
   if (n==0UL) aprsstr_Assign(s, s_len, "TCPIP", 6ul);
   else {
      u = udpsocks;
      i0 = 1UL;
      while (u && i0<n) {
         ++i0;
         u = u->next;
      }
      if (u==0 || u->portname[0U]==0) {
         aprsstr_IntToStr((int32_t)n, 1UL, s, s_len);
      }
      else aprsstr_Assign(s, s_len, u->portname, 10ul);
   }
} /* end getportname() */

#define udpgate4_SEP "|"
/* field separator */

#define udpgate4_SEPESC " "
/* set instead of SEP */

#define udpgate4_TEMPFN "~"
/* append to filename for temp file */

#define udpgate4_LINELEN 99
/* fill up lines to equal size */


static void App(uint32_t * p, char h[256], char s[],
                uint32_t s_len)
{
   uint32_t i0;
   X2C_PCOPY((void **)&s,s_len);
   i0 = 0UL;
   while (i0<=s_len-1 && s[i0]) {
      if (s[i0]!='|') h[*p] = s[i0];
      else h[*p] = ' ';
      ++*p;
      ++i0;
   }
   X2C_PFREE(s);
} /* end App() */


static void MHtoFile(void)
{
   char h[256];
   char h1[16];
   FILENAME fn;
   pHEARD hn;
   uint32_t t;
   int32_t fd;
   uint32_t jc;
   uint32_t cnt;
   uint32_t p;
   aprsstr_Assign(fn, 1024ul, mhfilename, 1024ul);
   aprsstr_Append(fn, 1024ul, "~", 2ul);
   fd = osi_OpenWrite(fn, 1024ul);
   if (fd<0L) return;
   cnt = mhfilelines;
   hn = hearddir;
   while (cnt>0UL && hn) {
      p = 0UL;
      if (hn->mhtime+heardtimew>systime && !hn->ungate) {
         App(&p, h, hn->call, 10ul);
         h[p] = '|';
         ++p;
         App(&p, h, (char *) &hn->symt, 1u/1u);
         App(&p, h, (char *) &hn->sym, 1u/1u);
         h[p] = '|';
         ++p;
         /*      IntToStr(hn^.fromrx, 1, h1); App(h1); */
         getportname(hn->fromrx, h1, 16ul);
         App(&p, h, h1, 16ul);
         h[p] = '|';
         ++p;
         t = hn->mhtime;
         if (systime<t || t==0UL) t = 0UL;
         else t = systime-t;
         aprsstr_IntToStr((int32_t)t, 1UL, h1, 16ul);
         App(&p, h, h1, 16ul);
         h[p] = '|';
         ++p;
         aprsstr_IntToStr((int32_t)MHcount(hn, heardtimew, &jc), 1UL, h1,
                16ul);
         App(&p, h, h1, 16ul);
         h[p] = '|';
         ++p;
         if (aprspos_posvalid(hn->position) && aprspos_posvalid(home)) {
            aprsstr_FixToStr(aprspos_distance(home, hn->position)+0.05f, 2UL,
                 h1, 16ul);
            App(&p, h, h1, 16ul);
         }
         h[p] = '|';
         ++p;
         if (hn->datatyp=='C') {
            aprsstr_FixToStr(hn->data, 2UL, h1, 16ul);
            aprsstr_Append(h1, 16ul, "\260C", 3ul);
         }
         else if (hn->datatyp=='S') {
            aprsstr_FixToStr(hn->data, 0UL, h1, 16ul);
            aprsstr_Append(h1, 16ul, "kmh", 4ul);
         }
         else h1[0] = 0;
         App(&p, h, h1, 16ul);
         h[p] = '|';
         ++p;
         App(&p, h, hn->head, 41ul);
         h[p] = '\012';
         ++p;
         osi_WrBin(fd, (char *)h, 256u/1u, p);
      }
      hn = hn->next;
      --cnt;
   }
   osic_Close(fd);
   osi_Rename(fn, 1024ul, mhfilename, 1024ul);
} /* end MHtoFile() */

/*
PROCEDURE finddupe(s-:DUPESTRING):BOOLEAN;
VAR i, sum:CARDINAL;
    t:TIME;  
BEGIN
  i:=0;
  sum:=0;  
  WHILE (i<=HIGH(s)) & (s[i]<>0C) DO INC(sum, ORD(s[i]); INC(i) END;
                (* make simple hash *)

  sum:=sum MOD (HIGH(dupehashs)+1);
  t:=dupehashs[sum].time;
  IF systime>=t+dupetime THEN
    WITH dupestrings[dupewrite] DO
      str:=s;
      time:=systime;
      next:=dupewrite;
    END;
    dupehashs[sum].hash:=dupewrite;
    dupehashs[sum].time:=systime;
    dupewrite:=(dupewrite+1) MOD (HIGH(dupestrings)+1);
  ELSE
    DEC(i);
    LOOP
      IF dupestrings[i]=s THEN RETURN TRUE END;












END finddupe;

*/

static int32_t callchk(uint32_t * qpos, uint8_t * unset,
                char buf[], uint32_t buf_len, uint32_t * p,
                uint32_t * pssid, char withstar,
                char iscall0)
{
   uint32_t lit;
   uint32_t num;
   uint32_t j;
   uint32_t i0;
   char c;
   uint8_t u;
   *pssid = 0UL;
   for (u = udpgate4_gUNGATE;; u++) {
      j = 0UL;
      i0 = *p;
      while (ungates[u][j]==buf[i0]) {
         ++i0;
         ++j;
         if (ungates[u][j]=='*' || ungates[u][j]==0 && (buf[i0]
                ==',' || buf[i0]==':')) *unset |= (1U<<u);
      }
      if (u==udpgate4_gQ) break;
   } /* end for */
   if (iscall0) {
      /* we want a callsign */
      num = 0UL;
      lit = 0UL;
      c = buf[*p];
      if ((uint8_t)c>='A' && (uint8_t)c<='Z' || c=='^') ++lit;
      else if ((uint8_t)c<'0' || (uint8_t)c>'9') return -1L;
      ++*p;
      for (;;) {
         c = buf[*p];
         if ((uint8_t)c>='0' && (uint8_t)c<='9') ++num;
         else if ((uint8_t)c>='A' && (uint8_t)c<='Z' || c=='^') ++lit;
         else break;
         ++*p;
      }
      if ((lit<2UL || num==0UL) || num>2UL) return -1L;
      *pssid = *p;
      if (buf[*p]=='-') {
         ++*p;
         if (buf[*p]=='1') {
            ++*p;
            if ((uint8_t)buf[*p]>='0' && (uint8_t)buf[*p]<='5') ++*p;
         }
         else {
            if ((uint8_t)buf[*p]<'1' || (uint8_t)buf[*p]>'9') return -1L;
            ++*p;
         }
      }
   }
   else if ((buf[*p]=='q' && buf[*p+1UL]=='A') && (uint8_t)
                buf[*p+2UL]>='A') {
      /* qAx */
      *qpos = *p+2UL;
      *p += 3UL;
   }
   else {
      /* we skip any text */
      for (;;) {
         c = buf[*p];
         if (c=='-') *pssid = *p;
         if (((((uint8_t)c<=' ' || c=='>') || c=='*') || c==',') || c==':')
                 break;
         ++*p;
      }
   }
   if (*pssid==0UL) *pssid = *p;
   if (buf[*p]=='*' && withstar) ++*p;
   return 0L;
} /* end callchk() */


static void Iconstruct(char qtext[32], uint32_t * pins,
                char logcall[], uint32_t logcall_len, char buf[],
                uint32_t buf_len, uint32_t * p)
{
   uint32_t k;
   uint32_t j;
   uint32_t i0;
   if (*p<=4UL || buf[*p-2UL]!='I') return;
   /* ,CALL,I: */
   i0 = *p-4UL;
   while (buf[i0]!=',') {
      /* hop back call */
      if (i0==0UL) return;
      --i0;
   }
   k = i0; /* "," before call */
   ++i0;
   j = 0UL;
   while (j<=logcall_len-1 && logcall[j]) {
      if (logcall[j]!=buf[i0]) return;
      ++i0;
      ++j;
   }
   /* logcall = call,I */
   *pins = k;
   i0 = *p-1UL;
   *p = k+1UL; /* payload */
   for (;;) {
      buf[k] = buf[i0];
      if (i0>=buf_len-1 || buf[i0]==0) break;
      ++k;
      ++i0;
   }
   /* delete ,call,I */
   strncpy(qtext,",qAR,",32u); /* insert ,qAR,call */
   aprsstr_Append(qtext, 32ul, logcall, logcall_len);
/*
OE0AAA-9>T8SV40,WIDE1-1,DB0WGS,I:test
OE0AAA-9>T8SV40,WIDE1-1:test
OE0AAA-9>T8SV40,WIDE1-1,qAR,DB0WGS:test 
*/
} /* end Iconstruct() */


static int32_t AprsIs(char buf[], uint32_t buf_len,
                uint8_t datafilt, uint8_t msgfilt,
                const char logcall[], uint32_t logcall_len,
                uint32_t udpchan, char valid,
                struct POSCALL * poscall0)
{
   uint32_t qpos;
   uint32_t micedest;
   uint32_t payload;
   uint32_t ha;
   uint32_t ilen;
   uint32_t psum1;
   uint32_t psum0;
   uint32_t pssid;
   uint32_t pins;
   uint32_t len;
   uint32_t p;
   uint32_t i0;
   uint8_t hashh;
   uint8_t hashl;
   char qtext[32];
   char ungat;
   char postyp;
   uint8_t unset;
   FRAMEBUF com;
   uint32_t tmp;
   hashl = 0U;
   hashh = 0U;
   p = 0UL;
   pins = 0UL;
   pssid = 0UL; /* for savety */
   len = 0UL;
   poscall0->pos.lat = 0.0f;
   poscall0->pos.long0 = 0.0f;
   while ((len<buf_len-1 && buf[len]) && buf[len]!='\015') ++len;
   if (len==0UL || len+35UL>buf_len-1) return -2L;
   unset = 0U;
   qpos = 0UL;
   for (;;) {
      /* thirdparty loop */
      psum0 = p;
      if (callchk(&qpos, &unset, buf, buf_len, &p, &pssid, 0,
                udpchan || callsrc)) return -3L;
      /* src call */
      if (buf[p]!='>') return -3L;
      i0 = 0UL;
      while (i0<9UL && i0<p) {
         poscall0->call[i0] = buf[i0];
         ++i0;
      }
      poscall0->call[i0] = 0;
      ++p;
      micedest = p;
      if (callchk(&qpos, &unset, buf, buf_len, &p, &pssid, 0, 0)) return -3L;
      /* dest call */
      psum1 = pssid;
      while (buf[p]==',') {
         ++p;
         if (callchk(&qpos, &unset, buf, buf_len, &p, &pssid, 1, 0)) {
            return -3L;
         }
      }
      /* via calls */
      if (buf[p]!=':') return -3L;
      if (pins==0UL) pins = p;
      ++p;
      if (buf[p]!='}') break;
      ++p;
   }
   if (p>=len) return -2L;
   qtext[0U] = 0;
   if (qpos>0UL) {
      /* qA */
      if (X2C_CAP(buf[qpos])=='Z') return -5L;
      /* qAZ */
      if (X2C_CAP(buf[qpos])=='I') {
         aprsstr_Append(qtext, 32ul, ",", 2ul); /* qAI */
         aprsstr_Append(qtext, 32ul, servercall, 10ul);
      }
   }
   else if (udpchan) {
      aprsstr_Append(qtext, 32ul, ",qAU,", 6ul);
      aprsstr_Append(qtext, 32ul, servercall, 10ul);
   }
   else if (logcall[0UL]) {
      if (cmpfrom(buf, buf_len, 0UL, logcall, logcall_len)) {
         strncpy(qtext,",qAC,",32u);
      }
      else strncpy(qtext,",qAS,",32u);
      aprsstr_Append(qtext, 32ul, servercall, 10ul);
      Iconstruct(qtext, &pins, logcall, logcall_len, buf, buf_len, &p);
   }
   /*
   IO.WrStr(" <");FOR i:=0 TO Length(buf)-1 DO IO.WrHex(ORD(buf[i]),3) END;
   IO.WrStr("> "); IO.WrLn; IO.WrCard(pins, 10); IO.WrCard(p, 10); IO.WrLn;
   */
   payload = p;
   poscall0->typ0 = buf[p];
   if (buf[p]=='?') return -5L;
   if (udpchan || valid) {
      Getmsg(buf, buf_len, udpchan, (unset&msgfilt)==0U, &ungat);
      if (ungat) return -6L;
   }
   if ((unset&datafilt)!=0U) return -3L;
   tmp = psum1-1UL;
   i0 = psum0;
   if (i0<=tmp) for (;; i0++) {
      aprsstr_HashCh(buf[i0], &hashl, &hashh);
      if (i0==tmp) break;
   } /* end for */
   while ((p<len && buf[p]!='\015') && buf[p]!='\012') {
      aprsstr_HashCh(buf[p], &hashl, &hashh);
      ++p;
   }
   ha = (uint32_t)(uint8_t)(char)hashl+(uint32_t)(uint8_t)
                (char)hashh*256UL&65535UL;
   if (timehash[ha]+dupetime>systime) return -5L;
   /* a duplicate */
   if (udpchan || valid) timehash[ha] = systime;
   aprspos_GetPos(&poscall0->pos, &vspeed, &vcourse, &valt, &vsym, &vsymt,
                buf, buf_len, micedest, payload, com, 512ul, &postyp);
   if (udpchan==0UL && !valid) return -1L;
   buf[p] = '\015';
   ++p;
   buf[p] = '\012';
   ++p;
   buf[p] = 0;
   ++p;
   ilen = aprsstr_Length(qtext, 32ul);
   if (ilen>0UL) {
      while (p>pins) {
         --p;
         buf[p+ilen] = buf[p];
      }
      i0 = 0UL;
      while (i0<ilen) {
         buf[p] = qtext[i0];
         ++p;
         ++i0;
      }
   }
   return 0L;
} /* end AprsIs() */


static char getfix(uint32_t * p, char s[], uint32_t s_len,
                float * x, float mul)
{
   uint32_t i0;
   FRAMEBUF h;
   if (s[*p]=='/') {
      ++*p;
      i0 = 0UL;
      while (((uint8_t)s[*p]>' ' && s[*p]!='/') && i0<511UL) {
         h[i0] = s[*p];
         ++i0;
         ++*p;
      }
      h[i0] = 0;
      if (aprsstr_StrToFix(x, h, 512ul)) {
         /*
         IO.WrFixed(x, 4,10); IO.WrStr("   fix<"); IO.WrStr(h);
                IO.WrStrLn(">");
         */
         *x =  *x*mul;
         return 1;
      }
   }
   return 0;
} /* end getfix() */


static void getcalls(char s[], uint32_t s_len, uint32_t * p,
                MONCALL table[], uint32_t table_len)
{
   uint32_t j;
   uint32_t i0;
   ++*p;
   j = 0UL;
   while (s[*p]=='/') {
      ++*p;
      i0 = 0UL;
      while ((*p<=s_len-1 && (uint8_t)s[*p]>' ') && s[*p]!='/') {
         if (j<=table_len-1 && i0<=9UL) table[j][i0] = s[*p];
         ++*p;
         ++i0;
      }
      ++j;
   }
} /* end getcalls() */

typedef uint32_t CHSET0[4];

static CHSET0 _cnst0 = {0x00000000UL,0x00080000UL,0x00000000UL,0x00BBE200UL}
                ;

static void gettyps(char s[], uint32_t s_len, uint32_t * p,
                char t[], uint32_t t_len)
{
   uint32_t j;
   ++*p;
   j = 0UL;
   t[0UL] = 0;
   if (s[*p]=='/') {
      ++*p;
      while ((j<t_len-1 && *p<=s_len-1) && (uint8_t)s[*p]>' ') {
         if (X2C_INL((int32_t)(uint8_t)s[*p],128,
                _cnst0) && aprsstr_InStr(t, t_len, (char *) &s[*p],
                1u/1u)<0L) {
            t[j] = s[*p];
            t[j+1UL] = 0;
            ++j;
         }
         ++*p;
      }
   }
} /* end gettyps() */


static void GetFilters(struct FILTERS * filters, const char s[],
                uint32_t s_len, uint32_t p, pTCPSOCK pongto)
{
   char not;
   FRAMEBUF pongstr;
   skipblank(s, s_len, &p);
   if (cmpfrom(s, s_len, p, "filter", 7ul)) {
      p += 6UL;
      not = 0;
      filters->typ0 = 0; /* re-init if new filter comes */
      memset((char *)filters->viacalls,(char)0,80UL);
      memset((char *)filters->entrycalls,(char)0,80UL);
      memset((char *)filters->prefixes,(char)0,80UL);
      memset((char *)filters->bud,(char)0,80UL);
      memset((char *)filters->objects,(char)0,80UL);
      memset((char *)filters->typs,(char)0,13UL);
      memset((char *)filters->destcalls,(char)0,80UL);
      if (s[p]==' ') {
         skipblank(s, s_len, &p);
         while ((uint8_t)s[p]>=' ') {
            if (s[p]==' ') not = 0;
            skipblank(s, s_len, &p);
            if (s[p]=='m') {
               ++p;
               if (getfix(&p, s, s_len, &filters->radius, 1.0f)) {
                  filters->typ0 = 'm';
               }
               else return;
            }
            else if (s[p]=='r') {
               ++p;
               if (!getfix(&p, s, s_len, &filters->base.lat,
                1.7453292519444E-2f)) return;
               if (!getfix(&p, s, s_len, &filters->base.long0,
                1.7453292519444E-2f)) return;
               if (!getfix(&p, s, s_len, &filters->radius, 1.0f)) return;
               if (!aprspos_posvalid(filters->base)) return;
               filters->typ0 = 'r';
            }
            else if (s[p]=='a') {
               ++p;
               if (!getfix(&p, s, s_len, &filters->base.lat,
                1.7453292519444E-2f)) return;
               if (!getfix(&p, s, s_len, &filters->base.long0,
                1.7453292519444E-2f)) return;
               if (!getfix(&p, s, s_len, &filters->edge.lat,
                1.7453292519444E-2f)) return;
               if (!getfix(&p, s, s_len, &filters->edge.long0,
                1.7453292519444E-2f)) return;
               if (!aprspos_posvalid(filters->base)) return;
               if (!aprspos_posvalid(filters->edge)) return;
               filters->typ0 = 'a';
            }
            else if (s[p]=='d') {
               getcalls(s, s_len, &p, filters->viacalls, 8ul);
               filters->notvia = not;
            }
            else if (s[p]=='e') {
               getcalls(s, s_len, &p, filters->entrycalls, 8ul);
               filters->notentry = not;
            }
            else if (s[p]=='u') {
               getcalls(s, s_len, &p, filters->destcalls, 8ul);
               filters->notdestcall = not;
            }
            else if (s[p]=='p') {
               getcalls(s, s_len, &p, filters->prefixes, 8ul);
               filters->notprefix = not;
            }
            else if (s[p]=='b') {
               getcalls(s, s_len, &p, filters->bud, 8ul);
               filters->notbud = not;
            }
            else if (s[p]=='o') {
               getcalls(s, s_len, &p, filters->objects, 8ul);
               filters->notobject = not;
            }
            else if (s[p]=='t') {
               gettyps(s, s_len, &p, filters->typs, 13ul);
               filters->nottyps = not;
            }
            else if (s[p]=='-') {
               not = 1;
               ++p;
            }
            else return;
         }
      }
   }
   else if (pongto && cmpfrom(s, s_len, p, "ping ", 6ul)) {
      aprsstr_Assign(pongstr, 512ul, s, s_len);
      pongstr[p+1UL] = 'o'; /* ping -> pong */
      while (p<513UL && (uint8_t)pongstr[p]>=' ') ++p;
      pongstr[p] = '\015';
      pongstr[p+1UL] = '\012';
      pongstr[p+2UL] = 0;
      Sendtcp(pongto, pongstr);
      if (maxpongtime>0UL) {
         if (pongto->pongtime+maxpongtime>systime) pongto->pingout = 0;
         pongto->pongtime = systime+maxpongtime;
      }
   }
} /* end GetFilters() */


static void saypongout(pTCPSOCK pt)
{
   FRAMEBUF h;
   FRAMEBUF s;
   strncpy(s,"# ping timeout ",512u);
   aprsstr_IntToStr((int32_t)((systime+maxpongtime)-pt->pongtime), 0UL, h,
                512ul);
   aprsstr_Append(s, 512ul, h, 512ul);
   aprsstr_Append(s, 512ul, "s - data forwarding stopped\015\012", 30ul);
   Sendtcp(pt, s);
} /* end saypongout() */


static char Auth(const char mbuf0[], uint32_t mbuf_len,
                pTCPSOCK pu)
{
   uint32_t vport;
   uint32_t j;
   uint32_t i0;
   FRAMEBUF h1;
   FRAMEBUF h;
   pHEARD pmh;
   struct TCPSOCK * anonym;
   { /* with */
      struct TCPSOCK * anonym = pu;
      i0 = 0UL;
      while ((uint8_t)mbuf0[i0]<=' ') ++i0;
      if (mbuf0[i0]=='#') {
         GetFilters(&anonym->filters, mbuf0, mbuf_len, i0+1UL, 0);
         return 1;
      }
      if (mbuf0[i0]!='u') return 0;
      /* quick test */
      if (cmpfrom(mbuf0, mbuf_len, i0, "user ", 6ul)) {
         i0 += 5UL;
         skipblank(mbuf0, mbuf_len, &i0);
         j = 0UL;
         while (j<9UL && (uint8_t)mbuf0[i0]>' ') {
            anonym->user.call[j] = X2C_CAP(mbuf0[i0]);
            ++i0;
            ++j;
         }
         anonym->user.call[j] = 0;
         skipblank(mbuf0, mbuf_len, &i0);
         if (cmpfrom(mbuf0, mbuf_len, i0, "pass ", 6ul)) {
            i0 += 5UL;
            skipblank(mbuf0, mbuf_len, &i0);
            j = 0UL;
            while (((uint8_t)mbuf0[i0]>='0' && (uint8_t)mbuf0[i0]<='9')
                && j<32768UL) {
               j = (j*10UL+(uint32_t)(uint8_t)mbuf0[i0])-48UL;
               ++i0;
            }
            anonym->valid = aprsstr_Length(anonym->user.call,
                10ul)>=3UL && j==call2pass(anonym->user.call, 10ul);
            skipblank(mbuf0, mbuf_len, &i0);
         }
         if (cmpfrom(mbuf0, mbuf_len, i0, "vers ", 6ul)) {
            i0 += 5UL;
            skipblank(mbuf0, mbuf_len, &i0);
            j = 0UL;
            while ((uint8_t)mbuf0[i0]>=' ' && !cmpfrom(mbuf0, mbuf_len, i0,
                 "filter", 7ul)) {
               if (j<20UL) {
                  anonym->vers[j] = mbuf0[i0];
                  ++j;
               }
               ++i0;
            }
            while (j>0UL && anonym->vers[j-1UL]==' ') --j;
            anonym->vers[j] = 0;
         }
         GetFilters(&anonym->filters, mbuf0, mbuf_len, i0, 0);
         pmh = heardtcp;
         if ((!aprspos_posvalid(anonym->user.pos) && FindUserHeard(&pmh,
                anonym->user.call, &vport)>0UL) && pmh) {
            anonym->user.pos = pmh->position;
         }
         aprsstr_Assign(h, 512ul, "# logresp ", 11ul);
         aprsstr_Append(h, 512ul, anonym->user.call, 10ul);
         if (anonym->valid) aprsstr_Append(h, 512ul, " verified,", 11ul);
         aprsstr_Append(h, 512ul, " server ", 9ul);
         aprsstr_Append(h, 512ul, servercall, 10ul);
         FiltToStr(anonym->filters, h1, 512ul);
         if (h1[0U]) {
            aprsstr_Append(h, 512ul, " filter ", 9ul);
            aprsstr_Append(h, 512ul, h1, 512ul);
         }
         aprsstr_Assign(h1, 512ul, h, 512ul);
         if (anonym->vers[0U]) {
            aprsstr_Append(h1, 512ul, " vers: ", 8ul);
            aprsstr_Append(h1, 512ul, anonym->vers, 21ul);
         }
         logline(1L, h1, 512ul);
         aprsstr_Append(h, 512ul, "\015\012", 3ul);
         Sendtcp(pu, h);
      }
   }
   return 1;
} /* end Auth() */


static void closetcp(pTCPSOCK * w, char fin)
{
   pWWWBUF pb;
   struct TCPSOCK * anonym;
   { /* with */
      struct TCPSOCK * anonym = *w;
      while (anonym->txbuf) {
         pb = anonym->txbuf;
         anonym->txbuf = anonym->txbuf->next;
         /*
         IO.WrStrLn("deallocw");
         */
         osic_free((char * *) &pb, sizeof(struct WWWBUF));
         if (!fin && anonym->txbuf) return;
      }
      if (anonym->fd>=0L) osic_Close(anonym->fd);
      anonym->fd = -1L;
   }
} /* end closetcp() */


static void sendwww(pTCPSOCK * w, WWWB b, int32_t len, char push)
{
   pWWWBUF pp;
   pWWWBUF pb;
   if (len==0L) return;
   /*
   IO.WrStrLn("allocw");
   */
   osic_alloc((char * *) &pb, sizeof(struct WWWBUF));
   if (pb==0) return;
   memcpy(pb->buf,b,1401u);
   pb->tlen = len;
   pb->push = push;
   pb->next = 0;
   if ((*w)->txbuf==0) (*w)->txbuf = pb;
   else {
      pp = (*w)->txbuf;
      while (pp->next) pp = pp->next;
      pp->next = pb;
   }
} /* end sendwww() */

#define udpgate4_RELURL "?reload="

#define udpgate4_SORTURL "?sort="

#define udpgate4_sortTIME "t"

#define udpgate4_sortCALL "c"

#define udpgate4_sortPORT "p"

#define udpgate4_sortHEARD "h"

#define udpgate4_sortDIST "d"

#define udpgate4_sortUP "u"

#define udpgate4_sortDOWN "d"


static void Appwww(pTCPSOCK * wsock, WWWB wbuf, const char s[],
                uint32_t s_len)
{
   uint32_t i0;
   uint32_t slen;
   uint32_t wlen;
   /*
       len:=Length(wbuf);
       IF len+Length(s)>=HIGH(wbuf) THEN
         sendwww(wsock, wbuf, len, FALSE);
         wbuf[0]:=0C;
       END;
       Append(wbuf, s);
   */
   wlen = aprsstr_Length(wbuf, 1401ul);
   slen = aprsstr_Length(s, s_len);
   i0 = 0UL;
   while (i0<slen) {
      if (wlen>=1400UL) {
         sendwww(wsock, wbuf, (int32_t)wlen, 0);
         wlen = 0UL;
      }
      wbuf[wlen] = s[i0];
      ++i0;
      ++wlen;
   }
   wbuf[wlen] = 0;
} /* end Appwww() */


static void AppCall(WWWB wbuf, pTCPSOCK * wsock, char c[],
                uint32_t c_len, char ungate, char center,
                const char link[], uint32_t link_len)
{
   char b[4096];
   uint32_t k;
   uint32_t j;
   uint32_t i0;
   X2C_PCOPY((void **)&c,c_len);
   Appwww(wsock, wbuf, "<TD style=text-align:", 22ul);
   if (center) Appwww(wsock, wbuf, "center>", 8ul);
   else Appwww(wsock, wbuf, "left>", 6ul);
   j = 0UL;
   i0 = 0UL;
   while ((uint8_t)link[i0]>=' ') {
      if (link[i0]=='$') {
         ++i0;
         if (link[i0]=='c') {
            k = 0UL;
            while ((k<=c_len-1 && c[k]) && j<4095UL) {
               b[j] = c[k];
               ++j;
               ++k;
            }
         }
         else if (link[i0]=='$') {
            b[j] = '$';
            if (j<4095UL) ++j;
         }
      }
      else {
         b[j] = link[i0];
         if (j<4095UL) ++j;
      }
      ++i0;
   }
   b[j] = 0;
   if (b[0U]) {
      Appwww(wsock, wbuf, "<a href=", 9ul);
      Appwww(wsock, wbuf, b, 4096ul);
      Appwww(wsock, wbuf, ">", 2ul);
   }
   if (ungate) {
      Appwww(wsock, wbuf, "#", 2ul);
   }
   Appwww(wsock, wbuf, c, c_len);
   if (b[0U]) Appwww(wsock, wbuf, "</a>", 5ul);
   Appwww(wsock, wbuf, "</TD>", 6ul);
   X2C_PFREE(c);
} /* end AppCall() */


static void AppTime(WWWB wbuf, pTCPSOCK * wsock, uint32_t t,
                char tab)
{
   char h1[256];
   if (tab) Appwww(wsock, wbuf, "<td>", 5ul);
   if (systime<t || t==0UL) t = 0UL;
   else t = systime-t;
   aprsstr_TimeToStr(t, h1, 256ul);
   Appwww(wsock, wbuf, h1, 256ul);
   if (tab) Appwww(wsock, wbuf, "</td>", 6ul);
} /* end AppTime() */


static void AppMinSec(WWWB wbuf, pTCPSOCK * wsock, uint32_t t)
{
   char h1[21];
   /*
       Appwww("<td>");
   */
   if (systime<t || t==0UL) t = 0UL;
   else t = systime-t;
   if (t/3600UL>0UL) {
      aprsstr_IntToStr((int32_t)(t/3600UL), 1UL, h1, 21ul);
      Appwww(wsock, wbuf, h1, 21ul);
      Appwww(wsock, wbuf, "h", 2ul);
      t = t%3600UL;
   }
   if (t/60UL>0UL) {
      aprsstr_IntToStr((int32_t)(t/60UL), 1UL, h1, 21ul);
      Appwww(wsock, wbuf, h1, 21ul);
      Appwww(wsock, wbuf, "m", 2ul);
      t = t%60UL;
   }
   aprsstr_IntToStr((int32_t)t, 1UL, h1, 21ul);
   if (h1[1U]==0) {
      Appwww(wsock, wbuf, "&nbsp;", 7ul);
   }
   Appwww(wsock, wbuf, h1, 21ul);
   Appwww(wsock, wbuf, "s", 2ul);
/*
    Appwww("</td>");
*/
} /* end AppMinSec() */


static void AppInt(WWWB wbuf, pTCPSOCK * wsock, int32_t n)
{
   char h1[256];
   Appwww(wsock, wbuf, "<td>", 5ul);
   aprsstr_IntToStr(n, 1UL, h1, 256ul);
   Appwww(wsock, wbuf, h1, 256ul);
   Appwww(wsock, wbuf, "</td>", 6ul);
} /* end AppInt() */


static void AppTxt(WWWB wbuf, pTCPSOCK * wsock, const char style[],
                uint32_t style_len, const char s[], uint32_t s_len)
{
   char c;
   int32_t i0;
   char h1[256];
   int32_t tmp;
   Appwww(wsock, wbuf, style, style_len);
   tmp = (int32_t)aprsstr_Length(s, s_len)-1L;
   i0 = 0L;
   if (i0<=tmp) for (;; i0++) {
      c = s[i0];
      switch ((unsigned)c) {
      case ' ':
      case '.':
      case ',':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
      case 'H':
      case 'I':
      case 'J':
      case 'K':
      case 'L':
      case 'M':
      case 'N':
      case 'O':
      case 'P':
      case 'Q':
      case 'R':
      case 'S':
      case 'T':
      case 'U':
      case 'V':
      case 'W':
      case 'X':
      case 'Y':
      case 'Z':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      case 'j':
      case 'k':
      case 'l':
      case 'm':
      case 'n':
      case 'o':
      case 'p':
      case 'q':
      case 'r':
      case 's':
      case 't':
      case 'u':
      case 'v':
      case 'w':
      case 'x':
      case 'y':
      case 'z':
         Appwww(wsock, wbuf, (char *) &c, 1u/1u);
         break;
      case 0:
      case '\001':
      case '\002':
      case '\003':
      case '\004':
      case '\005':
      case '\006':
      case '\007':
      case '\010':
      case '\011':
      case '\012':
      case '\013':
      case '\014':
      case '\015':
      case '\016':
      case '\017':
      case '\020':
      case '\021':
      case '\022':
      case '\023':
      case '\024':
      case '\025':
      case '\026':
      case '\027':
      case '\030':
      case '\031':
      case '\032':
      case '\033':
      case '\034':
      case '\035':
      case '\036':
      case '\037':
         break;
      default:;
         Appwww(wsock, wbuf, "&#", 3ul);
         aprsstr_IntToStr((int32_t)(uint32_t)(uint8_t)c, 1UL, h1,
                256ul);
         Appwww(wsock, wbuf, h1, 256ul);
         Appwww(wsock, wbuf, ";", 2ul);
         break;
      } /* end switch */
      if (i0==tmp) break;
   } /* end for */
   Appwww(wsock, wbuf, "</td>", 6ul);
} /* end AppTxt() */


static char hex(uint32_t n)
{
   if (n>9UL) n += 7UL;
   return (char)(n+48UL);
} /* end hex() */


static void setcol(uint32_t c, uint32_t n, char h1[],
                uint32_t h1_len)
{
   if (n>100UL) n = 100UL;
   n += 155UL;
   h1[c] = hex(n/16UL);
   h1[c+1UL] = hex(n&15UL);
} /* end setcol() */


static void redgreen(int32_t c, char h1[], uint32_t h1_len)
{
   if (c<0L) c = 0L;
   if (c<100L) {
      setcol(0UL, (uint32_t)c, h1, h1_len);
      setcol(2UL, 100UL, h1, h1_len);
   }
   else {
      if (c>200L) c = 200L;
      setcol(0UL, 100UL, h1, h1_len);
      setcol(2UL, (uint32_t)(200L-c), h1, h1_len);
   }
} /* end redgreen() */


static void green(uint32_t cnt, uint32_t maxt, char driving,
                char h1[], uint32_t h1_len)
{
   uint32_t t;
   t = systime-uptime;
   if (t>maxt) t = maxt;
   if (driving) cnt = cnt*2000UL;
   else cnt = cnt*20000UL;
   redgreen((int32_t)(cnt/(t+300UL)), h1, h1_len);
} /* end green() */


static void wcard64(WWWB wbuf, pTCPSOCK * wsock, uint32_t hi,
                uint32_t lo)
{
   char h1[16];
   if (hi>0UL) {
      aprsstr_CardToStr(lo/1024UL+hi*4194304UL, 1UL, h1, 16ul);
      aprsstr_Append(h1, 16ul, "k", 2ul);
   }
   else aprsstr_CardToStr(lo, 1UL, h1, 16ul);
   Appwww(wsock, wbuf, "<TD style=\"text-align:right\">", 30ul);
   Appwww(wsock, wbuf, h1, 16ul);
   Appwww(wsock, wbuf, "</TD>", 6ul);
} /* end wcard64() */


static void wint(WWWB wbuf, pTCPSOCK * wsock, int32_t n)
{
   char h1[16];
   aprsstr_IntToStr(n, 1UL, h1, 16ul);
   Appwww(wsock, wbuf, "<TD style=\"text-align:right\">", 30ul);
   Appwww(wsock, wbuf, h1, 16ul);
   Appwww(wsock, wbuf, "</TD>", 6ul);
} /* end wint() */


static void wintint(WWWB wbuf, pTCPSOCK * wsock, int32_t n, int32_t k)
{
   char h1[16];
   aprsstr_IntToStr(n, 1UL, h1, 16ul);
   Appwww(wsock, wbuf, "<TD style=\"text-align:right\">", 30ul);
   Appwww(wsock, wbuf, h1, 16ul);
   Appwww(wsock, wbuf, "<BR>", 5ul);
   aprsstr_IntToStr(k, 1UL, h1, 16ul);
   Appwww(wsock, wbuf, h1, 16ul);
   Appwww(wsock, wbuf, "</TD>", 6ul);
} /* end wintint() */


static char openfile(char fn[], uint32_t fn_len, int32_t * fd,
                 int32_t * flen)
{
   char h[4097];
   char hh[2];
   /*
         s:stat_t;
   */
   uint32_t i0;
   char openfile_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   if (fn[0UL]=='/') {
      openfile_ret = 0;
      goto label;
   }
   i0 = 0UL;
   for (;;) {
      if (i0+2UL>=fn_len-1 || fn[i0]==0) break;
      if ((uint8_t)fn[i0]<=' ' || (uint8_t)fn[i0]>=(uint8_t)'\200') {
         openfile_ret = 0;
         goto label;
      }
      if ((fn[i0]=='.' && fn[i0+1UL]=='.') && fn[i0+2UL]=='/') {
         openfile_ret = 0;
         goto label;
      }
      ++i0;
   }
   aprsstr_Assign(h, 4097ul, wwwdir, 1024ul);
   aprsstr_Append(h, 4097ul, fn, fn_len);
   *fd = osi_OpenRead(h, 4097ul);
   if (*fd<0L || osi_RdBin(*fd, (char *)hh, 2u/1u, 1UL)!=1L) {
      openfile_ret = 0;
      goto label;
   }
   /* test read if regular file */
   osic_Seek(*fd, 0UL);
   /*
       fstat(fd, s);
       IF CAST(BITSET, s.st_mode)*CAST(BITSET,170000B)<>CAST(BITSET,
                100000B) THEN RETURN FALSE END;
   */
   *flen = osic_Size(*fd);
   if (*flen<=0L || *flen>=wwwsizelimit) {
      openfile_ret = 0;
      goto label;
   }
   openfile_ret = 1;
   label:;
   X2C_PFREE(fn);
   return openfile_ret;
} /* end openfile() */


static void appsorturl(WWWB wbuf, pTCPSOCK * wsock, const char sortby[],
                uint32_t sortby_len)
{
   char h[3];
   if (sortby[0UL]) {
      Appwww(wsock, wbuf, "?sort=", 7ul);
      h[0U] = sortby[0UL];
      h[1U] = sortby[1UL];
      h[2U] = 0;
      Appwww(wsock, wbuf, h, 3ul);
   }
} /* end appsorturl() */


static void appreloadurl(WWWB wbuf, pTCPSOCK * wsock, uint32_t t)
{
   char h[32];
   if (t) {
      Appwww(wsock, wbuf, "?reload=", 9ul);
      aprsstr_IntToStr((int32_t)t, 1UL, h, 32ul);
      Appwww(wsock, wbuf, h, 32ul);
   }
} /* end appreloadurl() */


static void klick(WWWB wbuf, pTCPSOCK * wsock, char path[],
                uint32_t path_len, char text[], uint32_t text_len,
                char sort)
{
   char s[2];
   X2C_PCOPY((void **)&path,path_len);
   X2C_PCOPY((void **)&text,text_len);
   aprsstr_Assign(s, 2ul, (*wsock)->sortby, 2ul);
   if (sort) {
      if (s[0U]==sort) {
         if (s[1U]!='u') s[1U] = 'u';
         else if (s[1U]!='d') s[1U] = 'd';
      }
      else {
         s[0U] = sort;
         s[1U] = 'u';
      }
   }
   Appwww(wsock, wbuf, "<a href=\"", 10ul);
   Appwww(wsock, wbuf, path, path_len);
   appsorturl(wbuf, wsock, s, 2ul);
   appreloadurl(wbuf, wsock, (*wsock)->reload);
   Appwww(wsock, wbuf, "\">", 3ul);
   Appwww(wsock, wbuf, text, text_len);
   Appwww(wsock, wbuf, "</a> ", 6ul);
   X2C_PFREE(path);
   X2C_PFREE(text);
} /* end klick() */


static void klickraw(WWWB wbuf, pTCPSOCK * wsock, char path[],
                uint32_t path_len, char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&path,path_len);
   X2C_PCOPY((void **)&text,text_len);
   Appwww(wsock, wbuf, "<a href=\"", 10ul);
   Appwww(wsock, wbuf, path, path_len);
   appreloadurl(wbuf, wsock, (*wsock)->reload);
   Appwww(wsock, wbuf, "\">", 3ul);
   Appwww(wsock, wbuf, text, text_len);
   Appwww(wsock, wbuf, "</a>", 5ul);
   X2C_PFREE(path);
   X2C_PFREE(text);
} /* end klickraw() */


static char iconf(char sym, char symt, char fn[],
                uint32_t fn_len, char * overlay)
{
   char h[256];
   uint32_t i0;
   i0 = (uint32_t)(uint8_t)sym;
   if (i0<33UL || i0>127UL) return 0;
   aprsstr_Assign(fn, fn_len, "icon/", 6ul);
   aprsstr_IntToStr((int32_t)(i0+67UL), 3UL, h, 256ul);
   *overlay = 0;
   switch ((unsigned)symt) {
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
   case 'A':
   case 'B':
   case 'C':
   case 'D':
   case 'E':
   case 'F':
   case 'G':
   case 'H':
   case 'I':
   case 'J':
   case 'K':
   case 'L':
   case 'M':
   case 'N':
   case 'O':
   case 'P':
   case 'Q':
   case 'R':
   case 'S':
   case 'T':
   case 'U':
   case 'V':
   case 'W':
   case 'X':
   case 'Y':
   case 'Z':
      h[0U] = '2';
      *overlay = symt;
      break;
   case '\\':
      h[0U] = '2';
      break;
   case '/':
      break;
   default:;
      return 0;
   } /* end switch */
   aprsstr_Append(fn, fn_len, h, 256ul);
   aprsstr_Append(fn, fn_len, ".gif", 5ul);
   aprsstr_Assign(h, 256ul, wwwdir, 1024ul);
   aprsstr_Append(h, 256ul, fn, fn_len);
   return osi_Exists(h, 256ul);
} /* end iconf() */


static void sortindex(pHEARD ph, uint32_t maxtime, const char sortby[],
                 uint32_t sortby_len, char * withqual)
/* make LONGREAL of sort key */
{
   char homevalid;
   uint32_t cj;
   uint32_t i0;
   char c;
   char s;
   double xs;
   *withqual = 0;
   xs = 0.0;
   homevalid = aprspos_posvalid(home);
   s = sortby[0UL];
   while (ph) {
      switch ((unsigned)s) {
      case 't': /* sort by time as it is */
         xs = xs+1.0;
         break;
      case 'c': /* sort by call */
         xs = 0.0;
         for (i0 = 0UL; i0<=9UL; i0++) {
            c = ph->call[i0];
            xs = xs*128.0+(double)(uint32_t)(uint8_t)c;
         } /* end for */
         break;
      case 'p': /* sort by port */
         xs = (double)ph->fromrx;
         break;
      case 'h': /* sort by count */
         xs = (double)MHcount(ph, maxtime, &cj);
         break;
      case 'd': /* sort by km */
         if (homevalid && aprspos_posvalid(ph->position)) {
            xs = (double)aprspos_distance(home, ph->position);
         }
         else xs = 0.0;
         break;
      } /* end switch */
      if (sortby[1UL]=='d') ph->sortval = -xs;
      else ph->sortval = xs;
      if ((ph->txd || ph->level) || ph->quali) *withqual = 1;
      ph = ph->next;
   }
} /* end sortindex() */


static void getlinkfile(char b[], uint32_t b_len, const char fn[],
                uint32_t fn_len)
{
   int32_t len;
   int32_t fd;
   aprsstr_Assign(b, b_len, wwwdir, 1024ul);
   aprsstr_Append(b, b_len, fn, fn_len);
   fd = osi_OpenRead(b, b_len);
   b[0UL] = 0;
   if (fd>=0L) {
      len = osi_RdBin(fd, (char *)b, (b_len)/1u, b_len-1UL);
      if (len<0L) len = 0L;
      b[len] = 0;
      osic_Close(fd);
   }
} /* end getlinkfile() */


static void apppos(WWWB wbuf, pTCPSOCK * wsock, struct aprsstr_POSITION pos,
                char withloc)
{
   char h[32];
   if (aprspos_posvalid(pos)) {
      aprsstr_FixToStr(X2C_DIVR(pos.lat,1.7453292519444E-2f), 5UL, h, 32ul);
      Appwww(wsock, wbuf, " ", 2ul);
      Appwww(wsock, wbuf, h, 32ul);
      aprsstr_FixToStr(X2C_DIVR(pos.long0,1.7453292519444E-2f), 5UL, h,
                32ul);
      Appwww(wsock, wbuf, "/", 2ul);
      Appwww(wsock, wbuf, h, 32ul);
      if (withloc) {
         postoloc(h, 32ul, pos);
         Appwww(wsock, wbuf, " ", 2ul);
         Appwww(wsock, wbuf, h, 32ul);
      }
   }
   else Appwww(wsock, wbuf, " (NoPos)", 9ul);
} /* end apppos() */


static void appmhcnt(WWWB wbuf, pTCPSOCK * wsock, uint32_t n,
                uint32_t maxtime, char dir, char net,
                char jnk, char drive, char call[],
                uint32_t call_len, uint32_t port)
{
   char h3[32];
   char h2[32];
   char h1[32];
   uint32_t nc;
   X2C_PCOPY((void **)&call,call_len);
   nc = n;
   if (jnk) nc = nc*10UL;
   Appwww(wsock, wbuf, "<td style=\"background-color:#", 30ul);
   strncpy(h1,"80FF80",32u);
   green(nc, maxtime, drive, h1, 32ul);
   Appwww(wsock, wbuf, h1, 32ul);
   Appwww(wsock, wbuf, "\">", 3ul);
   aprsstr_IntToStr((int32_t)n, 1UL, h1, 32ul);
   if (rawlines==0UL || n==0UL) Appwww(wsock, wbuf, h1, 32ul);
   else {
      strncpy(h2,"raw?",32u);
      if (dir) aprsstr_Append(h2, 32ul, "d", 2ul);
      else if (net) aprsstr_Append(h2, 32ul, "n", 2ul);
      else aprsstr_Append(h2, 32ul, "v", 2ul);
      if (jnk) aprsstr_Append(h2, 32ul, "j", 2ul);
      else aprsstr_Append(h2, 32ul, "c", 2ul);
      aprsstr_IntToStr((int32_t)port, 1UL, h3, 32ul);
      aprsstr_Append(h2, 32ul, h3, 32ul);
      aprsstr_Append(h2, 32ul, "=", 2ul);
      aprsstr_Append(h2, 32ul, call, call_len);
      klickraw(wbuf, wsock, h2, 32ul, h1, 32ul);
   }
   Appwww(wsock, wbuf, "</td>", 6ul);
   X2C_PFREE(call);
} /* end appmhcnt() */


static void showmh(WWWB wbuf, pTCPSOCK * wsock, char h1[256], pHEARD ph0,
                 char dir, char net, uint32_t maxtime,
                char title0[], uint32_t title_len,
                const char sortby[], uint32_t sortby_len)
{
   char withqual;
   char withicon;
   char withport;
   uint32_t cj;
   uint32_t ci;
   char hch;
   pHEARD phs;
   pHEARD ph;
   double xs;
   char callink[1024];
   X2C_PCOPY((void **)&title0,title_len);
   getlinkfile(callink, 1024ul, "calllink.txt", 13ul);
   sortindex(ph0, maxtime, sortby, sortby_len, &withqual);
   withport = udpsocks && udpsocks->next;
   aprsstr_Assign(h1, 256ul, wwwdir, 1024ul);
   aprsstr_Append(h1, 256ul, "icon", 5ul);
   withicon = osi_Exists(h1, 256ul);
   Appwww(wsock, wbuf, "<table id=mheard border=0 align=center CELLPADDING=3 \
CELLSPACING=1 BGCOLOR=#FFFFFF><tr class=tab-mh-titel BGCOLOR=#A9EEFF><th cols\
pan=", 135ul);
   aprsstr_IntToStr((int32_t)(7UL+(uint32_t)withicon+(uint32_t)
                withport+(uint32_t)(dir && withqual)*3UL+(uint32_t)net),
                1UL, h1, 256ul);
   Appwww(wsock, wbuf, h1, 256ul);
   Appwww(wsock, wbuf, ">", 2ul);
   Appwww(wsock, wbuf, title0, title_len);
   if (maxtime%3600UL==0UL) {
      aprsstr_IntToStr((int32_t)(maxtime/3600UL), 1UL, h1, 256ul);
      aprsstr_Append(h1, 256ul, "h", 2ul);
   }
   else {
      aprsstr_IntToStr((int32_t)(maxtime/60UL), 1UL, h1, 256ul);
      aprsstr_Append(h1, 256ul, "min", 4ul);
   }
   Appwww(wsock, wbuf, h1, 256ul);
   /*    Appwww('</th></tr><tr class=tab-mh-head BGCOLOR=#E0C080><th>Call</th>'
                ); */
   Appwww(wsock, wbuf, "</th></tr><tr class=tab-mh-head BGCOLOR=#E0C080><th>",
                 53ul);
   klick(wbuf, wsock, "mh", 3ul, "Call", 5ul, 'c');
   Appwww(wsock, wbuf, "</th>", 6ul);
   if (withicon) Appwww(wsock, wbuf, "<th>Icon</th>", 14ul);
   /*    IF withport THEN Appwww('<th>Port</th>') END; */
   if (withport) {
      Appwww(wsock, wbuf, "<th>", 5ul);
      klick(wbuf, wsock, "mh", 3ul, "Port", 5ul, 'p');
      Appwww(wsock, wbuf, "</th>", 6ul);
   }
   /*    Appwww('<th>Last Heard</th><th>Pack</th><th>QRB km</th><th>Data</th><th>Path</th></tr>'
                ); */
   Appwww(wsock, wbuf, "<th>", 5ul);
   klick(wbuf, wsock, "mh", 3ul, "Last Heard", 11ul, 't');
   if (dir && withqual) {
      Appwww(wsock, wbuf, "</th><th>Txd</th><th>Lev</th><th>q&#37;", 40ul);
   }
   if (net) Appwww(wsock, wbuf, "</th><th>Position", 18ul);
   Appwww(wsock, wbuf, "</th><th>", 10ul);
   klick(wbuf, wsock, "mh", 3ul, "Pack", 5ul, 'h');
   Appwww(wsock, wbuf, "</th><th>Junk</th><th>", 23ul);
   klick(wbuf, wsock, "mh", 3ul, "QRB km", 7ul, 'd');
   Appwww(wsock, wbuf, "</th><th>Data</th><th>Path</th></tr>", 37ul);
   for (;;) {
      phs = ph0;
      xs = X2C_max_longreal;
      ph = 0;
      while (phs) {
         if (phs->sortval<xs) {
            xs = phs->sortval;
            ph = phs;
         }
         phs = phs->next;
      }
      if (ph==0) break;
      ph->sortval = X2C_max_longreal;
      if (ph->mhtime+maxtime>systime) {
         ci = MHcount(ph, maxtime, &cj);
         if (cj>ci) cj = ci;
         Appwww(wsock, wbuf, "<tr style=\"background-color:#90FF90; text-alig\
n:right\">", 56ul);
         AppCall(wbuf, wsock, ph->call, 10ul, ph->ungate, 0, callink,
                1024ul);
         if (withicon) {
            Appwww(wsock, wbuf, "<td", 4ul);
            if (iconf(ph->sym, ph->symt, h1, 256ul, &hch)) {
               Appwww(wsock, wbuf, " style=\"background-image:url(", 30ul);
               Appwww(wsock, wbuf, h1, 256ul);
               Appwww(wsock, wbuf, "); background-repeat:no-repeat;background\
-position:center center;", 66ul);
               if (hch) {
                  Appwww(wsock, wbuf, "text-align:center\">", 20ul);
                  Appwww(wsock, wbuf, (char *) &hch, 1u/1u);
               }
               else Appwww(wsock, wbuf, "\">", 3ul);
            }
            else Appwww(wsock, wbuf, ">", 2ul);
            Appwww(wsock, wbuf, "</td>", 6ul);
         }
         if (withport) {
            Appwww(wsock, wbuf, "<td>", 5ul);
            getportname(ph->fromrx, h1, 256ul);
            Appwww(wsock, wbuf, h1, 256ul);
            Appwww(wsock, wbuf, "</td>", 6ul);
         }
         Appwww(wsock, wbuf, "<td>", 5ul);
         AppMinSec(wbuf, wsock, ph->mhtime);
         Appwww(wsock, wbuf, "</td>", 6ul);
         if (dir && withqual) {
            /* txdel */
            Appwww(wsock, wbuf, "<td style=\"background-color:#", 30ul);
            strncpy(h1,"80FF80",256u);
            redgreen((int32_t)ph->txd-50L, h1, 256ul);
            Appwww(wsock, wbuf, h1, 256ul);
            Appwww(wsock, wbuf, "\">", 3ul);
            if (ph->txd) {
               aprsstr_IntToStr((int32_t)ph->txd, 1UL, h1, 256ul);
               Appwww(wsock, wbuf, h1, 256ul);
            }
            Appwww(wsock, wbuf, "</td>", 6ul);
            /* level */
            Appwww(wsock, wbuf, "<td style=\"background-color:#", 30ul);
            strncpy(h1,"80FF80",256u);
            if (ph->level) {
               redgreen(((int32_t)Min((uint32_t)(uint8_t)
                abs(ph->level+15), 15UL)-5L)*10L, h1, 256ul);
            }
            Appwww(wsock, wbuf, h1, 256ul);
            Appwww(wsock, wbuf, "\">", 3ul);
            if (ph->level) {
               aprsstr_IntToStr((int32_t)ph->level, 1UL, h1, 256ul);
               Appwww(wsock, wbuf, h1, 256ul);
            }
            Appwww(wsock, wbuf, "</td>", 6ul);
            /*quality */
            Appwww(wsock, wbuf, "<td>", 5ul);
            if (ph->quali) {
               aprsstr_IntToStr((int32_t)ph->quali, 1UL, h1, 256ul);
               Appwww(wsock, wbuf, h1, 256ul);
            }
            Appwww(wsock, wbuf, "</td>", 6ul);
         }
         if (net) {
            Appwww(wsock, wbuf, "<td>", 5ul);
            apppos(wbuf, wsock, ph->position, 0);
            Appwww(wsock, wbuf, "</td>", 6ul);
         }
         /*pack count */
         appmhcnt(wbuf, wsock, ci-cj, maxtime, dir, net, 0, ph->datatyp=='S',
                 ph->call, 10ul, ph->fromrx);
         /*Junk count */
         appmhcnt(wbuf, wsock, cj, maxtime, dir, net, 1, 0, ph->call, 10ul,
                ph->fromrx);
         /*dist */
         h1[0U] = 0;
         if (aprspos_posvalid(ph->position) && aprspos_posvalid(home)) {
            aprsstr_FixToStr(aprspos_distance(home, ph->position)+0.05f, 2UL,
                 h1, 256ul);
         }
         Appwww(wsock, wbuf, "<td>", 5ul);
         Appwww(wsock, wbuf, h1, 256ul);
         Appwww(wsock, wbuf, "</td>", 6ul);
         Appwww(wsock, wbuf, "<td>", 5ul);
         if (ph->datatyp=='C') {
            aprsstr_FixToStr(ph->data, 2UL, h1, 256ul);
            aprsstr_Append(h1, 256ul, "\260C", 3ul);
         }
         else if (ph->datatyp=='S') {
            aprsstr_FixToStr(ph->data, 0UL, h1, 256ul);
            aprsstr_Append(h1, 256ul, "kmh", 4ul);
         }
         else h1[0] = 0;
         Appwww(wsock, wbuf, h1, 256ul);
         Appwww(wsock, wbuf, "</td>", 6ul);
         AppTxt(wbuf, wsock, "<td style=text-align:left>", 27ul, ph->head,
                41ul);
         Appwww(wsock, wbuf, "</tr>\015\012", 8ul);
      }
   }
   Appwww(wsock, wbuf, "</table>&nbsp;", 15ul);
   X2C_PFREE(title0);
} /* end showmh() */


static void ap(uint32_t * j, char h[4096], char s[],
                uint32_t s_len)
{
   uint32_t ii;
   X2C_PCOPY((void **)&s,s_len);
   ii = 0UL;
   while (ii<=s_len-1 && s[ii]) {
      h[*j] = s[ii];
      ++*j;
      ++ii;
   }
   X2C_PFREE(s);
} /* end ap() */


static void listraw(WWWB wbuf, pTCPSOCK * wsock, const char s[],
                uint32_t s_len)
{
   pHEARD ph;
   pRAWTEXT pr;
   uint32_t ju;
   uint32_t port;
   uint32_t j;
   uint32_t i0;
   MONCALL call;
   char ch;
   char dir;
   char h[4096];
   dir = 0;
   if (aprsstr_InStr(s, s_len, "?d", 3ul)==3L) {
      ph = hearddir;
      dir = 1;
   }
   else if (aprsstr_InStr(s, s_len, "?v", 3ul)==3L) ph = heardvia;
   else if (aprsstr_InStr(s, s_len, "?n", 3ul)==3L) ph = heardtcp;
   else ph = 0;
   ju = (uint32_t)(s[5UL]=='j');
   i0 = 6UL;
   port = 0UL;
   while ((uint8_t)s[i0]>='0' && (uint8_t)s[i0]<='9') {
      port = (port*10UL+(uint32_t)(uint8_t)s[i0])-48UL;
      ++i0;
   }
   call[0] = 0;
   if (s[i0]=='=') {
      ++i0;
      while ((i0<=s_len-1 && ((uint8_t)s[i0]>='0' && (uint8_t)
                s[i0]<='9' || (uint8_t)s[i0]>='A' && (uint8_t)s[i0]<='Z')
                 || s[i0]=='^') || s[i0]=='-') {
         aprsstr_Append(call, 10ul, (char *) &s[i0], 1u/1u);
         ++i0;
      }
   }
   Appwww(wsock, wbuf, "<title>", 8ul);
   Appwww(wsock, wbuf, call, 10ul);
   Appwww(wsock, wbuf, "</title></head><body><div id=\"wrapper-junk\">\015\01\
2", 47ul);
   while (ph) {
      if (ph->fromrx==port && (call[0U]==0 || aprsstr_StrCmp(call, 10ul,
                ph->call, 10ul))) {
         pr = ph->rawtext[ju];
         while (pr) {
            Appwww(wsock, wbuf, "<div class=\"junk-mh\">", 22ul);
            aprsstr_DateToStr(pr->htime, h, 4096ul);
            h[4U] = h[5U];
            h[5U] = h[6U];
            h[6U] = h[8U];
            h[7U] = h[9U];
            h[8U] = ' ';
            h[9U] = h[11U];
            h[10U] = h[12U];
            h[11U] = h[14U];
            h[12U] = h[15U];
            h[13U] = h[17U];
            h[14U] = h[18U];
            h[15U] = ' ';
            h[16U] = 0;
            Appwww(wsock, wbuf, "<span class=\"junk-ts\">", 23ul);
            Appwww(wsock, wbuf, h, 4096ul);
            Appwww(wsock, wbuf, "</span>", 8ul);
            i0 = 0UL;
            j = 0UL;
            for (;;) {
               ch = pr->text[i0];
               if (ch==0) break;
               if (ch=='<') {
                  h[j] = '&';
                  ++j;
                  h[j] = 'l';
                  ++j;
                  h[j] = 't';
                  ++j;
                  h[j] = ';';
                  ++j;
               }
               else if (ch=='>') {
                  h[j] = '&';
                  ++j;
                  h[j] = 'g';
                  ++j;
                  h[j] = 't';
                  ++j;
                  h[j] = ';';
                  ++j;
               }
               else if (ch=='&') {
                  h[j] = '&';
                  ++j;
                  h[j] = 'a';
                  ++j;
                  h[j] = 'm';
                  ++j;
                  h[j] = 'p';
                  ++j;
                  h[j] = ';';
                  ++j;
               }
               else if (ch=='\"') {
                  h[j] = '&';
                  ++j;
                  h[j] = 'q';
                  ++j;
                  h[j] = 'u';
                  ++j;
                  h[j] = 'o';
                  ++j;
                  h[j] = 't';
                  ++j;
                  h[j] = ';';
                  ++j;
               }
               else if (ch=='\'') {
                  h[j] = '&';
                  ++j;
                  h[j] = 'a';
                  ++j;
                  h[j] = 'p';
                  ++j;
                  h[j] = 'o';
                  ++j;
                  h[j] = 's';
                  ++j;
                  h[j] = ';';
                  ++j;
               }
               else if (ch=='/') {
                  h[j] = '&';
                  ++j;
                  h[j] = '#';
                  ++j;
                  h[j] = '4';
                  ++j;
                  h[j] = '7';
                  ++j;
                  h[j] = ';';
                  ++j;
               }
               else if ((uint8_t)ch<' ' || (uint8_t)ch>='\177') {
                  ap(&j, h, "<span class=\"junk-bin\">", 24ul);
                  h[j] = '&';
                  ++j;
                  h[j] = 'l';
                  ++j;
                  h[j] = 't';
                  ++j;
                  h[j] = ';';
                  ++j;
                  h[j] = hex((uint32_t)(uint8_t)ch/16UL);
                  ++j;
                  h[j] = hex((uint32_t)(uint8_t)ch&15UL);
                  ++j;
                  h[j] = '&';
                  ++j;
                  h[j] = 'g';
                  ++j;
                  h[j] = 't';
                  ++j;
                  h[j] = ';';
                  ++j;
                  ap(&j, h, "</span>", 8ul);
               }
               else {
                  h[j] = ch;
                  ++j;
               }
               if (j>4045UL) {
                  h[j] = 0;
                  Appwww(wsock, wbuf, h, 4096ul);
                  j = 0UL;
               }
               ++i0;
               if (i0>1023UL) break;
            }
            if (j>0UL) {
               h[j] = 0;
               Appwww(wsock, wbuf, h, 4096ul);
            }
            if (ju==1UL) {
               Appwww(wsock, wbuf, "<span class=\"junk-reason\">&lt;", 31ul);
               Appwww(wsock, wbuf, "no or invalid position", 23ul);
               Appwww(wsock, wbuf, "&gt;</span>", 12ul);
            }
            if (dir && pr->txd>0UL) {
               Appwww(wsock, wbuf, "<span class=\"junk-txd\"> (txd=", 30ul);
               aprsstr_IntToStr((int32_t)pr->txd, 0UL, h, 4096ul);
               Appwww(wsock, wbuf, h, 4096ul);
               Appwww(wsock, wbuf, ")</span>", 9ul);
            }
            Appwww(wsock, wbuf, "</div>\015\012", 9ul);
            pr = pr->next;
         }
      }
      ph = ph->next;
   }
   Appwww(wsock, wbuf, "</div></body></html>\015\012", 23ul);
} /* end listraw() */


static void getreload(char s[], uint32_t s_len, uint32_t * r)
{
   int32_t len;
   int32_t j;
   int32_t i0;
   uint32_t n;
   n = 0UL;
   i0 = aprsstr_InStr(s, s_len, "?reload=", 9ul);
   if (i0>=0L) {
      len = (int32_t)aprsstr_Length(s, s_len);
      j = i0;
      i0 += (int32_t)aprsstr_Length("?reload=", 9ul);
      while ((i0<len && (uint8_t)s[i0]>='0') && (uint8_t)s[i0]<='9') {
         n = (n*10UL+(uint32_t)(uint8_t)s[i0])-48UL;
         ++i0;
      }
      while (i0<=len) {
         s[j] = s[i0];
         ++i0;
         ++j;
      }
   }
   if (n>0UL) *r = n;
} /* end getreload() */


static void getmh(char s[], uint32_t s_len, char sortby[],
                uint32_t sortby_len)
{
   int32_t len;
   int32_t j;
   int32_t i0;
   sortby[0UL] = 0;
   sortby[1UL] = 0;
   i0 = aprsstr_InStr(s, s_len, "?sort=", 7ul);
   if (i0>=0L) {
      len = (int32_t)aprsstr_Length(s, s_len);
      j = i0;
      i0 += (int32_t)aprsstr_Length("?sort=", 7ul);
      sortby[0UL] = s[i0];
      ++i0;
      sortby[1UL] = s[i0];
      ++i0;
      while (i0<=len) {
         s[j] = s[i0];
         ++i0;
         ++j;
      }
   }
} /* end getmh() */


static void appreload(WWWB wbuf, pTCPSOCK * wsock, uint32_t t)
{
   char h[32];
   if (t) {
      Appwww(wsock, wbuf, "<meta http-equiv=\"refresh\" content=\"", 37ul);
      aprsstr_IntToStr((int32_t)(t*60UL), 1UL, h, 32ul);
      Appwww(wsock, wbuf, h, 32ul);
      Appwww(wsock, wbuf, "\">", 3ul);
   }
} /* end appreload() */


static void reloadklick(WWWB wbuf, pTCPSOCK * wsock)
{
   char h1[16];
   /*
       Appwww('<form method="get">reload min:<input type="text" value="');
   */
   Appwww(wsock, wbuf, "Reload (min):<input type=\"text\" value=\"", 40ul);
   aprsstr_IntToStr((int32_t)(*wsock)->reload, 1UL, h1, 16ul);
   Appwww(wsock, wbuf, h1, 16ul);
   Appwww(wsock, wbuf, "\" name=\"reload\" size=\"4\" maxlength=\"3\"><input \
type=\"submit\" value=\"set reload time\"/></form>", 93ul);
} /* end reloadklick() */


static void klicks(WWWB wbuf, pTCPSOCK * wsock)
{
   /*
       Appwww('<div style="text-align:center" class="nav">');
   */
   Appwww(wsock, wbuf, "<div style=\"text-align:center\" class=\"nav\"><form \
method=\"get\">", 63ul);
   klick(wbuf, wsock, "/", 2ul, "CONNECTS", 9ul, 0);
   klick(wbuf, wsock, "mh", 3ul, "HEARD", 6ul, 0);
   if (maxmsg>0UL) klick(wbuf, wsock, "msg", 4ul, "MSGRELAY", 9ul, 0);
   klick(wbuf, wsock, "info.html", 10ul, "INFO", 5ul, 0);
   reloadklick(wbuf, wsock);
   Appwww(wsock, wbuf, "</div>", 7ul);
} /* end klicks() */


static void conthead(pTCPSOCK * wsock, WWWB wbuf, int32_t flen)
{
   char h[31];
   strncpy(wbuf,"HTTP/1.0 200 OK\015\012",1401u);
   if (flen>=0L) {
      Appwww(wsock, wbuf, "Content-Length: ", 17ul);
      aprsstr_IntToStr(flen, 1UL, h, 31ul);
      aprsstr_Append(h, 31ul, "\015\012\015\012", 5ul);
      Appwww(wsock, wbuf, h, 31ul);
   }
   else {
      Appwww(wsock, wbuf, "Content-Type: text/html; charset=iso-8859-1\015\01\
2\015\012<html><head><link href=\"style.css\" rel=\"stylesheet\" type=\"text/\
css\"><meta charset=\"utf-8\">", 138ul);
      appreload(wbuf, wsock, (*wsock)->reload);
   }
/*
    IF ascii THEN Append(wbuf, STYLE+'</head><body>'+CR+LF);
    ELSE Append(wbuf, "<html><head>") END;
    appreload(wsock^.reload);
*/
} /* end conthead() */


static void title(WWWB wbuf, pTCPSOCK * wsock, uint32_t * cnt,
                char maintit, const char comment0[],
                uint32_t comment_len, const char msgvia[],
                uint32_t msgvia_len)
{
   char h[32];
   ++*cnt;
   Appwww(wsock, wbuf, "<title>", 8ul);
   Appwww(wsock, wbuf, servercall, 10ul);
   Appwww(wsock, wbuf, comment0, comment_len);
   Appwww(wsock, wbuf, " - dxlAPRS Toolchain</title></head>\015\012<body><div\
 style=\"text-align:center\"><h3>\015\012", 81ul);
   if (maintit) {
      if (servercall[0U]) {
         Appwww(wsock, wbuf, "Server ", 8ul);
         Appwww(wsock, wbuf, servercall, 10ul);
      }
      if (tcpbindport[0U]) {
         Appwww(wsock, wbuf, "  Port ", 8ul);
         Appwww(wsock, wbuf, tcpbindport, 6ul);
      }
      Appwww(wsock, wbuf, " [udpgate 0.68] Maxusers ", 26ul);
      aprsstr_IntToStr((int32_t)maxusers, 1UL, h, 32ul);
      Appwww(wsock, wbuf, h, 32ul);
      Appwww(wsock, wbuf, " http#", 7ul);
   }
   else {
      if (msgvia[0UL]) {
         Appwww(wsock, wbuf, " MsgCall ", 10ul);
         Appwww(wsock, wbuf, msgvia, msgvia_len);
      }
      else Appwww(wsock, wbuf, servercall, 10ul);
      apppos(wbuf, wsock, home, 1);
      Appwww(wsock, wbuf, " [udpgate 0.68] http#", 22ul);
   }
   aprsstr_IntToStr((int32_t)*cnt, 1UL, h, 32ul);
   Appwww(wsock, wbuf, h, 32ul);
   Appwww(wsock, wbuf, " Uptime ", 9ul);
   AppTime(wbuf, wsock, uptime, 0);
   Appwww(wsock, wbuf, "</h3></div>\015\012", 14ul);
} /* end title() */


static void Www(pTCPSOCK wsock)
{
   WWWB wbuf;
   char h1[256];
   int32_t res0;
   int32_t flen;
   int32_t fdw;
   uint32_t ut;
   uint32_t tt;
   pTCPSOCK ss;
   pUDPSOCK us;
   uint32_t i0;
   char serverlink[1024];
   pMESSAGE mp;
   uint8_t tp;
   char hok;
   struct TCPSOCK * anonym;
   struct TCPSOCK * anonym0;
   struct _0 * anonym1;
   struct TCPSOCK * anonym2;
   /* Www */
   res0 = readsock(wsock->fd, wbuf, 1401L);
   i0 = 0UL;
   for (;;) {
      if ((int32_t)i0>=res0) {
         if (res0<=0L) {
            osic_Close(wsock->fd);
            wsock->fd = -1L;
         }
         return;
      }
      { /* with */
         struct TCPSOCK * anonym = wsock;
         anonym->get[anonym->wwwst] = wbuf[i0];
         if (anonym->wwwst<255UL) ++anonym->wwwst;
         else {
            anonym->get[anonym->wwwst-4UL] = anonym->get[anonym->wwwst-3UL];
            anonym->get[anonym->wwwst-3UL] = anonym->get[anonym->wwwst-2UL];
            anonym->get[anonym->wwwst-2UL] = anonym->get[anonym->wwwst-1UL];
            anonym->get[anonym->wwwst-1UL] = anonym->get[anonym->wwwst];
         }
         anonym->get[anonym->wwwst] = 0;
         /*
         IO.WrCard(wwwst, 10); FOR tt:=0 TO wwwst DO IO.WrHex(ORD(get[tt]), 3);
                 END; IO.WrLn;
         */
         if (((((anonym->wwwst>=4UL && anonym->get[anonym->wwwst-4UL]
                =='\015') && anonym->get[anonym->wwwst-3UL]=='\012')
                && anonym->get[anonym->wwwst-2UL]=='\015')
                && anonym->get[anonym->wwwst-1UL]=='\012')
                && cmpfrom(anonym->get, 256ul, 0UL, "GET /", 6ul)) break;
         ++i0;
      }
   }
   { /* with */
      struct TCPSOCK * anonym0 = wsock;
      i0 = 5UL;
      while (i0<=255UL && (uint8_t)anonym0->get[i0]>' ') {
         anonym0->get[i0-5UL] = anonym0->get[i0];
         ++i0;
      }
      anonym0->get[i0-5UL] = 0;
      getreload(anonym0->get, 256ul, &anonym0->reload);
      getmh(anonym0->get, 256ul, anonym0->sortby, 2ul);
   }
   if (wsock->get[0U]==0) {
      /*    INC(httpcount); */
      conthead(&wsock, wbuf, -1L);
      title(wbuf, &wsock, &httpcount, 1, " Status Report", 15ul, "", 1ul);
      /*
          IF QWatch.qsize>0 THEN
            Appwww(" txq ");
            IntToStr(QWatch.qsize, 1, h1); Appwww(h1); Appwww("s");
          END;
      */
      klicks(wbuf, &wsock);
      /* udp ports*/
      Appwww(&wsock, wbuf, "<table id=connections BORDER=\"0\" CELLPADDING=3 \
CELLSPACING=1 BGCOLOR=#000000 align=center SUMMARY=\"Connection Table\">\015\01\
2<TR class=tab-con-titel VALIGN=\"BASELINE\" BGCOLOR=\"#A999CC\"><TH COLSPAN=\
14 style=\"text-align:center\">Server Connections</TH></TR>\015\012<TR class=\
tab-con-head BGCOLOR=\"#CCCCFF\">\015\012<TH>Dir</TH><TH>IPnum</TH><TH>Port</\
TH><TH>Call/Port</TH><TH>V</TH> <TH>Software</TH> <TH NOWRAP>Range Filter</TH\
><TH>TxByte</TH> <TH>TxFr</TH><TH NOWRAP>bit/s</TH><TH>RxByte</TH> <TH>RxFr</\
TH><TH NOWRAP>bit/s</TH><TH>Up</TH></TR>", 524ul);
      us = udpsocks;
      while (us) {
         if (us->txbytes>0UL) {
            showpip(us->ip, 0UL, h1, 256ul);
            Appwww(&wsock, wbuf, "<TR class=tab-con-udp BGCOLOR=#E0E0F0 style\
=\"text-align:center\">\015\012<TD>udp</TD><TD>", 83ul);
            Appwww(&wsock, wbuf, h1, 256ul);
            Appwww(&wsock, wbuf, "</TD>", 6ul);
            wint(wbuf, &wsock, (int32_t)us->dport);
            Appwww(&wsock, wbuf, "<TD style=\"text-align:left\">T:", 31ul);
            Appwww(&wsock, wbuf, us->portname, 10ul);
            Appwww(&wsock, wbuf, "</TD><TD></TD><TD></TD>", 24ul);
            if (us->torfradius>=1.0f || us->maxbytes>0UL) {
               Appwww(&wsock, wbuf, "<td>", 5ul);
               if (us->torfradius>=1.0f) {
                  aprsstr_IntToStr((int32_t)(uint32_t)
                X2C_TRUNCC(us->torfradius,0UL,X2C_max_longcard), 1UL, h1,
                256ul);
                  Appwww(&wsock, wbuf, h1, 256ul);
                  Appwww(&wsock, wbuf, "km ", 4ul);
               }
               if (us->maxbytes>0UL) {
                  aprsstr_IntToStr((int32_t)us->maxbytes, 1UL, h1, 256ul);
                  Appwww(&wsock, wbuf, h1, 256ul);
                  Appwww(&wsock, wbuf, "B/s", 4ul);
               }
               Appwww(&wsock, wbuf, "</td>", 6ul);
            }
            else Appwww(&wsock, wbuf, "<TD></TD>", 10ul);
            wint(wbuf, &wsock, (int32_t)us->txbytes);
            wint(wbuf, &wsock, (int32_t)us->txframes);
            if (uptime<systime) ut = systime-uptime;
            else ut = 1UL;
            wint(wbuf, &wsock, (int32_t)(us->txbytes/(ut/8UL+1UL)));
            wint(wbuf, &wsock, 0L);
            wint(wbuf, &wsock, 0L);
            wint(wbuf, &wsock, 0L);
            Appwww(&wsock, wbuf, "<TD>", 5ul);
            aprsstr_TimeToStr(ut, h1, 256ul);
            Appwww(&wsock, wbuf, h1, 256ul);
            Appwww(&wsock, wbuf, "</TD></TR>", 11ul);
         }
         for (i0 = 0UL; i0<=15UL; i0++) {
            { /* with */
               struct _0 * anonym1 = &us->stat[i0];
               if (anonym1->utime+600UL>systime) {
                  showpip(anonym1->uip, 0UL, h1, 256ul);
                  Appwww(&wsock, wbuf, "<TR class=tab-con-udp BGCOLOR=#E0E0F0\
 style=\"text-align:center\">\015\012<TD>udp</TD><TD>", 83ul);
                  Appwww(&wsock, wbuf, h1, 256ul);
                  Appwww(&wsock, wbuf, "</TD>", 6ul);
                  wint(wbuf, &wsock, (int32_t)anonym1->uport);
                  Appwww(&wsock, wbuf, "<TD style=\"text-align:left\">R:",
                31ul);
                  Appwww(&wsock, wbuf, us->portname, 10ul);
                  Appwww(&wsock, wbuf, "</TD><TD></TD><TD></TD><TD></TD>",
                33ul);
                  wint(wbuf, &wsock, 0L);
                  wint(wbuf, &wsock, 0L);
                  if (anonym1->dtime<systime) ut = systime-anonym1->dtime;
                  else ut = 1UL;
                  wint(wbuf, &wsock, 0L);
                  wint(wbuf, &wsock, (int32_t)anonym1->rxbytes);
                  wint(wbuf, &wsock, (int32_t)anonym1->rxframes);
                  wint(wbuf, &wsock,
                (int32_t)(anonym1->rxbytes/(ut/8UL+1UL)));
                  Appwww(&wsock, wbuf, "<TD>", 5ul);
                  aprsstr_TimeToStr(ut, h1, 256ul);
                  Appwww(&wsock, wbuf, h1, 256ul);
                  Appwww(&wsock, wbuf, "</TD></TR>", 11ul);
               }
            }
         } /* end for */
         us = us->next;
      }
      /* tcp ports */
      getlinkfile(serverlink, 1024ul, "serverlink.txt", 15ul);
      ss = tcpsocks;
      while (ss) {
         { /* with */
            struct TCPSOCK * anonym2 = ss;
            if (anonym2->service=='S' || anonym2->service=='G') {
               if (anonym2->connt>0UL && anonym2->connt<systime) {
                  tt = systime-anonym2->connt;
               }
               else tt = 1UL;
               /*
                         Appwww('<TR class=tab-con-tcp style="
                text-align:center" ');
               */
               if (anonym2->service=='S') {
                  Appwww(&wsock, wbuf, "<TR class=tab-con-in style=\"text-ali\
gn:center\" BGCOLOR=\"#C0E0C0\"><TD>in", 72ul);
               }
               else {
                  Appwww(&wsock, wbuf, "<TR class=tab-con-out style=\"text-al\
ign:center\" BGCOLOR=\"#D0C0C0\"><TD>out", 74ul);
               }
               Appwww(&wsock, wbuf, "</TD>", 6ul);
               if (serverlink[0U]==0) {
                  /* make klickable link */
                  Appwww(&wsock, wbuf, "<TD style=\"text-align:center\">",
                31ul);
                  Appwww(&wsock, wbuf, anonym2->ipnum, 64ul);
                  Appwww(&wsock, wbuf, "</TD>", 6ul);
               }
               else {
                  AppCall(wbuf, &wsock, anonym2->ipnum, 64ul, 0, 1,
                serverlink, 1024ul);
               }
               Appwww(&wsock, wbuf, "<TD style=\"text-align:right\">", 30ul);
               if (anonym2->service=='S') {
                  Appwww(&wsock, wbuf, tcpbindport, 6ul);
               }
               else Appwww(&wsock, wbuf, anonym2->port, 6ul);
               Appwww(&wsock, wbuf, "</TD><TD style=\"text-align:left\">",
                34ul);
               Appwww(&wsock, wbuf, anonym2->user.call, 10ul);
               if (anonym2->service=='G' && anonym2->qwatch.qsize>0L) {
                  Appwww(&wsock, wbuf, " q=", 4ul);
                  aprsstr_IntToStr(anonym2->qwatch.qsize, 1UL, h1, 256ul);
                  Appwww(&wsock, wbuf, h1, 256ul);
                  Appwww(&wsock, wbuf, "s", 2ul);
               }
               Appwww(&wsock, wbuf, "</TD><TD>", 10ul);
               if (anonym2->connt>0UL) {
                  if (anonym2->valid) Appwww(&wsock, wbuf, "v", 2ul);
                  if (aprspos_posvalid(anonym2->user.pos)) {
                     Appwww(&wsock, wbuf, "p", 2ul);
                  }
               }
               Appwww(&wsock, wbuf, "</TD><TD WIDTH=50>", 19ul);
               Appwww(&wsock, wbuf, anonym2->vers, 21ul);
               Appwww(&wsock, wbuf, "</TD><TD WIDTH=50>", 19ul);
               if (anonym2->service=='S') {
                  FiltToStr(anonym2->filters, h1, 256ul);
               }
               else aprsstr_Assign(h1, 256ul, anonym2->outfilterst, 256ul);
               Appwww(&wsock, wbuf, h1, 256ul);
               Appwww(&wsock, wbuf, "</TD>", 6ul);
               wcard64(wbuf, &wsock, anonym2->txbytesh, anonym2->txbytes);
               if (anonym2->losttxframes>0UL) {
                  wintint(wbuf, &wsock, (int32_t)anonym2->txframes,
                -(int32_t)anonym2->losttxframes);
               }
               else wint(wbuf, &wsock, (int32_t)anonym2->txframes);
               wint(wbuf, &wsock, (int32_t)bitsec64(anonym2->txbytesh,
                anonym2->txbytes, tt));
               wcard64(wbuf, &wsock, anonym2->rxbytesh, anonym2->rxbytes);
               if (anonym2->lostrxframes>0UL) {
                  wintint(wbuf, &wsock, (int32_t)anonym2->rxframes,
                -(int32_t)anonym2->lostrxframes);
               }
               else wint(wbuf, &wsock, (int32_t)anonym2->rxframes);
               wint(wbuf, &wsock, (int32_t)bitsec64(anonym2->rxbytesh,
                anonym2->rxbytes, tt));
               Appwww(&wsock, wbuf, "<TD>", 5ul);
               if (anonym2->connt>0UL) {
                  aprsstr_TimeToStr(tt, h1, 256ul);
                  Appwww(&wsock, wbuf, h1, 256ul);
               }
               Appwww(&wsock, wbuf, "</TD></TR>", 11ul);
            }
            ss = anonym2->next;
         }
      }
      Appwww(&wsock, wbuf, "</table></body></html>\015\012", 25ul);
   }
   else if (aprsstr_StrCmp(wsock->get, 256ul, "mh", 3ul)) {
      conthead(&wsock, wbuf, -1L);
      title(wbuf, &wsock, &mhhttpcount, 0, " MHeard", 8ul, "", 1ul);
      klicks(wbuf, &wsock);
      if (heardtimew>0UL) {
         showmh(wbuf, &wsock, h1, hearddir, 1, 0, heardtimew, "Heard Stations\
 Since Last ", 27ul, wsock->sortby, 2ul);
      }
      if (heardtimevia>0UL) {
         showmh(wbuf, &wsock, h1, heardvia, 0, 0, heardtimevia, "Via RF Heard\
 Stations Since Last ", 34ul, wsock->sortby, 2ul);
      }
      if (heardtimetcp>0UL) {
         showmh(wbuf, &wsock, h1, heardtcp, 0, 1, heardtimetcp, "Via TCP Conn\
ected Stations Since Last ", 39ul, wsock->sortby, 2ul);
      }
      Appwww(&wsock, wbuf, "</body></html>\015\012", 17ul);
   }
   else if (aprsstr_StrCmp(wsock->get, 256ul, "msg", 4ul)) {
      conthead(&wsock, wbuf, -1L);
      title(wbuf, &wsock, &msghttpcount, 0, " Message Relay", 15ul, viacall,
                10ul);
      klicks(wbuf, &wsock);
      for (tp = udpgate4_DIR; tp>=udpgate4_NET; tp--) {
         Appwww(&wsock, wbuf, "<table id=msg border=0 align=center CELLPADDIN\
G=3 CELLSPACING=1 BGCOLOR=\"#FFFFFF\">", 83ul);
         if (viacall[0U]==0) {
            if (tp==udpgate4_DIR) strncpy(h1,"Heard Messages (tx off)",256u);
            else if (tp==udpgate4_INDIR) {
               strncpy(h1,"Via Digi Heard Messages (tx off)",256u);
            }
            else strncpy(h1,"Via Net Heard Messages (tx off)",256u);
         }
         else if (tp==udpgate4_DIR) {
            strncpy(h1,"Heard Messages (Relayed to direct heard and Net)",
                256u);
         }
         else if (tp==udpgate4_INDIR) {
            strncpy(h1,"Via Digi Heard Messages (Relayed to direct heard)",
                256u);
         }
         else {
            strncpy(h1,"Via Net Heard Messages (Relayed to direct heard)",
                256u);
         }
         Appwww(&wsock, wbuf, "<tr class=tab-msg-titel BGCOLOR=\"#A9EEFF\"><t\
h colspan=13>", 58ul);
         Appwww(&wsock, wbuf, h1, 256ul);
         Appwww(&wsock, wbuf, "</th></tr>", 11ul);
         hok = 0;
         mp = messages;
         while (mp) {
            if (mp->src==tp) {
               if (!hok) {
                  Appwww(&wsock, wbuf, "<tr class=tab-msg-head BGCOLOR=\"#F0A\
070\"><th>From</th><th>To</th><th>hms ago</th><th>hms tx</th><th>po</th><th>r\
etr</th><th>m>r</th><th>m>n</th><th>a>r</th><th>a>n</th><th>A</th><th>Ack</th\
><th>Text</th></tr>", 209ul);
                  hok = 1;
               }
               Appwww(&wsock, wbuf, "<tr class=tab-msg-body BGCOLOR=\"#DDCC88\\
">", 42ul);
               AppCall(wbuf, &wsock, mp->from, 10ul, 0, 0, "", 1ul);
               AppCall(wbuf, &wsock, mp->to, 10ul, 0, 0, "", 1ul);
               AppTime(wbuf, &wsock, mp->gentime, 1);
               if (!mp->acked && !ismsgactiv(mp)) {
                  Appwww(&wsock, wbuf, "<td>timed out</td>", 19ul);
               }
               else if (mp->retryc>0UL && mp->txtime>0UL) {
                  AppTime(wbuf, &wsock, mp->txtime, 1);
               }
               else Appwww(&wsock, wbuf, "<td></td>", 10ul);
               AppInt(wbuf, &wsock, (int32_t)mp->txport);
               AppInt(wbuf, &wsock, (int32_t)mp->retryc);
               AppInt(wbuf, &wsock, (int32_t)mp->msg2rf);
               AppInt(wbuf, &wsock, (int32_t)mp->msg2net);
               AppInt(wbuf, &wsock, (int32_t)mp->ack2rf);
               AppInt(wbuf, &wsock, (int32_t)mp->ack2net);
               Appwww(&wsock, wbuf, "<td>", 5ul);
               if (mp->acked) {
                  if (mp->acksrc==udpgate4_DIR) strncpy(h1,"D",256u);
                  else if (mp->acksrc==udpgate4_INDIR) strncpy(h1,"V",256u);
                  else if (mp->acksrc==udpgate4_NET) strncpy(h1,"N",256u);
                  else strncpy(h1,"X",256u);
                  if (mp->reject) aprsstr_Append(h1, 256ul, "R", 2ul);
                  if (!mp->ackackt) {
                     h1[0U] = (char)((uint32_t)(uint8_t)h1[0U]+32UL);
                  }
                  Appwww(&wsock, wbuf, h1, 256ul);
               }
               Appwww(&wsock, wbuf, "</td>", 6ul);
               aprsstr_Assign(h1, 256ul, mp->ack, 5ul);
               aprsstr_Append(h1, 256ul, mp->replyack, 2ul);
               AppTxt(wbuf, &wsock, "<td>", 5ul, h1, 256ul);
               AppTxt(wbuf, &wsock, "<td>", 5ul, mp->text, 68ul);
               Appwww(&wsock, wbuf, "</tr>\015\012", 8ul);
            }
            mp = mp->next;
         }
         if (hok) {
            Appwww(&wsock, wbuf, "</table>&nbsp;", 15ul);
         }
      } /* end for */
      Appwww(&wsock, wbuf, "</body></html>\015\012", 17ul);
   }
   else if (aprsstr_InStr(wsock->get, 256ul, "raw", 4ul)==0L) {
      conthead(&wsock, wbuf, -1L);
      listraw(wbuf, &wsock, wsock->get, 256ul);
   }
   else if (wwwdir[0U] && openfile(wsock->get, 256ul, &fdw, &flen)) {
      conthead(&wsock, wbuf, flen);
      /*
          wbuf:='HTTP/1.0 200 OK'+CR+LF
      (*
          +'Content-Type: text/html;
                charset=iso-8859-1'+CR+LF+'Content-Length: ';
      *)
          +'Content-Length: ';
          IntToStr(flen, 1, h1);
          Append(wbuf, h1);
          Append(wbuf, CR+LF+CR+LF);
      */
      sendwww(&wsock, wbuf, (int32_t)aprsstr_Length(wbuf, 1401ul), 1);
      while (flen>0L) {
         res0 = osi_RdBin(fdw, (char *)wbuf, 1401u/1u, 1401UL);
         if (res0>0L) {
            sendwww(&wsock, wbuf, res0, 0);
            flen -= res0;
         }
         else flen = 0L;
      }
      osic_Close(fdw);
      wbuf[0U] = 0;
   }
   else strncpy(wbuf,"HTTP/1.1 404\015\012",1401u);
   if (wbuf[0U]) {
      sendwww(&wsock, wbuf, (int32_t)aprsstr_Length(wbuf, 1401ul), 0);
   }
} /* end Www() */


static void Wwwtx(pTCPSOCK * w)
{
   int32_t i0;
   int32_t len;
   struct TCPSOCK * anonym;
   int32_t tmp;
   { /* with */
      struct TCPSOCK * anonym = *w;
      if (anonym->txbuf) {
         if (anonym->txbuf->push) {
            len = sendsock(anonym->fd, anonym->txbuf->buf,
                anonym->txbuf->tlen);
         }
         else {
            len = sendmore(anonym->fd, anonym->txbuf->buf,
                anonym->txbuf->tlen);
         }
         if (len>0L) {
            tmp = anonym->txbuf->tlen-1L;
            i0 = len;
            if (i0<=tmp) for (;; i0++) {
               anonym->txbuf->buf[i0-len] = anonym->txbuf->buf[i0];
                /* delete the sent */
               if (i0==tmp) break;
            } /* end for */
            anonym->txbuf->tlen -= len;
            if (anonym->connt==0UL) anonym->connt = systime;
         }
         else anonym->txbuf->tlen = 0L;
         if (anonym->txbuf->tlen<=0L) closetcp(w, 0);
      }
   }
} /* end Wwwtx() */


static void saybusy(int32_t * fd, char s[], uint32_t s_len)
{
   int32_t res0;
   X2C_PCOPY((void **)&s,s_len);
   if (*fd>=0L) {
      res0 = sendsock(*fd, s, (int32_t)aprsstr_Length(s, s_len));
      osic_Close(*fd);
      *fd = -1L;
   }
   X2C_PFREE(s);
} /* end saybusy() */


static void KillDoubleUser(pTCPSOCK u)
{
   pTCPSOCK v;
   v = u->next;
   while (v) {
      if ((v->service=='S' && v->user.call[0U])
                && aprsstr_StrCmp(u->user.call, 10ul, v->user.call, 10ul)) {
         if (v->valid && !u->valid) {
            saybusy(&u->fd, "# unvalidated double login\015\012", 29ul);
         }
         else {
            saybusy(&v->fd, "# double login\015\012", 17ul);
                /* kill older or the unvalid */
         }
      }
      v = v->next;
   }
} /* end KillDoubleUser() */


static char tcpconn(pTCPSOCK * sockchain, int32_t f,
                char cservice)
{
   pTCPSOCK cp;
   FRAMEBUF h2;
   FRAMEBUF h1;
   FRAMEBUF h;
   int32_t res0;
   struct TCPSOCK * anonym;
   if (f<0L) return 0;
   osic_alloc((char * *) &cp, sizeof(struct TCPSOCK));
   if (cp==0) {
      osic_Close(f);
      if (verb) osi_WrStrLn("tcp conn out of memory", 23ul);
      return 0;
   }
   memset((char *)cp,(char)0,sizeof(struct TCPSOCK));
   { /* with */
      struct TCPSOCK * anonym = cp;
      anonym->fd = f;
      anonym->valid = cservice=='G';
      anonym->service = cservice;
      res0 = socknonblock(f);
      anonym->txbuf = 0;
   }
   cp->next = *sockchain;
   *sockchain = cp;
   if (cservice=='G') {
      if (servercall[0U]) {
         aprsstr_Assign(h, 512ul, "user ", 6ul);
         aprsstr_Append(h, 512ul, servercall, 10ul);
         if (passwd[0U]) {
            aprsstr_Append(h, 512ul, " pass ", 7ul);
            aprsstr_Append(h, 512ul, passwd, 6ul);
         }
         aprsstr_Append(h, 512ul, " vers ", 7ul);
         aprsstr_Append(h, 512ul, "udpgate 0.68", 13ul);
         if (actfilter[0U]) {
            aprsstr_Append(h, 512ul, " filter ", 9ul);
            aprsstr_Append(h, 512ul, actfilter, 256ul);
         }
         aprsstr_Append(h, 512ul, "\015\012", 3ul);
         Sendtcp(cp, h);
      }
   }
   else if (cservice=='S') {
      /*
      cp^.txbytes:=4294967200;
      cp^.txbytesh:=0;
      cp^.rxbytes:=4294965296;
      cp^.rxbytesh:=0;
      */
      res0 = 512L;
      res0 = getpeeripnum(f, h1, &res0);
      if (res0>=0L) {
         /*
                   tcp.ipnum2str(h1, h, SIZE(h));
         */
         ipnumport2str(h1, 512UL, h, 512UL, h2, 512UL);
         aprsstr_Assign(cp->ipnum, 64ul, h, 512ul);
         aprsstr_Assign(cp->port, 6ul, h2, 512ul);
         strncpy(h1,"T:connect to: ",512u);
         aprsstr_Append(h1, 512ul, h, 512ul);
         aprsstr_Append(h1, 512ul, " ", 2ul);
         aprsstr_Append(h1, 512ul, h2, 512ul);
         logline(1L, h1, 512ul);
      }
      aprsstr_Assign(h, 512ul, "# udpgate 0.68\015\012", 17ul);
      Sendtcp(cp, h);
   }
   return 1;
} /* end tcpconn() */


static void Gateconn(pTCPSOCK * cp)
{
   int32_t fd;
   pTCPSOCK kill;
   pTCPSOCK try0;
   pTCPSOCK act;
   pTCPSOCK p;
   FRAMEBUF h;
   char hh[21];
   uint32_t max0;
   struct _2 * anonym;
   act = 0;
   try0 = 0;
   p = *cp;
   while (p) {
      if (p->service=='G') {
         if (p->connt>0UL) {
            /* we have gate connect */
            if (act) {
               /* we have 2 connects */
               if (act->gatepri>p->gatepri) {
                  /* stop second best */
                  kill = act;
                  act = p;
               }
               else kill = p;
               saybusy(&kill->fd, "# remove double connect\015\012", 26ul);
            }
            else act = p;
         }
         else try0 = p;
      }
      p = p->next;
   }
   if (try0==0) {
      if (gatesfn[0U]) readurlsfile(gatesfn, 1024ul);
      if (act==0) max0 = 20UL;
      else max0 = act->gatepri;
      if ((trygate>=max0 || trygate>20UL) || gateways[trygate].url[0U]==0) {
         trygate = 0UL; /* loop thru table */
      }
      if (lastdnstime>systime) lastdnstime = systime;
      if ((trygate<max0 && Watchclock(&gateways[trygate].connecttime,
                gateconndelay)) && lastdnstime+gateways[trygate].resolvtime*5UL<=systime)
                 {
         { /* with */
            struct _2 * anonym = &gateways[trygate];
            if (anonym->url[0U]) {
               /*check dns time  */
               spintime();
               lastdnstime = systime;
               /*WrStrLn("start resolve"); */
               fd = connectto(anonym->url, anonym->port);
               spintime();
               anonym->resolvtime = systime-lastdnstime;
               if (anonym->resolvtime>5UL) {
                  strncpy(h,"T:slow DNS ",512u);
                  aprsstr_IntToStr((int32_t)anonym->resolvtime, 1UL, hh,
                21ul);
                  aprsstr_Append(h, 512ul, hh, 21ul);
                  aprsstr_Append(h, 512ul, "s", 2ul);
                  logline(1L, h, 512ul);
                  if (anonym->resolvtime>300UL) anonym->resolvtime = 120UL;
               }
               /*check dns time */
               if (fd>=0L) {
                  if (anonym->filterst[0U]) {
                     aprsstr_Assign(actfilter, 256ul, anonym->filterst,
                256ul);
                  }
                  else {
                     aprsstr_Assign(actfilter, 256ul, serverrangefilter,
                256ul);
                  }
                  if (tcpconn(cp, fd, 'G')) {
                     aprsstr_Assign((*cp)->ipnum, 64ul, anonym->url, 256ul);
                     aprsstr_Assign((*cp)->port, 6ul, anonym->port, 6ul);
                     aprsstr_Assign((*cp)->outfilterst, 256ul,
                anonym->filterst, 256ul);
                     (*cp)->gatepri = trygate;
                     strncpy(h,"T:connect to: ",512u);
                     aprsstr_Append(h, 512ul, anonym->url, 256ul);
                     aprsstr_Append(h, 512ul, " ", 2ul);
                     aprsstr_Append(h, 512ul, anonym->port, 6ul);
                     logline(1L, h, 512ul);
                  }
               }
            }
         }
      }
      ++trygate;
   }
} /* end Gateconn() */


static void WatchTXQ(pTCPSOCK sock)
{
   int32_t sent;
   int32_t j;
   int32_t i0;
   int32_t rb;
   int32_t acked;
   int32_t qb;
   char rest;
   int32_t tmp;
   qb = getunack(sock->fd); /* tcp unack sendqueue in byte */
   sent = (int32_t)(sock->txbytes-sock->qwatch.lasttb);
                /* last second sent bytes */
   if (qb<=0L || sent<0L) sock->qwatch.qsize = 0L;
   else {
      /* we have unack bytes */
      acked = (sock->qwatch.lastqb-qb)+sent; /* acked bytes in last s */
      j = 0L;
      rest = 0;
      tmp = sock->qwatch.qsize-1L;
      i0 = 0L;
      if (i0<=tmp) for (;; i0++) {
         rb = sock->qwatch.txbyte[i0]-acked;
         sock->qwatch.txbyte[j] = rb;
         if (rb>0L) rest = 1;
         else acked = -rb;
         if (rest) {
            acked = 0L;
            ++j;
         }
         if (i0==tmp) break;
      } /* end for */
      sock->qwatch.qsize = j;
      if (sock->qwatch.qsize<=59L) {
         sock->qwatch.txbyte[sock->qwatch.qsize] = sent;
         ++sock->qwatch.qsize;
         if (sock->qwatch.qsize>qmaxtime) {
            stoptxrx(sock->fd, 2L); /* delete unsent data */
            saybusy(&sock->fd, "\015\012", 3ul); /* close tcp immediately */
            sock->slowlink = 1; /* for log message */
         }
      }
   }
   sock->qwatch.lastqb = qb;
   sock->qwatch.lasttb = sock->txbytes;
} /* end WatchTXQ() */

static int32_t listensock;

static int32_t wwwsock;

static FRAMEBUF mbuf;

static int32_t res;

static int32_t i;

static pTCPSOCK acttcp;

static pTCPSOCK acttmp;

static pUDPSOCK actudp;

static uint32_t usercnt;

static uint32_t lenh;

static struct POSCALL poscall;

static char voidungate;

static char rfhered;

static uint32_t keepconn;

static uint32_t lasttime;


static void addsock(int32_t fd, char wtoo)
/* insert socket in fdset for select */
{
   if (fd>=0L) {
      fdsetr((uint32_t)fd);
      if (wtoo) fdsetw((uint32_t)fd);
   }
} /* end addsock() */

static CHSET _cnst1 = {0x30000000UL,0x20008092UL,0x80000001UL,0x00000001UL};

X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(MONCALL)!=10) X2C_ASSERT(0);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(FRAMEBUF)!=512) X2C_ASSERT(0);
   if (sizeof(FILTERST)!=256) X2C_ASSERT(0);
   if (sizeof(WWWB)!=1401) X2C_ASSERT(0);
   if (sizeof(MSGTEXT)!=68) X2C_ASSERT(0);
   if (sizeof(ACKTEXT)!=5) X2C_ASSERT(0);
   if (sizeof(REPLYACK)!=2) X2C_ASSERT(0);
   aprsstr_BEGIN();
   aprspos_BEGIN();
   osi_BEGIN();
   strncpy(rfdestcall,"RFONLY",10u);
   strncpy(netdestcall,"NOGATE",10u);
   viacall[0U] = 0;
   nettorfpath[0U] = 0;
   /*  heardtime:=7200; */
   heardtimew = 86400UL;
   heardtimevia = 1800UL;
   heardtimetcp = 86400UL;
   purgemsg = 86400UL;
   purgeacked = 86400UL;
   purgeunack = 600UL;
   purgeunacksent = 30UL;
   gateconndelay = 30UL;
   msgsendtime = 21600UL;
   rfquiet = 10UL;
   maxatonce = 2UL;
   maxmsg = 1000UL;
   messages = 0;
   hearddir = 0;
   heardvia = 0;
   heardtcp = 0;
   sendnetmsg = 1;
   mhfilelines = 0UL;
   memset((char *)gateways,(char)0,sizeof(struct _2 [21]));
   realtime = 0UL;
   systime = 946728000UL; /* start of systime about 1.1.2000 */
   qmaxtime = 15L;
   ungates[udpgate4_gUNGATE][0] = 0;
   strncpy(ungates[udpgate4_gRFONLY],"RFONLY*",11u);
   strncpy(ungates[udpgate4_gNOGATE],"NOGATE*",11u);
   strncpy(ungates[udpgate4_gTCPIP],"TCPIP*",11u);
   strncpy(ungates[udpgate4_gTCPXX],"TCPXX*",11u);
   strncpy(ungates[udpgate4_gQ],"q*",11u);
   dupetime = 60UL;
   lastdnstime = 0UL;
   netbeaconfn[0U] = 0;
   maxusers = 50UL;
   logframename[0U] = 0;
   rawlogname[0U] = 0;
   logframes = 6L;
   tcpbindport[0U] = 0;
   servercall[0U] = 0;
   passwd[0U] = 0;
   serverrangefilter[0U] = 0;
   actfilter[0U] = 0;
   rfhered = 0;
   wwwbindport[0] = 0;
   wwwdir[0U] = 0;
   wwwsizelimit = 1048576L;
   udpsocks = 0;
   callsrc = 0;
   mhperport = 0;
   qas = 0UL;
   qasc = 0UL;
   maxpongtime = 30UL;
   gatesfn[0U] = 0;
   rawlines = 20UL;
   msgretries = 12UL;
   parms();
   if (aprsstr_StrCmp(viacall, 10ul, "-", 2ul)) viacall[0U] = 0;
   else if (viacall[0U]==0) memcpy(viacall,servercall,10u);
   msghttpcount = 0UL;
   mhhttpcount = 0UL;
   netmhout = 0UL;
   netmhin = 0UL;
   aprsstr_Assign(ungates[udpgate4_gUNGATE], 11ul, servercall, 10ul);
   uptime = systime;
   udpdonetime = systime;
   keepconn = 0UL;
   trygate = 0UL;
   httpcount = 0UL;
   Netbeacon(mbuf, 512ul, 0, 1);
   listensock = -1L;
   wwwsock = -1L;
   tcpsocks = 0;
   for (;;) {
      spintime();
      if (systime!=lasttime) {
         if (listensock<0L && tcpbindport[0U]) {
            /* open listensocket tcp connects */
            listensock = waitconnect(tcpbindport, 4UL);
            if (verb && listensock<0L) {
               osi_WrStr("cant bind to port ", 19ul);
               osi_WrStrLn(tcpbindport, 6ul);
            }
         }
         if (wwwsock<0L && wwwbindport[0U]) {
            /* open listensocket www connects */
            wwwsock = waitconnect(wwwbindport, 16UL);
            /*        IF (wwwsock>=0) & (tcp.sockreuse(wwwsock)<0)
                & verb THEN WrStr("cant set sockopts to port ");
                WrStrLn(wwwbindport) END;   */
            if (verb && wwwsock<0L) {
               osi_WrStr("cant bind to port ", 19ul);
               osi_WrStrLn(wwwbindport, 6ul);
            }
         }
      }
      fdclr();
      actudp = udpsocks;
      while (actudp) {
         addsock(actudp->fd, 0);
         actudp = actudp->next;
      }
      addsock(listensock, 0);
      addsock(wwwsock, 0);
      if (systime!=lasttime) {
         /* once a second is enough */
         RfTimer();
         SendNet();
         if (mhfilelines>0UL) MHtoFile();
         rfhered = keeptime==0UL || keepconn>systime;
         acttcp = tcpsocks;
         while (acttcp) {
            { /* with */
               struct TCPSOCK * anonym = acttcp;
               if (anonym->beacont<systime) Timebeacon(acttcp);
               if (anonym->service=='G') {
                  if (keeptime>0UL && keepconn<=systime) {
                     saybusy(&anonym->fd, "# timeout\015\012", 12ul);
                  }
                  else if (qmaxtime>0L) WatchTXQ(acttcp);
               }
               else if (anonym->service=='W') {
                  if (anonym->connt==0UL) anonym->connt = systime;
                  else if (anonym->connt+60UL<systime) {
                     saybusy(&anonym->fd, "timeout\015\012", 10ul);
                  }
               }
               else if (anonym->service=='S') {
                  KillDoubleUser(acttcp);
                  keepconn = systime+keeptime; /* connect to gateway */
               }
               acttcp = anonym->next;
            }
         }
      }
      if (systime!=lasttime) {
         if (rfhered) Gateconn(&tcpsocks);
         else trygate = 0UL;
      }
      lasttime = systime;
      usercnt = 0UL;
      acttcp = tcpsocks;
      while (acttcp) {
         addsock(acttcp->fd, acttcp->tlen>0L || acttcp->txbuf);
         ++usercnt;
         acttcp = acttcp->next;
      }
      mbuf[0U] = 0;
      res = selectrw(2UL, 0UL);
      if (listensock>=0L && issetr((uint32_t)listensock)) {
         res = 512L;
         res = acceptconnect(listensock, mbuf, &res);
         if (res>=0L) {
            if (usercnt>=maxusers || !tcpconn(&tcpsocks, res, 'S')) {
               saybusy(&res, "# server full\015\012", 16ul);
            }
         }
      }
      if (wwwsock>=0L && issetr((uint32_t)wwwsock)) {
         res = 512L;
         res = acceptconnect(wwwsock, mbuf, &res);
         if (res>=0L) {
            if (usercnt>=maxusers+1UL || !tcpconn(&tcpsocks, res, 'W')) {
               saybusy(&res, "server full\015\012", 14ul);
            }
         }
      }
      spintime();
      actudp = udpsocks;
      i = 1L;
      while (actudp) {
         if (issetr((uint32_t)actudp->fd)) {
            while (getudp(actudp, mbuf, actudp->ghosts)) {
               if (systime<udpdonetime+5UL) {
                  /* last time all data read is no too long */
                  res = AprsIs(mbuf, 512ul, 0x3FU, 0x3FU, "", 1ul,
                (uint32_t)i, 0, &poscall);
                  if (res>=0L) {
                     Sendall(mbuf, 0L, poscall);
                     keepconn = systime+keeptime; /* connect to gateway */
                  }
               }
               else {
                  /*task was blocked too long */
                  if (verb) {
                     osi_WrStrLn("AXUDP-frames thrown away - too long program\
 delay", 50ul);
                  }
                  logline(1L, "Frame discarded - too long program delay",
                41ul);
               }
               if (verb || logframes>1L) {
                  showframe(res, 0, actudp, mbuf, 512ul, poscall.pos);
               }
            }
         }
         ++i;
         actudp = actudp->next;
      }
      spintime();
      udpdonetime = systime; /* store fresh time of last all read input */
      acttcp = tcpsocks;
      while (acttcp) {
         if (acttcp->fd>=0L && issetw((uint32_t)acttcp->fd)) {
            { /* with */
               struct TCPSOCK * anonym0 = acttcp;
               if (anonym0->service=='W') Wwwtx(&acttcp);
               else {
                  sendtcpbuf(acttcp);
                  if (anonym0->connt==0UL) anonym0->connt = systime;
               }
            }
         }
         if (acttcp->fd>=0L && issetr((uint32_t)acttcp->fd)) {
            if (acttcp->service=='W') Www(acttcp);
            else {
               for (;;) {
                  res = Gettcp(acttcp->fd, mbuf, acttcp->rbuf,
                &acttcp->rpos);
                  if (res<0L) {
                     osic_Close(acttcp->fd);
                     acttcp->fd = -1L;
                     break;
                  }
                  if (res<=0L) break;
                  ++acttcp->rxframes;
                  lenh = aprsstr_Length(mbuf, 512ul);
                  acttcp->rxbytes += lenh;
                  if (acttcp->rxbytes<lenh) ++acttcp->rxbytesh;
                  if (acttcp->pongtime>0UL && acttcp->service=='S') {
                     /* pingpong active */
                     if (acttcp->pongtime<systime) {
                        /* pinpong enabled - stop transfer */
                        saypongout(acttcp);
                        acttcp->pingout = 1;
                        ++acttcp->lostrxframes;
                     }
                  }
                  if ((acttcp->valid || acttcp->service=='G') || !Auth(mbuf,
                512ul, acttcp)) {
                     if (mbuf[0U]=='#') {
                        GetFilters(&acttcp->filters, mbuf, 512ul, 1UL,
                acttcp);
                        res = -7L;
                     }
                     else {
                        res = AprsIs(mbuf, 512ul, 0x3U, 0x17U,
                acttcp->user.call, 10ul, 0UL, acttcp->valid, &poscall);
                     }
                     if (((res>=-1L && aprspos_posvalid(poscall.pos))
                && aprsstr_StrCmp(poscall.call, 10ul, acttcp->user.call,
                10ul)) && X2C_INL((int32_t)(uint8_t)poscall.typ0,128,
                _cnst1)) {
                        acttcp->user.pos = poscall.pos;
                        if (heardtimetcp>48UL) {
                           AddHeard(&heardtcp, heardtimetcp, poscall.call,
                0UL, mbuf, 512ul, &voidungate, 0);
                /* store pos in mh to remember pos at next connect */
                        }
                     }
                     if (acttcp->valid && !acttcp->pingout) {
                        if (res>=0L) Sendall(mbuf, acttcp->fd, poscall);
                        if (verb || logframes>1L) {
                           showframe(res, acttcp, 0, mbuf, 512ul,
                poscall.pos);
                        }
                     }
                  }
               }
            }
         }
         if (acttcp->fd<0L) {
            /* connection lost */
            if (acttcp->connt && acttcp->service!='W') showlogout(acttcp);
            closetcp(&acttcp, 1); /* free www buffers */
            if (acttcp==tcpsocks) tcpsocks = tcpsocks->next;
            else {
               acttmp = tcpsocks;
               while (acttmp->next!=acttcp) acttmp = acttmp->next;
               acttmp->next = acttcp->next;
            }
            osic_free((char * *) &acttcp, sizeof(struct TCPSOCK));
            acttcp = tcpsocks;
         }
         else acttcp = acttcp->next;
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
