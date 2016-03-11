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
#include "osi.h"

/* string lib by oe5dxl */
/*FROM osi IMPORT WrInt, WrStrLn; */
static unsigned char CRCL[256];

static unsigned char CRCH[256];


extern void aprsstr_Assign(char a[], unsigned long a_len, char b[],
                unsigned long b_len)
{
   unsigned long i;
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


extern void aprsstr_Append(char a[], unsigned long a_len, char b[],
                unsigned long b_len)
{
   unsigned long j;
   unsigned long i;
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


extern void aprsstr_Delstr(char a[], unsigned long a_len, unsigned long from,
                 unsigned long len)
{
   unsigned long l;
   l = aprsstr_Length(a, a_len);
   while (from+len<l) {
      a[from] = a[from+len];
      ++from;
   }
   if (from<=l) a[from] = 0;
} /* end Delstr() */


extern unsigned long aprsstr_Length(char a[], unsigned long a_len)
{
   unsigned long i;
   i = 0UL;
   while (i<=a_len-1 && a[i]) ++i;
   return i;
} /* end Length() */


extern void aprsstr_Caps(char s[], unsigned long s_len)
{
   unsigned long i;
   i = 0UL;
   while (i<=s_len-1 && s[i]) {
      s[i] = X2C_CAP(s[i]);
      ++i;
   }
} /* end Caps() */


extern void aprsstr_Extractword(char from[], unsigned long from_len,
                char word[], unsigned long word_len)
{
   unsigned long j;
   unsigned long i;
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


extern void aprsstr_IntToStr(long x, unsigned long f, char s[],
                unsigned long s_len)
{
   unsigned long j;
   unsigned long i;
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
      x = (long)((unsigned long)x/10UL);
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


extern void aprsstr_CardToStr(unsigned long x, unsigned long f, char s[],
                unsigned long s_len)
{
   unsigned long j;
   unsigned long i;
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


extern void aprsstr_FixToStr(float x, unsigned long f, char s[],
                unsigned long s_len)
{
   unsigned long i;
   long n;
   char neg;
   neg = x<0.0f;
   if (neg) x = -x;
   n = (long)(unsigned long)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   x = x-(float)n;
   aprsstr_IntToStr(n, 1UL, s, s_len);
   i = aprsstr_Length(s, s_len);
   if (neg) {
      n = (long)i;
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
         n = (long)(unsigned long)X2C_TRUNCC(x,0UL,X2C_max_longcard);
         x = x-(float)n;
         s[i] = (char)(n+48L);
         ++i;
         --f;
      }
      s[i] = 0;
   }
} /* end FixToStr() */


extern char aprsstr_StrToCard(char s[], unsigned long s_len,
                unsigned long * x)
{
   unsigned long i;
   char ok0;
   unsigned long tmp;
   *x = 0UL;
   ok0 = 0;
   tmp = s_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if ((unsigned char)s[i]>='0' && (unsigned char)s[i]<='9') {
         *x =  *x*10UL+((unsigned long)(unsigned char)s[i]-48UL);
         ok0 = 1;
      }
      else return ok0 && s[i]==0;
      if (i==tmp) break;
   } /* end for */
   return ok0;
} /* end StrToCard() */


extern char aprsstr_StrToInt(char s[], unsigned long s_len, long * x)
{
   unsigned long i;
   char ok0;
   *x = 0L;
   ok0 = 0;
   i = 0UL;
   if (s[0UL]=='-') i = 1UL;
   while ((i<=s_len-1 && (unsigned char)s[i]>='0') && (unsigned char)
                s[i]<='9') {
      *x =  *x*10L+(long)((unsigned long)(unsigned char)s[i]-48UL);
      ok0 = 1;
      ++i;
   }
   if (i>s_len-1 || s[i]) ok0 = 0;
   if (s[0UL]=='-') *x = -*x;
   return ok0;
} /* end StrToInt() */

#define aprsstr_M (X2C_DIVR(X2C_max_real,100.0f))


extern char aprsstr_StrToFix(float * x, char s[], unsigned long s_len)
{
   unsigned long i;
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
   while ((i<=s_len-1 && (unsigned char)s[i]>='0') && (unsigned char)
                s[i]<='9') {
      if (*x<X2C_DIVR(X2C_max_real,100.0f)) {
         *x =  *x*10.0f+(float)((unsigned long)(unsigned char)s[i]-48UL);
      }
      ++i;
      ok0 = 1;
   }
   if (i<=s_len-1 && s[i]=='.') {
      ++i;
      p = 0.1f;
      while ((i<=s_len-1 && (unsigned char)s[i]>='0') && (unsigned char)
                s[i]<='9') {
         *x = *x+p*(float)((unsigned long)(unsigned char)s[i]-48UL);
         p = p*0.1f;
         ++i;
         ok0 = 1;
      }
   }
   if (neg) *x = -*x;
   return ok0 && (i>s_len-1 || s[i]==0);
} /* end StrToFix() */


extern char aprsstr_StrCmp(char a[], unsigned long a_len, char b[],
                unsigned long b_len)
{
   unsigned long i;
   i = 0UL;
   for (;;) {
      if (a[i]!=b[i]) return 0;
      if ((a[i]==0 || i>=a_len-1) || i>=b_len-1) return 1;
      ++i;
   }
   return 0;
} /* end StrCmp() */


extern long aprsstr_InStr(char a[], unsigned long a_len, char b[],
                unsigned long b_len)
/* position b in a else -1*/
{
   unsigned long j;
   unsigned long i;
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
            if (j>b_len-1 || b[j]==0) return (long)i;
         }
      }
   }
   return -1L;
} /* end InStr() */


extern void aprsstr_rightbound(char s[], unsigned long s_len,
                unsigned long p)
{
   unsigned long i;
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


extern void aprsstr_TimeToStr(unsigned long t, char s[],
                unsigned long s_len)
{
   char h[21];
   unsigned long d;
   d = t/86400UL;
   s[0UL] = 0;
   if (d>0UL) {
      aprsstr_IntToStr((long)d, 1UL, s, s_len);
      aprsstr_Append(s, s_len, "d", 2ul);
   }
   aprsstr_IntToStr((long)((t/3600UL)%24UL), 2UL, h, 21ul);
   if (h[0U]==' ') h[0U] = '0';
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((long)((t/60UL)%60UL), 2UL, h, 21ul);
   if (h[0U]==' ') h[0U] = '0';
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr((long)(t%60UL), 2UL, h, 21ul);
   if (h[0U]==' ') h[0U] = '0';
   aprsstr_Append(s, s_len, h, 21ul);
} /* end TimeToStr() */


static void tostr(char tstr[], unsigned long tstr_len, unsigned long n,
                const char s[], unsigned long s_len)
{
   char h[2];
   h[1U] = 0;
   h[0U] = (char)(48UL+n/10UL);
   aprsstr_Append(tstr, tstr_len, h, 2ul);
   h[0U] = (char)(48UL+n%10UL);
   aprsstr_Append(tstr, tstr_len, h, 2ul);
   aprsstr_Append(tstr, tstr_len, s, s_len);
} /* end tostr() */


extern void aprsstr_DateToStr(unsigned long time0, char tstr[],
                unsigned long tstr_len)
{
   unsigned long monthlen;
   unsigned long day;
   unsigned long year;
   unsigned long mon;
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
      monthlen = (unsigned long)(unsigned char)k[mon]-20UL;
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


static unsigned long dig(char * err, char c)
{
   if ((unsigned char)c<'0' || (unsigned char)c>'9') {
      *err = 1;
      return 0UL;
   }
   else return (unsigned long)(unsigned char)c-48UL;
   return 0;
} /* end dig() */


extern char aprsstr_StrToTime(char s[], unsigned long s_len,
                unsigned long * time0)
/* <yyyy mm dd hh> to s since 1970*/
{
   unsigned long mi;
   unsigned long h;
   unsigned long k;
   unsigned long d;
   unsigned long m;
   unsigned long y;
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
   k = (unsigned long)((y&3UL)==0UL);
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


static char Hex(unsigned long d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


extern void aprsstr_CtrlHex(char s[], unsigned long s_len)
/* replace ctrl char by <hex> */
{
   unsigned long len;
   unsigned long j;
   unsigned long i;
   unsigned long tmp;
   len = aprsstr_Length(s, s_len);
   i = 0UL;
   while (i<len && len<(s_len-1)-4UL) {
      if ((unsigned char)s[i]<' ') {
         len += 3UL;
         tmp = i+4UL;
         j = len;
         if (j>=tmp) for (;; j--) {
            s[j] = s[j-3UL];
            if (j==tmp) break;
         } /* end for */
         j = (unsigned long)(unsigned char)s[i];
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


extern long aprsstr_GetIp2(char h[], unsigned long h_len, unsigned long * ip,
                 unsigned long * dp, unsigned long * lp, char * check)
{
   unsigned long n;
   unsigned long p;
   unsigned long i;
   char ok0;
   long aprsstr_GetIp2_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=5UL; i++) {
      if (i>=3UL || h[0UL]!=':') {
         n = 0UL;
         ok0 = 0;
         while ((unsigned char)h[p]>='0' && (unsigned char)h[p]<='9') {
            ok0 = 1;
            n = (n*10UL+(unsigned long)(unsigned char)h[p])-48UL;
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


extern void aprsstr_ipv4tostr(unsigned long ip, char s[],
                unsigned long s_len)
{
   char h[21];
   aprsstr_IntToStr((long)(ip/16777216UL), 0UL, s, s_len);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip/65536UL&255UL), 0UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip/256UL&255UL), 0UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
   aprsstr_Append(s, s_len, ".", 2ul);
   aprsstr_IntToStr((long)(ip&255UL), 0UL, h, 21ul);
   aprsstr_Append(s, s_len, h, 21ul);
} /* end ipv4tostr() */


extern void aprsstr_HashCh(char c, unsigned char * hashl,
                unsigned char * hashh)
{
   unsigned char b;
   if (c!=' ') {
      b = (unsigned char)((unsigned char)(unsigned char)c^*hashl);
      *hashl = CRCL[b]^*hashh;
      *hashh = CRCH[b];
   }
} /* end HashCh() */


extern void aprsstr_AppCRC(char frame[], unsigned long frame_len, long size)
{
   unsigned char h;
   unsigned char l;
   unsigned char b;
   long i;
   long tmp;
   l = 0U;
   h = 0U;
   tmp = size-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (unsigned char)((unsigned char)(unsigned char)frame[i]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i==tmp) break;
   } /* end for */
   frame[size] = (char)l;
   frame[size+1L] = (char)h;
} /* end AppCRC() */


extern unsigned long aprsstr_Hash(char frame[], unsigned long frame_len,
                long start, long end)
{
   unsigned char h;
   unsigned char l;
   unsigned char b;
   long i;
   char c;
   long tmp;
   l = 0U;
   h = 0U;
   tmp = end-1L;
   i = start;
   if (i<=tmp) for (;; i++) {
      c = frame[i];
      if (c!=' ') {
         b = (unsigned char)((unsigned char)(unsigned char)c^l);
         l = CRCL[b]^h;
         h = CRCH[b];
      }
      if (i==tmp) break;
   } /* end for */
   return (unsigned long)(unsigned char)(char)l+(unsigned long)
                (unsigned char)(char)h*256UL;
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


static char call(long * p, char raw[], unsigned long raw_len,
                unsigned long * i, char mon[], unsigned long mon_len,
                char sep1, char sep2, char sep3, unsigned long sbase)
{
   unsigned long s;
   unsigned long l;
   l = 0UL;
   while ((((mon[*i] && mon[*i]!=sep1) && mon[*i]!=sep2) && mon[*i]!=sep3)
                && mon[*i]!='-') {
      s = (unsigned long)(unsigned char)mon[*i]*2UL&255UL;
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
      while ((unsigned char)mon[*i]>='0' && (unsigned char)mon[*i]<='9') {
         s = (s*10UL+(unsigned long)(unsigned char)mon[*i])-48UL;
         ++*i;
      }
      if (s>15UL) return 0;
   }
   raw[*p] = (char)((s+sbase)*2UL);
   ++*p;
   return 1;
} /* end call() */


extern void aprsstr_mon2raw(char mon[], unsigned long mon_len, char raw[],
                unsigned long raw_len, long * p)
{
   unsigned long r;
   unsigned long n;
   unsigned long i;
   unsigned long tmp;
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
         r = (unsigned long)*p;
         if (r>=21UL) for (tmp = (unsigned long)(r-21UL)/7UL;;) {
            raw[r-1UL] = (char)((unsigned long)(unsigned char)
                raw[r-1UL]+128UL); /* set "has repeated" flags */
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
   raw[*p-1L] = (char)((unsigned long)(unsigned char)raw[*p-1L]+1UL);
                /* end address field mark */
   raw[*p] = '\003';
   ++*p;
   raw[*p] = '\360';
   ++*p;
   ++i;
   n = 256UL;
   while (mon[i] && i<=mon_len-1) {
      /* copy info part */
      if (*p>=(long)(raw_len-1)-2L || n==0UL) {
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


extern char aprsstr_Call2Str(char r[], unsigned long r_len, char t[],
                unsigned long t_len, unsigned long pos, unsigned long * len)
{
   unsigned long ssid;
   unsigned long e;
   unsigned long i;
   char c;
   unsigned long tmp;
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
      c = (char)((unsigned long)(unsigned char)r[i]>>1);
      if ((unsigned char)c<=' ') {
         t[*len] = 0;
         *len = 0UL;
         return 0;
      }
      t[*len] = c;
      ++*len;
      if (i==tmp) break;
   } /* end for */
   ssid = (unsigned long)(unsigned char)r[pos+6UL]>>1&15UL;
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


static void brandghost(char b[], unsigned long b_len, unsigned long brand)
{
   unsigned long l;
   unsigned long j;
   unsigned long i;
   char s1[21];
   char s[21];
   unsigned long tmp;
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
      if (brand<256UL && ((((j<=3UL || (unsigned char)b[j-1UL]<'0')
                || (unsigned char)b[j-1UL]>'9') || b[j-2UL]!='-')
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
   aprsstr_IntToStr((long)brand, 0UL, s1, 21ul);
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


extern void aprsstr_raw2mon(char raw[], unsigned long raw_len, char mon[],
                unsigned long mon_len, unsigned long len, unsigned long * p,
                aprsstr_GHOSTSET ghostset)
{
   unsigned long brand;
   unsigned long i;
   char hcheck;
   if ((((((len>21UL && !((unsigned long)(unsigned char)raw[13UL]&1))
                && raw[14UL]=='\202') && raw[15UL]=='\240')
                && raw[16UL]=='\244') && raw[17UL]=='\246')
                && raw[18UL]=='@') brand = 256UL;
   else {
      brand = (unsigned long)(unsigned char)raw[6UL]/32UL+((unsigned long)
                (unsigned char)raw[13UL]/32UL)*8UL;
   }
   *p = 0UL;
   mon[0UL] = 0;
   i = 0UL;
   while (!((unsigned long)(unsigned char)raw[i]&1)) {
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
   while (i+6UL<len && !((unsigned long)(unsigned char)raw[i-1UL]&1)) {
      mon[*p] = ',';
      ++*p;
      if (!aprsstr_Call2Str(raw, raw_len, mon, mon_len, i, p)) {
         mon[0UL] = 0;
         return;
      }
      if ((unsigned long)(unsigned char)raw[i+6UL]>=128UL) {
         if (!hcheck) {
            mon[0UL] = 0; /* wrong H bit */
            return;
         }
         if (((unsigned long)(unsigned char)raw[i+6UL]&1) || (unsigned long)
                (unsigned char)raw[i+13UL]<128UL) {
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
   if (X2C_INL((long)256,257,ghostset) && brand>255UL || X2C_INL(brand,257,
                ghostset)) brandghost(mon, mon_len, brand);
/*WrInt(ORD(raw[6]) DIV 32,1); WrInt(ORD(raw[13]) DIV 32,1);WrStrLn(mon); */
} /* end raw2mon() */


extern void aprsstr_extrudp2(char ib[], unsigned long ib_len, char ud[],
                unsigned long ud_len, long * len)
/* extract axudp2 header */
{
   long j;
   long i;
   i = 0L;
   j = 0L;
   do {
      ud[i] = ib[i];
      ++i;
   } while (!((i>=(long)(ud_len-1) || i>=*len) || ib[i]==0));
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


extern void aprsstr_cleanfilename(char s[], unsigned long s_len)
{
   unsigned long j;
   unsigned long i;
   i = 0UL;
   j = 0UL;
   while (i<=s_len-1 && s[i]) {
      /* remove leftside junk */
      if (j>0UL || (unsigned char)s[i]>' ') {
         s[j] = s[i];
         ++j;
      }
      ++i;
   }
   while (j>0UL && (unsigned char)s[j-1UL]<=' ') --j;
   if (j<=s_len-1) s[j] = 0;
} /* end cleanfilename() */

#define aprsstr_POLINOM 0x8408 


static void Gencrctab(void)
{
   unsigned long c;
   unsigned long crc;
   unsigned long i;
   for (c = 0UL; c<=255UL; c++) {
      crc = 255UL-c;
      for (i = 0UL; i<=7UL; i++) {
         if ((crc&1)) {
            crc = (unsigned long)((unsigned long)(crc>>1)^0x8408UL);
         }
         else crc = crc>>1;
      } /* end for */
      CRCL[c] = (unsigned char)crc;
      CRCH[c] = (unsigned char)(255UL-(crc>>8));
   } /* end for */
} /* end Gencrctab() */


extern void aprsstr_BEGIN(void)
{
   static int aprsstr_init = 0;
   if (aprsstr_init) return;
   aprsstr_init = 1;
   if (sizeof(unsigned char)!=1) X2C_ASSERT(0);
   if (sizeof(aprsstr_GHOSTSET)!=36) X2C_ASSERT(0);
   Gencrctab();
}

