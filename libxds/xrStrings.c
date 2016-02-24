/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrStrings.c Feb  3 14:31:18 2012" */
#include "xrStrings.h"
#define xrStrings_C_
#include "xmRTS.h"


extern X2C_CHAR X2C_CAP(X2C_CHAR x)
{
   if ((X2C_CARD8)x>='a' && (X2C_CARD8)x<='z') {
      x = (X2C_CHAR)(((X2C_CARD32)(X2C_CARD8)x+65UL)-97UL);
   }
   return x;
} /* end X2C_CAP() */


extern X2C_pVOID X2C_COPY(X2C_pVOID s, size_t s_len, X2C_pVOID d,
                size_t d_len)
{
   X2C_pCHAR y;
   X2C_pCHAR x;
   size_t i;
   x = (X2C_pCHAR)s;
   y = (X2C_pCHAR)d;
   if (s_len>=d_len) s_len = d_len-1U;
   i = 0U;
   while (i<s_len && *x) {
      *y = *x;
      x = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)x+(X2C_INT32)
                1UL);
      y = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)y+(X2C_INT32)
                1UL);
      ++i;
   }
   *y = 0;
   return d;
} /* end X2C_COPY() */


extern size_t X2C_LENGTH(X2C_pVOID s, size_t s_len)
{
   X2C_pCHAR x;
   size_t i;
   x = (X2C_pCHAR)s;
   i = 0U;
   while (i<s_len && *x) {
      x = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)x+(X2C_INT32)
                1UL);
      ++i;
   }
   return i;
} /* end X2C_LENGTH() */

