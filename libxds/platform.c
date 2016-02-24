/* Copyright (c) xTech Ltd, Russia 1991,95. All Rights Reserved */
/* "@(#)platform.c 2.00 X2C" */

#include "X2C.h"
#include "platform.h"
#include "ctype.h"

#ifdef _msdos
char pl_msdos = 1;
char pl_fatfs = 1;
#else
char pl_msdos = 0;
char pl_fatfs = 0;
#endif

#ifdef _vms
char pl_vms = 1;
#else
char pl_vms = 0;
#endif

#ifdef _unix
char pl_unix = 1;
#else
char pl_unix = 0;
#endif

#ifdef _amiga
char pl_amiga = 1;
#else
char pl_amiga = 0;
#endif

char extSep = '.';

#ifdef _vms
char pathSep   = '\0';
char pathBeg   = '[';
char pathEnd   = ']';
char lineSep[8]= "\n";
char drvSep    = ':';
#endif

#ifdef _msdos
char pathSep   = '\\';
char pathBeg   = '\\';
char pathEnd   = '\\';
char lineSep[8]= "\r\n";
char drvSep    = ':';
#endif

#ifdef _amiga
char pathSep   = '/';
char pathBeg   = '/';
char pathEnd   = '/';
char lineSep[8]= "\r\n";
char drvSep    = ':';
#endif

#ifdef X2C_mac_os
char pathSep   = ':';
char pathBeg   = ':';
char pathEnd   = ':';
char lineSep[8]= "\015";
char drvSep    = '\0';
#endif

#if !defined(_vms) && !defined(_msdos) && \
    !defined(_amiga) && !defined(X2C_mac_os)
char pathSep   = '/';
char pathBeg   = '/';
char pathEnd   = '/';
char lineSep[8]= "\n";
char drvSep    = '\0';
#endif

char textSep[8]= "\n";

#ifndef X2C_KRC
X2C_BOOLEAN X2C_PROCLASS IsPathDelim(X2C_CHAR c)
#else
X2C_BOOLEAN X2C_PROCLASS IsPathDelim(c) X2C_CHAR c;
#endif
{
#if defined(_msdos) || defined(_amiga) || defined(X2C_nocolon)
  return (isspace (c) || c == ';' || c == '(' || c == ')');
#else
#if defined(X2C_mac_os)
  return (c == ';');
#else
  return (isspace (c) || c == ';' || c == '(' || c == ')' || c == ':');
#endif
#endif
}

