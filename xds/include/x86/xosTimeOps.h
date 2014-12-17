/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosTimeOps_H_
#define xosTimeOps_H_
#include "X2C.h"
#include "xlibOS.h"

extern X2C_INT32 X2C_TimeCompare(struct X2C_TimeStruct,
                struct X2C_TimeStruct);

extern X2C_CARD32 X2C_TimeDayInt(struct X2C_TimeStruct,
                struct X2C_TimeStruct);

extern X2C_CARD32 X2C_TimeSecInt(struct X2C_TimeStruct,
                struct X2C_TimeStruct);

extern void X2C_TimeDayAdd(struct X2C_TimeStruct, X2C_CARD32,
                struct X2C_TimeStruct *);

extern void X2C_TimeSecAdd(struct X2C_TimeStruct, X2C_CARD32,
                struct X2C_TimeStruct *);

extern X2C_CARD32 X2C_TimeDayNum(X2C_CARD32, X2C_CARD32, X2C_CARD32);


#endif /* xosTimeOps_H_ */
