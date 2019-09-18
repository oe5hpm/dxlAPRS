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
#define panorama_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef pngwrite_H_
#include "pngwrite.h"
#endif
#ifndef pngread_H_
#include "pngread.h"
#endif
#ifndef imagetext_H_
#include "imagetext.h"
#endif
#include <math.h>

/* make panorama image with srtm data by oe5dxl */
/*, wgs84r, wgs84s*/
#define panorama_NOALT 20000

#define panorama_PI 3.1415926535

#define panorama_PI2 6.283185307

#define panorama_NL "\012"

#define panorama_LF "\012"

#define panorama_REFRACC 7.85E-8

#define panorama_MAXSYMSIZE 48
/* max input symbol size */

#define panorama_FOGLUM 400.0

#define panorama_SUNSIZE 4.7996554428472E-3

#define panorama_MAXTREESIZE 40

struct COLTYP;


struct COLTYP {
   uint32_t r;
   uint32_t g;
   uint32_t b;
};
/*
     PIX=RECORD r,g,b:CARD16 END;
     IMAGELINE=ARRAY OF PIX;
     IMAGE=ARRAY OF IMAGELINE;
     pIMAGE=POINTER TO IMAGE;
*/

struct POSITIONL;


struct POSITIONL {
   double long0;
   double lat;
};

struct PANOWIN;


struct PANOWIN {
   char flatscreen0;
   struct aprsstr_POSITION eye;
   struct aprsstr_POSITION horizon;
   float eyealt;
   float angle0;
   float elevation;
   float yzoom0;
   float refract;
   int32_t hx; /* mouse pos on panwin and fullwin */
   int32_t hy;
   int32_t mx;
   int32_t my;
   float sunazi;
   float sunele;
   float ang; /* poi fast preselect circular segment */
   float hseg;
   float dist;
   imagetext_pIMAGE image0;
};

struct PIX8A;


struct PIX8A {
   uint8_t r8; /* colour pixel with alpha */
   uint8_t g8;
   uint8_t b8;
   uint8_t alpha;
};

typedef struct PIX8A SYMBOL[48][48];

struct POI;

typedef struct POI * pPOI;


struct POI {
   pPOI next;
   uint32_t r;
   uint32_t g;
   uint32_t b;
   uint32_t alpha;
   SYMBOL symbol;
   uint32_t xs;
   uint32_t ys;
   char poifn[1024];
   char iconfn[1024];
   char iconhint[1024];
};

struct NORMVEC;


struct NORMVEC {
   float x;
   float y;
   float z;
};

struct ROTVEC;


struct ROTVEC {
   float sx;
   float cx;
   float sy;
   float cy;
};

struct RGB;


struct RGB {
   float r;
   float g;
   float b;
};

static imagetext_pIMAGE image;

static pPOI poifiles;

static char imagefn[1024];

static char csvfn[1024];

static int32_t xsize;

static int32_t ysize;

static struct aprsstr_POSITION posa;

static struct aprsstr_POSITION posb;

static float alta;

static float refraction;

static float igamma;

static float elev;

static float angle;

static float yzoom;

static float wavelength;

static float waterslant;

static char gammatab[2048];

static uint32_t accesses;

static uint32_t rastercsv;

static uint32_t poisize;

static uint32_t scalesize;

static uint32_t fonttyp;

static struct COLTYP heavencol;

static struct RGB desertcol;

static char urbanoff;

static char flatscreen;

static char verb;

static char altOG;

static char flatwater;

static double maxmountain;

static double rndseed;

static float sunaz;

static float sunel;

static float snowalt;

static float slowaltslant;

static float waveamp;

static float dusthighpass;

static float treesize;

static float botanicslant;

static float ignorblack;

static float ignorwhite;

static float minpoialt;

static int32_t csvfd;

struct _0;

struct _1;


struct _1 {
   struct aprsstr_POSITION p;
   float a;
};


struct _0 {
   uint32_t act;
   struct _1 v[1000];
};

static struct _0 csvvec;

struct _2;


struct _3 {
   struct _2 * Adr;
   size_t Len0;
};

static struct _3 * pnoise;


struct _2 {
   float v1;
   float v2;
   float v3;
};


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static double Random(void)
{
   rndseed = rndseed+3.1415926535;
   rndseed = rndseed*rndseed;
   rndseed = rndseed*rndseed;
   rndseed = rndseed*rndseed;
   rndseed = rndseed-(double)(uint32_t)X2C_TRUNCC(rndseed,0UL,
                X2C_max_longcard);
   return rndseed;
} /* end Random() */


static uint32_t truncc(float r)
{
   if (r<=0.0f) return 0UL;
   else if (r>=2.E+9f) return 2000000000UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static float frac(float r)
{
   return r-(float)(int32_t)X2C_TRUNCI(r,X2C_min_longint,
                X2C_max_longint);
} /* end frac() */


static float sqr(float x)
{
   return x*x;
} /* end sqr() */


static float plus(float x)
{
   if (x>=0.0f) return x;
   return 0.0f;
} /* end plus() */


static float one(float x)
{
   if (x<0.0f) x = 0.0f;
   else if (x>1.0f) x = 1.0f;
   return x;
} /* end one() */


static float atang2(float x, float y)
{
   float w;
   if ((float)fabs(x)>(float)fabs(y)) {
      w = osic_arctan(X2C_DIVR(y,x));
      if (x<0.0f) {
         if (y>0.0f) w = 3.1415926535f+w;
         else w = w-3.1415926535f;
      }
   }
   else if (y!=0.0f) {
      w = 1.57079632675f-osic_arctan(X2C_DIVR(x,y));
      if (y<0.0f) w = w-3.1415926535f;
   }
   else w = 0.0f;
   return w;
} /* end atang2() */

/*
PROCEDURE rndx(x:LONGREAL):REAL;
BEGIN
  x:=x*10.0;
  RETURN (sin(x*072030.0)*0.0020-sin(x*1721560.0)*0.0014+sin(x*5303500.0)
                *0.0033
         -sin(x*787666.0)*0.0044+sin(x*1287666.0)*0.0051-sin(x*2355636.0)
                *0.0065)*5.5;
END rndx;
PROCEDURE rndy(x:LONGREAL):REAL;
BEGIN
  x:=x*10.0;
  RETURN (sin(x*104830.0)*0.0014-sin(x*2741560.0)*0.0026-sin(x*6373800.0)
                *0.0035
         +sin(x*986626.0)*0.0032-sin(x*2247676.0)*0.0053+sin(x*3254666.0)
                *0.0047)*5.5;
END rndy;
*/

static void posinval(struct aprsstr_POSITION * pos)
{
   pos->long0 = 0.0f;
   pos->lat = 0.0f;
} /* end posinval() */


static char posvalid(const struct aprsstr_POSITION pos)
{
   return pos.lat!=0.0f || pos.long0!=0.0f;
} /* end posvalid() */


static struct aprsstr_POSITION radtodeg(struct aprsstr_POSITION p)
{
   p.lat = p.lat*5.729577951472E+1f;
   p.long0 = p.long0*5.729577951472E+1f;
   return p;
} /* end radtodeg() */


static float radmod(float w)
{
   while (w>3.1415926535f) w = w-6.283185307f;
   while (w<(-3.1415926535f)) w = w+6.283185307f;
   return w;
} /* end radmod() */


static void wgs84sl(double lat, double long0, double nn,
                double * x, double * y, double * z)
{
   double c;
   double h;
   h = nn+6.37E+6;
   *z = h*sin(lat);
   c = cos(lat);
   *y = h*sin(long0)*c;
   *x = h*cos(long0)*c;
} /* end wgs84sl() */


static void wgs84rl(double x, double y, double z,
                double * lat, double * long0,
                double * heig)
{
   double h;
   h = x*x+y*y;
   if (fabs(x)>fabs(y)) {
      *long0 = atan(X2C_DIVL(y,x));
      if (x<0.0) {
         if (y>0.0) *long0 = 3.1415926535+*long0;
         else *long0 = *long0-3.1415926535;
      }
   }
   else {
      *long0 = 1.57079632675-atan(X2C_DIVL(x,y));
      if (y<0.0) *long0 = *long0-3.1415926535;
   }
   *lat = atan(X2C_DIVL(z,sqrt(h)));
   *heig = sqrt(h+z*z)-6.37E+6;
} /* end wgs84rl() */

#define panorama_BYTESPERPIX 4

typedef struct PIX8A * pROWS;


static void readsymbol(SYMBOL img, uint32_t * x, uint32_t * y,
                const char fn[], uint32_t fn_len)
{
   pROWS rows[48];
   int32_t i;
   int32_t res;
   int32_t maxxbyte;
   int32_t maxy;
   int32_t maxx;
   maxx = 48L;
   maxy = 48L;
   maxxbyte = maxx*4L; /* maxxbyte = maxx*4: switch on alpha channel */
   for (i = 0L; i<=47L; i++) {
      rows[i] = (pROWS) &img[i][0U];
   } /* end for */
   res = readpng(fn, (char * *)rows, &maxx, &maxy, &maxxbyte);
   if (res==0L) {
      *x = (uint32_t)maxx;
      *y = (uint32_t)maxy;
   }
   else Error("icon image not readable", 24ul);
/*    x:=0; */
/*    y:=0; */
/*WrInt(res, 5); WrInt(maxx, 5); WrInt(maxy, 5); WrStrLn(""); */
} /* end readsymbol() */


static void makegammatab(uint32_t min0, uint32_t max0)
{
   uint32_t span;
   uint32_t c;
   float g;
   if (max0<=min0) {
      min0 = 0UL;
      max0 = 2047UL;
   }
   span = max0-min0;
   g = X2C_DIVR(1.0f,igamma);
   for (c = 0UL; c<=2047UL; c++) {
      if (c>min0) {
         if (c<=max0) {
            gammatab[c] = (char)(uint32_t)
                X2C_TRUNCC(exp(log((double)(X2C_DIVR((float)(c-min0)
                ,(float)span)))*(double)g)*255.5,0UL,
                X2C_max_longcard);
         }
         else gammatab[c] = '\377';
      }
      else gammatab[c] = 0;
   } /* end for */
} /* end makegammatab() */

#define panorama_MAXH 2047
/* max estimated brightness */

#define panorama_WGHTR 2
/* weight of colours */

#define panorama_WGHTG 4

#define panorama_WGHTB 1
/*      IGNORBLACK=80;                       (* part of all pixel cut off as exotic dark *)  */

/*      IGNORWHITE=30;                       (* part of all pixel cut off as exotic bright *) */
#define panorama_PERC 0.08


static void histogram(imagetext_pIMAGE img, uint32_t * min0,
                uint32_t * max0)
{
   uint32_t ib;
   uint32_t iw;
   uint32_t pc;
   uint32_t y;
   uint32_t x;
   uint32_t h[2048];
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   memset((char *)h,(char)0,8192UL);
   tmp = img->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = img->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct imagetext_PIX * anonym = &img->Adr[(x)*img->Len0+y];
            if (anonym->r>=0U && anonym->r<=2047U) h[anonym->r] += 2UL;
            else h[2047U] += 2UL;
            if (anonym->g>=0U && anonym->g<=2047U) h[anonym->g] += 4UL;
            else h[2047U] += 4UL;
            if (anonym->b>=0U && anonym->b<=2047U) ++h[anonym->b];
            else ++h[2047U];
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   pc = ((img->Len1-1)+1UL)*((img->Len0-1)+1UL);
   ib = truncc((float)pc*ignorblack*0.08f);
   iw = truncc((float)pc*ignorwhite*0.08f);
   *min0 = 0UL;
   while (ib>=h[*min0] && *min0<2047UL) {
      ib -= h[*min0];
      ++*min0;
   }
   *max0 = 2047UL;
   while (iw>=h[*max0] && *max0>0UL) {
      iw -= h[*max0];
      --*max0;
   }
} /* end histogram() */


static uint8_t pngc(uint16_t c)
{
   if (c<=2047U) return gammatab[c];
   else return 255U;
   return 0;
} /* end pngc() */


static void wrpng(void)
{
   struct PNGPIXMAP pngimg;
   int32_t ret;
   int32_t y;
   int32_t x;
   uint32_t max0;
   uint32_t min0;
   struct imagetext_PIX * anonym;
   struct PNGPIXEL * anonym0;
   int32_t tmp;
   int32_t tmp0;
   osic_alloc((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   if (pngimg.image) {
      histogram(image, &min0, &max0);
      if (verb) {
         osi_WrStr("min..max: ", 11ul);
         osic_WrINT32(min0, 5UL);
         osic_WrINT32(max0, 5UL);
         osi_WrStrLn("", 1ul);
      }
      /*IF max<800 THEN max:=800 END; */
      makegammatab(min0, max0);
      tmp = ysize-1L;
      y = 0L;
      if (y<=tmp) for (;; y++) {
         tmp0 = xsize-1L;
         x = 0L;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct imagetext_PIX * anonym = &image->Adr[(x)
                *image->Len0+((ysize-1L)-y)];
               { /* with */
                  struct PNGPIXEL * anonym0 = &pngimg.image[x+y*xsize];
                  anonym0->red = pngc(anonym->r);
                  anonym0->green = pngc(anonym->g);
                  anonym0->blue = pngc(anonym->b);
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
      pngimg.width = (uint32_t)xsize;
      pngimg.height = (uint32_t)ysize;
      ret = writepng(imagefn, &pngimg);
      osic_free((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   }
   else osi_Werr("png write out of memory\012", 25ul);
} /* end wrpng() */


static void getpos(char h[1024], struct aprsstr_POSITION * p,
                const char e[], uint32_t e_len)
{
   osi_NextArg(h, 1024ul);
   aprsstr_loctopos(p, h, 1024ul);
   if (!posvalid(*p)) {
      if (!aprsstr_StrToFix(&p->lat, h,
                1024ul) || (float)fabs(p->lat)>=90.0f) Error(e, e_len);
      osi_NextArg(h, 1024ul);
      if (!aprsstr_StrToFix(&p->long0, h,
                1024ul) || (float)fabs(p->long0)>180.0f) Error(e, e_len);
      p->lat = p->lat*1.7453292519444E-2f;
      p->long0 = p->long0*1.7453292519444E-2f;
   }
} /* end getpos() */


static void Parms(void)
{
   char err;
   char iconhint[1024];
   char iconfn[1024];
   char h[1024];
   float rn;
   float dist;
   float pan;
   uint32_t label;
   pPOI ppoi;
   int32_t col;
   err = 0;
   label = 0UL;
   iconfn[0] = 0;
   iconhint[0] = 0;
   pan = 0.0f;
   dist = 0.0f;
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
         else if (h[1U]=='P') {
            osic_alloc((char * *) &ppoi, sizeof(struct POI));
            if (ppoi==0) Error("poifile, out of memory", 23ul);
            ppoi->r = 255UL;
            ppoi->g = 255UL;
            ppoi->b = 255UL;
            ppoi->alpha = 255UL;
            osi_NextArg(h, 1024ul);
            if (aprsstr_StrToInt(h, 1024ul, &col)) {
               ppoi->r = (uint32_t)col;
               osi_NextArg(h, 1024ul);
               if (aprsstr_StrToInt(h, 1024ul, &col)) {
                  ppoi->g = (uint32_t)col;
                  osi_NextArg(h, 1024ul);
                  if (aprsstr_StrToInt(h, 1024ul, &col)) {
                     ppoi->b = (uint32_t)col;
                     osi_NextArg(h, 1024ul);
                     if (aprsstr_StrToInt(h, 1024ul, &col)) {
                        ppoi->alpha = (uint32_t)col;
                        osi_NextArg(h, 1024ul);
                     }
                  }
               }
            }
            aprsstr_Assign(ppoi->poifn, 1024ul, h, 1024ul);
            if (ppoi->poifn[0U]==0 || ppoi->poifn[0U]=='-') {
               Error("-P <POI filename>", 18ul);
            }
            aprsstr_Assign(ppoi->iconfn, 1024ul, iconfn, 1024ul);
            aprsstr_Assign(ppoi->iconhint, 1024ul, iconhint, 1024ul);
            ppoi->next = poifiles;
            poifiles = ppoi;
         }
         else if (h[1U]=='c') {
            osi_NextArg(csvfn, 1024ul);
            if (csvfn[0U]==0 || csvfn[0U]=='-') {
               Error("-c <csv filename>", 18ul);
            }
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &col) || col<0L) {
               Error("-C <number>", 12ul);
            }
            rastercsv = (uint32_t)col;
         }
         else if (h[1U]=='I') {
            osi_NextArg(iconfn, 1024ul);
            if (iconfn[0U]==0 || iconfn[0U]=='-') {
               Error("-I <icon filename> (png)", 25ul);
            }
         }
         else if (h[1U]=='J') {
            osi_NextArg(iconhint, 1024ul);
            if (iconhint[0U]==0 || iconhint[0U]=='-') {
               Error("-J <icon hint>", 15ul);
            }
            if (iconhint[0U]==',') iconhint[0U] = 0;
         }
         else if (h[1U]=='f') flatscreen = 1;
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='O') altOG = 1;
         else if (h[1U]=='u') urbanoff = 1;
         else if (h[1U]=='x') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &xsize) || xsize<20L) || xsize>=32000L) {
               Error("-x <size>", 10ul);
            }
         }
         else if (h[1U]=='y') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &ysize) || ysize<20L) || ysize>=8192L) {
               Error("-y <size>", 10ul);
            }
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &libsrtm_srtmmaxmem)) {
               Error("-M <bytes>", 11ul);
            }
         }
         else if (h[1U]=='p') {
            osi_NextArg(libsrtm_srtmdir, 1024ul);
            if (libsrtm_srtmdir[0U]==0 || libsrtm_srtmdir[0U]=='-') {
               Error("-p <srtm-folder-path>", 22ul);
            }
         }
         else if (h[1U]=='a') {
            getpos(h, &posa, "-a <lat> <long> or <locator>", 29ul);
         }
         else if (h[1U]=='b') {
            getpos(h, &posb, "-b <lat> <long> or <locator>", 29ul);
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&alta, h,
                1024ul) || alta<0.0f) || alta>50000.0f) {
               Error("-A <meter> (0..50000)", 22ul);
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&pan, h, 1024ul)) {
               Error("-d <deg> <km>", 14ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dist, h, 1024ul)) {
               Error("-d <deg> <km>", 14ul);
            }
         }
         else if (h[1U]=='S') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&sunaz, h, 1024ul)) {
               Error("-S <deg> <deg>", 15ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&sunel, h, 1024ul) || sunel>90.0f) {
               Error("-S <deg> <deg>", 15ul);
            }
         }
         else if (h[1U]=='W') {
            /*     ELSIF h[1]="q" THEN bicubic:=TRUE; */
            flatwater = 1;
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&waveamp, h,
                1024ul) || waveamp<0.0f) || waveamp>10.0f) {
               Error("-W <wavehigth> <wavelength> <flatness>", 39ul);
            }
            waveamp = waveamp*0.01f;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&wavelength, h, 1024ul)) {
               Error("-W <wavehigth> <wavelength> <flatness>", 39ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&waterslant, h, 1024ul)) {
               Error("-W <wavehigth> <wavelength> <flatness>", 39ul);
            }
         }
         else if (h[1U]=='m') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&rn, h, 1024ul)) {
               Error("-m <maxmountainhight>", 22ul);
            }
            maxmountain = (double)rn;
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&treesize, h, 1024ul)) {
               Error("-t <treesize> <botanicslant>", 29ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&botanicslant, h, 1024ul)) {
               Error("-t <treesize> <botanicslant>", 29ul);
            }
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&ignorblack, h, 1024ul)) {
               Error("-l <black%> <white%>", 21ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&ignorwhite, h, 1024ul)) {
               Error("-t <black%> <white%>", 21ul);
            }
         }
         else if (h[1U]=='D') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&desertcol.r, h, 1024ul)) {
               Error("-D <r> <g> <b>", 15ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&desertcol.g, h, 1024ul)) {
               Error("-D <r> <g> <b>", 15ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&desertcol.b, h, 1024ul)) {
               Error("-D <r> <g> <b>", 15ul);
            }
         }
         else if (h[1U]=='o') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&minpoialt, h,
                1024ul) || minpoialt<(-2.E+4f)) || minpoialt>20000.0f) {
               Error("-o <meter> (0..20000)", 22ul);
            }
         }
         else if (h[1U]=='G') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&snowalt, h, 1024ul)) {
               Error("-G <meter> <meter>", 19ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&slowaltslant, h, 1024ul)) {
               Error("-G <meter> <meter>", 19ul);
            }
            if (slowaltslant<1.0f) slowaltslant = 1.0f;
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&refraction, h,
                1024ul) || refraction<0.0f) || refraction>1.0f) {
               Error("-r <refraction> [0.0..1.0]", 27ul);
            }
         }
         else if (h[1U]=='e') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&elev, h,
                1024ul) || elev<(-90.0f)) || elev>90.0f) {
               Error("-e <elevation> [-90.0..90.0]", 29ul);
            }
         }
         else if (h[1U]=='w') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&angle, h,
                1024ul) || angle<0.1f) || angle>360.0f) {
               Error("-w <degrees> [0.1..360.0]", 26ul);
            }
         }
         else if (h[1U]=='z') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&yzoom, h,
                1024ul) || yzoom<0.001f) || yzoom>10.0f) {
               Error("-z <factor> [0.0..10.0]", 24ul);
            }
         }
         else if (h[1U]=='s') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &poisize) || poisize>20UL) {
               Error("-s <size> 1..20", 16ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&igamma, h,
                1024ul) || igamma<0.01f) || igamma>10.0f) {
               Error("-g <gamma> [0.1..10]", 21ul);
            }
         }
         else if (h[1U]=='H') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &heavencol.r) || heavencol.r>=256UL) {
               Error("-H <r> <g> <b> 0..1023", 23ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &heavencol.g) || heavencol.g>=256UL) {
               Error("-H <r> <g> <b> 0..1023", 23ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &heavencol.b) || heavencol.b>=256UL) {
               Error("-H <r> <g> <b> 0..1023", 23ul);
            }
            heavencol.r = heavencol.r*4UL;
            heavencol.g = heavencol.g*4UL;
            heavencol.b = heavencol.b*4UL;
         }
         else if (h[1U]=='F') {
            osi_NextArg(h, 1024ul);
            if (h[0U]=='1') fonttyp = 6UL;
            else if (h[0U]=='2') fonttyp = 8UL;
            else if (h[0U]=='3') fonttyp = 10UL;
            else Error("-F <font> (1:6x10 2:10x20)", 27ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("Panorama", 9ul);
               osi_WrStrLn(" -A <m>                            Camera over gr\
ound [m] (10)", 63ul);
               osi_WrStrLn(" -a <lat> <long> | [locator]       Camera positio\
n lat long (degrees) or qth locator", 85ul);
               osi_WrStrLn(" -b <lat> <long> | [locator]       Pan to point l\
at long (degrees) or qth locator", 82ul);
               osi_WrStrLn(" -C <N>                            write CSV line\
 for every N\'th column", 72ul);
               osi_WrStrLn(" -c <filename>                     CSV file name",
                 49ul);
               osi_WrStrLn(" -d <deg> <km>                     Relative to po\
sition a (alternativ to -b)", 77ul);
               osi_WrStrLn(" -D <r> <g> <b>                    Sand/Rock colo\
ur", 52ul);
               osi_WrStrLn("                                     km distance \
sight limit", 61ul);
               osi_WrStrLn(" -e <degrees>                      Camera Elevati\
on (degrees) (0.0)", 68ul);
               osi_WrStrLn("                                     speed optimi\
zer, surface contrast and dust calculation", 92ul);
               osi_WrStrLn("                                     are able to \
handle only small elevations!", 79ul);
               osi_WrStrLn(" -F <font>                         Font Size (1) \
1: 6x10, 2: 8x14, 3: 10x20", 76ul);
               osi_WrStrLn(" -f                                Flat screen el\
se Curved projection area (needed at >=180deg sight)", 102ul);
               osi_WrStrLn(" -G <meter> <meter>                Glazier/Snow a\
ltitude and fade out (3000 500)", 81ul);
               osi_WrStrLn(" -g <gamma>                        Image Gamma 0.\
1..10 (2.2)", 61ul);
               osi_WrStrLn(" -H <r> <g> <b>                    Heaven colour \
(50 70 300)", 61ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I <filename>                     Symbol Image F\
ile Name, set before -P", 73ul);
               osi_WrStrLn(" -i <filename>                     Image File Nam\
e", 51ul);
               osi_WrStrLn(" -J <poi-hint>                     text appended \
to csv lines, set before -P, off with -J ,", 92ul);
               osi_WrStrLn(" -l <%> <%>                        fit image cont\
rast to saturate % of all black/white pixels (0.1 0.5)", 104ul);
               osi_WrStrLn(" -M <bytes>                        SRTM-Cache Lim\
it (100000000)", 64ul);
               osi_WrStrLn(" -m <meter>                        limit searchin\
g montain size in viewed area for speed (8900)", 96ul);
               osi_WrStrLn(" -O                                POI File altit\
ude is over ground not NN", 75ul);
               osi_WrStrLn(" -o <m>                            min. POI altit\
ude over ground (1)", 69ul);
               osi_WrStrLn("                                     if given in \
POI file and higher use this", 78ul);
               osi_WrStrLn(" -P [<r> <g> <b> [<transp>]] <filename>  optional\
 icon colours (0..255) POI File Name", 86ul);
               osi_WrStrLn("                                     repeat for m\
ore files", 59ul);
               osi_WrStrLn(" -p <pathname>                     folder with /s\
rtm1 /srtm3 /srtm30", 69ul);
               /*        WrStrLn("
                -q                                bicubic srtm interpolation instead of bilinear"
                ); */
               osi_WrStrLn(" -r <refraction>                   0.0(vacuum)..1\
.0(earth is a disk) (0.13)", 76ul);
               osi_WrStrLn(" -S <deg> <deg>                    Sun azimuth an\
d elevation", 61ul);
               osi_WrStrLn(" -s <size>                         POI symbol siz\
e (5)", 55ul);
               osi_WrStrLn(" -t <meter> <steep>                Tree size (30)\
 lowered to 0 from 1000 to 2000m NN, 0 wood off", 97ul);
               osi_WrStrLn("                                   steep=0 all ro\
ck, steep=5 rock if steeper (5) lowered to 0 from 1000 to 2000m NN, 0 wood of\
f", 128ul);
               osi_WrStrLn(" -u                                Urban area off\
", 50ul);
               osi_WrStrLn(" -v                                Say something",
                 49ul);
               osi_WrStrLn(" -W <wavehigth> <wavelength> <flatness>  draw Wat\
er with waves (0.6) (1.0) (0.0)", 81ul);
               osi_WrStrLn(" -w <degrees>                      Camera horizon\
tal sight angle (degrees) (45.0)", 82ul);
               osi_WrStrLn(" -x <size>                         Image size x (\
600)", 54ul);
               osi_WrStrLn(" -y <size>                         Image size y (\
400)", 54ul);
               osi_WrStrLn(" -z <factor>                       Vertical Zoom \
(for 360deg panorama) (1.0)", 77ul);
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
   if ((!posvalid(posb) && posvalid(posa)) && dist>0.01f) {
      pan = pan*1.7453292519444E-2f;
      posb.lat = (float)((double)
                posa.lat+X2C_DIVL(cos((double)pan)*(double)dist,
                6370.0));
      posb.long0 = (float)((double)
                posa.long0+X2C_DIVL((X2C_DIVL(sin((double)pan),
                cos((double)posa.lat)))*(double)dist,6370.0));
      /*WrFixed(posb.lat/RAD, 5,10); WrFixed(posb.long/RAD, 5,10); */
      /*WrFixed(azimuth(posa, posb), 2,10);
                WrFixed(distance(posa, posb), 3,10); WrStrLn(""); */
      if (!posvalid(posb)) Error("camera sight point out of database", 35ul);
   }
} /* end Parms() */


static void wrcsv(int32_t px, int32_t py,
                const struct aprsstr_POSITION posdeg, float alti,
                const char text[], uint32_t text_len,
                const char iconhint[], uint32_t iconhint_len)
{
   char h[1001];
   char s[1001];
   if (csvfn[0U]) {
      if (csvfd<0L) csvfd = osi_OpenWrite(csvfn, 1024ul);
      if (csvfd<0L) Error("cannot write csv-file", 22ul);
      aprsstr_IntToStr(px, 0UL, s, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      aprsstr_IntToStr(py, 0UL, h, 1001ul);
      aprsstr_Append(s, 1001ul, h, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      aprsstr_FixToStr(posdeg.lat, 6UL, h, 1001ul);
      aprsstr_Append(s, 1001ul, h, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      aprsstr_FixToStr(posdeg.long0, 6UL, h, 1001ul);
      aprsstr_Append(s, 1001ul, h, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      if (alti>(-1000.0f) && alti<10000.0f) {
         aprsstr_IntToStr((int32_t)X2C_TRUNCI(alti+0.5f,X2C_min_longint,
                X2C_max_longint), 0UL, h, 1001ul);
         aprsstr_Append(s, 1001ul, h, 1001ul);
      }
      if (text[0UL]) {
         aprsstr_Append(s, 1001ul, ",", 2ul);
         aprsstr_Append(s, 1001ul, text, text_len);
      }
      if (iconhint[0UL]) {
         aprsstr_Append(s, 1001ul, ",", 2ul);
         aprsstr_Append(s, 1001ul, iconhint, iconhint_len);
      }
      aprsstr_Append(s, 1001ul, "\012", 2ul);
      osi_WrBin(csvfd, (char *)s, 1001u/1u, aprsstr_Length(s, 1001ul));
   }
/* WITH image^[px][py] DO INC(r, 200); INC(g, 200); END; */
} /* end wrcsv() */

#define panorama_DDEG 7.85398163375E-6

#define panorama_DALT 10.0


static void xycsv(uint32_t px, uint32_t py, struct POSITIONL posl,
                char flush, char cont)
{
   float nf;
   float ka;
   float ky;
   float kx;
   float a0;
   float y00;
   float x0;
   float dx;
   float resol;
   float alt;
   uint32_t i;
   uint32_t n;
   char eov;
   struct aprsstr_POSITION pos;
   pos.long0 = (float)posl.long0;
   pos.lat = (float)posl.lat;
   alt = libsrtm_getsrtm(pos, 0UL, &resol);
   if (cont) {
      n = csvvec.act+1UL;
      if (n>2UL || n>0UL && flush) {
         if (flush || n>=999UL) eov = 1;
         else {
            nf = (float)n;
            x0 = csvvec.v[0U].p.long0;
            y00 = csvvec.v[0U].p.lat;
            a0 = csvvec.v[0U].a;
            kx = X2C_DIVR(pos.long0-x0,nf);
            ky = X2C_DIVR(pos.lat-y00,nf);
            ka = X2C_DIVR(alt-a0,(float)n);
            eov = 0;
            dx = (float)(7.85398163375E-6*cos((double)pos.lat));
            i = n;
            do {
               --i;
               nf = (float)i;
               if (((i>=n || (float)fabs((x0+kx*nf)-csvvec.v[i].p.long0)
                >dx) || (float)fabs((y00+ky*nf)-csvvec.v[i].p.lat)
                >7.85398163375E-6f) || (float)fabs((a0+ka*nf)
                -csvvec.v[i].a)>10.0f) eov = 1;
            } while (!(i==0UL || eov));
         }
         if (eov) {
            wrcsv((int32_t)px, (int32_t)(py-n), radtodeg(csvvec.v[0U].p),
                 csvvec.v[0U].a, "", 1ul, "", 1ul);
            if (n>1UL) {
               wrcsv((int32_t)px, (int32_t)(py-1UL),
                radtodeg(csvvec.v[n-1UL].p), csvvec.v[n-1UL].a, "", 1ul, "",
                1ul);
            }
            n = 0UL;
         }
      }
   }
   else n = 0UL;
   csvvec.act = n;
   csvvec.v[n].a = alt;
   csvvec.v[n].p = pos;
} /* end xycsv() */


static void colblend(float v, float r, float g, float b,
                float r1, float g1, float b1, float * rr,
                float * gg, float * bb)
{
   float vv;
   if (v<0.0f) v = 0.0f;
   else if (v>1.0f) v = 1.0f;
   vv = 1.0f-v;
   *rr = r*vv+r1*v;
   *gg = g*vv+g1*v;
   *bb = b*vv+b1*v;
} /* end colblend() */


static void azieletonvec(float az, float el, struct ROTVEC * n)
{
   n->sx = osic_sin(az);
   n->cx = osic_cos(az);
   n->sy = osic_sin(el);
   n->cy = osic_cos(el);
} /* end azieletonvec() */

#define panorama_TESTDIST 10.0


static void normvec(struct POSITIONL pos, libsrtm_pMETAINFO pmeta,
                float * slant, double * h, struct NORMVEC * nvec,
                uint8_t * water, uint8_t * wood, uint8_t * urban)
{
   float resol;
   uint8_t attr;
   pmeta->withslant = 1;
   pmeta->aliasattr = 1;
   *h = (double)libsrtm_getsrtmlong(pos.lat, pos.long0, 1UL, 1, &resol,
                 &attr, pmeta);
   *water = (uint8_t)(uint32_t)X2C_TRUNCC(pmeta->attrweights[3U]*100.0f,
                0UL,X2C_max_longcard);
   if (treesize>0.0f) {
      *wood = (uint8_t)(uint32_t)
                X2C_TRUNCC(pmeta->attrweights[2U]*100.0f,0UL,
                X2C_max_longcard);
   }
   else *wood = 0U;
   if (urbanoff) *urban = 0U;
   else {
      *urban = (uint8_t)(uint32_t)
                X2C_TRUNCC(pmeta->attrweights[1U]*100.0f,0UL,
                X2C_max_longcard);
   }
   /*dir:=atang2(meta.slantx, meta.slanty); */
   /*slant:=1.0-osi.arctan(osi.sqrt(meta.slantx*meta.slantx+meta.slanty*meta.slanty)
                )*(0.5/PI); */
   *slant = osic_cos(osic_arctan(osic_sqrt(pmeta->slantx*pmeta->slantx+pmeta->slanty*pmeta->slanty)
                ));
   nvec->z = X2C_DIVR(1.0f,osic_sqrt(pmeta->slantx*pmeta->slantx+pmeta->slanty*pmeta->slanty+1.0f));
   nvec->x = pmeta->slantx*nvec->z;
   nvec->y = pmeta->slanty*nvec->z;
/*
  pos1.lat:=pos.lat + TESTDIST/20000000.0*PI;
  pos1.long:=pos.long;
--WrFixed(meta.slantx, 2, 6);WrFixed(meta.slanty, 2, 6);
--WrFixed(osi.arctan(osi.sqrt(meta.slantx*meta.slantx+meta.slanty*meta.slanty)
                ), 2, 6);
--WrFixed(1.0-(meta.slantx*meta.slantx+meta.slanty*meta.slanty)*0.5, 3, 7);

meta.withslant:=FALSE;
  h1:=getsrtmlong(pos1.lat, pos1.long, 1, TRUE, resol, attr, NIL);
  pos1.long:=pos.long + (TESTDIST/20000000.0*PI)/cos(pos.lat);
  pos1.lat:=pos.lat;
  h2:=getsrtmlong(pos1.lat, pos1.long, 1, TRUE, resol, attr, NIL);
  sx:=osi.arctan((h1-h)*(1.0/TESTDIST));
  sy:=osi.arctan((h2-h)*(1.0/TESTDIST));
  slant:=osi.cos(sx)*osi.cos(sy);
--  dir:=atang2(sy, sx);
WrFixed(slant, 3, 6); WrStrLn(",");
*/
} /* end normvec() */

/*
 Naeherungsloesung für die Refraktion fuer ein Objekt bei Hoehe hoehe über mathematischem Horizont
 Refraktion beträgt bei Sonnenaufgang 34 Bogenminuten = 0.56667°
 Falls die Höhe der Sonne nicht genauer als auf 0.5° gewuenscht ist,
                kann diese Funktion ignoriert werden
*/
/* Luftdruck der Standard-Atmosphäre in hPa (=mbar) */
/* Temperatur der Standard-Atmosphäre in gradC */
/* ueber 15grad - einfachere Formel */
/* Refraktion in Radians */

static float inv(float x)
{
   x = (float)fabs(x);
   if (x==0.0f) return X2C_max_real;
   return X2C_DIVR(1.0f,x);
} /* end inv() */

#define panorama_DUST 1.0

#define panorama_MAXSUN 8000.0

#define panorama_MAXSUNDUST 6000.0

#define panorama_LENSEFFECT 40.0


static void sundisk(double wx, double wy, double sunazi,
                double sunele, float dist, float sunarea,
                char lenseffect, float * lr, float * lg,
                float * lb)
{
   float sb;
   float sg;
   float sr;
   float hb;
   float hg;
   float hr;
   float lens;
   float ash;
   float as;
   float es;
   float ed;
   float sy;
   float s;
   float d;
   float y;
   float x;
   /*
   IF dist=0.0 THEN
   WrFixed(wx, 2, 8);WrFixed(wy, 2, 8);WrFixed(sunazi, 2, 8);
                WrFixed(sunele, 2, 8); WrStrLn("");
   END;
   */
   x = radmod((float)(wx-sunazi));
   y = radmod((float)(wy-sunele));
   d = x*x+y*y;
   ed = (float)(1.0-wy*6.3661977238578E-1);
   if (ed<=0.0f) ed = 0.0f;
   else ed = (float)pow((double)ed, 6.0);
   if (ed>1.0f) ed = 1.0f;
   s = (float)(1.0-sunele*6.3661977238578E-1); /* sunlight colour */
   if (s>1.0f) s = 1.0f;
   else if (s<0.001f) s = 0.001f;
   es = (float)pow((double)s, 12.0);
   sr = 1.0f+1.0f*es;
   sg = 1.0f-1.0f*es*0.7f;
   sb = 1.0f-1.0f*es*1.5f;
   if (sb<0.0f) sb = 0.0f;
   if (dist<0.0f) {
      /* sunlight colour only*/
      *lr = sr;
      *lg = sg;
      *lb = sb;
   }
   else if (dist==0.0f && d<=2.3036692370053E-5f) {
      /* sun */
      *lr = sr*8000.0f;
      *lg = sg*8000.0f;
      *lb = sb*8000.0f;
   }
   else {
      if (dist==0.0f) {
         s = (float)(X2C_DIVL(0.2,
                (double)(1.0f+x*x*4.0f)*(1.0+fabs(sunele)*2.0)));
                /* sun dust horizontal expansion in heaven */
         sy = (float)(X2C_DIVL(1.0,1.0+X2C_DIVL(wy*9.0,
                1.0+fabs(sunele)*10.0)));
         ash = s*sy;
      }
      /*ash:=0.0; */
      as = (float)((double)d*(4.0+sunele*8.0));
      if (as>1.0f) as = 1.0f;
      as = (float)(pow((double)(1.0f-as), 4.0)*(0.06+sunele*0.0));
      /*as:=0.0; */
      colblend(ed, (float)heavencol.r, (float)heavencol.g,
                (float)heavencol.b, 400.0f, 400.0f, 400.0f, &hr, &hg,
                &hb);
      colblend((float)((double)es*(X2C_DIVL(1.0-cos((double)x)
                ,3.0))), hr, hg, hb, 0.0f, 0.0f, 0.0f, &hr, &hg, &hb);
                /* blue sky */
      colblend(es*0.6f, hr, hg, hb, 0.0f, 0.0f, 0.0f, &hr, &hg, &hb);
                /* low altitude blue */
      if (dist==0.0f) {
         /* add sun dust to heaven */
         /*
               colblend(es*ed,
                      MAXSUNDUST,MAXSUNDUST,MAXSUNDUST,
                      MAXSUNDUST*(1.0-DUST*0.0),MAXSUNDUST*(1.0-DUST*0.95),
                MAXSUNDUST*(1.0-DUST),
                      lr,lg,lb);                                 (* sun dust colour *)
         */
         *lr = hr+sr*(ash+as)*6000.0f;
         *lg = hg+sg*(ash+as)*6000.0f;
         *lb = hb+sb*(ash+as)*6000.0f; /* sun dust horizontal */
      }
      else {
         /*      lr:=hr+MAXSUNDUST*(1.0-DUST*0.0)*as*es*ed; */
         /*      lg:=hg+MAXSUNDUST*(1.0-DUST*0.95)*as*es*ed; */
         /*      lb:=hb+MAXSUNDUST*(1.0-DUST)*as*es*ed;
                (* sun dust around sun *)  */
         /* dust from earth to eye */
         /*
               colblend(es*ed,
                    MAXSUNDUST,MAXSUNDUST,MAXSUNDUST,
                    MAXSUNDUST*(1.0-DUST*0.0),MAXSUNDUST*(1.0-DUST*0.95),
                MAXSUNDUST*(1.0-DUST),
                    sr,sg,sb);
         */
         as = as*sunarea*6000.0f;
         hr = hr+sr*as;
         hg = hg+sg*as;
         hb = hb+sb*as;
         colblend(dist, *lr, *lg, *lb, hr, hg, hb, lr, lg, lb);
      }
      if (lenseffect) {
         lens = inv((float)fabs(sin((double)(atang2(x, y)*7.0f))));
         if (lens>1.5f) lens = 1.5f;
         /*    IF lens>d*150.0+0.1 THEN lens:=d*150.0+0.1 END; */
         /*WrFixed(sin(atang2(x,y)),2,8); */
         /*    lens:=inv(x)+inv(y)+inv(x-y)+inv(x+y); */
         lens = X2C_DIVR(lens,d*150.0f+0.1f);
         /*    IF lens>100.0 THEN lens:=100.0 END; */
         lens = lens*sunarea*40.0f;
         *lr = *lr+sr*lens;
         *lg = *lg+sg*lens;
         *lb = *lb+sb*lens; /* lens effect */
      }
   }
} /* end sundisk() */

/*
PROCEDURE rand(y:CARDINAL; VAR r1,r2,r3:REAL);
                (* 2 dimensional random noise filter *)
CONST LIM=0.1;
VAR r:REAL; 
BEGIN
  r1:=Random()-0.5;
  rndsum1:=rndsum1*0.9 + r1;
  r2:=Random()-0.5;
  rndsum2:=rndsum2*0.9 + r2;
  r3:=Random()-0.5;
  rndsum3:=rndsum3*0.9 + r3;

  pnoise^[y].v1:=pnoise^[y].v1*0.9 + rndsum1;
  pnoise^[y].v2:=pnoise^[y].v2*0.9 + rndsum2;
  pnoise^[y].v3:=pnoise^[y].v3*0.9 + rndsum3;
  r:=pnoise^[y].v1;
  r1:=r1*0.3 + r*r*r*0.004;
  IF r1>LIM THEN r1:=LIM ELSIF r1<-LIM THEN r1:=-LIM END;
  r:=pnoise^[y].v2;
  r2:=r2*0.3 + r*r*r*0.004;
  IF r2>LIM THEN r2:=LIM ELSIF r2<-LIM THEN r2:=-LIM END;
  r:=pnoise^[y].v3;
  r3:=r3*0.3 + r*r*r*0.004;
END rand;
*/

static void randstep(uint32_t y)
/* 2 dimensional random noise filter */
{
   ++y;
   pnoise->Adr[y].v1 = (float)(((double)(pnoise->Adr[y].v1*0.7f)
                +Random())-0.5);
   pnoise->Adr[y].v1 = pnoise->Adr[y].v1*0.3333f+pnoise->Adr[y-1UL]
                .v1*0.3333f+pnoise->Adr[y+1UL].v1*0.3333f;
   pnoise->Adr[y].v2 = (float)(((double)(pnoise->Adr[y].v2*0.7f)
                +Random())-0.5);
   pnoise->Adr[y].v2 = pnoise->Adr[y].v2*0.3333f+pnoise->Adr[y-1UL]
                .v2*0.3333f+pnoise->Adr[y+1UL].v2*0.3333f;
   pnoise->Adr[y].v3 = (float)(((double)(pnoise->Adr[y].v3*0.7f)
                +Random())-0.5);
   pnoise->Adr[y].v3 = pnoise->Adr[y].v3*0.3333f+pnoise->Adr[y-1UL]
                .v3*0.3333f+pnoise->Adr[y+1UL].v3*0.3333f;
} /* end randstep() */

#define panorama_LIM 0.2


static void randc(uint32_t y, float * r1, float * r2, float * r3)
/* 2 dimensional random noise filter */
{
   float r;
   r = pnoise->Adr[y].v1;
   *r1 = r*r*r*12.0f;
   if (*r1>0.2f) *r1 = 0.2f;
   else if (*r1<(-0.2f)) *r1 = (-0.2f);
   r = pnoise->Adr[y].v2;
   *r2 = r*r*r*12.0f;
   if (*r2>0.2f) *r2 = 0.2f;
   else if (*r2<(-0.2f)) *r2 = (-0.2f);
   r = pnoise->Adr[y].v3;
   *r3 = r*r*r*12.0f;
   if (*r3>0.2f) *r3 = 0.2f;
   else if (*r3<(-0.2f)) *r3 = (-0.2f);
} /* end randc() */

/*
PROCEDURE treeform(a:LONGREAL):REAL;
BEGIN
  a:=a-VAL(LONGREAL, VAL(INTEGER,a));
  IF a>0.5 THEN a:=1.0-a END;
  RETURN a*2.0
END treeform;
*/
/*  h:=h+woodh*30.0*treeform(pos.lat*1000000.0)
                *treeform(pos.long*1000000.0*cos(pos.lat)); */
/*  h:=h+woodh*(ABS(osi.sin(pos.lat*6000000.0))
                *ABS(osi.sin(pos.long*6000000.0*osi.cos(pos.lat)))
                /(0.2+dist*0.0001)+25.0); */
/*  h:=h+woodh*(30.0-ABS(osi.sin(pos.lat*6000000.0))/(0.16+dist*0.0001)); */
#define panorama_LINSTEP 4000.0


static void raytrace(double x0, double y00, double z0,
                double dx, double dy, double dz,
                float maxdist, float altrise, float refrac,
                double * dist, float * subpix,
                struct POSITIONL * pos)
{
   double alt;
   double sp;
   float h;
   float tol;
   float ypixalt;
   float resol;
   float step;
   float lastsp;
   float spo;
   uint32_t iter;
   uint8_t attr;
   struct libsrtm_METAINFO meta;
   double ddist;
   double dist1;
   double dist0;
   double alt1;
   double long1;
   double lat1;
   double alt0;
   double long0;
   double lat0;
   double dalt;
   double dlong;
   double dlat;
   step = 0.0f;
   if (*dist==0.0) lastsp = 0.0f;
   else lastsp = X2C_max_real;
   alt = (double)X2C_max_real;
   *subpix = 0.0f;
   /*  ypixalt:=0.0; */
   spo = 0.0f;
   /*WrStrLn("---+ "); */
   /*WrFixed(dist, 1,5); WrStr("=dist "); */
   meta.withslant = 0;
   meta.aliasattr = 1; /* watsh for wood */
   iter = 3UL;
   dist0 = *dist;
   dist1 = *dist;
   wgs84rl(x0+dx*dist1, y00+dy*dist1, z0+dz*dist1, &lat1, &long1, &alt1);
   alt1 = alt1-dist1*dist1*(double)refrac;
   for (;;) {
      if (*dist>=dist1) {
         /* end of linearized peace */
         lat0 = lat1;
         long0 = long1;
         alt0 = alt1;
         dist0 = dist1;
         dist1 = dist1+4000.0;
         wgs84rl(x0+dx*dist1, y00+dy*dist1, z0+dz*dist1, &lat1, &long1,
                &alt1);
         alt1 = alt1-dist1*dist1*(double)refrac;
         dalt = X2C_DIVL(alt1-alt0,4000.0);
         dlong = X2C_DIVL(long1-long0,4000.0);
         dlat = X2C_DIVL(lat1-lat0,4000.0);
      }
      ddist = *dist-dist0; /* go ahead linearized */
      pos->lat = lat0+dlat*ddist;
      pos->long0 = long0+dlong*ddist;
      alt = alt0+dalt*ddist;
      ypixalt = (float)((double)(float)fabs(altrise)* *dist);
                /* pixel size */
      tol = ypixalt*0.25f;
      if (tol<0.1f) tol = 0.1f;
      h = libsrtm_getsrtmlong(pos->lat, pos->long0,
                (uint32_t)X2C_TRUNCC((float)fabs(ypixalt)*0.5f,0UL,
                X2C_max_longcard), ypixalt<5.0f, &resol, &attr, &meta);
      ++accesses;
      if (h<10000.0f) {
         sp = alt-(double)h;
         /*IF altrise<0.0 THEN WrFixed(h, 1,7); WrFixed(pos.lat/RAD, 6,12);
                WrFixed(pos.long/RAD, 6,12); END; */
         if (((treesize>0.0f && sp<40.0) && meta.attrweights[2U]>0.51f)
                && *dist>300.0) {
            /*        treegrow(pos, meta, ypixalt, h, dist, x0,y0,z0, dx,dy,
                dz); */
            sp = sp-(double)
                (meta.attrweights[2U]*treesize*one((2000.0f-h)*0.001f));
         }
         if (sp>(double)tol) {
            /* over ground */
            if (sp>20.0) step = (float)(sp*0.25);
            else step = (float)sp;
            if (altrise<0.0f) step = step*2.0f;
            if (*subpix==0.0f) {
               if (sp<(double)lastsp) lastsp = (float)sp;
               else if (ypixalt!=0.0f) *subpix = X2C_DIVR(lastsp,ypixalt);
            }
         }
         else {
            /*WrFixed(sp, 1,6); WrFixed(lastsp, 1,6); WrFixed(minsp, 1,6);
                WrFixed(step, 1,6); WrStrLn(" go "); */
            if (sp>(double) -tol) break;
            /* under ground */
            if (spo<=0.0f) {
               /* again under ground */
               step = (float)sp;
            }
            else {
               /*WrStr(" back-back "); */
               --iter;
               if (iter==0UL) break;
               step = (float)((X2C_DIVL(sp,
                (double)spo-sp))*(double)step);
                /* first time under ground jump back with newton */
            }
         }
      }
      else if ((float)fabs(step)<tol) {
         /*WrFixed(step, 1,6); WrFixed(spo-sp, 1,7); WrInt(iter, 3);
                WrStrLn(" newton "); */
         /* jump over hole in erth */
         --iter;
         if (iter==0UL) break;
         sp = 0.0;
         if (step<30.0f) step = 30.0f;
      }
      spo = (float)sp;
      *dist = *dist+(double)step;
      if (*dist>(double)maxdist || alt>maxmountain) {
         *dist = (double)(maxdist+100.0f);
                /* we are in dust or heaven */
         break;
      }
   }
   /*
   IF dist<2.0 THEN
   WrStrLn(""); WrFixed(dist, 2,14); WrStr("=dist ");
   END;
   */
   /*  IF ypixalt<>0.0 THEN */
   /*    subpix:=minsp/ypixalt; */
   if (*subpix>1.0f) *subpix = 1.0f;
/*  END; */
/*IF subpix<>0.0 THEN WrFixed(subpix, 2,7); WrStr("=retsubpix ") END; */
} /* end raytrace() */
/* antialiasing: search for peaks and set highest pixel smooth */


static void rotvector(double * a, double * b, double cw,
                double sw)
{
   double h;
   h =  *a*cw+ *b*sw;
   *b =  *b*cw- *a*sw;
   *a = h;
} /* end rotvector() */


static void angtoxyz(double wx, double wy, double slat,
                double clat, double slong, double clong,
                double * x, double * y, double * z)
{
   *x = -sin(wy);
   *y = sin(wx)*cos(wy);
   *z = cos(wx)*cos(wy);
   rotvector(z, x, clat, slat);
   rotvector(y, x, clong, slong);
   *x = -*x;
} /* end angtoxyz() */

#define panorama_STEPS 8


static float sunvisability(double sunaz0, double sunel0,
                double slat, double clat, double slong,
                double clong, double x0, double y00,
                double z0, float maxdist, float refrac)
{
   double zn;
   double yn;
   double xn;
   double wy;
   double wx;
   double dd;
   float r;
   float sum;
   float space;
   struct POSITIONL pos;
   uint32_t i;
   r = 0.0f;
   sum = 0.0f;
   for (i = 0UL; i<=7UL; i++) {
      wx = sunaz0+(double)(2.3998277214236E-3f*osic_sin(r));
      wy = sunel0+(double)(2.3998277214236E-3f*osic_cos(r));
      angtoxyz(wx, wy, slat, clat, slong, clong, &xn, &yn, &zn);
      dd = 0.0;
      raytrace(x0, y00, z0, xn, yn, zn, maxdist, 0.0f, refrac, &dd, &space,
                &pos);
      /*WrFixed(sum, 2, 7); */
      if (dd>(double)maxdist) sum = sum+0.125f;
      r = r+7.85398163375E-1f;
   } /* end for */
   /*WrStrLn(" sums"); */
   return sum;
} /* end sunvisability() */


static void rotveclight(float * xsun, struct NORMVEC v, struct ROTVEC r)
{
   *xsun = (v.x*r.cx+v.y*r.sx)*r.cy+v.z*r.sy;
} /* end rotveclight() */


static void snowcol(float * slr, float * slg, float * slb,
                float sr, float sg, float sb, float xsun)
{
   float x;
   x = xsun+2.0f;
   *slr = 15.0f*x;
   *slg = 20.0f*x;
   *slb = 30.0f*x;
   if (xsun>0.0f) {
      /* surface sees sun */
      *slr = *slr+700.0f*xsun*sr;
      *slg = *slg+600.0f*xsun*sg;
      *slb = *slb+600.0f*xsun*sb;
   }
} /* end snowcol() */


static void botaniccol(float * slr, float * slg, float * slb,
                float sr, float sg, float sb, float xsun)
{
   float x;
   x = (xsun+2.5f)*(sr+sg+sb)*3.3333333E-1f;
   *slr = 0.1f*x;
   *slg = 0.12f*x;
   *slb = 0.2f*x;
   if (xsun>0.0f) {
      *slr = *slr+xsun*sr;
      *slg = *slg+xsun*sg;
      *slb = *slb+xsun*sb;
   }
} /* end botaniccol() */


static void storepix(imagetext_pIMAGE image0, uint32_t x, uint32_t y,
                float nr, float ng, float nb, float alias)
{
   struct imagetext_PIX * anonym;
   struct imagetext_PIX * anonym0;
   { /* with */
      struct imagetext_PIX * anonym = &image0->Adr[(x)*image0->Len0+y];
      anonym->r = (uint16_t)truncc(nr);
      anonym->g = (uint16_t)truncc(ng);
      anonym->b = (uint16_t)truncc(nb);
   }
   if (y>0UL) {
      { /* with */
         struct imagetext_PIX * anonym0 = &image0->Adr[(x)
                *image0->Len0+(y-1UL)];
         colblend(alias, (float)anonym0->r, (float)anonym0->g,
                (float)anonym0->b, nr, ng, nb, &nr, &ng, &nb);
                /* antialiasing */
         anonym0->r = (uint16_t)truncc(nr);
         anonym0->g = (uint16_t)truncc(ng);
         anonym0->b = (uint16_t)truncc(nb);
      }
   }
} /* end storepix() */


static void wateralias(float wy, float snow, float www,
                float wwr, float wwg, float wwb, float * lr,
                float * lg, float * lb)
{
   colblend(one((float)fabs(wy*2.5f))*0.9f, *lr, *lg, *lb,
                7.0f+snow*0.006f, 35.0f+snowalt*0.012f, 55.0f, lr, lg, lb);
   /*           10.0+(snowalt-oldh)*0.007,50.0+(snowalt-oldh)*0.015,70.0,
                (* water colour *) */
   colblend(www, wwr, wwg, wwb, *lr, *lg, *lb, lr, lg, lb);
                /* water aliasing */
} /* end wateralias() */

#define panorama_ERRALT 30000

#define panorama_FULLUM 1000.0
/* full bright in image */

#define panorama_MAXHP 0.08
/* vertical spatial luminance highpass */

#define panorama_DDIST 50.0

#define panorama_LIGHT90 0.35
/* luminance at 90 deg sun */

#define panorama_SELFSHADOW 0.6
/* dark/light of surface 90/180 deg to sun */

#define panorama_WAVLENX 5.E+6

#define panorama_WAVLENY 3.E+6


static void Panofind(uint32_t csvstep, const struct PANOWIN panpar,
                float * res, struct POSITIONL * pos)
{
   uint32_t treecnt;
   uint32_t yi;
   uint32_t xi;
   double z0m;
   double y0m;
   double x0m;
   double z0;
   double y00;
   double x0;
   double oldh;
   double refrac;
   double clat;
   double slat;
   double wy;
   double wx;
   double ddm;
   double dm;
   double dd;
   double azi;
   double zn;
   double yn;
   double xn;
   double clong;
   double slong;
   double ele0;
   double eled;
   double azid;
   double maxdist;
   double azi0;
   float street;
   float sunarea;
   float xsun;
   float slb;
   float slg;
   float slr;
   float treeclb;
   float www;
   float wwb;
   float wwg;
   float wwr;
   float space1;
   float bob;
   float bog;
   float bor;
   float sunb;
   float sung;
   float sunr;
   float ngb;
   float ngg;
   float ngr;
   float lb;
   float lg;
   float lr;
   float rnd3;
   float pixs;
   float noisw;
   float noisr;
   float resoltx;
   float wavelengthy;
   float wavelengthx;
   float wxx;
   float foga;
   float slant;
   float space;
   float fgb;
   float fgg;
   float fgr;
   float nsb;
   float nsg;
   float nsr;
   float altf;
   float tree;
   float llum;
   float lum;
   float lastlum;
   float dlum;
   float pixsize1;
   float pixsizew;
   float pixsize;
   float s2;
   float s0;
   float nn;
   char mirrord;
   char firsty;
   char heaven;
   uint8_t attu;
   uint8_t attw;
   uint8_t attr;
   struct ROTVEC sunvec;
   struct NORMVEC nvec;
   struct libsrtm_METAINFO meta;
   libsrtm_pMETAINFO pmeta;
   char tmp;
   pmeta = &meta;
   if ((!posvalid(panpar.eye) || !posvalid(panpar.horizon))
                || libsrtm_getsrtmlong((double)panpar.horizon.lat,
                (double)panpar.horizon.long0, 0UL, 1, &resoltx, &attr,
                0)>=30000.0f) Error("no altitude data at view point", 31ul);
   maxdist = (double)(aprspos_distance(panpar.eye,
                panpar.horizon)*1000.0f);
   if (maxdist<100.0) return;
   /* horizon too near */
   /*  
     hr:=FLOAT(heavencol.r)*(1000.0/256.0);
     hg:=FLOAT(heavencol.g)*(1000.0/256.0);
     hb:=FLOAT(heavencol.b)*(1000.0/256.0);
   */
   /*  nn:=getsrtm(panpar.eye, 0, resoltx);
                (* altitude of tx and srtm resolution in m here *) */
   nn = libsrtm_getsrtmlong((double)panpar.eye.lat,
                (double)panpar.eye.long0, 0UL, 1, &resoltx, &attr, 0);
                /* altitude of tx and srtm resolution in m here */
   if (nn>=30000.0f) {
      libsrtm_closesrtmfile();
      Error("no altitude data at camera position", 36ul);
   }
   wavelengthx = wavelength*5.E+6f;
   wavelengthy = wavelength*3.E+6f;
   azieletonvec(4.71238898025f-panpar.sunazi, panpar.sunele, &sunvec);
   wgs84sl((double)panpar.eye.lat, (double)panpar.eye.long0,
                (double)(nn+panpar.eyealt), &x0, &y00, &z0);
   azi = (double)(aprspos_azimuth(panpar.eye,
                panpar.horizon)*1.7453292519444E-2f);
   azi0 = (double)(panpar.angle0*0.5f*1.7453292519444E-2f);
   /*WrFixed(panpar.eye.lat*180.0/PI, 1, 7);WrStrLn(" lat"); */
   azid = (double)(X2C_DIVR(panpar.angle0*1.7453292519444E-2f,
                (float)((panpar.image0->Len1-1)+1UL)));
                /* h rad per pixel */
   /*WrFixed(azid*180.0/PI, 2, 7);WrStrLn(" deg/pixel"); */
   eled = X2C_DIVL(azid,(double)panpar.yzoom0);
   ele0 = (double)((panpar.elevation-X2C_DIVR((X2C_DIVR(panpar.angle0*0.5f,
                panpar.yzoom0))*(float)((panpar.image0->Len0-1)+1UL),
                (float)((panpar.image0->Len1-1)+1UL)))
                *1.7453292519444E-2f);
   slat = sin((double) -panpar.eye.lat);
   clat = cos((double) -panpar.eye.lat);
   slong = sin((double) -panpar.eye.long0);
   clong = cos((double) -panpar.eye.long0);
   xi = 0UL;
   refrac = (double)(panpar.refract*7.85E-8f);
   sunarea = sunvisability((double)panpar.sunazi,
                (double)panpar.sunele, slat, clat, slong, clong, x0,
                y00, z0, (float)maxdist, (float)refrac);
   if (verb) {
      osic_WrFixed((float)(X2C_DIVL(azi*180.0,3.1415926535)), 1L, 1UL);
      osi_WrStrLn(" cam azimuth", 13ul);
      osic_WrFixed(sunarea*100.0f, 1L, 1UL);
      osi_WrStrLn("% of sun visable", 17ul);
   }
   do {
      wx = azid*(double)xi-azi0;
      if (panpar.flatscreen0) wx = atan(wx);
      wxx = (float)(wx+azi);
      yi = 0UL;
      dd = 0.0;
      dlum = 0.0f;
      lastlum = 0.0f;
      heaven = 0;
      firsty = 0;
      mirrord = 0;
      treecnt = 0UL;
      treeclb = (-1000.0f);
      do {
         wx = (double)wxx;
         wy = ele0+eled*(double)yi;
         if (mirrord) {
            /* double beam deviation in sight direction */
            /*        wy:=-wy+waveamp*sin((pos.lat)*wavelengthx)
                +waveamp*0.6*sin(pos.long*wavelengthy); */
            /*        wx:= wx+0.5*(waveamp*sin((pos.long)*wavelengthx)
                +waveamp*0.6*sin(pos.lat*wavelengthy)); */
            wy = -wy+(double)waveamp*sin(pos->lat*(double)
                wavelengthx)+(double)(waveamp*0.5f)
                *sin(pos->lat*(double)wavelengthx*3.7)+(double)
                (waveamp*0.6f)*sin(pos->long0*(double)wavelengthy);
            wx = wx+0.5*((double)waveamp*sin(pos->long0*(double)
                wavelengthx)+(double)(0.4f*waveamp)
                *sin(pos->long0*(double)wavelengthx*2.9)+(double)
                (waveamp*0.6f)*sin(pos->lat*(double)wavelengthy));
         }
         if (wy>=(-1.5690509974981) && wy<1.5690509974981) {
            /*        IF NOT mirrord THEN space:=0.0 END;  */
            if (!heaven) {
               /*& (treeclb<=VAL(REAL, yi)+2.0)*/
               if (panpar.flatscreen0) wy = atan(wy);
               angtoxyz(wx, wy, slat, clat, slong, clong, &xn, &yn, &zn);
               if (mirrord) {
                  dm = 0.0;
                  raytrace(x0m, y0m, z0m, xn, yn, zn, (float)(maxdist-dd),
                 (float) -eled, (float)refrac, &dm, &space1, pos);
                  /*WrFixed(dm, 1,8); wrxyz(x0m,y0m,z0m);
                wrxyz(x0m+xn,y0m+yn,z0m+zn);WrFixed(pos.lat/RAD, 6,12);
                WrFixed(pos.long/RAD, 6,12);WrStrLn(""); */
                  /*            space:=1.0; */
                  ddm = dd+dm;
                  if (dm>maxdist-dd) heaven = 1;
               }
               else {
                  raytrace(x0, y00, z0, xn, yn, zn, (float)maxdist,
                (float)eled, (float)refrac, &dd, &space, pos);
                  ddm = dd;
                  if (dd>maxdist) heaven = 1;
               }
               if ((!mirrord && csvstep) && xi%csvstep==0UL) {
                  xycsv(xi, yi, *pos, heaven, firsty);
               }
            }
            if (ddm<1.0) return;
            /* under ground */
            if (heaven) {
               if (mirrord) {
                  ddm = maxdist*4.0*(0.25+X2C_DIVL((double)yi,
                (double)(panpar.image0->Len0-1)));
               }
               else {
                  /*            hc:=one((FLOAT(HIGH(panpar.image^[0]))
                -FLOAT(yi)-panpar.elevation*RAD/azid)*2.0/VAL(REAL,
                HIGH(panpar.image^[0]))-1.0); */
                  ddm = maxdist*4.0*(double)(1.25f-X2C_DIVR((float)
                yi,(float)(panpar.image0->Len0-1)));
               }
            }
            /*            hc:=one((FLOAT(yi)+panpar.elevation*RAD/azid)
                *2.0/VAL(REAL, HIGH(panpar.image^[0]))-1.0); */
            lum = (float)(ddm*0.2);
            if (!mirrord) {
               llum = (lum-lastlum)*20.0f;
               if ((double)llum>0.08*maxdist) {
                  llum = (float)(0.08*maxdist);
               }
               if (llum>dlum) dlum = llum;
               lastlum = lum; /* lum is distance */
            }
            if (!heaven) {
               if ((-1000.0f)<=(float)yi+2.0f) {
                  normvec(*pos, pmeta, &slant, &oldh, &nvec, &attr, &attw,
                &attu);
                  rotveclight(&xsun, nvec, sunvec);
                  /*slant:=slant**3.5; */
                  /*            slant:=nvec.z; */
                  /*            dir:=atang2(nvec.x,nvec.y);  */
                  /*WrFixed(nvec.x,3,6);WrFixed(nvec.y,3,6);
                WrFixed(nvec.z,3,6);WrStr(","); */
                  sundisk(wx, wy, (double)panpar.sunazi,
                (double)panpar.sunele, (-1.0f), 0.0f, 0, &sunr, &sung,
                &sunb);
                  pixs = one((float)((X2C_DIVL(ddm*(double)
                panpar.angle0,(double)(float)(panpar.image0->Len1-1)))*0.00125));
                 /* 1/pixelsize */
                  pixsize = (float)((X2C_DIVL((double)(float)
                (panpar.image0->Len1-1),
                ddm*(double)panpar.angle0))*5.729577951472E+1);
                  pixsize1 = one(pixsize);
                  pixsizew = one((float)((X2C_DIVL((double)
                (float)(panpar.image0->Len1-1),
                ddm*(double)panpar.angle0))*200.0));
                  snowcol(&slr, &slg, &slb, sunr, sung, sunb,
                xsun+(X2C_DIVR(noisw*plus(1.0f-pixs*10.0f),
                (float)fabs(xsun)+0.1f))*0.05f);
               }
               /*IF mirrord THEN WrFixed(dm, 1, 10);WrStr("/");
                WrFixed(oldh, 1, 1); END; */
               /*          sloped:=panpar.angle*ddm/900000.0; */
               /*          IF sloped>1.0 THEN sloped:=1.0 END; */
               tree = (float)(1600.0-oldh);
               if (tree<0.0f) tree = 0.0f;
               else if (tree>600.0f) tree = 600.0f;
               /*        tree:=tree*(0.95/600.0)*(1.0-d/maxdist); */
               tree = tree*1.6666666666667E-3f;
               /*WrFixed(pixsize, 3, 10); WrFixed(ddm, 3, 10); WrStrLn("");
                */
               randc(yi, &noisw, &noisr, &rnd3);
               if (!mirrord) randstep(yi);
               /*          s1:=slant**3; */
               s0 = (float)pow((double)(1.0f-slant), 0.1);
               s2 = (float)(X2C_DIVL((oldh-(double)((float)
                attw*5.0f))-(double)snowalt,
                (double)slowaltslant)+(double)
                (noisw*1.0f*(1.25f-slant)));
               s2 = one((s2+slant*slant)*slowaltslant*0.005f+0.5f);
               s2 = s2*one((float)(((pow((double)slant,
                0.5)-0.55)+(double)(noisw*1.0f))*5.0));
                /* never snow on big slant */
               /*IF attw>50 THEN s2:=0.0 END; */
               /*          ldir:=dir+panpar.sunazi + 90*RAD; */
               /*          sdir:=radmod(ldir + noisw*plus(1.0-pixs*10.0)*1.0)
                ; */
               /*          ldir:=radmod(ldir + noisw*(1.0-pixs*0.8)*1.0); */
               /*          IF ABS(ldir)>PI*0.5 THEN light:=(PI-ABS(ldir))
                *(LIGHT90*SELFSHADOW/(PI*0.5))+LIGHT90*(1.0-SELFSHADOW); */
               /*          ELSE light:=LIGHT90+cos(ldir)*(1.0-LIGHT90) END;
                */
               /*
               IF NOT mirrord & (attw>=90) & (pixsize<1.0) & (s2<0.5)
                & (treeclb<=VAL(REAL, yi)) THEN
                 treelen:=35.0*one((2000.0-oldh)*0.001);
               --  IF treelen>2.0 THEN treeclb:=VAL(REAL,
                yi)+(((Random()-0.5)*0.45*(1.1-pixsize1)+0.75)
                *treelen*FLOAT(attw)*0.01)*pixsize END;
                 IF treelen>2.0 THEN treeclb:=(((Random()-0.5)*0.5+0.5)
                *treelen*FLOAT(attw)*0.01)*pixsize ELSE treeclb:=0.0 END;
                 IF treeclb>2.0 THEN treeclb:=treeclb+VAL(REAL,
                yi) ELSE treeclb:=0.0 END;
               END;
               */
               /*
               IF treeclb>VAL(REAL, yi) THEN
                attw:=TRUNC(100.0*one((treeclb-VAL(REAL, yi))*4.0));
                          (* fade out tree *)
                attr:=attr*(100-attw) DIV 100;
                attu:=attu*(100-attw) DIV 100;
                s2:=s2*FLOAT(100-attw)*0.01;
                          (* fade out tree to snow *)
               END;
               */
               /*flatamp:=s0*FLOAT(100-attu)*FLOAT(150-attw)
                *(1.0/(100.0*150.0));
                (* sun angle dependency per attribut *) */
               /*light:=light*flatamp + (1.0-flatamp)*LIGHT90;
                (* dampen azimuth effect on flat *) */
               /*          slight:=slight*s0 + (1.0-s0)*0.1; */
               /*          s1:=0.25+s0*(0.1*cos(ldir)-0.1); */
               /*          slight:=slight*s0+(1.0-s0)*s1; */
               altf = one(plus((float)((oldh-1500.0)*0.0025))
                +plus(1.0f-plus(((slant-0.4f)-(float)(100U-attw)*0.002f)
                +noisr*1.0f)*botanicslant)); /* rock / green */
               /*IF attw>50 THEN altf:=0.0 END;
                (* wood on any slant if wood attr *) */
               /* wood */
               colblend((float)(Random()*3.25-3.0), 25.0f,
                35.0f*(0.75f+one(0.5f+noisr*8.0f*pixsizew)), 20.0f, 25.0f,
                12.0f, 5.0f, &fgr, &fgg, &fgb);
               /*colblend(slant, 500.0, 0.0, 0.0, 0.0, 500.0, 0.0, fgr,fgg,
                fgb); */
               /* meadow with snow destruction */
               colblend(one((float)((X2C_DIVL((oldh-(double)snowalt)
                +(double)slowaltslant,
                (double)slowaltslant))*4.0)), 45.0f, 100.0f, 30.0f,
                70.0f, 75.0f, 45.0f, &ngr, &ngg, &ngb);
               /* colour variance meadow */
               ngr = (float)((double)
                (ngr*plus(1.0f-rnd3*1.2f*pixsize1))*(1.0+oldh*0.0003));
               ngg = ngg*plus(1.0f+noisr*1.2f*pixsize1);
               ngb = ngb*plus((float)(1.0-oldh*0.0003));
               /* wood/meadow */
               /*          colblend(one(one(1.0-pixsize1*0.25)*FLOAT(attw)
                *0.01), ngr,ngg,ngb, fgr,fgg,fgb, nsr,nsg,nsb); */
               colblend(one((float)attw-50.0f), ngr, ngg, ngb, fgr, fgg,
                fgb, &nsr, &nsg, &nsb);
               /* green/rock */
               if (altf>0.0f) {
                  /* else save time */
                  if (desertcol.r<0.0f) {
                     bor = 130.0f*(0.2f+plus(noisr*2.5f)
                +one(0.8f+rnd3*pixsizew*3.0f));
                     bog = 120.0f*(0.2f+one(0.8f+rnd3*pixsizew*4.0f));
                     bob = 100.0f*(0.2f+one(0.8f+rnd3*pixsizew*3.0f));
                  }
                  else {
                     bor = desertcol.r;
                     bog = desertcol.g;
                     bob = desertcol.b;
                  }
                  colblend(altf, nsr, nsg, nsb, bor, bog, bob, &nsr, &nsg,
                &nsb);
               }
               /* city */
               if (attu>0U) {
                  /* else save time */
                  street = 1.0f;
                  if (frac((float)(pos->lat*1.7E+5))
                <0.3f || frac((float)(pos->long0*2.000009E+5))<0.3f) {
                     street = 0.0f;
                  }
                  colblend((float)attu*0.01f*one(4.0f-pixsize1*2.5f), nsr,
                 nsg, nsb, 150.0f*(0.2f+street*one(rnd3*50.0f-0.25f)),
                150.0f*(0.25f+street*one(rnd3*50.0f-1.0f)),
                150.0f*(0.25f+street*one(rnd3*50.0f-1.0f)), &nsr, &nsg,
                &nsb);
               }
               /*flatamp:=FLOAT(100-attu)*FLOAT(150-attw)*(1.0/(100.0*150.0))
                ;  (* sun angle dependency per attribut *) */
               /*flatamp:=FLOAT(130-attu)*FLOAT(400-VAL(CARDINAL,
                attw))*(1.0/(130.0*400.0));
                (* sun angle dependency per attribut *) */
               /*            botaniccol(bor,bog,bob,sunr,sung,sunb,
                xsun*flatamp+noisw*(1.0-pixs*0.8)/(ABS(xsun)+0.1)*0.05); */
               /*            botaniccol(bor,bog,bob,sunr,sung,sunb,
                xsun*flatamp+noisw*(1.0-pixs*0.8)*0.5); */
               botaniccol(&bor, &bog, &bob, sunr, sung, sunb,
                xsun+noisw*(1.0f-pixs*0.8f)*0.5f);
               colblend(s2, bor*nsr, bog*nsg, bob*nsb, slr, slg, slb, &lr,
                &lg, &lb);
               /*IF treeclb>VAL(REAL, yi) THEN lr:=fgr; lg:=fgg; lb:=fgb END;
                 */
               /* lake */
               if (((flatwater && ((uint32_t)attr>10UL+(uint32_t)
                X2C_TRUNCC(pixsize1*40.0f,0UL,
                X2C_max_longcard) || waterslant<0.0f)) && !mirrord)
                && slant>(float)fabs(waterslant)) {
                  /* & (flatamp<0.6)*/
                  wgs84sl(pos->lat, pos->long0, oldh+1.0, &x0m, &y0m, &z0m);
                  dm = 0.0;
                  mirrord = 1;
                  /*-            wwr:=lr; wwg:=lg;wwb:=lb; */
                  if (waterslant<0.0f) www = 1.0f;
                  else www = (float)attr*0.01f;
                  colblend((one((float)(2.0+wy*12.0))+1.0f)-pixsize1,
                130.0f*plus(1.0f-rnd3*1.8f*pixsize1),
                120.0f*plus(1.0f+noisr*1.8f*pixsize1), 55.0f, lr, lg, lb,
                &wwr, &wwg, &wwb);
               }
               else if (mirrord) {
                  /* underwater/coast colour*/
                  wateralias((float)wy,
                (float)((double)snowalt-oldh), www, wwr, wwg, wwb,
                &lr, &lg, &lb);
                  /*lr:=500.0; lg:=0.0; lb:=0.0; */
                  /*
                                colblend(one(ABS(wy*3.5))*0.90, lr,lg,lb,
                  --                       10.0+(snowalt-oldh)*0.007,
                50.0+(snowalt-oldh)*0.015,70.0,
                                         7.0+(snowalt-oldh)*0.006,
                35.0+(snowalt-oldh)*0.012,55.0,
                                         lr,lg,lb);
                                colblend(www, wwr,wwg,wwb, lr,lg,lb, lr,lg,
                lb);                   (* water aliasing *)
                  */
                  mirrord = 0;
               }
               /*fogl:=FOGLUM (*+oldh*0.05*); */
               /*foga:=1.0-oldh*0.0001; */
               /*IF foga<0.2 THEN foga:=0.2 END; */
               /*foga:=1.1-ddm/maxdist (*foga*); */
               /*IF foga>1.0 THEN foga:=1.0 END; */
               /*IF foga<0.2 THEN foga:=0.2 END; */
               /*          colblend(foga, fogl,fogl,fogl, lr,lg,lb, lr,lg,
                lb); */
               sundisk(wx, wy, (double)panpar.sunazi,
                (double)panpar.sunele, one((float)(X2C_DIVL(ddm,
                maxdist)-0.1))+0.01f, sunarea, 0, &lr, &lg, &lb);
               /*egde fog */
               foga = (float)((double)dlum*(1.0-X2C_DIVL(ddm,
                maxdist))*(double)dusthighpass*cos(wy));
               lr = lr+foga;
               lg = lg+foga;
               lb = lb+foga;
            }
            else {
               /* heaven */
               /*
               fogl:=FOGLUM;
                         colblend(hc, fogl,fogl,fogl, hr,hg,hb, lr,lg,lb);
               */
               sundisk(wx, wy, (double)panpar.sunazi,
                (double)panpar.sunele, 0.0f, sunarea, 1, &lr, &lg,
                &lb);
               if (mirrord) {
                  wateralias((float)wy,
                (float)((double)snowalt-oldh), www, wwr, wwg, wwb,
                &lr, &lg, &lb);
                  /*
                              colblend(one(0.1+ABS(wy*4.0))*0.9, lr,lg,lb,
                                       10.0+(snowalt-oldh)*0.007,
                50.0+(snowalt-oldh)*0.015,70.0, (* water colour *)
                                       lr,lg,lb);
                                   (* reflection angle function *)
                              colblend(www, wwr,wwg,wwb, lr,lg,lb, lr,lg,lb);
                                   (* water aliasing *)
                  */
                  dlum = 0.0f;
                  heaven = 0;
                  mirrord = 0;
               }
            }
            if (!mirrord) {
               storepix(panpar.image0, xi, yi, lr, lg, lb, space);
               dlum = (float)((double)dlum*((1.0-(X2C_DIVL(ddm,
                maxdist))*0.2)-X2C_DIVL(120.0,
                (double)(panpar.image0->Len0-1))));
                /* fade out distance highpass peak */
            }
         }
         if (!mirrord) {
            ++yi;
            firsty = 1;
         }
      } while (yi<=panpar.image0->Len0-1);
      if (verb) {
         osic_WrINT32(xi, 5UL);
         osi_WrStr((char *)(tmp = '\015',&tmp), 1u/1u);
         osic_flush();
      }
      /* WrFixed(pos.lat*180.0/pi, 4,8); WrFixed(pos.long*180.0/pi, 4,8);
                WrStrLn(" xi lat long az"); */
      ++xi;
   } while (xi<=panpar.image0->Len1-1);
} /* end Panofind() */


static void Panorama1(const struct PANOWIN panpar)
{
   float res;
   struct POSITIONL pos;
   Panofind(rastercsv, panpar, &res, &pos);
} /* end Panorama1() */

/* ------- poi resverse raytrace */

static char drawpoiicon(float xi, float yi,
                struct PANOWIN panpar, const SYMBOL sym, uint32_t xs,
                uint32_t ys, uint32_t cr, uint32_t cg, uint32_t cb,
                uint32_t calpha)
{
   uint32_t ar;
   uint32_t a;
   uint32_t iyr;
   uint32_t ix;
   uint32_t iy;
   uint32_t y;
   uint32_t x;
   char ok0;
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   ok0 = 0;
   if (xi<=0.0f || yi<=0.0f) return 0;
   x = (uint32_t)X2C_TRUNCC(xi+0.5f,0UL,X2C_max_longcard);
   if (x>=xs && x+xs<=panpar.image0->Len1-1) {
      x = (uint32_t)X2C_TRUNCC((xi+0.5f)-(float)xs*0.5f,0UL,
                X2C_max_longcard);
      y = (uint32_t)X2C_TRUNCC(yi+0.5f,0UL,X2C_max_longcard);
      tmp = ys-1UL;
      iy = 0UL;
      if (iy<=tmp) for (;; iy++) {
         iyr = (ys-iy)-1UL;
         if (iy+y<=panpar.image0->Len0-1) {
            tmp0 = xs-1UL;
            ix = 0UL;
            if (ix<=tmp0) for (;; ix++) {
               { /* with */
                  struct imagetext_PIX * anonym = &panpar.image0->Adr[(x+ix)
                *panpar.image0->Len0+(y+iy)];
                  a = ((uint32_t)sym[iyr][ix].alpha*calpha)/256UL;
                  ar = 255UL-a;
                  /*WrInt(sym[iyr][ix].alpha, 4); */
                  anonym->r = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].r8*cr)/64UL+ar*(uint32_t)anonym->r)/255UL);
                  anonym->g = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].g8*cg)/64UL+ar*(uint32_t)anonym->g)/255UL);
                  anonym->b = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].b8*cb)/64UL+ar*(uint32_t)anonym->b)/255UL);
               }
               if (ix==tmp0) break;
            } /* end for */
         }
         if (iy==tmp) break;
      } /* end for */
      ok0 = 1;
   }
   return ok0;
} /* end drawpoiicon() */


static char drawpoi(float xi, float yi, struct PANOWIN panpar,
                uint32_t size, int32_t cr, int32_t cg, int32_t cb,
                int32_t calpha)
{
   uint32_t ix;
   uint32_t iy;
   uint32_t y;
   uint32_t x;
   int32_t n;
   int32_t s;
   int32_t c;
   int32_t a;
   char ok0;
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   ok0 = 0;
   if (xi<=0.0f || yi<=0.0f) return 0;
   x = (uint32_t)X2C_TRUNCC(xi+0.5f,0UL,X2C_max_longcard);
   y = (uint32_t)X2C_TRUNCC(yi+0.5f,0UL,X2C_max_longcard);
   if (x>=size && x+size<=panpar.image0->Len1-1) {
      tmp = size*2UL-1UL;
      iy = 0UL;
      if (iy<=tmp) for (;; iy++) {
         if (iy+y<=panpar.image0->Len0-1) {
            tmp0 = size*2UL-1UL;
            ix = 0UL;
            if (ix<=tmp0) for (;; ix++) {
               a = labs((int32_t)ix-(int32_t)size);
               c = 0L;
               a = (int32_t)iy-2L*a;
               if (a==-1L) c = 256L;
               else if (a==0L) c = 768L;
               else if (a>0L) c = 1024L;
               if (c>0L) {
                  s = 0L;
                  if (c<1000L) {
                     if (ix<size) {
                        if (iy!=size*2UL-1UL) s = 200L;
                     }
                     else s = -400L;
                  }
                  else if (iy==size*2UL-1UL) {
                     c = 500L;
                     s = -300L;
                  }
                  c = (c*calpha)/256L;
                  { /* with */
                     struct imagetext_PIX * anonym = &panpar.image0->Adr[((x+ix)
                -size)*panpar.image0->Len0+(y+iy)];
                     n = ((1024L-c)*(int32_t)anonym->r)/1024L+(c*cr)
                /256L+s;
                     if (n<0L) anonym->r = 0U;
                     else anonym->r = (uint16_t)n;
                     n = ((1024L-c)*(int32_t)anonym->g)/1024L+(c*cg)
                /256L+s;
                     if (n<0L) anonym->g = 0U;
                     else anonym->g = (uint16_t)n;
                     n = ((1024L-c)*(int32_t)anonym->b)/1024L+(c*cb)
                /256L+s;
                     if (n<0L) anonym->b = 0U;
                     else anonym->b = (uint16_t)n;
                  }
               }
               if (ix==tmp0) break;
            } /* end for */
            ok0 = 1;
         }
         if (iy==tmp) break;
      } /* end for */
   }
   return ok0;
} /* end drawpoi() */


static void poipixel(struct aprsstr_POSITION ppos, float palt,
                const struct PANOWIN panpar, int32_t * px, int32_t * py,
                pPOI pp)
{
   float refrac;
   float yi;
   float xi;
   float d2;
   float aeye;
   float nnp;
   float nne;
   float subpix;
   float eled;
   float ele0;
   float azi;
   float azi0;
   float azid;
   float resoltx;
   double zn;
   double yn;
   double xn;
   double z1;
   double y1;
   double x1;
   double z0;
   double y00;
   double x0;
   double rd;
   uint8_t attr;
   struct POSITIONL rpos;
   char ok0;
   struct POI * anonym;
   *px = -1L;
   *py = -1L;
   if (!posvalid(ppos)) return;
   refrac = panpar.refract*7.85E-8f;
   nne = libsrtm_getsrtmlong((double)panpar.eye.lat,
                (double)panpar.eye.long0, 0UL, 1, &resoltx, &attr, 0);
   nnp = libsrtm_getsrtmlong((double)ppos.lat,
                (double)ppos.long0, 0UL, 1, &resoltx, &attr, 0);
   if (altOG) {
      if (palt<minpoialt) palt = minpoialt;
      nnp = nnp+palt;
   }
   else {
      nnp = nnp+minpoialt;
      if (nnp<palt) nnp = palt;
   }
   aeye = nne+panpar.eyealt;
   wgs84sl((double)panpar.eye.lat, (double)panpar.eye.long0,
                (double)aeye, &x0, &y00, &z0);
   wgs84sl((double)ppos.lat, (double)ppos.long0,
                (double)nnp, &x1, &y1, &z1);
   d2 = (float)sqrt((double)(sqr((float)(x1-x0))+sqr((float)
                (y1-y00))+sqr((float)(z1-z0))));
   wgs84sl((double)ppos.lat, (double)ppos.long0,
                (double)(nnp+refrac*d2*d2), &x1, &y1, &z1);
   d2 = (float)sqrt((double)(sqr((float)(x1-x0))+sqr((float)
                (y1-y00))+sqr((float)(z1-z0))));
   xn = X2C_DIVL(x1-x0,(double)d2);
   yn = X2C_DIVL(y1-y00,(double)d2);
   zn = X2C_DIVL(z1-z0,(double)d2);
   rd = 0.0;
   raytrace(x0, y00, z0, xn, yn, zn, d2, 0.0f, refrac, &rd, &subpix, &rpos);
   /*WrFixed(d2, 1, 8); WrFixed(rd, 1, 8); WrStrLn("=d,rd"); */
   if (rd+30.0>=(double)d2) {
      /* poi visable */
      xn = -xn;
      /*WrFixed(xn,2,6);WrFixed(yn,2,6);WrFixed(zn,2,6); WrStrLn(" xyz1"); */
      rotvector(&yn, &xn, cos((double)panpar.eye.long0),
                sin((double)panpar.eye.long0));
      rotvector(&zn, &xn, cos((double)panpar.eye.lat),
                sin((double)panpar.eye.lat));
      /*WrFixed(xn,2,6);WrFixed(yn,2,6);WrFixed(zn,2,6); WrStrLn(" xyz2"); */
      azi = aprspos_azimuth(panpar.eye, panpar.horizon)*1.7453292519444E-2f;
      azi0 = panpar.angle0*0.5f*1.7453292519444E-2f;
      azid = X2C_DIVR(panpar.angle0*1.7453292519444E-2f,
                (float)((panpar.image0->Len1-1)+1UL));
                /* h rad per pixel */
      xi = atang2((float)zn, (float)yn)-azi;
      /*WrFixed(xi*180.0/PI,2,8); WrStr("=xi "); */
      if (xi<(-3.1415926535f)) xi = xi+6.283185307f;
      else if (xi>3.1415926535f) xi = xi-6.283185307f;
      if (panpar.flatscreen0) xi = osic_tan(xi);
      /*WrFixed(xi*180.0/PI,2,8); WrStr("=axi "); */
      xi = X2C_DIVR(xi+azi0,azid);
      /*WrFixed(xi,1,8); */
      eled = X2C_DIVR(azid,panpar.yzoom0); /* v deg per pixel */
      ele0 = (panpar.elevation-X2C_DIVR((X2C_DIVR(panpar.angle0*0.5f,
                panpar.yzoom0))*(float)((panpar.image0->Len0-1)+1UL),
                (float)((panpar.image0->Len1-1)+1UL)))
                *1.7453292519444E-2f;
      yi = atang2((float)sqrt(zn*zn+yn*yn), (float) -xn);
      if (panpar.flatscreen0) yi = (float)tan((double)yi);
      yi = X2C_DIVR(yi-ele0,eled);
      /*WrFixed(yi,1,8); WrStrLn(""); */
      { /* with */
         struct POI * anonym = pp;
         if (anonym->xs==0UL) {
            ok0 = drawpoi(xi, yi, panpar, poisize, (int32_t)anonym->r,
                (int32_t)anonym->g, (int32_t)anonym->b,
                (int32_t)anonym->alpha);
         }
         else {
            ok0 = drawpoiicon(xi, yi, panpar, anonym->symbol, anonym->xs,
                anonym->ys, anonym->r, anonym->g, anonym->b, anonym->alpha);
         }
      }
      if (ok0) {
         *px = (int32_t)X2C_TRUNCI(xi+0.5f,X2C_min_longint,
                X2C_max_longint);
         *py = (int32_t)X2C_TRUNCI(yi+0.5f,X2C_min_longint,
                X2C_max_longint);
      }
   }
} /* end poipixel() */


static void setpoi(const struct PANOWIN panpar,
                const struct aprsstr_POSITION posdeg, float alti,
                const char text[], uint32_t text_len, pPOI pp)
{
   float resol;
   float alt;
   float d;
   float az;
   struct aprsstr_POSITION pos;
   uint8_t attr;
   int32_t py;
   int32_t px;
   pos.lat = posdeg.lat*1.7453292519444E-2f;
   pos.long0 = posdeg.long0*1.7453292519444E-2f;
   /*WrFixed(posdeg.lat,2,9); WrStr(" "); */
   /*WrFixed(posdeg.long,2,9); WrStr(" "); */
   d = aprspos_distance(panpar.eye, pos);
   if (d>panpar.dist || d<0.05f) return;
   az = panpar.ang-aprspos_azimuth(panpar.eye, pos)*1.7453292519444E-2f;
   if (az<(-3.1415926535f)) az = az+6.283185307f;
   else if (az>3.1415926535f) az = az-6.283185307f;
   if ((float)fabs(az)>panpar.hseg) return;
   poipixel(pos, alti, panpar, &px, &py, pp);
   if (verb) {
      osic_WrFixed(aprspos_distance(panpar.eye, pos), 3L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(aprspos_azimuth(panpar.eye, pos), 2L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(X2C_DIVR(az,1.7453292519444E-2f), 2L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(alti, 0L, 9UL);
      osi_WrStr(" ", 2ul);
      osi_WrStr(text, text_len);
      if (px<0L) {
         osi_WrStr(" #", 3ul);
      }
      osi_WrStrLn("", 1ul);
   }
   if (px>=0L) {
      alt = libsrtm_getsrtmlong((double)pos.lat,
                (double)pos.long0, 0UL, 1, &resol, &attr, 0);
      if (alti>=10000.0f || alti<alt) alti = alt;
      wrcsv(px, py, posdeg, alti, text, text_len, pp->iconhint, 1024ul);
   }
} /* end setpoi() */

/* ------- poi resverse raytrace */
#define panorama_POIINFOSIZE 4096


static char getch(char b[4096], int32_t fd, int32_t * len,
                int32_t * p)
{
   if (*p>=*len) {
      *len = osi_RdBin(fd, (char *)b, 4096u/1u, 4096UL);
      if (*len<=0L) return 0;
      *p = 0L;
   }
   ++*p;
   return b[*p-1L];
} /* end getch() */


static int32_t getword(int32_t * p, int32_t * len, int32_t fd,
                char b[4096], char s[], uint32_t s_len)
{
   uint32_t i;
   char inqu;
   i = 0UL;
   inqu = 0;
   for (;;) {
      s[i] = getch(b, fd, len, p);
      if (s[i]==0) return -1L;
      if (s[i]=='\012') {
         s[i] = 0;
         return 0L;
      }
      if (!inqu && s[i]==',') {
         s[i] = 0;
         return 1L;
      }
      if (s[i]=='\"') inqu = !inqu;
      else if ((i<s_len-1 && (uint8_t)s[i]>=' ') && (uint8_t)
                s[i]<(uint8_t)'\200') ++i;
   }
   return 0;
} /* end getword() */


static void rdmountain(const struct PANOWIN panpar, char fn[],
                uint32_t fn_len, pPOI pp)
/* import csv file with mountain name, pos, altitude */
{
   int32_t r;
   int32_t len;
   int32_t p;
   int32_t fd;
   struct aprsstr_POSITION pos;
   float alt;
   char b[4096];
   char s[1024];
   char text[4097];
   char name[100];
   char long0[100];
   char lat[100];
   char com[100];
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenRead(fn, fn_len);
   if (!osic_FdValid(fd)) Error("poi-file not found", 19ul);
   p = 0L;
   len = 0L;
   for (;;) {
      r = getword(&p, &len, fd, b, com, 100ul);
      if (r>0L) {
         r = getword(&p, &len, fd, b, name, 100ul);
         if (r>0L) {
            r = getword(&p, &len, fd, b, text, 4097ul);
            if (r>0L) {
               r = getword(&p, &len, fd, b, lat, 100ul);
               if (r>0L) {
                  r = getword(&p, &len, fd, b, long0, 100ul);
                  if (r>0L) {
                     r = getword(&p, &len, fd, b, s, 1024ul);
                     if (r<0L || !aprsstr_StrToFix(&alt, s, 1024ul)) {
                        alt = (-3.E+4f);
                     }
                     while (r>0L) r = getword(&p, &len, fd, b, s, 1024ul);
                  }
                  else alt = 0.0f;
               }
            }
         }
      }
      if (r<0L) break;
      if ((((com[0U]!='#' && name[0U]) && aprsstr_StrToFix(&pos.lat, lat,
                100ul)) && aprsstr_StrToFix(&pos.long0, long0,
                100ul)) && posvalid(pos)) {
         setpoi(panpar, pos, alt, name, 100ul, pp);
      }
   }
   osic_Close(fd);
   X2C_PFREE(fn);
} /* end rdmountain() */


static void rdmountains(const struct PANOWIN panpar)
{
   pPOI pp;
   pp = poifiles;
   while (pp) {
      pp->xs = 0UL;
      pp->ys = 0UL;
      if (pp->iconfn[0U]) {
         readsymbol(pp->symbol, &pp->xs, &pp->ys, pp->iconfn, 1024ul);
      }
      rdmountain(panpar, pp->poifn, 1024ul, pp);
      pp = pp->next;
   }
} /* end rdmountains() */


static void drawscale(const struct PANOWIN panpar, uint32_t px,
                uint32_t deg)
{
   uint32_t y;
   char s[101];
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   if (px>2UL && px+2UL<=panpar.image0->Len1-1) {
      tmp = scalesize/10UL;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         { /* with */
            struct imagetext_PIX * anonym = &panpar.image0->Adr[(px)
                *panpar.image0->Len0+y];
            anonym->r = anonym->r+500U;
            anonym->g = anonym->g+500U;
         }
         if (y==tmp) break;
      } /* end for */
      if (px+scalesize<=panpar.image0->Len1-1) {
         if (deg==0UL) strncpy(s,"N",101u);
         else if (deg==90UL) strncpy(s,"E",101u);
         else if (deg==180UL) strncpy(s,"S",101u);
         else if (deg==270UL) strncpy(s,"W",101u);
         else aprsstr_IntToStr((int32_t)deg, 0UL, s, 101ul);
         imagetext_writestr(panpar.image0, px+2UL, 0UL, fonttyp, 200UL,
                200UL, 200UL, s, 101ul);
      }
   }
} /* end drawscale() */


static void WrLatLong(const struct PANOWIN panpar)
{
   uint32_t y;
   char h[100];
   char s[100];
   y = imagetext_fontsizey(fonttyp);
   aprsstr_FixToStr(X2C_DIVR(panpar.eye.lat,1.7453292519444E-2f), 6UL, s,
                100ul);
   aprsstr_FixToStr(X2C_DIVR(panpar.eye.long0,1.7453292519444E-2f), 6UL, h,
                100ul);
   aprsstr_Append(s, 100ul, " ", 2ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_FixToStr(panpar.dist, 2UL, h, 100ul);
   aprsstr_Append(s, 100ul, " ", 2ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_FixToStr(panpar.elevation, 2UL, h, 100ul);
   aprsstr_Append(s, 100ul, " ", 2ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_FixToStr(X2C_DIVR(panpar.sunazi,1.7453292519444E-2f), 2UL, h,
                100ul);
   aprsstr_Append(s, 100ul, " ", 2ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_FixToStr(X2C_DIVR(panpar.sunele,1.7453292519444E-2f), 2UL, h,
                100ul);
   aprsstr_Append(s, 100ul, " ", 2ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   imagetext_writestr(panpar.image0, 5UL, y, fonttyp, 120UL, 120UL, 120UL, s,
                 100ul);
} /* end WrLatLong() */


static void scale(const struct PANOWIN panpar)
{
   uint32_t i;
   uint32_t w;
   uint32_t d;
   uint32_t b;
   uint32_t a;
   float wx;
   float azid;
   float azi;
   uint32_t tmp;
   azid = X2C_DIVR(panpar.angle0,(float)((panpar.image0->Len1-1)+1UL));
                /* deg / pixel */
   d = (uint32_t)X2C_TRUNCC((float)scalesize*azid,0UL,X2C_max_longcard);
   a = 1UL;
   b = 1UL;
   while (a*b<d) {
      if (a==1UL) a = 2UL;
      else if (a==2UL) a = 5UL;
      else if (a==5UL) {
         a = 1UL;
         b = b*10UL;
      }
   }
   a = a*b; /* 1 2 5 10 deg / step */
   if (a>90UL) a = 180UL;
   else if (a>60UL) a = 90UL;
   else if (a>20UL) a = 60UL;
   /*WrInt(a, 10); WrStrLn(" deg/ step"); */
   azi = aprspos_azimuth(panpar.eye, panpar.horizon);
   wx = azi-panpar.angle0*0.5f;
   if (wx<0.0f) wx = wx+360.0f;
   w = ((uint32_t)X2C_TRUNCC(wx,0UL,X2C_max_longcard)/a)*a;
   tmp = ((uint32_t)X2C_TRUNCC(panpar.angle0,0UL,X2C_max_longcard)+a)/a;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /*WrInt(w, 4); */
      wx = ((float)w-azi)*1.7453292519444E-2f;
      if (wx<(-3.1415926535f)) wx = wx+6.283185307f;
      else if (wx>3.1415926535f) wx = wx-6.283185307f;
      if (panpar.flatscreen0) wx = (float)tan((double)wx);
      wx = X2C_DIVR(wx*5.729577951472E+1f+panpar.angle0*0.5f,azid);
      if (wx>0.0f) {
         drawscale(panpar, (uint32_t)X2C_TRUNCC(wx+0.5f,0UL,
                X2C_max_longcard), w);
      }
      /*WrFixed(wx,1, 7); */
      w += a;
      if (w>=360UL) w -= 360UL;
      if (i==tmp) break;
   } /* end for */
   WrLatLong(panpar);
/*WrStrLn(""); */
} /* end scale() */


static void drawimage(void)
{
   struct PANOWIN panowin;
   /*sym:SYMBOL; */
   /*readsymbol(sym, "/tmp/s1.png"); */
   panowin.image0 = image;
   panowin.flatscreen0 = flatscreen;
   panowin.eye = posa;
   panowin.horizon = posb;
   panowin.angle0 = angle;
   panowin.elevation = elev;
   panowin.eyealt = alta;
   panowin.yzoom0 = yzoom;
   panowin.refract = refraction;
   panowin.sunazi = sunaz*1.7453292519444E-2f;
   panowin.sunele = sunel*1.7453292519444E-2f;
   panowin.ang = aprspos_azimuth(panowin.eye,
                panowin.horizon)*1.7453292519444E-2f;
   panowin.hseg = angle*1.7453292519444E-2f*0.5f;
   panowin.dist = aprspos_distance(panowin.eye, panowin.horizon);
   if (imagefn[0U]) Panorama1(panowin);
   if (poifiles) rdmountains(panowin);
   if (fonttyp>0UL) scale(panowin);
} /* end drawimage() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[2];
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   imagetext_BEGIN();
   aprsstr_BEGIN();
   aprspos_BEGIN();
   libsrtm_BEGIN();
   osi_BEGIN();
   imagefn[0] = 0;
   poifiles = 0;
   csvfn[0] = 0;
   csvfd = -1L;
   image = 0;
   xsize = 600L;
   ysize = 400L;
   posinval(&posa);
   posinval(&posb);
   alta = 10.0f;
   refraction = 0.13f;
   igamma = 2.2f;
   libsrtm_srtmmaxmem = 100000000UL;
   heavencol.r = 120UL;
   heavencol.g = 180UL;
   heavencol.b = 800UL;
   elev = 0.0f;
   angle = 45.0f;
   yzoom = 1.0f;
   flatscreen = 0;
   poisize = 5UL;
   minpoialt = 1.0f;
   fonttyp = 0UL;
   altOG = 0;
   verb = 0;
   /*  bicubic:=FALSE; */
   rastercsv = 0UL;
   sunaz = 180.0f;
   sunel = 30.0f;
   snowalt = 3000.0f;
   slowaltslant = 500.0f;
   flatwater = 0;
   waveamp = 0.006f;
   dusthighpass = 0.002f;
   maxmountain = 8900.0;
   wavelength = 1.0f;
   waterslant = 0.0f;
   treesize = 25.0f;
   urbanoff = 0;
   botanicslant = 5.0f;
   ignorblack = 0.1f;
   ignorwhite = 0.5f;
   desertcol.r = (-1.0f);
   Parms();
   if (xsize<20L) Error("xsize too less", 15ul);
   if (ysize<20L) Error("ysize too less", 15ul);
   if (angle>=180.0f && flatscreen) {
      Error("sight > 180 deggrees needs courved screen on", 45ul);
   }
   /*  IF imagefn[0]=0C THEN Error("need inage filename") END; */
   scalesize = imagetext_fontsizex(fonttyp)*7UL;
   /*WrInt(scalesize, 5); WrStrLn("scale"); */
   X2C_DYNALLOCATE((char **) &image,sizeof(struct imagetext_PIX),
                (tmp[0] = (size_t)xsize,tmp[1] = (size_t)ysize,tmp),2u);
   if (image==0) Error("out of memory", 14ul);
   X2C_DYNALLOCATE((char **) &pnoise,sizeof(struct _2),
                (tmp[0] = (size_t)(ysize+2L),tmp),1u);
   if (pnoise==0) Error("out of memory", 14ul);
   drawimage();
   libsrtm_closesrtmfile();
   if (csvfd>=0L) osic_Close(csvfd);
   if (imagefn[0U]) wrpng();
   osi_WrStrLn("", 1ul);
   osi_WrStr("srtmaccesses:", 14ul);
   osic_WrINT32(accesses, 1UL);
   osi_WrStrLn("", 1ul);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
