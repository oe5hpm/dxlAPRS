/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <sys/ioctl.h>
#ifndef MACOS
#include <linux/kd.h>
#endif
#include <fcntl.h>
#include <unistd.h>

int cbell(int hz, int ms)
{
int fd;
int ret;

  ret=-2;
  fd=open("/dev/tty0", O_WRONLY);
  if (fd<0) fd=open("/dev/vc/0", O_WRONLY);
  if (fd<0) fd=open("/dev/console", O_WRONLY);
  if (fd<0) return -1;

#ifndef MACOS
  /* Not supported on MacOS */
  if (ioctl(fd, KIOCSOUND, 1193180/hz)>=0)
    {
     usleep(ms*1000);
     ioctl(fd, KIOCSOUND, 0);
     ret=0;
    }
#endif
  close(fd);
  return ret;
}


