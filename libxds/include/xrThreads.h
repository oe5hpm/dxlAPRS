/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrThreads_H_
#define xrThreads_H_
#include "X2C.h"
#include "xmRTS.h"

extern void X2C_SetCurrent(X2C_Coroutine);

extern X2C_Coroutine X2C_GetCurrent(void);

extern X2C_INT32 X2C_InitThreads(void);

extern void X2C_PrepareToGC(void);

extern void X2C_FreeAfterGC(void);


#endif /* xrThreads_H_ */
