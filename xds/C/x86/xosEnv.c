/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosEnv.c Feb  3 14:30:28 2012" */
#include "xosEnv.h"
#define xosEnv_C_
#include "xmRTS.h"
#include "xPOSIX.h"
#include "X2C.h"

typedef X2C_CHAR * PSTR;


extern X2C_CARD32 X2C_EnvStringLength(X2C_pCHAR name)
{
   X2C_pCHAR p;
   p = (X2C_pCHAR)getenv(name);
   if (p==0) return 0UL;
   return strlen(p);
} /* end X2C_EnvStringLength() */


extern void X2C_EnvString(X2C_pCHAR name, X2C_pCHAR buf, X2C_CARD32 blen)
{
   X2C_pCHAR p;
   PSTR t;
   PSTR f;
   X2C_CARD32 i;
   p = (X2C_pCHAR)getenv(name);
   if (p==0) return;
   f = (PSTR)p;
   t = (PSTR)buf;
   i = 0UL;
   while (i<blen && f[i]) {
      t[i] = f[i];
      ++i;
   }
   if (i<blen) t[i] = 0;
} /* end X2C_EnvString() */

static X2C_CHAR EmptyName[1] = "";


extern X2C_pCHAR X2C_GetProgramName(void)
{
   if (X2C_argc==0) return EmptyName;
   else return (X2C_pCHAR)(*X2C_argv);
   return 0;
} /* end X2C_GetProgramName() */

