/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef aprstat_H_
#define aprstat_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef maptool_H_
#include "maptool.h"
#endif

struct aprstat_LASTVAL;


struct aprstat_LASTVAL {
   float temp;
   float hyg;
   float baro;
   float winds;
   float winddir;
   float gust;
   float rain;
   float rain24;
   float lumi;
   float siev;
};

extern void aprstat_kmhist(maptool_pIMAGE *, aprsdecode_pOPHIST,
                char *);

extern void aprstat_althist(maptool_pIMAGE *, aprsdecode_pOPHIST,
                char *, float *, uint32_t *, uint32_t *,
                uint32_t *, uint32_t *);

extern void aprstat_btimehist(maptool_pIMAGE *, aprsdecode_pOPHIST);

extern void aprstat_wxgraph(maptool_pIMAGE *, aprsdecode_pOPHIST, uint32_t,
                 uint16_t *, struct aprstat_LASTVAL *);


extern void aprstat_BEGIN(void);


#endif /* aprstat_H_ */
