#include <time.h>
#include "X2C.h"

#ifdef __IBMC__
#define timezone _timezone
#endif

typedef struct X2C_TimeStruct {
        X2C_CARD32 year;
        X2C_CARD32 month;
        X2C_CARD32 day;
        X2C_CARD32 hour;
        X2C_CARD32 min;
        X2C_CARD32 sec;
        X2C_CARD32 fracs;
        X2C_INT32 zone;
        X2C_BOOLEAN stf;
} X2C_TimeStruct;

#if defined(X2C_OS9)
extern X2C_BOOLEAN X2C_CanGetTime(void)
#else
extern X2C_BOOLEAN X2C_CanGetTime()
#endif
{
        return (X2C_BOOLEAN)1;
}

#if defined(X2C_OS9)
extern X2C_BOOLEAN X2C_CanSetTime(void)
#else
extern X2C_BOOLEAN X2C_CanSetTime()
#endif
{
        return (X2C_BOOLEAN)0;
}

#ifndef X2C_KRC
extern void X2C_UnpackTime(time_t t, struct X2C_TimeStruct *res)
#else
extern void X2C_UnpackTime(t, res) 
	time_t t;
	struct X2C_TimeStruct *res;
#endif
{
        struct tm *p = localtime(&t);

        if (p == 0) {
                res->year = 1970;
                res->month = 1;
                res->day = 1;
                res->hour = 0;
                res->min = 0;
                res->sec = 0;
                res->fracs = 0;
                res->zone = 0;
                res->stf = (X2C_BOOLEAN)0;
                return;
        }

        res->year = p->tm_year;
        if (res->year<1900) {
                if (res->year<70) res->year += 2000;
                else res->year += 1900;
        }
        res->month = p->tm_mon+1;
        res->day = p->tm_mday;
        res->hour = p->tm_hour;
        res->min = p->tm_min;
        res->sec = p->tm_sec;

/* folowing "ifndef" is somewhat silly.
   It made to satisfy SunOS4.1.
   The leading idea is that CLOCKS_PER_SEC should be defined together
   with clock() func.
 */
#ifndef CLOCKS_PER_SEC
        res->fracs = 0;
#else
        res->fracs = clock() % CLOCKS_PER_SEC;
#endif

#if !defined(X2C_OS9)
        tzset();
#endif

#if defined(__FreeBSD__) || \
    (defined(__ultrix) && !defined(SYSTEM_FIVE)) || \
    defined(DJGPP)
        res->zone = p->tm_gmtoff / 60;
        res->stf  = (X2C_BOOLEAN)(p->tm_isdst>0);
#elif !defined(__SC__) && !defined(X2C_OS9) && defined(CLOCKS_PER_SEC)
        res->zone = timezone / 60;
        res->stf = (X2C_BOOLEAN)daylight;
#endif
}

#ifndef X2C_KRC
extern void X2C_GetTime(struct X2C_TimeStruct *res)
#else
extern void X2C_GetTime(res) struct X2C_TimeStruct *res;
#endif
{
        time_t t;
        time(&t);
	X2C_UnpackTime(t, res);
}

#ifndef X2C_KRC
extern void X2C_SetTime(struct X2C_TimeStruct *res)
#else
extern void X2C_SetTime(res) struct X2C_TimeStruct *res;
#endif
{
}
#if defined(X2C_OS9)
extern X2C_CARD32 X2C_FracsPerSec(void)
#else
extern X2C_CARD32 X2C_FracsPerSec()
#endif
{
#ifndef CLOCKS_PER_SEC
        return (100);
#else
        return (CLOCKS_PER_SEC);
#endif
}
