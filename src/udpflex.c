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
#define udpflex_C_
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef osi_H_
#include "osic.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif

/* interface rmnc-flex-kiss <> axudp by oe5dxl */
#define udpflex_DEFTTY "/dev/ttyS0"

#define udpflex_SLEEPWINLOOP 20
/* win poll data loop ms */

#define udpflex_SLEEPCONNTCP 2000
/* wait before new tcp network kiss connect ms */

static unsigned short udpflex_POLYNOM[256] = {3975U,7694U,11413U,15644U,
                18851U,22570U,27313U,31544U,33743U,37446U,41181U,45396U,
                50667U,54370U,59129U,63344U,7942U,3727U,15380U,11677U,22818U,
                18603U,31280U,27577U,37710U,33479U,45148U,41429U,54634U,
                50403U,63096U,59377U,11909U,16140U,3479U,7198U,26785U,31016U,
                19379U,23098U,41677U,45892U,33247U,36950U,58601U,62816U,
                51195U,54898U,15876U,12173U,7446U,3231U,30752U,27049U,23346U,
                19131U,45644U,41925U,37214U,32983U,62568U,58849U,55162U,
                50931U,19843U,23562U,28305U,32536U,2983U,6702U,10421U,14652U,
                49611U,53314U,58073U,62288U,34799U,38502U,42237U,46452U,
                23810U,19595U,32272U,28569U,6950U,2735U,14388U,10685U,53578U,
                49347U,62040U,58321U,38766U,34535U,46204U,42485U,27777U,
                32008U,20371U,24090U,10917U,15148U,2487U,6206U,57545U,61760U,
                50139U,53842U,42733U,46948U,34303U,38006U,31744U,28041U,
                24338U,20123U,14884U,11181U,6454U,2239U,61512U,57793U,54106U,
                49875U,46700U,42981U,38270U,34039U,35727U,39430U,43165U,
                47380U,52651U,56354U,61113U,65328U,1991U,5710U,9429U,13660U,
                16867U,20586U,25329U,29560U,39694U,35463U,47132U,43413U,
                56618U,52387U,65080U,61361U,5958U,1743U,13396U,9693U,20834U,
                16619U,29296U,25593U,43661U,47876U,35231U,38934U,60585U,
                64800U,53179U,56882U,9925U,14156U,1495U,5214U,24801U,29032U,
                17395U,21114U,47628U,43909U,39198U,34967U,64552U,60833U,
                57146U,52915U,13892U,10189U,5462U,1247U,28768U,25065U,21362U,
                17147U,51595U,55298U,60057U,64272U,36783U,40486U,44221U,
                48436U,17859U,21578U,26321U,30552U,999U,4718U,8437U,12668U,
                55562U,51331U,64024U,60305U,40750U,36519U,48188U,44469U,
                21826U,17611U,30288U,26585U,4966U,751U,12404U,8701U,59529U,
                63744U,52123U,55826U,44717U,48932U,36287U,39990U,25793U,
                30024U,18387U,22106U,8933U,13164U,503U,4222U,63496U,59777U,
                56090U,51859U,48684U,44965U,40254U,36023U,29760U,26057U,
                22354U,18139U,12900U,9197U,4470U,255U};

static unsigned short udpflex_SMACK[256] = {0U,49345U,49537U,320U,49921U,
                960U,640U,49729U,50689U,1728U,1920U,51009U,1280U,50625U,
                50305U,1088U,52225U,3264U,3456U,52545U,3840U,53185U,52865U,
                3648U,2560U,51905U,52097U,2880U,51457U,2496U,2176U,51265U,
                55297U,6336U,6528U,55617U,6912U,56257U,55937U,6720U,7680U,
                57025U,57217U,8000U,56577U,7616U,7296U,56385U,5120U,54465U,
                54657U,5440U,55041U,6080U,5760U,54849U,53761U,4800U,4992U,
                54081U,4352U,53697U,53377U,4160U,61441U,12480U,12672U,61761U,
                13056U,62401U,62081U,12864U,13824U,63169U,63361U,14144U,
                62721U,13760U,13440U,62529U,15360U,64705U,64897U,15680U,
                65281U,16320U,16000U,65089U,64001U,15040U,15232U,64321U,
                14592U,63937U,63617U,14400U,10240U,59585U,59777U,10560U,
                60161U,11200U,10880U,59969U,60929U,11968U,12160U,61249U,
                11520U,60865U,60545U,11328U,58369U,9408U,9600U,58689U,9984U,
                59329U,59009U,9792U,8704U,58049U,58241U,9024U,57601U,8640U,
                8320U,57409U,40961U,24768U,24960U,41281U,25344U,41921U,
                41601U,25152U,26112U,42689U,42881U,26432U,42241U,26048U,
                25728U,42049U,27648U,44225U,44417U,27968U,44801U,28608U,
                28288U,44609U,43521U,27328U,27520U,43841U,26880U,43457U,
                43137U,26688U,30720U,47297U,47489U,31040U,47873U,31680U,
                31360U,47681U,48641U,32448U,32640U,48961U,32000U,48577U,
                48257U,31808U,46081U,29888U,30080U,46401U,30464U,47041U,
                46721U,30272U,29184U,45761U,45953U,29504U,45313U,29120U,
                28800U,45121U,20480U,37057U,37249U,20800U,37633U,21440U,
                21120U,37441U,38401U,22208U,22400U,38721U,21760U,38337U,
                38017U,21568U,39937U,23744U,23936U,40257U,24320U,40897U,
                40577U,24128U,23040U,39617U,39809U,23360U,39169U,22976U,
                22656U,38977U,34817U,18624U,18816U,35137U,19200U,35777U,
                35457U,19008U,19968U,36545U,36737U,20288U,36097U,19904U,
                19584U,35905U,17408U,33985U,34177U,17728U,34561U,18368U,
                18048U,34369U,33281U,17088U,17280U,33601U,16640U,33217U,
                32897U,16448U};

#define udpflex_FEND 192

#define udpflex_FESC 219

#define udpflex_TFEND 220

#define udpflex_TFESC 221

static char udpflex_FLEXKISSBYTE = ' ';

static char udpflex_SMACKBYTE = '\200';

#define udpflex_ESC "\033"

#define udpflex_CR "\015"

#define udpflex_DEFAULTPORT "8001"

static unsigned char CRCL[256];

static unsigned char CRCH[256];

static char ifn[701];

static char ubuf[701];

static char tbuf[701];

static char kbuf[701];

static char ttynamee[701];

static unsigned long inilen;

static unsigned long kissm;

static unsigned long baud;

static unsigned long flexmod;

struct _0;


struct _0 {
   long sock;
   unsigned long ipnum;
   unsigned long fromport;
   unsigned long toport;
   char anyip;
};

static struct _0 udpsocks[8];

static long tty;

static struct termios saved;

static long len;

static long rp;

static long upos;

static long tpos;

static long i;

static char c;

static char auto0;

static char verb;

static char hexdump;

static char kisson;

static char usbrobust;

static char direwolf;

static unsigned long errtime;

static long tcpfd;

static char direwolfurl[2048];

static char direwolfport[11];


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osic_WrStr("udpflex: ", 10ul);
   osic_WrStr(text, text_len);
   osic_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void initfile(char fn[], unsigned long fn_len)
{
   char b[4001];
   long fd;
   long len0;
   X2C_PCOPY((void **)&fn,fn_len);
   if (fn[0UL]) {
      fd = osic_OpenRead(fn, fn_len);
      if (fd!=-1L) {
         len0 = osic_RdBin(fd, (char *)b, 4001u/1u, 4001UL);
         osic_WrBin(tty, (char *)b, 4001u/1u, (unsigned long)len0);
         osic_Close(fd);
      }
      else Error("initfile not found", 19ul);
   }
   X2C_PFREE(fn);
} /* end initfile() */


static void inittnc(void)
{
   errtime = TimeConv_time();
   initfile(ifn, 701ul);
   if (kisson) {
      strncpy(tbuf,"\015\033@K\015",701u);
      usleep(500000UL);
      osic_WrBin(tty, (char *)tbuf, 701u/1u, 5UL);
   }
   if (inilen>0UL) {
      usleep(500000UL);
      osic_WrBin(tty, (char *)kbuf, 701u/1u, inilen);
   }
   if (verb) osic_WrStrLn("send init to tnc", 17ul);
/*  kissm:=0; */
} /* end inittnc() */


static void SetComMode(long fd, unsigned long baud0)
{
   struct termios term;
   long res;
   unsigned long bd;
   struct termios * anonym;
   if (baud0==1200UL) bd = 9UL;
   else if (baud0==2400UL) bd = 11UL;
   else if (baud0==4800UL) bd = 12UL;
   else if (baud0==9600UL) bd = 13UL;
   else if (baud0==19200UL) bd = 14UL;
   else if (baud0==38400UL) bd = 15UL;
   else if (baud0==57600UL) bd = 4097UL;
   else if (baud0==115200UL) bd = 4098UL;
   else if (baud0==230400UL) bd = 4099UL;
   else if (baud0==460800UL) bd = 4100UL;
   else Error("unknown baudrate", 17ul);
   res = tcgetattr(fd, &saved);
   res = tcgetattr(fd, &term);
   { /* with */
      struct termios * anonym = &term;
      anonym->c_lflag = 0UL;
      anonym->c_oflag = 0UL;
      anonym->c_iflag = 0UL;
      /*  cfmakeraw(&termios);*/
      anonym->c_cflag = 2224UL+bd; /*+CRTSCTS*/ /*0800018B2H*/
   }
   res = tcsetattr(fd, 2L, &term);
} /* end SetComMode() */


static void opentty(void)
{
   for (;;) {
      tty = osic_OpenRW(ttynamee, 701ul);
      if (tty>=0L) {
         SetComMode(tty, baud);
         break;
      }
      if (!usbrobust) Error("tty open", 9ul);
      if (verb) osic_WrStrLn("tty open error", 15ul);
      usleep(1000000UL);
   }
} /* end opentty() */


static void testtty(long len0)
{
   if (len0<=0L) {
      osic_Close(tty);
      usleep(1000000UL);
      opentty();
   }
} /* end testtty() */

#define udpflex_ERRWAIT 4


static void kisserr(void)
{
   if (errtime+4UL<TimeConv_time()) errtime = 0UL;
} /* end kisserr() */


static char GetNum(const char h[], unsigned long h_len, char eot,
                unsigned long * p, unsigned long * n)
{
   *n = 0UL;
   while ((unsigned char)h[*p]>='0' && (unsigned char)h[*p]<='9') {
      *n = ( *n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
      ++*p;
   }
   return h[*p]==eot;
} /* end GetNum() */


static void app(unsigned long c0, char esc)
{
   if (esc) {
      if (c0==192UL) {
         kbuf[inilen] = '\333';
         ++inilen;
         c0 = 220UL;
      }
      else if (c0==219UL) {
         kbuf[inilen] = '\333';
         ++inilen;
         c0 = 221UL;
      }
   }
   kbuf[inilen] = (char)c0;
   ++inilen;
} /* end app() */


static void urlport(char url[], unsigned long url_len, char port[],
                unsigned long port_len, char s[], unsigned long s_len)
{
   long ii;
   long i0;
   X2C_PCOPY((void **)&s,s_len);
   s[s_len-1] = 0;
   if (s[0UL]=='[') {
      ii = 1L;
      while (s[ii] && s[ii]!=']') ++ii;
      if (s[ii]!=']' || s[ii+1L]!=':') Error("-g [url]:port", 14ul);
      s[ii] = 0;
      i0 = 1L;
      while (i0<=(long)(s_len-1)) {
         s[i0-1L] = s[i0];
         ++i0;
      }
   }
   else ii = aprsstr_InStr(s, s_len, ":", 2ul);
   if (ii>=0L) s[ii] = 0;
   if (ii==0L) aprsstr_Assign(url, url_len, "127.0.0.1", 10ul);
   else aprsstr_Assign(url, url_len, s, s_len);
   port[0UL] = 0;
   if (ii>=0L) {
      /* port number */
      ++ii;
      i0 = 0L;
      while (ii<=(long)(s_len-1) && i0<(long)(port_len-1)) {
         port[i0] = s[ii];
         ++i0;
         ++ii;
      }
      port[i0] = 0;
   }
   if (port[0UL]==0) aprsstr_Assign(port, port_len, "8001", 5ul);
   X2C_PFREE(s);
} /* end urlport() */


static void Parms(void)
{
   char err;
   char h[1024];
   unsigned long sockc;
   unsigned long val;
   unsigned long cmd;
   unsigned long i0;
   struct _0 * anonym;
   err = 0;
   sockc = 0UL;
   for (;;) {
      Lib_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='c') {
         }
         else if (h[1U]=='r') {
            /* dummy for compatibility */
            flexmod = 1UL; /* RMNC crc type */
         }
         else if (h[1U]=='s') flexmod = 2UL;
         else if (h[1U]=='a') {
            auto0 = 1;
            if (sockc>1UL) Error("-a only with 1 UDP socket, ", 28ul);
         }
         else if (h[1U]=='k') kisson = 1;
         else if (h[1U]=='p') {
            Lib_NextArg(h, 1024ul);
            i0 = 0UL;
            if (!GetNum(h, 1024ul, ':', &i0, &cmd)) {
               Error("-p cmd:value", 13ul);
            }
            ++i0;
            if (!GetNum(h, 1024ul, 0, &i0, &val)) {
               Error("-p cmd:value", 13ul);
            }
            app(192UL, 0);
            app(cmd, 1);
            app(val, 1);
            app(192UL, 0);
         }
         else if (h[1U]=='U') {
            if (auto0 && sockc>0UL) {
               Error("-U only 1 UDP with -a Mode", 27ul);
            }
            Lib_NextArg(h, 1024ul);
            { /* with */
               struct _0 * anonym = &udpsocks[sockc];
               if (aprsstr_GetIp2(h, 1024ul, &anonym->ipnum, &anonym->toport,
                 &anonym->fromport, &anonym->anyip)<0L) {
                  Error("-U wrong ip:port:port number", 29ul);
               }
               anonym->sock = openudp();
               if (anonym->sock<0L || bindudp(anonym->sock,
                anonym->fromport)<0L) {
                  /* say with port */
                  Error("-U cannot open udp socket", 26ul);
               }
            }
            ++sockc;
         }
         else if (h[1U]=='u') usbrobust = 1;
         else if (h[1U]=='t') {
            Lib_NextArg(h, 1024ul);
            i0 = 0UL;
            while ((h[i0] && h[i0]!=':') && i0<700UL) {
               ttynamee[i0] = h[i0];
               ++i0;
            }
            ttynamee[i0] = 0;
            if (h[i0]) {
               ++i0;
               if (!GetNum(h, 1024ul, 0, &i0, &baud)) {
                  Error("need ttydevice:baud", 20ul);
               }
            }
         }
         else if (h[1U]=='T') {
            Lib_NextArg(h, 1024ul);
            urlport(direwolfurl, 2048ul, direwolfport, 11ul, h, 1024ul);
            direwolf = 1;
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            hexdump = 1;
         }
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osic_WrStrLn(" -a                                automatic swit\
ch to KISS/FLEX/SMACK mode", 76ul);
               osic_WrStrLn("                                   (only with 1 P\
orts)", 55ul);
               osic_WrStrLn(" -r                                use rmnc-crc",
                 48ul);
               osic_WrStrLn(" -i <filename>                     send this file\
 to tty to switch on kiss", 75ul);
               osic_WrStrLn(" -k                                tnc2 tf switch\
 on kiss", 58ul);
               osic_WrStrLn(" -p <cmd>:<value>                  tnc2 parameter\
 1=txd, 2=p", 61ul);
               osic_WrStrLn("                                   3=slottime, 25\
5:13 kiss exit", 64ul);
               osic_WrStrLn("                                   (add 16 to cmd\
 for next Port)", 65ul);
               osic_WrStrLn(" -s                                SMACK (crc) on\
", 50ul);
               osic_WrStrLn(" -t <tty>:<baud>                   /dev/ttyS0:960\
0", 51ul);
               osic_WrStrLn(" -T [ip]:[port]                    TCP-KISS -T 12\
7.0.0.1:8001 (default)", 72ul);
               osic_WrStrLn("                                   dire-wolf soun\
dmodem -T :", 61ul);
               osic_WrStrLn(" -U <x.x.x.x:destport:listenport>  axudp  destpor\
t/listenport check ip", 71ul);
               osic_WrStrLn("                                   (repeat for mo\
re Ports)", 59ul);
               osic_WrStrLn(" -u                                retry until (r\
e)pluged (USB) tty", 68ul);
               osic_WrStrLn("                                   and on kiss er\
ror reinitialize kiss mode", 76ul);
               osic_WrStrLn(" -v                                verbous error \
messages", 58ul);
               osic_WrStrLn(" -V                                verbous errors\
 and monitor data to stdout", 77ul);
               osic_WrStrLn(" -h                                this", 40ul);
               osic_WrLn();
               X2C_ABORT();
            }
            if (h[1U]=='i') Lib_NextArg(ifn, 701ul);
            else err = 1;
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
   if (flexmod==1UL && sockc>1UL) Error("only 1 UDP with flexnet", 24ul);
   if (err) {
      osic_WrStr(">", 2ul);
      osic_WrStr(h, 1024ul);
      osic_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */

#define udpflex_POLINOM 0x8408 


static void Gencrctab(void)
{
   unsigned long c0;
   unsigned long crc0;
   unsigned long i0;
   for (c0 = 0UL; c0<=255UL; c0++) {
      crc0 = 255UL-c0;
      for (i0 = 0UL; i0<=7UL; i0++) {
         if ((crc0&1)) {
            crc0 = (unsigned long)((unsigned long)(crc0>>1)^0x8408UL);
         }
         else crc0 = crc0>>1;
      } /* end for */
      CRCL[c0] = (unsigned char)crc0;
      CRCH[c0] = (unsigned char)(255UL-(crc0>>8));
   } /* end for */
} /* end Gencrctab() */


static unsigned long UDPCRC(const char frame[], unsigned long frame_len,
                long size)
{
   unsigned char h;
   unsigned char l;
   unsigned char b;
   long i0;
   long tmp;
   l = 0U;
   h = 0U;
   tmp = size-1L;
   i0 = 0L;
   if (i0<=tmp) for (;; i0++) {
      b = (unsigned char)((unsigned char)(unsigned char)frame[i0]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i0==tmp) break;
   } /* end for */
   return (unsigned long)(unsigned char)(char)l+256UL*(unsigned long)
                (unsigned char)(char)h;
} /* end UDPCRC() */

static unsigned short _cnst[256] = {3975U,7694U,11413U,15644U,18851U,22570U,
                27313U,31544U,33743U,37446U,41181U,45396U,50667U,54370U,
                59129U,63344U,7942U,3727U,15380U,11677U,22818U,18603U,31280U,
                27577U,37710U,33479U,45148U,41429U,54634U,50403U,63096U,
                59377U,11909U,16140U,3479U,7198U,26785U,31016U,19379U,23098U,
                41677U,45892U,33247U,36950U,58601U,62816U,51195U,54898U,
                15876U,12173U,7446U,3231U,30752U,27049U,23346U,19131U,45644U,
                41925U,37214U,32983U,62568U,58849U,55162U,50931U,19843U,
                23562U,28305U,32536U,2983U,6702U,10421U,14652U,49611U,53314U,
                58073U,62288U,34799U,38502U,42237U,46452U,23810U,19595U,
                32272U,28569U,6950U,2735U,14388U,10685U,53578U,49347U,62040U,
                58321U,38766U,34535U,46204U,42485U,27777U,32008U,20371U,
                24090U,10917U,15148U,2487U,6206U,57545U,61760U,50139U,53842U,
                42733U,46948U,34303U,38006U,31744U,28041U,24338U,20123U,
                14884U,11181U,6454U,2239U,61512U,57793U,54106U,49875U,46700U,
                42981U,38270U,34039U,35727U,39430U,43165U,47380U,52651U,
                56354U,61113U,65328U,1991U,5710U,9429U,13660U,16867U,20586U,
                25329U,29560U,39694U,35463U,47132U,43413U,56618U,52387U,
                65080U,61361U,5958U,1743U,13396U,9693U,20834U,16619U,29296U,
                25593U,43661U,47876U,35231U,38934U,60585U,64800U,53179U,
                56882U,9925U,14156U,1495U,5214U,24801U,29032U,17395U,21114U,
                47628U,43909U,39198U,34967U,64552U,60833U,57146U,52915U,
                13892U,10189U,5462U,1247U,28768U,25065U,21362U,17147U,51595U,
                55298U,60057U,64272U,36783U,40486U,44221U,48436U,17859U,
                21578U,26321U,30552U,999U,4718U,8437U,12668U,55562U,51331U,
                64024U,60305U,40750U,36519U,48188U,44469U,21826U,17611U,
                30288U,26585U,4966U,751U,12404U,8701U,59529U,63744U,52123U,
                55826U,44717U,48932U,36287U,39990U,25793U,30024U,18387U,
                22106U,8933U,13164U,503U,4222U,63496U,59777U,56090U,51859U,
                48684U,44965U,40254U,36023U,29760U,26057U,22354U,18139U,
                12900U,9197U,4470U,255U};

static unsigned long flexcrc(const char buf[], unsigned long buf_len,
                unsigned long a, unsigned long b)
{
   unsigned short crc0;
   crc0 = 0xFFFFU;
   while (a<=b) {
      crc0 = X2C_LSH(crc0,16,8)^(unsigned short)_cnst[(unsigned long)(X2C_LSH(crc0,
                16,-8)^(unsigned short)(unsigned long)(unsigned char)buf[a])];
      ++a;
   }
   return (unsigned long)crc0;
} /* end flexcrc() */

static unsigned short _cnst0[256] = {0U,49345U,49537U,320U,49921U,960U,640U,
                49729U,50689U,1728U,1920U,51009U,1280U,50625U,50305U,1088U,
                52225U,3264U,3456U,52545U,3840U,53185U,52865U,3648U,2560U,
                51905U,52097U,2880U,51457U,2496U,2176U,51265U,55297U,6336U,
                6528U,55617U,6912U,56257U,55937U,6720U,7680U,57025U,57217U,
                8000U,56577U,7616U,7296U,56385U,5120U,54465U,54657U,5440U,
                55041U,6080U,5760U,54849U,53761U,4800U,4992U,54081U,4352U,
                53697U,53377U,4160U,61441U,12480U,12672U,61761U,13056U,
                62401U,62081U,12864U,13824U,63169U,63361U,14144U,62721U,
                13760U,13440U,62529U,15360U,64705U,64897U,15680U,65281U,
                16320U,16000U,65089U,64001U,15040U,15232U,64321U,14592U,
                63937U,63617U,14400U,10240U,59585U,59777U,10560U,60161U,
                11200U,10880U,59969U,60929U,11968U,12160U,61249U,11520U,
                60865U,60545U,11328U,58369U,9408U,9600U,58689U,9984U,59329U,
                59009U,9792U,8704U,58049U,58241U,9024U,57601U,8640U,8320U,
                57409U,40961U,24768U,24960U,41281U,25344U,41921U,41601U,
                25152U,26112U,42689U,42881U,26432U,42241U,26048U,25728U,
                42049U,27648U,44225U,44417U,27968U,44801U,28608U,28288U,
                44609U,43521U,27328U,27520U,43841U,26880U,43457U,43137U,
                26688U,30720U,47297U,47489U,31040U,47873U,31680U,31360U,
                47681U,48641U,32448U,32640U,48961U,32000U,48577U,48257U,
                31808U,46081U,29888U,30080U,46401U,30464U,47041U,46721U,
                30272U,29184U,45761U,45953U,29504U,45313U,29120U,28800U,
                45121U,20480U,37057U,37249U,20800U,37633U,21440U,21120U,
                37441U,38401U,22208U,22400U,38721U,21760U,38337U,38017U,
                21568U,39937U,23744U,23936U,40257U,24320U,40897U,40577U,
                24128U,23040U,39617U,39809U,23360U,39169U,22976U,22656U,
                38977U,34817U,18624U,18816U,35137U,19200U,35777U,35457U,
                19008U,19968U,36545U,36737U,20288U,36097U,19904U,19584U,
                35905U,17408U,33985U,34177U,17728U,34561U,18368U,18048U,
                34369U,33281U,17088U,17280U,33601U,16640U,33217U,32897U,
                16448U};

static unsigned long smackcrc(const char buf[], unsigned long buf_len,
                unsigned long a, unsigned long b)
{
   unsigned short crc0;
   crc0 = 0U;
   while (a<=b) {
      /*  crc = ((crc >> 8) & 0xff) ^ crc_table[(crc ^ *buf++) & 0xff]; */
      crc0 = X2C_LSH(crc0,16,-8)^(unsigned short)_cnst0[(unsigned long)((crc0^(unsigned short)(unsigned long)(unsigned char)buf[a])&0xFFU)];
      ++a;
   }
   /*WrHex(CAST(CARDINAL, crc) DIV 256, 3);
                WrHex(CAST(CARDINAL, crc) MOD 256, 3);  WrStrLn("=crc"); */
   return (unsigned long)(X2C_LSH(crc0,16,-8)|X2C_LSH(crc0&0xFFU,16,8));
} /* end smackcrc() */


static void sendudp(char buf[], unsigned long buf_len, long len0,
                unsigned long port)
{
   long i0;
   unsigned long crc0;
   struct _0 * anonym;
   X2C_PCOPY((void **)&buf,buf_len);
   crc0 = UDPCRC(buf, buf_len, len0);
   buf[len0] = (char)(crc0&255UL);
   buf[len0+1L] = (char)(crc0/256UL);
   { /* with */
      struct _0 * anonym = &udpsocks[port];
      i0 = udpsend(anonym->sock, buf, len0+2L, anonym->toport,
                anonym->ipnum);
   }
   X2C_PFREE(buf);
/*
FOR i:=0 TO upos-2 DO IO.WrHex(ORD(buf[i]), 3) END; WrLn;
*/
} /* end sendudp() */


static long getudp(long fd, char buf[], unsigned long buf_len,
                unsigned long inip, char any)
{
   unsigned long fromport;
   unsigned long ip;
   long len0;
   char udp2[100];
   len0 = udpreceive(fd, buf, (long)(buf_len), &fromport, &ip);
   if (len0==-1L) return 0L;
   if ((len0<=2L || len0>=(long)(buf_len)) || any && inip!=ip) return -1L;
   if ((unsigned long)(unsigned char)buf[len0-2L]+256UL*(unsigned long)
                (unsigned char)buf[len0-1L]!=UDPCRC(buf, buf_len, len0-2L)) {
      return -2L;
   }
   if (buf[0UL]=='\001') aprsstr_extrudp2(buf, buf_len, udp2, 100ul, &len0);
   return len0;
} /* end getudp() */


static void conntcp(long * fd, char url[], unsigned long url_len,
                char port[], unsigned long port_len)
{
   long res;
   X2C_PCOPY((void **)&url,url_len);
   X2C_PCOPY((void **)&port,port_len);
   *fd = connectto(url, port);
   if ((long)*fd>=0L) res = socknonblock(*fd);
   X2C_PFREE(url);
   X2C_PFREE(port);
} /* end conntcp() */


static void WCh(char c0)
{
   if (c0!='\015') {
      if ((unsigned char)c0<' ' || (unsigned char)c0>='\177') {
         osic_WrStr(".", 2ul);
      }
      else osic_WrStr((char *) &c0, 1u/1u);
   }
} /* end WCh() */


static void ShowCall(char f[], unsigned long f_len, unsigned long pos)
{
   unsigned long e;
   unsigned long i0;
   unsigned long tmp;
   char tmp0;
   e = pos;
   tmp = pos+5UL;
   i0 = pos;
   if (i0<=tmp) for (;; i0++) {
      if (f[i0]!='@') e = i0;
      if (i0==tmp) break;
   } /* end for */
   tmp = e;
   i0 = pos;
   if (i0<=tmp) for (;; i0++) {
      WCh((char)((unsigned long)(unsigned char)f[i0]>>1));
      if (i0==tmp) break;
   } /* end for */
   i0 = (unsigned long)(unsigned char)f[pos+6UL]>>1&15UL;
   if (i0) {
      osic_WrStr("-", 2ul);
      if (i0>=10UL) {
         osic_WrStr((char *)(tmp0 = (char)(i0/10UL+48UL),&tmp0),
                1u/1u);
      }
      osic_WrStr((char *)(tmp0 = (char)(i0%10UL+48UL),&tmp0), 1u/1u);
   }
} /* end ShowCall() */

static unsigned long udpflex_UA = 0x63UL;

static unsigned long udpflex_DM = 0xFUL;

static unsigned long udpflex_SABM = 0x2FUL;

static unsigned long udpflex_DISC = 0x43UL;

static unsigned long udpflex_FRMR = 0x87UL;

static unsigned long udpflex_UI = 0x3UL;

static unsigned long udpflex_RR = 0x1UL;

static unsigned long udpflex_REJ = 0x9UL;

static unsigned long udpflex_RNR = 0x5UL;


static void Showctl(unsigned long com, unsigned long cmd)
{
   unsigned long cm;
   char PF[4];
   char tmp;
   osic_WrStr(" ctl ", 6ul);
   cm = (unsigned long)cmd&~0x10UL;
   if ((cm&0xFUL)==0x1UL) {
      osic_WrStr("RR", 3ul);
      osic_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0xFUL)==0x5UL) {
      osic_WrStr("RNR", 4ul);
      osic_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0xFUL)==0x9UL) {
      osic_WrStr("REJ", 4ul);
      osic_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0x1UL)==0UL) {
      osic_WrStr("I", 2ul);
      osic_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
      osic_WrStr((char *)(tmp = (char)(48UL+(cmd>>1&7UL)),&tmp),
                1u/1u);
   }
   else if (cm==0x3UL) osic_WrStr("UI", 3ul);
   else if (cm==0xFUL) osic_WrStr("DM", 3ul);
   else if (cm==0x2FUL) osic_WrStr("SABM", 5ul);
   else if (cm==0x43UL) osic_WrStr("DISC", 5ul);
   else if (cm==0x63UL) osic_WrStr("UA", 3ul);
   else if (cm==0x87UL) osic_WrStr("FRMR", 5ul);
   else osic_WrHex(cmd, 1UL);
   strncpy(PF,"v^-+",4u);
   if (com==0UL || com==3UL) osic_WrStr("v1", 3ul);
   else {
      osic_WrStr((char *) &PF[(com&1UL)+2UL*(unsigned long)
                ((0x10UL & (unsigned long)cmd)!=0)], 1u/1u);
   }
} /* end Showctl() */


static void ShowFrame(char f[], unsigned long f_len, unsigned long len0,
                char port)
{
   unsigned long i0;
   char d;
   char v;
   osic_WrStr((char *) &port, 1u/1u);
   i0 = 0UL;
   while (!((unsigned long)(unsigned char)f[i0]&1)) {
      ++i0;
      if (i0>len0) {
         osic_WrStrLn(" no axudp (no address end mark)", 32ul);
         return;
      }
   }
   /* no address end mark found */
   if (i0%7UL!=6UL) {
      osic_WrStrLn(" no axudp (address field size not modulo 7)", 44ul);
      return;
   }
   /* address end not modulo 7 error */
   osic_WrStr(":fm ", 5ul);
   ShowCall(f, f_len, 7UL);
   osic_WrStr(" to ", 5ul);
   ShowCall(f, f_len, 0UL);
   i0 = 14UL;
   v = 1;
   while (i0+6UL<len0 && !((unsigned long)(unsigned char)f[i0-1UL]&1)) {
      if (v) {
         osic_WrStr(" via", 5ul);
         v = 0;
      }
      osic_WrStr(" ", 2ul);
      ShowCall(f, f_len, i0);
      if ((unsigned long)(unsigned char)f[i0+6UL]>=128UL && (((unsigned long)
                (unsigned char)f[i0+6UL]&1) || (unsigned long)(unsigned char)
                f[i0+13UL]<128UL)) osic_WrStr("*", 2ul);
      i0 += 7UL;
   }
   Showctl((unsigned long)((0x80U & (unsigned char)(unsigned char)f[6UL])!=0)
                +2UL*(unsigned long)((0x80U & (unsigned char)(unsigned char)
                f[13UL])!=0), (unsigned long)(unsigned char)f[i0]);
   ++i0;
   if (i0<len0) {
      osic_WrStr(" pid ", 6ul);
      osic_WrHex((unsigned long)(unsigned char)f[i0], 1UL);
   }
   ++i0;
   osic_WrLn();
   /*  IF NOT noinfo THEN */
   d = 0;
   while (i0<len0) {
      if (f[i0]!='\015') {
         WCh(f[i0]);
         d = 1;
      }
      else if (d) {
         osic_WrLn();
         d = 0;
      }
      ++i0;
   }
   if (d) osic_WrLn();
/*  END; */
} /* end ShowFrame() */

static unsigned long tncport;

static unsigned long crc;

/*testloop:CARDINAL; */

X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   long tmp;
   long tmp0;
   Lib_BEGIN(argc, argv);
   osic_BEGIN();
   aprsstr_BEGIN();
   auto0 = 0;
   kisson = 0;
   verb = 0;
   hexdump = 0;
   flexmod = 0UL;
   usbrobust = 0;
   direwolf = 0;
   baud = 9600UL;
   ttynamee[0] = 0;
   ifn[0] = 0;
   tcpfd = -1L;
   for (tncport = 0UL; tncport<=7UL; tncport++) {
      udpsocks[tncport].sock = -1L;
   } /* end for */
   inilen = 0UL;
   Parms();
   Gencrctab();
   if (ttynamee[0U]) opentty();
   /*IF udpsock<0 THEN Error("udpport open") END; */
   /*WrInt(udpsock, 10); WrStrLn(" sock"); */
   errtime = 0UL;
   upos = 0L;
   for (;;) {
      if (errtime==0UL) inittnc();
      if (direwolf && tcpfd<0L) {
         if (verb) {
            osic_WrStr("connect ", 9ul);
            osic_WrStr(direwolfurl, 2048ul);
            osic_WrStr(":", 2ul);
            osic_WrStrLn(direwolfport, 11ul);
         }
         conntcp(&tcpfd, direwolfurl, 2048ul, direwolfport, 11ul);
      }
      fdclr();
      if (direwolf) {
         if (tcpfd>=0L) fdsetr((unsigned long)tcpfd);
      }
      else fdsetr((unsigned long)tty);
      for (tncport = 0UL; tncport<=7UL; tncport++) {
         if (udpsocks[tncport].sock!=-1L) {
            fdsetr((unsigned long)udpsocks[tncport].sock);
         }
      } /* end for */
      if (selectr(0UL, 0UL)>=0L) {
         if ((direwolf && tcpfd>=0L) && issetr((unsigned long)tcpfd)
                || !direwolf && issetr((unsigned long)tty)) {
            if (direwolf) {
               len = readsock(tcpfd, tbuf, 701L);
               if (len<0L) {
                  osic_CloseSock(tcpfd);
                  tcpfd = -1L;
                  len = 0L;
                  usleep(2000000UL);
               }
            }
            else {
               /* disconnected */
               len = osic_RdBin(tty, (char *)tbuf, 701u/1u, 701UL);
            }
            if (!direwolf && usbrobust) testtty(len);
            /*WrInt(len, 5); WrLn; */
            tmp = len-1L;
            rp = 0L;
            if (rp<=tmp) for (;; rp++) {
               c = tbuf[rp];
               /*WrStr(c); */
               if (kissm==0UL) {
                  if (c=='\300') {
                     upos = 0L;
                     kissm = 1UL;
                  }
                  else kisserr();
               }
               else if (kissm==1UL) {
                  if (c=='\300') {
                     if (upos==0L || flexmod==1UL) tncport = 0UL;
                     else {
                        tncport = (unsigned long)(unsigned char)
                ubuf[0U]/16UL&7UL;
                     }
                     /*            IF hexdump & (upos>0) THEN */
                     /*              IF ubuf[0]
                =FLEXKISSBYTE THEN WrStr("FLEX") */
                     /*              ELSIF ubuf[0]
                =SMACKBYTE THEN WrStr("SMACK") */
                     /*              ELSE WrStr("KISS") END; */
                     /*              FOR i:=0 TO upos-1 DO WrHex(ORD(ubuf[i])
                , 3) END; */
                     /*              WrLn; */
                     /*              ShowFrame(ubuf, upos,
                CHR(tncport+ORD("0"))); */
                     /*            END; */
                     if (upos>4L) {
                        if (auto0) {
                           if (ubuf[0U]==' ') {
                              if (verb && flexmod!=1UL) {
                                 osic_WrStrLn("switching to FLEXKISS", 22ul);
                              }
                              flexmod = 1UL;
                           }
                           else if (ubuf[0U]=='\200') {
                              if (verb && flexmod!=2UL) {
                                 osic_WrStrLn("switching to SMACK", 19ul);
                              }
                              flexmod = 2UL;
                           }
                           else {
                              if (verb && flexmod) {
                                 osic_WrStrLn("switching to KISS", 18ul);
                              }
                              flexmod = 0UL;
                           }
                        }
                        /*                IF (flexmod=0)
                & (ubuf[0]<>FLEXKISSBYTE) & (ubuf[0]<>SMACKBYTE) */
                        if ((flexmod==0UL || flexmod==1UL && flexcrc(ubuf,
                701ul, 0UL, (unsigned long)(upos-1L))==28784UL) || flexmod==2UL && smackcrc(ubuf,
                 701ul, 0UL, (unsigned long)(upos-1L))==0UL) {
                           if (flexmod>0UL) {
                              upos -= 2L; /* remove crc */
                           }
                           c = ubuf[0U];
                           tmp0 = upos-2L;
                           i = 0L;
                           if (i<=tmp0) for (;; i++) {
                              ubuf[i] = ubuf[i+1L]; /* remove port byte */
                              if (i==tmp0) break;
                           } /* end for */
                           --upos;
                           if (hexdump) {
                              if (flexmod==1UL && c==' ') {
                                 osic_WrStr("FLEX", 5ul);
                              }
                              else if ((unsigned char)c>=(unsigned char)
                '\200') osic_WrStr("SMACK", 6ul);
                              else {
                                 osic_WrStr("KISS", 5ul);
                              }
                              ShowFrame(ubuf, 701ul, (unsigned long)upos,
                (char)(tncport+48UL));
                           }
                           sendudp(ubuf, 701ul, upos, tncport);
                        }
                        else if (verb) {
                           osic_WrStrLn("serialport-crc error", 21ul);
                        }
                     }
                     /*                FOR i:=0 TO upos-1 DO WrHex(ORD(ubuf[i]
                ), 3) END; */
                     /*                WrLn; */
                     upos = 0L;
                     kissm = 1UL;
                  }
                  else if (c=='\333') {
                     kissm = 2UL;
                  }
                  else if (upos<700L) {
                     ubuf[upos] = c;
                     ++upos;
                  }
                  else kisserr();
               }
               else if (kissm==2UL) {
                  if (c=='\334') c = '\300';
                  else if (c=='\335') c = '\333';
                  if (upos<700L) {
                     ubuf[upos] = c;
                     ++upos;
                  }
                  else kisserr();
                  kissm = 1UL;
               }
               if (rp==tmp) break;
            } /* end for */
         }
         for (tncport = 0UL; tncport<=7UL; tncport++) {
            if (udpsocks[tncport].sock!=-1L) {
               if (issetr((unsigned long)udpsocks[tncport].sock)) {
                  { /* with */
                     struct _0 * anonym = &udpsocks[tncport];
                     len = getudp(anonym->sock, ubuf, 701ul, anonym->ipnum,
                anonym->anyip);
                  }
                  if (len>=2L && len<698L) {
                     if (hexdump) {
                        osic_WrStr("UDP", 4ul);
                        ShowFrame(ubuf, 701ul, (unsigned long)(len-2L),
                (char)(tncport+48UL));
                     }
                     for (i = len; i>=1L; i--) {
                        ubuf[i] = ubuf[i-1L];
                     } /* end for */
                     if (flexmod==1UL) ubuf[0U] = ' ';
                     else if (flexmod==2UL) {
                        ubuf[0U] = (char)(128UL+16UL*tncport);
                     }
                     else ubuf[0U] = (char)(16UL*tncport);
                     --len;
                     if (flexmod>0UL) {
                        if (flexmod==1UL) {
                           crc = flexcrc(ubuf, 701ul, 0UL,
                (unsigned long)(len-1L));
                        }
                        else {
                           crc = smackcrc(ubuf, 701ul, 0UL,
                (unsigned long)(len-1L));
                        }
                        ubuf[len+1L] = (char)(crc&255UL);
                        ubuf[len] = (char)(crc/256UL);
                        len += 2L;
                     }
                     /*            IF hexdump THEN */
                     /*              WrStr("UDP:");
                WrStr(CHR(tncport+ORD("0"))); WrStr(" ");  */
                     /*              FOR i:=0 TO len-1 DO WrHex(ORD(ubuf[i]),
                 3) END; */
                     /*              WrLn; */
                     /*            END;  */
                     kbuf[0U] = '\300';
                     tpos = 1L;
                     tmp = len-1L;
                     rp = 0L;
                     if (rp<=tmp) for (;; rp++) {
                        c = ubuf[rp];
                        if (c=='\300') {
                           kbuf[tpos] = '\333';
                           ++tpos;
                           c = '\334';
                        }
                        else if (c=='\333') {
                           kbuf[tpos] = '\333';
                           ++tpos;
                           c = '\335';
                        }
                        kbuf[tpos] = c;
                        ++tpos;
                        if (rp==tmp) break;
                     } /* end for */
                     kbuf[tpos] = '\300';
                     if (direwolf) {
                        if ((long)tcpfd>=0L) {
                           if (sendsock(tcpfd, kbuf, tpos+1L)<0L) {
                              /* disconnected */
                              osic_CloseSock(tcpfd);
                              tcpfd = -1L;
                           }
                        }
                     }
                     else {
                        osic_WrBin(tty, (char *)kbuf, 701u/1u,
                (unsigned long)(tpos+1L));
                     }
                  }
                  else if (verb) {
                     /*WrInt(tpos+1, 10); WrInt(dlen, 10); WrLn; */
                     if (len==-2L) osic_WrStrLn("axudp crc error", 16ul);
                     else if (len==-1L) {
                        osic_WrStrLn("axudp from wrong source ip", 27ul);
                     }
                     else if (len) osic_WrStrLn("axudp length error", 19ul);
                  }
               }
            }
         } /* end for */
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
