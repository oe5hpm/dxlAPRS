/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <stdlib.h>

int getptsname(fd, name, len)
int fd;
char *name;
int len;
{
  return ptsname_r(fd, name, len);
}

int grantpts(fd)
int fd;
{
  return grantpt(fd);
}

int unlockpts(fd)
int fd;
{
  return unlockpt(fd);
}
