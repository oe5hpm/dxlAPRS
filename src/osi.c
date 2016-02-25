/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include "Lib.h"
#include "osi.h"

#define DEBUG
#ifdef DEBUG
# define DBG(...)	printf(__VA_ARGS__)
#else
# define DBG(...)	;
#endif

#define FNLENCHECK	1

void osi_WrLn(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);
}

void osi_WrStr(char s[], unsigned long s_len)
{
	if (FNLENCHECK && strnlen(s, s_len) >= s_len)
		return;
	fprintf(stdout, "%s", s);
	osi_WrLn();
}

void osi_WrStrLn(char s[], unsigned long s_len)
{
	if (FNLENCHECK && strnlen(s, s_len) >= s_len)
		return;
	fprintf(stdout, "%s\n", s);
	osi_WrLn();
}

void osi_WrUINT32(uint32_t x, unsigned long witdh)
{
	fprintf(stdout, "%*d", (int)witdh, x);
}

void osi_WrFixed(float x, long place, unsigned long witdh)
{
	fprintf(stdout, "%*.*f", (int)witdh, (int)place, x);
}

static void h(unsigned long n)
{
	char tmp;
	if (n < 10)
		osi_WrStr((char *)(tmp = (char)(n+48UL),&tmp), 1);
	else
		osi_WrStr((char *)(tmp = (char)((n-10UL)+65UL),&tmp), 1);
}

void osi_WrHex(unsigned long n, unsigned long f)
{
	h(n / 0x10);
	h(n & 0x0F);

	while (f>=3UL) {
		osi_WrStr(" ", 2ul);
		--f;
	}
}

extern long osi_OpenAppendLong(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return open(fn, O_WRONLY | O_APPEND | O_LARGEFILE);
}


extern long osi_OpenAppend(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return open(fn, O_WRONLY | O_APPEND);
}

extern long osi_OpenWrite(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return creat(fn, 0664);
}

extern long osi_OpenReadLong(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return open(fn, O_RDONLY | O_LARGEFILE);
}

extern long osi_OpenRead(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return open(fn, O_RDONLY);
}

extern long osi_OpenRW(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return open(fn, O_RDWR);
}
extern long osi_OpenNONBLOCK(char fn[], unsigned long fn_len)
{
	if (FNLENCHECK && strnlen(fn, fn_len) >= fn_len)
		return -1;
	return open(fn, O_RDWR | O_NONBLOCK);
}
extern char osi_FdValid(long fd)
{
	return (fd >= 0L);
}

extern void osi_Close(long fd)
{
	close(fd);
}

extern void osi_CloseSock(long fd)
{
	close(fd);
}

extern long osi_RdBin(long fd,
		      char buf[], unsigned long buf_len,
		      unsigned long size)
{
	if (size > (buf_len-1) + 1UL)
		size = (buf_len-1)+1UL;
	return read(fd, (char *)buf, size);
}

extern void osi_WrBin(long fd, char buf[],
		      unsigned long buf_len,
		      unsigned long size)
{
	if (size > (buf_len-1)+1UL)
		size = (buf_len-1)+1UL;
	write(fd, (char *)buf, size);
}

extern void osi_Rename(char fname[], unsigned long fname_len,
		       char newname[], unsigned long newname_len)
{
	if (FNLENCHECK && strnlen(fname, fname_len) >= fname_len)
		return;
	if (FNLENCHECK && strnlen(newname, newname_len) >= newname_len)
		return;
	rename(fname, newname);
}

int osi_Size(int fd)
{
	struct stat st = { };
	fstat(fd, &st);
	return st.st_size;
}

extern void osi_Seek(long fd, unsigned long pos)
{
	lseek(fd, (long)pos, SEEK_SET);
}


extern void osi_Seekcur(long fd, long rel)
{
	if (lseek64(fd, rel, (unsigned long)SEEK_CUR) < 0L)
		lseek(fd, 0L, SEEK_SET);
}

extern void osi_BEGIN(void)
{
	static int osi_init = 0;
	if (osi_init)
		return;
	osi_init = 1;

	StdChans_BEGIN();
	Lib_BEGIN();
}

