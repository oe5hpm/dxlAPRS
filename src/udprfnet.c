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
#define udprfnet_C_
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* udp aprs grid network by OE5DXL */
/*
FROM FdSet IMPORT FdSet, FD_ZERO, FD_SET, FD_ISSET, TimeVal;
FROM mlib IMPORT select;
*/
#define udprfnet_MAXLEN 338

#define udprfnet_MINLEN 17

static unsigned long udprfnet_POLYNOM = 0x8408UL;

static unsigned long udprfnet_CRCINIT = 0xFFFFUL;

static unsigned long udprfnet_CRCRESULT = 0x9F0BUL;

#define udprfnet_CR "\015"

#define udprfnet_LF "\012"

#define udprfnet_MAXPATHLEN 80
/* header path size limit */

#define udprfnet_NOPORT 0
/* locked route udp port */

#define udprfnet_HTTPTIMEOUT 10

#define udprfnet_VERS "udprfnet(c) 0.1"

typedef char MONCALL[10];

typedef char FILENAME[1024];

struct NEIGHBOUR;

typedef struct NEIGHBOUR * pNEIGHBOUR;


struct NEIGHBOUR {
   pNEIGHBOUR next;
   MONCALL call;
   unsigned long toport;
   unsigned long ipnum;
   unsigned long pri;
   unsigned long uptime;
   unsigned long heard;
   unsigned long pollrtp;
   unsigned long pollrtr;
   unsigned long pollcnt;
   unsigned long pollns;
   char wantdata;
   unsigned long nsent;
   unsigned long nrec;
   unsigned long bsent;
   unsigned long brec;
   unsigned long medrtt;
};

struct TIMEHR;


struct TIMEHR {
   unsigned long sec;
   unsigned long nsec0;
};

static unsigned char CRCL[256];

static unsigned char CRCH[256];

static char verb;

static char localecho;

static char autonode;

static char localcheckip;

static unsigned long duptime;

static unsigned long systime;

static unsigned long qtime;

static unsigned long checktime;

static unsigned long removetime;

static unsigned long fastcheck;

static unsigned long slowcheck;

static long localsock;

static long netsock;

static unsigned long localfromport;

static unsigned long netport;

static unsigned long tx2port;

static pNEIGHBOUR neibors;

static unsigned long hashtab[65536];

static char starthead[64];

static char netname[64];

static FILENAME nodefile;

static struct NEIGHBOUR nlocal;

static char wwwbindport[6];

static char tbuf[512];

static long tlen;

static unsigned long connt;

static unsigned long upt;

static unsigned long httpcount;

extern long clock_gettime(unsigned long, struct TIMEHR *);


static void Err(const char text[], unsigned long text_len)
{
   osi_WrStr("udprfnet: ", 11ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */


static unsigned long nsec(void)
{
   struct TIMEHR t;
   long res0;
   /*
     gettime(t);
   */
   res0 = clock_gettime(1UL, &t);
   return t.nsec0;
} /* end nsec() */

#define udprfnet_POLINOM 0x8408 


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


static void getudp(long fd, char buf[], unsigned long buf_len,
                unsigned long * fromip0, unsigned long * fromport)
{
   long len;
   len = udpreceive(fd, buf, (long)(buf_len-1UL), fromport, fromip0);
   if (len<0L) len = 0L;
   buf[len] = 0;
} /* end getudp() */

/*
PROCEDURE sendudp(b-:ARRAY OF CHAR);
VAR i:INTEGER;
BEGIN
  i:=udp.udpsend(udpsock, buf, len+2, toport, ipnum);
END sendudp;
*/

static void hash(unsigned char * hashh, unsigned char * hashl, char c)
{
   unsigned char b;
   /*
   IO.WrStr("<");IO.WrChar(c);IO.WrStr(">");
   */
   if (c!=' ') {
      b = (unsigned char)((unsigned char)(unsigned char)c^*hashl);
      *hashl = CRCL[b]^*hashh;
      *hashh = CRCH[b];
   }
} /* end hash() */


static char Dup(const char b[], unsigned long b_len)
{
   unsigned long sum;
   unsigned long in;
   unsigned long p;
   unsigned char hashh;
   unsigned char hashl;
   char last;
   hashl = 0U;
   hashh = 0U;
   p = 0UL;
   in = 0UL;
   last = 0;
   /*
     WHILE (b[p]<>0C) & ((last<>":") OR (b[p]="}
                ")) DO last:=b[p]; INC(p) END;
   */
   for (;;) {
      if (b[p]==0) break;
      if (last==':') {
         if (b[p]=='}') in = p+1UL;
         else break;
      }
      last = b[p];
      ++p;
   }
   while (b[in] && b[in]!='>') {
      hash(&hashh, &hashl, b[in]);
      ++in;
   }
   while (((b[in] && b[in]!='-') && b[in]!=',') && b[in]!=':') {
      hash(&hashh, &hashl, b[in]);
      ++in;
   }
   while (b[p] && b[p]!='\015') {
      hash(&hashh, &hashl, b[p]);
      ++p;
   }
   sum = (unsigned long)(unsigned char)(char)hashl+(unsigned long)
                (unsigned char)(char)hashh*256UL;
   if (hashtab[sum]>systime) return 1;
   hashtab[sum] = systime+duptime;
   return 0;
} /* end Dup() */


static pNEIGHBOUR findneibor(pNEIGHBOUR chain0, unsigned long ip)
{
   while (chain0 && chain0->ipnum!=ip) chain0 = chain0->next;
   return chain0;
} /* end findneibor() */


static void unchain(pNEIGHBOUR * chain0, pNEIGHBOUR u)
{
   pNEIGHBOUR last;
   pNEIGHBOUR n;
   n = *chain0;
   last = 0;
   while (n && n!=u) {
      last = n;
      n = n->next;
   }
   if (n) {
      if (last) last->next = n->next;
      if (*chain0==n) *chain0 = n->next;
   }
} /* end unchain() */


static void chain(pNEIGHBOUR u)
{
   pNEIGHBOUR n;
   u->next = 0;
   n = neibors;
   if (n==0) neibors = u;
   else {
      while (n->next) n = n->next;
      n->next = u;
   }
} /* end chain() */


static pNEIGHBOUR addneibor(unsigned long ip, unsigned long port)
{
   pNEIGHBOUR n;
   n = findneibor(neibors, ip);
   if (n==0) {
      osic_alloc((X2C_ADDRESS *) &n, sizeof(struct NEIGHBOUR));
      if (n==0) Err("out of memory", 14ul);
      memset((X2C_ADDRESS)n,(char)0,sizeof(struct NEIGHBOUR));
      n->ipnum = ip;
      chain(n);
   }
   n->toport = port;
   return n;
} /* end addneibor() */


static long str2ip(const char h[], unsigned long h_len, unsigned long * p,
                unsigned long * ip, char nolp, unsigned long * dp,
                unsigned long * lp, char * check)
{
   unsigned long n;
   unsigned long i;
   char ok0;
   /*
   i:=p;
   WHILE h[i]>" " DO IO.WrChar(h[i]); INC(i) END; IO.WrLn;
   */
   *ip = 0UL;
   for (i = 0UL; i<=5UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((*p<h_len-1 && (unsigned char)h[*p]>='0') && (unsigned char)
                h[*p]<='9') {
         ok0 = 1;
         n = (n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
         ++*p;
      }
      if (!ok0) return -1L;
      if (i<3UL) {
         if (h[*p]!='.' || n>255UL) return -1L;
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[*p]!=':' || n>255UL) return -1L;
      }
      else if (i==4UL) {
         *dp = n;
         if (n>65535UL) return -1L;
         if (nolp) return 0L;
         *check = h[*p]=='/';
         if (h[*p]!=':' && h[*p]!='/') return -1L;
      }
      else if (n>65535UL) return -1L;
      *lp = n;
      ++*p;
   } /* end for */
   return 0L;
} /* end str2ip() */


static char GetNum(const char h[], unsigned long h_len, char eot,
                unsigned long * p, unsigned long * n)
{
   *n = 0UL;
   while ((unsigned char)h[*p]>='0' && (unsigned char)h[*p]<='9') {
      *n = ( *n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
      ++*p;
   }
   ++*p;
   return h[*p-1UL]==eot;
} /* end GetNum() */

/*
PROCEDURE GetSec(h:ARRAY OF CHAR; VAR p, n:CARDINAL):INTEGER;
VAR ok:BOOLEAN;
BEGIN
  h[HIGH(h)]:=0C;
  n:=0;
  ok:=FALSE;
  WHILE (h[p]>="0") & (h[p]<="9") DO
    ok:=TRUE;
    n:=n*10+ORD(h[p])-ORD("0");
    INC(p);
  END;
  IF NOT ok THEN RETURN -1 END;
(*
IO.WrCard(n,10); IO.WrLn;
*)
  RETURN 0
END GetSec;
*/
/*
PROCEDURE getfix(VAR x:REAL; s-:ARRAY OF CHAR; VAR p:CARDINAL):BOOLEAN;
VAR i:CARDINAL;
    h:ARRAY[0..255] OF CHAR;
BEGIN
  i:=0;
  WHILE (s[p]>" ") & (s[p]<>"/") & (i<HIGH(h)) DO h[i]:=s[p]; INC(i);
                INC(p) END;
  h[i]:=0C;
  IF StrToFix(x, h) THEN RETURN TRUE END;
  RETURN FALSE
END getfix;
*/

static void parms(void)
{
   char h[4096];
   char err;
   char lasth;
   unsigned long n;
   unsigned long i;
   err = 0;
   for (;;) {
      osi_NextArg(h, 4096ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         lasth = h[1U];
         if (lasth=='a') autonode = 1;
         else if (lasth=='c') {
            osi_NextArg(h, 4096ul);
            i = 0UL;
            if (!GetNum(h, 4096ul, ':', &i, &checktime) || !GetNum(h, 4096ul,
                 0, &i, &n)) Err("-c s:s", 7ul);
            qtime = checktime+n;
         }
         else if (lasth=='i') osi_NextArg(nlocal.call, 10ul);
         else if (lasth=='d') {
            osi_NextArg(h, 4096ul);
            i = 0UL;
            if (!GetNum(h, 4096ul, 0, &i, &duptime)) Err("-d number", 10ul);
         }
         else if (lasth=='e') localecho = 1;
         else if (lasth=='f') {
            osi_NextArg(h, 4096ul);
            i = 0UL;
            if (!GetNum(h, 4096ul, 0, &i, &fastcheck)) {
               Err("-f seconds", 11ul);
            }
         }
         else if (lasth=='n') osi_NextArg(netname, 64ul);
         else {
            if (lasth=='h') {
               osic_WrLn();
               osi_WrStrLn(" -a                     accept new neighbours",
                46ul);
               osi_WrStrLn(" -c <sec:sec>           check link time : stop tr\
ansfer after no reply", 71ul);
               osi_WrStrLn(" -d <seconds>           dupe filter time (default\
 57s min 27s)", 63ul);
               osi_WrStrLn(" -e                     echo local received frame\
s", 51ul);
               osi_WrStrLn(" -f <seconds>           poll intervall until repl\
y (default 10s)", 65ul);
               osi_WrStrLn(" -h                     this", 29ul);
               osi_WrStrLn(" -i <mycall>", 13ul);
               osi_WrStrLn(" -p <portnum>           network udp listen port",
                 48ul);
               osi_WrStrLn(" -M <ipnum:dport:lport> tnc-2 format local udp li\
nk", 52ul);
               osi_WrStrLn(" -m <portnum>           send a copy in tnc-2 moni\
tor to this port", 66ul);
               osi_WrStrLn(" -n <netname>", 14ul);
               osi_WrStrLn(" -r <filename>          static routes config file\
", 50ul);
               osi_WrStrLn(" -s <seconds>           poll intervall dead link \
(default 180s)", 64ul);
               osi_WrStrLn(" -v                     verbous to stdout",
                42ul);
               osi_WrStrLn(" -w <portnum>           www server listen port",
                47ul);
               osi_WrStrLn("udprfnet -i OE0AAA -a -e -M 127.0.0.1:2010:2020 -\
r routes.txt", 62ul);
               osic_WrLn();
               X2C_ABORT();
            }
            if (lasth=='M') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (str2ip(h, 4096ul, &i, &nlocal.ipnum, 0, &nlocal.toport,
                &localfromport, &localcheckip)<0L) {
                  Err("need -M ipnum:dport:lport", 26ul);
               }
               localsock = openudp();
               if (localsock<0L || bindudp(localsock, localfromport)<0L) {
                  Err("cannot open udp socket", 23ul);
               }
            }
            else if (lasth=='p') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (!GetNum(h, 4096ul, 0, &i, &netport)) {
                  Err("-p number", 10ul);
               }
            }
            else if (lasth=='m') {
               if (nlocal.toport==0UL) {
                  Err("need -M before -m secondport", 29ul);
               }
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (!GetNum(h, 4096ul, 0, &i, &tx2port)) {
                  Err("-m number", 10ul);
               }
            }
            else if (lasth=='r') osi_NextArg(nodefile, 1024ul);
            else if (lasth=='s') {
               osi_NextArg(h, 4096ul);
               i = 0UL;
               if (!GetNum(h, 4096ul, 0, &i, &slowcheck)) {
                  Err("-s seconds", 11ul);
               }
            }
            else if (lasth=='v') verb = 1;
            else if (lasth=='w') osi_NextArg(wwwbindport, 6ul);
            else err = 1;
         }
         h[0U] = 0;
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 4096ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end parms() */


static void showpip(unsigned long ip, unsigned long port)
{
   osic_WrINT32(ip/16777216UL, 1UL);
   osi_WrStr(".", 2ul);
   osic_WrINT32(ip/65536UL&255UL, 1UL);
   osi_WrStr(".", 2ul);
   osic_WrINT32(ip/256UL&255UL, 1UL);
   osi_WrStr(".", 2ul);
   osic_WrINT32(ip&255UL, 1UL);
   osi_WrStr(":", 2ul);
   osic_WrINT32(port, 1UL);
} /* end showpip() */


static void ipp2str(unsigned long ip, unsigned long port, char s[],
                unsigned long s_len)
{
   char h[256];
   aprsstr_IntToStr((long)(ip/16777216UL), 1UL, s, s_len);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip/65536UL&255UL), 1UL, h, 256ul);
   aprsstr_Append(s, s_len, h, 256ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip/256UL&255UL), 1UL, h, 256ul);
   aprsstr_Append(s, s_len, h, 256ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip&255UL), 1UL, h, 256ul);
   aprsstr_Append(s, s_len, h, 256ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((long)port, 1UL, h, 256ul);
   aprsstr_Append(s, s_len, h, 256ul);
} /* end ipp2str() */


static void showdata(char b[], unsigned long b_len, unsigned long len)
{
   X2C_PCOPY((void **)&b,b_len);
   if (len<=b_len-1) b[len] = 0;
   aprsstr_CtrlHex(b, b_len);
   osi_WrStrLn(b, b_len);
   X2C_PFREE(b);
/*
  IF len>70 THEN IO.WrLn END;
*/
} /* end showdata() */


static char noloop(const char b[], unsigned long b_len, const char w[],
                unsigned long w_len)
{
   unsigned long j;
   unsigned long i;
   i = 0UL;
   while (b[i] && b[i]!=':') {
      j = 0UL;
      while (b[i+j]==w[j]) ++j;
      if (w[j]==0) return 0;
      ++i;
   }
   return 1;
} /* end noloop() */


static char alive(pNEIGHBOUR n, unsigned long since)
{
   return n->call[0U] && n->pollrtr+since>=systime;
} /* end alive() */


static void sendall(char b[], unsigned long b_len, char local)
{
   unsigned long lastpri;
   unsigned long j;
   unsigned long len;
   unsigned long hlen;
   unsigned long mylen;
   unsigned long ilen;
   char best;
   long res0;
   pNEIGHBOUR n;
   struct NEIGHBOUR * anonym;
   unsigned long tmp;
   X2C_PCOPY((void **)&b,b_len);
   if (b[0UL]==0) goto label;
   if (Dup(b, b_len)) {
      if (verb) {
         osi_WrStr("dup:", 5ul);
         showdata(b, b_len, aprsstr_Length(b, b_len));
      }
      goto label;
   }
   if (verb) {
      osic_WrLn();
      osi_WrStr("rx: ", 5ul);
      showdata(b, b_len, aprsstr_Length(b, b_len));
   }
   ilen = 0UL;
   while (b[ilen]!=':') {
      /* find end of transport head */
      if (b[ilen]==0) goto label;
      ++ilen;
   }
   hlen = ilen;
   if (hlen>80UL) goto label;
   /* limit hopps */
   while (b[ilen]) ++ilen;
   ++ilen; /* 0C at end will be sent */
   mylen = aprsstr_Length(nlocal.call, 10ul)+1UL;
   if (mylen<2UL) goto label;
   /* no valid append call */
   if (b[hlen+1UL]!='}') goto label;
   /* not a transport head */
   if (local && nlocal.toport) {
      res0 = udpsend(localsock, b, (long)ilen, nlocal.toport, nlocal.ipnum);
      if (tx2port) {
         res0 = udpsend(localsock, b, (long)ilen, tx2port, nlocal.ipnum);
      }
      ++nlocal.nsent;
      nlocal.bsent += ilen;
      if (verb) {
         osi_WrStr("tx: ", 5ul);
         showpip(nlocal.ipnum, nlocal.toport);
         osic_WrLn();
      }
   }
   len = ilen+mylen;
   if (len>=b_len-1) goto label;
   /* result too long */
   while (ilen>=hlen) {
      b[ilen+mylen] = b[ilen];
      --ilen;
   }
   ++ilen;
   b[ilen] = ',';
   ++ilen;
   tmp = mylen-2UL;
   j = 0UL;
   if (j<=tmp) for (;; j++) {
      b[ilen] = nlocal.call[j];
      ++ilen;
      if (j==tmp) break;
   } /* end for */
   n = neibors;
   lastpri = X2C_max_longcard;
   best = 1;
   while (n) {
      { /* with */
         struct NEIGHBOUR * anonym = n;
         if (anonym->pri<=lastpri) best = 1;
         lastpri = anonym->pri;
         if (anonym->toport && alive(n, qtime)) {
            if ((anonym->wantdata && best) && noloop(b, b_len, anonym->call,
                10ul)) {
               if (verb) {
                  osi_WrStr("tx: ", 5ul);
                  showpip(anonym->ipnum, anonym->toport);
                  osic_WrLn();
               }
               res0 = udpsend(netsock, b, (long)len, anonym->toport,
                anonym->ipnum);
               ++anonym->nsent;
               anonym->bsent += len;
            }
            best = 0;
         }
      }
      n = n->next;
   }
   label:;
   X2C_PFREE(b);
/*
IO.WrStr("<"); IO.WrStr(b); IO.WrStrLn(">");
*/
} /* end sendall() */


static void pollstr(char b[], unsigned long b_len, unsigned long * p,
                pNEIGHBOUR pn)
{
   unsigned long i;
   *p = 0UL;
   while (*p<=63UL && starthead[*p]!=':') {
      b[*p] = starthead[*p];
      ++*p;
   }
   b[*p] = ',';
   ++*p;
   i = 0UL;
   if (pn->call[0U]) {
      while (i<=9UL && pn->call[i]) {
         b[*p] = pn->call[i];
         ++*p;
         ++i;
      }
   }
   else {
      b[*p] = 'W';
      ++*p;
      b[*p] = 'H';
      ++*p;
      b[*p] = 'O';
      ++*p;
      b[*p] = 'I';
      ++*p;
      b[*p] = 'S';
      ++*p;
   }
   b[*p] = ',';
   ++*p;
   b[*p] = 'R';
   ++*p;
   b[*p] = 'T';
   ++*p;
} /* end pollstr() */


static void pollnr(char b[], unsigned long b_len, unsigned long n)
{
   b[0UL] = (char)((n/100UL)%10UL+48UL);
   b[1UL] = (char)((n/10UL)%10UL+48UL);
   b[2UL] = (char)(n%10UL+48UL);
} /* end pollnr() */


static char checksrc(const char b[], unsigned long b_len, const char c[],
                unsigned long c_len)
{
   unsigned long i;
   i = 0UL;
   while (i<c_len-1 && c[i]==b[i]) ++i;
   return b[i]=='>' && c[i]==0;
} /* end checksrc() */


static char checkdest(const char b[], unsigned long b_len,
                unsigned long via)
{
   unsigned long i;
   i = 0UL;
   while (i<9UL && nlocal.call[i]==b[via]) {
      ++i;
      ++via;
   }
   return b[via]==',' && nlocal.call[i]==0;
} /* end checkdest() */


static void setcall(const char b[], unsigned long b_len, char c[],
                unsigned long c_len)
{
   unsigned long i;
   i = 0UL;
   while (i<c_len-1 && b[i]!='>') {
      c[i] = b[i];
      ++i;
   }
   c[i] = 0;
} /* end setcall() */


static char statneibor(const char b[], unsigned long b_len,
                unsigned long fromip0, unsigned long fromport)
{
   pNEIGHBOUR n;
   unsigned long rtt;
   unsigned long via;
   unsigned long len;
   unsigned long j;
   unsigned long i;
   char h[256];
   char pb[256];
   long res0;
   struct NEIGHBOUR * anonym;
   len = aprsstr_Length(b, b_len);
   n = findneibor(neibors, fromip0);
   if (n && n->toport==0UL) return 0;
   /* route locked */
   if (n==0) {
      if (autonode) n = addneibor(fromip0, fromport);
      else return 0;
      if (n==0) return 0;
      /* out of memory or locked */
      n->pri = 0UL; /* mark as autonode */
   }
   n->heard = systime;
   i = 0UL;
   while (i<len && b[i]!='>') ++i;
   ++i;
   j = 0UL;
   for (;;) {
      if (netname[j]==0) break;
      if (netname[j]!=b[i]) {
         if (verb) {
            showpip(n->ipnum, n->toport);
            osi_WrStrLn(" got wrong netname", 19ul);
         }
         return 0;
      }
      /* netname wrong */
      ++i;
      ++j;
   }
   if (b[i]!=',') return 0;
   /* from>netname, */
   ++i;
   via = i;
   while (i<len) {
      if ((b[i]==',' && b[i+1UL]=='R') && b[i+2UL]=='T') {
         { /* with */
            struct NEIGHBOUR * anonym = n;
            if (b[i+3UL]=='P' || b[i+3UL]=='S') {
               /* got RT(P/S)nnn */
               anonym->wantdata = b[i+3UL]=='P';
               if (anonym->pri>1UL && anonym->pollrtp+qtime<systime) {
                  return 0;
               }
               /* sleeping 2nd link */
               if (!(checksrc(b, b_len, anonym->call, 10ul) && checkdest(b,
                b_len, via))) {
                  /* get poll from unkown src */
                  anonym->pollrtr = 0UL;
                  anonym->medrtt = 0UL;
                  if (((((b[via]=='W' && b[via+1UL]=='H') && b[via+2UL]=='O')
                 && b[via+3UL]=='I') && b[via+4UL]=='S') && b[via+5UL]==',') {
                     anonym->pollrtp = 0UL;
                     setcall(b, b_len, anonym->call, 10ul);
                     if (verb) {
                        showpip(anonym->ipnum, anonym->toport);
                        osi_WrStr(" got WHOIS from ", 17ul);
                        osi_WrStrLn(anonym->call, 10ul);
                     }
                  }
                  else {
                     /*
                                   IF verb THEN showpip(ipnum, toport);
                WrStrLn(" sends wrong mycall ") END;
                     */
                     anonym->call[0U] = 0;
                     return 0;
                  }
               }
               pollstr(pb, 256ul, &j, n);
               if (anonym->pri==0UL) pb[j] = 'A';
               else pb[j] = 'R';
               ++j;
               pb[j] = b[i+4UL];
               ++j;
               pb[j] = b[i+5UL];
               ++j;
               pb[j] = b[i+6UL];
               ++j;
               pb[j] = ':';
               ++j;
               pb[j] = 0;
               ++j;
               res0 = udpsend(netsock, pb, (long)j, anonym->toport,
                anonym->ipnum);
               return 0;
            }
            if (b[i+3UL]=='R' || b[i+3UL]=='A') {
               /* got RT(R/A)nnn */
               pollnr(h, 256ul, anonym->pollcnt);
               if ((b[i+4UL]==h[0U] && b[i+5UL]==h[1U]) && b[i+6UL]==h[2U]) {
                  /* RTR num fits */
                  if (checkdest(b, b_len, via)) {
                     /* destination call fits */
                     if (anonym->uptime==0UL) {
                        anonym->uptime = systime;
                        anonym->nsent = 0UL;
                        anonym->bsent = 0UL;
                        anonym->nrec = 0UL;
                        anonym->brec = 0UL;
                        anonym->medrtt = 0UL;
                        anonym->call[0U] = 0;
                     }
                     if (!checksrc(b, b_len, anonym->call, 10ul)) {
                        setcall(b, b_len, anonym->call, 10ul);
                        if (verb) {
                           showpip(anonym->ipnum, anonym->toport);
                           osi_WrStr(" got RTR and set call from ", 28ul);
                           osi_WrStrLn(anonym->call, 10ul);
                        }
                     }
                     if (anonym->pri==0UL && b[i+3UL]=='A') {
                        anonym->pollrtr = 0UL; /* both sides autoroute */
                     }
                     else anonym->pollrtr = systime;
                     rtt = nsec();
                     rtt = (rtt-anonym->pollns)/1000UL;
                     if (anonym->medrtt==0UL) anonym->medrtt = rtt;
                     else {
                        anonym->medrtt += (unsigned long)((long)
                (rtt-anonym->medrtt)/10L);
                     }
                     if (verb) {
                        showpip(anonym->ipnum, anonym->toport);
                        osi_WrStr(" new rtt=", 10ul);
                        osic_WrINT32(anonym->medrtt, 1UL);
                        osi_WrStrLn("us", 3ul);
                     }
                  }
                  else {
                     if (verb) {
                        showpip(anonym->ipnum, anonym->toport);
                        osi_WrStrLn(" reply wrong mycall ", 21ul);
                     }
                     anonym->call[0U] = 0;
                     anonym->pollrtr = 0UL; /* reset link */
                  }
               }
               else {
                  if (verb) {
                     showpip(anonym->ipnum, anonym->toport);
                     osi_WrStrLn(" got wrong RTR number", 22ul);
                  }
                  anonym->pollrtr = systime-qtime;
                /* long delay, stop data tx */
               }
               return 0;
            }
         }
      }
      ++i;
   }
   ++n->nrec;
   n->brec += len;
   return 1;
} /* end statneibor() */


static char needdata(pNEIGHBOUR tn)
{
   pNEIGHBOUR n;
   if (nlocal.toport) return 1;
   /* local need data */
   n = neibors; /* look if other neibor needs data */
   while (n) {
      if (n!=tn && n->wantdata) return 1;
      n = n->next;
   }
   return 0;
/* noone needs data */
} /* end needdata() */


static void checklinks(void)
{
   pNEIGHBOUR n;
   unsigned long lastpri;
   unsigned long j;
   char try0;
   unsigned long ct;
   char h[256];
   char pb[256];
   long res0;
   struct NEIGHBOUR * anonym;
   /*
   IO.WrCard(pollrtr,1);IO.WrStrLn("=pollrtr");
   */
   lastpri = X2C_max_longcard;
   try0 = 1;
   n = neibors;
   while (n) {
      { /* with */
         struct NEIGHBOUR * anonym = n;
         if (anonym->pri<=lastpri) try0 = 1;
         lastpri = anonym->pri;
         ct = slowcheck;
         if (alive(n, qtime)) ct = fastcheck;
         else anonym->uptime = 0UL;
         if ((anonym->toport && try0) && anonym->pollrtr+checktime<systime) {
            if (anonym->pollrtp+ct<systime) {
               anonym->pollrtp = systime;
               ++anonym->pollcnt;
               anonym->pollns = nsec();
               if (verb) {
                  showpip(anonym->ipnum, anonym->toport);
                  osi_WrStrLn(" send check poll", 17ul);
               }
               pollstr(pb, 256ul, &j, n);
               if (needdata(n)) pb[j] = 'P';
               else pb[j] = 'S';
               ++j;
               pollnr(h, 256ul, anonym->pollcnt);
               pb[j] = h[0U];
               ++j;
               pb[j] = h[1U];
               ++j;
               pb[j] = h[2U];
               ++j;
               pb[j] = ':';
               ++j;
               pb[j] = 0;
               ++j;
               res0 = udpsend(netsock, pb, (long)j, n->toport, n->ipnum);
            }
         }
         if (alive(n, qtime)) try0 = 0;
         n = anonym->next;
      }
   }
} /* end checklinks() */


static void readroutes(void)
{
   long l;
   long f;
   pNEIGHBOUR old;
   pNEIGHBOUR n;
   char h[65536];
   unsigned long len;
   unsigned long p;
   unsigned long pr;
   char ok0;
   unsigned long ip;
   unsigned long port;
   unsigned long fromport;
   len = 0UL;
   if (nodefile[0U]) {
      f = osi_OpenRead(nodefile, 1024ul);
      if (f<0L) {
         if (verb) {
            osi_WrStr("routefile not readable <", 25ul);
            osi_WrStr(nodefile, 1024ul);
            osi_WrStrLn(">", 2ul);
         }
      }
      else {
         l = osi_RdBin(f, (char *)h, 65536u/1u, 65535UL);
         osic_Close(f);
         if (l<0L) {
            if (verb) {
               osi_WrStr("routefile not read error <", 27ul);
               osi_WrStr(nodefile, 1024ul);
               osi_WrStrLn(">", 2ul);
            }
         }
         else len = (unsigned long)l;
      }
   }
   h[len] = 0;
   old = neibors;
   neibors = 0;
   p = 0UL;
   pr = 0UL;
   while (p<len) {
      if (h[p]=='#') {
         while (p<len && (unsigned char)h[p]>=' ') ++p;
         while (p<len && (unsigned char)h[p]<' ') ++p;
      }
      else {
         if (str2ip(h, 65536ul, &p, &ip, 1, &port, &fromport, &ok0)>=0L) {
            n = findneibor(old, ip);
            if (n) {
               unchain(&old, n);
               n->toport = port;
               chain(n);
            }
            else n = addneibor(ip, port);
            if (n) n->pri = pr+1UL;
            ++pr;
         }
         else if (verb) osi_WrStrLn("routefile wrong ip:port number", 31ul);
         while (p<len && h[p]==' ') ++p;
         if ((unsigned char)h[p]<' ') pr = 0UL;
         while (p<len && (unsigned char)h[p]<' ') ++p;
      }
   }
   while (old) {
      n = old;
      old = old->next;
      if (n->pri==0UL && alive(n, removetime)) chain(n);
      else {
         if (verb) {
            showpip(n->ipnum, n->toport);
            osi_WrStrLn(" dead autoroute removed", 24ul);
         }
         osic_free((X2C_ADDRESS *) &n, sizeof(pNEIGHBOUR));
      }
   }
} /* end readroutes() */


static void showroutes(void)
{
   pNEIGHBOUR n;
   char h[256];
   struct NEIGHBOUR * anonym;
   n = neibors;
   osi_WrStrLn("routing table:==============================", 45ul);
   while (n) {
      { /* with */
         struct NEIGHBOUR * anonym = n;
         showpip(anonym->ipnum, anonym->toport);
         osi_WrStr(" pri=", 6ul);
         osic_WrINT32(anonym->pri, 1UL);
         if (anonym->uptime>0UL) {
            aprsstr_TimeToStr(systime-anonym->uptime, h, 256ul);
            osi_WrStr(" up:", 5ul);
            osi_WrStr(h, 256ul);
         }
         osi_WrStr(" ", 2ul);
         osi_WrStr(anonym->call, 10ul);
         if (anonym->heard>0UL) {
            osi_WrStr(" heard:", 8ul);
            osic_WrINT32(systime-anonym->heard, 1UL);
         }
         osi_WrStr(" rtt:", 6ul);
         osic_WrINT32(anonym->medrtt, 1UL);
         osi_WrStr(" tf:", 5ul);
         osic_WrINT32(anonym->nsent, 1UL);
         osi_WrStr(" rf:", 5ul);
         osic_WrINT32(anonym->nrec, 1UL);
         osic_WrLn();
         n = anonym->next;
      }
   }
   osi_WrStrLn("============================================", 45ul);
} /* end showroutes() */


static char cmpfrom(const char a[], unsigned long a_len, unsigned long from,
                const char b[], unsigned long b_len)
{
   unsigned long i;
   i = 0UL;
   while (from<=a_len-1 && b[i]) {
      if (a[from]!=b[i]) return 0;
      ++i;
      ++from;
   }
   return 1;
} /* end cmpfrom() */

#define udprfnet_MAXRED 120


static char hex(unsigned long n)
{
   if (n>9UL) n += 7UL;
   return (char)(n+48UL);
} /* end hex() */


static void wint(char h[1024], long n, long col, char red)
{
   char h1[16];
   char b;
   char a;
   col = 224L-col;
   if (col<0L) col = 0L;
   strncpy(h1,"E0E0E0",16u);
   a = hex((unsigned long)(col/16L));
   b = hex((unsigned long)(col&15L));
   h1[4U] = a;
   h1[5U] = b;
   if (red) {
      h1[2U] = a;
      h1[3U] = b;
   }
   else {
      h1[0U] = a;
      h1[1U] = b;
   }
   aprsstr_Append(h, 1024ul, "<TD style=\"text-align:right\" BGCOLOR=\"#",
                40ul);
   aprsstr_Append(h, 1024ul, h1, 16ul);
   aprsstr_Append(h, 1024ul, "\">", 3ul);
   aprsstr_IntToStr(n, 1UL, h1, 16ul);
   aprsstr_Append(h, 1024ul, h1, 16ul);
   aprsstr_Append(h, 1024ul, "</TD>", 6ul);
} /* end wint() */


static void Www(long * sock)
{
   char h[1024];
   char h1[256];
   long maxbs;
   long res0;
   unsigned long tt;
   unsigned long i;
   pNEIGHBOUR ln;
   pNEIGHBOUR n;
   struct NEIGHBOUR * anonym;
   struct NEIGHBOUR * anonym0;
   res0 = readsock(*sock, h, 1023L);
   if (res0<0L) {
      osic_Close(*sock);
      *sock = -1L;
      return;
   }
   h[res0] = 0;
   i = 0UL;
   while (i<=8UL && !cmpfrom(h, 1024ul, i, " / ", 4ul)) ++i;
   do {
   } while (readsock(*sock, h, 1024L)>0L);
   /* clear iput buffer */
   if (i>=8UL) strncpy(h,"HTTP/1.1 404\015\012",1024u);
   else {
      ++httpcount;
      strncpy(h,"HTTP/1.0 200 OK\015\012Content-Type: text/html; charset=iso-\
8859-1\015\012\015\012",1024u);
      res0 = sendsock(*sock, h, (long)aprsstr_Length(h, 1024ul));
      strncpy(h,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//\
EN\"\015\012\"http://www.w3.org/TR/html4/loose.dtd\">\015\012<HTML><HEAD><TIT\
LE>",1024u);
      aprsstr_Append(h, 1024ul, nlocal.call, 10ul);
      aprsstr_Append(h, 1024ul, " Status Report</TITLE></HEAD><BODY>\015\012",
                 38ul);
      res0 = sendsock(*sock, h, (long)aprsstr_Length(h, 1024ul));
      h[0] = 0;
      strncpy(h,"<TABLE BORDER=\"0\" CELLPADDING=\"3\" CELLSPACING=\"1\" BGCO\
LOR=\"#000000\" align=\"center\" SUMMARY=\"Connection Table\">\015\012<TR VAL\
IGN=\"BASELINE\" BGCOLOR=\"#E0C0A0\"><TH COLSPAN=14 style=\"text-align:center\\
">",1024u);
      if (nlocal.call[0U]) {
         aprsstr_Append(h, 1024ul, "Server ", 8ul);
         aprsstr_Append(h, 1024ul, nlocal.call, 10ul);
      }
      aprsstr_Append(h, 1024ul, " [udprfnet(c) 0.1] http#", 25ul);
      aprsstr_IntToStr((long)httpcount, 1UL, h1, 256ul);
      aprsstr_Append(h, 1024ul, h1, 256ul);
      aprsstr_Append(h, 1024ul, " Netname [", 11ul);
      aprsstr_Append(h, 1024ul, netname, 64ul);
      aprsstr_Append(h, 1024ul, "] Inport ", 10ul);
      aprsstr_IntToStr((long)netport, 1UL, h1, 256ul);
      aprsstr_Append(h, 1024ul, h1, 256ul);
      if (upt<systime) {
         aprsstr_TimeToStr(systime-upt, h1, 256ul);
         aprsstr_Append(h, 1024ul, " Uptime ", 9ul);
         aprsstr_Append(h, 1024ul, h1, 256ul);
      }
      aprsstr_Append(h, 1024ul, "</TH></TR>\015\012<TR BGCOLOR=\"#CCCCFF\">\01\
5\012<TH>Pri</TH><TH>IP</TH><TH>Call</TH><TH>Data</TH><TH>TxByte</TH><TH>TxFr\
</TH><TH>bit/s</TH><TH>RxByte</TH><TH>RxFr</TH><TH>bit/s</TH><TH>RTT(us)</TH>\
<TH>Heard</TH><TH>Up</TH></TR>", 216ul);
      res0 = sendmore(*sock, h, (long)aprsstr_Length(h, 1024ul));
      nlocal.next = neibors;
      maxbs = 1L;
      ln = &nlocal;
      n = ln;
      while (n) {
         { /* with */
            struct NEIGHBOUR * anonym = n;
            if (anonym->uptime>0UL && anonym->uptime<systime) {
               tt = systime-anonym->uptime;
               res0 = (long)((anonym->bsent*8UL)/tt);
               if (res0>maxbs) maxbs = res0;
               res0 = (long)((anonym->brec*8UL)/tt);
               if (res0>maxbs) maxbs = res0;
            }
            n = anonym->next;
         }
      }
      n = ln;
      while (n) {
         { /* with */
            struct NEIGHBOUR * anonym0 = n;
            if (anonym0->uptime>0UL && anonym0->uptime<systime) {
               tt = systime-anonym0->uptime;
            }
            else {
               tt = 0UL;
            }
            strncpy(h,"<TR style=\"text-align:center\" BGCOLOR=\"#",1024u);
            if (anonym0->uptime==0UL) strncpy(h1,"E0E0E0",256u);
            else if (n==ln) strncpy(h1,"E0E0B0",256u);
            else if (anonym0->pri==0UL) strncpy(h1,"C0D0E0",256u);
            else strncpy(h1,"C0E0C0",256u);
            aprsstr_Append(h, 1024ul, h1, 256ul);
            aprsstr_Append(h, 1024ul, "\"><TD>", 7ul);
            if (anonym0->pri==0UL) strncpy(h1,"auto",256u);
            else aprsstr_IntToStr((long)anonym0->pri, 1UL, h1, 256ul);
            aprsstr_Append(h, 1024ul, h1, 256ul);
            aprsstr_Append(h, 1024ul,
                "</TD><TD style=\"text-align:center\">", 36ul);
            ipp2str(anonym0->ipnum, anonym0->toport, h1, 256ul);
            aprsstr_Append(h, 1024ul, h1, 256ul);
            aprsstr_Append(h, 1024ul, "</TD><TD style=\"text-align:left\">",
                34ul);
            aprsstr_Append(h, 1024ul, anonym0->call, 10ul);
            aprsstr_Append(h, 1024ul,
                "</TD><TD style=\"text-align:center\">", 36ul);
            if (anonym0->wantdata) aprsstr_Append(h, 1024ul, "yes", 4ul);
            else aprsstr_Append(h, 1024ul, "no", 3ul);
            aprsstr_Append(h, 1024ul, "</TD>", 6ul);
            if (tt>0UL) res0 = (long)((anonym0->bsent*8UL)/tt);
            else res0 = 0L;
            wint(h, (long)anonym0->bsent, (res0*120L)/maxbs, 1);
            wint(h, (long)anonym0->nsent, (res0*120L)/maxbs, 1);
            wint(h, res0, (res0*120L)/maxbs, 1);
            if (tt>0UL) res0 = (long)((anonym0->brec*8UL)/tt);
            else res0 = 0L;
            wint(h, (long)anonym0->brec, (res0*120L)/maxbs, 0);
            wint(h, (long)anonym0->nrec, (res0*120L)/maxbs, 0);
            wint(h, res0, (res0*120L)/maxbs, 0);
            aprsstr_Append(h, 1024ul, "<TD style=\"text-align:right\">",
                30ul);
            aprsstr_IntToStr((long)anonym0->medrtt, 1UL, h1, 256ul);
            aprsstr_Append(h, 1024ul, h1, 256ul);
            aprsstr_Append(h, 1024ul, "</TD><TD>", 10ul);
            if (anonym0->uptime>0UL && systime>=anonym0->heard) {
               aprsstr_TimeToStr(systime-anonym0->heard, h1, 256ul);
               aprsstr_Append(h, 1024ul, h1, 256ul);
            }
            aprsstr_Append(h, 1024ul, "</TD><TD>", 10ul);
            if (anonym0->uptime>0UL) {
               aprsstr_TimeToStr(tt, h1, 256ul);
               aprsstr_Append(h, 1024ul, h1, 256ul);
            }
            aprsstr_Append(h, 1024ul, "</TD></TR>", 11ul);
            res0 = sendmore(*sock, h, (long)aprsstr_Length(h, 1024ul));
            n = anonym0->next;
         }
      }
      strncpy(h,"</TABLE></BODY></HTML>\015\012",1024u);
   }
   res0 = sendsock(*sock, h, (long)aprsstr_Length(h, 1024ul));
   osic_Close(*sock);
   *sock = -1L;
} /* end Www() */


static void addhead(char b[], unsigned long b_len)
{
   unsigned long k;
   unsigned long j;
   unsigned long i;
   struct NEIGHBOUR * anonym;
   unsigned long tmp;
   j = aprsstr_Length(b, b_len);
   { /* with */
      struct NEIGHBOUR * anonym = &nlocal;
      ++anonym->nrec;
      anonym->brec += j;
      if (anonym->uptime==0UL || anonym->heard+600UL<systime) {
         anonym->uptime = systime;
         anonym->nsent = 0UL;
         anonym->bsent = 0UL;
         anonym->nrec = 0UL;
         anonym->brec = 0UL;
         anonym->medrtt = 0UL;
      }
      anonym->heard = systime;
   }
   k = aprsstr_Length(starthead, 64ul);
   if (k==0UL) return;
   if (j+k>=b_len-1) {
      b[0UL] = 0;
      return;
   }
   for (i = j;; i--) {
      b[i+k] = b[i];
      if (i==0UL) break;
   } /* end for */
   tmp = k-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      b[i] = starthead[i];
      if (i==tmp) break;
   } /* end for */
} /* end addhead() */

static long res;

/*maxsock,*/
static long resi;

static char mbuf[512];
/*
    rset, wset            : FdSet;
    tv                    : TimeVal;
*/

static unsigned long fromip;

static unsigned long lasttime;

static unsigned long sport;

static long wwwsock;

static long tcpsock;


static void addsock(long fd, char wtoo)
/* insert socket in fdset for select */
{
   if (fd>=0L) {
      fdsetr((unsigned long)fd);
      if (wtoo) fdsetw((unsigned long)fd);
   }
/*
    IF fd>maxsock THEN maxsock:=fd END;
*/
} /* end addsock() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   long tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(MONCALL)!=10) X2C_ASSERT(0);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   Gencrctab();
   memset((char *) &nlocal,(char)0,sizeof(struct NEIGHBOUR));
   nlocal.pri = 1UL;
   tx2port = 0UL;
   neibors = 0;
   duptime = 57UL;
   strncpy(nlocal.call,"NOCALL",10u);
   strncpy(netname,"RFNET",64u);
   autonode = 0;
   nodefile[0U] = 0;
   netport = 9100UL;
   localecho = 0;
   checktime = 120UL; /* poll link fast */
   qtime = checktime+60UL; /* stop data poll slow */
   fastcheck = 10UL; /* poll active link */
   slowcheck = qtime; /* poll dead link */
   verb = 0;
   strncpy(wwwbindport,"9080",6u);
   upt = systime;
   parms();
   removetime = qtime+60UL; /* delete autolink */
   if (nlocal.call[0U]==0) Err("need mycall", 12ul);
   if (netname[0U]==0) Err("need netname", 13ul);
   netname[63U] = 0;
   nlocal.call[9U] = 0;
   systime = osic_time();
   upt = systime;
   if (nlocal.toport) {
      nlocal.uptime = systime;
      nlocal.heard = systime;
      nlocal.wantdata = 1;
   }
   tcpsock = -1L;
   wwwsock = -1L;
   tlen = 0L;
   aprsstr_Assign(starthead, 64ul, nlocal.call, 10ul);
   aprsstr_Append(starthead, 64ul, ">", 2ul);
   aprsstr_Append(starthead, 64ul, netname, 64ul);
   aprsstr_Append(starthead, 64ul, ":}", 3ul);
   netsock = openudp();
   if (netsock>=0L && bindudp(netsock, netport)<0L) {
      Err("cannot bind inport", 19ul);
   }
   readroutes();
   for (;;) {
      systime = osic_time();
      if (lasttime!=systime) {
         lasttime = systime;
         checklinks();
         if (systime%30UL==0UL) {
            readroutes();
            if (verb) showroutes();
         }
         if (tcpsock<0L && wwwbindport[0U]) {
            /* open listensocket www connects */
            tcpsock = waitconnect(wwwbindport, 4UL);
            if (verb && tcpsock<0L) {
               osi_WrStr("cant bind to port ", 19ul);
               osi_WrStrLn(wwwbindport, 6ul);
            }
         }
         if (wwwsock>=0L) {
            if (connt==0UL) connt = systime;
            else if (connt+10UL<systime) {
               strncpy(tbuf,"timeout\015\012",512u);
               res = sendsock(wwwsock, tbuf, (long)aprsstr_Length(tbuf,
                512ul));
               osic_Close(wwwsock);
               wwwsock = -1L;
            }
         }
      }
      fdclr();
      /*    
          FD_ZERO(rset);
          FD_ZERO(wset);
          maxsock:=0;
      */
      addsock(localsock, 0);
      addsock(netsock, 0);
      if (wwwsock<0L) addsock(tcpsock, 0);
      else addsock(wwwsock, tlen>0L);
      /*
          tv.tv_usec:=0;
          tv.tv_sec:=1;
          res:=select(maxsock+1, ADR(rset), ADR(wset), NIL, ADR(tv));
          IF FD_ISSET(localsock, rset) THEN
      */
      res = selectr(1UL, 0UL);
      if (issetr((unsigned long)localsock)) {
         getudp(localsock, mbuf, 512ul, &fromip, &sport);
         if (!localcheckip || fromip==nlocal.ipnum) {
            addhead(mbuf, 512ul);
            sendall(mbuf, 512ul, localecho);
         }
      }
      if (issetr((unsigned long)netsock)) {
         getudp(netsock, mbuf, 512ul, &fromip, &sport);
         if (statneibor(mbuf, 512ul, fromip, sport) && noloop(mbuf, 512ul,
                nlocal.call, 10ul)) sendall(mbuf, 512ul, 1);
      }
      if (tcpsock>=0L && issetr((unsigned long)tcpsock)) {
         /* tcp listensocket has news */
         res = 512L;
         wwwsock = acceptconnect(tcpsock, mbuf, &res);
         if (wwwsock>=0L) {
            /* a new www connect */
            res = socknonblock(wwwsock);
            connt = 0UL;
            tlen = 0L;
         }
      }
      if (wwwsock>=0L) {
         if (issetw((unsigned long)wwwsock)) {
            /* data to www */
            res = sendsock(wwwsock, tbuf, tlen);
            if (res>0L) {
               tmp = tlen-1L;
               resi = res;
               if (resi<=tmp) for (;; resi++) {
                  tbuf[resi-res] = tbuf[resi]; /* delete the sent part */
                  if (resi==tmp) break;
               } /* end for */
               tlen -= res;
               if (connt==0UL) connt = systime;
            }
            else tlen = 0L;
         }
         else if (issetr((unsigned long)wwwsock)) Www(&wwwsock);
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
