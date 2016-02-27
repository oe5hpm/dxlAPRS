/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef sdr_H_
#define sdr_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

typedef short sdr_AUDIOSAMPLE[65536];

typedef short * sdr_pAUDIOSAMPLE;

struct sdr_TAP;


struct sdr_TAP {
   long uc1;
   long uc2;
   long il;
};

struct sdr_RX;


struct sdr_RX {
   unsigned long df;
   unsigned long maxafc;
   unsigned long width;
   char squelch;
   unsigned long phase;
   struct sdr_TAP tapre;
   struct sdr_TAP tapim;
   long median;
   long afckhz;
   float w1;
   float lastlev;
   float rssi;
   float sqmed;
   sdr_pAUDIOSAMPLE samples;
};

typedef struct sdr_RX * sdr_pRX;

extern long sdr_getsdr(unsigned long, sdr_pRX [], unsigned long);

extern void sdr_setparm(unsigned long, unsigned long);

extern char sdr_startsdr(char [], unsigned long, char [], unsigned long,
                unsigned long, unsigned long, char);


extern void sdr_BEGIN(void);


#endif /* sdr_H_ */
