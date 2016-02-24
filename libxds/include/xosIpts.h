/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosIpts_H_
#define xosIpts_H_
#include "X2C.h"

extern void X2C_EnableIpts(void);

extern void X2C_DisableIpts(void);

extern void X2C_SaveIptHandler(X2C_CARD32);

extern void X2C_RestoreIptHandler(X2C_CARD32);

extern X2C_BOOLEAN X2C_SetIptHandler(X2C_CARD32);


#endif /* xosIpts_H_ */
