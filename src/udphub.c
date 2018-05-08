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
#define udphub_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif

/* axudp bidirectional digi - user hub by OE5DXL */
#define udphub_CALLLEN 7

#define udphub_MAXLEN 338

#define udphub_MINLEN 17

static uint32_t udphub_POLYNOM = 0x8408UL;

static uint32_t udphub_CRCINIT = 0xFFFFUL;

#define udphub_SOURCECALL 7

#define udphub_DESTCALL 0

#define udphub_MAXFD 31

#define udphub_cCOM "#"

#define udphub_cSPOOF "p"

#define udphub_cBCIN "b"

#define udphub_cBCOUT "B"

#define udphub_cSENDALL "A"

#define udphub_LF "\012"

#define udphub_TICKER 15

typedef char RAWCALL[7];


struct TIMEVAL {
   int32_t tvsec;
   int32_t tvusec;
};

struct USER;

typedef struct USER * pUSER;


struct USER {
   pUSER next;
   RAWCALL call;
   uint32_t uip;
   uint32_t dport;
   char datagot; /* not send same data twice to same ip/port */
   char bcin; /* broadcast in accept */
   char bcout; /* broadcast out allowed */
   char willall; /* like host send all to */
   char nopurge; /* entry from file no purge */
   char nospoof; /* not overwrite ip:port */
   char allssid; /* fits with any ssid */
   uint32_t htime;
   uint32_t framesin;
   uint32_t framesout;
};

static uint8_t CRCL[256];

static uint8_t CRCH[256];

static char noinf;

static char modified;

static char peertopeer;

static char defaultbcout;

/* no broadcast send to new users */
static char defaultbcin;

static char show;

/* no broadcast accept for new users */
static char checkdigiip;

static pUSER users;
/*  alllifetime,                                       (* time for all/unknown ssid *) */

static uint32_t systime;

static uint32_t lifetime;

static uint32_t uptime;

static uint32_t touserport;

static uint32_t todigiport;

static uint32_t digiip;

static int32_t digisock;

static int32_t usersock;

static uint32_t maxentries;

static char initfn[1025];

static char wrfn[1025];

static RAWCALL broadcastdest;

static uint32_t framecnt;

static uint32_t dupecnt;

static uint32_t dupewp;

static uint32_t maxdupetime;

struct _0;


struct _0 {
   uint16_t crc;
   uint32_t time0;
};

static struct _0 dupetab[64];


static void Err(const char text[], uint32_t text_len)
{
   osi_WrStr("udphub: ", 9ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */

#define udphub_POLINOM 0x8408 


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
   char tmp0;
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
   i = (uint32_t)(uint8_t)f[pos+6UL]>>1&15UL;
   if (i) {
      osi_WrStr("-", 2ul);
      if (i>=10UL) {
         osi_WrStr((char *)(tmp0 = (char)(i/10UL+48UL),&tmp0),
                1u/1u);
      }
      osi_WrStr((char *)(tmp0 = (char)(i%10UL+48UL),&tmp0), 1u/1u);
   }
} /* end ShowCall() */

static uint32_t udphub_UA = 0x63UL;

static uint32_t udphub_DM = 0xFUL;

static uint32_t udphub_SABM = 0x2FUL;

static uint32_t udphub_DISC = 0x43UL;

static uint32_t udphub_FRMR = 0x87UL;

static uint32_t udphub_UI = 0x3UL;

static uint32_t udphub_RR = 0x1UL;

static uint32_t udphub_REJ = 0x9UL;

static uint32_t udphub_RNR = 0x5UL;


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
                char noinfo)
{
   uint32_t i;
   char d;
   char v;
   i = 0UL;
   while (!((uint32_t)(uint8_t)f[i]&1)) {
      ++i;
      if (i>len) return;
   }
   /* no address end mark found */
   if (i%7UL!=6UL) return;
   /* address end not modulo 7 error */
   osi_WrStr(" fm ", 5ul);
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
   Showctl((uint32_t)((0x80U & (uint8_t)(uint8_t)f[6UL])!=0)
                +2UL*(uint32_t)((0x80U & (uint8_t)(uint8_t)f[13UL])!=0)
                , (uint32_t)(uint8_t)f[i]);
   ++i;
   if (i<len) {
      osi_WrStr(" pid ", 6ul);
      osi_WrHex((uint32_t)(uint8_t)f[i], 1UL);
   }
   ++i;
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
} /* end ShowFrame() */


static char testCRC(char frame[], uint32_t frame_len,
                int32_t size)
{
   uint8_t h;
   uint8_t l;
   uint8_t b;
   int32_t i;
   int32_t tmp;
   l = 0U;
   h = 0U;
   tmp = size-3L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (uint8_t)((uint8_t)(uint8_t)frame[i]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i==tmp) break;
   } /* end for */
   return frame[size-2L]==(char)l && frame[size-1L]==(char)h;
} /* end testCRC() */


static int32_t GetIp1(char h[], uint32_t h_len, uint32_t * ip,
                uint32_t * port)
{
   uint32_t p;
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t GetIp1_ret;
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
         GetIp1_ret = -1L;
         goto label;
      }
      if (i<3UL) {
         if (h[p]!='.' || n>255UL) {
            GetIp1_ret = -1L;
            goto label;
         }
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[p]!=':' || n>255UL) {
            GetIp1_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         GetIp1_ret = -1L;
         goto label;
      }
      *port = n;
      ++p;
   } /* end for */
   GetIp1_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp1_ret;
} /* end GetIp1() */


static int32_t getudp(int32_t fd, char buf[], uint32_t buf_len,
                uint32_t * fromip0, uint32_t * fromport,
                char checkip)
{
   uint32_t ip;
   int32_t len;
   len = udpreceive(fd, buf, (int32_t)(buf_len), fromport, &ip);
   if (len<2L || !testCRC(buf, buf_len, len)) {
      if (show) osi_WrStrLn(" axudp crc error ", 18ul);
      return -1L;
   }
   if (checkip && *fromip0!=ip) return -2L;
   *fromip0 = ip;
   return len;
} /* end getudp() */


static char Call2Str(const char r[], uint32_t r_len,
                char t[], uint32_t t_len, uint32_t pos,
                uint32_t * len, char zerossid)
{
   uint32_t ssid;
   uint32_t e;
   uint32_t i;
   char c;
   uint32_t tmp;
   e = pos;
   tmp = pos+5UL;
   i = pos;
   if (i<=tmp) for (;; i++) {
      if (r[i]!='@') e = i;
      if (i==tmp) break;
   } /* end for */
   tmp = e;
   i = pos;
   if (i<=tmp) for (;; i++) {
      c = (char)((uint32_t)(uint8_t)r[i]>>1);
      if ((uint8_t)c<=' ') {
         t[*len] = 0;
         *len = 0UL;
         return 0;
      }
      t[*len] = c;
      ++*len;
      if (i==tmp) break;
   } /* end for */
   ssid = (uint32_t)(uint8_t)r[pos+6UL]>>1&15UL;
   if (zerossid || ssid>0UL) {
      t[*len] = '-';
      ++*len;
      if (ssid>9UL) {
         t[*len] = '1';
         ++*len;
      }
      t[*len] = (char)(ssid%10UL+48UL);
      ++*len;
   }
   return 1;
} /* end Call2Str() */

#define udphub_SSID "-"


static char Str2Call(char s[], uint32_t s_len, uint32_t * i,
                uint32_t p, char cb[], uint32_t cb_len,
                char * hasssid)
{
   uint32_t j;
   char Str2Call_ret;
   X2C_PCOPY((void **)&s,s_len);
   *hasssid = 0;
   j = p;
   while ((*i<=s_len-1 && (uint8_t)s[*i]>' ') && s[*i]!='-') {
      if (j<p+6UL) {
         cb[j] = (char)((uint32_t)(uint8_t)s[*i]*2UL);
         ++j;
      }
      ++*i;
   }
   while (j<p+6UL) {
      cb[j] = '@';
      ++j;
   }
   j = 0UL;
   if (s[*i]=='-') {
      *hasssid = 1;
      ++*i;
      j = 16UL;
      if ((uint8_t)s[*i]>='0' && (uint8_t)s[*i]<='9') {
         j = (uint32_t)(uint8_t)s[*i]-48UL;
         ++*i;
      }
      if ((uint8_t)s[*i]>='0' && (uint8_t)s[*i]<='9') {
         j = (j*10UL+(uint32_t)(uint8_t)s[*i])-48UL;
         ++*i;
      }
   }
   cb[p+6UL] = (char)(j*2UL+1UL); /* ssid */
   Str2Call_ret = j<=15UL;
   X2C_PFREE(s);
   return Str2Call_ret;
} /* end Str2Call() */


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


static char GetNum(const char h[], uint32_t h_len,
                uint32_t * n)
{
   uint32_t i;
   *n = 0UL;
   i = 0UL;
   while ((uint8_t)h[i]>='0' && (uint8_t)h[i]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[i])-48UL;
      ++i;
   }
   return h[i]==0;
} /* end GetNum() */


static void parms(void)
{
   char ssid;
   char err0;
   char h[1024];
   uint32_t i;
   uint32_t fromdigiport;
   err0 = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &i)) Err("-l minutes", 11ul);
            lifetime = i*60UL;
         }
         else if (h[1U]=='a') {
            /*
                  ELSIF h[1]="L" THEN
                    NextArg(h);
                    IF NOT GetNum(h, i) THEN Err("-L minutes") END;
                    alllifetime:=i*60;
            */
            peertopeer = 1;
         }
         else if (h[1U]=='I') defaultbcin = 1;
         else if (h[1U]=='O') defaultbcout = 1;
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (h[0U]==0) Err("-b call", 8ul);
            i = 0UL;
            if (!Str2Call(h, 1024ul, &i, 0UL, broadcastdest, 7ul, &ssid)) {
               Err("-b wrong SSID", 14ul);
            }
         }
         else if (h[1U]=='i') {
            /* init filename */
            osi_NextArg(initfn, 1025ul);
         }
         else if (h[1U]=='w') {
            /* write table filename */
            osi_NextArg(wrfn, 1025ul);
         }
         else if (h[1U]=='m') {
            osi_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &maxentries)) Err("-m number", 10ul);
         }
         else if (h[1U]=='p') {
            osi_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &touserport)) Err("-p portnumber", 14ul);
         }
         else if (h[1U]=='u') {
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &digiip, &todigiport, &fromdigiport,
                &digisock, &checkdigiip)<0L) {
               Err("cannot open digi udp socket", 28ul);
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &maxdupetime)) {
               Err("-d maxdupetime", 15ul);
            }
         }
         else if (h[1U]=='v') show = 1;
         else if (h[1U]=='V') {
            show = 1;
            noinf = 0;
         }
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn(" -a                                route user-to-\
digi AND user-to-user", 71ul);
               osi_WrStrLn(" -b <call>                         broadcast dest\
ination call", 62ul);
               osi_WrStrLn(" -d <ms>                           dupefilter, in\
 milliseconds intervall discard frames with same CRC", 102ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I                                for new user: \
broadcast INPUT on", 68ul);
               osi_WrStrLn(" -i <file>                         init routes fr\
om file", 57ul);
               /*      WrStrLn(" -L <time>                         minutes route to all ssid'
                s (default 10 min)"); */
               osi_WrStrLn("                                   0 no all ssid \
routing", 57ul);
               osi_WrStrLn(" -l <time>                         minutes lifeti\
me (default 1 week)", 69ul);
               osi_WrStrLn(" -m <maxentries>                   else delete ol\
d entries (default 1000)", 74ul);
               osi_WrStrLn(" -O                                for new user: \
broadcast OUTPUT on", 69ul);
               osi_WrStrLn(" -p <userport>                     udp port for u\
sers", 54ul);
               osi_WrStrLn(" -u <x.x.x.x:destport:listenport>  axudp to digi \
/listenport check ip", 70ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osi_WrStrLn(" -V                                verbous + fram\
es with Text", 62ul);
               osi_WrStrLn(" -w <file>                         write user tab\
le to file (only if new entries and max. every 15s)", 101ul);
               osic_WrLn();
               osi_WrStrLn("Initfile:", 10ul);
               osi_WrStrLn("NOCALL-15 192.168.0.1:4711 #comment", 36ul);
               osi_WrStrLn("NOCALL-15 p 192.168.0.1:4711 #protected entry",
                46ul);
               osi_WrStrLn("#comment", 9ul);
               osi_WrStrLn("NOCALL-15 p 0.0.0.0:0 no data to this call except\
 to digi port", 63ul);
               osi_WrStrLn("b enable broadcast input", 25ul);
               osi_WrStrLn("B enable broadcast output", 26ul);
               osi_WrStrLn("A send all frames", 18ul);
               osic_WrLn();
               osi_WrStrLn("Routing Table:", 15ul);
               osi_WrStrLn("(-l) time old table entries will be purged except\
 those from init file", 71ul);
               osic_WrLn();
               osi_WrStrLn("Source: AX.25 Source Call or last Digi with H-bit\
 makes table entry with call/ip/sourceport/date", 97ul);
               osi_WrStrLn("        Exception: Protected entry updates Date o\
nly", 53ul);
               osic_WrLn();
               osi_WrStrLn("Destinationcall: (call used for routing)", 41ul);
               osi_WrStrLn("  First Digi with no H-bit, if not present, ax25 \
destination call is used", 74ul);
               osic_WrLn();
               osi_WrStrLn("Broadcast: if user enabled to input broadcast and\
 ax25-destination equals broadcast call", 89ul);
               osic_WrLn();
               osi_WrStrLn("Destination: axudp ip/port", 27ul);
               osic_WrLn();
               osi_WrStrLn("Routing: frame will be sent to if", 34ul);
               osi_WrStrLn("  user enabled to get all data", 31ul);
               osi_WrStrLn("  OR broadcast and user enabled to get broadcast",
                 49ul);
               osi_WrStrLn("  OR destinationcall with ssid equals user",
                43ul);
               osi_WrStrLn("  OR destinationcall but not ssid fits to user in\
 table", 56ul);
               osi_WrStrLn("     if user is added from heard frames or vom in\
it file with no ssid (-0 is with ssid)", 88ul);
               osi_WrStrLn("  exception 1: data never sent (back) to ip/port \
where came from", 65ul);
               osi_WrStrLn("  exception 2: data sent only one time to ip/port\
 even if more destinationcalls share same ip/port", 99ul);
               osi_WrStrLn("  exception 3: with not \'-a\' only data from dig\
i port routes to destinationcall", 80ul);
               osic_WrLn();
               X2C_ABORT();
            }
            err0 = 1;
         }
      }
      else {
         /*
               h[0]:=0C;
         */
         err0 = 1;
      }
      if (err0) break;
   }
   if (err0) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end parms() */


static char isdupe(const char ubuf0[], uint32_t ubuf_len,
                int32_t blen0, uint32_t tms0)
{
   uint16_t c;
   uint32_t i;
   if (maxdupetime==0UL) return 0;
   c = (uint16_t)((uint32_t)(uint8_t)ubuf0[blen0]+(uint32_t)
                (uint8_t)ubuf0[blen0+1L]*256UL);
   i = dupewp;
   for (;;) {
      if (tms0-dupetab[i].time0>=maxdupetime) break;
      if (dupetab[i].crc==c) return 1;
      if (i>0UL) --i;
      else i = 63UL;
      if (i==dupewp) break;
   }
   ++dupewp;
   if (dupewp>63UL) dupewp = 0UL;
   dupetab[dupewp].crc = c;
   dupetab[dupewp].time0 = tms0;
   return 0;
} /* end isdupe() */


static void ip2str(uint32_t ip, uint32_t port, char s[],
                uint32_t s_len)
{
   char h[21];
   s[0UL] = 0;
   aprsstr_IntToStr((int32_t)(ip/16777216UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip/65536UL&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip/256UL&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((int32_t)port, 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
} /* end ip2str() */


static void showpip(uint32_t ip, uint32_t port)
{
   char h[51];
   ip2str(ip, port, h, 51ul);
   osi_WrStr(h, 51ul);
} /* end showpip() */


static void showcall(const char b[], uint32_t b_len, uint32_t start)
{
   char h[16];
   uint32_t l;
   l = 0UL;
   if (Call2Str(b, b_len, h, 16ul, start, &l, 1)) {
      h[l] = 0;
      osi_WrStr(h, 16ul);
   }
} /* end showcall() */


static void listtab(char fn[], uint32_t fn_len)
{
   int32_t fd;
   pUSER u;
   char s[201];
   char h[201];
   uint32_t j;
   uint32_t i;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenWrite(fn, fn_len);
   if (osic_FdValid(fd)) {
      u = users;
      while (u) {
         i = 0UL;
         if (Call2Str(u->call, 7ul, h, 201ul, 0UL, &i, !u->allssid)) {
            while (i<10UL) {
               h[i] = ' ';
               ++i;
            }
            h[i] = 0;
         }
         else h[0U] = 0;
         j = 0UL;
         if (u->nospoof) {
            aprsstr_Append(h, 201ul, "p", 2ul);
            ++j;
         }
         else if (u->nopurge) {
            aprsstr_Append(h, 201ul, "f", 2ul);
            ++j;
         }
         if (u->bcout) {
            aprsstr_Append(h, 201ul, "B", 2ul);
            ++j;
         }
         if (u->bcin) {
            aprsstr_Append(h, 201ul, "b", 2ul);
            ++j;
         }
         if (u->willall) {
            aprsstr_Append(h, 201ul, "A", 2ul);
            ++j;
         }
         while (j<4UL) {
            aprsstr_Append(h, 201ul, " ", 2ul);
            ++j;
         }
         ip2str(u->uip, u->dport, s, 201ul);
         aprsstr_Append(h, 201ul, s, 201ul);
         if ((u->htime>0UL || u->framesin>0UL) || u->framesout>0UL) {
            i = aprsstr_Length(h, 201ul);
            while (i<36UL) {
               h[i] = ' ';
               ++i;
            }
            h[i] = 0;
            if (u->htime>0UL) {
               aprsstr_DateToStr(u->htime, s, 201ul);
               aprsstr_Append(h, 201ul, s, 201ul);
               aprsstr_Append(h, 201ul, " ", 2ul);
            }
            aprsstr_Append(h, 201ul, "i=", 3ul);
            aprsstr_CardToStr(u->framesin, 1UL, s, 201ul);
            aprsstr_Append(h, 201ul, s, 201ul);
            aprsstr_Append(h, 201ul, " o=", 4ul);
            aprsstr_CardToStr(u->framesout, 1UL, s, 201ul);
            aprsstr_Append(h, 201ul, s, 201ul);
         }
         aprsstr_Append(h, 201ul, "\012", 2ul);
         osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
         u = u->next;
      }
      strncpy(h,"dupetime:",201u);
      aprsstr_CardToStr(maxdupetime, 0UL, s, 201ul);
      aprsstr_Append(h, 201ul, s, 201ul);
      aprsstr_Append(h, 201ul, "ms  frames:", 12ul);
      aprsstr_CardToStr(framecnt, 0UL, s, 201ul);
      aprsstr_Append(h, 201ul, s, 201ul);
      aprsstr_Append(h, 201ul, "  dupes:", 9ul);
      aprsstr_CardToStr(dupecnt, 0UL, s, 201ul);
      aprsstr_Append(h, 201ul, s, 201ul);
      aprsstr_Append(h, 201ul, "\012", 2ul);
      osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      strncpy(h,"f from init file\012p ip/port protected\012B BC out\012b BC \
in\012A gets all\012",201u);
      osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      if (peertopeer) {
         strncpy(h,"peer-to-peer routing on\012",201u);
         osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      }
      i = 0UL;
      if (Call2Str(broadcastdest, 7ul, h, 201ul, 0UL, &i, 0)) {
         h[i] = 0;
         aprsstr_Append(h, 201ul, " broadcast destination\012", 24ul);
         osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      }
      aprsstr_IntToStr((int32_t)touserport, 0UL, h, 201ul);
      aprsstr_Append(h, 201ul, " user UDP port\012", 16ul);
      osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      strncpy(h,"Uptime: ",201u);
      aprsstr_TimeToStr(systime-uptime, s, 201ul);
      aprsstr_Append(h, 201ul, s, 201ul);
      aprsstr_Append(h, 201ul, "\012", 2ul);
      osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      osic_Close(fd);
   }
   else Err("-w File Create", 15ul);
   X2C_PFREE(fn);
} /* end listtab() */


static char cmpcall(const RAWCALL c, const char b[],
                uint32_t b_len, uint32_t start, char withssid)
{
   uint32_t i;
   for (i = 0UL; i<=5UL; i++) {
      if (c[i]!=b[i+start]) return 0;
   } /* end for */
   return !withssid || ((uint32_t)(uint8_t)c[6U]/2UL&15UL)==((uint32_t)
                (uint8_t)b[start+6UL]/2UL&15UL);
} /* end cmpcall() */


static pUSER Realloc(char alloc)
{
   uint32_t cnt;
   pUSER new0;
   pUSER last;
   pUSER u;
   cnt = 0UL;
   last = 0;
   new0 = 0;
   u = users;
   while (u) {
      if (!u->nopurge && (u->htime+lifetime<systime || cnt>=maxentries)) {
         /* old or too much entries */
         if (last==0) users = u->next;
         else last->next = u->next;
         if (show) {
            osi_WrStr("Purge User ", 12ul);
            showcall(u->call, 7ul, 0UL);
            osic_WrLn();
         }
         if (alloc && new0==0) new0 = u;
         else osic_free((char * *) &u, sizeof(struct USER));
         if (last==0) u = users;
         else u = last->next;
         modified = 1;
      }
      else {
         last = u;
         u = u->next;
         ++cnt;
      }
   }
   if (alloc && new0==0) {
      osic_alloc((char * *) &new0, sizeof(struct USER));
   }
   /*  IF show THEN WrStr(" Table entries="); WrInt(cnt, 1); WrLn; END; */
   return new0;
} /* end Realloc() */


static void showu(uint32_t dp, pUSER u)
{
   if (u->nospoof) osi_WrStr(" writeprotected", 16ul);
   if (u->bcout) osi_WrStr(" bc-out", 8ul);
   if (u->bcin) osi_WrStr(" bc-in", 7ul);
   if (u->willall) osi_WrStr(" gets-all", 10ul);
   osi_WrStr(" IP:", 5ul);
   showpip(u->uip, dp);
   osic_WrLn();
} /* end showu() */


static void AddIp(uint32_t ip, uint32_t dp, char fix,
                char nspoof, char * hasbcin,
                char defbcin, char defbcout,
                char getsall, char defssid,
                const char buf[], uint32_t buf_len)
{
   pUSER last;
   uint32_t cp;
   uint32_t i;
   pUSER u;
   /* for fast find, rechain to first position */
   struct USER * anonym;
   struct USER * anonym0;
   struct USER * anonym1;
   *hasbcin = 0;
   cp = 7UL;
   while ((cp<=56UL && !((uint32_t)(uint8_t)buf[cp+6UL]&1))
                && (uint8_t)buf[cp+13UL]>=(uint8_t)'\200') cp += 7UL;
   u = users;
   last = 0;
   for (;;) {
      if (u==0) break;
      if (cmpcall(u->call, buf, buf_len, cp, 1)) {
         if (!fix && last) {
            last->next = u->next;
            u->next = users;
            users = u;
         }
         { /* with */
            struct USER * anonym = u;
            if (!anonym->nospoof) {
               anonym->uip = ip; /* store if ip changed */
               anonym->dport = dp;
            }
         }
         break;
      }
      last = u;
      u = u->next;
   }
   if (u==0) {
      u = Realloc(1);
      if (u==0) {
         if (show) osi_WrStrLn(" user add out of memory", 24ul);
      }
      else {
         { /* with */
            struct USER * anonym0 = u;
            for (i = 0UL; i<=6UL; i++) {
               anonym0->call[i] = buf[cp+i];
            } /* end for */
            anonym0->call[6U] = (char)(((uint32_t)(uint8_t)
                anonym0->call[6U]/2UL&15UL)*2UL); /* extract pure ssid */
            anonym0->uip = ip;
            anonym0->dport = dp;
            anonym0->htime = 0UL;
            anonym0->framesout = 0UL;
            anonym0->framesin = 0UL;
         }
         if (show) {
            osi_WrStr("Add User ", 10ul);
            showcall(buf, buf_len, cp);
            showu(dp, u);
         }
         u->next = users;
         users = u;
      }
   }
   if (u) {
      { /* with */
         struct USER * anonym1 = u;
         if (!fix) {
            anonym1->htime = systime;
            ++anonym1->framesin;
         }
         anonym1->nospoof = nspoof;
         anonym1->nopurge = fix;
         anonym1->bcin = defbcin;
         anonym1->bcout = defbcout;
         anonym1->willall = getsall;
         *hasbcin = anonym1->bcin;
         anonym1->allssid = !defssid;
      }
   }
} /* end AddIp() */


static char sendtouser(char ubuf0[], uint32_t ubuf_len,
                int32_t blen0, char fromdigi, char bcin,
                uint32_t fromip0, uint32_t fromport)
{
   pUSER uu;
   pUSER u;
   pUSER exactu;
   int32_t ci;
   int32_t res0;
   char ok0;
   char broadcast;
   char topeer;
   ok0 = 0;
   ci = 7L;
   if (((uint32_t)(uint8_t)ubuf0[13UL]&1)) ci = 0L;
   else {
      /* find first not done via */
      for (;;) {
         ci += 7L;
         if (ci>63L || ci+7L>=blen0) return 0;
         /* no valid frame */
         if ((uint8_t)ubuf0[ci+6L]<(uint8_t)'\200') break;
         if (((uint32_t)(uint8_t)ubuf0[ci+6L]&1)) {
            ci = 0L; /* all h-bits set */
            break;
         }
      }
   }
   /*WrInt(ci, 1); WrStrLn(" =ci"); */
   broadcast = (((fromdigi || bcin) && ci==0L) && broadcastdest[0U])
                && cmpcall(broadcastdest, ubuf0, ubuf_len, 0UL, 1);
   topeer = !broadcast && (peertopeer || fromdigi); /* data for an user */
   if (topeer) {
      exactu = users;
      while (exactu && !cmpcall(exactu->call, ubuf0, ubuf_len,
                (uint32_t)ci, 1)) {
         /* compare with ssid */
         exactu = exactu->next; /* find call + ssid match */
      }
   }
   else exactu = 0;
   /*WrInt(ORD(exactu<>NIL), 1); WrStrLn(" =exactu"); */
   u = users;
   while (u) {
      u->datagot = 0;
      if ((u->dport>0UL && ((fromdigi || fromip0!=u->uip)
                || fromport!=u->dport)) && (((u->willall || u==exactu)
                || broadcast && u->bcout) || ((u->allssid && topeer)
                && exactu==0) && cmpcall(u->call, ubuf0, ubuf_len,
                (uint32_t)ci, 0))) {
         /* user enabled */
         /* send never same way back */
         /* try all same call without ssid match */
         uu = users;
         while (uu!=u && ((!uu->datagot || uu->uip!=u->uip)
                || uu->dport!=u->dport)) uu = uu->next;
         if (uu==u) {
            /* have not sent jet to ip/port */
            u->datagot = 1;
            res0 = udpsend(usersock, ubuf0, blen0, u->dport, u->uip);
            ++u->framesout;
            if (show) {
               osi_WrStr("< send to user ", 16ul);
               showcall(u->call, 7ul, 0UL);
               osi_WrStr(" ", 2ul);
               showpip(u->uip, u->dport);
               osi_WrStrLn("", 1ul);
            }
            if (u==exactu || broadcast) ok0 = 1;
         }
      }
      /*
      ELSE
      showcall(u^.call, 0); WrStrLn(" sendno");
      WrInt(ORD(u=exactu), 1); WrStr(" =u=exactu ");
      showpip(u^.uip, u^.dport); WrStr(" =userip ");
      showpip(fromip, fromport); WrStr(" =fromip ");
      */
      u = u->next;
   }
   /*  IF show & ok THEN WrStrLn("") END; */
   return ok0;
} /* end sendtouser() */


static void err(char h[], uint32_t h_len, char fn[],
                uint32_t fn_len, uint32_t lc)
{
   char s[4001];
   X2C_PCOPY((void **)&h,h_len);
   X2C_PCOPY((void **)&fn,fn_len);
   osi_WrStr("Error in line ", 15ul);
   aprsstr_IntToStr((int32_t)lc, 1UL, s, 4001ul);
   aprsstr_Append(s, 4001ul, ":[", 3ul);
   aprsstr_Append(s, 4001ul, fn, fn_len);
   aprsstr_Append(s, 4001ul, "] ", 3ul);
   aprsstr_Append(s, 4001ul, h, h_len);
   osi_WrStrLn(s, 4001ul);
   X2C_PFREE(h);
   X2C_PFREE(fn);
} /* end err() */


static void initroutes(char fn[], uint32_t fn_len)
{
   int32_t fd;
   char call[201];
   char b[201];
   uint32_t lc;
   uint32_t j;
   uint32_t i;
   char dbcin0;
   char all;
   char bci;
   char bco;
   char spoof;
   char ssid;
   uint32_t ip;
   uint32_t dp;
   pUSER pu;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenRead(fn, fn_len);
   if (osic_FdValid(fd)) {
      pu = users;
      while (pu) {
         pu->nopurge = 0;
         pu = pu->next;
      }
      lc = 1UL;
      for (;;) {
         i = 0UL;
         do {
            if (osi_RdBin(fd, (char *) &b[i], 1u/1u, 1UL)<1L) {
               goto loop_exit;
            }
            ++i;
         } while (!(i>=200UL || b[i-1UL]=='\012'));
         b[i] = 0;
         if (b[0U]!='#') {
            i = 0UL;
            if (!Str2Call(b, 201ul, &i, 7UL, call, 201ul, &ssid)) {
               err("wrong SSID in Init File", 24ul, fn, fn_len, lc);
            }
            while (b[i]==' ') ++i;
            spoof = b[i]=='p';
            if (spoof) ++i;
            all = b[i]=='A';
            if (all) ++i;
            bci = b[i]=='b';
            if (bci) ++i;
            bco = b[i]=='B';
            if (bco) ++i;
            while (b[i]==' ') ++i;
            if (all && bco) {
               err("Broadcast out AND get-all? in Init File", 40ul, fn,
                fn_len, lc);
            }
            j = 0UL;
            while ((uint8_t)b[i]>' ' && b[i]!='#') {
               b[j] = b[i];
               ++j;
               ++i;
            }
            b[j] = 0;
            if (GetIp1(b, 201ul, &ip, &dp)<0L) {
               err("wrong IP:PORT in Init File", 27ul, fn, fn_len, lc);
            }
            else {
               AddIp(ip, dp, 1, spoof, &dbcin0, bci, bco, all, ssid, call,
                201ul);
            }
         }
         ++lc;
      }
      loop_exit:;
      osic_Close(fd);
   }
   else Err("-i File not found", 18ul);
   X2C_PFREE(fn);
} /* end initroutes() */

static char ubuf[338];

static int32_t blen;

static int32_t res;

static uint32_t fromip;

static uint32_t userdport;

static uint32_t lastlist;

static char dbcin;

static char dupe;

static pUSER voidu;

static uint32_t ts;

static uint32_t tus;

static uint32_t tms;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(RAWCALL)!=7) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   Gencrctab();
   noinf = 1;
   show = 0;
   peertopeer = 0;
   defaultbcin = 0;
   defaultbcout = 0;
   touserport = 0UL;
   digisock = -1L;
   maxentries = 1000UL;
   lifetime = 604800UL;
   /*  alllifetime:=60*10; */
   users = 0;
   initfn[0U] = 0;
   wrfn[0U] = 0;
   broadcastdest[0U] = 0;
   framecnt = 0UL;
   dupecnt = 0UL;
   dupewp = 0UL;
   maxdupetime = 0UL;
   memset((char *)dupetab,(char)0,sizeof(struct _0 [64]));
   parms();
   if (initfn[0U]) initroutes(initfn, 1025ul);
   modified = 1;
   if (!peertopeer && digisock<0L) Err("need -u parameter", 18ul);
   usersock = openudp();
   if ((touserport==0UL || usersock<0L) || bindudp(usersock, touserport)<0L) {
      Err("cannot bind userport (-p userport)", 35ul);
   }
   systime = osic_time();
   uptime = systime;
   for (;;) {
      fdclr();
      if (digisock>=0L) fdsetr((uint32_t)digisock);
      fdsetr((uint32_t)usersock);
      tus = 0UL;
      ts = 15UL;
      res = selectrwt(&ts, &tus);
      tms += ((15UL-ts)*1000UL-tus/1000UL)+1UL;
      if (digisock>=0L && issetr((uint32_t)digisock)) {
         /* data from digi */
         fromip = digiip;
         blen = getudp(digisock, ubuf, 338ul, &fromip, &userdport,
                checkdigiip);
         if (blen>=17L) {
            if (show) {
               osi_WrStr("> from digi", 12ul);
               ShowFrame(ubuf, 338ul, (uint32_t)(blen-2L), noinf);
            }
            if (!sendtouser(ubuf, 338ul, blen, 1, 1, digiip,
                userdport) && show) {
               osi_WrStrLn(" digi out user not found", 25ul);
            }
            if (show) osi_WrStrLn("-----", 6ul);
         }
         ++framecnt;
      }
      if (issetr((uint32_t)usersock)) {
         /* data from user */
         fromip = 0UL;
         blen = getudp(usersock, ubuf, 338ul, &fromip, &userdport, 0);
         if (blen>=17L) {
            dupe = isdupe(ubuf, 338ul, blen-2L, tms);
            if (show) {
               if (dupe) osi_WrStr("[dupe]", 7ul);
               osi_WrStr("> from user ", 13ul);
               showpip(fromip, userdport);
               ShowFrame(ubuf, 338ul, (uint32_t)(blen-2L), noinf);
            }
            if (!dupe) {
               AddIp(fromip, userdport, 0, 0, &dbcin, defaultbcin,
                defaultbcout, 0, 0, ubuf, 338ul);
               modified = 1;
               if (digisock>=0L) {
                  res = udpsend(digisock, ubuf, blen, todigiport, digiip);
               }
               if (!sendtouser(ubuf, 338ul, blen, 0, dbcin, fromip,
                userdport) && show) {
                  osi_WrStrLn(" peer-to-peer no user found", 28ul);
               }
            }
            else ++dupecnt;
            if (show) osi_WrStrLn("-----", 6ul);
         }
         ++framecnt;
      }
      systime = osic_time();
      if (lastlist+15UL<systime || lastlist>systime) {
         if (initfn[0U]) {
            initroutes(initfn, 1025ul);
         }
         voidu = Realloc(0);
         if (modified && wrfn[0U]) {
            listtab(wrfn, 1025ul);
            modified = 0;
         }
         lastlist = systime;
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
