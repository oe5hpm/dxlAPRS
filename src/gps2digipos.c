/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* "@(#)gps2digipos.c Mar 18  4:03:15 2014" */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define gps2digipos_C_
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#include <mlib.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

#define gps2digipos_CR "\015"

#define gps2digipos_LF "\012"

#define gps2digipos_FEET 3.2808398950131

#define gps2digipos_KNOTS 1.851984

#define gps2digipos_PI 3.1415926535898

static X2C_CHAR tbuf[1024];

static X2C_CHAR ttynamee[1024];

static X2C_CHAR basefilename[1024];

static X2C_CHAR symt;

static X2C_CHAR symb;

static X2C_CARD32 baud;

static X2C_INT32 tty;

static X2C_BOOLEAN verb;

static X2C_BOOLEAN usbrobust;

static X2C_BOOLEAN junk;

static X2C_BOOLEAN sumoff;

static X2C_BOOLEAN withalti;

static X2C_CARD32 comptyp;

static X2C_CARD32 comintval;

static X2C_CARD32 comcnt;

static X2C_CARD32 mediantime;

static X2C_CARD32 medians;

static struct termios saved;

static X2C_LONGREAL speed;

static X2C_LONGREAL course;

static X2C_LONGREAL lat;

static X2C_LONGREAL long0;

static X2C_LONGREAL alt;

static X2C_BOOLEAN altok;

static X2C_BOOLEAN posok;

struct _0;


struct _0 {
   X2C_LONGREAL mlat;
   X2C_LONGREAL mlong;
   X2C_LONGREAL malt;
   X2C_BOOLEAN ok0;
};

static struct _0 median[500];


static void Error(X2C_CHAR text[], X2C_CARD32 text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   InOut_WriteString("agps2digipos: ", 15ul);
   InOut_WriteString(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static X2C_CARD32 truncc(X2C_LONGREAL r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (X2C_CARD32)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static void SetComMode(X2C_INT32 fd, X2C_CARD32 baud0)
{
   struct termios term;
   X2C_INT32 res;
   X2C_CARD32 bd;
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
      anonym->c_cflag = 2224UL+bd;
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
} /* end opentty() */


static X2C_BOOLEAN GetNum(const X2C_CHAR h[], X2C_CARD32 h_len, X2C_CHAR eot,
                 X2C_CARD32 * p, X2C_CARD32 * n)
{
   *n = 0UL;
   while ((X2C_CARD8)h[*p]>='0' && (X2C_CARD8)h[*p]<='9') {
      *n = ( *n*10UL+(X2C_CARD32)(X2C_CARD8)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static void Parms(void)
{
   X2C_BOOLEAN err;
   X2C_CHAR h[1024];
   X2C_CARD32 i;
   err = 0;
   for (;;) {
      Lib_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='u') usbrobust = 0;
         else if (h[1U]=='s') sumoff = 1;
         else if (h[1U]=='a') withalti = 1;
         else if (h[1U]=='i') {
            Lib_NextArg(h, 1024ul);
            if ((X2C_CARD8)h[0U]>' ') {
               symt = h[0U];
               if ((X2C_CARD8)h[1U]>' ') symb = h[1U];
               else symb = 0;
            }
            else Error("-i <icon> (house /-)", 21ul);
         }
         else if (h[1U]=='f') {
            Lib_NextArg(basefilename, 1024ul);
            if (basefilename[0U]==0) Error("-f filename", 12ul);
         }
         else if (h[1U]=='t') {
            Lib_NextArg(h, 1024ul);
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
            Lib_NextArg(h, 1024ul);
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
            Lib_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &comptyp) || comptyp>2UL) {
               Error("-f <format> 0=uncomp, 1=comp, 2=mic-e", 38ul);
            }
         }
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osi_WrLn();
               osi_WrStrLn("Read serial GPS and make position string to inser\
t into APRS-beacon", 68ul);
               osi_WrStrLn(" -a                                altitude on",
                47ul);
               osi_WrStrLn(" -f                                writes <filena\
me.pos> and <filename.alt>", 76ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -i <icon>                         2 Icon chars \\
"/-\" (House), \"/>\" (Car)...", 76ul);
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
               osi_WrLn();
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      InOut_WriteString(">", 2ul);
      InOut_WriteString(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */


static void skip(const X2C_CHAR b[], X2C_CARD32 b_len, X2C_CARD32 * p,
                X2C_CARD32 len0)
{
   while (*p<len0 && b[*p]!=',') ++*p;
   if (*p<len0) ++*p;
} /* end skip() */


static X2C_BOOLEAN getnum(const X2C_CHAR b[], X2C_CARD32 b_len,
                X2C_CARD32 * p, X2C_CARD32 len0, X2C_CARD32 * n)
{
   if ((*p<len0 && (X2C_CARD8)b[*p]>='0') && (X2C_CARD8)b[*p]<='9') {
      *n = (X2C_CARD32)(X2C_CARD8)b[*p]-48UL;
      ++*p;
      return 1;
   }
   *n = 0UL;
   return 0;
} /* end getnum() */


static void decodeline(const X2C_CHAR b[], X2C_CARD32 b_len,
                X2C_CARD32 len0)
{
   X2C_CARD32 n;
   X2C_CARD32 i;
   X2C_LONGREAL div0;
   X2C_BOOLEAN sign;
   if ((b[0UL]=='$' && b[1UL]=='G') && b[2UL]=='P') {
      if ((b[3UL]=='R' && b[4UL]=='M') && b[5UL]=='C') {
         i = 7UL;
         skip(b, b_len, &i, len0);
         if (b[i]!='A') return;
         skip(b, b_len, &i, len0);
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = (X2C_LONGREAL)(X2C_REAL)(n*10UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_REAL)n;
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,6.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,60.0f));
         }
         else return;
         if (b[i]=='.') ++i;
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,600.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,6000.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,60000.0f));
         }
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,6.E+5f));
         }
         skip(b, b_len, &i, len0);
         if (b[i]=='S') lat = -lat;
         else if (b[i]!='N') return;
         skip(b, b_len, &i, len0);
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = (X2C_LONGREAL)(X2C_REAL)(n*100UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_REAL)(n*10UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_REAL)n;
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,6.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,60.0f));
         }
         else return;
         if (b[i]=='.') ++i;
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,600.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,6000.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,60000.0f));
         }
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(X2C_LONGREAL)(X2C_DIVR((X2C_REAL)n,6.E+5f));
         }
         skip(b, b_len, &i, len0);
         if (b[i]=='W') long0 = -long0;
         else if (b[i]!='E') return;
         skip(b, b_len, &i, len0);
         speed = 0.0;
         while (getnum(b, b_len, &i, len0, &n)) {
            speed = speed*10.0+(X2C_LONGREAL)(X2C_REAL)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               speed = speed+(X2C_LONGREAL)(X2C_REAL)n*div0;
               div0 = div0*0.1;
            }
         }
         speed = speed*1.851984;
         skip(b, b_len, &i, len0);
         course = 0.0;
         while (getnum(b, b_len, &i, len0, &n)) {
            course = course*10.0+(X2C_LONGREAL)(X2C_REAL)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               course = course+(X2C_LONGREAL)(X2C_REAL)n*div0;
               div0 = div0*0.1;
            }
         }
         posok = 1;
      }
      else if ((b[3UL]=='G' && b[4UL]=='G') && b[5UL]=='A') {
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
            alt = alt*10.0+(X2C_LONGREAL)(X2C_REAL)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               alt = alt+(X2C_LONGREAL)(X2C_REAL)n*div0;
               div0 = div0*0.1;
            }
         }
         if (sign) alt = -alt;
      }
   }
} /* end decodeline() */


static X2C_CHAR Hex(X2C_CARD32 d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (X2C_CHAR)(d+48UL);
} /* end Hex() */


static X2C_BOOLEAN checksum(const X2C_CHAR b[], X2C_CARD32 b_len,
                X2C_CARD32 len0)
{
   X2C_CARD32 i;
   X2C_CARD8 cs;
   X2C_BOOLEAN ok0;
   ok0 = 1;
   i = 1UL;
   cs = 0U;
   while (i<len0 && b[i]!='*') {
      cs = cs^(X2C_CARD8)(X2C_CARD8)b[i];
      ++i;
   }
   if (i+2UL>=len0) ok0 = 0;
   if (ok0) {
      if (b[i+1UL]!=Hex((X2C_CARD32)cs/16UL) || b[i+2UL]!=Hex((X2C_CARD32)
                cs&15UL)) ok0 = 0;
   }
   if (verb && !ok0) osi_WrStrLn("GPS Checksum Error", 19ul);
   return ok0;
} /* end checksum() */


static void showline(const X2C_CHAR b[], X2C_CARD32 b_len, X2C_CARD32 len0)
{
   X2C_CARD32 i;
   i = 0UL;
   while (i<len0) {
      if ((X2C_CARD8)b[i]<' ') {
         if (b[i]=='\012') osi_WrLn();
         else InOut_WriteString(".", 2ul);
      }
      else InOut_WriteString((X2C_CHAR *) &b[i], 1u/1u);
      ++i;
   }
   osi_WrLn();
} /* end showline() */


static void wrfile(X2C_CHAR b[], X2C_CARD32 b_len, X2C_CARD32 len0,
                X2C_CHAR ext[], X2C_CARD32 ext_len)
{
   X2C_CHAR s[2001];
   X2C_INT32 f;
   X2C_PCOPY((void **)&b,b_len);
   X2C_PCOPY((void **)&ext,ext_len);
   aprsstr_Assign(s, 2001ul, basefilename, 1024ul);
   aprsstr_Append(s, 2001ul, ext, ext_len);
   f = osi_OpenWrite(s, 2001ul);
   if (f>=0L) {
      if (len0>0UL) osi_WrBin(f, (X2C_LOC *)b, (b_len)/1u, len0);
      osi_Close(f);
   }
   X2C_PFREE(b);
   X2C_PFREE(ext);
} /* end wrfile() */


static X2C_CHAR num(X2C_CARD32 n)
{
   return (X2C_CHAR)(n%10UL+48UL);
} /* end num() */


static void wrpos(X2C_LONGREAL lat0, X2C_LONGREAL long1, X2C_LONGREAL alt0,
                X2C_BOOLEAN withalt)
{
   X2C_CHAR b[201];
   X2C_CARD32 n;
   X2C_CARD32 i;
   X2C_LONGREAL a;
   i = 0UL;
   a = fabs(lat0);
   n = truncc(a);
   b[0U] = num(n/10UL);
   ++i;
   b[1U] = num(n);
   ++i;
   n = truncc((a-(X2C_LONGREAL)(X2C_REAL)n)*6000.0);
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
   b[8U] = symt;
   ++i;
   a = fabs(long1);
   n = truncc(a);
   b[9U] = num(n/100UL);
   ++i;
   b[10U] = num(n/10UL);
   ++i;
   b[11U] = num(n);
   ++i;
   n = truncc((a-(X2C_LONGREAL)(X2C_REAL)n)*6000.0);
   b[12U] = num(n/1000UL);
   ++i;
   b[13U] = num(n/100UL);
   ++i;
   b[14U] = '.';
   ++i;
   b[15U] = num(n/10UL);
   ++i;
   b[16U] = num(n);
   ++i;
   if (lat0>=0.0) b[17U] = 'E';
   else b[17U] = 'W';
   ++i;
   if ((X2C_CARD8)symb>' ') {
      b[18U] = symb;
      ++i;
   }
   wrfile(b, 201ul, i, ".pos", 5ul);
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


static void medianpos(X2C_LONGREAL lat0, X2C_LONGREAL long1,
                X2C_LONGREAL alt0)
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


static void getmedian(X2C_LONGREAL * lat0, X2C_LONGREAL * long1,
                X2C_LONGREAL * alt0)
{
   X2C_CARD32 by;
   X2C_CARD32 ay;
   X2C_CARD32 bx;
   X2C_CARD32 ax;
   X2C_CARD32 j;
   X2C_CARD32 i;
   X2C_LONGREAL maxy;
   X2C_LONGREAL miny;
   X2C_LONGREAL maxx;
   X2C_LONGREAL minx;
   struct _0 * anonym;
   struct _0 * anonym0;
   struct _0 * anonym1;
   X2C_CARD32 tmp;
   if (medians==0UL) Error("no positions got", 17ul);
   j = medians/10UL;
   tmp = medians-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      median[i].ok0 = 1;
      if (i==tmp) break;
   } /* end for */
   for (;;) {
      minx = (X2C_LONGREAL)X2C_max_real;
      maxx = (X2C_LONGREAL)X2C_min_real;
      ax = 0UL;
      bx = 0UL;
      miny = (X2C_LONGREAL)X2C_max_real;
      maxy = (X2C_LONGREAL)X2C_min_real;
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
            osi_WrFixed((X2C_REAL)anonym1->mlat, 8L, 14UL);
            osi_WrFixed((X2C_REAL)anonym1->mlong, 8L, 14UL);
            osi_WrFixed((X2C_REAL)anonym1->malt, 1L, 14UL);
            InOut_WriteInt((X2C_INT32)(X2C_CARD32)anonym1->ok0, 2UL);
            osi_WrStrLn("", 1ul);
         }
      }
      if (i==tmp) break;
   } /* end for */
   if (j==0UL) Error("no positions got", 17ul);
   *lat0 = X2C_DIVL(*lat0,(X2C_LONGREAL)j);
   *long1 = X2C_DIVL(*long1,(X2C_LONGREAL)j);
   *alt0 = X2C_DIVL(*alt0,(X2C_LONGREAL)j);
} /* end getmedian() */

static X2C_CHAR c;

static X2C_INT32 len;

static X2C_INT32 rp;

static X2C_CARD32 gpsp;

static X2C_CHAR gpsb[100];


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_INT32 tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   Lib_BEGIN();
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
   symt = '/';
   symb = 'U';
   comptyp = 0UL;
   withalti = 0;
   comintval = 5UL;
   comcnt = 0UL;
   Parms();
   opentty();
   for (;;) {
      len = osi_RdBin(tty, (X2C_LOC *)tbuf, 1024u/1u, 1024UL);
      if (len<=0L) {
         osi_Close(tty);
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
                           InOut_WriteInt((X2C_INT32)mediantime, 4UL);
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
            else if ((X2C_CARD8)c>' ' && gpsp<99UL) {
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
