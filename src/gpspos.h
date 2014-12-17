/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef gpspos_H_
#define gpspos_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

struct gpspos_SAT;


struct gpspos_SAT {
   unsigned long prn;
   unsigned long almidx;
   long rang;
   long rang1;
   long rang3;
   long rangabs;
   long lastd;
   long lastd1;
   long freq0;
   double userspeed;
   char badspeed;
};

typedef struct gpspos_SAT gpspos_SATS[12];

extern long gpspos_getposit(unsigned long, unsigned long *, gpspos_SATS,
                double, double, double, double *, double *, double *,
                double *, double *, double *, float *, float *);

extern char gpspos_readalmanach(char [], unsigned long, char [],
                unsigned long, char [], unsigned long, unsigned long);


extern void gpspos_BEGIN(void);


#endif /* gpspos_H_ */
