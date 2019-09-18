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

#define radiorange_GAINS 360

#define radiorange_ROTATECHAR "v"

#define radiorange_WOODSHADOW 255


struct IMAGE {
   uint8_t * Adr;
   size_t Len0;
   size_t Size1;
   size_t Len1;
};

typedef struct IMAGE * pIMAGE;

typedef int32_t COLOUR[5];

struct _0;


struct _0 {
   uint32_t r;
   uint32_t g;
   uint32_t b;
   uint32_t t;
};

typedef struct _0 COLTAB[256];

static pIMAGE image;

static pIMAGE image2;

static pIMAGE tileimg;

static char imagefn[1024];

static char osmdir[1024];

static char percentfn[1024];

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

static int32_t lastpercent;

static int32_t percenttyp;

static int32_t woodshadowbri;

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

static float woodsize;

static uint8_t gammatab[256];

static struct PNGPALETTE palette[256];

static uint8_t transpaency[256];

static uint8_t clut[65536];

static char writeempty;

static char verb;

static char srtminterpol;

struct _1;


struct _1 {
   char on;
   char cacheok;
   char rotate;
   char rotate2;
   float lastele;
   float mindB;
   float maxdB;
   float dBmul;
   float logmul;
   float lpower;
   float mhz;
   float azimuth;
   float elevation;
   float azimuth2;
   float elevation2;
   float gainA;
   float gainB;
   float cachegain;
   float hgain[360];
   float vgain[360];
   char coltabfn[1024];
   char gainfn[1024];
   char gainfn2[1024];
   uint32_t colours;
};

static struct _1 diagram;


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

#define radiorange_TENOVERLOGTEN 4.342944819


static void readantenna(const char fn[], uint32_t fn_len,
                float gain)
{
   int32_t fd;
   char fb[10001];
   char b[101];
   int32_t n;
   int32_t p;
   int32_t i;
   int32_t len;
   float r;
   float round;
   if (verb) {
      Werrint((int32_t)diagram.colours);
      osi_Werr(" colours\012", 10ul);
   }
   if (diagram.azimuth<0.0f) {
      Error("need Antenne Azimuth and Elevation", 35ul);
   }
   round = X2C_DIVR(0.5f,(float)(diagram.colours+1UL));
   diagram.dBmul = X2C_DIVR(1.0f-round,diagram.maxdB-diagram.mindB);
   diagram.logmul = 4.342944819f*diagram.dBmul;
   diagram.lpower = (float)((((double)(gain-(-27.8f))
                -log((double)diagram.mhz)*4.342944819*2.0)
                -(double)diagram.mindB)*(double)
                diagram.dBmul+(double)round);
   /* -60db till meters^2 not km distance */
   fd = osi_OpenRead(fn, fn_len);
   len = 0L;
   if (fd<0L) {
      osi_Werr("Antenna Gain file [", 20ul);
      osi_Werr(fn, fn_len);
      osi_Werr("] not found\012", 13ul);
   }
   if (fd>=0L) {
      len = osi_RdBin(fd, (char *)fb, 10001u/1u, 10001UL);
      if (len<=0L) {
         osi_Werr("Antenna Gain file [", 20ul);
         osi_Werr(fn, fn_len);
         osi_Werr("] not readable\012", 16ul);
      }
   }
   p = 0L;
   n = 0L;
   memset((char *)diagram.hgain,(char)0,sizeof(float [360]));
   memset((char *)diagram.vgain,(char)0,sizeof(float [360]));
   do {
      i = 0L;
      while (p<len && (uint8_t)fb[p]<=' ') ++p;
      while ((p<len && i<100L) && (uint8_t)fb[p]>' ') {
         b[i] = fb[p];
         ++i;
         ++p;
      }
      b[i] = 0;
      if (i>0L && aprsstr_StrToFix(&r, b, 101ul)) {
         r = (float)fabs(r);
         if (r>200.0f) r = 200.0f;
         if (diagram.rotate) {
            if (n<360L) diagram.vgain[(n+90L)%360L] = r;
            else diagram.hgain[359L-(n+630L)%360L] = r;
         }
         else if (n<360L) diagram.hgain[359L-n] = r;
         else diagram.vgain[(899L-n)%360L] = r;
      }
      ++n;
   } while (n<720L);
   diagram.cacheok = 0;
} /* end readantenna() */

#define radiorange_PERR "-P <MHz> <mindBm> <maxdBm> <RGBTfile>|-"

#define radiorange_AERR "<m> <azimuth 0..360> [v]<elevation -90..90> <gain su\
m dBm> <RGBTfilename>"


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
         else if (h[1U]=='X') {
            osi_NextArg(percentfn, 1024ul);
            if (percentfn[0U]==0 || percentfn[0U]=='-') {
               Error("-X <progress-percent-filename>", 31ul);
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
            if (diagram.on) {
               osi_NextArg(h, 1024ul);
               if ((!aprsstr_StrToFix(&diagram.azimuth, h,
                1024ul) || diagram.azimuth<0.0f) || diagram.azimuth>360.0f) {
                  Error("-A<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
               osi_NextArg(h, 1024ul);
               if (h[0U]=='v') {
                  aprsstr_Delstr(h, 1024ul, 0UL, 1UL);
                  diagram.rotate = 1;
               }
               if (!aprsstr_StrToFix(&diagram.elevation, h,
                1024ul) || (float)fabs(diagram.elevation)>90.0f) {
                  Error("-A<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&diagram.gainA, h,
                1024ul) || (float)fabs(diagram.gainA)>200.0f) {
                  Error("-A<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
               osi_NextArg(diagram.gainfn, 1024ul);
               if (diagram.gainfn[0U]=='-') {
                  Error("-A<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
            }
         }
         else if (h[1U]=='B') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &antb) || antb<0L) || antb>10000L) {
               Error("-B <meter> (0..10000)", 22ul);
            }
            if (diagram.on) {
               osi_NextArg(h, 1024ul);
               if ((!aprsstr_StrToFix(&diagram.azimuth2, h,
                1024ul) || diagram.azimuth2<0.0f) || diagram.azimuth2>360.0f)
                 {
                  Error("-B<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
               osi_NextArg(h, 1024ul);
               if (h[0U]=='v') {
                  aprsstr_Delstr(h, 1024ul, 0UL, 1UL);
                  diagram.rotate2 = 1;
               }
               if (!aprsstr_StrToFix(&diagram.elevation2, h,
                1024ul) || (float)fabs(diagram.elevation2)>90.0f) {
                  Error("-B<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&diagram.gainB, h,
                1024ul) || (float)fabs(diagram.gainB)>200.0f) {
                  Error("-B<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
               osi_NextArg(diagram.gainfn2, 1024ul);
               if (diagram.gainfn2[0U]=='-') {
                  Error("-B<m> <azimuth 0..360> [v]<elevation -90..90> <gain \
sum dBm> <RGBTfilename>", 76ul);
               }
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
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&woodsize, h,
                1024ul) || woodsize<0.0f) || woodsize>100.0f) {
               Error("-t <treesize[0..100]> <brightness>", 35ul);
            }
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &woodshadowbri) || woodshadowbri<0L) || woodshadowbri>255L) {
               Error("-t <treesize> <brightness[0..255]>", 35ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&igamma, h,
                1024ul) || igamma<0.1f) || igamma>10.0f) {
               Error("-g <gamma> [0.1..10.0]", 23ul);
            }
         }
         else if (h[1U]=='P') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&diagram.mhz, h,
                1024ul) || diagram.mhz<0.001f) {
               Error("-P <MHz> <mindBm> <maxdBm> <RGBTfile>|-", 40ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&diagram.mindB, h, 1024ul)) {
               Error("-P <MHz> <mindBm> <maxdBm> <RGBTfile>|-", 40ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&diagram.maxdB, h, 1024ul)) {
               Error("-P <MHz> <mindBm> <maxdBm> <RGBTfile>|-", 40ul);
            }
            osi_NextArg(diagram.coltabfn, 1024ul);
            if (diagram.coltabfn[0U]==0 || diagram.coltabfn[0U]
                =='-' && diagram.coltabfn[1U]) {
               Error("-P <MHz> <mindBm> <maxdBm> <RGBTfile>|-", 40ul);
            }
            diagram.on = 1;
            if ((float)fabs(diagram.mindB)>200.0f) {
               diagram.mindB = (-90.0f);
               osi_Werr("dBm > 200dBm ?\012", 16ul);
            }
            if ((float)fabs(diagram.maxdB)>200.0f) {
               diagram.maxdB = (-60.0f);
               osi_Werr("dBm > 200dBm ?\012", 16ul);
            }
            if (diagram.maxdB<=diagram.mindB+0.001f) {
               diagram.mindB = diagram.maxdB-30.0f;
               osi_Werr("from >= to dBm ?\012", 18ul);
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
               osi_WrStrLn(" -A <m> <azimuth> [v]<elevation> <gain sum dBm> <\
gainfilename> if -P defined before", 84ul);
               osi_WrStrLn("                                   v before eleva\
tion for H/V polarisation swap", 80ul);
               osi_WrStrLn("                                   sum of tx-powe\
r, tx1 and rx antenna gain dBm", 80ul);
               osi_WrStrLn(" -a <lat> <long> | [locator]       Position A lat\
 long (degrees) or qth locator", 80ul);
               osi_WrStrLn(" -B <m>                            Antenna B over\
 ground [m] (10)", 66ul);
               osi_WrStrLn(" -B <m> <azimuth> [v]<elevation> <gain  sum dBm> \
<gainfilename> if -P defined before", 85ul);
               osi_WrStrLn("                                   v before eleva\
tion for H/V polarisation swap", 80ul);
               osi_WrStrLn("                                   sum of tx-powe\
r, tx2 and rx antenna gain dBm", 80ul);
               osi_WrStrLn("                                   set -B <m> bef\
ore -P for no antenna gains", 77ul);
               osi_WrStrLn(" -b <lat> <long> | [locator]       Position B lat\
 long (degrees) or qth locator", 80ul);
               osi_WrStrLn(" -C <m>                            0 for ground e\
cho or Repeater Antenna over ground [m] (0)", 93ul);
               osi_WrStrLn(" -c <contrast>                     0 for hard 30 \
for smooth area margin (3)", 76ul);
               osi_WrStrLn(" -d <depth>                        png palette bi\
ts/pixel 1,2,4,8 (1) or (2) if 2 antennas", 91ul);
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
               osi_WrStrLn(" -P <MHz> <min dBm> <max dBm> <RGBfile>|-",
                42ul);
               osi_WrStrLn("                                   show dBm range\
 colours min..max eg. -90..-60", 80ul);
               osi_WrStrLn("                                   RGBTfile or - \
for standard colours (1 Antenna)", 82ul);
               osi_WrStrLn("                                   RGBTfile: line\
s of <r> <g> <b> <transparency>", 81ul);
               osi_WrStrLn("                                     line 1: no s\
ight, last line: maximum gain", 79ul);
               osi_WrStrLn("                                   2 Antennas: li\
ne 1: no sight, line 2: min Ant A", 83ul);
               osi_WrStrLn("                                     line n: max \
Ant A, line n+1: Both have sight", 82ul);
               osi_WrStrLn("                                     line n+2: mi\
n Ant B, last line: max Ant B", 79ul);
               osi_WrStrLn("                                     use same num\
ber of colours per antenna", 76ul);
               osi_WrStrLn("                                     set -d <dept\
h> fitting to colours", 71ul);
               osi_WrStrLn("                                     -U -V -W -g \
ignored", 57ul);
               osi_WrStrLn(" -p <path>                         srtm directory\
 path", 55ul);
               osi_WrStrLn(" -Q                                srtm interpola\
tion off (on)", 63ul);
               osi_WrStrLn(" -q <quality>                      0=fast 3=best \
(2)", 53ul);
               osi_WrStrLn(" -R <refraction>                   0.0(vacuum), 1\
.0(earth is a disk) (0.25)", 76ul);
               osi_WrStrLn(" -t <treesize> <brightness>        tree size (m),\
 decreased to 0 from 1000-2000m NN (0)", 88ul);
               osi_WrStrLn("                                   brightness/pat\
tern wood shadow 0..255, 0 for full shadow", 92ul);
               osi_WrStrLn("                                   pattern used i\
f 2 tx or 1 bit png", 69ul);
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
               osi_WrStrLn(" -X <filename>                     write % of com\
pletion in this file", 70ul);
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
   /*  IF diagram.on THEN woodshadowbri:=0 END; */
   if (woodsize==0.0f) woodshadowbri = 0L;
} /* end Parms() */


static char TwoTx(void)
{
   return antc>=0L;
} /* end TwoTx() */


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


static uint8_t joincolour(uint32_t y, uint32_t x, uint32_t scale)
{
   uint32_t z1;
   uint32_t z0;
   char pattern;
   /*  IF y<HIGH(image^) THEN INC(y) END; */
   if (x>=image->Len0-1) return 0U;
   z0 = (uint32_t)image->Adr[(y)*image->Len0+x];
   if (woodshadowbri>0L) {
      pattern = (x/scale+(y/scale)*3UL)%(uint32_t)woodshadowbri!=0UL;
      if ((x&1)) {
         if (z0==255UL) {
            if (pattern) z0 = 0UL;
            else z0 = (uint32_t)image->Adr[(y)*image->Len0+(x-1UL)];
         }
      }
      else if (pattern && image->Adr[(y)*image->Len0+(x+1UL)]==255U) {
         z0 = 0UL;
      }
   }
   if (image2==0) return clut[z0];
   z1 = (uint32_t)image2->Adr[(y)*image2->Len0+x];
   if (woodshadowbri>0L) {
      if ((x&1)) {
         if (z1==255UL) {
            if (pattern) z1 = 0UL;
            else z1 = (uint32_t)image2->Adr[(y)*image2->Len0+(x-1UL)];
         }
      }
      else if (pattern && image2->Adr[(y)*image2->Len0+(x+1UL)]==255U) {
         z1 = 0UL;
      }
   }
   return clut[z0+z1*256UL];
} /* end joincolour() */


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
               if (tileimg==0) {
                  cc = (uint32_t)image->Adr[(yi)*image->Len0+is];
               }
               else {
                  cc = (uint32_t)joincolour((uint32_t)yi, (uint32_t)is,
                 (uint32_t)scale);
               }
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


static float antdiagram(float azi, float ele)
/* interpolate gain table azi 0..359, ele -90..90 */
{
   uint32_t a;
   float azv;
   float e;
   float f;
   float gv;
   float gvh;
   float gh;
   if (ele>90.0f) {
      ele = 180.0f-ele;
      azi = azi+180.0f;
   }
   else if (ele<(-90.0f)) {
      ele = 180.0f+ele;
      azi = azi+180.0f;
   }
   if (azi>=360.0f) azi = azi-360.0f;
   a = (uint32_t)X2C_TRUNCC(azi,0UL,X2C_max_longcard);
   f = azi-(float)a;
   gh = diagram.hgain[a]*(1.0f-f)+diagram.hgain[(a+1UL)%360UL]*f;
                /* interpolated hor */
   azv = (float)fabs(azi*5.5555555555556E-3f-1.0f);
                /* front/back weight */
   e = ele+90.0f;
   a = (uint32_t)X2C_TRUNCC(e,0UL,X2C_max_longcard);
   f = e-(float)a;
   gv = diagram.vgain[a]*(1.0f-f)+diagram.vgain[a+1UL]*f;
                /* interpolate vert front */
   gvh = (diagram.vgain[(360UL-a)%360UL]*(1.0f-f)+diagram.vgain[359UL-a]*f)
                -diagram.vgain[270U]; /* vert back */
   gv = gv*azv+gvh*(1.0f-azv); /* front/back wighted vert */
   gh = gh*(1.0f-(float)fabs(ele*0.0f)); /* vert wighted hor */
   return (float)sqrt((double)(gh*gh+gv*gv));
/* dB */
/* geometric hor vert sum */
} /* end antdiagram() */


static float antgain(float azi, float alt, float oomaxdist,
                float * ddist)
{
   float dist;
   float ele;
   float db;
   dist = X2C_DIVR(*ddist,oomaxdist);
   ele = X2C_DIVR(alt,dist);
   if (diagram.cacheok && (float)fabs(ele-diagram.lastele)>(float)
                fabs(ele)*0.01f) diagram.cacheok = 0;
   if (!diagram.cacheok) {
      diagram.lastele = ele;
      azi = diagram.azimuth-azi;
      if (azi<0.0f) azi = azi+360.0f;
      ele = (float)(atan((double)ele)*5.729577951472E+1);
      ele = (float)((double)ele-(double)
                diagram.elevation*cos((double)(azi*1.7453292519444E-2f)
                ));
      diagram.cachegain = antdiagram(azi, ele)*diagram.dBmul;
      diagram.cacheok = 1;
   }
   db = (float)((double)diagram.lpower-log((double)
                (dist*dist+alt*alt))*(double)diagram.logmul);
                /* with 0dBi antenna */
   if (db<0.0f) {
      *ddist = 1000.0f; /* abort sigthline on distance loss */
      return 0.0f;
   }
   db = db-diagram.cachegain; /* apply antenna diagram */
   if (db<0.0f) db = 0.0f;
   else if (db>1.0f) db = 1.0f;
   return db;
} /* end antgain() */


static void postfilter(pIMAGE image0)
/* set missing pixels with median of neighbours */
{
   uint32_t y;
   uint32_t x;
   uint16_t d;
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
            if (c==255U) c = 0U;
            d = (uint16_t)image0->Adr[(y+1UL)*image0->Len0+x];
            if (d<255U) c += d;
            d = (uint16_t)image0->Adr[(y)*image0->Len0+(x-1UL)];
            if (d<255U) c += d;
            d = (uint16_t)image0->Adr[(y-1UL)*image0->Len0+x];
            if (d<255U) c += d;
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


static void percent(float t, float tmax, uint32_t frame)
{
   float p;
   int32_t i;
   int32_t fd;
   char s[11];
   p = X2C_DIVR(t,tmax)+(float)frame;
   if (percenttyp==0L) p = p*25.0f;
   else {
      p = p*12.5f; /* two images */
      if (percenttyp==2L) p = p+50.0f;
   }
   i = (int32_t)truncc(p);
   if (i!=lastpercent) {
      lastpercent = i;
      fd = osi_OpenWrite(percentfn, 1024ul);
      if (fd>=0L) {
         aprsstr_IntToStr(lastpercent, 0UL, s, 11ul);
         osi_WrBin(fd, (char *)s, 11u/1u, aprsstr_Length(s, 11ul));
         osic_Close(fd);
      }
   }
/*    WrInt(lastpercent, 3); WrStrLn("%"); */
} /* end percent() */

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
   uint8_t attr;
   uint8_t bri;
   int32_t void0;
   int32_t nn;
   float hsw;
   float hwood;
   float raisw;
   float antdiff;
   float azi;
   float hgnd;
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
   char woodshadow;
   nn = (int32_t)X2C_TRUNCI(libsrtm_getsrtmlong((double)txpos.lat,
                (double)txpos.long0, 0UL, 0, &resoltx, &attr, 0),
                X2C_min_longint,X2C_max_longint);
                /* altitude of tx and map resolution in m here */
   if (nn>=10000L) {
      libsrtm_closesrtmfile();
      return -1L;
   }
   atx = (float)(nn+ant1); /* ant1 over NN */
   wgs84s(txpos.lat, txpos.long0, atx*0.001f, &x0, &y00, &z0);
   arx = (float)ant2; /* ant2 over ground */
   antdiff = arx-atx;
   xi = 0.0f;
   yi = 0.0f;
   nn = mapxy(txpos, &xtx, &ytx);
   frame = 0UL;
   mperpix = meterperpix(image0); /* meter per pixel */
   if (mperpix<1.0f) {
      libsrtm_closesrtmfile();
      return -2L;
   }
   /*  IF woodshadowbri=0 THEN woodshadowlum:=0.0; */
   /*  ELSIF TwoTx() OR (pngdepth=1)
                THEN woodshadowlum:=64.0 ELSE woodshadowlum:=FLOAT(woodshadowbri)
                 END; */
   /*  ELSE woodshadowlum:=FLOAT(WOODSHADOW) END; */
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
         if (diagram.on) {
            azi = aprspos_azimuth(txpos, pos); /* for antenna diagram */
            diagram.cacheok = 0; /* so no need to check azi too */
         }
         dx = x1-x0;
         dy = y1-y00;
         dz = z1-z0;
         oodist = dx*dx+dy*dy+dz*dz;
         if (oodist>1.E-6f) {
            dm = oodist*refr; /* oodist full dist^2 */
            oodist = (float)(X2C_DIVL(0.001,sqrt((double)oodist)));
                /* 1/sight line length in m */
            pixstep = 0.0f;
            resol = resoltx;
            d = resol*oodist;
            rais = (-1.E+9f); /* initial sightline angle **/
            raisw = (-1.E+9f); /* initial sightline angle **/
            dnext = 0.0f;
            dpnext = 0.0f;
            do {
               /* do one sight line */
               if (d>=dnext) {
                  /* next fixpoint in interpolate sight line pos */
                  dnext = d+6000.0f*oodist;
                  if (dnext>1.0f) {
                     dnext = 1.0f;
                  }
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
               alt = (alt0+dalt*d)-d*d*dm;
                /* -dist(km)^2 * refrac * 0.0785 */
               hgnd = libsrtm_getsrtmlong((double)(pos0.lat+dpos.lat*d)
                , (double)(pos0.long0+dpos.long0*d),
                (uint32_t)X2C_TRUNCC((float)fabs(sight)+1.0f,0UL,
                X2C_max_longcard)*qual, 0, &resol, &attr, 0);
                /* ground over NN in m */
               if (hgnd<10000.0f) {
                  /* srtm valid */
                  /*>wood */
                  woodshadow = 0;
                  if (woodsize!=0.0f) {
                     if (attr==2U) {
                        /* we have wood */
                        if (hgnd>=1000.0f) {
                           hwood = woodsize*(2000.0f-hgnd)*0.001f;
                           if (hwood<0.0f) hwood = 0.0f;
                        }
                        else hwood = woodsize;
                        hwood = hwood+hgnd;
                     }
                     else hwood = hgnd;
                     hwood = hwood-alt;
                     hsw = raisw*d; /* h sight line m over wood searchpath */
                     if (hwood>hsw) {
                        raisw = X2C_DIVR(hwood,d);
                /* this point has light on wood */
                     }
                     if ((hwood+arx)-hsw<0.0f) woodshadow = 1;
                  }
                  /*>wood */
                  h = hgnd-alt; /* m ground over searchpath */
                  hs = rais*d; /* h sight line m over searchpath */
                  if (h>hs) {
                     rais = X2C_DIVR(h,d);
                /* this point has light on ground */
                  }
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
                           if (diagram.on) {
                              if (lum>255.0f) lum = 255.0f;
                              lum = lum*antgain(azi, hgnd+antdiff, oodist,
                &d);
                           }
                           if (lum>254.99f) bri = 254U;
                           else {
                              bri = (uint8_t)(uint32_t)X2C_TRUNCC(lum,
                0UL,X2C_max_longcard);
                           }
                           if ((woodshadow && woodsize!=0.0f) && bri>0U) {
                              if (woodshadowbri>0L) {
                                 /* prepare pixel pattern */
                                 if ((xp&1)) {
                                    /* mark as wood shadow,
                set every 2nd pixel 255 */
                                    image0->Adr[(yp)*image0->Len0+(xp-1UL)
                ] = bri;
                                    bri = 255U;
                                 }
                                 else if (xp<image0->Len0-1) {
                                    image0->Adr[(yp)*image0->Len0+(xp+1UL)
                ] = 255U;
                                 }
                              }
                              else bri = 0U;
                           }
                           image0->Adr[(yp)*image0->Len0+xp] = bri;
                           if (bri<255U && qualnum<=1UL) {
                              /* fill possibly missing neighbours */
                              bri = (uint8_t)(((uint32_t)bri*6UL)/8UL);
                              setcol(bri,
                &image0->Adr[(yp)*image0->Len0+(xp+1UL)]);
                              setcol(bri,
                &image0->Adr[(yp+1UL)*image0->Len0+xp]);
                              setcol(bri,
                &image0->Adr[(yp+1UL)*image0->Len0+(xp-1UL)]);
                              setcol(bri,
                &image0->Adr[(yp-1UL)*image0->Len0+xp]);
                              if (qualnum==0UL) {
                                 bri = (uint8_t)(((uint32_t)bri*6UL)/8UL)
                ;
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
         percent(xi, (float)(image0->Len0-1), frame);
         if (xi>=(float)(image0->Len0-1)) {
            xi = 0.0f;
            ++frame;
         }
         break;
      case 1UL:
         yi = yi+goonframe(framestep, yi, ytx, xi, xtx);
         percent(yi, (float)(image0->Len1-1), frame);
         if (yi>=(float)(image0->Len1-1)) {
            yi = (float)(image0->Len1-1);
            ++frame;
         }
         break;
      case 2UL:
         xi = xi+goonframe(framestep, xi, xtx, yi, ytx);
         percent(xi, (float)(image0->Len0-1), frame);
         if (xi>=(float)(image0->Len0-1)) {
            xi = (float)(image0->Len0-1);
            ++frame;
         }
         break;
      default:;
         if (yi<0.0f) goto loop_exit;
         percent((float)(image0->Len1-1)-yi, (float)(image0->Len1-1),
                frame);
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


static uint32_t readcolourfile(COLTAB col, const char fn[],
                uint32_t fn_len)
{
   int32_t fd;
   char fb[10001];
   char b[101];
   int32_t j;
   int32_t n;
   int32_t p;
   int32_t i;
   int32_t len;
   int32_t x;
   fd = osi_OpenRead(fn, fn_len);
   if (fd<0L) Error("Colourtable file not found", 27ul);
   len = osi_RdBin(fd, (char *)fb, 10001u/1u, 10001UL);
   if (len<=0L) Error("Colourtable file not readable", 30ul);
   p = 0L;
   n = 0L;
   j = 0L;
   memset((char *)col,(char)0,sizeof(COLTAB));
   for (;;) {
      i = 0L;
      while (p<len && (uint8_t)fb[p]<=' ') ++p;
      while ((p<len && i<100L) && (uint8_t)fb[p]>' ') {
         b[i] = fb[p];
         ++i;
         ++p;
      }
      b[i] = 0;
      if ((n<=255L && i>0L) && aprsstr_StrToInt(b, 101ul, &x)) {
         if (x<0L) x = 0L;
         else if (x>255L) x = 255L;
         if (j==0L) {
            col[n].r = (uint32_t)x;
            ++j;
         }
         else if (j==1L) {
            col[n].g = (uint32_t)x;
            ++j;
         }
         else if (j==2L) {
            col[n].b = (uint32_t)x;
            ++j;
         }
         else {
            col[n].t = (uint32_t)x;
            j = 0L;
            ++n;
         }
      }
      else break;
   }
   return (uint32_t)n;
} /* end readcolourfile() */


static uint32_t lim(uint32_t n, uint32_t d, uint32_t l)
{
   n = n/d;
   if (n>l) return l;
   return n;
} /* end lim() */


static void genpalette(void)
{
   uint32_t cc;
   uint32_t m;
   uint32_t i;
   COLTAB col;
   if (((pngdepth!=1L && pngdepth!=2L) && pngdepth!=4L) && pngdepth!=8L) {
      osi_Werr("png depth 1, 2, 4 or 8 needed\012", 31ul);
   }
   makegammatab();
   if (diagram.on) {
      if ((pngdepth!=2L && pngdepth!=4L) && pngdepth!=8L) {
         osi_Werr("png depth 2, 4 or 8 needed+LF", 30ul);
      }
      if (diagram.coltabfn[0U] && diagram.coltabfn[0U]!='-') {
         /* read colours from file */
         m = readcolourfile(col, diagram.coltabfn, 1024ul);
         if (TwoTx()) m = m/2UL;
         if (m==0UL) Error("no values in colour table", 26ul);
         if (pngdepth==2L && m>4UL || pngdepth==4L && m>16UL) {
            Error("too much colours for specified png size", 40ul);
         }
         for (i = 0UL; i<=255UL; i++) {
            clut[i] = (uint8_t)lim(i, 256UL/m, m-1UL);
            if (i<m) {
               transpaency[i] = (uint8_t)col[i].t;
               palette[i].r = (uint8_t)col[i].r;
               palette[i].g = (uint8_t)col[i].g;
               palette[i].b = (uint8_t)col[i].b;
            }
         } /* end for */
         if (TwoTx()) {
            /* 2 antennas */
            if (((pngdepth<2L || pngdepth==2L && m>2UL)
                || pngdepth==4L && m>8UL) || m>128UL) {
               Error("too much colours for specified png size", 40ul);
            }
            for (i = 0UL; i<=65535UL; i++) {
               clut[i] = clut[i&255UL]; /* ant A values */
            } /* end for */
            for (i = 0UL; i<=65535UL; i++) {
               if (clut[i]==0U) {
                  /* ant A no sight */
                  cc = lim(i, 65536UL/m, m-1UL);
                  if (cc) clut[i] = (uint8_t)(cc+m);
               }
               else if (cc) clut[i] = (uint8_t)m;
               if (i<m) {
                  transpaency[i+m] = (uint8_t)col[i+m].t;
                  palette[i+m].r = (uint8_t)col[i+m].r;
                  palette[i+m].g = (uint8_t)col[i+m].g;
                  palette[i+m].b = (uint8_t)col[i+m].b;
               }
            } /* end for */
         }
      }
      else {
         for (i = 0UL; i<=255UL; i++) {
            transpaency[i] = (uint8_t)colour1[3U];
            if (pngdepth==2L) {
               m = 4UL;
               clut[i] = (uint8_t)lim(i+32UL, 64UL, 3UL);
               if (i<4UL) {
                  palette[i].r = 0U;
                  palette[i].g = 0U;
                  palette[i].b = 0U;
                  if (i==1UL) {
                     palette[i].r = gammatab[255U];
                     palette[i].g = gammatab[0U];
                  }
                  else if (i==2UL) {
                     palette[i].r = gammatab[255U];
                     palette[i].g = gammatab[255U];
                  }
                  else if (i==3UL) {
                     palette[i].r = gammatab[0U];
                     palette[i].g = gammatab[255U];
                  }
               }
            }
            else if (pngdepth==4L) {
               m = 16UL;
               clut[i] = (uint8_t)lim(i, 16UL, 15UL);
               if (i<16UL) {
                  palette[i].r = 0U;
                  palette[i].g = 0U;
                  palette[i].b = 0U;
                  if (i>0UL) {
                     palette[i].r = gammatab[lim(480UL-i*32UL, 1UL, 255UL)];
                     palette[i].g = gammatab[lim(i*32UL, 1UL, 255UL)];
                  }
               }
            }
            else {
               /* pngdepth 8 */
               m = 256UL;
               clut[i] = (uint8_t)i;
               palette[i].r = 0U;
               palette[i].g = 0U;
               palette[i].b = 0U;
               if (i>0UL) {
                  palette[i].r = gammatab[lim(511UL-i*2UL, 1UL, 255UL)];
                  palette[i].g = gammatab[lim(i*2UL, 1UL, 255UL)];
               }
            }
         } /* end for */
      }
      diagram.colours = m;
   }
   else if (antc<0L) {
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
      transpaency[0U] = (uint8_t)colour1[4U];
   }
   else {
      if (pngdepth<2L) {
         osi_Werr("2 antennas need png depth 2, 4 or 8\012", 37ul);
      }
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
      transpaency[0U] = (uint8_t)colour1[4U];
   }
} /* end genpalette() */

/*
PROCEDURE joincolours(image, image2:pIMAGE);
VAR x,y,z0,z1,w:CARDINAL;
    pattern:BOOLEAN;
BEGIN
  w:=woodshadowbri;
  IF image2=NIL THEN
    pattern:=(w>0) & (pngdepth=1);
    FOR y:=0 TO HIGH(image^) DO
      FOR x:=0 TO HIGH(image^[0]) DO
        z0:=image^[y][x];
        IF pattern & (z0<128) & ((x+y*3) MOD w<>0) THEN z0:=0 END;
        image^[y][x]:=clut[z0];
      END;
    END;
  ELSE
    pattern:=w>0;
    FOR y:=0 TO HIGH(image^) DO
      FOR x:=0 TO HIGH(image^[0]) DO
        z0:=image^[y][x];
        z1:=image2^[y][x];
        IF pattern & ((x+y*3) MOD w<>0) THEN
          IF z0<128 THEN z0:=0 END;
          IF z1<128 THEN z1:=0 END;
        END;
        image^[y][x]:=clut[z0+z1*256];
      END;
    END;
  END;
END joincolours;
*/

static void joincolours(void)
{
   uint32_t y;
   uint32_t x;
   uint8_t b1;
   uint8_t b0;
   uint32_t tmp;
   uint32_t tmp0;
   tmp = image->Len1-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      b1 = 0U;
      tmp0 = image->Len0-1;
      x = 1UL;
      if (x<=tmp0) for (;; x++) {
         b0 = joincolour(y, x, 1UL);
         image->Adr[(y)*image->Len0+(x-1UL)] = b1;
         b1 = b0; /* so not overwrite woodshadow pixel pair */
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
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
   percentfn[0] = 0;
   xsize = 600L;
   ysize = 400L;
   posinval(&posa);
   posinval(&posb);
   anta = 10L;
   antb = 10L;
   antc = -1L;
   refraction = 0.25f;
   igamma = 2.2f;
   woodsize = 0.0f;
   woodshadowbri = 0L;
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
   memset((char *) &diagram,(char)0,sizeof(struct _1));
   diagram.maxdB = (-60.0f);
   diagram.mindB = (-90.0f);
   diagram.mhz = 2400.0f;
   diagram.azimuth = (-1.0f);
   diagram.azimuth2 = (-1.0f);
   Parms();
   if (TwoTx() && pngdepth<2L) pngdepth = 2L;
   lastpercent = 0L;
   percenttyp = 0L;
   genpalette();
   if (diagram.on) readantenna(diagram.gainfn, 1024ul, diagram.gainA);
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
   if (TwoTx() && !posvalid(posb)) Error("-C needs -b", 12ul);
   image2 = 0;
   X2C_DYNALLOCATE((char **) &image,1u,(tmp[0] = (size_t)ysize,
                tmp[1] = (size_t)xsize,tmp),2u);
   if (image==0) Error("out of memory", 14ul);
   if (TwoTx()) {
      percenttyp = 1L;
      if (verb) osi_Werr("antenna 1\012", 11ul);
      ret = Radiorange(image, posa, anta, antc, (uint32_t)contrast,
                (uint32_t)draft, refraction);
      if (ret==-1L) Error("no altitude at antenne A", 25ul);
      X2C_DYNALLOCATE((char **) &image2,1u,(tmp[0] = (size_t)ysize,
                tmp[1] = (size_t)xsize,tmp),2u);
      percenttyp = 2L;
      if (image2==0) Error("out of memory", 14ul);
      if (verb) osi_Werr("antenna 2\012", 11ul);
      if (diagram.on) {
         diagram.azimuth = diagram.azimuth2;
         diagram.elevation = diagram.elevation2;
         diagram.rotate = diagram.rotate2;
         readantenna(diagram.gainfn2, 1024ul, diagram.gainB);
      }
      ret = Radiorange(image2, posb, antb, antc, (uint32_t)contrast,
                (uint32_t)draft, refraction);
      if (ret==-1L) Error("no altitude at antenne B", 25ul);
      if (verb) osi_Werr("join images\012", 13ul);
   }
   else {
      if (pngdepth>=4L && draft>=2L) ++draft;
      ret = Radiorange(image, posa, anta, antb, (uint32_t)contrast,
                (uint32_t)draft, refraction);
      if (ret==-1L) Error("no altitude at antenne A", 25ul);
   }
   libsrtm_closesrtmfile(); /* free srtm cache */
   if (verb) osi_Werr("make png\012", 10ul);
   if (osmdir[0U]) wrtiles(mappos, initzoom, tozoom);
   if (imagefn[0U]) {
      joincolours();
      if (image2) X2C_DYNDEALLOCATE((char **) &image2);
      ret = wrpng(imagefn, 1024ul, pngdepth, 0L, 0L, xsize, ysize, 1L);
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
