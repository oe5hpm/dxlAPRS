/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrAReal.c Feb  3 14:30:28 2012" */
#include "xrAReal.h"
#define xrAReal_C_
#include "X2C.h"
#include "M2EXCEPTION.h"


extern X2C_INT32 X2C_ENTIER(X2C_LONGREAL x)
{
   X2C_INT32 i;
   if (x<(-2.147483648E+9) || x>2.147483647E+9) X2C_TRAP(1L);
   i=(long)x;
   if ((X2C_LONGREAL)i>x) --i;
   return i;
} /* end X2C_ENTIER() */


extern X2C_INT32 X2C_TRUNCI(X2C_LONGREAL x, X2C_INT32 min0, X2C_INT32 max0)
{
   X2C_INT32 i;
   if (x<(X2C_LONGREAL)min0 || x>(X2C_LONGREAL)max0) X2C_TRAP(1L);
   i=(long)x;
   if (x>0.0) {
      if ((X2C_LONGREAL)i>x) --i;
   }
   else if ((X2C_LONGREAL)i<x) ++i;
   return i;
} /* end X2C_TRUNCI() */


extern X2C_CARD32 X2C_TRUNCC(X2C_LONGREAL x, X2C_CARD32 min0,
                X2C_CARD32 max0)
{
   X2C_CARD32 i;
   if (x<(X2C_LONGREAL)min0 || x>(X2C_LONGREAL)max0) X2C_TRAP(1L);
   i=(unsigned long)x;
   if ((X2C_LONGREAL)i>x) --i;
   return i;
} /* end X2C_TRUNCC() */

