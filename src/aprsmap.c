/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define aprsmap_C_
#ifndef Storage_H_
#include "Storage.h"
#endif
#ifndef TimeConv_H_
#include "TimeConv.h"
#endif
#ifndef useri_H_
#include "useri.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef maptool_H_
#include "maptool.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef xosi_H_
#include "xosi.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#include <math.h>
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef FileSys_H_
#include "FileSys.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef aprstext_H_
#include "aprstext.h"
#endif
#include <signal.h>

/* aprs tracks on osm map by oe5dxl */
#define aprsmap_MINLIG 60

#define aprsmap_MAXZOOMOBJ 14

#define aprsmap_MAXCOLORS 10

#define aprsmap_MINCOLOR 48

#define aprsmap_MAPGETTIMEOUT 30

#define aprsmap_HISTSIZE 20

#define aprsmap_VIDEOFN "map.y4m"

#define aprsmap_FLOATERRCORR (-6.E-7)
/* try to correct summed rounding errors by multiple zoom in */

#define aprsmap_VIDEORATE 25

#define aprsmap_MARKERTIME 10
/* marker life time for unimportant marker settings */

#define aprsmap_NOSYMT "/"
/* show this symbol if user has none */

#define aprsmap_NOSYMB "/"

#define aprsmap_POIFILENAME "poi.txt"

enum MHOPS {aprsmap_OPHEARD, aprsmap_OPSENT, aprsmap_OPOBJ};


struct VIEW;


struct VIEW {
   struct aprspos_POSITION pos;
   float zoom;
   aprsdecode_MONCALL mhop;
   long rf;
   unsigned char mhtx0;
   char wxcol;
   unsigned short lumtext;
   char focus;
};

static char logdone;

static char quit;

static maptool_pIMAGE image;

static maptool_pIMAGE rfimg;

static char withx;

static unsigned long lastxupdate;

static unsigned long maptime;

static unsigned long logstarttime;

static unsigned long laststatref;

static unsigned long logredcnt;

static unsigned long cycleorder;

static unsigned long maptrys;

static unsigned long uptime;

static struct aprspos_POSITION newpos0;

static struct aprspos_POSITION newpos1;
/*    squerpos: RECORD pos0, pos1:POSITION END; */

static long videofd;

static char gammatab[1024];

static unsigned char mhtx;

static char pandone;

static char onetipp;

static unsigned long stkpo;

static unsigned long stktop;

static struct VIEW posstk[20];

static struct VIEW alttabview;

static char * vidbuf;

static char mestxt[201];

struct _0;


struct _0 {
   char wasradio;
   struct aprspos_POSITION markpos;
   struct aprspos_POSITION measurepos;
   struct aprspos_POSITION mappos;
   long initzoom;
   float finezoom;
   long ant1;
   long ant2;
   long ant3;
   long bri;
   long contr;
   long qual;
   struct aprsdecode_COLTYP c1;
   struct aprsdecode_COLTYP c2;
};

static struct _0 radio;

static struct aprspos_POSITION clickwatchpos;


static void Error(char text0[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text0,text_len);
   InOut_WriteString(text0, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text0);
} /* end Error() */


static float sq(float x)
{
   return x*x;
} /* end sq() */

#define aprsmap_GAMMA 4.5454545454545E-1


static unsigned long gammac(unsigned long c)
{
   if (c==0UL) return 0UL;
   if (c<1024UL) {
      return aprsdecode_trunc(RealMath_exp(RealMath_ln(X2C_DIVR((float)c,
                1024.0f))*4.5454545454545E-1f)*255.5f);
   }
   return 255UL;
} /* end gammac() */


static void makegammatab(void)
{
   unsigned long i;
   for (i = 0UL; i<=1023UL; i++) {
      gammatab[i] = (char)gammac(i);
   } /* end for */
} /* end makegammatab() */


static void tooltips(char typ)
{
   char s[21];
   struct aprspos_POSITION pos;
   if (typ==' ') {
      if (aprsdecode_initzoom<=1L && uptime+2UL==aprsdecode_realtime) {
         useri_starthint(10005UL, 1);
      }
   }
   else if (typ=='n') {
      if (useri_configon(useri_fCONNECT) && uptime+60UL>aprsdecode_realtime) {
         useri_confstr(useri_fMYPOS, s, 21ul);
         aprstext_degtopos(s, 21ul, &pos);
         useri_confstr(useri_fSERVERFILT, s, 21ul);
         if (X2C_CAP(s[0U])=='M' && !(aprsdecode_getmypos(&pos)
                && useri_configon(useri_fALLOWNETTX))) {
            useri_starthint(10007UL, 1);
         }
         else useri_starthint(10006UL, 1);
      }
   }
   else if (typ=='m') {
      if (uptime+180UL>aprsdecode_realtime) useri_starthint(10010UL, 1);
   }
   else if (typ=='b') {
      if (!onetipp) useri_starthint(10015UL, 1);
      onetipp = 1;
   }
   else if (typ=='B') {
      if (!onetipp) useri_starthint(10016UL, 1);
      onetipp = 1;
   }
} /* end tooltips() */


static float movest(unsigned long width)
{
   return RealMath_power(2.0f, -maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom))*(float)width*(float)
                aprsdecode_lums.movestep*0.0002f;
} /* end movest() */


static float shiftfine(void)
{
   if (xosi_Shift) return 0.1f;
   else return 1.0f;
   return 0;
} /* end shiftfine() */

static unsigned long aprsmap_MON[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,
                181UL,212UL,243UL,273UL,304UL,334UL};

static unsigned long _cnst[13] = {0UL,0UL,31UL,59UL,90UL,120UL,151UL,181UL,
                212UL,243UL,273UL,304UL,334UL};

static char rdlogdate(const char s[], unsigned long s_len, unsigned long * t,
                 unsigned long * p)
/* 20130526:235959 */
{
   unsigned long i;
   unsigned long y;
   unsigned long d;
   char c;
   char dt; /* second since 1970 or yyyymmdd:hhmmss */
   dt = s[8UL]==':';
   i = 0UL;
   d = 0UL;
   for (;;) {
      c = s[i];
      if ((unsigned char)c<'0' || (unsigned char)c>'9') break;
      d = (d*10UL+(unsigned long)(unsigned char)c)-48UL;
      ++i;
      if (dt) {
         if (i==4UL) {
            /* year */
            if (d<1970UL || d>2100UL) return 0;
            *t = (d-1970UL)*365UL+(d-1969UL)/4UL; /* days since 1970 */
            y = d;
            d = 0UL;
         }
         else if (i==6UL) {
            /* month */
            if (d>12UL) return 0;
            *t += _cnst[d];
            if ((y&3UL)==0UL && d>2UL) ++*t;
            d = 0UL;
         }
         else if (i==8UL) {
            /* day */
            if (d<=0UL) return 0;
            *t = ((*t+d)-1UL)*86400UL;
            ++i;
            d = 0UL;
         }
         else if (i==11UL) {
            *t += d*3600UL;
            d = 0UL;
         }
         else if (i==13UL) {
            *t += d*60UL;
            d = 0UL;
         }
         else {
            if (i==15UL) {
               *t += d;
               break;
            }
            if (i>15UL) return 0;
         }
      }
   }
   if (!dt) *t = d;
   *p = i;
   return 1;
} /* end rdlogdate() */


static char cmpcall(const char a[], unsigned long a_len, unsigned long i,
                const char b[], unsigned long b_len)
{
   unsigned long j;
   j = 0UL;
   do {
      if (a[i]!=b[j]) return 0;
      ++i;
      ++j;
   } while (b[j]);
   return a[i]=='>';
} /* end cmpcall() */

#define aprsmap_MINSTEP 5000
/* stop binary search */

#define aprsmap_SEEKBUF 1024
/* buffer size while binary search */

#define aprsmap_RETR 200
/* if unsort file break binary search */

/*-hhh:ARRAY[0..50] OF CHAR; */

static void binseek(unsigned long * i, long fc, unsigned long * wp,
                unsigned long time0, char bof, unsigned long * first,
                unsigned long * last)
{
   long seekstep;
   long len;
   long rp;
   char sb[1024];
   aprsdecode_FRAMEBUF mbuf;
   unsigned long retry;
   unsigned long lfc;
   unsigned long ftime;
   retry = 0UL;
   *first = 0UL;
   *last = 0UL;
   seekstep = 1073741823L;
   for (;;) {
      len = 0L;
      rp = 0L;
      *wp = 0UL;
      if (bof) lfc = 2UL;
      else lfc = 0UL;
      bof = 0;
      for (;;) {
         if (rp>=len) {
            len = osi_RdBin(fc, (char *)sb, 1024u/1u, 1024UL);
            rp = 0L;
         }
         if (len<=0L) {
            lfc = 0UL;
            break;
         }
         mbuf[*wp] = sb[rp];
         if (mbuf[*wp]=='\012') {
            mbuf[*wp] = 0;
            *wp = 0UL;
            ++lfc;
            if (lfc>=2UL) break;
         }
         else if (*wp<510UL) ++*wp;
         else {
            *wp = 0UL;
            lfc = 0UL;
         }
         ++rp;
      }
      if (lfc>=2UL && rdlogdate(mbuf, 512ul, &ftime, i)) {
         /* we are in file */
         /*DateToStr(ftime, hhh); WrStr(hhh); ; WrStrLn(" =ftime "); */
         if (*first==0UL) {
            *first = ftime;
            if (time0<*first) {
               time0 = 0UL;
               break;
            }
         }
         if (ftime>*last) *last = ftime;
      }
      else ftime = X2C_max_longcard;
      if (ftime<time0==seekstep<0L) seekstep = -X2C_DIV(seekstep,2L);
      if (seekstep>=0L && seekstep<5000L || retry>=200UL) break;
      osi_Seekcur(fc, seekstep-1024L);
      ++retry;
   }
   if (time0<=*first) osi_Seek(fc, 0UL);
   else if (retry>=200UL) {
      osi_Seek(fc, 0UL);
      useri_textautosize(0L, 0L, 4UL, 180UL, 'r', "unsort logfile, trying lin\
ear search", 37ul);
      osi_WrStrLn("unsort logfile, trying linear search", 37ul);
   }
   else osi_Seekcur(fc, -6024L);
} /* end binseek() */


static char rdlog(aprsdecode_pOPHIST * optab, char fn[],
                unsigned long fn_len, unsigned long * from, long span,
                char find0[], unsigned long find_len)
{
   long fc;
   long ret;
   long len;
   long rp;
   char ib[32768];
   unsigned long j;
   unsigned long i;
   unsigned long wp;
   aprsdecode_FRAMEBUF mbuf;
   unsigned long end;
   unsigned long start;
   aprsdecode_pOPHIST op;
   struct aprsdecode_DAT dat;
   unsigned long lfc;
   char rdlog_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   X2C_PCOPY((void **)&find0,find_len);
   aprsstr_cleanfilename(fn, fn_len);
   if (fn[0UL]==0) {
      rdlog_ret = 0;
      goto label;
   }
   logredcnt = 0UL;
   fc = osi_OpenRead(fn, fn_len);
   if (!osi_FdValid(fc)) {
      rdlog_ret = 0;
      goto label;
   }
   find0[find_len-1] = 0;
   logstarttime = 0UL;
   /*DateToStr(from, hhh); WrStr(hhh); WrInt(span, 10); WrStrLn(" =from ");
                */
   end = 0UL;
   if (*from>0UL) {
      if (span<0L) {
         if (*from>(unsigned long) -span) *from += (unsigned long)span;
         else *from = 0UL;
      }
      binseek(&i, fc, &wp, *from, 1, &start, &end);
      logstarttime = start;
      /*DateToStr(logstarttime, hhh); WrStr(hhh);
                WrStrLn(" =logstarttime1 "); */
      /*DateToStr(from, hhh); WrStr(hhh); WrStrLn(" =from2 "); */
      /*DateToStr(start, hhh); WrStr(hhh); WrStrLn(" =start2 "); */
      /*DateToStr(end, hhh); WrStr(hhh); WrStrLn(" =end2 "); */
      aprsdecode_systime = end;
      /*
          IF from>end THEN
            IF span>0 THEN
              from:=end-VAL(CARDINAL, span);
              binseek(from, v1, v2);
            ELSE from:=end END;
          END;
      */
      if (*from>end) *from = end;
      if (*from<start) *from = 0UL;
      else if (find0[0UL]==0 || span<0L) {
         end = *from+(unsigned long)labs(span);
      }
      else end = 0UL;
   }
   /*binseek(from, v1, v2); */
   /*DateToStr(from, hhh); WrStr(hhh); WrStrLn(" =from3 "); */
   /*DateToStr(end, hhh); WrStr(hhh); WrStrLn(" =end3 "); */
   len = 0L;
   rp = 0L;
   wp = 0UL;
   if (*from==0UL) lfc = 2UL;
   else lfc = 0UL;
   for (;;) {
      if (rp>=len) {
         len = osi_RdBin(fc, (char *)ib, 32768u/1u, 32768UL);
         if (len<=0L) break;
         rp = 0L;
      }
      mbuf[wp] = ib[rp];
      if (mbuf[wp]=='\012') {
         ++lfc;
         mbuf[wp] = 0;
         /*WrStrLn(mbuf); */
         if ((lfc>=2UL && rdlogdate(mbuf, 512ul, &start,
                &i)) && start>=*from) {
            if (logstarttime==0UL) logstarttime = start;
            if (find0[0UL]==0 || cmpcall(mbuf, 512ul, i+1UL, find0,
                find_len)) {
               if (*from<start) *from = start;
               if (end==0UL) end = *from+(unsigned long)labs(span);
               if (start>=end) break;
               /*
                         IF start<logstarttime THEN 
                           WrStrLn(mbuf);
                           WrStrLn("unsort logfile, end search");
                           EXIT
                         END; 
               */
               aprsdecode_systime = start;
               ++i;
               j = 0UL;
               do {
                  mbuf[j] = mbuf[i];
                  ++j;
                  ++i;
               } while (!(i>=511UL || mbuf[i]==0));
               mbuf[j] = 0;
               if (aprsdecode_Decode(mbuf, 512ul, &dat)>=0L) {
                  ret = aprsdecode_Stoframe(optab, mbuf, 512ul, start, 1,
                dat);
                  ++logredcnt;
               }
            }
         }
         wp = 0UL;
      }
      else if (wp<510UL) ++wp;
      ++rp;
   }
   if (end>0UL) *from = end;
   osi_Close(fc);
   op = *optab;
   while (op) {
      aprsdecode_Checktrack(op, 0);
      op = op->next;
   }
   rdlog_ret = 1;
   label:;
   X2C_PFREE(fn);
   X2C_PFREE(find0);
   return rdlog_ret;
} /* end rdlog() */


static char moving(aprsdecode_pOPHIST op)
{
   struct aprsdecode_OPHIST * anonym;
   { /* with */
      struct aprsdecode_OPHIST * anonym = op;
      return !aprsdecode_lums.moving || ((anonym->lastinftyp>0U && anonym->lastinftyp<100U)
                 && anonym->lastkmh>0)
                && aprsdecode_systime<anonym->lasttime+aprsdecode_lums.kmhtime;
                
   }
} /* end moving() */


static unsigned long fade(unsigned long t1, aprsdecode_pOPHIST op)
{
   unsigned long l;
   unsigned long t0;
   if (!moving(op)) return (unsigned long)aprsdecode_lums.nomov;
   t0 = aprsdecode_systime-aprsdecode_lums.firstdim;
   if (t0<t1) t1 = t0;
   t0 -= t1;
   if (t0>aprsdecode_lums.maxdim) t0 = aprsdecode_lums.maxdim;
   if (aprsdecode_lums.maxdim>0UL) {
      l = 60UL+(196UL*(aprsdecode_lums.maxdim-t0))/aprsdecode_lums.maxdim;
   }
   else l = 256UL;
   return l;
} /* end fade() */

/*
PROCEDURE runaway(x0,y0,x1,y1:REAL);
BEGIN
  IF (x0<0.0)<>(x1<0.0) THEN y0:=y0+(y1-y0)*x0/(x1-x0); x0:=0;
  ELSIF (x0<FLOAT(xsize))<>(x1<FLOAT(xsize))
  THEN y0:=y0+(y1-y0)*(x0-FLOAT(xsize))/(x1-x0); x0:=FLOAT(xsize); END;
  
  IF (y0<0.0)<>(y1<0.0) THEN x0:=x0+(x1-x0)*y0/(y1-y0); y0:=0;
  ELSIF (y0<FLOAT(ysize))<>(y1<FLOAT(ysize))
  THEN x0:=x0+(x1-x0)*(y0-FLOAT(ysize))/(y1-y0); y0:=FLOAT(ysize); END;

END runanway;
*/

static unsigned long findfreecol(aprsdecode_pOPHIST opn)
{
   unsigned long n;
   unsigned long imax;
   unsigned long max1;
   unsigned long c;
   unsigned long i;
   unsigned long cs;
   aprsdecode_pOPHIST op;
   struct aprspos_POSITION pos;
   c = (unsigned long)(unsigned char)opn->trackcol;
   if (c<48UL) {
      pos = opn->lastpos;
      cs = 0UL;
      if (aprspos_posvalid(pos)) {
         op = aprsdecode_ophist0;
         while (op) {
            if ((op->trackcol>=48 && op->trackcol<58)
                && aprspos_distance(op->lastpos, pos)<50.0f) {
               cs |= (1UL<<op->trackcol-48);
            }
            op = op->next;
         }
      }
      c = 0UL;
      i = 0UL;
      while (i<=8UL && opn->call[i]) {
         c += (unsigned long)(unsigned char)opn->call[i];
         ++i;
      }
      c = c%10UL; /* checksum default color */
      if (X2C_IN(c,32,cs)) {
         /* color in use */
         n = 0UL;
         max1 = 0UL;
         imax = 0UL;
         for (i = 0UL; i<=9UL; i++) {
            if (X2C_IN(i,32,cs)) n = 0UL;
            else {
               ++n; /* count free colors */
               if (n>max1) {
                  max1 = n;
                  imax = i;
               }
            }
         } /* end for */
         if (max1>0UL) c = imax-max1/2UL;
      }
      c = c%10UL+48UL;
   }
   return c;
} /* end findfreecol() */


static void changecolor(aprsdecode_pOPHIST op)
{
   if (op && op->trackcol>=48) {
      op->trackcol = (signed char)X2C_MOD((op->trackcol-48)+1,11)+48;
   }
} /* end changecolor() */

/*
PROCEDURE wrtime(t:TIME);
VAR h:ARRAY[0..100] OF CHAR;
BEGIN  DateToStr(t,h); WrStr(h) END wrtime;
*/
#define aprsmap_MAXDIST 5.E+5

#define aprsmap_MINWPDIST 9.0
/* waypoints on tracks minimum distance */


static void tracks(maptool_pIMAGE img, aprsdecode_pOPHIST op,
                char highlight0, unsigned long tilltime)
{
   float y1;
   float x1;
   float y00;
   float x0;
   unsigned long ligw;
   unsigned long lig;
   struct aprspos_POSITION oldpos;
   struct aprsdecode_COLTYP col;
   signed char coln;
   struct aprsdecode_VARDAT * anonym;
   aprsdecode_click.ops = op;
   if (!highlight0) aprsdecode_click.typ = aprsdecode_tTRACK;
   while (aprsdecode_click.ops) {
      if ((((aprsdecode_click.ops->drawhints&0x18U)
                !=0U && (aprsdecode_click.ops->lastinftyp>=100U || aprsdecode_lums.wxcol==0)
                ) && aprsdecode_click.ops->trackcol<58)
                && aprsdecode_click.ops->sym.tab!='\001') {
         /* make a random track color */
         coln = (signed char)findfreecol(aprsdecode_click.ops);
         maptool_Colset(&col, (char)coln);
         aprsdecode_click.pf0 = 0;
         aprsdecode_click.pf = aprsdecode_click.ops->frames;
         aprsdecode_posinval(&oldpos);
         while (aprsdecode_click.pf && aprsdecode_click.pf->time0<=tilltime) {
            { /* with */
               struct aprsdecode_VARDAT * anonym = aprsdecode_click.pf->vardat;
                
               if (((!useri_configon(useri_fTRACKFILT)
                || !(aprsdecode_click.pf->nodraw&~0x1U))
                && aprspos_posvalid(anonym->pos))
                && maptool_vistime(aprsdecode_click.pf->time0)) {
                  if (aprspos_posvalid(oldpos)) {
                     if (highlight0) lig = 700UL;
                     else {
                        lig = (fade(aprsdecode_click.pf->time0,
                aprsdecode_click.ops)*600UL)/256UL;
                     }
                     if ((maptool_mapxy(oldpos, &x0,
                &y00)>=-1L && maptool_mapxy(anonym->pos, &x1,
                &y1)>=-1L) && sq(x0-x1)+sq(y00-y1)>=9.0f) {
                        /* collect short tracks to 1 */
                        aprsdecode_click.ops->trackcol = coln;
                        ligw = (lig*(unsigned long)aprsdecode_lums.waypoint)
                /1024UL;
                        lig = (lig*(unsigned long)aprsdecode_lums.track)
                /1024UL;
                        maptool_vector(img, x0, y00, x1, y1,
                (long)((lig*col.r)/256UL), (long)((lig*col.g)/256UL),
                (long)((lig*col.b)/256UL), 450UL, 0.0f);
                        if (!aprsdecode_click.dryrun) {
                           maptool_waypoint(img, x0, y00, 1.5f, (long)ligw,
                (long)ligw, (long)ligw);
                        }
                        oldpos = anonym->pos;
                        aprsdecode_click.pf0 = aprsdecode_click.pf;
                     }
                     else aprsdecode_click.pf0 = aprsdecode_click.pf;
                  }
                  else {
                     oldpos = anonym->pos;
                     aprsdecode_click.pf0 = aprsdecode_click.pf;
                  }
               }
            }
            aprsdecode_click.pf = aprsdecode_click.pf->next;
         }
         aprsdecode_click.pf0 = 0;
         if (highlight0) return;
      }
      aprsdecode_click.ops = aprsdecode_click.ops->next;
   }
} /* end tracks() */


static void symbols(aprsdecode_pOPHIST op, char objects, char highlight0)
{
   float y;
   float x;
   unsigned long lig;
   struct aprsdecode_COLTYP col;
   /*  symt, symb:CHAR; */
   aprsdecode_click.ops = op;
   if (!highlight0) {
      if (objects) aprsdecode_click.typ = aprsdecode_tOBJECT;
      else aprsdecode_click.typ = aprsdecode_tSYMBOL;
   }
   while (aprsdecode_click.ops) {
      if ((((((0x8U & aprsdecode_click.ops->drawhints)
                && ((0x2U & aprsdecode_click.ops->drawhints)!=0)==objects)
                && aprsdecode_click.ops->sym.tab!='\001')
                && maptool_mapxy(aprsdecode_click.ops->lastpos, &x,
                &y)>=0L) && maptool_vistime(aprsdecode_click.ops->lasttime))
                && (aprsdecode_click.ops->lastinftyp>=100U || aprsdecode_lums.wxcol==0)
                ) {
         /*
               symt:=click.ops^.sym.tab;
               symb:=click.ops^.sym.pic; 
               IF symt<" " THEN symt:=NOSYMT; symb:=NOSYMB END;
                (* set default symbol *)    
         */
         if (highlight0) lig = 1000UL;
         else {
            lig = fade(aprsdecode_click.ops->lasttime, aprsdecode_click.ops);
         }
         if (objects) lig = (lig*(unsigned long)aprsdecode_lums.obj)/1024UL;
         else lig = (lig*(unsigned long)aprsdecode_lums.sym)/1024UL;
         maptool_drawsym(image, aprsdecode_click.ops->sym.tab,
                aprsdecode_click.ops->sym.pic,
                (0x1U & aprsdecode_click.ops->drawhints)!=0, x, y, lig);
         if (aprsdecode_click.ops->lastkmh>0) {
            if (aprsdecode_click.ops->lastinftyp>=10U && aprsdecode_click.ops->lastinftyp<100U)
                 {
               if (useri_configon(useri_fARROW)
                && aprsdecode_click.ops->lasttime+aprsdecode_lums.kmhtime>aprsdecode_systime)
                 {
                  col.r = 500UL;
                  col.g = 500UL;
                  col.b = 0UL;
                  maptool_drawarrow(image, x, y, 22.0f,
                (float)(aprsdecode_click.ops->lastinftyp-10U)
                *(-6.9813170079773E-2f), 0UL, lig, col);
               }
            }
            else if (aprsdecode_click.ops->lastinftyp>=110U && aprsdecode_click.ops->lastinftyp<200U)
                 {
               if (useri_configon(useri_fWINDSYM)
                && aprsdecode_click.ops->temptime+3600UL>aprsdecode_systime) {
                  col.r = 0UL;
                  col.g = 400UL;
                  col.b = 400UL;
                  maptool_drawarrow(image, x, y, 19.0f,
                (float)(aprsdecode_click.ops->lastinftyp-110U)
                *(-6.9813170079773E-2f),
                (unsigned long)(unsigned short)aprsdecode_click.ops->lastkmh,
                 lig, col);
               }
            }
         }
      }
      if (highlight0) return;
      aprsdecode_click.ops = aprsdecode_click.ops->next;
   }
} /* end symbols() */


static void text(aprsdecode_pOPHIST op, char yesno, char objmove,
                char highlight0, char withvalues)
{
   float y;
   float x;
   unsigned long lumtext;
   unsigned long lig;
   char s1[256];
   char s[256];
   struct aprsdecode_COLTYP colo;
   struct aprsdecode_COLTYP colw;
   struct aprsdecode_COLTYP col;
   char temponly;
   char object;
   char fix;
   temponly = aprsdecode_lums.wxcol && aprsdecode_lums.text==0L;
                /* labels off but wxonly */
   if (temponly) lumtext = 700UL;
   else lumtext = (unsigned long)aprsdecode_lums.text;
   aprsdecode_click.ops = op;
   useri_ColConfset(&colw, useri_fCOLMAPTEXT);
   useri_ColConfset(&colo, useri_fCOLOBJTEXT);
   while (aprsdecode_click.ops) {
      object = (0x2U & aprsdecode_click.ops->drawhints)!=0;
      if (((((((0x8U & aprsdecode_click.ops->drawhints)
                && aprsdecode_click.ops->sym.tab!='\001')
                && (aprsdecode_click.ops->lastinftyp>=100U || aprsdecode_lums.wxcol==0)
                ) && (objmove && object==yesno || !objmove && moving(aprsdecode_click.ops)
                ==yesno)) && maptool_vistime(aprsdecode_click.ops->lasttime))
                 && (aprsdecode_click.ops->lastinftyp>=100U || aprsdecode_lums.wxcol==0)
                ) && maptool_mapxy(aprsdecode_click.ops->lastpos, &x,
                &y)>=0L) {
         lig = (fade(aprsdecode_click.ops->lasttime,
                aprsdecode_click.ops)*lumtext)/256UL;
         if (object) {
            if (lig>(unsigned long)aprsdecode_lums.obj) {
               lig = (unsigned long)aprsdecode_lums.obj;
            }
            col = colo;
         }
         else col = colw;
         fix = aprsdecode_click.dryrun;
         if (highlight0) {
            lig = 1000UL;
            fix = 1;
         }
         else aprsdecode_click.typ = aprsdecode_tTEXT;
         aprsstr_Assign(s, 256ul, aprsdecode_click.ops->call, 9ul);
         if (!temponly) {
            maptool_drawstr(image, s, 256ul, (float)floor((double)(x+7.0f)),
                (float)floor((double)(y-(float)
                (aprsdecode_lums.fontysize/2UL))), lig, 1UL, col,
                &aprsdecode_click.ops->textpos, 3UL, fix,
                aprsdecode_click.dryrun);
         }
         if (withvalues) {
            if (aprsdecode_click.ops->lastinftyp>=100U) {
               /* temperature */
               if (((useri_configon(useri_fTEMP)
                && aprsdecode_click.ops->lasttempalt>=-99)
                && aprsdecode_click.ops->lasttempalt<=99)
                && aprsdecode_click.ops->temptime+3600UL>aprsdecode_systime) {
                  if (object) aprsdecode_click.typ = aprsdecode_tDEGREEOBJ;
                  else aprsdecode_click.typ = aprsdecode_tDEGREE;
                  aprsstr_IntToStr((long)aprsdecode_click.ops->lasttempalt,
                1UL, s, 256ul);
                  aprsstr_Append(s, 256ul, "\177C", 3ul);
                  if (aprsdecode_click.ops->lasttempalt<0) {
                     col.r = 20UL;
                     col.g = 180UL;
                     col.b = 255UL;
                  }
                  else maptool_Colset(&col, 'Y');
                  /*          drawstr(image, s, VAL(INTEGER,x+6),
                VAL(INTEGER,y-6), lig, 1, col, click.ops^.valuepos, 4, fix);
                */
                  maptool_drawstr(image, s, 256ul,
                (float)floor((double)(x+6.0f)),
                (float)floor((double)(y-(float)
                (aprsdecode_lums.fontysize/2UL))), lig, 0UL, col,
                &aprsdecode_click.ops->valuepos, 4UL, fix,
                aprsdecode_click.dryrun);
               }
            }
            else {
               /*        ELSIF click.ops^.lastinftyp<100 THEN                           (* kmh *)
                 */
               s[0] = 0;
               if (useri_configon(useri_fKMH)) {
                  /* & (click.ops^.lastinftyp>0) THEN */
                  if (object) aprsdecode_click.typ = aprsdecode_tKMHOBJ;
                  else aprsdecode_click.typ = aprsdecode_tKMH;
                  if ((aprsdecode_click.ops->lastinftyp>0U && aprsdecode_click.ops->lastkmh>0)
                 && aprsdecode_click.ops->lasttime+aprsdecode_lums.kmhtime>aprsdecode_systime)
                 {
                     aprsstr_IntToStr((long)aprsdecode_click.ops->lastkmh,
                1UL, s, 256ul);
                     useri_confappend(useri_fKMH, s, 256ul);
                  }
               }
               if ((long)aprsdecode_click.ops->lasttempalt+22768L>useri_conf2int(useri_fALTMIN,
                 0UL, -10000L, 65535L, -10000L)) {
                  if (s[0U]) aprsstr_Append(s, 256ul, " ", 2ul);
                  aprsstr_IntToStr((long)
                aprsdecode_click.ops->lasttempalt+22768L, 1UL, s1, 256ul);
                  aprsstr_Append(s, 256ul, s1, 256ul);
                  aprsstr_Append(s, 256ul, "m", 2ul);
               }
               if (s[0U]) {
                  maptool_Colset(&col, 'Y');
                  maptool_drawstr(image, s, 256ul,
                (float)floor((double)(x+7.0f)),
                (float)floor((double)(y-(float)
                (aprsdecode_lums.fontysize/2UL))), lig, 0UL, col,
                &aprsdecode_click.ops->valuepos, 4UL, fix,
                aprsdecode_click.dryrun);
               }
            }
         }
      }
      if (highlight0) return;
      aprsdecode_click.ops = aprsdecode_click.ops->next;
   }
} /* end text() */


static char getgate(aprsdecode_pVARDAT v, aprsdecode_pOPHIST * gate)
{
   aprsdecode_MONCALL digi;
   struct aprsdecode_VARDAT * anonym;
   { /* with */
      struct aprsdecode_VARDAT * anonym = v;
      if (anonym->igatelen==0U) return 0;
      aprstext_strcp(anonym->raw, 500ul, (unsigned long)anonym->igatepos,
                (unsigned long)anonym->igatelen, digi, 9ul);
      *gate = aprsdecode_ophist0;
      while (*gate && X2C_STRCMP((*gate)->call,9u,digi,9u)) {
         *gate = (*gate)->next;
      }
      if (((*gate==0 || !maptool_vistime((*gate)->lasttime))
                || !aprspos_posvalid((*gate)->lastpos)) || (unsigned char)
                (*gate)->sym.tab<=' ') return 0;
   }
   return 1;
} /* end getgate() */


static void rftracks(const aprsdecode_MONCALL opcall, char * clrimg)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pOPHIST ig;
   float y;
   float x;
   float y00;
   float x0;
   unsigned long lig;
   /* track filter off */
   char trk; /* show no waypoints so show no rfpath to this */
   char nofilt;
   struct aprsdecode_VARDAT * anonym;
   op = aprstext_oppo(opcall);
   aprsdecode_click.ops = op;
   aprsdecode_click.typ = aprsdecode_tRFPATH;
   nofilt = !useri_configon(useri_fTRACKFILT);
   trk = aprsdecode_lums.track>1L;
   if (aprsdecode_click.ops) {
      aprsdecode_click.pf = aprsdecode_click.ops->frames;
      while (aprsdecode_click.pf) {
         if ((trk || aprsdecode_click.pf->next==0)
                && maptool_vistime(aprsdecode_click.pf->time0)) {
            { /* with */
               struct aprsdecode_VARDAT * anonym = aprsdecode_click.pf->vardat;
                
               if (anonym->lastref==aprsdecode_click.pf && (nofilt || !(aprsdecode_click.pf->nodraw&~0x9U)
                )) {
                  if ((getgate(aprsdecode_click.pf->vardat,
                &ig) && maptool_mapxy(anonym->pos, &x0,
                &y00)>=0L) && maptool_mapxy(ig->lastpos, &x, &y)>=-1L) {
                     /*
                                     lig:=fade(click.pf^,time, click.ops);
                                     lig:=lig*240 DIV 1024*(refcnt+1);
                     */
                     if (*clrimg) {
                        maptool_clr(rfimg);
                        *clrimg = 0;
                     }
                     lig = (unsigned long)(60U*(anonym->refcnt+1U));
                     maptool_vector(rfimg, x0, y00, x, y, (long)lig,
                (long)lig, (long)lig, 256UL, 25.0f);
                  }
               }
            }
         }
         aprsdecode_click.pf = aprsdecode_click.pf->next;
      }
   }
} /* end rftracks() */


static void mhtracks(const aprsdecode_MONCALL hcall, char * clrimg)
{
   float y;
   float x;
   float y00;
   float x0;
   unsigned long lig;
   aprsdecode_MONCALL digi;
   aprsdecode_pOPHIST hop;
   /* track filter off */
   char trk; /* show no waypoints so show no rfpath to this */
   char nofilt;
   struct aprsdecode_VARDAT * anonym;
   hop = aprstext_oppo(hcall);
   if (hop==0) return;
   aprsdecode_click.ops = aprsdecode_ophist0;
   aprsdecode_click.typ = aprsdecode_tRFPATH;
   nofilt = !useri_configon(useri_fTRACKFILT);
   trk = aprsdecode_lums.track>1L;
   while (aprsdecode_click.ops) {
      if (((0x2U & aprsdecode_click.ops->drawhints)
                ==0 && (0x8U & aprsdecode_click.ops->drawhints))
                && (unsigned char)aprsdecode_click.ops->sym.tab>' ') {
         aprsdecode_click.pf = aprsdecode_click.ops->frames;
         while (aprsdecode_click.pf) {
            if ((trk || aprsdecode_click.pf->next==0)
                && maptool_vistime(aprsdecode_click.pf->time0)) {
               { /* with */
                  struct aprsdecode_VARDAT * anonym = aprsdecode_click.pf->vardat;
                
                  if (anonym->lastref==aprsdecode_click.pf && (nofilt || !(aprsdecode_click.pf->nodraw&~0x9U)
                )) {
                     if (anonym->igatelen>0U && maptool_mapxy(anonym->pos,
                &x0, &y00)>=0L) {
                        aprstext_strcp(anonym->raw, 500ul,
                (unsigned long)anonym->igatepos,
                (unsigned long)anonym->igatelen, digi, 9ul);
                        if (X2C_STRCMP(hop->call,9u,digi,
                9u)==0 && maptool_mapxy(hop->lastpos, &x, &y)>=-1L) {
                           if (*clrimg) {
                              maptool_clr(rfimg);
                              *clrimg = 0;
                           }
                           lig = (unsigned long)(50U*(anonym->refcnt+1U));
                           maptool_vector(rfimg, x0, y00, x, y, (long)lig,
                (long)lig, (long)lig, 256UL, 25.0f);
                        }
                     }
                  }
               }
            }
            aprsdecode_click.pf = aprsdecode_click.pf->next;
         }
      }
      aprsdecode_click.ops = aprsdecode_click.ops->next;
   }
} /* end mhtracks() */

#define aprsmap_TMIN (-30)

#define aprsmap_TMAX 50

#define aprsmap_INCH 0.254

#define aprsmap_WXNIL 10000.0

static long aprsmap_RED[39] = {33L,63L,112L,156L,199L,47L,104L,156L,255L,
                255L,7L,7L,7L,7L,7L,0L,0L,0L,0L,0L,255L,255L,255L,255L,255L,
                255L,255L,255L,255L,219L,156L,128L,104L,82L,63L,43L,26L,13L,
                4L};

static long aprsmap_GRN[39] = {33L,63L,112L,156L,199L,7L,7L,7L,7L,82L,0L,33L,
                82L,156L,255L,47L,81L,128L,186L,255L,255L,219L,186L,156L,
                128L,104L,82L,47L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L};

static long aprsmap_BLU[39] = {33L,63L,112L,156L,199L,47L,104L,156L,255L,
                255L,156L,255L,255L,255L,255L,0L,0L,0L,0L,0L,7L,7L,7L,7L,7L,
                0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L};

#define aprsmap_MAX24 50.0

#define aprsmap_MAX1 5.0

#define aprsmap_TIMESPAN 3600


static long getrain(char what, aprsdecode_pOPHIST op)
{
   aprsdecode_pFRAMEHIST f;
   struct aprsdecode_DAT dat;
   float max1;
   float r;
   long ret;
   ret = 0L;
   f = op->frames;
   while (f) {
      if ((f->next==0 && f->time0+3600UL>=aprsdecode_systime)
                && aprsdecode_Decode(f->vardat->raw, 500ul, &dat)>=0L) {
         if (what=='1') {
            max1 = 5.0f;
            r = dat.wx.rain1;
         }
         else {
            max1 = 50.0f;
            r = dat.wx.rain24;
         }
         if (r<10000.0f) {
            r = r*0.254f;
            if (r>200.0f) r = 0.0f;
            if (r>max1) r = max1;
            if (r>=0.1f) {
               ret = (long)aprsdecode_trunc((X2C_DIVR(r,max1))*32767.0f);
            }
         }
      }
      f = f->next;
   }
   return ret;
} /* end getrain() */


static long gettemp(aprsdecode_pOPHIST op)
{
   long t;
   t = (long)op->lasttempalt;
   if (t>=-30L && t<=50L || !useri_configon(useri_fTRACKFILT)) {
      /* show or not garbage */
      if (t<-30L) t = -30L;
      else if (t>50L) t = 50L;
      t = X2C_DIV((t-(-30L))*32766L,80L)+1L;
   }
   else t = 0L;
   return t;
} /* end gettemp() */

static long _cnst2[39] = {33L,63L,112L,156L,199L,47L,104L,156L,255L,255L,
                156L,255L,255L,255L,255L,0L,0L,0L,0L,0L,7L,7L,7L,7L,7L,0L,0L,
                0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L};
static long _cnst1[39] = {33L,63L,112L,156L,199L,7L,7L,7L,7L,82L,0L,33L,82L,
                156L,255L,47L,81L,128L,186L,255L,255L,219L,186L,156L,128L,
                104L,82L,47L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L,0L};
static long _cnst0[39] = {33L,63L,112L,156L,199L,47L,104L,156L,255L,255L,7L,
                7L,7L,7L,7L,0L,0L,0L,0L,0L,255L,255L,255L,255L,255L,255L,
                255L,255L,255L,219L,156L,128L,104L,82L,63L,43L,26L,13L,4L};

static void metercolor(char what)
{
   float oor;
   float f1;
   float f;
   float qq;
   float y;
   float x;
   long rbr;
   long radius;
   long q;
   long y00;
   long x0;
   long my;
   long mx;
   long yi;
   long xi;
   long t;
   unsigned long gb1;
   unsigned long gb;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   long tmp;
   long tmp0;
   radius = (long)aprsdecode_trunc(RealMath_power(2.0f,
                maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom)+0.2f));
   if (radius>1000L) radius = 1000L;
   if (what=='1') radius = X2C_DIV(radius,2L);
   rbr = radius*radius;
   oor = X2C_DIVR(1.0f,(float)radius);
   aprsdecode_click.ops = aprsdecode_ophist0;
   aprsdecode_click.typ = aprsdecode_tMETEOCOLOR;
   while (aprsdecode_click.ops) {
      if ((((((aprspos_posvalid(aprsdecode_click.ops->lastpos)
                && (unsigned char)aprsdecode_click.ops->sym.tab>' ')
                && (aprsdecode_lums.obj>0L || (0x2U & aprsdecode_click.ops->drawhints)
                ==0)) && maptool_vistime(aprsdecode_click.ops->lasttime))
                && maptool_mapxy(aprsdecode_click.ops->lastpos, &x,
                &y)>=-1L) && aprsdecode_click.ops->temptime+3600UL>aprsdecode_systime)
                 && ((aprsdecode_click.ops->lastinftyp>=100U || what=='1')
                || what=='R')) {
         x0 = (long)X2C_TRUNCI(x,X2C_min_longint,X2C_max_longint);
         y00 = (long)X2C_TRUNCI(y,X2C_min_longint,X2C_max_longint);
         if (((x0>-radius && x0<maptool_xsize+radius) && y00>-radius)
                && y00<maptool_ysize+radius) {
            /* test for speed */
            if (what=='T') t = gettemp(aprsdecode_click.ops);
            else t = getrain(what, aprsdecode_click.ops);
            if (t>0L) {
               tmp = radius;
               yi = -radius;
               if (yi<=tmp) for (;; yi++) {
                  my = y00+yi;
                  if (my>0L && my<maptool_ysize) {
                     tmp0 = radius;
                     xi = -radius;
                     if (xi<=tmp0) for (;; xi++) {
                        mx = x0+xi;
                        if (mx>0L && mx<maptool_xsize) {
                           q = xi*xi+yi*yi;
                           if (q<rbr) {
                              qq = 1.0f-RealMath_sqrt((float)q)*oor;
                              f = qq*qq;
                              f = f*f;
                              { /* with */
                                 struct maptool_PIX * anonym = &rfimg->Adr[(mx)
                *rfimg->Len0+my];
                                 gb = aprsdecode_trunc(f*1.E+7f)+1UL;
                                 /*                      IF (what="T")
                & (r=0) THEN r:=trunc(FLOAT(t)*(1.0-FLOAT(q)/FLOAT(rbr))); */
                                 if (what=='T' && anonym->r==0U) {
                                    anonym->r = (unsigned short)t;
                                 }
                                 else {
                                    gb1 = (unsigned long)
                anonym->g+(unsigned long)anonym->b*65536UL;
                                    f1 = (float)gb1*1.E-7f;
                                    if (what=='T') {
                                    anonym->r = (unsigned short)(long)
                X2C_TRUNCI(X2C_DIVR((float)t*f+(float)anonym->r*f1,f+f1),
                X2C_min_longint,X2C_max_longint);
                                    }
                                    else {
                                    anonym->r = (unsigned short)(long)
                X2C_TRUNCI(X2C_DIVR((float)t*f*(1.0f-X2C_DIVR((float)q,
                (float)rbr))+(float)anonym->r*f1,f+f1),X2C_min_longint,
                X2C_max_longint);
                                    }
                                    if (anonym->r<=0U) {
                                    anonym->r = 1U;
                                    }
                                    gb += gb1;
                                 }
                                 anonym->g = (unsigned short)((unsigned long)
                gb&0xFFFFUL);
                                 anonym->b = (unsigned short)
                X2C_LSH((unsigned long)gb,32,-16);
                              }
                           }
                        }
                        if (xi==tmp0) break;
                     } /* end for */
                  }
                  if (yi==tmp) break;
               } /* end for */
            }
         }
      }
      /*                    g:=gb MOD 65536; */
      /*                    b:=gb DIV 65536; */
      aprsdecode_click.ops = aprsdecode_click.ops->next;
   }
   tmp = maptool_ysize-1L;
   yi = 0L;
   if (yi<=tmp) for (;; yi++) {
      tmp0 = maptool_xsize-1L;
      xi = 0L;
      if (xi<=tmp0) for (;; xi++) {
         { /* with */
            struct maptool_PIX * anonym0 = &rfimg->Adr[(xi)*rfimg->Len0+yi];
            if (anonym0->r>0U) {
               if (what=='T') {
                  t = (long)(((unsigned long)anonym0->r*80UL)/32768UL);
                  t = X2C_DIV(t,2L);
                  if (t<0L) t = 0L;
                  else if (t>38L) t = 38L;
                  anonym0->r = (unsigned short)(_cnst0[t]*2L);
                  anonym0->g = (unsigned short)(_cnst1[t]*2L);
                  anonym0->b = (unsigned short)(_cnst2[t]*2L);
               }
               else if (what=='1') {
                  anonym0->r = (unsigned short)((((unsigned long)
                anonym0->r*1000UL+1048576UL)/2097152UL)*64UL);
                  anonym0->b = 0U;
                  anonym0->g = 0U;
               }
               else {
                  anonym0->b = (unsigned short)((((unsigned long)
                anonym0->r*1000UL+1048576UL)/2097152UL)*64UL);
                  anonym0->r = 0U;
                  anonym0->g = anonym0->b;
               }
            }
         }
         if (xi==tmp0) break;
      } /* end for */
      if (yi==tmp) break;
   } /* end for */
} /* end metercolor() */


static void revert(void)
{
   aprsdecode_pOPHIST on;
   aprsdecode_pOPHIST oo;
   aprsdecode_pOPHIST o;
   oo = 0;
   o = aprsdecode_ophist0;
   while (o) {
      on = o->next;
      o->next = oo;
      oo = o;
      o = on;
   }
   aprsdecode_ophist0 = oo;
} /* end revert() */

#define aprsmap_TODATE ""

#define aprsmap_TOSTART "\001"

#define aprsmap_BACK "\002"

#define aprsmap_FORW "\003"

#define aprsmap_TOEND "\004"

#define aprsmap_TONOW "\005"

#define aprsmap_RDWLOG "\006"

#define aprsmap_ASKDELLOG "\007"

#define aprsmap_NODELLOG "\010"

#define aprsmap_DELLOG "\011"


static void importlog(char cmd)
{
   unsigned long fromto;
   long span;
   char h1[1025];
   char h[1025];
   char fn[1025];
   char color;
   char ok0;
   h[0U] = 0;
   fn[0U] = 0;
   color = 'e';
   logredcnt = 0UL;
   logstarttime = 0UL;
   /*WrInt(ORD(cmd), 1); WrStrLn(" cmd"); */
   if (cmd=='\011') {
      useri_confstr(useri_fLOGWFN, fn, 1025ul);
      ok0 = 0;
      if (fn[0U]) FileSys_Remove(fn, 1025ul, &ok0);
      if (ok0) {
         strncpy(h,"[",1025u);
         aprsstr_Append(h, 1025ul, fn, 1025ul);
         aprsstr_Append(h, 1025ul, "] deleted", 10ul);
      }
      else {
         strncpy(h,"cannot delete [",1025u);
         aprsstr_Append(h, 1025ul, fn, 1025ul);
         aprsstr_Append(h, 1025ul, "]", 2ul);
      }
   }
   else if (cmd=='\007') {
      useri_confstr(useri_fLOGWFN, fn, 1025ul);
      if (fn[0U]==0) strncpy(h,"no logfile defined",1025u);
      else {
         if (useri_guesssize(fn, 1025ul, h, 1025ul)>=0L) {
            /*
                  h1:="kB"; 
                  IF flen>4000 THEN flen:=flen DIV 1000; h1:="MB" END;
                  IF flen>4000 THEN flen:=flen DIV 1000; h1:="GB" END;
                  IntToStr(flen, 0, h);
                  Append(h, h1);
            */
            aprsstr_Append(fn, 1025ul, "] ", 3ul);
            aprsstr_Append(fn, 1025ul, " ", 2ul);
            aprsstr_Append(fn, 1025ul, h, 1025ul);
            strncpy(h,"Logfile [",1025u);
            aprsstr_Append(h, 1025ul, fn, 1025ul);
         }
         else strncpy(h,"no logfile found",1025u);
         color = 'r';
      }
   }
   else if (cmd!='\010') {
      if (cmd=='\005' || cmd=='\006') {
         if (aprsdecode_lums.logmode) {
            stkpo = 0UL;
            stktop = 0UL;
            aprsdecode_click.mhop[0UL] = 0;
            aprsdecode_click.entries = 0UL;
            aprsdecode_purge(&aprsdecode_ophist0, X2C_max_longcard,
                X2C_max_longcard);
            aprsdecode_ophist0 = aprsdecode_ophist2;
            aprsdecode_ophist2 = 0;
            aprsdecode_lums.logmode = 0;
            aprsdecode_systime = aprsdecode_realtime;
         }
         if (cmd=='\006') {
            useri_confstr(useri_fLOGWFN, fn, 1025ul);
            stkpo = 0UL;
            stktop = 0UL;
            aprsdecode_click.mhop[0UL] = 0;
            aprsdecode_click.entries = 0UL;
            aprsdecode_purge(&aprsdecode_ophist0, X2C_max_longcard,
                X2C_max_longcard);
            aprsdecode_systime = TimeConv_time();
            if (fn[0U]) {
               fromto = aprsdecode_systime-aprsdecode_lums.purgetime;
               if (!rdlog(&aprsdecode_ophist0, fn, 1025ul, &fromto,
                (long)aprsdecode_lums.purgetime, "", 1ul)) {
                  revert();
                  aprsdecode_purge(&aprsdecode_ophist0,
                aprsdecode_systime-aprsdecode_lums.purgetime,
                aprsdecode_systime-aprsdecode_lums.purgetimeobj);
                  aprsdecode_tracenew.winevent = 0UL;
               }
               else {
                  fn[0U] = 0;
                  strncpy(h,"file not found",1025u);
               }
            }
            else strncpy(h,"need filename",1025u);
         }
         fromto = aprsdecode_systime;
      }
      else {
         useri_confstr(useri_fLOGFN, fn, 1025ul);
         if (aprsdecode_lums.logmode) {
            stkpo = 0UL;
            stktop = 0UL;
            aprsdecode_click.mhop[0UL] = 0;
            aprsdecode_click.entries = 0UL;
            aprsdecode_purge(&aprsdecode_ophist0, X2C_max_longcard,
                X2C_max_longcard);
         }
         else {
            aprsdecode_ophist2 = aprsdecode_ophist0;
            aprsdecode_ophist0 = 0;
            aprsdecode_lums.logmode = 1;
         }
         useri_confstr(useri_fLOGDATE, h, 1025ul);
         if (!aprsstr_StrToTime(h, 1025ul, &fromto)) fromto = 0UL;
         else if (fromto>useri_localtime()) fromto -= useri_localtime();
         span = (long)aprsdecode_lums.firstdim;
         if (cmd=='\001') fromto = 0UL;
         else if (cmd=='\002') {
            span = -span;
            if (fromto>aprsdecode_lums.firstdim) {
               fromto -= aprsdecode_lums.firstdim;
            }
            else fromto = 0UL;
         }
         else if (cmd=='\004') fromto = TimeConv_time();
         if (fn[0U]) {
            useri_confstr(useri_fLOGFIND, h, 1025ul);
            if (rdlog(&aprsdecode_ophist0, fn, 1025ul, &fromto, span, h,
                1025ul)) {
               revert();
               aprsdecode_purge(&aprsdecode_ophist0,
                aprsdecode_systime-aprsdecode_lums.purgetime,
                aprsdecode_systime-aprsdecode_lums.purgetime);
            }
            else {
               /*      purge(ophist, systime-lums.purgetime,
                systime-lums.purgetimeobj); */
               /*      IF logstarttime+lums.firstdim<systime THEN logstarttime:=systime-lums.firstdim END;
                 */
               fn[0U] = 0;
               strncpy(h,"file not found",1025u);
            }
            aprsdecode_tracenew.winevent = 0UL;
         }
         else strncpy(h,"need filename",1025u);
      }
      aprsstr_DateToStr(fromto+useri_localtime(), h, 1025ul);
      h[16U] = 0;
      useri_AddConfLine(useri_fLOGDATE, 0U, h, 1025ul);
      if (logredcnt>0UL) {
         aprsstr_IntToStr((long)logredcnt, 1UL, h, 1025ul);
         aprsstr_Append(h, 1025ul, " Lines from ", 13ul);
         aprstext_DateLocToStr(logstarttime, h1, 1025ul);
         aprsstr_Append(h, 1025ul, h1, 1025ul);
         aprsstr_Append(h, 1025ul, " to ", 5ul);
         aprstext_DateLocToStr(aprsdecode_systime, h1, 1025ul);
         aprsstr_Append(h, 1025ul, h1, 1025ul);
         color = 'b';
      }
      else if (logstarttime>0UL) {
         strncpy(h,"no data at ",1025u);
         aprstext_DateLocToStr(fromto+useri_localtime(), h1, 1025ul);
         aprsstr_Append(h, 1025ul, h1, 1025ul);
         if (aprsdecode_systime<fromto) {
            aprsstr_Append(h, 1025ul, " (log end ", 11ul);
            aprstext_DateLocToStr(aprsdecode_systime, h1, 1025ul);
            aprsstr_Append(h, 1025ul, h1, 1025ul);
            aprsstr_Append(h, 1025ul, ")", 2ul);
         }
         else if (logstarttime>0UL) {
            aprsstr_Append(h, 1025ul, " (log start ", 13ul);
            aprstext_DateLocToStr(logstarttime, h1, 1025ul);
            aprsstr_Append(h, 1025ul, h1, 1025ul);
            aprsstr_Append(h, 1025ul, ")", 2ul);
         }
      }
      else if (fn[0U]) strncpy(h,"file not found",1025u);
      if (cmd=='\005') strncpy(h,"back to realtime",1025u);
   }
   useri_textautosize(0L, 0L, 4UL, 0UL, color, h, 1025ul);
   useri_refrlog();
} /* end importlog() */


static void bootreadlog(void)
{
   char fn[1000];
   char s[1000];
   unsigned long logt;
   useri_confstr(useri_fLOGWFN, fn, 1000ul);
   if (!useri_configon(useri_fLOGWFN) || fn[0U]==0) {
      useri_textautosize(0L, 0L, 4UL, 5UL, 'b', "No Log Data", 12ul);
      useri_redraw(image);
   }
   else {
      useri_textautosize(0L, 0L, 4UL, 0UL, 'b', "Read Log", 9ul);
      useri_redraw(image);
      logt = TimeConv_time()-aprsdecode_lums.purgetime;
      if (!rdlog(&aprsdecode_ophist0, fn, 1000ul, &logt,
                (long)aprsdecode_lums.purgetime, "", 1ul)) {
         aprsdecode_realtime = TimeConv_time();
         strncpy(s,"logile \'",1000u);
         aprsstr_Append(s, 1000ul, fn, 1000ul);
         aprsstr_Append(s, 1000ul, "\' not found", 12ul);
         useri_textautosize(0L, 0L, 4UL, 5UL, 'r', s, 1000ul);
         useri_redraw(image);
      }
      else {
         aprsdecode_realtime = TimeConv_time();
         aprsstr_IntToStr((long)logredcnt, 1UL, s, 1000ul);
         aprsstr_Append(s, 1000ul, " lines \'", 9ul);
         aprsstr_Append(s, 1000ul, fn, 1000ul);
         aprsstr_Append(s, 1000ul, "\' imported", 11ul);
         useri_textautosize(0L, 0L, 4UL, 2UL, 'b', s, 1000ul);
         useri_redraw(image);
      }
   }
   revert();
   aprsdecode_purge(&aprsdecode_ophist0,
                TimeConv_time()-aprsdecode_lums.purgetime,
                TimeConv_time()-aprsdecode_lums.purgetimeobj);
   logdone = 1;
} /* end bootreadlog() */


static void markvisable(const aprsdecode_MONCALL singlecall)
{
   aprsdecode_pOPHIST singleop;
   aprsdecode_pOPHIST op;
   struct aprspos_POSITION rightdown;
   struct aprsdecode_OPHIST * anonym;
   singleop = aprstext_oppo(singlecall);
   maptool_xytodeg((float)maptool_xsize, 0.0f, &rightdown);
   op = aprsdecode_ophist0;
   while (op) {
      { /* with */
         struct aprsdecode_OPHIST * anonym = op;
         if (((aprspos_posvalid(anonym->lastpos)
                && maptool_vistime(anonym->lasttime))
                && (op==singleop || (((singleop==0 && anonym->margin0.long0<=rightdown.long0)
                 && anonym->margin0.lat>=rightdown.lat)
                && anonym->margin1.long0>=aprsdecode_mappos.long0)
                && anonym->margin1.lat<=aprsdecode_mappos.lat))
                && (aprsdecode_lums.obj>0L || (0x2U & anonym->drawhints)==0))
                 anonym->drawhints |= 0x8U;
         else anonym->drawhints = anonym->drawhints&~0x18U;
         op = anonym->next;
      }
   }
   aprsdecode_tracenew.winpos0 = aprsdecode_mappos;
   aprsdecode_tracenew.winpos1 = rightdown;
} /* end markvisable() */


static aprsdecode_pOPHIST findop(const char call[], unsigned long call_len,
                char totable)
{
   aprsdecode_pOPHIST opfound;
   aprsdecode_pOPHIST op;
   aprsdecode_MONCALL Mc;
   aprsdecode_MONCALL mc0;
   unsigned long i;
   char c;
   c = 0;
   for (i = 0UL; i<=8UL; i++) {
      if (i<=call_len-1) {
         mc0[i] = call[i];
         if (c!='*') c = X2C_CAP(call[i]);
         if (c=='*') Mc[i] = '?';
         else Mc[i] = c;
      }
      else {
         mc0[i] = 0;
         Mc[i] = 0;
      }
   } /* end for */
   if (totable) {
      aprsdecode_click.entries = 0UL;
      aprsdecode_click.selected = 0UL;
   }
   op = aprsdecode_ophist0;
   while (op) {
      opfound = 0;
      if (X2C_STRCMP(op->call,9u,mc0,9u)==0) opfound = op;
      else {
         i = 0UL;
         for (;;) {
            if (Mc[i]!='?' && Mc[i]!=op->call[i]) break;
            ++i;
            if (i>8UL) {
               opfound = op;
               break;
            }
         }
      }
      if (opfound && maptool_vistime(opfound->lasttime)) {
         if (!totable) return opfound;
         if (aprsdecode_click.entries>9UL) {
            return aprsdecode_click.table[0UL].opf;
         }
         aprsdecode_click.table[aprsdecode_click.entries].opf = opfound;
         aprsdecode_click.table[aprsdecode_click.entries].pff = 0;
         aprsdecode_click.table[aprsdecode_click.entries].pff0 = 0;
         aprsdecode_click.table[aprsdecode_click.entries]
                .typf = aprsdecode_tSYMBOL;
         aprsdecode_click.selected = 0UL;
         ++aprsdecode_click.entries;
      }
      op = op->next;
   }
   if (aprsdecode_click.entries>0UL) return aprsdecode_click.table[0UL].opf;
   else return 0;
   return 0;
} /* end findop() */


static void reset(struct aprspos_POSITION * pos1,
                struct aprspos_POSITION * pos0)
{
   pos0->lat = (-10.0f);
   pos0->long0 = 10.0f;
   pos1->lat = 10.0f;
   pos1->long0 = (-10.0f);
} /* end reset() */


static void max0(struct aprspos_POSITION * pos1,
                struct aprspos_POSITION * pos0, struct aprspos_POSITION p)
{
   if (aprspos_posvalid(p)) {
      if (pos0->lat<p.lat) pos0->lat = p.lat;
      if (pos1->lat>p.lat) pos1->lat = p.lat;
      if (pos0->long0>p.long0) pos0->long0 = p.long0;
      if (pos1->long0<p.long0) pos1->long0 = p.long0;
   }
} /* end max() */


static void findsize(struct aprspos_POSITION * pos0,
                struct aprspos_POSITION * pos1,
                const aprsdecode_MONCALL opcall, char typ)
{
   aprsdecode_pFRAMEHIST f;
   aprsdecode_pVARDAT v;
   aprsdecode_pOPHIST op;
   aprsdecode_pOPHIST ig;
   unsigned long gl;
   unsigned long j;
   unsigned long i;
   struct aprsdecode_DAT dat;
   char nofilt;
   char trk;
   struct aprsdecode_VARDAT * anonym;
   struct aprsdecode_FRAMEHIST * anonym0;
   reset(pos1, pos0);
   op = aprstext_oppo(opcall);
   nofilt = !useri_configon(useri_fTRACKFILT);
   trk = aprsdecode_lums.track>1L;
   if (op) {
      if (typ=='S') {
         *pos0 = op->lastpos;
         *pos1 = *pos0;
      }
      else if (typ=='T' || typ=='R') {
         if (typ=='R') {
            ig = aprsdecode_ophist0;
            while (ig) {
               ig->drawhints = ig->drawhints&~0x18U;
               ig = ig->next;
            }
            op->drawhints |= 0x8U;
         }
         f = op->frames;
         while (f) {
            v = f->vardat;
            if ((trk || f->next==0) && aprspos_posvalid(v->pos)) {
               { /* with */
                  struct aprsdecode_VARDAT * anonym = v;
                  if (maptool_vistime(f->time0)
                && (nofilt || !(f->nodraw&~0x9U))) {
                     max0(pos1, pos0, anonym->pos);
                     if (typ=='R' && getgate(v, &ig)) {
                        ig->drawhints |= 0x8U;
                        max0(pos1, pos0, ig->lastpos);
                     }
                  }
               }
            }
            f = f->next;
         }
      }
      else if (typ=='H') {
         /*
         WrStr(typ); WrFixed(pos0.lat, 4,10);WrFixed(pos0.lat, 4,10);
                WrFixed(pos0.long, 4,10);
         WrFixed(pos1.lat, 4,10); WrFixed(pos1.long, 4,10); WrStr(op^.call);
                WrLn;
         */
         /* mark all heard this op */
         gl = aprsstr_Length(op->call, 9ul);
         if (gl>0UL) {
            ig = aprsdecode_ophist0;
            while (ig) {
               ig->drawhints = ig->drawhints&~0x18U;
               if ((0x2U & ig->drawhints)==0 && maptool_vistime(ig->lasttime)
                ) {
                  f = ig->frames;
                  for (;;) {
                     if (f==0) break;
                     { /* with */
                        struct aprsdecode_FRAMEHIST * anonym0 = f;
                        if (((((trk || f->next==0)
                && maptool_vistime(anonym0->time0))
                && aprspos_posvalid(anonym0->vardat->pos))
                && (nofilt || !(f->nodraw&~0x9U))) && (unsigned long)
                anonym0->vardat->igatelen==gl) {
                           j = (unsigned long)anonym0->vardat->igatepos;
                           i = 0UL;
                           while (anonym0->vardat->raw[j]==op->call[i]) {
                              ++j;
                              ++i;
                              if (i>=gl) {
                                 ig->drawhints |= 0x8U;
                                 max0(pos1, pos0, anonym0->vardat->pos);
                                 goto loop_exit;
                              }
                           }
                        }
                     }
                     f = f->next;
                  }
                  loop_exit:;
                  if ((0x8U & ig->drawhints)) {
                     max0(pos1, pos0, ig->lastpos);
                /* show symbol in map size */
                  }
               }
               ig = ig->next;
            }
            op->drawhints |= 0x8U;
            max0(pos1, pos0, op->lastpos);
         }
      }
   }
   if (typ=='O') {
      /* mark all objects of this op */
      ig = aprsdecode_ophist0;
      while (ig) {
         ig->drawhints = ig->drawhints&~0x18U;
         if (((0x2U & ig->drawhints) && aprspos_posvalid(ig->lastpos))
                && maptool_vistime(ig->lasttime)) {
            f = ig->frames;
            if (f) {
               while (f->next) f = f->next;
               if ((aprsdecode_Decode(f->vardat->raw, 500ul,
                &dat)>=0L && (dat.type==aprsdecode_OBJ || dat.type==aprsdecode_ITEM)
                ) && X2C_STRCMP(dat.objectfrom,9u,opcall,9u)==0) {
                  ig->drawhints |= 0x8U;
                  max0(pos1, pos0, ig->lastpos);
               }
            }
         }
         ig = ig->next;
      }
   }
   if ((((float)fabs(pos0->lat)>1.5707963267949f || (float)fabs(pos1->lat)
                >1.5707963267949f) || (float)fabs(pos0->long0)
                >3.1415926535898f) || (float)fabs(pos1->long0)
                >3.1415926535898f) {
      aprsdecode_posinval(pos0);
      aprsdecode_posinval(pos1);
   }
   maptool_limpos(pos0);
   maptool_limpos(pos1);
/*
WrFixed(pos0.long*180.0/pi, 5,10);
WrFixed(pos0.lat*180.0/pi, 5,10);
WrFixed(pos1.long*180.0/pi, 5,10);
WrFixed(pos1.lat*180.0/pi, 5,10);
WrLn;
*/
} /* end findsize() */


static void pantoop(aprsdecode_pOPHIST op)
{
   struct aprspos_POSITION p;
   struct aprspos_POSITION rightdown;
   float cf;
   float m;
   if (op && aprspos_posvalid(op->lastpos)) {
      maptool_xytodeg((float)maptool_xsize, 0.0f, &rightdown);
      p = op->lastpos;
      cf = (float)aprsdecode_lums.centering*0.005f;
                /* hold obj in percent of image to center */
      m = (rightdown.long0-aprsdecode_mappos.long0)*cf;
      if (p.long0-m<aprsdecode_mappos.long0) {
         aprsdecode_mappos.long0 = p.long0-m;
      }
      else if (p.long0+m>rightdown.long0) {
         aprsdecode_mappos.long0 = (aprsdecode_mappos.long0+p.long0+m)
                -rightdown.long0;
      }
      m = (aprsdecode_mappos.lat-rightdown.lat)*cf;
      if (p.lat+m>aprsdecode_mappos.lat) aprsdecode_mappos.lat = p.lat+m;
      else if (p.lat-m<rightdown.lat) {
         aprsdecode_mappos.lat = ((aprsdecode_mappos.lat+p.lat)-m)
                -rightdown.lat;
      }
      maptool_limpos(&aprsdecode_mappos);
   }
} /* end pantoop() */


static void toggview(void)
{
   /* save view and restore alternate */
   struct VIEW v;
   v = alttabview;
   alttabview.pos = aprsdecode_mappos;
   alttabview.zoom = maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom);
   memcpy(alttabview.mhop,aprsdecode_click.mhop,9u);
   alttabview.rf = aprsdecode_lums.rf;
   alttabview.mhtx0 = mhtx;
   alttabview.wxcol = aprsdecode_lums.wxcol;
   alttabview.lumtext = (unsigned short)aprsdecode_lums.text;
   alttabview.focus = aprsdecode_click.watchmhop;
   if (aprspos_posvalid(v.pos)) {
      aprsdecode_mappos = v.pos;
      aprsdecode_initzoom = (long)aprsdecode_trunc(v.zoom);
      aprsdecode_finezoom = (v.zoom-(float)aprsdecode_initzoom)+1.0f;
      memcpy(aprsdecode_click.mhop,v.mhop,9u);
      aprsdecode_lums.rf = v.rf;
      mhtx = v.mhtx0;
      aprsdecode_lums.wxcol = v.wxcol;
      aprsdecode_lums.text = (long)v.lumtext;
      aprsdecode_click.watchmhop = v.focus;
      aprsdecode_lums.moving = 0;
      if (aprsdecode_click.watchmhop && aprsdecode_click.mhop[0UL]) {
         pantoop(findop(aprsdecode_click.mhop, 9ul, 0));
      }
   }
} /* end toggview() */


static void push(struct aprspos_POSITION pos, float zoom)
{
   unsigned long i;
   unsigned long tmp;
   if (((stkpo==0UL || posstk[stkpo-1UL].pos.lat!=pos.lat)
                || posstk[stkpo-1UL].pos.long0!=pos.long0) || (long)
                X2C_TRUNCI(posstk[stkpo-1UL].zoom*10.0f,X2C_min_longint,
                X2C_max_longint)!=(long)X2C_TRUNCI(zoom*10.0f,
                X2C_min_longint,X2C_max_longint)) {
      /*WrInt(stkpo,10); WrFixed(zoom,5,10);WrFixed(pos.long,5,10);
                WrFixed(pos.lat,5,10);WrStrLn("push");  */
      posstk[stkpo].pos = pos;
      posstk[stkpo].zoom = zoom;
      /*WrFixed(zoom, 10,2); WrStrLn("pu"); */
      memcpy(posstk[stkpo].mhop,aprsdecode_click.mhop,9u);
      posstk[stkpo].rf = aprsdecode_lums.rf;
      posstk[stkpo].mhtx0 = mhtx;
      if (stkpo<19UL) ++stkpo;
      else {
         tmp = stkpo-1UL;
         i = 1UL;
         if (i<=tmp) for (;; i++) {
            posstk[i] = posstk[i+1UL];
            if (i==tmp) break;
         } /* end for */
      }
      if (stktop<stkpo) stktop = stkpo;
   }
} /* end push() */


static void pop(struct aprspos_POSITION * pos, long * iz, float * fz)
{
   if (stkpo>0UL) {
      --stkpo;
      /*
      WrInt(stkpo,10); WrFixed(posstk[stkpo].zoom,10,16);
                WrFixed(posstk[stkpo].pos.long,10,16);
                WrFixed(posstk[stkpo].pos.lat,10,16);
      WrFixed(realzoom(iz, fz),10,16);WrFixed(pos.long,10,16);
                WrFixed(pos.lat,10,16);
      WrStrLn("pop");
      */
      if (((stkpo>0UL && posstk[stkpo].pos.lat==pos->lat)
                && posstk[stkpo].pos.long0==pos->long0) && (long)
                X2C_TRUNCI(posstk[stkpo].zoom*10.0f,X2C_min_longint,
                X2C_max_longint)==(long)X2C_TRUNCI(maptool_realzoom(*iz,
                *fz)*10.0f,X2C_min_longint,X2C_max_longint)) --stkpo;
      *pos = posstk[stkpo].pos;
      *fz = posstk[stkpo].zoom;
      *iz = (long)aprsdecode_trunc(*fz);
      /*    fz:=finezoom-FLOAT(iz)+1.0; */
      *fz = (*fz-(float)*iz)+1.0f;
      /*WrFixed(fz, 10,2); WrInt(iz,5); WrStrLn("po"); */
      memcpy(aprsdecode_click.mhop,posstk[stkpo].mhop,9u);
      aprsdecode_lums.rf = posstk[stkpo].rf;
      mhtx = posstk[stkpo].mhtx0;
   }
   aprsdecode_lums.moving = 0;
   aprsdecode_lums.wxcol = 0;
} /* end pop() */


static long getant(unsigned char a)
{
   return useri_conf2int(a, 0UL, -1000000L, 100000L, -1000000L);
} /* end getant() */


static void closeradio(void)
{
   if (aprsdecode_click.withradio) maptool_closesrtmfile();
   aprsdecode_click.withradio = 0;
   aprsdecode_click.panorama = 0;
} /* end closeradio() */


static void measureline(maptool_pIMAGE img, struct aprspos_POSITION pos0,
                struct aprspos_POSITION pos1, long pos0alt)
{
   float el2;
   float el1;
   float mhz;
   float a2;
   float a1;
   float dist;
   float y1;
   float x1;
   float y00;
   float x0;
   long ret;
   long wave;
   long ant2;
   long ant1;
   char vec;
   char altok;
   char ant1obj;
   char ok0;
   char h[100];
   char s[100];
   if ((((aprspos_posvalid(pos0) && aprspos_posvalid(pos1))
                && (pos0.lat!=pos1.lat || pos0.long0!=pos1.long0))
                && maptool_mapxy(pos0, &x0,
                &y00)>=-1L) && maptool_mapxy(pos1, &x1, &y1)>=-1L) {
      vec = 1;
      ok0 = 0;
      altok = 0;
      if (!aprsdecode_click.withradio) {
         ant1 = getant(useri_fANT1);
         ant2 = getant(useri_fANT2);
         mhz = useri_conf2real(useri_fFRESNELL, 0UL, 0.0f, X2C_max_real,
                0.0f);
         if (mhz>=0.1f) wave = (long)aprsdecode_trunc(X2C_DIVR(3.E+5f,mhz));
         else wave = 0L;
         if ((ant1>-1000000L && ant2>-1000000L)
                && useri_configon(useri_fGEOPROFIL)) {
            ant1obj = (useri_gpsalt(useri_fANT1) && pos0alt<100000L)
                && pos0alt>-10000L; /* take alt from waypoint NN */
            if (ant1obj) ant1 += pos0alt;
            ret = maptool_geoprofile(img, pos0, pos1, (float)wave*0.001f,
                ant1obj, ant1, ant2, &dist, &a1, &a2, &el1, &el2);
            if (ret==0L) {
               ok0 = 1;
               vec = 0;
               altok = 1;
            }
            else if (ret==-1L) {
               strncpy(s,"Radiolink: need altitude data on Marker Positions",
                100u);
            }
            else strncpy(s,"Radiolink: distance too long",100u);
         }
         else {
            dist = aprspos_distance(pos0, pos1)*1000.0f;
            ok0 = 1;
         }
         if (ok0) {
            aprsstr_FixToStr(dist*0.001f, 4UL, s, 100ul);
            aprsstr_Append(s, 100ul, "km ", 4ul);
            if (altok) {
               aprsstr_Append(s, 100ul, "\367", 2ul);
               aprsstr_IntToStr((long)X2C_TRUNCI(a1,X2C_min_longint,
                X2C_max_longint), 0UL, h, 100ul);
               aprsstr_Append(s, 100ul, h, 100ul);
               aprsstr_Append(s, 100ul, "m/", 3ul);
               aprsstr_IntToStr((long)X2C_TRUNCI(a2,X2C_min_longint,
                X2C_max_longint), 0UL, h, 100ul);
               aprsstr_Append(s, 100ul, h, 100ul);
               aprsstr_Append(s, 100ul, "m \376", 4ul);
            }
            aprsstr_Append(s, 100ul, "az:", 4ul);
            aprsstr_FixToStr(aprspos_azimuth(pos0, pos1), 2UL, h, 100ul);
            aprsstr_Append(s, 100ul, h, 100ul);
            aprsstr_Append(s, 100ul, "\177/", 3ul);
            aprsstr_FixToStr(aprspos_azimuth(pos1, pos0), 2UL, h, 100ul);
            aprsstr_Append(s, 100ul, h, 100ul);
            aprsstr_Append(s, 100ul, "\177 ", 3ul);
            if (altok) {
               aprsstr_Append(s, 100ul, "\367ele:", 6ul);
               aprsstr_FixToStr(el1, 2UL, h, 100ul);
               aprsstr_Append(s, 100ul, h, 100ul);
               aprsstr_Append(s, 100ul, "\177/", 3ul);
               aprsstr_FixToStr(el2, 2UL, h, 100ul);
               aprsstr_Append(s, 100ul, h, 100ul);
               aprsstr_Append(s, 100ul, "\177 \376", 4ul);
            }
            if (mhz>=0.1f) {
               aprsstr_FixToStr(32.2f+8.685889638065f*RealMath_ln(dist*0.001f*mhz)
                , 2UL, h, 100ul);
               aprsstr_Append(s, 100ul, h, 100ul);
               aprsstr_Append(s, 100ul, "dBi", 4ul);
            }
            useri_textautosize(0L, 0L, 7UL, 20UL, 'h', s, 100ul);
         }
         else useri_textautosize(0L, 0L, 3UL, 2UL, 'r', s, 100ul);
      }
      if (vec) {
         maptool_vector(img, x0, y00, x1, y1, 20L, 220L, 20L, 350UL, 0.0f);
      }
   }
} /* end measureline() */


static long geobri(void)
{
   return useri_conf2int(useri_fGEOBRIGHTNESS, 0UL, 0L, 100L, 30L);
} /* end geobri() */


static long geocontr(void)
{
   return useri_conf2int(useri_fGEOCONTRAST, 0UL, 0L, 10000L, 0L);
} /* end geocontr() */

#define aprsmap_ALTINVAL (-10000)


static void panorama(maptool_pIMAGE img, struct aprspos_POSITION pos,
                struct aprsdecode_COLTYP col, char * abo)
{
   long ant;
   if (aprspos_posvalid(pos)) {
      ant = getant(useri_fANT1);
      if (ant>-10000L) maptool_Panorama(img, pos, ant, col, abo);
      else {
         useri_textautosize(0L, 0L, 3UL, 2UL, 'r', "Panorama: need Antenna hi\
gth", 29ul);
      }
   }
} /* end panorama() */

#define aprsmap_ALTINVAL0 (-10000)


static void radioimage(maptool_pIMAGE img, struct aprspos_POSITION pos,
                unsigned long colnum, char * abo)
{
   long qual;
   long ant3;
   long ant2;
   long ant1;
   if (aprspos_posvalid(pos)) {
      ant1 = getant(useri_fANT1);
      ant2 = getant(useri_fANT2);
      ant3 = getant(useri_fANT3);
      if (colnum && ant2>-10000L) ant1 = ant2;
      if (ant1>-10000L && ant3>-10000L) {
         qual = (long)useri_confflags(useri_fSRTMCACHE, 0UL);
         maptool_Radiorange(img, pos, ant1, ant3, (unsigned long)geocontr(),
                (unsigned long)(colnum!=0UL), (unsigned long)qual, abo);
      }
      else {
         useri_textautosize(0L, 0L, 3UL, 2UL, 'r', "Radiolink: need Antenna h\
igths", 31ul);
      }
   }
} /* end radioimage() */


static void getgeocol(unsigned char c, long bri, unsigned long dr,
                unsigned long dg, unsigned long db,
                struct aprsdecode_COLTYP * col)
{
   col->r = (unsigned long)X2C_DIV(useri_conf2int(c, 0UL, 0L, 100L,
                (long)dr)*bri,100L);
   col->g = (unsigned long)X2C_DIV(useri_conf2int(c, 1UL, 0L, 100L,
                (long)dg)*bri,100L);
   col->b = (unsigned long)X2C_DIV(useri_conf2int(c, 2UL, 0L, 100L,
                (long)db)*bri,100L);
} /* end getgeocol() */


static void reliefcolors(maptool_pIMAGE img, char color)
{
   unsigned long bri2;
   unsigned long bri;
   long def;
   unsigned long y;
   unsigned long x;
   struct maptool_PIX * anonym;
   unsigned long tmp;
   unsigned long tmp0;
   def = geobri();
   getgeocol(useri_fCOLMARK1, def, 100UL, 0UL, 0UL, &radio.c1);
                /* marker 1 defaults to red */
   getgeocol(useri_fCOLMARK2, def, 0UL, 100UL, 0UL, &radio.c2);
                /* marker 2 defaults to green */
   tmp = img->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = img->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+y];
            if (color) {
               bri = (unsigned long)anonym->r;
               bri2 = (unsigned long)anonym->g;
               anonym->r = (unsigned short)((radio.c1.r*bri+radio.c2.r*bri2)
                /100UL);
               anonym->g = (unsigned short)((radio.c1.g*bri+radio.c2.g*bri2)
                /100UL);
               anonym->b = (unsigned short)((radio.c1.b*bri+radio.c2.b*bri2)
                /100UL);
            }
            else {
               /*        ELSE r:=r*bri DIV 100; g:=r; b:=r END; */
               bri = (unsigned long)X2C_DIV((long)anonym->r*def,100L);
               anonym->r = (unsigned short)bri;
               anonym->g = (unsigned short)bri;
               anonym->b = (unsigned short)bri;
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end reliefcolors() */


static void copypastepos(struct aprspos_POSITION pos)
/* in decimal deg */
{
   char h[101];
   char s[101];
   aprsstr_FixToStr(X2C_DIVR(pos.lat,1.7453292519444E-2f), 6UL, s, 101ul);
   aprsstr_Append(s, 101ul, " ", 2ul);
   aprsstr_FixToStr(X2C_DIVR(pos.long0,1.7453292519444E-2f), 6UL, h, 101ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   useri_copypaste(s, 101ul);
/*
PROCEDURE testlist(op:pOPHIST);
VAR pf:pFRAMEHIST;
BEGIN
  WrStrLn(op^.call);
  pf:=op^.frames;
  WHILE pf<>NIL DO
    WrCard(CAST(CARDINAL, pf^.vardat), 15); WrInt(pf^.vardat^.igatelen, 10);
                WrFixed(pf^.vardat^.pos.long, 4, 10);
                WrFixed(pf^.vardat^.pos.lat, 4, 10); WrLn;
    pf:=pf^.next;
  END;
  WrLn;
END testlist;
*/
} /* end copypastepos() */

#define aprsmap_MARGIN 0.07

#define aprsmap_YMARGIN 0.025


static void mapzoom(struct aprspos_POSITION pos0,
                struct aprspos_POSITION pos1, unsigned long maxz,
                char withmargin)
{
   float wmax;
   float wy;
   float wx;
   float fo;
   struct aprspos_POSITION testpos;
   struct aprspos_POSITION pos2;
   struct aprspos_POSITION mid;
   struct aprspos_POSITION mo;
   char blown;
   char nofit;
   char done;
   long testty;
   long testtx;
   float steps;
   float testshy;
   float testshx;
   if (!aprspos_posvalid(pos0) || !aprspos_posvalid(pos1)) return;
   push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
   wx = pos1.long0-pos0.long0;
   wy = pos0.lat-pos1.lat;
   pos0.lat = pos0.lat+X2C_DIVR(wy*(float)aprsdecode_lums.fontysize,
                (float)maptool_ysize);
   pos1.lat = pos1.lat-X2C_DIVR(wy*(float)aprsdecode_lums.fontysize*0.25f,
                (float)maptool_ysize);
   wy = pos0.lat-pos1.lat;
   wmax = wx;
   if (wmax<wy) wmax = wy;
   if (!withmargin) wmax = 0.0f;
   mid.long0 = (pos1.long0+pos0.long0+wmax*0.07f)*0.5f;
   mid.lat = (pos1.lat+pos0.lat+wmax*0.025f)*0.5f;
   aprsdecode_mappos.long0 = pos0.long0-wmax*0.07f;
   aprsdecode_mappos.lat = pos0.lat+wmax*0.025f;
   pos2.long0 = pos1.long0+wmax*0.07f;
   pos2.lat = pos1.lat-wmax*0.025f;
   /* zoom to fit in integer zoom*/
   aprsdecode_initzoom = (long)maxz;
   aprsdecode_finezoom = 1.0f;
   nofit = 0;
   for (;;) {
      maptool_mercator(aprsdecode_mappos.long0, aprsdecode_mappos.lat,
                aprsdecode_initzoom, &aprsdecode_inittilex,
                &aprsdecode_inittiley, &maptool_shiftx, &maptool_shifty);
      if (aprsdecode_initzoom<=1L) break;
      /* test if map is complete */
      if (useri_configon(useri_fZOOMMISS)) {
         maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                mid, &testpos);
         maptool_mercator(testpos.long0, testpos.lat, aprsdecode_initzoom,
                &testtx, &testty, &testshx, &testshy);
         maptool_loadmap(image, testtx, testty, aprsdecode_initzoom,
                aprsdecode_finezoom, testshx, testshy, &done, &blown,
                useri_configon(useri_fALLOWEXP), 1);
      }
      else done = 1;
      /* test if map is complete */
      if (maptool_mapxy(pos2, &wx, &wy)<0L) nofit = 1;
      else {
         if (done) break;
         nofit = 0;
      }
      /*  IF (mapxy(pos2, wx, wy)>=0) OR (initzoom<=MINZOOM) THEN EXIT END;
                */
      --aprsdecode_initzoom;
   }
   if (aprsdecode_initzoom!=(long)maxz) tooltips('b');
   /* fine zoom */
   aprsdecode_finezoom = 1.0f;
   /*IF initzoom<MAXZOOMOBJ THEN */
   if (nofit) {
      steps = useri_conf2real(useri_fZOOMSTEP, 0UL, 0.05f, 1.0f, 0.05f);
      for (;;) {
         fo = aprsdecode_finezoom;
         aprsdecode_finezoom = aprsdecode_finezoom+steps;
         maptool_mercator(aprsdecode_mappos.long0, aprsdecode_mappos.lat,
                aprsdecode_initzoom, &aprsdecode_inittilex,
                &aprsdecode_inittiley, &maptool_shiftx, &maptool_shifty);
         if (aprsdecode_finezoom>1.95f || maptool_mapxy(pos2, &wx, &wy)<0L) {
            break;
         }
      }
      aprsdecode_finezoom = fo;
   }
   mo = aprsdecode_mappos;
   maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                mid, &aprsdecode_mappos);
   if (aprsdecode_mappos.lat<mo.lat) aprsdecode_mappos.lat = mo.lat;
} /* end mapzoom() */


static void drawsquer(maptool_pIMAGE img)
{
   float y1;
   float x1;
   float y00;
   float x0;
   struct aprsdecode_COLTYP col;
   if (((aprspos_posvalid(aprsdecode_click.squerpos0)
                && aprspos_posvalid(aprsdecode_click.squerpos1))
                && maptool_mapxy(aprsdecode_click.squerpos0, &x0,
                &y1)>=-1L) && maptool_mapxy(aprsdecode_click.squerpos1, &x1,
                &y00)>=-1L) {
      col.r = 0UL;
      col.g = 60UL;
      col.b = 0UL;
      maptool_area(img, (long)X2C_TRUNCI(x0,X2C_min_longint,X2C_max_longint),
                 (long)X2C_TRUNCI(y00,X2C_min_longint,X2C_max_longint),
                (long)X2C_TRUNCI(x1,X2C_min_longint,X2C_max_longint),
                (long)X2C_TRUNCI(y1,X2C_min_longint,X2C_max_longint), col,
                1);
   }
} /* end drawsquer() */

#define aprsmap_R 300

#define aprsmap_G 300

#define aprsmap_B 200

#define aprsmap_W 350


static void drawzoomsquer(maptool_pIMAGE img)
{
   float y1;
   float x1;
   float y;
   float x;
   if (aprsdecode_click.zoomtox>=0L) {
      x = (float)aprsdecode_click.x;
      y = (float)aprsdecode_click.y;
      x1 = (float)aprsdecode_click.zoomtox;
      y1 = (float)aprsdecode_click.zoomtoy;
      maptool_vector(img, x, y, x1, y, 300L, 300L, 200L, 350UL, 0.0f);
      maptool_vector(img, x1, y, x1, y1, 300L, 300L, 200L, 350UL, 0.0f);
      maptool_vector(img, x1, y1, x, y1, 300L, 300L, 200L, 350UL, 0.0f);
      maptool_vector(img, x, y1, x, y, 300L, 300L, 200L, 350UL, 0.0f);
   }
/*    col.r:=60; col.g:=60; col.b:=100; */
/*    area(image, click.x, click.y, click.zoomtox, click.zoomtoy, col, TRUE);
                 */
} /* end drawzoomsquer() */

#define aprsmap_FIELD 7.272205216643E-4


static char qth(char loc[], unsigned long loc_len)
{
   struct aprspos_POSITION pos1;
   struct aprspos_POSITION pos0;
   char qth_ret;
   X2C_PCOPY((void **)&loc,loc_len);
   maptool_loctopos(&pos0, loc, loc_len);
   if (!aprspos_posvalid(pos0)) {
      qth_ret = 0;
      goto label;
   }
   aprstext_setmark1(pos0, 1, X2C_max_longint, 0UL);
   /*  click.markpos:=pos0; click.marktime:=0; click.markalti:=MAX(INTEGER);
                */
   pos1.lat = pos0.lat-7.272205216643E-4f;
   pos0.lat = pos0.lat+7.272205216643E-4f;
   pos1.long0 = pos0.long0+1.4544410433286E-3f;
   pos0.long0 = pos0.long0-1.4544410433286E-3f;
   maptool_limpos(&pos0);
   maptool_limpos(&pos1);
   mapzoom(pos0, pos1, (unsigned long)useri_conf2int(useri_fDEFZOOM, 0UL, 1L,
                 18L, 14L), 1);
   maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.markpos, &aprsdecode_mappos);
   aprsdecode_click.squerpos0.lat = aprsdecode_click.markpos.lat+3.6361026083215E-4f;
   aprsdecode_click.squerpos1.lat = aprsdecode_click.markpos.lat-3.6361026083215E-4f;
   aprsdecode_click.squerpos0.long0 = aprsdecode_click.markpos.long0-7.272205216643E-4f;
   aprsdecode_click.squerpos1.long0 = aprsdecode_click.markpos.long0+7.272205216643E-4f;
   maptool_limpos(&aprsdecode_click.squerpos0);
   maptool_limpos(&aprsdecode_click.squerpos1);
   qth_ret = 1;
   label:;
   X2C_PFREE(loc);
   return qth_ret;
} /* end qth() */


static void midscreenpos(struct aprspos_POSITION * pos)
{
   maptool_xytodeg((float)maptool_xsize*0.5f, (float)maptool_ysize*0.5f,
                pos);
} /* end midscreenpos() */


static void zoominout(char in, char fine)
{
   float fz;
   float z;
   struct aprspos_POSITION mid;
   long maxz;
   z = maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom);
   /*IF NOT fine THEN push(mappos, z) END; */
   /*push(mappos, z); */
   midscreenpos(&mid);
   mid.lat = mid.lat+(-6.E-7f);
   if (fine) {
      fz = useri_conf2real(useri_fZOOMSTEP, 0UL, 0.0f, 1.0f, 0.1f);
      if (in) aprsdecode_finezoom = aprsdecode_finezoom+fz;
      else aprsdecode_finezoom = aprsdecode_finezoom-fz;
      if (aprsdecode_finezoom<1.0f && aprsdecode_finezoom>1.0f-fz*0.5f) {
         aprsdecode_finezoom = 1.0f;
      }
      if (aprsdecode_finezoom<1.0f) {
         if (aprsdecode_initzoom>0L) {
            --aprsdecode_initzoom;
            aprsdecode_finezoom = 2.0f-fz;
         }
         else aprsdecode_finezoom = 1.0f;
      }
      else if (aprsdecode_finezoom>=2.0f-fz*0.5f) {
         ++aprsdecode_initzoom;
         aprsdecode_finezoom = 1.0f;
      }
   }
   else {
      if (in) {
         ++aprsdecode_initzoom;
         if (aprsdecode_finezoom>1.75f) ++aprsdecode_initzoom;
      }
      else if (aprsdecode_finezoom<1.25f) --aprsdecode_initzoom;
      aprsdecode_finezoom = 1.0f;
   }
   if (z==(float)aprsdecode_trunc(z)) push(aprsdecode_mappos, z);
   maxz = useri_conf2int(useri_fMAXZOOM, 0UL, 1L, 18L, 18L);
   if (aprsdecode_initzoom<1L) {
      aprsdecode_initzoom = 1L;
      aprsdecode_finezoom = 1.0f;
   }
   else if (aprsdecode_initzoom>=maxz) {
      aprsdecode_initzoom = maxz;
      aprsdecode_finezoom = 1.0f;
   }
   z = maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom);
   maptool_shiftmap(X2C_DIV(maptool_xsize,2L), X2C_DIV(maptool_ysize,2L),
                maptool_ysize, z, &mid);
   aprsdecode_mappos = mid;
} /* end zoominout() */


static void find(void)
{
   char h[201];
   aprsdecode_MONCALL hm;
   char err;
   aprsdecode_pOPHIST op;
   struct aprspos_POSITION pos1;
   struct aprspos_POSITION pos;
   useri_confstr(useri_fFIND, h, 201ul);
   if (!qth(h, 201ul)) {
      /* not a locator */
      aprstext_degtopos(h, 201ul, &pos); /* DDMM.MMNDDDMM.MME */
      if (!aprspos_posvalid(pos)) aprstext_deghtopos(h, 201ul, &pos);
      if (!aprspos_posvalid(pos)) aprstext_degdeztopos(h, 201ul, &pos);
      if (!aprspos_posvalid(pos)) maptool_POIfind(&pos, h, 201ul);
      if (aprspos_posvalid(pos)) {
         /* lat / long */
         aprsdecode_click.mhop[0UL] = 0;
         aprstext_setmark1(pos, 1, X2C_max_longint, 0UL);
         /*    click.markpos:=pos;  click.marktime:=0;
                click.markalti:=MAX(INTEGER);     */
         maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                pos, &aprsdecode_mappos);
         pandone = 0;
         useri_textautosize(0L, 0L, 3UL, 4UL, 'b', "marker set", 11ul);
      }
      else {
         /* object name */
         op = findop(h, 201ul, 1);
         if (op) {
            aprsdecode_click.mhop[0UL] = 0;
            useri_mainpop();
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            if (aprsdecode_click.entries>0UL && aprsdecode_click.table[0UL]
                .opf) {
               op = aprstext_oppo(aprsdecode_click.table[0UL].opf->call);
               if (op) memcpy(aprsdecode_click.mhop,op->call,9u);
            }
            mhtx = aprsmap_OPSENT;
            pandone = 0;
            aprsdecode_lums.rf = 0L;
            if (op) {
               err = 0;
               aprsstr_Assign(h, 201ul, op->call, 9ul);
               if (!aprspos_posvalid(op->lastpos)) {
                  err = 1;
                  aprsstr_Append(h, 201ul, " No valid Position!", 20ul);
               }
               if ((unsigned char)op->sym.tab<' ') {
                  err = 1;
                  aprsstr_Append(h, 201ul, " No valid Symbol!", 18ul);
                  if (aprspos_posvalid(op->lastpos)) {
                     /* set marker instead of missing symbol */
                     aprstext_setmark1(op->lastpos, 0, X2C_max_longint, 0UL);
                  }
               }
               /*              click.markpos:=op^.lastpos; */
               /*              click.marktime:=realtime; */
               /*              click.markalti:=MAX(INTEGER); */
               if (err) useri_textautosize(0L, 0L, 3UL, 0UL, 'b', h, 201ul);
            }
         }
         else {
            aprsstr_Assign(hm, 9ul, h, 201ul);
            findsize(&pos, &pos1, hm, 'O');
            if (aprspos_posvalid(pos)) {
               push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
               memcpy(aprsdecode_click.mhop,hm,9u);
               mhtx = aprsmap_OPOBJ;
               pandone = 0;
            }
            else {
               useri_textautosize(0L, 0L, 3UL, 10UL, 'e', "not found!",
                11ul);
            }
         }
      }
   }
} /* end find() */


static void internstat(void)
{
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST fr;
   unsigned long bytec;
   unsigned long varc;
   unsigned long frc;
   unsigned long opc;
   unsigned long newest;
   unsigned long oldest;
   char s[10001];
   char h[31];
   unsigned long ut;
   unsigned long i;
   newest = 0UL;
   oldest = X2C_max_longcard;
   opc = 0UL;
   frc = 0UL;
   varc = 0UL;
   bytec = 0UL;
   op = aprsdecode_ophist0;
   while (op) {
      ++opc;
      fr = op->frames;
      while (fr) {
         ++frc;
         if (fr->time0<oldest) oldest = fr->time0;
         if (fr->time0>newest) newest = fr->time0;
         if (fr->vardat->lastref==fr) {
            ++varc;
            bytec += aprsstr_Length(fr->vardat->raw, 500ul);
         }
         fr = fr->next;
      }
      op = op->next;
   }
   aprsstr_Assign(s, 10001ul, "System Stat", 12ul);
   ut = TimeConv_time();
   if (ut>uptime) {
      aprsstr_Append(s, 10001ul, "\012Uptime:", 9ul);
      aprsstr_TimeToStr(ut-uptime, h, 31ul);
      aprsstr_Append(s, 10001ul, h, 31ul);
   }
   aprsstr_Append(s, 10001ul, "\012Objects:", 10ul);
   aprsstr_IntToStr((long)opc, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsstr_Append(s, 10001ul, "\012Frames:", 9ul);
   aprsstr_IntToStr((long)frc, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsstr_Append(s, 10001ul, "\012Different Frames:", 19ul);
   aprsstr_IntToStr((long)varc, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsstr_Append(s, 10001ul, "\012Rawdata Bytes:", 16ul);
   aprsstr_IntToStr((long)bytec, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   if (frc>0UL) {
      aprsstr_Append(s, 10001ul, "\012Compressed to:", 16ul);
      aprsstr_IntToStr((long)((100UL*varc)/frc), 1UL, h, 31ul);
      aprsstr_Append(s, 10001ul, h, 31ul);
      aprsstr_Append(s, 10001ul, "%", 2ul);
   }
   if (newest>0UL) {
      aprsstr_Append(s, 10001ul, "\012Oldest:", 9ul);
      aprstext_DateLocToStr(oldest, h, 31ul);
      aprsstr_Append(s, 10001ul, h, 31ul);
      aprsstr_Append(s, 10001ul, "\012Newest:", 9ul);
      aprstext_DateLocToStr(newest, h, 31ul);
      aprsstr_Append(s, 10001ul, h, 31ul);
   }
   aprsstr_Append(s, 10001ul, "\012Heap Rawdata:", 15ul);
   aprsstr_IntToStr((long)useri_debugmem.mon, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsstr_Append(s, 10001ul, "\012Heap Screen: ", 15ul);
   aprsstr_IntToStr((long)useri_debugmem.screens, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsstr_Append(s, 10001ul, "\012Heap Menus:  ", 15ul);
   aprsstr_IntToStr((long)useri_debugmem.menus, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsstr_Append(s, 10001ul, "\012Srtm Cache:  ", 15ul);
   aprsstr_IntToStr((long)useri_debugmem.srtm, 1UL, h, 31ul);
   aprsstr_Append(s, 10001ul, h, 31ul);
   aprsdecode_tcpconnstat(s, 10001ul);
   for (i = 0UL; i<=3UL; i++) {
      aprsdecode_udpconnstat(i, s, 10001ul);
   } /* end for */
   useri_textautosize(0L, 0L, 4UL, 0UL, 'b', s, 10001ul);
} /* end internstat() */


static void setshowall(void)
{
   aprsdecode_click.mhop[0UL] = 0;
   aprsdecode_click.watchmhop = 0;
   aprsdecode_lums.wxcol = 0;
   useri_textautosize(0L, 0L, 4UL, 2UL, 'g', "Show All", 9ul);
} /* end setshowall() */


static void View(unsigned long n)
{
   float z;
   struct aprspos_POSITION mid;
   struct aprspos_POSITION pos;
   char h[101];
   char s[101];
   if (xosi_Shift) {
      z = maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom);
      aprsstr_FixToStr(z+0.05f, 2UL, s, 101ul);
      midscreenpos(&mid);
      aprsstr_Append(s, 101ul, " ", 2ul);
      aprstext_postostr(mid, '2', h, 101ul);
      aprsstr_Append(s, 101ul, h, 101ul);
      useri_setview((long)n, s, 101ul);
      useri_killallmenus();
      useri_textautosize(0L, 0L, 4UL, 4UL, 'r', "View stored!", 13ul);
   }
   else {
      z = 0.0f;
      aprsdecode_posinval(&pos);
      useri_getview(useri_fVIEW, n, &z, &pos);
      push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
      if (z!=0.0f) {
         midscreenpos(&mid);
         mid.lat = mid.lat+(-6.E-7f);
         aprsdecode_initzoom = (long)aprsdecode_trunc(z);
         aprsdecode_finezoom = (1.0f+z)-(float)aprsdecode_initzoom;
         maptool_shiftmap(X2C_DIV(maptool_xsize,2L), X2C_DIV(maptool_ysize,
                2L), maptool_ysize, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom), &mid);
         aprsdecode_mappos = mid;
      }
      if (aprspos_posvalid(pos)) {
         maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                pos, &aprsdecode_mappos);
      }
      pandone = 0;
      if (aprsdecode_click.mhop[0UL]) setshowall();
   }
} /* end View() */


static void follow(void)
{
   aprsdecode_pOPHIST op;
   aprsdecode_tracenew.winevent = 0UL;
   op = findop(aprsdecode_tracenew.call, 9ul, 0);
   if (op) {
      /*
      WrInt(ORD(tracenew.follow), 2);
      WrInt(ORD(click.mhop[0]=0C), 2);
      WrInt(ORD(click.mhop=op^.call), 2);
      WrInt(ORD(tracenew.beep), 2);
      WrStrLn(" follow");
      */
      /*    IF tracenew.follow & (click.mhop[0]=0C) OR (click.mhop=op^.call)
                THEN pantoop(op) END; */
      if (aprsdecode_tracenew.follow && (aprsdecode_click.mhop[0UL]
                ==0 || X2C_STRCMP(aprsdecode_click.mhop,9u,op->call,9u)==0)) {
         pantoop(op);
      }
      if (aprsdecode_tracenew.beep && useri_configon(useri_fBEEPWATCH)) {
         aprsdecode_beeplim(100L,
                (unsigned long)useri_conf2int(useri_fBEEPWATCH, 0UL, 20L,
                8000L, 800L), (unsigned long)useri_conf2int(useri_fBEEPWATCH,
                 1UL, 0L, 5000L, 100L));
      }
      clickwatchpos = op->lastpos;
      useri_popwatchcall(aprsdecode_tracenew.call, 9ul);
      if (aprsdecode_tracenew.follow) aprsdecode_click.markpos = op->lastpos;
      maptrys = 30UL;
   }
   aprsdecode_tracenew.call[0UL] = 0;
} /* end follow() */


static void MapPackage(void)
{
   struct aprspos_POSITION rd;
   struct aprspos_POSITION lu;
   char s1[1000];
   char s[1000];
   maptool_xytodeg(0.0f, (float)maptool_ysize, &lu);
   maptool_xytodeg((float)maptool_xsize, 0.0f, &rd);
   useri_textautosize(0L, 0L, 16UL, 0UL, 'b', "Calculating Sizes", 18ul);
   useri_redraw(image);
   maptool_StartMapPackage(lu, rd, useri_conf2int(useri_fDOWNLOADZOOM, 0UL,
                1L, 18L, 6L), 1);
   strncpy(s,"Left up     : ",1000u);
   aprstext_postostr(maptool_mappack.leftup, '2', s1, 1000ul);
   aprsstr_Append(s, 1000ul, s1, 1000ul);
   aprsstr_Append(s, 1000ul, "\012Right down: ", 14ul);
   aprstext_postostr(maptool_mappack.rightdown, '2', s1, 1000ul);
   aprsstr_Append(s, 1000ul, s1, 1000ul);
   aprsstr_Append(s, 1000ul, "\012From Zoom 1 to ", 17ul);
   aprsstr_IntToStr(maptool_mappack.tozoom, 1UL, s1, 1000ul);
   aprsstr_Append(s, 1000ul, s1, 1000ul);
   if (maptool_mappack.overflow) {
      aprsstr_Append(s, 1000ul, "\012Too Many Tiles, split into more Download\
s!", 44ul);
   }
   else {
      aprsstr_Append(s, 1000ul, "\012Total Tiles     : ", 20ul);
      aprsstr_IntToStr((long)maptool_mappack.mapscnt, 1UL, s1, 1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      aprsstr_Append(s, 1000ul, "\012Download Tiles: ", 18ul);
      aprsstr_IntToStr((long)maptool_mappack.needcnt, 1UL, s1, 1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      aprsstr_Append(s, 1000ul, "\012Estimated ", 12ul);
      aprsstr_FixToStr((float)maptool_mappack.needcnt*0.018f, 3UL, s1,
                1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      aprsstr_Append(s, 1000ul, "MByte", 6ul);
   }
   useri_killmenuid(16UL);
   if (maptool_mappack.needcnt>0UL) {
      useri_textautomenu(0L, 0L, 16UL, 0UL, 'r', s, 1000ul, "        Start Do\
wnload", 23ul, "\242", 2ul);
   }
   else useri_textautosize(0L, 0L, 16UL, 0UL, 'b', s, 1000ul);
} /* end MapPackage() */


static void zoomtomarks(struct aprspos_POSITION mpos,
                struct aprspos_POSITION clickpos)
{
   float h;
   if (aprspos_posvalid(mpos) && aprspos_posvalid(clickpos)) {
      if (mpos.lat<clickpos.lat) {
         h = mpos.lat;
         mpos.lat = clickpos.lat;
         clickpos.lat = h;
      }
      if (mpos.long0>clickpos.long0) {
         h = mpos.long0;
         mpos.long0 = clickpos.long0;
         clickpos.long0 = h;
      }
      mapzoom(mpos, clickpos, (unsigned long)useri_conf2int(useri_fMAXZOOM,
                0UL, 1L, 18L, 18L), 1);
   }
} /* end zoomtomarks() */

#define aprsmap_TOSMALL 5


static void zoomtosquare(void)
{
   struct aprspos_POSITION p2;
   struct aprspos_POSITION p1;
   if (aprsdecode_click.zoomtox>=0L && (labs(aprsdecode_click.x-aprsdecode_click.zoomtox)
                >5L || labs(aprsdecode_click.y-aprsdecode_click.zoomtoy)>5L))
                 {
      maptool_xytodeg((float)aprsdecode_click.x, (float)aprsdecode_click.y,
                &p1);
      maptool_xytodeg((float)aprsdecode_click.zoomtox,
                (float)aprsdecode_click.zoomtoy, &p2);
      zoomtomarks(p1, p2);
   }
   aprsdecode_click.zoomtox = -1L; /* zoom done */
} /* end zoomtosquare() */

#define aprsmap_TEXTH 9


static void highlight(void)
{
   unsigned long entc;
   long texth;
   struct aprsdecode_OPHIST * anonym;
   entc = aprsdecode_click.entries;
   if (entc>0UL) {
      --entc;
      if (aprsdecode_click.table[entc].opf) {
         texth = (long)(aprsdecode_lums.fontysize-3UL);
         { /* with */
            struct aprsdecode_OPHIST * anonym = aprsdecode_click.table[entc]
                .opf;
            if ((long)abs(anonym->textpos-anonym->valuepos)<texth) {
               if (anonym->textpos<anonym->valuepos) {
                  anonym->valuepos = (signed char)((long)
                anonym->textpos+texth);
               }
               else {
                  anonym->valuepos = (signed char)((long)
                anonym->textpos-texth);
               }
            }
         }
         if ((aprsdecode_click.table[entc]
                .typf==aprsdecode_tSYMBOL || aprsdecode_click.table[entc]
                .typf==aprsdecode_tTEXT)
                || aprsdecode_click.table[entc].typf==aprsdecode_tDEGREE) {
            symbols(aprsdecode_click.table[entc].opf, 0, 1);
            text(aprsdecode_click.table[entc].opf, 0, 1, 1, 1);
         }
         else if ((aprsdecode_click.table[entc]
                .typf==aprsdecode_tOBJECT || aprsdecode_click.table[entc]
                .typf==aprsdecode_tOBJECTTEXT)
                || aprsdecode_click.table[entc].typf==aprsdecode_tDEGREEOBJ) {
            symbols(aprsdecode_click.table[entc].opf, 1, 1);
            text(aprsdecode_click.table[entc].opf, 1, 1, 1, 1);
         }
         else if ((aprsdecode_click.table[entc]
                .typf==aprsdecode_tTRACK || aprsdecode_click.table[entc]
                .typf==aprsdecode_tRFPATH)
                || aprsdecode_click.table[entc].typf==aprsdecode_tKMH) {
            tracks(image, aprsdecode_click.table[entc].opf, 1,
                X2C_max_longcard);
            symbols(aprsdecode_click.table[entc].opf, 0, 1);
            text(aprsdecode_click.table[entc].opf, 0, 1, 1, 1);
         }
      }
   }
/*
        click.table[entc].opf^.tracklited:=TRUE;
*/
} /* end highlight() */


static void screenshot(void)
{
   char hh[1000];
   char h[1000];
   char s[1000];
   char c;
   unsigned long n;
   unsigned long j;
   unsigned long i;
   long ok0;
   useri_confstr(useri_fFOTOFN, s, 1000ul);
   s[999U] = 0;
   memcpy(h,s,1000u);
   i = 0UL;
   while (s[i] && s[i]!='%') ++i;
   if (s[i]=='%') {
      if (s[i+1UL]=='t') {
         /* insert date in filename */
         s[i] = 0;
         aprsstr_DateToStr(TimeConv_time()+useri_localtime(), hh, 1000ul);
         n = 0UL;
         while (hh[n]) {
            if ((unsigned char)hh[n]<'0' || (unsigned char)hh[n]>'9') {
               hh[n] = '-';
            }
            ++n;
         }
         aprsstr_Append(s, 1000ul, hh, 1000ul);
         i += 2UL;
         while (h[i]) {
            aprsstr_Append(s, 1000ul, (char *) &h[i], 1u/1u);
            ++i;
         }
      }
      else if (s[i+1UL]=='n') {
         /* insert serial number until new file */
         n = 0UL;
         do {
            s[i] = (char)((n/100UL)%10UL+48UL);
            s[i+1UL] = (char)((n/10UL)%10UL+48UL);
            s[i+2UL] = (char)(n%10UL+48UL);
            s[i+3UL] = 0;
            j = i+2UL;
            while (h[j]) {
               aprsstr_Append(s, 1000ul, (char *) &h[j], 1u/1u);
               ++j;
            }
            ++n;
         } while (!(n>999UL || !FileSys_Exists(s, 1000ul)));
      }
   }
   if (s[0]) {
      useri_killallmenus(); /* dump panorama image pointer */
      if (useri_panoimage) {
         ok0 = maptool_saveppm(s, 1000ul, useri_panoimage,
                (long)((useri_panoimage->Len1-1)+1UL),
                (long)((useri_panoimage->Len0-1)+1UL));
      }
      else {
         ok0 = maptool_saveppm(s, 1000ul, image, maptool_xsize,
                maptool_ysize);
      }
      if (ok0<0L) {
         aprsstr_Append(s, 1000ul, " write error", 13ul);
         c = 'e';
      }
      else {
         aprsstr_Append(s, 1000ul, " saved", 7ul);
         c = 'b';
      }
      useri_textautosize(0L, 0L, 6UL, 0UL, c, s, 1000ul);
   }
   else useri_textautosize(0L, 0L, 6UL, 0UL, 'e', "no filename", 12ul);
} /* end screenshot() */

typedef unsigned long sCHGEN[2];

static sCHGEN aprsmap_CHGEN[13] = {{0xA318C62EUL,0x00000003UL}
                ,{0x884214C4UL,0x00000003UL},{0xC444422EUL,0x00000007UL}
                ,{0xA107420EUL,0x00000003UL},{0x11F4A521UL,0x00000002UL}
                ,{0xA3083C3FUL,0x00000003UL},{0xA3178444UL,0x00000003UL}
                ,{0x8422221FUL,0x00000000UL},{0xA317462EUL,0x00000003UL}
                ,{0xA10F462EUL,0x00000003UL},{0x40000000UL,0x00000000UL}
                ,{0x02000400UL,0x00000000UL},{0x00000000UL,0x00000000UL}};

#define aprsmap_POSX 4

#define aprsmap_POSY 18

#define aprsmap_dimmlev 51200.0

static sCHGEN _cnst3[13] = {{0xA318C62EUL,0x00000003UL}
                ,{0x884214C4UL,0x00000003UL},{0xC444422EUL,0x00000007UL}
                ,{0xA107420EUL,0x00000003UL},{0x11F4A521UL,0x00000002UL}
                ,{0xA3083C3FUL,0x00000003UL},{0xA3178444UL,0x00000003UL}
                ,{0x8422221FUL,0x00000000UL},{0xA317462EUL,0x00000003UL}
                ,{0xA10F462EUL,0x00000003UL},{0x40000000UL,0x00000000UL}
                ,{0x02000400UL,0x00000000UL},{0x00000000UL,0x00000000UL}};

static void drawtime(maptool_pIMAGE img, unsigned long t, long fast)
{
   char s[32];
   unsigned long ch;
   struct aprsdecode_COLTYP col;
   unsigned long l;
   unsigned long px;
   unsigned long y;
   unsigned long x;
   unsigned long i;
   float lum;
   struct maptool_PIX * anonym;
   unsigned long tmp;
   aprsstr_DateToStr(t+useri_localtime(), s, 32ul);
   s[16U] = 0; /* strip off seconds */
   if (fast<0L) fast = 0L;
   else if (fast>200L) fast = 200L;
   fast = fast*4L;
   col.r = 1000UL;
   col.g = (unsigned long)(1000L-fast);
   col.b = (unsigned long)(200L+fast);
   l = 0UL;
   i = 0UL;
   px = 4UL;
   while ((unsigned char)s[i]>0) {
      ch = (unsigned long)(unsigned char)s[i];
      if (ch>=48UL && ch<=57UL) ch -= 48UL;
      else if (ch==46UL) ch = 10UL;
      else if (ch==58UL) ch = 11UL;
      else ch = 12UL;
      if (ch<=12UL) {
         s[l] = (char)ch;
         ++l;
      }
      if (ch<=9UL) px += 12UL;
      else px += 5UL;
      ++i;
   }
   for (y = 19UL; y>=3UL; y--) {
      tmp = px;
      x = 4UL;
      if (x<=tmp) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+y];
            lum = (float)((unsigned long)anonym->r*87UL+(unsigned long)
                anonym->g*140UL+(unsigned long)anonym->b*28UL);
            if (lum>51200.0f) {
               lum = X2C_DIVR(51200.0f,lum);
               anonym->r = (unsigned short)aprsdecode_trunc((float)
                anonym->r*lum);
               anonym->g = (unsigned short)aprsdecode_trunc((float)
                anonym->g*lum);
               anonym->b = (unsigned short)aprsdecode_trunc((float)
                anonym->b*lum);
            }
         }
         if (x==tmp) break;
      } /* end for */
   } /* end for */
   px = 4UL;
   i = 0UL;
   while (i<l) {
      ch = (unsigned long)(unsigned char)s[i];
      for (y = 0UL; y<=6UL; y++) {
         for (x = 0UL; x<=4UL; x++) {
            if (X2C_INL(x+y*5UL,35,_cnst3[ch])) {
               maptool_waypoint(img, (float)(x*2UL+px+5UL)-(float)y*0.25f,
                (float)(18UL-y*2UL), 1.4f, (long)col.r, (long)col.g,
                (long)col.b);
            }
         } /* end for */
      } /* end for */
      if (ch<=9UL) px += 12UL;
      else px += 5UL;
      ++i;
   }
} /* end drawtime() */


static void nearwaypoint(void)
{
   /* find nearest waypoint on click to track */
   struct aprspos_POSITION oldpos;
   struct aprspos_POSITION clickpos;
   aprsdecode_pFRAMEHIST pf;
   float d1;
   float d0;
   if (aprsdecode_click.entries>0UL) {
      pf = aprsdecode_click.table[aprsdecode_click.selected].pff0;
      if (pf && aprspos_posvalid(pf->vardat->pos)) {
         oldpos = pf->vardat->pos;
         maptool_xytodeg((float)aprsdecode_click.x,
                (float)aprsdecode_click.y, &clickpos);
         d0 = aprspos_distance(clickpos, pf->vardat->pos);
         /*WrInt(trunc(d0*1000.0), 10); WrLn; */
         for (;;) {
            pf = pf->next;
            if (pf==0) break;
            if (aprspos_posvalid(pf->vardat->pos)
                && (pf->vardat->pos.lat!=oldpos.lat || pf->vardat->pos.long0!=oldpos.long0)
                ) {
               d1 = aprspos_distance(clickpos, pf->vardat->pos);
               /*WrInt(trunc(d1*1000.0), 15);WrInt(trunc(d0*1000.0), 15);
                WrLn; */
               if (d1<d0) {
                  aprsdecode_click.table[aprsdecode_click.selected]
                .pff0 = pf;
                  d0 = d1;
               }
               else if (d1>d0) break;
            }
         }
      }
   }
} /* end nearwaypoint() */


static void clickdelwaypoint(void)
{
   char s[101];
   struct aprsdecode__D1 * anonym;
   nearwaypoint();
   { /* with */
      struct aprsdecode__D1 * anonym = &aprsdecode_click.table[0UL];
      aprsdecode_delwaypoint(anonym->opf, &anonym->pff0);
      aprsstr_Assign(s, 101ul, anonym->opf->call, 9ul);
      aprsstr_Append(s, 101ul, " waypoint Deleted", 18ul);
      useri_textautosize(0L, 0L, 4UL, 4UL, 'r', s, 101ul);
   }
} /* end clickdelwaypoint() */

#define aprsmap_LF "\012"


static long flo(short c)
{
   if (c<=1023) return (long)(unsigned long)(unsigned char)gammatab[c];
   else return 255L;
   return 0;
} /* end flo() */


static void savevideo420(maptool_pIMAGE img, char fn[], unsigned long fn_len,
                 char format, float * bytecnt)
{
   long y;
   long x;
   long bb;
   long gg;
   long rr;
   long ww;
   unsigned long pb;
   unsigned long pr;
   unsigned long pw;
   char s[256];
   char h[256];
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   struct maptool_PIX * anonym3;
   long tmp;
   X2C_PCOPY((void **)&fn,fn_len);
   if (!osi_FdValid(videofd)) {
      /*
          videofd:=Create(fn);
      */
      if (FileSys_Exists(fn, fn_len)) videofd = osi_OpenWrite(fn, fn_len);
      else videofd = osi_OpenWrite(fn, fn_len);
      if (!osi_FdValid(videofd)) goto label;
      if (format=='M') {
         strncpy(h,"YUV4MPEG2 C420jpeg W",256u);
         aprsstr_IntToStr(maptool_xsize, 1UL, s, 256ul);
         aprsstr_Append(h, 256ul, s, 256ul);
         aprsstr_Append(h, 256ul, " H", 3ul);
         aprsstr_IntToStr(maptool_ysize, 1UL, s, 256ul);
         aprsstr_Append(h, 256ul, s, 256ul);
         aprsstr_Append(h, 256ul, " F25000000:1000000 Ip\012", 23ul);
         osi_WrBin(videofd, (char *)h, 256u/1u, aprsstr_Length(h, 256ul));
      }
   }
   if (vidbuf==0) {
      Storage_ALLOCATE((X2C_ADDRESS *) &vidbuf,
                (unsigned long)X2C_DIV(maptool_xsize*maptool_ysize*3L,2L));
      useri_debugmem.req = (unsigned long)
                X2C_DIV(maptool_xsize*maptool_ysize*3L,2L);
      useri_debugmem.screens += useri_debugmem.req;
      if (vidbuf==0) Error("video buffer alloc", 19ul);
   }
   if (format=='M') {
      strncpy(h,"FRAME\012",256u);
      osi_WrBin(videofd, (char *)h, 256u/1u, 6UL);
   }
   pw = 0UL;
   pb = (unsigned long)(maptool_xsize*maptool_ysize);
   pr = pb+pb/4UL;
   for (y = maptool_ysize-1L; y>=0L; y--) {
      tmp = maptool_xsize-1L;
      x = 0L;
      if (x<=tmp) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+y];
            if (format=='M') {
               vidbuf[pw] = (char)flo((short)(((unsigned long)
                anonym->r*76UL+(unsigned long)anonym->g*150UL+(unsigned long)
                anonym->b*29UL)/256UL));
            }
            else {
               vidbuf[pw] = (char)(X2C_DIV(flo((short)(((unsigned long)
                anonym->r*76UL+(unsigned long)anonym->g*150UL+(unsigned long)
                anonym->b*29UL)/256UL))*219L,256L)+16L);
            }
            ++pw;
         }
         if ((y&1) && (x&1)) {
            { /* with */
               struct maptool_PIX * anonym0 = &img->Adr[(x-1L)*img->Len0+y];
               rr = (long)anonym0->r;
               gg = (long)anonym0->g;
               bb = (long)anonym0->b;
            }
            { /* with */
               struct maptool_PIX * anonym1 = &img->Adr[(x)*img->Len0+y];
               rr += (long)anonym1->r;
               gg += (long)anonym1->g;
               bb += (long)anonym1->b;
            }
            { /* with */
               struct maptool_PIX * anonym2 = &img->Adr[(x-1L)
                *img->Len0+(y-1L)];
               rr += (long)anonym2->r;
               gg += (long)anonym2->g;
               bb += (long)anonym2->b;
            }
            { /* with */
               struct maptool_PIX * anonym3 = &img->Adr[(x)*img->Len0+(y-1L)
                ];
               rr += (long)anonym3->r;
               gg += (long)anonym3->g;
               bb += (long)anonym3->b;
            }
            rr = flo((short)X2C_DIV(rr,4L));
            gg = flo((short)X2C_DIV(gg,4L));
            bb = flo((short)X2C_DIV(bb,4L));
            ww = X2C_DIV(rr*76L+gg*150L+bb*29L,256L);
            rr = X2C_DIV((rr-ww)*145L,256L)+128L;
            if (rr<0L) rr = 0L;
            else if (rr>255L) rr = 255L;
            bb = X2C_DIV((bb-ww)*182L,256L)+128L;
            if (bb<0L) bb = 0L;
            else if (bb>255L) bb = 255L;
            if (format!='M') {
               vidbuf[pr] = (char)bb;
               vidbuf[pb] = (char)rr;
            }
            else {
               vidbuf[pr] = (char)rr;
               vidbuf[pb] = (char)bb;
            }
            ++pr;
            ++pb;
         }
         if (x==tmp) break;
      } /* end for */
   } /* end for */
   osi_WrBin(videofd, (char *)vidbuf, 10000001u/1u,
                (unsigned long)X2C_DIV(maptool_xsize*maptool_ysize*3L,2L));
   *bytecnt = *bytecnt+(float)X2C_DIV(maptool_xsize*maptool_ysize*3L,2L);
   label:;
   X2C_PFREE(fn);
} /* end savevideo420() */

/*
PROCEDURE savevideo444(img:pIMAGE; n:CARDINAL);
VAR x,y,p:INTEGER;
    buf:ARRAY[0..65535] OF CHAR;
    c, yy, rr, gg, bb:REAL;

  PROCEDURE flo(c:CARD16):REAL;
  BEGIN
    IF c<=HIGH(gammatab) THEN RETURN FLOAT(ORD(gammatab[c]))
                ELSE RETURN 255.0 END;
  END flo; 

BEGIN
  IF NOT FdValid(videofd) THEN RETURN END; 
  p:=0;

  FOR y:=ysize-1 TO 0 BY -1 DO
    FOR x:=0 TO xsize-1 DO
      WITH img^[x+y*xsize] DO rr:=flo(r); gg:=flo(g); bb:=flo(b) END;
      yy:=rr*0.299 + gg*0.587 +bb*0.114;
      buf[p]:=CHR(TRUNC(yy*(219.0/255.0))+16);    
      INC(p); IF p>HIGH(buf) THEN WrBin(videofd, buf, p); p:=0 END;

      c:=(bb-yy)*0.565+128.0;
      IF c<0.0 THEN c:=0.0 ELSIF c>255.9 THEN c:=255.0 END;   
      buf[p]:=CHR(TRUNC(c));   
      INC(p); IF p>HIGH(buf) THEN WrBin(videofd, buf, p); p:=0 END;

      c:=(rr-yy)*0.713+128.0;                                       
      IF c<0.0 THEN c:=0.0 ELSIF c>255.9 THEN c:=255.0 END;   
      buf[p]:=CHR(TRUNC(c));   
      INC(p); IF p>HIGH(buf) THEN WrBin(videofd, buf, p); p:=0 END;

    END;
  END;
  IF p>0 THEN WrBin(videofd, buf, p) END; 
END savevideo444;
*/

static void wrvidsize(float b)
{
   char s[101];
   aprsstr_FixToStr(X2C_DIVR(b,1.E+6f), 2UL, s, 101ul);
   aprsstr_Append(s, 101ul, " MBytes written", 16ul);
   useri_textautosize(0L, 0L, 4UL, 0UL, 'b', s, 101ul);
} /* end wrvidsize() */


static void copy(struct maptool_PIX dest[], unsigned long dest_len,
                unsigned long dest_len0, struct maptool_PIX src[],
                unsigned long src_len, unsigned long src_len0)
{
   unsigned long x;
   unsigned long tmp;
   tmp = dest_len-1;
   x = 0UL;
   if (x<=tmp) for (;; x++) {
      X2C_MOVE((char *)(src+(x)*src_len0),(char *)(dest+(x)*dest_len0),
                dest_len0*sizeof(struct maptool_PIX));
      if (x==tmp) break;
   } /* end for */
} /* end copy() */


static char cmpcol(struct aprsdecode_COLTYP c1, struct aprsdecode_COLTYP c2)
{
   return (c1.r!=c2.r || c1.g!=c2.g) || c1.b!=c2.b;
} /* end cmpcol() */


static void addradio(void)
{
   /* radiation visability map */
   char abort0;
   struct aprsdecode_COLTYP c2;
   struct aprsdecode_COLTYP c1;
   if (aprsdecode_click.panorama) {
      if (aprspos_posvalid(aprsdecode_click.markpos)) {
         aprsdecode_lums.rf = 0L;
         abort0 = 0;
         maptool_clr(rfimg);
         getgeocol(useri_fCOLMARK1, geobri(), 100UL, 100UL, 100UL, &c1);
         panorama(rfimg, aprsdecode_click.markpos, c1, &abort0);
         radio.wasradio = 1;
         if (abort0) {
            useri_textautosize(0L, 0L, 5UL, 2UL, 'r', "panorama aborted",
                17ul);
            closeradio();
            radio.wasradio = 0;
            useri_sayonoff("Panorama", 9ul, aprsdecode_click.withradio);
         }
      }
      if (radio.wasradio) maptool_addmap(image, rfimg);
   }
   else if (aprspos_posvalid(aprsdecode_click.markpos)
                || aprspos_posvalid(aprsdecode_click.measurepos)) {
      aprsdecode_lums.rf = 0L;
      getgeocol(useri_fCOLMARK1, geobri(), 100UL, 0UL, 0UL, &c1);
      getgeocol(useri_fCOLMARK2, geobri(), 0UL, 100UL, 0UL, &c2);
      if ((((((((((((((((!radio.wasradio || radio.markpos.lat!=aprsdecode_click.markpos.lat)
                 || radio.markpos.long0!=aprsdecode_click.markpos.long0)
                || radio.measurepos.lat!=aprsdecode_click.measurepos.lat)
                || radio.measurepos.long0!=aprsdecode_click.measurepos.long0)
                 || radio.mappos.lat!=aprsdecode_mappos.lat)
                || radio.mappos.long0!=aprsdecode_mappos.long0)
                || radio.initzoom!=aprsdecode_initzoom)
                || radio.finezoom!=aprsdecode_finezoom)
                || radio.ant1!=getant(useri_fANT1))
                || radio.ant2!=getant(useri_fANT2))
                || radio.ant3!=getant(useri_fANT3)) || radio.bri!=geobri())
                || radio.contr!=geocontr()) || cmpcol(radio.c1,
                c1)) || cmpcol(radio.c2,
                c2)) || radio.qual!=(long)useri_confflags(useri_fSRTMCACHE,
                0UL)) {
         abort0 = 0;
         maptool_clr(rfimg);
         if (aprspos_posvalid(aprsdecode_click.markpos)) {
            radioimage(rfimg, aprsdecode_click.markpos, 0UL, &abort0);
         }
         if (!abort0 && aprspos_posvalid(aprsdecode_click.measurepos)) {
            radioimage(rfimg, aprsdecode_click.measurepos,
                (unsigned long)aprspos_posvalid(aprsdecode_click.markpos),
                &abort0);
         }
         /*      IF (lums.map>0) & posvalid(click.measurepos)
                & posvalid(click.markpos) THEN makebw(image) END; */
         radio.wasradio = 1;
         radio.markpos = aprsdecode_click.markpos;
         radio.measurepos = aprsdecode_click.measurepos;
         radio.mappos = aprsdecode_mappos;
         radio.initzoom = aprsdecode_initzoom;
         radio.finezoom = aprsdecode_finezoom;
         radio.ant1 = getant(useri_fANT1);
         radio.ant2 = getant(useri_fANT2);
         radio.ant3 = getant(useri_fANT3);
         radio.bri = geobri();
         radio.contr = geocontr();
         radio.c1 = c1;
         radio.c2 = c2;
         radio.qual = (long)useri_confflags(useri_fSRTMCACHE, 0UL);
         reliefcolors(rfimg, aprspos_posvalid(aprsdecode_click.measurepos) && aprspos_posvalid(aprsdecode_click.markpos));
         if (abort0) {
            useri_textautosize(0L, 0L, 5UL, 2UL, 'r', "radiorange aborted",
                19ul);
            closeradio();
            radio.wasradio = 0;
            useri_sayonoff("Radiorange Map", 15ul,
                aprsdecode_click.withradio);
         }
      }
      if (radio.wasradio) maptool_addmap(image, rfimg);
   }
} /* end addradio() */


static void xytomark(void)
{
   struct aprspos_POSITION pos;
   maptool_xytodeg((float)aprsdecode_click.x, (float)aprsdecode_click.y,
                &pos);
   aprstext_setmark1(pos, 1, X2C_max_longint, 0UL);
   useri_postoconfig(pos);
   aprsdecode_click.waysum = 0.0f;
} /* end xytomark() */

/*
PROCEDURE clicktomark;
VAR pos:POSITION;
BEGIN
  IF (click.entries>0) & (click.table[click.selected].opf<>NIL)
  & posvalid(click.table[click.selected].opf^.lastpos)
  THEN pos:=click.table[click.selected]
                .opf^.lastpos ELSE pos:=click.clickpos END;
  setmark1(pos, TRUE, MAX(INTEGER), 0);
END clicktomark;
*/

static void clicktomark(void)
{
   if ((aprsdecode_click.entries>0UL && aprsdecode_click.table[aprsdecode_click.selected]
                .opf) && aprspos_posvalid(aprsdecode_click.table[aprsdecode_click.selected].opf->lastpos)
                ) {
      aprstext_setmarkalti(aprsdecode_click.table[aprsdecode_click.selected]
                .pff0, aprsdecode_click.table[aprsdecode_click.selected].opf,
                 1);
   }
   else {
      aprstext_setmark1(aprsdecode_click.clickpos, 1, X2C_max_longint, 0UL);
   }
} /* end clicktomark() */

#define aprsmap_ADDDELAY 55


static unsigned long slowupdate(void)
/* add image update time on cpu intensiv options */
{
   return (unsigned long)((aprsdecode_lums.wxcol=='R' || aprsdecode_lums.wxcol=='W')
                 || aprsdecode_click.withradio)*55UL;
} /* end slowupdate() */


static void addrftracks(char dryrun, char tx)
{
   char clr;
   clr = !dryrun; /* clr old image before draw */
   if (tx) mhtracks(aprsdecode_click.mhop, &clr);
   else rftracks(aprsdecode_click.mhop, &clr);
   if (!(dryrun || clr)) {
      /* not dryrun and something drawn */
      radio.wasradio = 0; /* no radiolink data in rfimg now */
      maptool_shine(rfimg, aprsdecode_lums.rfbri);
      maptool_addmap(image, rfimg);
   }
} /* end addrftracks() */

#define aprsmap_QUICKMOV 10
/* faster if nothing visable moves */

#define aprsmap_FASTDELAY 70
/* delay switching to fast */

#define aprsmap_SLOWER 3
/* steps befor next move switch to slow */

#define aprsmap_TRAILER 10
/* steps at end of move till video end */


static void animate(const aprsdecode_MONCALL singlecall, unsigned long step,
                char tofile[], unsigned long tofile_len)
{
   aprsdecode_pOPHIST singleop;
   aprsdecode_pOPHIST op;
   aprsdecode_pFRAMEHIST pf1;
   aprsdecode_pFRAMEHIST pf;
   unsigned char efil;
   unsigned long fractime;
   /*    icnt, */
   unsigned long showt;
   unsigned long stime;
   unsigned long endtime;
   unsigned long vtime;
   unsigned long nextmov;
   long fastdelay;
   long fast;
   unsigned long skip;
   float nomove;
   float y;
   float x;
   struct aprsdecode_COLTYP col;
   signed char textpos;
   char stop;
   char blown;
   char mapok;
   char dir;
   struct aprspos_POSITION ipos;
   float itime;
   float bytew;
   char s1[41];
   char s[41];
   long minalt;
   struct aprsdecode_DAT dat;
   struct aprsdecode_OPHIST * anonym;
   struct aprsdecode_VARDAT * anonym0;
   aprsdecode_MONCALL tmp;
   X2C_PCOPY((void **)&tofile,tofile_len);
   closeradio();
   singleop = aprstext_oppo(singlecall);
   if (!pandone) {
      findsize(&newpos0, &newpos1, aprsdecode_click.mhop, 'T');
      mapzoom(newpos0, newpos1, (unsigned long)useri_conf2int(useri_fDEFZOOM,
                 0UL, 1L, 18L, 14L), 1);
      pandone = 1;
      maptool_mercator(aprsdecode_mappos.long0, aprsdecode_mappos.lat,
                aprsdecode_initzoom, &aprsdecode_inittilex,
                &aprsdecode_inittiley, &maptool_shiftx, &maptool_shifty);
   }
   showt = 0UL;
   bytew = 0.0f;
   aprsdecode_click.dryrun = 0;
   vtime = aprsdecode_systime; /* last whole data time */
   endtime = vtime-(aprsdecode_lums.firstdim+aprsdecode_lums.maxdim);
                /* start time minimum */
   stime = endtime;
   nomove = movest(500UL)*5.0f; /* min km/s for a moving object */
   if (step==0UL) {
      step = aprsdecode_trunc(RealMath_sqrt(nomove)*(float)
                (25L*useri_conf2int(useri_fANIMSPEED, 0UL, 0L, 10000L,
                400L)))+1UL;
   }
   fastdelay = -useri_conf2int(useri_fVIDEOFAST, 0UL, 0L, 10000L, 0L);
   markvisable(*(aprsdecode_MONCALL *)memcpy(&tmp,"",1u));
   op = aprsdecode_ophist0;
   maptool_xytodeg((float)maptool_xsize, 0.0f, &ipos);
   if (useri_configon(useri_fTRACKFILT)) efil = 0x1U;
   else efil = 0x25U;
   minalt = useri_conf2int(useri_fALTMIN, 0UL, -10000L, 65535L, -10000L);
   stop = 0;
   while (op) {
      { /* with */
         struct aprsdecode_OPHIST * anonym = op;
         anonym->drawhints &= ~0x10U;
         if ((0x8U & anonym->drawhints)) {
            if ((aprspos_distance(anonym->margin0,
                anonym->margin1)>nomove*400.0f && (singleop==0 || op==singleop)
                ) && op->sym.tab!='\001') {
               /* they will move */
               pf1 = 0;
               pf = op->frames;
               while (pf) {
                  if ((pf->nodraw&~efil)
                ==0U && aprspos_posvalid(pf->vardat->pos)) {
                     if (pf1) {
                        { /* with */
                           struct aprsdecode_VARDAT * anonym0 = pf1->vardat;
                           if ((((((pf1->time0>=stime && aprspos_posvalid(anonym0->pos)
                ) && anonym0->pos.long0<=ipos.long0)
                && anonym0->pos.lat>=ipos.lat)
                && anonym0->pos.long0>=aprsdecode_mappos.long0)
                && anonym0->pos.lat<=aprsdecode_mappos.lat)
                && aprspos_distance(anonym0->pos,
                pf->vardat->pos)>nomove*10.0f) {
                              /*                & (pf^.next<>NIL)
                & posvalid(pf^.next^.vardat^.pos)  */
                              /* moving */
                              if (pf->time0>endtime) {
                                 endtime = pf->time0;
                /* show till latest mover */
                              }
                              if (pf1->time0<vtime) {
                                 vtime = pf1->time0; /* from 1st mover */
                              }
                              anonym->drawhints |= 0x10U;
                           }
                        }
                     }
                     pf1 = pf;
                  }
                  pf = pf->next;
               }
               /*        INCL(drawhints, MOVES); */
               anonym->drawhints &= ~0x8U;
            }
         }
         /*        IF lasttime>endtime THEN endtime:=lasttime END;
                (* show till latest mover *) */
         /*        IF (frames<>NIL) & (frames^.time<vtime)
                THEN vtime:=frames^.time END; (* from 1st mover *) */
         op = anonym->next;
      }
   }
   aprsdecode_lums.moving = 1; /* make not moving darker */
   if (aprsdecode_lums.map>0L) {
      maptool_loadmap(image, aprsdecode_inittilex, aprsdecode_inittiley,
                aprsdecode_initzoom, aprsdecode_finezoom, maptool_shiftx,
                maptool_shifty, &mapok, &blown,
                useri_configon(useri_fALLOWEXP), 0);
   }
   else maptool_clr(image);
   /*
     IF lums.obj>0 THEN 
       symbols(ophist, TRUE, FALSE);
       text(ophist, TRUE, FALSE, FALSE);
     END;
   */
   if (aprsdecode_lums.sym>0L) symbols(aprsdecode_ophist0, 0, 0);
   if (aprsdecode_lums.text>0L) {
      text(aprsdecode_ophist0, 0, 0, 0, 1); /* first draw dimmed */
      text(aprsdecode_ophist0, 1, 0, 0, 1); /* overdraw dimmed */
   }
   maptool_cc(image, vtime, endtime);
   op = aprsdecode_ophist0;
   while (op) {
      op->drawhints &= ~0x8U; /* now use only mover(s) */
      op = op->next;
   }
   aprsdecode_lums.moving = 0;
   videofd = -1L;
   if (vtime>stime) stime = vtime;
   /*  icnt:=0; */
   fast = fastdelay;
   skip = 0UL;
   fractime = 0UL;
   do {
      if (skip==0UL) {
         /*
               MOVE(image, rfimg, SIZE(PIX)*xsize*ysize);
         */
         copy(rfimg->Adr, rfimg->Len1, rfimg->Len0, image->Adr, image->Len1,
                image->Len0);
         if (aprsdecode_lums.track>1L) {
            tracks(rfimg, aprsdecode_ophist0, 0, vtime);
         }
      }
      op = aprsdecode_ophist0;
      nextmov = endtime;
      while (op) {
         if ((0x10U & op->drawhints)) {
            pf = 0;
            pf1 = op->frames;
            while (pf1 && vtime>pf1->time0) {
               if ((pf1->nodraw&~efil)==0U) pf = pf1;
               pf1 = pf1->next;
            }
            while (pf1 && (pf1->nodraw&~efil)!=0U) pf1 = pf1->next;
            if (pf) {
               if ((pf1 && pf1->time0>pf->time0) && vtime>pf->time0) {
                  /* interpolate waypoints */
                  itime = X2C_DIVR((float)(vtime-pf->time0)+(float)
                fractime*0.04f,(float)(pf1->time0-pf->time0));
                  /*
                              IF itime>1.0 THEN itime:=1.0 ELSIF itime<0.0 THEN itime:=0.0 END;
                  
                  */
                  ipos.long0 = pf->vardat->pos.long0*(1.0f-itime)
                +pf1->vardat->pos.long0*itime;
                  ipos.lat = pf->vardat->pos.lat*(1.0f-itime)
                +pf1->vardat->pos.lat*itime;
                  dir = pf->vardat->pos.long0>pf1->vardat->pos.long0;
                /* mirror symbol on long deg */
               }
               else {
                  /* end of track */
                  ipos = pf->vardat->pos;
                  dir = (0x1U & op->drawhints)!=0;
               }
               if (maptool_mapxy(ipos, &x,
                &y)>=0L && maptool_vistime(pf->time0)) {
                  if (skip==0UL) {
                     maptool_drawsym(rfimg, op->sym.tab, op->sym.pic, dir, x,
                 y, (unsigned long)aprsdecode_lums.sym);
                     /*
                                   IF configon(fARROW)
                & (Decode(pf^.vardat^.raw)>=0) & (dat.course<360) THEN
                                     col.r:=500; col.g:=500; col.b:=0;
                                     drawarrow(rfimg, x, y,
                FLOAT(dat.speed)*0.25+10.0, FLOAT(dat.course)*(-pi/180.0), 0,
                 256, col);
                                   END;
                     */
                     if (useri_configon(useri_fKMH)
                && aprsdecode_Decode(pf->vardat->raw, 500ul, &dat)>=0L) {
                        s[0] = 0;
                        if (dat.speed>0UL && dat.speed<2147483647UL) {
                           useri_confstr(useri_fKMH, s1, 41ul);
                           if (s1[0U]) {
                              aprsstr_IntToStr((long)aprsdecode_trunc((float)
                dat.speed*1.852f), 1UL, s, 41ul);
                              aprsstr_Append(s, 41ul, s1, 41ul);
                           }
                        }
                        if (dat.altitude<X2C_max_longint && dat.altitude>=minalt)
                 {
                           if (s[0U]) {
                              aprsstr_Append(s, 41ul, " ", 2ul);
                           }
                           aprsstr_IntToStr(dat.altitude, 1UL, s1, 41ul);
                           aprsstr_Append(s, 41ul, s1, 41ul);
                           aprsstr_Append(s, 41ul, "m", 2ul);
                        }
                        if (s[0U]) {
                           maptool_Colset(&col, 'Y');
                           textpos = -8;
                           maptool_drawstr(rfimg, s, 41ul, x+7.0f,
                y-(float)(aprsdecode_lums.fontysize+3UL),
                (unsigned long)aprsdecode_lums.text, 0UL, col, &textpos, 0UL,
                 1, aprsdecode_click.dryrun);
                        }
                     }
                     maptool_Colset(&col, 'W');
                     textpos = 0;
                     maptool_drawstr(rfimg, op->call, 9ul, x+8.0f,
                y-(float)(aprsdecode_lums.fontysize/2UL),
                (unsigned long)aprsdecode_lums.text, 0UL, col, &textpos, 0UL,
                 1, aprsdecode_click.dryrun);
                  }
                  if ((pf1 && X2C_DIVR(aprspos_distance(pf->vardat->pos,
                pf1->vardat->pos),
                (float)(pf1->time0-pf->time0))>nomove) && pf1->time0<nextmov)
                 nextmov = pf1->time0;
               }
            }
         }
         op = op->next;
      }
      if (vtime+step*3UL<nextmov) {
         /*/VIDEORATE*/
         ++fast;
      }
      else {
         fast = fastdelay;
         skip = 0UL;
      }
      if (skip==0UL) {
         drawtime(rfimg, vtime, fast); /*>FASTDELAY*/
         if (endtime>stime) {
            col.r = 100UL;
            col.g = 800UL;
            col.b = 1000UL;
            maptool_area(rfimg, 0L, 1L,
                (long)aprsdecode_trunc(X2C_DIVR((float)maptool_xsize*(float)
                (vtime-stime),(float)(endtime-stime))), 3L, col, 0);
         }
         if (tofile[0UL]) {
            savevideo420(rfimg, tofile, tofile_len, 'M', &bytew);
            /*        INC(icnt); */
            if (showt!=TimeConv_time()) {
               showt = TimeConv_time();
               wrvidsize(bytew);
               useri_redraw(rfimg);
            }
            xosi_Eventloop(1UL);
         }
         else {
            useri_refresh = 1;
            for (;;) {
               /* while stop pressed */
               if (useri_refresh) useri_redraw(rfimg);
               aprsdecode_lums.actfps = (long)step;
                /* for faster/slower button */
               xosi_Eventloop(1000UL);
               step = (unsigned long)aprsdecode_lums.actfps;
               if (aprsdecode_click.cmd==' ') {
                  if (stop) {
                     stop = 0;
                     aprsdecode_click.cmd = 'A';
                  }
                  else {
                     stop = 1;
                     aprsdecode_click.cmd = 'v';
                  }
               }
               else if (aprsdecode_click.cmd!='v') stop = 0;
               if (!stop && aprsdecode_click.cmd!='v') break;
               aprsdecode_realtime = TimeConv_time();
            }
         }
         if (fast>250L) fast = 250L;
         if (fast>0L) skip = (unsigned long)X2C_DIV(fast,25L);
         else skip = 0UL;
      }
      else --skip;
      fractime += step;
      vtime += fractime/25UL;
      fractime = fractime%25UL;
      /*
          IF fast>FASTDELAY THEN INC(vtime, step*STEPMUL) ELSE INC(vtime,
                step) END;
      */
      aprsdecode_realtime = TimeConv_time();
   } while (!((vtime>endtime+(step*10UL)/25UL || (aprsdecode_click.cmd!='A' && aprsdecode_click.cmd!='a') && aprsdecode_click.cmd!='\312') || useri_newxsize>0UL));
   if (osi_FdValid(videofd)) {
      osi_Close(videofd);
      useri_textautosize(0L, 0L, 4UL, 0UL, 'b', "map.y4m Saved", 14ul);
   }
   if (vidbuf) {
      useri_debugmem.screens -= (unsigned long)
                X2C_DIV(maptool_xsize*maptool_ysize*3L,2L);
      Storage_DEALLOCATE((X2C_ADDRESS *) &vidbuf,
                (unsigned long)X2C_DIV(maptool_xsize*maptool_ysize*3L,2L));
      vidbuf = 0;
   }
   X2C_PFREE(tofile);
} /* end animate() */


static void makeimage(char dryrun)
{
   char mapok;
   markvisable(aprsdecode_click.mhop);
   if (aprsdecode_click.mhop[0UL] && mhtx==aprsmap_OPHEARD) {
      findsize(&newpos0, &newpos1, aprsdecode_click.mhop, 'H');
   }
   else if (aprsdecode_click.mhop[0UL] && mhtx==aprsmap_OPOBJ) {
      findsize(&newpos0, &newpos1, aprsdecode_click.mhop, 'O');
   }
   else if (aprsdecode_lums.rf>1L) {
      findsize(&newpos0, &newpos1, aprsdecode_click.mhop, 'R');
   }
   else findsize(&newpos0, &newpos1, aprsdecode_click.mhop, 'T');
   if (!pandone) {
      mapzoom(newpos0, newpos1, (unsigned long)useri_conf2int(useri_fDEFZOOM,
                 0UL, 1L, 18L, 14L), 1);
      pandone = 1;
      aprsdecode_lums.moving = 0;
   }
   maptool_mercator(aprsdecode_mappos.long0, aprsdecode_mappos.lat,
                aprsdecode_initzoom, &aprsdecode_inittilex,
                &aprsdecode_inittiley, &maptool_shiftx, &maptool_shifty);
   aprsdecode_click.dryrun = dryrun;
   if (dryrun) {
      aprsdecode_click.min0 = 8L;
      aprsdecode_click.entries = 0UL;
      aprsdecode_click.pf0 = 0;
   }
   else if (aprsdecode_lums.map>0L) {
      /*
          FILL(image, 0C, SIZE(PIX)*xsize*ysize);
          IF lums.map>0 THEN
      
            loadmap(map, FALSE);
            addmap(image, map);
            loadmap(image, FALSE);
          END;
      */
      maptool_loadmap(image, aprsdecode_inittilex, aprsdecode_inittiley,
                aprsdecode_initzoom, aprsdecode_finezoom, maptool_shiftx,
                maptool_shifty, &mapok, &useri_isblown,
                useri_configon(useri_fALLOWEXP), 0);
      if (!mapok) tooltips('B');
      /*IF isblown THEN WrStrLn("loadmapblown")
                ELSE WrStrLn("loadmapNOTblown") END; */
      if (!mapok && useri_configon(useri_fGETMAPS)) maptrys = 30UL;
      else maptrys = 0UL;
   }
   else {
      /*      IF isblown THEN textautosize(0, 0, 10, "g",
                "Expanded Tiles!") ELSE killmenuid(10) END; */
      maptool_clr(image);
   }
   /*  IF lums.wxcol<>0C THEN */
   if (aprsdecode_lums.wxcol=='R' || aprsdecode_lums.wxcol=='W') {
      /* "w" is show wx stations only */
      if (!dryrun) {
         closeradio();
         if (aprsdecode_lums.map>0L) maptool_makebw(image);
         maptool_clr(rfimg);
         /*      metercolor; */
         if (aprsdecode_lums.wxcol=='R') {
            metercolor('R');
            maptool_addmap(image, rfimg);
            maptool_clr(rfimg);
            metercolor('1');
         }
         else metercolor('T');
         maptool_addmap(image, rfimg);
      }
   }
   else if ((aprsdecode_lums.rf>1L && aprsdecode_click.mhop[0UL])
                && mhtx==aprsmap_OPSENT) addrftracks(dryrun, 0);
   else if (aprsdecode_click.mhop[0UL] && mhtx==aprsmap_OPHEARD) {
      addrftracks(dryrun, 1);
   }
   if (aprsdecode_lums.moving) {
      text(aprsdecode_ophist0, 0, 0, 0, 0);
      if (aprsdecode_lums.track>1L) {
         tracks(image, aprsdecode_ophist0, 0, X2C_max_longcard);
      }
      symbols(aprsdecode_ophist0, 1, 0);
      if (aprsdecode_lums.sym>0L) {
         symbols(aprsdecode_ophist0, 0, 0);
      }
      text(aprsdecode_ophist0, 1, 0, 0, 0);
   }
   else {
      if (aprsdecode_lums.track>1L) {
         tracks(image, aprsdecode_ophist0, 0, X2C_max_longcard);
      }
      if (aprsdecode_lums.obj>0L) {
         symbols(aprsdecode_ophist0, 1, 0);
         text(aprsdecode_ophist0, 1, 1, 0, 1);
      }
      if (aprsdecode_lums.sym>0L) symbols(aprsdecode_ophist0, 0, 0);
      if (aprsdecode_lums.text>0L || aprsdecode_lums.wxcol) {
         text(aprsdecode_ophist0, 0, 1, 0, 1);
      }
   }
   if (!dryrun) {
      if (aprsdecode_click.marktime==0UL || aprsdecode_click.marktime+10UL>aprsdecode_realtime)
                 {
         maptool_setmark(image, aprsdecode_click.markpos,
                aprsdecode_click.marktime==0UL);
      }
      drawsquer(image);
      if (!aprsdecode_click.withradio) {
         measureline(image, aprsdecode_click.markpos,
                aprsdecode_click.measurepos, aprsdecode_click.markalti);
      }
      /*WrFixed(click.markpos.long, 10, 4);
                WrFixed(click.markpos.lat, 10, 4); */
      /*WrFixed(click.measurepos.long, 10, 4);
                WrFixed(click.measurepos.lat, 10, 4); */
      /*WrLn; */
      /*
          IF xmouse.x>0 THEN mouseshow END;
      */
      maptool_cc(image, TimeConv_time(), 0UL);
      if (useri_configon(useri_fRULER)) maptool_ruler(image);
      drawzoomsquer(image);
      if (aprsdecode_click.withradio) addradio();
      else radio.wasradio = 0;
   }
   else {
      nearwaypoint();
      aprsdecode_click.dryrun = 0;
      if (aprsdecode_click.mhop[0UL]==0) highlight();
   }
   if (aprsdecode_lums.wxcol=='w') {
      useri_textautosize(0L, 0L, 4UL, 10UL, 'g', "Wx Stations (exit with ESC)\
", 28ul);
   }
   else if (aprsdecode_lums.wxcol=='W') {
      useri_textautosize(0L, 0L, 4UL, 5UL, 'g', "Temperatue Map (exit with ES\
C)", 31ul);
   }
   else if (aprsdecode_lums.wxcol=='R') {
      useri_textautosize(0L, 0L, 4UL, 5UL, 'g', "Rain Map (exit with ESC)",
                25ul);
   }
   useri_refresh = 1;
   lastxupdate = aprsdecode_realtime;
} /* end makeimage() */


static void MainEvent(void)
{
   char ch;
   char cfgs[21];
   char menu;
   char raw;
   menu = 0;
   raw = 0;
   aprsdecode_realtime = TimeConv_time();
   if (!aprsdecode_lums.logmode) aprsdecode_systime = aprsdecode_realtime;
   if (aprsdecode_realtime<lastxupdate) lastxupdate = aprsdecode_realtime;
   if (aprsdecode_realtime<laststatref) laststatref = aprsdecode_realtime;
   if (aprsdecode_click.dryrun) {
      makeimage(1);
      if (aprsdecode_click.entries>1UL && aprsdecode_click.entries<9UL) {
         /* rotate find order */
         cycleorder = (cycleorder+1UL)%aprsdecode_click.entries;
         aprsdecode_click.table[9UL] = aprsdecode_click.table[0UL];
         aprsdecode_click.table[0UL] = aprsdecode_click.table[cycleorder];
         aprsdecode_click.table[cycleorder] = aprsdecode_click.table[9UL];
      }
      mestxt[0U] = 0;
      maptool_xytodeg((float)aprsdecode_click.x, (float)aprsdecode_click.y,
                &aprsdecode_click.clickpos);
      if (aprsdecode_click.entries>=1UL) aprsdecode_lums.errorstep = 0;
      aprsdecode_click.cmd = 0;
      if (xosi_Shift) aprsdecode_click.cmd = 'X';
      else if (aprsdecode_click.entries==1UL) {
         if (aprsdecode_click.table[0UL]
                .typf==aprsdecode_tSYMBOL || aprsdecode_click.table[0UL]
                .typf==aprsdecode_tOBJECT) {
            if (aprsdecode_click.table[0UL].opf->lastinftyp>=100U) {
               /* wx symbol */
               useri_confstr(useri_fCLICKWXSYM, cfgs, 21ul);
               if (aprsstr_InStr(cfgs, 21ul, ".", 2ul)>=0L) {
                  aprsdecode_click.cmd = '.';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "=", 2ul)>=0L) {
                  aprsdecode_click.cmd = '=';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "H", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'H';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "C", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'c';
               }
            }
            else {
               /* not wx symbol */
               useri_confstr(useri_fCLICKSYM, cfgs, 21ul);
               if (aprsstr_InStr(cfgs, 21ul, ".", 2ul)>=0L) {
                  aprsdecode_click.cmd = '.';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "=", 2ul)>=0L) {
                  aprsdecode_click.cmd = '=';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "H", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'H';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "C", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'c';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "A", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'a';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "X", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'x';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "Y", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'y';
               }
            }
         }
         else if (aprsdecode_click.table[0UL].typf==aprsdecode_tTEXT) {
            useri_confstr(useri_fCLICKTEXT, cfgs, 21ul);
            if (aprsstr_InStr(cfgs, 21ul, ".", 2ul)>=0L) {
               aprsdecode_click.cmd = '.';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "=", 2ul)>=0L) {
               aprsdecode_click.cmd = '=';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "H", 2ul)>=0L) {
               aprsdecode_click.cmd = 'H';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "C", 2ul)>=0L) {
               aprsdecode_click.cmd = 'c';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "A", 2ul)>=0L) {
               aprsdecode_click.cmd = 'a';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "X", 2ul)>=0L) {
               aprsdecode_click.cmd = 'x';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "Y", 2ul)>=0L) {
               aprsdecode_click.cmd = 'y';
            }
         }
         else if (aprsdecode_click.table[0UL].typf==aprsdecode_tTRACK) {
            useri_confstr(useri_fCLICKTRACK, cfgs, 21ul);
            if (aprsdecode_click.mhop[0UL]
                && X2C_STRCMP(aprsdecode_click.mhop,9u,
                aprsdecode_click.table[0UL].opf->call,9u)==0) {
               /* in single user mode and click same track */
               if (aprsstr_InStr(cfgs, 21ul, "q", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'q';
               }
               else if (aprsstr_InStr(cfgs, 21ul, "A", 2ul)>=0L) {
                  aprsdecode_click.cmd = 'a';
               }
            }
            else if (aprsstr_InStr(cfgs, 21ul, ".", 2ul)>=0L) {
               aprsdecode_click.cmd = '.';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "=", 2ul)>=0L) {
               aprsdecode_click.cmd = '=';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "A", 2ul)>=0L) {
               aprsdecode_click.cmd = 'a';
            }
            else if (aprsstr_InStr(cfgs, 21ul, "q", 2ul)>=0L) {
               aprsdecode_click.cmd = 'q';
            }
            nearwaypoint(); /* set marker on click to track */
            aprstext_setmarkalti(aprsdecode_click.table[aprsdecode_click.selected]
                .pff0, aprsdecode_click.table[0UL].opf, 1);
            aprsdecode_click.marktime = aprsdecode_realtime;
         }
         if (aprsstr_InStr(cfgs, 21ul, "u", 2ul)>=0L) {
            raw = 1;
            menu = 1;
         }
         if (aprsdecode_click.cmd==0) menu = 1;
         if (aprsdecode_click.marktime==0UL) {
            /* hard marker */
            aprstext_measure(aprsdecode_click.markpos,
                aprsdecode_click.clickpos, mestxt, 201ul, 0);
         }
         else {
            aprstext_measure(aprsdecode_click.markpos,
                aprsdecode_click.measurepos, mestxt, 201ul, 0);
         }
         aprsdecode_importbeacon(aprsdecode_click.table[aprsdecode_click.selected]
                .opf); /* copy to beacon editor */
      }
      else {
         /* clicked empty map */
         if (aprsdecode_click.marktime==0UL || aprsdecode_click.marktime+10UL>=aprsdecode_realtime)
                 {
            aprstext_measure(aprsdecode_click.markpos,
                aprsdecode_click.clickpos, mestxt, 201ul, 1);
         }
         menu = 1;
         useri_beaconimported[0UL] = 0;
      }
      useri_confstr(useri_fCLICKMAP, (char *) &ch, 1u/1u);
      if (!xosi_Shift && ch=='2') {
         if (aprspos_posvalid(aprsdecode_click.markpos)) ch = 'Y';
         else ch = 'X';
      }
      if (ch=='C') aprsdecode_click.cmd = 'C';
      else if (!xosi_Shift && ch=='X' || xosi_Shift && ch=='Y') {
         aprsdecode_click.cmd = 'X';
      }
      else if (!xosi_Shift && ch=='Y' || xosi_Shift && ch=='X') {
         aprsdecode_click.cmd = 'Y';
         aprstext_measure(aprsdecode_click.markpos,
                aprsdecode_click.clickpos, mestxt, 201ul, 0);
      }
      if ((aprsdecode_click.entries>0UL && aprsdecode_click.table[aprsdecode_click.selected]
                .typf==aprsdecode_tTEXT)
                && aprsdecode_click.table[aprsdecode_click.selected].opf) {
         useri_copypaste(aprsdecode_click.table[aprsdecode_click.selected]
                .opf->call, 9ul);
      }
      if (!xosi_Shift) {
         /* not close menus on shift click to map */
         if (!aprsdecode_lums.headmenuy && ((aprsdecode_click.entries==0UL || menu)
                 || aprsdecode_click.mhop[0UL])
                || (aprsdecode_lums.headmenuy && aprsdecode_click.entries>0UL)
                 && (menu || aprsdecode_click.mhop[0UL])) {
            ch = aprsdecode_click.cmd;
            useri_mainpop();
            aprsdecode_click.cmd = ch;
         }
         else useri_killallmenus();
      }
      /*          THEN mainpop; click.cmd:=" "; ELSE killallmenus END; */
      if (mestxt[0U]) {
         useri_textautosize(0L, 0L, 6UL, 10UL, 'b', mestxt, 201ul);
      }
   }
   /*WrInt(ORD(click.cmd), 10); WrStrLn(" cmd1"); */
   if (raw || aprsdecode_click.cmd) {
      maptool_startmapdelay();
      if (aprsdecode_click.cmd=='\030') {
         aprsdecode_mappos.lat = aprsdecode_mappos.lat-movest((unsigned long)
                maptool_ysize)*shiftfine();
         maptool_limpos(&aprsdecode_mappos);
      }
      else if (aprsdecode_click.cmd=='\005') {
         aprsdecode_mappos.lat = aprsdecode_mappos.lat+movest((unsigned long)
                maptool_ysize)*shiftfine();
         maptool_limpos(&aprsdecode_mappos);
      }
      else if (aprsdecode_click.cmd=='\023') {
         aprsdecode_mappos.long0 = aprsdecode_mappos.long0-movest((unsigned long)
                maptool_xsize)*shiftfine();
         maptool_limpos(&aprsdecode_mappos);
      }
      else if (aprsdecode_click.cmd=='\004') {
         aprsdecode_mappos.long0 = aprsdecode_mappos.long0+movest((unsigned long)
                maptool_xsize)*shiftfine();
         maptool_limpos(&aprsdecode_mappos);
      }
      else if (aprsdecode_click.cmd=='T') {
         if (aprspos_posvalid(newpos0) && aprspos_posvalid(newpos1)) {
            mapzoom(newpos0, newpos1,
                (unsigned long)useri_conf2int(useri_fDEFZOOM, 0UL, 1L, 18L,
                14L), 1);
         }
      }
      else if (aprsdecode_click.cmd=='o') {
         aprsdecode_click.dryrun = 0;
         push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
         aprsdecode_objsender(aprsdecode_click.table[aprsdecode_click.selected]
                .opf, aprsdecode_click.mhop, 9ul);
         mhtx = aprsmap_OPOBJ;
         pandone = 0;
      }
      else if (aprsdecode_click.cmd=='H') {
         aprsdecode_click.dryrun = 0;
         if (aprsdecode_click.entries>0UL) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            if (aprsdecode_click.table[aprsdecode_click.selected].opf) {
               memcpy(aprsdecode_click.mhop,
                aprsdecode_click.table[aprsdecode_click.selected].opf->call,
                9u);
            }
            mhtx = aprsmap_OPHEARD;
            pandone = 0;
         }
      }
      else if (aprsdecode_click.cmd=='.' || aprsdecode_click.cmd=='=') {
         /*
                     tracenew.call:=click.mhop;
                (* watch call for incoming data *) 
         */
         aprsdecode_click.dryrun = 0;
         if (aprsdecode_click.entries>0UL) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            if (aprsdecode_click.table[aprsdecode_click.selected].opf) {
               memcpy(aprsdecode_click.mhop,
                aprsdecode_click.table[aprsdecode_click.selected].opf->call,
                9u);
            }
            /*           IF beaconediting()
                THEN importbeacon(click.table[click.selected].opf) END;
                (* copy to beacon editor *) */
            mhtx = aprsmap_OPSENT;
            pandone = 0;
            aprsdecode_lums.errorstep = 0;
            if (aprsdecode_click.cmd=='=') {
               aprsdecode_lums.rf = useri_conf2int(useri_fLRF, 0UL, 0L, 100L,
                 30L)*10L;
            }
            else aprsdecode_lums.rf = 0L;
         }
      }
      else if (aprsdecode_click.cmd=='a') {
         if (aprsdecode_click.entries>0UL) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            if (aprsdecode_click.table[aprsdecode_click.selected].opf) {
               memcpy(aprsdecode_click.mhop,
                aprsdecode_click.table[aprsdecode_click.selected].opf->call,
                9u);
            }
            pandone = 0;
         }
         animate(aprsdecode_click.mhop, (unsigned long)aprsdecode_lums.fps, "\
", 1ul);
      }
      else if (aprsdecode_click.cmd=='0') setshowall();
      else if (aprsdecode_click.cmd=='1') View(0UL);
      else if (aprsdecode_click.cmd=='2') View(1UL);
      else if (aprsdecode_click.cmd=='3') View(2UL);
      else if (aprsdecode_click.cmd=='4') View(3UL);
      else if (aprsdecode_click.cmd=='b' || aprsdecode_click.cmd=='\010') {
         pop(&aprsdecode_mappos, &aprsdecode_initzoom, &aprsdecode_finezoom);
         maptool_mercator(aprsdecode_mappos.long0, aprsdecode_mappos.lat,
                aprsdecode_initzoom, &aprsdecode_inittilex,
                &aprsdecode_inittiley, &maptool_shiftx, &maptool_shifty);
      }
      else if (aprsdecode_click.cmd=='+') {
         /*
                 ELSIF click.cmd="D" THEN
                   IF click.table[click.selected].opf<>NIL 
                   THEN tracenew.call:=click.table[click.selected]
                .opf^.call END;      (* watch call for incoming data *)
         */
         zoominout(1, xosi_Shift);
      }
      else if (aprsdecode_click.cmd=='-') zoominout(0, xosi_Shift);
      else if (aprsdecode_click.cmd=='\310') zoominout(1, 1);
      else if (aprsdecode_click.cmd=='\311') zoominout(0, 1);
      else if (aprsdecode_click.cmd=='\237') MapPackage();
      else if (aprsdecode_click.cmd=='S') screenshot();
      else if (aprsdecode_click.cmd=='E') {
         aprsdecode_lums.errorstep = !aprsdecode_lums.errorstep;
         useri_sayonoff("Show errors", 12ul, aprsdecode_lums.errorstep);
      }
      else if (aprsdecode_click.cmd=='f') {
         useri_configbool(useri_fTRACKFILT,
                !useri_configon(useri_fTRACKFILT));
         useri_sayonoff("Trackfilter", 12ul,
                useri_configon(useri_fTRACKFILT));
      }
      else if (aprsdecode_click.cmd=='m') {
         aprsdecode_lums.moving = !aprsdecode_lums.moving;
         useri_sayonoff("Dimm not Moving", 16ul, aprsdecode_lums.moving);
      }
      else if (aprsdecode_click.cmd=='R') {
         if (aprsdecode_lums.rf==0L) aprsdecode_lums.rf = 300L;
         else aprsdecode_lums.rf = 0L;
      }
      else if (aprsdecode_click.cmd=='\022') {
         if (aprsdecode_click.withradio) closeradio();
         else aprsdecode_click.withradio = 1;
         aprsdecode_lums.wxcol = 0;
         useri_sayonoff("Radiorange Map", 15ul, aprsdecode_click.withradio);
      }
      else if (aprsdecode_click.cmd=='O') {
         /*
                 ELSIF click.cmd="P" THEN
                   IF click.withradio THEN closeradio ELSE click.withradio:=TRUE;
                 click.panorama:=TRUE END;
                   lums.wxcol:=0C;
                   sayonoff("Panorama", click.withradio);
         */
         if (aprsdecode_lums.obj==0L) {
            aprsdecode_lums.obj = 10L*useri_conf2int(useri_fLOBJ, 0UL, 0L,
                100L, 100L);
         }
         else aprsdecode_lums.obj = 0L;
         useri_sayonoff("Show Items/Objects", 19ul, aprsdecode_lums.obj!=0L);
      }
      else if (aprsdecode_click.cmd=='L') {
         if (aprsdecode_lums.text==0L) {
            aprsdecode_lums.text = 10L*useri_conf2int(useri_fLTEXT, 0UL, 0L,
                100L, 100L);
         }
         else aprsdecode_lums.text = 0L;
         useri_sayonoff("Labels", 7ul, aprsdecode_lums.text!=0L);
      }
      else if (X2C_CAP(aprsdecode_click.cmd)=='W') {
         if (xosi_Shift && aprsdecode_click.cmd=='W') {
            aprsdecode_click.cmd = 'w';
         }
         /*
                   IF click.cmd="W" THEN
                     IF lums.wxcol<>"W" THEN lums.wxcol:="W"
                ELSE lums.wxcol:=0C END;
                   ELSIF lums.wxcol<>"R" THEN lums.wxcol:="R"
                ELSE lums.wxcol:=0C END;
                   sayonoff("Wx Colormap", lums.wxcol<>0C);
         */
         if (aprsdecode_click.cmd=='W') {
            if (aprsdecode_lums.wxcol=='W') aprsdecode_lums.wxcol = 0;
            else if (aprsdecode_lums.wxcol=='w') aprsdecode_lums.wxcol = 'W';
            else aprsdecode_lums.wxcol = 'w';
         }
         else if (aprsdecode_lums.wxcol!='R') aprsdecode_lums.wxcol = 'R';
         else aprsdecode_lums.wxcol = 0;
         aprsdecode_click.mhop[0UL] = 0;
         closeradio();
      }
      else if (aprsdecode_click.cmd=='C') {
         if (aprspos_posvalid(aprsdecode_click.clickpos)) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.clickpos, &aprsdecode_mappos);
            aprsdecode_posinval(&aprsdecode_click.clickpos);
         }
         else if (aprspos_posvalid(aprsdecode_click.bubblpos)) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.bubblpos, &aprsdecode_mappos);
         }
      }
      else if (aprsdecode_click.cmd=='t'
                && aprspos_posvalid(aprsdecode_click.markpos)) {
         /* click to listwin line */
         maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.markpos, &aprsdecode_mappos);
         aprsdecode_click.marktime = aprsdecode_realtime;
         if (aprsdecode_click.mhop[0UL]) setshowall();
      }
      else if (aprsdecode_click.cmd=='\322'
                && aprspos_posvalid(clickwatchpos)) {
         /* click to watchcall popup */
         aprsdecode_click.markpos = clickwatchpos;
         maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.markpos, &aprsdecode_mappos);
         aprsdecode_click.marktime = aprsdecode_realtime;
         if (aprsdecode_click.mhop[0UL]) setshowall();
      }
      else if (aprsdecode_click.cmd=='c') {
         /* center */
         if ((aprsdecode_click.entries>0UL && aprsdecode_click.table[aprsdecode_click.selected]
                .opf) && aprspos_posvalid(aprsdecode_click.table[aprsdecode_click.selected].opf->lastpos)
                ) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.table[aprsdecode_click.selected]
                .opf->lastpos, &aprsdecode_mappos);
         }
         else if (aprspos_posvalid(aprsdecode_click.clickpos)) {
            push(aprsdecode_mappos, maptool_realzoom(aprsdecode_initzoom,
                aprsdecode_finezoom));
            maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_click.clickpos, &aprsdecode_mappos);
         }
      }
      else if (aprsdecode_click.cmd=='X') xytomark();
      else if (aprsdecode_click.cmd=='x') {
         /*
                   xytodeg(VAL(REAL,click.x), VAL(REAL,click.y),
                click.markpos);
                   click.marktime:=0;
                   click.markalti:=MAX(INTEGER);
                   postoconfig(click.markpos);
                   click.waysum:=0.0;
         */
         /* set marker 1 to object lastpos */
         /*
                   IF (click.entries>0)
                & (click.table[click.selected].opf<>NIL)
                   & posvalid(click.table[click.selected].opf^.lastpos)
                   THEN click.markpos:=click.table[click.selected]
                .opf^.lastpos; click.markalti:=MAX(INTEGER);
                   ELSE xytodeg(VAL(REAL,click.x), VAL(REAL,click.y),
                click.markpos) END;
                   click.marktime:=realtime;
         */
         clicktomark();
         useri_mainpop();
      }
      else if (aprsdecode_click.cmd=='y') {
         /* set marker 2 to object lastpos */
         if ((aprsdecode_click.entries>0UL && aprsdecode_click.table[aprsdecode_click.selected]
                .opf) && aprspos_posvalid(aprsdecode_click.table[aprsdecode_click.selected].opf->lastpos)
                ) {
            aprsdecode_click.measurepos = aprsdecode_click.table[aprsdecode_click.selected]
                .opf->lastpos;
         }
         else aprsdecode_click.measurepos = aprsdecode_click.clickpos;
      }
      else if (aprsdecode_click.cmd=='q') clickdelwaypoint();
      else if (aprsdecode_click.cmd=='/') {
         zoomtomarks(aprsdecode_click.markpos, aprsdecode_click.measurepos);
      }
      else if (aprsdecode_click.cmd=='\307') zoomtosquare();
      else if (aprsdecode_click.cmd==':') {
         aprsdecode_posinval(&aprsdecode_click.markpos);
         aprsdecode_posinval(&aprsdecode_click.measurepos);
         aprsdecode_click.waysum = 0.0f;
         aprsdecode_posinval(&aprsdecode_click.squerpos0);
         /*        mainpop; */
         useri_killallmenus();
         useri_sayonoff("Markers", 8ul, 0);
      }
      else if (aprsdecode_click.cmd=='@') {
         /*        textautosize(0, 0, 4, 2, "b", "Markers Off"); */
         useri_AddConfLine(useri_fCLICKMAP, 0U, "", 1ul);
         useri_AddConfLine(useri_fCLICKSYM, 0U, "", 1ul);
         useri_AddConfLine(useri_fCLICKTRACK, 0U, "", 1ul);
         useri_AddConfLine(useri_fCLICKTEXT, 0U, "", 1ul);
         useri_AddConfLine(useri_fCLICKWXSYM, 0U, "", 1ul);
         useri_killallmenus();
         useri_textautosize(0L, 0L, 4UL, 10UL, 'b', "\'ON Next Click\' Reset \
to Defaults", 34ul);
      }
      else if (aprsdecode_click.cmd=='Y') {
         aprsdecode_click.measurepos = aprsdecode_click.clickpos;
         copypastepos(aprsdecode_click.clickpos);
      }
      else if (aprsdecode_click.cmd=='~') {
         changecolor(aprsdecode_click.table[aprsdecode_click.selected].opf);
      }
      else if (aprsdecode_click.cmd=='A') {
         animate(aprsdecode_click.mhop, (unsigned long)aprsdecode_lums.fps, "\
", 1ul);
      }
      else if (aprsdecode_click.cmd=='\312') {
         useri_textautosize(0L, 0L, 4UL, 0UL, 'b', "Saving map.y4m", 15ul);
         animate(aprsdecode_click.mhop,
                (unsigned long)aprsdecode_lums.actfps, "map.y4m", 8ul);
      }
      else if (aprsdecode_click.cmd=='I') internstat();
      else if (aprsdecode_click.cmd=='\245') {
         aprsdecode_click.dryrun = 0;
         find();
      }
      else if (aprsdecode_click.cmd=='\\') {
         useri_helptext(0UL, 0UL, 0UL, 0UL, "en-shortcuts", 13ul);
      }
      else if (aprsdecode_click.cmd=='7') useri_Setmap(0UL);
      else if (aprsdecode_click.cmd=='8') useri_Setmap(1UL);
      else if (aprsdecode_click.cmd=='9') useri_Setmap(2UL);
      else if (aprsdecode_click.cmd=='Q') quit = 1;
      else if (aprsdecode_click.cmd=='e') aprsdecode_click.dryrun = 0;
      else if (aprsdecode_click.cmd=='\216') {
         importlog(aprsdecode_click.cmdatt);
         aprsdecode_click.cmdatt = 0;
      }
      else if (aprsdecode_click.cmd=='\011') toggview();
      makeimage(0);
      aprsdecode_click.cmd = 0;
   }
   else if ((aprsdecode_tracenew.call[0UL] && lastxupdate+2UL+slowupdate()
                <=aprsdecode_realtime)
                && !(aprsdecode_click.withradio && (aprspos_posvalid(aprsdecode_click.markpos)
                 || aprspos_posvalid(aprsdecode_click.measurepos)))) {
      follow();
      if (maptrys>0UL) {
         if (aprsdecode_click.watchlast) useri_refrinfo();
         makeimage(0);
      }
   }
   else if (aprsdecode_tracenew.winevent>0UL && lastxupdate+5UL+slowupdate()
                <=aprsdecode_realtime) {
      aprsdecode_tracenew.winevent = 0UL;
      if (aprsdecode_click.watchlast) useri_refrinfo();
      if (!(aprsdecode_click.withradio || aprspos_posvalid(aprsdecode_click.markpos)
                 && aprspos_posvalid(aprsdecode_click.measurepos))) {
         maptool_closesrtmfile();
      }
      makeimage(0);
   }
   else if (useri_newxsize>0UL) {
      /* window resize request */
      /*      IF newxsize<MINXSIZE THEN newxsize:=MINXSIZE */
      /*      ELSIF newxsize>MAXXSIZE THEN newxsize:=MAXXSIZE END; */
      /*      IF newysize<MINYSIZE THEN newysize:=MINYSIZE */
      /*      ELSIF newysize>MAXYSIZE THEN newysize:=MAXYSIZE END; */
      if ((useri_newxsize&1)) --useri_newxsize;
      if ((useri_newysize&1)) --useri_newysize;
      xosi_allocxbuf(useri_newxsize, useri_newysize);
      useri_allocimage(&image, (long)useri_newxsize, (long)useri_newysize,
                0);
      useri_allocimage(&rfimg, (long)useri_newxsize, (long)useri_newysize,
                0);
      maptool_xsize = (long)useri_newxsize;
      maptool_ysize = (long)useri_newysize;
      if (!useri_maximized) {
         useri_saveXYtocfg(useri_fXYSIZE, maptool_xsize, maptool_ysize);
      }
      useri_newxsize = 0UL;
      useri_newysize = 0UL;
      radio.wasradio = 0;
      aprsdecode_posinval(&radio.mappos);
      makeimage(0);
   }
   else if ((aprsdecode_lasttcprx+60UL>aprsdecode_realtime || aprsdecode_lastanyudprx+60UL>aprsdecode_realtime)
                 && laststatref+5UL<aprsdecode_realtime) {
      /*
            ELSIF xmouse.x>0 THEN                                      (* mouse move *)
              makeimage(FALSE);
      */
      if (aprsdecode_lasttcprx+50UL<aprsdecode_realtime) tooltips('n');
      useri_refresh = 1;
      laststatref = aprsdecode_realtime;
   }
   tooltips(' ');
   if (maptrys>0UL && maptime!=aprsdecode_realtime) {
      maptime = aprsdecode_realtime;
      --maptrys;
      if (maptool_IsMapLoaded()) makeimage(0);
      else if (maptrys==20UL) tooltips('m');
   }
   if (aprsdecode_click.bubblstr[0UL]) {
      useri_textbubble(aprsdecode_click.bubblpos, aprsdecode_click.bubblstr,
                50ul, aprsdecode_click.lastpoi);
      aprsdecode_click.bubblstr[0UL] = 0;
   }
   if (useri_refresh) useri_redraw(image);
   if (!logdone) {
      bootreadlog();
      ++aprsdecode_tracenew.winevent;
   }
} /* end MainEvent() */


static void getinitview(void)
{
   float z;
   aprsdecode_initzoom = 8L;
   aprsdecode_finezoom = 1.0f;
   z = 0.0f;
   useri_getview(useri_fVIEW, 0UL, &z, &aprsdecode_mappos);
   if (z!=0.0f) {
      aprsdecode_initzoom = (long)aprsdecode_trunc(z);
      aprsdecode_finezoom = (1.0f+z)-(float)aprsdecode_initzoom;
   }
   maptool_limpos(&aprsdecode_mappos);
   if (aprsdecode_parmzoom>0L) {
      aprsdecode_initzoom = aprsdecode_parmzoom;
      aprsdecode_finezoom = aprsdecode_parmfinezoom;
   }
   if (aprsdecode_initzoom<1L) aprsdecode_initzoom = 1L;
   else if (aprsdecode_initzoom>18L) aprsdecode_initzoom = 18L;
} /* end getinitview() */

static void killsave(long);


static void killsave(long signum)
{
   if (!quit && useri_configon(useri_fAUTOSAVE)) useri_saveconfig();
   InOut_WriteString("exit ", 6ul);
   InOut_WriteInt(signum, 0UL);
   osi_WrStrLn("!", 2ul);
   X2C_HALT((unsigned long)signum);
} /* end killsave() */


static char getch(char b[4096], long fd, long * len, long * p)
{
   if (*p>=*len) {
      *len = osi_RdBin(fd, (char *)b, 4096u/1u, 4096UL);
      if (*len<=0L) return 0;
      *p = 0L;
   }
   ++*p;
   return b[*p-1L];
} /* end getch() */


static long getword(long * p, long * len, long fd, char b[4096], char s[],
                unsigned long s_len)
{
   unsigned long i;
   i = 0UL;
   for (;;) {
      s[i] = getch(b, fd, len, p);
      if (s[i]==0) return -1L;
      if (s[i]=='\012') {
         s[i] = 0;
         return 0L;
      }
      if (s[i]==',') {
         s[i] = 0;
         return 1L;
      }
      if (i<s_len-1 && (unsigned char)s[i]>=' ') ++i;
   }
   return 0;
} /* end getword() */


static void rdmountains(void)
{
   /* import csv file with mountain name, pos, altitude */
   long r;
   long len;
   long p;
   long fd;
   aprsdecode_pMOUNTAIN pm;
   struct aprspos_POSITION pos;
   float alt;
   char b[4096];
   char s[1024];
   char name[100];
   char long0[100];
   char lat[100];
   char com[100];
   aprsdecode_mountains = 0;
   b[0U] = 0;
   useri_confstr(useri_fOSMDIR, s, 1024ul);
   aprsstr_Append(b, 4096ul, s, 1024ul);
   aprsstr_Append(b, 4096ul, "/poi.txt", 9ul);
   fd = osi_OpenRead(b, 4096ul);
   if (!osi_FdValid(fd)) return;
   p = 0L;
   len = 0L;
   for (;;) {
      r = getword(&p, &len, fd, b, com, 100ul);
      if (r>0L) {
         r = getword(&p, &len, fd, b, name, 100ul);
         if (r>0L) {
            r = getword(&p, &len, fd, b, s, 1024ul);
            if (r>0L) {
               r = getword(&p, &len, fd, b, lat, 100ul);
               if (r>0L) {
                  r = getword(&p, &len, fd, b, long0, 100ul);
                  if (r>0L) {
                     r = getword(&p, &len, fd, b, s, 1024ul);
                     if (r<0L || !aprsstr_StrToFix(&alt, s, 1024ul)) {
                        alt = 0.0f;
                     }
                     while (r>0L) r = getword(&p, &len, fd, b, s, 1024ul);
                  }
               }
            }
         }
      }
      if (r<0L) break;
      if ((((com[0U]!='#' && name[0U]) && aprsstr_StrToFix(&pos.lat, lat,
                100ul)) && aprsstr_StrToFix(&pos.long0, long0,
                100ul)) && aprspos_posvalid(pos)) {
         Storage_ALLOCATE((X2C_ADDRESS *) &pm,
                sizeof(struct aprsdecode_MOUNTAIN));
         if (pm==0) break;
         useri_debugmem.srtm += sizeof(struct aprsdecode_MOUNTAIN);
         aprsstr_Assign(pm->name, 32ul, name, 100ul);
         pm->pos.lat = pos.lat*1.7453292519444E-2f;
         pm->pos.long0 = pos.long0*1.7453292519444E-2f;
         if (alt<0.0f || alt>9999.0f) alt = 0.0f;
         pm->alt = (short)aprsdecode_trunc(alt);
         pm->next = aprsdecode_mountains;
         aprsdecode_mountains = pm;
      }
   }
   /*WrInt(pm^.alt, 10); WrStrLn(pm^.name); */
   osi_Close(fd);
} /* end rdmountains() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,1000000000l);
   aprstext_BEGIN();
   aprsstr_BEGIN();
   aprspos_BEGIN();
   aprsdecode_BEGIN();
   maptool_BEGIN();
   xosi_BEGIN();
   osi_BEGIN();
   useri_BEGIN();
   TimeConv_BEGIN();
   Storage_BEGIN();
   memset((char *) &useri_debugmem,(char)0,sizeof(struct useri__D0));
   useri_clrconfig();
   aprsdecode_initparms();
   aprsdecode_posinval(&aprsdecode_click.markpos);
   useri_loadconfig(0);
   maptool_loadfont();
   useri_maximized = 0;
   useri_getstartxysize(&maptool_xsize, &maptool_ysize);
   rdmountains();
   if (aprsdecode_initxsize>0L) maptool_xsize = aprsdecode_initxsize;
   /*  IF xsize<MINXSIZE THEN xsize:=MINXSIZE ELSIF xsize>MAXXSIZE THEN xsize:=MAXXSIZE END;
                 */
   if (aprsdecode_initysize>0L) maptool_ysize = aprsdecode_initysize;
   /*  IF ysize<MINYSIZE THEN ysize:=MINYSIZE ELSIF ysize>MAXYSIZE THEN ysize:=MAXYSIZE END;
                 */
   aprsdecode_mappos.long0 = 2.0943951023333E-1f;
   aprsdecode_mappos.lat = 8.5521133345278E-1f;
   getinitview();
   maptool_mercator(aprsdecode_mappos.long0, aprsdecode_mappos.lat,
                aprsdecode_initzoom, &aprsdecode_inittilex,
                &aprsdecode_inittiley, &maptool_shiftx, &maptool_shifty);
   maptool_center(maptool_xsize, maptool_ysize,
                maptool_realzoom(aprsdecode_initzoom, aprsdecode_finezoom),
                aprsdecode_mappos, &aprsdecode_mappos);
   /*
   WrInt(inittiley, 10); WrStrLn("=inittiley");
   WrFixed(shifty, 3, 10); WrStrLn("=shifty");
   */
   makegammatab();
   vidbuf = 0;
   image = 0;
   rfimg = 0;
   /*
     xsize:=initxsize;
     ysize:=initysize;  
   */
   useri_allocimage(&image, maptool_xsize, maptool_ysize, 0);
   /*
     allocimage(map, xsize, ysize, FALSE);
   */
   useri_allocimage(&rfimg, maptool_xsize, maptool_ysize, 0);
   aprsdecode_posinval(&alttabview.pos);
   aprsdecode_posinval(&clickwatchpos);
   stkpo = 0UL;
   stktop = 0UL;
   maptrys = 0UL;
   memset((char *) &aprsdecode_serialpid,(char)0,
                sizeof(struct xosi_PROCESSHANDLE));
   memset((char *) &aprsdecode_serialpid2,(char)0,
                sizeof(struct xosi_PROCESSHANDLE));
   aprsdecode_click.mhop[0UL] = 0;
   aprsdecode_click.zoomtox = -1L;
   aprsdecode_posinval(&aprsdecode_click.squerpos0);
   aprsdecode_posinval(&aprsdecode_click.measurepos);
   memset((char *) &aprsdecode_tracenew,(char)0,
                sizeof(struct aprsdecode__D3));
   pandone = 1;
   useri_newxsize = 0UL;
   useri_newysize = 0UL;
   cycleorder = 0UL;
   onetipp = 0;
   logdone = 0;
   uptime = TimeConv_time();
   withx = xosi_InitX("Aprsmap", 8ul, "Aprsmap", 8ul,
                (unsigned long)maptool_xsize,
                (unsigned long)maptool_ysize)>=0L;
   if (!withx) {
      osi_WrStrLn("cannot open xwindow, image generation only", 43ul);
   }
   xosi_Gammatab(aprsdecode_lums.gamma);
   aprsdecode_realtime = TimeConv_time();
   useri_initmenus();
   quit = 0;
   aprsdecode_tracenew.winevent = 1UL;
   aprsdecode_posinval(&newpos0);
   aprsdecode_posinval(&newpos1);
   signal(SIGTERM, killsave);
   signal(SIGINT, killsave);
   signal(SIGPIPE, killsave);
   if (withx) {
      aprsdecode_realtime = TimeConv_time();
      aprsdecode_rxidle = 0UL;
      useri_refresh = 1;
      aprsdecode_lastlooped = aprsdecode_realtime;
      while (!quit) {
         MainEvent();
         xosi_Eventloop(250000UL);
      }
      if (useri_configon(useri_fAUTOSAVE)) useri_saveconfig();
      xosi_StopProg(&aprsdecode_maploadpid);
      xosi_StopProg(&aprsdecode_serialpid);
      xosi_StopProg(&aprsdecode_serialpid2);
   }
/*
data structure op

ophist   (ophist2 is online ophist in logmode)

ophist > op1.next > op2.next > NIL
op.frames > frame1.next > frame2.next > NIL
frame1 > vardat1.lastref > frame5
frame2 > vardat2.lastref > frame4
frame3 > vardat1.lastref > frame5
frame4 > vardat2.lastref > frame4
frame5 > vardat1.lastref > frame5
*/
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
