/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xlibOS_H_
#define xlibOS_H_
#include "X2C.h"
#include "xmRTS.h"
#include "ChanConsts.h"
#include "xrInt64.h"

extern X2C_CARD32 X2C_EnvStringLength(X2C_pCHAR);

extern void X2C_EnvString(X2C_pCHAR, X2C_pCHAR, X2C_CARD32);

extern X2C_BOOLEAN X2C_Exists(X2C_pCHAR);

extern void X2C_ModifyTime(X2C_pCHAR, X2C_CARD32 *, X2C_BOOLEAN *);

extern void X2C_SetFileTime(X2C_pCHAR, X2C_CARD32);

extern int X2C_Remove(X2C_pCHAR);

extern int X2C_Rename(X2C_pCHAR, X2C_pCHAR);

extern void X2C_FullName(X2C_pCHAR, X2C_CARD32, X2C_pCHAR);

struct X2C_TimeStruct;


struct X2C_TimeStruct {
   X2C_CARD32 year;
   X2C_CARD32 month;
   X2C_CARD32 day;
   X2C_CARD32 hour;
   X2C_CARD32 min0;
   X2C_CARD32 sec;
   X2C_CARD32 fracs;
   X2C_INT32 zone;
   X2C_BOOLEAN stf;
};

extern X2C_BOOLEAN X2C_CanGetTime(void);

extern X2C_BOOLEAN X2C_CanSetTime(void);

extern void X2C_GetTime(struct X2C_TimeStruct *);

extern void X2C_SetTime(struct X2C_TimeStruct *);

extern X2C_CARD32 X2C_FracsPerSec(void);

extern X2C_INT32 X2C_TimeCompare(struct X2C_TimeStruct,
                struct X2C_TimeStruct);

extern X2C_CARD32 X2C_TimeDayInt(struct X2C_TimeStruct,
                struct X2C_TimeStruct);

extern X2C_CARD32 X2C_TimeSecInt(struct X2C_TimeStruct,
                struct X2C_TimeStruct);

extern void X2C_TimeDayAdd(struct X2C_TimeStruct, X2C_CARD32,
                struct X2C_TimeStruct *);

extern void X2C_TimeSecAdd(struct X2C_TimeStruct, X2C_CARD32,
                struct X2C_TimeStruct *);

extern X2C_CARD32 X2C_TimeDayNum(X2C_CARD32, X2C_CARD32, X2C_CARD32);

#define X2C_fSeekSet 0

#define X2C_fSeekCur 1

#define X2C_fSeekEnd 2

extern X2C_CARD32 X2C_fAccessRead;

extern X2C_CARD32 X2C_fAccessWrite;

extern X2C_CARD32 X2C_fModeNew;

extern X2C_CARD32 X2C_fModeText;

extern X2C_CARD32 X2C_fModeRaw;

typedef void *X2C_OSFHANDLE;

typedef struct X2C_int64 X2C_FPOS;

#define X2C_MAXXATTRS 8

typedef X2C_CARD32 X2C_FXATTRS[8];

extern X2C_BOOLEAN X2C_IsMixAllowed(void);

extern void X2C_fGetXAttrs(X2C_FXATTRS);

extern void X2C_fSetXAttrs(X2C_FXATTRS);

extern X2C_CARD8 X2C_fOpen(X2C_OSFHANDLE *, X2C_CHAR [], X2C_CARD32);

extern int X2C_fClose(X2C_OSFHANDLE *);

extern int X2C_fRead(X2C_OSFHANDLE, X2C_ADDRESS, X2C_CARD32, X2C_CARD32 *);

extern int X2C_fWrite(X2C_OSFHANDLE, X2C_ADDRESS, X2C_CARD32, X2C_CARD32 *);

extern int X2C_fSeek(X2C_OSFHANDLE, struct X2C_int64 *, int);

extern int X2C_fTell(X2C_OSFHANDLE, struct X2C_int64 *);

extern int X2C_fSize(X2C_OSFHANDLE, struct X2C_int64 *);

extern int X2C_fFlush(X2C_OSFHANDLE);

extern int X2C_fChSize(X2C_OSFHANDLE);

#define X2C_fStdIn 0

#define X2C_fStdOut 1

#define X2C_fStdErr 2

extern int X2C_fGetStd(X2C_OSFHANDLE *, int);

extern int X2C_fSetStd(X2C_OSFHANDLE, int);

#define X2C_ftDisk 0

#define X2C_ftChar 1

#define X2C_ftPipe 2

#define X2C_ftUnk 3

extern int X2C_fGetFileType(X2C_OSFHANDLE);

extern X2C_INT32 X2C_Execute(X2C_pCHAR, X2C_pCHAR, int, X2C_CARD32 *);

extern X2C_INT32 X2C_ttyReadNE(X2C_ADDRESS, X2C_CARD32, X2C_CARD32 *);

extern X2C_INT32 X2C_ttyReadLE(X2C_ADDRESS, X2C_CARD32, X2C_CARD32 *);

extern X2C_INT32 X2C_ttyWrite(X2C_ADDRESS, X2C_CARD32);

extern X2C_INT32 X2C_InitTTY(void);

#define X2C_DirSysAreaSize 1024

struct X2C_Dir;


struct X2C_Dir {
   X2C_LOC sys[1024];
   X2C_CARD32 namelen;
   X2C_CARD32 size;
   struct X2C_TimeStruct cretime;
   struct X2C_TimeStruct mdftime;
   X2C_BOOLEAN is_dir;
   X2C_BOOLEAN done;
};

extern void X2C_DirOpen(struct X2C_Dir *, X2C_pCHAR);

extern void X2C_DirClose(struct X2C_Dir *);

extern void X2C_DirNext(struct X2C_Dir *);

extern void X2C_DirGetName(struct X2C_Dir *, X2C_pCHAR, X2C_CARD32);

extern X2C_CARD32 X2C_GetCDNameLength(void);

extern void X2C_GetCDName(X2C_pCHAR, X2C_CARD32);

extern X2C_BOOLEAN X2C_SetCD(X2C_pCHAR);

extern X2C_BOOLEAN X2C_CreateDirectory(X2C_pCHAR);

extern X2C_BOOLEAN X2C_RemoveDirectory(X2C_pCHAR);

extern X2C_INT32 X2C_GetDrive(X2C_CHAR *);

extern X2C_INT32 X2C_SetDrive(X2C_CHAR);

extern X2C_INT32 X2C_GetDriveCDNameLength(X2C_CHAR, X2C_CARD32 *);

extern X2C_INT32 X2C_GetDriveCDName(X2C_CHAR, X2C_pCHAR, X2C_CARD32);

extern X2C_INT32 X2C_GetLabel(X2C_CHAR, X2C_pCHAR, X2C_CARD32);


#endif /* xlibOS_H_ */
