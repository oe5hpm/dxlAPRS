/* Copyright (C) 2000 Excelsior. */

#include "X2C.h"


#if (defined(__WATCOMC__) && defined(__NT__)) || (defined(_MSC_VER) && defined(_WIN32))

#include <windows.h>
void xtsLib_Delay(X2C_CARD32 t) {
  Sleep(t);
}
void xtsLib_Speaker(X2C_CARD32 FreqHz, X2C_CARD32 TimeMs) {
  Beep( FreqHz, TimeMs);
}


#elif defined(_unix)

#include <unistd.h>
void xtsLib_Delay(X2C_CARD32 t) {
    unsigned long sec = t/1000, milisec = t%1000;
    if(sec!=0) {
        sleep(sec);
    }
    usleep(milisec*1000); //time in microsec
}
void xtsLib_Speaker(X2C_CARD32 FreqHz, X2C_CARD32 TimeMs) {
}


#else

void xtsLib_Delay(X2C_CARD32 t) {
}

void xtsLib_Speaker(X2C_CARD32 FreqHz, X2C_CARD32 TimeMs) {
}
#endif

void xtsLib_Environment(X2C_CARD32 N, X2C_CHAR result[], X2C_CARD32 resultLen) {
#if defined(_linux)
    char *p = __environ[N];
#else
    char *p = environ[N];
#endif
    while(*p!=0 && *p!='=') {
        p++;
    }
    if(*p==0) {
        result[0] = 0;
        return;
    }
    strncpy(result, ++p, resultLen);
}


void xtsLib_BEGIN(void) { /* !!! */
}
