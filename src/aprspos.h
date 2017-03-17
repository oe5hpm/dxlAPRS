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

/* get aprs position by OE5DXL */
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

struct aprspos_POSITION;


struct aprspos_POSITION {
   float long0;
   float lat;
};

extern float aprspos_rad0(float);

extern char aprspos_posvalid(struct aprspos_POSITION);

extern float aprspos_distance(struct aprspos_POSITION,
                struct aprspos_POSITION);

extern float aprspos_azimuth(struct aprspos_POSITION,
                struct aprspos_POSITION);

extern void aprspos_GetPos(struct aprspos_POSITION *, unsigned long *,
                unsigned long *, long *, char *, char *, char [],
                unsigned long, unsigned long, unsigned long, char [],
                unsigned long, char *);

extern void aprspos_GetSym(char [], unsigned long, char *, char *);
/* symbol out of destination call */


extern void aprspos_BEGIN(void);


#endif /* aprspos_H_ */
