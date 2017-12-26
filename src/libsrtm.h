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

extern uint32_t libsrtm_srtmmem;

extern uint32_t libsrtm_srtmmaxmem;

extern char libsrtm_srtmdir[1024];

extern int32_t libsrtm_opensrtm(uint8_t, uint32_t, uint32_t);

extern float libsrtm_getsrtm(struct aprsstr_POSITION, uint32_t,
                float *);

extern void libsrtm_initsrtm(void);

extern void libsrtm_closesrtmfile(void);


extern void libsrtm_BEGIN(void);


#endif /* libsrtm_H_ */
