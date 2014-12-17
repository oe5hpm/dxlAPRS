/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosFiles.c Feb  3 14:30:28 2012" */
#include "xosFiles.h"
#define xosFiles_C_
#include "xlibOS.h"
#include "ChanConsts.h"
#include "xrInt64.h"
#include "xPOSIX.h"

X2C_CARD32 X2C_fAccessRead = 0x1UL;
X2C_CARD32 X2C_fAccessWrite = 0x2UL;
X2C_CARD32 X2C_fModeNew = 0x4UL;
X2C_CARD32 X2C_fModeText = 0x8UL;
X2C_CARD32 X2C_fModeRaw = 0x10UL;
typedef FILE * File;


extern X2C_BOOLEAN X2C_IsMixAllowed(void)
{
   #ifdef _unix
   return (X2C_BOOLEAN)1;
   #endif
   return 0;
} /* end X2C_IsMixAllowed() */


extern void X2C_fGetXAttrs(X2C_FXATTRS data)
{
} /* end X2C_fGetXAttrs() */


extern void X2C_fSetXAttrs(X2C_FXATTRS data)
{
} /* end X2C_fSetXAttrs() */

static X2C_CARD32 mix = 0x18UL;


extern X2C_CARD8 X2C_fOpen(X2C_OSFHANDLE * f, X2C_CHAR name[],
                X2C_CARD32 tags)
{
   X2C_CHAR mode[4];
   X2C_CARD32 c;
   File cf;
   if ((tags&0x18UL)==0x18UL && !X2C_IsMixAllowed()) {
      return ChanConsts_noMixedOperations;
   }
   c = (X2C_CARD32)((tags&0x8UL)!=0UL)+(X2C_CARD32)((tags&0x1UL)!=0UL)
                *2UL+(X2C_CARD32)((tags&0x2UL)!=0UL)*4UL+(X2C_CARD32)
                ((tags&0x4UL)!=0UL)*8UL;
   switch (c) {
   case 0UL:
   case 2UL:
      strncpy(mode,"rb",4u);
      break;
   case 1UL:
   case 3UL:
      strncpy(mode,"r",4u);
      break;
   case 4UL:
   case 6UL:
      strncpy(mode,"r+b",4u);
      break;
   case 5UL:
   case 7UL:
      strncpy(mode,"r+",4u);
      break;
   case 8UL:
   case 12UL:
      strncpy(mode,"wb",4u);
      break;
   case 9UL:
   case 13UL:
      strncpy(mode,"w",4u);
      break;
   case 10UL:
   case 14UL:
      strncpy(mode,"w+b",4u);
      break;
   case 11UL:
   case 15UL:
      strncpy(mode,"w+",4u);
      break;
   default:
      X2C_TRAP(X2C_CASE_TRAP);
   } /* end switch */
   cf = (File)fopen(name, mode);
   if (cf==0) {
      #ifdef ENOENT
      if (errno==ENOENT) return ChanConsts_noSuchFile;
      #endif
      #ifdef ENOENT
      if (errno==ENOENT) return ChanConsts_noSuchFile;
      #endif
      #ifdef EMFILE
      if (errno==EMFILE) return ChanConsts_tooManyOpen;
      #endif
      #ifdef EACCES
      if (errno==EACCES) return ChanConsts_wrongPermissions;
      #endif
      #ifdef ENOSPC
      if (errno==ENOSPC) return ChanConsts_noRoomOnDevice;
      #endif
      #ifdef EISDIR
      if (errno==EISDIR) return ChanConsts_wrongFileType;
      #endif
      return ChanConsts_otherProblem;
   }
   *f = (X2C_OSFHANDLE)cf;
   return ChanConsts_opened;
} /* end X2C_fOpen() */


extern int X2C_fClose(X2C_OSFHANDLE * f)
{
   File cf;
   cf = (File)*f;
   return fclose(cf);
} /* end X2C_fClose() */


extern int X2C_fRead(X2C_OSFHANDLE f, X2C_ADDRESS buf, X2C_CARD32 size,
                X2C_CARD32 * rd)
{
   File cf;
   cf = (File)f;
   *rd = fread(buf, 1U, size, cf);
   if (*rd<size && feof(cf)==0) return errno;
   return 0;
} /* end X2C_fRead() */


extern int X2C_fWrite(X2C_OSFHANDLE f, X2C_ADDRESS buf, X2C_CARD32 size,
                X2C_CARD32 * wr)
{
   File cf;
   cf = (File)f;
   *wr = fwrite(buf, 1U, size, cf);
   if (*wr<size) return errno;
   return 0;
} /* end X2C_fWrite() */


extern int X2C_fSeek(X2C_OSFHANDLE f, struct X2C_int64 * ofs, int how)
{
   X2C_INT32 pos;
   File cf;
   cf = (File)f;
   if (X2C_64TOINT(&pos, *ofs)) return 1;
   if (fseek(cf, pos, how)) return errno;
   pos = ftell(cf);
   if (pos==-1L) return errno;
   X2C_INTTO64(ofs, pos);
   return 0;
} /* end X2C_fSeek() */


extern int X2C_fTell(X2C_OSFHANDLE f, struct X2C_int64 * ofs)
{
   X2C_INT32 pos;
   File cf;
   cf = (File)f;
   pos = ftell(cf);
   if (pos==-1L) return errno;
   X2C_INTTO64(ofs, pos);
   return 0;
} /* end X2C_fTell() */


extern int X2C_fSize(X2C_OSFHANDLE f, struct X2C_int64 * siz)
{
   X2C_INT32 eof;
   X2C_INT32 cp;
   File cf;
   cf = (File)f;
   cp = ftell(cf);
   if (cp==-1L) return errno;
   if (fseek(cf, 0L, SEEK_END)) return errno;
   eof = ftell(cf);
   if (eof==-1L) return errno;
   if (fseek(cf, cp, SEEK_SET)) return errno;
   X2C_INTTO64(siz, eof);
   return 0;
} /* end X2C_fSize() */


extern int X2C_fFlush(X2C_OSFHANDLE f)
{
   File cf;
   cf = (File)f;
   if (fflush(cf)==0) return 0;
   return errno;
} /* end X2C_fFlush() */

extern int X2C_ChSize(File, X2C_INT32);


extern int X2C_fChSize(X2C_OSFHANDLE f)
{
   File cf;
   X2C_INT32 pos;
   cf = (File)f;
   if (fflush(cf)) return errno;
   pos = ftell(cf);
   if (pos==-1L) return errno;
   if (X2C_ChSize(cf, pos)==0) return 0;
   return errno;
} /* end X2C_fChSize() */


extern int X2C_fGetStd(X2C_OSFHANDLE * f, int what)
{
   switch (what) {
   case 0:
      *f = (X2C_OSFHANDLE)stdin;
      break;
   case 1:
      *f = (X2C_OSFHANDLE)stdout;
      break;
   default:;
      *f = (X2C_OSFHANDLE)stderr;
      break;
   } /* end switch */
   return 0;
} /* end X2C_fGetStd() */


static int X2C_dup2(FILE * f1, X2C_ADDRESS * f2)
{
   #if defined(_unix) || defined(_msdos)
     return dup2(fileno(f1),fileno(*(FILE**)f2));
   #else
   (*f2)=(X2C_ADDRESS)f1;
   return 0;
   #endif
   X2C_TRAP(X2C_RETURN_TRAP);
   return 0;
} /* end X2C_dup2() */


extern int X2C_fSetStd(X2C_OSFHANDLE new, int which)
{
   File cf;
   File yf;
   cf = (File)new;
   switch (which) {
   case 0:
      yf = (File)stdin;
      break;
   case 1:
      yf = (File)stdout;
      break;
   default:;
      yf = (File)stderr;
      break;
   } /* end switch */
   return X2C_dup2(cf, (X2C_ADDRESS *) &yf);
} /* end X2C_fSetStd() */

extern int X2C_GetFileType(X2C_OSFHANDLE);


extern int X2C_fGetFileType(X2C_OSFHANDLE f)
{
   return X2C_GetFileType(f);
} /* end X2C_fGetFileType() */

