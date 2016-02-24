/* Copyright (c) 1998 XDS Ltd, Russia. All Rights Reserved. */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "X2C.h"
#include "xlibOS.h"

#if defined _unix
/* Assume that directory functions are POSIX-compliant */
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#elif defined _msdos
/* MS-DOS derived systems - a variety of compilers */
#if defined __WATCOMC__
/* Watcom */
#include <direct.h>
#elif defined _MSC_VER
/* MSVC */
#include <direct.h>
#elif defined __IBMC__
/* IBM CSet++ or Visual Age C++ */
#include <direct.h>
#if defined __OS2__
#else
#endif /* __OS2__ */
#else
#endif /* __WATCOMC__ */
#else
#endif /* _unix */

#ifndef X2C_KRC
extern void X2C_UnpackTime(time_t, struct X2C_TimeStruct *);
#else
extern void X2C_UnpackTime();
#endif
/* Implemented in xosTime.c */

#if defined(_unix) || defined(__WATCOMC__)
struct DirIterator {
	DIR *dp;
	X2C_CARD32 magic;
	char name[1];
};

#define DI_MAGIC 0xCAFECAFE
#define DI_NAME_MAX (X2C_DirSysAreaSize-sizeof(struct DirIterator))
#endif

#ifndef X2C_KRC
void X2C_DirOpen(struct X2C_Dir *dir, X2C_pCHAR name)
#else
void X2C_DirOpen(dir, name)
	struct X2C_Dir *dir;
	X2C_pCHAR name;
#endif
{       
#if defined(_unix) || defined(__WATCOMC__)
	DIR *dp = opendir(name);
	if (dp == NULL) {	
		dir->done = 0;
		return;
	}
	((struct DirIterator*)&dir->sys)->dp = dp;
	((struct DirIterator*)&dir->sys)->magic = DI_MAGIC;
	X2C_DirNext(dir);	/* It will set dir->done */
#else
	dir->done = 0;
#endif
} /* end X2C_DirOpen() */

#ifndef X2C_KRC
void X2C_DirNext(struct X2C_Dir *dir)
#else
void X2C_DirNext(dir)
	struct X2C_Dir *dir;
#endif
{
#if defined(_unix) || defined(__WATCOMC__)
	struct dirent* dep;
	struct stat buf;
	dir->done = 0;
	if (((struct DirIterator*)&dir->sys)->magic != DI_MAGIC) return;
	dep = readdir(((struct DirIterator*)&dir->sys)->dp);
	if (dep == NULL) return;
	strncpy(((struct DirIterator*)&dir->sys)->name,
		dep->d_name,
		(size_t)DI_NAME_MAX);
	dir->namelen = (X2C_CARD32)strlen(dep->d_name);
	stat(dep->d_name,&buf);
	dir->size    = (X2C_CARD32)buf.st_size;
	X2C_UnpackTime(buf.st_mtime,&dir->cretime);
	X2C_UnpackTime(buf.st_mtime,&dir->mdftime);
#ifdef S_ISDIR
	dir->is_dir  = S_ISDIR(buf.st_mode);
#else
        dir->is_dir  = ((buf.st_mode & S_IFMT) == S_IFDIR) 
#endif
	dir->done = 1;
#else
	dir->done = 0;
#endif
} /* end X2C_DirNext() */

#ifndef X2C_KRC
void X2C_DirClose(struct X2C_Dir *dir)
#else
void X2C_DirClose(dir)
	struct X2C_Dir *dir;
#endif
{
#if defined(_unix) || defined(__WATCOMC__)
	dir->done = 0;
	if (((struct DirIterator*)&dir->sys)->magic != DI_MAGIC) return;
	dir->done = (closedir(((struct DirIterator*)&dir->sys)->dp) == 0);
	if (dir->done) ((struct DirIterator*)&dir->sys)->magic = 0;
#else
	dir->done = 0;
#endif
} /* end X2C_DirClose() */

#ifndef X2C_KRC
void X2C_DirGetName(struct X2C_Dir *dir,
		    X2C_pCHAR name,
		    X2C_CARD32 nmlen)
#else
void X2C_DirGetName(dir, name, nmlen)
	struct X2C_Dir *dir;
	X2C_pCHAR name;
	X2C_CARD32 nmlen;
#endif
{
#if defined(_unix) || defined(__WATCOMC__)
	dir->done = (((struct DirIterator*)&dir->sys)->magic == DI_MAGIC);
	if (dir->done)
		strncpy(name,
		((struct DirIterator*)&dir->sys)->name,
		(size_t)nmlen);
#else
	dir->done = 0;
#endif
} /* end X2C_DirGetName() */

#ifndef X2C_KRC
X2C_CARD32 X2C_GetCDNameLength(void)
#else
X2C_CARD32 X2C_GetCDNameLength()
#endif
{
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
	static char *buf = NULL;
	static int size = PATH_MAX;
	if (buf == NULL) buf = (char*)malloc(PATH_MAX);
	for (;;) {
		if (getcwd(buf,size) != NULL) return strlen(buf);
		if (errno != ERANGE) return 0;
		/* Buffer too small */ 
		free(buf);
		size *= 2;
		buf = (char*)malloc(size);
	}
} /* end X2C_GetCDNameLength() */

#ifndef X2C_KRC
void X2C_GetCDName(X2C_pCHAR s, X2C_CARD32 slen)
#else
void X2C_GetCDName(s, slen)
	X2C_pCHAR s;
	X2C_CARD32 slen;
#endif
{
	if (getcwd(s,slen) == NULL) s[0] = 0;
} /* end X2C_GetCDName() */

#ifndef X2C_KRC
X2C_BOOLEAN X2C_SetCD(X2C_pCHAR s)
#else
X2C_BOOLEAN X2C_SetCD(s)
	X2C_pCHAR s;
#endif
{
	return (chdir(s) == 0);
} /* end X2C_SetCD() */

#ifndef X2C_KRC
X2C_BOOLEAN X2C_CreateDirectory(X2C_pCHAR name)
#else
X2C_BOOLEAN X2C_CreateDirectory(name)
	X2C_pCHAR name;
#endif
{
#if defined _unix
	return (mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#else
	return (mkdir(name) == 0);
#endif
} /* end X2C_CreateDirectory() */

#ifndef X2C_KRC
X2C_BOOLEAN X2C_RemoveDirectory(X2C_pCHAR name)
#else
X2C_BOOLEAN X2C_RemoveDirectory(name)
	X2C_pCHAR name;
#endif
{
	return (rmdir(name) == 0);
} /* end X2C_RemoveDirectory() */

