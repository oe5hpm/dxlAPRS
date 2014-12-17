/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosTimeOps.c Feb  3 14:30:28 2012" */
#include "xosTimeOps.h"
#define xosTimeOps_C_
#include "xlibOS.h"

#define y001days 365

#define y004days 1461

#define y100days 36524

#define y400days 146097

static X2C_CARD32 monthAdd[12] = {0UL,3UL,3UL,6UL,8UL,11UL,13UL,16UL,19UL,
                21UL,24UL,26UL};

static X2C_CARD32 monthLast[12] = {31UL,59UL,90UL,120UL,151UL,181UL,212UL,
                243UL,273UL,304UL,334UL,365UL};

#define FirstValidYear 1

static struct X2C_TimeStruct FirstDate = {1UL,1UL,1UL,0UL,0UL,0UL,0UL,0L,0};


static X2C_BOOLEAN is_leap(X2C_CARD32 y)
{
   return (y&3UL)==0UL && y%100UL || y%400UL==0UL;
} /* end is_leap() */

static X2C_CARD32 xosTimeOps_m30days = 0xA50UL;


static X2C_BOOLEAN is_valid_day(X2C_CARD32 y, X2C_CARD32 m, X2C_CARD32 d)
{
   if (d<1UL || d>31UL) return 0;
   if (m<1UL || m>12UL) return 0;
   if (y<1UL) return 0;
   if (X2C_IN(m,32,0xA50UL) && d>30UL) return 0;
   if (m==2UL && d>28UL+(X2C_CARD32)is_leap(y)) return 0;
   return 1;
} /* end is_valid_day() */

static X2C_CARD32 xosTimeOps_m30days0 = 0xA50UL;


static X2C_BOOLEAN is_valid(const struct X2C_TimeStruct d)
{
   if (is_valid_day(d.year, d.month, d.day)) {
      if ((d.hour>23UL || d.min0>59UL) || d.sec>59UL) return 0;
      return 1;
   }
   return 0;
} /* end is_valid() */

static X2C_CARD32 _cnst0[12] = {0UL,3UL,3UL,6UL,8UL,11UL,13UL,16UL,19UL,21UL,
                24UL,26UL};

static X2C_CARD32 day_of_year(X2C_CARD32 y, X2C_CARD32 m, X2C_CARD32 d)
{
   X2C_CARD32 x;
   --m;
   x = d+m*28UL+_cnst0[m];
   if (m>=2UL && is_leap(y)) ++x;
   return x;
} /* end day_of_year() */


static X2C_CARD32 the_day(X2C_CARD32 y, X2C_CARD32 m, X2C_CARD32 d)
{
   X2C_CARD32 year;
   X2C_CARD32 day;
   year = y;
   --y;
   day = (y/400UL)*146097UL;
   y = y%400UL;
   day += (y/100UL)*36524UL;
   y = y%100UL;
   day += (y/4UL)*1461UL;
   day += (y&3UL)*365UL;
   day += day_of_year(year, m, d);
   return day;
} /* end the_day() */


extern X2C_CARD32 X2C_TimeDayNum(X2C_CARD32 y, X2C_CARD32 m, X2C_CARD32 d)
{
   if (is_valid_day(y, m, d)) return the_day(y, m, d);
   return 0UL;
} /* end X2C_TimeDayNum() */


static X2C_CARD32 sub_days(struct X2C_TimeStruct g, struct X2C_TimeStruct l)
{
   X2C_CARD32 c;
   c = ((l.year-1UL)/400UL)*400UL;
   g.year = g.year-c;
   l.year = l.year-c;
   return the_day(g.year, g.month, g.day)-the_day(l.year, l.month, l.day);
} /* end sub_days() */


static X2C_CARD32 day_sec(X2C_CARD32 h, X2C_CARD32 m, X2C_CARD32 s)
{
   return s+m*60UL+h*3600UL;
} /* end day_sec() */


static X2C_CARD32 sub_secs(const struct X2C_TimeStruct g,
                const struct X2C_TimeStruct l)
{
   X2C_CARD32 days;
   days = sub_days(g, l);
   return (days*86400UL+day_sec(g.hour, g.min0, g.sec))-day_sec(l.hour,
                l.min0, l.sec);
} /* end sub_secs() */

static X2C_CARD32 _cnst1[12] = {31UL,59UL,90UL,120UL,151UL,181UL,212UL,243UL,
                273UL,304UL,334UL,365UL};

static void unpack_day(X2C_CARD32 day, X2C_CARD32 * y, X2C_CARD32 * m,
                X2C_CARD32 * d)
{
   X2C_CARD32 i;
   X2C_BOOLEAN leap;
   --day;
   *y = 400UL*(day/146097UL);
   day = day%146097UL;
   i = day/36524UL;
   if (i==4UL) i = 3UL;
   *y += i*100UL;
   day -= i*36524UL;
   i = day/1461UL;
   day -= i*1461UL;
   *y += i*4UL;
   i = day/365UL;
   if (i==4UL) i = 3UL;
   *y += i;
   day -= i*365UL;
   leap = is_leap(*y+1UL);
   ++day;
   *m = day/32UL;
   day -= (X2C_CARD32)(leap && day>31UL);
   while (*m<=11UL && day>_cnst1[*m]) ++*m;
   *d = day;
   if (*m>0UL) *d -= _cnst1[*m-1UL];
   ++*m;
   *d += (X2C_CARD32)(leap && *m==2UL);
} /* end unpack_day() */


static void add_days(X2C_CARD32 * y, X2C_CARD32 * m, X2C_CARD32 * d,
                X2C_CARD32 days)
{
   X2C_CARD32 cy400s;
   cy400s = (*y-1UL)/400UL;
   *y -= cy400s*400UL;
   cy400s += days/146097UL;
   days = days%146097UL;
   unpack_day(days+the_day(*y, *m, *d), y, m, d);
   *y = *y+cy400s*400UL+1UL;
} /* end add_days() */


static void add_secs(X2C_CARD32 * y, X2C_CARD32 * m, X2C_CARD32 * d,
                X2C_CARD32 * h, X2C_CARD32 * mi, X2C_CARD32 * s,
                X2C_CARD32 secs)
{
   X2C_CARD32 days;
   secs += day_sec(*h, *mi, *s);
   days = secs/86400UL;
   secs = secs%86400UL;
   add_days(y, m, d, days);
   *h = secs/3600UL;
   secs = secs%3600UL;
   *mi = secs/60UL;
   *s = secs%60UL;
} /* end add_secs() */


extern X2C_INT32 X2C_TimeCompare(struct X2C_TimeStruct dl,
                struct X2C_TimeStruct dr)
{
   X2C_INT32 r;
   if (!(is_valid(dl) && is_valid(dr))) return 0L;
   r = (X2C_INT32)dl.year-(X2C_INT32)dr.year;
   if (r) return r;
   r = (X2C_INT32)dl.month-(X2C_INT32)dr.month;
   if (r) return r;
   r = (X2C_INT32)dl.day-(X2C_INT32)dr.day;
   if (r) return r;
   r = (X2C_INT32)dl.hour-(X2C_INT32)dr.hour;
   if (r) return r;
   r = (X2C_INT32)dl.min0-(X2C_INT32)dr.min0;
   if (r) return r;
   r = (X2C_INT32)dl.sec-(X2C_INT32)dr.sec;
   if (r) return r;
   return (X2C_INT32)dl.fracs-(X2C_INT32)dr.fracs;
} /* end X2C_TimeCompare() */


extern X2C_CARD32 X2C_TimeDayInt(struct X2C_TimeStruct dl,
                struct X2C_TimeStruct dr)
{
   if (!(is_valid(dl) && is_valid(dr))) return 0UL;
   if (X2C_TimeCompare(dl, dr)<=0L) return 0UL;
   return sub_days(dl, dr);
} /* end X2C_TimeDayInt() */


extern X2C_CARD32 X2C_TimeSecInt(struct X2C_TimeStruct dl,
                struct X2C_TimeStruct dr)
{
   if (!(is_valid(dl) && is_valid(dr))) return 0UL;
   if (X2C_TimeCompare(dl, dr)<=0L) return 0UL;
   return sub_secs(dl, dr);
} /* end X2C_TimeSecInt() */

static struct X2C_TimeStruct _cnst = {1UL,1UL,1UL,0UL,0UL,0UL,0UL,0L,0};

extern void X2C_TimeDayAdd(struct X2C_TimeStruct D, X2C_CARD32 days,
                struct X2C_TimeStruct * res)
{
   X2C_CARD32 d;
   X2C_CARD32 m;
   X2C_CARD32 y;
   *res = _cnst;
   if (!is_valid(D)) return;
   y = D.year;
   m = D.month;
   d = D.day;
   add_days(&y, &m, &d, days);
   *res = D;
   res->year = y;
   res->month = m;
   res->day = d;
} /* end X2C_TimeDayAdd() */


extern void X2C_TimeSecAdd(struct X2C_TimeStruct D, X2C_CARD32 secs,
                struct X2C_TimeStruct * res)
{
   X2C_CARD32 s;
   X2C_CARD32 mi;
   X2C_CARD32 h;
   X2C_CARD32 d;
   X2C_CARD32 m;
   X2C_CARD32 y;
   *res = _cnst;
   if (!is_valid(D)) return;
   y = D.year;
   m = D.month;
   d = D.day;
   h = D.hour;
   mi = D.min0;
   s = D.sec;
   add_secs(&y, &m, &d, &h, &mi, &s, secs);
   res->year = y;
   res->month = m;
   res->day = d;
   res->hour = h;
   res->min0 = mi;
   res->sec = s;
   res->fracs = D.fracs;
   res->zone = D.zone;
   res->stf = D.stf;
} /* end X2C_TimeSecAdd() */

