/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosBalls.c Feb  3 14:31:19 2012" */
#include "xosBalls.h"
#define xosBalls_C_
#include "xrtsOS.h"
#include "xrnMman.h"

#define hardPageSize 4096

static X2C_CARD32 bSize;

static X2C_ADDRESS heapBase;

static X2C_CARD32 numBlocks;


extern X2C_ADDRESS X2C_initBalls(X2C_CARD32 nBlocks, X2C_CARD32 blockSize)
{
   bSize = blockSize;
   heapBase = mmap(0, nBlocks*bSize, 0UL, 34UL, -1L, 0UL);
   return heapBase;
} /* end X2C_initBalls() */


extern X2C_ADDRESS X2C_allocBlock(X2C_ADDRESS adr)
{
   mprotect(adr, bSize, 3UL);
   return adr;
} /* end X2C_allocBlock() */


extern void X2C_freeBlock(X2C_ADDRESS adr)
{
   mprotect(adr, bSize, 0UL);
} /* end X2C_freeBlock() */


extern void X2C_freeAll(void)
{
   munmap(heapBase, numBlocks*bSize);
} /* end X2C_freeAll() */

