/*
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 *
 * this files substitutes the absolute minimum of the original XDS lib
 * for our project.
 */
#ifndef X2C_H_
#define X2C_H_

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

#define X2C_INT32			long
#define X2C_CARD32			unsigned long
#define X2C_REAL			float
#define X2C_LONGREAL			double
#define X2C_CHAR			char
#define X2C_BOOLEAN			char

typedef char				*X2C_ADDRESS;
typedef char				*X2C_pCHAR;
typedef X2C_CARD32			* LSET;

typedef X2C_CARD32			X2C_LSET_BASE;
typedef X2C_LSET_BASE			* X2C_LSET;
#define X2C_LSET_SIZE			(sizeof(X2C_LSET_BASE) * 8)

#define X2C_max_longint			0x7FFFFFFFL
#define X2C_min_longint			(-0x7FFFFFFFL-1)
#define X2C_max_longcard		0xFFFFFFFFUL
#define X2C_max_real			((X2C_REAL)FLT_MAX)
#define X2C_min_real			(-((X2C_REAL)FLT_MAX))
#define X2C_max_longreal		((X2C_LONGREAL)DBL_MAX)
#define X2C_min_longreal		(-((X2C_LONGREAL)DBL_MAX))
#define X2C_max_longdouble		((X2C_LONGDOUBLE)LDBL_MAX)
#define X2C_min_longdouble		(-((X2C_LONGDOUBLE)LDBL_MAX))

#define X2C_MIN(type, min_value)	((type)(min_value))
#define X2C_MAX(type, max_value)	((type)(max_value))
#define X2C_INRANGE(v, min, max)	(((v) >= (min)) && ((v) <= (max)))

#define X2C_CASE_TRAP			0

#define X2C_ABORT(void)			exit(-1)
#define X2C_EXIT(void)			exit(0)
#define X2C_HALT(x)			(exit(x))

#define X2C_CAP(x)			toupper(x)

#define X2C_DIVL(a, b)			((a)/(b))
#define X2C_PFREE(p)			(free(p))
#define X2C_COPY(src, src_len, dst, dst_len) \
	memcpy(dst, src, src_len <= dst_len ? src_len : dst_len)

#define X2C_MOVE(source, dest, size)	memcpy(dest, source, size)
#define X2C_STRCMP(a, alen, b, blen)	strncmp(a, b, alen < blen ? alen : blen)

#define X2C_CHKNIL(T, p)		((T)(X2C_CHKNIL_F((void *)(p))))
#define X2C_CHKPROC(T, p)		((T)(X2C_CHKPROC_F((void *)(p))))

#define X2C_CAST(val, fr, to, to_ref) \
	((sizeof(fr) < sizeof(to)) ? (assert(0), (to_ref)0) : (to_ref)(val))

#define X2C_MAIN_DEFINITION
#define X2C_STACK_LIMIT(x)

#endif
