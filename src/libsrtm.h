/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef libsrtm_H_
#define libsrtm_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* get altitude out of srtm files directory tree by oe5dxl */
#define libsrtm_ATTRWATER 3
/* srtm tag number */

#define libsrtm_ATTRWOOD 2
/* srtm tag number */

#define libsrtm_ATTRURBAN 1
/* srtm tag number */

struct libsrtm_METAINFO;


struct libsrtm_METAINFO {
   char withslant;
   char aliasattr;
   float slantx;
   float slanty;
   float attrweights[8];
};

typedef struct libsrtm_METAINFO * libsrtm_pMETAINFO;

extern uint32_t libsrtm_srtmmem;

extern uint32_t libsrtm_srtmmaxmem;

extern char libsrtm_srtmdir[1024];

extern char libsrtm_bicubic; /* only for compatibility */

extern int32_t libsrtm_opensrtm(uint8_t, uint32_t, uint32_t);

extern float libsrtm_getsrtm(struct aprsstr_POSITION, uint32_t,
                float *);

extern float libsrtm_getsrtmlong(double, double, uint32_t,
                char, float *, uint8_t *, libsrtm_pMETAINFO);

extern void libsrtm_initsrtm(void);

extern void libsrtm_closesrtmfile(void);

extern float libsrtm_egm96(struct aprsstr_POSITION, char *);
/* read and interpolate geoid correction from datafile */


extern void libsrtm_BEGIN(void);


#endif /* libsrtm_H_ */
