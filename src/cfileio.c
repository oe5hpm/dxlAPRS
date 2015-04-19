/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <sys/types.h>
#include <stdlib.h>
#include <asm/fcntl.h>


int __inline cOpenAppendLong(char *fn)
{
  return open(fn, O_WRONLY|O_APPEND|O_LARGEFILE);
}

int __inline cOpenAppend(char *fn)
{
  return open(fn, O_WRONLY|O_APPEND);
}

int __inline cOpenWrite(char *fn)
{
  return creat(fn, 0664);
}

int __inline cOpenReadLong(char *fn)
{
  return open(fn, O_RDONLY|O_LARGEFILE);
}

int __inline cOpenRead(char *fn)
{
  return open(fn, O_RDONLY);
}

int __inline cOpenRW(char *fn)
{
  return open(fn, O_RDWR);
}

int __inline cOpenNONBLOCK(char *fn)
{
  return open(fn, O_RDWR|O_NONBLOCK);
}
