/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef xosi_H_
#include "xosi.h"
#endif
#define xosi_C_
#include <unistd.h>
#ifndef signal_H_
#include "signal.h"
#endif
#include <sys/wait.h>
#ifndef beep_H_
#include "beep.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef useri_H_
#include "useri.h"
#endif

#include <X11/Xlib.h>

#include "xosic.h"
#include "pastewrapper.h"

#include <stdio.h>

struct xosi__D0 xosi_gammalut[1024];




uint32_t xosi_newxsize;
uint32_t xosi_newysize;


struct xosi__D1 xosi_xmouse;
char xosi_zooming;
char xosi_pulling;
char xosi_Shift;
char xosi_Ctrl;
char xosi_headmh[251];
struct xosi_CUTBUF xosi_cutbuffer;
/* graphical os interface linux/win32 */
/*FROM InOut IMPORT WriteLn, WriteString; */
/*FROM fcntl IMPORT creat, open; */
/*FROM IOChan IMPORT ChanId; */
/*FROM RealIO IMPORT WriteFixed; */
struct XWIN;


struct XWIN {
   XSizeHints myhint;
   uint32_t win; /* My window */
   PtrVisual pvis;
   PtrXImage ximage0;
   uint32_t redmask;
   uint32_t greenmask;
   uint32_t bluemask;
   int32_t redshift;
   int32_t greenshift;
   int32_t blueshift;
   uint32_t Black;
   uint32_t White;
   uint32_t bitperpixel;
   GC gc;
   uint32_t pixmap;
   int32_t screen; /* Default screen number */
   PtrScreen screenp;
   XFontStruct fontinfo;
   uint32_t xsizen;
   uint32_t ysizen;
};


struct _0 {
   uint16_t * Adr;
   size_t Len0;
};

static struct _0 * xbuf016;

static int32_t xbuf0size;

static uint32_t xbufxsize;

static uint32_t xbufysize;

static struct XWIN mainwin;

static PtrDisplay dis; /* Connection to the X server */

static int32_t disfd;

static union _XEvent event;

static KeySym mykey;

static struct _XComposeStatus mystat;

static uint32_t timesum;

static uint32_t cursor;

static char cursorset;

static Atom wclose;


static uint32_t Gamma(uint32_t c, float g)
{
   if (c==0UL) return 0UL;
   if (c<1024UL) {
      return (uint32_t)X2C_TRUNCC(osic_exp(X2C_DIVR(osic_ln(X2C_DIVR((float)
                c,1024.0f)),g))*255.5f,0UL,X2C_max_longcard);
   }
   return 255UL;
} /* end Gamma() */


extern void xosi_Gammatab(float g)
{
   uint32_t ga;
   uint32_t i;
   struct XWIN * anonym;
   struct xosi__D0 * anonym0;
   { /* with */
      struct XWIN * anonym = &mainwin;
      for (i = 0UL; i<=1023UL; i++) {
         ga = Gamma(i, g);
         { /* with */
            struct xosi__D0 * anonym0 = &xosi_gammalut[i];
            anonym0->r = X2C_LSH((uint32_t)ga,32,
                anonym->redshift)&anonym->redmask;
            anonym0->g = X2C_LSH((uint32_t)ga,32,
                anonym->greenshift)&anonym->greenmask;
            anonym0->b = X2C_LSH((uint32_t)ga,32,
                anonym->blueshift)&anonym->bluemask;
         }
      } /* end for */
   }
} /* end Gammatab() */


extern void xosi_WrHeadline(void)
{
   if (dis) XStoreName(dis, mainwin.win, xosi_headmh);
} /* end WrHeadline() */


static int32_t MaxBit(uint32_t colmask)
{
   int32_t i;
   i = 31L;
   while (i>0L && !X2C_IN(i,32,colmask)) --i;
   return i+1L;
} /* end MaxBit() */


static void allocxbufw(struct XWIN * w, uint32_t xsizeh,
                uint32_t ysizeh)
{
   struct XWIN * anonym;
   /*WrInt(xsizeh, 1);WrStr(" x ");WrInt(ysizeh, 1); WrLn; */
   { /* with */
      struct XWIN * anonym = w;
      xbufxsize = xsizeh;
      xbufysize = ysizeh;
      if (anonym->ximage0) {
         useri_debugmem.screens -= (uint32_t)xbuf0size;
         osic_free((char * *) &xbuf016, (uint32_t)xbuf0size);
         xbuf016 = 0;
         anonym->ximage0->data = 0;
         XDestroyImage(anonym->ximage0); /* xbuf0 deallocated too */
      }
      xbuf0size = (int32_t)(2UL*ysizeh*xsizeh);
      if (anonym->bitperpixel>16UL) xbuf0size = xbuf0size*2L;
      osic_alloc((char * *) &xbuf016, (uint32_t)xbuf0size);
      useri_debugmem.req = (uint32_t)xbuf0size;
      useri_debugmem.screens += useri_debugmem.req;
      if (xbuf016==0) {
         osi_WrStrLn("initx: out of memory", 21ul);
         useri_wrheap();
         X2C_ABORT();
      }
      anonym->ximage0 = XCreateImage(dis, anonym->pvis, anonym->bitperpixel,
                2L, 0L, (char *)xbuf016, xsizeh, ysizeh, 32L, 0L);
      if (anonym->ximage0==0) {
         osi_WrStrLn("XCreateImage returns NIL", 25ul);
         X2C_ABORT();
      }
      /*
          s:="aprsmap by oe5dxl ";
          IntToStr(xsizeh, 1, h); Append(s, h);
          Append(s, "x");
          IntToStr(ysizeh, 1, h); Append(s, h);
          Xlib.XStoreName(dis, win, s);
      */
      xosi_headmh[0UL] = 0;
   }
} /* end allocxbufw() */


extern void xosi_allocxbuf(uint32_t xsize, uint32_t ysize)
{
   allocxbufw(&mainwin, xsize, ysize);
} /* end allocxbuf() */

static int32_t MakeMainWin(
		char winname[], uint32_t winname_len,
		char iconname[], uint32_t iconname_len,
		uint32_t xsizeh, uint32_t ysizeh)
{
	struct XWIN *pwin = &mainwin;

	pwin->myhint.height = ysizeh;
	pwin->myhint.width = xsizeh;
	pwin->myhint.x = 1;
	pwin->myhint.y = 1;
	pwin->myhint.flags = 0xCUL;

	/* Connect to the X server */
	dis = XOpenDisplay("");
	if (dis == 0) {
		fprintf(stderr, "couldn't open display!\n");
		return -1;
	}

	pwin->screen = DefaultScreen(dis);

	/* Create a window */
	pwin->win = XCreateSimpleWindow(dis,
					DefaultRootWindow(dis),
					pwin->myhint.x,
					pwin->myhint.y,
					pwin->myhint.width,
					pwin->myhint.height,
					1, 0, 0);
	if (pwin->win == 0) {
		fprintf(stderr, "couldn't create window!\n");
		return -1;
	}

	wclose = XInternAtom(dis, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(dis, pwin->win, &wclose, 1);

	pwin->pvis = DefaultVisual(dis, 0);
	if (pwin->pvis == 0) {
		fprintf(stderr, "cannot get visual!\n");
		return -1;
	}
	XSetStandardProperties(dis,
			       pwin->win,
			       winname, iconname,
			       0, 0, 0,
			       &pwin->myhint);

	pwin->redmask = (uint32_t)pwin->pvis->red_mask;
	pwin->greenmask = (uint32_t)pwin->pvis->green_mask;
	pwin->bluemask = (uint32_t)pwin->pvis->blue_mask;
	pwin->redshift = MaxBit(pwin->redmask)-8L;
	pwin->greenshift = MaxBit(pwin->greenmask)-8L;
	pwin->blueshift = MaxBit(pwin->bluemask)-8L;
	pwin->bitperpixel = DefaultDepth(dis, pwin->screen);

	pwin->ximage0 = 0;
	allocxbufw(&mainwin, xsizeh, ysizeh);

	XSelectInput(dis, pwin->win, 0x2804FUL);

	XMapWindow(dis, pwin->win);

	disfd = ConnectionNumber(dis);
	xosi_pulling = 0;
	cursorset = 0;

	return 0;
}

extern void xosi_closewin(void)
{
   XCloseDisplay(dis);
} /* end closewin() */


extern int32_t xosi_InitX(char winname[], uint32_t winname_len,
                char iconname[], uint32_t iconname_len,
                uint32_t xsizeh, uint32_t ysizeh)
{
   int32_t xosi_InitX_ret;
   X2C_PCOPY((void **)&winname,winname_len);
   X2C_PCOPY((void **)&iconname,iconname_len);
   xbuf016 = 0;
   if (MakeMainWin(winname, winname_len, iconname, iconname_len, xsizeh,
                ysizeh)<0L) {
      xosi_InitX_ret = -1L;
      goto label;
   }
   xosi_Shift = 0;
   xosi_Ctrl = 0;
   xosi_InitX_ret = 0L;
   label:;
   X2C_PFREE(winname);
   X2C_PFREE(iconname);
   return xosi_InitX_ret;
} /* end InitX() */


extern void xosi_getscreenbuf(char * * adr, uint32_t * xsize,
                uint32_t * ysize, uint32_t * incadr)
{
   *adr = (char *)xbuf016;
   *xsize = xbufxsize;
   *ysize = xbufysize;
   if (mainwin.bitperpixel<=16UL) *incadr = 2UL;
   else *incadr = 4UL;
} /* end getscreenbuf() */


extern void xosi_rgbtoscreen(void)
{
   XPutImage(dis, mainwin.win, DefaultGC(dis, 0L), mainwin.ximage0, 0L, 0L,
                0L, 0L, xbufxsize, xbufysize);
} /* end rgbtoscreen() */


extern void xosi_beep(int32_t lev, uint32_t hz, uint32_t ms)
{
   XKeyboardControl val;
   if (hz>0UL && ms>0UL) {
      if (cbell((int32_t)hz, (int32_t)ms)<0L && dis) {
         val.bell_pitch = (int32_t)hz;
         val.bell_duration = (int32_t)(ms/2UL);
         XChangeKeyboardControl(dis, (uint32_t)0xCUL,
                (PAXKeyboardControl) &val);
         XBell(dis, lev);
      }
   }
} /* end beep() */


extern void xosi_Umlaut(char c[], uint32_t c_len)
/* umlaut converter */
{
   switch ((uint32_t)(uint8_t)c[0UL]) {
   case 228UL:
      c[0UL] = 'a';
      c[1UL] = 'e';
      c[2UL] = 0;
      break;
   case 246UL:
      c[0UL] = 'o';
      c[1UL] = 'e';
      c[2UL] = 0;
      break;
   case 252UL:
      c[0UL] = 'u';
      c[1UL] = 'e';
      c[2UL] = 0;
      break;
   case 196UL:
      c[0UL] = 'A';
      c[1UL] = 'e';
      c[2UL] = 0;
      break;
   case 214UL:
      c[0UL] = 'O';
      c[1UL] = 'e';
      c[2UL] = 0;
      break;
   case 220UL:
      c[0UL] = 'U';
      c[1UL] = 'e';
      c[2UL] = 0;
      break;
   case 223UL:
      c[0UL] = 's';
      c[1UL] = 's';
      c[2UL] = 0;
      break;
   } /* end switch */
} /* end Umlaut() */

/*
    pb:POINTER TO ARRAY[0..1000000] OF CHAR;
    len:INTEGER;
    cmd:CHAR;
*/

extern void xosi_paste(void)
{
   Window cw;
   uint32_t pa;
   /*
     pb:=Xlib.XFetchBytes(dis, len);
     IF pb<>NIL THEN
       FOR i:=0 TO len-1 DO
         cmd:=pb^[i];
         keychar(cmd, TRUE, FALSE);
       END;
     END;
   */
   cw = XGetSelectionOwner(dis, 1UL);
   if (cw) {
      pa = XInternAtom(dis, "STRING", 0);
      XConvertSelection(dis, 1UL, pa, pa, mainwin.win, 0UL);
   }
} /* end paste() */


extern void xosi_cutpaste(void)
{
   /*Xlib.XStoreBytes(dis, ADR(s), Length(s)); */
   /*Xlib.XRotateBuffers(dis, 1); */
   XSetSelectionOwner(dis, 1UL, mainwin.win, 0UL);
   if (XGetSelectionOwner(dis, 1UL)!=mainwin.win) useri_clrcpmarks();
} /* end cutpaste() */

typedef uint8_t * CP;


static void sendcopypasteevent(XSelectionRequestEvent xreq)
{
   XSelectionEvent ev;
   CP cp;
   /*WrInt(xreq.requestor, 15); WrInt(xreq.selection, 15); */
   /*WrInt(xreq.target, 15); WrInt(xreq.property, 15); WrInt(xreq.time, 15);
                WrStrLn("esrq"); */
   if (xreq.target==XInternAtom(dis, "STRING", 0) || xreq.target==294UL) {
      cp = (CP)xosi_cutbuffer.text;
      XChangeProperty(dis, xreq.requestor, xreq.property, xreq.target, 8L,
                0L, cp, (int32_t)xosi_cutbuffer.cutlen);
      ev.property = xreq.property;
   }
   else ev.property = 0UL;
   ev.type = 31L;
   ev.serial = xreq.serial;
   ev.send_event = 1;
   ev.display = xreq.display;
   ev.requestor = xreq.requestor;
   ev.selection = xreq.selection;
   ev.target = xreq.target;
   ev.time = xreq.time;
   XSendEvent(xreq.display, xreq.requestor, 1L, 0L, (PtrXEvent) &ev);
} /* end sendcopypasteevent() */

/*
PROCEDURE pulloff;
BEGIN
  IF pullhand THEN
    Xlib.XUndefineCursor(dis, mainwin.win);
    Xlib.XFreeCursor(dis, cursor);
    pullhand:=FALSE;
  END;
END pulloff;
*/

extern void xosi_setxwinsize(int32_t x, int32_t y)
{
   XMoveResizeWindow(dis, mainwin.win, 0L, 0L, (uint32_t)x, (uint32_t)y);
} /* end setxwinsize() */


static void maximize(void)
{
   XWindowAttributes xwa;
   XGetWindowAttributes(dis, DefaultRootWindow(dis), &xwa);
   useri_resizewin(xwa.width, xwa.height, 1);
} /* end maximize() */


extern void xosi_sethand(uint8_t t)
{
   int32_t curs;
   if (t) {
      if (!cursorset) {
         switch ((unsigned)t) {
         case xosi_cPULL4:
            curs = 52L;
            break;
         case xosi_cPULLLEFT:
            curs = 70L;
            break;
         case xosi_cPULLRIGHT:
            curs = 96L;
            break;
         case xosi_cPULLUP:
            curs = 138L;
            break;
         case xosi_cPULLDOWN:
            curs = 16L;
            break;
         case xosi_cPULLLEFTUP:
            curs = 134L;
            break;
         case xosi_cPULLRIGHTDOWN:
            curs = 14L;
            break;
         default:;
            curs = 68L;
            break;
         } /* end switch */
         cursor = XCreateFontCursor(dis, (uint32_t)curs);
         XDefineCursor(dis, mainwin.win, cursor);
         cursorset = 1;
      }
   }
   else if (cursorset) {
      XUndefineCursor(dis, mainwin.win);
      XFreeCursor(dis, cursor);
      cursorset = 0;
   }
} /* end sethand() */


static void checkbuttons(void)
{
   /* check mouse buttons if lost events */
   Window c;
   Window r;
   int32_t wy;
   int32_t wx;
   int32_t y;
   int32_t x;
   uint32_t m;
   if ((xosi_pulling && XQueryPointer(dis, mainwin.win, &r, &c, &x, &y, &wx,
                &wy, &m)) && ((uint32_t)m&0x300UL)==0UL) {
      xosi_sethand(xosi_cOFF);
   }
} /* end checkbuttons() */


extern void xosi_xevent(void)
{
   char h[256];
   XWindowAttributes attr;
   int32_t res;
   char cmd;
   uint32_t nn;
   PAUChar pstr;
   int32_t tmp;
   while (XPending(dis)) {
      XNextEvent(dis, &event);
      /*IF event.type<>6 THEN WrInt(event.type, 10); WrStrLn(" event") END;
                */
      switch (event.type) {
      case 4L:
         switch (event.xbutton.button) {
         case 1UL:
            /*
                               IF Shift THEN
                                 Pull(event.xmotion.x, event.xmotion.y, 0);
                                 sethand;
                               ELSE mouseleft(event.xmotion.x,
                event.xmotion.y) END;
            */
            useri_mouseleftdown(event.xmotion.x, event.xmotion.y);
            break;
         case 2UL: /*paste;*/
            useri_mousemiddle(event.xmotion.x, event.xmotion.y);
            break;
         case 3UL:
            useri_mouserightdown(event.xmotion.x, event.xmotion.y);
            break;
         case 4UL:
            useri_keychar('\310', 0, 1);
            break;
         case 5UL:
            useri_keychar('\311', 0, 1);
            break;
         } /* end switch */
         break;
      case 2L:
         cmd = 0;
         res = XLookupString(&event.xkey, (PChar)h, 256L, &mykey, &mystat);
         if (res==1L) cmd = h[0U];
         else if (res==0L) {
            if (mykey==65364UL) cmd = '\030';
            else if (mykey==65361UL) cmd = '\023';
            else if (mykey==65362UL) cmd = '\005';
            else if (mykey==65365UL) cmd = '\022';
            else if (mykey==65366UL) cmd = '\003';
            else if (mykey==65363UL) cmd = '\004';
            else if (mykey==65288UL) cmd = '\010';
            else if (mykey==65535UL) cmd = '\177';
            else if (mykey==65360UL) cmd = '\001';
            else if (mykey==65367UL) cmd = '\002';
            else if (mykey==65379UL) cmd = '\017';
            else if (mykey==65480UL) maximize();
            else if (mykey==65505UL || mykey==65506UL) xosi_Shift = 1;
            else if (mykey==65507UL || mykey==65508UL) xosi_Ctrl = 1;
         }
         if (cmd=='\026') xosi_paste();
         else if (cmd) useri_keychar(cmd, 0, 0);
         break;
      case 3L:
         res = XLookupString(&event.xkey, (PChar)h, 256L, &mykey, &mystat);
         if (res==0L) {
            if (mykey==65505UL || mykey==65506UL) xosi_Shift = 0;
            else if (mykey==65507UL || mykey==65508UL) xosi_Ctrl = 0;
         }
         break;
      case 22L:
         /*WrInt(event.xconfigure.window, 10); WrInt(mainwin.win, 10); */
         /*WrStrLn(" ConfigureNotify"); */
         XGetWindowAttributes(dis, mainwin.win, &attr);
         useri_resizewin(attr.width, attr.height, 0);
         break;
      case 12L:
         if (xosi_newxsize==0UL) useri_refreshwin();
         break;
      case 6L:
         /*          Pull(event.xmotion.x, event.xmotion.y, 1); */
         useri_mousemove(event.xmotion.x, event.xmotion.y);
         break;
      case 5L:
         useri_mouserelease();
         break;
      case 31L:
         pasteget(dis, mainwin.win, event.xselection.property, &nn, &pstr);
         if ((char *)pstr) {
            tmp = (int32_t)nn-1L;
            res = 0L;
            if (res<=tmp) for (;; res++) {
               cmd = pstr[res];
               if ((uint8_t)cmd>=' ' && (uint8_t)cmd<=(uint8_t)'\201') {
                  useri_keychar(cmd, 1, 0);
               }
               if (res==tmp) break;
            } /* end for */
            XFree((XVaNestedList)pstr);
         }
         break;
      case 30L:
         sendcopypasteevent(event.xselectionrequest);
         break;
      case 29L:
         useri_clrcpmarks();
         break;
      case 33L:
         if ((uint32_t)event.xclient.data.l[0U]==wclose) {
            aprsdecode_quit = 1;
            osi_WrStrLn("Main Window Close", 18ul); /* window close event */
         }
         break;
      } /* end switch */
   }
} /* end xevent() */


static void addsock(int32_t fd, char wtoo)
/* insert socket in fdset for select */
{
   if (fd>=0L) {
      fdsetr((uint32_t)fd);
      if (wtoo) fdsetw((uint32_t)fd);
   }
} /* end addsock() */


extern void xosi_CheckProg(struct xosi_PROCESSHANDLE * chpid)
{
   int32_t ret;
   int32_t stat;
   if (chpid->runs) {
      ret = waitpid(chpid->pid, &stat, WNOHANG);
      if (ret==chpid->pid) chpid->exitcode = stat;
      if (ret<0L) chpid->runs = 0;
   }
} /* end CheckProg() */

#define xosi_ARGS 100

#define xosi_QUOTT "\""


extern void xosi_StartProg(char name[], uint32_t name_len,
                struct xosi_PROCESSHANDLE * chpid)
{
   char pgmname[1025];
   char argt[100][1025];
   X2C_pCHAR * args;
   char * arga[100];
   char quot;
   uint32_t a;
   uint32_t len;
   uint32_t j;
   uint32_t i;
   X2C_PCOPY((void **)&name,name_len);
   /*WrStr("start: "); WrStrLn(name); */
   chpid->pid = fork();
   if (chpid->pid==0L) {
      /* child */
      len = aprsstr_Length(name, name_len);
      i = 0UL;
      j = 0UL;
      quot = 0;
      while (i<len && (quot || (uint8_t)name[i]>' ')) {
         if (name[i]=='\"') quot = !quot;
         else if (j<1024UL) {
            pgmname[j] = name[i];
            ++j;
         }
         ++i;
      }
      pgmname[j] = 0;
      /*WrStr(" <<<");WrStr(pgmname);WrStrLn(">>>"); */
      /*  WHILE (i<len) & (name[i]>" ") DO INC(i) END;
                (* end of program name *) */
      quot = 0;
      while (i>0UL && name[i-1UL]!='/') {
         if (name[i]=='\"') quot = !quot;
         --i;
      }
      /* program name without path */
      for (a = 0UL; a<=99UL; a++) {
         while (i<len && name[i]==' ') ++i;
         j = 0UL;
         while (i<len && (quot || (uint8_t)name[i]>' ')) {
            if (name[i]=='\"') quot = !quot;
            else if (j<1024UL) {
               argt[a][j] = name[i];
               ++j;
            }
            ++i;
         }
         argt[a][j] = 0;
         /*WrInt(a, 5);  WrStr(" <<");WrStr(argt[a]);WrStrLn(">>"); */
         if (j>0UL) arga[a] = (char *)argt[a];
         else arga[a] = 0;
      } /* end for */
      /*WrStrLn("child start"); */
      args = (X2C_pCHAR *)arga;
      execv(pgmname, args);
      /*WrStrLn("child end"); */
      X2C_ABORT();
   }
   chpid->runs = chpid->pid>=0L; /* ok or fork error */ /* parent */
   X2C_PFREE(name);
} /* end StartProg() */


extern void xosi_StopProg(struct xosi_PROCESSHANDLE * chpid)
{
   /*WrInt(chpid.pid, 9);WrStrLn(" closepid"); */
   if (chpid->runs) kill(chpid->pid, SIGTERM);
} /* end StopProg() */


extern void xosi_Eventloop(uint32_t timeout)
{
   aprsdecode_pTCPSOCK acttmp;
   aprsdecode_pTCPSOCK acttcp;
   int32_t i;
   uint32_t tus;
   uint32_t ts;
   uint32_t t;
   fdclr(); /* clear select bitset */
   for (i = 0L; i<=3L; i++) {
      addsock(aprsdecode_udpsocks0[i].fd, 0); /* set udp ports */
   } /* end for */
   addsock(disfd, 0); /* x11 fd */
   acttcp = aprsdecode_tcpsocks;
   while (acttcp) {
      addsock(acttcp->fd, acttcp->tlen>0L); /* set tcp ports */
      acttcp = acttcp->next;
   }
   ts = 0UL;
   tus = timeout;
   selectrwt(&ts, &tus);
   timesum += timeout-tus;
   if (timesum>=timeout) {
      useri_timerevent();
      timesum = 0UL;
   }
   /*  IF issetr(disfd) THEN xevent END;
                    (* kbd, mouse *) */
   xosi_xevent();
   checkbuttons();
   t = osic_time();
   if (aprsdecode_lastlooped+5UL<t) aprsdecode_rxidle = 0UL;
   else ++aprsdecode_rxidle;
   aprsdecode_lastlooped = t;
   for (i = 0L; i<=3L; i++) {
      if (aprsdecode_udpsocks0[i].fd>=0L && issetr((uint32_t)
                aprsdecode_udpsocks0[i].fd)) aprsdecode_udpin((uint32_t)i);
   } /* end for */
   acttcp = aprsdecode_tcpsocks;
   while (acttcp) {
      if (acttcp->fd>=0L && issetw((uint32_t)acttcp->fd)) {
         aprsdecode_tcpout(acttcp);
      }
      if (acttcp->fd>=0L && issetr((uint32_t)acttcp->fd)) {
         aprsdecode_tcpin(acttcp);
      }
      if (acttcp->fd<0L) {
         /* connection lost */
         aprsdecode_tcpclose(acttcp, 1);
         if (acttcp==aprsdecode_tcpsocks) {
            aprsdecode_tcpsocks = aprsdecode_tcpsocks->next;
         }
         else {
            acttmp = aprsdecode_tcpsocks;
            while (acttmp->next!=acttcp) acttmp = acttmp->next;
            acttmp->next = acttcp->next;
         }
         useri_debugmem.mon -= sizeof(struct aprsdecode_TCPSOCK);
         osic_free((char * *) &acttcp,
                sizeof(struct aprsdecode_TCPSOCK));
         acttcp = aprsdecode_tcpsocks;
      }
      else acttcp = acttcp->next;
   }
} /* end Eventloop() */


extern void xosi_BEGIN(void)
{
   static int xosi_init = 0;
   if (xosi_init) return;
   xosi_init = 1;
   osi_BEGIN();
   useri_BEGIN();
   aprsstr_BEGIN();
   aprsdecode_BEGIN();
   xosi_headmh[0UL] = 0;
   xosi_cutbuffer.text = 0;
}

