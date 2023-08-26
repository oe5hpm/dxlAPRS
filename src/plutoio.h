/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef plutoio_H_
#define plutoio_H_
#include "X2C.h"

typedef short * pIQBUF;

extern int32_t initiio(uint32_t, uint32_t, uint32_t, uint32_t,
                uint32_t, uint32_t, uint32_t, uint32_t);

extern int32_t Initiio(char []);

extern void closeiio(void);

extern int32_t receive(pIQBUF);

extern int32_t transmitbuf(void);

extern void PlutoTxOff(char);

extern void PlutoRxOff(char);

extern void setPlutoGpo(int32_t);

extern int32_t startrx(uint32_t);

extern int32_t starttx(uint32_t, pIQBUF *);

extern int32_t setrxgainmode(char []);

extern int32_t setrxgain(short);

extern int32_t settxgain(short);

extern int32_t setddsfreq(int32_t);

extern int32_t setfreq(double, char);

extern int32_t setbw(uint32_t, char);

extern int32_t setsampel(uint32_t, char);

extern int32_t setport(char [], char);

extern int32_t settxbufs(uint32_t);

extern int32_t firenable(char);


#endif /* plutoio_H_ */
