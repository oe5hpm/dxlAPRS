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

void osic_flush(void);

void osic_WrLn(void);

void osic_WerrLn(void);

void osic_WrStr(char s[], uint32_t s_len);

void osic_WrStrLn(char s[], uint32_t s_len);

void osic_WerrStr(char s[], uint32_t s_len);

void osic_WerrStrLn(char s[], uint32_t s_len);

void osic_WrUINT32(uint32_t x, uint32_t witdh);

void osic_WrINT32(int32_t x, uint32_t witdh);

void osic_WrFixed(float x, int32_t place, uint32_t witdh);

void osic_WrHex(uint32_t n, uint32_t f);

int osic_getptsname(int fd, char *name, int len);

int osic_grantpts(int fd);

int osic_unlockpts(int fd);

int32_t osic_OpenAppendLong(char fn[], uint32_t fn_len);

int32_t osic_OpenAppend(char fn[], uint32_t fn_len);

int32_t osic_OpenWrite(char fn[], uint32_t fn_len);

int32_t osic_OpenReadLong(char fn[], uint32_t fn_len);

int32_t osic_OpenRead(char fn[], uint32_t fn_len);

int32_t osic_OpenRW(char fn[], uint32_t fn_len);

int32_t osic_OpenNONBLOCK(char fn[], uint32_t fn_len);

char osic_FdValid(int32_t fd);

void osic_Close(int32_t fd);

void osic_CloseSock(int32_t fd);

int32_t osic_RdBin(int32_t fd,
		char buf[], uint32_t buf_len,
		uint32_t size);

void osic_WrBin(int32_t fd, char buf[],
		uint32_t buf_len,
		uint32_t size);

void osic_Rename(char fname[], uint32_t fname_len,
		 char newname[], uint32_t newname_len);

int osic_Size(int fd);

void osic_Seek(int32_t fd, uint32_t pos);

void osic_Seekcur(int32_t fd, int32_t rel);

void osic_Remove(char fname[], uint32_t fname_len, char *done);

char osic_Exists(char fname[], uint32_t fname_len);

int osic_symblink(char *existing, char *newname);

char osic_mkdir(char fname[], uint32_t fname_len, uint32_t perm);

void osic_NextArg(char s[], uint32_t s_len);

void osic_Fill(char *buf, uint32_t len, char byte);

double osic_Random(void);

void X2C_BEGIN(int *argc, char *argv[],
	       int gc_auto, int32_t gc_threshold, int32_t heap_limit);

void X2C_PCOPY(void **ppcpy, size_t size);

int32_t X2C_TRUNCI(double x, int32_t min0, int32_t max0);

uint32_t X2C_TRUNCC(double x, uint32_t min0, uint32_t max0);

float osic_cos(float x);

float osic_ln(float x);

float osic_sqrt(float x);

float osic_sin(float x);

float osic_arctan(float x);

float osic_tan(float x);

float osic_exp(float x);

float osic_power(float base, float exponent);

float osic_arccos(float x);

float osic_floor(float x);

void osic_alloc(X2C_ADDRESS *p, uint32_t size);

void osic_free(X2C_ADDRESS *a, uint32_t size);

uint32_t osic_time(void);

uint32_t X2C_LSH(uint32_t a, int32_t length, int32_t n);

char X2C_IN(uint32_t i, uint32_t bits, uint32_t set);

char X2C_INL(uint32_t i, uint32_t bits, uint32_t *set);

void X2C_INCL(uint32_t *set, uint32_t i, uint32_t bits);

void X2C_EXCL(uint32_t *set, uint32_t i, uint32_t bits);

uint32_t *X2C_COMPLEMENT(uint32_t *res, uint32_t *a, uint32_t length);

uint32_t X2C_SET(uint32_t a, uint32_t b, uint16_t bits);

void X2C_DYNALLOCATE(char **a, size_t size, size_t lens[], size_t dims);

void X2C_DYNDEALLOCATE(char **a);

void *osic_chkptr(void *p);

#endif /* __OSIC_H__ */
