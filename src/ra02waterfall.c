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
#define ra02waterfall_C_
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <signal.h>
#include <math.h>
#ifndef pngwritepalette_H_
#include "pngwritepalette.h"
#endif

/* gpio or lpt to scp ra02 radio module(s) spectrum/waterfall */
#define ra02waterfall_LF "\012"

#define ra02waterfall_STOUT 2

#define ra02waterfall_STIN 1

#define ra02waterfall_STATLEN 1
/* 30 */

#define ra02waterfall_CALMASK 0

#define ra02waterfall_NETID 0x12 

#define ra02waterfall_XTAL 3.2E+7

#define ra02waterfall_FSTEP 6.103515625E+1

static int32_t ra02waterfall_MINBAUD = 490L;

static float ra02waterfall_BWTAB[10] = {7.8f,10.4f,15.6f,20.8f,31.25f,41.7f,62.5f,125.0f,250.0f,500.0f};

#define ra02waterfall_RegSyncWord 0x39 

#define ra02waterfall_RegFeiMsb 0x28 

#define ra02waterfall_RegFeiMid 0x29 

#define ra02waterfall_RegFeiLsb 0x2A 

#define ra02waterfall_RegPacketRssiLor 0x1A 

#define ra02waterfall_RegPaRamp 0xA 

#define ra02waterfall_RegPaDac 0x4D 

#define ra02waterfall_RegOcp 0xB 

#define ra02waterfall_RegPktSnrValue 0x19 

#define ra02waterfall_RegFifoRxCurrent 0x10 

#define ra02waterfall_RegOpMode 0x1 

#define ra02waterfall_RegModemStat 0x18 

#define ra02waterfall_RegIrqFlagsMask 0x11 

#define ra02waterfall_RegIrqFlags 0x12 

#define ra02waterfall_RegRxNbBytes 0x13 

#define ra02waterfall_RegFifoAddrPtr 0xD 

#define ra02waterfall_RegHopChannel 0x1C 

#define ra02waterfall_RegRssiValueLora 0x1B 

#define ra02waterfall_RegRxPacketCntVa 0x17 

#define ra02waterfall_RegFifo 0x0 

#define ra02waterfall_RegFifoTxBaseAdd 0xE 

#define ra02waterfall_RegFrMsb 0x6 

#define ra02waterfall_RegFrMid 0x7 

#define ra02waterfall_RegFrLsb 0x8 

#define ra02waterfall_RegDetectOptimic 0x31 

#define ra02waterfall_RegDetectionThre 0x37 

#define ra02waterfall_RegLna 0xC 

#define ra02waterfall_RegModemConfig3 0x26 

#define ra02waterfall_RegModemConfig2 0x1E 

#define ra02waterfall_RegModemConfig1 0x1D 

#define ra02waterfall_RegPaConfig 0x9 

#define ra02waterfall_RegPayloadLength 0x22 

#define ra02waterfall_RegPreambleMsb 0x20 

#define ra02waterfall_RegPreambleLsb 0x21 

#define ra02waterfall_RegInvertIQ 0x33 

#define ra02waterfall_RegInvertIQ2 0x3B 

#define ra02waterfall_RegTemp 0x3C 

#define ra02waterfall_RegHighBwOptimiz 0x36 

#define ra02waterfall_RegHighBwOptimi0 0x3A 
/*fsk */

#define ra02waterfall_RegBitrateMsb 0x2 

#define ra02waterfall_RegBitrateLsb 0x3 

#define ra02waterfall_RegFdevMsb 0x4 

#define ra02waterfall_RegFdevLsb 0x5 

#define ra02waterfall_RegPreampleMsb 0x25 

#define ra02waterfall_RegPreampleLsb 0x26 

#define ra02waterfall_RegSyncConfig 0x27 

#define ra02waterfall_RegPacketConfig1 0x30 

#define ra02waterfall_RegPacketConfig2 0x31 

#define ra02waterfall_RegPayloadLengt0 0x32 

#define ra02waterfall_RegFifoThresh 0x35 

#define ra02waterfall_RegSeqConfig1 0x36 

#define ra02waterfall_RegSeqConfig2 0x37 

#define ra02waterfall_RegIrqFlags1 0x3E 

#define ra02waterfall_RegIrqFlags2 0x3F 

#define ra02waterfall_RegBitrateFrac 0x5D 

#define ra02waterfall_RegRxConfig 0xD 

#define ra02waterfall_RegPllHf 0x70 

#define ra02waterfall_RegPllHop 0x44 

#define ra02waterfall_RegRssiConfig 0xE 

#define ra02waterfall_RegRssiValueFsk 0x11 

#define ra02waterfall_RegRxBw 0x12 

#define ra02waterfall_RegAfcBw 0x13 

#define ra02waterfall_RegAfcFei 0x1A 

#define ra02waterfall_RegAfcMsbFsk 0x1B 

#define ra02waterfall_RegAfcLsbFsk 0x1C 

#define ra02waterfall_RegFeiMsbFsk 0x1D 

#define ra02waterfall_RegFeiLsbFsk 0x1E 

#define ra02waterfall_RegPreambleDetec 0x1F 

#define ra02waterfall_RegFifoRxBaseAdd 0xF 

#define ra02waterfall_RegSyncValue1 0x28 

#define ra02waterfall_RegOokPeak 0x14 

#define ra02waterfall_RegImageCal 0x3B 

#define ra02waterfall_FIFOFILL 8

#define ra02waterfall_OVERSAMP 11

#define ra02waterfall_AFSKBAUD 13200

#define ra02waterfall_AFSKRXSAMP 19200

#define ra02waterfall_GPIOFN "/sys/class/gpio"

#define ra02waterfall_GPIOEXPORT "/sys/class/gpio/export"

#define ra02waterfall_GPIOUNEXPORT "/sys/class/gpio/unexport"

#define ra02waterfall_GPIOX "/sys/class/gpio/gpio"

#define ra02waterfall_GPIODIRECTION "/direction"

#define ra02waterfall_GPIOIN "in"

#define ra02waterfall_GPIOOUT "out"

#define ra02waterfall_GPIODATA "/value"

#define ra02waterfall_GPIOS 1024

#define ra02waterfall_PALETTELEN 256

#define ra02waterfall_SPECWHITE 15

#define ra02waterfall_SPECLINES 64
/* color num of dB lines in spectrum */

#define ra02waterfall_WHITE 255

#define ra02waterfall_LOWMHZ 137.0

#define ra02waterfall_HIGHMHZ 1020.0

struct GPIO;


struct GPIO {
   uint32_t ceN;
   uint32_t mosiN;
   uint32_t misoN;
   uint32_t sckN;
   int32_t ceFD;
   int32_t mosiFD;
   int32_t misoFD;
   int32_t sckFD;
   char misoFN[100];
};

struct CHIP;


struct CHIP {
   struct GPIO gpio;
   uint8_t state;
   uint32_t band;
   uint32_t mso;
   int32_t rssicorr;
   float rxmhz0;
   float rxmhz1;
   float ppm;
   char lnaboost;
};

typedef uint32_t GPIOSET[32];


struct IMAGELINE {
   uint8_t * Adr;
   size_t Len0;
};

typedef struct IMAGELINE * pIMAGELINE;


struct IMAGE {
   pIMAGELINE * Adr;
   size_t Len0;
};

#define ra02waterfall_CHARX 6

static uint8_t ra02waterfall_CHARGEN[66] = {0x7CU,0x82U,0x82U,0x82U,0x7CU,0U,0U,0x42U,0xFEU,0x2U,0U,0U,0x46U,0x8AU,
                0x92U,0x92U,0x66U,0U,0x44U,0x92U,0x92U,0x92U,0x6CU,0U,0x18U,0x28U,0x4AU,0xFEU,0xAU,0U,0xE4U,0xA2U,0xA2U,
                0xA2U,0x9CU,0U,0x3CU,0x52U,0x92U,0x92U,0xCU,0U,0U,0x80U,0x8EU,0x90U,0xE0U,0U,0x6CU,0x92U,0x92U,0x92U,
                0x6CU,0U,0x60U,0x92U,0x92U,0x94U,0x78U,0U,0U,0x6U,0x6U,0U,0U,0U}; /* 0123456789. */

static uint32_t i;

static uint32_t xsize;

static uint32_t ysize;

static uint32_t gbp;

static uint32_t ftablen;

static uint32_t linetime;

static uint32_t baud;

static uint32_t gpsdir;

static uint32_t smooth;

static uint32_t specy;

static uint32_t roll;

static float gpslat;

static float gpslong;

static int32_t gpsfd;

static int32_t jfd;

static char usbrobust;

static char timestamp;

static char withhead;

static char transp;

static char verb;

static char verb2;

static struct IMAGE * spimage;

static struct IMAGE * headline;

static struct IMAGE * image;

static char ttyfname[1024];

static char imagefn[1024];

static char tmpfn[1024];

static char gb[1024];

static GPIOSET gpostate;

static int32_t gpiofds[1024];

static struct CHIP chip;

static struct PNGPALETTE palette[256];

static uint8_t transpaency[256];

static char mirrory;

static pLINE blackline;

struct _0;


struct _0 {
   float f;
   uint32_t bw;
};

static struct _0 ftab[4096];


static void Error(const char text[], uint32_t text_len)
{
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
} /* end Error() */


static char hex(uint32_t n)
{
   n = n&15UL;
   if (n<=9UL) return (char)(n+48UL);
   return (char)(n+55UL);
} /* end hex() */


static void WrHex(uint32_t x0, uint32_t digits, uint32_t len)
{
   uint32_t i0;
   char s[256];
   if (digits>255UL) digits = 255UL;
   i0 = digits;
   while (i0<len && i0<255UL) {
      s[i0] = ' ';
      ++i0;
   }
   s[i0] = 0;
   while (digits>0UL) {
      --digits;
      s[digits] = hex(x0);
      x0 = x0/16UL;
   }
   osi_WrStr(s, 256ul);
} /* end WrHex() */

/*  term : termios; */
/*  bd   : INTEGER; */

static void SetComMode(int32_t fd, uint32_t baud0)
{
   if (osi_setttybaudraw(fd, baud0)<0L) {
      if (verb) osi_WrStrLn("cannot config tty", 18ul);
   }
} /* end SetComMode() */


static void opentty(void)
{
   gpsfd = osi_OpenNONBLOCK(ttyfname, 1024ul);
   if (gpsfd>=0L) SetComMode(gpsfd, baud);
   else {
      if (!usbrobust) Error("tty open", 9ul);
      if (verb) osi_WrStrLn("tty open error", 15ul);
   }
} /* end opentty() */


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


static void rdgps(void)
{
   int32_t res;
   uint32_t n;
   uint32_t eol;
   uint32_t i0;
   float tmplong;
   float tmplat;
   float course;
   char * p;
   p = (char *) &gb[gbp];
   res = osi_RdBin(gpsfd, p, 65536ul, 1024UL-gbp);
   if (res<=0L) {
      if (res==0L) {
         osic_Close(gpsfd);
         opentty();
      }
   }
   else {
      gbp += (uint32_t)res;
      for (;;) {
         eol = 0UL;
         while (eol<gbp && (uint8_t)gb[eol]>=' ') ++eol;
         while (eol<gbp && (uint8_t)gb[eol]<' ') ++eol;
         if (eol>=gbp) break;
         i0 = 0UL;
         for (;;) {
            if (((((i0+20UL<eol && gb[i0]=='$') && gb[i0+1UL]=='G') && gb[i0+3UL]=='R') && gb[i0+4UL]=='M')
                && gb[i0+5UL]=='C') {
               i0 += 6UL;
               skip(gb, 1024ul, &i0, eol);
               skip(gb, 1024ul, &i0, eol);
               if (gb[i0]!='A') break;
               skip(gb, 1024ul, &i0, eol);
               if (getnum(gb, 1024ul, &i0, eol, &n)) tmplat = (float)(n*10UL);
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) tmplat = tmplat+(float)n;
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplat = tmplat+X2C_DIVR((float)n,6.0f);
               }
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplat = tmplat+X2C_DIVR((float)n,60.0f);
               }
               else break;
               if (gb[i0]=='.') ++i0;
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplat = tmplat+X2C_DIVR((float)n,600.0f);
               }
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplat = tmplat+X2C_DIVR((float)n,6000.0f);
               }
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplat = tmplat+X2C_DIVR((float)n,60000.0f);
               }
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplat = tmplat+X2C_DIVR((float)n,6.E+5f);
               }
               skip(gb, 1024ul, &i0, eol);
               if (gb[i0]=='S') tmplat = -tmplat;
               else if (gb[i0]!='N') break;
               skip(gb, 1024ul, &i0, eol);
               if (getnum(gb, 1024ul, &i0, eol, &n)) tmplong = (float)(n*100UL);
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) tmplong = tmplong+(float)(n*10UL);
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) tmplong = tmplong+(float)n;
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplong = tmplong+X2C_DIVR((float)n,6.0f);
               }
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplong = tmplong+X2C_DIVR((float)n,60.0f);
               }
               else break;
               if (gb[i0]=='.') ++i0;
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplong = tmplong+X2C_DIVR((float)n,600.0f);
               }
               else {
                  break;
               }
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplong = tmplong+X2C_DIVR((float)n,6000.0f);
               }
               else break;
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplong = tmplong+X2C_DIVR((float)n,60000.0f);
               }
               if (getnum(gb, 1024ul, &i0, eol, &n)) {
                  tmplong = tmplong+X2C_DIVR((float)n,6.E+5f);
               }
               skip(gb, 1024ul, &i0, eol);
               if (gb[i0]=='W') tmplong = -tmplong;
               else if (gb[i0]!='E') break;
               skip(gb, 1024ul, &i0, eol);
               skip(gb, 1024ul, &i0, eol);
               course = 0.0f;
               while (getnum(gb, 1024ul, &i0, eol, &n)) course = course*10.0f+(float)n;
               gpslong = tmplong;
               gpslat = tmplat;
               gpsdir = (uint32_t)X2C_TRUNCC(course,0UL,X2C_max_longcard);
            }
            break;
         }
         i0 = 0UL;
         while (eol+i0<gbp) {
            gb[i0] = gb[eol+i0];
            ++i0;
         }
         if (eol>gbp) gbp = 0UL;
         else gbp -= eol;
      }
   }
/* GPRMC,224104.000,A,7815.0879,N,02302.2740,E,1.10,314.23,030824,,,A*6A */
/*WrFixed(gpslat,6,1); WrStr(" "); WrFixed(gpslong,6,1);WrStrLn("");  */
} /* end rdgps() */


static void card(const char s[], uint32_t s_len, uint32_t * p, uint8_t * res, uint32_t len,
                char * ok0)
{
   uint32_t n;
   *ok0 = 0;
   n = 0UL;
   while (*p<len && s[*p]==' ') ++*p;
   while ((*p<len && (uint8_t)s[*p]>='0') && (uint8_t)s[*p]<='9') {
      n = (n*10UL+(uint32_t)(uint8_t)s[*p])-48UL;
      *ok0 = 1;
      ++*p;
   }
   if (n>1023UL) *ok0 = 0;
   if (*ok0) *res = (uint8_t)n;
} /* end card() */


static void readlut(const char fn[], uint32_t fn_len)
{
   uint32_t lc;
   uint32_t p;
   uint32_t last;
   uint32_t next;
   uint32_t gap;
   uint32_t i0;
   uint8_t idx;
   int32_t len;
   int32_t fd;
   char s[20001];
   char h[201];
   char ok0;
   float k;
   fd = osi_OpenRead(fn, fn_len);
   if (fd>=0L) {
      memset((char *)palette,(char)0,sizeof(struct PNGPALETTE [256]));
      len = osi_RdBin(fd, (char *)s, 20001u/1u, 20000UL);
      osic_Close(fd);
      if (len>=0L) s[len] = 0;
      lc = 1UL;
      p = 0UL;
      for (;;) {
         while ((int32_t)p<len && (uint8_t)s[p]<' ') ++p;
         if ((int32_t)p<len && s[p]!='#') {
            card(s, 20001ul, &p, &idx, (uint32_t)len, &ok0);
            if (idx>255U) {
               osi_Werr("colour index > 255\012", 20ul);
               break;
            }
            if (ok0) card(s, 20001ul, &p, &palette[idx].r, (uint32_t)len, &ok0);
            if (ok0) card(s, 20001ul, &p, &palette[idx].g, (uint32_t)len, &ok0);
            if (ok0) card(s, 20001ul, &p, &palette[idx].b, (uint32_t)len, &ok0);
            if (!ok0) {
               aprsstr_CardToStr(lc, 1UL, h, 201ul);
               aprsstr_Append(h, 201ul, ": colour file syntax error\012", 28ul);
               osi_Werr(h, 201ul);
               break;
            }
         }
         while ((int32_t)p<len && (uint8_t)s[p]>=' ') ++p;
         ++lc;
         if ((int32_t)p>=len) break;
      }
      p = 0UL; /* interpolate missing colours */
      do {
         i0 = p+1UL;
         while (((i0<=255UL && palette[i0].r==0U) && palette[i0].g==0U) && palette[i0].b==0U) ++i0;
         gap = (i0-p)-1UL;
         if (gap>0UL) {
            /* holes */
            if (i0<=255UL) next = i0;
            else {
               next = p; /* continue same colour till end */
               i0 = 255UL;
            }
            last = p;
            do {
               ++p;
               k = X2C_DIVR((float)(p-last),(float)(gap+1UL));
               palette[p].r = (uint8_t)(uint32_t)X2C_TRUNCC((float)palette[last].r+((float)
                palette[next].r-(float)palette[last].r)*k,0UL,X2C_max_longcard);
               palette[p].g = (uint8_t)(uint32_t)X2C_TRUNCC((float)palette[last].g+((float)
                palette[next].g-(float)palette[last].g)*k,0UL,X2C_max_longcard);
               palette[p].b = (uint8_t)(uint32_t)X2C_TRUNCC((float)palette[last].b+((float)
                palette[next].b-(float)palette[last].b)*k,0UL,X2C_max_longcard);
            } while (p<i0);
         }
         else ++p;
      } while (p<255UL);
   }
   else {
      /*FOR i:=0 TO 255 DO WrInt(palette[i].r,4);WrInt(palette[i].g,4);WrInt(palette[i].b,4); WrStrLn(""); END; */
      strncpy(s,"[",20001u);
      aprsstr_Append(s, 20001ul, fn, fn_len);
      aprsstr_Append(s, 20001ul, "] colour file not found\012", 25ul);
      osi_Werr(s, 20001ul);
   }
} /* end readlut() */


static void wrpng(uint32_t fromy, uint32_t xsize0, uint32_t ysize0, const char fn[], uint32_t fn_len)
{
   int32_t ret;
   uint32_t yh;
   uint32_t yy;
   uint32_t py;
   uint32_t y1;
   pROWPOINTERS pimage;
   pPNGPALETTE ppalette;
   uint32_t palettedepth;
   uint32_t palettelen;
   pRNS trns;
   pIMAGELINE anonym;
   uint32_t tmp;
   yh = 0UL;
   if (spimage) yh += (spimage->Len0-1)+1UL;
   if (headline) yh += (headline->Len0-1)+1UL;
   if (ysize0==0UL) while (ysize0<=image->Len0-1 && image->Adr[ysize0]) ++ysize0;
   if (ysize0==0UL) {
      osi_Werr("zero lines - no image write\012", 29ul);
      return;
   }
   palettelen = 256UL;
   ppalette = (pPNGPALETTE)palette;
   palettedepth = 8UL;
   trns = (pRNS)transpaency;
   osic_alloc((char * *) &pimage, sizeof(pLINE)*(ysize0+yh));
   if (pimage==0) {
      osi_Werr("png write out of memory\012", 25ul);
      return;
   }
   py = 0UL;
   if (spimage) {
      tmp = spimage->Len0-1;
      y1 = 0UL;
      if (y1<=tmp) for (;; y1++) {
         pimage[py] = (pLINE)spimage->Adr[(spimage->Len0-1)-y1]->Adr;
         ++py;
         if (y1==tmp) break;
      } /* end for */
   }
   if (headline) {
      tmp = headline->Len0-1;
      y1 = 0UL;
      if (y1<=tmp) for (;; y1++) {
         pimage[py] = (pLINE)headline->Adr[(headline->Len0-1)-y1]->Adr;
         ++py;
         if (y1==tmp) break;
      } /* end for */
   }
   tmp = ysize0-1UL;
   y1 = 0UL;
   if (y1<=tmp) for (;; y1++) {
      if (mirrory) yy = (ysize0-1UL)-y1;
      else yy = y1;
      yy = (((ysize0-1UL)+fromy)-yy)%ysize0;
      if (yy>image->Len0-1 || image->Adr[yy]==0) pimage[y1+yh] = blackline;
      else pimage[y1+yh] = (pLINE)(anonym = image->Adr[yy],&anonym->Adr[0UL]);
      if (y1==tmp) break;
   } /* end for */
   ret = writepng(fn, pimage, xsize0, ysize0+yh, ppalette, 256UL, 8UL, trns);
   osic_free((char * *) &pimage, sizeof(pLINE)*(ysize0+yh));
   if (ret<0L) osi_Werr("png write failed\012", 18ul);
} /* end wrpng() */


static void makelut(void)
{
   uint32_t i0;
   struct PNGPALETTE * anonym;
   for (i0 = 0UL; i0<=255UL; i0++) {
      { /* with */
         struct PNGPALETTE * anonym = &palette[i0];
         anonym->r = 0U;
         anonym->g = 0U;
         anonym->b = 0U;
         if (i0==0UL && transp) transpaency[i0] = 0U;
         else transpaency[i0] = 255U;
         if (i0<16UL) {
            anonym->r = (uint8_t)(i0*16UL);
            anonym->g = (uint8_t)(i0*16UL);
            anonym->b = (uint8_t)(i0*16UL);
         }
         else if (i0<64UL) anonym->b = (uint8_t)(i0*4UL);
         else if (i0<128UL) {
            anonym->b = (uint8_t)(255UL-(i0-64UL)*4UL);
            anonym->g = (uint8_t)((i0-64UL)*4UL);
            anonym->r = anonym->g;
         }
         else if (i0<192UL) {
            anonym->g = (uint8_t)(255UL-(i0-128UL)*4UL);
            anonym->r = 255U;
         }
         else if (i0<255UL) {
            anonym->r = 255U;
            anonym->g = (uint8_t)((i0-192UL)*4UL);
            anonym->b = anonym->g;
         }
         else {
            anonym->r = 255U;
            anonym->g = 255U;
            anonym->b = 255U;
         }
      }
   } /* end for */
} /* end makelut() */


static int32_t opengpio(uint32_t n, char out)
{
   char hp[100];
   char hh[100];
   char h[100];
   int32_t fd;
   uint32_t tr;
   aprsstr_CardToStr(n, 1UL, h, 100ul);
   strncpy(hp,"/sys/class/gpio/gpio",100u); /* /sys/class/gpio/gpio */
   aprsstr_Append(hp, 100ul, h, 100ul); /* /sys/class/gpio/gpio<n> */
   memcpy(hh,hp,100u);
   aprsstr_Append(hp, 100ul, "/value", 7ul); /* /sys/class/gpio/gpio<n>/value */
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
      osi_WrBin(fd, (char *)h, 100u/1u, aprsstr_Length(h, 100ul));
      osic_Close(fd);
      fd = osi_OpenWrite("/sys/class/gpio/export", 23ul); /* /sys/class/gpio/export */
      if (fd<0L) Error("cannot open gpio export", 24ul);
      osi_WrBin(fd, (char *)h, 100u/1u, aprsstr_Length(h, 100ul));
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
      if (out) strncpy(h,"out",100u);
      else strncpy(h,"in",100u);
      osi_WrBin(fd, (char *)h, 100u/1u, aprsstr_Length(h, 100ul)); /* in / out */
      osic_Close(fd);
      if (out) fd = osi_OpenRW(hp, 100ul);
      else fd = osi_OpenRead(hp, 100ul);
      if (fd<0L) Error("cannot open gpio value", 23ul);
      gpiofds[n] = fd;
   }
   return fd;
} /* end opengpio() */


static void testimage(void)
{
   uint32_t y1;
   uint32_t x0;
   pIMAGELINE anonym;
   size_t tmp[1];
   uint32_t tmp0;
   y1 = 0UL;
   X2C_DYNALLOCATE((char **) &image,sizeof(pIMAGELINE),(tmp[0] = ysize,tmp),1u);
   if (image==0) Error("out of memory", 14ul);
   do {
      X2C_DYNALLOCATE((char **) &image->Adr[y1],1u,(tmp[0] = xsize,tmp),1u);
      if (image->Adr[y1]==0) Error("out of memory", 14ul);
      tmp0 = xsize-1UL;
      x0 = 0UL;
      if (x0<=tmp0) for (;; x0++) {
         *(anonym = image->Adr[y1],&anonym->Adr[x0]) = (uint8_t)((x0*255UL)/(xsize-1UL));
         if (x0==tmp0) break;
      } /* end for */
      ++y1;
   } while (y1<ysize);
   wrpng(0UL, xsize, ysize, imagefn, 1024ul);
} /* end testimage() */


static char GetNum(const char h[], uint32_t h_len, char eot, uint32_t * p, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */

static GPIOSET _cnst = {0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL};

static void chkports(void)
{
   GPIOSET outs;
   GPIOSET ins;
   uint32_t i0;
   memcpy(ins,_cnst,128u);
   memcpy(outs,_cnst,128u);
   X2C_INCL(outs,chip.gpio.mosiN,1024);
   X2C_INCL(outs,chip.gpio.sckN,1024);
   if (X2C_INL(chip.gpio.misoN,1024,_cnst)) {
      osi_Werr("hint, shared inputs need nss-pullup if driver is started for not all connected chips (floating nss pin)\01\
2", 105ul);
   }
   X2C_INCL(ins,chip.gpio.misoN,1024);
   for (i0 = 0UL; i0<=1023UL; i0++) {
      if (X2C_INL(i0,1024,ins) && X2C_INL(i0,1024,outs)) Error("input<->output port conflict", 29ul);
   } /* end for */
   for (i0 = 0UL; i0<=1023UL; i0++) {
      if (X2C_INL(i0,1024,ins)) X2C_INCL(outs,i0,1024);
   } /* end for */
   if (X2C_INL(chip.gpio.ceN,1024,outs)) Error("ce must not be shared", 22ul);
   X2C_INCL(outs,chip.gpio.ceN,1024);
} /* end chkports() */


static void storechip(uint32_t * sck0, uint32_t * miso0, uint32_t * mosi0, uint32_t * ce0)
{
   struct GPIO * anonym;
   { /* with */
      struct GPIO * anonym = &chip.gpio;
      anonym->ceN = *ce0;
      anonym->mosiN = *mosi0;
      anonym->misoN = *miso0;
      anonym->sckN = *sck0;
      anonym->ceFD = opengpio(anonym->ceN, 1);
      anonym->mosiFD = opengpio(anonym->mosiN, 1);
      anonym->misoFD = opengpio(anonym->misoN, 0);
      anonym->sckFD = opengpio(anonym->sckN, 1);
   }
   if (chip.rxmhz0<400.0f) chip.band = 1UL;
   else if (chip.rxmhz0<500.0f) chip.band = 2UL;
   else chip.band = 3UL;
} /* end storechip() */


static void Parms(void)
{
   char err;
   char h[4096];
   uint32_t lptnum;
   uint32_t pcnt;
   uint32_t sck0;
   uint32_t mosi0;
   uint32_t miso0;
   uint32_t ce0;
   int32_t res;
   int32_t ii;
   for (pcnt = 0UL; pcnt<=1023UL; pcnt++) {
      gpiofds[pcnt] = -2L;
   } /* end for */
   lptnum = 0UL;
   err = 0;
   pcnt = 0UL;
   ce0 = 8UL;
   mosi0 = 10UL;
   miso0 = 9UL;
   sck0 = 11UL;
   memset((char *) &chip,(char)0,sizeof(struct CHIP));
   chip.lnaboost = 0;
   chip.ppm = 0.0f;
   chip.rssicorr = 0L;
   chip.rxmhz0 = 430.0f;
   chip.rxmhz1 = 440.0f;
   smooth = 3UL;
   xsize = 256UL;
   ysize = 256UL;
   mirrory = 0;
   spimage = 0;
   headline = 0;
   usbrobust = 1;
   for (;;) {
      osi_NextArg(h, 4096ul);
      if (h[0U]==0) break;
      if (h[0U]=='-' && h[1U]) {
         if (h[1U]=='p') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToCard(h, 4096ul, &ce0) || ce0>=1024UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToCard(h, 4096ul, &mosi0) || mosi0>=1024UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToCard(h, 4096ul, &miso0) || miso0>=1024UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToCard(h, 4096ul, &sck0) || sck0>=1024UL) {
               Error("-p <ce0> <mosi0> <miso0> <sck0>", 32ul);
            }
         }
         else if (h[1U]=='l') chip.lnaboost = 1;
         else if (h[1U]=='x') {
            osi_NextArg(h, 4096ul);
            if ((!aprsstr_StrToCard(h, 4096ul, &xsize) || xsize<32UL) || xsize>=32000UL) {
               Error("-x <size>", 10ul);
            }
         }
         else if (h[1U]=='y') {
            osi_NextArg(h, 4096ul);
            if ((!aprsstr_StrToCard(h, 4096ul, &ysize) || ysize<1UL) || ysize>=32000UL) {
               Error("-y <size>", 10ul);
            }
         }
         else if (h[1U]=='w') {
            osi_NextArg(imagefn, 1024ul);
            if (imagefn[0U]==0 || imagefn[0U]=='-') Error("-i <imagefilename>", 19ul);
         }
         else if (h[1U]=='j') {
            osi_NextArg(h, 4096ul);
            jfd = osi_OpenRead(h, 4096ul);
            if (jfd<0L) jfd = osi_OpenWrite(h, 4096ul);
            else {
               osic_Close(jfd);
               jfd = osi_OpenAppend(h, 4096ul);
            }
            if (jfd<0L) Error("-j <jsonfilename>", 18ul);
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h[1U]=='m') mirrory = 1;
         else if (h[1U]=='t') transp = 1;
         else if (h[1U]=='T') timestamp = 1;
         else if (h[1U]=='R') withhead = 1;
         else if (h[1U]=='r') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToCard(h, 4096ul, &roll)) Error("-r <lines>", 11ul);
         }
         else if (h[1U]=='s') {
            osi_NextArg(h, 4096ul);
            if ((!aprsstr_StrToCard(h, 4096ul, &specy) || specy<32UL) || specy>=256UL) {
               Error("-s <hight> (32..256)", 21ul);
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToCard(h, 4096ul, &linetime)) {
               Error("-d <line-every-seconds> else as fast as can (0)", 48ul);
            }
         }
         else if (h[1U]=='P') {
            osi_NextArg(h, 4096ul);
            if ((!aprsstr_StrToFix(&chip.ppm, h, 4096ul) || chip.ppm<(-133.0f)) || chip.ppm>133.0f) {
               Error("-P <ppm>", 9ul);
            }
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 4096ul);
            for (;;) {
               res = aprsstr_InStr(h, 4096ul, "/", 2ul);
               if (res<0L) break;
               h[res] = 0;
               if ((!aprsstr_StrToFix(&ftab[ftablen].f, h,
                4096ul) || ftab[ftablen].f<137.0f) || ftab[ftablen].f>1020.0f) {
                  Error("-f <MHz>/<bw>,<MHz>/<bw>,... (137..1020)", 41ul);
               }
               for (ii = res+1L; ii<=4095L; ii++) {
                  h[(ii-res)-1L] = h[ii];
               } /* end for */
               res = aprsstr_InStr(h, 4096ul, ",", 2ul);
               if (res>=0L) h[res] = 0;
               if (!aprsstr_StrToCard(h, 4096ul, &ftab[ftablen].bw)) {
                  Error("-f <MHz>/<bw>,<MHz>/<bw>,... (137..1020)", 41ul);
               }
               ++ftablen;
               if (ftablen>4095UL) Error("-f frequency table full", 24ul);
               if (res<0L) break;
               for (ii = res+1L; ii<=4095L; ii++) {
                  h[(ii-res)-1L] = h[ii];
               } /* end for */
            }
            if (ftablen==0UL) {
               /* from-to */
               if ((!aprsstr_StrToFix(&chip.rxmhz0, h, 4096ul) || chip.rxmhz0<137.0f) || chip.rxmhz0>1020.0f) {
                  Error("-f <MHz> <MHz> (137..1020)", 27ul);
               }
               osi_NextArg(h, 4096ul);
               if (((!aprsstr_StrToFix(&chip.rxmhz1, h,
                4096ul) || chip.rxmhz1<137.0f) || chip.rxmhz1>1020.0f) || chip.rxmhz0>=chip.rxmhz1) {
                  Error("-f <MHz> <MHz> (137..1020)", 27ul);
               }
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 4096ul);
            readlut(h, 4096ul);
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 4096ul);
            i = 0UL;
            while ((h[i] && h[i]!=':') && i<1023UL) {
               ttyfname[i] = h[i];
               ++i;
            }
            ttyfname[i] = 0;
            if (h[i]) {
               ++i;
               if (!GetNum(h, 4096ul, 0, &i, &baud)) Error("need ttydevice:baud", 20ul);
            }
         }
         else if (h[1U]=='h') {
            osi_WrStrLn(" ra-02 (sx127x) via LPT or GPIO specrum and waterfall image by oe5dxl", 70ul);
            osi_WrStrLn("", 1ul);
            osi_WrStrLn(" -c <file>          read colour lines \"<index> <r> <g> <b>\" (0..255) gaps interpolated",
                87ul);
            osi_WrStrLn("                      color 0 to 15 for spectrum, 255 for text", 63ul);
            osi_WrStrLn("                      for colours testimage start with -r 100000", 65ul);
            osi_WrStrLn(" -d <seconds>       delay between lines with peak hold, 0 as fast as can (0)", 77ul);
            osi_WrStrLn(" -f <MHz> <MHz>     frequency range MHz (430.0..440.0) (137..1020)", 67ul);
            osi_WrStrLn(" -f <MHz>/<bw>,<MHz>/<bw>...   scan frequency table MHz/bandwidth(Hz) (137..1020)", 82ul);
            osi_WrStrLn(" -g <tty>:<baud>    read serial GPS and add lat,long,course to json levels (/dev/ttyUSB0:4800)",
                 95ul);
            osi_WrStrLn(" -h                 this", 25ul);
            osi_WrStrLn(" -j <filename>      write (append) level table + gps data to file", 66ul);
            osi_WrStrLn(" -l                 lna boost on, better ip3 by more supply current", 68ul);
            osi_WrStrLn(" -m                 vertical mirror waterfall (downward)", 57ul);
            osi_WrStrLn(" -P <ppm>           x-tal correction +-128 (0.0)", 49ul);
            osi_WrStrLn(" -p <nss> <mosi> <miso> <sck>  GPIO numbers", 44ul);
            osi_WrStrLn(" -R                 add scaler with Mhz", 40ul);
            osi_WrStrLn(" -r <lines>         write new image every lines (else write 1 image when it is full)", 85ul);
            osi_WrStrLn(" -s <lines>         draw spectrum on top of waterfall", 54ul);
            osi_WrStrLn(" -T                 timestamps on waterfall", 44ul);
            osi_WrStrLn(" -t                 transparent image on not yet filled part", 61ul);
            osi_WrStrLn(" -V                 show more infos on stdout", 46ul);
            osi_WrStrLn(" -v                 show some infos on stdout", 46ul);
            osi_WrStrLn(" -w <imagefilename> path + filename of png image", 49ul);
            osi_WrStrLn(" -x <pixel>         x-size of picture (256)", 44ul);
            osi_WrStrLn(" -y <pixel>         y-size of picture (256) + spectrum higth", 61ul);
            osi_WrStrLn("", 1ul);
            osi_WrStrLn("./ra02waterfall -p 0 1 2 3 -f 430 440 -w /tmp/w.png -r 2 -x 1000 -y 500 -R -T -s 120", 85ul);
            osi_WrStrLn("", 1ul);
            X2C_ABORT();
         }
         else {
            h[2U] = 0;
            aprsstr_Append(h, 4096ul, "? use -h", 9ul);
            Error(h, 4096ul);
         }
         h[0U] = 0;
      }
      else {
         h[1U] = 0;
         aprsstr_Append(h, 4096ul, "? use -h", 9ul);
         Error(h, 4096ul);
      }
   }
   if (roll==100000UL) {
      testimage();
      X2C_ABORT();
   }
   storechip(&sck0, &miso0, &mosi0, &ce0);
   chkports();
} /* end Parms() */


static void delay(void)
{
/*  usleep(20); */
} /* end delay() */


static char scp(const struct GPIO gpio, char rd, char nss, char sck, char mosi)
{
   char h[2];
   char res;
   int32_t r;
   if (rd) {
      osic_Seek(gpio.misoFD, 0UL);
      r = osi_RdBin(gpio.misoFD, (char *)h, 2u/1u, 1UL);
      res = h[0U]!='0';
   }
   else res = 0;
   h[0U] = (char)(48UL+(uint32_t)sck);
   osi_WrBin(gpio.sckFD, (char *)h, 2u/1u, 1UL);
   if (nss!=X2C_INL(gpio.ceN,1024,gpostate)) {
      if (nss) {
         h[0U] = '1';
         X2C_INCL(gpostate,gpio.ceN,1024);
      }
      else {
         h[0U] = '0';
         X2C_EXCL(gpostate,gpio.ceN,1024);
      }
      osi_WrBin(gpio.ceFD, (char *)h, 2u/1u, 1UL);
   }
   if (mosi!=X2C_INL(gpio.mosiN,1024,gpostate)) {
      if (mosi) {
         h[0U] = '1';
         X2C_INCL(gpostate,gpio.mosiN,1024);
      }
      else {
         h[0U] = '0';
         X2C_EXCL(gpostate,gpio.mosiN,1024);
      }
      osi_WrBin(gpio.mosiFD, (char *)h, 2u/1u, 1UL);
   }
   return res;
} /* end scp() */


static void scpio(const struct GPIO gpio, char wr, uint32_t a, uint32_t len, char s[], uint32_t s_len)
/* send or receive a buffer */
{
   uint32_t d;
   uint32_t rd;
   uint32_t j;
   uint32_t i0;
   char b;
   j = 0UL;
   i0 = 16UL;
   while (j<len) {
      d = ((uint32_t)(uint8_t)s[j]+(a&127UL)*256UL+32768UL*(uint32_t)wr)*2UL;
      rd = 0UL;
      for (;;) {
         rd += rd+(uint32_t)scp(gpio, !wr && i0<=7UL, i0==0UL && j==len-1UL, 0, X2C_IN(i0,32,(uint32_t)d));
         delay();
         if (i0==0UL) break;
         b = scp(gpio, 0, 0, 1, X2C_IN(i0,32,(uint32_t)d));
         delay();
         --i0;
      }
      if (!wr) s[j] = (char)rd;
      ++j;
      i0 = 8UL;
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


static void Showregs(struct CHIP chp)
{
   uint32_t i0;
   for (i0 = 0UL; i0<=100UL; i0++) {
      WrHex(i0, 2UL, 0UL);
      osi_WrStr(":", 2ul);
      WrHex(scpi(chp.gpio, i0), 2UL, 0UL);
      osi_WrStr(" ", 2ul);
      if ((i0&15UL)==15UL) osi_WrStrLn("", 1ul);
   } /* end for */
   osi_WrStrLn("", 1ul);
} /* end Showregs() */


static void setsynth(struct CHIP chp, float mhz)
{
   uint32_t synt;
   mhz = mhz+mhz*chp.ppm*1.E-6f;
   synt = (uint32_t)X2C_TRUNCC(mhz*16384.0f+0.5f,0UL,X2C_max_longcard);
   scpo(chp.gpio, 6UL, synt/65536UL); /* freq high */
   scpo(chp.gpio, 7UL, synt/256UL&255UL); /* freq mid */
   scpo(chp.gpio, 8UL, synt&255UL); /* freq low */
} /* end setsynth() */


static void Setmode(struct CHIP chp, uint32_t m, char check, char tx)
{
   uint32_t n;
   m += 8UL*(uint32_t)(chp.rxmhz0<=779.0f);
   n = 0UL;
   for (;;) {
      scpo(chp.gpio, 1UL, m);
      if (!check || scpi(chp.gpio, 1UL)==m) break;
      usleep(1000UL);
      if (scpi(chp.gpio, 1UL)/128UL!=m/128UL) {
         if (n>5UL) {
            scpo(chp.gpio, 1UL, 0UL);
            scpo(chp.gpio, 54UL, 64UL); /* stop sequencer */
            scpo(chp.gpio, 54UL, 0UL); /* 83H why ever */
            scpo(chp.gpio, 1UL, 0UL);
            scpo(chp.gpio, 1UL, 1UL);
            scpo(chp.gpio, 1UL, 4UL);
            scpo(chp.gpio, 1UL, 0UL);
            scpo(chp.gpio, 1UL, (m/128UL)*128UL);
         }
         ++n;
         if ((n&255UL)==0UL) {
            osi_WrStr(" try set mode:", 15ul);
            WrHex(m, 2UL, 0UL);
            osi_WrStr(" chip has:", 11ul);
            WrHex(scpi(chp.gpio, 1UL), 2UL, 0UL);
            osi_WrStrLn("", 1ul);
         }
      }
   }
} /* end Setmode() */

static void freegpio(int32_t);


static void freegpio(int32_t signum)
{
   uint32_t i0;
   char h[2];
   int32_t fd;
   if (jfd>=0L) osic_Close(jfd);
   for (i0 = 0UL; i0<=1023UL; i0++) {
      if (gpiofds[i0]!=-2L) {
         aprsstr_CardToStr(i0, 1UL, h, 2ul);
         fd = osi_OpenWrite("/sys/class/gpio/unexport", 25ul); /* /sys/class/gpio/unexport */
         if (fd>=0L) {
            osi_WrBin(fd, (char *)h, 2u/1u, 1UL);
            osic_Close(fd);
         }
      }
   } /* end for */
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   X2C_HALT((uint32_t)signum);
} /* end freegpio() */


static uint32_t rxbw(float hz)
{
   uint32_t j;
   uint32_t i0;
   for (j = 7UL; j>=1UL; j--) {
      for (i0 = 2UL;; i0--) {
         if (X2C_DIVL(3.2E+7,pow(2.0,
                (double)(float)(j+2UL))*(double)(float)(i0*4UL+16UL))>(double)hz) {
            return j+i0*8UL;
         }
         if (i0==0UL) break;
      } /* end for */
   } /* end for */
   return 1UL;
} /* end rxbw() */


static void clrline(uint32_t y1)
{
   uint32_t x0;
   pIMAGELINE anonym;
   uint32_t tmp;
   if (image->Adr[y1]) {
      tmp = xsize-1UL;
      x0 = 0UL;
      if (x0<=tmp) for (;; x0++) {
         *(anonym = image->Adr[y1],&anonym->Adr[x0]) = 0U;
         if (x0==tmp) break;
      } /* end for */
   }
} /* end clrline() */

static uint8_t _cnst0[66] = {0x7CU,0x82U,0x82U,0x82U,0x7CU,0U,0U,0x42U,0xFEU,0x2U,0U,0U,0x46U,0x8AU,0x92U,0x92U,0x66U,
                0U,0x44U,0x92U,0x92U,0x92U,0x6CU,0U,0x18U,0x28U,0x4AU,0xFEU,0xAU,0U,0xE4U,0xA2U,0xA2U,0xA2U,0x9CU,0U,
                0x3CU,0x52U,0x92U,0x92U,0xCU,0U,0U,0x80U,0x8EU,0x90U,0xE0U,0U,0x6CU,0x92U,0x92U,0x92U,0x6CU,0U,0x60U,
                0x92U,0x92U,0x94U,0x78U,0U,0U,0x6U,0x6U,0U,0U,0U};

static void prnt(char c, uint32_t * x0, uint32_t y1, const pIMAGELINE image0[], uint32_t image_len)
{
   uint32_t yj;
   uint32_t n;
   uint32_t j;
   uint32_t i0;
   for (j = 0UL; j<=7UL; j++) {
      for (i0 = 0UL; i0<=5UL; i0++) {
         n = (uint32_t)(uint8_t)c;
         if (n>=48UL && n<=57UL) n -= 48UL;
         else if (n==46UL) n = 10UL;
         else n = 256UL;
         yj = y1-7UL;
         if (mirrory) yj -= j;
         else yj += j;
         if ((((n<=10UL && i0+*x0<=image0[0UL]->Len0-1) && yj<=image_len-1) && image0[yj]) && X2C_IN(j,8,
                _cnst0[n*6UL+i0])) image0[yj]->Adr[i0+*x0] = 255U;
      } /* end for */
   } /* end for */
   *x0 += 6UL;
} /* end prnt() */


static void inc(uint8_t * n, float r)
{
   uint32_t a;
   if (r<=0.0f) r = 0.0f;
   a = (uint32_t)*n+(uint32_t)X2C_TRUNCC(r*15.0f,0UL,X2C_max_longcard);
   if (a>15UL) a = 15UL;
   else if (a==0UL) a = 1UL;
   *n = (uint8_t)a;
} /* end inc() */


static void spectrum(const uint8_t line0[], uint32_t line_len)
{
   uint32_t d;
   uint32_t b;
   uint32_t a;
   uint32_t j;
   uint32_t i0;
   float ir;
   float r;
   pIMAGELINE anonym;
   pIMAGELINE anonym0;
   pIMAGELINE anonym1;
   pIMAGELINE anonym2;
   pIMAGELINE anonym3;
   size_t tmp[1];
   uint32_t tmp0;
   uint32_t tmp1;
   if (spimage==0) {
      X2C_DYNALLOCATE((char **) &spimage,sizeof(pIMAGELINE),(tmp[0] = specy,tmp),1u);
      if (spimage==0) Error("out of memory", 14ul);
      tmp0 = spimage->Len0-1;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         X2C_DYNALLOCATE((char **) &spimage->Adr[i0],1u,(tmp[0] = xsize,tmp),1u);
         if (spimage->Adr[i0]==0) Error("out of memory", 14ul);
         if (i0==tmp0) break;
      } /* end for */
   }
   tmp0 = spimage->Len0-1;
   i0 = 0UL;
   if (i0<=tmp0) for (;; i0++) {
      memset((char *)spimage->Adr[i0]->Adr,(char)0,spimage->Adr[0UL]->Len0*1u);
      if (i0==tmp0) break;
   } /* end for */
   a = (uint32_t)line0[0UL];
   if (a>spimage->Len0-1) a = spimage->Len0-1;
   tmp0 = xsize-1UL;
   j = 1UL;
   if (j<=tmp0) for (;; j++) {
      b = (uint32_t)line0[j];
      if (b>spimage->Len0-1) b = spimage->Len0-1;
      d = (uint32_t)labs((int32_t)b-(int32_t)a);
      if (d==0UL) *(anonym = spimage->Adr[a],&anonym->Adr[j-1UL]) = 15U;
      else {
         r = X2C_DIVR(1.0f,(float)d);
         ir = 0.0f;
         do {
            inc((anonym0 = spimage->Adr[a],&anonym0->Adr[j-1UL]), 1.0f-ir);
            inc((anonym1 = spimage->Adr[a],&anonym1->Adr[j]), ir);
            ir = ir+r;
            if (a>b) --a;
            else ++a;
         } while (a!=b);
      }
      if (j==tmp0) break;
   } /* end for */
   tmp0 = spimage->Len0-1;
   j = 0UL;
   if (j<=tmp0) for (tmp0 = (uint32_t)(tmp0-j)/20UL;;) {
      tmp1 = xsize-1UL;
      i0 = 0UL;
      if (i0<=tmp1) for (;; i0++) {
         if ((anonym2 = spimage->Adr[j],anonym2->Adr[i0])==0U) {
            *(anonym3 = spimage->Adr[j],&anonym3->Adr[i0]) = 64U;
         }
         if (i0==tmp1) break;
      } /* end for */
      if (!tmp0) break;
      --tmp0;
      j += 20UL;
   } /* end for */
   i0 = 1UL;
   prnt('1', &i0, 28UL, spimage->Adr, spimage->Len0);
   prnt('0', &i0, 28UL, spimage->Adr, spimage->Len0);
} /* end spectrum() */


static void ruler(float a, float b)
{
   uint32_t f;
   uint32_t e;
   uint32_t d;
   uint32_t m;
   uint32_t n;
   uint32_t x0;
   uint32_t p;
   uint32_t i0;
   float aa;
   float r;
   float dx;
   char s[21];
   pIMAGELINE anonym;
   pIMAGELINE anonym0;
   pIMAGELINE anonym1;
   pIMAGELINE anonym2;
   pIMAGELINE anonym3;
   pIMAGELINE anonym4;
   pIMAGELINE anonym5;
   pIMAGELINE anonym6;
   pIMAGELINE anonym7;
   pIMAGELINE anonym8;
   size_t tmp[1];
   uint32_t tmp0;
   X2C_DYNALLOCATE((char **) &headline,sizeof(pIMAGELINE),(tmp[0] = 10U,tmp),1u);
   if (headline==0) Error("out of memory", 14ul);
   tmp0 = headline->Len0-1;
   i0 = 0UL;
   if (i0<=tmp0) for (;; i0++) {
      X2C_DYNALLOCATE((char **) &headline->Adr[i0],1u,(tmp[0] = xsize,tmp),1u);
      if (headline->Adr[i0]==0) Error("out of memory", 14ul);
      memset((char *)headline->Adr[i0]->Adr,(char)0,headline->Adr[0UL]->Len0*1u);
      if (i0==tmp0) break;
   } /* end for */
   dx = X2C_DIVR(b-a,(float)xsize);
   r = dx*190.0f; /* mhz text spacing */
   d = 1UL;
   while (r>=10.0f) {
      r = r*0.1f;
      d = d*10UL;
   }
   e = 1UL;
   f = 10UL;
   if (r>=5.0f) {
      e = 5UL;
      f = 4UL;
   }
   else if (r>=2.0f) e = 2UL;
   d = d*e;
   aa = X2C_DIVR(a,(float)d);
   aa = (aa-(float)(uint32_t)X2C_TRUNCC(aa,0UL,X2C_max_longcard))*(float)d;
   r = X2C_DIVR(aa*20.0f-0.00001f,(float)d);
   if (r<0.0f) r = r-1.0f;
   n = (uint32_t)(int32_t)X2C_TRUNCI(r,X2C_min_longint,X2C_max_longint);
   tmp0 = xsize-1UL;
   i0 = 0UL;
   if (i0<=tmp0) for (;; i0++) {
      m = (uint32_t)(int32_t)X2C_TRUNCI(X2C_DIVR(((float)(i0+1UL)*dx+aa)*20.0f,(float)d),X2C_min_longint,
                X2C_max_longint);
      *(anonym = headline->Adr[0UL],&anonym->Adr[i0]) = 255U;
      if (n!=m) {
         if (e!=5UL) {
            *(anonym0 = headline->Adr[1UL],&anonym0->Adr[i0]) = 255U;
            if (e==2UL) *(anonym1 = headline->Adr[2UL],&anonym1->Adr[i0]) = 255U;
         }
         if (n/2UL!=m/2UL) {
            *(anonym2 = headline->Adr[1UL],&anonym2->Adr[i0]) = 255U;
            *(anonym3 = headline->Adr[2UL],&anonym3->Adr[i0]) = 255U;
         }
         if (n/f!=m/f) *(anonym4 = headline->Adr[3UL],&anonym4->Adr[i0]) = 255U;
         if (n/20UL!=m/20UL) {
            *(anonym5 = headline->Adr[1UL],&anonym5->Adr[i0]) = 255U;
            *(anonym6 = headline->Adr[2UL],&anonym6->Adr[i0]) = 255U;
            *(anonym7 = headline->Adr[3UL],&anonym7->Adr[i0]) = 255U;
            *(anonym8 = headline->Adr[4UL],&anonym8->Adr[i0]) = 255U;
            aprsstr_IntToStr((int32_t)(uint32_t)X2C_TRUNCC(0.5f+a+X2C_DIVR((b-a)*(float)i0,(float)xsize),0UL,
                X2C_max_longcard), 1UL, s, 21ul);
            p = 0UL;
            x0 = i0+1UL;
            while (p<=20UL && s[p]) {
               prnt(s[p], &x0, headline->Len0-1, headline->Adr, headline->Len0);
               ++p;
            }
         }
      }
      n = m;
      if (i0==tmp0) break;
   } /* end for */
} /* end ruler() */


static void wrj(const uint8_t line0[], uint32_t line_len, uint32_t len, uint32_t tablen)
{
   uint32_t i0;
   char pos;
   char s[20001];
   char h[31];
   strncpy(s,"{",20001u);
   pos = 0;
   if (gpslat!=0.0f || gpslong!=0.0f) {
      aprsstr_Append(s, 20001ul, "\"lat\":", 7ul);
      aprsstr_FixToStr(gpslat, 6UL, h, 31ul);
      aprsstr_Append(s, 20001ul, h, 31ul);
      aprsstr_Append(s, 20001ul, ",\"long\":", 9ul);
      aprsstr_FixToStr(gpslong, 6UL, h, 31ul);
      aprsstr_Append(s, 20001ul, h, 31ul);
      aprsstr_Append(s, 20001ul, ",\"dir\":", 8ul);
      aprsstr_CardToStr(gpsdir, 1UL, h, 31ul);
      aprsstr_Append(s, 20001ul, h, 31ul);
      pos = 1;
   }
   gpslat = 0.0f;
   gpslong = 0.0f;
   if (len>0UL) {
      if (pos) aprsstr_Append(s, 20001ul, ",", 2ul);
      if (tablen>0UL) {
         aprsstr_Append(s, 20001ul, "\"mhzlev\":[", 11ul);
         i0 = 0UL;
         for (;;) {
            aprsstr_Append(s, 20001ul, "{", 2ul);
            aprsstr_FixToStr(ftab[i0].f+0.0005f, 4UL, h, 31ul);
            aprsstr_Append(s, 20001ul, h, 31ul);
            aprsstr_Append(s, 20001ul, ":", 2ul);
            aprsstr_CardToStr((uint32_t)line0[i0], 1UL, h, 31ul);
            aprsstr_Append(s, 20001ul, h, 31ul);
            aprsstr_Append(s, 20001ul, "}", 2ul);
            ++i0;
            if (i0>=tablen) break;
            aprsstr_Append(s, 20001ul, ",", 2ul);
         }
         aprsstr_Append(s, 20001ul, "]", 2ul);
      }
      else {
         aprsstr_Append(s, 20001ul, "\"frommhz\":", 11ul);
         aprsstr_FixToStr(chip.rxmhz0, 4UL, h, 31ul);
         aprsstr_Append(s, 20001ul, h, 31ul);
         aprsstr_Append(s, 20001ul, "\",tomhz\":", 10ul);
         aprsstr_FixToStr(chip.rxmhz1, 4UL, h, 31ul);
         aprsstr_Append(s, 20001ul, h, 31ul);
         aprsstr_Append(s, 20001ul, "\",lev\":[", 9ul);
         i0 = 0UL;
         for (;;) {
            aprsstr_CardToStr((uint32_t)line0[i0], 1UL, h, 31ul);
            aprsstr_Append(s, 20001ul, h, 31ul);
            ++i0;
            if (i0>=len) break;
            aprsstr_Append(s, 20001ul, ",", 2ul);
         }
         aprsstr_Append(s, 20001ul, "]", 2ul);
      }
   }
   aprsstr_Append(s, 20001ul, "}\012", 3ul);
   osi_WrBin(jfd, (char *)s, 20001u/1u, aprsstr_Length(s, 20001ul));
} /* end wrj() */


static void imageline(uint32_t y1, uint8_t line0[], uint32_t line_len)
{
   uint32_t i0;
   int32_t n;
   char clr;
   pIMAGELINE anonym;
   pIMAGELINE anonym0;
   size_t tmp[1];
   uint32_t tmp0;
   clr = 0;
   if (imagefn[0]) {
      if (image->Adr[y1]==0) {
         X2C_DYNALLOCATE((char **) &image->Adr[y1],1u,(tmp[0] = xsize,tmp),1u);
         if (image->Adr[y1]==0) Error("out of memory", 14ul);
         tmp0 = xsize-1UL;
         i0 = 0UL;
         if (i0<=tmp0) for (;; i0++) {
            *(anonym = image->Adr[y1],&anonym->Adr[i0]) = 0U;
            if (i0==tmp0) break;
         } /* end for */
      }
      tmp0 = xsize-1UL;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         n = (int32_t)((uint32_t)line0[i0]*2UL+16UL);
         if (n>254L) n = 254L;
         *(anonym0 = image->Adr[y1],&anonym0->Adr[i0]) = (uint8_t)n;
         if (i0==tmp0) break;
      } /* end for */
      if (specy>32UL) spectrum(line0, line_len);
      clr = 1;
   }
   if (jfd>=0L) {
      if (ttyfname[0]) rdgps();
      if (gpslat!=0.0f || gpslong!=0.0f) {
         wrj(line0, line_len, xsize, ftablen);
         clr = 1;
      }
   }
   if (clr) memset((char *)line0,(char)0,line_len);
} /* end imageline() */


static void prtime(uint32_t y1, uint32_t t)
{
   uint32_t x0;
   uint32_t i0;
   char s[31];
   pIMAGELINE anonym;
   if (image->Adr[y1]) {
      for (i0 = 0UL; i0<=32UL; i0++) {
         *(anonym = image->Adr[y1],&anonym->Adr[i0]) = 255U;
      } /* end for */
      if (y1>=7UL) {
         aprsstr_TimeToStr(t%86400UL, s, 31ul);
         s[5U] = 0;
         s[2U] = '.';
         i0 = 0UL;
         x0 = 4UL;
         while (i0<=30UL && s[i0]) {
            prnt(s[i0], &x0, y1-2UL, image->Adr, image->Len0);
            ++i0;
         }
      }
   }
} /* end prtime() */


static void peakhold(uint8_t * p, uint8_t v)
{
   if (v>*p) *p = v;
} /* end peakhold() */

static float kx;

static float bandw;

static uint32_t mescnt;

static uint32_t tslinc;

static uint32_t rxbandw;

static uint32_t rollcnt;

static uint32_t x;

static uint32_t y;

static uint32_t rssitime;

static uint32_t time0;

static uint32_t tm;

static uint32_t ts;

static uint32_t timei;


struct _1 {
   uint8_t * Adr;
   size_t Len0;
};

static struct _1 * line;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   uint32_t tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(GPIOSET)!=128) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGTERM, freegpio);
   signal(SIGINT, freegpio);
   signal(SIGPIPE, freegpio);
   jfd = -1L;
   makelut();
   Parms();
   if (ttyfname[0]) opentty();
   if (ftablen>0UL) xsize = ftablen;
   if (imagefn[0]) {
      if (roll && ysize==0UL) Error("need -y <size>", 15ul);
      y = ysize;
      if (y==0UL) y = 32000UL;
      X2C_DYNALLOCATE((char **) &image,sizeof(pIMAGELINE),(tmp[0] = y,tmp),1u);
      if (image==0) Error("out of memory", 14ul);
      tmp0 = image->Len0-1;
      y = 0UL;
      if (y<=tmp0) for (;; y++) {
         image->Adr[y] = 0;
         if (y==tmp0) break;
      } /* end for */
      osic_alloc((char * *) &blackline, xsize);
      if (blackline==0) Error("out of memory", 14ul);
      tmp0 = xsize-1UL;
      y = 0UL;
      if (y<=tmp0) for (;; y++) {
         blackline[y] = 0U;
         if (y==tmp0) break;
      } /* end for */
      memcpy(tmpfn,imagefn,1024u);
      aprsstr_Append(tmpfn, 1024ul, "~", 2ul);
   }
   X2C_DYNALLOCATE((char **) &line,1u,(tmp[0] = xsize,tmp),1u);
   if (line==0) Error("out of memory", 14ul);
   scpo(chip.gpio, 54UL, 64UL); /* stop sequencer */
   scpo(chip.gpio, 54UL, 0UL); /* 83H why ever */
   if (verb2) {
      Setmode(chip, 0UL, 1, 0);
      Showregs(chip);
      Setmode(chip, 128UL, 1, 0);
      Showregs(chip);
   }
   Setmode(chip, 0UL, 1, 0);
   scpo(chip.gpio, 54UL, 64UL); /* stop sequencer */
   scpo(chip.gpio, 54UL, 0UL); /* 83H why ever */
   Setmode(chip, 1UL, 1, 0);
   /*  Setmode(chip, 4, TRUE, FALSE);                                       (* clear fifo *) */
   /*  Setmode(chip, 0, TRUE, FALSE); */
   /*  Setmode(chip, 80H, TRUE, FALSE); */
   /*  usleep(1000); */
   /*  Setmode(chip, 80H, TRUE, FALSE); */
   /*  usleep(1000); */
   /*  Setmode(chip, 85H, TRUE, FALSE); */
   /*  Setmode(chip, 80H, TRUE, FALSE); */
   setsynth(chip, chip.rxmhz0);
   scpo(chip.gpio, 37UL, 0UL);
   scpo(chip.gpio, 38UL, 0UL);
   scpo(chip.gpio, 39UL, 0UL);
   scpo(chip.gpio, 48UL, 0UL);
   scpo(chip.gpio, 49UL, 64UL); /*40H*/
   scpo(chip.gpio, 50UL, 0UL);
   scpo(chip.gpio, 53UL, 10UL);
   scpo(chip.gpio, 54UL, 184UL); /*B8*/ /* start sequencer tx on fifo thres */
   for (i = 0UL; i<=150UL; i++) {
      scpo(chip.gpio, 0UL, 0UL);
   } /* end for */
   usleep(100000UL);
   scpo(chip.gpio, 54UL, 64UL); /* stop sequencer */
   Setmode(chip, 1UL, 1, 0);
   scpo(chip.gpio, 4UL, 0UL); /* why ever */
   if (verb) {
      osi_WrStr("T=", 3ul);
      osic_WrINT32((uint32_t)(uint8_t) -(signed char)scpi(chip.gpio, 60UL), 1UL);
      osi_WrStrLn("C", 2ul);
   }
   setsynth(chip, chip.rxmhz0);
   scpo(chip.gpio, 1UL, 4UL); /* FSR */
   usleep(500UL);
   scpo(chip.gpio, 1UL, 1UL);
   scpo(chip.gpio, 59UL, 64UL); /* start calibration */
   if (verb) osi_WrStr("Calibration:", 13ul);
   while ((0x20U & (uint8_t)scpi(chip.gpio, 59UL))) {
      usleep(1000UL);
      if (verb) osi_WrStr(".", 2ul);
   }
   if (verb) {
      osi_WrStrLn("done", 5ul);
   }
   scpo(chip.gpio, 13UL, 8UL); /* afc, agc=on, freq changed */
   scpo(chip.gpio, 26UL, 2UL);
   bandw = X2C_DIVR(1.E+6f*(chip.rxmhz1-chip.rxmhz0),(float)xsize);
   if (bandw<2000.0f) bandw = 2000.0f;
   else if (bandw>2.5E+5f) bandw = 2.5E+5f;
   rxbandw = rxbw(bandw);
   rssitime = (uint32_t)X2C_TRUNCC(X2C_DIVL(pow(2.0, (double)(float)(smooth+1UL)),
                (double)(bandw*4.E-6f)),0UL,X2C_max_longcard);
   scpo(chip.gpio, 31UL, 0UL);
   scpo(chip.gpio, 39UL, 16UL); /* 10H */
   scpo(chip.gpio, 40UL, 255UL);
   scpo(chip.gpio, 48UL, 0UL);
   scpo(chip.gpio, 49UL, 64UL);
   scpo(chip.gpio, 50UL, 0UL);
   scpo(chip.gpio, 20UL, 32UL);
   scpo(chip.gpio, 54UL, 64UL); /* stop sequencer */
   scpo(chip.gpio, 54UL, 0UL); /* 83H why ever */
   Setmode(chip, 4UL, 1, 0);
   Setmode(chip, 5UL, 1, 0);
   kx = X2C_DIVR(chip.rxmhz1-chip.rxmhz0,(float)xsize);
   y = 0UL;
   if (verb) {
      osi_WrStr("bw=", 4ul);
      osic_WrFixed(bandw*0.001f, 2L, 1UL);
      osi_WrStrLn("kHz", 4ul);
      osi_WrStr("smoothtime=", 12ul);
      osic_WrINT32(rssitime, 1UL);
      osi_WrStrLn("ms", 3ul);
   }
   if (imagefn[0]) ruler(chip.rxmhz0, chip.rxmhz1);
   mescnt = X2C_max_longcard;
   for (;;) {
      if (mescnt>1000UL) {
         /* chip destroys some registers so reload it */
         mescnt = 0UL;
         scpo(chip.gpio, 26UL, 18UL);
         scpo(chip.gpio, 12UL, 32UL+(uint32_t)chip.lnaboost*27UL); /* modifies self... lna gain, current */
         scpo(chip.gpio, 14UL, smooth+8UL*((uint32_t)chip.rssicorr&31UL)); /* modifies self... */
         scpo(chip.gpio, 68UL, 128UL);
         scpo(chip.gpio, 2UL, 26UL);
         scpo(chip.gpio, 3UL, 11UL);
         scpo(chip.gpio, 18UL, rxbandw); /* rx bw 32000000/(m*2^x)) */
         scpo(chip.gpio, 19UL, rxbandw); /* afc width 32000000/(m*2^x)) */
      }
      tmp0 = xsize-1UL;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         if (ftablen>0UL) {
            rxbandw = rxbw((float)ftab[x].bw);
            rssitime = (uint32_t)X2C_TRUNCC(X2C_DIVL(pow(2.0, (double)(float)(smooth+1UL)),
                (double)((float)ftab[x].bw*4.E-6f)),0UL,X2C_max_longcard);
            scpo(chip.gpio, 18UL, rxbandw); /* rx bw 32000000/(m*2^x)) */
            scpo(chip.gpio, 19UL, rxbandw); /* afc width 32000000/(m*2^x)) */
            setsynth(chip, ftab[x].f);
         }
         else setsynth(chip, chip.rxmhz0+kx*(float)x);
         scpo(chip.gpio, 13UL, 40UL); /* trigger rx and agc on */
         usleep(rssitime);
         peakhold(&line->Adr[x], (uint8_t)(255UL-scpi(chip.gpio, 17UL)));
         ++mescnt;
         if (x==tmp0) break;
      } /* end for */
      ts = osic_time();
      if (ts>=timei+linetime) {
         /* draw/store a new line */
         imageline(y, line->Adr, line->Len0);
         timei = ts;
         if (imagefn[0]) {
            if (timestamp) {
               tm = ts/60UL;
               if (tm!=time0) {
                  time0 = tm;
                  if (tslinc>=40UL) {
                     prtime(y, tm*60UL);
                     tslinc = 0UL;
                  }
               }
            }
            ++tslinc;
            if (roll) {
               y = (y+1UL)%ysize;
               ++rollcnt;
               if (rollcnt>=roll) {
                  wrpng(y, xsize, ysize, tmpfn, 1024ul);
                  osi_Rename(tmpfn, 1024ul, imagefn, 1024ul);
                  rollcnt = 0UL;
               }
               /*Showregs(chip); */
               clrline(y);
            }
            else {
               ++y;
               if (y>image->Len0-1 || ysize && y>=ysize) break;
            }
         }
      }
      else if (ts<timei) timei = ts;
   }
   if (imagefn[0]) {
      if (roll) {
         if (rollcnt>0UL) {
            /* flush image in roll mode*/
            wrpng(y, xsize, ysize, tmpfn, 1024ul);
            osi_Rename(tmpfn, 1024ul, imagefn, 1024ul);
         }
      }
      else wrpng(0UL, xsize, ysize, imagefn, 1024ul);
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
