/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrO2MM_H_
#define xrO2MM_H_
#include "X2C.h"
#include "xmRTS.h"

extern void X2C_NEW(X2C_TD, X2C_ADDRESS *, size_t, X2C_BOOLEAN);

extern void X2C_NEW_OPEN(X2C_TD, X2C_ADDRESS *, size_t, size_t [], size_t,
                X2C_BOOLEAN);

extern void X2C_DISPOSE(X2C_ADDRESS *);

extern void X2C_DESTRUCTOR(X2C_ADDRESS, X2C_DPROC);

extern void X2C_MODULEXE(X2C_MD, X2C_ADDRESS);

extern void X2C_MODULEDLL(X2C_MD *, X2C_MD, X2C_ADDRESS);

extern void X2C_DISABLE_COMPONENT(X2C_MD);

extern X2C_CARD32 MutatorPeriod;

extern X2C_CARD32 GCPeriod;

extern X2C_CARD32 TotalGCTime;

extern void X2C_COLLECT(void);


#endif /* xrO2MM_H_ */
