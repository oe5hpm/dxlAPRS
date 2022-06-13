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
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

typedef uint32_t sondeaprs_TIME;

struct sondeaprs_SDRBLOCK;


struct sondeaprs_SDRBLOCK {
   uint32_t freq;
   uint32_t maxafc;
   uint32_t db;
   int32_t afc;
   char name[4];
   char valid;
};

#define sondeaprs_VERSION "sondemod 1.37"

#define sondeaprs_minusG "G"

#define sondeaprs_minusE "E"

#define sondeaprs_minusP "P"

#define sondeaprs_minusa "a"

#define sondeaprs_LASTSECONDS 3600

#define sondeaprs_BEFOREBURST 100000

struct sondeaprs__D0;


struct sondeaprs__D0 {
   uint32_t num;
   uint32_t level;
};

typedef struct sondeaprs__D0 sondeaprs_SATSIG[12];

typedef struct sondeaprs__D0 * sondeaprs_pSATSIG;

extern void sondeaprs_senddata(double, double, double,
                double, double, double, double,
                double, double, double, double,
                double, double, double, double,
                double, uint32_t, uint32_t, char [],
                uint32_t, uint32_t, uint32_t, uint32_t, double,
                 char [], uint32_t, uint32_t, double,
                char, char, int32_t, char [], uint32_t,
                 char [], uint32_t, sondeaprs_pSATSIG,
                struct sondeaprs_SDRBLOCK);

extern int32_t sondeaprs_GetIp(char [], uint32_t, uint32_t *,
                uint32_t *, uint32_t *);

extern char sondeaprs_via[100];

extern char sondeaprs_destcall[100];

extern char sondeaprs_objname[100];

extern char sondeaprs_commentfn[1025];

extern char sondeaprs_csvfilename[1025];

extern char sondeaprs_sym[2];

extern uint32_t sondeaprs_toport;

extern uint32_t sondeaprs_maxsenddistance;

extern uint32_t sondeaprs_expire;

extern uint32_t sondeaprs_ipnum;

extern char sondeaprs_verb;

extern char sondeaprs_verb2;

extern char sondeaprs_nofilter;

extern char sondeaprs_json;

extern int32_t sondeaprs_comptyp;

extern int32_t sondeaprs_udpsock;

extern char sondeaprs_anyip;

extern char sondeaprs_sendmon;

extern char sondeaprs_dao;

extern struct aprsstr_POSITION sondeaprs_mypos;

extern float sondeaprs_myalt;

struct sondeaprs__D1;


struct sondeaprs__D1 {
   uint32_t beacontime;
   uint32_t below;
};

extern struct sondeaprs__D1 sondeaprs_beacontimes[20];

struct sondeaprs__D2;


struct sondeaprs__D2 {
   struct aprsstr_POSITION leftdown;
   struct aprsstr_POSITION rightup;
};

extern struct sondeaprs__D2 sondeaprs_rectfence;


extern void sondeaprs_BEGIN(void);


#endif /* sondeaprs_H_ */
