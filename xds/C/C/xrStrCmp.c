/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrStrCmp.c Feb  3 14:31:19 2012" */
#include "xrStrCmp.h"
#define xrStrCmp_C_
#include "xmRTS.h"


extern int X2C_STRCMP_PROC(X2C_pVOID x, size_t alen, X2C_pVOID y,
                size_t blen)
{
   X2C_pCHAR b;
   X2C_pCHAR a;
   size_t m;
   size_t i;
   a = (X2C_pCHAR)x;
   b = (X2C_pCHAR)y;
   m = alen;
   if (m>blen) m = blen;
   i = 0U;
   while ((i<m && *a) && *a==*b) {
      a = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)a+(X2C_INT32)
                1UL);
      b = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)b+(X2C_INT32)
                1UL);
      ++i;
   }
   if (i>=m) {
      if (i<alen && *a==0) alen = i;
      if (i<blen && *b==0) blen = i;
      if (alen<blen) return -1;
      if (alen>blen) return 1;
      return 0;
   }
   if ((X2C_CARD8)(*a)<(X2C_CARD8)(*b)) return -1;
   if ((X2C_CARD8)(*a)>(X2C_CARD8)(*b)) return 1;
   return 0;
} /* end X2C_STRCMP_PROC() */

