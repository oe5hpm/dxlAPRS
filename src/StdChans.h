/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef StdChans_H_
#define StdChans_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef IOChan_H_
#include "IOChan.h"
#endif

typedef IOChan_ChanId StdChans_ChanId;

extern IOChan_ChanId StdChans_StdInChan(void);

extern IOChan_ChanId StdChans_StdOutChan(void);

extern IOChan_ChanId StdChans_StdErrChan(void);

extern IOChan_ChanId StdChans_NullChan(void);

extern IOChan_ChanId StdChans_InChan(void);

extern IOChan_ChanId StdChans_OutChan(void);

extern IOChan_ChanId StdChans_ErrChan(void);

extern void StdChans_SetInChan(IOChan_ChanId);

extern void StdChans_SetOutChan(IOChan_ChanId);

extern void StdChans_SetErrChan(IOChan_ChanId);


extern void StdChans_BEGIN(void);


#endif /* StdChans_H_ */
