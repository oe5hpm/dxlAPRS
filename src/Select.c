/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h> 

static fd_set rset, wset;
static unsigned maxfd;

int selectrw(long sec, long usec)
{
  struct timeval tv;

  tv.tv_sec  = sec;
  tv.tv_usec = usec;
  return select(maxfd+1, &rset, &wset, NULL, &tv);
}

int selectr(long sec, long usec)
{
  struct timeval tv;

  tv.tv_sec  = sec;
  tv.tv_usec = usec;
  return select(maxfd+1, &rset, NULL, NULL, (usec == 0 && sec == 0) ? NULL : &tv);
}

void fdclr()
{
  FD_ZERO(&rset);
  FD_ZERO(&wset);
  maxfd = 0;
}

void fdsetr(long n)
{
  if (n>maxfd) maxfd = n; 
  FD_SET(n, &rset);
}

void fdsetw(long n)
{
  if (n>maxfd) maxfd = n;
  FD_SET(n, &wset);
}

unsigned issetr(long n)
{
  if FD_ISSET(n, &rset) return 1;
  return 0;
}

unsigned issetw(long n)
{
  if FD_ISSET(n, &wset) return 1;
  return 0;
}

int selectrwt(long *sec, long *usec)
{
  struct timeval tv;
  int ret;

  tv.tv_sec  = *sec;
  tv.tv_usec = *usec;
  ret=select(maxfd+1, &rset, &wset, NULL, &tv);
  *sec = tv.tv_sec;
  *usec= tv.tv_usec;
  return ret;
}

/*
void Usleep(long microseconds)
{
  usleep(microseconds);
}
*/
/*
unsigned Systime()
{
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);
  return tv.tv_sec;
}

unsigned monotonictime()
{
  struct timespec tv;
  clock_gettime(CLOCK_MONOTONIC, &tv);
  return tv.tv_sec;
}
*/
