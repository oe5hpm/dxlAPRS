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

#include "X2C.h"

void osic_WrLn(void);

void osic_WerrLn(void);

void osic_WrStr(char s[], unsigned long s_len);

void osic_WrStrLn(char s[], unsigned long s_len);

void osic_WerrStr(char s[], unsigned long s_len);

void osic_WerrStrLn(char s[], unsigned long s_len);

void osic_WrUINT32(uint32_t x, unsigned long witdh);

void osic_WrFixed(float x, long place, unsigned long witdh);

void osic_WrHex(unsigned long n, unsigned long f);

int getptsname(int fd, char *name, int len);

int grantpts(int fd);

int unlockpts(int fd);

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

void osic_Remove(X2C_CHAR fname[], X2C_CARD32 fname_len, X2C_BOOLEAN *done);

X2C_BOOLEAN osic_Exists(X2C_CHAR fname[], X2C_CARD32 fname_len);

int osic_symblink(char *existing, char *newname);

void osic_NextArg(char s[], unsigned long s_len);

void osic_Fill(X2C_ADDRESS buf, unsigned long len, char byte);

double osic_Random(void);

void X2C_BEGIN(int *argc, char *argv[],
	       int gc_auto, long gc_threshold, long heap_limit);

void X2C_PCOPY(void **ppcpy, size_t size);

X2C_INT32 X2C_TRUNCI(X2C_LONGREAL x, X2C_INT32 min0, X2C_INT32 max0);

X2C_CARD32 X2C_TRUNCC(X2C_LONGREAL x, X2C_CARD32 min0, X2C_CARD32 max0);

X2C_REAL X2C_DIVR(X2C_REAL a, X2C_REAL b);

X2C_REAL RealMath_cos(X2C_REAL x);

X2C_REAL RealMath_ln(X2C_REAL x);

X2C_REAL RealMath_sqrt(X2C_REAL x);

X2C_REAL RealMath_sin(X2C_REAL x);

X2C_REAL RealMath_arctan(X2C_REAL x);

X2C_REAL RealMath_tan(X2C_REAL x);

X2C_REAL RealMath_exp(X2C_REAL x);

X2C_REAL RealMath_power(X2C_REAL base, X2C_REAL exponent);

void Storage_ALLOCATE(X2C_ADDRESS *p, X2C_CARD32 size);

void Storage_DEALLOCATE(X2C_ADDRESS *a, X2C_CARD32 size);

X2C_CARD32 TimeConv_time(void);

X2C_CARD32 X2C_LSH(X2C_CARD32 a, X2C_INT32 length, X2C_INT32 n);

X2C_BOOLEAN X2C_IN(X2C_CARD32 i, X2C_CARD32 bits, X2C_CARD32 set);

X2C_BOOLEAN X2C_INL(X2C_CARD32 i, X2C_CARD32 bits, X2C_LSET set);

X2C_INT32 X2C_MOD(X2C_INT32 a, X2C_INT32 b);

X2C_LSET X2C_INCL(X2C_LSET set, X2C_CARD32 i, X2C_CARD32 bits);

X2C_LSET X2C_COMPLEMENT(X2C_LSET res, X2C_LSET a, X2C_CARD32 length);

void X2C_DYNALLOCATE(X2C_ADDRESS *a, size_t size, size_t lens[], size_t dims);

void X2C_DYNDEALLOCATE(X2C_ADDRESS *a);

X2C_INT32 X2C_QUO(X2C_INT32 a, X2C_INT32 b);

X2C_LONGREAL X2C_EXPRI(X2C_LONGREAL base, X2C_INT32 ex);

void *osic_chkptr(void *p);

#endif /* __OSIC_H__ */
