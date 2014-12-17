/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xosFiles_H_
#define xosFiles_H_
#include "X2C.h"
#include "xlibOS.h"
#include "xrInt64.h"
#include "ChanConsts.h"

#define X2C_fSeekSet 0

#define X2C_fSeekCur 1

#define X2C_fSeekEnd 2

extern X2C_CARD32 X2C_fAccessRead;

extern X2C_CARD32 X2C_fAccessWrite;

extern X2C_CARD32 X2C_fModeNew;

extern X2C_CARD32 X2C_fModeText;

extern X2C_CARD32 X2C_fModeRaw;

#define X2C_ftDisk 0

#define X2C_ftChar 1

#define X2C_ftPipe 2

#define X2C_ftUnk 3

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

extern int X2C_fGetFileType(X2C_OSFHANDLE);


#endif /* xosFiles_H_ */
