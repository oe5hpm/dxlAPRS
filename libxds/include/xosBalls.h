/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosBalls_H_
#define xosBalls_H_
#include "X2C.h"

extern X2C_ADDRESS X2C_initBalls(X2C_CARD32, X2C_CARD32);

extern X2C_ADDRESS X2C_allocBlock(X2C_ADDRESS);

extern void X2C_freeBlock(X2C_ADDRESS);

extern void X2C_freeAll(void);


#endif /* xosBalls_H_ */
