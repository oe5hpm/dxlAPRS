/* Copyright (c) xTech 1992,96.  All Rights Reserved */
/*
  XDS library.
  Interface to mathematic functions.
  Note that "ifdefed" functions are implemented in xMathAds.c
*/

#ifndef xMath_H_
#define xMath_H_

#include <math.h>

#define X2C_sin(x)    ((float)sin(x))
#define X2C_cos(x)    ((float)cos(x))
#define X2C_sqrt(x)   ((float)sqrt(x))
#define X2C_tan(x)    ((float)tan(x))
#define X2C_arcsin(x) ((float)asin(x))
#define X2C_arccos(x) ((float)acos(x))
#define X2C_arctan(x) ((float)atan(x))
#define X2C_arctan2(y,x) ((float)atan2(y,x))
#define X2C_exp(x)    ((float)exp(x))
#define X2C_ln(x)     ((float)log(x))
#define X2C_lg(x)     ((float)log10(x))
#define X2C_floor(x)  ((float)floor(x))
#define X2C_pow(x,y)  ((float)pow(x,y))
#define X2C_sinh(x)   ((float)sinh(x))
#define X2C_cosh(x)   ((float)cosh(x))
#define X2C_tanh(x)   ((float)tanh(x))
#define X2C_rem(x,y)  ((float)fmod(x,y))

#ifndef X2C_no_asinh
#define X2C_arcsinh(x) ((float)asinh(x))
#define X2C_arcsinhl asinh
#else
extern float X2C_arcsinh(float);
extern double X2C_arcsinhl(double);
#endif

#ifndef X2C_no_acosh
#define X2C_arccosh(x) ((float)acosh(x))
#define X2C_arccoshl acosh
#else
extern float X2C_arccosh(float);
extern double X2C_arccoshl(double);
#endif

#ifndef X2C_no_atanh
#define X2C_arctanh(x) ((float)atanh(x))
#define X2C_arctanhl atanh
#else
extern float X2C_arctanh(float);
extern double X2C_arctanhl(double);
#endif

#define X2C_sinl     sin
#define X2C_cosl     cos
#define X2C_sqrtl    sqrt
#define X2C_tanl     tan
#define X2C_arcsinl  asin
#define X2C_arccosl  acos
#define X2C_arctanl  atan
#define X2C_arctan2l atan2
#define X2C_expl     exp
#define X2C_lnl      log
#define X2C_lgl      log10
#define X2C_floorl   floor
#define X2C_powl     pow
#define X2C_sinhl    sinh
#define X2C_coshl    cosh
#define X2C_tanhl    tanh
#define X2C_reml     fmod

#if defined(__SC__)
#define X2C_frexp(x,y)    frexp(x,(int *)(y))
#else
#define X2C_frexp    frexp
#endif
#define X2C_ldexp    ldexp
#define X2C_modf     modf

/* folowing function should be ifdefed by C-compiler options */
#define X2C_controlfp(new,mask) (0)

#endif

