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
#define panorama_PI 3.1415926535

#define panorama_PI2 6.283185307

#define panorama_RAD 1.7453292519444E-2

#define panorama_EARTH 6370.0

#define panorama_NOALT 20000

#define panorama_NL "\012"

#define panorama_LF "\012"

#define panorama_REFRACC 7.85E-8

#define panorama_MAXSYMSIZE 48
/* max input symbol size */

#define panorama_SUNSIZE 4.7996554428472E-3

#define panorama_SHOWTREEDIST 20
/* show trees m from cam */

#define panorama_ZEROTREEALT 2600

#define panorama_FULLTREEALT 1200

#define panorama_TREERASTERSIZE 8.0
/* m full treesize */

#define panorama_TREESIZE 25.0

#define panorama_STAMTHICKNESS 0.01

#define panorama_TREEFORM 0.9

#define panorama_STAMLEN 0.25

#define panorama_MINTREESIZE 2

#define panorama_NOISESIZE 1024

#define panorama_MAXHIST 250

#define panorama_MINHIST 10

#define panorama_MAXLUM 16383

#define panorama_TREERAND 3
/* tree position noise */

#define panorama_LUMMUL 8.0
/* real to cardinal colour values in main image */

#define panorama_ALTICOLORANG 1000
/* colourized altitude */

#define panorama_WATERMIRRORGAP 0.2
/* m over water start ray path */

#define panorama_TREENOTCH (-0.3)
/* random tree kill on altitude and slant */

#define panorama_cFOGLUMr 900.0
/* low elevation heaven light */

#define panorama_cFOGLUMg 1250.0

#define panorama_cFOGLUMb 2300.0

#define panorama_SUNLUM 5000.0

#define panorama_cMAXCHROMr 1.0
/* sun passed thu air */

#define panorama_cMAXCHROMg 0.9

#define panorama_cMAXCHROMb 0.6
/*0.8*/

#define panorama_cFARCHROMr 1.0
/* sun in space */

#define panorama_cFARCHROMg 1.0

#define panorama_cFARCHROMb 1.0

#define panorama_cMEADOWr 310.0

#define panorama_cMEADOWg 430.0

#define panorama_cMEADOWb 235.0

#define panorama_cMEADOWVIODr 458.0

#define panorama_cMEADOWVIODg 458.0

#define panorama_cMEADOWVIODb 429.0

#define panorama_cSNOWr 840.0
/*940 930 920 */

#define panorama_cSNOWg 830.0

#define panorama_cSNOWb 820.0

#define panorama_cSNOWSHADOWr 79.0
/*100 145 280*/

#define panorama_cSNOWSHADOWg 115.0

#define panorama_cSNOWSHADOWb 220.0

#define panorama_cSTAMr 140.0
/* 292 271 249 */

#define panorama_cSTAMg 130.0

#define panorama_cSTAMb 120.0

#define panorama_cSTAMVAR 20.0

#define panorama_cTREEr 210.0
/* 300 354 223 */

#define panorama_cTREEg 290.0

#define panorama_cTREEb 200.0

#define panorama_cCITYCOL 500.0

#define panorama_cCITYMEADOWr 258.0

#define panorama_cCITYMEADOWg 366.0

#define panorama_cCITYMEADOWb 292.0

#define panorama_cCITYSTREETr 249.0

#define panorama_cCITYSTREETg 282.0

#define panorama_cCITYSTREETb 282.0

#define panorama_cROCKr 418

#define panorama_cROCKg 458

#define panorama_cROCKb 448

#define panorama_cUNDERWATERr 260.0
/* 360 360 330 */

#define panorama_cUNDERWATERg 210.0

#define panorama_cUNDERWATERb 180.0

#define panorama_cWATERALIASr 110.0
/* watercolour 110 271 330 */

#define panorama_cWATERALIASg 271.0

#define panorama_cWATERALIASb 330.0

#define panorama_cHEAVENr 150.0

#define panorama_cHEAVENg 300.0

#define panorama_cHEAVENb 3500.0

#define panorama_cLAMPHr 0.3

#define panorama_cLAMPHg 0.24

#define panorama_cLAMPHb 0.2

#define panorama_cLAMPWr 0.3

#define panorama_cLAMPWg 0.3

#define panorama_cLAMPWb 0.3

#define panorama_cLAMPTr 0.4

#define panorama_cLAMPTg 0.1

#define panorama_cLAMPTb 0.0

struct COL;


struct COL {
   float r;
   float g;
   float b;
};

struct CCOL;


struct CCOL {
   uint32_t r;
   uint32_t g;
   uint32_t b;
};

struct POSITIONL;


struct POSITIONL {
   double long0;
   double lat;
};

struct PANOWIN;


struct PANOWIN {
   char flatscreen0;
   struct POSITIONL eye;
   struct POSITIONL horizon;
   float eyealt;
   float angle0;
   float elevation;
   float yzoom0;
   float refract;
   int32_t hx; /* mouse pos on panwin and fullwin */
   int32_t hy;
   int32_t mx;
   int32_t my;
   uint32_t minlum; /* hint for text contrast */
   uint32_t maxlum;
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
   SYMBOL woodsymbol;
   uint32_t xs;
   uint32_t ys;
   char altIsOG;
   char poifn[1024];
   char iconfn[1024];
   char iconwoodfn[1024];
   char iconhint[1024];
};

struct NORMVEC;


struct NORMVEC {
   float x;
   float y;
   float z;
};


struct LVEC {
   double x;
   double y;
   double z;
};

struct ROTVEC;


struct ROTVEC {
   double sx;
   double cx;
   double sy;
   double cy;
};

struct RGB;


struct RGB {
   float r;
   float g;
   float b;
};


struct GLANCESET {
   struct NORMVEC sunvec;
   struct NORMVEC eyevec;
   struct NORMVEC surfacevec;
};

struct TREEHINT;


struct TREEHINT {
   float hith;
   float hitx;
   float hity;
   float sx;
   float sy;
   float peakh;
   float areapeak;
   float regionpeak;
   float hitradius;
   float stamtoeye;
   float og;
   float passdist;
   float missspace;
   int32_t nx;
   int32_t ny;
   char treepass;
   char isstam;
   float tslant;
   float varh;
   float rooth;
   struct libsrtm_METAINFO meta;
   struct NORMVEC nvec;
   uint8_t water;
   uint8_t wood;
   uint8_t urban;
};

typedef struct TREEHINT TREEHINTS[2];

typedef float NOISEMAP[1024][1024];

struct ALTCOLTAB;


struct ALTCOLTAB {
   struct CCOL c[1024];
   struct CCOL tag[5];
   uint32_t len;
};

struct DROPSHADOW;

typedef struct DROPSHADOW * pDROPSHADOW;


struct DROPSHADOW {
   pDROPSHADOW next;
   double dist;
   double alt;
   int32_t colum;
};

struct DDSTACK;


struct DDSTACK {
   float startdist;
   float stoppalt;
};

static imagetext_pIMAGE image;

static pPOI poifiles;

static char dateh[1024];

static char timeh[1024];

static char imagefn[1024];

static char csvfn[1024];

static char colfn[1024];

static int32_t xsize;

static int32_t ysize;

static struct POSITIONL posa;

static struct POSITIONL posb;

static struct POSITIONL teye;

static float refraction;

static float igamma;

static float elev;

static float angle;

static float yzoom;

static float wavelength;

static float waterslant;

static char gammatab[16384];

static uint32_t altcolstep;

static uint32_t rastercsv;

static uint32_t poisize;

static uint32_t scalesize;

static uint32_t whitelim;

static uint32_t blacklim;

static uint32_t fonttyp;

static struct COL heavencol;

static struct RGB desertcol;

static char dropshadows;

static char alticolour;

static char defaultocean;

static char flatscreen;

static char verb;

static char poilabel;

static char flatwater;

static double treeraster;

static double coslat;

static double treerasterlat;

static double treerasterlong;

static double treescramblat;

static double treescramblong;

static double maxmountain;

static double rndseed;

static float treerastersize;

static float abscolmin;

static float abscolmax;

static float alticolmin;

static float alticolmax;

static float zerotreealt;

static float fulltreealt;

static float treeshrink;

static float alta;

static float sunaz;

static float sunel;

static float snowalt;

static float slowaltslant;

static float newsnow;

static float waveamp;

static float dusthighpass;

static float treesize;

static float botanicslant;

static float ignorblack;

static float ignorwhite;

static float showstamdist;

static float videolat;

static float logfilm;

static float urbanoff;

static float sandgranularity;

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

static uint8_t CRCL[256];

static uint8_t CRCH[256];

static uint16_t CRC[65536];

static NOISEMAP noisemap;

static float noisevec[8192];

static struct ALTCOLTAB altcoltab;

static pDROPSHADOW freeshadow;

static pDROPSHADOW newshadow;

static float debugx;

static float debugy;

#define panorama_M (X2C_max_longreal*0.01)


static char StrToFixL(double * x, const char s[],
                uint32_t s_len)
{
   uint32_t i;
   double p;
   char ok0;
   char neg;
   i = 0UL;
   neg = 0;
   ok0 = 0;
   while (i<=s_len-1 && s[i]==' ') ++i;
   if (s[i]=='-') {
      neg = 1;
      ++i;
   }
   *x = 0.0;
   while ((i<=s_len-1 && (uint8_t)s[i]>='0') && (uint8_t)s[i]<='9') {
      if (*x<X2C_max_longreal*0.01) {
         *x =  *x*10.0+(double)((uint32_t)(uint8_t)s[i]-48UL);
      }
      ++i;
      ok0 = 1;
   }
   if (i<=s_len-1 && s[i]=='.') {
      ++i;
      p = 0.1;
      while ((i<=s_len-1 && (uint8_t)s[i]>='0') && (uint8_t)s[i]<='9') {
         *x = *x+p*(double)((uint32_t)(uint8_t)s[i]-48UL);
         p = p*0.1;
         ++i;
         ok0 = 1;
      }
   }
   if (neg) *x = -*x;
   return ok0 && (i>s_len-1 || s[i]==0);
} /* end StrToFixL() */


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

#define panorama_POLINOM 0x8408 


static void Gencrctab(void)
{
   uint32_t c;
   uint32_t crc;
   uint32_t i;
   for (c = 0UL; c<=255UL; c++) {
      crc = 255UL-c;
      for (i = 0UL; i<=7UL; i++) {
         if ((crc&1)) crc = (uint32_t)((uint32_t)(crc>>1)^0x8408UL);
         else crc = crc>>1;
      } /* end for */
      CRCL[c] = (uint8_t)crc;
      CRCH[c] = (uint8_t)(255UL-(crc>>8));
   } /* end for */
   crc = 1UL;
   for (i = 0UL; i<=65535UL; i++) {
      if ((crc&1)) crc = (uint32_t)((uint32_t)(crc>>1)^0x8408UL);
      else crc = crc>>1;
      CRC[i] = (uint16_t)crc;
   } /* end for */
} /* end Gencrctab() */


static float frac(double r)
{
   return (float)(r-floor(r));
} /* end frac() */


static float fracint(double r, int32_t * i)
{
   double ri;
   ri = floor(r);
   *i = (int32_t)X2C_TRUNCI(ri,X2C_min_longint,X2C_max_longint);
   return (float)(r-ri);
} /* end fracint() */


static float min0(float a, float b)
{
   if (a<b) return a;
   return b;
} /* end min() */


static int32_t imin(int32_t a, int32_t b)
{
   if (a<b) return a;
   return b;
} /* end imin() */


static int32_t imax(int32_t a, int32_t b)
{
   if (a>b) return a;
   return b;
} /* end imax() */


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


static char posvalidl(const struct POSITIONL pos)
{
   return pos.lat!=0.0 || pos.long0!=0.0;
} /* end posvalidl() */


static float distancel(struct POSITIONL home, struct POSITIONL dist)
{
   float y;
   float x;
   x = (float)fabs(dist.long0-home.long0);
   y = (float)fabs(dist.lat-home.lat);
   if (x==0.0f && y==0.0f) return 0.0f;
   else if (x+y<0.04f) {
      /* near */
      x = (float)((double)x*cos((home.lat+dist.lat)*0.5));
      return (float)(6370.0*sqrt((double)(x*x+y*y)));
   }
   else {
      /* far */
      if (x>6.283185307f) x = 6.283185307f-x;
      x = (float)(sin(dist.lat)*sin(home.lat)+cos(dist.lat)*cos(home.lat)
                *cos((double)x));
      if ((float)fabs(x)>=1.0f) return 0.0f;
      return 6370.0f*osic_arccos(x);
   }
   return 0;
} /* end distancel() */


static float azimuthl(struct POSITIONL home, struct POSITIONL dist)
{
   float ldiff;
   float h;
   ldiff = (float)(dist.long0-home.long0);
   if ((ldiff==0.0f || cos(home.lat)==0.0) || cos(dist.lat)==0.0) {
      if (home.lat<=dist.lat) return 0.0f;
      else return 180.0f;
   }
   else {
      h = 5.729577951472E+1f*osic_arctan((float)(cos(home.lat)
                *(X2C_DIVL(tan(home.lat)*cos((double)ldiff)
                -tan(dist.lat),sin((double)ldiff)))));
      if (ldiff<0.0f) return h+270.0f;
      else return h+90.0f;
   }
   return 0;
} /* end azimuthl() */


static void makenoise(void)
{
   uint32_t y;
   uint32_t x;
   float v;
   memset((char *)noisemap,(char)0,sizeof(NOISEMAP));
   for (y = 0UL; y<=1024UL; y++) {
      for (x = 0UL; x<=1023UL; x++) {
         v = (float)(Random()-0.5);
         noisemap[y&1023UL][x] = noisemap[y&1023UL][x]*0.7f+v;
         noisemap[y&1023UL][x] = noisemap[y&1023UL][x]
                *0.3333f+noisemap[y+1023UL&1023UL][x]
                *0.3333f+noisemap[y+1UL&1023UL][x]*0.3333f;
      } /* end for */
   } /* end for */
   memset((char *)noisevec,(char)0,sizeof(float [8192]));
   for (x = 0UL; x<=65535UL; x++) {
      noisevec[x&8191UL] = (noisevec[x&8191UL]*0.7f+(float)
                CRC[x]*1.52587890625E-5f)-0.5f;
      noisevec[x&8191UL] = noisevec[x+8191UL&8191UL]
                *0.3333f+noisevec[x&8191UL]*0.3333f+noisevec[x+1UL&8191UL]
                *0.3333f;
   } /* end for */
   v = 0.0f;
   for (x = 1UL; x<=8191UL; x++) {
      v = noisevec[x]-noisevec[x-1UL];
      if (v>0.1f) v = 0.1f;
      else if (v<(-0.1f)) v = (-0.1f);
      noisevec[x] = noisevec[x-1UL]+v;
   } /* end for */
   /*
       IF noisevec[x]>0.0 THEN noisevec[x]:=sqrt(noisevec[x])
       ELSIF noisevec[x]<0.0 THEN noisevec[x]:=-sqrt(-noisevec[x]) END;
       v:=v+(noisevec[x]-v)*0.2;
       noisevec[x]:=noisevec[x]-v;
   */
   v = 0.0f; /* normalize level */
   for (x = 0UL; x<=1023UL; x++) {
      for (y = 0UL; y<=1023UL; y++) {
         v = v+(float)fabs(noisemap[y][x]);
      } /* end for */
   } /* end for */
   v = X2C_DIVR(sqr(1024.0f),v);
   for (x = 0UL; x<=1023UL; x++) {
      for (y = 0UL; y<=1023UL; y++) {
         noisemap[y][x] = noisemap[y][x]*v;
      } /* end for */
   } /* end for */
/*  WrFixed(v/sqr(FLOAT(HIGH(noisemap)+1)), 3, 10); WrStrLn(""); */
} /* end makenoise() */

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

static void posinval(struct POSITIONL * pos)
{
   pos->long0 = 0.0;
   pos->lat = 0.0;
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


static float distnear(const struct POSITIONL a, const struct POSITIONL b)
{
   float y;
   float x;
   x = (float)((a.long0-b.long0)*coslat);
   y = (float)(a.lat-b.lat);
   return 6.37E+6f*osic_sqrt(x*x+y*y);
} /* end distnear() */

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


static void makegammatab(uint32_t min1, uint32_t max0)
{
   uint32_t span;
   uint32_t c;
   float g;
   if (max0<=min1) {
      min1 = 0UL;
      max0 = 16383UL;
   }
   span = max0-min1;
   g = X2C_DIVR(1.0f,igamma);
   for (c = 0UL; c<=16383UL; c++) {
      if (c>min1) {
         if (c<=max0) {
            gammatab[c] = (char)(uint32_t)
                X2C_TRUNCC(exp(log((double)(X2C_DIVR((float)(c-min1)
                ,(float)span)))*(double)g)*255.5,0UL,
                X2C_max_longcard);
         }
         else gammatab[c] = '\377';
      }
      else gammatab[c] = 0;
   } /* end for */
} /* end makegammatab() */


static void histogram(imagetext_pIMAGE img, char nosat,
                uint32_t * min1, uint32_t * max0)
{
   uint32_t maxe;
   uint32_t ib;
   uint32_t iw;
   uint32_t pixc;
   uint32_t pc;
   uint32_t y;
   uint32_t x;
   float llog;
   float ilog;
   float rw;
   float w;
   float fb;
   float fg;
   float fr;
   float mul;
   float fmin;
   uint32_t hh[16384];
   /*    hr:ARRAY[0..MAXLUM] OF REAL; */
   uint32_t h[16384];
   struct imagetext_PIX * anonym;
   struct imagetext_PIX * anonym0;
   uint32_t tmp;
   uint32_t tmp0;
   memset((char *)h,(char)0,65536UL);
   memset((char *)hh,(char)0,65536UL);
   tmp = img->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = img->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct imagetext_PIX * anonym = &img->Adr[(x)*img->Len0+y];
            pc = ((uint32_t)anonym->r*3UL+(uint32_t)
                anonym->g*5UL+(uint32_t)anonym->b*2UL)/10UL;
            if (nosat) {
               if ((uint32_t)anonym->r>pc) {
                  pc = (uint32_t)anonym->r; /* saturated single colours */
               }
               if ((uint32_t)anonym->g>pc) pc = (uint32_t)anonym->g;
               if ((uint32_t)anonym->b>pc) pc = (uint32_t)anonym->b;
            }
            if (pc>16383UL) pc = 16383UL;
            ++hh[pc]; /* whole image */
            if (!(anonym->b&1)) ++h[pc];
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   pixc = ((img->Len1-1)+1UL)*((img->Len0-1)+1UL);
   ib = (uint32_t)X2C_TRUNCC((float)pixc*ignorblack*0.01f,0UL,
                X2C_max_longcard);
   iw = (uint32_t)X2C_TRUNCC((float)pixc*ignorwhite*0.01f,0UL,
                X2C_max_longcard);
   *min1 = 1UL; /* dont use unfilled black stripes */
   while (ib>=h[*min1] && *min1<16383UL) {
      ib -= h[*min1];
      ++*min1;
   }
   *max0 = 16383UL;
   while (iw>=hh[*max0] && *max0>0UL) {
      iw -= hh[*max0];
      --*max0;
   }
   iw = (uint32_t)X2C_TRUNCC((float)pixc*ignorwhite*0.01f,0UL,
                X2C_max_longcard);
   maxe = 16383UL;
   while (iw>=h[maxe] && maxe>0UL) {
      iw -= h[maxe];
      --maxe;
   }
   maxe = (uint32_t)imax((int32_t)maxe, (int32_t)whitelim);
   *min1 = (uint32_t)imin((int32_t)*min1, (int32_t)blacklim);
   *max0 = (uint32_t)imax((int32_t)*max0, (int32_t)whitelim);
                /* with heaven */
   /*- check contrast for auto filmlinearity */
   llog = logfilm;
   if (logfilm<0.0f) {
      /* contrast auto */
      llog = 2.E+5f;
      while (llog>100.0f && llog*osic_ln(1.0f+X2C_DIVR((float)*max0,
                llog))>(float)maxe) llog = llog*0.95f;
   }
   if (llog>0.0f) {
      ilog = X2C_DIVR(1.0f,llog);
      *max0 = (uint32_t)X2C_TRUNCC(llog*osic_ln(1.0f+(float)*max0*ilog),
                0UL,X2C_max_longcard);
   }
   fmin = (float)*min1;
   mul = X2C_DIVR(16383.0f,(float)*max0-fmin);
   if (verb) {
      osi_WrStr("min..allmax, earthmax, log: ", 29ul);
      osic_WrINT32(*min1, 6UL);
      osic_WrINT32(*max0, 6UL);
      osic_WrINT32(maxe, 6UL);
      osic_WrFixed(llog, 1L, 10UL);
      osi_WrStrLn("", 1ul);
   }
   tmp = img->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = img->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct imagetext_PIX * anonym0 = &img->Adr[(x)*img->Len0+y];
            fr = (float)anonym0->r;
            fg = (float)anonym0->g;
            fb = (float)anonym0->b;
            w = fr*0.3f+fg*0.5f+fb*0.2f+0.1f;
            rw = X2C_DIVR(1.0f,w);
            fr = fr*rw;
            fg = fg*rw;
            fb = fb*rw;
            w = w-fmin;
            if (w<0.1f) w = 0.1f;
            if (llog!=0.0f) w = llog*osic_ln(1.0f+w*ilog);
            w = mul*w;
            fr = fr*w;
            if (fr>16382.0f) fr = 16382.0f;
            fg = fg*w;
            if (fg>16382.0f) fg = 16382.0f;
            fb = fb*w;
            if (fb>16382.0f) fb = 16382.0f;
            anonym0->r = (uint16_t)(uint32_t)X2C_TRUNCC(fr,0UL,
                X2C_max_longcard);
            anonym0->g = (uint16_t)(uint32_t)X2C_TRUNCC(fg,0UL,
                X2C_max_longcard);
            anonym0->b = (uint16_t)(uint32_t)X2C_TRUNCC(fb,0UL,
                X2C_max_longcard);
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   /*
           r:=TRUNC(w*fr);
           g:=TRUNC(w*fg);
           b:=TRUNC(w*fb);
           IF r>MAXLUM THEN r:=MAXLUM END;
           IF g>MAXLUM THEN g:=MAXLUM END;
           IF b>MAXLUM THEN b:=MAXLUM END;
   */
   *min1 = 0UL;
   *max0 = 16383UL;
} /* end histogram() */


static uint8_t pngc(uint16_t c)
{
   if (c<=16383U) return gammatab[c];
   else return 255U;
   return 0;
} /* end pngc() */


static int32_t wrpng(void)
{
   struct PNGPIXMAP pngimg;
   int32_t ret;
   /*    min, max:CARDINAL; */
   int32_t y;
   int32_t x;
   struct imagetext_PIX * anonym;
   struct PNGPIXEL * anonym0;
   int32_t tmp;
   int32_t tmp0;
   ret = -2L;
   osic_alloc((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   if (pngimg.image) {
      /*
          histogram(image, min, max);
          makegammatab(min, max);
      */
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
   return ret;
} /* end wrpng() */

/*
------- moon

CONST E=RAD*23.4397; (* obliquity of the Earth *)


PROCEDURE fdeclination(l, b:LONGREAL):LONGREAL;
BEGIN RETURN asin(sin(b)*cos(E) + cos(b)*sin(E)*sin(l)) END fdeclination;

PROCEDURE frightascension(l, b:LONGREAL):LONGREAL;
BEGIN RETURN atang2(sin(l)*cos(E)-tan(b)*sin(E), cos(l)) END frightascension;

PROCEDURE siderealtime(d, lw:LONGREAL):LONGREAL;
BEGIN RETURN RAD*(280.16 + 360.9856235*d) - lw END siderealtime;

PROCEDURE faltitude(h, phi, dec:REAL):REAL;
BEGIN RETURN asin(sin(phi)*sin(dec) + cos(phi)*cos(dec)*cos(h))
                END faltitude;

PROCEDURE fazimuth(h, phi, dec:REAL):REAL;
BEGIN RETURN atang2(sin(h), cos(h)*sin(phi) - tan(dec)*cos(phi)) END fazimuth;

PROCEDURE mooncoords(d:LONGREAL; VAR declination, distance, rightascension:REAL);
VAR el, m, f, l, b:LONGREAL;
BEGIN
  el:=(218.316 + 13.176396*d)*RAD;
  m:= (134.963 + 13.064993*d)*RAD;
  f:= (93.272  + 13.229350*d)*RAD;

  l:=el + 6.289*RAD*sin(m);         (* latitude *)
  b:=5.128*RAD*sin(f);              (* longitude *)
  distance:=385001 - 20905*cos(m);        (* distance to the moon in km *)
  declination:=fdeclination(l, b);
  rightascension:=frightascension(l, b);
END mooncoords;

PROCEDURE moonposition(lat, long:LONGREAL; time:CARDINAL;
                VAR az, el, distance:REAL);
VAR lw, phi, declination, bigh, rightascension, h:REAL;
    d:LONGREAL;
s:ARRAY[0..30] OF CHAR;
BEGIN
  lw:=-long;
  phi:=lat;
  d:=(LFLOAT(time) - LFLOAT(86400*((2000-1970)*1461 DIV 4)))
                /86400.0 - 0.5 (*49925*);
WrFixed(d, 5, 15);WrStrLn("=jd");
  mooncoords(d, declination, distance, rightascension);
  bigh:=siderealtime(d, lw) - rightascension;
  h:=faltitude(bigh, phi, declination);

  (* Altitude correction for refraction *)
  h:=h + (RAD*0.017)/tan(h + (RAD*10.26)/(h + RAD*5.10));

  az:=fazimuth(bigh, phi, declination);
  el:=h;
END moonposition;

PROCEDURE time2moon(t:CARDINAL; mypos:POSITIONL; VAR moonaz, moonel:REAL);
BEGIN
  moonposition(mypos.lat, mypos.long, t, moonaz, moonel, moondist);
  moonaz:=moonaz+PI;
  IF moonaz>2*PI THEN moonaz:=moonaz-2*PI END;
WrStr("moon az el:"); WrFixed(moonaz/RAD, 2, 7); WrFixed(moonel/RAD, 2, 7);
                WrFixed(moondist, 0, 12);WrStrLn("");
END time2moon;

------- moon
*/
/*---- sun */

static void wrsun(float sunaz0, float sunel0)
{
   if (verb) {
      osic_WrFixed(sunaz0, 2L, 7UL);
      osi_WrStrLn(" sunaz", 7ul);
      osic_WrFixed(sunel0, 2L, 7UL);
      osi_WrStrLn(" sunel", 7ul);
   }
} /* end wrsun() */


static uint32_t dig(char c, char * err)
{
   if ((uint8_t)c<'0' || (uint8_t)c>'9') {
      *err = 1;
      return 0UL;
   }
   else return (uint32_t)(uint8_t)c-48UL;
   return 0;
} /* end dig() */


static char StrToDate(char s[], uint32_t s_len,
                uint32_t * time0)
/* <yyyy mm dd> */
{
   uint32_t k;
   uint32_t d;
   uint32_t m;
   uint32_t y;
   char err;
   char StrToDate_ret;
   X2C_PCOPY((void **)&s,s_len);
   err = 0;
   y = dig(s[0UL], &err)*1000UL+dig(s[1UL], &err)*100UL+dig(s[2UL],
                &err)*10UL+dig(s[3UL], &err);
   if (y<1970UL || y>2100UL) {
      StrToDate_ret = 0;
      goto label;
   }
   *time0 = (y-1970UL)*365UL+(y-1969UL)/4UL;
   m = dig(s[4UL], &err)*10UL+dig(s[5UL], &err);
   d = dig(s[6UL], &err)*10UL+dig(s[7UL], &err);
   if (d<1UL || d>31UL) {
      StrToDate_ret = 0;
      goto label;
   }
   k = (uint32_t)((y&3UL)==0UL);
   *time0 += d-1UL;
   if (m>2UL) *time0 += k;
   switch (m) {
   case 1UL:
      break;
   case 2UL:
      if (d>28UL+k) err = 1;
      *time0 += 31UL;
      break;
   case 3UL:
      *time0 += 59UL;
      break;
   case 4UL:
      if (d>30UL) err = 1;
      *time0 += 90UL;
      break;
   case 5UL:
      *time0 += 120UL;
      break;
   case 6UL:
      if (d>30UL) err = 1;
      *time0 += 151UL;
      break;
   case 7UL:
      *time0 += 181UL;
      break;
   case 8UL:
      *time0 += 212UL;
      break;
   case 9UL:
      if (d>30UL) err = 1;
      *time0 += 243UL;
      break;
   case 10UL:
      *time0 += 273UL;
      break;
   case 11UL:
      if (d>30UL) err = 1;
      *time0 += 304UL;
      break;
   case 12UL:
      *time0 += 334UL;
      break;
   default:;
      err = 1;
      break;
   } /* end switch */
   StrToDate_ret = !err;
   label:;
   X2C_PFREE(s);
   return StrToDate_ret;
} /* end StrToDate() */


static char StrToDaytime(char s[], uint32_t s_len,
                uint32_t * time0)
/* <yyyy mm dd> */
{
   uint32_t se;
   uint32_t mi;
   uint32_t h;
   char err;
   char StrToDaytime_ret;
   X2C_PCOPY((void **)&s,s_len);
   err = 0;
   h = dig(s[0UL], &err)*10UL+dig(s[1UL], &err);
   if (h>24UL) {
      StrToDaytime_ret = 0;
      goto label;
   }
   mi = dig(s[2UL], &err)*10UL+dig(s[3UL], &err);
   if (mi>=60UL) {
      StrToDaytime_ret = 0;
      goto label;
   }
   se = dig(s[4UL], &err)*10UL+dig(s[5UL], &err);
   if (se>=60UL) {
      StrToDaytime_ret = 0;
      goto label;
   }
   *time0 = h*3600UL+mi*60UL+se;
   StrToDaytime_ret = !err;
   label:;
   X2C_PFREE(s);
   return StrToDaytime_ret;
} /* end StrToDaytime() */


static float arcsin(float x)
{
   return osic_arctan((float)(X2C_DIVL((double)x,
                sqrt((double)(1.0f-x*x)))));
} /* end arcsin() */

/*
  PROCEDURE atan2(y, x:REAL):REAL;
  BEGIN
    IF x=0.0 THEN
      IF y>=0.0 THEN RETURN PI*0.5 ELSE RETURN -PI*0.5 END;
    ELSIF x>=0.0 THEN RETURN osi.arctan(y/x)
    ELSIF y>=0.0 THEN RETURN osi.arctan(y/x)+PI ELSE RETURN osi.arctan(y/x)
                -PI END;
  END atan2;
  
  PROCEDURE sonnendeklination(t:CARDINAL):REAL;
  BEGIN
-- Deklination der Sonne in Radians
-- Formula 2008 by Arnold(at)Barmettler.com,
                fit to 20 years of average declinations (2008-2017)
    RETURN 0.409526325277017*sin(0.0169060504029192*(FLOAT(t)
                -80.0856919827619)); 
  END sonnendeklination;

  PROCEDURE zeitdifferenz(Deklination, lat:REAL):REAL;
  BEGIN
-- Dauer des halben Tagbogens in Stunden: Zeit von Sonnenaufgang (Höhe h)
                bis zum höchsten Stand im Süden
    RETURN 12.0*osi.arccos((-0.0145438976516299 - sin(lat)*sin(Deklination))
                / (cos(lat)*cos(Deklination)))/PI;
  END zeitdifferenz;

  PROCEDURE zeitgleichung(t:CARDINAL):REAL;
  BEGIN
-- Differenz zwischen wahrer und mittlerer Sonnenzeit
-- formula 2008 by Arnold(at)Barmettler.com,
                fit to 20 years of average equation of time (2008-2017)
    RETURN -0.170869921174742*sin(0.0336997028793971 * FLOAT(t)
                + 0.465419984181394)
                - 0.129890681040717*sin(0.0178674832556871*FLOAT(t)
                - 0.167936777524864);
  END zeitgleichung;


  PROCEDURE AzimutHoehe(lat:REAL; t:CARDINAL; ZeitSeitMittag:REAL;
                VAR azimut, hoehe:REAL);
  VAR dk, cosdec, sindec, lha, coslha, sinlha, coslat, sinlat:REAL;
  BEGIN
-- Azimut und Höhe der Sonne (ohne Refraktion)
    dk:=sonnendeklination(t);
    cosdec:=cos(dk);
    sindec:=sin(dk);
    lha:=ZeitSeitMittag*((1.0027379-1.0/365.25)*15.0*RAD);
                (* Stundenwinkel seit wahrem Mittag in Radians *)
-- 1.0027379: Faktor für Sternzeitkorrektur,
                1./365.25: Fortschreiten der Rektaszension der Sonne in einem Tag in Grad
    coslha:=cos(lha);
    sinlha:=sin(lha);
    coslat:=cos(lat);
    sinlat:=sin(lat);
    azimut:=atan2(-cosdec * sinlha,
                sindec * coslat - cosdec * coslha * sinlat);
    IF azimut<0.0 THEN azimut:=azimut+2.0*PI END;
                (* Azimut. Norden=0, Osten=pi/2, Westen=3/4pi *)
    hoehe:=arcsin(sindec * sinlat + cosdec * coslha * coslat);
                (* Hoehe des Sonnenmittelpunkts *)
  END AzimutHoehe;

  PROCEDURE Refraktion(hoehe:REAL):REAL;
  VAR r, t, p:REAL;
  BEGIN
-- Naeherungsloesung für die Refraktion fuer ein Objekt bei Hoehe hoehe über mathematischem Horizont
-- Refraktion beträgt bei Sonnenaufgang 34 Bogenminuten = 0.56667°
-- Falls die Höhe der Sonne nicht genauer als auf 0.5° gewuenscht ist,
                kann diese Funktion ignoriert werden
    p:=1013.25;       (* Luftdruck der Standard-Atmosphäre in hPa (=mbar) *)
    t:=15.0;          (* Temperatur der Standard-Atmosphäre in °C *)
    r:=0.0;
    IF hoehe>=15.0*RAD THEN r:=0.00452*RAD*p/tan(hoehe)/(273.0+t);
                (* ueber 15° - einfachere Formel *)
    ELSIF hoehe>-1.0*RAD THEN r:=RAD*p*(0.1594+0.0196*hoehe+0.00002*hoehe*hoehe)
                /((273.0+t)*(1+0.505*hoehe+0.0845*hoehe*hoehe)) END;
    RETURN r          (* Refraktion in Radians *)
  END Refraktion;
*/

static float modrad(float w)
{
   return w-(float)((uint32_t)X2C_TRUNCC(X2C_DIVR(w,360.0f),0UL,
                X2C_max_longcard)*360UL);
} /* end modrad() */


static void sunpos(uint32_t tt, float lat, float long0,
                float * azi, float * ele)
{
   float ta;
   float fl;
   float fg;
   float fhg;
   float n;
   float t0;
   float ga;
   float c;
   float alpha;
   float e;
   float aa;
   float r;
   float g;
   float l;
   n = X2C_DIVR((float)(tt-946684800UL),86400.0f)-0.5f;
   l = 280.46f+9.856474E-1f*n;
   l = modrad(l);
   g = 357.528f+9.856003E-1f*n;
   g = modrad(g)*1.7453292519444E-2f;
   aa = (float)((double)l+1.915*sin((double)g)
                +0.02*sin((double)(g*2.0f)));
   e = 23.439f-4.E-7f*n;
   c = (float)cos((double)(aa*1.7453292519444E-2f));
   alpha = X2C_DIVR(osic_arctan((float)(X2C_DIVL(cos((double)
                (e*1.7453292519444E-2f))*sin((double)
                (aa*1.7453292519444E-2f)),(double)c))),
                1.7453292519444E-2f);
   if (c<0.0f) alpha = alpha+180.0f;
   ga = X2C_DIVR(arcsin((float)(sin((double)(e*1.7453292519444E-2f))
                *sin((double)(aa*1.7453292519444E-2f)))),
                1.7453292519444E-2f);
   t0 = X2C_DIVR((float)(int32_t)X2C_TRUNCI(n,X2C_min_longint,
                X2C_max_longint)+0.5f,36525.0f);
   fhg = 6.697376f+2.40005134E+3f*t0+X2C_DIVR(1.002738f*(float)
                (tt%86400UL),3600.0f);
   fhg = fhg-(float)(uint32_t)X2C_TRUNCC(X2C_DIVR(fhg,24.0f),0UL,
                X2C_max_longcard)*24.0f;
   fg = fhg*15.0f;
   fl = fg+long0;
   ta = fl-alpha;
   c = (float)(cos((double)(ta*1.7453292519444E-2f))
                *sin((double)(lat*1.7453292519444E-2f))
                -tan((double)(ga*1.7453292519444E-2f))
                *cos((double)(lat*1.7453292519444E-2f)));
   *azi = X2C_DIVR(osic_arctan((float)(X2C_DIVL(sin((double)
                (ta*1.7453292519444E-2f)),(double)c))),
                1.7453292519444E-2f)+360.0f+180.0f;
   if (c<0.0f) *azi = *azi+180.0f;
   *azi = *azi-(float)(uint32_t)X2C_TRUNCC(X2C_DIVR(*azi,360.0f),0UL,
                X2C_max_longcard)*360.0f;
   *ele = X2C_DIVR(arcsin((float)(cos((double)
                (ga*1.7453292519444E-2f))*cos((double)
                (ta*1.7453292519444E-2f))*cos((double)
                (lat*1.7453292519444E-2f))+sin((double)
                (ga*1.7453292519444E-2f))*sin((double)
                (lat*1.7453292519444E-2f)))),1.7453292519444E-2f);
   r = (float)(X2C_DIVL(0.017,tan((double)((*ele+X2C_DIVR(10.3f,
                *ele+5.11f))*1.7453292519444E-2f))));
   *ele = *ele+r;
} /* end sunpos() */


static char time2sun(char date[], uint32_t date_len,
                char time0[], uint32_t time_len,
                struct POSITIONL mypos, float * sunaz0,
                float * sunel0)
{
   uint32_t day;
   uint32_t t;
   char time2sun_ret;
   X2C_PCOPY((void **)&date,date_len);
   X2C_PCOPY((void **)&time0,time_len);
   if (StrToDate(date, date_len, &day) && StrToDaytime(time0, time_len, &t)) {
      sunpos(t+day*86400UL, (float)(X2C_DIVL(mypos.lat,
                1.7453292519444E-2)), (float)(X2C_DIVL(mypos.long0,
                1.7453292519444E-2)), sunaz0, sunel0);
      /*
          sunaz:=sunaz*RAD;
          sunel:=sunel*RAD;
      
          day:=25568+t DIV (60*60*24);
          day:=1 + day*4 MOD 1461 DIV 4;
       
          AzimutHoehe(mypos.lat, day,
                FLOAT(t MOD 86400)/3600.0 + mypos.long/(RAD*15.0)
                - 12.0+zeitgleichung(day), sunaz, sunel);
          sunel:=sunel + Refraktion(sunel);
      */
      wrsun(*sunaz0, *sunel0);
      /*    time2moon(t+day*86400, mypos, moonazim, moonelev); */
      time2sun_ret = 1;
      goto label;
   }
   time2sun_ret = 0;
   label:;
   X2C_PFREE(date);
   X2C_PFREE(time0);
   return time2sun_ret;
} /* end time2sun() */


static void autosun(float * az, float * el, struct POSITIONL pos,
                float pan)
{
   if (pos.lat>0.0==pan<180.0f) *az = 90.0f;
   else *az = (-90.0f);
   *az = pan+*az;
   if (*az<0.0f) *az = *az+360.0f;
   else if (*az>360.0f) *az = *az-360.0f;
   *el = (float)(10.0+0.65*(90.0-X2C_DIVL(fabs(pos.lat),
                1.7453292519444E-2))*fabs(cos((double)
                ( *az*1.7453292519444E-2f))));
/*WrFixed(ABS(pos.lat)/RAD, 3, 9); WrStrLn("=r"); */
} /* end autosun() */

/*----- sun */

static void getpos(char h[1024], struct POSITIONL * p,
                const char e[], uint32_t e_len)
{
   struct aprsstr_POSITION ps;
   osi_NextArg(h, 1024ul);
   aprsstr_loctopos(&ps, h, 1024ul);
   if (posvalid(ps)) {
      p->lat = (double)ps.lat;
      p->long0 = (double)ps.long0;
   }
   else {
      if (!StrToFixL(&p->lat, h, 1024ul) || fabs(p->lat)>=90.0) {
         Error(e, e_len);
      }
      osi_NextArg(h, 1024ul);
      if (!StrToFixL(&p->long0, h, 1024ul) || fabs(p->long0)>180.0) {
         Error(e, e_len);
      }
      p->lat = p->lat*1.7453292519444E-2;
      p->long0 = p->long0*1.7453292519444E-2;
   }
} /* end getpos() */


static void Parms(void)
{
   char err;
   char altOG;
   char iconhint[1024];
   char iconwoodfn[1024];
   char iconfn[1024];
   char h[1024];
   float sunelev;
   float sunazim;
   float rn;
   float dist;
   float pan;
   uint32_t label;
   pPOI ppoi;
   int32_t col;
   err = 0;
   label = 0UL;
   iconfn[0] = 0;
   iconwoodfn[0] = 0;
   iconhint[0] = 0;
   pan = 0.0f;
   dist = 0.0f;
   dateh[0] = 0;
   timeh[0] = 0;
   altOG = 0;
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
            aprsstr_Assign(ppoi->iconwoodfn, 1024ul, iconwoodfn, 1024ul);
            aprsstr_Assign(ppoi->iconhint, 1024ul, iconhint, 1024ul);
            ppoi->altIsOG = altOG;
            altOG = 0;
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
               Error("-I <icon filename> <icon no wood filename> (png)",
                49ul);
            }
            osi_NextArg(iconwoodfn, 1024ul);
            if (iconwoodfn[0U]==0 || iconwoodfn[0U]=='-') {
               Error("-I <icon filename> <icon no wood filename> (png)",
                49ul);
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
         else if (h[1U]=='V') poilabel = 1;
         else if (h[1U]=='O') altOG = 1;
         else if (h[1U]=='Z') defaultocean = 1;
         else if (h[1U]=='j') dropshadows = 1;
         else if (h[1U]=='u') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&urbanoff, h, 1024ul)) {
               Error("-u <pixel per meter>", 21ul);
            }
         }
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
            osi_NextArg(dateh, 1024ul);
            osi_NextArg(timeh, 1024ul);
         }
         else if (h[1U]=='W') {
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
         else if (h[1U]=='N') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&abscolmin, h, 1024ul)) {
               aprsstr_Assign(colfn, 1024ul, h, 1024ul);
               osi_NextArg(h, 1024ul);
               if (!aprsstr_StrToFix(&abscolmin, h, 1024ul)) {
                  Error("-N [<coloufile>] <minalt> <maxalt> <step>", 42ul);
               }
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&abscolmax, h, 1024ul)) {
               Error("-N [<coloufile>] <minalt> <maxalt> <step>", 42ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &altcolstep)) {
               Error("-N [<coloufile>] <minalt> <maxalt> <step>", 42ul);
            }
            alticolour = 1;
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&treesize, h, 1024ul)) {
               Error("-t <treesize> <botanicslant> <treedist>", 40ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&botanicslant, h, 1024ul)) {
               Error("-t <treesize> <botanicslant> <treedist>", 40ul);
            }
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&treerastersize, h,
                1024ul) || treerastersize<1.0f) || treerastersize>100.0f) {
               Error("-t <treesize> <botanicslant> <treedist> [1..100m]",
                50ul);
            }
         }
         else if (h[1U]=='T') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&zerotreealt, h, 1024ul)) {
               Error("-T <no-tree-alt> <full-tree-alt>", 33ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&fulltreealt, h, 1024ul)) {
               Error("-T <no-tree-alt> <full-tree-alt>", 33ul);
            }
            if (zerotreealt<=fulltreealt) {
               Error("-T no_tree_alt <= full_tree_alt ?", 34ul);
            }
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&ignorblack, h, 1024ul)) {
               Error("-l <black%> <white%>", 21ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&ignorwhite, h, 1024ul)) {
               Error("-l <black%> <white%>", 21ul);
            }
         }
         else if (h[1U]=='D') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&desertcol.r, h, 1024ul)) {
               Error("-D <r> <g> <b> <granular>", 26ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&desertcol.g, h, 1024ul)) {
               Error("-D <r> <g> <b> <granular>", 26ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&desertcol.b, h, 1024ul)) {
               Error("-D <r> <g> <b> <granular>", 26ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&sandgranularity, h, 1024ul)) {
               Error("-D <r> <g> <b> <granular>", 26ul);
            }
         }
         else if (h[1U]=='o') {
            /*
                    desertcol.r:=desertcol.r*16.0;
                    desertcol.g:=desertcol.g*16.0;
                    desertcol.b:=desertcol.b*16.0;
            */
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&minpoialt, h,
                1024ul) || minpoialt<(-2.E+4f)) || minpoialt>20000.0f) {
               Error("-o <meter> (0..20000)", 22ul);
            }
         }
         else if (h[1U]=='K') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&logfilm, h, 1024ul)) {
               Error("-K <filmlinearity>", 19ul);
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
         else if (h[1U]=='L') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &blacklim)) {
               Error("-L <blacklim> <whitelim>", 25ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &whitelim)) {
               Error("-L <blacklim> <whitelim>", 25ul);
            }
         }
         else if (h[1U]=='Q') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&videolat, h,
                1024ul) || videolat<(-89.0f)) || videolat>89.0f) {
               Error("-Q <degrees> [-89..89]", 23ul);
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
                1024ul) || igamma<0.1f) || igamma>10.0f) {
               Error("-g <gamma> [0.1..10] (1.0)", 27ul);
            }
         }
         else if (h[1U]=='H') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&heavencol.r, h,
                1024ul) || heavencol.r>=256.0f) {
               Error("-H <r> <g> <b> 0..255", 22ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&heavencol.g, h,
                1024ul) || heavencol.g>=256.0f) {
               Error("-H <r> <g> <b> 0..255", 22ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&heavencol.b, h,
                1024ul) || heavencol.b>=256.0f) {
               Error("-H <r> <g> <b> 0..255", 22ul);
            }
            heavencol.r = heavencol.r*16.0f;
            heavencol.g = heavencol.g*16.0f;
            heavencol.b = heavencol.b*16.0f;
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
ound or over NN [m] (10)", 74ul);
               osi_WrStrLn(" -a <lat> <long> | [locator]       Camera positio\
n lat long (degrees) or qth locator", 85ul);
               osi_WrStrLn(" -b <lat> <long> | [locator]       Pan to point l\
at long (degrees) or qth locator", 82ul);
               osi_WrStrLn(" -C <N>                            write CSV line\
 for every N\'th column", 72ul);
               osi_WrStrLn(" -c <filename>                     CSV file name",
                 49ul);
               osi_WrStrLn(" -D <r> <g> <b> <granularity>      Sand/Rock colo\
ur and granularity (with -t 0 0 -u 0)", 87ul);
               osi_WrStrLn(" -d <deg> <km>                     Relative to po\
sition a (alternativ to -b)", 77ul);
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
1..10 (1.0)", 61ul);
               osi_WrStrLn(" -H <r> <g> <b>                    Heaven colour \
(50 70 300)", 61ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I <filename> <filename>          Symbol Image F\
ile Name with, w/o wood, set before -P", 88ul);
               osi_WrStrLn(" -i <filename>                     Image File Nam\
e", 51ul);
               osi_WrStrLn(" -J <poi-hint>                     text appended \
to csv lines, set before -P, off with -J ,", 92ul);
               osi_WrStrLn(" -K <filmcharacteristic>           compress sunli\
ght/dust to fit in image brightness (4000) 0=linear else auto", 111ul);
               osi_WrStrLn(" -l <%> <%>                        fit image cont\
rast to saturate % of all black/white pixels (0.1 0.5)", 104ul);
               osi_WrStrLn(" -L <n> <n>                        limit contrast\
 expansion black white (100 1000)", 83ul);
               osi_WrStrLn(" -M <bytes>                        SRTM-Cache Lim\
it (100000000)", 64ul);
               osi_WrStrLn(" -m <meter>                        limit searchin\
g montain size in viewed area for speed on long distance (8900)", 113ul);
               osi_WrStrLn(" -N [<colourfile>] <meter> <meter> <meter>  Colou\
rize altitude <from><to><step> m, 0 is autoscale", 98ul);
               osi_WrStrLn("                                      file: [<a>]\
 <r> <g> <b> ... min to max (0..255)", 86ul);
               osi_WrStrLn("                                      <a>: w wate\
r, t trees, u urban, x ?, y ?", 79ul);
               osi_WrStrLn(" -O                                POI File altit\
ude is over ground not NN, set before -P", 90ul);
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
               osi_WrStrLn(" -Q <lat>                          for video ente\
r raw latitude for squarish tree raster (defaul camera lat)", 109ul);
               osi_WrStrLn("                                     camera altit\
ude now over NN", 65ul);
               osi_WrStrLn(" -r <refraction>                   0.0(vacuum)..1\
.0(earth is a disk) (0.13)", 76ul);
               osi_WrStrLn(" -S <deg> <deg> | <date> <time>    Sun azimuth an\
d elevation or utc: 20191231 115959", 85ul);
               osi_WrStrLn(" -s <size>                         internal gener\
ated POI symbol size (5)", 74ul);
               osi_WrStrLn(" -T <no-tree-alt> <full-tree-alt>  2500 1200 m al\
titude shrink trees from full size to <2m", 91ul);
               osi_WrStrLn(" -t <meter> <steep> <dist>         Tree size (25)\
 lowered linear as given in -T, 0=wood off", 92ul);
               osi_WrStrLn("                                     steep=0 all \
rock, steep=5 rock if steeper (8)", 83ul);
               osi_WrStrLn("                                     median tree \
distance in wood (8)", 70ul);
               osi_WrStrLn(" -u <meter>                        Urban area fad\
e-in at pixel per meter, 0 urban off", 86ul);
               osi_WrStrLn(" -V                                Write POI-Name\
 over Icon", 60ul);
               osi_WrStrLn(" -v                                Say something",
                 49ul);
               osi_WrStrLn(" -W <wavehigth> <wavelength> <flatness>  draw Wat\
er with waves (0.3) (1.0) (0.0), flatness 0 no filter, 0.99 very flat",
                119ul);
               osi_WrStrLn("                                     -0.99 make w\
ater on flat if no tagged srtm", 80ul);
               osi_WrStrLn(" -w <degrees>                      Camera horizon\
tal sight angle (degrees) (45.0)", 82ul);
               osi_WrStrLn(" -x <size>                         Image size x (\
600)", 54ul);
               osi_WrStrLn(" -y <size>                         Image size y (\
400)", 54ul);
               osi_WrStrLn(" -Z                                Ozean if no (v\
alid) srtm data or srtm30 with 0m", 83ul);
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
   if ((altOG && poifiles) && poifiles->next==0) {
      poifiles->altIsOG = altOG; /* exact 1 poi file so altOG switch may be anywhere */
   }
   if (time2sun(dateh, 1024ul, timeh, 1024ul, posa, &sunazim, &sunelev)) {
      /*    dateh:=""; timeh:=""; */
      sunaz = sunazim;
      sunel = sunelev;
   }
   else if (aprsstr_StrToFix(&sunazim, dateh,
                1024ul) && aprsstr_StrToFix(&sunelev, timeh, 1024ul)) {
      if ((float)fabs(sunelev)>90.0f) {
         Error("-S <deg> <deg> or yyyymmdd hhmmss", 34ul);
      }
      dateh[0] = 0;
      timeh[0] = 0;
      sunaz = sunazim;
      sunel = sunelev;
   }
   if ((!posvalidl(posb) && posvalidl(posa)) && dist>0.01f) {
      pan = pan*1.7453292519444E-2f;
      posb.lat = posa.lat+X2C_DIVL(cos((double)pan)*(double)dist,
                6370.0);
      posb.long0 = posa.long0+X2C_DIVL((X2C_DIVL(sin((double)pan),
                cos(posa.lat)))*(double)dist,6370.0);
      /*WrFixed(posb.lat/RAD, 5,10); WrFixed(posb.long/RAD, 5,10); */
      /*WrFixed(azimuth(posa, posb), 2,10);
                WrFixed(distance(posa, posb), 3,10); WrStrLn(""); */
      if (!posvalidl(posb)) {
         Error("camera sight point out of database", 35ul);
      }
   }
   if (sunel<(-90.0f)) {
      autosun(&sunaz, &sunel, posa, azimuthl(posa, posb));
      /*    IF verb THEN Werr('no "-S",
                default sun position generatet'+LF) END; */
      wrsun(sunaz, sunel);
   }
   treeshrink = X2C_DIVR(1.0f,zerotreealt-fulltreealt);
   if (alticolour && logfilm<0.0f) logfilm = 0.0f;
} /* end Parms() */


static void readcolourfile(struct ALTCOLTAB * col, const char fn[],
                uint32_t fn_len)
{
   int32_t fd;
   char fb[30001];
   char b[101];
   int32_t tag;
   int32_t x;
   int32_t j;
   int32_t p;
   int32_t i;
   int32_t l;
   struct CCOL hc;
   memset((char *)col,(char)0,sizeof(struct ALTCOLTAB));
   if (fn[0UL]==0) return;
   fd = osi_OpenRead(fn, fn_len);
   if (fd<0L) Error("Colourtable file not found", 27ul);
   l = osi_RdBin(fd, (char *)fb, 30001u/1u, 30001UL);
   if (l<=0L) Error("Colourtable file not readable", 30ul);
   if (l==30001L) Error("Colourtable too long", 21ul);
   p = 0L;
   j = 0L;
   tag = -1L;
   for (;;) {
      i = 0L;
      while (p<l && (uint8_t)fb[p]<=' ') ++p;
      while ((p<l && i<100L) && (uint8_t)fb[p]>' ') {
         b[i] = fb[p];
         ++i;
         ++p;
      }
      b[i] = 0;
      if (col->len>1023UL) Error("too much Colours", 17ul);
      if (i>0L) {
         if (j==0L && b[0U]=='w') tag = 0L;
         else if (j==0L && b[0U]=='t') tag = 1L;
         else if (j==0L && b[0U]=='u') tag = 2L;
         else if (j==0L && b[0U]=='s') tag = 3L;
         else if (j==0L && b[0U]=='x') tag = 4L;
         else if (j==0L && b[0U]=='y') tag = 5L;
         else {
            if (!aprsstr_StrToInt(b, 101ul, &x)) {
               Error("Junc in Colourfile", 19ul);
            }
            if (x<0L) x = 0L;
            else if (x>255L) x = 255L;
            if (j==0L) {
               hc.r = (uint32_t)x;
               ++j;
            }
            else if (j==1L) {
               hc.g = (uint32_t)x;
               ++j;
            }
            else {
               hc.b = (uint32_t)x;
               j = 0L;
               if (tag>=0L) {
                  col->tag[tag] = hc;
                  tag = -1L;
               }
               else {
                  col->c[col->len] = hc;
                  ++col->len;
               }
            }
         }
      }
      else break;
   }
} /* end readcolourfile() */


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
   if (defaultocean && alt>20000.0f) alt = 0.0f;
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

/* trace goes back */

static void Shadowmask(int32_t xi)
{
} /* end Shadowmask() */


static void colblend(float v, const struct COL c0, const struct COL c1,
                struct COL * c)
{
   float vv;
   if (v<0.0f) v = 0.0f;
   else if (v>1.0f) v = 1.0f;
   vv = 1.0f-v;
   c->r = c0.r*vv+c1.r*v;
   c->g = c0.g*vv+c1.g*v;
   c->b = c0.b*vv+c1.b*v;
} /* end colblend() */


static void azieletorotvec(float az, float el, struct ROTVEC * n)
{
   n->sx = (double)osic_sin(az);
   n->cx = (double)osic_cos(az);
   n->sy = (double)osic_sin(el);
   n->cy = (double)osic_cos(el);
} /* end azieletorotvec() */

#define panorama_TESTDIST 10.0


static void normvec(const struct POSITIONL pos, libsrtm_pMETAINFO pmeta,
                float * slant, float * h, struct NORMVEC * nvec,
                uint8_t * water, uint8_t * wood, uint8_t * urban)
{
   float resol;
   float sqxy;
   uint8_t attr;
   pmeta->withslant = 1;
   pmeta->aliasattr = 1;
   *h = libsrtm_getsrtmlong(pos.lat, pos.long0, 1UL, 1, &resol, &attr,
                pmeta);
   if (defaultocean && (*h>20000.0f || *h==0.0f && resol>=900.0f)) {
      *h = 0.0f;
      pmeta->attrweights[3U] = 1.0f;
   }
   *water = (uint8_t)(uint32_t)X2C_TRUNCC(pmeta->attrweights[3U]*100.0f,
                0UL,X2C_max_longcard);
   if (treesize>0.0f) {
      *wood = (uint8_t)(uint32_t)
                X2C_TRUNCC(pmeta->attrweights[2U]*100.0f,0UL,
                X2C_max_longcard);
   }
   else *wood = 0U;
   *urban = (uint8_t)(uint32_t)X2C_TRUNCC(pmeta->attrweights[1U]*100.0f,
                0UL,X2C_max_longcard);
   /*  IF NOT alticolour THEN */
   /*dir:=atang2(meta.slantx, meta.slanty); */
   /*slant:=1.0-osi.arctan(osi.sqrt(sqr(meta.slantx)+sqr(meta.slanty)))
                *(0.5/PI); */
   sqxy = sqr(pmeta->slantx)+sqr(pmeta->slanty);
   *slant = osic_cos(osic_arctan(osic_sqrt(sqxy))); /* do it faster */
   /*  END; */
   nvec->z = X2C_DIVR(1.0f,osic_sqrt(sqxy+1.0f));
   nvec->x = pmeta->slantx*nvec->z;
   nvec->y = pmeta->slanty*nvec->z;
} /* end normvec() */

/*
PROCEDURE normvectree(VAR th:TREEHINT);
BEGIN
  WITH th DO
    IF hitradius<1.0 THEN
      nvec.x:=-hitx; nvec.y:=-hity;
                nvec.z:=1.0-osi.sqrt(hitradius)*treesize/peakh;
    ELSE nvec.x:=0.0; nvec.y:=0.0; nvec.z:=1.0; END;
  END;
END normvectree;
*/
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

static void suncolour(float sunele, struct COL * cfar, float * sunlum)
{
   float es;
   float s;
   s = 1.0f-sunele*6.3661977238578E-1f; /* sunlight colour */
   if (s<0.001f) s = 0.001f;
   es = (float)pow((double)s, 12.0);
   if (es>1.0f) es = X2C_DIVR(1.0f,es);
   cfar->r = 1.0f-es*0.1f;
   cfar->g = 1.0f-es*0.6f;
   cfar->b = 1.0f-es*1.0f;
   *sunlum = cfar->r*0.3f+cfar->g*0.5f+cfar->b*0.2f;
                /* luminance part of sun */
   if (sunele<0.0f) {
      s = 1.0f+sunele*5.0f;
      if (s<0.05f) s = 0.05f;
      *sunlum =  *sunlum*s;
   }
} /* end suncolour() */


static void shadowcolour(float shadowval, const struct COL suncol,
                struct COL * shadwocol)
{
   if (shadowval<0.0f) shadowval = 0.0f;
   else if (shadowval>1.0f) shadowval = 1.0f;
   shadwocol->r = suncol.r*1.0f*shadowval;
   shadwocol->g = suncol.g*0.9f*shadowval;
   shadwocol->b = suncol.b*0.6f*shadowval;
} /* end shadowcolour() */


static float irand(double x, double y)
/* bilinear interpolated random map */
{
   float fy;
   float fx;
   float d;
   float c;
   float b;
   float a;
   uint32_t iyy;
   uint32_t ixx;
   uint32_t iy;
   uint32_t ix;
   x = fabs(x);
   y = fabs(y);
   ix = (uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   fx = (float)(x-(double)ix);
   iy = (uint32_t)X2C_TRUNCC(y,0UL,X2C_max_longcard);
   fy = (float)(y-(double)iy);
   ix = ix&1023UL;
   iy = iy&1023UL;
   ixx = ix+1UL&1023UL;
   iyy = iy+1UL&1023UL;
   a = noisemap[ix][iy];
   b = noisemap[ixx][iy];
   c = noisemap[ix][iyy];
   d = noisemap[ixx][iyy];
   a = a*(1.0f-fx)+b*fx;
   b = c*(1.0f-fx)+d*fx;
   return a*(1.0f-fy)+b*fy;
} /* end irand() */


static float qint(float a, float b, float c, float v)
/* spline interpolator */
{
   a = (a-b)*0.5f;
   c = (c-b)*0.5f;
   return b+(v*v+0.25f)*(a+c)+v*(c-a);
} /* end qint() */


static float qrand(double x, double y)
/* biqubic interpolated random map */
{
   float fy;
   float fx;
   uint32_t iyyy;
   uint32_t ixxx;
   uint32_t iyy;
   uint32_t ixx;
   uint32_t iy;
   uint32_t ix;
   x = fabs(x);
   y = fabs(y);
   ix = (uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   fx = (float)(x-(double)ix);
   iy = (uint32_t)X2C_TRUNCC(y,0UL,X2C_max_longcard);
   fy = (float)(y-(double)iy);
   if (fx<0.5f) {
      if (ix>0UL) --ix;
   }
   else fx = fx-1.0f;
   if (fy<0.5f) --iy;
   else fy = fy-1.0f;
   ix = ix&1023UL;
   iy = iy&1023UL;
   ixx = ix+1UL&1023UL;
   iyy = iy+1UL&1023UL;
   ixxx = ix+2UL&1023UL;
   iyyy = iy+2UL&1023UL;
   return qint(qint(noisemap[ix][iy], noisemap[ixx][iy], noisemap[ixxx][iy],
                fx), qint(noisemap[ix][iyy], noisemap[ixx][iyy],
                noisemap[ixxx][iyy], fx), qint(noisemap[ix][iyyy],
                noisemap[ixx][iyyy], noisemap[ixxx][iyyy], fx), fy);
} /* end qrand() */


static float lrand(float x)
/* linear interpolated random vector */
{
   float fx;
   uint32_t ix;
   x = (float)fabs(x);
   ix = (uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   fx = x-(float)ix;
   return noisevec[ix&8191UL]*(1.0f-fx)+noisevec[ix+1UL&8191UL]*fx;
} /* end lrand() */


static float dlrand(float x)
/* derivative linear interpolated random vector */
{
   uint32_t ix;
   x = (float)fabs(x);
   ix = (uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   return noisevec[ix+1UL&8191UL]-noisevec[ix&8191UL];
} /* end dlrand() */


static void rotveclight(float * xsun, struct NORMVEC v, struct ROTVEC r)
{
   *xsun = (float)(((double)v.x*r.cx+(double)v.y*r.sx)
                *r.cy+(double)v.z*r.sy);
} /* end rotveclight() */


static void treepos(struct POSITIONL pos, char odd,
                struct TREEHINT * th)
{
   double sh;
   double lo;
   double la;
   float dy;
   float dx;
   int32_t ony;
   int32_t onx;
   struct POSITIONL stampos;
   struct TREEHINT * anonym;
   { /* with */
      struct TREEHINT * anonym = th;
      anonym->sx = 3.0f*lrand((float)(pos.long0*treescramblong));
      anonym->sy = 3.0f*lrand((float)(pos.lat*treescramblat));
      la = pos.lat*treerasterlat+(double)anonym->sx;
      lo = pos.long0*treerasterlong+(double)anonym->sy;
      /*dxn:=RANDN*dlrand(pos.long*TREESCAMBLONG); */
      /*dyn:=RANDN*dlrand(pos.lat *TREESCAMBLAT); */
      if (odd) {
         la = la+0.5;
         lo = lo+0.5;
      }
      /*    onx:=VAL(INTEGER, floor(lo+0.25))*2; */
      /*    ony:=VAL(INTEGER, floor(la+0.25))*2; */
      anonym->hity = (fracint(la+0.25, &ony)-0.5f)*2.0f;
                /* radius from center tree pos / rastersize */
      anonym->hitx = (fracint(lo+0.25, &onx)-0.5f)*2.0f;
      onx = onx*2L;
      ony = ony*2L;
      sh = 0.0;
      if (odd) {
         ++onx;
         ++ony;
         sh = 0.5;
      }
      if (anonym->nx!=onx || anonym->ny!=ony) {
         /* another tree */
         /*
         IF (nx=285097) & (ny=712409) THEN
         WrFixed(shla, 3, 9); WrFixed(shlo, 3, 9); WrStrLn("shlalo");
         END;
         */
         anonym->nx = onx;
         anonym->ny = ony;
         stampos.lat = X2C_DIVL((double)anonym->ny*0.5-sh,
                treerasterlat);
         stampos.long0 = X2C_DIVL((double)anonym->nx*0.5-sh,
                treerasterlong);
         dx = 3.0f*lrand((float)(stampos.long0*treescramblong));
         dy = 3.0f*lrand((float)(stampos.lat*treescramblat));
         stampos.lat = stampos.lat-X2C_DIVL((double)dx,treerasterlat);
         stampos.long0 = stampos.long0-X2C_DIVL((double)dy,
                treerasterlong);
         normvec(stampos, &anonym->meta, &anonym->tslant, &anonym->rooth,
                &anonym->nvec, &anonym->water, &anonym->wood,
                &anonym->urban);
         anonym->stamtoeye = distnear(teye, stampos);
      }
   }
} /* end treepos() */


static float treesizealt(float alt)
{
   return treesize*one((zerotreealt-alt)*treeshrink);
/**one(dist*(1.0/SHOWTREEDIST))*/
} /* end treesizealt() */

/*BEGIN RETURN (* one(FLOAT(attw)*0.02-1.0)* *) min(one((2100.0-alt)*0.0025),
                one((slant-0.4)*botanicslant)) END wooddensity; */

static float wooddensity(uint8_t attw, float alt, float slant)
/*BEGIN RETURN FLOAT(attw)*0.01*one((2100.0-alt)*0.0025)*one((slant-0.4)
                *botanicslant) END wooddensity; */
{
   return min0(sqr(one(treesizealt(alt)*0.033f)),
                one((slant-0.4f)*botanicslant));
} /* end wooddensity() */


static uint8_t treeform(struct POSITIONL pos, float eyeh,
                char odd, char * stacked,
                struct TREEHINT * th)
{
   float noise;
   float thickness;
   uint32_t inois;
   char stam;
   struct TREEHINT * anonym;
   /*IF (nx<>285097) OR (ny<>712409) THEN (*debugg:=500.0;*) RETURN 0 END; */
   treepos(pos, odd, th);
   { /* with */
      struct TREEHINT * anonym = th;
      /*WrInt(nx, 10); WrInt(ny, 10); WrStrLn("nxny"); */
      if (anonym->stamtoeye<20.0f) return 0U;
      inois = (uint32_t)CRC[(anonym->ny&255L)+(anonym->nx&255L)*256L];
      noise = (float)inois*1.52587890625E-5f;
      if (anonym->wood<50U || wooddensity(anonym->wood, anonym->rooth,
                anonym->tslant)-(-0.3f)<noise) return 0U;
      /* remove random trees */
      anonym->regionpeak = treesizealt(anonym->rooth)
                *one(0.1f+plus(anonym->tslant-0.4f)*botanicslant);
                /* treesize on altitude and slant */
      /*    areapeak:=regionpeak*one(FLOAT(wood)*0.01+0.5); */
      anonym->areapeak = anonym->regionpeak;
      noise = (float)CRC[anonym->nx*17L+anonym->ny*3L&65535L];
      anonym->peakh = anonym->areapeak*(1.0f-noise*6.103515625E-6f);
                /* random tree size */
      if (anonym->peakh<2.0f) return 0U;
      /*    thickness:=1.0 (*sqr(20.0/peakh)*); */
      /*    thickness:=regionpeak/peakh; */
      /*    hitx:=(pos.long-stampos.long)*treerasterlat*2; */
      /*    hity:=(pos.lat -stampos.lat )*treerasterlong*2; */
      anonym->hitradius = anonym->hitx*anonym->hitx+anonym->hity*anonym->hity;
                 /* radius^2 normalized to treeraster */
      thickness = X2C_DIVR(treerastersize*2.0f,anonym->peakh);
      if (thickness<1.0f) thickness = 1.0f;
      thickness = anonym->hitradius*thickness;
      if (thickness<1.0f) {
         anonym->hith = (float)((double)
                anonym->peakh*(1.0-pow((double)thickness, 0.9)));
      }
      else anonym->hith = 0.0f;
      anonym->og = eyeh-anonym->rooth;
      /*    IF og<0.0 THEN og:=0.0 END; */
      stam = anonym->og<anonym->peakh*(noise+32000.0f)*3.814697265625E-6f;
      anonym->isstam = (anonym->stamtoeye<showstamdist && stam)
                && thickness<=0.01f; /*& (og>0.0)*/
      if ((!anonym->isstam && anonym->stamtoeye<showstamdist) && stam) {
         /* & (og>0.0)*/
         *stacked = 1;
         return 1U;
      }
      /* transparent */
      return 2U;
   }
/* hit tree */
} /* end treeform() */

/*
PROCEDURE treevec(pos:POSITIONL; VAR nvec:NORMVEC);
VAR x,y,r:REAL;
BEGIN
  treepos(pos, x, y);
  r:=osi.sqrt(x*x + y*y);
  IF r<>0 THEN r:=0.5/r ELSE r:=0.0 END;  
  nvec.z:=0.85;
  nvec.x:=-x*r;
  nvec.y:=-y*r;
END treevec;    
*/
#define panorama_LINSTEP 4000.0


static void raytrace(double x0, double y00, double z0,
                double dx, double dy, double dz,
                float maxdist, float altrise, float refrac,
                double mirrordist, double * dist,
                float * subpix, struct POSITIONL * pos,
                TREEHINTS treehints, struct DDSTACK * ddstack,
                char * istree, char * odd, char shad)
{
   float tolw;
   float tol;
   float ypixalt;
   float resol;
   float step;
   float lastsp;
   float spo;
   float sp;
   uint32_t iter;
   uint8_t od;
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
   double h;
   double alt;
   char stk;
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
   meta.aliasattr = 1; /* watch for wood */
   iter = 5UL;
   dist0 = *dist;
   dist1 = *dist;
   wgs84rl(x0+dx*dist1, y00+dy*dist1, z0+dz*dist1, &lat1, &long1, &alt1);
   alt1 = alt1-dist1*dist1*(double)refrac;
   if ((double)ddstack->startdist==*dist && alt1>(double)
                ddstack->stoppalt) ddstack->startdist = 0.0f;
   for (;;) {
      treehints[0].peakh = 0.0f;
      treehints[1].peakh = 0.0f;
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
                /*+mirrordist*/ /* pixel size */
      tol = ypixalt*0.25f;
      if (tol<0.1f) tol = 0.1f;
      h = (double)libsrtm_getsrtmlong(pos->lat, pos->long0,
                (uint32_t)X2C_TRUNCC((float)fabs(ypixalt)*0.5f,0UL,
                X2C_max_longcard), ypixalt<5.0f, &resol, &attr, &meta);
      if (defaultocean && (h>20000.0 || h==0.0 && resol>=900.0f)) {
         h = 0.0; /* no srtm or srtm30=0 is water */
      }
      if (h<10000.0) {
         sp = (float)(alt-h);
         /*IF altrise<0.0 THEN WrFixed(h, 1,7); WrFixed(pos.lat/RAD, 6,12);
                WrFixed(pos.long/RAD, 6,12); END; */
         *odd = 0;
         *istree = 0;
         if (((treesize>0.0f && sp<=treesize) && !alticolour)
                && meta.attrweights[2U]>0.0f) {
            /*& (dist>SHOWTREEDIST)*/
            if (treehints[0].treepass) {
               /* use constant wood higth for poi */
               if (((meta.attrweights[2U]>0.5f && sp<treesizealt((float)h)
                ) && sp<treehints[0].hith) && mirrordist==0.0) {
                  treehints[0].hith = sp;
                  treehints[0].passdist = (float)*dist;
               }
            }
            else {
               stk = 0;
               *odd = 0;
               /*          treehints[FALSE].peakh:=0.0;
                treehints[TRUE].peakh:=0.0; */
               od = treeform(*pos, (float)alt, 0, &stk, &treehints[0]);
               /*          IF od=0 THEN treehints[odd]
                .peakh:=0.0 ELSIF treehints[odd].hith<sp THEN od:=0 END; */
               if (od==2U && (double)
                (treehints[0].hith+treehints[0].rooth)<alt) {
                  od = 0U;
               }
               if (od<2U) {
                  /*            odd:=TRUE; */
                  od = treeform(*pos, (float)alt, 1, &stk, &treehints[1]);
                  /*            IF od=0 THEN treehints[odd]
                .peakh:=0.0 ELSIF treehints[odd].hith<sp THEN od:=0 END; */
                  if (od==2U && (double)
                (treehints[1].hith+treehints[1].rooth)<alt) od = 0U;
                  if (od>0U) *odd = 1;
               }
               if ((stk && mirrordist==0.0) && ddstack->startdist==0.0f) {
                  ddstack->startdist = (float)*dist;
                  ddstack->stoppalt = treehints[*odd].peakh+treehints[*odd]
                .rooth;
               }
               if (od==2U && treehints[*odd].hith>0.0f) {
                  /*& (treehints[odd].rooth+treehints[odd].hith>h)*/
                  *istree = 1;
                  /*            sp:=sp-treehints[odd].hith;
                          (* add treeform to srtm *) */
                  sp = (float)(alt-(double)
                (treehints[*odd].rooth+treehints[*odd].hith));
               }
            }
            tolw = tol*0.25f;
         }
         else tolw = tol;
         if ((double)sp<(double)
                treehints[0].missspace* *dist && *dist!=0.0) {
            /* store lowest pass over earth/tree */
            treehints[0].missspace = (float)(X2C_DIVL((double)sp,
                *dist));
         }
         /*      IF shad THEN AddShadow(dist, h+sp) END; */
         if (sp>tolw) {
            /* over ground */
            if (((sp<=treesize && !alticolour) && meta.attrweights[2U]>0.0f)
                && *dist+mirrordist<(double)showstamdist) step = 0.2f;
            else step = sp*0.25f;
            if (*subpix==0.0f) {
               if (sp<lastsp) lastsp = sp;
               else if (ypixalt!=0.0f) *subpix = X2C_DIVR(lastsp,ypixalt);
            }
         }
         else {
            /*WrFixed(sp, 1,6); WrFixed(lastsp, 1,6); WrFixed(minsp, 1,6);
                WrFixed(step, 1,6); WrStrLn(" go "); */
            if (sp>-tolw) break;
            /* under ground */
            if (mirrordist>0.0) break;
            if (spo<=0.0f) {
               /* again under ground */
               step = sp*0.5f;
            }
            else {
               /*IF mirrordist>0.0 THEN step:=step*0.1 END; */
               /*WrStr(" back-back "); */
               /*EXIT;  */
               --iter;
               if (iter==0UL) break;
               step = (X2C_DIVR(sp,spo-sp))*step;
                /* first time under ground jump back with newton */
            }
         }
      }
      else if ((float)fabs(step)<tolw) {
         /*WrFixed(step, 1,6); WrFixed(spo-sp, 1,7); WrInt(iter, 3);
                WrStrLn(" newton "); */
         /* jump over hole in erth */
         --iter;
         if (iter==0UL) break;
         sp = 0.0f;
         if (step<30.0f) step = 30.0f;
      }
      spo = sp;
      *dist = *dist+(double)step;
      if ((*dist<(-10.0) || *dist>(double)maxdist)
                || alt>maxmountain && dalt>0.0) {
         *dist = (double)(maxdist+100.0f);
                /* we are in dust or heaven */
         break;
      }
   }
   if (*subpix>1.0f) *subpix = 1.0f;
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


static void posdirtoxyz(const struct POSITIONL pos, double az,
                double el, double * xn, double * yn,
                double * zn)
{
   angtoxyz(az, el, sin(-pos.lat), cos(-pos.lat), sin(-pos.long0),
                cos(-pos.long0), xn, yn, zn);
} /* end posdirtoxyz() */


static float seesun(const struct POSITIONL frompos, double alt,
                float sunaz0, float sunele, float maxdist,
                float refrac)
{
   double z0;
   double y00;
   double x0;
   double zn;
   double yn;
   double xn;
   double dd;
   struct POSITIONL pos;
   float space;
   TREEHINTS th;
   char odd;
   char istree;
   struct DDSTACK ddstack;
   posdirtoxyz(frompos, (double)sunaz0, (double)sunele, &xn, &yn,
                 &zn);
   wgs84sl(frompos.lat, frompos.long0, alt, &x0, &y00, &z0);
   memset((char *)th,(char)0,sizeof(TREEHINTS));
   odd = 0;
   dd = 1.0;
   th[0].missspace = X2C_max_real;
   ddstack.startdist = 0.0f;
   raytrace(x0, y00, z0, xn, yn, zn, maxdist, 0.0f, refrac, 0.001, &dd,
                &space, &pos, th, &ddstack, &istree, &odd, 0);
   space = 0.0f;
   if (dd>(double)maxdist) {
      space = th[0].missspace*1.0417414457222E+2f;
      if (space>1.0f) space = 1.0f;
   }
   return space;
} /* end seesun() */


static float inv(float x)
{
   x = (float)fabs(x);
   if (x==0.0f) return X2C_max_real;
   return X2C_DIVR(1.0f,x);
} /* end inv() */

#define panorama_MAXSUNDUST 4000.0
/* 6000 */

#define panorama_LENSEFFECT 40.0

#define panorama_HEAVENDUST 2.0

#define panorama_RAD90 6.3661977238578E-1


static void sundisk(float x, float wy, float sunazi,
                float sunele, float dist, float sunarea,
                float maxdist, uint8_t lenseffect,
                const struct COL suncol, const struct POSITIONL pos,
                float alt, float refrac, float dropsun,
                struct COL * c)
/* dust and all else in heaven */
{
   float lens;
   float ed;
   float s;
   float sd;
   float y;
   struct COL nh;
   struct COL hc;
   /* dist =0 heaven, >0 earth */
   y = radmod(wy-sunele);
   sd = x*x+y*y;
   if (dist==0.0f) {
      /* heaven */
      hc.r = 1.0f;
      hc.g = 1.0f;
      hc.b = 1.0f;
      colblend((float)fabs(x)*0.3f, suncol, hc, &hc);
                /* azimuth sun colour in dust */
   }
   else {
      /* earth */
      hc.r = suncol.r*0.25f;
      hc.g = suncol.g*0.4f;
      hc.b = suncol.b*1.1f;
      s = hc.r*0.15f+hc.g*0.25f+hc.b*0.1f;
      hc.r = hc.r*0.5f+s;
      hc.g = hc.g*0.5f+s;
      hc.b = hc.b*0.5f+s;
      colblend((float)((0.5-sqrt((double)sd)*3.0)*(double)
                sunarea), hc, suncol, &hc);
                /* sun angle dust colour on earth */
   }
   s = 600.0f;
   if (sunele<0.0f) {
      /* night */
      s = 600.0f+sunele*3500.0f;
      if (s<20.0f) s = 20.0f;
   }
   hc.r = s*hc.r;
   hc.g = s*hc.g;
   hc.b = s*hc.b;
   if (dist==0.0f) {
      /* heaven */
      /*
          IF d<=SUNSIZE*SUNSIZE THEN                          (* view into sun *)
            c.r:=suncol.r*SUNLUM;
            c.g:=suncol.g*SUNLUM;
            c.b:=suncol.b*SUNLUM;
       
          ELSE                                                (* heaven not in sun *)
                 
      */
      s = maxdist*4.2441318159052E-5f; /* lesser blue sky on shorter sight */
      if (s>6.3661977238578E-1f) s = 6.3661977238578E-1f;
      if (wy<0.0f) wy = 0.0f;
      ed = 1.0f-wy*s;
      s = 2.0f+x*x+(float)fabs(sunele);
      if (s>6.0f) s = 6.0f;
      if (ed<=0.0f) ed = 0.0f;
      else ed = (float)pow((double)ed, (double)s);
      if (sunele<0.0f) {
         /* night heaven blue */
         s = 1.0f+sunele*10.0f;
         if (s<0.02f) s = 0.02f;
         nh.r = heavencol.r*s;
         nh.g = heavencol.g*s;
         nh.b = heavencol.b*s;
      }
      else nh = heavencol;
      colblend(ed, nh, hc, c); /* blue sky down to dust */
   }
   else {
      /*  END; */
      /* earth */
      colblend(dist, *c, hc, c);
   }
   if (lenseffect==1U) {
      /* direct sun */
      if (sd<=2.3036692370053E-5f) {
         /* view into sun */
         lens = 5000.0f;
      }
      else {
         lens = 0.5f+inv((float)fabs(sin((double)(atang2(x,
                y)*7.0f))));
         if (lens>1.5f) lens = 1.5f;
         lens = X2C_DIVR(lens,sd*250.0f+0.1f);
         lens = lens*sunarea*40.0f;
      }
      c->r = c->r+suncol.r*lens;
      c->g = c->g+suncol.g*lens;
      c->b = c->b+suncol.b*lens;
   }
   else if (lenseffect==2U) {
      /* sun in water */
      /*    lens:=3.0/(d+0.005)*irand(pos.lat*300000.0,
                pos.long*coslat*300000.0); */
      /*    lens:=0.4/(d+0.002)*(0.2+sqr(qrand(pos.lat*20000000.0,
                pos.long*coslat*20000000.0))); */
      lens = (X2C_DIVR((2.0f-sunele)*0.12f,
                x*x+y*y*sunele*3.1830988619289E-1f+0.0005f))
                *(1.0f+sqr(qrand((double)((x+1.0f)*3000.0f),
                (double)((y+1.0f)*3000.0f))));
      if (lens>0.1f) {
         if (!dropshadows) {
            dropsun = seesun(pos, (double)(alt+0.2f), sunazi,
                sunele+4.7996554428472E-3f, maxdist, refrac);
         }
         if (dropsun>0.0f) {
            lens = lens*dropsun;
            c->r = c->r+suncol.r*lens;
            c->g = c->g+suncol.g*lens;
            c->b = c->b+suncol.b*lens;
         }
      }
   }
} /* end sundisk() */

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
   TREEHINTS th;
   char odd;
   char istree;
   struct DDSTACK ddstack;
   r = 0.0f;
   sum = 0.0f;
   memset((char *)th,(char)0,sizeof(TREEHINTS));
   odd = 0;
   for (i = 0UL; i<=7UL; i++) {
      wx = sunaz0+(double)(2.3998277214236E-3f*osic_sin(r));
      wy = sunel0+(double)(2.3998277214236E-3f*osic_cos(r));
      angtoxyz(wx, wy, slat, clat, slong, clong, &xn, &yn, &zn);
      dd = 0.0;
      ddstack.startdist = 0.0f;
      raytrace(x0, y00, z0, xn, yn, zn, maxdist, 0.0f, refrac, 0.0, &dd,
                &space, &pos, th, &ddstack, &istree, &odd, 0);
      /*WrFixed(sum, 2, 7); */
      if (dd>(double)maxdist) sum = sum+0.125f;
      r = r+7.85398163375E-1f;
   } /* end for */
   /*WrStrLn(" sums"); */
   return sum;
} /* end sunvisability() */


static void snowcol(struct COL * c, const struct COL sun, float xsun,
                float sunlum)
{
   float x;
   if (xsun<(-0.3f)) xsun = (-0.3f);
   x = (xsun+2.0f)*sunlum;
   /*  x:=xsun+2.0; */
   c->r = 79.0f*x;
   c->g = 115.0f*x;
   c->b = 220.0f*x;
   if (xsun>0.0f) {
      /* surface sees sun */
      c->r = c->r+840.0f*xsun*sun.r;
      c->g = c->g+830.0f*xsun*sun.g;
      c->b = c->b+820.0f*xsun*sun.b;
   }
} /* end snowcol() */


static void botaniccol(struct COL * c, const struct COL suncol,
                float suncos, float directsun, float diffuslum)
{
   float x;
   /*  x:=(xsun+2.5)*(sun.r+sun.g+sun.b)*0.33333333; */
   if (suncos<(-0.3f)) suncos = (-0.3f);
   x = (suncos+3.5f)*diffuslum;
   c->r = 0.05f*x;
   c->g = 0.055f*x;
   c->b = 0.09f*x;
   if (suncos>0.0f) {
      suncos = suncos*directsun;
      c->r = c->r+suncos*suncol.r;
      c->g = c->g+suncos*suncol.g;
      c->b = c->b+suncos*suncol.b;
   }
} /* end botaniccol() */


static void treecol(struct POSITIONL pos, const struct TREEHINT th,
                const struct ROTVEC sunv, float pixsize,
                float diffuslum, float dense, float snowdense,
                float eyesunaz, float xsun, uint8_t hitwood,
                const struct COL suncol, struct COL * c)
{
   float pixshh;
   float pixst;
   float dens;
   float pixs;
   float geosun;
   float ang;
   float trestruc;
   float rnd;
   float sun;
   float sh;
   float h;
   struct NORMVEC v;
   float bi;
   float gi;
   float ri;
   struct COL s;
   pixst = one(pixsize*0.3f);
   pixs = one(pixsize*0.5f);
   pixshh = one(pixsize*2.0f);
   if (dropshadows) geosun = diffuslum;
   else geosun = one(diffuslum*20.0f);
   ang = 4.71238898025f+atang2(th.hitx-3.0f*dlrand((float)
                (pos.lat*treescramblat))*th.hity,
                th.hity-3.0f*dlrand((float)(pos.long0*treescramblong))
                *th.hitx);
   rnd = irand((double)((float)(th.nx*7L+th.ny*6L&511L)+ang*6.0f),
                (double)((th.og+30.0f)*8.0f)); /* tree structure */
   /*  h:=th.peakh/th.areapeak; */
   /*  IF xsun>0.0 THEN h:=h*(1.0-xsun) + xsun END;
                (* small trees in flat sun *) */
   trestruc = (float)fabs(rnd)*pixst+(1.0f-pixst);
                /* flatten struct on distance */
   if (th.hitradius<1.0f) {
      v.x = -th.hitx;
      v.y = -th.hity;
      v.z = 1.0f-X2C_DIVR(osic_sqrt(th.hitradius)*treesize,th.peakh);
      rotveclight(&sun, v, sunv); /* sun angle effekt */
   }
   else sun = (-1.0f);
   sun = sun+rnd*(1.2f-(float)fabs(sun))*0.1f;
                /* random rotate sun angle */
   if (sun<(-1.0f)) sun = (-1.0f);
   else if (sun>1.0f) sun = 1.0f;
   if (xsun>0.0f) {
      xsun = xsun*((1.0f-xsun)*((float)fabs(eyesunaz)
                *1.5915494309644E-1f+0.5f)+xsun);
                /* cos median wood brightness */
   }
   sun = pixs*sun+(1.0f-pixs)*xsun; /* lower individual sun angle on dist */
   /*  sun:=pixs*treesun + (1.0-pixs)*mainsun;
                (* lower individual sun angle on dist *) */
   dens = dense*one((float)hitwood*0.02f-1.0f);
   /*green */
   if (dropshadows) sh = 1.0f;
   else sh = sun*0.15f+0.85f;
   if (th.isstam) {
      /* stam */
      /*    snowdense:=4.0*(snowdense-0.75); */
      /*    sh:=sh*(1.0-FLOAT(th.wood)*0.005);
                (* stam darker deep in wood *) */
      /*    snowcol(rs,gs,bs, sunr,sung,sunb, sh); */
      snowdense = 0.0f;
      h = irand((double)(float)(th.nx*7L+th.ny*5L),
                (double)(float)(th.nx*4L+th.ny*9L))*20.0f;
      sh = sh*trestruc;
      ri = sh*140.0f;
      gi = sh*(130.0f+h);
      bi = sh*(120.0f+h);
   }
   else {
      /*    snowdense:=one(snowdense*2.0-pixsh*(2.0-ABS(qrand(FLOAT((th.nx*13+th.ny*6)
                 MOD 512)+ang*3.0, th.og*4.0)))); */
      /*    snowdense:=snowdense*pixsh + snowdense*qrand(FLOAT(th.nx),
                FLOAT(th.ny))*(0.1-pixsh)
                *pixshh + snowdense*(0.35-pixshh*0.35); */
      snowdense = one(snowdense*1.6f-pixs*(2.0f-(float)
                fabs(qrand((double)((float)(th.nx*13L+th.ny*6L&511L)
                +ang*3.0f), (double)(th.og*4.0f)))));
      snowdense = snowdense*pixs+snowdense*qrand((double)(float)
                th.nx, (double)(float)th.ny)*(0.1f-pixs)*pixshh+snowdense*(0.35f-pixshh*0.35f);
      if (dropshadows) h = trestruc;
      else {
         h = sh*(1.0f-one((0.8f-X2C_DIVR(th.hith,
                th.areapeak))*2.5f)*1.2f*dens*pixs);
                /* use relative hight dark in wood */
         h = h+pixs*one((X2C_DIVR(th.hith,th.regionpeak)-0.8f)*4.0f)*0.6f;
                /* illuminate high peaks */
         h = h*trestruc;
      }
      if (h<0.08f) h = 0.08f;
      ri = 210.0f*(h+0.1f);
      gi = 290.0f*h;
      bi = 200.0f*(h+0.1f);
   }
   ri = ri*(1.0f+irand((double)(float)(th.nx*9L+th.ny*12L),
                (double)(float)(th.nx*3L+th.ny))*0.025f);
                /* colour variance of trees */
   gi = gi*(1.0f+irand((double)(float)(th.nx*7L+th.ny*5L),
                (double)(float)(th.nx*4L+th.ny*5L))*0.08f);
   botaniccol(c, suncol, sun, geosun, diffuslum);
   /* c:COL; suncol-:COL; suncos, directsun, diffuslum:REAL */
   c->r = c->r*ri;
   c->g = c->g*gi;
   c->b = c->b*bi;
   /*snowdense:=1.0; */
   if (snowdense!=0.0f) {
      /*    snowcol(s,  sunlight, (sun*pixs + mainsun*0.5)*(1.0-pixs*0.5),
                sunlum); */
      snowcol(&s, suncol, sun, geosun);
      colblend(snowdense, *c, s, c); /* mix green with snow */
   }
} /* end treecol() */

/*
PROCEDURE treecol(pos:POSITIONL; th-:TREEHINT; sunv-:ROTVEC;
                  pixsize, diffuslum, dense, snowdense, eyesunaz, xsun:REAL;
                hitwood:CARD8;
                  suncol-:COL; VAR c:COL);

VAR h,sh,sun, rnd, trestruc, ang, geosun, pixs, dens, pixsh, pixshh, hxx,
                hxy:REAL;
    v:NORMVEC;
    ri, gi, bi:REAL;
    s, ss:COL;
BEGIN
  pixsize:=plus((pixsize-0.1)*1.2);

  pixs:=one(pixsize*0.4);
  pixsh :=one(pixsize*0.8);
  pixshh:=one(pixsize*2.0);
  IF dropshadows THEN geosun:=diffuslum ELSE geosun:=one(diffuslum*20.0) END;

  ang:=PI*1.5+atang2(th.hitx - TREERAND*dlrand(pos.lat *treescramblat)
                *th.hity,
                 th.hity - TREERAND*dlrand(pos.long*treescramblong)*th.hitx);
  rnd:=irand(FLOAT((th.nx*7+th.ny*6) MOD 512)+ang*6.0, (th.og+30.0)*8.0);
                (* tree structure *)
  trestruc:=ABS(rnd)*pixs + plus(0.75-pixs);


  IF pixs>0.0 THEN                                            (* else save cpu *)
    IF th.hitradius<1.0 THEN
      v.x:=-th.hitx; v.y:=-th.hity;
                v.z:=1.0-osi.sqrt(th.hitradius)*treesize/th.peakh;
      rotveclight(sun, v, sunv);                               (* sun angle effekt *)
    ELSE sun:=-1.0 END;

    sun:=sun+rnd*(1.2-ABS(sun))*0.1;
    IF sun<-1.0 THEN sun:=-1.0 ELSIF sun>1.0 THEN sun:=1.0 END;  


    sun:=pixs*sun + (1.0-pixs)*diffuslum;
                (* lower individual sun angle on dist *)
  ELSE sun:=diffuslum END;
  dens:=dense*one(FLOAT(hitwood)*0.02-1.0);

--green
  IF dropshadows THEN sh:=1.0 ELSE sh:=(sun*0.15+0.85) END;
                                      (* sun angle effekt *)

  IF th.isstam THEN                                           (* stam *)
    snowdense:=0.0;
    h:=irand(FLOAT(th.nx*7+th.ny*5), FLOAT(th.nx*4+th.ny*9))*cSTAMVAR; 
    sh:=sh*trestruc;
    ri:=sh*cSTAMr;
    gi:=sh*(cSTAMg+h);
    bi:=sh*(cSTAMb+h);
  ELSE
    snowdense:=one(snowdense*1.6-pixsh*(2.0-ABS(qrand(FLOAT((th.nx*13+th.ny*6)
                 MOD 512)+ang*3.0, th.og*4.0))));
    snowdense:=snowdense*pixsh + snowdense*qrand(FLOAT(th.nx),
                FLOAT(th.ny))*(0.1-pixsh)
                *pixshh + snowdense*(0.35-pixshh*0.35);

    IF dropshadows THEN h:=trestruc ELSE
      h:=sh*(1.0-one((0.8-th.hith/th.areapeak)*2.5)*1.2*dens*pixsh);
                (* use relative hight dark in wood *) 
      h:=h+pixsh*one((th.hith/th.regionpeak-0.8)*4.0)*0.6;
                (* illuminate high peaks *) 
      h:=h*trestruc;
    END;
    IF h<0.08 THEN h:=0.08 END;
    ri:=cTREEr*(h+0.1);
    gi:=cTREEg*h;
    bi:=cTREEb*(h+0.1);
  END;

  ri:=ri*(1.0+irand(FLOAT(th.nx*9+th.ny*12), FLOAT(th.nx*3+th.ny))*0.025);
                (* colour variance of trees *)
  gi:=gi*(1.0+irand(FLOAT(th.nx*7+th.ny*5), FLOAT(th.nx*4+th.ny*5))*0.08);
  botaniccol(c, suncol, sun, geosun, diffuslum);

-- c:COL; suncol-:COL; suncos, directsun, diffuslum:REAL

  c.r:=c.r*ri;
  c.g:=c.g*gi;
  c.b:=c.b*bi;
--snowdense:=1.0;
  IF snowdense<>0.0 THEN
    snowcol(s,  suncol, sun, geosun);
    colblend(snowdense, c, s, c);             (* mix green with snow *)
  END;

END treecol;
*/
#define panorama_PARCEL 2.1E+5


static void citycol(uint8_t attu, const struct POSITIONL pos,
                float pixs, struct COL * c, struct COL * lamp)
{
   float sh;
   float vis;
   float l;
   float ay;
   float ax;
   float gravel;
   float hw;
   float hr;
   float street;
   int32_t rx;
   int32_t ry;
   struct COL hc;
   uint32_t i;
   lamp->r = 0.0f;
   lamp->g = 0.0f;
   lamp->b = 0.0f;
   if (attu>0U && urbanoff!=0.0f) {
      ax = fracint(pos.lat*2.1E+5, &rx);
      ay = fracint(pos.long0*coslat*2.1E+5, &ry);
      street = min0(ax, ay);
      hw = 10.0f*irand((double)rx, (double)ry);
      hr = 0.25f*frac((double)hw);
      vis = (float)attu*0.01f*one(4.0f-(X2C_DIVR(pixs,urbanoff))*4.0f);
      if (street<0.07f) {
         hc.r = 258.0f; /* meadow */
         hc.g = 366.0f;
         hc.b = 292.0f;
      }
      else if (street<0.23f+hr) {
         hc.r = 249.0f; /* street */
         hc.g = 282.0f;
         hc.b = 282.0f;
      }
      else if (street<0.3f+hr) {
         hc.r = 258.0f; /* meadow */
         hc.g = 366.0f;
         hc.b = 292.0f;
      }
      else {
         /* house */
         gravel = irand(pos.lat*3.E+7, pos.long0*coslat*3.E+7);
         hr = one(hw-8.0f);
         hw = one(hw+3.0f);
         gravel = 1.0f-(1.0f-one((float)sqrt((double)(float)
                fabs(gravel))))*0.8f*(hw-hr)*one(pixs*0.1f);
         hc.r = 500.0f*(0.8f+hw)*gravel;
         hc.g = 500.0f*((1.0f+hw)-hr)*gravel;
         hc.b = 500.0f*((1.0f+hw)-hr)*gravel;
      }
      /*lights */
      i = (uint32_t)CRC[((uint32_t)ry&255UL)+((uint32_t)rx&255UL)
                *256UL];
      sh = (float)(1UL+(i/32UL&7UL));
      switch (i&31UL) {
      case 0UL:
         lamp->r = 0.4f;
         lamp->g = 0.1f;
         lamp->b = 0.0f;
         sh = 1.0f;
         break;
      case 1UL:
      case 2UL:
      case 3UL:
      case 4UL:
      case 5UL:
      case 6UL:
      case 7UL:
         lamp->r = 0.3f;
         lamp->g = 0.24f;
         lamp->b = 0.2f;
         break;
      case 8UL:
      case 9UL:
      case 10UL:
      case 11UL:
      case 12UL:
      case 13UL:
      case 14UL:
      case 15UL:
         lamp->r = 0.3f;
         lamp->g = 0.3f;
         lamp->b = 0.3f;
         break;
      } /* end switch */
      l = X2C_DIVR(vis*sh,sqr(ax-0.5f)+sqr(ay-0.5f)+0.002f*sh);
      lamp->r = lamp->r*l;
      lamp->g = lamp->g*l;
      lamp->b = lamp->b*l;
      /*-lights */
      colblend(vis, *c, hc, c);
   }
} /* end citycol() */


static float snownoise(const struct POSITIONL pos, float sunang,
                float pixs)
{
   float f;
   float s;
   float h;
   f = qrand(pos.long0*4.E+7, pos.lat*4.E+7);
   f = f*f*f*0.2f;
   if (f>1.0f) f = 1.0f;
   else if (f<(-1.0f)) f = (-1.0f);
   f = f*one(pixs*0.3f);
   h = qrand(pos.long0*6.E+6*coslat, pos.lat*6.E+6); /* near */
   h = h*h*h*0.1f;
   if (h>1.0f) h = 1.0f;
   else if (h<(-1.0f)) h = (-1.0f);
   h = f+h*one(pixs*1.5f)*plus(1.0f-pixs*0.3f);
   s = qrand(pos.long0*5.2E+5*coslat, pos.lat*5.2E+5); /* far */
   /*  h:=h+s*0.1; */
   h = (sunang-0.1f)+s*0.02f+(X2C_DIVR(h,(float)fabs(sunang)+1.0f))*0.1f;
   if (h>1.0f) h = 1.0f;
   else if (h<(-1.0f)) h = (-1.0f);
   return h;
} /* end snownoise() */


static void meadowcol(struct POSITIONL pos, float slant, float alt,
                float pixs, float sunang, float * sunvec,
                struct COL * c)
{
   float sv;
   float m;
   float f;
   float d;
   float k;
   float brown;
   float q;
   float s;
   float h;
   struct COL hc;
   /*  ops:=one(pixs*0.5); */
   pos.long0 = pos.long0*coslat;
   f = qrand(pos.long0*3.E+7, pos.lat*3.E+7);
   h = qrand(pos.long0*9.E+6, pos.lat*9.E+6);
   s = qrand(pos.long0*8.E+6, pos.lat*8.E+6);
   m = qrand(pos.long0*2.2E+6, pos.lat*2.2E+6);
   k = qrand(pos.long0*5.5E+5, pos.lat*5.5E+5);
   s = s+one(pixs*0.2f)*f;
   sv = 1.0f-0.25f*f*one(pixs*0.15f);
   /*  flower:=plus(1.0-ABS(f*f-0.05)*3.0)*one(pixs*0.15)*one(1.0-pixs*0.05)
                *one((s-0.95)*4.0); */
   /*  brown:=one(s*4.0-1.0)*(1.0-ops) + one(m*4.0)*ops;
                (* high altitude green/yellow noise *) */
   brown = one((1.0f-slant*slant)+one((float)((double)
                alt*(0.00025+fabs(pos.lat*0.0003))+(double)
                (((alt+1000.0f)-snowalt)*0.001f)))+one(X2C_DIVR((alt-snowalt)
                +slowaltslant*2.0f,slowaltslant))*0.6f);
                /*(brown+2.0)*/ /* slant brown */
   /*                       + one((alt+1000.0-snowalt)*0.0008)
                (* altitude brown *) */
   /* altitude brown */
   /* snow disturb */
   q = h*h;
   q = q*q;
   d = (k+m)*0.04f*(1.7f-one(pixs*2.0f)); /* far variance */
   *sunvec = s*one(pixs*3.0f)*0.6f;
   /*
     c.r:=cMEADOWr*sv*plus(1.0 + brown*0.2 - d - (h+q)*0.05*ops);
     c.g:=cMEADOWg*sv*plus(1.0 - brown*0.12 + d + (s*s*s*0.01 + s*0.015)*ops)
                ;
     c.b:=cMEADOWb*sv*plus(1.0 + brown*0.15 - q*0.05*ops);
   */
   c->r = 310.0f*sv*plus((1.0f+brown*0.2f)-d);
   c->g = 430.0f*sv*plus((1.0f-brown*0.12f)+d);
   c->b = 235.0f*sv*plus(1.0f+brown*0.15f);
   hc.r = 458.0f;
   hc.g = 458.0f;
   hc.b = 429.0f;
   colblend(one(k*k*20.0f)+(float)fabs(s)*one(0.25f+pixs), hc, *c, c);
                /* meadow voids */
/*
  IF flower>0.0 THEN                                          (* else save time *)
    c.r:=c.r+flower*one(h*4.0)*160.0;
    c.g:=c.g+flower*one((h+m-1.0)*3.0)*100.0;
    c.b:=c.b+flower*one(m*5.0)*170.0;
  END;
*/
} /* end meadowcol() */


static void desert(const struct POSITIONL pos, float pixs,
                struct COL * c)
{
   float n;
   n = qrand(pos.long0*8.E+6, pos.lat*8.E+6);
   n = n+one(pixs*0.4f)*qrand(pos.long0*3.E+7, pos.lat*3.E+7);
   n = n+one(pixs)*qrand(pos.long0*4.5E+5, pos.lat*4.5E+5);
   n = n+qrand(pos.lat*1.17E+5, pos.long0*1.23E+5);
   n = 0.05f+plus(1.0f+n*0.05f*sandgranularity);
   c->r = desertcol.r*n;
   c->g = desertcol.g*n;
   c->b = desertcol.b*n;
} /* end desert() */


static void rockcol(struct POSITIONL pos, float slant, float alt,
                float pixs, float sunang, uint8_t attw,
                char istree, float * lastsunv, struct COL * c)
/*glance-:GLANCESET;*/
{
   float nh;
   float rock;
   float na;
   float n2;
   float n1;
   float ns;
   float nr;
   struct COL hc;
   pos.long0 = pos.long0*coslat;
   n1 = qrand(pos.long0*9.E+6, pos.lat*9.E+6)*one(pixs);
   nr = n1+qrand(pos.long0*1.1E+5, pos.lat*11000.0);
   rock = one((1.1f*one(plus(((alt+slowaltslant*0.5f*(float)(uint32_t)
                istree)-2200.0f)*0.005f))+plus(1.0f-plus(((slant-0.4f)
                -(float)(100U-attw)*0.002f)+nr*0.1f)*botanicslant))-0.1f);
   /* rock / green */
   rock = rock+rock*0.3f*one(qrand(pos.long0*1.5E+6,
                pos.lat*1.5E+6)*5.0f-2.0f); /* rock / green noise */
   if (rock>0.0f) {
      /* else save time */
      if (desertcol.r<0.0f) {
         /* not user defined colour */
         nh = qrand(pos.long0*8.E+6, pos.lat*8.E+6);
         n2 = nh+one(pixs*0.4f)*qrand(pos.long0*3.E+7, pos.lat*3.E+7);
         ns = qrand(pos.long0*4.5E+5, pos.lat*4.5E+5);
         na = qrand((double)(alt*3.0f+nr*5.0f),
                pos.lat*1.17E+5+pos.long0*1.23E+5);
                /* altitude dependent structure */
         na = one(na*10.0f)*0.3f*(float)fabs(ns)*one(pixs);
         n2 = (n2+ns*0.5f)*one(pixs*2.0f);
         na = na-(ns*0.3f+nh*nh*0.7f*one(pixs*0.5f))*plus(1.0f-pixs*3.0f);
                /* far rock structure */
         hc.r = 418.0f*(na+plus(0.95f+one(n1*n1*n1-0.1f)*0.15f+n2*0.07f));
         hc.g = 458.0f*(na+one(0.95f+n2*0.1f));
         hc.b = 448.0f*(na+one(0.95f+n2*0.15f));
      }
      else {
         desert(pos, pixs, &hc);
         if (sandgranularity<0.0f) rock = 1.0f;
         /*      addglance(pos, glance, ABS(sandgranularity),
                sandgranularity, sunang, pixs, hc); */
         rock = rock*1.1f;
         /*addglance(pos, glance, ABS(sandgranularity), sandgranularity,
                sunang, pixs, hc); */
         n2 = 0.0f;
      }
      colblend(rock, *c, hc, c);
      *lastsunv = rock*n2*0.4f+(1.0f-rock)* *lastsunv;
                /* blend meadow/rock sun vector variance */
   }
   *lastsunv = sunang+ *lastsunv*0.05f;
   if (desertcol.r<0.0f) {
      *lastsunv = sunang*(0.95f+X2C_DIVR(0.005f,sqr(sunang)+0.005f));
   }
} /* end rockcol() */


static float snowdensity(struct POSITIONL pos, float slant,
                float alt, float pixs, const struct TREEHINT th,
                uint8_t attw)
{
   float h;
   float s;
   h = 0.75f-sqr(qrand(pos.long0*1.E+6*coslat, pos.lat*1.E+6));
   h = h*one(pixs)*0.2f+qrand(pos.long0*1.E+7*coslat,
                pos.lat*1.E+7)*0.2f*one(pixs*0.4f);
   h = h*(0.7f-newsnow*0.5f);
   if ((treesize>0.0f && attw>0U) && th.peakh>0.0f) {
      h = h-0.25f*one((float)((1.0-sqrt((double)th.hitradius))*2.0))
                ; /* less snow under tree */
   }
   s = X2C_DIVR(alt-snowalt,slowaltslant);
   s = one((s+slant*slant)*slowaltslant*0.005f+0.5f+h);
   return s*one((float)((pow((double)slant, 0.5)-0.5)*5.0));
/* never snow on big slant */
} /* end snowdensity() */


static void darkundertree(uint8_t attw, float wooddens,
                float snowdense, const struct TREEHINT th,
                struct COL * c)
/* dark under tree */
{
   float a;
   struct COL hh;
   struct COL hc;
   if (treesize>0.0f && attw>0U) {
      /* tree enabled and tree area */
      a = one(1.3f-(float)attw*one(wooddens)*treesize*0.0005f);
      if (th.peakh>0.0f) {
         a = min0(a, (float)(0.3+0.7*sqrt((double)th.hitradius)));
                /* dark disk under tree */
      }
      hc.r = c->r*0.15f;
      hc.g = c->g*0.2f;
      hc.b = c->b*0.12f;
      hh.r = c->r*0.4f;
      hh.g = c->g*0.4f;
      hh.b = c->b*0.4f;
      colblend(snowdense, hc, hh, &hc); /* snow or not under trees */
      colblend(a, hc, *c, c);
   }
} /* end darkundertree() */


static void underwatercol(struct POSITIONL pos, float wy, float pixs,
                float sunlum, uint8_t attw, struct COL * c)
{
   float h;
   struct COL hc;
   h = one(pixs)*qrand(pos.long0*8.E+6, pos.lat*8.E+6);
   h = h+one(pixs*0.2f)*qrand(pos.long0*2.5E+7, pos.lat*2.5E+7);
   h = one(h*h);
   hc.r = 260.0f*(1.0f+h*0.4f)*sunlum;
   hc.g = 210.0f*(1.0f+h*0.5f)*sunlum;
   hc.b = 180.0f*sunlum;
   colblend(((one(2.0f+wy*12.0f)+1.0f)-one(pixs))+(float)attw, hc, *c, c);
                 /* underwater/coast colour*/
} /* end underwatercol() */

#define panorama_WATERSURFACEDUST 0.07


static void wateralias(float wy, float snow, float wsh,
                float shad, float sunel0, float sunlum,
                const struct COL wg, struct COL * c)
{
   struct COL h;
   float lum;
   if (sunel0<0.0f) sunel0 = 0.0f;
   lum = (0.07f+shad*sunel0)*sunlum; /* sun into water */
   h.r = (110.0f+snow*0.006f)*lum;
   h.g = (271.0f+snowalt*0.012f)*lum;
                /* guess temperature and botanic activity */
   h.b = 330.0f*lum;
   /*  colblend(one(ABS(wy*2.5))*0.90, c, h, c); */
   colblend((float)sqrt((double)((float)fabs(wy)*0.6f+0.03f)),
                *c, h, c);
   colblend(wsh, wg, *c, c); /* water end aliasing */
} /* end wateralias() */


static void storepix(imagetext_pIMAGE image0, uint32_t x, uint32_t y,
                struct COL c, float alias, char heaven)
{
   struct COL h;
   struct imagetext_PIX * anonym;
   struct imagetext_PIX * anonym0;
   /*IF heaven THEN INC(debugyi) ELSE INC(debugxi) END; */
   if (alticolour) {
      if (heaven) c.b = 6.0f;
   }
   else {
      c.r = c.r*8.0f;
      c.g = c.g*8.0f;
      c.b = c.b*8.0f;
   }
   if (c.r>30000.0f) c.r = 30000.0f;
   if (c.g>30000.0f) c.g = 30000.0f;
   if (c.b>30000.0f) c.b = 30000.0f;
   { /* with */
      struct imagetext_PIX * anonym = &image0->Adr[(x)*image0->Len0+y];
      anonym->r = (uint16_t)truncc(c.r);
      anonym->g = (uint16_t)truncc(c.g);
      anonym->b = (uint16_t)((truncc(c.b)/2UL)*2UL+(uint32_t)heaven);
   }
   if (!alticolour && y>0UL) {
      { /* with */
         struct imagetext_PIX * anonym0 = &image0->Adr[(x)
                *image0->Len0+(y-1UL)];
         if (alias<1.0f) {
            h.r = (float)anonym0->r;
            h.g = (float)anonym0->g;
            h.b = (float)anonym0->b;
            colblend(alias, h, c, &c); /* antialiasing */
         }
         anonym0->r = (uint16_t)truncc(c.r);
         anonym0->g = (uint16_t)truncc(c.g);
         anonym0->b = (uint16_t)((truncc(c.b)/2UL)*2UL+(uint32_t)
                ((anonym0->b&1) || heaven));
      }
   }
} /* end storepix() */

#define panorama_FULLUM 1000.0
/* full bright in image */

#define panorama_MAXHP 0.05
/* vertical spatial luminance highpass */

#define panorama_DDIST 50.0

#define panorama_LIGHT90 0.35
/* luminance at 90 deg sun */

#define panorama_SELFSHADOW 0.6
/* dark/light of surface 90/180 deg to sun */

#define panorama_WAVLEN 5.E+6

/*    camvec:LVEC; */

static void Panofind(uint32_t csvstep, const struct PANOWIN panpar,
                float * res, struct POSITIONL * pos)
{
   uint32_t yi;
   uint32_t xi;
   double z0m;
   double y0m;
   double x0m;
   double z0;
   double y00;
   double x0;
   double refrac;
   double clat;
   double slat;
   double wy;
   double wx;
   double ddm;
   double dm;
   double dd;
   double suneyeaz;
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
   float aliassum;
   float pixsizewater;
   float sunlum;
   float xsunm;
   float wooddens;
   float dropsun;
   float sunarea;
   float xsun;
   float watershape;
   float space1;
   float oldh;
   float waterh;
   float wavelengthy;
   float wavelengthx;
   float wxx;
   float foga;
   float slant;
   float space;
   float llum;
   float lum;
   float lastlum;
   float dlum;
   float pixsize;
   float snowdense;
   struct POSITIONL waterpos;
   char heavenc;
   char mirrord;
   char firsty;
   char heaven;
   uint8_t attu;
   uint8_t attw;
   uint8_t attr;
   struct ROTVEC sunrotvec;
   struct NORMVEC nvec;
   struct COL citylamp;
   struct COL aliascol;
   struct COL col;
   struct COL suncolair;
   struct COL suncol;
   struct COL snowc;
   struct COL watergroundcol;
   struct COL boc;
   struct COL hc;
   /*    glance:GLANCESET; */
   struct libsrtm_METAINFO meta;
   libsrtm_pMETAINFO pmeta;
   char odd;
   char istree;
   TREEHINTS treehints;
   struct DDSTACK ddstack;
   char tmp;
   memset((char *)treehints,(char)0,sizeof(TREEHINTS));
   pmeta = &meta;
   if (videolat<=90.0f) {
      coslat = cos((double)(videolat*1.7453292519444E-2f));
   }
   else coslat = cos(panpar.eye.lat);
   treerasterlat = treeraster;
   treerasterlong = treerasterlat*coslat;
   maxdist = (double)(distancel(panpar.eye, panpar.horizon)*1000.0f);
   if (maxdist<50.0) return;
   /* horizon too near */
   wavelengthx = wavelength*5.E+6f;
   wavelengthy = (float)((double)wavelengthx*coslat);
   azieletorotvec(4.71238898025f-panpar.sunazi, panpar.sunele, &sunrotvec);
   /*azieletovec(-panpar.sunazi, panpar.sunele, glance.sunvec); */
   wgs84sl(panpar.eye.lat, panpar.eye.long0, (double)panpar.eyealt,
                &x0, &y00, &z0);
   azi = (double)(azimuthl(panpar.eye,
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
   slat = sin(-panpar.eye.lat);
   clat = cos(-panpar.eye.lat);
   slong = sin(-panpar.eye.long0);
   clong = cos(-panpar.eye.long0);
   xi = 0UL;
   refrac = (double)(panpar.refract*7.85E-8f);
   sunarea = sunvisability((double)panpar.sunazi,
                (double)panpar.sunele, slat, clat, slong, clong, x0,
                y00, z0, (float)maxdist, (float)refrac);
   if (verb) {
      osic_WrFixed((float)(X2C_DIVL(azi*180.0,3.1415926535)), 1L, 1UL);
      osi_WrStr(", ", 3ul);
      osic_WrFixed(panpar.eyealt, 1L, 1UL);
      osi_WrStrLn(" cam azimuth, alt", 18ul);
      osic_WrFixed(sunarea*100.0f, 1L, 1UL);
      osi_WrStrLn("% of sun visable", 17ul);
   }
   teye.lat = panpar.eye.lat;
   teye.long0 = panpar.eye.long0;
   showstamdist = (X2C_DIVR((float)(panpar.image0->Len1-1),
                panpar.angle0))*60.0f;
   suncolour(panpar.sunele, &suncol, &sunlum);
   do {
      if (dropshadows) Shadowmask((int32_t)xi);
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
      ddstack.startdist = 0.0f;
      /*WrStrLn(" colum="); WrInt(xi,1); */
      do {
         wx = (double)wxx;
         wy = ele0+eled*(double)yi;
         if (mirrord) {
            /* double beam deviation in sight direction */
            wy = -wy+(double)waveamp*sin(pos->lat*(double)
                wavelengthx)+(double)(waveamp*0.5f)
                *sin(pos->lat*(double)wavelengthx*3.7)+(double)
                (waveamp*0.6f)*sin(pos->long0*(double)wavelengthy);
            wx = wx+0.5*((double)waveamp*sin(pos->long0*(double)
                wavelengthx)+(double)(0.4f*waveamp)
                *sin(pos->long0*(double)wavelengthx*2.9)+(double)
                (waveamp*0.6f)*sin(pos->lat*(double)wavelengthy));
         }
         if (panpar.flatscreen0) wy = atan(wy);
         if (wy>=(-1.5690509974981) && wy<1.5690509974981) {
            /*WrFixed(wy, 3, 10); WrStrLn("=wy"); */
            if (!heaven) {
               /*WrFixed(wx/RAD, 3, 10);WrFixed(wy/RAD, 3, 10);
                WrStrLn("=wxwy"); */
               angtoxyz(wx, wy, slat, clat, slong, clong, &xn, &yn, &zn);
               if (mirrord) {
                  dm = 0.0;
                  /*          raytrace(x0m,y0m,z0m, camvec.x,camvec.y,
                camvec.z, maxdist-dd, -eled, refrac, dm, space1, pos); */
                  raytrace(x0m, y0m, z0m, xn, yn, zn, (float)(maxdist-dd),
                 (float) -eled, (float)refrac, dd, &dm, &space1, pos,
                treehints, &ddstack, &istree, &odd, 0);
                  /*WrFixed(dm, 1,8); wrxyz(x0m,y0m,z0m);
                wrxyz(x0m+xn,y0m+yn,z0m+zn);WrFixed(pos.lat/RAD, 6,12);
                WrFixed(pos.long/RAD, 6,12);WrStrLn(""); */
                  ddm = dd+dm;
                  if (dm>maxdist-dd) heaven = 1;
               }
               else {
                  /*          raytrace(x0,y0,z0, camvec.x,camvec.y,camvec.z,
                maxdist, eled, refrac, dd, space, pos); */
                  raytrace(x0, y00, z0, xn, yn, zn, (float)maxdist,
                (float)eled, (float)refrac, 0.0, &dd, &space, pos,
                treehints, &ddstack, &istree, &odd, dropshadows);
                  ddm = dd;
                  if (dd>maxdist) heaven = 1;
                  if (csvstep && xi%csvstep==0UL) {
                     xycsv(xi, yi, *pos, heaven, firsty);
                  }
               }
               if (ddm<1.0) return;
            }
            /* under ground */
            /*          IF mirrord THEN ddm:=maxdist*4.0*(0.25+FLOAT(yi)
                /FLOAT(HIGH(panpar.image^[0])));  */
            /*          ELSE ddm:=maxdist*4.0*(1.25-FLOAT(yi)
                /FLOAT(HIGH(panpar.image^[0]))); END; */
            lum = (float)(ddm*0.2);
            suneyeaz = (double)radmod((float)(wx-(double)
                panpar.sunazi));
            if (!mirrord) {
               llum = (lum-lastlum)*20.0f;
               if ((double)llum>0.05*maxdist) {
                  llum = (float)(0.05*maxdist);
               }
               if (llum>dlum) dlum = llum;
               lastlum = lum; /* lum is distance */
            }
            heavenc = heaven;
            if (!heaven) {
               normvec(*pos, pmeta, &slant, &oldh, &nvec, &attr, &attw,
                &attu);
               rotveclight(&xsun, nvec, sunrotvec);
                /* cos sunangle on ground */
               if (istree) {
                  /*            normvectree(treehints[odd]); */
                  /*            rotveclight(xsuntree, treehints[odd].nvec,
                sunrotvec);(* cos sunangle on tree *) */
                  oldh = treehints[odd].rooth+treehints[odd].hith;
               }
               /*          ELSE xsuntree:=0.0 END; */
               if (dropshadows && !alticolour) {
                  /*            IF (xsun>0.0) OR (xsuntree>0.0)
                THEN                        (* not selfshadow *) */
                  if (xsun>0.0f) {
                     /* not selfshadow */
                     dropsun = seesun(*pos, (double)(oldh+0.2f),
                panpar.sunazi, panpar.sunele+4.7996554428472E-3f,
                (float)maxdist, (float)refrac);
                  }
                  else dropsun = 0.0f;
               }
               else dropsun = 1.0f;
               if (panpar.sunele<0.0f) {
                  dropsun = dropsun+panpar.sunele*15.0f; /* night light */
               }
               shadowcolour(dropsun, suncol, &suncolair);
               /*        shadowcolour(1.0+panpar.sunele*15.0, suncol,
                suncolair); */
               if (alticolour) {
                  /* only colorized altitude */
                  hc.r = (xsun+1.0f)*500.0f;
                  hc.g = oldh;
                  hc.b = 0.0f;
                  if (attr>50U) hc.b = 1.0f;
                  else if (attw>50U) hc.b = 2.0f;
                  else if (attu>50U) hc.b = 3.0f;
                  if (oldh<alticolmin) alticolmin = oldh;
                  if (oldh>alticolmax) alticolmax = oldh;
               }
               else {
                  pixsize = (float)((X2C_DIVL((double)(float)
                (panpar.image0->Len1-1),
                ddm*(double)panpar.angle0))*5.729577951472E+1);
                /* pixel/m */
                  wooddens = wooddensity(attw, oldh, slant);
                  if (istree) {
                     treecol(*pos, treehints[odd], sunrotvec, pixsize,
                sunlum, wooddens, one(1.0f-X2C_DIVR(snowalt-oldh,
                slowaltslant)), (float)suneyeaz, xsun, attw, suncolair,
                &hc);
                  }
                  else {
                     meadowcol(*pos, slant, oldh, pixsize, xsun, &xsunm,
                &col); /* meadow with snow destruction */
                     rockcol(*pos, slant, oldh, pixsize, xsun, attw, istree,
                &xsunm, &col); /*glance,*/ /* green/rock */
                     citycol(attu, *pos, pixsize, &col, &citylamp);
                /* city */
                     botaniccol(&boc, suncolair, xsunm, 1.0f, sunlum);
                /* apply sun */
                     snowdense = snowdensity(*pos, slant, oldh, pixsize,
                treehints[odd], attw);
                     /*              snowcol(snowc, suncolair,  xsun-0.05,
                sunlum); */
                     snowcol(&snowc, suncolair, snownoise(*pos, xsun,
                pixsize), sunlum);
                     hc.r = col.r*boc.r+citylamp.r;
                     hc.g = col.g*boc.g+citylamp.g;
                     hc.b = col.b*boc.b+citylamp.b;
                     colblend(snowdense, hc, snowc, &hc); /* add snow */
                     darkundertree(attw, wooddens, snowdense, treehints[odd],
                 &hc); /* dark under tree */
                  }
                  /* lake */
                  if ((((!mirrord && !istree) && flatwater)
                && (attr>0U || waterslant<0.0f)) && slant>(float)
                fabs(waterslant)) {
                     pixsizewater = (float)((double)
                pixsize*fabs(sin(wy)));
                     if (pixsizewater<0.03333f) {
                        pixsizewater = 0.03333f;
                /* size of srtm is limit for aliasing */
                     }
                     aliassum = one(0.5f+((float)attr-50.0f)
                *0.3f*pixsizewater);
                  }
                  /*IF aliassum>0.0 THEN aliascol:=hc END; */
                  /*   
                              IF NOT mirrord & NOT istree & NOT treepass & flatwater
                              & ((attr>0) & (aliassum>0.0)
                              OR (waterslant<0.0)) & (slant>ABS(waterslant))
                THEN
                  */
                  if (!mirrord && aliassum>0.0f) {
                     waterpos = *pos;
                     waterh = oldh+0.2f;
                     wgs84sl(pos->lat, pos->long0, (double)waterh,
                &x0m, &y0m, &z0m);
                     dm = 0.0;
                     mirrord = 1;
                     if (waterslant<0.0f) watershape = 1.0f;
                     else watershape = (float)attr*0.01f;
                     watergroundcol = hc;
                     underwatercol(*pos, (float)wy, pixsize, sunlum, attw,
                 &watergroundcol);
                  }
                  else if (mirrord) {
                     wateralias((float)wy, snowalt-oldh, watershape, 1.0f,
                 panpar.sunele, sunlum, watergroundcol, &hc);
                     mirrord = 0;
                  }
                  sundisk((float)suneyeaz, 0.0f, panpar.sunazi,
                panpar.sunele, one((float)(X2C_DIVL(ddm,
                maxdist)-0.02))+0.01f, sunarea, (float)maxdist, 0U,
                suncol, waterpos, waterh, (float)refrac, dropsun, &hc);
                  /*egde fog */
                  foga = (float)((double)dlum*(1.0-X2C_DIVL(ddm,
                maxdist))*(double)dusthighpass*cos(wy));
                  hc.r = hc.r+foga*suncol.r;
                  hc.g = hc.g+foga*suncol.g;
                  hc.b = hc.b+foga*suncol.b;
               }
            }
            else {
               /* heaven */
               if (mirrord) {
                  wateralias((float)wy, snowalt-oldh, watershape, 1.0f,
                panpar.sunele, sunlum, watergroundcol, &hc);
                  dlum = 0.0f;
                  heaven = 0;
               }
               sundisk((float)suneyeaz, (float)wy, panpar.sunazi,
                panpar.sunele, 0.0f, sunarea, (float)maxdist,
                (uint8_t)((uint32_t)mirrord+1UL), suncol, waterpos,
                waterh, (float)refrac, 1.0f, &hc);
               mirrord = 0;
            }
            if (mirrord && aliassum>0.0f) aliascol = hc;
            if (!mirrord) {
               if (!alticolour && aliassum>0.0f) {
                  colblend(aliassum, aliascol, hc, &hc);
                  aliassum = 0.0f;
               }
               /*hc.r:=hc.r*aliassum; */
               /*hc.g:=hc.g*aliassum; */
               /*hc.b:=hc.b*aliassum; */
               storepix(panpar.image0, xi, yi, hc, space, heavenc);
               dlum = (float)((double)dlum*((1.0-(X2C_DIVL(ddm,
                maxdist))*0.2)-X2C_DIVL(120.0,
                (double)(panpar.image0->Len0-1))));
                /* fade out distance highpass peak */
            }
         }
         if (!mirrord) {
            if (ddstack.startdist!=0.0f) {
               /* jump back to transparency entry */
               dd = (double)ddstack.startdist;
               ddstack.startdist = 0.0f;
               heaven = 0;
            }
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
   cr = (cr*panpar.maxlum)/256UL;
   cg = (cg*panpar.maxlum)/256UL;
   cb = (cb*panpar.maxlum)/256UL;
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
                sym[iyr][ix].r8*cr)/256UL+ar*(uint32_t)anonym->r)/255UL);
                  anonym->g = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].g8*cg)/256UL+ar*(uint32_t)anonym->g)/255UL);
                  anonym->b = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].b8*cb)/256UL+ar*(uint32_t)anonym->b)/255UL);
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

#define panorama_MH 4096


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
   cr = (int32_t)(((uint32_t)cr*panpar.maxlum)/1024UL);
   cg = (int32_t)(((uint32_t)cg*panpar.maxlum)/1024UL);
   cb = (int32_t)(((uint32_t)cb*panpar.maxlum)/1024UL);
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
               if (a==-1L) c = 1024L;
               else if (a==0L) c = 3072L;
               else if (a>0L) c = 4096L;
               if (c>0L) {
                  s = 0L;
                  if (c<4096L) {
                     if (ix<size) {
                        if (iy!=size*2UL-1UL) s = 800L;
                     }
                     else s = -1600L;
                  }
                  else if (iy==size*2UL-1UL) {
                     c = 2000L;
                     s = -1200L;
                  }
                  c = (c*calpha)/256L;
                  { /* with */
                     struct imagetext_PIX * anonym = &panpar.image0->Adr[((x+ix)
                -size)*panpar.image0->Len0+(y+iy)];
                     n = ((4096L-c)*(int32_t)anonym->r)/4096L+(c*cr)
                /256L+s;
                     if (n<0L) anonym->r = 0U;
                     else anonym->r = (uint16_t)n;
                     n = ((4096L-c)*(int32_t)anonym->g)/4096L+(c*cg)
                /256L+s;
                     if (n<0L) anonym->g = 0U;
                     else anonym->g = (uint16_t)n;
                     n = ((4096L-c)*(int32_t)anonym->b)/4096L+(c*cb)
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


static void poipixel(struct POSITIONL ppos, float palt,
                const struct PANOWIN panpar, int32_t * px, int32_t * py,
                pPOI pp, TREEHINTS treehints, const char text[],
                uint32_t text_len)
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
   char odd;
   char istree;
   uint32_t poiy;
   int32_t xt;
   struct DDSTACK ddstack;
   struct POI * anonym;
   *px = -1L;
   *py = -1L;
   if (!posvalidl(ppos)) return;
   refrac = panpar.refract*7.85E-8f;
   nne = libsrtm_getsrtmlong(panpar.eye.lat, panpar.eye.long0, 0UL, 1,
                &resoltx, &attr, 0);
   if (defaultocean && nne>20000.0f) nne = 0.0f;
   nnp = libsrtm_getsrtmlong(ppos.lat, ppos.long0, 0UL, 1, &resoltx, &attr,
                0);
   if (defaultocean && nnp>20000.0f) nnp = 0.0f;
   if (pp->altIsOG) {
      if (palt<minpoialt) palt = minpoialt;
      nnp = nnp+palt;
   }
   else {
      nnp = nnp+minpoialt;
      if (nnp<palt) nnp = palt;
   }
   aeye = panpar.eyealt;
   wgs84sl(panpar.eye.lat, panpar.eye.long0, (double)aeye, &x0, &y00,
                &z0);
   wgs84sl(ppos.lat, ppos.long0, (double)nnp, &x1, &y1, &z1);
   d2 = (float)sqrt((double)(sqr((float)(x1-x0))+sqr((float)
                (y1-y00))+sqr((float)(z1-z0))));
   wgs84sl(ppos.lat, ppos.long0, (double)(nnp+refrac*d2*d2), &x1, &y1,
                &z1);
   d2 = (float)sqrt((double)(sqr((float)(x1-x0))+sqr((float)
                (y1-y00))+sqr((float)(z1-z0))));
   xn = X2C_DIVL(x1-x0,(double)d2);
   yn = X2C_DIVL(y1-y00,(double)d2);
   zn = X2C_DIVL(z1-z0,(double)d2);
   rd = 0.0;
   treehints[0].treepass = 1;
   treehints[0].hith = treesize;
   ddstack.startdist = 0.0f;
   raytrace(x0, y00, z0, xn, yn, zn, d2, 0.0f, refrac, 0.0, &rd, &subpix,
                &rpos, treehints, &ddstack, &istree, &odd, 0);
   /*WrFixed(d2, 1, 8); WrFixed(rd, 1, 8); WrStrLn("=d,rd"); */
   if (rd+30.0>=(double)d2) {
      /* poi visable */
      xn = -xn;
      /*WrFixed(xn,2,6);WrFixed(yn,2,6);WrFixed(zn,2,6); WrStrLn(" xyz1"); */
      rotvector(&yn, &xn, cos(panpar.eye.long0), sin(panpar.eye.long0));
      rotvector(&zn, &xn, cos(panpar.eye.lat), sin(panpar.eye.lat));
      /*WrFixed(xn,2,6);WrFixed(yn,2,6);WrFixed(zn,2,6); WrStrLn(" xyz2"); */
      azi = azimuthl(panpar.eye, panpar.horizon)*1.7453292519444E-2f;
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
         poiy = anonym->ys;
         if (anonym->xs==0UL) {
            ok0 = drawpoi(xi, yi, panpar, poisize, (int32_t)anonym->r,
                (int32_t)anonym->g, (int32_t)anonym->b,
                (int32_t)anonym->alpha);
            poiy = poisize*2UL;
         }
         else if (treehints[0].hith<treesize) {
            ok0 = drawpoiicon(xi, yi, panpar, anonym->woodsymbol, anonym->xs,
                 anonym->ys, anonym->r, anonym->g, anonym->b, anonym->alpha);
         }
         else {
            ok0 = drawpoiicon(xi, yi, panpar, anonym->symbol, anonym->xs,
                anonym->ys, anonym->r, anonym->g, anonym->b, anonym->alpha);
         }
         /*- vertical text to icon */
         if (poilabel) {
            xt = (int32_t)X2C_TRUNCI(xi,X2C_min_longint,
                X2C_max_longint)-(int32_t)(imagetext_fontsizey(fonttyp)
                /2UL);
            if (xt>=0L) {
               imagetext_writestr(panpar.image0, (uint32_t)xt,
                truncc(yi)+poiy+2UL, fonttyp, 1UL, 5UL,
                (int32_t)(panpar.maxlum/4UL),
                (int32_t)(panpar.maxlum/4UL),
                (int32_t)(panpar.maxlum/4UL), text, text_len);
            }
         }
      }
      /*- vertical text to icon */
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
   struct POSITIONL pos;
   uint8_t attr;
   int32_t py;
   int32_t px;
   TREEHINTS treehints;
   pos.lat = (double)(posdeg.lat*1.7453292519444E-2f);
   pos.long0 = (double)(posdeg.long0*1.7453292519444E-2f);
   /*WrFixed(posdeg.lat,2,9); WrStr(" "); */
   /*WrFixed(posdeg.long,2,9); WrStr(" "); */
   d = distancel(panpar.eye, pos);
   if (d>panpar.dist || d<0.05f) return;
   az = panpar.ang-azimuthl(panpar.eye, pos)*1.7453292519444E-2f;
   if (az<(-3.1415926535f)) az = az+6.283185307f;
   else if (az>3.1415926535f) az = az-6.283185307f;
   if ((float)fabs(az)>panpar.hseg) return;
   poipixel(pos, alti, panpar, &px, &py, pp, treehints, text, text_len);
   if (verb) {
      osic_WrFixed(distancel(panpar.eye, pos), 3L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(azimuthl(panpar.eye, pos), 2L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(X2C_DIVR(az,1.7453292519444E-2f), 2L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(alti, 0L, 9UL);
      osi_WrStr(" ", 2ul);
      osi_WrStr(text, text_len);
      if (px<0L) {
         osi_WrStr(" #", 3ul);
      }
      else if (treehints[0].hith<treesize) {
         osi_WrStr(" (", 3ul);
         osic_WrFixed(treehints[0].hith, 1L, 1UL);
         osi_WrStr("m/", 3ul);
         osic_WrFixed(treehints[0].passdist*0.001f, 3L, 1UL);
         osi_WrStr("km)", 4ul);
      }
      osi_WrStrLn("", 1ul);
   }
   if (px>=0L) {
      alt = libsrtm_getsrtmlong(pos.lat, pos.long0, 0UL, 1, &resol, &attr,
                0);
      if (defaultocean && alt>20000.0f) alt = 0.0f;
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
      if (pp->iconwoodfn[0U]) {
         readsymbol(pp->woodsymbol, &pp->xs, &pp->ys, pp->iconwoodfn,
                1024ul);
      }
      rdmountain(panpar, pp->poifn, 1024ul, pp);
      pp = pp->next;
   }
} /* end rdmountains() */

#define panorama_DEGSYM "\177"


static void WrLatLong(const struct PANOWIN panpar)
{
   uint32_t lum;
   uint32_t y;
   char h[201];
   char s[201];
   y = imagetext_fontsizey(fonttyp);
   aprsstr_FixToStr((float)(X2C_DIVL(panpar.eye.lat,1.7453292519444E-2)),
                6UL, s, 201ul);
   aprsstr_Append(s, 201ul, "/", 2ul);
   aprsstr_FixToStr((float)(X2C_DIVL(panpar.eye.long0,
                1.7453292519444E-2)), 6UL, h, 201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   aprsstr_Append(s, 201ul, " A=", 4ul);
   aprsstr_FixToStr(panpar.eyealt, 2UL, h, 201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   aprsstr_Append(s, 201ul, " d=", 4ul);
   aprsstr_FixToStr(panpar.dist, 2UL, h, 201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   aprsstr_Append(s, 201ul, " e=", 4ul);
   aprsstr_FixToStr(panpar.elevation, 2UL, h, 201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   aprsstr_Append(s, 201ul, "\177 sun=", 7ul);
   aprsstr_FixToStr(X2C_DIVR(panpar.sunazi,1.7453292519444E-2f), 2UL, h,
                201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   aprsstr_Append(s, 201ul, "/", 2ul);
   aprsstr_FixToStr(X2C_DIVR(panpar.sunele,1.7453292519444E-2f), 2UL, h,
                201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   aprsstr_Append(s, 201ul, "\177", 2ul);
   if (alticolour) {
      aprsstr_Append(s, 201ul, " altcol=", 9ul);
      aprsstr_FixToStr(alticolmin, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, ":", 2ul);
      aprsstr_FixToStr(alticolmax, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      if (altcolstep>0UL) {
         aprsstr_Append(s, 201ul, "/", 2ul);
         aprsstr_IntToStr((int32_t)altcolstep, 0UL, h, 201ul);
         aprsstr_Append(s, 201ul, h, 201ul);
      }
      aprsstr_Append(s, 201ul, "m", 2ul);
   }
   else {
      aprsstr_Append(s, 201ul, " snow=", 7ul);
      aprsstr_FixToStr(snowalt, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "/", 2ul);
      aprsstr_FixToStr(slowaltslant, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "m W=", 5ul);
      aprsstr_FixToStr(waveamp*100.0f, 2UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "/", 2ul);
      aprsstr_FixToStr(wavelength, 2UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "/", 2ul);
      aprsstr_FixToStr(waterslant, 3UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, " T=", 4ul);
      aprsstr_FixToStr(treesize, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "/", 2ul);
      aprsstr_FixToStr(botanicslant, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "/", 2ul);
      aprsstr_FixToStr(zerotreealt, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "/", 2ul);
      aprsstr_FixToStr(fulltreealt, 0UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "m", 2ul);
   }
   if (dateh[0U]) {
      aprsstr_Append(s, 201ul, " utc:", 6ul);
      aprsstr_Append(s, 201ul, dateh, 1024ul);
      aprsstr_Append(s, 201ul, " ", 2ul);
      aprsstr_Append(s, 201ul, timeh, 1024ul);
   }
   lum = panpar.maxlum/6UL;
   imagetext_writestr(panpar.image0, 5UL, y, fonttyp, 0UL, 10UL,
                (int32_t)lum, (int32_t)lum, (int32_t)lum, s, 201ul);
} /* end WrLatLong() */


static void drawscale(const struct PANOWIN panpar, uint32_t px,
                uint32_t deg)
{
   uint32_t lum;
   uint32_t y;
   char s[101];
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   lum = panpar.maxlum/6UL;
   if (px>2UL && px+2UL<=panpar.image0->Len1-1) {
      tmp = scalesize/10UL;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         { /* with */
            struct imagetext_PIX * anonym = &panpar.image0->Adr[(px)
                *panpar.image0->Len0+y];
            anonym->r = (uint16_t)((uint32_t)anonym->r+lum);
            anonym->g = (uint16_t)((uint32_t)anonym->g+lum);
         }
         if (y==tmp) break;
      } /* end for */
      if (px<=panpar.image0->Len1-1) {
         if (deg==0UL) strncpy(s,"N",101u);
         else if (deg==90UL) strncpy(s,"E",101u);
         else if (deg==180UL) strncpy(s,"S",101u);
         else if (deg==270UL) strncpy(s,"W",101u);
         else aprsstr_IntToStr((int32_t)deg, 0UL, s, 101ul);
         imagetext_writestr(panpar.image0, px+2UL, 0UL, fonttyp, 0UL, 5UL,
                (int32_t)lum, (int32_t)lum, (int32_t)lum, s, 101ul);
      }
   }
} /* end drawscale() */


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
   azi = azimuthl(panpar.eye, panpar.horizon);
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

#define panorama_LUM 6

#define panorama_SHADOWLUM 0

#define panorama_TAGS 5

#define panorama_MAXLUT 1024


static float intpol(uint32_t ca, uint32_t cb, float f)
{
   return 2.3529411764706E-2f*((float)ca*(1.0f-f)+(float)cb*f);
} /* end intpol() */

struct _2;


struct _2 {
   float rr;
   float gg;
   float bb;
};

struct _3;


struct _3 {
   char on;
   float rr;
   float gg;
   float bb;
};


static void colourize(imagetext_pIMAGE img, float amin, float amax,
                const struct ALTCOLTAB atab)
/* make altitude to colour */
{
   float K;
   float f;
   float a;
   float amul;
   uint32_t c;
   uint32_t y;
   uint32_t x;
   int32_t hs;
   int32_t h;
   struct _2 lut[1024];
   struct _3 tlut[5];
   struct _2 * anonym;
   struct imagetext_PIX * anonym0;
   /* tagged & tag color set */
   struct _3 * anonym1;
   struct _2 * anonym2;
   uint32_t tmp;
   uint32_t tmp0;
   if (atab.len==0UL) {
      /* no file make default lut */
      K = 5.859375E-3f;
      for (c = 0UL; c<=1023UL; c++) {
         { /* with */
            struct _2 * anonym = &lut[c];
            if (c<400UL) anonym->rr = (float)(((400UL-c)*900UL)/400UL)*K;
            else anonym->rr = (float)(((c-400UL)*900UL)/624UL)*K;
            if (c<200UL) anonym->gg = (float)((c*900UL)/200UL)*K;
            else if (c<640UL) {
               anonym->gg = (float)(((640UL-c)*900UL)/440UL)*K;
            }
            else anonym->gg = (float)(((c-640UL)*900UL)/372UL)*K;
            if (c<320UL) anonym->bb = 0.0f;
            else if (c<640UL) {
               anonym->bb = (float)(((c-320UL)*900UL)/320UL)*K;
            }
            else anonym->bb = 900.0f*K;
         }
      } /* end for */
   }
   else {
      /*
              a:=LUM*0.5/(rr*0.3 + gg*0.5 + bb*0.2);
              rr:=rr*a;
              gg:=gg*a;
              bb:=bb*a; 
      */
      /* interpolate table */
      memset((char *)lut,(char)0,sizeof(struct _2 [1024]));
      memset((char *)tlut,(char)0,sizeof(struct _3 [5]));
      a = X2C_DIVR((float)(atab.len-1UL),1024.0f);
      for (x = 0UL; x<=1023UL; x++) {
         f = a*(float)x;
         y = (uint32_t)X2C_TRUNCC(f,0UL,X2C_max_longcard);
         f = f-(float)y;
         if (y+2UL>atab.len) y = atab.len-2UL;
         lut[x].rr = intpol(atab.c[y].r, atab.c[y+1UL].r, f);
         /*WrFixed(lut[x].rr, 0, 8); */
         lut[x].gg = intpol(atab.c[y].g, atab.c[y+1UL].g, f);
         lut[x].bb = intpol(atab.c[y].b, atab.c[y+1UL].b, f);
      } /* end for */
      for (x = 0UL; x<=4UL; x++) {
         c = 0UL;
         c += atab.tag[x].r;
         tlut[x].rr = 2.3529411764706E-2f*(float)atab.tag[x].r;
         c += atab.tag[x].g;
         tlut[x].gg = 2.3529411764706E-2f*(float)atab.tag[x].g;
         c += atab.tag[x].b;
         tlut[x].bb = 2.3529411764706E-2f*(float)atab.tag[x].b;
         tlut[x].on = c>0UL; /* tag is switched on */
      } /* end for */
   }
   amul = amax-amin;
   if (amul<1.0f) amul = 1.0f;
   amul = X2C_DIVR(1.0f,amul);
   hs = (int32_t)(altcolstep/2UL);
   tmp = img->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = img->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct imagetext_PIX * anonym0 = &img->Adr[(x)*img->Len0+y];
            if (anonym0->b<=5U) {
               /* not heaven */
               a = (float)(anonym0->r+0U); /* luminance */
               if (anonym0->b>0U && tlut[anonym0->b-1U].on) {
                  { /* with */
                     struct _3 * anonym1 = &tlut[anonym0->b-1U];
                     anonym0->r = (uint16_t)(int32_t)
                X2C_TRUNCI(anonym1->rr*a,X2C_min_longint,X2C_max_longint);
                     anonym0->g = (uint16_t)(int32_t)
                X2C_TRUNCI(anonym1->gg*a,X2C_min_longint,X2C_max_longint);
                     anonym0->b = (uint16_t)(int32_t)
                X2C_TRUNCI(anonym1->bb*a,X2C_min_longint,X2C_max_longint);
                  }
               }
               else {
                  h = (int32_t)X2C_TRUNCI(((float)anonym0->g-amin)
                *amul*1023.0f,X2C_min_longint,X2C_max_longint);
                  if (altcolstep>0UL && h>=0L) {
                     h = (int32_t)(((uint32_t)(h+hs)/altcolstep)
                *altcolstep);
                  }
                  if (h<0L) h = 0L;
                  else if (h>1023L) h = 1023L;
                  { /* with */
                     struct _2 * anonym2 = &lut[h];
                     anonym0->r = (uint16_t)(int32_t)
                X2C_TRUNCI(anonym2->rr*a,X2C_min_longint,X2C_max_longint);
                     anonym0->g = (uint16_t)(int32_t)
                X2C_TRUNCI(anonym2->gg*a,X2C_min_longint,X2C_max_longint);
                     anonym0->b = (uint16_t)(int32_t)
                X2C_TRUNCI(anonym2->bb*a,X2C_min_longint,X2C_max_longint);
                  }
               }
            }
            else {
               /* heaven */
               anonym0->r = 2000U;
               anonym0->g = 3000U;
               anonym0->b = 4500U;
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end colourize() */

#define panorama_W 9

#define panorama_H 153

#define panorama_V 20


static void testimage(imagetext_pIMAGE img, uint32_t * min1,
                uint32_t * max0)
{
   uint32_t y;
   uint32_t x;
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   if (img->Len0-1<=173UL || img->Len1-1<=9UL) return;
   for (y = 0UL; y<=152UL; y++) {
      tmp = img->Len1-1;
      x = (img->Len1-1)-8UL;
      if (x<=tmp) for (;; x++) {
         { /* with */
            struct imagetext_PIX * anonym = &img->Adr[(x)*img->Len0+(y+20UL)
                ];
            anonym->r = (uint16_t)(*min1+((*max0-*min1)*(y/9UL))/16UL);
            anonym->g = anonym->r;
            anonym->b = anonym->r;
         }
         if (x==tmp) break;
      } /* end for */
   } /* end for */
} /* end testimage() */


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
   panowin.ang = azimuthl(panowin.eye, panowin.horizon)*1.7453292519444E-2f;
   panowin.hseg = angle*1.7453292519444E-2f*0.5f;
   panowin.dist = distancel(panowin.eye, panowin.horizon);
   panowin.maxlum = 2000UL;
   if (imagefn[0U]) Panorama1(panowin);
   if (alticolour) {
      if (abscolmin!=0.0f) alticolmin = abscolmin;
      if (abscolmax!=0.0f) alticolmax = abscolmax;
      colourize(panowin.image0, alticolmin, alticolmax, altcoltab);
   }
   histogram(panowin.image0, alticolour, &panowin.minlum, &panowin.maxlum);
   makegammatab(panowin.minlum, panowin.maxlum);
   testimage(panowin.image0, &panowin.minlum, &panowin.maxlum);
   if (fonttyp>0UL) scale(panowin);
   if (poifiles) rdmountains(panowin);
} /* end drawimage() */


static void camalt(struct POSITIONL posa0, struct POSITIONL posb0,
                char fromnn, float * alta0)
{
   float resoltx;
   float nn;
   uint8_t attr;
   if (!posvalidl(posa0)) Error("neer camera position", 21ul);
   nn = libsrtm_getsrtmlong(posa0.lat, posa0.long0, 0UL, 1, &resoltx, &attr,
                0);
   if (defaultocean && nn>20000.0f) nn = 0.0f;
   if (nn>=20000.0f) {
      libsrtm_closesrtmfile();
      Error("no altitude data at camera position", 36ul);
   }
   if (!fromnn) *alta0 = *alta0+nn;
   if (*alta0<nn) Error("altitude of camera below ground", 32ul);
   nn = libsrtm_getsrtmlong(posb0.lat, posb0.long0, 0UL, 1, &resoltx, &attr,
                0);
   if (defaultocean && nn>20000.0f) nn = 0.0f;
   if (nn>=20000.0f) {
      libsrtm_closesrtmfile();
      Error("no altitude data at view point", 31ul);
   }
} /* end camalt() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[2];
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   imagetext_BEGIN();
   aprsstr_BEGIN();
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
   igamma = 1.0f;
   libsrtm_srtmmaxmem = 100000000UL;
   heavencol.r = 150.0f;
   heavencol.g = 300.0f;
   heavencol.b = 3500.0f;
   elev = 0.0f;
   angle = 45.0f;
   yzoom = 1.0f;
   flatscreen = 0;
   poisize = 5UL;
   minpoialt = 1.0f;
   fonttyp = 0UL;
   verb = 0;
   poilabel = 0;
   /*  bicubic:=FALSE; */
   rastercsv = 0UL;
   sunaz = 0.0f;
   sunel = (-100.0f);
   snowalt = 3000.0f;
   slowaltslant = 500.0f;
   flatwater = 0;
   waveamp = 0.006f;
   dusthighpass = 0.001f;
   maxmountain = 8900.0;
   wavelength = 1.0f;
   waterslant = 0.0f;
   treesize = 25.0f;
   urbanoff = 1.0f;
   botanicslant = 8.0f;
   ignorblack = 0.1f;
   ignorwhite = 0.5f;
   whitelim = 1000UL;
   blacklim = 100UL;
   desertcol.r = (-1.0f);
   videolat = 1000.0f;
   logfilm = (-1.0f);
   newsnow = 0.0f;
   sandgranularity = 1.0f;
   zerotreealt = 2600.0f;
   fulltreealt = 1200.0f;
   defaultocean = 0;
   alticolour = 0;
   alticolmin = X2C_max_real;
   alticolmax = -X2C_max_real;
   abscolmin = 0.0f;
   abscolmax = 0.0f;
   altcolstep = 0UL;
   colfn[0] = 0;
   freeshadow = 0;
   newshadow = 0;
   dropshadows = 0;
   treerastersize = 8.0f;
   debugx = (-10.0f);
   debugy = 10.0f;
   /*WrFixed(fracint(-2.1, csvfd), 3, 7); WrInt(csvfd, 10); WrStrLn(" fi");
                */
   Parms();
   treeraster = (double)(X2C_DIVR(6.3661977238578E+6f,treerastersize));
   treescramblat = treeraster;
   treescramblong = treeraster;
   if (xsize<20L) Error("xsize too less", 15ul);
   if (ysize<20L) Error("ysize too less", 15ul);
   if (angle>=180.0f && flatscreen) {
      Error("sight > 180 deggrees needs courved screen on", 45ul);
   }
   readcolourfile(&altcoltab, colfn, 1024ul);
   camalt(posa, posb, videolat<90.0f, &alta);
   Gencrctab();
   makenoise();
   /*  IF imagefn[0]=0C THEN Error("need inage filename") END; */
   scalesize = imagetext_fontsizex(fonttyp)*7UL;
   /*WrInt(scalesize, 5); WrStrLn("scale"); */
   X2C_DYNALLOCATE((char **) &image,sizeof(struct imagetext_PIX),
                (tmp[0] = (size_t)xsize,tmp[1] = (size_t)ysize,tmp),2u);
   if (image==0) Error("out of memory", 14ul);
   drawimage();
   libsrtm_closesrtmfile();
   if (csvfd>=0L) osic_Close(csvfd);
   if (imagefn[0U] && wrpng()<0L) osi_Werr("png write error!\012", 18ul);
   if (verb) {
      osi_WrStrLn("", 1ul);
      osi_WrStr("srtmaccesses:", 14ul);
      osic_WrINT32(libsrtm_accesses, 1UL);
      osi_WrStr("/", 2ul);
      osic_WrINT32(libsrtm_accessescached, 1UL);
      osi_WrStrLn("", 1ul);
   }
/*WrStrLn(""); WrFixed(fmod(-5.0, PI), 4, 9); WrFixed(debugx/RAD, 2, 9);
                WrFixed(debugy/RAD, 2, 9); WrStrLn(" xy"); */
/*WrInt(debugxi, 12);WrInt(debugyi, 12); WrStrLn("=pixc"); */
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
