#include <errno.h>
#include "X2C.h"

X2C_CARD32 SysErr_allRight      = 0;
X2C_CARD32 SysErr_noMoreFiles   = EMFILE;
X2C_CARD32 SysErr_writeProtect  = EACCES;
X2C_CARD32 SysErr_pathNotFound  = ENOENT;

#ifdef _unix
X2C_CARD32 SysErr_invalidDrive  = ENOENT;
#else
X2C_CARD32 SysErr_invalidDrive  = EACCES;
#endif

X2C_CARD32 SysErr_tooManyOpen   = EMFILE;
X2C_CARD32 SysErr_fileNotFound  = ENOENT;
X2C_CARD32 SysErr_fileExists    = EEXIST;

X2C_CARD32 SysErr_cannotMake    = EACCES;
X2C_CARD32 SysErr_alreadyExists = EEXIST;
X2C_CARD32 SysErr_dirNotEmpty   = ENOTEMPTY;

X2C_CARD32 SysErr_otherProblem  = 0xffffffff;

void SysErr_BEGIN(void) { /* !!! */
}
