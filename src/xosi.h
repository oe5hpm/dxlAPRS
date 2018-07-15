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
typedef int32_t xosi_PROCHANDLE;

#define xosi_WHITELEVEL 1024

struct xosi__D0;


/*+OVERSHOOT*/
struct xosi__D0 {
   uint32_t r;
   uint32_t g;
   uint32_t b;
};

extern struct xosi__D0 xosi_gammalut[1024];

struct xosi_PROCESSHANDLE;


struct xosi_PROCESSHANDLE {
   int32_t pid;
   char started;
   char runs;
   int32_t exitcode;
};

enum xosi_CURSORTYP {xosi_cOFF, xosi_cPULL4, xosi_cPULLLEFT, xosi_cPULLRIGHT,
                 xosi_cPULLUP, xosi_cPULLDOWN, xosi_cPULLLEFTUP,
                xosi_cPULLRIGHTDOWN};


struct xosi_CUTBUF;


struct xosi_CUTBUF {
   char * text;
   uint32_t cutlen;
   uint32_t cutpos;
};

extern uint32_t xosi_newxsize;

extern uint32_t xosi_newysize;

struct xosi__D1;


struct xosi__D1 {
   uint32_t x;
   uint32_t y;
};

extern struct xosi__D1 xosi_xmouse;

extern char xosi_zooming;

extern char xosi_pulling;

extern char xosi_Shift;

extern char xosi_Ctrl;

extern char xosi_headmh[251];

extern struct xosi_CUTBUF xosi_cutbuffer;

extern int32_t xosi_InitX(char [], uint32_t, char [], uint32_t,
                 uint32_t, uint32_t);

extern void xosi_Gammatab(float);

extern void xosi_getscreenbuf(char * *, uint32_t *, uint32_t *,
                uint32_t *);

extern void xosi_rgbtoscreen(void);

extern void xosi_allocxbuf(uint32_t, uint32_t);

extern void xosi_cutpaste(void);

extern void xosi_paste(void);

extern void xosi_beep(int32_t, uint32_t, uint32_t);

extern void xosi_xevent(void);

extern void xosi_Eventloop(uint32_t);

extern void xosi_sethand(uint8_t);

extern void xosi_StartProg(char [], uint32_t,
                struct xosi_PROCESSHANDLE *);

extern void xosi_CheckProg(struct xosi_PROCESSHANDLE *);

extern void xosi_StopProg(struct xosi_PROCESSHANDLE *);

extern void xosi_Umlaut(char [], uint32_t);

extern void xosi_setxwinsize(int32_t, int32_t);

extern void xosi_closewin(void);

extern void xosi_WrHeadline(void);

#define xosi_BYTEPIXEL 4
/* WIN32 screen rgb */


extern void xosi_BEGIN(void);


#endif /* xosi_H_ */
