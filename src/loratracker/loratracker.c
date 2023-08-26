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
#define loratracker_C_
#ifndef ESP32_H_
#include "ESP32.h"
#endif
#ifndef serial2_H_
#include "serial2.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif

/* gps to mice with ra02 tx */
#define loratracker_LF "\012"

#define loratracker_CR "\015"

#define loratracker_KNOTS 1.851984

#define loratracker_FEET 3.2808398950131

#define loratracker_MISO 19

#define loratracker_MOSI 27

#define loratracker_SCK 5

#define loratracker_NSS 18

#define loratracker_STOUT 2

#define loratracker_STIN 1

#define loratracker_STATLEN 1
/* 30 */

#define loratracker_CALMASK 0

#define loratracker_NETID 0x12 

#define loratracker_XTAL 3.2E+7

#define loratracker_FSTEP 6.103515625E+1

static int32_t loratracker_MINBAUD = 490L;

static float loratracker_BWTAB[10] = {7.8f,10.4f,15.6f,20.8f,31.25f,41.7f,62.5f,125.0f,250.0f,500.0f};

#define loratracker_RegSyncWord 0x39 

#define loratracker_RegFeiMsb 0x28 

#define loratracker_RegFeiMid 0x29 

#define loratracker_RegFeiLsb 0x2A 

#define loratracker_RegPacketRssi 0x1A 

#define loratracker_RegPaRamp 0xA 

#define loratracker_RegPaDac 0x4D 

#define loratracker_RegOcp 0xB 

#define loratracker_RegPktSnrValue 0x19 

#define loratracker_RegFifoRxCurrentAd 0x10 

#define loratracker_RegOpMode 0x1 

#define loratracker_RegModemStat 0x18 

#define loratracker_RegIrqFlagsMask 0x11 

#define loratracker_RegIrqFlags 0x12 

#define loratracker_RegRxNbBytes 0x13 

#define loratracker_RegFifoAddrPtr 0xD 

#define loratracker_RegHopChannel 0x1C 

#define loratracker_RegRssiValue 0x1B 

#define loratracker_RegRxPacketCntValu 0x17 

#define loratracker_RegFifo 0x0 

#define loratracker_RegFifoTxBaseAddr 0xE 

#define loratracker_RegFrMsb 0x6 

#define loratracker_RegFrMid 0x7 

#define loratracker_RegFrLsb 0x8 

#define loratracker_RegDetectOptimice 0x31 

#define loratracker_RegDetectionThresh 0x37 

#define loratracker_RegLna 0xC 

#define loratracker_RegModemConfig3 0x26 

#define loratracker_RegModemConfig2 0x1E 

#define loratracker_RegModemConfig1 0x1D 

#define loratracker_RegPaConfig 0x9 

#define loratracker_RegPayloadLength 0x22 

#define loratracker_RegPreambleMsb 0x20 

#define loratracker_RegPreambleLsb 0x21 

#define loratracker_RegImageCal 0x3B 

#define loratracker_RegTemp 0x3C 
/*fsk */

#define loratracker_RegBitrateMsb 0x2 

#define loratracker_RegBitrateLsb 0x3 

#define loratracker_RegFdevMsb 0x4 

#define loratracker_RegFdevLsb 0x5 

#define loratracker_RegPreampleMsb 0x25 

#define loratracker_RegPreampleLsb 0x26 

#define loratracker_RegSyncConfig 0x27 

#define loratracker_RegPacketConfig1 0x30 

#define loratracker_RegPacketConfig2 0x31 

#define loratracker_RegPayloadLengthFs 0x32 

#define loratracker_RegFifoThresh 0x35 

#define loratracker_RegSeqConfig1 0x36 

#define loratracker_RegSeqConfig2 0x37 

#define loratracker_RegIrqFlags1 0x3E 

#define loratracker_RegIrqFlags2 0x3F 

#define loratracker_RegBitrateFrac 0x5D 

#define loratracker_RegRxConfig 0xD 

#define loratracker_RegPllHf 0x70 

#define loratracker_FIFOFILL 8

#define loratracker_OVERSAMP 11

#define loratracker_AFSKBAUD 13200

struct CHIP;


struct CHIP {
   uint8_t state;
   char implicit;
   char optimize;
   char rxon;
   char agc;
   char boost;
   uint32_t rxtxdel;
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
   uint32_t ocp;
   uint32_t calcnt;
   int32_t ramp;
   int32_t rssicorr;
   float rxmhz;
   float txmhz;
   float ppm;
   float symboltime;
   uint32_t txsf;
   uint32_t txbw;
   uint32_t txcr;
   uint32_t preamb;
   uint32_t power;
   uint32_t txftextlen;
   uint32_t txnetid;
   char txoptimize;
   char tximplicit;
};

struct POS;


struct POS {
   uint32_t daytime;
   uint32_t year;
   uint32_t month;
   uint32_t day;
   uint32_t sats;
   uint32_t fix;
   uint32_t satcnt;
   double lat;
   double long0;
   double speed;
   double course;
   double alt;
   char posok;
   char speedok;
   char altok;
   int32_t leapseconds;
   uint8_t timeflags;
};

static uint32_t BASE;

static uint32_t micessid;

static uint32_t count;

static struct CHIP chip;

static char withspeed;

static char verb;

static char mycall[9];

static char symt;

static char symb;

static char sumoff;

static uint32_t gpsp;

static char gpsb[251];

static struct POS pos;


static uint32_t Length(const char a[], uint32_t a_len)
{
   uint32_t i;
   i = 0UL;
   while (i<=a_len-1 && a[i]) ++i;
   return i;
} /* end Length() */


static void Append(char a[], uint32_t a_len, const char b[], uint32_t b_len)
{
   uint32_t j;
   uint32_t i;
   j = Length(a, a_len);
   i = 0UL;
   for (;;) {
      if (j>a_len-1) return;
      a[j] = b[i];
      if (a[j]==0) return;
      ++i;
      ++j;
      if (i>b_len-1) {
         if (j<=a_len-1) a[j] = 0;
         return;
      }
   }
} /* end Append() */


static uint32_t truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static void skip(const char b[], uint32_t b_len, uint32_t * p, uint32_t len)
{
   while (*p<len && b[*p]!=',') ++*p;
   if (*p<len) ++*p;
} /* end skip() */


static char getnum(const char b[], uint32_t b_len, uint32_t * p, uint32_t len, uint32_t * n)
{
   if ((*p<len && (uint8_t)b[*p]>='0') && (uint8_t)b[*p]<='9') {
      *n = (uint32_t)(uint8_t)b[*p]-48UL;
      ++*p;
      return 1;
   }
   *n = 0UL;
   return 0;
} /* end getnum() */


static void decodeline(const char b[], uint32_t b_len, uint32_t len, struct POS * p)
{
   uint32_t msg;
   uint32_t msgs;
   uint32_t n;
   uint32_t i;
   double div0;
   char sign;
   if (b[0UL]=='$') {
      if (b[1UL]=='G') {
         if ((b[3UL]=='R' && b[4UL]=='M') && b[5UL]=='C') {
            i = 7UL;
            if (getnum(b, b_len, &i, len, &n)) p->daytime = n*36000UL;
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->daytime += n*3600UL;
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->daytime += n*600UL;
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->daytime += n*60UL;
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->daytime += n*10UL;
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->daytime += n;
            else return;
            skip(b, b_len, &i, len);
            if (b[i]!='A') return;
            skip(b, b_len, &i, len);
            if (getnum(b, b_len, &i, len, &n)) p->lat = (double)(float)(n*10UL);
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->lat = p->lat+(double)(float)n;
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,6.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,60.0f));
            }
            else return;
            if (b[i]=='.') ++i;
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,600.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,6000.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,60000.0f));
            }
            if (getnum(b, b_len, &i, len, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,6.E+5f));
            }
            skip(b, b_len, &i, len);
            if (b[i]=='S') p->lat = -p->lat;
            else if (b[i]!='N') return;
            skip(b, b_len, &i, len);
            if (getnum(b, b_len, &i, len, &n)) p->long0 = (double)(float)(n*100UL);
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(float)(n*10UL);
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) p->long0 = p->long0+(double)(float)n;
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,6.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,60.0f));
            }
            else return;
            if (b[i]=='.') ++i;
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,600.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,6000.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,60000.0f));
            }
            if (getnum(b, b_len, &i, len, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,6.E+5f));
            }
            skip(b, b_len, &i, len);
            if (b[i]=='W') p->long0 = -p->long0;
            else if (b[i]!='E') return;
            skip(b, b_len, &i, len);
            p->speed = 0.0;
            while (getnum(b, b_len, &i, len, &n)) {
               p->speed = p->speed*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len, &n)) {
                  p->speed = p->speed+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            p->speed = p->speed*1.851984; /* knots to km/h */
            skip(b, b_len, &i, len);
            p->course = 0.0;
            while (getnum(b, b_len, &i, len, &n)) {
               p->course = p->course*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len, &n)) {
                  p->course = p->course+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            p->posok = 1;
         }
         else if ((b[3UL]=='G' && b[4UL]=='G') && b[5UL]=='A') {
            i = 7UL;
            skip(b, b_len, &i, len);
            skip(b, b_len, &i, len);
            skip(b, b_len, &i, len);
            skip(b, b_len, &i, len);
            skip(b, b_len, &i, len);
            if (getnum(b, b_len, &i, len, &n)) p->fix = n;
            else return;
            skip(b, b_len, &i, len);
            p->sats = 0UL;
            while (getnum(b, b_len, &i, len, &n)) p->sats = p->sats*10UL+n;
            skip(b, b_len, &i, len);
            skip(b, b_len, &i, len);
            p->alt = 0.0;
            if (b[i]=='-') {
               sign = 1;
               ++i;
            }
            else sign = 0;
            while (getnum(b, b_len, &i, len, &n)) {
               p->altok = 1;
               p->alt = p->alt*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len, &n)) {
                  p->alt = p->alt+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            if (sign) p->alt = -p->alt;
         }
         else if ((b[3UL]=='G' && b[4UL]=='S') && b[5UL]=='V') {
            /* sat num/az/el/dB */
            i = 7UL;
            if (!getnum(b, b_len, &i, len, &msgs)) return;
            skip(b, b_len, &i, len);
            if ((!getnum(b, b_len, &i, len, &msg) || msg==0UL) || msg>3UL) {
               return;
            }
            skip(b, b_len, &i, len);
            if (!getnum(b, b_len, &i, len, &n)) return;
            if (!getnum(b, b_len, &i, len, &p->satcnt)) return;
            p->satcnt += n*10UL;
         }
      }
   }
} /* end decodeline() */


static char Hex(uint32_t d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


static char checksum(const char b[], uint32_t b_len, uint32_t len)
{
   uint32_t i;
   uint8_t cs;
   char ok0;
   ok0 = 1;
   i = 1UL;
   cs = 0U;
   while (i<len && b[i]!='*') {
      cs = cs^(uint8_t)(uint8_t)b[i];
      ++i;
   }
   if (i+2UL>=len) ok0 = 0;
   if (ok0) {
      if (b[i+1UL]!=Hex((uint32_t)cs/16UL) || b[i+2UL]!=Hex((uint32_t)cs)) ok0 = 0;
   }
   return ok0;
} /* end checksum() */


static void delay(void)
{
   usleep(20UL);
} /* end delay() */


static char scp(char rd, char nss, char sck, char mosi)
{
   char res;
   res = 0;
   if (rd) res = ESP32_digitalRead(19UL);
   ESP32_digitalWrite(27UL, mosi);
   ESP32_digitalWrite(5UL, sck);
   ESP32_digitalWrite(18UL, nss);
   return res;
} /* end scp() */


static void scpio(char wr, uint32_t a, uint32_t len, char s[], uint32_t s_len)
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
         rd += rd+(uint32_t)scp(!wr && i<=7UL, i==0UL && j==len-1UL, 0, X2C_IN(i,32,(uint32_t)d));
         delay();
         if (i==0UL) break;
         b = scp(0, 0, 1, X2C_IN(i,32,(uint32_t)d));
         delay();
         --i;
      }
      if (!wr) s[j] = (char)rd;
      ++j;
      i = 8UL;
   }
} /* end scpio() */


static void scpo(uint32_t a, uint32_t d)
/* 1 byte out */
{
   char s;
   s = (char)d;
   scpio(1, a, 1UL, (char *) &s, 1u/1u);
} /* end scpo() */


static uint32_t scpi(uint32_t a)
/* 1 byte in */
{
   char s;
   scpio(0, a, 1UL, (char *) &s, 1u/1u);
   return (uint32_t)(uint8_t)s;
} /* end scpi() */


static void setsynth(float mhz)
{
   uint32_t synt;
   mhz = mhz+mhz*chip.ppm*1.E-6f;
   synt = (uint32_t)X2C_TRUNCC(mhz*16384.0f+0.5f,0UL,X2C_max_longcard);
   scpo(6UL, synt/65536UL); /* freq high */
   scpo(7UL, synt/256UL&255UL); /* freq mid */
   scpo(8UL, synt&255UL); /* freq low */
} /* end setsynth() */


static void Setmode(uint32_t m, char check, char tx)
{
   uint32_t n;
   if (tx) m += 8UL*(uint32_t)(chip.txmhz<=779.0f);
   else m += 8UL*(uint32_t)(chip.rxmhz<=779.0f);
   n = 0UL;
   for (;;) {
      scpo(1UL, m);
      if (!check || scpi(1UL)==m) break;
      usleep(1000UL);
   }
} /* end Setmode() */

/* datarate correction */
/* look where we want to send next */
/* bw cr explizit */
/* lna gain, current */
/* sleep */
/* spread factor + crc on */
/* agc txoptimize */
/* rxoptimize 3, on sf=6 5 */
/* syncword */
/* continous rx or dcd detect */

static void send(const char ftext[], uint32_t ftext_len)
{
   uint32_t pow0;
   uint32_t len;
   uint32_t i;
   char b[256];
   Setmode(129UL, 1, 0);
   if (chip.ocp>=0UL) scpo(11UL, chip.ocp&31UL);
   setsynth(chip.txmhz);
   pow0 = chip.power;
   if (chip.boost) {
      if (pow0>17UL) pow0 = 17UL;
      else if (pow0<2UL) pow0 = 2UL;
      pow0 -= 2UL;
   }
   else if (pow0>15UL) pow0 = 15UL;
   scpo(9UL, 240UL+pow0); /* tx power */
   if (chip.ramp>=0L) scpo(10UL, (uint32_t)(chip.ramp&15L));
   scpo(29UL, 16UL*chip.txbw+2UL*(chip.txcr-4UL)+(uint32_t)chip.tximplicit); /* bw cr explizit */
   scpo(30UL, chip.txsf*16UL+4UL); /* spread factor + crc on */
   scpo(38UL, (uint32_t)chip.agc*4UL+(uint32_t)chip.txoptimize*8UL); /* agc txoptimize */
   scpo(32UL, chip.preamb/256UL); /* preamble length */
   scpo(33UL, chip.preamb&255UL); /* preamble length */
   scpo(57UL, chip.txnetid); /* syncword */
   b[0U] = '<'; /* for whatever */
   b[1U] = '\377'; /* for whatever */
   b[2U] = '\001'; /* for whatever */
   len = 3UL;
   i = 0UL;
   while ((i<=ftext_len-1 && ftext[i]) && len<255UL) {
      b[len] = ftext[i];
      ++len;
      ++i;
   }
   scpo(34UL, len); /* payload length */
   scpo(13UL, scpi(14UL)); /* write pointer */
   scpio(1, 0UL, len, b, 256ul);
   scpo(18UL, 255UL); /* clear all irq */
   Setmode(131UL, 1, 1); /* transmit */
/*
PROCEDURE rx;
VAR i, flags, syncw, len:CARDINAL;
    rximplicit, hascrc, crcvalid:BOOLEAN;
    ferr, snr, level, afchz:INTEGER;
BEGIN
  rximplicit:=ODD(scpi(RegModemConfig1));
  hascrc:=6 IN CAST(SET8, scpi(RegHopChannel));
  crcvalid:=FALSE;
  IF NOT rximplicit THEN
    flags:=scpi(RegIrqFlags);
    crcvalid:=NOT (5 IN CAST(SET8, flags));
    ferr:=(scpi(RegFeiMsb) MOD 16)*10000H
        + scpi(RegFeiMid)*100H + scpi(RegFeiLsb);
    IF ferr>=80000H THEN ferr:=ferr-100000H END;
    afchz:=VAL(INTEGER, VAL(REAL,ferr)*(16777216.0/250.0/XTAL)*BWTAB[chip^.rxbw]*chip^.rxmhz*(0.001/0.95));
    syncw:=scpi(RegSyncWord);
    snr:=CAST(INT8, scpi(RegPktSnrValue)) DIV 4;
    level:=snr+VAL(INTEGER, scpi(RegPacketRssi))-164;
    INC(level, chip^.rssicorr); 
    scpo(RegFifoAddrPtr, scpi(RegFifoRxCurrentAddr));
    len:=scpi(RegRxNbBytes);
    IF verb THEN
      IF hascrc THEN
        IF crcvalid THEN WrStr(" crc:ok") ELSE WrStr(" crc:err") END;
      ELSE WrStr(" crc:none") END;
      WrStr(" txd:"); WrCard(chip^.rxtxdel,1);
      WrStr(" df:");WrInt(afchz,1);
      WrStr(" net: ");WrHex(syncw, 0);
      WrStr(" ih/crc:");WrInt(ORD(rximplicit),1);WrInt(ORD(hascrc),1);
      WrStr(" rssi:");WrInt(level, 1);
      WrStr(" snr:");WrInt(snr, 1);
      WrStr(" cr:");WrInt(scpi(RegModemStat) DIV 32+4, 1);
      WrStr(" len:"); WrInt(len, 1); WrStrLn(""); 
    END;

    IF (len>5) & (len<HIGH(h)) THEN
      scpio(FALSE,RegFifo,len,h);
      FOR i:=0 TO len-1 DO
        IF (i>=3) & (i<len) THEN h[i-3]:=h[i] END;
        IF verb THEN WrByte(h[i]) END;
      END;
      h[len-3]:=0C;
      IF h[len-4]<CHR(1CH) THEN h[len-4]:=0C END;              (* remove trailing LF *)
      IF hascrc & crcvalid THEN
        IF (chip^.netid>255) OR (chip^.netid=syncw) THEN
          sendaxudp2(chip, h, chip^.rxtxdel, level, snr, afchz);
        ELSIF verb THEN
          WrStrLn("");
          WrStr("packet filtert as wrong sync word received="); WrHex(syncw,1);
          WrStr(" filter="); WrHex(chip^.netid, 1);
        END;
      END;
      IF verb THEN WrStrLn("") END;

    ELSIF verb THEN WrStrLn("zereo len data") END;
    scpo(RegIrqFlags,255);
    Setmode(chip, 80H+5, FALSE, FALSE);                                    (* rx continous *)
  ELSIF verb THEN WrStrLn(" implicit header") END;
END rx; 
*/
} /* end send() */


static char num(uint32_t n)
{
   return (char)(n%10UL+48UL);
} /* end num() */


static uint32_t dao91(double x)
/* radix91(xx/1.1) of dddmm.mmxx */
{
   double a;
   a = fabs(x);
   return ((truncc((a-(double)truncc(a))*6.E+5)%100UL)*20UL+11UL)/22UL;
} /* end dao91() */


static void sendaprs(double lat, double long0, double alt, double course, double speed,
                uint32_t comp0, char withspd, char withalt, char dao, char com,
                char comm[], uint32_t comm_len, const char viapath[], uint32_t viapath_len)
{
   char b[251];
   uint32_t micdest;
   uint32_t nl;
   uint32_t n;
   uint32_t i;
   double a;
   char tmp;
   X2C_PCOPY((void **)&comm,comm_len);
   /* OE0AAA-9>APRS,RELAY,WIDE2-2:!4805.44N/01333.64E>325/016/A=001824 */
   b[0] = 0;
   Append(b, 251ul, mycall, 9ul);
   micdest = Length(b, 251ul)+1UL;
   Append(b, 251ul, ">APLT01", 8ul);
   if (micessid>0UL) {
      Append(b, 251ul, "-", 2ul);
      Append(b, 251ul, (char *)(tmp = (char)(micessid+48UL),&tmp), 1u/1u);
   }
   if (viapath[0UL]) {
      Append(b, 251ul, ",", 2ul);
      Append(b, 251ul, viapath, viapath_len);
   }
   if (comp0==0UL) {
      /* uncompressed */
      Append(b, 251ul, ":!", 3ul);
      i = Length(b, 251ul);
      a = fabs(lat);
      n = truncc(a);
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncc((a-(double)n)*6000.0);
      b[i] = num(n/1000UL);
      ++i;
      b[i] = num(n/100UL);
      ++i;
      b[i] = '.';
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      if (lat>=0.0) b[i] = 'N';
      else b[i] = 'S';
      ++i;
      b[i] = symt;
      ++i;
      a = fabs(long0);
      n = truncc(a);
      b[i] = num(n/100UL);
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncc((a-(double)n)*6000.0);
      b[i] = num(n/1000UL);
      ++i;
      b[i] = num(n/100UL);
      ++i;
      b[i] = '.';
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      if (long0>=0.0) b[i] = 'E';
      else b[i] = 'W';
      ++i;
      b[i] = symb;
      ++i;
      if (withspd) {
         n = truncc(course+1.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
         b[i] = '/';
         ++i;
         n = truncc(speed*5.3996146834962E-1+0.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
      if (withalt) {
         b[i] = '/';
         ++i;
         b[i] = 'A';
         ++i;
         b[i] = '=';
         ++i;
         n = truncc(fabs(alt*3.2808398950131+0.5));
         if (alt>=0.0) b[i] = num(n/100000UL);
         else b[i] = '-';
         ++i;
         b[i] = num(n/10000UL);
         ++i;
         b[i] = num(n/1000UL);
         ++i;
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
   }
   else if (comp0==1UL) {
      /* compressed */
      Append(b, 251ul, ":!", 3ul);
      i = Length(b, 251ul);
      b[i] = symt;
      ++i;
      if (lat<90.0) n = truncc((90.0-lat)*3.80926E+5);
      else n = 0UL;
      b[i] = (char)(33UL+n/753571UL);
      ++i;
      b[i] = (char)(33UL+(n/8281UL)%91UL);
      ++i;
      b[i] = (char)(33UL+(n/91UL)%91UL);
      ++i;
      b[i] = (char)(33UL+n%91UL);
      ++i;
      if (long0>(-180.0)) n = truncc((180.0+long0)*1.90463E+5);
      else n = 0UL;
      b[i] = (char)(33UL+n/753571UL);
      ++i;
      b[i] = (char)(33UL+(n/8281UL)%91UL);
      ++i;
      b[i] = (char)(33UL+(n/91UL)%91UL);
      ++i;
      b[i] = (char)(33UL+n%91UL);
      ++i;
      b[i] = symb;
      ++i;
      if (withspd) {
         b[i] = (char)(33UL+truncc(course)/4UL);
         ++i;
         b[i] = (char)(33UL+truncc((double)(osic_ln((float)(speed*5.3996146834962E-1+1.0))
                *1.29935872129E+1f)));
         ++i;
         b[i] = '_';
         ++i;
      }
      else if (withalt) {
         if (alt*3.2808398950131>1.0) {
            n = truncc((double)(osic_ln((float)(alt*3.2808398950131))*500.5f));
         }
         else n = 0UL;
         if (n>=8281UL) n = 8280UL;
         b[i] = (char)(33UL+n/91UL);
         ++i;
         b[i] = (char)(33UL+n%91UL);
         ++i;
         b[i] = 'W';
         ++i;
      }
      else {
         b[i] = ' ';
         ++i;
         b[i] = ' ';
         ++i;
         b[i] = '_';
         ++i;
      }
      if (withspeed && withalt) {
         b[i] = '/';
         ++i;
         b[i] = 'A';
         ++i;
         b[i] = '=';
         ++i;
         n = truncc(alt*3.2808398950131+0.5);
         if (alt>=0.0) b[i] = num(n/10000UL);
         else b[i] = '-';
         ++i;
         b[i] = num(n/10000UL);
         ++i;
         b[i] = num(n/1000UL);
         ++i;
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
   }
   else if (comp0==2UL) {
      /* mic-e */
      Append(b, 251ul, ":`", 3ul);
      i = micdest;
      nl = truncc(fabs(long0));
      n = truncc(fabs(lat));
      b[i] = (char)(80UL+n/10UL);
      ++i;
      b[i] = (char)(80UL+n%10UL);
      ++i;
      n = truncc((fabs(lat)-(double)n)*6000.0);
      b[i] = (char)(80UL+n/1000UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(lat>=0.0)+(n/100UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(nl<10UL || nl>=100UL)+(n/10UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(long0<0.0)+n%10UL);
      i = Length(b, 251ul);
      if (nl<10UL) b[i] = (char)(nl+118UL);
      else if (nl>=100UL) {
         if (nl<110UL) b[i] = (char)(nl+8UL);
         else b[i] = (char)(nl-72UL);
      }
      else b[i] = (char)(nl+28UL);
      ++i;
      nl = truncc((fabs(long0)-(double)nl)*6000.0); /* long min*100 */
      n = nl/100UL;
      if (n<10UL) n += 60UL;
      b[i] = (char)(n+28UL);
      ++i;
      b[i] = (char)(nl%100UL+28UL);
      ++i;
      n = truncc(speed*5.3996146834962E-1+0.5);
      b[i] = (char)(n/10UL+28UL);
      ++i;
      nl = truncc(course);
      b[i] = (char)(32UL+(n%10UL)*10UL+nl/100UL);
      ++i;
      b[i] = (char)(28UL+nl%100UL);
      ++i;
      b[i] = symb;
      ++i;
      b[i] = symt;
      ++i;
      if (withalt) {
         if (alt>(-1.E+4)) n = truncc(alt+10000.5);
         else n = 0UL;
         b[i] = (char)(33UL+(n/8281UL)%91UL);
         ++i;
         b[i] = (char)(33UL+(n/91UL)%91UL);
         ++i;
         b[i] = (char)(33UL+n%91UL);
         ++i;
         b[i] = '}';
         ++i;
      }
   }
   if (dao) {
      b[i] = '!';
      ++i;
      b[i] = 'w';
      ++i;
      b[i] = (char)(33UL+dao91(lat));
      ++i;
      b[i] = (char)(33UL+dao91(long0));
      ++i;
      b[i] = '!';
      ++i;
   }
   b[i] = 0;
   if (com) {
      /*    beaconmacros(comm); */
      Append(b, 251ul, comm, comm_len);
   }
   /*  Append(b, CR+LF); */
   send(b, 251ul);
   X2C_PFREE(comm);
} /* end sendaprs() */


static void gpsbyte(char c)
{
   if (c=='\015' || c=='\012') {
      if (gpsp>0UL) {
         pos.posok = 0;
         if (sumoff || checksum(gpsb, 251ul, gpsp)) {
            decodeline(gpsb, 251ul, gpsp, &pos);
            if (pos.posok) {
               ++count;
               if (count>10UL) {
                  sendaprs(pos.lat, pos.long0, pos.alt, pos.course, pos.speed, 2UL, 1, 1, 0, 1, "test", 5ul, "", 1ul);
                  count = 0UL;
               }
            }
         }
         else pos.posok = 0;
      }
      gpsp = 0UL;
   }
   else if ((uint8_t)c>' ' && gpsp<250UL) {
      gpsb[gpsp] = c;
      ++gpsp;
   }
} /* end gpsbyte() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   osi_BEGIN();
   serial2_BEGIN();
   ESP32_BEGIN();
   strncpy(mycall,"OE5DXL-8",9u);
   micessid = 1UL;
   symt = '/';
   symb = 'b';
   withspeed = 1;
   sumoff = 0;
   memset((char *) &chip,(char)0,sizeof(struct CHIP));
   { /* with */
      struct CHIP * anonym = &chip;
      anonym->implicit = 0;
      anonym->optimize = 1;
      anonym->agc = 1;
      anonym->boost = 1;
      anonym->lnaboost = 3UL;
      anonym->lnagain = 1UL;
      anonym->rxsf = 12UL;
      anonym->rxbw = 7UL;
      anonym->netid = 18UL;
      anonym->rxmhz = 433.775f;
      anonym->txmhz = 433.775f;
      anonym->ppm = 0.0f;
      anonym->txsf = 12UL;
      anonym->txbw = 7UL;
      anonym->txcr = 5UL;
      anonym->preamb = 8UL;
      anonym->power = 2UL;
      anonym->txnetid = 18UL;
      anonym->txoptimize = 1;
   }
   if (chip.rxmhz<400.0f) chip.band = 1UL;
   else if (chip.rxmhz<500.0f) chip.band = 2UL;
   else chip.band = 3UL;
   count = 0UL;
   gpsp = 0UL;
   verb = 1;
   BASE = 888UL;
   ESP32_pinMode(19UL, 0UL);
   ESP32_pinMode(27UL, 1UL);
   ESP32_pinMode(5UL, 1UL);
   ESP32_pinMode(18UL, 1UL);
   ESP32_uart_set_pin(2L, -1L, 34L, -1L, -1L);
   serial2_begin(9600UL);
   for (;;) gpsbyte(serial2_read());
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
