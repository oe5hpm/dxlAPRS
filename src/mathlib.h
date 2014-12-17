/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef mathlib_H_
#define mathlib_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

#define mathlib_PI 3.1415926535898

#define mathlib_e 2.718281828459

extern float mathlib_epsilon;

extern float mathlib_Sin(float);

extern float mathlib_Cos(float);

extern float mathlib_Tan(float);

extern float mathlib_Cot(float);

extern float mathlib_ASin(float);

extern float mathlib_ACos(float);

extern float mathlib_ATan(float);

extern float mathlib_Log(float);

extern float mathlib_Exp(float);

extern float mathlib_Sqrt(float);

extern float mathlib_Power(float, float);

extern long mathlib_Round(float);

extern float mathlib_Floor(float);


extern void mathlib_BEGIN(void);


#endif /* mathlib_H_ */
