/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* "@(#)hostint.c Mar 14 15:35:58 2021" */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define hostint_C_
#ifndef l2_H_
#include "l2.h"
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
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef frameio_H_
#include "frameio.h"
#endif

#define hostint_HANDLES 30

struct HANDLE;

typedef struct HANDLE * pTASK;


struct HANDLE {
   uint16_t events;
   uint8_t mynum;
   l2_pLINK plink;
   l2_CALLTYP mycall;
};

static int32_t ttyfd;

static struct HANDLE handles[30];

static uint16_t nextevent;

static uint16_t maxlinks;

static char txbuf[260];

static uint16_t txpos;

static char rxbuf[260];

static char uipath[15][71];

static uint16_t parmsport;

static uint16_t timestamp;

static uint16_t rxcount;

static uint16_t rxpos;

static l2_pLINK pmon;

static char moninfo;

static uint16_t numval;

/*result of NumParm*/
static uint16_t montyp; /*0=off, 1=on, 2=on(no info)*/

static struct l2_GETSTAT getstat;

static struct l2_GETADRESS getadress;

static struct l2_CONNECT connect;

static struct l2_PARMS0 parms;

static struct l2_SENDUI sendui;

static struct l2_PORTSTAT portstat;

static uint32_t sockc;

static l2_CALLTYP maincall;

static char verb;

static char ttynamee[1024];

static uint32_t baud;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void CardToStr(uint16_t x, uint16_t f, char s[],
                uint32_t s_len)
{
   uint16_t j;
   uint16_t i0;
   s[s_len-1] = 0;
   i0 = (uint16_t)(s_len-1);
   do {
      --i0;
      s[i0] = (char)(x%10U+48U);
      x = x/10U;
   } while (!(x==0U || i0==0U));
   if ((uint32_t)f<s_len-1) f = (uint16_t)((s_len-1)-(uint32_t)f);
   else f = 1U;
   while (i0>f) {
      --i0;
      s[i0] = ' ';
   }
   if (i0>0U) {
      j = 0U;
      while ((uint32_t)i0<=s_len-1 && s[i0]) {
         s[j] = s[i0];
         ++i0;
         ++j;
      }
      s[j] = 0;
   }
} /* end CardToStr() */

static void Event(char * *, l2_pLINK, uint8_t);


static void Event(char * * atask, l2_pLINK link, uint8_t event)
{
   uint16_t i0;
   struct l2_GETADRESS gadr;
   l2_pLINK pl;
   pTASK ptask;
   if (event==l2_eCONNECTED) {
      if (*atask==0) {
         l2_GetAdress0(link, &gadr);
         /*WrInt(gadr.my, 3); WrStrLn(":my "); */
         i0 = 1U;
         for (;;) {
            if (i0>=maxlinks) {
               *atask = 0;
               break;
            }
            /*say BUSY*/
            if (handles[i0].mynum==(uint8_t)
                gadr.my && (0x40U & handles[i0].events)) {
               /*WrStrLn(" listen "); */
               *atask = (char *) &handles[i0];
               handles[i0].events = 0x1U;
               handles[i0].plink = link;
               break;
            }
            ++i0;
         }
      }
      else {
         ptask = (pTASK)*atask;
         ptask->events = 0x1U;
      }
   }
   else if (event==l2_eCONNREQ) {
      l2_GetAdress0(link, &gadr);
      /*WrStr(gadr.adress); */
      /*WrStrLn(" flexconn");   */
      memset((char *) &connect,(char)0,sizeof(struct l2_CONNECT));
      connect.port = gadr.port;
      connect.baud = 1U;
      connect.cpid = 240U;
      connect.l2adr = (l2_pSTRING)gadr.adress;
      connect.typ = l2_cFLEXback;
      i0 = 1U;
      for (;;) {
         if (i0>=maxlinks) {
            *atask = 0;
            connect.typ = l2_cFLEXbusy;
            pl = l2_Connect0((char *) &handles[i0], &connect);
            break;
         }
         /*say BUSY*/
         if ((0x40U & handles[i0].events)
                && (gadr.my==0U || handles[i0].mynum==(uint8_t)gadr.my)) {
            *atask = (char *) &handles[i0];
            handles[i0].events = 0x1U;
            handles[i0].plink = l2_Connect0((char *) &handles[i0],
                &connect);
            if (gadr.my==0U) handles[i0].mynum = 1U;
            break;
         }
         ++i0;
      }
   }
   else if (event==l2_eFLEXDATA) l2_GetAdress0(link, &gadr);
   else if (*atask) {
      /*WrStr(gadr.adress); */
      /*WrInt(dbuf^.len, 10); */
      /*WrStrLn(" flexdata"); */
      /*WrInt(ORD(event), 1);WrStrLn(":host got event"); */
      ptask = (pTASK)*atask;
      ptask->events |= (1U<<event);
   }
} /* end Event() */


static void AppendTime(char s[], uint32_t s_len)
{
   char h[31];
   aprsstr_TimeToStr(osic_time()%86400UL, h, 31ul);
   aprsstr_Append(s, s_len, " - ", 4ul);
   aprsstr_Append(s, s_len, h, 31ul);
/*
  p:=Length(s);
  IF p+21<=HIGH(s) THEN
    s[p]:=" "; INC(p);
    s[p]:="-"; INC(p);
    s[p]:=" "; INC(p);
    r.AH:=2AH;
    Dos(r);
    digit(r.DL);
    s[p]:="."; INC(p);
    digit(r.DH);
    s[p]:="."; INC(p);
    digit(r.CX MOD 100);
    s[p]:=" "; INC(p);
    r.AH:=2CH;
    Dos(r);
    digit(CARD16(r.CH));
    s[p]:=":"; INC(p);
    digit(CARD16(r.CL));
    s[p]:=":"; INC(p);
    digit(CARD16(r.DH));
    s[p]:=0C;
  END;
*/
} /* end AppendTime() */

static uint16_t hostint_UA = 0x63U;

static uint16_t hostint_DM = 0xFU;

static uint16_t hostint_SABM = 0x2FU;

static uint16_t hostint_DISC = 0x43U;

static uint16_t hostint_FRMR = 0x87U;

static uint16_t hostint_UI = 0x3U;

static uint16_t hostint_RR = 0x1U;

static uint16_t hostint_REJ = 0x9U;

static uint16_t hostint_RNR = 0x5U;

#define hostint_LF "\015"

#define hostint_LINELEN 78

static char hostint_oneline = 0;


static void Nibble(uint16_t * p, uint16_t n)
{
   rxbuf[*p] = (char)((uint32_t)(48U+n)+7UL*(uint32_t)(n>9U));
   ++*p;
} /* end Nibble() */


static void WriteHex(uint16_t * p, char c)
{
   Nibble(p, (uint16_t)((uint32_t)(uint8_t)c/16UL));
   Nibble(p, (uint16_t)((uint32_t)(uint8_t)c&15UL));
} /* end WriteHex() */


static void ShowCall(uint16_t * p, uint16_t cp, char hbit)
{
   uint16_t i0;
   char c;
   uint16_t tmp;
   tmp = cp+5U;
   i0 = cp;
   if (i0<=tmp) for (;; i0++) {
      c = getadress.adress[i0];
      if (c!=' ') {
         if ((uint8_t)c>' ') {
            rxbuf[*p] = c;
            ++*p;
         }
         else WriteHex(p, c);
      }
      if (i0==tmp) break;
   } /* end for */
   i0 = (uint16_t)((uint32_t)(uint8_t)getadress.adress[cp+6U]&15UL);
   if (i0) {
      rxbuf[*p] = '-';
      ++*p;
      if (i0>9U) {
         rxbuf[*p] = (char)(48U+i0/10U);
         ++*p;
      }
      rxbuf[*p] = (char)(48U+i0%10U);
      ++*p;
   }
   if (hbit && (0x40U & (uint16_t)(uint32_t)(uint8_t)
                getadress.adress[cp+6U])) {
      rxbuf[*p] = '*';
      ++*p;
   }
} /* end ShowCall() */


static void MonHead(uint16_t mn, l2_pLINK l)
{
   uint16_t com;
   uint16_t p;
   uint16_t i0;
   uint16_t cmd;
   char cr;
   char pf;
   char PF0[4];
   char udp2[100];
   l2_GetAdress0(l, &getadress);
   strncpy(rxbuf,"  fm ",260u);
   /*rxbuf[2]:=CHR(64+getadress.port);
     rxbuf[3]:="/";*/
   p = 5U;
   ShowCall(&p, 7U, 0);
   rxbuf[p] = ' ';
   ++p;
   rxbuf[p] = 't';
   ++p;
   rxbuf[p] = 'o';
   ++p;
   rxbuf[p] = ' ';
   ++p;
   ShowCall(&p, 0U, 0);
   rxbuf[p] = ' ';
   ++p;
   i0 = 14U;
   while (i0+7U<71U && getadress.adress[i0]) {
      if (i0==14U) {
         rxbuf[p] = 'v';
         ++p;
         rxbuf[p] = 'i';
         ++p;
         rxbuf[p] = 'a';
         ++p;
         rxbuf[p] = ' ';
         ++p;
      }
      ShowCall(&p, i0, 1);
      rxbuf[p] = ' ';
      ++p;
      i0 += 7U;
   }
   rxbuf[p] = 'c';
   ++p;
   rxbuf[p] = 't';
   ++p;
   rxbuf[p] = 'l';
   ++p;
   rxbuf[p] = ' ';
   ++p;
   cmd = (uint16_t)getadress.my;
   com = (uint16_t)((uint32_t)((0x40U & (uint16_t)(uint32_t)
                (uint8_t)getadress.adress[6U])!=0)+2UL*(uint32_t)
                ((0x40U & (uint16_t)(uint32_t)(uint8_t)
                getadress.adress[13U])!=0));
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
   else WriteHex(&p, (char)getadress.my);
   for (i0 = 0U; i0<=3U; i0++) {
      if (PF0[i0]!=' ') {
         rxbuf[p] = PF0[i0];
         ++p;
      }
   } /* end for */
   strncpy(PF0,"v^-+",4u);
   cr = 1;
   if (com==0U || com==3U) {
      rxbuf[p] = 'v';
      ++p;
      rxbuf[p] = '1';
      ++p;
   }
   else {
      rxbuf[p] = PF0[(uint32_t)(com&1U)+2UL*(uint32_t)pf];
      ++p;
   }
   if (getadress.cpid<=255U) {
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
      WriteHex(&p, (char)getadress.cpid);
   }
   if ((0x10U & (uint16_t)(uint32_t)(uint8_t)getadress.adress[13U])==0)
                 {
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
   l2_Getudp2info(l, udp2, 100ul);
   if (udp2[0U]) {
      rxbuf[p] = ' ';
      ++p;
      i0 = 0U;
      while (p<259U && udp2[i0]) {
         rxbuf[p] = udp2[i0];
         ++p;
         ++i0;
      }
   }
   rxbuf[p] = 0;
   if (timestamp==2U) {
      AppendTime(rxbuf, 260ul);
      rxcount = p+12U;
   }
   else rxcount = p+1U;
   rxbuf[0U] = 0;
   rxbuf[1U] = (char)mn;
   rxpos = 0U;
} /* end MonHead() */


static void GetCall(const char from[], uint32_t from_len,
                uint32_t * p, uint32_t len0, l2_pSTRING to)
{
   uint16_t i0;
   /*WrStrLn("getcall:"); */
   /*  len:=Length(from); */
   while (*p<len0 && (uint8_t)from[*p]<=' ') ++*p;
   for (i0 = 0U; i0<=5U; i0++) {
      if ((*p<len0 && from[*p]!='-') && from[*p]!=' ') {
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
/*WrStr("getcall<");WrStr(to^);WrStrLn(">"); */
} /* end GetCall() */


static void CallStr(char c[], uint32_t c_len, char s[],
                uint32_t s_len)
{
   char h[11];
   uint16_t p;
   uint16_t n;
   uint16_t ip;
   X2C_PCOPY((void **)&c,c_len);
   ip = 0U;
   for (;;) {
      p = 0U;
      for (n = 0U; n<=5U; n++) {
         if (c[ip]!=' ') {
            h[p] = c[ip];
            ++p;
         }
         ++ip;
      } /* end for */
      h[p] = 0;
      n = (uint16_t)((uint32_t)(uint8_t)c[ip]&15UL);
      ++ip;
      aprsstr_Append(s, s_len, h, 11ul);
      if (n) {
         CardToStr(n, 0U, h, 11ul);
         aprsstr_Append(s, s_len, "-", 2ul);
         aprsstr_Append(s, s_len, h, 11ul);
      }
      if (ip==7U) ip = 14U;
      if ((uint32_t)(ip+6U)>c_len-1 || c[ip]==0) break;
      if (ip==14U) aprsstr_Append(s, s_len, " via ", 6ul);
      else aprsstr_Append(s, s_len, " ", 2ul);
   }
   X2C_PFREE(c);
} /* end CallStr() */


static void SetMy(uint16_t port)
{
   uint16_t n;
   uint16_t i0;
   uint16_t m;
   char h[113];
   l2_CALLTYP hc[16];
   uint16_t tmp;
   memset((char *)hc,(char)0,112UL);
   memcpy(hc[0U],handles[0U].mycall,7u); /* via digi call */
   n = 1U;
   tmp = maxlinks;
   m = 1U;
   if (m<=tmp) for (;; m++) {
      handles[m-1U].mynum = 0U;
      if (m==tmp) break;
   } /* end for */
   if (maxlinks>1U) {
      tmp = maxlinks-1U;
      m = 1U;
      if (m<=tmp) for (;; m++) {
         if (handles[m].mycall[0U]) {
            i0 = 1U;
            for (;;) {
               if (i0>=n) {
                  /* append call to L2 mycalls */
                  if (n<=15U) {
                     memcpy(hc[n],handles[m].mycall,7u);
                     handles[m].mynum = (uint8_t)n;
                     ++n;
                  }
                  break;
               }
               if (aprsstr_StrCmp(handles[m].mycall, 7ul, hc[i0], 7ul)) {
                  handles[m].mynum = (uint8_t)n;
                /* call is in L2 mycalls */
                  break;
               }
               ++i0;
            }
         }
         if (m==tmp) break;
      } /* end for */
   }
   /*    IF verb THEN WrStr("mycall:"); WrInt(port,1); WrStr(" ");
                WrStrLn(handles[m].mycall); END; */
   tmp = n;
   i0 = 1U;
   if (i0<=tmp) for (;; i0++) {
      X2C_MOVE((char *)hc[i0-1U],(char *) &h[(i0-1U)*7U],7UL);
      if (i0==tmp) break;
   } /* end for */
   h[n*7U] = 0;
   parms.test = 0;
   parms.parm = 0U;
   parms.port = parmsport;
   parms.str = (l2_pSTRING)h;
   l2_Parm(&parms);
} /* end SetMy() */


static char GetNumm(l2_pSTRING s, uint32_t * v)
{
   uint16_t i0;
   char ok0;
   i0 = 0U;
   *v = 0UL;
   ok0 = 0;
   while (s[i0]==' ') ++i0;
   while ((uint8_t)s[i0]>='0' && (uint8_t)s[i0]<='9') {
      ok0 = 1;
      *v = ( *v*10UL+(uint32_t)(uint8_t)s[i0])-48UL;
      ++i0;
   }
   return ok0;
} /* end GetNumm() */


static void Msg(uint16_t port, uint16_t n, uint16_t len0)
{
   rxbuf[0U] = (char)port;
   rxbuf[1U] = (char)n;
   rxcount = len0;
   rxpos = 0U;
} /* end Msg() */


static void Nothing(uint16_t port)
{
   Msg(port, 0U, 2U);
} /* end Nothing() */


static void Err(uint16_t port, char c)
{
   strncpy(rxbuf,"  INVALID COMMAND:   ",260u);
   rxbuf[19U] = c;
   Msg(port, 2U, 22U);
} /* end Err() */


static void InvalCh(uint16_t port)
{
   strncpy(rxbuf,"  INVALID CHANNEL NUMBER",260u);
   Msg(port, 2U, 25U);
} /* end InvalCh() */


static void RetNum(uint16_t port, uint16_t v)
{
   CardToStr(v, 8U, rxbuf, 260ul);
   Msg(port, 1U, (uint16_t)(aprsstr_Length(rxbuf, 260ul)+1UL));
} /* end RetNum() */


static void RetV(uint16_t port)
{
   /*
       n:=0;
       L1Parm(n,parmsport-1);
       CardToStr(n,8,rxbuf);
   */
   strncpy(rxbuf," L2PCX",260u);
   Msg(port, 1U, (uint16_t)(aprsstr_Length(rxbuf, 260ul)+1UL));
} /* end RetV() */


static char GetInfo(uint16_t port)
{
   uint16_t size;
   struct HANDLE * anonym;
   { /* with */
      struct HANDLE * anonym = &handles[port];
      size = 0U;
      if (anonym->plink) {
         size = l2_GetStr(anonym->plink, 256U, (l2_pSTRING) &rxbuf[3U]);
      }
      if (size) {
         rxbuf[2U] = (char)(size-1U);
         Msg(port, 7U, size+3U);
      }
      else Nothing(port);
   }
   return size!=0U;
} /* end GetInfo() */


static void DiscBusyMsg(uint16_t port, char dealloc)
{
   struct HANDLE * anonym;
   { /* with */
      struct HANDLE * anonym = &handles[port];
      if (anonym->plink) {
         l2_GetAdress0(anonym->plink, &getadress);
         CallStr(getadress.adress, 72ul, rxbuf, 260ul);
         if (dealloc) {
            /*WrStrLn(" busy disc ");*/
            l2_Disconnect(&anonym->plink, 1);
         }
      }
   }
   if (timestamp>=1U) AppendTime(rxbuf, 260ul);
   Msg(port, 3U, (uint16_t)(aprsstr_Length(rxbuf, 260ul)+1UL));
} /* end DiscBusyMsg() */


static void Linkstatus(uint16_t port, char unbuf)
{
   struct HANDLE * anonym;
   { /* with */
      struct HANDLE * anonym = &handles[port];
      if ((0x1U & anonym->events)) {
         anonym->events &= ~0x1U;
         l2_GetAdress0(anonym->plink, &getadress);
         strncpy(rxbuf,"  (1) CONNECTED to ",260u);
         DiscBusyMsg(port, 0);
      }
      else if ((0x2U & anonym->events)) {
         /*WrStrLn("host gets disc");  */
         if (unbuf || !GetInfo(port)) {
            strncpy(rxbuf,"  (1) DISCONNECTED fm ",260u);
            DiscBusyMsg(port, 1);
            anonym->events = 0x40U;
         }
      }
      else if ((0x4U & anonym->events)) {
         strncpy(rxbuf,"  (1) BUSY fm ",260u);
         DiscBusyMsg(port, 1);
         anonym->events = 0x40U;
      }
      else unbuf || GetInfo(port);
   }
} /* end Linkstatus() */


static void SendInfo(uint16_t port)
{
   char uib[257];
   struct HANDLE * anonym;
   if (port) {
      { /* with */
         struct HANDLE * anonym = &handles[port];
         if (anonym->plink) {
            do {
            } while (!l2_SendStr(anonym->plink, (uint16_t)((uint32_t)(uint8_t)txbuf[2U]+1UL), (l2_pSTRING) &txbuf[3U]));
         }
      }
   }
   else {
      sendui.port = parmsport;
      sendui.baud = 0U;
      sendui.cmd = '\023';
      sendui.path = (l2_pSTRING)uipath[(parmsport-1U)%15U];
      sendui.datalen = (uint16_t)((uint32_t)(uint8_t)txbuf[2U]+2UL);
      X2C_MOVE((char *) &txbuf[3U],(char *) &uib[1U],256UL);
      uib[0U] = '\360';
      sendui.data = (l2_pSTRING)uib;
      l2_SendRaw(&sendui);
   }
   Nothing(port);
} /* end SendInfo() */


static void Conn(uint16_t port)
{
   uint32_t i0;
   uint16_t cport;
   uint16_t j;
   struct HANDLE * anonym;
   if (txpos>5U) {
      cport = 1U;
      i0 = 4UL;
      GetCall(txbuf, 260ul, &i0, (uint32_t)txpos, (l2_pSTRING)rxbuf);
      X2C_MOVE((char *)handles[port].mycall,(char *) &rxbuf[7U],
                7UL);
      j = 14U;
      if (i0<(uint32_t)txpos) {
         GetCall(txbuf, 260ul, &i0, (uint32_t)txpos,
                (l2_pSTRING) &rxbuf[14U]);
         if ((rxbuf[15U]==' ' && (uint8_t)rxbuf[14U]>='1') && (uint32_t)
                (uint8_t)rxbuf[14U]<=63UL) {
            cport = (uint16_t)((uint32_t)(uint8_t)rxbuf[14U]-48UL);
         }
         else if (rxbuf[14U]!=' ') j += 7U;
         while (j<70U && i0<(uint32_t)txpos) {
            GetCall(txbuf, 260ul, &i0, (uint32_t)txpos,
                (l2_pSTRING) &rxbuf[j]);
            j += 7U;
         }
      }
      rxbuf[j] = 0;
      if (port>0U) {
         connect.port = cport;
         connect.baud = 1U;
         connect.cpid = 240U;
         connect.l2adr = (l2_pSTRING)rxbuf;
         connect.l3adr = 0;
         connect.typ = l2_cNORMAL;
         if (handles[port].plink==0) {
            handles[port].plink = l2_Connect0((char *) &handles[port],
                &connect);
         }
      }
      else {
         X2C_MOVE((char *)rxbuf,(char *)uipath[(parmsport-1U)%15U],
                71UL); /* ui path */
         uipath[(parmsport-1U)%15U][6U] = (char)((uint32_t)(uint8_t)
                uipath[(parmsport-1U)%15U][6U]+64UL); /* make v2 command */
      }
      Nothing(port);
   }
   else {
      { /* with */
         struct HANDLE * anonym = &handles[port];
         if (anonym->plink) {
            l2_GetAdress0(anonym->plink, &getadress);
            strncpy(rxbuf,"  ",260u);
            CallStr(getadress.adress, 72ul, rxbuf, 260ul);
            Msg(port, 1U, (uint16_t)(aprsstr_Length(rxbuf, 260ul)+1UL));
         }
         else if (port==0U) {
            strncpy(rxbuf,"  ID",260u);
            Msg(port, 1U, 5U);
         }
         else {
            strncpy(rxbuf,"  CHANNEL NOT CONNECTED",260u);
            Msg(port, 1U, 24U);
         }
      }
   }
} /* end Conn() */


static void ChanStat(char * ch, uint16_t port)
{
   char h[11];
   struct HANDLE * anonym;
   if (port==0U) {
      /*monitor*/
      if (pmon==0) pmon = l2_GetMon();
      strncpy(rxbuf,"  0 0",260u);
      if (pmon) rxbuf[4U] = '1';
      Msg(port, 1U, 6U);
   }
   else {
      /*port*/
      strncpy(rxbuf,"  0 0 ",260u);
      { /* with */
         struct HANDLE * anonym = &handles[port];
         if ((anonym->events&0xFU)!=0U) rxbuf[2U] = '1';
         if (anonym->plink) {
            if (l2_GetChar(anonym->plink, 0, ch)) rxbuf[4U] = '1';
            getstat.l = anonym->plink;
            getstat.n = 0U;
            l2_GetStat0(&getstat);
            CardToStr(getstat.txbu, 0U, h, 11ul);
            aprsstr_Append(h, 11ul, " 0 ", 4ul);
            aprsstr_Append(rxbuf, 260ul, h, 11ul);
            CardToStr(getstat.ret, 0U, h, 11ul);
            aprsstr_Append(h, 11ul, " ", 2ul);
            aprsstr_Append(rxbuf, 260ul, h, 11ul);
            if (getstat.st<=4U) {
               CardToStr((uint16_t)((uint32_t)getstat.st-(uint32_t)
                (getstat.st!=0U)), 0U, h, 11ul);
            }
            else {
               getstat.st += (uint16_t)((uint32_t)
                getstat.bsy+2UL*(uint32_t)getstat.remb);
               if (getstat.st>=6U) getstat.st += 2U;
               CardToStr(getstat.st-1U, 0U, h, 11ul);
            }
            aprsstr_Append(rxbuf, 260ul, h, 11ul);
         }
         else aprsstr_Append(rxbuf, 260ul, "0 0 0 0", 8ul);
      }
      Msg(port, 1U, (uint16_t)(aprsstr_Length(rxbuf, 260ul)+1UL));
   }
} /* end ChanStat() */


static void NumParm(uint16_t port, uint16_t start, uint16_t par,
                uint16_t min0)
{
   parms.test = start>=txpos || !GetNumm((l2_pSTRING) &txbuf[start],
                &parms.val);
   if (parms.val<(uint32_t)min0) parms.val = (uint32_t)min0;
   parms.parm = par;
   /*IF par=0 THEN WrStrLn(" setmy ") END; */
   parms.port = parmsport;
   l2_Parm(&parms);
   if (parms.test) RetNum(port, (uint16_t)parms.val);
   else Nothing(port);
   numval = (uint16_t)parms.val;
} /* end NumParm() */


static void Bufs(uint16_t port)
{
   portstat.port = 1U;
   portstat.minits5 = 1U;
   if (l2_PortStat0(&portstat)) RetNum(port, portstat.mem*8U);
   else Nothing(port);
} /* end Bufs() */


static void TxCMD(void)
{
   uint16_t port;
   uint32_t n;
   char ch;
   struct HANDLE * anonym;
   /*
   WrStr("ch :"); WrInt(ORD(txbuf[0]), 2);
   WrStr(" cmd:"); WrInt(ORD(txbuf[1]), 2);
   WrStr(" len:"); WrInt(ORD(txbuf[2])+1, 2);
   FOR n:=0 TO ORD(txbuf[2]) DO WrInt(ORD(txbuf[n+3]), 4) END; WrLn;
   */
   port = (uint16_t)(uint8_t)txbuf[0U];
   /*IF port=255 THEN port:=0 END; */
   if (port>29U) InvalCh(port);
   else if (txbuf[1U]) {
      /*cmd*/
      if (txpos<=259U) txbuf[txpos] = 0;
      switch ((unsigned)X2C_CAP(txbuf[3U])) {
      case 'B':
         NumParm(port, 4U, 29U, 3000U);
         break;
      case 'C':
         Conn(port);
         break;
      case 'D':
         { /* with */
            struct HANDLE * anonym = &handles[port];
            if (port && anonym->plink) {
               /*WrStrLn(" term disc ");*/
               l2_Disconnect(&anonym->plink, 0);
            }
         }
         /*   rxbuf:="  (1) DISCONNECTED";
              Msg(3,19); */
         Nothing(port);
         break;
      case 'F': /*frac*/
         NumParm(port, 4U, 24U, 1U);
         break;
      case 'G':
         if (port==0U) {
            /*monitor*/
            if (txpos==5U && txbuf[4U]=='1') Nothing(port);
            else if (moninfo) {
               /*not G1*/
               /*info to header*/
               rxbuf[2U] = (char)(l2_GetStr(pmon, 256U,
                (l2_pSTRING) &rxbuf[3U])-1U);
               Msg(port, 6U, (uint16_t)((uint32_t)(uint8_t)rxbuf[2U]+4UL));
               /*WrStrLn(" mon info disc "); */
               l2_Disconnect(&pmon, 1); /*release buffers*/
               moninfo = 0;
            }
            else {
               if (pmon==0) pmon = l2_GetMon();
               if (pmon) {
                  if (montyp!=2U && l2_GetChar(pmon, 0, &ch)) {
                     /*test if info to header*/
                     MonHead(5U, pmon);
                     moninfo = 1;
                  }
                  else {
                     MonHead(4U, pmon);
                     l2_Disconnect(&pmon, 1); /*release buffer*/
                  }
               }
               else Nothing(port);
            }
         }
         else if (txpos==5U) {
            /*ports*/
            if (txbuf[4U]=='1') {
               /*G1*/
               Linkstatus(port, 1);
            }
            else {
               /*G0*/
               GetInfo(port);
            }
         }
         else {
            /*G*/
            Linkstatus(port, 0);
         }
         break;
      case 'I':
         n = 4UL;
         if (txpos>4U) {
            /*WrStr("<<");WrStrhex(txbuf, txpos);WrStrLn(">>"); */
            GetCall(txbuf, 260ul, &n, (uint32_t)txpos,
                (l2_pSTRING)handles[port].mycall);
            if (verb) {
               osi_WrStr("mycall port=", 13ul);
               osic_WrINT32((uint32_t)port, 1UL);
               osi_WrStr(":", 2ul);
               osi_WrStrLn(handles[port].mycall, 7ul);
            }
            if (port==0U) {
               /* set UI mycall */
               X2C_MOVE((char *)handles[port].mycall,
                (char *) &uipath[0U][7U],71UL); /* ui path */
               uipath[0U][13U] = (char)((uint32_t)(uint8_t)
                uipath[0U][13U]+128UL);
            }
            SetMy(port);
            Nothing(port);
         }
         else {
            strncpy(rxbuf,"  ",260u);
            if (handles[port].mycall[0U]==0) {
               CallStr("NOCALL0", 8ul, rxbuf, 260ul);
            }
            else CallStr(handles[port].mycall, 7ul, rxbuf, 260ul);
            /*WrStr("getI port=");WrInt(port,1); WrStr(":");
                WrStrLn(handles[port].mycall); */
            Msg(port, 1U, (uint16_t)(aprsstr_Length(rxbuf, 260ul)+1UL));
         }
         break;
      case 'J':
         Nothing(port);
         break;
      case 'K':
         if (GetNumm((l2_pSTRING) &txbuf[4U], &n) && n<=2UL) {
            timestamp = (uint16_t)n;
            Nothing(port);
         }
         else RetNum(port, timestamp);
         break;
      case 'L':
         ChanStat(&ch, port);
         break;
      case 'M': /*monitor*/
         NumParm(port, 4U, 10U, 0U);
         montyp = numval;
         break;
      case 'N': /*retry*/
         NumParm(port, 4U, 27U, 1U);
         break;
      case 'O': /*maxframes*/
         NumParm(port, 4U, 23U, 1U);
         break;
      case 'P':
         if (GetNumm((l2_pSTRING) &txbuf[4U], &n) && n>=1UL) {
            if (n>=103UL) NumParm(port, 7U, (uint16_t)(n-100UL), 0U);
            else {
               if (n<=15UL) parmsport = (uint16_t)n;
               Nothing(port);
            }
         }
         else RetNum(port, parmsport);
         break;
      case 'R': /*digi*/
         NumParm(port, 4U, 7U, 0U);
         break;
      case 'T': /*txdel*/
         NumParm(port, 4U, 19U, 0U);
         break;
      case 'U':
         Nothing(port);
         break;
      case 'V':
         if (GetNumm((l2_pSTRING) &txbuf[4U], &n)) {
            /*        L1Parm(n,parmsport-1); */
            Nothing(port);
         }
         else RetV(port);
         break;
      case 'W': /*dcdwait*/
         NumParm(port, 4U, 20U, 0U);
         break;
      case 'Y': /*maxlinks*/
         if (GetNumm((l2_pSTRING) &txbuf[4U], &n)) {
            if (n<=29UL) maxlinks = (uint16_t)n;
            else maxlinks = 30U;
            Nothing(port);
         }
         else RetNum(port, maxlinks);
         break;
      case '@':
         switch ((unsigned)X2C_CAP(txbuf[4U])) {
         case 'B':
            Bufs(port);
            break;
         case 'D':
            Nothing(port);
            break;
         case 'S':
            Nothing(port);
            break;
         case 'T': /*T3*/
            if (txbuf[5U]=='3') NumParm(port, 6U, 25U, 0U);
            else Nothing(port);
            break;
         case 'I': /*IP*/
            NumParm(port, 5U, 26U, 0U);
            break;
         default:;
            Err(port, txbuf[4U]);
            break;
         } /* end switch */
         break;
      default:;
         Err(port, txbuf[3U]);
         break;
      } /* end switch */
   }
   else {
      /*data*/
      SendInfo(port);
   }
} /* end TxCMD() */


static void ttypar(char fn[], uint32_t fn_len)
{
   struct termios term;
   int32_t fd;
   int32_t res;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenNONBLOCK(fn, fn_len);
   if (fd>=0L) {
      res = tcgetattr(fd, &term);
      /*
          term.c_lflag:=CAST(CARDINAL, CAST(BITSET,
                term.c_lflag) - CAST(BITSET, ECHO));
      */
      term.c_lflag = 0UL;
      term.c_iflag = 0UL;
      term.c_oflag = 0UL;
      res = tcsetattr(fd, 0L, &term);
   }
   /*
       res:=tcsetattr (fd, 0, term);
   */
   osic_Close(fd);
   X2C_PFREE(fn);
} /* end ttypar() */


static int32_t Opentty(char linkname[], uint32_t linkname_len)
{
   int32_t fd;
   char ok0;
   char ptsname[4096];
   int32_t Opentty_ret;
   X2C_PCOPY((void **)&linkname,linkname_len);
   fd = osi_OpenNONBLOCK("/dev/ptmx", 10ul);
   if (fd<0L) Error("/dev/ptmx open", 15ul);
   if (osi_getptsname(fd, (char *)ptsname, 4096UL)) {
      Error("get no ptty", 12ul);
   }
   if (verb) {
      osi_WrStr("ptty:", 6ul);
      osi_WrStrLn(ptsname, 4096ul);
   }
   if (osic_grantpts(fd)) Error("ptty grant", 11ul);
   if (osic_unlockpts(fd)) Error("ptty unlock", 12ul);
   if (verb) {
      osi_WrStr("unlock ", 8ul);
      osi_WrStrLn(ptsname, 4096ul);
   }
   ttypar(ptsname, 4096ul);
   /*make link*/
   osi_Erase(linkname, linkname_len, &ok0);
   if (osi_symblink((char *)ptsname, (char *)linkname)) {
      osi_WrStr("cannot create link <", 21ul);
      osi_WrStr(linkname, linkname_len);
      osi_WrStrLn(">, starting without hostmode interface", 39ul);
      osic_Close(fd);
      fd = -1L;
   }
   else if (verb) {
      osi_WrStr("make link to ", 14ul);
      osi_WrStrLn(linkname, linkname_len);
   }
   Opentty_ret = fd;
   X2C_PFREE(linkname);
   return Opentty_ret;
} /* end Opentty() */


static void SetComMode(int32_t fd, uint32_t baud0)
{
   struct termios term;
   int32_t res;
   uint32_t bd;
   struct termios * anonym;
   if (baud0==1200UL) bd = 9UL;
   else if (baud0==2400UL) bd = 11UL;
   else if (baud0==4800UL) bd = 12UL;
   else if (baud0==9600UL) bd = 13UL;
   else if (baud0==19200UL) bd = 14UL;
   else if (baud0==38400UL) bd = 15UL;
   else if (baud0==57600UL) bd = 4097UL;
   else if (baud0==115200UL) bd = 4098UL;
   else if (baud0==230400UL) bd = 4099UL;
   else if (baud0==460800UL) bd = 4100UL;
   else bd = 4098UL;
   res = tcgetattr(fd, &term);
   { /* with */
      struct termios * anonym = &term;
      anonym->c_lflag = 0UL;
      anonym->c_oflag = 0UL;
      anonym->c_iflag = 0UL;
      /*  cfmakeraw(&termios);*/
      anonym->c_cflag = 2224UL+bd; /*+CRTSCTS*/ /*0800018B2H*/
   }
   res = tcsetattr(fd, 2L, &term);
} /* end SetComMode() */


static int32_t Openrealtty(char fn[], uint32_t fn_len,
                uint32_t baud0)
{
   int32_t fd;
   int32_t Openrealtty_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenRW(fn, fn_len);
   /*  fd:=open(fn,  oRDWR+oNONBLOCK); */
   if (fd<0L) Error("/dev/tty open", 14ul);
   SetComMode(fd, baud0);
   if (verb) osi_WrStrLn("set baudrate", 13ul);
   Openrealtty_ret = fd;
   X2C_PFREE(fn);
   return Openrealtty_ret;
} /* end Openrealtty() */


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
   char h[1024];
   uint32_t i0;
   struct frameio_UDPSOCK * anonym;
   err = 0;
   sockc = 0UL;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            /* -I mycall */
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            GetCall(h, 1024ul, &i0, aprsstr_Length(h, 1024ul),
                (l2_pSTRING)maincall);
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            while ((h[i0] && h[i0]!=':') && i0<1023UL) {
               ttynamee[i0] = h[i0];
               ++i0;
            }
            ttynamee[i0] = 0;
            if (h[i0]) {
               ++i0;
               if (!GetNum(h, 1024ul, 0, &i0, &baud)) {
                  Error("need ttydevice:baud", 20ul);
               }
            }
         }
         else if (h[1U]=='U') {
            osi_NextArg(h, 1024ul);
            { /* with */
               struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[sockc];
               if (aprsstr_GetIp2(h, 1024ul, &anonym->ipnum, &anonym->toport,
                 &anonym->fromport, &anonym->checkip)<0L) {
                  Error("-U wrong ip:port:port number", 29ul);
               }
               anonym->fd = openudp();
               if (anonym->fd<0L || bindudp(anonym->fd,
                anonym->fromport)<0L) {
                  /* say with port */
                  Error("-U cannot open udp socket", 26ul);
               }
            }
            ++sockc;
         }
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -i <mycall>", 13ul);
               osi_WrStrLn(" -t <tty:baud>                     baud=0 is pseu\
do tty ", 57ul);
               osi_WrStrLn(" -U <x.x.x.x:destport:listenport>  axudp  /listen\
port not check ip", 67ul);
               osi_WrStrLn("                                   repeat -U for \
more ports", 60ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osic_WrLn();
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) Error(" use -h", 8ul);
   }
} /* end Parms() */

#define hostint_LOOPTIME 10000

static uint16_t i;

static uint32_t ts;

static uint32_t timeout;

static uint32_t timesum;

static uint32_t tntpanik;

static char ttybuf[512];

static int32_t len;

static int32_t bufp;

static int32_t ret;

static l2_CALLBACKPROC eventproc;

static uint16_t sockset;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   uint16_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   frameio_BEGIN();
   osi_BEGIN();
   aprsstr_BEGIN();
   l2_BEGIN();
   memset((char *)frameio_udpsocks0,(char)0,
                sizeof(struct frameio_UDPSOCK [15]));
   ttynamee[0] = 0;
   strncpy(maincall,"      0",7u);
   baud = 0UL;
   Parms();
   /*WrStr(":");WrStr(maincall);WrStrLn(":"); */
   for (i = 0U; i<=29U; i++) {
      { /* with */
         struct HANDLE * anonym = &handles[i];
         if (i==0U) anonym->events = 0x80U;
         else anonym->events = 0x40U;
         anonym->plink = 0;
         memcpy(anonym->mycall,maincall,7u);
      }
   } /* end for */
   for (i = 0U; i<=14U; i++) {
      strncpy(uipath[i],"TEST  p",71u);
      aprsstr_Append(uipath[i], 71ul, maincall, 7ul);
   } /* end for */
   maxlinks = 30U;
   nextevent = 0U;
   txpos = 0U;
   rxcount = 0U;
   rxpos = 0U;
   pmon = 0;
   moninfo = 0;
   timestamp = 1U;
   montyp = 1U;
   parmsport = 1U;
   eventproc = Event;
   sockset = 0U;
   tmp = (uint16_t)sockc;
   i = 0U;
   if (i<=tmp) for (;; i++) {
      sockset |= (1U<<i);
      if (i==tmp) break;
   } /* end for */
   l2_L2Init(1000U, sockset, eventproc);
   for (i = 0U; i<=29U; i++) {
      SetMy(i);
   } /* end for */
   if (baud==0UL) ttyfd = Opentty(ttynamee, 1024ul);
   else ttyfd = Openrealtty(ttynamee, 1024ul, baud);
   if (ttyfd<0L) Error("tty pipe open error", 20ul);
   bufp = 0L;
   len = 0L;
   timesum = 0UL;
   tntpanik = 0UL;
   for (;;) {
      fdclr();
      if (tntpanik<100UL) fdsetr((uint32_t)ttyfd);
      ts = 0UL;
      timeout = 10000UL;
      ret = selectrwt(&ts, &timeout);
      timesum += 10000UL-timeout;
      if (timesum>=10000UL) {
         l2_Layer2();
         timesum -= 10000UL;
      }
      if (issetr((uint32_t)ttyfd)) {
         ++tntpanik;
         len = osi_RdBin(ttyfd, (char *)ttybuf, 512u/1u, 512UL);
         if (len<0L) {
            usleep(10000UL);
            timesum += 10000UL;
         }
         bufp = 0L;
         while (bufp<len) {
            txbuf[txpos] = ttybuf[bufp];
            ++bufp;
            ++txpos;
            if (txpos>=3U && (uint32_t)txpos>=(uint32_t)(uint8_t)
                txbuf[2U]+4UL) {
               TxCMD();
               txpos = 0U;
            }
            if (rxcount>0U) {
               osi_WrBin(ttyfd, (char *)rxbuf, 260u/1u,
                (uint32_t)rxcount);
               rxcount = 0U;
            }
         }
      }
      else tntpanik = 0UL;
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
