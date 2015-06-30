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
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef xosi_H_
#include "xosi.h"
#endif

/* connect to aprs-is gateway and decode data for archive and map by OE5DXL */
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

#define aprsdecode_VERS "aprsmap(cu) 0.50"

typedef char aprsdecode_MONCALL[9];

typedef char aprsdecode_FRAMEBUF[512];

typedef char aprsdecode_WWWB[1401];

typedef char aprsdecode_FILENAME[1024];

struct aprsdecode_POSCALL;


struct aprsdecode_POSCALL {
   aprsdecode_MONCALL call;
   char typ;
   struct aprspos_POSITION pos;
};

struct aprsdecode_WWWBUF;

typedef struct aprsdecode_WWWBUF * aprsdecode_pWWWBUF;


struct aprsdecode_WWWBUF {
   aprsdecode_WWWB buf;
   long tlen;
   char push;
   aprsdecode_pWWWBUF next;
};

struct aprsdecode_QWatch;


struct aprsdecode_QWatch {
   unsigned long lasttb;
   long qsize;
   long lastqb;
   long txbyte[60];
};

struct aprsdecode_TCPSOCK;

typedef struct aprsdecode_TCPSOCK * aprsdecode_pTCPSOCK;


struct aprsdecode_TCPSOCK {
   aprsdecode_pTCPSOCK next;
   long fd;
   unsigned long watchtime;
   unsigned long beacont;
   unsigned long connt;
   unsigned long gatepri;
   char ipnum[64];
   char port[6];
   struct aprsdecode_POSCALL user;
   unsigned long txframes;
   unsigned long txbytes;
   unsigned long rxframes;
   unsigned long rxbytes;
   long rpos;
   long tlen;
   aprsdecode_FRAMEBUF rbuf;
   aprsdecode_FRAMEBUF tbuf;
   struct aprsdecode_QWatch qwatch;
   unsigned char waitpong;
   unsigned long lastpong;
   unsigned long lastping;
   unsigned long windoofwatch;
};

typedef unsigned long aprsdecode_DUPETIMES[65536];

struct aprsdecode_UDPSOCK;


struct aprsdecode_UDPSOCK {
   long fd;
   char rawread;
   char checkip;
   unsigned long ip;
   unsigned long bindport;
   unsigned long dport;
   long chan; /* channel number 1.. */
   unsigned long lastudprx;
   unsigned long lastudptx;
   unsigned long starttime;
   unsigned long rxframes;
   unsigned long rxbytes;
   unsigned long txframes;
   unsigned long txbytes;
   unsigned long uip;
   unsigned long * pdupetimes;
};

typedef char aprsdecode_MSGTEXT[67];

typedef char aprsdecode_ACKTEXT[5];

typedef char aprsdecode_ACKREPTEXT[2];

enum aprsdecode_ERRFLAGS {aprsdecode_eDIST, aprsdecode_eSPIKE,
                aprsdecode_eSYMB, aprsdecode_eDUPE, aprsdecode_eSPEED,
                aprsdecode_eNODRAW, aprsdecode_eNOPOS};


typedef unsigned char aprsdecode_ERRSET;

enum aprsdecode_DRAWHINTS {aprsdecode_MIRRORSYM, aprsdecode_ISOBJECT,
                aprsdecode_HIDE, aprsdecode_MARKED, aprsdecode_MOVES};


typedef unsigned char aprsdecode_sDRAWHINTS;

struct aprsdecode_SYMBOL;


struct aprsdecode_SYMBOL {
   char tab;
   char pic;
};

struct aprsdecode_FRAMEHIST;

typedef struct aprsdecode_FRAMEHIST * aprsdecode_pFRAMEHIST;

struct aprsdecode_VARDAT;

typedef struct aprsdecode_VARDAT * aprsdecode_pVARDAT;


struct aprsdecode_FRAMEHIST {
   aprsdecode_pFRAMEHIST next;
   unsigned long time0;
   aprsdecode_pVARDAT vardat;
   unsigned char nodraw;
};


struct aprsdecode_VARDAT {
   aprsdecode_pFRAMEHIST lastref;
   struct aprspos_POSITION pos;
   unsigned short refcnt; /* starts at 0 */
   /*             altitude :INT16;*/
   unsigned char igatepos;
   unsigned char igatelen;
   char raw[500]; /* last part allocatet with real size */
};

struct aprsdecode_OPHIST;

typedef struct aprsdecode_OPHIST * aprsdecode_pOPHIST;


struct aprsdecode_OPHIST {
   aprsdecode_pOPHIST next;
   aprsdecode_pFRAMEHIST frames;
   struct aprspos_POSITION margin0; /* movement frame for quick draw */
   struct aprspos_POSITION margin1;
   struct aprspos_POSITION lastpos;
   unsigned long lasttime;
   unsigned long temptime;
   short lastkmh; /* drive kmh, wx kmh */
   short lasttempalt; /* drive altitude, wx temp */
   float framerate; /* thruput frames/s */
   aprsdecode_MONCALL call;
   struct aprsdecode_SYMBOL sym;
   unsigned char drawhints;
   signed char trackcol;
   signed char textpos;
   signed char valuepos;
   unsigned char lastinftyp; /*0 no inf, 10..99 drive dir, 100 wx no dir 110..199 wind dir*/
};

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
};

enum aprsdecode_TYPES {aprsdecode_UNKNOWN, aprsdecode_MICE, aprsdecode_POS,
                aprsdecode_WETH, aprsdecode_PWETH, aprsdecode_MSG,
                aprsdecode_OBJ, aprsdecode_ITEM, aprsdecode_SCAP,
                aprsdecode_STAT, aprsdecode_TELE, aprsdecode_QUERY};


typedef unsigned short aprsdecode_TYPSET;

enum aprsdecode_ACKREJ {aprsdecode_MSGMSG, aprsdecode_MSGACK,
                aprsdecode_MSGREJ};


struct aprsdecode_HRTPOS;


struct aprsdecode_HRTPOS {
   float dlong;
   float dlat;
   float dalt;
   unsigned long dtime;
   char notinterpolated;
};

struct aprsdecode_DAT;


struct aprsdecode_DAT {
   aprsdecode_MONCALL srccall;
   aprsdecode_MONCALL dstcall;
   aprsdecode_MONCALL viacalls[10];
   aprsdecode_MONCALL msgto;
   aprsdecode_MONCALL objectfrom;
   unsigned long hbitp;
   unsigned long igaterawp;
   unsigned long igatelen;
   unsigned long igatep;
   char postyp;
   char typc;
   struct aprspos_POSITION pos;
   unsigned long speed;
   unsigned long course;
   long altitude;
   char sym;
   char symt;
   char timestamp;
   char objkill;
   struct aprsdecode_WX wx;
   unsigned char type;
   float moved;
   /*
               stack      : CARDINAL;
   */
   aprsdecode_MSGTEXT msgtext;
   aprsdecode_ACKTEXT acktext;
   unsigned char ackrej;
   struct aprsdecode_HRTPOS hrtposes[32];
   unsigned long hrtlen;
   unsigned long hrttime; /* if non zero is hrt defined */
   char comment0[256];
};

enum aprsdecode_WXSET {aprsdecode_wTEMP, aprsdecode_wBARO, aprsdecode_wHYG,
                aprsdecode_wWIND, aprsdecode_wWINDDIR, aprsdecode_wRAIN,
                aprsdecode_wLUMI, aprsdecode_wSHIST, aprsdecode_wBHIST,
                aprsdecode_wAHIST, aprsdecode_wSIEV};


typedef unsigned short aprsdecode_sWXSET;

enum aprsdecode_CLICKTYPS {aprsdecode_tTEXT, aprsdecode_tTRACK,
                aprsdecode_tSYMBOL, aprsdecode_tMAP, aprsdecode_tOBJECT,
                aprsdecode_tOBJECTTEXT, aprsdecode_tRFPATH,
                aprsdecode_tKMHOBJ, aprsdecode_tKMH, 
   aprsdecode_tDEGREE, aprsdecode_tDEGREEOBJ, aprsdecode_tMETEOCOLOR};


struct aprsdecode_TXMESSAGE;

typedef struct aprsdecode_TXMESSAGE * aprsdecode_pTXMESSAGE;


struct aprsdecode_TXMESSAGE {
   aprsdecode_pTXMESSAGE next;
   unsigned long txtime;
   unsigned long acktime;
   unsigned long txcnt;
   char port;
   aprsdecode_MONCALL to;
   aprsdecode_MSGTEXT msgtext;
   unsigned long acknum;
   char heard;
   char rej;
};

struct aprsdecode_MSGFIFO;

typedef struct aprsdecode_MSGFIFO * aprsdecode_pMSGFIFO;


struct aprsdecode_MSGFIFO {
   aprsdecode_pMSGFIFO next;
   unsigned long time0;
   char port;
   aprsdecode_MONCALL from;
   aprsdecode_MONCALL to;
   aprsdecode_MSGTEXT txt;
   aprsdecode_ACKTEXT ack;
   char query;
};

struct aprsdecode_ZOOMFRAME;


struct aprsdecode_ZOOMFRAME {
   long x0;
   long y00;
   long x1;
   long y1;
};

struct aprsdecode_COLTYP;


struct aprsdecode_COLTYP {
   unsigned long r;
   unsigned long g;
   unsigned long b;
};

struct aprsdecode_MOUNTAIN;

typedef struct aprsdecode_MOUNTAIN * aprsdecode_pMOUNTAIN;


struct aprsdecode_MOUNTAIN {
   aprsdecode_pMOUNTAIN next;
   struct aprspos_POSITION pos;
   short alt;
   char name[32];
};

struct aprsdecode_CLICKOBJECT;


struct aprsdecode_CLICKOBJECT {
   aprsdecode_pOPHIST opf;
   aprsdecode_pFRAMEHIST pff;
   aprsdecode_pFRAMEHIST pff0;
   unsigned char typf;
};

typedef char aprsdecode_MAPNAME[41];

extern unsigned long aprsdecode_systime;

extern unsigned long aprsdecode_realtime;

extern unsigned long aprsdecode_lastlooped;

extern unsigned long aprsdecode_rxidle;

extern char aprsdecode_verb;

struct aprsdecode__D0;


struct aprsdecode__D0 {
   char dryrun;
   char withradio;
   char panorama;
   char altimap;
   char watchmhop;
   char lastpoi;
   char watchlast;
   long x;
   long y;
   long min0;
   aprsdecode_MONCALL mhop;
   struct aprsdecode_SYMBOL onesymbol;
   aprsdecode_pOPHIST ops;
   aprsdecode_pFRAMEHIST pf;
   aprsdecode_pFRAMEHIST pf0;
   long zoomtox;
   long zoomtoy;
   unsigned char typ;
   char cmd;
   char cmdatt;
   struct aprspos_POSITION pullpos;
   struct aprspos_POSITION clickpos;
   struct aprspos_POSITION squerpos0;
   struct aprspos_POSITION squerpos1;
   struct aprspos_POSITION measurepos;
   struct aprspos_POSITION markpos;
   unsigned long markpost; /* waypoint time to markpos to make uniqe */
   unsigned long marktime; /* delete marker if no set by set marker */
   long markalti; /* for geoprofile */
   struct aprspos_POSITION sumpos; /* store last clickpos for waysum */
   float waysum; /* sum up marker set distances */
   unsigned short graphset; /* update graphs if marked last waypoint */
   unsigned long selected;
   unsigned long entries;
   struct aprspos_POSITION bubblpos; /* found position of POI */
   char bubblstr[50]; /* found text of POI */
   struct aprsdecode_CLICKOBJECT table[10];
};

extern struct aprsdecode__D0 aprsdecode_click;

extern struct aprspos_POSITION aprsdecode_mappos;

extern long aprsdecode_inittilex;

extern long aprsdecode_inittiley;

extern long aprsdecode_initxsize;

extern long aprsdecode_initysize;

extern long aprsdecode_parmzoom;

extern long aprsdecode_initzoom;

extern float aprsdecode_finezoom;

extern float aprsdecode_parmfinezoom;

extern char aprsdecode_mapdir[1025];

extern aprsdecode_pMOUNTAIN aprsdecode_mountains;

struct aprsdecode__D1;


struct aprsdecode__D1 {
   char moving;
   char moded;
   char errorstep;
   char logmode;
   char headmenuy;
   aprsdecode_MAPNAME mapname;
   char configfn[257];
   long maplumcorr;
   long map;
   long rf;
   long rfbri;
   long track;
   long waypoint;
   long sym;
   long obj;
   long text;
   long nomov;
   long centering;
   long fps;
   long actfps;
   unsigned long fontysize;
   char wxcol;
   unsigned long movestep;
   unsigned long dupemaxtime;
   unsigned long kmhtime;
   unsigned long purgetime;
   unsigned long purgetimeobj;
   unsigned long firstdim;
   unsigned long maxdim;
   float gamma;
   char menutransp;
   struct aprsdecode_COLTYP menubackcol;
   struct aprsdecode_COLTYP menucol;
};

extern struct aprsdecode__D1 aprsdecode_lums;

struct aprsdecode__D2;


struct aprsdecode__D2 {
   struct aprspos_POSITION winpos0;
   struct aprspos_POSITION winpos1;
   aprsdecode_MONCALL call;
   unsigned long winevent;
   char follow;
   char beep;
};

extern struct aprsdecode__D2 aprsdecode_tracenew;

extern aprsdecode_pTXMESSAGE aprsdecode_txmessages;

extern aprsdecode_pMSGFIFO aprsdecode_msgfifo0;

extern float aprsdecode_spikesens;

extern float aprsdecode_maxhop;

extern aprsdecode_pOPHIST aprsdecode_ophist2;

extern aprsdecode_pOPHIST aprsdecode_ophist0;

struct aprsdecode__D3;


struct aprsdecode__D3 {
   char url[256];
   char port[6];
   char filterst[256];
};
/*
    servercall            : MONCALL;
*/

extern struct aprsdecode__D3 aprsdecode_gateways[10];

extern aprsdecode_pTCPSOCK aprsdecode_tcpsocks;

extern struct aprsdecode_UDPSOCK aprsdecode_udpsocks0[4];

extern unsigned long aprsdecode_lasttcptx;

extern unsigned long aprsdecode_lastanyudprx;

extern unsigned long aprsdecode_lastpurge;

extern unsigned long aprsdecode_lasttcprx;

extern aprsdecode_FRAMEBUF aprsdecode_testbeaconbuf;
                /* fill with port:time:beacon then sent once */

extern struct xosi_PROCESSHANDLE aprsdecode_serialpid;

extern struct xosi_PROCESSHANDLE aprsdecode_serialpid2;

extern struct xosi_PROCESSHANDLE aprsdecode_maploadpid;

extern long aprsdecode_Decode(char [], unsigned long,
                struct aprsdecode_DAT *);

extern long aprsdecode_Stoframe(aprsdecode_pOPHIST *, char [], unsigned long,
                 unsigned long, char, struct aprsdecode_DAT);

extern void aprsdecode_Checktrack(aprsdecode_pOPHIST, aprsdecode_pFRAMEHIST);

extern void aprsdecode_purge(aprsdecode_pOPHIST *, unsigned long,
                unsigned long);

extern long aprsdecode_knottokmh(long);

extern void aprsdecode_posinval(struct aprspos_POSITION *);

extern unsigned long aprsdecode_trunc(float);

extern float aprsdecode_floor(float);

extern long aprsdecode_tcpout(aprsdecode_pTCPSOCK);

extern void aprsdecode_tcpin(aprsdecode_pTCPSOCK);

extern void aprsdecode_udpin(unsigned long);

extern void aprsdecode_tcpclose(aprsdecode_pTCPSOCK, char);

extern void aprsdecode_tcpjobs(void);

extern void aprsdecode_initparms(void);

extern void aprsdecode_delwaypoint(aprsdecode_pOPHIST,
                aprsdecode_pFRAMEHIST *);

extern unsigned long aprsdecode_finddup(aprsdecode_pFRAMEHIST,
                aprsdecode_pFRAMEHIST);

extern void aprsdecode_savetrack(void);

extern void aprsdecode_makemsg(char);

extern void aprsdecode_deltxmsg(unsigned long, unsigned long);

extern void aprsdecode_acknumstr(char [], unsigned long, unsigned long);

extern void aprsdecode_getactack(aprsdecode_MONCALL, char [], unsigned long);

extern void aprsdecode_objsender(aprsdecode_pOPHIST, char [], unsigned long);

extern aprsdecode_pOPHIST aprsdecode_selop(void);

extern void aprsdecode_tcpconnstat(char [], unsigned long);

extern void aprsdecode_udpconnstat(unsigned long, char [], unsigned long);

extern void aprsdecode_beeplim(long, unsigned long, unsigned long);

extern void aprsdecode_importbeacon(aprsdecode_pOPHIST);

extern void aprsdecode_extractbeacon(char [], unsigned long, char, char);

extern void aprsdecode_getbeaconname(char [], unsigned long, char [],
                unsigned long, char [], unsigned long, char *, char *,
                char *);

extern void aprsdecode_drawbeacon(char [], unsigned long);

extern void aprsdecode_makelogfn(char [], unsigned long);

extern char aprsdecode_checksymb(char, char);
/* true for bad symbol */


extern void aprsdecode_BEGIN(void);


#endif /* aprsdecode_H_ */
