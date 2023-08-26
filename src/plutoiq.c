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
#define plutoiq_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <signal.h>
#ifndef plutoio_H_
#include "plutoio.h"
#endif

/* plutosdr send iq file/pipe */
/*
gcc -Wall -Wno-unused-variable -Wno-parentheses -Wno-pointer-sign -Wno-format -Wno-return-type -Wno-char-subscripts -Wno-uninitialized -Werror=implicit-function-declaration -I. -c -O2 -fdata-sections -ffunction-sections -o plutoiq.o plutoiq.c
cc -Wl,--gc-sections -o plutoiq aprsstr.o osic.o osi.o -lm plutoiq.o plutoio.o -liio
*/
#define plutoiq_SAMPLES 16384

#define plutoiq_LF "\012"

static pIQBUF buf;

static uint32_t sample;

static float txmhz;

static float txgain;

static int32_t txbufs;

static int32_t len;

static int32_t fd;

static char exit0;

static char exitpipe;

static char ifn[4096];

static char uri[4096];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void Parms(void)
{
   char err;
   char h[4096];
   err = 0;
   for (;;) {
      osi_NextArg(h, 4096ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='F') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToFix(&txmhz, h, 4096ul)) {
               Error("-F <tx-MHz>", 12ul);
            }
         }
         else if (h[1U]=='G') {
            osi_NextArg(h, 4096ul);
            if (!aprsstr_StrToFix(&txgain, h, 4096ul)) {
               Error("-G <tx-gain>", 13ul);
            }
         }
         else if (h[1U]=='B') osi_NextArg(h, 4096ul);
         else if (h[1U]=='e') exitpipe = 1;
         else if (h[1U]=='i') {
            osi_NextArg(ifn, 4096ul);
            if (ifn[0U]==0 || ifn[0U]=='-') {
               Error("-i <input-iq-filename>", 23ul);
            }
         }
         else if (h[1U]=='u') {
            osi_NextArg(uri, 4096ul);
            if (uri[0U]==0) Error("-u <pluto-uri", 14ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn(" iq-File/Pipe to Plutosdr", 26ul);
               osi_WrStrLn(" -B <n>              tx bufs", 29ul);
               osi_WrStrLn(" -F <MHz>            tx LO (433.5)", 35ul);
               osi_WrStrLn(" -G <v>              tx Gain (-8)", 34ul);
               osi_WrStrLn(" -h                  this", 26ul);
               osi_WrStrLn(" -i <filename>       input iq file/pipe (/dev/std\
in)", 53ul);
               osi_WrStrLn(" -u <uri>            pluto uri (192.168.2.1)",
                45ul);
               osi_WrStrLn("", 1ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_Werr(">", 2ul);
      osi_Werr(h, 4096ul);
      osi_Werr("< use -h\012", 10ul);
      X2C_ABORT();
   }
} /* end Parms() */


static char rdpipe(char b[], uint32_t b_len, int32_t fd0,
                uint32_t len0)
{
   uint32_t i;
   int32_t res;
   char * p;
   i = 0UL;
   do {
      p = (char *) &b[i];
      res = osi_RdBin(fd0, p, 65536ul, len0-i);
      if (res<=0L) return 0;
      /* eof */
      i += (uint32_t)res;
   } while (i<len0);
   return 1;
} /* end rdpipe() */

static void exitprog(int32_t);


static void exitprog(int32_t signum)
{
   osi_WrStr("exit ", 6ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
   if (exitpipe || signum!=SIGPIPE) exit0 = 1;
} /* end exitprog() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   /*
     signal(SIGTERM, exitprog);
     signal(SIGINT, exitprog);
   */
   signal(SIGPIPE, exitprog);
   sample = 2112000UL;
   strncpy(ifn,"/dev/stdin",4096u);
   strncpy(uri,"192.168.2.1",4096u);
   txmhz = 433.5f;
   txgain = (-8.0f);
   txbufs = 8L;
   exitpipe = 0;
   Parms();
   fd = osi_OpenRead(ifn, 4096ul);
   if (fd>=0L) {
      osic_WrINT32((uint32_t)Initiio(uri), 1UL);
      osi_WrStrLn(":init", 6ul);
      osic_WrINT32((uint32_t)starttx(16384UL, &buf), 1UL);
      osi_WrStrLn(":starttx", 9ul);
      osic_WrINT32((uint32_t)settxbufs((uint32_t)txbufs), 1UL);
      osi_WrStrLn(":settxbufs", 11ul);
      osic_WrINT32((uint32_t)settxgain((short)(int32_t)
                X2C_TRUNCI(txgain,X2C_min_longint,X2C_max_longint)), 1UL);
      osi_WrStrLn(":txgain", 8ul);
      osic_WrINT32((uint32_t)setfreq((double)txmhz, 1), 1UL);
      osi_WrStrLn(":mhz", 5ul);
      osic_WrINT32((uint32_t)firenable(0), 1UL);
      osi_WrStrLn(":fir", 5ul);
      osic_WrINT32((uint32_t)setsampel(sample, 1), 1UL);
      osi_WrStrLn(":samphz", 8ul);
      osic_WrINT32((uint32_t)setbw(sample, 1), 1UL);
      osi_WrStrLn(":bwhz", 6ul);
      osic_WrINT32((uint32_t)setport("A", 1), 1UL);
      osi_WrStrLn(":rxport", 8ul);
      exit0 = 0;
      do {
         if (rdpipe((char *) &buf[0U], 2u/1u, fd, 65536UL)) {
            len = transmitbuf();
            osic_WrINT32((uint32_t)len, 8UL);
         }
         else if (exitpipe) exit0 = 1;
      } while (!exit0);
      closeiio();
   }
   else osi_Werr("Input file not readable\012", 25ul);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
