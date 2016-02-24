/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrtsOS_H_
#define xrtsOS_H_
#include "X2C.h"
#include "xmRTS.h"

typedef void *X2C_OSFILE;

#define X2C_beg 0

#define X2C_cur 1

#define X2C_end 2

extern int X2C_FileOpenRead(X2C_OSFILE *, X2C_CHAR []);

extern int X2C_FileClose(X2C_OSFILE);

extern int X2C_FileSeek(X2C_OSFILE, X2C_WORD, int);

extern int X2C_FileOpenWrite(X2C_OSFILE *, X2C_CHAR []);

extern int X2C_FileOpenRW(X2C_OSFILE *, X2C_CHAR []);

extern int X2C_FileRead(X2C_OSFILE, X2C_ADDRESS, X2C_CARD32 *);

extern int X2C_FileWrite(X2C_OSFILE, X2C_ADDRESS, X2C_CARD32 *);

extern X2C_ADDRESS X2C_AllocMem(X2C_CARD32);

extern void X2C_InitMem(void);

extern X2C_ADDRESS X2C_malloc(X2C_CARD32);

extern void X2C_free(X2C_ADDRESS, X2C_CARD32);

extern void X2C_InitHeap(X2C_CARD32, X2C_BOOLEAN);

extern void X2C_ZEROMEM(X2C_ADDRESS, X2C_CARD32);

extern void X2C_StdOut(X2C_CHAR [], X2C_CARD32);

extern void X2C_StdOutS(X2C_CHAR [], X2C_CARD32);

extern void X2C_StdOutD(X2C_CARD32, X2C_CARD32);

extern void X2C_StdOutH(X2C_CARD32, X2C_CARD32);

extern void X2C_StdOutN(void);

extern void X2C_StdOutFlush(void);

extern void X2C_DecToStr(X2C_CHAR [], X2C_CARD32 *, X2C_CARD32);

extern void X2C_HexToStr(X2C_CHAR [], X2C_CARD32 *, X2C_CARD32);

extern void X2C_EnableIpts(void);

extern void X2C_DisableIpts(void);

extern void X2C_SaveIptHandler(X2C_CARD32);

extern void X2C_RestoreIptHandler(X2C_CARD32);

extern X2C_BOOLEAN X2C_SetIptHandler(X2C_CARD32);

extern void X2C_INT_HANDLER(X2C_CARD32);

extern void X2C_iniexit(void);

extern void X2C_atexit(X2C_EXIT_PROC);

extern void X2C_doexit(X2C_INT32);

extern X2C_ADDRESS X2C_StackTop(void);


#endif /* xrtsOS_H_ */
