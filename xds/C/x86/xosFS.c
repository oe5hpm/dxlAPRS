/* Copyright (c) 1991,96 xTech Ltd, Russia. All Rights Reserved. */

#include "X2C.h"
#include "x2cLib.h"
#include "xlibOS.h"

#include <time.h>
#include <stdio.h>
#include <errno.h>

#if defined __WATCOMC__
/* Watcom */
#include <direct.h>
#elif defined _MSC_VER
/* MSVC */
#include <direct.h>
#elif defined __IBMC__
/* IBM CSet++ or Visual Age C++ */
#include <direct.h>
#else
#endif /* __WATCOMC__ */

#if defined(__OS2__) && defined (__ZTC__)
#include <dos.h>
#else
#if defined(__OS2__) && defined (__BORLANDC__)
#include <dir.h>
#else
#if defined(__TSC__)
#include <stat.h>
#else
#if defined(X2C_mac_os)
#define __useAppleExts__
#include <Files.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif
#endif
#endif

#if defined(_unix)
#include <unistd.h>
#else
#if defined(_msdos)
#include <io.h>
#endif
#endif

#ifndef X2C_KRC
X2C_BOOLEAN X2C_Exists(X2C_CHAR *name)
#else
X2C_BOOLEAN X2C_Exists(name) X2C_CHAR *name;
#endif
{
#if defined(__OS2__) && defined (__ZTC__)
  return (findfirst(name, 0) != 0);
#else
#if defined(__OS2__) && defined (__BORLANDC__)
    struct ffblk f;
    return (findfirst(name,&f,0) == 0);
#else
#if defined(X2C_mac_os)
    Str255 	FileName;
	int 	len;
	FInfo 	fpb;

	/*convert the filename to length-preffixed form*/
	len = 0;
	while (len<sizeof(FileName) && name[len]) {
		FileName[len+1] = name[len];
		len++;
	}
	FileName[0]=(char)len;

	return (GetFInfo(FileName,0,&fpb) == 0);
#else
      struct stat buf;
      return (stat (name, &buf) == 0);
#endif
#endif
#endif
} /* END Exists */

#ifndef X2C_KRC
void X2C_ModifyTime(
		X2C_CHAR    *name,
		X2C_CARD32  *time,
		X2C_BOOLEAN *exist)
#else
void X2C_ModifyTime(name, time, exist)
	X2C_CHAR    *name;
	X2C_CARD32  *time;
	X2C_BOOLEAN *exist;
#endif
{
#if defined(__OS2__) && defined (__ZTC__)
  struct FIND *buf;
  *exist = 0;
  if ((buf = findfirst (name, 0)) != 0) {
    *exist = 1;
    *time  = ((((long)buf->date)<<16) + buf->time);
  }
#else
#if defined(__OS2__) && defined (__BORLANDC__)
  struct ffblk f;
  *exist = 0;
  if (findfirst(name,&f,0) == 0) {
    *exist = 1;
    *time  = ((((long)f.ff_fdate)<<16) + f.ff_ftime);
  }
#else
#if defined(X2C_mac_os)
    Str255 		FileName;
	int 		len;
	FileParam 	fpb;

	/*convert the filename to length-preffixed form*/
	len = 0;
	while (len<sizeof(FileName) && name[len]) {
		FileName[len+1] = name[len];
		len++;
	}
	FileName[0]=(char)len;

	fpb.ioNamePtr = FileName;	/*filename to search*/
	fpb.ioFDirIndex = 0;		/*find a single file*/
	fpb.ioVRefNum = 0;			/*use the current default volume*/
	*exist = (PBGetFInfo((ParmBlkPtr)&fpb,0)==0);
	if (*exist) *time = fpb.ioFlMdDat;
#else
    struct stat buf;
    *exist = 0;
    if (stat(name, &buf) == 0) {
      *exist = 1;
      *time  = buf.st_mtime;
    }
#endif
#endif
#endif
} /* END ModifyTime */



#ifndef X2C_KRC
extern int X2C_Remove(X2C_pCHAR name)
#else
extern int X2C_Remove(name) X2C_pCHAR name;
#endif
{
        if (remove(name)) return errno;
        return 0;
} /* END X2C_Remove */

#ifndef X2C_KRC
extern int X2C_Rename(X2C_pCHAR from, X2C_pCHAR to)
#else
extern int X2C_Rename(from,to) X2C_pCHAR from; X2C_pCHAR to;
#endif
{
        if (rename(from,to)) return errno;
        return 0;
} /* END X2C_Rename */

#ifndef X2C_KRC
extern void X2C_FullName(X2C_pCHAR full, X2C_CARD32 len, X2C_pCHAR name)
#else
extern void X2C_FullName(full,len,name) X2C_pCHAR full; X2C_CARD32 len; X2C_pCHAR name;
#endif
{
	char *s, *d;
#if defined _unix
	if (name[0] == '/') {
		d = full;
	}
	else {
		X2C_GetCDName(full,len);
		d = strchr(full,0);
		*d++ = '/';
	}
#elif defined _msdos
	int l = strlen(name);
	if ((l>=3) && 
           ((name[1] == ':') && (name[2] == '\\')) || 
	   ((name[0] == '\\')  && (name[1] == '\\'))) {
		d = full;
	}
	else if (name[0] == '\\') {
		X2C_GetCDName(full,len);
		d = full+2;
	}
	else {
		X2C_GetCDName(full,len);
		d = strchr(full,0);
		*d++ = '\\';
	}
#else
	full[0] = 0; return;
#endif	
	for (s = name; *s;) {
		if ((d-full) >= len) {
			full[0] = 0;
			return;
		}
		*d++=*s++;
	}
	if (d-full < len) *d = 0;
} /* END X2C_FullName */

#ifndef X2C_KRC
extern X2C_INT32 X2C_GetDrive(char *drive)
#else
extern X2C_INT32 X2C_GetDrive(drive) char *drive;
#endif
{
#if defined __IBMC__
	*drive = 'A'-1+_getdrive(); return 0;
#else
	return ENOSYS;
#endif
}

#ifndef X2C_KRC
extern X2C_INT32 X2C_SetDrive(char drive)
#else
extern X2C_INT32 X2C_SetDrive(drive) char drive;
#endif
{
#if defined __IBMC__
	if (_chdrive(drive-'A'+1)) return errno; else return 0;
#else
	return ENOSYS;
#endif
}

#ifndef X2C_KRC
extern X2C_INT32 X2C_GetDriveCDNameLength(char drive, X2C_CARD32 *len)
#else
extern X2C_INT32 X2C_GetDriveCDNameLength(drive, len) char drive; X2C_CARD32 *len;
#endif
{
#if defined __IBMC__
	char *dcwd = _getdcwd(drive-'A'+1, NULL, 0);
	if (!dcwd) return errno;
	*len = strlen(dcwd)+1;
	free(dcwd); 
	return 0;
#else
	return ENOSYS;
#endif
}

#ifndef X2C_KRC
extern X2C_INT32 X2C_GetDriveCDName(char drive, X2C_pCHAR dir, X2C_CARD32 len)
#else
extern X2C_INT32 X2C_GetDriveCDName(drive, dir, len) char drive; X2C_pCHAR dir; X2C_CARD32 len;
#endif
{
#if defined __IBMC__
	if (!_getdcwd(drive-'A'+1, dir, len)) return errno;
	return 0;
#else
	return ENOSYS;
#endif
}

#ifndef X2C_KRC
extern X2C_INT32 X2C_GetLabel(char drive, X2C_pCHAR label, X2C_CARD32 len)
#else
extern X2C_INT32 X2C_GetLabel(drive, label, len) char drive; X2C_pCHAR label; X2C_CARD32 len;
#endif
{
#if defined __IBMC__
	return EOS2ERR;
#else
	return ENOSYS;
#endif
}

#ifndef X2C_KRC
extern void X2C_SetFileTime(X2C_pCHAR fname, X2C_CARD32 time)
#else
extern void X2C_SetFileTime(fname, time) X2C_pCHAR fname; X2C_CARD32 time;
#endif
{
}
