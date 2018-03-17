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
/* get altitude out of srtm files directory tree by oe5dxl */
#define libsrtm_SRTMXY 3600

#define libsrtm_STRIPS 3

#define libsrtm_AGELEVELS 5

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

#define libsrtm_NOALT0 32767.0


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
   *div0 = 1UL;
   ydeg = ilat/3600UL;
   xdeg = ilong/3600UL;
   if (xdeg>359UL || ydeg>179UL) return 32767.0f;
   if (srtmcache[xdeg]==0) {
      /* empty lat array */
      osic_alloc((char * *) &srtmcache[xdeg], sizeof(SRTMLAT));
      if (srtmcache[xdeg]==0) return 32767.0f;
      /* out of memory */
      libsrtm_srtmmem += sizeof(SRTMLAT);
      memset((char *)srtmcache[xdeg],(char)0,sizeof(SRTMLAT));
   }
   else if (srtmcache[xdeg][ydeg]==srtmmiss) return 32767.0f;
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
               return 32767.0f;
            }
         }
      }
      /*INC(open); */
      osic_alloc((char * *) &pt, sizeof(struct SRTMTILE));
                /* a new 1x1 deg buffer */
      if (pt==0) return 32767.0f;
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
            else return 32767.0f;
            rdsize = 240UL; /* fill 1/10 buffer */
         }
         /*INC(miss); */
         osic_alloc((char * *) &pb, rdsize);
         if (pb==0) return 32767.0f;
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
      if (a>30000L || a<-30000L) return 32767.0f;
      return (float)a;
   }
} /* end getsrtm1() */

#define libsrtm_NOALT 32767.0

#define libsrtm_ERRALT 30000.0


extern float libsrtm_getsrtm(struct aprsstr_POSITION pos,
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
   if (libsrtm_srtmmaxmem>0UL && libsrtm_srtmmem>libsrtm_srtmmaxmem) {
      /*WrInt(srtmmem, 15); */
      purgesrtm(0);
   }
   /*WrInt(srtmmem, 15);WrStrLn(" purged");  */
   pos.lat = 3.24E+5f+pos.lat*2.0626480625299E+5f;
   pos.long0 = 6.48E+5f+pos.long0*2.0626480625299E+5f;
   if (pos.lat>=0.0f) {
      ilat = (uint32_t)X2C_TRUNCC(pos.lat,0UL,X2C_max_longcard);
   }
   else ilat = 0UL;
   if (pos.long0>=0.0f) {
      ilong = (uint32_t)X2C_TRUNCC(pos.long0,0UL,X2C_max_longcard);
   }
   else ilong = 0UL;
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
}

