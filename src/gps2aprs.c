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
#define gps2aprs_C_
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif
#include <signal.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* read serial gps and make axudp aprs frames */
/*
FROM mlib IMPORT termios, tcgetattr, tcsetattr,
                 CS8, CLOCAL, CREAD, TCSAFLUSH,
                 B1200, B2400, B4800, B9600, B19200, B38400,
                 B57600, B115200, B230400, B460800;
*/
struct SAT;


struct SAT {
   uint32_t id;
   uint32_t az;
   uint32_t el;
   uint32_t db;
};

struct POS;


struct POS {
   uint32_t daytime;
   uint32_t year;
   uint32_t month;
   uint32_t day;
   uint32_t date;
   uint32_t altok;
   uint32_t sats;
   uint32_t fix;
   double lat;
   double long0;
   double speed;
   double course;
   double alt;
   double climb;
   double vx;
   double vy;
   double vz;
   char posok;
   char speedok;
   char dateok;
   int32_t leapseconds;
   uint8_t timeflags;
   uint32_t satcnt;
   uint32_t lastsat;
   struct SAT satpos[12];
};

#define gps2aprs_DEFTTY "/dev/ttyS0"

#define gps2aprs_CR "\015"

#define gps2aprs_LF "\012"

#define gps2aprs_KNOTS 1.851984

#define gps2aprs_FEET 3.2808398950131

#define gps2aprs_DEFAULTPORT 9002

#define gps2aprs_DEFAULTIP 0x7F000001 
/* 127.0.0.1 */

#define gps2aprs_BTIMENAVI 2
/* fast beacon to aprsmap as navi -N */

#define gps2aprs_ALTLIFE 10
/* seconds altitude valid */

static uint8_t gps2aprs_SETNMEA[22] = {6U,0U,1U,0U,0U,0U,208U,8U,0U,0U,0U,0U,0U,0U,3U,0U,2U,0U,0U,0U,0U,0U};

static uint8_t gps2aprs_INITBAUD[22] = {6U,0U,1U,0U,0U,0U,208U,8U,0U,0U,0U,0U,0U,0U,1U,0U,1U,0U,0U,0U,0U,0U};
/*                                 port      com mode    baud     in   outproto */

struct CONF;

typedef struct CONF * pCONF;


struct CONF {
   pCONF next;
   char str[251];
};

/*CRCL, CRCH: ARRAY[0..255] OF SET8;*/
static char tbuf[1024];

static char ttynamee[1024];

static char logfilename[1024];

static char mycall[100];

static char via[100];

static char viaakt[100];

static char comment0[100];

static char symt;

static char symb;

static uint32_t toport;

static uint32_t toport2;

static uint32_t baud;

static uint32_t ipnum;

static uint32_t ipnum2;

static int32_t timecorr;

static int32_t timepuls;

static int32_t udpsock;

static char verb;

static char verb2;

static char anyip;

static char usbrobust;

static char junk;

static char sumoff;

static char withalti;

static char withspeed;

static char withdao;

static char useaxudp;

static char balloon;

static char ubloxraw;

static uint32_t comptyp;

static uint32_t micessid;

static uint32_t btime0;

static uint32_t btimedrive;

static uint32_t drivekm;

static uint32_t comintval;

static uint32_t comcnt;

static uint32_t btime;

static uint32_t msgtyp;

static uint32_t btimeN;

static uint32_t btimenavi;

static uint32_t frames;

static uint32_t dynamicmodel;

static uint32_t fixalt;

static uint32_t fixmode;

static uint32_t sumerrcnt;

static uint32_t rawframeerr;

static uint32_t duration;

static char timesetdone;

static char terminatetimeset;

static struct POS pos;

static uint32_t gpsp;

static char gpsb[251];

static char Logl[251];

static pCONF pconfig;

static int32_t tty;
/*  saved: termios; */

/*
<* IF TARGET_FAMILY="WIN32" THEN *>
<* ELSE *>
PROCEDURE ["C"] / stime(VAR time:CARDINAL):INTEGER;   (* set system time, needs root *)
<* END *>
*/

static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr("aprstracker: ", 14ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static uint32_t truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.147483647E+9) return 2147483647UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */

#define gps2aprs_DEFAULTIP0 0x7F000001 

#define gps2aprs_PORTSEP ":"


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * p, uint32_t * ip, uint32_t * port)
{
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   *p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      if (i>=3UL || h[0UL]!=':') {
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
      }
      if (i<3UL) {
         if (h[0UL]!=':') {
            if (h[*p]!='.' || n>255UL) {
               GetIp_ret = -1L;
               goto label;
            }
            *ip =  *ip*256UL+n;
         }
      }
      else if (i==3UL) {
         if (h[0UL]!=':') {
            *ip =  *ip*256UL+n;
            if (h[*p]!=':' || n>255UL) {
               GetIp_ret = -1L;
               goto label;
            }
         }
         else {
            *p = 0UL;
            *ip = 2130706433UL;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++*p;
   } /* end for */
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */

/*  term : termios; */
/*  bd   : INTEGER; */

static void SetComMode(int32_t fd, uint32_t baud0)
{
   if (osi_setttybaudraw(fd, baud0)<0L) {
      if (verb) osi_WrStrLn("cannot config tty", 18ul);
   }
/*
  IF baud=1200 THEN bd:=B1200;
  ELSIF baud=2400 THEN bd:=B2400;
  ELSIF baud=4800 THEN bd:=B4800;
  ELSIF baud=9600 THEN bd:=B9600;
  ELSIF baud=19200 THEN bd:=B19200;
  ELSIF baud=38400 THEN bd:=B38400;
  ELSIF baud=57600 THEN bd:=B57600;
  ELSIF baud=115200 THEN bd:=B115200;
  ELSIF baud=230400 THEN bd:=B230400;
  ELSIF baud=460800 THEN bd:=B460800;
  ELSE Error("unknown baudrate") END;

  res:=tcgetattr(fd, saved);
  res:=tcgetattr (fd, term);
  WITH term DO
    c_lflag :=0;
    c_oflag :=0;
    c_iflag :=0;
(*  cfmakeraw(&termios);*)
    c_cflag :=CS8+CLOCAL+CREAD(*+CRTSCTS*)+bd;  (*0800018B2H*)
  END;
  res:=tcsetattr (fd, TCSAFLUSH, term);
*/
} /* end SetComMode() */


static void opentty(void)
{
   for (;;) {
      tty = osi_OpenRW(ttynamee, 1024ul);
      if (tty>=0L) {
         SetComMode(tty, baud);
         break;
      }
      if (!usbrobust) Error("tty open", 9ul);
      if (verb) osi_WrStrLn("tty open error", 15ul);
      usleep(1000000UL);
   }
} /* end opentty() */


static void testtty(int32_t len0, char * err)
{
   if (len0<=0L) {
      osic_Close(tty);
      usleep(1000000UL);
      opentty();
      *err = 1;
   }
} /* end testtty() */


static char GetNum(const char h[], uint32_t h_len, char eot, uint32_t * p, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static char GetInt(const char h[], uint32_t h_len, char eot, uint32_t * p, int32_t * n)
{
   char s;
   *n = 0L;
   s = 0;
   if (h[*p]=='-') {
      s = 1;
      ++*p;
   }
   while ((uint8_t)h[*p]>='0' && (uint8_t)h[*p]<='9') {
      *n =  *n*10L+(int32_t)((uint32_t)(uint8_t)h[*p]-48UL);
      ++*p;
   }
   if (s) *n = -*n;
   return h[*p]==eot;
} /* end GetInt() */


static void Parms(void)
{
   char err;
   char h[1024];
   uint32_t i;
   pCONF pch;
   pCONF pc;
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='m' || h[1U]=='r') {
            useaxudp = h[1U]=='r';
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (GetIp(h, 1024ul, &i, &ipnum, &toport)<0L) {
               Error("-m or -r ip:port number", 24ul);
            }
         }
         else if (h[1U]=='N') {
            useaxudp = 1;
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (GetIp(h, 1024ul, &i, &ipnum2, &toport2)<0L) Error("-N ip:port", 11ul);
         }
         else if (h[1U]=='a') withalti = 0;
         else if (h[1U]=='u') usbrobust = 0;
         else if (h[1U]=='k') withspeed = 0;
         else if (h[1U]=='s') sumoff = 1;
         else if (h[1U]=='D') withdao = 1;
         else if (h[1U]=='B') balloon = 1;
         else if (h[1U]=='U') ubloxraw = 1;
         else if (X2C_CAP(h[1U])=='Z') {
            timesetdone = 0;
            if (h[1U]=='Z') terminatetimeset = 1;
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &timecorr)) Error("-Z <seconds>", 13ul);
         }
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 100ul);
            if ((uint8_t)mycall[0U]<'0') Error("-I <mycall>", 12ul);
         }
         else if (h[1U]=='w') {
            osi_NextArg(via, 100ul);
            if ((uint8_t)via[0U]<=' ') Error("-m vias like RELAY,WIDE1-1", 27ul);
         }
         else if (h[1U]=='c') {
            osi_NextArg(comment0, 100ul);
            if (aprsstr_Length(comment0, 100ul)>60UL) {
               Error("-c <comment>  (max 60 byte)", 28ul);
            }
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (h[0U]=='-') Error("-C <configstring>", 18ul);
            osic_alloc((char * *) &pc, sizeof(struct CONF));
            if (pc==0) Error("-C out of memory>", 18ul);
            pc->next = 0;
            aprsstr_Assign(pc->str, 251ul, h, 1024ul);
            if (pconfig) {
               pch = pconfig;
               while (pch->next) pch = pch->next;
               pch->next = pc;
            }
            else pconfig = pc;
         }
         else if (h[1U]=='i') {
            osi_NextArg(h, 1024ul);
            if ((uint8_t)h[0U]>' ' && (uint8_t)h[1U]>' ') {
               symt = h[0U];
               symb = h[1U];
            }
            else Error("-i <icon> (house /-)", 21ul);
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            while ((h[i] && h[i]!=':') && i<1023UL) {
               ttynamee[i] = h[i];
               ++i;
            }
            ttynamee[i] = 0;
            if (h[i]) {
               ++i;
               if (!GetNum(h, 1024ul, 0, &i, &baud)) Error("need ttydevice:baud", 20ul);
            }
         }
         else if (h[1U]=='L') osi_NextArg(logfilename, 1024ul);
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &btimedrive)) {
               Error("-b <s>", 7ul);
            }
         }
         else if (h[1U]=='n') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &btimenavi)) Error("-n <s>", 7ul);
         }
         else if (h[1U]=='0') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &btime0)) Error("-0 <s>", 7ul);
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &drivekm)) Error("-g <km/h>", 10ul);
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &fixalt)) Error("-A <cm> <fixmode>", 18ul);
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &fixmode)) Error("-A <cm> <fixmode>", 18ul);
         }
         else if (h[1U]=='T') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetInt(h, 1024ul, 0, &i, &timepuls)) Error("-T <periode> <duration>", 24ul);
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &duration)) Error("-T <periode> <duration>", 24ul);
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &dynamicmodel)) Error("-M <num>", 9ul);
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &comintval)) Error("-l <n>", 7ul);
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &comptyp) || comptyp>2UL) {
               Error("-f <format> 0=uncomp, 1=comp, 2=mic-e", 38ul);
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &micessid) || micessid>7UL) {
               Error("-d <ssid> 0..7", 15ul);
            }
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn("Read serial GPS and make normal/compressed/mic-e Beacon as AXUDP or monitor", 76ul);
               osi_WrStrLn(" -0 <s>                            standing Beacon Time in Seconds (180)", 73ul);
               osi_WrStrLn(" -A <cm> <fixmode>                 in Raw-Mode set fixed 2d altitude in cm NN", 78ul);
               osi_WrStrLn("                                     <fixmode> 1 2D, 2 3D, 4 auto-2D/3D", 72ul);
               osi_WrStrLn(" -a                                altitude OFF", 48ul);
               osi_WrStrLn(" -B                                Balloon mode: -g <m> is altitude(m)", 71ul);
               osi_WrStrLn("                                     -b <s> for below -0 <s> for above", 71ul);
               osi_WrStrLn("                                     -w <path> is switched on below", 68ul);
               osi_WrStrLn(" -b <s>                            driving Beacon Time in Seconds (15)", 71ul);
               osi_WrStrLn(" -c <commentstring>                APRS Comment (max 60 char)", 62ul);
               osi_WrStrLn("                                     insert time hhmmss  : \\\\h", 63ul);
               osi_WrStrLn("                                     insert time ddhhmm  : \\\\z", 63ul);
               osi_WrStrLn("                                     insert file         : \\\\:filename:", 72ul);
               osi_WrStrLn("                                     insert Clb=<m/s>    : \\\\c", 63ul);
               osi_WrStrLn("                                     insert time hh-mm-ss: \\\\t", 63ul);
               osi_WrStrLn("                                     insert sats         : \\\\s", 63ul);
               osi_WrStrLn("                                     insert fix          : \\\\f", 63ul);
               osi_WrStrLn("                                     insert framecounter : \\\\n", 63ul);
               osi_WrStrLn("                                     insert \\\\           : \\\\\\", 63ul);
               osi_WrStrLn("                                     double all \\ to pass thru bash eg. \\\\\\\\h", 78ul);
               osi_WrStrLn(" -C <configstring>                 Send Cfg to GPS, / for $, // for /", 70ul);
               osi_WrStrLn("                                     /PUBX,41,1,0007,0003,19200,0", 66ul);
               osi_WrStrLn(" -d <x>                            Destination Call SSID 0..7", 62ul);
               osi_WrStrLn(" -D                                DAO Extension on for 20cm Resolution", 72ul);
               osi_WrStrLn(" -f <x>                            format 0=normal 1=compressed 2=mic-e (0)", 76ul);
               osi_WrStrLn(" -g <km/h>                         min. Speed for driving Beacon Time (4)", 74ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I <mycall>                       Mycall with SSID like NOCALL-15", 67ul);
               osi_WrStrLn(" -i <icon>                         2 Icon chars \"/-\" (House), \"/>\" (Car)...(//)", 80ul);
               osi_WrStrLn(" -k                                Speed/Course OFF (not in mic-e)", 67ul);
               osi_WrStrLn(" -l <n>                            every n Beacons send one with Comment (5)", 77ul);
               osi_WrStrLn(" -L <filename>                     Append raw GPS text to this File", 68ul);
               osi_WrStrLn(" -M <n>                            in Raw-Mode set dynamic model (balloon 1g)", 78ul);
               osi_WrStrLn("                                     0 portabel, 2 stationary, 3 pedestrian, 4 car,", 84ul);
               osi_WrStrLn("                                     5 sea, 6 air 1g, 7 air 2g, air 4g", 71ul);
               osi_WrStrLn(" -m <x.x.x.x:destport>             use Monitor UDP format :port for localhost", 78ul);
               osi_WrStrLn(" -N <x.x.x.x:destport>             send Position AXUDP every 2s eg. to aprsmap", 79ul);
               osi_WrStrLn(" -n <s>                            Beacon Time in Seconds to -N destination (2)", 80ul);
               osi_WrStrLn(" -r <x.x.x.x:destport>             send AXUDP (to kiss-TNC or TCPKISS via udpflex,", 83ul);
               osi_WrStrLn("                                     to afskmodem or via aprsmap or udpgate to TCP)", 84ul);
               osi_WrStrLn(" -s                                GPS Checksum check OFF", 58ul);
               osi_WrStrLn(" -T <us> <us>                      Raw-Mode, Timepulse <[-]periode> <duration> (us)", 84ul);
               osi_WrStrLn(" -t <tty>:<baud>                   (/dev/ttyS0:4800)", 53ul);
               osi_WrStrLn(" -U                                switch Ublox>=M6 sV>=7 to Ublox-Raw (get vertical speed)", 92ul);
               osi_WrStrLn("                                     needs bidirectional connection", 68ul);
               osi_WrStrLn(" -u                                abort, not retry until open removable USB tty", 81ul);
               osi_WrStrLn(" -V                                verbous, show sats (GSV-frames)", 67ul);
               osi_WrStrLn(" -v                                verbous", 43ul);
               osi_WrStrLn(" -w <viapath>                      via Path like RELAY,WIDE1-1", 63ul);
               osi_WrStrLn(" -Z <s>                            set system time to GPStime+<s> (will need root)", 83ul);
               osi_WrStrLn(" -z <s>                            same but do not terminate after time set", 76ul);
               osic_WrLn();
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else {
         /*
               h[0]:=0C;
         */
         err = 1;
      }
      if (err) break;
   }
   if (comptyp==1UL && withdao) Error("DAO Option not with compressed Format", 38ul);
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */


static uint32_t gpstimetosystime(const struct POS p)
{
   char s[21];
   uint32_t sec;
   strncpy(s,"20  :  :  ",21u);
   s[2U] = (char)(p.year/10UL+48UL);
   s[3U] = (char)(p.year%10UL+48UL);
   s[5U] = (char)(p.month/10UL+48UL);
   s[6U] = (char)(p.month%10UL+48UL);
   s[8U] = (char)(p.day/10UL+48UL);
   s[9U] = (char)(p.day%10UL+48UL);
   if (!aprsstr_StrToTime(s, 21ul, &sec)) return 0UL;
   sec += p.daytime;
   aprsstr_DateToStr(sec, s, 21ul);
   return sec;
} /* end gpstimetosystime() */


static void setstime(const struct POS p)
{
   char s[21];
   uint32_t sec;
   sec = p.date;
   if (sec>0UL) {
      sec += (uint32_t)timecorr;
      aprsstr_DateToStr(sec, s, 21ul);
      if (verb) {
         osi_WrStr("time to sys:", 13ul);
         osi_WrStrLn(s, 21ul);
      }
      if (osi_settime(&sec)>=0L) osi_WrStrLn("system time set", 16ul);
      else osi_WrStrLn("--- error setting system time (will need root)", 47ul);
      if (terminatetimeset) X2C_ABORT();
      timesetdone = 1;
   }
} /* end setstime() */


static void sendudp(char buf[], uint32_t buf_len, int32_t len0, uint32_t ip, uint32_t port)
{
   int32_t i;
   /*  crc:CARDINAL;  */
   X2C_PCOPY((void **)&buf,buf_len);
   i = udpsend(udpsock, buf, len0, port, ip);
   X2C_PFREE(buf);
/*FOR i:=0 TO upos-2 DO IO.WrHex(ORD(buf[i]), 3) END; IO.WrLn; */
} /* end sendudp() */


static void skip(const char b[], uint32_t b_len, uint32_t * p, uint32_t len0)
{
   while (*p<len0 && b[*p]!=',') ++*p;
   if (*p<len0) ++*p;
} /* end skip() */


static char getnum(const char b[], uint32_t b_len, uint32_t * p, uint32_t len0, uint32_t * n)
{
   /*WrStr(b[p]); */
   if ((*p<len0 && (uint8_t)b[*p]>='0') && (uint8_t)b[*p]<='9') {
      *n = (uint32_t)(uint8_t)b[*p]-48UL;
      ++*p;
      return 1;
   }
   *n = 0UL;
   return 0;
} /* end getnum() */

#define gps2aprs_MSYM "\\"

#define gps2aprs_FILESYM ":"


static void beaconmacros(char s[], uint32_t s_len)
{
   uint32_t tt;
   uint32_t ic;
   uint32_t n;
   uint32_t i;
   int32_t j;
   int32_t len0;
   char ns[256];
   char ds[256];
   char fn[1024];
   int32_t f;
   i = 0UL;
   ns[0U] = 0;
   while (i<s_len-1 && s[i]) {
      if (s[i]=='\\' && s[i+1UL]=='\\') {
         i += 2UL;
         if (s[i]=='z') {
            /* insert day, hour, min */
            tt = pos.date;
            if (!pos.dateok) tt = osic_time();
            aprsstr_DateToStr(tt, ds, 256ul);
            aprsstr_DateToStr(osic_time(), ds, 256ul);
            ds[0U] = ds[8U];
            ds[1U] = ds[9U];
            ds[2U] = ds[11U];
            ds[3U] = ds[12U];
            ds[4U] = ds[14U];
            ds[5U] = ds[15U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='h') {
            /* insert hour, min, s */
            tt = pos.date;
            if (!pos.dateok) tt = osic_time();
            aprsstr_DateToStr(tt, ds, 256ul);
            ds[0U] = ds[11U];
            ds[1U] = ds[12U];
            ds[2U] = ds[14U];
            ds[3U] = ds[15U];
            ds[4U] = ds[17U];
            ds[5U] = ds[18U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]==':') {
            /* insert or write file */
            fn[0U] = 0;
            ++i;
            while ((i<s_len-1 && s[i]) && s[i]!=':') {
               aprsstr_Append(fn, 1024ul, (char *) &s[i], 1u/1u);
               ++i;
            }
            f = osi_OpenRead(fn, 1024ul);
            if (osic_FdValid(f)) {
               len0 = osi_RdBin(f, (char *)ds, 256u/1u, 255UL);
               osic_Close(f);
               j = 0L;
               while (((j<len0 && ds[j]!='\015') && ds[j]!='\012') && ds[j]) {
                  aprsstr_Append(ns, 256ul, (char *) &ds[j], 1u/1u);
                  ++j;
               }
            }
            else {
               aprsstr_Assign(s, s_len, "beacon macro file not readable ", 32ul);
               aprsstr_Append(s, s_len, fn, 1024ul);
               if (verb) osi_WrStrLn(s, s_len);
            }
         }
         else if (s[i]=='\\') {
            /*          s[0]:=0C; */
            /*          RETURN */
            aprsstr_Append(ns, 256ul, "\\\\", 3ul);
         }
         else if (s[i]=='s') {
            aprsstr_IntToStr((int32_t)pos.sats, 1UL, ds, 256ul);
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='f') {
            aprsstr_IntToStr((int32_t)pos.fix, 1UL, ds, 256ul);
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='t') {
            aprsstr_TimeToStr(pos.date%86400UL, ds, 256ul);
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='c') {
            strncpy(ds,"Clb=",256u);
            ic = 4UL;
            if (pos.climb<0.0) {
               ds[4U] = '-';
               ++ic;
            }
            n = (uint32_t)X2C_TRUNCC(fabs(pos.climb)*10.0+0.5,0UL,X2C_max_longcard);
            if (n>9999UL) n = 9999UL;
            if (n>=1000UL) {
               ds[ic] = (char)(n/1000UL+48UL);
               ++ic;
            }
            n = n%1000UL;
            if (n>=100UL) {
               ds[ic] = (char)(n/100UL+48UL);
               ++ic;
            }
            n = n%100UL;
            ds[ic] = (char)(n/10UL+48UL);
            ++ic;
            ds[ic] = '.';
            ++ic;
            ds[ic] = (char)(n%10UL+48UL);
            ++ic;
            ds[ic] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='n') {
            aprsstr_IntToStr((int32_t)frames, 1UL, ds, 256ul);
            aprsstr_Append(ns, 256ul, ds, 256ul);
            frames = (frames+1UL)%100000UL;
         }
         else {
            aprsstr_Assign(s, s_len, "bad beacon macro ", 18ul);
            aprsstr_Append(s, s_len, ns, 256ul);
            aprsstr_Append(s, s_len, "???", 4ul);
            if (verb) osi_WrStrLn(s, s_len);
            s[0UL] = 0;
            return;
         }
      }
      else aprsstr_Append(ns, 256ul, (char *) &s[i], 1u/1u);
      ++i;
   }
   aprsstr_Assign(s, s_len, ns, 256ul);
} /* end beaconmacros() */


static void decodeline(const char b[], uint32_t b_len, uint32_t len0, struct POS * p)
{
   uint32_t msg;
   uint32_t msgs;
   uint32_t j;
   uint32_t n;
   uint32_t i;
   double div0;
   char sign;
   struct SAT * anonym;
   if (b[0UL]=='$') {
      if (b[1UL]=='G') {
         if ((b[3UL]=='R' && b[4UL]=='M') && b[5UL]=='C') {
            i = 7UL;
            if (getnum(b, b_len, &i, len0, &n)) p->daytime = n*36000UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->daytime += n*3600UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->daytime += n*600UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->daytime += n*60UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->daytime += n*10UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->daytime += n;
            else return;
            skip(b, b_len, &i, len0);
            if (b[i]!='A') return;
            skip(b, b_len, &i, len0);
            if (getnum(b, b_len, &i, len0, &n)) p->lat = (double)(float)(n*10UL);
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->lat = p->lat+(double)(float)n;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,6.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,60.0f));
            }
            else return;
            if (b[i]=='.') ++i;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,600.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,6000.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,60000.0f));
            }
            if (getnum(b, b_len, &i, len0, &n)) {
               p->lat = p->lat+(double)(X2C_DIVR((float)n,6.E+5f));
            }
            skip(b, b_len, &i, len0);
            if (b[i]=='S') p->lat = -p->lat;
            else if (b[i]!='N') return;
            skip(b, b_len, &i, len0);
            if (getnum(b, b_len, &i, len0, &n)) p->long0 = (double)(float)(n*100UL);
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(float)(n*10UL);
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->long0 = p->long0+(double)(float)n;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,6.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,60.0f));
            }
            else return;
            if (b[i]=='.') ++i;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,600.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,6000.0f));
            }
            else return;
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,60000.0f));
            }
            if (getnum(b, b_len, &i, len0, &n)) {
               p->long0 = p->long0+(double)(X2C_DIVR((float)n,6.E+5f));
            }
            skip(b, b_len, &i, len0);
            if (b[i]=='W') p->long0 = -p->long0;
            else if (b[i]!='E') return;
            skip(b, b_len, &i, len0);
            p->speed = 0.0;
            while (getnum(b, b_len, &i, len0, &n)) {
               p->speed = p->speed*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len0, &n)) {
                  p->speed = p->speed+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            p->speed = p->speed*1.851984; /* knots to km/h */
            skip(b, b_len, &i, len0);
            p->course = 0.0;
            while (getnum(b, b_len, &i, len0, &n)) {
               p->course = p->course*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len0, &n)) {
                  p->course = p->course+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            p->posok = 1;
            skip(b, b_len, &i, len0);
            if (getnum(b, b_len, &i, len0, &n)) p->day = n*10UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->day += n;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->month = n*10UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->month += n;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->year = n*10UL;
            else return;
            if (getnum(b, b_len, &i, len0, &n)) p->year += n;
            else return;
            p->date = gpstimetosystime(*p);
            p->dateok = p->date>0UL;
         }
         else if ((b[3UL]=='G' && b[4UL]=='G') && b[5UL]=='A') {
            /* $GPGGA,152554,3938.5665,N,10346.2039,W,1,08,1.7,12382.7,M,-22.3,M,,*7B */
            /* $GPGGA,112435.00,4812.41112,N,01305.61998,E,1,08,1.04,398.3,M,44.9,M,,*59 */
            i = 7UL;
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            if (getnum(b, b_len, &i, len0, &n)) p->fix = n;
            else return;
            skip(b, b_len, &i, len0);
            p->sats = 0UL;
            while (getnum(b, b_len, &i, len0, &n)) p->sats = p->sats*10UL+n;
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            p->alt = 0.0;
            if (b[i]=='-') {
               sign = 1;
               ++i;
            }
            else sign = 0;
            while (getnum(b, b_len, &i, len0, &n)) {
               p->altok = 10UL;
               p->alt = p->alt*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len0, &n)) {
                  p->alt = p->alt+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            if (sign) p->alt = -p->alt;
         }
         else if ((b[3UL]=='G' && b[4UL]=='S') && b[5UL]=='V') {
            /* sat num/az/el/dB */
            /*
            $GPGSV,3,1,11,22,78,048,19,01,75,130,32,03,74,289,,21,53,136,40*76
            $GPGSV,3,2,11,17,41,291,16,04,31,195,37,19,24,315,,31,21,089,35*76
            $GPGSV,3,3,11,32,13,043,,08,01,176,,09,,210,*4E
            */
            i = 7UL;
            if (!getnum(b, b_len, &i, len0, &msgs)) return;
            skip(b, b_len, &i, len0);
            if ((!getnum(b, b_len, &i, len0, &msg) || msg==0UL) || msg>3UL) return;
            skip(b, b_len, &i, len0);
            if (!getnum(b, b_len, &i, len0, &n)) return;
            if (!getnum(b, b_len, &i, len0, &p->satcnt)) return;
            p->satcnt += n*10UL;
            skip(b, b_len, &i, len0);
            msg = (msg-1UL)*4UL;
            j = 0UL;
            for (;;) {
               if (msg+j>=p->satcnt) break;
               { /* with */
                  struct SAT * anonym = &p->satpos[msg+j];
                  getnum(b, b_len, &i, len0, &n);
                  if (getnum(b, b_len, &i, len0, &anonym->id)) anonym->id += n*10UL;
                  skip(b, b_len, &i, len0);
                  getnum(b, b_len, &i, len0, &n);
                  if (getnum(b, b_len, &i, len0, &anonym->el)) anonym->el += n*10UL;
                  skip(b, b_len, &i, len0);
                  getnum(b, b_len, &i, len0, &n);
                  if (getnum(b, b_len, &i, len0, &anonym->az)) anonym->az += n*10UL;
                  if (getnum(b, b_len, &i, len0, &n)) {
                     anonym->az = anonym->az*10UL+n;
                  }
                  skip(b, b_len, &i, len0);
                  getnum(b, b_len, &i, len0, &n);
                  if (getnum(b, b_len, &i, len0, &anonym->db)) anonym->db += n*10UL;
                  skip(b, b_len, &i, len0);
               }
               ++j;
               p->lastsat = msg+j;
               if (j>3UL) break;
            }
         }
      }
      else if (((b[1UL]=='P' && b[2UL]=='U') && b[3UL]=='B') && b[4UL]=='X') {
         /* ublox propietary for versical speed */
         /* $PUBX,00,230327.00,7825.08527,N,01202.28188,E,425.646,G3,3.4,7.8,0.050,331.58,-0.026,,1.11,1.95,1.33,7,0,0*49 */
         /*                                                                               -climb */
         i = 5UL;
         skip(b, b_len, &i, len0);
         if (b[i]=='0' && b[i+1UL]=='0') {
            /* frametype 00 */
            i += 2UL;
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            skip(b, b_len, &i, len0);
            p->climb = 0.0;
            if (b[i]=='-') {
               sign = 1;
               ++i;
            }
            else sign = 0;
            while (getnum(b, b_len, &i, len0, &n)) {
               p->climb = p->climb*10.0+(double)(float)n;
            }
            if (b[i]=='.') {
               ++i;
               div0 = 0.1;
               while (getnum(b, b_len, &i, len0, &n)) {
                  p->climb = p->climb+(double)(float)n*div0;
                  div0 = div0*0.1;
               }
            }
            if (!sign) p->climb = -p->climb;
         }
      }
   }
} /* end decodeline() */


static char Hex(uint32_t d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


static char checksum(const char b[], uint32_t b_len, uint32_t len0)
{
   uint32_t i;
   uint8_t cs;
   char ok0;
   ok0 = 1;
   i = 1UL;
   cs = 0U;
   while (i<len0 && b[i]!='*') {
      cs = cs^(uint8_t)(uint8_t)b[i];
      ++i;
   }
   if (i+2UL>=len0) ok0 = 0;
   if (ok0) {
      if (b[i+1UL]!=Hex((uint32_t)cs/16UL) || b[i+2UL]!=Hex((uint32_t)cs)) ok0 = 0;
   }
   if (verb && !ok0) osi_WrStrLn("GPS Checksum Error", 19ul);
   return ok0;
} /* end checksum() */


static void showline(const char b[], uint32_t b_len, uint32_t len0)
{
   uint32_t i;
   i = 0UL;
   while (i<len0) {
      osi_WrStr((char *) &b[i], 1u/1u);
      ++i;
   }
   osic_WrLn();
} /* end showline() */

#define gps2aprs_DB 4.342944819


static void showsats(uint32_t cnt, struct SAT sats[], uint32_t sats_len)
{
   uint32_t minn;
   uint32_t min0;
   uint32_t dh;
   uint32_t p;
   uint32_t medc;
   uint32_t med;
   uint32_t i;
   float pow0;
   char hh[201];
   char h[201];
   struct SAT * anonym;
   struct SAT * anonym0;
   uint32_t tmp;
   med = 0UL;
   medc = 0UL;
   pow0 = 0.0f;
   min0 = X2C_max_longcard;
   if (cnt>0UL && cnt<=(sats_len-1)+1UL) {
      tmp = cnt-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct SAT * anonym = &sats[i];
            if ((anonym->id>0UL && anonym->db>0UL) && anonym->db<100UL) {
               med += anonym->db;
               ++medc;
               pow0 = pow0+osic_exp((float)anonym->db*2.3025850930113E-1f);
               if (anonym->az<min0) min0 = anonym->az;
            }
         }
         if (i==tmp) break;
      } /* end for */
      strncpy(h,"id azi el dB Sats:",201u);
      aprsstr_IntToStr((int32_t)cnt, 1UL, hh, 201ul);
      aprsstr_Append(h, 201ul, hh, 201ul);
      aprsstr_Append(h, 201ul, " median:", 9ul);
      aprsstr_FixToStr(X2C_DIVR((float)med,(float)medc), 2UL, hh, 201ul);
      aprsstr_Append(h, 201ul, hh, 201ul);
      aprsstr_Append(h, 201ul, "dB", 3ul);
      if (pow0>0.1f) {
         aprsstr_Append(h, 201ul, " sum:", 6ul);
         aprsstr_FixToStr(osic_ln(pow0)*4.342944819f, 2UL, hh, 201ul);
         aprsstr_Append(h, 201ul, hh, 201ul);
         aprsstr_Append(h, 201ul, "dB", 3ul);
      }
      osi_WrStrLn(h, 201ul);
      do {
         /* sort up by atimuth */
         minn = X2C_max_longcard;
         tmp = cnt-1UL;
         i = 0UL;
         if (i<=tmp) for (;; i++) {
            { /* with */
               struct SAT * anonym0 = &sats[i];
               if (anonym0->id>0UL) {
                  if (anonym0->az==min0) {
                     aprsstr_IntToStr((int32_t)anonym0->id, 2UL, h, 201ul);
                     aprsstr_IntToStr((int32_t)anonym0->az, 4UL, hh, 201ul);
                     aprsstr_Append(h, 201ul, hh, 201ul);
                     aprsstr_IntToStr((int32_t)anonym0->el, 3UL, hh, 201ul);
                     aprsstr_Append(h, 201ul, hh, 201ul);
                     aprsstr_IntToStr((int32_t)anonym0->db, 3UL, hh, 201ul);
                     aprsstr_Append(h, 201ul, hh, 201ul);
                     p = aprsstr_Length(h, 201ul);
                     aprsstr_Append(h, 201ul, " **************************************************", 52ul);
                     dh = anonym0->db/2UL;
                     if (dh>49UL) dh = 49UL;
                     h[p+dh+1UL] = 0;
                     if ((anonym0->db&1)) aprsstr_Append(h, 201ul, "-", 2ul);
                     osi_WrStrLn(h, 201ul);
                  }
                  else if (anonym0->az>min0 && anonym0->az<minn) minn = anonym0->az;
               }
            }
            if (i==tmp) break;
         } /* end for */
         min0 = minn;
      } while (minn!=X2C_max_longcard);
   }
} /* end showsats() */


static void uploadcfg(char s[], uint32_t s_len)
{
   uint32_t i;
   uint8_t cs;
   char b[4001];
   char tmp;
   X2C_PCOPY((void **)&s,s_len);
   i = 0UL;
   b[0] = 0;
   while (i<s_len-1 && s[i]) {
      if (s[i]=='/') {
         ++i;
         if (s[i]!='/') aprsstr_Append(b, 4001ul, "$", 2ul);
      }
      aprsstr_Append(b, 4001ul, (char *) &s[i], 1u/1u);
      ++i;
   }
   i = 1UL;
   cs = 0U;
   while (i<4000UL && b[i]) {
      cs = cs^(uint8_t)(uint8_t)b[i];
      ++i;
   }
   aprsstr_Append(b, 4001ul, "*", 2ul);
   aprsstr_Append(b, 4001ul, (char *)(tmp = Hex((uint32_t)cs/16UL),&tmp), 1u/1u);
   aprsstr_Append(b, 4001ul, (char *)(tmp = Hex((uint32_t)cs),&tmp), 1u/1u);
   aprsstr_Append(b, 4001ul, "\015", 2ul);
   aprsstr_Append(b, 4001ul, "\012", 2ul);
   osi_WrBin(tty, (char *)b, 4001u/1u, aprsstr_Length(b, 4001ul));
   if (verb) {
      osi_WrStr("uploaded:", 10ul);
      osi_WrStrLn(b, 4001ul);
   }
   X2C_PFREE(s);
} /* end uploadcfg() */


static char num(uint32_t n)
{
   return (char)(n%10UL+48UL);
} /* end num() */


static uint32_t dao91(double x)
/* radix91(xx/1.1) of dddmm.mmxx */
{
   double a;
   a = fabs(x);
   return ((truncc((a-(double)truncc(a))*6.E+5)%100UL)*20UL+11UL)/22UL;
} /* end dao91() */


static void sendaprs(double lat, double long0, double alt, double course, double speed, uint32_t comp0, char withspd, char withalt, char dao, char com, char comm[],
                uint32_t comm_len, const char viapath[], uint32_t viapath_len, char local, uint32_t ip, uint32_t port)
{
   char b[251];
   char raw[361];
   int32_t rp0;
   uint32_t micdest;
   uint32_t nl;
   uint32_t n;
   uint32_t i;
   double a;
   char tmp;
   X2C_PCOPY((void **)&comm,comm_len);
   /* OE0AAA-9>APERXQ,RELAY,WIDE2-2:!4805.44N/01333.64E>325/016/A=001824 */
   b[0] = 0;
   aprsstr_Append(b, 251ul, mycall, 100ul);
   micdest = aprsstr_Length(b, 251ul)+1UL;
   if (local) aprsstr_Append(b, 251ul, ">NOGATE", 8ul);
   else {
      aprsstr_Append(b, 251ul, ">APLT01", 8ul);
      if (micessid>0UL) {
         aprsstr_Append(b, 251ul, "-", 2ul);
         aprsstr_Append(b, 251ul, (char *)(tmp = (char)(micessid+48UL),&tmp), 1u/1u);
      }
      if (viapath[0UL]) {
         aprsstr_Append(b, 251ul, ",", 2ul);
         aprsstr_Append(b, 251ul, viapath, viapath_len);
      }
   }
   if (comp0==0UL) {
      /* uncompressed */
      aprsstr_Append(b, 251ul, ":!", 3ul);
      i = aprsstr_Length(b, 251ul);
      a = fabs(lat);
      n = truncc(a);
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncc((a-(double)n)*6000.0);
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
      b[i] = symt;
      ++i;
      a = fabs(long0);
      n = truncc(a);
      b[i] = num(n/100UL);
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncc((a-(double)n)*6000.0);
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
      b[i] = symb;
      ++i;
      if (withspd) {
         n = truncc(course+1.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
         b[i] = '/';
         ++i;
         n = truncc(speed*5.3996146834962E-1+0.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
      }
      if (withalt) {
         b[i] = '/';
         ++i;
         b[i] = 'A';
         ++i;
         b[i] = '=';
         ++i;
         n = truncc(fabs(alt*3.2808398950131+0.5));
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
      aprsstr_Append(b, 251ul, ":!", 3ul);
      i = aprsstr_Length(b, 251ul);
      b[i] = symt;
      ++i;
      if (lat<90.0) n = truncc((90.0-lat)*3.80926E+5);
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
         n = truncc((180.0+long0)*1.90463E+5);
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
      b[i] = symb;
      ++i;
      if (withspd) {
         b[i] = (char)(33UL+truncc(course)/4UL);
         ++i;
         b[i] = (char)(33UL+truncc((double)(osic_ln((float)(speed*5.3996146834962E-1+1.0))*1.29935872129E+1f)));
         ++i;
         b[i] = '_';
         ++i;
      }
      else if (withalt) {
         if (alt*3.2808398950131>1.0) {
            n = truncc((double)(osic_ln((float)(alt*3.2808398950131))*500.5f));
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
      if (withspeed && withalt) {
         b[i] = '/';
         ++i;
         b[i] = 'A';
         ++i;
         b[i] = '=';
         ++i;
         n = truncc(alt*3.2808398950131+0.5);
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
      aprsstr_Append(b, 251ul, ":`", 3ul);
      i = micdest;
      nl = truncc(fabs(long0));
      n = truncc(fabs(lat));
      b[i] = (char)(80UL+n/10UL);
      ++i;
      b[i] = (char)(80UL+n%10UL);
      ++i;
      n = truncc((fabs(lat)-(double)n)*6000.0);
      b[i] = (char)(80UL+n/1000UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(lat>=0.0)+(n/100UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(nl<10UL || nl>=100UL)+(n/10UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(long0<0.0)+n%10UL);
      i = aprsstr_Length(b, 251ul);
      if (nl<10UL) b[i] = (char)(nl+118UL);
      else if (nl>=100UL) {
         if (nl<110UL) b[i] = (char)(nl+8UL);
         else b[i] = (char)(nl-72UL);
      }
      else b[i] = (char)(nl+28UL);
      ++i;
      nl = truncc((fabs(long0)-(double)nl)*6000.0); /* long min*100 */
      n = nl/100UL;
      if (n<10UL) n += 60UL;
      b[i] = (char)(n+28UL);
      ++i;
      b[i] = (char)(nl%100UL+28UL);
      ++i;
      n = truncc(speed*5.3996146834962E-1+0.5);
      b[i] = (char)(n/10UL+28UL);
      ++i;
      nl = truncc(course);
      b[i] = (char)(32UL+(n%10UL)*10UL+nl/100UL);
      ++i;
      b[i] = (char)(28UL+nl%100UL);
      ++i;
      b[i] = symb;
      ++i;
      b[i] = symt;
      ++i;
      if (withalt) {
         if (alt>(-1.E+4)) n = truncc(alt+10000.5);
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
   if (com) {
      beaconmacros(comm, comm_len);
      aprsstr_Append(b, 251ul, comm, comm_len);
   }
   /*  Append(b, CR+LF); */
   if (aprsstr_Length(mycall, 100ul)>=3UL) {
      if (useaxudp) {
         aprsstr_mon2raw(b, 251ul, raw, 361ul, &rp0);
         if (rp0>0L) sendudp(raw, 361ul, rp0, ip, port);
      }
      else sendudp(b, 251ul, (int32_t)(aprsstr_Length(b, 251ul)+1UL), ip, port);
   }
   if (verb) {
      osic_WrFixed((float)lat, 6L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)long0, 6L, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed((float)speed, 2L, 1UL);
      osi_WrStr("km/h ", 6ul);
      osic_WrFixed((float)course, 1L, 1UL);
      osi_WrStr("deg ", 5ul);
      osic_WrFixed((float)alt, 2L, 1UL);
      osi_WrStrLn("m", 2ul);
      osi_WrStrLn(b, 251ul);
   }
   X2C_PFREE(comm);
} /* end sendaprs() */


static void newpos(void)
{
   uint32_t balloonkm;
   if (pos.posok) {
      memcpy(viaakt,via,100u);
      ++btime;
      if (balloon) {
         if (pos.alt>0.0) balloonkm = truncc(pos.alt);
         else balloonkm = 0UL;
      }
      else balloonkm = truncc(pos.speed);
      if (balloonkm>=drivekm!=balloon) {
         if (btime>=btimedrive) btime = 0UL;
      }
      else {
         /* no drive / high altitide */
         if (balloon) viaakt[0U] = 0;
         if (btime>btime0) btime = 0UL;
      }
      if (btime==0UL) {
         sendaprs(pos.lat, pos.long0, pos.alt, pos.course, pos.speed, comptyp, withspeed, withalti && pos.altok>0UL, withdao, comcnt==0UL, comment0, 100ul, viaakt, 100ul, 0, ipnum, toport);
         ++comcnt;
         if (comcnt>=comintval) comcnt = 0UL;
      }
      if (toport2>0UL) {
         /* second (fast) beacon to map */
         ++btimeN;
         if (btimeN>=btimenavi) {
            sendaprs(pos.lat, pos.long0, pos.alt, pos.course, pos.speed, 0UL, 1, pos.altok>0UL, 1, 0, "", 1ul, "", 1ul, 1, ipnum2, toport2);
            btimeN = 0UL;
         }
      }
      if (pos.altok>0UL) --pos.altok;
      pos.posok = 0;
   }
   if (pos.dateok && !timesetdone) setstime(pos);
} /* end newpos() */


static void WrLog(const char b[], uint32_t b_len, uint32_t len0)
{
   int32_t logfd;
   if (logfilename[0U]) {
      logfd = osi_OpenAppend(logfilename, 1024ul);
      if (!osic_FdValid(logfd)) logfd = osi_OpenWrite(logfilename, 1024ul);
      if (osic_FdValid(logfd)) {
         osi_WrBin(logfd, (char *)b, (b_len)/1u, len0);
         osi_WrBin(logfd, (char *)"\012", 2u/1u, 1UL);
         osic_Close(logfd);
      }
      else {
         osi_WrStr("cannot write [", 15ul);
         osi_WrStr(logfilename, 1024ul);
         osi_WrStrLn("]", 2ul);
      }
   }
} /* end WrLog() */

/*-------- ublox raw */

static double atang2(double x, double y)
{
   float w;
   if (fabs(x)>fabs(y)) {
      w = osic_arctan((float)(X2C_DIVL(y,x)));
      if (x<0.0) {
         if (y>0.0) w = 3.1415926535898f+w;
         else w = w-3.1415926535898f;
      }
   }
   else if (y!=0.0) {
      w = 1.5707963267949f-osic_arctan((float)(X2C_DIVL(x,y)));
      if (y<0.0) w = w-3.1415926535898f;
   }
   else w = 0.0f;
   return (double)w;
} /* end atang2() */


static void initubloxraw(const uint8_t cmd[], uint32_t cmd_len)
{
   uint8_t sum2;
   uint8_t sum1;
   uint8_t b[51];
   uint32_t j;
   uint32_t i;
   uint32_t tmp;
   b[0U] = 181U;
   b[1U] = 98U;
   j = (cmd_len-1)+1UL;
   b[2U] = cmd[0UL];
   b[3U] = cmd[1UL];
   b[4U] = (uint8_t)(j-2UL);
   b[5U] = (uint8_t)((j-2UL)/256UL);
   i = 2UL;
   while (i<j && i+2UL<=50UL) {
      b[i+4UL] = cmd[i];
      ++i;
   }
   sum1 = b[2U]+b[3U];
   sum2 = b[2U]*2U+b[3U];
   i = 4UL;
   while (i<=j+3UL) {
      sum1 += b[i];
      sum2 += sum1;
      ++i;
   }
   b[i] = sum1;
   ++i;
   b[i] = sum2;
   osi_WrBin(tty, (char *)b, 51u/1u, i+1UL);
   if (verb) {
      osi_WrStr("t[", 3ul);
      tmp = i;
      j = 0UL;
      if (j<=tmp) for (;; j++) {
         osi_WrHex((uint32_t)b[j], 3UL);
         if (j==tmp) break;
      } /* end for */
      osi_WrStrLn("]", 2ul);
   }
} /* end initubloxraw() */


static void FrameOn(uint8_t class0, uint8_t subclass)
{
   uint8_t b[10];
   b[0U] = 6U;
   b[1U] = 1U;
   b[2U] = class0;
   b[3U] = subclass;
   b[4U] = 0U; /* port 0 */
   b[5U] = 1U; /* port 1 */
   b[6U] = 0U;
   b[7U] = 0U;
   b[8U] = 0U;
   b[9U] = 0U;
   initubloxraw(b, 10ul);
} /* end FrameOn() */


static void SetDynamicModel(uint8_t n)
{
   uint8_t b[38];
   memset((char *)b,(char)0,38UL);
   b[0U] = 6U;
   b[1U] = 36U;
   b[2U] = 1U; /* mask */
   b[4U] = n; /* 0 portabel, 2 stationary, 3 pedestrian, 4 automotive, 5 sea, 6 air 1g, 7 air 2g, air 4g */
   initubloxraw(b, 38ul);
   if (verb) {
      osi_WrStr("set dynamic model to ", 22ul);
      switch ((unsigned)n) {
      case 0U:
         osi_WrStr("portabel", 9ul);
         break;
      case 2U:
         osi_WrStr("stationary", 11ul);
         break;
      case 3U:
         osi_WrStr("pedestrian", 11ul);
         break;
      case 4U:
         osi_WrStr("automotive", 11ul);
         break;
      case 5U:
         osi_WrStr("sea", 4ul);
         break;
      case 6U:
         osi_WrStr("air 1g", 7ul);
         break;
      case 7U:
         osi_WrStr("air 2g", 7ul);
         break;
      case 8U:
         osi_WrStr("air 4g", 7ul);
         break;
      default:;
         osi_WrStr("unknown", 8ul);
         break;
      } /* end switch */
      osi_WrStrLn("", 1ul);
   }
} /* end SetDynamicModel() */


static void SetFixAlt(uint32_t m)
/* fixed altitude cm */
{
   uint8_t b[38];
   memset((char *)b,(char)0,38UL);
   b[0U] = 6U;
   b[1U] = 36U;
   b[2U] = 4U; /* mask */
   b[5U] = (uint8_t)fixmode;
   b[6U] = (uint8_t)m;
   b[7U] = (uint8_t)(m/256UL);
   b[8U] = (uint8_t)(m/65536UL);
   b[9U] = (uint8_t)(m/16777216UL);
   initubloxraw(b, 38ul);
   if (verb) {
      osi_WrStr("set altitude to ", 17ul);
      osic_WrFixed((float)m*0.01f, 2L, 1UL);
      osi_WrStrLn("m", 2ul);
   }
} /* end SetFixAlt() */


static void TimePuls(int32_t us, uint32_t dur)
{
   uint8_t b[22];
   uint32_t t;
   t = (uint32_t)labs(us);
   memset((char *)b,(char)0,22UL);
   b[0U] = 6U;
   b[1U] = 7U;
   b[2U] = (uint8_t)t;
   b[3U] = (uint8_t)(t/256UL);
   b[4U] = (uint8_t)(t/65536UL);
   b[5U] = (uint8_t)(t/16777216UL);
   b[6U] = (uint8_t)dur;
   b[7U] = (uint8_t)(dur/256UL);
   b[8U] = (uint8_t)(dur/65536UL);
   b[9U] = (uint8_t)(dur/16777216UL);
   b[10U] = 1U; /* polarity */
   if (us<0L) b[10U] = 255U;
   initubloxraw(b, 22ul);
   if (verb) {
      if (t>0UL && dur<t) {
         osi_WrStr("timepulse set to ", 18ul);
         osic_WrFixed(X2C_DIVR(1.E+6f,(float)t), 2L, 1UL);
         osi_WrStrLn("Hz", 3ul);
      }
      else osi_WrStr("timepuls periode<duration ?", 28ul);
   }
} /* end TimePuls() */

/* energy management */
/*
r[B5 62 06 3B 2C 00  vers: 01 06 00 00  flags: 00 90 02 00 upd: E8 03 00 00
 aqu:  10 27 00 00 offgrid: 00 00 00 00 ontime: 02 00 minaq: 00 00  2C 01 00 00 4F C1 03 00 86 02 00 00 FE 00 00 00 64 40 01 00 95 C5 ]
t[B5 62 06 3B 2C 00        01 00 00 00         00 11 03 00      88 13 00 00
       10 27 00 00          00 00 05 00         00 00        00 00  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 59 06 ]
r[B5 62 05 01 02 00 06 3B 49 72 ]

B5 62 01 06 34 00 C0 1C 43 08 AE 6B FB FF 4D 08 00 4C 86 69 ED 1E 38 D8 28 15 DE EC 67 06 C0 DF B6 26 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 0F 27 02 00 22 67 01 00 D3 B5

*/
/* get emepheris */
/*  b[2]:=1; */

static void showh(char b[], uint32_t b_len, int32_t len0)
{
   int32_t i;
   int32_t tmp;
   X2C_PCOPY((void **)&b,b_len);
   osi_WrStr("r[", 3ul);
   tmp = len0-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      osi_WrHex((uint32_t)(uint8_t)b[i], 3UL);
      if (i==tmp) break;
   } /* end for */
   osi_WrStrLn("]", 2ul);
   X2C_PFREE(b);
} /* end showh() */


static int32_t btow(const char b[], uint32_t b_len, uint32_t p, uint32_t len0)
{
   uint32_t w;
   w = 0UL;
   while (len0>0UL) {
      --len0;
      w = w*256UL+(uint32_t)(uint8_t)b[p+len0];
   }
   return (int32_t)w;
} /* end btow() */

#define gps2aprs_ZEROTIME 315964800

#define gps2aprs_RAD 1.7453292519943E-2


static void rawbyte(char c)
{
   uint32_t week;
   uint32_t tow;
   uint32_t ps;
   uint8_t sum2;
   uint8_t sum1;
   double clat;
   double slat;
   double clong;
   double slong;
   double ve;
   double vn;
   char s[31];
   /*WrHex(ORD(c), 3); */
   gpsb[gpsp] = c;
   if (gpsp==0UL) {
      if (c=='\265') ++gpsp;
      else ++rawframeerr;
   }
   else if (gpsp==1UL) {
      if (c!='b') gpsp = 0UL;
      else ++gpsp;
   }
   else {
      ++gpsp;
      if (gpsp>250UL) gpsp = 0UL;
      else if (gpsp>=5UL) {
         if (gpsp>(uint32_t)(uint8_t)gpsb[4U]+(uint32_t)(uint8_t)gpsb[5U]*256UL+7UL) {
            sum1 = (uint8_t)((uint32_t)(uint8_t)gpsb[2U]+(uint32_t)(uint8_t)gpsb[3U]);
            sum2 = (uint8_t)((uint32_t)(uint8_t)gpsb[2U]*2UL+(uint32_t)(uint8_t)gpsb[3U]);
            ps = 4UL;
            while (ps+2UL<gpsp) {
               sum1 += gpsb[ps];
               sum2 += sum1;
               ++ps;
            }
            if ((uint32_t)sum1==(uint32_t)(uint8_t)gpsb[gpsp-2UL] && (uint32_t)sum2==(uint32_t)(uint8_t)gpsb[gpsp-1UL]) {
               /* checksum ok */
               if (((uint32_t)(uint8_t)gpsb[2U]==1UL && (uint32_t)(uint8_t)gpsb[3U]==6UL) && gpsp==60UL) {
                  /* ecef frame */
                  tow = (uint32_t)btow(gpsb, 251ul, 6UL, 4UL);
                  week = (uint32_t)btow(gpsb, 251ul, 14UL, 2UL);
                  pos.fix = (uint32_t)btow(gpsb, 251ul, 16UL, 1UL); /* 0 no fix, 2 2d, 3 3d, 5 time only */
                  pos.date = week*604800UL+tow/1000UL+(uint32_t)(315964800L-pos.leapseconds);
                  pos.dateok = pos.fix>0UL && pos.timeflags==7U;
                  pos.vx = (double)btow(gpsb, 251ul, 34UL, 4UL)*0.01;
                  pos.vy = (double)btow(gpsb, 251ul, 38UL, 4UL)*0.01;
                  pos.vz = (double)btow(gpsb, 251ul, 42UL, 4UL)*0.01;
                  pos.sats = (uint32_t)btow(gpsb, 251ul, 53UL, 1UL);
                  pos.speedok = 1;
               }
               else if (((uint32_t)(uint8_t)gpsb[2U]==1UL && (uint32_t)(uint8_t)gpsb[3U]==2UL) && gpsp==36UL) {
                  /* navPOSLLH frame */
                  pos.long0 = (double)btow(gpsb, 251ul, 10UL, 4UL)*1.E-7;
                  pos.lat = (double)btow(gpsb, 251ul, 14UL, 4UL)*1.E-7;
                  pos.alt = (double)btow(gpsb, 251ul, 22UL, 4UL)*0.001;
                  pos.posok = 1;
                  if (pos.fix==3UL) pos.altok = 10UL;
               }
               else if (((uint32_t)(uint8_t)gpsb[2U]==1UL && (uint32_t)(uint8_t)gpsb[3U]==32UL) && gpsp==24UL) {
                  /* navTIMEGPS frame */
                  pos.leapseconds = btow(gpsb, 251ul, 16UL, 1UL);
                  if (pos.leapseconds>=128L) pos.leapseconds -= 256L;
                  /*          WrInt(btow(gpsb,6,4) DIV 1000, 10);                        (* ms in week *) */
                  pos.timeflags = (uint8_t)btow(gpsb, 251ul, 17UL, 1UL); /* 1:tow ok 2:week ok 4:leapsecond ok */
               }
               else if (verb) {
                  /* other frame types */
                  showh(gpsb, 251ul, (int32_t)gpsp);
               }
               if (pos.posok && pos.speedok) {
                  if (pos.fix>1UL && pos.fix<5UL) {
                     slong = (double)osic_sin((float)(pos.long0*1.7453292519943E-2));
                     clong = (double)osic_cos((float)(pos.long0*1.7453292519943E-2));
                     slat = (double)osic_sin((float)(pos.lat*1.7453292519943E-2));
                     clat = (double)osic_cos((float)(pos.lat*1.7453292519943E-2));
                     vn = (-(pos.vx*slat*clong)-pos.vy*slat*slong)+pos.vz*clat;
                     ve = -(pos.vx*slong)+pos.vy*clong;
                     pos.climb = pos.vx*clat*clong+pos.vy*clat*slong+pos.vz*slat;
                     pos.course = atang2(vn, ve)*5.7295779513082E+1;
                     if (pos.course<0.0) pos.course = 360.0+pos.course;
                     pos.speed = (double)(osic_sqrt((float)(vn*vn+ve*ve))*3.6f);
                     newpos();
                  }
                  strncpy(Logl,"sats=",251u);
                  aprsstr_IntToStr((int32_t)pos.sats, 1UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " fix=", 6ul);
                  aprsstr_IntToStr((int32_t)pos.fix, 1UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " ", 2ul);
                  if (pos.dateok) aprsstr_DateToStr(pos.date, s, 31ul);
                  else strncpy(s,"[no time fix]",31u);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " vx=", 5ul);
                  aprsstr_FixToStr((float)pos.vx, 2UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " vy=", 5ul);
                  aprsstr_FixToStr((float)pos.vy, 2UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " vz=", 5ul);
                  aprsstr_FixToStr((float)pos.vz, 2UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " ", 2ul);
                  aprsstr_FixToStr((float)pos.lat, 5UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " ", 2ul);
                  aprsstr_FixToStr((float)pos.long0, 5UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, " ", 2ul);
                  aprsstr_FixToStr((float)pos.alt, 2UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, "m ", 3ul);
                  aprsstr_FixToStr((float)pos.speed, 2UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, "km/h ", 6ul);
                  aprsstr_IntToStr((int32_t)X2C_TRUNCI(pos.course,X2C_min_longint,X2C_max_longint), 1UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, "deg ", 5ul);
                  aprsstr_FixToStr((float)pos.climb, 2UL, s, 31ul);
                  aprsstr_Append(Logl, 251ul, s, 31ul);
                  aprsstr_Append(Logl, 251ul, "m/s", 4ul);
                  if (verb) osi_WrStrLn(Logl, 251ul);
                  WrLog(Logl, 251ul, aprsstr_Length(Logl, 251ul));
                  Logl[0] = 0;
                  pos.posok = 0;
                  pos.speedok = 0;
               }
               rawframeerr = 0UL;
               sumerrcnt = 0UL;
            }
            else {
               ++sumerrcnt;
               if (verb) {
                  osic_WrUINT32(sumerrcnt, 1UL);
                  osi_WrStrLn(" raw-checksum err", 18ul);
               }
            }
            gpsp = 0UL;
         }
      }
   }
} /* end rawbyte() */

static uint8_t _cnst0[22] = {6U,0U,1U,0U,0U,0U,208U,8U,0U,0U,0U,0U,0U,0U,3U,0U,2U,0U,0U,0U,0U,0U};
static uint8_t _cnst[22] = {6U,0U,1U,0U,0U,0U,208U,8U,0U,0U,0U,0U,0U,0U,1U,0U,1U,0U,0U,0U,0U,0U};

static void ubloxon(char on, uint32_t baud0)
{
   uint8_t s[22];
   uint32_t i;
   sumerrcnt = 0UL;
   rawframeerr = 0UL;
   if (on) {
      if (verb) osi_WrStrLn("switch gps to ubloxraw", 23ul);
      for (i = 0UL; i<=21UL; i++) {
         s[i] = _cnst[i];
      } /* end for */
   }
   else {
      if (verb) osi_WrStrLn("switch gps to nmea", 19ul);
      for (i = 0UL; i<=21UL; i++) {
         s[i] = _cnst0[i];
      } /* end for */
   }
   s[10U] = (uint8_t)(baud0&255UL);
   s[11U] = (uint8_t)(baud0/256UL&255UL);
   s[12U] = (uint8_t)(baud0/65536UL&255UL);
   initubloxraw(s, 22ul);
   if (on) {
      usleep(200000UL);
      FrameOn(1U, 6U); /* speeds */
      FrameOn(1U, 2U); /* egmalt, pos */
      FrameOn(1U, 32U); /* leap second */
      if (balloon) SetDynamicModel(6U);
      if (fixalt<X2C_max_longcard) SetFixAlt(fixalt);
      if (dynamicmodel<X2C_max_longcard) SetDynamicModel((uint8_t)dynamicmodel);
      if (timepuls) TimePuls(timepuls, duration);
   }
/*    Power(10000, 10000); */
} /* end ubloxon() */

/*-----ublox raw */
static int32_t len;

static int32_t rp;


static void gpsbyte(char c)
{
   if (c=='\015' || c=='\012') {
      if (gpsp>0UL) {
         pos.dateok = 0;
         if (sumoff || checksum(gpsb, 251ul, gpsp)) {
            decodeline(gpsb, 251ul, gpsp, &pos);
            WrLog(gpsb, 251ul, gpsp);
         }
         else pos.posok = 0;
         if (verb) showline(gpsb, 251ul, gpsp);
         if ((verb2 && pos.lastsat>0UL) && pos.satcnt==pos.lastsat) {
            showsats(pos.satcnt, pos.satpos, 12ul);
            memset((char *)pos.satpos,(char)0,sizeof(struct SAT [12]));
            pos.lastsat = 0UL;
         }
         newpos();
      }
      gpsp = 0UL;
   }
   else if ((uint8_t)c>' ' && gpsp<250UL) {
      gpsb[gpsp] = c;
      ++gpsp;
   }
} /* end gpsbyte() */

static void resetgps(int32_t);


static void resetgps(int32_t signum)
{
   if (ubloxraw) ubloxon(0, baud);
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   X2C_HALT((uint32_t)signum);
} /* end resetgps() */

/* Power(5000,10000) */

X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   int32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGTERM, resetgps);
   signal(SIGINT, resetgps);
   sumoff = 0;
   anyip = 1;
   junk = 1;
   memset((char *) &pos,(char)0,sizeof(struct POS));
   verb = 0;
   verb2 = 0;
   usbrobust = 1;
   balloon = 0;
   baud = 4800UL;
   strncpy(ttynamee,"/dev/ttyS0",1024u);
   logfilename[0] = 0;
   udpsock = -1L;
   gpsp = 0UL;
   mycall[0U] = 0;
   via[0U] = 0;
   comment0[0U] = 0;
   symt = '/';
   symb = '/';
   comptyp = 0UL;
   withalti = 1;
   withspeed = 1;
   withdao = 0;
   micessid = 0UL;
   btime0 = 180UL;
   btimedrive = 15UL;
   drivekm = 4UL;
   btimeN = 0UL;
   comintval = 5UL;
   comcnt = 0UL;
   btime = 0UL;
   msgtyp = 6UL;
   toport = 9002UL;
   ipnum = 2130706433UL;
   toport2 = 0UL;
   ipnum2 = 0UL;
   btimenavi = 2UL;
   useaxudp = 1;
   timesetdone = 1;
   terminatetimeset = 0;
   timecorr = 0L;
   frames = 0UL;
   Logl[0] = 0;
   dynamicmodel = X2C_max_longcard;
   fixalt = X2C_max_longcard;
   fixmode = 3UL;
   sumerrcnt = 0UL;
   timepuls = 0L;
   pconfig = 0;
   Parms();
   if (aprsstr_Length(mycall, 100ul)<3UL && !terminatetimeset) osi_WrStrLn("no tx without <mycall>", 23ul);
   /*Gencrctab; */
   opentty();
   udpsock = openudp();
   if (udpsock<0L) Error("cannot open udp socket", 23ul);
   while (pconfig) {
      uploadcfg(pconfig->str, 251ul);
      pconfig = pconfig->next;
   }
   if (ubloxraw) ubloxon(1, baud);
   for (;;) {
      fdclr();
      fdsetr((uint32_t)tty);
      if (selectr(0UL, 0UL)>=0L) {
         if (issetr((uint32_t)tty)) {
            len = osi_RdBin(tty, (char *)tbuf, 1024u/1u, 1024UL);
            testtty(len, &junk);
            if (!junk) {
               tmp = len-1L;
               rp = 0L;
               if (rp<=tmp) for (;; rp++) {
                  if (ubloxraw) rawbyte(tbuf[rp]);
                  else gpsbyte(tbuf[rp]);
                  if (rp==tmp) break;
               } /* end for */
            }
            else if (ubloxraw) ubloxon(1, baud);
            junk = 0;
            if (ubloxraw && rawframeerr>=200UL) {
               /* maybe gps reset and is in nmea */
               if (verb) osi_WrStrLn("rawmode framing errors", 23ul);
               ubloxon(1, baud);
            }
         }
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
