/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <linux/soundcard.h>
#include <sys/ioctl.h>

int samplesize(int fd, int bits)
{
return ioctl(fd, SOUND_PCM_WRITE_BITS, &bits);
}

int channels(int fd, int n)
{
return ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &n);
}

int sampelrate(int fd, int f)
{
return ioctl(fd, SOUND_PCM_WRITE_RATE, &f);
}

int setfragment(int fd, int f)
{
return ioctl(fd, SOUND_PCM_SETFRAGMENT, &f);
}

void pcmsync(int fd)
{
ioctl(fd, SOUND_PCM_SYNC, 0);
}

int getoutfilled(int fd)
{
int bytes;
ioctl(fd, SNDCTL_DSP_GETODELAY, &bytes);
return bytes;
}

int getsampelrate(int fd)
{
int f;
ioctl(fd, SOUND_PCM_READ_RATE, &f);
return f;
}

int getmixer(int fd, int device)
{
int level=-1;
ioctl(fd, MIXER_READ(device), &level);
return level;
}

int recnum()
{
return SOUND_MIXER_RECSRC;
}

void setmixer(int fd, int device, int level)
{
ioctl(fd, MIXER_WRITE(device), &level);
}

