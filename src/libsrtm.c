/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#define libsrtm_C_
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprspos_H_
#include "aprspos.h"
#endif



uint32_t libsrtm_srtmmem;
uint32_t libsrtm_srtmmaxmem;
char libsrtm_srtmdir[1024];
char libsrtm_bicubic;
/* get altitude out of srtm files directory tree by oe5dxl */
#define libsrtm_SRTMXY 3600

#define libsrtm_STRIPS 3

#define libsrtm_AGELEVELS 5

#define libsrtm_ATTRSUB 10000
/* segment altitude to add metadata */

#define libsrtm_ATTRNEG 1000
/* max under see level */

#define libsrtm_NOALT 32767

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

typedef struct _0 SRTM30FD[9][4];

static SRTMLONG srtmcache;

static pSRTMTILE srtmmiss;

/* cache no file info with pointer to here */
static SRTM30FD srtm30fd; /* open srtm30 files */

static char errflag;

#define libsrtm_SRTM3DIR "srtm3"

#define libsrtm_SRTM1DIR "srtm1"

#define libsrtm_SRTM30DIR "srtm30"


/* === srtm lib */
extern int32_t libsrtm_opensrtm(uint8_t t, uint32_t tlat,
                uint32_t tlong)
{
   char s[21];
   uint32_t xd;
   uint32_t yi;
   uint32_t xi;
   uint32_t n;
   int32_t f;
   char path[1025];
   aprsstr_Assign(path, 1025ul, libsrtm_srtmdir, 1024ul);
   if (t==3U) aprsstr_Append(path, 1025ul, "/srtm3/", 8ul);
   else if (t==1U) aprsstr_Append(path, 1025ul, "/srtm1/", 8ul);
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
      aprsstr_Append(path, 1025ul, s, 21ul);
      return osi_OpenRead(path, 1025ul);
   }
   else {
      aprsstr_Append(path, 1025ul, "/srtm30/", 9ul);
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
      else if (tlat>=80UL) {
         s[4U] = 'N';
         s[5U] = '4';
         yi = 2UL;
      }
      else if (tlat>=30UL) {
         s[4U] = 'S';
         s[5U] = '1';
         yi = 1UL;
      }
      else {
         /* does not exist */
         yi = 0UL;
         srtm30fd[xi][0U].fd = -1L;
         srtm30fd[xi][0U].havefile = 1;
      }
      /*WrInt(xi, 4);WrInt(yi, 4); WrInt(ORD(srtm30fd[xi, yi].havefile), 4);
                */
      /*WrInt(srtm30fd[xi, yi].fd, 5); WrStrLn(" x y have fd"); */
      if (srtm30fd[xi][yi].havefile) return srtm30fd[xi][yi].fd;
      s[6U] = '0';
      s[7U] = '.';
      s[8U] = 'D';
      s[9U] = 'E';
      s[10U] = 'M';
      s[11U] = 0;
      aprsstr_Append(path, 1025ul, s, 21ul);
      f = osi_OpenRead(path, 1025ul);
      srtm30fd[xi][yi].fd = f;
      srtm30fd[xi][yi].havefile = 1;
      return f;
   }
   return 0;
} /* end opensrtm() */

#define libsrtm_PURGEALL 10


static void purgesrtm(char all)
{
   uint32_t destmem;
   uint32_t asize;
   uint32_t y;
   uint32_t x;
   uint32_t yd;
   uint32_t xd;
   pSRTMTILE pt;
   pSRTMLAT pl;
   pSRTMSTRIP pb;
   char empty;
   struct SRTMTILE * anonym;
   destmem = libsrtm_srtmmaxmem-libsrtm_srtmmaxmem/10UL;
   for (xd = 0UL; xd<=359UL; xd++) {
      pl = srtmcache[xd];
      if (pl) {
         for (yd = 0UL; yd<=179UL; yd++) {
            empty = 1;
            pt = pl[yd];
            if (pt && pt!=srtmmiss) {
               { /* with */
                  struct SRTMTILE * anonym = pt;
                  for (y = 0UL; y<=3599UL; y++) {
                     for (x = 0UL; x<=2UL; x++) {
                        pb = anonym->strips[x][y];
                        if (pb) {
                           if (!all && anonym->used[x][y]>0U) {
                              --anonym->used[x][y]; /* increase age level */
                              empty = 0;
                           }
                           else {
                              asize = 2400UL;
                              if (anonym->typ>3U) {
                                 asize = 240UL;
                              }
                              osic_free((char * *) &pb, asize);
                              libsrtm_srtmmem -= asize;
                              anonym->strips[x][y] = 0;
                              if ((!all && !empty)
                && libsrtm_srtmmem<=destmem) return;
                           }
                        }
                     } /* end for */
                  } /* end for */
                  /* purged enough now */
                  if (all || empty) {
                     if (anonym->typ<=3U && anonym->fd!=-1L) {
                        osic_Close(anonym->fd);
                     }
                     osic_free((char * *) &pt, sizeof(struct SRTMTILE));
                     libsrtm_srtmmem -= sizeof(struct SRTMTILE);
                     pl[yd] = 0;
                  }
               }
            }
         } /* end for */
         if (all) {
            osic_free((char * *) &pl, sizeof(SRTMLAT));
            libsrtm_srtmmem -= sizeof(SRTMLAT);
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


static int32_t getsrtm1(uint32_t ilat, uint32_t ilong,
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
   *div0 = 1UL;
   ydeg = ilat/3600UL;
   xdeg = ilong/3600UL;
   if (xdeg>359UL || ydeg>179UL) return 32767L;
   if (srtmcache[xdeg]==0) {
      /* empty lat array */
      osic_alloc((char * *) &srtmcache[xdeg], sizeof(SRTMLAT));
      if (srtmcache[xdeg]==0) return 32767L;
      /* out of memory */
      libsrtm_srtmmem += sizeof(SRTMLAT);
      memset((char *)srtmcache[xdeg],(char)0,sizeof(SRTMLAT));
   }
   else if (srtmcache[xdeg][ydeg]==srtmmiss) return 32767L;
   /* tile file not avaliable */
   pt = srtmcache[xdeg][ydeg];
   if (pt==0) {
      t = 1U;
      f = libsrtm_opensrtm(1U, ydeg, xdeg);
      if (f==-1L) {
         t = 3U;
         f = libsrtm_opensrtm(3U, ydeg, xdeg);
         if (f==-1L) {
            t = 30U;
            f = libsrtm_opensrtm(30U, ydeg, xdeg);
            if (f==-1L) {
               srtmcache[xdeg][ydeg] = srtmmiss;
               return 32767L;
            }
         }
      }
      /*INC(open); */
      osic_alloc((char * *) &pt, sizeof(struct SRTMTILE));
                /* a new 1x1 deg buffer */
      if (pt==0) return 32767L;
      libsrtm_srtmmem += sizeof(struct SRTMTILE);
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
            seek = (int32_t)((xdeg%40UL)*240UL)-(int32_t)((ilat/30UL)
                *9600UL);
            if (ydeg>=130UL) seek += 207350400L;
            else if (ydeg>=80UL) seek += 149750400L;
            else if (ydeg>=30UL) seek += 92150400L;
            else return 32767L;
            rdsize = 240UL; /* fill 1/10 buffer */
         }
         /*INC(miss); */
         osic_alloc((char * *) &pb, rdsize);
         if (pb==0) return 32767L;
         libsrtm_srtmmem += rdsize;
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
      /*    IF used[xx][y]<AGELEVELS THEN INC(used[xx][y]) END; */
      anonym0->used[xx][y] = 5U;
      a = (int32_t)pb[x];
      if (a>32000L || a<-32000L) return 32767L;
      return a;
   }
} /* end getsrtm1() */


static float qint(float a, float b, float c, float v)
/* spline interpolator */
{
   a = (a-b)*0.5f;
   c = (c-b)*0.5f;
   return b+(v*v+0.25f)*(a+c)+v*(c-a);
} /* end qint() */


static float qintd(float a, float b, float c, float v)
/* spline interpolator and normalvector */
{
   return v*((a+c)-b*2.0f)+(c-a)*0.5f;
} /* end qintd() */

/*
PROCEDURE qintd(a,b,c,v:REAL; VAR nv:REAL):REAL;
                (* spline interpolator and normalvector *)
VAR ca, ac:REAL;
BEGIN
  a:=(a-b)*0.5;
  c:=(c-b)*0.5;
  ca:=c-a;
  ac:=a+c;
  nv:=2.0*v*ac + ca;
  RETURN b + (v*v+0.25)*ac + v*ca
END qintd;
*/

static uint32_t chkmeta(int32_t * a)
{
   return (uint32_t)(*a+41000L)/10000UL;
} /* end chkmeta() */


static float resmeta(int32_t a)
{
   return (float)((int32_t)((uint32_t)(a+41000L)%10000UL)-1000L);
} /* end resmeta() */


static float am(uint32_t ilat, uint32_t ilong)
{
   uint32_t d;
   int32_t a;
   a = getsrtm1(ilat, ilong, &d);
   if (a>=32767L) errflag = 1;
   return resmeta(a);
} /* end am() */


static float int4(float a0, float a1, float a2, float a3,
                float vx, float vy)
{
   return (a1*vx+a0*(1.0f-vx))*(1.0f-vy)+(a3*vx+a2*(1.0f-vx))*vy;
} /* end int4() */
/*BEGIN RETURN (a0*(1.0-vx) + a1*vx)*(1.0-vy) + (a2*(1.0-vx) + a3*vx)
                *vy END int4; */


extern float libsrtm_getsrtmlong(double lat, double long0,
                uint32_t quality, char bicubic,
                float * resolution, uint8_t * att0,
                libsrtm_pMETAINFO pmeta)
{
   uint32_t d;
   uint32_t div2;
   uint32_t div0;
   uint32_t ilong;
   uint32_t ilat;
   uint32_t ilongdd;
   uint32_t ilongd;
   uint32_t ilatdd;
   uint32_t ilatd;
   float a8;
   float a7;
   float a6;
   float a5;
   float a4;
   float a3;
   float a2;
   float a1;
   float a0;
   int32_t i3;
   int32_t i2;
   int32_t i1;
   int32_t i0;
   float vy5;
   float vx5;
   float vy;
   float vx;
   float dx2;
   float dx1;
   float dx0;
   uint8_t att;
   /* want attibute interpolation */
   struct libsrtm_METAINFO * anonym;
   if (libsrtm_srtmmaxmem>0UL && libsrtm_srtmmem>libsrtm_srtmmaxmem) {
      purgesrtm(0);
   }
   lat = 3.24E+5+lat*2.0626480625299E+5;
   long0 = 6.48E+5+long0*2.0626480625299E+5;
   if (lat>=0.0) ilat = (uint32_t)X2C_TRUNCC(lat,0UL,X2C_max_longcard);
   else ilat = 0UL;
   if (long0>=0.0) {
      ilong = (uint32_t)X2C_TRUNCC(long0,0UL,X2C_max_longcard);
   }
   else ilong = 0UL;
   i0 = getsrtm1(ilat, ilong, &div0);
   *att0 = (uint8_t)chkmeta(&i0);
   /*
     IF pmeta<>NIL THEN
       pmeta^.slantx:=0.0;
       pmeta^.slanty:=0.0;
       FILL(ADR(pmeta^.attrweights[0]), 0C, SIZE(pmeta^.attrweights));
     END;
   */
   if (i0>=32767L) return 32767.0f;
   a0 = resmeta(i0);
   if (pmeta) quality = 0UL;
   if (div0==1UL) {
      *resolution = 30.0f;
      if (quality>29UL) return a0;
      vx = (float)(long0-(double)ilong);
      vy = (float)(lat-(double)ilat);
   }
   else if (div0==3UL) {
      *resolution = 90.0f;
      if (quality>60UL) return a0;
      vx = (float)((long0-(double)((ilong/3UL)*3UL))
                *3.3333333333333E-1);
      vy = (float)((lat-(double)((ilat/3UL)*3UL))
                *3.3333333333333E-1);
   }
   else {
      *resolution = 900.0f;
      if (quality>300UL) return a0;
      vx = (float)((long0-(double)((ilong/30UL)*30UL))
                *3.3333333333333E-2);
      vy = (float)((lat-(double)((ilat/30UL)*30UL))
                *3.3333333333333E-2);
   }
   if (vx<0.5f) {
      if (ilong>=div0) ilong -= div0;
   }
   else vx = vx-1.0f;
   if (vy<0.5f) ilat -= div0;
   else vy = vy-1.0f;
   vx5 = vx+0.5f;
   vy5 = vy+0.5f;
   ilatd = ilat+div0;
   ilongd = ilong+div0;
   i0 = getsrtm1(ilat, ilong, &d);
   i1 = getsrtm1(ilat, ilongd, &d);
   i2 = getsrtm1(ilatd, ilong, &d);
   i3 = getsrtm1(ilatd, ilongd, &d);
   if (((i0>=32767L || i1>=32767L) || i2>=32767L) || i3>=32767L) return a0;
   if (pmeta && pmeta->aliasattr) {
      { /* with */
         struct libsrtm_METAINFO * anonym = pmeta;
         memset((char *) &anonym->attrweights[0U],(char)0,
                sizeof(float [8]));
         att = (uint8_t)chkmeta(&i0);
         anonym->attrweights[att] = anonym->attrweights[att]+(1.0f-vx5)
                *(1.0f-vy5);
         att = (uint8_t)chkmeta(&i1);
         anonym->attrweights[att] = anonym->attrweights[att]+vx5*(1.0f-vy5);
         att = (uint8_t)chkmeta(&i2);
         anonym->attrweights[att] = anonym->attrweights[att]+(1.0f-vx5)*vy5;
         att = (uint8_t)chkmeta(&i3);
         anonym->attrweights[att] = anonym->attrweights[att]+vx5*vy5;
      }
   }
   a0 = resmeta(i0);
   a1 = resmeta(i1);
   a2 = resmeta(i2);
   a3 = resmeta(i3);
   /*interpolate 4 dots */
   if (!bicubic) {
      /* bilinear */
      if (pmeta && pmeta->withslant) {
         /* bilinear slants */
         pmeta->slantx = X2C_DIVR((a1-a0)*(1.0f-vy5)+(a3-a2)*vy5,
                *resolution);
         pmeta->slanty = X2C_DIVR((a2-a0)*(1.0f-vx5)+(a3-a1)*vx5,
                *resolution);
      }
      return int4(a0, a1, a2, a3, vx5, vy5);
   }
   else {
      /* wighted median of 4 pixels*/
      /* biqubic */
      /*    IF vx<0.5 THEN IF ilong>=div THEN DEC(ilong, div) END; */
      /*    ELSE vx:=vx-1.0 END; */
      /*    IF vy<0.5 THEN DEC(ilat,  div) ELSE vy:=vy-1.0 END; */
      div2 = div0*2UL;
      ilatdd = ilat+div2;
      ilongdd = ilong+div2;
      errflag = 0;
      if (pmeta && pmeta->withslant) {
         /* cubic interpolate and slants */
         a4 = am(ilat, ilongdd);
         a5 = am(ilatd, ilongdd);
         a6 = am(ilatdd, ilong);
         a7 = am(ilatdd, ilongd);
         a8 = am(ilatdd, ilongdd);
         dx0 = qint(a0, a1, a4, vx);
         dx1 = qint(a2, a3, a5, vx);
         dx2 = qint(a6, a7, a8, vx);
         pmeta->slanty = X2C_DIVR(qintd(dx0, dx1, dx2, vy),*resolution);
         dx0 = qint(a0, a2, a6, vy);
         dx1 = qint(a1, a3, a7, vy);
         dx2 = qint(a4, a5, a8, vy);
         pmeta->slantx = X2C_DIVR(qintd(dx0, dx1, dx2, vx),*resolution);
         a1 = qint(dx0, dx1, dx2, vx);
         if (errflag) return a0;
         else return a1;
      }
      else {
         a1 = qint(qint(a0, a1, am(ilat, ilongdd), vx), qint(a2, a3,
                am(ilatd, ilongdd), vx), qint(am(ilatdd, ilong), am(ilatdd,
                ilongd), am(ilatdd, ilongdd), vx), vy);
         if (errflag) return a0;
         else return a1;
      }
   }
   return 0;
} /* end getsrtmlong() */

/*
PROCEDURE getsrtm(pos:POSITION; quality:CARDINAL; VAR resolution:REAL):REAL;
VAR attr:CARD8;
BEGIN RETURN getsrtmlong(pos.lat, pos.long, quality, FALSE, resolution, attr,
                 NIL) END getsrtm;
*/
/*
PROCEDURE getsrtm(pos:POSITION; quality:CARDINAL; VAR resolution:REAL):REAL;
VAR attr:CARD8;
    a:REAL;
BEGIN
  a:=getsrtmlong(pos.lat, pos.long, quality, bicubic, resolution, attr, NIL);
  IF attr=3 THEN a:=32000.0 END;
  RETURN a
END getsrtm;
*/

extern float libsrtm_getsrtm(struct aprsstr_POSITION pos,
                uint32_t quality, float * resolution)
{
   uint8_t attr;
   float a;
   struct libsrtm_METAINFO at;
   at.withslant = 1;
   at.aliasattr = 1;
   a = libsrtm_getsrtmlong((double)pos.lat, (double)pos.long0,
                quality, 0, resolution, &attr, &at);
   /*  IF (attr=3) OR (at.attr2=4) & ((attr=3) = (at.attrweight<0.5))
                THEN a:=32000.0 END; */
   /*IF attr=2 THEN a:=32000.0 END; */
   /*  IF at.attrweights[2]>0.5 THEN a:=32000.0 END; */
   /*  a:=1000.0+at.slanty*2.0; */
   return a;
} /* end getsrtm() */


extern void libsrtm_initsrtm(void)
{
   uint32_t yi;
   uint32_t xi;
   /*WrStrLn(" inits"); */
   /*open:=0; miss:=0; hit:=0; */
   memset((char *)srtmcache,(char)0,sizeof(SRTMLONG));
   for (xi = 0UL; xi<=8UL; xi++) {
      for (yi = 0UL; yi<=3UL; yi++) {
         srtm30fd[xi][yi].havefile = 0;
      } /* end for */
   } /* end for */
} /* end initsrtm() */


extern void libsrtm_closesrtmfile(void)
{
   /*WrStrLn(" closes"); */
   purgesrtm(1);
   /*WrInt(open, 10); WrInt(miss, 10); WrInt(hit, 10);
                WrStrLn(" open miss hit"); */
   libsrtm_initsrtm();
} /* end closesrtmfile() */

/* === srtm lib */
/* === geoid lib */
#define libsrtm_GEOIDFN "WW15MGH.DAC"

#define libsrtm_RESOL 4
/* 1/deg */

#define libsrtm_LONGS 1440
/* values in file around a latitude */


static float rdgeoid(int32_t fd, int32_t lat, int32_t long0,
                char * ok0)
{
   char b[2];
   int32_t n;
   osic_Seek(fd, (uint32_t)((lat*1440L+long0)*2L));
   if (osi_RdBin(fd, (char *)b, 2u/1u, 2UL)!=2L) {
      n = 0L; /* no data in file */
      n = 0L;
      *ok0 = 0;
   }
   else {
      n = (int32_t)((uint32_t)(uint8_t)b[1U]+(uint32_t)(uint8_t)
                b[0U]*256UL);
      if (n>=32768L) n -= 65536L;
   }
   return (float)n;
} /* end rdgeoid() */


extern float libsrtm_egm96(struct aprsstr_POSITION pos,
                char * ok0)
/* read and interpolate geoid correction from datafile */
{
   int32_t ilong;
   int32_t ilat;
   int32_t fd;
   float g;
   float flong;
   float flat;
   char path[1025];
   aprsstr_Assign(path, 1025ul, libsrtm_srtmdir, 1024ul);
   if (path[0]) aprsstr_Append(path, 1025ul, "/", 2ul);
   aprsstr_Append(path, 1025ul, "WW15MGH.DAC", 12ul);
   fd = osi_OpenRead(path, 1025ul);
   *ok0 = fd!=-1L;
   g = 0.0f;
   if (*ok0) {
      pos.lat = 90.0f-pos.lat*5.729577951472E+1f;
      pos.long0 = pos.long0*5.729577951472E+1f;
      if (pos.long0<0.0f) pos.long0 = 360.0f+pos.long0;
      pos.lat = pos.lat*4.0f;
      pos.long0 = pos.long0*4.0f;
      if (pos.lat>=0.0f) {
         ilat = (int32_t)(uint32_t)X2C_TRUNCC(pos.lat,0UL,
                X2C_max_longcard);
      }
      else ilat = 0L;
      if (pos.long0>=0.0f) {
         ilong = (int32_t)(uint32_t)X2C_TRUNCC(pos.long0,0UL,
                X2C_max_longcard);
      }
      else ilong = 0L;
      flat = pos.lat-(float)ilat;
      flong = pos.long0-(float)ilong;
      g = ((rdgeoid(fd, ilat, ilong, ok0)*(1.0f-flat)+rdgeoid(fd, ilat+1L,
                ilong, ok0)*flat)*(1.0f-flong)+(rdgeoid(fd, ilat, ilong+1L,
                ok0)*(1.0f-flat)+rdgeoid(fd, ilat+1L, ilong+1L,
                ok0)*flat)*flong)*0.01f;
      osic_Close(fd);
   }
   return g;
} /* end egm96() */


extern void libsrtm_BEGIN(void)
{
   static int libsrtm_init = 0;
   if (libsrtm_init) return;
   libsrtm_init = 1;
   aprspos_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   osic_alloc((char * *) &srtmmiss, 1UL);
                /* make empty tile for fast nofile hint */
   libsrtm_initsrtm();
   libsrtm_srtmmem = 0UL;
   libsrtm_srtmmaxmem = 0UL; /* 0 no auto purge */
   libsrtm_srtmdir[0] = 0;
   libsrtm_bicubic = 0;
}

