/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrcIncDec.c Feb  3 14:30:28 2012" */
#include "xrcIncDec.h"
#define xrcIncDec_C_
#include "X2C.h"
#include "M2EXCEPTION.h"


extern X2C_CHAR X2C_INCC(X2C_CHAR * x, X2C_CARD8 y, X2C_CHAR min0,
                X2C_CHAR max0)
{
   X2C_INT16 i;
   i = (X2C_INT16)(X2C_CARD8)*x+(X2C_INT16)y;
   if ((X2C_INT32)i<(X2C_INT32)(X2C_CARD32)(X2C_CARD8)min0 || (X2C_INT32)
                i>(X2C_INT32)(X2C_CARD32)(X2C_CARD8)max0) X2C_TRAP(1L);
   *x = (X2C_CHAR)i;
   return *x;
} /* end X2C_INCC() */


extern X2C_INT8 X2C_INCS(X2C_INT8 * x, X2C_INT8 y, X2C_INT8 min0,
                X2C_INT8 max0)
{
   X2C_INT16 i;
   i = (X2C_INT16)*x+(X2C_INT16)y;
   if (i<(X2C_INT16)min0 || i>(X2C_INT16)max0) X2C_TRAP(1L);
   *x = (X2C_INT8)i;
   return *x;
} /* end X2C_INCS() */


extern X2C_INT16 X2C_INCI(X2C_INT16 * x, X2C_INT16 y, X2C_INT16 min0,
                X2C_INT16 max0)
{
   X2C_INT32 i;
   i = (X2C_INT32)*x+(X2C_INT32)y;
   if (i<(X2C_INT32)min0 || i>(X2C_INT32)max0) X2C_TRAP(1L);
   *x = (X2C_INT16)i;
   return *x;
} /* end X2C_INCI() */


extern X2C_INT32 X2C_INC(X2C_INT32 * x, X2C_INT32 y, X2C_INT32 min0,
                X2C_INT32 max0)
{
   X2C_INT32 i;
   if (y>0L) {
      if (X2C_max_longint-y<*x) X2C_TRAP(1L);
   }
   else if (X2C_min_longint-y>*x) X2C_TRAP(1L);
   i = *x+y;
   if (i<min0 || i>max0) X2C_TRAP(1L);
   *x = i;
   return *x;
} /* end X2C_INC() */


extern X2C_CARD8 X2C_INCUS(X2C_CARD8 * x, X2C_CARD8 y, X2C_CARD8 min0,
                X2C_CARD8 max0)
{
   X2C_CARD16 i;
   i = (X2C_CARD16)*x+(X2C_CARD16)y;
   if (i<(X2C_CARD16)min0 || i>(X2C_CARD16)max0) X2C_TRAP(1L);
   *x = (X2C_CARD8)i;
   return *x;
} /* end X2C_INCUS() */


extern X2C_CARD16 X2C_INCUI(X2C_CARD16 * x, X2C_CARD16 y, X2C_CARD16 min0,
                X2C_CARD16 max0)
{
   X2C_CARD32 i;
   i = (X2C_CARD32)*x+(X2C_CARD32)y;
   if (i<(X2C_CARD32)min0 || i>(X2C_CARD32)max0) X2C_TRAP(1L);
   *x = (X2C_CARD16)i;
   return *x;
} /* end X2C_INCUI() */


extern X2C_CARD32 X2C_INCU(X2C_CARD32 * x, X2C_CARD32 y, X2C_CARD32 min0,
                X2C_CARD32 max0)
{
   X2C_CARD32 i;
   if (X2C_max_longcard-y<*x) X2C_TRAP(1L);
   i = *x+y;
   if (i<min0 || i>max0) X2C_TRAP(1L);
   *x = i;
   return *x;
} /* end X2C_INCU() */


extern X2C_CHAR X2C_DECC(X2C_CHAR * x, X2C_CARD8 y, X2C_CHAR min0,
                X2C_CHAR max0)
{
   X2C_INT16 i;
   i = (X2C_INT16)(X2C_CARD8)*x-(X2C_INT16)y;
   if ((X2C_INT32)i<(X2C_INT32)(X2C_CARD32)(X2C_CARD8)min0 || (X2C_INT32)
                i>(X2C_INT32)(X2C_CARD32)(X2C_CARD8)max0) X2C_TRAP(1L);
   *x = (X2C_CHAR)i;
   return *x;
} /* end X2C_DECC() */


extern X2C_INT8 X2C_DECS(X2C_INT8 * x, X2C_INT8 y, X2C_INT8 min0,
                X2C_INT8 max0)
{
   X2C_INT16 i;
   i = (X2C_INT16)*x-(X2C_INT16)y;
   if (i<(X2C_INT16)min0 || i>(X2C_INT16)max0) X2C_TRAP(1L);
   *x = (X2C_INT8)i;
   return *x;
} /* end X2C_DECS() */


extern X2C_INT16 X2C_DECI(X2C_INT16 * x, X2C_INT16 y, X2C_INT16 min0,
                X2C_INT16 max0)
{
   X2C_INT32 i;
   i = (X2C_INT32)*x-(X2C_INT32)y;
   if (i<(X2C_INT32)min0 || i>(X2C_INT32)max0) X2C_TRAP(1L);
   *x = (X2C_INT16)i;
   return *x;
} /* end X2C_DECI() */


extern X2C_INT32 X2C_DEC(X2C_INT32 * x, X2C_INT32 y, X2C_INT32 min0,
                X2C_INT32 max0)
{
   if (y<0L) {
      if (X2C_max_longint+y<*x) X2C_TRAP(1L);
   }
   else if (X2C_min_longint+y>*x) X2C_TRAP(1L);
   *x -= y;
   if (*x<min0 || *x>max0) X2C_TRAP(1L);
   return *x;
} /* end X2C_DEC() */


extern X2C_CARD8 X2C_DECUS(X2C_CARD8 * x, X2C_CARD8 y, X2C_CARD8 min0,
                X2C_CARD8 max0)
{
   if (y>*x) X2C_TRAP(1L);
   *x -= y;
   if (*x<min0 || *x>max0) X2C_TRAP(1L);
   return *x;
} /* end X2C_DECUS() */


extern X2C_CARD16 X2C_DECUI(X2C_CARD16 * x, X2C_CARD16 y, X2C_CARD16 min0,
                X2C_CARD16 max0)
{
   if (y>*x) X2C_TRAP(1L);
   *x -= y;
   if (*x<min0 || *x>max0) X2C_TRAP(1L);
   return *x;
} /* end X2C_DECUI() */


extern X2C_CARD32 X2C_DECU(X2C_CARD32 * x, X2C_CARD32 y, X2C_CARD32 min0,
                X2C_CARD32 max0)
{
   X2C_CARD32 i;
   if (y>*x) X2C_TRAP(1L);
   i = *x-y;
   if (i<min0 || i>max0) X2C_TRAP(1L);
   *x = i;
   return *x;
} /* end X2C_DECU() */

