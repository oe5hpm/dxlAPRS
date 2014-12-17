// missed natives

#include <math.h>
#include "X2C.h"


//PROCEDURE [CallConv] / X2C_frexp (x: LONGREAL; VAR i: SYSTEM.int): LONGREAL;
X2C_LONGREAL X2C_frexp(X2C_LONGREAL x, int *i) {
    return frexp(x, i);
}


//PROCEDURE [CallConv] / X2C_arctan2l(y,x: LONGREAL): LONGREAL;
X2C_LONGREAL X2C_arctan2l(X2C_LONGREAL y, X2C_LONGREAL x) {
    return atan2(y, x);
}

//PROCEDURE [CallConv] / X2C_reml(x,y: LONGREAL): LONGREAL;
X2C_LONGREAL X2C_reml(X2C_LONGREAL x, X2C_LONGREAL y) {
    return fmod(x, y);
}

