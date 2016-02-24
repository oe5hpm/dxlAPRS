/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrM2MM_H_
#define xrM2MM_H_
#include "X2C.h"

extern void X2C_ALLOCATE(X2C_ADDRESS *, size_t);

extern void X2C_DEALLOCATE(X2C_ADDRESS *);

extern void X2C_DYNALLOCATE(X2C_ADDRESS *, size_t, size_t [], size_t);

extern void X2C_DYNDEALLOCATE(X2C_ADDRESS *);

extern void X2C_DYNCALLOCATE(X2C_ADDRESS *, size_t, size_t [], size_t);

extern void X2C_DYNCDEALLOCATE(X2C_ADDRESS *);


#endif /* xrM2MM_H_ */
