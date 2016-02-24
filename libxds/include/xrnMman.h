/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrnMman_H_
#define xrnMman_H_
#include "X2C.h"

#define PROT_NONE 0x0 

#define PROT_READ 0x1 

#define PROT_WRITE 0x2 

#define PROT_EXEC 0x4 

#define MAP_SHARED 0x1 

#define MAP_PRIVATE 0x2 

#define MAP_TYPE 0xF 

#define MAP_FIXED 0x10 

#define MAP_FILE 0

#define MAP_ANONYMOUS 0x20 

#define MAP_ANON 32

#define MAP_GROWSDOWN 0x100 

#define MAP_DENYWRITE 0x800 

#define MAP_EXECUTABLE 0x1000 

#define MAP_LOCKED 0x2000 

#define MAP_NORESERVE 0x4000 

typedef X2C_INT32 INT32;

extern X2C_ADDRESS mmap(X2C_ADDRESS, X2C_CARD32, X2C_CARD32, X2C_CARD32,
                X2C_INT32, X2C_CARD32);

extern X2C_INT32 munmap(X2C_ADDRESS, X2C_CARD32);

extern X2C_INT32 mprotect(X2C_ADDRESS, X2C_CARD32, X2C_CARD32);


#endif /* xrnMman_H_ */
