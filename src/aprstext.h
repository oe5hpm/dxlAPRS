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
#ifndef aprspos_H_
#include "aprspos.h"
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

extern void aprstext_decode(char [], unsigned long, aprsdecode_pFRAMEHIST,
                aprsdecode_pFRAMEHIST, aprsdecode_pVARDAT, unsigned long,
                char, struct aprsdecode_DAT *);

extern void aprstext_strcp(char [], unsigned long, unsigned long,
                unsigned long, char [], unsigned long);

extern void aprstext_optext(unsigned long, struct aprsdecode_CLICKOBJECT *,
                char *, char [], unsigned long);

extern void aprstext_measure(struct aprspos_POSITION,
                struct aprspos_POSITION, char [], unsigned long, char);

extern void aprstext_postostr(struct aprspos_POSITION, char, char [],
                unsigned long);

extern void aprstext_degtopos(char [], unsigned long,
                struct aprspos_POSITION *);

extern void aprstext_deghtopos(char [], unsigned long,
                struct aprspos_POSITION *);

extern void aprstext_degdeztopos(char [], unsigned long,
                struct aprspos_POSITION *);

extern void aprstext_deganytopos(char [], unsigned long,
                struct aprspos_POSITION *);

extern char aprstext_getmypos(struct aprspos_POSITION *);

extern void aprstext_listop(char);

extern void aprstext_listtyps(char, char, char [], unsigned long);

extern void aprstext_listin(char [], unsigned long, char, char, char);

extern void aprstext_DateLocToStr(unsigned long, char [], unsigned long);
/* append (+localtime) to time */

extern void aprstext_encbeacon(char [], unsigned long, unsigned long *);

extern float aprstext_FtoC(float);
/* fahrenheit to celsius */

extern float aprstext_CtoF(float);

extern aprsdecode_pOPHIST aprstext_oppo(aprsdecode_MONCALL);

extern void aprstext_setmarkalti(aprsdecode_pFRAMEHIST, aprsdecode_pOPHIST,
                char);

extern void aprstext_setmark1(struct aprspos_POSITION, char, long,
                unsigned long);

extern void aprstext_Apphex(char [], unsigned long, char [], unsigned long);

extern char aprstext_isacall(char [], unsigned long);

extern void aprstext_logfndate(unsigned long, char [], unsigned long);

extern void aprstext_sievert2str(float, char [], unsigned long);

extern void aprstext_compressdata(struct aprspos_POSITION, unsigned long,
                unsigned long, long, char [], unsigned long, char [],
                unsigned long);


extern void aprstext_BEGIN(void);


#endif /* aprstext_H_ */
