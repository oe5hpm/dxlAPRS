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
   int32_t fd;
   uint32_t ipnum;
   uint32_t fromport;
   uint32_t toport;
   uint32_t dcdretry;
   uint32_t txbufretry;
   char checkip;
   char dcd;
   char hastxdata;
   uint32_t dupcnt; /* circular write pointer */
   uint32_t dupcrcs[32]; /* circular crc of frames buffer*/
};

extern char frameio_crcok;

extern struct frameio_UDPSOCK frameio_udpsocks0[15];

extern char frameio_GetFrame(uint32_t);

extern void frameio_SendFrame(uint32_t, uint32_t, char [],
                uint32_t, uint32_t, l2_pDATA);

extern char frameio_Sending(uint32_t);

extern char frameio_DCD(uint32_t);

extern void frameio_Modempoll(uint32_t);

extern void frameio_Init(void);


extern void frameio_BEGIN(void);


#endif /* frameio_H_ */
