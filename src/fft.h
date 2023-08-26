/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef fft_H_
#define fft_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef complex_H_
#include "complex.h"
#endif

extern void fft_Hamming(struct complex_Complex [], uint32_t);

extern void fft_Hann(struct complex_Complex [], uint32_t);

extern void fft_Blackmann(struct complex_Complex [], uint32_t);

extern void fft_Shift(struct complex_Complex [], uint32_t);

extern void fft_Transform(struct complex_Complex [], uint32_t,
                char);


extern void fft_BEGIN(void);


#endif /* fft_H_ */
