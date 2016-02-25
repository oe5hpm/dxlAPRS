/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __OSI_H__
#define __OSI_H__

#include <stdint.h>

void osi_WrLn(void);

void osi_WrStr(char s[], unsigned long s_len);

void osi_WrStrLn(char s[], unsigned long s_len);

void osi_WrUINT32(uint32_t x, unsigned long witdh);

void osi_WrFixed(float x, long place, unsigned long witdh);

void osi_WrHex(unsigned long n, unsigned long f);

extern long osi_OpenAppendLong(char fn[], unsigned long fn_len);

extern long osi_OpenAppend(char fn[], unsigned long fn_len);

extern long osi_OpenWrite(char fn[], unsigned long fn_len);

extern long osi_OpenReadLong(char fn[], unsigned long fn_len);

extern long osi_OpenRead(char fn[], unsigned long fn_len);

extern long osi_OpenRW(char fn[], unsigned long fn_len);

extern long osi_OpenNONBLOCK(char fn[], unsigned long fn_len);

extern char osi_FdValid(long fd);

extern void osi_Close(long fd);

extern void osi_CloseSock(long fd);

extern long osi_RdBin(long fd,
		      char buf[], unsigned long buf_len,
		      unsigned long size);

extern void osi_WrBin(long fd, char buf[],
		      unsigned long buf_len,
		      unsigned long size);

extern void osi_Rename(char fname[], unsigned long fname_len,
		       char newname[], unsigned long newname_len);

int osi_Size(int fd);

extern void osi_Seek(long fd, unsigned long pos);

extern void osi_Seekcur(long fd, long rel);

extern void osi_BEGIN(void);

#endif /* __OSI_H__ */
