/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef complex_H_
#include "complex.h"
#endif
#define complex_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>




extern void complex_cmplx(struct complex_Complex * Result, float X,
                float A)
{
   Result->Re = osic_cos(A)*X;
   Result->Im = osic_sin(A)*X;
} /* end cmplx() */


extern float complex_cabs(struct complex_Complex X)
{
   return osic_sqrt(X.Re*X.Re+X.Im*X.Im);
} /* end cabs() */


extern float complex_arc(struct complex_Complex X)
{
   struct complex_Complex * anonym;
   { /* with */
      struct complex_Complex * anonym = &X;
      if (anonym->Re==0.0f && anonym->Im==0.0f) return 0.0f;
      else if ((float)fabs(anonym->Re)>=(float)fabs(anonym->Im)) {
         if (anonym->Re>=0.0f) {
            return osic_arctan(X2C_DIVR(anonym->Im,anonym->Re));
         }
         else if (anonym->Im>=0.0f) {
            return 3.1415926536f+osic_arctan(X2C_DIVR(anonym->Im,
                anonym->Re));
         }
         else {
            return osic_arctan(X2C_DIVR(anonym->Im,
                anonym->Re))-3.1415926536f;
         }
      }
      else if (anonym->Im>=0.0f) {
         return 1.5707963268f-osic_arctan(X2C_DIVR(anonym->Re,anonym->Im));
      }
      else {
         return (-1.5707963268f)-osic_arctan(X2C_DIVR(anonym->Re,
                anonym->Im));
      }
   }
   return 0;
} /* end arc() */


extern void complex_cadd(struct complex_Complex * Result,
                struct complex_Complex A, struct complex_Complex B)
{
   Result->Re = A.Re+B.Re;
   Result->Im = A.Im+B.Im;
} /* end cadd() */


extern void complex_csub(struct complex_Complex * Result,
                struct complex_Complex A, struct complex_Complex B)
{
   Result->Re = A.Re-B.Re;
   Result->Im = A.Im-B.Im;
} /* end csub() */


extern void complex_cmul(struct complex_Complex * Result,
                struct complex_Complex A, struct complex_Complex B)
{
   Result->Re = A.Re*B.Re-A.Im*B.Im;
   Result->Im = A.Re*B.Im+A.Im*B.Re;
} /* end cmul() */


extern void complex_cdiv(struct complex_Complex * Result,
                struct complex_Complex A, struct complex_Complex B)
{
   float Nenner;
   Nenner = B.Re*B.Re+B.Im*B.Im;
   if (Nenner!=0.0f) {
      Result->Re = X2C_DIVR(A.Re*B.Re+A.Im*B.Im,Nenner);
      Result->Im = X2C_DIVR(A.Im*B.Re-A.Re*B.Im,Nenner);
   }
   else {
      Result->Re = 0.0f;
      Result->Im = 0.0f;
   }
} /* end cdiv() */


extern void complex_csqrt(struct complex_Complex * Result,
                struct complex_Complex X)
{
   complex_cmplx(Result, osic_sqrt(complex_cabs(X)), complex_arc(X)*0.5f);
} /* end csqrt() */


extern void complex_csin(struct complex_Complex * Result,
                struct complex_Complex X)
{
   float e;
   e = osic_exp(X.Im);
   Result->Re = osic_sin(X.Re)*(e+X2C_DIVR(1.0f,e))*0.5f;
   Result->Im = osic_cos(X.Re)*(e-X2C_DIVR(1.0f,e))*0.5f;
} /* end csin() */


extern void complex_ccos(struct complex_Complex * Result,
                struct complex_Complex X)
{
   float e;
   e = osic_exp(X.Im);
   Result->Re = osic_cos(X.Re)*(e+X2C_DIVR(1.0f,e))*0.5f;
   Result->Im = -(osic_sin(X.Re)*(e-X2C_DIVR(1.0f,e))*0.5f);
} /* end ccos() */


extern void complex_cexp(struct complex_Complex * Result,
                struct complex_Complex X)
{
   complex_cmplx(Result, osic_exp(X.Re), X.Im);
} /* end cexp() */


extern void complex_cln(struct complex_Complex * Result,
                struct complex_Complex X)
{
   Result->Re = osic_ln(complex_cabs(X));
   Result->Im = complex_arc(X);
} /* end cln() */


extern void complex_BEGIN(void)
{
   static int complex_init = 0;
   if (complex_init) return;
   complex_init = 1;
   osi_BEGIN();
}

