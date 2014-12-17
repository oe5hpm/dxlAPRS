/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosFileIO.c Feb  3 14:30:28 2012" */
#include "xosFileIO.h"
#define xosFileIO_C_
#include "xrtsOS.h"
#include "xPOSIX.h"

typedef FILE * File;


extern int X2C_FileOpenRead(X2C_OSFILE * f, X2C_CHAR name[])
{
   File t;
   t = (File)fopen(name, "r");
   if (t) {
      *f = (X2C_OSFILE)t;
      return 1;
   }
   else return 0;
   return 0;
} /* end X2C_FileOpenRead() */


extern int X2C_FileClose(X2C_OSFILE f)
{
   File cf;
   cf = (File)f;
   if (fclose(cf)==0) return 1;
   else return 0;
   return 0;
} /* end X2C_FileClose() */


extern int X2C_FileSeek(X2C_OSFILE f, X2C_WORD ofs, int org)
{
   File cf;
   cf = (File)f;
   if (fseek(cf, *(X2C_INT32 *)ofs, org)==0) return 1;
   else return 0;
   return 0;
} /* end X2C_FileSeek() */


extern int X2C_FileOpenWrite(X2C_OSFILE * f, X2C_CHAR name[])
{
   File t;
   t = (File)fopen(name, "rw");
   if (t) {
      *f = (X2C_OSFILE)t;
      return 1;
   }
   else return 0;
   return 0;
} /* end X2C_FileOpenWrite() */


extern int X2C_FileOpenRW(X2C_OSFILE * f, X2C_CHAR name[])
{
   File t;
   t = (File)fopen(name, "r+");
   if (t) {
      *f = (X2C_OSFILE)t;
      return 1;
   }
   else return 0;
   return 0;
} /* end X2C_FileOpenRW() */


extern int X2C_FileRead(X2C_OSFILE f, X2C_ADDRESS buf, X2C_CARD32 * len)
{
   File cf;
   cf = (File)f;
   if (fread(buf, *len, 1U, cf)==*len) return 1;
   else return 0;
   return 0;
} /* end X2C_FileRead() */


extern int X2C_FileWrite(X2C_OSFILE f, X2C_ADDRESS buf, X2C_CARD32 * len)
{
   File cf;
   cf = (File)f;
   if (fwrite(buf, *len, 1U, cf)==*len) return 1;
   else return 0;
   return 0;
} /* end X2C_FileWrite() */


extern void X2C_StdOut(X2C_CHAR s[], X2C_CARD32 len)
{
   fwrite((X2C_ADDRESS)s, 1U, len, stdout);
} /* end X2C_StdOut() */


extern void X2C_StdOutFlush(void)
{
} /* end X2C_StdOutFlush() */

