/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef aprstext_H_
#define aprstext_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* aprs tracks on osm map by oe5dxl */
#define aprstext_TEXTCOLEND "\376"

#define aprstext_TEXTCOLLGR "\367"

#define aprstext_TEXTCOLRED "\370"

#define aprstext_TEXTCOLBLU "\371"

#define aprstext_TEXTCOLYEL "\372"

#define aprstext_TEXTCOLORA "\373"

#define aprstext_TEXTCOLVIO "\374"

#define aprstext_TEXTINSERTSYMBOL "\375"

extern void aprstext_decode(char [], uint32_t, aprsdecode_pFRAMEHIST,
                aprsdecode_pFRAMEHIST, aprsdecode_pVARDAT, uint32_t,
                char, struct aprsdecode_DAT *);

extern void aprstext_strcp(char [], uint32_t, uint32_t, uint32_t,
                char [], uint32_t);

extern void aprstext_optext(uint32_t, struct aprsdecode_CLICKOBJECT *,
                char *, char [], uint32_t);

extern void aprstext_measure(struct aprsstr_POSITION,
                struct aprsstr_POSITION, char [], uint32_t,
                char);

extern void aprstext_postostr(struct aprsstr_POSITION, char, char [],
                 uint32_t);

extern void aprstext_degtopos(char [], uint32_t,
                struct aprsstr_POSITION *);

extern void aprstext_deghtopos(char [], uint32_t,
                struct aprsstr_POSITION *);

extern void aprstext_degdeztopos(char [], uint32_t,
                struct aprsstr_POSITION *);

extern void aprstext_deganytopos(char [], uint32_t,
                struct aprsstr_POSITION *);

extern char aprstext_getmypos(struct aprsstr_POSITION *);

extern void aprstext_listop(char);

extern void aprstext_listtyps(char, char, char [],
                uint32_t);

extern void aprstext_listin(char [], uint32_t, char, char,
                char, int32_t, int32_t, int32_t);

extern void aprstext_DateLocToStr(uint32_t, char [], uint32_t);
/* append (+localtime) to time */

extern void aprstext_encbeacon(char [], uint32_t, uint32_t *);

extern float aprstext_FtoC(float);
/* fahrenheit to celsius */

extern float aprstext_CtoF(float);

extern aprsdecode_pOPHIST aprstext_oppo(aprsdecode_MONCALL);

extern void aprstext_setmarkalti(aprsdecode_pFRAMEHIST, aprsdecode_pOPHIST,
                char);

extern void aprstext_setmark1(struct aprsstr_POSITION, char,
                int32_t, uint32_t);

extern void aprstext_Apphex(char [], uint32_t, char [],
                uint32_t);

extern char aprstext_isacall(char [], uint32_t);

extern void aprstext_logfndate(uint32_t, char [], uint32_t);

extern void aprstext_sievert2str(float, char [], uint32_t);

extern void aprstext_compressdata(struct aprsstr_POSITION, uint32_t,
                uint32_t, int32_t, char [], uint32_t, char [],
                uint32_t);

extern void aprstext_decodelistline(char [], uint32_t, char [],
                uint32_t, uint32_t);

extern char aprstext_callwrong(char [], uint32_t);


extern void aprstext_BEGIN(void);


#endif /* aprstext_H_ */
