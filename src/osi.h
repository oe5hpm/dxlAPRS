/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef osi_H_
#define osi_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef mlib_H_
#include "mlib.h"
#endif
#include <dirent.h>
#include <osic.h>

typedef int32_t osi_File;

typedef int32_t osi_SOCKET;

typedef DIR * osi_DIRCONTEXT;

#define osi_pi 3.1415926535898

#define osi_DIRSEP "/"

#define osi_DIRSEP2 "/"

#define osi_InvalidFd (-1)

#define osi_ln osic_ln

#define osi_sin osic_sin

#define osi_cos osic_cos

#define osi_tan osic_tan

#define osi_arctan osic_arctan

#define osi_arccos osic_arccos

#define osi_exp osic_exp

#define osi_power osic_power

#define osi_sqrt osic_sqrt

#define osi_floor osic_floor

#define osi_Close osic_Close

#define osi_WrFixed osic_WrFixed

#define osi_Seek osic_Seek

#define osi_Seekcur osic_Seekcur

#define osi_WrLn osic_WrLn

#define osi_FdValid osic_FdValid

#define osi_CloseSock osic_CloseSock

#define osi_ALLOCATE osic_alloc

#define osi_DEALLOCATE osic_free

#define osi_time osic_time

#define osi_Size osic_Size

#define osi_grantpts osic_grantpts

#define osi_unlockpts osic_unlockpts

#define osi_Random osic_Random

#define osi_WrCard osic_WrUINT32

#define osi_WrInt osic_WrINT32

#define osi_Flush osic_flush

#define osi_usleep usleep

#define osi_readsock readsock

#define osi_sendsock sendsock

#define osi_connectto connectto

#define osi_getunack getunack

#define osi_stoptxrx stoptxrx

#define osi_openudp openudp

#define osi_bindudp bindudp

#define osi_socknonblock socknonblock

#define osi_udpreceive udpreceive

#define osi_udpsend udpsend

extern int32_t osi_OpenAppendLong(char [], uint32_t);

extern int32_t osi_OpenAppend(char [], uint32_t);

extern int32_t osi_OpenWrite(char [], uint32_t);

extern int32_t osi_OpenReadLong(char [], uint32_t);

extern int32_t osi_OpenRead(char [], uint32_t);

extern int32_t osi_OpenRW(char [], uint32_t);

extern int32_t osi_OpenNONBLOCK(char [], uint32_t);

extern void osi_Rename(char [], uint32_t, char [], uint32_t);

extern void osi_WerrLn(char [], uint32_t);

extern int32_t osi_RdBin(int32_t, char [], uint32_t, uint32_t);

extern void osi_WrBin(int32_t, char [], uint32_t, uint32_t);

extern void osi_Werr(char [], uint32_t);

extern void osi_WrHex(uint32_t, uint32_t);

extern int32_t osi_OpenDir(char [], uint32_t, osi_DIRCONTEXT *);

extern void osi_ReadDirLine(char [], uint32_t, osi_DIRCONTEXT);

extern void osi_CloseDir(osi_DIRCONTEXT);

extern char osi_CreateDir(char [], uint32_t, uint32_t);

extern void osi_NextArg(char [], uint32_t);

extern void osi_WrStr(char [], uint32_t);

extern void osi_Erase(char [], uint32_t, char *);

extern char osi_Exists(char [], uint32_t);

extern int32_t osi_getptsname(int32_t, char *, uint32_t);

extern int32_t osi_symblink(char *, char *);

extern void osi_WrStrLn(char [], uint32_t);

extern int32_t osi_realint(float);

extern uint32_t osi_realcard(float);


extern void osi_BEGIN(void);


#endif /* osi_H_ */
