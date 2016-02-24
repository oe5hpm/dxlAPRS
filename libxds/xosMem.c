/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosMem.c Feb  3 14:31:18 2012" */
#include "xosMem.h"
#define xosMem_C_
#include "xPOSIX.h"


extern X2C_ADDRESS X2C_AllocMem(X2C_CARD32 size)
{
   return malloc(size);
} /* end X2C_AllocMem() */


extern void X2C_InitMem(void)
{
} /* end X2C_InitMem() */


extern X2C_CARD32 X2C_GetAvailablePhysicalMemory(void)
{
   return X2C_max_longcard;
} /* end X2C_GetAvailablePhysicalMemory() */

