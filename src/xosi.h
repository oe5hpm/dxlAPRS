/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef xosi_H_
#define xosi_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

/* grafical os interface */
typedef long xosi_PROCHANDLE;

#define xosi_WHITELEVEL 1024

struct xosi__D0;


/*+OVERSHOOT*/
struct xosi__D0 {
   unsigned long r;
   unsigned long g;
   unsigned long b;
};

extern struct xosi__D0 xosi_gammalut[1024];

struct xosi_PROCESSHANDLE;


struct xosi_PROCESSHANDLE {
   long pid;
   char started;
   char runs;
   long exitcode;
};

enum xosi_CURSORTYP {xosi_cOFF, xosi_cPULL4, xosi_cPULLLEFT, xosi_cPULLRIGHT,
                 xosi_cPULLUP, xosi_cPULLDOWN, xosi_cPULLLEFTUP,
                xosi_cPULLRIGHTDOWN};


struct xosi_CUTBUF;


struct xosi_CUTBUF {
   char * text;
   unsigned long cutlen;
   unsigned long cutpos;
};

extern unsigned long xosi_newxsize;

extern unsigned long xosi_newysize;

struct xosi__D1;


struct xosi__D1 {
   unsigned long x;
   unsigned long y;
};

extern struct xosi__D1 xosi_xmouse;

extern char xosi_zooming;

extern char xosi_pulling;

extern char xosi_Shift;

extern char xosi_headmh[251];

extern struct xosi_CUTBUF xosi_cutbuffer;

extern long xosi_InitX(char [], unsigned long, char [], unsigned long,
                unsigned long, unsigned long);

extern void xosi_Gammatab(float);

extern void xosi_getscreenbuf(X2C_ADDRESS *, unsigned long *,
                unsigned long *, unsigned long *);

extern void xosi_rgbtoscreen(void);

extern void xosi_allocxbuf(unsigned long, unsigned long);

extern void xosi_cutpaste(void);

extern void xosi_paste(void);

extern void xosi_beep(long, unsigned long, unsigned long);

extern void xosi_xevent(void);

extern void xosi_Eventloop(unsigned long);

extern void xosi_sethand(unsigned char);

extern void xosi_StartProg(char [], unsigned long,
                struct xosi_PROCESSHANDLE *);

extern void xosi_CheckProg(struct xosi_PROCESSHANDLE *);

extern void xosi_StopProg(struct xosi_PROCESSHANDLE *);

extern void xosi_Umlaut(char [], unsigned long);

extern void xosi_setxwinsize(long, long);

extern void xosi_closewin(void);

#define xosi_BYTEPIXEL 4
/* WIN32 screen rgb */


extern void xosi_BEGIN(void);


#endif /* xosi_H_ */
