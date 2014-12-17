/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* APRSmap APRS-Client Copyright (c) 2014 OE5DXL */
/* "@(#)Lib.c Oct  1 20:53:08 2014" */


#define X2C_int32
#define X2C_index32
#ifndef Lib_H_
#include "Lib.h"
#endif
#define Lib_C_
#ifndef ProgEnv_H_
#include "ProgEnv.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif

unsigned long Lib_argc;
#define Lib_PI 3.1415926535898

static double rnd;


extern void Lib_Fill(X2C_ADDRESS buf, unsigned long len, char byte)
{
   unsigned long i;
   char * p;
   unsigned long tmp;
   if (len>0UL) {
      p = (char *)buf;
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         p[i] = byte;
         if (i==tmp) break;
      } /* end for */
   }
} /* end Fill() */


extern void Lib_Move(X2C_ADDRESS from, X2C_ADDRESS to, unsigned long len)
{
   unsigned long i;
   char * t;
   char * f;
   unsigned long tmp;
   if (len>0UL) {
      f = (char *)from;
      t = (char *)to;
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         t[i] = f[i];
         if (i==tmp) break;
      } /* end for */
   }
} /* end Move() */


extern void Lib_NextArg(char s[], unsigned long s_len)
{
   ProgEnv_GetArg(Lib_argc, s, s_len);
   ++Lib_argc;
} /* end NextArg() */


extern void Lib_ArgN0(unsigned long n, char s[], unsigned long s_len)
{
   if (n>0UL) --n;
   ProgEnv_GetArg(n, s, s_len);
} /* end ArgN() */


extern void Lib_Randomize(double start)
{
   rnd = start;
   while (fabs(rnd)>=1.0) rnd = rnd*1.4159265358979E-1;
   if (rnd<0.0) rnd = rnd+1.0;
} /* end Randomize() */


extern double Lib_Random(void)
{
   rnd = rnd+3.1415926535898;
   rnd = rnd*rnd;
   rnd = rnd*rnd;
   rnd = rnd*rnd;
   rnd = rnd-(double)(unsigned long)X2C_TRUNCC(rnd,0UL,X2C_max_longcard);
   return rnd;
} /* end Random() */


extern float Lib_Noise12(void)
{
   unsigned long i;
   float r;
   r = 0.0f;
   for (i = 0UL; i<=11UL; i++) {
      r = r+(float)Lib_Random();
   } /* end for */
   return X2C_DIVR(r,12.0f);
} /* end Noise12() */


extern float Lib_NoiseGauss(void)
{
   float u;
   do {
      u = (float)Lib_Random();
   } while (u<=0.0f);
   return RealMath_sqrt(-(2.0f*RealMath_ln(u)))
                *RealMath_cos(6.2831853071796f*(float)Lib_Random());
} /* end NoiseGauss() */

/*
PROCEDURE Random():REAL;
VAR g:CARDINAL;
    r:REAL;

  PROCEDURE FRAC(x:REAL):REAL;
  BEGIN RETURN x - FLOAT(TRUNC(x)) END FRAC;

  PROCEDURE Quad(VAR g:CARDINAL; VAR r:REAL);
  VAR gr:REAL;
  BEGIN
    gr:=FLOAT(2*g)*r;
    r:=r*r + FRAC(gr);
    g:=(g*g) + VAL(CARDINAL, TRUNC(r) + TRUNC(gr));
    r:=FRAC(r);
  END Quad;
    
BEGIN
  r:=rnd + FRACPI;
  g:=3 + TRUNC(r);
  r:=FRAC(r);
  Quad(g,r);
  Quad(g,r);
  rnd:=FRAC(r*r + FRAC(FLOAT(2*g)*r));
  RETURN rnd
END Random;
*/

extern void Lib_BEGIN(void)
{
   static int Lib_init = 0;
   if (Lib_init) return;
   Lib_init = 1;
   RealMath_BEGIN();
   ProgEnv_BEGIN();
   Lib_argc = 0UL;
   rnd = 0.0;
}

