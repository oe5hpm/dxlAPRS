/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef platform_H_
#define platform_H_
#include "X2C.h"

extern X2C_BOOLEAN pl_unix;

extern X2C_BOOLEAN pl_vms;

extern X2C_BOOLEAN pl_msdos;

extern X2C_BOOLEAN pl_amiga;

extern X2C_BOOLEAN pl_fatfs;

extern X2C_CHAR extSep;

extern X2C_CHAR pathSep;

extern X2C_CHAR pathBeg;

extern X2C_CHAR pathEnd;

extern X2C_CHAR drvSep;

extern X2C_CHAR lineSep[8];

extern X2C_CHAR textSep[8];

#define chr_SEP0 '\012'
#define SEP0 "\012"

#define chr_SEP1 0
#define SEP1 ""

extern X2C_BOOLEAN IsPathDelim(X2C_CHAR);


#endif /* platform_H_ */
