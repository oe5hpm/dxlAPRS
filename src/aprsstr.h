/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef aprsstr_H_
#define aprsstr_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

/* string lib by oe5dxl */
typedef unsigned long aprsstr_TIME;

typedef unsigned long aprsstr_IPNUM;

typedef unsigned long aprsstr_UDPPORT;

typedef unsigned char aprsstr_SET8;

#define aprsstr_CALLLEN 7

#define aprsstr_HBIT 128

extern void aprsstr_Assign(char [], unsigned long, char [], unsigned long);

extern void aprsstr_Append(char [], unsigned long, char [], unsigned long);

extern void aprsstr_Delstr(char [], unsigned long, unsigned long,
                unsigned long);

extern void aprsstr_Extractword(char [], unsigned long, char [],
                unsigned long);

extern unsigned long aprsstr_Length(char [], unsigned long);

extern void aprsstr_IntToStr(long, unsigned long, char [], unsigned long);

extern void aprsstr_CardToStr(unsigned long, unsigned long, char [],
                unsigned long);

extern void aprsstr_FixToStr(float, unsigned long, char [], unsigned long);

extern char aprsstr_StrToCard(char [], unsigned long, unsigned long *);

extern char aprsstr_StrToInt(char [], unsigned long, long *);

extern char aprsstr_StrToFix(float *, char [], unsigned long);

extern char aprsstr_StrCmp(char [], unsigned long, char [], unsigned long);

extern long aprsstr_InStr(char [], unsigned long, char [], unsigned long);

extern void aprsstr_TimeToStr(unsigned long, char [], unsigned long);

extern void aprsstr_DateToStr(unsigned long, char [], unsigned long);

extern char aprsstr_StrToTime(char [], unsigned long, unsigned long *);

extern void aprsstr_CtrlHex(char [], unsigned long);

extern long aprsstr_GetIp2(char [], unsigned long, unsigned long *,
                unsigned long *, unsigned long *, char *);

extern void aprsstr_ipv4tostr(unsigned long, char [], unsigned long);

extern char aprsstr_Call2Str(char [], unsigned long, char [], unsigned long,
                unsigned long, unsigned long *);

extern void aprsstr_mon2raw(char [], unsigned long, char [], unsigned long,
                long *);

extern void aprsstr_raw2mon(char [], unsigned long, char [], unsigned long,
                unsigned long, unsigned long *);

extern void aprsstr_extrudp2(char [], unsigned long, char [], unsigned long,
                long *);

extern void aprsstr_AppCRC(char [], unsigned long, long);

extern void aprsstr_HashCh(char, unsigned char *, unsigned char *);

extern unsigned long aprsstr_Hash(char [], unsigned long, long, long);

extern void aprsstr_cleanfilename(char [], unsigned long);

extern void aprsstr_Caps(char [], unsigned long);

extern void aprsstr_rightbound(char [], unsigned long, unsigned long);


extern void aprsstr_BEGIN(void);


#endif /* aprsstr_H_ */
