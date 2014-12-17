/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrcChecks.c Feb  3 14:30:28 2012" */
#include "xrcChecks.h"
#define xrcChecks_C_
#include "xmRTS.h"
#include "M2EXCEPTION.h"


extern X2C_CARD16 X2C_CHKINX_F(X2C_CARD32 i, X2C_CARD16 length)
{
   if (i>=(X2C_CARD32)length) X2C_TRAP_F(0L);
   return (X2C_CARD16)i;
} /* end X2C_CHKINX_F() */


extern X2C_CARD32 X2C_CHKINXL_F(X2C_CARD32 i, X2C_CARD32 length)
{
   if (i>=length) X2C_TRAP_F(0L);
   return i;
} /* end X2C_CHKINXL_F() */


extern X2C_INT16 X2C_CHKS_F(X2C_INT16 i)
{
   if (i<0) X2C_TRAP_F(1L);
   return i;
} /* end X2C_CHKS_F() */


extern X2C_INT32 X2C_CHKSL_F(X2C_INT32 i)
{
   if (i<0L) X2C_TRAP_F(1L);
   return i;
} /* end X2C_CHKSL_F() */


extern X2C_INT16 X2C_CHK_F(X2C_INT16 a, X2C_INT16 min0, X2C_INT16 max0)
{
   if (a<min0 || a>max0) X2C_TRAP_F(1L);
   return a;
} /* end X2C_CHK_F() */


extern X2C_INT32 X2C_CHKL_F(X2C_INT32 a, X2C_INT32 min0, X2C_INT32 max0)
{
   if (a<min0 || a>max0) X2C_TRAP_F(1L);
   return a;
} /* end X2C_CHKL_F() */


extern X2C_CARD16 X2C_CHKU_F(X2C_CARD16 a, X2C_CARD16 min0, X2C_CARD16 max0)
{
   if (a<min0 || a>max0) X2C_TRAP_F(1L);
   return a;
} /* end X2C_CHKU_F() */


extern X2C_CARD32 X2C_CHKUL_F(X2C_CARD32 a, X2C_CARD32 min0,
                X2C_CARD32 max0)
{
   if (a<min0 || a>max0) X2C_TRAP_F(1L);
   return a;
} /* end X2C_CHKUL_F() */


extern X2C_pVOID X2C_CHKNIL_F(X2C_pVOID p)
{
   if (p==0) X2C_TRAP_F(3L);
   return p;
} /* end X2C_CHKNIL_F() */


extern X2C_PROC X2C_CHKPROC_F(X2C_PROC p)
{
   if (p==0) X2C_TRAP_F(3L);
   return p;
} /* end X2C_CHKPROC_F() */

