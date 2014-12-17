/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrExceptions_H_
#define xrExceptions_H_
#include "X2C.h"
#include "xmRTS.h"

extern void X2C_doRaise(X2C_XSource);

extern void X2C_XInitHandler(X2C_XHandler);

extern void X2C_XRETRY(void);

extern void X2C_XREMOVE(void);

extern void X2C_XOFF(void);

extern void X2C_XON(void);

extern void X2C_TRAP_FC(X2C_INT32, X2C_pCHAR, X2C_CARD32);

extern void X2C_TRAP_F(X2C_INT32);

extern void X2C_TRAP_G(X2C_INT32);

extern void X2C_ASSERT_F(X2C_CARD32);

extern void X2C_ASSERT_FC(X2C_CARD32, X2C_pCHAR, X2C_CARD32);

extern void X2C_init_exceptions(void);


#endif /* xrExceptions_H_ */
