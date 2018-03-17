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
#define radiorange_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#include <math.h>
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef pngwritepalette_H_
#include "pngwritepalette.h"
#endif

/* make image of radio range of 1 or 2 antennas with srtm data by oe5dxl */
/*, wgs84r, wgs84s*/
#define radiorange_NOALT 10000

#define radiorange_PI 3.1415926535

#define radiorange_PI2 6.283185307

#define radiorange_LF "\012"

#define radiorange_MAXLAT 1.484

#define radiorange_TILESIZE 256

#define radiorange_FULLUM 255
/* full bright in image */

#define radiorange_DIRPERM 493

#define radiorange_PALETTELEN 256

#define radiorange_NL "\012"


struct IMAGE {
   uint8_t * Adr;
   size_t Len0;
   size_t Size1;
   size_t Len1;
};

typedef struct IMAGE * pIMAGE;

typedef int32_t COLOUR[5];

static pIMAGE image;

static pIMAGE image2;

static pIMAGE tileimg;
/*    tileimg:POINTER TO ARRAY OF CARD8; */

static char imagefn[1024];

static char osmdir[1024];

static int32_t xsize;

static int32_t ysize;

static int32_t initzoom;

static int32_t tozoom;

static int32_t inittilex;

static int32_t inittiley;

static int32_t anta;

static int32_t antb;

static int32_t antc;

static int32_t contrast;

static int32_t ret;

static int32_t pngdepth;

static int32_t lasttilezoom;

static int32_t lasttiledir;

static int32_t draft;

static COLOUR colour1;

static COLOUR colour2;

static COLOUR colour3;

static struct aprsstr_POSITION posa;

static struct aprsstr_POSITION posb;

static struct aprsstr_POSITION mappos;

static struct aprsstr_POSITION mappos2;

static float refraction;

static float finezoom;

static float shiftx;

static float shifty;

static float igamma;

static float xs2;

static float ys2;

static uint8_t gammatab[256];

static struct PNGPALETTE palette[256];

static uint8_t transpaency[256];

static uint8_t clut[65536];

static char writeempty;

static char verb;

static char srtminterpol;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void Werrint(int32_t n)
{
   char s[100];
   aprsstr_IntToStr(n, 0UL, s, 100ul);
   osi_Werr(s, 100ul);
} /* end Werrint() */


static uint32_t truncc(float r)
{
   if (r<=0.0f) return 0UL;
   else if (r>=2.E+9f) return 2000000000UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static void posinval(struct aprsstr_POSITION * pos)
{
   pos->long0 = 0.0f;
   pos->lat = 0.0f;
} /* end posinval() */


static char posvalid(const struct aprsstr_POSITION pos)
{
   return pos.lat!=0.0f || pos.long0!=0.0f;
} /* end posvalid() */


static void limpos(struct aprsstr_POSITION * pos)
{
   if (pos->long0>3.1415926535f) pos->long0 = 3.1415926535f;
   else if (pos->long0<(-3.1415926535f)) pos->long0 = (-3.1415926535f);
   if (pos->lat>1.484f) pos->lat = 1.484f;
   else if (pos->lat<(-1.484f)) pos->lat = (-1.484f);
} /* end limpos() */


static void wgs84s(float lat, float long0, float nn, float * x,
                float * y, float * z)
/* km */
{
   float c;
   float h;
   h = nn+6370.0f;
   *z = (float)((double)h*sin((double)lat));
   c = (float)cos((double)lat);
   *y = (float)((double)h*sin((double)long0)*(double)c);
   *x = (float)((double)h*cos((double)long0)*(double)c);
} /* end wgs84s() */


static void wgs84r(float x, float y, float z, float * lat,
                float * long0, float * heig)
/* km */
{
   float h;
   h = x*x+y*y;
   if ((float)fabs(x)>(float)fabs(y)) {
      *long0 = (float)atan((double)(X2C_DIVR(y,x)));
      if (x<0.0f) {
         if (y>0.0f) *long0 = 3.1415926535f+*long0;
         else *long0 = *long0-3.1415926535f;
      }
   }
   else {
      *long0 = (float)(1.57079632675-atan((double)(X2C_DIVR(x,y))));
      if (y<0.0f) *long0 = *long0-3.1415926535f;
   }
   *lat = (float)atan(X2C_DIVL((double)z,sqrt((double)h)));
   *heig = (float)(sqrt((double)(h+z*z))-6370.0);
} /* end wgs84r() */


static float realzoom(int32_t zi, float zf)
{
   return ((float)zi+zf)-1.0f;
} /* end realzoom() */


static float expzoom(int32_t z)
{
   return (float)(uint32_t)X2C_LSH(0x1UL,32,z);
} /* end expzoom() */


static float latproj(float l)
{
   return (float)log(X2C_DIVL(sin((double)l)+1.0,
                cos((double)l)));
} /* end latproj() */

#define radiorange_TOL 0.0001


static void mercator(float lon, float lat, int32_t zoom,
                int32_t * tilex, int32_t * tiley, float * x,
                float * y)
{
   float z;
   if (lat>1.484f) lat = 1.484f;
   else if (lat<(-1.484f)) lat = (-1.484f);
   if (lon>3.1414926535f) lon = 3.1414926535f;
   else if (lon<(-3.1414926535f)) lon = (-3.1414926535f);
   z = expzoom(zoom);
   *x = (0.5f+lon*1.5915494309644E-1f)*z;
   *y = (0.5f-latproj(lat)*1.5915494309644E-1f)*z;
   *tilex = (int32_t)truncc(*x);
   *tiley = (int32_t)truncc(*y);
   *x = (*x-(float)*tilex)*256.0f;
   *y = (*y-(float)*tiley)*256.0f;
} /* end mercator() */


static int32_t mapxy(struct aprsstr_POSITION pos, float * x,
                float * y)
{
   int32_t tiley;
   int32_t tilex;
   float ys;
   float xs;
   if (posvalid(pos)) {
      mercator(pos.long0, pos.lat, initzoom, &tilex, &tiley, x, y);
      /*WrInt(tilex, 10);WrStrLn(" tilex"); */
      tilex -= inittilex;
      tiley -= inittiley;
      *x = ((*x+(float)(tilex*256L))-shiftx)*finezoom;
      *y = (float)ysize-(((float)(tiley*256L)+*y)-shifty)*finezoom;
      xs = (float)xsize;
      ys = (float)ysize;
      /*WrFixed(x,3, 10);WrStrLn(" x"); */
      if (((*x>=0.0f && *x<=xs) && *y>=0.0f) && *y<=ys) return 0L;
   }
   return -1L;
} /* end mapxy() */


static void xytodeg(float x, float y, struct aprsstr_POSITION * pos)
{
   int32_t zi;
   float ysf;
   float zoom;
   float pixrad;
   zoom = realzoom(initzoom, finezoom);
   ysf = (float)ysize;
   y = ysf-y;
   if (y<0.0f) y = 0.0f;
   else if (y>ysf) y = ysf;
   if ((float)fabs(mappos.lat)>1.484f) {
      posinval(pos);
      return;
   }
   zi = (int32_t)truncc(zoom);
   pixrad = X2C_DIVR(6.283185307f,
                ((1.0f+zoom)-(float)zi)*256.0f*expzoom((int32_t)
                truncc(zoom)));
   pos->long0 = mappos.long0+pixrad*x;
   pos->lat = (float)(2.0*atan(exp((double)(latproj(mappos.lat)
                -pixrad*y)))-1.57079632675);
   limpos(pos);
} /* end xytodeg() */


static void Parms(void)
{
   char err;
   char h[1024];
   int32_t n;
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            osi_NextArg(imagefn, 1024ul);
            if (imagefn[0U]==0 || imagefn[0U]=='-') {
               Error("-i <imagefilename>", 19ul);
            }
         }
         else if (h[1U]=='o') {
            osi_NextArg(osmdir, 1024ul);
            if (osmdir[0U]==0 || osmdir[0U]=='-') Error("-i <osmdir>", 12ul);
         }
         else if (h[1U]=='x') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &xsize) || xsize<32L) || xsize>32000L) {
               Error("-x <size>", 10ul);
            }
         }
         else if (h[1U]=='y') {
            /*         INC(xsize,2); */
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &ysize) || ysize<32L) || ysize>32000L) {
               Error("-y <size>", 10ul);
            }
         }
         else if (h[1U]=='c') {
            /*        INC(ysize,2); */
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &contrast) || contrast<0L) || contrast>=1000L) {
               Error("-c <contrast>", 14ul);
            }
         }
         else if (h[1U]=='p') {
            osi_NextArg(libsrtm_srtmdir, 1024ul);
            if (libsrtm_srtmdir[0U]==0 || libsrtm_srtmdir[0U]=='-') {
               Error("-p <srtm-folder-path>", 22ul);
            }
         }
         else if (h[1U]=='a') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&posa, h, 1024ul);
            if (!posvalid(posa)) {
               if (!aprsstr_StrToFix(&posa.lat, h,
                1024ul) || (float)fabs(posa.lat)>=90.0f) {
                  Error("-a <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&posa.long0, h,
                1024ul) || (float)fabs(posa.long0)>180.0f) {
                  Error("-a <lat> <long> or <locator>", 29ul);
               }
               posa.lat = posa.lat*1.7453292519444E-2f;
               posa.long0 = posa.long0*1.7453292519444E-2f;
            }
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&posb, h, 1024ul);
            if (!posvalid(posb)) {
               if (!aprsstr_StrToFix(&posb.lat, h,
                1024ul) || (float)fabs(posb.lat)>=90.0f) {
                  Error("-b <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&posb.long0, h,
                1024ul) || (float)fabs(posb.long0)>180.0f) {
                  Error("-b <lat> <long> or <locator>", 29ul);
               }
               posb.lat = posb.lat*1.7453292519444E-2f;
               posb.long0 = posb.long0*1.7453292519444E-2f;
            }
         }
         else if (h[1U]=='m') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&mappos, h, 1024ul);
            if (!posvalid(mappos)) {
               if (!aprsstr_StrToFix(&mappos.lat, h,
                1024ul) || (float)fabs(mappos.lat)>=90.0f) {
                  Error("-m <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&mappos.long0, h,
                1024ul) || (float)fabs(mappos.long0)>180.0f) {
                  Error("-m <lat> <long> or <locator>", 29ul);
               }
               mappos.lat = mappos.lat*1.7453292519444E-2f;
               mappos.long0 = mappos.long0*1.7453292519444E-2f;
            }
         }
         else if (h[1U]=='n') {
            osi_NextArg(h, 1024ul);
            aprsstr_loctopos(&mappos2, h, 1024ul);
            if (!posvalid(mappos2)) {
               if (!aprsstr_StrToFix(&mappos2.lat, h,
                1024ul) || (float)fabs(mappos2.lat)>=90.0f) {
                  Error("-n <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&mappos2.long0, h,
                1024ul) || (float)fabs(mappos2.long0)>180.0f) {
                  Error("-n <lat> <long> or <locator>", 29ul);
               }
               mappos2.lat = mappos2.lat*1.7453292519444E-2f;
               mappos2.long0 = mappos2.long0*1.7453292519444E-2f;
            }
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &n) || n<0L) {
               Error("-M <bytes>", 11ul);
            }
            libsrtm_srtmmaxmem = (uint32_t)n;
         }
         else if (h[1U]=='q') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &draft) || draft<0L) || draft>3L) {
               Error("-q <quality> 0=fast 3=best (2)", 31ul);
            }
         }
         else if (h[1U]=='W') {
            for (n = 0L; n<=4L; n++) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToInt(h, 1024ul, &colour1[n])) {
                  Error("-W <red> <green> <blue> <transp> <blacktransp>",
                47ul);
               }
            } /* end for */
         }
         else if (h[1U]=='U') {
            for (n = 0L; n<=3L; n++) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToInt(h, 1024ul, &colour2[n])) {
                  Error("-U <red> <green> <blue> <transp>", 33ul);
               }
            } /* end for */
         }
         else if (h[1U]=='V') {
            for (n = 0L; n<=3L; n++) {
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToInt(h, 1024ul, &colour3[n])) {
                  Error("-V <red> <green> <blue> <transp>", 33ul);
               }
            } /* end for */
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &anta) || anta<0L) || anta>10000L) {
               Error("-A <meter> (0..10000)", 22ul);
            }
         }
         else if (h[1U]=='B') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &antb) || antb<0L) || antb>10000L) {
               Error("-B <meter> (0..10000)", 22ul);
            }
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &antc) || antc<0L) || antc>10000L) {
               Error("-C <meter> (0..10000)", 22ul);
            }
         }
         else if (h[1U]=='z' || h[1U]=='Z') {
            writeempty = h[1U]=='Z';
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &initzoom) || initzoom<1L) || initzoom>18L) {
               Error("-z <fromzoom> <tozoom> (1..18)", 31ul);
            }
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &tozoom) || tozoom<1L) || tozoom>18L) {
               Error("-z <fromzoom> <tozoom> (1..18)", 31ul);
            }
            if (initzoom<tozoom) {
               n = tozoom;
               tozoom = initzoom;
               initzoom = n;
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul,
                &pngdepth) || ((pngdepth!=1L && pngdepth!=2L)
                && pngdepth!=4L) && pngdepth!=8L) {
               Error("-d <pngdepth> (1,2,4,8)", 24ul);
            }
         }
         else if (h[1U]=='R') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&refraction, h,
                1024ul) || refraction<0.0f) || refraction>1.0f) {
               Error("-R <refraction> [0.0..1.0]", 27ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&igamma, h,
                1024ul) || igamma<0.1f) || igamma>10.0f) {
               Error("-g <gamma> [0.1..10.0]", 23ul);
            }
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='Q') srtminterpol = 0;
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("Radiorange from 1 or 2 Position(s)", 35ul);
               osi_WrStrLn(" -A <m>                            Antenna A over\
 ground [m] (10)", 66ul);
               osi_WrStrLn(" -a <lat> <long> | [locator]       Position A lat\
 long (degrees) or qth locator", 80ul);
               osi_WrStrLn(" -B <m>                            Antenna B over\
 ground [m] (10)", 66ul);
               osi_WrStrLn(" -b <lat> <long> | [locator]       Position B lat\
 long (degrees) or qth locator", 80ul);
               osi_WrStrLn(" -C <m>                            0 for ground e\
cho or Repeater Antenna over ground [m] (0)", 93ul);
               osi_WrStrLn(" -c <contrast>                     0 for hard 30 \
for smooth area margin (3)", 76ul);
               osi_WrStrLn(" -d <depth>                        png palette bi\
ts/pixel 1,2,4,8 (1)", 70ul);
               osi_WrStrLn(" -g <gamma>                        image gamma 0.\
1..10.0 (2.2)", 63ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -i <filename>                     enable fullsiz\
e image with Image File Name", 78ul);
               osi_WrStrLn(" -M <bytes>                        srtmcache size\
 (200000000)", 62ul);
               osi_WrStrLn(" -m <lat> <long> | [locator]       left up positi\
on of image", 61ul);
               osi_WrStrLn(" -n <lat> <long> | [locator]       alternate to -\
x -y: right down position of image", 84ul);
               osi_WrStrLn("                                       set -x -y \
too as size limit", 67ul);
               osi_WrStrLn(" -o <path>                         enable tiles w\
ith Path Name", 63ul);
               osi_WrStrLn(" -p <path>                         srtm directory\
 path", 55ul);
               osi_WrStrLn(" -Q                                srtm interpola\
tion off (on)", 63ul);
               osi_WrStrLn(" -q <quality>                      0=fast 3=best \
(2)", 53ul);
               osi_WrStrLn(" -R <refraction>                   0.0(vacuum), 1\
.0(earth is a disk) (0.25)", 76ul);
               osi_WrStrLn(" -U <red> <green> <blue> <transp>  colour antenna\
 1", 52ul);
               osi_WrStrLn(" -V <red> <green> <blue> <transp>  colour antenna\
 2", 52ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osi_WrStrLn(" -W <red> <green> <blue> <transp> <blacktransp>",
                 48ul);
               osi_WrStrLn("                                   (0..255) colou\
r antenna 1 or reflection", 75ul);
               osi_WrStrLn(" -x <size>                         Image size (60\
0)", 52ul);
               osi_WrStrLn(" -y <size>                         Image size (40\
0)", 52ul);
               osi_WrStrLn(" -z <from> <to>                    make tiles fro\
m..to zoomlevel 1..18", 71ul);
               osi_WrStrLn(" -Z <from> <to>                    as -z but writ\
e empty tiles too", 67ul);
               osi_WrStrLn("", 1ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_Werr(">", 2ul);
      osi_Werr(h, 1024ul);
      osi_Werr("< use -h\012", 10ul);
      X2C_ABORT();
   }
} /* end Parms() */


static void makegammatab(void)
{
   uint32_t c;
   double g;
   g = (double)(X2C_DIVR(1.0f,igamma));
   gammatab[0U] = 0U;
   for (c = 1UL; c<=255UL; c++) {
      gammatab[c] = (uint8_t)(uint32_t)X2C_TRUNCC(exp(log((double)
                (X2C_DIVR((float)c,255.0f)))*g)*255.5,0UL,
                X2C_max_longcard);
   } /* end for */
} /* end makegammatab() */

/*
PROCEDURE wrpng;

VAR pngimg:pngwrite.PNGPIXMAP;
    x, y, ret:INTEGER;
    c:CARD8;
BEGIN

  ALLOCATE(pngimg.image, xsize*ysize*3);
  IF pngimg.image<>NIL THEN
--    makegammatab;
    FOR y:=0 TO ysize-3 DO
      FOR x:=0 TO xsize-3 DO
        c:=image^[x+1][ysize-y-3];
        WITH pngimg.image^[x+y*(xsize-2)] DO red:=c; green:=c; blue:=c END;
      END;
    END;
    pngimg.width:=xsize-2;
    pngimg.height:=ysize-2;
    ret:=pngwrite.writepng(imagefn, pngimg);
    DEALLOCATE(pngimg.image, xsize*ysize*3);
  ELSE Werr("png write out of memory"+LF) END;

END wrpng;
*/
/*
PROCEDURE wrpng;

VAR pngimg:pngwritebw.PNGPIXMAP;
    x, y, ret:INTEGER;
BEGIN

  ALLOCATE(pngimg.image, xsize*ysize);
  IF pngimg.image<>NIL THEN
--    makegammatab;
    FOR y:=0 TO ysize-3 DO
      FOR x:=0 TO xsize-3 DO
        pngimg.image^[x+y*(xsize-2)]:=gammatab[image^[x+1][ysize-y-3]];
      END;
    END;
    pngimg.width:=xsize-2;
    pngimg.height:=ysize-2;
    ret:=pngwritebw.writepng(imagefn, pngimg);
    DEALLOCATE(pngimg.image, xsize*ysize);
  ELSE Werr("png write out of memory"+LF) END;

END wrpng;
*/
/*
PROCEDURE wrpng(imagefn-:ARRAY OF CHAR; depth:INTEGER;
                x0, y0, xw, yw, scale:INTEGER):INTEGER;

VAR pngimg:pngwritepalette.PNGPIXMAP;
    x, y, i, xi, yi, is, ret, palettelen, d8:INTEGER;
    c, cc:CARDINAL;
    notempty:BOOLEAN;
BEGIN
  palettelen:=CAST(INTEGER,BITSET{depth});
  pngimg.palette:=ADR(palette);
  pngimg.palettelen:=palettelen;
  pngimg.palettedepth:=depth;
  pngimg.xbytes:=(xw*VAL(INTEGER, depth)+8) DIV 8;
  pngimg.trns:=ADR(transpaency);
  d8:=8 DIV depth;
  ALLOCATE(pngimg.image, VAL(CARDINAL, yw)*pngimg.xbytes);
--  FILL(pngimg.image, 0C, VAL(CARDINAL, yw)*pngimg.xbytes);
  notempty:=FALSE;
  IF pngimg.image<>NIL THEN
    FOR y:=0 TO yw-1 DO
      yi:=VAL(INTEGER,HIGH(image^[0]))-(y+y0)*scale;
      FOR x:=0 TO VAL(INTEGER,pngimg.xbytes)-1 DO
        xi:=x*d8+x0;
        FOR i:=xi TO xi+d8-1 DO
          is:=i*scale;
          cc:=0;
          IF (yi>=0) & (yi<=VAL(INTEGER,HIGH(image^[0])))
          & (is>=0) & (is<=VAL(INTEGER,
                HIGH(image^))) THEN cc:=image^[is][yi] END;
          c:=CAST(CARDINAL, SHIFT(CAST(BITSET,c), depth)) + cc;
        END;
        pngimg.image^[x+y*VAL(INTEGER, pngimg.xbytes)]:=c;
        IF c<>0 THEN notempty:=TRUE END;
      END;
    END;
    IF writeempty OR notempty THEN
      pngimg.width:=xw;
      pngimg.height:=yw;
      ret:=pngwritepalette.writepng(imagefn, pngimg);
      IF ret<0 THEN Werr("png write failed"+LF) END;
    END;
    DEALLOCATE(pngimg.image, VAL(CARDINAL,yw)*pngimg.xbytes);
  ELSE Werr("png write out of memory"+LF) END;

  RETURN 0
END wrpng;
*/
/*
PROCEDURE wrpng(imagefn-:ARRAY OF CHAR; depth:INTEGER;
                x0, y0, xw, yw, scale:INTEGER):INTEGER;

VAR pngimg:pngwritepalette.PNGPIXMAP;
    x, y, i, xi, yi, is, ret, palettelen, d8, xbytes:INTEGER;
    c, cc:CARDINAL;
    notempty:BOOLEAN;
himage:POINTER TO ARRAY[0..02FFFFFFFH] OF CARD8;

BEGIN
  palettelen:=CAST(INTEGER,BITSET{depth});
  pngimg.palette:=ADR(palette);
  pngimg.palettelen:=palettelen;
  pngimg.palettedepth:=depth;
  xbytes:=(xw*VAL(INTEGER, depth)+8) DIV 8;
  pngimg.trns:=ADR(transpaency);
  d8:=8 DIV depth;
  ALLOCATE(himage, yw*xbytes); IF himage=NIL THEN Werr("png write out of memory"+LF) END;
  ALLOCATE(pngimg.image, VAL(INTEGER,SIZE(pngwritepalette.pLINE))*yw);
                IF pngimg.image=NIL THEN Werr("png write out of memory"+LF)
                END; 

--  FILL(pngimg.image, 0C, VAL(CARDINAL, yw)*pngimg.xbytes);
  notempty:=FALSE;
  FOR y:=0 TO yw-1 DO
    pngimg.image^[y]:=ADR(himage^[y*xbytes]);
    yi:=VAL(INTEGER,HIGH(image^[0]))-(y+y0)*scale;
    FOR x:=0 TO xbytes-1 DO
      xi:=x*d8+x0;
      FOR i:=xi TO xi+d8-1 DO
        is:=i*scale;
        cc:=0;
        IF (yi>=0) & (yi<=VAL(INTEGER,HIGH(image^[0])))
        & (is>=0) & (is<=VAL(INTEGER,
                HIGH(image^))) THEN cc:=image^[is][yi] END;
        c:=CAST(CARDINAL, SHIFT(CAST(BITSET,c), depth)) + cc;
      END;
      himage^[x+y*xbytes]:=c;
      IF c<>0 THEN notempty:=TRUE END;
    END;
  END;
  IF writeempty OR notempty THEN
    pngimg.width:=xw;
    pngimg.height:=yw;
    ret:=pngwritepalette.writepng(imagefn, pngimg);
    IF ret<0 THEN Werr("png write failed"+LF) END;
  END;
  DEALLOCATE(himage, yw*xbytes);
  DEALLOCATE(pngimg.image, yw*xbytes);

  RETURN 0
END wrpng;
*/

static int32_t wrpng(const char imagefn0[], uint32_t imagefn_len,
                int32_t depth, int32_t x0, int32_t y00, int32_t xw,
                int32_t yw, int32_t scale)
{
   int32_t xbytes;
   int32_t d8;
   int32_t ret0;
   int32_t is;
   int32_t yi;
   int32_t xi;
   int32_t i;
   int32_t y;
   int32_t x;
   uint32_t cc;
   uint32_t c;
   char notempty;
   pROWPOINTERS pimage;
   pPNGPALETTE ppalette;
   uint32_t palettedepth;
   uint32_t palettelen;
   pRNS trns;
   int32_t tmp;
   int32_t tmp0;
   int32_t tmp1;
   palettelen = (uint32_t)(int32_t)(1UL<<depth);
   ppalette = (pPNGPALETTE)palette;
   palettedepth = (uint32_t)depth;
   xbytes = (xw*depth+7L)/8L;
   trns = (pRNS)transpaency;
   d8 = 8L/depth;
   osic_alloc((char * *) &pimage,
                (uint32_t)((int32_t)sizeof(pLINE)*yw));
   if (pimage==0) osi_Werr("png write out of memory\012", 25ul);
   /*FILL(pngimg.image, 0C, VAL(CARDINAL, yw)*pngimg.xbytes); */
   notempty = 0;
   tmp = yw-1L;
   y = 0L;
   if (y<=tmp) for (;; y++) {
      /*  pngimg.image^[y]:=ADR(himage^[y*xbytes]); */
      yi = (y+y00)*scale;
      if (tileimg==0) {
         pimage[y] = (pLINE) &image->Adr[((yw-y)-1L)*image->Len0+0UL];
      }
      else {
         pimage[y] = (pLINE) &tileimg->Adr[(y)*tileimg->Len0+0UL];
         yi = (int32_t)(image->Len1-1)-yi;
      }
      tmp0 = xbytes-1L;
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         xi = x*d8+x0;
         tmp1 = (xi+d8)-1L;
         i = xi;
         if (i<=tmp1) for (;; i++) {
            is = i*scale;
            cc = 0UL;
            if (((yi>=0L && yi<=(int32_t)(image->Len1-1)) && is>=0L)
                && is<=(int32_t)(image->Len0-1)) {
               cc = (uint32_t)image->Adr[(yi)*image->Len0+is];
            }
            c = (uint32_t)X2C_LSH((uint32_t)c,32,depth)+cc;
            if (i==tmp1) break;
         } /* end for */
         if (tileimg==0) image->Adr[(y)*image->Len0+x] = (uint8_t)c;
         else tileimg->Adr[(y)*tileimg->Len0+x] = (uint8_t)c;
         if (c) notempty = 1;
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   if (writeempty || notempty) {
      ret0 = writepng(imagefn0, pimage, (uint32_t)xw, (uint32_t)yw,
                ppalette, palettelen, palettedepth, trns);
      if (ret0<0L) osi_Werr("png write failed\012", 18ul);
   }
   osic_free((char * *) &pimage,
                (uint32_t)((int32_t)sizeof(pLINE)*yw));
   return 0L;
} /* end wrpng() */


static void postfilter(pIMAGE image0)
/* set missing pixels with median of neighbours */
{
   uint32_t y;
   uint32_t x;
   uint16_t c;
   uint32_t tmp;
   uint32_t tmp0;
   tmp = (image0->Len1-1)-1UL;
   y = 1UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = (image0->Len0-1)-1UL;
      x = 1UL;
      if (x<=tmp0) for (;; x++) {
         if (image0->Adr[(y)*image0->Len0+x]==0U) {
            c = (uint16_t)image0->Adr[(y)*image0->Len0+(x+1UL)];
            c += (uint16_t)image0->Adr[(y+1UL)*image0->Len0+x];
            c += (uint16_t)image0->Adr[(y)*image0->Len0+(x-1UL)];
            c += (uint16_t)image0->Adr[(y-1UL)*image0->Len0+x];
            image0->Adr[(y-1UL)*image0->Len0+x] = (uint8_t)(c/4U);
         }
         else {
            image0->Adr[(y-1UL)*image0->Len0+x] = image0->Adr[(y)
                *image0->Len0+x];
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end postfilter() */

#define radiorange_PART 0.2


static float meterperpix(pIMAGE image0)
{
   struct aprsstr_POSITION pos1;
   struct aprsstr_POSITION pos;
   float y;
   float x;
   float d;
   x = (float)(image0->Len0-1)*0.5f;
   y = (float)(image0->Len1-1)*0.5f;
   d = x*0.2f;
   xytodeg(x-d, y, &pos);
   xytodeg(x+d, y, &pos1);
   return X2C_DIVR(aprspos_distance(pos, pos1)*500.0f,d);
/* meter per pixel */
} /* end meterperpix() */

/*  VAR bri:CARD16; */
#define radiorange_MAXSLANT 0.5
/* shadow deepness to step relation */

#define radiorange_DISTHOP 6000
/* m jump sightline linear interpolated */

#define radiorange_PIXHOP 50000
/* m jump pixels linear interpolated */


static float goonframe(float framestep, float x, float tx,
                float y, float ty)
{
   if (y!=ty) return (1.0f+(float)fabs(X2C_DIVR(x-tx,y-ty)))*framestep;
   return X2C_max_real;
} /* end goonframe() */


static void setcol(uint8_t bri, uint8_t * p)
{
   if (bri>*p) *p = bri;
} /* end setcol() */


static int32_t Radiorange(pIMAGE image0, struct aprsstr_POSITION txpos,
                int32_t ant1, int32_t ant2, uint32_t smooth,
                uint32_t qualnum, float refrac)
{
   uint32_t qual;
   uint32_t frame;
   uint32_t yp;
   uint32_t xp;
   uint8_t bri;
   int32_t void0;
   int32_t nn;
   float refr;
   float dm;
   float alt1;
   float dyimag;
   float dximag;
   float mperpix;
   float sight;
   float hs;
   float atx;
   float arx;
   float h;
   float rais;
   float dnext;
   float dd;
   float d;
   float dpnext;
   float py0;
   float px0;
   float pixstep;
   float framestep;
   float osmooth;
   float lum;
   float dalt;
   float alt0;
   float alt;
   float resoltx;
   float resol;
   float oodist;
   float yi;
   float xi;
   float ytx;
   float xtx;
   float y;
   float x;
   float dz;
   float dy;
   float dx;
   float z1;
   float y1;
   float x1;
   float z0;
   float y00;
   float x0;
   struct aprsstr_POSITION dpos;
   struct aprsstr_POSITION pos1;
   struct aprsstr_POSITION pos0;
   struct aprsstr_POSITION pos;
   nn = (int32_t)X2C_TRUNCI(libsrtm_getsrtm(txpos, 0UL, &resoltx),
                X2C_min_longint,X2C_max_longint);
                /* altitude of tx and map resolution in m here */
   if (nn>=10000L) {
      libsrtm_closesrtmfile();
      return -1L;
   }
   atx = (float)(nn+ant1); /* ant1 over NN */
   wgs84s(txpos.lat, txpos.long0, atx*0.001f, &x0, &y00, &z0);
   arx = (float)ant2; /* ant2 over ground */
   xi = 0.0f;
   yi = 0.0f;
   nn = mapxy(txpos, &xtx, &ytx);
   frame = 0UL;
   mperpix = meterperpix(image0); /* meter per pixel */
   if (mperpix<1.0f) {
      libsrtm_closesrtmfile();
      return -2L;
   }
   if (qualnum==0UL) {
      framestep = 2.2f; /* pixel step along corner of image */
      qual = (uint32_t)X2C_TRUNCC(mperpix*2.0f,0UL,X2C_max_longcard);
      smooth = (smooth*22UL)/10UL;
   }
   else if (qualnum==1UL) {
      framestep = 1.5f;
      qual = (uint32_t)X2C_TRUNCC(mperpix,0UL,X2C_max_longcard);
      smooth = (smooth*15UL)/10UL;
   }
   else if (qualnum==2UL) {
      framestep = 1.0f;
      qual = (uint32_t)X2C_TRUNCC(mperpix*0.5f,0UL,X2C_max_longcard);
   }
   else {
      framestep = 0.95f;
      qual = 0UL;
   }
   if (!srtminterpol) qual = 1000UL;
   if (smooth>0UL) osmooth = X2C_DIVR(255.0f,(float)smooth);
   else osmooth = 255.0f;
   refr = refrac*0.0785f;
   for (;;) {
      sight = 0.0f;
      xytodeg(xi, yi, &pos); /* screen frame pos */
      if (((frame==0UL && ytx>0.0f || frame==2UL && ytx<(float)ysize)
                || frame==1UL && xtx>0.0f) || frame==3UL && xtx<(float)
                xsize) {
         wgs84s(pos.lat, pos.long0, atx*0.001f, &x1, &y1, &z1);
                /* screen frame xyz at ant1 alt */
         dx = x1-x0;
         dy = y1-y00;
         dz = z1-z0;
         oodist = dx*dx+dy*dy+dz*dz;
         if (oodist>1.E-6f) {
            dm = oodist*refr; /* full dist^2 */
            oodist = (float)(X2C_DIVL(0.001,sqrt((double)oodist)));
                /* 1/sight line length in m */
            pixstep = 0.0f;
            resol = resoltx;
            d = resol*oodist;
            rais = (-1.E+9f); /* initial sightline angle **/
            dnext = 0.0f;
            dpnext = 0.0f;
            do {
               /* do one sight line */
               if (d>=dnext) {
                  /* next fixpoint in interpolate sight line pos */
                  dnext = d+6000.0f*oodist;
                  if (dnext>1.0f) dnext = 1.0f;
                  dd = dnext-d;
                  if (dd!=0.0f) {
                     wgs84r(x0+dx*d, y00+dy*d, z0+dz*d, &pos1.lat,
                &pos1.long0, &alt1);
                     alt1 = alt1*1000.0f;
                     wgs84r(x0+dx*dnext, y00+dy*dnext, z0+dz*dnext, &pos.lat,
                 &pos.long0, &alt);
                     alt = alt*1000.0f;
                     dpos.lat = X2C_DIVR(pos.lat-pos1.lat,dd);
                     dpos.long0 = X2C_DIVR(pos.long0-pos1.long0,dd);
                     dalt = X2C_DIVR(alt-alt1,dd);
                     pos0.lat = pos1.lat-dpos.lat*d;
                     pos0.long0 = pos1.long0-dpos.long0*d;
                     alt0 = alt1-dalt*d;
                  }
               }
               pos.lat = pos0.lat+dpos.lat*d;
               pos.long0 = pos0.long0+dpos.long0*d;
               alt = (alt0+dalt*d)-d*d*dm;
                /* -dist(km)^2 * refrac * 0.0785 */
               h = libsrtm_getsrtm(pos,
                (uint32_t)X2C_TRUNCC((float)fabs(sight)+1.0f,0UL,
                X2C_max_longcard)*qual, &resol); /* ground over NN in m */
               if (h<10000.0f) {
                  /* srtm valid */
                  h = h-alt; /* m ground over searchpath */
                  hs = rais*d; /* h sight line m over searchpath */
                  sight = (h+arx)-hs;
                  if (sight>0.0f) {
                     /* rx antenna in sight */
                     if (d>=dpnext) {
                        /* next fixpoint in image pixel interpolation */
                        dpnext = d+50000.0f*oodist;
                        if (dpnext>1.0f) dpnext = 1.0f;
                        dd = dpnext-d;
                        if (dd!=0.0f) {
                           wgs84r(x0+dx*d, y00+dy*d, z0+dz*d, &pos1.lat,
                &pos1.long0, &alt1);
                           wgs84r(x0+dx*dpnext, y00+dy*dpnext, z0+dz*dpnext,
                &pos.lat, &pos.long0, &alt);
                           void0 = mapxy(pos, &x, &y);
                           void0 = mapxy(pos1, &px0, &py0);
                           dximag = X2C_DIVR(x-px0,dd);
                           dyimag = X2C_DIVR(y-py0,dd);
                           px0 = px0-dximag*d;
                           py0 = py0-dyimag*d;
                        }
                     }
                     x = px0+dximag*d;
                     y = py0+dyimag*d;
                     if (x>1.5f && y>1.5f) {
                        /* room for large low res pixel */
                        if (pixstep==0.0f) {
                           pixstep = mperpix*oodist*framestep;
                        }
                        xp = (uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
                        yp = (uint32_t)X2C_TRUNCC(y,0UL,
                X2C_max_longcard)+1UL; /* postfilter shifts 1 down */
                        if (xp<image0->Len0-1 && yp<image0->Len1-1) {
                           /* 1 pixel room for large low res pixel */
                           lum = sight*osmooth;
                /* luma is equal to riseing higth */
                           if (lum>255.0f) lum = 255.0f;
                           bri = (uint8_t)(uint32_t)X2C_TRUNCC(lum,0UL,
                X2C_max_longcard);
                           image0->Adr[(yp)*image0->Len0+xp] = (uint8_t)
                (uint32_t)X2C_TRUNCC(lum,0UL,X2C_max_longcard);
                           if (qualnum<=1UL) {
                              bri = (uint8_t)(uint32_t)
                X2C_TRUNCC(lum*0.8f,0UL,X2C_max_longcard);
                              setcol(bri,
                &image0->Adr[(yp)*image0->Len0+(xp+1UL)]);
                              setcol(bri,
                &image0->Adr[(yp+1UL)*image0->Len0+xp]);
                              setcol(bri,
                &image0->Adr[(yp+1UL)*image0->Len0+(xp-1UL)]);
                              setcol(bri,
                &image0->Adr[(yp-1UL)*image0->Len0+xp]);
                              if (qualnum==0UL) {
                                 bri = (uint8_t)(uint32_t)
                X2C_TRUNCC(lum*0.5f,0UL,X2C_max_longcard);
                                 setcol(bri,
                &image0->Adr[(yp+1UL)*image0->Len0+(xp-1UL)]);
                                 setcol(bri,
                &image0->Adr[(yp-1UL)*image0->Len0+(xp-1UL)]);
                                 setcol(bri,
                &image0->Adr[(yp+1UL)*image0->Len0+(xp+1UL)]);
                                 setcol(bri,
                &image0->Adr[(yp-1UL)*image0->Len0+(xp+1UL)]);
                              }
                           }
                        }
                     }
                  }
                  if (h>hs) {
                     rais = X2C_DIVR(h,d);
                /* this point has light on ground */
                  }
                  if (sight>-resol) {
                     if (pixstep==0.0f) d = d+resol*oodist;
                     else d = d+pixstep;
                  }
                  else if (resol>-(sight*0.5f)) d = d+resol*oodist;
                  else d = d-sight*oodist*0.5f;
               }
               else d = d+resol*oodist;
            } while (d<=1.0f);
         }
      }
      /* go along next side of image */
      switch (frame) {
      case 0UL:
         xi = xi+goonframe(framestep, xi, xtx, yi, ytx);
         if (xi>=(float)(image0->Len0-1)) {
            xi = 0.0f;
            ++frame;
         }
         break;
      case 1UL:
         yi = yi+goonframe(framestep, yi, ytx, xi, xtx);
         if (yi>=(float)(image0->Len1-1)) {
            yi = (float)(image0->Len1-1);
            ++frame;
         }
         break;
      case 2UL:
         xi = xi+goonframe(framestep, xi, xtx, yi, ytx);
         if (xi>=(float)(image0->Len0-1)) {
            xi = (float)(image0->Len0-1);
            ++frame;
         }
         break;
      default:;
         if (yi<0.0f) goto loop_exit;
         yi = yi-goonframe(framestep, yi, ytx, xi, xtx);
         break;
      } /* end switch */
   }
   loop_exit:;
   postfilter(image0); /* fill in missing pixels */
   return 0L;
} /* end Radiorange() */


static void wrtiles(struct aprsstr_POSITION mpos, int32_t zoom,
                int32_t tozoom0)
{
   int32_t scale;
   int32_t ret0;
   int32_t y00;
   int32_t x0;
   int32_t ty;
   int32_t ty0;
   int32_t y;
   int32_t tx;
   float yr;
   float xr;
   char s[256];
   char fn[256];
   char ok0;
   size_t tmp[2];
   /*
     scale:=1;
     REPEAT
       mercator(mpos.long, mpos.lat, zoom, tx0, ty, xr, yr);
       x0:=VAL(INTEGER, xr+0.5);
       y0:=(1-VAL(INTEGER, yr+0.5));
   WrInt(tx0,10);WrInt(ty,10);WrInt(x0,10);WrInt(y0,10);
                WrStrLn(" tx ty x y");
       REPEAT
         tx:=tx0;
         x:=-x0;
   
         REPEAT
           Assign(fn, osmdir);
           Append(fn, DIRSEP);
           IntToStr(zoom, 0, s); Append(fn, s);
           IF lasttilezoom<>zoom THEN                    (* make dir once *)
             ok:=CreateDir(fn, DIRPERM);
              lasttilezoom:=zoom;
           END;
           Append(fn, DIRSEP);
           IntToStr(tx, 0, s); Append(fn, s);
           IF lasttiledir<>tx THEN                       (* make dir once *)
             ok:=CreateDir(fn, DIRPERM);
             lasttiledir:=tx;
           END;
           Append(fn, DIRSEP);
           IntToStr(ty, 0, s); Append(fn, s); Append(fn, ".png");
   WrStrLn(fn);
           ret:=wrpng(fn, pngdepth, colour1, x, y0, TILESIZE, TILESIZE,
                scale);
           INC(tx);
           INC(x, TILESIZE);
         UNTIL x>xsize/scale;
         INC(ty);
         INC(y0, TILESIZE);
       UNTIL y0>ysize/scale;
   
       DEC(zoom);
       INC(scale, scale);
     UNTIL zoom<tozoom;
   */
   X2C_DYNALLOCATE((char **) &tileimg,1u,(tmp[0] = 256U,tmp[1] = 256U,
                tmp),2u);
   scale = 1L;
   do {
      mercator(mpos.long0, mpos.lat, zoom, &tx, &ty0, &xr, &yr);
      x0 = -(int32_t)X2C_TRUNCI(xr+0.5f,X2C_min_longint,X2C_max_longint);
      y00 = (int32_t)X2C_TRUNCI(yr+0.5f,X2C_min_longint,X2C_max_longint);
      do {
         ty = ty0;
         y = -y00;
         do {
            aprsstr_Assign(fn, 256ul, osmdir, 1024ul);
            aprsstr_Append(fn, 256ul, "/", 2ul);
            aprsstr_IntToStr(zoom, 0UL, s, 256ul);
            aprsstr_Append(fn, 256ul, s, 256ul);
            ok0 = osi_CreateDir(fn, 256ul, 493UL);
            aprsstr_Append(fn, 256ul, "/", 2ul);
            aprsstr_IntToStr(tx, 0UL, s, 256ul);
            aprsstr_Append(fn, 256ul, s, 256ul);
            ok0 = osi_CreateDir(fn, 256ul, 493UL);
            aprsstr_Append(fn, 256ul, "/", 2ul);
            aprsstr_IntToStr(ty, 0UL, s, 256ul);
            aprsstr_Append(fn, 256ul, s, 256ul);
            aprsstr_Append(fn, 256ul, ".png", 5ul);
            if (verb) {
               osi_Werr(fn, 256ul);
               osi_Werr("\012", 2ul);
            }
            ret0 = wrpng(fn, 256ul, pngdepth, x0, y, 256L, 256L, scale);
            ++ty;
            y += 256L;
         } while (y<=ysize/scale);
         ++tx;
         x0 += 256L;
      } while (x0<=xsize/scale);
      --zoom;
      scale += scale;
   } while (zoom>=tozoom0);
   X2C_DYNDEALLOCATE((char **) &tileimg);
   tileimg = 0;
} /* end wrtiles() */


static uint32_t lim(uint32_t n, uint32_t d, uint32_t l)
{
   n = n/d;
   if (n>l) return l;
   return n;
} /* end lim() */


static void genpalette(void)
{
   uint32_t m;
   uint32_t i;
   if (((pngdepth!=1L && pngdepth!=2L) && pngdepth!=4L) && pngdepth!=8L) {
      osi_Werr("png depth 1, 2, 4 or 8 needed", 30ul);
   }
   makegammatab();
   if (antc<0L) {
      for (i = 0UL; i<=255UL; i++) {
         transpaency[i] = (uint8_t)colour1[3U];
         if (pngdepth==1L) {
            clut[i] = (uint8_t)lim(i+100UL, 128UL, 1UL);
            palette[0U].r = 0U;
            palette[0U].g = 0U;
            palette[0U].b = 0U;
            palette[1U].r = (uint8_t)colour1[0U];
            palette[1U].g = (uint8_t)colour1[1U];
            palette[1U].b = (uint8_t)colour1[2U];
         }
         else if (pngdepth==2L) {
            if (draft==0L) clut[i] = (uint8_t)lim(i+60UL, 64UL, 3UL);
            else clut[i] = (uint8_t)lim(i+32UL, 64UL, 3UL);
            if (i<4UL) {
               palette[i].r = (uint8_t)(((int32_t)
                gammatab[i*85UL]*colour1[0U])/256L);
               palette[i].g = (uint8_t)(((int32_t)
                gammatab[i*85UL]*colour1[1U])/256L);
               palette[i].b = (uint8_t)(((int32_t)
                gammatab[i*85UL]*colour1[2U])/256L);
            }
         }
         else if (pngdepth==4L) {
            clut[i] = (uint8_t)lim(i+8UL, 16UL, 15UL);
            if (i<16UL) {
               palette[i].r = (uint8_t)(((int32_t)
                gammatab[i*16UL]*colour1[0U])/256L);
               palette[i].g = (uint8_t)(((int32_t)
                gammatab[i*16UL]*colour1[1U])/256L);
               palette[i].b = (uint8_t)(((int32_t)
                gammatab[i*16UL]*colour1[2U])/256L);
            }
         }
         else {
            /* pngdepth 8 */
            clut[i] = (uint8_t)i;
            palette[i].r = (uint8_t)(((int32_t)gammatab[i]*colour1[0U])
                /256L);
            palette[i].g = (uint8_t)(((int32_t)gammatab[i]*colour1[1U])
                /256L);
            palette[i].b = (uint8_t)(((int32_t)gammatab[i]*colour1[2U])
                /256L);
         }
      } /* end for */
   }
   else {
      if (pngdepth<2L) osi_Werr("2 antennas need png depth 2, 4 or 8", 36ul);
      for (i = 0UL; i<=65535UL; i++) {
         if (pngdepth==2L) {
            clut[i] = (uint8_t)(2UL*lim(i/256UL+100UL, 128UL,
                1UL)+lim((i&255UL)+100UL, 128UL, 1UL));
            palette[0U].r = 0U;
            palette[0U].g = 0U;
            palette[0U].b = 0U;
            palette[1U].r = (uint8_t)colour2[0U];
            palette[1U].g = (uint8_t)colour2[1U];
            palette[1U].b = (uint8_t)colour2[2U];
            transpaency[1U] = (uint8_t)colour2[3U];
            palette[2U].r = (uint8_t)colour3[0U];
            palette[2U].g = (uint8_t)colour3[1U];
            palette[2U].b = (uint8_t)colour3[2U];
            transpaency[2U] = (uint8_t)colour3[3U];
            palette[3U].r = (uint8_t)colour1[0U];
            palette[3U].g = (uint8_t)colour1[1U];
            palette[3U].b = (uint8_t)colour1[2U];
            transpaency[3U] = (uint8_t)colour1[3U];
         }
         else if (pngdepth==4L) {
            if (i<256UL) clut[i] = (uint8_t)lim(i+25UL, 43UL, 5UL);
            else if ((i&255UL)==0UL) {
               clut[i] = (uint8_t)(6UL+lim(i/256UL+25UL, 43UL, 5UL));
            }
            else clut[i] = (uint8_t)(12UL+(i/256UL+(i&255UL))/171UL);
            /*      ELSE clut[i]:=10+(((i DIV 256)*(i MOD 256))+7224)
                DIV 14450; */
            if (i<6UL) {
               palette[i].r = (uint8_t)(((int32_t)
                gammatab[i*42UL]*colour2[0U])/256L);
               palette[i].g = (uint8_t)(((int32_t)
                gammatab[i*42UL]*colour2[1U])/256L);
               palette[i].b = (uint8_t)(((int32_t)
                gammatab[i*42UL]*colour2[2U])/256L);
               transpaency[i] = (uint8_t)colour2[3U];
               palette[i+6UL].r = (uint8_t)(((int32_t)
                gammatab[i*42UL]*colour3[0U])/256L);
               palette[i+6UL].g = (uint8_t)(((int32_t)
                gammatab[i*42UL]*colour3[1U])/256L);
               palette[i+6UL].b = (uint8_t)(((int32_t)
                gammatab[i*42UL]*colour3[2U])/256L);
               transpaency[i+6UL] = (uint8_t)colour3[3U];
               palette[i+12UL].r = (uint8_t)(((int32_t)
                gammatab[215UL+i*8UL]*colour1[0U])/256L);
               palette[i+12UL].g = (uint8_t)(((int32_t)
                gammatab[215UL+i*8UL]*colour1[1U])/256L);
               palette[i+12UL].b = (uint8_t)(((int32_t)
                gammatab[215UL+i*8UL]*colour1[2U])/256L);
               transpaency[i+12UL] = (uint8_t)colour1[3U];
            }
         }
         else {
            /* pngdepth 8 */
            m = 100UL;
            if (i/256UL==0UL) clut[i] = (uint8_t)((i*100UL)/256UL);
            else if ((i&255UL)==0UL) {
               clut[i] = (uint8_t)(100UL+((i/256UL)*100UL)/256UL);
            }
            else {
               clut[i] = (uint8_t)(200UL+((i/256UL+(i&255UL))*55UL)/512UL);
            }
            if (i<100UL) {
               palette[i].r = (uint8_t)(((int32_t)gammatab[(i*255UL)
                /100UL]*colour2[0U])/256L);
               palette[i].g = (uint8_t)(((int32_t)gammatab[(i*255UL)
                /100UL]*colour2[1U])/256L);
               palette[i].b = (uint8_t)(((int32_t)gammatab[(i*255UL)
                /100UL]*colour2[2U])/256L);
               transpaency[i] = (uint8_t)colour2[3U];
               palette[i+100UL].r = (uint8_t)(((int32_t)
                gammatab[(i*255UL)/100UL]*colour3[0U])/256L);
               palette[i+100UL].g = (uint8_t)(((int32_t)
                gammatab[(i*255UL)/100UL]*colour3[1U])/256L);
               palette[i+100UL].b = (uint8_t)(((int32_t)
                gammatab[(i*255UL)/100UL]*colour3[2U])/256L);
               transpaency[i+100UL] = (uint8_t)colour3[3U];
               if (i<56UL) {
                  palette[i+200UL].r = (uint8_t)(((int32_t)
                gammatab[192UL+(i*63UL)/56UL]*colour1[0U])/256L);
                  palette[i+200UL].g = (uint8_t)(((int32_t)
                gammatab[192UL+(i*63UL)/56UL]*colour1[1U])/256L);
                  palette[i+200UL].b = (uint8_t)(((int32_t)
                gammatab[192UL+(i*63UL)/56UL]*colour1[2U])/256L);
                  transpaency[i+200UL] = (uint8_t)colour1[3U];
               }
            }
         }
      } /* end for */
   }
   transpaency[0U] = (uint8_t)colour1[4U];
} /* end genpalette() */


static void joincolours(pIMAGE image0, pIMAGE image20)
{
   uint32_t y;
   uint32_t x;
   uint32_t tmp;
   uint32_t tmp0;
   if (image20==0) {
      tmp = image0->Len1-1;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         tmp0 = image0->Len0-1;
         x = 0UL;
         if (x<=tmp0) for (;; x++) {
            image0->Adr[(y)*image0->Len0+x] = clut[image0->Adr[(y)
                *image0->Len0+x]];
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
   }
   else {
      tmp = image0->Len1-1;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         tmp0 = image0->Len0-1;
         x = 0UL;
         if (x<=tmp0) for (;; x++) {
            image0->Adr[(y)*image0->Len0+x] = clut[(uint32_t)
                image0->Adr[(y)*image0->Len0+x]+(uint32_t)image20->Adr[(y)
                *image20->Len0+x]*256UL];
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
   }
} /* end joincolours() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[2];
   X2C_BEGIN(&argc,argv,1,4000000l,1500000000l);
   if (sizeof(COLOUR)!=20) X2C_ASSERT(0);
   aprsstr_BEGIN();
   aprspos_BEGIN();
   libsrtm_BEGIN();
   osi_BEGIN();
   imagefn[0] = 0;
   osmdir[0] = 0;
   xsize = 600L;
   ysize = 400L;
   posinval(&posa);
   posinval(&posb);
   anta = 10L;
   antb = 10L;
   antc = -1L;
   refraction = 0.25f;
   igamma = 2.2f;
   libsrtm_srtmmaxmem = 200000000UL;
   initzoom = 13L;
   tozoom = 9L;
   finezoom = 1.0f;
   mappos.long0 = 0.0f;
   mappos.lat = 0.0f;
   mappos2.long0 = 0.0f;
   mappos2.lat = 0.0f;
   contrast = 3L;
   pngdepth = 1L;
   writeempty = 0;
   colour1[0U] = 255L;
   colour1[1U] = 255L;
   colour1[2U] = 255L;
   colour1[3U] = 128L;
   colour1[4U] = 0L;
   colour2[0U] = 255L;
   colour2[1U] = 100L;
   colour2[2U] = 0L;
   colour2[3U] = 128L;
   colour2[4U] = 0L;
   colour3[0U] = 0L;
   colour3[1U] = 100L;
   colour3[2U] = 255L;
   colour3[3U] = 128L;
   colour3[4U] = 0L;
   verb = 0;
   draft = 2L;
   srtminterpol = 1;
   Parms();
   lasttilezoom = -1L;
   lasttiledir = -1L;
   tileimg = 0;
   mercator(mappos.long0, mappos.lat, initzoom, &inittilex, &inittiley,
                &shiftx, &shifty);
   if (posa.lat==0.0f && posa.long0==0.0f) Error("need Position A", 16ul);
   if (mappos2.long0!=0.0f || mappos2.lat!=0.0f) {
      /* 2 corners defined */
      if (mapxy(mappos2, &xs2, &ys2)<0L) {
         Error("right down map position outsize image x y limits", 49ul);
      }
      xsize = (int32_t)X2C_TRUNCI(xs2,X2C_min_longint,X2C_max_longint);
      ysize -= (int32_t)X2C_TRUNCI(ys2,X2C_min_longint,X2C_max_longint);
      if (verb) {
         osi_Werr("x=", 3ul);
         Werrint(xsize);
         osi_Werr(" y=", 4ul);
         Werrint(ysize);
         osi_Werr("\012", 2ul);
      }
   }
   if (imagefn[0U]==0 && osmdir[0U]==0) {
      Error("need Image Filename or Tile Path", 33ul);
   }
   if (libsrtm_srtmdir[0U]==0) Error("need SRTM Directory Path", 25ul);
   if (xsize<32L) Error("xsize too less", 15ul);
   if (ysize<32L) Error("ysize too less", 15ul);
   if (antc>=0L && !posvalid(posb)) Error("-C needs -b", 12ul);
   genpalette();
   image2 = 0;
   X2C_DYNALLOCATE((char **) &image,1u,(tmp[0] = (size_t)ysize,
                tmp[1] = (size_t)xsize,tmp),2u);
   if (image==0) Error("out of memory", 14ul);
   if (antc>=0L) {
      if (verb) osi_Werr("antenna 1\012", 11ul);
      ret = Radiorange(image, posa, anta, antc, (uint32_t)contrast,
                (uint32_t)draft, refraction);
      if (ret==-1L) Error("no altitude at antenne A", 25ul);
      X2C_DYNALLOCATE((char **) &image2,1u,(tmp[0] = (size_t)ysize,
                tmp[1] = (size_t)xsize,tmp),2u);
      if (image2==0) Error("out of memory", 14ul);
      if (verb) osi_Werr("antenna 2\012", 11ul);
      ret = Radiorange(image2, posb, antb, antc, (uint32_t)contrast,
                (uint32_t)draft, refraction);
      if (ret==-1L) Error("no altitude at antenne B", 25ul);
      if (verb) osi_Werr("join images\012", 13ul);
   }
   else {
      if (pngdepth>=4L && draft>=2L) {
         ++draft;
      }
      ret = Radiorange(image, posa, anta, antb, (uint32_t)contrast,
                (uint32_t)draft, refraction);
      if (ret==-1L) Error("no altitude at antenne A", 25ul);
   }
   libsrtm_closesrtmfile(); /* free srtm cache */
   joincolours(image, image2);
   if (image2) X2C_DYNDEALLOCATE((char **) &image2);
   if (verb) osi_Werr("make png\012", 10ul);
   if (osmdir[0U]) wrtiles(mappos, initzoom, tozoom);
   if (imagefn[0U]) {
      ret = wrpng(imagefn, 1024ul, pngdepth, 0L, 0L, xsize, ysize, 1L);
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
