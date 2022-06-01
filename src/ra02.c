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
#define ra02a_C_
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <signal.h>

/* gpio or lpt to scp ra02 radio module + afsk tx */
#define ra02a_LF "\012"

#define ra02a_STOUT 2

#define ra02a_STIN 1

#define ra02a_STATLEN 1
/* 30 */

#define ra02a_CALINTERVAL 180
/* loops until next check temperature calibration */

#define ra02a_CALMASK 0

#define ra02a_NETID 0x12 

#define ra02a_XTAL 3.2E+7

#define ra02a_FSTEP 6.103515625E+1

static int32_t ra02a_MINBAUD = 490L;

static float ra02a_BWTAB[10] = {7.8f,10.4f,15.6f,20.8f,31.25f,41.7f,62.5f,125.0f,250.0f,500.0f};

#define ra02a_RegSyncWord 0x39 

#define ra02a_RegFeiMsb 0x28 

#define ra02a_RegFeiMid 0x29 

#define ra02a_RegFeiLsb 0x2A 

#define ra02a_RegPacketRssi 0x1A 

#define ra02a_RegPaRamp 0xA 

#define ra02a_RegPaDac 0x4D 

#define ra02a_RegOcp 0xB 

#define ra02a_RegPktSnrValue 0x19 

#define ra02a_RegFifoRxCurrentAddr 0x10 

#define ra02a_RegOpMode 0x1 

#define ra02a_RegModemStat 0x18 

#define ra02a_RegIrqFlagsMask 0x11 

#define ra02a_RegIrqFlags 0x12 

#define ra02a_RegRxNbBytes 0x13 

#define ra02a_RegFifoAddrPtr 0xD 

#define ra02a_RegHopChannel 0x1C 

#define ra02a_RegRssiValue 0x1B 

#define ra02a_RegRxPacketCntValueLsb 0x17 

#define ra02a_RegFifo 0x0 

#define ra02a_RegFifoTxBaseAddr 0xE 

#define ra02a_RegFrMsb 0x6 

#define ra02a_RegFrMid 0x7 

#define ra02a_RegFrLsb 0x8 

#define ra02a_RegDetectOptimice 0x31 

#define ra02a_RegDetectionThreshold 0x37 

#define ra02a_RegLna 0xC 

#define ra02a_RegModemConfig3 0x26 

#define ra02a_RegModemConfig2 0x1E 

#define ra02a_RegModemConfig1 0x1D 

#define ra02a_RegPaConfig 0x9 

#define ra02a_RegPayloadLength 0x22 

#define ra02a_RegPreambleMsb 0x20 

#define ra02a_RegPreambleLsb 0x21 

#define ra02a_RegImageCal 0x3B 

#define ra02a_RegTemp 0x3C 
/*fsk */

#define ra02a_RegBitrateMsb 0x2 

#define ra02a_RegBitrateLsb 0x3 

#define ra02a_RegFdevMsb 0x4 

#define ra02a_RegFdevLsb 0x5 

#define ra02a_RegPreampleMsb 0x25 

#define ra02a_RegPreampleLsb 0x26 

#define ra02a_RegSyncConfig 0x27 

#define ra02a_RegPacketConfig1 0x30 

#define ra02a_RegPacketConfig2 0x31 

#define ra02a_RegPayloadLengthFsk 0x32 

#define ra02a_RegFifoThresh 0x35 

#define ra02a_RegSeqConfig1 0x36 

#define ra02a_RegSeqConfig2 0x37 

#define ra02a_RegIrqFlags1 0x3E 

#define ra02a_RegIrqFlags2 0x3F 

#define ra02a_RegBitrateFrac 0x5D 

#define ra02a_RegRxConfig 0xD 

#define ra02a_RegPllHf 0x70 

#define ra02a_FIFOFILL 8

#define ra02a_OVERSAMP 11

#define ra02a_AFSKBAUD 13200

#define ra02a_GPIOFN "/sys/class/gpio"

#define ra02a_GPIOEXPORT "/sys/class/gpio/export"

#define ra02a_GPIOUNEXPORT "/sys/class/gpio/unexport"

#define ra02a_GPIOX "/sys/class/gpio/gpio"

#define ra02a_GPIODIRECTION "/direction"

#define ra02a_GPIOIN "in"

#define ra02a_GPIOOUT "out"

#define ra02a_GPIODATA "/value"

#define ra02a_GPIOS 256

enum STATE {ra02a_stSLEEP, ra02a_stRX, ra02a_stWAITDCD, ra02a_stTX};


struct TXCONTEXT;

typedef struct TXCONTEXT * pTXCONTEXT;


struct TXCONTEXT {
   pTXCONTEXT next;
   float symboltime;
   float mhz;
   uint32_t sf;
   uint32_t bw;
   uint32_t cr;
   uint32_t ftextlen;
   uint32_t preamb;
   uint32_t power;
   uint32_t txdel;
   uint32_t deviation;
   uint32_t netid;
   uint32_t scrambler;
   int32_t cfgoptimize;
   int32_t baud;
   int32_t fskp;
   int32_t cfgramp;
   char usedcd;
   char rawfsk;
   char sendfsk;
   char checkip;
   char optimize;
   char implicit;
   uint32_t udpip;
   uint32_t udpbind;
   uint32_t udpport;
   int32_t udpsocket;
   char udp2;
   char ftext[3851];
};

struct GPIO;


struct GPIO {
   uint32_t ceN;
   uint32_t mosiN;
   uint32_t misoN;
   uint32_t sckN;
   int32_t ceFD;
   int32_t mosiFD;
   int32_t sckFD;
   char misoFN[100];
};

struct CHIP;

typedef struct CHIP * pCHIP;


struct CHIP {
   pCHIP next;
   struct GPIO gpio;
   uint8_t state;
   pTXCONTEXT atx;
   pTXCONTEXT ptx;
   char implicit;
   char optimize;
   char rxon;
   char agc;
   char boost;
   uint32_t rxtxdel;
   uint32_t num;
   uint32_t lnaboost;
   uint32_t lnagain;
   uint32_t rxcr;
   uint32_t rxsf;
   uint32_t rxbw;
   uint32_t band;
   uint32_t uso;
   uint32_t stato;
   uint32_t statu;
   uint32_t netid;
   uint32_t calcnt;
   int32_t cfgoptimize;
   int32_t cfgramp;
   int32_t rssicorr;
   int32_t cfgocp;
   float symboltime;
   float rxmhz;
   float ppm;
};

typedef uint32_t GPIOSET[8];

static uint32_t tcnt;

static uint32_t loopdelay;

static uint32_t loopdelayfast;

static char verb;

static char verb2;

static char h[501];

static pCHIP chips;

static GPIOSET gpostate;

static int32_t gpiofds[256];
/*
PROCEDURE ["C"] / clock_gettime(typ:CARDINAL; VAR tv:TIMEHR):INTEGER;
PROCEDURE usec():CARDINAL;
VAR t:TIMEHR;
    res:INTEGER;
BEGIN
  res:=clock_gettime(1,t);
  RETURN t.nsec DIV 1000 MOD 1000000 + t.sec*1000000
END usec;
*/


static uint32_t usec(void)
{
   uint32_t ns;
   uint32_t s;
   osic_timens(1, &s, &ns);
   return (ns/1000UL)%1000000UL+s*1000000UL;
} /* end usec() */


static void Error(const char text[], uint32_t text_len)
{
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
} /* end Error() */


static char StrToHex(const char s[], uint32_t s_len, uint32_t * n)
{
   uint32_t i;
   char c;
   i = 0UL;
   *n = 0UL;
   while (i<=s_len-1 && s[i]) {
      *n =  *n*16UL;
      c = X2C_CAP(s[i]);
      if ((uint8_t)c>='0' && (uint8_t)c<='9') *n += (uint32_t)(uint8_t)c-48UL;
      else if ((uint8_t)c>='A' && (uint8_t)c<='F') *n += (uint32_t)(uint8_t)c-55UL;
      else return 0;
      ++i;
   }
   return 1;
} /* end StrToHex() */


static int32_t GetIp(const char h0[], uint32_t h_len, uint32_t * ip, uint32_t * dp, uint32_t * lp,
                int32_t * fd, char * check)
{
   if (aprsstr_GetIp2(h0, h_len, ip, dp, lp, check)<0L) return -1L;
   *fd = openudp();
   if (*fd<0L || bindudp(*fd, *lp)<0L) {
      /*OR (udp.udpnonblock(fd)<0)*/
      return -1L;
   }
   return 0L;
} /* end GetIp() */


static int32_t opengpio(uint32_t n, char out, char fnr[], uint32_t fnr_len)
{
   char hp[100];
   char hh[100];
   char h0[100];
   int32_t fd;
   uint32_t tr;
   aprsstr_CardToStr(n, 1UL, h0, 100ul);
   strncpy(hp,"/sys/class/gpio/gpio",100u); /* /sys/class/gpio/gpio */
   aprsstr_Append(hp, 100ul, h0, 100ul); /* /sys/class/gpio/gpio<n> */
   memcpy(hh,hp,100u);
   aprsstr_Append(hp, 100ul, "/value", 7ul); /* /sys/class/gpio/gpio<n>/value */
   if (!out) aprsstr_Assign(fnr, fnr_len, hp, 100ul);
   fd = gpiofds[n];
   if (fd==-2L) {
      /* port is not open jet */
      tr = 0UL;
      for (;;) {
         fd = osi_OpenWrite("/sys/class/gpio/unexport", 25ul); /* /sys/class/gpio/unexport */
         if (fd>=0L) break;
         usleep(100000UL); /* pray for udev junk is fast enough */
         ++tr;
         if (tr>4UL) break;
      }
      osi_WrBin(fd, (char *)h0, 100u/1u, aprsstr_Length(h0, 100ul));
      osic_Close(fd);
      fd = osi_OpenWrite("/sys/class/gpio/export", 23ul); /* /sys/class/gpio/export */
      if (fd<0L) Error("cannot open gpio export", 24ul);
      osi_WrBin(fd, (char *)h0, 100u/1u, aprsstr_Length(h0, 100ul));
      osic_Close(fd);
      aprsstr_Append(hh, 100ul, "/direction", 11ul);
      tr = 0UL;
      for (;;) {
         fd = osi_OpenWrite(hh, 100ul); /* /sys/class/gpio/gpio<n> */
         if (fd>=0L) break;
         usleep(100000UL); /* pray for udev junk is fast enough */
         ++tr;
         if (tr>4UL) break;
      }
      if (fd<0L) Error("cannot open gpio direction", 27ul);
      if (out) strncpy(h0,"out",100u);
      else strncpy(h0,"in",100u);
      osi_WrBin(fd, (char *)h0, 100u/1u, aprsstr_Length(h0, 100ul)); /* in / out */
      osic_Close(fd);
      if (out) {
         fd = osi_OpenRW(hp, 100ul);
         if (fd<0L) Error("cannot open gpio value", 23ul);
      }
      else fd = -1L;
      gpiofds[n] = fd;
   }
   return fd;
} /* end opengpio() */


static void Wrchipnum(pCHIP chip0, char nl)
{
   if (chips->next) {
      osi_WrStr("chip:", 6ul);
      osic_WrUINT32(chip0->num, 1UL);
      if (nl) osi_WrStrLn("", 1ul);
   }
} /* end Wrchipnum() */

static GPIOSET _cnst0 = {0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL};

static void chkports(void)
{
   pCHIP chip0;
   GPIOSET outs;
   GPIOSET ins;
   uint32_t i;
   memcpy(ins,_cnst0,32u);
   memcpy(outs,_cnst0,32u);
   chip0 = chips;
   while (chip0) {
      X2C_INCL(outs,chip0->gpio.mosiN,256);
      X2C_INCL(outs,chip0->gpio.sckN,256);
      if (X2C_INL(chip0->gpio.misoN,256,ins)) {
         osi_Werr("warning, shared inputs needs ce-pullup for not used chip\012", 58ul);
      }
      X2C_INCL(ins,chip0->gpio.misoN,256);
      chip0 = chip0->next;
   }
   for (i = 0UL; i<=255UL; i++) {
      if (X2C_INL(i,256,ins) && X2C_INL(i,256,outs)) Error("input<->output port conflict", 29ul);
   } /* end for */
   for (i = 0UL; i<=255UL; i++) {
      if (X2C_INL(i,256,ins)) X2C_INCL(outs,i,256);
   } /* end for */
   chip0 = chips;
   while (chip0) {
      if (X2C_INL(chip0->gpio.ceN,256,outs)) Error("ce must not be shared", 22ul);
      X2C_INCL(outs,chip0->gpio.ceN,256);
      chip0 = chip0->next;
   }
} /* end chkports() */

static float _cnst[10] = {7.8f,10.4f,15.6f,20.8f,31.25f,41.7f,62.5f,125.0f,250.0f,500.0f};

static void autoopt(pTXCONTEXT tx)
{
   tx->symboltime = X2C_DIVR((float)(uint32_t)X2C_LSH(0x1UL,32,(int32_t)tx->sf),_cnst[tx->bw]);
   tx->optimize = tx->symboltime>16.0f;
   if (tx->cfgoptimize==0L) tx->optimize = 0;
   else if (tx->cfgoptimize==1L) tx->optimize = 1;
} /* end autoopt() */


static void rxautoopt(pCHIP chp)
{
   chp->symboltime = X2C_DIVR((float)(uint32_t)X2C_LSH(0x1UL,32,(int32_t)chp->rxsf),_cnst[chp->rxbw]);
   chp->optimize = chp->symboltime>16.0f;
   if (chp->cfgoptimize==0L) chp->optimize = 0;
   else if (chp->cfgoptimize==1L) chp->optimize = 1;
} /* end rxautoopt() */


static pCHIP newchip(void)
{
   pCHIP chp;
   osic_alloc((char * *) &chp, sizeof(struct CHIP));
   if (chp==0) Error("out of memory", 14ul);
   memset((char *)chp,(char)0,sizeof(struct CHIP));
   chp->lnaboost = 3UL;
   chp->lnagain = 1UL;
   chp->rxon = 0;
   chp->ppm = 0.0f;
   chp->rssicorr = 0L;
   chp->cfgocp = -1L;
   chp->cfgramp = -1L;
   chp->agc = 0;
   chp->rxmhz = 433.775f;
   chp->rxsf = 12UL;
   chp->rxbw = 7UL;
   chp->rxcr = 5UL;
   chp->netid = 18UL;
   chp->cfgoptimize = -1L;
   return chp;
} /* end newchip() */


static void storechip(uint32_t pcnt, int32_t * res, uint32_t * sck0, uint32_t * miso0, uint32_t * mosi0,
                uint32_t * ce0, pCHIP * chip0)
{
   char fn[100];
   struct GPIO * anonym;
   { /* with */
      struct GPIO * anonym = &(*chip0)->gpio;
      anonym->ceN = *ce0;
      anonym->mosiN = *mosi0;
      anonym->misoN = *miso0;
      anonym->sckN = *sck0;
      anonym->ceFD = opengpio(anonym->ceN, 1, fn, 100ul);
      anonym->mosiFD = opengpio(anonym->mosiN, 1, fn, 100ul);
      *res = opengpio(anonym->misoN, 0, anonym->misoFN, 100ul);
      anonym->sckFD = opengpio(anonym->sckN, 1, fn, 100ul);
   }
   (*chip0)->num = pcnt;
   if ((*chip0)->rxmhz<400.0f) (*chip0)->band = 1UL;
   else if ((*chip0)->rxmhz<500.0f) (*chip0)->band = 2UL;
   else (*chip0)->band = 3UL;
   rxautoopt(*chip0);
   (*chip0)->next = chips;
   chips = *chip0;
   *chip0 = newchip();
} /* end storechip() */


static pTXCONTEXT newtx(void)
{
   pTXCONTEXT tx;
   osic_alloc((char * *) &tx, sizeof(struct TXCONTEXT));
   if (tx==0) Error("out of memory", 14ul);
   memset((char *)tx,(char)0,sizeof(struct TXCONTEXT));
   tx->cfgoptimize = -1L;
   tx->cfgramp = -1L;
   tx->mhz = 433.775f;
   tx->sf = 12UL;
   tx->bw = 7UL;
   tx->cr = 5UL;
   tx->netid = 18UL;
   tx->power = 10UL;
   tx->preamb = 8UL;
   tx->txdel = 4UL;
   tx->deviation = 3000UL;
   tx->baud = 0L;
   tx->sendfsk = 0;
   tx->rawfsk = 0;
   return tx;
} /* end newtx() */


static void storetx(pCHIP chip0, pTXCONTEXT tx)
{
   autoopt(tx);
   /*  IF tx^.udpbind<>0 THEN */
   tx->next = chip0->ptx;
   chip0->ptx = tx;
/*  ELSE DEALLOCATE(tx, SIZE(tx^)); END; */
} /* end storetx() */


static void Parms(void)
{
   char err;
   char hh[4096];
   char h0[4096];
   uint32_t lptnum;
   uint32_t txcnt;
   uint32_t pcnt;
   uint32_t sck0;
   uint32_t mosi0;
   uint32_t miso0;
   uint32_t ce0;
   int32_t res;
   pTXCONTEXT tx;
   pCHIP chip0;
   struct CHIP * anonym;
   for (pcnt = 0UL; pcnt<=255UL; pcnt++) {
      gpiofds[pcnt] = -2L;
   } /* end for */
   lptnum = 0UL;
   err = 0;
   pcnt = 0UL;
   chips = 0;
   ce0 = 8UL;
   mosi0 = 10UL;
   miso0 = 9UL;
   sck0 = 11UL;
   loopdelay = 50000UL;
   loopdelayfast = 20000UL;
   chip0 = 0;
   tx = newtx();
   txcnt = 0UL;
   for (;;) {
      if (chip0==0) {
         chip0 = newchip();
         txcnt = 0UL;
      }
      osi_NextArg(h0, 4096ul);
      if (h0[0U]==0) break;
      if (h0[0U]=='-' && h0[1U]) {
         if (h0[1U]=='p') {
            if (pcnt>0UL) {
               storetx(chip0, tx);
               storechip(pcnt, &res, &sck0, &miso0, &mosi0, &ce0, &chip0);
               tx = newtx();
               txcnt = 0UL;
            }
            ++pcnt;
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &ce0) || ce0>=256UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &mosi0) || mosi0>=256UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &miso0) || miso0>=256UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &sck0) || sck0>=256UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
         }
         else if (h0[1U]=='l') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &chip0->lnaboost) || chip0->lnaboost>3UL) {
               Error("-l <lnaboost 0..3>", 19ul);
            }
         }
         else if (h0[1U]=='u') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &loopdelay)) {
               Error("-u <poll-sleep us> <poll-sleep-tx us>", 38ul);
            }
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &loopdelayfast)) {
               Error("-u <poll-intervall us>", 23ul);
            }
         }
         else if (h0[1U]=='C') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToCard(h0, 4096ul, &tx->cr) || tx->cr<5UL) || tx->cr>8UL) {
               Error("-C <cr 5..8>", 13ul);
            }
         }
         else if (h0[1U]=='c') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToCard(h0, 4096ul, &chip0->rxcr) || chip0->rxcr<5UL) || chip0->rxcr>8UL) {
               Error("-c <cr 5..8>", 13ul);
            }
         }
         else if (h0[1U]=='D') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &tx->deviation) || tx->deviation>20000UL) {
               Error("-D <deviation 0..200008 (3000)>", 32ul);
            }
         }
         else if (h0[1U]=='s') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToCard(h0, 4096ul, &chip0->rxsf) || chip0->rxsf<6UL) || chip0->rxsf>12UL) {
               Error("-S <sf 6..12>", 14ul);
            }
         }
         else if (h0[1U]=='S') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToCard(h0, 4096ul, &tx->sf) || tx->sf<6UL) || tx->sf>12UL) {
               Error("-S <sf 6..12>", 14ul);
            }
         }
         else if (h0[1U]=='n') {
            osi_NextArg(h0, 4096ul);
            if (!StrToHex(h0, 4096ul, &chip0->netid)) Error("-n <netid>", 11ul);
         }
         else if (h0[1U]=='N') {
            osi_NextArg(h0, 4096ul);
            if (!StrToHex(h0, 4096ul, &tx->netid)) Error("-N <netid>", 11ul);
         }
         else if (h0[1U]=='g') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToCard(h0, 4096ul, &chip0->lnagain) || chip0->lnagain>6UL) || chip0->lnagain<1UL) {
               Error("-g <lnagain 1..6>", 18ul);
            }
         }
         else if (h0[1U]=='T') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &tx->txdel) || tx->txdel>100UL) {
               Error("-T <txdel bytes> (0..100)", 26ul);
            }
         }
         else if (h0[1U]=='H') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &tx->preamb)) Error("-H <preamble", 13ul);
         }
         else if (h0[1U]=='b') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &chip0->rxbw) || chip0->rxbw>9UL) {
               Error("-b <bandwidth 0..9", 19ul);
            }
         }
         else if (h0[1U]=='B') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &tx->bw) || tx->bw>9UL) {
               Error("-B <bandwidth 0..9", 19ul);
            }
         }
         else if (h0[1U]=='w') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToCard(h0, 4096ul, &tx->power) || tx->power>17UL) {
               Error("-w <power 0..17", 16ul);
            }
         }
         else if (h0[1U]=='U' || h0[1U]=='L') {
            if (txcnt>0UL) {
               storetx(chip0, tx);
               tx = newtx();
            }
            ++txcnt;
            tx->udp2 = h0[1U]!='U'; /* switch on axudp2 */
            osi_NextArg(h0, 4096ul);
            if (GetIp(h0, 4096ul, &tx->udpip, &tx->udpport, &tx->udpbind, &tx->udpsocket, &tx->checkip)<0L) {
               strncpy(h0,"cannot open udp socket port ",4096u);
               aprsstr_CardToStr(tx->udpbind, 1UL, hh, 4096ul);
               aprsstr_Append(h0, 4096ul, hh, 4096ul);
               Error(h0, 4096ul);
            }
            if (tx->udpport) chip0->rxon = 1;
         }
         else if (h0[1U]=='v') verb = 1;
         else if (h0[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h0[1U]=='a') chip0->agc = 1;
         else if (h0[1U]=='E') tx->rawfsk = 1;
         else if (h0[1U]=='I') tx->implicit = 1;
         else if (h0[1U]=='i') chip0->implicit = 1;
         else if (h0[1U]=='A') tx->sendfsk = 1;
         else if (h0[1U]=='d') tx->usedcd = 1;
         else if (h0[1U]=='G') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToInt(h0, 4096ul, &tx->baud)) {
               Error("-G [-]<baud>", 13ul);
            }
            if (labs(tx->baud)<490L) {
               if (tx->baud<0L) tx->baud = -490L;
               else tx->baud = 490L;
            }
            tx->sendfsk = 1;
         }
         else if (h0[1U]=='R') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToInt(h0, 4096ul, &chip0->cfgramp)) Error("-R <n> 0..15", 13ul);
         }
         else if (h0[1U]=='r') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToInt(h0, 4096ul, &chip0->rssicorr)) Error("-r <n>", 7ul);
         }
         else if (h0[1U]=='I') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToInt(h0, 4096ul, &chip0->cfgocp)) Error("-I <n>", 7ul);
         }
         else if (h0[1U]=='O') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToInt(h0, 4096ul, &tx->cfgoptimize)) Error("-o <-1..1", 10ul);
         }
         else if (h0[1U]=='o') {
            osi_NextArg(h0, 4096ul);
            if (!aprsstr_StrToInt(h0, 4096ul, &chip0->cfgoptimize)) Error("-O <-1..1", 10ul);
         }
         else if (h0[1U]=='P') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToFix(&chip0->ppm, h0, 4096ul) || chip0->ppm<(-128.0f)) || chip0->ppm>128.0f) {
               Error("-P <ppm>", 9ul);
            }
         }
         else if (h0[1U]=='F') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToFix(&tx->mhz, h0, 4096ul) || tx->mhz<137.0f) || tx->mhz>1020.0f) {
               Error("-F <MHz>", 9ul);
            }
         }
         else if (h0[1U]=='f') {
            osi_NextArg(h0, 4096ul);
            if ((!aprsstr_StrToFix(&chip0->rxmhz, h0, 4096ul) || chip0->rxmhz<137.0f) || chip0->rxmhz>1020.0f) {
               Error("-f <MHz>", 9ul);
            }
         }
         else if (h0[1U]=='h') {
            osi_WrStrLn(" ra-02 (sx127x) via LPT or multiple chips via GPIO to axudp by oe5dxl", 70ul);
            osi_WrStrLn(" -A                 tx AFSK 1200 Baud", 38ul);
            osi_WrStrLn(" -a                 AGC on", 27ul);
            osi_WrStrLn(" -B <n>             tx bandwidth kHz 0:7.8 1:10.4 2:15.6 3:20.8 4:31.25 5:41.7 6:62.5 7:125 8:2\
50 9:500 (7)", 108ul);
            osi_WrStrLn(" -b <n>             rx bandwidth (7)", 37ul);
            osi_WrStrLn(" -C <cr>            tx coding rate 5..8 (5)", 44ul);
            osi_WrStrLn(" -c <cr>            rx coding rate 5..8 (5)", 44ul);
            osi_WrStrLn(" -D <Hz>            (a)fsk deviation +-Hz (3000)", 49ul);
            osi_WrStrLn(" -d                 do not send while DCD on this band or if dcd on tx sf/bw)", 78ul);
            osi_WrStrLn("                      timebase is detection duration, some ms see chip manual", 78ul);
            osi_WrStrLn(" -E                 Send UDP frame unmodified in FSK up to 1500Byte eg. for POCSAG", 83ul);
            osi_WrStrLn(" -F <MHz>           tx MHz (433.775)", 37ul);
            osi_WrStrLn(" -f <MHz>           rx MHz (433.775)", 37ul);
            osi_WrStrLn(" -G [-]<baud>       Send GFSK <baud> -baud same but g3ruh scrambler off (490..250000)", 86ul);
            osi_WrStrLn(" -g <n>             lna gain 6..1, 1 is maximum gain! see chip manual(1)", 73ul);
            osi_WrStrLn(" -H <n>             Preample length (8) sx seems to need minimum 4", 67ul);
            osi_WrStrLn(" -h                 this", 25ul);
            osi_WrStrLn(" -I                 tx implicit header on", 42ul);
            osi_WrStrLn(" -i                 rx implicit header on", 42ul);
            osi_WrStrLn(" -L ip:sendport:listenport AXUDPv2 data, apply before all othere parameters for this channel",
                93ul);
            osi_WrStrLn("                      repeat for more tx contexts with different listen ports", 78ul);
            osi_WrStrLn("                      same rx data are sent to all non zero sendports", 70ul);
            osi_WrStrLn(" -l <n>             lna boost 0..3, more for better ip3 by more supply current (3)", 83ul);
            osi_WrStrLn(" -n <netid>         network id (sync word) (not use 34=lorawan) (12)", 69ul);
            osi_WrStrLn(" -I <n>             set overcurrent protection, use with care, see chip manual", 79ul);
            osi_WrStrLn(" -O <offon>         tx low datarate optimize 0=off 1=on else automatic (-1)", 76ul);
            osi_WrStrLn(" -o <offon>         rx low datarate optimize 0=off 1=on else automatic (-1)", 76ul);
            osi_WrStrLn(" -P <ppm>           x-tal correction +-128 (0.0)", 49ul);
            osi_WrStrLn(" -p <nss> <mosi> <miso> <sck>  GPIO numbers, apply before all parameter to this chip", 85ul);
            osi_WrStrLn("                      repeat for more chips (8 10 9 11)", 56ul);
            osi_WrStrLn("                      with different <nss> pins. Sharing <miso> needs <nss> high", 81ul);
            osi_WrStrLn("                      on not configed chips by pullup or setting <nss> manual", 78ul);
            osi_WrStrLn(" -S <sf>            tx spread factor 6..12 (12)", 48ul);
            osi_WrStrLn(" -s <sf>            rx spread factor 6..12 (12)", 48ul);
            osi_WrStrLn(" -R <n>             PaRamp how fast tx goes to power, see chip manual (9)", 74ul);
            osi_WrStrLn(" -r <n>             add to rssi value to compensate internal and external preamps (0)", 86ul);
            osi_WrStrLn(" -T <n>             (A)FSK txdel in byte (4), not used for raw mode -E", 71ul);
            osi_WrStrLn(" -U ip:sendport:receiveport AXUDP data, same as -L but standard AXUDP (no metadata)", 84ul);
            osi_WrStrLn(" -u <us> <us>       sleep time betrween divice polls rx/(a)fsk tx, more:faster response, more c\
pu (50000 20000)", 112ul);
            osi_WrStrLn("                      afsk needs 20000 or less to avoid underruns", 66ul);
            osi_WrStrLn(" -V                 show more infos on stdout", 46ul);
            osi_WrStrLn(" -v                 show some infos on stdout", 46ul);
            osi_WrStrLn(" -w <dBm>           tx power 0..17 (10)", 40ul);
            osi_WrStrLn("", 1ul);
            osi_WrStrLn("ra02 -L 127.0.0.1:9000:9001 -d -r -10 -f 433.775 -w 17 -v", 58ul);
            osi_WrStrLn("ra02 -p 8 10 9 11 -L 127.0.0.1:2400:2401 -P -2 -d -S 9 -B 6 -F 434.1 -f 433.775 -s 12 -b 7 -L 1\
27.0.0.1:0:2400 -v", 114ul);
            osi_WrStrLn("ra02 -P <gpio/chip1> -L ... <parameters chip1 tx1/rx> -L <parameters chip1 tx2> ... -P <gpio/ch\
ip2> -L ... <parameters chip2 tx1/rx> -L ... -v", 143ul);
            osi_WrStrLn("", 1ul);
            X2C_ABORT();
         }
         else {
            h0[2U] = 0;
            aprsstr_Append(h0, 4096ul, "? use -h", 9ul);
            Error(h0, 4096ul);
         }
         h0[0U] = 0;
      }
      else {
         h0[1U] = 0;
         aprsstr_Append(h0, 4096ul, "? use -h", 9ul);
         Error(h0, 4096ul);
      }
   }
   if (chips==0 || pcnt>0UL) {
      storetx(chip0, tx);
      storechip(pcnt, &res, &sck0, &miso0, &mosi0, &ce0, &chip0);
   }
   chkports();
   if (verb) {
      chip0 = chips;
      while (chip0) {
         { /* with */
            struct CHIP * anonym = chip0;
            Wrchipnum(chip0, 1);
            osi_WrStr("RX:", 4ul);
            tx = anonym->ptx;
            txcnt = 0UL;
            while (tx) {
               if (tx->udpport) {
                  if (txcnt==0UL) osi_WrStr(" port=", 7ul);
                  else osi_WrStr(",", 2ul);
                  osic_WrUINT32(tx->udpport, 1UL);
                  ++txcnt;
               }
               tx = tx->next;
            }
            osi_WrStr(" on=", 5ul);
            osic_WrUINT32((uint32_t)anonym->rxon, 1UL);
            osi_WrStr(" opt=", 6ul);
            osic_WrUINT32((uint32_t)anonym->optimize, 1UL);
            osi_WrStr(" agc=", 6ul);
            osic_WrUINT32((uint32_t)anonym->agc, 1UL);
            osi_WrStr(" f=", 4ul);
            osic_WrFixed(anonym->rxmhz, 4L, 1UL);
            osi_WrStr("MHz", 4ul);
            osi_WrStr(" sf=", 5ul);
            osic_WrUINT32(anonym->rxsf, 1UL);
            osi_WrStr(" bw=", 5ul);
            osic_WrFixed(_cnst[anonym->rxbw], 1L, 1UL);
            osi_WrStr("kHz", 4ul);
            osi_WrStr(" id=", 5ul);
            osi_WrHex(anonym->netid, 0UL);
            osi_WrStr(" lnaboost=", 11ul);
            osic_WrUINT32(anonym->lnaboost, 1UL);
            osi_WrStr(" symt=", 7ul);
            osic_WrFixed(anonym->symboltime, 3L, 1UL);
            osi_WrStr("ms", 3ul);
            osi_WrStrLn("", 1ul);
            tx = anonym->ptx;
            while (tx) {
               if (tx->udpbind) {
                  osi_WrStr("TX:", 4ul);
                  osi_WrStr(" port=", 7ul);
                  osic_WrUINT32(tx->udpbind, 1UL);
                  osi_WrStr(" pwr=", 6ul);
                  osic_WrUINT32(tx->power, 1UL);
                  osi_WrStr("dBm", 4ul);
                  osi_WrStr(" f=", 4ul);
                  osic_WrFixed(tx->mhz, 4L, 1UL);
                  osi_WrStr("MHz", 4ul);
                  if (!tx->rawfsk && !tx->sendfsk) {
                     osi_WrStr(" opt=", 6ul);
                     osic_WrUINT32((uint32_t)tx->optimize, 1UL);
                     osi_WrStr(" imp=", 6ul);
                     osic_WrUINT32((uint32_t)tx->implicit, 1UL);
                     osi_WrStr(" sf=", 5ul);
                     osic_WrUINT32(tx->sf, 1UL);
                     osi_WrStr(" cr=", 5ul);
                     osic_WrUINT32(tx->cr, 1UL);
                     osi_WrStr(" bw=", 5ul);
                     osic_WrFixed(_cnst[tx->bw], 1L, 1UL);
                     osi_WrStr("kHz", 4ul);
                     osi_WrStr(" id=", 5ul);
                     osi_WrHex(tx->netid, 0UL);
                     osi_WrStr(" preamb=", 9ul);
                     osic_WrFixed(tx->symboltime*(float)tx->preamb, 2L, 1UL);
                     osi_WrStr("ms", 3ul);
                  }
                  else {
                     if (!tx->rawfsk) {
                        osi_WrStr(" txdel=", 8ul);
                        osic_WrUINT32(tx->txdel, 1UL);
                        osi_WrStr("byte", 5ul);
                        if (tx->baud==0L) {
                           osi_WrStr(" baud=1200 AFSK", 16ul);
                        }
                        else {
                           osi_WrStr("baud=", 6ul);
                           osic_WrUINT32((uint32_t)labs(tx->baud), 1UL);
                           if (tx->baud<0L) {
                              osi_WrStr(" FSK", 5ul);
                           }
                           else osi_WrStr(" g3ruh", 7ul);
                        }
                     }
                     else {
                        osi_WrStr(" baud=", 7ul);
                        osic_WrUINT32((uint32_t)labs(tx->baud), 1UL);
                        osi_WrStr(" raw FSK", 9ul);
                     }
                     osi_WrStr(" deviation+-=", 14ul);
                     osic_WrUINT32(tx->deviation, 1UL);
                  }
                  osi_WrStrLn("", 1ul);
               }
               tx = tx->next;
            }
            chip0 = anonym->next;
         }
      }
   }
} /* end Parms() */


static void delay(void)
{
/*  usleep(20); */
} /* end delay() */


static char scp(const struct GPIO gpio, char rd, char nss, char sck, char mosi)
{
   char h0[2];
   char res;
   int32_t fd;
   int32_t r;
   if (rd) {
      fd = osi_OpenRead(gpio.misoFN, 100ul);
      if (fd>=0L) {
         r = osi_RdBin(fd, (char *)h0, 2u/1u, 1UL);
         osic_Close(fd);
         res = h0[0U]!='0';
      }
      else res = 0;
   }
   else res = 0;
   h0[0U] = (char)(48UL+(uint32_t)sck);
   osi_WrBin(gpio.sckFD, (char *)h0, 2u/1u, 1UL);
   if (nss!=X2C_INL(gpio.ceN,256,gpostate)) {
      if (nss) {
         h0[0U] = '1';
         X2C_INCL(gpostate,gpio.ceN,256);
      }
      else {
         h0[0U] = '0';
         X2C_EXCL(gpostate,gpio.ceN,256);
      }
      osi_WrBin(gpio.ceFD, (char *)h0, 2u/1u, 1UL);
   }
   if (mosi!=X2C_INL(gpio.mosiN,256,gpostate)) {
      if (mosi) {
         h0[0U] = '1';
         X2C_INCL(gpostate,gpio.mosiN,256);
      }
      else {
         h0[0U] = '0';
         X2C_EXCL(gpostate,gpio.mosiN,256);
      }
      osi_WrBin(gpio.mosiFD, (char *)h0, 2u/1u, 1UL);
   }
   return res;
} /* end scp() */


static void scpio(const struct GPIO gpio, char wr, uint32_t a, uint32_t len, char s[], uint32_t s_len)
/* send or receive a buffer */
{
   uint32_t d;
   uint32_t rd;
   uint32_t j;
   uint32_t i;
   char b;
   j = 0UL;
   i = 16UL;
   while (j<len) {
      d = ((uint32_t)(uint8_t)s[j]+(a&127UL)*256UL+32768UL*(uint32_t)wr)*2UL;
      rd = 0UL;
      for (;;) {
         rd += rd+(uint32_t)scp(gpio, !wr && i<=7UL, i==0UL && j==len-1UL, 0, X2C_IN(i,32,(uint32_t)d));
         delay();
         if (i==0UL) break;
         b = scp(gpio, 0, 0, 1, X2C_IN(i,32,(uint32_t)d));
         delay();
         --i;
      }
      if (!wr) s[j] = (char)rd;
      ++j;
      i = 8UL;
   }
} /* end scpio() */


static void scpo(const struct GPIO gpio, uint32_t a, uint32_t d)
/* 1 byte out */
{
   char s;
   s = (char)d;
   scpio(gpio, 1, a, 1UL, (char *) &s, 1u/1u);
} /* end scpo() */


static uint32_t scpi(const struct GPIO gpio, uint32_t a)
/* 1 byte in */
{
   char s;
   scpio(gpio, 0, a, 1UL, (char *) &s, 1u/1u);
   return (uint32_t)(uint8_t)s;
} /* end scpi() */


static void WrByte(char c)
{
   if ((uint8_t)c>=' ' && (uint8_t)c<'\177') osi_WrStr((char *) &c, 1u/1u);
   else {
      osi_WrStr("[", 2ul);
      osi_WrHex((uint32_t)(uint8_t)c, 1UL);
      osi_WrStr("]", 2ul);
   }
} /* end WrByte() */


static void WrBytes(const char s[], uint32_t s_len, int32_t len)
{
   int32_t i;
   int32_t tmp;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      WrByte(s[i]);
      if (i==tmp) break;
   } /* end for */
} /* end WrBytes() */


static void Showregs(pCHIP chp)
{
   uint32_t i;
   for (i = 0UL; i<=100UL; i++) {
      osi_WrHex(i, 1UL);
      osi_WrStr(":", 2ul);
      osi_WrHex(scpi(chp->gpio, i), 0UL);
      osi_WrStr(" ", 2ul);
      if ((i&15UL)==15UL) osi_WrStrLn("", 1ul);
   } /* end for */
   osi_WrStrLn("", 1ul);
} /* end Showregs() */


static void app(uint32_t * i, uint32_t * p, char b[501], char c, int32_t v)
{
   char s[51];
   b[*p] = c;
   ++*p;
   aprsstr_IntToStr(v, 0UL, s, 51ul);
   *i = 0UL;
   while (s[*i]) {
      b[*p] = s[*i];
      ++*p;
      ++*i;
   }
   b[*p] = ' ';
   ++*p;
} /* end app() */


static void sendaxudp2(pCHIP chp, char mon[], uint32_t mon_len, int32_t txd, int32_t lev, int32_t snr,
                int32_t afc)
{
   char data[501];
   char b[501];
   int32_t datalen;
   int32_t ret;
   uint32_t i;
   uint32_t p;
   pTXCONTEXT tx;
   X2C_PCOPY((void **)&mon,mon_len);
   if (chp->rxon) {
      aprsstr_mon2raw(mon, mon_len, data, 501ul, &datalen);
      if (datalen>2L) {
         datalen -= 2L; /* remove crc */
         b[0U] = '\001';
         b[1U] = '0';
         p = 2UL;
         app(&i, &p, b, 'T', txd);
         app(&i, &p, b, 'V', lev);
         app(&i, &p, b, 'S', snr);
         app(&i, &p, b, 'A', afc);
         b[p] = 0; /* end of axudp2 header */
         ++p;
         i = 0UL;
         do {
            b[p] = data[i];
            ++p;
            ++i;
         } while ((int32_t)i<datalen);
         aprsstr_AppCRC(b, 501ul, (int32_t)p);
         tx = chp->ptx;
         while (tx) {
            /* send rx data to all tx contexts */
            if (tx->udpport) {
               ret = udpsend(tx->udpsocket, b, (int32_t)(p+2UL), tx->udpport, tx->udpip);
            }
            tx = tx->next; /* prx next points to ptx table */
         }
      }
   }
   X2C_PFREE(mon);
} /* end sendaxudp2() */


static void hdlc(char b[], uint32_t b_len, int32_t len, int32_t txdel, char h0[], uint32_t h_len,
                uint32_t * hlen, char fsk, char scramb, char notxd, uint32_t * scrambler)
{
   uint32_t dds;
   uint32_t sc;
   uint32_t obc;
   uint32_t bitc;
   uint32_t stuff;
   int32_t p;
   uint16_t c;
   char dostuff;
   char nrzi;
   nrzi = 0;
   bitc = 0UL;
   if (notxd) p = -2L;
   else p = -txdel;
   stuff = 0UL;
   dostuff = 0;
   obc = 0UL;
   *hlen = 0UL;
   dds = 0UL;
   do {
      if (stuff>=5UL) {
         nrzi = !nrzi; /* stuffbit */
         stuff = 0UL;
      }
      else {
         if (bitc==0UL) {
            dostuff = p>=0L && p<len;
            if (dostuff) c = (uint16_t)(uint8_t)b[p];
            else {
               c = 126U;
               if (p<-1L) c = 0U;
               stuff = 0UL;
            }
            ++p;
            bitc = 8UL;
         }
         if ((c&1)) {
            if (dostuff) ++stuff;
         }
         else {
            nrzi = !nrzi;
            stuff = 0UL;
         }
         c = c>>1;
         --bitc;
      }
      /* store nrzi bit */
      if (fsk) {
         if (*hlen<(h_len-1)-8UL) {
            *scrambler = X2C_LSH(*scrambler,32,1);
            if (scramb) {
               if (nrzi!=(((0x1000UL & *scrambler)!=0)!=((0x20000UL & *scrambler)!=0))) {
                  *scrambler |= 0x1UL;
               }
            }
            else if (nrzi) *scrambler |= 0x1UL;
            h0[*hlen] = (char)((uint32_t)(uint8_t)h0[*hlen]*2UL+(uint32_t)((0x1UL & *scrambler)!=0));
            ++obc;
            if (obc>7UL) {
               obc = 0UL;
               ++*hlen;
            }
         }
      }
      else {
         for (sc = 0UL; sc<=10UL; sc++) {
            if (*hlen<(h_len-1)-8UL) {
               if (nrzi) dds += 715784192UL;
               else dds += 390397952UL;
               h0[*hlen] = (char)((uint32_t)(uint8_t)h0[*hlen]*2UL+(uint32_t)(dds>=0x080000000UL));
               ++obc;
               if (obc>7UL) {
                  obc = 0UL;
                  ++*hlen;
               }
            }
         } /* end for */
      }
   } while (!(p>len+1L && obc==0UL));
/* store nrzi bit */
/*+2*VAL(INTEGER,ORD(scramb))*/
/* trailing flags */
/*WrStrLn(""); FOR p:=0 TO VAL(INTEGER,hlen) DO WrHex(ORD(h[p]),3) END; WrStrLn(""); */
} /* end hdlc() */


static void showbits(const char b[], uint32_t b_len, int32_t len)
{
   int32_t j;
   int32_t i;
   int32_t tmp;
   if (len>5L) len = 3L;
   tmp = len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      for (j = 7L; j>=0L; j--) {
         osic_WrINT32((uint32_t)X2C_IN(j,8,(uint8_t)(uint8_t)b[i]), 1UL);
      } /* end for */
      if (i==tmp) break;
   } /* end for */
} /* end showbits() */

static aprsstr_GHOSTSET _cnst1 = {0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL};

static void getaxudp2(pCHIP chp)
{
   uint32_t fromport;
   uint32_t ip;
   char crc2;
   char crc1;
   int32_t len;
   uint32_t slen;
   char showtext[1501];
   char rb[1501];
   char udp2[64]; /* axudp2 header */
   char cont;
   char ok0;
   struct CHIP * anonym;
   /* axudp2 */
   struct TXCONTEXT * anonym0;
   cont = chp->atx!=0;
   if (!cont) chp->atx = chp->ptx;
   len = 0L;
   for (;;) {
      if (chp->atx==0) return;
      if (chp->atx->udpbind) {
         len = udpreceive(chp->atx->udpsocket, rb, 1501L, &fromport, &ip);
         if (len>0L) break;
      }
      if (cont) {
         chp->atx = 0;
         return;
      }
      chp->atx = chp->atx->next;
   }
   { /* with */
      struct CHIP * anonym = chp;
      ok0 = 0;
      anonym->atx->ftextlen = 0UL;
      if ((len>14L && len<1501L) && ((!anonym->atx->checkip || anonym->atx->udpip==0UL) || anonym->atx->udpip==ip)) {
         crc1 = rb[len-2L];
         crc2 = rb[len-1L];
         if (!anonym->atx->rawfsk) aprsstr_AppCRC(rb, 1501ul, len-2L);
         udp2[0U] = 0;
         if (anonym->atx->rawfsk || crc1==rb[len-2L] && crc2==rb[len-1L]) {
            if (!anonym->atx->rawfsk && rb[0U]=='\001') {
               aprsstr_extrudp2(rb, 1501ul, udp2, 64ul, &len);
            }
            { /* with */
               struct TXCONTEXT * anonym0 = anonym->atx;
               slen = 0UL;
               if (anonym0->rawfsk) {
                  anonym0->ftextlen = 0UL;
                  while (anonym0->ftextlen<=3850UL && anonym0->ftextlen<(uint32_t)len) {
                     anonym0->ftext[anonym0->ftextlen] = rb[anonym0->ftextlen];
                     ++anonym0->ftextlen;
                  }
                  ok0 = 1;
               }
               else {
                  aprsstr_raw2mon(rb, 1501ul, anonym0->ftext, 3851ul, (uint32_t)(len-2L), &anonym0->ftextlen, _cnst1);
                  if (anonym0->ftextlen>0UL) --anonym0->ftextlen;
                  while (slen<=1500UL && slen<anonym0->ftextlen) {
                     showtext[slen] = anonym0->ftext[slen];
                     ++slen;
                  }
                  if (anonym0->sendfsk) {
                     hdlc(rb, 1501ul, len, (int32_t)anonym0->txdel, anonym0->ftext, 3851ul, &anonym0->ftextlen,
                anonym->atx->baud!=0L, anonym->atx->baud>0L, cont, &anonym0->scrambler);
                  }
                  ok0 = anonym0->ftextlen>2UL;
               }
               if (verb) {
                  Wrchipnum(chp, 0);
                  if (anonym0->sendfsk) {
                     if (anonym->atx->baud) {
                        if (anonym0->rawfsk) {
                           osi_WrStr(" tx raw fsk:[", 14ul);
                           showbits(anonym0->ftext, 3851ul, (int32_t)anonym0->ftextlen);
                           osi_WrStr("...] ", 6ul);
                        }
                        else if (anonym->atx->baud>0L) {
                           osi_WrStr(" tx g3ruh:", 11ul);
                        }
                        else osi_WrStr(" tx hdlc fsk:", 14ul);
                        osic_WrUINT32((uint32_t)labs(anonym->atx->baud), 1UL);
                        osi_WrStr("Bd", 3ul);
                     }
                     else osi_WrStr(" tx afsk:", 10ul);
                  }
                  else osi_WrStr(" tx:", 5ul);
                  osi_WrStr(" port:", 7ul);
                  osic_WrUINT32(anonym->atx->udpbind, 1UL);
                  osi_WrStr(" f:", 4ul);
                  osic_WrFixed(anonym->atx->mhz, 4L, 1UL);
                  if (slen>2UL) {
                     osi_WrStr(" [", 3ul);
                     WrBytes(showtext, 1501ul, (int32_t)slen);
                     osi_WrStrLn("]", 2ul);
                  }
                  else osi_WrStrLn("", 1ul);
               }
            }
         }
         else if (verb) osi_WrStrLn(" axudp crc error ", 18ul);
      }
      if (ok0) anonym->atx->fskp = 0L;
      else anonym->atx = 0;
   }
} /* end getaxudp2() */


static void setsynth(pCHIP chp, float mhz)
{
   uint32_t synt;
   mhz = mhz+mhz*chp->ppm*1.E-6f;
   synt = (uint32_t)X2C_TRUNCC(mhz*16384.0f+0.5f,0UL,X2C_max_longcard);
   scpo(chp->gpio, 6UL, synt/65536UL); /* freq high */
   scpo(chp->gpio, 7UL, synt/256UL&255UL); /* freq mid */
   scpo(chp->gpio, 8UL, synt&255UL); /* freq low */
} /* end setsynth() */


static void Setmode(pCHIP chp, uint32_t m, char check, char tx)
{
   uint32_t n;
   if (tx && chp->atx) m += 8UL*(uint32_t)(chp->atx->mhz<=779.0f);
   else m += 8UL*(uint32_t)(chp->rxmhz<=779.0f);
   n = 0UL;
   for (;;) {
      scpo(chp->gpio, 1UL, m);
      if (!check || scpi(chp->gpio, 1UL)==m) break;
      usleep(1000UL);
      ++n;
      if ((n&255UL)==0UL) {
         Wrchipnum(chp, 0);
         osi_WrStr(" try set mode:", 15ul);
         osi_WrHex(m, 1UL);
         osi_WrStr(" chip has:", 11ul);
         osi_WrHex(scpi(chp->gpio, 1UL), 1UL);
         osi_WrStrLn("", 1ul);
      }
   }
} /* end Setmode() */


static void startfsk(pCHIP chp)
{
   uint32_t pow0;
   uint32_t bd;
   /*
     IF verb THEN
       Wrchipnum(chp, FALSE);
       WrStr(" afsk: port:");
       WrCard(chp^.atx^.udpbind, 1);
       WrStr(" f:");
       WrFixed(chp^.atx^.mhz, 4, 1);
     END;
   */
   setsynth(chp, chp->atx->mhz);
   Setmode(chp, 0UL, 1, 1); /* sleep */
   /*Setmode(chp, 3, TRUE); */
   /*usleep(100000); */
   Setmode(chp, 4UL, 1, 1); /* rx to clear fifo */
   Setmode(chp, 0UL, 1, 1); /* sleep */
   pow0 = chp->atx->power;
   if (chp->boost) {
      /*  IF pow=20 THEN scpo(chp^.gpio, RegPaDac,7) END;                    (* 20dBm switch send time too long*) */
      if (pow0>17UL) pow0 = 17UL;
      else if (pow0<2UL) pow0 = 2UL;
      pow0 -= 2UL;
   }
   else if (pow0>15UL) pow0 = 15UL;
   scpo(chp->gpio, 9UL, 240UL+pow0); /* tx power */
   scpo(chp->gpio, 10UL, 41UL+64UL*(uint32_t)(chp->atx->baud==0L)); /* baseband lowpass 0.3B on afsk, pa ramp */
   /*  scpo(chp^.gpio, RegPllHf, CAST(CARDINAL, CAST(SET32, scpi(chp^.gpio, RegPllHf))*SET32{0..5}
                )); (* 75khz pll bandwidth *)  */
   if (chp->atx->baud) bd = 512000000UL/(uint32_t)labs(chp->atx->baud);
   else bd = 38787UL;
   scpo(chp->gpio, 2UL, bd/4096UL);
   scpo(chp->gpio, 3UL, bd/16UL&255UL);
   scpo(chp->gpio, 93UL, bd&15UL);
   bd = chp->atx->deviation/61UL;
   scpo(chp->gpio, 4UL, bd/256UL);
   scpo(chp->gpio, 5UL, bd&255UL);
   scpo(chp->gpio, 37UL, 0UL);
   scpo(chp->gpio, 38UL, 0UL);
   scpo(chp->gpio, 39UL, 0UL);
   scpo(chp->gpio, 48UL, 0UL);
   scpo(chp->gpio, 49UL, 64UL); /*40H*/
   scpo(chp->gpio, 50UL, 0UL);
   bd = 56UL;
   if (56UL>=chp->atx->ftextlen) bd = chp->atx->ftextlen-1UL;
   scpo(chp->gpio, 53UL, bd);
   /*chp^.atx^.fskp:=0; */
   /*WrStrLn(""); FOR i:=0 TO 3FH DO WrHex(scpi(chp^.gpio, i), 3) END; WrStrLn(""); */
   /*scpo(chp^.gpio, RegOpMode,3);   (* use if sequencer dead *) */
   /*REPEAT WrHex(scpi(chp^.gpio, RegIrqFlags1),1) UNTIL scpi(chp^.gpio, RegIrqFlags1)>=128;       (* mode ready *) */
   /*WrHex(scpi(chp^.gpio, RegSeqConfig1),3);WrStrLn("=seq"); */
   /*  scpo(chp^.gpio, RegSeqConfig1, 0B8H); (*B8*)               (* start sequencer tx on fifo thres *) */
   /*WrHex(scpi(chp^.gpio, RegSeqConfig1),3);WrStrLn("=seq"); */
   scpo(chp->gpio, 54UL, 184UL); /*B8*/ /* start sequencer tx on fifo thres */
} /* end startfsk() */


static void setppm(pCHIP chp)
{
   scpo(chp->gpio, 39UL, (uint32_t)(uint8_t)(signed char)X2C_TRUNCI(chp->ppm*0.95f+0.5f,-128,127));
                /* datarate correction */
} /* end setppm() */


static void startrx(pCHIP chp, char dcdonly)
{
   uint32_t id;
   uint32_t sf;
   uint32_t bw;
   struct CHIP * anonym;
   { /* with */
      struct CHIP * anonym = chp;
      bw = anonym->rxbw;
      sf = anonym->rxsf;
      id = anonym->netid;
      if (dcdonly) {
         /* look where we want to send next */
         bw = anonym->atx->bw;
         sf = anonym->atx->sf;
         id = anonym->atx->netid;
      }
      Setmode(chp, 128UL, 1, 0);
      scpo(anonym->gpio, 29UL, 16UL*bw+2UL+(uint32_t)chp->implicit); /* bw cr explizit */
      scpo(anonym->gpio, 12UL, 32UL*(chp->lnagain&7UL)+8UL*(anonym->lnaboost&3UL)); /* lna gain, current */
      Setmode(chp, 128UL, 0, 0); /* sleep */
      scpo(anonym->gpio, 30UL, sf*16UL+4UL); /* spread factor + crc on */
      scpo(anonym->gpio, 38UL, (uint32_t)anonym->agc*4UL+(uint32_t)anonym->optimize*8UL); /* agc txoptimize */
      scpo(anonym->gpio, 49UL, 3UL+2UL*(uint32_t)(sf==6UL)); /* rxoptimize 3, on sf=6 5 */
      scpo(anonym->gpio, 55UL, 10UL+2UL*(uint32_t)(sf==6UL));
      scpo(anonym->gpio, 57UL, id); /* syncword */
      setsynth(chp, anonym->rxmhz);
      Setmode(chp, 133UL+2UL*(uint32_t)dcdonly, 0, 0); /* continous rx or dcd detect */
   }
} /* end startrx() */


static void send(pCHIP chp)
{
   uint32_t pow0;
   uint32_t len;
   uint32_t i;
   char b[501];
   Setmode(chp, 129UL, 1, 0);
   if (chp->cfgocp>=0L) scpo(chp->gpio, 11UL, (uint32_t)(chp->cfgocp&31L));
   setsynth(chp, chp->atx->mhz);
   pow0 = chp->atx->power;
   if (chp->boost) {
      /*  IF pow=20 THEN scpo(chp^.gpio, RegPaDac,7) END;                    (* 20dBm switch send time too long*) */
      if (pow0>17UL) pow0 = 17UL;
      else if (pow0<2UL) pow0 = 2UL;
      pow0 -= 2UL;
   }
   else if (pow0>15UL) pow0 = 15UL;
   scpo(chp->gpio, 9UL, 240UL+pow0); /* tx power */
   if (chp->cfgramp>=0L) scpo(chp->gpio, 10UL, (uint32_t)(chp->cfgramp&15L));
   scpo(chp->gpio, 29UL, 16UL*chp->atx->bw+2UL*(chp->atx->cr-4UL)+(uint32_t)chp->atx->implicit); /* bw cr explizit */
   scpo(chp->gpio, 30UL, chp->atx->sf*16UL+4UL); /* spread factor + crc on */
   scpo(chp->gpio, 38UL, (uint32_t)chp->agc*4UL+(uint32_t)chp->atx->optimize*8UL); /* agc txoptimize */
   scpo(chp->gpio, 32UL, chp->atx->preamb/256UL); /* preamble length */
   scpo(chp->gpio, 33UL, chp->atx->preamb&255UL); /* preamble length */
   scpo(chp->gpio, 57UL, chp->atx->netid); /* syncword */
   b[0U] = '<'; /* for whatever */
   b[1U] = '\377'; /* for whatever */
   b[2U] = '\001'; /* for whatever */
   len = 3UL;
   i = 0UL;
   if (chp->atx->ftextlen>255UL) {
      chp->atx->ftextlen = 255UL;
      if (verb) osi_WrStrLn("---packet length stripped to 255", 33ul);
   }
   while (i<chp->atx->ftextlen && len<500UL) {
      b[len] = chp->atx->ftext[i];
      ++len;
      ++i;
   }
   /*  IF verb THEN WrStr(" [");WrBytes(b, len); WrStrLn("]") END; */
   scpo(chp->gpio, 34UL, len); /* payload length */
   scpo(chp->gpio, 13UL, scpi(chp->gpio, 14UL)); /* write pointer */
   scpio(chp->gpio, 1, 0UL, len, b, 501ul);
   scpo(chp->gpio, 18UL, 255UL); /* clear all irq */
   Setmode(chp, 131UL, 1, 1); /* transmit */
} /* end send() */


static void txfill(pCHIP chp)
{
   char b[8];
   uint32_t f;
   int32_t i;
   int32_t r;
   int32_t tmp;
   if (chp->atx) {
      for (;;) {
         f = scpi(chp->gpio, 63UL);
         /*WrHex(f,1); WrStr("=f "); */
         if ((0x20U & (uint8_t)f)) break;
         if (chp->atx->fskp && (0x40U & (uint8_t)f)) break;
         for (;;) {
            r = (int32_t)chp->atx->ftextlen-chp->atx->fskp;
            if (r>0L) break;
            getaxudp2(chp); /* more data as tx is on */
            if (chp->atx==0) break;
         }
         if (r<=0L) break;
         if (r>8L) r = 8L;
         tmp = r-1L;
         i = 0L;
         if (i<=tmp) for (;; i++) {
            b[i] = chp->atx->ftext[chp->atx->fskp];
            ++chp->atx->fskp;
            if (i==tmp) break;
         } /* end for */
         scpio(chp->gpio, 1, 0UL, (uint32_t)r, b, 8ul);
      }
   }
/*WrStr("+"); */
} /* end txfill() */


static void startdcdcheck(pCHIP chip0)
{
   scpo(chip0->gpio, 18UL, 255UL);
   startrx(chip0, 1); /* start dcd detect on tx mhz/mod */
   if (verb2) {
      osi_WrStr("start cad ", 11ul);
      osic_WrFixed(chip0->atx->mhz, 3L, 1UL);
      osi_WrStrLn("", 1ul);
   }
} /* end startdcdcheck() */


static char dcddone(pCHIP chip0, char * dcd0)
{
   if ((0x4U & (uint8_t)scpi(chip0->gpio, 18UL))==0) return 0;
   /* cad not done */
   *dcd0 = (0x1U & (uint8_t)scpi(chip0->gpio, 18UL))!=0;
   if (verb2) {
      osic_WrUINT32((uint32_t)*dcd0, 1UL);
      osi_WrStrLn(" cad done", 10ul);
   }
   return 1;
} /* end dcddone() */


static void showv2(pCHIP chip0, uint8_t bandmap)
{
   Wrchipnum(chip0, 0);
   osi_WrStr(" st:", 5ul);
   osi_WrHex(scpi(chip0->gpio, 1UL), 0UL);
   osi_WrStr(" cnt:", 6ul);
   osic_WrINT32(scpi(chip0->gpio, 23UL), 1UL);
   osi_WrStr(" rssi:", 7ul);
   osic_WrINT32(scpi(chip0->gpio, 27UL), 1UL);
   osi_WrStr(" stat:", 7ul);
   osi_WrHex(scpi(chip0->gpio, 24UL), 1UL);
   osi_WrStr(" flags:", 8ul);
   osi_WrHex(scpi(chip0->gpio, 18UL), 1UL);
   osi_WrStr(" state:", 8ul);
   osic_WrUINT32((uint32_t)chip0->state, 1UL);
   osi_WrStr(" dcd:", 6ul);
   osic_WrUINT32((uint32_t)(uint8_t)bandmap, 1UL);
   osi_WrStrLn("", 1ul);
} /* end showv2() */


static void rx(pCHIP chip0)
{
   uint32_t len;
   uint32_t syncw;
   uint32_t flags0;
   uint32_t i;
   char crcvalid;
   char hascrc;
   char rximplicit;
   int32_t afchz;
   int32_t level;
   int32_t snr;
   int32_t ferr;
   uint32_t tmp;
   rximplicit = (char)(scpi(chip0->gpio, 29UL)&1);
   hascrc = (0x40U & (uint8_t)scpi(chip0->gpio, 28UL))!=0;
   crcvalid = 0;
   if (!rximplicit) {
      flags0 = scpi(chip0->gpio, 18UL);
      crcvalid = (0x20U & (uint8_t)flags0)==0;
      ferr = (int32_t)((scpi(chip0->gpio, 40UL)&15UL)*65536UL+scpi(chip0->gpio, 41UL)*256UL+scpi(chip0->gpio, 42UL));
      if (ferr>=524288L) ferr -= 1048576L;
      afchz = (int32_t)X2C_TRUNCI((float)ferr*2.097152E-3f*_cnst[chip0->rxbw]*chip0->rxmhz*1.0526315789474E-3f,
                X2C_min_longint,X2C_max_longint);
      syncw = scpi(chip0->gpio, 57UL);
      snr = (int32_t)((signed char)scpi(chip0->gpio, 25UL)/4);
      level = (snr+(int32_t)scpi(chip0->gpio, 26UL))-164L;
      level += chip0->rssicorr;
      scpo(chip0->gpio, 13UL, scpi(chip0->gpio, 16UL));
      len = scpi(chip0->gpio, 19UL);
      if (verb) {
         Wrchipnum(chip0, 0);
         if (hascrc) {
            if (crcvalid) osi_WrStr(" crc:ok", 8ul);
            else osi_WrStr(" crc:err", 9ul);
         }
         else osi_WrStr(" crc:none", 10ul);
         osi_WrStr(" txd:", 6ul);
         osic_WrUINT32(chip0->rxtxdel, 1UL);
         osi_WrStr(" df:", 5ul);
         osic_WrINT32((uint32_t)afchz, 1UL);
         osi_WrStr(" net: ", 7ul);
         osi_WrHex(syncw, 0UL);
         osi_WrStr(" ih/crc:", 9ul);
         osic_WrINT32((uint32_t)rximplicit, 1UL);
         osic_WrINT32((uint32_t)hascrc, 1UL);
         osi_WrStr(" rssi:", 7ul);
         osic_WrINT32((uint32_t)level, 1UL);
         osi_WrStr(" snr:", 6ul);
         osic_WrINT32((uint32_t)snr, 1UL);
         osi_WrStr(" cr:", 5ul);
         osic_WrINT32(scpi(chip0->gpio, 24UL)/32UL+4UL, 1UL);
         osi_WrStr(" len:", 6ul);
         osic_WrINT32(len, 1UL);
         osi_WrStrLn("", 1ul);
      }
      if (len>5UL && len<500UL) {
         scpio(chip0->gpio, 0, 0UL, len, h, 501ul);
         tmp = len-1UL;
         i = 0UL;
         if (i<=tmp) for (;; i++) {
            if (i>=3UL && i<len) h[i-3UL] = h[i];
            if (verb) WrByte(h[i]);
            if (i==tmp) break;
         } /* end for */
         h[len-3UL] = 0;
         if ((uint8_t)h[len-4UL]<'\034') h[len-4UL] = 0;
         if (hascrc && crcvalid) {
            if (chip0->netid>255UL || chip0->netid==syncw) {
               sendaxudp2(chip0, h, 501ul, (int32_t)chip0->rxtxdel, level, snr, afchz);
            }
            else if (verb) {
               osi_WrStrLn("", 1ul);
               osi_WrStr("packet filtert as wrong sync word received=", 44ul);
               osi_WrHex(syncw, 1UL);
               osi_WrStr(" filter=", 9ul);
               osi_WrHex(chip0->netid, 1UL);
            }
         }
         if (verb) osi_WrStrLn("", 1ul);
      }
      else if (verb) osi_WrStrLn("zereo len data", 15ul);
      scpo(chip0->gpio, 18UL, 255UL);
      Setmode(chip0, 133UL, 0, 0); /* rx continous */
   }
   else if (verb) {
      Wrchipnum(chip0, 0);
      osi_WrStrLn(" implicit header", 17ul);
   }
} /* end rx() */

/* sleep */
/* fsk sleep */
/* FSR */
/* temperature change occured */
/* fsk standby */
/* trigger calibration */
/* wait until done */
/* sleep */
/* sleep */
static void freegpio(int32_t);


static void freegpio(int32_t signum)
{
   uint32_t i;
   char h0[2];
   int32_t fd;
   for (i = 0UL; i<=255UL; i++) {
      if (gpiofds[i]!=-2L) {
         aprsstr_CardToStr(i, 1UL, h0, 2ul);
         fd = osi_OpenWrite("/sys/class/gpio/unexport", 25ul); /* /sys/class/gpio/unexport */
         if (fd>=0L) {
            osi_WrBin(fd, (char *)h0, 2u/1u, 1UL);
            osic_Close(fd);
         }
      }
   } /* end for */
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   X2C_HALT((uint32_t)signum);
} /* end freegpio() */

static uint32_t flags;

static uint32_t st;

static pCHIP chip;

static char dcd;

static char txfast;

static uint8_t banddcdall;

static uint8_t banddcd;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(GPIOSET)!=32) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGTERM, freegpio);
   signal(SIGINT, freegpio);
   signal(SIGPIPE, freegpio);
   Parms();
   chip = chips;
   while (chip) {
      /* set gpios */
      dcd = scp(chip->gpio, 0, 1, 0, 0); /* NSS off */
      chip->state = ra02a_stSLEEP;
      chip = chip->next;
   }
   chip = chips;
   usleep(10000UL);
   while (chip) {
      usleep(1000UL);
      if (verb2) {
         Setmode(chip, 0UL, 1, 0);
         Showregs(chip);
         Setmode(chip, 128UL, 1, 0);
         Showregs(chip);
      }
      Setmode(chip, 0UL, 1, 0);
      scpo(chip->gpio, 54UL, 64UL); /* stop sequencer */
      scpo(chip->gpio, 54UL, 0UL); /* 83H why ever */
      /*scpo(chip^.gpio, RegFifoThresh, 1); */
      Setmode(chip, 0UL, 1, 0);
      Setmode(chip, 1UL, 1, 0);
      Setmode(chip, 4UL, 1, 0); /* clear fifo */
      Setmode(chip, 0UL, 1, 0);
      chip = chip->next;
   }
   txfast = 0;
   for (;;) {
      if (chip==0) {
         chip = chips;
         banddcdall = banddcd;
         banddcd = 0U;
         ++tcnt;
         if (txfast) usleep(loopdelayfast);
         else usleep(loopdelay);
         txfast = 0;
      }
      if (chip->atx && chip->atx->sendfsk) txfast = 1;
      ++chip->calcnt;
      st = scpi(chip->gpio, 1UL);
      if (((uint8_t)st&0x6U)==0U) {
         /* sleep or standby */
         /*WrHex(scpi(chip^.gpio, RegOpMode),1); WrStr(" "); WrHex(scpi(chip^.gpio, RegIrqFlags),1);
                WrStrLn("=sleep/stby"); */
         if (st==0UL) {
            Setmode(chip, 128UL, 1, 0);
            setppm(chip);
            if (verb) {
               Wrchipnum(chip, 0);
               osi_WrStrLn(" reseted", 9ul);
            }
         }
         if (chip->state!=ra02a_stWAITDCD) {
            if (chip->state==ra02a_stTX) chip->atx = 0;
            chip->state = ra02a_stSLEEP;
         }
      }
      if (chip->state==ra02a_stSLEEP) {
         if (chip->rxon) {
            startrx(chip, 0);
            chip->state = ra02a_stRX;
         }
      }
      /*      ELSE Setmode(chip, 80H, TRUE, FALSE) END;                        (* sleep *) */
      if (chip->rxon && chip->state==ra02a_stRX) {
         flags = scpi(chip->gpio, 18UL);
         chip->stato = chip->statu;
         chip->statu = scpi(chip->gpio, 24UL);
         if ((chip->statu&1)) {
            /* dcd */
            banddcd |= (1U<<chip->band); /* set dcd for this band */
            if (!(chip->stato&1)) chip->uso = usec();
            else if ((0x8U & (uint8_t)chip->statu) && chip->rxtxdel==0UL) {
               chip->rxtxdel = (usec()-chip->uso)/1000UL; /* header ok */
            }
         }
         if ((0x40U & (uint8_t)flags)) rx(chip);
      }
      if (chip->state==ra02a_stWAITDCD) {
         dcd = 0;
         if ((chip->atx==0 || !chip->atx->usedcd) || dcddone(chip, &dcd)) {
            if (dcd) startdcdcheck(chip);
            else {
               send(chip);
               chip->state = ra02a_stTX;
            }
         }
      }
      else if ((chip->atx && chip->state!=ra02a_stTX) && !X2C_IN(chip->band,8,banddcd|banddcdall)) {
         if (chip->atx->sendfsk) {
            startfsk(chip);
            chip->state = ra02a_stTX;
         }
         else {
            if (chip->atx->usedcd) startdcdcheck(chip);
            chip->state = ra02a_stWAITDCD;
         }
      }
      if (chip->state==ra02a_stTX && chip->atx) {
         if (chip->atx->sendfsk) txfill(chip);
         else if ((0x8U & (uint8_t)scpi(chip->gpio, 18UL))) {
            /* tx done */
            chip->atx = 0;
            Setmode(chip, 129UL, 1, 0);
         }
      }
      if (verb2 && (tcnt&7UL)==0UL) showv2(chip, banddcdall|banddcd);
      if (chip->atx==0) getaxudp2(chip);
      chip = chip->next;
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
