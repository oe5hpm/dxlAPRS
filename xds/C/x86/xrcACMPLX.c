/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrcACMPLX.c Feb  3 14:30:28 2012" */
#include "xrcACMPLX.h"
#define xrcACMPLX_C_
#include "X2C.h"
#include "M2EXCEPTION.h"


extern int CPLX_CMP(X2C_COMPLEX x, X2C_COMPLEX y)
{
   if (x.re==y.re && x.im==y.im) return 0;
   return 1;
} /* end CPLX_CMP() */


extern X2C_COMPLEX CPLX_ADD(X2C_COMPLEX x, X2C_COMPLEX y)
{
   X2C_COMPLEX tmp;
   return (tmp.re = x.re+y.re,tmp.im = x.im+y.im,tmp);
} /* end CPLX_ADD() */


extern X2C_COMPLEX CPLX_SUB(X2C_COMPLEX x, X2C_COMPLEX y)
{
   X2C_COMPLEX tmp;
   return (tmp.re = x.re-y.re,tmp.im = x.im-y.im,tmp);
} /* end CPLX_SUB() */


extern X2C_COMPLEX CPLX_MUL(X2C_COMPLEX x, X2C_COMPLEX y)
{
   X2C_COMPLEX tmp;
   return (tmp.re = x.re*y.re-x.im*y.im,tmp.im = x.re*y.im+x.im*y.re,tmp);
} /* end CPLX_MUL() */


extern X2C_COMPLEX CPLX_DIV(X2C_COMPLEX x, X2C_COMPLEX y)
{
   X2C_REAL d;
   X2C_COMPLEX tmp;
   d = y.re*y.re+y.im*y.im;
   if (d==0.0f) X2C_TRAP(10L);
   return (tmp.re = X2C_DIVR(x.re*y.re+x.im*y.im,d),
                tmp.im = X2C_DIVR(x.im*y.re-x.re*y.im,d),tmp);
} /* end CPLX_DIV() */


extern X2C_COMPLEX CPLX_NEG(X2C_COMPLEX x)
{
   X2C_COMPLEX tmp;
   return (tmp.re = -x.re,tmp.im = -x.im,tmp);
} /* end CPLX_NEG() */


extern int CPLX_LCMP(X2C_LONGCOMPLEX x, X2C_LONGCOMPLEX y)
{
   if (x.re==y.re && x.im==y.im) return 0;
   return 1;
} /* end CPLX_LCMP() */


extern X2C_LONGCOMPLEX CPLX_LADD(X2C_LONGCOMPLEX x, X2C_LONGCOMPLEX y)
{
   X2C_LONGCOMPLEX tmp;
   return (tmp.re = x.re+y.re,tmp.im = x.im+y.im,tmp);
} /* end CPLX_LADD() */


extern X2C_LONGCOMPLEX CPLX_LSUB(X2C_LONGCOMPLEX x, X2C_LONGCOMPLEX y)
{
   X2C_LONGCOMPLEX tmp;
   return (tmp.re = x.re-y.re,tmp.im = x.im-y.im,tmp);
} /* end CPLX_LSUB() */


extern X2C_LONGCOMPLEX CPLX_LMUL(X2C_LONGCOMPLEX x, X2C_LONGCOMPLEX y)
{
   X2C_LONGCOMPLEX tmp;
   return (tmp.re = x.re*y.re-x.im*y.im,tmp.im = x.re*y.im+x.im*y.re,tmp);
} /* end CPLX_LMUL() */


extern X2C_LONGCOMPLEX CPLX_LDIV(X2C_LONGCOMPLEX x, X2C_LONGCOMPLEX y)
{
   X2C_LONGREAL d;
   X2C_LONGCOMPLEX tmp;
   d = y.re*y.re+y.im*y.im;
   if (d==0.0) X2C_TRAP(10L);
   return (tmp.re = X2C_DIVL(x.re*y.re+x.im*y.im,d),
                tmp.im = X2C_DIVL(x.im*y.re-x.re*y.im,d),tmp);
} /* end CPLX_LDIV() */


extern X2C_LONGCOMPLEX CPLX_LNEG(X2C_LONGCOMPLEX x)
{
   X2C_LONGCOMPLEX tmp;
   return (tmp.re = -x.re,tmp.im = -x.im,tmp);
} /* end CPLX_LNEG() */


extern X2C_LONGCOMPLEX CPLX_L(X2C_COMPLEX x)
{
   X2C_LONGCOMPLEX tmp;
   return (tmp.re = (X2C_LONGREAL)x.re,tmp.im = (X2C_LONGREAL)x.im,tmp);
} /* end CPLX_L() */

