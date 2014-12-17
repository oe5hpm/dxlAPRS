/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrInt64.c Feb  3 14:30:28 2012" */
#include "xrInt64.h"
#define xrInt64_C_



typedef X2C_CARD32 Arr[4];

#define M 0x10000 

#define S 0x7FFF 


extern void X2C_INTTO64(struct X2C_int64 * res, X2C_INT32 val)
{
   if (val<0L) res->high = X2C_max_longcard;
   else res->high = 0UL;
   res->low = (X2C_CARD32)val;
} /* end X2C_INTTO64() */


extern void X2C_CARDTO64(struct X2C_int64 * res, X2C_CARD32 val)
{
   res->high = 0UL;
   res->low = val;
} /* end X2C_CARDTO64() */


extern X2C_BOOLEAN X2C_IsNeg64(struct X2C_int64 a)
{
   return a.high>2147483647UL;
} /* end X2C_IsNeg64() */


static void toArr(Arr a, const struct X2C_int64 val)
{
   a[0U] = val.low&65535UL;
   a[1U] = val.low/65536UL;
   a[2U] = val.high&65535UL;
   a[3U] = val.high/65536UL;
} /* end toArr() */


static void to64(struct X2C_int64 * x, const Arr a)
{
   x->low = a[1U]*65536UL+a[0U];
   x->high = a[3U]*65536UL+a[2U];
} /* end to64() */


static void neg(Arr a)
{
   X2C_CARD32 i;
   X2C_CARD32 r;
   for (i = 0UL; i<=3UL; i++) {
      a[i] = (65536UL-a[i])-1UL;
   } /* end for */
   r = 1UL;
   i = 0UL;
   while (i<4UL && r) {
      r += a[i];
      a[i] = r&65535UL;
      r = r/65536UL;
      ++i;
   }
} /* end neg() */


static X2C_BOOLEAN add(Arr res, const Arr a, const Arr b)
{
   X2C_CARD32 i;
   X2C_CARD32 r;
   r = 0UL;
   for (i = 0UL; i<=3UL; i++) {
      r = a[i]+b[i]+r;
      res[i] = r&65535UL;
      r = r/65536UL;
   } /* end for */
   return r!=0UL;
} /* end add() */


static void mul(Arr res, X2C_CARD32 a, X2C_CARD32 b)
{
   X2C_CARD32 r;
   X2C_CARD32 b1;
   X2C_CARD32 b0;
   X2C_CARD32 a1;
   X2C_CARD32 a0;
   a0 = a&65535UL;
   a1 = a/65536UL;
   b0 = b&65535UL;
   b1 = b/65536UL;
   r = a0*b0;
   res[0U] = r&65535UL;
   r = r/65536UL;
   r += a0*b1+b0*a1;
   res[1U] = r&65535UL;
   r = r/65536UL;
   r += a1*b1;
   res[2U] = r&65535UL;
   res[3U] = r/65536UL;
} /* end mul() */


extern X2C_BOOLEAN X2C_UnMinus64(struct X2C_int64 * res, struct X2C_int64 x)
{
   Arr a;
   X2C_BOOLEAN s;
   toArr(a, x);
   s = a[3U]>32767UL;
   neg(a);
   if (s==a[3U]>32767UL && (x.high || x.low)) return 1;
   to64(res, a);
   return 0;
} /* end X2C_UnMinus64() */


extern X2C_BOOLEAN X2C_ADD64(struct X2C_int64 * res, struct X2C_int64 A,
                struct X2C_int64 B)
{
   Arr r;
   Arr b;
   Arr a;
   X2C_BOOLEAN cr;
   X2C_BOOLEAN sb;
   X2C_BOOLEAN sa;
   toArr(a, A);
   toArr(b, B);
   sa = a[3U]>32767UL;
   sb = b[3U]>32767UL;
   cr = add(r, a, b);
   to64(res, r);
   return sa==sb && r[3U]>32767UL!=sa;
} /* end X2C_ADD64() */


extern X2C_BOOLEAN X2C_64TOINT(X2C_INT32 * res, struct X2C_int64 x)
{
   *res = (X2C_INT32)x.low;
   return (x.high || x.low>=0x080000000UL)
                && (x.high!=X2C_max_longcard || x.low<0x080000000UL);
} /* end X2C_64TOINT() */


extern X2C_BOOLEAN X2C_64TOCARD(X2C_CARD32 * res, struct X2C_int64 x)
{
   *res = x.low;
   return x.high!=0UL;
} /* end X2C_64TOCARD() */


extern void X2C_MUL64(struct X2C_int64 * res, X2C_INT32 a, X2C_CARD32 b)
{
   X2C_CARD32 x;
   X2C_BOOLEAN sig;
   Arr ra;
   sig = a<0L;
   if (sig) {
      if (a==X2C_min_longint) x = 0x080000000UL;
      else x = (X2C_CARD32) -a;
   }
   else x = (X2C_CARD32)a;
   mul(ra, x, b);
   if (sig) neg(ra);
   to64(res, ra);
} /* end X2C_MUL64() */


extern int X2C_CMP64(struct X2C_int64 a, struct X2C_int64 b)
{
   X2C_BOOLEAN bn;
   X2C_BOOLEAN an;
   an = a.high>2147483647UL;
   bn = b.high>2147483647UL;
   if (an && !bn) return -1;
   if (bn && !an) return 1;
   if (a.high>b.high) return 1;
   if (a.high<b.high) return -1;
   if (a.low>b.low) return 1;
   if (a.low<b.low) return -1;
   return 0;
} /* end X2C_CMP64() */

