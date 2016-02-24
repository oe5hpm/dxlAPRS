#include "X2C.h"

#ifndef X2C_KRC
void X2C_PROCLASS Coroutine_start (void)
#else
void X2C_PROCLASS Coroutine_start ()
#endif
{
        X2C_Coroutine current;

        current=X2C_GetCurrent();
        current->stk_end=0;
        current->proc();
        X2C_TRAP_F(X2C_coException);
}
