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
#define ttytee_C_
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
#include <signal.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* read serial port send to unbreakable pipe(s) */
struct PIPE;

typedef struct PIPE * pPIPE;


struct PIPE {
   pPIPE next;
   char pipename[1024];
   int32_t pipefd;
};

static pPIPE outpipes;

static char dat[1024];

static char ttynamee[1024];

static char verb;

static char usbrobust;

static int32_t datlen;

static int32_t tty;

static uint32_t baud;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr("ttytee: ", 9ul);
   osi_Werr(text, text_len);
   osi_WerrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void SetComMode(int32_t fd, uint32_t baud0)
{
   if (osi_setttybaudraw(fd, baud0)<0L) {
      if (verb) osi_WerrLn("cannot config tty", 18ul);
   }
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
      if (verb) osi_WerrLn("tty open error", 15ul);
      usleep(1000000UL);
   }
} /* end opentty() */


static void testtty(int32_t len)
{
   if (len<=0L) {
      osic_Close(tty);
      usleep(1000000UL);
      opentty();
   }
} /* end testtty() */


static char GetNum(const char h[], uint32_t h_len, char eot, uint32_t * p0, uint32_t * n)
{
   *n = 0UL;
   while ((uint8_t)h[*p0]>='0' && (uint8_t)h[*p0]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)h[*p0])-48UL;
      ++*p0;
   }
   return h[*p0]==eot;
} /* end GetNum() */


static void Parms(void)
{
   char err;
   char h[1024];
   pPIPE p0;
   uint32_t i;
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='u') usbrobust = 0;
         else if (h[1U]=='p') {
            osi_NextArg(h, 1024ul);
            osic_alloc((char * *) &p0, sizeof(struct PIPE));
            if (p0==0) Error("out of memory", 14ul);
            aprsstr_Assign(p0->pipename, 1024ul, h, 1024ul);
            p0->pipefd = -1L;
            p0->next = outpipes;
            outpipes = p0;
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
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osic_WrLn();
               osi_WrStrLn("Read (unbreakable USB) serial port and send to unbreakable pipe(s)", 67ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -p <pipename>                     may be repeatet", 51ul);
               osi_WrStrLn(" -t <tty>:<baud>                   (/dev/ttyS0:4800)", 53ul);
               osi_WrStrLn(" -u                                abort, not retry until open removable USB tty", 81ul);
               osi_WrStrLn(" -v                                verbous", 43ul);
               osic_WrLn();
               osi_WrStrLn("before use make pipes with \"mknod <pipename> p\"", 48ul);
               osi_WrStrLn("for data on stdout as \"tee\" does, append \"-p /dev/stdout\"", 58ul);
               osic_WrLn();
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_WrStr(">", 2ul);
      osi_WrStr(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end Parms() */

static void pipebroken(int32_t);


static void pipebroken(int32_t signum)
{
   osi_WrStr("got signal ", 12ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
} /* end pipebroken() */

static pPIPE p;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   signal(SIGPIPE, pipebroken);
   verb = 0;
   usbrobust = 1;
   baud = 4800UL;
   strncpy(ttynamee,"/dev/ttyS0",1024u);
   outpipes = 0;
   Parms();
   opentty();
   for (;;) {
      datlen = osi_RdBin(tty, (char *)dat, 1024u/1u, 1024UL);
      testtty(datlen);
      p = outpipes;
      while (p) {
         { /* with */
            struct PIPE * anonym = p;
            if (anonym->pipename[0U]) {
               if (anonym->pipefd<0L) {
                  anonym->pipefd = osi_OpenNONBLOCK(anonym->pipename, 1024ul);
                  if ((verb && anonym->pipefd>=0L) && !osi_IsFifo(anonym->pipefd)) {
                     osi_WerrLn("", 1ul);
                     osi_Werr("warning: ", 10ul);
                     osi_Werr(anonym->pipename, 1024ul);
                     osi_WerrLn(" is not a fifo!", 16ul);
                  }
               }
            }
            /*          IF pipefd<0 THEN pipefd:=OpenWrite(pipename) END; */
            if (anonym->pipefd>=0L) {
               osi_WrBin(anonym->pipefd, (char *)dat, 1024u/1u, (uint32_t)datlen);
            }
         }
         p = p->next;
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
