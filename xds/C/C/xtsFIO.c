/* Copyright (C) 2000 Excelsior. */

#include <errno.h>
#include <stdio.h>

#include "X2C.h"
#include "IOChan.h"
#include "SysClock.h"
#include "xtsFIO.h"
#ifdef __OS2__
#include "ChanCons.h"
#include "POSIXIOC.h"
#else
#include "ChanConsts.h"
#include "POSIXIOChan.h"
#endif

#if defined _msdos
#if (defined(_MSC_VER) && defined(_WIN32))
#include <direct.h>
#include <io.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utime.h>
#define GETDRIVE _getdrive
#define CHDRIVE _chdrive
#define GETCWD _getcwd
#define GETDCWD _getdcwd
#define CHDIR _chdir
#define MKDIR(PATH, OPT) _mkdir(PATH)
#define RMDIR _rmdir
#define FILENO _fileno
#define FSTAT _fstat
#define STAT _stat
#elif defined(__WATCOMC__)
#include <direct.h>
#include <time.h>
#define GETDRIVE _getdrive
#define CHDRIVE _chdrive
#define CHDIR chdir
#define GETCWD _getcwd
#define GETDCWD _getdcwd
#define MKDIR(PATH, OPT) mkdir(PATH)
#define RMDIR rmdir
#define FILENO fileno
#define FSTAT fstat
#define STAT stat
#elif defined(__IBMC__)
#include <direct.h>
#include <sys/stat.h>
#include <time.h>
#define GETDRIVE _getdrive
#define CHDRIVE _chdrive
#define CHDIR _chdir
#define GETCWD _getcwd
#define MKDIR(PATH, OPT) _mkdir(PATH)
#define RMDIR _rmdir
#define FILENO _fileno
#define FSTAT _fstat
#define STAT stat
#endif
#elif defined _unix
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>
#define CHDIR chdir
#define GETCWD getcwd
#define MKDIR(PATH, OPT) mkdir(PATH, OPT)
#define RMDIR rmdir
#define FILENO fileno
#define FSTAT fstat
#define STAT stat
#endif

void xtsFIO_setShM(X2C_BOOLEAN denyRd, X2C_BOOLEAN denyWr) {
}


void xtsFIO_restoreShM(void) {
}
 
void xtsFIO_mk_chan(IOChan_ChanId cid,
             X2C_CARD32 fh,
             ChanConsts_FlagSet flags,
             X2C_CARD8 *res) {
  *res = ChanConsts_otherProblem;
}

void makeDriveStr(X2C_CARD32 drive, char *drvStr, X2C_CARD32 len) {
#ifdef _msdos
    if (len >= 2) {
        drvStr[0] = 'A'+drive-1;
        drvStr[1] = ':';
        drvStr[2] = 0;
    }
    else
        drvStr[0] = 0;
#else
    drvStr[0] = 0;
#endif
}

void makeRootStr(X2C_CARD32 drive, char *drvStr, X2C_CARD32 len) {
#ifdef _msdos
    if (len >= 3) {
        drvStr[0] = 'A'+drive-1;
        drvStr[1] = ':';
        drvStr[2] = '\\';
        drvStr[3] = 0;
    }
    else
        drvStr[0] = 0;
#else
    drvStr[0] = 0;
#endif
}

void xtsFIO_SetDrive(X2C_CARD32 drive, X2C_CARD32 *res) {
#ifdef _msdos
    *res = CHDRIVE(drive) ? errno : 0;
#else
    *res = 0xff;
#endif
}

X2C_CARD32 xtsFIO_GetDrive(void) {
#ifdef _msdos
    return GETDRIVE();
#else
    return 0xff;
#endif
}

void xtsFIO_GetSpaceInfo(X2C_CARD32 drive, struct xtsFIO_SpaceInfo *info,
                X2C_CARD32 *res) {
#ifdef _msdos
    *res = 0xff;
#else
    *res = 0xff;
#endif
}

void xtsFIO_GetLabel(X2C_CARD32 drive, char *label, X2C_CARD32 len,
                X2C_CARD32 *res) {
#ifdef _msdos
    *res = 0xff;
#else
    *res = 0xff;
#endif
}

void xtsFIO_ChgDir(char *path, X2C_CARD32 len, X2C_CARD32 *res) {
/* !!!    StrToC (path, CPath); */
    *res = CHDIR(path) ? errno : 0;
}

void xtsFIO_GetDir(X2C_CARD32 drive, char *path, X2C_CARD32 len,
                X2C_CARD32 *res) {
#if defined _msdos
    char *s, *d;
    if (drive) 
        *res = ( GETDCWD(drive, path, (size_t)len) == NULL ) ? errno : 0;
    else 
        *res = ( GETCWD(path, (size_t)len) == NULL ) ? errno : 0;
    /* GETCWD returns full path; remove drive name from 'path' */
    if (!(*res) && (path[1]==':'))
        /* (s-1) is used to copy terminating null character */
        for (s = path+2,d = path; len && *(s-1); len--) *d++ = *s++;
#elif defined _unix
    if (drive) *res = ENOENT;
    else
        *res = GETCWD(path, (size_t)len) ? errno : 0;
#endif
}

void xtsFIO_CreateDir(char *path, X2C_CARD32 len, X2C_CARD32 *res) {
/* !!!    StrToC (path, CPath); */
    *res = MKDIR(path, S_IRWXO+S_IRWXG+S_IRWXU) ? errno : 0;
}

void xtsFIO_RmvDir(char *path, X2C_CARD32 len, X2C_CARD32 *res) {
/* !!!    StrToC (path, CPath); */
    *res = RMDIR(path) ? errno : 0;
}


/* Directory scanning */
char curpath[512];
#if defined(_unix) || defined(__WATCOMC__)
DIR *dir;
struct dirent* dep;
#elif (defined(_MSC_VER) && defined(_WIN32))
long dirhandle;
struct _finddata_t fileinfo;
#endif

void TransfRes(struct FIO_DirEntry *info) {
#if defined(_unix)
    struct stat *buf;
    struct tm *tms;    
    
    STAT (dep->d_name, buf);
    info->attr    = buf->st_mode;
    
    tms = localtime(&(buf->st_mtime));
    info->timePkd = tms->tm_hour<<11+
                   tms->tm_min<<5+
                   tms->tm_sec/2;
    info->datePkd = (tms->tm_year-80)<<9+
                   tms->tm_mon<<5+
                   tms->tm_mday;
                   
    info->sizeL   = buf->st_size;
    info->sizeH   = 0;
    strncpy((char*)&info->name, dep->d_name, (size_t)MAXNAMLEN);
/*
#ifdef S_ISDIR
    dir->is_dir  = S_ISDIR(buf.st_mode);
#else
    dir->is_dir  = ((buf.st_mode & S_IFMT) == S_IFDIR)
#endif
*/
#elif defined(__WATCOMC__)
    info->attr    = dep->d_attr;
    info->timePkd = dep->d_time;
    info->datePkd = dep->d_date;
    info->sizeL   = dep->d_size;
    info->sizeH   = 0;
    strncpy(info->name, dep->d_name, (size_t)NAME_MAX);
#elif (defined(_MSC_VER) && defined(_WIN32))
    struct tm *tms;
    info->attr    = fileinfo.attrib;
    tms = localtime(&fileinfo.time_write);
    info->timePkd = tms->tm_hour<<11+
                   tms->tm_min<<5+
                   tms->tm_sec/2;
    info->datePkd = (tms->tm_year-80)<<9+
                   tms->tm_mon<<5+
                   tms->tm_mday;
    info->sizeL   = (X2C_CARD32)(fileinfo.size/ULONG_MAX);
    info->sizeH   = (X2C_CARD32)(fileinfo.size%ULONG_MAX);
    strncpy(info->name, fileinfo.name, (size_t)_MAX_FNAME);
#endif
}

void xtsFIO_ScanFirst(char *path, X2C_CARD32 len, X2C_CARD8 attr,
                struct FIO_DirEntry *info, X2C_CARD32 *res) {
#if defined(_unix) || defined(__WATCOMC__)
    strncpy(curpath,path,len < 512 ? len : 512);
    dir = opendir(path);
    if (dir)
        xtsFIO_ScanNext(info, res);
    else
	*res = errno;
#elif (defined(_MSC_VER) && defined(_WIN32))
    if ((dirhandle = _findfirst(path, &fileinfo)) != -1) {
        TransfRes(info);
        *res = 0;
    }
    else
        *res = errno;
#endif   
}

void xtsFIO_ScanNext(struct FIO_DirEntry *info, X2C_CARD32 *res) {
#if defined(_unix) || defined(__WATCOMC__)
    dep = readdir(dir);
    if (dep == NULL) {
        if (errno) *res = errno;
        else *res = ENOENT;
        return;
    }
    TransfRes(info);
    *res = 0;
#elif (defined(_MSC_VER) && defined(_WIN32))
    if (_findnext(dirhandle, &fileinfo)) {
        *res=errno;
        return;
    }
    TransfRes(info);
    *res = 0;
#endif
}

void xtsFIO_ScanClose(struct FIO_DirEntry *info) {
#if defined(_unix) || defined(__WATCOMC__)
    closedir(dir);
#elif (defined(_MSC_VER) && defined(_WIN32))
    _findclose(dirhandle);
#endif
}

void xtsFIO_SetFileDate(IOChan_ChanId f, struct SysClock_DateTime bb) {
    struct SysClock_DateTime *b = &bb;
#if (defined(_MSC_VER) && defined(_WIN32))
    struct _utimbuf timebuf;
    struct tm tms;
    
    tms.tm_year  = b->year-1900;
    tms.tm_mon   = b->month-1;
    tms.tm_mday  = b->day;
    tms.tm_hour  = b->hour;
    tms.tm_min   = b->minute;
    tms.tm_sec   = b->second;
    tms.tm_isdst = 0;
    timebuf.actime  = mktime(&tms);
    timebuf.modtime = timebuf.actime;
    _futime(FILENO((FILE *)POSIXIOChan_GetFilePtr(f)),&timebuf);

#elif defined(_unix)
    struct utimbuf timebuf;
    char filename[FILENAME_MAX+1];
    xDevData_FileName xdsFileName;
    int len;
    struct tm tms;
    
    tms.tm_year  = b->year-1900;
    tms.tm_mon   = b->month-1;
    tms.tm_mday  = b->day;
    tms.tm_hour  = b->hour;
    tms.tm_min   = b->minute;
    tms.tm_sec   = b->second;
    tms.tm_isdst = 0;
    timebuf.actime  = mktime(&tms);
    timebuf.modtime = timebuf.actime;

    xdsFileName = POSIXIOChan_GetFileName(f);
    len = xdsFileName->Len0;
    if(len > FILENAME_MAX) {
	// nothing to do :(
        return;
    }
    strcpy(filename, (char*)xdsFileName->Adr);
    filename[len+1] = '\0';
    utime(filename, &timebuf);
#endif
}

X2C_BOOLEAN xtsFIO_GetFileStamp(IOChan_ChanId f, struct SysClock_DateTime *b) {
    int handle = FILENO((FILE *)POSIXIOChan_GetFilePtr(f));
    struct STAT buf;
    struct tm *tms;

    if (FSTAT(handle, &buf)) return 0;
    tms = localtime(&buf.st_mtime);
    b->year   = tms->tm_year+1900;
    b->month  = tms->tm_mon+1;
    b->day    = tms->tm_mday;
    b->hour   = tms->tm_hour;
    b->minute = tms->tm_min;
    b->second = tms->tm_sec;
    b->zone   = 0;
    return 1;
}

void xtsFIO_BEGIN(void){ 
   static int xtsFIO_init = 0;
   if (xtsFIO_init) return;
   xtsFIO_init = 1;
   IOChan_BEGIN ();
   SysClock_BEGIN ();
#ifdef __OS2__
   ChanCons_BEGIN ();
   POSIXIOC_BEGIN ();
#else
   ChanConsts_BEGIN ();
   POSIXIOChan_BEGIN();
#endif
}
