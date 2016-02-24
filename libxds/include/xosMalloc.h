/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosMalloc_H_
#define xosMalloc_H_
#include "X2C.h"

extern void X2C_InitHeap(X2C_CARD32, X2C_BOOLEAN);

extern X2C_ADDRESS X2C_malloc(X2C_CARD32);

extern void X2C_free(X2C_ADDRESS, X2C_CARD32);

extern X2C_ADDRESS X2C_gmalloc(X2C_CARD32);

extern void X2C_gfree(X2C_ADDRESS);

extern void X2C_DestroyHeap(void);

extern void xosMalloc_init(void);


#endif /* xosMalloc_H_ */
