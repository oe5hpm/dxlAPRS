/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)XDSRTL.c Feb  3 14:30:28 2012" */
#include "XDSRTL.h"
#define XDSRTL_C_
#include "xmRTS.h"


extern void XDSRTL_Init(PINT argc, PPCHAR argv, X2C_INT32 gcauto,
                X2C_INT32 gcthreshold, X2C_INT32 heaplimit)
{
   X2C_BEGIN(argc, (X2C_ppCHAR)argv, gcauto, gcthreshold, heaplimit);
} /* end XDSRTL_Init() */


extern void XDSRTL_Exit(void)
{
   X2C_EXIT();
} /* end XDSRTL_Exit() */

