/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef aprsdecode_H_
#define aprsdecode_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef xosi_H_
#include "xosi.h"
#endif

/* connect to aprs-is gateway and decode data for archive and map by OE5DXL */
typedef uint32_t aprsdecode_SET256[8];

#define aprsdecode_CR "\015"

#define aprsdecode_LF "\012"

#define aprsdecode_DEGSYM "\177"

#define aprsdecode_DELETSYM "\001"

#define aprsdecode_WXNIL 1.E+6

#define aprsdecode_MSGSIZE 67

#define aprsdecode_ACKSIZE 5

#define aprsdecode_REPLYACK "}"

#define aprsdecode_UDPSOCKS 4

#define aprsdecode_DESTCALL "APLM01"

#define aprsdecode_SHOWTEMPWIND 3600
/* till seconds after last wx info show temp or windvane */

#define aprsdecode_MAXTEMP 99
/* ignore out of range temps */

#define aprsdecode_MINTEMP (-99)
/* ignore out of range temps */

#define aprsdecode_IGATEMAXDELAY 5
/* seconds to discard igate frames if mainloop is too slow */

#define aprsdecode_BEGINOFTIME 1388534400
/* oldest possible log date */

#define aprsdecode_MAXMULTILINES 40
/* AE5PL limits to 23 multiline elements */

extern aprsdecode_SET256 aprsdecode_SYMTABLE;

#define aprsdecode_POIINFOSIZE 4096

#define aprsdecode_VERSNUM "0.77"

#define aprsdecode_DEFAULTPOISYMBOL "//"

#define aprsdecode_VERS "aprsmap(cu) 0.77"

typedef char aprsdecode_MONCALL[9];

typedef char aprsdecode_FRAMEBUF[512];

typedef char aprsdecode_WWWB[1401];

typedef char aprsdecode_FILENAME[1024];

struct aprsdecode_POSCALL;


struct aprsdecode_POSCALL {
   aprsdecode_MONCALL call;
   char typ;
   struct aprsstr_POSITION pos;
};

struct aprsdecode_WWWBUF;

typedef struct aprsdecode_WWWBUF * aprsdecode_pWWWBUF;


struct aprsdecode_WWWBUF {
   aprsdecode_WWWB buf;
   int32_t tlen;
   char push;
   aprsdecode_pWWWBUF next;
};

struct aprsdecode_QWatch;


struct aprsdecode_QWatch {
   uint32_t lasttb;
   int32_t qsize;
   int32_t lastqb;
   int32_t txbyte[60];
};

struct aprsdecode_TCPSOCK;

typedef struct aprsdecode_TCPSOCK * aprsdecode_pTCPSOCK;


struct aprsdecode_TCPSOCK {
   aprsdecode_pTCPSOCK next;
   int32_t fd;
   uint32_t watchtime;
   uint32_t beacont;
   uint32_t connt;
   uint32_t gatepri;
   char ipnum[64];
   char port[6];
   struct aprsdecode_POSCALL user;
   uint32_t txframes;
   uint32_t txbytes;
   uint32_t rxframes;
   uint32_t rxbytes;
   int32_t rpos;
   int32_t tlen;
   aprsdecode_FRAMEBUF rbuf;
   aprsdecode_FRAMEBUF tbuf;
   struct aprsdecode_QWatch qwatch;
   uint8_t waitpong;
   uint32_t lastpong;
   uint32_t lastping;
   uint32_t windoofwatch;
};

typedef uint32_t aprsdecode_DUPETIMES[65536];

struct aprsdecode_UDPSOCK;


struct aprsdecode_UDPSOCK {
   int32_t fd;
   char rawread;
   char checkip;
   uint32_t ip;
   uint32_t bindport;
   uint32_t dport;
   int32_t chan; /* channel number 1.. */
   uint32_t lastudprx;
   uint32_t lastudptx;
   uint32_t starttime;
   uint32_t bpstime;
   uint32_t rxframes;
   uint32_t rxbytes;
   uint32_t txframes;
   uint32_t txbytes; /* for tx bit/s limiter */
   uint32_t uip;
   uint32_t * pdupetimes;
};

typedef char aprsdecode_MSGTEXT[67];

typedef char aprsdecode_ACKTEXT[5];

typedef char aprsdecode_ACKREPTEXT[2];

enum aprsdecode_ERRFLAGS {aprsdecode_eDIST, aprsdecode_eSPIKE,
                aprsdecode_eSYMB, aprsdecode_eDUPE, aprsdecode_eSPEED,
                aprsdecode_eNODRAW, aprsdecode_eNOPOS};


typedef uint8_t aprsdecode_ERRSET;

enum aprsdecode_DRAWHINTS {aprsdecode_MIRRORSYM, aprsdecode_ISOBJECT,
                aprsdecode_HIDE, aprsdecode_MARKED, aprsdecode_MOVES};


typedef uint8_t aprsdecode_sDRAWHINTS;

struct aprsdecode_SYMBOL;


struct aprsdecode_SYMBOL {
   char tab;
   char pic;
};

struct aprsdecode_AREASYMB;


struct aprsdecode_AREASYMB {
   char typ;
   uint8_t color;
   struct aprsstr_POSITION dpos;
};

struct aprsdecode_FRAMEHIST;

typedef struct aprsdecode_FRAMEHIST * aprsdecode_pFRAMEHIST;

struct aprsdecode_VARDAT;

typedef struct aprsdecode_VARDAT * aprsdecode_pVARDAT;


struct aprsdecode_FRAMEHIST {
   aprsdecode_pFRAMEHIST next;
   uint32_t time0;
   aprsdecode_pVARDAT vardat;
   uint8_t nodraw;
};


struct aprsdecode_VARDAT {
   aprsdecode_pFRAMEHIST lastref;
   struct aprsstr_POSITION pos;
   uint32_t refcnt; /* starts at 0 */
   /*             altitude :INT16;*/
   uint8_t igatepos;
   uint8_t igatelen;
   char raw[500]; /* last part allocatet with real size */
};

typedef signed char aprsdecode_CLBTYP;

struct aprsdecode_OPHIST;

typedef struct aprsdecode_OPHIST * aprsdecode_pOPHIST;


struct aprsdecode_OPHIST {
   aprsdecode_pOPHIST next; /* symbol call chain */
   aprsdecode_pFRAMEHIST frames; /* beacons with this call */
   aprsdecode_pFRAMEHIST lastfrp;
                /* log import quick write pointer, no compress */
   struct aprsstr_POSITION margin0; /* movement frame for quick draw */
   struct aprsstr_POSITION margin1;
   struct aprsstr_POSITION lastpos;
   uint32_t lasttime;
   uint32_t temptime;
   short lastkmh; /* drive kmh, wx kmh */
   short lasttempalt; /* drive altitude, wx temp */
   float framerate; /* thruput frames/s */
   aprsdecode_MONCALL call;
   struct aprsdecode_SYMBOL sym;
   uint8_t drawhints;
   signed char clb;
   signed char trackcol;
   signed char textpos;
   signed char valuepos;
   uint8_t lastinftyp; /*0 no inf, 10..99 drive dir, 100 wx no dir 110..199 wind dir*/
   char lastrxport; /* for msg routing, in future remove if port is seen in rawframes */
   struct aprsdecode_AREASYMB areasymb; /* area object data */
   char poligon;
   char dirty; /* Checktrack not done */
};

enum aprsdecode_STORM {aprsdecode_WXNOWX, aprsdecode_WXNORMAL,
                aprsdecode_WXTS, aprsdecode_WXHC, aprsdecode_WXTD};


struct aprsdecode_WX;


struct aprsdecode_WX {
   float gust;
   float temp;
   float hygro;
   float baro;
   float rain1;
   float rain24;
   float raintoday;
   float lum;
   float sievert;
   uint8_t storm;
   float sustaind;
   float radiushurr;
   float radiusstorm;
   float wholegale;
};

enum aprsdecode_TYPES {aprsdecode_UNKNOWN, aprsdecode_MICE, aprsdecode_POS,
                aprsdecode_WETH, aprsdecode_PWETH, aprsdecode_MSG,
                aprsdecode_OBJ, aprsdecode_ITEM, aprsdecode_SCAP,
                aprsdecode_STAT, aprsdecode_TELE, aprsdecode_QUERY};


typedef uint16_t aprsdecode_TYPSET;

enum aprsdecode_ACKREJ {aprsdecode_MSGMSG, aprsdecode_MSGACK,
                aprsdecode_MSGREJ};


struct aprsdecode_HRTPOS;


struct aprsdecode_HRTPOS {
   float dlong;
   float dlat;
   float dalt;
   uint32_t dtime;
   char notinterpolated;
};

struct aprsdecode_MULTILINE;


struct aprsdecode_MULTILINE {
   uint32_t size;
   char linetyp;
   char filltyp;
   struct aprsstr_POSITION vec[41];
};

typedef uint16_t aprsdecode_TELEMETRY[7];

struct aprsdecode_DAT;


struct aprsdecode_DAT {
   aprsdecode_MONCALL srcall;
   aprsdecode_MONCALL symcall;
   aprsdecode_MONCALL dstcall;
   aprsdecode_MONCALL viacalls[10];
   aprsdecode_MONCALL msgto;
   aprsdecode_MONCALL objectfrom;
   uint32_t hbitp;
   uint32_t igaterawp;
   uint32_t igatelen;
   uint32_t igatep;
   uint32_t payload;
   char postyp;
   char typc;
   struct aprsdecode_AREASYMB areasymb;
   struct aprsstr_POSITION pos;
   uint32_t speed;
   uint32_t course;
   int32_t climb;
   int32_t altitude;
   char sym;
   char symt;
   char timestamp;
   char objkill;
   char lastrxport;
   struct aprsdecode_WX wx;
   uint8_t type;
   float moved;
   /*
               stack      : CARDINAL;
   */
   aprsdecode_MSGTEXT msgtext;
   aprsdecode_ACKTEXT acktext;
   uint8_t ackrej;
   struct aprsdecode_HRTPOS hrtposes[32];
   uint32_t hrtlen;
   uint32_t hrttime; /* if non zero is hrt defined */
   struct aprsdecode_MULTILINE multiline;
   aprsdecode_TELEMETRY tlmvalues;
                /* 5 analog and if all 5 valid the sixth is bitset, 0 undef,
                incremented by 1 */
   char comment0[256];
};

enum aprsdecode_WXSET {aprsdecode_wTEMP, aprsdecode_wBARO, aprsdecode_wHYG,
                aprsdecode_wWIND, aprsdecode_wWINDDIR, aprsdecode_wRAIN,
                aprsdecode_wLUMI, aprsdecode_wSHIST, aprsdecode_wBHIST,
                aprsdecode_wAHIST, aprsdecode_wSIEV};


typedef uint16_t aprsdecode_sWXSET;

enum aprsdecode_CLICKTYPS {aprsdecode_tTEXT, aprsdecode_tTRACK,
                aprsdecode_tSYMBOL, aprsdecode_tMAP, aprsdecode_tOBJECT,
                aprsdecode_tOBJECTTEXT, aprsdecode_tRFPATH,
                aprsdecode_tKMHOBJ, aprsdecode_tKMH, 
   aprsdecode_tDEGREE, aprsdecode_tDEGREEOBJ, aprsdecode_tMETEOCOLOR};


struct aprsdecode_TXMESSAGE;

typedef struct aprsdecode_TXMESSAGE * aprsdecode_pTXMESSAGE;


struct aprsdecode_TXMESSAGE {
   aprsdecode_pTXMESSAGE next;
   uint32_t txtime;
   uint32_t acktime;
   uint32_t txcnt;
   char port;
   aprsdecode_MONCALL to;
   aprsdecode_MSGTEXT msgtext;
   uint32_t acknum;
   char heard;
   char rej;
};

struct aprsdecode_MSGFIFO;

typedef struct aprsdecode_MSGFIFO * aprsdecode_pMSGFIFO;


struct aprsdecode_MSGFIFO {
   aprsdecode_pMSGFIFO next;
   uint32_t time0;
   char port;
   aprsdecode_MONCALL itemname;
   aprsdecode_MONCALL from;
   aprsdecode_MONCALL to;
   aprsdecode_MSGTEXT txt;
   aprsdecode_ACKTEXT ack;
   char query;
   char deleteitem;
   struct aprsstr_POSITION itempos;
};

struct aprsdecode_ZOOMFRAME;


struct aprsdecode_ZOOMFRAME {
   int32_t x0;
   int32_t y00;
   int32_t x1;
   int32_t y1;
};

struct aprsdecode_COLTYP;


struct aprsdecode_COLTYP {
   uint32_t r;
   uint32_t g;
   uint32_t b;
};

typedef char * aprsdecode_pMOUNTAININFO;

struct aprsdecode_MOUNTAIN;

typedef struct aprsdecode_MOUNTAIN * aprsdecode_pMOUNTAIN;


struct aprsdecode_MOUNTAIN {
   aprsdecode_pMOUNTAIN next;
   aprsdecode_pMOUNTAININFO pinfo;
   struct aprsstr_POSITION pos;
   short alt;
   uint16_t index;
   char name[32];
   char category[10];
};

struct aprsdecode_CLICKOBJECT;


struct aprsdecode_CLICKOBJECT {
   aprsdecode_pOPHIST opf;
   aprsdecode_pFRAMEHIST pff;
   aprsdecode_pFRAMEHIST pff0;
   uint8_t typf;
};

typedef char aprsdecode_MAPNAME[41];

typedef uint32_t aprsdecode_SYMBOLSET[6];

typedef uint32_t aprsdecode_MAPGAMMATAB[257];

extern uint32_t aprsdecode_systime;

extern uint32_t aprsdecode_realtime;

extern uint32_t aprsdecode_lastlooped;

extern uint32_t aprsdecode_rxidle;

extern char aprsdecode_quit;

extern char aprsdecode_verb;

extern char aprsdecode_logdone;

struct aprsdecode__D0;


struct aprsdecode__D0 {
   char dryrun;
   char withradio;
   char chkmaps;
   char panorama;
   char altimap;
   char watchmhop;
   char lastpoi;
   char insreplaceline;
   char watchlast;
   char abort0;
   int32_t x;
   int32_t y;
   int32_t min0;
   aprsdecode_MONCALL mhop;
   struct aprsdecode_SYMBOL onesymbol;
   aprsdecode_SYMBOLSET onesymbolset;
   aprsdecode_pOPHIST ops;
   aprsdecode_pFRAMEHIST pf;
   aprsdecode_pFRAMEHIST pf0;
   int32_t zoomtox;
   int32_t zoomtoy;
   uint8_t typ;
   char cmd;
   char cmdatt;
   struct aprsstr_POSITION pullpos;
   struct aprsstr_POSITION clickpos;
   struct aprsstr_POSITION squerpos0;
   struct aprsstr_POSITION squerpos1;
   struct aprsstr_POSITION squerspos0;
   struct aprsstr_POSITION squerspos1;
   struct aprsstr_POSITION measurepos;
   struct aprsstr_POSITION markpos;
   uint32_t markpost; /* waypoint time to markpos to make uniqe */
   uint32_t marktime; /* delete marker if no set by set marker */
   int32_t markalti; /* for geoprofile */
   struct aprsstr_POSITION sumpos; /* store last clickpos for waysum */
   float waysum; /* sum up marker set distances */
   uint16_t graphset; /* update graphs if marked last waypoint */
   uint32_t selected;
   uint32_t entries;
   struct aprsstr_POSITION bubblpos; /* found position of POI */
   char bubblstr[50]; /* found text of POI */
   char bubblinfo[4096]; /* found additional info of POI */
   struct aprsdecode_CLICKOBJECT table[10];
   uint32_t polilinecursor; /* edit which edge of polinine object */
};

extern struct aprsdecode__D0 aprsdecode_click;

extern struct aprsstr_POSITION aprsdecode_mappos;

extern int32_t aprsdecode_inittilex;

extern int32_t aprsdecode_inittiley;

extern int32_t aprsdecode_initxsize;

extern int32_t aprsdecode_initysize;

extern int32_t aprsdecode_parmzoom;

extern int32_t aprsdecode_initzoom;

extern float aprsdecode_finezoom;

extern float aprsdecode_parmfinezoom;

extern char aprsdecode_mapdir[1025];

extern aprsdecode_pMOUNTAIN aprsdecode_mountains;

struct aprsdecode__D1;


struct aprsdecode__D1 {
   uint32_t count;
   char on;
   char symbol[2];
   char name[10];
};

extern struct aprsdecode__D1 aprsdecode_poifiles[30];

struct aprsdecode__D2;


struct aprsdecode__D2 {
   char moving;
   char moded;
   char errorstep;
   char logmode;
   char headmenuy;
   aprsdecode_MAPNAME mapname;
   uint32_t mapnum;
   char configfn[257];
   aprsdecode_MAPGAMMATAB maplumcorr;
   int32_t map;
   int32_t rf;
   int32_t rfbri;
   int32_t track;
   int32_t waypoint;
   int32_t sym;
   int32_t obj;
   int32_t text;
   int32_t nomov;
   int32_t centering;
   int32_t fps;
   int32_t actfps;
   uint32_t symsize;
   uint32_t fontxsize;
   uint32_t fontysize;
   uint32_t fontsymbolpercent;
   char wxcol;
   uint32_t movestep;
   uint32_t dupemaxtime;
   uint32_t kmhtime;
   uint32_t purgetime;
   uint32_t purgetimeobj;
   uint32_t firstdim;
   uint32_t maxdim;
   float gamma;
   char menutransp;
   struct aprsdecode_COLTYP menubackcol;
   struct aprsdecode_COLTYP menucol;
};

extern struct aprsdecode__D2 aprsdecode_lums;

struct aprsdecode__D3;


struct aprsdecode__D3 {
   struct aprsstr_POSITION winpos0;
   struct aprsstr_POSITION winpos1;
   aprsdecode_MONCALL call;
   uint32_t winevent;
   char follow;
   char beep;
};

extern struct aprsdecode__D3 aprsdecode_tracenew;

extern aprsdecode_pTXMESSAGE aprsdecode_txmessages;

extern aprsdecode_pMSGFIFO aprsdecode_msgfifo0;

extern float aprsdecode_spikesens;

extern float aprsdecode_maxhop;

extern aprsdecode_pOPHIST aprsdecode_ophist2;

extern aprsdecode_pOPHIST aprsdecode_ophist0;

struct aprsdecode__D4;


struct aprsdecode__D4 {
   char url[256];
   char port[6];
   char filterst[256];
};
/*
    servercall            : MONCALL;
*/

extern struct aprsdecode__D4 aprsdecode_gateways[10];

extern aprsdecode_pTCPSOCK aprsdecode_tcpsocks;

extern struct aprsdecode_UDPSOCK aprsdecode_udpsocks0[4];

extern uint32_t aprsdecode_lasttcptx;

extern uint32_t aprsdecode_lastanyudprx;

extern uint32_t aprsdecode_lastpurge;

extern uint32_t aprsdecode_lasttcprx;

extern aprsdecode_FRAMEBUF aprsdecode_testbeaconbuf;
                /* fill with port:time:beacon then sent once */

extern struct xosi_PROCESSHANDLE aprsdecode_checkversion;

extern struct xosi_PROCESSHANDLE aprsdecode_serialpid;

extern struct xosi_PROCESSHANDLE aprsdecode_serialpid2;

extern struct xosi_PROCESSHANDLE aprsdecode_maploadpid;

extern int32_t aprsdecode_Decode(char [], uint32_t,
                struct aprsdecode_DAT *);

extern int32_t aprsdecode_Stoframe(aprsdecode_pOPHIST *, char [],
                uint32_t, uint32_t, char, uint32_t *,
                struct aprsdecode_DAT);

/*
PROCEDURE Checktrack(op:pOPHIST; lastf:pFRAMEHIST);
*/
extern void aprsdecode_Checktracks(void);

extern void aprsdecode_purge(aprsdecode_pOPHIST *, uint32_t, uint32_t);

extern int32_t aprsdecode_knottokmh(int32_t);

extern uint32_t aprsdecode_trunc(float);

extern float aprsdecode_floor(float);

extern int32_t aprsdecode_tcpout(aprsdecode_pTCPSOCK);

extern void aprsdecode_tcpin(aprsdecode_pTCPSOCK);

extern void aprsdecode_udpin(uint32_t);

extern void aprsdecode_tcpclose(aprsdecode_pTCPSOCK, char);

extern void aprsdecode_tcpjobs(void);

extern void aprsdecode_initparms(void);

extern void aprsdecode_delwaypoint(aprsdecode_pOPHIST,
                aprsdecode_pFRAMEHIST *);

extern uint32_t aprsdecode_finddup(aprsdecode_pFRAMEHIST,
                aprsdecode_pFRAMEHIST);

extern void aprsdecode_savetrack(void);

extern void aprsdecode_makemsg(char);

extern void aprsdecode_deltxmsg(uint32_t, uint32_t);

extern void aprsdecode_acknumstr(char [], uint32_t, uint32_t);

extern void aprsdecode_getactack(aprsdecode_MONCALL, char [],
                uint32_t);

extern void aprsdecode_objsender(aprsdecode_pOPHIST, char [],
                uint32_t);

extern aprsdecode_pOPHIST aprsdecode_selop(void);

extern void aprsdecode_tcpconnstat(char [], uint32_t);

extern void aprsdecode_udpconnstat(uint32_t, char [], uint32_t);

extern void aprsdecode_beeplim(int32_t, uint32_t, uint32_t);

extern void aprsdecode_importbeacon(void);

extern void aprsdecode_extractbeacon(char [], uint32_t, char,
                char);

extern void aprsdecode_getbeaconname(char [], uint32_t, char [],
                uint32_t, char [], uint32_t, char *,
                char *, char *);

extern void aprsdecode_drawbeacon(char [], uint32_t);

extern void aprsdecode_makelogfn(char [], uint32_t);
/*
PROCEDURE checksymb(symt, symb:CHAR):BOOLEAN;
                (* true for bad symbol *)
*/

extern void aprsdecode_appendmultiline(struct aprsstr_POSITION);

extern void aprsdecode_GetMultiline(char [], uint32_t, uint32_t *,
                struct aprsdecode_MULTILINE *);

extern char aprsdecode_ismultiline(char);

extern void aprsdecode_modmultiline(uint32_t);

extern void aprsdecode_Stopticker(void);


extern void aprsdecode_BEGIN(void);


#endif /* aprsdecode_H_ */
