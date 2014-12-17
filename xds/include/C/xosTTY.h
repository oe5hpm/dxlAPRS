/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosTTY_H_
#define xosTTY_H_
#include "X2C.h"

extern X2C_INT32 X2C_ttyReadNE(X2C_ADDRESS, X2C_CARD32, X2C_CARD32 *);

extern X2C_INT32 X2C_ttyReadLE(X2C_ADDRESS, X2C_CARD32, X2C_CARD32 *);

extern X2C_INT32 X2C_ttyWrite(X2C_ADDRESS, X2C_CARD32);

extern X2C_INT32 X2C_InitTTY(void);


#endif /* xosTTY_H_ */
