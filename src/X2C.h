/*
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 *
 * this files substitutes the absolute minimum of the original XDS lib
 * for our project.
 */
#ifndef X2C_H_
#define X2C_H_

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <float.h>
#include <assert.h>

#define X2C_ADDRESS			char *
#define X2C_pCHAR			char *
#define X2C_LSET_SIZE			(sizeof(uint32_t) * 8)

#define X2C_max_longint			0x7FFFFFFFL
#define X2C_min_longint			(-0x7FFFFFFFL-1)
#define X2C_max_longcard		0xFFFFFFFFUL
#define X2C_max_real			((float)FLT_MAX)
#define X2C_min_real			(-((float)FLT_MAX))
#define X2C_max_longreal		((double)DBL_MAX)
#define X2C_min_longreal		(-((double)DBL_MAX))
#define X2C_max_longdouble		((X2C_LONGDOUBLE)LDBL_MAX)
#define X2C_min_longdouble		(-((X2C_LONGDOUBLE)LDBL_MAX))

#define X2C_ASSERT(x)			assert(x)
#define X2C_TRAP(x)			assert(0)
#define X2C_CASE_TRAP			0

#define X2C_ABORT(void)			exit(-1)
#define X2C_EXIT(void)			exit(0)
#define X2C_HALT(x)			(exit(x))

#define X2C_CAP(x)			toupper(x)

#define X2C_DIVL(a, b)			((a)/(b))
#define X2C_DIVR(a, b)			((b) != 0.0f ? (a)/(b) : (a))
#define X2C_EXPRI(a, b)			(pow(a,b))
#define X2C_PFREE(p)			(free(p))
#define X2C_COPY(src, src_len, dst, dst_len) \
	memcpy(dst, src, src_len <= dst_len ? src_len : dst_len)

#define X2C_MOVE(source, dest, size)	memcpy(dest, source, size)
#define X2C_STRCMP(a, alen, b, blen)	strncmp(a, b, alen < blen ? alen : blen)

#define X2C_CHKNIL(T, p)		((T)(osic_chkptr((void *)(p))))
#define X2C_CHKPROC(T, p)		((T)(osic_chkptr((void *)(p))))

#define X2C_CAST(val, fr, to, to_ref) \
	((sizeof(fr) < sizeof(to)) ? (assert(0), (to_ref)0) : (to_ref)(val))

#define X2C_MAIN_DEFINITION
#define X2C_STACK_LIMIT(x)

#endif
