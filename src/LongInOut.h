/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef LongInOut_H_
#define LongInOut_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

extern X2C_BOOLEAN LongInOut_Done;

extern void LongInOut_ReadReal(X2C_LONGREAL *);

extern void LongInOut_WriteReal(X2C_LONGREAL, X2C_CARD32);


extern void LongInOut_BEGIN(void);


#endif /* LongInOut_H_ */
