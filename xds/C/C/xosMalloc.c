/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosMalloc.c Feb  3 14:31:18 2012" */
#include "xosMalloc.h"
#define xosMalloc_C_
#include "xPOSIX.h"


extern X2C_ADDRESS X2C_malloc(X2C_CARD32 size)
{
   return malloc(size);
} /* end X2C_malloc() */


extern void X2C_free(X2C_ADDRESS adr, X2C_CARD32 size)
{
   free(adr);
} /* end X2C_free() */


extern X2C_ADDRESS X2C_gmalloc(X2C_CARD32 size)
{
   return malloc(size);
} /* end X2C_gmalloc() */


extern void X2C_gfree(X2C_ADDRESS p)
{
   free(p);
} /* end X2C_gfree() */


extern void X2C_InitHeap(X2C_CARD32 limit, X2C_BOOLEAN isIncr)
{
} /* end X2C_InitHeap() */


extern void X2C_ZEROMEM(X2C_ADDRESS adr, X2C_CARD32 qsize)
{
   memset(adr, 0, qsize*4UL);
} /* end X2C_ZEROMEM() */


extern void X2C_DestroyHeap(void)
{
} /* end X2C_DestroyHeap() */


extern void xosMalloc_init(void)
{
} /* end xosMalloc_init() */

