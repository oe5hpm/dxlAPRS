/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#define aprsdecode_C_
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef xosi_H_
#include "xosi.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef useri_H_
#include "useri.h"
#endif
#ifndef aprstext_H_
#include "aprstext.h"
#endif

aprsdecode_SET256 aprsdecode_SYMTABLE = {0x00000000UL,0x03FF8000UL,
                0x17FFFFFEUL,0x07FFFFFEUL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL};








































uint32_t aprsdecode_systime;
uint32_t aprsdecode_realtime;
uint32_t aprsdecode_lastlooped;
uint32_t aprsdecode_rxidle;
char aprsdecode_quit;
char aprsdecode_verb;
char aprsdecode_logdone;


struct aprsdecode__D0 aprsdecode_click;
struct aprsstr_POSITION aprsdecode_mappos;
int32_t aprsdecode_inittilex;
int32_t aprsdecode_inittiley;
int32_t aprsdecode_initxsize;
int32_t aprsdecode_initysize;
int32_t aprsdecode_parmzoom;
int32_t aprsdecode_initzoom;
float aprsdecode_finezoom;
float aprsdecode_parmfinezoom;
char aprsdecode_mapdir[1025];
aprsdecode_pMOUNTAIN aprsdecode_mountains;


struct aprsdecode__D1 aprsdecode_poifiles[30];


struct aprsdecode__D2 aprsdecode_lums;


struct aprsdecode__D3 aprsdecode_tracenew;
aprsdecode_pTXMESSAGE aprsdecode_txmessages;
aprsdecode_pMSGFIFO aprsdecode_msgfifo0;
float aprsdecode_spikesens;
float aprsdecode_maxhop;
aprsdecode_pOPHIST aprsdecode_ophist2;
aprsdecode_pOPHIST aprsdecode_ophist0;


struct aprsdecode__D4 aprsdecode_gateways[10];
aprsdecode_pTCPSOCK aprsdecode_tcpsocks;
struct aprsdecode_UDPSOCK aprsdecode_udpsocks0[4];
uint32_t aprsdecode_lasttcptx;
uint32_t aprsdecode_lastanyudprx;
uint32_t aprsdecode_lastpurge;
uint32_t aprsdecode_lasttcprx;
aprsdecode_FRAMEBUF aprsdecode_testbeaconbuf;
struct xosi_PROCESSHANDLE aprsdecode_checkversion;
struct xosi_PROCESSHANDLE aprsdecode_serialpid;
struct xosi_PROCESSHANDLE aprsdecode_serialpid2;
struct xosi_PROCESSHANDLE aprsdecode_maploadpid;
/* connect to aprs-is gateway and decode data for archive and map by OE5DXL */
#define aprsdecode_CALLLEN 7

#define aprsdecode_HASHSIZE 65536

#define aprsdecode_GATECONNDELAY 30

#define aprsdecode_WATCH 180

#define aprsdecode_QMAXTIME 15

#define aprsdecode_MIDMOD 62

#define aprsdecode_HOLDT 0x0FFFFFFFF

#define aprsdecode_BEEPINVERVALL 5
/* min time between beeps */

#define aprsdecode_TICKEROFFTIME 60
/* switch off window headline ticker after no data*/

#define aprsdecode_COMPRESSLIMIT 20
/* different beacons then switch off compression in log read */

#define aprsdecode_cUSERMESSAGE ":"

#define aprsdecode_cTHIRDPARTY "}"

#define aprsdecode_DUPHASHSIZE 65536

#define aprsdecode_PINGTIMEOUT 10
/* ping-pong ping timeout */

#define aprsdecode_PINGINTERVALL 20
/* ping-pong ping intervall */

#define aprsdecode_DIRIN ">"

#define aprsdecode_DIROUT "<"

#define aprsdecode_DIRHAVIT "-"

#define aprsdecode_DIRJUNC "#"

#define aprsdecode_cMSGACK "{"

typedef uint32_t CHSET[4];


struct BEACON {
   uint32_t bintervall;
   aprsdecode_FILENAME bfile;
   uint32_t btime;
};

typedef char TICKERCALL[31];

struct UDPSET;


struct UDPSET {
   int32_t txd;
   int32_t level;
   int32_t quali;
};

static char sentemptymsg;

static uint32_t uptime;

static aprsdecode_FILENAME logframename;
/*    netbeaconintervall,  */

static uint32_t beaconrandomstart;

static uint32_t nextbeep;

static uint32_t rfbecondone;

static uint32_t lastmsgtx;

static uint32_t qwatchtime;

static uint32_t connecttime;

static uint32_t cycleservers;

static uint32_t qas;

static uint32_t qasc;

static int32_t logframes;
/*    passwd                : ARRAY[0..5] OF CHAR; */

static uint32_t trygate;
/*    actudp                : pUDPSOCK; */

static aprsdecode_MONCALL ungate[7];

static uint32_t timehash[65536];

static uint32_t rfbeacont;

static uint32_t dupetime;

static uint32_t msgmid;

static TICKERCALL frameticker[20];

/* calls in ticker headline */
static uint32_t tickertime;

static float maxspeed;

static uint16_t crctable12[256];


static void Err(const char text[], uint32_t text_len)
{
   osi_WrStr("aprsmap: ", 10ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */


extern uint32_t aprsdecode_trunc(float r)
{
   if (r<=0.0f) return 0UL;
   else if (r>=2.E+9f) return 2000000000UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end trunc() */


extern float aprsdecode_floor(float r)
{
   float f;
   f = (float)osi_realint(r);
   if (f>r) f = f-1.0f;
   return f;
} /* end floor() */


static float sqr(float x)
{
   return x*x;
} /* end sqr() */


static char Watchclock(uint32_t * t, uint32_t intervall)
{
   uint32_t tn;
   if (intervall>0UL || *t==0UL) {
      /* send once */
      tn = aprsdecode_realtime;
      if (*t<=tn) {
         *t += intervall;
         if (*t<=tn) *t = tn+intervall;
         return 1;
      }
      if (*t>tn+intervall) *t = tn+intervall;
   }
   return 0;
} /* end Watchclock() */


extern void aprsdecode_beeplim(int32_t lev, uint32_t hz, uint32_t ms)
/* beep with minimum quiet time */
{
   if (aprsdecode_realtime>=nextbeep) {
      xosi_beep(lev, hz, ms);
      nextbeep = aprsdecode_realtime+5UL;
   }
} /* end beeplim() */


static void filepath(char s[], uint32_t s_len)
/* get path out of path + filename */
{
   uint32_t j;
   uint32_t i;
   i = 0UL;
   j = 0UL;
   while (i<s_len-1 && s[i]) {
      if (s[i]=='/' || s[i]=='/') j = i;
      ++i;
   }
   s[j] = 0;
} /* end filepath() */


static void alfanum(char s[], uint32_t s_len)
/* clean received call to filename */
{
   uint32_t j;
   uint32_t i;
   char c;
   i = 0UL;
   j = 0UL;
   while (i<s_len-1 && s[i]) {
      c = X2C_CAP(s[i]);
      if ((((((((uint8_t)c>='A' && (uint8_t)c<='Z' || (uint8_t)
                c>='0' && (uint8_t)c<='9') || c=='-') || c=='+') || c=='_')
                 || c=='(') || c==')') || c=='=') {
         s[j] = c;
         ++j;
      }
      ++i;
   }
   if (j<=s_len-1) s[j] = 0;
} /* end alfanum() */


static int32_t GetNum(char h[], uint32_t h_len, uint32_t * p,
                uint32_t * n)
{
   char ok0;
   int32_t GetNum_ret;
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
      GetNum_ret = -1L;
      goto label;
   }
   GetNum_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetNum_ret;
} /* end GetNum() */


static void parms(void)
{
   char h[4096];
   char err;
   char lasth;
   uint32_t n;
   uint32_t i;
   uint32_t gatecnt;
   int32_t ii;
   err = 0;
   aprsdecode_verb = 0;
   gatecnt = 0UL;
   for (;;) {
      osi_NextArg(h, 4096ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         lasth = h[1U];
         if (lasth=='x') {
            osi_NextArg(h, 4096ul);
            i = 0UL;
            if (GetNum(h, 4096ul, &i, &n)>=0L && n>=30UL) {
               aprsdecode_initxsize = (int32_t)n;
            }
            else Err("-x xsize", 9ul);
         }
         else if (lasth=='y') {
            osi_NextArg(h, 4096ul);
            i = 0UL;
            if (GetNum(h, 4096ul, &i, &n)>=0L && n>=20UL) {
               aprsdecode_initysize = (int32_t)n;
            }
            else Err("-y xsize", 9ul);
         }
         else if (lasth=='z') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToFix(&aprsdecode_parmfinezoom, h,
                4096ul) || aprsdecode_parmfinezoom<=1.0f) {
               Err("-z zoomlevel", 13ul);
            }
            n = aprsdecode_trunc(aprsdecode_parmfinezoom);
            aprsdecode_parmfinezoom = (1.0f+aprsdecode_parmfinezoom)
                -(float)n;
            aprsdecode_parmzoom = (int32_t)n;
         }
         else if (lasth=='m') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToFix(&aprsdecode_mappos.long0, h, 4096ul)) {
               Err("-m long lat", 12ul);
            }
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToFix(&aprsdecode_mappos.lat, h, 4096ul)) {
               Err("-m long lat", 12ul);
            }
            aprsdecode_mappos.long0 = aprspos_rad0(aprsdecode_mappos.long0);
            aprsdecode_mappos.lat = aprspos_rad0(aprsdecode_mappos.lat);
         }
         else if (lasth=='c') osi_NextArg(aprsdecode_lums.configfn, 257ul);
         else if (lasth=='g') {
            osi_NextArg(h, 4096ul);
            if (gatecnt>9UL) Err("-g gateway table full", 22ul);
            h[4095U] = 0;
            if (h[0U]=='[') {
               ii = 1L;
               while (h[ii] && h[ii]!=']') ++ii;
               if (h[ii]!=']' || h[ii+1L]!=':') Err("-g [url]:port", 14ul);
               h[ii] = 0;
               i = 1UL;
               while (i<=4095UL) {
                  h[i-1UL] = h[i];
                  ++i;
               }
            }
            else ii = aprsstr_InStr(h, 4096ul, ":", 2ul);
            if (ii>=0L) h[ii] = 0;
            aprsstr_Assign(aprsdecode_gateways[gatecnt].url, 256ul, h,
                4096ul);
            if (ii>0L) {
               /* port number */
               ++ii;
               i = 0UL;
               while (ii<=4095L) {
                  h[i] = h[ii];
                  ++i;
                  ++ii;
               }
            }
            else osi_NextArg(h, 4096ul);
            h[4095U] = 0;
            ii = aprsstr_InStr(h, 4096ul, "#", 2ul);
            if (ii>=0L) h[ii] = 0;
            if (h[0U]==0) Err("-g url:port", 12ul);
            aprsstr_Assign(aprsdecode_gateways[gatecnt].port, 6ul, h,
                4096ul);
            if (ii>0L) {
               /* we have a filter string */
               ++ii;
               i = 0UL;
               while (ii<=4095L) {
                  if (h[ii]==',') h[ii] = ' ';
                  h[i] = h[ii];
                  ++i;
                  ++ii;
               }
               aprsstr_Assign(aprsdecode_gateways[gatecnt].filterst, 256ul,
                h, 4096ul);
            }
            ++gatecnt;
         }
         else {
            if (lasth=='h') {
               osi_WrStrLn(" -c <configfilename>", 21ul);
               osi_WrStrLn(" -g <url>:<port>#<filters> connect to APRS-IS gat\
eway, repeat -g to other", 74ul);
               osi_WrStrLn("                gateways write favorites first fo\
r 1 activ link to the best", 76ul);
               osi_WrStrLn("                reachable and a polling link to r\
evert immediately to preferred again", 86ul);
               osi_WrStrLn("                -g www.db0anf.de:14580#m/300 -g 1\
27.0.0.1:3000 -g [::1]:3000", 77ul);
               osi_WrStrLn("                <filters> text sent to out connec\
ted server -f m/50", 68ul);
               osi_WrStrLn("                if blanks dont pass parameter set\
tings use , (-f m/30,-d/CW)", 77ul);
               osi_WrStrLn("                ipv6 if enabled by kernel",
                42ul);
               osi_WrStrLn(" -h             this", 21ul);
               osi_WrStrLn(" -M <ip>:<dport>/<lport>[+<byte/s>[:<radius>]]",
                47ul);
               osi_WrStrLn("                udp rf port (monitor frame format\
) for local (t)rx", 67ul);
               osi_WrStrLn("                <dport>:<lport> read from any ip,\
 dport=0 no tx", 64ul);
               osi_WrStrLn("                +byte/s enable inet to rf for ser\
vices like WLNK, WHO-IS", 73ul);
               osi_WrStrLn("                :radius enable all inet to rf gat\
e (from km around digi)", 73ul);
               osi_WrStrLn("                repeat -M for each radio port wit\
h a tx", 56ul);
               osi_WrStrLn(" -o <outfile>   out file, delete after read",
                44ul);
               osi_WrStrLn(" -R             same as -M but axudp format",
                44ul);
               osi_WrStrLn(" -v             show frames and analytics on stdo\
ut", 52ul);
               osic_WrLn();
               X2C_ABORT();
            }
            if (lasth=='C') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=100UL) {
                  aprsdecode_lums.centering = (int32_t)n;
               }
               else Err("-C center%", 11ul);
            }
            else if (lasth=='X') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.sym = (int32_t)n;
               }
               else Err("-X symbollumen", 15ul);
            }
            else if (lasth=='O') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.obj = (int32_t)n;
               }
               else Err("-O objectlumen", 15ul);
            }
            else if (lasth=='W') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.waypoint = (int32_t)n;
               }
               else Err("-W waypointlumen", 17ul);
            }
            else if (lasth=='A') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.text = (int32_t)n;
               }
               else Err("-A textlumen", 13ul);
            }
            else if (lasth=='T') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.track = (int32_t)n;
               }
               else Err("-T tracklumen", 14ul);
            }
            else if (lasth=='F') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n>0UL) {
                  aprsdecode_lums.firstdim = n*60UL;
               }
               else Err("-F fadetime (min)", 18ul);
            }
            else if (lasth=='V') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n>0UL) {
                  aprsdecode_lums.fps = (int32_t)n;
               }
               else Err("-V fps", 7ul);
            }
            else if (lasth=='B') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n>0UL) {
                  aprsdecode_lums.maxdim = n*60UL;
               }
               else Err("-B blanktime (min)", 19ul);
            }
            else if (lasth=='P') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n>0UL) {
                  aprsdecode_lums.purgetime = n*60UL;
               }
               else Err("-P purgetime (min)", 19ul);
            }
            else if (lasth=='r') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.rf = (int32_t)n;
               }
               else Err("-r rftracklumen", 16ul);
            }
            else if (lasth=='o') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (GetNum(h, 4096ul, &i, &n)>=0L && n<=1024UL) {
                  aprsdecode_lums.map = (int32_t)n;
               }
               else Err("-o maplumen", 12ul);
            }
            else if (lasth=='v') aprsdecode_verb = 1;
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
} /* end parms() */


static void getval(const char s[], uint32_t s_len, uint32_t * i,
                int32_t * v)
{
   char m;
   ++*i;
   *v = 0L;
   m = s[*i]=='-';
   if (m) ++*i;
   while ((*i<s_len-1 && (uint8_t)s[*i]>='0') && (uint8_t)s[*i]<='9') {
      *v =  *v*10L+(int32_t)((uint32_t)(uint8_t)s[*i]-48UL);
      ++*i;
   }
   while (*i<s_len-1 && (uint8_t)s[*i]>' ') ++*i;
   while (*i<s_len-1 && s[*i]==' ') ++*i;
   if (m) *v = -*v;
} /* end getval() */


static void decodeudp2(const char ub[], uint32_t ub_len,
                struct UDPSET * modeminfo)
{
   uint32_t i;
   int32_t res;
   if (ub[0UL] && ub[1UL]) {
      i = 2UL;
      while (i<ub_len-1 && ub[i]) {
         switch ((unsigned)ub[i]) {
         case 'T':
            getval(ub, ub_len, &i, &res);
            modeminfo->txd = res;
            break;
         case 'V':
            getval(ub, ub_len, &i, &res);
            modeminfo->level = res;
            break;
         case 'Q':
            getval(ub, ub_len, &i, &res);
            modeminfo->quali = res;
            break;
         default:;
            getval(ub, ub_len, &i, &res);
            break;
         } /* end switch */
      }
   }
} /* end decodeudp2() */


static char getudp(uint32_t usock, aprsdecode_FRAMEBUF buf,
                struct UDPSET * modeminfo, aprsstr_GHOSTSET ghostset)
{
   uint32_t fromport;
   uint32_t ipn;
   char crc2;
   char crc1;
   int32_t len;
   uint32_t mlen;
   aprsdecode_FRAMEBUF mbuf;
   char udp2[100];
   aprsstr_GHOSTSET tmp;
   ghostset = (uint32_t *)memcpy(tmp,ghostset,36u);
   if ((int32_t)aprsdecode_udpsocks0[usock].fd<0L) return 0;
   len = udpreceive(aprsdecode_udpsocks0[usock].fd, buf, 512L, &fromport,
                &ipn);
   if ((len>2L && len<512L) && (!aprsdecode_udpsocks0[usock].checkip || aprsdecode_udpsocks0[usock].ip==ipn)
                ) {
      /*WrInt(udpsend(usock^.fd, buf, len, usock^.dport, usock^.ip), 1);
                WrStrLn(" us"); */
      memset((char *)modeminfo,(char)0,sizeof(struct UDPSET));
      buf[len] = 0;
      if (aprsdecode_udpsocks0[usock].rawread) {
         len -= 2L;
         crc1 = buf[len];
         crc2 = buf[len+1L];
         aprsstr_AppCRC(buf, 512ul, len);
         if (crc1!=buf[len] || crc2!=buf[len+1L]) {
            if (aprsdecode_verb) osi_WrStrLn(" axudp crc error", 17ul);
            buf[0UL] = 0;
         }
         else {
            if (buf[0UL]=='\001') {
               aprsstr_extrudp2(buf, 512ul, udp2, 100ul, &len);
               decodeudp2(udp2, 100ul, modeminfo);
            }
            /*WrInt(modeminfo.txd, 5); WrInt(modeminfo.level, 5);
                WrInt(modeminfo.quali, 5); WrStrLn(" modem"); */
            aprsstr_raw2mon(buf, 512ul, mbuf, 512ul, (uint32_t)len, &mlen,
                ghostset);
            memcpy(buf,mbuf,512u);
         }
      }
   }
   else return 0;
   return 1;
} /* end getudp() */


static int32_t Gettcp(int32_t fd, aprsdecode_FRAMEBUF line,
                aprsdecode_FRAMEBUF buf, int32_t * pos)
{
   int32_t j;
   int32_t i;
   int32_t len;
   int32_t tmp;
   i = *pos;
   if (i>=512L) i = 0L;
   len = readsock(fd, line, 512L-i);
   if (len<0L) return -1L;
   if (*pos>=512L) {
      /* hunt mode */
      i = 0L;
      while (i<len && line[i]!='\012') ++i;
      if (i>=len) return 0L;
      /* no line start found */
      *pos = 0L;
      ++i;
      j = 0L;
      while (i<len) {
         line[j] = line[i];
         ++j;
         ++i;
      }
      len = j;
   }
   if (*pos>0L) {
      for (i = len-1L; i>=0L; i--) {
         line[i+*pos] = line[i]; /* move up new */
      } /* end for */
      tmp = *pos-1L;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         line[i] = buf[i]; /* ins buf before */
         if (i==tmp) break;
      } /* end for */
      len += *pos;
      *pos = 0L;
   }
   i = 0L;
   while (i<len && line[i]!='\012') ++i;
   if (i>=len) {
      /* no line end found */
      while (*pos<len) {
         buf[*pos] = line[*pos]; /* move to buf */
         ++*pos;
      }
      len = 0L;
   }
   else {
      j = i+1L;
      while (j<len) {
         buf[*pos] = line[j]; /* move rest to buf */
         ++*pos;
         ++j;
      }
      len = i;
   }
   line[len] = 0;
   return len;
} /* end Gettcp() */


static void saybusy(int32_t * fd, char s[], uint32_t s_len)
/* send good by text and close tcp */
{
   int32_t res;
   X2C_PCOPY((void **)&s,s_len);
   if ((int32_t)*fd>=0L) {
      res = sendsock(*fd, s, (int32_t)aprsstr_Length(s, s_len));
      osic_CloseSock(*fd);
      *fd = -1L;
   }
   X2C_PFREE(s);
} /* end saybusy() */


static void saylinkbad(int32_t s)
{
   char hh[101];
   char h[101];
   strncpy(h,"AprsIs uplink delays ",101u);
   aprsstr_IntToStr(s, 1UL, hh, 101ul);
   aprsstr_Append(h, 101ul, hh, 101ul);
   aprsstr_Append(h, 101ul, "s", 2ul);
   useri_textautosize(0L, 5L, 4UL, 10UL, 'r', h, 101ul);
} /* end saylinkbad() */


static void WatchTXQ(aprsdecode_pTCPSOCK sock)
{
   int32_t sent;
   int32_t j;
   int32_t i;
   int32_t rb;
   int32_t acked;
   int32_t qb;
   char rest;
   int32_t tmp;
   qb = getunack(sock->fd); /* tcp unack sendqueue in byte */
   if (sock->lastping==0UL && qb==-1L) {
      /* windoof */
      sock->lastping = 1UL; /* switch on ping-pong mode */
      return;
   }
   /*
       configbool(fALLOWGATE, FALSE);
       textautosize(0, 5, 6, 5, "r",
                "TCP-Stack on this OS not usable for Igate,
                gateing switched off");
     END; 
   */
   sent = (int32_t)(sock->txbytes-sock->qwatch.lasttb);
                /* last second sent bytes */
   if (qb<=0L || sent<0L) sock->qwatch.qsize = 0L;
   else {
      /* we have unack bytes */
      acked = (sock->qwatch.lastqb-qb)+sent; /* acked bytes in last s */
      j = 0L;
      rest = 0;
      tmp = sock->qwatch.qsize-1L;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         rb = sock->qwatch.txbyte[i]-acked;
         sock->qwatch.txbyte[j] = rb;
         if (rb>0L) rest = 1;
         else acked = -rb;
         if (rest) {
            acked = 0L;
            ++j;
         }
         if (i==tmp) break;
      } /* end for */
      sock->qwatch.qsize = j;
      if (sock->qwatch.qsize<=59L) {
         sock->qwatch.txbyte[sock->qwatch.qsize] = sent;
         ++sock->qwatch.qsize;
         if (sock->qwatch.qsize>1L) saylinkbad(sock->qwatch.qsize);
         if (sock->qwatch.qsize>15L) {
            stoptxrx(sock->fd, 2L); /* delete unsent data */
            saybusy(&sock->fd, "\015\012", 3ul); /* close tcp immediately */
         }
      }
   }
   /*        sock^.slowlink:=TRUE;
                (* for log message *) */
   sock->qwatch.lastqb = qb;
   sock->qwatch.lasttb = sock->txbytes;
} /* end WatchTXQ() */


static void tickermon(const char port[], uint32_t port_len,
                char dir, uint32_t ot, const char s[],
                uint32_t s_len)
{
   struct aprsdecode_DAT dat;
   TICKERCALL h2;
   TICKERCALL h1;
   TICKERCALL h;
   uint32_t cnt;
   uint32_t i;
   uint32_t tmp;
   cnt = (uint32_t)useri_conf2int(useri_fWRTICKER, 0UL, 0L, 99L, 1L);
   if (cnt==0UL && ot==0UL) cnt = 1UL;
   if ((ot!=1UL && cnt>0UL) && aprsdecode_Decode(s, s_len, &dat)>=0L) {
      h2[0] = 0;
      if (cnt==1UL) {
         if (ot==0UL) strncpy(h2,"New User: ",31u);
         else if (ot<=aprsdecode_realtime) {
            strncpy(h2,"since ",31u);
            ot = aprsdecode_realtime-ot;
            if (ot<180UL) {
               aprsstr_IntToStr((int32_t)ot, 3UL, h1, 31ul);
               aprsstr_Append(h1, 31ul, "s: ", 4ul);
            }
            else if (ot<10790UL) {
               aprsstr_IntToStr((int32_t)((ot+10UL)/60UL), 3UL, h1, 31ul);
               aprsstr_Append(h1, 31ul, "m: ", 4ul);
            }
            else {
               aprsstr_IntToStr((int32_t)((ot+10UL)/3600UL), 3UL, h1,
                31ul);
               aprsstr_Append(h1, 31ul, "h: ", 4ul);
            }
            aprsstr_Append(h2, 31ul, h1, 31ul);
         }
      }
      aprsstr_Assign(h1, 31ul, port, port_len);
      h1[0U] = X2C_CAP(h1[0U]);
      aprsstr_Append(h2, 31ul, h1, 31ul);
      aprsstr_Append(h2, 31ul, (char *) &dir, 1u/1u);
      aprsstr_Append(h2, 31ul, dat.symcall, 9ul);
      if (dat.wx.temp!=1.E+6f) {
         aprsstr_FixToStr(aprstext_FtoC(dat.wx.temp), 2UL, h, 31ul);
         aprsstr_Append(h2, 31ul, " ", 2ul);
         aprsstr_Append(h2, 31ul, h, 31ul);
         aprsstr_Append(h2, 31ul, "C", 2ul);
      }
      else if ((dat.altitude<X2C_max_longint && dat.altitude>=1000L)
                && dat.altitude>=useri_conf2int(useri_fALTMIN, 0UL, -10000L,
                65535L, -10000L)) {
         aprsstr_Append(h2, 31ul, " ", 2ul);
         aprsstr_IntToStr(dat.altitude, 1UL, h, 31ul);
         aprsstr_Append(h2, 31ul, h, 31ul);
         aprsstr_Append(h2, 31ul, "m", 2ul);
      }
      else if (dat.speed>0UL && dat.speed<2147483647UL) {
         useri_confstr(useri_fKMH, h1, 31ul);
         if (h1[0U]) {
            aprsstr_IntToStr((int32_t)aprsdecode_trunc((float)
                dat.speed*1.852f), 1UL, h, 31ul);
            aprsstr_Append(h2, 31ul, " ", 2ul);
            aprsstr_Append(h2, 31ul, h, 31ul);
            aprsstr_Append(h2, 31ul, h1, 31ul);
         }
      }
      for (i = 19UL; i>=1UL; i--) {
         memcpy(frameticker[i],frameticker[i-1UL],31u);
      } /* end for */
      memcpy(frameticker[0U],h2,31u);
      if (tickertime!=aprsdecode_realtime) {
         aprsstr_Assign(xosi_headmh, 251ul, h2, 31ul);
         if (cnt>19UL) cnt = 19UL;
         tmp = cnt-1UL;
         i = 1UL;
         if (i<=tmp) for (;; i++) {
            aprsstr_Append(xosi_headmh, 251ul, "  *  ", 6ul);
            aprsstr_Append(xosi_headmh, 251ul, frameticker[i], 31ul);
            if (i==tmp) break;
         } /* end for */
         tickertime = aprsdecode_realtime;
         xosi_WrHeadline();
      }
   }
} /* end tickermon() */


extern void aprsdecode_Stopticker(void)
{
   char h[61];
   char s[61];
   char * xbufa;
   uint32_t xbufysize;
   uint32_t xbufxsize;
   uint32_t inca;
   xosi_getscreenbuf(&xbufa, &xbufxsize, &xbufysize, &inca);
   strncpy(s,"aprsmap by oe5dxl ",61u);
   aprsstr_IntToStr((int32_t)xbufxsize, 1UL, h, 61ul);
   aprsstr_Append(s, 61ul, h, 61ul);
   aprsstr_Append(s, 61ul, "x", 2ul);
   aprsstr_IntToStr((int32_t)xbufysize, 1UL, h, 61ul);
   aprsstr_Append(s, 61ul, h, 61ul);
   aprsstr_Append(s, 61ul, " Map:[", 7ul);
   aprsstr_Append(s, 61ul, aprsdecode_lums.mapname, 41ul);
   aprsstr_Append(s, 61ul, "] Zoom:", 8ul);
   aprsstr_FixToStr(((float)aprsdecode_initzoom-0.95f)
                +aprsdecode_finezoom, 2UL, h, 61ul);
   aprsstr_Append(s, 61ul, h, 61ul);
   if (!aprsstr_StrCmp(s, 61ul, xosi_headmh, 251ul)) {
      aprsstr_Assign(xosi_headmh, 251ul, s, 61ul);
      xosi_WrHeadline();
   }
   tickertime = 0x0FFFFFFC3UL;
   memset((char *)frameticker,(char)0,620UL);
} /* end Stopticker() */


static void wrmon(uint32_t port, char dir,
                const struct UDPSET modeminfo, uint32_t oldtime,
                const char s[], uint32_t s_len)
{
   char mcon[10];
   char ms[31];
   if (port==0UL) strncpy(mcon,"n",10u);
   else aprsstr_IntToStr((int32_t)port, 0UL, mcon, 10ul);
   if ((dir=='>' || dir=='-') && useri_configon(useri_fWRTICKER)) {
      tickermon(mcon, 10ul, dir, oldtime, s, s_len);
   }
   /*IF configon(fWRINCOM) THEN */
   useri_confstr(useri_fWRINCOM, ms, 31ul);
   if (aprsstr_InStr(ms, 31ul, mcon, 10ul)>=0L) {
      aprsstr_Append(mcon, 10ul, "+", 2ul);
      aprstext_listin(s, s_len, X2C_CAP(mcon[0U]), dir, aprsstr_InStr(ms,
                31ul, mcon, 10ul)>=0L, modeminfo.quali, modeminfo.txd,
                modeminfo.level);
   }
/*END; */
} /* end wrmon() */


static char filtdupes(const char tb[], uint32_t tb_len,
                uint32_t oport, uint32_t dt)
{
   uint32_t n2;
   uint32_t n1;
   uint32_t n;
   uint32_t p;
   uint8_t hashh;
   uint8_t hashl;
   struct aprsdecode_UDPSOCK * anonym;
   hashl = 0U;
   hashh = 0U;
   p = 0UL;
   n1 = 0UL;
   n2 = 0UL;
   for (;;) {
      while (tb[p] && tb[p]!=':') ++p;
      if (tb[p]!=':') break;
      n1 = p;
      ++p;
      if (tb[p]!='}') break;
      ++p;
      n2 = p;
   }
   p = n2;
   while (p<n1 && tb[p]!='>') {
      aprsstr_HashCh(tb[p], &hashl, &hashh);
      ++p;
   }
   while ((p<n1 && tb[p]!='-') && tb[p]!=',') {
      aprsstr_HashCh(tb[p], &hashl, &hashh);
      ++p;
   }
   p = n1;
   while (tb[p]) {
      aprsstr_HashCh(tb[p], &hashl, &hashh);
      ++p;
   }
   n = (uint32_t)(uint8_t)(char)hashl+(uint32_t)(uint8_t)
                (char)hashh*256UL&65535UL;
   { /* with */
      struct aprsdecode_UDPSOCK * anonym = &aprsdecode_udpsocks0[oport];
      if (anonym->pdupetimes==0) {
         osic_alloc((char * *) &anonym->pdupetimes, 262144UL);
         useri_debugmem.req = 262144UL;
         useri_debugmem.mon += 262144UL;
         if (anonym->pdupetimes==0) {
            osi_WrStrLn("duptime out of memory", 22ul);
            useri_wrheap();
            return 0;
         }
         memset((char *)anonym->pdupetimes,(char)0,262144UL);
      }
      if (anonym->pdupetimes[n]+dt>aprsdecode_realtime) return 0;
      /* it is a dupe */
      anonym->pdupetimes[n] = aprsdecode_realtime;
   }
   return 1;
} /* end filtdupes() */


static char Sendtcp(aprsdecode_pTCPSOCK to,
                const aprsdecode_FRAMEBUF buf)
{
   int32_t i;
   int32_t len;
   char ok0;
   struct UDPSET modeminfo;
   struct aprsdecode_TCPSOCK * anonym;
   int32_t tmp;
   ok0 = 0;
   if (to) {
      if (to->connt>0UL) {
         memset((char *) &modeminfo,(char)0,sizeof(struct UDPSET));
         wrmon(0UL, '<', modeminfo, 1UL, buf, 512ul);
      }
      len = (int32_t)aprsstr_Length(buf, 512ul);
      { /* with */
         struct aprsdecode_TCPSOCK * anonym = to;
         if (anonym->tlen+len<512L) {
            tmp = len-1L;
            i = 0L;
            if (i<=tmp) for (;; i++) {
               anonym->tbuf[anonym->tlen] = buf[i];
               ++anonym->tlen;
               if (i==tmp) break;
            } /* end for */
            ++anonym->txframes;
            anonym->txbytes += (uint32_t)len;
            aprsdecode_lasttcptx = aprsdecode_realtime;
            ok0 = 1;
         }
         else if (aprsdecode_verb) osi_WrStrLn("tx buf overflow", 16ul);
      }
   }
   return ok0;
} /* end Sendtcp() */


static int32_t Sendudp(const char s[], uint32_t s_len,
                uint32_t uport, uint32_t dupetime0)
{
   int32_t len;
   aprsdecode_FRAMEBUF raw;
   struct UDPSET modeminfo;
   struct aprsdecode_UDPSOCK * anonym;
   len = -1L;
   if ((uport<=3UL && (int32_t)aprsdecode_udpsocks0[uport].fd>=0L)
                && aprsdecode_udpsocks0[uport].dport>0UL) {
      if (dupetime0==0UL || filtdupes(s, s_len, uport, dupetime0)) {
         aprsstr_mon2raw(s, s_len, raw, 512ul, &len);
         if (len>1L) {
            len = udpsend(aprsdecode_udpsocks0[uport].fd, raw, len,
                aprsdecode_udpsocks0[uport].dport,
                aprsdecode_udpsocks0[uport].ip);
            if (len>0L) {
               memset((char *) &modeminfo,(char)0,
                sizeof(struct UDPSET));
               wrmon(uport+1UL, '<', modeminfo, 1UL, s, s_len);
               { /* with */
                  struct aprsdecode_UDPSOCK * anonym = &aprsdecode_udpsocks0[uport]
                ;
                  ++anonym->txframes;
                  anonym->txbytes += (uint32_t)len;
                  anonym->lastudptx = aprsdecode_realtime;
                  if (anonym->starttime==0UL) {
                     anonym->starttime = aprsdecode_realtime;
                  }
               }
            }
         }
         else if (aprsdecode_verb) {
            osi_WrStrLn("wrong rf monitor format", 24ul);
         }
      }
      else len = 0L;
   }
   return len;
} /* end Sendudp() */

#define aprsdecode_MAXTXBIT 2000
/* clamp to +- bit */

#define aprsdecode_MSGPREF 1000
/* prefer msg time space */


static char Checkbps(uint32_t uport, uint32_t bps,
                uint32_t bytes, char msgprefer)
/* check/set bit/s for net>rf*/
{
   uint32_t t;
   struct aprsdecode_UDPSOCK * anonym;
   if (bps>0UL && uport<=3UL) {
      t = 2000UL/bps;
      { /* with */
         struct aprsdecode_UDPSOCK * anonym = &aprsdecode_udpsocks0[uport];
         if (bytes==0UL) {
            /* check */
            /*WrInt(bpstime-realtime, 10); WrStrLn(" check tx"); */
            if (anonym->bpstime<=aprsdecode_realtime+(1000UL/bps)
                *(uint32_t)msgprefer) return 1;
            t += aprsdecode_realtime;
            if (anonym->bpstime>t) anonym->bpstime = t;
         }
         else {
            /* set next tx time */
            if (anonym->bpstime+t<aprsdecode_realtime) {
               anonym->bpstime = aprsdecode_realtime-t;
                /* limit transmission at once */
            }
            anonym->bpstime += (bytes*8UL)/bps;
         }
      }
   }
   /*WrInt(bpstime-realtime, 10); WrStrLn(" next tx"); */
   return 0;
} /* end Checkbps() */

#define aprsdecode_RAININCH 3.9370078740157

#define aprsdecode_SEP ","


static char csvget(char w[], uint32_t w_len, const char s[],
                 uint32_t s_len, uint32_t n)
{
   uint32_t j;
   uint32_t i;
   w[0UL] = 0;
   i = 0UL;
   while (n>0UL) {
      if (i>s_len-1 || s[i]==0) return 0;
      if (s[i]==',') --n;
      ++i;
   }
   j = 0UL;
   while (((j<=w_len-1 && i<s_len-1) && s[i]) && s[i]!=',') {
      w[j] = s[i];
      ++i;
      ++j;
   }
   if (j<=w_len-1) w[j] = 0;
   return 1;
} /* end csvget() */


static char getreal(float * x, const char s[],
                uint32_t s_len)
{
   char val;
   char dot;
   float div0;
   uint32_t i;
   dot = 0;
   val = 0;
   *x = 0.0f;
   div0 = 1.0f;
   i = 0UL;
   for (;;) {
      if (div0>=0.0f && s[i]=='-') div0 = -div0;
      else if ((uint8_t)s[i]>='0' && (uint8_t)s[i]<='9') {
         *x =  *x*10.0f+(float)((uint32_t)(uint8_t)s[i]-48UL);
         if (*x>1.E+6f) {
            val = 0;
            break;
         }
         if (dot) div0 = div0*0.1f;
         val = 1;
      }
      else if (!dot && s[i]=='.') dot = 1;
      else break;
      ++i;
      if (i>s_len-1) break;
   }
   *x =  *x*div0;
   return val;
} /* end getreal() */


static void apd(char ws[], uint32_t ws_len, char c, float v,
                uint32_t f)
{
   int32_t d;
   int32_t n;
   char tmp;
   if (c) aprsstr_Append(ws, ws_len, (char *) &c, 1u/1u);
   n = (int32_t)X2C_TRUNCI(v+0.5f,X2C_min_longint,X2C_max_longint);
   if (n<0L) {
      aprsstr_Append(ws, ws_len, "-", 2ul);
      n = -n;
      --f;
   }
   d = 1L;
   while (f>1UL) {
      d = d*10L;
      --f;
   }
   do {
      aprsstr_Append(ws, ws_len, (char *)(tmp = (char)((n/d)%10L+48L),
                &tmp), 1u/1u);
      d = d/10L;
   } while (d);
} /* end apd() */


static void wxmacro(char ws[], uint32_t ws_len, char wms[],
                uint32_t wms_len)
/* \\!wx.txt,,t,,h,b,,w3.6,d! */
{
   struct aprsdecode_WX w;
   uint32_t n;
   int32_t payload;
   int32_t len;
   int32_t f;
   float mul;
   float v;
   float winddir;
   float wind;
   char cb[1024];
   char ns[1024];
   char fn[1024];
   char cn[21];
   char c;
   struct aprsdecode_DAT d;
   X2C_PCOPY((void **)&wms,wms_len);
   if (csvget(fn, 1024ul, wms, wms_len, 0UL)) {
      /* csv filename */
      len = 0L;
      if (fn[0U]) {
         f = osi_OpenRead(fn, 1024ul);
         if (osic_FdValid(f)) {
            n = (uint32_t)osic_Size(f);
            if (n>1024UL) osic_Seek(f, n-1024UL);
            len = osi_RdBin(f, (char *)cb, 1024u/1u, 1024UL);
         }
         if (len>0L) {
            while (len>0L && (uint8_t)cb[len-1L]<=' ') {
               --len; /* find first visable char from end */
            }
            if (len<=1023L) cb[len] = 0;
            while (len>0L && (uint8_t)cb[len-1L]>=' ') {
               --len; /* find begin of last csv line */
            }
            aprsstr_Delstr(cb, 1024ul, 0UL, (uint32_t)len);
         }
         else cb[0U] = 0;
         if (cb[0U]==0) {
            aprsstr_Assign(ns, 1024ul, "wx csv file not readable ", 26ul);
            aprsstr_Append(ns, 1024ul, fn, 1024ul);
            useri_xerrmsg(ns, 1024ul);
            wms[0UL] = 0;
            goto label;
         }
         w.temp = 1.E+6f;
         winddir = 1.E+6f;
         wind = 1.E+6f;
         w.gust = 1.E+6f;
         w.hygro = 1.E+6f;
         w.baro = 1.E+6f;
         w.lum = 1.E+6f;
         w.rain1 = 1.E+6f;
         w.rain24 = 1.E+6f;
         w.raintoday = 1.E+6f;
         n = 1UL;
         while (csvget(cn, 21ul, wms, wms_len, n)) {
            c = cn[0U]; /* w3.6 */
            if (c) {
               aprsstr_Delstr(cn, 21ul, 0UL, 1UL); /* 3.6 */
               if (!getreal(&mul, cn, 21ul)) mul = 1.0f;
               if (csvget(ns, 1024ul, cb, 1024ul, n-1UL) && getreal(&v, ns,
                1024ul)) {
                  v = v*mul;
                  switch ((unsigned)c) {
                  case 't':
                     if (v>=(-60.0f) && v<=79.0f) w.temp = v;
                     break;
                  case 'g':
                     if (v>=0.0f && v<=500.0f) w.gust = v;
                     break;
                  case 'w':
                     if (v>=0.0f && v<=500.0f) wind = v;
                     break;
                  case 'd':
                     if (v>=0.0f && v<360.0f) winddir = v;
                     break;
                  case 'b':
                     if (v>=900.0f && v<=1100.0f) w.baro = v;
                     break;
                  case 'h':
                     if (v>=0.0f && v<=100.0f) w.hygro = v;
                     break;
                  case 'L':
                     if (v>=0.0f && v<=1500.0f) w.lum = v;
                     break;
                  case 'r':
                     if (v>=0.0f && v<=500.0f) w.rain1 = v;
                     break;
                  case 'P':
                     if (v>=0.0f && v<=500.0f) w.rain24 = v;
                     break;
                  case 'p':
                     if (v>=0.0f && v<=500.0f) w.raintoday = v;
                     break;
                  } /* end switch */
               }
            }
            ++n;
         }
         d.postyp = 'g';
         payload = aprsstr_InStr(ws, ws_len, ":", 2ul);
         if (payload>0L) {
            aprspos_GetPos(&d.pos, &d.speed, &d.course, &d.altitude, &d.sym,
                &d.symt, ws, ws_len, 0UL, (uint32_t)(payload+1L),
                d.comment0, 256ul, &d.postyp);
            /* exchange spd/dir */
            if (d.postyp=='c') {
               aprsstr_Delstr(ws, ws_len, (uint32_t)(payload+2L), ws_len);
               d.speed = aprsdecode_trunc(X2C_DIVR(wind,1.609f));
               d.course = aprsdecode_trunc(winddir);
               cb[0U] = d.symt;
               cb[1U] = d.sym;
               cb[2U] = 0;
               aprstext_compressdata(d.pos, d.speed, d.course, 0L, cb,
                1024ul, cb, 1024ul);
               aprsstr_Append(ws, ws_len, cb, 1024ul);
            }
            else if (d.postyp!='g') {
               aprsstr_Assign(ns, 1024ul, "wx beacon not MICE encodable",
                29ul);
               useri_xerrmsg(ns, 1024ul);
            }
         }
         if (d.postyp!='c') {
            if (winddir!=1.E+6f) apd(ws, ws_len, 0, winddir+1.0f, 3UL);
            else aprsstr_Append(ws, ws_len, "...", 4ul);
            if (wind!=1.E+6f) {
               apd(ws, ws_len, '/', X2C_DIVR(wind,1.609f), 3UL);
            }
            else aprsstr_Append(ws, ws_len, "/...", 5ul);
         }
         if (w.gust!=1.E+6f) {
            apd(ws, ws_len, 'g', X2C_DIVR(w.gust,1.609f), 3UL);
         }
         else aprsstr_Append(ws, ws_len, "g...", 5ul);
         if (w.temp!=1.E+6f) {
            apd(ws, ws_len, 't', aprstext_CtoF(w.temp), 3UL);
         }
         else aprsstr_Append(ws, ws_len, "t...", 5ul);
         if (w.rain1!=1.E+6f) {
            apd(ws, ws_len, 'r', w.rain1*3.9370078740157f, 3UL);
         }
         if (w.rain24!=1.E+6f) {
            apd(ws, ws_len, 'P', w.rain24*3.9370078740157f, 3UL);
         }
         if (w.raintoday!=1.E+6f) {
            apd(ws, ws_len, 'p', w.raintoday*3.9370078740157f, 3UL);
         }
         if (w.hygro!=1.E+6f) apd(ws, ws_len, 'h', w.hygro, 2UL);
         if (w.baro!=1.E+6f) apd(ws, ws_len, 'b', w.baro*10.0f, 5UL);
         if (w.lum!=1.E+6f) {
            if (w.lum>=1000.0f) apd(ws, ws_len, 'l', w.lum-1000.0f, 3UL);
            else apd(ws, ws_len, 'L', w.lum, 3UL);
         }
      }
      else {
         aprsstr_Assign(ns, 1024ul, "wx beacon macro needs valid filename ",
                38ul);
         useri_xerrmsg(ns, 1024ul);
         ws[0UL] = 0;
      }
   }
   else {
      aprsstr_Assign(ns, 1024ul, "wx beacon macro \\\\!filename,...! ",
                34ul);
      useri_xerrmsg(ns, 1024ul);
      ws[0UL] = 0;
   }
   label:;
   X2C_PFREE(wms);
} /* end wxmacro() */

#define aprsdecode_MSYM "\\"

#define aprsdecode_INSOPEN "<"

#define aprsdecode_INSCLOSE ">"

#define aprsdecode_DELOPEN "["

#define aprsdecode_DELCLOSE "]"

#define aprsdecode_WROPEN ">"

#define aprsdecode_WRCLOSE "<"

#define aprsdecode_WXOPEN "!"

#define aprsdecode_WXCLOSE "!"


static void beaconmacros(char s[], uint32_t s_len,
                const char path[], uint32_t path_len, char wdata[],
                 uint32_t wdata_len, char all)
{
   uint32_t i;
   int32_t j;
   int32_t len;
   char ns[256];
   char ds[256];
   char fn[1024];
   int32_t f;
   char ok0;
   char rwc;
   char rw;
   struct aprsstr_POSITION pos;
   X2C_PCOPY((void **)&wdata,wdata_len);
   i = 0UL;
   ns[0U] = 0;
   while (i<s_len-1 && s[i]) {
      if (s[i]=='\\' && s[i+1UL]=='\\') {
         i += 2UL;
         if (s[i]=='z' && all || s[i]=='Z') {
            /* insert day, hour, min */
            aprsstr_DateToStr(aprsdecode_realtime, ds, 256ul);
            ds[0U] = ds[8U];
            ds[1U] = ds[9U];
            ds[2U] = ds[11U];
            ds[3U] = ds[12U];
            ds[4U] = ds[14U];
            ds[5U] = ds[15U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='h' && all || s[i]=='H') {
            /* insert hour, min, s */
            aprsstr_DateToStr(aprsdecode_realtime, ds, 256ul);
            ds[0U] = ds[11U];
            ds[1U] = ds[12U];
            ds[2U] = ds[14U];
            ds[3U] = ds[15U];
            ds[4U] = ds[17U];
            ds[5U] = ds[18U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (all) {
            rw = s[i];
            if (rw=='<') rwc = '>';
            else if (rw=='>') rwc = '<';
            else if (rw=='[') rwc = ']';
            else if (rw=='!') rwc = '!';
            else rwc = 0;
            if (rwc) {
               /* insert or write file or insert and delete */
               fn[0U] = 0;
               ++i;
               while ((i<s_len-1 && s[i]) && s[i]!=rwc) {
                  aprsstr_Append(fn, 1024ul, (char *) &s[i], 1u/1u);
                  ++i;
               }
               if (rw=='<' || rw=='[') {
                  /* read file */
                  f = osi_OpenRead(fn, 1024ul);
                  if (osic_FdValid(f)) {
                     len = osi_RdBin(f, (char *)ds, 256u/1u, 255UL);
                     osic_Close(f);
                     if (rw=='[') osi_Erase(fn, 1024ul, &ok0);
                     j = 0L;
                     while (((j<len && ds[j]!='\015') && ds[j]!='\012')
                && ds[j]) {
                        aprsstr_Append(ns, 256ul, (char *) &ds[j],
                1u/1u);
                        ++j;
                     }
                  }
                  else {
                     aprsstr_Assign(s, s_len, "beacon macro file not readable\
 ", 32ul);
                     aprsstr_Append(s, s_len, fn, 1024ul);
                     useri_xerrmsg(s, s_len);
                     s[0UL] = 0;
                     goto label;
                  }
               }
               else if (rw=='>') {
                  if (wdata[0UL]) {
                     /* write file */
                     f = osi_OpenWrite(fn, 1024ul);
                     if (osic_FdValid(f)) {
                        osi_WrBin(f, (char *)wdata, (wdata_len)/1u,
                aprsstr_Length(wdata, wdata_len));
                        osic_Close(f);
                     }
                  }
               }
               else if (rw=='!') wxmacro(ns, 256ul, fn, 1024ul);
            }
            else if (s[i]=='\\') {
               aprsstr_Append(ns, 256ul, "\\\\", 3ul);
            }
            else if (s[i]=='v') {
               aprsstr_Append(ns, 256ul, "aprsmap(cu) 0.77", 17ul);
            }
            else if (s[i]=='l') {
               if (aprstext_getmypos(&pos)) {
                  aprstext_postostr(pos, '2', ds, 256ul);
                  aprsstr_Append(ns, 256ul, ds, 256ul);
               }
            }
            else if (s[i]=='p') aprsstr_Append(ns, 256ul, path, path_len);
            else {
               aprsstr_Assign(s, s_len, "bad beacon macro ", 18ul);
               aprsstr_Append(s, s_len, ns, 256ul);
               aprsstr_Append(s, s_len, "???", 4ul);
               useri_xerrmsg(s, s_len);
               s[0UL] = 0;
               goto label;
            }
         }
         else {
            aprsstr_Append(ns, 256ul, "\\\\", 3ul);
            aprsstr_Append(ns, 256ul, (char *) &s[i], 1u/1u);
         }
      }
      else aprsstr_Append(ns, 256ul, (char *) &s[i], 1u/1u);
      ++i;
   }
   aprsstr_Assign(s, s_len, ns, 256ul);
   label:;
   X2C_PFREE(wdata);
} /* end beaconmacros() */


static void BuildNetBeacon(char h[], uint32_t h_len)
/* "!ddmm.mmNsdddmm.mms....." */
{
   struct aprsstr_POSITION mypos;
   char h3[301];
   char h2[301];
   char h1[301];
   useri_confstr(useri_fMYSYM, h3, 301ul);
   useri_confstr(useri_fNBTEXT, h2, 301ul);
   if (!aprstext_getmypos(&mypos)) {
      useri_xerrmsg("netbeacon: wrong my position", 29ul);
      h[0UL] = 0;
      return;
   }
   if (aprsstr_Length(h3, 301ul)!=2UL) {
      useri_xerrmsg("netbeacon: wrong symbol", 24ul);
      h[0UL] = 0;
      return;
   }
   aprstext_postostr(mypos, '2', h1, 301ul);
   aprsstr_Assign(h, h_len, "=", 2ul);
   h1[8U] = h3[0U];
   h1[18U] = h3[1U];
   h1[19U] = 0;
   aprsstr_Append(h, h_len, h1, 301ul);
   beaconmacros(h2, 301ul, "", 1ul, "", 1ul, 1);
   aprsstr_Append(h, h_len, h2, 301ul);
} /* end BuildNetBeacon() */


static void SendNetBeacon(aprsdecode_pTCPSOCK cp,
                const aprsdecode_FRAMEBUF b, char manual,
                char errtxt[], uint32_t errtxt_len)
{
   int32_t ret;
   struct aprsdecode_DAT dat;
   uint32_t t;
   if (b[0UL]==0 || aprsdecode_Decode(b, 512ul, &dat)<0L) {
      aprsstr_Assign(errtxt, errtxt_len, "Net Beacon not Decodable", 25ul);
      return;
   }
   if (!Sendtcp(cp, b)) {
      if (manual) {
         aprsstr_Assign(errtxt, errtxt_len, "Netport not connected", 22ul);
      }
      return;
   }
   if (aprsdecode_lums.logmode) {
      ret = aprsdecode_Stoframe(&aprsdecode_ophist2, b, 512ul,
                aprsdecode_realtime, 0, &t, dat);
   }
   else {
      /*IF logdone THEN*/
      ret = aprsdecode_Stoframe(&aprsdecode_ophist0, b, 512ul,
                aprsdecode_realtime, 0, &t, dat);
   }
   if (aprsdecode_logdone) aprsdecode_Checktracks();
} /* end SendNetBeacon() */


static void SendNet(aprsdecode_pTCPSOCK cp, char mycall[],
                uint32_t mycall_len, const char data[],
                uint32_t data_len, char manual, char errtxt[],
                uint32_t errtxt_len)
/* make net header append string and send */
{
   aprsdecode_FRAMEBUF s;
   char pass[51];
   char h[51];
   uint32_t i;
   X2C_PCOPY((void **)&mycall,mycall_len);
   errtxt[0UL] = 0;
   if (!useri_configon(useri_fALLOWNETTX) || !useri_configon(useri_fCONNECT))
                 {
      if (manual) {
         aprsstr_Assign(errtxt, errtxt_len,
                "Net Port not configured to Send", 32ul);
      }
      goto label;
   }
   if (mycall[0UL]) aprsstr_Assign(s, 512ul, mycall, mycall_len);
   else useri_confstr(useri_fMYCALL, s, 512ul);
   i = 0UL;
   while (i<511UL && s[i]) {
      if ((uint8_t)s[i]<=' ' || (uint8_t)s[i]>='\177') s[0UL] = 0;
      ++i;
   }
   if (aprstext_callwrong(s, 512ul)) {
      aprsstr_Assign(errtxt, errtxt_len, "netbeacon: no valid mycall", 27ul);
      goto label;
   }
   useri_confstr(useri_fMSGNETDEST, h, 51ul);
   if (h[0U]==0) {
      aprsstr_Assign(errtxt, errtxt_len, "netbeacon: no destination call",
                31ul);
      goto label;
   }
   if (manual) {
      useri_confstr(useri_fPW, pass, 51ul);
      if (pass[0U]==0) {
         useri_xerrmsg("sending to Net without Passcode", 32ul);
      }
   }
   aprsstr_Append(s, 512ul, ">", 2ul);
   aprsstr_Append(s, 512ul, h, 51ul);
   aprsstr_Append(s, 512ul, ",TCPIP*:", 9ul);
   aprsstr_Append(s, 512ul, data, data_len);
   aprsstr_Append(s, 512ul, "\015\012", 3ul);
   SendNetBeacon(cp, s, manual, errtxt, errtxt_len);
   label:;
   X2C_PFREE(mycall);
} /* end SendNet() */


static void makeping(uint32_t t, char pong, char b[],
                uint32_t b_len)
{
   char s[21];
   if (pong) aprsstr_Assign(b, b_len, "pong ", 6ul);
   else aprsstr_Assign(b, b_len, "ping ", 6ul);
   aprsstr_IntToStr((int32_t)(t%86400UL), 0UL, s, 21ul);
   aprsstr_Append(b, b_len, s, 21ul);
} /* end makeping() */


static void sendping(aprsdecode_pTCPSOCK tp)
{
   char h[21];
   aprsdecode_FRAMEBUF s;
   char ok0;
   if (aprsdecode_realtime<tp->lastping) tp->lastping = aprsdecode_realtime;
   if (aprsdecode_realtime>=tp->lastping+20UL) {
      tp->lastping = aprsdecode_realtime;
      if (tp->waitpong<=1U) ++tp->waitpong;
      strncpy(s,"#",512u);
      makeping(tp->lastping, 0, h, 21ul);
      aprsstr_Append(s, 512ul, h, 21ul);
      aprsstr_Append(s, 512ul, "\015\012", 3ul);
      ok0 = Sendtcp(tp, s);
   }
} /* end sendping() */


static void getpong(aprsdecode_pTCPSOCK tp, const char mb[],
                uint32_t mb_len)
{
   char s[21];
   int32_t i;
   /*# logresp AA0AAA verified, server AA0AAA-10 */
   i = aprsstr_InStr(mb, mb_len, "logresp ", 9ul);
   if (i>0L && i<=2L) useri_textautosize(0L, 5L, 6UL, 10UL, 'b', mb, mb_len);
   makeping(tp->lastping, 1, s, 21ul);
   if (aprsdecode_realtime<=tp->lastping+10UL && aprsstr_InStr(mb, mb_len, s,
                 21ul)>=1L) {
      tp->lastpong = aprsdecode_realtime;
      tp->waitpong = 0U;
   }
} /* end getpong() */


static void Timebeacon(aprsdecode_pTCPSOCK cp)
{
   aprsdecode_FRAMEBUF h;
   uint32_t bt;
   char err[100];
   if (useri_configon(useri_fALLOWNETTX) && useri_configon(useri_fCONNECT)) {
      bt = (uint32_t)useri_conf2int(useri_fNETBTIME, 0UL, 0L,
                X2C_max_longint, 0L);
      if ((bt>=1UL || cp->beacont==0UL) && Watchclock(&cp->beacont, bt)) {
         if ((bt<60UL && bt>0UL) && cp->connt>0UL) {
            useri_xerrmsg("Netbeacon: too fast", 20ul);
         }
         BuildNetBeacon(h, 512ul);
         SendNet(cp, "", 1ul, h, 512ul, 0, err, 100ul);
         useri_xerrmsg(err, 100ul);
      }
      if (cp->lastping>0UL && useri_configon(useri_fALLOWGATE)) sendping(cp);
   }
} /* end Timebeacon() */

/*
PROCEDURE nibb(n:CARDINAL; VAR s:ARRAY OF CHAR);
BEGIN IF n<=9 THEN Append(s, CHR(n+ORD("0"))) ELSE Append(s,
                CHR(n+(ORD("a")-10))) END END nibb;
*/

extern int32_t aprsdecode_knottokmh(int32_t kn)
{
   if (kn>=0L && kn<50000L) {
      return (int32_t)aprsdecode_trunc((float)kn*1.852f+0.5f);
   }
   else return 0L;
   return 0;
} /* end knottokmh() */


static void NoWX(struct aprsdecode_WX * wx)
{
   wx->gust = 1.E+6f;
   wx->temp = 1.E+6f;
   wx->rain1 = 1.E+6f;
   wx->rain24 = 1.E+6f;
   wx->raintoday = 1.E+6f;
   wx->hygro = 1.E+6f;
   wx->baro = 1.E+6f;
   wx->lum = 1.E+6f;
   wx->sievert = 1.E+6f;
   wx->storm = aprsdecode_WXNOWX;
   wx->sustaind = 0.0f;
   wx->radiushurr = 0.0f;
   wx->radiusstorm = 0.0f;
   wx->wholegale = 0.0f;
} /* end NoWX() */


static void wpar(uint32_t * p, char buf[], uint32_t buf_len,
                float * v, int32_t mindig, int32_t maxdig)
{
   char empty;
   char dot;
   char sn;
   char c;
   float x;
   float div0;
   dot = 0;
   empty = 1;
   sn = buf[*p]=='t';
   x = 0.0f;
   div0 = 1.0f;
   for (;;) {
      ++*p;
      c = buf[*p];
      if (sn && c=='-') {
         div0 = -div0;
         empty = 0;
         --mindig;
         --maxdig;
      }
      else if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         if (!dot && maxdig<=0L) {
            x = 0.0f;
            break;
         }
         x = x*10.0f+(float)((uint32_t)(uint8_t)buf[*p]-48UL);
         if (x>1.E+5f) return;
         if (dot) div0 = div0*0.1f;
         else {
            --mindig;
            --maxdig;
         }
         empty = 0;
      }
      else if (empty) {
         if (c!=' ' && c!='.') break;
      }
      else if (c=='.') {
         if (dot) break;
         dot = 1;
      }
      else break;
      sn = 0;
   }
   if (!empty && mindig<=0L) *v = x*div0;
} /* end wpar() */


static void wexp(char buf[], uint32_t buf_len, uint32_t * p,
                float * v, float mul)
{
   uint32_t n;
   float x;
   x = 0.0f;
   ++*p;
   if ((uint8_t)buf[*p]>='0' && (uint8_t)buf[*p]<='9') {
      x = 10.0f*(float)((uint32_t)(uint8_t)buf[*p]-48UL);
   }
   else return;
   ++*p;
   if ((uint8_t)buf[*p]>='0' && (uint8_t)buf[*p]<='9') {
      x = x+(float)((uint32_t)(uint8_t)buf[*p]-48UL);
   }
   else return;
   ++*p;
   if ((uint8_t)buf[*p]>='0' && (uint8_t)buf[*p]<='9') {
      n = (uint32_t)(uint8_t)buf[*p]-48UL;
      while (n>0UL) {
         x = x*10.0f; /* exponent */
         --n;
      }
      ++*p;
   }
   else return;
   *v = x*mul;
} /* end wexp() */


static void GetWX(struct aprsdecode_WX * wx, uint32_t * course,
                uint32_t * speed, char buf[], uint32_t buf_len,
                char storm)
{
   float wdir;
   float wwind;
   uint32_t p;
   p = 0UL;
   NoWX(wx);
   wwind = 1.E+6f;
   wdir = 1.E+6f;
   /*  IF buf[p]="_" THEN INC(p, 9) END;     (* positionless wx *) */
   if (storm) {
      /* storm data */
      /*4903.50N\07202.75W@088/036/HC/150^200/0980>090&030%040 */
      if (((buf[0UL]=='/' && buf[3UL]=='/') && buf[7UL]=='^')
                && buf[11UL]=='/') {
         ++p;
         if (buf[1UL]=='T' && buf[2UL]=='S') wx->storm = aprsdecode_WXTS;
         else if (buf[1UL]=='H' && buf[2UL]=='C') {
            wx->storm = aprsdecode_WXHC;
         }
         else if (buf[1UL]=='T' && buf[2UL]=='D') {
            wx->storm = aprsdecode_WXTD;
         }
         if (wx->storm) {
            p += 2UL;
            wpar(&p, buf, buf_len, &wx->sustaind, 3L, 3L);
            wpar(&p, buf, buf_len, &wx->gust, 3L, 3L);
            wpar(&p, buf, buf_len, &wx->baro, 4L, 4L);
            if (buf[p]=='>') wpar(&p, buf, buf_len, &wx->radiushurr, 3L, 3L);
            if (buf[p]=='&') {
               wpar(&p, buf, buf_len, &wx->radiusstorm, 3L, 3L);
            }
            if (buf[p]=='%') wpar(&p, buf, buf_len, &wx->wholegale, 3L, 3L);
         }
      }
   }
   else {
      /* normal wx */
      for (;;) {
         switch ((unsigned)buf[p]) {
         case 'g':
            wpar(&p, buf, buf_len, &wx->gust, 3L, 3L);
            break;
         case 't':
            wpar(&p, buf, buf_len, &wx->temp, 3L, 3L);
            break;
         case 'r':
            wpar(&p, buf, buf_len, &wx->rain1, 3L, 3L);
            break;
         case 'p':
            wpar(&p, buf, buf_len, &wx->rain24, 3L, 3L);
            break;
         case 'P':
            wpar(&p, buf, buf_len, &wx->raintoday, 3L, 3L);
            break;
         case 'h':
            wpar(&p, buf, buf_len, &wx->hygro, 2L, 3L);
            if (wx->hygro==0.0f) wx->hygro = 100.0f;
            break;
         case 'b':
            wpar(&p, buf, buf_len, &wx->baro, 5L, 5L);
            break;
         case 'L':
            wpar(&p, buf, buf_len, &wx->lum, 3L, 3L);
            break;
         case 'l':
            wpar(&p, buf, buf_len, &wx->lum, 3L, 3L);
            wx->lum = wx->lum+1000.0f;
            break;
         case 'c':
            wpar(&p, buf, buf_len, &wdir, 3L, 3L);
            break;
         case 's':
            wpar(&p, buf, buf_len, &wwind, 3L, 3L);
            break;
         case 'X':
            wexp(buf, buf_len, &p, &wx->sievert, 1.E-9f);
            break;
         default:;
            goto loop_exit;
         } /* end switch */
      }
      loop_exit:;
      wx->storm = aprsdecode_WXNORMAL;
   }
   wdir = wdir+0.5f;
   if (wdir>=1.0f && wdir<361.0f) *course = aprsdecode_trunc(wdir);
   if (wwind>=0.0f && wwind<1000.0f) *speed = aprsdecode_trunc(wwind);
   aprsstr_Delstr(buf, buf_len, 0UL, p);
/* X123 is 12 * 10^3 nanosieverts/hr*/
} /* end GetWX() */

#define aprsdecode_DATELEN 15


static char wrlog(const char b[], uint32_t b_len,
                uint32_t time0, char wfn[], uint32_t wfn_len,
                int32_t good)
{
   int32_t f;
   char h[1001];
   uint32_t i;
   uint32_t l;
   char wrlog_ret;
   X2C_PCOPY((void **)&wfn,wfn_len);
   if (wfn[0UL]==0) {
      wrlog_ret = 0;
      goto label;
   }
   l = aprsstr_Length(b, b_len);
   while (l>0UL && (uint8_t)b[l-1UL]<='\015') --l;
   if (l>0UL) {
      aprsstr_cleanfilename(wfn, wfn_len);
      f = osi_OpenAppendLong(wfn, wfn_len);
      if (!osic_FdValid(f)) f = osi_OpenWrite(wfn, wfn_len);
      if (!osic_FdValid(f)) {
         wrlog_ret = 0;
         goto label;
      }
      aprsstr_DateToStr(time0, h, 1001ul);
      h[4U] = h[5U];
      h[5U] = h[6U];
      h[6U] = h[8U];
      h[7U] = h[9U];
      h[8U] = ':';
      h[9U] = h[11U];
      h[10U] = h[12U];
      h[11U] = h[14U];
      h[12U] = h[15U];
      h[13U] = h[17U];
      h[14U] = h[18U];
      if (good>=0L) h[15U] = ' ';
      else h[15U] = '-';
      if (l>=984UL) l = 984UL;
      i = 0UL;
      do {
         h[i+16UL] = b[i];
         ++i;
      } while (i<l);
      h[i+16UL] = '\012';
      ++i;
      osi_WrBin(f, (char *)h, 1001u/1u, i+16UL);
      osic_Close(f);
   }
   wrlog_ret = 1;
   label:;
   X2C_PFREE(wfn);
   return wrlog_ret;
} /* end wrlog() */


static aprsdecode_pOPHIST findop(aprsdecode_MONCALL call,
                char online)
{
   aprsdecode_pOPHIST op;
   aprsdecode_MONCALL tmp;
   call = (char *)memcpy(tmp,call,9u);
   if (online && aprsdecode_lums.logmode) op = aprsdecode_ophist2;
   else op = aprsdecode_ophist0;
   while (op && X2C_STRCMP(call,9u,op->call,9u)) op = op->next;
   return op;
} /* end findop() */


static void poplogerr(char fn[], uint32_t fn_len)
{
   char h[1001];
   X2C_PCOPY((void **)&fn,fn_len);
   strncpy(h,"\012logfile write error ",1001u);
   aprsstr_Append(h, 1001ul, fn, fn_len);
   aprsstr_Append(h, 1001ul, "\012", 2ul);
   useri_textautosize(0L, 5L, 6UL, 4UL, 'e', h, 1001ul);
   X2C_PFREE(fn);
} /* end poplogerr() */


extern void aprsdecode_savetrack(void)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pf;
   char h[101];
   char fn[1001];
   char ap;
   aprsdecode_MONCALL name;
   name[0UL] = 0;
   if (aprsdecode_click.mhop[0UL]) memcpy(name,aprsdecode_click.mhop,9u);
   else if (aprsdecode_click.entries>0UL && aprsdecode_click.table[aprsdecode_click.selected]
                .opf) {
      memcpy(name,aprsdecode_click.table[aprsdecode_click.selected].opf->call,
                9u);
   }
   if (name[0UL]) {
      op = findop(name, 0);
      if (op) {
         aprsstr_Assign(fn, 1001ul, op->call, 9ul); /* make 0C terminated */
         alfanum(fn, 1001ul); /* remove bad chars */
         aprsdecode_makelogfn(fn, 1001ul);
         ap = osi_Exists(fn, 1001ul);
         pf = op->frames;
         while (pf) {
            if (!wrlog(pf->vardat->raw, 500ul, pf->time0, fn, 1001ul, 0L)) {
               poplogerr(fn, 1001ul);
               return;
            }
            pf = pf->next;
         }
         if (ap) strncpy(h,"Append to logfile ",101u);
         else strncpy(h,"Write logfile ",101u);
         aprsstr_Append(h, 101ul, fn, 1001ul);
         useri_say(h, 101ul, 4UL, 'b');
      }
   }
} /* end savetrack() */


static char replyack(char rep[], uint32_t rep_len,
                char restack[], uint32_t restack_len,
                const char ack[], uint32_t ack_len)
{
   aprsstr_Assign(rep, rep_len, ack, ack_len);
   aprsstr_Assign(restack, restack_len, ack, ack_len);
   if (aprsstr_Length(ack, ack_len)>2UL && ack[2UL]=='}') {
      /* {xx}yy */
      rep[0UL] = ack[3UL];
      rep[1UL] = ack[4UL];
      if (rep_len-1>=2UL) rep[2UL] = 0;
      if (restack_len-1>=2UL) restack[2UL] = 0;
      return 1;
   }
   return 0;
} /* end replyack() */


static char iscall(const aprsdecode_MONCALL b)
{
   uint32_t i;
   uint32_t nu;
   uint32_t li;
   char c;
   li = 0UL;
   nu = 0UL;
   for (i = 0UL; i<=2UL; i++) {
      c = b[i];
      if ((uint8_t)c>='A' && (uint8_t)c<='Z') ++li;
      else if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         if (i==0UL) ++li;
         else ++nu;
      }
   } /* end for */
   return (nu+li==3UL && nu>=1UL) && nu<=2UL;
} /* end iscall() */


static char viaghost(const char b[], uint32_t b_len)
/* n<>N */
{
   uint32_t i;
   char c;
   i = aprsstr_Length(b, b_len);
   if (i>3UL) {
      c = b[i-1UL];
      if (((uint8_t)c>='0' && (uint8_t)c<='9') && c!=b[i-3UL]) return 1;
   }
   return 0;
} /* end viaghost() */

/*
PROCEDURE viaghost(b-:ARRAY OF CHAR):BOOLEAN;
                (* ECHO GATE RELAY n=n ELSE ghost *)
VAR i:CARDINAL;
    c:CHAR;
BEGIN
  IF (b[0]="E") & (b[1]="C") & (b[2]="H") & (b[3]="O") & (b[4]=0C)
                THEN RETURN FALSE END;
  IF (b[0]="G") & (b[1]="A") & (b[2]="T") & (b[3]="E") & (b[4]=0C)
                THEN RETURN FALSE END;
  IF (b[0]="R") & (b[1]="E") & (b[2]="L") & (b[3]="A") & (b[4]="Y")
                & (b[5]=0C) THEN RETURN FALSE E
  i:=Length(b);
  c:=b[i-1];
  IF (i>3) & (b[i-2]="-") & (c=b[i-3]) & (c>="1") & (c<="7")
                THEN RETURN FALSE END;
  RETURN TRUE
END viaghost;
*/

static float num(char buf[], uint32_t buf_len, uint32_t * p)
{
   char c;
   char sig;
   float f;
   float r;
   r = 0.0f;
   sig = 0;
   f = 1.0f;
   for (;;) {
      if (*p>buf_len-1 || (float)fabs(r)>=2.147483647E+9f) break;
      c = buf[*p];
      if (sig==0 && c=='-' || c=='+') sig = c;
      else if (f>0.5f && c=='.') f = f*0.1f;
      else if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         if (f>0.5f) r = r*10.0f+(float)((uint32_t)(uint8_t)c-48UL);
         else {
            r = r+(float)((uint32_t)(uint8_t)c-48UL)*f;
            f = f*0.1f;
         }
      }
      else break;
      ++*p;
      if (sig==0) sig = '\001';
   }
   if (r>=2.147483647E+9f) r = 2.147483647E+9f;
   if (sig=='-') r = -r;
   return r;
} /* end num() */


static void skip(char buf[], uint32_t buf_len, uint32_t * p)
{
   if (*p<=buf_len-1 && buf[*p]==',') ++*p;
} /* end skip() */


static uint32_t corrtime(uint32_t syst, uint32_t mt, uint32_t span)
/* complete modulo time to absolute */
{
   mt = (span+mt%span)-syst%span;
   if (mt<span/2UL) mt += span;
   else if (mt>span+span/2UL) mt -= span;
   mt += syst;
   if (mt>=span) mt -= span;
   return mt;
} /* end corrtime() */

#define aprsdecode_H 3600

#define aprsdecode_DAY 86400

#define aprsdecode_DEGUNIT 1.7453292519444E-7
/* 1/100000 deg = 1.1111m*/


static void GetHRT(struct aprsstr_POSITION * pos, int32_t * altitude,
                uint32_t * speed, const char buf[], uint32_t buf_len,
                 struct aprsdecode_HRTPOS hrtposes[],
                uint32_t hrtposes_len, uint32_t * hrtlen,
                uint32_t * hrttime)
{
   uint32_t p;
   char cmd;
   uint32_t i;
   uint32_t crc;
   uint32_t duration;
   uint32_t wcnt;
   float r;
   float unit;
   uint32_t ts;
   struct aprsstr_POSITION posh;
   struct aprsstr_POSITION dao;
   char newcompos;
   struct aprsdecode_HRTPOS * anonym;
   uint32_t tmp;
   /* $HR[t46531u1.5p48.0235,13.1512a367d15,11w-479,622,33w-5,
                2-4w7+2-3ww4-1-5wg435644353-10c27001]
      t s utc modulo 24h modulo 1h or absolut since 1970
      p deg lat/long 
      a m altitude
      v m/s speed
      y m/s climb
      d deg lat/long dao
      w deg,deg,m,s lat long alt time
      u 0.00001 deg,m  unit
      g direct heard gateway
      c crc 
      q qrv mhz
      r rx freq mhz or hz
   */
   /*WrStrLn(buf); */
   *hrtlen = 0UL;
   *hrttime = 0UL;
   p = 0UL;
   newcompos = 1;
   /*WrInt(p, 10);WrStrLn(" compos"); */
   for (;;) {
      if (p+3UL>buf_len-1 || buf[p]==0) return;
      if (((buf[p]=='$' && buf[p+1UL]=='H') && buf[p+2UL]=='R')
                && buf[p+3UL]=='[') break;
      /*WrStrLn(" newcompos"); */
      ++p;
      newcompos = 0;
   }
   p += 4UL;
   ts = aprsdecode_systime;
   unit = 1.0f;
   duration = 0UL;
   wcnt = 0UL;
   dao.lat = 0.0f;
   dao.long0 = 0.0f;
   memset((char *)hrtposes,(char)0,
                hrtposes_len*sizeof(struct aprsdecode_HRTPOS));
   crc = 0UL;
   for (;;) {
      cmd = buf[p];
      ++p;
      if (cmd==0 || p>buf_len-1) {
         *hrtlen = 0UL; /* end of string? */
         return;
      }
      if (cmd=='t') {
         ts = aprsdecode_trunc(num(buf, buf_len, &p)+0.5f);
         if (ts<3600UL) {
            ts = corrtime(aprsdecode_systime, ts, 3600UL);
                /* move system time max +-30min */
         }
         else if (ts<90000UL) {
            ts = corrtime(aprsdecode_systime, ts-3600UL, 86400UL);
                /* move system time max +-12h */
         }
         /* absolute time */
         *hrttime = ts;
      }
      else if (cmd=='u') unit = num(buf, buf_len, &p);
      else if (cmd=='p') {
         /* take position from extension if correct */
         posh.lat = aprspos_rad0(num(buf, buf_len, &p));
         skip(buf, buf_len, &p);
         posh.long0 = aprspos_rad0(num(buf, buf_len, &p));
         if (aprspos_posvalid(posh)) *pos = posh;
      }
      else if (cmd=='a') {
         /* take altitude from extension if correct */
         r = num(buf, buf_len, &p);
         if (r>(-1.E+4f) && r<50000.0f) {
            *altitude = (int32_t)X2C_TRUNCI(r+0.5f,X2C_min_longint,
                X2C_max_longint);
         }
      }
      else if (cmd=='v') {
         /* take speed from extension if correct */
         r = num(buf, buf_len, &p);
         if (r>0.0f && r<50000.0f) {
            *speed = (uint32_t)(int32_t)X2C_TRUNCI(r+0.5f,
                X2C_min_longint,X2C_max_longint);
         }
      }
      else if (cmd=='d') {
         /*  ELSIF cmd="i" THEN                              (* define constant time intervall *)
                 */
         /*    duration:=trunc(num()+0.5); */
         /* DAO, correct position */
         dao.lat = num(buf, buf_len, &p);
         skip(buf, buf_len, &p);
         dao.long0 = num(buf, buf_len, &p);
      }
      else if (cmd=='w') {
         /* waypoints, first is difference from now */
         if (wcnt<=hrtposes_len-1) {
            { /* with */
               struct aprsdecode_HRTPOS * anonym = &hrtposes[wcnt];
               anonym->dlat = num(buf, buf_len, &p);
               skip(buf, buf_len, &p);
               anonym->dlong = num(buf, buf_len, &p);
               skip(buf, buf_len, &p);
               anonym->dalt = num(buf, buf_len, &p);
               anonym->notinterpolated = (anonym->dlat!=0.0f || anonym->dlong!=0.0f)
                 || anonym->dalt!=0.0f;
               skip(buf, buf_len, &p);
               r = num(buf, buf_len, &p);
               anonym->dtime = aprsdecode_trunc((float)fabs(r)+0.5f);
               if (r<0.0f) anonym->dtime = 0UL-anonym->dtime;
            }
            ++wcnt;
         }
      }
      else if (cmd=='c') {
         /* crc incl sender call and destination without ssid */
         crc = aprsdecode_trunc(num(buf, buf_len, &p)+0.5f);
      }
      else if (cmd==']') break;
   }
   /*
     IF (wcnt>0) & (hrtposes[0].dtime>0) THEN
       r:=1.0/FLOAT(wcnt);
       kt:=r*FLOAT(hrtposes[0].dtime);
       kx:=r*hrtposes[0].dlong;
       ky:=r*hrtposes[0].dlat;
       kh:=r*hrtposes[0].dalt;
       FOR i:=1 TO wcnt-1 DO
         r:=FLOAT(wcnt-i); 
         hrtposes[i].dtime:=trunc(kt*r + 0.5)-hrtposes[i].dtime;  
         hrtposes[i].dlong:=hrtposes[i].dlong + kx*r;
         hrtposes[i].dlat :=hrtposes[i].dlat  + ky*r;
         hrtposes[i].dalt :=hrtposes[i].dalt  + kh*r;
       END;
    
       r:=unit*DEGUNIT; 
       pos.long:=pos.long + dao.long*r;
       pos.lat :=pos.lat  + dao.lat *r;
   
       FOR i:=0 TO wcnt-1 DO
         hrtposes[i].dtime:=ts - hrtposes[i].dtime;
         hrtposes[i].dlong:=pos.long + hrtposes[i].dlong*r;
         hrtposes[i].dlat :=pos.lat  + hrtposes[i].dlat*r;
         hrtposes[i].dalt :=VAL(REAL, altitude) + hrtposes[i].dalt*unit;
   --WrInt(hrtposes[i].dtime, 6); WrFixed(r, 8,15); WrFixed(dao.lat*r, 8,15);
                 WrFixed(hrtposes[i].dlat*360.0/PI2, 8,15);
                WrFixed(hrtposes[i].dlong*360.0/PI2, 8,15);
                WrInt(VAL(INTEGER, hrtposes[i].dalt), 6);WrStrLn("");
   
       END;
       hrtlen:=wcnt;
     END;
   */
   if (wcnt>0UL && hrtposes[0UL].dtime>0UL) {
      tmp = wcnt-1UL;
      i = 1UL;
      if (i<=tmp) for (;; i++) {
         hrtposes[i].dtime += hrtposes[i-1UL].dtime-hrtposes[i-1UL]
                .dtime/((wcnt-i)+1UL);
         if (hrtposes[i-1UL].dtime>0UL) {
            r = X2C_DIVR((float)hrtposes[i].dtime,
                (float)hrtposes[i-1UL].dtime);
            hrtposes[i].dlong = hrtposes[i].dlong+hrtposes[i-1UL].dlong*r;
            hrtposes[i].dlat = hrtposes[i].dlat+hrtposes[i-1UL].dlat*r;
            hrtposes[i].dalt = hrtposes[i].dalt+hrtposes[i-1UL].dalt*r;
         }
         if (i==tmp) break;
      } /* end for */
      r = unit*1.7453292519444E-7f;
      pos->long0 = pos->long0+dao.long0*r;
      pos->lat = pos->lat+dao.lat*r;
      tmp = wcnt-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         hrtposes[i].dtime = ts-hrtposes[i].dtime;
         hrtposes[i].dlong = pos->long0+hrtposes[i].dlong*r;
         hrtposes[i].dlat = pos->lat+hrtposes[i].dlat*r;
         hrtposes[i].dalt = (float)*altitude+hrtposes[i].dalt*unit;
         if (i==tmp) break;
      } /* end for */
      /*WrInt(hrtposes[i].dtime, 6); WrFixed(r, 8,15);
                WrFixed(dao.lat*r, 8,15);
                WrFixed(hrtposes[i].dlat*360.0/PI2, 8,15);
                WrFixed(hrtposes[i].dlong*360.0/PI2, 8,15);
                WrInt(VAL(INTEGER, hrtposes[i].dalt), 6);WrStrLn(""); */
      *hrtlen = wcnt;
   }
/*  IF newcompos THEN compos:=p END; */
/*WrInt(compos, 10);WrStrLn(" composret"); */
} /* end GetHRT() */


static char r91(uint16_t * n, char c)
{
   if ((uint8_t)c<'!' || (uint8_t)c>'|') return 0;
   *n = (uint16_t)((uint32_t)( *n*91U)+((uint32_t)(uint8_t)c-33UL));
   return 1;
} /* end r91() */


static void GetTLM(aprsdecode_TELEMETRY v, char b[], uint32_t b_len)
{
   uint32_t ib;
   uint32_t ia;
   uint32_t j;
   uint32_t i;
   aprsdecode_TELEMETRY t;
   uint32_t tmp;
   ia = 0UL;
   ib = 0UL;
   i = 0UL;
   while (i<b_len-1 && b[i]) {
      if (b[i]=='|') {
         ib = ia;
         ia = i+1UL;
      }
      ++i;
   }
   if (((ib>0UL && (ia-ib&1)) && ia-ib>=5UL) && ia-ib<=15UL) {
      /*& (b[ia]=0C)*/
      i = 0UL;
      j = ib;
      do {
         t[i] = 0U;
         if (!r91(&t[i], b[j]) || !r91(&t[i], b[j+1UL])) return;
         ++i;
         j += 2UL;
      } while (j+1UL<ia);
      tmp = i-1UL;
      ia = 0UL;
      if (ia<=tmp) for (;; ia++) {
         v[ia] = t[ia]+1U;
         if (ia==tmp) break;
      } /* end for */
      /*    b[ib-1]:=0C; */
      aprsstr_Delstr(b, b_len, ib-1UL, (j-ib)+2UL);
   }
} /* end GetTLM() */


static void GetClb(int32_t * clb, char com[], uint32_t com_len)
{
   int32_t i;
   char sig;
   *clb = 0L;
   i = aprsstr_InStr(com, com_len, "Clb=", 5ul);
   if (i>=0L) {
      i += 4L;
      sig = 0;
      if (com[i]=='-') {
         sig = 1;
         ++i;
      }
      while (((i<=(int32_t)(com_len-1) && *clb<1000L) && (uint8_t)
                com[i]>='0') && (uint8_t)com[i]<='9') {
         *clb =  *clb*10L+(int32_t)((uint32_t)(uint8_t)com[i]-48UL);
         ++i;
      }
      if (sig) *clb = -*clb;
   }
} /* end GetClb() */

/* === multiline */

extern void aprsdecode_GetMultiline(char buf[], uint32_t buf_len,
                uint32_t * delfrom, struct aprsdecode_MULTILINE * md)
{
   uint32_t idx;
   uint32_t s;
   uint32_t i;
   char c;
   float v;
   float scale;
   md->size = 0UL;
   md->linetyp = 'a';
   md->filltyp = '0';
   i = 0UL;
   s = 0UL;
   *delfrom = (buf_len-1)+1UL;
   while (i<buf_len-1 && buf[i]) {
      c = buf[i];
      if (s==0UL) {
         if (c==' ') {
            s = 1UL;
            *delfrom = i;
         }
      }
      else if (s==1UL) {
         if (c=='}') s = 2UL;
         else s = 0UL;
      }
      else if (s==2UL) {
         if ((uint8_t)c>='a' && (uint8_t)c<='l') {
            md->linetyp = c;
            s = 3UL;
         }
         else s = 0UL;
      }
      else if (s==3UL) {
         if ((uint8_t)c>='0' && (uint8_t)c<='9') {
            md->filltyp = c;
            s = 4UL;
         }
         else s = 0UL;
      }
      else if (s==4UL) {
         if ((uint8_t)c>='!' && (uint8_t)c<='q') {
            scale = osic_exp((float)((uint32_t)(uint8_t)c-33UL)
                *1.15129255E-1f)*1.7453292519444E-6f;
                /* 10^(x/20)*0.0001 deg */
            s = 5UL;
            idx = 0UL;
         }
         else s = 0UL;
      }
      else if (s==5UL || s==6UL) {
         if ((uint8_t)c>='!' && (uint8_t)c<='z') {
            if (idx<=40UL) {
               v = ((float)(uint32_t)(uint8_t)c-78.0f)*scale;
               /*WrFixed(v/RAD, 5, 10); WrStrLn(" deg"); */
               if (s==5UL) {
                  md->vec[idx].lat = v;
                  s = 6UL;
               }
               else {
                  md->vec[idx].long0 = -v;
                  s = 5UL;
                  ++idx;
               }
            }
            else s = 0UL;
         }
         else if (s==5UL && c=='{') s = 7UL;
         else s = 0UL;
         /*
             ELSIF (s>=7) & (s<=11) THEN
               IF c>" " THEN INC(s) ELSE s:=0 END;
               IF s=12 THEN
                 md.size:=idx;
                 RETURN                                                         (* protocol done *)
         
               END;
         */
         if (s==7UL) {
            md->size = idx;
            return;
         }
      }
      else {
         /* protocol done */
         s = 0UL;
      }
      ++i;
   }
} /* end GetMultiline() */


extern char aprsdecode_ismultiline(char editing)
{
   char s[251];
   uint32_t i;
   struct aprsdecode_MULTILINE ml;
   if (editing) {
      useri_confstr(useri_fRBPOSTYP, s, 251ul);
      if (s[0U]!='L') return 0;
   }
   useri_confstr(useri_fRBCOMMENT, s, 251ul);
   aprsdecode_GetMultiline(s, 251ul, &i, &ml);
   return ml.size>=2UL;
} /* end ismultiline() */

#define aprsdecode_GRIDSIZE 88

#define aprsdecode_RA 3.4377467707849E+7


static void app(char buf[], uint32_t buf_len, float d)
{
   char tmp;
   /*WrFixed(d, 5, 8); WrStr(" "); */
   d = d+78.5f;
   if (d>=33.0f && d<=209.0f) {
      aprsstr_Append(buf, buf_len,
                (char *)(tmp = (char)aprsdecode_trunc(d),&tmp),
                1u/1u);
   }
} /* end app() */


static void EncMultiline(char buf[], uint32_t buf_len,
                struct aprsstr_POSITION * center,
                struct aprsdecode_MULTILINE md)
{
   uint32_t i;
   uint32_t scaler;
   float scale;
   struct aprsstr_POSITION max0;
   struct aprsstr_POSITION min0;
   /* find size */
   struct aprsstr_POSITION * anonym;
   char tmp;
   min0.lat = 6.283185307f;
   min0.long0 = 6.283185307f;
   max0.lat = (-6.283185307f);
   max0.long0 = (-6.283185307f);
   i = 0UL;
   while (i<md.size) {
      { /* with */
         struct aprsstr_POSITION * anonym = &md.vec[i];
         if (anonym->lat<min0.lat) min0.lat = anonym->lat;
         if (anonym->lat>max0.lat) max0.lat = anonym->lat;
         if (anonym->long0<min0.long0) min0.long0 = anonym->long0;
         if (anonym->long0>max0.long0) max0.long0 = anonym->long0;
      }
      ++i;
   }
   center->lat = (max0.lat+min0.lat)*0.5f;
   center->long0 = (max0.long0+min0.long0)*0.5f;
   scale = max0.lat-min0.lat;
   if (max0.long0-min0.long0>scale) scale = max0.long0-min0.long0;
   /*WrFixed(scale, 10, 15); WrStrLn(" scaleo"); */
   if (scale<1.57079632675f) {
      if (scale>0.0f) {
         scaler = aprsdecode_trunc(X2C_DIVR(osic_ln(X2C_DIVR(scale,
                1.5358897417111E-4f)),1.15129255E-1f)+1.0f);
         /*WrInt(scaler, 10);WrStrLn(" scaler"); */
         scale = osic_exp((float)scaler*1.15129255E-1f)
                *1.5358897417111E-4f; /* 10^(x/20)*0.0001 deg */
         /*WrFixed(scale, 10, 15); WrStrLn(" scale"); */
         scale = X2C_DIVR(88.0f,scale);
      }
      else {
         scaler = 0UL;
         scale = 0.0f;
      }
      aprsstr_Assign(buf, buf_len, " }", 3ul);
      aprsstr_Append(buf, buf_len, (char *) &md.linetyp, 1u/1u);
      aprsstr_Append(buf, buf_len, (char *) &md.filltyp, 1u/1u);
      aprsstr_Append(buf, buf_len,
                (char *)(tmp = (char)(33UL+scaler),&tmp), 1u/1u);
      i = 0UL;
      while (i<md.size) {
         app(buf, buf_len, (md.vec[i].lat-center->lat)*scale);
         app(buf, buf_len, (center->long0-md.vec[i].long0)*scale);
         ++i;
      }
      /*    scale:=ln(scale/(0.0001*RAD))/(2.3025851/20.0); */
      aprsstr_Append(buf, buf_len, "{", 2ul);
   }
   else {
      /*
        lati :=trunc(ABS(center.lat) *RA);
        longi:=trunc(ABS(center.long)*RA);
        latd :=VAL(CARDINAL, lati ) MOD 100;
        longd:=VAL(CARDINAL, longi) MOD 100;
        lati :=lati -latd;
        longi:=longi-longd;
       
      WrInt(latd,0); WrStrLn(" latd");
      */
      /*WrStrLn(" <-appended"); */
      buf[0UL] = 0;
   }
} /* end EncMultiline() */


extern void aprsdecode_appendmultiline(struct aprsstr_POSITION pos)
{
   char h[251];
   char cso[251];
   char cs[251];
   struct aprsdecode_MULTILINE ml;
   struct aprsstr_POSITION center;
   uint32_t i;
   char msgc;
   /* make absolute */
   struct aprsstr_POSITION * anonym;
   useri_confstr(useri_fRBPOS, cs, 251ul);
   if (cs[0U]) aprstext_deganytopos(cs, 251ul, &center);
   else aprsstr_posinval(&center);
   useri_confstr(useri_fRBCOMMENT, cso, 251ul);
   aprsdecode_GetMultiline(cso, 251ul, &i, &ml);
   if (i<=250UL) cso[i] = 0;
   if (ml.size<2UL) aprsdecode_click.insreplaceline = 1;
   /*WrStr("<<<");WrStr(cso);WrStrLn(">>> csodel"); */
   i = 0UL;
   while (i<ml.size) {
      { /* with */
         struct aprsstr_POSITION * anonym = &ml.vec[i];
         anonym->lat = anonym->lat+center.lat;
         anonym->long0 = anonym->long0+center.long0;
      }
      ++i;
   }
   if (ml.size>40UL) ml.size = 41UL;
   if (aprsdecode_click.polilinecursor>=ml.size) {
      aprsdecode_click.polilinecursor = ml.size;
   }
   if (aprspos_posvalid(pos)) {
      if (aprsdecode_click.insreplaceline) {
         if (ml.size<40UL) {
            /* insert line */
            i = 40UL;
            while (i>0UL && i>aprsdecode_click.polilinecursor) {
               ml.vec[i] = ml.vec[i-1UL];
               --i;
            }
            ml.vec[aprsdecode_click.polilinecursor] = pos;
            if (aprsdecode_click.polilinecursor>=ml.size) {
               ++aprsdecode_click.polilinecursor;
            }
            ++ml.size;
         }
      }
      else ml.vec[aprsdecode_click.polilinecursor] = pos;
   }
   else {
      /* delete line */
      i = aprsdecode_click.polilinecursor;
      while (i<40UL) {
         ml.vec[i] = ml.vec[i+1UL];
         ++i;
      }
      if (ml.size>0UL) --ml.size;
   }
   if (ml.size<=33UL) msgc = 'b';
   else if (ml.size<40UL) msgc = 'r';
   else msgc = 'e';
   aprsstr_IntToStr((int32_t)ml.size, 0UL, h, 251ul);
   aprsstr_Append(h, 251ul, " elements", 10ul);
   useri_textautosize(0L, 0L, 6UL, 2UL, msgc, h, 251ul);
   EncMultiline(cs, 251ul, &center, ml);
   aprsstr_Append(cso, 251ul, cs, 251ul);
   useri_AddConfLine(useri_fRBCOMMENT, 1U, cso, 251ul);
   /*WrInt(ml.size, 10); WrStr("<<<");WrStr(cso);WrStrLn(">>> csoapp"); */
   aprstext_postostr(center, '3', cs, 251ul);
   useri_AddConfLine(useri_fRBPOS, 1U, cs, 251ul); /* update new center */
   aprsdecode_tracenew.winevent = 2000UL;
} /* end appendmultiline() */


extern void aprsdecode_modmultiline(uint32_t n)
{
   struct aprsstr_POSITION pos;
   struct aprsdecode_MULTILINE ml;
   uint32_t i;
   char s[251];
   uint32_t v;
   if (n==1UL) {
      /* del line */
      aprsstr_posinval(&pos);
      aprsdecode_appendmultiline(pos);
   }
   else {
      useri_confstr(useri_fRBCOMMENT, s, 251ul);
      aprsdecode_GetMultiline(s, 251ul, &i, &ml);
      if (n==2UL) {
         /* colour */
         if (ml.size>0UL) {
            v = (uint32_t)(uint8_t)s[i+2UL];
            v += 3UL;
            if (v>108UL) v -= 12UL;
            s[i+2UL] = (char)v;
         }
      }
      else if (n==3UL) {
         /* colour */
         if (ml.size>0UL) {
            v = (uint32_t)(uint8_t)s[i+2UL];
            ++v;
            if (v%3UL==1UL) v -= 3UL;
            s[i+2UL] = (char)v;
         }
      }
      else if (n==4UL) {
         /* poliline/poligone */
         if (ml.size>0UL) {
            s[i+3UL] = (char)((uint32_t)(uint8_t)s[i+3UL]+1UL);
            if ((uint8_t)s[i+3UL]<'0' || (uint8_t)s[i+3UL]>'9') {
               s[i+3UL] = '0';
            }
         }
      }
      useri_AddConfLine(useri_fRBCOMMENT, 1U, s, 251ul);
   }
   aprsdecode_tracenew.winevent = 2000UL;
} /* end modmultiline() */

/* === multiline */
/*
PROCEDURE checksymb(symt, symb:CHAR):BOOLEAN;
                (* true for bad symbol *)
BEGIN
(*
  RETURN (symt<>"/") & (symt<>"\") & ((InStr("#&0>AW^_acnsuvz", symb)<0)
  OR ((symt<"0") OR (symt>"9")) & ((symt<"A") OR (symt>"Z")))
*)
  RETURN (symt<>"/") & (symt<>"\") & (((symt<"0") OR (symt>"9"))
  & ((symt<"A") OR (symt>"Z")) & ((symt<"a") OR (symt>"z")))
END checksymb;
*/
static aprsdecode_SET256 _cnst = {0x00000000UL,0x03FF8000UL,0x17FFFFFEUL,
                0x07FFFFFEUL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL};

extern int32_t aprsdecode_Decode(char buf[], uint32_t buf_len,
                struct aprsdecode_DAT * dat)
{
   uint32_t ia;
   uint32_t iv;
   uint32_t micedest;
   uint32_t len;
   uint32_t p;
   uint32_t i;
   char thirdparty;
   memset((char *)dat,(char)0,sizeof(struct aprsdecode_DAT));
   dat->speed = X2C_max_longcard;
   dat->altitude = X2C_max_longint;
   NoWX(&dat->wx);
   len = 0UL;
   while ((len<buf_len-1 && buf[len]) && buf[len]!='\015') ++len;
   if (len==0UL || buf[0UL]=='#') return -1L;
   i = 0UL;
   do {
      /* skip 3rd party headers */
      p = i;
      while (i<len && buf[i]!=':') ++i;
      i += 2UL;
   } while (!(i>len || buf[i-1UL]!='}'));
   thirdparty = p!=0UL;
   i = 0UL;
   while (buf[p]!='>') {
      /* source call */
      if (p>=len || i>8UL) return -1L;
      dat->symcall[i] = buf[p];
      ++p;
      ++i;
   }
   memcpy(dat->srcall,dat->symcall,9u);
   ++p;
   micedest = p;
   i = 0UL;
   while (buf[p]!=',' && buf[p]!=':') {
      /* dest call */
      if (p>=len || i>8UL) return -1L;
      dat->dstcall[i] = buf[p];
      ++p;
      ++i;
   }
   iv = 0UL;
   while (buf[p]==',') {
      if (iv>9UL) iv = 9UL;
      ++p;
      if (((((thirdparty && buf[p]=='T') && buf[p+1UL]=='C')
                && buf[p+2UL]=='P') && buf[p+3UL]=='I')
                && buf[p+4UL]=='P' || ((buf[p]=='q' && buf[p+1UL]=='A')
                && buf[p+2UL]!='C') && iv<9UL) dat->igatep = iv+1UL;
      if (dat->igatep==iv) dat->igaterawp = p;
      i = 0UL;
      while (buf[p]!=',' && buf[p]!=':') {
         if (p>=len) return -1L;
         if (buf[p]=='*') {
            if (dat->igatep==0UL && dat->hbitp==0UL) dat->hbitp = iv+1UL;
         }
         else if (i<=8UL) {
            dat->viacalls[iv][i] = buf[p];
            ++i;
         }
         ++p;
      }
      /*    IF dat.igatep=iv THEN dat.igatelen:=p-dat.igaterawp END; */
      if (dat->igatep==iv) dat->igatelen = i;
      ++iv;
   }
   if (dat->hbitp>0UL || iv>(uint32_t)(dat->igatep!=0UL)
                *2UL && viaghost(dat->viacalls[0UL], 9ul)) {
      /* maybe not direct */
      i = 0UL;
      while ((i<9UL && i<dat->hbitp) && iscall(dat->viacalls[i])) ++i;
      dat->hbitp = i+1UL;
   }
   else dat->hbitp = 0UL;
   /*WrInt(dat.hbitp, 1);WrStrLn("=h"); */
   if (p+1UL>=len || buf[p]!=':') return -1L;
   ++p;
   dat->payload = p;
   dat->typc = buf[dat->payload];
   switch ((unsigned)dat->typc) {
   case '\034':
   case '\035':
   case '\'':
   case '`':
      dat->type = aprsdecode_MICE;
      break;
   case '!':
   case '$':
   case '/':
   case '=':
   case '@':
      dat->type = aprsdecode_POS;
      break;
   case '#':
   case '*':
      dat->type = aprsdecode_WETH;
      break;
   case '_':
      dat->type = aprsdecode_PWETH;
      dat->sym = '_'; /* positionless wx */
      dat->symt = '/';
      break;
   case ':':
      dat->type = aprsdecode_MSG;
      break;
   case ';':
      dat->type = aprsdecode_OBJ;
      break;
   case ')':
      dat->type = aprsdecode_ITEM;
      break;
   case '<':
      dat->type = aprsdecode_SCAP;
      break;
   case '>':
      dat->type = aprsdecode_STAT;
      break;
   case 'T':
      dat->type = aprsdecode_TELE;
      break;
   case '?':
      dat->type = aprsdecode_QUERY;
      break;
   default:;
      dat->type = aprsdecode_UNKNOWN;
      break;
   } /* end switch */
   ++p;
   if (dat->type==aprsdecode_OBJ) {
      /* object */
      if (buf[p+9UL]=='_' || buf[p+9UL]=='*') {
         memcpy(dat->objectfrom,dat->symcall,9u);
         i = 0UL;
         while (i<=8UL) {
            if (p+i>=len) return -1L;
            dat->symcall[i] = buf[p+i];
            ++i;
         }
         if (buf[p+i]=='_') dat->objkill = '1';
         else if (buf[p+i]!='*') dat->type = aprsdecode_UNKNOWN;
         dat->timestamp = buf[p+16UL];
         while (i>0UL && (uint8_t)dat->symcall[i-1UL]<=' ') {
            dat->symcall[i-1UL] = 0;
            --i;
         }
      }
      else dat->type = aprsdecode_UNKNOWN;
   }
   else if (dat->typc==')') {
      /* item */
      memcpy(dat->objectfrom,dat->symcall,9u);
      i = 0UL;
      while (buf[p+i]!='!' && buf[p+i]!='_') {
         if (p+i>=len) return -1L;
         if (i<=8UL) dat->symcall[i] = buf[p+i];
         ++i;
      }
      if (i<=8UL) dat->symcall[i] = 0;
      if (buf[p+i]=='_') dat->objkill = '1';
   }
   else if (dat->typc==':') {
      /* msg, bulletin */
      if (buf[p+9UL]==':') {
         i = 0UL;
         while (i<9UL && p<len) {
            if (i<=8UL && (uint8_t)buf[p]>' ') dat->msgto[i] = buf[p];
            ++i;
            ++p;
         }
         if (X2C_STRCMP(dat->msgto,9u,dat->symcall,9u)==0) {
            dat->type = aprsdecode_TELE; /* msg to itself */
         }
         ++p;
         i = 0UL;
         if ((buf[p]=='r' && buf[p+1UL]=='e') && buf[p+2UL]=='j') {
            dat->ackrej = aprsdecode_MSGREJ; /* reject */
         }
         else if ((buf[p]=='a' && buf[p+1UL]=='c') && buf[p+2UL]=='k') {
            dat->ackrej = aprsdecode_MSGACK; /* ack */
         }
         if (dat->ackrej>aprsdecode_MSGMSG) {
            p += 3UL;
            while ((i<=4UL && (uint8_t)buf[p]>' ') && p<len) {
               dat->acktext[i] = buf[p];
               ++i;
               ++p;
            }
         }
         else {
            i = p;
            ia = len;
            while (i<len) {
               /* find last { */
               if (buf[i]=='{') ia = i;
               ++i;
            }
            i = 0UL;
            while (i<=66UL && p<ia) {
               /* message text */
               dat->msgtext[i] = buf[p];
               ++i;
               ++p;
            }
            dat->ackrej = aprsdecode_MSGMSG;
            if (buf[p]=='{') {
               /* message ack */
               i = 0UL;
               ++p;
               while ((i<=4UL && (uint8_t)buf[p]>' ') && p<len) {
                  dat->acktext[i] = buf[p];
                  ++i;
                  ++p;
               }
            }
         }
      }
   }
   else if (dat->typc=='?') {
      /* general query */
      i = 0UL;
      while ((i<9UL && (uint8_t)buf[p]>' ') && p<len) {
         if (i<=8UL) dat->msgto[i] = buf[p];
         ++i;
         ++p;
      }
   }
   /*  dat.course:=MAX(CARDINAL); */
   dat->course = 0UL;
   if (X2C_IN((int32_t)dat->type,12,0xCEU)) {
      aprspos_GetPos(&dat->pos, &dat->speed, &dat->course, &dat->altitude,
                &dat->sym, &dat->symt, buf, buf_len, micedest, dat->payload,
                dat->comment0, 256ul, &dat->postyp);
      aprspos_GetSym(dat->dstcall, 9ul, &dat->sym, &dat->symt);
      if (!X2C_INL((uint8_t)dat->symt,256,_cnst)) {
         dat->symt = 0; /* wrong symbol */
         dat->sym = 0;
      }
      if (dat->postyp=='A' && dat->type==aprsdecode_OBJ) {
         /* area object */
         dat->areasymb.typ = (char)((dat->course/100UL)%10UL+48UL);
         dat->areasymb.color = (uint8_t)(dat->speed/100UL);
         dat->areasymb.dpos.lat = sqr((float)(dat->course%100UL))
                *1.454441043287E-6f;
         dat->areasymb.dpos.long0 = sqr((float)(dat->speed%100UL))
                *1.454441043287E-6f;
         if (dat->areasymb.typ=='6') {
            dat->areasymb.dpos.long0 = -dat->areasymb.dpos.long0;
                /* line to left down */
         }
         dat->pos.lat = dat->pos.lat-dat->areasymb.dpos.lat;
         dat->pos.long0 = dat->pos.long0+dat->areasymb.dpos.long0;
                /* shift pos to middle of area */
         dat->speed = 0UL;
         dat->course = 0UL;
      }
      if (dat->type==aprsdecode_OBJ || dat->type==aprsdecode_ITEM) {
         aprsdecode_GetMultiline(dat->comment0, 256ul, &i, &dat->multiline);
         if (dat->multiline.size>0UL) dat->postyp = 'L';
      }
      GetHRT(&dat->pos, &dat->altitude, &dat->speed, dat->comment0, 256ul,
                dat->hrtposes, 32ul, &dat->hrtlen, &dat->hrttime);
      GetTLM(dat->tlmvalues, dat->comment0, 256ul);
      GetClb(&dat->climb, dat->comment0, 256ul);
   }
   else {
      i = 0UL;
      p = dat->payload+(uint32_t)(dat->type!=aprsdecode_UNKNOWN);
      while (p<len && i<=255UL) {
         dat->comment0[i] = buf[p];
         ++p;
         ++i;
      }
   }
   if (X2C_IN((int32_t)dat->type,12,0xCCU)) {
      if (dat->sym=='_' || dat->type==aprsdecode_PWETH) {
         if (dat->type==aprsdecode_PWETH) {
            aprsstr_Delstr(dat->comment0, 256ul, 0UL, 8UL);
         }
         GetWX(&dat->wx, &dat->course, &dat->speed, dat->comment0, 256ul, 0);
      }
      else if (dat->sym=='@') {
         GetWX(&dat->wx, &dat->course, &dat->speed, dat->comment0, 256ul, 1);
      }
      /*
            IF dat.wx.storm>WXNORMAL THEN
              dat.areasymb.typ:="5";                      (* circle *)
              dat.areasymb.color:=5;
              dat.areasymb.dpos.lat :=0.006;
              dat.areasymb.dpos.long:=0.0;
            END;
      */
      if (dat->course<1UL || dat->course>360UL) dat->course = 0UL;
   }
   return 0L;
} /* end Decode() */


static char getbeaconparm(char s[], uint32_t s_len,
                uint32_t * time0, char * port)
/* time:port:rawframe */
{
   uint32_t i;
   *port = s[0UL];
   if ((uint8_t)*port<' ' || s[1UL]!=':') return 0;
   *time0 = 0UL;
   i = 2UL;
   while ((uint8_t)s[i]>='0' && (uint8_t)s[i]<='9') {
      *time0 = ( *time0*10UL+(uint32_t)(uint8_t)s[i])-48UL;
      ++i;
   }
   if (s[i]!=':') return 0;
   aprsstr_Delstr(s, s_len, 0UL, i+1UL);
   return 1;
} /* end getbeaconparm() */


extern void aprsdecode_getbeaconname(char raw[], uint32_t raw_len,
                char name[], uint32_t name_len, char symb[],
                uint32_t symb_len, char * isobj,
                char * isdel, char * isbad)
/* item object else src name */
{
   struct aprsdecode_DAT dat;
   char port;
   uint32_t time0;
   X2C_PCOPY((void **)&raw,raw_len);
   *isobj = 0;
   *isdel = 0;
   *isbad = 1;
   name[0UL] = 0;
   if (getbeaconparm(raw, raw_len, &time0, &port)) {
      beaconmacros(raw, raw_len, "", 1ul, "", 1ul, 0);
      if (aprsdecode_Decode(raw, raw_len, &dat)==0L) {
         aprsstr_Assign(name, name_len, dat.symcall, 9ul);
         aprsstr_Assign(symb, symb_len, (char *) &dat.symt, 1u/1u);
         aprsstr_Append(symb, symb_len, (char *) &dat.sym, 1u/1u);
         *isobj = dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM;
         *isdel = dat.objkill=='1';
         *isbad = (uint8_t)dat.symt<' ' || (uint8_t)name[0UL]<=' ';
      }
      else *isbad = 1;
   }
   X2C_PFREE(raw);
} /* end getbeaconname() */


extern void aprsdecode_extractbeacon(char raw[], uint32_t raw_len,
                char withparm, char macros)
/* monitor line to beacon config */
{
   struct aprsdecode_DAT dat;
   char port;
   uint32_t i;
   uint32_t time0;
   int32_t j;
   char s[1000];
   X2C_PCOPY((void **)&raw,raw_len);
   if (withparm) {
      if (getbeaconparm(raw, raw_len, &time0, &port)) {
         if (macros) beaconmacros(raw, raw_len, "", 1ul, "", 1ul, 0);
         aprsstr_IntToStr((int32_t)time0, 0UL, s, 1000ul);
         useri_AddConfLine(useri_fRBTIME, 1U, s, 1000ul);
         useri_AddConfLine(useri_fRBPORT, 1U, (char *) &port, 1u/1u);
      }
      else {
         useri_say("\012beacon syntax wrong, delete line!\012", 36ul, 4UL,
                'e');
      }
   }
   if (aprsdecode_Decode(raw, raw_len, &dat)==0L) {
      if (dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM) {
         aprsstr_Assign(s, 1000ul, dat.symcall, 9ul);
      }
      else {
         useri_confstr(useri_fMYCALL, s, 1000ul);
         if (aprstext_callwrong(s, 1000ul)) {
            useri_xerrmsg("need legal \'My Call\' to build Beacon", 37ul);
         }
      }
      useri_AddConfLine(useri_fRBNAME, 1U, s, 1000ul);
      if (aprspos_posvalid(dat.pos)) {
         if (dat.postyp=='A' && dat.type==aprsdecode_OBJ) {
            /* area object */
            dat.pos.lat = dat.pos.lat+dat.areasymb.dpos.lat;
            dat.pos.long0 = dat.pos.long0-dat.areasymb.dpos.long0;
                /* shift pos to upleft of area */
         }
         aprstext_postostr(dat.pos, '3', s, 1000ul);
         useri_AddConfLine(useri_fRBPOS, 1U, s, 1000ul);
      }
      useri_AddConfLine(useri_fRBCOMMENT, 1U, dat.comment0, 256ul);
      s[0U] = dat.symt;
      s[1U] = dat.sym;
      s[2U] = 0;
      useri_AddConfLine(useri_fRBSYMB, 1U, s, 1000ul);
      useri_AddConfLine(useri_fRBSPEED, 1U, "", 1ul);
      useri_AddConfLine(useri_fRBDIR, 1U, "", 1ul);
      if ((uint8_t)dat.areasymb.typ>='0') {
         /*      IntToStr(dat.areasymb.color*100+trunc(sqrt(dat.areasymb.dpos.long/(RAD/200.0/60.0)
                )), 1, s); */
         aprsstr_IntToStr((int32_t)((uint32_t)
                dat.areasymb.color*100UL+aprsdecode_trunc(osic_sqrt(X2C_DIVR((float)
                fabs(dat.areasymb.dpos.long0),1.454441043287E-6f)))), 1UL, s,
                 1000ul);
         useri_AddConfLine(useri_fRBSPEED, 1U, s, 1000ul);
         aprsstr_IntToStr((int32_t)(((uint32_t)(uint8_t)
                dat.areasymb.typ-48UL)
                *100UL+aprsdecode_trunc(osic_sqrt(X2C_DIVR((float)
                fabs(dat.areasymb.dpos.lat),1.454441043287E-6f)))), 1UL, s,
                1000ul);
         useri_AddConfLine(useri_fRBDIR, 1U, s, 1000ul);
      }
      else if (dat.speed>0UL && dat.speed<2147483647UL) {
         aprsstr_IntToStr((int32_t)aprsdecode_trunc((float)
                dat.speed*1.852f), 1UL, s, 1000ul);
         useri_AddConfLine(useri_fRBSPEED, 1U, s, 1000ul);
         s[0] = 0;
         if (dat.course>0UL && dat.course<=360UL) {
            aprsstr_IntToStr((int32_t)(dat.course%360UL), 1UL, s, 1000ul);
         }
         useri_AddConfLine(useri_fRBDIR, 1U, s, 1000ul);
      }
      if (dat.altitude<X2C_max_longint) {
         aprsstr_IntToStr(dat.altitude, 1UL, s, 1000ul);
      }
      else {
         s[0U] = 0;
      }
      useri_AddConfLine(useri_fRBALT, 1U, s, 1000ul);
      s[1U] = 0;
      s[0U] = 'b';
      if (dat.type==aprsdecode_OBJ) {
         if (dat.timestamp=='h') s[0U] = 'H';
         else s[0U] = 'O';
         if (dat.objkill=='1') s[0U] = 'P';
      }
      else if (dat.type==aprsdecode_ITEM) {
         if (dat.objkill=='1') s[0U] = 'J';
         else s[0U] = 'I';
      }
      useri_AddConfLine(useri_fRBTYP, 1U, s, 1000ul);
      useri_AddConfLine(useri_fRBPOSTYP, 1U, (char *) &dat.postyp,
                1u/1u);
      if (withparm) {
         s[0U] = 0;
         j = aprsstr_InStr(dat.dstcall, 9ul, "-", 2ul);
                /* move dest ssid to viapath */
         if (j>=0L) {
            aprsstr_Assign(s, 1000ul, dat.dstcall, 9ul);
            aprsstr_Delstr(s, 1000ul, 0UL, (uint32_t)j);
            dat.dstcall[j] = 0;
         }
         if (dat.type==aprsdecode_MICE) strncpy(dat.dstcall,"APLM01",9u);
         useri_AddConfLine(useri_fRBDEST, 1U, dat.dstcall, 9ul);
         useri_AddConfLine(useri_fRBNAME, 1U, dat.symcall, 9ul);
         i = 0UL;
         while (i<=9UL && dat.viacalls[i][0UL]) {
            /* make via path */
            if (s[0U]) aprsstr_Append(s, 1000ul, ",", 2ul);
            aprsstr_Append(s, 1000ul, dat.viacalls[i], 9ul);
            ++i;
         }
         useri_AddConfLine(useri_fRBPATH, 1U, s, 1000ul);
      }
      else useri_AddConfLine(useri_fRBDEST, 1U, "APLM01", 7ul);
   }
   X2C_PFREE(raw);
} /* end extractbeacon() */


extern void aprsdecode_importbeacon(void)
{
   /* op last pos frame to beacon config */
   aprsdecode_pOPHIST opf;
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pl;
   aprsdecode_pFRAMEHIST pf;
   struct aprsdecode_DAT dat;
   if (useri_beaconediting && aprsdecode_click.entries>0UL) {
      opf = aprsdecode_click.table[aprsdecode_click.selected].opf;
      if (opf) {
         op = aprsdecode_ophist0;
         while (op && X2C_STRCMP(opf->call,9u,op->call,9u)) op = op->next;
         if (op) {
            pl = 0;
            pf = op->frames;
            while (pf) {
               if (aprspos_posvalid(pf->vardat->pos)) pl = pf;
               pf = pf->next;
            }
            if ((pl && pl->vardat) && aprsdecode_Decode(pl->vardat->raw,
                500ul, &dat)==0L) {
               aprsdecode_extractbeacon(pl->vardat->raw, 500ul, 0, 0);
               useri_beaconed = 1;
               useri_textautosize(0L, 0L, 5UL, 5UL, 'b', "Object Cloned to Be\
acon Editor!", 32ul);
            }
         }
      }
   }
} /* end importbeacon() */


static void beepmsg(char isack)
{
   if (useri_configon(useri_fBEEPMSG)) {
      aprsdecode_beeplim(100L, (uint32_t)useri_conf2int(useri_fBEEPMSG,
                (uint32_t)isack*2UL, 20L, 8000L, 1000L),
                (uint32_t)useri_conf2int(useri_fBEEPMSG,
                (uint32_t)isack*2UL+1UL, 0L, 5000L, 250L));
   }
} /* end beepmsg() */


static void popupmessage(const char from[], uint32_t from_len,
                const char to[], uint32_t to_len, const char txt[],
                 uint32_t txt_len, const char ack[],
                uint32_t ack_len, uint32_t time0, char port,
                char isquery0, char kill,
                struct aprsstr_POSITION pos, const char item[],
                uint32_t item_len)
{
   aprsdecode_pMSGFIFO pl;
   aprsdecode_pMSGFIFO pm;
   int32_t cnt;
   pm = aprsdecode_msgfifo0;
   pl = 0;
   while (pm) {
      /* delete older same messages */
      if (((aprsstr_StrCmp(pm->from, 9ul, from,
                from_len) && aprsstr_StrCmp(pm->to, 9ul, to,
                to_len)) && aprsstr_StrCmp(pm->txt, 67ul, txt,
                txt_len)) && aprsstr_StrCmp(pm->ack, 5ul, ack, ack_len)) {
         if (pl==0) aprsdecode_msgfifo0 = pm->next;
         else pl->next = pm->next;
         osic_free((char * *) &pm, sizeof(struct aprsdecode_MSGFIFO));
         pm = aprsdecode_msgfifo0;
         pl = 0;
      }
      else {
         pl = pm;
         pm = pm->next;
      }
   }
   osic_alloc((char * *) &pm, sizeof(struct aprsdecode_MSGFIFO));
   if (pm==0) {
      osi_WrStrLn("msg out of memory", 18ul);
      return;
   }
   aprsstr_Assign(pm->from, 9ul, from, from_len);
   aprsstr_Assign(pm->to, 9ul, to, to_len);
   aprsstr_Assign(pm->txt, 67ul, txt, txt_len);
   aprsstr_Assign(pm->ack, 5ul, ack, ack_len);
   aprsstr_Assign(pm->itemname, 9ul, item, item_len);
   pm->itempos = pos;
   pm->time0 = time0;
   pm->port = port;
   pm->query = isquery0;
   pm->next = 0;
   pm->deleteitem = kill;
   cnt = 1L;
   if (aprsdecode_msgfifo0==0) aprsdecode_msgfifo0 = pm;
   else {
      cnt = 2L;
      pl = aprsdecode_msgfifo0;
      while (pl->next) {
         pl = pl->next;
         ++cnt;
      }
      pl->next = pm;
   }
   if (useri_nextmsg==0L) {
      if (useri_configon(useri_fPOPUPMSG)) useri_nextmsg = -cnt;
   }
   else useri_nextmsg = -labs(useri_nextmsg);
} /* end popupmessage() */


extern void aprsdecode_getactack(aprsdecode_MONCALL call, char ack[],
                uint32_t ack_len)
/* get actual message ack */
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pf;
   struct aprsdecode_DAT dat;
   char mid[2];
   char aa[2];
   char ok0;
   aprsdecode_MONCALL my;
   ack[0UL] = 0;
   op = findop(call, 0);
   if (op) {
      useri_confstr(useri_fMYCALL, my, 9ul);
      pf = op->frames;
      while (pf) {
         if (((aprsdecode_Decode(pf->vardat->raw, 500ul,
                &dat)>=0L && dat.type==aprsdecode_MSG)
                && dat.ackrej==aprsdecode_MSGMSG) && X2C_STRCMP(dat.msgto,9u,
                my,9u)==0) {
            ok0 = replyack(aa, 2ul, mid, 2ul, dat.acktext, 5ul);
            if (mid[0U]) aprsstr_Assign(ack, ack_len, mid, 2ul);
         }
         pf = pf->next;
      }
   }
} /* end getactack() */


static char ch(uint32_t n)
{
   if (n<10UL) n += 48UL;
   else if (n<36UL) n += 55UL;
   else n += 61UL;
   return (char)n;
} /* end ch() */


extern void aprsdecode_acknumstr(char aa[], uint32_t aa_len,
                uint32_t n)
{
   aa[0UL] = ch((n/62UL)%62UL);
   aa[1UL] = ch(n%62UL);
   if (aa_len-1>=2UL) aa[2UL] = 0;
} /* end acknumstr() */


static char sendtxmsg(uint32_t acknum, const aprsdecode_MONCALL to,
                const char txt[], uint32_t txt_len, char port,
                char rfonly, uint32_t ackcnt, char errm[],
                uint32_t errm_len)
{
   uint32_t i;
   aprsdecode_FRAMEBUF s;
   aprsdecode_FRAMEBUF rfs;
   char am[51];
   char h1[51];
   char h[51];
   char tmp;
   if (ackcnt==0UL) strncpy(am,"Ack ",51u);
   else strncpy(am,"Msg ",51u);
   errm[0UL] = 0;
   if (to[0UL]==0) return 0;
   useri_confstr(useri_fMYCALL, rfs, 512ul);
   if (aprstext_callwrong(rfs, 512ul)) {
      aprsstr_Assign(errm, errm_len, "Msg not sent (no valid Mycall)", 31ul);
      return 0;
   }
   aprsstr_Append(rfs, 512ul, ">", 2ul);
   if (rfonly) useri_confstr(useri_fMSGRFDEST, h, 51ul);
   else useri_confstr(useri_fMSGNETDEST, h, 51ul);
   aprsstr_Append(rfs, 512ul, h, 51ul);
   if (h[0U]==0) {
      aprsstr_Assign(errm, errm_len, am, 51ul);
      aprsstr_Append(errm, errm_len, "not sent (no Port set)", 23ul);
      return 0;
   }
   if (port=='A' || (uint8_t)port>='1' && (uint8_t)port<='9') {
      /* radio so add via path */
      useri_confstr(useri_fMSGPATH, h, 51ul);
      if ((uint8_t)h[0U]>='1' && (uint8_t)h[0U]<='7') {
         aprsstr_Append(rfs, 512ul, "-", 2ul);
         aprsstr_Append(rfs, 512ul, (char *) &h[0U], 1u/1u);
      }
      else if (h[0U]) {
         aprsstr_Append(rfs, 512ul, ",", 2ul);
         aprsstr_Append(rfs, 512ul, h, 51ul);
      }
   }
   aprsstr_Append(rfs, 512ul, ":", 2ul);
   strncpy(s,":",512u);
   aprsstr_Append(s, 512ul, to, 9ul);
   for (i = aprsstr_Length(to, 9ul); i<=8UL; i++) {
      aprsstr_Append(s, 512ul, " ", 2ul);
   } /* end for */
   aprsstr_Append(s, 512ul, ":", 2ul);
   if (ackcnt==0UL) aprsstr_Append(s, 512ul, "ack", 4ul);
   aprsstr_Append(s, 512ul, txt, txt_len);
   if (acknum>0UL) {
      aprsstr_Append(s, 512ul, "{", 2ul);
      aprsdecode_acknumstr(h, 51ul, acknum);
      aprsstr_Append(s, 512ul, h, 51ul);
      aprsstr_Append(s, 512ul, "}", 2ul);
      aprsdecode_getactack(to, h, 51ul);
      if (h[0U]) aprsstr_Append(s, 512ul, h, 51ul);
   }
   /*WrInt(ORD(rfonly), 10);WrStrLn(port); */
   aprsstr_Append(rfs, 512ul, s, 512ul);
   for (i = 0UL; i<=3UL; i++) {
      /* try the rf ports */
      if ((useri_configon((uint8_t)(38UL+i))
                && (port=='A' || port==(char)(i+49UL))) && Sendudp(rfs,
                512ul, i, 0UL)>0L) {
         memcpy(h,am,51u);
         if (ackcnt==1UL) aprsstr_Append(h, 51ul, "sent", 5ul);
         else {
            aprsstr_Append(h, 51ul, "resent(", 8ul);
            aprsstr_IntToStr((int32_t)ackcnt, 0UL, h1, 51ul);
            aprsstr_Append(h, 51ul, h1, 51ul);
            aprsstr_Append(h, 51ul, ")", 2ul);
         }
         aprsstr_Append(h, 51ul, " to ", 5ul);
         aprsstr_Append(h, 51ul, to, 9ul);
         aprsstr_Append(h, 51ul, " on Port ", 10ul);
         aprsstr_Append(h, 51ul, (char *)(tmp = (char)(i+49UL),&tmp),
                 1u/1u);
         useri_say(h, 51ul, 2UL, 'b');
         return 1;
      }
   } /* end for */
   /* make it better */
   if (!rfonly && port=='N') {
      SendNet(aprsdecode_tcpsocks, "", 1ul, s, 512ul, 1, errm, errm_len);
      if (errm[0UL]) {
         memcpy(h,am,51u);
         aprsstr_Append(h, 51ul, "not sent ", 10ul);
         aprsstr_Append(h, 51ul, errm, errm_len);
         aprsstr_Assign(errm, errm_len, h, 51ul);
      }
      else {
         memcpy(h,am,51u);
         aprsstr_Append(h, 51ul, "(re)sent to ", 13ul);
         aprsstr_Append(h, 51ul, to, 9ul);
         aprsstr_Append(h, 51ul, " on Net", 8ul);
         useri_say(h, 51ul, 2UL, 'b');
      }
      return 1;
   }
   else {
      aprsstr_Assign(errm, errm_len, am, 51ul);
      aprsstr_Append(errm, errm_len, "not sent (delete Msg or open outgoing P\
ort)", 44ul);
      return 0;
   }
   return 0;
} /* end sendtxmsg() */


static void iteminmsg(const char from[], uint32_t from_len,
                const char to[], uint32_t to_len, const char txt[],
                 uint32_t txt_len, struct aprsstr_POSITION * pos,
                char name[], uint32_t name_len, char * del)
/* test for and draw item in a message */
{
   char h[1000];
   char s[1000];
   struct aprsdecode_DAT dat;
   aprsstr_posinval(pos);
   aprsstr_Assign(s, 1000ul, from, from_len);
   aprsstr_Append(s, 1000ul, ">", 2ul);
   aprsstr_Append(s, 1000ul, to, to_len);
   aprsstr_Append(s, 1000ul, ":", 2ul);
   aprsstr_Append(s, 1000ul, txt, txt_len);
   if ((aprsdecode_Decode(s, 1000ul,
                &dat)>=0L && dat.type==aprsdecode_ITEM)
                && aprspos_posvalid(dat.pos)) {
      aprsstr_Assign(h, 1000ul, "0:0:", 5ul);
      aprsstr_Append(h, 1000ul, s, 1000ul);
      aprsdecode_drawbeacon(h, 1000ul);
      *pos = dat.pos;
      aprsstr_Assign(name, name_len, dat.symcall, 9ul);
      *del = dat.objkill=='1';
   }
} /* end iteminmsg() */


static void ackmsg(const char from[], uint32_t from_len,
                const char rep[], uint32_t rep_len, uint8_t rej)
{
   aprsdecode_pTXMESSAGE pm;
   char bb[2];
   char aa[2];
   /*
     WrStr("replyack:");
     WrStr(from);
     WrStr("|");
     WrStr(rep);
     WrStr("|");
     WrLn;
   */
   if (rej==aprsdecode_MSGMSG) {
      if (!replyack(bb, 2ul, aa, 2ul, rep, rep_len)) return;
   }
   else {
      /* message only */
      aprsstr_Assign(bb, 2ul, rep, rep_len);
   }
   pm = aprsdecode_txmessages;
   while (pm) {
      if (pm->acktime==0UL && X2C_STRCMP(from,from_len,pm->to,9u)==0) {
         aprsdecode_acknumstr(aa, 2ul, pm->acknum);
         if (X2C_STRCMP(aa,2u,bb,2u)==0) {
            /* message acked now */
            pm->acktime = aprsdecode_realtime;
            pm->rej = rej==aprsdecode_MSGREJ;
            useri_refresh = 1;
            /*WrInt(ORD(rej), 10); WrStrLn("=rej"); */
            beepmsg(1);
         }
      }
      pm = pm->next;
   }
} /* end ackmsg() */

#define aprsdecode_QUERYRESPONSEWINDOW 30
/* time after sending a msg to display already received query response */


static void showmsg(const char from[], uint32_t from_len,
                const char to[], uint32_t to_len, const char txt[],
                 uint32_t txt_len, const char ack[],
                uint32_t ack_len, uint8_t rej, char my,
                char query, char port)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST opf;
   aprsdecode_ACKTEXT aack;
   aprsdecode_ACKTEXT rep;
   aprsdecode_MONCALL call;
   struct aprsdecode_DAT dat;
   struct aprsstr_POSITION itempos;
   aprsdecode_MONCALL itemname;
   char delitem;
   char s[501];
   opf = 0;
   if (ack[0UL]) {
      aprsstr_Assign(call, 9ul, from, from_len);
      op = findop(call, 1); /* find sender of msg */
      if (op) {
         opf = op->frames;
         for (;;) {
            if (opf==0) break;
            if ((((aprsdecode_Decode(opf->vardat->raw, 500ul,
                &dat)>=0L && dat.type==aprsdecode_MSG)
                && dat.ackrej==aprsdecode_MSGMSG) && X2C_STRCMP(dat.msgto,9u,
                to,to_len)==0) && X2C_STRCMP(txt,txt_len,dat.msgtext,
                67u)==0) {
               if (ack[0UL]==0) {
                  if (lastmsgtx+30UL<aprsdecode_realtime) {
                     break; /* we do not wait for a query response */
                  }
               }
               else if (X2C_STRCMP(dat.acktext,5u,ack,
                ack_len)==0 || replyack(rep, 5ul, aack, 5ul, dat.acktext,
                5ul) && X2C_STRCMP(aack,5u,ack,ack_len)==0) {
                  break; /* we have this msg got before */
               }
            }
            opf = opf->next;
         }
      }
   }
   if (opf==0) {
      if (my || useri_configon(useri_fMSGALLSSID) && (!aprsstr_StrCmp(from,
                from_len, to, to_len) || useri_configon(useri_fPASSSELFMSG))) {
         if (!useri_configon(useri_fPOPUPMSG)) {
            if (query) strncpy(s,"Query from ",501u);
            else strncpy(s,"Message from ",501u);
            aprsstr_Append(s, 501ul, from, from_len);
            aprsstr_Append(s, 501ul, " to ", 5ul);
            aprsstr_Append(s, 501ul, to, to_len);
            aprsstr_Append(s, 501ul, " (click on M Button)", 21ul);
            useri_textautosize(0L, 5L, 6UL, 0UL, 'y', s, 501ul);
         }
         iteminmsg(from, from_len, to, to_len, txt, txt_len, &itempos,
                itemname, 9ul, &delitem);
         popupmessage(from, from_len, to, to_len, txt, txt_len, ack, ack_len,
                 aprsdecode_realtime, port, query, delitem, itempos,
                itemname, 9ul);
         ++aprsdecode_tracenew.winevent;
         beepmsg(0);
      }
   }
   else if (aprsdecode_verb) {
      osi_WrStr("seen msg:", 10ul);
      osi_WrStr(from, from_len);
      osi_WrStr(">", 2ul);
      osi_WrStr(to, to_len);
      osi_WrStr("|", 2ul);
      osi_WrStr(txt, txt_len);
      osi_WrStr("|", 2ul);
      osi_WrStr(ack, ack_len);
      osi_WrStr("|", 2ul);
      osi_WrStr((char *) &port, 1u/1u);
      osic_WrLn();
   }
} /* end showmsg() */


static char isquery(const struct aprsdecode_DAT dat, char port)
{
   uint32_t j;
   uint32_t i;
   char err[201];
   char wdata[201];
   char key[201];
   char path[201];
   char s[201];
   /*  IF dat.msgtext[0]<>"?" THEN RETURN FALSE END; */
   aprsstr_Assign(path, 201ul, dat.symcall, 9ul);
   aprsstr_Append(path, 201ul, ">", 2ul);
   aprsstr_Append(path, 201ul, dat.dstcall, 9ul);
   i = 0UL;
   while (dat.viacalls[i][0UL]) {
      aprsstr_Append(path, 201ul, ",", 2ul);
      aprsstr_Append(path, 201ul, dat.viacalls[i], 9ul);
      if (i+2UL==dat.hbitp) aprsstr_Append(path, 201ul, "*", 2ul);
      ++i;
   }
   i = 0UL;
   for (;;) {
      useri_confstrings(useri_fQUERYS, i, 0, s, 201ul);
      if (s[0U]==0) break;
      j = (uint32_t)aprsstr_InStr(s, 201ul, ":", 2ul);
      if (j>0UL) {
         aprsstr_Assign(key, 201ul, s, 201ul);
         key[j] = 0;
         aprsstr_Delstr(s, 201ul, 0UL, j+1UL);
         if (aprsstr_InStr(dat.msgtext, 67ul, key, 201ul)==0L) {
            aprsstr_Assign(wdata, 201ul, dat.msgtext, 67ul);
            aprsstr_Delstr(wdata, 201ul, 0UL, aprsstr_Length(key, 201ul));
            beaconmacros(s, 201ul, path, 201ul, wdata, 201ul, 1);
            if (s[0U]==0) return 1;
            if (!sendtxmsg(0UL, dat.symcall, s, 201ul, port, port!='N', 1UL,
                err, 201ul)) {
               useri_xerrmsg(err, 201ul);
               return 0;
            }
            else {
               useri_xerrmsg(err, 201ul);
               return 1;
            }
         }
      }
      ++i;
   }
   /*WrStrLn(port);WrStrLn(dat.symcall);WrStrLn(dat.msgtext); */
   return 0;
} /* end isquery() */


static void getmessage(aprsdecode_pTCPSOCK cp, uint32_t up,
                const struct aprsdecode_DAT dat)
/* if first: popup, if full match & lastack>10s: send ack */
{
   uint32_t i;
   char void0;
   char query;
   char fit;
   char my[100];
   char h[100];
   char port;
   /*    my:MONCALL; */
   char ch0;
   useri_confstr(useri_fMYCALL, my, 100ul);
   if (aprstext_callwrong(my, 100ul)) return;
   aprsstr_Assign(h, 100ul, dat.msgto, 9ul);
   fit = 0;
   i = 0UL;
   while (h[i]==my[i] && h[i]) {
      if (h[i]=='-' || my[i]=='-') fit = 1;
      ++i;
   }
   if (i==0UL) return;
   ch0 = my[i];
   if (h[i]==0 && ch0=='-' || h[i]=='-' && ch0==0) fit = 1;
   if (cp) port = 'N';
   else port = (char)(up+48UL);
   if (ch0==0 && h[i]==0) {
      /*  WrStr("my message "); */
      if (dat.ackrej==aprsdecode_MSGMSG) {
         /*FOR i:=0 TO HIGH(dat.acktext) DO WrInt(ORD(dat.acktext[i]), 4);
                END; WrStrLn("=ack"); */
         query = isquery(dat, port);
         showmsg(dat.symcall, 9ul, dat.msgto, 9ul, dat.msgtext, 67ul,
                dat.acktext, 5ul, dat.ackrej, 1, query, port);
         if (!query && dat.acktext[0UL]) {
            void0 = sendtxmsg(0UL, dat.symcall, dat.acktext, 5ul, port,
                port!='N', 0UL, h, 100ul);
            useri_xerrmsg(h, 100ul);
         }
      }
      if (dat.acktext[0UL]) {
         ackmsg(dat.symcall, 9ul, dat.acktext, 5ul, dat.ackrej);
      }
   }
   else if (fit && dat.ackrej==aprsdecode_MSGMSG) {
      /*  WrStr("other ssid message "); */
      showmsg(dat.symcall, 9ul, dat.msgto, 9ul, dat.msgtext, 67ul,
                dat.acktext, 5ul, dat.ackrej, 0, 0, port);
   }
} /* end getmessage() */

#define aprsdecode_SETPORT 2

#define aprsdecode_DEL 6

#define aprsdecode_KICK 5

#define aprsdecode_RESTART 4

#define aprsdecode_HOLD 3

#define aprsdecode_EDIT 1


extern void aprsdecode_deltxmsg(uint32_t cmd, uint32_t n)
/* delete/edit tx message number */
{
   aprsdecode_pTXMESSAGE po;
   aprsdecode_pTXMESSAGE pm;
   pm = aprsdecode_txmessages;
   if (pm) {
      if (cmd==6UL || cmd==1UL) {
         if (n==0UL) aprsdecode_txmessages = aprsdecode_txmessages->next;
         else {
            do {
               po = pm;
               pm = pm->next;
               --n;
            } while (!(n==0UL || pm==0));
            if (pm) po->next = pm->next;
         }
         if (pm) {
            if (cmd==1UL) {
               useri_AddConfLine(useri_fMSGTEXT, 0U, pm->msgtext, 67ul);
               useri_AddConfLine(useri_fMSGTO, 0U, pm->to, 9ul);
               useri_AddConfLine(useri_fMSGPORT, 0U, (char *) &pm->port,
                1u/1u);
            }
            osic_free((char * *) &pm,
                sizeof(struct aprsdecode_TXMESSAGE));
         }
      }
      else {
         while (n>0UL && pm) {
            pm = pm->next;
            --n;
         }
         if (pm && pm->acktime==0UL) {
            if (cmd==5UL) pm->txtime = 0UL;
            else if (cmd==4UL) {
               pm->txtime = 0UL;
               pm->acktime = 0UL;
               pm->txcnt = 0UL;
               pm->heard = 0;
            }
            else if (cmd==3UL) pm->txtime = X2C_max_longcard;
            else if (cmd==2UL) {
               useri_confstr(useri_fMSGPORT, (char *) &pm->port, 1u/1u);
            }
         }
      }
   }
} /* end deltxmsg() */


extern void aprsdecode_makemsg(char ack)
{
   aprsdecode_pTXMESSAGE pn;
   aprsdecode_pTXMESSAGE pm;
   char po;
   char ht[101];
   char hm[101];
   char mes[101];
   int32_t i;
   char c;
   struct aprsdecode_TXMESSAGE * anonym;
   int32_t tmp;
   mes[0U] = 0;
   useri_confstr(useri_fMSGPORT, (char *) &po, 1u/1u);
   if (po==' ' || X2C_CAP(po)=='A') po = 'A';
   else if (X2C_CAP(po)=='N' || X2C_CAP(po)=='I') po = 'N';
   else if ((uint8_t)po<'0' || (uint8_t)po>'9') {
      strncpy(mes,"need Port where to send",101u);
   }
   useri_confstr(useri_fMSGTO, ht, 101ul); /* no destination call */
   if (ht[0U]==0) strncpy(mes,"need call to send to",101u);
   useri_confstr(useri_fMSGTEXT, hm, 101ul);
   if (hm[0U]==0) {
      /* empty message line */
      if (!sentemptymsg) {
         strncpy(mes,"send empty line? if so, enter again",101u);
         sentemptymsg = 1;
      }
      else sentemptymsg = 0;
   }
   else sentemptymsg = 0;
   pn = aprsdecode_txmessages; /* look for same msg pending */
   while (pn) {
      if (((((ack && pn->acknum>0UL) && pn->acktime==0UL)
                && aprsstr_StrCmp(pn->to, 9ul, ht,
                101ul)) && aprsstr_StrCmp(pn->msgtext, 67ul, hm,
                101ul)) && pn->port==po) {
         strncpy(mes,"same message pending, try [kick] or delete",101u);
      }
      pn = pn->next;
   }
   if (mes[0U]==0) {
      osic_alloc((char * *) &pm, sizeof(struct aprsdecode_TXMESSAGE));
      if (pm==0) Err("out of memory", 14ul);
      memset((char *)pm,(char)0,sizeof(struct aprsdecode_TXMESSAGE));
      { /* with */
         struct aprsdecode_TXMESSAGE * anonym = pm;
         anonym->port = po;
         useri_confstr(useri_fMSGTO, anonym->to, 9ul);
         aprsstr_Caps(anonym->to, 9ul);
         useri_confstr(useri_fMSGTEXT, anonym->msgtext, 67ul);
         tmp = (int32_t)aprsstr_Length(anonym->msgtext, 67ul)-1L;
         i = 0L;
         if (i<=tmp) for (;; i++) {
            c = anonym->msgtext[i];
            if (c=='|' || c=='~') {
               /* OR (c=cMSGACK) */
               anonym->msgtext[i] = ' ';
            }
            if (i==tmp) break;
         } /* end for */
         if (ack) {
            if (msgmid==0UL) {
               msgmid = (aprsdecode_realtime%68400UL)/30UL;
                /* init msg number with 30s daytime */
            }
            msgmid = (msgmid+1UL)%3843UL;
            anonym->acknum = msgmid+1UL; /* 0 is query with no ack */
         }
      }
      pn = aprsdecode_txmessages;
      if (pn==0) aprsdecode_txmessages = pm;
      else {
         while (pn->next) pn = pn->next;
         pn->next = pm;
      }
      useri_clrmsgtext("", 1ul);
      useri_textautosize(0L, 5L, 6UL, 5UL, 'b', "Msg Stored", 11ul);
   }
   useri_xerrmsg(mes, 101ul);
} /* end makemsg() */

#define aprsdecode_MAXMSGTXTIME 3600

#define aprsdecode_MAXMSGRETRY 20

#define aprsdecode_MAXNOACKRETRY 1

#define aprsdecode_MSGNOTXTIME 10

#define aprsdecode_MSGTXTIME 15

/*    my:MONCALL; */

static void sendmsg(void)
{
   aprsdecode_pTXMESSAGE pn;
   aprsdecode_pTXMESSAGE pm;
   uint32_t t;
   uint32_t maxret;
   uint32_t tc;
   char port;
   char sent;
   char err[101];
   if (lastmsgtx>aprsdecode_realtime) lastmsgtx = aprsdecode_realtime;
   /*  confstr(fMYCALL, my); */
   /*  IF (lastmsgtx+MSGNOTXTIME<realtime) & (Length(my)>=3) THEN */
   if (lastmsgtx+10UL<aprsdecode_realtime) {
      err[0U] = 0;
      pm = aprsdecode_txmessages;
      for (;;) {
         if (pm==0) break;
         if (pm->acknum>0UL) maxret = 20UL;
         else maxret = 1UL;
         if ((pm->acktime==0UL && pm->txtime!=X2C_max_longcard)
                && (pm->txcnt<maxret || pm->txtime==0UL)) {
            tc = pm->txcnt;
            t = 15UL;
            for (;;) {
               if (tc<=1UL) break;
               t += t;
               --tc;
               if (t>3600UL) {
                  t = 3600UL;
                  break;
               }
            }
            if (pm->txtime+t<=aprsdecode_realtime) {
               pn = aprsdecode_txmessages;
               while (pn!=pm && ((((pn->acknum==0UL || pn->acktime>0UL)
                || pn->txcnt>=20UL) || !aprsstr_StrCmp(pn->to, 9ul, pm->to,
                9ul)) || pn->txtime==X2C_max_longcard)) pn = pn->next;
               if (pn==pm) {
                  /* not older unacked msg to this call */
                  port = pm->port;
                  sent = 0;
                  if ((pm->txcnt<=(uint32_t)pm->heard || pm->txtime==0UL)
                && (port=='N' || port=='A')) {
                     if (sendtxmsg(pm->acknum, pm->to, pm->msgtext, 67ul,
                'N', 0, pm->txcnt+1UL, err, 101ul) && err[0U]==0) sent = 1;
                  }
                  if (port=='A' || (uint8_t)port>='1' && (uint8_t)
                port<='9') {
                     if (sendtxmsg(pm->acknum, pm->to, pm->msgtext, 67ul,
                port, sent || port!='A', pm->txcnt+1UL, err,
                101ul) && err[0U]==0) sent = 1;
                     else if (sent || pm->txcnt>0UL) err[0U] = 0;
                  }
                  if (sent) {
                     ++pm->txcnt;
                     pm->txtime = aprsdecode_realtime;
                     lastmsgtx = aprsdecode_realtime;
                     useri_refresh = 1;
                     break; /* send 1 msg a time */
                  }
               }
            }
         }
         pm = pm->next;
      }
      useri_xerrmsg(err, 101ul);
   }
} /* end sendmsg() */


static void setmsgheard(const aprsdecode_MONCALL us)
/* set heard flag in tx msg if user heard now */
{
   aprsdecode_pTXMESSAGE pm;
   pm = aprsdecode_txmessages;
   for (;;) {
      if (pm==0) break;
      if ((pm->acktime==0UL && pm->txcnt==1UL) && aprsstr_StrCmp(pm->to, 9ul,
                 us, 9ul)) pm->heard = 1;
      pm = pm->next;
   }
} /* end setmsgheard() */


extern aprsdecode_pOPHIST aprsdecode_selop(void)
{
   if (aprsdecode_click.entries>0UL) {
      return aprsdecode_click.table[aprsdecode_click.selected].opf;
   }
   else return 0;
   return 0;
} /* end selop() */

/*
PROCEDURE objsender(op:pOPHIST; VAR s:ARRAY OF CHAR);
                (* find (last) sender of an item/object *)
VAR dat:DAT;
    f:pFRAMEHIST;
BEGIN
  s[0]:=0C;
  IF (op<>NIL) & (ISOBJECT IN op^.drawhints) THEN
    f:=op^.frames;
    IF f<>NIL THEN
      WHILE f^.next<>NIL DO f:=f^.next END;
                (* goto last frame *)
      IF (Decode(f^.vardat^.raw, dat)>=0) & ((dat.type=OBJ) OR (dat.type=ITEM))
      THEN Assign(s, dat.objectfrom) END;
    END;
  END;
END objsender;
*/

extern void aprsdecode_objsender(aprsdecode_pOPHIST op, char s[],
                uint32_t s_len)
/* find (last) sender of an item/object */
{
   struct aprsdecode_DAT dat;
   aprsdecode_pFRAMEHIST f;
   aprsdecode_pOPHIST o;
   s[0UL] = 0;
   if (op) {
      if ((0x2U & op->drawhints)) {
         f = op->frames;
         if (f) {
            while (f->next) f = f->next;
            if (aprsdecode_Decode(f->vardat->raw, 500ul,
                &dat)>=0L && (dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM)
                ) aprsstr_Assign(s, s_len, dat.objectfrom, 9ul);
         }
      }
      else {
         o = aprsdecode_ophist0;
         for (;;) {
            if (o==0) break;
            f = o->frames;
            if (((f && aprsdecode_Decode(f->vardat->raw, 500ul,
                &dat)>=0L) && (dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM)
                ) && X2C_STRCMP(dat.objectfrom,9u,op->call,9u)==0) {
               aprsstr_Assign(s, s_len, op->call, 9ul);
               break;
            }
            o = o->next;
         }
      }
   }
} /* end objsender() */

#define aprsdecode_MAXDELAY 30
/* add seconds to time */


static void timespeed(aprsdecode_pFRAMEHIST lastf,
                struct aprsstr_POSITION pos1, uint32_t time1,
                uint8_t * err)
{
   float d;
   uint32_t t;
   if (((((*err&0xFU)!=0U || lastf==0) || (lastf->nodraw&0xFU)!=0U)
                || !aprspos_posvalid(lastf->vardat->pos))
                || !aprspos_posvalid(pos1)) return;
   d = aprspos_distance(lastf->vardat->pos, pos1);
   /*
     IF d>50.0 THEN INCL(err, eDIST) END;
   */
   if (lastf->time0>=time1) t = 0UL;
   else t = time1-lastf->time0;
   if (X2C_DIVR(d*3600.0f,(float)(t+30UL))>maxspeed) *err |= 0x10U;
} /* end timespeed() */

#define aprsdecode_MINHOP 20.0

#define aprsdecode_HOPPART 0.5
/*      MINSPIKE=0.02;                               (* min distance to mark as spike *) */

#define aprsdecode_SPIKEANGLE 30.0
/* min deg spanned by 3 waypoints for no SPIKE */


/* 1 wayout, delete before, 2 wayout join last good with last else delete before */
static void joinchunk(aprsdecode_pOPHIST op)
{
   aprsdecode_pFRAMEHIST lbad;
   aprsdecode_pFRAMEHIST bad;
   aprsdecode_pFRAMEHIST lllpf;
   aprsdecode_pFRAMEHIST llpf;
   aprsdecode_pFRAMEHIST lpf;
   aprsdecode_pFRAMEHIST pf;
   aprsdecode_pVARDAT v;
   float kml;
   float km;
   float ang;
   float waylen;
   waylen = (float)op->lastkmh*0.03f+20.0f;
   lllpf = 0;
   llpf = 0;
   lpf = 0;
   pf = op->frames;
   lbad = 0;
   while (pf) {
      v = pf->vardat;
      if (aprspos_posvalid(v->pos) && (pf->nodraw&0xCU)==0U) {
         if (lpf) {
            km = aprspos_distance(lpf->vardat->pos, v->pos);
            if (llpf && km!=0.0f) {
               /* we have 3 waypoints */
               kml = aprspos_distance(lpf->vardat->pos, llpf->vardat->pos);
               if (kml!=0.0f) {
                  ang = (float)fabs(aprspos_azimuth(lpf->vardat->pos,
                v->pos)-aprspos_azimuth(lpf->vardat->pos,
                llpf->vardat->pos));
                  if (ang>180.0f) ang = 360.0f-ang;
                  if (ang<30.0f) {
                     /* a spike */
                     if (lllpf==0) {
                        /* first spike */
                        lpf->nodraw = lpf->nodraw&~0x10U|0x2U;
                        pf->nodraw &= ~0x10U;
                        lllpf = llpf;
                     }
                     else {
                        /* second spike */
                        if (km>aprspos_distance(llpf->vardat->pos, v->pos)) {
                           lpf->nodraw = lpf->nodraw&~0x10U|0x2U;
                           pf->nodraw &= ~0x10U;
                           llpf->nodraw &= ~0x2U; /* the second is bad */
                        }
                        lllpf = 0;
                     }
                  }
                  else lllpf = 0;
               }
            }
            if (aprsdecode_maxhop>0.0f) {
               if (km>waylen) lbad = pf;
               else {
                  waylen = waylen+km*0.5f;
                /* inc max hop if there are some steps */
                  if (waylen>aprsdecode_maxhop) waylen = aprsdecode_maxhop;
               }
            }
         }
         if ((pf->nodraw&0xCU)==0U) {
            /* use only until now ok waypoints */
            llpf = lpf;
            lpf = pf;
         }
      }
      pf = pf->next;
   }
   if (lbad) {
      /* there is a dist */
      bad = 0;
      lpf = 0;
      pf = op->frames;
      for (;;) {
         /* find last long hop in track */
         if (pf==0) break;
         if (aprspos_posvalid(pf->vardat->pos) && (pf->nodraw&0x1EU)==0U) {
            if (lpf && aprspos_distance(lpf->vardat->pos,
                pf->vardat->pos)>waylen) bad = pf;
            lpf = pf;
         }
         if (lpf==lbad) break;
         pf = pf->next;
      }
      if (bad) {
         /* hide path until last bad */
         pf = op->frames;
         while (pf!=bad) {
            pf->nodraw |= 0x20U;
            pf = pf->next;
         }
      }
   }
} /* end joinchunk() */


extern void aprsdecode_makelogfn(char s[], uint32_t s_len)
{
   aprsdecode_FILENAME fn;
   useri_confstr(useri_fLOGWFN, fn, 1024ul);
   filepath(fn, 1024ul);
   if (fn[0UL]) aprsstr_Append(fn, 1024ul, "/", 2ul);
   aprsstr_Append(fn, 1024ul, s, s_len);
   aprsstr_Assign(s, s_len, fn, 1024ul);
/*WrStr("<"); WrStr(s);WrStrLn(">"); */
} /* end makelogfn() */


static void inwindow(aprsdecode_pOPHIST op, const char rawbuf[],
                uint32_t rawbuf_len)
{
   uint32_t matchlen;
   uint32_t j;
   uint32_t ii;
   uint32_t i;
   char in;
   aprsdecode_pFRAMEHIST pf;
   char wc[100];
   aprsdecode_FILENAME fn;
   aprsdecode_pVARDAT pv;
   uint8_t doset;
   struct aprsstr_POSITION * anonym;
   in = 0;
   aprsdecode_tracenew.call[0UL] = 0;
   doset = 0U;
   if (op) {
      setmsgheard(op->call);
      pf = op->frames;
      while (pf && !in) {
         /* find waypoint inside screen */
         pv = pf->vardat;
         if (aprspos_posvalid(pv->pos)) {
            { /* with */
               struct aprsstr_POSITION * anonym = &pv->pos;
               if (((anonym->long0>aprsdecode_tracenew.winpos0.long0 && anonym->lat<aprsdecode_tracenew.winpos0.lat)
                 && anonym->long0<aprsdecode_tracenew.winpos1.long0)
                && anonym->lat>aprsdecode_tracenew.winpos1.lat) in = 1;
            }
         }
         pf = pf->next;
      }
      if (in) ++aprsdecode_tracenew.winevent;
      /*find user in watchlist */
      /*  IF posvalid(op^.lastpos) THEN */
      j = 0UL;
      matchlen = 0UL;
      for (;;) {
         useri_confstrings(useri_fWATCH, j, 1, wc, 100ul);
         if (wc[0U]==0) break;
         i = 0UL;
         ii = 0UL;
         for (;;) {
            if (((i>99UL || ii>8UL) || wc[i]==0) || op->call[ii]==0) {
               if ((ii>8UL || op->call[ii]==0) && (wc[i]==0 || wc[i]=='*')) {
                  if (i>=matchlen) {
                     memcpy(aprsdecode_tracenew.call,op->call,9u);
                     doset = useri_confflags(useri_fWATCH, j);
                     matchlen = i;
                  }
               }
               break;
            }
            if (wc[i]=='*') {
               if (wc[i+1UL]) ++i;
            }
            else {
               if (X2C_CAP(wc[i])!=X2C_CAP(op->call[ii])) break;
               ++i;
            }
            ++ii;
         }
         ++j;
      }
      /*  END; */
      aprsdecode_tracenew.beep = (char)(doset&1);
      aprsdecode_tracenew.follow = (char)(doset/2U&1);
      if ((!aprsdecode_tracenew.follow && aprsdecode_click.watchmhop)
                && X2C_STRCMP(op->call,9u,aprsdecode_click.mhop,9u)==0) {
         aprsdecode_tracenew.follow = 1;
         memcpy(aprsdecode_tracenew.call,op->call,9u);
      }
      /*WrStrLn(op^.call); WrStrLn(click.mhop);
                WrInt(ORD(click.watchmhop), 1);
                WrInt(ORD(tracenew.follow), 1); WrStrLn(""); */
      if ((doset/4U&1)) {
         /* append to watchcall file */
         aprsstr_Assign(fn, 1024ul, op->call, 9ul);
         alfanum(fn, 1024ul); /* remove bad chars */
         /*
               confstr(fLOGWFN, fn);
               filepath(fn);
               IF fn[0]<>0C THEN Append(fn, DIRSEP) END;
               Append(fn, wc);
         */
         aprsdecode_makelogfn(fn, 1024ul);
         if (!wrlog(rawbuf, rawbuf_len, aprsdecode_realtime, fn, 1024ul,
                0L)) poplogerr(op->call, 9ul);
      }
   }
} /* end inwindow() */


extern uint32_t aprsdecode_finddup(aprsdecode_pFRAMEHIST pf,
                aprsdecode_pFRAMEHIST frame)
/* return delay time if dupe found */
{
   struct aprsstr_POSITION apos;
   uint32_t ret;
   uint32_t dupedelay;
   uint32_t speed;
   uint32_t course;
   int32_t altitude;
   struct aprsdecode_DAT dat;
   /* search from begin to 1 frame before */
   struct aprsdecode_VARDAT * anonym;
   apos = frame->vardat->pos;
   ret = 0UL;
   if ((pf!=frame && aprspos_posvalid(apos))
                && aprsdecode_Decode(frame->vardat->raw, 500ul, &dat)>=0L) {
      speed = dat.speed;
      course = dat.course;
      altitude = dat.altitude;
      dupedelay = frame->time0-aprsdecode_lums.dupemaxtime;
      while (pf->next!=frame) {
         { /* with */
            struct aprsdecode_VARDAT * anonym = pf->vardat;
            if (pf->time0>=dupedelay && (frame->vardat==pf->vardat || ((((apos.lat==anonym->pos.lat && apos.long0==anonym->pos.long0)
                 && aprsdecode_Decode(anonym->raw, 500ul,
                &dat)>=0L) && speed==dat.speed) && course==dat.course)
                && altitude==dat.altitude)) {
               /* dupe pos found */
               ret = frame->time0-pf->time0;
            }
            else if (ret>0UL) return ret;
         }
         /* pos changed so we have real dupe */
         pf = pf->next;
      }
   }
   return 0UL;
} /* end finddup() */

#define aprsdecode_POLY13 0x112D 

#define aprsdecode_TOPBIT 12


static void initcrc12(void)
{
   uint16_t remainder;
   uint32_t bit;
   uint32_t dividend;
   for (dividend = 0UL; dividend<=255UL; dividend++) {
      remainder = X2C_LSH((uint16_t)dividend,16,4);
      for (bit = 8UL;; bit--) {
         if ((0x1000U & remainder)) {
            remainder = X2C_LSH(remainder,16,1)^0x112DU;
         }
         else remainder = X2C_LSH(remainder,16,1);
         if (bit==0UL) break;
      } /* end for */
      crctable12[dividend] = remainder&0x1FFFU;
   } /* end for */
} /* end initcrc12() */

typedef uint8_t * RAW;

#define aprsdecode_TOPBIT0 12


static void crca12(const RAW data, int32_t len, uint16_t * rem)
{
   int32_t i;
   int32_t tmp;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      *rem = X2C_LSH(*rem,16,8)^crctable12[(uint16_t)(X2C_LSH(*rem,16,
                -5)&0xFFU)]^(uint16_t)data[i];
      if (i==tmp) break;
   } /* end for */
} /* end crca12() */

struct _0;


struct _0 {
   aprsdecode_pFRAMEHIST wayp;
   uint16_t next;
};


static char lookup(uint32_t * wridx, struct _0 table[4096],
                uint16_t index[8192], aprsdecode_pFRAMEHIST waypoint)
{
   uint32_t sum;
   uint16_t ojp;
   uint16_t jp;
   uint16_t ip;
   struct aprsdecode_DAT dat1;
   struct aprsdecode_DAT dat;
   uint16_t rem;
   struct aprsdecode_FRAMEHIST * anonym;
   if (aprsdecode_Decode(waypoint->vardat->raw, 500ul,
                &dat)>=0L && aprspos_posvalid(dat.pos)) {
      rem = 0U;
      crca12((RAW) &dat.speed, 2L, &rem);
      crca12((RAW) &dat.course, 2L, &rem);
      crca12((RAW) &dat.altitude, 2L, &rem);
      crca12((RAW) &dat.pos.lat, (int32_t)sizeof(float), &rem);
      crca12((RAW) &dat.pos.long0, (int32_t)sizeof(float), &rem);
      sum = (uint32_t)(rem&0x1FFFU);
      ip = index[sum];
      /*WrInt(sum, 10);WrInt(ip, 10);WrStrLn("=ip"); */
      if (ip) {
         /* starts with 1, not found */
         --ip;
         jp = ip;
         for (;;) {
            { /* with */
               struct aprsdecode_FRAMEHIST * anonym = table[jp].wayp;
               if (anonym->vardat==waypoint->vardat || ((((anonym->vardat->pos.lat==dat.pos.lat && anonym->vardat->pos.long0==dat.pos.long0)
                 && aprsdecode_Decode(anonym->vardat->raw, 500ul,
                &dat1)>=0L) && dat1.speed==dat.speed)
                && dat1.course==dat.course) && dat1.altitude==dat.altitude) {
                  if (anonym->time0+aprsdecode_lums.dupemaxtime>waypoint->time0)
                 return 1;
                  else break;
               }
            }
            ojp = jp;
            jp = table[jp].next;
            if (jp>4095U || ojp<=jp==((uint32_t)ojp<*wridx==(uint32_t)
                jp<*wridx)) {
               /*WrInt(ORD(jp>HIGH(table)), 2); WrInt(ojp, 6); WrInt(jp, 6);
                WrInt(wridx, 6); WrStr("o"); */
               break;
            }
         }
      }
      else {
         /*INC(collis); */
         ip = 65535U;
      }
      index[sum] = (uint16_t)(*wridx+1UL);
      table[*wridx].wayp = waypoint;
      table[*wridx].next = ip;
      *wridx = *wridx+1UL&4095UL;
   }
   return 0;
} /* end lookup() */


static void Checktrack(aprsdecode_pOPHIST op, aprsdecode_pFRAMEHIST lastf)
{
   aprsdecode_pFRAMEHIST f;
   aprsdecode_pFRAMEHIST frame;
   struct aprsstr_POSITION last2;
   struct aprsstr_POSITION last;
   struct aprsstr_POSITION apos;
   uint32_t starttime;
   char cmp;
   uint32_t wridx;
   uint16_t index[8192];
   struct _0 table[4096];
   struct aprsdecode_OPHIST * anonym;
   if (lastf) frame = lastf->next;
   else {
      op->margin0.long0 = 3.1415926535f;
      op->margin0.lat = (-1.57079632675f);
      op->margin1.long0 = (-3.1415926535f);
      op->margin1.lat = 1.57079632675f;
      frame = op->frames;
      while (frame) {
         frame->vardat->refcnt = 0UL;
         frame = frame->next;
      }
      frame = op->frames;
      while (frame) {
         ++frame->vardat->refcnt;
         frame = frame->next;
      }
      frame = op->frames;
   }
   if (frame==0) return;
   starttime = frame->time0; /* from this waypoint dupes will be set */
   f = op->frames;
   while (f && f->time0+aprsdecode_lums.dupemaxtime<starttime) f = f->next;
   memset((char *)index,(char)0,16384UL);
   wridx = 0UL;
   lastf = 0;
   cmp = 0;
   aprsstr_posinval(&last);
   aprsstr_posinval(&last2);
   while (f) {
      if (f==frame) cmp = 1;
      apos = f->vardat->pos;
      if (aprspos_posvalid(apos)) {
         if (cmp) {
            { /* with */
               struct aprsdecode_OPHIST * anonym = op;
               if (apos.long0<anonym->margin0.long0) {
                  anonym->margin0.long0 = apos.long0;
               }
               if (apos.long0>anonym->margin1.long0) {
                  anonym->margin1.long0 = apos.long0;
               }
               if (apos.lat>anonym->margin0.lat) {
                  anonym->margin0.lat = apos.lat;
               }
               if (apos.lat<anonym->margin1.lat) {
                  anonym->margin1.lat = apos.lat;
               }
            }
            f->nodraw &= ~0x8U;
         }
         if (lookup(&wridx, table, index, f) && cmp) {
            if (apos.lat!=last.lat || apos.long0!=last.long0) {
               if (apos.lat!=last2.lat || apos.long0!=last2.long0) {
                  f->nodraw |= 0x8U;
               }
            }
         }
         if (maxspeed>0.0f) timespeed(lastf, apos, f->time0, &f->nodraw);
         if (!(f->nodraw&~0x14U) || !aprspos_posvalid(op->lastpos)) {
            op->lastpos = apos;
         }
         last2 = last;
         last = apos;
      }
      else f->nodraw |= 0x40U;
      lastf = f;
      f = f->next;
   }
   joinchunk(op);
} /* end Checktrack() */


static char strcmp0(const char a[], uint32_t a_len,
                const char b[], uint32_t b_len)
/* faster but needs 0 termination */
{
   uint32_t i;
   i = 0UL;
   for (;;) {
      if (a[i]!=b[i]) return 0;
      if (a[i]==0) return 1;
      ++i;
   }
   return 0;
} /* end strcmp() */


static char callfilt(uint8_t v, const char str[],
                uint32_t str_len, char fullmatch)
{
   uint32_t j;
   uint32_t i;
   char s[31];
   if (str[0UL]==0) return 0;
   i = 0UL;
   for (;;) {
      useri_confstrings(v, i, 0, s, 31ul);
      if (s[0U]==0) return 0;
      s[30U] = 0;
      /*  IF fullmatch THEN IF StrCmp(s, str) THEN RETURN TRUE END;  */
      if (fullmatch) {
         j = 0UL;
         for (;;) {
            if (s[j]=='*') {
               if (s[j+1UL]==0 || str[j]==0) return 1;
            }
            else {
               if (s[j]!=str[j]) break;
               if (str[j]==0) return 1;
            }
            ++j;
            if (j>str_len-1) return 1;
         }
      }
      else if (aprsstr_InStr(str, str_len, s, 31ul)>=0L) return 1;
      /* comment filter */
      ++i;
   }
   return 0;
} /* end callfilt() */


static void inserthrt(const struct aprsdecode_DAT dat,
                aprsdecode_pOPHIST * op, uint8_t errs)
{
   aprsdecode_pFRAMEHIST lastf;
   aprsdecode_pFRAMEHIST f;
   aprsdecode_pVARDAT v;
   uint32_t len;
   uint32_t ih;
   uint32_t i;
   uint32_t stime;
   char s[51];
   char rb[51];
   struct aprsdecode_VARDAT * anonym;
   uint32_t tmp;
   f = (*op)->frames;
   lastf = 0;
   tmp = dat.hrtlen-1UL;
   ih = 0UL;
   if (ih<=tmp) for (;; ih++) {
      if (dat.hrtposes[ih].notinterpolated) {
         /* interpolating does vector drawer */
         stime = dat.hrtposes[ih].dtime;
         while (f && f->time0<stime) {
            lastf = f;
            f = f->next;
         }
         if (f==0 || f->time0!=stime) {
            /* not a redundant same time frame */
            osic_alloc((char * *) &f,
                sizeof(struct aprsdecode_FRAMEHIST));
            useri_debugmem.req = sizeof(struct aprsdecode_FRAMEHIST);
            useri_debugmem.mon += useri_debugmem.req;
            if (f==0) return;
            memset((char *)f,(char)0,
                sizeof(struct aprsdecode_FRAMEHIST));
            if ((float)fabs(dat.hrtposes[ih].dalt)<2.147483646E+9f) {
               aprsstr_IntToStr((int32_t)X2C_TRUNCI(dat.hrtposes[ih].dalt,
                X2C_min_longint,X2C_max_longint), 0UL, s, 51ul);
               strncpy(rb,">:$HR[a",51u);
               aprsstr_Append(rb, 51ul, s, 51ul);
               aprsstr_Append(rb, 51ul, "]", 2ul);
            }
            else rb[0U] = 0;
            len = (sizeof(struct aprsdecode_VARDAT)-499UL)+aprsstr_Length(rb,
                 51ul);
            osic_alloc((char * *) &v, len);
            useri_debugmem.req = len;
            useri_debugmem.mon += len;
            if (v==0) return;
            memset((char *)v,(char)0,len);
            { /* with */
               struct aprsdecode_VARDAT * anonym = v;
               anonym->pos.long0 = dat.hrtposes[ih].dlong;
               anonym->pos.lat = dat.hrtposes[ih].dlat;
               i = 0UL;
               do {
                  anonym->raw[i] = rb[i];
                  ++i;
               } while (rb[i]);
            }
            v->lastref = f;
            f->vardat = v;
            f->time0 = stime;
            f->nodraw = errs;
            if (lastf==0) {
               f->next = (*op)->frames;
               (*op)->frames = f; /* new track */
            }
            else {
               f->next = lastf->next; /* not nil if insert older frame */
               lastf->next = f; /* append waypoint */
            }
         }
      }
      if (ih==tmp) break;
   } /* end for */
} /* end inserthrt() */


static void settempspeed(aprsdecode_pOPHIST op,
                const struct aprsdecode_DAT dat)
{
   float rv;
   op->drawhints = op->drawhints&~0x3U;
   if (dat.objectfrom[0UL]) op->drawhints |= 0x2U;
   /*  EXCL(op^.drawhints, MIRRORSYM); */
   if (dat.sym=='_') {
      op->lastinftyp = 0U;
      op->lasttempalt = -32768;
      if (dat.wx.temp!=1.E+6f) {
         rv = aprstext_FtoC(dat.wx.temp);
         if (rv>=(-99.0f) && rv<=99.0f) {
            op->lasttempalt = (short)X2C_TRUNCI(rv,-32768,32767);
            op->temptime = aprsdecode_systime;
            op->lastinftyp = 100U;
         }
      }
      if (dat.course>0UL && dat.course<=360UL) {
         op->lastinftyp = (uint8_t)(110UL+(dat.course%360UL)/4UL);
         op->temptime = aprsdecode_systime;
      }
      else op->lastinftyp = 100U;
      if (dat.speed<500UL) {
         op->lastkmh = (short)X2C_TRUNCI((float)dat.speed*1.609f+0.5f,
                -32768,32767);
         if (op->lastinftyp<100U) op->lastinftyp = 100U;
         op->temptime = aprsdecode_systime;
      }
      else op->lastkmh = 0;
   }
   else if (dat.speed<30000UL) {
      op->lastkmh = (short)X2C_TRUNCI((float)dat.speed*1.852f+0.5f,
                -32768,32767);
      if (dat.speed>0UL && dat.course>180UL) op->drawhints |= 0x1U;
      if (dat.course<=360UL) {
         op->lastinftyp = (uint8_t)(10UL+(dat.course%360UL)/4UL);
      }
      else op->lastinftyp = 1U;
   }
   else if (op->lastinftyp<100U) op->lastinftyp = 0U;
   if (op->lastinftyp<100U) {
      if (dat.altitude>-10000L && dat.altitude<55535L) {
         op->lasttempalt = (short)(dat.altitude+(-22768L));
      }
      else op->lasttempalt = -32768;
   }
   if (dat.climb<-128L) op->clb = -128;
   else if (dat.climb>127L) op->clb = 127;
   else op->clb = (signed char)dat.climb;
} /* end settempspeed() */


static char widefilt(void)
{
   char s[101];
   useri_confstr(useri_fFINGERPRINT, s, 101ul);
   return aprsstr_InStr(s, 101ul, "0:2", 4ul)>=0L;
} /* end widefilt() */

#define aprsdecode_SAMETIME 27
/* max time diff for merged in frames */

static CHSET _cnst0 = {0x30000000UL,0x280086BAUL,0x80000001UL,0x00000001UL};

extern int32_t aprsdecode_Stoframe(aprsdecode_pOPHIST * optab,
                char rawbuf[], uint32_t rawbuf_len, uint32_t stime,
                char logmode, uint32_t * oldtime,
                struct aprsdecode_DAT dat)
{
   aprsdecode_pOPHIST opo;
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST lastf;
   aprsdecode_pFRAMEHIST frame;
   aprsdecode_pVARDAT same;
   aprsdecode_MONCALL igate;
   uint32_t cnt;
   uint32_t len;
   uint32_t i;
   uint16_t ch0;
   struct aprsdecode_VARDAT * anonym;
   int32_t aprsdecode_Stoframe_ret;
   X2C_PCOPY((void **)&rawbuf,rawbuf_len);
   *oldtime = 1UL; /* say not "new user" if filtered out */
   i = 0UL;
   do {
      ch0 = (uint16_t)(uint8_t)rawbuf[i];
      ++i;
   } while (ch0);
   do {
      --i;
   } while (!(i==0UL || (rawbuf[i]!='\015' && rawbuf[i]!='\012') && rawbuf[i]));
   rawbuf[i+1UL] = 0; /* remove trailing CR LF */
   /*FOR len:=0 TO i DO WrInt(ORD(rawbuf[len]), 4) END; WrLn; */
   if (dat.igatep>0UL && dat.igatep<=9UL) {
      memcpy(igate,dat.viacalls[dat.igatep],9u);
   }
   else igate[0UL] = 0;
   if (((((callfilt(useri_fOPFILT, dat.symcall, 9ul,
                1) || callfilt(useri_fIGATEFILT, igate, 9ul,
                1)) || callfilt(useri_fOBJSRCFILT, dat.objectfrom, 9ul,
                1)) || callfilt(useri_fDESTFILT, dat.dstcall, 9ul,
                0)) || callfilt(useri_fSTRFILT, dat.comment0, 256ul,
                0)) || callfilt(useri_fRAWFILT, rawbuf, rawbuf_len, 0)) {
      aprsdecode_Stoframe_ret = -1L;
      goto label;
   }
   if (useri_configon(useri_fMUSTBECALL)) {
      if (dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM) {
         if (!aprstext_isacall(dat.objectfrom, 9ul)) {
            aprsdecode_Stoframe_ret = -1L;
            goto label;
         }
      }
      else if (!aprstext_isacall(dat.symcall, 9ul)) {
         aprsdecode_Stoframe_ret = -1L;
         goto label;
      }
   }
   *oldtime = 0UL;
   cnt = 0UL; /* check for 2 WIDE chunk */
   if (widefilt()) {
      i = 0UL;
      while (((cnt<2UL && i<=9UL) && dat.viacalls[i][0UL])
                && (dat.igatep==0UL || i<dat.igatep)) {
         if (aprsstr_InStr(dat.viacalls[i], 9ul, "WIDE", 5ul)==0L) ++cnt;
         ++i;
      }
   }
   opo = 0;
   op = *optab;
   while (op && X2C_STRCMP(dat.symcall,9u,op->call,9u)) {
      opo = op;
      op = op->next;
   }
   if (op==0) {
      osic_alloc((char * *) &op, sizeof(struct aprsdecode_OPHIST));
      useri_debugmem.req = sizeof(struct aprsdecode_OPHIST);
      useri_debugmem.mon += useri_debugmem.req;
      if (op==0) {
         aprsdecode_Stoframe_ret = -2L;
         goto label;
      }
      memset((char *)op,(char)0,sizeof(struct aprsdecode_OPHIST));
      memcpy(op->call,dat.symcall,9u);
      if (logmode || opo==0) {
         /* store it at begin of list for quick find */
         op->next = *optab;
         *optab = op;
      }
      else opo->next = op;
   }
   else if (logmode && opo) {
      /* move it to begin of list */
      opo->next = op->next;
      op->next = *optab;
      *optab = op;
   }
   else *oldtime = op->lasttime;
   if (dat.hrttime>0UL) stime = dat.hrttime;
   if (op->lasttime<=stime) {
      /* new waypoint */
      settempspeed(op, dat);
      op->lasttime = stime;
   }
   frame = op->frames;
   lastf = 0;
   same = 0;
   /*
   
     IF logmode THEN                                              (* refcnt used as raw string hash *)
       IF (op^.lastfrp<>NIL) & (op^.lastfrp^.time>=stime)
                THEN op^.lastfrp:=NIL END; 
       IF op^.lastfrp<>NIL THEN                                   (* quick write no compress mode *)
         frame:=NIL;
         lastf:=op^.lastfrp;
       ELSE                                                       (* look for same frame *)
                 
         i:=0;
         WHILE (frame<>NIL) & ((frame^.vardat^.refcnt<>hash) 
         OR NOT strcmp(frame^.vardat^.raw, rawbuf)) DO
           lastf:=frame;
           frame:=frame^.next;
           INC(i);
         END;
         IF i>COMPRESSLIMIT THEN op^.lastfrp:=lastf END;
                (* switch to compress off *)
       END;
     ELSE                                                         (* online data find without hash *)
                 
       WHILE (frame<>NIL) & NOT strcmp(frame^.vardat^.raw, rawbuf) 
       DO lastf:=frame; frame:=frame^.next END;      
     END; 
   */
   if (op->lastfrp && op->lastfrp->time0>stime) op->lastfrp = 0;
   if (op->lastfrp) frame = op->lastfrp;
   i = 0UL;
   while (frame && !strcmp0(frame->vardat->raw, 500ul, rawbuf, rawbuf_len)) {
      lastf = frame;
      frame = frame->next;
      ++i;
   }
   if (i>20UL) op->lastfrp = lastf;
   if (frame) {
      /* found same frame */
      same = frame->vardat;
      lastf = same->lastref;
      while (lastf->next) lastf = lastf->next;
   }
   if (lastf && lastf->time0>stime) {
      /* older frame to insert */
      lastf = 0;
      frame = op->frames;
      while (frame && frame->time0<stime) {
         lastf = frame;
         frame = frame->next;
      }
      if (frame && frame->time0-stime<27UL) {
         aprsdecode_Stoframe_ret = -2L;
         goto label;
      }
   }
   /* not insert same frame */
   osic_alloc((char * *) &frame, sizeof(struct aprsdecode_FRAMEHIST));
   useri_debugmem.req = sizeof(struct aprsdecode_FRAMEHIST);
   useri_debugmem.mon += useri_debugmem.req;
   if (frame==0) {
      aprsdecode_Stoframe_ret = -2L;
      goto label;
   }
   memset((char *)frame,(char)0,sizeof(struct aprsdecode_FRAMEHIST));
   if (same==0) {
      /* found new data */
      len = (sizeof(struct aprsdecode_VARDAT)-499UL)+aprsstr_Length(rawbuf,
                rawbuf_len);
      osic_alloc((char * *) &same, len);
      useri_debugmem.req = len;
      useri_debugmem.mon += len;
      if (same==0) {
         aprsdecode_Stoframe_ret = -2L;
         goto label;
      }
      memset((char *)same,(char)0,len);
      { /* with */
         struct aprsdecode_VARDAT * anonym = same;
         anonym->pos = dat.pos;
         /*      altitude:=dat.altitude; */
         if (dat.hbitp==0UL && dat.igaterawp<=255UL) {
            /* direct to igate */
            anonym->igatepos = (uint8_t)dat.igaterawp;
            anonym->igatelen = (uint8_t)dat.igatelen;
         }
         i = 0UL;
         do {
            anonym->raw[i] = rawbuf[i];
            ++i;
         } while (rawbuf[i]);
      }
   }
   else ++same->refcnt;
   if (same->lastref==0 || same->lastref->time0<=stime) {
      same->lastref = frame;
   }
   frame->vardat = same;
   frame->time0 = stime;
   if (X2C_INL((int32_t)(uint8_t)dat.typc,128,_cnst0)) {
      if (((X2C_INL((uint8_t)dat.symt,256,
                _cnst) && (uint8_t)dat.sym>' ') && (uint8_t)
                dat.sym<'\177') && op->sym.tab!='\001') {
         op->sym.tab = dat.symt;
         op->sym.pic = dat.sym;
      }
      else frame->nodraw |= 0x4U;
      if (cnt>=2UL || dat.objkill=='1') {
         op->sym.tab = '\001';
         op->sym.pic = 0;
      }
   }
   op->areasymb = dat.areasymb;
   op->poligon = dat.multiline.size>2UL;
   op->lastrxport = dat.lastrxport;
   if (lastf==0) {
      frame->next = op->frames;
      op->frames = frame; /* new track */
   }
   else {
      frame->next = lastf->next; /* not nil if insert older frame */
      lastf->next = frame; /* append waypoint */
   }
   /*  IF op^.lastfrp<>NIL THEN op^.lastfrp:=frame END; */
   if (dat.hrtlen>0UL) inserthrt(dat, &op, frame->nodraw);
   if (!logmode) {
      /* read log check whole track at end */
      /*    Checktrack(op, lastf); */
      inwindow(op, rawbuf, rawbuf_len);
   }
   op->dirty = 1; /* Checktrack must follow */
   if (!(frame->nodraw&~0x1U)) aprsdecode_Stoframe_ret = 0L;
   else aprsdecode_Stoframe_ret = -1L;
   label:;
   X2C_PFREE(rawbuf);
   return aprsdecode_Stoframe_ret;
} /* end Stoframe() */


extern void aprsdecode_Checktracks(void)
{
   aprsdecode_pOPHIST op;
   if (aprsdecode_lums.logmode) op = aprsdecode_ophist2;
   else op = aprsdecode_ophist0;
   while (op) {
      if (op->dirty) {
         Checktrack(op, 0);
         op->dirty = 0;
      }
      op = op->next;
   }
} /* end Checktracks() */


static char locked(aprsdecode_pOPHIST op)
/* prevent entries in click table from purge */
{
   uint32_t i;
   i = 0UL;
   while (i<aprsdecode_click.entries) {
      if (aprsdecode_click.table[i].opf==op) return 1;
      ++i;
   }
   return 0;
} /* end locked() */


static void freevardat(aprsdecode_pVARDAT v)
{
   uint32_t alen;
   alen = (sizeof(struct aprsdecode_VARDAT)-499UL)+aprsstr_Length(v->raw,
                500ul);
   osic_free((char * *) &v, alen);
   useri_debugmem.mon -= alen;
} /* end freevardat() */

#define aprsdecode_CHKINTERVALL 120
/* seconds to next check track */


extern void aprsdecode_purge(aprsdecode_pOPHIST * ops, uint32_t oldt,
                uint32_t oldobj)
{
   aprsdecode_pOPHIST opx;
   aprsdecode_pOPHIST lastop;
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST frame;
   aprsdecode_pVARDAT v;
   uint32_t old;
   char chk;
   /*chkptr(ops); */
   /*
   WrInt(ORD(lums.logmode), 1);
   IF ops=ophist THEN WrStrLn("purge oph")
                ELSIF ops=ophist2 THEN WrStrLn("purge oph2")
                ELSE WrStrLn("purge error") END;
   */
   op = *ops;
   lastop = 0;
   while (op) {
      if (op->dirty) Checktrack(op, 0);
      if (op->sym.tab=='\001') old = X2C_max_longcard;
      else if ((0x2U & op->drawhints) && op->lastinftyp<100U) {
         old = oldobj; /* object but no wx obj for wx graphs */
      }
      else old = oldt;
      chk = 0;
      for (;;) {
         frame = op->frames;
         if (frame==0) break;
         /*WrCard(systime-frame^.time, 6);
                IF frame^.next<>NIL THEN WrStr("+") END; */
         if (frame->time0>=old || locked(op)) break;
         if (frame->next && frame->time0/120UL!=frame->next->time0/120UL) {
            chk = 1;
         }
         v = frame->vardat;
         if (v->lastref==frame) freevardat(v);
         else --v->refcnt;
         op->frames = frame->next;
         op->lastfrp = 0;
         /*WrStr("(purge)"); */
         useri_debugmem.mon -= sizeof(struct aprsdecode_FRAMEHIST);
         osic_free((char * *) &frame,
                sizeof(struct aprsdecode_FRAMEHIST));
      }
      if (op->frames==0 && !locked(op)) {
         /* empty op */
         opx = op;
         op = op->next;
         if (lastop==0) *ops = op;
         else lastop->next = op;
         /*WrStr(opx^.call); WrStrLn("(purgop) "); */
         useri_debugmem.mon -= sizeof(struct aprsdecode_OPHIST);
         osic_free((char * *) &opx, sizeof(struct aprsdecode_OPHIST));
      }
      else {
         if (chk) {
            Checktrack(op, 0); /*WrStr(op^.call); WrStr(" ")*/
         }
         lastop = op;
         op = op->next;
      }
   }
/*WrLn; */
/*WrStrLn(" --purge"); */
} /* end purge() */


extern void aprsdecode_delwaypoint(aprsdecode_pOPHIST op,
                aprsdecode_pFRAMEHIST * frame)
{
   aprsdecode_pFRAMEHIST lastf;
   aprsdecode_pFRAMEHIST lf;
   aprsdecode_pFRAMEHIST f;
   int32_t mi;
   int32_t di;
   int32_t i;
   struct aprsdecode_DAT dat;
   if (op==0 || *frame==0) return;
   op->lastfrp = 0;
   i = 0L;
   di = 0L;
   mi = 0L;
   lf = 0;
   f = op->frames;
   while (f) {
      if (f->next==*frame) {
         lf = f; /* next frame is to delete */
         di = mi;
      }
      if (f->vardat==(*frame)->vardat) ++i;
      f->vardat->refcnt = 0UL;
      ++mi;
      f = f->next;
   }
   if (i==1L) freevardat((*frame)->vardat);
   if (lf==0) op->frames = (*frame)->next;
   else lf->next = (*frame)->next;
   useri_debugmem.mon -= sizeof(struct aprsdecode_FRAMEHIST);
   osic_free((char * *)frame, sizeof(struct aprsdecode_FRAMEHIST));
   f = op->frames;
   while (f) {
      if (f->vardat->refcnt<65535UL) ++f->vardat->refcnt;
      f->vardat->lastref = f;
      f->nodraw = 0U; /* mark errors new later */
      f = f->next;
   }
   Checktrack(op, 0);
   i = 0L; /* set new marker to the nearest from deleted with error */
   mi = X2C_max_longint;
   *frame = 0;
   lf = 0;
   lastf = 0;
   f = op->frames;
   while (f) {
      if ((!aprsdecode_lums.errorstep || (f->nodraw&~0x40U)!=0U)
                && labs(i-di)<mi) {
         mi = labs(i-di);
         *frame = f;
      }
      if (i==di) lf = f;
      ++i;
      lastf = f;
      f = f->next;
   }
   if (*frame==0) *frame = lf;
   if (lastf) {
      if (aprsdecode_Decode(lastf->vardat->raw, 500ul, &dat)>=0L) {
         settempspeed(op, dat);
      }
      op->lasttime = lastf->time0;
                /* deleted last frame so correct op time */
   }
} /* end delwaypoint() */


static char cmpfrom(const char a[], uint32_t a_len,
                uint32_t from, const char b[], uint32_t b_len)
{
   uint32_t i;
   i = 0UL;
   while (from<=a_len-1 && b[i]) {
      if (a[from]!=b[i]) return 0;
      ++i;
      ++from;
   }
   return 1;
} /* end cmpfrom() */

/*
PROCEDURE AppQ(VAR h:ARRAY OF CHAR; q-:ARRAY OF CHAR; appcall:BOOLEAN);
BEGIN
  Append(h, ">"+TOCALL+",");
  Append(h, q);
  IF appcall THEN Append(h, servercall) END;
  Append(h, ":");
END AppQ;
*/

static char callchk(uint32_t * qpos, char * tablechk,
                char buf[], uint32_t buf_len, uint32_t * p,
                uint32_t tablen, uint32_t * pssid, char withstar,
                char iscall0)
{
   uint32_t lit;
   uint32_t num1;
   uint32_t u;
   uint32_t j;
   uint32_t i;
   char c;
   *pssid = 0UL;
   u = 0UL;
   for (;;) {
      if (u>=tablen) break;
      j = 0UL;
      i = *p;
      while (ungate[u][j]==buf[i]) {
         ++i;
         ++j;
         if (ungate[u][j]==0) {
            *tablechk = 1;
            goto loop_exit;
         }
      }
      ++u;
   }
   loop_exit:;
   if (iscall0) {
      /* we want a callsign */
      num1 = 0UL;
      lit = 0UL;
      for (;;) {
         c = buf[*p];
         if ((uint8_t)c>='0' && (uint8_t)c<='9') ++num1;
         else if ((uint8_t)c>='A' && (uint8_t)c<='Z') ++lit;
         else break;
         ++*p;
      }
      if ((lit<2UL || num1==0UL) || num1>2UL) return 0;
      *pssid = *p;
      if (buf[*p]=='-') {
         ++*p;
         if (buf[*p]=='1') {
            ++*p;
            if ((uint8_t)buf[*p]>='0' && (uint8_t)buf[*p]<='5') ++*p;
         }
         else {
            if ((uint8_t)buf[*p]<'1' || (uint8_t)buf[*p]>'9') return 0;
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
   return 1;
} /* end callchk() */


static void Iconstruct(char qtext[32], uint32_t * pins,
                char logcall[], uint32_t logcall_len, char buf[],
                uint32_t buf_len, uint32_t * p)
{
   uint32_t k;
   uint32_t j;
   uint32_t i;
   if (*p<=4UL || buf[*p-2UL]!='I') return;
   /* ,CALL,I: */
   i = *p-4UL;
   while (buf[i]!=',') {
      /* hop back call */
      if (i==0UL) return;
      --i;
   }
   k = i; /* "," before call */
   ++i;
   j = 0UL;
   while (j<=logcall_len-1 && logcall[j]) {
      if (logcall[j]!=buf[i]) return;
      ++i;
      ++j;
   }
   /* logcall = call,I */
   *pins = k;
   i = *p-1UL;
   *p = k+1UL; /* payload */
   for (;;) {
      buf[k] = buf[i];
      if (i>=buf_len-1 || buf[i]==0) break;
      ++k;
      ++i;
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
                uint32_t tablen, const char logcall[],
                uint32_t logcall_len, uint32_t udpchan,
                char valid, char chkrfcall,
                char * tablechk)
{
   uint32_t qpos;
   uint32_t ha;
   uint32_t ilen;
   uint32_t psum1;
   uint32_t psum0;
   uint32_t pssid;
   uint32_t pins;
   uint32_t len;
   uint32_t p;
   uint32_t i;
   uint8_t hashh;
   uint8_t hashl;
   char qtext[32];
   uint32_t tmp;
   hashl = 0U;
   hashh = 0U;
   p = 0UL;
   pins = 0UL;
   pssid = 0UL; /* for savety */
   len = 0UL;
   while ((len<buf_len-1 && buf[len]) && buf[len]!='\015') ++len;
   if (len==0UL || len+35UL>buf_len-1) return -2L;
   qpos = 0UL;
   *tablechk = 0;
   for (;;) {
      psum0 = p;
      if (!callchk(&qpos, tablechk, buf, buf_len, &p, tablen, &pssid, 0,
                udpchan && chkrfcall)) return -3L;
      if (buf[p]!='>') return -1L;
      ++p;
      if (!callchk(&qpos, tablechk, buf, buf_len, &p, tablen, &pssid, 0, 0)) {
         return -3L;
      }
      psum1 = pssid;
      while (buf[p]==',') {
         ++p;
         if (!callchk(&qpos, tablechk, buf, buf_len, &p, tablen, &pssid, 1,
                0)) return -3L;
      }
      if (buf[p]!=':') return -1L;
      if (pins==0UL) pins = p;
      ++p;
      if (buf[p]!='}') break;
      ++p;
   }
   if (p>=len) return -2L;
   qtext[0U] = 0;
   if (qpos>0UL) {
      /* qA */
      if (X2C_CAP(buf[qpos])=='Z') return -3L;
      if (X2C_CAP(buf[qpos])=='I') {
         aprsstr_Append(qtext, 32ul, ",", 2ul); /* qAI */
         useri_confappend(useri_fMYCALL, qtext, 32ul);
      }
   }
   else if (udpchan) {
      aprsstr_Append(qtext, 32ul, ",qAU,", 6ul);
      useri_confappend(useri_fMYCALL, qtext, 32ul);
   }
   else if (logcall[0UL]) {
      if (cmpfrom(buf, buf_len, 0UL, logcall, logcall_len)) {
         strncpy(qtext,",qAC,",32u);
      }
      else strncpy(qtext,",qAS,",32u);
      useri_confappend(useri_fMYCALL, qtext, 32ul);
      Iconstruct(qtext, &pins, logcall, logcall_len, buf, buf_len, &p);
   }
   if (buf[p]=='?') return -4L;
   tmp = psum1-1UL;
   i = psum0;
   if (i<=tmp) for (;; i++) {
      aprsstr_HashCh(buf[i], &hashl, &hashh);
      if (i==tmp) break;
   } /* end for */
   while ((p<len && buf[p]!='\015') && buf[p]!='\012') {
      aprsstr_HashCh(buf[p], &hashl, &hashh);
      ++p;
   }
   ha = (uint32_t)(uint8_t)(char)hashl+(uint32_t)(uint8_t)
                (char)hashh*256UL&65535UL;
   if (timehash[ha]+dupetime>aprsdecode_realtime) return -5L;
   /* a duplicate */
   if (udpchan || valid) timehash[ha] = aprsdecode_realtime;
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
      i = 0UL;
      while (i<ilen) {
         buf[p] = qtext[i];
         ++p;
         ++i;
      }
   }
   return 0L;
} /* end AprsIs() */


static void connstr(aprsdecode_pTCPSOCK cp, char s[], uint32_t s_len)
{
   struct aprsdecode_TCPSOCK * anonym;
   { /* with */
      struct aprsdecode_TCPSOCK * anonym = cp;
      useri_confstr(useri_fMYCALL, s, s_len);
      aprsstr_Append(s, s_len, " connected to ", 15ul);
      aprsstr_Append(s, s_len, anonym->ipnum, 64ul);
      aprsstr_Append(s, s_len, ":", 2ul);
      aprsstr_Append(s, s_len, anonym->port, 6ul);
   }
} /* end connstr() */


static void wrconnected(aprsdecode_pTCPSOCK cp)
{
   char h[301];
   connstr(cp, h, 301ul);
   useri_textautosize(0L, 5L, 6UL, 5UL, 'b', h, 301ul);
} /* end wrconnected() */


extern void aprsdecode_tcpconnstat(char s[], uint32_t s_len)
{
   aprsdecode_pTCPSOCK tcp0;
   char h[100];
   struct aprsdecode_TCPSOCK * anonym;
   tcp0 = aprsdecode_tcpsocks;
   while (tcp0) {
      { /* with */
         struct aprsdecode_TCPSOCK * anonym = tcp0;
         if (anonym->connt>0UL && anonym->connt<aprsdecode_realtime) {
            if (s[0UL]) aprsstr_Append(s, s_len, "\012", 2ul);
            aprsstr_Append(s, s_len, "TCP:", 5ul);
            aprsstr_Append(s, s_len, anonym->ipnum, 64ul);
            aprsstr_Append(s, s_len, ":", 2ul);
            aprsstr_Append(s, s_len, anonym->port, 6ul);
            aprsstr_Append(s, s_len, "\012  Connecttime:", 16ul);
            aprsstr_TimeToStr(aprsdecode_realtime-anonym->connt, h, 100ul);
            aprsstr_Append(s, s_len, h, 100ul);
            if (aprsdecode_lasttcprx<=aprsdecode_realtime && aprsdecode_realtime-aprsdecode_lasttcprx<3600UL)
                 {
               aprsstr_Append(s, s_len, "\012  No rx since:", 16ul);
               aprsstr_TimeToStr(aprsdecode_realtime-aprsdecode_lasttcprx, h,
                 100ul);
               aprsstr_Append(s, s_len, h, 100ul);
            }
            aprsstr_Append(s, s_len, "\012  Rx Frames:", 14ul);
            aprsstr_IntToStr((int32_t)anonym->rxframes, 1UL, h, 100ul);
            aprsstr_Append(s, s_len, h, 100ul);
            aprsstr_Append(s, s_len, "\012  Rx Bytes:", 13ul);
            aprsstr_IntToStr((int32_t)anonym->rxbytes, 1UL, h, 100ul);
            aprsstr_Append(s, s_len, h, 100ul);
            aprsstr_Append(s, s_len, "\012  Tx Frames:", 14ul);
            aprsstr_IntToStr((int32_t)anonym->txframes, 1UL, h, 100ul);
            aprsstr_Append(s, s_len, h, 100ul);
            aprsstr_Append(s, s_len, "\012  Tx Bytes:", 13ul);
            aprsstr_IntToStr((int32_t)anonym->txbytes, 1UL, h, 100ul);
            aprsstr_Append(s, s_len, h, 100ul);
         }
         tcp0 = anonym->next;
      }
   }
} /* end tcpconnstat() */


extern void aprsdecode_udpconnstat(uint32_t port, char s[],
                uint32_t s_len)
{
   char h[51];
   uint32_t i;
   struct aprsdecode_UDPSOCK * anonym;
   if (port<=3UL) {
      { /* with */
         struct aprsdecode_UDPSOCK * anonym = &aprsdecode_udpsocks0[port];
         if (anonym->starttime>0UL) {
            if (aprsdecode_realtime<anonym->starttime) {
               anonym->starttime = aprsdecode_realtime;
            }
            if (s[0UL]) aprsstr_Append(s, s_len, "\012", 2ul);
            aprsstr_Append(s, s_len, "Rf ", 4ul);
            aprsstr_IntToStr((int32_t)(port+1UL), 1UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            aprsstr_Append(s, s_len, ":", 2ul);
            i = 1UL;
            for (;;) {
               useri_conf2str((uint8_t)(38UL+port), 0UL, i, 1, h, 51ul);
               if (h[0U]==0) break;
               if (i>1UL) aprsstr_Append(s, s_len, " ", 2ul);
               aprsstr_Append(s, s_len, h, 51ul);
               ++i;
            }
            aprsstr_Append(s, s_len, "\012  ", 4ul);
            aprsstr_ipv4tostr(anonym->ip, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            aprsstr_Append(s, s_len, ":", 2ul);
            aprsstr_IntToStr((int32_t)anonym->dport, 0UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            if (anonym->checkip) aprsstr_Append(s, s_len, "/", 2ul);
            else aprsstr_Append(s, s_len, ":", 2ul);
            aprsstr_IntToStr((int32_t)anonym->bindport, 0UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            aprsstr_Append(s, s_len, "\012  Uptime :", 12ul);
            aprsstr_TimeToStr(aprsdecode_realtime-anonym->starttime, h,
                51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            if (anonym->lastudprx>0UL && anonym->lastudprx<=aprsdecode_realtime)
                 {
               aprsstr_Append(s, s_len, "\012  Last rx:", 12ul);
               aprsstr_TimeToStr(aprsdecode_realtime-anonym->lastudprx, h,
                51ul);
               aprsstr_Append(s, s_len, h, 51ul);
            }
            if (anonym->lastudptx>0UL && anonym->lastudptx<=aprsdecode_realtime)
                 {
               aprsstr_Append(s, s_len, "\012  Last tx:", 12ul);
               aprsstr_TimeToStr(aprsdecode_realtime-anonym->lastudptx, h,
                51ul);
               aprsstr_Append(s, s_len, h, 51ul);
            }
            if (aprsdecode_realtime<anonym->bpstime && anonym->bpstime-aprsdecode_realtime<3600UL)
                 {
               aprsstr_Append(s, s_len, "\012  Next net>rf(s):", 19ul);
               aprsstr_IntToStr((int32_t)
                (anonym->bpstime-aprsdecode_realtime), 1UL, h, 51ul);
               aprsstr_Append(s, s_len, h, 51ul);
            }
            aprsstr_Append(s, s_len, "\012  Rx Frames:", 14ul);
            aprsstr_IntToStr((int32_t)anonym->rxframes, 1UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            aprsstr_Append(s, s_len, "\012  Rx Bytes:", 13ul);
            aprsstr_IntToStr((int32_t)anonym->rxbytes, 1UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            aprsstr_Append(s, s_len, "\012  Tx Frames:", 14ul);
            aprsstr_IntToStr((int32_t)anonym->txframes, 1UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
            aprsstr_Append(s, s_len, "\012  Tx Bytes:", 13ul);
            aprsstr_IntToStr((int32_t)anonym->txbytes, 1UL, h, 51ul);
            aprsstr_Append(s, s_len, h, 51ul);
         }
      }
   }
} /* end udpconnstat() */


extern void aprsdecode_tcpclose(aprsdecode_pTCPSOCK w, char fin)
{
   char h[201];
   struct aprsdecode_TCPSOCK * anonym;
   { /* with */
      struct aprsdecode_TCPSOCK * anonym = w;
      if ((int32_t)anonym->fd>=0L) osic_CloseSock(anonym->fd);
      anonym->fd = -1L;
      aprsdecode_lasttcprx = 0UL;
      strncpy(h,"\012TCP Close ",201u);
      aprsstr_Append(h, 201ul, anonym->ipnum, 64ul);
      aprsstr_Append(h, 201ul, ":", 2ul);
      aprsstr_Append(h, 201ul, anonym->port, 6ul);
      aprsstr_Append(h, 201ul, "\012", 2ul);
      useri_textautosize(0L, 5L, 6UL, 4UL, 'e', h, 201ul);
   }
} /* end tcpclose() */


static void Watchbeacon(aprsdecode_pTCPSOCK cp)
{
   char ok0;
   aprsdecode_FRAMEBUF tmp;
   ok0 = Sendtcp(cp, *(aprsdecode_FRAMEBUF *)memcpy(&tmp,"# watchbeacon\015\01\
2",16u));
   cp->watchtime = aprsdecode_realtime;
} /* end Watchbeacon() */


static void appfilter(char s[], uint32_t s_len, char nulltoo)
/* build string "filter m/300" */
{
   char h[1000];
   useri_confstr(useri_fSERVERFILT, h, 1000ul);
   if (nulltoo || h[0U]) {
      aprsstr_Append(s, s_len, " filter ", 9ul);
      aprsstr_Append(s, s_len, h, 1000ul);
   }
} /* end appfilter() */


static void updatefilters(aprsdecode_pTCPSOCK cp)
{
   char ok0;
   aprsdecode_FRAMEBUF s;
   strncpy(s,"#",512u);
   appfilter(s, 512ul, 1);
   aprsstr_Append(s, 512ul, "\015\012", 3ul);
   ok0 = Sendtcp(cp, s);
   strncpy(s,"Sent to Server",512u);
   appfilter(s, 512ul, 1);
   useri_textautosize(0L, 5L, 6UL, 4UL, 'b', s, 512ul);
} /* end updatefilters() */


static char tcpconn(aprsdecode_pTCPSOCK * sockchain, int32_t f)
{
   aprsdecode_pTCPSOCK cp;
   aprsdecode_FRAMEBUF h;
   int32_t res;
   char ok0;
   char s[100];
   struct aprsdecode_TCPSOCK * anonym;
   if ((int32_t)f<0L) return 0;
   osic_alloc((char * *) &cp, sizeof(struct aprsdecode_TCPSOCK));
   useri_debugmem.req = sizeof(struct aprsdecode_TCPSOCK);
   useri_debugmem.mon += useri_debugmem.req;
   if (cp==0) {
      osic_CloseSock(f);
      if (aprsdecode_verb) osi_WrStrLn("tcp conn out of memory", 23ul);
      return 0;
   }
   memset((char *)cp,(char)0,sizeof(struct aprsdecode_TCPSOCK));
   { /* with */
      struct aprsdecode_TCPSOCK * anonym = cp;
      anonym->fd = f;
      res = socknonblock(f);
      /*    txbuf:=NIL; */
      anonym->watchtime = aprsdecode_realtime;
   }
   cp->next = *sockchain;
   *sockchain = cp;
   useri_confstr(useri_fMYCALL, s, 100ul);
   if (s[0U]) {
      if (aprstext_callwrong(s, 100ul)) {
         useri_xerrmsg("illegal \'My Call\' in login", 27ul);
      }
      aprsstr_Assign(h, 512ul, "user ", 6ul);
      aprsstr_Append(h, 512ul, s, 100ul);
      useri_confstr(useri_fPW, s, 100ul);
      if (s[0U]) {
         /* & configon(fPW)*/
         aprsstr_Append(h, 512ul, " pass ", 7ul);
         aprsstr_Append(h, 512ul, s, 100ul);
      }
      aprsstr_Append(h, 512ul, " vers ", 7ul);
      aprsstr_Append(h, 512ul, "aprsmap(cu) 0.77", 17ul);
      appfilter(h, 512ul, 0);
      /*    IF filter[0]<>0C THEN Append(h, " filter ");
                Append(h, filter) END; */
      aprsstr_Append(h, 512ul, "\015\012", 3ul);
      ok0 = Sendtcp(cp, h);
   }
   cp->txframes = 0UL;
   cp->txbytes = 0UL;
   return 1;
} /* end tcpconn() */


static void urlport(char s[], uint32_t s_len, char url[],
                uint32_t url_len, char port[], uint32_t port_len)
/* url:port to url port */
{
   int32_t ii;
   uint32_t j;
   uint32_t i;
   X2C_PCOPY((void **)&s,s_len);
   url[0UL] = 0;
   port[0UL] = 0;
   i = 0UL;
   while (i<=s_len-1 && s[i]==' ') ++i;
   if (i<=s_len-1 && s[i]=='[') {
      /* ipv6 url */
      j = 0UL;
      ++i;
      while ((i<=s_len-1 && s[i]) && s[i]!=']') {
         if (j<=url_len-1) {
            url[j] = s[i];
            ++j;
         }
         ++i;
      }
      if (j<=url_len-1) url[j] = 0;
      if (i<=s_len-1 && s[i]==']') {
         ++i;
         if (i<=s_len-1 && s[i]==':') {
            /* ipv6 port */
            j = 0UL;
            ++i;
            while (i<=s_len-1 && (uint8_t)s[i]>' ') {
               if (j<=port_len-1) {
                  port[j] = s[i];
                  ++j;
               }
               ++i;
            }
            if (j<=port_len-1) port[j] = 0;
         }
      }
   }
   else {
      /* ipv4 */
      aprsstr_Assign(url, url_len, s, s_len);
      ii = aprsstr_InStr(url, url_len, ":", 2ul);
      if (ii>0L) {
         url[ii] = 0;
         aprsstr_Delstr(s, s_len, 0UL, (uint32_t)(ii+1L));
         aprsstr_Assign(port, port_len, s, s_len);
      }
   }
   X2C_PFREE(s);
} /* end urlport() */

#define aprsdecode_DEFAULTPORT "14580"


static void Gateconn(aprsdecode_pTCPSOCK * cp)
{
   int32_t fd;
   char port[1000];
   char ip[1000];
   char s[1000];
   /*WrInt(cycleservers, 5); WrStrLn(" gateconn"); */
   if (*cp) {
      if ((int32_t)(*cp)->fd>=0L && !useri_configon(useri_fCONNECT)) {
         saybusy(&(*cp)->fd, "", 1ul);
      }
   }
   else if (useri_configon(useri_fCONNECT) && Watchclock(&connecttime,
                30UL)) {
      for (;;) {
         useri_confstrings(useri_fSERVERURL, cycleservers, 0, s, 1000ul);
         if (s[0U] || cycleservers==0UL) break;
         cycleservers = 0UL;
      }
      if (s[0U]) {
         ++cycleservers;
         urlport(s, 1000ul, ip, 1000ul, port, 1000ul);
         if (port[0U]==0) strncpy(port,"14580",1000u);
         /*WrInt(cycleservers, 5);WrStr(" <"); WrStr(ip);WrStr("><");
                WrStr(port);WrStrLn(">"); */
         fd = connectto(ip, port);
         if ((int32_t)fd>=0L) {
            if (tcpconn(cp, fd)) {
               aprsstr_Assign((*cp)->ipnum, 64ul, ip, 1000ul);
               aprsstr_Assign((*cp)->port, 6ul, port, 1000ul);
               (*cp)->beacont = 0UL; /* send next beacon now */
            }
         }
      }
      else useri_xerrmsg("Net: No Online/Server URL configured", 37ul);
   }
} /* end Gateconn() */


static void porttext(uint32_t p, char s[], uint32_t s_len)
{
   char h[31];
   aprsstr_IntToStr((int32_t)(p+1UL), 1UL, h, 31ul);
   aprsstr_Assign(s, s_len, "UDP ", 5ul);
   aprsstr_Append(s, s_len, h, 31ul);
} /* end porttext() */


static void rfbeacons(void)
{
   uint32_t i;
   uint32_t nb;
   int32_t j;
   uint32_t shift;
   uint32_t bshift;
   uint32_t bn;
   uint32_t bt;
   aprsdecode_FRAMEBUF s;
   char h[101];
   char says[101];
   char port;
   struct aprsdecode_DAT dat;
   char tmp;
   nb = 0UL;
   shift = beaconrandomstart;
   bshift = (uint32_t)useri_conf2int(useri_fRFBTSHIFT, 0UL, 0L, 86400L,
                120L);
   if (rfbecondone==0UL || rfbecondone>aprsdecode_realtime) {
      rfbecondone = aprsdecode_realtime;
   }
   for (;;) {
      if (aprsdecode_testbeaconbuf[0UL]) {
         /* manual sent beacon */
         aprsstr_Assign(s, 512ul, aprsdecode_testbeaconbuf, 512ul);
         aprsdecode_testbeaconbuf[0UL] = 0;
         strncpy(says,"Sent: ",101u);
         aprsstr_Append(says, 101ul, s, 512ul);
         if (getbeaconparm(s, 512ul, &bt, &port)) bt = 1UL;
         else bt = 0UL;
      }
      else {
         useri_confstrings(useri_fRBTEXT, nb, 0, s, 512ul);
         if (s[0UL]==0) break;
         if (!getbeaconparm(s, 512ul, &bt, &port)) bt = 0UL;
         says[0U] = 0;
      }
      if (bt>0UL) {
         /* valid time */
         bn = (aprsdecode_realtime+shift)%bt;
         /*WrInt(realtime MOD 86400, 10); WrInt(realtime-rfbecondone, 4);
                WrInt(bshift, 4); WrInt(shift, 14); WrStrLn(" bshift"); */
         if (bn<aprsdecode_realtime-rfbecondone) {
            beaconmacros(s, 512ul, "", 1ul, "", 1ul, 1);
            if (s[0UL]) {
               if (aprsdecode_Decode(s, 512ul, &dat)>=0L) {
                  if ((uint8_t)dat.symt<' ') {
                     strncpy(h,"beacon warning: no symbol: ",101u);
                     aprsstr_Append(h, 101ul, s, 512ul);
                     useri_xerrmsg(h, 101ul);
                  }
                  if (X2C_CAP(port)=='N') {
                     /* beacon to net */
                     if (dat.type!=aprsdecode_MICE) {
                        j = aprsstr_InStr(s, 512ul, ":", 2ul);
                        if (j>=0L) {
                           aprsstr_Delstr(s, 512ul, 0UL, (uint32_t)(j+1L));
                 /* remove rf adress */
                           SendNet(aprsdecode_tcpsocks, dat.srcall, 9ul, s,
                512ul, 1, h, 101ul);
                           if (h[0U]) {
                              strncpy(says,"beacon: not sent ",101u);
                              aprsstr_Append(says, 101ul, h, 101ul);
                           }
                        }
                     }
                     else strncpy(says,"beacon: not sent mic-e to Net",101u);
                  }
                  else if ((uint8_t)port>='1') {
                     if (s[0UL]) {
                        i = (uint32_t)(uint8_t)port-49UL;
                        if (i<4UL && useri_configon((uint8_t)(38UL+i))) {
                           if (Sendudp(s, 512ul, i, 1UL)<0L) {
                              strncpy(says,"beacon: Rfport ",101u);
                              aprsstr_Append(says, 101ul,
                (char *)(tmp = (char)(i+49UL),&tmp), 1u/1u);
                              aprsstr_Append(says, 101ul, " not configured to\
 send", 24ul);
                           }
                        }
                        else {
                           strncpy(says,"beacon: Rfport ",101u);
                           aprsstr_Append(says, 101ul,
                (char *)(tmp = (char)(i+49UL),&tmp), 1u/1u);
                           aprsstr_Append(says, 101ul, " not enabled", 13ul);
                        }
                     }
                  }
                  if (says[0U]) useri_say(says, 101ul, 2UL, 'b');
               }
               else {
                  strncpy(h,"rfbeacon not decodeable: ",101u);
                  aprsstr_Append(h, 101ul, s, 512ul);
                  useri_xerrmsg(h, 101ul);
               }
            }
         }
         shift += bshift; /* delay next beacon by shift time to before beacon */
      }
      ++nb;
   }
   rfbecondone = aprsdecode_realtime;
} /* end rfbeacons() */


static void startserial(struct xosi_PROCESSHANDLE * pid, uint8_t cfg)
{
   char h[1001];
   char s[1001];
   if (useri_configon(cfg)) {
      if (!(pid->runs || pid->started)) {
         useri_confstr(cfg, s, 1001ul);
         if (s[0U]) {
            xosi_StartProg(s, 1001ul, pid);
            pid->started = 1;
            if (pid->runs) {
               useri_say("Start Serial Interface", 23ul, 3UL, 'g');
            }
            else {
               strncpy(h,"can not start ",1001u);
               aprsstr_Append(h, 1001ul, s, 1001ul);
               useri_xerrmsg(h, 1001ul);
               useri_configbool(cfg, 0);
               useri_refresh = 1;
            }
         }
      }
   }
   else pid->started = 0;
} /* end startserial() */


extern void aprsdecode_tcpjobs(void)
{
   aprsdecode_pTCPSOCK acttcp;
   int32_t i;
   int32_t len;
   char ok0;
   char s[1000];
   struct aprsdecode_TCPSOCK * anonym;
   struct aprsdecode_UDPSOCK * anonym0;
   if (qwatchtime!=aprsdecode_realtime) {
      acttcp = aprsdecode_tcpsocks;
      while (acttcp) {
         { /* with */
            struct aprsdecode_TCPSOCK * anonym = acttcp;
            Timebeacon(acttcp);
            if (anonym->watchtime+180UL<aprsdecode_realtime) {
               Watchbeacon(acttcp);
            }
            if (useri_configon(useri_fALLOWGATE)) WatchTXQ(acttcp);
            if (useri_isupdated(useri_fSERVERFILT)) updatefilters(acttcp);
            acttcp = anonym->next;
         }
      }
      qwatchtime = aprsdecode_realtime;
      Gateconn(&aprsdecode_tcpsocks);
      rfbeacons();
   }
   /*UDP */
   for (i = 0L; i<=3L; i++) {
      { /* with */
         struct aprsdecode_UDPSOCK * anonym0 = &aprsdecode_udpsocks0[i];
         if (useri_configon((uint8_t)(38UL+(uint32_t)i))
                && !useri_isupdated((uint8_t)(38UL+(uint32_t)i))) {
            if ((int32_t)anonym0->fd<0L) {
               ok0 = 0;
               useri_confstr((uint8_t)(38UL+(uint32_t)i), s, 1000ul);
               if (s[0U]) {
                  memset((char *) &aprsdecode_udpsocks0[i],(char)0,
                sizeof(struct aprsdecode_UDPSOCK));
                  anonym0->fd = -1L;
                  if (aprsstr_GetIp2(s, 1000ul, &anonym0->ip,
                &anonym0->dport, &anonym0->bindport, &anonym0->checkip)<0L) {
                     anonym0->rawread = 1;
                     porttext((uint32_t)i, s, 1000ul);
                     aprsstr_Append(s, 1000ul, " Config Error <IPNumber:TxPor\
t:RxPort>", 39ul);
                     /*              textautosize(0, 5, 6, 10, "e", s); */
                     useri_xerrmsg(s, 1000ul);
                  }
                  else {
                     anonym0->fd = openudp();
                     anonym0->rawread = 1;
                     porttext((uint32_t)i, s, 1000ul);
                     if ((int32_t)anonym0->fd<0L || bindudp(anonym0->fd,
                anonym0->bindport)<0L) {
                        aprsstr_Append(s, 1000ul, " Bindport Error, Illegal (\
Rx)Port or in Use", 44ul);
                        /*                textautosize(0, 5, 6, 10, "e", s);
                */
                        useri_xerrmsg(s, 1000ul);
                        osic_CloseSock(anonym0->fd);
                        anonym0->fd = -1L;
                     }
                     else {
                        aprsstr_Append(s, 1000ul, " Open", 6ul);
                        len = socknonblock(anonym0->fd);
                        useri_textautosize(0L, 5L, 6UL, 5UL, 'b', s, 1000ul);
                        ok0 = 1;
                     }
                  }
               }
               else {
                  porttext((uint32_t)i, s, 1000ul);
                  aprsstr_Append(s, 1000ul, " Not Configured", 16ul);
                  /*            textautosize(0, 5, 6, 10, "e", s); */
                  useri_xerrmsg(s, 1000ul);
               }
               if (!ok0) {
                  useri_configbool((uint8_t)(38UL+(uint32_t)i), 0);
               }
            }
         }
         else if ((int32_t)anonym0->fd>=0L) {
            osic_CloseSock(anonym0->fd);
            anonym0->fd = -1L;
            anonym0->starttime = 0UL;
            porttext((uint32_t)i, s, 1000ul);
            aprsstr_Append(s, 1000ul, " Close", 7ul);
            /*        textautosize(0, 5, 6, 4, "e", s); */
            useri_xerrmsg(s, 1000ul);
         }
      }
   } /* end for */
   startserial(&aprsdecode_serialpid, useri_fSERIALTASK);
   startserial(&aprsdecode_serialpid2, useri_fSERIALTASK2);
   sendmsg();
   /*WrInt(tickertime, 15); WrInt(realtime, 15); WrLn; */
   if (tickertime+60UL<aprsdecode_realtime) aprsdecode_Stopticker();
} /* end tcpjobs() */


static void beepprox(float d, float km)
{
   if ((useri_configon(useri_fBEEPPROX) && km!=0.0f) && d<km) {
      d = 1.0f-X2C_DIVR(d,km);
      d = d*d;
      d = d*d;
      aprsdecode_beeplim(100L, aprsdecode_trunc((1.0f-d)*(float)useri_conf2int(useri_fBEEPPROX,
                 0UL, 20L, 8000L,
                400L)+d*(float)useri_conf2int(useri_fBEEPPROX, 2UL, 20L,
                8000L, 400L)), aprsdecode_trunc((1.0f-d)*(float)useri_conf2int(useri_fBEEPPROX,
                 1UL, 0L, 5000L, 40L)+d*(float)useri_conf2int(useri_fBEEPPROX,
                 3UL, 0L, 5000L, 1000L)));
   }
} /* end beepprox() */

#define aprsdecode_WARNFN "proxwarn"

#define aprsdecode_LOOKBACK 180
/* last s look if nearest pos is last */


static void approxywarn(struct aprsstr_POSITION pos, const char call[],
                uint32_t call_len)
{
   char s[31];
   char h[101];
   float d;
   float mino;
   float mind;
   float km;
   struct aprsstr_POSITION mypos;
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pf;
   int32_t fd;
   km = useri_conf2real(useri_fAPPROXY, 0UL, 0.0f, 30000.0f, 0.0f);
   if (km!=0.0f) {
      if (aprstext_getmypos(&mypos)) {
         d = aprspos_distance(pos, mypos);
         if (d<=km) {
            /* quick check if it is in radius */
            op = aprsdecode_ophist0;
            while (op && X2C_STRCMP(op->call,9u,call,call_len)) {
               op = op->next;
            }
            if (op) {
               mino = 30000.0f;
               mind = 30000.0f;
               pf = op->frames;
               while (pf) {
                  if (pf->time0+180UL>aprsdecode_realtime && (pf->nodraw&0x4FU)
                ==0U) {
                     d = aprspos_distance(pf->vardat->pos, mypos);
                     /*
                     FixToStr(distance(pf^.vardat^.pos, mypos), 4, s);
                WrStr(s); WrStr(" ");
                     FixToStr(distance(mypos, pf^.vardat^.pos), 4, s);
                WrStr(s); WrStr(" ");
                     postostr(pf^.vardat^.pos, s); WrStr(s); WrStr(" ");
                postostr(mypos, s); WrStrLn(s);
                     */
                     mino = mind;
                     if (d<mind) mind = d;
                  }
                  pf = pf->next;
               }
               if ((mino<29000.0f && mino!=d) && mino>d) {
                  beepprox(d, km);
                  strncpy(h,"approach ",101u);
                  aprsstr_FixToStr(d, 3UL, s, 31ul);
                  aprsstr_Append(h, 101ul, s, 31ul);
                  aprsstr_Append(h, 101ul, "km ", 4ul);
                  aprsstr_Append(h, 101ul, call, call_len);
                  useri_textautosize(0L, 5L, 6UL, 120UL, 'r', h, 101ul);
                  if (!osi_Exists("proxwarn", 9ul)) {
                     fd = osi_OpenWrite("proxwarn", 9ul);
                     if (osic_FdValid(fd)) {
                        aprsstr_Assign(h, 101ul, call, call_len);
                        aprsstr_IntToStr((int32_t)
                aprsdecode_trunc(d*1000.0f), 0UL, s, 31ul);
                        aprsstr_Append(h, 101ul, " p ", 4ul);
                        aprsstr_Append(h, 101ul, s, 31ul);
                        osi_WrBin(fd, (char *)h, 101u/1u,
                aprsstr_Length(h, 101ul));
                        osic_Close(fd);
                     }
                  }
               }
            }
         }
      }
   }
} /* end approxywarn() */

#define aprsdecode_DUPTIME 27

#define aprsdecode_NN "n"

#define aprsdecode_BAD "x"

#define aprsdecode_VIA "v"


static void app0(uint32_t * viac, aprsdecode_FRAMEBUF tb, uint32_t * tp,
                char c)
{
   if (*tp<511UL) {
      tb[*tp] = c;
      ++*tp;
   }
   if (c==',') ++*viac;
} /* end app() */


static char via(char words[], uint32_t words_len,
                aprsdecode_FRAMEBUF b, char typ, uint32_t from,
                uint32_t to)
{
   uint32_t i;
   char h[31];
   if (from>=to) return 0;
   h[0U] = typ;
   i = 1UL;
   while (i<28UL && from<to) {
      h[i] = b[from];
      ++from;
      ++i;
   }
   h[i] = ' ';
   h[i+1UL] = 0;
   return aprsstr_InStr(words, words_len, h, 31ul)>=0L;
} /* end via() */


static char num0(aprsdecode_FRAMEBUF b, uint32_t i, uint32_t * n)
{
   if ((i<511UL && (uint8_t)b[i]>='0') && (uint8_t)b[i]<='9') {
      *n = (uint32_t)(uint8_t)b[i]-48UL;
      return 1;
   }
   return 0;
} /* end num() */


static char scanpath(char words[], uint32_t words_len,
                uint32_t * tp, aprsdecode_FRAMEBUF tb, uint32_t * viac,
                aprsdecode_FRAMEBUF b, uint32_t * p, uint32_t to,
                char store)
{
   uint32_t hp;
   hp = *p;
   while (*p<to) {
      if (store) app0(viac, tb, tp, b[*p]);
      if (*p+1UL>=to || b[*p]==',') {
         if (via(words, words_len, b, 'x', hp+1UL,
                *p+(uint32_t)(*p+1UL>=to))) return 0;
         /* bad word */
         hp = *p;
      }
      ++*p;
   }
   return 1;
} /* end scanpath() */


static void digi(const aprsdecode_FRAMEBUF b, char fromrf,
                uint32_t outport, char directonly,
                char appendrest, uint32_t nomovetime,
                uint32_t radius, char words[], uint32_t words_len,
                int32_t * txbyte)
{
   struct aprsdecode_DAT dat;
   uint32_t pn;
   uint32_t viac;
   uint32_t n2;
   uint32_t n1;
   uint32_t n;
   uint32_t ssid;
   uint32_t len;
   uint32_t tp;
   uint32_t p;
   aprsdecode_FRAMEBUF tb;
   /*    hashl, hashh:SET8; */
   uint32_t dt;
   char norout;
   struct aprsstr_POSITION mypos;
   char s2[31];
   char s1[31];
   char mycall[31];
   *txbyte = 0L;
   if (nomovetime<27UL) nomovetime = 27UL;
   if (fromrf) {
      tp = 0UL;
      p = 0UL;
      len = 0UL;
      ssid = 0UL;
      viac = 0UL;
      useri_confstr(useri_fMYCALL, mycall, 31ul);
      if (aprstext_callwrong(mycall, 31ul)) {
         useri_xerrmsg("Digi needs \'My Call\'", 21ul); /* no mycall */
         return;
      }
      norout = aprsstr_InStr(words, words_len, " x-", 4ul)>=0L;
                /* ssid routing off */
      if (aprsstr_InStr(words, words_len, " xi", 4ul)<0L) {
         aprsstr_Append(words, words_len, " v", 3ul);
                /* if not disabled append mycall for via */
         aprsstr_Append(words, words_len, mycall, 31ul);
      }
      aprsstr_Append(words, words_len, " x", 3ul); /* mycall as badword */
      aprsstr_Append(words, words_len, mycall, 31ul);
      aprsstr_Append(words, words_len, " ", 2ul);
      while ((len<511UL && b[len]) && b[len]!=':') ++len;
      while (p<len && b[p]!='>') {
         app0(&viac, tb, &tp, b[p]); /* end of src call */
         ++p;
      }
      if (via(words, words_len, b, 'x', 0UL, p)) return;
      /* bad src call */
      if (useri_configon(useri_fMUSTBECALL)) {
         tb[tp] = 0;
         if (!aprstext_isacall(tb, 512ul)) return;
      }
      n = p+1UL;
      while ((p<len && b[p]!='-') && b[p]!=',') {
         app0(&viac, tb, &tp, b[p]); /* ssid or end of dst call */
         ++p;
      }
      if (via(words, words_len, b, 'x', n, p)) return;
      /* bad dest call */
      if (b[p]=='-') {
         /* we have a ssid */
         ++p;
         if (num0(b, p, &n)) {
            ssid = n;
            ++p;
            if (num0(b, p, &n)) {
               ssid = ssid*10UL+n;
               ++p;
            }
         }
      }
      if (ssid==0UL || ssid>7UL) norout = 1;
      n = p;
      while (n<len && b[n]!='*') ++n;
      if (n+1UL<len) norout = 1;
      if (!norout) {
         /* use ssid routing */
         if (appendrest) --ssid;
         else ssid = 0UL;
      }
      if (ssid>0UL) {
         /* write old or new ssid */
         app0(&viac, tb, &tp, '-');
         if (ssid>=10UL) app0(&viac, tb, &tp, '1');
         app0(&viac, tb, &tp, (char)(ssid%10UL+48UL));
      }
      if (n<len) {
         if (directonly) return;
         /* not direct heard */
         if (!scanpath(words, words_len, &tp, tb, &viac, b, &p, n, 1)) {
            return;
         }
         /* append path till * and find badwords */
         ++p;
      }
      app0(&viac, tb, &tp, ',');
      n = 0UL;
      while (mycall[n]) {
         app0(&viac, tb, &tp, mycall[n]); /* append mycall */
         ++n;
      }
      app0(&viac, tb, &tp, '*');
      if (p<len) {
         /* look for via routing */
         norout = 1;
         if (b[p]==',') ++p;
         n = p;
         while (p<len && b[p]!=',') ++p;
         if (((((((via(words, words_len, b, 'n', n, p-3UL) && num0(b, p-3UL,
                &n1)) && n1>0UL) && n1<=7UL) && b[p-2UL]=='-') && num0(b,
                p-1UL, &n2)) && n2>0UL) && n2<=n1) {
            if (directonly && n1!=n2) {
               return;
            }
            /* not direct heard */
            if (appendrest && n2>1UL) {
               /* append not n-0 */
               app0(&viac, tb, &tp, ',');
               pn = n;
               while (pn+3UL<p) {
                  app0(&viac, tb, &tp, b[pn]); /* append via word */
                  ++pn;
               }
               app0(&viac, tb, &tp, (char)(n1+48UL)); /* append n-n */
               app0(&viac, tb, &tp, '-');
               app0(&viac, tb, &tp, (char)(n2+47UL));
            }
            norout = 0; /* append rest path later*/
         }
         else if (via(words, words_len, b, 'v', n, p)) norout = 0;
      }
      if (norout) return;
      /* nothing to digipeat */
      if (!scanpath(words, words_len, &tp, tb, &viac, b, &p, len,
                appendrest)) return;
      /* append rest path and find badwords */
      if (viac>7UL) return;
      /* too much via */
      p = len;
      while (p<511UL && b[p]) {
         app0(&viac, tb, &tp, b[p]);
         ++p;
      }
      app0(&viac, tb, &tp, 0);
   }
   else memcpy(tb,b,512u);
   if (aprsdecode_Decode(tb, 512ul, &dat)<0L) return;
   /*radius */
   if (dat.type!=aprsdecode_MSG && radius>0UL) {
      if (!aprstext_getmypos(&mypos)) {
         useri_xerrmsg("Digi with Radius needs MY Position", 35ul);
         return;
      }
      if (!aprspos_posvalid(dat.pos) || aprspos_distance(mypos,
                dat.pos)>(float)radius) return;
   }
   if (dat.type==aprsdecode_MSG && aprstext_isacall(dat.msgto, 9ul)) {
      dt = 27UL; /* real messages fast dupetime */
   }
   else dt = nomovetime;
   if (outport>0UL && outport-1UL<=3UL) {
      *txbyte = Sendudp(tb, 512ul, outport-1UL, dt);
      if (*txbyte<0L) {
         strncpy(s1,"aprsdigi:can not send to port ",31u);
         aprsstr_IntToStr((int32_t)outport, 0UL, s2, 31ul);
         aprsstr_Append(s1, 31ul, s2, 31ul);
         useri_xerrmsg(s1, 31ul);
      }
   }
} /* end digi() */

#define aprsdecode_MAXPAYLOAD 200
/* spare min 36 byte for 3rd party header */

#define aprsdecode_MSGMHTIME 3600
/* last heard age for msgs sending */


static void digitorf(const aprsdecode_FRAMEBUF b, uint32_t outport,
                uint32_t nomovetime, uint32_t radius, int32_t maxbps,
                char words[], uint32_t words_len)
{
   uint32_t j;
   uint32_t i;
   int32_t sentbytes;
   aprsdecode_pOPHIST op;
   char ok0;
   aprsdecode_FRAMEBUF tb;
   struct aprsdecode_DAT dat;
   char via0[201];
   char ww[201];
   char w[201];
   /*"OH4ZZZ-5>APZ123,WIDE2-2:}OH2XYZ-11>APZYXW-4,TCPIP,OH4ZZZ-5*:>packet  "*/
   /*WrStr(words); WrStrLn(b); */
   if (aprsdecode_Decode(b, 512ul, &dat)<0L || dat.igatep==0UL) return;
   aprsstr_Assign(w, 201ul, words, words_len);
   do {
      aprsstr_Extractword(w, 201ul, ww, 201ul);
   } while (!(ww[0U]==0 || ww[0U]=='n'));
   if (ww[0U]==0) return;
   aprsstr_Delstr(ww, 201ul, 0UL, 1UL);
   if (!aprsstr_StrToInt(ww, 201ul, &maxbps)) return;
   aprsstr_Extractword(w, 201ul, via0, 201ul); /* via path */
   if (via0[0U]=='n') aprsstr_Delstr(via0, 201ul, 0UL, 1UL);
   else via0[0U] = 0;
   if (maxbps<0L) {
      if (dat.type!=aprsdecode_MSG) return;
      /* if neg bps send msg only */
      /*WrStr(dat.msgto);  WrStrLn(" msgto"); */
      op = findop(dat.msgto, 1);
      /*IF (op<>NIL) THEN WrStr(op^.lastrxport); WrStrLn(" opok") END; */
      if ((op==0 || op->lastrxport!=(char)(outport+48UL))
                || op->lasttime+3600UL<aprsdecode_realtime) return;
   }
   /* user not heard on this port */
   if (!Checkbps(outport-1UL, (uint32_t)labs(maxbps), 0UL,
                dat.type==aprsdecode_MSG)) return;
   /* -bps only msg */
   useri_confstr(useri_fMYCALL, tb, 512ul);
   if (aprstext_callwrong(tb, 512ul)) {
      useri_xerrmsg("Gate to rf needs valid \'My Call\'", 33ul);
      return;
   }
   aprsstr_Append(tb, 512ul, ">APLM01", 8ul);
   if (via0[0U]) {
      if (via0[0U]!='-') aprsstr_Append(tb, 512ul, ",", 2ul);
      else if (((uint8_t)via0[1U]<'1' || (uint8_t)via0[1U]>'9')
                || via0[2U]) return;
      /* not correct ssid */
      aprsstr_Append(tb, 512ul, via0, 201ul);
   }
   aprsstr_Append(tb, 512ul, ":}", 3ul);
   aprsstr_Append(tb, 512ul, dat.srcall, 9ul);
   aprsstr_Append(tb, 512ul, ">", 2ul);
   aprsstr_Append(tb, 512ul, dat.dstcall, 9ul);
   aprsstr_Append(tb, 512ul, ",TCPIP,", 8ul);
   aprsstr_Append(tb, 512ul, dat.viacalls[dat.igatep], 9ul);
   aprsstr_Append(tb, 512ul, "*:", 3ul);
   i = dat.payload;
   j = aprsstr_Length(tb, 512ul);
   while (((i<=511UL && b[i]) && b[i]!='\015') && j<511UL) {
      tb[j] = b[i];
      ++j;
      ++i;
   }
   tb[j] = 0;
   if (i-dat.payload>200UL) return;
   /* frame too long */
   w[0] = 0;
   digi(tb, 0, outport, 0, 1, nomovetime, radius, w, 201ul, &sentbytes);
   if (sentbytes>0L) {
      ok0 = Checkbps(outport-1UL, (uint32_t)labs(maxbps),
                (uint32_t)sentbytes, 0); /* sum up txbytes */
   }
} /* end digitorf() */


static void digipeat(const aprsdecode_FRAMEBUF b, uint32_t udpch)
{
   uint32_t line;
   uint32_t outport;
   uint32_t radius;
   int32_t sentbytes;
   uint32_t nomovetime;
   char directonly;
   char appendrest;
   char words[201];
   char word[11];
   char cin;
   char c;
   line = 0UL;
   for (;;) {
      useri_confstrings(useri_fDIGI, line, 0, words, 201ul);
      if (words[0U]==0) break;
      cin = words[0U]; /* input port */
      if ((cin=='*' && udpch>0UL || cin==(char)(udpch+48UL))
                || cin=='N' && udpch==0UL) {
         /* input port, 0 is from net */
         c = words[1U]; /* output port */
         if ((uint8_t)c>='1') outport = (uint32_t)(uint8_t)c-48UL;
         else outport = 0UL;
         directonly = 1;
         appendrest = 0;
         switch ((unsigned)words[2U]) { /* direct/delpath */
         case '1':
            appendrest = 1;
            break;
         case '2':
            directonly = 0;
            break;
         case '3':
            directonly = 0;
            appendrest = 1;
            break;
         } /* end switch */
         aprsstr_Extractword(words, 201ul, word, 11ul);
         aprsstr_Extractword(words, 201ul, word, 11ul);
         if (aprsstr_StrToCard(word, 11ul, &nomovetime)) {
            aprsstr_Extractword(words, 201ul, word, 11ul);
            if (aprsstr_StrToCard(word, 11ul, &radius)) {
               if (cin=='N') {
                  digitorf(b, outport, nomovetime, radius, -5L, words,
                201ul);
               }
               else if (udpch>0UL) {
                  digi(b, 1, outport, directonly, appendrest, nomovetime,
                radius, words, 201ul, &sentbytes);
               }
            }
         }
      }
      ++line;
   }
} /* end digipeat() */

#define aprsdecode_PURGEINTERVALL 20
/* seconds+1 with no purge */


static void storedata(aprsdecode_FRAMEBUF mb, aprsdecode_pTCPSOCK cp,
                uint32_t udpch, const struct UDPSET modeminfo,
                char valid, char local, char withchk)
{
   int32_t res;
   char fn[1000];
   char s[21];
   aprsdecode_MONCALL server;
   char tabchk;
   char ok0;
   aprsdecode_pTCPSOCK tp;
   struct aprsdecode_DAT dat;
   char dir;
   uint32_t otime;
   char tmp;
   if (aprsdecode_lastpurge!=aprsdecode_realtime/20UL) {
      if (aprsdecode_lums.logmode) {
         aprsdecode_purge(&aprsdecode_ophist2,
                aprsdecode_realtime-aprsdecode_lums.purgetime,
                aprsdecode_realtime-aprsdecode_lums.purgetimeobj);
      }
      else {
         aprsdecode_purge(&aprsdecode_ophist0,
                aprsdecode_realtime-aprsdecode_lums.purgetime,
                aprsdecode_realtime-aprsdecode_lums.purgetimeobj);
      }
      aprsdecode_lastpurge = aprsdecode_realtime/20UL;
   }
   if (cp) memcpy(server,cp->user.call,9u);
   else server[0UL] = 0;
   /*WrInt(rxidle, 15); WrInt(lastlooped+IGATEMAXDELAY, 15);
                WrInt(time(), 15); WrStrLn(" ig"); */
   if (local) res = 0L;
   else if (aprsdecode_rxidle>=3UL && aprsdecode_lastlooped+5UL>=osic_time())
                 {
      /* not too long delayd data */
      digipeat(mb, udpch);
      res = AprsIs(mb, 512ul, 6UL, server, 9ul, udpch, valid,
                useri_configon(useri_fMUSTBECALL), &tabchk);
      if (((res>=0L && useri_configon(useri_fALLOWGATE)) && !tabchk)
                && udpch>0UL) {
         useri_confstr(useri_fALLOWGATE, s, 21ul);
         if (aprsstr_InStr(s, 21ul,
                (char *)(tmp = (char)(udpch+48UL),&tmp), 1u/1u)>=0L) {
            tp = aprsdecode_tcpsocks;
            while (tp) {
               if (tp->lastping>0UL && (tp->waitpong==1U && tp->lastping+10UL<aprsdecode_realtime || tp->waitpong>1U)
                ) {
                  if (tp->lastpong>0UL) {
                     saylinkbad((int32_t)(aprsdecode_realtime-tp->lastpong)
                );
                  }
                  useri_xerrmsg("Ping-Timeout, Rf to AprsIs transfer stopped \
temporarily", 56ul);
               }
               else ok0 = Sendtcp(tp, mb);
               tp = tp->next;
            }
         }
      }
   }
   else {
      /* delayed */
      res = 0L;
      if (aprsdecode_verb) {
         osi_WrStrLn("---discarded delayed digpeat-frame", 35ul);
      }
   }
   otime = 0UL;
   if (aprsdecode_verb && udpch>0UL) {
      osic_WrINT32((uint32_t)res, 3UL);
      osi_WrStr(" udp ", 6ul);
      osi_WrStrLn(mb, 512ul);
   }
   if (res>=0L && aprsdecode_Decode(mb, 512ul, &dat)>=0L) {
      if (udpch>0UL) dat.lastrxport = (char)(udpch+48UL);
      else dat.lastrxport = 'N';
      if (dat.type==aprsdecode_MSG) {
         if (udpch>0UL) getmessage(0, udpch, dat);
         else getmessage(cp, 0UL, dat);
      }
      if (aprsdecode_lums.logmode) {
         res = aprsdecode_Stoframe(&aprsdecode_ophist2, mb, 512ul,
                aprsdecode_realtime, 0, &otime, dat);
      }
      else {
         /*IF logdone THEN*/
         res = aprsdecode_Stoframe(&aprsdecode_ophist0, mb, 512ul,
                aprsdecode_realtime, 0, &otime, dat);
      }
      if (withchk && aprsdecode_logdone) aprsdecode_Checktracks();
      if (!local) {
         if (useri_configon(useri_fLOGWFN)) {
            useri_confstr(useri_fLOGWFN, fn, 1000ul);
            aprstext_logfndate(aprsdecode_realtime, fn, 1000ul);
            if (!wrlog(mb, 512ul, aprsdecode_realtime, fn, 1000ul, res)) {
               poplogerr(fn, 1000ul);
            }
         }
         if (aprspos_posvalid(dat.pos) && useri_configon(useri_fAPPROXY)) {
            approxywarn(dat.pos, dat.symcall, 9ul);
         }
      }
   }
   if (res==-2L) {
      dir = '#';
   }
   else if (res<0L) dir = '-';
   else dir = '>';
   if (udpch>0UL) wrmon(udpch, dir, modeminfo, otime, mb, 512ul);
   else wrmon(0UL, dir, modeminfo, otime, mb, 512ul);
} /* end storedata() */


extern void aprsdecode_drawbeacon(char raw[], uint32_t raw_len)
{
   aprsdecode_FRAMEBUF b;
   char port;
   uint32_t time0;
   struct UDPSET modeminfo;
   aprsstr_Assign(b, 512ul, raw, raw_len);
   if (getbeaconparm(b, 512ul, &time0, &port)) {
      beaconmacros(b, 512ul, "", 1ul, "", 1ul, 1);
      if (b[0UL]) {
         memset((char *) &modeminfo,(char)0,sizeof(struct UDPSET));
         if (aprsdecode_lums.logmode) {
            useri_say("Not Drawn to imported Log!", 27ul, 5UL, 'r');
         }
         else {
            storedata(b, 0, 0UL, modeminfo, 0, 1, 1);
            if (aprsdecode_click.mhop[0UL]) {
               useri_say("switch to \"show all\" to see new object", 39ul,
                5UL, 'r');
            }
         }
      }
   }
   else useri_xerrmsg("decode error", 13ul);
   aprsdecode_tracenew.winevent = 2000UL;
} /* end drawbeacon() */

static aprsstr_GHOSTSET _cnst2 = {0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,
                0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,
                0xFFFFFFFFUL,0x00000001UL};
static aprsstr_GHOSTSET _cnst1 = {0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL};

static void getghostset(uint32_t port, aprsstr_GHOSTSET g)
{
   uint32_t n;
   uint32_t i;
   char s[11];
   char p;
   p = (char)(port+49UL);
   memcpy(g,_cnst1,36u);
   i = 0UL;
   for (;;) {
      useri_conf2str(useri_fFINGERPRINT, 0UL, i, 1, s, 11ul);
      if (s[0U]==0) break;
      if (s[0U]==p) {
         if (s[1U]==':') {
            aprsstr_Delstr(s, 11ul, 0UL, 2UL);
            if (aprsstr_StrToCard(s, 11ul, &n) && n<=256UL) {
               X2C_INCL(g,n,257);
            }
         }
         else if (s[1U]==0) memcpy(g,_cnst2,36u);
      }
      ++i;
   }
} /* end getghostset() */


extern void aprsdecode_udpin(uint32_t port)
{
   aprsdecode_FRAMEBUF mbuf;
   struct UDPSET modeminfo;
   aprsstr_GHOSTSET gs;
   struct aprsdecode_UDPSOCK * anonym;
   getghostset(port, gs);
   while (getudp(port, mbuf, &modeminfo, gs)) {
      if (mbuf[0UL]) {
         { /* with */
            struct aprsdecode_UDPSOCK * anonym = &aprsdecode_udpsocks0[port];
                
            anonym->lastudprx = aprsdecode_realtime;
            ++anonym->rxframes;
            anonym->rxbytes += aprsstr_Length(mbuf, 512ul);
            if (anonym->starttime==0UL) {
               anonym->starttime = aprsdecode_realtime;
            }
         }
         aprsdecode_lastanyudprx = aprsdecode_realtime;
         storedata(mbuf, 0, port+1UL, modeminfo, 0, 0, 1);
      }
      else if (aprsdecode_verb) osi_WrStrLn("axudp decode error", 19ul);
   }
} /* end udpin() */


extern int32_t aprsdecode_tcpout(aprsdecode_pTCPSOCK acttcp)
{
   int32_t i;
   int32_t res;
   struct aprsdecode_TCPSOCK * anonym;
   int32_t tmp;
   if ((int32_t)acttcp->fd>=0L) {
      { /* with */
         struct aprsdecode_TCPSOCK * anonym = acttcp;
         res = sendsock(anonym->fd, anonym->tbuf, anonym->tlen);
         if (res>0L) {
            tmp = anonym->tlen-1L;
            i = res;
            if (i<=tmp) for (;; i++) {
               anonym->tbuf[i-res] = anonym->tbuf[i];
               if (i==tmp) break;
            } /* end for */
            anonym->tlen -= res;
            if (anonym->connt==0UL) {
               anonym->connt = aprsdecode_realtime;
               wrconnected(acttcp);
            }
            anonym->watchtime = aprsdecode_realtime;
         }
      }
   }
   else {
      /*    ELSE tlen:=0;                        (* should not happen *) */
      res = -1L;
   }
   return res;
} /* end tcpout() */


extern void aprsdecode_tcpin(aprsdecode_pTCPSOCK acttcp)
{
   int32_t res;
   aprsdecode_FRAMEBUF mbuf;
   struct UDPSET modeminfo;
   uint32_t tlim;
   if ((int32_t)acttcp->fd>=0L) {
      tlim = aprsdecode_realtime;
      for (;;) {
         res = Gettcp(acttcp->fd, mbuf, acttcp->rbuf, &acttcp->rpos);
         /*WrInt(res, 1);WrStrLn(" gettcp"); */
         if (res<0L) {
            /*WrStrLn("closesock"); */
            osic_CloseSock(acttcp->fd);
            acttcp->fd = -1L;
            break;
         }
         if (res<=0L) break;
         ++acttcp->rxframes;
         acttcp->rxbytes += aprsstr_Length(mbuf, 512ul);
         aprsdecode_realtime = osic_time();
         acttcp->watchtime = aprsdecode_realtime;
         /*    WrMon(mbuf); */
         if (mbuf[0UL]=='#') getpong(acttcp, mbuf, 512ul);
         memset((char *) &modeminfo,(char)0,sizeof(struct UDPSET));
         storedata(mbuf, acttcp, 0UL, modeminfo, 1, 0, 0);
         aprsdecode_lasttcprx = aprsdecode_realtime;
         if (tlim+6UL<aprsdecode_realtime) break;
      }
      if (aprsdecode_logdone) aprsdecode_Checktracks();
   }
} /* end tcpin() */


extern void aprsdecode_initparms(void)
{
   uint32_t i;
   struct aprsdecode__D2 * anonym;
   initcrc12();
   memset((char *)aprsdecode_gateways,(char)0,
                sizeof(struct aprsdecode__D4 [10]));
   memset((char *)aprsdecode_udpsocks0,(char)0,
                sizeof(struct aprsdecode_UDPSOCK [4]));
   for (i = 0UL; i<=3UL; i++) {
      aprsdecode_udpsocks0[i].fd = -1L;
   } /* end for */
   connecttime = 0UL;
   cycleservers = 0UL;
   logframename[0UL] = 0;
   logframes = 6L;
   aprsdecode_testbeaconbuf[0UL] = 0;
   /*  servercall[0]:=0C;  */
   /*  passwd[0]:=0C; */
   /*  igatedelay:=TRUE; */
   rfbeacont = 0UL;
   qas = 0UL;
   qasc = 0UL;
   aprsdecode_initxsize = 0L;
   aprsdecode_initysize = 0L;
   aprsdecode_initzoom = 0L;
   aprsdecode_finezoom = 1.0f;
   aprsstr_posinval(&aprsdecode_mappos);
   aprsdecode_spikesens = 2.0f;
   aprsdecode_maxhop = 100.0f;
   maxspeed = 1000.0f;
   nextbeep = 0UL;
   { /* with */
      struct aprsdecode__D2 * anonym = &aprsdecode_lums;
      anonym->map = 350L;
      anonym->rfbri = 350L;
      anonym->track = 1000L;
      anonym->waypoint = 800L;
      anonym->sym = 1000L;
      anonym->obj = 1000L;
      anonym->text = 750L;
      anonym->nomov = 90L;
      anonym->maxdim = 3600UL;
      anonym->firstdim = 82800UL;
      anonym->purgetime = 90000UL;
      anonym->purgetimeobj = 90000UL;
      anonym->dupemaxtime = 10800UL;
      anonym->fps = 0L;
      /*  mapmode:=REFLATER; */
      strncpy(anonym->mapname,"tiles",41u);
      useri_Tilegamma0(0.45f);
      anonym->gamma = 2.2f;
      anonym->centering = 40L;
      anonym->movestep = 10UL;
      strncpy(anonym->configfn,"aprsmap.cfg",257u);
      anonym->wxcol = 0;
   }
   aprsdecode_lastpurge = 0UL;
   aprsdecode_lastanyudprx = 0UL;
   aprsdecode_lasttcprx = 0UL;
   strncpy(aprsdecode_mapdir,"osm",1025u);
   parms();
   aprsdecode_ophist0 = 0;
   aprsdecode_ophist2 = 0;
   aprsdecode_txmessages = 0;
   aprsdecode_msgfifo0 = 0;
   msgmid = 0UL;
   aprsdecode_realtime = osic_time();
   uptime = aprsdecode_realtime;
   trygate = 0UL;
   dupetime = 60UL;
   ungate[0U][0] = 0;
   strncpy(ungate[1U],"NOGATE",9u);
   strncpy(ungate[2U],"RFONLY",9u);
   strncpy(ungate[3U],"TCPIP",9u);
   strncpy(ungate[4U],"TCPXX",9u);
   strncpy(ungate[5U],"q",9u);
   ungate[6U][0] = 0;
   aprsdecode_tcpsocks = 0;
   aprsdecode_Stopticker();
   tickertime = 0UL;
   sentemptymsg = 0;
   beaconrandomstart = osic_time();
                /* set a beacon scheduler start second in minute */
   beaconrandomstart = (beaconrandomstart/60UL)%60UL+(beaconrandomstart%60UL)
                *60UL;
} /* end initparms() */


extern void aprsdecode_BEGIN(void)
{
   static int aprsdecode_init = 0;
   if (aprsdecode_init) return;
   aprsdecode_init = 1;
   if (sizeof(aprsdecode_SET256)!=32) X2C_ASSERT(0);
   if (sizeof(aprsdecode_MONCALL)!=9) X2C_ASSERT(0);
   if (sizeof(aprsdecode_FRAMEBUF)!=512) X2C_ASSERT(0);
   if (sizeof(aprsdecode_WWWB)!=1401) X2C_ASSERT(0);
   if (sizeof(aprsdecode_FILENAME)!=1024) X2C_ASSERT(0);
   if (sizeof(aprsdecode_DUPETIMES)!=262144) X2C_ASSERT(0);
   if (sizeof(aprsdecode_MSGTEXT)!=67) X2C_ASSERT(0);
   if (sizeof(aprsdecode_ACKTEXT)!=5) X2C_ASSERT(0);
   if (sizeof(aprsdecode_ACKREPTEXT)!=2) X2C_ASSERT(0);
   if (sizeof(uint8_t)!=1) X2C_ASSERT(0);
   if (sizeof(uint8_t)!=1) X2C_ASSERT(0);
   if (sizeof(uint16_t)!=2) X2C_ASSERT(0);
   if (sizeof(aprsdecode_TELEMETRY)!=14) X2C_ASSERT(0);
   if (sizeof(uint16_t)!=2) X2C_ASSERT(0);
   if (sizeof(aprsdecode_MAPNAME)!=41) X2C_ASSERT(0);
   if (sizeof(aprsdecode_SYMBOLSET)!=24) X2C_ASSERT(0);
   if (sizeof(aprsdecode_MAPGAMMATAB)!=1028) X2C_ASSERT(0);
   if (sizeof(CHSET)!=16) X2C_ASSERT(0);
   if (sizeof(TICKERCALL)!=31) X2C_ASSERT(0);
   aprstext_BEGIN();
   useri_BEGIN();
   aprspos_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   xosi_BEGIN();
}

