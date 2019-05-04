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
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif



char sondeaprs_via[100];
char sondeaprs_destcall[100];
char sondeaprs_objname[100];
char sondeaprs_commentfn[1025];
char sondeaprs_csvfilename[1025];
char sondeaprs_sym[2];
uint32_t sondeaprs_beacontime;
uint32_t sondeaprs_lowaltbeacontime;
uint32_t sondeaprs_lowalt;
uint32_t sondeaprs_toport;
uint32_t sondeaprs_maxsenddistance;
uint32_t sondeaprs_expire;
uint32_t sondeaprs_ipnum;
char sondeaprs_verb;
char sondeaprs_verb2;
char sondeaprs_nofilter;
int32_t sondeaprs_comptyp;
int32_t sondeaprs_udpsock;
char sondeaprs_anyip;
char sondeaprs_sendmon;
char sondeaprs_dao;
struct aprsstr_POSITION sondeaprs_mypos;
float sondeaprs_myalt;
/* encode demodulated sonde to aprs axudp by OE5DXL */
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
   double hrms;
   double vrms;
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
   uint32_t time0;
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
   uint32_t speedcnt;
   uint32_t lastused;
   uint32_t lastbeacon;
   uint32_t commentline;
};

/*CRCL, CRCH: ARRAY[0..255] OF SET8;*/
static pCONTEXT contexts;

static uint16_t chk;

static char egmoff;

static char mhzfromsonde;


static uint32_t truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.E+9) return 2000000000UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


extern int32_t sondeaprs_GetIp(char h[], uint32_t h_len,
                uint32_t * p, uint32_t * ip, uint32_t * port)
{
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t sondeaprs_GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   *p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
         ok0 = 1;
         n = (n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
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


static void wrcsv(uint32_t sattime, const char typstr[],
                uint32_t typstr_len, char objname[],
                uint32_t objname_len, double lat, double long0,
                 double alt, double speed, double dir,
                double clb, double egmalt, double og,
                double mhz, uint32_t goodsats, uint32_t burstKill,
                uint32_t uptime, double hp, double hyg,
                double temp, double ozon, double otemp,
                double pumpmA, double pumpv,
                const struct sondeaprs_SDRBLOCK sdr, double dist,
                double azi, double ele, const char fullid[],
                uint32_t fullid_len)
{
   int32_t fd;
   char h[1000];
   char s[1000];
   X2C_PCOPY((void **)&objname,objname_len);
   fd = osi_OpenAppend(sondeaprs_csvfilename, 1025ul);
   if (fd<0L) {
      fd = osi_OpenWrite(sondeaprs_csvfilename, 1025ul);
      strncpy(s,"Date,Time,Type,Name,lat,long,alt,speed,dir,clb,egmalt,og,mhz\
,sats,bk,uptime,hPa,hum,temp,ozon,ozont,pumpmA,pumpV,RxMHz,AFC,maxAFC,rssi,de\
v,dist,azimuth,elevation,ser\012",1000u);
   }
   else s[0] = 0;
   if (fd<0L) {
      osi_WrStrLn("cannot write csv-file", 22ul);
      goto label;
   }
   if (s[0U]) {
      osi_WrBin(fd, (char *)s, 1000u/1u, aprsstr_Length(s, 1000ul));
                /* csv headline */
   }
   s[0] = 0;
   if (sattime>=86400UL) {
      aprsstr_DateToStr(sattime, s, 1000ul);
      s[10U] = 0;
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (sattime>0UL) {
      aprsstr_TimeToStr(sattime%86400UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_Append(s, 1000ul, typstr, typstr_len);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_Append(s, 1000ul, objname, objname_len);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_FixToStr((float)(lat*5.7295779513082E+1), 6UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_FixToStr((float)(long0*5.7295779513082E+1), 6UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(alt,X2C_min_longint,
                X2C_max_longint), 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_FixToStr((float)(speed*3.6), 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_IntToStr((int32_t)X2C_TRUNCI(dir,X2C_min_longint,
                X2C_max_longint), 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   aprsstr_FixToStr((float)clb, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (egmalt>(-1.E+4) && egmalt<1.E+5) {
      aprsstr_IntToStr((int32_t)X2C_TRUNCI(egmalt,X2C_min_longint,
                X2C_max_longint), 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (og>(-1.E+4) && og<1.E+5) {
      aprsstr_IntToStr((int32_t)X2C_TRUNCI(og,X2C_min_longint,
                X2C_max_longint), 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (mhz>0.1) {
      aprsstr_FixToStr((float)(mhz+0.005), 3UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (goodsats>0UL) {
      aprsstr_IntToStr((int32_t)goodsats, 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (burstKill>0UL) {
      aprsstr_IntToStr((int32_t)(burstKill-1UL), 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (uptime>0UL) {
      aprsstr_TimeToStr(uptime, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (hp>0.1 && hp<2000.0) {
      aprsstr_FixToStr((float)hp, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (hyg>0.1 && hyg<=100.0) {
      aprsstr_IntToStr((int32_t)X2C_TRUNCI(hyg,X2C_min_longint,
                X2C_max_longint), 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (temp>(-1000.0) && temp<1000.0) {
      aprsstr_FixToStr((float)temp, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (ozon>0.1) {
      aprsstr_FixToStr((float)ozon, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (ozon>0.1) {
      aprsstr_FixToStr((float)otemp, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (ozon>0.1 && pumpmA>0.1) {
      aprsstr_FixToStr((float)pumpmA, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (ozon>0.1 && pumpv>0.1) {
      aprsstr_FixToStr((float)pumpv, 3UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (sdr.freq) {
      aprsstr_FixToStr((float)sdr.freq*0.00001f+0.0005f, 4UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (sdr.maxafc) {
      aprsstr_IntToStr(sdr.afc, 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (sdr.maxafc) {
      aprsstr_IntToStr((int32_t)sdr.maxafc, 1UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (sdr.db) {
      aprsstr_FixToStr((float)sdr.db*0.1f, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (sdr.name[0UL]) aprsstr_Append(s, 1000ul, sdr.name, 4ul);
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (dist>=0.0) {
      aprsstr_FixToStr((float)dist, 4UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (azi>=0.0) {
      aprsstr_FixToStr((float)azi, 2UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",", 2ul);
   if (ele>=(-90.0)) {
      aprsstr_FixToStr((float)ele, 3UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   if (fullid[0UL]) {
      aprsstr_Append(s, 1000ul, ",", 2ul);
      aprsstr_Append(s, 1000ul, fullid, fullid_len);
   }
   aprsstr_Append(s, 1000ul, "\012", 2ul);
   osi_WrBin(fd, (char *)s, 1000u/1u, aprsstr_Length(s, 1000ul));
   osic_Close(fd);
   label:;
   X2C_PFREE(objname);
} /* end wrcsv() */


static double egm96corr(double lat, double long0,
                double alt)
{
   struct aprsstr_POSITION pos;
   char ok0;
   pos.lat = (float)lat;
   pos.long0 = (float)long0;
   alt = alt-(double)libsrtm_egm96(pos, &ok0);
   if (ok0) return alt;
   if (sondeaprs_verb) {
      osi_WrStrLn("--- no EGM96 data - only raw Sonde Altitudes", 45ul);
   }
   egmoff = 1;
   return (-1.E+5);
} /* end egm96corr() */


static double getoverground(double lat, double long0,
                double alt)
{
   struct aprsstr_POSITION pos;
   float resolution;
   double srtm;
   libsrtm_srtmmaxmem = 1000000UL;
   pos.lat = (float)lat;
   pos.long0 = (float)long0;
   srtm = (double)libsrtm_getsrtm(pos, 1UL, &resolution);
   if (srtm<10000.0 && srtm>(-1000.0)) {
      if (alt<=(-3.E+4)) return srtm;
      /* srtm request */
      return alt-srtm;
   }
   /*  ELSIF verb THEN WrStrLn("---no SRTM data"); */
   return (-1.E+5);
} /* end getoverground() */


static void comment0(char buf[], uint32_t buf_len, uint32_t uptime,
                uint32_t sats, double hrms, uint32_t * linec,
                const struct sondeaprs_SDRBLOCK sdr, double myazi,
                double myele, double mydist)
{
   int32_t len;
   int32_t lc;
   int32_t eol;
   int32_t bol;
   int32_t i;
   int32_t f;
   char fb[32768];
   char hb[120];
   char h[120];
   buf[0UL] = 0;
   len = 0L;
   if (sondeaprs_commentfn[0UL]) {
      f = osi_OpenRead(sondeaprs_commentfn, 1025ul);
      if (f>=0L) {
         len = osi_RdBin(f, (char *)fb, 32768u/1u, 32767UL);
         osic_Close(f);
         while (len>0L && (uint8_t)fb[len-1L]<=' ') --len;
         if (len>0L && len<32767L) {
            fb[len] = '\012';
            ++len;
         }
         do {
            lc = (int32_t)*linec;
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
         hb[0] = 0;
         while (eol+2L>=bol && fb[bol]=='%') {
            if (fb[bol+1L]=='u') {
               /* insert uptime */
               if (uptime>0UL) {
                  aprsstr_Append(hb, 120ul, " powerup h:m:s ", 16ul);
                  aprsstr_TimeToStr(uptime, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
               }
            }
            else if (fb[bol+1L]=='n') {
               /* insert framenumber */
               if (uptime>0UL) {
                  aprsstr_Append(hb, 120ul, " FN=", 5ul);
                  aprsstr_IntToStr((int32_t)uptime, 1UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
               }
            }
            else if (fb[bol+1L]=='v') {
               /* insert version */
               aprsstr_Append(hb, 120ul, " sondemod 1.34", 15ul);
            }
            else if (fb[bol+1L]=='s') {
               /* insert sat count */
               if (sats>0UL) {
                  aprsstr_Append(hb, 120ul, " Sats=", 7ul);
                  aprsstr_IntToStr((int32_t)sats, 1UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
               }
            }
            else if (fb[bol+1L]=='r') {
               /* hrms +3m from tropomodel */
               if (sats>4UL && hrms>=0.0) {
                  aprsstr_Append(hb, 120ul, " hdil=", 7ul);
                  aprsstr_FixToStr((float)(hrms+3.0), 2UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
                  aprsstr_Append(hb, 120ul, "m", 2ul);
               }
               else fb[0] = 0;
            }
            else if (X2C_CAP(fb[bol+1L])=='F') {
               /* sdr freq and afc */
               if ((sdr.valid && (!mhzfromsonde || fb[bol+1L]=='F'))
                && sdr.freq) {
                  aprsstr_Append(hb, 120ul, " rx=", 5ul);
                  aprsstr_IntToStr((int32_t)(sdr.freq/100UL), 1UL, h,
                120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
                  if (sdr.maxafc) {
                     aprsstr_Append(hb, 120ul, "(", 2ul);
                     if (sdr.afc>=0L) aprsstr_Append(hb, 120ul, "+", 2ul);
                     aprsstr_IntToStr(sdr.afc, 1UL, h, 120ul);
                     aprsstr_Append(hb, 120ul, h, 120ul);
                     aprsstr_Append(hb, 120ul, "/", 2ul);
                     aprsstr_IntToStr((int32_t)sdr.maxafc, 1UL, h, 120ul);
                     aprsstr_Append(hb, 120ul, h, 120ul);
                     aprsstr_Append(hb, 120ul, ")", 2ul);
                  }
               }
            }
            else if (fb[bol+1L]=='d') {
               /* rssi */
               if (sdr.valid && sdr.db) {
                  aprsstr_Append(hb, 120ul, " rssi=", 7ul);
                  aprsstr_FixToStr((float)sdr.db*0.1f, 2UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
                  aprsstr_Append(hb, 120ul, "dB", 3ul);
               }
            }
            else if (fb[bol+1L]=='l') {
               /* label */
               if (sdr.valid && sdr.name[0UL]) {
                  aprsstr_Append(hb, 120ul, " dev=", 6ul);
                  aprsstr_Append(hb, 120ul, sdr.name, 4ul);
               }
            }
            else if (fb[bol+1L]=='D') {
               /* distance */
               if (mydist>=0.0) {
                  aprsstr_Append(hb, 120ul, " dist=", 7ul);
                  aprsstr_FixToStr((float)mydist, 4UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
               }
            }
            else if (fb[bol+1L]=='A') {
               /* azimuth */
               if (myazi>=0.0) {
                  aprsstr_Append(hb, 120ul, " azimuth=", 10ul);
                  aprsstr_IntToStr((int32_t)X2C_TRUNCI(myazi,
                X2C_min_longint,X2C_max_longint), 1UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
               }
            }
            else if (fb[bol+1L]=='E') {
               /* elevation */
               if (myele>=0.0) {
                  aprsstr_Append(hb, 120ul, " elevation=", 12ul);
                  aprsstr_FixToStr((float)myele, 3UL, h, 120ul);
                  aprsstr_Append(hb, 120ul, h, 120ul);
               }
            }
            bol += 2L;
         }
         aprsstr_Assign(buf, buf_len, hb, 120ul);
         i = (int32_t)aprsstr_Length(buf, buf_len);
         while (bol<eol && i<(int32_t)(buf_len-1)) {
            buf[i] = fb[bol];
            ++i;
            ++bol;
         }
         buf[i] = 0;
      }
      else if (sondeaprs_verb) osi_WrStrLn("beacon file not found", 22ul);
   }
} /* end comment() */


static void sendudp(char buf[], uint32_t buf_len, int32_t len)
{
   int32_t i;
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


static char num(uint32_t n)
{
   return (char)(n%10UL+48UL);
} /* end num() */


static uint32_t dao91(double x)
/* radix91(xx/1.1) of dddmm.mmxx */
{
   double a;
   a = fabs(x);
   return ((truncc((a-(double)(float)truncc(a))*6.E+5)%100UL)
                *20UL+11UL)/22UL;
} /* end dao91() */


static void sendaprs(uint32_t comp0, uint32_t micessid, char dao,
                uint32_t time0, uint32_t uptime, char mycall[],
                uint32_t mycall_len, char destcall[],
                uint32_t destcall_len, char via[], uint32_t via_len,
                char sym[], uint32_t sym_len, char obj[],
                uint32_t obj_len, double lat, double long0,
                double alt, double course, double speed,
                uint32_t goodsats, double hrms, char comm[],
                uint32_t comm_len, uint32_t * commentcnt,
                const struct sondeaprs_SDRBLOCK sdr, double myazi,
                double myele, double mydist)
{
   char ds[201];
   char h[201];
   char b[201];
   char raw[361];
   int32_t rp;
   uint32_t micdest;
   uint32_t nl;
   uint32_t n;
   uint32_t i;
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
      aprsstr_Append(b, 201ul, (char *)(tmp = (char)(micessid+48UL),
                &tmp), 1u/1u);
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
      n = osi_realcard((float)a);
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = osi_realcard((float)((a-(double)(float)n)*6000.0));
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
      n = osi_realcard((float)a);
      b[i] = num(n/100UL);
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = osi_realcard((float)((a-(double)(float)n)*6000.0));
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
      if (long0>=0.0) b[i] = 'E';
      else b[i] = 'W';
      ++i;
      b[i] = sym[1UL];
      ++i;
      if (speed>0.5) {
         n = osi_realcard((float)(course+1.5));
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
         b[i] = '/';
         ++i;
         n = osi_realcard((float)(speed*5.3996146834962E-1+0.5));
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
         n = osi_realcard((float)fabs(alt*3.2808398950131+0.5));
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
      if (lat<90.0) n = osi_realcard((float)((90.0-lat)*3.80926E+5));
      else n = 0UL;
      b[i] = (char)(33UL+n/753571UL);
      ++i;
      b[i] = (char)(33UL+(n/8281UL)%91UL);
      ++i;
      b[i] = (char)(33UL+(n/91UL)%91UL);
      ++i;
      b[i] = (char)(33UL+n%91UL);
      ++i;
      if (long0>(-180.0)) {
         n = osi_realcard((float)((180.0+long0)*1.90463E+5));
      }
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
         b[i] = (char)(33UL+osi_realcard((float)course)/4UL);
         ++i;
         b[i] = (char)(33UL+osi_realcard(osic_ln((float)
                (speed*5.3996146834962E-1+1.0))*1.29935872129E+1f));
         ++i;
         b[i] = '_';
         ++i;
      }
      else if (alt>0.5) {
         if (alt*3.2808398950131>1.0) {
            n = osi_realcard(osic_ln((float)(alt*3.2808398950131))*500.5f)
                ;
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
         n = osi_realcard((float)(alt*3.2808398950131+0.5));
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
      nl = osi_realcard((float)fabs(long0));
      n = osi_realcard((float)fabs(lat));
      b[i] = (char)(80UL+n/10UL);
      ++i;
      b[i] = (char)(80UL+n%10UL);
      ++i;
      n = osi_realcard((float)((fabs(lat)-(double)(float)n)
                *6000.0));
      b[i] = (char)(80UL+n/1000UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(lat>=0.0)+(n/100UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(nl<10UL || nl>=100UL)+(n/10UL)
                %10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(long0<0.0)+n%10UL);
      i = aprsstr_Length(b, 201ul);
      if (nl<10UL) b[i] = (char)(nl+118UL);
      else if (nl>=100UL) {
         if (nl<110UL) b[i] = (char)(nl+8UL);
         else b[i] = (char)(nl-72UL);
      }
      else b[i] = (char)(nl+28UL);
      ++i;
      nl = osi_realcard((float)((fabs(long0)-(double)(float)nl)
                *6000.0)); /* long min*100 */
      n = nl/100UL;
      if (n<10UL) n += 60UL;
      b[i] = (char)(n+28UL);
      ++i;
      b[i] = (char)(nl%100UL+28UL);
      ++i;
      n = osi_realcard((float)(speed*5.3996146834962E-1+0.5));
      b[i] = (char)(n/10UL+28UL);
      ++i;
      nl = osi_realcard((float)course);
      b[i] = (char)(32UL+(n%10UL)*10UL+nl/100UL);
      ++i;
      b[i] = (char)(28UL+nl%100UL);
      ++i;
      b[i] = sym[1UL];
      ++i;
      b[i] = sym[0UL];
      ++i;
      if (alt>0.5) {
         if (alt>(-1.E+4)) n = osi_realcard((float)(alt+10000.5));
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
   comment0(h, 201ul, uptime, goodsats, hrms, commentcnt, sdr, myazi, myele,
                mydist);
   aprsstr_Append(b, 201ul, h, 201ul);
   /*  Append(b, CR+LF); */
   if (aprsstr_Length(mycall, mycall_len)>=3UL) {
      if (!sondeaprs_sendmon) {
         aprsstr_mon2raw(b, 201ul, raw, 361ul, &rp);
         if (rp>0L) sendudp(raw, 361ul, rp);
      }
      else sendudp(b, 201ul, (int32_t)(aprsstr_Length(b, 201ul)+1UL));
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


static void degtostr(double d, char lat, char form,
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
   if (d<0.0) {
      d = -d;
      if (lat) s[i] = 'S';
      else s[i+1UL] = 'W';
   }
   else if (lat) s[i] = 'N';
   else s[i+1UL] = 'E';
   if (form=='2') {
      /* DDMM.MMNDDMM.MME */
      n = osi_realcard((float)(d*3.4377467707849E+5+0.5));
      s[0UL] = (char)((n/600000UL)%10UL+48UL);
      i = (uint32_t)!lat;
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
      n = osi_realcard((float)(d*3.4377467707849E+6+0.5));
      s[0UL] = (char)((n/6000000UL)%10UL+48UL);
      i = (uint32_t)!lat;
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
      n = osi_realcard((float)(d*2.062648062471E+5+0.5));
      s[0UL] = (char)((n/360000UL)%10UL+48UL);
      i = (uint32_t)!lat;
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
                uint32_t s_len)
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
   osic_WrINT32(osi_realcard((float)d.hyg), 2UL);
   osi_WrStr("% ", 3ul);
   osic_WrINT32(osi_realcard((float)(d.speed*3.6)), 3UL);
   osi_WrStr("km/h ", 6ul);
   osic_WrINT32(osi_realcard((float)d.dir), 3UL);
   osi_WrStr("dir ", 5ul);
   WrDeg(d.lat, d.long0);
   osi_WrStr(" ", 2ul);
   /*WrFixed(d.gpsalt, 1, 8); WrStr("m "); */
   if (d.alt>=(-2.E+4) && d.alt>=1.E+5) {
      osic_WrINT32((uint32_t)osi_realint((float)d.alt), 1UL);
      osi_WrStr("m ", 3ul);
   }
   osic_WrFixed((float)d.clb, 1L, 5UL);
   osi_WrStr("m/s ", 5ul);
   aprsstr_TimeToStr(d.time0, s, 31ul);
   osi_WrStr(s, 31ul);
} /* end show() */


static char Checkval(const double a[], uint32_t a_len,
                const uint32_t t[], uint32_t t_len, double err,
                double min0, double max0,
                double unitspers)
{
   uint32_t i;
   double y;
   double m;
   double k;
   uint32_t tmp;
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

#define sondeaprs_MAXTIMESPAN 20


static void Checkvals(const DATS d, uint16_t * e)
{
   uint32_t n;
   uint32_t i;
   double v[4];
   uint32_t t[4];
   uint32_t tmp;
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
   tmp = n;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      if (d[i].hrms>50.0) *e |= 0x200U;
      if (d[i].vrms>500.0) *e |= 0x200U;
      if (d[i].lat==0.0 && d[i].long0==0.0) *e |= 0x200U;
      if (i==tmp) break;
   } /* end for */
   /*
     FOR i:=0 TO n DO 
       IF (i>0) & (ta<>(d[i].time+1) MOD (3600*24)) THEN INCL(e, eMISS) END;
       ta:=d[i].time;
     END;
   */
   if (((d[0U].time0+86400UL)-d[n].time0)%86400UL>20UL) *e |= 0x100U;
} /* end Checkvals() */


static void shift(DATS d)
{
   uint32_t i;
   for (i = 59UL; i>=1UL; i--) {
      d[i] = d[i-1UL];
   } /* end for */
} /* end shift() */


static pCONTEXT findcontext(char n[], uint32_t n_len, uint32_t t)
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
         osic_alloc((char * *) &c, sizeof(struct CONTEXT));
         memset((char *)c,(char)0,sizeof(struct CONTEXT));
         c->next = contexts;
         contexts = c;
      }
      else {
         /* reuse old context */
         p = c->next;
         memset((char *)c,(char)0,sizeof(struct CONTEXT));
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

static void elevation(double * el, double * c,
                struct aprsstr_POSITION home, double homealt,
                struct aprsstr_POSITION dist, double distalt)
{
   float z1;
   float y1;
   float x1;
   float z0;
   float y00;
   float x0;
   float sb;
   float r;
   float s;
   float b;
   float a;
   *el = (-1000.0);
   aprspos_wgs84s(home.lat, home.long0, (float)(homealt*0.001), &x0, &y00,
                 &z0);
   aprspos_wgs84s(dist.lat, dist.long0, (float)(distalt*0.001), &x1, &y1,
                &z1);
   a = osic_sqrt(x0*x0+y00*y00+z0*z0);
   b = osic_sqrt(x1*x1+y1*y1+z1*z1);
   x1 = x1-x0;
   y1 = y1-y00;
   z1 = z1-z0;
   *c = (double)osic_sqrt(x1*x1+y1*y1+z1*z1);
   /* halbwinkelsatz */
   s = (float)(((double)(a+b)+*c)*0.5);
   if (s==0.0f) return;
   r = (float)(X2C_DIVL((double)((s-a)*(s-b))*((double)s-*c),
                (double)s));
   if (r<=0.0f) return;
   r = osic_sqrt(r);
   sb = s-b;
   if (sb==0.0f) *el = 90.0;
   else {
      /*  el:=(360.0/PI)*arctan(r/(s-a))-90.0; */
      *el = (double)(1.1459155902616E+2f*osic_arctan(X2C_DIVR(r,
                sb))-90.0f);
   }
} /* end elevation() */


extern void sondeaprs_senddata(double lat, double long0,
                double alt, double speed, double dir,
                double clb, double hp, double hyg,
                double temp, double ozon, double otemp,
                double pumpmA, double pumpv, double mhz,
                double hrms, double vrms, uint32_t sattime,
                uint32_t uptime, char objname[],
                uint32_t objname_len, uint32_t almanachage,
                uint32_t goodsats, char usercall[],
                uint32_t usercall_len, uint32_t calperc,
                uint32_t burstKill, char force, char typstr[],
                uint32_t typstr_len, char fullid[],
                uint32_t fullid_len, struct sondeaprs_SDRBLOCK sdr)
{
   uint8_t e;
   pCONTEXT ct;
   char h[251];
   char s[251];
   uint32_t systime;
   uint32_t bt;
   double btalt;
   double og;
   double egmalt;
   struct aprsstr_POSITION dpos;
   double mygrounddist;
   double mydist;
   double myele;
   double myazi;
   struct CONTEXT * anonym;
   X2C_PCOPY((void **)&objname,objname_len);
   og = (-3.2E+4);
   btalt = alt;
   if (!egmoff) {
      egmalt = egm96corr(lat, long0, alt);
                /* make NN out of wgs84 altitude */
      if (egmalt>(-1000.0)) {
         og = getoverground(lat, long0, egmalt);
         if (og>=0.0) btalt = og;
      }
      else if (fabs(egmalt-alt)>250.0) {
         osic_WrFixed((float)(egmalt-alt), 2L, 1UL);
         osi_WrStrLn("m egm96 correction?", 20ul);
      }
   }
   /*- azimuth elevation distance */
   myazi = (-2.E+4);
   myele = (-2.E+4);
   mydist = (-2.E+4);
   mygrounddist = 0.0;
   dpos.lat = (float)lat;
   dpos.long0 = (float)long0;
   if (aprspos_posvalid(sondeaprs_mypos) && aprspos_posvalid(dpos)) {
      if (sondeaprs_myalt<(-2.E+5f)) {
         /* try to get own alt from srtm */
         sondeaprs_myalt = (float)getoverground((double)
                sondeaprs_mypos.lat, (double)sondeaprs_mypos.long0,
                (-1.E+5));
         if ((sondeaprs_verb && sondeaprs_myalt>(-2.E+4f))
                && sondeaprs_myalt<20000.0f) {
            osi_WrStrLn("", 1ul);
            osi_WrStr("got Your altitude from SRTM as ", 32ul);
            osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI(sondeaprs_myalt,
                X2C_min_longint,X2C_max_longint), 1UL);
            osi_WrStrLn("m or overwrite with -N", 23ul);
         }
      }
      myazi = (double)aprspos_azimuth(sondeaprs_mypos, dpos);
      mygrounddist = (double)aprspos_distance(sondeaprs_mypos, dpos);
      if (mygrounddist>2.E+7) mygrounddist = 2.E+7;
      if ((egmalt>=(-2.E+4) && sondeaprs_myalt>=(-2.E+4f))
                && sondeaprs_myalt<=20000.0f) {
         elevation(&myele, &mydist, sondeaprs_mypos,
                (double)sondeaprs_myalt, dpos, egmalt);
      }
   }
   /*- azimuth elevation distance */
   if (sondeaprs_csvfilename[0UL]) {
      wrcsv(sattime, typstr, typstr_len, objname, objname_len, lat, long0,
                alt, speed, dir, clb, egmalt, og, mhz, goodsats, burstKill,
                uptime, hp, hyg, temp, ozon, otemp, pumpmA, pumpv, sdr,
                mydist, myazi, myele, fullid, fullid_len);
   }
   if (aprsstr_Length(usercall, usercall_len)<3UL) {
      osi_WrStrLn("no tx without <mycall>", 23ul);
      goto label;
   }
   if (aprsstr_Length(objname, objname_len)<3UL) {
      osi_WrStrLn("no tx witout <objectname>", 26ul);
      goto label;
   }
   systime = osic_time();
   mhzfromsonde = 0;
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
         anonym->dat[0U].lat = lat*5.7295779513082E+1;
         anonym->dat[0U].long0 = long0*5.7295779513082E+1;
         if (sattime>=86400UL) anonym->dat[0U].time0 = sattime%86400UL;
         else anonym->dat[0U].time0 = osic_time()%86400UL;
         anonym->dat[0U].clb = clb;
         anonym->dat[0U].hrms = hrms;
         anonym->dat[0U].vrms = vrms;
         /*    climb(dat); */
         Checkvals(anonym->dat, &chk);
         /*    IF (hrms>MAXHRMS) OR (vrms>MAXVRMS) THEN INCL(chk, eRMS) END;
                */
         if (sondeaprs_verb) {
            osi_WrStrLn("", 1ul);
            show(anonym->dat[0U]);
            if (almanachage) {
               osi_WrStr(" AlmAge ", 9ul);
               osic_WrFixed((float)(X2C_DIVL((double)almanachage,
                3600.0)), 1L, 3UL);
               osi_WrStrLn("h ", 3ul);
            }
            else osi_WrStrLn("", 1ul);
            for (e = sondeaprs_ePRES;; e++) {
               if (X2C_IN((int32_t)e,10,chk)) {
                  switch ((unsigned)e) {
                  case sondeaprs_eSPEED:
                     /*              ePRES : WrStr("p"); */
                     /*             |eTEMP : WrStr("t"); */
                     /*             |eHYG  : WrStr("h"); */
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
         if (clb<(-0.25) && btalt<(double)sondeaprs_lowalt) {
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
               aprsstr_FixToStr((float)anonym->dat[0U].hpa, 2UL, h,
                251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "hPa", 4ul);
            }
            if ((0x2U & chk)==0) {
               aprsstr_Append(s, 251ul, " t=", 4ul);
               aprsstr_FixToStr((float)anonym->dat[0U].temp, 2UL, h,
                251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "C", 2ul);
            }
            if (hyg>=0.5 && (0x4U & chk)==0) {
               aprsstr_Append(s, 251ul, " h=", 4ul);
               aprsstr_IntToStr((int32_t)truncc(anonym->dat[0U].hyg+0.5),
                1UL, h, 251ul);
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
               if (pumpmA>0.1) {
                  aprsstr_Append(s, 251ul, " Pump=", 7ul);
                  aprsstr_IntToStr((int32_t)truncc(pumpmA), 1UL, h, 251ul);
                  aprsstr_Append(s, 251ul, h, 251ul);
                  aprsstr_Append(s, 251ul, "mA", 3ul);
               }
               if (pumpv>0.1) {
                  aprsstr_Append(s, 251ul, " ", 2ul);
                  aprsstr_FixToStr((float)pumpv, 2UL, h, 251ul);
                  aprsstr_Append(s, 251ul, h, 251ul);
                  aprsstr_Append(s, 251ul, "V", 2ul);
               }
            }
            /*
                    IF (dewp>-100.0) & (dewp<100.0) THEN
                      Append(s, " dp=");
                      FixToStr(dewp, 2, h); Append(s, h);
                      Append(s, "C");
                    END;
            */
            if (calperc>0UL && calperc<100UL) {
               aprsstr_Append(s, 251ul, " calibration ", 14ul);
               aprsstr_IntToStr((int32_t)calperc, 1UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "%", 2ul);
            }
            if (mhz>0.0) {
               aprsstr_Append(s, 251ul, " ", 2ul);
               aprsstr_FixToStr((float)mhz, 3UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "MHz", 4ul);
               mhzfromsonde = 1; /* mhz sent so do not with %f */
            }
            if (typstr[0UL]) {
               aprsstr_Append(s, 251ul, " Type=", 7ul);
               aprsstr_Append(s, 251ul, typstr, typstr_len);
            }
            if (og>=0.0 && og<=(double)sondeaprs_lowalt) {
               aprsstr_Append(s, 251ul, " OG=", 5ul);
               aprsstr_IntToStr((int32_t)X2C_TRUNCI(og,X2C_min_longint,
                X2C_max_longint), 1UL, h, 251ul);
               aprsstr_Append(s, 251ul, h, 251ul);
               aprsstr_Append(s, 251ul, "m", 2ul);
            }
            /* appended by SQ7BR */
            if (burstKill==1UL || burstKill==2UL) {
               aprsstr_Append(s, 251ul, " BK=", 5ul);
               if (burstKill==1UL) {
                  aprsstr_Append(s, 251ul, "Off", 4ul);
               }
               else aprsstr_Append(s, 251ul, "On", 3ul);
            }
            /* appended by SQ7BR */
            if (fullid[0UL]) {
               aprsstr_Append(s, 251ul, " ser=", 6ul);
               aprsstr_Append(s, 251ul, fullid, fullid_len);
            }
            if (force) aprsstr_Append(s, 251ul, " Unchecked-Data", 16ul);
            if (sondeaprs_expire>0UL && (systime>sattime+sondeaprs_expire || systime+sondeaprs_expire<sattime)
                ) {
               if (sondeaprs_verb) {
                  if (sattime==0UL) strncpy(h,"got no Time",251u);
                  else {
                     aprsstr_IntToStr((int32_t)(sattime-systime), 1UL, h,
                251ul);
                     aprsstr_Append(h, 251ul, "s", 2ul);
                  }
                  osi_WrStr(h, 251ul);
                  osi_WrStrLn(" ----------- GPStime-SystemTime NO DATA SENT (\
-E ... )", 55ul);
               }
            }
            else if (sondeaprs_maxsenddistance>0UL && (double)
                sondeaprs_maxsenddistance<mygrounddist) {
               if (sondeaprs_verb) {
                  aprsstr_FixToStr((float)mygrounddist, 4UL, h, 251ul);
                  osi_WrStr(h, 251ul);
                  osi_WrStrLn("km,  ---------- NO SEND (Out of Radius -G ... \
)", 48ul);
               }
            }
            else {
               sendaprs(0UL, 0UL, sondeaprs_dao, anonym->dat[0U].time0,
                uptime, usercall, usercall_len, sondeaprs_destcall, 100ul,
                sondeaprs_via, 100ul, sondeaprs_sym, 2ul, objname,
                objname_len, anonym->dat[0U].lat, anonym->dat[0U].long0,
                anonym->dat[0U].alt,
                (double)(float)(truncc(anonym->dat[0U].dir)%360UL),
                anonym->dat[0U].speed*3.6, goodsats, hrms, s, 251ul,
                &anonym->commentline, sdr, myazi, myele, mydist);
            }
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
   aprspos_BEGIN();
   libsrtm_BEGIN();
   osi_BEGIN();
   aprsstr_BEGIN();
   contexts = 0;
   sondeaprs_udpsock = -1L;
   sondeaprs_commentfn[0UL] = 0;
   sondeaprs_csvfilename[0UL] = 0;
   strncpy(sondeaprs_destcall,"APLWS2",100u);
   sondeaprs_via[0UL] = 0;
   strncpy(sondeaprs_sym,"/O",2u);
   sondeaprs_objname[0UL] = 0;
   sondeaprs_beacontime = 30UL;
   sondeaprs_lowaltbeacontime = 0UL;
   sondeaprs_lowalt = 1000UL;
   sondeaprs_nofilter = 0;
   egmoff = 0;
/*  FILL(ADR(dat), 0C, SIZE(dat)); */
/*  lastbeacon:=0; */
/*  commentline:=0; */
/*  speedcnt:=0; */
/*  speedsum:=0.0; */
}

