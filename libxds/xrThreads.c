/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrThreads.c Feb  3 14:31:18 2012" */
#include "xrThreads.h"
#define xrThreads_C_
#include "xmRTS.h"

static X2C_Coroutine current;


extern void X2C_SetCurrent(X2C_Coroutine c)
{
   current = c;
} /* end X2C_SetCurrent() */


extern X2C_Coroutine X2C_GetCurrent(void)
{
   return current;
} /* end X2C_GetCurrent() */


extern X2C_INT32 X2C_InitThreads(void)
{
   current = 0;
   return 0L;
} /* end X2C_InitThreads() */


extern void X2C_PrepareToGC(void)
{
   X2C_Coroutine c;
   c = current;
   do {
      X2C_CopyJmpBuf(c);
      c = c->fwd;
   } while (c!=current);
} /* end X2C_PrepareToGC() */


extern void X2C_FreeAfterGC(void)
{
} /* end X2C_FreeAfterGC() */

