/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosFileIO_H_
#define xosFileIO_H_
#include "X2C.h"
#include "xrtsOS.h"

extern int X2C_FileOpenRead(X2C_OSFILE *, X2C_CHAR []);

extern int X2C_FileClose(X2C_OSFILE);

extern int X2C_FileSeek(X2C_OSFILE, X2C_WORD, int);

extern int X2C_FileOpenWrite(X2C_OSFILE *, X2C_CHAR []);

extern int X2C_FileOpenRW(X2C_OSFILE *, X2C_CHAR []);

extern int X2C_FileRead(X2C_OSFILE, X2C_ADDRESS, X2C_CARD32 *);

extern int X2C_FileWrite(X2C_OSFILE, X2C_ADDRESS, X2C_CARD32 *);

extern void X2C_StdOut(X2C_CHAR [], X2C_CARD32);

extern void X2C_StdOutFlush(void);


#endif /* xosFileIO_H_ */
