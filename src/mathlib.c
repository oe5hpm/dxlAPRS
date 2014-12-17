/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* "@(#)mathlib.c Aug 21 21:55:25 2013" */


#define X2C_int32
#define X2C_index32
#ifndef mathlib_H_
#include "mathlib.h"
#endif
#define mathlib_C_

float mathlib_epsilon;
#define mathlib_NoOfCoeff 10

#define mathlib_MaxPosReal 1.E38

#define mathlib_MinPosReal 1.E-38

#define mathlib_oneOe 3.67879515E-1

#define mathlib_PIo20 1.5707963267949

#define mathlib_PIo40 7.8539816339745E-1

#define mathlib_PI20 6.2831853071796

#define mathlib_PI0 3.1415926531932

#define mathlib_PIo2 1.5707963265966

#define mathlib_PIo4 7.853981632983E-1

#define mathlib_PI2 6.2831853063864


extern float mathlib_Sin(float x)
{
   float xd;
   float xc;
   float xb;
   float xa;
   float y;
   char q;
   char sign;
   if (x>=0.0f) sign = 0;
   else {
      sign = 1;
      x = -x;
   }
   if (x>6.2831853063864f) {
      x = x-6.2831853063864f*(float)(unsigned long)X2C_TRUNCC(X2C_DIVR(x,
                6.2831853063864f),0UL,X2C_max_longcard);
   }
   if (x>3.1415926531932f) {
      sign = !sign;
      x = x-3.1415926531932f;
   }
   q = 0;
   while (x>7.853981632983E-1f) {
      x = (float)fabs(x-1.5707963265966f);
      q = !q;
   }
   xa = x*x;
   if (q) {
      xb = xa*xa;
      y = ((1.0f-0.5f*xa)+4.1666666666E-2f*xb)-1.3888888888E-3f*xb*xa;
      if (x>0.3f) {
         xc = xb*xb;
         y = y+2.48015873E-5f*xc;
         if (x>0.65f) y = y-2.755731922E-7f*xc*xa;
      }
   }
   else {
      xb = xa*x;
      xc = xb*xa;
      y = (x-1.6666666666666E-1f*xb)+8.333333333333E-3f*xc;
      if (x>0.2f) {
         xd = xc*xa;
         y = y-1.984126984E-4f*xd;
         if (x>0.45f) y = y+2.75573E-6f*xd*xa;
      }
   }
   if (sign) y = -y;
   return y;
} /* end Sin() */


extern float mathlib_Cos(float x)
{
   return mathlib_Sin(x+1.5707963267949f);
} /* end Cos() */


extern float mathlib_Tan(float x)
{
   return X2C_DIVR(mathlib_Sin(x),mathlib_Cos(x));
} /* end Tan() */


extern float mathlib_Cot(float x)
{
   return X2C_DIVR(mathlib_Cos(x),mathlib_Sin(x));
} /* end Cot() */

#define mathlib_C0 4.63647609E-1

#define mathlib_C1 (-1.570796327)

#define mathlib_C2 (-8.034270561E+1)

#define mathlib_C3 (-1.191447224)

#define mathlib_C4 (-7.833542786E-2)

#define mathlib_C5 1.258464112

#define mathlib_C6 2.104518952

#define mathlib_C7 6.364241687

#define mathlib_C8 2.627277525E+1

#define mathlib_C9 1.237469388E+1


extern float mathlib_ATan(float x)
{
   float h;
   float y;
   char sign;
   if (x>=0.0f) sign = 0;
   else {
      sign = 1;
      x = -x;
   }
   if (x>1.0f) {
      x = X2C_DIVR(1.0f,x);
      h = (-1.570796327f);
   }
   else h = 0.0f;
   if (x>0.5f) {
      h = h+4.63647609E-1f;
      x = X2C_DIVR(x*2.0f-1.0f,x+2.0f);
   }
   y = x*x;
   y = (float)fabs((X2C_DIVR(x,X2C_DIVR((-8.034270561E+1f),
                X2C_DIVR((-1.191447224f),X2C_DIVR((-7.833542786E-2f),
                y+1.258464112f)+2.104518952f+y)+6.364241687f+y)
                +2.627277525E+1f+y))*1.237469388E+1f+h);
   if (sign) y = -y;
   return y;
} /* end ATan() */


extern float mathlib_Sqrt(float x)
{
   float ee;
   float e;
   float z;
   float y;
   if (x>0.0f) {
      y = 2.0f;
      do {
         z = y;
         y = z*z;
      } while (y<=x);
      e = y;
      for (;;) {
         y = z;
         z = 0.5f*(y+X2C_DIVR(x,y));
         ee = (float)fabs(y-z);
         if (ee==e || ee>=e) break;
         e = ee;
      }
      return z;
   }
   return 0.0f;
} /* end Sqrt() */


extern float mathlib_ASin(float x)
{
   float s;
   s = 1.0f-x*x;
   if (s<=0.0f) {
      if (x>=0.0f) return 1.5707963267949f;
      else return (-1.5707963267949f);
   }
   return mathlib_ATan(X2C_DIVR(x,mathlib_Sqrt(s)));
} /* end ASin() */


extern float mathlib_ACos(float x)
{
   return 1.5707963267949f-mathlib_ASin(x);
} /* end ACos() */


extern float mathlib_Log(float x)
{
   float y;
   float xp;
   float xm;
   float t;
   float s;
   float f;
   unsigned long i;
   if (x<=0.0f) return (-1.E38f);
   if (x<3.67879515E-1f) {
      i = 0UL;
      do {
         x = x*2.718281828459f;
         X2C_INCU(&i,1UL,0UL,X2C_max_longcard);
      } while (x<=3.67879515E-1f);
      return mathlib_Log(x)-(float)i;
   }
   else if (x<=2.718281828459f) {
      i = 1UL;
      xm = x-1.0f;
      xp = x+1.0f;
      s = X2C_DIVR(xm,xp);
      f = s;
      y = f*f;
      do {
         X2C_INCU(&i,2UL,0UL,X2C_max_longcard);
         f = f*y;
         t = X2C_DIVR(f,(float)i);
         s = s+t;
      } while ((float)fabs(t)>mathlib_epsilon);
      return 2.0f*s;
   }
   else {
      i = 0UL;
      do {
         x = x*3.67879515E-1f;
         X2C_INCU(&i,1UL,0UL,X2C_max_longcard);
      } while (x>=2.718281828459f);
      return mathlib_Log(x)+(float)i;
   }
   return 0;
} /* end Log() */


extern float mathlib_Exp(float x)
{
   float s;
   float f;
   unsigned long i;
   unsigned long tmp;
   if (x<(-45.0f)) return 1.E-38f;
   else if (x<0.0f) return X2C_DIVR(1.0f,mathlib_Exp(-x));
   else if (x<1.0f) {
      i = 0UL;
      s = 1.0f;
      f = 1.0f;
      do {
         X2C_INCU(&i,1UL,0UL,X2C_max_longcard);
         f = X2C_DIVR(f*x,(float)i);
         s = s+f;
      } while (f>=mathlib_epsilon);
      return s;
   }
   else if (x<45.0f) {
      f = mathlib_Exp(x-(float)(unsigned long)X2C_TRUNCC(x,0UL,
                X2C_max_longcard));
      tmp = (unsigned long)X2C_TRUNCC(x,0UL,X2C_max_longcard);
      i = 1UL;
      if (i<=tmp) for (;; i++) {
         f = f*2.718281828459f;
         if (i==tmp) break;
      } /* end for */
      return f;
   }
   else return 1.E38f;
   return 0;
} /* end Exp() */


extern float mathlib_Power(float x, float y)
{
   return mathlib_Exp(mathlib_Log(x)*y);
} /* end Power() */


extern long mathlib_Round(float x)
{
   if (x>=0.0f) x = x+0.5f;
   else x = x-0.5f;
   return (long)X2C_TRUNCI(x,X2C_min_longint,X2C_max_longint);
} /* end Round() */


extern float mathlib_Floor(float x)
{
   float y;
   y = (float)(long)X2C_TRUNCI(x,X2C_min_longint,X2C_max_longint);
   if (y>x) y = y-1.0f;
   return y;
} /* end Floor() */

static float heps;


extern void mathlib_BEGIN(void)
{
   static int mathlib_init = 0;
   if (mathlib_init) return;
   mathlib_init = 1;
   heps = 0.125f;
   do {
      mathlib_epsilon = heps;
      heps = X2C_DIVR(heps,2.0f);
   } while (heps+2.0f>2.0f);
}

