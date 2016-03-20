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

#define sdr_ZFIRSIZE 256

#define sdr_mSSB "s"

#define sdr_mFM "f"

#define sdr_mAM "a"

typedef short sdr_AUDIOSAMPLE[65536];

typedef short * sdr_pAUDIOSAMPLE;

struct sdr_TAP;


struct sdr_TAP {
   long uc1;
   long uc2;
   long il;
   float ucr1;
   float ucr2;
   float ilr;
};

struct sdr_SSBTAP;


struct sdr_SSBTAP {
   float uc1;
   float uc2;
   float uc3;
   float il;
   float il2;
};

struct sdr_RX;


struct sdr_RX {
   unsigned long df;
   unsigned long dffrac;
   unsigned long width;
   unsigned long agc;
   unsigned long idx;
   long maxafc;
   char squelch;
   char modulation;
   sdr_pAUDIOSAMPLE samples;
   unsigned long phase;
   unsigned long bfo;
   unsigned long fine;
   struct sdr_TAP tapre;
   struct sdr_TAP tapim;
   long median;
   long afckhz;
   float w1;
   float lastlev;
   float rssi;
   float sqsum;
   float a1;
   unsigned long fracphase;
   unsigned long bfophase;
   struct sdr_SSBTAP ssbre;
   struct sdr_SSBTAP ssbim;
   float ssbfgq;
   float ssbfg;
   float agcspeed;
};

typedef struct sdr_RX * sdr_pRX;

extern long sdr_getsdr(unsigned long, sdr_pRX [], unsigned long);

extern void sdr_setparm(unsigned long, unsigned long);

extern char sdr_startsdr(char [], unsigned long, char [], unsigned long,
                unsigned long, unsigned long, char);


extern void sdr_BEGIN(void);


#endif /* sdr_H_ */
