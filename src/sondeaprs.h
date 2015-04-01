/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef sondeaprs_H_
#define sondeaprs_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

typedef unsigned long sondeaprs_TIME;

#define sondeaprs_VERSION "sondemod(c) 0.2"

extern void sondeaprs_senddata(double, double, double, double, double,
                double, double, double, double, double, double, double,
                unsigned long, unsigned long, char [], unsigned long,
                unsigned long);

extern long sondeaprs_GetIp(char [], unsigned long, unsigned long *,
                unsigned long *, unsigned long *);

extern char sondeaprs_mycall[100];

extern char sondeaprs_via[100];

extern char sondeaprs_destcall[100];

extern char sondeaprs_objname[100];

extern char sondeaprs_commentfn[1025];

extern char sondeaprs_sym[2];

extern unsigned long sondeaprs_beacontime;

extern unsigned long sondeaprs_lowaltbeacontime;

extern unsigned long sondeaprs_lowalt;

extern unsigned long sondeaprs_toport;

extern unsigned long sondeaprs_ipnum;

extern char sondeaprs_verb;

extern char sondeaprs_verb2;

extern char sondeaprs_nofilter;

extern long sondeaprs_comptyp;

extern long sondeaprs_micessid;

extern long sondeaprs_udpsock;

extern char sondeaprs_anyip;

extern char sondeaprs_sendmon;

extern char sondeaprs_dao;


extern void sondeaprs_BEGIN(void);


#endif /* sondeaprs_H_ */
