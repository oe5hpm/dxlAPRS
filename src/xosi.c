/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
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

static void *paintbuf;
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
	if (c == 0)
		return 0;
	if (c < 1024) {
		return X2C_TRUNCC(osic_exp(X2C_DIVR(osic_ln(X2C_DIVR((float)c, 1024.0f)), g)) * 255.5f,
				  0,
				  X2C_max_longcard);
	}
	return 255;
}

extern void xosi_Gammatab(float g)
{
	uint32_t ga;
	uint32_t i;
	struct XWIN *pwin = &mainwin;;
	struct xosi__D0 *prgb;

	for (i = 0; i <= 1023; i++) {
		ga = Gamma(i, g);
		prgb = &xosi_gammalut[i];

		prgb->r = X2C_LSH(ga, 32, pwin->redshift)&pwin->redmask;
		prgb->g = X2C_LSH(ga, 32, pwin->greenshift)&pwin->greenmask;
		prgb->b = X2C_LSH(ga, 32, pwin->blueshift)&pwin->bluemask;
	}
}

extern void xosi_WrHeadline(void)
{
	if (dis)
		XStoreName(dis, mainwin.win, xosi_headmh);
}

static int32_t MaxBit(uint32_t colmask)
{
	int32_t i = 31;

	while (i > 0 && !X2C_IN(i, 32, colmask))
		i--;

	return (i + 1);
}

static void allocxbufw(struct XWIN *w, uint32_t xsizeh, uint32_t ysizeh)
{
	xbufxsize = xsizeh;
	xbufysize = ysizeh;

	if (w->ximage0) {
		useri_debugmem.screens -= (uint32_t)xbuf0size;
		if (paintbuf)
			free(paintbuf);
		paintbuf = NULL;
		w->ximage0->data = 0;
		XDestroyImage(w->ximage0); /* xbuf0 deallocated too */
	}

	xbuf0size = 2 * ysizeh * xsizeh;
	if (w->bitperpixel > 16)
		xbuf0size = xbuf0size * 2;
	paintbuf = malloc(xbuf0size);
	if (paintbuf == NULL) {
		fprintf(stderr, "initx: out of memory!\n");
		useri_wrheap();
		X2C_ABORT();
	}
	useri_debugmem.req = (uint32_t)xbuf0size;
	useri_debugmem.screens += useri_debugmem.req;
	w->ximage0 = XCreateImage(dis,
				  w->pvis,
				  w->bitperpixel,
				  2, 0,
				  paintbuf, xsizeh, ysizeh,
				  32, 0);
	if (w->ximage0 == 0) {
		fprintf(stderr, "XCreateImage failed!\n");
		X2C_ABORT();
	}
	xosi_headmh[0] = 0;
}

extern void xosi_allocxbuf(uint32_t xsize, uint32_t ysize)
{
	allocxbufw(&mainwin, xsize, ysize);
}

static int MakeMainWin(
		char winname[], uint32_t winname_len,
		char iconname[], uint32_t iconname_len,
		uint32_t xsizeh, uint32_t ysizeh)
{
	struct XWIN *pwin = &mainwin;

	pwin->myhint.height = ysizeh;
	pwin->myhint.width = xsizeh;
	pwin->myhint.x = 1;
	pwin->myhint.y = 1;
	pwin->myhint.flags = 0xC;

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

	pwin->redmask = pwin->pvis->red_mask;
	pwin->greenmask = pwin->pvis->green_mask;
	pwin->bluemask = pwin->pvis->blue_mask;
	pwin->redshift = MaxBit(pwin->redmask) - 8;
	pwin->greenshift = MaxBit(pwin->greenmask) - 8;
	pwin->blueshift = MaxBit(pwin->bluemask) - 8;
	pwin->bitperpixel = DefaultDepth(dis, pwin->screen);

	pwin->ximage0 = 0;
	allocxbufw(&mainwin, xsizeh, ysizeh);

	XSelectInput(dis,pwin->win,
		     KeyPressMask |
		     KeyReleaseMask |
		     ButtonPressMask |
		     ButtonReleaseMask |
		     PointerMotionMask |
		     ExposureMask |
		     StructureNotifyMask);

	XMapWindow(dis, pwin->win);

	disfd = ConnectionNumber(dis);
	xosi_pulling = 0;
	cursorset = 0;

	return 0;
}

extern void xosi_closewin(void)
{
	XCloseDisplay(dis);
}

extern int32_t xosi_InitX(
		char winname[], uint32_t winname_len,
		char iconname[], uint32_t iconname_len,
		uint32_t xsizeh, uint32_t ysizeh)
{
	int rc;

	paintbuf = NULL;
	rc = MakeMainWin(winname,
			 winname_len, iconname, iconname_len,
			 xsizeh, ysizeh);
	if (rc < 0)
		return -1;

	xosi_Shift = 0;
	xosi_Ctrl = 0;

	return 0;
}

extern void xosi_getscreenbuf(
		char **adr,
		uint32_t *xsize, uint32_t *ysize, uint32_t *incadr)
{
	*adr = (char *)paintbuf;
	*xsize = xbufxsize;
	*ysize = xbufysize;
	if (mainwin.bitperpixel <= 16)
		*incadr = 2;
	else
		*incadr = 4;
}

extern void xosi_rgbtoscreen(void)
{
	XPutImage(dis,
		  mainwin.win,
		  DefaultGC(dis, 0),
		  mainwin.ximage0,
		  0, 0,
		  0, 0,
		  xbufxsize, xbufysize);
}

extern void xosi_beep(int32_t lev, uint32_t hz, uint32_t ms)
{
	XKeyboardControl val;

	if (hz > 0 && ms > 0) {
		if (cbell((int32_t)hz, (int32_t)ms) < 0 && dis) {
			val.bell_pitch = (int32_t)hz;
			val.bell_duration = (int32_t)(ms / 2);
			XChangeKeyboardControl(dis, (uint32_t)0xC,
					      (PAXKeyboardControl)&val);
			XBell(dis, lev);
		}
	}
}

extern void xosi_Umlaut(char c[], uint32_t c_len)
{
	switch ((uint8_t)c[0]) {
	case 228:
		c[0] = 'a';
		c[1] = 'e';
		c[2] = 0;
		break;
	case 246:
		c[0] = 'o';
		c[1] = 'e';
		c[2] = 0;
		break;
	case 252:
		c[0] = 'u';
		c[1] = 'e';
		c[2] = 0;
		break;
	case 196:
		c[0] = 'A';
		c[1] = 'e';
		c[2] = 0;
		break;
	case 214:
		c[0] = 'O';
		c[1] = 'e';
		c[2] = 0;
		break;
	case 220:
		c[0] = 'U';
		c[1] = 'e';
		c[2] = 0;
		break;
	case 223:
		c[0] = 's';
		c[1] = 's';
		c[2] = 0;
		break;
	}
}

extern void xosi_paste(void)
{
	Window cw;
	Atom pa;

	cw = XGetSelectionOwner(dis, 1);
	if (cw) {
		pa = XInternAtom(dis, "STRING", 0);
		XConvertSelection(dis, 1, pa, pa, mainwin.win, 0);
	}
}

extern void xosi_cutpaste(void)
{
	XSetSelectionOwner(dis, 1, mainwin.win, 0);
	if (XGetSelectionOwner(dis, 1) != mainwin.win)
		useri_clrcpmarks();
}

static void sendcopypasteevent(XSelectionRequestEvent xreq)
{
	XSelectionEvent ev;
	uint8_t *cp;

	if (xreq.target == XInternAtom(dis, "STRING", 0) ||
	    xreq.target == 294) {
		cp = (uint8_t *)xosi_cutbuffer.text;
		XChangeProperty(dis,
				xreq.requestor,
				xreq.property,
				xreq.target,
				8,
				0,
				cp, xosi_cutbuffer.cutlen);
		ev.property = xreq.property;
	} else {
		ev.property = 0;
	}

	ev.type = SelectionNotify;
	ev.serial = xreq.serial;
	ev.send_event = 1;
	ev.display = xreq.display;
	ev.requestor = xreq.requestor;
	ev.selection = xreq.selection;
	ev.target = xreq.target;
	ev.time = xreq.time;
	XSendEvent(xreq.display, xreq.requestor, 1, 0, (PtrXEvent)&ev);
}

extern void xosi_setxwinsize(int32_t x, int32_t y)
{
	XMoveResizeWindow(dis, mainwin.win, 0, 0, (uint32_t)x, (uint32_t)y);
}

static void maximize(void)
{
	XWindowAttributes xwa;
	XGetWindowAttributes(dis, DefaultRootWindow(dis), &xwa);
	useri_resizewin(xwa.width, xwa.height, 1);
}

extern void xosi_sethand(uint8_t t)
{
	int32_t curs;
	if (t) {
		if (!cursorset) {
			switch ((unsigned)t) {
			case xosi_cPULL4:
				curs = 52;
				break;
			case xosi_cPULLLEFT:
				curs = 70;
				break;
			case xosi_cPULLRIGHT:
				curs = 96;
				break;
			case xosi_cPULLUP:
				curs = 138;
				break;
			case xosi_cPULLDOWN:
				curs = 16;
				break;
			case xosi_cPULLLEFTUP:
				curs = 134;
				break;
			case xosi_cPULLRIGHTDOWN:
				curs = 14;
				break;
			default:;
				curs = 68;
				break;
			}
			cursor = XCreateFontCursor(dis, (uint32_t)curs);
			XDefineCursor(dis, mainwin.win, cursor);
			cursorset = 1;
		}
	} else if (cursorset) {
		XUndefineCursor(dis, mainwin.win);
		XFreeCursor(dis, cursor);
		cursorset = 0;
	}
}

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
	if ((xosi_pulling &&
	     XQueryPointer(dis, mainwin.win, &r, &c, &x, &y, &wx, &wy, &m)) &&
	     ((uint32_t)m & 0x300) == 0) {
		xosi_sethand(xosi_cOFF);
	}
}

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

		switch (event.type) {
		case ButtonPress:
			switch (event.xbutton.button) {
			case 1:
				useri_mouseleftdown(event.xmotion.x,
						    event.xmotion.y);
				break;
			case 2: /*paste;*/
				useri_mousemiddle(event.xmotion.x,
						  event.xmotion.y);
				break;
			case 3:
				useri_mouserightdown(event.xmotion.x,
						     event.xmotion.y);
				break;
			case 4:
				useri_keychar('\310', 0, 1);
				break;
			case 5:
				useri_keychar('\311', 0, 1);
				break;
			}
			break;
		case KeyPress:
			cmd = 0;
			res = XLookupString(&event.xkey,
					    (PChar)h, sizeof(h),
					    &mykey, &mystat);
			if (res==1) {
				cmd = h[0];
			} else if (res == 0) {
				if (mykey == 65364)
					cmd = '\030';
				else if (mykey == 65361)
					cmd = '\023';
				else if (mykey == 65362)
					cmd = '\005';
				else if (mykey == 65365)
					cmd = '\022';
				else if (mykey == 65366)
					cmd = '\003';
				else if (mykey == 65363)
					cmd = '\004';
				else if (mykey == 65288)
					cmd = '\010';
				else if (mykey == 65535)
					cmd = '\177';
				else if (mykey == 65360)
					cmd = '\001';
				else if (mykey == 65367)
					cmd = '\002';
				else if (mykey == 65379)
					cmd = '\017';
				else if (mykey == 65480)
					maximize();
				else if (mykey == 65505 || mykey == 65506)
					xosi_Shift = 1;
				else if (mykey == 65507 || mykey == 65508)
					xosi_Ctrl = 1;
			}
			if (cmd == '\026')
				xosi_paste();
			else if (cmd)
				useri_keychar(cmd, 0, 0);
			break;
		case KeyRelease:
			res = XLookupString(&event.xkey,
					    (PChar)h, sizeof(h),
					    &mykey, &mystat);
			if (res == 0) {
				if (mykey == 65505 || mykey == 65506)
					xosi_Shift = 0;
				else if (mykey == 65507 || mykey == 65508)
					xosi_Ctrl = 0;
			}
			break;
		case ConfigureNotify:
			XGetWindowAttributes(dis, mainwin.win, &attr);
			useri_resizewin(attr.width, attr.height, 0);
			break;
		case Expose:
			if (xosi_newxsize == 0)
				useri_refreshwin();
			break;
		case MotionNotify:
			useri_mousemove(event.xmotion.x, event.xmotion.y);
			break;
		case ButtonRelease:
			useri_mouserelease();
			break;
		case SelectionNotify:
			pasteget(dis,
				 mainwin.win,
				 event.xselection.property, &nn, &pstr);
			if ((char *)pstr) {
				tmp = (int32_t)nn - 1;
				res = 0;
				if (res <= tmp)
					for (;; res++) {
						cmd = pstr[res];
						if ((uint8_t)cmd >= ' ' &&
						    (uint8_t)cmd <= '\201') {
							useri_keychar(cmd,
								      1, 0);
						}
						if (res == tmp)
							break;
					}
				XFree((XVaNestedList)pstr);
			}
			break;
		case SelectionRequest:
			sendcopypasteevent(event.xselectionrequest);
			break;
		case SelectionClear:
			useri_clrcpmarks();
			break;
		case ClientMessage: /* window close event */
			if (event.xclient.data.l[0] == wclose) {
				aprsdecode_quit = 1;
				fprintf(stderr, "Main Window Close");
			}
			break;
		}
	}
}

static void addsock(int32_t fd, char wtoo)
{
	if (fd >= 0) {
		fdsetr((uint32_t)fd);
		if (wtoo) fdsetw((uint32_t)fd);
	}
}

extern void xosi_CheckProg(struct xosi_PROCESSHANDLE * chpid)
{
	int32_t ret;
	int32_t stat;
	if (chpid->runs) {
		ret = waitpid(chpid->pid, &stat, WNOHANG);
		if (ret == chpid->pid)
			chpid->exitcode = stat;
		if (ret < 0)
			chpid->runs = 0;
	}
}

#define xosi_ARGS 100
#define xosi_QUOTT "\""

extern void xosi_StartProg(
		char name[], uint32_t name_len,
		struct xosi_PROCESSHANDLE *chpid)
{
	char pgmname[1025];
	char argt[100][1025];
	X2C_pCHAR *args;
	char *arga[100];
	char quot;
	uint32_t a;
	uint32_t len;
	uint32_t j;
	uint32_t i;

	X2C_PCOPY((void **)&name,name_len);

	chpid->pid = fork();
	if (chpid->pid == 0) {
		/* child */
		len = strnlen(name, name_len);
		i = 0;
		j = 0;
		quot = 0;
		while (i < len && (quot || (uint8_t)name[i] > ' ')) {
			if (name[i] == '\"') {
				quot = !quot;
			} else if (j < 1024) {
				pgmname[j] = name[i];
				j++;
			}
			i++;
		}
		pgmname[j] = 0;

		quot = 0;
		while (i > 0 && name[i-1] != '/') {
			if (name[i] == '\"')
				quot = !quot;
			i--;
		}
		/* program name without path */
		for (a = 0; a <= 99; a++) {
			while (i < len && name[i] == ' ')
				i++;
			j = 0;
			while (i < len && (quot || (uint8_t)name[i] > ' ')) {
				if (name[i] == '\"') {
					quot = !quot;
				} else if (j < 1024) {
					argt[a][j] = name[i];
					j++;
				}
				i++;
			}
			argt[a][j] = 0;

			if (j > 0)
				arga[a] = (char *)argt[a];
			else
				arga[a] = 0;
		}

		args = (X2C_pCHAR *)arga;
		execv(pgmname, args);

		X2C_ABORT();
	}
	chpid->runs = (chpid->pid >= 0); /* ok or fork error */ /* parent */
	X2C_PFREE(name);
}

extern void xosi_StopProg(struct xosi_PROCESSHANDLE * chpid)
{
	if (chpid->runs)
		kill(chpid->pid, SIGTERM);
}

extern void xosi_Eventloop(uint32_t timeout)
{
	aprsdecode_pTCPSOCK acttmp;
	aprsdecode_pTCPSOCK acttcp;
	int32_t i;
	uint32_t tus;
	uint32_t ts;
	uint32_t t;

	fdclr(); /* clear select bitset */
	for (i = 0; i <= 3; i++) {
		addsock(aprsdecode_udpsocks0[i].fd, 0); /* set udp ports */
	}
	addsock(disfd, 0); /* x11 fd */
	acttcp = aprsdecode_tcpsocks;
	while (acttcp) {
		addsock(acttcp->fd, acttcp->tlen > 0); /* set tcp ports */
		acttcp = acttcp->next;
	}

	ts = 0;
	tus = timeout;
	selectrwt(&ts, &tus);
	timesum += timeout-tus;
	if (timesum >= timeout) {
		useri_timerevent();
		timesum = 0;
	}

	xosi_xevent();
	checkbuttons();
	t = osic_time();
	if (aprsdecode_lastlooped + 5 < t)
		aprsdecode_rxidle = 0;
	else
		aprsdecode_rxidle++;

	aprsdecode_lastlooped = t;
	for (i = 0; i <= 3; i++) {
		if (aprsdecode_udpsocks0[i].fd >= 0 &&
		    issetr((uint32_t)aprsdecode_udpsocks0[i].fd))
			aprsdecode_udpin((uint32_t)i);
	}

	acttcp = aprsdecode_tcpsocks;
	while (acttcp) {
		if (acttcp->fd >= 0 && issetw((uint32_t)acttcp->fd))
			aprsdecode_tcpout(acttcp);

		if (acttcp->fd >= 0 && issetr((uint32_t)acttcp->fd))
			aprsdecode_tcpin(acttcp);

		if (acttcp->fd < 0) {
			/* connection lost */
			aprsdecode_tcpclose(acttcp, 1);
			if (acttcp == aprsdecode_tcpsocks) {
				aprsdecode_tcpsocks = aprsdecode_tcpsocks->next;
			} else {
				acttmp = aprsdecode_tcpsocks;
				while (acttmp->next != acttcp)
					acttmp = acttmp->next;
				acttmp->next = acttcp->next;
			}

			useri_debugmem.mon -= sizeof(struct aprsdecode_TCPSOCK);
			osic_free((char **)&acttcp,
				   sizeof(struct aprsdecode_TCPSOCK));
			acttcp = aprsdecode_tcpsocks;
		} else {
			acttcp = acttcp->next;
		}
	}
}

extern void xosi_BEGIN(void)
{
	static int xosi_init = 0;

	if (xosi_init)
		return;

	xosi_init = 1;
	osi_BEGIN();
	useri_BEGIN();
	aprsstr_BEGIN();
	aprsdecode_BEGIN();
	xosi_headmh[0] = 0;
	xosi_cutbuffer.text = 0;
}
