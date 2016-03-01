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
#include <keysym.h>
#include <unistd.h>
#ifndef signal_H_
#include "signal.h"
#endif
#include <wait.h>
#ifndef beep_H_
#include "beep.h"
#endif
#include <X.h>
#include <Xutil.h>
#include <Xlib.h>
#include <cursorfont.h>
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



struct xosi__D0 xosi_gammalut[1024];




unsigned long xosi_newxsize;
unsigned long xosi_newysize;


struct xosi__D1 xosi_xmouse;
char xosi_zooming;
char xosi_pulling;
char xosi_Shift;
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
   unsigned long win; /* My window */
   PtrVisual pvis;
   PtrXImage ximage0;
   unsigned long redmask;
   unsigned long greenmask;
   unsigned long bluemask;
   long redshift;
   long greenshift;
   long blueshift;
   unsigned long Black;
   unsigned long White;
   unsigned long bitperpixel;
   GC gc;
   unsigned long pixmap;
   long screen; /* Default screen number */
   PtrScreen screenp;
   XFontStruct fontinfo;
   unsigned long xsizen;
   unsigned long ysizen;
};


struct _0 {
   unsigned short * Adr;
   size_t Len0;
};

static struct _0 * xbuf016;

static long xbuf0size;

static unsigned long xbufxsize;

static unsigned long xbufysize;

static struct XWIN mainwin;

static PtrDisplay dis; /* Connection to the X server */

static long disfd;

static union _XEvent event;

static unsigned long mykey;

static struct _XComposeStatus mystat;

static unsigned long timesum;

static unsigned long cursor;

static char cursorset;


static unsigned long Gamma(unsigned long c, float g)
{
   if (c==0UL) return 0UL;
   if (c<1024UL) {
      return (unsigned long)X2C_TRUNCC(osic_exp(X2C_DIVR(osic_ln(X2C_DIVR((float)
                c,1024.0f)),g))*255.5f,0UL,X2C_max_longcard);
   }
   return 255UL;
} /* end Gamma() */


extern void xosi_Gammatab(float g)
{
   unsigned long ga;
   unsigned long i;
   struct XWIN * anonym;
   struct xosi__D0 * anonym0;
   { /* with */
      struct XWIN * anonym = &mainwin;
      for (i = 0UL; i<=1023UL; i++) {
         ga = Gamma(i, g);
         { /* with */
            struct xosi__D0 * anonym0 = &xosi_gammalut[i];
            anonym0->r = X2C_LSH((unsigned long)ga,32,
                anonym->redshift)&anonym->redmask;
            anonym0->g = X2C_LSH((unsigned long)ga,32,
                anonym->greenshift)&anonym->greenmask;
            anonym0->b = X2C_LSH((unsigned long)ga,32,
                anonym->blueshift)&anonym->bluemask;
         }
      } /* end for */
   }
} /* end Gammatab() */


static void WrHeadline(void)
{
   char h[61];
   char s[61];
   if (dis) {
      if ((unsigned char)xosi_headmh[0UL]<' ') {
         strncpy(s,"aprsmap by oe5dxl ",61u);
         aprsstr_IntToStr((long)xbufxsize, 1UL, h, 61ul);
         aprsstr_Append(s, 61ul, h, 61ul);
         aprsstr_Append(s, 61ul, "x", 2ul);
         aprsstr_IntToStr((long)xbufysize, 1UL, h, 61ul);
         aprsstr_Append(s, 61ul, h, 61ul);
         XStoreName(dis, mainwin.win, s);
      }
      else XStoreName(dis, mainwin.win, xosi_headmh);
      xosi_headmh[0UL] = 0;
   }
} /* end WrHeadline() */


static long MaxBit(unsigned long colmask)
{
   long i;
   i = 31L;
   while (i>0L && !X2C_IN(i,32,colmask)) --i;
   return i+1L;
} /* end MaxBit() */


static void allocxbufw(struct XWIN * w, unsigned long xsizeh,
                unsigned long ysizeh)
{
   struct XWIN * anonym;
   /*WrInt(xsizeh, 1);WrStr(" x ");WrInt(ysizeh, 1); WrLn; */
   { /* with */
      struct XWIN * anonym = w;
      xbufxsize = xsizeh;
      xbufysize = ysizeh;
      if (anonym->ximage0) {
         useri_debugmem.screens -= (unsigned long)xbuf0size;
         osic_free((X2C_ADDRESS *) &xbuf016, (unsigned long)xbuf0size);
         xbuf016 = 0;
         anonym->ximage0->data = 0;
         XDestroyImage(anonym->ximage0); /* xbuf0 deallocated too */
      }
      xbuf0size = (long)(2UL*ysizeh*xsizeh);
      if (anonym->bitperpixel>16UL) xbuf0size = xbuf0size*2L;
      osic_alloc((X2C_ADDRESS *) &xbuf016, (unsigned long)xbuf0size);
      useri_debugmem.req = (unsigned long)xbuf0size;
      useri_debugmem.screens += useri_debugmem.req;
      if (xbuf016==0) {
         osi_WrStrLn("initx: out of memory", 21ul);
         useri_wrheap();
         X2C_ABORT();
      }
      anonym->ximage0 = XCreateImage(dis, anonym->pvis, anonym->bitperpixel,
                2L, 0L, (X2C_ADDRESS)xbuf016, xsizeh, ysizeh, 32L, 0L);
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
      WrHeadline();
   }
} /* end allocxbufw() */


extern void xosi_allocxbuf(unsigned long xsize, unsigned long ysize)
{
   allocxbufw(&mainwin, xsize, ysize);
} /* end allocxbuf() */


static long MakeMainWin(char winname[], unsigned long winname_len,
                char iconname[], unsigned long iconname_len,
                unsigned long xsizeh, unsigned long ysizeh)
{
   struct XWIN * anonym;
   long MakeMainWin_ret;
   X2C_PCOPY((void **)&winname,winname_len);
   X2C_PCOPY((void **)&iconname,iconname_len);
   { /* with */
      struct XWIN * anonym = &mainwin;
      anonym->myhint.height = (long)ysizeh;
      anonym->myhint.width = (long)xsizeh;
      anonym->myhint.x = 1L;
      anonym->myhint.y = 1L;
      anonym->myhint.flags = 0xCUL;
      /* Connect to the X server */
      dis = XOpenDisplay("");
      if (dis==0) {
         osi_WrStrLn("Couldnt open display", 21ul);
         MakeMainWin_ret = -1L;
         goto label;
      }
      anonym->screen = DefaultScreen(dis);
      /*
         Black:= Xlib.BlackPixel(dis, screen);
         White:= Xlib.WhitePixel(dis, screen);
      */
      /* Create a window */
      anonym->win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),
                anonym->myhint.x, anonym->myhint.y,
                (unsigned long)anonym->myhint.width,
                (unsigned long)anonym->myhint.height, 1UL, 0UL, 0UL);
      if (anonym->win==0UL) {
         osi_WrStrLn("Couldnt open window", 20ul);
         MakeMainWin_ret = -1L;
         goto label;
      }
      anonym->pvis = DefaultVisual(dis, 0L);
      if (anonym->pvis==0) {
         osi_WrStrLn("no visual", 10ul);
         MakeMainWin_ret = -1L;
         goto label;
      }
      XSetStandardProperties(dis, anonym->win, winname, iconname, 0UL, 0, 0L,
                 &anonym->myhint);
      /*
       (* Create a graphic context *)
        gc:= XCreateGC(dis, win, 0, NIL);
      */
      anonym->redmask = (unsigned long)anonym->pvis->red_mask;
      anonym->greenmask = (unsigned long)anonym->pvis->green_mask;
      anonym->bluemask = (unsigned long)anonym->pvis->blue_mask;
      anonym->redshift = MaxBit(anonym->redmask)-8L;
      anonym->greenshift = MaxBit(anonym->greenmask)-8L;
      anonym->blueshift = MaxBit(anonym->bluemask)-8L;
      anonym->bitperpixel = (unsigned long)
                MaxBit(anonym->redmask|anonym->greenmask|anonym->bluemask);
      /*
        xbuf0size:=SIZE(XPIX)*ysizeh*xsizeh;
        IF bitperpixel>16 THEN xbuf0size:=xbuf0size*2 END;
        ALLOCATE(xbuf0, xbuf0size);
        IF xbuf0=NIL THEN WrStrLn("initx: out of memory"); RETURN -1 END;
      */
      anonym->ximage0 = 0;
      allocxbufw(&mainwin, xsizeh, ysizeh);
      /*
        ximage0:=Xlib.XCreateImage(dis, pvis, bitperpixel, ZPixmap, 0, xbuf0,
                 xsizeh, ysizeh, 32, 0);
      */
      /*
      WrHex(pvis^.blue_mask,9);
      WrHex(pvis^.green_mask,9);
      WrHex(pvis^.red_mask,9);
      WrCard(pvis^.bits_per_rgb,9);
      WrCard(bitperpixel,4);
      WrCard(xsizeh,9);
      WrCard(ysizeh,9);
      WrLn;
      */
      /*Xlib.XSelectInput(dis, Xlib.RootWindow(dis, mainwin.screen),
                StructureNotifyMask); */
      XSelectInput(dis, anonym->win, 0x2804FUL);
      XMapWindow(dis, anonym->win);
      /* Bring the window to the front */
      /*    Xlib.XMapRaised(dis, win); */
      /*
        XSync(dis, 0);
        XClearArea(dis,win,0,0,xsizeh+1,ysizeh+1,FALSE);
      */
      disfd = ConnectionNumber(dis);
      xosi_pulling = 0;
      cursorset = 0;
      MakeMainWin_ret = 0L;
   }
   label:;
   X2C_PFREE(winname);
   X2C_PFREE(iconname);
   return MakeMainWin_ret;
} /* end MakeMainWin() */


extern void xosi_closewin(void)
{
   XCloseDisplay(dis);
} /* end closewin() */


extern long xosi_InitX(char winname[], unsigned long winname_len,
                char iconname[], unsigned long iconname_len,
                unsigned long xsizeh, unsigned long ysizeh)
{
   long xosi_InitX_ret;
   X2C_PCOPY((void **)&winname,winname_len);
   X2C_PCOPY((void **)&iconname,iconname_len);
   xbuf016 = 0;
   if (MakeMainWin(winname, winname_len, iconname, iconname_len, xsizeh,
                ysizeh)<0L) {
      xosi_InitX_ret = -1L;
      goto label;
   }
   xosi_Shift = 0;
   xosi_InitX_ret = 0L;
   label:;
   X2C_PFREE(winname);
   X2C_PFREE(iconname);
   return xosi_InitX_ret;
} /* end InitX() */


extern void xosi_getscreenbuf(X2C_ADDRESS * adr, unsigned long * xsize,
                unsigned long * ysize, unsigned long * incadr)
{
   *adr = (X2C_ADDRESS)xbuf016;
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


extern void xosi_beep(long lev, unsigned long hz, unsigned long ms)
{
   XKeyboardControl val;
   if (hz>0UL && ms>0UL) {
      if (cbell((long)hz, (long)ms)<0L && dis) {
         val.bell_pitch = (long)hz;
         val.bell_duration = (long)(ms/2UL);
         XChangeKeyboardControl(dis, (unsigned long)0xCUL,
                (PAXKeyboardControl) &val);
         XBell(dis, lev);
      }
   }
} /* end beep() */


extern void xosi_Umlaut(char c[], unsigned long c_len)
/* umlaut converter */
{
   switch ((unsigned long)(unsigned char)c[0UL]) {
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


extern void xosi_paste(void)
{
   long i;
   unsigned long cw;
   unsigned long pa;
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
      i = XConvertSelection(dis, 1UL, pa, pa, mainwin.win, 0UL);
   }
} /* end paste() */


extern void xosi_cutpaste(void)
{
   /*Xlib.XStoreBytes(dis, ADR(s), Length(s)); */
   /*Xlib.XRotateBuffers(dis, 1); */
   XSetSelectionOwner(dis, 1UL, mainwin.win, 0UL);
   if (XGetSelectionOwner(dis, 1UL)!=mainwin.win) useri_clrcpmarks();
} /* end cutpaste() */

typedef unsigned char * CP;


static void sendcopypasteevent(XSelectionRequestEvent xreq)
{
   XSelectionEvent ev;
   long ret;
   CP cp;
   /*WrInt(xreq.requestor, 15); WrInt(xreq.selection, 15); */
   /*WrInt(xreq.target, 15); WrInt(xreq.property, 15); WrInt(xreq.time, 15);
                WrStrLn("esrq"); */
   if (xreq.target==XInternAtom(dis, "STRING", 0) || xreq.target==294UL) {
      cp = (CP)xosi_cutbuffer.text;
      XChangeProperty(dis, xreq.requestor, xreq.property, xreq.target, 8L,
                0L, cp, (long)xosi_cutbuffer.cutlen);
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
   ret = XSendEvent(xreq.display, xreq.requestor, 1L, 0L, (PtrXEvent) &ev);
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

extern void xosi_setxwinsize(long x, long y)
{
   XMoveResizeWindow(dis, mainwin.win, 0L, 0L, (unsigned long)x,
                (unsigned long)y);
} /* end setxwinsize() */


static void maximize(void)
{
   XWindowAttributes xwa;
   XGetWindowAttributes(dis, DefaultRootWindow(dis), &xwa);
   useri_resizewin(xwa.width, xwa.height, 1);
} /* end maximize() */


extern void xosi_sethand(unsigned char t)
{
   long curs;
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
         cursor = XCreateFontCursor(dis, (unsigned long)curs);
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
   unsigned long c;
   unsigned long r;
   long wy;
   long wx;
   long y;
   long x;
   unsigned long m;
   if ((xosi_pulling && XQueryPointer(dis, mainwin.win, &r, &c, &x, &y, &wx,
                &wy, &m)) && ((unsigned long)m&0x300UL)==0UL) {
      xosi_sethand(xosi_cOFF);
   }
} /* end checkbuttons() */


extern void xosi_xevent(void)
{
   char h[256];
   XWindowAttributes attr;
   long res;
   char cmd;
   long ii;
   unsigned long nc;
   unsigned long nn;
   unsigned long atom1;
   PAUChar pstr;
   long tmp;
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
         }
         if (cmd=='\026') xosi_paste();
         else if (cmd) useri_keychar(cmd, 0, 0);
         break;
      case 3L:
         res = XLookupString(&event.xkey, (PChar)h, 256L, &mykey, &mystat);
         if (res==0L) {
            if (mykey==65505UL || mykey==65506UL) xosi_Shift = 0;
         }
         break;
      case 22L:
         /*WrInt(event.xconfigure.window, 10); WrInt(mainwin.win, 10); */
         /*WrStrLn(" ConfigureNotify"); */
         XGetWindowAttributes(dis, mainwin.win, &attr);
         useri_resizewin(attr.width, attr.height, 0);
         break;
      case 12L:
         useri_refreshwin();
         break;
      case 6L:
         /*          Pull(event.xmotion.x, event.xmotion.y, 1); */
         useri_mousemove(event.xmotion.x, event.xmotion.y);
         break;
      case 5L:
         useri_mouserelease();
         break;
      case 31L:
         /*        pulloff; */
         /*          res:=Xlib.XConvertSelection(dis, 1, 31, 31, mainwin.win,
                 CurrentTime); */
         res = XGetWindowProperty(dis, mainwin.win,
                event.xselection.property, 0L, 64L, 0L, 0UL, &atom1, &ii,
                &nn, &nc, &pstr);
         if ((X2C_ADDRESS)pstr) {
            tmp = (long)nn-1L;
            res = 0L;
            if (res<=tmp) for (;; res++) {
               cmd = pstr[res];
               if ((unsigned char)cmd>=' ' && (unsigned char)
                cmd<=(unsigned char)'\201') useri_keychar(cmd, 1, 0);
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
      } /* end switch */
   }
} /* end xevent() */


static void addsock(long fd, char wtoo)
/* insert socket in fdset for select */
{
   if (fd>=0L) {
      fdsetr((unsigned long)fd);
      if (wtoo) fdsetw((unsigned long)fd);
   }
} /* end addsock() */


extern void xosi_CheckProg(struct xosi_PROCESSHANDLE * chpid)
{
   long ret;
   long stat;
   if (chpid->runs) {
      ret = waitpid(chpid->pid, &stat, WNOHANG);
      if (ret==chpid->pid) chpid->exitcode = stat;
      if (ret<0L) chpid->runs = 0;
   }
} /* end CheckProg() */

#define xosi_ARGS 100

#define xosi_QUOT "\""


extern void xosi_StartProg(char name[], unsigned long name_len,
                struct xosi_PROCESSHANDLE * chpid)
{
   long e;
   char pgmname[1025];
   char argt[100][1025];
   X2C_pCHAR * args;
   X2C_ADDRESS arga[100];
   char quot;
   unsigned long a;
   unsigned long len;
   unsigned long j;
   unsigned long i;
   X2C_PCOPY((void **)&name,name_len);
   /*WrStr("start: "); WrStrLn(name); */
   chpid->pid = fork();
   if (chpid->pid==0L) {
      /* child */
      len = aprsstr_Length(name, name_len);
      i = 0UL;
      j = 0UL;
      quot = 0;
      while (i<len && (quot || (unsigned char)name[i]>' ')) {
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
         while (i<len && (quot || (unsigned char)name[i]>' ')) {
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
      e = execv(pgmname, args);
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


extern void xosi_Eventloop(unsigned long timeout)
{
   aprsdecode_pTCPSOCK acttmp;
   aprsdecode_pTCPSOCK acttcp;
   long i;
   long ret;
   unsigned long tus;
   unsigned long ts;
   unsigned long t;
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
   ret = selectrwt(&ts, &tus);
   timesum += timeout-tus;
   if (timesum>=timeout) {
      useri_timerevent();
      if (xosi_headmh[0UL]) WrHeadline();
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
      if (aprsdecode_udpsocks0[i].fd>=0L && issetr((unsigned long)
                aprsdecode_udpsocks0[i].fd)) {
         aprsdecode_udpin((unsigned long)i);
      }
   } /* end for */
   acttcp = aprsdecode_tcpsocks;
   while (acttcp) {
      if (acttcp->fd>=0L && issetw((unsigned long)acttcp->fd)) {
         ret = aprsdecode_tcpout(acttcp);
      }
      if (acttcp->fd>=0L && issetr((unsigned long)acttcp->fd)) {
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
         osic_free((X2C_ADDRESS *) &acttcp,
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

