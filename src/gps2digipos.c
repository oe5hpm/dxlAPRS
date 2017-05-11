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
#define gps2digipos_C_
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

/* read serial gps and make "3704.04N/00805.14W" string to file */
#define gps2digipos_CR "\015"

#define gps2digipos_LF "\012"

#define gps2digipos_FEET 3.2808398950131

#define gps2digipos_KNOTS 1.851984

#define gps2digipos_PI 3.1415926535898

static char tbuf[1024];

static char ttynamee[1024];

static char basefilename[1024];
/*    symt, symb:CHAR; */

static uint32_t baud;

static int32_t tty;

static char verb;

static char usbrobust;

static char junk;

static char sumoff;

static char withalti;

static uint32_t comptyp;

static uint32_t comintval;

static uint32_t comcnt;

static uint32_t mediantime;

static uint32_t medians;

static struct termios saved;

static double speed;

static double course;

static double lat;

static double long0;

static double alt;

static char altok;

static char posok;

struct _0;


struct _0 {
   double mlat;
   double mlong;
   double malt;
   char ok0;
};

static struct _0 median[500];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr("agps2digipos: ", 15ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static uint32_t truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static char SetStatLine(int32_t fd, char rts,
                char dtr)
{
   uint32_t arg[256];
   int32_t r;
   arg[0U] = 4UL;
   if (rts) r = ioctl(fd, 21526L, (char *)arg);
   else r = ioctl(fd, 21527L, (char *)arg);
   if (r>=0L) {
      arg[0U] = 2UL;
      if (dtr) r = ioctl(fd, 21526L, (char *)arg);
      else r = ioctl(fd, 21527L, (char *)arg);
   }
   return r>=0L;
} /* end SetStatLine() */


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
   else Error("unknown baudrate", 17ul);
   res = tcgetattr(fd, &saved);
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


static void opentty(void)
{
   for (;;) {
      tty = osi_OpenRW(ttynamee, 1024ul);
      if (tty>=0L) {
         SetComMode(tty, baud);
         break;
      }
      if (!usbrobust) Error("tty open", 9ul);
      usleep(1000000UL);
   }
   SetStatLine(tty, 0, 0);
} /* end opentty() */


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
   uint32_t i;
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='u') usbrobust = 0;
         else if (h[1U]=='s') sumoff = 1;
         else if (h[1U]=='a') withalti = 1;
         else if (h[1U]=='f') {
            /*
                  ELSIF h[1]="i" THEN 
                    NextArg(h);
                    IF h[0]>" " THEN
                      symt:=h[0];
                      IF h[1]>" " THEN symb:=h[1] ELSE symb:=0C END;
                    ELSE Error("-i <icon> (house /-)") END;
            */
            osi_NextArg(basefilename, 1024ul);
            if (basefilename[0U]==0) Error("-f filename", 12ul);
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            while ((h[i] && h[i]!=':') && i<1023UL) {
               ttynamee[i] = h[i];
               ++i;
            }
            ttynamee[i] = 0;
            if (h[i]) {
               ++i;
               if (!GetNum(h, 1024ul, 0, &i, &baud)) {
                  Error("need ttydevice:baud", 20ul);
               }
            }
         }
         else if (h[1U]=='m') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &mediantime)) {
               Error("-m <seconds>", 13ul);
            }
            if (mediantime>499UL) {
               aprsstr_IntToStr(499L, 1UL, h, 1024ul);
               aprsstr_Append(h, 1024ul, " maximum -m", 12ul);
               Error(h, 1024ul);
            }
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &comptyp) || comptyp>2UL) {
               Error("-f <format> 0=uncomp, 1=comp, 2=mic-e", 38ul);
            }
         }
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn("Read serial GPS and make position string to inser\
t into APRS-beacon", 68ul);
               osi_WrStrLn(" -a                                altitude on",
                47ul);
               osi_WrStrLn(" -f <filename>                     writes <fn.lat\
> <fn.long> and <filename.alt>", 80ul);
               osi_WrStrLn(" -h                                this", 40ul);
               /*        WrStrLn('
                -i <icon>                         2 Icon chars "/-" (House),
                "/>" (Car)...'); */
               osi_WrStrLn(" -m <seconds>                      time to read g\
ps to make median position", 76ul);
               osi_WrStrLn(" -s                                GPS Checksum c\
heck OFF", 58ul);
               osi_WrStrLn(" -t <tty>:<baud>                   default /dev/t\
tyS0:9600", 59ul);
               osi_WrStrLn(" -u                                not retry unti\
l open removable USB tty", 74ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osi_WrStrLn(" example:  -t /dev/ttyS0:9600 -u -f test -i \"/-\\
" -a -m 30 -v", 61ul);
               osic_WrLn();
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
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */


static void skip(const char b[], uint32_t b_len, uint32_t * p,
                uint32_t len0)
{
   while (*p<len0 && b[*p]!=',') ++*p;
   if (*p<len0) ++*p;
} /* end skip() */


static char getnum(const char b[], uint32_t b_len,
                uint32_t * p, uint32_t len0, uint32_t * n)
{
   if ((*p<len0 && (uint8_t)b[*p]>='0') && (uint8_t)b[*p]<='9') {
      *n = (uint32_t)(uint8_t)b[*p]-48UL;
      ++*p;
      return 1;
   }
   *n = 0UL;
   return 0;
} /* end getnum() */


static void decodeline(const char b[], uint32_t b_len,
                uint32_t len0)
{
   uint32_t n;
   uint32_t i;
   double div0;
   char sign;
   if ((b[0UL]=='$' && b[1UL]=='G') && b[2UL]=='P') {
      if ((b[3UL]=='R' && b[4UL]=='M') && b[5UL]=='C') {
         /* $GPRMC,141333.593,A,8915.1000,N,01300.2000,E,0.00,00.00,140410,0,
                ,A*7C */
         /* $GPRMC,112430.00,A,8912.41130,N,01300.61995,E,0.039,,200513,,,
                A*77 */
         i = 7UL;
         skip(b, b_len, &i, len0);
         if (b[i]!='A') return;
         skip(b, b_len, &i, len0);
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = (double)(float)(n*10UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(float)n;
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,6.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,60.0f));
         }
         else return;
         if (b[i]=='.') ++i;
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,600.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,6000.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,60000.0f));
         }
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,6.E+5f));
         }
         skip(b, b_len, &i, len0);
         if (b[i]=='S') lat = -lat;
         else if (b[i]!='N') return;
         skip(b, b_len, &i, len0);
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = (double)(float)(n*100UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(float)(n*10UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(float)n;
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,6.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,60.0f));
         }
         else return;
         if (b[i]=='.') ++i;
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,600.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,6000.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,60000.0f));
         }
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,6.E+5f));
         }
         skip(b, b_len, &i, len0);
         if (b[i]=='W') long0 = -long0;
         else if (b[i]!='E') return;
         skip(b, b_len, &i, len0);
         speed = 0.0;
         while (getnum(b, b_len, &i, len0, &n)) {
            speed = speed*10.0+(double)(float)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               speed = speed+(double)(float)n*div0;
               div0 = div0*0.1;
            }
         }
         speed = speed*1.851984; /* knots to km/h */
         skip(b, b_len, &i, len0);
         course = 0.0;
         while (getnum(b, b_len, &i, len0, &n)) {
            course = course*10.0+(double)(float)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               course = course+(double)(float)n*div0;
               div0 = div0*0.1;
            }
         }
         posok = 1;
      }
      else if ((b[3UL]=='G' && b[4UL]=='G') && b[5UL]=='A') {
         /* $GPGGA,152554,3938.5665,N,10346.2039,W,1,08,1.7,12382.7,M,-22.3,
                M,,*7B */
         /* $GPGGA,112435.00,4812.41112,N,01305.61998,E,1,08,1.04,398.3,M,
                44.9,M,,*59 */
         i = 7UL;
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         alt = 0.0;
         if (b[i]=='-') {
            sign = 1;
            ++i;
         }
         else sign = 0;
         while (getnum(b, b_len, &i, len0, &n)) {
            altok = 1;
            alt = alt*10.0+(double)(float)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               alt = alt+(double)(float)n*div0;
               div0 = div0*0.1;
            }
         }
         if (sign) alt = -alt;
      }
   }
} /* end decodeline() */


static char Hex(uint32_t d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


static char checksum(const char b[], uint32_t b_len,
                uint32_t len0)
{
   uint32_t i;
   uint8_t cs;
   char ok0;
   ok0 = 1;
   i = 1UL;
   cs = 0U;
   while (i<len0 && b[i]!='*') {
      cs = cs^(uint8_t)(uint8_t)b[i];
      ++i;
   }
   if (i+2UL>=len0) ok0 = 0;
   if (ok0) {
      if (b[i+1UL]!=Hex((uint32_t)cs/16UL) || b[i+2UL]!=Hex((uint32_t)
                cs&15UL)) ok0 = 0;
   }
   if (verb && !ok0) osi_WrStrLn("GPS Checksum Error", 19ul);
   return ok0;
} /* end checksum() */


static void showline(const char b[], uint32_t b_len, uint32_t len0)
{
   uint32_t i;
   i = 0UL;
   while (i<len0) {
      if ((uint8_t)b[i]<' ') {
         if (b[i]=='\012') osic_WrLn();
         else osi_WrStr(".", 2ul);
      }
      else osi_WrStr((char *) &b[i], 1u/1u);
      ++i;
   }
   osic_WrLn();
} /* end showline() */


static void wrfile(char b[], uint32_t b_len, uint32_t len0,
                char ext[], uint32_t ext_len)
{
   char s[2001];
   int32_t f;
   X2C_PCOPY((void **)&b,b_len);
   X2C_PCOPY((void **)&ext,ext_len);
   aprsstr_Assign(s, 2001ul, basefilename, 1024ul);
   aprsstr_Append(s, 2001ul, ext, ext_len);
   f = osi_OpenWrite(s, 2001ul);
   if (f>=0L) {
      if (len0>0UL) osi_WrBin(f, (char *)b, (b_len)/1u, len0);
      osic_Close(f);
   }
   X2C_PFREE(b);
   X2C_PFREE(ext);
} /* end wrfile() */


static char num(uint32_t n)
{
   return (char)(n%10UL+48UL);
} /* end num() */


static void wrpos(double lat0, double long1, double alt0,
                char withalt)
{
   char b[201];
   uint32_t n;
   uint32_t i;
   double a;
   /* "4805.44N" "01333.64E" "/A=000000"*/
   i = 0UL;
   a = fabs(lat0);
   n = truncc(a);
   b[0U] = num(n/10UL);
   ++i;
   b[1U] = num(n);
   ++i;
   n = truncc((a-(double)(float)n)*6000.0);
   b[2U] = num(n/1000UL);
   ++i;
   b[3U] = num(n/100UL);
   ++i;
   b[4U] = '.';
   ++i;
   b[5U] = num(n/10UL);
   ++i;
   b[6U] = num(n);
   ++i;
   if (lat0>=0.0) b[7U] = 'N';
   else b[7U] = 'S';
   ++i;
   /*  b[i]:=symt; INC(i); */
   wrfile(b, 201ul, 8UL, ".lat", 5ul);
   i = 0UL;
   a = fabs(long1);
   n = truncc(a);
   b[0U] = num(n/100UL);
   ++i;
   b[1U] = num(n/10UL);
   ++i;
   b[2U] = num(n);
   ++i;
   n = truncc((a-(double)(float)n)*6000.0);
   b[3U] = num(n/1000UL);
   ++i;
   b[4U] = num(n/100UL);
   ++i;
   b[5U] = '.';
   ++i;
   b[6U] = num(n/10UL);
   ++i;
   b[7U] = num(n);
   ++i;
   if (lat0>=0.0) b[8U] = 'E';
   else b[8U] = 'W';
   ++i;
   /*  IF symb>" " THEN b[i]:=symb; INC(i) END; */
   wrfile(b, 201ul, 9UL, ".long", 6ul);
   if (withalt) {
      i = 0UL;
      if (alt0>0.0) {
         b[0U] = '/';
         ++i;
         b[1U] = 'A';
         ++i;
         b[2U] = '=';
         ++i;
         n = truncc(fabs(alt0*3.2808398950131+0.5));
         if (alt0>=0.0) b[3U] = num(n/100000UL);
         else b[3U] = '-';
         ++i;
         b[4U] = num(n/10000UL);
         ++i;
         b[5U] = num(n/1000UL);
         ++i;
         b[6U] = num(n/100UL);
         ++i;
         b[7U] = num(n/10UL);
         ++i;
         b[8U] = num(n);
         ++i;
      }
      wrfile(b, 201ul, i, ".alt", 5ul);
   }
} /* end wrpos() */


static void medianpos(double lat0, double long1,
                double alt0)
{
   struct _0 * anonym;
   if (medians<=499UL) {
      { /* with */
         struct _0 * anonym = &median[medians];
         anonym->mlat = lat0;
         anonym->mlong = long1;
         anonym->malt = alt0;
      }
      ++medians;
   }
} /* end medianpos() */


static void getmedian(double * lat0, double * long1,
                double * alt0)
{
   uint32_t by;
   uint32_t ay;
   uint32_t bx;
   uint32_t ax;
   uint32_t j;
   uint32_t i;
   double maxy;
   double miny;
   double maxx;
   double minx;
   struct _0 * anonym;
   struct _0 * anonym0;
   struct _0 * anonym1;
   uint32_t tmp;
   if (medians==0UL) Error("no positions got", 17ul);
   j = medians/10UL;
   tmp = medians-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      median[i].ok0 = 1;
      if (i==tmp) break;
   } /* end for */
   for (;;) {
      /* kill extrem values */
      minx = (double)X2C_max_real;
      maxx = (double)X2C_min_real;
      ax = 0UL;
      bx = 0UL;
      miny = (double)X2C_max_real;
      maxy = (double)X2C_min_real;
      ay = 0UL;
      by = 0UL;
      j = 0UL;
      tmp = medians-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         if (median[i].ok0) {
            { /* with */
               struct _0 * anonym = &median[i];
               if (anonym->mlat<minx) {
                  minx = anonym->mlat;
                  ax = i;
               }
               if (anonym->mlat>maxx) {
                  maxx = anonym->mlat;
                  bx = i;
               }
               if (anonym->mlong<miny) {
                  miny = anonym->mlong;
                  ay = i;
               }
               if (anonym->mlong>maxy) {
                  maxy = anonym->mlong;
                  by = i;
               }
            }
            ++j;
         }
         if (i==tmp) break;
      } /* end for */
      if (j<=medians/2UL+5UL) break;
      median[ax].ok0 = 0;
      median[bx].ok0 = 0;
      median[ay].ok0 = 0;
      median[by].ok0 = 0;
   }
   j = 0UL;
   *lat0 = 0.0;
   *long1 = 0.0;
   *alt0 = 0.0;
   if (verb) {
      osi_WrStrLn("   lat           long                alt  ok", 45ul);
   }
   tmp = medians-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /* make mean valus */
      if (median[i].ok0) {
         { /* with */
            struct _0 * anonym0 = &median[i];
            *lat0 = *lat0+anonym0->mlat;
            *long1 = *long1+anonym0->mlong;
            *alt0 = *alt0+anonym0->malt;
            ++j;
         }
      }
      if (verb) {
         { /* with */
            struct _0 * anonym1 = &median[i];
            osic_WrFixed((float)anonym1->mlat, 8L, 14UL);
            osic_WrFixed((float)anonym1->mlong, 8L, 14UL);
            osic_WrFixed((float)anonym1->malt, 1L, 14UL);
            osic_WrINT32((uint32_t)anonym1->ok0, 2UL);
            osi_WrStrLn("", 1ul);
         }
      }
      if (i==tmp) break;
   } /* end for */
   if (j==0UL) Error("no positions got", 17ul);
   *lat0 = X2C_DIVL(*lat0,(double)j);
   *long1 = X2C_DIVL(*long1,(double)j);
   *alt0 = X2C_DIVL(*alt0,(double)j);
} /* end getmedian() */

static char c;

static int32_t len;

static int32_t rp;

static uint32_t gpsp;

static char gpsb[100];


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   int32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   sumoff = 0;
   junk = 1;
   posok = 0;
   altok = 0;
   verb = 0;
   usbrobust = 1;
   medians = 0UL;
   mediantime = 30UL;
   baud = 9600UL;
   strncpy(ttynamee,"/dev/ttyS0",1024u);
   strncpy(basefilename,"gpspos",1024u);
   gpsp = 0UL;
   /*  symt:="/"; */
   /*  symb:="U"; */
   comptyp = 0UL;
   withalti = 0;
   comintval = 5UL;
   comcnt = 0UL;
   Parms();
   opentty();
   for (;;) {
      len = osi_RdBin(tty, (char *)tbuf, 1024u/1u, 1024UL);
      if (len<=0L) {
         osic_Close(tty);
         usleep(1000000UL);
         opentty();
         junk = 1;
      }
      if (!junk) {
         tmp = len-1L;
         rp = 0L;
         if (rp<=tmp) for (;; rp++) {
            c = tbuf[rp];
            if (c=='\015' || c=='\012') {
               if (gpsp>0UL) {
                  if (sumoff || checksum(gpsb, 100ul, gpsp)) {
                     decodeline(gpsb, 100ul, gpsp);
                  }
                  else {
                     altok = 0;
                     posok = 0;
                  }
                  if (verb) showline(gpsb, 100ul, gpsp);
                  if (posok && (altok || !withalti)) {
                     medianpos(lat, long0, alt);
                     if (mediantime>0UL) {
                        --mediantime;
                        if (verb) {
                           osic_WrINT32(mediantime, 4UL);
                           osi_WrStrLn(" mediantime", 12ul);
                        }
                     }
                     else {
                        getmedian(&lat, &long0, &alt);
                        wrpos(lat, long0, alt, withalti && altok);
                        goto loop_exit;
                     }
                     altok = 0;
                     posok = 0;
                  }
               }
               gpsp = 0UL;
            }
            else if ((uint8_t)c>' ' && gpsp<99UL) {
               gpsb[gpsp] = c;
               ++gpsp;
            }
            if (rp==tmp) break;
         } /* end for */
      }
      junk = 0;
   }
   loop_exit:;
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
