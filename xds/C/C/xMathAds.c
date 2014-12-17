#include <math.h>
#include "xMath.h"

int X2C_xMathAdds_INIT()
{
        /*
         this function is intentionally empty
         It it defined here just to avoid compilation errors
         (sample is Watcom compiler that does not like to
         compile empty files
        */
        return 0;
}

#if (defined(X2C_no_asinh))

extern float X2C_arcsinh(float x)
{
        return (float)(log(x+sqrt(x*x+1)));
}

extern double X2C_arcsinhl(double x)
{
        return log(x+sqrt(x*x+1));
}

#endif

#if (defined(X2C_no_acosh))

extern float X2C_arccosh(float x)
{
        return (float)(log(x+sqrt(x*x-1)));
}

extern double X2C_arccoshl(double x)
{
        return log(x+sqrt(x*x-1));
}

#endif


#if (defined(X2C_no_atanh))

extern float X2C_arctanh(float x)
{
        return (float)(0.5*log((1+x)/(1-x)));
}

extern double X2C_arctanhl(double x)
{
        return 0.5*log((1+x)/(1-x));
}

#endif
