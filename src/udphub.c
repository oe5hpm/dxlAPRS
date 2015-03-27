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
#ifndef Storage_H_
#include "Storage.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef udp_H_
#include "udp.h"
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

/* axudp bidirectional digi - user hub by OE5DXL */
/*
FROM FIO IMPORT OpenMode, Close, oRDONLY, oAPPEND, oWRONLY, oNONBLOCK,
                Create, WrBin, RdBin, File;
*/
#define udphub_CALLLEN 7

#define udphub_MAXLEN 338

#define udphub_MINLEN 17

static unsigned long udphub_POLYNOM = 0x8408UL;

static unsigned long udphub_CRCINIT = 0xFFFFUL;

#define udphub_SOURCECALL 7

#define udphub_DESTCALL 0

#define udphub_MAXFD 31

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
   unsigned long htime;
};

static unsigned char CRCL[256];

static unsigned char CRCH[256];

static char peertopeer;

static char show;

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

/*
PROCEDURE ["C"] / select(n: INTEGER; readfds: ADDRESS; writefds: ADDRESS;
                         exceptfds: ADDRESS; timeout: ADDRESS) : INTEGER;
*/

static void Err(const char text[], unsigned long text_len)
{
   InOut_WriteString("udphub: ", 9ul);
   InOut_WriteString(text, text_len);
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

/*
PROCEDURE Inittable(fn:ARRAY OF CHAR);
VAR fd:INTEGER;
BEGIN
  fd:=OpenRead(fn);
  IF fd<0 THEN Err("-f: file not found") END;  

    

END Inittable;
*/

static void parms(void)
{
   char err;
   char h[1024];
   unsigned long i;
   unsigned long fromdigiport;
   err = 0;
   for (;;) {
      Lib_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='l') {
            Lib_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &i)) Err("-l minutes", 11ul);
            lifetime = i*60UL;
         }
         else if (h[1U]=='L') {
            Lib_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &i)) Err("-L minutes", 11ul);
            alllifetime = i*60UL;
         }
         else if (h[1U]=='a') peertopeer = 1;
         else if (h[1U]=='m') {
            /*
                  ELSIF h[1]="f" THEN                                       (* init filename *)
                    NextArg(h);
                    Inittable(h);
            */
            Lib_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &maxentries)) Err("-m number", 10ul);
         }
         else if (h[1U]=='p') {
            Lib_NextArg(h, 1024ul);
            if (!GetNum(h, 1024ul, &touserport)) Err("-p portnumber", 14ul);
         }
         else if (h[1U]=='u') {
            Lib_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &digiip, &todigiport, &fromdigiport,
                &digisock, &checkdigiip)<0L) {
               Err("cannot open digi udp socket", 28ul);
            }
         }
         else if (h[1U]=='v') show = 1;
         else {
            if (h[1U]=='h') {
               osi_WrLn();
               osi_WrStrLn(" -a                                route user-to-\
host and user-to-user", 71ul);
               osi_WrStrLn(" -L <time>                         minutes route \
to all ssid\'s (default 10 min)", 80ul);
               osi_WrStrLn("                                   0 no all ssid \
routing", 57ul);
               osi_WrStrLn(" -l <time>                         minutes lifeti\
me (default 1 week)", 69ul);
               osi_WrStrLn(" -m <maxentries>                   else delete ol\
d entries (default 1000)", 74ul);
               osi_WrStrLn(" -p <userport>                     udp port for u\
sers", 54ul);
               osi_WrStrLn(" -u <x.x.x.x:destport:listenport>  axudp to digi \
/listenport check ip", 70ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrLn();
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else {
         /*
               h[0]:=0C;
         */
         err = 1;
      }
      if (err) break;
   }
   if (err) {
      InOut_WriteString(">", 2ul);
      InOut_WriteString(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end parms() */


static void showpip(unsigned long ip, unsigned long port)
{
   InOut_WriteInt((long)(ip/16777216UL), 1UL);
   InOut_WriteString(".", 2ul);
   InOut_WriteInt((long)(ip/65536UL&255UL), 1UL);
   InOut_WriteString(".", 2ul);
   InOut_WriteInt((long)(ip/256UL&255UL), 1UL);
   InOut_WriteString(".", 2ul);
   InOut_WriteInt((long)(ip&255UL), 1UL);
   InOut_WriteString(":", 2ul);
   InOut_WriteInt((long)port, 1UL);
} /* end showpip() */


static void showcall(const char b[], unsigned long b_len,
                unsigned long start)
{
   char h[16];
   unsigned long l;
   l = 0UL;
   if (Call2Str(b, b_len, h, 16ul, start, &l)) {
      h[l] = 0;
      InOut_WriteString(h, 16ul);
   }
} /* end showcall() */


static void showframe(const char s[], unsigned long s_len, const char b[],
                unsigned long b_len)
{
   InOut_WriteString(s, s_len);
   showcall(b, b_len, 7UL);
   InOut_WriteString(">", 2ul);
   showcall(b, b_len, 0UL);
   osi_WrLn();
} /* end showframe() */


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


static pUSER Realloc(void)
{
   unsigned long cnt;
   pUSER new0;
   pUSER last;
   pUSER u;
   cnt = 0UL;
   last = 0;
   new0 = 0;
   u = users;
   for (;;) {
      if (u==0) break;
      if (u->htime+lifetime<systime || cnt>=maxentries) {
         /* old oder too much entries */
         if (last==0) users = 0;
         else last->next = 0;
         do {
            /* free rest of chain */
            last = u;
            u = u->next;
            if (new0==0) new0 = last;
            else {
               Storage_DEALLOCATE((X2C_ADDRESS *) &last,
                sizeof(struct USER));
            }
         } while (u);
         break;
      }
      last = u;
      u = u->next;
      ++cnt;
   }
   if (new0==0) Storage_ALLOCATE((X2C_ADDRESS *) &new0, sizeof(struct USER));
   if (show) {
      InOut_WriteString(" Table entries=", 16ul);
      InOut_WriteInt((long)cnt, 1UL);
      osi_WrLn();
   }
   return new0;
} /* end Realloc() */


static void AddIp(unsigned long ip, unsigned long dp, const char buf[],
                unsigned long buf_len)
{
   pUSER last;
   pUSER u;
   unsigned long i;
   /* for fast find, rechain to first position*/
   struct USER * anonym;
   struct USER * anonym0;
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
            anonym->uip = ip; /* store if ip changed */
            anonym->dport = dp;
         }
         if (show) {
            InOut_WriteString("Found User ", 12ul);
            showcall(buf, buf_len, 7UL);
            InOut_WriteString(" IP:", 5ul);
            showpip(u->uip, dp);
            osi_WrLn();
         }
         return;
      }
      last = u;
      u = u->next;
   }
   if (u==0) {
      if (show) {
         InOut_WriteString("Add User ", 10ul);
         showcall(buf, buf_len, 7UL);
         InOut_WriteString(" IP:", 5ul);
         showpip(ip, dp);
         osi_WrLn();
      }
      u = Realloc();
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
         }
         u->next = users;
         users = u;
      }
   }
} /* end AddIp() */


static char FindIp(unsigned long * toip, unsigned long * toport,
                const char buf[], unsigned long buf_len,
                unsigned long callpos)
{
   pUSER u;
   u = users;
   while (u) {
      if (cmpcall(u->call, buf, buf_len, callpos, 1)) {
         *toip = u->uip;
         *toport = u->dport;
         return 1;
      }
      u = u->next;
   }
   return 0;
} /* end FindIp() */


static char FindNextSsid(pUSER * u, unsigned long * toip,
                unsigned long * toport, const char buf[],
                unsigned long buf_len, unsigned long callpos)
{
   if (*u==0) *u = users;
   else *u = (*u)->next;
   while (*u) {
      if ((*u)->htime+alllifetime>=systime && cmpcall((*u)->call, buf,
                buf_len, callpos, 0)) {
         *toip = (*u)->uip;
         *toport = (*u)->dport;
         return 1;
      }
      *u = (*u)->next;
   }
   return 0;
} /* end FindNextSsid() */


static char FindsameIP(pUSER un, unsigned long toip, unsigned long toport,
                const char buf[], unsigned long buf_len,
                unsigned long callpos)
{
   pUSER u;
   u = users;
   while (u!=un) {
      if (((u->htime+alllifetime>=systime && cmpcall(u->call, buf, buf_len,
                callpos, 0)) && toip==u->uip) && toport==u->dport) return 1;
      u = u->next;
   }
   return 0;
} /* end FindsameIP() */


static char sendtouser(char ubuf0[], unsigned long ubuf_len, long blen0)
{
   unsigned long toip;
   unsigned long userdport0;
   pUSER u;
   long ci;
   long res0;
   char ok0;
   ok0 = 0;
   ci = 0L;
   do {
      /*WrInt(ci, 5); WrInt(ORD(ubuf[ci]), 5);
                WrInt(ORD(ubuf[ci+(CALLLEN-1)]), 5); WrStrLn("<<<"); */
      if (ci==0L || (unsigned char)ubuf0[ci+6L]<(unsigned char)'\200') {
         /* dest or via call with no h-bit */
         if (FindIp(&toip, &userdport0, ubuf0, ubuf_len, (unsigned long)ci)) {
            /* user and ssid fits */
            res0 = udpsend(usersock, ubuf0, blen0, userdport0, toip);
            ok0 = 1;
         }
         else if (alllifetime>0UL) {
            /* look for fitting user with any ssid */
            u = 0;
            while (FindNextSsid(&u, &toip, &userdport0, ubuf0, ubuf_len,
                (unsigned long)ci)) {
               if (!(ok0 && FindsameIP(u, toip, userdport0, ubuf0, ubuf_len,
                (unsigned long)ci))) {
                  /* have not sent to same ip:port before */
                  res0 = udpsend(usersock, ubuf0, blen0, userdport0, toip);
                  ok0 = 1;
               }
            }
         }
      }
      if (ci==0L) ci = 14L;
      else ci += 7L;
   } while (!((ci>63L || ci+7L>=blen0) || ((unsigned long)(unsigned char)ubuf0[ci-1L]&1)));
   /* address field end */
   return ok0;
} /* end sendtouser() */

static char ubuf[338];

static long blen;

static long res;

static unsigned long fromip;

static unsigned long userdport;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(RAWCALL)!=7) X2C_ASSERT(0);
   TimeConv_BEGIN();
   Lib_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   Storage_BEGIN();
   Gencrctab();
   show = 0;
   peertopeer = 0;
   touserport = 0UL;
   digisock = -1L;
   maxentries = 1000UL;
   lifetime = 604800UL;
   alllifetime = 600UL;
   users = 0;
   parms();
   if (!peertopeer && digisock<0L) Err("need -u parameter", 18ul);
   usersock = openudp();
   if ((touserport==0UL || usersock<0L) || bindudp(usersock, touserport)<0L) {
      Err("cannot bind userport (-p userport)", 35ul);
   }
   systime = TimeConv_time();
   for (;;) {
      fdclr();
      if (digisock>=0L) fdsetr((unsigned long)digisock);
      fdsetr((unsigned long)usersock);
      if (selectrw(10UL, 0UL)>0L) {
         if (digisock>=0L && issetr((unsigned long)digisock)) {
            /* data from digi */
            fromip = digiip;
            blen = getudp(digisock, ubuf, 338ul, &fromip, &userdport,
                checkdigiip);
            if (blen>=17L) {
               if (show) showframe("from digi ", 11ul, ubuf, 338ul);
               if (!sendtouser(ubuf, 338ul, blen) && show) {
                  osi_WrStrLn(" digi out user not found", 25ul);
               }
            }
         }
         if (issetr((unsigned long)usersock)) {
            /* data from user */
            fromip = 0UL;
            blen = getudp(usersock, ubuf, 338ul, &fromip, &userdport, 0);
            if (blen>=17L) {
               if (show) {
                  showpip(fromip, touserport);
                  showframe(" from user ", 12ul, ubuf, 338ul);
               }
               AddIp(fromip, userdport, ubuf, 338ul);
               if (digisock>=0L) {
                  res = udpsend(digisock, ubuf, blen, todigiport, digiip);
               }
               if ((peertopeer && !sendtouser(ubuf, 338ul, blen)) && show) {
                  osi_WrStrLn(" peer-to-peer no user found", 28ul);
               }
            }
         }
      }
      else systime = TimeConv_time();
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
