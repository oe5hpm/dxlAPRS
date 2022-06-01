/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef fttw_H_
#define fttw_H_

#include "X2C.h"

struct Complex;

struct Complex {
   float Re;
   float Im;
};


struct INOUT {
   struct Complex * Adr;
   size_t Len0;
};

typedef struct Complex * pINOUT;

extern pINOUT plan(uint32_t);

extern void fft3(void);

extern void clean(pINOUT *);



#endif /* fttw_H_ */
