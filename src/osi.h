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
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef flush_H_
#include "flush.h"
#endif
#include <stdio.h>
#ifndef FileSys_H_
#include "FileSys.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef filesize_H_
#include "filesize.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#include <math.h>

/* os interface */
typedef long osi_File;

typedef long osi_SOCKET;

/*
CONST sin=   math.sin;
      cos=   math.cos;
      arctan=math.atan;
      arccos=math.acos;
      tan=   math.tan;
      sqrt=  math.sqrt;
      exp=   math.exp;
      ln=    math.log;
      power= math.pow;
      floor= math.floor;
*/
#define osi_sin RealMath_sin

#define osi_cos RealMath_cos

#define osi_arctan RealMath_arctan

#define osi_arccos RealMath_arccos

#define osi_tan RealMath_tan

#define osi_sqrt RealMath_sqrt

#define osi_exp RealMath_exp

#define osi_ln RealMath_ln

#define osi_power RealMath_power

#define osi_floor floor

/*
<* IF __GEN_C__ THEN *>
VAR
      O_TRUNC     -: INTEGER;    (* open with truncation *)
      O_APPEND    -: INTEGER;    (* append, i.e writes at the end *)
      O_NONBLOCK  -: INTEGER;    (* open and accesses never block *)
      O_RDWR      -: INTEGER;    (* open for reading and writing *)
      O_WRONLY    -: INTEGER;    (* open for writing only *)
      O_RDONLY    -: INTEGER;    (* open for reading only *)
      O_CREAT     -: INTEGER;    (* create if not exists *)
      O_LARGEFILE -: INTEGER;
<* ELSE *>
<* END *>
*/
#define osi_pi 3.1415926535898

#define osi_DIRSEP "/"

#define osi_DIRSEP2 "/"

#define osi_InvalidFd (-1)

#define osi_Flush Flush

#define osi_NextArg Lib_NextArg

#define osi_WrStr InOut_WriteString

#define osi_WrCard InOut_WriteCard

#define osi_WrInt InOut_WriteInt

#define osi_Exists FileSys_Exists

#define osi_Erase FileSys_Remove

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

#define osi_remove remove

#define osi_Size Size

extern long osi_OpenAppendLong(char [], unsigned long);

extern long osi_OpenAppend(char [], unsigned long);

extern long osi_OpenWrite(char [], unsigned long);

extern long osi_OpenReadLong(char [], unsigned long);

extern long osi_OpenRead(char [], unsigned long);

extern long osi_OpenRW(char [], unsigned long);

extern long osi_OpenNONBLOCK(char [], unsigned long);

extern void osi_Close(long);

extern long osi_RdBin(long, char [], unsigned long, unsigned long);

extern void osi_WrBin(long, char [], unsigned long, unsigned long);

extern void osi_WrFixed(float, long, unsigned long);

extern void osi_Seek(long, unsigned long);

extern void osi_Seekcur(long, long);

extern void osi_WrLn(void);

extern void osi_WrStrLn(char [], unsigned long);

extern char osi_FdValid(long);
/*PROCEDURE Create(fn:ARRAY OF CHAR):File;*/

extern void osi_CloseSock(long);

extern void osi_WrHex(unsigned long, unsigned long);


extern void osi_BEGIN(void);


#endif /* osi_H_ */
