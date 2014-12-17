/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef udp_H_
#define udp_H_
#include "X2C.h"

extern long openudp(void);

extern long bindudp(long, unsigned long);

extern long socknonblock(long);

extern long udpreceive(long, char [], long, unsigned long *,
                unsigned long *);

extern long udpreceiveblock(long, char [], long, unsigned long *,
                unsigned long *);

extern long udpsend(long, char [], long, unsigned long, unsigned long);


#endif /* udp_H_ */
