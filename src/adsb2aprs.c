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
#define adsb2aprs_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef tcpb_H_
#include "tcpb.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#include <math.h>
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif

/* dump1090 tcp output to aprs beacon by OE5DXL */
#define adsb2aprs_TIMETOL 20
/* max seconds between dir/speed and pos */

#define adsb2aprs_PURGETIME 120
/* seconds keep context */

#define adsb2aprs_DEFAULTBEACONTIME 20

#define adsb2aprs_SYMBOL "/^"

#define adsb2aprs_KNOTS 1.852
/* nautic miles */

#define adsb2aprs_FEET 0.3048

#define adsb2aprs_PI 3.1415926535898

#define adsb2aprs_RAD 1.7453292519943E-2

#define adsb2aprs_LF "\012"

typedef char CSV[100][21];

struct FLY;

typedef struct FLY * pFLY;


struct FLY {
   pFLY next;
   char hex[6];
   char name[21];
   char squawk[4];
   float lat;
   float long0;
   float alt;
   float speed;
   float dir;
   float clb;
   uint32_t speedtime;
   uint32_t postime;
   uint32_t lasttime;
   uint32_t lastbeacon;
   char newpos;
};

static char url[1001];

static char port[1001];

static char reconn;

static char verb;

static char verb2;

static int32_t fd;

static pFLY dbase;

static uint32_t btime;

static char mycall[10];

static char symbol[3];

static char comment0[201];

static int32_t udpsock;

static uint32_t ipnum;

static uint32_t toport;

static float altcorr;

static struct aprsstr_POSITION homepos;

static float homealt;

static float homealtwgs;

static char titles[41][41];

static char csvfn[1024];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_WerrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static char GetNum(const char h[], uint32_t h_len, char eot,
                 uint32_t * p, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static uint32_t truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static uint32_t truncr(float r)
{
   if (r<=0.0f) return 0UL;
   else if (r>=2.147483647E+9f) return 2147483647UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncr() */


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * p,
                uint32_t * ip0, uint32_t * port0)
{
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   *p = 0UL;
   h[h_len-1] = 0;
   *ip0 = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
         ok0 = 1;
         n = (n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
         ++*p;
      }
      if (!ok0) {
         GetIp_ret = -1L;
         goto label;
      }
      if (i<3UL) {
         if (h[*p]!='.' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
         *ip0 =  *ip0*256UL+n;
      }
      else if (i==3UL) {
         *ip0 =  *ip0*256UL+n;
         if (h[*p]!=':' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port0 = n;
      ++*p;
   } /* end for */
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static void Parms(void)
{
   char s[1001];
   uint32_t n;
   uint32_t m;
   char titlesset;
   reconn = 0;
   verb = 0;
   verb2 = 0;
   strncpy(url,"127.0.0.1",1001u);
   strncpy(port,"30003",1001u);
   mycall[0] = 0;
   btime = 20UL;
   strncpy(symbol,"/^",3u);
   altcorr = 0.0f;
   comment0[0] = 0;
   homealt = (-1.E+4f);
   homepos.lat = 0.0f;
   homepos.long0 = 0.0f;
   titlesset = 0;
   libsrtm_srtmdir[0] = 0;
   memset((char *)titles,(char)0,1681UL);
   for (;;) {
      osi_NextArg(s, 1001ul);
      if (s[0U]==0) break;
      if ((s[0U]=='-' && s[1U]) && s[2U]==0) {
         if (s[1U]=='t') {
            osi_NextArg(s, 1001ul); /* url */
            n = 0UL;
            while ((n<1000UL && s[n]) && s[n]!=':') {
               if (n<1000UL) url[n] = s[n];
               ++n;
            }
            if (n>1000UL) n = 1000UL;
            url[n] = 0;
            if (s[n]==':') {
               m = 0UL;
               ++n;
               while ((n<1000UL && s[n]) && m<1000UL) {
                  port[m] = s[n];
                  ++n;
                  ++m;
               }
               if (m>1000UL) m = 1000UL;
               port[m] = 0;
            }
         }
         else if (s[1U]=='k') reconn = 1;
         else if (s[1U]=='a') {
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToFix(&altcorr, s, 1001ul)) {
               Error("-a <meter>", 11ul);
            }
         }
         else if (s[1U]=='A') {
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToFix(&homealt, s, 1001ul)) {
               Error("-A <meter>", 11ul);
            }
         }
         else if (s[1U]=='b') {
            osi_NextArg(s, 1001ul);
            n = 0UL;
            if (!GetNum(s, 1001ul, 0, &n, &btime)) Error("-b <s>", 7ul);
         }
         else if (s[1U]=='I') {
            osi_NextArg(mycall, 10ul);
            if (aprsstr_Length(mycall, 10ul)<3UL || aprsstr_Length(mycall,
                10ul)>9UL) Error("-I <callsign>", 14ul);
         }
         else if (s[1U]=='s') {
            osi_NextArg(symbol, 3ul);
            if (aprsstr_Length(symbol, 3ul)!=2UL || symbol[0U]=='-') {
               Error("-s <symbol>", 12ul);
            }
         }
         else if (s[1U]=='c') osi_NextArg(comment0, 201ul);
         else if (s[1U]=='u') {
            osi_NextArg(s, 1001ul);
            n = 0UL;
            if (GetIp(s, 1001ul, &n, &ipnum, &toport)<0L) {
               Error("-u ip:port number", 18ul);
            }
            udpsock = openudp();
            if (udpsock<0L) Error("cannot open udp socket", 23ul);
         }
         else if (s[1U]=='v') verb = 1;
         else if (s[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (s[1U]=='P') {
            osi_NextArg(s, 1001ul);
            aprsstr_loctopos(&homepos, s, 1001ul);
            if (!aprspos_posvalid(homepos)) {
               if (!aprsstr_StrToFix(&homepos.lat, s,
                1001ul) || (float)fabs(homepos.lat)>=90.0f) {
                  Error("-P <lat> <long> or <locator>", 29ul);
               }
               osi_NextArg(s, 1001ul);
               if (!aprsstr_StrToFix(&homepos.long0, s,
                1001ul) || (float)fabs(homepos.long0)>180.0f) {
                  Error("-P <lat> <long> or <locator>", 29ul);
               }
               homepos.lat = homepos.lat*1.7453292519943E-2f;
               homepos.long0 = homepos.long0*1.7453292519943E-2f;
            }
         }
         else if (s[1U]=='S') osi_NextArg(libsrtm_srtmdir, 1024ul);
         else if (s[1U]=='f') {
            osi_NextArg(s, 1001ul);
            m = 1UL;
            n = 0UL;
            titlesset = 1;
            s[1000U] = 0;
            while (m<=40UL && s[n]) {
               if (s[n]==',') ++m;
               else {
                  aprsstr_Append(titles[m], 41ul, (char *) &s[n], 1u/1u);
               }
               ++n;
            }
         }
         else if (s[1U]=='D') osi_NextArg(csvfn, 1024ul);
         else if (s[1U]=='h') {
            osi_WrStrLn("", 1ul);
            osi_WrStrLn("dump1090 basestation format tcp output to aprs objec\
t beacon", 61ul);
            osi_WrStrLn("", 1ul);
            osi_WrStrLn(" -A <meter>          my altitude for elevation else \
(if avaliable) from srtm data (egm)", 88ul);
            osi_WrStrLn(" -a <meter>          correct altitude -a 50 (0) bett\
er use WW15MGH.DAC file in -S", 82ul);
            osi_WrStrLn(" -b <seconds>        aprs minimum send intervall -b \
10 (20)", 60ul);
            osi_WrStrLn(" -c <comment>        append text to beacon and enabl\
e \"Clb=\"", 61ul);
            osi_WrStrLn(" -D <databasefile>   append text to beacon out of ai\
rcraft-database.csv (index is ICAO number)", 95ul);
            osi_WrStrLn(" -f <title>,<title>,... set titles according to csv \
database fields (index is first field ICAO)", 96ul);
            osi_WrStrLn("                        empty fields are skipped \"R\
eg,,,Model,,Serial\"", 71ul);
            osi_WrStrLn(" -h                  help", 26ul);
            osi_WrStrLn(" -I <mycall>         Sender of Object Callsign -I OE\
0AAA", 57ul);
            osi_WrStrLn(" -k                  keep tcp connection", 41ul);
            osi_WrStrLn(" -P <lat> <long> or <locator>  my Position for Dista\
nce/Azimuth/Elevation", 74ul);
            osi_WrStrLn("                       eg. -P JQ50AB12CD or -P 70.05\
06 10.0092", 63ul);
            osi_WrStrLn(" -S <pathname>       directory with SRTM(1/3/30) Dat\
a and WW15MGH.DAC file (egm96-Geoid)", 89ul);
            osi_WrStrLn("                       for Overground Calculation",
                50ul);
            osi_WrStrLn("                       example with: -S /home/pi",
                49ul);
            osi_WrStrLn("                       /home/pi/WW15MGH.DAC        (\
2076480Byte, covers whole World)", 85ul);
            osi_WrStrLn("                       /home/pi/srtm1/N48E014.hgt  (\
25934402Byte)", 66ul);
            osi_WrStrLn("                       /home/pi/srtm1/N48E015.hgt",
                50ul);
            osi_WrStrLn(" -s <symbol>         aprs symbol (/^)", 38ul);
            osi_WrStrLn(" -t <url:port>       connect dump1090 tcp server (12\
7.0.0.1:30003) \"dump1090 --net\"", 84ul);
            osi_WrStrLn(" -u <ip>:<port>      send AXUDP -u 127.0.0.1:9001 us\
e udpgate4 or aprsmap as receiver", 86ul);
            osi_WrStrLn(" -V                  very verbous", 34ul);
            osi_WrStrLn(" -v                  verbous", 29ul);
            osi_WrStrLn("example: -t 127.0.0.1:30003 -I YOURCALL-11 -u 127.0.\
0.1:9002 -k -v -c 1090MHz", 78ul);
            osi_WrStrLn("before this start \"dump1090 --net\"", 35ul);
            osi_WrStrLn("", 1ul);
            X2C_ABORT();
         }
         else {
            osi_Werr(">", 2ul);
            osi_Werr(s, 1001ul);
            Error("< ? use -h", 11ul);
         }
      }
      else Error("-h", 3ul);
   }
   if (!titlesset) {
      strncpy(titles[1U],"Reg",41u);
      strncpy(titles[2U],"ManICAO",41u);
      strncpy(titles[3U],"ManName",41u);
      strncpy(titles[4U],"Model",41u);
      strncpy(titles[5U],"Type",41u);
      strncpy(titles[6U],"Ser",41u);
      strncpy(titles[7U],"Linenum",41u);
      /*  titles[8 ]:="icaotype"; */
      strncpy(titles[9U],"op",41u);
      strncpy(titles[10U],"opcall",41u);
      /*  titles[11]:="opicao"; */
      /*  titles[12]:="opiata"; */
      strncpy(titles[13U],"owner",41u);
      strncpy(titles[15U],"reg",41u);
      strncpy(titles[16U],"till",41u);
      strncpy(titles[18U],"built",41u);
      strncpy(titles[19U],"fitst",41u);
   }
} /* end Parms() */


static void decodeline(const char line0[], uint32_t line_len,
                CSV csv0)
{
   uint32_t j;
   uint32_t w;
   uint32_t i;
   memset((char *)csv0,(char)0,2100UL);
   i = 0UL;
   j = 0UL;
   w = 0UL;
   while (i<=line_len-1 && (uint8_t)line0[i]>=' ') {
      if (line0[i]!=',') {
         if (w<=99UL && j<=20UL) {
            csv0[w][j] = line0[i];
            ++j;
         }
      }
      else {
         ++w;
         j = 0UL;
      }
      ++i;
   }
} /* end decodeline() */


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


static void sendaprs(char dao, uint32_t time0, char mycall0[],
                uint32_t mycall_len, char destcall[],
                uint32_t destcall_len, char via[], uint32_t via_len,
                char sym[], uint32_t sym_len, char obj[],
                uint32_t obj_len, double lat, double long0,
                double alt, double course, double speed,
                float clb, char comm[], uint32_t comm_len)
{
   char ds[501];
   char h[501];
   char b[501];
   char raw[361];
   int32_t rp;
   uint32_t n;
   uint32_t i;
   float a;
   X2C_PCOPY((void **)&mycall0,mycall_len);
   X2C_PCOPY((void **)&destcall,destcall_len);
   X2C_PCOPY((void **)&via,via_len);
   X2C_PCOPY((void **)&sym,sym_len);
   X2C_PCOPY((void **)&obj,obj_len);
   X2C_PCOPY((void **)&comm,comm_len);
   b[0] = 0;
   aprsstr_Append(b, 501ul, mycall0, mycall_len);
   aprsstr_Append(b, 501ul, ">", 2ul);
   aprsstr_Append(b, 501ul, destcall, destcall_len);
   if (via[0UL]) {
      aprsstr_Append(b, 501ul, ",", 2ul);
      aprsstr_Append(b, 501ul, via, via_len);
   }
   aprsstr_Append(b, 501ul, ":;", 3ul);
   aprsstr_Assign(h, 501ul, obj, obj_len);
   aprsstr_Append(h, 501ul, "         ", 10ul);
   h[9U] = 0;
   aprsstr_Append(b, 501ul, h, 501ul);
   aprsstr_Append(b, 501ul, "*", 2ul);
   aprsstr_DateToStr(time0, ds, 501ul);
   ds[0U] = ds[11U];
   ds[1U] = ds[12U];
   ds[2U] = ds[14U];
   ds[3U] = ds[15U];
   ds[4U] = ds[17U];
   ds[5U] = ds[18U];
   ds[6U] = 0;
   aprsstr_Append(b, 501ul, ds, 501ul);
   aprsstr_Append(b, 501ul, "h", 2ul);
   i = aprsstr_Length(b, 501ul);
   a = (float)fabs(lat);
   n = truncr(a);
   b[i] = num(n/10UL);
   ++i;
   b[i] = num(n);
   ++i;
   n = truncr((a-(float)n)*6000.0f);
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
   a = (float)fabs(long0);
   n = truncr(a);
   b[i] = num(n/100UL);
   ++i;
   b[i] = num(n/10UL);
   ++i;
   b[i] = num(n);
   ++i;
   n = truncr((a-(float)n)*6000.0f);
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
      n = truncr((float)(course+1.5));
      b[i] = num(n/100UL);
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      b[i] = '/';
      ++i;
      n = truncr((float)(speed*5.3995680345572E-1+0.5));
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
      n = truncr((float)fabs(alt*3.2808398950131+0.5));
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
   /*  IF clb<>0.0 THEN */
   b[i] = 'C';
   ++i;
   b[i] = 'l';
   ++i;
   b[i] = 'b';
   ++i;
   b[i] = '=';
   ++i;
   b[i] = 0;
   aprsstr_FixToStr(clb*4.7625E-3f, 2UL, h, 501ul);
                /* looks like feet/s * 64 */
   aprsstr_Append(b, 501ul, h, 501ul);
   /*  END; */
   if (comm[0UL]) {
      aprsstr_Append(b, 501ul, " ", 2ul);
      aprsstr_Append(b, 501ul, comm, comm_len);
   }
   if (verb) osi_WrStrLn(b, 501ul);
   b[254U] = 0; /* limit len for aprs */
   aprsstr_mon2raw(b, 501ul, raw, 361ul, &rp);
   if (rp==0L) {
      osi_WerrLn("axudp encode error (possibly call not encodable", 48ul);
   }
   rp = udpsend(udpsock, raw, rp, toport, ipnum);
   X2C_PFREE(mycall0);
   X2C_PFREE(destcall);
   X2C_PFREE(via);
   X2C_PFREE(sym);
   X2C_PFREE(obj);
   X2C_PFREE(comm);
} /* end sendaprs() */

#define adsb2aprs_SEP "\""

struct DB;

typedef struct DB * pDB;


struct DB {
   pDB next;
   uint8_t icaomsb;
   char str[65556];
};

static pDB dbidx[65536];


static uint32_t ghex(const char s[], uint32_t s_len)
{
   uint32_t n;
   uint32_t i;
   char c;
   n = 0UL;
   i = 0UL;
   for (;;) {
      c = X2C_CAP(s[i]);
      if ((uint8_t)c>='0' && (uint8_t)c<='9') {
         n = (n*16UL+(uint32_t)(uint8_t)c)-48UL;
      }
      else if ((uint8_t)c>='A' && (uint8_t)c<='F') {
         n = (n*16UL+(uint32_t)(uint8_t)c)-55UL;
      }
      else break;
      ++i;
      if (i>s_len-1) break;
   }
   return n;
} /* end ghex() */


static char getch(char b[4096], int32_t fd0, int32_t * len,
                int32_t * p)
{
   if (*p>=*len) {
      *len = osi_RdBin(fd0, (char *)b, 4096u/1u, 4096UL);
      if (*len<=0L) return 0;
      *p = 0L;
   }
   ++*p;
   return b[*p-1L];
} /* end getch() */


static int32_t getword(int32_t * p, int32_t * len, int32_t fd0,
                char b[4096], char s[], uint32_t s_len)
{
   uint32_t i;
   char inqu;
   i = 0UL;
   inqu = 0;
   for (;;) {
      s[i] = getch(b, fd0, len, p);
      if (s[i]==0) return -1L;
      if (s[i]=='\012') {
         s[i] = 0;
         return 0L;
      }
      if (!inqu && s[i]==',') {
         s[i] = 0;
         return 1L;
      }
      if (s[i]=='\"') inqu = !inqu;
      else if ((i<s_len-1 && (uint8_t)s[i]>=' ') && (uint8_t)
                s[i]<(uint8_t)'\200') ++i;
   }
   return 0;
} /* end getword() */


static void rdcsv(char fn[], uint32_t fn_len)
{
   int32_t r;
   int32_t len;
   int32_t p;
   uint32_t cnt;
   uint32_t line0;
   uint32_t i;
   int32_t fd0;
   /*    pm:pDB; */
   char b[4096];
   char h[1024];
   char s[1024];
   char f[50][100];
   pDB pd;
   uint32_t dupes;
   uint32_t msb;
   uint32_t wc;
   uint32_t icao;
   X2C_PCOPY((void **)&fn,fn_len);
   memset((char *)dbidx,(char)0,sizeof(pDB [65536]));
   cnt = 0UL;
   fd0 = osi_OpenRead(fn, fn_len);
   if (fd0<0L) Error("database file not readable", 27ul);
   if (verb) osi_WerrLn("importing aircraft database", 28ul);
   line0 = 0UL;
   dupes = 0UL;
   p = 0L;
   len = 0L;
   for (;;) {
      wc = 0UL;
      for (;;) {
         r = getword(&p, &len, fd0, b, f[wc], 100ul);
         if (r<=0L) break;
         if (wc<49UL) ++wc;
      }
      if (r>=0L) {
         icao = ghex(f[0U], 100ul);
         msb = icao/65536UL;
         icao = icao&65535UL;
         pd = dbidx[icao];
         while (pd && (uint32_t)pd->icaomsb!=msb) pd = pd->next;
         if (pd==0) {
            h[0] = 0;
            i = 1UL;
            while (i<wc) {
               if (i<=40UL) {
                  aprsstr_Append(h, 1024ul, "\"", 2ul);
                  aprsstr_Append(h, 1024ul, f[i], 100ul);
               }
               ++i;
            }
            osic_alloc((char * *) &pd,
                (sizeof(struct DB)-65535UL)+aprsstr_Length(h, 1024ul));
            if (pd==0) {
               aprsstr_IntToStr((int32_t)line0, 1UL, s, 1024ul);
               osi_Werr("in line:", 9ul);
               osi_Werr(s, 1024ul);
               Error(" out of memory", 15ul);
            }
            pd->icaomsb = (uint8_t)msb;
            aprsstr_Assign(pd->str, 65556ul, h, 1024ul);
            pd->next = dbidx[icao];
            dbidx[icao] = pd;
            ++line0;
         }
         else ++dupes;
      }
      else {
         if (verb) {
            aprsstr_IntToStr((int32_t)line0, 1UL, s, 1024ul);
            osi_Werr(s, 1024ul);
            osi_WerrLn(" Lines imported", 16ul);
            if (dupes) {
               aprsstr_IntToStr((int32_t)dupes, 1UL, s, 1024ul);
               osi_Werr(s, 1024ul);
               osi_WerrLn(" duplicates removed", 20ul);
            }
         }
         break;
      }
   }
   osic_Close(fd0);
   X2C_PFREE(fn);
} /* end rdcsv() */


static void appenddb(const char icao[], uint32_t icao_len,
                char s[], uint32_t s_len)
{
   uint32_t i;
   uint32_t col;
   uint32_t ic;
   uint8_t icm;
   pDB p;
   ic = ghex(icao, icao_len);
   p = dbidx[ic&65535UL];
   icm = (uint8_t)(ic/65536UL);
   while (p && p->icaomsb!=icm) p = p->next;
   if (p) {
      /*WrStr("<<<");WrStr(p^.str); WrStrLn(">>>"); */
      col = 0UL;
      i = 0UL;
      while (p->str[i] && col<=40UL) {
         if (p->str[i]=='\"') {
            ++col;
            if ((p->str[i+1UL]!='\"' && (uint8_t)p->str[i+1UL]>' ')
                && titles[col][0U]) {
               aprsstr_Append(s, s_len, " ", 2ul);
               aprsstr_Append(s, s_len, titles[col], 41ul);
               aprsstr_Append(s, s_len, ":", 2ul);
            }
         }
         else if (titles[col][0U]) {
            aprsstr_Append(s, s_len, (char *) &p->str[i], 1u/1u);
         }
         ++i;
      }
   }
} /* end appenddb() */


static void elevation(double * el, double * c,
                struct aprsstr_POSITION home, double homealt0,
                struct aprsstr_POSITION dist, double distalt)
{
   float z1;
   float y1;
   float x1;
   float z0;
   float y00;
   float x0;
   double sb;
   double r;
   double s;
   double b;
   double a;
   *el = (-1000.0);
   aprspos_wgs84s(home.lat, home.long0, (float)(homealt0*0.001), &x0,
                &y00, &z0);
   aprspos_wgs84s(dist.lat, dist.long0, (float)(distalt*0.001), &x1, &y1,
                &z1);
   a = sqrt((double)(x0*x0+y00*y00+z0*z0));
   b = sqrt((double)(x1*x1+y1*y1+z1*z1));
   x1 = x1-x0;
   y1 = y1-y00;
   z1 = z1-z0;
   *c = sqrt((double)(x1*x1+y1*y1+z1*z1));
   /* halbwinkelsatz */
   s = (a+b+*c)*0.5;
   if (s==0.0) return;
   r = X2C_DIVL((s-a)*(s-b)*(s-*c),s);
   if (r<=0.0) return;
   r = sqrt(r);
   sb = s-b;
   if (sb!=0.0) *el = 1.1459155902616E+2*atan(X2C_DIVL(r,sb))-90.0;
   else *el = 90.0;
} /* end elevation() */


static float getoverground(float lat, float long0, float alt)
{
   struct aprsstr_POSITION pos;
   float resolution;
   float srtm;
   libsrtm_srtmmaxmem = 1000000UL;
   pos.lat = lat;
   pos.long0 = long0;
   srtm = libsrtm_getsrtm(pos, 1UL, &resolution);
   if (srtm<10000.0f && srtm>(-1000.0f)) {
      if (alt<=(-3.E+4f)) return srtm;
      /* srtm request */
      return alt-srtm;
   }
   return (-1.E+5f);
} /* end getoverground() */


static float egmcorr(const struct aprsstr_POSITION pos,
                char invers, float a)
/* correct altitude wgs84/egm */
{
   float e;
   char ok0;
   if (libsrtm_srtmdir[0U]) {
      ok0 = 1;
      e = libsrtm_egm96(pos, &ok0);
      if (ok0) {
         if (invers) e = -e;
         a = a-e;
      }
   }
   return a;
} /* end egmcorr() */


static void aprs(const struct FLY f)
{
   char h[31];
   char obj[31];
   char ct[501];
   double dist;
   double ele;
   float azi;
   float altegm;
   float altwgs;
   float og;
   struct aprsstr_POSITION pos;
   aprsstr_Assign(obj, 31ul, f.name, 21ul);
   obj[9U] = 0;
   while (aprsstr_Length(obj, 31ul)<9UL) aprsstr_Append(obj, 31ul, " ", 2ul);
   strncpy(ct,"ICAO:",501u);
   aprsstr_Append(ct, 501ul, f.hex, 6ul);
   aprsstr_Append(ct, 501ul, " ", 2ul);
   altwgs = f.alt;
   pos.lat = f.lat*1.7453292519943E-2f;
   pos.long0 = f.long0*1.7453292519943E-2f;
   if (altcorr!=0.0f) altegm = altwgs+altcorr;
   else if (libsrtm_srtmdir[0U]) altegm = egmcorr(pos, 0, altwgs);
   if (aprspos_posvalid(homepos)) {
      azi = aprspos_azimuth(homepos, pos);
      aprsstr_FixToStr(azi, 0UL, h, 31ul);
      aprsstr_Append(ct, 501ul, "az=", 4ul);
      aprsstr_Append(ct, 501ul, h, 31ul);
      if (homealtwgs>(-1000.0f)) {
         /* elevation */
         ele = (-100.0);
         elevation(&ele, &dist, homepos, (double)homealtwgs, pos,
                (double)altwgs);
         aprsstr_FixToStr((float)dist, 4UL, h, 31ul);
         aprsstr_Append(ct, 501ul, " d=", 4ul);
         aprsstr_Append(ct, 501ul, h, 31ul);
         aprsstr_Append(ct, 501ul, "km", 3ul);
         if (fabs(ele)<=90.0) {
            aprsstr_FixToStr((float)ele, 3UL, h, 31ul);
            aprsstr_Append(ct, 501ul, " el=", 5ul);
            aprsstr_Append(ct, 501ul, h, 31ul);
         }
         if (libsrtm_srtmdir[0U]) {
            og = getoverground(pos.lat, pos.long0, altegm);
            if (og>(-1000.0f)) {
               aprsstr_FixToStr(og, 0UL, h, 31ul);
               aprsstr_Append(ct, 501ul, " og=", 5ul);
               aprsstr_Append(ct, 501ul, h, 31ul);
               aprsstr_Append(ct, 501ul, "m", 2ul);
            }
         }
      }
   }
   if (f.squawk[0U]) {
      aprsstr_Append(ct, 501ul, " Sq=", 5ul);
      aprsstr_Append(ct, 501ul, f.squawk, 4ul);
      if (aprsstr_StrCmp(f.squawk, 4ul, "7700", 5ul)) {
         aprsstr_Append(ct, 501ul, "[EMERGENCY]", 12ul);
      }
      else if (aprsstr_StrCmp(f.squawk, 4ul, "7600", 5ul)) {
         aprsstr_Append(ct, 501ul, "[RADIO FAILURE]", 16ul);
      }
      else if (aprsstr_StrCmp(f.squawk, 4ul, "7500", 5ul)) {
         aprsstr_Append(ct, 501ul, "[HIJACKING]", 12ul);
      }
   }
   appenddb(f.hex, 6ul, ct, 501ul);
   if (comment0[0U]) {
      aprsstr_Append(ct, 501ul, " ", 2ul);
      aprsstr_Append(ct, 501ul, comment0, 201ul);
   }
   sendaprs(1, f.postime, mycall, 10ul, "APLFR1", 7ul, "", 1ul, symbol, 3ul,
                obj, 31ul, (double)f.lat, (double)f.long0,
                (double)altegm, (double)f.dir,
                (double)(f.speed*1.852f), f.clb, ct, 501ul);
} /* end aprs() */


static void store(const CSV csv0)
{
   pFLY f0;
   pFLY f1;
   pFLY f;
   uint32_t msg;
   uint32_t t;
   float oalt;
   float olong;
   float olat;
   t = osic_time();
   if ((((csv0[0U][0U]=='M' && csv0[0U][1U]=='S') && csv0[0U][2U]=='G')
                && aprsstr_StrToCard(csv0[1U], 21ul,
                &msg)) && ((((msg==1UL || msg==2UL) || msg==3UL) || msg==4UL)
                 || msg==6UL)) {
      f = dbase;
      f0 = 0;
      while (f && !aprsstr_StrCmp(f->hex, 6ul, csv0[4U], 21ul)) {
         f1 = f->next;
         if (f->lasttime+120UL<t) {
            if (f0==0) dbase = f1;
            else f0->next = f1;
            if (verb2) {
               osi_WrStr("purge ", 7ul);
               osi_WrStrLn(f->hex, 6ul);
            }
            osic_free((char * *) &f, sizeof(struct FLY));
         }
         else f0 = f;
         f = f1;
      }
      if (f==0) {
         osic_alloc((char * *) &f, sizeof(struct FLY));
         if (f==0) {
            osi_WerrLn("Out of Memory", 14ul);
            return;
         }
         memset((char *)f,(char)0,sizeof(struct FLY));
         f->next = dbase;
         dbase = f;
         aprsstr_Assign(f->hex, 6ul, csv0[4U], 21ul);
         if (verb2) {
            osi_WrStr("new ", 5ul);
            osi_WrStrLn(f->hex, 6ul);
         }
      }
      f->lasttime = t;
      if (msg==1UL) {
         if (verb2 && f->name[0U]==0) {
            osi_WrStr("found name ", 12ul);
            osi_WrStr(f->hex, 6ul);
            osi_WrStr(" ", 2ul);
            osi_WrStrLn(f->name, 21ul);
         }
         aprsstr_Assign(f->name, 21ul, csv0[10U], 21ul);
      }
      else if (msg==4UL) {
         if ((((aprsstr_StrToFix(&f->speed, csv0[12U],
                21ul) && aprsstr_StrToFix(&f->dir, csv0[13U],
                21ul)) && aprsstr_StrToFix(&f->clb, csv0[16U],
                21ul)) && f->dir>=0.0f) && f->dir<=360.0f) f->speedtime = t;
      }
      else if (msg==3UL || msg==2UL) {
         oalt = 0.0f;
         if ((((((((msg==2UL || aprsstr_StrToFix(&oalt, csv0[11U],
                21ul)) && aprsstr_StrToFix(&olat, csv0[14U],
                21ul)) && olat>(-90.0f)) && olat<90.0f)
                && aprsstr_StrToFix(&olong, csv0[15U],
                21ul)) && olong>(-180.0f)) && olong<180.0f)
                && (olong!=f->long0 || olat!=f->lat)) {
            f->postime = t;
            f->newpos = 1;
            f->lat = olat;
            f->long0 = olong;
            if (oalt!=0.0f) f->alt = oalt*0.3048f+altcorr;
            else f->alt = 0.0f;
            if (msg==2UL) f->speedtime = t;
         }
      }
      else if (msg==6UL) aprsstr_Assign(f->squawk, 4ul, csv0[17U], 21ul);
      /*IF verb & (csv[17][0]<>0C) THEN WrStr("Squawk:"); WrStrLn(csv[17]);
                END; (* 7700 luftnotfall up/down, 7600 funkausfall, 7500 hijacking *) */
      if (f->lastbeacon>t) f->lastbeacon = t;
      if (((((((f->newpos && f->name[0U]) && f->postime+20UL>=t)
                && f->speedtime+20UL>=t) && f->speed>=0.0f) && f->lat!=0.0f)
                && f->long0!=0.0f) && f->lastbeacon+btime<t) {
         aprs(*f);
         f->newpos = 0;
         f->lastbeacon = t;
      }
   }
} /* end store() */

static char ibuf[201];

static char line[201];

static uint32_t ip;

static uint32_t lp;

static CSV csv;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(CSV)!=2100) X2C_ASSERT(0);
   libsrtm_BEGIN();
   aprspos_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   Parms();
   if (csvfn[0U]) rdcsv(csvfn, 1024ul);
   homealtwgs = homealt;
   if ((aprspos_posvalid(homepos) && libsrtm_srtmdir[0U])
                && homealt<=(-1000.0f)) {
      homealt = -getoverground(homepos.lat, homepos.long0, 0.0f);
      if (verb) osi_WrStrLn("get home altitude from srtm", 28ul);
      homealtwgs = egmcorr(homepos, 1, homealt); /* altitude to wgs84 */
   }
   if (verb && homealt>(-1000.0f)) {
      osi_WrStr("home altitude (egm):", 21ul);
      osic_WrFixed(homealt, 1L, 1UL);
      osi_WrStrLn("m", 2ul);
      if (homealt!=homealtwgs) {
         osi_WrStr("home altitude (wgs84):", 23ul);
         osic_WrFixed(homealtwgs, 1L, 1UL);
         osi_WrStrLn("m", 2ul);
      }
   }
   fd = -1L;
   dbase = 0;
   fd = connecttob(url, port);
   lp = 0UL;
   for (;;) {
      if (fd>=0L) {
         if (readsockb(fd, (char *)ibuf, 201L)<0L) {
            /* connect lost */
            osic_Close(fd);
            fd = -1L;
         }
         else {
            for (ip = 0UL; ip<=200UL; ip++) {
               if ((uint8_t)ibuf[ip]<' ') {
                  if (lp<200UL) line[lp] = 0;
                  if (aprsstr_Length(line, 201ul)>2UL) {
                     if (verb2) osi_WrStrLn(line, 201ul);
                     decodeline(line, 201ul, csv);
                     store(csv);
                     lp = 0UL;
                  }
               }
               else if (lp<200UL) {
                  line[lp] = ibuf[ip];
                  ++lp;
               }
            } /* end for */
         }
      }
      else {
         osi_WerrLn("connection lost", 16ul);
         if (reconn) {
            usleep(1000000UL);
            fd = connecttob(url, port);
         }
         else break;
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
