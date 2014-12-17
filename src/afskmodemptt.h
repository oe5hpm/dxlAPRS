#ifndef afskmodemptt_H_
#define afskmodemptt_H_
#include "X2C.h"

extern X2C_ADDRESS pttinit(X2C_ADDRESS, long);

extern void pttDestroy(X2C_ADDRESS);

extern void pttSetclaim(X2C_ADDRESS, long);

extern void pttHelp(X2C_ADDRESS, unsigned long);

extern void ptt(X2C_ADDRESS, long);


#endif /* afskmodemptt_H_ */
