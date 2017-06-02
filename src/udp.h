/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef udp_H_
#define udp_H_

extern long openudp(void);

extern long bindudp(long, unsigned long);

extern long socknonblock(long);

extern long udpreceive(long, char [], long, uint32_t *, uint32_t *);

extern long udpreceiveblock(long, char [], long, uint32_t *, uint32_t *);

extern long udpsend(long, char [], long, unsigned long, unsigned long);


#endif /* udp_H_ */
