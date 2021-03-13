/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef complex_H_
#define complex_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

struct complex_Complex;


struct complex_Complex {
   float Re;
   float Im;
};

#define complex_PI 3.1415926536

extern void complex_cmplx(struct complex_Complex *, float, float);

extern float complex_cabs(struct complex_Complex);

extern float complex_arc(struct complex_Complex);

extern void complex_cadd(struct complex_Complex *, struct complex_Complex,
                struct complex_Complex);

extern void complex_csub(struct complex_Complex *, struct complex_Complex,
                struct complex_Complex);

extern void complex_cmul(struct complex_Complex *, struct complex_Complex,
                struct complex_Complex);

extern void complex_cdiv(struct complex_Complex *, struct complex_Complex,
                struct complex_Complex);

extern void complex_csqrt(struct complex_Complex *, struct complex_Complex);

extern void complex_csin(struct complex_Complex *, struct complex_Complex);

extern void complex_ccos(struct complex_Complex *, struct complex_Complex);

extern void complex_cexp(struct complex_Complex *, struct complex_Complex);

extern void complex_cln(struct complex_Complex *, struct complex_Complex);


extern void complex_BEGIN(void);


#endif /* complex_H_ */
