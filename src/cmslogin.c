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
#define cmslogin_C_
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif

/* start tcp connection and make user/passwd dialog */
#define cmslogin_CR "\015"

#define cmslogin_LF "\012"

static char user[1025];

static char pw[1025];

static char h[1025];

static char b[1025];

static char ip[1025];

static char port[1025];

static int32_t fd;

static int32_t l;

static struct termios saved0;


static void Err(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr("cmslogin: ", 11ul);
   osi_WrStr(text, text_len);
   osi_WrStrLn("", 1ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Err() */


static void urlport(char s[], uint32_t s_len, char url[],
                uint32_t url_len, char port0[], uint32_t port_len)
/* url:port to url port */
{
   int32_t ii;
   uint32_t j;
   uint32_t i;
   X2C_PCOPY((void **)&s,s_len);
   url[0UL] = 0;
   port0[0UL] = 0;
   i = 0UL;
   while (i<=s_len-1 && s[i]==' ') ++i;
   if (i<=s_len-1 && s[i]=='[') {
      /* ipv6 url */
      j = 0UL;
      ++i;
      while ((i<=s_len-1 && s[i]) && s[i]!=']') {
         if (j<=url_len-1) {
            url[j] = s[i];
            ++j;
         }
         ++i;
      }
      if (j<=url_len-1) url[j] = 0;
      if (i<=s_len-1 && s[i]==']') {
         ++i;
         if (i<=s_len-1 && s[i]==':') {
            /* ipv6 port */
            j = 0UL;
            ++i;
            while (i<=s_len-1 && (uint8_t)s[i]>' ') {
               if (j<=port_len-1) {
                  port0[j] = s[i];
                  ++j;
               }
               ++i;
            }
            if (j<=port_len-1) port0[j] = 0;
         }
      }
   }
   else {
      /* ipv4 */
      aprsstr_Assign(url, url_len, s, s_len);
      ii = aprsstr_InStr(url, url_len, ":", 2ul);
      if (ii>0L) {
         url[ii] = 0;
         aprsstr_Delstr(s, s_len, 0UL, (uint32_t)(ii+1L));
         aprsstr_Assign(port0, port_len, s, s_len);
      }
   }
   X2C_PFREE(s);
} /* end urlport() */


static void SetComMode(int32_t fd0, struct termios * save)
{
   struct termios term;
   int32_t res;
   struct termios * anonym;
   res = tcgetattr(fd0, save);
   res = tcgetattr(fd0, &term);
   { /* with */
      struct termios * anonym = &term;
      anonym->c_lflag = 0UL;
      anonym->c_oflag = 0UL;
      anonym->c_iflag = 0UL;
   }
   /*  cfmakeraw(&termios);*/
   /*  c_cflag :=CS8+CLOCAL+CREAD; */
   res = tcsetattr(fd0, 2L, &term);
} /* end SetComMode() */


static void ReSetComMode(int32_t fd0, struct termios * save)
{
   int32_t res;
   res = tcsetattr(fd0, 2L, save);
} /* end ReSetComMode() */


static int32_t waitprompt(int32_t fd0)
{
   uint32_t st;
   char ch;
   st = 0UL;
   for (;;) {
      fdclr();
      fdsetr((uint32_t)fd0);
      if (selectr(0UL, 0UL)>=0L && issetr((uint32_t)fd0)) {
         if (osi_RdBin(fd0, (char *) &ch, 1u/1u, 1UL)!=1L) return -1L;
         if (st==0UL && ch==':') st = 1UL;
         else if (st==1UL && (ch=='\015' || ch=='\012')) break;
      }
   }
   return 0L;
} /* end waitprompt() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   osi_BEGIN();
   aprsstr_BEGIN();
   osi_NextArg(h, 1025ul);
   if (h[0U]=='-' && h[1U]=='h') {
      Err("usage: cmslogin <url> <usercall> <password>\012example: ./l2cat -i\
 OE0AAA-11 -U 127.0.0.1:2602:2603 -m 0 -c \"./cmslogin cmslogin server.winlin\
k.org:8772 %U CMSTelnet\"", 164ul);
   }
   urlport(h, 1025ul, ip, 1025ul, port, 1025ul);
   osi_NextArg(user, 1025ul);
   if (user[0U]==0) Err("need a user call", 17ul);
   osi_NextArg(pw, 1025ul);
   if (pw[0U]==0) Err("need a password", 16ul);
   fd = connectto(ip, port);
   if ((int32_t)fd<0L) Err("connection failed", 18ul);
   /*  l:=socknonblock(fd); */
   if (waitprompt(fd)<0L) Err("connection lost", 16ul);
   aprsstr_Append(user, 1025ul, "\015", 2ul);
   osi_WrBin(fd, (char *)user, 1025u/1u, aprsstr_Length(user, 1025ul));
   if (waitprompt(fd)<0L) Err("connection lost", 16ul);
   aprsstr_Append(pw, 1025ul, "\015", 2ul);
   osi_WrBin(fd, (char *)pw, 1025u/1u, aprsstr_Length(pw, 1025ul));
   SetComMode(0L, &saved0);
   for (;;) {
      fdclr();
      fdsetr((uint32_t)fd);
      fdsetr(0UL);
      if (selectr(0UL, 0UL)>=0L) {
         if (issetr((uint32_t)fd)) {
            l = osi_RdBin(fd, (char *)b, 1025u/1u, 1025UL);
            if (l<=0L) break;
            osi_WrBin(1L, (char *)b, 1025u/1u, (uint32_t)l);
         }
         if (issetr(0UL)) {
            l = osi_RdBin(0L, (char *)b, 1025u/1u, 1025UL);
            if (l<=0L) break;
            osi_WrBin(fd, (char *)b, 1025u/1u, (uint32_t)l);
         }
      }
   }
   ReSetComMode(0L, &saved0);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
