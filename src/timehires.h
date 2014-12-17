#ifndef timehires_H_
#define timehires_H_
#include "X2C.h"

struct TIMEHR;


struct TIMEHR {
   X2C_CARD32 sec;
   X2C_CARD32 nsec;
};

extern void gettime(struct TIMEHR *);


#endif /* timehires_H_ */
