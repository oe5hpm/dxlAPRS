/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef reedsolomon_H_
#define reedsolomon_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

typedef char reedsolomon_DTYP;

extern void reedsolomon_encode(char [], unsigned long, char [],
                unsigned long);

extern long reedsolomon_decode(char [], unsigned long, unsigned long [],
                unsigned long, unsigned long);

extern void reedsolomon_CRC(char [], unsigned long, unsigned short *,
                unsigned long);


extern void reedsolomon_BEGIN(void);


#endif /* reedsolomon_H_ */
