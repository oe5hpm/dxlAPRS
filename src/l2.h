/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef l2_H_
#define l2_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

typedef uint16_t l2_SET16;

typedef uint8_t l2_SET8;

extern uint16_t l2_UA;

extern uint16_t l2_DM;

extern uint16_t l2_SABM;

extern uint16_t l2_DISC;

extern uint16_t l2_FRMR;

extern uint16_t l2_UI;

extern uint16_t l2_RR;

extern uint16_t l2_REJ;

extern uint16_t l2_RNR;

#define l2_PORTS 15
/* portset is SET16 */

#define l2_MYCALLS 16

typedef char l2_CALLTYP[7];

typedef char * l2_pSTRING;

struct l2_DFIELD;

typedef struct l2_DFIELD * l2_pDATA;


struct l2_DFIELD {
   l2_pDATA next;
   uint16_t len;
   char info[257]; /*incl PID*/
};

typedef void *l2_pLINK;

typedef char l2_AFIELD[71];

enum l2_EVENT {l2_eCONNECTED, l2_eDISCONNECTED, l2_eBUSY, l2_eTXFREE,
                l2_eRXDATA, l2_eMONITOR, 
   l2_eLISTEN, l2_eIDLE, l2_eCONNREQ, l2_eFLEXDATA};


extern char l2_L2Lock; /*disable interrupt starting L2*/

extern uint16_t l2_LostINT;

extern l2_pDATA l2_dbuf; /*must be new allocated after use*/

extern l2_AFIELD l2_adress; /*static adress field*/

extern uint16_t l2_asize; /*length of static adress*/

extern char l2_l2verb;

extern uint32_t l2_dupchk;

extern char l2_udp2buf[100]; /* axudp2 header after Getframe */

struct l2__D0;

struct l2__D1;


struct l2__D1 {
   uint16_t Isent; /*I-frames sent, without I.P*/
   uint16_t Iack; /*therefrom ackowledged*/
   uint16_t Igot; /*I-frames got and accepted*/
   uint32_t Bytesent; /*all frames + FCS + 1 flag*/
   uint32_t Bytegot; /*all frames + FCS + 1 flag*/
};


struct l2__D0 {
   uint16_t Links; /*statistics: links, including inactives*/
   uint16_t MaxLinks;
   struct l2__D1 Flows[13];
};

extern struct l2__D0 l2_PortSt[16];

struct l2__D2;


struct l2__D2 {
   union {
      /* (*c*) = rangecheck necessary*/
      uint16_t card[17];
      struct {
         uint16_t SendPort; /*c*/ /*associated send port to that uplinkport*/
         uint16_t Diversity; /*c*/ /*accept frames to existing links from ports with same numbers*/
         uint16_t DCDlevel;
         uint16_t TXwait; /*wait after sending before sending again*/
         uint16_t TXtail; /*for fullduplex*/
         uint16_t TXdel; /*txdelay*/
         uint16_t DCDwait; /*wait after dcd before sending*/
         uint16_t BaudSwBytes; /*c*/ /*number of flags sent after baud-switch*/
         uint16_t txDelByte; /*bit pattern sent for sync*/
         uint16_t MaxFrames; /*c*/ /*send windowsize*/
         uint16_t T1;
         uint16_t T3;
         uint16_t IPoll; /*max info length for I.Poll*/
         uint16_t Retries;
         uint16_t MhTime; /*0 = mh off*/
         uint16_t DCDIGNOR; /*ignore dcd longer than this time/dama slave off*/
      } _0;
   } _;
   char monitor;
   char passall;
   char DamaMaster;
   char HalfDuplex;
   char Digipeat;
   char Echo;
   char SendBauds[17]; /*relation mycall <-> baud*/
};

extern struct l2__D2 l2_Parms[15];

typedef uint16_t l2_CMD;

enum l2_CONNTYP {l2_cNORMAL, l2_cCONNAK, l2_cFLEXback, l2_cFLEXbusy,
                l2_cTEST};


struct l2_CONNECT;

typedef struct l2_CONNECT * l2_pCONNECT;


struct l2_CONNECT {
   /*       cmd:CARD16;*/
   uint16_t handle;
   uint16_t port; /*the port number of uplink port*/
   uint16_t baud; /*baud is the index of sendbaud-table*/
   uint16_t cpid; /*PID*/
   l2_pSTRING l2adr; /*0-terminated unshifted raw ax.25 adress*/
   l2_pSTRING l3adr; /*0-terminated SABM-with-info text for routing
 unused if points to NIL*/
   uint8_t typ; /*TEST returns an existing link with same adress*/
};

struct l2_SENDUI;

typedef struct l2_SENDUI * l2_pSENDUI;


struct l2_SENDUI {
   char cmd; /* frame cmd */
   uint16_t port;
   uint16_t baud;
   l2_pSTRING path; /*0-terminated, raw, unshifted ax.25 adress-field*/
   uint16_t datalen;
   l2_pSTRING data; /* PID is part of data */
};

struct l2_GETADRESS;

typedef struct l2_GETADRESS * l2_pGETADRESS;


struct l2_GETADRESS {
   uint16_t cmd;
   uint16_t port; /*number of uplink port*/
   uint16_t my; /*number of mycall*/
   uint16_t cpid; /*65535 if not known*/
   char adress[72]; /*0-terminated, raw ax.25 adress*/
};

struct l2_GETSTAT;

typedef struct l2_GETSTAT * l2_pGETSTAT;


struct l2_GETSTAT {
   uint16_t cmd;
   l2_pLINK l;
   uint16_t n;
   uint16_t st;
   uint16_t port;
   uint16_t my;
   uint16_t txbu;
   uint16_t ret;
   char bsy;
   char remb;
   l2_pLINK nudl;
   char adress[72];
   uint32_t sent;
   uint32_t ack;
   uint32_t rcvd;
   uint32_t since;
};

typedef uint32_t l2_SUMMS[7];

struct l2_PORTSTAT;

typedef struct l2_PORTSTAT * l2_pPORTSTAT;


struct l2_PORTSTAT {
   uint16_t cmd;
   uint16_t port;
   uint16_t minits5;
   l2_SUMMS sums;
   uint16_t mem;
   uint32_t time0;
};

struct l2_PARMS0;

typedef struct l2_PARMS0 * l2_pPARMS;


struct l2_PARMS0 {
   uint16_t cmd;
   uint16_t port;
   uint16_t parm;
   char test; /* get instead of set parm */
   uint32_t val; /* for numeric values */
   l2_pSTRING str; /* for string values */
};

typedef void ( *l2_CALLBACKPROC)(char * *, l2_pLINK, uint8_t);

extern l2_pLINK l2_Connect0(char *, l2_pCONNECT);
/* NIL if no link */
/*mp: adress that is enclosed in all messages from L2 to
the host and may point to host-task-context*/

extern void l2_Disconnect(l2_pLINK *, char);
/*if dealloc, the owner of the link will be l2, else owner stays 
the host and it must send a  disconnect with dealloc later to 
release memory and timers*/

extern void l2_Circuit(l2_pLINK *, l2_pLINK *, char, char,
                uint16_t);
/*links streams of l1 and l2, if not reconn, then
the link will then belong to L2 again 
reconn: if the other link disconnects, this links
datastream will be switched back to host
exitchar: if first char in l1-input-stream equals exitchar
the stream of l1 will be switched to host*/

extern void l2_GetAdress0(l2_pLINK, l2_pGETADRESS);

extern void l2_Getudp2info(l2_pLINK, char [], uint32_t);

extern char l2_GetChar(l2_pLINK, char, char *);
/*returns false if buffer is empty
delete: delete char from buffer after get*/

extern uint16_t l2_GetStr(l2_pLINK, uint16_t, l2_pSTRING);
/*return is length of data */

extern char l2_SendStr(l2_pLINK, uint16_t, l2_pSTRING);
/* max 256 byte, false if data not sent (sendbuffer full) */

extern void l2_SendRaw(l2_pSENDUI);
/* pid >255 send no pid */

extern void l2_GetStat0(l2_pGETSTAT);

extern char l2_PortStat0(l2_pPORTSTAT);
/* true if port is enabled */

extern void l2_Parm(l2_pPARMS);

extern l2_pLINK l2_GetMon(void);

extern void l2_Layer2(void);
/*started by 10ms interrupt-ticker if not L2Lock*/

extern void l2_L2Init(uint16_t, uint16_t, l2_CALLBACKPROC);


extern void l2_BEGIN(void);


#endif /* l2_H_ */
