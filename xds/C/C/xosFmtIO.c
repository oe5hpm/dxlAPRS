/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosFmtIO.c Feb  3 14:31:18 2012" */
#include "xosFmtIO.h"
#define xosFmtIO_C_
#include "xrtsOS.h"

#define spaces "        "


static void outs(const X2C_CHAR s[], X2C_CARD32 len)
{
   if (len==0UL) return;
   X2C_StdOut(s, len);
} /* end outs() */


extern void X2C_StdOutS(X2C_CHAR s[], X2C_CARD32 w)
{
   X2C_CARD32 l;
   l = 0UL;
   while (s[l]) ++l;
   outs(s, l);
   if (w>l) {
      while (w-l>8UL) {
         outs("        ", 8UL);
         l += 8UL;
      }
      if (w>l) outs("        ", w-l);
   }
} /* end X2C_StdOutS() */


extern void X2C_HexToStr(X2C_CHAR s[], X2C_CARD32 * pos, X2C_CARD32 no)
{
   X2C_CARD32 i;
   X2C_CARD32 d;
   *pos += 8UL;
   for (i = 0UL; i<=7UL; i++) {
      d = no&15UL;
      no = no/16UL;
      --*pos;
      if (d>9UL) s[*pos] = (X2C_CHAR)((65UL+d)-10UL);
      else s[*pos] = (X2C_CHAR)(48UL+d);
   } /* end for */
} /* end X2C_HexToStr() */


extern void X2C_StdOutH(X2C_CARD32 no, X2C_CARD32 w)
{
   X2C_CHAR buf[12];
   X2C_CARD32 pos;
   pos = 0UL;
   X2C_HexToStr(buf, &pos, no);
   if (w>8UL) {
      while (w>16UL) {
         outs("        ", 8UL);
         w -= 8UL;
      }
      if (w>8UL) outs("        ", w-8UL);
   }
   outs(buf, 8UL);
} /* end X2C_StdOutH() */


extern void X2C_DecToStr(X2C_CHAR s[], X2C_CARD32 * pos, X2C_CARD32 no)
{
   X2C_CARD32 i;
   X2C_CARD32 l;
   i = 1000000000UL;
   l = 10UL;
   while (i>no) {
      i = i/10UL;
      --l;
   }
   if (l==0UL) l = 1UL;
   *pos += l;
   i = *pos;
   while (l>0UL) {
      --i;
      s[i] = (X2C_CHAR)(48UL+no%10UL);
      no = no/10UL;
      --l;
   }
} /* end X2C_DecToStr() */


extern void X2C_StdOutD(X2C_CARD32 no, X2C_CARD32 w)
{
   X2C_CHAR buf[12];
   X2C_CARD32 pos;
   pos = 0UL;
   X2C_DecToStr(buf, &pos, no);
   if (w>pos) {
      while (w-pos>8UL) {
         outs("        ", 8UL);
         w -= 8UL;
      }
      if (w>pos) outs("        ", w-pos);
   }
   outs(buf, pos);
} /* end X2C_StdOutD() */

