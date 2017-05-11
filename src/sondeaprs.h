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

typedef uint32_t sondeaprs_TIME;

#define sondeaprs_VERSION "sondemod(c) 1.0"

extern void sondeaprs_senddata(double, double, double,
                double, double, double, double,
                double, double, double, double,
                double, double, double, double,
                double, uint32_t, uint32_t, char [],
                uint32_t, uint32_t, uint32_t, char [], uint32_t,
                uint32_t, uint32_t, char);

extern int32_t sondeaprs_GetIp(char [], uint32_t, uint32_t *,
                uint32_t *, uint32_t *);

extern char sondeaprs_via[100];

extern char sondeaprs_destcall[100];

extern char sondeaprs_objname[100];

extern char sondeaprs_commentfn[1025];

extern char sondeaprs_sym[2];

extern uint32_t sondeaprs_beacontime;

extern uint32_t sondeaprs_lowaltbeacontime;

extern uint32_t sondeaprs_lowalt;

extern uint32_t sondeaprs_toport;

extern uint32_t sondeaprs_ipnum;

extern char sondeaprs_verb;

extern char sondeaprs_verb2;

extern char sondeaprs_nofilter;

extern int32_t sondeaprs_comptyp;

extern int32_t sondeaprs_micessid;

extern int32_t sondeaprs_udpsock;

extern char sondeaprs_anyip;

extern char sondeaprs_sendmon;

extern char sondeaprs_dao;


extern void sondeaprs_BEGIN(void);


#endif /* sondeaprs_H_ */
