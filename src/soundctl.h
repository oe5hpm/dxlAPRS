/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef soundctl_H_
#define soundctl_H_
#include "X2C.h"

extern long samplesize(long, unsigned long);

extern long channels(long, unsigned long);

extern long sampelrate(long, unsigned long);

extern long setfragment(long, unsigned long);

extern void pcmsync(long);

extern long getoutfilled(long);

extern unsigned long getsampelrate(long);

extern unsigned long getmixer(long, long);

extern void setmixer(long, unsigned long, unsigned long);

extern unsigned long recnum(void);


#endif /* soundctl_H_ */
