/* Copyright (C) 2000 Excelsior. */

#include "X2C.h"

#if (defined(__WATCOMC__) && defined(__NT__)) || (defined(_MSC_VER) && defined(_WIN32))

#include <windows.h>
void xtsIO_SetConMode(X2C_BOOLEAN set, X2C_CARD32* tmp) {
    HANDLE hCon = GetStdHandle(STD_INPUT_HANDLE);
    DWORD conOldState;
    if (set) {
        GetConsoleMode(hCon, &conOldState);
        SetConsoleMode(hCon, conOldState | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        *tmp = conOldState;
    }
    else
        SetConsoleMode(hCon, *tmp);
}

#else

/* This function is only used under Windows */
void xtsIO_SetConMode(X2C_BOOLEAN set, X2C_CARD32* tmp) {
    tmp = 0;
}

#endif

void xtsIO_BEGIN(void) { /* !!! */
}
