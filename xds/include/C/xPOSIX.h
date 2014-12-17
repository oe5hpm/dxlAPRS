#ifndef xPOSIX_H_
#define xPOSIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <fcntl.h>

#if defined(_unix)
#include <unistd.h>
#else
#if defined(_msdos)
#include <io.h>
#endif
#endif

#ifndef X2C_no_spawn
#if defined(_msdos)
#include <process.h>
#endif
#else
#include <X2C.h>
#ifndef X2C_KRC
extern int spawnv(int,char *,X2C_ppcCHAR);
#else
extern int spawnv();
#endif
#endif 

#ifndef P_WAIT
#define P_WAIT 0
#define P_NOWAIT 1
#define P_OVERLAY 2
#endif

typedef char *xPOSIX_PCHAR;

#endif
