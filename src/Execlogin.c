/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef Execlogin_H_
#include "Execlogin.h"
#endif
#define Execlogin_C_
#include <unistd.h>
#include <fcntl.h>
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#include <stdlib.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <signal.h>



/*FROM stat IMPORT stat, stat_t;*/
/*, execv*/
/*IMPORT ptty; */
/**********************************************************************/
/* Find a currently unused pseudo tty and open its master side;
 * return-value: 0=ok, <0=error
 * on successfull function return, fd will contain the file descriptor,
 * and PtyName the absolute path to the master-pty in the file system */

static int32_t GetNewTty(int32_t * fd, char ptsname[],
                uint32_t ptsname_len)
{
   int32_t res;
   struct termios term;
   *fd = open("/dev/ptmx", O_RDWR+O_NONBLOCK);
   if (*fd<0L) osi_WrStrLn("error /dev/ptmx open", 21ul);
   if (osi_getptsname(*fd, (char *)ptsname, ptsname_len)) {
      osi_WrStrLn("no ttyname", 11ul);
   }
   /*  WrStrLn(ptsname); */
   if (osic_grantpts(*fd)) osi_WrStrLn("error ptty grant", 17ul);
   if (osic_unlockpts(*fd)) osi_WrStrLn("error ptty unlock", 18ul);
   res = tcgetattr(*fd, &term);
   term.c_lflag = 0UL;
   res = tcsetattr(*fd, 0L, &term);
   return 0L;
} /* end GetNewTty() */

/**********************************************************************/
/* open the corresponding slave of a pseudo tty pair, adjust the
 * terminal setting, redirect our stdin/out/err to this tty, and
 * finalle make it our controlling terminal.
 * return: 0=success, -1=failure */

static int32_t GetSlavePty(char name[], uint32_t name_len)
{
   int32_t fd;
   int32_t res;
   struct termios term;
   struct termios * anonym;
   int32_t GetSlavePty_ret;
   X2C_PCOPY((void **)&name,name_len);
   res = chown((char *)name, 0L, 0L);
   res = chmod((char *)name, 384L);
   /*        IF setsid()<0 THEN Close(fd); RETURN -1; END; */
   /*        fd:=open(name, O_RDWR+O_NOCTTY); */
   fd = open(name, O_RDWR);
   if (fd<0L) {
      GetSlavePty_ret = -1L;
      goto label;
   }
   /* now adjust the terminal settings */
   res = tcgetattr(fd, &term);
   { /* with */
      struct termios * anonym = &term;
      anonym->c_lflag = (uint32_t)((uint32_t)anonym->c_lflag|0x8UL|0x2UL)
                ;
      anonym->c_oflag = (uint32_t)((uint32_t)anonym->c_oflag|0x4UL);
      anonym->c_iflag = (uint32_t)(((uint32_t)
                anonym->c_iflag|0x100UL|0x1UL)&~0x1000UL);
   }
   /*          c_cflag := CARDINAL( BITSET(c_cflag)-BITSET(CLOCAL)); */
   res = tcsetattr(fd, 0L, &term);
   /* create new session id, and set controlling terminal */
   if (setsid()<0L) {
      osic_Close(fd);
      GetSlavePty_ret = -1L;
      goto label;
   }
   if (ioctl(fd, 21518L, 0)<0L) {
      osic_Close(fd);
      GetSlavePty_ret = -1L;
      goto label;
   }
   /* and finally redirect stdin, stdout, stderr */
   if (fd) res = dup2(fd, 0L);
   if (fd!=1L) res = dup2(fd, 1L);
   if (fd!=2L) res = dup2(fd, 2L);
   if (fd>2L) osic_Close(fd);
   GetSlavePty_ret = 0L;
   label:;
   X2C_PFREE(name);
   return GetSlavePty_ret;
} /* end GetSlavePty() */

/**********************************************************************/
/* Construct an array of arguments, and execute /bin/login with it.
 * 'name' contains the username, which will be passed to /bin/login.
 *        it may be empty, in which case login will ask for the name.
 * 'host' may optionally contain the users remote hostname, which e.g.
 *        is displayed by the 'who' command.
 * 'passwd' can be set TRUE to force skipping the password authentication.
 *        This requires 'name' to be non-empty. */

extern int32_t Execlogin_StartLogin(struct Execlogin_tEXEC * cmd)
{
   int32_t res;
   int32_t PtyFd;
   char PtyName[1024];
   if (GetNewTty(&PtyFd, PtyName, 1024ul)<0L) return -1L;
   res = fork();
   if (res<0L) {
      osic_Close(PtyFd);
      return -1L;
   }
   if (res>0L) {
      /* daddy */
      /* signal(SIGINT, SIG_IGN); */
      signal(SIGCHLD, SIG_IGN);
      return PtyFd;
   }
   else {
      /* child */
      for (res = 0L; res<=1023L; res++) {
         osic_Close(res);
      } /* end for */
      osic_Close(PtyFd);
      /*
                       signal.signal(signal.SIGINT, signal.SIG_IGN);
                       signal.signal(signal.SIGHUP, signal.SIG_IGN);
      */
      res = GetSlavePty(PtyName, 1024ul);
      if (res<0L) exit(1L);
      res = execv(cmd->cmdfn, cmd->args);
      /*              res := execve(cmd.cmdfn^, cmd.args^, NIL); */
      /*              RETURN -1; */
      X2C_ABORT();
   }
   return 0;
} /* end StartLogin() */


extern void Execlogin_BEGIN(void)
{
   static int Execlogin_init = 0;
   if (Execlogin_init) return;
   Execlogin_init = 1;
   aprsstr_BEGIN();
   osi_BEGIN();
}

