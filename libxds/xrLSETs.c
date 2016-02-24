/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrLSETs.c Feb  3 14:31:18 2012" */
#include "xrLSETs.h"
#define xrLSETs_C_
#include "xmRTS.h"
#include "X2C.h"
#include "M2EXCEPTION.h"


extern LSET X2C_AND(LSET res, LSET a, LSET b, X2C_CARD16 length)
{
   LSET c;
   c = res;
   while (length) { length--; *c++ = *a++ & *b++; }
   return res;
} /* end X2C_AND() */


extern LSET X2C_OR(LSET res, LSET a, LSET b, X2C_CARD16 length)
{
   LSET c;
   c = res;
   while (length) { length--; *c++ = *a++ | *b++; }
   return res;
} /* end X2C_OR() */


extern LSET X2C_XOR(LSET res, LSET a, LSET b, X2C_CARD16 length)
{
   LSET c;
   c = res;
   while (length) { length--; *c++ = *a++ ^ *b++; }
   return res;
} /* end X2C_XOR() */


extern LSET X2C_BIC(LSET res, LSET a, LSET b, X2C_CARD16 length)
{
   LSET c;
   c = res;
   while (length) { length--; *c++ = *a++ & ~( *b++ ); }
   return res;
} /* end X2C_BIC() */


extern LSET X2C_COMPLEMENT(LSET res, LSET a, X2C_CARD16 length)
{
   LSET c;
   c = res;
   while (length) { length--; *c++ = ~( *a++ ); }
   return res;
} /* end X2C_COMPLEMENT() */


extern X2C_BOOLEAN X2C_SET_EQU(LSET a, LSET b, X2C_CARD16 bits)
{
   X2C_LSET_BASE mask;
   while (bits >= X2C_LSET_SIZE) {
     if ( *a++ != *b++ ) return 0;
     bits -= X2C_LSET_SIZE;
   }
   if (bits == 0) return 1;
   mask = (1<<bits) - 1;
   return ( *a & mask ) == ( *b & mask );
   return 0;
} /* end X2C_SET_EQU() */


extern X2C_BOOLEAN X2C_SET_LEQ(LSET a, LSET b, X2C_CARD16 bits)
{
   X2C_LSET_BASE mask;
   while (bits >= X2C_LSET_SIZE) {
     if (( *a++ & ~*b++ ) != 0) return 0;
     bits -= X2C_LSET_SIZE;
   }
   if (bits == 0) return 1;
   mask = (1 << bits) - 1;
   return ((( *a & mask ) & (~( *b & mask ))) == 0);
   return 0;
} /* end X2C_SET_LEQ() */


extern LSET X2C_INCL(LSET set, X2C_CARD32 i, X2C_CARD16 bits)
{
   if (i>=(X2C_CARD32)bits) X2C_TRAP(1L);
   set[(int)i/X2C_LSET_SIZE] |= 1L << ((int)i%X2C_LSET_SIZE);
   return set;
} /* end X2C_INCL() */


extern LSET X2C_EXCL(LSET set, X2C_CARD32 i, X2C_CARD16 bits)
{
   if (i>=(X2C_CARD32)bits) X2C_TRAP(1L);
   set[(int)i/X2C_LSET_SIZE] &= ~(1L << ((int)i%X2C_LSET_SIZE));
   return set;
} /* end X2C_EXCL() */


extern LSET X2C_LONGSET(LSET set, X2C_CARD32 a, X2C_CARD32 b,
                X2C_CARD16 bits)
{
   if ((a>b || a>=(X2C_CARD32)bits) || b>=(X2C_CARD32)bits) X2C_TRAP(1L);
   while (a <= b) { set[(int)a/X2C_LSET_SIZE] |= 1L << ((int)a%X2C_LSET_SIZE)
                ; ++a; }
   return set;
} /* end X2C_LONGSET() */


extern X2C_BOOLEAN X2C_INL(X2C_CARD32 i, X2C_CARD16 bits, LSET set)
{
   if (i<(X2C_CARD32)bits) {
      return (set[(int)i/X2C_LSET_SIZE] & (1L << ((int)i%X2C_LSET_SIZE)))
                != 0;
   }
   return 0;
} /* end X2C_INL() */


extern LSET X2C_ROTL(LSET res, LSET a, X2C_INT16 length, X2C_INT32 n)
{
   X2C_INT16 j;
   X2C_INT16 i;
   i = 0;
   j = 0;
   memset(res,0,(length+X2C_LSET_SIZE-1)/X2C_LSET_SIZE*(X2C_LSET_SIZE/8));
   if (n>=0L) {
      j=(short)(n % length);
      for (i=0; i<length; i++)
        if (a[i/X2C_LSET_SIZE] & (1<<(i%X2C_LSET_SIZE)))
            res[(i+j)%length/X2C_LSET_SIZE]|=1<<((i+j)%length%X2C_LSET_SIZE);
   }
   else {
      j=(short)(-n % length);
      for (i=0; i<length; i++)
        if (a[(i+j)%length/X2C_LSET_SIZE] & (1<<((i+j)%length%X2C_LSET_SIZE))
                )
            res[i/X2C_LSET_SIZE]|=1<<(i%X2C_LSET_SIZE);
   }
   return res;
} /* end X2C_ROTL() */


extern LSET X2C_LSHL(LSET res, LSET a, X2C_INT16 length, X2C_INT32 n)
{
   X2C_INT16 j;
   X2C_INT16 i;
   i = 0;
   j = 0;
   memset(res,0,(length+X2C_LSET_SIZE-1)/X2C_LSET_SIZE*(X2C_LSET_SIZE/8));
   if (n>=0L) {
      for (i=0; i+n<length; i++)
        if (a[i/X2C_LSET_SIZE] & (1<<(i%X2C_LSET_SIZE)))
            res[(i+n)/X2C_LSET_SIZE]|=1<<((i+n)%X2C_LSET_SIZE);
   }
   else {
      for (i=-n; i<length; i++)
        if (a[i/X2C_LSET_SIZE] & (1<<(i%X2C_LSET_SIZE)))
            res[(i+n)/X2C_LSET_SIZE]|=1<<((i+n)%X2C_LSET_SIZE);
   }
   return res;
} /* end X2C_LSHL() */

