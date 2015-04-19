/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef cfileio_H_
#define cfileio_H_
#include "X2C.h"

extern long cOpenAppendLong(char []);

extern long cOpenAppend(char []);

extern long cOpenWrite(char []);

extern long cOpenReadLong(char []);

extern long cOpenRead(char []);

extern long cOpenRW(char []);

extern long cOpenNONBLOCK(char []);


#endif /* cfileio_H_ */
