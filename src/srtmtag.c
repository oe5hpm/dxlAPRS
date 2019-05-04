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
#define srtmtag_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef pngread_H_
#include "pngread.h"
#endif

/* subtract 10000m in srtm file if in png blue, 20000 if green,
                30000 if black */
#define srtmtag_SRTM1 25934402

#define srtmtag_SRTM3 2884802

#define srtmtag_SRTM1XY 3601

#define srtmtag_SRTM3XY 1201

#define srtmtag_ATTRSUB 10000

#define srtmtag_ATTRNEG 1000

#define srtmtag_BYTESPERPIX 3

struct PIX8;


struct PIX8 {
   uint8_t r8;
   uint8_t g8;
   uint8_t b8;
};

typedef struct PIX8 * pROWS;

typedef pROWS PNGBUF[3601];

static char fn[4096];

static char fnt[4096];

static uint32_t i;

static uint32_t j;

static uint32_t ssize;

static uint16_t ar;

static short ai;

static int32_t f1;

static int32_t l;

static int32_t lat;

static int32_t long0;

static int32_t ilat;

static int32_t ilong;

static uint8_t doset;

static uint32_t x;

static uint32_t y;

static uint32_t xx;

static uint32_t yy;

static uint32_t stati[8];

static char verb;

static char modified;

static char add;


struct _0 {
   short * Adr;
   size_t Len0;
   size_t Size1;
   size_t Len1;
};

static struct _0 * ps;

static PNGBUF rows;

static int32_t maxx;

static int32_t maxy;

static int32_t maxxbyte;

static int32_t res;

static char bu[3601][10803];


static void Err(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Err() */


static int32_t num(const char s[], uint32_t s_len, uint32_t p)
{
   if ((p>s_len-1 || (uint8_t)s[p]<'0') || (uint8_t)s[p]>'9') {
      Err("file name not a srtm file", 26ul);
   }
   return (int32_t)((uint32_t)(uint8_t)s[p]-48UL);
} /* end num() */


static uint8_t coltran(const char c[], uint32_t c_len, uint32_t p)
{
   uint32_t b;
   uint32_t g;
   uint32_t r;
   uint32_t s;
   r = (uint32_t)(uint8_t)c[p];
   g = (uint32_t)(uint8_t)c[p+1UL];
   b = (uint32_t)(uint8_t)c[p+2UL];
   s = (r+g+b)/3UL;
   if (b>=2UL*s) return 3U;
   /* blue */
   if (g>=2UL*s) return 2U;
   /* green */
   /*  IF r>=2*s THEN WrStr("r") END;   (* red *) */
   if ((s<70UL && labs((int32_t)(s-b))<10L) && labs((int32_t)(s-g))<10L) {
      return 1U;
   }
   /* black */
   return 0U;
} /* end coltran() */


static uint32_t chkmeta(short a)
{
   return (uint32_t)((int32_t)a+41000L)/10000UL;
} /* end chkmeta() */


static void resmeta(short * a)
{
   *a = (short)((int32_t)((uint32_t)((int32_t)*a+41000L)%10000UL)
                -1000L);
} /* end resmeta() */


static void setmeta(short * a, int32_t t)
{
   resmeta(a);
   *a += (short)((t-4L)*10000L);
} /* end setmeta() */


static void showst(const char h[], uint32_t h_len)
{
   uint32_t x0;
   if (verb) {
      osi_WrStrLn(h, h_len);
      osi_WrStr("error pixels:", 14ul);
      osic_WrINT32(stati[0U]+stati[7U], 1UL);
      osi_WrStrLn("", 1ul);
      for (x0 = 1UL; x0<=6UL; x0++) {
         if (stati[x0]) {
            osi_WrStr("pixels typ:", 12ul);
            osic_WrINT32(x0, 1UL);
            osi_WrStr(" count: ", 9ul);
            osic_WrINT32(stati[x0], 1UL);
            osi_WrStrLn("", 1ul);
         }
      } /* end for */
   }
} /* end showst() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[2];
   uint32_t tmp0;
   uint32_t tmp1;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   verb = 1;
   modified = 0;
   add = 1;
   doset = 0U;
   osi_NextArg(fn, 4096ul);
   if (fn[0U]=='-') {
      if (fn[1U]=='t') {
         doset = 0U;
         osi_NextArg(fn, 4096ul);
         i = 0UL;
         while (i<=4095UL && fn[i]) {
            l = (int32_t)((uint32_t)(uint8_t)fn[i]-48UL);
            if ((l<1L || l>6L) || l==4L) Err("tags 12356", 11ul);
            doset |= (1U<<l);
            ++i;
         }
      }
      else if (fn[1U]=='h') {
         osi_WrStrLn("", 1ul);
         osi_WrStrLn("srtmtag [-t <taglist>] <srtmfile> [<extentfile>]",
                49ul);
         osi_WrStrLn("modify srtm-files by add/subtract multiples of 10000m t\
o store attributes", 74ul);
         osi_WrStrLn("-t <taglist>           manage layers 1(urban), 2(wood),\
 3(water), 5(?), 6(?) -t 123", 84ul);
         osi_WrStrLn("                       4(original), no tags: inspect sr\
tm file only", 68ul);
         osi_WrStrLn("<srtmfile>             3601x3601 srtm1 or 1201x1201 srt\
m3 file", 63ul);
         osi_WrStrLn("<extentfile>           3601x3601 png image with tag col\
ours", 60ul);
         osi_WrStrLn("                       no file: delete tags in srtmfile\
 if in taglist", 70ul);
         X2C_ABORT();
      }
      else Err("usage [[-t <taglist>] <srtmfile> [<extentfile>]", 48ul);
      osi_NextArg(fn, 4096ul);
   }
   f1 = osi_OpenRead(fn, 4096ul);
   if (f1<0L) Err("srtm file open", 15ul);
   i = (uint32_t)osic_Size(f1);
   if (i==25934402UL) ssize = 3601UL;
   else if (i==2884802UL) ssize = 1201UL;
   else Err("file not srtem1/3 size", 23ul);
   i = aprsstr_Length(fn, 4096ul);
   if (i<11UL) Err("file name not a srtm file", 26ul);
   if (((fn[i-1UL]!='t' || fn[i-2UL]!='g') || fn[i-3UL]!='h')
                || fn[i-4UL]!='.') Err("file name not a srtm file", 26ul);
   lat = num(fn, 4096ul, i-9UL)+num(fn, 4096ul, i-10UL)*10L;
   if (fn[i-11UL]=='S') lat = -lat;
   else if (fn[i-11UL]!='N') Err("file name not a srtm file", 26ul);
   long0 = num(fn, 4096ul, i-5UL)+num(fn, 4096ul, i-6UL)*10L+num(fn, 4096ul,
                i-7UL)*100L;
   if (fn[i-8UL]=='W') long0 = -long0;
   else if (fn[i-8UL]!='E') Err("file name not a srtm file", 26ul);
   ilat = 90L-(90L-lat);
   ilong = (180L+long0)-180L;
   /*WrInt(ilat, 10);WrInt(ilong, 10); WrStrLn("=ilat ilong"); */
   osi_NextArg(fnt, 4096ul);
   if (fnt[0U]) {
      if (verb) {
         osi_WrStr("open extent file ", 18ul);
         osi_WrStrLn(fnt, 4096ul);
      }
      maxx = 3601L;
      maxy = 3601L;
      maxxbyte = maxx*3L; /* maxxbyte = maxx*4: switch on alpha channel */
      for (i = 0UL; i<=3600UL; i++) {
         rows[i] = (pROWS)bu[i];
      } /* end for */
      if (verb) osi_WrStrLn(fnt, 4096ul);
      res = readpng(fnt, (char * *)rows, &maxx, &maxy, &maxxbyte);
      if (res<0L) Err("png read", 9ul);
      if (verb) {
         osic_WrINT32((uint32_t)maxx, 1UL);
         osi_WrStr("x", 2ul);
         osic_WrINT32((uint32_t)maxy, 1UL);
         osi_WrStrLn("=extent image", 14ul);
      }
   }
   if (verb) osi_WrStrLn("read srtm", 10ul);
   X2C_DYNALLOCATE((char **) &ps,2u,(tmp[0] = ssize,tmp[1] = ssize,tmp),
                2u);
   if (ps==0) Err("srtm image out of memory", 25ul);
   memset((char *)stati,(char)0,32UL);
   tmp0 = ssize-1UL;
   y = 0UL;
   if (y<=tmp0) for (;; y++) {
      l = osi_RdBin(f1, (char *)(ps->Adr+(y)*ps->Len0), ps->Size1/1u,
                ps->Size1);
      if (l<(int32_t)ps->Size1) Err("srtm file read error", 21ul);
      tmp1 = ps->Len0-1;
      x = 0UL;
      if (x<=tmp1) for (;; x++) {
         ar = (uint16_t)ps->Adr[(y)*ps->Len0+x];
         ai = (short)(ar/256U+ar*256U); /* reverse byte order */
         j = chkmeta(ai);
         if (j<=7UL) ++stati[j];
         if (X2C_IN(j,8,doset)) {
            resmeta(&ai);
            modified = 1;
         }
         ps->Adr[(y)*ps->Len0+x] = ai;
         if (x==tmp1) break;
      } /* end for */
      if (y==tmp0) break;
   } /* end for */
   osic_Close(f1);
   showst("now:", 5ul);
   if (ssize!=3601UL && ssize!=1201UL) Err("not srtm size", 14ul);
   memset((char *)stati,(char)0,32UL);
   if (fnt[0U]) {
      tmp0 = ssize-1UL;
      y = 0UL;
      if (y<=tmp0) for (;; y++) {
         tmp1 = ssize-1UL;
         x = 0UL;
         if (x<=tmp1) for (;; x++) {
            if (ssize!=3601UL) {
               xx = x*3UL;
               yy = y*3UL;
            }
            else {
               xx = x;
               yy = y;
            }
            if (X2C_IN(chkmeta(ps->Adr[(y)*ps->Len0+x]),8,0x7EU)) {
               /* do not modify error pixels */
               j = (uint32_t)coltran(bu[yy], 10803ul, xx*3UL);
               if (X2C_IN(j,8,doset)) {
                  setmeta(&ps->Adr[(y)*ps->Len0+x], (int32_t)j);
                  if (j<=7UL) ++stati[j];
                  modified = 1;
               }
            }
            if (x==tmp1) break;
         } /* end for */
         if (y==tmp0) break;
      } /* end for */
   }
   showst("added:", 7ul);
   if (modified) {
      if (verb) osi_WrStrLn("write srtm", 11ul);
      f1 = osi_OpenWrite(fn, 4096ul);
      if (f1<0L) Err("srtm file write", 16ul);
      tmp0 = ssize-1UL;
      y = 0UL;
      if (y<=tmp0) for (;; y++) {
         tmp1 = ssize-1UL;
         x = 0UL;
         if (x<=tmp1) for (;; x++) {
            ar = (uint16_t)ps->Adr[(y)*ps->Len0+x];
            ps->Adr[(y)*ps->Len0+x] = (short)(ar/256U+ar*256U);
                /* reverse byte order */
            if (x==tmp1) break;
         } /* end for */
         osi_WrBin(f1, (char *)(ps->Adr+(y)*ps->Len0), ps->Size1/1u,
                ps->Size1);
         if (y==tmp0) break;
      } /* end for */
      osic_Close(f1);
   }
/*  WrStrLn(""); */
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
