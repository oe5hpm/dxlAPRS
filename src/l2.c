/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef l2_H_
#include "l2.h"
#endif
#define l2_C_
#ifndef frameio_H_
#include "frameio.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>

uint16_t l2_UA = 0x63U;
uint16_t l2_DM = 0xFU;
uint16_t l2_SABM = 0x2FU;
uint16_t l2_DISC = 0x43U;
uint16_t l2_FRMR = 0x87U;
uint16_t l2_UI = 0x3U;
uint16_t l2_RR = 0x1U;
uint16_t l2_REJ = 0x9U;
uint16_t l2_RNR = 0x5U;


char l2_L2Lock;
uint16_t l2_LostINT;
l2_pDATA l2_dbuf;
l2_AFIELD l2_adress;
uint16_t l2_asize;
char l2_l2verb;
uint32_t l2_dupchk;
char l2_udp2buf[100];


struct l2__D0 l2_PortSt[16];




struct l2__D2 l2_Parms[15];












/*
Interface Layer2 <-> Host:

Call L2Init;
Call (about) every 10ms Layer2

Call any Time
* not during Layer2 running and its Callback

  Connect     * (start a connect)

  Disconnect  * (stop a connect or close a handle with data)

  Circuit     (join 2 connections back to back)
              if reconn on, the host must not forget this host handle
              and continue interaction after reconnect

  GetAdress   (get address)

  Getudp2info (get axudp extention info from modem)

  GetChar     (get 1 received char of a connection, delete it or keep it)

  GetStr      (get more received chars of a connection)

  SendStr     (send data to a connection)

  SendRaw     (send UI or raw frame)

  GetStat     (get status of a connection)

  PortStat    (portwise statistics)

  Parm        (get or set port parameters)

  GetMon      (get monitor data)


Callback:
  contains A handle from layer2, is pointer to a (hidden) link structure.
           The host has to use this pointer in any call according to this link.             

           An event number

           A handle given from host to layer2, a pointer, layer2 looks only
           if NULL for "the host does not want this event and its link any longer"
                .

  Events:
    eCONNECTED: a connection (in or out) is esteblished
                if (from out) and the host is able to accept set the host-handle
                to not NULL. The handle will always be included in further
                callbacks for this link and the host MUST call Disconnect with
                dealloc TRUE to free this handle.

    eDISCONNECTED: a link ends, if there is a host-handle the host must 
                   call Disconnect with dealloc TRUE to free this handle.

    eBUSY: outgoing connection got busy (DM)
           if there is a host-handle the host must call disconnect to free this handle.

    eTXFREE: a connection is able to accept new data

    eRXDATA: a connection received new data

    eMONITOR: monitor frames are in buffer

    eCONNREQ:  a SABM (via digicall first in mycall table) arrived,
                this is only
               a message with no action of layer2, no host-handle and
               so no disconnect for freeing.
               the host has to find route,
                make a connect and if connect is esteblished
               make a "back connection"
                with Connect connect.typ cFLEXback or cCONNAK.
               If the destination connection is in same layer2,
                join the 2 connections
               with call to Circuit with both reconn off.
               if destination is away,
                host has to get data with GetStr and transport it
               any way and reverse send with SendStr 

    eFLEXDATA: a Frame via digicall (first in mycall table)
                arrived. this is only
               a message with no action of layer2, no host-handle and
               so no disconnect for freeing.
               retrive data immediately during callback vom adress/asize/dbuf
               route and resent with SendUI.  

    eLISTEN: not used by Layer2

    eIDLE: not used by Layer2

*/
/* (C)OE5DXL 1993-2015, GPL3 */
static uint16_t l2_SSIDMASK = 0x5EU;

static uint16_t l2_DAMARR = 0x20U;

#define l2_RESP 0

#define l2_FINAL 1

#define l2_COMMAND 2

#define l2_POLL 3

static uint16_t l2_NULL = 0x8000U;

static uint16_t l2_DAMAIDLE = 0x4000U;

#define l2_SENDQUES 3

#define l2_HASHSIZE 16
/*greater than number of ports*/

#define l2_MEMRR 20

#define l2_MEMTX 20

#define l2_MEMRX 10

#define l2_MEMSAMM 20

#define l2_MEMFRMR 1

#define l2_MEMLINK 5

#define l2_TILT 4

#define l2_IGNORE 8

#define l2_SHORTINFO 80

#define l2_IPRETRIES 10

#define l2_MAGIC 2091827346

#define l2_LOOPSPERS 100

#define l2_SECMOD 20000
/* timer base cycle range, max timer1 is half */

#define l2_TIMERBASE 10
/* timer base / layer2 looptime */

#define l2_T3OFF 65535

enum TXSTATE {l2_TWAIT, l2_TXDEL, l2_TXTAIL, l2_DCDWAIT, l2_ECHO};


typedef char * AFIELDPO;

struct LINK;


struct LINK {
   uint32_t Magic; /*magic number*/
   uint16_t State; /*0 = host is owner of the record*/
   uint16_t mycall; /*0=via, MAX(CARD16)=downlink*/
   uint16_t Baud; /*downlink baud*/
   uint16_t Uplinkport;
   uint8_t cCONNtyp; /*normal or link is for UI transfer only*/
   uint16_t IsInQue; /*link is in this send-que*/
   l2_pLINK SendQuenext; /*sendque-chain*/
   l2_pLINK SendQueprev;
   uint16_t Hash;
   l2_pLINK Hashnext; /*address-hash-chain*/
   l2_pLINK Hashprev;
   uint16_t Qtime; /*time since forced disc*/
   uint16_t T3out; /*Second for next T3 end or T3OFF for T1 running */
   uint16_t damasecond; /*last slice in absolut time*/
   uint16_t T1out; /*second when T1 will run out*/
   l2_pLINK Timernext; /*timer-chain*/
   l2_pLINK Timerprev;
   char highspeed; /*for prefered queing*/
   char damastop; /*no frames until dama-slot*/
   char twoway; /*the other is in INFO state too*/
   char discing; /*disc after send-info-buffer is empty*/
   char rejgot; /*got a REJ*/
   char rejsent; /*sent a REJ*/
   char infostop; /*send-info-buffer was empty*/
   char busyend; /*tell the other that i am not longer busy*/
   char flexback; /* flex backlink say no DM if SABM */
   char haschains; /* link has chains else simple dealloc */
   char rembusy; /*the other is busy*/
   uint16_t busy; /*0=not busy*/
   uint16_t retry; /*increments after timeout-polls*/
   uint16_t ns; /*send-sequence-number*/
   uint16_t lowsent; /*till here info is acknowled*/
   uint16_t highsent; /*till here -1 info is sent*/
   uint16_t sentnr; /*last physically sent nr*/
   uint16_t nr; /*receive sequence number*/
   char pid; /*PID for this link*/
   l2_pDATA sammler[8];
   uint16_t summs[8];
   l2_AFIELD Adress; /*adress field + command byte*/
   uint16_t AdrLen; /*length of adress + command byte*/
   uint16_t cp; /*send response, final, command or poll*/
   uint16_t sendcmd;
   l2_pDATA sendinfo; /*parallel to ns running info-pointer*/
   l2_pDATA txInfoIn; /*entry pointer to transmit buffer*/
   l2_pDATA txInfoOut; /*end of transmit data*/
   uint16_t txbuffers; /*used send-buffers*/
   char txfull; /*tx buffer full, for message or clearbusy*/
   char modifylast; /*last buffer not sent, so modifyable*/
   l2_pLINK nudel; /*the downlink, else NIL*/
   uint16_t exitch; /*switch linked links back to host*/
   char * host; /*to there the messages are sent*/
   char saiddisc; /*send only 1 disc message to host*/
   l2_pDATA rxInfoIn; /*entry pointer to receive buffer*/
   l2_pDATA rxInfoOut; /*end of receive data*/
   uint16_t rxbuffers; /*used buffers*/
   uint16_t readp; /*getchar readpointer*/
   l2_pDATA SABMinfo; /*extention, SABM with info*/
   /*statistics*/
   uint32_t SentBytes;
   uint32_t AckBytes;
   uint32_t RcvdBytes;
   uint32_t Started; /*for statistics: starttime of link*/
   char udp2info[31]; /* axudp2 frame infos from modem */
};

struct _0;

struct _1;


struct _1 {
   /*que 0 has highest priority*/
   l2_pLINK SendQueIn; /*chain of links that will send*/
   l2_pLINK SendQueOut;
};


struct _0 {
   uint8_t sendstate;
   char damat1;
   char dama; /*dama slave*/
   char dcd; /*all diversity ports set it*/
   uint16_t dcdguard; /*ignore dcd if it lasts too long*/
   uint16_t txwait; /*set while sending, reset when receiving*/
   uint16_t Ticker; /* change synchron to main clock */
   uint16_t QTicker; /* change only while not send or dcd */
   char lookinque; /*to decide quick if sendques are empty*/
   l2_pLINK damaloop;
   l2_pLINK T1In; /*timer-chain, each link has always a timer*/
   l2_pLINK T1Out;
   struct _1 SendQues[3];
};

static struct _0 PortL2[15];

static l2_pLINK AdrHash[16]; /*hashtable for calls*/

static l2_CALLTYP MyCalls[17]; /*mycall[0] is digipeat*/

static uint16_t maxrxbuffers;

static uint16_t maxtxbuffers;

static uint16_t DMTIME; /*save downlinks for sending DM*/

static uint16_t QTIME;

/*time for discing links with unackn. data*/
static uint16_t memory; /*used memory*/

static uint16_t DAMAMAXTIME; /*longest time till next slot*/

static uint16_t DAMAFASTPOLLS;

/*slots which have fast intervall*/
static uint32_t Time; /* seconds counting */

static uint32_t AbsTime;

static uint16_t Second;

static uint16_t monbufs;

static uint16_t maxmonbufs;

static uint16_t Minute5;

static uint16_t ticker5;

static char * * Freechain;

static l2_CALLTYP DNot[9];

static l2_CALLTYP mfrom;

static l2_pLINK monBufOut;

static l2_pLINK monBufIn;

static uint16_t hosts;

static uint32_t tick10;

static l2_CALLBACKPROC Event;


static uint16_t Min(uint16_t a, uint16_t b)
{
   if (a>=b) return b;
   else return a;
   return 0;
} /* end Min() */


static char Alloc(char * * p, uint16_t reserv)
{
   if (memory<=reserv || Freechain==0) return 0;
   *p = (char *)Freechain;
   Freechain = (char * *)(*Freechain);
   --memory;
   return 1;
} /* end Alloc() */


static void Dealloc(char * * p)
{
   char * * unnedig;
   /*WrInt(memory, 6); WrStrLn(" before dealloc "); */
   if (*p==0) return;
   unnedig = (char * *)*p;
   *unnedig = (char *)Freechain;
   Freechain = (char * *)*p;
   ++memory;
   *p = 0;
} /* end Dealloc() */


static void Unlock(void)
{
   if (l2_LostINT>0U) {
      l2_L2Lock = 1;
      l2_Layer2();
   }
   l2_L2Lock = 0;
} /* end Unlock() */


static uint16_t Minut5(void)
{
   return (uint16_t)((Time/300UL)%13UL);
} /* end Minut5() */


static void CheckParms(uint16_t port)
{
   struct l2__D2 * anonym;
   { /* with */
      struct l2__D2 * anonym = &l2_Parms[port-1U];
      if (anonym->_._0.MaxFrames>7U) anonym->_._0.MaxFrames = 7U;
      if (anonym->_._0.SendPort>15U) anonym->_._0.SendPort = port;
      if (anonym->_._0.Diversity>15U) anonym->_._0.Diversity = 0U;
      if (anonym->_._0.BaudSwBytes>100U) anonym->_._0.BaudSwBytes = 100U;
      if (anonym->_._0.T1==0U) anonym->_._0.T1 = 1U;
      else if (anonym->_._0.T1>=10000U) anonym->_._0.T1 = 10000U;
   }
} /* end CheckParms() */


static void call(l2_pPARMS p, l2_CALLTYP list[], uint32_t list_len)
{
   uint16_t k;
   uint16_t j;
   uint16_t i;
   uint16_t tmp;
   k = 0U;
   tmp = (uint16_t)((list_len-1)-1UL);
   i = 0U;
   if (i<=tmp) for (;; i++) {
      for (j = 0U; j<=6U; j++) {
         if (p->test) {
            p->str[k] = (char)((uint32_t)(uint8_t)list[i][j]/2UL);
         }
         else list[i][j] = (char)((uint32_t)(uint8_t)p->str[k]*2UL);
         ++k;
      } /* end for */
      if (i==tmp) break;
   } /* end for */
   if (p->test) p->str[k] = 0;
} /* end call() */


static void value(l2_pPARMS p, uint16_t * x)
{
   if (p->test) p->val = (uint32_t)*x;
   else *x = (uint16_t)p->val;
} /* end value() */


static void bool(l2_pPARMS p, char * b)
{
   if (p->test) p->val = (uint32_t)*b;
   else *b = p->val!=0UL;
} /* end bool() */


extern void l2_Parm(l2_pPARMS p)
{
   uint16_t i;
   struct l2_PARMS0 * anonym;
   struct l2__D2 * anonym0;
   { /* with */
      struct l2_PARMS0 * anonym = p;
      if (anonym->parm<=6U) anonym->port = 1U;
      else if (anonym->port==0U || anonym->port>15U) return;
      { /* with */
         struct l2__D2 * anonym0 = &l2_Parms[anonym->port-1U];
         switch (anonym->parm) {
         case 0U: /* set all mycalls, first is digi */
            call(p, MyCalls, 17ul);
            break;
         case 1U:
            call(p, DNot, 9ul);
            break;
         case 2U:
            X2C_MOVE((char *)anonym->str,(char *)mfrom,7UL);
            break;
         case 3U:
            value(p, &DMTIME);
            break;
         case 4U:
            value(p, &QTIME);
            break;
         case 5U:
            value(p, &maxrxbuffers);
            break;
         case 6U:
            value(p, &maxtxbuffers);
            break;
         case 7U:
            bool(p, &anonym0->Digipeat);
            break;
         case 8U:
            bool(p, &anonym0->HalfDuplex);
            break;
         case 9U:
            bool(p, &anonym0->DamaMaster);
            break;
         case 10U:
            bool(p, &anonym0->monitor);
            break;
         case 11U:
            bool(p, &anonym0->passall);
            break;
         case 12U:
            bool(p, &anonym0->Echo);
            break;
         case 13U:
            if (anonym->test) {
               X2C_MOVE((char *)anonym0->SendBauds,
                (char *)anonym->str,17UL);
            }
            else {
               for (i = 0U; i<=16U; i++) {
                  if ((uint8_t)anonym->str[i]>='0' && (uint8_t)
                anonym->str[i]<='7') anonym0->SendBauds[i] = anonym->str[i];
               } /* end for */
            }
            break;
         default:;
            anonym->parm -= 14U;
            if (anonym->parm<=16U) {
               value(p, &anonym0->_.card[anonym->parm]);
               CheckParms(anonym->port);
            }
            break;
         } /* end switch */
      }
   }
} /* end Parm() */


extern void l2_Getudp2info(l2_pLINK l, char s[], uint32_t s_len)
{
   uint16_t j;
   uint16_t i;
   s[0UL] = 0;
   if (((struct LINK *)l && ((struct LINK *)l)->udp2info[0U])
                && ((struct LINK *)l)->udp2info[1U]) {
      j = 0U;
      i = 2U;
      while ((uint32_t)j<s_len-1 && ((struct LINK *)l)->udp2info[i]) {
         s[j] = ((struct LINK *)l)->udp2info[i];
         ++j;
         ++i;
      }
      s[j] = 0;
   }
} /* end Getudp2info() */


extern void l2_GetAdress0(l2_pLINK l, l2_pGETADRESS p)
{
   uint16_t len;
   uint16_t i;
   struct l2_GETADRESS * anonym;
   struct LINK * anonym0;
   uint16_t tmp;
   { /* with */
      struct l2_GETADRESS * anonym = p;
      { /* with */
         struct LINK * anonym0 = (struct LINK *)l;
         anonym->port = anonym0->Uplinkport;
         anonym->my = anonym0->mycall;
         len = Min(71U, anonym0->AdrLen-1U);
         tmp = len;
         i = 0U;
         if (i<=tmp) for (;; i++) {
            anonym->adress[i] = (char)((uint32_t)(uint8_t)
                anonym0->Adress[i]/2UL);
            if (i==tmp) break;
         } /* end for */
         anonym->adress[len] = 0;
         if (anonym0->rxInfoOut && anonym0->rxInfoOut->len) {
            anonym->cpid = (uint16_t)(uint8_t)
                anonym0->rxInfoOut->info[0U];
         }
         else anonym->cpid = 65535U;
      }
   }
} /* end GetAdress() */


extern void l2_GetStat0(l2_pGETSTAT p)
{
   uint16_t v;
   uint16_t i;
   struct l2_GETSTAT * anonym;
   struct LINK * anonym0;
   uint16_t tmp;
   l2_L2Lock = 1;
   { /* with */
      struct l2_GETSTAT * anonym = p;
      if ((struct LINK *)anonym->l==0) {
         for (;;) {
            v = anonym->n/16U;
            anonym->l = AdrHash[anonym->n&15U];
            i = 0U;
            while ((struct LINK *)anonym->l && i<v) {
               ++i;
               anonym->l = ((struct LINK *)anonym->l)->Hashnext;
            }
            if ((struct LINK *)anonym->l==0) {
               anonym->n = anonym->n&15U;
               if (anonym->n==15U) {
                  anonym->n = 0U;
                  break;
               }
               ++anonym->n;
            }
            else {
               anonym->n += 16U;
               break;
            }
         }
      }
      if ((struct LINK *)anonym->l) {
         { /* with */
            struct LINK * anonym0 = (struct LINK *)anonym->l;
            anonym->st = anonym0->State;
            anonym->port = anonym0->Uplinkport;
            anonym->txbu = anonym0->txbuffers;
            anonym->my = anonym0->mycall;
            anonym->ret = anonym0->retry;
            anonym->bsy = anonym0->busy!=0U;
            anonym->remb = anonym0->rembusy;
            anonym->nudl = anonym0->nudel;
            tmp = Min(71U, anonym0->AdrLen);
            i = 0U;
            if (i<=tmp) for (;; i++) {
               anonym->adress[i] = (char)((uint32_t)(uint8_t)
                anonym0->Adress[i]/2UL);
               if (i==tmp) break;
            } /* end for */
            if (anonym0->AdrLen<=71U) anonym->adress[anonym0->AdrLen] = 0;
            anonym->sent = anonym0->SentBytes;
            anonym->ack = anonym0->AckBytes;
            anonym->rcvd = anonym0->RcvdBytes;
            anonym->since = Time-anonym0->Started;
         }
      }
      Unlock();
   }
} /* end GetStat() */


extern char l2_PortStat0(l2_pPORTSTAT p)
{
   uint16_t j;
   uint16_t i;
   struct l2_PORTSTAT * anonym;
   struct l2__D0 * anonym0;
   /*prev intervall*/
   struct l2__D1 * anonym1;
   uint16_t tmp;
   { /* with */
      struct l2_PORTSTAT * anonym = p;
      memset((char *)anonym->sums,(char)0,28UL);
      anonym->mem = memory;
      anonym->time0 = Time;
      if ((anonym->port>0U && anonym->port<=15U)
                && l2_Parms[anonym->port-1U]._._0.Diversity) {
         { /* with */
            struct l2__D0 * anonym0 = &l2_PortSt[anonym->port];
            j = Minut5(); /*aktual intervall*/
            tmp = anonym->minits5;
            i = 1U;
            if (i<=tmp) for (;; i++) {
               if (j==0U) j = 12U;
               else --j;
               { /* with */
                  struct l2__D1 * anonym1 = &anonym0->Flows[j];
                  anonym->sums[0U] += (uint32_t)anonym1->Igot;
                  anonym->sums[1U] += (uint32_t)anonym1->Isent;
                  anonym->sums[2U] += (uint32_t)anonym1->Iack;
                  anonym->sums[3U] += anonym1->Bytegot;
                  anonym->sums[4U] += anonym1->Bytesent;
               }
               if (i==tmp) break;
            } /* end for */
            anonym->sums[5U] = (uint32_t)anonym0->Links;
            anonym->sums[6U] = (uint32_t)anonym0->MaxLinks;
         }
      }
      else return 0;
      return 1;
   }
} /* end PortStat() */


static void ToSendQue(l2_pLINK l, uint16_t why, uint16_t c)
{
   /*for safety*/
   struct LINK * anonym;
   struct _1 * anonym0;
   if ((struct LINK *)l) {
      { /* with */
         struct LINK * anonym = (struct LINK *)l;
         anonym->sendcmd = why;
         anonym->cp = c;
         PortL2[anonym->Uplinkport-1U].lookinque = 1;
                /*only to find out quick if what in a que*/
         if (anonym->IsInQue>=3U && !anonym->damastop) {
            anonym->IsInQue = 0U;
            if (anonym->mycall==0U) anonym->IsInQue = 2U;
            else if ((!anonym->highspeed && why==0x8000U)
                && (anonym->sendinfo && anonym->sendinfo->len>80U || anonym->txInfoOut && anonym->txInfoOut->len>80U)
                ) anonym->IsInQue = 1U;
            { /* with */
               struct _1 * anonym0 = &PortL2[anonym->Uplinkport-1U]
                .SendQues[anonym->IsInQue];
               if ((struct LINK *)anonym0->SendQueOut==0) {
                  anonym0->SendQueOut = l;
               }
               else {
                  ((struct LINK *)l)->SendQueprev = anonym0->SendQueIn;
                  ((struct LINK *)anonym0->SendQueIn)->SendQuenext = l;
               }
               anonym0->SendQueIn = l;
            }
         }
      }
   }
} /* end ToSendQue() */


static l2_pDATA GetSendInfo(l2_pLINK l)
{
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if ((anonym->infostop || anonym->rembusy) || (((anonym->ns+9U)
                -anonym->lowsent&7U)
                >=l2_Parms[anonym->Uplinkport-1U]
                ._._0.MaxFrames || anonym->rejgot) && anonym->sendinfo) {
         return 0;
      }
      if (anonym->sendinfo==0) return anonym->txInfoOut;
      return anonym->sendinfo->next;
   }
} /* end GetSendInfo() */


static void Appendtx(l2_pLINK link, l2_pDATA data)
{
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)link;
      if ((((data->info[0U]=='\360' && anonym->txInfoOut) && data->len>1U)
                && anonym->modifylast)
                && anonym->txInfoIn->len+data->len<=258U) {
         /*F0 PID*/
         /*append to previous buffer*/
         X2C_MOVE((char *) &data->info[1U],
                (char *) &anonym->txInfoIn->info[anonym->txInfoIn->len],
                (uint32_t)(data->len-1U));
         anonym->txInfoIn->len += data->len-1U;
         Dealloc((char * *) &data);
         return;
      }
      data->next = 0;
      if (anonym->txInfoOut==0) anonym->txInfoOut = data;
      else anonym->txInfoIn->next = data;
      anonym->txInfoIn = data;
      ++anonym->txbuffers;
      anonym->modifylast = 1; /*this buffer was not sent till now*/
      if (anonym->IsInQue>=3U && GetSendInfo(link)) {
         PortL2[anonym->Uplinkport-1U]
                .damaloop = PortL2[anonym->Uplinkport-1U].T1Out;
                /*start a quick roundtrip*/
         ToSendQue(link, 0x8000U, 2U); /*schedule link to send info*/
      }
   }
} /* end Appendtx() */


static void CopyI(l2_pLINK from, l2_pLINK to, char reconn)
{
   l2_pDATA ip;
   struct LINK * anonym; /*copy rest of rx-info to sender*/
   struct l2_DFIELD * anonym0;
   { /* with */
      struct LINK * anonym = (struct LINK *)from;
      if (anonym->rxInfoOut) {
         { /* with */
            struct l2_DFIELD * anonym0 = anonym->rxInfoOut;
            if (anonym->readp && anonym0->len>anonym->readp) {
               /*shift info to begin of buffer*/
               X2C_MOVE((char *) &anonym0->info[anonym->readp+1U],
                (char *) &anonym0->info[1U],
                (uint32_t)((anonym0->len-anonym->readp)-1U));
               anonym0->len -= anonym->readp;
               anonym->readp = 0U;
            }
         }
         do {
            ip = anonym->rxInfoOut->next;
            Appendtx(to, anonym->rxInfoOut);
            --anonym->rxbuffers;
            anonym->rxInfoOut = ip;
         } while (anonym->rxInfoOut);
      }
      anonym->nudel = to;
      if (!reconn) {
         --hosts;
         anonym->Magic = 0UL;
         anonym->host = 0;
      }
   }
} /* end CopyI() */


extern void l2_Circuit(l2_pLINK * l1, l2_pLINK * l20, char reconn1,
                char reconn2, uint16_t exitchar)
{
   l2_L2Lock = 1;
   if (((((struct LINK *)*l1)->Magic==2091827346UL && ((struct LINK *)*l20)
                ->Magic==2091827346UL) && ((struct LINK *)*l1)->State)
                && ((struct LINK *)*l20)->State) {
      CopyI(*l1, *l20, reconn1);
      CopyI(*l20, *l1, reconn2);
      ((struct LINK *)*l1)->exitch = exitchar;
   }
   if (!reconn1) *l1 = 0;
   if (!reconn2) *l20 = 0;
   Unlock();
} /* end Circuit() */


extern char l2_SendStr(l2_pLINK l, uint16_t size, l2_pSTRING buf)
{
   l2_pDATA b;
   struct LINK * anonym;
   l2_L2Lock = 1;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if ((anonym->Magic==2091827346UL && anonym->State>=2U) && size<=256U) {
         if (size==0U) return !anonym->txfull;
         /* check if txbuffer full */
         if (((anonym->txInfoOut && anonym->modifylast)
                && anonym->txInfoIn->len+size<=256U)
                && anonym->cCONNtyp==l2_cNORMAL) {
            /*      IF (txInfoOut<>NIL)
                & modifylast & (txInfoIn^.len+size<=257) & (cCONNtyp=cNORMAL)
                 THEN */
            X2C_MOVE((char *)buf,
                (char *) &anonym->txInfoIn->info[anonym->txInfoIn->len],
                (uint32_t)size); /*append to previous buffer*/
            anonym->txInfoIn->len += size;
         }
         else if (anonym->txbuffers<=maxtxbuffers && Alloc((char * *)
                &b, 20U)) {
            b->info[0U] = anonym->pid;
            X2C_MOVE((char *)buf,(char *) &b->info[1U],
                (uint32_t)size);
            b->len = size+1U;
            Appendtx(l, b);
         }
         else {
            anonym->txfull = 1;
            Unlock();
            return 0;
         }
      }
   }
   Unlock();
   return 1;
} /* end SendStr() */


static void RemoveTimer(l2_pLINK l)
{
   struct _0 * anonym;
   { /* with */
      struct _0 * anonym = &PortL2[((struct LINK *)l)->Uplinkport-1U];
      if ((struct LINK *)anonym->T1Out==(struct LINK *)anonym->T1In) {
         anonym->T1Out = 0;
      }
      else if ((struct LINK *)l==(struct LINK *)anonym->T1Out) {
         anonym->T1Out = ((struct LINK *)l)->Timernext;
      }
      else if ((struct LINK *)l==(struct LINK *)anonym->T1In) {
         anonym->T1In = ((struct LINK *)l)->Timerprev;
      }
      else {
         ((struct LINK *)((struct LINK *)l)->Timerprev)
                ->Timernext = ((struct LINK *)l)->Timernext;
         ((struct LINK *)((struct LINK *)l)->Timernext)
                ->Timerprev = ((struct LINK *)l)->Timerprev;
      }
   }
} /* end RemoveTimer() */


static void StartTimer(l2_pLINK l)
{
   /*WrStrLn("<T1>"); */
   struct LINK * anonym;
   struct _0 * anonym0;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      { /* with */
         struct _0 * anonym0 = &PortL2[anonym->Uplinkport-1U];
         anonym->T1out = ((anonym0->QTicker+l2_Parms[anonym->Uplinkport-1U]
                ._._0.T1)-1U)%20000U; /*next timeout*/
         /*WrInt(T1out, 5); WrStr(":tout1 "); */
         if ((struct LINK *)anonym0->T1Out==0) anonym0->T1Out = l;
         else {
            if ((anonym->T1out-((struct LINK *)anonym0->T1In)->T1out)
                %20000U>10000U) {
               anonym->T1out = ((struct LINK *)anonym0->T1In)->T1out;
            }
            /*WrInt(T1out, 5); WrStr(":tout2 "); */
            anonym->Timerprev = anonym0->T1In;
            ((struct LINK *)anonym0->T1In)->Timernext = l;
         }
         /*append to que*/
         anonym0->T1In = l;
      }
   }
} /* end StartTimer() */


static void MakePoll(l2_pLINK linkp)
{
   uint16_t cmd;
   uint16_t r;
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)linkp;
      if (anonym->State>=2U && anonym->cCONNtyp==l2_cNORMAL) {
         r = l2_Parms[anonym->Uplinkport-1U]._._0.Retries;
         if (anonym->State!=5U || !anonym->twoway) r = r/4U+1U;
         ++anonym->retry;
         if (anonym->retry>r) {
            /*WrStrLn("state1"); */
            anonym->State = 1U;
         }
         /*retried out*/
         cmd = 0x43U;
         if (anonym->State==5U) cmd = 0x1U;
         else if (anonym->State==2U) cmd = 0x2FU;
         ToSendQue(linkp, cmd, 3U);
      }
      else if (anonym->State==2U && (anonym->cCONNtyp==l2_cFLEXback || anonym->cCONNtyp==l2_cCONNAK)
                ) {
         anonym->State = 5U;
         cmd = 0x63U;
         anonym->cCONNtyp = l2_cNORMAL;
         ToSendQue(linkp, 0x63U, 1U);
      }
      else if (anonym->State==2U && anonym->cCONNtyp==l2_cFLEXbusy) {
         /*WrStrLn("pollflexbusy"); */
         anonym->State = 1U;
         cmd = 0xFU;
         ToSendQue(linkp, 0xFU, 1U);
      }
   }
} /* end MakePoll() */


static void ClearBusy(l2_pLINK l)
{
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if (anonym->busy && anonym->State==5U) {
         anonym->busy = 0U;
         anonym->busyend = 1; /*until the other knows*/
         ToSendQue(l, 0x1U, 0U);
         RemoveTimer(l);
         anonym->T3out = (Second+1U)%20000U; /*start minimal T3*/
         StartTimer(l);
      }
   }
} /* end ClearBusy() */


static void Disc(l2_pLINK l)
{
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if (anonym->cCONNtyp) anonym->State = 1U;
      else if (anonym->discing || anonym->State!=5U) {
         /*do it quick*/
         if (anonym->State>=2U) ToSendQue(l, 0x43U, 3U);
         anonym->State = 1U;
      }
      else if (anonym->txInfoOut==0) {
         anonym->State = 4U; /*disc state*/
         anonym->retry = 0U;
         RemoveTimer(l);
         anonym->T3out = 65535U; /*start T1*/
         StartTimer(l);
         MakePoll(l); /*DISC ...*/
      }
      else {
         anonym->discing = 1; /*first send all infos*/
         anonym->Qtime = 0U;
      }
   }
} /* end Disc() */


extern void l2_Disconnect(l2_pLINK * l, char dealloc)
{
   l2_pDATA ip;
   struct LINK * anonym;
   /*WrInt(memory, 6); WrInt(hosts, 6); WrInt(l^.State, 6);
                WrInt(ORD(dealloc),1); WrStrLn(" flexdisc "); */
   l2_L2Lock = 1;
   if (((struct LINK *)*l)->Magic==2091827346UL) {
      if (((struct LINK *)*l)->State>=2U) Disc(*l);
      if (dealloc) {
         /*host gives link back to l2*/
         --hosts;
         { /* with */
            struct LINK * anonym = (struct LINK *)*l;
            while (anonym->rxInfoOut) {
               /*deallocate rx-info*/
               ip = anonym->rxInfoOut->next;
               Dealloc((char * *) &anonym->rxInfoOut);
               anonym->rxInfoOut = ip;
            }
            anonym->Magic = 0UL;
            /*WrStr("L2: state:"); WrInt(State, 5); WrStrLn(""); */
            if (anonym->State==0U) Dealloc((char * *)l);
            else {
               /*WrStrLn("l2:dealloc"); */
               anonym->host = 0;
            }
         }
         *l = 0;
      }
   }
   Unlock();
} /* end Disconnect() */


static void Cut(l2_pLINK l)
{
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if ((struct LINK *)anonym->nudel) {
         ((struct LINK *)anonym->nudel)->nudel = 0; /*remove pointer to me*/
         if (((struct LINK *)anonym->nudel)->host==0) Disc(anonym->nudel);
         else ClearBusy(anonym->nudel);
         anonym->nudel = 0;
      }
   }
} /* end Cut() */


static void DisposeLink(l2_pLINK l, char immediate)
{
   l2_pDATA ip;
   uint16_t i;
   /*throw away bufferd data*/
   struct LINK * anonym;
   /*remove link from sendque*/
   struct LINK * anonym0;
   struct _1 * anonym1;
   /*WrStrLn("L2: disposelink"); */
   if ((struct LINK *)l) {
      /*for safety*/
      { /* with */
         struct LINK * anonym = (struct LINK *)l;
         for (i = 0U; i<=7U; i++) {
            Dealloc((char * *) &anonym->sammler[i]);
         } /* end for */
         while (anonym->txInfoOut) {
            ip = anonym->txInfoOut->next;
            Dealloc((char * *) &anonym->txInfoOut);
            anonym->txInfoOut = ip;
         }
         Dealloc((char * *) &anonym->SABMinfo);
         /*send the (sad or lucky) message to host*/
         if (anonym->host && !anonym->saiddisc) {
            /*do it only once*/
            /*WrStrLn(" say disc/busy"); */
            if (anonym->State==2U) {
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym->host, l,
                l2_eBUSY);
            }
            else {
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym->host, l,
                l2_eDISCONNECTED);
            }
            anonym->saiddisc = 1;
         }
         Cut(l); /*disconnect nudelport*/
      }
      /*WrStr("dispo:");WrInt(ORD(immediate),2); WrInt(l^.mycall,2);
                WrStrLn(""); */
      /*    IF immediate OR (l^.mycall<>MAX(CARD16))
                THEN   (*not a downlink*) */
      if (immediate || !((struct LINK *)l)->flexback) {
         /*not a downlink*/
         { /* with */
            struct LINK * anonym0 = (struct LINK *)l;
            if (anonym0->IsInQue<3U) {
               { /* with */
                  struct _1 * anonym1 = &PortL2[anonym0->Uplinkport-1U]
                .SendQues[anonym0->IsInQue];
                  if ((struct LINK *)anonym1->SendQueOut==(struct LINK *)
                anonym1->SendQueIn) anonym1->SendQueOut = 0;
                  else if ((struct LINK *)l==(struct LINK *)
                anonym1->SendQueOut) {
                     anonym1->SendQueOut = anonym0->SendQuenext;
                  }
                  else if ((struct LINK *)l==(struct LINK *)
                anonym1->SendQueIn) {
                     anonym1->SendQueIn = anonym0->SendQueprev;
                  }
                  else {
                     ((struct LINK *)anonym0->SendQueprev)
                ->SendQuenext = anonym0->SendQuenext;
                     ((struct LINK *)anonym0->SendQuenext)
                ->SendQueprev = anonym0->SendQueprev;
                  }
               }
            }
            /*remove link from hashchain*/
            if ((struct LINK *)l==(struct LINK *)AdrHash[anonym0->Hash]) {
               AdrHash[anonym0->Hash] = anonym0->Hashnext;
            }
            else {
               ((struct LINK *)anonym0->Hashprev)
                ->Hashnext = ((struct LINK *)l)->Hashnext;
               if ((struct LINK *)anonym0->Hashnext) {
                  ((struct LINK *)anonym0->Hashnext)
                ->Hashprev = anonym0->Hashprev;
               }
            }
         }
         RemoveTimer(l);
         --l2_PortSt[((struct LINK *)l)->Uplinkport].Links;
         /*now the link has no relation with L2,
                so give it to host or deallocate*/
         if (((struct LINK *)l)->host) {
            /*WrStrLn("state12"); */
            ((struct LINK *)l)->State = 0U;
         }
         else {
            /*WrStrLn("disposedealloc"); */
            ((struct LINK *)l)->Magic = 0UL;
            Dealloc((char * *) &l);
         }
      }
   }
/*WrStr("disposelink mem=");WrInt(memory,1); */
/*WrStr(" hosts=");WrInt(hosts,1); */
/*WrStrLn(""); (*for tests*) */
} /* end DisposeLink() */


static void TestMem(l2_pLINK l)
{
   struct LINK * anonym;
   if (memory>=20U) {
      { /* with */
         struct LINK * anonym = (struct LINK *)l;
         if ((struct LINK *)anonym->nudel) {
            if (anonym->txbuffers+4U<memory/4U) {
               ClearBusy(anonym->nudel);
               anonym->txfull = 0;
            }
         }
         else if (anonym->host && anonym->txbuffers<=maxtxbuffers) {
            X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym->host, l, l2_eTXFREE);
            anonym->txfull = 0;
         }
      }
   }
} /* end TestMem() */


static void StartT3(l2_pLINK l)
{
   ((struct LINK *)l)->T3out = (Second+l2_Parms[((struct LINK *)l)
                ->Uplinkport-1U]._._0.T3)%20000U;
} /* end StartT3() */


static uint16_t T3resttime(l2_pLINK l)
{
   short th;
   if (((struct LINK *)l)->T3out==65535U) return 0U;
   th = (short)((struct LINK *)l)->T3out-(short)Second;
   if (th<0) th += 20000;
   if (th>10000) return 0U;
   /* T3 ran out */
   return (uint16_t)th;
} /* end T3resttime() */


static void Timeout(l2_pLINK l)
{
   struct LINK * anonym;
   /*WrInt(Qtime, 6);WrInt(State, 6);WrInt(memory, 6);
                WrStr(":qtime/state/mem "); */
   struct _0 * anonym0; /*remove timer from end of que*/
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if (anonym->State<=1U && anonym->Qtime>DMTIME) {
         DisposeLink(l, 1);
         return;
      }
      { /* with */
         struct _0 * anonym0 = &PortL2[anonym->Uplinkport-1U];
         if ((struct LINK *)anonym0->T1Out==(struct LINK *)anonym0->T1In) {
            anonym0->T1Out = 0;
         }
         else anonym0->T1Out = ((struct LINK *)anonym0->T1Out)->Timernext;
      }
      StartTimer(l);
      if (anonym->State>=2U) {
         if (anonym->T3out!=65535U) {
            if (anonym->discing && anonym->State==5U) {
               if (anonym->Qtime<QTIME) ++anonym->Qtime;
               else anonym->State = 4U;
            }
            if (T3resttime(l)==0U) anonym->T3out = 65535U;
         }
      }
      else {
         /*lifetimer for state 1 downlink*/
         ++anonym->Qtime;
         StartT3(l);
      }
      /*WrStrLn(" incq "); */
      if (l2_Parms[anonym->Uplinkport-1U].DamaMaster) {
         anonym->damastop = 0; /*link now is in its slot*/
         if (anonym->sendcmd!=0x4000U) {
            /*there is a frame to be sent*/
            ToSendQue(l, anonym->sendcmd, anonym->cp);
                /*damastop is over, so its qued*/
            anonym->damasecond = Second; /*last slot in absolute time*/
            return;
         }
         if ((anonym->State==5U && anonym->T3out!=65535U) && ((T3resttime(l)
                <=DAMAFASTPOLLS || (PortL2[anonym->Uplinkport-1U].QTicker+1U)
                %20000U==Second) || ((Second+20000U)-anonym->damasecond)
                %20000U>=DAMAMAXTIME)) {
            /*& damabit*/
            /*its a dama slot*/
            /*activ link*/
            /*port has still idle times*/
            ToSendQue(l, 0x1U, 2U); /*enable the other to send now*/
            anonym->damasecond = Second; /*store last absolute slot time*/
         }
      }
      if (anonym->txfull) TestMem(l);
      if (anonym->T3out==65535U || anonym->State<=4U) MakePoll(l);
   }
} /* end Timeout() */


extern char l2_GetChar(l2_pLINK l, char delete0,
                char * char0)
{
   l2_pDATA ip;
   char ok0;
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      while (anonym->rxInfoOut) {
         ok0 = anonym->rxInfoOut->len>1U;
         *char0 = anonym->rxInfoOut->info[anonym->readp+1U];
         if (!delete0 && ok0) return 1;
         if (anonym->Magic!=2091827346UL) return 0;
         ++anonym->readp;
         if (anonym->readp+1U<anonym->rxInfoOut->len) return 1;
         anonym->readp = 0U;
         l2_L2Lock = 1;
         ip = anonym->rxInfoOut->next;
         Dealloc((char * *) &anonym->rxInfoOut);
         anonym->rxInfoOut = ip;
         --anonym->rxbuffers;
         if (anonym->rxbuffers<=maxrxbuffers && memory>=20U) ClearBusy(l);
         Unlock();
         if (ok0) return 1;
      }
   }
   return 0;
} /* end GetChar() */


extern uint16_t l2_GetStr(l2_pLINK l, uint16_t size, l2_pSTRING buf)
{
   l2_pDATA ip;
   uint16_t n;
   uint16_t retsize;
   struct LINK * anonym;
   retsize = 0U;
   { /* with */
      struct LINK * anonym = (struct LINK *)l;
      if (anonym->Magic==2091827346UL) {
         while (retsize<size && anonym->rxInfoOut) {
            n = anonym->rxInfoOut->len-anonym->readp;
            if (n>1U) {
               --n; /*pid*/
               if (n+retsize>size) n = size-retsize;
               X2C_MOVE((char *)
                &anonym->rxInfoOut->info[anonym->readp+1U],
                (char *) &buf[retsize],(uint32_t)n);
               retsize += n;
               anonym->readp += n;
            }
            if (anonym->readp+1U>=anonym->rxInfoOut->len) {
               anonym->readp = 0U;
               l2_L2Lock = 1;
               ip = anonym->rxInfoOut->next;
               Dealloc((char * *) &anonym->rxInfoOut);
               anonym->rxInfoOut = ip;
               --anonym->rxbuffers;
               if (anonym->rxbuffers<=maxrxbuffers && memory>=20U) {
                  ClearBusy(l);
               }
               Unlock();
            }
         }
      }
   }
   return retsize;
} /* end GetStr() */


static uint16_t CP(void)
{
   return (uint16_t)(2UL*(uint32_t)((uint8_t)l2_adress[6U]>=(uint8_t)
                '\200')+(uint32_t)((0x10U & (uint16_t)(uint32_t)
                (uint8_t)l2_adress[l2_asize-1U])!=0));
} /* end CP() */
/*p/f bit*/
/*command bit*/

#define l2_SUMLEN 5


static void Appendrx(l2_pLINK linkp, l2_pDATA b)
{
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)linkp;
      ++l2_PortSt[anonym->Uplinkport].Flows[Minute5].Igot;
      anonym->RcvdBytes = anonym->RcvdBytes+(uint32_t)b->len;
      if ((uint32_t)(uint8_t)b->info[1U]==(uint32_t)
                anonym->exitch && anonym->host) Cut(linkp);
      if ((struct LINK *)anonym->nudel) {
         Appendtx(anonym->nudel, b);
         if (((struct LINK *)anonym->nudel)->txbuffers+4U>memory/4U) {
            ++anonym->busy;
            ((struct LINK *)anonym->nudel)->txfull = 1;
         }
      }
      else if (anonym->host) {
         b->next = 0;
         if (anonym->rxInfoOut==0) anonym->rxInfoOut = b;
         else anonym->rxInfoIn->next = b;
         anonym->rxInfoIn = b;
         ++anonym->rxbuffers;
         if (anonym->rxbuffers>=maxrxbuffers) ++anonym->busy;
         if (anonym->rxbuffers==1U) {
            X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym->host, linkp,
                l2_eRXDATA);
         }
      }
      else Dealloc((char * *) &b);
   }
} /* end Appendrx() */


static char Ack(l2_pLINK linkp, uint16_t n, char resend)
{
   l2_pDATA dp;
   /*ackowlege my sent info*/
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)linkp;
      if (((anonym->highsent+8U)-anonym->lowsent&7U)<((n+8U)
                -anonym->lowsent&7U)) {
         ToSendQue(linkp, 0x87U, 0U); /*acknowledges never sended info*/
         return 0;
      }
      anonym->twoway = 1;
      while (anonym->lowsent!=n) {
         /*delete acknowledged from sendbuffer*/
         if (anonym->txInfoOut) {
            /*only for safety*/
            if (anonym->ns==anonym->lowsent) resend = 1;
            dp = anonym->txInfoOut->next;
            anonym->AckBytes = anonym->AckBytes+(uint32_t)
                anonym->txInfoOut->len;
            ++l2_PortSt[anonym->Uplinkport].Flows[Minute5].Iack;
            Dealloc((char * *) &anonym->txInfoOut);
            anonym->txInfoOut = dp;
            --anonym->txbuffers;
         }
         else return 0;
         /*dramatic internal error*/
         anonym->lowsent = anonym->lowsent+1U&7U;
      }
      if (resend) {
         /*all sent acknolled or resend info*/
         anonym->ns = n;
         anonym->sendinfo = 0;
         anonym->retry = 0U;
      }
      if (resend || anonym->rembusy) {
         anonym->infostop = 0;
         StartT3(linkp); /*restart T3*/
      }
      if (anonym->IsInQue>=3U && GetSendInfo(linkp)) {
         ToSendQue(linkp, 0x8000U, 2U); /*schedule for sending next info*/
         PortL2[anonym->Uplinkport-1U]
                .damaloop = PortL2[anonym->Uplinkport-1U].T1Out;
                /*start quick roundtrip*/
      }
      if (anonym->txfull) TestMem(linkp);
      if (anonym->discing && anonym->txInfoOut==0) {
         anonym->State = 4U;
         MakePoll(linkp); /*DISC ...*/
      }
   }
   return 1;
} /* end Ack() */


static void Uselink(l2_pLINK linkp)
{
   uint16_t compoll;
   uint16_t i;
   uint16_t sum;
   uint16_t n;
   l2_pDATA ip;
   int32_t j;
   uint16_t cmd;
   char wasbusy;
   char slave;
   struct LINK * anonym;
   int32_t tmp;
   /*analyse (addressed to me) frames and reply*/
   /*WrStrLn("uselink"); */
   { /* with */
      struct LINK * anonym = (struct LINK *)linkp;
      slave = !l2_Parms[anonym->Uplinkport-1U].DamaMaster && ((uint16_t)
                (uint32_t)(uint8_t)l2_adress[13U]&0x20U)==0U;
      cmd = 0xEFU&(uint16_t)(uint32_t)(uint8_t)l2_adress[l2_asize-1U];
                /*command field without p/f-bit*/
      compoll = CP(); /*response, final, command or poll*/
      if (anonym->cCONNtyp==l2_cNORMAL) {
         if (anonym->T3out!=65535U) StartT3(linkp);
         if ((cmd&0x1U)==0U) {
            /*I-frame                         */
            if (anonym->State==5U) {
               /*info transfer                   */
               n = (uint16_t)cmd/2U&7U;
                /*send sequence number            */
               if (Ack(linkp, (uint16_t)cmd/32U&7U, slave)) {
                  /*acknols my frames*/
                  if (anonym->State==5U) {
                     /*maybe disc after all acknolled  */
                     i = l2_dbuf->len;
                     if (i>5U) i = 5U;
                     sum = 0U;
                     tmp = (int32_t)i-1L;
                     j = 0L;
                     if (j<=tmp) for (;; j++) {
                        sum += (uint16_t)(uint8_t)l2_dbuf->info[j];
                        if (j==tmp) break;
                     } /* end for */
                     if (n==anonym->nr && anonym->busy<8U) {
                        /*I-frame has expected number*/
                        if ((n+9U&7U)!=anonym->sentnr) {
                           /*check windowsize  */
                           anonym->summs[n] = sum; /*mark it as old    */
                           ip = anonym->sammler[n];
                           if (ip || Alloc((char * *) &ip, 10U)) {
                              /*do not say rnr with an*/
                              Appendrx(linkp, l2_dbuf);
                /*empty downlink buffer,*/
                              l2_dbuf = ip; /*noone would clear this*/
                              anonym->sammler[n] = 0;
                              for (;;) {
                                 anonym->nr = anonym->nr+1U&7U;
                                 if (anonym->sammler[anonym->nr]==0) {
                                    /*no fitting frames in sammler*/
                                    anonym->rejsent = anonym->sammler[anonym->nr+1U&7U]
                !=0; /*a hole in sammler?*/
                                    break;
                /*so request only 1 I-frame by REJ if 1 is missing*/
                                 }
                                 Appendrx(linkp,
                anonym->sammler[anonym->nr]);
                /*frame from sammler is in sequence*/
                                 anonym->sammler[anonym->nr] = 0;
                              }
                           }
                        }
                        else {
                           anonym->State = 1U; /*got 8 frames!*/
                           ToSendQue(linkp, 0x87U, 0U);
                        }
                     }
                     else {
                        if (anonym->summs[n]!=sum) {
                           /*frame is from aktual window*/
                           anonym->summs[n] = sum;
                /*so put it into sammler     */
                           ip = anonym->sammler[n];
                           if (ip || Alloc((char * *) &ip, 20U)) {
                              anonym->sammler[n] = l2_dbuf;
                              l2_dbuf = ip;
                           }
                        }
                        anonym->rejsent = compoll!=3U;
                /*say not rej on i-poll      */
                     }
                     ToSendQue(linkp, 0x1U, (uint16_t)(compoll==3U));
                /*make final on poll    */
                  }
               }
            }
            else if (compoll==3U && anonym->State!=2U) {
               /*WrStrLn("state13"); */
               anonym->State = 1U;
               ToSendQue(linkp, 0xFU, 1U);
            }
         }
         else if ((cmd&0x3U)==0x1U) {
            /*R?R-Frame, REJ with RNR is allowed*/
            wasbusy = anonym->rembusy;
            anonym->rembusy = (0x4U & cmd)!=0; /*remote busy  */
            anonym->rejgot = (0x8U & cmd)!=0; /*REJ          */
            if (compoll==3U) anonym->busyend = 0;
            /*      IF wasbusy & NOT rembusy THEN T3out:=T3OFF END;
                restart T1   */
            if (anonym->State!=5U || Ack(linkp, (uint16_t)cmd/32U&7U,
                ((slave || compoll==1U) || anonym->rejgot)
                || wasbusy && !anonym->rembusy)) {
               if (compoll==3U) {
                  /*R?R.Poll     */
                  if (anonym->State==5U) ToSendQue(linkp, 0x1U, 1U);
                  else if (anonym->State!=2U) {
                     /*WrStrLn("state14"); */
                     anonym->State = 1U; /*the other is polling*/
                     ToSendQue(linkp, 0xFU, 1U);
                  }
               }
            }
         }
         else if (cmd==0x2FU && compoll==3U) {
            /*only SABM.Poll accepted*/
            Dealloc((char * *) &anonym->SABMinfo);
            if ((anonym->State!=2U && anonym->mycall==65535U)
                && anonym->twoway) {
               /*mycall=MAX is a downlink*/
               /*WrStrLn("state15"); */
               anonym->State = 1U;
               if (anonym->flexback) DisposeLink(linkp, 1);
               else ToSendQue(linkp, 0xFU, 1U);
            }
            else if (anonym->State==2U || anonym->State==1U && Alloc((char * *)
                 &ip, 0U)) {
               if (anonym->host) --hosts;
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym->host, linkp,
                l2_eCONNECTED);
               if (anonym->host) ++hosts;
               if (anonym->State==1U) {
                  /*connect comes from outside*/
                  if (anonym->host && l2_dbuf->len) {
                     Appendrx(linkp, l2_dbuf);
                     l2_dbuf = ip;
                  }
                  else Dealloc((char * *) &ip);
               }
               if (anonym->host) {
                  /*host accepts that connect*/
                  anonym->State = 5U;
                  anonym->Magic = 2091827346UL;
                  ToSendQue(linkp, 0x63U, 1U);
               }
               else {
                  /*WrStrLn("state16"); */
                  anonym->State = 1U;
                  ToSendQue(linkp, 0xFU, 1U);
                /*got no host, say busy or nothing if host needs time */
               }
            }
            else if (anonym->State<=4U || anonym->twoway) {
               anonym->State = 1U;
                /*linkreset, wait for new link. this SABM is lost*/
               /*WrStrLn("dispose3"); */
               DisposeLink(linkp, 0); /*a downlink will say DM*/
            }
            else {
               ToSendQue(linkp, 0x63U, 1U);
               RemoveTimer(linkp);
               anonym->T3out = 65535U; /*restart T1*/
               StartTimer(linkp); /*test if the other has a receiver*/
               anonym->infostop = 1; /*before resending the first I-frames*/
            }
         }
         else if (cmd==0x63U && compoll==1U) {
            /*only UA.Final because i send SABM.P*/
            Dealloc((char * *) &anonym->SABMinfo);
            if (anonym->State==2U) {
               if (anonym->host) --hosts;
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym->host, linkp,
                l2_eCONNECTED);
               if (anonym->host) {
                  /*host is still alive*/
                  ++hosts;
                  anonym->State = 5U;
                  anonym->twoway = 1; /*both are in state 5 now*/
                  anonym->Magic = 2091827346UL; /*...a magic feeling*/
                  anonym->retry = 0U;
                  if (anonym->txInfoOut==0) {
                     StartT3(linkp); /*stop T1 if no info to send*/
                  }
                  else anonym->T3out = 65535U;
               }
               else {
                  /*WrStrLn("state17"); */
                  anonym->State = 1U;
               }
            }
            else if (anonym->State!=5U) {
               /*host died after it started Conn*/
               anonym->State = 1U;
               /*WrStrLn("dispose4"); */
               DisposeLink(linkp, anonym->twoway);
                /*UA in twoway must be a reply of DISC*/
            }
         }
         else if (cmd==0x43U || cmd==0x87U) {
            if (anonym->State!=2U) {
               /*ignor discs while connecting*/
               /*WrStrLn("state18"); */
               anonym->State = 1U;
               ToSendQue(linkp, 0xFU, 1U);
            }
         }
         else if (cmd==0xFU) {
            /*else ignore DISCs and go ahead with SABM*/
            if (compoll==1U) {
               /*WrStrLn("dispose5"); */
               DisposeLink(linkp, 1);
            }
         }
         else if (cmd==0x3U) {
            /*ignore not final*/
            if (compoll==3U && anonym->State==1U) {
               ToSendQue(linkp, 0xFU, 1U);
            }
         }
         else ToSendQue(linkp, 0x87U, 0U);
      }
      else if ((anonym->cCONNtyp==l2_cFLEXback || anonym->cCONNtyp==l2_cCONNAK)
                 || anonym->cCONNtyp==l2_cFLEXbusy) {
         /*
             ELSIF cmd=UI THEN
               IF (State=5) & (dbuf^.len#0) & ((cCONNtyp=cUI)
                OR (cCONNtyp=cUIpoll)) THEN
                 IF Alloc(ip,MEMRR) THEN
                   Appendrx(dbuf);
                   dbuf:=ip;
                 END;
               END;
         */
         /*WrStrLn("uselink back"); */
         if (cmd==0x2FU && compoll==3U) {
            /*only SABM.Poll accepted*/
            Dealloc((char * *) &anonym->SABMinfo);
            /*WrStrLn("dispose15"); */
            DisposeLink(linkp, 1);
         }
         else {
            /*WrStrLn("state1fb"); */
            anonym->State = 1U;
            ToSendQue(linkp, 0xFU, 1U);
         }
      }
      if (slave) {
         PortL2[anonym->Uplinkport-1U].dama = 1;
         if ((cmd&0x1U)!=0U || compoll==3U) {
            /*poll or not info*/
            PortL2[anonym->Uplinkport-1U].dcd = 0;
            PortL2[anonym->Uplinkport-1U].damat1 = 1;
         }
      }
   }
} /* end Uselink() */

#define l2_WILD "T"


static void Mon(uint16_t port, l2_AFIELD adr, uint16_t alen,
                l2_pDATA info)
{
   uint16_t i;
   l2_pLINK lp;
   struct LINK * anonym;
   if (mfrom[0U]) {
      i = 0U;
      while (i<=5U && (mfrom[i]=='T' || mfrom[i]==adr[i])) ++i;
      if (i<6U) {
         for (i = 0U; i<=5U; i++) {
            if (mfrom[i]!='T' && mfrom[i]!=adr[i+7U]) return;
         } /* end for */
      }
   }
   if (monbufs<maxmonbufs && Alloc((char * *) &lp, 5U)) {
      { /* with */
         struct LINK * anonym = (struct LINK *)lp;
         memset((char *)lp,(char)0,sizeof(struct LINK)); /*State:=0;
                readp:=0*/
         anonym->rxInfoOut = 0;
         anonym->Magic = 2091827346UL;
         memcpy(anonym->Adress,adr,71u);
         anonym->AdrLen = alen;
         anonym->mycall = (uint16_t)(uint8_t)adr[alen-1U];
         anonym->Uplinkport = port;
         i = 0U; /* axuudp2 info to monitor */
         do {
            anonym->udp2info[i] = l2_udp2buf[i];
            ++i;
         } while (!((i>=30U || i>=99U) || l2_udp2buf[i]==0));
         anonym->udp2info[i] = 0;
         if (info && Alloc((char * *) &anonym->rxInfoOut, 0U)) {
            *anonym->rxInfoOut = *info;
            anonym->rxInfoOut->next = 0;
            anonym->rxbuffers = 1U;
         }
         anonym->Hashnext = 0;
         if ((struct LINK *)monBufOut==0) monBufOut = lp;
         else ((struct LINK *)monBufIn)->Hashnext = lp;
         monBufIn = lp;
         ++monbufs;
      }
   }
} /* end Mon() */


extern l2_pLINK l2_GetMon(void)
{
   l2_pLINK l;
   l2_L2Lock = 1;
   l = monBufOut;
   if ((struct LINK *)monBufOut) {
      monBufOut = ((struct LINK *)monBufOut)->Hashnext;
      --monbufs;
      ++hosts;
   }
   Unlock();
   return l;
} /* end GetMon() */


static void SendBufferd(uint16_t port, uint16_t baud, uint16_t cmd)
/*send a frame with no link*/
{
   l2_pLINK linkp;
   struct LINK * anonym;
   /*succeeds only if there is enough memory*/
   if (Alloc((char * *) &linkp, 6U)) {
      memset((char *)linkp,(char)0,sizeof(struct LINK));
      { /* with */
         struct LINK * anonym = (struct LINK *)linkp;
         anonym->IsInQue = 3U;
         anonym->mycall = 0U; /*dispose link after sending*/
         anonym->Uplinkport = port;
         anonym->State = 1U;
         anonym->Baud = baud;
         anonym->damastop = 0;
         memcpy(anonym->Adress,l2_adress,71u);
                /*useing static address field*/
         anonym->AdrLen = l2_asize;
         if (l2_dbuf->len) {
            anonym->sendinfo = l2_dbuf;
            if (!Alloc((char * *) &l2_dbuf, 5U)) {
               Dealloc((char * *) &linkp); /*so forget it all*/
            }
         }
         else anonym->sendinfo = 0;
         if ((struct LINK *)linkp) ToSendQue(linkp, cmd, 3U);
      }
   }
} /* end SendBufferd() */


extern void l2_SendRaw(l2_pSENDUI p)
{
   struct l2_SENDUI * anonym;
   struct l2_DFIELD * anonym0;
   l2_L2Lock = 1;
   { /* with */
      struct l2_SENDUI * anonym = p;
      if ((anonym->port>0U && anonym->port<=15U)
                && l2_Parms[anonym->port-1U]._._0.Diversity>0U) {
         l2_asize = 0U;
         while (l2_asize<70U && anonym->path[l2_asize]) {
            l2_adress[l2_asize] = (char)((uint32_t)(uint8_t)
                anonym->path[l2_asize]*2UL+(uint32_t)
                (anonym->path[l2_asize+1U]==0));
            ++l2_asize;
         }
         l2_adress[l2_asize] = anonym->cmd;
         ++l2_asize;
         { /* with */
            struct l2_DFIELD * anonym0 = l2_dbuf;
            anonym0->len = anonym->datalen;
            if (anonym0->len>257U) anonym0->len = 257U;
            X2C_MOVE((char *)anonym->data,(char *)anonym0->info,
                (uint32_t)anonym0->len);
         }
         SendBufferd(anonym->port, anonym->baud, 0x8000U);
      }
   }
   Unlock();
} /* end SendRaw() */


static char Initlink(l2_pLINK * linkp, uint16_t hash,
                uint16_t Uport, uint16_t baud)
{
   struct l2__D0 * anonym; /*statistics*/
   struct LINK * anonym0;
   if (Alloc((char * *)linkp, 6U)) {
      memset((char *)*linkp,(char)0,sizeof(struct LINK));
      { /* with */
         struct l2__D0 * anonym = &l2_PortSt[Uport];
         ++anonym->Links;
         if (anonym->Links>anonym->MaxLinks) {
            anonym->MaxLinks = anonym->Links;
         }
      }
      { /* with */
         struct LINK * anonym0 = (struct LINK *)*linkp;
         anonym0->Hash = hash;
         if ((struct LINK *)AdrHash[anonym0->Hash]==0) {
            anonym0->Hashnext = 0; /*append link to hashque*/
         }
         else {
            anonym0->Hashnext = AdrHash[anonym0->Hash];
            ((struct LINK *)AdrHash[anonym0->Hash])->Hashprev = *linkp;
         }
         AdrHash[anonym0->Hash] = *linkp;
         anonym0->Baud = baud;
         anonym0->IsInQue = 3U; /*not in sendque*/
         anonym0->Uplinkport = Uport;
         anonym0->pid = '\360';
         anonym0->Started = Time;
         anonym0->T3out = 65535U; /*start T1*/
         StartTimer(*linkp); /*each link has a timer till its disposed*/
         anonym0->haschains = 1;
      }
      return 1;
   }
   return 0;
} /* end Initlink() */


static void Hashf(uint16_t * hash, uint16_t port)
{
   uint16_t i;
   *hash = 0U;
   for (i = 0U; i<=5U; i++) {
      *hash += (uint16_t)((uint32_t)(uint8_t)l2_adress[i]+(uint32_t)
                (uint8_t)l2_adress[i+7U]); /*hash function*/
   } /* end for */
   *hash =  *hash/2U+port&15U;
} /* end Hashf() */


static char CallComp(uint16_t p, AFIELDPO adr, uint16_t pp)
{
   uint16_t i;
   for (i = 5U;; i--) {
      if (l2_adress[p+i]!=adr[pp+i]) return 0;
      if (i==0U) break;
   } /* end for */
   return ((uint16_t)(uint32_t)(uint8_t)l2_adress[p+6U]&0x5EU)
                ==((uint16_t)(uint32_t)(uint8_t)adr[pp+6U]&0x5EU);
} /* end CallComp() */


extern l2_pLINK l2_Connect0(char * mp, l2_pCONNECT p)
/*NIL if no link*/
{
   l2_pLINK linkp;
   uint16_t hash;
   l2_pDATA sabmi;
   struct l2_CONNECT * anonym;
   struct l2_DFIELD * anonym0;
   struct LINK * anonym1;
   { /* with */
      struct l2_CONNECT * anonym = p;
      if ((anonym->port==0U || anonym->port>15U)
                || l2_Parms[anonym->port-1U]._._0.Diversity==0U) return 0;
      l2_L2Lock = 1;
      l2_asize = 0U; /*uses static adress buffer, so not reentrant*/
      while (l2_asize<70U && anonym->l2adr[l2_asize]) {
         l2_adress[l2_asize] = (char)((uint32_t)(uint8_t)
                anonym->l2adr[l2_asize]*2UL+(uint32_t)
                (anonym->l2adr[l2_asize+1U]==0));
         ++l2_asize;
      }
      ++l2_asize;
      Hashf(&hash, l2_Parms[anonym->port-1U]._._0.Diversity);
      linkp = AdrHash[hash];
      for (;;) {
         /*look if a link from-to same calls exists*/
         if ((struct LINK *)linkp==0) break;
         if (CallComp(0U, (AFIELDPO)((struct LINK *)linkp)->Adress,
                0U) && CallComp(7U, (AFIELDPO)((struct LINK *)linkp)->Adress,
                 7U)) {
            if (anonym->typ==l2_cTEST && ((struct LINK *)linkp)->State>1U) {
               Unlock(); /*link exists*/
               return linkp;
            }
            /*WrStrLn("dispose6"); */
            DisposeLink(linkp, 1); /*and let the old one disappeare*/
            break;
         }
         linkp = ((struct LINK *)linkp)->Hashnext;
      }
      if (anonym->typ==l2_cTEST) {
         Unlock();
         return 0;
      }
      /*extention, SABM with info*/
      sabmi = 0;
      if (anonym->l3adr && anonym->l3adr[0U]) {
         if (Alloc((char * *) &sabmi, 5U)) {
            { /* with */
               struct l2_DFIELD * anonym0 = sabmi;
               anonym0->len = 0U;
               while (anonym0->len<=256U && anonym->l3adr[anonym0->len]) {
                  anonym0->info[anonym0->len] = anonym->l3adr[anonym0->len];
                  ++anonym0->len;
               }
            }
         }
         else {
            Unlock();
            return 0;
         }
      }
      if (Initlink(&linkp, hash, anonym->port,
                (uint16_t)((uint32_t)(uint8_t)
                l2_Parms[anonym->port-1U].SendBauds[0U]-48UL))) {
         { /* with */
            struct LINK * anonym1 = (struct LINK *)linkp;
            anonym1->mycall = 65535U; /*is a downlink*/
            anonym1->host = mp;
            anonym1->Magic = 2091827346UL;
            anonym1->pid = (char)anonym->cpid;
            anonym1->cCONNtyp = anonym->typ;
            anonym1->State = 2U;
            anonym1->T3out = 65535U; /*start T1*/
            if (anonym->typ==l2_cFLEXback || anonym1->cCONNtyp==l2_cCONNAK) {
               anonym1->flexback = anonym->typ==l2_cFLEXback;
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym1->host, linkp,
                l2_eCONNECTED);
               StartT3(linkp); /*restart T3*/
            }
            /*link setup*/
            anonym1->SABMinfo = sabmi;
            anonym1->AdrLen = l2_asize;
            memcpy(anonym1->Adress,l2_adress,71u);
            if (l2_Parms[anonym->port-1U].DamaMaster) {
               anonym1->Adress[13U] = (char)((uint16_t)(uint32_t)
                (uint8_t)anonym1->Adress[13U]&~0x20U);
            }
            MakePoll(linkp); /*SABM ...*/
            if (anonym1->host) ++hosts;
         }
         Unlock();
         return linkp;
      }
      else {
         if (sabmi) Dealloc((char * *) &sabmi);
         Unlock();
         return 0;
      }
   }
   return 0;
} /* end Connect() */


static void CopyCall(l2_pLINK linkp, uint16_t to, uint16_t from)
/*and clean ssid from additional bits*/
{
   uint16_t i;
   struct LINK * anonym;
   { /* with */
      struct LINK * anonym = (struct LINK *)linkp;
      for (i = 0U; i<=5U; i++) {
         anonym->Adress[to+i] = l2_adress[from+i];
      } /* end for */
      anonym->Adress[to+6U] = (char)((uint16_t)(uint32_t)(uint8_t)
                l2_adress[from+6U]&0x7EU);
   }
} /* end CopyCall() */


static void invertpath(l2_pLINK linkp, char master, char flex)
{
   uint16_t j;
   uint16_t i;
   struct LINK * anonym;
   CopyCall(linkp, 0U, 7U); /*invert direction*/
   CopyCall(linkp, 7U, 0U);
   { /* with */
      struct LINK * anonym = (struct LINK *)linkp;
      if (master) {
         anonym->Adress[13U] = (char)((uint16_t)(uint32_t)(uint8_t)
                anonym->Adress[13U]&~0x20U); /*set master bit*/
      }
      i = 14U; /*invert digi-path*/
      j = l2_asize-8U;
      while (j>=14U) {
         CopyCall(linkp, i, j);
         i += 7U;
         j -= 7U;
      }
      if (flex) {
         anonym->Adress[20U] = (char)((uint8_t)(uint8_t)
                anonym->Adress[20U]|0x80U); /* set h-bit for pseudodigi */
      }
      anonym->Adress[i-1U] = (char)((uint32_t)(uint8_t)
                anonym->Adress[i-1U]+1UL); /*set address-end-bit*/
      anonym->AdrLen = i+1U;
   }
} /* end invertpath() */


static void rx(uint16_t Port)
/*analyse address-field*/
{
   uint16_t hash;
   uint16_t j;
   uint16_t i;
   l2_pLINK linkp;
   char poll;
   char v2;
   char sabm;
   char digisdone;
   char isflex;
   struct LINK lh;
   struct l2__D1 * anonym;
   struct LINK * anonym0;
   /*memory avaliable*/
   struct LINK * anonym1;
   /*memory avaliable*/
   struct LINK * anonym2;
   { /* with */
      struct l2__D1 * anonym = &l2_PortSt[Port].Flows[Minute5];
      anonym->Bytegot = anonym->Bytegot+(uint32_t)l2_asize+(uint32_t)
                l2_dbuf->len+3UL; /* + fcs + 1 flag */
   }
   digisdone = l2_asize<22U || (uint8_t)l2_adress[l2_asize-2U]>=(uint8_t)
                '\200'; /* no digis or last with h-bit */
   v2 = (uint8_t)l2_adress[6U]>=(uint8_t)'\200'==(uint8_t)
                l2_adress[13U]<(uint8_t)'\200';
   poll = CP()==3U;
   sabm = (v2 && poll) && (0xEFU&(uint16_t)(uint32_t)(uint8_t)
                l2_adress[l2_asize-1U])==0x2FU; /* SABM.P v2.0 */
   isflex = (!digisdone && (l2_asize<29U || (uint8_t)
                l2_adress[l2_asize-9U]>=(uint8_t)'\200'))
                && CallComp(l2_asize-8U, (AFIELDPO)MyCalls[0U], 0U);
                /* min 1 digi & last h-bit not set */
   /* 1 digi or second last h-bit set */
   /* my digi ask host for flexnet link */
   /*first look in existing links*/
   Hashf(&hash, l2_Parms[Port-1U]._._0.Diversity);
   /*WrInt(hash, 10); WrStrLn(":hash "); */
   linkp = AdrHash[hash];
   /*WrInt(CAST(CARDINAL, linkp), 15); WrStrLn(":po "); */
   for (;;) {
      if ((struct LINK *)linkp==0) break;
      { /* with */
         struct LINK * anonym0 = (struct LINK *)linkp;
         if (CallComp(0U, (AFIELDPO)anonym0->Adress, 7U) && CallComp(7U,
                (AFIELDPO)anonym0->Adress, 0U)) {
            i = 14U;
            j = l2_asize-8U; /*all pseudodigis are compared too*/
            while (((j>=14U && (uint8_t)anonym0->Adress[i+6U]>=(uint8_t)
                '\200') && (uint8_t)l2_adress[j+6U]<(uint8_t)'\200')
                && CallComp(j, (AFIELDPO)anonym0->Adress, i)) {
               i += 7U;
               j -= 7U;
            }
            if (j<14U || (uint8_t)anonym0->Adress[i+6U]<(uint8_t)
                '\200' && (uint8_t)l2_adress[j+6U]>=(uint8_t)'\200') {
               break;
            }
         }
         linkp = anonym0->Hashnext;
                /*there may be more entries with same hash*/
      }
   }
   if ((struct LINK *)linkp) {
      /*existing link*/
      if (v2) Uselink(linkp);
   }
   else if (digisdone && poll) {
      /* new link */
      i = 1U;
      for (;;) {
         if (MyCalls[i][0U]==0) break;
         if (CallComp(0U, (AFIELDPO)MyCalls[i], 0U) && Initlink(&linkp, hash,
                 Port, (uint16_t)((uint32_t)(uint8_t)l2_Parms[Port-1U].SendBauds[i]-48UL))) {
            /*fitting mycall found*/
            { /* with */
               struct LINK * anonym1 = (struct LINK *)linkp;
               anonym1->mycall = i;
               anonym1->State = 1U;
               invertpath(linkp, l2_Parms[Port-1U].DamaMaster, 0);
               if (sabm) {
                  lh = *(struct LINK *)linkp;
                  /*WrStrLn("dispose1"); */
                  DisposeLink(linkp, 1);
                /* dispose before same flex backlink does it */
                  X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym1->host,
                (l2_pLINK) &lh, l2_eCONNREQ);
               }
               else {
                  /*
                            StartT3(linkp);
                (*start with T3*)
                            IF sabm THEN Uselink(linkp);
                (*now it is an existing link*)
                  */
                  ToSendQue(linkp, 0xFU, 1U);
                /* say DM.F to any poll to mycall */
               }
            }
            break;
         }
         ++i;
      }
   }
   else if (isflex) {
      if (Initlink(&linkp, hash, Port, 0U)) {
         { /* with */
            struct LINK * anonym2 = (struct LINK *)linkp;
            anonym2->mycall = 0U;
            anonym2->State = 1U;
            invertpath(linkp, l2_Parms[Port-1U].DamaMaster, 1);
            lh = *(struct LINK *)linkp;
            /*WrStrLn("dispose1"); */
            DisposeLink(linkp, 1);
                /* dispose before same flex backlink does it */
            if (sabm) {
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym2->host,
                (l2_pLINK) &lh, l2_eCONNREQ);
                /* call router to make link later */
            }
            else {
               X2C_CHKPROC(l2_CALLBACKPROC,Event)(&anonym2->host,
                (l2_pLINK) &lh, l2_eFLEXDATA);
                /* give router frame to digipeat */
            }
         }
      }
   }
/*

    (*else if poll bit look for uplinks to a mycall*)
  ELSIF (asize<=15) OR (adress[asize-2]>=200C)
                OR isflex THEN  (* no more digis in path or h-bits set *)
--WrStrLn(" newlink ");
    i:=1;
    LOOP
      IF NOT isflex & (MyCalls[i][0]=0C) THEN EXIT END;
                (*end of mycall-table*)

      IF isflex OR CallComp(0,ADR(MyCalls[i]),
                0) THEN          (*fitting mycall found*)
        IF (CP()=POLL)                             (*only polls a interesting*)
        & ((adress[6]>=200C)=(adress[13]<200C))    (*is v2*)
        & Initlink(linkp,hash,Port,
                ORD(Parms[Port].SendBauds[i])-ORD("0"))
                THEN (*memory avaliable*)
          WITH linkp^ DO
            mycall:=i; 
            IF isflex THEN mycall:=0 END;          (* flex *)
            State:=1;
            CopyCall(0,7);                         (*invert direction*)
            CopyCall(7,0);
            IF Parms[Port].DamaMaster THEN
              Adress[13]:=CAST(CHAR, CAST(SET16, ORD(Adress[13]))-DAMARR);
                (*set master bit*)
            END;
            i:=14;                                 (*invert digi-path*)
            j:=asize-8;
            WHILE j>=14 DO CopyCall(i,j); INC(i,7); DEC(j,7) END;
            IF isflex THEN Adress[20]:=CAST(CHAR, CAST(SET8,
                Adress[20])+SET8{7}) END; (* set h-bit for pseudodigi *)
            Adress[i-1]:=CHR(ORD(Adress[i-1])+1);  (*set address-end-bit*)
            AdrLen:=i+1;
            IF isflex THEN
              lh:=linkp^;
--WrStrLn("dispose1");
              DisposeLink(linkp, TRUE);
                (* dispose before same flex backlink does it *) 
              IF flexsabm THEN Event(host, ADR(lh), eCONNREQ)
              ELSE Event(host, ADR(lh), eFLEXDATA) END;
            ELSE
              StartT3(linkp);                      (*start with T3*)
              Uselink(linkp);                      (*now it is an existing link*)
            END;
          END;
        END;
        EXIT

      END;
      INC(i);
    END;

  (*at least look for digipeating*)
  ELSIF Parms[Port].Digipeat & (asize>=15) THEN
    i:=20;
    LOOP
      IF adress[i]<200C THEN                           (*no h-bit*)
        IF CallComp(i-6,ADR(MyCalls[0]),0) THEN        (*is my digicall*)
          j:=0;
          LOOP
            IF DNot[j][0]=0C THEN                      (*end of dnot-table*)
              adress[i]:=CHR(ORD(adress[i])+128);      (*set h-bit*)
--WrStrLn("sendbuf");
              SendBufferd(Port,ORD(Parms[Port].SendBauds[0])-ORD("0"),NULL);
              EXIT

            END;
            k:=0;
            WHILE DNot[j][k]=adress[k+7] DO
              IF k=5 THEN EXIT END;

              INC(k);
            END;
            INC(j);
          END;
        END;
        EXIT

      END;
      INC(i,7);
      IF i>asize THEN EXIT END;

    END;
  END;
*/
} /* end rx() */


static void tx(l2_pLINK * SendQueIn, l2_pLINK * SendQueOut)
{
   l2_pLINK last;
   l2_pDATA sendi;
   char dealloc;
   char isent;
   struct LINK * anonym;
   struct l2_DFIELD * anonym0;
   /*WrStr("[");WrHex(CAST(CARD16,sendcmd), 1);WrStrLn("]"); */
   struct l2__D1 * anonym1;
   struct LINK * anonym2;
   /*WrStrLn("tx"); */
   while ((struct LINK *)*SendQueOut) {
      { /* with */
         struct LINK * anonym = (struct LINK *)*SendQueOut;
         sendi = 0;
         isent = 0;
         dealloc = 0;
         if (anonym->mycall) {
            if (anonym->State==5U && anonym->sendcmd==0x8000U || (anonym->sendcmd==0x1U && anonym->cp!=1U)
                 && !anonym->busyend) {
               if (anonym->cCONNtyp) {
                  if (anonym->sendcmd==0x8000U) {
                     sendi = GetSendInfo(*SendQueOut);
                     if (sendi) {
                        anonym->sendcmd = 0x3U;
                        dealloc = 1;
                        anonym->txInfoOut = anonym->txInfoOut->next;
                        --anonym->txbuffers;
                        if (anonym->txfull) TestMem(*SendQueOut);
                     }
                  }
                  else anonym->sendcmd = 0x8000U;
               }
               else if (((((anonym->cp==3U && anonym->sendcmd==0x1U)
                && anonym->sendinfo)
                && anonym->sendinfo->len<=l2_Parms[anonym->Uplinkport-1U]
                ._._0.IPoll) && !anonym->rembusy) && anonym->retry<=10U) {
                  /*& ((ns+9) MOD 8<>highsent)*/
                  anonym->sendcmd = (uint16_t)
                (2U*anonym->ns+32U*anonym->nr); /*I.P instead of RR.P*/
                  anonym->sentnr = anonym->nr;
                /*only for later check of windowsize*/
                  sendi = anonym->sendinfo;
               }
               else {
                  /*RR may be sent as I*/
                  sendi = GetSendInfo(*SendQueOut);
                  if (sendi) {
                     /*there is info to send*/
                     if (sendi==anonym->txInfoIn) anonym->modifylast = 0;
                     ++l2_PortSt[anonym->Uplinkport].Flows[Minute5].Isent;
                     anonym->SentBytes = anonym->SentBytes+(uint32_t)
                sendi->len;
                     if (anonym->sendinfo) anonym->ns = anonym->ns+1U&7U;
                     anonym->sendinfo = sendi;
                     anonym->sendcmd = (uint16_t)
                (2U*anonym->ns+32U*anonym->nr); /*I-frame*/
                     anonym->sentnr = anonym->nr;
                /*only for later check of windowsize*/
                     if (l2_Parms[anonym->Uplinkport-1U]
                .DamaMaster && GetSendInfo(*SendQueOut)==0) {
                        anonym->cp = 3U;
                        anonym->damastop = 1;
                     }
                     else anonym->cp = 2U;
                     if (anonym->ns==anonym->highsent) {
                        anonym->highsent = anonym->highsent+1U&7U;
                     }
                     isent = 1;
                  }
               }
            }
            else if (anonym->sendcmd==0x2FU) {
               sendi = anonym->SABMinfo; /*extention, SABM with info*/
            }
            else if (anonym->sendcmd==0x87U) {
               if (Alloc((char * *) &sendi, 1U)) {
                  { /* with */
                     struct l2_DFIELD * anonym0 = sendi;
                     anonym0->info[0U] = anonym->pid;
                     anonym0->info[1U] = 0;
                /*may someone make it correct...*/
                     anonym0->info[2U] = (char)anonym->nr;
                     anonym0->info[3U] = (char)anonym->ns;
                     anonym0->len = 4U;
                  }
                  dealloc = 1;
               }
               anonym->State = 1U;
            }
         }
         else {
            sendi = anonym->sendinfo;
            dealloc = sendi!=0;
         }
         if (anonym->sendcmd!=0x8000U) {
            if (anonym->sendcmd==0x1U) {
               anonym->sendcmd = (uint16_t)((uint32_t)(uint16_t)
                anonym->sendcmd+4UL*(uint32_t)(anonym->busy!=0U)
                +8UL*(uint32_t)(anonym->rejsent && anonym->busy==0U)
                +(uint32_t)(32U*anonym->nr));
               anonym->sentnr = anonym->nr;
                /*only for later check of windowsize*/
            }
            anonym->Adress[anonym->AdrLen-1U] = (char)((uint32_t)
                (uint16_t)anonym->sendcmd+16UL*(uint32_t)(char)
                (anonym->cp&1)); /*p/f bits*/
            anonym->Adress[6U] = (char)(128UL*(uint32_t)
                (anonym->cp>=2U)+((uint32_t)(uint8_t)
                anonym->Adress[6U]&127UL));
            anonym->Adress[13U] = (char)(128UL*(uint32_t)
                (anonym->cp<2U)+((uint32_t)(uint8_t)
                anonym->Adress[13U]&127UL));
         }
         if (anonym->mycall==0U || anonym->sendcmd!=0x8000U) {
            frameio_SendFrame((uint32_t)
                l2_Parms[anonym->Uplinkport-1U]._._0.SendPort,
                (uint32_t)anonym->Baud, anonym->Adress, 71ul,
                (uint32_t)anonym->AdrLen, sendi);
            /*WrStr("F:"); */
            /*IF sendi<>NIL THEN WrInt(sendi^.len,1) END; */
            { /* with */
               struct l2__D1 * anonym1 = &l2_PortSt[anonym->Uplinkport]
                .Flows[Minute5];
               anonym1->Bytesent = anonym1->Bytesent+(uint32_t)
                anonym->AdrLen+3UL; /* + fcs and 1 flag*/
               if (sendi) {
                  anonym1->Bytesent = anonym1->Bytesent+(uint32_t)
                sendi->len;
               }
            }
            if (l2_Parms[anonym->Uplinkport-1U].monitor) {
               Mon(l2_Parms[anonym->Uplinkport-1U]._._0.SendPort,
                anonym->Adress, anonym->AdrLen, sendi);
            }
         }
         if (dealloc) Dealloc((char * *) &sendi);
         anonym->IsInQue = 3U;
         last = *SendQueOut;
         if ((struct LINK *)*SendQueOut==(struct LINK *)*SendQueIn) {
            *SendQueOut = 0; /*remove link from sendque*/
         }
         else *SendQueOut = ((struct LINK *)*SendQueOut)->SendQuenext;
      }
      { /* with */
         struct LINK * anonym2 = (struct LINK *)last;
         if (!anonym2->haschains) {
            Dealloc((char * *) &last);
                /*simple dealloc, there are no chains*/
         }
         else {
            anonym2->sendcmd = 0x4000U;
            if (anonym2->State==5U) {
               if (GetSendInfo(last)==0) {
                  /*no more info to send*/
                  if (isent) {
                     /*but info sended*/
                     RemoveTimer(last);
                     anonym2->T3out = 65535U; /*so restart T1*/
                     StartTimer(last);
                  }
                  /*infostop:=Parms[Uplinkport].HalfDuplex;*/
                  anonym2->damastop = l2_Parms[anonym2->Uplinkport-1U]
                .DamaMaster;
               }
               else {
                  ToSendQue(last, 0x8000U, 2U);
                /*reschedule for more I-frames*/
               }
            }
            else if (anonym2->State<=1U) DisposeLink(last, 0);
         }
      }
   }
} /* end tx() */


static void inctime(void)
{
   /* make always forward going time */
   uint32_t t;
   uint32_t i;
   t = osic_time();
   if (AbsTime!=t) {
      AbsTime = t;
      ++Time;
      Minute5 = Minut5();
      if (ticker5!=Minute5) {
         /*clear next statistics intervall*/
         ticker5 = Minute5;
         for (i = 1UL; i<=15UL; i++) {
            memset((char *) &l2_PortSt[i].Flows[Minute5],(char)0,
                sizeof(struct l2__D1));
         } /* end for */
      }
   }
   ++tick10;
   if (tick10%10UL==0UL) Second = (Second+1U)%20000U;
} /* end inctime() */

/*
PROCEDURE Layer2;                       (*started by ticker*)
VAR port,i:CARD16;
    norx:BOOLEAN;
BEGIN
(*
  IF LostINT>5 THEN LostINT:=5 END;
(*IF NOT L2Lock THEN
    L2Lock:=TRUE; *)                    (*L2-interrupt must not be reentered!*)
*)
    norx:=FALSE;
    REPEAT
      FOR port:=1 TO PORTS DO
        WITH Parms[port] DO
          WITH PortL2[port] DO
            IF Diversity<>0 THEN                              (*port is enabled*)

              IF HalfDuplex THEN
                IF dama THEN
                  IF dcd THEN
                    IF dcdguard<DCDIGNOR THEN INC(dcdguard)
                ELSE dama:=FALSE END;
                  ELSE dcdguard:=0 END;
                ELSIF DCD(port) THEN
                  IF dcdguard<DCDIGNOR THEN
                    PortL2[Diversity].dcd:=TRUE;
                    INC(dcdguard);
                  END; 
                ELSE dcdguard:=0 END;
              END;
              IF GetFrame(port) THEN
                IF monitor & (crcok OR passall) THEN Mon(port,adress,asize,
                dbuf) END;
                IF crcok THEN
                  IF MhTime<>0 THEN MH(port,adress,asize) END;
                  IF PortL2[SendPort].sendstate=TWAIT THEN PortL2[SendPort]
                .txwait:=0 END;

                  rx(Diversity);

                  (*dcdguard:=0;*)
                END;
              ELSE norx:=TRUE END;

            END;

            IF Diversity=port THEN                    (*this ports has a sender*)
              IF DamaMaster THEN

                IF (T1Out<>NIL) & NOT dcd & (PortL2[SendPort].txwait=0) THEN
                  Timeout(T1Out);
                END;
              ELSIF Ticker<>Second THEN
                IF (NOT HalfDuplex OR (PortL2[SendPort].txwait=0)
                & (NOT dcd OR damat1)) THEN (*timer is running*)

                  IF (T1Out=NIL) OR (T1Out^.T1out<>QTicker) THEN
                    QTicker:=(QTicker+1) MOD SECMOD;
                (*no timer in this second*)
                  ELSE Timeout(T1Out) END;
                  IF damat1 & (Ticker=Second) THEN damat1:=FALSE END;
                END;
                Ticker:=Second;   
              END;

              IF txwait<>0 THEN
                IF (sendstate=TXDEL) OR NOT Sending(port) THEN DEC(txwait)
                END;
                IF txwait=0 THEN
                  IF sendstate=TXDEL THEN txwait:=TXtail; sendstate:=TXTAIL;
                  ELSIF (sendstate=TXTAIL) OR (sendstate=ECHO) THEN
--                    StopFlags(port);
                    IF HalfDuplex THEN
                      txwait:=TXwait;
(*
                      IF T3count=0 THEN                 (*so restart T1*)
                        RemoveTimer(last);
                        StartTimer(last);
                      END; 
*)
                    END;
                    sendstate:=TWAIT;
                  ELSE sendstate:=TWAIT END;
                END;

              END;


              IF dcd & (PortL2[SendPort].sendstate<>TXTAIL) THEN
                dcd:=dama;
                IF ((sendstate=TWAIT) OR (sendstate=DCDWAIT))
                & (txwait<=1) & (DCDwait<>0) & NOT dama THEN
                  IF NOT DamaMaster THEN
                    txwait:=((DCDwait*(tick10 MOD 256)) DIV 256)+2;
--                    txwait:=DCDwait; 
                  END;
                  sendstate:=DCDWAIT;
                END;

              ELSIF lookinque THEN                (*maybe sendque has an entry*)
                WITH PortL2[SendPort] DO
                  IF (sendstate=TXTAIL) OR (txwait=0) THEN
                    i:=0;
                    LOOP
                      WITH PortL2[port].SendQues[i] DO
                        IF (SendQueOut<>NIL) THEN
                          IF (sendstate=TWAIT) & (TXdel<>0) THEN
--                              StartFlags(Parms[port].SendPort,
                SendQueOut^.Baud);
                            txwait:=TXdel;
                            sendstate:=TXDEL;
                            EXIT

                          END;
                          tx(SendQueIn,SendQueOut);
                          txwait:=TXtail;
                          IF SendQueOut<>NIL THEN EXIT END;
                (*sendbuffer is full*)

                        END;
                      END;
                      INC(i);                              (*next sendque*)
                      IF i>=SENDQUES THEN PortL2[port].lookinque:=FALSE;
                EXIT END;

                    END;
                  END;
                END;
              ELSIF dama THEN dcd:=TRUE END;
            END;
          END;
        END;
      END;

      inctime;
(*
      IF LostINT=0 THEN EXIT END;

      DEC(LostINT);
      EXIT;
*)

     
    UNTIL norx;
(*  L2Lock:=FALSE;
  END;  *)
END Layer2;
*/

extern void l2_Layer2(void)
{
   /*started by ticker*/
   uint16_t i;
   uint16_t port;
   char test1;
   char norx;
   struct l2__D2 * anonym;
   struct _0 * anonym0;
   struct _1 * anonym1;
   norx = 0;
   do {
      for (port = 1U; port<=15U; port++) {
         { /* with */
            struct l2__D2 * anonym = &l2_Parms[port-1U];
            { /* with */
               struct _0 * anonym0 = &PortL2[port-1U];
               if (anonym->_._0.Diversity) {
                  /*port is enabled*/
                  test1 = PortL2[anonym->_._0.Diversity-1U].dcd;
                  PortL2[anonym->_._0.Diversity-1U].dcd = 0;
                  if (anonym->HalfDuplex && frameio_DCD((uint32_t)port)) {
                     if (anonym0->dcdguard<anonym->_._0.DCDIGNOR) {
                        PortL2[anonym->_._0.Diversity-1U].dcd = 1;
                        ++anonym0->dcdguard;
                     }
                  }
                  else anonym0->dcdguard = 0U;
                  /*IF PortL2[Diversity].dcd<>test1 THEN WrStr("d");
                WrInt(ORD(PortL2[Diversity].dcd),1) END; */
                  if (frameio_GetFrame((uint32_t)port)) {
                     /*WrStr("R"); */
                     if (anonym->monitor && (frameio_crcok || anonym->passall)
                ) Mon(port, l2_adress, l2_asize, l2_dbuf);
                     if (frameio_crcok) {
                        if (PortL2[anonym->_._0.SendPort-1U]
                .sendstate==l2_TWAIT) {
                           PortL2[anonym->_._0.SendPort-1U].txwait = 0U;
                        }
                        rx(anonym->_._0.Diversity);
                     }
                  }
                  else norx = 1;
               }
               if (anonym->_._0.Diversity==port) {
                  /*this ports has a sender*/
                  if (anonym0->Ticker!=Second) {
                     if (!(PortL2[anonym->_._0.Diversity-1U]
                .dcd || frameio_Sending((uint32_t)port))) {
                        /*timer is running*/
                        if ((struct LINK *)
                anonym0->T1Out==0 || ((struct LINK *)anonym0->T1Out)
                ->T1out!=anonym0->QTicker) {
                           anonym0->QTicker = (anonym0->QTicker+1U)%20000U;
                /*no timer in this second*/
                        }
                        else Timeout(anonym0->T1Out);
                     }
                     anonym0->Ticker = Second;
                  }
                  if (!PortL2[anonym->_._0.Diversity-1U].dcd) {
                     i = 0U;
                     for (;;) {
                        { /* with */
                           struct _1 * anonym1 = &PortL2[anonym->_._0.SendPort-1U]
                .SendQues[i];
                           if ((struct LINK *)anonym1->SendQueOut) {
                              /*WrStr("T"); */
                              tx(&anonym1->SendQueIn, &anonym1->SendQueOut);
                              if ((struct LINK *)anonym1->SendQueOut) {
                                 break; /*sendbuffer is full*/
                              }
                           }
                        }
                        ++i; /*next sendque*/
                        if (i>=3U) {
                           break;
                        }
                     }
                  }
               }
            }
         }
      } /* end for */
      inctime();
   } while (!norx);
} /* end Layer2() */


extern void l2_L2Init(uint16_t bufs, uint16_t portset,
                l2_CALLBACKPROC callback)
{
   uint16_t j;
   uint16_t i;
   struct l2__D2 * anonym;
   struct _0 * anonym0;
   /* init with default parameters */
   l2_l2verb = 0;
   l2_dupchk = 0UL;
   Event = callback;
   hosts = 0U;
   monbufs = 0U;
   monBufOut = 0;
   memset((char *)MyCalls,(char)0,119UL);
   memset((char *)DNot,(char)0,63UL);
   mfrom[0U] = 0;
   maxrxbuffers = bufs/10U;
   maxtxbuffers = 30U;
   maxmonbufs = bufs/5U;
   QTIME = 48U;
   DMTIME = 60U;
   DAMAMAXTIME = 15U;
   DAMAFASTPOLLS = 4U;
   l2_L2Lock = 0;
   l2_LostINT = 0U;
   for (i = 0U; i<=15U; i++) {
      AdrHash[i] = 0;
   } /* end for */
   Time = 0UL;
   AbsTime = 0UL;
   Second = 0U;
   Minute5 = 0U;
   memory = 0U;
   Freechain = 0;
   i = (uint16_t)sizeof(struct l2_DFIELD);
   if (sizeof(struct LINK)>(uint32_t)i) {
      i = (uint16_t)sizeof(struct LINK);
   }
   for (;;) {
      osic_alloc((char * *) &l2_dbuf, (uint32_t)i);
      if (memory>=bufs) break;
      Dealloc((char * *) &l2_dbuf);
   }
   /*WrInt(SIZE(dbuf^), 10); WrInt(SIZE(monBufOut^), 10); WrStrLn(" d,l"); */
   for (i = 1U; i<=15U; i++) {
      { /* with */
         struct l2__D2 * anonym = &l2_Parms[i-1U];
         memset((char *) &l2_Parms[i-1U],(char)0,
                sizeof(struct l2__D2));
         /*defaults*/
         anonym->_._0.T1 = 20U;
         anonym->_._0.T3 = 1800U;
         anonym->_._0.BaudSwBytes = 5U;
         anonym->_._0.TXdel = 20U; /* 25 */
         anonym->_._0.txDelByte = 126U;
         anonym->_._0.TXwait = 2U; /* 80 */
         anonym->_._0.TXtail = 1U; /* 3 */
         anonym->_._0.DCDwait = 2U;
         anonym->_._0.DCDlevel = 27000U;
         anonym->_._0.SendPort = i;
         /*    IF (i>=8) & (i<12) THEN  Diversity:=i; END;*/
         if (X2C_IN((int32_t)i,16,portset)) anonym->_._0.Diversity = i;
         anonym->_._0.Retries = 25U;
         anonym->HalfDuplex = 1;
         anonym->DamaMaster = 0;
         anonym->_._0.MaxFrames = 7U;
         anonym->_._0.IPoll = 80U;
         anonym->_._0.MhTime = 0U;
         anonym->monitor = 0;
         anonym->Digipeat = 1;
         strncpy(anonym->SendBauds,"222222222",17u);
         anonym->passall = 0;
         anonym->_._0.DCDIGNOR = 6000U; /* 6000 */
      }
      /*      MODEMPOLL:=500; */
      { /* with */
         struct _0 * anonym0 = &PortL2[i-1U];
         memset((char *) &PortL2[i-1U],(char)0,sizeof(struct _0));
         anonym0->Ticker = 0U;
         anonym0->QTicker = 0U;
         anonym0->txwait = 0U;
         anonym0->sendstate = l2_TWAIT;
         anonym0->T1Out = 0;
         anonym0->damaloop = 0;
         for (j = 0U; j<=2U; j++) {
            anonym0->SendQues[j].SendQueOut = 0;
         } /* end for */
      }
      CheckParms(i);
   } /* end for */
   memset((char *)l2_PortSt,(char)0,sizeof(struct l2__D0 [16]));
   frameio_Init();
} /* end L2Init() */


extern void l2_BEGIN(void)
{
   static int l2_init = 0;
   if (l2_init) return;
   l2_init = 1;
   if (sizeof(uint16_t)!=2) X2C_ASSERT(0);
   if (sizeof(uint8_t)!=1) X2C_ASSERT(0);
   if (sizeof(l2_CALLTYP)!=7) X2C_ASSERT(0);
   if (sizeof(l2_AFIELD)!=71) X2C_ASSERT(0);
   if (sizeof(l2_SUMMS)!=28) X2C_ASSERT(0);
   osi_BEGIN();
   frameio_BEGIN();
}

