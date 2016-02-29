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
#include <math.h>
#ifndef flush_H_
#include "flush.h"
#endif
#include <osic.h>
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
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif

/* os interface linux/win32 */

extern long osi_OpenAppendLong(char fn[], unsigned long fn_len)
{
   long osi_OpenAppendLong_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenAppendLong_ret = osic_OpenAppendLong(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenAppendLong_ret;
} /* end OpenAppendLong() */


extern long osi_OpenAppend(char fn[], unsigned long fn_len)
{
   long osi_OpenAppend_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenAppend_ret = osic_OpenAppend(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenAppend_ret;
} /* end OpenAppend() */


extern long osi_OpenWrite(char fn[], unsigned long fn_len)
{
   long osi_OpenWrite_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenWrite_ret = osic_OpenWrite(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenWrite_ret;
} /* end OpenWrite() */


extern long osi_OpenReadLong(char fn[], unsigned long fn_len)
{
   long osi_OpenReadLong_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenReadLong_ret = osic_OpenReadLong(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenReadLong_ret;
} /* end OpenReadLong() */


extern long osi_OpenRead(char fn[], unsigned long fn_len)
{
   long osi_OpenRead_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenRead_ret = osic_OpenRead(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenRead_ret;
} /* end OpenRead() */


extern long osi_OpenRW(char fn[], unsigned long fn_len)
{
   long osi_OpenRW_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenRW_ret = osic_OpenRW(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenRW_ret;
} /* end OpenRW() */


extern long osi_OpenNONBLOCK(char fn[], unsigned long fn_len)
{
   long osi_OpenNONBLOCK_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   osi_OpenNONBLOCK_ret = osic_OpenNONBLOCK(fn, fn_len);
   X2C_PFREE(fn);
   return osi_OpenNONBLOCK_ret;
} /* end OpenNONBLOCK() */


extern void osi_NextArg(char s[], unsigned long s_len)
{
   osic_NextArg(s, s_len);
} /* end NextArg() */


extern long osi_RdBin(long fd, char buf[], unsigned long buf_len,
                unsigned long size)
{
   return osic_RdBin(fd, buf, buf_len, size);
} /* end RdBin() */


extern void osi_WrBin(long fd, char buf[], unsigned long buf_len,
                unsigned long size)
{
   osic_WrBin(fd, buf, buf_len, size);
} /* end WrBin() */


extern void osi_Rename(char fname[], unsigned long fname_len, char newname[],
                 unsigned long newname_len)
{
   X2C_PCOPY((void **)&fname,fname_len);
   X2C_PCOPY((void **)&newname,newname_len);
   osic_Rename(fname, fname_len, newname, newname_len);
   X2C_PFREE(fname);
   X2C_PFREE(newname);
} /* end Rename() */


extern void osi_Werr(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osic_WerrStr(text, text_len);
   X2C_PFREE(text);
} /* end Werr() */


extern void osi_WerrLn(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osic_WerrStrLn(text, text_len);
   X2C_PFREE(text);
} /* end WerrLn() */


extern void osi_WrStrLn(char s[], unsigned long s_len)
{
   X2C_PCOPY((void **)&s,s_len);
   osic_WrStrLn(s, s_len);
   X2C_PFREE(s);
} /* end WrStrLn() */


extern void osi_BEGIN(void)
{
   static int osi_init = 0;
   if (osi_init) return;
   osi_init = 1;
   InOut_BEGIN();
   FileSys_BEGIN();
   Lib_BEGIN();
   RealMath_BEGIN();
}

