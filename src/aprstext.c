/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef aprstext_H_
#include "aprstext.h"
#endif
#define aprstext_C_
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef maptool_H_
#include "maptool.h"
#endif
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef useri_H_
#include "useri.h"
#endif

/* aprs tracks on osm map by oe5dxl */
/*FROM osi IMPORT WrInt, WrStrLn; */
#define aprstext_PI 3.1415926535898


extern void aprstext_strcp(char from[], uint32_t from_len,
                uint32_t p, uint32_t l, char to[],
                uint32_t to_len)
{
   uint32_t i;
   i = 0UL;
   while (l>0UL && i<=to_len-1) {
      to[i] = from[p];
      ++i;
      ++p;
      --l;
   }
   if (i<=to_len-1) to[i] = 0;
} /* end strcp() */

#define aprstext_DAY 86400


extern void aprstext_DateLocToStr(uint32_t time0, char s[],
                uint32_t s_len)
/* append (+localtime) to time */
{
   uint32_t lo;
   char h[10];
   lo = useri_localtime();
   if (time0+86400UL<aprsdecode_realtime) {
      aprsstr_DateToStr(time0+lo, s, s_len);
   }
   else aprsstr_TimeToStr((time0+lo)%86400UL, s, s_len);
   if (lo) {
      aprsstr_IntToStr((int32_t)lo/3600L, 0UL, h, 10ul);
      aprsstr_Append(s, s_len, "(", 2ul);
      aprsstr_Append(s, s_len, h, 10ul);
      aprsstr_Append(s, s_len, ")", 2ul);
   }
} /* end DateLocToStr() */


static char IsBulletin(const struct aprsdecode_DAT dat)
{
   return ((dat.type==aprsdecode_MSG && dat.msgto[0UL]=='B')
                && dat.msgto[1UL]=='L') && dat.msgto[2UL]=='N';
} /* end IsBulletin() */


extern void aprstext_logfndate(uint32_t time0, char fn[],
                uint32_t fn_len)
/* replace %d by date */
{
   int32_t p;
   char s[16];
   p = aprsstr_InStr(fn, fn_len, "%d", 3ul);
   if (p>=0L && p+2L==(int32_t)aprsstr_Length(fn, fn_len)) {
      aprsstr_DateToStr(time0, s, 16ul);
      fn[p] = s[0U];
      ++p;
      fn[p] = s[1U];
      ++p;
      fn[p] = s[2U];
      ++p;
      fn[p] = s[3U];
      ++p;
      fn[p] = s[5U];
      ++p;
      fn[p] = s[6U];
      ++p;
      fn[p] = s[8U];
      ++p;
      fn[p] = s[9U];
      ++p;
      fn[p] = 0;
   }
} /* end logfndate() */


extern float aprstext_FtoC(float tempf)
{
   return X2C_DIVR(tempf-32.0f,1.8f);
} /* end FtoC() */


extern float aprstext_CtoF(float tempc)
{
   return tempc*1.8f+32.0f;
} /* end CtoF() */


extern char aprstext_isacall(char s[], uint32_t s_len)
{
   uint32_t p;
   uint32_t lit;
   uint32_t num0;
   char c1;
   p = 0UL;
   num0 = 0UL;
   lit = 0UL;
   for (;;) {
      c1 = s[p];
      if ((uint8_t)c1>='0' && (uint8_t)c1<='9') ++num0;
      else if ((uint8_t)c1>='A' && (uint8_t)c1<='Z') ++lit;
      else break;
      ++p;
      if (p>5UL) break;
   }
   if ((lit<2UL || num0==0UL) || num0>2UL) return 0;
   if (s[p]=='-') {
      ++p;
      if (s[p]=='1') {
         ++p;
         if ((uint8_t)s[p]>='0' && (uint8_t)s[p]<='5') ++p;
      }
      else {
         if ((uint8_t)s[p]<'1' || (uint8_t)s[p]>'9') return 0;
         ++p;
      }
   }
   return p>s_len-1 || s[p]==0;
} /* end isacall() */


extern void aprstext_sievert2str(float v, char s[], uint32_t s_len)
{
   if (v<1.E-6f) {
      aprsstr_FixToStr(v*1.E+9f+0.5f, 0UL, s, s_len);
      aprsstr_Append(s, s_len, "n", 2ul);
   }
   else if (v<0.001f) {
      aprsstr_FixToStr(v*1.E+6f+0.005f, 2UL, s, s_len);
      aprsstr_Append(s, s_len, "u", 2ul);
   }
   else {
      aprsstr_FixToStr(v*1000.0f+0.005f, 2UL, s, s_len);
      aprsstr_Append(s, s_len, "m", 2ul);
   }
   aprsstr_Append(s, s_len, "Sv/h", 5ul);
} /* end sievert2str() */


static void Errtxt(char s[], uint32_t s_len, aprsdecode_pFRAMEHIST pf,
                aprsdecode_pFRAMEHIST frame)
{
   char hh[100];
   char h[100];
   uint32_t l;
   uint8_t e;
   if (frame) {
      e = frame->nodraw;
      if ((frame->vardat && frame->vardat->lastref)
                && !aprspos_posvalid(frame->vardat->pos)) e |= 0x40U;
      h[0U] = 0;
      if ((0x1U & e)) aprsstr_Append(h, 100ul, "DIST,", 6ul);
      if ((0x2U & e)) aprsstr_Append(h, 100ul, "SPIKE,", 7ul);
      if ((0x4U & e)) aprsstr_Append(h, 100ul, "SYMBOL,", 8ul);
      if ((0x10U & e)) aprsstr_Append(h, 100ul, "SPEED,", 7ul);
      if ((0x40U & e)) aprsstr_Append(h, 100ul, "NOPOS,", 7ul);
      if ((0x8U & e)) {
         aprsstr_Append(h, 100ul, "DUPE,", 6ul);
         if (pf) {
            aprsstr_IntToStr((int32_t)aprsdecode_finddup(pf, frame), 0UL,
                hh, 100ul);
            aprsstr_Append(h, 100ul, hh, 100ul);
            aprsstr_Append(h, 100ul, "s,", 3ul);
         }
      }
      if ((0x20U & e)) aprsstr_Append(h, 100ul, "SEG,", 5ul);
      l = aprsstr_Length(h, 100ul);
      if (l>0UL) {
         h[l-1UL] = 0;
         aprsstr_Append(h, 100ul, "]\376", 3ul);
         aprsstr_Append(s, s_len, "\370[", 3ul);
         aprsstr_Append(s, s_len, h, 100ul);
      }
   }
} /* end Errtxt() */


static char Hex(uint32_t d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


extern void aprstext_Apphex(char s[], uint32_t s_len, char h[],
                uint32_t h_len)
{
   uint32_t j;
   uint32_t i;
   i = 0UL;
   j = aprsstr_Length(s, s_len);
   while ((i<=h_len-1 && h[i]) && j+10UL<s_len-1) {
      if (h[i]!='\015') {
         if ((uint8_t)h[i]<' ' || (uint8_t)h[i]>='\177') {
            s[j] = '\371';
            ++j;
            s[j] = '<';
            ++j;
            s[j] = Hex((uint32_t)(uint8_t)h[i]/16UL);
            ++j;
            s[j] = Hex((uint32_t)(uint8_t)h[i]);
            ++j;
            s[j] = '>';
            ++j;
            s[j] = '\376';
         }
         else s[j] = h[i];
         ++j;
      }
      ++i;
   }
   s[j] = 0;
} /* end Apphex() */

#define aprstext_TAB "\012 "


static void rfdist(aprsdecode_pVARDAT v, char h[], uint32_t h_len)
{
   aprsdecode_MONCALL digi;
   aprsdecode_pOPHIST ig;
   char s[32];
   struct aprsdecode_VARDAT * anonym;
   { /* with */
      struct aprsdecode_VARDAT * anonym = v;
      if (anonym->igatelen==0U || !aprspos_posvalid(anonym->pos)) return;
      aprstext_strcp(anonym->raw, 500ul, (uint32_t)anonym->igatepos,
                (uint32_t)anonym->igatelen, digi, 9ul);
      ig = aprsdecode_ophist0;
      while (ig && X2C_STRCMP(ig->call,9u,digi,9u)) ig = ig->next;
      if (ig==0 || !aprspos_posvalid(ig->lastpos)) return;
      aprsstr_Append(h, h_len, " Igate:", 8ul);
      aprsstr_Append(h, h_len, digi, 9ul);
      aprsstr_Append(h, h_len, "(", 2ul);
      aprsstr_FixToStr(aprspos_distance(ig->lastpos, anonym->pos), 4UL, s,
                32ul);
      aprsstr_Append(h, h_len, s, 32ul);
      aprsstr_Append(h, h_len, "km)", 4ul);
   }
} /* end rfdist() */


static void objitem(char s[], uint32_t s_len,
                struct aprsdecode_DAT * dat)
{
   if (dat->type==aprsdecode_OBJ || dat->type==aprsdecode_ITEM) {
      aprsstr_Append(s, s_len, "\012 ", 3ul);
      if (dat->objkill=='1') aprsstr_Append(s, s_len, "Killed ", 8ul);
      if (dat->type==aprsdecode_OBJ && (uint8_t)dat->areasymb.typ>='0') {
         if ((uint8_t)dat->areasymb.typ>='5') {
            aprsstr_Append(s, s_len, "filled ", 8ul);
         }
         switch (((uint32_t)(uint8_t)dat->areasymb.typ-48UL)%5UL) {
         case 0UL:
            aprsstr_Append(s, s_len, "Circle", 7ul);
            break;
         case 1UL:
            aprsstr_Append(s, s_len, "Line", 5ul);
            break;
         case 2UL:
            aprsstr_Append(s, s_len, "Ellipse", 8ul);
            break;
         case 3UL:
            aprsstr_Append(s, s_len, "Triangle", 9ul);
            break;
         case 4UL:
            aprsstr_Append(s, s_len, "Box", 4ul);
            break;
         } /* end switch */
         aprsstr_Append(s, s_len, " Area ", 7ul);
      }
      if (dat->multiline.size>0UL) {
         aprsstr_Append(s, s_len, "Multiline ", 11ul);
      }
      if (dat->type==aprsdecode_OBJ) {
         aprsstr_Append(s, s_len, "Object from:", 13ul);
      }
      else aprsstr_Append(s, s_len, "Item from:", 11ul);
      aprstext_Apphex(s, s_len, dat->objectfrom, 9ul);
   }
} /* end objitem() */


extern void aprstext_decode(char s[], uint32_t s_len,
                aprsdecode_pFRAMEHIST pf0, aprsdecode_pFRAMEHIST pf,
                aprsdecode_pVARDAT oldvar, uint32_t odate,
                char decoded, struct aprsdecode_DAT * dat)
{
   char h[512];
   char colalt;
   char nl;
   int32_t ret;
   int32_t og;
   uint32_t tn;
   float resol;
   char tmp;
   if (pf->time0>0UL) {
      s[0UL] = '\367';
      s[1UL] = 0;
      aprstext_DateLocToStr(pf->time0, h, 512ul);
      aprsstr_Append(s, s_len, h, 512ul);
      aprsstr_Append(s, s_len, ":\376", 3ul);
   }
   else s[0UL] = 0;
   ret = aprsdecode_Decode(pf->vardat->raw, 500ul, dat);
   if (decoded) {
      aprstext_Apphex(s, s_len, dat->symcall, 9ul);
      og = X2C_max_longint;
      if (!aprspos_posvalid(dat->pos)) dat->pos = pf->vardat->pos;
      if (aprspos_posvalid(dat->pos)) {
         aprstext_postostr(dat->pos, '3', h, 512ul);
         aprsstr_Append(s, s_len, " \367", 3ul);
         aprsstr_Append(s, s_len, h, 512ul);
         aprsstr_Append(s, s_len, "\376 (", 4ul);
         aprsstr_postoloc(h, 512ul, dat->pos);
         aprsstr_Append(s, s_len, h, 512ul);
         aprsstr_Append(s, s_len, ") ", 3ul);
         og = (int32_t)X2C_TRUNCI(libsrtm_getsrtm(dat->pos, 0UL, &resol),
                X2C_min_longint,X2C_max_longint);
      }
      nl = 1;
      if ((uint8_t)dat->symt>' ' && (uint8_t)dat->sym>' ') {
         aprsstr_Append(s, s_len, "\012 ", 3ul);
         nl = 0;
         aprsstr_Append(s, s_len, "\375", 2ul);
         /*      Append(s, "Sym:"); */
         aprstext_Apphex(s, s_len, (char *) &dat->symt, 1u/1u);
         aprstext_Apphex(s, s_len, (char *) &dat->sym, 1u/1u);
      }
      if (dat->speed<X2C_max_longcard) {
         if (nl) {
            aprsstr_Append(s, s_len, "\012 ", 3ul);
            nl = 0;
         }
         else aprsstr_Append(s, s_len, " \367", 3ul);
         if (dat->sym=='_') {
            aprsstr_FixToStr((float)dat->speed*1.609f, 2UL, h, 512ul);
         }
         else aprsstr_FixToStr((float)dat->speed*1.852f, 0UL, h, 512ul);
         aprsstr_Append(s, s_len, h, 512ul);
         aprsstr_Append(s, s_len, "km/h\376", 6ul);
         if (dat->course>0UL) {
            aprsstr_Append(s, s_len, " dir:", 6ul);
            aprsstr_IntToStr((int32_t)(dat->course%360UL), 1UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "deg", 4ul);
         }
      }
      if (dat->altitude<X2C_max_longint) {
         if (nl) {
            aprsstr_Append(s, s_len, "\012 ", 3ul);
            nl = 0;
         }
         else aprsstr_Append(s, s_len, " ", 2ul);
         colalt = useri_conf2int(useri_fALTMIN, 0UL, -10000L, 100000L,
                -10000L)<=dat->altitude;
         if (colalt) aprsstr_Append(s, s_len, "\367", 2ul);
         aprsstr_Append(s, s_len, "NN:", 4ul);
         aprsstr_IntToStr(dat->altitude, 1UL, h, 512ul);
         aprsstr_Append(s, s_len, h, 512ul);
         aprsstr_Append(s, s_len, "m", 2ul);
         if (colalt) aprsstr_Append(s, s_len, "\376", 2ul);
         if (og<30000L) {
            aprsstr_Append(s, s_len, " OG:", 5ul);
            aprsstr_IntToStr(dat->altitude-og, 1UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "m", 2ul);
         }
      }
      rfdist(pf->vardat, s, s_len);
      if ((oldvar && aprspos_posvalid(oldvar->pos))
                && aprspos_posvalid(dat->pos)) {
         if (nl) {
            aprsstr_Append(s, s_len, "\012 ", 3ul);
            nl = 0;
         }
         else aprsstr_Append(s, s_len, " ", 2ul);
         aprsstr_Append(s, s_len, "moved:", 7ul);
         aprsstr_FixToStr(aprspos_distance(oldvar->pos, dat->pos), 4UL, h,
                512ul);
         aprsstr_Append(s, s_len, h, 512ul);
         aprsstr_Append(s, s_len, "km", 3ul);
      }
      if (odate>0UL && pf->time0>=odate) {
         if (nl) {
            aprsstr_Append(s, s_len, "\012 ", 3ul);
            nl = 0;
         }
         else aprsstr_Append(s, s_len, " ", 2ul);
         aprsstr_Append(s, s_len, "since ", 7ul);
         aprsstr_IntToStr((int32_t)(pf->time0-odate), 1UL, h, 512ul);
         aprsstr_Append(s, s_len, h, 512ul);
         aprsstr_Append(s, s_len, "s", 2ul);
      }
      if (dat->type!=aprsdecode_MSG) Errtxt(s, s_len, pf0, pf);
      nl = 1;
      if (dat->wx.storm==aprsdecode_WXNORMAL) {
         if (dat->type==aprsdecode_OBJ || dat->type==aprsdecode_ITEM) {
            objitem(s, s_len, dat);
         }
         if (dat->wx.gust!=1.E+6f) {
            nl = 0;
            aprsstr_Append(s, s_len, "\012 Gust:", 8ul);
            aprsstr_FixToStr(dat->wx.gust*1.609f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "km/h", 5ul);
         }
         if (dat->wx.temp!=1.E+6f) {
            if (nl) {
               aprsstr_Append(s, s_len, "\012 ", 3ul);
               nl = 0;
            }
            aprsstr_Append(s, s_len, " Temp:", 7ul);
            aprsstr_FixToStr(aprstext_FtoC(dat->wx.temp), 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "C", 2ul);
         }
         if (dat->wx.hygro!=1.E+6f) {
            if (nl) {
               aprsstr_Append(s, s_len, "\012 ", 3ul);
               nl = 0;
            }
            aprsstr_Append(s, s_len, " Hum:", 6ul);
            aprsstr_IntToStr((int32_t)X2C_TRUNCI(dat->wx.hygro+0.5f,
                X2C_min_longint,X2C_max_longint), 1UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "%", 2ul);
         }
         if (dat->wx.baro!=1.E+6f) {
            if (nl) {
               aprsstr_Append(s, s_len, "\012 ", 3ul);
               nl = 0;
            }
            aprsstr_Append(s, s_len, " Baro:", 7ul);
            aprsstr_FixToStr(dat->wx.baro*0.1f+0.05f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "hPa", 4ul);
         }
         if (dat->wx.rain1!=1.E+6f) {
            if (nl) {
               aprsstr_Append(s, s_len, "\012 ", 3ul);
               nl = 0;
            }
            aprsstr_Append(s, s_len, " Rain1h:", 9ul);
            aprsstr_FixToStr(dat->wx.rain1*0.254f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "mm", 3ul);
         }
         if (dat->wx.rain24!=1.E+6f) {
            if (nl) {
               aprsstr_Append(s, s_len, "\012 ", 3ul);
               nl = 0;
            }
            aprsstr_Append(s, s_len, " Rain24h:", 10ul);
            aprsstr_FixToStr(dat->wx.rain24*0.254f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "mm", 3ul);
         }
         if (dat->wx.raintoday!=1.E+6f) {
            if (nl) {
               aprsstr_Append(s, s_len, "\012 ", 3ul);
               nl = 0;
            }
            aprsstr_Append(s, s_len, " Rain00:", 9ul);
            aprsstr_FixToStr(dat->wx.raintoday*0.254f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "mm", 3ul);
         }
         if (dat->wx.lum!=1.E+6f) {
            if (nl) aprsstr_Append(s, s_len, "\012 ", 3ul);
            aprsstr_Append(s, s_len, " Luminosity:", 13ul);
            aprsstr_FixToStr(dat->wx.lum+0.5f, 0UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "W", 2ul);
         }
         if (dat->wx.sievert!=1.E+6f) {
            if (nl) aprsstr_Append(s, s_len, "\012 ", 3ul);
            aprsstr_Append(s, s_len, " Radiation:", 12ul);
            aprstext_sievert2str(dat->wx.sievert, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
         }
      }
      else if (dat->wx.storm>aprsdecode_WXNORMAL) {
         if (dat->type==aprsdecode_OBJ || dat->type==aprsdecode_ITEM) {
            objitem(s, s_len, dat);
         }
         nl = 0;
         aprsstr_Append(s, s_len, "\012 ", 3ul);
         if (dat->wx.storm==aprsdecode_WXTS) {
            aprsstr_Append(s, s_len, "Tropical Storm", 15ul);
         }
         else if (dat->wx.storm==aprsdecode_WXHC) {
            aprsstr_Append(s, s_len, "Hurricane", 10ul);
         }
         else if (dat->wx.storm==aprsdecode_WXTD) {
            aprsstr_Append(s, s_len, "Tropical Depression", 20ul);
         }
         nl = 0;
         if (dat->wx.gust!=1.E+6f) {
            aprsstr_Append(s, s_len, " Gust:", 7ul);
            aprsstr_FixToStr(dat->wx.gust*1.609f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "km/h", 5ul);
         }
         if (dat->wx.sustaind!=0.0f) {
            aprsstr_Append(s, s_len, " Sustaind Speed:", 17ul);
            aprsstr_FixToStr(dat->wx.sustaind*1.609f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "km/h", 5ul);
         }
         if (dat->wx.baro!=1.E+6f) {
            aprsstr_Append(s, s_len, " Baro:", 7ul);
            aprsstr_FixToStr(dat->wx.baro, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "hPa", 4ul);
         }
         if (dat->wx.radiushurr!=0.0f) {
            aprsstr_Append(s, s_len, " Radius Hurricane Winds:", 25ul);
            aprsstr_FixToStr(dat->wx.radiushurr*1.609f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "km", 3ul);
         }
         if (dat->wx.radiusstorm!=0.0f) {
            aprsstr_Append(s, s_len, " Storm Winds:", 14ul);
            aprsstr_FixToStr(dat->wx.radiusstorm*1.609f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "km", 3ul);
         }
         if (dat->wx.wholegale!=0.0f) {
            aprsstr_Append(s, s_len, " Whole gale:", 13ul);
            aprsstr_FixToStr(dat->wx.wholegale*1.609f, 2UL, h, 512ul);
            aprsstr_Append(s, s_len, h, 512ul);
            aprsstr_Append(s, s_len, "km", 3ul);
         }
      }
      else if (dat->type==aprsdecode_MSG) {
         aprsstr_Append(s, s_len, "\012  Msg To:", 11ul);
         aprstext_Apphex(s, s_len, dat->msgto, 9ul);
         if (dat->msgtext[0UL]) {
            aprsstr_Append(s, s_len, " Text:[", 8ul);
            aprstext_Apphex(s, s_len, dat->msgtext, 67ul);
            aprsstr_Append(s, s_len, "]", 2ul);
         }
         if (dat->acktext[0UL]) {
            aprsstr_Append(s, s_len, " Ack:[", 7ul);
            aprstext_Apphex(s, s_len, dat->acktext, 5ul);
            aprsstr_Append(s, s_len, "]", 2ul);
         }
         if (dat->ackrej==aprsdecode_MSGREJ) {
            aprsstr_Append(s, s_len, " Reject", 8ul);
         }
      }
      else if (dat->type==aprsdecode_OBJ || dat->type==aprsdecode_ITEM) {
         objitem(s, s_len, dat);
      }
      if (dat->type!=aprsdecode_MSG && dat->comment0[0UL]) {
         if (dat->type==aprsdecode_TELE) {
            aprsstr_Append(s, s_len, "\012 Telemetry: [", 15ul);
         }
         else aprsstr_Append(s, s_len, "\012 Comment: [", 13ul);
         aprstext_Apphex(s, s_len, dat->comment0, 256ul);
         aprsstr_Append(s, s_len, "]", 2ul);
      }
      if (dat->tlmvalues[0UL]) {
         aprsstr_Append(s, s_len, "\012  Mic-e Telemetry Seq:", 24ul);
         ret = 0L;
         for (;;) {
            if (dat->tlmvalues[ret]) {
               if (ret!=6L) {
                  aprsstr_IntToStr((int32_t)(dat->tlmvalues[ret]-1U), 1UL,
                h, 512ul);
                  aprsstr_Append(s, s_len, h, 512ul);
               }
               else {
                  tn = (uint32_t)(dat->tlmvalues[ret]-1U);
                  if (tn>=256UL) tn = (tn&8191UL)+8192UL;
                  else tn = (tn&255UL)+256UL;
                  while (tn>1UL) {
                     aprsstr_Append(s, s_len,
                (char *)(tmp = (char)((uint32_t)(char)(tn&1)
                +48UL),&tmp), 1u/1u);
                     tn = tn/2UL;
                  }
               }
            }
            ++ret;
            if (ret>6L) break;
            aprsstr_Append(s, s_len, ",", 2ul);
         }
      }
      aprsstr_Append(s, s_len, "\012 ", 3ul);
   }
   aprsstr_Append(s, s_len, "[", 2ul);
   aprstext_Apphex(s, s_len, pf->vardat->raw, 500ul);
   aprsstr_Append(s, s_len, "]", 2ul);
   if (!decoded) Errtxt(s, s_len, pf0, pf);
} /* end decode() */


extern void aprstext_decodelistline(char s[], uint32_t s_len,
                char text[], uint32_t text_len, uint32_t time0)
{
   uint32_t i;
   struct aprsdecode_DAT dat;
   struct aprsdecode_FRAMEHIST f;
   struct aprsdecode_VARDAT vardat;
   X2C_PCOPY((void **)&text,text_len);
   i = 0UL;
   while ((i<=text_len-1 && text[i]) && text[i]!='[') ++i;
   if (i>text_len-1 || text[i]==0) i = 0UL;
   aprsstr_Delstr(text, text_len, 0UL, i+1UL); /* remove port/time[ */
   i = aprsstr_Length(text, text_len);
   if (i>1UL) {
      text[i-1UL] = 0; /* remove ] */
      memset((char *) &vardat,(char)0,
                sizeof(struct aprsdecode_VARDAT));
      aprsstr_Assign(vardat.raw, 500ul, text, text_len);
      memset((char *) &f,(char)0,
                sizeof(struct aprsdecode_FRAMEHIST));
      f.vardat = &vardat;
      f.time0 = time0;
      aprstext_decode(s, s_len, 0, &f, 0, 0UL, 1, &dat);
   }
   X2C_PFREE(text);
} /* end decodelistline() */


extern void aprstext_setmark1(struct aprsstr_POSITION pos,
                char overwrite, int32_t alt, uint32_t timestamp)
{
   if ((overwrite || !aprspos_posvalid(aprsdecode_click.markpos))
                || aprsdecode_click.marktime) {
      aprsdecode_click.markpos = pos;
      if (overwrite) aprsdecode_click.marktime = 0UL;
      else aprsdecode_click.marktime = aprsdecode_realtime;
      aprsdecode_click.markalti = alt;
      aprsdecode_click.markpost = timestamp;
   }
} /* end setmark1() */


extern void aprstext_setmarkalti(aprsdecode_pFRAMEHIST pf,
                aprsdecode_pOPHIST op, char overwrite)
{
   struct aprsdecode_DAT dat;
   int32_t alt;
   struct aprsstr_POSITION pos;
   uint32_t t;
   aprsstr_posinval(&pos);
   alt = X2C_max_longint;
   t = aprsdecode_realtime;
   if (pf && aprsdecode_Decode(pf->vardat->raw, 500ul, &dat)==0L) {
      if (dat.altitude>-10000L) alt = dat.altitude;
      pos = dat.pos;
      t = pf->time0;
   }
   else if (op && op->lastinftyp<100U) {
      alt = (int32_t)op->lasttempalt+22768L;
   }
   if (!aprspos_posvalid(pos) && op) pos = op->lastpos;
   aprstext_setmark1(pos, overwrite, alt, t);
} /* end setmarkalti() */


extern void aprstext_optext(uint32_t typ,
                struct aprsdecode_CLICKOBJECT * obj, char * last,
                char s[], uint32_t s_len)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pfe;
   aprsdecode_pFRAMEHIST pf1;
   aprsdecode_pFRAMEHIST pf;
   uint32_t cn;
   uint32_t cx;
   char ss[1000];
   struct aprsdecode_DAT dat;
   /*    islast:BOOLEAN; */
   op = obj->opf;
   pf = obj->pff0;
   if (pf==0 && op) pf = op->frames;
   s[0UL] = 0;
   /*  islast:=last; */
   *last = 0;
   if (op && pf) {
      if (typ==2UL) {
         /* find last raw frame */
         pf = op->frames;
         if (pf) {
            while (pf->next) pf = pf->next;
            obj->pff0 = pf;
         }
      }
      else if (typ==1UL) {
         /* next frame */
         pf1 = pf;
         do {
            if (pf->next==0) pf = op->frames;
            else pf = pf->next;
         } while (!((!aprsdecode_lums.errorstep || pf1==pf) || (pf->nodraw&~0x40U)!=0U));
         /*      IF pf^.next<>NIL THEN pf:=pf^.next ELSE pf:=op^.frames END;
                */
         if (pf) {
            obj->pff0 = pf;
            obj->pff = pf;
            obj->typf = aprsdecode_tTRACK; /* set "track found" */
         }
      }
      else if (typ==0UL) {
         /* back to last frame */
         pf1 = pf;
         pfe = pf;
         do {
            if (!aprsdecode_lums.errorstep || (pfe->nodraw&~0x40U)!=0U) {
               pf = pfe;
            }
            if (pfe->next==0) pfe = op->frames;
            else pfe = pfe->next;
         } while (pfe!=pf1);
         obj->pff0 = pf;
         obj->pff = pf;
         obj->typf = aprsdecode_tTRACK; /* set "track found" */
      }
      /*    IF ((typ=1) OR (typ=0)) & lums.errorstep & (pf=pf1)
                THEN Assign(s, "no more errors found"); */
      if ((aprsdecode_lums.errorstep && pf) && (pf->nodraw&~0x40U)==0U) {
         aprsstr_Assign(s, s_len, "Show errors mode: no (more) errors found",
                 41ul);
      }
      else if (pf) {
         cn = 0UL;
         cx = 0UL;
         pfe = 0;
         pf1 = op->frames;
         while (pf1) {
            ++cn;
            if (pf1==pf) cx = cn;
            if (pf1->next==pf) pfe = pf1;
            pf1 = pf1->next;
         }
         aprsstr_IntToStr((int32_t)cx, 0UL, s, s_len);
         aprsstr_Append(s, s_len, "/", 2ul);
         aprsstr_IntToStr((int32_t)cn, 0UL, ss, 1000ul);
         aprsstr_Append(s, s_len, ss, 1000ul);
         aprsstr_Append(s, s_len, " ", 2ul);
         if (pfe==0) {
            aprstext_decode(ss, 1000ul, op->frames, pf, 0, 0UL, 1, &dat);
         }
         else {
            aprstext_decode(ss, 1000ul, op->frames, pf, pfe->vardat,
                pfe->time0, 1, &dat);
         }
         aprsstr_Append(s, s_len, ss, 1000ul);
         *last = pf->next==0;
      }
   }
} /* end optext() */


extern aprsdecode_pOPHIST aprstext_oppo(aprsdecode_MONCALL opcall)
/* find pointer to call */
{
   aprsdecode_pOPHIST op;
   if (opcall[0UL]==0) return 0;
   op = aprsdecode_ophist0;
   while (op && X2C_STRCMP(op->call,9u,opcall,9u)) op = op->next;
   return op;
} /* end oppo() */


extern void aprstext_listop(char decoded)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pf;
   aprsdecode_pVARDAT oldv;
   uint32_t oldt;
   char s[1000];
   struct aprsdecode_DAT dat;
   op = aprsdecode_click.table[aprsdecode_click.selected].opf;
   if (op) {
      pf = op->frames;
      oldv = 0;
      oldt = 0UL;
      while (pf) {
         aprstext_decode(s, 1000ul, op->frames, pf, oldv, oldt, decoded,
                &dat);
         if (aprspos_posvalid(pf->vardat->pos)) oldv = pf->vardat;
         oldt = pf->time0;
         aprsstr_Append(s, 1000ul, "\012", 2ul);
         if (decoded) aprsstr_Append(s, 1000ul, "\012", 2ul);
         useri_wrstrlist(s, 1000ul, dat.symcall, pf->vardat->pos, pf->time0);
         pf = pf->next;
      }
   }
} /* end listop() */


extern void aprstext_listin(char r[], uint32_t r_len, char port,
                char dir, char decoded, int32_t quali,
                int32_t txd, int32_t level)
{
   char s2[1000];
   char s1[1000];
   char s[1000];
   struct aprsdecode_VARDAT vard;
   struct aprsdecode_FRAMEHIST pf;
   uint32_t j;
   uint32_t i;
   struct aprsdecode_DAT dat;
   memset((char *) &vard,(char)0,sizeof(struct aprsdecode_VARDAT));
   i = 0UL;
   j = 0UL;
   while ((i<499UL && i<=r_len-1) && r[i]) {
      if (r[i]!='\015' && r[i]!='\012') j = i;
      vard.raw[i] = r[i];
      ++i;
   }
   vard.raw[j+1UL] = 0;
   memset((char *) &pf,(char)0,sizeof(struct aprsdecode_FRAMEHIST));
   pf.vardat = &vard;
   aprstext_decode(s, 1000ul, 0, &pf, 0, 0UL, decoded, &dat);
   if (decoded) {
      s1[0U] = '\012';
      s1[1U] = port;
      s1[2U] = 0;
   }
   else {
      s1[0U] = port;
      s1[1U] = 0;
   }
   if (dir=='<') aprsstr_Append(s1, 1000ul, "\370<\376", 4ul);
   else aprsstr_Append(s1, 1000ul, (char *) &dir, 1u/1u);
   if (txd>0L || quali>0L) {
      aprsstr_Append(s1, 1000ul, "(", 2ul);
      if (txd>0L) aprsstr_IntToStr(txd, 3UL, s2, 1000ul);
      else strncpy(s2,"   ",1000u);
      aprsstr_Append(s1, 1000ul, s2, 1000ul);
      aprsstr_Append(s1, 1000ul, "/", 2ul);
      if (level) aprsstr_IntToStr(level, 3UL, s2, 1000ul);
      else strncpy(s2,"   ",1000u);
      aprsstr_Append(s1, 1000ul, s2, 1000ul);
      aprsstr_Append(s1, 1000ul, "/", 2ul);
      if (quali>0L) aprsstr_IntToStr(quali, 2UL, s2, 1000ul);
      else strncpy(s2,"  ",1000u);
      aprsstr_Append(s1, 1000ul, s2, 1000ul);
      aprsstr_Append(s1, 1000ul, ")", 2ul);
   }
   aprsstr_Append(s1, 1000ul, s, 1000ul);
   useri_wrstrmon(s1, 1000ul, dat.pos);
} /* end listin() */

#define aprstext_TRIVIAL 3.1397174254317E-6
/* 20m */


extern void aprstext_listtyps(char typ, char decod,
                char oneop[], uint32_t oneop_len)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pf;
   char s[1000];
   char lasttext[101];
   char lastto[101];
   struct aprsdecode_DAT dat;
   X2C_PCOPY((void **)&oneop,oneop_len);
   op = aprsdecode_ophist0;
   while (op) {
      pf = op->frames;
      if (typ=='N') {
         /* no pos */
         if (pf && !aprspos_posvalid(op->lastpos)) {
            while (pf->next) pf = pf->next;
            aprstext_decode(s, 1000ul, 0, pf, 0, 0UL, decod, &dat);
            if (decod) aprsstr_Append(s, 1000ul, "\012\012", 3ul);
            else aprsstr_Append(s, 1000ul, "\012", 2ul);
            useri_wrstrlist(s, 1000ul, dat.symcall, pf->vardat->pos,
                pf->time0);
         }
      }
      else if (typ=='D') {
         /* moveing stations */
         if (pf && (op->margin0.lat>op->margin1.lat+3.1397174254317E-6f || op->margin0.long0+3.1397174254317E-6f<op->margin1.long0)
                ) {
            while (pf->next) pf = pf->next;
            aprstext_decode(s, 1000ul, 0, pf, 0, 0UL, decod, &dat);
            if (decod) aprsstr_Append(s, 1000ul, "\012\012", 3ul);
            else aprsstr_Append(s, 1000ul, "\012", 2ul);
            useri_wrstrlist(s, 1000ul, dat.symcall, pf->vardat->pos,
                pf->time0);
         }
      }
      else if (typ=='W') {
         if (pf) {
            while (pf->next) pf = pf->next;
            aprstext_decode(s, 1000ul, 0, pf, 0, 0UL, decod, &dat);
            /*        IF (dat.symt="/") & (dat.sym="_") */
            /*        OR (op^.temptime+SHOWTEMPWIND>systime)
                & (op^.lastinftyp>=100) */
            /*        & (op^.lasttempalt>=-99) & (op^.lasttempalt<=99)
                THEN */
            if (dat.symt=='/' && dat.sym=='_') {
               if (decod) aprsstr_Append(s, 1000ul, "\012\012", 3ul);
               else aprsstr_Append(s, 1000ul, "\012", 2ul);
               useri_wrstrlist(s, 1000ul, dat.symcall, pf->vardat->pos,
                pf->time0);
            }
         }
      }
      else if (typ=='M' || typ=='B' && (oneop[0UL]==0 || X2C_STRCMP(oneop,
                oneop_len,op->call,9u)==0)) {
         /* bulletins messages */
         lastto[0U] = 0;
         lasttext[0U] = 0;
         while (pf) {
            if (((((((pf->vardat->lastref==pf && aprsdecode_Decode(pf->vardat->raw,
                 500ul, &dat)>=0L) && (typ=='M' || typ=='B')) && dat.type==aprsdecode_MSG) && X2C_STRCMP(dat.symcall,
                9u,dat.msgto,9u)) && dat.msgtext[0UL]) && (typ=='B')==IsBulletin(dat)) && ((oneop[0UL]==0 || X2C_STRCMP(oneop,
                oneop_len,op->call,9u)==0) || X2C_STRCMP(oneop,oneop_len,
                dat.msgto,9u)==0)) {
               if (!(aprsstr_StrCmp(lastto, 101ul, dat.msgto,
                9ul) && aprsstr_StrCmp(lasttext, 101ul, dat.msgtext, 67ul))) {
                  aprstext_DateLocToStr(pf->time0, s, 1000ul);
                  aprsstr_Append(s, 1000ul, " ", 2ul);
                  aprsstr_Append(s, 1000ul, dat.symcall, 9ul);
                  aprsstr_Append(s, 1000ul, ">", 2ul);
                  aprsstr_Append(s, 1000ul, dat.msgto, 9ul);
                  aprsstr_Append(s, 1000ul, ":[", 3ul);
                  aprstext_Apphex(s, 1000ul, dat.msgtext, 67ul);
                  aprsstr_Append(s, 1000ul, "]", 2ul);
                  if (dat.acktext[0UL]) {
                     aprstext_Apphex(s, 1000ul, " Ack[", 6ul);
                     aprsstr_Append(s, 1000ul, dat.acktext, 5ul);
                     aprsstr_Append(s, 1000ul, "]", 2ul);
                  }
                  aprsstr_Append(s, 1000ul, "\012", 2ul);
                  useri_wrstrlist(s, 1000ul, dat.symcall, pf->vardat->pos,
                pf->time0);
                  aprsstr_Assign(lastto, 101ul, dat.msgto, 9ul);
                  aprsstr_Assign(lasttext, 101ul, dat.msgtext, 67ul);
               }
            }
            pf = pf->next;
         }
      }
      else if (typ=='O') {
         /*
             ELSIF ((typ="M") OR (typ="B")) & ((oneop[0]=0C)
                OR (oneop=op^.call)) THEN (* bulletins messages *)
               lastto[0]:=0C;
               lasttext[0]:=0C;
         
               WHILE pf<>NIL DO
                 IF (pf^.vardat^.lastref=pf) & (Decode(pf^.vardat^.raw,
                dat)>=0) THEN
                   IF (typ="M") OR (typ="B") THEN
                     IF (dat.type=MSG) & (dat.symcall<>dat.msgto)
                & (dat.msgtext[0]<>0C) THEN
                       IF (typ="B")=IsBulletin(dat) THEN 
                         IF NOT (StrCmp(lastto, dat.msgto) & StrCmp(lasttext,
                 dat.msgtext)) THEN 
                           DateLocToStr(pf^.time, s); Append(s, " ");
                           Append(s, dat.symcall); Append(s, ">");
                           Append(s, dat.msgto); Append(s, ":[");
                           Apphex(s, dat.msgtext); Append(s, "]");
                           IF dat.acktext[0]<>0C THEN 
                             Apphex(s, " Ack[");Append(s, dat.acktext);
                Append(s, "]");
                           END;
                           Append(s, LF);
                           wrstrlist(s, dat.symcall, pf^.vardat^.pos,
                pf^.time);
         
         <* IF WITHSTDOUT THEN *>
                           WrStr(s);
         <* END *>
                           Assign(lastto, dat.msgto);
                           Assign(lasttext, dat.msgtext);
                         END;
                       END;
                     END;
                   END;  
                 END;
                 pf:=pf^.next;
               END;
         */
         if (pf) {
            while (pf->next) pf = pf->next;
            aprstext_decode(s, 1000ul, 0, pf, 0, 0UL, decod, &dat);
            if ((dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM)
                && (oneop[0UL]==0 || X2C_STRCMP(oneop,oneop_len,
                dat.objectfrom,9u)==0)) {
               if (decod) {
                  aprsstr_Append(s, 1000ul, "\012\012", 3ul);
                  useri_wrstrlist(s, 1000ul, dat.symcall, pf->vardat->pos,
                pf->time0);
               }
               else {
                  aprsstr_Append(s, 1000ul, "\012", 2ul);
                  useri_wrstrlist(s, 1000ul, dat.objectfrom, pf->vardat->pos,
                 pf->time0);
               }
            }
         }
      }
      op = op->next;
   }
   X2C_PFREE(oneop);
} /* end listtyps() */

#define aprstext_Z 48


static void degtostr(float d, char lat, char form,
                char s[], uint32_t s_len)
{
   uint32_t i;
   uint32_t n;
   if (s_len-1<11UL) {
      s[0UL] = 0;
      return;
   }
   if (form=='2') i = 7UL;
   else if (form=='3') i = 8UL;
   else i = 9UL;
   if (d<0.0f) {
      d = -d;
      if (lat) s[i] = 'S';
      else s[i+1UL] = 'W';
   }
   else if (lat) s[i] = 'N';
   else s[i+1UL] = 'E';
   i = (uint32_t)!lat;
   if (form=='2') {
      /* DDMM.MMNDDMM.MME */
      /*    n:=trunc(d*(6000*180/PI)+0.5); */
      n = aprsdecode_trunc(d*3.4377467707849E+5f);
      s[0UL] = (char)((n/600000UL)%10UL+48UL);
      s[i] = (char)((n/60000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/6000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/1000UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/100UL)%10UL+48UL);
      ++i;
      s[i] = '.';
      ++i;
      s[i] = (char)((n/10UL)%10UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
   }
   else if (form=='3') {
      /* DDMM.MMMNDDMM.MMME */
      n = aprsdecode_trunc(d*3.4377467707849E+6f+0.5f);
      s[0UL] = (char)((n/6000000UL)%10UL+48UL);
      s[i] = (char)((n/600000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/60000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/10000UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/1000UL)%10UL+48UL);
      ++i;
      s[i] = '.';
      ++i;
      s[i] = (char)((n/100UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/10UL)%10UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
   }
   else {
      /* DDMMSS */
      n = aprsdecode_trunc(d*2.062648062471E+5f+0.5f);
      s[0UL] = (char)((n/360000UL)%10UL+48UL);
      s[i] = (char)((n/36000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/3600UL)%10UL+48UL);
      ++i;
      s[i] = '\177';
      ++i;
      s[i] = (char)((n/600UL)%6UL+48UL);
      ++i;
      s[i] = (char)((n/60UL)%10UL+48UL);
      ++i;
      s[i] = '\'';
      ++i;
      s[i] = (char)((n/10UL)%6UL+48UL);
      ++i;
      s[i] = (char)(n%10UL+48UL);
      ++i;
      s[i] = '\"';
      ++i;
   }
   ++i;
   s[i] = 0;
} /* end degtostr() */


extern void aprstext_postostr(struct aprsstr_POSITION pos, char form,
                char s[], uint32_t s_len)
{
   char h[32];
   degtostr(pos.lat, 1, form, s, s_len);
   aprsstr_Append(s, s_len, "/", 2ul);
   degtostr(pos.long0, 0, form, h, 32ul);
   aprsstr_Append(s, s_len, h, 32ul);
} /* end postostr() */


static int32_t myround(float x)
{
   if (x>=0.0f) x = x+0.5f;
   else x = x-0.5f;
   return (int32_t)X2C_TRUNCI(x,X2C_min_longint,X2C_max_longint);
} /* end myround() */


extern void aprstext_measure(struct aprsstr_POSITION pos0,
                struct aprsstr_POSITION pos1, char s[], uint32_t s_len,
                 char sum)
{
   char h[32];
   if (aprspos_posvalid(pos0) && aprspos_posvalid(pos1)) {
      aprstext_postostr(pos1, '3', s, s_len);
      aprsstr_Append(s, s_len, " [", 3ul);
      aprsstr_postoloc(h, 32ul, pos1);
      aprsstr_Append(s, s_len, h, 32ul);
      aprsstr_Append(s, s_len, "] ", 3ul);
      aprsstr_FixToStr(aprspos_distance(pos0, pos1), 4UL, h, 32ul);
      aprsstr_Append(s, s_len, h, 32ul);
      aprsstr_Append(s, s_len, "km,", 4ul);
      aprsstr_IntToStr(myround(aprspos_azimuth(pos0, pos1)), 1UL, h, 32ul);
      aprsstr_Append(s, s_len, h, 32ul);
      aprsstr_Append(s, s_len, "deg to Marker", 14ul);
      if (sum) {
         if (aprsdecode_click.waysum==0.0f) aprsdecode_click.sumpos = pos0;
         aprsdecode_click.waysum = aprsdecode_click.waysum+aprspos_distance(aprsdecode_click.sumpos,
                 pos1);
         aprsdecode_click.sumpos = pos1;
         aprsstr_Append(s, s_len, " sum:", 6ul);
         aprsstr_FixToStr(aprsdecode_click.waysum, 4UL, h, 32ul);
         aprsstr_Append(s, s_len, h, 32ul);
         aprsstr_Append(s, s_len, "km", 3ul);
      }
   }
   else s[0UL] = 0;
} /* end measure() */


static uint32_t c(char * err, char ch)
{
   uint32_t n;
   if ((uint8_t)ch>='0') n = (uint32_t)(uint8_t)ch-48UL;
   else {
      n = 0UL;
      *err = 1;
   }
   if (n>9UL) *err = 1;
   return n;
} /* end c() */


extern void aprstext_degtopos(char s[], uint32_t s_len,
                struct aprsstr_POSITION * pos)
/* DDMM.MMNDDDMM.MME */
{
   char err;
   uint32_t d;
   X2C_PCOPY((void **)&s,s_len);
   err = 0;
   d = c(&err, s[0UL])*60000UL+c(&err, s[1UL])*6000UL+c(&err,
                s[2UL])*1000UL+c(&err, s[3UL])*100UL+c(&err,
                s[5UL])*10UL+c(&err, s[6UL]);
   pos->lat = (float)d*2.9088820866572E-6f;
   if (pos->lat>=1.5707963267949f) err = 1;
   if (X2C_CAP(s[7UL])=='S') pos->lat = -pos->lat;
   else if (X2C_CAP(s[7UL])!='N') err = 1;
   d = c(&err, s[9UL])*600000UL+c(&err, s[10UL])*60000UL+c(&err,
                s[11UL])*6000UL+c(&err, s[12UL])*1000UL+c(&err,
                s[13UL])*100UL+c(&err, s[15UL])*10UL+c(&err, s[16UL]);
   pos->long0 = (float)d*2.9088820866572E-6f;
   if (pos->long0>=3.1415926535898f) err = 1;
   if (X2C_CAP(s[17UL])=='W') pos->long0 = -pos->long0;
   else if (X2C_CAP(s[17UL])!='E') err = 1;
   if (((uint8_t)s[2UL]>='6' || (uint8_t)s[12UL]>='6') || err) {
      aprsstr_posinval(pos);
   }
   X2C_PFREE(s);
} /* end degtopos() */


static char c0(uint32_t * d, uint32_t * i, char s[],
                uint32_t s_len, uint32_t mul)
{
   uint32_t n;
   char ch;
   ch = s[*i];
   if (mul>0UL) {
      if ((uint8_t)ch>='0') n = (uint32_t)(uint8_t)ch-48UL;
      else return 0;
      if (n>9UL) return 0;
      *d += n*mul;
   }
   else if (ch!='.') return 0;
   ++*i;
   return 1;
} /* end c() */


extern void aprstext_deghtopos(char s[], uint32_t s_len,
                struct aprsstr_POSITION * pos)
/* DDMM.MMMNDDDMM.MMME */
{
   char e;
   char err;
   uint32_t i;
   uint32_t d;
   X2C_PCOPY((void **)&s,s_len);
   err = 0;
   d = 0UL;
   i = 0UL;
   e = ((((((c0(&d, &i, s, s_len, 600000UL) && c0(&d, &i, s, s_len,
                60000UL)) && c0(&d, &i, s, s_len, 10000UL)) && c0(&d, &i, s,
                s_len, 1000UL)) && c0(&d, &i, s, s_len, 0UL)) && c0(&d, &i,
                s, s_len, 100UL)) && c0(&d, &i, s, s_len, 10UL)) && c0(&d,
                &i, s, s_len, 1UL);
   pos->lat = (float)d*2.9088820866572E-7f;
   if (pos->lat>=1.5707963267949f) err = 1;
   if (s[i]=='S') pos->lat = -pos->lat;
   else if (s[i]!='N') err = 1;
   i += 2UL;
   d = 0UL;
   e = (((((((c0(&d, &i, s, s_len, 6000000UL) && c0(&d, &i, s, s_len,
                600000UL)) && c0(&d, &i, s, s_len, 60000UL)) && c0(&d, &i, s,
                 s_len, 10000UL)) && c0(&d, &i, s, s_len, 1000UL)) && c0(&d,
                &i, s, s_len, 0UL)) && c0(&d, &i, s, s_len, 100UL)) && c0(&d,
                 &i, s, s_len, 10UL)) && c0(&d, &i, s, s_len, 1UL);
   pos->long0 = (float)d*2.9088820866572E-7f;
   if (pos->long0>=3.1415926535898f) err = 1;
   if (s[i]=='W') pos->long0 = -pos->long0;
   else if (s[i]!='E') err = 1;
   if (((uint8_t)s[2UL]>='6' || (uint8_t)s[13UL]>='6') || err) {
      aprsstr_posinval(pos);
   }
   X2C_PFREE(s);
} /* end deghtopos() */


static void cleanposstr(char s[], uint32_t s_len)
{
   uint32_t i;
   i = 0UL;
   while (i<=s_len-1 && s[i]) {
      if ((s[i]==',' || s[i]=='/') || s[i]=='\\') s[i] = ' ';
      ++i;
   }
} /* end cleanposstr() */


extern void aprstext_degdeztopos(char s[], uint32_t s_len,
                struct aprsstr_POSITION * pos)
/* lat long in float deg */
{
   float d;
   char h[31];
   X2C_PCOPY((void **)&s,s_len);
   cleanposstr(s, s_len);
   aprsstr_Extractword(s, s_len, h, 31ul);
   if (aprsstr_StrToFix(&d, h, 31ul)) {
      pos->lat = d*1.7453292519943E-2f;
      aprsstr_Extractword(s, s_len, h, 31ul);
      if (aprsstr_StrToFix(&d, h, 31ul)) {
         pos->long0 = d*1.7453292519943E-2f;
         if ((float)fabs(pos->long0)<3.1415926535898f && (float)
                fabs(pos->lat)<=1.484f) goto label;
      }
   }
   aprsstr_posinval(pos);
   label:;
   X2C_PFREE(s);
} /* end degdeztopos() */


extern void aprstext_deganytopos(char s[], uint32_t s_len,
                struct aprsstr_POSITION * pos)
/* lat long any format in float deg */
{
   X2C_PCOPY((void **)&s,s_len);
   aprstext_degtopos(s, s_len, pos); /* DDMM.MMNDDDMM.MME */
   if (!aprspos_posvalid(*pos)) aprstext_deghtopos(s, s_len, pos);
   if (!aprspos_posvalid(*pos)) aprstext_degdeztopos(s, s_len, pos);
   X2C_PFREE(s);
} /* end deganytopos() */


extern char aprstext_getmypos(struct aprsstr_POSITION * pos)
{
   char s[51];
   useri_confstr(useri_fMYPOS, s, 51ul);
   aprstext_deganytopos(s, 51ul, pos);
   return aprspos_posvalid(*pos);
} /* end getmypos() */


static char num(int32_t n)
{
   return (char)(labs(n)%10L+48L);
} /* end num() */


static uint32_t dao91(uint32_t x)
/* 33 + radix91(xx/1.1) of dddmm.mmxx */
{
   return 33UL+(x*20UL+11UL)/22UL;
} /* end dao91() */


static void daostr(int32_t latd, int32_t longd, char s[],
                uint32_t s_len)
{
   s[0UL] = '!';
   s[1UL] = 'w';
   s[2UL] = (char)dao91((uint32_t)latd);
   s[3UL] = (char)dao91((uint32_t)longd);
   s[4UL] = '!';
   s[5UL] = 0;
} /* end daostr() */


static void micedest(int32_t lat, int32_t long0, char s[],
                uint32_t s_len)
{
   uint32_t nl;
   uint32_t nb;
   nl = (uint32_t)(labs(long0)/6000L);
   nb = (uint32_t)(labs(lat)/6000L);
   s[0UL] = (char)(80UL+nb/10UL);
   s[1UL] = (char)(80UL+nb%10UL);
   nb = (uint32_t)labs(lat)-nb*6000UL;
   s[2UL] = (char)(80UL+nb/1000UL);
   s[3UL] = (char)(48UL+32UL*(uint32_t)(lat>=0L)+(nb/100UL)%10UL);
   s[4UL] = (char)(48UL+32UL*(uint32_t)(nl<10UL || nl>=100UL)+(nb/10UL)
                %10UL);
   s[5UL] = (char)(48UL+32UL*(uint32_t)(long0<0L)+nb%10UL);
   s[6UL] = 0;
} /* end micedest() */


static void micedata(int32_t lat, int32_t long0, uint32_t knots,
                uint32_t dir, int32_t alt, char sym[],
                uint32_t sym_len, char s[], uint32_t s_len)
{
   uint32_t n;
   uint32_t nl;
   X2C_PCOPY((void **)&sym,sym_len);
   dir = dir%360UL;
   /*IF dir>0 THEN DEC(dir) END; */
   /*IF dir>359 THEN dir:=359 END; */
   if (knots>799UL) knots = 0UL;
   nl = (uint32_t)(labs(long0)/6000L);
   if (nl<10UL) s[0UL] = (char)(nl+118UL);
   else if (nl>=100UL) {
      if (nl<110UL) s[0UL] = (char)(nl+8UL);
      else s[0UL] = (char)(nl-72UL);
   }
   else s[0UL] = (char)(nl+28UL);
   nl = (uint32_t)labs(long0)-nl*6000UL; /* long min*100 */
   n = nl/100UL;
   if (n<10UL) n += 60UL;
   s[1UL] = (char)(n+28UL);
   s[2UL] = (char)(nl%100UL+28UL);
   s[3UL] = (char)(knots/10UL+28UL);
   s[4UL] = (char)(32UL+(knots%10UL)*10UL+dir/100UL);
   s[5UL] = (char)(28UL+dir%100UL);
   s[6UL] = sym[1UL];
   s[7UL] = sym[0UL];
   if (alt>-10000L) {
      n = (uint32_t)(alt+10000L);
      s[8UL] = (char)(33UL+(n/8281UL)%91UL);
      s[9UL] = (char)(33UL+(n/91UL)%91UL);
      s[10UL] = (char)(33UL+n%91UL);
      s[11UL] = '}';
      s[12UL] = 0;
   }
   else s[8UL] = 0;
   X2C_PFREE(sym);
} /* end micedata() */


static void alt2str(int32_t feet, char s[], uint32_t s_len)
{
   if (feet>-100000L) {
      s[0UL] = '/';
      s[1UL] = 'A';
      s[2UL] = '=';
      if (feet<0L) {
         feet = -feet;
         s[3UL] = '-';
      }
      else s[3UL] = num(feet/100000L);
      s[4UL] = num(feet/10000L);
      s[5UL] = num(feet/1000L);
      s[6UL] = num(feet/100L);
      s[7UL] = num(feet/10L);
      s[8UL] = num(feet);
      s[9UL] = 0;
   }
   else s[0UL] = 0;
} /* end alt2str() */


static void speeddir2str(int32_t knots, int32_t dir, char areaobj,
                 char s[], uint32_t s_len)
{
   if (areaobj || dir>0L && dir<=360L) {
      /*    IF dir=0 THEN dir:=360 END; */
      s[0UL] = num(dir/100L);
      s[1UL] = num(dir/10L);
      s[2UL] = num(dir);
   }
   else {
      s[0UL] = '.';
      s[1UL] = '.';
      s[2UL] = '.';
   }
   if (areaobj && knots>=1000L) s[3UL] = num(knots/1000L);
   else s[3UL] = '/';
   s[4UL] = num(knots/100L);
   s[5UL] = num(knots/10L);
   s[6UL] = num(knots);
   s[7UL] = 0;
} /* end speeddir2str() */


extern void aprstext_compressdata(struct aprsstr_POSITION pos,
                uint32_t knots, uint32_t dir, int32_t feet,
                char sym[], uint32_t sym_len, char s[],
                uint32_t s_len)
{
   int32_t n;
   char h[201];
   X2C_PCOPY((void **)&sym,sym_len);
   pos.lat = pos.lat*5.7295779513082E+1f;
   pos.long0 = pos.long0*5.7295779513082E+1f;
   s[0UL] = sym[0UL];
   if (pos.lat<90.0f) {
      n = (int32_t)aprsdecode_trunc((90.0f-pos.lat)*3.80926E+5f);
   }
   else n = 0L;
   s[1UL] = (char)(33L+n/753571L);
   s[2UL] = (char)(33L+(n/8281L)%91L);
   s[3UL] = (char)(33L+(n/91L)%91L);
   s[4UL] = (char)(33L+n%91L);
   if (pos.long0>(-180.0f)) {
      n = (int32_t)aprsdecode_trunc((180.0f+pos.long0)*1.90463E+5f);
   }
   else n = 0L;
   s[5UL] = (char)(33L+n/753571L);
   s[6UL] = (char)(33L+(n/8281L)%91L);
   s[7UL] = (char)(33L+(n/91L)%91L);
   s[8UL] = (char)(33L+n%91L);
   s[9UL] = sym[1UL];
   if (knots>0UL) {
      if (dir>=360UL) dir = 0UL;
      s[10UL] = (char)(33UL+dir/4UL);
      s[11UL] = (char)(33UL+aprsdecode_trunc(osic_ln((float)
                (knots+1UL))*1.29935872129E+1f));
      s[12UL] = '_';
   }
   else if (feet>0L) {
      n = (int32_t)aprsdecode_trunc(osic_ln((float)feet)*500.5f+0.5f);
      if (n>=8281L) n = 8280L;
      s[10UL] = (char)(33L+n/91L);
      s[11UL] = (char)(33L+n%91L);
      s[12UL] = 'W';
   }
   else {
      s[10UL] = ' ';
      s[11UL] = ' ';
      s[12UL] = '_';
   }
   s[13UL] = 0;
   if (knots>0UL && feet>0L) {
      alt2str(feet, h, 201ul);
      aprsstr_Append(s, s_len, h, 201ul);
   }
   X2C_PFREE(sym);
} /* end compressdata() */

#define aprstext_Z0 48


static void deg2str(int32_t lat, int32_t long0, char s[],
                uint32_t s_len)
/* DDMM.MMNDDDMM.MME */
{
   if (lat<0L) {
      lat = -lat;
      s[7UL] = 'S';
   }
   else s[7UL] = 'N';
   if (long0<0L) {
      long0 = -long0;
      s[17UL] = 'W';
   }
   else s[17UL] = 'E';
   s[0UL] = (char)(lat/60000L+48L);
   s[1UL] = (char)((lat/6000L)%10L+48L);
   s[2UL] = (char)((lat/1000L)%6L+48L);
   s[3UL] = (char)((lat/100L)%10L+48L);
   s[4UL] = '.';
   s[5UL] = (char)((lat/10L)%10L+48L);
   s[6UL] = (char)(lat%10L+48L);
   s[9UL] = (char)(long0/600000L+48L);
   s[10UL] = (char)((long0/60000L)%10L+48L);
   s[11UL] = (char)((long0/6000L)%10L+48L);
   s[12UL] = (char)((long0/1000L)%6L+48L);
   s[13UL] = (char)((long0/100L)%10L+48L);
   s[14UL] = '.';
   s[15UL] = (char)((long0/10L)%10L+48L);
   s[16UL] = (char)(long0%10L+48L);
   s[18UL] = 0;
} /* end deg2str() */


static void getbeaconpos(struct aprsstr_POSITION * pos, char * err)
{
   int32_t fd;
   int32_t len;
   char s[1001];
   useri_confstr(useri_fRBPOS, s, 1001ul);
   if (s[0U]==':') {
      /* get beacon position from file */
      aprsstr_Delstr(s, 1001ul, 0UL, 1UL);
      fd = osi_OpenRead(s, 1001ul);
      s[0U] = 0;
      if (osic_FdValid(fd)) {
         len = osi_RdBin(fd, (char *)s, 1001u/1u, 1000UL);
         if (len<1L) {
            len = 0L;
            useri_say("\012beacon position file not readable\012", 36ul,
                20UL, 'e');
         }
         else if (len>1000L) len = 1000L;
         s[len] = 0;
         osic_Close(fd);
      }
      else {
         useri_say("\012beacon position file not found\012", 33ul, 20UL,
                'e');
      }
   }
   if (s[0U]) {
      aprstext_deganytopos(s, 1001ul, pos);
      if (!aprspos_posvalid(*pos)) {
         useri_say("\012beacon: object/item position wrong\012", 37ul, 4UL,
                'e');
         *err = 1;
      }
   }
   else {
      useri_say("\012beacon: no object/item position\012", 34ul, 4UL, 'e');
      *err = 1;
      aprsstr_posinval(pos);
   }
} /* end getbeaconpos() */


extern char aprstext_callwrong(char s[], uint32_t s_len)
{
   uint32_t i;
   i = 0UL;
   while (i<=s_len-1 && s[i]) {
      if ((uint8_t)s[i]<=' ' || (uint8_t)s[i]>='\177') return 1;
      ++i;
   }
   return i<3UL || i>9UL;
} /* end callwrong() */

#define aprstext_RAD 3.4377467707849E+7


extern void aprstext_encbeacon(char s[], uint32_t s_len,
                uint32_t * len)
/* assemble beacon string */
{
   char h[201];
   char symb[2];
   char postyp;
   char typ;
   char mull;
   char areaobj;
   char err;
   char dao;
   char bkn;
   struct aprsstr_POSITION pos;
   int32_t i;
   int32_t longd;
   int32_t latd;
   int32_t long0;
   int32_t lat;
   int32_t alt;
   int32_t feet;
   int32_t knots;
   int32_t dir;
   uint32_t datastart;
   err = 0;
   *len = 16UL; /* ax.25 address + UI + PID */
   useri_confstr(useri_fRBTYP, s, s_len);
   typ = X2C_CAP(s[0UL]);
   bkn = (((typ!='O' && typ!='H') && typ!='P') && typ!='I') && typ!='J';
   useri_confstr(useri_fRBPOSTYP, (char *) &postyp, 1u/1u);
   useri_confstr(useri_fRBSYMB, symb, 2ul);
   areaobj = (typ=='O' && symb[0U]=='\\') && symb[1U]=='l';
   if (aprsstr_Length(symb, 2ul)!=2UL || (uint8_t)symb[1U]<=' ') {
      useri_say("\012no symbol\012", 12ul, 4UL, 'e');
      err = 1;
   }
   dao = postyp=='G' || postyp=='M';
   knots = useri_conf2int(useri_fRBSPEED, 0UL, 0L, 32767L, 0L);
   if (!areaobj) {
      knots = (int32_t)aprsdecode_trunc((float)
                knots*5.3995680345572E-1f+0.5f);
   }
   dir = useri_conf2int(useri_fRBDIR, 0UL, 0L, 1000L, 1000L);
   /*WrInt(dir, 9); WrStrLn("defdir"); */
   if (dir<1000L) {
      if (!areaobj) {
         if (dir>359L) {
            useri_say("\012beacon: Direction <360\012", 25ul, 20UL, 'e');
            err = 1;
         }
         else if (knots==0L) {
            useri_say("\012direction needs speed>0\012", 26ul, 20UL, 'e');
         }
         if (dir==0L) dir = 360L;
      }
   }
   else dir = 0L;
   alt = useri_conf2int(useri_fRBALT, 0UL, -10000L, 1000000L, -32768L);
   feet = (int32_t)aprsdecode_trunc((float)fabs(X2C_DIVR((float)alt,
                0.3048f)+0.5f));
   if (alt<0L) feet = -feet;
   useri_confstr(useri_fRBPORT, s, s_len); /* port */
   aprsstr_Append(s, s_len, ":", 2ul);
   aprsstr_IntToStr(useri_conf2int(useri_fRBTIME, 0UL, 0L, 32767L, 3600L),
                1UL, h, 201ul);
   aprsstr_Append(s, s_len, h, 201ul); /* repeat time */
   aprsstr_Append(s, s_len, ":", 2ul);
   if (bkn) {
      useri_confstr(useri_fRBNAME, h, 201ul);
      if ((uint8_t)h[0U]<=' ') {
         useri_say("\012no beacon call?\012", 18ul, 4UL, 'e');
      }
      if (useri_configon(useri_fMUSTBECALL) && !aprstext_isacall(h, 201ul)) {
         useri_say("\012not valid callsign as beacon sender?\012", 39ul,
                20UL, 'e');
      }
      aprsstr_Append(s, s_len, h, 201ul);
      if (aprstext_getmypos(&pos)) {
         if (!aprspos_posvalid(pos)) {
            useri_say("\012Net beacon position wrong\012", 28ul, 4UL, 'e');
            err = 1;
         }
      }
      else getbeaconpos(&pos, &err);
   }
   else {
      useri_confstr(useri_fMYCALL, h, 201ul);
      if ((uint8_t)h[0U]<=' ') {
         useri_say("\012object/item needs a Config/Online/My Call\012", 44ul,
                 4UL, 'e');
      }
      aprsstr_Append(s, s_len, h, 201ul);
      getbeaconpos(&pos, &err);
   }
   lat = (int32_t)aprsdecode_trunc((float)fabs(pos.lat)
                *3.4377467707849E+7f);
   long0 = (int32_t)aprsdecode_trunc((float)fabs(pos.long0)
                *3.4377467707849E+7f);
   latd = (int32_t)((uint32_t)lat%100UL);
   longd = (int32_t)((uint32_t)long0%100UL);
   if (dao) {
      lat -= latd;
      long0 -= longd;
   }
   lat = lat/100L;
   long0 = long0/100L;
   if (pos.lat<0.0f) lat = -lat;
   if (pos.long0<0.0f) {
      long0 = -long0;
   }
   aprsstr_Append(s, s_len, ">", 2ul);
   if (X2C_CAP(postyp)=='M') micedest(lat, long0, h, 201ul);
   else {
      useri_confstr(useri_fRBDEST, h, 201ul);
      if (h[0U]==0) strncpy(h,"APLM01",201u);
      i = aprsstr_InStr(h, 201ul, "-", 2ul);
      if (i>0L) aprsstr_Delstr(h, 201ul, (uint32_t)i, 201UL);
   }
   aprsstr_Append(s, s_len, h, 201ul);
   useri_confstr(useri_fRBPATH, h, 201ul);
   if ((uint8_t)h[0U]>' ') {
      if (h[0U]!='-') aprsstr_Append(s, s_len, ",", 2ul);
      aprsstr_Append(s, s_len, h, 201ul); /* dest ssid + via path */
      i = 0L;
      while (s[i]) {
         if (s[i]==',') *len += 7UL;
         ++i;
      }
   }
   aprsstr_Append(s, s_len, ":", 2ul); /* end of address */
   datastart = aprsstr_Length(s, s_len); /* rest of line for byte count */
   if (!bkn) {
      useri_confstr(useri_fRBNAME, h, 201ul);
      if ((uint8_t)h[0U]<=' ') {
         useri_say("\012no object/item name?\012", 23ul, 20UL, 'e');
         err = 1;
      }
      aprsstr_Append(h, 201ul, "         ", 10ul); /* fix size */
      h[9U] = 0;
      if ((typ=='O' || typ=='H') || typ=='P') {
         aprsstr_Append(s, s_len, ";", 2ul); /* object */
         aprsstr_Append(s, s_len, h, 201ul);
         if (typ=='P') aprsstr_Append(s, s_len, "_", 2ul);
         else aprsstr_Append(s, s_len, "*", 2ul);
         if (typ=='O') aprsstr_Append(s, s_len, "\\\\Zz", 5ul);
         else aprsstr_Append(s, s_len, "\\\\Hh", 5ul);
      }
      else {
         aprsstr_Append(s, s_len, ")", 2ul); /* item */
         i = 3L;
         while ((uint8_t)h[i]>' ') ++i;
         h[i] = 0; /* item size 3..9 */
         aprsstr_Append(s, s_len, h, 201ul);
         if (typ=='J') aprsstr_Append(s, s_len, "_", 2ul);
         else aprsstr_Append(s, s_len, "!", 2ul);
      }
   }
   else {
      if (X2C_CAP(postyp)=='M') strncpy(h,"`",201u);
      else strncpy(h,"=",201u);
      aprsstr_Append(s, s_len, h, 201ul); /* beacon protocol */
   }
   switch ((unsigned)X2C_CAP(postyp)) {
   case 'M':
      micedata(lat, long0, (uint32_t)knots, (uint32_t)dir, alt, symb,
                2ul, h, 201ul);
      aprsstr_Append(s, s_len, h, 201ul);
      break;
   case 'C':
      if (feet<0L && feet>-10000L) {
         useri_say("\012no negative altitude in compressed mode\012", 42ul,
                20UL, 'e');
         err = 1;
      }
      if ((uint8_t)symb[0U]>='a' && (uint8_t)symb[0U]<='j') {
         useri_say("\012overlay character a..j not in compressed mode\012",
                48ul, 20UL, 'e');
         err = 1;
      }
      if ((uint8_t)symb[0U]>='0' && (uint8_t)symb[0U]<='9') {
         symb[0U] = (char)((uint32_t)(uint8_t)symb[0U]+49UL);
      }
      aprstext_compressdata(pos, (uint32_t)knots, (uint32_t)dir, feet,
                symb, 2ul, h, 201ul);
      aprsstr_Append(s, s_len, h, 201ul);
      break;
   default:;
      deg2str(lat, long0, h, 201ul);
      h[8U] = symb[0U]; /* symbol */
      h[18U] = symb[1U];
      h[19U] = 0;
      aprsstr_Append(s, s_len, h, 201ul);
      if (knots>0L) {
         /* dir, speed */
         speeddir2str(knots, dir, areaobj, h, 201ul);
         aprsstr_Append(s, s_len, h, 201ul);
      }
      if (alt>-10000L) {
         alt2str(feet, h, 201ul);
         aprsstr_Append(s, s_len, h, 201ul);
      }
      break;
   } /* end switch */
   mull = aprsdecode_ismultiline(0);
   /*  IF mull THEN confappend(fRBCOMMENT, s) END;
                (* multiline: append dao at end *) */
   useri_confappend(useri_fRBCOMMENT, s, s_len);
   if (X2C_CAP(postyp)!='C' && (dao || mull)) {
      /* DAO */
      daostr(latd, longd, h, 201ul);
      aprsstr_Append(s, s_len, h, 201ul);
   }
   /*  IF NOT mull THEN confappend(fRBCOMMENT, s) END; */
   *len += aprsstr_Length(s, s_len)-datastart;
   if (err) s[0UL] = 0;
} /* end encbeacon() */


extern void aprstext_BEGIN(void)
{
   static int aprstext_init = 0;
   if (aprstext_init) return;
   aprstext_init = 1;
   useri_BEGIN();
   osi_BEGIN();
   aprspos_BEGIN();
   libsrtm_BEGIN();
   maptool_BEGIN();
   aprsdecode_BEGIN();
   aprsstr_BEGIN();
}

