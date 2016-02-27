/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __OSIC_H__
#define __OSIC_H__

#include <stdint.h>
#include "X2C.h"

void osic_WrLn(void);

void osic_WrStr(char s[], unsigned long s_len);

void osic_WrStrLn(char s[], unsigned long s_len);

void osic_WrUINT32(uint32_t x, unsigned long witdh);

void osic_WrFixed(float x, long place, unsigned long witdh);

void osic_WrHex(unsigned long n, unsigned long f);

long osic_OpenAppendLong(char fn[], unsigned long fn_len);

long osic_OpenAppend(char fn[], unsigned long fn_len);

long osic_OpenWrite(char fn[], unsigned long fn_len);

long osic_OpenReadLong(char fn[], unsigned long fn_len);

long osic_OpenRead(char fn[], unsigned long fn_len);

long osic_OpenRW(char fn[], unsigned long fn_len);

long osic_OpenNONBLOCK(char fn[], unsigned long fn_len);

char osic_FdValid(long fd);

void osic_Close(long fd);

void osic_CloseSock(long fd);

long osic_RdBin(long fd,
		      char buf[], unsigned long buf_len,
		      unsigned long size);

void osic_WrBin(long fd, char buf[],
		      unsigned long buf_len,
		      unsigned long size);

void osic_Rename(char fname[], unsigned long fname_len,
		       char newname[], unsigned long newname_len);

int osic_Size(int fd);

void osic_Seek(long fd, unsigned long pos);

void osic_Seekcur(long fd, long rel);

void osic_BEGIN(void);

X2C_REAL RealMath_cos(X2C_REAL x);

X2C_REAL RealMath_ln(X2C_REAL x);

X2C_REAL RealMath_sqrt(X2C_REAL x);

X2C_REAL RealMath_sin(X2C_REAL x);

X2C_REAL RealMath_arctan(X2C_REAL x);

X2C_REAL RealMath_tan(X2C_REAL x);

X2C_REAL RealMath_exp(X2C_REAL x);

X2C_REAL RealMath_power(X2C_REAL base, X2C_REAL exponent);

X2C_CARD32 TimeConv_time(void);

void Storage_ALLOCATE(X2C_ADDRESS *p, X2C_CARD32 size);

void Storage_DEALLOCATE(X2C_ADDRESS *a, X2C_CARD32 size);

#endif /* __OSIC_H__ */
