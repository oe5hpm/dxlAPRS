/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef RealInOut_H_
#define RealInOut_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

extern X2C_BOOLEAN RealInOut_Done;

extern void RealInOut_ReadReal(X2C_REAL *);

extern void RealInOut_WriteReal(X2C_REAL, X2C_CARD32);


extern void RealInOut_BEGIN(void);


#endif /* RealInOut_H_ */
