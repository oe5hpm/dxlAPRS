/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrExponents.c Feb  3 14:30:28 2012" */
#include "xrExponents.h"
#define xrExponents_C_
#include "xmRTS.h"
#include "xPOSIX.h"


static X2C_LONGREAL c_abs(X2C_LONGREAL re, X2C_LONGREAL im)
{
   return sqrt(re*re+im*im);
} /* end c_abs() */


static X2C_LONGREAL c_arg(X2C_LONGREAL re, X2C_LONGREAL im)
{
   if (re==0.0 && im==0.0) return 0.0;
   return atan2(im, re);
} /* end c_arg() */


extern X2C_LONGREAL X2C_EXPRI(X2C_LONGREAL base, X2C_INT32 ex)
{
   X2C_LONGREAL res;
   if (ex<0L || ex>8L) return pow(base, (X2C_LONGREAL)ex);
   res = 1.0;
   while (ex>0L) {
      res = res*base;
      --ex;
   }
   return res;
} /* end X2C_EXPRI() */

static X2C_COMPLEX _cnst = {1.0,0.0};

extern X2C_COMPLEX X2C_EXPCI(X2C_COMPLEX base, X2C_INT32 ex)
{
   X2C_COMPLEX res;
   if (ex<0L || ex>8L) return X2C_EXPCR(base, (X2C_LONGREAL)ex);
   res = _cnst;
   while (ex>0L) {
      res = CPLX_MUL(res, base);
      --ex;
   }
   return res;
} /* end X2C_EXPCI() */

static X2C_LONGCOMPLEX _cnst1 = {1.0,0.0};

extern X2C_LONGCOMPLEX X2C_EXPLI(X2C_LONGCOMPLEX base, X2C_INT32 ex)
{
   X2C_LONGCOMPLEX res;
   if (ex<0L || ex>8L) return X2C_EXPLR(base, (X2C_LONGREAL)ex);
   res = _cnst1;
   while (ex>0L) {
      res = CPLX_LMUL(res, base);
      --ex;
   }
   return res;
} /* end X2C_EXPLI() */

static X2C_COMPLEX _cnst0 = {0.0,0.0};

extern X2C_COMPLEX X2C_EXPCR(X2C_COMPLEX base, X2C_LONGREAL ex)
{
   X2C_REAL x;
   X2C_LONGREAL y;
   X2C_COMPLEX tmp;
   if ((base.re==0.0f && base.im==0.0f) && ex>0.0) return _cnst0;
   x = (X2C_REAL)pow(c_abs((X2C_LONGREAL)base.re, (X2C_LONGREAL)base.im),
                ex);
   y = ex*c_arg((X2C_LONGREAL)base.re, (X2C_LONGREAL)base.im);
   return (tmp.re = x*(X2C_REAL)cos(y),tmp.im = x*(X2C_REAL)sin(y),tmp);
} /* end X2C_EXPCR() */

static X2C_LONGCOMPLEX _cnst2 = {0.0,0.0};

extern X2C_LONGCOMPLEX X2C_EXPLR(X2C_LONGCOMPLEX base, X2C_LONGREAL ex)
{
   X2C_LONGREAL y;
   X2C_LONGREAL x;
   X2C_LONGCOMPLEX tmp;
   if ((base.re==0.0 && base.im==0.0) && ex>0.0) return _cnst2;
   x = pow(c_abs(base.re, base.im), ex);
   y = ex*c_arg(base.re, base.im);
   return (tmp.re = x*cos(y),tmp.im = x*sin(y),tmp);
} /* end X2C_EXPLR() */

