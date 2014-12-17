/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef ptty_H_
#define ptty_H_
#include "X2C.h"

extern long getptsname(long, X2C_ADDRESS, unsigned long);

extern long grantpts(long);

extern long unlockpts(long);


#endif /* ptty_H_ */
