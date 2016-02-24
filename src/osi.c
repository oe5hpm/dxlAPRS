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
#ifndef cfileio_H_
#include "cfileio.h"
#endif

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


extern long osi_OpenAppendLong(char fn[], unsigned long fn_len)
{
   long osi_OpenAppendLong_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenAppendLong_ret = cOpenAppendLong(fn);
   X2C_PFREE(fn);
   return osi_OpenAppendLong_ret;
} /* end OpenAppendLong() */


extern long osi_OpenAppend(char fn[], unsigned long fn_len)
{
   long osi_OpenAppend_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenAppend_ret = cOpenAppend(fn);
   X2C_PFREE(fn);
   return osi_OpenAppend_ret;
} /* end OpenAppend() */


extern long osi_OpenWrite(char fn[], unsigned long fn_len)
{
   long osi_OpenWrite_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenWrite_ret = cOpenWrite(fn);
   X2C_PFREE(fn);
   return osi_OpenWrite_ret;
} /* end OpenWrite() */


extern long osi_OpenReadLong(char fn[], unsigned long fn_len)
{
   long osi_OpenReadLong_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenReadLong_ret = cOpenReadLong(fn);
   X2C_PFREE(fn);
   return osi_OpenReadLong_ret;
} /* end OpenReadLong() */


extern long osi_OpenRead(char fn[], unsigned long fn_len)
{
   long osi_OpenRead_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenRead_ret = cOpenRead(fn);
   X2C_PFREE(fn);
   return osi_OpenRead_ret;
} /* end OpenRead() */


extern long osi_OpenRW(char fn[], unsigned long fn_len)
{
   long osi_OpenRW_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenRW_ret = cOpenRW(fn);
   X2C_PFREE(fn);
   return osi_OpenRW_ret;
} /* end OpenRW() */


extern long osi_OpenNONBLOCK(char fn[], unsigned long fn_len)
{
   long osi_OpenNONBLOCK_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenNONBLOCK_ret = cOpenNONBLOCK(fn);
   X2C_PFREE(fn);
   return osi_OpenNONBLOCK_ret;
} /* end OpenNONBLOCK() */


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


extern void osi_Rename(char fname[], unsigned long fname_len, char newname[],
                 unsigned long newname_len)
{
   char ok0;
   X2C_PCOPY((void **)&fname,fname_len);
   X2C_PCOPY((void **)&newname,newname_len);
   FileSys_Rename(fname, fname_len, newname, newname_len, &ok0);
   X2C_PFREE(fname);
   X2C_PFREE(newname);
} /* end Rename() */

/*
PROCEDURE Size(fd:File):CARDINAL;
VAR st:stat.stat_t;
BEGIN
  stat.fstat(fd, st);
  RETURN st.st_size
END Size;
*/

int osi_Size(int fd)
{
  struct stat st;
  fstat(fd, &st);
  return st.st_size;
}

static int LSeek(int fd, long lo, int whence)
{
  if (lseek64(fd, lo, whence)<0) return -1;
  return 0;
}

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

