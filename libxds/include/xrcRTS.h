/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrcRTS_H_
#define xrcRTS_H_
#include "X2C.h"
#include "xmRTS.h"

extern X2C_BOOLEAN X2C_IN(X2C_CARD32, X2C_CARD16, X2C_CARD32);

extern X2C_CARD32 X2C_SET(X2C_CARD32, X2C_CARD32, X2C_CARD16);

extern void X2C_PCOPY(X2C_pVOID *, size_t);

extern void X2C_PFREE(X2C_pVOID);

extern X2C_PROTECTION X2C_PROT(void);


#endif /* xrcRTS_H_ */
