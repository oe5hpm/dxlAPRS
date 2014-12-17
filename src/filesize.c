/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define _LARGEFILE64_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int Size(int fd)
{
  struct stat st;
  fstat(fd, &st);
  return st.st_size;
}

int LSeek(int fd, long lo, int whence)
{
  if (lseek64(fd, lo, whence)<0) return -1;
  return 0;
}
