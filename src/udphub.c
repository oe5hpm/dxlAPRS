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

static unsigned long udphub_POLYNOM = 0x8408UL;

static unsigned long udphub_CRCINIT = 0xFFFFUL;

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
   long tvsec;
   long tvusec;
};

struct USER;

typedef struct USER * pUSER;


struct USER {
   pUSER next;
   RAWCALL call;
   unsigned long uip;
   unsigned long dport;
   char datagot; /* not send same data twice to same ip/port */
   char bcin; /* broadcast in accept */
   char bcout; /* broadcast out allowed */
   char willall; /* like host send all to */
   char nopurge; /* entry from file no purge */
   char nospoof; /* not overwrite ip:port */
   unsigned long htime;
};

static unsigned char CRCL[256];

static unsigned char CRCH[256];

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

static unsigned long alllifetime;

static unsigned long systime;

/* time for all/unknown ssid */
static unsigned long lifetime;

static unsigned long touserport;

static unsigned long todigiport;

static unsigned long digiip;

static long digisock;

static long usersock;

static unsigned long maxentries;

static char initfn[1025];

static char wrfn[1025];

static RAWCALL broadcastdest;
/*
PROCEDURE ["C"] / select(n: INTEGER; readfds: ADDRESS; writefds: ADDRESS;
                         exceptfds: ADDRESS; timeout: ADDRESS) : INTEGER;
*/


static void Err(const char text[], unsigned long text_len)
{
   osi_WrStr("udphub: ", 9ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */

#define udphub_POLINOM 0x8408 


static void Gencrctab(void)
{
   unsigned long c;
   unsigned long crc;
   unsigned long i;
   for (c = 0UL; c<=255UL; c++) {
      crc = 255UL-c;
      for (i = 0UL; i<=7UL; i++) {
         if ((crc&1)) {
            crc = (unsigned long)((unsigned long)(crc>>1)^0x8408UL);
         }
         else crc = crc>>1;
      } /* end for */
      CRCL[c] = (unsigned char)crc;
      CRCH[c] = (unsigned char)(255UL-(crc>>8));
   } /* end for */
} /* end Gencrctab() */


static void WCh(char c)
{
   if (c!='\015') {
      if ((unsigned char)c<' ' || (unsigned char)c>='\177') {
         osi_WrStr(".", 2ul);
      }
      else osi_WrStr((char *) &c, 1u/1u);
   }
} /* end WCh() */


static void ShowCall(char f[], unsigned long f_len, unsigned long pos)
{
   unsigned long e;
   unsigned long i;
   unsigned long tmp;
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
      WCh((char)((unsigned long)(unsigned char)f[i]>>1));
      if (i==tmp) break;
   } /* end for */
   i = (unsigned long)(unsigned char)f[pos+6UL]>>1&15UL;
   if (i) {
      osi_WrStr("-", 2ul);
      if (i>=10UL) {
         osi_WrStr((char *)(tmp0 = (char)(i/10UL+48UL),&tmp0), 1u/1u);
      }
      osi_WrStr((char *)(tmp0 = (char)(i%10UL+48UL),&tmp0), 1u/1u);
   }
} /* end ShowCall() */

static unsigned long udphub_UA = 0x63UL;

static unsigned long udphub_DM = 0xFUL;

static unsigned long udphub_SABM = 0x2FUL;

static unsigned long udphub_DISC = 0x43UL;

static unsigned long udphub_FRMR = 0x87UL;

static unsigned long udphub_UI = 0x3UL;

static unsigned long udphub_RR = 0x1UL;

static unsigned long udphub_REJ = 0x9UL;

static unsigned long udphub_RNR = 0x5UL;


static void Showctl(unsigned long com, unsigned long cmd)
{
   unsigned long cm;
   char PF[4];
   char tmp;
   osi_WrStr(" ctl ", 6ul);
   cm = (unsigned long)cmd&~0x10UL;
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
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>1&7UL)),&tmp), 1u/1u);
   }
   else if (cm==0x3UL) osi_WrStr("UI", 3ul);
   else if (cm==0xFUL) osi_WrStr("DM", 3ul);
   else if (cm==0x2FUL) osi_WrStr("SABM", 5ul);
   else if (cm==0x43UL) osi_WrStr("DISC", 5ul);
   else if (cm==0x63UL) osi_WrStr("UA", 3ul);
   else if (cm==0x87UL) osi_WrStr("FRMR", 5ul);
   else osic_WrHex(cmd, 1UL);
   strncpy(PF,"v^-+",4u);
   if (com==0UL || com==3UL) osi_WrStr("v1", 3ul);
   else {
      osi_WrStr((char *) &PF[(com&1UL)+2UL*(unsigned long)
                ((0x10UL & (unsigned long)cmd)!=0)], 1u/1u);
   }
} /* end Showctl() */


static void ShowFrame(char f[], unsigned long f_len, unsigned long len,
                char noinfo)
{
   unsigned long i;
   char d;
   char v;
   i = 0UL;
   while (!((unsigned long)(unsigned char)f[i]&1)) {
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
   while (i+6UL<len && !((unsigned long)(unsigned char)f[i-1UL]&1)) {
      if (v) {
         osi_WrStr(" via", 5ul);
         v = 0;
      }
      osi_WrStr(" ", 2ul);
      ShowCall(f, f_len, i);
      if ((unsigned long)(unsigned char)f[i+6UL]>=128UL && (((unsigned long)
                (unsigned char)f[i+6UL]&1) || (unsigned long)(unsigned char)
                f[i+13UL]<128UL)) osi_WrStr("*", 2ul);
      i += 7UL;
   }
   Showctl((unsigned long)((0x80U & (unsigned char)(unsigned char)f[6UL])!=0)
                +2UL*(unsigned long)((0x80U & (unsigned char)(unsigned char)
                f[13UL])!=0), (unsigned long)(unsigned char)f[i]);
   ++i;
   if (i<len) {
      osi_WrStr(" pid ", 6ul);
      osic_WrHex((unsigned long)(unsigned char)f[i], 1UL);
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


static char testCRC(char frame[], unsigned long frame_len, long size)
{
   unsigned char h;
   unsigned char l;
   unsigned char b;
   long i;
   long tmp;
   l = 0U;
   h = 0U;
   tmp = size-3L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (unsigned char)((unsigned char)(unsigned char)frame[i]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i==tmp) break;
   } /* end for */
   return frame[size-2L]==(char)l && frame[size-1L]==(char)h;
} /* end testCRC() */

/*
PROCEDURE GetIp1(h:ARRAY OF CHAR; VAR ip:IPNUM; VAR dp:UDPPORT):INTEGER;
CONST PORTSEP=":";
      DEFAULTIP=7F000001H;

VAR i, p, n:CARDINAL;
    ok:BOOLEAN;
BEGIN
  p:=0;
  h[HIGH(h)]:=0C;
  ip:=0;
  FOR i:=0 TO 4 DO
    IF (i>=3) OR (h[0]<>PORTSEP) THEN
      n:=0;
      ok:=FALSE;
      WHILE (h[p]>="0") & (h[p]<="9") DO
        ok:=TRUE;
        n:=n*10+ORD(h[p])-ORD("0");
        INC(p);
      END;
      IF NOT ok THEN RETURN -1 END;

    END;
    IF i<3 THEN
      IF h[0]<>PORTSEP THEN
        IF (h[p]<>".") OR (n>255) THEN RETURN -1 END;

        ip:=ip*256+n;
      END;

    ELSIF i=3 THEN
      IF h[0]<>PORTSEP THEN
        ip:=ip*256+n;
        IF (h[p]<>PORTSEP) OR (n>255) THEN RETURN -1 END;

      ELSE p:=0; ip:=DEFAULTIP END;

    ELSIF n>65535 THEN RETURN -1 END;

    dp:=n;
    INC(p);
  END;
  RETURN 0
END GetIp1;
*/

static long GetIp1(char h[], unsigned long h_len, unsigned long * ip,
                unsigned long * port)
{
   unsigned long p;
   unsigned long n;
   unsigned long i;
   char ok0;
   long GetIp1_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((unsigned char)h[p]>='0' && (unsigned char)h[p]<='9') {
         ok0 = 1;
         n = (n*10UL+(unsigned long)(unsigned char)h[p])-48UL;
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


static long getudp(long fd, char buf[], unsigned long buf_len,
                unsigned long * fromip0, unsigned long * fromport,
                char checkip)
{
   unsigned long ip;
   long len;
   len = udpreceive(fd, buf, (long)(buf_len), fromport, &ip);
   if (len<2L || !testCRC(buf, buf_len, len)) {
      if (show) osi_WrStrLn(" axudp crc error ", 18ul);
      return -1L;
   }
   if (checkip && *fromip0!=ip) return -2L;
   *fromip0 = ip;
   return len;
} /* end getudp() */


static char Call2Str(const char r[], unsigned long r_len, char t[],
                unsigned long t_len, unsigned long pos, unsigned long * len)
{
   unsigned long ssid;
   unsigned long e;
   unsigned long i;
   char c;
   unsigned long tmp;
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
      c = (char)((unsigned long)(unsigned char)r[i]>>1);
      if ((unsigned char)c<=' ') {
         t[*len] = 0;
         *len = 0UL;
         return 0;
      }
      t[*len] = c;
      ++*len;
      if (i==tmp) break;
   } /* end for */
   ssid = (unsigned long)(unsigned char)r[pos+6UL]>>1&15UL;
   if (ssid>0UL) {
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


static char Str2Call(char s[], unsigned long s_len, unsigned long * i,
                unsigned long p, char cb[], unsigned long cb_len)
{
   unsigned long j;
   char Str2Call_ret;
   X2C_PCOPY((void **)&s,s_len);
   j = p;
   while ((*i<=s_len-1 && (unsigned char)s[*i]>' ') && s[*i]!='-') {
      if (j<p+6UL) {
         cb[j] = (char)((unsigned long)(unsigned char)s[*i]*2UL);
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
      ++*i;
      j = 16UL;
      if ((unsigned char)s[*i]>='0' && (unsigned char)s[*i]<='9') {
         j = (unsigned long)(unsigned char)s[*i]-48UL;
         ++*i;
      }
      if ((unsigned char)s[*i]>='0' && (unsigned char)s[*i]<='9') {
         j = (j*10UL+(unsigned long)(unsigned char)s[*i])-48UL;
         ++*i;
      }
   }
   cb[p+6UL] = (char)(j*2UL+1UL); /* ssid */
   Str2Call_ret = j<=15UL;
   X2C_PFREE(s);
   return Str2Call_ret;
} /* end Str2Call() */


static long GetIp(char h[], unsigned long h_len, unsigned long * ip,
                unsigned long * dp, unsigned long * lp, long * fd,
                char * check)
{
   long GetIp_ret;
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


static char GetNum(const char h[], unsigned long h_len, unsigned long * n)
{
   unsigned long i;
   *n = 0UL;
   i = 0UL;
   while ((unsigned char)h[i]>='0' && (unsigned char)h[i]<='9') {
      *n = ( *n*10UL+(unsigned long)(unsigned char)h[i])-48UL;
      ++i;
   }
   return h[i]==0;
} /* end GetNum() */


static void parms(void)
{
   char err0;
   char h[1024];
   unsigned long i;
   unsigned long fromdigiport;
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
         else if (h[1U]=='L') {
            osi_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &i)) Err("-L minutes", 11ul);
            alllifetime = i*60UL;
         }
         else if (h[1U]=='a') peertopeer = 1;
         else if (h[1U]=='I') defaultbcin = 1;
         else if (h[1U]=='O') defaultbcout = 1;
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (h[0U]==0) Err("-b call", 8ul);
            i = 0UL;
            if (!Str2Call(h, 1024ul, &i, 0UL, broadcastdest, 7ul)) {
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
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I                                for new user: \
broadcast INPUT on", 68ul);
               osi_WrStrLn(" -i <file>                         init routes fr\
om file", 57ul);
               osi_WrStrLn(" -L <time>                         minutes route \
to all ssid\'s (default 10 min)", 80ul);
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
               osi_WrStrLn("Source: AX.25 Source Call makes table entry with \
call/ip/sourceport/date", 73ul);
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
               osi_WrStrLn("  OR destinationcall with ssid fits to no user in\
 table", 56ul);
               osi_WrStrLn("     but destinationcall without ssid equals user\
 seen since -L time", 69ul);
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


static void ip2str(unsigned long ip, unsigned long port, char s[],
                unsigned long s_len)
{
   char h[21];
   s[0UL] = 0;
   aprsstr_IntToStr((long)(ip/16777216UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip/65536UL&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip/256UL&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip&255UL), 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((long)port, 1UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
} /* end ip2str() */


static void showpip(unsigned long ip, unsigned long port)
{
   char h[51];
   ip2str(ip, port, h, 51ul);
   osi_WrStr(h, 51ul);
} /* end showpip() */


static void showcall(const char b[], unsigned long b_len,
                unsigned long start)
{
   char h[16];
   unsigned long l;
   l = 0UL;
   if (Call2Str(b, b_len, h, 16ul, start, &l)) {
      h[l] = 0;
      osi_WrStr(h, 16ul);
   }
} /* end showcall() */


static void listtab(char fn[], unsigned long fn_len)
{
   long fd;
   pUSER u;
   char s[201];
   char h[201];
   unsigned long j;
   unsigned long i;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenWrite(fn, fn_len);
   if (osic_FdValid(fd)) {
      u = users;
      while (u) {
         i = 0UL;
         if (Call2Str(u->call, 7ul, h, 201ul, 0UL, &i)) {
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
         if (u->htime>0UL) {
            i = aprsstr_Length(h, 201ul);
            while (i<36UL) {
               h[i] = ' ';
               ++i;
            }
            h[i] = 0;
            aprsstr_DateToStr(u->htime, s, 201ul);
            aprsstr_Append(h, 201ul, s, 201ul);
         }
         aprsstr_Append(h, 201ul, "\012", 2ul);
         osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
         u = u->next;
      }
      strncpy(h,"f from init file, \012p ip/port protected\012B BC out\012b B\
C in\012A gets all\012",201u);
      osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      if (peertopeer) {
         strncpy(h,"peer-to-peer routing on\012",201u);
         osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      }
      i = 0UL;
      if (Call2Str(broadcastdest, 7ul, h, 201ul, 0UL, &i)) {
         h[i] = 0;
         aprsstr_Append(h, 201ul, " broadcast destination\012", 24ul);
         osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      }
      aprsstr_IntToStr((long)touserport, 0UL, h, 201ul);
      aprsstr_Append(h, 201ul, " user UDP port\012", 16ul);
      osi_WrBin(fd, (char *)h, 201u/1u, aprsstr_Length(h, 201ul));
      osic_Close(fd);
   }
   else Err("-w File Create", 15ul);
   X2C_PFREE(fn);
} /* end listtab() */


static char cmpcall(const RAWCALL c, const char b[], unsigned long b_len,
                unsigned long start, char withssid)
{
   unsigned long i;
   for (i = 0UL; i<=5UL; i++) {
      if (c[i]!=b[i+start]) return 0;
   } /* end for */
   return !withssid || ((unsigned long)(unsigned char)c[6U]/2UL&15UL)
                ==((unsigned long)(unsigned char)b[start+6UL]/2UL&15UL);
} /* end cmpcall() */


static pUSER Realloc(char alloc)
{
   unsigned long cnt;
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
         else osic_free((X2C_ADDRESS *) &u, sizeof(struct USER));
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
      osic_alloc((X2C_ADDRESS *) &new0, sizeof(struct USER));
   }
   /*  IF show THEN WrStr(" Table entries="); WrInt(cnt, 1); WrLn; END; */
   return new0;
} /* end Realloc() */


static void showu(unsigned long dp, pUSER u)
{
   if (u->nospoof) osi_WrStr(" writeprotected", 16ul);
   if (u->bcout) osi_WrStr(" bc-out", 8ul);
   if (u->bcin) osi_WrStr(" bc-in", 7ul);
   if (u->willall) osi_WrStr(" gets-all", 10ul);
   osi_WrStr(" IP:", 5ul);
   showpip(u->uip, dp);
   osic_WrLn();
} /* end showu() */


static void AddIp(unsigned long ip, unsigned long dp, char fix, char nspoof,
                char * hasbcin, char defbcin, char defbcout, char getsall,
                const char buf[], unsigned long buf_len)
{
   pUSER last;
   unsigned long i;
   pUSER u;
   /* for fast find, rechain to first position */
   struct USER * anonym;
   struct USER * anonym0;
   *hasbcin = 0;
   if (!((unsigned long)(unsigned char)buf[13UL]&1) && (unsigned char)
                buf[20UL]>=(unsigned char)'\200') return;
   /* via digi, store only direct heard */
   u = users;
   last = 0;
   while (u) {
      if (cmpcall(u->call, buf, buf_len, 7UL, 1)) {
         if (last) {
            last->next = u->next;
            u->next = users;
            users = u;
         }
         { /* with */
            struct USER * anonym = u;
            anonym->htime = systime;
            *hasbcin = anonym->bcin;
            if (!anonym->nospoof) {
               anonym->uip = ip; /* store if ip changed */
               anonym->dport = dp;
            }
         }
         /*
               IF show THEN
                 WrStr("Found User "); showcall(buf, SOURCECALL);
                 showu(u);
               END;
         */
         return;
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
               anonym0->call[i] = buf[7UL+i];
            } /* end for */
            anonym0->call[6U] = (char)(((unsigned long)(unsigned char)
                anonym0->call[6U]/2UL&15UL)*2UL); /* extract pure ssid */
            anonym0->uip = ip;
            anonym0->dport = dp;
            anonym0->htime = systime;
            anonym0->nospoof = nspoof;
            anonym0->nopurge = fix;
            anonym0->bcin = defbcin;
            anonym0->bcout = defbcout;
            anonym0->willall = getsall;
         }
         *hasbcin = defbcin;
         if (show) {
            osi_WrStr("Add User ", 10ul);
            showcall(buf, buf_len, 7UL);
            showu(dp, u);
         }
         u->next = users;
         users = u;
      }
   }
} /* end AddIp() */


static char sendtouser(char ubuf0[], unsigned long ubuf_len, long blen0,
                char fromdigi, char bcin, unsigned long fromip0,
                unsigned long fromport)
{
   pUSER uu;
   pUSER u;
   pUSER exactu;
   long ci;
   long res0;
   char ok0;
   char broadcast;
   char topeer;
   ok0 = 0;
   ci = 7L;
   if (((unsigned long)(unsigned char)ubuf0[13UL]&1)) ci = 0L;
   else {
      /* find first not done via */
      for (;;) {
         ci += 7L;
         if (ci>63L || ci+7L>=blen0) return 0;
         /* no valid frame */
         if ((unsigned char)ubuf0[ci+6L]<(unsigned char)'\200') break;
         if (((unsigned long)(unsigned char)ubuf0[ci+6L]&1)) {
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
                (unsigned long)ci, 1)) {
         /* conpare with ssid */
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
                || broadcast && u->bcout) || (topeer && exactu==0)
                && cmpcall(u->call, ubuf0, ubuf_len, (unsigned long)ci, 0))) {
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


static void err(char h[], unsigned long h_len, char fn[],
                unsigned long fn_len, unsigned long lc)
{
   char s[4001];
   X2C_PCOPY((void **)&h,h_len);
   X2C_PCOPY((void **)&fn,fn_len);
   osi_WrStr("Error in line ", 15ul);
   aprsstr_IntToStr((long)lc, 1UL, s, 4001ul);
   aprsstr_Append(s, 4001ul, ":[", 3ul);
   aprsstr_Append(s, 4001ul, fn, fn_len);
   aprsstr_Append(s, 4001ul, "] ", 3ul);
   aprsstr_Append(s, 4001ul, h, h_len);
   osi_WrStrLn(s, 4001ul);
   X2C_PFREE(h);
   X2C_PFREE(fn);
} /* end err() */


static void initroutes(char fn[], unsigned long fn_len)
{
   long fd;
   char call[201];
   char b[201];
   unsigned long lc;
   unsigned long j;
   unsigned long i;
   char dbcin0;
   char all;
   char bci;
   char bco;
   char spoof;
   unsigned long ip;
   unsigned long dp;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenRead(fn, fn_len);
   if (osic_FdValid(fd)) {
      lc = 1UL;
      for (;;) {
         i = 0UL;
         do {
            if (osi_RdBin(fd, (char *) &b[i], 1u/1u, 1UL)<1L) goto loop_exit;
            ++i;
         } while (!(i>=200UL || b[i-1UL]=='\012'));
         b[i] = 0;
         if (b[0U]!='#') {
            i = 0UL;
            if (!Str2Call(b, 201ul, &i, 7UL, call, 201ul)) {
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
            while ((unsigned char)b[i]>' ' && b[i]!='#') {
               b[j] = b[i];
               ++j;
               ++i;
            }
            b[j] = 0;
            if (GetIp1(b, 201ul, &ip, &dp)<0L) {
               err("wrong IP:PORT in Init File", 27ul, fn, fn_len, lc);
            }
            else {
               AddIp(ip, dp, 1, spoof, &dbcin0, bci, bco, all, call, 201ul);
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

static long blen;

static long res;

static unsigned long fromip;

static unsigned long userdport;

static unsigned long lastlist;

static char dbcin;

static pUSER voidu;


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
   alllifetime = 600UL;
   users = 0;
   initfn[0U] = 0;
   wrfn[0U] = 0;
   broadcastdest[0U] = 0;
   parms();
   if (initfn[0U]) initroutes(initfn, 1025ul);
   modified = 1;
   if (!peertopeer && digisock<0L) Err("need -u parameter", 18ul);
   usersock = openudp();
   if ((touserport==0UL || usersock<0L) || bindudp(usersock, touserport)<0L) {
      Err("cannot bind userport (-p userport)", 35ul);
   }
   systime = osic_time();
   for (;;) {
      fdclr();
      if (digisock>=0L) fdsetr((unsigned long)digisock);
      fdsetr((unsigned long)usersock);
      if (selectrw(15UL, 0UL)>0L) {
         if (digisock>=0L && issetr((unsigned long)digisock)) {
            /* data from digi */
            fromip = digiip;
            blen = getudp(digisock, ubuf, 338ul, &fromip, &userdport,
                checkdigiip);
            if (blen>=17L) {
               if (show) {
                  osi_WrStr("> from digi", 12ul);
                  ShowFrame(ubuf, 338ul, (unsigned long)(blen-2L), noinf);
               }
               if (!sendtouser(ubuf, 338ul, blen, 1, 1, digiip,
                userdport) && show) {
                  osi_WrStrLn(" digi out user not found", 25ul);
               }
               if (show) osi_WrStrLn("-----", 6ul);
            }
         }
         if (issetr((unsigned long)usersock)) {
            /* data from user */
            fromip = 0UL;
            blen = getudp(usersock, ubuf, 338ul, &fromip, &userdport, 0);
            if (blen>=17L) {
               if (show) {
                  osi_WrStr("> from user ", 13ul);
                  showpip(fromip, userdport);
                  ShowFrame(ubuf, 338ul, (unsigned long)(blen-2L), noinf);
               }
               AddIp(fromip, userdport, 0, 0, &dbcin, defaultbcin,
                defaultbcout, 0, ubuf, 338ul);
               modified = 1;
               if (digisock>=0L) {
                  res = udpsend(digisock, ubuf, blen, todigiport, digiip);
               }
               if (!sendtouser(ubuf, 338ul, blen, 0, dbcin, fromip,
                userdport) && show) {
                  osi_WrStrLn(" peer-to-peer no user found", 28ul);
               }
               if (show) osi_WrStrLn("-----", 6ul);
            }
         }
      }
      systime = osic_time();
      if (lastlist+15UL<systime || lastlist>systime) {
         voidu = Realloc(0); /* cyclic purge */
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
