/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef x2cLib_H_
#define x2cLib_H_
#include "X2C.h"
#include "xPOSIX.h"

extern X2C_BOOLEAN X2C_Exists(X2C_CHAR []);

extern void X2C_ModifyTime(X2C_CHAR [], X2C_CARD32 *, X2C_BOOLEAN *);

extern X2C_CARD32 X2C_Time(void);

extern X2C_CARD32 X2C_Clock(void);

extern void X2C_SetFileModes(X2C_CHAR [], X2C_CHAR);

extern X2C_ADDRESS X2C_FDup(FILE *, X2C_CHAR []);

extern int X2C_FDup2(FILE *, X2C_ADDRESS *);

extern int X2C_ChSize(FILE *, X2C_INT32);

extern int X2C_GetFileType(FILE *);


#endif /* x2cLib_H_ */
