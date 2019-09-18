/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#define aprsstr_C_
#ifndef osi_H_
#include "osi.h"
#endif



char aprsstr_showctrl;
/* string lib by oe5dxl */
/* needed by hpm-libs */
#define aprsstr_pi 3.1415926535898

static uint8_t CRCL[256];

static uint8_t CRCH[256];


extern void aprsstr_Assign(char a[], uint32_t a_len, char b[],
                uint32_t b_len)
{
   uint32_t i;
   i = 0UL;
   for (;;) {
      a[i] = b[i];
      if (a[i]==0 || i>=a_len-1) return;
      ++i;
      if (i>b_len-1) {
         if (i<=a_len-1) a[i] = 0;
         return;
      }
   }
} /* end Assign() */


extern void aprsstr_Append(char a[], uint32_t a_len, char b[],
                uint32_t b_len)
{
   uint32_t j;
   uint32_t i;
   j = aprsstr_Length(a, a_len);
   i = 0UL;
   for (;;) {
      if (j>a_len-1) return;
      a[j] = b[i];
      if (a[j]==0) return;
      ++i;
      ++j;
      if (i>b_len-1) {
         if (j<=a_len-1) a[j] = 0;
         return;
      }
   }
} /* end Append() */


extern void aprsstr_Delstr(char a[], uint32_t a_len, uint32_t from,
                uint32_t len)
{
   uint32_t l;
   l = aprsstr_Length(a, a_len);
   while (from+len<l) {
      a[from] = a[from+len];
      ++from;
   }
   if (from<=l) a[from] = 0;
} /* end Delstr() */


extern uint32_t aprsstr_Length(char a[], uint32_t a_len)
{
   uint32_t i;
   i = 0UL;
   while (i<=a_len-1 && a[i]) ++i;
   return i;
} /* end Length() */


extern void aprsstr_Caps(char s[], uint32_t s_len)
{
   uint32_t i;
   i = 0UL;
   while (i<=s_len-1 && s[i]) {
      s[i] = X2C_CAP(s[i]);
      ++i;
   }
} /* end Caps() */


extern void aprsstr_Extractword(char from[], uint32_t from_len,
                char word[], uint32_t word_len)
{
   uint32_t j;
   uint32_t i;
   i = 0UL;
   while ((i<=from_len-1 && from[i]) && from[i]==' ') ++i;
   j = 0UL;
   while ((i<=from_len-1 && from[i]) && from[i]!=' ') {
      /* copy first word */
      if (j<=word_len-1) {
         word[j] = from[i];
         ++j;
      }
      ++i;
   }
   if (j<=word_len-1) word[j] = 0;
   j = 0UL;
   while ((i<=from_len-1 && from[i]) && from[i]==' ') ++i;
   while (i<=from_len-1 && from[i]) {
      from[j] = from[i]; /* delete extract */
      ++j;
      ++i;
   }
   if (j<=from_len-1) from[j] = 0;
} /* end Extractword() */


extern void aprsstr_IntToStr(int32_t x, uint32_t f, char s[],
                uint32_t s_len)
{
   uint32_t j;
   uint32_t i;
   char neg;
   s[s_len-1] = 0;
   i = s_len-1;
   if (x<0L) {
      x = -x;
      neg = 1;
   }
   else neg = 0;
   do {
      --i;
      s[i] = (char)(x%10L+48L);
      x = (int32_t)((uint32_t)x/10UL);
   } while (!(x==0L || i==0UL));
   if (neg && i>0UL) {
      --i;
      s[i] = '-';
   }
   if (f<s_len-1) f = (s_len-1)-f;
   else f = 1UL;
   while (i>f) {
      --i;
      s[i] = ' ';
   }
   if (i>0UL) {
      j = 0UL;
      while (i<=s_len-1) {
         s[j] = s[i];
         ++j;
         ++i;
      }
   }
} /* end IntToStr() */


extern void aprsstr_CardToStr(uint32_t x, uint32_t f, char s[],
                uint32_t s_len)
{
   uint32_t j;
   uint32_t i;
   s[s_len-1] = 0;
   i = s_len-1;
   do {
      --i;
      s[i] = (char)(x%10UL+48UL);
      x = x/10UL;
   } while (!(x==0UL || i==0UL));
   if (f<s_len-1) f = (s_len-1)-f;
   else f = 1UL;
   while (i>f) {
      --i;
      s[i] = ' ';
   }
   if (i>0UL) {
      j = 0UL;
      while (i<=s_len-1) {
         s[j] = s[i];
         ++j;
         ++i;
      }
   }
} /* end CardToStr() */


extern void aprsstr_FixToStr(float x, uint32_t f, char s[],
                uint32_t s_len)
{
   uint32_t i;
   int32_t n;
   char neg;
   neg = x<0.0f;
   if (neg) x = -x;
   n = (int32_t)(uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   x = x-(float)n;
   aprsstr_IntToStr(n, 1UL, s, s_len);
   i = aprsstr_Length(s, s_len);
   if (neg) {
      n = (int32_t)i;
      while (n>0L) {
         s[n] = s[n-1L];
         --n;
      }
      s[0UL] = '-';
      ++i;
   }
   if (f>0UL) {
      s[i] = '.';
      ++i;
      while (f>1UL) {
         /*& (x>0.0)*/
         x = x*10.0f;
         n = (int32_t)(uint32_t)X2C_TRUNCC(x,0UL,X2C_max_longcard);
         x = x-(float)n;
         s[i] = (char)(n+48L);
         ++i;
         --f;
      }
      s[i] = 0;
   }
} /* end FixToStr() */


extern char aprsstr_StrToCard(char s[], uint32_t s_len,
                uint32_t * x)
{
   uint32_t i;
   char ok0;
   uint32_t tmp;
   *x = 0UL;
   ok0 = 0;
   tmp = s_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if ((uint8_t)s[i]>='0' && (uint8_t)s[i]<='9') {
         *x =  *x*10UL+((uint32_t)(uint8_t)s[i]-48UL);
         ok0 = 1;
      }
      else return ok0 && s[i]==0;
      if (i==tmp) break;
   } /* end for */
   return ok0;
} /* end StrToCard() */


extern char aprsstr_StrToInt(char s[], uint32_t s_len,
                int32_t * x)
{
   uint32_t i;
   char ok0;
   *x = 0L;
   ok0 = 0;
   i = 0UL;
   if (s[0UL]=='-') i = 1UL;
   while ((i<=s_len-1 && (uint8_t)s[i]>='0') && (uint8_t)s[i]<='9') {
      *x =  *x*10L+(int32_t)((uint32_t)(uint8_t)s[i]-48UL);
      ok0 = 1;
      ++i;
   }
   if (i>s_len-1 || s[i]) ok0 = 0;
   if (s[0UL]=='-') *x = -*x;
   return ok0;
} /* end StrToInt() */

#define aprsstr_M (X2C_DIVR(X2C_max_real,100.0f))


extern char aprsstr_StrToFix(float * x, char s[],
                uint32_t s_len)
{
   uint32_t i;
   float p;
   char ok0;
   char neg;
   i = 0UL;
   neg = 0;
   ok0 = 0;
   while (i<=s_len-1 && s[i]==' ') ++i;
   if (s[i]=='-') {
      neg = 1;
      ++i;
   }
   *x = 0.0f;
   while ((i<=s_len-1 && (uint8_t)s[i]>='0') && (uint8_t)s[i]<='9') {
      if (*x<X2C_DIVR(X2C_max_real,100.0f)) {
         *x =  *x*10.0f+(float)((uint32_t)(uint8_t)s[i]-48UL);
      }
      ++i;
      ok0 = 1;
   }
   if (i<=s_len-1 && s[i]=='.') {
      ++i;
      p = 0.1f;
      while ((i<=s_len-1 && (uint8_t)s[i]>='0') && (uint8_t)s[i]<='9') {
         *x = *x+p*(float)((uint32_t)(uint8_t)s[i]-48UL);
         p = p*0.1f;
         ++i;
         ok0 = 1;
      }
   }
   if (neg) *x = -*x;
   return ok0 && (i>s_len-1 || s[i]==0);
} /* end StrToFix() */


extern char aprsstr_StrCmp(char a[], uint32_t a_len,
                char b[], uint32_t b_len)
{
   uint32_t i;
   i = 0UL;
   for (;;) {
      if (a[i]!=b[i]) return 0;
      if ((a[i]==0 || i>=a_len-1) || i>=b_len-1) return 1;
      ++i;
   }
   return 0;
} /* end StrCmp() */


extern int32_t aprsstr_InStr(char a[], uint32_t a_len, char b[],
                uint32_t b_len)
/* position b in a else -1*/
{
   uint32_t j;
   uint32_t i;
   if (b[0UL]) {
      i = 0UL;
      j = 0UL;
      while (i+j<=a_len-1 && a[i+j]) {
         if (a[i+j]!=b[j]) {
            j = 0UL;
            ++i;
         }
         else {
            ++j;
            if (j>b_len-1 || b[j]==0) return (int32_t)i;
         }
      }
   }
   return -1L;
} /* end InStr() */


extern void aprsstr_rightbound(char s[], uint32_t s_len, uint32_t p)
{
   uint32_t i;
   if (s_len-1<p) p = s_len-1;
   if (p<s_len-1) s[p] = 0;
   i = aprsstr_Length(s, s_len);
   while (p>0UL) {
      --p;
      if (i>0UL) {
         --i;
         s[p] = s[i];
      }
      else s[p] = ' ';
   }
} /* end rightbound() */


extern void aprsstr_TimeToStr(uint32_t t, char s[], uint32_t s_len)
{
   char h[21];
   uint32_t d;
   d = t/86400UL;
   s[0UL] = 0;
   if (d>0UL) {
      aprsstr_IntToStr((int32_t)d, 1UL, s, s_len);
      aprsstr_Append(s, s_len, "d", 2ul);
   }
   aprsstr_IntToStr((int32_t)((t/3600UL)%24UL), 2UL, h, 21ul);
   if (h[0U]==' ') h[0U] = '0';
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((int32_t)((t/60UL)%60UL), 2UL, h, 21ul);
   if (h[0U]==' ') h[0U] = '0';
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((int32_t)(t%60UL), 2UL, h, 21ul);
   if (h[0U]==' ') h[0U] = '0';
   aprsstr_Append(s, s_len, h, 21ul);
} /* end TimeToStr() */


static void tostr(char tstr[], uint32_t tstr_len, uint32_t n,
                const char s[], uint32_t s_len)
{
   char h[2];
   h[1U] = 0;
   h[0U] = (char)(48UL+n/10UL);
   aprsstr_Append(tstr, tstr_len, h, 2ul);
   h[0U] = (char)(48UL+n%10UL);
   aprsstr_Append(tstr, tstr_len, h, 2ul);
   aprsstr_Append(tstr, tstr_len, s, s_len);
} /* end tostr() */


extern void aprsstr_DateToStr(uint32_t time0, char tstr[],
                uint32_t tstr_len)
{
   uint32_t monthlen;
   uint32_t day;
   uint32_t year;
   uint32_t mon;
   char k[13];
   tstr[0UL] = 0;
   day = 25568UL+time0/86400UL;
   /*IF day>=36584 THEN INC(day) END;*/
   year = (day*4UL)/1461UL;
   day = 1UL+((day*4UL)%1461UL)/4UL;
   mon = 0UL;
   k[0U] = 0;
   aprsstr_Append(k, 13ul, "3303232332323", 14ul);
   if ((year&3UL)==0UL) k[2U] = '1';
   for (;;) {
      ++mon;
      monthlen = (uint32_t)(uint8_t)k[mon]-20UL;
      if (day<=monthlen) break;
      day -= monthlen;
   }
   tostr(tstr, tstr_len, 19UL+year/100UL, "", 1ul);
   tostr(tstr, tstr_len, year%100UL, ".", 2ul);
   tostr(tstr, tstr_len, mon, ".", 2ul);
   tostr(tstr, tstr_len, day, " ", 2ul);
   tostr(tstr, tstr_len, (time0/3600UL)%24UL, ":", 2ul);
   tostr(tstr, tstr_len, (time0/60UL)%60UL, ":", 2ul);
   tostr(tstr, tstr_len, time0%60UL, "", 1ul);
} /* end DateToStr() */


static uint32_t dig(char * err, char c)
{
   if ((uint8_t)c<'0' || (uint8_t)c>'9') {
      *err = 1;
      return 0UL;
   }
   else return (uint32_t)(uint8_t)c-48UL;
   return 0;
} /* end dig() */


extern char aprsstr_StrToTime(char s[], uint32_t s_len,
                uint32_t * time0)
/* <yyyy mm dd hh> to s since 1970*/
{
   uint32_t mi;
   uint32_t h;
   uint32_t k;
   uint32_t d;
   uint32_t m;
   uint32_t y;
   char err;
   char aprsstr_StrToTime_ret;
   X2C_PCOPY((void **)&s,s_len);
   err = 0;
   y = dig(&err, s[0UL])*1000UL+dig(&err, s[1UL])*100UL+dig(&err,
                s[2UL])*10UL+dig(&err, s[3UL]);
   if (y<1970UL || y>2100UL) {
      aprsstr_StrToTime_ret = 0;
      goto label;
   }
   *time0 = (y-1970UL)*365UL+(y-1969UL)/4UL;
   m = dig(&err, s[5UL])*10UL+dig(&err, s[6UL]);
   d = dig(&err, s[8UL])*10UL+dig(&err, s[9UL]);
   if (d<1UL || d>31UL) {
      aprsstr_StrToTime_ret = 0;
      goto label;
   }
   k = (uint32_t)((y&3UL)==0UL);
   *time0 += d-1UL;
   if (m>2UL) *time0 += k;
   switch (m) {
   case 1UL:
      break;
   case 2UL:
      if (d>28UL+k) err = 1;
      *time0 += 31UL;
      break;
   case 3UL:
      *time0 += 59UL;
      break;
   case 4UL:
      if (d>30UL) err = 1;
      *time0 += 90UL;
      break;
   case 5UL:
      *time0 += 120UL;
      break;
   case 6UL:
      if (d>30UL) err = 1;
      *time0 += 151UL;
      break;
   case 7UL:
      *time0 += 181UL;
      break;
   case 8UL:
      *time0 += 212UL;
      break;
   case 9UL:
      if (d>30UL) err = 1;
      *time0 += 243UL;
      break;
   case 10UL:
      *time0 += 273UL;
      break;
   case 11UL:
      if (d>30UL) err = 1;
      *time0 += 304UL;
      break;
   case 12UL:
      *time0 += 334UL;
      break;
   default:;
      err = 1;
      break;
   } /* end switch */
   if (err) {
      aprsstr_StrToTime_ret = 0;
      goto label;
   }
   h = dig(&err, s[11UL])*10UL+dig(&err, s[12UL]);
   if (h>24UL) {
      aprsstr_StrToTime_ret = 0;
      goto label;
   }
   mi = dig(&err, s[14UL])*10UL+dig(&err, s[15UL]);
   if (mi>=60UL) {
      aprsstr_StrToTime_ret = 0;
      goto label;
   }
   *time0 = ( *time0*24UL+h)*3600UL+mi*60UL;
   aprsstr_StrToTime_ret = 1;
   label:;
   X2C_PFREE(s);
   return aprsstr_StrToTime_ret;
} /* end StrToTime() */


static char Hex(uint32_t d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


extern void aprsstr_CtrlHex(char s[], uint32_t s_len)
/* replace ctrl char by <hex> */
{
   uint32_t len;
   uint32_t j;
   uint32_t i;
   uint32_t tmp;
   len = aprsstr_Length(s, s_len);
   i = 0UL;
   while (i<len && len<(s_len-1)-4UL) {
      if ((uint8_t)s[i]<' ') {
         len += 3UL;
         tmp = i+4UL;
         j = len;
         if (j>=tmp) for (;; j--) {
            s[j] = s[j-3UL];
            if (j==tmp) break;
         } /* end for */
         j = (uint32_t)(uint8_t)s[i];
         s[i] = '<';
         ++i;
         s[i] = Hex(j/16UL);
         ++i;
         s[i] = Hex(j);
         ++i;
         s[i] = '>';
      }
      ++i;
   }
   s[i] = 0;
} /* end CtrlHex() */

#define aprsstr_PORTSEP ":"

#define aprsstr_CHECKSEP "/"

#define aprsstr_DEFAULTIP 0x7F000001 


extern int32_t aprsstr_GetIp2(char h[], uint32_t h_len,
                uint32_t * ip, uint32_t * dp, uint32_t * lp,
                char * check)
{
   uint32_t n;
   uint32_t p;
   uint32_t i;
   char ok0;
   int32_t aprsstr_GetIp2_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=5UL; i++) {
      if (i>=3UL || h[0UL]!=':') {
         n = 0UL;
         ok0 = 0;
         while ((uint8_t)h[p]>='0' && (uint8_t)h[p]<='9') {
            ok0 = 1;
            n = (n*10UL+(uint32_t)(uint8_t)h[p])-48UL;
            ++p;
         }
         if (!ok0) {
            aprsstr_GetIp2_ret = -1L;
            goto label;
         }
      }
      if (i<3UL) {
         if (h[0UL]!=':') {
            if (h[p]!='.' || n>255UL) {
               aprsstr_GetIp2_ret = -1L;
               goto label;
            }
            *ip =  *ip*256UL+n;
         }
      }
      else if (i==3UL) {
         if (h[0UL]!=':') {
            *ip =  *ip*256UL+n;
            if (h[p]!=':' || n>255UL) {
               aprsstr_GetIp2_ret = -1L;
               goto label;
            }
         }
         else {
            p = 0UL;
            *ip = 2130706433UL;
         }
      }
      else if (i==4UL) {
         *check = h[p]=='/';
         if (h[p]!=':' && h[p]!='/' || n>65535UL) {
            aprsstr_GetIp2_ret = -1L;
            goto label;
         }
         *dp = n;
      }
      else if (n>65535UL) {
         aprsstr_GetIp2_ret = -1L;
         goto label;
      }
      *lp = n;
      ++p;
   } /* end for */
   aprsstr_GetIp2_ret = 0L;
   label:;
   X2C_PFREE(h);
   return aprsstr_GetIp2_ret;
} /* end GetIp2() */


extern void aprsstr_ipv4tostr(uint32_t ip, char s[], uint32_t s_len)
{
   char h[21];
   aprsstr_IntToStr((int32_t)(ip/16777216UL), 0UL, s, s_len);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip/65536UL&255UL), 0UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip/256UL&255UL), 0UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((int32_t)(ip&255UL), 0UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
} /* end ipv4tostr() */


extern void aprsstr_HashCh(char c, uint8_t * hashl, uint8_t * hashh)
{
   uint8_t b;
   if (c!=' ') {
      b = (uint8_t)((uint8_t)(uint8_t)c^*hashl);
      *hashl = CRCL[b]^*hashh;
      *hashh = CRCH[b];
   }
} /* end HashCh() */


extern void aprsstr_AppCRC(char frame[], uint32_t frame_len,
                int32_t size)
{
   uint8_t h;
   uint8_t l;
   uint8_t b;
   int32_t i;
   int32_t tmp;
   l = 0U;
   h = 0U;
   tmp = size-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (uint8_t)((uint8_t)(uint8_t)frame[i]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i==tmp) break;
   } /* end for */
   frame[size] = (char)l;
   frame[size+1L] = (char)h;
} /* end AppCRC() */


extern uint32_t aprsstr_Hash(char frame[], uint32_t frame_len,
                int32_t start, int32_t end)
{
   uint8_t h;
   uint8_t l;
   uint8_t b;
   int32_t i;
   char c;
   int32_t tmp;
   l = 0U;
   h = 0U;
   tmp = end-1L;
   i = start;
   if (i<=tmp) for (;; i++) {
      c = frame[i];
      if (c!=' ') {
         b = (uint8_t)((uint8_t)(uint8_t)c^l);
         l = CRCL[b]^h;
         h = CRCH[b];
      }
      if (i==tmp) break;
   } /* end for */
   return (uint32_t)(uint8_t)(char)l+(uint32_t)(uint8_t)
                (char)h*256UL;
} /* end Hash() */

static char aprsstr_CTRL = '\003';

static char aprsstr_PID = '\360';

#define aprsstr_cTO ">"

#define aprsstr_cVIA ","

#define aprsstr_cLASTCALL ":"

#define aprsstr_cSSID "-"

#define aprsstr_cREPEATED "*"

#define aprsstr_MAXVIAS 8

#define aprsstr_MAXSSID 15

#define aprsstr_SSIDBASE 48

#define aprsstr_CALLFILL " "

#define aprsstr_MAXINFOLEN 256


static char call(int32_t * p, char raw[], uint32_t raw_len,
                uint32_t * i, char mon[], uint32_t mon_len,
                char sep1, char sep2, char sep3,
                uint32_t sbase)
{
   uint32_t s;
   uint32_t l;
   l = 0UL;
   while ((((mon[*i] && mon[*i]!=sep1) && mon[*i]!=sep2) && mon[*i]!=sep3)
                && mon[*i]!='-') {
      s = (uint32_t)(uint8_t)mon[*i]*2UL&255UL;
      if (s<=64UL) return 0;
      raw[*p] = (char)s;
      ++*p;
      ++*i;
      ++l;
      if (l>=7UL) return 0;
   }
   while (l<6UL) {
      raw[*p] = '@';
      ++*p;
      ++l;
   }
   s = 0UL;
   if (mon[*i]=='-') {
      ++*i;
      while ((uint8_t)mon[*i]>='0' && (uint8_t)mon[*i]<='9') {
         s = (s*10UL+(uint32_t)(uint8_t)mon[*i])-48UL;
         ++*i;
      }
      if (s>15UL) return 0;
   }
   raw[*p] = (char)((s+sbase)*2UL);
   ++*p;
   return 1;
} /* end call() */


extern void aprsstr_mon2raw(char mon[], uint32_t mon_len,
                char raw[], uint32_t raw_len, int32_t * p)
{
   uint32_t r;
   uint32_t n;
   uint32_t i;
   uint32_t tmp;
   *p = 7L;
   i = 0UL;
   if (!call(p, raw, raw_len, &i, mon, mon_len, '>', 0, 0, 48UL)) {
      *p = 0L; /* from call */
      return;
   }
   *p = 0L;
   if (mon[i]!='>') return;
   /* ">" */
   ++i;
   if (!call(p, raw, raw_len, &i, mon, mon_len, ':', ',', 0, 112UL)) {
      *p = 0L; /* dest call bit 7 for UI v2 command */
      return;
   }
   /*  IF NOT call(cLASTCALL, cVIA, 0C, SSIDBASE) THEN p:=0; RETURN END;
                (* dest call bit 7 for UI v2 command *) */
   *p = 14L;
   n = 0UL;
   while (mon[i]==',') {
      ++i;
      if (!call(p, raw, raw_len, &i, mon, mon_len, ':', ',', '*', 48UL)) {
         *p = 0L;
         return;
      }
      ++n;
      if (n>8UL) {
         *p = 0L;
         return;
      }
      if (mon[i]=='*') {
         /* "*" has repeatet sign */
         ++i;
         r = (uint32_t)*p;
         if (r>=21UL) for (tmp = (uint32_t)(r-21UL)/7UL;;) {
            raw[r-1UL] = (char)((uint32_t)(uint8_t)raw[r-1UL]+128UL);
                 /* set "has repeated" flags */
            if (!tmp) break;
            --tmp;
            r -= 7UL;
         } /* end for */
      }
   }
   if (*p==0L || mon[i]!=':') {
      *p = 0L; /* ":" start of info sign */
      return;
   }
   raw[*p-1L] = (char)((uint32_t)(uint8_t)raw[*p-1L]+1UL);
                /* end address field mark */
   raw[*p] = '\003';
   ++*p;
   raw[*p] = '\360';
   ++*p;
   ++i;
   n = 256UL;
   while (mon[i] && i<=mon_len-1) {
      /* copy info part */
      if (*p>=(int32_t)(raw_len-1)-2L || n==0UL) {
         *p = 0L; /* spare 2 bytes for crc */
         return;
      }
      raw[*p] = mon[i];
      ++*p;
      ++i;
      --n;
   }
   aprsstr_AppCRC(raw, raw_len, *p);
   *p += 2L;
} /* end mon2raw() */


extern char aprsstr_Call2Str(char r[], uint32_t r_len,
                char t[], uint32_t t_len, uint32_t pos,
                uint32_t * len)
{
   uint32_t ssid;
   uint32_t e;
   uint32_t i;
   char c;
   uint32_t tmp;
   e = pos;
   tmp = pos+5UL;
   i = pos;
   if (i<=tmp) for (;; i++) {
      if (r[i]!='@') e = i;
      if (i==tmp) break;
   } /* end for */
   tmp = e;
   i = pos;
   if (i<=tmp) for (;; i++) {
      c = (char)((uint32_t)(uint8_t)r[i]>>1);
      if ((uint8_t)c<=' ') {
         /* ctrl char in call */
         if (aprsstr_showctrl) c = '^';
         else {
            t[*len] = 0;
            *len = 0UL;
            return 0;
         }
      }
      t[*len] = c;
      ++*len;
      if (i==tmp) break;
   } /* end for */
   ssid = (uint32_t)(uint8_t)r[pos+6UL]>>1&15UL;
   if (ssid>0UL) {
      t[*len] = '-';
      ++*len;
      if (ssid>9UL) {
         t[*len] = '1';
         ++*len;
      }
      t[*len] = (char)(ssid%10UL+48UL);
      ++*len;
   }
   return 1;
} /* end Call2Str() */


static void brandghost(char b[], uint32_t b_len, uint32_t brand)
{
   uint32_t l;
   uint32_t j;
   uint32_t i;
   char s1[21];
   char s[21];
   uint32_t tmp;
   i = 0UL;
   for (;;) {
      if (i>=b_len-1 || b[i]==0) return;
      if (b[i]==',' || b[i]==':') break;
      ++i;
   }
   /*
     IF NOT v1 & (b[i]=",") THEN
       IF ((b[i+1]<>"W") OR (b[i+2]<>"I") OR (b[i+3]<>"D") OR (b[i+4]<>"E")
       OR (b[i+5]<>"1") OR (b[i+6]<>"-") OR (b[i+7]<>"1")
       OR (b[i+8]<>",")
       OR (b[i+9]<>"W") OR (b[i+10]<>"I") OR (b[i+11]<>"D") OR (b[i+12]<>"E")
                 OR (b[i+13]<>"2")
       OR (b[i+14]<>"-") OR (b[i+15]<>"1") & (b[i+15]<>"2") OR (b[i+16]<>":")
                )
       & ((b[i+1]<>"R") OR (b[i+2]<>"E") OR (b[i+3]<>"L") OR (b[i+4]<>"A")
                OR (b[i+5]<>"Y")
       OR (b[i+6]<>":")) THEN v1:=TRUE END;
     END;
   */
   if (b[i]==',') {
      j = i+1UL;
      for (;;) {
         /* goto end of first via */
         if (j>=b_len-1 || b[j]==0) return;
         if (b[j]==':' || b[j]==',') break;
         ++j;
      }
      if (brand<256UL && ((((j<=3UL || (uint8_t)b[j-1UL]<'0')
                || (uint8_t)b[j-1UL]>'9') || b[j-2UL]!='-')
                || b[j-1UL]!=b[j-3UL])) return;
      /* frame has n<>N so known as not direct */
      j = i+1UL;
      for (;;) {
         /* look for h-bit */
         if (j>=b_len-1 || b[j]==0) return;
         if (b[j]==':') break;
         if (b[j]=='*') return;
         /* frame has h bit so known as not direct */
         ++j;
      }
   }
   strncpy(s,",GHOST",21u);
   aprsstr_IntToStr((int32_t)brand, 0UL, s1, 21ul);
   aprsstr_Append(s, 21ul, s1, 21ul);
   aprsstr_Append(s, 21ul, "*", 2ul);
   l = aprsstr_Length(s, 21ul);
   i = 0UL;
   while ((i<=b_len-1 && b[i]) && b[i]!=':') ++i;
   if (b[i]==':') {
      j = i;
      while (j<=b_len-1 && b[j]) ++j;
      if (b[j]==0 && j+l<b_len-1) {
         /* insert ,GHOSTnnn* */
         while (j>=i) {
            b[j+l] = b[j];
            --j;
         }
         tmp = l-1UL;
         i = 0UL;
         if (i<=tmp) for (;; i++) {
            ++j;
            b[j] = s[i];
            if (i==tmp) break;
         } /* end for */
      }
   }
} /* end brandghost() */


extern void aprsstr_raw2mon(char raw[], uint32_t raw_len,
                char mon[], uint32_t mon_len, uint32_t len,
                uint32_t * p, aprsstr_GHOSTSET ghostset)
{
   uint32_t brand;
   uint32_t i;
   char hcheck;
   if ((((((len>21UL && !((uint32_t)(uint8_t)raw[13UL]&1))
                && raw[14UL]=='\202') && raw[15UL]=='\240')
                && raw[16UL]=='\244') && raw[17UL]=='\246')
                && raw[18UL]=='@') brand = 256UL;
   else {
      brand = (uint32_t)(uint8_t)raw[6UL]/32UL+((uint32_t)(uint8_t)
                raw[13UL]/32UL)*8UL;
   }
   *p = 0UL;
   mon[0UL] = 0;
   i = 0UL;
   while (!((uint32_t)(uint8_t)raw[i]&1)) {
      ++i;
      if (i>len) {
         mon[0UL] = 0; /* no address end mark found */
         return;
      }
   }
   if (i%7UL!=6UL) {
      mon[0UL] = 0; /* address end not modulo 7 error */
      return;
   }
   if (!aprsstr_Call2Str(raw, raw_len, mon, mon_len, 7UL, p)) {
      mon[0UL] = 0;
      return;
   }
   mon[*p] = '>';
   ++*p;
   if (!aprsstr_Call2Str(raw, raw_len, mon, mon_len, 0UL, p)) {
      mon[0UL] = 0;
      return;
   }
   i = 14UL;
   hcheck = 1;
   while (i+6UL<len && !((uint32_t)(uint8_t)raw[i-1UL]&1)) {
      mon[*p] = ',';
      ++*p;
      if (!aprsstr_Call2Str(raw, raw_len, mon, mon_len, i, p)) {
         mon[0UL] = 0;
         return;
      }
      if ((uint32_t)(uint8_t)raw[i+6UL]>=128UL) {
         if (!hcheck) {
            mon[0UL] = 0; /* wrong H bit */
            return;
         }
         if (((uint32_t)(uint8_t)raw[i+6UL]&1) || (uint32_t)(uint8_t)
                raw[i+13UL]<128UL) {
            mon[*p] = '*';
            ++*p;
         }
      }
      else hcheck = 0;
      i += 7UL;
   }
   if (raw[i]!='\003' && raw[i]!='\023') {
      mon[0UL] = 0; /* not UI frame */
      return;
   }
   if (raw[i]=='\023') brand += 64UL;
   if (raw[i+1UL]!='\360') brand += 128UL;
   i += 2UL; /* ctrl, pid */
   mon[*p] = ':';
   ++*p;
   while (i<len && *p<(mon_len-1)-1UL) {
      if (raw[i]) {
         mon[*p] = raw[i];
         ++*p;
      }
      ++i;
   }
   /*
     mon[p]:=15C; INC(p);
     mon[p]:=12C; INC(p);
   */
   mon[*p] = 0;
   ++*p;
   mon[*p] = 0;
   /*FOR i:=0 TO 256 DO IF i IN ghostset THEN WrInt(i, 4) END; END;
                WrStrLn("=ghs"); */
   if (X2C_INL((int32_t)256,257,ghostset) && brand>255UL || X2C_INL(brand,
                257,ghostset)) brandghost(mon, mon_len, brand);
/*WrInt(ORD(raw[6]) DIV 32,1); WrInt(ORD(raw[13]) DIV 32,1);WrStrLn(mon); */
} /* end raw2mon() */


extern void aprsstr_extrudp2(char ib[], uint32_t ib_len, char ud[],
                 uint32_t ud_len, int32_t * len)
/* extract axudp2 header */
{
   int32_t j;
   int32_t i;
   i = 0L;
   j = 0L;
   do {
      ud[i] = ib[i];
      ++i;
   } while (!((i>=(int32_t)(ud_len-1) || i>=*len) || ib[i]==0));
   ud[i] = 0;
   ++i;
   if (i>=*len) *len = 0L;
   else {
      *len -= i;
      while (j<*len) {
         ib[j] = ib[i];
         ++j;
         ++i;
      }
   }
} /* end extrudp2() */


extern void aprsstr_cleanfilename(char s[], uint32_t s_len)
{
   uint32_t j;
   uint32_t i;
   i = 0UL;
   j = 0UL;
   while (i<=s_len-1 && s[i]) {
      /* remove leftside junk */
      if (j>0UL || (uint8_t)s[i]>' ') {
         s[j] = s[i];
         ++j;
      }
      ++i;
   }
   while (j>0UL && (uint8_t)s[j-1UL]<=' ') --j;
   if (j<=s_len-1) s[j] = 0;
} /* end cleanfilename() */

#define aprsstr_POLINOM 0x8408 


static void Gencrctab(void)
{
   uint32_t c;
   uint32_t crc;
   uint32_t i;
   for (c = 0UL; c<=255UL; c++) {
      crc = 255UL-c;
      for (i = 0UL; i<=7UL; i++) {
         if ((crc&1)) crc = (uint32_t)((uint32_t)(crc>>1)^0x8408UL);
         else crc = crc>>1;
      } /* end for */
      CRCL[c] = (uint8_t)crc;
      CRCH[c] = (uint8_t)(255UL-(crc>>8));
   } /* end for */
} /* end Gencrctab() */


extern void aprsstr_loctopos(struct aprsstr_POSITION * pos, char loc[],
                uint32_t loc_len)
{
   uint32_t l;
   uint32_t i;
   char ok0;
   X2C_PCOPY((void **)&loc,loc_len);
   ok0 = 0;
   l = aprsstr_Length(loc, loc_len);
   i = 0UL;
   while (i<l) {
      loc[i] = X2C_CAP(loc[i]);
      ++i;
   }
   if ((((((((((((l>=6UL && (uint8_t)loc[0UL]>='A') && (uint8_t)
                loc[0UL]<='R') && (uint8_t)loc[1UL]>='A') && (uint8_t)
                loc[1UL]<='R') && (uint8_t)loc[2UL]>='0') && (uint8_t)
                loc[2UL]<='9') && (uint8_t)loc[3UL]>='0') && (uint8_t)
                loc[3UL]<='9') && (uint8_t)loc[4UL]>='A') && (uint8_t)
                loc[4UL]<='X') && (uint8_t)loc[5UL]>='A') && (uint8_t)
                loc[5UL]<='X') {
      pos->long0 = (float)((uint32_t)(uint8_t)loc[0UL]-65UL)
                *20.0f+(float)((uint32_t)(uint8_t)loc[2UL]-48UL)
                *2.0f+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[4UL]-65UL)+0.5f,12.0f);
      pos->lat = (float)((uint32_t)(uint8_t)loc[1UL]-65UL)
                *10.0f+(float)((uint32_t)(uint8_t)loc[3UL]-48UL)
                +X2C_DIVR((float)((uint32_t)(uint8_t)loc[5UL]-65UL)
                +0.5f,24.0f);
      if (l==6UL) ok0 = 1;
      if ((((l>=8UL && (uint8_t)loc[6UL]>='0') && (uint8_t)loc[6UL]<='9')
                 && (uint8_t)loc[7UL]>='0') && (uint8_t)loc[7UL]<='9') {
         pos->long0 = (pos->long0+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[6UL]-48UL),120.0f))-0.0375f;
         pos->lat = (pos->lat+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[7UL]-48UL),240.0f))-0.01875f;
         if (l==8UL) ok0 = 1;
      }
      if ((((l>=10UL && (uint8_t)loc[8UL]>='A') && (uint8_t)
                loc[8UL]<='X') && (uint8_t)loc[9UL]>='A') && (uint8_t)
                loc[9UL]<='X') {
         pos->long0 = (pos->long0+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[8UL]-65UL),2880.0f))-3.9930555555556E-3f;
         pos->lat = (pos->lat+X2C_DIVR((float)((uint32_t)(uint8_t)
                loc[9UL]-65UL),5760.0f))-1.9965277777778E-3f;
         if (l==10UL) ok0 = 1;
      }
   }
   if (ok0) {
      pos->long0 = (pos->long0-180.0f)*1.7453292519943E-2f;
      pos->lat = (pos->lat-90.0f)*1.7453292519943E-2f;
   }
   else aprsstr_posinval(pos);
   X2C_PFREE(loc);
} /* end loctopos() */


extern void aprsstr_postoloc(char loc[], uint32_t loc_len,
                struct aprsstr_POSITION pos)
{
   uint32_t bc;
   uint32_t lc;
   float br;
   float lr;
   lr = (pos.long0*5.7295779513082E+1f+180.0f)*2880.0f;
   if (lr<0.0f) lr = 0.0f;
   lc = (uint32_t)X2C_TRUNCC(lr,0UL,X2C_max_longcard);
   br = (pos.lat*5.7295779513082E+1f+90.0f)*5760.0f;
   if (br<0.0f) br = 0.0f;
   bc = (uint32_t)X2C_TRUNCC(br,0UL,X2C_max_longcard);
   loc[0UL] = (char)(65UL+lc/57600UL);
   loc[1UL] = (char)(65UL+bc/57600UL);
   loc[2UL] = (char)(48UL+(lc/5760UL)%10UL);
   loc[3UL] = (char)(48UL+(bc/5760UL)%10UL);
   loc[4UL] = (char)(65UL+(lc/240UL)%24UL);
   loc[5UL] = (char)(65UL+(bc/240UL)%24UL);
   loc[6UL] = (char)(48UL+(lc/24UL)%10UL);
   loc[7UL] = (char)(48UL+(bc/24UL)%10UL);
   loc[8UL] = (char)(65UL+lc%24UL);
   loc[9UL] = (char)(65UL+bc%24UL);
   loc[10UL] = 0;
} /* end postoloc() */


extern void aprsstr_posinval(struct aprsstr_POSITION * pos)
{
   pos->long0 = 0.0f;
   pos->lat = 0.0f;
} /* end posinval() */


extern void aprsstr_BEGIN(void)
{
   static int aprsstr_init = 0;
   if (aprsstr_init) return;
   aprsstr_init = 1;
   if (sizeof(uint8_t)!=1) X2C_ASSERT(0);
   if (sizeof(aprsstr_GHOSTSET)!=36) X2C_ASSERT(0);
   osi_BEGIN();
   aprsstr_showctrl = 0;
   Gencrctab();
}

