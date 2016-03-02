/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif
#define sondeaprs_C_
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef udp_H_
#include "udp.h"
#endif

char sondeaprs_via[100];
char sondeaprs_destcall[100];
char sondeaprs_objname[100];
char sondeaprs_commentfn[1025];
char sondeaprs_sym[2];
unsigned long sondeaprs_beacontime;
unsigned long sondeaprs_lowaltbeacontime;
unsigned long sondeaprs_lowalt;
unsigned long sondeaprs_toport;
unsigned long sondeaprs_ipnum;
char sondeaprs_verb;
char sondeaprs_verb2;
char sondeaprs_nofilter;
long sondeaprs_comptyp;
long sondeaprs_micessid;
long sondeaprs_udpsock;
char sondeaprs_anyip;
char sondeaprs_sendmon;
char sondeaprs_dao;
/* encode demodulated sonde to aprs axudp by OE5DXL */
/*FROM TimeConv IMPORT time; */
/*FROM RealMath IMPORT ln; */
/*FROM Storage IMPORT ALLOCATE; */
#define sondeaprs_CR "\015"

#define sondeaprs_LF "\012"

#define sondeaprs_KNOTS 1.851984

#define sondeaprs_FEET 3.2808398950131

#define sondeaprs_LINESBUF 60
/* seconds pos history */

#define sondeaprs_PI 3.1415926535898

#define sondeaprs_DAYSEC 86400

#define sondeaprs_RAD 1.7453292519943E-2

#define sondeaprs_MAXHRMS 50.0
/* not send if gps h pos spreads more meters */

#define sondeaprs_MAXVRMS 500.0
/* not send if gps v pos spreads more meters */

#define sondeaprs_MAXAGE 86400
/* context lifetime */

enum ERRS {sondeaprs_ePRES, sondeaprs_eTEMP, sondeaprs_eHYG,
                sondeaprs_eSPEED, sondeaprs_eDIR, sondeaprs_eLAT,
                sondeaprs_eLONG, sondeaprs_eALT, sondeaprs_eMISS,
                sondeaprs_eRMS};


struct DATLINE;


struct DATLINE {
   double hpa;
   double temp;
   double hyg;
   double alt;
   double speed;
   double dir;
   double lat;
   double long0;
   /*-       climb, */
   double clb;
   unsigned long time0;
   unsigned long uptime;
};

struct POSITION;


struct POSITION {
   double long0;
   double lat;
};

typedef struct DATLINE DATS[60];

struct CONTEXT;

typedef struct CONTEXT * pCONTEXT;


struct CONTEXT {
   pCONTEXT next;
   char name[12];
   DATS dat;
   double speedsum;
   unsigned long speedcnt;
   unsigned long lastused;
   unsigned long lastbeacon;
   unsigned long commentline;
};

/*CRCL, CRCH: ARRAY[0..255] OF SET8;*/
static pCONTEXT contexts;
/*    dat     :ARRAY[0..LINESBUF-1] OF DATLINE; */

/*    speedsum:LONGREAL; */
/*    speedcnt:CARDINAL; */
static unsigned short chk;

/*    systime, lastbeacon:TIME; */
/*    commentline:CARDINAL; */

static unsigned long truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (unsigned long)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static unsigned long truncr(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (unsigned long)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncr() */


extern long sondeaprs_GetIp(char h[], unsigned long h_len, unsigned long * p,
                 unsigned long * ip, unsigned long * port)
{
   unsigned long n;
   unsigned long i;
   char ok0;
   long sondeaprs_GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   *p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((unsigned char)h[*p]>='0' && (unsigned char)h[*p]<='9') {
         ok0 = 1;
         n = (n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
         ++*p;
      }
      if (!ok0) {
         sondeaprs_GetIp_ret = -1L;
         goto label;
      }
      if (i<3UL) {
         if (h[*p]!='.' || n>255UL) {
            sondeaprs_GetIp_ret = -1L;
            goto label;
         }
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[*p]!=':' || n>255UL) {
            sondeaprs_GetIp_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         sondeaprs_GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++*p;
   } /* end for */
   sondeaprs_GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return sondeaprs_GetIp_ret;
} /* end GetIp() */


static void comment0(char buf[], unsigned long buf_len, unsigned long uptime,
                 unsigned long sats, double hrms, unsigned long * linec)
{
   long len;
   long lc;
   long eol;
   long bol;
   long i;
   long f;
   char fb[32768];
   char h[100];
   buf[0UL] = 0;
   len = 0L;
   if (sondeaprs_commentfn[0UL]) {
      f = osi_OpenRead(sondeaprs_commentfn, 1025ul);
      if (f>=0L) {
         len = osi_RdBin(f, (char *)fb, 32768u/1u, 32767UL);
         osic_Close(f);
         while (len>0L && (unsigned char)fb[len-1L]<=' ') --len;
         if (len>0L && len<32767L) {
            fb[len] = '\012';
            ++len;
         }
         do {
            lc = (long)*linec;
            eol = 0L;
            for (;;) {
               bol = eol;
               while (eol<len && fb[eol]!='\012') ++eol;
               if (eol>=len) {
                  bol = eol;
                  if (*linec) {
                     lc = 1L;
                     *linec = 0UL;
                  }
                  break;
               }
               if (fb[bol]!='#') {
                  if (lc==0L) {
                     ++*linec;
                     break;
                  }
                  --lc;
               }
               ++eol;
            }
         } while (lc);
         if (eol+2L>=bol && fb[bol]=='%') {
            if (fb[bol+1L]=='u') {
               /* insert uptime */
               if (uptime>0UL) {
                  strncpy(fb," powerup h:m:s ",32768u);
                  aprsstr_TimeToStr(uptime, h, 100ul);
                  aprsstr_Append(fb, 32768ul, h, 100ul);
               }
               else fb[0] = 0;
            }
            else if (fb[bol+1L]=='v') {
               /* insert version */
               strncpy(fb," sondemod(c) 0.5",32768u);
            }
            else if (fb[bol+1L]=='s') {
               /* insert sat count */
               if (sats>0UL) {
                  strncpy(fb," Sats ",32768u);
                  aprsstr_IntToStr((long)sats, 1UL, h, 100ul);
                  aprsstr_Append(fb, 32768ul, h, 100ul);
               }
               else fb[0] = 0;
            }
            else if (fb[bol+1L]=='r') {
               /* hrms +3m from tropomodel */
               if (sats>4UL) {
                  strncpy(fb," hdil=",32768u);
                  aprsstr_FixToStr((float)(hrms+3.0), 2UL, h, 100ul);
                  aprsstr_Append(fb, 32768ul, h, 100ul);
                  aprsstr_Append(fb, 32768ul, "m", 2ul);
               }
               else fb[0] = 0;
            }
            else fb[0] = 0;
            bol = 0L;
            eol = (long)aprsstr_Length(fb, 32768ul);
         }
         i = 0L;
         while (bol<eol && i<(long)(buf_len-1)) {
            buf[i] = fb[bol];
            ++i;
            ++bol;
         }
         buf[i] = 0;
      }
      else if (sondeaprs_verb) osi_WrStrLn("beacon file not found", 22ul);
   }
} /* end comment() */


static void sendudp(char buf[], unsigned long buf_len, long len)
{
   long i;
   /*  crc:CARDINAL;  */
   X2C_PCOPY((void **)&buf,buf_len);
   /*
     IF withcrc THEN
       crc:=UDPCRC(buf, len);
       buf[len]:=CHR(crc MOD 256);
       buf[len+1]:=CHR(crc DIV 256);
       INC(len, 2);
     END;
     AppCRC(buf, len);
   */
   i = udpsend(sondeaprs_udpsock, buf, len, sondeaprs_toport,
                sondeaprs_ipnum);
   X2C_PFREE(buf);
/*
FOR i:=0 TO upos-2 DO IO.WrHex(ORD(buf[i]), 3) END; IO.WrLn;
*/
} /* end sendudp() */


static char num(unsigned long n)
{
   return (char)(n%10UL+48UL);
} /* end num() */


static unsigned long dao91(double x)
/* radix91(xx/1.1) of dddmm.mmxx */
{
   double a;
   a = fabs(x);
   return ((truncc((a-(double)(float)truncc(a))*6.E+5)%100UL)*20UL+11UL)
                /22UL;
} /* end dao91() */


static void sendaprs(unsigned long comp0, unsigned long micessid, char dao,
                unsigned long time0, unsigned long uptime, char mycall[],
                unsigned long mycall_len, char destcall[],
                unsigned long destcall_len, char via[],
                unsigned long via_len, char sym[], unsigned long sym_len,
                char obj[], unsigned long obj_len, double lat, double long0,
                double alt, double course, double speed,
                unsigned long goodsats, double hrms, char comm[],
                unsigned long comm_len, unsigned long * commentcnt)
{
   char ds[201];
   char h[201];
   char b[201];
   char raw[361];
   long rp;
   unsigned long micdest;
   unsigned long nl;
   unsigned long n;
   unsigned long i;
   double a;
   char tmp;
   X2C_PCOPY((void **)&mycall,mycall_len);
   X2C_PCOPY((void **)&destcall,destcall_len);
   X2C_PCOPY((void **)&via,via_len);
   X2C_PCOPY((void **)&sym,sym_len);
   X2C_PCOPY((void **)&obj,obj_len);
   X2C_PCOPY((void **)&comm,comm_len);
   b[0] = 0;
   aprsstr_Append(b, 201ul, mycall, mycall_len);
   micdest = aprsstr_Length(b, 201ul)+1UL;
   aprsstr_Append(b, 201ul, ">", 2ul);
   aprsstr_Append(b, 201ul, destcall, destcall_len);
   if (micessid>0UL) {
      aprsstr_Append(b, 201ul, "-", 2ul);
      aprsstr_Append(b, 201ul, (char *)(tmp = (char)(micessid+48UL),&tmp),
                1u/1u);
   }
   if (via[0UL]) {
      aprsstr_Append(b, 201ul, ",", 2ul);
      aprsstr_Append(b, 201ul, via, via_len);
   }
   if (comp0==0UL) {
      /* uncompressed */
      aprsstr_Append(b, 201ul, ":;", 3ul);
      aprsstr_Assign(h, 201ul, obj, obj_len);
      aprsstr_Append(h, 201ul, "         ", 10ul);
      h[9U] = 0;
      aprsstr_Append(b, 201ul, h, 201ul);
      aprsstr_Append(b, 201ul, "*", 2ul);
      aprsstr_DateToStr(time0, ds, 201ul);
      ds[0U] = ds[11U];
      ds[1U] = ds[12U];
      ds[2U] = ds[14U];
      ds[3U] = ds[15U];
      ds[4U] = ds[17U];
      ds[5U] = ds[18U];
      ds[6U] = 0;
      aprsstr_Append(b, 201ul, ds, 201ul);
      aprsstr_Append(b, 201ul, "h", 2ul);
      i = aprsstr_Length(b, 201ul);
      a = fabs(lat);
      n = truncr(a);
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncr((a-(double)(float)n)*6000.0);
      b[i] = num(n/1000UL);
      ++i;
      b[i] = num(n/100UL);
      ++i;
      b[i] = '.';
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      if (lat>=0.0) b[i] = 'N';
      else b[i] = 'S';
      ++i;
      b[i] = sym[0UL];
      ++i;
      a = fabs(long0);
      n = truncr(a);
      b[i] = num(n/100UL);
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncr((a-(double)(float)n)*6000.0);
      b[i] = num(n/1000UL);
      ++i;
      b[i] = num(n/100UL);
      ++i;
      b[i] = '.';
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      if (lat>=0.0) b[i] = 'E';
      else b[i] = 'W';
      ++i;
      b[i] = sym[1UL];
      ++i;
      if (speed>0.5) {
         n = truncr(course+1.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
         b[i] = '/';
         ++i;
         n = truncr(speed*5.3996146834962E-1+0.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
      if (alt>0.5) {
         b[i] = '/';
         ++i;
         b[i] = 'A';
         ++i;
         b[i] = '=';
         ++i;
         n = truncr(fabs(alt*3.2808398950131+0.5));
         if (alt>=0.0) b[i] = num(n/100000UL);
         else b[i] = '-';
         ++i;
         b[i] = num(n/10000UL);
         ++i;
         b[i] = num(n/1000UL);
         ++i;
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
   }
   else if (comp0==1UL) {
      /* compressed */
      aprsstr_Append(b, 201ul, ":!", 3ul);
      i = aprsstr_Length(b, 201ul);
      b[i] = sym[0UL];
      ++i;
      if (lat<90.0) n = truncr((90.0-lat)*3.80926E+5);
      else n = 0UL;
      b[i] = (char)(33UL+n/753571UL);
      ++i;
      b[i] = (char)(33UL+(n/8281UL)%91UL);
      ++i;
      b[i] = (char)(33UL+(n/91UL)%91UL);
      ++i;
      b[i] = (char)(33UL+n%91UL);
      ++i;
      if (long0>(-180.0)) n = truncr((180.0+long0)*1.90463E+5);
      else n = 0UL;
      b[i] = (char)(33UL+n/753571UL);
      ++i;
      b[i] = (char)(33UL+(n/8281UL)%91UL);
      ++i;
      b[i] = (char)(33UL+(n/91UL)%91UL);
      ++i;
      b[i] = (char)(33UL+n%91UL);
      ++i;
      b[i] = sym[1UL];
      ++i;
      if (speed>0.5) {
         b[i] = (char)(33UL+truncr(course)/4UL);
         ++i;
         b[i] = (char)(33UL+truncr((double)(osic_ln((float)
                (speed*5.3996146834962E-1+1.0))*1.29935872129E+1f)));
         ++i;
         b[i] = '_';
         ++i;
      }
      else if (alt>0.5) {
         if (alt*3.2808398950131>1.0) {
            n = truncr((double)(osic_ln((float)(alt*3.2808398950131))*500.5f)
                );
         }
         else n = 0UL;
         if (n>=8281UL) n = 8280UL;
         b[i] = (char)(33UL+n/91UL);
         ++i;
         b[i] = (char)(33UL+n%91UL);
         ++i;
         b[i] = 'W';
         ++i;
      }
      else {
         b[i] = ' ';
         ++i;
         b[i] = ' ';
         ++i;
         b[i] = '_';
         ++i;
      }
      if (speed>0.5) {
         b[i] = '/';
         ++i;
         b[i] = 'A';
         ++i;
         b[i] = '=';
         ++i;
         n = truncr(alt*3.2808398950131+0.5);
         if (alt>=0.0) b[i] = num(n/10000UL);
         else b[i] = '-';
         ++i;
         b[i] = num(n/10000UL);
         ++i;
         b[i] = num(n/1000UL);
         ++i;
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
   }
   else if (comp0==2UL) {
      /* mic-e */
      aprsstr_Append(b, 201ul, ":`", 3ul);
      i = micdest;
      nl = truncr(fabs(long0));
      n = truncr(fabs(lat));
      b[i] = (char)(80UL+n/10UL);
      ++i;
      b[i] = (char)(80UL+n%10UL);
      ++i;
      n = truncr((fabs(lat)-(double)(float)n)*6000.0);
      b[i] = (char)(80UL+n/1000UL);
      ++i;
      b[i] = (char)(48UL+32UL*(unsigned long)(lat>=0.0)+(n/100UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(unsigned long)(nl<10UL || nl>=100UL)+(n/10UL)
                %10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(unsigned long)(long0<0.0)+n%10UL);
      i = aprsstr_Length(b, 201ul);
      if (nl<10UL) b[i] = (char)(nl+118UL);
      else if (nl>=100UL) {
         if (nl<110UL) b[i] = (char)(nl+8UL);
         else b[i] = (char)(nl-72UL);
      }
      else b[i] = (char)(nl+28UL);
      ++i;
      nl = truncr((fabs(long0)-(double)(float)nl)*6000.0); /* long min*100 */
      n = nl/100UL;
      if (n<10UL) {
         n += 60UL;
      }
      b[i] = (char)(n+28UL);
      ++i;
      b[i] = (char)(nl%100UL+28UL);
      ++i;
      n = truncr(speed*5.3996146834962E-1+0.5);
      b[i] = (char)(n/10UL+28UL);
      ++i;
      nl = truncr(course);
      b[i] = (char)(32UL+(n%10UL)*10UL+nl/100UL);
      ++i;
      b[i] = (char)(28UL+nl%100UL);
      ++i;
      b[i] = sym[1UL];
      ++i;
      b[i] = sym[0UL];
      ++i;
      if (alt>0.5) {
         if (alt>(-1.E+4)) n = truncr(alt+10000.5);
         else n = 0UL;
         b[i] = (char)(33UL+(n/8281UL)%91UL);
         ++i;
         b[i] = (char)(33UL+(n/91UL)%91UL);
         ++i;
         b[i] = (char)(33UL+n%91UL);
         ++i;
         b[i] = '}';
         ++i;
      }
   }
   if (dao) {
      b[i] = '!';
      ++i;
      b[i] = 'w';
      ++i;
      b[i] = (char)(33UL+dao91(lat));
      ++i;
      b[i] = (char)(33UL+dao91(long0));
      ++i;
      b[i] = '!';
      ++i;
   }
   b[i] = 0;
   aprsstr_Append(b, 201ul, comm, comm_len);
   comment0(h, 201ul, uptime, goodsats, hrms, commentcnt);
   aprsstr_Append(b, 201ul, h, 201ul);
   /*  Append(b, CR+LF); */
   if (aprsstr_Length(mycall, mycall_len)>=3UL) {
      if (!sondeaprs_sendmon) {
         aprsstr_mon2raw(b, 201ul, raw, 361ul, &rp);
         if (rp>0L) sendudp(raw, 361ul, rp);
      }
      else sendudp(b, 201ul, (long)(aprsstr_Length(b, 201ul)+1UL));
   }
   if (sondeaprs_verb) osi_WrStrLn(b, 201ul);
   X2C_PFREE(mycall);
   X2C_PFREE(destcall);
   X2C_PFREE(via);
   X2C_PFREE(sym);
   X2C_PFREE(obj);
   X2C_PFREE(comm);
} /* end sendaprs() */

#define sondeaprs_Z 48


static void degtostr(double d, char lat, char form, char s[],
                unsigned long s_len)
{
   unsigned long i;
   unsigned long n;
   if (s_len-1<11UL) {
      s[0UL] = 0;
      return;
   }
   if (form=='2') i = 7UL;
   else if (form=='3') i = 8UL;
   else i = 9UL;
   if (d<0.0) {
      d = -d;
      if (lat) s[i] = 'S';
      else s[i+1UL] = 'W';
   }
   else if (lat) s[i] = 'N';
   else s[i+1UL] = 'E';
   if (form=='2') {
      /* DDMM.MMNDDMM.MME */
      n = truncr(d*3.4377467707849E+5+0.5);
      s[0UL] = (char)((n/600000UL)%10UL+48UL);
      i = (unsigned long)!lat;
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
      n = truncr(d*3.4377467707849E+6+0.5);
      s[0UL] = (char)((n/6000000UL)%10UL+48UL);
      i = (unsigned long)!lat;
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
      n = truncr(d*2.062648062471E+5+0.5);
      s[0UL] = (char)((n/360000UL)%10UL+48UL);
      i = (unsigned long)!lat;
      s[i] = (char)((n/36000UL)%10UL+48UL);
      ++i;
      s[i] = (char)((n/3600UL)%10UL+48UL);
      ++i;
      s[i] = 'o';
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


static void postostr(struct POSITION pos, char form, char s[],
                unsigned long s_len)
{
   char h[32];
   degtostr(pos.lat, 1, form, s, s_len);
   aprsstr_Append(s, s_len, "/", 2ul);
   degtostr(pos.long0, 0, form, h, 32ul);
   aprsstr_Append(s, s_len, h, 32ul);
} /* end postostr() */

#define sondeaprs_RAD0 1.7453292519943E-2


static void WrDeg(double la, double lo)
{
   char s[31];
   struct POSITION pos;
   pos.lat = la*1.7453292519943E-2;
   pos.long0 = lo*1.7453292519943E-2;
   postostr(pos, '2', s, 31ul);
   osi_WrStr(s, 31ul);
} /* end WrDeg() */


static void show(struct DATLINE d)
{
   char s[31];
   osic_WrFixed((float)d.hpa, 1L, 6UL);
   osi_WrStr("hPa ", 5ul);
   if (d.temp<100.0) {
      osic_WrFixed((float)d.temp, 1L, 5UL);
      osi_WrStr("C ", 3ul);
   }
   osic_WrINT32(truncr(d.hyg), 2UL);
   osi_WrStr("% ", 3ul);
   osic_WrINT32((unsigned long)(long)X2C_TRUNCI(d.speed*3.6,X2C_min_longint,
                X2C_max_longint), 3UL);
   osi_WrStr("km/h ", 6ul);
   osic_WrINT32(truncr(d.dir), 3UL);
   osi_WrStr("dir ", 5ul);
   WrDeg(d.lat, d.long0);
   osi_WrStr(" ", 2ul);
   /*WrFixed(d.gpsalt, 1, 8); WrStr("m "); */
   osic_WrINT32((unsigned long)(long)X2C_TRUNCI(d.alt,X2C_min_longint,
                X2C_max_longint), 1UL);
   osi_WrStr("m ", 3ul);
   osic_WrFixed((float)d.clb, 1L, 5UL);
   osi_WrStr("m/s ", 5ul);
   aprsstr_TimeToStr(d.time0, s, 31ul);
   osi_WrStr(s, 31ul);
} /* end show() */


static char Checkval(const double a[], unsigned long a_len,
                const unsigned long t[], unsigned long t_len, double err,
                double min0, double max0, double unitspers)
{
   unsigned long i;
   double y;
   double m;
   double k;
   unsigned long tmp;
   tmp = a_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if (a[i]<min0 || a[i]>max0) return 0;
      /* >=1 value out of range */
      if (i>0UL && t[i]<t[i-1UL]) return 0;
      if (i==tmp) break;
   } /* end for */
   /* time goes back */
   k = 0.0;
   tmp = a_len-1;
   i = 1UL;
   if (i<=tmp) for (;; i++) {
      if (t[i]>t[0UL]) {
         y = X2C_DIVL(a[i]-a[0UL],(double)(float)(t[i]-t[0UL]));
         if (fabs(y)>unitspers) return 0;
         k = k+y; /* median slope */
      }
      if (i==tmp) break;
   } /* end for */
   k = X2C_DIVL(k,(double)(float)(a_len-1));
   m = 0.0;
   tmp = a_len-1;
   i = 1UL;
   if (i<=tmp) for (;; i++) {
      if (t[i]>t[0UL]) {
         y = fabs((a[i]-a[0UL])-(double)(float)(t[i]-t[0UL])*k);
         if (y>m) m = y;
      }
      if (i==tmp) break;
   } /* end for */
   /*  k:=ABS(k); */
   /*  IF k<err THEN k:=err END; */
   /*
   FOR i:=0 TO HIGH(a) DO WrFixed(a[i], 5, 0); WrStr("/"); END;
   WrFixed(k, 5, 0); WrStr(" "); WrFixed(m, 5, 0); WrStr(" ");
                WrFixed(err, 5, 0); WrStr(" ");
   WrInt(t[HIGH(t)]-t[0],6); WrStrLn(" k,m,err,timespan"); 
   */
   return m<err;
} /* end Checkval() */

/*
PROCEDURE climb(VAR d:DATS);
VAR i:CARDINAL;
    k:REAL;
BEGIN
  k:=0.0;
  FOR i:=0 TO HIGH(d)-1 DO k:=k+d[i+1].alt-d[i].alt END;
                (* median slope *)
  d[0].climb:=-k/FLOAT(HIGH(d));
END climb;
*/
#define sondeaprs_MAXTIMESPAN 15


static void Checkvals(const DATS d, unsigned short * e)
{
   unsigned long n;
   unsigned long i;
   double v[4];
   unsigned long t[4];
   unsigned long tmp;
   *e = 0U;
   n = 3UL;
   for (i = 0UL; i<=3UL; i++) {
      v[n-i] = d[i].hpa;
      t[n-i] = d[i].time0;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 2000.0, 0.0, 1100.0, 200.0)) *e |= 0x1U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].temp;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 200.0, (-150.0), 80.0, 5.0)) *e |= 0x2U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].hyg;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 100.0, 0.0, 100.0, 20.0)) *e |= 0x4U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].speed;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 100.0, 0.0, 300.0, 100.0)) *e |= 0x8U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].dir;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 360.0, 0.0, 359.0, 1000.0)) *e |= 0x10U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].lat;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 4.5454545454545E-4, (-85.0), 85.0,
                2.7272727272727E-3)) *e |= 0x20U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].long0;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 4.5454545454545E-4, (-180.0), 180.0,
                2.7272727272727E-3)) *e |= 0x40U;
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      v[n-i] = d[i].alt;
      if (i==tmp) break;
   } /* end for */
   if (!Checkval(v, 4ul, t, 4ul, 100.0, 5.0, 60000.0, 1000.0)) *e |= 0x80U;
   /*
     FOR i:=0 TO n DO 
       IF (i>0) & (ta<>(d[i].time+1) MOD (3600*24)) THEN INCL(e, eMISS) END;
       ta:=d[i].time;
     END;
   */
   if (((d[0U].time0+86400UL)-d[n].time0)%86400UL>15UL) *e |= 0x100U;
} /* end Checkvals() */


static void shift(DATS d)
{
   unsigned long i;
   for (i = 59UL; i>=1UL; i--) {
      d[i] = d[i-1UL];
   } /* end for */
} /* end shift() */


static pCONTEXT findcontext(char n[], unsigned long n_len, unsigned long t)
{
   pCONTEXT p;
   pCONTEXT c;
   pCONTEXT findcontext_ret;
   X2C_PCOPY((void **)&n,n_len);
   c = contexts;
   while (c && !aprsstr_StrCmp(c->name, 12ul, n, n_len)) c = c->next;
   if (c==0) {
      c = contexts;
      while (c && c->lastused+86400UL>t) c = c->next;
      if (c==0) {
         osic_alloc((X2C_ADDRESS *) &c, sizeof(struct CONTEXT));
         memset((X2C_ADDRESS)c,(char)0,sizeof(struct CONTEXT));
         c->next = contexts;
         contexts = c;
      }
      else {
         /* reuse old context */
         p = c->next;
         memset((X2C_ADDRESS)c,(char)0,sizeof(struct CONTEXT));
         c->next = p;
      }
      aprsstr_Assign(c->name, 12ul, n, n_len);
   }
   if (c) c->lastused = t;
   findcontext_ret = c;
   X2C_PFREE(n);
   return findcontext_ret;
} /* end findcontext() */

/*
PROCEDURE highresstr(hrstr:ARRAY OF CHAR; dat-:DATS; bt:TIME);
CONST STEP=2;
      DEGUNIT=PI2/360.0/100000.0;  (* 1/100000 deg = 1.1111m*)

TYPE  VEC=ARRAY[0..LINESBUF-1] OF RECORD lat, long, alt:REAL; time:TIME END;

VAR i,n:CARDINAL;
    vec:VEC;
    t:TIME;
BEGIN
  hrstr[0]:=0C;
  t:=dat[0].time;
  IF (bt<=STEP) OR (t<bt) THEN RETURN END;

  i:=0;
  n:=0;
  LOOP
    DEC(t, STEP);
    IF (i>HIGH(dat)) OR (t+bt<dat[0].time) THEN EXIT END;
    
    IF t<=dat[i].time THEN
      vec[n].lat :=dat[i].lat;
      vec[n].long:=dat[i].long;
      vec[n].alt :=dat[i].alt;
      vec[n].time:=dat[i].time;
      INC(n);
    END;
    INC(i);
  END;
  






END highresstr;
*/

extern void sondeaprs_senddata(double lat, double long0, double alt,
                double speed, double dir, double clb, double hp, double hyg,
                double temp, double ozon, double otemp, double dewp,
                double mhz, double hrms, double vrms, unsigned long sattime,
                unsigned long uptime, char objname[],
                unsigned long objname_len, unsigned long almanachage,
                unsigned long goodsats, char usercall[],
                unsigned long usercall_len, unsigned long calperc)
{
   unsigned char e;
   pCONTEXT ct;
   char h[251];
   char s[251];
   unsigned long systime;
   unsigned long bt;
   struct CONTEXT * anonym;
   X2C_PCOPY((void **)&objname,objname_len);
   if (aprsstr_Length(usercall, usercall_len)<3UL) {
      osi_WrStrLn("no tx witout <mycall>", 22ul);
      goto label;
   }
   if (aprsstr_Length(objname, objname_len)<3UL) {
      osi_WrStrLn("no tx witout <objectname>", 26ul);
      goto label;
   }
   systime = osic_time();
   ct = findcontext(objname, objname_len, systime);
   if (ct) {
      { /* with */
         struct CONTEXT * anonym = ct;
         shift(anonym->dat);
         anonym->speedsum = anonym->speedsum+speed;
         ++anonym->speedcnt;
         anonym->dat[0U].hpa = hp;
         anonym->dat[0U].temp = temp;
         anonym->dat[0U].hyg = hyg;
         anonym->dat[0U].alt = alt;
         anonym->dat[0U].speed = X2C_DIVL(anonym->speedsum,
                (double)anonym->speedcnt);
         anonym->dat[0U].dir = dir;
         anonym->dat[0U].lat = X2C_DIVL(lat,1.7453292519943E-2);
         anonym->dat[0U].long0 = X2C_DIVL(long0,1.7453292519943E-2);
         /*    dat[0].time:=(sattime+DAYSEC-GPSTIMECORR) MOD DAYSEC; */
         anonym->dat[0U].time0 = sattime%86400UL;
         anonym->dat[0U].uptime = sattime%86400UL;
         anonym->dat[0U].clb = clb;
         /*    climb(dat); */
         Checkvals(anonym->dat, &chk);
         if (hrms>50.0 || vrms>500.0) chk |= 0x200U;
         if (sondeaprs_verb) {
            osi_WrStrLn("", 1ul);
            show(anonym->dat[0U]);
            if (almanachage) {
               osi_WrStr(" AlmAge ", 9ul);
               osic_WrFixed((float)(X2C_DIVL((double)almanachage,3600.0)),
                1L, 3UL);
               osi_WrStrLn("h ", 3ul);
            }
            else osi_WrStrLn("", 1ul);
            for (e = sondeaprs_ePRES;; e++) {
               if (X2C_IN((long)e,10,chk)) {
                  switch ((unsigned)e) {
                  case sondeaprs_ePRES:
                     osi_WrStr("p", 2ul);
                     break;
                  case sondeaprs_eTEMP:
                     osi_WrStr("t", 2ul);
                     break;
                  case sondeaprs_eHYG:
                     osi_WrStr("h", 2ul);
                     break;
                  case sondeaprs_eSPEED:
                     osi_WrStr("v", 2ul);
                     break;
                  case sondeaprs_eDIR:
                     osi_WrStr("d", 2ul);
                     break;
                  case sondeaprs_eLAT:
                     osi_WrStr("y", 2ul);
                     break;
                  case sondeaprs_eLONG:
                     osi_WrStr("x", 2ul);
                     break;
                  case sondeaprs_eALT:
                     osi_WrStr("a", 2ul);
                     break;
                  case sondeaprs_eMISS:
                     osi_WrStr("s", 2ul);
                     break;
                  case sondeaprs_eRMS: /*WrFixed(vrms, 1,5); WrStr(" ");
                WrFixed(hrms, 1,5);*/
                     osi_WrStr("r", 2ul);
                     break;
                  } /* end switch */
               }
               if (e==sondeaprs_eRMS) break;
            } /* end for */
         }
         if (clb<0.0 && anonym->dat[0U].alt<(double)sondeaprs_lowalt) {
            bt = sondeaprs_lowaltbeacontime;
         }
         else bt = sondeaprs_beacontime;
         if ((bt>0UL && anonym->lastbeacon+bt<=systime)
                && (sondeaprs_nofilter || (chk&0x3E0U)==0U)) {
            strncpy(s,"Clb=",251u);
            aprsstr_FixToStr((float)clb, 2UL, h, 251ul); /*dat[0].climb*/
            aprsstr_Append(s, 251ul, h, 251ul);
            aprsstr_Append(s, 251ul, "m/s", 4ul);
            if ((0x1U & chk)==0 && anonym->dat[0U].hpa>=1.0) {
               aprsstr_Append(s, 251ul, " p=", 4ul);
               aprsstr_FixToStr((float)anonym->dat[0U].hpa, 2UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "hPa", 4ul);
            }
            if ((0x2U & chk)==0) {
               aprsstr_Append(s, 251ul, " t=", 4ul);
               aprsstr_FixToStr((float)anonym->dat[0U].temp, 2UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "C", 2ul);
            }
            if (hyg>=0.5 && (0x4U & chk)==0) {
               aprsstr_Append(s, 251ul, " h=", 4ul);
               aprsstr_IntToStr((long)truncc(anonym->dat[0U].hyg+0.5), 1UL,
                h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "%", 2ul);
            }
            if (ozon>0.1) {
               aprsstr_Append(s, 251ul, " o3=", 5ul);
               aprsstr_FixToStr((float)ozon, 2UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "mPa ti=", 8ul);
               aprsstr_FixToStr((float)otemp, 2UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "C", 2ul);
            }
            if (dewp>(-100.0) && dewp<100.0) {
               aprsstr_Append(s, 251ul, " dp=", 5ul);
               aprsstr_FixToStr((float)dewp, 2UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "C", 2ul);
            }
            if (calperc>0UL && calperc<100UL) {
               aprsstr_Append(s, 251ul, " calibration ", 14ul);
               aprsstr_IntToStr((long)calperc, 1UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "%", 2ul);
            }
            if (mhz>0.0) {
               aprsstr_Append(s, 251ul, " ", 2ul);
               aprsstr_FixToStr((float)mhz, 3UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "MHz", 4ul);
            }
            /*        highresstr(hrstr, dat, bt); */
            sendaprs(0UL, 0UL, sondeaprs_dao, anonym->dat[0U].time0, uptime,
                usercall, usercall_len, sondeaprs_destcall, 100ul,
                sondeaprs_via, 100ul, sondeaprs_sym, 2ul, objname,
                objname_len, anonym->dat[0U].lat, anonym->dat[0U].long0,
                anonym->dat[0U].alt,
                (double)(float)(truncc(anonym->dat[0U].dir)%360UL),
                anonym->dat[0U].speed*3.6, goodsats, hrms, s, 251ul,
                &anonym->commentline);
            anonym->lastbeacon = systime;
            anonym->speedcnt = 0UL;
            anonym->speedsum = 0.0;
         }
      }
   }
   label:;
   X2C_PFREE(objname);
} /* end senddata() */


extern void sondeaprs_BEGIN(void)
{
   static int sondeaprs_init = 0;
   if (sondeaprs_init) return;
   sondeaprs_init = 1;
   osi_BEGIN();
   aprsstr_BEGIN();
   contexts = 0;
   sondeaprs_udpsock = -1L;
   sondeaprs_commentfn[0UL] = 0;
   strncpy(sondeaprs_destcall,"APLWS2",100u);
   sondeaprs_via[0UL] = 0;
   strncpy(sondeaprs_sym,"/O",2u);
   sondeaprs_objname[0UL] = 0;
   sondeaprs_beacontime = 30UL;
   sondeaprs_lowaltbeacontime = 0UL;
   sondeaprs_lowalt = 1000UL;
   sondeaprs_nofilter = 0;
/*  FILL(ADR(dat), 0C, SIZE(dat)); */
/*  lastbeacon:=0; */
/*  commentline:=0; */
/*  speedcnt:=0; */
/*  speedsum:=0.0; */
}

