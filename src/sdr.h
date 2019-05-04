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

#define sdr_mSCAN "S"

typedef short sdr_AUDIOSAMPLE[65536];

typedef short * sdr_pAUDIOSAMPLE;

struct sdr_TAP;


struct sdr_TAP {
   int32_t uc1;
   int32_t uc2;
   int32_t il;
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
   uint32_t df;
   uint32_t dffrac;
   uint32_t width;
   uint32_t agc;
   uint32_t idx;
   int32_t maxafc;
   char squelch;
   char afcrun;
   char modulation;
   sdr_pAUDIOSAMPLE samples;
   uint32_t phase;
   uint32_t bfo;
   uint32_t fine;
   struct sdr_TAP tapre;
   struct sdr_TAP tapim;
   int32_t median;
   int32_t afckhz;
   float w1;
   float lastlev;
   float rssi;
   float afcrssi;
   float sqsum;
   float a1;
   uint32_t fracphase;
   uint32_t bfophase;
   struct sdr_SSBTAP ssbre;
   struct sdr_SSBTAP ssbim;
   float ssbfgq;
   float ssbfg;
   float agcspeed;
};

typedef struct sdr_RX * sdr_pRX;

extern int32_t sdr_getsdr(uint32_t, sdr_pRX [], uint32_t);

extern void sdr_setparm(uint32_t, uint32_t);

extern char sdr_startsdr(char [], uint32_t, char [],
                uint32_t, uint32_t, uint32_t, char);


extern void sdr_BEGIN(void);


#endif /* sdr_H_ */
