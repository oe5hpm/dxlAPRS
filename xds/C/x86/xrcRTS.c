/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrcRTS.c Feb  3 14:30:28 2012" */
#include "xrcRTS.h"
#define xrcRTS_C_
#include "xmRTS.h"
#include "M2EXCEPTION.h"
#include "X2C.h"
#include "xPOSIX.h"


extern X2C_BOOLEAN X2C_IN(X2C_CARD32 i, X2C_CARD16 bits, X2C_CARD32 set)
{
   if (i<(X2C_CARD32)bits) return (((1L << (int)i) & set) != 0);
   return 0;
} /* end X2C_IN() */


extern X2C_CARD32 X2C_SET(X2C_CARD32 a, X2C_CARD32 b, X2C_CARD16 bits)
{
   if ((a>b || a>=(X2C_CARD32)bits) || b>=(X2C_CARD32)bits) X2C_TRAP(1L);
   return ((X2C_SET32) ((2L<<(int)b) - (1L<<(int)a)));
   return 0UL;
} /* end X2C_SET() */


extern void X2C_PCOPY(X2C_pVOID * p, size_t size)
{
   X2C_ADDRESS a;
   X2C_ALLOCATE(&a, size);
   if (a==0) X2C_TRAP((X2C_INT32)X2C_noMemoryException);
   memcpy(a, (X2C_ADDRESS)*p, size);
   *p = (X2C_pVOID)a;
} /* end X2C_PCOPY() */


extern void X2C_PFREE(X2C_pVOID p)
{
   X2C_ADDRESS a;
   a = (X2C_ADDRESS)p;
   X2C_DEALLOCATE(&a);
} /* end X2C_PFREE() */


extern X2C_PROTECTION X2C_PROT(void)
{
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   return current->prot;
} /* end X2C_PROT() */

