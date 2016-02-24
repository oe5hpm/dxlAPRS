/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosFmtIO_H_
#define xosFmtIO_H_
#include "X2C.h"

extern void X2C_StdOutS(X2C_CHAR [], X2C_CARD32);

extern void X2C_StdOutD(X2C_CARD32, X2C_CARD32);

extern void X2C_StdOutH(X2C_CARD32, X2C_CARD32);

extern void X2C_DecToStr(X2C_CHAR [], X2C_CARD32 *, X2C_CARD32);

extern void X2C_HexToStr(X2C_CHAR [], X2C_CARD32 *, X2C_CARD32);


#endif /* xosFmtIO_H_ */
