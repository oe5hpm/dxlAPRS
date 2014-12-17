/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosEnv_H_
#define xosEnv_H_
#include "X2C.h"
#include "xmRTS.h"

extern X2C_CARD32 X2C_EnvStringLength(X2C_pCHAR);

extern void X2C_EnvString(X2C_pCHAR, X2C_pCHAR, X2C_CARD32);

extern X2C_pCHAR X2C_GetProgramName(void);


#endif /* xosEnv_H_ */
