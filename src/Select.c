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
#include <stdint.h>

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
  if (FD_ISSET(n, &rset)) return 1;
  return 0;
}

unsigned issetw(long n)
{
  if (FD_ISSET(n, &wset)) return 1;
  return 0;
}

int selectrwt(uint32_t *sec, uint32_t *usec)
{
  struct timeval tv;
  int ret;
#ifdef MACOS
  struct timeval spec1, spec2;
  time_t s;
  long us;
  gettimeofday(&spec1, NULL);
#endif

  tv.tv_sec  = *sec;
  tv.tv_usec = *usec;
  ret=select(maxfd+1, &rset, &wset, NULL, &tv);
#ifdef MACOS
  /* MacOS does not update tv value in select */
  gettimeofday(&spec2, NULL);
  s = spec2.tv_sec-spec1.tv_sec;
  us = spec2.tv_usec - spec1.tv_usec;
  if (us < 0) {
          us += 1000000;
          s -= 1;
  }
  *sec -= s;
  *usec -= us;
  if (*usec < 0) {
          *usec += 1000000;
          *sec -= 1;
  }
  if (*sec < 0) {
          *sec = 0;
          *usec = 0;
  }
#else
  *sec = tv.tv_sec;
  *usec= tv.tv_usec;
#endif
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
