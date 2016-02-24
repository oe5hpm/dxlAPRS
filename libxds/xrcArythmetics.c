/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrcArythmetics.c Feb  3 14:31:19 2012" */
#include "xrcArythmetics.h"
#define xrcArythmetics_C_
#include "X2C.h"
#include "M2EXCEPTION.h"


extern X2C_INT32 X2C_REM_F(X2C_INT32 a, X2C_INT32 b)
{
   if (b==0L) X2C_TRAP(6L);
   if (a>=0L) {
      if (b>0L) return (a%b);
      else return (a%(-b));
   }
   else if (b>0L) return (-((-a)%b));
   else return (-(-a)%(-b));
   return 0L;
} /* end X2C_REM_F() */


extern X2C_INT32 X2C_QUO_F(X2C_INT32 a, X2C_INT32 b)
{
   if (b==0L) X2C_TRAP(6L);
   if (a>=0L) {
      if (b>0L) return (a/b);
      else return (-(a/(-b)));
   }
   else if (b>0L) return (-((-a)/b));
   else return ((-a)/(-b));
   return 0L;
} /* end X2C_QUO_F() */


extern X2C_INT32 X2C_MOD_F(X2C_INT32 a, X2C_INT32 b)
{
   X2C_INT32 c;
   if (b<=0L) X2C_TRAP(6L);
   c = (a % b);
   if (a<0L && c<0L) c += b;
   return c;
} /* end X2C_MOD_F() */


extern X2C_INT32 X2C_DIV_F(X2C_INT32 a, X2C_INT32 b)
{
   X2C_INT32 c;
   if (b<=0L) X2C_TRAP(6L);
   c = (a/b);
   if (a<0L && c*b>a) --c;
   return c;
} /* end X2C_DIV_F() */


extern X2C_REAL X2C_DIVR_F(X2C_REAL a, X2C_REAL b)
{
 //  if (b==0.0f) X2C_TRAP(8L);
   if(0.0f==b) return a;
   else return (a/b);
   return 0.0f;
} /* end X2C_DIVR_F() */


extern X2C_LONGREAL X2C_DIVL_F(X2C_LONGREAL a, X2C_LONGREAL b)
{
   if (b==0.0) X2C_TRAP(8L);
   else return (a/b);
   return 0.0;
} /* end X2C_DIVL_F() */


extern X2C_INT8 X2C_ABS_INT8(X2C_INT8 x)
{
   if (x>=0) return x;
   if (x==-128) X2C_TRAP(1L);
   return -x;
} /* end X2C_ABS_INT8() */


extern X2C_INT16 X2C_ABS_INT16(X2C_INT16 x)
{
   if (x>=0) return x;
   if (x==-32768) X2C_TRAP(1L);
   return -x;
} /* end X2C_ABS_INT16() */


extern X2C_INT32 X2C_ABS_INT32(X2C_INT32 x)
{
   if (x>=0L) return x;
   if (x==X2C_min_longint) X2C_TRAP(1L);
   return -x;
} /* end X2C_ABS_INT32() */

