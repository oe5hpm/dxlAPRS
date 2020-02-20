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
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* read serial gps and make axudp aprs frames */
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

/*CRCL, CRCH: ARRAY[0..255] OF SET8;*/
static char tbuf[1024];

static char ttynamee[1024];

static char logfilename[1024];

static char mycall[100];

static char via[100];

static char comment0[100];

static char symt;

static char symb;

static uint32_t toport;

static uint32_t toport2;

static uint32_t baud;

static uint32_t ipnum;

static uint32_t ipnum2;

static int32_t udpsock;

static char verb;

static char anyip;

static char usbrobust;

static char junk;

static char sumoff;

static char withalti;

static char withspeed;

static char withdao;

static char useaxudp;

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

static double lat;

static double long0;

static double speed;

static double course;

static double alt;

static char posok;

static char altok;

static int32_t tty;

static struct termios saved;


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


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * p,
                uint32_t * ip, uint32_t * port)
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


static void SetComMode(int32_t fd, uint32_t baud0)
{
   struct termios term;
   int32_t res;
   int32_t bd;
   struct termios * anonym;
   if (baud0==1200UL) bd = 9L;
   else if (baud0==2400UL) bd = 11L;
   else if (baud0==4800UL) bd = 12L;
   else if (baud0==9600UL) bd = 13L;
   else if (baud0==19200UL) bd = 14L;
   else if (baud0==38400UL) bd = 15L;
   else if (baud0==57600UL) bd = 4097L;
   else if (baud0==115200UL) bd = 4098L;
   else if (baud0==230400UL) bd = 4099L;
   else if (baud0==460800UL) bd = 4100L;
   else Error("unknown baudrate", 17ul);
   res = tcgetattr(fd, &saved);
   res = tcgetattr(fd, &term);
   { /* with */
      struct termios * anonym = &term;
      anonym->c_lflag = 0UL;
      anonym->c_oflag = 0UL;
      anonym->c_iflag = 0UL;
      /*  cfmakeraw(&termios);*/
      anonym->c_cflag = (uint32_t)(2224L+bd); /*+CRTSCTS*/ /*0800018B2H*/
   }
   res = tcsetattr(fd, 2L, &term);
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


static void Parms(void)
{
   char err;
   char h[1024];
   uint32_t i;
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
            if (GetIp(h, 1024ul, &i, &ipnum2, &toport2)<0L) {
               Error("-N ip:port", 11ul);
            }
         }
         else if (h[1U]=='a') withalti = 0;
         else if (h[1U]=='u') usbrobust = 0;
         else if (h[1U]=='k') withspeed = 0;
         else if (h[1U]=='s') sumoff = 1;
         else if (h[1U]=='D') withdao = 1;
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 100ul);
            if ((uint8_t)mycall[0U]<'0') Error("-I <mycall>", 12ul);
         }
         else if (h[1U]=='w') {
            osi_NextArg(via, 100ul);
            if ((uint8_t)via[0U]<=' ') {
               Error("-m vias like RELAY,WIDE1-1", 27ul);
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(comment0, 100ul);
            if (aprsstr_Length(comment0, 100ul)>40UL) {
               Error("-c <comment>  (max 40 byte)", 28ul);
            }
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
               if (!GetNum(h, 1024ul, 0, &i, &baud)) {
                  Error("need ttydevice:baud", 20ul);
               }
            }
         }
         else if (h[1U]=='L') osi_NextArg(logfilename, 1024ul);
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &btimedrive)) Error("-b <s>", 7ul);
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
            if (!GetNum(h, 1024ul, 0, &i, &drivekm)) {
               Error("-g <km/h>", 10ul);
            }
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            i = 0UL;
            if (!GetNum(h, 1024ul, 0, &i, &comintval)) {
               Error("-l <n>", 7ul);
            }
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
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn("Read serial GPS and make normal/compressed/mic-e \
Beacon as AXUDP or monitor", 76ul);
               osi_WrStrLn(" -0 <s>                            standing Beaco\
n Time in Seconds (180)", 73ul);
               osi_WrStrLn(" -a                                altitude OFF",
                 48ul);
               osi_WrStrLn(" -b <s>                            driving Beacon\
 Time in Seconds (15)", 71ul);
               osi_WrStrLn(" -c <commentstring>                APRS Comment (\
max 40 char)", 62ul);
               osi_WrStrLn("                                     insert time \
hhmmss: \\\\h", 61ul);
               osi_WrStrLn("                                     insert time \
ddhhmm: \\\\z", 61ul);
               osi_WrStrLn("                                     insert file \
      : \\\\:filename:", 70ul);
               osi_WrStrLn("                                     insert \\\\ \
        : \\\\\\", 61ul);
               osi_WrStrLn("                                     double all \\
\ to pass thru bash eg. \\\\\\\\h", 78ul);
               osi_WrStrLn(" -d <x>                            Destination Ca\
ll SSID 0..7", 62ul);
               osi_WrStrLn(" -D                                DAO Extension \
on for 20cm Resolution", 72ul);
               osi_WrStrLn(" -f <x>                            format 0=norma\
l 1=compressed 2=mic-e (0)", 76ul);
               osi_WrStrLn(" -g <km/h>                         min. Speed for\
 driving Beacon Time (4)", 74ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I <mycall>                       Mycall with SS\
ID like NOCALL-15", 67ul);
               osi_WrStrLn(" -i <icon>                         2 Icon chars \\
"/-\" (House), \"/>\" (Car)...(//)", 80ul);
               osi_WrStrLn(" -k                                Speed/Course O\
FF (not in mic-e)", 67ul);
               osi_WrStrLn(" -l <n>                            every n Beacon\
s send one with Comment", 73ul);
               osi_WrStrLn(" -L <filename>                     Append raw GPS\
 text to this File", 68ul);
               osi_WrStrLn(" -m <x.x.x.x:destport>             use Monitor UD\
P format :port for localhost", 78ul);
               osi_WrStrLn(" -N <x.x.x.x:destport>             send Position \
AXUDP every 2s eg. to aprsmap", 79ul);
               osi_WrStrLn(" -n <s>                            Beacon Time in\
 Seconds to -N destination (2)", 80ul);
               osi_WrStrLn(" -r <x.x.x.x:destport>             send AXUDP (to\
 kiss-TNC or TCPKISS via udpflex,", 83ul);
               osi_WrStrLn("                                     to afskmodem\
 or via aprsmap or udpgate to TCP)", 84ul);
               osi_WrStrLn(" -s                                GPS Checksum c\
heck OFF", 58ul);
               osi_WrStrLn(" -t <tty>:<baud>                   (/dev/ttyS0:48\
00)", 53ul);
               osi_WrStrLn(" -u                                abort, not ret\
ry until open removable USB tty", 81ul);
               osi_WrStrLn(" -v                                verbous",
                43ul);
               osi_WrStrLn(" -w <viapath>                      via Path like \
RELAY,WIDE1-1", 63ul);
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
   if (comptyp==1UL && withdao) {
      Error("DAO Option not with compressed Format", 38ul);
   }
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */

/*
PROCEDURE Gencrctab;
CONST POLINOM=08408H;
VAR i,crc,c:CARDINAL;
BEGIN
  FOR c:=0 TO 255 DO
    crc:=255-c;
    FOR i:=0 TO 7 DO
      IF ODD(crc) THEN crc:=CAST(CARDINAL, CAST(BITSET, ASH(crc,
                -1))/CAST(BITSET,POLINOM))
      ELSE crc:=ASH(crc, -1) END;
    END;
    CRCL[c]:=CAST(SET8, crc);
    CRCH[c]:=CAST(SET8, 255 - ASH(crc, -8));
  END;
END Gencrctab;


PROCEDURE UDPCRC(frame-:ARRAY OF CHAR; size:INTEGER):CARDINAL;
VAR l,h:SET8;
    b:CARD8;
    i:INTEGER;
BEGIN
  l:=SET8{};
  h:=SET8{};
  FOR i:=0 TO size-1 DO
    b:=CAST(CARD8, CAST(SET8, frame[i]) / l);
    l:=CRCL[b] / h;
    h:=CRCH[b];
  END;
  RETURN ORD(CAST(CHAR, l))+256*ORD(CAST(CHAR, h))
END UDPCRC;
*/

static void sendudp(char buf[], uint32_t buf_len, int32_t len0,
                uint32_t ip, uint32_t port)
{
   int32_t i;
   /*  crc:CARDINAL;  */
   X2C_PCOPY((void **)&buf,buf_len);
   i = udpsend(udpsock, buf, len0, port, ip);
   X2C_PFREE(buf);
/*FOR i:=0 TO upos-2 DO IO.WrHex(ORD(buf[i]), 3) END; IO.WrLn; */
} /* end sendudp() */


static void skip(const char b[], uint32_t b_len, uint32_t * p,
                uint32_t len0)
{
   while (*p<len0 && b[*p]!=',') ++*p;
   if (*p<len0) ++*p;
} /* end skip() */


static char getnum(const char b[], uint32_t b_len,
                uint32_t * p, uint32_t len0, uint32_t * n)
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
            aprsstr_DateToStr(osic_time(), ds, 256ul);
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
               aprsstr_Assign(s, s_len, "beacon macro file not readable ",
                32ul);
               aprsstr_Append(s, s_len, fn, 1024ul);
               if (verb) osi_WrStrLn(s, s_len);
               s[0UL] = 0;
               return;
            }
         }
         else if (s[i]=='\\') aprsstr_Append(ns, 256ul, "\\\\", 3ul);
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


static void decodeline(const char b[], uint32_t b_len,
                uint32_t len0)
{
   uint32_t n;
   uint32_t i;
   double div0;
   char sign;
   if (b[0UL]=='$' && b[1UL]=='G') {
      /*& (b[2]="P")*/
      if ((b[3UL]=='R' && b[4UL]=='M') && b[5UL]=='C') {
         /* $GPRMC,141333.593,A,9815.1000,N,01302.2000,E,0.00,00.00,140410,0,
                ,A*7C */
         /* $GPRMC,112430.00,A,4812.41130,N,01305.61995,E,0.039,,200513,,,
                A*77 */
         i = 7UL;
         skip(b, b_len, &i, len0);
         if (b[i]!='A') return;
         skip(b, b_len, &i, len0);
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = (double)(float)(n*10UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(float)n;
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,6.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,60.0f));
         }
         else return;
         if (b[i]=='.') ++i;
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,600.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,6000.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,60000.0f));
         }
         if (getnum(b, b_len, &i, len0, &n)) {
            lat = lat+(double)(X2C_DIVR((float)n,6.E+5f));
         }
         skip(b, b_len, &i, len0);
         if (b[i]=='S') lat = -lat;
         else if (b[i]!='N') return;
         skip(b, b_len, &i, len0);
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = (double)(float)(n*100UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(float)(n*10UL);
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(float)n;
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,6.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,60.0f));
         }
         else return;
         if (b[i]=='.') ++i;
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,600.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,6000.0f));
         }
         else return;
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,60000.0f));
         }
         if (getnum(b, b_len, &i, len0, &n)) {
            long0 = long0+(double)(X2C_DIVR((float)n,6.E+5f));
         }
         skip(b, b_len, &i, len0);
         if (b[i]=='W') long0 = -long0;
         else if (b[i]!='E') return;
         skip(b, b_len, &i, len0);
         speed = 0.0;
         while (getnum(b, b_len, &i, len0, &n)) {
            speed = speed*10.0+(double)(float)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               speed = speed+(double)(float)n*div0;
               div0 = div0*0.1;
            }
         }
         speed = speed*1.851984; /* knots to km/h */
         skip(b, b_len, &i, len0);
         course = 0.0;
         while (getnum(b, b_len, &i, len0, &n)) {
            course = course*10.0+(double)(float)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               course = course+(double)(float)n*div0;
               div0 = div0*0.1;
            }
         }
         posok = 1;
      }
      else if ((b[3UL]=='G' && b[4UL]=='G') && b[5UL]=='A') {
         /* $GPGGA,152554,3938.5665,N,10346.2039,W,1,08,1.7,12382.7,M,-22.3,
                M,,*7B */
         /* $GPGGA,112435.00,4812.41112,N,01305.61998,E,1,08,1.04,398.3,M,
                44.9,M,,*59 */
         i = 7UL;
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         skip(b, b_len, &i, len0);
         alt = 0.0;
         if (b[i]=='-') {
            sign = 1;
            ++i;
         }
         else sign = 0;
         while (getnum(b, b_len, &i, len0, &n)) {
            altok = 1;
            alt = alt*10.0+(double)(float)n;
         }
         if (b[i]=='.') {
            ++i;
            div0 = 0.1;
            while (getnum(b, b_len, &i, len0, &n)) {
               alt = alt+(double)(float)n*div0;
               div0 = div0*0.1;
            }
         }
         if (sign) alt = -alt;
      }
   }
} /* end decodeline() */


static char Hex(uint32_t d)
{
   d = d&15UL;
   if (d>9UL) d += 7UL;
   return (char)(d+48UL);
} /* end Hex() */


static char checksum(const char b[], uint32_t b_len,
                uint32_t len0)
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
      if (b[i+1UL]!=Hex((uint32_t)cs/16UL) || b[i+2UL]!=Hex((uint32_t)
                cs&15UL)) ok0 = 0;
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


static void sendaprs(double lat0, double long1,
                double alt0, double course0, double speed0,
                 uint32_t comp0, char withspd, char withalt,
                char dao, char com, char comm[],
                uint32_t comm_len, char local, uint32_t ip,
                uint32_t port)
{
   char b[201];
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
   aprsstr_Append(b, 201ul, mycall, 100ul);
   micdest = aprsstr_Length(b, 201ul)+1UL;
   if (local) aprsstr_Append(b, 201ul, ">NOGATE", 8ul);
   else {
      aprsstr_Append(b, 201ul, ">APLT01", 8ul);
      if (micessid>0UL) {
         aprsstr_Append(b, 201ul, "-", 2ul);
         aprsstr_Append(b, 201ul,
                (char *)(tmp = (char)(micessid+48UL),&tmp), 1u/1u);
      }
      if (via[0U]) {
         aprsstr_Append(b, 201ul, ",", 2ul);
         aprsstr_Append(b, 201ul, via, 100ul);
      }
   }
   if (comp0==0UL) {
      /* uncompressed */
      aprsstr_Append(b, 201ul, ":!", 3ul);
      i = aprsstr_Length(b, 201ul);
      a = fabs(lat0);
      n = truncc(a);
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncc((a-(double)(float)n)*6000.0);
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
      if (lat0>=0.0) b[i] = 'N';
      else b[i] = 'S';
      ++i;
      b[i] = symt;
      ++i;
      a = fabs(long1);
      n = truncc(a);
      b[i] = num(n/100UL);
      ++i;
      b[i] = num(n/10UL);
      ++i;
      b[i] = num(n);
      ++i;
      n = truncc((a-(double)(float)n)*6000.0);
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
      if (long1>=0.0) b[i] = 'E';
      else b[i] = 'W';
      ++i;
      b[i] = symb;
      ++i;
      if (withspd) {
         n = truncc(course0+1.5);
         b[i] = num(n/100UL);
         ++i;
         b[i] = num(n/10UL);
         ++i;
         b[i] = num(n);
         ++i;
         b[i] = '/';
         ++i;
         n = truncc(speed0*5.3996146834962E-1+0.5);
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
         n = truncc(fabs(alt0*3.2808398950131+0.5));
         if (alt0>=0.0) b[i] = num(n/100000UL);
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
      b[i] = symt;
      ++i;
      if (lat0<90.0) n = truncc((90.0-lat0)*3.80926E+5);
      else n = 0UL;
      b[i] = (char)(33UL+n/753571UL);
      ++i;
      b[i] = (char)(33UL+(n/8281UL)%91UL);
      ++i;
      b[i] = (char)(33UL+(n/91UL)%91UL);
      ++i;
      b[i] = (char)(33UL+n%91UL);
      ++i;
      if (long1>(-180.0)) {
         n = truncc((180.0+long1)*1.90463E+5);
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
         b[i] = (char)(33UL+truncc(course0)/4UL);
         ++i;
         b[i] = (char)(33UL+truncc((double)(osic_ln((float)
                (speed0*5.3996146834962E-1+1.0))*1.29935872129E+1f)));
         ++i;
         b[i] = '_';
         ++i;
      }
      else if (withalt) {
         if (alt0*3.2808398950131>1.0) {
            n = truncc((double)(osic_ln((float)
                (alt0*3.2808398950131))*500.5f));
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
         n = truncc(alt0*3.2808398950131+0.5);
         if (alt0>=0.0) b[i] = num(n/10000UL);
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
      nl = truncc(fabs(long1));
      n = truncc(fabs(lat0));
      b[i] = (char)(80UL+n/10UL);
      ++i;
      b[i] = (char)(80UL+n%10UL);
      ++i;
      n = truncc((fabs(lat0)-(double)(float)n)*6000.0);
      b[i] = (char)(80UL+n/1000UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(lat0>=0.0)+(n/100UL)%10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(nl<10UL || nl>=100UL)+(n/10UL)
                %10UL);
      ++i;
      b[i] = (char)(48UL+32UL*(uint32_t)(long1<0.0)+n%10UL);
      i = aprsstr_Length(b, 201ul);
      if (nl<10UL) b[i] = (char)(nl+118UL);
      else if (nl>=100UL) {
         if (nl<110UL) b[i] = (char)(nl+8UL);
         else b[i] = (char)(nl-72UL);
      }
      else b[i] = (char)(nl+28UL);
      ++i;
      nl = truncc((fabs(long1)-(double)(float)nl)*6000.0);
                /* long min*100 */
      n = nl/100UL;
      if (n<10UL) n += 60UL;
      b[i] = (char)(n+28UL);
      ++i;
      b[i] = (char)(nl%100UL+28UL);
      ++i;
      n = truncc(speed0*5.3996146834962E-1+0.5);
      b[i] = (char)(n/10UL+28UL);
      ++i;
      nl = truncc(course0);
      b[i] = (char)(32UL+(n%10UL)*10UL+nl/100UL);
      ++i;
      b[i] = (char)(28UL+nl%100UL);
      ++i;
      b[i] = symb;
      ++i;
      b[i] = symt;
      ++i;
      if (withalt) {
         if (alt0>(-1.E+4)) {
            n = truncc(alt0+10000.5);
         }
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
      b[i] = (char)(33UL+dao91(lat0));
      ++i;
      b[i] = (char)(33UL+dao91(long1));
      ++i;
      b[i] = '!';
      ++i;
   }
   b[i] = 0;
   if (com) {
      beaconmacros(comm, comm_len);
      aprsstr_Append(b, 201ul, comm, comm_len);
   }
   /*  Append(b, CR+LF); */
   if (aprsstr_Length(mycall, 100ul)>=3UL) {
      if (useaxudp) {
         aprsstr_mon2raw(b, 201ul, raw, 361ul, &rp0);
         if (rp0>0L) sendudp(raw, 361ul, rp0, ip, port);
      }
      else {
         sendudp(b, 201ul, (int32_t)(aprsstr_Length(b, 201ul)+1UL), ip,
                port);
      }
   }
   if (verb) {
      osic_WrFixed((float)lat0, 6L, 10UL);
      osic_WrFixed((float)long1, 6L, 10UL);
      osic_WrFixed((float)speed0, 1L, 10UL);
      osi_WrStr("km/h", 5ul);
      osic_WrFixed((float)course0, 1L, 7UL);
      osi_WrStr("deg", 4ul);
      osic_WrFixed((float)alt0, 1L, 10UL);
      osi_WrStrLn("m", 2ul);
      osi_WrStrLn(b, 201ul);
   }
   X2C_PFREE(comm);
} /* end sendaprs() */

static char c;

static int32_t len;

static int32_t rp;

static uint32_t gpsp;

static char gpsb[100];

static int32_t logfd;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   int32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   sumoff = 0;
   anyip = 1;
   junk = 1;
   posok = 0;
   altok = 0;
   verb = 0;
   usbrobust = 1;
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
   Parms();
   if (aprsstr_Length(mycall, 100ul)<3UL) {
      osi_WrStrLn("no tx witout <mycall>", 22ul);
   }
   /*Gencrctab; */
   opentty();
   udpsock = openudp();
   if (udpsock<0L) Error("cannot open udp socket", 23ul);
   for (;;) {
      fdclr();
      fdsetr((uint32_t)tty);
      /*  fdsetr(udpsock); */
      if (selectr(0UL, 0UL)>=0L) {
         if (issetr((uint32_t)tty)) {
            len = osi_RdBin(tty, (char *)tbuf, 1024u/1u, 1024UL);
            testtty(len, &junk);
            if (!junk) {
               tmp = len-1L;
               rp = 0L;
               if (rp<=tmp) for (;; rp++) {
                  c = tbuf[rp];
                  if (c=='\015' || c=='\012') {
                     if (gpsp>0UL) {
                        if (sumoff || checksum(gpsb, 100ul, gpsp)) {
                           decodeline(gpsb, 100ul, gpsp);
                           if (logfilename[0U]) {
                              logfd = osi_OpenAppend(logfilename, 1024ul);
                              if (!osic_FdValid(logfd)) {
                                 logfd = osi_OpenWrite(logfilename, 1024ul);
                              }
                              if (osic_FdValid(logfd)) {
                                 osi_WrBin(logfd, (char *)gpsb, 100u/1u,
                gpsp);
                                 osi_WrBin(logfd, (char *)"\012", 2u/1u,
                1UL);
                                 osic_Close(logfd);
                              }
                              else {
                                 osi_WrStr("cannot write [", 15ul);
                                 osi_WrStr(logfilename, 1024ul);
                                 osi_WrStrLn("]", 2ul);
                              }
                           }
                        }
                        else {
                           altok = 0;
                           posok = 0;
                        }
                        if (verb) showline(gpsb, 100ul, gpsp);
                        if (posok) {
                           ++btime;
                           if (truncc(speed)>=drivekm) {
                              if (btime>=btimedrive) {
                                 btime = 0UL;
                              }
                           }
                           else if (btime>btime0) {
                              btime = 0UL;
                           }
                           if (btime==0UL) {
                              sendaprs(lat, long0, alt, course, speed,
                comptyp, withspeed, withalti && altok, withdao, comcnt==0UL,
                comment0, 100ul, 0, ipnum, toport);
                              ++comcnt;
                              if (comcnt>=comintval) {
                                 comcnt = 0UL;
                              }
                           }
                           if (toport2>0UL) {
                              /* second (fast) beacon to map */
                              ++btimeN;
                              if (btimeN>=btimenavi) {
                                 sendaprs(lat, long0, alt, course, speed,
                0UL, 1, altok, 1, 0, "", 1ul, 1, ipnum2, toport2);
                                 btimeN = 0UL;
                              }
                           }
                           altok = 0;
                           posok = 0;
                        }
                     }
                     gpsp = 0UL;
                  }
                  else if ((uint8_t)c>' ' && gpsp<99UL) {
                     gpsb[gpsp] = c;
                     ++gpsp;
                  }
                  if (rp==tmp) break;
               } /* end for */
            }
            junk = 0;
         }
      }
   }
/*    ELSIF issetr(udpsock) THEN */
/*      len:=getudp(udpsock, ubuf, ipnum, anyip); */
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
