/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrsetjmp_H_
#define xrsetjmp_H_
#include "X2C.h"

struct xrsetjmp__D0;


struct xrsetjmp__D0 {
   X2C_CARD32 ebx;
   X2C_CARD32 ecx;
   X2C_CARD32 edx;
   X2C_CARD32 esi;
   X2C_CARD32 edi;
   X2C_CARD32 ebp;
   X2C_CARD32 eip;
   X2C_CARD32 esp;
   X2C_CARD16 es;
   X2C_CARD16 ds;
   X2C_CARD16 cs;
   X2C_CARD16 fs;
   X2C_CARD16 gs;
   X2C_CARD16 ss;
};

typedef struct xrsetjmp__D0 X2C_jmp_buf[1];

extern int X2C_setjmp(X2C_jmp_buf);

extern void X2C_longjmp(X2C_jmp_buf, int);


#endif /* xrsetjmp_H_ */
