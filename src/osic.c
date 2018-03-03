/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

#include "osic.h"

#undef DEBUG
#ifdef DEBUG
# define DBG(...)	printf(__VA_ARGS__)
#else
# define DBG(...)	do { } while (0)
#endif

#define FNLENCHECK	1

/* Does not exist - and is not necessary - on MacOS */
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

static int osic_argc, argc_delivered;
static char **osic_argv;

void osic_flush(void)
{
	fflush(stdout);
	fflush(stderr);
}

void osic_WrLn(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);
}

void osic_WerrLn(void)
{
	fprintf(stderr, "\n");
	fflush(stderr);
}

void osic_WrStr(char s[], uint32_t s_len)
{
	size_t len;

	len = strnlen(s, s_len);
	fprintf(stdout, "%.*s", len, s);
}

void osic_WrStrLn(char s[], uint32_t s_len)
{
	size_t len;

	len = strnlen(s, s_len);
	fprintf(stdout, "%.*s", len, s);
	osic_WrLn();
}

void osic_WerrStr(char s[], uint32_t s_len)
{
	size_t len;

	len = strnlen(s, s_len);
	fprintf(stderr, "%.*s", len, s);
}

void osic_WerrStrLn(char s[], uint32_t s_len)
{
	size_t len;

	len = strnlen(s, s_len);
	fprintf(stderr, "%.*s", len, s);
	osic_WrLn();
}

void osic_WrUINT32(uint32_t x, uint32_t witdh)
{
	fprintf(stdout, "%*d", (int)witdh, x);
}

void osic_WrINT32(int32_t x, uint32_t witdh)
{
	fprintf(stdout, "%*d", (int)witdh, x);
}

void osic_WrFixed(float x, int32_t place, uint32_t witdh)
{
	fprintf(stdout, "%*.*f", (int)witdh, (int)place, x);
}

void osic_WrHex(uint32_t n, uint32_t f)
{
	fprintf(stdout, "%*X", (int)f, (uint32_t)n);
	fflush(stdout);
}

int osic_getptsname(int fd, char *name, int len)
{
#ifndef MACOS
	return ptsname_r(fd, name, len);
#else
	{
		/* MacOS does not have ptsname_r, emulate it with ptsname */
		if (name == NULL)
			return -EINVAL;
		char *n = ptsname(fd);
		if (n == NULL)
			return -ENOTTY;
		if (len < strlen(n))
			return -ERANGE;
		strncpy(name, n, len);

		return 0;
	}
#endif
}

int osic_grantpts(int fd)
{
	return grantpt(fd);
}

int osic_unlockpts(int fd)
{
	return unlockpt(fd);
}

int32_t osic_OpenAppendLong(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_WRONLY | O_APPEND | O_LARGEFILE);
}

int32_t osic_OpenAppend(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_WRONLY | O_APPEND);
}

int32_t osic_OpenWrite(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return creat(fn, 0664);
}

int32_t osic_OpenReadLong(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDONLY | O_LARGEFILE);
}

int32_t osic_OpenRead(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDONLY);
}

int32_t osic_OpenRW(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDWR);
}

int32_t osic_OpenNONBLOCK(char fn[], uint32_t fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) > fn_len)
		return -1;
	return open(fn, O_RDWR | O_NONBLOCK);
}

char osic_FdValid(int32_t fd)
{
	return (fd >= 0);
}

void osic_Close(int32_t fd)
{
	close(fd);
}

void osic_CloseSock(int32_t fd)
{
	close(fd);
}

int32_t osic_RdBin(int32_t fd,
		char buf[], uint32_t buf_len,
		uint32_t size)
{
	if (size > (buf_len-1) + 1)
		size = (buf_len-1)+1;
	return read(fd, (char *)buf, size);
}

void osic_WrBin(int32_t fd, char buf[],
		uint32_t buf_len,
		uint32_t size)
{
	int rc;
	if (size > (buf_len-1)+1)
		size = (buf_len-1)+1;
	rc = write(fd, (char *)buf, size);
	(void)rc;
}

void osic_Rename(char fname[], uint32_t fname_len,
		 char newname[], uint32_t newname_len)
{
	if (FNLENCHECK && strnlen(fname, fname_len) >= fname_len)
		return;
	if (FNLENCHECK && strnlen(newname, newname_len) >= newname_len)
		return;
	rename(fname, newname);
}

int osic_Size(int fd)
{
	struct stat st = { };

	fstat(fd, &st);
	return st.st_size;
}

void osic_Seek(int32_t fd, uint32_t pos)
{
	lseek(fd, (int32_t)pos, SEEK_SET);
}

void osic_Seekcur(int32_t fd, int32_t rel)
{
#ifndef MACOS
	if (lseek64(fd, rel, (uint32_t)SEEK_CUR) < 0)
		lseek(fd, 0, SEEK_SET);
#else
	/* MacOS has no lseek64 */
	if (lseek(fd, rel, (uint32_t)SEEK_CUR) < 0)
		lseek(fd, 0, SEEK_SET);
#endif
}

void osic_Remove(char fname[], uint32_t fname_len, char *done)
{
	int rc;

	rc = remove(fname);
	if (rc == 0)
		*done = 1;
	else
		*done = 0;
}

char osic_Exists(char fname[], uint32_t fname_len)
{
	if (access(fname, F_OK) != -1)
		return 1;
	return 0;
}

int osic_symblink(char *existing, char *newname)
{
	return symlink(existing, newname);
}

char osic_mkdir(char path[], uint32_t fname_len, uint32_t perm)
{
        if (mkdir(path, perm) != -1);
                return 1;
        return 0;
}

void osic_NextArg(char s[], uint32_t s_len)
{
	if (argc_delivered >= osic_argc-1) {
		s[0] = 0;
		return;
	}
	strncpy(s, osic_argv[argc_delivered+1], s_len);
	argc_delivered++;
}

void osic_Fill(char *buf, uint32_t len, char byte)
{
	memset(buf, byte, len);
}

double osic_Random(void)
{
	return (rand() / (double)RAND_MAX);
}

void X2C_BEGIN(int *argc, char *argv[],
	       int gc_auto, int32_t gc_threshold, int32_t heap_limit)
{
	if (osic_argc != 0)
		return;
	osic_argc = *argc;
	osic_argv = argv;
}

void X2C_PCOPY(void **ppcpy, size_t size)
{
	void *pnew;

	pnew = malloc(size);
	if (pnew == NULL)
		return;
	memcpy(pnew, *ppcpy, size);
	*ppcpy = pnew;
}

int32_t X2C_TRUNCI(double x, int32_t min0, int32_t max0)
{
	int32_t i;

	if (x < (double)min0 || x > (double)max0)
		assert(0);
	i = (int32_t)x;
	if (x > 0.0) {
		if ((double)i > x)
			--i;
	} else if ((double)i < x) {
		++i;
	}
	return i;
}

uint32_t X2C_TRUNCC(double x, uint32_t min0, uint32_t max0)
{
	uint32_t i;

	if (x < (double)min0)
		i = (uint32_t)min0;
	if (x > (double)max0)
		i = (uint32_t)max0;

	i = (uint32_t)x;
	if ((double)i > x)
		--i;
	return i;
}

float osic_cos(float x)
{
	return cos(x);
}

float osic_ln(float x)
{
	if (x <= 0.0)
		assert(0);
	return log(x);
}

float osic_sqrt(float x)
{
	if (x < 0.0)
		assert(0);
	return sqrt(x);
}

float osic_sin(float x)
{
	return sin(x);
}
float osic_arctan(float x)
{
	return atan(x);
}

float osic_tan(float x)
{
	return tan(x);
}

float osic_exp(float x)
{
	return exp(x);
}

float osic_power(float base, float exponent)
{
	if (base <= 0.0)
		assert(0);
	return pow(base, exponent);
}

float osic_arccos(float x)
{
	return acos(x);
}

float osic_floor(float x)
{
	return floorf(x);
}

void osic_alloc(X2C_ADDRESS *p, uint32_t size)
{
	void *pmem;

	pmem = malloc(size);
	if (pmem == NULL)
		return;
	*p = pmem;
}

void osic_free(X2C_ADDRESS *a, uint32_t size)
{
	assert(*a);
	free(*a);
}

uint32_t osic_time(void)
{
	return time(NULL);
}

uint32_t X2C_LSH(uint32_t a, int32_t length, int32_t n)
{
	uint32_t m;

	m = 0;
	m = (length == 32) ? 0xFFFFFFFFl : (1 << length) - 1;
	if (n > 0) {
		if (n >= (int32_t)length)
			return 0;
		return (a << n) & m;
	}

	if (n <= (int32_t)-length)
		return 0;
	return (a >> -n) & m;
}

char X2C_IN(uint32_t i, uint32_t bits, uint32_t set)
{
	if (i < bits)
		return (((1 << (int)i) & set) != 0);
	return 0;
}

char X2C_INL(uint32_t i, uint32_t bits, uint32_t *set)
{
	if (i < bits)
		return (set[(int)i / X2C_LSET_SIZE] & (1 << ((int)i % X2C_LSET_SIZE))) != 0;
	return 0;
}

void X2C_INCL(uint32_t *set, uint32_t i, uint32_t bits)
{
	if (i >= bits)
		assert(0);
	set[(int)i / X2C_LSET_SIZE] |= 1L << ((int)i % X2C_LSET_SIZE);
}

void X2C_EXCL(uint32_t *set, uint32_t i, uint32_t bits)
{
	if (i >= bits)
		assert(0);
	set[(int)i / X2C_LSET_SIZE] &= ~(1L << ((int)i % X2C_LSET_SIZE));
}

uint32_t *X2C_COMPLEMENT(uint32_t *res, uint32_t *a, uint32_t length)
{
	uint32_t *pres = res;
	while (length--)
		*res++ = ~(*a++);

	return pres;
}

uint32_t X2C_SET(uint32_t a, uint32_t b, uint16_t bits)
{
	if ((a > b || a >= (uint32_t)bits) || b >= (uint32_t)bits)
		return 0;

	return ((uint32_t)((2<<(int)b) - (1<<(int)a)));
}

struct xrMM_Dynarr {
	char *a;
	size_t n[15];
};

size_t xrMM_DynarrDescSize(size_t i)
{
	return (sizeof(char *) + i * sizeof(size_t) * 2);
}

void X2C_InitDesc(struct xrMM_Dynarr *d, size_t *size, size_t lens[], size_t dims)
{
	uint32_t i;
	uint32_t tmp;

	d->n[(dims - 1) * 2] = lens[0];
	tmp = dims - 1;
	i = 1;
	if (i <= tmp) {
		for (;; i++) {
			*size =  *size * lens[dims-i];
			d->n[i * 2 - 1] = *size;
			d->n[(dims - i) * 2 - 2] = lens[i];
			if (i == tmp)
				break;
		}
	}
	*size =  *size*lens[0];
}

void X2C_DYNALLOCATE(char **a, size_t size, size_t lens[], size_t dims)
{
	struct xrMM_Dynarr *desc;

	*a = 0;
	desc = malloc(xrMM_DynarrDescSize(dims));
	if (desc) {
		X2C_InitDesc(desc, &size, lens, dims);
		desc->a = malloc(size);
		if (desc->a == 0) {
			free(desc);
		} else {
			DBG("%s: %p (%d bytes) with child %p (%d bytes)\n",
			    __func__, desc, xrMM_DynarrDescSize(dims),
			    desc->a, size);
			*a = (char *)desc;
		}
	}
}

void X2C_DYNDEALLOCATE(char **a)
{
	struct xrMM_Dynarr *d;

	if (*a) {
		d = (struct xrMM_Dynarr *)*a;
		DBG("%s: have %p with child %p\n", __func__, d, d->a);
		free(d->a);
		free(d);
	}
}

void *osic_chkptr(void *p)
{
	assert(p);
	return p;
}
