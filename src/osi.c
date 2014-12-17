/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef osi_H_
#include "osi.h"
#endif
#define osi_C_
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#include <math.h>
#ifndef flush_H_
#include "flush.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef FileSys_H_
#include "FileSys.h"
#endif
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#include <stdio.h>
#ifndef filesize_H_
#include "filesize.h"
#endif
#ifndef StdChans_H_
#include "StdChans.h"
#endif
#ifndef IOChan_H_
#include "IOChan.h"
#endif
#include <fcntl.h>
#ifndef RealIO_H_
#include "RealIO.h"
#endif
#include <unistd.h>

/* os interface linux/win32 */
/*FROM Storage IMPORT ALLOCATE, DEALLOCATE; */
/*FROM Select IMPORT fdsetr, fdsetw; */
/*FROM aprsstr IMPORT IntToStr, Append, Length; */
static IOChan_ChanId cid;


extern void osi_WrLn(void)
{
   InOut_WriteLn();
   Flush();
} /* end WrLn() */


extern void osi_WrStrLn(char s[], unsigned long s_len)
{
   X2C_PCOPY((void **)&s,s_len);
   InOut_WriteString(s, s_len);
   osi_WrLn();
   X2C_PFREE(s);
} /* end WrStrLn() */


extern void osi_WrFixed(float x, long place, unsigned long witdh)
{
   RealIO_WriteFixed(cid, x, place, witdh);
} /* end WrFixed() */


static void h(unsigned long n)
{
   char tmp;
   if (n<10UL) InOut_WriteString((char *)(tmp = (char)(n+48UL),&tmp), 1u/1u);
   else InOut_WriteString((char *)(tmp = (char)((n-10UL)+65UL),&tmp), 1u/1u);
} /* end h() */


extern void osi_WrHex(unsigned long n, unsigned long f)
{
   h(n/16UL&15UL);
   h(n&15UL);
   while (f>=3UL) {
      InOut_WriteString(" ", 2ul);
      --f;
   }
} /* end WrHex() */

/*
PROCEDURE OpenMode(name:ARRAY OF CHAR; mode:INTEGER):File;
BEGIN
  IF (name[0] = '-') AND ((HIGH(name) = 0) OR (name[1] = CHR(0)))
                THEN RETURN 0 END;
  RETURN open(name, mode)
END OpenMode;

--PROCEDURE Create(name:ARRAY OF CHAR):File;
--BEGIN RETURN creat(name, 644B) END Create;

PROCEDURE OpenAppend(fn:ARRAY OF CHAR):File;
BEGIN RETURN OpenMode(fn, oWRONLY+oAPPEND+oLARGEFILE) END OpenAppend;

PROCEDURE OpenWrite(fn:ARRAY OF CHAR):File;
BEGIN RETURN creat(fn, 644B) END OpenWrite;

PROCEDURE OpenRead(fn:ARRAY OF CHAR):File;
BEGIN RETURN OpenMode(fn, oRDONLY+oLARGEFILE) END OpenRead;

PROCEDURE OpenRW(fn:ARRAY OF CHAR):File;
BEGIN RETURN OpenMode(fn, oRDWR) END OpenRW;
*/

extern long osi_OpenAppend(char fn[], unsigned long fn_len)
{
   long osi_OpenAppend_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenAppend_ret = open(fn, 33793L);
   X2C_PFREE(fn);
   return osi_OpenAppend_ret;
} /* end OpenAppend() */


extern long osi_OpenWrite(char fn[], unsigned long fn_len)
{
   long osi_OpenWrite_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenWrite_ret = creat(fn, 420L);
   X2C_PFREE(fn);
   return osi_OpenWrite_ret;
} /* end OpenWrite() */


extern long osi_OpenRead(char fn[], unsigned long fn_len)
{
   long osi_OpenRead_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenRead_ret = open(fn, 32768L);
   X2C_PFREE(fn);
   return osi_OpenRead_ret;
} /* end OpenRead() */


extern long osi_OpenRW(char fn[], unsigned long fn_len)
{
   long osi_OpenRW_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenRW_ret = open(fn, 2L);
   X2C_PFREE(fn);
   return osi_OpenRW_ret;
} /* end OpenRW() */


extern char osi_FdValid(long fd)
{
   return fd>=0L;
} /* end FdValid() */


extern void osi_Close(long fd)
{
   long res;
   res = close(fd);
} /* end Close() */


extern void osi_CloseSock(long fd)
{
   long res;
   res = close(fd);
} /* end CloseSock() */


extern long osi_RdBin(long fd, char buf[], unsigned long buf_len,
                unsigned long size)
{
   if (size>(buf_len-1)+1UL) size = (buf_len-1)+1UL;
   return read(fd, (char *)buf, size);
} /* end RdBin() */


extern void osi_WrBin(long fd, char buf[], unsigned long buf_len,
                unsigned long size)
{
   long res;
   if (size>(buf_len-1)+1UL) size = (buf_len-1)+1UL;
   res = write(fd, (char *)buf, size);
} /* end WrBin() */

/*
PROCEDURE Size(fd:File):CARDINAL;
VAR st:stat.stat_t;
BEGIN
  stat.fstat(fd, st);
  RETURN st.st_size
END Size;
*/

extern void osi_Seek(long fd, unsigned long pos)
{
   long res;
   res = lseek(fd, (long)pos, SEEK_SET);
} /* end Seek() */


extern void osi_Seekcur(long fd, long rel)
{
   if (LSeek(fd, rel, (unsigned long)SEEK_CUR)<0L) lseek(fd, 0L, SEEK_SET);
} /* end Seekcur() */


extern void osi_BEGIN(void)
{
   static int osi_init = 0;
   if (osi_init) return;
   osi_init = 1;
   RealIO_BEGIN();
   IOChan_BEGIN();
   StdChans_BEGIN();
   InOut_BEGIN();
   FileSys_BEGIN();
   Lib_BEGIN();
   RealMath_BEGIN();
   cid = StdChans_StdOutChan();
}

