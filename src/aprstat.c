/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef aprstat_H_
#include "aprstat.h"
#endif
#define aprstat_C_
#ifndef maptool_H_
#include "maptool.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef useri_H_
#include "useri.h"
#endif
#ifndef aprstext_H_
#include "aprstext.h"
#endif



/* aprs statistc graphs by oe5dxl */
#define aprstat_KMHTIME 600


static void setpix(maptool_pIMAGE img, long x, long y, long rr, long gg,
                long bb)
{
   struct maptool_PIX * anonym;
   { /* with */
      struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+y];
      anonym->r = (unsigned short)rr;
      anonym->g = (unsigned short)gg;
      anonym->b = (unsigned short)bb;
   }
} /* end setpix() */


static void str(maptool_pIMAGE img, unsigned long x, unsigned long y,
                char s[], unsigned long s_len)
{
   unsigned long i;
   unsigned long l;
   long inc;
   struct aprsdecode_COLTYP col;
   X2C_PCOPY((void **)&s,s_len);
   l = aprsstr_Length(s, s_len);
   i = 0UL;
   maptool_Colset(&col, 'W');
   while (i<l) {
      maptool_drawchar(img, s[i], (float)x, (float)y, &inc, 700UL, 1UL, col,
                0);
      x += (unsigned long)inc;
      ++i;
   }
   X2C_PFREE(s);
} /* end str() */


static void num(maptool_pIMAGE img, long x, long y, long n, char h[],
                unsigned long h_len)
{
   char s[51];
   X2C_PCOPY((void **)&h,h_len);
   aprsstr_IntToStr(n, 1UL, s, 51ul);
   aprsstr_Append(s, 51ul, h, h_len);
   str(img, (unsigned long)x, (unsigned long)y, s, 51ul);
   X2C_PFREE(h);
} /* end num() */


static float sq(float x)
{
   return x*x;
} /* end sq() */


static unsigned long pixl(float x, float y)
{
   x = x*x+y*y;
   if (x<=0.0f) return 0UL;
   return aprsdecode_trunc(256.0f*RealMath_sqrt(x));
} /* end pixl() */
/*
BEGIN RETURN TRUNC(256.0*(ABS(x) + ABS(y))) END pixl;
*/

#define aprstat_KL 512


static void addpix(maptool_pIMAGE img, float x, float y, unsigned long rr,
                unsigned long gg, unsigned long bb)
{
   float fy;
   float fx;
   unsigned long l;
   unsigned long yy;
   unsigned long xx;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   xx = aprsdecode_trunc(x);
   yy = aprsdecode_trunc(y);
   fx = x-(float)xx;
   fy = y-(float)yy;
   l = pixl(1.0f-fx, 1.0f-fy);
   { /* with */
      struct maptool_PIX * anonym = &img->Adr[(xx)*img->Len0+yy];
      anonym->r += (unsigned short)((rr*l)/512UL);
      anonym->g += (unsigned short)((gg*l)/512UL);
      anonym->b += (unsigned short)((bb*l)/512UL);
   }
   l = pixl(fx, 1.0f-fy);
   { /* with */
      struct maptool_PIX * anonym0 = &img->Adr[(xx+1UL)*img->Len0+yy];
      anonym0->r += (unsigned short)((rr*l)/512UL);
      anonym0->g += (unsigned short)((gg*l)/512UL);
      anonym0->b += (unsigned short)((bb*l)/512UL);
   }
   l = pixl(1.0f-fx, fy);
   { /* with */
      struct maptool_PIX * anonym1 = &img->Adr[(xx)*img->Len0+(yy+1UL)];
      anonym1->r += (unsigned short)((rr*l)/512UL);
      anonym1->g += (unsigned short)((gg*l)/512UL);
      anonym1->b += (unsigned short)((bb*l)/512UL);
   }
   l = pixl(fx, fy);
   { /* with */
      struct maptool_PIX * anonym2 = &img->Adr[(xx+1UL)*img->Len0+(yy+1UL)];
      anonym2->r += (unsigned short)((rr*l)/512UL);
      anonym2->g += (unsigned short)((gg*l)/512UL);
      anonym2->b += (unsigned short)((bb*l)/512UL);
   }
} /* end addpix() */


static unsigned long sfact(float y)
{
   unsigned long m;
   unsigned long s;
   s = 1UL;
   m = 2UL;
   while (X2C_DIVR(y,(float)s)>X2C_DIVR(200.0f,
                (float)(aprsdecode_lums.fontysize+2UL))) {
      s = s*m;
      if (m==2UL) m = 5UL;
      else m = 2UL;
   }
   return s;
} /* end sfact() */


static unsigned long dynmaxx(unsigned long margin, unsigned long min0,
                unsigned long max0)
{
   if (max0+margin*2UL>(unsigned long)maptool_xsize) {
      if ((unsigned long)maptool_xsize>min0) {
         max0 = (unsigned long)maptool_xsize-margin*2UL;
      }
      else max0 = min0-margin*2UL;
   }
   return max0;
} /* end dynmaxx() */

#define aprstat_MAXT 7200

#define aprstat_X 620

#define aprstat_Y 150

#define aprstat_MARGIN1 16

#define aprstat_XI 636

#define aprstat_YI 166


extern void aprstat_btimehist(maptool_pIMAGE * img, aprsdecode_pOPHIST op)
{
   unsigned long t[7200];
   aprsdecode_pFRAMEHIST pf;
   unsigned long textd;
   unsigned long textx;
   unsigned long max0;
   unsigned long expand;
   unsigned long scale0;
   unsigned long xm;
   unsigned long sum;
   unsigned long y;
   unsigned long i;
   unsigned long dt;
   char s[256];
   unsigned long tmp;
   size_t tmp0[2];
   if (op==0) return;
   for (i = 0UL; i<=7199UL; i++) {
      t[i] = 0UL;
   } /* end for */
   pf = op->frames;
   sum = 0UL;
   if (pf) {
      while (pf->next) {
         if (maptool_vistime(pf->time0) && pf->next->time0>=pf->time0) {
            dt = pf->next->time0-pf->time0;
            if (dt<=7199UL) {
               ++t[dt];
               ++sum;
            }
         }
         pf = pf->next;
      }
   }
   sum -= sum/20UL;
   xm = 0UL;
   y = 0UL;
   do {
      y += t[xm];
      ++xm;
   } while (!(xm>=7199UL || y>=sum));
   ++xm;
   if (xm<120UL) xm = 120UL;
   for (i = xm+1UL; i<=7199UL; i++) {
      t[xm] += t[i]; /* add not shown rest to last shown element */
   } /* end for */
   scale0 = ((xm+620UL)-1UL)/620UL;
   textd = 50UL;
   if (scale0>30UL) {
      scale0 = 60UL;
      textx = 50UL;
   }
   else if (scale0>12UL) {
      scale0 = 30UL;
      textx = 20UL;
   }
   else if (scale0>6UL) {
      scale0 = 12UL;
      textx = 10UL;
   }
   else if (scale0>3UL) {
      scale0 = 6UL;
      textx = 5UL;
   }
   else if (scale0>2UL) {
      scale0 = 3UL;
      textx = 3UL;
      textd = 60UL;
   }
   else if (scale0>1UL) {
      scale0 = 2UL;
      textx = 2UL;
      textd = 60UL;
   }
   else {
      scale0 = 1UL;
      textx = 1UL;
      textd = 60UL;
   }
   if (scale0>1UL) {
      for (i = 1UL; i<=7199UL; i++) {
         if (i%scale0==0UL) t[i/scale0] = 0UL;
         t[i/scale0] += t[i];
      } /* end for */
   }
   xm = ((xm+scale0)-1UL)/scale0;
   expand = 1UL;
   if (xm<310UL) expand = 2UL;
   else if (xm>620UL) xm = 620UL;
   else if (xm==0UL) return;
   xm = xm*expand;
   max0 = 0UL;
   tmp = xm-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if (t[i]>max0) max0 = t[i];
      if (i==tmp) break;
   } /* end for */
   if (max0==0UL) return;
   tmp = xm-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      t[i] = (t[i]*150UL)/max0;
      if (i==tmp) break;
   } /* end for */
   X2C_DYNALLOCATE((char **)img,sizeof(struct maptool_PIX),
                (tmp0[0] = xm+16UL,tmp0[1] = 166U,tmp0),2u);
   useri_debugmem.screens += (*img)->Len1*(*img)->Size1;
   if (*img==0) {
      osi_WrStrLn("error image alloc", 18ul);
      return;
   }
   maptool_clr(*img);
   tmp = xm-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      setpix(*img, (long)(i+8UL), 7L, 0L, 500L, 800L);
      if ((i/expand)%10UL==0UL) {
         setpix(*img, (long)(i+8UL), 6L, 0L, 500L, 800L);
      }
      if ((i/expand)%textd==0UL) {
         setpix(*img, (long)(i+8UL), 5L, 0L, 500L, 800L);
         setpix(*img, (long)(i+8UL), 4L, 0L, 500L, 800L);
      }
      for (y = 0UL; y<=149UL; y++) {
         setpix(*img, (long)(i+8UL), (long)(y+8UL), 50L,
                (long)(20UL*(unsigned long)(char)((i/expand)/10UL&1)+30UL),
                30L);
      } /* end for */
      y = 0UL;
      while (y<t[i/expand]) {
         setpix(*img, (long)(i+8UL), (long)(y+8UL), 500L, 500L, 0L);
         ++y;
      }
      if (i==tmp) break;
   } /* end for */
   for (i = 0UL; i<=149UL; i++) {
      setpix(*img, 7L, (long)((i+8UL)-1UL), 0L, 500L, 800L);
      if (i%10UL==0UL) setpix(*img, 6L, (long)((i+8UL)-1UL), 0L, 500L, 800L);
   } /* end for */
   strncpy(s," Beacons per Time ",256u);
   aprsstr_Append(s, 256ul, op->call, 9ul);
   num(*img, 10L, (long)((166UL-aprsdecode_lums.fontysize)-1UL), (long)max0,
                s, 256ul);
   num(*img, (long)((expand*textd+8UL)-12UL), 8L, (long)textx, "m", 2ul);
/*
  DISPOSE(img);
*/
} /* end btimehist() */

#define aprstat_MAXX 1584

#define aprstat_MINX 584

#define aprstat_MAXY 120

#define aprstat_MARGIN 8

#define aprstat_XSTEPS 8
/* in 1 pixel */

#define aprstat_FILTERLEN 6
/* median filter span in pixels */

#define aprstat_NOINFOTIME 300
/* seconds timeout to no info */

#define aprstat_MINSPAN 3600

#define aprstat_MAXSPAN 28800


extern void aprstat_kmhist(maptool_pIMAGE * img, aprsdecode_pOPHIST op,
                char * test)
{
   aprsdecode_pFRAMEHIST fold;
   aprsdecode_pFRAMEHIST fto;
   aprsdecode_pFRAMEHIST ffrom;
   aprsdecode_pFRAMEHIST fr;
   unsigned long ot;
   unsigned long t1;
   unsigned long t0;
   unsigned long markkm;
   unsigned long markx;
   unsigned long maxx;
   unsigned long maxkm10;
   unsigned long maxkm;
   unsigned long tf;
   unsigned long sp;
   unsigned long i;
   unsigned long xi;
   unsigned long xt;
   float ys;
   float dy;
   float maxy;
   char h[256];
   char s[256];
   struct aprspos_POSITION po;
   float vt[12720];
   char marks[1584];
   unsigned long tops[5];
   struct aprsdecode_DAT dat;
   unsigned long tmp;
   size_t tmp0[2];
   if (op==0 || op->frames==0) {
      *test = 0;
      return;
   }
   fto = 0;
   ffrom = 0;
   fr = op->frames;
   po = fr->vardat->pos;
   fold = 0;
   do {
      /* find first to last move */
      if (((maptool_vistime(fr->time0) && !(fr->nodraw&~0x1U))
                && aprspos_posvalid(fr->vardat->pos)) && aprspos_distance(po,
                 fr->vardat->pos)>0.05f) {
         /* find last move */
         fto = fr;
         po = fr->vardat->pos;
         if (ffrom==0) ffrom = fold;
         fold = fr;
      }
      fr = fr->next;
   } while (fr);
   if (fto==0 || ffrom==0) {
      *test = 0; /* no move */
      return;
   }
   if (*test) return;
   *img = 0;
   for (xi = 0UL; xi<=1583UL; xi++) {
      marks[xi] = 0;
   } /* end for */
   /*
     IF ffrom=NIL THEN ffrom:=op^.frames END;
   */
   t1 = fto->time0;
   while (ffrom->next && (!(~ffrom->nodraw&0x1U) || ffrom->time0+28800UL<t1))
                 ffrom = ffrom->next;
   t0 = ffrom->time0;
   if (t1<t0+3600UL) {
      t1 = t0+3600UL;
      fto = 0;
   }
   /* find peak speeds to filter extrem values */
   for (i = 0UL; i<=4UL; i++) {
      tops[i] = 0UL;
   } /* end for */
   fr = ffrom;
   for (;;) {
      if ((!(fr->nodraw&~0x1U) && aprsdecode_Decode(fr->vardat->raw, 500ul,
                &dat)>=0L) && dat.speed<X2C_max_longcard) {
         i = 0UL;
         for (;;) {
            if (dat.speed>=tops[i]) {
               tops[i] = dat.speed;
               break;
            }
            ++i;
            if (i>4UL) break;
         }
      }
      if (fr==fto) break;
      fr = fr->next;
      if (fr==0) break;
   }
   /*FOR i:=0 TO HIGH(tops) DO WrInt(tops[i], 10) END; */
   while ((float)tops[0U]>(float)tops[1U]*1.5f) {
      for (i = 0UL; i<=3UL; i++) {
         tops[i] = tops[i+1UL];
      } /* end for */
   }
   markx = 0UL;
   markkm = 0UL;
   maxx = (1584UL*((t1-t0)+3600UL))/28800UL; /* guess picture width */
   if (maxx<584UL) maxx = 584UL;
   else if (maxx>1584UL) maxx = 1584UL;
   ot = t0;
   fr = ffrom;
   xi = 0UL;
   maxy = 0.0f;
   dy = 0.0f;
   tf = 0UL;
   for (;;) {
      if ((!(fr->nodraw&~0x1U) && aprsdecode_Decode(fr->vardat->raw, 500ul,
                &dat)>=0L) && (!useri_configon(useri_fTRACKFILT)
                || dat.speed<tops[0U])) {
         dy = (float)aprsdecode_knottokmh((long)dat.speed)+0.5f;
         if (dy>1000.0f) dy = 1000.0f;
         if (fr->time0>ot+300UL) tf = 48UL;
         xt = aprsdecode_trunc((X2C_DIVR((float)(fr->time0-t0),
                (float)(t1-t0)))*(float)(maxx*8UL-1UL));
                /* x position in picture */
         if (xt>=maxx*8UL-1UL) xt = maxx*8UL;
         if (xi/8UL>3UL) marks[xi/8UL-3UL] = 1;
         if (dy>maxy) maxy = dy;
         while (xi<xt) {
            /* constant speed till next waypoint */
            vt[xi] = dy;
            ++xi;
            if (tf>0UL) {
               --tf;
               if (tf==0UL) dy = 0.0f;
            }
         }
         ot = fr->time0;
         if ((dat.pos.lat==aprsdecode_click.markpos.lat && dat.pos.long0==aprsdecode_click.markpos.long0)
                 && aprsdecode_click.markpost==ot) {
            markx = xi/8UL;
            markkm = (unsigned long)aprsdecode_knottokmh((long)dat.speed);
         }
      }
      if (fr==fto) break;
      fr = fr->next;
      if (fr==0) break;
   }
   tf = 48UL;
   while (xi<=12719UL) {
      vt[xi] = dy;
      ++xi;
      if (tf>0UL) --tf;
      else dy = 0.0f;
   }
   tmp = maxx*8UL-1UL;
   xi = 0UL;
   if (xi<=tmp) for (;; xi++) {
      dy = 0.0f;
      for (xt = 0UL; xt<=47UL; xt++) {
         dy = dy+vt[xi+xt]*(1.0f-sq((float)xt*4.1666666666667E-2f-1.0f));
                /* FIR lowpass */
      } /* end for */
      vt[xi] = dy;
      if (xi==tmp) break;
   } /* end for */
   maxkm = aprsdecode_trunc(maxy);
   maxkm10 = ((maxkm+9UL)/10UL)*10UL;
   if (maxy>=2.0f) {
      maxy = X2C_DIVR(3.75f,(float)maxkm10); /* FIR gain integral 1-x^2 */
      for (xi = 0UL; xi<=12719UL; xi++) {
         vt[xi] = vt[xi]*maxy; /* fit y in picture */
      } /* end for */
      X2C_DYNALLOCATE((char **)img,sizeof(struct maptool_PIX),
                (tmp0[0] = maxx+16UL,tmp0[1] = 136U,tmp0),2u);
      useri_debugmem.screens += (*img)->Len1*(*img)->Size1;
      if (*img==0) {
         osi_WrStrLn("error image alloc", 18ul);
         return;
      }
      maptool_clr(*img);
      /*
          FOR xt:=MARGIN TO MAXY+MARGIN-1 DO  
            FOR xi:=MARGIN TO MAXX+MARGIN-1 DO
              WITH img^[xi+xt*(MAXX+2*MARGIN)] DO 
                r:=30; g:=60+50*ORD(ODD(xt DIV 10));
                b:=80+200*ORD(marks[xi-MARGIN]);
              END;
            END;
          END;
      */
      dy = (X2C_DIVR(120.0f,(float)maxkm10))*10.0f;
      ys = 0.0f;
      xt = 0UL;
      sp = 0UL;
      do {
         /* draw y scale and paper and waypoints*/
         for (i = 6UL-2UL*(unsigned long)(sp%5UL==0UL); i<=8UL; i++) {
            setpix(*img, (long)i, (long)(aprsdecode_trunc(ys+0.5f)+8UL),
                200L, 1000L, 200L);
         } /* end for */
         ys = ys+dy;
         while (xt<aprsdecode_trunc(ys) && xt<120UL) {
            tmp = (maxx+8UL)-1UL;
            xi = 8UL;
            if (xi<=tmp) for (;; xi++) {
               setpix(*img, (long)xi, (long)(xt+8UL), 20L,
                (long)(30UL+50UL*(unsigned long)(char)(sp&1)),
                (long)(60UL+200UL*(unsigned long)marks[xi-8UL]));
               if (xi==tmp) break;
            } /* end for */
            ++xt;
         }
         ++sp;
      } while (ys<121.0f);
      tmp = maxx-1UL;
      xi = 0UL;
      if (xi<=tmp) for (;; xi++) {
         setpix(*img, (long)(xi+8UL), 8L, 200L, 1000L, 200L);
         if (xi==tmp) break;
      } /* end for */
      for (xi = 0UL; xi<=120UL; xi++) {
         setpix(*img, 8L, (long)(xi+8UL), 200L, 1000L, 200L);
      } /* end for */
      sp = t0/3600UL+1UL;
      ot = sp*3600UL;
      ys = X2C_DIVR((float)(maxx*(ot-t0)),(float)(t1-t0));
                /* x of first full hour */
      dy = X2C_DIVR((float)(maxx*3600UL),(float)(t1-t0)); /* x size of 1h */
      if (dy>20.0f && ys<=(float)maxx) {
         if (ys-dy*0.5f>0.0f) {
            /* draw 1/2 h mark */
            for (i = 6UL; i<=7UL; i++) {
               setpix(*img, (long)(aprsdecode_trunc(ys-dy*0.5f)+8UL),
                (long)i, 200L, 1000L, 200L);
            } /* end for */
         }
         sp = sp+24UL+useri_localtime()/3600UL;
         do {
            /* draw h and rest of 1/2h marks */
            for (i = 4UL; i<=7UL; i++) {
               setpix(*img, (long)(aprsdecode_trunc(ys)+8UL), (long)i, 200L,
                1000L, 200L);
            } /* end for */
            if (ys+dy*0.5f<=(float)maxx) {
               for (i = 6UL; i<=7UL; i++) {
                  setpix(*img, (long)(aprsdecode_trunc(ys+dy*0.5f)+8UL),
                (long)i, 200L, 1000L, 200L);
               } /* end for */
            }
            num(*img, (long)((aprsdecode_trunc(ys)+8UL)-6UL), 8L,
                (long)(sp%24UL), "h", 2ul);
            ++sp;
            ys = ys+dy;
         } while (ys<=(float)maxx);
      }
      if (markx>3UL) {
         markx -= 3UL;
         for (xi = 8UL; xi<=128UL; xi++) {
            setpix(*img, (long)(markx+8UL), (long)xi, 500L, 200L, 50L);
         } /* end for */
      }
      tmp = maxx*8UL-1UL;
      xi = 8UL;
      if (xi<=tmp) for (;; xi++) {
         /* draw graph */
         dy = vt[xi];
         if (dy>=0.1f) {
            if (dy>120.0f) dy = 120.0f;
            addpix(*img, (float)xi*0.125f+8.0f, dy+8.0f, 125UL, 125UL, 12UL);
         }
         if (xi==tmp) break;
      } /* end for */
      strncpy(s,"  ",256u);
      aprsstr_Append(s, 256ul, op->call, 9ul);
      aprsstr_Append(s, 256ul, " max=", 6ul);
      aprsstr_IntToStr((long)maxkm, 1UL, h, 256ul);
      aprsstr_Append(s, 256ul, h, 256ul);
      if (markx>0UL) {
         aprsstr_Append(s, 256ul, " cursor=", 9ul);
         aprsstr_IntToStr((long)markkm, 1UL, h, 256ul);
         aprsstr_Append(s, 256ul, h, 256ul);
      }
      aprsstr_Append(s, 256ul, "km/h", 5ul);
      num(*img, 10L, (long)((136UL-aprsdecode_lums.fontysize)-1UL),
                (long)maxkm10, s, 256ul);
   }
/*
  IF img<>NIL THEN DISPOSE(img) END;
*/
} /* end kmhist() */


static void paper(maptool_pIMAGE * img, float yax0, float yax1,
                unsigned long step, unsigned long margin, unsigned long maxx,
                 unsigned long maxy, char name[], unsigned long name_len)
{
   unsigned long y;
   unsigned long x;
   long so;
   long s;
   float v;
   struct maptool_PIX * anonym;
   unsigned long tmp;
   unsigned long tmp0;
   X2C_PCOPY((void **)&name,name_len);
   so = (long)X2C_TRUNCI(aprsdecode_floor(X2C_DIVR(yax0,(float)step)),
                X2C_min_longint,X2C_max_longint);
   tmp = maxy-1UL;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      setpix(*img, (long)margin, (long)(y+margin), 200L, 1000L, 200L);
      v = yax0+(yax1-yax0)*(float)y*(X2C_DIVR(1.0f,(float)maxy));
      s = (long)X2C_TRUNCI(aprsdecode_floor(X2C_DIVR(v,(float)step)),
                X2C_min_longint,X2C_max_longint);
      tmp0 = (maxx-1UL)+margin;
      x = margin+1UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &(*img)->Adr[(x)*(*img)
                ->Len0+(y+margin)];
            anonym->r += 60U;
            anonym->g += (unsigned short)(50UL+40UL*(unsigned long)(char)
                (s&1));
            anonym->b += 60U;
         }
         if (x==tmp0) break;
      } /* end for */
      if (s!=so) {
         tmp0 = margin-1UL;
         x = margin-3UL;
         if (x<=tmp0) for (;; x++) {
            setpix(*img, (long)x, (long)(y+margin), 200L, 1000L, 200L);
            if (x==tmp0) break;
         } /* end for */
         num(*img, (long)(margin+1UL), (long)((y+margin)-6UL), s*(long)step, \
"", 1ul);
         so = s;
      }
      if (y==tmp) break;
   } /* end for */
   str(*img, margin+50UL, ((maxy+margin*2UL)-aprsdecode_lums.fontysize)-1UL,
                name, name_len);
   X2C_PFREE(name);
} /* end paper() */

#define aprstat_MAXXX 720

#define aprstat_MAXY0 120

#define aprstat_MARGIN0 8

#define aprstat_XSTEPS0 8
/* in 1 pixel */

#define aprstat_EMPTY (-10000)

#define aprstat_MAXALT 50000

static float aprstat_E = (-1.E+4f);


static void decodealt(float * way, unsigned long * beacons, float wdiv,
                unsigned long * rejs, unsigned long * acks,
                unsigned long * msgs, struct aprsdecode_DAT * dat,
                float alt[5760], float * waysum, aprsdecode_pOPHIST op,
                long * markalt, unsigned long * markx1,
                unsigned long * markx, char do0)
{
   aprsdecode_pFRAMEHIST fr;
   struct aprspos_POSITION opos;
   float a2;
   float a1;
   float a;
   unsigned long xc;
   unsigned long x;
   *markx = 0UL;
   *markx1 = 0UL;
   *markalt = X2C_min_longint;
   fr = op->frames;
   aprsdecode_posinval(&opos);
   *waysum = 0.0f;
   xc = 1UL;
   a = (-1.E+4f);
   a2 = (-1.E+4f);
   for (x = 0UL; x<=5759UL; x++) {
      alt[x] = (-1.E+4f);
   } /* end for */
   do {
      /* sum up km driven */
      if (maptool_vistime(fr->time0) && aprsdecode_Decode(fr->vardat->raw,
                500ul, dat)>=0L) {
         if (dat->type==aprsdecode_MSG) {
            /* count msg stuff */
            if (dat->ackrej==aprsdecode_MSGMSG) ++*msgs;
            else if (dat->ackrej==aprsdecode_MSGACK) ++*acks;
            else ++*rejs;
         }
         if (!useri_configon(useri_fTRACKFILT) || !(fr->nodraw&~0x1U)) {
            if (!aprspos_posvalid(dat->pos)) dat->pos = fr->vardat->pos;
            if (aprspos_posvalid(dat->pos)) {
               if (aprspos_posvalid(opos)) {
                  *waysum = *waysum+aprspos_distance(opos, dat->pos);
               }
               if (dat->altitude>-10000L && dat->altitude<=50000L) {
                  a1 = (float)dat->altitude;
                  if (do0) {
                     x = aprsdecode_trunc( *waysum*wdiv);
                     if (x>5759UL) x = 5759UL;
                     if (a2>(-1.E+4f)) {
                        if (!useri_configon(useri_fTRACKFILT) || (float)
                fabs(((a-a1)+a)-a2)*0.25f<(float)fabs(a2-a1)) {
                           /* median error filter */
                           if (alt[x]<=(-1.E+4f)) {
                              alt[x] = a;
                              xc = 1UL;
                           }
                           else {
                              ++xc;
                              alt[x] = alt[x]*(1.0f-X2C_DIVR(1.0f,
                (float)xc))+X2C_DIVR(a,(float)xc);
                           }
                        }
                     }
                     if (*markalt>X2C_min_longint && *markx==0UL) {
                        *markx = x/8UL;
                     }
                     if ((dat->pos.lat==aprsdecode_click.markpos.lat && dat->pos.long0==aprsdecode_click.markpos.long0)
                 && aprsdecode_click.markpost==fr->time0) {
                        *markalt = dat->altitude;
                     }
                  }
                  a2 = a;
                  a = a1;
               }
               opos = dat->pos;
            }
            ++*beacons;
         }
      }
      fr = fr->next;
   } while (fr);
   *way = *waysum;
   if (a2<=(-1.E+4f)) *waysum = 0.0f;
} /* end decodealt() */

static float aprstat_E0 = (-1.E+4f);


static void interpol(float alt[5760])
{
   unsigned long j;
   unsigned long i;
   float k;
   float y;
   i = 0UL;
   j = 0UL;
   y = (-1.E+4f);
   while (i<=5759UL) {
      if (alt[i]>(-1.E+4f)) {
         if (y<=(-1.E+4f)) y = alt[i];
         if (i>j) {
            k = X2C_DIVR(1.0f,(float)(i-j));
            while (j<i) {
               alt[j] = y+(alt[i]-y)*(1.0f-(float)(i-j)*k);
               ++j;
            }
         }
         else ++j;
         y = alt[i];
      }
      ++i;
   }
   while (j<=5759UL) {
      alt[j] = y;
      ++j;
   }
} /* end interpol() */


static void norm(float * hdiv, float alt[5760], float * maxalt,
                float * minalt)
{
   unsigned long i;
   *minalt = 2.147483647E+9f;
   *maxalt = (-2.147483648E+9f);
   for (i = 0UL; i<=5759UL; i++) {
      if (alt[i]<*minalt) *minalt = alt[i];
      if (alt[i]>*maxalt) *maxalt = alt[i];
   } /* end for */
   if (*maxalt-*minalt>8.3333333333333E-3f) {
      *hdiv = X2C_DIVR(120.0f,*maxalt-*minalt); /* normalize */
   }
   else *hdiv = 1.0f;
   for (i = 0UL; i<=5759UL; i++) {
      alt[i] = (alt[i]-*minalt)* *hdiv;
   } /* end for */
} /* end norm() */


extern void aprstat_althist(maptool_pIMAGE * img, aprsdecode_pOPHIST op,
                char * test, float * way, unsigned long * beacons,
                unsigned long * msgs, unsigned long * acks,
                unsigned long * rejs)
{
   char h[256];
   char s[256];
   float maxalt;
   float minalt;
   float alt[5760];
   float hdiv;
   float wdiv;
   unsigned long markx1;
   unsigned long markx;
   unsigned long xc;
   unsigned long sc;
   unsigned long x;
   unsigned long Maxx;
   struct aprsdecode_DAT dat;
   float waysum;
   long markalt;
   size_t tmp[2];
   unsigned long tmp0;
   *way = 0.0f;
   *beacons = 0UL;
   *msgs = 0UL;
   *acks = 0UL;
   *rejs = 0UL;
   if (op==0 || op->frames==0) {
      *test = 0;
      return;
   }
   Maxx = dynmaxx(8UL, 400UL, 720UL);
   decodealt(way, beacons, wdiv, rejs, acks, msgs, &dat, alt, &waysum, op,
                &markalt, &markx1, &markx, 0);
   if (waysum<0.05f) {
      *test = 0; /* no altitudes or km */
      return;
   }
   if (*test) return;
   X2C_DYNALLOCATE((char **)img,sizeof(struct maptool_PIX),
                (tmp[0] = Maxx+16UL,tmp[1] = 136U,tmp),2u);
   useri_debugmem.screens += (*img)->Len1*(*img)->Size1;
   if (*img==0) {
      osi_WrStrLn("error image alloc", 18ul);
      return;
   }
   maptool_clr(*img);
   wdiv = X2C_DIVR((float)(Maxx*8UL-1UL),waysum);
   decodealt(way, beacons, wdiv, rejs, acks, msgs, &dat, alt, &waysum, op,
                &markalt, &markx1, &markx, 1);
   interpol(alt);
   norm(&hdiv, alt, &maxalt, &minalt);
   /*
   FOR x:=0 TO HIGH(alt) DO
    IF alt[x]>=0.0 THEN WrFixed(alt[x], 3,11) ELSE WrStr(".") END;
   END;
    WrStrLn("");
   */
   /*sc:=VAL(CARDINAL, maxalt-minalt) DIV sfact(FLOAT(maxalt-minalt)); */
   sc = sfact(maxalt-minalt);
   strncpy(s,"  ",256u);
   aprsstr_Append(s, 256ul, op->call, 9ul);
   aprsstr_Append(s, 256ul, " dist=", 7ul);
   aprsstr_FixToStr(waysum, 2UL, h, 256ul);
   aprsstr_Append(s, 256ul, h, 256ul);
   aprsstr_Append(s, 256ul, "km min=", 8ul);
   aprsstr_IntToStr((long)X2C_TRUNCI(minalt,X2C_min_longint,X2C_max_longint),
                 1UL, h, 256ul);
   aprsstr_Append(s, 256ul, h, 256ul);
   aprsstr_Append(s, 256ul, "m max=", 7ul);
   aprsstr_IntToStr((long)X2C_TRUNCI(maxalt,X2C_min_longint,X2C_max_longint),
                 1UL, h, 256ul);
   aprsstr_Append(s, 256ul, h, 256ul);
   if (markalt>X2C_min_longint) {
      aprsstr_Append(s, 256ul, "m curs=", 8ul);
      aprsstr_IntToStr(markalt, 1UL, h, 256ul);
      aprsstr_Append(s, 256ul, h, 256ul);
   }
   aprsstr_Append(s, 256ul, "m (NN)", 7ul);
   paper(img, minalt, maxalt, sc, 8UL, Maxx, 120UL, s, 256ul);
   tmp0 = Maxx+8UL;
   x = 5UL;
   if (x<=tmp0) for (;; x++) {
      setpix(*img, (long)x, 8L, 200L, 1000L, 200L);
      if (x==tmp0) break;
   } /* end for */
   strncpy(h,"km",256u);
   if (waysum<5.0f) {
      waysum = waysum*1000.0f;
      strncpy(h,"m",256u);
   }
   sc = sfact(waysum);
   hdiv = X2C_DIVR((float)sc*(float)Maxx,waysum);
   wdiv = 0.0f;
   xc = 0UL;
   do {
      for (x = 5UL; x<=7UL; x++) {
         setpix(*img, (long)(aprsdecode_trunc(wdiv)+8UL), (long)x, 200L,
                1000L, 200L);
      } /* end for */
      if (xc>0UL && aprsdecode_trunc(wdiv)<Maxx-10UL) {
         num(*img, (long)((aprsdecode_trunc(wdiv)+8UL)-6UL), 8L, (long)xc, h,
                 256ul);
         h[0U] = 0;
      }
      xc += sc;
      wdiv = wdiv+hdiv;
   } while (aprsdecode_trunc(wdiv)<=Maxx);
   if (markx>0UL) {
      for (x = 8UL; x<=128UL; x++) {
         setpix(*img, (long)(markx+8UL), (long)x, 50L, 400L, 500L);
      } /* end for */
   }
   tmp0 = Maxx*8UL-1UL;
   x = 0UL;
   if (x<=tmp0) for (;; x++) {
      /* draw graph */
      addpix(*img, (float)x*0.125f+8.0f, alt[x]+8.0f, 62UL, 62UL, 87UL);
      if (x==tmp0) break;
   } /* end for */
} /* end althist() */

#define aprstat_TIMESPAN 86400

#define aprstat_MAXXX0 720

#define aprstat_MINXSIZE 320

#define aprstat_MAXY1 120

#define aprstat_MARGIN2 8

#define aprstat_ARRSIZE 1440
/*    XSTEP=FLOAT(MAXXX)/FLOAT(ARRSIZE); */

#define aprstat_INCHMM 0.254
/* inch/100 to mm */

#define aprstat_INVAL (-1.E+4)

#define aprstat_MAXJOIN 60
/* maximum interpolated span */

struct WX;


struct WX {
   float temp;
   float hyg;
   float baro;
   float wind;
   float rain;
   float lumi;
};

/*
  PROCEDURE join(VAR v:ARRAY OF REAL);
  VAR i,io:CARDINAL;
      yo,k:REAL;
  BEGIN
    yo:=INVAL;
    io:=0;
    i:=0;
    REPEAT
      IF v[i]<>INVAL THEN
        IF (yo<>INVAL) & (i>io) & (io+MAXJOIN>=i) THEN 
          k:=(v[i]-yo)/FLOAT(i-io+1);
          WHILE io<i DO
            yo:=yo+k;
            v[io]:=yo;
            INC(io);
          END;
        END;
        yo:=v[i];
        io:=i+1; 
      END;
      INC(i);
    UNTIL i>HIGH(v);
  END join;
*/
#define aprstat_F 0.9


static void scale(float v[], unsigned long v_len, float min0, float max0,
                float ysize, float maxamp, float * smin, float * smax,
                unsigned long * step)
{
   unsigned long i;
   float d;
   float a;
   float k;
   unsigned long tmp;
   if (min0==(-1.E+4f)) {
      min0 = 0.0f;
      d = max0;
      if (d<=maxamp) d = maxamp;
      a = X2C_DIVR(ysize,d);
      k = 0.0f;
   }
   else {
      d = max0-min0;
      if (d<=maxamp) d = maxamp;
      a = X2C_DIVR(ysize*0.9f,d);
      k = ysize*0.05f;
   }
   *step = sfact(d);
   *smin = min0-X2C_DIVR(k,a);
   *smax = min0+X2C_DIVR(k,a)+d;
   tmp = v_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if (v[i]!=(-1.E+4f)) v[i] = (v[i]-min0)*a+k;
      if (i==tmp) break;
   } /* end for */
} /* end scale() */


static char newimg(unsigned long Maxx, maptool_pIMAGE * img)
{
   size_t tmp[2];
   if (*img==0) {
      X2C_DYNALLOCATE((char **)img,sizeof(struct maptool_PIX),
                (tmp[0] = Maxx+16UL,tmp[1] = 136U,tmp),2u);
      useri_debugmem.screens += (*img)->Len1*(*img)->Size1;
   }
   if (*img==0) return 0;
   maptool_clr(*img);
   return 1;
} /* end newimg() */


static void timeline(unsigned long stime, maptool_pIMAGE * img,
                unsigned long Maxx)
{
   unsigned long y;
   unsigned long x;
   unsigned long to;
   unsigned long t;
   unsigned long tmp;
   to = 0UL;
   tmp = Maxx-1UL;
   x = 0UL;
   if (x<=tmp) for (;; x++) {
      setpix(*img, (long)(x+8UL), 8L, 200L, 1000L, 200L);
      t = ((stime+useri_localtime())-(((Maxx-1UL)-x)*86400UL)/Maxx)/3600UL;
      if (to==0UL) to = t;
      if (to!=t) {
         to = t;
         for (y = 5UL; y<=7UL; y++) {
            setpix(*img, (long)(x+8UL), (long)y, 200L, 1000L, 200L);
         } /* end for */
         if (x>28UL && t%3UL==0UL) {
            num(*img, (long)((x+8UL)-6UL), 8L, (long)(t%24UL), "h", 2ul);
         }
      }
      if (x==tmp) break;
   } /* end for */
} /* end timeline() */


static void dots(float XStep, maptool_pIMAGE * img, float v[],
                unsigned long v_len, char join, unsigned long r,
                unsigned long g, unsigned long b)
{
   unsigned long i;
   float dx;
   float io;
   float k;
   float yo;
   X2C_PCOPY((void **)&v,v_len*sizeof(float));
   yo = (-1.E+4f);
   io = 0.0f;
   i = 0UL;
   do {
      if (v[i]!=(-1.E+4f)) {
         maptool_waypoint(*img, (float)i*XStep+8.0f+0.5f, v[i]+8.0f, 2.0f,
                (long)r, (long)g, (long)b);
         if (((join && yo!=(-1.E+4f)) && i>aprsdecode_trunc(io))
                && aprsdecode_trunc(io)+60UL>=i) {
            k = X2C_DIVR(v[i]-yo,((float)i-io)+1.0f);
            if ((float)fabs(k)>2.0f) {
               dx = X2C_DIVR(2.0f,(float)fabs(k));
               k = k*dx;
            }
            else dx = 1.0f;
            while (aprsdecode_trunc(io)<=i) {
               yo = yo+k;
               maptool_waypoint(*img, io*XStep+8.0f, yo+8.0f, 1.2f,
                (long)(r/2UL), (long)(g/2UL), (long)(b/2UL));
               io = io+dx;
            }
         }
         yo = v[i];
         io = (float)(i+1UL);
      }
      ++i;
   } while (i<=v_len-1);
   X2C_PFREE(v);
} /* end dots() */


extern void aprstat_wxgraph(maptool_pIMAGE * img, aprsdecode_pOPHIST op,
                unsigned long stime, unsigned short * what,
                struct aprstat_LASTVAL * lastval)
{
   aprsdecode_pFRAMEHIST fr;
   unsigned long Maxx;
   unsigned long step;
   unsigned long xt;
   unsigned long xi;
   float XStep;
   float yax1;
   float yax0;
   float vh;
   struct WX min0;
   struct WX max0;
   char h[256];
   char s[256];
   struct aprsdecode_DAT dat;
   float temp[1440];
   float hyg[1440];
   float baro[1440];
   float winds[1440];
   float windd[1440];
   float gust[1440];
   float rain1[1440];
   float rain24[1440];
   float rain0[1440];
   float lumi[1440];
   unsigned short have;
   struct WX * anonym;
   if (op==0 || op->frames==0) {
      /*OR (op^.lastinftyp<100)*/
      return;
   }
   Maxx = dynmaxx(8UL, 320UL, 720UL);
   XStep = X2C_DIVR((float)Maxx,1440.0f);
   *img = 0;
   for (xi = 0UL; xi<=1439UL; xi++) {
      temp[xi] = (-1.E+4f);
      hyg[xi] = (-1.E+4f);
      baro[xi] = (-1.E+4f);
      winds[xi] = (-1.E+4f);
      windd[xi] = (-1.E+4f);
      gust[xi] = (-1.E+4f);
      rain1[xi] = (-1.E+4f);
      rain24[xi] = (-1.E+4f);
      rain0[xi] = (-1.E+4f);
      lumi[xi] = (-1.E+4f);
   } /* end for */
   { /* with */
      struct WX * anonym = &max0;
      anonym->temp = (-1.E+4f);
      anonym->hyg = (-1.E+4f);
      anonym->baro = (-1.E+4f);
      anonym->wind = (-1.E+4f);
      anonym->rain = (-1.E+4f);
      anonym->lumi = (-1.E+4f);
   }
   min0.temp = X2C_max_real;
   min0.baro = X2C_max_real;
   memset((char *)lastval,(char)0,sizeof(struct aprstat_LASTVAL));
   fr = op->frames;
   do {
      if (((fr->time0>stime-86400UL && fr->time0<=stime)
                && aprsdecode_Decode(fr->vardat->raw, 500ul,
                &dat)>=0L) && dat.sym=='_') {
         xt = aprsdecode_trunc((float)(fr->time0-(stime-86400UL))
                *1.6666666666667E-2f);
         if (xt>=1440UL) xt = 1439UL;
         vh = X2C_DIVR(dat.wx.temp-32.0f,1.8f);
         if (vh>=(-99.0f) && vh<=99.0f) {
            temp[xt] = vh;
            if (vh>max0.temp) max0.temp = vh;
            if (vh<min0.temp) min0.temp = vh;
            lastval->temp = vh;
         }
         if (dat.wx.hygro>=0.0f && dat.wx.hygro<=100.0f) {
            hyg[xt] = dat.wx.hygro;
            if (dat.wx.hygro>max0.hyg) max0.hyg = dat.wx.hygro;
            lastval->hyg = dat.wx.hygro;
         }
         vh = dat.wx.baro*0.1f;
         if (vh>=900.0f && vh<=1100.0f) {
            baro[xt] = vh;
            if (vh>max0.baro) max0.baro = vh;
            if (vh<min0.baro) min0.baro = vh;
            lastval->baro = vh;
         }
         vh = dat.wx.rain24*0.254f;
         if (vh>=0.0f && vh<300.0f) {
            rain24[xt] = vh;
            if (vh>max0.rain) max0.rain = vh;
            lastval->rain24 = vh;
         }
         vh = dat.wx.raintoday*0.254f;
         if (vh>=0.0f && vh<300.0f) {
            rain0[xt] = vh;
            if (vh>max0.rain) max0.rain = vh;
         }
         vh = dat.wx.rain1*0.254f;
         if (vh>=0.0f && vh<300.0f) {
            rain1[xt] = vh;
            if (vh>max0.rain) max0.rain = vh;
            lastval->rain = vh;
         }
         if (dat.wx.lum>=0.0f && dat.wx.lum<=2000.0f) {
            lumi[xt] = dat.wx.lum;
            if (dat.wx.lum>max0.lumi) max0.lumi = dat.wx.lum;
            lastval->lumi = dat.wx.lum;
         }
         if (dat.course<360UL) {
            windd[xt] = (float)dat.course;
            lastval->winddir = (float)dat.course;
            vh = (float)dat.speed*1.609f;
            if (vh>=0.0f && vh<=1000.0f) {
               winds[xt] = vh;
               lastval->winds = vh;
               if (vh>max0.wind) max0.wind = vh;
            }
         }
         vh = dat.wx.gust*1.609f;
         if (vh>=0.0f && vh<=1000.0f) {
            gust[xt] = vh;
            if (vh>max0.wind) max0.wind = vh;
         }
      }
      fr = fr->next;
   } while (fr);
   aprstext_DateLocToStr(stime, h, 256ul);
   aprsstr_Append(h, 256ul, " ", 2ul);
   aprsstr_Append(h, 256ul, op->call, 9ul);
   have = 0U;
   if (max0.temp!=(-1.E+4f)) {
      have |= 0x1U;
      if ((0x1U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(temp, 1440ul, min0.temp, max0.temp, 120.0f, 10.0f, &yax0,
                &yax1, &step);
         aprsstr_FixToStr(lastval->temp, 2UL, s, 256ul);
         aprsstr_Append(s, 256ul, "\177C ", 4ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, step, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, temp, 1440ul, 1, 200UL, 700UL, 40UL);
      }
   }
   if (max0.baro!=(-1.E+4f)) {
      have |= 0x2U;
      if ((0x2U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(baro, 1440ul, min0.baro, max0.baro, 120.0f, 2.0f, &yax0,
                &yax1, &step);
         aprsstr_FixToStr(lastval->baro, 2UL, s, 256ul);
         aprsstr_Append(s, 256ul, "hPa ", 5ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, step, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, baro, 1440ul, 1, 500UL, 400UL, 500UL);
      }
   }
   if (max0.wind!=(-1.E+4f)) {
      have |= 0x8U;
      have |= 0x10U;
      if ((0x8U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(winds, 1440ul, (-1.E+4f), max0.wind, 120.0f, 20.0f, &yax0,
                &yax1, &step);
         scale(gust, 1440ul, (-1.E+4f), max0.wind, 120.0f, 30.0f, &yax0,
                &yax1, &step);
         aprsstr_FixToStr(lastval->winds, 0UL, s, 256ul);
         aprsstr_Append(s, 256ul, "km/h Wind/Gust ", 16ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, step, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, winds, 1440ul, 1, 100UL, 500UL, 700UL);
         dots(XStep, img, gust, 1440ul, 1, 600UL, 100UL, 0UL);
      }
      if ((0x10U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(windd, 1440ul, (-1.E+4f), 360.0f, 120.0f, 365.0f, &yax0,
                &yax1, &step);
         aprsstr_FixToStr(lastval->winddir, 0UL, s, 256ul);
         aprsstr_Append(s, 256ul, "deg Wind Direction ", 20ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, 90UL, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, windd, 1440ul, 0, 200UL, 700UL, 700UL);
      }
   }
   if (max0.hyg!=(-1.E+4f)) {
      have |= 0x4U;
      if ((0x4U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(hyg, 1440ul, (-1.E+4f), 100.0f, 120.0f, 101.0f, &yax0, &yax1,
                &step);
         aprsstr_FixToStr(lastval->hyg, 0UL, s, 256ul);
         aprsstr_Append(s, 256ul, "% Humidty ", 11ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, step, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, hyg, 1440ul, 1, 0UL, 500UL, 700UL);
      }
   }
   if (max0.lumi!=(-1.E+4f)) {
      have |= 0x40U;
      if ((0x40U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(lumi, 1440ul, (-1.E+4f), max0.lumi, 120.0f, 50.0f, &yax0,
                &yax1, &step);
         aprsstr_FixToStr(lastval->lumi, 0UL, s, 256ul);
         aprsstr_Append(s, 256ul, "W/m^2 Luminosity ", 18ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, step, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, lumi, 1440ul, 1, 600UL, 600UL, 0UL);
      }
   }
   if (max0.rain!=(-1.E+4f)) {
      have |= 0x20U;
      if ((0x20U & *what)) {
         if (!newimg(Maxx, img)) return;
         scale(rain1, 1440ul, (-1.E+4f), max0.rain, 120.0f, 5.0f, &yax0,
                &yax1, &step);
         scale(rain24, 1440ul, (-1.E+4f), max0.rain, 120.0f, 5.0f, &yax0,
                &yax1, &step);
         scale(rain0, 1440ul, (-1.E+4f), max0.rain, 120.0f, 5.0f, &yax0,
                &yax1, &step);
         aprsstr_FixToStr(lastval->rain, 2UL, s, 256ul);
         aprsstr_Append(s, 256ul, "mm Rain ", 9ul);
         aprsstr_Append(s, 256ul, h, 256ul);
         paper(img, yax0, yax1, step, 8UL, Maxx, 120UL, s, 256ul);
         timeline(stime, img, Maxx);
         dots(XStep, img, rain1, 1440ul, 1, 500UL, 100UL, 0UL);
         dots(XStep, img, rain24, 1440ul, 1, 50UL, 600UL, 50UL);
         dots(XStep, img, rain0, 1440ul, 1, 100UL, 100UL, 700UL);
      }
   }
   *what = have;
/*
  IF img<>NIL THEN DISPOSE(img) END;
*/
} /* end wxgraph() */


extern void aprstat_BEGIN(void)
{
   static int aprstat_init = 0;
   if (aprstat_init) return;
   aprstat_init = 1;
   aprstext_BEGIN();
   useri_BEGIN();
   aprspos_BEGIN();
   osi_BEGIN();
   aprsdecode_BEGIN();
   aprsstr_BEGIN();
   maptool_BEGIN();
}

