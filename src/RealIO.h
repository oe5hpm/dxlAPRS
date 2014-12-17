/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* XDS v2.51: Copyright (c) 1999-2003 Excelsior, LLC. All Rights Reserved. */


#ifndef RealIO_H_
#define RealIO_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef IOChan_H_
#include "IOChan.h"
#endif

extern void RealIO_ReadReal(IOChan_ChanId, X2C_REAL *);

extern void RealIO_WriteFloat(IOChan_ChanId, X2C_REAL, X2C_CARD32,
                X2C_CARD32);

extern void RealIO_WriteEng(IOChan_ChanId, X2C_REAL, X2C_CARD32, X2C_CARD32);

extern void RealIO_WriteFixed(IOChan_ChanId, X2C_REAL, X2C_INT32,
                X2C_CARD32);

extern void RealIO_WriteReal(IOChan_ChanId, X2C_REAL, X2C_CARD32);


extern void RealIO_BEGIN(void);


#endif /* RealIO_H_ */
