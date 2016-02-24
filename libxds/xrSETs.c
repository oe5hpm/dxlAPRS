/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrSETs.c Feb  3 14:31:18 2012" */
#include "xrSETs.h"
#define xrSETs_C_


extern X2C_INT32 X2C_ASH(X2C_INT32 a, X2C_INT32 b)
{
   if (b>31L) return 0L;
   if (b<-31L) b = -31L;
   return (b >= 0) ? (a << b) : (a >> (-b));
   return 0L;
} /* end X2C_ASH() */


extern X2C_INT16 X2C_ASH16(X2C_INT16 a, X2C_INT32 b)
{
   X2C_INT16 n;
   if (b>15L) return 0;
   if (b<-15L) n = -15;
   else n = (X2C_INT16)b;
   return (n >= 0) ? (a << n) : (a >> (-n));
   return 0;
} /* end X2C_ASH16() */


extern X2C_INT8 X2C_ASH8(X2C_INT8 a, X2C_INT32 b)
{
   X2C_INT8 n;
   if (b>7L) return 0;
   if (b<-7L) n = -7;
   else n = (X2C_INT8)b;
   return (n >= 0) ? (a << n) : (a >> (-n));
   return 0;
} /* end X2C_ASH8() */


extern X2C_CARD32 X2C_ROT(X2C_CARD32 a, X2C_INT16 length, X2C_INT32 n)
{
   X2C_CARD32 m;
   m = 0UL;
   m = (length==32) ? 0xFFFFFFFFl : (1l << length)-1;
   if (n>0L) {
      n=n % length;
      return ((a << n) | (a >> (length - n))) & m;
   }
   else {
      n= -n % length;
      return ((a >> n) | (a << (length - n))) & m;
   }
   return 0UL;
} /* end X2C_ROT() */


extern X2C_CARD32 X2C_LSH(X2C_CARD32 a, X2C_INT16 length, X2C_INT32 n)
{
   X2C_CARD32 m;
   m = 0UL;
   m = (length==32) ? 0xFFFFFFFFl : (1l << length)-1;
   if (n>0L) {
      if (n>=(X2C_INT32)length) return 0UL;
      return (a << n) & m;
   }
   else {
      if (n<=(X2C_INT32) -length) return 0UL;
      return (a >> -n) & m;
   }
   return 0UL;
} /* end X2C_LSH() */

