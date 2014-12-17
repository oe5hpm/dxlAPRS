/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef XDSRTL_H_
#define XDSRTL_H_
#include "X2C.h"

typedef int * PINT;

typedef X2C_CHAR * * PPCHAR;

extern void XDSRTL_Init(PINT, PPCHAR, X2C_INT32, X2C_INT32, X2C_INT32);

extern void XDSRTL_Exit(void);


#endif /* XDSRTL_H_ */
