/* Copyright (c) 1991,98 XDS Ltd, Russia. All Rights Reserved. */

#include "X2C.h"
#include "x2cLib.h"

#include <time.h>
#include <stdio.h>
#include <xPOSIX.h>

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
void X2C_SetFileModes(
		X2C_CHAR    *name,
                X2C_CHAR    type)
#else
void X2C_SetFileModes(name, type)
	X2C_CHAR    *name;
	X2C_CHAR    type;
#endif
{
#ifdef X2C_mac_os
static const unsigned long CreatorConst =
	256UL*256UL*256UL*(unsigned char)('M') +
	      256UL*256UL*(unsigned char)('P') +
		    256UL*(unsigned char)('S') +
			  (unsigned char)(' ');
static const unsigned long TextTypeConst =
	256UL*256UL*256UL*(unsigned char)('T') +
	      256UL*256UL*(unsigned char)('E') +
		    256UL*(unsigned char)('X') +
			  (unsigned char)('T');
static const unsigned long BinTypeConst =
	256UL*256UL*256UL*(unsigned char)('?') +
	      256UL*256UL*(unsigned char)('?') +
		    256UL*(unsigned char)('?') +
			  (unsigned char)('?');
	if (type=='T')
		fsetfileinfo(name, CreatorConst, TextTypeConst);
	else
		fsetfileinfo(name, CreatorConst, BinTypeConst);
#else
/* nothing known */
#endif
}

#ifndef X2C_KRC
X2C_CARD32 X2C_Time(void)
#else
X2C_CARD32 X2C_Time()
#endif
{
  time_t t;
  time(&t);
  return (X2C_CARD32) t;
} /* END Time */

#ifndef X2C_KRC
X2C_CARD32 X2C_Clock(void)
#else
X2C_CARD32 X2C_Clock()
#endif
{
	static X2C_CARD32 Time0=0;
#ifdef X2C_SCO
	return clock() / 10000;
#else
#ifdef CLOCKS_PER_SEC
	clock_t c = clock();
	if (CLOCKS_PER_SEC == 100) return (c);
	return (X2C_CARD32) ((float) c / (CLOCKS_PER_SEC / 100.));
#else
#ifdef CLK_TCK
	clock_t c = clock();
	if (CLK_TCK == 100) return (c);
	return (X2C_CARD32) ((float) c / (CLK_TCK / 100.));
#else
	if (!Time0) Time0=X2C_Time();
	return (X2C_Time()-Time0) * 100;
#endif
#endif
#endif
}

#ifndef X2C_KRC
X2C_ADDRESS X2C_FDup(FILE * f, char * type)
#else
X2C_ADDRESS X2C_FDup(f,type) FILE * f; char * type;
#endif
{
#if defined(_unix) || defined(_msdos)
	return (X2C_ADDRESS)fdopen(dup(fileno(f)),type);
#else
	return (X2C_ADDRESS)f;
#endif
}

#ifndef X2C_KRC
int X2C_FDup2(FILE * f1, X2C_ADDRESS *f2)
#else
int X2C_FDup2(f1,f2) FILE * f1; X2C_ADDRESS *f2;
#endif
{
#if defined(_unix) || defined(_msdos)
     	return dup2(fileno(f1),fileno(*(FILE**)f2));
#else
	(*f2)=(X2C_ADDRESS)f1;
	return 0;
#endif
}

#ifndef X2C_KRC
int X2C_ChSize(FILE * f, X2C_INT32 sz)
#else
int X2C_ChSize(f,sz) FILE * f; X2C_INT32 sz;
#endif
{
#if defined(_msdos) || defined(X2C_SCO)
     	return chsize(fileno(f),sz);
#else
#ifdef _unix
	return ftruncate(fileno(f),sz);
#else
	return -1;
#endif
#endif
}

#ifndef X2C_KRC
int X2C_exec(char *path, X2C_ppcCHAR argv, int overlay)
#else
int X2C_exec(path, argv, overlay) char *path; X2C_ppcCHAR argv; int overlay;
#endif
{
        if (overlay)
#if defined(_msdos)
                return spawnv(P_WAIT,path,argv);
#else
                return execv(path,argv);
#endif
        else
                return spawnv(P_WAIT,path,argv);

}

#ifndef X2C_KRC
int X2C_GetFileType(FILE *fp)
#else
int X2C_GetFileType(fp) FILE *fp;
#endif
{
        struct stat buf;
        if (fstat(fileno(fp), &buf)) return 3;  /* No diagnostics here yet */
#if defined S_ISREG
/* Assume target is POSIX-compliant (at least in file-type macros respect) */
        if      (S_ISREG(buf.st_mode))  return 0;
        else if (S_ISCHR(buf.st_mode))  return 1;
        else if (S_ISFIFO(buf.st_mode)) return 2;
        else                            return 3;
#elif defined S_IFMT
        if      ((buf.st_mode & S_IFMT) == S_IFREG)  return 0;
        else if ((buf.st_mode & S_IFMT) == S_IFCHR)  return 1;
#ifdef S_IFFIFO
        else if ((buf.st_mode & S_IFMT) == S_IFFIFO) return 2;
#endif
	else                                         return 3;
#elif defined S_IFREG
        if      ((buf.st_mode & S_IFREG) == S_IFREG)  return 0;
        else if ((buf.st_mode & S_IFCHR) == S_IFCHR)  return 1;
	else                                          return 3;
#else
	return 3;
#endif
}
