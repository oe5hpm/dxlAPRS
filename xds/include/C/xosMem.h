/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosMem_H_
#define xosMem_H_
#include "X2C.h"

extern X2C_ADDRESS X2C_AllocMem(X2C_CARD32);

extern void X2C_InitMem(void);

extern X2C_CARD32 X2C_GetAvailablePhysicalMemory(void);


#endif /* xosMem_H_ */
