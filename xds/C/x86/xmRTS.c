/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xmRTS.c Feb  3 14:30:28 2012" */
#include "xmRTS.h"
#define xmRTS_C_
#include "xrsetjmp.h"













X2C_XSource X2C_rtsSource;
X2C_XSource X2C_assertSrc;
X2C_MD X2C_MODULES;
X2C_INT32 X2C_hline;
X2C_BOOLEAN XDSLIB_INITIALIZATION_FAILED;
X2C_BOOLEAN SUPPRESS_XDSLIB_FINALIZATION;
X2C_CARD32 X2C_objects;
X2C_CARD32 X2C_busymem;
X2C_CARD32 X2C_busylheap;
X2C_CARD32 X2C_smallbusy;
X2C_CARD32 X2C_normalbusy;
X2C_CARD32 X2C_largebusy;
X2C_CARD32 X2C_usedmem;
X2C_CARD32 X2C_smallused;
X2C_CARD32 X2C_normalused;
X2C_CARD32 X2C_usedlheap;
X2C_CARD32 X2C_maxmem;
X2C_CARD32 X2C_threshold;
X2C_TD x2c_td_null;
X2C_TD x2c_td_ptr;
X2C_BOOLEAN X2C_fs_init;
X2C_CARD32 X2C_MaxGCTimePercent;
X2C_BOOLEAN X2C_GCThrashWarning;

extern void X2C_INIT_RTS(void)
{
} /* end X2C_INIT_RTS() */

extern void X2C_MODULEXE(X2C_MD, X2C_ADDRESS);


extern void X2C_MODULE(X2C_MD md)
{
   X2C_MODULEXE(md, 0);
} /* end X2C_MODULE() */

