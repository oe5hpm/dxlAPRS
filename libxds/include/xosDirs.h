/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosDirs_H_
#define xosDirs_H_
#include "X2C.h"
#include "xlibOS.h"
#include "xmRTS.h"

extern void X2C_DirOpen(struct X2C_Dir *, X2C_pCHAR);

extern void X2C_DirClose(struct X2C_Dir *);

extern void X2C_DirNext(struct X2C_Dir *);

extern void X2C_DirGetName(struct X2C_Dir *, X2C_pCHAR, X2C_CARD32);

extern X2C_CARD32 X2C_GetCDNameLength(void);

extern void X2C_GetCDName(X2C_pCHAR, X2C_CARD32);

extern X2C_BOOLEAN X2C_SetCD(X2C_pCHAR);

extern X2C_BOOLEAN X2C_CreateDirectory(X2C_pCHAR);

extern X2C_BOOLEAN X2C_RemoveDirectory(X2C_pCHAR);


#endif /* xosDirs_H_ */
