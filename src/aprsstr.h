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
typedef uint32_t aprsstr_TIME;

typedef uint32_t aprsstr_IPNUM;

typedef uint32_t aprsstr_UDPPORT;

typedef uint8_t aprsstr_SET8;

typedef uint32_t aprsstr_GHOSTSET[9];

struct aprsstr_POSITION;


struct aprsstr_POSITION {
   float long0;
   float lat;
};

#define aprsstr_CALLLEN 7

#define aprsstr_HBIT 128

extern void aprsstr_Assign(char [], uint32_t, char [], uint32_t);

extern void aprsstr_Append(char [], uint32_t, char [], uint32_t);

extern void aprsstr_Delstr(char [], uint32_t, uint32_t, uint32_t);

extern void aprsstr_Extractword(char [], uint32_t, char [],
                uint32_t);

extern uint32_t aprsstr_Length(char [], uint32_t);

extern void aprsstr_IntToStr(int32_t, uint32_t, char [], uint32_t);

extern void aprsstr_CardToStr(uint32_t, uint32_t, char [],
                uint32_t);

extern void aprsstr_FixToStr(float, uint32_t, char [], uint32_t);

extern char aprsstr_StrToCard(char [], uint32_t, uint32_t *);

extern char aprsstr_StrToInt(char [], uint32_t, int32_t *);

extern char aprsstr_StrToFix(float *, char [], uint32_t);

extern char aprsstr_StrCmp(char [], uint32_t, char [],
                uint32_t);

extern int32_t aprsstr_InStr(char [], uint32_t, char [],
                uint32_t);

extern void aprsstr_TimeToStr(uint32_t, char [], uint32_t);

extern void aprsstr_DateToStr(uint32_t, char [], uint32_t);

extern char aprsstr_StrToTime(char [], uint32_t, uint32_t *);

extern void aprsstr_CtrlHex(char [], uint32_t);

extern int32_t aprsstr_GetIp2(char [], uint32_t, uint32_t *,
                uint32_t *, uint32_t *, char *);

extern void aprsstr_ipv4tostr(uint32_t, char [], uint32_t);

extern char aprsstr_Call2Str(char [], uint32_t, char [],
                uint32_t, uint32_t, uint32_t *);

extern void aprsstr_mon2raw(char [], uint32_t, char [], uint32_t,
                 int32_t *);

extern void aprsstr_raw2mon(char [], uint32_t, char [], uint32_t,
                 uint32_t, uint32_t *, aprsstr_GHOSTSET);

extern void aprsstr_extrudp2(char [], uint32_t, char [],
                uint32_t, int32_t *);

extern void aprsstr_AppCRC(char [], uint32_t, int32_t);

extern void aprsstr_HashCh(char, uint8_t *, uint8_t *);

extern uint32_t aprsstr_Hash(char [], uint32_t, int32_t,
                int32_t);

extern void aprsstr_cleanfilename(char [], uint32_t);

extern void aprsstr_Caps(char [], uint32_t);

extern void aprsstr_rightbound(char [], uint32_t, uint32_t);

extern void aprsstr_loctopos(struct aprsstr_POSITION *, char [],
                uint32_t);

extern void aprsstr_postoloc(char [], uint32_t,
                struct aprsstr_POSITION);

extern void aprsstr_posinval(struct aprsstr_POSITION *);


extern void aprsstr_BEGIN(void);


#endif /* aprsstr_H_ */
