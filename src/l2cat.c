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
#define l2cat_C_
#ifndef Execlogin_H_
#include "Execlogin.h"
#endif
#include <unistd.h>
#include <signal.h>
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef cleanup_H_
#include "cleanup.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef l2_H_
#include "l2.h"
#endif
#ifndef frameio_H_
#include "frameio.h"
#endif
#ifndef deflate_H_
#include "deflate.h"
#endif

/* layer2 to std in/out or start a task*/
/*
FROM mlib IMPORT tcsetattr, tcgetattr, tcflag_t, 
                 TCSAFLUSH, TIOCMGET, TIOCMBIS, TIOCMBIC, TCSANOW, ISIG,
                 CS8, CLOCAL, CREAD, CRTSCTS, B1200, B2400, 
                 B4800, B9600, B19200, B38400, B57600, B115200, B230400, 
                 B460800, TIOCM_CD, TIOCM_DTR, TIOCSCTTY;
*/
#define l2cat_MAXCMDWORDS 64

#define l2cat_NL "\012"

#define l2cat_PPPFLAG 0x7E 

#define l2cat_PPPESC 0x7D 

#define l2cat_LF "\012"

#define l2cat_LOOPTIME 10000

#define l2cat_MINRECONTIME 2000000

#define l2cat_MAXRECONTIME 120000000

#define l2cat_RCTIMEBASE 1000

#define l2cat_FLUSHDELAY 3
/* LOOPTIMES wait sending out rest of buffer */

typedef char IOBUF[256];

struct JUNKBUF;


struct JUNKBUF {
   IOBUF buf;
   int32_t len;
   int32_t time0;
};

struct DEFLATBUF;


struct DEFLATBUF {
   char outbuf[20001]; /* 20000 */
   int32_t outlen;
   char xoutbuf[4096];
   char pppbuf[4096];
   int32_t ppplen;
   int32_t xoutlen;
   int32_t inp0;
   int32_t xinp;
   char pppstate;
   struct deflate_CONTEXT deflatcontext;
   struct deflate_XCONTEXT expandcontext;
   char done;
   char pppframing;
};

struct TASK;

typedef struct TASK * pTASK;


struct TASK {
   pTASK next;
   IOBUF inbuf;
   IOBUF outbuf;
   int32_t inlen;
   int32_t outlen;
   struct JUNKBUF junkbuf;
   struct l2_GETADRESS l2addr;
   l2_pLINK link0;
   uint16_t events;
   int32_t infd;
   int32_t outfd;
   uint8_t state;
   uint32_t crlfmode;
   uint32_t detectdone;
   uint32_t inignor;
   struct DEFLATBUF * pcdeflat;
};

static char terminate;

static char keepconnected;

static char verb;

static char verb2;

static char usedeflat;

static char pppmode;

static char autodet;

static pTASK tasks;

static uint32_t sockc;

static uint32_t convlfcr;

static uint32_t convtopipe;

static uint32_t montyp;

static uint32_t axudpchk;

static uint32_t recontime;

static uint32_t minrecontime;

static uint32_t maxrecontime;

static int32_t timesum;

static int32_t pppdiscard;

static char mycall[113];

static char cmdline[4096];

static char pipename[4096];

static char connectto0[256]; /* if active connect to this server */

static struct l2_PARMS0 parms;

struct _0;


struct _0 {
   int32_t port;
   int32_t parmnum;
   int32_t val;
};

static struct _0 numpar[256];

static l2_pLINK pmon;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */

#define l2cat_QUOT "\""

#define l2cat_SEP " "


static void NextArgs(char s[], uint32_t s_len)
{
   char h[65536];
   int32_t l;
   s[0UL] = 0;
   l = 0L;
   do {
      osi_NextArg(h, 65536ul);
      if (l==0L) {
         if (h[0U]=='\"') aprsstr_Delstr(h, 65536ul, 0UL, 1UL);
         else {
            aprsstr_Append(s, s_len, h, 65536ul);
            return;
         }
      }
      l = (int32_t)aprsstr_Length(h, 65536ul);
      if (l==0L) return;
      if (h[l-1L]=='\"') {
         aprsstr_Delstr(h, 65536ul, (uint32_t)(l-1L), 1UL);
         l = 0L;
      }
      aprsstr_Append(s, s_len, " ", 2ul);
      aprsstr_Append(s, s_len, h, 65536ul);
   } while (l);
} /* end NextArgs() */


static void Whex(char hd[], uint32_t hd_len, const char bu[],
                uint32_t bu_len, int32_t len)
{
   int32_t i0;
   uint32_t b;
   int32_t tmp;
   char tmp0;
   X2C_PCOPY((void **)&hd,hd_len);
   osi_Werr(hd, hd_len);
   tmp = len-1L;
   i0 = 0L;
   if (i0<=tmp) for (;; i0++) {
      b = (uint32_t)(uint8_t)bu[i0]/16UL;
      if (b>=10UL) {
         osi_Werr((char *)(tmp0 = (char)(b+55UL),&tmp0), 1u/1u);
      }
      else osi_Werr((char *)(tmp0 = (char)(b+48UL),&tmp0), 1u/1u);
      b = (uint32_t)(uint8_t)bu[i0]&15UL;
      if (b>=10UL) {
         osi_Werr((char *)(tmp0 = (char)(b+55UL),&tmp0), 1u/1u);
      }
      else osi_Werr((char *)(tmp0 = (char)(b+48UL),&tmp0), 1u/1u);
      if ((i0&31L)==31L) osi_Werr("\012", 2ul);
      else osi_Werr(" ", 2ul);
      if (i0==tmp) break;
   } /* end for */
   osi_Werr("\012", 2ul);
   X2C_PFREE(hd);
} /* end Whex() */


static void CallStr(char c[], uint32_t c_len, uint32_t n,
                char withssid, char s[], uint32_t s_len)
{
   uint32_t p;
   uint32_t i0;
   X2C_PCOPY((void **)&c,c_len);
   n = n*7UL;
   p = 0UL;
   for (i0 = 0UL; i0<=5UL; i0++) {
      if (c[i0+n]!=' ') {
         s[p] = c[i0+n];
         ++p;
      }
   } /* end for */
   s[p] = 0;
   if (withssid) {
      i0 = (uint32_t)(uint8_t)c[n+6UL]&15UL;
      if (i0) {
         s[p] = '-';
         ++p;
         if (i0>=10UL) {
            s[p] = (char)(i0/10UL+48UL);
            ++p;
         }
         s[p] = (char)(i0%10UL+48UL);
         ++p;
      }
      s[p] = 0;
   }
   X2C_PFREE(c);
} /* end CallStr() */


static void WerrCall(pTASK task)
{
   char h[256];
   if (verb && task) {
      CallStr(task->l2addr.adress, 72ul, 0UL, 1, h, 256ul);
      osi_Werr(h, 256ul);
   }
} /* end WerrCall() */


static void GetCall(const char from[], uint32_t from_len,
                uint32_t * p, l2_pSTRING to)
{
   uint16_t len;
   uint16_t i0;
   len = (uint16_t)aprsstr_Length(from, from_len);
   while (*p<(uint32_t)len && (uint8_t)from[*p]<=' ') ++*p;
   for (i0 = 0U; i0<=5U; i0++) {
      if ((*p<(uint32_t)len && from[*p]!='-') && from[*p]!=' ') {
         to[i0] = X2C_CAP(from[*p]);
         ++*p;
      }
      else to[i0] = ' ';
   } /* end for */
   i0 = 0U;
   if (from[*p]=='-') {
      ++*p;
      if ((uint8_t)from[*p]>='0' && (uint8_t)from[*p]<='9') {
         i0 = (uint16_t)((uint32_t)(uint8_t)from[*p]-48UL);
         ++*p;
      }
      if ((uint8_t)from[*p]>='0' && (uint8_t)from[*p]<='9') {
         i0 = (uint16_t)(((uint32_t)(i0*10U)+(uint32_t)(uint8_t)
                from[*p])-48UL);
         ++*p;
      }
   }
   to[6U] = (char)((i0&15U)+48U);
} /* end GetCall() */

static void Event(char * *, l2_pLINK, uint8_t);


static void Event(char * * atask, l2_pLINK link0, uint8_t event)
{
   struct l2_GETADRESS gadr;
   pTASK ntask;
   struct l2_CONNECT connect;
   pTASK ptask;
   if (event==l2_eCONNREQ) {
      osic_alloc((char * *) &ntask, sizeof(struct TASK));
      if (ntask==0) return;
      memset((char *)ntask,(char)0,sizeof(struct TASK));
      ntask->infd = -1L;
      ntask->outfd = -1L;
      ntask->link0 = link0;
      ntask->state = 1U;
      ntask->crlfmode = convlfcr;
      ntask->detectdone = (uint32_t)autodet;
      l2_GetAdress0(link0, &ntask->l2addr);
      if (ntask->l2addr.my>0U) {
         /* not via digiall */
         memset((char *) &connect,(char)0,sizeof(struct l2_CONNECT));
         connect.port = ntask->l2addr.port;
         connect.baud = 1U;
         connect.cpid = 240U;
         connect.l2adr = (l2_pSTRING)ntask->l2addr.adress;
         connect.typ = l2_cCONNAK;
         ntask->link0 = l2_Connect0((char *)ntask, &connect);
         ntask->next = tasks;
         tasks = ntask;
         if (verb) {
            WerrCall(ntask);
            osi_Werr(" connreq\012", 10ul);
         }
      }
      else {
         osic_free((char * *) &ntask, sizeof(struct TASK));
         if (verb) osi_Werr(" via\012", 6ul);
      }
   }
   else if (event==l2_eFLEXDATA) l2_GetAdress0(link0, &gadr);
   else if (event==l2_eCONNECTED) {
      /*WrStr(gadr.adress); */
      /*WrInt(dbuf^.len, 10); */
      /*WrStrLn(" flexdata"); */
      if (verb) {
         WerrCall((pTASK)*atask);
         osi_Werr(" connected!\012", 13ul);
      }
      if (*atask) {
         ptask = (pTASK)*atask;
         ptask->state = 1U;
      }
   }
   else if (*atask) {
      /*  ELSIF event=eDISCONNECTED THEN */
      /*WrInt(ORD(event), 1);WrStrLn(":host got event"); */
      ptask = (pTASK)*atask;
      ptask->events |= (1U<<event);
   }
} /* end Event() */


static void GetWord(char s[], uint32_t s_len, char w[],
                uint32_t w_len)
{
   uint32_t j;
   uint32_t i0;
   char e;
   i0 = 0UL;
   j = 0UL;
   e = ' ';
   if (s[0UL]=='\"') {
      e = '\"';
      s[0UL] = ' ';
      i0 = 1UL;
   }
   for (;;) {
      if (i0>s_len-1 || s[i0]==0) break;
      if (s[i0]==e) {
         s[i0] = ' ';
         break;
      }
      if (j<w_len-1) {
         w[j] = s[i0];
         ++j;
      }
      s[i0] = ' ';
      ++i0;
   }
   w[j] = 0;
} /* end GetWord() */


static void taskparms(char s[], uint32_t s_len, char argbuf[],
                uint32_t argbuf_len, char * argwords[],
                uint32_t argwords_len)
{
   uint32_t i0;
   uint32_t argc;
   uint32_t argp;
   char h[1024];
   X2C_PCOPY((void **)&s,s_len);
   memset((char *)argwords,(char)0,argwords_len*sizeof(char *));
   argc = 0UL;
   argp = 0UL;
   for (;;) {
      while (s[0UL]==' ') aprsstr_Delstr(s, s_len, 0UL, 1UL);
      GetWord(s, s_len, h, 1024ul);
      /*WrStrLn(h); */
      h[1023U] = 0;
      if (h[0U]==0) break;
      argwords[argc] = (char *) &argbuf[argp];
      i0 = 0UL;
      for (;;) {
         if (argp>=argbuf_len-1) {
            /*argbuffer full*/
            argbuf[argbuf_len-1] = 0;
            osi_Werr("ArgBuf full!\012", 14ul);
            break;
         }
         argbuf[argp] = h[i0];
         ++argp;
         if (h[i0]==0) break;
         ++i0;
      }
      ++argc;
      if (argc>=argwords_len-1) break;
   }
   argwords[argc] = 0;
   X2C_PFREE(s);
} /* end taskparms() */

#define l2cat_M "%"


static void Makros(char c[], uint32_t c_len, char a[],
                uint32_t a_len, char s[], uint32_t s_len)
{
   uint32_t i0;
   char h[16];
   char m;
   X2C_PCOPY((void **)&c,c_len);
   X2C_PCOPY((void **)&a,a_len);
   s[0UL] = 0;
   i0 = 0UL;
   m = 0;
   while (i0<=c_len-1 && c[i0]) {
      if (m) {
         if (c[i0]=='%') aprsstr_Append(s, s_len, "%", 2ul);
         else if (c[i0]=='m') {
            CallStr(a, a_len, 1UL, 1, h, 16ul);
            aprsstr_Append(s, s_len, h, 16ul);
         }
         else if (c[i0]=='u') {
            CallStr(a, a_len, 0UL, 1, h, 16ul);
            aprsstr_Append(s, s_len, h, 16ul);
         }
         else if (c[i0]=='U') {
            CallStr(a, a_len, 0UL, 0, h, 16ul);
            aprsstr_Append(s, s_len, h, 16ul);
         }
         else {
            aprsstr_Append(s, s_len, "%", 2ul);
            aprsstr_Append(s, s_len, (char *) &c[i0], 1u/1u);
         }
         m = 0;
      }
      else if (c[i0]=='%') m = 1;
      else aprsstr_Append(s, s_len, (char *) &c[i0], 1u/1u);
      ++i0;
   }
   X2C_PFREE(c);
   X2C_PFREE(a);
} /* end Makros() */


static void Login(int32_t * fd, char l2adr[], uint32_t l2adr_len)
{
   struct Execlogin_tEXEC cmdvec;
   char argstr[8192];
   char argbuf[8192];
   char * argwords[65];
   X2C_PCOPY((void **)&l2adr,l2adr_len);
   /*args:="/bin/login login"; */
   Makros(cmdline, 4096ul, l2adr, l2adr_len, argstr, 8192ul);
   taskparms(argstr, 8192ul, argbuf, 8192ul, argwords, 65ul);
   /*p:=argwords[0];WrLn;WrInt(CAST(CARDINAL, argwords[0]), 15);
                WrInt(CAST(CARDINAL, p), 15);WrLn; */
   /*WrStrLn(p^);p:=argwords[1];WrStrLn(p^);WrStrLn(argbuf); */
   *fd = -1L;
   cmdvec.cmdfn = (char *)argwords[0U];
   cmdvec.args = (X2C_pCHAR *) &argwords[1U];
   if (verb) {
      osi_Werr("start: ", 8ul);
      osi_Werr(cmdline, 4096ul);
      osi_Werr("\012", 2ul);
   }
   if (cmdvec.cmdfn) *fd = Execlogin_StartLogin(&cmdvec);
   /*
     IF fd>=0 THEN
       ret:=tcgetattr(fd, tio);
       tio.c_lflag:=CAST(tcflag_t, CAST(BITSET, tio.c_lflag) + CAST(BITSET,
                ISIG));
       ret:=tcsetattr (fd, TCSAFLUSH, tio);
     END;
   */
   if (*fd>=2L && *fd<=255L) {
      if (verb) {
         osi_Werr("login fd=", 10ul);
         aprsstr_IntToStr(*fd, 1UL, argbuf, 8192ul);
         osi_Werr(argbuf, 8192ul);
         osi_Werr("\012", 2ul);
      }
   }
   else {
      if (verb) osi_Werr("login fd error\012", 16ul);
      *fd = -1L;
   }
   X2C_PFREE(l2adr);
} /* end Login() */


static char GetNum(const char h[], uint32_t h_len, char eot,
                 uint32_t * p, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static void Parms(void)
{
   char err;
   char hh[1024];
   char h[1024];
   uint32_t l2parcnt;
   uint32_t p;
   uint32_t k;
   uint32_t j;
   uint32_t i0;
   struct frameio_UDPSOCK * anonym;
   err = 0;
   sockc = 0UL;
   for (i0 = 0UL; i0<=255UL; i0++) {
      numpar[i0].port = 0L;
   } /* end for */
   l2parcnt = 0UL;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if (h[0U]=='-' && h[2U]==0) {
         if (h[1U]=='C') {
            /* -C ax25 destination */
            NextArgs(h, 1024ul);
            i0 = 0UL;
            j = 0UL;
            while (j<63UL && i0<aprsstr_Length(h, 1024ul)) {
               GetCall(h, 1024ul, &i0, (l2_pSTRING) &connectto0[j]);
               j += 7UL;
               if (j==7UL) j = 14UL;
            }
            connectto0[j] = 0;
         }
         else if (h[1U]=='c') {
            /* -c comand line */
            NextArgs(cmdline, 4096ul);
         }
         else if (h[1U]=='p') {
            /*        keepconnected:=TRUE; */
            /* -p pipe */
            osi_NextArg(pipename, 4096ul);
         }
         else if (h[1U]=='i') {
            /* -i mycall */
            NextArgs(h, 1024ul);
            j = 7UL; /* first call is digi call, insert later */
            i0 = 0UL;
            do {
               GetCall(h, 1024ul, &i0, (l2_pSTRING) &mycall[j]);
               j += 7UL;
            } while (!(j>=112UL || h[i0]!=' '));
            if (j<=112UL) mycall[j] = 0;
            for (i0 = 0UL; i0<=6UL; i0++) {
               mycall[i0] = mycall[i0+7UL]; /* first mycall is digi call*/
            } /* end for */
         }
         else if (h[1U]=='j') {
            /* -j <num> convert cr lf to pipe */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &convtopipe) || convtopipe>6UL) {
               err = 1;
               osi_WrStrLn("-j <0..6>", 10ul);
            }
         }
         else if (h[1U]=='m') {
            /* -j <num> convert cr lf to pipe */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &montyp)) {
               err = 1;
               osi_WrStrLn("-m <0..1>", 10ul);
            }
         }
         else if (h[1U]=='w') {
            /* -w <num> check axudp dupes *10ms */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &axudpchk) || axudpchk>31UL) {
               err = 1;
               osi_WrStrLn("-w <0..31>", 11ul);
            }
         }
         else if (h[1U]=='D') {
            /* -D <n> discard frame len */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &j)) {
               err = 1;
               osi_WrStrLn("-D <0..1500>", 13ul);
            }
            pppdiscard = (int32_t)j;
         }
         else if (h[1U]=='r') {
            /* -j <num> convert cr lf to pipe */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &j) || j>4000000UL) {
               err = 1;
               osi_WrStrLn("-r <ms> (max 4000000)", 22ul);
            }
            else minrecontime = j*1000UL;
            keepconnected = 1;
         }
         else if (h[1U]=='R') {
            /* -r <ms> min reconn time */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &j) || j>4000000UL) {
               err = 1;
               osi_WrStrLn("-R <ms> (max 4000000)", 22ul);
            }
            else maxrecontime = j*1000UL;
            keepconnected = 1;
         }
         else if (h[1U]=='n') {
            /* -R <ms> max reconn time */
            osi_NextArg(h, 1024ul);
            if (h[0U]=='a') p = 16UL;
            else {
               i0 = 0UL;
               if (!GetNum(h, 1024ul, 0, &i0, &p) || p>15UL) {
                  err = 1;
                  osi_WrStrLn("-n <port> <parameter> <value>", 30ul);
               }
            }
            osi_NextArg(h, 1024ul);
            switch ((unsigned)h[0U]) {
            case 't': /* txwait */
               j = 17UL;
               break;
            case 'd': /* dwait */
               j = 20UL;
               break;
            case 'o': /* maxframes */
               j = 23UL;
               break;
            case 'f': /* t1 */
               j = 24UL;
               break;
            case 'F': /* t3 */
               j = 25UL;
               break;
            case 'I': /* ipoll */
               j = 26UL;
               break;
            case 'n': /* retr */
               j = 27UL;
               break;
            case 'h': /* halfduplex */
               j = 24UL;
               break;
            default:;
               i0 = 0UL;
               if (!GetNum(h, 1024ul, 0, &i0, &j) || j>255UL) {
                  err = 1;
                  osi_WrStrLn("-n <port> <parameter> <value>", 30ul);
               }
               break;
            } /* end switch */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, 0, &i0, &k)) {
               err = 1;
               osi_WrStrLn("-n <port> <parameter> <value>", 30ul);
            }
            if (!err) {
               if (l2parcnt>255UL) Error("parameter table full", 21ul);
               numpar[l2parcnt].port = (int32_t)p;
               numpar[l2parcnt].parmnum = (int32_t)j;
               numpar[l2parcnt].val = (int32_t)k;
               ++l2parcnt;
            }
         }
         else if (h[1U]=='U') {
            osi_NextArg(h, 1024ul);
            if (sockc>14UL) Error("too many ports", 15ul);
            { /* with */
               struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[sockc];
               if (aprsstr_GetIp2(h, 1024ul, &anonym->ipnum, &anonym->toport,
                 &anonym->fromport, &anonym->checkip)<0L) {
                  Error("-U wrong ip:port:port number", 29ul);
               }
               anonym->fd = openudp();
               if (anonym->fd<0L || bindudp(anonym->fd,
                anonym->fromport)<0L) {
                  strncpy(h,"-U cannot open udp port ",1024u);
                  aprsstr_IntToStr((int32_t)anonym->fromport, 0UL, hh,
                1024ul);
                  aprsstr_Append(h, 1024ul, hh, 1024ul);
                  Error(h, 1024ul);
               }
            }
            ++sockc;
         }
         else if (h[1U]=='a') autodet = 1;
         else if (h[1U]=='P') {
            pppmode = 1;
            usedeflat = 1;
         }
         else if (h[1U]=='d') usedeflat = 1;
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h[1U]=='l') convlfcr = 1UL;
         else if (h[1U]=='L') convlfcr = 2UL;
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn(" -a                                autodetect def\
late + ppp on first bytes", 75ul);
               osi_WrStrLn("                                     ppp + deflat\
e:\"!p\",deflate;\"!d\"", 69ul);
               osi_WrStrLn("                                     LF to CR:\"!\
l\", strip LF:\"!L\", transparent:\"!8\"", 84ul);
               osi_WrStrLn(" -C <destination>                  slave connect \
destination -C \"OE0AAA-12 OE5XBL\"", 83ul);
               osi_WrStrLn("                                     if no \"-C\"\
 run as server waiting for connect", 82ul);
               osi_WrStrLn(" -c <cmdline>                      execute comman\
d on connect -c \"/bin/bash bash %m %u\"", 88ul);
               osi_WrStrLn("                                     %m=mycall %u\
=usercall %U=same no SSID %%=%", 80ul);
               osi_WrStrLn("                                     if no \"-c\"\
 data path is stdin/stdout", 74ul);
               osi_WrStrLn(" -D <bytes>                        discard longer\
 ppp frames if pr tx buffer full", 82ul);
               osi_WrStrLn(" -d                                deflate on",
                46ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -i <mycall> <mycall> ...          connectable ca\
lls -i \"OE0AAA OE0AAA-15\"", 75ul);
               osi_WrStrLn("                                     first mycall\
 is digi and used to connect out", 82ul);
               osi_WrStrLn(" -j <1..6>                         1:del LF, 2:de\
l LF+CR>LF, 3:CR>LF, 4:del CR", 79ul);
               osi_WrStrLn("                                     5:del CR+LF>\
CR, 6:LF>CR in stream from pr", 79ul);
               osi_WrStrLn(" -l                                convert LF to \
CR in stream to pr", 68ul);
               osi_WrStrLn(" -L                                strip LF in st\
ream to pr", 60ul);
               osi_WrStrLn(" -m <0..2>                         ax25 Monitor t\
o stderr (2 with info)", 72ul);
               osi_WrStrLn(" -n <port> <parm> <value>          layer 2 parms,\
 \"-n a\" set on all ports", 74ul);
               osi_WrStrLn("                                     parm t:txwai\
t d:dwait o:maxfr f:t1 F:t3", 77ul);
               osi_WrStrLn("                                     parm I:ipoll\
 n:retr h:halfdup", 67ul);
               osi_WrStrLn(" -P                                ppp + deflate \
on", 52ul);
               osi_WrStrLn(" -p <filename>                     data to/from t\
his device/pipe -p /dev/ttyS0", 79ul);
               osi_WrStrLn(" -R <ms>                           enable slave \\
"keep connected\" with max reconnet delay", 89ul);
               osi_WrStr("                                     (max 4000000ms\
) (", 55ul);
               osic_WrINT32(maxrecontime/1000UL, 1UL);
               osi_WrStrLn(")", 2ul);
               osi_WrStrLn(" -r <ms>                           enable slave \\
"keep connected\" with min reconnet delay", 89ul);
               osi_WrStr("                                     and doubles de\
lay each conn try till \"-R\"  (", 82ul);
               osic_WrINT32(minrecontime/1000UL, 1UL);
               osi_WrStrLn(")", 2ul);
               osi_WrStrLn(" -U <x.x.x.x:destport:listenport>  axudp (autodet\
ect axudp2)", 61ul);
               osi_WrStrLn("                                     listenport=0\
 uses a free port", 67ul);
               osi_WrStrLn("                                     repeat -U fo\
r more ports", 62ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osi_WrStrLn(" -V                                more verbous",
                 48ul);
               osi_WrStrLn(" -w <n>                            check dupe axu\
dp frames last n frames or n*10ms", 83ul);
               osic_WrLn();
               X2C_ABORT();
            }
            if (h[1U]=='-') Error("- - ?", 6ul);
            if (h[1U]==0) Error("-  ?", 5ul);
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (connectto0[0U]) {
      /* insert mycall as it is valid after whole cmd line */
      if (mycall[7U]==0) Error("need -i <call> for -C ", 23ul);
      X2C_MOVE((char *) &mycall[7U],(char *) &connectto0[7U],7UL);
   }
   else keepconnected = 1;
   if (sockc==0UL) Error("need at least one -U ...", 25ul);
} /* end Parms() */


static void Connectto(void)
{
   pTASK ntask;
   struct l2_CONNECT connect;
   osic_alloc((char * *) &ntask, sizeof(struct TASK));
   if (ntask==0) return;
   memset((char *)ntask,(char)0,sizeof(struct TASK));
   ntask->infd = -1L;
   ntask->outfd = -1L;
   /*  GetAdress(link, ADR(ntask^.l2addr)); */
   memset((char *) &connect,(char)0,sizeof(struct l2_CONNECT));
   connect.port = 1U;
   connect.baud = 1U;
   connect.cpid = 240U;
   connect.l2adr = (l2_pSTRING)connectto0;
   connect.l3adr = 0;
   connect.typ = l2_cNORMAL;
   ntask->link0 = l2_Connect0((char *)ntask, &connect);
   ntask->state = 5U;
   ntask->next = tasks;
   ntask->crlfmode = convlfcr;
   tasks = ntask;
} /* end Connectto() */

static void killdisc(int32_t);


static void killdisc(int32_t signum)
{
   if (verb) osi_Werr("sigint\012", 8ul);
   /*  IF (tasks<>NIL) & (tasks^.state=2) THEN WrStrLn("disc!");
                Disconnect(tasks^.link, TRUE); */
   /*  ELSE HALT(signum) END; */
   if (terminate) X2C_ABORT();
   terminate = 1;
} /* end killdisc() */


static void initcompress(pTASK pt0)
{
   /*IntToStr(SIZE(pt^.pcdeflat^), 1, deb); Werr(deb);Werr("=mem"); */
   osic_alloc((char * *) &pt0->pcdeflat, sizeof(struct DEFLATBUF));
   if (pt0->pcdeflat==0) Error("alloc compres memory", 21ul);
   deflate_Initdeflate(&pt0->pcdeflat->deflatcontext);
   deflate_Initexpand(&pt0->pcdeflat->expandcontext);
   pt0->pcdeflat->outlen = 0L;
   pt0->pcdeflat->xoutlen = 0L;
   pt0->pcdeflat->inp0 = 0L;
   pt0->pcdeflat->xinp = 0L;
   pt0->pcdeflat->done = 0;
   if (pppmode) {
      /* ppp per comand line */
      pt0->pcdeflat->pppframing = 1;
      if (verb) osi_Werr("init ppp + compression\012", 24ul);
   }
   else if (verb) osi_Werr("init compression\012", 18ul);
   pt0->pcdeflat->pppstate = 0;
} /* end initcompress() */


static char pppcrc(char b[], uint32_t b_len, int32_t len)
{
   char crc2;
   char crc1;
   char ok0;
   /*ShowHex(b, len); */
   if (len<2L) return 0;
   len -= 2L;
   crc1 = b[len];
   crc2 = b[len+1L];
   aprsstr_AppCRC(b, b_len, len);
   ok0 = crc1==b[len] && crc2==b[len+1L];
   if (!ok0) osi_Werr(" crc-err \012", 11ul);
   return ok0;
} /* end pppcrc() */


static void compress(pTASK pt0)
{
   int32_t i0;
   struct TASK * anonym;
   int32_t tmp;
   { /* with */
      struct TASK * anonym = pt0;
      if (anonym->pcdeflat->pppframing) {
         for (;;) {
            if (anonym->pcdeflat->inp0>=anonym->inlen) {
               /* out of data */
               anonym->pcdeflat->inp0 = 0L;
               anonym->inlen = 0L;
               break;
            }
            /*IF verb2 THEN WrHex(ORD(inbuf[pcdeflat^.inp]), 3) END; */
            if (anonym->pcdeflat->pppstate==0 && anonym->inbuf[anonym->pcdeflat->inp0]
                =='~') {
               /* hunt mode */
               anonym->pcdeflat->pppstate = '\001'; /* in frame */
               anonym->pcdeflat->ppplen = 0L;
            }
            else if (anonym->pcdeflat->pppstate=='\001') {
               if (anonym->inbuf[anonym->pcdeflat->inp0]=='}') {
                  anonym->pcdeflat->pppstate = '\002';
                /* escaped char follows */
               }
               else {
                  if (anonym->inbuf[anonym->pcdeflat->inp0]=='~') {
                     /* frame complete */
                     if (anonym->pcdeflat->ppplen>0L) {
                        if (pppcrc(anonym->pcdeflat->pppbuf, 4096ul,
                anonym->pcdeflat->ppplen)) {
                           /* frame good */
                           /*IntToStr(pcdeflat^.ppplen, 7, deb);Werr(deb);
                Werr("=c "); */
                           if (anonym->pcdeflat->ppplen<=pppdiscard || l2_SendStr(anonym->link0,
                 0U, 0)) {
                              tmp = anonym->pcdeflat->ppplen-1L;
                              i0 = 0L;
                              if (i0<=tmp) for (;; i0++) {
                                 deflate_Deflatbyte(&anonym->pcdeflat->deflatcontext,
                 anonym->pcdeflat->pppbuf[i0], 0, anonym->pcdeflat->outbuf,
                20001ul, &anonym->pcdeflat->outlen);
                                 if (i0==tmp) break;
                              } /* end for */
                              deflate_Deflatbyte(&anonym->pcdeflat->deflatcontext,
                 0, '\002', anonym->pcdeflat->outbuf, 20001ul,
                &anonym->pcdeflat->outlen);
                           }
                           else if (verb) {
                              osi_Werr("ppp frame discarded\012", 21ul);
                           }
                        }
                        else osi_Werr("ppp frame crc error\012", 21ul);
                        /*IntToStr(pcdeflat^.outlen, 7, deb);Werr(deb);
                Werr("=co "); */
                        anonym->pcdeflat->ppplen = 0L;
                     }
                     ++anonym->pcdeflat->inp0;
                     break;
                  }
                  anonym->pcdeflat->pppbuf[anonym->pcdeflat->ppplen]
                = anonym->inbuf[anonym->pcdeflat->inp0];
                  if (anonym->pcdeflat->ppplen<4095L) {
                     ++anonym->pcdeflat->ppplen;
                  }
               }
            }
            else if (anonym->pcdeflat->pppstate=='\002') {
               /* stuffed byte */
               anonym->pcdeflat->pppbuf[anonym->pcdeflat->ppplen]
                = (char)((uint8_t)(uint8_t)
                anonym->inbuf[anonym->pcdeflat->inp0]^0x20U);
               if (anonym->pcdeflat->ppplen<4095L) {
                  ++anonym->pcdeflat->ppplen;
               }
               anonym->pcdeflat->pppstate = '\001';
            }
            ++anonym->pcdeflat->inp0;
         }
      }
      else {
         while (anonym->pcdeflat->inp0<anonym->inlen && anonym->pcdeflat->outlen<3601L)
                 {
            deflate_Deflatbyte(&anonym->pcdeflat->deflatcontext,
                anonym->inbuf[anonym->pcdeflat->inp0], 0,
                anonym->pcdeflat->outbuf, 20001ul,
                &anonym->pcdeflat->outlen);
            ++anonym->pcdeflat->inp0;
         }
         if (anonym->pcdeflat->inp0>=anonym->inlen) {
            /* inbuffer empty so send compressed block */
            deflate_Deflatbyte(&anonym->pcdeflat->deflatcontext, 0, '\002',
                anonym->pcdeflat->outbuf, 20001ul,
                &anonym->pcdeflat->outlen); /* flush */
            anonym->pcdeflat->inp0 = 0L;
            anonym->inlen = 0L;
         }
      }
   }
/*ShowHex(pcdeflat^.outbuf, pcdeflat^.outlen); WrStrLn(""); */
} /* end compress() */


static void expand(pTASK pt0)
{
   char done;
   char pb[3001];
   int32_t j;
   int32_t i0;
   struct TASK * anonym;
   /*ShowHex(outbuf, outlen); */
   int32_t tmp;
   { /* with */
      struct TASK * anonym = pt0;
      for (;;) {
         /*IF pcdeflat^.xoutlen>=SIZE(pcdeflat^.xoutbuf)
                -300 THEN Werr(" outfull ") END; */
         if ((anonym->pcdeflat->xinp>=anonym->outlen || anonym->pcdeflat->xoutlen>=3796L)
                 || anonym->pcdeflat->xoutlen<0L) break;
         /*WrHex(ORD(outbuf[pcdeflat^.xinp]), 3);INC(deb1); */
         deflate_Expandbyte(&anonym->pcdeflat->expandcontext,
                anonym->outbuf[anonym->pcdeflat->xinp],
                anonym->pcdeflat->xoutbuf, 4096ul,
                &anonym->pcdeflat->xoutlen, &done);
         /*IF pcdeflat^.xoutlen<0 THEN Werr(" expand error"+NL) END; */
         /*Werr("(");IntToStr(pcdeflat^.xoutlen, 1, deb);Werr(deb);Werr(")");
                 */
         ++anonym->pcdeflat->xinp;
         if (done && anonym->pcdeflat->pppframing) {
            /* rebuild ppp frame */
            /*IntToStr(deb1, 7, deb);Werr(deb);Werr("=xi ");deb1:=0; */
            /*IntToStr(pcdeflat^.xoutlen, 7, deb);Werr(deb);Werr("=x "); */
            /*WrStrLn("done");ShowHex(pcdeflat^.xoutbuf, pcdeflat^.xoutlen);
                */
            if (pppcrc(anonym->pcdeflat->xoutbuf, 4096ul,
                anonym->pcdeflat->xoutlen)) {
               /*WrStrLn("");WrStr("Y:");
                ShowHex(pcdeflat^.xoutbuf, pcdeflat^.xoutlen); WrStrLn("");
                */
               pb[0U] = '~';
               j = 1L;
               tmp = anonym->pcdeflat->xoutlen-1L;
               i0 = 0L;
               if (i0<=tmp) for (;; i0++) {
                  if ((anonym->pcdeflat->xoutbuf[i0]
                =='~' || anonym->pcdeflat->xoutbuf[i0]=='}') || (uint8_t)
                anonym->pcdeflat->xoutbuf[i0]<' ') {
                     /* stuff */
                     pb[j] = '}';
                     if (j<3000L) ++j;
                     pb[j] = (char)((uint8_t)(uint8_t)
                anonym->pcdeflat->xoutbuf[i0]^0x20U);
                  }
                  else pb[j] = anonym->pcdeflat->xoutbuf[i0];
                  if (j<3000L) ++j;
                  if (i0==tmp) break;
               } /* end for */
               pb[j] = '~';
               ++j;
               i0 = 0L;
               while (i0<4095L && i0<j) {
                  anonym->pcdeflat->xoutbuf[i0] = pb[i0];
                  ++i0;
               }
               anonym->pcdeflat->xoutlen = i0;
               anonym->pcdeflat->done = 1;
               /*WrStrLn("");WrStr("X:");
                ShowHex(pcdeflat^.xoutbuf, pcdeflat^.xoutlen); WrStrLn("");
                */
               break;
            }
            anonym->pcdeflat->xoutlen = -1L; /* crc error */
         }
      }
      if (anonym->pcdeflat->xinp>=anonym->outlen) {
         /* outbuffer empty */
         anonym->pcdeflat->xinp = 0L;
         anonym->outlen = 0L;
      }
   }
} /* end expand() */


static void wrpipe(int32_t fd, char b[], uint32_t b_len,
                int32_t * blen)
{
   int32_t i0;
   int32_t len;
   int32_t tmp;
   /*WrStrLn("");WrStr("P:"); ShowHex(b, blen); WrStrLn(""); */
   len = write(fd, (char *)b, (uint32_t)*blen);
   if (len>*blen) len = *blen;
   if (len>0L) {
      *blen -= len;
      tmp = *blen-1L;
      i0 = 0L;
      if (i0<=tmp) for (;; i0++) {
         b[i0] = b[i0+len];
         if (i0==tmp) break;
      } /* end for */
   }
/*    IF blen>0 THEN MOVE(ADR(b[len]), ADR(b), blen) END; */
} /* end wrpipe() */


static void sendjunk(l2_pLINK link0, struct JUNKBUF * junk)
{
   if (junk->len==256L || junk->len>=0L && junk->time0<=0L) {
      if (l2_SendStr(link0, (uint16_t)junk->len, (l2_pSTRING)junk->buf)) {
         junk->len = 0L;
      }
   }
} /* end sendjunk() */


static void wrl2(l2_pLINK link0, char b[], uint32_t b_len,
                int32_t * blen, struct JUNKBUF * junk)
{
   int32_t j;
   int32_t i0;
   int32_t len;
   /*WrStr("L:"); WrInt(blen, 1); WrStr(" "); */
   if (*blen==0L) return;
   if (junk->len<256L) {
      len = 0L;
      while (junk->len<256L && len<*blen) {
         junk->buf[junk->len] = b[len];
         ++junk->len;
         ++len;
      }
      j = len;
      i0 = 0L;
      while (j<*blen) {
         b[i0] = b[j];
         ++j;
         ++i0;
      }
      *blen -= len;
      junk->time0 = 3L;
   }
   sendjunk(link0, junk);
} /* end wrl2() */


static void wrclose(pTASK pt0, int32_t fd, char c[], uint32_t c_len)
{
   char s[100];
   X2C_PCOPY((void **)&c,c_len);
   WerrCall(pt0);
   aprsstr_IntToStr(fd, 1UL, s, 100ul);
   osi_Werr(c, c_len);
   osi_Werr(s, 100ul);
   osi_Werr("\012", 2ul);
   X2C_PFREE(c);
} /* end wrclose() */


static void crlf(char b[], uint32_t b_len, int32_t * len,
                uint32_t mode)
{
   int32_t j;
   int32_t i0;
   if (mode>0UL) {
      /* convert LF to pr */
      i0 = 0L;
      j = 0L;
      while (i0<*len) {
         if (b[i0]=='\012') {
            if (mode==1UL) {
               b[j] = '\015';
               ++j;
            }
         }
         else {
            b[j] = b[i0];
            ++j;
         }
         ++i0;
      }
      *len = j;
   }
} /* end crlf() */


static void crlfp(char b[], uint32_t b_len, int32_t * len,
                uint32_t mode)
/*1 del lf, 2 del lf cr>lf, 3 cr>lf, 4 del cr, 5 del cr lf>cr, 6 lf>cr */
{
   int32_t j;
   int32_t i0;
   if (mode>0UL) {
      /* convert LF to pr */
      i0 = 0L;
      j = 0L;
      while (i0<*len) {
         if (mode==1UL) {
            if (b[i0]!='\012') {
               b[j] = b[i0];
               ++j;
            }
         }
         else if (mode==2UL) {
            if (b[i0]=='\015') {
               b[j] = '\012';
               ++j;
            }
            else if (b[i0]!='\012') {
               b[j] = b[i0];
               ++j;
            }
         }
         else if (mode==3UL) {
            if (b[i0]=='\015') b[j] = '\012';
            else b[j] = b[i0];
            ++j;
         }
         else if (mode==4UL) {
            if (b[i0]!='\015') {
               b[j] = b[i0];
               ++j;
            }
         }
         else if (mode==5UL) {
            if (b[i0]=='\012') {
               b[j] = '\015';
               ++j;
            }
            else if (b[i0]!='\015') {
               b[j] = b[i0];
               ++j;
            }
         }
         else if (mode==6UL) {
            if (b[i0]=='\012') b[j] = '\015';
            else b[j] = b[i0];
            ++j;
         }
         ++i0;
      }
      *len = j;
   }
} /* end crlfp() */


static void detect(pTASK pt0)
/* startet task first sends command on stdout */
{
   int32_t j;
   int32_t i0;
   struct TASK * anonym;
   { /* with */
      struct TASK * anonym = pt0;
      while (anonym->inlen>0L && anonym->detectdone>0UL) {
         i0 = 0L;
         if (anonym->detectdone==1UL) {
            if (anonym->inbuf[0U]=='!') {
               anonym->detectdone = 2UL;
               i0 = 1L;
            }
            else anonym->detectdone = 0UL;
         }
         else if (anonym->detectdone==2UL) {
            if (anonym->inbuf[0U]=='d') {
               initcompress(pt0);
               i0 = 1L;
               if (verb) osi_Werr("task switched deflate on\012", 26ul);
            }
            else if (anonym->inbuf[0U]=='p') {
               initcompress(pt0);
               anonym->pcdeflat->pppframing = 1;
               i0 = 1L;
               if (verb) {
                  osi_Werr("task switched ppp + deflate on\012", 32ul);
               }
            }
            else if (anonym->inbuf[0U]=='l') {
               anonym->crlfmode = 1UL;
               i0 = 1L;
               if (verb) osi_Werr("task switched LF to CR on\012", 27ul);
            }
            else if (anonym->inbuf[0U]=='L') {
               anonym->crlfmode = 2UL;
               i0 = 1L;
               if (verb) osi_Werr("task switched delete LF on\012", 28ul);
            }
            else if (anonym->inbuf[0U]=='8') {
               i0 = 1L;
               if (verb) osi_Werr("task switched transparent\012", 27ul);
            }
            anonym->detectdone = 0UL;
         }
         else anonym->detectdone = 0UL;
         if (i0>0L) {
            /* delete command */
            j = 0L;
            while (i0<anonym->inlen) {
               anonym->inbuf[j] = anonym->inbuf[i0];
               ++i0;
               ++j;
            }
            anonym->inlen = j;
         }
      }
   }
} /* end detect() */


static void AppendTime(char s[], uint32_t s_len)
{
   char h[31];
   aprsstr_TimeToStr(osic_time()%86400UL, h, 31ul);
   aprsstr_Append(s, s_len, " - ", 4ul);
   aprsstr_Append(s, s_len, h, 31ul);
} /* end AppendTime() */

static uint16_t l2cat_UA = 0x63U;

static uint16_t l2cat_DM = 0xFU;

static uint16_t l2cat_SABM = 0x2FU;

static uint16_t l2cat_DISC = 0x43U;

static uint16_t l2cat_FRMR = 0x87U;

static uint16_t l2cat_UI = 0x3U;

static uint16_t l2cat_RR = 0x1U;

static uint16_t l2cat_REJ = 0x9U;

static uint16_t l2cat_RNR = 0x5U;

#define l2cat_LF0 "\015"

#define l2cat_LINELEN 78

static char l2cat_oneline = 0;


static void Nibble(int32_t * p, char rxbuf[2048], uint16_t n)
{
   rxbuf[*p] = (char)((uint32_t)(48U+n)+7UL*(uint32_t)(n>9U));
   ++*p;
} /* end Nibble() */


static void WriteHex(char rxbuf[2048], int32_t * p, char c)
{
   Nibble(p, rxbuf, (uint16_t)((uint32_t)(uint8_t)c/16UL));
   Nibble(p, rxbuf, (uint16_t)((uint32_t)(uint8_t)c&15UL));
} /* end WriteHex() */


static void ShowCall(int32_t * p, char rxbuf[2048],
                struct l2_GETADRESS * getadress, uint16_t cp,
                char hbit)
{
   uint32_t i0;
   char c;
   uint32_t tmp;
   tmp = (uint32_t)(cp+5U);
   i0 = (uint32_t)cp;
   if (i0<=tmp) for (;; i0++) {
      c = getadress->adress[i0];
      if (c!=' ') {
         if ((uint8_t)c>' ') {
            rxbuf[*p] = c;
            ++*p;
         }
         else WriteHex(rxbuf, p, c);
      }
      if (i0==tmp) break;
   } /* end for */
   i0 = (uint32_t)(uint8_t)getadress->adress[cp+6U]&15UL;
   if (i0) {
      rxbuf[*p] = '-';
      ++*p;
      if (i0>9UL) {
         rxbuf[*p] = (char)(48UL+i0/10UL);
         ++*p;
      }
      rxbuf[*p] = (char)(48UL+i0%10UL);
      ++*p;
   }
   if (hbit && (0x40U & (uint16_t)(uint32_t)(uint8_t)
                getadress->adress[cp+6U])) {
      rxbuf[*p] = '*';
      ++*p;
   }
} /* end ShowCall() */


static void MonHead(char rxbuf[2048], struct l2_GETADRESS * getadress,
                l2_pLINK l, int32_t ilen, const char info[],
                uint32_t info_len)
{
   int32_t p;
   int32_t i0;
   uint32_t com;
   uint16_t cmd;
   char cr;
   char pf;
   char PF0[4];
   char s[21];
   /*    udp2:ARRAY[0..99] OF CHAR; */
   int32_t tmp;
   l2_GetAdress0(l, getadress);
   aprsstr_IntToStr((int32_t)getadress->port, 1UL, rxbuf, 2048ul);
   aprsstr_Append(rxbuf, 2048ul, ":fm ", 5ul);
   p = (int32_t)aprsstr_Length(rxbuf, 2048ul);
   ShowCall(&p, rxbuf, getadress, 7U, 0);
   rxbuf[p] = ' ';
   ++p;
   rxbuf[p] = 't';
   ++p;
   rxbuf[p] = 'o';
   ++p;
   rxbuf[p] = ' ';
   ++p;
   ShowCall(&p, rxbuf, getadress, 0U, 0);
   rxbuf[p] = ' ';
   ++p;
   i0 = 14L;
   while (i0+7L<71L && getadress->adress[i0]) {
      if (i0==14L) {
         rxbuf[p] = 'v';
         ++p;
         rxbuf[p] = 'i';
         ++p;
         rxbuf[p] = 'a';
         ++p;
         rxbuf[p] = ' ';
         ++p;
      }
      ShowCall(&p, rxbuf, getadress, (uint16_t)i0, 1);
      rxbuf[p] = ' ';
      ++p;
      i0 += 7L;
   }
   rxbuf[p] = 'c';
   ++p;
   rxbuf[p] = 't';
   ++p;
   rxbuf[p] = 'l';
   ++p;
   rxbuf[p] = ' ';
   ++p;
   cmd = (uint16_t)getadress->my;
   com = (uint32_t)((0x40U & (uint16_t)(uint32_t)(uint8_t)
                getadress->adress[6U])!=0)+2UL*(uint32_t)
                ((0x40U & (uint16_t)(uint32_t)(uint8_t)
                getadress->adress[13U])!=0);
   pf = (0x10U & cmd)!=0;
   cmd = cmd&~0x10U;
   if ((cmd&0xFU)==0x1U) {
      strncpy(PF0,"RR  ",4u);
      PF0[2U] = (char)(48U+(uint16_t)cmd/32U);
   }
   else if ((cmd&0xFU)==0x5U) {
      strncpy(PF0,"RNR ",4u);
      PF0[3U] = (char)(48U+(uint16_t)cmd/32U);
   }
   else if ((cmd&0xFU)==0x9U) {
      strncpy(PF0,"REJ ",4u);
      PF0[3U] = (char)(48U+(uint16_t)cmd/32U);
   }
   else if ((cmd&0x1U)==0U) {
      strncpy(PF0,"I   ",4u);
      PF0[2U] = (char)(48U+((uint16_t)cmd/2U&7U));
      PF0[1U] = (char)(48U+(uint16_t)cmd/32U);
   }
   else if (cmd==0x3U) strncpy(PF0,"UI  ",4u);
   else if (cmd==0xFU) strncpy(PF0,"DM  ",4u);
   else if (cmd==0x2FU) strncpy(PF0,"SABM",4u);
   else if (cmd==0x43U) strncpy(PF0,"DISC",4u);
   else if (cmd==0x63U) strncpy(PF0,"UA  ",4u);
   else if (cmd==0x87U) strncpy(PF0,"FRMR",4u);
   else WriteHex(rxbuf, &p, (char)getadress->my);
   for (i0 = 0L; i0<=3L; i0++) {
      if (PF0[i0]!=' ') {
         rxbuf[p] = PF0[i0];
         ++p;
      }
   } /* end for */
   strncpy(PF0,"v^-+",4u);
   cr = 1;
   if (com==0UL || com==3UL) {
      rxbuf[p] = 'v';
      ++p;
      rxbuf[p] = '1';
      ++p;
   }
   else {
      rxbuf[p] = PF0[(com&1UL)+2UL*(uint32_t)pf];
      ++p;
   }
   if (getadress->cpid<=255U) {
      rxbuf[p] = ' ';
      ++p;
      rxbuf[p] = 'p';
      ++p;
      rxbuf[p] = 'i';
      ++p;
      rxbuf[p] = 'd';
      ++p;
      rxbuf[p] = ' ';
      ++p;
      WriteHex(rxbuf, &p, (char)getadress->cpid);
   }
   if ((0x10U & (uint16_t)(uint32_t)(uint8_t)getadress->adress[13U])
                ==0) {
      rxbuf[p] = ' ';
      ++p;
      rxbuf[p] = 'd';
      ++p;
      rxbuf[p] = 'a';
      ++p;
      rxbuf[p] = 'm';
      ++p;
      rxbuf[p] = 'a';
      ++p;
      rxbuf[p] = ' ';
      ++p;
   }
   /*
     Getudp2info(l, udp2);
     IF udp2[0]<>0C THEN
       rxbuf[p]:=" ";
       INC(p);
       i:=0;
       WHILE (p<HIGH(rxbuf)) & (udp2[i]<>0C) DO rxbuf[p]:=udp2[i]; INC(p);
                INC(i) END;
     END;
   */
   rxbuf[p] = 0;
   if (ilen>0L && montyp==1UL) {
      aprsstr_IntToStr(ilen, 1UL, s, 21ul);
      aprsstr_Append(rxbuf, 2048ul, " ", 2ul);
      aprsstr_Append(rxbuf, 2048ul, s, 21ul);
      aprsstr_Append(rxbuf, 2048ul, "B", 2ul);
   }
   AppendTime(rxbuf, 2048ul);
   if (ilen>0L && montyp>1UL) {
      p = (int32_t)aprsstr_Length(rxbuf, 2048ul);
      rxbuf[p] = '\012';
      ++p;
      tmp = ilen-1L;
      i0 = 0L;
      if (i0<=tmp) for (;; i0++) {
         if ((uint8_t)info[i0]>=' ' && (uint8_t)info[i0]<'\177') {
            rxbuf[p] = info[i0];
         }
         else rxbuf[p] = '.';
         ++p;
         if (i0==tmp) break;
      } /* end for */
      rxbuf[p] = 0;
   }
   osi_Werr(rxbuf, 2048ul);
   osi_Werr("\012", 2ul);
} /* end MonHead() */


static void Monitor(void)
{
   char rxbuf[2048];
   struct l2_GETADRESS getadress;
   int32_t ilen;
   char ibuf[300];
   if (pmon==0) pmon = l2_GetMon();
   if (pmon) {
      ilen = (int32_t)l2_GetStr(pmon, 256U, (l2_pSTRING)ibuf);
      MonHead(rxbuf, &getadress, pmon, ilen, ibuf, 300ul);
      l2_Disconnect(&pmon, 1); /*release buffer*/
   }
} /* end Monitor() */


static void sleeprecon(void)
{
   char s[21];
   if (!terminate) {
      if (verb) {
         aprsstr_IntToStr((int32_t)(recontime/1000UL), 1UL, s, 21ul);
         osi_Werr(" delay ", 8ul);
         osi_Werr(s, 21ul);
         osi_Werr("ms before next connect\012", 24ul);
      }
      usleep(recontime);
   }
   if (recontime>maxrecontime/2UL) recontime = maxrecontime;
   else recontime = recontime*2UL;
} /* end sleeprecon() */

static uint32_t i;

static uint32_t ts;

static uint32_t timeout;

static int32_t ret;

static l2_CALLBACKPROC eventproc;

static pTASK pt;

static pTASK pth;

static int32_t pp0;

static int32_t pp1;

static uint16_t sockset;

static char l2tick;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   uint32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(IOBUF)!=256) X2C_ASSERT(0);
   deflate_BEGIN();
   frameio_BEGIN();
   l2_BEGIN();
   osi_BEGIN();
   aprsstr_BEGIN();
   Execlogin_BEGIN();
   memset((char *)frameio_udpsocks0,(char)0,
                sizeof(struct frameio_UDPSOCK [15]));
   cmdline[0] = 0;
   pipename[0] = 0;
   connectto0[0] = 0;
   memset((char *)mycall,(char)0,113UL);
   convlfcr = 0UL;
   convtopipe = 0UL;
   terminate = 0;
   keepconnected = 0;
   usedeflat = 0;
   verb2 = 0;
   verb = 0;
   pppmode = 0;
   autodet = 0;
   pppdiscard = X2C_max_longint;
   montyp = 0UL;
   minrecontime = 2000000UL;
   maxrecontime = 120000000UL;
   axudpchk = 0UL;
   Parms();
   pmon = 0;
   eventproc = Event;
   sockset = 0U;
   tmp = sockc;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      sockset |= (1U<<i);
      if (i==tmp) break;
   } /* end for */
   l2_L2Init(1000U, sockset, eventproc);
   l2_l2verb = verb;
   l2_dupchk = axudpchk;
   signal(SIGCHLD, cleanup); /*CleanWho*/
   /*
     MOVE(ADR(mycall), ADR(hc[0*7]), 7);          (* via call *)
     MOVE(ADR(mycall), ADR(hc[1*7]), 7);          (* connect call *)
     hc[2*7]:=0C;
   */
   parms.test = 0;
   parms.parm = 0U; /* mycalls */
   parms.port = 1U;
   parms.str = (l2_pSTRING)mycall;
   l2_Parm(&parms);
   for (i = 0UL; i<=255UL; i++) {
      parms.test = 0;
      parms.port = (uint16_t)i;
      parms.parm = 10U;
      parms.val = montyp;
      l2_Parm(&parms);
      if (numpar[i].port>0L) {
         /* port configured */
         pp0 = numpar[i].port;
         pp1 = pp0;
         if (pp0>15L) {
            /* set parameter to all ports */
            pp0 = 1L;
            pp1 = 15L;
         }
         do {
            parms.test = 0;
            parms.port = (uint16_t)pp0;
            parms.parm = (uint16_t)numpar[i].parmnum;
            parms.val = (uint32_t)numpar[i].val;
            if (verb2) {
               osi_WrStr("set port ", 10ul);
               osic_WrINT32((uint32_t)parms.port, 1UL);
               osi_WrStr(" parmnum ", 10ul);
               osic_WrINT32((uint32_t)parms.parm, 1UL);
               osi_WrStr(" to value ", 11ul);
               osic_WrINT32(parms.val, 1UL);
               osi_WrStrLn("", 1ul);
            }
            l2_Parm(&parms);
            ++pp0;
         } while (pp0<=pp1);
      }
   } /* end for */
   if (connectto0[0U]) {
      signal(SIGTERM, killdisc);
      signal(SIGINT, killdisc);
      signal(SIGPIPE, killdisc);
   }
   recontime = minrecontime;
   tasks = 0;
   timesum = 0L;
   for (;;) {
      if (tasks==0) {
         if (terminate) X2C_ABORT();
         if (connectto0[0U]) {
            for (;;) {
               Connectto();
               if (terminate || tasks) break;
               sleeprecon();
            }
         }
      }
      fdclr();
      pt = tasks;
      while (pt) {
         if (pt->state==1U) {
            /*        recontime:=minrecontime; */
            if (cmdline[0U]) {
               /* start task */
               Login(&pt->infd, pt->l2addr.adress, 72ul);
               pt->outfd = pt->infd;
               /*WrInt(pt^.fd, 1); WrStrLn(" fd"); */
               if (pt->infd>=0L) {
                  /*ioctl(pt^.fd, TIOCSCTTY, ADR(iocbuf));  */
                  pt->state = 2U;
               }
               else {
                  pt->state = 3U;
                  if (verb) osi_Werr("task start failed\012", 19ul);
               }
            }
            else if (pipename[0U]) {
               /* use pipe */
               pt->infd = osi_OpenRW(pipename, 4096ul);
               pt->outfd = pt->infd;
               if (pt->infd>=0L) pt->state = 2U;
               else {
                  pt->state = 3U;
                  if (verb) osi_Werr("pipe open error\012", 17ul);
               }
            }
            else {
               pt->state = 2U; /* use stdin stdout */
               pt->infd = 0L;
               pt->outfd = 1L;
            }
            if (usedeflat) initcompress(pt);
            else pt->pcdeflat = 0;
         }
         else if (pt->state==2U) {
            if ((terminate || (0x2U & pt->events)) || (0x4U & pt->events)) {
               pt->state = 3U;
               if (verb) {
                  WerrCall(pt);
                  osi_Werr(" layer2 disconnect\012", 20ul);
               }
               if (cmdline[0U]==0 && connectto0[0U]==0) X2C_ABORT();
            }
            if ((pt->infd>=0L && pt->inlen==0L) && pt->inignor==0UL) {
               fdsetr((uint32_t)pt->infd);
            }
            if (pt->inignor>0UL) --pt->inignor;
         }
         else if (pt->state==3U) {
            if (verb) {
               WerrCall(pt);
               osi_Werr(" disconnect\012", 13ul);
            }
            if (cmdline[0U]) {
               if (pt->infd>=0L && pt->infd!=pt->outfd) {
                  if (verb) wrclose(pt, pt->infd, " close in pipe ", 16ul);
                  osic_Close(pt->infd);
               }
               if (pt->outfd>=0L) {
                  if (verb) wrclose(pt, pt->outfd, " close out pipe ", 17ul);
                  osic_Close(pt->outfd);
               }
               pt->infd = -1L;
               pt->outfd = -1L;
               fdclr();
            }
            l2_Disconnect(&pt->link0, 0);
            pt->state = 4U;
         }
         if (pt->state==4U || pt->state==5U) {
            if ((terminate || (0x2U & pt->events)) || (0x4U & pt->events)) {
               if (verb) {
                  WerrCall(pt);
                  osi_Werr(" deallocate link\012", 18ul);
               }
               l2_Disconnect(&pt->link0, 1);
               if (verb) {
                  WerrCall(pt);
                  osi_Werr(" disconnect close layer2\012", 26ul);
               }
               if (tasks==pt) tasks = pt->next;
               else {
                  pth = tasks;
                  while (pth->next!=pt) pth = pth->next;
                  pth->next = pt->next;
               }
               if (pt->pcdeflat) {
                  osic_free((char * *) &pt->pcdeflat,
                sizeof(struct DEFLATBUF));
               }
               osic_free((char * *) &pt, sizeof(struct TASK));
               pt = 0;
               if (tasks==0) {
                  if (!keepconnected) terminate = 1;
                  else if (connectto0[0U]) sleeprecon();
               }
            }
            else pt = pt->next;
         }
         else pt = pt->next;
      }
      ts = 0UL;
      timeout = 10000UL;
      ret = selectrwt(&ts, &timeout);
      timesum += (int32_t)(10000UL-timeout);
      if (timesum>=10000L) {
         /*    IF timeout<=0 THEN */
         l2_Layer2();
         if (montyp>0UL) Monitor();
         timesum -= 10000L;
         l2tick = 1;
      }
      else l2tick = 0;
      pt = tasks;
      while (pt) {
         if (pt->state==2U && pt->infd>=0L) {
            /*WrInt(pt^.state, 1); WrStrLn(" st"); */
            if (issetr((uint32_t)pt->infd)) {
               if (pt->inlen==0L) {
                  if (!terminate) {
                     /* read blocks after a SIGINT - why ever */
                     pt->inlen = osi_RdBin(pt->infd, (char *)pt->inbuf,
                256u/1u, 256UL);
                     if (pt->inlen==0L) {
                        pt->inignor = 20UL; /* read from pipe why ever */
                     }
                  }
               }
               if (verb2 && pt->inlen>0L) {
                  Whex("IN ", 4ul, pt->inbuf, 256ul, pt->inlen);
               }
            }
            /*WrStrLn("");WrStr("I:"); ShowHex(pt^.inbuf, pt^.inlen); */
            if (pt->inlen>0L) {
               if (~pt->detectdone>0UL) {
                  detect(pt); /* switch option with first bytes in stdin */
               }
               if (pt->pcdeflat) {
                  /* compress */
                  if (pt->pcdeflat->outlen==0L) compress(pt);
                  wrl2(pt->link0, pt->pcdeflat->outbuf, 20001ul,
                &pt->pcdeflat->outlen, &pt->junkbuf);
               }
               else {
                  /*            IF pt^.pcdeflat^.pppframing THEN pt^.junkbuf.time:=0 END;
                 */
                  crlf(pt->inbuf, 256ul, &pt->inlen, pt->crlfmode);
                  wrl2(pt->link0, pt->inbuf, 256ul, &pt->inlen,
                &pt->junkbuf);
               }
            }
            else if (pt->inlen<0L) {
               if (pt->junkbuf.len>0L) {
                  /* send rest */
                  pt->junkbuf.time0 = 0L;
                  sendjunk(pt->link0, &pt->junkbuf);
               }
               else {
                  pt->state = 3U;
                  if (verb) osi_Werr("read pipe from task broken\012", 28ul);
               }
            }
            if (pt->state==2U) {
               for (;;) {
                  if (pt->outlen>0L) {
                     if (verb2) {
                        Whex("OUT ", 5ul, pt->outbuf, 256ul, pt->outlen);
                     }
                     if (pt->pcdeflat) {
                        /* decompress */
                        if (!pt->pcdeflat->done) {
                           expand(pt);
                           if (pt->pcdeflat->xoutlen<0L) {
                              /* expand or crc error */
                              pt->state = 3U;
                              if (verb) {
                                 osi_Werr("deflate or crc error\012", 22ul);
                              }
                              break;
                           }
                        }
                        if (!pt->pcdeflat->pppframing || pt->pcdeflat->done) {
                           if (pt->pcdeflat->xoutlen>0L) {
                              wrpipe(pt->outfd, pt->pcdeflat->xoutbuf,
                4096ul, &pt->pcdeflat->xoutlen);
                           }
                           if (pt->pcdeflat->xoutlen==0L) {
                              pt->pcdeflat->done = 0;
                           }
                        }
                     }
                     else {
                        crlfp(pt->outbuf, 256ul, &pt->outlen, convtopipe);
                        wrpipe(pt->outfd, pt->outbuf, 256ul, &pt->outlen);
                     }
                  }
                  if (pt->outlen==0L && pt->detectdone==0UL) {
                     /* protokoll detect must be done */
                     pt->outlen = (int32_t)l2_GetStr(pt->link0, 256U,
                (l2_pSTRING)pt->outbuf);
                     if (pt->outlen<=0L) break;
                  }
                  else break;
               }
            }
         }
         if (l2tick && pt->junkbuf.len>0L) {
            if (pt->junkbuf.time0>0L) --pt->junkbuf.time0;
            else sendjunk(pt->link0, &pt->junkbuf);
         }
         pt = pt->next;
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
