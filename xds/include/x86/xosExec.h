/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosExec_H_
#define xosExec_H_
#include "X2C.h"
#include "xmRTS.h"

extern X2C_INT32 X2C_Execute(X2C_pCHAR, X2C_pCHAR, int, X2C_CARD32 *);

extern X2C_INT32 X2C_ExecuteNoWindow(X2C_pCHAR, X2C_pCHAR, int,
                X2C_CARD32 *);

extern X2C_INT32 X2C_Command(X2C_pCHAR, X2C_CARD32 *);


#endif /* xosExec_H_ */
