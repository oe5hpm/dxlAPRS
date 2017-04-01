/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef frameio_H_
#define frameio_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef l2_H_
#include "l2.h"
#endif

#define frameio_UDPSOCKS 15

struct frameio_UDPSOCK;


struct frameio_UDPSOCK {
   long fd;
   unsigned long ipnum;
   unsigned long fromport;
   unsigned long toport;
   unsigned long dcdretry;
   unsigned long txbufretry;
   char checkip;
   char dcd;
   char hastxdata;
};

extern char frameio_crcok;

extern struct frameio_UDPSOCK frameio_udpsocks0[15];

extern char frameio_GetFrame(unsigned long);

extern void frameio_SendFrame(unsigned long, unsigned long, char [],
                unsigned long, unsigned long, l2_pDATA);

extern char frameio_Sending(unsigned long);

extern char frameio_DCD(unsigned long);

extern void frameio_Modempoll(unsigned long);

extern void frameio_Init(void);


extern void frameio_BEGIN(void);


#endif /* frameio_H_ */
