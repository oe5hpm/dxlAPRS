/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef IOc_H_
#define IOc_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

extern void IOc_WrChar(char);

extern void IOc_WrStr(char [], unsigned long);

extern void IOc_WrLn(void);

extern void IOc_WrStrLn(char [], unsigned long);

extern void IOc_WrInt(long, unsigned long);

extern void IOc_WrCard(unsigned long, unsigned long);

extern void IOc_WrHex(unsigned long, unsigned long);


extern void IOc_BEGIN(void);


#endif /* IOc_H_ */
