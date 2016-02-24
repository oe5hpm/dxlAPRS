/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrInt64_H_
#define xrInt64_H_
#include "X2C.h"

typedef X2C_CARD32 CARD32;

typedef X2C_INT32 INT32;

struct X2C_int64;


struct X2C_int64 {
   X2C_CARD32 low;
   X2C_CARD32 high;
};

extern void X2C_INTTO64(struct X2C_int64 *, X2C_INT32);

extern void X2C_CARDTO64(struct X2C_int64 *, X2C_CARD32);

extern X2C_BOOLEAN X2C_IsNeg64(struct X2C_int64);

extern int X2C_CMP64(struct X2C_int64, struct X2C_int64);

extern X2C_BOOLEAN X2C_64TOINT(X2C_INT32 *, struct X2C_int64);

extern X2C_BOOLEAN X2C_64TOCARD(X2C_CARD32 *, struct X2C_int64);

extern X2C_BOOLEAN X2C_UnMinus64(struct X2C_int64 *, struct X2C_int64);

extern X2C_BOOLEAN X2C_ADD64(struct X2C_int64 *, struct X2C_int64,
                struct X2C_int64);

extern void X2C_MUL64(struct X2C_int64 *, X2C_INT32, X2C_CARD32);


#endif /* xrInt64_H_ */
