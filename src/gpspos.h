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
   uint32_t prn;
   uint32_t almidx;
   int32_t rang;
   int32_t rang1;
   int32_t rang3;
   int32_t rangabs;
   int32_t lastd;
   int32_t lastd1;
   int32_t freq0;
   double userspeed;
   char badspeed;
};

typedef struct gpspos_SAT gpspos_SATS[12];

extern int32_t gpspos_getposit(uint32_t, uint32_t *, gpspos_SATS,
                double, double, double, double *,
                double *, double *, double *,
                double *, double *, float *, float *,
                uint32_t *);

extern char gpspos_readalmanach(char [], uint32_t, char [],
                uint32_t, char [], uint32_t, uint32_t,
                uint32_t *, char);


extern void gpspos_BEGIN(void);


#endif /* gpspos_H_ */
