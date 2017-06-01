/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef Select_H_
#define Select_H_
#include "X2C.h"

extern long selectrw(unsigned long, unsigned long);

extern long selectr(unsigned long, unsigned long);

extern void fdclr(void);

extern void fdsetr(unsigned long);

extern char issetr(unsigned long);

extern void fdsetw(unsigned long);

extern char issetw(unsigned long);

extern long selectrwt(uint32_t *, uint32_t *);


#endif /* Select_H_ */
