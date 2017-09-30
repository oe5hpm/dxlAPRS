/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef maptool_H_
#include "maptool.h"
#endif
#define maptool_C_
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef xosi_H_
#include "xosi.h"
#endif
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
#ifndef pngwrite_H_
#include "pngwrite.h"
#endif
#ifndef jpgdec_H_
#include "jpgdec.h"
#endif
#ifndef useri_H_
#include "useri.h"
#endif
#ifndef aprstext_H_
#include "aprstext.h"
#endif





int32_t maptool_xsize;
int32_t maptool_ysize;
float maptool_shiftx;
float maptool_shifty;


struct maptool__D0 maptool_mappack;
/* aprs tracks on osm map by oe5dxl */
/*FROM TimeConv IMPORT time; */
/*FROM Storage IMPORT ALLOCATE, DEALLOCATE; */
#define maptool_LF "\012"

#define maptool_MAXCOL 30000

#define maptool_SYMX 16

#define maptool_SYMY 16

#define maptool_SYMN 192

#define maptool_MAPGETFN "gettiles"
/* tiles request filename */

#define maptool_MAXFONTY 18

#define maptool_MINFONTY 7

#define maptool_DEFAULTFONTY 10

#define maptool_FONTFN "font.png"

#define maptool_PNGEXT ".png"

#define maptool_JPGEXT ".jpg"

#define maptool_CHARS 97

#define maptool_NOMAPR 100
/* have no map fill colour */

#define maptool_NOMAPG 120

#define maptool_NOMAPB 100

#define maptool_SRTMXY 3600

#define maptool_STRIPS 3

struct PIX8;


struct PIX8 {
   uint8_t r8;
   uint8_t g8;
   uint8_t b8;
};

typedef struct PIX8 ROWS0[256];

typedef struct PIX8 * pROWS0;

typedef pROWS0 PNGBUF[256];

typedef pROWS0 * pPNGBUF;

typedef char FN[1024];

/* srtm */
typedef short * pSRTMSTRIP;

struct SRTMTILE;

typedef struct SRTMTILE * pSRTMTILE;


struct SRTMTILE {
   uint8_t typ;
   int32_t fd;
   uint8_t used[3][3600];
   pSRTMSTRIP strips[3][3600];
};

typedef pSRTMTILE SRTMLAT[180];

typedef pSRTMTILE * pSRTMLAT;

typedef pSRTMLAT SRTMLONG[360];

struct _0;


struct _0 {
   int32_t fd;
   char havefile; /* have tried to open file */
};
/* srtm */

typedef struct _0 SRTM30FD[9][4];

static struct PIX8 symbols[3072][17];

static PNGBUF pngbuf;

struct _1;


struct _1 {
   uint8_t char0[19][8];
   uint16_t mask[21];
   uint8_t width;
};

static struct _1 font[97];

static char gammatab[1024];

static uint32_t maploadstart;

static char mapnamesbuf[4096]; /* tile name buffer */

static uint8_t mapnamesdone;

/* tile names written to file */
static uint32_t lastmapreq; /* time of last map requested */

static uint32_t maploopcnt;

/* count same tile requests */
static uint32_t mapdelay; /* delay map load start on map moves */

static SRTMLONG srtmcache;

static pSRTMTILE srtmmiss; /* cache no file info with pointer to here */

static SRTM30FD srtm30fd; /* open srtm30 files */

static uint32_t lastpoinum;

/*open, miss, hit:CARDINAL; */

static float sqr(float x)
{
   return x*x;
} /* end sqr() */

#define maptool_GAMMA 4.5454545454545E-1


static void makegammatab(void)
{
   uint32_t c;
   gammatab[0U] = 0;
   for (c = 1UL; c<=1023UL; c++) {
      gammatab[c] = (char)aprsdecode_trunc(osic_exp(osic_ln(X2C_DIVR((float)
                c,1024.0f))*4.5454545454545E-1f)*255.5f);
   } /* end for */
} /* end makegammatab() */


extern void maptool_clr(maptool_pIMAGE img)
{
   uint32_t x;
   uint32_t tmp;
   tmp = img->Len1-1;
   x = 0UL;
   if (x<=tmp) for (;; x++) {
      memset((char *)(img->Adr+(x)*img->Len0),(char)0,img->Size1);
      if (x==tmp) break;
   } /* end for */
} /* end clr() */


extern char maptool_vistime(uint32_t t)
{
   return aprsdecode_systime<t+aprsdecode_lums.maxdim+aprsdecode_lums.firstdim;
                
} /* end vistime() */


extern void maptool_limpos(struct aprspos_POSITION * pos)
{
   if (pos->long0>3.1415926535898f) pos->long0 = 3.1415926535898f;
   else if (pos->long0<(-3.1415926535898f)) pos->long0 = (-3.1415926535898f);
   if (pos->lat>1.484f) pos->lat = 1.484f;
   else if (pos->lat<(-1.484f)) pos->lat = (-1.484f);
} /* end limpos() */


extern void maptool_startmapdelay(void)
{
   mapdelay = (uint32_t)useri_conf2int(useri_fDELAYGETMAP, 0UL, 0L, 60L,
                0L);
   lastmapreq = aprsdecode_realtime;
   maploopcnt = 0UL; /* operator moves map */
} /* end startmapdelay() */


static void findinfo(int32_t nx, int32_t ny)
{
   int32_t d;
   uint32_t i;
   struct aprsdecode__D0 * anonym;
   { /* with */
      struct aprsdecode__D0 * anonym = &aprsdecode_click;
      d = labs(anonym->x-nx)+labs(anonym->y-ny);
      /*IF (d<=5) & (ops<>NIL) THEN WrInt(d, 3);WrInt(entries, 3);
                WrStrLn(ops^.call); END; */
      if (d<=anonym->min0 && anonym->ops) {
         /* ops to ignore cc text */
         if (anonym->min0>0L) anonym->entries = 0UL;
         else if (anonym->entries>9UL) anonym->entries = 9UL;
         anonym->min0 = d;
         i = 0UL;
         while (i<anonym->entries && anonym->table[i].opf!=anonym->ops) ++i;
         anonym->table[i].opf = anonym->ops;
         anonym->table[i].pff = anonym->pf;
         anonym->table[i].pff0 = anonym->pf0;
         anonym->table[i].typf = anonym->typ;
         if (i>=anonym->entries) ++anonym->entries;
      }
   }
} /* end findinfo() */


extern void maptool_area(maptool_pIMAGE img, int32_t x0, int32_t y00,
                int32_t x1, int32_t y1, struct aprsdecode_COLTYP col0,
                char add)
{
   int32_t h;
   int32_t yy;
   int32_t xx;
   struct maptool_PIX * anonym;
   int32_t tmp;
   int32_t tmp0;
   if (x0>x1) {
      h = x0;
      x0 = x1;
      x1 = h;
   }
   if (y00>y1) {
      h = y00;
      y00 = y1;
      y1 = h;
   }
   if (x0<0L) x0 = 0L;
   if (x1<0L) x1 = 0L;
   if (y00<0L) y00 = 0L;
   if (y1<0L) y1 = 0L;
   if (x0>(int32_t)(img->Len1-1)) x0 = (int32_t)(img->Len1-1);
   if (x1>(int32_t)(img->Len1-1)) x1 = (int32_t)(img->Len1-1);
   if (y00>(int32_t)(img->Len0-1)) y00 = (int32_t)(img->Len0-1);
   if (y1>(int32_t)(img->Len0-1)) y1 = (int32_t)(img->Len0-1);
   tmp = y1;
   yy = y00;
   if (yy<=tmp) for (;; yy++) {
      tmp0 = x1;
      xx = x0;
      if (xx<=tmp0) for (;; xx++) {
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(xx)*img->Len0+yy];
            if (add) {
               anonym->r += (uint16_t)col0.r;
               anonym->g += (uint16_t)col0.g;
               anonym->b += (uint16_t)col0.b;
            }
            else {
               anonym->r = (uint16_t)col0.r;
               anonym->g = (uint16_t)col0.g;
               anonym->b = (uint16_t)col0.b;
            }
         }
         if (xx==tmp0) break;
      } /* end for */
      if (yy==tmp) break;
   } /* end for */
} /* end area() */


extern float maptool_realzoom(int32_t zi, float zf)
{
   return ((float)zi+zf)-1.0f;
} /* end realzoom() */


static float expzoom(int32_t z)
{
   return (float)(uint32_t)X2C_LSH(0x1UL,32,z);
} /* end expzoom() */


static float latproj(float l)
{
   return osic_ln(X2C_DIVR(osic_sin(l)+1.0f,osic_cos(l)));
} /* end latproj() */


extern void maptool_xytodeg(float x, float y,
                struct aprspos_POSITION * pos)
{
   int32_t zi;
   float ysf;
   float zoom;
   float pixrad;
   zoom = maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom);
   ysf = (float)maptool_ysize;
   y = ysf-y;
   if (y<0.0f) y = 0.0f;
   else if (y>ysf) y = ysf;
   if ((float)fabs(aprsdecode_mappos.lat)>1.484f) {
      aprsdecode_posinval(pos);
      return;
   }
   zi = (int32_t)aprsdecode_trunc(zoom);
   pixrad = X2C_DIVR(6.2831853071796f,
                ((1.0f+zoom)-(float)zi)*256.0f*expzoom((int32_t)
                aprsdecode_trunc(zoom)));
   pos->long0 = aprsdecode_mappos.long0+pixrad*x;
   pos->lat = 2.0f*osic_arctan(osic_exp(latproj(aprsdecode_mappos.lat)
                -pixrad*y))-1.5707963267949f;
   maptool_limpos(pos);
} /* end xytodeg() */


extern void maptool_shiftmap(int32_t x, int32_t y, int32_t ysize,
                float zoom, struct aprspos_POSITION * pos)
{
   int32_t zi;
   float pixrad;
   zi = (int32_t)aprsdecode_trunc(zoom);
   pixrad = ((1.0f+zoom)-(float)zi)*256.0f*expzoom((int32_t)
                aprsdecode_trunc(zoom));
   pos->long0 = pos->long0-(X2C_DIVR(6.2831853071796f,pixrad))*(float)x;
   pos->lat = 2.0f*osic_arctan(osic_exp(latproj(pos->lat)
                +(X2C_DIVR(6.2831853071796f,
                pixrad))*(float)(ysize-y)))-1.5707963267949f;
   maptool_limpos(pos);
} /* end shiftmap() */


extern void maptool_center(int32_t xsize, int32_t ysize, float zoom,
                struct aprspos_POSITION centpos,
                struct aprspos_POSITION * pos)
{
   int32_t zi;
   float pixrad;
   zi = (int32_t)aprsdecode_trunc(zoom);
   pixrad = ((1.0f+zoom)-(float)zi)*256.0f*expzoom((int32_t)
                aprsdecode_trunc(zoom));
   pos->long0 = centpos.long0-(X2C_DIVR(6.2831853071796f,
                pixrad))*(float)(xsize/2L);
   pos->lat = 2.0f*osic_arctan(osic_exp(latproj(centpos.lat)
                +(X2C_DIVR(6.2831853071796f,
                pixrad))*(float)(ysize/2L)))-1.5707963267949f;
   maptool_limpos(pos);
} /* end center() */

#define maptool_TOL 0.0001


extern void maptool_mercator(float lon, float lat, int32_t zoom,
                int32_t * tilex, int32_t * tiley, float * x,
                float * y)
{
   float z;
   if (lat>1.484f) lat = 1.484f;
   else if (lat<(-1.484f)) lat = (-1.484f);
   if (lon>3.1414926535898f) lon = 3.1414926535898f;
   else if (lon<(-3.1414926535898f)) lon = (-3.1414926535898f);
   z = expzoom(zoom);
   *x = (0.5f+lon*1.591549430919E-1f)*z;
   *y = (0.5f-latproj(lat)*1.591549430919E-1f)*z;
   *tilex = (int32_t)aprsdecode_trunc(*x);
   *tiley = (int32_t)aprsdecode_trunc(*y);
   *x = (*x-(float)*tilex)*256.0f;
   *y = (*y-(float)*tiley)*256.0f;
} /* end mercator() */


extern int32_t maptool_mapxy(struct aprspos_POSITION pos, float * x,
                float * y)
{
   int32_t tiley;
   int32_t tilex;
   float ys;
   float xs;
   if (aprspos_posvalid(pos)) {
      maptool_mercator(pos.long0, pos.lat, aprsdecode_initzoom, &tilex,
                &tiley, x, y);
      tilex -= aprsdecode_inittilex;
      tiley -= aprsdecode_inittiley;
      *x = ((*x+(float)(tilex*256L))-maptool_shiftx)*aprsdecode_finezoom;
      *y = (float)maptool_ysize-(((float)(tiley*256L)+*y)
                -maptool_shifty)*aprsdecode_finezoom;
      /*WrFixed(x, 5, 10); WrFixed(FLOAT(xsize), 5, 10); WrFixed(y, 5, 10);
                WrFixed(FLOAT(ysize), 5, 10); WrLn; */
      /*doreorder error:    IF (x>=0.0) & (x<=FLOAT(xsize)) & (y>=0.0)
                & (y<=FLOAT(ysize)) THEN RETURN 0 END; */
      xs = (float)maptool_xsize;
      ys = (float)maptool_ysize;
      if (((*x>=0.0f && *x<=xs) && *y>=0.0f) && *y<=ys) return 0L;
   }
   return -1L;
} /* end mapxy() */


extern void maptool_pullmap(int32_t x, int32_t y, char init)
{
   struct aprspos_POSITION top;
   uint32_t i;
   if (init) {
      maptool_xytodeg((float)x, (float)y, &aprsdecode_click.pullpos);
      xosi_sethand(xosi_cPULL4);
      xosi_pulling = 1;
   }
   else {
      for (i = 0UL; i<=9UL; i++) {
         maptool_xytodeg((float)x, (float)y, &top);
         aprsdecode_mappos.lat = aprsdecode_mappos.lat-(top.lat-aprsdecode_click.pullpos.lat)
                *0.5f;
         aprsdecode_mappos.long0 = aprsdecode_mappos.long0-(top.long0-aprsdecode_click.pullpos.long0)
                ;
         maptool_limpos(&aprsdecode_mappos);
      } /* end for */
   }
} /* end pullmap() */


extern void maptool_loctopos(struct aprspos_POSITION * pos, char loc[],
                uint32_t loc_len)
{
   uint32_t l;
   uint32_t i;
   char ok0;
   X2C_PCOPY((void **)&loc,loc_len);
   ok0 = 0;
   l = aprsstr_Length(loc, loc_len);
   i = 0UL;
   while (i<l) {
      loc[i] = X2C_CAP(loc[i]);
      ++i;
   }
   if ((((((((((((l>=6UL && (uint8_t)loc[0UL]>='A') && (uint8_t)
                loc[0UL]<='R') && (uint8_t)loc[1UL]>='A') && (uint8_t)
                loc[1UL]<='R') && (uint8_t)loc[2UL]>='0') && (uint8_t)
                loc[2UL]<='9') && (uint8_t)loc[3UL]>='0') && (uint8_t)
                loc[3UL]<='9') && (uint8_t)loc[4UL]>='A') && (uint8_t)
                loc[4UL]<='X') && (uint8_t)loc[5UL]>='A') && (uint8_t)
                loc[5UL]<='X') {
      pos->long0 = (float)((uint32_t)(uint8_t)loc[0UL]-65UL)
                *20.0f+(float)((uint32_t)(uint8_t)loc[2UL]-48UL)
                *2.0f+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[4UL]-65UL)+0.5f,12.0f);
      pos->lat = (float)((uint32_t)(uint8_t)loc[1UL]-65UL)
                *10.0f+(float)((uint32_t)(uint8_t)loc[3UL]-48UL)
                +X2C_DIVR((float)((uint32_t)(uint8_t)loc[5UL]-65UL)
                +0.5f,24.0f);
      if (l==6UL) ok0 = 1;
      if ((((l>=8UL && (uint8_t)loc[6UL]>='0') && (uint8_t)loc[6UL]<='9')
                 && (uint8_t)loc[7UL]>='0') && (uint8_t)loc[7UL]<='9') {
         pos->long0 = (pos->long0+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[6UL]-48UL),120.0f))-0.0375f;
         pos->lat = (pos->lat+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[7UL]-48UL),240.0f))-0.01875f;
         if (l==8UL) ok0 = 1;
      }
      if ((((l>=10UL && (uint8_t)loc[8UL]>='A') && (uint8_t)
                loc[8UL]<='X') && (uint8_t)loc[9UL]>='A') && (uint8_t)
                loc[9UL]<='X') {
         pos->long0 = (pos->long0+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[8UL]-65UL),2880.0f))-3.9930555555556E-3f;
         pos->lat = (pos->lat+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[9UL]-65UL),5760.0f))-1.9965277777778E-3f;
         if (l==10UL) ok0 = 1;
      }
   }
   if (ok0) {
      pos->long0 = (pos->long0-180.0f)*1.7453292519943E-2f;
      pos->lat = (pos->lat-90.0f)*1.7453292519943E-2f;
   }
   else aprsdecode_posinval(pos);
   X2C_PFREE(loc);
} /* end loctopos() */


extern void maptool_postoloc(char loc[], uint32_t loc_len,
                struct aprspos_POSITION pos)
{
   uint32_t bc;
   uint32_t lc;
   float br;
   float lr;
   maptool_limpos(&pos);
   lr = (pos.long0*5.7295779513082E+1f+180.0f)*2880.0f;
   if (lr<0.0f) lr = 0.0f;
   lc = aprsdecode_trunc(lr);
   br = (pos.lat*5.7295779513082E+1f+90.0f)*5760.0f;
   if (br<0.0f) br = 0.0f;
   bc = aprsdecode_trunc(br);
   loc[0UL] = (char)(65UL+lc/57600UL);
   loc[1UL] = (char)(65UL+bc/57600UL);
   loc[2UL] = (char)(48UL+(lc/5760UL)%10UL);
   loc[3UL] = (char)(48UL+(bc/5760UL)%10UL);
   loc[4UL] = (char)(65UL+(lc/240UL)%24UL);
   loc[5UL] = (char)(65UL+(bc/240UL)%24UL);
   loc[6UL] = (char)(48UL+(lc/24UL)%10UL);
   loc[7UL] = (char)(48UL+(bc/24UL)%10UL);
   loc[8UL] = (char)(65UL+lc%24UL);
   loc[9UL] = (char)(65UL+bc%24UL);
   loc[10UL] = 0;
} /* end postoloc() */

#define maptool_SRTM3DIR "srtm3"

#define maptool_SRTM1DIR "srtm1"

#define maptool_SRTM30DIR "srtm30"


/* === srtm lib */
static int32_t opensrtm(uint8_t t, uint32_t tlat, uint32_t tlong)
{
   char s[21];
   FN path;
   uint32_t xd;
   uint32_t yi;
   uint32_t xi;
   uint32_t n;
   int32_t f;
   useri_confstr(useri_fOSMDIR, path, 1024ul);
   if (t==3U) aprsstr_Append(path, 1024ul, "/srtm3/", 8ul);
   else if (t==1U) aprsstr_Append(path, 1024ul, "/srtm1/", 8ul);
   if (t<=3U) {
      if (tlat<90UL) {
         s[0U] = 'S';
         n = 90UL-tlat;
      }
      else {
         s[0U] = 'N';
         n = tlat-90UL;
      }
      s[1U] = (char)(n/10UL+48UL);
      s[2U] = (char)(n%10UL+48UL);
      if (tlong<180UL) {
         s[3U] = 'W';
         n = 180UL-tlong;
      }
      else {
         s[3U] = 'E';
         n = tlong-180UL;
      }
      s[4U] = (char)(n/100UL+48UL);
      s[5U] = (char)((n/10UL)%10UL+48UL);
      s[6U] = (char)(n%10UL+48UL);
      s[7U] = '.';
      s[8U] = 'h';
      s[9U] = 'g';
      s[10U] = 't';
      s[11U] = 0;
      aprsstr_Append(path, 1024ul, s, 21ul);
      return osi_OpenRead(path, 1024ul);
   }
   else {
      aprsstr_Append(path, 1024ul, "/srtm30/", 9ul);
      xi = tlong/40UL;
      xd = xi*40UL;
      if (xd<180UL) {
         s[0U] = 'W';
         n = 180UL-xd;
      }
      else {
         s[0U] = 'E';
         n = xd-180UL;
      }
      s[1U] = (char)(n/100UL+48UL);
      s[2U] = (char)((n/10UL)%10UL+48UL);
      s[3U] = (char)(n%10UL+48UL);
      if (tlat>=130UL) {
         s[4U] = 'N';
         s[5U] = '9';
         yi = 3UL;
      }
      else if (tlat>=90UL) {
         s[4U] = 'N';
         s[5U] = '4';
         yi = 2UL;
      }
      else if (tlat>=50UL) {
         s[4U] = 'S';
         s[5U] = '0';
         yi = 1UL;
      }
      else {
         s[4U] = 'S';
         s[5U] = '4';
         yi = 0UL;
      }
      if (srtm30fd[xi][yi].havefile && srtm30fd[xi][yi].fd!=-1L) {
         return srtm30fd[xi][yi].fd;
      }
      s[6U] = '0';
      s[7U] = '.';
      s[8U] = 'D';
      s[9U] = 'E';
      s[10U] = 'M';
      s[11U] = 0;
      aprsstr_Append(path, 1024ul, s, 21ul);
      f = osi_OpenRead(path, 1024ul);
      srtm30fd[xi][yi].fd = f;
      srtm30fd[xi][yi].havefile = 1;
      return f;
   }
   return 0;
} /* end opensrtm() */


static void purgesrtm(char all)
{
   uint32_t asize;
   uint32_t y;
   uint32_t x;
   uint32_t yd;
   uint32_t xd;
   pSRTMTILE pt;
   pSRTMLAT pl;
   pSRTMSTRIP pb;
   struct SRTMTILE * anonym;
   for (xd = 0UL; xd<=359UL; xd++) {
      pl = srtmcache[xd];
      if (pl) {
         for (yd = 0UL; yd<=179UL; yd++) {
            pt = pl[yd];
            if (pt && pt!=srtmmiss) {
               { /* with */
                  struct SRTMTILE * anonym = pt;
                  for (y = 0UL; y<=3599UL; y++) {
                     for (x = 0UL; x<=2UL; x++) {
                        pb = anonym->strips[x][y];
                        if (pb) {
                           if (anonym->used[x][y]>0U) {
                              --anonym->used[x][y];
                           }
                           if (all || anonym->used[x][y]==0U) {
                              asize = 2400UL;
                              if (pt->typ>3U) asize = 240UL;
                              osic_free((char * *) &pb, asize);
                              useri_debugmem.srtm -= asize;
                              anonym->strips[x][y] = 0;
                           }
                        }
                     } /* end for */
                  } /* end for */
               }
               if (all) {
                  if (pt->fd!=-1L) osic_Close(pt->fd);
                  osic_free((char * *) &pt, sizeof(struct SRTMTILE));
                  useri_debugmem.srtm -= sizeof(struct SRTMTILE);
                  pl[yd] = 0;
               }
            }
         } /* end for */
         if (all) {
            osic_free((char * *) &pl, sizeof(SRTMLAT));
            useri_debugmem.srtm -= sizeof(SRTMLAT);
            srtmcache[xd] = 0;
         }
      }
   } /* end for */
   if (all) {
      for (x = 0UL; x<=8UL; x++) {
         for (y = 0UL; y<=3UL; y++) {
            if (srtm30fd[x][y].havefile && srtm30fd[x][y].fd!=-1L) {
               osic_Close(srtm30fd[x][y].fd);
               srtm30fd[x][y].fd = -1L;
            }
         } /* end for */
      } /* end for */
   }
} /* end purgesrtm() */

#define maptool_NOALT0 32767.0


static float getsrtm1(uint32_t ilat, uint32_t ilong,
                uint32_t * div0)
/* 1 pixel altitude */
{
   uint32_t rdsize;
   uint32_t xdeg;
   uint32_t ydeg;
   uint32_t xx;
   uint32_t y;
   uint32_t x;
   uint32_t i;
   int32_t seek;
   int32_t f;
   pSRTMTILE pt;
   pSRTMSTRIP pb;
   int32_t a;
   uint8_t t;
   struct SRTMTILE * anonym;
   struct SRTMTILE * anonym0;
   uint32_t tmp;
   ydeg = ilat/3600UL;
   xdeg = ilong/3600UL;
   if (xdeg>359UL || ydeg>179UL) return 32767.0f;
   if (srtmcache[xdeg]==0) {
      /* empty lat array */
      osic_alloc((char * *) &srtmcache[xdeg], sizeof(SRTMLAT));
      if (srtmcache[xdeg]==0) return 32767.0f;
      /* out of memory */
      useri_debugmem.srtm += sizeof(SRTMLAT);
      memset((char *)srtmcache[xdeg],(char)0,sizeof(SRTMLAT));
   }
   else if (srtmcache[xdeg][ydeg]==srtmmiss) return 32767.0f;
   /* tile file not avaliable */
   pt = srtmcache[xdeg][ydeg];
   if (pt==0) {
      t = 1U;
      f = opensrtm(1U, ydeg, xdeg);
      if (f==-1L) {
         t = 3U;
         f = opensrtm(3U, ydeg, xdeg);
         if (f==-1L) {
            t = 30U;
            f = opensrtm(30U, ydeg, xdeg);
            if (f==-1L) {
               srtmcache[xdeg][ydeg] = srtmmiss;
               return 32767.0f;
            }
         }
      }
      /*INC(open); */
      osic_alloc((char * *) &pt, sizeof(struct SRTMTILE));
                /* a new 1x1 deg buffer */
      if (pt==0) return 32767.0f;
      useri_debugmem.srtm += sizeof(struct SRTMTILE);
      { /* with */
         struct SRTMTILE * anonym = pt;
         memset((char *)anonym->strips,(char)0,
                sizeof(pSRTMSTRIP [3][3600]));
         memset((char *)anonym->used,(char)0,10800UL);
         anonym->typ = t;
         anonym->fd = f;
      }
      srtmcache[xdeg][ydeg] = pt;
   }
   { /* with */
      struct SRTMTILE * anonym0 = pt;
      *div0 = (uint32_t)anonym0->typ;
      if (anonym0->typ==1U) {
         y = ilat%3600UL;
         x = ilong%3600UL;
         xx = x/1200UL;
         x = x%1200UL;
      }
      else if (anonym0->typ==3U) {
         y = (ilat%3600UL)/3UL;
         x = (ilong%3600UL)/3UL;
         xx = 0UL;
      }
      else {
         y = (ilat%3600UL)/30UL;
         x = (ilong%3600UL)/30UL;
         xx = 0UL;
      }
      pb = anonym0->strips[xx][y];
      if (pb==0) {
         if (anonym0->typ==1U) {
            seek = (int32_t)((3599UL-y)*7202UL+xx*2400UL);
            rdsize = 2400UL;
         }
         else if (anonym0->typ==3U) {
            seek = (int32_t)((1199UL-y)*2402UL);
            rdsize = 2400UL;
         }
         else {
            seek = (int32_t)((xdeg%40UL)*240UL-(ilat/30UL)*9600UL);
            if (ydeg>=130UL) seek += 207350400L;
            else if (ydeg>=90UL) seek += 149750400L;
            else if (ydeg>=50UL) seek += 103670400L;
            else seek += 57590400L;
            rdsize = 240UL; /* fill 1/10 buffer */
         }
         /*INC(miss); */
         osic_alloc((char * *) &pb, rdsize);
         if (pb==0) return 32767.0f;
         useri_debugmem.srtm += rdsize;
         anonym0->strips[xx][y] = pb;
         osic_Seek(anonym0->fd, (uint32_t)seek);
         if (osi_RdBin(anonym0->fd, (char *)pb, 2400u/1u,
                rdsize)!=(int32_t)rdsize) {
            tmp = rdsize/2UL-1UL;
            i = 0UL;
            if (i<=tmp) for (;; i++) {
               pb[i] = 32767;
               if (i==tmp) break;
            } /* end for */
         }
         else {
            /*
            c-translator frissts nicht
                    FOR i:=0 TO rdsize DIV 2-1 DO pb^[i]:=pb^[i]<<8 + pb^[i]
                >>8 END;  (* motorola format *)  
            */
            tmp = rdsize/2UL-1UL;
            i = 0UL;
            if (i<=tmp) for (;; i++) {
               pb[i] = (short)(X2C_LSH((uint16_t)pb[i],16,
                8)|X2C_LSH((uint16_t)pb[i],16,-8));
               if (i==tmp) break;
            } /* end for */
         }
      }
      /* motorola format */
      /*ELSE INC(hit); */
      anonym0->used[xx][y] = 10U;
      a = (int32_t)pb[x];
      if (a>30000L || a<-30000L) return 32767.0f;
      return (float)a;
   }
} /* end getsrtm1() */

#define maptool_NOALT 32767.0

#define maptool_ERRALT 30000.0


extern float maptool_getsrtm(struct aprspos_POSITION pos,
                uint32_t quality, float * resolution)
{
   uint32_t d;
   uint32_t div0;
   uint32_t ilong;
   uint32_t ilat;
   float vy;
   float vx;
   float a3;
   float a2;
   float a1;
   float a0;
   /*limpos(pos); */
   pos.lat = 3.24E+5f+pos.lat*2.0626480625299E+5f;
   pos.long0 = 6.48E+5f+pos.long0*2.0626480625299E+5f;
   ilat = aprsdecode_trunc(pos.lat);
   ilong = aprsdecode_trunc(pos.long0);
   a0 = getsrtm1(ilat, ilong, &div0);
   if (div0==1UL) {
      *resolution = 30.0f;
      if (quality>29UL) return a0;
   }
   else if (div0==3UL) {
      *resolution = 90.0f;
      if (quality>60UL) return a0;
   }
   else {
      *resolution = 900.0f;
      if (quality>300UL) return a0;
   }
   /*interpolate 4 dots */
   a1 = getsrtm1(ilat, ilong+div0, &d);
   a2 = getsrtm1(ilat+div0, ilong, &d);
   a3 = getsrtm1(ilat+div0, ilong+div0, &d);
   if (a0>30000.0f) {
      /* ignore missing pixels */
      a0 = a1;
      if (a0>30000.0f) {
         a0 = a2;
         if (a0>30000.0f) {
            a0 = a3;
            if (a0>30000.0f) return 32767.0f;
         }
      }
   }
   if (a1>30000.0f) a1 = a0;
   if (a2>30000.0f) a2 = a0;
   if (a3>30000.0f) a3 = a0;
   if (div0==1UL) {
      /* interpolation wights */
      vx = pos.long0-(float)ilong;
      vy = pos.lat-(float)ilat;
   }
   else if (div0==3UL) {
      vx = (pos.long0-(float)((ilong/3UL)*3UL))*3.3333333333333E-1f;
      vy = (pos.lat-(float)((ilat/3UL)*3UL))*3.3333333333333E-1f;
   }
   else {
      vx = (pos.long0-(float)((ilong/30UL)*30UL))*3.3333333333333E-2f;
      vy = (pos.lat-(float)((ilat/30UL)*30UL))*3.3333333333333E-2f;
   }
   return (a0*(1.0f-vx)+a1*vx)*(1.0f-vy)+(a2*(1.0f-vx)+a3*vx)*vy;
/* wighted median of 4 pixels*/
} /* end getsrtm() */


static void initsrtm(void)
{
   uint32_t yi;
   uint32_t xi;
   /*open:=0; miss:=0; hit:=0; */
   memset((char *)srtmcache,(char)0,sizeof(SRTMLONG));
   for (xi = 0UL; xi<=8UL; xi++) {
      for (yi = 0UL; yi<=3UL; yi++) {
         srtm30fd[xi][yi].havefile = 0;
      } /* end for */
   } /* end for */
} /* end initsrtm() */


extern void maptool_closesrtmfile(void)
{
   purgesrtm(1);
   /*WrInt(open, 10); WrInt(miss, 10); WrInt(hit, 10);
                WrStrLn(" open miss hit"); */
   initsrtm();
} /* end closesrtmfile() */

/* === srtm lib */
/*
PROCEDURE wgs84(lat, long, heig:REAL; VAR x,y,z:REAL);       (* wgs84 ecef *)
CONST
      A=6378137;
--      B=6356752;
--      F=(A-B)/A;
--      E=2*F-F*F;
      E=0.081819190842522;

VAR n,sl,h:REAL;
BEGIN
  sl:=sin(lat);
  n:=A/sqrt(1.0-E*E*sl*sl);
  h:=heig+n;
  z:=(n*(1.0-E*E)+heig)*sl;
  y:=h*sin(long)*cos(lat);
  x:=h*cos(long)*cos(lat);
END wgs84;
*/

static void wgs84s(float lat, float long0, float nn, float * x,
                float * y, float * z)
/* km */
{
   float c;
   float h;
   h = nn+6370.0f;
   *z = h*osic_sin(lat);
   c = osic_cos(lat);
   *y = h*osic_sin(long0)*c;
   *x = h*osic_cos(long0)*c;
} /* end wgs84s() */


static void wgs84r(float x, float y, float z, float * lat,
                float * long0, float * heig)
/* km */
{
   float h;
   h = x*x+y*y;
   if ((float)fabs(x)>(float)fabs(y)) {
      *long0 = osic_arctan(X2C_DIVR(y,x));
      if (x<0.0f) {
         if (y>0.0f) *long0 = 3.1415926535898f+*long0;
         else *long0 = *long0-3.1415926535898f;
      }
   }
   else {
      *long0 = 1.5707963267949f-osic_arctan(X2C_DIVR(x,y));
      if (y<0.0f) *long0 = *long0-3.1415926535898f;
   }
   *lat = osic_arctan(X2C_DIVR(z,osic_sqrt(h)));
   *heig = osic_sqrt(h+z*z)-6370.0f;
} /* end wgs84r() */


static float fresnel(float a, float b, float lambda)
{
   if (lambda==0.0f) return 0.0f;
   else return osic_sqrt(X2C_DIVR(lambda*a*b,a+b));
   return 0;
} /* end fresnel() */

#define maptool_M 700


static void frescol(float a, uint16_t * r, uint16_t * g,
                uint16_t * b)
{
   *b = 700U;
   *r = 0U;
   *g = 100U;
   if (a<=0.001f) {
      *r = 700U;
      *g = 0U;
      *b = 0U;
   }
   else if (a<0.999f) {
      *b = 0U;
      if (a<0.5f) {
         *r = 700U;
         *g = (uint16_t)aprsdecode_trunc(a*1400.0f);
      }
      else {
         *g = 700U;
         *r = (uint16_t)aprsdecode_trunc((1.0f-a)*1400.0f);
      }
   }
} /* end frescol() */


static void elevation(float x0, float y00, float z0, float x1,
                float y1, float z1, float * e0, float * e1)
{
   float r;
   float s;
   float c;
   float b;
   float a;
   *e1 = 0.0f;
   *e0 = 0.0f;
   a = osic_sqrt(x0*x0+y00*y00+z0*z0);
   b = osic_sqrt(x1*x1+y1*y1+z1*z1);
   x1 = x1-x0;
   y1 = y1-y00;
   z1 = z1-z0;
   c = osic_sqrt(x1*x1+y1*y1+z1*z1);
   /* halbwinkelsatz */
   s = (a+b+c)*0.5f;
   if (s==0.0f) return;
   r = X2C_DIVR((s-a)*(s-b)*(s-c),s);
   if (r<=0.0f) return;
   r = osic_sqrt(r);
   *e1 = 1.1459155902616E+2f*osic_arctan(X2C_DIVR(r,s-a))-90.0f;
   *e0 = 1.1459155902616E+2f*osic_arctan(X2C_DIVR(r,s-b))-90.0f;
} /* end elevation() */


static void ruler(maptool_pIMAGE image, float m, float x, float y,
                char over, char right)
/* write meter to largest heigth */
{
   signed char void0;
   char s[21];
   struct aprsdecode_COLTYP c;
   uint32_t l;
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(m,X2C_min_longint,X2C_max_longint),
                 0UL, s, 21ul);
   aprsstr_Append(s, 21ul, "m", 2ul);
   l = aprsstr_Length(s, 21ul)*6UL;
   c.r = 400UL;
   c.g = 400UL;
   c.b = 400UL;
   if (right) x = x-(float)l;
   if (over) y = y-(float)aprsdecode_lums.fontysize;
   maptool_drawstr(image, s, 21ul, osic_floor(x), osic_floor(y), 250UL, 0UL,
                c, &void0, 0UL, 1, 0);
} /* end ruler() */

#define maptool_SCALEHLINES 0.1
/* size of headroom lines to rf path size */

#define maptool_DOTSTEP 2

#define maptool_MAXDIST 1000000

#define maptool_HERR (-2.E+4)

#define maptool_ER 6.37E+6

struct HTAB;

typedef struct HTAB * pHTAB;


struct HTAB {
   pHTAB next;
   float tx;
   float ty;
   float alt;
};


extern int32_t maptool_geoprofile(maptool_pIMAGE image,
                struct aprspos_POSITION pos0, struct aprspos_POSITION pos1,
                float lambda, char ant1nn, int32_t ant1,
                int32_t ant2, float * dist, float * a1,
                float * a2, float * ele1, float * ele2)
{
   int32_t fstep;
   int32_t fs;
   int32_t nn;
   float yproj;
   float xproj;
   float ysh;
   float xsh;
   float wdif;
   float a;
   float resol;
   float hmid;
   float h1;
   float h0;
   float dh;
   float fullsum;
   float fresmin;
   float fressum;
   float refrac;
   float odist;
   float nv;
   float kf;
   float yo;
   float xo;
   float maxfres;
   float fres;
   float y;
   float x;
   float d;
   float h;
   float dz;
   float dy;
   float dx;
   float z1;
   float y1;
   float x1;
   float z0;
   float y00;
   float x0;
   struct aprspos_POSITION posf;
   struct aprspos_POSITION pos;
   uint16_t red;
   uint16_t green;
   uint16_t blue;
   pHTAB phmax;
   pHTAB pht;
   pHTAB phtab;
   uint32_t maxcache;
   int32_t tmp;
   maxcache = (uint32_t)(useri_conf2int(useri_fSRTMCACHE, 0UL, 0L, 2000L,
                20L)*1000000L);
   *dist = 0.0f;
   if (ant1nn) nn = 0L;
   else {
      nn = (int32_t)X2C_TRUNCI(maptool_getsrtm(pos0, 0UL, &resol),
                X2C_min_longint,X2C_max_longint);
      if (nn>=30000L) {
         maptool_closesrtmfile();
         return -1L;
      }
   }
   *a1 = (float)(nn+ant1);
   wgs84s(pos0.lat, pos0.long0,  *a1*0.001f, &x0, &y00, &z0);
   nn = (int32_t)X2C_TRUNCI(maptool_getsrtm(pos1, 0UL, &resol),
                X2C_min_longint,X2C_max_longint);
   if (nn>=30000L) {
      maptool_closesrtmfile();
      return -1L;
   }
   *a2 = (float)(nn+ant2);
   wgs84s(pos1.lat, pos1.long0,  *a2*0.001f, &x1, &y1, &z1);
   elevation(x0, y00, z0, x1, y1, z1, ele1, ele2);
   x1 = x1-x0;
   y1 = y1-y00;
   z1 = z1-z0;
   *dist = osic_sqrt(x1*x1+y1*y1+z1*z1)*1000.0f;
   if (*dist<1.0f) {
      maptool_closesrtmfile();
      return -3L;
   }
   if (*dist>1.E+6f) {
      maptool_closesrtmfile();
      return -2L;
   }
   refrac = (6.37E+6f-osic_sqrt(4.05769E+13f+ *dist* *dist*0.25f))
                *useri_conf2real(useri_fREFRACT, 0UL, (-10.0f), 10.0f,
                0.0f)*4.0f;
   /*WrFixed(refrac,3, 10); WrStrLn(" refra"); */
   odist = X2C_DIVR(0.7071f,*dist); /* step 1/sqrt(2) of map resolution */
   d = 0.0f;
   maxfres = fresnel( *dist*0.5f,  *dist*0.5f, lambda);
   if (maxfres==0.0f) maxfres = 1.0f;
   xo = 0.0f;
   yo = 0.0f;
   fresmin = 1.0f;
   nv = aprspos_azimuth(pos0, pos1)*1.7453292519943E-2f;
   phtab = 0;
   do {
      fres = fresnel( *dist*d,  *dist*(1.0f-d), lambda);
      dx = x0+x1*d;
      dy = y00+y1*d;
      dz = z0+z1*d;
      wgs84r(dx, dy, dz, &pos.lat, &pos.long0, &hmid);
      if (maptool_mapxy(pos, &x, &y)>=0L) {
         hmid = hmid*1000.0f-(d-d*d)*refrac;
         fressum = 0.0f;
         fullsum = 0.0f;
         fstep = (int32_t)aprsdecode_trunc(X2C_DIVR(fres*0.5f,resol));
         tmp = fstep;
         fs = -fstep;
         if (fs<=tmp) for (;; fs++) {
            kf = (X2C_DIVR((float)fs*resol,2.E+7f))*3.1415926535898f;
            posf.lat = pos.lat+kf*osic_sin(nv);
            posf.long0 = pos.long0-kf*osic_cos(nv);
            dh = sqr(fres*0.5f)-sqr((float)fs*resol);
            nn = (int32_t)X2C_TRUNCI(maptool_getsrtm(posf, 0UL,
                &resol)+0.5f,X2C_min_longint,X2C_max_longint);
            if (nn>=30000L) h = (-2.E+4f);
            else h = (float)nn;
            if (dh>0.0f) {
               dh = osic_sqrt(dh);
               h0 = hmid-dh;
               h1 = hmid+dh;
               /*WrFixed(dh, 3, 10); WrFixed(h0, 3, 10); WrFixed(h1, 3, 10);
                WrStrLn(" dh"); */
               fullsum = fullsum+(h1-h0);
               /*WrFixed(h, 3, 10); WrFixed(h0, 3, 10); WrStrLn(" hh0"); */
               if (h>h0) h0 = h;
               if (h1>h0) fressum = fressum+(h1-h0);
            }
            else {
               fullsum = 1.0f;
               if (h<hmid) fressum = 1.0f;
            }
            if (fs==tmp) break;
         } /* end for */
         if (fullsum>0.0f) {
            fressum = X2C_DIVR(fressum,fullsum);
            if (fressum<fresmin) fresmin = fressum;
            /*WrFixed(fullsum, 3, 10); WrFixed(fressum, 3, 10);
                WrStrLn(" ff"); */
            if (sqr(xo-x)+sqr(yo-y)>4.0f) {
               if (h==(-2.E+4f)) {
                  maptool_waypoint(image, x, y, 1.5f, 500L, 500L, 500L);
                /* have no srtm */
               }
               else {
                  frescol(fresmin, &red, &green, &blue);
                  maptool_waypoint(image, x, y, 1.25f+X2C_DIVR(1.25f*fres,
                maxfres), (int32_t)red, (int32_t)green, (int32_t)blue);
                  osic_alloc((char * *) &pht, sizeof(struct HTAB));
                /* store headroom values for scaling later */
                  if (pht) {
                     pht->next = phtab;
                     phtab = pht;
                     pht->alt = hmid-(float)nn;
                /* meters over (or under) ground */
                     pht->tx = x; /* pixel on screen */
                     pht->ty = y;
                  }
               }
               xo = x;
               yo = y;
               fresmin = 1.0f;
            }
         }
      }
      d = d+resol*odist;
      if (useri_debugmem.srtm>=maxcache) purgesrtm(0);
   } while (d<=1.0f);
   if (phtab) {
      phmax = 0;
      pht = phtab;
      d = X2C_max_real;
      a = 1.0f;
      for (;;) {
         /* find maximum for scaling */
         if ((float)fabs(pht->alt)>a) a = (float)fabs(pht->alt);
         h = (float)fabs((float)(image->Len1-1)*0.5f-pht->tx)
                +(float)fabs((float)(image->Len0-1)*0.5f-pht->ty);
         if (h<d) {
            phmax = pht;
            d = h;
         }
         if (pht->next==0) break;
         pht = pht->next;
      }
      /*WrFixed(a, 10,1);WrStrLn("m"); */
      d = osic_sqrt(1.0f+sqr(phtab->tx-pht->tx)+sqr(phtab->ty-pht->ty));
                /* rftrack pixels on screen */
      wdif = sqr((pos0.long0-pos1.long0)*osic_cos(pos0.lat))
                +sqr(pos0.lat-pos1.lat);
      if (wdif!=0.0f) {
         wdif = osic_sqrt(wdif); /* dist pos0-pos1 */
         wdif = X2C_DIVR(wdif*a,d*0.1f);
         xproj = X2C_DIVR(pos0.lat-pos1.lat,wdif);
                /* 90 deg rotatationsvector */
         yproj = X2C_DIVR((pos0.long0-pos1.long0)*osic_cos(pos0.lat),wdif);
                /* for headroom lines to track */
      }
      else {
         xproj = 0.0f;
         yproj = 0.0f;
      }
      while (phtab) {
         a = phtab->alt;
         if (a<0.0f) {
            red = 100U; /* no sight */
            green = 0U;
            blue = 0U;
         }
         else {
            red = 0U; /* sight */
            green = 50U;
            blue = 150U;
         }
         if (yproj<0.0f) a = -a;
         xsh = phtab->tx+a*xproj; /* rotate to normalvector */
         ysh = phtab->ty-a*yproj; /* rotate to normalvector */
         if (phtab==phmax && (float)fabs(a)<10000.0f) {
            /* max sight marker */
            ruler(image, phtab->alt, xsh, ysh, a*yproj>=0.0f, a*xproj<0.0f);
            red = 150U;
            green = 200U;
         }
         maptool_vector(image, phtab->tx, phtab->ty, xsh, ysh,
                (int32_t)red, (int32_t)green, (int32_t)blue, 256UL,
                0.0f); /* headroom line */
         maptool_waypoint(image, xsh, ysh, 1.5f, (int32_t)(red*3U),
                (int32_t)(green*3U), (int32_t)(blue*3U));
                /* thicken end of headroom line */
         pht = phtab;
         phtab = phtab->next;
         osic_free((char * *) &pht, sizeof(struct HTAB));
      }
   }
   return 0L;
} /* end geoprofile() */


static void progress(uint32_t startt, const char s[], uint32_t s_len,
                 uint32_t perc, char final)
{
   uint32_t rt;
   char ss1[101];
   char ss[101];
   rt = osic_time();
   if (final || rt!=aprsdecode_realtime) {
      aprsdecode_realtime = rt;
      if (final || startt+4UL<rt) {
         aprsdecode_click.cmd = 0;
         useri_refresh = 1;
         aprsstr_Assign(ss, 101ul, s, s_len);
         if (!final) {
            aprsstr_IntToStr((int32_t)perc, 3UL, ss1, 101ul);
            aprsstr_Append(ss, 101ul, ss1, 101ul);
            aprsstr_Append(ss, 101ul, "% (ESC to abort)", 17ul);
         }
         useri_textautosize(0L, 0L, 5UL, 6UL, 'g', ss, 101ul);
         xosi_Eventloop(1UL);
      }
   }
} /* end progress() */


static void postfilter(maptool_pIMAGE image, uint32_t colnr)
/* set missing pixels with median of neighbours */
{
   uint32_t y;
   uint32_t x;
   uint16_t c;
   struct maptool_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   tmp = (image->Len0-1)-1UL;
   y = 1UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = (image->Len1-1)-1UL;
      x = 1UL;
      if (x<=tmp0) for (;; x++) {
         if (colnr==0UL) {
            c = image->Adr[(x+1UL)*image->Len0+y].r;
            c += image->Adr[(x)*image->Len0+(y+1UL)].r;
            c += image->Adr[(x-1UL)*image->Len0+y].r;
            c += image->Adr[(x)*image->Len0+(y-1UL)].r;
            image->Adr[(x)*image->Len0+y].b = c/4U;
         }
         else {
            c = image->Adr[(x+1UL)*image->Len0+y].g;
            c += image->Adr[(x)*image->Len0+(y+1UL)].g;
            c += image->Adr[(x-1UL)*image->Len0+y].g;
            c += image->Adr[(x)*image->Len0+(y-1UL)].g;
            image->Adr[(x)*image->Len0+y].b = c/4U;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   tmp = (image->Len0-1)-1UL;
   y = 1UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = (image->Len1-1)-1UL;
      x = 1UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &image->Adr[(x)*image->Len0+y];
            if (colnr==0UL) {
               if (anonym->b>anonym->r) anonym->r = anonym->b;
            }
            else if (anonym->b>anonym->g) anonym->g = anonym->b;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end postfilter() */


static float meterperpix(maptool_pIMAGE image)
{
   struct aprspos_POSITION pos1;
   struct aprspos_POSITION pos;
   maptool_xytodeg(0.0f, 0.0f, &pos);
   maptool_xytodeg((float)(image->Len1-1), 0.0f, &pos1);
   return X2C_DIVR(aprspos_distance(pos, pos1)*1000.0f,
                (float)(image->Len1-1));
/* meter per pixel */
} /* end meterperpix() */


static void setcol(uint16_t bri, uint32_t colnr, struct maptool_PIX * p)
{
   if (colnr==0UL) {
      if (bri>p->r) p->r = bri;
   }
   else if (bri>p->g) p->g = bri;
} /* end setcol() */

/*
  PROCEDURE setcol(VAR p:PIX);
  BEGIN
    IF colnr=0 THEN p.r:=bri ELSE p.g:=bri END;
  END setcol;
*/
#define maptool_MAXSLANT 0.5
/* shadow deepness to step relation */

#define maptool_FULLUM 1000.0
/* full bright in image */

#define maptool_DISTHOP 6000
/* m jump sightline linear interpolated */

#define maptool_PIXHOP 50000
/* m jump pixels linear interpolated */


extern void maptool_Radiorange(maptool_pIMAGE image,
                struct aprspos_POSITION txpos, int32_t ant1,
                int32_t ant2, uint32_t smooth, uint32_t colnr,
                uint32_t qualnum, char * abort0)
{
   uint16_t bri;
   uint32_t progr;
   uint32_t qual;
   uint32_t maxcache;
   uint32_t frame;
   uint32_t yp;
   uint32_t xp;
   int32_t void0;
   int32_t nn;
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
   struct aprspos_POSITION dpos;
   struct aprspos_POSITION pos1;
   struct aprspos_POSITION pos0;
   struct aprspos_POSITION pos;
   uint32_t startt;
   char ss[101];
   startt = osic_time();
   nn = (int32_t)X2C_TRUNCI(maptool_getsrtm(txpos, 0UL, &resoltx),
                X2C_min_longint,X2C_max_longint);
                /* altitude of tx an map resolution in m here */
   if (nn>=30000L) {
      maptool_closesrtmfile();
      return;
   }
   atx = (float)(nn+ant1); /* ant1 over NN */
   wgs84s(txpos.lat, txpos.long0, atx*0.001f, &x0, &y00, &z0);
   arx = (float)ant2; /* ant2 over ground */
   xi = 0.0f;
   yi = 0.0f;
   nn = maptool_mapxy(txpos, &xtx, &ytx);
   frame = 0UL;
   maxcache = (uint32_t)(useri_conf2int(useri_fSRTMCACHE, 0UL, 0L, 2000L,
                100L)*1000000L);
   mperpix = meterperpix(image); /* meter per pixel */
   if (mperpix<1.0f) {
      maptool_closesrtmfile();
      return;
   }
   if (qualnum==0UL) {
      framestep = 2.2f; /* pixel step along corner of image */
      qual = (uint32_t)X2C_TRUNCC(mperpix,0UL,X2C_max_longcard);
      smooth = (smooth*22UL)/10UL;
   }
   else if (qualnum==1UL) {
      framestep = 1.5f;
      qual = (uint32_t)X2C_TRUNCC(mperpix*0.5f,0UL,X2C_max_longcard);
      smooth = (smooth*15UL)/10UL;
   }
   else {
      framestep = 1.0f;
      qual = (uint32_t)X2C_TRUNCC(mperpix*0.25f,0UL,X2C_max_longcard);
   }
   if (smooth>0UL) osmooth = X2C_DIVR(1000.0f,(float)smooth);
   else osmooth = 1000.0f;
   progr = 0UL;
   for (;;) {
      maptool_xytodeg(xi, yi, &pos); /* screen frame pos */
      if (((frame==0UL && ytx>0.0f || frame==2UL && ytx<(float)
                maptool_ysize) || frame==1UL && xtx>0.0f)
                || frame==3UL && xtx<(float)maptool_xsize) {
         wgs84s(pos.lat, pos.long0, atx*0.001f, &x1, &y1, &z1);
                /* screen frame xyz at ant1 alt */
         dx = x1-x0;
         dy = y1-y00;
         dz = z1-z0;
         oodist = dx*dx+dy*dy+dz*dz;
         if (oodist>1.E-6f) {
            oodist = X2C_DIVR(0.001f,osic_sqrt(oodist));
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
               alt = alt0+dalt*d;
               h = maptool_getsrtm(pos, qual, &resol);
                /* ground over NN in m */
               if (h<30000.0f) {
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
                           void0 = maptool_mapxy(pos, &x, &y);
                           void0 = maptool_mapxy(pos1, &px0, &py0);
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
                        yp = (uint32_t)X2C_TRUNCC(y,0UL,X2C_max_longcard);
                        if (xp<image->Len1-1 && yp<image->Len0-1) {
                           /* 1 pixel room for large low res pixel */
                           lum = sight*osmooth;
                /* luma is equal to riseing higth */
                           if (lum>1000.0f) lum = 1000.0f;
                           bri = (uint16_t)(uint32_t)X2C_TRUNCC(lum,0UL,
                X2C_max_longcard);
                           if (colnr==0UL) {
                              image->Adr[(xp)
                *image->Len0+yp].r = (uint16_t)(uint32_t)X2C_TRUNCC(lum,
                0UL,X2C_max_longcard);
                           }
                           else {
                              image->Adr[(xp)
                *image->Len0+yp].g = (uint16_t)(uint32_t)X2C_TRUNCC(lum,
                0UL,X2C_max_longcard);
                           }
                           if (qualnum<=1UL) {
                              bri = (uint16_t)(uint32_t)
                X2C_TRUNCC(lum*0.8f,0UL,X2C_max_longcard);
                              setcol(bri, colnr,
                &image->Adr[(xp+1UL)*image->Len0+yp]);
                              setcol(bri, colnr,
                &image->Adr[(xp)*image->Len0+(yp+1UL)]);
                              setcol(bri, colnr,
                &image->Adr[(xp-1UL)*image->Len0+(yp+1UL)]);
                              setcol(bri, colnr,
                &image->Adr[(xp)*image->Len0+(yp-1UL)]);
                              if (qualnum==0UL) {
                                 bri = (uint16_t)(uint32_t)
                X2C_TRUNCC(lum*0.5f,0UL,X2C_max_longcard);
                                 setcol(bri, colnr,
                &image->Adr[(xp-1UL)*image->Len0+(yp+1UL)]);
                                 setcol(bri, colnr,
                &image->Adr[(xp-1UL)*image->Len0+(yp-1UL)]);
                                 setcol(bri, colnr,
                &image->Adr[(xp+1UL)*image->Len0+(yp+1UL)]);
                                 setcol(bri, colnr,
                &image->Adr[(xp+1UL)*image->Len0+(yp-1UL)]);
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
         xi = xi+(1.0f+(float)fabs(X2C_DIVR(xi-xtx,yi-ytx)))*framestep;
         if (xi>=(float)(image->Len1-1)) {
            xi = 0.0f;
            ++frame;
         }
         break;
      case 1UL:
         yi = yi+(1.0f+(float)fabs(X2C_DIVR(yi-ytx,xi-xtx)))*framestep;
         if (yi>=(float)(image->Len0-1)) {
            yi = (float)(image->Len0-1);
            ++frame;
         }
         break;
      case 2UL:
         xi = xi+(1.0f+(float)fabs(X2C_DIVR(xi-xtx,yi-ytx)))*framestep;
         if (xi>=(float)(image->Len1-1)) {
            xi = (float)(image->Len1-1);
            ++frame;
         }
         break;
      default:;
         if (yi>0.0f) {
            yi = yi-(1.0f+(float)fabs(X2C_DIVR(yi-ytx,xi-xtx)))*framestep;
         }
         else goto loop_exit;
         break;
      } /* end switch */
      progr += 100UL;
      strncpy(ss,"Radiorange",101u);
      if (colnr) aprsstr_Append(ss, 101ul, " 2", 3ul);
      progress(startt, ss, 101ul,
                progr/(((image->Len0-1)+(image->Len1-1))*2UL), 0);
      if (!aprsdecode_click.withradio) {
         *abort0 = 1;
         break;
      }
      if (useri_debugmem.srtm>=maxcache) purgesrtm(0);
   }
   loop_exit:;
   postfilter(image, colnr); /* fill in missing pixels */
   useri_killmenuid(5UL);
} /* end Radiorange() */

#define maptool_MINH 1000
/* to max all alt positiv */

#define maptool_MINHISTPIX 10
/* min pixel in alt hist for max/min */

#define maptool_DIFMIN 10000

#define maptool_MAXDIF 500000


extern char maptool_SimpleRelief(maptool_pIMAGE image)
{
   uint32_t sum;
   uint32_t jump;
   uint32_t qual;
   uint32_t maxcache;
   uint32_t xi;
   uint32_t yp;
   uint32_t xp;
   float jm;
   float jd;
   float jr;
   float hr;
   float mperpix;
   float resol;
   struct aprspos_POSITION pos1;
   struct aprspos_POSITION pos;
   int32_t bri;
   int32_t mul;
   int32_t max0;
   int32_t min0;
   int32_t h;
   uint32_t startt;
   char ok0;
   uint32_t hist[10000];
   struct maptool_PIX lut[1024];
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   uint32_t tmp;
   uint32_t tmp0;
   startt = osic_time();
   maxcache = (uint32_t)(useri_conf2int(useri_fSRTMCACHE, 0UL, 0L, 2000L,
                100L)*1000000L);
   for (xi = 0UL; xi<=1023UL; xi++) {
      { /* with */
         struct maptool_PIX * anonym = &lut[xi];
         if (xi<400UL) anonym->r = (uint16_t)(((400UL-xi)*900UL)/400UL);
         else anonym->r = (uint16_t)(((xi-400UL)*900UL)/624UL);
         if (xi<200UL) anonym->g = (uint16_t)((xi*900UL)/200UL);
         else if (xi<640UL) {
            anonym->g = (uint16_t)(((640UL-xi)*900UL)/440UL);
         }
         else anonym->g = (uint16_t)(((xi-640UL)*900UL)/372UL);
         if (xi<320UL) anonym->b = 0U;
         else if (xi<640UL) {
            anonym->b = (uint16_t)(((xi-320UL)*900UL)/320UL);
         }
         else anonym->b = 900U;
      }
   } /* end for */
   /*
         r:=xi;
         g:=xi;
         b:=xi; 
   */
   mperpix = meterperpix(image);
   if (mperpix<1.0f) {
      maptool_closesrtmfile();
      return 0;
   }
   qual = (uint32_t)X2C_TRUNCC(mperpix*0.25f,0UL,X2C_max_longcard);
   jump = 1UL+aprsdecode_trunc(X2C_DIVR(6000.0f,mperpix));
   memset((char *)hist,(char)0,40000UL);
   yp = 0UL;
   ok0 = 0;
   do {
      xp = 0UL;
      maptool_xytodeg(0.0f, (float)yp, &pos);
      for (;;) {
         xi = xp;
         xp += jump;
         if (xp>image->Len1-1) xp = (image->Len1-1)+1UL;
         if (xi>=xp) break;
         jr = (float)(xp-xi);
         maptool_xytodeg((float)xp, (float)yp, &pos1);
         pos1.lat = X2C_DIVR(pos1.lat-pos.lat,jr);
         pos1.long0 = X2C_DIVR(pos1.long0-pos.long0,jr);
         do {
            hr = maptool_getsrtm(pos, qual, &resol);
            if (hr<10000.0f) {
               h = (int32_t)aprsdecode_trunc(1000.0f+hr);
                /* ground over NN in m */
               image->Adr[(xi)*image->Len0+yp].r = (uint16_t)h;
               /*        IF xi=0 THEN ho:=h END; */
               /*        image^[xi][yp].g:=h-ho+10000; */
               /*        ho:=h; */
               if (xi>0UL && yp>0UL) {
                  image->Adr[(xi)*image->Len0+yp].g = (uint16_t)
                (((h*3L-(int32_t)image->Adr[(xi-1UL)*image->Len0+yp].r*2L)
                -(int32_t)image->Adr[(xi)*image->Len0+(yp-1UL)].r)+10000L);
               }
               else image->Adr[(xi)*image->Len0+yp].g = 0U;
               if (h<=9999L) ++hist[h];
               ok0 = 1;
            }
            pos.lat = pos.lat+pos1.lat;
            pos.long0 = pos.long0+pos1.long0;
            ++xi;
         } while (xi<xp);
      }
      if (useri_debugmem.srtm>=maxcache) purgesrtm(0);
      ++yp;
      if (yp%20UL==0UL) {
         progress(startt, "Geomap", 7ul, (yp*100UL)/(image->Len0-1), 0);
      }
   } while (!(yp>image->Len0-1 || !aprsdecode_click.withradio));
   min0 = 0L;
   sum = 0UL;
   while (min0<9999L && sum<10UL) {
      sum += hist[min0];
      ++min0;
   }
   max0 = 9999L;
   sum = 0UL;
   while (max0>0L && sum<10UL) {
      sum += hist[max0];
      --max0;
   }
   if (aprsdecode_click.withradio && max0>0L) {
      h = max0-min0;
      if (h<10L) h = 10L;
      bri = useri_conf2int(useri_fGEOBRIGHTNESS, 0UL, 0L, 100L, 50L);
      mul = 1024000L/h;
      /*    difmul:=trunc(20000000.0/((FLOAT(h)*sqrt(mperpix))));
                (* highpass level *) */
      /*    difmul:=trunc(50000000.0/(sqrt(mperpix)*FLOAT(h)));
                (* highpass level *) */
      jm = X2C_DIVR(2.0f*(float)bri,(float)h*osic_sqrt(mperpix));
      tmp = image->Len0-1;
      yp = 0UL;
      if (yp<=tmp) for (;; yp++) {
         tmp0 = image->Len1-1;
         xp = 0UL;
         if (xp<=tmp0) for (;; xp++) {
            h = (int32_t)image->Adr[(xp)*image->Len0+yp].r;
            if (h) {
               h -= min0;
               /*          dif:=(VAL(INTEGER,
                image^[xp][yp].g)-DIFMIN)*difmul; */
               /*          IF dif>MAXDIF THEN dif:=MAXDIF ELSIF dif<-MAXDIF THEN dif:=-MAXDIF END;
                 */
               if (h<10000L) h = (h*mul)/1024L;
               else h = 0L;
               if (h<0L) h = 0L;
               else if (h>1023L) h = 1023L;
               { /* with */
                  struct maptool_PIX * anonym0 = &image->Adr[(xp)
                *image->Len0+yp];
                  jd = 1.0f+((float)anonym0->g-10000.0f)*jm;
                  if (jd<0.4f) jd = 0.4f;
                  else if (jd>2.0f) jd = 2.0f;
                  jd = jd*(float)bri*0.01f;
                  jr = (float)lut[h].r*jd;
                  if (jr<0.0f) jr = 0.0f;
                  anonym0->r = (uint16_t)aprsdecode_trunc(jr);
                  jr = (float)lut[h].g*jd;
                  if (jr<0.0f) jr = 0.0f;
                  anonym0->g = (uint16_t)aprsdecode_trunc(jr);
                  jr = (float)lut[h].b*jd;
                  if (jr<0.0f) jr = 0.0f;
                  anonym0->b = (uint16_t)aprsdecode_trunc(jr);
               }
            }
            if (xp==tmp0) break;
         } /* end for */
         if (yp==tmp) break;
      } /* end for */
   }
   else maptool_clr(image);
   useri_killmenuid(5UL);
   return ok0;
} /* end SimpleRelief() */

/*Panorama */
/*
PROCEDURE normvector(var nx,ny,nz:real;face:facepo);
var cx,cy,cz:real;
begin
  with face^ do begin
    with edge3^ do begin
      cx:=x;
      cy:=y;
      cz:=z;
    end;
    nx:=(edge1^.y-cy)*(edge2^.z-cz) - (edge1^.z-cz)*(edge2^.y-cy);
    ny:=(edge1^.z-cz)*(edge2^.x-cx) - (edge1^.x-cx)*(edge2^.z-cz);
    nz:=(edge1^.x-cx)*(edge2^.y-cy) - (edge1^.y-cy)*(edge2^.x-cx);
  end;
END normvector;
*/
#define maptool_TESTDIST 30.0


static void raytrace(float minqual, float x0, float y00,
                float z0, float dx, float dy, float dz,
                float maxdist, float * dist, float * lum,
                float * h, float * alt, float * subpix,
                struct aprspos_POSITION * pos)
{
   float deltah;
   float minsp;
   float sp;
   float lastsp;
   float h2;
   float h1;
   float resol;
   float qual;
   struct aprspos_POSITION pos1;
   *lum = 1.0f;
   qual = minqual;
   minsp = 0.0f;
   if (*dist==0.0f) lastsp = 0.0f;
   else lastsp = X2C_max_real;
   deltah = *alt;
   wgs84r(x0+dx* *dist, y00+dy* *dist, z0+dz* *dist, &pos->lat, &pos->long0,
                alt);
   *alt =  *alt*1000.0f;
   deltah = *alt-deltah;
   /*WrStr("next:"); WrFixed(deltah, 2, 12); */
   *subpix = 0.0f;
   do {
      wgs84r(x0+dx* *dist, y00+dy* *dist, z0+dz* *dist, &pos->lat,
                &pos->long0, alt);
      *alt =  *alt*1000.0f;
      /*
      IF mapxy(pos, xtt, ytt)>=-1 THEN
      waypoint(testimg, xtt,ytt,1.0, 255,255,100); END;
      */
      *h = maptool_getsrtm(*pos, aprsdecode_trunc(qual), &resol);
                /* ground over NN in m */
      /*WrFixed(dist, 1,15); WrFixed(alt, 1,15); WrFixed(h, 1,15);
                WrStr(" =d alt h"); */
      if (*h<30000.0f) {
         sp = *alt-*h;
         if (sp>0.0f) {
            qual = sp*0.25f;
            if (qual>250.0f) qual = 250.0f;
            /*        IF dist>1.0 THEN qual:=qual*dist END; */
            if (qual<minqual) qual = minqual;
            if (sp<lastsp) lastsp = sp;
            else if (minsp==0.0f) minsp = lastsp;
         }
         else {
            /* hit earth */
            pos1.lat = pos->lat+4.7123889803847E-6f;
            pos1.long0 = pos->long0;
            h1 = maptool_getsrtm(pos1, 1UL, &resol);
            pos1.long0 = pos->long0+X2C_DIVR(4.7123889803847E-6f,
                osic_cos(pos->lat));
            pos1.lat = pos->lat;
            h2 = maptool_getsrtm(pos1, 1UL, &resol);
            *lum = osic_cos(osic_arctan((h1-*h)*3.3333333333333E-2f))
                *osic_cos(osic_arctan((h2-*h)*3.3333333333333E-2f));
            /*        lum:=1.0-ABS(h-h1)*(2.0/TESTDIST); */
            /*        IF lum>1.0 THEN lum:=1.0 END; */
            /*
                    pos.long:=pos.long + (TESTDIST/20000000.0*pi)
                /cos(pos.lat);
                    h2:=getsrtm(pos, trunc(qual), resol);
                    lum:=cos(arctan(ABS(h1-h)*(1.0/TESTDIST)))
                *cos(arctan(ABS(h2-h)*(1.0/TESTDIST)));
            */
            if (deltah!=0.0f) {
               *subpix = X2C_DIVR(minsp,deltah);
               if (*subpix>1.0f) *subpix = 1.0f;
            }
            /*WrStr("subpix:"); WrFixed(subpix, 3, 7); */
            return;
         }
      }
      else qual = resol;
      *dist = *dist+qual;
   } while (*dist<=maxdist);
   /* we are in dust or heaven */
   if (deltah!=0.0f) {
      *subpix = X2C_DIVR(minsp,deltah);
      if (*subpix>1.0f) *subpix = 1.0f;
   }
} /* end raytrace() */
/* antialiasing: search for peaks and set highest pixel smooth */


static void rotvector(float * a, float * b, float cw, float sw)
{
   float h;
   h =  *a*cw+ *b*sw;
   *b =  *b*cw- *a*sw;
   *a = h;
} /* end rotvector() */

#define maptool_ERRALT0 30000

#define maptool_FULLUM0 1000.0
/* full bright in image */

#define maptool_MAXHP 0.08
/* vertical spatial luminance highpass */


static void Panofind(char find, const struct maptool_PANOWIN panpar,
                float * res, struct aprspos_POSITION * pos)
{
   uint32_t yi;
   uint32_t xi;
   int32_t nn;
   float azi;
   float cazi;
   float sazi;
   float zn;
   float yn;
   float xn;
   float clong;
   float slong;
   float ele0;
   float eled;
   float azid;
   float resoltx;
   float space;
   float lasth;
   float maxdist;
   float hc1;
   float hc;
   float lb;
   float lg;
   float lr;
   float hb;
   float hg;
   float hr;
   float z0;
   float y00;
   float x0;
   float azi0;
   float clat;
   float slat;
   float wy;
   float wx;
   float tree;
   float light;
   float llum;
   float lum;
   float lastlum;
   float dlum;
   float lummul;
   float atx;
   float oldh;
   float d;
   float oob;
   float oog;
   float oor;
   float ob;
   float og;
   float or;
   char heaven;
   uint32_t startt;
   struct aprsdecode_COLTYP col0;
   struct maptool_PIX * anonym;
   startt = osic_time();
   if ((!aprspos_posvalid(panpar.eye) || !aprspos_posvalid(panpar.horizon))
                || maptool_getsrtm(panpar.horizon, 0UL, &resoltx)>=30000.0f) {
      return;
   }
   /* no alt at horizon */
   maxdist = aprspos_distance(panpar.eye, panpar.horizon)*1000.0f;
   if (maxdist<100.0f) return;
   /* horizon too near */
   /*azi0:=conf2real(fANT2, 0, -360.0, 360.0, 0.0)-90; */
   /*maxdist:=FLOAT(conf2int(fANT2, 1, 1, 150, 20))*1000.0; */
   /*WrFixed(yzoom, 4,15); WrFixed(maxdist, 4,15); WrStrLn("yzoom dist"); */
   if (!find) {
      col0.r = (uint32_t)useri_conf2int(useri_fCOLMARK1, 0UL, 0L, 100L,
                0L);
      col0.g = (uint32_t)useri_conf2int(useri_fCOLMARK1, 1UL, 0L, 100L,
                0L);
      col0.b = (uint32_t)useri_conf2int(useri_fCOLMARK1, 2UL, 0L, 100L,
                100L);
      hr = (float)col0.r*0.01f;
      hg = (float)col0.g*0.01f;
      hb = (float)col0.b*0.01f;
      lummul = X2C_DIVR(1000.0f,maxdist);
   }
   nn = (int32_t)X2C_TRUNCI(maptool_getsrtm(panpar.eye, 0UL, &resoltx),
                X2C_min_longint,X2C_max_longint);
                /* altitude of tx an map resolution in m here */
   if (nn>=30000L) {
      maptool_closesrtmfile();
      return;
   }
   atx = (float)(nn+panpar.eyealt); /* ant1 over NN */
   wgs84s(panpar.eye.lat, panpar.eye.long0, atx*0.001f, &x0, &y00, &z0);
   azi = aprspos_azimuth(panpar.eye, panpar.horizon)*1.7453292519444E-2f;
   azi0 = -(panpar.angle*0.5f)*1.7453292519444E-2f;
   azid = X2C_DIVR(panpar.angle*1.7453292519444E-2f,
                (float)((panpar.image->Len1-1)+1UL));
   eled = X2C_DIVR(azid,panpar.yzoom);
   ele0 = (panpar.elevation-X2C_DIVR((X2C_DIVR(panpar.angle*0.5f,
                panpar.yzoom))*(float)((panpar.image->Len0-1)+1UL),
                (float)((panpar.image->Len1-1)+1UL)))*1.7453292519444E-2f;
   slat = osic_sin(-panpar.eye.lat);
   clat = osic_cos(-panpar.eye.lat);
   slong = osic_sin(-panpar.eye.long0);
   clong = osic_cos(-panpar.eye.long0);
   sazi = osic_sin(azi);
   cazi = osic_cos(azi);
   /*WrFixed(azi0/RAD, 4, 10); WrFixed(azid/RAD, 4, 10);
                WrStrLn(" adi0 azid"); */
   /*WrFixed(panpar.elevation*RAD/azid, 4, 12); WrStrLn(" adi0 azid"); */
   xi = 0UL;
   if (find) xi = (uint32_t)panpar.hx;
   do {
      /*    wx:=panpar.angle*RAD*(FLOAT(xi)-FLOAT(HIGH(panpar.image^)+1)*0.5)
                /FLOAT(HIGH(panpar.image^)+1); */
      wx = azi0+azid*(float)xi;
      if (panpar.flatscreen) wx = osic_arctan(wx);
      yi = 0UL;
      if (find) {
         yi = (uint32_t)((int32_t)((panpar.image->Len0-1)+1UL)-panpar.hy)
                ;
      }
      d = 0.0f;
      dlum = 0.0f;
      lastlum = 0.0f;
      heaven = 0;
      do {
         if (!heaven) {
            wy = ele0+eled*(float)yi;
            if (panpar.flatscreen) wy = osic_arctan(wy);
            /*IF xi=0 THEN WrFixed(wx/RAD, 2, 8); WrFixed(wy/RAD, 2, 8);
                WrStr(" wx wy"); END; */
            zn = osic_cos(wx)*osic_cos(wy);
            yn = osic_sin(wx);
            xn = osic_sin(wy);
            rotvector(&yn, &zn, cazi, sazi);
            /*IF xi=0 THEN WrFixed(xn, 2, 7); WrFixed(yn, 2, 7);
                WrFixed(zn, 2, 7); END; */
            rotvector(&xn, &zn, clat, slat);
            rotvector(&xn, &yn, clong, slong);
            /*IF (ABS(wy)>0.5) & (d>maxdist*0.1) THEN d:=d-maxdist*0.1 END;
                (* jump back if sight from above *) */
            raytrace(5.0f, x0, y00, z0, xn*0.001f, yn*0.001f, zn*0.001f,
                maxdist, &d, &light, &oldh, &lasth, &space, pos);
            if (d>maxdist) heaven = 1;
            if (find) {
               if (heaven) aprsdecode_posinval(pos);
               return;
            }
         }
         else light = 1.0f;
         if (heaven) {
            d = maxdist*4.0f*(1.25f-X2C_DIVR((float)yi,
                (float)(panpar.image->Len0-1)));
         }
         lum = d*0.2f;
         llum = (lum-lastlum)*20.0f;
         if (llum>0.08f*maxdist) llum = 0.08f*maxdist;
         if (llum>dlum) dlum = llum;
         lastlum = lum;
         if (!heaven) {
            tree = 1600.0f-oldh;
            if (tree<0.0f) tree = 0.0f;
            else if (tree>600.0f) tree = 600.0f;
            tree = tree*1.5833333333333E-3f*(1.0f-X2C_DIVR(d,maxdist));
            /*      tree:=tree*(0.95/600.0); */
            /*WrFixed(light, 5,2); WrStr(" "); */
            light = light-0.65f;
            if (light<0.0f) {
               light = light*(-2.0f);
               if (light>1.0f) light = 1.0f;
               light = 1.0f+light; /**(1.0-d/maxdist)*/
               light = light*(1.7f-tree);
               lr = lum*light*1.4f+dlum;
               lg = lum*light*1.1f+dlum;
               lb = lum*light*0.8f+dlum;
            }
            else {
               lr = lum*(1.7f-tree*1.3f)+dlum;
               lg = lum*(1.7f-tree*0.8f)+dlum;
               lb = lum*(1.7f-tree*1.7f)+dlum;
            }
            if (lr>maxdist) lr = maxdist;
            if (lg>maxdist) lg = maxdist;
            if (lb>maxdist) lb = maxdist;
            lr = lr*lummul;
            lg = lg*lummul;
            lb = lb*lummul;
         }
         lum = lum+dlum;
         if (lum>maxdist) lum = maxdist;
         lum = lum*lummul;
         dlum = dlum*0.84f;
         { /* with */
            struct maptool_PIX * anonym = &panpar.image->Adr[(xi)
                *panpar.image->Len0+yi];
            if (heaven) {
               /* heaven */
               /*          hc:=VAL(REAL, 2*yi)/VAL(REAL,
                HIGH(panpar.image^[0]))-1.0; */
               /*          hc:=(ele0 + eled*FLOAT(yi))*6.0; */
               hc = X2C_DIVR(((float)
                yi+X2C_DIVR(panpar.elevation*1.7453292519444E-2f,azid))*2.0f,
                (float)(panpar.image->Len0-1))-1.0f;
               if (hc<0.0f) hc = 0.0f;
               else if (hc>1.0f) hc = 1.0f;
               hc1 = 1.0f-hc;
               anonym->r = (uint16_t)aprsdecode_trunc(lum*hc1+lum*hr*hc);
               anonym->g = (uint16_t)aprsdecode_trunc(lum*hc1+lum*hg*hc);
               anonym->b = (uint16_t)aprsdecode_trunc(lum*hc1+lum*hb*hc);
            }
            else {
               anonym->r = (uint16_t)aprsdecode_trunc(lr);
               anonym->g = (uint16_t)aprsdecode_trunc(lg);
               anonym->b = (uint16_t)aprsdecode_trunc(lb);
            }
            or = (float)anonym->r;
            og = (float)anonym->g;
            ob = (float)anonym->b;
            if (yi>1UL) {
               anonym->r = (uint16_t)
                aprsdecode_trunc(or*space+oor*(1.0f-space));
               anonym->g = (uint16_t)
                aprsdecode_trunc(og*space+oog*(1.0f-space));
               anonym->b = (uint16_t)
                aprsdecode_trunc(ob*space+oob*(1.0f-space));
            }
            oor = or;
            oog = og;
            oob = ob;
         }
         ++yi;
      } while (yi<=panpar.image->Len0-1);
      /*WrInt(xi, 5); WrFixed(pos.lat*180.0/pi, 4,8);
                WrFixed(pos.long*180.0/pi, 4,8); WrStrLn(" xi lat long az");
                */
      ++xi;
   } while (xi<=panpar.image->Len1-1);
/*  killmenuid(5); */
} /* end Panofind() */


extern void maptool_Panorama(maptool_pIMAGE testimg,
                struct maptool_PANOWIN panpar, char * abort0)
{
   float res;
   struct aprspos_POSITION pos;
   Panofind(0, panpar, &res, &pos);
} /* end Panorama() */


extern void maptool_findpanopos(struct maptool_PANOWIN panpar,
                struct aprspos_POSITION * pos, float * dist,
                int32_t * alt)
{
   float resol;
   float res;
   Panofind(1, panpar, &res, pos);
   if (aprspos_posvalid(*pos)) {
      *alt = (int32_t)X2C_TRUNCI(maptool_getsrtm(*pos, 0UL, &resol)+0.5f,
                X2C_min_longint,X2C_max_longint);
      *dist = aprspos_distance(panpar.eye, *pos);
   }
/*WrFixed(res, 1, 10); WrStrLn("=d"); */
} /* end findpanopos() */

/*
find exact earth point with cross point of trace and earth line
yzoom xzoom(deg) ypan xpan(deg) maxdist ant mousepos show-marker2 sun-xy snow-alt heaven-col
cursor ^ v < > + -

no interpolate in search mode
antialiasing measure scanline to peack distance
start rescan next y with same d
soft change wood-rock
*/
/*Panorama */

extern void maptool_xytoloc(struct aprspos_POSITION mpos, char s[],
                uint32_t s_len)
/* lat/long + locator string of pos */
{
   char h[101];
   int32_t nn;
   float resol;
   /*  xytodeg(VAL(REAL,x), VAL(REAL,y), mpos); */
   aprstext_postostr(mpos, '3', s, s_len);
   aprsstr_Append(s, s_len, " \373", 3ul);
   aprstext_postostr(mpos, '1', h, 101ul);
   aprsstr_Append(s, s_len, h, 101ul);
   aprsstr_Append(s, s_len, " \376", 3ul);
   aprsstr_FixToStr(X2C_DIVR(mpos.lat,1.7453292519444E-2f), 6UL, h, 101ul);
   aprsstr_Append(s, s_len, h, 101ul);
   aprsstr_Append(s, s_len, " ", 2ul);
   aprsstr_FixToStr(X2C_DIVR(mpos.long0,1.7453292519444E-2f), 6UL, h, 101ul);
   aprsstr_Append(s, s_len, h, 101ul);
   aprsstr_Append(s, s_len, " \373", 3ul);
   maptool_postoloc(h, 101ul, mpos);
   aprsstr_Append(s, s_len, h, 101ul);
   aprsstr_Append(s, s_len, "\376", 2ul);
   nn = (int32_t)X2C_TRUNCI(maptool_getsrtm(mpos, 0UL, &resol)+0.5f,
                X2C_min_longint,X2C_max_longint);
   if (nn<30000L) {
      aprsstr_IntToStr(nn, 0UL, h, 101ul);
      aprsstr_Append(s, s_len, " ", 2ul);
      aprsstr_Append(s, s_len, h, 101ul);
      aprsstr_Append(s, s_len, "m", 2ul);
   }
   if (aprspos_posvalid(aprsdecode_click.markpos)) {
      /*(click.marktime=0) &*/
      aprsstr_FixToStr(aprspos_distance(aprsdecode_click.markpos, mpos), 4UL,
                 h, 101ul);
      aprsstr_Append(s, s_len, " \373", 3ul);
      aprsstr_Append(s, s_len, h, 101ul);
      aprsstr_Append(s, s_len, "km ", 4ul);
      aprsstr_FixToStr(aprspos_azimuth(aprsdecode_click.markpos, mpos), 2UL,
                h, 101ul);
      aprsstr_Append(s, s_len, h, 101ul);
      aprsstr_Append(s, s_len, "\177\376", 3ul);
   }
} /* end xytoloc() */

#define maptool_MOUNTAINDIST 10.0


extern void maptool_POIname(struct aprspos_POSITION * mpos, char s[],
                uint32_t s_len)
/* get name of nearest POI */
{
   char h[101];
   float mindist;
   float d;
   aprsdecode_pMOUNTAIN pmin;
   aprsdecode_pMOUNTAIN pm;
   struct aprsdecode_MOUNTAIN * anonym;
   pm = aprsdecode_mountains;
   mindist = 2560.0f*osic_power(2.0f, -maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
   if (mindist>10.0f) mindist = 10.0f;
   /*WrFixed(mpos.lat, 4, 8);WrFixed(mpos.long, 4, 8); WrStrLn(""); */
   pmin = 0;
   while (pm) {
      { /* with */
         struct aprsdecode_MOUNTAIN * anonym = pm;
         if ((float)fabs(anonym->pos.lat-mpos->lat)
                <1.5707963267949E-3f && (float)
                fabs(anonym->pos.long0-mpos->long0)<1.5707963267949E-3f) {
            d = aprspos_distance(anonym->pos, *mpos);
            /*WrFixed(d, 1, 6); WrStrLn(""); */
            if (d<mindist) {
               mindist = d;
               pmin = pm;
            }
         }
         pm = anonym->next;
      }
   }
   if (pmin) {
      aprsstr_Assign(s, s_len, pmin->name, 32ul);
      if (pmin->alt>0) {
         aprsstr_Append(s, s_len, " ", 2ul);
         aprsstr_IntToStr((int32_t)pmin->alt, 0UL, h, 101ul);
         aprsstr_Append(s, s_len, h, 101ul);
         aprsstr_Append(s, s_len, "m", 2ul);
      }
      *mpos = pmin->pos;
   }
   else s[0UL] = 0;
} /* end POIname() */

#define maptool_WILDCARD "*"

#define maptool_WILD "?"


static void POIfindfrom(struct aprspos_POSITION * mpos, char s[],
                uint32_t s_len)
/* get position of POI name */
{
   uint32_t cnt;
   uint32_t ls;
   uint32_t i;
   aprsdecode_pMOUNTAIN pmax;
   aprsdecode_pMOUNTAIN pm;
   struct aprsdecode_MOUNTAIN * anonym;
   X2C_PCOPY((void **)&s,s_len);
   pm = aprsdecode_mountains;
   pmax = 0;
   if (31UL<s_len-1) s[31UL] = 0;
   s[s_len-1] = 0;
   ls = aprsstr_Length(s, s_len);
   cnt = 0UL;
   for (;;) {
      if (pm==0) break;
      { /* with */
         struct aprsdecode_MOUNTAIN * anonym = pm;
         if (cnt>=lastpoinum) {
            /* start from last time */
            i = 0UL;
            while (i<ls && (s[i]=='?' || X2C_CAP(anonym->name[i])
                ==X2C_CAP(s[i]))) ++i;
            if (i>=ls && anonym->name[i]==0 || s[i]=='*') {
               pmax = pm; /* fit */
               break;
            }
         }
         ++cnt;
         pm = anonym->next;
      }
   }
   if (pmax) {
      *mpos = pmax->pos;
      aprsdecode_click.bubblpos = pmax->pos;
      aprsstr_Assign(aprsdecode_click.bubblstr, 50ul, pmax->name, 32ul);
      aprsdecode_click.lastpoi = lastpoinum==0UL;
      lastpoinum = cnt+1UL; /* next time goon from this entry */
   }
   else aprsdecode_posinval(mpos);
   X2C_PFREE(s);
} /* end POIfindfrom() */


extern void maptool_POIfind(struct aprspos_POSITION * mpos, char s[],
                uint32_t s_len)
/* get position of POI name */
{
   if (lastpoinum==0UL) POIfindfrom(mpos, s, s_len);
   else {
      POIfindfrom(mpos, s, s_len); /* find next */
      if (!aprspos_posvalid(*mpos)) {
         lastpoinum = 0UL;
         POIfindfrom(mpos, s, s_len); /* else try again from start */
      }
   }
} /* end POIfind() */


extern void maptool_Colset(struct aprsdecode_COLTYP * c, char w)
{
   c->r = 256UL;
   c->g = 256UL;
   c->b = 256UL;
   if (w=='Y') c->b = 0UL;
   else if (w=='R') {
      c->b = 0UL;
      c->g = 0UL;
   }
   else if (w=='G') {
      c->r = 0UL;
      c->b = 0UL;
   }
   else if (w=='B') {
      c->r = 0UL;
      c->g = 60UL;
   }
   else if (w=='O') {
      c->b = 0UL;
      c->g = 150UL;
   }
   else if (w=='V') c->g = 0UL;
   else if (w=='L') c->r = 160UL;
   else if (w=='0') {
      c->r = 255UL;
      c->g = 40UL;
      c->b = 10UL;
   }
   else if (w=='1') {
      c->r = 240UL;
      c->g = 120UL;
      c->b = 10UL;
   }
   else if (w=='2') {
      c->r = 240UL;
      c->g = 190UL;
      c->b = 10UL;
   }
   else if (w=='3') {
      c->r = 240UL;
      c->g = 250UL;
      c->b = 10UL;
   }
   else if (w=='4') {
      c->r = 110UL;
      c->g = 250UL;
      c->b = 10UL;
   }
   else if (w=='5') {
      c->r = 10UL;
      c->g = 250UL;
      c->b = 50UL;
   }
   else if (w=='6') {
      c->r = 10UL;
      c->g = 190UL;
      c->b = 220UL;
   }
   else if (w=='7') {
      c->r = 10UL;
      c->g = 90UL;
      c->b = 255UL;
   }
   else if (w=='8') {
      c->r = 140UL;
      c->g = 5UL;
      c->b = 255UL;
   }
   else if (w=='9') {
      c->r = 250UL;
      c->g = 5UL;
      c->b = 200UL;
   }
} /* end Colset() */


static void addcol(struct maptool_PIX * pixel, int32_t rr, int32_t gg,
                int32_t bb, int32_t f)
{
   int32_t fh;
   struct maptool_PIX * anonym;
   { /* with */
      struct maptool_PIX * anonym = pixel;
      fh = (int32_t)anonym->r+(rr*f)/256L;
      if (fh<0L) fh = 0L;
      else if (fh>30000L) fh = 30000L;
      anonym->r = (uint16_t)fh;
      fh = (int32_t)anonym->g+(gg*f)/256L;
      if (fh<0L) fh = 0L;
      else if (fh>30000L) fh = 30000L;
      anonym->g = (uint16_t)fh;
      fh = (int32_t)anonym->b+(bb*f)/256L;
      if (fh<0L) fh = 0L;
      else if (fh>30000L) fh = 30000L;
      anonym->b = (uint16_t)fh;
   }
} /* end addcol() */


extern void maptool_waypoint(maptool_pIMAGE image, float x, float y,
                float r, int32_t rr, int32_t gg, int32_t bb)
{
   int32_t ri;
   int32_t ty;
   int32_t tx;
   int32_t yi;
   int32_t xi;
   float h;
   float my;
   float mx;
   float fy;
   float fx;
   int32_t tmp;
   int32_t tmp0;
   if (((x>r && x<(float)((image->Len1-1)+1UL)-r) && y>r) && y<(float)
                ((image->Len0-1)+1UL)-r) {
      ri = (int32_t)(aprsdecode_trunc(r)+1UL);
      tx = (int32_t)aprsdecode_trunc(x);
      fx = x-(float)tx;
      ty = (int32_t)aprsdecode_trunc(y);
      fy = y-(float)ty;
      tmp = ri;
      yi = -ri;
      if (yi<=tmp) for (;; yi++) {
         tmp0 = ri;
         xi = -ri;
         if (xi<=tmp0) for (;; xi++) {
            mx = (float)xi-fx;
            my = (float)yi-fy;
            h = mx*mx+my*my;
            if (h>0.0f) h = osic_sqrt(h);
            h = r-h;
            if (h>0.0f) {
               if (h>1.0f) h = 1.0f;
               addcol(&image->Adr[(tx+xi)*image->Len0+(ty+yi)], rr, gg, bb,
                (int32_t)aprsdecode_trunc(h*256.0f));
            }
            if (xi==tmp0) break;
         } /* end for */
         if (yi==tmp) break;
      } /* end for */
   }
} /* end waypoint() */

#define maptool_K 65536

#define maptool_KF 256


extern void maptool_vector(maptool_pIMAGE image, float x0, float y00,
                float x1, float y1, int32_t rr, int32_t gg,
                int32_t bb, uint32_t width, float glow)
{
   float ro;
   float w2;
   float w1;
   float r;
   float h;
   uint32_t f1;
   uint32_t iw2;
   uint32_t e1;
   uint32_t de0;
   uint32_t e0;
   uint32_t n0;
   uint32_t n;
   uint32_t yb;
   uint32_t ya;
   uint32_t yie;
   uint32_t yi;
   uint32_t xi;
   int32_t fgg;
   int32_t fbb;
   int32_t frr;
   int32_t k;
   char mirr;
   char mirror;
   char flip;
   /*scissoring*/
   mirr = y00>y1;
   if (x0>x1) {
      h = x1;
      x1 = x0;
      x0 = h;
      h = y1;
      y1 = y00;
      y00 = h;
      mirror = 1;
   }
   else mirror = 0;
   if (x0>=(float)((image->Len1-1)-1UL) || x1<=0.0f) return;
   /* hor out */
   if (x0<=0.0f) {
      y00 = y00+X2C_DIVR((y1-y00)*x0,x0-x1);
      x0 = 1.0f;
   }
   if (x1>=(float)((image->Len1-1)-1UL)) {
      y1 = y00+X2C_DIVR((y1-y00)*((float)((image->Len1-1)-1UL)-x0),x1-x0);
      x1 = (float)((image->Len1-1)-1UL);
   }
   if (y00<=0.0f) {
      if (y1<=0.0f) return;
      /* bottom out */
      x0 = x0+X2C_DIVR((x1-x0)*y00,y00-y1);
      y00 = 1.0f;
   }
   else if (y00>=(float)((image->Len0-1)-1UL)) {
      if (y1>=(float)((image->Len0-1)-1UL)) return;
      /* top out */
      x0 = x0+X2C_DIVR((x1-x0)*((float)((image->Len0-1)-1UL)-y00),y1-y00);
      y00 = (float)((image->Len0-1)-1UL);
   }
   if (y1<=0.0f) {
      x1 = x1+X2C_DIVR((x1-x0)*y1,y00-y1);
      y1 = 1.0f;
   }
   else if (y1>=(float)((image->Len0-1)-1UL)) {
      x1 = x0+X2C_DIVR((x1-x0)*((float)((image->Len0-1)-1UL)-y00),y1-y00);
      y1 = (float)((image->Len0-1)-1UL);
   }
   /*scissoring*/
   if (x0<2.0f) x0 = 2.0f;
   if (x0>(float)((image->Len1-1)-1UL)) {
      x0 = (float)((image->Len1-1)-1UL);
   }
   if (y00<2.0f) y00 = 2.0f;
   if (y00>(float)((image->Len0-1)-1UL)) {
      y00 = (float)((image->Len0-1)-1UL);
   }
   if (x1<2.0f) x1 = 2.0f;
   if (x1>(float)((image->Len1-1)-1UL)) {
      x1 = (float)((image->Len1-1)-1UL);
   }
   if (y1<2.0f) y1 = 2.0f;
   if (y1>(float)((image->Len0-1)-1UL)) {
      y1 = (float)((image->Len0-1)-1UL);
   }
   w1 = X2C_DIVR((float)width,512.0f);
   if (x1-x0<(float)fabs(y1-y00)) {
      h = x0;
      x0 = y00;
      y00 = h;
      h = x1;
      x1 = y1;
      y1 = h;
      flip = 1;
   }
   else flip = 0;
   if (x0>x1) {
      h = y1;
      y1 = y00;
      y00 = h;
      h = x1;
      x1 = x0;
      x0 = h;
      mirror = !mirror;
   }
   if (x1==x0) return;
   h = X2C_DIVR(y1-y00,x1-x0);
   ro = osic_sqrt(1.0f+h*h);
   w2 = w1*ro;
   iw2 = aprsdecode_trunc(w2*65536.0f);
   r = w1*osic_sin(osic_arctan(h));
   xi = aprsdecode_trunc(x0-(float)fabs(r));
   w1 = (((y00-h*(float)fabs(r))-w2)-0.5f)*65536.0f;
   if (w1<0.0f) ya = 0UL;
   else ya = aprsdecode_trunc(w1);
   /*
     ya:=TRUNC((y0-h*ABS(r)-w2-0.5)*K);
   */
   e0 = iw2*2UL;
   yb = ya+e0;
   if ((float)fabs(r)>0.1f) {
      de0 = aprsdecode_trunc(X2C_DIVR((float)e0,(float)fabs(r)*2.0f));
   }
   else de0 = e0;
   k = (int32_t)X2C_TRUNCI(h*65536.0f,X2C_min_longint,X2C_max_longint);
   n = aprsdecode_trunc((x1-(float)xi)+(float)fabs(r));
   mirr = y00>y1;
   if (aprsdecode_click.dryrun) ya = (ya+yb)/2UL;
   glow = X2C_DIVR(glow,ro);
   e1 = 0UL;
   n0 = n;
   while (n>0UL) {
      if (aprsdecode_click.dryrun) {
         yi = ya/65536UL;
         if (flip) {
            findinfo((int32_t)(yi-1UL), (int32_t)xi);
            findinfo((int32_t)yi, (int32_t)xi);
            findinfo((int32_t)(yi+1UL), (int32_t)xi);
         }
         else {
            findinfo((int32_t)xi, (int32_t)(yi-1UL));
            findinfo((int32_t)xi, (int32_t)yi);
            findinfo((int32_t)xi, (int32_t)(yi+1UL));
         }
      }
      else {
         if (mirror) h = (float)n;
         else h = (float)(n0-n);
         if (h<glow) {
            h = X2C_DIVR(h,glow);
            frr = (int32_t)aprsdecode_trunc((float)rr*(1.125f-h)*6.0f);
            fgg = (int32_t)aprsdecode_trunc((float)gg*h);
            fbb = (int32_t)aprsdecode_trunc((float)bb*h);
         }
         else {
            frr = rr;
            fbb = bb;
            fgg = gg;
         }
         if (mirr) {
            if (e0>de0) {
               yie = yb-e0;
               e0 -= de0;
            }
            else yie = yb;
            if (n<=aprsdecode_trunc((float)fabs(r)*2.0f)) ya += de0;
            yi = ya;
         }
         else {
            if (e0>de0) {
               yi = ya+e0;
               e0 -= de0;
            }
            else yi = ya;
            if (n<=aprsdecode_trunc((float)fabs(r)*2.0f)) yb -= de0;
            yie = yb;
         }
         f1 = 256UL-(yi&65535UL)/256UL;
         yi = (yi/65536UL)*65536UL;
         do {
            if (flip) {
               addcol(&image->Adr[(yi/65536UL)*image->Len0+xi], frr, fgg,
                fbb, (int32_t)f1);
            }
            else {
               addcol(&image->Adr[(xi)*image->Len0+yi/65536UL], frr, fgg,
                fbb, (int32_t)f1);
            }
            yi += 65536UL;
            f1 = 256UL;
         } while (yi<yie);
         f1 = ((65536UL+yie)-yi&65535UL)/256UL;
         if (flip) {
            addcol(&image->Adr[(yi/65536UL)*image->Len0+xi], frr, fgg, fbb,
                (int32_t)f1);
         }
         else {
            addcol(&image->Adr[(xi)*image->Len0+yi/65536UL], frr, fgg, fbb,
                (int32_t)f1);
         }
         /*      INC(yb, k); */
         yb = (uint32_t)((int32_t)yb+k);
      }
      /*    INC(ya, k); */
      ya = (uint32_t)((int32_t)ya+k);
      ++xi;
      --n;
   }
} /* end vector() */


extern void maptool_setmark(maptool_pIMAGE image,
                struct aprspos_POSITION pos, char hard)
{
   int32_t i;
   float y;
   float x;
   struct aprsdecode_COLTYP col0;
   if (aprspos_posvalid(pos) && maptool_mapxy(pos, &x, &y)>=0L) {
      col0.r = 0UL;
      col0.g = 1000UL;
      col0.b = 1000UL;
      if (hard) {
         col0.r = 1000UL;
         col0.b = 0UL;
      }
      for (i = -6L; i<=6L; i++) {
         maptool_waypoint(image, x+(float)(i*3L), y, 1.3f,
                (int32_t)col0.r, (int32_t)col0.g, (int32_t)col0.b);
         maptool_waypoint(image, x, y+(float)(i*3L), 1.3f,
                (int32_t)col0.r, (int32_t)col0.g, (int32_t)col0.b);
      } /* end for */
   }
} /* end setmark() */

#define maptool_ALPHA 255

#define maptool_WHITE 1023


static void dim(float dimmlev, maptool_pIMAGE img, int32_t x,
                int32_t y)
{
   float lum;
   struct maptool_PIX * anonym;
   if (((x<0L || x>=(int32_t)(img->Len1-1)) || y<0L) || y>=(int32_t)
                (img->Len0-1)) return;
   { /* with */
      struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+y];
      lum = (float)((uint32_t)anonym->r*87UL+(uint32_t)
                anonym->g*140UL+(uint32_t)anonym->b*28UL);
      if (lum>dimmlev) {
         lum = X2C_DIVR(dimmlev,lum);
         anonym->r = (uint16_t)aprsdecode_trunc((float)anonym->r*lum);
         anonym->g = (uint16_t)aprsdecode_trunc((float)anonym->g*lum);
         anonym->b = (uint16_t)aprsdecode_trunc((float)anonym->b*lum);
      }
   }
} /* end dim() */


extern void maptool_drawchar(maptool_pIMAGE img, char ch, float x0r,
                float y0r, int32_t * inc0, uint32_t bri,
                uint32_t contrast, struct aprsdecode_COLTYP col0,
                char dryrun)
{
   int32_t fine;
   int32_t yy;
   int32_t xx;
   int32_t y00;
   int32_t x0;
   int32_t y;
   int32_t x;
   uint32_t cn;
   uint32_t cf;
   uint32_t c;
   uint32_t fy;
   uint32_t fx;
   float dimmlev;
   struct _1 * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   struct maptool_PIX * anonym3;
   int32_t tmp;
   if (((uint8_t)ch>=' ' && x0r>=0.0f) && y0r>=0.0f) {
      cn = (uint32_t)(uint8_t)ch-32UL;
      if (cn>96UL) cn = 0UL;
      dimmlev = 20480.0f;
      if (contrast==1UL) {
         dimmlev = (float)((col0.r*87UL+col0.g*140UL+col0.b*28UL)*bri)
                *0.002f;
      }
      x0 = (int32_t)aprsdecode_trunc(x0r);
      y00 = (int32_t)aprsdecode_trunc(y0r);
      fx = aprsdecode_trunc((x0r-(float)x0)*256.0f);
      fy = aprsdecode_trunc((y0r-(float)y00)*256.0f);
      fine = (int32_t)(uint32_t)(fx || fy);
      { /* with */
         struct _1 * anonym = &font[cn];
         if (contrast>0UL && !dryrun) {
            tmp = (int32_t)(aprsdecode_lums.fontysize-1UL);
            y = 0L;
            if (y<=tmp) for (;; y++) {
               for (x = 0L; x<=15L; x++) {
                  /*
                              IF hicontrast & (mask[y]<>SET16{}
                ) OR (x IN mask[y]) THEN dim(x0+x, y0+y) END;
                  */
                  if (X2C_IN(x,16,anonym->mask[y])) {
                     dim(dimmlev, img, x0+x+fine, y00+y);
                  }
               } /* end for */
               if (y==tmp) break;
            } /* end for */
         }
         tmp = (int32_t)(aprsdecode_lums.fontysize-3UL);
         y = 0L;
         if (y<=tmp) for (;; y++) {
            yy = y00+y+1L;
            for (x = 0L; x<=7L; x++) {
               xx = x0+x+1L;
               c = ((uint32_t)anonym->char0[y][x]*bri)/256UL;
               if ((((c>0UL && xx>=0L) && xx+1L<(int32_t)(img->Len1-1))
                && yy>=0L) && yy+1L<(int32_t)(img->Len0-1)) {
                  if (dryrun) findinfo(xx, yy);
                  else {
                     cf = (c*(256UL-fx)*(256UL-fy))/65536UL;
                     { /* with */
                        struct maptool_PIX * anonym0 = &img->Adr[(xx)
                *img->Len0+yy];
                        anonym0->r += (uint16_t)((cf*col0.r)/256UL);
                        anonym0->g += (uint16_t)((cf*col0.g)/256UL);
                        anonym0->b += (uint16_t)((cf*col0.b)/256UL);
                        if (((uint32_t)anonym0->g+col0.g&1)) {
                           --anonym0->g; /* even is invert enable */
                        }
                     }
                     if (fine) {
                        /* save cpu */
                        cf = (c*fx*(256UL-fy))/65536UL;
                        { /* with */
                           struct maptool_PIX * anonym1 = &img->Adr[(xx+1L)
                *img->Len0+yy];
                           anonym1->r += (uint16_t)((cf*col0.r)/256UL);
                           anonym1->g += (uint16_t)((cf*col0.g)/256UL);
                           anonym1->b += (uint16_t)((cf*col0.b)/256UL);
                        }
                        cf = (c*(256UL-fx)*fy)/65536UL;
                        { /* with */
                           struct maptool_PIX * anonym2 = &img->Adr[(xx)
                *img->Len0+(yy+1L)];
                           anonym2->r += (uint16_t)((cf*col0.r)/256UL);
                           anonym2->g += (uint16_t)((cf*col0.g)/256UL);
                           anonym2->b += (uint16_t)((cf*col0.b)/256UL);
                        }
                        cf = (c*fx*fy)/65536UL;
                        { /* with */
                           struct maptool_PIX * anonym3 = &img->Adr[(xx+1L)
                *img->Len0+(yy+1L)];
                           anonym3->r += (uint16_t)((cf*col0.r)/256UL);
                           anonym3->g += (uint16_t)((cf*col0.g)/256UL);
                           anonym3->b += (uint16_t)((cf*col0.b)/256UL);
                        }
                     }
                  }
               }
            } /* end for */
            if (y==tmp) break;
         } /* end for */
         *inc0 = (int32_t)(anonym->width+1U);
      }
   }
} /* end drawchar() */


extern uint32_t maptool_charwidth0(char ch)
{
   uint32_t cn;
   if ((uint8_t)ch>=' ') {
      cn = (uint32_t)(uint8_t)ch-32UL;
      if (cn>96UL) cn = 0UL;
      return (uint32_t)(font[cn].width+1U);
   }
   return 0UL;
} /* end charwidth() */

#define maptool_ALPHA0 220

#define maptool_WHITELEV 250


extern void maptool_drawsym(maptool_pIMAGE image, char tab, char sym,
                 char mirror, float x0r, float y0r,
                uint32_t bri)
{
   int32_t sx;
   int32_t xi;
   int32_t y00;
   int32_t x0;
   int32_t y;
   int32_t x;
   uint32_t b;
   uint32_t br0d;
   uint32_t br1d;
   uint32_t br0c;
   uint32_t br1c;
   uint32_t br0b;
   uint32_t br1b;
   uint32_t br0a;
   uint32_t br1a;
   struct aprsdecode_COLTYP col0;
   uint32_t fy;
   uint32_t fx;
   struct PIX8 * anonym;
   struct maptool_PIX * anonym0;
   /* save cpu */
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   struct maptool_PIX * anonym3;
   if (bri==0UL) return;
   sx = (int32_t)(uint8_t)sym-32L;
   if (((((sx<=0L || sx>=96L) || x0r<(-16.0f)) || y0r<(-16.0f))
                || x0r>(float)((image->Len1-1)+16UL)) || y0r>(float)
                ((image->Len0-1)+16UL)) return;
   if (tab!='/') sx += 96L;
   sx = sx*16L;
   if (bri>255UL) bri = 255UL;
   x0 = (int32_t)X2C_TRUNCI(x0r,X2C_min_longint,X2C_max_longint);
   y00 = (int32_t)X2C_TRUNCI(y0r,X2C_min_longint,X2C_max_longint);
   if (x0<0L) fx = 0UL;
   else fx = aprsdecode_trunc((x0r-(float)x0)*256.0f);
   if (y00<0L) fy = 0UL;
   else fy = aprsdecode_trunc((y0r-(float)y00)*256.0f);
   x0 -= 8L;
   y00 -= 8L;
   b = (bri*(256UL-fx)*(256UL-fy))/65536UL;
   br1a = 256UL-b;
   br0a = (b*256000UL)/65536UL;
   if (fx || fy) {
      /* save cpu */
      b = (bri*fx*(256UL-fy))/65536UL;
      br1b = 256UL-b;
      br0b = (b*256000UL)/65536UL;
      b = (bri*(256UL-fx)*fy)/65536UL;
      br1c = 256UL-b;
      br0c = (b*256000UL)/65536UL;
      b = (bri*fx*fy)/65536UL;
      br1d = 256UL-b;
      br0d = (b*256000UL)/65536UL;
   }
   for (y = 0L; y<=15L; y++) {
      for (x = 0L; x<=15L; x++) {
         if (((x0+x>=0L && y00+y>=0L) && x+x0+1L<(int32_t)(image->Len1-1))
                && y+y00+1L<(int32_t)(image->Len0-1)) {
            if (mirror) xi = (15L-x)+sx;
            else xi = x+sx;
            { /* with */
               struct PIX8 * anonym = &symbols[xi][y];
               if ((anonym->r8<220U || anonym->g8<220U) || anonym->b8<220U) {
                  if (aprsdecode_click.dryrun) findinfo(x+x0, y+y00);
                  else {
                     { /* with */
                        struct maptool_PIX * anonym0 = &image->Adr[(x+x0)
                *image->Len0+(y+y00)];
                        anonym0->r = (uint16_t)(((uint32_t)
                anonym0->r*br1a+(uint32_t)anonym->r8*br0a)/256UL);
                        anonym0->g = (uint16_t)(((uint32_t)
                anonym0->g*br1a+(uint32_t)anonym->g8*br0a)/256UL);
                        anonym0->b = (uint16_t)(((uint32_t)
                anonym0->b*br1a+(uint32_t)anonym->b8*br0a)/256UL);
                     }
                     if (fx || fy) {
                        { /* with */
                           struct maptool_PIX * anonym1 = &image->Adr[(x+x0+1L)
                *image->Len0+(y+y00)];
                           anonym1->r = (uint16_t)(((uint32_t)
                anonym1->r*br1b+(uint32_t)anonym->r8*br0b)/256UL);
                           anonym1->g = (uint16_t)(((uint32_t)
                anonym1->g*br1b+(uint32_t)anonym->g8*br0b)/256UL);
                           anonym1->b = (uint16_t)(((uint32_t)
                anonym1->b*br1b+(uint32_t)anonym->b8*br0b)/256UL);
                        }
                        { /* with */
                           struct maptool_PIX * anonym2 = &image->Adr[(x+x0)
                *image->Len0+(y+y00+1L)];
                           anonym2->r = (uint16_t)(((uint32_t)
                anonym2->r*br1c+(uint32_t)anonym->r8*br0c)/256UL);
                           anonym2->g = (uint16_t)(((uint32_t)
                anonym2->g*br1c+(uint32_t)anonym->g8*br0c)/256UL);
                           anonym2->b = (uint16_t)(((uint32_t)
                anonym2->b*br1c+(uint32_t)anonym->b8*br0c)/256UL);
                        }
                        { /* with */
                           struct maptool_PIX * anonym3 = &image->Adr[(x+x0+1L)
                *image->Len0+(y+y00+1L)];
                           anonym3->r = (uint16_t)(((uint32_t)
                anonym3->r*br1d+(uint32_t)anonym->r8*br0d)/256UL);
                           anonym3->g = (uint16_t)(((uint32_t)
                anonym3->g*br1d+(uint32_t)anonym->g8*br0d)/256UL);
                           anonym3->b = (uint16_t)(((uint32_t)
                anonym3->b*br1d+(uint32_t)anonym->b8*br0d)/256UL);
                        }
                     }
                  }
               }
            }
         }
      } /* end for */
   } /* end for */
   if (((uint8_t)tab>='0' && (uint8_t)tab<='9' || (uint8_t)
                tab>='A' && (uint8_t)tab<='Z') || (uint8_t)
                tab>='a' && (uint8_t)tab<='z') {
      maptool_Colset(&col0, 'W');
      maptool_drawchar(image, tab, x0r-4.0f,
                y0r-((8.0f-(16.0f-(float)aprsdecode_lums.fontysize)*0.5f)
                +0.5f), &sx, bri*2UL, 1UL, col0, aprsdecode_click.dryrun);
   }
} /* end drawsym() */

#define maptool_FILLLUM 30


static uint32_t lim(float x, uint32_t m)
{
   uint32_t r;
   if (x<=0.0f) return 0UL;
   r = aprsdecode_trunc(x);
   if (r>m) return m;
   return r;
} /* end lim() */


static void arc(uint32_t b, uint32_t g, uint32_t r,
                maptool_pIMAGE image, char fill, float xh,
                float * y1, float yh, float * y00, uint32_t yi0,
                float * x0, float * x1, uint32_t i, char oct,
                float * xhh)
{
   uint32_t lum;
   if (*xhh>(-1.0f)) {
      if (oct) {
         *xhh = 1.0f-sqr(X2C_DIVR((float)i-*x1,*x1-*x0));
         if (*xhh<=0.0f) *xhh = 0.0f;
         *xhh = (float)fabs(((float)yi0-*y00)-yh)-(*y1-*y00)
                *0.5f*osic_sqrt(*xhh);
      }
      else *xhh = (float)fabs(*x1-(float)i)-xh;
      *xhh = (1.5f-(float)fabs(*xhh+1.5f))*1.7066666666667E+2f;
   }
   if (*xhh>0.0f) {
      if (aprsdecode_click.dryrun) findinfo((int32_t)i, (int32_t)yi0);
      lum = aprsdecode_trunc(*xhh);
   }
   else if (fill) lum = 30UL;
   else lum = 0UL;
   addcol(&image->Adr[(i)*image->Len0+yi0], (int32_t)r, (int32_t)g,
                (int32_t)b, (int32_t)lum);
} /* end arc() */


extern void maptool_drawareasym(maptool_pIMAGE image,
                struct aprspos_POSITION pm, struct aprsdecode_AREASYMB area,
                uint32_t bri)
{
   int32_t ret;
   struct aprspos_POSITION p1;
   struct aprspos_POSITION p0;
   float xho;
   float yh;
   float xhh;
   float xh;
   float y1;
   float x1;
   float y00;
   float x0;
   uint32_t xm;
   uint32_t i;
   uint32_t yi1;
   uint32_t xi1;
   uint32_t yi0;
   uint32_t xi0;
   char oct;
   char fill;
   uint32_t b;
   uint32_t g;
   uint32_t r;
   uint32_t tmp;
   switch ((unsigned)(area.color&7U)) {
   case 0U: /* we have no black so make gray */
      r = 128UL;
      g = 128UL;
      b = 128UL;
      break;
   case 1U:
      r = 0UL;
      g = 0UL;
      b = 255UL;
      break;
   case 2U:
      r = 0UL;
      g = 255UL;
      b = 0UL;
      break;
   case 3U:
      r = 0UL;
      g = 255UL;
      b = 255UL;
      break;
   case 4U:
      r = 255UL;
      g = 0UL;
      b = 0UL;
      break;
   case 5U:
      r = 255UL;
      g = 0UL;
      b = 255UL;
      break;
   case 6U:
      r = 255UL;
      g = 255UL;
      b = 0UL;
      break;
   case 7U:
      r = 255UL;
      g = 255UL;
      b = 255UL;
      break;
   } /* end switch */
   if (area.color>=8U) {
      r = r/2UL;
      g = g/2UL;
      b = b/2UL;
   }
   r = (r*bri)/64UL;
   g = (g*bri)/64UL;
   b = (b*bri)/64UL;
   if (area.typ=='1' || area.typ=='6') {
      /* line */
      p0.lat = pm.lat+area.dpos.lat;
      p0.long0 = pm.long0-area.dpos.long0;
      p1.lat = pm.lat-area.dpos.lat;
      p1.long0 = pm.long0+area.dpos.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      ret = maptool_mapxy(p1, &x1, &y1);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 300UL, 0.0f);
   }
   else if (area.typ=='4' || area.typ=='9') {
      /* box */
      p0.lat = pm.lat+area.dpos.lat;
      p0.long0 = pm.long0-area.dpos.long0;
      p1.lat = p0.lat;
      p1.long0 = pm.long0+area.dpos.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      ret = maptool_mapxy(p1, &x1, &y1);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      p0.lat = pm.lat-area.dpos.lat;
      p0.long0 = pm.long0+area.dpos.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      p1.lat = pm.lat-area.dpos.lat;
      p1.long0 = pm.long0-area.dpos.long0;
      ret = maptool_mapxy(p1, &x1, &y1);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      p0.lat = pm.lat+area.dpos.lat;
      p0.long0 = pm.long0-area.dpos.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      if (area.typ=='9') {
         /* fill */
         p0.lat = pm.lat-area.dpos.lat;
         p0.long0 = pm.long0-area.dpos.long0;
         p1.lat = pm.lat+area.dpos.lat;
         p1.long0 = pm.long0+area.dpos.long0;
         ret = maptool_mapxy(p0, &x0, &y00);
         ret = maptool_mapxy(p1, &x1, &y1);
         xi0 = lim(x0, image->Len1-1);
         xi1 = lim(x1, image->Len1-1);
         yi0 = lim(y00, image->Len0-1);
         yi1 = lim(y1, image->Len0-1);
         while (yi0<yi1) {
            tmp = xi1;
            i = xi0;
            if (i<=tmp) for (;; i++) {
               addcol(&image->Adr[(i)*image->Len0+yi0], (int32_t)r,
                (int32_t)g, (int32_t)b, 30L);
               if (i==tmp) break;
            } /* end for */
            ++yi0;
         }
      }
   }
   else if (area.typ=='3' || area.typ=='8') {
      /* triangle */
      p0.lat = pm.lat+area.dpos.lat;
      p0.long0 = pm.long0;
      p1.lat = pm.lat-area.dpos.lat;
      p1.long0 = pm.long0-area.dpos.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      ret = maptool_mapxy(p1, &x1, &y1);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      p1.long0 = pm.long0+area.dpos.long0;
      ret = maptool_mapxy(p1, &x1, &y1);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      p0.lat = pm.lat-area.dpos.lat;
      p0.long0 = pm.long0-area.dpos.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      maptool_vector(image, x0, y00, x1, y1, (int32_t)r, (int32_t)g,
                (int32_t)b, 200UL, 0.0f);
      if (area.typ=='8') {
         /* fill */
         p0.lat = pm.lat-area.dpos.lat;
         p0.long0 = pm.long0-area.dpos.long0;
         p1.lat = pm.lat+area.dpos.lat;
         p1.long0 = pm.long0;
         ret = maptool_mapxy(p0, &x0, &y00);
         ret = maptool_mapxy(p1, &x1, &y1);
         xhh = y1-y00;
         if (xhh!=0.0f) {
            yi0 = lim(y00, image->Len0-1);
            yi1 = lim(y1, image->Len0-1);
            xhh = X2C_DIVR(x1-x0,xhh);
            while (yi0<yi1) {
               xh = xhh*(y1-(float)yi0);
               tmp = lim(x1+xh, image->Len1-1);
               i = lim(x1-xh, image->Len1-1);
               if (i<=tmp) for (;; i++) {
                  addcol(&image->Adr[(i)*image->Len0+yi0], (int32_t)r,
                (int32_t)g, (int32_t)b, 30L);
                  if (i==tmp) break;
               } /* end for */
               ++yi0;
            }
         }
      }
   }
   else if (((area.typ=='0' || area.typ=='2') || area.typ=='5')
                || area.typ=='7') {
      /* circle ellipse */
      p0.lat = pm.lat-area.dpos.lat;
      p0.long0 = pm.long0-area.dpos.long0;
      p1.lat = pm.lat+area.dpos.lat;
      p1.long0 = pm.long0;
      ret = maptool_mapxy(p0, &x0, &y00);
      ret = maptool_mapxy(p1, &x1, &y1);
      if (area.typ=='0' || area.typ=='5') x0 = x1-(y1-y00)*0.5f;
      yi0 = lim(y00+0.5f, image->Len0-1);
      yi1 = lim(y1+0.5f, image->Len0-1);
      yh = (y1-y00)*0.5f;
      xho = 0.0f;
      fill = area.typ=='5' || area.typ=='7';
      if (yh!=0.0f && x0!=x1) {
         while (yi0<yi1) {
            xh = 1.0f-sqr(X2C_DIVR((y00+yh)-(float)yi0,yh));
            if (xh<=0.0f) xh = 0.0f;
            xh = (x1-x0)*osic_sqrt(xh);
            oct = (float)fabs(xh-xho)>=1.0f;
            xho = xh;
            xm = lim(x1+0.5f, image->Len1-1);
            xhh = 0.0f;
            tmp = xm;
            i = lim((x1-xh)+0.5f, image->Len1-1)+1UL;
            if (i<=tmp) for (;; i++) {
               arc(b, g, r, image, fill, xh, &y1, yh, &y00, yi0, &x0, &x1, i,
                 oct, &xhh);
               if (i==tmp) break;
            } /* end for */
            xhh = 0.0f;
            tmp = xm+1UL;
            i = lim(x1+xh+0.5f, image->Len1-1)-1UL;
            if (i>=tmp) for (;; i--) {
               arc(b, g, r, image, fill, xh, &y1, yh, &y00, yi0, &x0, &x1, i,
                 oct, &xhh);
               if (i==tmp) break;
            } /* end for */
            ++yi0;
         }
      }
   }
} /* end drawareasym() */


static void dashvec(maptool_pIMAGE image, float x0, float y00,
                float x1, float y1, uint32_t r, uint32_t g,
                uint32_t b, float double0, uint32_t len,
                uint32_t wid)
{
   float dy;
   float dx;
   float l;
   float m1;
   float m0;
   float k;
   float y;
   float x;
   uint32_t d;
   uint32_t i;
   uint32_t tmp;
   x = x1-x0;
   y = y1-y00;
   k = x*x+y*y;
   if (k==0.0f) return;
   k = osic_sqrt(k);
   dx = X2C_DIVR(double0*y,k);
   dy = -(X2C_DIVR(double0*x,k)); /* rotate vector 90 deg for double dash */
   l = (float)len;
   if (k<100.0f) l = l*0.8f;
   else if (k<50.0f) l = l*0.5f;
   d = 1UL+(aprsdecode_trunc(X2C_DIVR(k,l))/2UL)*2UL;
   k = X2C_DIVR(1.0f,(float)d);
   tmp = d-1UL;
   i = 0UL;
   if (i<=tmp) for (tmp = (uint32_t)(tmp-i)/2UL;;) {
      m0 = k*(float)i;
      m1 = k*(float)(i+1UL);
      maptool_vector(image, x0+x*m0, y00+y*m0, x0+x*m1, y00+y*m1,
                (int32_t)r, (int32_t)g, (int32_t)b, wid, 0.0f);
      if (double0!=0.0f) {
         maptool_vector(image, x0+x*m0+dx, y00+y*m0+dy, x0+x*m1+dx,
                y00+y*m1+dy, (int32_t)r, (int32_t)g, (int32_t)b, wid,
                0.0f);
      }
      if (!tmp) break;
      --tmp;
      i += 2UL;
   } /* end for */
} /* end dashvec() */


static void policolor(uint32_t c, uint32_t bri, uint32_t * r,
                uint32_t * g, uint32_t * b)
/* colour for poliline object */
{
   c = c&3UL;
   *r = 0UL;
   *g = 0UL;
   *b = 0UL;
   switch (c) {
   case 0UL:
      *r = 255UL;
      break;
   case 1UL:
      *r = 170UL;
      *g = 180UL;
      break;
   case 2UL:
      *g = 50UL;
      *b = 400UL;
      break;
   case 3UL:
      *g = 230UL;
      break;
   } /* end switch */
   *r = ( *r*bri)/64UL;
   *g = ( *g*bri)/64UL;
   *b = ( *b*bri)/64UL;
} /* end policolor() */

struct _2;


struct _2 {
   int32_t xi;
   int32_t yi;
};


static void fillpoligon(maptool_pIMAGE image, struct aprspos_POSITION pm,
                const struct aprsdecode_MULTILINE md, uint32_t bri)
{
   uint32_t hachuresize;
   uint32_t nc;
   uint32_t nv;
   uint32_t j;
   uint32_t i;
   uint32_t bf;
   uint32_t gf;
   uint32_t rf;
   int32_t miny;
   int32_t maxy;
   int32_t minx;
   int32_t maxx;
   int32_t x;
   int32_t ret;
   struct _2 vert[41];
   int32_t cross[41];
   struct aprspos_POSITION p;
   float yr;
   float xr;
   char done;
   struct _2 * anonym;
   struct maptool_PIX * anonym0;
   int32_t tmp;
   if ((uint8_t)md.filltyp<'2' || (uint8_t)md.filltyp>'9') return;
   i = (uint32_t)(uint8_t)md.filltyp-50UL;
   if (i<4UL) {
      hachuresize = 0UL;
      bri = bri/6UL;
   }
   else {
      hachuresize = 3UL;
      bri = bri/2UL;
   }
   policolor(i, bri, &rf, &gf, &bf);
   maxx = X2C_min_longint;
   maxy = X2C_min_longint;
   minx = X2C_max_longint;
   miny = X2C_max_longint;
   nv = md.size+1UL; /* always a poligon */
   if (nv<4UL || nv>40UL) return;
   i = 0UL;
   while (i<nv) {
      j = i%md.size;
      p.lat = pm.lat+md.vec[j].lat;
      p.long0 = pm.long0+md.vec[j].long0;
      { /* with */
         struct _2 * anonym = &vert[i];
         ret = maptool_mapxy(p, &xr, &yr);
         anonym->xi = (int32_t)X2C_TRUNCI(xr,X2C_min_longint,
                X2C_max_longint);
         anonym->yi = (int32_t)X2C_TRUNCI(yr,X2C_min_longint,
                X2C_max_longint);
         if (anonym->xi>maxx) maxx = anonym->xi;
         if (anonym->xi<minx) minx = anonym->xi;
         if (anonym->yi>maxy) maxy = anonym->yi;
         if (anonym->yi<miny) miny = anonym->yi;
      }
      ++i;
   }
   if (miny<0L) miny = 0L;
   if (maxy>(int32_t)(image->Len0-1)) maxy = (int32_t)(image->Len0-1);
   if (minx<0L) minx = 0L;
   if (maxx>(int32_t)(image->Len1-1)) maxx = (int32_t)(image->Len1-1);
   if (hachuresize>0UL) {
      /* modify hachure with image size */
      x = ((maxx-minx)+maxy)-miny;
      if (x>0L) {
         hachuresize += aprsdecode_trunc(0.35f*osic_sqrt((float)x));
      }
   }
   while (maxy>miny) {
      nc = 0UL;
      i = 0UL;
      while (i<nv-1UL) {
         /* find all crossings to scanline */
         if (vert[i].yi>maxy!=vert[i+1UL].yi>maxy) {
            /* vector crosses scanline */
            x = vert[i].xi+(int32_t)X2C_TRUNCI(X2C_DIVR((float)
                (vert[i+1UL].xi-vert[i].xi)*(float)(vert[i].yi-maxy),
                (float)(vert[i].yi-vert[i+1UL].yi)),X2C_min_longint,
                X2C_max_longint);
            if (x<minx) x = minx;
            else if (x>maxx) x = maxx;
            cross[nc] = x;
            ++nc;
         }
         ++i;
      }
      if (nc>=2UL && !(nc&1)) {
         /* should always be even */
         do {
            /* sort crossings from left to right */
            i = 0UL;
            done = 1;
            while (i<nc-1UL) {
               if (cross[i]>cross[i+1UL]) {
                  x = cross[i];
                  cross[i] = cross[i+1UL];
                  cross[i+1UL] = x;
                  done = 0;
               }
               ++i;
            }
         } while (!done);
         /*      done:=TRUE; */
         j = 1UL;
         tmp = cross[nc-1UL];
         x = cross[0U];
         if (x<=tmp) for (;; x++) {
            while (x==cross[j]) {
               done = !done;
               ++j;
            }
            if (done && (hachuresize==0UL || (uint32_t)(maxy+x)
                %hachuresize==0UL)) {
               { /* with */
                  struct maptool_PIX * anonym0 = &image->Adr[(x)
                *image->Len0+maxy];
                  anonym0->r += (uint16_t)rf;
                  anonym0->g += (uint16_t)gf;
                  anonym0->b += (uint16_t)bf;
               }
            }
            if (x==tmp) break;
         } /* end for */
      }
      --maxy;
   }
} /* end fillpoligon() */


extern void maptool_drawpoligon(maptool_pIMAGE image,
                struct aprspos_POSITION pm, struct aprsdecode_MULTILINE md,
                char tab, char sym, uint32_t bri)
{
   int32_t ret;
   struct aprspos_POSITION p;
   float y1;
   float x1;
   float y00;
   float x0;
   uint32_t j;
   uint32_t i;
   uint32_t sz;
   uint32_t widt;
   uint32_t col0;
   uint32_t b;
   uint32_t g;
   uint32_t r;
   if ((uint8_t)md.linetyp<'a' || md.size<=1UL) return;
   col0 = (uint32_t)(uint8_t)md.linetyp-97UL;
   policolor(col0/3UL&3UL, bri, &r, &g, &b);
   i = 0UL;
   widt = 300UL;
   if ((uint8_t)md.filltyp>'1') widt = 160UL;
   sz = md.size+(uint32_t)(md.filltyp!='1'); /* closed poligon */
   while (i<sz) {
      j = i%md.size;
      p.lat = pm.lat+md.vec[j].lat;
      p.long0 = pm.long0+md.vec[j].long0;
      ret = maptool_mapxy(p, &x1, &y1);
      if (i>0UL) {
         switch (col0%3UL) {
         case 0UL:
            maptool_vector(image, x0, y00, x1, y1, (int32_t)r,
                (int32_t)g, (int32_t)b, widt, 0.0f);
            break;
         case 1UL:
            dashvec(image, x0, y00, x1, y1, r, g, b, 0.0f, 8UL, widt);
            break;
         case 2UL:
            dashvec(image, x0, y00, x1, y1, r, g, b, 2.5f, 6UL, 200UL);
            break;
         } /* end switch */
      }
      x0 = x1;
      y00 = y1;
      ++i;
   }
   if (i>0UL && md.filltyp=='1') {
      maptool_drawsym(image, tab, sym, 0, x0, y00, bri);
   }
   fillpoligon(image, pm, md, bri);
} /* end drawpoligon() */


extern void maptool_drawpoliobj(maptool_pIMAGE image)
{
   char cs[251];
   struct aprsdecode_MULTILINE ml;
   struct aprspos_POSITION center;
   uint32_t i;
   useri_confstr(useri_fRBPOS, cs, 251ul);
   aprstext_deganytopos(cs, 251ul, &center);
   if (!aprspos_posvalid(center)) return;
   useri_confstr(useri_fRBCOMMENT, cs, 251ul);
   aprsdecode_GetMultiline(cs, 251ul, &i, &ml);
   useri_confstr(useri_fRBSYMB, cs, 251ul);
   maptool_drawpoligon(image, center, ml, cs[0U], cs[1U], 250UL);
} /* end drawpoliobj() */


extern char maptool_findmultiline(struct aprspos_POSITION pos,
                struct aprspos_POSITION * foundpos)
{
   char cs[251];
   struct aprsdecode_MULTILINE ml;
   struct aprspos_POSITION center;
   uint32_t mini;
   uint32_t i;
   float d;
   float mind;
   useri_confstr(useri_fRBPOS, cs, 251ul);
   aprstext_deganytopos(cs, 251ul, &center);
   if (!aprspos_posvalid(center)) return 0;
   useri_confstr(useri_fRBCOMMENT, cs, 251ul);
   aprsdecode_GetMultiline(cs, 251ul, &i, &ml);
   mind = X2C_max_real;
   mini = ml.size;
   i = 0UL;
   while (i<ml.size) {
      d = (float)fabs((ml.vec[i].lat+center.lat)-pos.lat)+(float)
                fabs((ml.vec[i].long0+center.long0)-pos.long0);
      if (d<mind) {
         mind = d;
         mini = i;
      }
      ++i;
   }
   if (mini<ml.size) {
      foundpos->lat = ml.vec[mini].lat+center.lat;
      foundpos->long0 = ml.vec[mini].long0+center.long0;
      aprsdecode_click.polilinecursor = mini;
      return 1;
   }
   return 0;
} /* end findmultiline() */

#define maptool_HY 18

#define maptool_MARGIN 2


static void OptTextPlace(maptool_pIMAGE img, char s[], uint32_t s_len,
                int32_t * xt, int32_t * yt)
{
   uint32_t cn;
   uint32_t i;
   /*max,*/
   int32_t fonty;
   int32_t n;
   int32_t ymin;
   int32_t min0;
   int32_t lb;
   int32_t lg;
   int32_t lr;
   int32_t cont;
   int32_t yh;
   int32_t y;
   int32_t x;
   int32_t wid;
   int32_t ct[54];
   struct maptool_PIX * anonym;
   int32_t tmp;
   int32_t tmp0;
   X2C_PCOPY((void **)&s,s_len);
   fonty = (int32_t)(aprsdecode_lums.fontysize-3UL);
   if (((*xt<0L || *xt>=(int32_t)(img->Len1-1)) || *yt<-fonty)
                || *yt>=(int32_t)(img->Len0-1)+fonty) goto label;
   i = 0UL;
   wid = 0L;
   while (i<=s_len-1 && s[i]) {
      if ((uint8_t)s[i]>=' ') {
         cn = (uint32_t)(uint8_t)s[i]-32UL;
         if (cn>96UL) cn = 0UL;
         wid += (int32_t)font[cn].width;
      }
      ++i;
   }
   min0 = X2C_max_longint;
   n = 0L;
   tmp = 3L*fonty-1L;
   y = 0L;
   if (y<=tmp) for (;; y++) {
      yh = (*yt+y)-fonty;
      if (yh>=2L && yh<(int32_t)((img->Len0-1)+1UL)-2L) {
         /* never place text outside drawable */
         cont = 0L;
         lr = 0L;
         lg = 0L;
         lb = 0L;
         tmp0 = (*xt+wid)-1L;
         x = *xt;
         if (x<=tmp0) for (;; x++) {
            if (x<(int32_t)((img->Len1-1)-1UL)) {
               { /* with */
                  struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+yh];
                  cont += labs((int32_t)anonym->r-lr)+labs((int32_t)
                anonym->g-lg)+labs((int32_t)anonym->b-lb);
                  lr = (int32_t)anonym->r;
                  lg = (int32_t)anonym->g;
                  lb = (int32_t)anonym->b;
               }
            }
            if (x==tmp0) break;
         } /* end for */
      }
      else cont = wid*32768L;
      if (y<fonty || y>=fonty*2L) cont = (cont*400L)/256L;
      ct[y] = cont;
      n += cont;
      if (y>=fonty) {
         n -= ct[y-fonty];
         if (n<min0) {
            min0 = n;
            ymin = y;
         }
      }
      if (y==tmp) break;
   } /* end for */
   *yt += ymin-fonty*2L;
   label:;
   X2C_PFREE(s);
} /* end OptTextPlace() */


extern void maptool_drawstr(maptool_pIMAGE image, char s[],
                uint32_t s_len, float xr, float yr, uint32_t bri,
                uint32_t contrast, struct aprsdecode_COLTYP col0,
                signed char * pos, uint32_t slide, char fixpos,
                char dryrun)
{
   uint32_t i;
   int32_t y;
   int32_t x;
   int32_t inc0;
   int32_t dy;
   if (bri==0UL) return;
   if (slide>0UL) {
      if (fixpos) yr = yr+(float)*pos;
      else {
         y = (int32_t)X2C_TRUNCI(yr,X2C_min_longint,X2C_max_longint);
         x = (int32_t)X2C_TRUNCI(xr,X2C_min_longint,X2C_max_longint);
         dy = y;
         OptTextPlace(image, s, s_len, &x, &y);
         y = (y/2L)*2L+1L; /* not odd lines for better 420 color conversion */
         *pos = (signed char)(y-dy);
         xr = (float)x;
         yr = (float)y;
      }
   }
   i = 0UL;
   while (i<=s_len-1 && s[i]) {
      maptool_drawchar(image, s[i], xr, yr, &inc0, bri, contrast, col0,
                dryrun);
      xr = xr+(float)inc0;
      /*    xr:=xr + CHARWIDTH; */
      ++i;
   }
} /* end drawstr() */


extern void maptool_drawstri(maptool_pIMAGE image, char s[],
                uint32_t s_len, int32_t xr, int32_t yr, uint32_t bri,
                 uint32_t contrast, struct aprsdecode_COLTYP col0,
                char proportional, char dryrun)
{
   uint32_t i;
   int32_t inc0;
   char c;
   i = 0UL;
   for (;;) {
      if (i>s_len-1) break;
      c = s[i];
      if (c==0) break;
      if ((uint8_t)c>=(uint8_t)'\360' && (uint8_t)c<=(uint8_t)'\370')
                 xr += (int32_t)((uint32_t)(uint8_t)c-239UL);
      else {
         maptool_drawchar(image, s[i], (float)xr, (float)yr, &inc0,
                bri, contrast, col0, dryrun);
         if (proportional) xr += inc0;
         else xr += 6L;
      }
      ++i;
   }
} /* end drawstri() */

#define maptool_L 7

#define maptool_WS 20
/* wind fethers per kmh */

#define maptool_MAXWIND 250
/* limit to nonsense windspeed */

#define maptool_LW 8

#define maptool_WW 4

#define maptool_A 0.12

#define maptool_W (-1.25)


extern void maptool_drawarrow(maptool_pIMAGE image, float x0,
                float y00, float len, float ang, uint32_t wind,
                uint32_t bri, struct aprsdecode_COLTYP col0)
{
   int32_t wi;
   int32_t b;
   int32_t g;
   int32_t r;
   float c1;
   float s1;
   float l;
   float c;
   float s;
   float y1;
   float x1;
   /*WrInt(wind, 10);WrStrLn("=w"); */
   s = osic_sin(ang);
   c = osic_cos(ang);
   r = (int32_t)((bri*col0.r)/256UL);
   g = (int32_t)((bri*col0.g)/256UL);
   b = (int32_t)((bri*col0.b)/256UL);
   if (wind==0UL) {
      maptool_vector(image, x0, y00, x0-(len-5.0f)*s, y00+(len-5.0f)*c, r, g,
                 b, 400UL, 0.0f);
      x1 = x0-len*s;
      y1 = y00+len*c;
      l = len-7.0f;
      maptool_vector(image, x1, y1, x0-l*osic_sin(ang+0.12f),
                y00+l*osic_cos(ang+0.12f), r, g, b, 200UL, 0.0f);
      maptool_vector(image, x1, y1, x0-l*osic_sin(ang-0.12f),
                y00+l*osic_cos(ang-0.12f), r, g, b, 200UL, 0.0f);
   }
   else {
      len = len+(float)((4UL*wind)/20UL);
      maptool_vector(image, x0, y00, x0-len*s, y00+len*c, r, g, b, 250UL,
                0.0f);
      s1 = osic_sin(ang+(-1.25f));
      c1 = osic_cos(ang+(-1.25f));
      wi = (int32_t)wind;
      if (wi<20L) wi = 20L;
      else if (wi>250L) wi = 250L;
      do {
         x1 = x0-len*s; /*-s1*/
         y1 = y00+len*c; /*+c1*/
         l = 8.0f;
         if (wi<20L) l = X2C_DIVR(8.0f*(float)wi,20.0f);
         maptool_vector(image, x1, y1, x1-l*s1, y1+l*c1, r, g, b, 250UL,
                0.0f);
         wi -= 20L;
         len = len-4.0f;
      } while (wi>0L);
   }
} /* end drawarrow() */


extern void maptool_cc(maptool_pIMAGE img, uint32_t from, uint32_t to)
{
   struct aprsdecode_COLTYP col0;
   char h[1024];
   char s[1024];
   signed char pos;
   aprsdecode_click.ops = 0;
   if (aprsdecode_lums.map>0L) {
      strncpy(s,"Maps:(c)www.openstreetmap.org",1024u);
   }
   else s[0] = 0;
   if (img->Len1-1>=399UL || aprsdecode_lums.map==0L) {
      /*    Append(s, " Time:"); */
      aprsstr_Append(s, 1024ul, " ", 2ul);
      aprstext_DateLocToStr(from, h, 1024ul);
      aprsstr_Append(s, 1024ul, h, 1024ul);
      if (to) {
         aprsstr_Append(s, 1024ul, "-", 2ul);
         aprsstr_TimeToStr((to+useri_localtime())%86400UL, h, 1024ul);
         aprsstr_Append(s, 1024ul, h, 1024ul);
      }
   }
   maptool_Colset(&col0, 'G');
   aprsstr_Append(s, 1024ul, " Zoom:", 7ul);
   aprsstr_FixToStr(((float)aprsdecode_initzoom-0.995f)
                +aprsdecode_finezoom, 3UL, h, 1024ul);
   aprsstr_Append(s, 1024ul, h, 1024ul);
   maptool_drawstr(img, s, 1024ul, 5.0f,
                (float)(((img->Len0-1)-aprsdecode_lums.fontysize)-30UL),
                300UL, 0UL, col0, &pos, 3UL, 0, aprsdecode_click.dryrun);
} /* end cc() */

#define maptool_RULERY 10

#define maptool_RULERX0 20

#define maptool_RULERX1 300


extern void maptool_ruler(maptool_pIMAGE img)
{
   struct aprspos_POSITION rpos;
   struct aprspos_POSITION lpos;
   char s[101];
   char h[101];
   float e;
   float d;
   float r;
   uint32_t w;
   uint32_t m;
   signed char pos;
   struct aprsdecode_COLTYP col0;
   if (300L>maptool_xsize || aprsdecode_initzoom<6L) return;
   maptool_xytodeg(20.0f, 10.0f, &lpos);
   maptool_xytodeg(300.0f, 10.0f, &rpos);
   d = 1000.0f*aprspos_distance(lpos, rpos);
   if (d==0.0f) return;
   r = d;
   m = 1UL;
   while (r>=10.0f) {
      r = r*0.1f;
      m = m*10UL;
   }
   e = r;
   if (r>=5.0f) r = 5.0f;
   else if (r>=2.0f) r = 2.0f;
   else r = 1.0f;
   e = (X2C_DIVR(r,e))*280.0f;
   maptool_vector(img, 20.0f, 10.0f, 20.0f+e, 10.0f, 0L, 120L, 100L, 200UL,
                0.0f);
   maptool_vector(img, 20.0f, 7.0f, 20.0f, 13.0f, 0L, 120L, 100L, 200UL,
                0.0f);
   maptool_vector(img, 20.0f+e, 7.0f, 20.0f+e, 13.0f, 0L, 120L, 100L, 200UL,
                0.0f);
   m = m*aprsdecode_trunc(r);
   w = m;
   if (w>=1000UL) w = w/1000UL;
   aprsstr_IntToStr((int32_t)w, 1UL, h, 101ul);
   if (m>=1000UL) aprsstr_Append(h, 101ul, "km", 3ul);
   else aprsstr_Append(h, 101ul, "m", 2ul);
   maptool_Colset(&col0, 'G');
   aprsstr_Append(h, 101ul, " [", 3ul);
   aprsstr_FixToStr(((float)aprsdecode_initzoom-0.95f)
                +aprsdecode_finezoom, 2UL, s, 101ul);
   aprsstr_Append(h, 101ul, s, 101ul);
   aprsstr_Append(h, 101ul, "]", 2ul);
   maptool_drawstr(img, h, 101ul, osic_floor((20.0f+e*0.5f)-21.0f), 10.0f,
                250UL, 0UL, col0, &pos, 0UL, 1, aprsdecode_click.dryrun);
} /* end ruler() */

#define maptool_VIS 80.0

#define maptool_PEAK 1000.0


static int32_t smoo(float ex, int32_t x)
{
   if (x<=0L) return x;
   else {
      return (int32_t)aprsdecode_trunc(osic_exp(osic_ln(X2C_DIVR((float)
                x,80.0f))*ex)*80.0f);
   }
   return 0;
} /* end smoo() */

/*BEGIN RETURN trunc(FLOAT(x)/FLOAT(max)*5000.0) END smoo; */

extern void maptool_shine(maptool_pIMAGE image, int32_t lum)
{
   int32_t max0;
   int32_t y;
   int32_t x;
   uint32_t bb;
   uint32_t gg;
   uint32_t rr;
   uint32_t c;
   uint32_t f;
   char done;
   float ex;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   struct maptool_PIX * anonym3;
   struct maptool_PIX * anonym4;
   struct maptool_PIX * anonym5;
   struct maptool_PIX * anonym6;
   struct maptool_PIX * anonym7;
   struct maptool_PIX * anonym8;
   struct maptool_PIX * anonym9;
   struct maptool_PIX * anonym10;
   struct maptool_PIX * anonym11;
   struct maptool_PIX * anonym12;
   struct maptool_PIX * anonym13;
   struct maptool_PIX * anonym14;
   int32_t tmp;
   int32_t tmp0;
   max0 = (int32_t)(aprsdecode_trunc(80.0f)+1UL);
   tmp = (int32_t)(image->Len0-1);
   y = 0L;
   if (y<=tmp) for (;; y++) {
      tmp0 = (int32_t)(image->Len1-1);
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &image->Adr[(x)*image->Len0+y];
            if ((int32_t)anonym->g>max0) max0 = (int32_t)anonym->g;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   ex = X2C_DIVR(osic_ln(12.5f),osic_ln(X2C_DIVR((float)max0,80.0f)));
   if (ex>1.5f) ex = 1.5f;
   /*WrInt(max, 10);WrFixed(ex, 3,10);WrLn; */
   tmp = (int32_t)(image->Len0-1);
   y = 0L;
   if (y<=tmp) for (;; y++) {
      tmp0 = (int32_t)(image->Len1-1);
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym0 = &image->Adr[(x)*image->Len0+y];
            anonym0->r = (uint16_t)smoo(ex, (int32_t)anonym0->r);
            anonym0->g = (uint16_t)smoo(ex, (int32_t)anonym0->g);
            anonym0->b = (uint16_t)smoo(ex, (int32_t)anonym0->b);
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   do {
      done = 1;
      tmp = (int32_t)((image->Len0-1)-2UL);
      y = 2L;
      if (y<=tmp) for (;; y++) {
         tmp0 = (int32_t)((image->Len1-1)-2UL);
         x = 2L;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct maptool_PIX * anonym1 = &image->Adr[(x)*image->Len0+y];
                
               c = (uint32_t)(anonym1->r/2U);
               if ((uint32_t)anonym1->g>c) c = (uint32_t)anonym1->g;
               if ((uint32_t)anonym1->b>c) c = (uint32_t)anonym1->b;
               if (c>=256UL) {
                  rr = 0UL;
                  gg = 0UL;
                  bb = 0UL;
                  f = 65535UL/c;
                  c = ((uint32_t)anonym1->r*f)/256UL;
                  if ((uint32_t)anonym1->r>c) {
                     rr = ((uint32_t)anonym1->r-c)/10UL;
                  }
                  anonym1->r = (uint16_t)c;
                  c = ((uint32_t)anonym1->g*f)/256UL;
                  if ((uint32_t)anonym1->g>c) {
                     gg = ((uint32_t)anonym1->g-c)/12UL;
                  }
                  anonym1->g = (uint16_t)c;
                  c = ((uint32_t)anonym1->b*f)/256UL;
                  if ((uint32_t)anonym1->b>c) {
                     bb = ((uint32_t)anonym1->b-c)/12UL;
                  }
                  anonym1->b = (uint16_t)c;
                  { /* with */
                     struct maptool_PIX * anonym2 = &image->Adr[(x-1L)
                *image->Len0+(y-1L)];
                     anonym2->r += (uint16_t)rr;
                     anonym2->g += (uint16_t)gg;
                     anonym2->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym3 = &image->Adr[(x-1L)
                *image->Len0+y];
                     anonym3->r += (uint16_t)rr;
                     anonym3->g += (uint16_t)gg;
                     anonym3->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym4 = &image->Adr[(x-1L)
                *image->Len0+(y+1L)];
                     anonym4->r += (uint16_t)rr;
                     anonym4->g += (uint16_t)gg;
                     anonym4->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym5 = &image->Adr[(x+1L)
                *image->Len0+(y-1L)];
                     anonym5->r += (uint16_t)rr;
                     anonym5->g += (uint16_t)gg;
                     anonym5->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym6 = &image->Adr[(x+1L)
                *image->Len0+y];
                     anonym6->r += (uint16_t)rr;
                     anonym6->g += (uint16_t)gg;
                     anonym6->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym7 = &image->Adr[(x+1L)
                *image->Len0+(y+1L)];
                     anonym7->r += (uint16_t)rr;
                     anonym7->g += (uint16_t)gg;
                     anonym7->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym8 = &image->Adr[(x)
                *image->Len0+(y+1L)];
                     anonym8->r += (uint16_t)rr;
                     anonym8->g += (uint16_t)gg;
                     anonym8->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym9 = &image->Adr[(x)
                *image->Len0+(y-1L)];
                     anonym9->r += (uint16_t)rr;
                     anonym9->g += (uint16_t)gg;
                     anonym9->b += (uint16_t)bb;
                  }
                  { /* with */
                     struct maptool_PIX * anonym10 = &image->Adr[(x-2L)
                *image->Len0+y];
                     anonym10->r += (uint16_t)(rr/2UL);
                     anonym10->g += (uint16_t)(gg/2UL);
                     anonym10->b += (uint16_t)(bb/2UL);
                  }
                  { /* with */
                     struct maptool_PIX * anonym11 = &image->Adr[(x+2L)
                *image->Len0+y];
                     anonym11->r += (uint16_t)(rr/2UL);
                     anonym11->g += (uint16_t)(gg/2UL);
                     anonym11->b += (uint16_t)(bb/2UL);
                  }
                  { /* with */
                     struct maptool_PIX * anonym12 = &image->Adr[(x)
                *image->Len0+(y-2L)];
                     anonym12->r += (uint16_t)(rr/2UL);
                     anonym12->g += (uint16_t)(gg/2UL);
                     anonym12->b += (uint16_t)(bb/2UL);
                  }
                  { /* with */
                     struct maptool_PIX * anonym13 = &image->Adr[(x)
                *image->Len0+(y+2L)];
                     anonym13->r += (uint16_t)(rr/2UL);
                     anonym13->g += (uint16_t)(gg/2UL);
                     anonym13->b += (uint16_t)(bb/2UL);
                  }
                  done = 0;
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
   } while (!done);
   tmp = (int32_t)(image->Len0-1);
   y = 0L;
   if (y<=tmp) for (;; y++) {
      tmp0 = (int32_t)(image->Len1-1);
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym14 = &image->Adr[(x)*image->Len0+y];
            anonym14->r = (uint16_t)(((int32_t)anonym14->r*lum)/512L);
            anonym14->g = (uint16_t)(((int32_t)anonym14->g*lum)/512L);
            anonym14->b = (uint16_t)(((int32_t)anonym14->b*lum)/512L);
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end shine() */


extern void maptool_makebw(maptool_pIMAGE p)
{
   uint32_t w;
   uint32_t y;
   uint32_t x;
   struct maptool_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   tmp = p->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = p->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &p->Adr[(x)*p->Len0+y];
            w = ((uint32_t)anonym->r*340UL+(uint32_t)
                anonym->g*550UL+(uint32_t)anonym->b*110UL)/1000UL;
            anonym->r = (uint16_t)w;
            anonym->g = (uint16_t)w;
            anonym->b = (uint16_t)w;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end makebw() */

/*
PROCEDURE lim(n:CARDINAL):CARD16;
BEGIN IF n>MAX(CARD16) THEN RETURN MAX(CARD16) ELSE RETURN n END; END lim;
*/

static void addpix(struct maptool_PIX * s, struct maptool_PIX * a)
{
   /*
      h:=s.r + a.r; IF h>MAX(CARD16) THEN s.r:=MAX(CARD16) ELSE s.r:=h END;
      h:=s.g + a.g; IF h>MAX(CARD16) THEN s.g:=MAX(CARD16) ELSE s.g:=h END;
      h:=s.b + a.b; IF h>MAX(CARD16) THEN s.b:=MAX(CARD16) ELSE s.b:=h END;
   */
   s->r += a->r;
   s->g += a->g;
   s->b += a->b;
} /* end addpix() */


extern void maptool_addmap(maptool_pIMAGE image, maptool_pIMAGE map)
{
   int32_t y;
   int32_t x;
   int32_t tmp;
   int32_t tmp0;
   tmp = (int32_t)(image->Len1-1);
   x = 0L;
   if (x<=tmp) for (;; x++) {
      tmp0 = (int32_t)(image->Len0-1);
      y = 0L;
      if (y<=tmp0) for (;; y++) {
         addpix(&image->Adr[(x)*image->Len0+y], &map->Adr[(x)*map->Len0+y]);
         if (y==tmp0) break;
      } /* end for */
      if (x==tmp) break;
   } /* end for */
/*
      image^[x+xsize*y].r:=lim(map^[x+xsize*y].r + image^[x+xsize*y].r);
      image^[x+xsize*y].g:=lim(map^[x+xsize*y].g + image^[x+xsize*y].g);
      image^[x+xsize*y].b:=lim(map^[x+xsize*y].b + image^[x+xsize*y].b);
*/
} /* end addmap() */

/*
PROCEDURE openppm(fn:ARRAY OF CHAR; verb:BOOLEAN):File;
VAR fd:File;
    line:INTEGER;
    ch, cho:CHAR;
BEGIN
  cleanfilename(fn);
  fd:=OpenRead(fn);
  IF NOT FdValid(fd) THEN 
    IF verb THEN WrStr(fn); WrStrLn(" not found"); END;
    RETURN InvalidFd;
  END;
 
  line:=0;
  cho:=0C;
  LOOP
    IF RdBin(fd, ch, 1)<>1 THEN 
      WrStr(fn); WrStrLn(" read error"); Close(fd); RETURN InvalidFd
    END;
    IF ch=LF THEN INC(line) END; 
    IF line=3 THEN EXIT END;
    IF (ch="#") & (cho=LF) THEN DEC(line) END;
    cho:=ch;
  END;
  RETURN fd
END openppm; 
*/

static void mapname(int32_t x, int32_t y, int32_t zoom, char fn[],
                uint32_t fn_len, char reqn[], uint32_t reqn_len)
{
   char hh[21];
   char path[1001];
   fn[0UL] = 0;
   useri_confstr(useri_fOSMDIR, path, 1001ul);
   aprsstr_Append(fn, fn_len, path, 1001ul);
   aprsstr_Append(fn, fn_len, "/", 2ul);
   aprsstr_Append(fn, fn_len, aprsdecode_lums.mapname, 41ul);
   aprsstr_Append(fn, fn_len, "/", 2ul);
   /*WrInt(zoom,6);WrInt(x,6);WrInt(y,6);WrStrLn(" zoom x y"); */
   aprsstr_IntToStr(zoom, 1UL, hh, 21ul);
   aprsstr_Append(fn, fn_len, hh, 21ul);
   aprsstr_Append(fn, fn_len, "/", 2ul);
   /*    Append(fn, hh); */
   reqn[0UL] = 0;
   aprsstr_Append(reqn, reqn_len, aprsdecode_lums.mapname, 41ul);
   aprsstr_Append(reqn, reqn_len, " ", 2ul);
   aprsstr_Append(reqn, reqn_len, hh, 21ul);
   aprsstr_Append(reqn, reqn_len, " ", 2ul);
   /*
       IntToStr(x, 1, hh);
   
       Append(fn,"-");
       Append(fn, hh);
       Append(fn,"-");
   */
   aprsstr_IntToStr(x, 1UL, hh, 21ul);
   aprsstr_Append(fn, fn_len, hh, 21ul);
   aprsstr_Append(fn, fn_len, "/", 2ul);
   aprsstr_Append(reqn, reqn_len, hh, 21ul);
   aprsstr_Append(reqn, reqn_len, " ", 2ul);
   aprsstr_IntToStr(y, 1UL, hh, 21ul);
   aprsstr_Append(fn, fn_len, hh, 21ul);
   /*    Append(fn,".png"); */
   aprsstr_Append(reqn, reqn_len, hh, 21ul);
   aprsstr_Append(reqn, reqn_len, "\012", 2ul);
} /* end mapname() */


static uint32_t squaredelay(uint32_t c)
{
   uint32_t n;
   n = 1UL;
   while (c>0UL) {
      n += n;
      --c;
   }
   return n-1UL;
} /* end squaredelay() */


static char decodetile(const char fn[], uint32_t fn_len,
                pPNGBUF ppngbuf, int32_t maxx, int32_t maxy,
                int32_t maxxbyte)
{
   char s[100];
   aprsstr_Assign(s, 100ul, fn, fn_len);
   aprsstr_Append(s, 100ul, ".png", 5ul);
   if (ppngbuf) {
      if (readpng(s, (char * *)ppngbuf, &maxx, &maxy,
                &maxxbyte)>=0L || readjpg(s, (char * *)ppngbuf, &maxx,
                &maxy, &maxxbyte)>=0L) {
         /* normal png in .png */
         return 1;
      }
   }
   else if (osi_Exists(s, 100ul)) {
      /* jpg hided in .png */
      return 1;
   }
   aprsstr_Assign(s, 100ul, fn, fn_len);
   aprsstr_Append(s, 100ul, ".jpg", 5ul);
   if (ppngbuf) {
      return readjpg(s, (char * *)ppngbuf, &maxx, &maxy, &maxxbyte)>=0L;
                
   }
   /* jpg in .jpg */
   return osi_Exists(s, 100ul);
} /* end decodetile() */

/*
PROCEDURE existsimg(fn-:ARRAY OF CHAR):BOOLEAN;
VAR s:ARRAY[0..99] OF CHAR;
BEGIN
  Assign(s, fn); Append(s, PNGEXT);
  IF Exists(s) THEN RETURN TRUE END;
  Assign(s, fn); Append(s, JPGEXT);
  RETURN Exists(s)
END existsimg;
*/
#define maptool_MAPLOADPROGSTARTDELAY 10

#define maptool_MINSPACE 80


static void reqmap(char wfn[], uint32_t wfn_len, char byop)
/* append filename of missing tiles to file, 0C is flush */
{
   char h[1000];
   char s[1000];
   int32_t fd;
   uint32_t lb;
   X2C_PCOPY((void **)&wfn,wfn_len);
   /*WrStr(">");WrStr(wfn); */
   if (!byop) mapdelay = 0UL;
   if (mapnamesdone==1U) {
      /* filled buffer written */
      mapnamesdone = 0U;
      if (!osi_Exists("gettiles", 9ul)) {
         /* a maploader is at work */
         if ((byop && wfn[0UL]) && aprsstr_InStr(mapnamesbuf, 4096ul, wfn,
                wfn_len)>=0L) mapnamesdone = 2U;
         else maploopcnt = 0UL;
      }
      mapnamesbuf[0U] = 0;
   }
   if (maploopcnt<5UL || mapnamesbuf[0U]==0) {
      aprsstr_Append(mapnamesbuf, 4096ul, wfn, wfn_len);
                /* store 1 line if looping */
   }
   lb = aprsstr_Length(mapnamesbuf, 4096ul);
   if (lb>0UL && (wfn[0UL]==0 || lb+80UL>=4096UL)) {
      /* namebuffer full or flush */
      /*WrInt(maploopcnt, 10); WrInt(mapdelay, 10);
                WrInt(realtime-lastmapreq, 10); WrInt(realtime, 15);
                WrInt(lastmapreq, 15); WrStrLn(" delay");   */
      if (lastmapreq+mapdelay+squaredelay(maploopcnt)<=aprsdecode_realtime) {
         if (mapnamesdone==2U) {
            /* looping mapload */
            if (maploopcnt>=4UL) {
               useri_textautosize(0L, 0L, 5UL, 10UL, 'e', "get no map from do\
wnloader", 27ul);
            }
            if (maploopcnt<8UL) ++maploopcnt;
         }
         fd = osi_OpenWrite("gettiles", 9ul);
         if (osic_FdValid(fd)) {
            osi_WrBin(fd, (char *)mapnamesbuf, 4096u/1u, lb);
            osic_Close(fd);
            if (aprsdecode_verb) {
               osi_WrStr("try:", 5ul);
               osic_WrINT32(maploopcnt, 1UL);
               osi_WrStrLn(" written gettiles:", 19ul);
               osi_WrStr(mapnamesbuf, 4096ul);
            }
            lastmapreq = aprsdecode_realtime;
            mapdelay = 0UL;
            if (maploadstart+10UL<aprsdecode_realtime) {
               /* delay for new start */
               /*WrStrLn("cp"); */
               if (aprsdecode_maploadpid.runs) {
                  xosi_CheckProg(&aprsdecode_maploadpid);
               }
               if (!aprsdecode_maploadpid.runs) {
                  /* start map load task */
                  /*WrStrLn("ps"); */
                  useri_confstr(useri_fGETMAPS, s, 1000ul);
                /* maploader program name */
                  if (s[0U]) {
                     useri_confstr(useri_fOSMDIR, h, 1000ul);
                     if (h[0U]) {
                        aprsstr_Append(s, 1000ul, " -r \"", 6ul);
                /* add -r rootdir */
                        aprsstr_Append(s, 1000ul, h, 1000ul);
                        aprsstr_Append(s, 1000ul, "\"", 2ul);
                     }
                     xosi_StartProg(s, 1000ul, &aprsdecode_maploadpid);
                     if (aprsdecode_maploadpid.runs) {
                        maploadstart = aprsdecode_realtime;
                        useri_say("Start Mapdownload", 18ul, 3UL, 'g');
                     }
                     else {
                        strncpy(h,"can not start ",1000u);
                        aprsstr_Append(h, 1000ul, s, 1000ul);
                        useri_xerrmsg(h, 1000ul);
                        useri_refresh = 1;
                     }
                  }
               }
            }
            else if (maploadstart>aprsdecode_realtime) {
               maploadstart = 0UL; /* if clock jumped back */
            }
         }
         else osi_WrStrLn("cannot write gettiles", 22ul);
      }
      else if (lastmapreq>aprsdecode_realtime) {
         lastmapreq = aprsdecode_realtime; /* systime gone backward */
      }
      mapnamesdone = 1U;
   }
   X2C_PFREE(wfn);
} /* end reqmap() */


static void zoommap(float fzoom, maptool_pIMAGE map)
{
   int32_t yim;
   int32_t yi;
   int32_t mum;
   int32_t mul;
   int32_t bb;
   int32_t gg;
   int32_t rr;
   int32_t y00;
   int32_t y;
   int32_t x;
   float r;
   uint32_t tr;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   struct maptool_PIX * anonym3;
   struct maptool_PIX * anonym4;
   int32_t tmp;
   int32_t tmp0;
   y00 = (int32_t)((map->Len0-1)+1UL)-(int32_t)
                X2C_TRUNCI(X2C_DIVR((float)((map->Len0-1)+1UL),fzoom),
                X2C_min_longint,X2C_max_longint);
   for (x = (int32_t)(map->Len1-1); x>=0L; x--) {
      r = X2C_DIVR((float)x,fzoom);
      tr = aprsdecode_trunc(r);
      mul = (int32_t)X2C_TRUNCI(256.0f*(r-(float)tr),X2C_min_longint,
                X2C_max_longint);
      mum = 256L-mul;
      tmp = y00;
      y = (int32_t)(map->Len0-1);
      if (y>=tmp) for (;; y--) {
         { /* with */
            struct maptool_PIX * anonym = &map->Adr[(tr+1UL)*map->Len0+y];
            rr = (int32_t)anonym->r*mul;
            gg = (int32_t)anonym->g*mul;
            bb = (int32_t)anonym->b*mul;
         }
         { /* with */
            struct maptool_PIX * anonym0 = &map->Adr[(tr)*map->Len0+y];
            rr += (int32_t)anonym0->r*mum;
            gg += (int32_t)anonym0->g*mum;
            bb += (int32_t)anonym0->b*mum;
         }
         { /* with */
            struct maptool_PIX * anonym1 = &map->Adr[(x)*map->Len0+y];
            anonym1->r = (uint16_t)(rr/256L);
            anonym1->g = (uint16_t)(gg/256L);
            anonym1->b = (uint16_t)(bb/256L);
         }
         if (y==tmp) break;
      } /* end for */
   } /* end for */
   tmp = (int32_t)(map->Len0-1);
   y = 0L;
   if (y<=tmp) for (;; y++) {
      r = X2C_DIVR((float)((int32_t)(map->Len0-1)-y),fzoom);
      /*      yi:=VAL(INTEGER,HIGH(map^[0]))-VAL(INTEGER, r);  */
      /*      tr:=TRUNC(r); */
      tr = aprsdecode_trunc(r);
      yi = (int32_t)((map->Len0-1)-tr);
      mul = (int32_t)X2C_TRUNCI(256.0f*(r-(float)tr),X2C_min_longint,
                X2C_max_longint);
      mum = 256L-mul;
      if (yi>0L) yim = yi-1L;
      else yim = 0L;
      tmp0 = (int32_t)(map->Len1-1);
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym2 = &map->Adr[(x)*map->Len0+yim];
            rr = (int32_t)anonym2->r*mul;
            gg = (int32_t)anonym2->g*mul;
            bb = (int32_t)anonym2->b*mul;
         }
         { /* with */
            struct maptool_PIX * anonym3 = &map->Adr[(x)*map->Len0+yi];
            rr += (int32_t)anonym3->r*mum;
            gg += (int32_t)anonym3->g*mum;
            bb += (int32_t)anonym3->b*mum;
         }
         { /* with */
            struct maptool_PIX * anonym4 = &map->Adr[(x)*map->Len0+y];
            anonym4->r = (uint16_t)(rr/256L);
            anonym4->g = (uint16_t)(gg/256L);
            anonym4->b = (uint16_t)(bb/256L);
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end zoommap() */


static uint16_t col(int32_t c)
{
   return (uint16_t)((aprsdecode_lums.maplumcorr[c]*(uint32_t)
                aprsdecode_lums.map)/1024UL);
} /* end col() */


static char loadtile(maptool_pIMAGE map, char * done,
                char dryrun, const char h[], uint32_t h_len,
                const char wfn[], uint32_t wfn_len, int32_t dx,
                int32_t dy, uint32_t doubx, uint32_t douby)
{
   int32_t iy;
   int32_t ix;
   int32_t yy;
   int32_t xx;
   int32_t y;
   int32_t x;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   int32_t tmp;
   int32_t tmp0;
   if (dryrun) {
      if (decodetile(h, h_len, 0, 0L, 0L, 0L)) return 1;
      else {
         *done = 0;
         return 0;
      }
   }
   if (map==0) return 0;
   if (aprsdecode_verb) {
      osi_WrStr("open>", 6ul);
      osi_WrStr(h, h_len);
      osi_WrStrLn("<", 2ul);
   }
   if (!decodetile(h, h_len, (pPNGBUF)pngbuf, 256L, 256L,
                768L) || useri_reloadmap) {
      if (wfn[0]) {
         if ((useri_configon(useri_fGETMAPS) || maptool_mappack.run)
                || useri_reloadmap) reqmap(wfn, wfn_len, !useri_reloadmap);
         *done = 0;
      }
      tmp = (dy+256L)-1L;
      y = dy;
      if (y<=tmp) for (;; y++) {
         tmp0 = (dx+256L)-1L;
         x = dx;
         if (x<=tmp0) for (;; x++) {
            if (((x>=0L && x<(int32_t)((map->Len1-1)+1UL)) && y>=0L)
                && y<(int32_t)((map->Len0-1)+1UL)) {
               { /* with */
                  struct maptool_PIX * anonym = &map->Adr[(x)*map->Len0+y];
                  anonym->r = 100U;
                  anonym->g = 120U;
                  anonym->b = 100U;
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
      return 0;
   }
   if (doubx>0UL) {
      /* use half zoom tile and double size */
      if (douby==1UL) {
         yy = 0L;
         y = 255L;
         iy = -1L;
      }
      else {
         yy = 128L;
         y = 0L;
         iy = 1L;
      }
      do {
         if (doubx==1UL) {
            xx = 0L;
            x = 255L;
            ix = -1L;
         }
         else {
            xx = 128L;
            x = 0L;
            ix = 1L;
         }
         do {
            pngbuf[y][x] = pngbuf[yy+y/2L][xx+x/2L];
            x += ix;
         } while (!(x<0L || x>255L));
         y += iy;
      } while (!(y<0L || y>255L));
   }
   for (y = 0L; y<=255L; y++) {
      yy = (255L-y)+dy;
      for (x = 0L; x<=255L; x++) {
         xx = x+dx;
         if (((xx>=0L && xx<(int32_t)((map->Len1-1)+1UL)) && yy>=0L)
                && yy<(int32_t)((map->Len0-1)+1UL)) {
            { /* with */
               struct maptool_PIX * anonym0 = &map->Adr[(xx)*map->Len0+yy];
               anonym0->r = col((int32_t)pngbuf[y][x].r8);
               anonym0->g = col((int32_t)pngbuf[y][x].g8);
               anonym0->b = col((int32_t)pngbuf[y][x].b8);
            }
         }
      } /* end for */
   } /* end for */
   return 1;
} /* end loadtile() */


extern void maptool_loadmap(maptool_pIMAGE map, int32_t tx, int32_t ty,
                int32_t zoom, float fzoom, float shftx,
                float shfty, char * done, char * blown,
                char blow, char dryrun)
{
   int32_t yh;
   int32_t xh;
   int32_t dy;
   int32_t dx;
   int32_t maxtil;
   int32_t y;
   int32_t x;
   char wfn[4096];
   char fnn[4096];
   char ok0;
   int32_t tmp;
   int32_t tmp0;
   *done = 1;
   *blown = 0;
   /*  FILL(map, 0C, SIZE(PIX)*xsize*ysize); */
   maxtil = (int32_t)(aprsdecode_trunc(expzoom(zoom))-1UL);
   yh = (int32_t)X2C_TRUNCI(X2C_DIVR((float)(map->Len0-1),fzoom)+shfty,
                X2C_min_longint,X2C_max_longint)/256L;
   xh = (int32_t)X2C_TRUNCI(X2C_DIVR((float)(map->Len1-1),fzoom)+shftx,
                X2C_min_longint,X2C_max_longint)/256L;
   /*  IF ((tx+1>=maxtil) OR (ty+1>=maxtil)) & NOT dryrun THEN clr(map) END;
                */
   /*  FOR y:=0 TO VAL(INTEGER, FLOAT(HIGH(map^[0]))/fzoom+shfty) DIV TILESIZE DO */
   /*    FOR x:=0 TO VAL(INTEGER,
                FLOAT(HIGH(map^))/fzoom+shftx) DIV TILESIZE DO */
   if ((tx+xh>maxtil || ty+yh>maxtil) && !dryrun) maptool_clr(map);
   tmp = yh;
   y = 0L;
   if (y<=tmp) for (;; y++) {
      tmp0 = xh;
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         if (tx+x<=maxtil && ty+y<=maxtil) {
            mapname(tx+x, ty+y, zoom, fnn, 4096ul, wfn, 4096ul);
            dx = x*256L-(int32_t)X2C_TRUNCI(shftx,X2C_min_longint,
                X2C_max_longint);
            dy = (((int32_t)((map->Len0-1)+1UL)-256L)-y*256L)+(int32_t)
                X2C_TRUNCI(shfty,X2C_min_longint,X2C_max_longint);
            if (((!loadtile(map, done, dryrun, fnn, 4096ul, wfn, 4096ul, dx,
                dy, 0UL, 0UL) && zoom>4L) && !dryrun) && blow) {
               mapname((tx+x)/2L, (ty+y)/2L, zoom-1L, fnn, 4096ul, wfn,
                4096ul);
               ok0 = loadtile(map, done, dryrun, fnn, 4096ul, "", 1ul, dx,
                dy, (uint32_t)((tx+x&1L)+1L), (uint32_t)((ty+y&1L)+1L));
               if (ok0) *blown = 1;
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   if (!dryrun) {
      if (fzoom>1.0f) zoommap(fzoom, map);
      if (mapnamesbuf[0U]) reqmap("", 1ul, !useri_reloadmap);
      else osi_Erase("gettiles", 9ul, &ok0);
   }
   useri_reloadmap = 0;
} /* end loadmap() */


extern char maptool_IsMapLoaded(void)
{
   return !aprsdecode_maploadpid.runs || !osi_Exists("gettiles", 9ul);
} /* end IsMapLoaded() */


static void margin(int32_t * ty1, int32_t * tx1, int32_t * ty0,
                int32_t * tx0, int32_t * z)
{
   int32_t maxtil;
   float py;
   float px;
   struct maptool__D0 * anonym;
   { /* with */
      struct maptool__D0 * anonym = &maptool_mappack;
      maptool_mercator(anonym->leftup.long0, anonym->leftup.lat, *z, tx0,
                ty0, &px, &py);
      maptool_mercator(anonym->rightdown.long0, anonym->rightdown.lat, *z,
                tx1, ty1, &px, &py);
      maxtil = (int32_t)(aprsdecode_trunc(expzoom(*z))-1UL);
      if (*tx0>maxtil) *tx0 = maxtil;
      if (*tx1>maxtil) *tx1 = maxtil;
      if (*ty0>maxtil) *ty0 = maxtil;
      if (*ty1>maxtil) *ty1 = maxtil;
   }
} /* end margin() */


static char inc(int32_t * x, int32_t * y, int32_t * z)
{
   int32_t ty1;
   int32_t ty0;
   int32_t tx1;
   int32_t tx0;
   if (*z==0L) {
      *z = 1L;
      margin(&ty1, &tx1, &ty0, &tx0, z);
      *x = tx0;
      *y = ty0;
   }
   else {
      margin(&ty1, &tx1, &ty0, &tx0, z);
      ++*x;
      if (*x>tx1) {
         *x = tx0;
         ++*y;
         if (*y>ty1) {
            ++*z;
            if (*z>maptool_mappack.tozoom) return 0;
            margin(&ty1, &tx1, &ty0, &tx0, z);
            *x = tx0;
            *y = ty0;
         }
      }
   }
   return 1;
} /* end inc() */


static char checktile(char fn[], uint32_t fn_len)
{
   char checktile_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   checktile_ret = decodetile(fn, fn_len, (pPNGBUF)pngbuf, 256L, 256L, 768L);
                
   X2C_PFREE(fn);
   return checktile_ret;
} /* end checktile() */

#define maptool_MAXREQ 10

#define maptool_MAXRETRYS 10


extern void maptool_MapPackageJob(char dryrun)
{
   int32_t z;
   int32_t y;
   int32_t x;
   char rfn[4096];
   char fn[4096];
   char s1[100];
   char s[100];
   uint32_t mapc;
   uint32_t rcnt;
   uint32_t startt;
   char done;
   struct maptool__D0 * anonym;
   { /* with */
      struct maptool__D0 * anonym = &maptool_mappack;
      if (dryrun) {
         mapc = 0UL;
         do {
            ++mapc;
         } while (inc(&anonym->tx, &anonym->ty, &anonym->zoom));
         anonym->zoom = 0L; /* reinit counters */
         startt = osic_time();
         aprsdecode_click.chkmaps = 1;
         do {
            if (anonym->zoom>0L) {
               /* zoom 0 is init */
               mapname(anonym->tx, anonym->ty, anonym->zoom, fn, 4096ul, rfn,
                 4096ul);
               if (!decodetile(fn, 4096ul, 0, 0L, 0L, 0L)) ++anonym->needcnt;
               /*            IF needcnt>MAXLOOKUP THEN overflow:=TRUE END;
                   (* to long calculation time *) */
               ++anonym->mapscnt;
            }
            done = !inc(&anonym->tx, &anonym->ty, &anonym->zoom);
            if (done || (anonym->mapscnt&15UL)==15UL) {
               strncpy(s,"Checked Maps:",100u);
               aprsstr_IntToStr((int32_t)anonym->mapscnt, 0UL, s1, 100ul);
               aprsstr_Append(s, 100ul, s1, 100ul);
               aprsstr_Append(s, 100ul, "  missing:", 11ul);
               aprsstr_IntToStr((int32_t)anonym->needcnt, 0UL, s1, 100ul);
               aprsstr_Append(s, 100ul, s1, 100ul);
               progress(startt, s, 100ul, (100UL*anonym->mapscnt)/mapc,
                done);
            }
         } while (!(done || !aprsdecode_click.chkmaps));
         if (!aprsdecode_click.chkmaps) maptool_mappack.needcnt = 0UL;
         return;
      }
      if (!anonym->run) return;
      if (anonym->delay+1UL<anonym->retrys*anonym->retrys) {
         ++anonym->delay; /* increasing delay */
         return;
      }
      anonym->delay = 0UL;
      if (maptool_IsMapLoaded()) {
         /* check what have */
         /*WrStrLn("gettiles deleted"); */
         for (;;) {
            if (anonym->zoom>0L) {
               /* zoom 0 is init */
               mapname(anonym->tx, anonym->ty, anonym->zoom, fn, 4096ul, rfn,
                 4096ul);
               if (!checktile(fn, 4096ul)) {
                  if (anonym->retrys<10UL) break;
                  ++anonym->givups;
               }
               ++anonym->donecnt;
               anonym->retrys = 0UL;
            }
            if (!inc(&anonym->tx, &anonym->ty, &anonym->zoom)) {
               anonym->run = 0;
               useri_downloadprogress(); /* say end of job */
               return;
            }
         }
         /* all done */
         useri_downloadprogress();
         rcnt = 0UL;
         x = anonym->tx;
         y = anonym->ty;
         z = anonym->zoom;
         do {
            mapname(x, y, z, fn, 4096ul, rfn, 4096ul);
            if (!checktile(fn, 4096ul)) {
               reqmap(rfn, 4096ul, 0);
               ++rcnt;
            }
         } while (!(!inc(&x, &y, &z) || rcnt>=10UL));
         /* last map */
         reqmap("", 1ul, 0); /* flush request buffer */
         if (anonym->retrys>0UL) ++anonym->retrysum;
         ++anonym->retrys;
      }
   }
} /* end MapPackageJob() */


extern void maptool_StartMapPackage(struct aprspos_POSITION lu,
                struct aprspos_POSITION rd, int32_t tillzoom,
                char dryrun)
{
   struct maptool__D0 * anonym;
   memset((char *) &maptool_mappack,(char)0,
                sizeof(struct maptool__D0));
   { /* with */
      struct maptool__D0 * anonym = &maptool_mappack;
      anonym->leftup = lu;
      anonym->rightdown = rd;
      anonym->tozoom = tillzoom;
      anonym->run = !dryrun;
   }
   if (dryrun) maptool_MapPackageJob(1);
} /* end StartMapPackage() */

typedef struct PIX8 * pROWS1;


static void loadsym(char h[], uint32_t h_len)
{
   uint32_t y;
   uint32_t x;
   pROWS1 rows[16];
   int32_t res;
   int32_t maxxbyte;
   int32_t maxy;
   int32_t maxx;
   char bu[16][9216];
   struct PIX8 * anonym;
   X2C_PCOPY((void **)&h,h_len);
   for (y = 0UL; y<=15UL; y++) {
      rows[y] = (pROWS1) &bu[y][0U];
   } /* end for */
   maxx = 3072L;
   maxy = 16L;
   maxxbyte = maxx*3L;
   res = readpng(h, (char * *)rows, &maxx, &maxy, &maxxbyte);
   if (res<0L) {
      osi_WrStr(h, h_len);
      osi_WrStrLn(" file read error ", 18ul);
      osic_WrINT32((uint32_t)res, 1UL);
      osi_WrStrLn("", 1ul);
      goto label;
   }
   memset((char *)symbols,(char)0,sizeof(struct PIX8 [3072][17]));
   for (y = 0UL; y<=15UL; y++) {
      for (x = 0UL; x<=3071UL; x++) {
         { /* with */
            struct PIX8 * anonym = &symbols[x][15UL-y];
            anonym->r8 = rows[y][x].r8;
            anonym->g8 = rows[y][x].g8;
            anonym->b8 = rows[y][x].b8;
         }
      } /* end for */
   } /* end for */
   label:;
   X2C_PFREE(h);
} /* end loadsym() */


static float sinc(float x, uint32_t w)
/* sin(x)/x with hann windown */
{
   float win;
   if ((float)fabs(x)<0.001f) return 1.0f;
   win = X2C_DIVR(3.1415926535f*x,(float)w);
   x = x*3.1415926535f;
   return X2C_DIVR((X2C_DIVR(osic_sin(x),x))*osic_sin(win),win);
} /* end sinc() */

#define maptool_MAXY 24
/* font image max y size */

#define maptool_MAXX 600
/* font image max x size */

#define maptool_CHARX 6
/* chars x size pixel */

#define maptool_MINWIDTH 3

#define maptool_GARBAGE 500
/* sum of pixels outside y font image */

#define maptool_FIRLEN 3

typedef char * pROWS;


extern void maptool_loadfont(void)
{
   uint32_t higth;
   uint32_t i;
   uint32_t fonty;
   uint32_t y1;
   uint32_t y00;
   uint32_t xshift;
   uint32_t y;
   uint32_t x;
   uint32_t c;
   uint16_t m;
   int32_t res;
   int32_t maxxbyte;
   int32_t maxy;
   int32_t maxx;
   pROWS rows[24];
   char bu[24][600];
   uint32_t xhist[6];
   uint32_t yhist[24];
   float fir[31];
   float yr;
   float sum;
   uint32_t nin;
   char fn[1025];
   struct _1 * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   higth = (uint32_t)useri_conf2int(useri_fFONTSIZE, 0UL, 7L, 18L, 10L);
   memset((char *)font,(char)0,sizeof(struct _1 [97]));
   for (y = 0UL; y<=23UL; y++) {
      rows[y] = (pROWS) &bu[y][0U];
   } /* end for */
   maxx = 600L;
   maxy = 24L;
   maxxbyte = maxx;
   strncpy(fn,"font.png",1025u);
   res = readpng(fn, (char * *)rows, &maxx, &maxy, &maxxbyte);
   if ((((res<0L || maxx<1L) || maxx>600L) || maxy<1L) || maxy>24L) {
      osic_WrINT32((uint32_t)res, 1UL);
      osi_WrStrLn(" fontfile read error", 21ul);
      aprsdecode_lums.fontysize = higth+3UL;
      return;
   }
   for (x = 0UL; x<=5UL; x++) {
      xhist[x] = 0UL;
   } /* end for */
   for (y = 0UL; y<=23UL; y++) {
      yhist[y] = 0UL;
   } /* end for */
   tmp = (uint32_t)(maxx-1L);
   x = 0UL;
   if (x<=tmp) for (;; x++) {
      /* make column histogram to find char position in font file */
      tmp0 = (uint32_t)(maxy-1L);
      y = 0UL;
      if (y<=tmp0) for (;; y++) {
         xhist[x%6UL] += (uint32_t)(uint8_t)rows[y][x];
         yhist[y] += (uint32_t)(uint8_t)rows[y][x];
         if (y==tmp0) break;
      } /* end for */
      if (x==tmp) break;
   } /* end for */
   c = X2C_max_longcard;
   for (x = 0UL; x<=5UL; x++) {
      if (xhist[x]<c) {
         c = xhist[x]; /* find space between chars */
         xshift = x;
      }
   } /* end for */
   y00 = 0UL;
   y1 = 0UL;
   for (y = 0UL; y<=23UL; y++) {
      /* find y position and size */
      if (yhist[y]>500UL) {
         if (y00==0UL) y00 = y;
         y1 = y;
      }
   } /* end for */
   if (y1<y00+4UL) {
      osi_WrStrLn(" font too small error", 22ul);
      return;
   }
   /*
     fonty:=y1-y0+3;
     IF fonty>MAXFONTY THEN
       WrStrLn(" font too high");
       fonty:=MAXFONTY; 
     END;
   */
   nin = (y1-y00)+1UL;
   /*WrInt(y0, 10); WrInt(y1, 10); WrLn; */
   fonty = higth+3UL;
   for (y = 0UL; y<=30UL; y++) {
      fir[y] = 0.0f;
   } /* end for */
   for (x = 0UL; x<=581UL; x++) {
      tmp = nin-1UL;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         fir[y+3UL] = (float)(uint32_t)(uint8_t)rows[y00+y][x+xshift];
         if (y==tmp) break;
      } /* end for */
      /*FOR y:=0 TO HIGH(fir) DO WrInt(trunc(fir[y]), 8); END; WrLn; */
      tmp = higth-1UL;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         if (higth!=nin) {
            yr = X2C_DIVR((float)(y*nin),(float)higth);
            sum = 0.0f;
            for (i = 1UL; i<=6UL; i++) {
               sum = sum+sinc(((float)i-3.0f)-(yr-(float)
                aprsdecode_trunc(yr)), 3UL)*fir[i+aprsdecode_trunc(yr)];
            } /* end for */
         }
         else sum = fir[y+3UL];
         if (sum<=0.0f) c = 0UL;
         else if (sum>255.9f) c = 255UL;
         else c = aprsdecode_trunc(sum);
         font[x/6UL].char0[(fonty-y)-3UL][x%6UL] = (uint8_t)c;
         if (y==tmp) break;
      } /* end for */
   } /* end for */
   /*
     FOR y:=0 TO fonty-3 DO
       FOR x:=0 TO CHARS*CHARX-1 DO
         WITH font[x DIV CHARX] DO char[fonty-y-3][x MOD 6]:=ORD(rows[y0+y]
                ^[x+xshift]) END;
       END; 
     END;
   */
   aprsdecode_lums.fontysize = fonty;
   for (c = 0UL; c<=96UL; c++) {
      { /* with */
         struct _1 * anonym = &font[c];
         anonym->width = 3U;
         tmp = fonty-3UL;
         y = 0UL;
         if (y<=tmp) for (;; y++) {
            for (x = 0UL; x<=7UL; x++) {
               if (anonym->char0[y][x]>=90U) {
                  /* 128 */
                  if (c) {
                     m = X2C_LSH(0x1FU,16,(int32_t)x-1L)&0xFEU;
                     anonym->mask[y] = anonym->mask[y]|m;
                     if (y+1UL<=20UL) {
                        anonym->mask[y+1UL] = anonym->mask[y+1UL]|m;
                     }
                     if (y+2UL<=20UL) {
                        anonym->mask[y+2UL] = anonym->mask[y+2UL]|m;
                     }
                  }
                  if (x>(uint32_t)anonym->width) {
                     anonym->width = (uint8_t)x;
                  }
               }
            } /* end for */
            if (y==tmp) break;
         } /* end for */
      }
   } /* end for */
} /* end loadfont() */

#define maptool_BMPHLEN 54


static void numh(char h[256], uint32_t n, uint32_t pos,
                uint32_t size)
{
   uint32_t i;
   uint32_t tmp;
   tmp = size-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      h[pos+i] = (char)(n&255UL);
      n = n/256UL;
      if (i==tmp) break;
   } /* end for */
} /* end numh() */


static void wr(int32_t fd, uint32_t * len, char b[32768],
                uint16_t c)
{
   if (c<=1023U) b[*len] = gammatab[c];
   else b[*len] = '\377';
   ++*len;
   if (*len>32767UL) {
      osi_WrBin(fd, (char *)b, 32768u/1u, *len);
      *len = 0UL;
   }
} /* end wr() */


static uint8_t pngc(uint16_t c)
{
   if (c<=1023U) return gammatab[c];
   else return 255U;
   return 0;
} /* end pngc() */


extern int32_t maptool_saveppm(char fn[], uint32_t fn_len,
                maptool_pIMAGE image, int32_t xsize, int32_t ysize)
{
   int32_t fd;
   char h[256];
   int32_t y;
   int32_t x;
   char b[32768];
   uint32_t len;
   struct PNGPIXMAP pngimg;
   int32_t ret;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct PNGPIXEL * anonym1;
   struct maptool_PIX * anonym2;
   int32_t tmp;
   int32_t tmp0;
   int32_t maptool_saveppm_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   aprsstr_cleanfilename(fn, fn_len);
   fd = osi_OpenWrite(fn, fn_len);
   if (!osic_FdValid(fd)) {
      /*    WrStr(fn); WrStrLn(" not writeable"); */
      maptool_saveppm_ret = -1L;
      goto label;
   }
   len = aprsstr_Length(fn, fn_len); /* check fn if ppm or bmp */
   if ((((len>=4UL && fn[len-4UL]=='.') && X2C_CAP(fn[len-3UL])=='B')
                && X2C_CAP(fn[len-2UL])=='M') && X2C_CAP(fn[len-1UL])=='P') {
      /* make BMP */
      memset((char *)h,(char)0,256UL);
      h[0U] = 'B';
      h[1U] = 'M';
      numh(h, (uint32_t)(((xsize*3L+4L)/4L)*4L*ysize+54L), 2UL, 4UL);
                /* file len pad lines to x4 byte */
      numh(h, 54UL, 10UL, 4UL); /* headerlen */
      numh(h, 40UL, 14UL, 4UL); /* DWORD   biSize */
      numh(h, (uint32_t)xsize, 18UL, 4UL); /* LONG    biWidth */
      numh(h, (uint32_t)ysize, 22UL, 4UL); /* LONG    biHeight */
      numh(h, 1UL, 26UL, 2UL); /* WORD    biPlanes */
      numh(h, 24UL, 28UL, 2UL); /* WORD    biBitCount */
      numh(h, 0UL, 30UL, 4UL); /* DWORD   biCompression */
      osi_WrBin(fd, (char *)h, 256u/1u, 54UL);
      len = 0UL;
      tmp = ysize-1L;
      y = 0L;
      if (y<=tmp) for (;; y++) {
         tmp0 = xsize-1L;
         x = 0L;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct maptool_PIX * anonym = &image->Adr[(x)*image->Len0+y];
               wr(fd, &len, b, anonym->b);
               wr(fd, &len, b, anonym->g);
               wr(fd, &len, b, anonym->r);
            }
            if (x==tmp0) break;
         } /* end for */
         while (len&3UL) wr(fd, &len, b, 0U);
         if (y==tmp) break;
      } /* end for */
      if (len>0UL) osi_WrBin(fd, (char *)b, 32768u/1u, len);
      osic_Close(fd);
      ret = 0L;
   }
   else if ((((len>=4UL && fn[len-4UL]=='.') && X2C_CAP(fn[len-3UL])=='P')
                && X2C_CAP(fn[len-2UL])=='N') && X2C_CAP(fn[len-1UL])=='G') {
      /* make PNG */
      ret = -1L;
      osic_Close(fd);
      osic_alloc((char * *) &pngimg.image,
                (uint32_t)(xsize*ysize*3L));
      if (pngimg.image) {
         tmp = ysize-1L;
         y = 0L;
         if (y<=tmp) for (;; y++) {
            tmp0 = xsize-1L;
            x = 0L;
            if (x<=tmp0) for (;; x++) {
               { /* with */
                  struct maptool_PIX * anonym0 = &image->Adr[(x)
                *image->Len0+((ysize-1L)-y)];
                  { /* with */
                     struct PNGPIXEL * anonym1 = &pngimg.image[x+y*xsize];
                     anonym1->red = pngc(anonym0->r);
                     anonym1->green = pngc(anonym0->g);
                     anonym1->blue = pngc(anonym0->b);
                  }
               }
               if (x==tmp0) break;
            } /* end for */
            if (y==tmp) break;
         } /* end for */
         pngimg.width = (uint32_t)xsize;
         pngimg.height = (uint32_t)ysize;
         ret = writepng(fn, &pngimg);
         osic_free((char * *) &pngimg.image,
                (uint32_t)(xsize*ysize*3L));
      }
      else osi_WrStrLn("png write out of memory", 24ul);
   }
   else {
      /* ppm header */
      strncpy(h,"P6\012",256u);
      osi_WrBin(fd, (char *)h, 256u/1u, aprsstr_Length(h, 256ul));
      aprsstr_IntToStr(xsize, 1UL, h, 256ul);
      osi_WrBin(fd, (char *)h, 256u/1u, aprsstr_Length(h, 256ul));
      strncpy(h," ",256u);
      osi_WrBin(fd, (char *)h, 256u/1u, aprsstr_Length(h, 256ul));
      aprsstr_IntToStr(ysize, 1UL, h, 256ul);
      osi_WrBin(fd, (char *)h, 256u/1u, aprsstr_Length(h, 256ul));
      strncpy(h,"\012255\012",256u);
      osi_WrBin(fd, (char *)h, 256u/1u, aprsstr_Length(h, 256ul));
      len = 0UL;
      for (y = ysize-1L; y>=0L; y--) {
         tmp = xsize-1L;
         x = 0L;
         if (x<=tmp) for (;; x++) {
            { /* with */
               struct maptool_PIX * anonym2 = &image->Adr[(x)*image->Len0+y];
                
               wr(fd, &len, b, anonym2->r);
               wr(fd, &len, b, anonym2->g);
               wr(fd, &len, b, anonym2->b);
            }
            if (x==tmp) break;
         } /* end for */
      } /* end for */
      if (len>0UL) osi_WrBin(fd, (char *)b, 32768u/1u, len);
      osic_Close(fd);
      ret = 0L;
   }
   maptool_saveppm_ret = ret;
   label:;
   X2C_PFREE(fn);
   return maptool_saveppm_ret;
} /* end saveppm() */


static void allocpngbuf(void)
{
   uint32_t i;
   for (i = 0UL; i<=255UL; i++) {
      osic_alloc((char **) &pngbuf[i], sizeof(ROWS0));
      useri_debugmem.req = sizeof(pROWS0);
      useri_debugmem.screens += useri_debugmem.req;
      if (pngbuf[i]==0) {
         osi_WrStrLn("pngbuf out of memory", 21ul);
         useri_wrheap();
         X2C_ABORT();
      }
   } /* end for */
} /* end allocpngbuf() */


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
   i = 0UL;
   for (;;) {
      s[i] = getch(b, fd, len, p);
      if (s[i]==0) return -1L;
      if (s[i]=='\012') {
         s[i] = 0;
         return 0L;
      }
      if (s[i]==',') {
         s[i] = 0;
         return 1L;
      }
      if (i<s_len-1 && (uint8_t)s[i]>=' ') ++i;
   }
   return 0;
} /* end getword() */


extern void maptool_rdmountains(char fn[], uint32_t fn_len,
                char add)
/* import csv file with mountain name, pos, altitude */
{
   int32_t r;
   int32_t len;
   int32_t p;
   int32_t fd;
   aprsdecode_pMOUNTAIN pm;
   struct aprspos_POSITION pos;
   float alt;
   char b[4096];
   char s[1024];
   char name[100];
   char long0[100];
   char lat[100];
   char com[100];
   X2C_PCOPY((void **)&fn,fn_len);
   if (!add) {
      /* delete data */
      while (aprsdecode_mountains) {
         pm = aprsdecode_mountains;
         aprsdecode_mountains = pm->next;
         osic_free((char * *) &pm, sizeof(struct aprsdecode_MOUNTAIN));
         useri_debugmem.srtm -= sizeof(struct aprsdecode_MOUNTAIN);
      }
   }
   b[0U] = 0;
   useri_confstr(useri_fOSMDIR, s, 1024ul);
   aprsstr_Append(b, 4096ul, s, 1024ul);
   aprsstr_Append(b, 4096ul, "/", 2ul);
   aprsstr_Append(b, 4096ul, fn, fn_len);
   fd = osi_OpenRead(b, 4096ul);
   if (!osic_FdValid(fd)) goto label;
   p = 0L;
   len = 0L;
   for (;;) {
      r = getword(&p, &len, fd, b, com, 100ul);
      if (r>0L) {
         r = getword(&p, &len, fd, b, name, 100ul);
         if (r>0L) {
            r = getword(&p, &len, fd, b, s, 1024ul);
            if (r>0L) {
               r = getword(&p, &len, fd, b, lat, 100ul);
               if (r>0L) {
                  r = getword(&p, &len, fd, b, long0, 100ul);
                  if (r>0L) {
                     r = getword(&p, &len, fd, b, s, 1024ul);
                     if (r<0L || !aprsstr_StrToFix(&alt, s, 1024ul)) {
                        alt = 0.0f;
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
                100ul)) && aprspos_posvalid(pos)) {
         osic_alloc((char * *) &pm, sizeof(struct aprsdecode_MOUNTAIN));
         if (pm==0) break;
         useri_debugmem.srtm += sizeof(struct aprsdecode_MOUNTAIN);
         aprsstr_Assign(pm->name, 32ul, name, 100ul);
         pm->pos.lat = pos.lat*1.7453292519444E-2f;
         pm->pos.long0 = pos.long0*1.7453292519444E-2f;
         if (alt<0.0f || alt>9999.0f) alt = 0.0f;
         pm->alt = (short)aprsdecode_trunc(alt);
         pm->next = aprsdecode_mountains;
         aprsdecode_mountains = pm;
      }
   }
   /*WrInt(pm^.alt, 10); WrStrLn(pm^.name); */
   osic_Close(fd);
   label:;
   X2C_PFREE(fn);
} /* end rdmountains() */


extern void maptool_BEGIN(void)
{
   static int maptool_init = 0;
   if (maptool_init) return;
   maptool_init = 1;
   if (sizeof(FN)!=1024) X2C_ASSERT(0);
   aprstext_BEGIN();
   useri_BEGIN();
   xosi_BEGIN();
   osi_BEGIN();
   aprspos_BEGIN();
   aprsstr_BEGIN();
   aprsdecode_BEGIN();
   aprsdecode_maploadpid.runs = 0;
   maploadstart = 0UL;
   loadsym("symbols.png", 12ul);
   /*  loadfont; */
   makegammatab();
   allocpngbuf();
   mapnamesbuf[0U] = 0;
   mapnamesdone = 0U;
   maploopcnt = 0UL;
   lastmapreq = 0UL;
   mapdelay = 0UL;
   lastpoinum = 0UL;
   osic_alloc((char * *) &srtmmiss, 1UL);
                /* make empty tile for fast nofile hint */
   initsrtm();
}

