/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef fft_H_
#include "fft.h"
#endif
#define fft_C_
#ifndef complex_H_
#include "complex.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>


extern void fft_Hamming(struct complex_Complex feld[], uint32_t feld_len)
{
   uint32_t i;
   uint32_t tmp;
   tmp = feld_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      feld[i].Re = feld[i].Re*(0.54f+0.46f*osic_cos(3.1415926536f*(X2C_DIVR((float)
                (2UL*i),(float)(1UL+(feld_len-1)))-1.0f)));
      if (i==tmp) break;
   } /* end for */
} /* end Hamming() */


extern void fft_Hann(struct complex_Complex feld[], uint32_t feld_len)
{
   uint32_t i;
   float p;
   float m;
   uint32_t tmp;
   m = (float)((feld_len-1)+1UL)*0.5f;
   tmp = feld_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      p = (float)i-m;
      if (p!=0.0f) {
         feld[i].Re = X2C_DIVR(feld[i]
                .Re*osic_sin(X2C_DIVR(3.1415926536f*((float)i-m),m)),
                X2C_DIVR(3.1415926536f*((float)i-m),m));
      }
      if (i==tmp) break;
   } /* end for */
} /* end Hann() */


extern void fft_Blackmann(struct complex_Complex feld[],
                uint32_t feld_len)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   N = X2C_DIVR(6.2831853072f,(float)((feld_len-1)+1UL));
   tmp = feld_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      feld[i].Re = feld[i].Re*((0.42f-0.5f*osic_cos(N*(float)i))
                +0.08f*osic_cos(N*2.0f*(float)i));
      if (i==tmp) break;
   } /* end for */
} /* end Blackmann() */


extern void fft_Shift(struct complex_Complex u[], uint32_t u_len)
{
   uint32_t m;
   uint32_t i;
   struct complex_Complex h;
   uint32_t tmp;
   m = ((u_len-1)+1UL)/2UL;
   tmp = m-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      h = u[i];
      u[i] = u[i+m];
      u[i+m] = h;
      if (i==tmp) break;
   } /* end for */
} /* end Shift() */


extern void fft_Transform(struct complex_Complex feld[], uint32_t feld_len,
                 char INVERS)
{
   uint32_t n2;
   uint32_t n1;
   uint32_t ri;
   uint32_t N;
   uint32_t M;
   uint32_t id;
   uint32_t idd2;
   uint32_t z;
   uint32_t j;
   uint32_t i;
   struct complex_Complex wcpx;
   struct complex_Complex h;
   float sn;
   float cs;
   float sk;
   float r;
   float ck;
   float wk;
   uint32_t tmp;
   uint32_t tmp0;
   N = (feld_len-1)+1UL;
   /*bitrev*/
   ri = 1UL;
   n2 = N/2UL;
   n1 = N-1UL;
   tmp = n1;
   i = 1UL;
   if (i<=tmp) for (;; i++) {
      if (i<ri) {
         h = feld[ri-1UL];
         feld[ri-1UL] = feld[i-1UL];
         feld[i-1UL] = h;
      }
      j = n2;
      while (j<ri) {
         ri -= j;
         j = j/2UL;
      }
      ri += j;
      if (i==tmp) break;
   } /* end for */
   /*bitrev*/
   M = (uint32_t)X2C_TRUNCC(0.5f+osic_ln((float)N)*1.44269504f,0UL,
                X2C_max_longcard);
   wk = 3.1415926536f;
   idd2 = 1UL;
   if (INVERS) {
      h.Re = 2.0f;
      h.Im = 0.0f;
      tmp = N-1UL;
      i = 1UL;
      if (i<=tmp) for (;; i++) {
         complex_cdiv(&feld[i], feld[i], h);
         if (i==tmp) break;
      } /* end for */
      wk = (-3.1415926536f);
   }
   tmp = M;
   z = 1UL;
   if (z<=tmp) for (;; z++) {
      id = 2UL*idd2;
      ck = osic_sin(wk*0.5f);
      ck = 2.0f*ck*ck;
      r = -(2.0f*ck);
      sk = osic_sin(wk);
      wk = wk*0.5f;
      cs = 1.0f;
      sn = 0.0f;
      i = 0UL;
      while (i<N) {
         h = feld[i+idd2];
         complex_csub(&feld[i+idd2], feld[i], h);
         complex_cadd(&feld[i], feld[i], h);
         i += id;
      }
      if (z!=1UL) {
         tmp0 = idd2-1UL;
         j = 1UL;
         if (j<=tmp0) for (;; j++) {
            ck = r*cs+ck;
            cs = cs+ck;
            sk = r*sn+sk;
            sn = sn+sk;
            wcpx.Re = cs;
            wcpx.Im = sn;
            i = j;
            while (i<N) {
               complex_cmul(&h, wcpx, feld[i+idd2]);
               complex_csub(&feld[i+idd2], feld[i], h);
               complex_cadd(&feld[i], feld[i], h);
               i += id;
            }
            if (j==tmp0) break;
         } /* end for */
      }
      idd2 = id;
      if (z==tmp) break;
   } /* end for */
   if (!INVERS) {
      h.Re = (float)N;
      h.Im = 0.0f;
      complex_cdiv(&feld[0UL], feld[0UL], h);
      h.Re = (float)(N/2UL);
      h.Im = 0.0f;
      tmp = N-1UL;
      i = 1UL;
      if (i<=tmp) for (;; i++) {
         complex_cdiv(&feld[i], feld[i], h);
         if (i==tmp) break;
      } /* end for */
   }
} /* end Transform() */


extern void fft_BEGIN(void)
{
   static int fft_init = 0;
   if (fft_init) return;
   fft_init = 1;
   osi_BEGIN();
   complex_BEGIN();
}

