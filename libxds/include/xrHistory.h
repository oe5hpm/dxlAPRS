/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xrHistory_H_
#define xrHistory_H_
#include "X2C.h"
#include "xmRTS.h"

extern void X2C_PROC_INP_F(X2C_CHAR [], X2C_INT32);

extern void X2C_PROC_PRF_F(X2C_CHAR [], X2C_INT32, struct X2C_Profile_STR *);

extern void X2C_PROC_OUT_F(void);

extern void X2C_Profiler_clock(void);

extern void X2C_scanStackHistory(X2C_ADDRESS, X2C_ADDRESS, X2C_BOOLEAN);

extern void X2C_HIS_SAVE(X2C_INT16 *);

extern void X2C_HIS_RESTORE(X2C_INT16);

extern void X2C_show_history(void);

extern void X2C_show_profile(void);


#endif /* xrHistory_H_ */
