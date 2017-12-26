/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef aprspos_H_
#define aprspos_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* get aprs position by OE5DXL */
#define aprspos_PI 3.1415926535

#define aprspos_PI2 6.283185307

#define aprspos_RAD 1.7453292519444E-2

#define aprspos_FEET 0.3048

#define aprspos_KNOTS 1.852
/* nautic miles */

#define aprspos_WKNOTS 1.609
/* wx knots */

#define aprspos_SKNOTS 1.609
/* stormdata knots */

#define aprspos_ENCODEGPS "g"

#define aprspos_ENCODEMICE "m"

#define aprspos_ENCODECOMP "c"

#define aprspos_ENCODEGPSDAO "G"

#define aprspos_ENCODEMICEDAO "M"

#define aprspos_ENCODEAREA "A"

#define aprspos_ENCODEMULTILINE "L"

#define aprspos_EARTH 6370.0

#define aprspos_AREASYMT "\\"
/* area object symbol table */

#define aprspos_AREASYM "l"
/* area object symbol */

extern float aprspos_rad0(float);

extern char aprspos_posvalid(struct aprsstr_POSITION);

extern float aprspos_distance(struct aprsstr_POSITION,
                struct aprsstr_POSITION);

extern float aprspos_azimuth(struct aprsstr_POSITION,
                struct aprsstr_POSITION);

extern void aprspos_GetPos(struct aprsstr_POSITION *, uint32_t *,
                uint32_t *, int32_t *, char *, char *,
                char [], uint32_t, uint32_t, uint32_t, char [],
                 uint32_t, char *);

extern void aprspos_GetSym(char [], uint32_t, char *, char *);
/* symbol out of destination call */

extern void aprspos_wgs84s(float, float, float, float *,
                float *, float *);
/* km */

extern void aprspos_wgs84r(float, float, float, float *,
                float *, float *);
/* km */


extern void aprspos_BEGIN(void);


#endif /* aprspos_H_ */
