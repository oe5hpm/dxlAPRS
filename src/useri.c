/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef useri_H_
#include "useri.h"
#endif
#define useri_C_
#ifndef maptool_H_
#include "maptool.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif
#ifndef xosi_H_
#include "xosi.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef FileSys_H_
#include "FileSys.h"
#endif
#ifndef filesize_H_
#include "filesize.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef Storage_H_
#include "Storage.h"
#endif
#ifndef TimeConv_H_
#include "TimeConv.h"
#endif
#ifndef aprstext_H_
#include "aprstext.h"
#endif
#ifndef aprstat_H_
#include "aprstat.h"
#endif



unsigned long useri_newxsize;
unsigned long useri_newysize;
struct useri_MOUSEPOS useri_xmouse;
char useri_listwin;


struct useri__D0 useri_debugmem;
char useri_beaconediting;
char useri_reloadmap;
char useri_refresh;
char useri_beaconed;
char useri_maximized;
char useri_isblown;
long useri_nextmsg;
maptool_pIMAGE useri_panoimage;
/* aprsmap user interface */
#define useri_MAPLOADER "gm.sh"

#define useri_SHOTFORMATS "(.ppm/.png)"

#define useri_SERIAL1 "udpflex -U :9002:9001 -i kiss.txt -u -t /dev/ttyUSB0:9\
600"

#define useri_SERIAL2 "afskmodem -f 22050 -C 0 -p /dev/ttyS0 0 -M 0 -U 127.0.\
0.1:9002:9001 -m 0"

static char useri_TICKERHEADLINE = 0;
                /* some window managers do not free window headline mem */

#define useri_POIFILENAME "poi.txt"

#define useri_MYPOIFILENAME "mypoi.txt"

#define useri_MAXXSIZE 32000

#define useri_MAXYSIZE 32000

#define useri_MINXSIZE 100

#define useri_MINYSIZE 50

#define useri_DEFAULTXSIZE 600

#define useri_DEFAULTYSIZE 400

#define useri_COLOURS 100
/* colour chooser size */

#define useri_COLOURSFRAME 5

#define useri_COLOURLUMA 10

#define useri_ONLINESETUPURL 5
/* menu line */

#define useri_RFCFGRFPORTS 12
/* menu line */

#define useri_MAPCFGLINE 7
/* menu line */

#define useri_MAPMOVLINE 6
/* menu line */

#define useri_TOGGMOUSESHOW 11
/* menu line */

#define useri_TEXTINFOWINID 2
/* raw + decoded text window id */

#define useri_HOVERINFOWINID 7

#define useri_WXWINID 20
/* set of wx graphics windows ids */

#define useri_MOUSESHOWID 19
/* win d mouseshow */

#define useri_MAXKNOBS 30
/* menu lines */

#define useri_CONFIGWIDTH 200

#define useri_HELPDEPTH 10
/* history size of help hyperlinks */

#define useri_MAXSUBKNOBS 20
/* menu columns */

#define useri_HINTDELAY 2
/* wait before open hint */

#define useri_MINHINTTIME 12
/* min opentime for hint window */

#define useri_HINTREADSPEED 15
/* characters per second extra readtime for hints */

#define useri_LISTMAXLEN 2000
/* lines in list window */

#define useri_TEXTWINXMARGIN 4

#define useri_PULLFRAME 3
/* frame width to change window size */

#define useri_MINH 100

#define useri_POTIOFF 8

#define useri_POTWIDTH 220

#define useri_WDIV 4

#define useri_TDIV 409

#define useri_CONFIGLINES 9

#define useri_TEXTBUTTONMARGIN 4

#define useri_MINSROLLKNOB 8

#define useri_SCRMUL 256

#define useri_MOVERSIZE 8

#define useri_MSGPOPID 211

#define useri_STATUSID 254

#define useri_DEFLTEXT 80
/* default text luminance */

#define useri_DEFLOBJ 70
/* default objects luminance */

#define useri_DEFLMAP 30
/* default map luminance */

#define useri_DEFLTRACK 80
/* default tracks luminance */

#define useri_DEFLSYM 90
/* default symbols luminance */

#define useri_DEFLWAY 70
/* default waypoints luminance */

#define useri_BEACONEDI 3
/* beacon edit shortcut knob */

#define useri_PULLMENUY 8
/* ysize of menu from top to be pulled */

#define useri_STARTPULLMOVE 2
/* way to move after leftclick before pull start */

#define useri_STARTPULLZOOM 5
/* way to move after shift leftclick before zooming */

#define useri_HINTJITTER 8

#define useri_SPOO "\365\365"

#define useri_MOP0 "\346"
/* proportional off */

#define useri_MOP1 "\347"
/* color background yellow */

#define useri_MOP2 "\350"
/* color background 1 button */

#define useri_MOP3 "\351"
/* highlite off */

#define useri_MOP4 "\352"
/* color background whole line */

#define useri_MOP5 "\353"
/* color background red */

#define useri_MOP6 "\354"

#define useri_MOP7 "\355"
/* color background blue */

#define useri_MOP8 "\356"
/* dimm text */

#define useri_MOP9 "\357"
/* hilite whole line */

#define useri_iMAXIMIZED "M"

#define useri_iICON "I"

#define useri_STATUSHINT 17

/*      OVERSHOOT=WHITELEVEL*3;            (* fast white level limiter via table *) */
#define useri_TRANSPARENCE 65535

#define useri_NL "\012"

#define useri_POTIFPS 10

#define useri_TOOLBARX 187

#define useri_TOOLBARID 255

#define useri_LOGMENUID 227

#define useri_SENDMSGID 228

#define useri_CONFIGEDITID 229

#define useri_HINTWINID 220

#define useri_FIXPOSIDS 200
/* window id > is fixed position */

#define useri_BUBBLWINID 201

#define useri_DIGIEDITID 224

#define useri_BEACONEDITID 226

#define useri_LISTWINID 225

#define useri_PANOWINID 202

#define useri_OVERLAYID 203
/* overlay symbol chooser id */
/*      ZOOMTO=7C;  */

/*      BLOWOK=11C; */
#define useri_CURSESC "\033"

#define useri_MAPCLICK "\201"

#define useri_SYMCLICK "\202"

#define useri_TEXTCLICK "\203"

#define useri_TRACKCLICK "\204"

#define useri_ADDWATCH "\205"

#define useri_TOOLBAR "\206"

#define useri_DELWATCH "\210"

#define useri_CONFIGM2 "\211"

#define useri_NOTYET "\212"

#define useri_TOOLOFF "\214"

#define useri_TOOLON "\215"
/*    CMDRDLOG=216C;             (* in def *) */

#define useri_FILEMENU "\217"

#define useri_TOOLSMENU "\220"

#define useri_HELPMENU "\221"

#define useri_LISTMENU "\222"

#define useri_CURSDOWNM "\224"

#define useri_CURSUPPM "\225"

#define useri_CURSDOWNPM "\226"

#define useri_CONFSAVE "\227"

#define useri_CONFLOAD "\230"

#define useri_CONFCLICK "\231"

#define useri_WXCLICK "\232"

#define useri_SCREENSHOT "\233"

#define useri_CONFSYM "\234"

#define useri_FILERDLOG "\235"
/*    CMDDOWNLOAD=237C;          (* in def *) */

#define useri_STOPDOWNLOAD "\240"

#define useri_DOWNLOADMAP "\241"
/*    CMDSTARTDOWNLOAD=242C;     (* in def *) */

#define useri_MSGOK "\243"

#define useri_CONFBSYM "\244"
/*    CMDFIND=245C;              (* in def *) */

#define useri_LISTDEBUG "\246"

#define useri_LISTINGS "\247"

#define useri_LISTUSERMSGS "\250"

#define useri_SETMARKERS "\251"

#define useri_SETMARKERSMAP "\252"

#define useri_CMDRELOADMAP "\236"

#define useri_CMDADDBEACON "\223"

#define useri_CMDDOBEACON "\213"

#define useri_CMDFPS "\006"

#define useri_CMDWATCHDO "\253"

#define useri_CMDMSG "\254"

#define useri_CMDSENDMSG "\255"

#define useri_CMDCOMPOMSG "\256"

#define useri_CMDMANMSG "\257"

#define useri_CMDDOMSG "\260"

#define useri_CMDSAVETRACK "\261"

#define useri_CMDCONFIG "\262"

#define useri_CMDCONFIGMAN "\263"

#define useri_CMDMAINCFG "\264"

#define useri_CMDMANFILT "\265"

#define useri_CMDCONFIGDEL "\266"

#define useri_CMDCONFDELMAN "\267"

#define useri_CMDONLINE "\270"

#define useri_CMDRF "\271"

#define useri_CMDMAP "\272"

#define useri_CMDDOTIMERS "\273"

#define useri_CMDDOMAP "\274"

#define useri_CMDDOMOVE "\275"

#define useri_CMDONCLICK "\276"

#define useri_CMDSTATUS "\277"

#define useri_CMDRAIN "\300"

#define useri_CMDMAPADD "\301"

#define useri_CMDMAPCHOOSE "\302"

#define useri_CMDMAPDIR "\303"

#define useri_CMDDOBEEP "\304"

#define useri_CMDHELPTEXT "\305"

#define useri_CMDDOHELP "\306"
/*    CMDZOOMSQUARE=307C; (* in def *) */

/*    CMDFZOOMIN=310C;    (* in def *) */
/*    CMDFZOOMOUT=311C;   (* in def *) */
/*    CMDVIDEO=312C;      (* in def *) */
#define useri_CMDGEOPROFIL "\313"

#define useri_CMDDIGILINE "\314"

#define useri_CMDDIGIED "\315"

#define useri_CMDCOLOURS "\316"

#define useri_CMDCOLOURCLICK "\317"

#define useri_CMDENTEROK "\320"

#define useri_CMDCOLOURSGEO "\321"

#define useri_CMDFILE "\323"

#define useri_CMDSPECIALMAP "\324"

#define useri_CONFONESYM "\325"

#define useri_CHOOSEONESYM "\326"

#define useri_HOVERCLICK "\327"

#define useri_CMDHEARD "\330"

#define useri_CMDPOLIGON "\331"

#define useri_CMDCLONE "\332"

struct CONFLINE;

typedef struct CONFLINE * pCONFLINE;


struct CONFLINE {
   pCONFLINE next;
   unsigned char active;
   char line[201];
};

enum CONFIGTYP {useri_cBOOL, useri_cLINE, useri_cBLINE, useri_cLIST,
                useri_cBLIST};


struct CONFIG;


struct CONFIG {
   unsigned char typ;
   char updated; /* updated by editor */
   char title[31];
   unsigned short curspos;
   unsigned short width;
   unsigned char on;
   unsigned long hint;
   pCONFLINE lines;
};

enum BACKGROUND {useri_bBLACK, useri_bCOLOR, useri_bTRANSP};


struct MENU;

typedef struct MENU * pMENU;


struct MENU {
   pMENU next;
   maptool_pIMAGE image;
   unsigned long yknob;
   unsigned long x0;
   unsigned long y00;
   unsigned long xsize;
   unsigned long ysize;
   unsigned long nowx; /* real position while on screen */
   unsigned long nowy;
   unsigned long oldknob;
   unsigned long hiknob;
   unsigned long oldsub;
   unsigned long wid; /* window class id */
   unsigned long scroll; /* lines in window shifted */
   unsigned char minnormmax; /* minimized normal maximized */
   unsigned char sizeconf; /* store to this config when resized */
   unsigned char pullconf; /* pull menu on and store in config */
   unsigned long pullyknob; /* click size from top to move window */
   unsigned long scrysize; /* lines collumns */
   unsigned long scrxsize;
   long scrx; /* pixel lines*linehight */
   long scry;
   void ( *redrawproc)(pMENU);
   unsigned long timeout;
   char notoverdraw;
   char drawn;
   char hidden;
   char fullclamp; /* clamp win for cursor scrolling */
   char saveimage;
   unsigned char background;
   char submen[31];
   char cmds[31];
   unsigned short helpindex[31];
   unsigned char confidx[31];
   unsigned short subk[31][21];
   unsigned long nohilite; /* use keyboard */
   unsigned long noprop;
   unsigned long clampkb;
};

struct LISTLINE;

typedef struct LISTLINE * pLISTLINE;


struct LISTLINE {
   pLISTLINE next;
   struct aprspos_POSITION position; /* to show marker on map */
   aprsdecode_MONCALL call; /* to sort by call */
   unsigned long time0; /* to sort by time */
   char withport; /* first char is a port to colorize */
   unsigned long markcp;
   unsigned long startcp; /* c/p highlight */
   unsigned long endcp;
   unsigned long len; /* for dealloc */
   char text[501];
};

struct LISTBUFFER;


struct LISTBUFFER {
   unsigned long newlines; /* count lines to compensate v scroll position */
   unsigned long listlinecnt;
   char sortby;
   unsigned char isicon;
   unsigned char xy;
   unsigned char size;
   pLISTLINE listlines;
   char listwintitle[31];
};
/*       winid:CARDINAL; */

static char leftbutton;

static char rightbutton;

static char overtype;

static char sndmsg;

static char digied;

static char hinton;

static char hoveropen;

static char poioff;

static maptool_pIMAGE redrawimg;

static unsigned long clampedline;
                /* on click to a clamped text line set cursor*/

static unsigned long dellog;

static unsigned long focuswid;

/* window class last touched with pointer */
static unsigned long helpdepth;

static unsigned long configedit;

static unsigned long mouseshowcnt;

static pMENU menus;

static unsigned long hinttime;

static struct useri_MOUSEPOS hintmouse;
                /* store mouse to close hover hints if moved a radius away */

static unsigned long hintnum;

static char helpscroll[11][41];

static struct CONFIG configs[153];

static char pullmenuwhat;

/* store what should be moved on a menu at pull startpoint */
static long pullmenuwid;

static long cpendx;

static long cpendy;

static long pullmenux;

static long pullmenuy;

static struct LISTBUFFER listbuffer;

static struct LISTBUFFER monbuffer;

static struct maptool_PANOWIN panowin;

static char digiedline[201];


static long inclim(long n, long d, long min0)
/* inc CARD INT with limit to min */
{
   n += d;
   if (n<min0) n = min0;
   return n;
} /* end inclim() */


static void disposeimg(maptool_pIMAGE * image)
{
   if (*image) {
      useri_debugmem.screens -= (*image)->Len1*(*image)->Size1;
      X2C_DYNDEALLOCATE((char **)image);
      *image = 0;
   }
} /* end disposeimg() */


extern void useri_allocimage(maptool_pIMAGE * image, long x, long y,
                char save)
{
   size_t tmp[2];
   if (save) {
      if (*image) disposeimg(image);
      else {
         *image = useri_panoimage;
         useri_panoimage = 0;
      }
   }
   if (*image && (x!=(long)(((*image)->Len1-1)+1UL) || y!=(long)(((*image)
                ->Len0-1)+1UL))) disposeimg(image);
   if (*image==0) {
      X2C_DYNALLOCATE((char **)image,sizeof(struct maptool_PIX),
                (tmp[0] = (size_t)x,tmp[1] = (size_t)y,tmp),2u);
      useri_debugmem.req = (*image)->Len1*(*image)->Size1;
      useri_debugmem.screens += useri_debugmem.req;
      if (*image==0) {
         osi_WrStrLn("image out of memory", 20ul);
         useri_wrheap();
         X2C_ABORT();
      }
      if (aprsdecode_verb) useri_wrheap();
   }
   else maptool_clr(*image);
} /* end allocimage() */


static long daylylogsize(const char fname[], unsigned long fname_len,
                char lenstr[], unsigned long lenstr_len)
{
   aprsdecode_FILENAME fn;
   aprsdecode_FILENAME fnd;
   unsigned long t;
   long fc;
   float size;
   long ret;
   aprsstr_Assign(fn, 1024ul, fname, fname_len);
   t = aprsdecode_realtime;
   size = 0.0f;
   ret = -1L;
   do {
      memcpy(fnd,fn,1024u);
      aprstext_logfndate(t, fnd, 1024ul);
      if (X2C_STRCMP(fnd,1024u,fn,1024u)==0) return -1L;
      /* not dayly log */
      fc = osi_OpenRead(fnd, 1024ul);
      if (osi_FdValid(fc)) {
         ret = 0L;
         size = size+(float)Size(fc);
         osi_Close(fc);
      }
      t -= 86400UL;
   } while (t>=1388534400UL);
   /* oldest possible file */
   if (size<10000.0f) {
      aprsstr_IntToStr((long)aprsdecode_trunc(size), 1UL, lenstr,
                lenstr_len);
      aprsstr_Append(lenstr, lenstr_len, "B", 2ul);
   }
   else if (size<10000.0f) {
      aprsstr_IntToStr((long)aprsdecode_trunc(X2C_DIVR(size,1000.0f)), 1UL,
                lenstr, lenstr_len);
      aprsstr_Append(lenstr, lenstr_len, "kB", 3ul);
   }
   else {
      aprsstr_IntToStr((long)aprsdecode_trunc(X2C_DIVR(size,1.E+6f)), 1UL,
                lenstr, lenstr_len);
      aprsstr_Append(lenstr, lenstr_len, "MB", 3ul);
   }
   if (size!=0.0f) ret = 1L;
   return ret;
} /* end daylylogsize() */


extern long useri_guesssize(char fn[], unsigned long fn_len, char lenstr[],
                unsigned long lenstr_len)
/* return filsize in kb not exact >2gb */
{
   long fc;
   unsigned long byte;
   unsigned long pos;
   char b;
   char s[100];
   long ret;
   ret = daylylogsize(fn, fn_len, lenstr, lenstr_len);
   if (ret>=0L) return ret;
   /* dayly log */
   lenstr[0UL] = 0;
   fc = osi_OpenRead(fn, fn_len);
   if (!osi_FdValid(fc)) return -1L;
   pos = 0UL;
   for (;;) {
      osi_Seekcur(fc, 1000000000L); /* seek in 100mb steps for eof */
      if (osi_RdBin(fc, (char *) &b, 1u/1u, 1UL)!=1L) break;
      pos += 131072UL;
   }
   if (pos<2000000UL) {
      byte = Size(fc);
      pos = byte/1000UL;
   }
   osi_Close(fc);
   /* make text kb mb gb... */
   if (pos<100UL) {
      strncpy(s,"B",100u);
      pos = byte;
   }
   else {
      strncpy(s,"kB",100u);
      if (pos>4000UL) {
         pos = pos/1000UL;
         strncpy(s,"MB",100u);
      }
      if (pos>4000UL) {
         pos = pos/1000UL;
         strncpy(s,"GB",100u);
      }
   }
   aprsstr_CardToStr(pos, 0UL, lenstr, lenstr_len);
   aprsstr_Append(lenstr, lenstr_len, s, 100ul);
   return (long)pos;
} /* end guesssize() */


extern void useri_AddConfLine(unsigned char v, unsigned char act, char s[],
                unsigned long s_len)
{
   pCONFLINE pl;
   if (v<=useri_fEDITLINE) {
      /*WrInt(ORD(configs[v].on), 5);WrStrLn(configs[v].title); */
      configs[v].on = act;
      if (configs[v].typ) {
         if (s[0UL] || configs[v].typ<=useri_cBLINE) {
            /* not add list empty lines */
            pl = configs[v].lines;
            if (configs[v].typ>=useri_cLIST) {
               while (pl && !aprsstr_StrCmp(pl->line, 201ul, s, s_len)) {
                  pl = pl->next;
               }
            }
            if (pl==0) {
               /* not same text in table */
               Storage_ALLOCATE((X2C_ADDRESS *) &pl,
                sizeof(struct CONFLINE));
               if (pl==0) {
                  osi_WrStrLn("menu out of memory", 19ul);
                  useri_wrheap();
                  return;
               }
               memset((X2C_ADDRESS)pl,(char)0,sizeof(struct CONFLINE));
               pl->next = configs[v].lines;
               configs[v].lines = pl;
            }
            aprsstr_Assign(pl->line, 201ul, s, s_len);
            pl->active = act;
         }
      }
   }
/*WrStr(configs[v].title); WrStr(configs[v].lines^.line);
                WrInt(ORD(configs[v].lines^.active), 2); WrLn; */
} /* end AddConfLine() */


static void icfg(unsigned char v, const char s[], unsigned long s_len)
{
   useri_AddConfLine(v, 0U, s, s_len);
} /* end icfg() */


extern void useri_int2cfg(unsigned char cfg, long v)
{
   char s[31];
   aprsstr_IntToStr(v, 0UL, s, 31ul);
   icfg(cfg, s, 31ul);
} /* end int2cfg() */


extern void useri_clrconfig(void)
{
   memset((char *)configs,(char)0,sizeof(struct CONFIG [153]));
} /* end clrconfig() */


static void initc(unsigned char v, const char tit[], unsigned long tit_len,
                unsigned char t, const char text[], unsigned long text_len,
                char o, unsigned long hi)
{
   struct CONFIG * anonym;
   { /* with */
      struct CONFIG * anonym = &configs[v];
      aprsstr_Assign(anonym->title, 31ul, tit, tit_len);
      anonym->typ = t;
      anonym->on = o;
      anonym->hint = hi+20000UL;
   }
   if (text[0UL]) useri_AddConfLine(v, o, text, text_len);
} /* end initc() */


static void initconfig1(void)
{
   initc(useri_fTFADE, "Time Fade Out Min", 18ul, useri_cLINE, "30", 3ul, 0,
                155UL);
   initc(useri_fTFULL, "Time Full Bright Min", 21ul, useri_cLINE, "360", 4ul,
                 0, 160UL);
   initc(useri_fLWAY, "Brightness Waypoint", 20ul, useri_cLINE, "75", 3ul, 0,
                 175UL);
   initc(useri_fTRANSP, "Menu Background", 16ul, useri_cBLINE, "60", 3ul, 1,
                180UL);
   initc(useri_fLTEXT, "Brightness Text", 16ul, useri_cLINE, "70", 3ul, 0,
                185UL);
   initc(useri_fLSYM, "Brightness Symbol", 18ul, useri_cLINE, "80", 3ul, 0,
                190UL);
   initc(useri_fLTRACK, "Brightness Track", 17ul, useri_cLINE, "65", 3ul, 0,
                195UL);
   initc(useri_fLOBJ, "Brightness Object", 18ul, useri_cLINE, "70", 3ul, 0,
                200UL);
   initc(useri_fLMAP, "Brightness Map", 15ul, useri_cLINE, "30", 3ul, 0,
                205UL);
   initc(useri_fLRF, "Brightness Heard Path", 22ul, useri_cLINE, "35", 3ul,
                0, 210UL);
   initc(useri_fNOMOV, "Brightness Nomover", 19ul, useri_cLINE, "9", 2ul, 0,
                215UL);
   initc(useri_fGAMMA, "Screen Gamma", 13ul, useri_cLINE, "2.2", 4ul, 0,
                225UL);
   initc(useri_fMOUSELOC, "Mouse-Over function", 20ul, useri_cBLINE, "9",
                2ul, 1, 240UL);
   initc(useri_fDUPDEL, "Filter delayed Waypoints Min", 29ul, useri_cLINE, "1\
80", 4ul, 0, 250UL);
   initc(useri_fARROW, "Show Coursearrow", 17ul, useri_cBOOL, "", 1ul, 1,
                255UL);
   initc(useri_fINVMOV, "Swap Left/Right Up/Down", 24ul, useri_cBOOL, "",
                1ul, 0, 260UL);
   initc(useri_fRULER, "Show Scale", 11ul, useri_cBOOL, "", 1ul, 1, 265UL);
   initc(useri_fKMHTIME, "Km/h Show Time", 15ul, useri_cLINE, "600", 4ul, 0,
                270UL);
   initc(useri_fTEMP, "Show Temperature", 17ul, useri_cBOOL, "", 1ul, 1,
                275UL);
   initc(useri_fWINDSYM, "Show Wind Vane", 15ul, useri_cBOOL, "", 1ul, 1,
                280UL);
   initc(useri_fALTMIN, "Altitude Show min m", 20ul, useri_cBLINE, "-10000",
                7ul, 0, 285UL);
   initc(useri_fMOVESTEP, "Map Move Stepwidth", 19ul, useri_cLINE, "10", 3ul,
                 0, 325UL);
   initc(useri_fZOOMSTEP, "Finezoom Stepwidth", 19ul, useri_cLINE, "0.1",
                4ul, 0, 330UL);
   initc(useri_fCLICKMAP, "Click Map", 10ul, useri_cLINE, "m", 2ul, 0,
                335UL);
   initc(useri_fCLICKSYM, "Click Sym", 10ul, useri_cLINE, "u", 2ul, 0,
                340UL);
   initc(useri_fCLICKWXSYM, "Click Wxsym", 12ul, useri_cLINE, "0111111u",
                9ul, 0, 345UL);
   initc(useri_fCLICKTEXT, "Click Text", 11ul, useri_cLINE, "H", 2ul, 0,
                350UL);
   initc(useri_fCLICKTRACK, "Click Track", 12ul, useri_cLINE, ".u", 3ul, 0,
                355UL);
   initc(useri_fHOVERSET, "Hover Set", 10ul, useri_cLINE, "u", 2ul, 0,
                356UL);
   initc(useri_fMARKPOS, "Marker1", 8ul, useri_cLINE, "", 1ul, 0, 420UL);
   initc(useri_fGEOPROFIL, "Geoprofile", 11ul, useri_cBOOL, "", 1ul, 0,
                421UL);
   /* do not change order */
   initc(useri_fCOLMAPTEXT, "Colour Maptext", 15ul, useri_cLINE, "100 100 100\
", 12ul, 0, 491UL);
   initc(useri_fCOLOBJTEXT, "Colour Objecttext", 18ul, useri_cLINE, "60 100 1\
00", 11ul, 0, 491UL);
   initc(useri_fCOLMENUTEXT, "Colour Menutext", 16ul, useri_cLINE, "100 100 1\
00", 12ul, 0, 491UL);
   initc(useri_fCOLMENUBACK, "Colour Background", 18ul, useri_cLINE, "100 100\
 100", 12ul, 0, 491UL);
   initc(useri_fCOLMARK1, "Colour Rf Marker1", 18ul, useri_cLINE, "100 20 0",
                 9ul, 0, 491UL);
   initc(useri_fCOLMARK2, "Colour Rf Marker2", 18ul, useri_cLINE,
                "0 100 100", 10ul, 0, 491UL);
/* do not change order */
} /* end initconfig1() */


static void initconfig(void)
{
   unsigned char i;
   char s[100];
   pCONFLINE ph;
   pCONFLINE pl;
   for (i = useri_fFIND;; i++) {
      pl = configs[i].lines;
      while (pl) {
         ph = pl->next;
         Storage_DEALLOCATE((X2C_ADDRESS *) &pl, sizeof(struct CONFLINE));
         pl = ph;
      }
      if (i==useri_fEDITLINE) break;
   } /* end for */
   useri_clrconfig();
   for (i = useri_fFIND;; i++) {
      configs[i].width = 201U;
      if (i==useri_fEDITLINE) break;
   } /* end for */
   initc(useri_fTPURGE, "Keep Data in Memory Min", 24ul, useri_cLINE, "1500",
                 5ul, 0, 165UL);
   initc(useri_fTPURGEOBJ, "Keep Objects in Memory Min", 27ul, useri_cLINE, "\
180", 4ul, 0, 170UL);
   initc(useri_fTRACKFILT, "Trackfilter", 12ul, useri_cBOOL, "", 1ul, 1,
                245UL);
   initc(useri_fWATCH, "Watch Calls", 12ul, useri_cLIST, "", 1ul, 0, 5UL);
   initc(useri_fAPPROXY, "Approxy Warn (km)", 18ul, useri_cBLINE, "", 1ul, 0,
                 5UL);
   initc(useri_fFIND, "Find Call or Locator", 21ul, useri_cLINE, "", 1ul, 0,
                10UL);
   initc(useri_fLOGWFN, "WriteLogfilename", 17ul, useri_cBLINE, "logs/rawlog%\
d", 14ul, 1, 15UL);
   initc(useri_fLOGDAYS, "Delete Log Days", 16ul, useri_cLINE, "31", 3ul, 0,
                17UL);
   initc(useri_fLOGFN, "ImportLogfilename", 18ul, useri_cLINE, "", 1ul, 0,
                20UL);
   aprsstr_DateToStr(aprsdecode_realtime, s, 100ul);
   s[16U] = 0;
   initc(useri_fLOGDATE, "Start yyyy.mm.dd hh:mm", 23ul, useri_cLINE, s,
                100ul, 0, 25UL);
   configs[useri_fLOGDATE].width = 17U;
   initc(useri_fLOGFIND, "Find Call In Log", 17ul, useri_cLINE, "", 1ul, 0,
                30UL);
   configs[useri_fLOGFIND].width = 10U;
   initc(useri_fFOTOFN, "Screenshotfile(.ppm/.png)", 26ul, useri_cLINE, "",
                1ul, 0, 35UL);
   initc(useri_fXYSIZE, "Windowsize X Y", 15ul, useri_cLINE, "600 400", 8ul,
                0, 40UL);
   initc(useri_fMYCALL, "MyCall", 7ul, useri_cLINE, "", 1ul, 0, 60UL);
   initc(useri_fMYPOS, "MyPos", 6ul, useri_cLINE, "0000.00N/00000.00E", 19ul,
                 0, 65UL);
   initc(useri_fRBTEXT, "RfBeacons", 10ul, useri_cBLIST, "", 1ul, 0, 70UL);
   initc(useri_fRBTYP, "Beacontype", 11ul, useri_cLINE, "B", 2ul, 0, 71UL);
   configs[useri_fRBTYP].width = 1U;
   initc(useri_fRBNAME, "Objectname", 11ul, useri_cLINE, "", 1ul, 0, 72UL);
   configs[useri_fRBNAME].width = 9U;
   initc(useri_fRBSYMB, "Symbol", 7ul, useri_cLINE, "/-", 3ul, 0, 73UL);
   configs[useri_fRBSYMB].width = 3U;
   initc(useri_fRBPOS, "Position", 9ul, useri_cLINE, "", 1ul, 0, 74UL);
   initc(useri_fRBPOSTYP, "RfBeaconPosType", 16ul, useri_cLINE, "g", 2ul, 0,
                75UL);
   configs[useri_fRBPOSTYP].width = 1U;
   initc(useri_fRBALT, "Altitude(m NN)", 15ul, useri_cLINE, "", 1ul, 0,
                76UL);
   initc(useri_fRBSPEED, "Speed (km/h)", 13ul, useri_cLINE, "", 1ul, 0,
                77UL);
   initc(useri_fRBDIR, "Direction (deg)", 16ul, useri_cLINE, "", 1ul, 0,
                78UL);
   initc(useri_fRBCOMMENT, "Comment", 8ul, useri_cLINE, "", 1ul, 0, 79UL);
   initc(useri_fRBTIME, "Intervall (s)", 14ul, useri_cLINE, "3600", 5ul, 0,
                80UL);
   initc(useri_fRBPORT, "RfBeaconPort", 13ul, useri_cLINE, "1", 2ul, 0,
                81UL);
   configs[useri_fRBPORT].width = 1U;
   initc(useri_fRBPATH, "Via Path", 9ul, useri_cLINE, "-1", 3ul, 0, 82UL);
   initc(useri_fRBDEST, "Destination", 12ul, useri_cLINE, "APLM01", 7ul, 0,
                82UL);
   configs[useri_fRBDEST].width = 9U;
   initc(useri_fNBTEXT, "Net Beacontext", 15ul, useri_cLINE, "Test aprsmap",
                13ul, 0, 83UL);
   initc(useri_fMYSYM, "My Symbol", 10ul, useri_cLINE, "/-", 3ul, 0, 84UL);
   configs[useri_fMYSYM].width = 2U;
   initc(useri_fNETBTIME, "Netbeacontime (s)", 18ul, useri_cLINE, "600", 4ul,
                 0, 85UL);
   initc(useri_fRFBTSHIFT, "Rfbeaconshift (s)", 18ul, useri_cLINE, "120",
                4ul, 1, 90UL);
   initc(useri_fSERVERURL, "AprsIs-URL", 11ul, useri_cBLIST, "", 1ul, 0,
                95UL);
   initc(useri_fSERVERFILT, "AprsIs-Filter", 14ul, useri_cLINE, "m/200", 6ul,
                 0, 100UL);
   initc(useri_fPW, "PassCode", 9ul, useri_cLINE, "", 1ul, 0, 105UL);
   initc(useri_fCONNECT, "Connect Server", 15ul, useri_cBOOL, "", 1ul, 0,
                110UL);
   initc(useri_fALLOWGATE, "Allow Gateing RFtoNET", 22ul, useri_cBLINE, "1234\
", 5ul, 0, 115UL);
   initc(useri_fALLOWNETTX, "Allow Tx to NET", 16ul, useri_cBOOL, "", 1ul, 1,
                 120UL);
   initc(useri_fUDP1, "UDP1(ip:send:listen)", 21ul, useri_cBLINE, "127.0.0.1:\
9001:9002", 20ul, 0, 125UL);
   initc(useri_fUDP2, "UDP2(ip:send:listen)", 21ul, useri_cBLINE, "", 1ul, 0,
                 130UL);
   initc(useri_fUDP3, "UDP3(ip:send:listen)", 21ul, useri_cBLINE, "", 1ul, 0,
                 135UL);
   initc(useri_fUDP4, "UDP4(ip:send:listen)", 21ul, useri_cBLINE, "", 1ul, 0,
                 140UL);
   initc(useri_fSERIALTASK, "Serial Task", 12ul, useri_cBLINE, "udpflex -U :9\
002:9001 -i kiss.txt -u -t /dev/ttyUSB0:9600", 58ul, 0, 137UL);
   initc(useri_fSERIALTASK2, "Serial Task2", 13ul, useri_cBLINE, "afskmodem -\
f 22050 -C 0 -p /dev/ttyS0 0 -M 0 -U 127.0.0.1:9002:9001 -m 0", 73ul, 0,
                137UL);
   initc(useri_fDIGI, "Digipeater", 11ul, useri_cBLIST, "", 1ul, 0, 139UL);
   initc(useri_fDIGITIME, "block same Content [s]", 23ul, useri_cLINE, "890",
                 4ul, 0, 140UL);
   initc(useri_fDIGIRADIUS, "from Radius [km]", 17ul, useri_cLINE, "30000",
                6ul, 0, 140UL);
   initc(useri_fDIGIVIA, "Via Alias", 10ul, useri_cLINE, "RELAY ECHO", 11ul,
                0, 140UL);
   initc(useri_fDIGINN, "N-n Words", 10ul, useri_cLINE, "WIDE TRACE", 11ul,
                0, 140UL);
   initc(useri_fDIGIX, "block if in Path", 17ul, useri_cLINE, "TCPIP NOCALL",
                 13ul, 0, 140UL);
   initc(useri_fVIDEOCMD, "Videocompress Cmd", 18ul, useri_cLINE, "", 1ul, 0,
                 145UL);
   initc(useri_fOSMDIR, "Map Directory Path", 19ul, useri_cLINE, "osm", 4ul,
                0, 150UL);
   initc(useri_fDELAYGETMAP, "Delay Start Loadmap s", 22ul, useri_cLINE, "0",
                 2ul, 0, 172UL);
   initc(useri_fCENTER, "Center Watch Object", 20ul, useri_cLINE, "40", 3ul,
                0, 220UL);
   initc(useri_fALLOWEXP, "Allow Expand Tile", 18ul, useri_cBOOL, "", 1ul, 1,
                 230UL);
   initc(useri_fZOOMMISS, "Autozoom up to having Tiles", 28ul, useri_cBOOL, "\
", 1ul, 0, 235UL);
   initc(useri_fONESYMB, "Show One Symbol", 16ul, useri_cLINE, "", 1ul, 0,
                271UL);
   initc(useri_fDEFZOOM, "Default Zoom", 13ul, useri_cLINE, "14", 3ul, 0,
                290UL);
   initc(useri_fANIMSPEED, "Animation Speed", 16ul, useri_cLINE, "200", 4ul,
                0, 295UL);
   initc(useri_fVIDEOFAST, "Video Accel Delay", 18ul, useri_cLINE, "0", 2ul,
                0, 300UL);
   initc(useri_fFONTSIZE, "Fontsize", 9ul, useri_cLINE, "10", 3ul, 0, 302UL);
   initc(useri_fVIEW, "View", 5ul, useri_cLIST, "", 1ul, 0, 305UL);
   initc(useri_fMAXZOOM, "Max Zoom", 9ul, useri_cLINE, "18", 3ul, 0, 310UL);
   initc(useri_fDOWNLOADZOOM, "Download to Zoom", 17ul, useri_cLINE, "", 1ul,
                 0, 315UL);
   initc(useri_fGETMAPS, "Get Map Program", 16ul, useri_cBLINE, "gm.sh", 6ul,
                 1, 320UL);
   initc(useri_fMAPNAMES, "Map Names", 10ul, useri_cBLIST, "", 1ul, 0,
                322UL);
   initc(useri_fMSGRFDEST, "MsgRfdestcall", 14ul, useri_cLINE, "NOGATE", 7ul,
                 0, 360UL);
   configs[useri_fMSGRFDEST].width = 9U;
   initc(useri_fMSGNETDEST, "MsgNetdestcall", 15ul, useri_cLINE, "APLM01",
                7ul, 0, 365UL);
   configs[useri_fMSGNETDEST].width = 9U;
   initc(useri_fMSGPATH, "Msg Path", 9ul, useri_cLINE, "2", 2ul, 0, 370UL);
   initc(useri_fPOPUPMSG, "Popupmsg", 9ul, useri_cBOOL, "", 1ul, 1, 372UL);
   initc(useri_fMSGALLSSID, "Show All Msg SSID", 18ul, useri_cBOOL, "", 1ul,
                1, 374UL);
   initc(useri_fPASSSELFMSG, "Show Msg To Myself", 19ul, useri_cBOOL, "",
                1ul, 0, 376UL);
   initc(useri_fQUERYS, "Querys", 7ul, useri_cBLIST, "", 1ul, 0, 380UL);
   initc(useri_fKMH, "Km/h Text", 10ul, useri_cBLINE, "km/h", 5ul, 1, 385UL);
   initc(useri_fWRINCOM, "Monitor InOut", 14ul, useri_cLINE, "", 1ul, 0,
                388UL);
   initc(useri_fWRTICKER, "Show Headline", 14ul, useri_cBLINE, "1", 2ul, 0,
                390UL);
   initc(useri_fLOCALTIME, "Local Time h", 13ul, useri_cLINE, "0", 2ul, 0,
                395UL);
   initc(useri_fOBJSRCFILT, "Obj Source Filter", 18ul, useri_cBLIST, "", 1ul,
                 0, 400UL);
   initc(useri_fIGATEFILT, "Igate Filter", 13ul, useri_cBLIST, "", 1ul, 0,
                405UL);
   initc(useri_fFINGERPRINT, "Bad Paths Ports", 16ul, useri_cLINE, "", 1ul,
                0, 407UL);
   initc(useri_fOPFILT, "Call Filter", 12ul, useri_cBLIST, "", 1ul, 0,
                410UL);
   initc(useri_fSTRFILT, "Comment Filter", 15ul, useri_cBLIST, "", 1ul, 0,
                415UL);
   initc(useri_fDESTFILT, "Destination Filter", 19ul, useri_cBLIST, "", 1ul,
                0, 417UL);
   initc(useri_fRAWFILT, "Raw Frame Filter", 17ul, useri_cBLIST, "", 1ul, 0,
                418UL);
   initc(useri_fMUSTBECALL, "Callcheck", 10ul, useri_cBOOL, "", 1ul, 1,
                420UL);
   initc(useri_fGEOBRIGHTNESS, "Brightness", 11ul, useri_cLINE, "30", 3ul, 0,
                 422UL);
   initc(useri_fGEOCONTRAST, "Contrast", 9ul, useri_cLINE, "5", 2ul, 0,
                423UL);
   initc(useri_fREFRACT, "Refraction Factor", 18ul, useri_cLINE, "0.25", 5ul,
                 0, 424UL);
   initc(useri_fFRESNELL, "MHz", 4ul, useri_cLINE, "2425.0", 7ul, 0, 425UL);
   initc(useri_fANT1, "Antenna 1 m", 12ul, useri_cLINE, "10", 3ul, 0, 426UL);
   initc(useri_fANT2, "Antenna 2 m", 12ul, useri_cLINE, "10", 3ul, 0, 427UL);
   initc(useri_fANT3, "Antenna 3 m", 12ul, useri_cLINE, "10", 3ul, 0, 428UL);
   initc(useri_fPANOSIZE, "Panorama x y", 13ul, useri_cLINE, "", 1ul, 0,
                460UL);
   initc(useri_fPANOPOS, "Panopos x y", 12ul, useri_cLINE, "", 1ul, 0,
                460UL);
   initc(useri_fSRTMCACHE, "Srtmcache MByte", 16ul, useri_cBLINE, "100", 4ul,
                 0, 431UL);
   initc(useri_fBEEPPROX, "Bell on Approxy", 16ul, useri_cBLINE, "400 40 2000\
 1000", 17ul, 0, 428UL);
   initc(useri_fBEEPWATCH, "Bell on Watchcall", 18ul, useri_cBLINE, "350 50",
                 7ul, 0, 429UL);
   initc(useri_fBEEPMSG, "Bell on Message/Ack", 20ul, useri_cBLINE, "800 500 \
1000 100", 17ul, 0, 430UL);
   initc(useri_fMSGTO, "To Call", 8ul, useri_cLINE, "", 1ul, 0, 435UL);
   configs[useri_fMSGTO].width = 9U;
   initc(useri_fMSGTEXT, "MsgText", 8ul, useri_cLINE, "", 1ul, 0, 431UL);
   configs[useri_fMSGTEXT].width = 67U;
   initc(useri_fMSGPORT, "To Port", 8ul, useri_cLINE, "A", 2ul, 0, 435UL);
   configs[useri_fMSGPORT].width = 1U;
   initc(useri_fMENUXYEDIT, "Line editor x y", 16ul, useri_cLINE, "", 1ul, 0,
                 440UL);
   initc(useri_fMENUXYMSG, "Message editor x y", 19ul, useri_cLINE, "", 1ul,
                0, 445UL);
   initc(useri_fMENUXYBEACON, "Beacon editor x y", 18ul, useri_cLINE, "",
                1ul, 0, 450UL);
   initc(useri_fMENUXYDIGI, "Digi editor x y", 16ul, useri_cLINE, "", 1ul, 0,
                 451UL);
   initc(useri_fMENUXYSTATUS, "Statusbar x y", 14ul, useri_cLINE, "", 1ul, 0,
                 455UL);
   initc(useri_fMENUXYLIST, "Listwindow x y", 15ul, useri_cLINE, "", 1ul, 0,
                460UL);
   initc(useri_fMENUXYLISTMIN, "Listwindowmin x y", 18ul, useri_cLINE, "32000\
 32000", 12ul, 0, 465UL);
   initc(useri_fMENULISTICON, "Listwindowicon", 15ul, useri_cLINE, "", 1ul,
                0, 470UL);
   initc(useri_fMENUSIZELIST, "Listwindow size", 16ul, useri_cLINE,
                "300 200", 8ul, 0, 475UL);
   initc(useri_fMENUMONICON, "Monwindowicon", 14ul, useri_cBLINE, "I", 2ul,
                1, 480UL);
   initc(useri_fMENUXYMON, "Monwindow x y", 14ul, useri_cLINE, "", 1ul, 0,
                485UL);
   initc(useri_fMENUSIZEMON, "Monwindow size", 15ul, useri_cLINE, "300 100",
                8ul, 0, 490UL);
   initc(useri_fAUTOSAVE, "Save Config On Quit", 20ul, useri_cBOOL, "", 1ul,
                1, 495UL);
   initc(useri_fEDITLINE, "", 1ul, useri_cLINE, " ", 2ul, 0, 499UL);
   initconfig1();
} /* end initconfig() */


static char isbool(unsigned char typ)
{
   return (typ==useri_cBOOL || typ==useri_cBLINE) || typ==useri_cBLIST;
} /* end isbool() */


extern char useri_configon(unsigned char v)
{
   if (configs[v].typ==useri_cBOOL) return configs[v].on>0U;
   else if (configs[v].typ==useri_cBLINE && configs[v].lines) {
      return configs[v].lines->active>0U;
   }
   else return 0;
   return 0;
} /* end configon() */


static char configson(unsigned char v, unsigned long line)
{
   pCONFLINE pl;
   pl = configs[v].lines;
   for (;;) {
      if (pl==0) break;
      if (line==0UL) return pl->active>0U;
      --line;
      pl = pl->next;
   }
   return 0;
} /* end configson() */

#define useri_ESC0 "~"


extern void useri_saveconfig(void)
{
   long fd;
   char h[1000];
   unsigned char i;
   pCONFLINE pl;
   unsigned long j;
   unsigned long n;
   char c;
   unsigned long tmp;
   aprstext_postostr(aprsdecode_click.markpos, '3', h, 1000ul);
   useri_AddConfLine(useri_fMARKPOS, 1U, h, 1000ul);
   aprsstr_cleanfilename(aprsdecode_lums.configfn, 257ul);
   fd = osi_OpenWrite(aprsdecode_lums.configfn, 257ul);
   if (!osi_FdValid(fd)) {
      strncpy(h,"Can not write ",1000u);
      aprsstr_Append(h, 1000ul, aprsdecode_lums.configfn, 257ul);
      useri_textautosize(0L, 0L, 6UL, 4UL, 'e', h, 1000ul);
      useri_refresh = 1;
      return;
   }
   for (i = useri_fFIND;; i++) {
      if (configs[i].title[0U] && i!=useri_fEDITLINE) {
         n = 0UL;
         pl = configs[i].lines;
         while (pl) {
            pl = pl->next;
            ++n;
         }
         for (;;) {
            pl = configs[i].lines;
            tmp = n;
            j = 2UL;
            if (j<=tmp) for (;; j++) {
               pl = pl->next;
               if (j==tmp) break;
            } /* end for */
            aprsstr_Assign(h, 1000ul, configs[i].title, 31ul);
            c = '0';
            if (configs[i].typ==useri_cLIST || configs[i].typ==useri_cBLIST) {
               if (pl) c = (pl->active&7U)+48U;
            }
            else c = (configs[i].on&7U)+48U;
            if (c=='0') aprsstr_Append(h, 1000ul, "||", 3ul);
            else {
               aprsstr_Append(h, 1000ul, "|", 2ul);
               aprsstr_Append(h, 1000ul, (char *) &c, 1u/1u);
               aprsstr_Append(h, 1000ul, "|", 2ul);
            }
            if (pl) {
               j = 0UL;
               while (j<200UL && pl->line[j]) {
                  c = pl->line[j];
                  if ((unsigned char)c<' ') {
                     aprsstr_Append(h, 1000ul, "~", 2ul);
                     c = (char)((unsigned long)(unsigned char)c+64UL);
                  }
                  else if (c=='|') {
                     aprsstr_Append(h, 1000ul, "~", 2ul);
                     c = '/';
                  }
                  else if (c=='~') aprsstr_Append(h, 1000ul, "~", 2ul);
                  aprsstr_Append(h, 1000ul, (char *) &c, 1u/1u);
                  ++j;
               }
            }
            aprsstr_Append(h, 1000ul, "\012", 2ul);
            osi_WrBin(fd, (char *)h, 1000u/1u, aprsstr_Length(h, 1000ul));
            if (n<=1UL) break;
            --n;
         }
      }
      if (i==useri_fEDITLINE) break;
   } /* end for */
   osi_Close(fd);
   useri_textautosize(0L, 0L, 6UL, 4UL, 'b', "Config Saved", 13ul);
   useri_rdlums();
} /* end saveconfig() */


static void conflineno(unsigned char v, unsigned long lineno, char all,
                pCONFLINE * pl)
{
   *pl = configs[v].lines;
   for (;;) {
      if (*pl==0) break;
      if (all || (*pl)->active>0U) {
         if (lineno==0UL) break;
         --lineno;
      }
      *pl = (*pl)->next;
   }
} /* end conflineno() */


extern void useri_confstrings(unsigned char v, unsigned long lineno,
                char all, char s[], unsigned long s_len)
{
   pCONFLINE pl;
   conflineno(v, lineno, all, &pl);
   if (pl) aprsstr_Assign(s, s_len, pl->line, 201ul);
   else s[0UL] = 0;
} /* end confstrings() */


extern void useri_confstr(unsigned char v, char s[], unsigned long s_len)
{
   useri_confstrings(v, 0UL, 1, s, s_len);
} /* end confstr() */


extern unsigned char useri_confflags(unsigned char v, unsigned long lineno)
{
   pCONFLINE pl;
   if (configs[v].typ==useri_cBLIST || configs[v].typ==useri_cLIST) {
      conflineno(v, lineno, 1, &pl);
      if (pl) return pl->active;
      else return 0U;
   }
   else return configs[v].on;
   return 0;
} /* end confflags() */

#define useri_ESC "~"


extern void useri_loadconfig(char verb)
{
   long fd;
   char s[1000];
   char h[1000];
   unsigned char i;
   char c;
   unsigned long j;
   unsigned long titmod;
   unsigned char on;
   initconfig();
   titmod = 0UL;
   aprsstr_cleanfilename(aprsdecode_lums.configfn, 257ul);
   fd = osi_OpenRead(aprsdecode_lums.configfn, 257ul);
   if (osi_FdValid(fd)) {
      h[0U] = 0;
      while (osi_RdBin(fd, (char *) &c, 1u/1u, 1UL)==1L) {
         if (c!='\015') {
            if (c=='\012') {
               if (titmod==2UL) {
                  j = 0UL;
                  memset((char *)s,(char)0,1000UL);
                  while (j<999UL && h[j]) {
                     c = h[j];
                     if (c=='~') {
                        ++j;
                        c = h[j];
                        if ((unsigned char)c>='@' && (unsigned char)c<='`') {
                           c = (char)((unsigned long)(unsigned char)c-64UL);
                        }
                        else if (c=='/') c = '|';
                     }
                     aprsstr_Append(s, 1000ul, (char *) &c, 1u/1u);
                     ++j;
                  }
                  if (i<useri_fEDITLINE) useri_AddConfLine(i, on, s, 1000ul);
               }
               titmod = 0UL;
               h[0U] = 0;
            }
            else if (titmod==0UL) {
               if (c=='|') {
                  if (h[0U]) {
                     i = useri_fFIND;
                     while (i<useri_fEDITLINE && !aprsstr_StrCmp(h, 1000ul,
                configs[i].title, 31ul)) ++i;
                  }
                  else {
                     i = useri_fEDITLINE; /* else compiler sees error */
                     ++i;
                  }
                  titmod = 1UL;
                  on = 0U;
               }
               else if ((unsigned char)c>=' ') {
                  aprsstr_Append(h, 1000ul, (char *) &c, 1u/1u);
               }
               else titmod = 3UL;
            }
            else if (titmod==1UL) {
               if (c=='Y' || c=='y') on = 1U;
               else if ((unsigned char)c>='0' && (unsigned char)c<='7') {
                  on = (unsigned char)((unsigned long)(unsigned char)c-48UL);
               }
               else if (c=='|') {
                  titmod = 2UL;
                  h[0U] = 0;
               }
            }
            else if (titmod==2UL) {
               if ((unsigned char)c<' ') titmod = 3UL;
               else aprsstr_Append(h, 1000ul, (char *) &c, 1u/1u);
            }
         }
      }
      osi_Close(fd);
   }
   else {
      strncpy(h,"Can not read ",1000u);
      aprsstr_Append(h, 1000ul, aprsdecode_lums.configfn, 257ul);
      useri_textautosize(0L, 0L, 6UL, 10UL, 'e', h, 1000ul);
      useri_refresh = 1;
      if (verb) osi_WrStrLn(h, 1000ul);
   }
   useri_rdlums();
   useri_confstr(useri_fMAPNAMES, s, 1000ul);
   if (s[0U]==0) {
      useri_AddConfLine(useri_fMAPNAMES, 1U, "tiles -128", 11ul);
      useri_AddConfLine(useri_fMAPNAMES, 1U, "tiles_topo -128", 16ul);
      useri_AddConfLine(useri_fMAPNAMES, 1U, "tiles_quest -128", 17ul);
      useri_AddConfLine(useri_fMAPNAMES, 1U, "tiles_cyclemap 0", 17ul);
      useri_AddConfLine(useri_fMAPNAMES, 1U, "tiles_sat 0", 12ul);
   }
   /*  confstr(fDIGI, s); */
   /*  IF s[0]=0C THEN AddConfLine(fDIGI, 0,
                "*10 890 30000 nWIDE nTRACE vWIDE vRELAY vECHO xTCPIP") END;
                */
   useri_confstr(useri_fQUERYS, s, 1000ul);
   if (s[0U]==0) {
      useri_AddConfLine(useri_fQUERYS, 1U, "?APRST:PATH=\\\\p", 16ul);
      useri_AddConfLine(useri_fQUERYS, 1U, "?PING:PATH=\\\\p", 15ul);
      useri_AddConfLine(useri_fQUERYS, 1U, "?APRSP:\\\\l", 11ul);
      useri_AddConfLine(useri_fQUERYS, 1U, "?VER:VERSION \\\\v", 17ul);
      useri_AddConfLine(useri_fQUERYS, 1U, "?HELP:?APRST,?APRSP,?PING,?VER",
                31ul);
   }
   useri_confstr(useri_fMARKPOS, s, 1000ul);
   aprstext_deghtopos(s, 1000ul, &aprsdecode_click.markpos);
   maptool_closesrtmfile(); /* to reread possibly updated files */
   maptool_rdmountains("poi.txt", 8ul, 0);
                /* to reread possibly updated file */
   maptool_rdmountains("mypoi.txt", 10ul, 1);
} /* end loadconfig() */


extern void useri_confappend(unsigned char v, char s[], unsigned long s_len)
/* append a config line to string */
{
   char h[1000];
   useri_confstr(v, h, 1000ul);
   aprsstr_Append(s, s_len, h, 1000ul);
} /* end confappend() */


static void confreplace(unsigned char v, unsigned long lineno, char all,
                const char s[], unsigned long s_len)
{
   pCONFLINE pl;
   conflineno(v, lineno, all, &pl);
   if (pl && pl->line[0U]) aprsstr_Assign(pl->line, 201ul, s, s_len);
} /* end confreplace() */


extern void useri_configbool(unsigned char v, char on)
/* set config to TRUE OR FALSE */
{
   configs[v].on = on;
   if (configs[v].typ==useri_cBLINE) {
      if (configs[v].lines==0) useri_AddConfLine(v, on, "", 1ul);
      else configs[v].lines->active = on;
   }
} /* end configbool() */


static void configtogg(unsigned char v)
/* invert TRUE and FALSE of a config */
{
   useri_configbool(v, !useri_configon(v));
} /* end configtogg() */


extern char useri_isupdated(unsigned char v)
{
   char u;
   u = configs[v].updated;
   configs[v].updated = 0;
   return u;
} /* end isupdated() */


extern void useri_setview(long n, char s[], unsigned long s_len)
{
   pCONFLINE pl;
   X2C_PCOPY((void **)&s,s_len);
   pl = configs[useri_fVIEW].lines;
   while (pl && n>0L) {
      pl = pl->next;
      --n;
   }
   if (pl) {
      aprsstr_Assign(pl->line, 201ul, s, s_len);
      pl->active = 1U;
   }
   else {
      useri_AddConfLine(useri_fVIEW, 1U, s, s_len);
      pl = configs[useri_fVIEW].lines;
      if (pl->next) {
         configs[useri_fVIEW].lines = pl->next;
         while (pl->next->next) pl->next = pl->next->next;
         pl->next->next = pl;
         pl->next = 0;
      }
   }
   X2C_PFREE(s);
} /* end setview() */


static void alloccutbuf(unsigned long len)
{
   if (xosi_cutbuffer.text) {
      useri_debugmem.screens -= xosi_cutbuffer.cutlen;
      Storage_DEALLOCATE((X2C_ADDRESS *) &xosi_cutbuffer.text,
                xosi_cutbuffer.cutlen);
   }
   xosi_cutbuffer.text = 0;
   xosi_cutbuffer.cutlen = len;
   if (xosi_cutbuffer.cutlen>0UL) {
      Storage_ALLOCATE((X2C_ADDRESS *) &xosi_cutbuffer.text,
                xosi_cutbuffer.cutlen);
      useri_debugmem.req = xosi_cutbuffer.cutlen;
      useri_debugmem.screens += useri_debugmem.req;
      if (xosi_cutbuffer.text==0) {
         osi_WrStrLn("cutbuf: out of memory", 22ul);
         useri_wrheap();
         X2C_ABORT();
      }
   }
} /* end alloccutbuf() */


extern void useri_copypaste(char s[], unsigned long s_len)
{
   unsigned long i;
   unsigned long len;
   unsigned long tmp;
   len = aprsstr_Length(s, s_len);
   if (len>0UL) {
      alloccutbuf(len);
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         xosi_cutbuffer.text[i] = s[i];
         if (i==tmp) break;
      } /* end for */
      xosi_cutpaste();
   }
} /* end copypaste() */

/*
PROCEDURE ismultiline():BOOLEAN;
VAR c:CHAR;
BEGIN confstr(fRBPOSTYP, c); RETURN c=ENCODEAREA END ismultiline;
*/

extern void useri_postoconfig(struct aprspos_POSITION pos)
/* copy position to editline */
{
   char s[100];
   /*  postostr(pos, "3", s); */
   /*  AddConfLine(fRBPOS, 1, s); */
   aprstext_postostr(pos, '3', s, 100ul);
   useri_AddConfLine(useri_fEDITLINE, 1U, s, 100ul);
   if (s[0U]) useri_copypaste(s, 100ul);
   if (useri_beaconediting) {
      useri_confstr(useri_fRBPOSTYP, s, 100ul);
      if (X2C_STRCMP(s,100u,"A",2u)==0) aprsdecode_appendmultiline(pos);
      else {
         aprstext_postostr(pos, '3', s, 100ul);
         useri_AddConfLine(useri_fRBPOS, 1U, s, 100ul);
      }
   }
} /* end postoconfig() */


extern void useri_conf2str(unsigned char v, unsigned long valnum, char s[],
                unsigned long s_len)
{
   unsigned long i;
   useri_confstr(v, s, s_len);
   for (;;) {
      while (s[0UL]==' ') aprsstr_Delstr(s, s_len, 0UL, 1UL);
      i = 0UL;
      while (i<s_len-1 && (unsigned char)s[i]>' ') ++i;
      if (s[0UL]==0 || valnum==0UL) {
         s[i] = 0;
         break;
      }
      --valnum;
      if (i>0UL) aprsstr_Delstr(s, s_len, 0UL, i);
   }
} /* end conf2str() */


extern long useri_conf2int(unsigned char v, unsigned long valnum, long min0,
                long max0, long default0)
{
   float rval;
   long n;
   char s[51];
   useri_conf2str(v, valnum, s, 51ul);
   if (aprsstr_StrToFix(&rval, s, 51ul)) {
      n = (long)aprsdecode_trunc((float)fabs(rval));
      if (rval<0.0f) n = -n;
   }
   else n = default0;
   if (n<min0) return min0;
   if (n>max0) return max0;
   return n;
} /* end conf2int() */


extern float useri_conf2real(unsigned char v, unsigned long valnum,
                float min0, float max0, float default0)
{
   float r;
   char s[51];
   useri_conf2str(v, valnum, s, 51ul);
   if (!aprsstr_StrToFix(&r, s, 51ul)) r = default0;
   if (r<min0) return min0;
   if (r>max0) return max0;
   return r;
} /* end conf2real() */


static unsigned long cntconfigs(unsigned char v)
{
   unsigned long n;
   char s[201];
   n = 0UL;
   for (;;) {
      useri_confstrings(v, n, 1, s, 201ul);
      if (s[0U]==0) break;
      ++n;
   }
   return n;
} /* end cntconfigs() */


static void clreditline(void)
{
   if (configs[useri_fEDITLINE].lines) {
      configs[useri_fEDITLINE].lines->line[0U] = 0;
   }
   configs[useri_fEDITLINE].curspos = 0U;
} /* end clreditline() */


static void DelCharStr(char s[], unsigned long s_len, char c)
{
   unsigned long j;
   unsigned long i;
   i = 0UL;
   j = 0UL;
   while (i<=s_len-1 && s[i]) {
      if (s[i]!=c) {
         s[j] = s[i];
         ++j;
      }
      ++i;
   }
   if (j<=s_len-1) s[j] = 0;
} /* end DelCharStr() */


static void toggcfg(unsigned char v, char c, const char del[],
                unsigned long del_len)
/* toggle chars in config string */
{
   unsigned long i;
   char on;
   char s[301];
   useri_confstr(v, s, 301ul);
   on = aprsstr_InStr(s, 301ul, (char *) &c, 1u/1u)>=0L;
   if (del[0UL]==0) s[0U] = 0;
   else {
      DelCharStr(s, 301ul, c);
      i = 0UL;
      while (i<=del_len-1 && del[i]) {
         DelCharStr(s, 301ul, del[i]);
         ++i;
      }
   }
   if (!on) aprsstr_Append(s, 301ul, (char *) &c, 1u/1u);
   icfg(v, s, 301ul);
} /* end toggcfg() */


extern void useri_ColConfset(struct aprsdecode_COLTYP * c, unsigned char v)
{
   c->r = (unsigned long)X2C_DIV(useri_conf2int(v, 0UL, 0L, 100L, 100L)*256L,
                100L);
   c->g = (unsigned long)X2C_DIV(useri_conf2int(v, 1UL, 0L, 100L, 100L)*256L,
                100L);
   c->b = (unsigned long)X2C_DIV(useri_conf2int(v, 2UL, 0L, 100L, 100L)*256L,
                100L);
   if (c->r+c->g+c->b<100UL) {
      c->r = 256UL; /* no self knocking out */
      c->g = 256UL;
      c->b = 256UL;
   }
} /* end ColConfset() */


extern unsigned long useri_localtime(void)
{
   return (unsigned long)(useri_conf2int(useri_fLOCALTIME, 0UL, -24L, 24L,
                0L)*3600L);
} /* end localtime() */


extern void useri_getview(unsigned char v, unsigned long n, float * zoom,
                struct aprspos_POSITION * pos)
{
   float rval;
   struct aprspos_POSITION rpos;
   char h[101];
   char s[101];
   unsigned long j;
   unsigned long i;
   useri_confstrings(v, n, 1, s, 101ul);
   s[100U] = 0;
   while (s[0U]) {
      j = 0UL;
      i = 0UL;
      while ((unsigned char)s[i]>' ') {
         h[j] = s[i];
         ++j;
         ++i;
      }
      h[j] = 0;
      j = 0UL;
      while (s[i]==' ') ++i;
      while ((unsigned char)s[i]>' ') {
         s[j] = s[i];
         ++j;
         ++i;
      }
      s[j] = 0;
      maptool_loctopos(&rpos, h, 101ul);
      if (!aprspos_posvalid(rpos)) aprstext_degtopos(h, 101ul, &rpos);
      if (!aprspos_posvalid(rpos)) {
         if (aprsstr_StrToFix(&rval, h, 101ul)) {
            if (rval<1.0f) rval = 1.0f;
            else if (rval>18.0f) rval = 18.0f;
            *zoom = rval;
         }
      }
      else *pos = rpos;
   }
} /* end getview() */


extern void useri_Setmap(unsigned long n)
{
   char s[101];
   unsigned long i;
   float lu;
   i = 0UL;
   for (;;) {
      /* count lines for reverse sort access */
      useri_confstrings(useri_fMAPNAMES, i, 0, s, 101ul);
      if (s[0U]==0) break;
      ++i;
   }
   if (n<i) {
      useri_confstrings(useri_fMAPNAMES, (i-n)-1UL, 0, s, 101ul);
      i = 0UL;
      while ((i<100UL && i<40UL) && (unsigned char)s[i]>' ') {
         aprsdecode_lums.mapname[i] = s[i];
         ++i;
      }
      aprsdecode_lums.mapname[i] = 0;
      while (i<100UL && s[i]==' ') ++i;
      aprsstr_Delstr(s, 101ul, 0UL, i);
      if (aprsstr_StrToFix(&lu, s, 101ul) && (float)fabs(lu)<256.0f) {
         aprsdecode_lums.maplumcorr = (long)X2C_TRUNCI(lu,X2C_min_longint,
                X2C_max_longint);
      }
      else aprsdecode_lums.maplumcorr = -128L;
   }
} /* end Setmap() */


extern void useri_saveXYtocfg(unsigned char c, long x, long y)
{
   char h[101];
   char s[101];
   aprsstr_IntToStr(x, 0UL, s, 101ul);
   aprsstr_IntToStr(y, 0UL, h, 101ul);
   aprsstr_Append(s, 101ul, " ", 2ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   useri_AddConfLine(c, 0U, s, 101ul);
} /* end saveXYtocfg() */


extern void useri_xerrmsg(char s[], unsigned long s_len)
{
   X2C_PCOPY((void **)&s,s_len);
   useri_textautosize(0L, 0L, 6UL, 0UL, 'e', s, s_len);
   X2C_PFREE(s);
} /* end xerrmsg() */


extern void useri_rdlums(void)
{
   unsigned long n;
   aprsdecode_lums.map = 10L*useri_conf2int(useri_fLMAP, 0UL, 0L, 100L, 30L);
   aprsdecode_lums.rfbri = 10L*useri_conf2int(useri_fLRF, 0UL, 0L, 100L,
                30L);
   aprsdecode_lums.track = 10L*useri_conf2int(useri_fLTRACK, 0UL, 0L, 100L,
                100L);
   aprsdecode_lums.waypoint = 10L*useri_conf2int(useri_fLWAY, 0UL, 0L, 100L,
                100L);
   aprsdecode_lums.sym = 10L*useri_conf2int(useri_fLSYM, 0UL, 0L, 100L,
                100L);
   aprsdecode_lums.obj = 10L*useri_conf2int(useri_fLOBJ, 0UL, 0L, 100L, 70L);
   aprsdecode_lums.text = 10L*useri_conf2int(useri_fLTEXT, 0UL, 0L, 100L,
                80L);
   aprsdecode_lums.nomov = 10L*useri_conf2int(useri_fNOMOV, 0UL, 0L, 100L,
                9L);
   aprsdecode_lums.centering = useri_conf2int(useri_fCENTER, 0UL, 0L, 100L,
                40L);
   aprsdecode_lums.purgetime = (unsigned long)
                (60L*useri_conf2int(useri_fTPURGE, 0UL, 0L, 35791394L,
                1500L));
   aprsdecode_lums.purgetimeobj = (unsigned long)
                (60L*useri_conf2int(useri_fTPURGEOBJ, 0UL, 0L, 35791394L,
                1500L));
   aprsdecode_lums.firstdim = (unsigned long)
                (60L*useri_conf2int(useri_fTFULL, 0UL, 0L, 35791394L,
                1410L));
   aprsdecode_lums.maxdim = (unsigned long)(60L*useri_conf2int(useri_fTFADE,
                0UL, 0L, 35791394L, 30L));
   aprsdecode_lums.kmhtime = (unsigned long)useri_conf2int(useri_fKMHTIME,
                0UL, 0L, X2C_max_longint, 600L);
   aprsdecode_lums.movestep = (unsigned long)useri_conf2int(useri_fMOVESTEP,
                0UL, 1L, 100L, 10L);
   aprsdecode_lums.dupemaxtime = (unsigned long)
                (60L*useri_conf2int(useri_fDUPDEL, 0UL, 0L, 1440L, 180L));
   aprsdecode_lums.gamma = useri_conf2real(useri_fGAMMA, 0UL, 0.1f, 10.0f,
                2.2f);
   xosi_Gammatab(aprsdecode_lums.gamma);
   aprsdecode_lums.moded = 0;
   aprsdecode_lums.menutransp = useri_configon(useri_fTRANSP);
   useri_ColConfset(&aprsdecode_lums.menubackcol, useri_fCOLMENUBACK);
   n = (unsigned long)useri_conf2int(useri_fTRANSP, 0UL, 0L, 100L, 60L);
   if (n==0UL) n = 100UL;
   if (aprsdecode_lums.menutransp) n = n*2UL;
   aprsdecode_lums.menubackcol.r = (aprsdecode_lums.menubackcol.r*n)/100UL;
   aprsdecode_lums.menubackcol.g = ((aprsdecode_lums.menubackcol.g*n)/200UL)
                *2UL;
   aprsdecode_lums.menubackcol.b = (aprsdecode_lums.menubackcol.b*n)/100UL;
   if ((aprsdecode_lums.menubackcol.g&1)) --aprsdecode_lums.menubackcol.g;
   useri_ColConfset(&aprsdecode_lums.menucol, useri_fCOLMENUTEXT);
   if ((aprsdecode_lums.menucol.g&1)) --aprsdecode_lums.menucol.g;
} /* end rdlums() */


extern void useri_rdonesymb(char on)
{
   char s[31];
   if (on) {
      useri_confstr(useri_fONESYMB, s, 31ul);
      if (!aprsdecode_checksymb(s[0U], s[1U])) {
         aprsdecode_click.onesymbol.tab = s[0U];
         aprsdecode_click.onesymbol.pic = s[1U];
      }
   }
   else aprsdecode_click.onesymbol.tab = 0;
   useri_sayonoff("Show One Symbol", 16ul,
                aprsdecode_click.onesymbol.tab!=0);
} /* end rdonesymb() */


static void onoffm(maptool_pIMAGE img, long x0, long y00, char on)
{
   long y;
   long x;
   struct maptool_PIX col;
   if (on) {
      col.r = 100U;
      col.g = 999U;
      col.b = 100U;
   }
   else {
      col.r = 150U;
      col.g = 151U;
      col.b = 150U;
   }
   if (x0<0L) {
      x0 = -x0;
      col = img->Adr[(x0-5L)*img->Len0+(y00-5L)];
   }
   for (y = -5L; y<=5L; y++) {
      for (x = -5L; x<=5L; x++) {
         if (x*x+y*y<25L) img->Adr[(x0+x)*img->Len0+(y00+y)] = col;
      } /* end for */
   } /* end for */
} /* end onoffm() */


static long midy(pMENU m)
/* middle y position of actual line */
{
   return (long)(m->oldknob*m->yknob-m->yknob/2UL);
} /* end midy() */


static void onoff(pMENU m, long x0, char on)
{
   onoffm(m->image, x0, midy(m), on);
} /* end onoff() */


static void subicon(maptool_pIMAGE img, long x0, long y00, long dir,
                long size)
{
   long h;
   long yy;
   long xx;
   long x;
   long y;
   struct maptool_PIX col;
   long tmp;
   long tmp0;
   col.r = 1000U;
   col.g = 1000U;
   col.b = 1000U;
   tmp = size;
   y = -size;
   if (y<=tmp) for (;; y++) {
      tmp0 = size-labs(y);
      x = 0L;
      if (x<=tmp0) for (;; x++) {
         xx = x;
         yy = y;
         if ((dir&1)) xx = -xx;
         if (dir>=3L) {
            h = xx;
            xx = yy;
            yy = h;
         }
         img->Adr[(x0+xx)*img->Len0+(y00+yy)] = col;
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end subicon() */


static void allocmenu(pMENU * m, unsigned long xsize, unsigned long ysize,
                char saveimage)
{
   Storage_ALLOCATE((X2C_ADDRESS *)m, sizeof(struct MENU));
   useri_debugmem.req = sizeof(struct MENU);
   useri_debugmem.menus += sizeof(struct MENU);
   if (*m==0) {
      osi_WrStrLn("menu out of memory", 19ul);
      useri_wrheap();
      return;
   }
   /*INC(menucnt); WrInt(menucnt, 3);WrStrLn("=menus+"); */
   memset((X2C_ADDRESS)*m,(char)0,sizeof(struct MENU));
   (*m)->saveimage = saveimage;
   if (xsize==0UL || ysize==0UL) (*m)->image = 0;
   else if (saveimage && useri_panoimage) {
      (*m)->image = useri_panoimage;
      useri_panoimage = 0;
   }
   else {
      useri_allocimage(&(*m)->image, (long)xsize, (long)ysize,
                (*m)->saveimage);
      if ((*m)->image==0) {
         osi_WrStrLn("menuimage out of memory", 24ul);
         useri_wrheap();
         return;
      }
   }
/*      clr(m^.image); */
} /* end allocmenu() */


static long popxbase(unsigned long y1)
/* find right end of docked windows */
{
   pMENU m;
   unsigned long xm;
   unsigned long x;
   x = 0UL;
   if (aprsdecode_lums.headmenuy) {
      m = menus;
      while (m) {
         if (m->notoverdraw && m->nowy<y1) {
            /* should fit above this win */
            xm = m->x0;
            if (m->nowx>=xm) xm = m->nowx;
            xm += m->xsize;
            if (xm>x) x = xm;
         }
         m = m->next;
      }
   }
   return (long)x;
} /* end popxbase() */


extern void useri_starthint(unsigned long num, char center)
{
   hintnum = num;
   if (num>0UL) {
      hinttime = aprsdecode_realtime;
      /*    hintmouse:=xmouse; */
      if (center) {
         useri_xmouse.x = 0L;
         useri_xmouse.y = maptool_ysize;
         hinttime = 0UL;
      }
   }
} /* end starthint() */


static void killmenus(pMENU from)
{
   pMENU m;
   while (from) {
      m = from->next;
      if (menus==from) menus = m;
      if (from->image) {
         if (from->saveimage) {
            if (useri_panoimage) disposeimg(&useri_panoimage);
            useri_panoimage = from->image;
            from->image = 0;
         }
         else disposeimg(&from->image);
      }
      /*  IF from=actmenu THEN actmenu:=NIL END;  */
      useri_debugmem.menus -= sizeof(struct MENU);
      Storage_DEALLOCATE((X2C_ADDRESS *) &from, sizeof(struct MENU));
      /*DEC(menucnt); WrInt(menucnt, 3);WrStrLn("=menus-"); */
      from = m;
   }
   useri_starthint(0UL, 0);
} /* end killmenus() */


extern void useri_killallmenus(void)
{
   killmenus(menus);
   aprsdecode_click.watchlast = 0;
/*  IF newfont THEN loadfont; newfont:=FALSE END; */
} /* end killallmenus() */


static void killmenu(pMENU km)
{
   pMENU m;
   if (km) {
      if (km==menus) {
         km = km->next;
         menus->next = 0;
         killmenus(menus);
         menus = km;
      }
      else {
         m = menus;
         while (m->next) {
            if (m->next==km) {
               m->next = km->next;
               km->next = 0;
               killmenus(km);
               return;
            }
            m = m->next;
         }
      }
   }
} /* end killmenu() */


static pMENU findmenuid(unsigned long id)
{
   pMENU m;
   m = menus;
   while (m) {
      if (m->wid==id) return m;
      m = m->next;
   }
   return 0;
} /* end findmenuid() */


extern void useri_killmenuid(unsigned long id)
{
   pMENU m;
   m = findmenuid(id);
   if (m) killmenu(m);
} /* end killmenuid() */


static unsigned long mainxs(void)
{
   if (redrawimg==0) return 0UL;
   else return (redrawimg->Len1-1)+1UL;
   return 0;
} /* end mainxs() */


extern unsigned long useri_mainys(void)
{
   if (redrawimg==0) return 0UL;
   else return (redrawimg->Len0-1)+1UL;
   return 0;
} /* end mainys() */


static pMENU lastmenu(void)
{
   pMENU m;
   m = menus;
   if (m) while (m->next) m = m->next;
   return m;
} /* end lastmenu() */


static void joinmenux(pMENU m, pMENU menu)
{
   unsigned long xm;
   if (m && menu->wid==0UL) {
      xm = menu->image->Len1-1; /* find x position */
      menu->x0 = (m->nowx+m->xsize)-2UL;
      if (menu->x0+xm>mainxs() && menu->x0>xm) {
         if (m->nowx+2UL>=xm) menu->x0 = (m->nowx+2UL)-xm;
         else menu->x0 = 0UL;
      }
   }
} /* end joinmenux() */


static void appendmenu(pMENU menu)
{
   pMENU mh;
   pMENU m;
   unsigned long ys;
   unsigned long y;
   if (menu->wid>0UL) useri_killmenuid(menu->wid);
   menu->next = 0;
   menu->oldknob = 0UL;
   m = lastmenu();
   if (m) m->next = menu;
   else menus = menu;
   joinmenux(m, menu);
   m = menus;
   while (m) {
      if (m->wid>0UL && m->wid<200UL) m->y00 = 0UL;
      m = m->next;
   }
   y = aprsdecode_lums.fontysize+1UL;
   for (;;) {
      ys = 0UL;
      mh = 0;
      m = menus;
      while (m) {
         if (((m->wid>0UL && m->wid<200UL) && m->y00==0UL) && m->ysize>=ys) {
            ys = m->ysize;
            mh = m;
         }
         m = m->next;
      }
      if (mh==0) break;
      mh->y00 = y;
      y += ys;
   }
} /* end appendmenu() */


static unsigned long knobtexty(unsigned long yk)
{
   if (yk<=aprsdecode_lums.fontysize) return 0UL;
   else return ((yk-aprsdecode_lums.fontysize)-1UL)/2UL;
   return 0;
} /* end knobtexty() */

#define useri_MB 0

#define useri_MW 200


static void addline(pMENU m, const char s[], unsigned long s_len, char c[],
                unsigned long c_len, unsigned long hint)
{
   unsigned long dimm;
   unsigned long mark;
   unsigned long sk;
   unsigned long si;
   unsigned long i;
   unsigned long y;
   unsigned long x;
   struct aprsdecode_COLTYP colc;
   struct aprsdecode_COLTYP col;
   char h[1001];
   short mbb;
   short mbg;
   short mbr;
   short mmb;
   short mmg;
   short mmr;
   char highlitewholline;
   struct maptool_PIX * anonym;
   maptool_pIMAGE anonym0;
   unsigned long tmp;
   unsigned long tmp0;
   X2C_PCOPY((void **)&c,c_len);
   i = 0UL;
   x = 0UL;
   si = 0UL;
   dimm = 1000UL;
   highlitewholline = 0;
   mark = X2C_max_longcard;
   m->nohilite &= ~(1UL<<m->oldknob);
   mmr = 200;
   mmg = 10;
   mmb = 10;
   while ((i<=s_len-1 && s[i]) && si<=20UL) {
      if (s[i]=='|') {
         m->subk[m->oldknob][si] = (unsigned short)x;
         ++si;
      }
      else if ((unsigned char)s[i]>=(unsigned char)'\360' && (unsigned char)
                s[i]<=(unsigned char)'\370') {
         /*WrInt(x,10);WrInt(si,10);WrStr(s);WrStrLn(" x-si"); */
         x += (unsigned long)(unsigned char)s[i]-239UL;
      }
      else if ((unsigned char)s[i]>=(unsigned char)'\346' && (unsigned char)
                s[i]<=(unsigned char)'\357') {
         if (s[i]=='\346') m->noprop |= (1UL<<m->oldknob);
         else if (s[i]=='\351') m->nohilite |= (1UL<<m->oldknob);
         else if (s[i]=='\350') mark = si;
         else if (s[i]=='\352') mark = 0x0FFFFFFFEUL;
         else if (s[i]=='\347') {
            mmr = 350; /* color background yellow */
            mmg = 250;
            mmb = 50;
         }
         else if (s[i]=='\355') {
            mmr = 10; /* color background blue */
            mmg = 10;
            mmb = 600;
         }
         else if (s[i]=='\356') dimm = 400UL;
         else if (s[i]=='\357') highlitewholline = 1;
      }
      else x += 6UL;
      ++i;
   }
   if (si<=20UL) m->subk[m->oldknob][si] = 0U;
   mbr = 2;
   mbg = 2;
   mbb = 2;
   if (aprsdecode_lums.menutransp) {
      /* user enabled transparency */
      if (m->background==useri_bCOLOR) {
         mbr = (short)(aprsdecode_lums.menubackcol.r/4UL);
         mbg = (short)((aprsdecode_lums.menubackcol.g/8UL)*2UL);
         mbb = (short)(aprsdecode_lums.menubackcol.b/4UL);
      }
   }
   else if (m->background) {
      mbr = (short)aprsdecode_lums.menubackcol.r;
      mbg = (short)aprsdecode_lums.menubackcol.g;
      mbb = (short)aprsdecode_lums.menubackcol.b;
   }
   tmp = m->yknob-1UL;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      si = 0UL;
      sk = 0UL;
      tmp0 = m->xsize-1UL;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = (anonym0 = m->image,
                &anonym0->Adr[(x)*anonym0->Len0+(y+m->oldknob*m->yknob)]);
            if ((x==m->xsize-1UL || sk==2UL) || (x>0UL && x<m->xsize-2UL)
                && y==0UL) {
               anonym->r = 0U;
               anonym->g = 1U;
               anonym->b = 0U;
            }
            else if ((x==0UL || sk==1UL) || (x>0UL && x<m->xsize-1UL)
                && y==m->yknob-1UL) {
               anonym->r = 200U;
               anonym->g = 201U;
               anonym->b = 200U;
            }
            else if (mark==0x0FFFFFFFEUL || si==mark) {
               anonym->r = (unsigned short)mmr;
               anonym->g = (unsigned short)mmg;
               anonym->b = (unsigned short)mmb;
            }
            else {
               anonym->r = (unsigned short)mbr;
               anonym->g = (unsigned short)mbg;
               anonym->b = (unsigned short)mbb;
            }
            if (sk>0UL) --sk;
            i = (unsigned long)m->subk[m->oldknob][si];
            if (i>1UL && i-2UL<=x) {
               /*          IF m^.oldknob IN m^.noprop THEN r:=100; g:=100;
                b:=100 ELSE sk:=2 END; */
               if (highlitewholline) {
                  anonym->r = 100U;
                  anonym->g = 100U;
                  anonym->b = 100U;
               }
               else sk = 2UL;
               ++si;
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   x = 0UL;
   si = 0UL;
   useri_ColConfset(&colc, useri_fCOLMENUTEXT);
   colc.g = (colc.g/2UL)*2UL;
   col = colc;
   i = 0UL;
   for (;;) {
      if ((s[i]!='|' && s[i]) && i<=s_len-1) {
         h[x] = s[i];
         if (s[i]=='\353') maptool_Colset(&col, 'R');
         else if (s[i]=='\354') maptool_Colset(&col, 'Y');
         if (x<1000UL && ((unsigned char)s[i]<(unsigned char)
                '\346' || (unsigned char)s[i]>(unsigned char)'\357')) ++x;
      }
      else {
         h[x] = 0;
         if (x>0UL) {
            if (si==0UL) x = 0UL;
            else x = (unsigned long)m->subk[m->oldknob][si-1UL];
            maptool_drawstri(m->image, h, 1001ul, (long)(x+1UL),
                (long)(m->oldknob*m->yknob+knobtexty(m->yknob)), dimm, 1UL,
                col, !X2C_IN(m->oldknob,31,m->noprop), 0);
         }
         ++si;
         x = 0UL;
         if ((si>20UL || s[i]==0) || i>s_len-1) break;
         /*      INC(y); */
         col = colc;
      }
      ++i;
   }
   if (highlitewholline) {
      /*  IF m^.oldknob IN m^.noprop THEN */
      for (i = 0UL; i<=20UL; i++) {
         m->subk[m->oldknob][i] = 0U;
      } /* end for */
   }
   m->cmds[m->oldknob+1UL] = c[0UL];
   if (m->oldknob==m->hiknob) m->hiknob = 0UL;
   m->oldsub = 0UL;
   ++m->oldknob;
   if ((c_len-1>0UL && c[0UL]) && c[1UL]) {
      m->submen[m->oldknob] = 1;
      subicon(m->image, (long)(m->xsize-9UL), midy(m), 0L, 5L);
   }
   m->helpindex[m->oldknob] = (unsigned short)hint;
   X2C_PFREE(c);
} /* end addline() */


static void addonoff(pMENU m, const char s[], unsigned long s_len, char c[],
                unsigned long c_len, unsigned long hint, long x0, char on)
{
   X2C_PCOPY((void **)&c,c_len);
   addline(m, s, s_len, c, c_len, hint);
   onoff(m, x0, on);
   X2C_PFREE(c);
} /* end addonoff() */


static void potival(pMENU m, unsigned long knob, unsigned long val,
                unsigned long textsx, unsigned long textwx, char s[],
                unsigned long s_len, char w[], unsigned long w_len)
{
   unsigned long y;
   unsigned long x;
   struct aprsdecode_COLTYP col;
   struct maptool_PIX * anonym;
   maptool_pIMAGE anonym0;
   unsigned long tmp;
   unsigned long tmp0;
   X2C_PCOPY((void **)&s,s_len);
   X2C_PCOPY((void **)&w,w_len);
   if (knob>0UL) {
      tmp = m->yknob-4UL;
      y = 4UL;
      if (y<=tmp) for (;; y++) {
         tmp0 = m->xsize-9UL;
         x = 8UL;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct maptool_PIX * anonym = (anonym0 = m->image,
                &anonym0->Adr[(x)*anonym0->Len0+(y+(knob-1UL)*m->yknob)]);
               if (y>=m->yknob-7UL) {
                  if (x<val+8UL) {
                     anonym->r = 700U;
                     anonym->g = 201U;
                     anonym->b = 0U;
                  }
                  else {
                     anonym->r = 40U;
                     anonym->g = 51U;
                     anonym->b = 50U;
                  }
               }
               else {
                  anonym->r = 150U;
                  anonym->g = 161U;
                  anonym->b = 150U;
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
   }
   /*  Colset(col, "Y"); */
   col.r = 256UL;
   col.g = 201UL;
   col.b = 256UL;
   maptool_drawstri(m->image, s, s_len, (long)textsx,
                (long)((knob-1UL)*m->yknob+2UL), 1000UL, 1UL, col, 0, 0);
   maptool_drawstri(m->image, w, w_len, (long)textwx,
                (long)((knob-1UL)*m->yknob+2UL), 1000UL, 1UL, col, 0, 0);
   X2C_PFREE(s);
   X2C_PFREE(w);
} /* end potival() */

#define useri_YCHAR 12


static void images(aprsdecode_pOPHIST op, char cmd, unsigned short wxset)
{
   pMENU menu;
   unsigned long ym;
   unsigned long xm;
   unsigned short wset;
   unsigned short ws;
   unsigned char wi;
   char test;
   struct aprstat_LASTVAL lastval;
   maptool_pIMAGE img1;
   float km;
   unsigned long rc;
   unsigned long ac;
   unsigned long mc;
   unsigned long bc;
   switch ((unsigned)cmd) {
   case 's':
      wset = 0x80U;
      break;
   case 'b':
      wset = 0x100U;
      break;
   case 'n':
      wset = 0x200U;
      break;
   case 'w':
      wset = 0x1U;
      break;
   case 'p':
      wset = 0x2U;
      break;
   case 'h':
      wset = 0x4U;
      break;
   case 'j':
      wset = 0x8U;
      break;
   case 'd':
      wset = 0x10U;
      break;
   case 'r':
      wset = 0x20U;
      break;
   case 'l':
      wset = 0x40U;
      break;
   default:;
      wset = wxset;
      break;
   } /* end switch */
   for (wi = aprsdecode_wTEMP;; wi++) {
      if (X2C_IN((long)wi,11,wset)) {
         test = 0;
         ws = 1U<<wi;
         img1 = 0;
         if (wi==aprsdecode_wSHIST) aprstat_kmhist(&img1, op, &test);
         else if (wi==aprsdecode_wBHIST) aprstat_btimehist(&img1, op);
         else if (wi==aprsdecode_wAHIST) {
            aprstat_althist(&img1, op, &test, &km, &bc, &mc, &ac, &rc);
         }
         else aprstat_wxgraph(&img1, op, aprsdecode_systime, &ws, &lastval);
         if (img1) {
            allocmenu(&menu, 0UL, 0UL, 0);
            menu->image = img1;
            xm = menu->image->Len1-1;
            ym = menu->image->Len0-1;
            /*        IF imagesx0<xm THEN menu^.x0:=imagesx1 */
            /*        ELSIF imagesx0>=xm THEN menu^.x0:=imagesx0-xm ELSE menu^.x0:=0 END;
                 */
            menu->x0 = (unsigned long)popxbase(2147483647UL);
            /*        menu^.y0:=mainys()-VAL(CARDINAL,click.y); */
            menu->xsize = xm+1UL;
            menu->ysize = ym+1UL;
            menu->wid = 20UL+(unsigned long)wi;
            /*        IF lums.headmenuy THEN joinmenux(menu) END; */
            menu->y00 = 0UL;
            appendmenu(menu);
         }
      }
      if (wi==aprsdecode_wSIEV) break;
   } /* end for */
} /* end images() */


static unsigned long menuimgy(unsigned long liney, unsigned long entries)
/* return menu image y size */
{
   return liney*entries+4UL;
} /* end menuimgy() */


static void wrcolor(maptool_pIMAGE img, const char s[], unsigned long s_len,
                unsigned long leftmargin, unsigned long bri, char prop,
                unsigned long * y)
{
   struct aprsdecode_COLTYP colw;
   struct aprsdecode_COLTYP col;
   long inc;
   unsigned long i;
   unsigned long x;
   x = leftmargin;
   /*  Colset(col, "W"); */
   useri_ColConfset(&colw, useri_fCOLMENUTEXT);
   col = colw;
   i = 0UL;
   while ((i<=s_len-1 && s[i]) && x<img->Len1-1) {
      if (s[i]=='\012') {
         x = leftmargin;
         *y -= aprsdecode_lums.fontysize;
      }
      else if (s[i]=='\371') maptool_Colset(&col, 'B');
      else if (s[i]=='\370') maptool_Colset(&col, 'R');
      else if (s[i]=='\372') maptool_Colset(&col, 'Y');
      else if (s[i]=='\373') maptool_Colset(&col, 'O');
      else if (s[i]=='\374') maptool_Colset(&col, 'V');
      else if (s[i]=='\367') maptool_Colset(&col, 'L');
      else if (s[i]=='\376') {
         col = colw; /*Colset(col, "W");*/
      }
      else if (s[i]=='\375') {
         maptool_drawsym(img, s[i+1UL], s[i+2UL], 0, (float)(x+8UL),
                (float)((*y+8UL)-1UL), 1000UL);
         x += 16UL;
      }
      else {
         maptool_drawchar(img, s[i], (float)x, (float)*y, &inc, bri, 1UL,
                col, 0);
         if (prop) x += (unsigned long)inc;
         else x += 6UL;
      }
      ++i;
   }
} /* end wrcolor() */

#define useri_R 8

#define useri_MARGIN0 3

#define useri_TEXTLINESPACE 7


static void textwin(unsigned long xw, unsigned long lines, unsigned long xpo,
                 unsigned long ypo, unsigned long ident, unsigned long time0,
                 char color, char s[], unsigned long s_len, char mstr[],
                unsigned long mstr_len, char cstr[], unsigned long cstr_len)
{
   pMENU menu;
   unsigned long i;
   unsigned long ye;
   unsigned long xe;
   unsigned long ym;
   unsigned long xm;
   unsigned long y;
   unsigned long x;
   long bb;
   long gg;
   long rr;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   maptool_pIMAGE anonym1;
   maptool_pIMAGE anonym2;
   unsigned long tmp;
   unsigned long tmp0;
   X2C_PCOPY((void **)&s,s_len);
   X2C_PCOPY((void **)&mstr,mstr_len);
   X2C_PCOPY((void **)&cstr,cstr_len);
   if (redrawimg==0) {
      osi_WrStrLn(s, s_len); /* no graphical window open */
      goto label;
   }
   if (xw==0UL) goto label;
   switch ((unsigned)color) {
   case 'g':
      rr = 50L;
      gg = 250L;
      bb = 50L;
      break;
   case 'e':
      rr = 250L;
      gg = 50L;
      bb = 0L;
      break;
   case 'b':
      rr = 50L;
      gg = 50L;
      bb = 400L;
      break;
   case 'r':
      rr = 220L;
      gg = 150L;
      bb = 30L;
      break;
   case 'h':
      rr = 230L;
      gg = 210L;
      bb = 0L;
      break;
   case 'm':
      rr = 130L;
      gg = 190L;
      bb = 100L;
      break;
   default:;
      rr = 250L;
      gg = 250L;
      bb = 250L;
      break;
   } /* end switch */
   /*  IF ident>0 THEN killmenuid(ident) END; */
   allocmenu(&menu, xw, menuimgy(aprsdecode_lums.fontysize,
                lines)+(aprsdecode_lums.fontysize+7UL)*(unsigned long)
                (mstr[0UL]!=0), 0);
   xm = menu->image->Len1-1;
   ym = menu->image->Len0-1;
   if ((unsigned long)aprsdecode_click.x<xm+10UL) {
      menu->x0 = (unsigned long)aprsdecode_click.x+20UL;
   }
   else menu->x0 = (unsigned long)aprsdecode_click.x-(xm+10UL);
   /*  menu^.y0:=mainys()-VAL(CARDINAL,click.y); */
   menu->xsize = xm+1UL;
   menu->ysize = ym+1UL;
   menu->wid = ident;
   menu->background = useri_bCOLOR;
   menu->x0 = xpo;
   menu->y00 = ypo;
   menu->nohilite = 0x7FFFFFFFUL;
   /*WrInt(menu^.y0, 10);WrStrLn("=y0"); */
   tmp = menu->ysize-1UL;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = menu->xsize-1UL;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         if (mstr[0UL]) {
            { /* with */
               struct maptool_PIX * anonym = (anonym1 = menu->image,
                &anonym1->Adr[(x)*anonym1->Len0+y]);
               if (x==0UL) {
                  anonym->r = (unsigned short)(rr*2L);
                  anonym->g = (unsigned short)(gg*2L);
                  anonym->b = (unsigned short)(bb*2L);
               }
               else if (x<menu->xsize-1UL) {
                  anonym->r = (unsigned short)rr;
                  anonym->g = (unsigned short)gg;
                  anonym->b = (unsigned short)bb;
               }
               else {
                  anonym->r = (unsigned short)X2C_DIV(rr,2L);
                  anonym->g = (unsigned short)X2C_DIV(gg,2L);
                  anonym->b = (unsigned short)X2C_DIV(bb,2L);
               }
            }
         }
         else {
            if (x<=8UL) xe = 8UL-x;
            else {
               xe = menu->xsize-x;
               if (xe<=8UL) xe = 8UL-xe;
               else xe = 0UL;
            }
            if (y<=8UL) ye = 8UL-y;
            else {
               ye = menu->ysize-y;
               if (ye<=8UL) ye = 8UL-ye;
               else ye = 0UL;
            }
            { /* with */
               struct maptool_PIX * anonym0 = (anonym2 = menu->image,
                &anonym2->Adr[(x)*anonym2->Len0+y]);
               anonym0->r = (unsigned short)rr;
               anonym0->g = (unsigned short)gg;
               if (xe*xe+ye*ye<64UL) anonym0->b = (unsigned short)bb;
               else anonym0->b = 65535U;
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   y = (ym-aprsdecode_lums.fontysize)-1UL;
   wrcolor(menu->image, s, s_len, 3UL, 700UL, 1, &y);
   if (mstr[0UL]) {
      menu->yknob = aprsdecode_lums.fontysize+7UL;
      menu->oldknob = 0UL;
      addline(menu, mstr, mstr_len, cstr, cstr_len, 0UL);
      for (i = 1UL; i<=30UL; i++) {
         menu->cmds[i] = cstr[0UL];
      } /* end for */
      menu->nohilite &= ~0x1UL;
   }
   appendmenu(menu);
   if (ypo) menu->y00 = ypo;
   useri_refresh = 1;
   if (time0>0UL) menu->timeout = aprsdecode_realtime+time0;
   label:;
   X2C_PFREE(s);
   X2C_PFREE(mstr);
   X2C_PFREE(cstr);
} /* end textwin() */


extern void useri_textautomenu(long x0, long y00, unsigned long id,
                unsigned long time0, char color, char s[],
                unsigned long s_len, char mtext[], unsigned long mtext_len,
                char cmd[], unsigned long cmd_len)
{
   unsigned long xmax;
   unsigned long x;
   unsigned long n;
   long l;
   long i;
   long tmp;
   if (s[0UL] || mtext[0UL]) {
      n = (unsigned long)(s[0UL]!=0);
      xmax = 0UL;
      x = 0UL;
      l = (long)aprsstr_Length(s, s_len)-1L;
      tmp = l;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         if (s[i]=='\012') {
            ++n;
            x = 0UL;
         }
         else if (s[i]=='\375') x += 16UL;
         else if ((unsigned char)s[i]<=(unsigned char)'\201') {
            x += maptool_charwidth0(s[i]);
            if (x>xmax) xmax = x;
         }
         if (i==tmp) break;
      } /* end for */
      i = 0L;
      x = 0UL;
      while (i<=(long)(mtext_len-1) && mtext[i]) {
         x += maptool_charwidth0(mtext[i]);
         ++i;
      }
      if (x>xmax) xmax = x;
      if (x0==-3L) {
         x0 = popxbase((unsigned long)(y00+(long)
                menuimgy(aprsdecode_lums.fontysize, n+1UL)));
      }
      else if (x0<0L) {
         /*
             ELSIF x0=-2 THEN
               x0:=0; 
               lastmenuxy(0, x0, y0);
         --WrInt(x0, 10); WrInt(y0, 10); WrLn;
         */
         x0 = X2C_DIV(maptool_xsize-(long)xmax,2L);
      }
      if (y00<0L) y00 = X2C_DIV(maptool_ysize,2L)-28L;
      if (x0<0L) x0 = 0L;
      if (y00<0L) y00 = 0L;
      textwin(xmax+8UL, n, (unsigned long)x0, (unsigned long)y00, id, time0,
                color, s, s_len, mtext, mtext_len, cmd, cmd_len);
      useri_refresh = 1;
   }
} /* end textautomenu() */


extern void useri_textautosize(long x0, long y00, unsigned long id,
                unsigned long time0, char color, char s[],
                unsigned long s_len)
{
   useri_textautomenu(x0, y00, id, time0, color, s, s_len, "", 1ul, "", 1ul);
} /* end textautosize() */


extern void useri_popwatchcall(char s[], unsigned long s_len)
{
   char h[41];
   strncpy(h,"\352\355Watch:",41u);
   aprsstr_Append(h, 41ul, s, s_len);
   useri_textautomenu(-3L, 0L, 3UL, 0UL, 'b', "", 1ul, h, 41ul, "\322", 2ul);
} /* end popwatchcall() */


extern void useri_poligonmenu(void)
{
   char h[141];
   if (aprsdecode_click.insreplaceline) strncpy(h," Insert",141u);
   else strncpy(h,"   Move",141u);
   aprsstr_Append(h, 141ul, " Line| Del Line| Colour| Line Type| Filled/Open \
   ", 52ul);
   useri_textautomenu(-3L, 0L, 3UL, 0UL, 'b', "", 1ul, h, 141ul, "\331",
                2ul);
} /* end poligonmenu() */

#define useri_HELPFILENAME "help.txt"

#define useri_HL "|"

#define useri_HX ":"

#define useri_HELPWID 4


static char rh(char fb[4096], long fd, long * fl, long * fp)
{
   if (*fp>=*fl) {
      *fl = osi_RdBin(fd, (char *)fb, 4096u/1u, 4096UL);
      if (*fl<=0L) return 0;
      *fp = 0L;
   }
   ++*fp;
   return fb[*fp-1L];
} /* end rh() */


static long hopen(char fb[4096], long * fd, long * fl, long * fp,
                const char h[], unsigned long h_len, unsigned long offset)
{
   unsigned long i;
   long lc;
   char c;
   *fp = 0L;
   *fl = 0L;
   *fd = osi_OpenRead("help.txt", 9ul);
   if (!osi_FdValid(*fd)) return -1L;
   lc = 0L;
   i = 0UL;
   for (;;) {
      if (h[i]==0) break;
      c = rh(fb, *fd, fl, fp);
      if (c==0) return 0L;
      if (c=='\012' || (unsigned char)c>' ') {
         if (h[i]==c && i<h_len-1) ++i;
         else i = 0UL;
         if (c=='\012') ++lc;
      }
   }
   while (offset>0UL) {
      c = rh(fb, *fd, fl, fp);
      if (c==0) return 0L;
      if (c=='\012') --offset;
   }
   return lc;
} /* end hopen() */


extern void useri_helptext(unsigned long line, unsigned long sub,
                unsigned long xclick, unsigned long yclick, char idx[],
                unsigned long idx_len)
/* line0 new, line1 back, else hyperlink click */
{
   long fd;
   long li;
   long fp;
   long fl;
   unsigned long px;
   unsigned long tp;
   char c;
   unsigned char inh;
   char fb[4096];
   char s[2001];
   if (line==0UL) {
      /* open help */
      helpdepth = 0UL;
      strncpy(helpscroll[0U],"|",41u);
      aprsstr_Append(helpscroll[0U], 41ul, idx, idx_len);
      aprsstr_Append(helpscroll[0U], 41ul, "\012", 2ul);
   }
   else if (line==1UL) {
      /* back in history */
      if (helpdepth>0UL) {
         if (sub==1UL) helpdepth = 1UL;
         else --helpdepth;
      }
      else {
         useri_killmenuid(4UL);
         useri_refresh = 1;
         return;
      }
   }
   do {
      /*FOR tp:=0 TO helpdepth DO WrStr("<"); WrStr(helpscroll[tp]);
                WrStrLn(">") END; */
      if (line>1UL) tp = yclick/aprsdecode_lums.fontysize;
      else tp = 0UL;
      li = hopen(fb, &fd, &fl, &fp, helpscroll[helpdepth], 41ul, tp);
      if (li<0L) {
         useri_textautosize(-1L, 0L, 5UL, 2UL, 'e', "help.txt not found",
                19ul);
         return;
      }
      /* file not found */
      if (li==0L && helpdepth>0UL) {
         /* anchor not found */
         aprsstr_Assign(s, 2001ul, helpscroll[helpdepth], 41ul);
         aprsstr_Append(s, 2001ul, "index not found", 16ul);
         --helpdepth; /* try last link */
         osi_Close(fd);
         line = 1UL;
         useri_textautosize(-1L, 0L, 5UL, 2UL, 'e', s, 2001ul);
      }
      else if (line>1UL) {
         /* hyperlink click */
         if (helpdepth>=10UL) {
            for (tp = 3UL; tp<=10UL; tp++) {
               memcpy(helpscroll[tp-1UL],helpscroll[tp],41u);
                /* strip history */
            } /* end for */
         }
         else ++helpdepth;
         helpscroll[helpdepth][0U] = 0;
         tp = 0UL;
         px = 4UL;
         for (;;) {
            c = rh(fb, fd, &fl, &fp);
            if (tp==0UL && c!='|') px += maptool_charwidth0(c);
            if (c=='|') {
               if (tp==0UL) {
                  helpscroll[helpdepth][0U] = '|';
                  tp = 1UL;
               }
               else tp = 0UL;
            }
            else if (c==':' && tp>0UL) {
               /* link name end */
               if (px>xclick) {
                  tp = 0UL; /* we are right behind of click */
                  break;
               }
               do {
                  c = rh(fb, fd, &fl, &fp);
                  if ((unsigned char)c<' ') {
                     tp = 0UL;
                     goto loop_exit;
                  }
                  if (c!=':') px += maptool_charwidth0(c);
               } while (c!=':');
               /* link text end */
               if (px>=xclick) break;
               tp = 0UL;
            }
            else {
               if ((unsigned char)c<' ') {
                  tp = 0UL; /* no link in this line */
                  break;
               }
               if (tp>0UL && tp<39UL) {
                  helpscroll[helpdepth][tp] = c;
                  ++tp;
               }
            }
         }
         loop_exit:;
         if (tp>0UL) {
            helpscroll[helpdepth][tp] = '\012';
            helpscroll[helpdepth][tp+1UL] = 0;
         }
         else if (helpdepth>0UL) {
            --helpdepth; /* no link found */
         }
         osi_Close(fd);
         line = 1UL;
      }
      else if (line>0UL) line = 0UL;
   } while (line);
   tp = 0UL;
   inh = 0U;
   for (;;) {
      c = rh(fb, fd, &fl, &fp);
      if (c==0) break;
      if (c=='|') inh = 1U;
      else {
         if ((unsigned char)c<' ' && inh) break;
         if (c==':') {
            if (inh==1U) {
               c = '\373';
               inh = 2U;
            }
            else if (inh==2U) {
               c = '\376';
               inh = 0U;
            }
         }
      }
      if (inh!=1U && (c=='\012' || (unsigned char)c>=' ')) {
         /* & ((tp<>0) OR (c<>LF))*/
         s[tp] = c;
         ++tp;
         if (tp>=2000UL) break;
      }
   }
   while (tp>0UL && (unsigned char)s[tp-1UL]<=' ') --tp;
   s[tp] = 0;
   if (s[0U]==0) strncpy(s,"no text",2001u);
   if (helpdepth==0UL) strncpy(fb," Close ",4096u);
   else if (helpdepth<2UL) strncpy(fb," < Back ",4096u);
   else strncpy(fb," < Back | Index ",4096u);
   useri_textautomenu(-1L, (long)aprsdecode_lums.fontysize, 4UL, 0UL, 'b', s,
                 2001ul, fb, 4096ul, "\306", 2ul);
   osi_Close(fd);
} /* end helptext() */


extern void useri_say(char s[], unsigned long s_len, unsigned long time0,
                char color)
{
   useri_textautosize(0L, 0L, 4UL, time0, color, s, s_len);
} /* end say() */


extern void useri_sayonoff(char s[], unsigned long s_len, char on)
{
   char h[100];
   X2C_PCOPY((void **)&s,s_len);
   aprsstr_Assign(h, 100ul, s, s_len);
   if (on) aprsstr_Append(h, 100ul, " On", 4ul);
   else aprsstr_Append(h, 100ul, " Off", 5ul);
   useri_say(h, 100ul, 4UL, 'b');
   X2C_PFREE(s);
} /* end sayonoff() */


extern void useri_downloadprogress(void)
{
   char s1[1000];
   char s[1000];
   struct maptool__D0 * anonym;
   { /* with */
      struct maptool__D0 * anonym = &maptool_mappack;
      strncpy(s,"Done ",1000u);
      aprsstr_IntToStr((long)anonym->donecnt, 1UL, s1, 1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      aprsstr_Append(s, 1000ul, " of ", 5ul);
      aprsstr_IntToStr((long)anonym->mapscnt, 1UL, s1, 1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      aprsstr_Append(s, 1000ul, " missed: ", 10ul);
      aprsstr_IntToStr((long)anonym->givups, 1UL, s1, 1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      aprsstr_Append(s, 1000ul, " retries: ", 11ul);
      if (anonym->run) {
         aprsstr_IntToStr((long)anonym->retrys, 1UL, s1, 1000ul);
      }
      else aprsstr_IntToStr((long)anonym->retrysum, 1UL, s1, 1000ul);
      aprsstr_Append(s, 1000ul, s1, 1000ul);
      if (!anonym->run) aprsstr_Append(s, 1000ul, " Ready", 7ul);
      useri_textautosize(-3L, 0L, 6UL, 0UL, 'b', s, 1000ul);
   }
} /* end downloadprogress() */


static void msgpop(void)
{
   char s1[201];
   char s[201];
   pMENU m;
   aprsdecode_pMSGFIFO pn;
   aprsdecode_pMSGFIFO pm;
   long mc;
   long i;
   long y00;
   char new0;
   char col;
   struct aprsdecode_MSGFIFO * anonym;
   mc = 0L;
   pn = aprsdecode_msgfifo0;
   pm = 0;
   new0 = useri_nextmsg<0L;
   /*  IF NOT new OR configon(fPOPUPMSG) THEN */
   useri_nextmsg = labs(useri_nextmsg);
   while (pn) {
      ++mc;
      if (mc==useri_nextmsg) pm = pn;
      pn = pn->next;
   }
   if (pm==0) {
      useri_nextmsg = 1L;
      pm = aprsdecode_msgfifo0;
   }
   if (pm) {
      if (new0) useri_killmenuid(211UL);
      m = findmenuid(211UL);
      if (m==0) {
         { /* with */
            struct aprsdecode_MSGFIFO * anonym = pm;
            if (anonym->query) strncpy(s,"Query (",201u);
            else strncpy(s,"Message (",201u);
            i = useri_nextmsg;
            if (i>mc) i = mc;
            aprsstr_IntToStr(i, 1UL, s1, 201ul);
            aprsstr_Append(s, 201ul, s1, 201ul);
            aprsstr_Append(s, 201ul, "/", 2ul);
            aprsstr_IntToStr(mc, 1UL, s1, 201ul);
            aprsstr_Append(s, 201ul, s1, 201ul);
            aprsstr_Append(s, 201ul, ") from [", 9ul);
            aprsstr_Append(s, 201ul, anonym->from, 9ul);
            aprsstr_Append(s, 201ul, "] to [", 7ul);
            aprsstr_Append(s, 201ul, anonym->to, 9ul);
            if (anonym->ack[0UL]) {
               aprsstr_Append(s, 201ul, "] ack[", 7ul);
               aprsstr_Append(s, 201ul, anonym->ack, 5ul);
            }
            aprsstr_Append(s, 201ul, "] Port[", 8ul);
            aprsstr_Append(s, 201ul, (char *) &anonym->port, 1u/1u);
            aprstext_DateLocToStr(anonym->time0, s1, 201ul);
            aprsstr_Append(s, 201ul, "] ", 3ul);
            aprsstr_Append(s, 201ul, s1, 201ul);
            aprsstr_Append(s, 201ul, "\012[", 3ul);
            aprsstr_Append(s, 201ul, anonym->txt, 67ul);
            aprsstr_Append(s, 201ul, "]", 2ul);
            if (sndmsg) y00 = 20L;
            else y00 = -1L;
            if (new0) col = 'r';
            else col = 'b';
            useri_textautomenu(-1L, y00, 211UL, 0UL, col, s, 201ul, "    Clos\
e  |     Next  |    Reply  |    Delete", 47ul, "\243", 2ul);
         }
      }
   }
   else {
      useri_nextmsg = 0L;
      useri_textautosize(0L, 0L, 6UL, 4UL, 'b', "No Messages", 12ul);
   }
/*  END; */
} /* end msgpop() */


extern void useri_clrmsgtext(char to[], unsigned long to_len)
{
   char s[100];
   useri_confstr(useri_fMSGTO, s, 100ul);
   if (!aprsstr_StrCmp(to, to_len, s, 100ul)) {
      /* msg to another call so clear text */
      icfg(useri_fMSGTEXT, "", 1ul);
      configs[useri_fMSGTEXT].curspos = 0U;
   }
} /* end clrmsgtext() */


static void reply(void)
{
   aprsdecode_pMSGFIFO pm;
   long i;
   char oldport;
   if (aprsdecode_msgfifo0) {
      pm = aprsdecode_msgfifo0;
      i = 1L;
      while (labs(useri_nextmsg)>i && pm->next) {
         pm = pm->next;
         ++i;
      }
      useri_clrmsgtext(pm->from, 9ul);
      icfg(useri_fMSGTO, pm->from, 9ul);
      useri_confstr(useri_fMSGPORT, (char *) &oldport, 1u/1u);
      if (oldport!='A') {
         icfg(useri_fMSGPORT, (char *) &pm->port, 1u/1u);
                /* if not autoport copy rx port to tx port */
      }
      sndmsg = 1;
   }
} /* end reply() */


static void msgtoobj(void)
{
   aprsdecode_pOPHIST op;
   op = aprsdecode_selop();
   if (op) {
      useri_clrmsgtext(op->call, 9ul);
      icfg(useri_fMSGTO, op->call, 9ul);
      if (op->lastrxport) {
         icfg(useri_fMSGPORT, (char *) &op->lastrxport, 1u/1u);
      }
      sndmsg = 1;
   }
} /* end msgtoobj() */


static void delmsgfifo(void)
{
   aprsdecode_pMSGFIFO po;
   aprsdecode_pMSGFIFO pm;
   long i;
   if (aprsdecode_msgfifo0) {
      /*WrInt(nextmsg, 4);WrStrLn(" n"); */
      pm = aprsdecode_msgfifo0;
      if (pm->next==0 || labs(useri_nextmsg)<=1L) {
         aprsdecode_msgfifo0 = aprsdecode_msgfifo0->next;
      }
      else {
         po = 0;
         i = 1L;
         while (useri_nextmsg>i && pm->next) {
            po = pm;
            pm = pm->next;
            ++i;
         }
         /*WrInt(i, 3);WrLn; */
         if (po) po->next = pm->next;
      }
      Storage_DEALLOCATE((X2C_ADDRESS *) &pm,
                sizeof(struct aprsdecode_MSGFIFO));
   }
   useri_killmenuid(211UL);
   useri_refresh = 1;
} /* end delmsgfifo() */


static void getwxset(unsigned char set, char * ch, unsigned short * wset)
{
   unsigned long i;
   char s[100];
   useri_confstr(set, s, 100ul);
   *wset = 0U;
   *ch = s[0U];
   if (*ch=='0') {
      *ch = 0;
      i = 0UL;
      for (;;) {
         if (s[i+1UL]=='1') *wset |= (1U<<i);
         else if (s[i+1UL]!='0') break;
         ++i;
         if (i>8UL) break;
      }
      if (s[i+1UL]=='u') *ch = 'u';
   }
   icfg(set, s, 100ul);
} /* end getwxset() */


static void textinfo(unsigned long typ)
{
   char s[1001];
   char col;
   struct aprsdecode_CLICKOBJECT * anonym;
   /*WrInt(typ, 5);WrStrLn("ti"); */
   aprstext_optext(typ, &aprsdecode_click.table[aprsdecode_click.selected],
                &aprsdecode_click.watchlast, s, 1001ul);
   { /* with */
      struct aprsdecode_CLICKOBJECT * anonym = &aprsdecode_click.table[aprsdecode_click.selected]
                ;
      if (anonym->opf && anonym->pff0) {
         if (typ==2UL) {
            aprstext_setmarkalti(0, anonym->opf, 0);
                /* not use last frame for marker because marker should be at symbol position but symbol is on last ERROR-FREE position */
         }
         else aprstext_setmarkalti(anonym->pff0, anonym->opf, 0);
      }
   }
   col = 'b';
   if (aprsdecode_click.watchlast) col = 'r';
   if (s[0U]) useri_textautosize(-3L, 0L, 2UL, 0UL, col, s, 1001ul);
} /* end textinfo() */


extern void useri_refrinfo(void)
{
   char s[100];
   unsigned char t;
   if (aprsdecode_click.entries>0UL) {
      t = aprsdecode_click.table[aprsdecode_click.selected].typf;
      s[0] = 0;
      if (t==aprsdecode_tTEXT || t==aprsdecode_tOBJECTTEXT) {
         useri_confstr(useri_fCLICKTEXT, s, 100ul);
      }
      else if (t==aprsdecode_tSYMBOL || t==aprsdecode_tOBJECT) {
         useri_confstr(useri_fCLICKSYM, s, 100ul);
      }
      if (aprsstr_InStr(s, 100ul, "u", 2ul)>=0L) textinfo(2UL);
      images(aprsdecode_click.table[aprsdecode_click.selected].opf, 0,
                aprsdecode_click.graphset);
   }
} /* end refrinfo() */


extern void useri_hoverinfo(struct aprsdecode_CLICKOBJECT obj)
{
   char hh;
   unsigned short graphset;
   char s[1001];
   char col;
   char void0;
   if (findmenuid(2UL)==0) {
      hintmouse = useri_xmouse;
      getwxset(useri_fHOVERSET, &hh, &graphset);
                /* what wx images are enabled */
      hoveropen = 1;
      /*  IF xmouse.x>20 THEN imagesx0:=xmouse.x-20 ELSE imagesx0:=0 END; */
      /*  imagesx1:=xmouse.x+20; */
      if (hh=='u') {
         aprstext_optext(2UL, &obj, &void0, s, 1001ul);
         col = 'm';
         if (s[0U]) useri_textautosize(-3L, 0L, 7UL, 0UL, 'm', s, 1001ul);
      }
      images(obj.opf, 0, graphset);
   }
} /* end hoverinfo() */


static void oneclickinfo(void)
{
   unsigned long i;
   unsigned short wset;
   maptool_pIMAGE img;
   struct aprstat_LASTVAL lastval;
   char hh;
   char s[100];
   unsigned char ct;
   if (aprsdecode_click.entries==0UL) return;
   i = aprsdecode_click.selected;
   hh = 0;
   aprsdecode_click.graphset = 0U;
   ct = aprsdecode_click.table[i].typf;
   if (ct==aprsdecode_tSYMBOL || ct==aprsdecode_tOBJECT) {
      getwxset(useri_fCLICKWXSYM, &hh, &aprsdecode_click.graphset);
                /* what wx images are enabled */
      wset = 0U;
      img = 0;
      aprstat_wxgraph(&img, aprsdecode_selop(), aprsdecode_systime, &wset,
                &lastval); /* what do we have */
      if ((wset&aprsdecode_click.graphset)==0U) {
         /* no wx to show */
         useri_confstr(useri_fCLICKSYM, s, 100ul);
         if (aprsstr_InStr(s, 100ul, "s", 2ul)>=0L) {
            aprsdecode_click.graphset |= 0x80U;
         }
         if (aprsstr_InStr(s, 100ul, "b", 2ul)>=0L) {
            aprsdecode_click.graphset |= 0x100U;
         }
         if (aprsstr_InStr(s, 100ul, "n", 2ul)>=0L) {
            aprsdecode_click.graphset |= 0x200U;
         }
         if (aprsstr_InStr(s, 100ul, "u", 2ul)>=0L) hh = 'u';
         else hh = 0;
         if (aprsdecode_click.graphset==0U) hh = s[0U];
      }
   }
   else if (ct==aprsdecode_tTEXT || ct==aprsdecode_tOBJECTTEXT) {
      /* text click */
      useri_confstr(useri_fCLICKTEXT, s, 100ul);
      if (aprsstr_InStr(s, 100ul, "s", 2ul)>=0L) {
         aprsdecode_click.graphset |= 0x80U;
      }
      if (aprsstr_InStr(s, 100ul, "b", 2ul)>=0L) {
         aprsdecode_click.graphset |= 0x100U;
      }
      if (aprsstr_InStr(s, 100ul, "n", 2ul)>=0L) {
         aprsdecode_click.graphset |= 0x200U;
      }
      if (aprsstr_InStr(s, 100ul, "u", 2ul)>=0L) hh = 'u';
   }
   else if (ct==aprsdecode_tTRACK) {
      /*    IF (click.graphset=sWXSET{}
                ) & (InStr(s, CFGRAWDECODED)>=0 THEN hh:=CFGRAWDECODED END;
                */
      /* track click */
      useri_confstr(useri_fCLICKTRACK, s, 100ul);
      if (aprsdecode_click.mhop[0UL] || aprsstr_InStr(s, 100ul, ".",
                2ul)<0L) {
         /* only if 1 user or not goto 1 user */
         if (aprsstr_InStr(s, 100ul, "s", 2ul)>=0L) {
            aprsdecode_click.graphset |= 0x80U;
         }
         if (aprsstr_InStr(s, 100ul, "b", 2ul)>=0L) {
            aprsdecode_click.graphset |= 0x100U;
         }
         if (aprsstr_InStr(s, 100ul, "n", 2ul)>=0L) {
            aprsdecode_click.graphset |= 0x200U;
         }
      }
      if (aprsstr_InStr(s, 100ul, "u", 2ul)>=0L) hh = 'u';
   }
   if (hh=='u') {
      /*WrStr(hh); WrStrLn(" texti"); */
      if (ct==aprsdecode_tTRACK) {
         textinfo(3UL); /* click.cmd:=">";*/ /* refresh:=TRUE;*/
      }
      else textinfo(2UL);
      aprsdecode_click.cmd = ' ';
   }
   images(aprsdecode_click.table[aprsdecode_click.selected].opf, hh,
                aprsdecode_click.graphset);
   useri_refresh = 1;
} /* end oneclickinfo() */


static void updatemenus(void)
{
   pMENU m;
   m = menus;
   while (m) {
      if (m->redrawproc!=0) {
         X2C_CHKPROC(void ( *)(pMENU),m->redrawproc)(m);
                /* m^.redrawproc:=NIL*/
      }
      m = m->next;
   }
   useri_refresh = 1;
} /* end updatemenus() */


static void Viewline(char s[], unsigned long s_len)
{
   struct aprspos_POSITION pos;
   float zoom;
   unsigned long i;
   char h[101];
   s[0UL] = 0;
   i = 0UL;
   do {
      zoom = 0.0f;
      aprsdecode_posinval(&pos);
      useri_getview(useri_fVIEW, i, &zoom, &pos);
      if (zoom==0.0f) {
         strncpy(h,"P ",101u);
         h[1U] = (char)(i+49UL);
      }
      else aprsstr_FixToStr(zoom+0.5f, 0UL, h, 101ul);
      if (aprsstr_Length(h, 101ul)==1UL) {
         aprsstr_Append(s, s_len, "\365", 2ul);
         aprsstr_Append(h, 101ul, "\363 ", 3ul);
      }
      else {
         aprsstr_Append(s, s_len, "\363", 2ul);
         aprsstr_Append(h, 101ul, "\367", 2ul);
      }
      aprsstr_Append(s, s_len, h, 101ul);
      ++i;
      if (i<4UL) aprsstr_Append(s, s_len, "|", 2ul);
   } while (!(zoom==0.0f || i>=4UL));
} /* end Viewline() */


static void setunderbar(pMENU m, long x)
{
   if (aprsdecode_lums.headmenuy) {
      if (x<0L) x = 0L;
      m->x0 = (unsigned long)x;
      m->y00 = aprsdecode_lums.fontysize;
   }
} /* end setunderbar() */


static void setmenu(pMENU menu, unsigned long liney, unsigned long entries,
                unsigned char background)
{
   unsigned long xm;
   long yh;
   xm = menu->image->Len1-1;
   if ((unsigned long)aprsdecode_click.x<xm+10UL) {
      menu->x0 = (unsigned long)aprsdecode_click.x+20UL;
   }
   else menu->x0 = (unsigned long)aprsdecode_click.x-(xm+10UL);
   menu->y00 = useri_mainys()-(unsigned long)aprsdecode_click.y;
   menu->yknob = liney;
   menu->xsize = xm+1UL;
   menu->background = background;
   yh = (long)liney;
   if (entries<=1UL) yh = (long)(liney/2UL);
   if (useri_xmouse.y>yh) menu->y00 = (unsigned long)(useri_xmouse.y-yh);
   else menu->y00 = 0UL;
   appendmenu(menu);
   clampedline = 0UL;
} /* end setmenu() */


static void newmenu(pMENU * menu, unsigned long linex, unsigned long liney,
                unsigned long entries, unsigned char background)
{
   allocmenu(menu, linex, menuimgy(liney, entries), 0);
   setmenu(*menu, liney, entries, background);
} /* end newmenu() */


static void refrmenu(pMENU * menu, unsigned long linex, unsigned long liney,
                unsigned long entries, unsigned char background,
                char saveimg)
/* refresh/resize menu */
{
   unsigned long ys;
   if (*menu) {
      if (saveimg) {
         if ((*menu)->image==0) {
            (*menu)->image = useri_panoimage;
            useri_panoimage = 0;
         }
      }
      /*
      WrInt(ORD(menu^.image=NIL), 3); WrInt(ORD(saveimg), 3);
      WrInt(linex, 6); WrInt(menu^.xsize, 6); 
      WrInt(liney, 6); WrInt(menu^.yknob, 6);
      WrStrLn("");
      */
      ys = menuimgy(liney, entries);
      /*IF menu^.image<>NIL THEN WrInt(ys, 10);
                WrInt(HIGH(menu^.image^[0])+1, 10); WrStrLn(" ys ysize") END;
                 */
      if ((((*menu)->image==0 || linex!=(*menu)->xsize) || liney!=(*menu)
                ->yknob) || ys!=((*menu)->image->Len0-1)+1UL) {
         disposeimg(&(*menu)->image);
         useri_allocimage(&(*menu)->image, (long)linex, (long)ys,
                (*menu)->saveimage);
         if ((*menu)->image==0) {
            osi_WrStrLn("menuimage out of memory", 24ul);
            useri_wrheap();
            return;
         }
         (*menu)->yknob = liney;
         (*menu)->xsize = linex;
         if (saveimg) maptool_clr((*menu)->image);
      }
   }
   else {
      allocmenu(menu, linex, menuimgy(liney, entries), saveimg);
      setmenu(*menu, liney, entries, background);
   }
} /* end refrmenu() */

static void domainpop(pMENU);


static void domainpop(pMENU m)
{
   unsigned long oldk;
   unsigned long olds;
   long entc;
   char s[101];
   struct aprsdecode_OPHIST * anonym;
   char tmp;
   oldk = m->oldknob;
   olds = m->oldsub;
   m->oldknob = 0UL;
   for (entc = (long)aprsdecode_click.entries-1L; entc>=0L; entc--) {
      if (aprsdecode_click.table[entc].opf && m->oldknob+1UL<10UL) {
         { /* with */
            struct aprsdecode_OPHIST * anonym = aprsdecode_click.table[entc]
                .opf;
            strncpy(s,"\365\366|\365\365\367",101u);
            aprsstr_Append(s, 101ul, anonym->call, 9ul);
            addonoff(m, s, 101ul, "?", 2ul, 101UL, 6L,
                entc==(long)aprsdecode_click.selected);
            maptool_drawsym(m->image, anonym->sym.tab, anonym->sym.pic, 0,
                24.0f, (float)((m->oldknob-1UL)*m->yknob+m->yknob/2UL),
                1000UL);
         }
      }
   } /* end for */
   if (!aprsdecode_lums.headmenuy) {
      addline(m, " Tools", 7ul, "\220>", 3ul, 103UL);
   }
   if (aprsdecode_click.entries>0UL) {
      if (aprsdecode_click.selected>=aprsdecode_click.entries) {
         aprsdecode_click.selected = 0UL;
      }
      addline(m, " More Infos", 12ul, "i>", 3ul, 104UL);
      if (aprsdecode_click.table[aprsdecode_click.selected]
                .opf && aprspos_distance(aprsdecode_click.table[aprsdecode_click.selected]
                .opf->margin0, aprsdecode_click.table[aprsdecode_click.selected].opf->margin1)>0.05f) {
         addline(m, " Animate Object", 16ul, "a", 2ul, 105UL);
      }
      /*  IF click.mhop[0]<>0C THEN addline(m, " Focus |+ Rfpath", CMD1USER,
                MINH+8); */
      /*  ELSE  */
      addline(m, " Show 1|+ Rfpath", 17ul, ".", 2ul, 106UL);
      /*  END; */
      addline(m, " Heard | Message", 17ul, "\330", 2ul, 110UL);
   }
   if (aprsdecode_click.mhop[0UL]) {
      addline(m, " Focus | Show All", 18ul, "0", 2ul, 112UL);
   }
   if (!aprsdecode_lums.headmenuy) {
      addline(m, " On next Click", 15ul, "K>", 3ul, 123UL);
   }
   if (aprsdecode_click.entries==0UL) {
      addline(m, "  1\363|\361Clr\363|\362Clr\362| 2", 20ul, "\252", 2ul,
                115UL);
      Viewline(s, 101ul);
      addline(m, s, 101ul, "1", 2ul, 125UL);
   }
   if (aprspos_posvalid(aprsdecode_click.markpos)
                && aprspos_posvalid(aprsdecode_click.measurepos)) {
      addline(m, " Zoom to Markers", 17ul, "/", 2ul, 120UL);
   }
   addline(m, "  -  |     \363|   +", 18ul, (char *)(tmp = '\030',&tmp),
                1u/1u, 130UL);
   subicon(m->image, 47L, midy(m)+2L, 3L, 7L);
   addline(m, "    |\363Center\364|", 15ul, (char *)(tmp = '\023',&tmp),
                1u/1u, 133UL);
   subicon(m->image, 13L, midy(m), 1L, 7L);
   subicon(m->image, (long)(m->xsize-13UL), midy(m), 2L, 7L);
   addline(m, " <<  |     \363|   X", 18ul, (char *)(tmp = '\005',&tmp),
                1u/1u, 136UL);
   subicon(m->image, 47L, midy(m)-2L, 4L, 7L);
   m->ysize = m->oldknob*m->yknob;
   m->redrawproc = domainpop;
   m->hiknob = 0UL;
   m->oldknob = oldk;
   m->oldsub = olds;
   m->notoverdraw = 1;
   if (aprsdecode_click.entries>0UL) oneclickinfo();
} /* end domainpop() */

#define useri_MENUXLEFT 16


extern void useri_mainpop(void)
{
   pMENU menu;
   long yh;
   useri_killallmenus();
   newmenu(&menu, 94UL, aprsdecode_lums.fontysize+7UL, 30UL, useri_bTRANSP);
   if (!aprsdecode_lums.headmenuy) {
      /* set popup mainmenue to not hide symbol and its texts */
      yh = (long)useri_mainys()-aprsdecode_click.y;
      if ((unsigned long)aprsdecode_click.x<menu->xsize+16UL) {
         menu->x0 = (unsigned long)aprsdecode_click.x+16UL;
         yh += 10L; /* under the sybol */
      }
      else {
         menu->x0 = (unsigned long)aprsdecode_click.x-(menu->xsize+16UL);
         yh -= 10L;
      }
      if (yh<0L) yh = 0L;
      menu->y00 = (unsigned long)yh;
   }
   else setunderbar(menu, 0L);
   domainpop(menu);
   menu->oldknob = 0UL;
} /* end mainpop() */


static void toolbar(void)
{
   pMENU menu;
   newmenu(&menu, 187UL, aprsdecode_lums.fontysize, 1UL, useri_bTRANSP);
   addline(menu, " File |Config\362| Zoom | Tools | Help ", 37ul, "\206",
                2ul, 200UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->x0 = 0UL;
   menu->y00 = 0UL;
   menu->wid = 255UL;
} /* end toolbar() */

static void dofilemenu(pMENU);


static void dofilemenu(pMENU menu)
{
   menu->oldknob = 0UL;
   addline(menu, " Quit", 6ul, "Q", 2ul, 300UL);
   addline(menu, " Keep Log Days", 15ul, "\323", 2ul, 309UL);
   addonoff(menu, "   |Write Log", 14ul, "\323", 2ul, 307UL, 8L,
                useri_configon(useri_fLOGWFN));
   addline(menu, " Make Video", 12ul, "\312", 2ul, 301UL);
   addline(menu, " Screenshot", 12ul, "\233>", 3ul, 302UL);
   addline(menu, " Import Log", 12ul, "\235>", 3ul, 303UL);
   menu->redrawproc = dofilemenu;
   menu->hiknob = 0UL;
} /* end dofilemenu() */


static void filemenu(void)
{
   pMENU menu;
   newmenu(&menu, 82UL, aprsdecode_lums.fontysize+7UL, 10UL, useri_bTRANSP);
   dofilemenu(menu);
   setunderbar(menu, 0L);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->wid = 249UL;
   menu->notoverdraw = 1;
} /* end filemenu() */

static void listmenu(pMENU);


static void listmenu(pMENU m)
{
   char h[100];
   char s[100];
   char mo[100];
   unsigned long i;
   m->oldknob = 0UL;
   strncpy(mo," - | - | - | - | - | Monitor",100u);
   useri_confstr(useri_fWRINCOM, s, 100ul);
   for (i = 0UL; i<=4UL; i++) {
      if (i==0UL) h[0U] = 'n';
      else h[0U] = (char)(i+48UL);
      h[1U] = 0;
      if (aprsstr_InStr(s, 100ul, h, 100ul)>=0L) {
         aprsstr_Append(h, 100ul, "+", 2ul);
         if (aprsstr_InStr(s, 100ul, h, 100ul)>=0L) mo[i*4UL+1UL] = 'd';
         else mo[i*4UL+1UL] = 'r';
      }
   } /* end for */
   addline(m, " Internals", 11ul, "\247", 2ul, 440UL);
   addline(m, " Bulletins", 11ul, "\247", 2ul, 435UL);
   addline(m, " Messages", 10ul, "\247", 2ul, 430UL);
   addline(m, mo, 100ul, "\247", 2ul, 420UL);
   addline(m, " No Pos Beacons| decode", 24ul, "\247", 2ul, 415UL);
   addline(m, " Moving Beacons| decode", 24ul, "\247", 2ul, 410UL);
   addline(m, " Wx Stations   | decode", 24ul, "\247", 2ul, 405UL);
   addline(m, " Objects/Items | decode", 24ul, "\247", 2ul, 400UL);
   m->redrawproc = listmenu;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end listmenu() */


static void listmen(void)
{
   pMENU menu;
   newmenu(&menu, 140UL, aprsdecode_lums.fontysize+7UL, 10UL, useri_bTRANSP);
   listmenu(menu);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   /*  setunderbar(menu, 0); */
   menu->wid = 249UL;
   menu->notoverdraw = 1;
} /* end listmen() */


static void monconfig(unsigned long sub)
{
   char h[100];
   char s[100];
   char mo[100];
   unsigned long st;
   unsigned long i;
   char tmp;
   mo[0U] = 0;
   useri_confstr(useri_fWRINCOM, s, 100ul);
   for (i = 0UL; i<=4UL; i++) {
      if (i==0UL) h[0U] = 'n';
      else h[0U] = (char)(i+48UL);
      h[1U] = 0;
      st = 0UL;
      if (aprsstr_InStr(s, 100ul, h, 100ul)>=0L) {
         aprsstr_Append(h, 100ul, "+", 2ul);
         if (aprsstr_InStr(s, 100ul, h, 100ul)>=0L) st = 2UL;
         else st = 1UL;
      }
      if (i==sub) st = (st+1UL)%3UL;
      if (st>0UL) {
         if (i==0UL) aprsstr_Append(mo, 100ul, "n", 2ul);
         else {
            aprsstr_Append(mo, 100ul, (char *)(tmp = (char)(i+48UL),&tmp),
                1u/1u);
         }
         if (st==2UL) aprsstr_Append(mo, 100ul, "+", 2ul);
      }
   } /* end for */
   icfg(useri_fWRINCOM, mo, 100ul);
   updatemenus();
} /* end monconfig() */

static void dotoolsmenu(pMENU);


static void dotoolsmenu(pMENU menu)
{
   menu->oldknob = 0UL;
   addline(menu, " Animate", 9ul, "A", 2ul, 505UL);
   /*  IF click.mhop[0]<>0C THEN addline(menu, " Show All", "0",
                MINH*5+10) END; */
   if (aprsdecode_lums.headmenuy && aprspos_posvalid(aprsdecode_click.markpos)
                ) addline(menu, " Clear Markers", 15ul, ":", 2ul, 515UL);
   /*  addonoff(menu, "   |Radiolink", CMDGEOPROFIL, MINH*5+17, 8,
                configon(fGEOPROFIL)); */
   /*
     addline(menu, " WXonly|  All ", CMDRAIN, MINH*5+20);
     addline(menu, " Temp  |  Rain", "W", MINH*5+25);
     addline(menu, " Dimm Notmover", "m", MINH*5+30);
   */
   addline(menu, " Send Message", 14ul, "\256", 2ul, 535UL);
   addline(menu, " Map directory", 15ul, "\303", 2ul, 536UL);
   addline(menu, " Add Maptypes", 14ul, "\301", 2ul, 538UL);
   addline(menu, " Select Data", 13ul, "\324>", 3ul, 522UL);
   addonoff(menu, "   |Radiolink", 14ul, "\313>", 3ul, 517UL, 8L,
                useri_configon(useri_fGEOPROFIL));
   addline(menu, " Find", 6ul, "F>", 3ul, 500UL);
   addline(menu, " Choose Maps", 13ul, "c>", 3ul, 540UL);
   addline(menu, " List", 6ul, "\222>", 3ul, 545UL);
   if (aprsdecode_lums.headmenuy) {
      addline(menu, " On next Click", 15ul, "K>", 3ul, 550UL);
      addline(menu, " Toolbar Off", 13ul, "\214", 2ul, 555UL);
      menu->x0 = 65UL;
      menu->y00 = aprsdecode_lums.fontysize;
   }
   else {
      addline(menu, " Help", 6ul, "\221>", 3ul, 560UL);
      addline(menu, " Config", 8ul, "\211>", 3ul, 565UL);
      addline(menu, " File", 6ul, "\217>", 3ul, 570UL);
      addline(menu, " Toolbar On", 12ul, "\215", 2ul, 575UL);
   }
   menu->redrawproc = dotoolsmenu;
   menu->ysize = menu->oldknob*menu->yknob;
   menu->hiknob = 0UL;
   menu->wid = 249UL;
   menu->notoverdraw = 1;
} /* end dotoolsmenu() */


static void toolsmenu(void)
{
   pMENU menu;
   newmenu(&menu, 88UL, aprsdecode_lums.fontysize+5UL, 16UL, useri_bTRANSP);
   dotoolsmenu(menu);
   /*  menu^.ysize:=menu^.oldknob*menu^.yknob; */
   menu->oldknob = 0UL;
} /* end toolsmenu() */


static void drawsquare(maptool_pIMAGE image, struct maptool_PIX col, long x0,
                 long y00, long x1, long y1)
{
   long y;
   long x;
   long tmp;
   long tmp0;
   if (x0<0L) x0 = 0L;
   if (x1<0L) x1 = 0L;
   if (y00<0L) y00 = 0L;
   if (y1<0L) y1 = 0L;
   if (x0>(long)(image->Len1-1)) x0 = (long)(image->Len1-1);
   if (x1>(long)(image->Len1-1)) x1 = (long)(image->Len1-1);
   if (y00>(long)(image->Len0-1)) y00 = (long)(image->Len0-1);
   if (y1>(long)(image->Len0-1)) y1 = (long)(image->Len0-1);
   tmp = y1;
   y = y00;
   if (y<=tmp) for (;; y++) {
      tmp0 = x1;
      x = x0;
      if (x<=tmp0) for (;; x++) {
         image->Adr[(x)*image->Len0+y] = col;
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end drawsquare() */


static void drawsymsquare(maptool_pIMAGE image, char tab, char sym, long x0,
                long y00)
{
   struct maptool_PIX col;
   if (aprsdecode_checksymb(tab, sym)) {
      tab = '\\';
      sym = '?';
      useri_textautosize(0L, 0L, 6UL, 2UL, 'e', "illegal symbol", 15ul);
   }
   col.r = 300U;
   col.g = 301U;
   col.b = 300U;
   /*  FOR y:=-(SYMSIZE DIV 2) TO SYMSIZE DIV 2 DO */
   /*    FOR x:=-(SYMSIZE DIV 2) TO SYMSIZE DIV 2 DO image^[x0+x][y0+y]:=col END;
                 */
   /*  END; */
   drawsquare(image, col, x0-8L, y00-8L, x0+8L, y00+8L);
   maptool_drawsym(image, tab, sym, 0, (float)x0, (float)y00, 1000UL);
} /* end drawsymsquare() */

static void specialmapmenu(pMENU);


static void specialmapmenu(pMENU m)
{
   char h[100];
   char s[100];
   m->oldknob = 0UL;
   addline(m, " Show All ", 11ul, "\300", 2ul, 8600UL);
   addline(m, " WX Stations", 13ul, "\300", 2ul, 8605UL);
   addline(m, " Temp Colourmap", 16ul, "W", 2ul, 8610UL);
   addline(m, " Rain Colourmap", 16ul, "W", 2ul, 8615UL);
   addline(m, " Dimm Notmover", 15ul, "m", 2ul, 8620UL);
   useri_confstr(useri_fONESYMB, s, 100ul);
   strncpy(h," Symbol |",100u);
   aprsstr_Append(h, 100ul, s, 100ul);
   addline(m, h, 100ul, "\325>", 3ul, 8625UL);
   if (!aprsdecode_checksymb(s[0U], s[1U])) {
      drawsymsquare(m->image, s[0U], s[1U], (long)(m->xsize-20UL),
                (long)((m->oldknob-1UL)*m->yknob+m->yknob/2UL));
   }
   m->redrawproc = specialmapmenu;
   m->hiknob = 0UL;
} /* end specialmapmenu() */


static void specialmap(void)
{
   pMENU menu;
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+7UL, 10UL, useri_bTRANSP);
   specialmapmenu(menu);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->wid = 249UL;
   menu->notoverdraw = 1;
} /* end specialmap() */


static void helpmenu(void)
{
   pMENU menu;
   newmenu(&menu, 150UL, aprsdecode_lums.fontysize+7UL, 3UL, useri_bTRANSP);
   /*  addline(menu, "Shortcuts", CMDSHORTCUTLIST, MINH*6); */
   addline(menu, "Helptext", 9ul, "\305", 2ul, 610UL);
   addline(menu, "aprsmap(cu) 0.55 by OE5DXL ", 28ul, " ", 2ul, 605UL);
   setunderbar(menu, 37L);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->wid = 249UL;
   menu->notoverdraw = 1;
} /* end helpmenu() */

#define useri_MAXBUTT 20


static void mapchoose(void)
{
   pMENU menu;
   unsigned long m;
   unsigned long n;
   unsigned long i;
   char s[101];
   char tmp;
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+7UL, 20UL, useri_bTRANSP);
   m = 0UL;
   for (n = 17UL;; n--) {
      useri_confstrings(useri_fMAPNAMES, n, 0, s, 101ul);
      if (s[0U]) {
         i = 0UL;
         while (i<100UL && (unsigned char)s[i]>' ') ++i;
         s[i] = 0;
         if (m<3UL) {
            aprsstr_Append(s, 101ul, " [", 3ul);
            aprsstr_Append(s, 101ul, (char *)(tmp = (char)(m+55UL),&tmp),
                1u/1u);
            aprsstr_Append(s, 101ul, "]", 2ul);
            ++m;
         }
         addline(menu, s, 101ul, "\302", 2ul, 710UL);
      }
      if (n==0UL) break;
   } /* end for */
   addline(menu, "Download", 9ul, "\241", 2ul, 700UL);
   addline(menu, "Reload", 7ul, "\236", 2ul, 701UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->notoverdraw = 1;
} /* end mapchoose() */


static void nextclick(void)
{
   pMENU menu;
   newmenu(&menu, 80UL, aprsdecode_lums.fontysize+7UL, 10UL, useri_bTRANSP);
   addline(menu, "Symbol Hover", 13ul, "\276>", 3ul, 905UL);
   addline(menu, "To Map", 7ul, "\276>", 3ul, 900UL);
   addline(menu, "To Text", 8ul, "\276>", 3ul, 901UL);
   addline(menu, "To Symbol", 10ul, "\276>", 3ul, 902UL);
   addline(menu, "To WxSymbol", 12ul, "\276>", 3ul, 903UL);
   addline(menu, "To Track", 9ul, "\276>", 3ul, 904UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->notoverdraw = 1;
} /* end nextclick() */


static void mapclick(void)
{
   pMENU menu;
   char s[100];
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+7UL, 10UL, useri_bTRANSP);
   useri_confstr(useri_fCLICKMAP, s, 100ul);
   addonoff(menu, "\365\365|Center", 10ul, "\201", 2ul, 1000UL, 6L,
                s[0U]=='C');
   addonoff(menu, "\365\365|Set Marker 1", 16ul, "\201", 2ul, 1005UL, 6L,
                s[0U]=='X');
   addonoff(menu, "\365\365|Set Marker 2", 16ul, "\201", 2ul, 1010UL, 6L,
                s[0U]=='Y');
   addonoff(menu, "\365\365|Marker 1 then 2", 19ul, "\201", 2ul, 1015UL, 6L,
                s[0U]=='2');
   if (!aprsdecode_lums.headmenuy) {
      addline(menu, "\365\365|Menu", 8ul, "\201", 2ul, 1020UL);
   }
   else addline(menu, "\365\365|Clear Menus", 15ul, "\201", 2ul, 1025UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end mapclick() */

#define useri_OX 6
/*      OY=8; */


static void wxonoff(pMENU menu)
{
   char ch;
   unsigned short wset;
   getwxset(useri_fCLICKWXSYM, &ch, &wset);
   menu->oldknob = 0UL;
   addonoff(menu, "\365\365|Center", 10ul, "\232", 2ul, 1100UL, 6L, ch=='C');
   addonoff(menu, "\365\365|Zoom To", 11ul, "\232", 2ul, 1101UL, 6L,
                ch=='.');
   addonoff(menu, "\365\365|Show Rf", 11ul, "\232", 2ul, 1102UL, 6L,
                ch=='=');
   addonoff(menu, "\365\365|Heard", 9ul, "\232", 2ul, 1103UL, 6L, ch=='H');
   addonoff(menu, "\365\365|Beacon Hist", 15ul, "\232", 2ul, 1104UL, 6L,
                ch=='b');
   addonoff(menu, "\365\365|Raw+Decoded", 15ul, "\232", 2ul, 1105UL, 6L,
                ch=='u');
   addonoff(menu, "\365\365|Luminosity", 14ul, "\232", 2ul, 1106UL, 6L,
                (0x40U & wset)!=0);
   addonoff(menu, "\365\365|Rain", 8ul, "\232", 2ul, 1107UL, 6L,
                (0x20U & wset)!=0);
   addonoff(menu, "\365\365|Wind Dir", 12ul, "\232", 2ul, 1108UL, 6L,
                (0x10U & wset)!=0);
   addonoff(menu, "\365\365|Wind", 8ul, "\232", 2ul, 1109UL, 6L,
                (0x8U & wset)!=0);
   addonoff(menu, "\365\365|Hygro", 9ul, "\232", 2ul, 1110UL, 6L,
                (0x4U & wset)!=0);
   addonoff(menu, "\365\365|Barometer", 13ul, "\232", 2ul, 1111UL, 6L,
                (0x2U & wset)!=0);
   addonoff(menu, "\365\365|Temp", 8ul, "\232", 2ul, 1112UL, 6L,
                (0x1U & wset)!=0);
   addline(menu, "\365\365|Menu", 8ul, "\232", 2ul, 1113UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->notoverdraw = 1;
} /* end wxonoff() */


static void wxclick(void)
{
   pMENU menu;
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+3UL, 20UL, useri_bTRANSP);
   wxonoff(menu);
} /* end wxclick() */

#define useri_OX0 6
/*      OY=8; */


static void hoveronoff(pMENU menu)
{
   char ch;
   unsigned short wset;
   getwxset(useri_fHOVERSET, &ch, &wset);
   menu->oldknob = 0UL;
   addonoff(menu, "\365\365|Raw+Decoded", 15ul, "\327", 2ul, 1151UL, 6L,
                ch=='u');
   addonoff(menu, "\365\365|Luminosity", 14ul, "\327", 2ul, 1152UL, 6L,
                (0x40U & wset)!=0);
   addonoff(menu, "\365\365|Rain", 8ul, "\327", 2ul, 1153UL, 6L,
                (0x20U & wset)!=0);
   addonoff(menu, "\365\365|Wind Dir", 12ul, "\327", 2ul, 1154UL, 6L,
                (0x10U & wset)!=0);
   addonoff(menu, "\365\365|Wind", 8ul, "\327", 2ul, 1155UL, 6L,
                (0x8U & wset)!=0);
   addonoff(menu, "\365\365|Hygro", 9ul, "\327", 2ul, 1156UL, 6L,
                (0x4U & wset)!=0);
   addonoff(menu, "\365\365|Barometer", 13ul, "\327", 2ul, 1157UL, 6L,
                (0x2U & wset)!=0);
   addonoff(menu, "\365\365|Temp", 8ul, "\327", 2ul, 1158UL, 6L,
                (0x1U & wset)!=0);
   addline(menu, "\365\365|Hover OFf", 13ul, "\327", 2ul, 1159UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->notoverdraw = 1;
} /* end hoveronoff() */


static void hoverclick(void)
{
   pMENU menu;
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+3UL, 20UL, useri_bTRANSP);
   hoveronoff(menu);
} /* end hoverclick() */


static void trackonoff(pMENU menu, char on, unsigned char v)
{
   char w[100];
   useri_confstr(v, w, 100ul);
   menu->oldknob = 0UL;
   addonoff(menu, "\365\365|Del Waypoint", 16ul, (char *) &on, 1u/1u, 1208UL,
                 6L, aprsstr_InStr(w, 100ul, "q", 2ul)>=0L);
   addonoff(menu, "\365\365|Altitude", 12ul, (char *) &on, 1u/1u, 1207UL, 6L,
                 aprsstr_InStr(w, 100ul, "n", 2ul)>=0L);
   addonoff(menu, "\365\365|Speed Hist", 14ul, (char *) &on, 1u/1u, 1200UL,
                6L, aprsstr_InStr(w, 100ul, "s", 2ul)>=0L);
   addonoff(menu, "\365\365|Animate", 11ul, (char *) &on, 1u/1u, 1201UL, 6L,
                aprsstr_InStr(w, 100ul, "A", 2ul)>=0L);
   addonoff(menu, "\365\365|Zoom To", 11ul, (char *) &on, 1u/1u, 1202UL, 6L,
                aprsstr_InStr(w, 100ul, ".", 2ul)>=0L);
   addonoff(menu, "\365\365|Show Rf", 11ul, (char *) &on, 1u/1u, 1203UL, 6L,
                aprsstr_InStr(w, 100ul, "=", 2ul)>=0L);
   addonoff(menu, "\365\365|Beacon Hist", 15ul, (char *) &on, 1u/1u, 1204UL,
                6L, aprsstr_InStr(w, 100ul, "b", 2ul)>=0L);
   addonoff(menu, "\365\365|Raw+Decoded", 15ul, (char *) &on, 1u/1u, 1205UL,
                6L, aprsstr_InStr(w, 100ul, "u", 2ul)>=0L);
   addline(menu, "\365\365|Menu", 8ul, (char *) &on, 1u/1u, 1206UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->notoverdraw = 1;
} /* end trackonoff() */


static void trackclick(void)
{
   pMENU menu;
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+3UL, 20UL, useri_bTRANSP);
   trackonoff(menu, '\204', useri_fCLICKTRACK);
} /* end trackclick() */

#define useri_OX1 6
/*      OY=8; */


static void symbolonoff(pMENU menu, char on, unsigned char v)
{
   char w[31];
   useri_confstr(v, w, 31ul);
   menu->oldknob = 0UL;
   addonoff(menu, "\365\365|Set Marker 2", 16ul, (char *) &on, 1u/1u, 1300UL,
                 6L, aprsstr_InStr(w, 31ul, "Y", 2ul)>=0L);
   addonoff(menu, "\365\365|Set Marker 1", 16ul, (char *) &on, 1u/1u, 1301UL,
                 6L, aprsstr_InStr(w, 31ul, "X", 2ul)>=0L);
   addonoff(menu, "\365\365|Center", 10ul, (char *) &on, 1u/1u, 1302UL, 6L,
                aprsstr_InStr(w, 31ul, "C", 2ul)>=0L);
   addonoff(menu, "\365\365|Raw+Decoded", 15ul, (char *) &on, 1u/1u, 1303UL,
                6L, aprsstr_InStr(w, 31ul, "u", 2ul)>=0L);
   addonoff(menu, "\365\365|Animate", 11ul, (char *) &on, 1u/1u, 1304UL, 6L,
                aprsstr_InStr(w, 31ul, "A", 2ul)>=0L);
   addonoff(menu, "\365\365|Zoom To", 11ul, (char *) &on, 1u/1u, 1305UL, 6L,
                aprsstr_InStr(w, 31ul, ".", 2ul)>=0L);
   addonoff(menu, "\365\365|Show Rf", 11ul, (char *) &on, 1u/1u, 1306UL, 6L,
                aprsstr_InStr(w, 31ul, "=", 2ul)>=0L);
   addonoff(menu, "\365\365|Heard", 9ul, (char *) &on, 1u/1u, 1307UL, 6L,
                aprsstr_InStr(w, 31ul, "H", 2ul)>=0L);
   addonoff(menu, "\365\365|Beacon Hist", 15ul, (char *) &on, 1u/1u, 1308UL,
                6L, aprsstr_InStr(w, 31ul, "b", 2ul)>=0L);
   addonoff(menu, "\365\365|Speed Hist", 14ul, (char *) &on, 1u/1u, 1309UL,
                6L, aprsstr_InStr(w, 31ul, "s", 2ul)>=0L);
   addonoff(menu, "\365\365|Altitude", 12ul, (char *) &on, 1u/1u, 1310UL, 6L,
                 aprsstr_InStr(w, 31ul, "n", 2ul)>=0L);
   addline(menu, "\365\365|Menue", 9ul, (char *) &on, 1u/1u, 1311UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   menu->notoverdraw = 1;
} /* end symbolonoff() */


static void symbolclick(char on, unsigned char v)
{
   pMENU menu;
   newmenu(&menu, 100UL, aprsdecode_lums.fontysize+3UL, 20UL, useri_bTRANSP);
   symbolonoff(menu, on, v);
} /* end symbolclick() */


static void toggwx(unsigned char set, unsigned char w)
{
   unsigned long n;
   char s[100];
   n = (unsigned long)w+1UL;
   useri_confstr(set, s, 100ul);
   if (s[0U]!='0') s[0U] = 0;
   while (aprsstr_Length(s, 100ul)<n) aprsstr_Append(s, 100ul, "0", 2ul);
   if (s[n]==0) aprsstr_Append(s, 100ul, "1", 2ul);
   else if (s[n]=='1') s[n] = '0';
   else s[n] = '1';
   icfg(set, s, 100ul);
} /* end toggwx() */


static char isinwatch(const char s[], unsigned long s_len)
{
   char h[31];
   unsigned long i;
   i = 0UL;
   for (;;) {
      useri_confstrings(useri_fWATCH, i, 0, h, 31ul);
      if (h[0U]==0) return 0;
      if (aprsstr_StrCmp(h, 31ul, s, s_len)) return 1;
      ++i;
   }
   return 0;
} /* end isinwatch() */


static char testdelwaypoint(void)
/* check if delete a waypoint or whole op */
{
   aprsdecode_pOPHIST op;
   struct aprsdecode_CLICKOBJECT * anonym;
   op = aprsdecode_selop();
   if (op) {
      { /* with */
         struct aprsdecode_CLICKOBJECT * anonym = &aprsdecode_click.table[aprsdecode_click.selected]
                ;
         if ((anonym->pff0 && anonym->typf!=aprsdecode_tOBJECT)
                && anonym->typf!=aprsdecode_tSYMBOL) return 1;
      }
   }
   return 0;
} /* end testdelwaypoint() */

static void infosdo(pMENU);


static void infosdo(pMENU menu)
{
   aprsdecode_pOPHIST op;
   char h[101];
   char s[101];
   unsigned short what;
   char test;
   struct aprstat_LASTVAL lastval;
   maptool_pIMAGE img1;
   float km;
   unsigned long olds;
   unsigned long oldk;
   unsigned long rejs;
   unsigned long msgs;
   unsigned long acks;
   unsigned long beacons;
   char tmp;
   op = aprsdecode_selop();
   oldk = menu->oldknob;
   olds = menu->oldsub;
   menu->oldknob = 0UL;
   if (aprsdecode_lums.errorstep) strncpy(s,"\361    |    Errors  |",101u);
   else strncpy(s,"\361    |  Waypoints |",101u);
   addline(menu, s, 101ul, "e", 2ul, 1400UL);
   subicon(menu->image, 13L, midy(menu), 1L, 7L);
   subicon(menu->image, (long)((menu->image->Len1-1)-14UL), midy(menu), 2L,
                7L);
   what = 0U;
   img1 = 0;
   aprstat_wxgraph(&img1, aprsdecode_selop(), aprsdecode_systime, &what,
                &lastval);
   addline(menu, " List Raw |+ Decoded", 21ul, "\246", 2ul, 1405UL);
   if (aprspos_posvalid(op->lastpos)) {
      addline(menu, " Marker 1 |  Marker 2", 22ul, "\251", 2ul, 1407UL);
   }
   test = 1;
   aprstat_kmhist(&img1, op, &test);
   if (test) addline(menu, " Speed Diagram", 15ul, "s", 2ul, 1410UL);
   test = 1;
   aprstat_althist(&img1, op, &test, &km, &beacons, &msgs, &acks, &rejs);
   strncpy(s," ",101u);
   if (km<999.9f) aprsstr_FixToStr(km, 3UL, h, 101ul);
   else aprsstr_FixToStr(km, 0UL, h, 101ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   aprsstr_Append(s, 101ul, "km ", 4ul);
   if (test) aprsstr_Append(s, 101ul, "Altitudes", 10ul);
   else aprsstr_Append(s, 101ul, "Moved", 6ul);
   if (km>0.05f || test) addline(menu, s, 101ul, "n", 2ul, 1415UL);
   strncpy(s," ",101u);
   aprsstr_IntToStr((long)beacons, 1UL, h, 101ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   aprsstr_Append(s, 101ul, " Beacon Times", 14ul);
   addline(menu, s, 101ul, "b", 2ul, 1420UL);
   addline(menu, " Save Info to File", 19ul, "\261", 2ul, 1425UL);
   if (useri_beaconediting) {
      addline(menu, " Clone to Beaconeditor", 23ul, "\332", 2ul, 1427UL);
   }
   if (op && isinwatch(op->call, 9ul)) {
      addline(menu, " Disable in Watchlist", 22ul, "\210", 2ul, 1430UL);
   }
   else addline(menu, " Add to Watchlist", 18ul, "\205", 2ul, 1435UL);
   aprsdecode_objsender(aprsdecode_selop(), h, 101ul);
   if (h[0U]) {
      /*  Assign(click.mhop, h); */
      strncpy(s," Objects of ",101u);
      aprsstr_Append(s, 101ul, h, 101ul);
      addline(menu, s, 101ul, "o", 2ul, 1438UL);
   }
   if (op) {
      strncpy(s," Delete ",101u);
      if (testdelwaypoint()) aprsstr_Append(s, 101ul, "Waypoint", 9ul);
      else aprsstr_Append(s, 101ul, op->call, 9ul);
      addline(menu, s, 101ul, (char *)(tmp = '\177',&tmp), 1u/1u, 1440UL);
   }
   if (km>0.05f) {
      addline(menu, " Change Trackcolour", 20ul, "~", 2ul, 1445UL);
   }
   strncpy(s," Msg ",101u);
   aprsstr_IntToStr((long)msgs, 1UL, h, 101ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   aprsstr_Append(s, 101ul, "/Ack ", 6ul);
   aprsstr_IntToStr((long)acks, 1UL, h, 101ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   aprsstr_Append(s, 101ul, "/Rej ", 6ul);
   aprsstr_IntToStr((long)rejs, 1UL, h, 101ul);
   aprsstr_Append(s, 101ul, h, 101ul);
   if (msgs>0UL) addline(menu, s, 101ul, "\250", 2ul, 1450UL);
   else addline(menu, s, 101ul, "", 1ul, 1450UL);
   if ((0x1U & what)) {
      aprsstr_FixToStr(lastval.temp, 2UL, s, 101ul);
      strncpy(h,"Temp       |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "\177C", 3ul);
      addline(menu, h, 101ul, "w", 2ul, 1455UL);
   }
   if ((0x2U & what)) {
      aprsstr_FixToStr(lastval.baro, 2UL, s, 101ul);
      strncpy(h,"Barometer  |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "hPa", 4ul);
      addline(menu, h, 101ul, "p", 2ul, 1458UL);
   }
   if ((0x4U & what)) {
      aprsstr_FixToStr(lastval.hyg, 0UL, s, 101ul);
      strncpy(h,"Hygrometer |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "%", 2ul);
      addline(menu, h, 101ul, "h", 2ul, 1460UL);
   }
   if ((0x8U & what)) {
      aprsstr_FixToStr(lastval.winds, 0UL, s, 101ul);
      strncpy(h,"Wind       |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "km/h", 5ul);
      addline(menu, h, 101ul, "j", 2ul, 1462UL);
   }
   if ((0x10U & what)) {
      aprsstr_FixToStr(lastval.winddir, 0UL, s, 101ul);
      strncpy(h,"Wind Dir   |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "deg", 4ul);
      addline(menu, h, 101ul, "d", 2ul, 1464UL);
   }
   if ((0x20U & what)) {
      aprsstr_FixToStr(lastval.rain, 2UL, s, 101ul);
      strncpy(h,"Rain       |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "mm", 3ul);
      addline(menu, h, 101ul, "r", 2ul, 1466UL);
      aprsstr_FixToStr(lastval.rain24, 2UL, s, 101ul);
      strncpy(h,"Rain in 24h|",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "mm", 3ul);
      addline(menu, h, 101ul, "r", 2ul, 1466UL);
   }
   if ((0x40U & what)) {
      aprsstr_FixToStr(lastval.lumi, 0UL, s, 101ul);
      strncpy(h,"Luminosity |",101u);
      aprsstr_Append(h, 101ul, s, 101ul);
      aprsstr_Append(h, 101ul, "W/m2", 5ul);
      addline(menu, h, 101ul, "l", 2ul, 1468UL);
   }
   if ((0x400U & what)) {
      strncpy(h,"Radiation  |",101u);
      aprstext_sievert2str(lastval.siev, s, 101ul);
      aprsstr_Append(h, 101ul, s, 101ul);
      addline(menu, h, 101ul, "g", 2ul, 1470UL);
   }
   menu->redrawproc = infosdo;
   menu->hiknob = 0UL;
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = oldk;
   menu->oldsub = olds;
   menu->notoverdraw = 1;
} /* end infosdo() */


static void infos(void)
{
   pMENU menu;
   newmenu(&menu, 125UL, aprsdecode_lums.fontysize+5UL, 23UL, useri_bTRANSP);
   infosdo(menu);
   /*  menu^.ysize:=menu^.oldknob*menu^.yknob; */
   menu->oldknob = 0UL;
} /* end infos() */

#define useri_MAXGAMMA 10

#define useri_MINBRIGHT 3600

#define useri_MAXFADE 36000

#define useri_MAXFPS 5000


static void perc(unsigned long * vd, char h[100], char s[100],
                unsigned long vw, unsigned long potx, unsigned char cfg,
                unsigned long def, long * lum)
{
   unsigned long v;
   v = (unsigned long)useri_conf2int(cfg, 0UL, 0L, 100L, (long)def);
   if (potx>0UL) {
      v = (vw*100UL)/1024UL;
      *lum = (long)vw;
   }
   aprsstr_Assign(s, 100ul, configs[cfg].title, 31ul);
   useri_int2cfg(cfg, (long)v);
   aprsstr_IntToStr((long)v, 0UL, h, 100ul);
   aprsstr_Append(h, 100ul, "%", 2ul);
   *vd = (v*220UL)/100UL;
} /* end perc() */


static void potimove(pMENU m, unsigned long potx, unsigned long knob)
{
   char h[100];
   char s[100];
   unsigned long tx1;
   unsigned long vd;
   unsigned long vw;
   unsigned long vp;
   unsigned long v;
   unsigned long maxbright;
   float gam;
   aprsdecode_lums.moving = 0;
   if (potx<8UL) vp = 0UL;
   else vp = potx-8UL;
   if (vp>220UL) vp = 220UL;
   tx1 = 180UL;
   vw = (vp*1024UL)/220UL;
   vd = 0UL;
   if (knob==1UL) {
      perc(&vd, h, s, vw, potx, useri_fLOBJ, 70UL, &aprsdecode_lums.obj);
   }
   else if (knob==2UL) {
      perc(&vd, h, s, vw, potx, useri_fLMAP, 30UL, &aprsdecode_lums.map);
   }
   else if (knob==3UL) {
      perc(&vd, h, s, vw, potx, useri_fLTRACK, 80UL, &aprsdecode_lums.track);
   }
   else if (knob==4UL) {
      gam = aprsdecode_lums.gamma;
      if (potx>0UL) {
         gam = X2C_DIVR((float)vp*10.0f,220.0f);
         if (gam<0.2f) gam = 0.2f;
         aprsdecode_lums.gamma = gam;
      }
      xosi_Gammatab(aprsdecode_lums.gamma);
      strncpy(s,"Gamma",100u);
      aprsstr_FixToStr(gam+0.05f, 2UL, h, 100ul);
      icfg(useri_fGAMMA, h, 100ul);
      vd = aprsdecode_trunc(X2C_DIVR(gam*220.0f,10.0f));
   }
   else if (knob==5UL) {
      perc(&vd, h, s, vw, potx, useri_fLSYM, 90UL, &aprsdecode_lums.sym);
   }
   else if (knob==6UL) {
      perc(&vd, h, s, vw, potx, useri_fLTEXT, 80UL, &aprsdecode_lums.text);
   }
   else if (knob==7UL) {
      perc(&vd, h, s, vw, potx, useri_fLWAY, 70UL,
                &aprsdecode_lums.waypoint);
   }
   else if (knob==8UL) {
      maxbright = aprsdecode_lums.purgetime;
      if (maxbright<3600UL) maxbright = 3600UL;
      v = aprsdecode_lums.firstdim;
      if (potx>0UL) {
         v = (vp*maxbright)/220UL;
         aprsdecode_lums.firstdim = v;
      }
      strncpy(s,"Time full Bright",100u);
      useri_int2cfg(useri_fTFULL, (long)(v/60UL));
      aprsstr_IntToStr((long)(v/3600UL), 0UL, h, 100ul);
      aprsstr_Append(h, 100ul, "h", 2ul);
      vd = (v*220UL)/maxbright;
   }
   else if (knob==9UL) {
      v = aprsdecode_lums.maxdim;
      if (potx>0UL) {
         v = (vp*36000UL)/220UL;
         aprsdecode_lums.maxdim = v;
      }
      strncpy(s,"Time Fade Out",100u);
      aprsstr_IntToStr((long)(v/60UL), 0UL, h, 100ul);
      icfg(useri_fTFADE, h, 100ul);
      aprsstr_Append(h, 100ul, "Min", 4ul);
      vd = (v*220UL)/36000UL;
   }
   else if (knob==10UL) {
      /*
          v:=lums.actfps;
          IF potx>0 THEN v:=vp*MAXFPS DIV POTWIDTH; lums.actfps:=v  END;
          IF lums.actfps=0 THEN lums.actfps:=1 END;
          s:="s/Frame";
          IntToStr(v, 0, h);
          vd:=v*POTWIDTH DIV MAXFPS;
      */
      v = (unsigned long)aprsdecode_lums.actfps;
      if (potx>0UL) v = (vp*vp*5000UL)/48400UL;
      if (v==0UL) v = 1UL;
      aprsdecode_lums.actfps = (long)v;
      strncpy(s,"s/Frame",100u);
      aprsstr_IntToStr((long)v, 0UL, h, 100ul);
      vd = (unsigned long)X2C_TRUNCC(RealMath_sqrt((float)((v*220UL*220UL)
                /5000UL)),0UL,X2C_max_longcard);
      tx1 = 90UL;
      knob = 1UL;
   }
   potival(m, knob, vd, 11UL, tx1, s, 100ul, h, 100ul);
   m->notoverdraw = 1;
} /* end potimove() */


static void potisonoff(pMENU menu)
{
   unsigned long i;
   menu->oldknob = 0UL;
   for (i = 1UL; i<=9UL; i++) {
      addline(menu, "", 1ul, "!", 2ul, 1500UL+i);
      potimove(menu, 0UL, i);
   } /* end for */
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end potisonoff() */


static void potis(void)
{
   pMENU menu;
   newmenu(&menu, 236UL, aprsdecode_lums.fontysize+9UL, 10UL, useri_bCOLOR);
   potisonoff(menu);
} /* end potis() */


static void abortpoponoff(pMENU menu)
{
   menu->oldknob = 0UL;
   addline(menu, "", 1ul, "\006", 2ul, 1900UL);
   potimove(menu, 0UL, 10UL);
   addline(menu, "Abort", 6ul, "\003", 2ul, 1905UL);
   addline(menu, "Continue", 9ul, "A", 2ul, 1910UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end abortpoponoff() */


static void abortpop(void)
{
   pMENU menu;
   useri_killallmenus();
   newmenu(&menu, 240UL, aprsdecode_lums.fontysize+7UL, 4UL, useri_bTRANSP);
   abortpoponoff(menu);
} /* end abortpop() */

static char useri_noprop = 1;


static void linewr(pMENU m, unsigned long knob, unsigned long x0,
                unsigned long curs, unsigned long width, char curson,
                char s[], unsigned long s_len, char s1[],
                unsigned long s1_len)
{
   unsigned long ic;
   unsigned long iw;
   unsigned long i;
   unsigned long y1;
   unsigned long x1;
   unsigned long y;
   unsigned long x;
   long inc;
   struct aprsdecode_COLTYP col;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   maptool_pIMAGE anonym2;
   maptool_pIMAGE anonym3;
   maptool_pIMAGE anonym4;
   unsigned long tmp;
   unsigned long tmp0;
   X2C_PCOPY((void **)&s,s_len);
   X2C_PCOPY((void **)&s1,s1_len);
   if (knob==0UL) goto label;
   tmp = m->yknob-3UL;
   y = 2UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = m->xsize-4UL;
      x = x0;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = (anonym2 = m->image,
                &anonym2->Adr[(x)*anonym2->Len0+(y+(knob-1UL)*m->yknob)]);
            anonym->r = 200U;
            anonym->g = 211U;
            anonym->b = 200U;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   /*  Colset(col, "W"); (*"Y"*) */
   col.r = 256UL;
   col.g = 201UL;
   col.b = 256UL;
   /*title */
   x = x0;
   y = (knob-1UL)*m->yknob+knobtexty(m->yknob);
   tmp = aprsstr_Length(s, s_len);
   i = 1UL;
   if (i<=tmp) for (;; i++) {
      maptool_drawchar(m->image, s[i-1UL], (float)x, (float)y, &inc, 1000UL,
                1UL, col, 0);
      inc = 6L;
      x += (unsigned long)inc;
      if (i==tmp) break;
   } /* end for */
   x1 = x+6UL;
   /*title */
   /*text */
   if (width>0UL) {
      i = m->xsize-4UL;
      /*  IF width>0 THEN */
      iw = x1+width*6UL+2UL;
      if (iw<i) i = iw;
      /*  END; */
      tmp = m->yknob-3UL;
      y1 = 2UL;
      if (y1<=tmp) for (;; y1++) {
         tmp0 = i;
         x = x1;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct maptool_PIX * anonym0 = (anonym3 = m->image,
                &anonym3->Adr[(x)*anonym3->Len0+(y1+(knob-1UL)*m->yknob)]);
               anonym0->r = 100U;
               anonym0->g = 111U;
               anonym0->b = 100U;
            }
            if (x==tmp0) break;
         } /* end for */
         if (y1==tmp) break;
      } /* end for */
      x = (curs+2UL)*6UL+x1+4UL;
      if (x>m->xsize) i = (x-m->xsize)/6UL;
      else i = 0UL;
      x = x1;
      for (;;) {
         inc = 6L;
         if (i<aprsstr_Length(s1, s1_len)) {
            if (x<(m->xsize-4UL)-6UL) {
               maptool_drawchar(m->image, s1[i], (float)x, (float)y, &inc,
                1000UL, 1UL, col, 0);
               inc = 6L;
            }
         }
         if (curson && i==curs) {
            if (i<curs) x += (curs-i)*6UL;
            do {
               if (x+4UL<m->xsize) {
                  for (ic = 0UL; ic<=1UL; ic++) {
                     { /* with */
                        struct maptool_PIX * anonym1 = (anonym4 = m->image,
                &anonym4->Adr[(x+1UL)*anonym4->Len0+(y+ic)]);
                        if (overtype) {
                           anonym1->r = 1000U;
                           anonym1->g = 0U;
                           anonym1->b = 0U;
                        }
                        else {
                           anonym1->r = 1000U;
                           anonym1->g = 999U;
                           anonym1->b = 1000U;
                        }
                     }
                  } /* end for */
               }
               ++x;
               --inc;
            } while (inc>0L);
         }
         else {
            x += (unsigned long)inc;
         }
         if (i>=s1_len-1) break;
         ++i;
      }
   }
   label:;
   X2C_PFREE(s);
   X2C_PFREE(s1);
/*text */
} /* end linewr() */


static void keybknob(pMENU m, unsigned long knob, char withcurs,
                unsigned long posx)
{
   unsigned char idx;
   unsigned long l;
   unsigned long x;
   char s[1000];
   struct CONFIG * anonym;
   idx = m->confidx[knob];
   if (idx<=useri_fEDITLINE && X2C_IN(knob,31,m->clampkb)) {
      { /* with */
         struct CONFIG * anonym = &configs[idx];
         if (isbool(anonym->typ)) x = 17UL;
         else x = 4UL;
         if (posx>x) {
            posx = (posx-x)/6UL;
            l = aprsstr_Length(anonym->title, 31ul)+1UL;
            if (clampedline==knob && posx>=l) {
               /*        IF posx>=l THEN  */
               anonym->curspos = (unsigned short)(posx-l);
               useri_refresh = 1;
            }
         }
         useri_confstr(idx, s, 1000ul);
         linewr(m, knob, x, (unsigned long)anonym->curspos,
                (unsigned long)anonym->width, withcurs, anonym->title, 31ul,
                s, 1000ul);
      }
      if (posx) clampedline = knob;
   }
} /* end keybknob() */


static void setmenupos(pMENU m, unsigned long defx, unsigned long defy)
{
   unsigned long yc;
   unsigned long xc;
   if (m && (unsigned long)m->pullconf>0UL) {
      xc = (unsigned long)useri_conf2int(m->pullconf, 0UL, 0L, 32000L, 0L);
      yc = (unsigned long)useri_conf2int(m->pullconf, 1UL, 0L, 32000L, 0L);
      if (xc==0UL && yc==0UL) {
         /* not pulled set default */
         m->x0 = defx;
         m->y00 = defy;
      }
      else {
         /* set pulled pos and save */
         /*      IF xc+m^.xsize>VAL(CARDINAL, xsize) THEN xc:=MAXXSIZE END;
                   (* connect to window frame *) */
         /*      IF yc+m^.ysize>VAL(CARDINAL, ysize) THEN yc:=MAXYSIZE END;
                   (* connect to window frame *) */
         if (xc+m->xsize>(unsigned long)maptool_xsize) {
            xc = (unsigned long)maptool_xsize-m->xsize;
         }
         if (yc+m->ysize>(unsigned long)maptool_ysize) {
            yc = (unsigned long)maptool_ysize-m->ysize;
         }
         m->x0 = xc;
         m->y00 = yc;
      }
   }
/*      saveXYtocfg(m^.pullconf, xc, yc); */
} /* end setmenupos() */

#define useri_W 1024


static void invline(maptool_pIMAGE im, unsigned long x0, unsigned long y00,
                unsigned long x1, unsigned long y1, char allcolor)
{
   unsigned long y;
   unsigned long x;
   struct maptool_PIX * anonym;
   /*
             IF allcolor OR ((r=g) & (g=b)) THEN
             IF allcolor OR NOT ODD(g) THEN
               w:=g;
               IF w>W THEN w:=0 ELSE w:=W-w END;
               r:=w; g:=w; b:=w;
             END;
   */
   unsigned long tmp;
   unsigned long tmp0;
   /*WrInt(x1, 10); WrInt(HIGH(im^), 10); WrInt(y1, 10);
                WrInt(HIGH(im^[0]), 10); WrLn; */
   if (y1<=im->Len0-1 && x1<=(im->Len1-1)+1UL) {
      tmp = y1;
      y = y00;
      if (y<=tmp) for (;; y++) {
         tmp0 = x1-1UL;
         x = x0+2UL;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct maptool_PIX * anonym = &im->Adr[(x)*im->Len0+y];
               if (allcolor || !(anonym->g&1)) {
                  if (anonym->r>1024U) anonym->r = 1024U;
                  if (anonym->g>1024U) anonym->g = 1024U;
                  if (anonym->b>1024U) anonym->b = 1024U;
                  anonym->r = 1024U-anonym->r;
                  anonym->g = 1024U-anonym->g;
                  anonym->b = 1024U-anonym->b;
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
   }
} /* end invline() */

#define useri_W0 1024


static void inv(pMENU m, unsigned long sub, unsigned long k)
{
   unsigned long y1;
   unsigned long x1;
   unsigned long y00;
   unsigned long x0;
   if (!X2C_IN(k-1UL,31,m->nohilite)) {
      if (X2C_IN(k,31,m->clampkb)) {
         x0 = (unsigned long)m->subk[k-1UL][1U];
         x1 = m->xsize;
         if (sub==0UL && x0>0UL) {
            x1 = x0;
            x0 = 0UL;
         }
      }
      else if (sub==0UL) {
         x0 = 0UL;
         x1 = (unsigned long)m->subk[k-1UL][0U];
      }
      else {
         x0 = (unsigned long)m->subk[k-1UL][sub-1UL];
         x1 = (unsigned long)m->subk[k-1UL][sub];
      }
      /*    IF k IN m^.noprop THEN x0:=0; x1:=0 END; */
      if (x1==0UL) x1 = m->xsize;
      y00 = (k-1UL)*m->yknob+1UL;
      y1 = k*m->yknob-2UL;
      /*WrInt(sub,3);WrInt(k,3);WrInt(y0,5);WrInt(y1,5);WrInt(x0,5);
                WrInt(x1,5);WrStrLn("y-y,x-x"); */
      invline(m->image, x0, y00, x1, y1, 0);
   }
} /* end inv() */

/*
PROCEDURE drawpullknob(image:pIMAGE; xs, y0, y1:INTEGER);
VAR x, y:INTEGER;
    col, col1, col2:PIX;
BEGIN
  col.r:=200; col.g:=200; col.b:=200;
  col1.r:=200; col1.g:=300; col1.b:=700;
  col2.r:=500; col2.g:=500; col2.b:=500;

  FOR y:=y0 TO y1 DO
    FOR x:=0 TO xs-2 DO
      IF (y=y1) OR (x=0) THEN image^[x][y]:=col2 
      ELSIF ODD(x DIV 2+y DIV 2) THEN image^[x][y]:=col ELSE image^[x][y]:=col1 END;
    END;
  END;
END drawpullknob;
*/

static void movewin(pMENU pm, long dx, long dy)
{
   unsigned long yc;
   unsigned long xc;
   if ((unsigned long)pm->pullconf>0UL) {
      xc = (unsigned long)useri_conf2int(pm->pullconf, 0UL, 0L, 32000L, 0L);
      yc = (unsigned long)useri_conf2int(pm->pullconf, 1UL, 0L, 32000L, 0L);
      xc = (unsigned long)inclim((long)xc, dx, 0L);
      yc = (unsigned long)inclim((long)yc, dy, 0L);
      if (xc+pm->xsize>=(unsigned long)maptool_xsize) xc = 32000UL;
      if (yc+pm->ysize>=(unsigned long)maptool_ysize) yc = 32000UL;
      useri_saveXYtocfg(pm->pullconf, (long)xc, (long)yc);
   }
} /* end movewin() */

#define useri_SYMS 17

#define useri_YS 12

#define useri_S "\370\367|"


static void symchoose(char myorbeacon)
/* draw symbol chart to select one */
{
   pMENU menu;
   unsigned long y;
   unsigned long x;
   char t;
   struct maptool_PIX * anonym;
   maptool_pIMAGE anonym0;
   unsigned long tmp;
   unsigned long tmp0;
   newmenu(&menu, 275UL, 17UL, 12UL, useri_bCOLOR);
   for (y = 0UL; y<=11UL; y++) {
      addline(menu, "\370\367|\370\367|\370\367|\370\367|\370\367|\370\367|\37\
0\367|\370\367|\370\367|\370\367|\370\367|\370\367|\370\367|\370\367|\370\367\
|\370\367|", 49ul, (char *) &myorbeacon, 1u/1u, 2100UL+16UL*(11UL-y));
   } /* end for */
   tmp = menu->image->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      if (y%17UL) {
         tmp0 = menu->image->Len1-1;
         x = 0UL;
         if (x<=tmp0) for (;; x++) {
            if (x%17UL!=1UL) {
               { /* with */
                  struct maptool_PIX * anonym = (anonym0 = menu->image,
                &anonym0->Adr[(x)*anonym0->Len0+y]);
                  anonym->r = 200U;
                  anonym->g = 200U;
                  anonym->b = 200U;
               }
            }
            if (x==tmp0) break;
         } /* end for */
      }
      if (y==tmp) break;
   } /* end for */
   for (y = 0UL; y<=11UL; y++) {
      if (y<6UL) t = '/';
      else t = '\\';
      for (x = 0UL; x<=15UL; x++) {
         maptool_drawsym(menu->image, t, (char)(32UL+x+16UL*(y%6UL)), 0,
                (float)(x*17UL+10UL), (float)((11UL-y)*17UL+9UL), 250UL);
      } /* end for */
   } /* end for */
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end symchoose() */

#define useri_SYMS0 17

#define useri_S0 "\370\367|"


static void overlaychoose(char myorbeacon)
{
   pMENU menu;
   newmenu(&menu, 156UL, aprsdecode_lums.fontysize+5UL, 3UL, useri_bCOLOR);
   addline(menu, "0 |1 |2 |3 |4 |5 |6 |7 |8 |9 |", 31ul,
                (char *) &myorbeacon, 1u/1u, 4100UL);
   addline(menu, "N |O |P |Q |R |S |T |U |V |W |X |Y |Z |", 40ul,
                (char *) &myorbeacon, 1u/1u, 2140UL);
   addline(menu, "A |B |C |D |E |F |G |H |I |J |K |L |M |", 40ul,
                (char *) &myorbeacon, 1u/1u, 2140UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   /*  setmenupos(menu, xmouse.x, xmouse.y); */
   menu->x0 = (unsigned long)useri_xmouse.x;
   if (useri_xmouse.y>8L) menu->y00 = (unsigned long)(useri_xmouse.y-8L);
   else menu->y00 = 0UL;
   menu->wid = 203UL;
} /* end overlaychoose() */


static void AppBlueButton(char s[], unsigned long s_len, char text[],
                unsigned long text_len, char on)
{
   X2C_PCOPY((void **)&text,text_len);
   if (on) aprsstr_Append(s, s_len, "\350", 2ul);
   aprsstr_Append(s, s_len, text, text_len);
   X2C_PFREE(text);
} /* end AppBlueButton() */


static void AddEditLine(pMENU m, char cmd[], unsigned long cmd_len,
                const char text[], unsigned long text_len,
                unsigned char conf, unsigned long help0)
{
   X2C_PCOPY((void **)&cmd,cmd_len);
   ++m->scroll;
   if (text[0UL]==0) {
      addline(m, "", 1ul, cmd, cmd_len, help0);
      m->clampkb |= (1UL<<m->scroll);
      m->confidx[m->scroll] = conf;
      keybknob(m, m->scroll, 0, 0UL);
   }
   else {
      addline(m, text, text_len, " ", 2ul, help0);
      m->clampkb &= ~(1UL<<m->scroll);
      m->confidx[m->scroll] = 0U;
   }
   X2C_PFREE(cmd);
} /* end AddEditLine() */


static void managemsg(unsigned long scroll, unsigned long num)
{
   pMENU menu;
   newmenu(&menu, 50UL, aprsdecode_lums.fontysize+5UL, 6UL, useri_bCOLOR);
   menu->scroll = scroll-num;
   addline(menu, "Edit", 5ul, "\260", 2ul, 6900UL);
   addline(menu, "Set Port", 9ul, "\260", 2ul, 6901UL);
   addline(menu, "Hold", 5ul, "\260", 2ul, 6902UL);
   addline(menu, "Restart", 8ul, "\260", 2ul, 6903UL);
   addline(menu, "Kick", 5ul, "\260", 2ul, 6904UL);
   addline(menu, "Delete", 7ul, "\260", 2ul, 6905UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end managemsg() */


static void setmsgport(unsigned long p)
{
   char c;
   if (p==0UL) c = 'A';
   else if (p==1UL) c = 'N';
   else c = (char)((p-1UL)+48UL);
   icfg(useri_fMSGPORT, (char *) &c, 1u/1u);
} /* end setmsgport() */

#define useri_YKNOB 20

#define useri_XSIZE 462

#define useri_YPOS 200

#define useri_MLINES 6
/* static menu lines */

#define useri_DAY 86400

#define useri_NOTIME "--:--:--"

#define useri_NOACKTIME "        "

#define useri_HOLD "  HOLD  "

#define useri_REJECTED "REJECTED"

#define useri_XCHAR 77


static void sendmsg(void)
{
   pMENU m;
   unsigned long i;
   unsigned long cnt;
   unsigned long cntm;
   unsigned long xw;
   aprsdecode_pTXMESSAGE pmh;
   aprsdecode_pTXMESSAGE pm;
   char h[201];
   char s[201];
   char po;
   unsigned long ohs;
   unsigned long hks;
   unsigned long oks;
   char tmp;
   cntm = 0UL;
   pm = aprsdecode_txmessages;
   while (pm) {
      ++cntm;
      pm = pm->next;
   }
   cnt = cntm;
   i = useri_mainys()/20UL;
   if (i<7UL) i = 1UL;
   else i -= 6UL;
   if (cnt>i) cnt = i;
   xw = 462UL;
   if (462UL>(unsigned long)maptool_xsize) xw = (unsigned long)maptool_xsize;
   m = findmenuid(228UL);
   refrmenu(&m, xw, aprsdecode_lums.fontysize+7UL, cnt+6UL, useri_bCOLOR, 0);
   oks = m->oldknob;
   hks = m->hiknob;
   ohs = m->oldsub;
   m->oldknob = 0UL;
   m->scroll = 0UL;
   /*
       h:=MOP3+"       Send with Ack     |        Send/Query       |        Close"
                ;
       confstr(fMSGTEXT, s);
       IF s[0]<>0C THEN
         confstr(fMSGTO, s);
         IF s[0]<>0C THEN h:="       Send with Ack     |        Send/Query       |        Close" END;
       END;
   */
   addline(m, "       Send with Ack     |        Send/Query       |        Cl\
ose", 66ul, "\255", 2ul, 6800UL);
   ++m->scroll;
   addline(m, "", 1ul, "\255", 2ul, 6810UL);
   ++m->scroll;
   m->clampkb |= 0x4UL;
   m->confidx[2U] = 130U;
   keybknob(m, 2UL, 0, 0UL);
   addline(m, "", 1ul, "\255", 2ul, 6815UL);
   ++m->scroll;
   m->clampkb |= 0x8UL;
   m->confidx[3U] = 129U;
   keybknob(m, 3UL, 0, 0UL);
   useri_confstr(useri_fMSGPORT, s, 201ul);
   po = X2C_CAP(s[0U]);
   strncpy(s,"\355",201u);
   AppBlueButton(s, 201ul, "      Auto  \363|", 15ul, po==' ' || po=='A');
   AppBlueButton(s, 201ul, "      Net   |", 14ul, po=='N' || po=='I');
   for (i = 1UL; i<=4UL; i++) {
      strncpy(h,"    RfPort ",201u);
      aprsstr_Append(h, 201ul, (char *)(tmp = (char)(i+48UL),&tmp), 1u/1u);
      if (i!=4UL) aprsstr_Append(h, 201ul, " |", 3ul);
      AppBlueButton(s, 201ul, h, 201ul, po==(char)(i+48UL));
   } /* end for */
   addline(m, s, 201ul, "\255", 2ul, 6820UL);
   ++m->scroll;
   m->scroll = (cntm-cnt)+4UL;
   m->oldknob = 4UL;
   while (cnt>0UL) {
      pm = aprsdecode_txmessages;
      i = cntm;
      while (i>1UL && pm) {
         pm = pm->next;
         --i;
      }
      if (pm) {
         strncpy(s,"\346\357",201u);
         if ((pm->rej || pm->acknum==0UL && pm->txcnt>0UL)
                || pm->acktime>0UL) aprsstr_Append(s, 201ul, "\356", 2ul);
         if (pm->txtime==X2C_max_longcard) strncpy(h,"  HOLD  ",201u);
         else if (pm->txtime>0UL) {
            aprsstr_TimeToStr(pm->txtime%86400UL, h, 201ul);
         }
         else strncpy(h,"--:--:--",201u);
         aprsstr_Append(s, 201ul, h, 201ul);
         aprsstr_Append(s, 201ul, " |", 3ul);
         if (pm->rej) strncpy(h,"REJECTED",201u);
         else if (pm->acknum==0UL) strncpy(h,"        ",201u);
         else if (pm->acktime>0UL) {
            aprsstr_TimeToStr(pm->acktime%86400UL, h, 201ul);
         }
         else {
            pmh = pm->next;
            while (pmh && !aprsstr_StrCmp(pmh->to, 9ul, pm->to, 9ul)) {
               pmh = pmh->next;
            }
            if (pmh) strncpy(h,"\350--:--:--",201u);
            else strncpy(h,"--:--:--",201u);
         }
         aprsstr_Append(s, 201ul, h, 201ul);
         aprsstr_Append(s, 201ul, " |", 3ul);
         aprsstr_IntToStr((long)pm->txcnt, 3UL, h, 201ul);
         aprsstr_Append(s, 201ul, h, 201ul);
         aprsstr_Append(s, 201ul, " |\361", 4ul);
         aprsstr_Append(s, 201ul, (char *) &pm->port, 1u/1u);
         aprsstr_Append(s, 201ul, "\363 |", 4ul);
         aprsstr_Assign(h, 201ul, pm->to, 9ul);
         aprsstr_Append(h, 201ul, "          ", 11ul);
         h[9U] = 0;
         aprsstr_Append(s, 201ul, h, 201ul);
         aprsstr_Append(s, 201ul, " |", 3ul);
         if (pm->acknum>0UL) {
            aprsdecode_acknumstr(h, 201ul, pm->acknum);
            aprsstr_Append(s, 201ul, h, 201ul);
            aprsdecode_getactack(pm->to, h, 201ul);
            if (h[0U]) {
               aprsstr_Append(s, 201ul, "}", 2ul);
               aprsstr_Append(s, 201ul, h, 201ul);
            }
            else aprsstr_Append(s, 201ul, "   ", 4ul);
         }
         else aprsstr_Append(s, 201ul, "-----", 6ul);
         aprsstr_Append(s, 201ul, "  |", 4ul);
         aprsstr_Append(s, 201ul, pm->msgtext, 67ul);
         if (aprsstr_Length(s, 201ul)>77UL) {
            s[77U] = 0;
            aprsstr_Append(s, 201ul, "...", 4ul);
         }
         addline(m, s, 201ul, "\257>", 3ul, 6804UL);
         ++m->scroll;
      }
      --cntm;
      --cnt;
   }
   if (aprsdecode_txmessages) {
      addline(m, "\346\347\351\352 TxTime  |AckTime  |Ret |Po |Receiver  |Ack\
Txt |Message", 60ul, " ", 2ul, 6805UL);
   }
   m->hiknob = hks;
   m->wid = 228UL;
   m->ysize = m->oldknob*m->yknob;
   m->oldknob = oks;
   /*    IF xw<VAL(CARDINAL,xsize) THEN m^.x0:=(VAL(CARDINAL,
                xsize)-xw) DIV 2 ELSE m^.x0:=0 END; */
   /*    m^.y0:=200; */
   m->pullconf = useri_fMENUXYMSG;
   m->pullyknob = 8UL;
   if (xw<(unsigned long)maptool_xsize) {
      i = ((unsigned long)maptool_xsize-xw)/2UL;
   }
   else i = 0UL;
   setmenupos(m, i, 200UL);
   m->oldsub = ohs;
   if (m->hiknob>0UL) {
      if (X2C_IN(m->hiknob,31,m->clampkb)) keybknob(m, m->hiknob, 1, 0UL);
      inv(m, m->oldsub, m->hiknob);
   }
/*  END; */
} /* end sendmsg() */


static void linenoton(pCONFLINE pl)
{
   if (pl) pl->active = pl->active==0U;
} /* end linenoton() */


static void configdelman(unsigned char cfg, unsigned long num,
                unsigned long n)
/* 1 on/off, 2 move to editline, 3 del, first n is 1 */
{
   pCONFLINE po;
   pCONFLINE pl;
   pl = 0;
   if ((unsigned long)cfg<=152UL) pl = configs[cfg].lines;
   po = 0;
   while (n>1UL && pl) {
      po = pl;
      pl = pl->next;
      --n;
   }
   if (pl) {
      if (num==1UL) linenoton(pl);
      else if (num==2UL || num==3UL) {
         if (po) po->next = pl->next;
         else configs[cfg].lines = pl->next;
         if (num==2UL) {
            icfg(useri_fEDITLINE, pl->line, 201ul);
                /* save deleted to editline */
         }
         Storage_DEALLOCATE((X2C_ADDRESS *) &pl, sizeof(struct CONFLINE));
      }
   }
   useri_refresh = 1;
} /* end configdelman() */

/* === beacons === */

static void DelBeaconByName(char dname[], unsigned long dname_len)
{
   aprsdecode_MONCALL name;
   unsigned long cnt;
   char symb[3];
   char isbad;
   char isdel;
   char isobj;
   char s[201];
   X2C_PCOPY((void **)&dname,dname_len);
   cnt = 0UL;
   for (;;) {
      useri_confstrings(useri_fRBTEXT, cnt, 1, s, 201ul);
      if (s[0U]==0) break;
      aprsdecode_getbeaconname(s, 201ul, name, 9ul, symb, 3ul, &isobj,
                &isdel, &isbad);
      ++cnt;
      if (aprsstr_StrCmp(name, 9ul, dname, dname_len)) {
         configdelman(useri_fRBTEXT, 3UL, cnt);
         cnt = 0UL;
      }
   }
   X2C_PFREE(dname);
} /* end DelBeaconByName() */

#define useri_XSIZE0 300
/*      YPOS=140; */

#define useri_MLINES0 15
/* static menu lines */


static void beaconeditor(void)
{
   pMENU m;
   unsigned long yp;
   unsigned long xp;
   unsigned long i;
   unsigned long cntm;
   unsigned long cnt;
   unsigned long xw;
   char h[201];
   char s[201];
   char bkn;
   char port;
   char postyp;
   char typ;
   aprsdecode_MONCALL ename;
   aprsdecode_MONCALL name;
   char symb[3];
   char replace;
   char isbad;
   char isdel;
   char isobj;
   struct aprspos_POSITION mypos;
   unsigned long ohs;
   unsigned long hks;
   unsigned long oks;
   char tmp;
   useri_confstr(useri_fRBNAME, ename, 9ul);
   replace = 0;
   cntm = 0UL;
   for (;;) {
      useri_confstrings(useri_fRBTEXT, cntm, 1, s, 201ul);
                /* beacon raw text */
      if (s[0U]==0) break;
      aprsdecode_getbeaconname(s, 201ul, name, 9ul, symb, 3ul, &isobj,
                &isdel, &isbad);
      if (isobj && aprsstr_StrCmp(name, 9ul, ename, 9ul)) replace = 1;
      ++cntm;
   }
   cnt = cntm;
   i = useri_mainys()/(aprsdecode_lums.fontysize+5UL);
   if (i<16UL) i = 1UL;
   else if (useri_beaconed) i -= 15UL;
   if (cnt>i) cnt = i;
   xw = 300UL;
   if (300UL>(unsigned long)maptool_xsize) xw = (unsigned long)maptool_xsize;
   m = findmenuid(226UL);
   /*IF m=NIL THEN beaconed:=TRUE END; */
   refrmenu(&m, xw, aprsdecode_lums.fontysize+5UL, cnt+15UL+1UL,
                useri_bCOLOR, 0); /* +1 for pullknob */
   oks = m->oldknob;
   hks = m->hiknob;
   ohs = m->oldsub;
   if (m->image==0) osi_WrStrLn("menu img nil", 13ul);
   m->oldknob = 0UL;
   m->scroll = 1UL;
   if (useri_beaconed) {
      if (replace) strncpy(h,"      Ok  ",201u);
      else strncpy(h,"    Add   ",201u);
      aprsstr_Append(h, 201ul, "|   Draw  | Send now|  Cancel |    Close",
                41ul);
      addline(m, h, 201ul, "\223", 2ul, 8100UL);
   }
   else {
      addline(m, "              New       |             Close", 44ul, "\223",
                 2ul, 8110UL);
   }
   if (useri_beaconed) {
      useri_confstr(useri_fRBTYP, s, 201ul);
      typ = X2C_CAP(s[0U]);
      bkn = (((typ!='O' && typ!='H') && typ!='P') && typ!='I') && typ!='J';
      useri_confstr(useri_fRBPOSTYP, (char *) &postyp, 1u/1u);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBNAME, 8115UL);
      if (bkn && aprstext_getmypos(&mypos)) {
         strncpy(s,"\346\356\351 ",201u);
         aprsstr_Append(s, 201ul, configs[useri_fRBPOS].title, 31ul);
         aprsstr_Append(s, 201ul, " (Config/Online) ", 18ul);
         useri_confappend(useri_fMYPOS, s, 201ul);
      }
      else s[0] = 0;
      AddEditLine(m, "\223", 2ul, s, 201ul, useri_fRBPOS, 8120UL);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBCOMMENT, 8122UL);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBTIME, 8124UL);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBPATH, 8126UL);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBALT, 8128UL);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBSPEED, 8130UL);
      AddEditLine(m, "\223", 2ul, "", 1ul, useri_fRBDIR, 8132UL);
      /*    AddEditLine(m, CMDADDBEACON, "", fRBSYMB,    MINH*81+34); */
      if (bkn && (postyp=='m' || postyp=='M')) {
         strncpy(s,"\346\356\351 ",201u);
         aprsstr_Append(s, 201ul, configs[useri_fRBDEST].title, 31ul);
         aprsstr_Append(s, 201ul, " ", 2ul);
         useri_confappend(useri_fRBDEST, s, 201ul);
      }
      else s[0] = 0;
      AddEditLine(m, "\223", 2ul, s, 201ul, useri_fRBDEST, 8136UL);
      strncpy(h," Choose Symbol  [",201u);
      useri_confstr(useri_fRBSYMB, s, 201ul);
      aprsstr_Append(h, 201ul, s, 201ul);
      aprsstr_Append(h, 201ul, "]", 2ul);
      if (s[0U]=='\\' && s[1U]=='l') {
         aprsstr_Append(h, 201ul, " Area Symbol!", 14ul);
      }
      addline(m, h, 201ul, "\223>", 3ul, 8138UL);
      ++m->scroll;
      drawsymsquare(m->image, s[0U], s[1U], (long)(m->xsize-25UL),
                (long)((m->oldknob-1UL)*m->yknob+m->yknob/2UL));
      strncpy(s,"\355",201u);
      AppBlueButton(s, 201ul, " Beacon |", 10ul, bkn);
      AppBlueButton(s, 201ul, "Object/d|", 10ul, typ=='O');
      AppBlueButton(s, 201ul, "Object/h|", 10ul, typ=='H');
      AppBlueButton(s, 201ul, "   Item |", 10ul, typ=='I');
      AppBlueButton(s, 201ul, "  -Item |", 10ul, typ=='J');
      AppBlueButton(s, 201ul, " -Object", 9ul, typ=='P');
      addline(m, s, 201ul, "\223", 2ul, 8140UL);
      ++m->scroll;
      strncpy(s,"\355",201u);
      AppBlueButton(s, 201ul, " Lat/Long |", 12ul, postyp=='g');
      if (bkn) AppBlueButton(s, 201ul, "    Mic-e |", 12ul, postyp=='m');
      else AppBlueButton(s, 201ul, " Multiline|", 12ul, postyp=='A');
      /*      ELSE AppBlueButton(s, "          |", FALSE) END;  */
      AppBlueButton(s, 201ul, "Compressed|", 12ul, postyp=='c');
      AppBlueButton(s, 201ul, " Lat/Long+|", 12ul, postyp=='G');
      if (bkn) AppBlueButton(s, 201ul, "   Mic-e+ |", 12ul, postyp=='M');
      else AppBlueButton(s, 201ul, "          |", 12ul, 0);
      addline(m, s, 201ul, "\223", 2ul, 8150UL);
      ++m->scroll;
      useri_confstr(useri_fRBPORT, s, 201ul);
      port = X2C_CAP(s[0U]);
      strncpy(s,"\355",201u);
      AppBlueButton(s, 201ul, "   Tx Net |", 12ul, port=='N');
      for (i = 1UL; i<=4UL; i++) {
         strncpy(h,"  TxPort ",201u);
         aprsstr_Append(h, 201ul, (char *)(tmp = (char)(i+48UL),&tmp),
                1u/1u);
         if (i!=4UL) aprsstr_Append(h, 201ul, "|", 2ul);
         AppBlueButton(s, 201ul, h, 201ul, port==(char)(i+48UL));
      } /* end for */
      addline(m, s, 201ul, "\223", 2ul, 8160UL);
      ++m->scroll;
   }
   useri_confstr(useri_fRBNAME, ename, 9ul);
   cntm = 0UL;
   while (cnt>0UL) {
      useri_confstrings(useri_fRBTEXT, cntm, 1, h, 201ul);
      aprsdecode_getbeaconname(h, 201ul, name, 9ul, symb, 3ul, &isobj,
                &isdel, &isbad);
      if (isbad) {
         if ((unsigned char)symb[0U]<' ') strncpy(s,"\352\353     ?? ",201u);
         else strncpy(s,"\352\353        ",201u);
      }
      else if (useri_beaconed && aprsstr_StrCmp(name, 9ul, ename, 9ul)) {
         strncpy(s,"\356        ",201u);
      }
      else if (isdel) {
         /*      ELSIF isbad THEN  */
         /*        IF symb[0]<" " THEN s:=MOP4+MOP5+"     ?? " ELSE s:=MOP4+MOP5+"        " END;
                 */
         strncpy(s,"\352\347        ",201u);
      }
      else strncpy(s,"        ",201u);
      aprsstr_Append(s, 201ul, h, 201ul);
      if (aprsstr_Length(s, 201ul)>=54UL) {
         s[53U] = 0;
         aprsstr_Append(s, 201ul, "...", 4ul);
      }
      addline(m, s, 201ul, "\223>", 3ul, 8170UL);
      if ((unsigned char)symb[0U]>=' ') {
         drawsymsquare(m->image, symb[0U], symb[1U], 24L,
                (long)((m->oldknob-1UL)*m->yknob+m->yknob/2UL));
      }
      onoff(m, 8L, configson(useri_fRBTEXT, cntm));
      ++cntm;
      --cnt;
   }
   /*    drawpullknob(m^.image, xw, m^.oldknob*m^.yknob+1,
                m^.oldknob*m^.yknob+YPULLBUTTON);  */
   /*    col.r:=400; col.g:=400; col.b:=400; */
   /*    drawsquare(m^.image, col, 0, m^.oldknob*m^.yknob+1, xw-2,
                m^.oldknob*m^.yknob+YPULLBUTTON); */
   m->hiknob = hks;
   m->ysize = m->oldknob*m->yknob;
   i = m->oldknob;
   m->oldknob = oks;
   m->wid = 226UL;
   m->pullconf = useri_fMENUXYBEACON;
   m->pullyknob = 8UL;
   if (xw<(unsigned long)maptool_xsize) {
      xp = ((unsigned long)maptool_xsize-xw)/2UL;
   }
   else xp = 0UL;
   if (aprsdecode_lums.headmenuy) yp = aprsdecode_lums.fontysize;
   else yp = 0UL;
   setmenupos(m, xp, yp);
   /*
       IF (menupullpos[BEACONEDITID].x=0) & (menupullpos[BEACONEDITID].y=0)
                THEN
   --    IF xw<VAL(CARDINAL,xsize) THEN m^.x0:=(VAL(CARDINAL,
                xsize)-xw) DIV 2 ELSE m^.x0:=0 END;
         IF xw<VAL(CARDINAL,xsize) THEN i:=(VAL(CARDINAL,
                xsize)-xw) DIV 2 ELSE i:=0 END;
         setunderbar(m, i);
   --    m^.y0:=YPOS;
         menupullpos[BEACONEDITID].x:=m^.x0;
         menupullpos[BEACONEDITID].y:=m^.y0;
       ELSE m^.x0:=menupullpos[BEACONEDITID].x;
                m^.y0:=menupullpos[BEACONEDITID].y; END;
   --      IF xw<VAL(CARDINAL,xsize) THEN m^.x0:=(VAL(CARDINAL,
                xsize)-xw) DIV 2 ELSE m^.x0:=0 END;
   --      m^.y0:=YPOS;
   */
   m->oldsub = ohs;
   if (m->hiknob>0UL && m->hiknob<=i) {
      if (X2C_IN(m->hiknob,31,m->clampkb)) keybknob(m, m->hiknob, 1, 0UL);
      inv(m, m->oldsub, m->hiknob);
   }
/*
  IF NOT beaconed & (beaconimported[0]<>0C) & (beaconimporttime+4>time())
                THEN
(*
    IF beaconimported[0]=LF THEN
      textautosize(0, 0, 5, 5, "e", "Object Cloned to Beacon Editor!");
      beaconimported[0]:=0C;
    ELSIF beaconimporttime+2>time() THEN

      s:="Click again ["; Append(s, beaconimported);
                Append(s, "] to Clone Object");
      textautosize(0, 0, 5, 2, "b", s);
*)

      s:="Clone ["; Append(s, beaconimported);
                Append(s, "] to Beacon Editor");
      textautomenu(DOCKX, 0, 5, 2, "g", "", s, CMDCLONE);

(*
    END;
*)
  END;
*/
} /* end beaconeditor() */


static void managebeacon(unsigned long scroll, unsigned long knob,
                unsigned long subknob, char folded)
{
   char ch;
   pMENU menu;
   unsigned long len;
   char typ;
   char h[1000];
   char s[1000];
   if (knob==1UL) {
      if (subknob==0UL) {
         if (useri_beaconed) {
            /* Add beacon */
            useri_confstr(useri_fRBTYP, (char *) &typ, 1u/1u);
            if ((((typ=='O' || typ=='H') || typ=='P') || typ=='I')
                || typ=='J') {
               /* obj item */
               useri_confstr(useri_fRBNAME, s, 1000ul);
               DelBeaconByName(s, 1000ul);
            }
            aprstext_encbeacon(s, 1000ul, &len);
            useri_AddConfLine(useri_fRBTEXT, 0U, s, 1000ul);
         }
         else {
            /* New baecon */
            useri_confstr(useri_fMYCALL, s, 1000ul);
            useri_AddConfLine(useri_fRBNAME, 1U, s, 1000ul);
            useri_AddConfLine(useri_fRBCOMMENT, 1U, "", 1ul);
            useri_AddConfLine(useri_fRBTIME, 1U, "3600", 5ul);
            useri_AddConfLine(useri_fRBPATH, 1U, "-1", 3ul);
            useri_AddConfLine(useri_fRBALT, 1U, "", 1ul);
            useri_AddConfLine(useri_fRBSPEED, 1U, "", 1ul);
            useri_AddConfLine(useri_fRBDIR, 1U, "", 1ul);
            useri_AddConfLine(useri_fRBSYMB, 1U, "/-", 3ul);
            useri_AddConfLine(useri_fRBDEST, 1U, "APLM01", 7ul);
            useri_AddConfLine(useri_fRBTYP, 1U, "b", 2ul);
            useri_AddConfLine(useri_fRBPOSTYP, 1U, "g", 2ul);
            useri_AddConfLine(useri_fRBPORT, 1U, "1", 2ul);
         }
         useri_killmenuid(226UL);
         useri_beaconed = 1;
      }
      else if (subknob==1UL) {
         /* draw local */
         if (useri_beaconed) {
            aprstext_encbeacon(s, 1000ul, &len);
            if (s[0U]) aprsdecode_drawbeacon(s, 1000ul);
         }
         else {
            useri_beaconediting = 0; /* Close */
            useri_killmenuid(226UL);
         }
      }
      else if (subknob==2UL) {
         if (useri_beaconed) {
            aprstext_encbeacon(aprsdecode_testbeaconbuf, 512ul, &len);
                /* Send beacon */
         }
      }
      else if (subknob==3UL) {
         /*      ELSE configedit:=0; killmenuid(BEACONEDITID) END;
                (* Close *)                    */
         /* Cancel beacon */
         useri_killmenuid(226UL);
         if (!useri_beaconed) configedit = 0UL;
         useri_beaconed = 0;
      }
      else {
         useri_beaconediting = 0; /* Close beacon */
         useri_killmenuid(226UL);
      }
   }
   else if (!useri_beaconed || knob>14UL) {
      if (folded) {
         newmenu(&menu, 55UL, aprsdecode_lums.fontysize+5UL, 8UL,
                useri_bTRANSP);
         menu->scroll = knob-scroll;
         addline(menu, "Edit Cut", 9ul, "\213", 2ul, 8180UL);
         addline(menu, "Send now", 9ul, "\213", 2ul, 8182UL);
         addline(menu, "Draw", 5ul, "\213", 2ul, 8184UL);
         addline(menu, "Raw Edit", 9ul, "\213", 2ul, 8186UL);
         addline(menu, "Delete", 7ul, "\213", 2ul, 8188UL);
         addline(menu, "Edit Copy", 10ul, "\213", 2ul, 8190UL);
         addline(menu, "On/Off", 7ul, "\213", 2ul, 8192UL);
         menu->ysize = menu->oldknob*menu->yknob;
         menu->oldknob = 0UL;
         useri_confstrings(useri_fRBTEXT, menu->scroll-1UL, 1, s, 1000ul);
         h[0U] = 0;
         aprstext_Apphex(h, 1000ul, s, 1000ul);
         useri_say(h, 1000ul, 4UL, 'b');
      }
   }
   else if (knob==11UL) {
      if (folded) symchoose('\244');
   }
   else if (knob==12UL) {
      if (subknob==1UL) ch = 'O';
      else if (subknob==2UL) ch = 'H';
      else if (subknob==3UL) ch = 'I';
      else if (subknob==4UL) ch = 'J';
      else if (subknob==5UL) ch = 'P';
      else ch = 'B';
      useri_AddConfLine(useri_fRBTYP, 1U, (char *) &ch, 1u/1u);
      if (ch!='B') {
         useri_confstr(useri_fRBPOSTYP, (char *) &ch, 1u/1u);
         ch = X2C_CAP(ch);
         if (ch=='m' || ch=='M') {
            /* no mice for items/objects */
            ch = 'c'; /* set to compressed */
            useri_AddConfLine(useri_fRBPOSTYP, 1U, (char *) &ch, 1u/1u);
            useri_say("encoding changed to \'Compressed\'", 33ul, 4UL, 'r');
         }
      }
   }
   else if (knob==13UL) {
      useri_confstr(useri_fRBTYP, (char *) &typ, 1u/1u);
      if (subknob==1UL) {
         if ((((typ=='O' || typ=='H') || typ=='P') || typ=='I') || typ=='J') {
            ch = 'A';
            aprsdecode_click.cmd = ' ';
         }
         else ch = 'm';
      }
      else if (subknob==2UL) ch = 'c';
      else if (subknob==3UL) ch = 'G';
      else if (subknob==4UL) ch = 'M';
      else ch = 'g';
      if ((((typ=='O' || typ=='H') || typ=='P') || typ=='I') || typ=='J') {
         /* obj item */
         if (ch=='m' || ch=='M') {
            /* mice only in beacon */
            ch = 'c';
            useri_say("encoding changed to \'Compressed\'", 33ul, 4UL, 'r');
         }
      }
      useri_AddConfLine(useri_fRBPOSTYP, 1U, (char *) &ch, 1u/1u);
   }
   else if (knob==14UL) {
      if (subknob==0UL) ch = 'N';
      else ch = (char)(subknob+48UL);
      useri_AddConfLine(useri_fRBPORT, 1U, (char *) &ch, 1u/1u);
   }
   useri_refresh = 1;
} /* end managebeacon() */


static void dobeacon(unsigned long scroll, unsigned long knob)
/* do beacon ed fold menu */
{
   char s[301];
   if (((knob==1UL || knob==2UL) || knob==3UL) || knob==6UL) {
      useri_confstrings(useri_fRBTEXT, scroll-1UL, 1, s, 301ul);
      if (knob==1UL || knob==6UL) aprsdecode_extractbeacon(s, 301ul, 1, 1);
   }
   if (knob==1UL) {
      useri_beaconed = 1;
      configdelman(useri_fRBTEXT, 2UL, scroll);
   }
   else if (knob==2UL) {
      aprsstr_Assign(aprsdecode_testbeaconbuf, 512ul, s, 301ul);
                /* beacon send now */
   }
   else if (knob==3UL) aprsdecode_drawbeacon(s, 301ul);
   else if (knob==5UL) configdelman(useri_fRBTEXT, 3UL, scroll);
   else if (knob==6UL) useri_beaconed = 1;
   else if (knob==7UL) configdelman(useri_fRBTEXT, 1UL, scroll);
   useri_refresh = 1;
} /* end dobeacon() */


static void beaconlen(void)
{
   char h[201];
   char s[201];
   unsigned long len;
   aprstext_encbeacon(s, 201ul, &len);
   strncpy(s,"Beacon len=",201u);
   aprsstr_IntToStr((long)len, 1UL, h, 201ul);
   aprsstr_Append(s, 201ul, h, 201ul);
   useri_say(s, 201ul, 4UL, 'b');
} /* end beaconlen() */

/* === beacons === */
/* === digi === */

static void extractdigi(char s[], unsigned long s_len, char vv[],
                unsigned long vv_len, char vn[], unsigned long vn_len,
                char vx[], unsigned long vx_len, char * from, char * to,
                char * dir, char * cut, unsigned long * t,
                unsigned long * km)
{
   char h[201];
   X2C_PCOPY((void **)&s,s_len);
   aprsstr_Extractword(s, s_len, h, 201ul);
   *from = h[0U];
   if (((unsigned char)*from<'1' || (unsigned char)*from>'4') && *from!='*') {
      *from = '*';
   }
   *to = h[1U];
   if ((unsigned char)*to<'1' || (unsigned char)*to>'4') *to = '1';
   *dir = (unsigned char)h[2U]<='1';
   *cut = !((unsigned long)(unsigned char)h[2U]&1);
   aprsstr_Extractword(s, s_len, h, 201ul);
   if (!aprsstr_StrToCard(h, 201ul, t) || *t>86400UL) *t = 890UL;
   aprsstr_Extractword(s, s_len, h, 201ul);
   if (!aprsstr_StrToCard(h, 201ul, km) || *km>30000UL) *km = 30000UL;
   vv[0UL] = 0;
   vn[0UL] = 0;
   vx[0UL] = 0;
   for (;;) {
      aprsstr_Extractword(s, s_len, h, 201ul);
      if (h[0U]==0) break;
      if (h[0U]=='v') {
         aprsstr_Delstr(h, 201ul, 0UL, 1UL);
         aprsstr_Append(vv, vv_len, h, 201ul);
         aprsstr_Append(vv, vv_len, " ", 2ul);
      }
      else if (h[0U]=='n') {
         aprsstr_Delstr(h, 201ul, 0UL, 1UL);
         aprsstr_Append(vn, vn_len, h, 201ul);
         aprsstr_Append(vn, vn_len, " ", 2ul);
      }
      else if (h[0U]=='x') {
         aprsstr_Delstr(h, 201ul, 0UL, 1UL);
         aprsstr_Append(vx, vx_len, h, 201ul);
         aprsstr_Append(vx, vx_len, " ", 2ul);
      }
   }
   X2C_PFREE(s);
} /* end extractdigi() */

#define useri_XSIZE1 300

#define useri_DIGIMLINES 6
/* static menu lines digi config */


static void digieditor(void)
{
   pMENU m;
   unsigned long radius;
   unsigned long duptime;
   unsigned long yp;
   unsigned long xp;
   unsigned long i;
   unsigned long cntm;
   unsigned long cnt;
   unsigned long xw;
   char vx[201];
   char vn[201];
   char vv[201];
   char h[201];
   char s[201];
   char to;
   char from;
   char cutpath;
   char direct;
   unsigned long ohs;
   unsigned long hks;
   unsigned long oks;
   /*  confstr(fDIGI, ename); */
   /*  replace:=FALSE; */
   cntm = cntconfigs(useri_fDIGI);
   cnt = cntm;
   /*  i:=mainys() DIV (lums.fontysize+5); */
   /*  IF i<DIGIMLINES+1 THEN i:=1 ELSIF digied THEN DEC(i, DIGIMLINES) END;
                 */
   /*  IF cnt>i THEN cnt:=i END; */
   if (cnt>=24UL) cnt = 24UL;
   xw = 300UL;
   if (300UL>(unsigned long)maptool_xsize) xw = (unsigned long)maptool_xsize;
   m = findmenuid(224UL);
   if (m==0) digiedline[0U] = 0;
   refrmenu(&m, xw, aprsdecode_lums.fontysize+5UL, cnt+6UL+1UL, useri_bCOLOR,
                 0); /* +1 for pullknob */
   oks = m->oldknob;
   hks = m->hiknob;
   ohs = m->oldsub;
   m->oldknob = 0UL;
   m->scroll = 1UL;
   /*    IF (cnt>0) & (digiedline[0]<>0C)
                THEN h:="        Add     |        Save     |      Close" */
   /*    ELSE          h:="           Add        |              Close" END;
                */
   /*    addline(m, h, CMDDIGILINE, MINH*83); */
   addline(m, "         Add     |        Save     |      Close", 48ul,
                "\314", 2ul, 8305UL);
   AddEditLine(m, "\314", 2ul, "", 1ul, useri_fDIGIX, 8310UL);
   AddEditLine(m, "\314", 2ul, "", 1ul, useri_fDIGINN, 8315UL);
   AddEditLine(m, "\314", 2ul, "", 1ul, useri_fDIGIVIA, 8320UL);
   AddEditLine(m, "\314", 2ul, "", 1ul, useri_fDIGIRADIUS, 8325UL);
   AddEditLine(m, "\314", 2ul, "", 1ul, useri_fDIGITIME, 8330UL);
   cntm = 0UL;
   while (cnt>0UL) {
      useri_confstrings(useri_fDIGI, cntm, 1, h, 201ul);
      extractdigi(h, 201ul, vv, 201ul, vn, 201ul, vx, 201ul, &from, &to,
                &direct, &cutpath, &duptime, &radius);
      if (X2C_STRCMP(digiedline,201u,h,201u)==0) strncpy(s,"\356",201u);
      else s[0U] = 0;
      strncpy(vv,"\346   |   |   |      |     |",201u);
      vv[6U] = from;
      vv[10U] = to;
      aprsstr_Append(s, 201ul, vv, 201ul);
      aprsstr_CardToStr(duptime, 5UL, vv, 201ul);
      aprsstr_Append(s, 201ul, vv, 201ul);
      aprsstr_Append(s, 201ul, " ", 2ul);
      aprsstr_CardToStr(radius, 5UL, vv, 201ul);
      aprsstr_Append(s, 201ul, vv, 201ul);
      aprsstr_Append(s, 201ul, " ", 2ul);
      aprsstr_Extractword(h, 201ul, vv, 201ul);
      aprsstr_Extractword(h, 201ul, vv, 201ul);
      aprsstr_Extractword(h, 201ul, vv, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      if (aprsstr_Length(s, 201ul)>=54UL) {
         s[53U] = 0;
         aprsstr_Append(s, 201ul, "...", 4ul);
      }
      addline(m, s, 201ul, "\314>", 3ul, 8335UL);
      onoff(m, 8L, configson(useri_fDIGI, cntm));
      onoff(m, 72L, direct);
      onoff(m, 105L, cutpath);
      ++cntm;
      --cnt;
   }
   if (cntm>0UL) {
      addline(m, "\346\347\351\352On |Rx |Tx |First |Last |Dupes |Radius",
                43ul, " ", 2ul, 8350UL);
   }
   m->hiknob = hks;
   m->ysize = m->oldknob*m->yknob;
   i = m->oldknob;
   m->oldknob = oks;
   m->wid = 224UL;
   m->pullconf = useri_fMENUXYDIGI;
   m->pullyknob = 8UL;
   if (xw<(unsigned long)maptool_xsize) {
      xp = ((unsigned long)maptool_xsize-xw)/2UL;
   }
   else xp = 0UL;
   if (aprsdecode_lums.headmenuy) {
      yp = aprsdecode_lums.fontysize;
   }
   else yp = 0UL;
   setmenupos(m, xp, yp);
   m->oldsub = ohs;
   if (m->hiknob>0UL && m->hiknob<=i) {
      if (X2C_IN(m->hiknob,31,m->clampkb)) keybknob(m, m->hiknob, 1, 0UL);
      inv(m, m->oldsub, m->hiknob);
   }
} /* end digieditor() */


static void appdigi(char s[], unsigned long s_len, unsigned char c,
                char typ)
{
   char ss[200];
   char h[200];
   useri_confstr(c, ss, 200ul);
   for (;;) {
      aprsstr_Extractword(ss, 200ul, h, 200ul);
      if (h[0U]==0) break;
      aprsstr_Append(s, s_len, (char *) &typ, 1u/1u);
      aprsstr_Append(s, s_len, h, 200ul);
      aprsstr_Append(s, s_len, " ", 2ul);
   }
} /* end appdigi() */


static void rollport(char * p, const char s[], unsigned long s_len)
/* return next char in string */
{
   *p = s[(unsigned long)(aprsstr_InStr(s, s_len, (char *)p,
                1u/1u)+1L)%aprsstr_Length(s, s_len)];
} /* end rollport() */


static void dodigi(unsigned long scroll, unsigned long knob,
                unsigned long subknob, unsigned char caller)
/* do digi editing */
{
   unsigned long i;
   unsigned long cd;
   unsigned long duptime;
   unsigned long radius;
   char vx[200];
   char vn[200];
   char vv[200];
   char h[200];
   char s[200];
   char to;
   char from;
   char cutpath;
   char direct;
   pMENU menu;
   /*WrInt(caller, 2); WrInt(scroll, 10); WrInt(knob, 10);
                WrInt(subknob, 10); WrStrLn(" dodigi"); */
   if (caller==2U) {
      if (subknob==5UL) {
         /* fold submenu */
         newmenu(&menu, 55UL, aprsdecode_lums.fontysize+5UL, 3UL,
                useri_bTRANSP);
         menu->scroll = knob-scroll;
         addline(menu, "Edit", 5ul, "\315", 2ul, 8360UL);
         addline(menu, "Delete", 7ul, "\315", 2ul, 8365UL);
         menu->ysize = menu->oldknob*menu->yknob;
         menu->oldknob = 0UL;
      }
   }
   else if (caller==1U) {
      /* clicked fold submenu */
      if (knob==1UL) {
         /* Edit */
         useri_confstrings(useri_fDIGI, scroll-1UL, 1, digiedline, 201ul);
         extractdigi(digiedline, 201ul, vv, 200ul, vn, 200ul, vx, 200ul,
                &from, &to, &direct, &cutpath, &duptime, &radius);
         icfg(useri_fDIGIX, vx, 200ul);
         icfg(useri_fDIGINN, vn, 200ul);
         icfg(useri_fDIGIVIA, vv, 200ul);
         useri_int2cfg(useri_fDIGIRADIUS, (long)radius);
         useri_int2cfg(useri_fDIGITIME, (long)duptime);
      }
      else if (knob==2UL) {
         /* Delete */
         configdelman(useri_fDIGI, 3UL, scroll);
      }
   }
   else if (knob==1UL) {
      /* click from digi menue */
      strncpy(s,"111 ",200u); /* default ports and mode */
      cd = 0UL;
      i = 0UL;
      for (;;) {
         useri_confstrings(useri_fDIGI, i, 1, h, 200ul);
         if (h[0U]==0) break;
         ++i;
         if (aprsstr_StrCmp(digiedline, 201ul, h, 200ul)) {
            /* this is the original line */
            memcpy(s,h,200u);
            cd = i;
         }
      }
      s[4U] = 0;
      useri_confappend(useri_fDIGITIME, s, 200ul);
                /* build digi config string */
      aprsstr_Append(s, 200ul, " ", 2ul);
      useri_confappend(useri_fDIGIRADIUS, s, 200ul);
      aprsstr_Append(s, 200ul, " ", 2ul);
      appdigi(s, 200ul, useri_fDIGIVIA, 'v');
      appdigi(s, 200ul, useri_fDIGINN, 'n');
      appdigi(s, 200ul, useri_fDIGIX, 'x');
      if (subknob==2UL) {
         digied = 0;
         useri_killmenuid(224UL);
      }
      else if (cd==0UL || subknob==0UL) icfg(useri_fDIGI, s, 200ul);
      else {
         /* Replace */
         aprsstr_Assign(digiedline, 201ul, s, 200ul);
         confreplace(useri_fDIGI, cd-1UL, 1, s, 200ul);
      }
   }
   else if (knob>=scroll) {
      useri_confstrings(useri_fDIGI, (knob-scroll)-1UL, 1, s, 200ul);
      if (subknob==0UL) configdelman(useri_fDIGI, 1UL, knob-scroll);
      else if (subknob==1UL) {
         /* change rx port */
         rollport(&s[0U], "1234*", 6ul);
         confreplace(useri_fDIGI, (knob-scroll)-1UL, 1, s, 200ul);
      }
      else if (subknob==2UL) {
         /* change tx port */
         rollport(&s[1U], "1234", 5ul);
         confreplace(useri_fDIGI, (knob-scroll)-1UL, 1, s, 200ul);
      }
      else if (subknob==3UL) {
         /* toggle First */
         s[2U] = (char)((unsigned char)(unsigned char)s[2U]^0x2U);
         confreplace(useri_fDIGI, (knob-scroll)-1UL, 1, s, 200ul);
      }
      else if (subknob==4UL) {
         /* toggle Last*/
         s[2U] = (char)((unsigned char)(unsigned char)s[2U]^0x1U);
         confreplace(useri_fDIGI, (knob-scroll)-1UL, 1, s, 200ul);
      }
   }
   useri_refresh = 1;
} /* end dodigi() */

/* === digi === */
/* === list window === */

static void freelist(struct LISTBUFFER * b, unsigned long from)
/* truncate listline buffer */
{
   pLISTLINE bo;
   /*    len:CARDINAL; */
   pLISTLINE bl;
   bl = b->listlines;
   bo = 0;
   while (bl && from>0UL) {
      bo = bl;
      bl = bl->next;
      --from;
   }
   if (bo==0) {
      b->listlinecnt = 0UL;
      b->listlines = 0;
   }
   else bo->next = 0;
   while (bl) {
      bo = bl;
      bl = bl->next;
      /*  len:=SIZE(bo^)-HIGH(bo^.text)+1+Length(bo^.text); */
      useri_debugmem.mon -= bo->len;
      Storage_DEALLOCATE((X2C_ADDRESS *) &bo, bo->len);
      if (b->listlinecnt>0UL) --b->listlinecnt;
   }
} /* end freelist() */


static void drawbutt(maptool_pIMAGE img, long x, long y, long xs, long ys)
{
   struct maptool_PIX buttcol;
   long i;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   long tmp;
   if (((((xs>=0L && ys>=0L) && x>=0L) && y>=0L) && (x+xs)-1L<=(long)
                (img->Len1-1)) && (y+ys)-1L<=(long)(img->Len0-1)) {
      buttcol.r = 200U;
      buttcol.g = 200U;
      buttcol.b = 200U;
      drawsquare(img, buttcol, x+2L, y+2L, (x+xs)-2L, (y+ys)-2L);
      tmp = (x+xs)-2L;
      i = x+1L;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(i)*img->Len0+(y+1L)];
            anonym->r = 20U;
            anonym->g = 20U;
            anonym->b = 2U;
         }
         { /* with */
            struct maptool_PIX * anonym0 = &img->Adr[(i)*img->Len0+((y+ys)
                -1L)];
            anonym0->r = 500U;
            anonym0->g = 500U;
            anonym0->b = 500U;
         }
         if (i==tmp) break;
      } /* end for */
      tmp = (y+ys)-1L;
      i = y+2L;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct maptool_PIX * anonym1 = &img->Adr[(x+1L)*img->Len0+i];
            anonym1->r = 500U;
            anonym1->g = 500U;
            anonym1->b = 500U;
         }
         { /* with */
            struct maptool_PIX * anonym2 = &img->Adr[((x+xs)-1L)
                *img->Len0+i];
            anonym2->r = 20U;
            anonym2->g = 20U;
            anonym2->b = 20U;
         }
         if (i==tmp) break;
      } /* end for */
   }
} /* end drawbutt() */


static void drawarrow(maptool_pIMAGE img, long x, long y, long size,
                char dir)
{
   long jj;
   long ii;
   long t;
   long s;
   long j;
   long i;
   struct maptool_PIX * anonym;
   long tmp;
   long tmp0;
   s = X2C_DIV(size,2L)-3L;
   tmp = s-1L;
   i = -s;
   if (i<=tmp) for (;; i++) {
      if (i<=-s) t = s+1L;
      else t = i+X2C_DIV(s,2L);
      tmp0 = t;
      j = -t;
      if (j<=tmp0) for (;; j++) {
         if (dir=='E' || dir=='S') ii = -i;
         else ii = i;
         if (dir=='N' || dir=='S') {
            jj = ii;
            ii = j;
         }
         else jj = j;
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(x+ii)*img->Len0+(y+jj)];
                
            anonym->r = 600U;
            anonym->g = 600U;
            anonym->b = 600U;
         }
         if (j==tmp0) break;
      } /* end for */
      if (i==tmp) break;
   } /* end for */
} /* end drawarrow() */


static void drawclosewin(maptool_pIMAGE img, long x, long y, long size)
{
   long s;
   long i;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   long tmp;
   s = X2C_DIV(size,3L);
   tmp = s;
   i = -s;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct maptool_PIX * anonym = &img->Adr[(x+i)*img->Len0+(y+i)];
         anonym->r = 600U;
         anonym->g = 600U;
         anonym->b = 600U;
      }
      { /* with */
         struct maptool_PIX * anonym0 = &img->Adr[(x+i)*img->Len0+(y-i)];
         anonym0->r = 600U;
         anonym0->g = 600U;
         anonym0->b = 600U;
      }
      if (i==tmp) break;
   } /* end for */
} /* end drawclosewin() */


static void drawmaximize(maptool_pIMAGE img, long x, long y, long size)
{
   long s;
   long i;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   long tmp;
   s = X2C_DIV(size,2L)-1L;
   tmp = s;
   i = -s;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct maptool_PIX * anonym = &img->Adr[(x+i)*img->Len0+(y-s)];
         anonym->r = 600U;
         anonym->g = 600U;
         anonym->b = 600U;
      }
      { /* with */
         struct maptool_PIX * anonym0 = &img->Adr[(x+i)*img->Len0+(y+s)];
         anonym0->r = 600U;
         anonym0->g = 600U;
         anonym0->b = 600U;
      }
      { /* with */
         struct maptool_PIX * anonym1 = &img->Adr[(x-s)*img->Len0+(y+i)];
         anonym1->r = 600U;
         anonym1->g = 600U;
         anonym1->b = 600U;
      }
      { /* with */
         struct maptool_PIX * anonym2 = &img->Adr[(x+s)*img->Len0+(y+i)];
         anonym2->r = 600U;
         anonym2->g = 600U;
         anonym2->b = 600U;
      }
      if (i==tmp) break;
   } /* end for */
} /* end drawmaximize() */


static void drawnormal(maptool_pIMAGE img, long x, long y, long size)
{
   long s;
   long i;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct maptool_PIX * anonym1;
   struct maptool_PIX * anonym2;
   long tmp;
   s = X2C_DIV(size,4L);
   tmp = s;
   i = -s;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct maptool_PIX * anonym = &img->Adr[(x+i)*img->Len0+(y-s)];
         anonym->r = 600U;
         anonym->g = 600U;
         anonym->b = 600U;
      }
      { /* with */
         struct maptool_PIX * anonym0 = &img->Adr[(x+i)*img->Len0+(y+s)];
         anonym0->r = 600U;
         anonym0->g = 600U;
         anonym0->b = 600U;
      }
      if (i==tmp) break;
   } /* end for */
   tmp = s;
   i = -s;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct maptool_PIX * anonym1 = &img->Adr[(x-s)*img->Len0+(y+i)];
         anonym1->r = 600U;
         anonym1->g = 600U;
         anonym1->b = 600U;
      }
      { /* with */
         struct maptool_PIX * anonym2 = &img->Adr[(x+s)*img->Len0+(y+i)];
         anonym2->r = 600U;
         anonym2->g = 600U;
         anonym2->b = 600U;
      }
      if (i==tmp) break;
   } /* end for */
} /* end drawnormal() */


static void drawminimize(maptool_pIMAGE img, long x, long y, long size)
{
   long s;
   long i;
   struct maptool_PIX * anonym;
   long tmp;
   s = X2C_DIV(size,3L);
   tmp = s;
   i = -s;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct maptool_PIX * anonym = &img->Adr[(x+i)*img->Len0+((y-s)-1L)];
                
         anonym->r = 600U;
         anonym->g = 600U;
         anonym->b = 600U;
      }
      if (i==tmp) break;
   } /* end for */
} /* end drawminimize() */


static long limscrbar(long pot, long drawsize, long potsize)
{
   if (X2C_DIV(pot,256L)+drawsize>potsize) pot = (potsize-drawsize)*256L;
   if (pot<0L) pot = 0L;
   return pot;
} /* end limscrbar() */


static void scrollbarpos(unsigned long * start, unsigned long * len,
                long min0, long max0, long pot, long potsize)
{
   long textsize;
   textsize = max0-min0;
   if (pot<0L) pot = 0L;
   pot = X2C_DIV(pot,256L);
   if (potsize<=0L) {
      *start = 0UL;
      *len = 0UL;
   }
   else {
      if (pot>potsize) pot = potsize;
      *start = (unsigned long)X2C_DIV(textsize*pot,potsize);
      *len = (unsigned long)X2C_DIV(textsize*textsize,potsize);
      if (*len<8UL) *len = 8UL;
      if ((long)*len>=textsize) {
         *len = (unsigned long)textsize;
         *start = 0UL;
      }
      else if ((long)(*start+*len)>textsize) {
         *start = (unsigned long)(textsize-(long)*len);
      }
   }
   *start += (unsigned long)min0;
} /* end scrollbarpos() */
/*
PROCEDURE starttextpos(drawsize, pot, potsize:INTEGER):CARDINAL;
BEGIN
  pot:=pot DIV SCRMUL;
  IF pot+drawsize>potsize THEN pot:=potsize-drawsize END;
  IF pot<0 THEN pot:=0 END;
  RETURN pot
END starttextpos;
*/


static void colorfind(char s[], unsigned long s_len)
/* mark text in listwin if find string */
{
   long l;
   long j;
   long i;
   long p;
   char f[10];
   useri_confstr(useri_fFIND, f, 10ul);
   p = aprsstr_InStr(s, s_len, f, 10ul);
   if (p>=0L) {
      j = (long)(aprsstr_Length(s, s_len)+2UL);
      if (j>(long)(s_len-1)) j = (long)(s_len-1);
      i = j-2L;
      l = (long)aprsstr_Length(f, 10ul);
      while (i>=0L) {
         s[j] = s[i];
         --j;
         if (i==p+l) {
            s[j] = '\376';
            --j;
         }
         else if (i==p) {
            s[j] = '\373';
            --j;
         }
         --i;
      }
   }
} /* end colorfind() */


static void cpinv(maptool_pIMAGE im, long x, long y, long xshift, long from,
                long to)
/* invert marked text */
{
   long x1;
   long x0;
   if (from<to) {
      x0 = (x-1L)+(from-xshift)*6L;
      x1 = x+2L+(to-xshift)*6L;
      if (x1>0L && x0<(long)(im->Len1-1)) {
         if (x0<0L) x0 = 0L;
         if (x1>(long)(im->Len1-1)) x1 = (long)(im->Len1-1);
         invline(im, (unsigned long)x0, (unsigned long)y, (unsigned long)x1,
                (unsigned long)(y+(long)aprsdecode_lums.fontysize), 1);
      }
   }
} /* end cpinv() */


static void makelistwin(struct LISTBUFFER * b)
{
   pMENU m;
   pLISTLINE blh;
   pLISTLINE bl;
   unsigned long ys;
   unsigned long xs;
   unsigned long linehi;
   unsigned long x0;
   unsigned long yp;
   unsigned long xp;
   unsigned long xch;
   unsigned long xh;
   unsigned long i;
   unsigned long lc;
   unsigned long cnt;
   unsigned long xw;
   struct maptool_PIX col;
   short mb;
   short mg;
   short mr;
   char s1[1001];
   char s[1001];
   char isicon;
   unsigned long ohs;
   unsigned long hks;
   unsigned long oks;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0; /* left white line */
   struct maptool_PIX * anonym1; /* text end white line */
   /* v bar background */
   struct maptool_PIX * anonym2;
   struct maptool_PIX * anonym3; /* black right line */
   struct maptool_PIX * anonym4; /* top white line */
   struct maptool_PIX * anonym5;
   struct maptool_PIX * anonym6; /* clr top field */
   struct maptool_PIX * anonym7; /* bottom black line */
   struct maptool_PIX * anonym8; /* bottom black line */
   struct maptool_PIX * anonym9;
   struct maptool_PIX * anonym10;
   struct maptool_PIX * anonym11; /* bottom text white line */
   struct maptool_PIX * anonym12; /* left white line */
   struct maptool_PIX * anonym13; /* right dark line */
   struct maptool_PIX * anonym14; /* top white line */
   struct maptool_PIX * anonym15; /* bottom white line */
   maptool_pIMAGE anonym16;
   maptool_pIMAGE anonym17;
   maptool_pIMAGE anonym18;
   maptool_pIMAGE anonym19;
   maptool_pIMAGE anonym20;
   maptool_pIMAGE anonym21;
   maptool_pIMAGE anonym22;
   maptool_pIMAGE anonym23;
   maptool_pIMAGE anonym24;
   maptool_pIMAGE anonym25;
   maptool_pIMAGE anonym26;
   maptool_pIMAGE anonym27;
   maptool_pIMAGE anonym28;
   maptool_pIMAGE anonym29;
   maptool_pIMAGE anonym30;
   maptool_pIMAGE anonym31;
   maptool_pIMAGE anonym32;
   unsigned long tmp;
   unsigned long tmp0;
   cnt = b->listlinecnt;
   if (cnt==1UL) cnt = 2UL;
   linehi = aprsdecode_lums.fontysize;
   useri_confstr(b->isicon, (char *) &isicon, 1u/1u);
   if (isicon=='M') {
      xw = mainxs();
      ys = useri_mainys()-linehi;
   }
   else if (isicon=='I') {
      xw = linehi*3UL+4UL+aprsstr_Length(b->listwintitle, 31ul)*6UL;
      ys = linehi;
   }
   else {
      xw = (unsigned long)useri_conf2int(b->size, 0UL,
                (long)(linehi*6UL+4UL), (long)mainxs(), 200L);
      ys = (unsigned long)useri_conf2int(b->size, 1UL,
                (long)(linehi*5UL+4UL), (long)(useri_mainys()-linehi), 200L);
   }
   /*WrInt(xw, 10);WrInt(ys, 10); WrStrLn(" size"); */
   if (xw>(unsigned long)maptool_xsize) xw = (unsigned long)maptool_xsize;
   if (ys>(unsigned long)maptool_ysize) ys = (unsigned long)maptool_ysize;
   i = ys/linehi;
   if (cnt>i) cnt = i;
   m = findmenuid(225UL);
   refrmenu(&m, xw, linehi, cnt+2UL, useri_bTRANSP, 0);
   oks = m->oldknob;
   hks = m->hiknob;
   ohs = m->oldsub;
   m->minnormmax = b->isicon;
   m->pullconf = b->xy; /* store extra position for normal size list or mon */
   m->sizeconf = b->size; /* store extra size for normal size list or mon */
   if (ys>(m->image->Len0-1)+1UL) ys = (m->image->Len0-1)+1UL;
   xs = m->image->Len1-1;
   m->oldknob = 0UL;
   m->scroll = 0UL;
   m->fullclamp = 1;
   m->ysize = ys;
   mr = 2;
   mg = 2;
   mb = 2;
   if (!aprsdecode_lums.menutransp) {
      /* user disabled transparency */
      mr = (short)aprsdecode_lums.menubackcol.r;
      mg = (short)aprsdecode_lums.menubackcol.g;
      mb = (short)aprsdecode_lums.menubackcol.b;
   }
   /*
     IF m^.background=bTRANSP THEN
       mr:=30;mg:=30;mb:=30;
     ELSIF m^.background=bCOLOR THEN
       mr:=lums.menubackcol.r;
       mg:=lums.menubackcol.g;
       mb:=lums.menubackcol.b;
     ELSE mr:=1;mg:=1;mb:=1 END;
   */
   tmp = ys-1UL;
   yp = 0UL;
   if (yp<=tmp) for (;; yp++) {
      /* fill win with background */
      tmp0 = xs-1UL;
      xp = 1UL;
      if (xp<=tmp0) for (;; xp++) {
         { /* with */
            struct maptool_PIX * anonym = (anonym16 = m->image,
                &anonym16->Adr[(xp)*anonym16->Len0+yp]);
            anonym->r = (unsigned short)mr;
            anonym->g = (unsigned short)mg;
            anonym->b = (unsigned short)mb;
         }
         if (xp==tmp0) break;
      } /* end for */
      if (yp==tmp) break;
   } /* end for */
   if (ys>linehi*4UL) {
      if (m->scry>0L) m->scry += (long)(b->newlines*linehi*256UL);
      b->newlines = 0UL;
      m->scrysize = b->listlinecnt*linehi;
      yp = (unsigned long)X2C_DIV(limscrbar(m->scry, (long)(ys-linehi*4UL),
                (long)m->scrysize),256L);
      m->scroll = yp;
      bl = b->listlines; /* shift text vertical */
      while (yp>=linehi && bl) {
         bl = bl->next;
         yp -= linehi;
      }
      blh = bl;
      xch = 0UL;
      tmp = cnt;
      i = 1UL;
      if (i<=tmp) for (;; i++) {
         /* find longest visable line length */
         if (blh) {
            xp = 0UL;
            while (blh->text[xp]) {
               ++xp;
            }
            if (xp>xch) xch = xp;
            blh = blh->next;
         }
         if (i==tmp) break;
      } /* end for */
      m->scrxsize = xch*6UL;
      x0 = (unsigned long)X2C_DIV(limscrbar(m->scrx, (long)(xs-linehi*3UL),
                (long)m->scrxsize),256L);
      xp = (x0+5UL)/6UL;
      x0 = (6UL-x0%6UL)%6UL;
      yp = linehi-yp;
      tmp = cnt;
      lc = 1UL;
      if (lc<=tmp) for (;; lc++) {
         /* text lines */
         if (bl) {
            i = 0UL;
            s[0U] = 0;
            xh = xp;
            for (;;) {
               if (i>=xh) {
                  if (i-xh<1000UL) s[i-xh] = bl->text[i];
               }
               else if ((unsigned char)bl->text[i]>(unsigned char)'\201') {
                  ++xh; /* do not count color switch */
               }
               if (bl->text[i]==0) break;
               ++i;
            }
            if (s[0U]) {
               if (xp==0UL && bl->withport) {
                  col.r = 50U;
                  col.g = 50U;
                  col.b = 50U;
                  if (s[0U]=='1') {
                     col.r = 200U;
                     col.g = 200U;
                     col.b = 0U;
                  }
                  else if (s[0U]=='2') {
                     col.r = 0U;
                     col.g = 200U;
                     col.b = 400U;
                  }
                  else if (s[0U]=='3') {
                     col.r = 150U;
                     col.g = 300U;
                     col.b = 50U;
                  }
                  else if (s[0U]=='4') {
                     col.r = 200U;
                     col.g = 0U;
                     col.b = 500U;
                  }
                  drawsquare(m->image, col, (long)(x0+2UL), (long)yp,
                (long)(x0+9UL), (long)(yp+(linehi-1UL)));
               }
               colorfind(s, 1001ul);
               wrcolor(m->image, s, 1001ul, x0+2UL, 1000UL, 0, &yp);
               cpinv(m->image, (long)(x0+2UL), (long)yp, (long)xp,
                (long)bl->startcp, (long)bl->endcp);
            }
            bl = bl->next;
            yp += linehi;
         }
         if (lc==tmp) break;
      } /* end for */
      m->hiknob = hks;
      tmp = ys-1UL;
      yp = 0UL;
      if (yp<=tmp) for (;; yp++) {
         { /* with */
            struct maptool_PIX * anonym0 = (anonym17 = m->image,
                &anonym17->Adr[(0UL)*anonym17->Len0+yp]);
            anonym0->r = 300U;
            anonym0->g = 300U;
            anonym0->b = 300U;
         }
         { /* with */
            struct maptool_PIX * anonym1 = (anonym18 = m->image,
                &anonym18->Adr[((xs-linehi)-1UL)*anonym18->Len0+yp]);
            anonym1->r = 300U;
            anonym1->g = 300U;
            anonym1->b = 300U;
         }
         tmp0 = xs-2UL;
         xp = xs-linehi;
         if (xp<=tmp0) for (;; xp++) {
            { /* with */
               struct maptool_PIX * anonym2 = (anonym19 = m->image,
                &anonym19->Adr[(xp)*anonym19->Len0+yp]);
               anonym2->r = (unsigned short)mr;
               anonym2->g = (unsigned short)mg;
               anonym2->b = (unsigned short)mb;
            }
            if (xp==tmp0) break;
         } /* end for */
         { /* with */
            struct maptool_PIX * anonym3 = (anonym20 = m->image,
                &anonym20->Adr[(xs-1UL)*anonym20->Len0+yp]);
            anonym3->r = 20U;
            anonym3->g = 20U;
            anonym3->b = 0U;
         }
         if (yp==tmp) break;
      } /* end for */
      tmp = xs-1UL;
      xp = 1UL;
      if (xp<=tmp) for (;; xp++) {
         { /* with */
            struct maptool_PIX * anonym4 = (anonym21 = m->image,
                &anonym21->Adr[(xp)*anonym21->Len0+(ys-1UL)]);
            anonym4->r = 300U;
            anonym4->g = 300U;
            anonym4->b = 300U;
         }
         tmp0 = ys-2UL;
         yp = (ys-1UL)-linehi;
         if (yp<=tmp0) for (;; yp++) {
            if (xp<2UL || xp<linehi && ys-yp<3UL) {
               { /* with */
                  struct maptool_PIX * anonym5 = (anonym22 = m->image,
                &anonym22->Adr[(xp)*anonym22->Len0+yp]);
                  anonym5->r = 250U;
                  anonym5->g = 250U;
                  anonym5->b = 250U;
               }
            }
            else {
               { /* with */
                  struct maptool_PIX * anonym6 = (anonym23 = m->image,
                &anonym23->Adr[(xp)*anonym23->Len0+yp]);
                  anonym6->r = (unsigned short)mr;
                  anonym6->g = (unsigned short)mg;
                  anonym6->b = (unsigned short)mb;
               }
            }
            if (yp==tmp0) break;
         } /* end for */
         { /* with */
            struct maptool_PIX * anonym7 = (anonym24 = m->image,
                &anonym24->Adr[(xp)*anonym24->Len0+(ys-linehi)]);
            anonym7->r = 20U;
            anonym7->g = 20U;
            anonym7->b = 0U;
         }
         { /* with */
            struct maptool_PIX * anonym8 = (anonym25 = m->image,
                &anonym25->Adr[(xp)*anonym25->Len0+0UL]);
            anonym8->r = 20U;
            anonym8->g = 20U;
            anonym8->b = 0U;
         }
         tmp0 = linehi-1UL;
         yp = 1UL;
         if (yp<=tmp0) for (;; yp++) {
            if (xp>=xs-2UL || xp+linehi>=xs && yp<3UL) {
               { /* with */
                  struct maptool_PIX * anonym9 = (anonym26 = m->image,
                &anonym26->Adr[(xp)*anonym26->Len0+yp]);
                  anonym9->r = 250U;
                  anonym9->g = 250U;
                  anonym9->b = 250U;
               }
            }
            else {
               { /* with */
                  struct maptool_PIX * anonym10 = (anonym27 = m->image,
                &anonym27->Adr[(xp)*anonym27->Len0+yp]);
                  anonym10->r = (unsigned short)mr;
                  anonym10->g = (unsigned short)mg;
                  anonym10->b = (unsigned short)mb;
               }
            }
            if (yp==tmp0) break;
         } /* end for */
         { /* with */
            struct maptool_PIX * anonym11 = (anonym28 = m->image,
                &anonym28->Adr[(xp)*anonym28->Len0+linehi]);
            anonym11->r = 150U;
            anonym11->g = 150U;
            anonym11->b = 150U;
         }
         if (xp==tmp) break;
      } /* end for */
      scrollbarpos(&x0, &xp, (long)(linehi*2UL), (long)(ys-linehi*2UL),
                m->scry, (long)m->scrysize);
      drawbutt(m->image, (long)(xs-linehi), (long)x0, (long)linehi,
                (long)xp);
      scrollbarpos(&x0, &xp, (long)linehi, (long)(xs-linehi*2UL), m->scrx,
                (long)m->scrxsize);
      drawbutt(m->image, (long)x0, 0L, (long)xp, (long)linehi);
      drawarrow(m->image, (long)(linehi/2UL), (long)(linehi/2UL+1UL),
                (long)linehi, 'W');
      drawarrow(m->image, (long)((xs-linehi*2UL)+linehi/2UL),
                (long)(linehi/2UL+1UL), (long)linehi, 'E');
      drawarrow(m->image, (long)((xs-linehi/2UL)-1UL),
                (long)(linehi+linehi/2UL+1UL), (long)linehi, 'N');
      drawarrow(m->image, (long)((xs-linehi/2UL)-1UL),
                (long)(((ys-linehi)-linehi/2UL)-1UL), (long)linehi, 'S');
   }
   else {
      tmp = ys-1UL;
      yp = 1UL;
      if (yp<=tmp) for (;; yp++) {
         { /* with */
            struct maptool_PIX * anonym12 = (anonym29 = m->image,
                &anonym29->Adr[(0UL)*anonym29->Len0+yp]);
            anonym12->r = 300U;
            anonym12->g = 300U;
            anonym12->b = 300U;
         }
         { /* with */
            struct maptool_PIX * anonym13 = (anonym30 = m->image,
                &anonym30->Adr[(xs-1UL)*anonym30->Len0+yp]);
            anonym13->r = 0U;
            anonym13->g = 0U;
            anonym13->b = 0U;
         }
         if (yp==tmp) break;
      } /* end for */
      tmp = xs-2UL;
      xp = 1UL;
      if (xp<=tmp) for (;; xp++) {
         { /* with */
            struct maptool_PIX * anonym14 = (anonym31 = m->image,
                &anonym31->Adr[(xp)*anonym31->Len0+(ys-1UL)]);
            anonym14->r = 300U;
            anonym14->g = 300U;
            anonym14->b = 300U;
         }
         { /* with */
            struct maptool_PIX * anonym15 = (anonym32 = m->image,
                &anonym32->Adr[(xp)*anonym32->Len0+0UL]);
            anonym15->r = 0U;
            anonym15->g = 0U;
            anonym15->b = 0U;
         }
         if (xp==tmp) break;
      } /* end for */
   }
   drawclosewin(m->image, (long)((xs-linehi/2UL)-1UL),
                (long)((ys-linehi/2UL)-1UL), (long)linehi);
   if (isicon=='M') {
      drawnormal(m->image, (long)(((xs-linehi)-linehi/2UL)-1UL),
                (long)((ys-linehi/2UL)-1UL), (long)linehi);
   }
   else {
      drawmaximize(m->image, (long)(((xs-linehi)-linehi/2UL)-1UL),
                (long)((ys-linehi/2UL)-1UL), (long)linehi);
   }
   if (isicon=='I') {
      drawnormal(m->image, (long)(((xs-linehi*2UL)-linehi/2UL)-1UL),
                (long)((ys-linehi/2UL)-1UL), (long)linehi);
   }
   else {
      drawminimize(m->image, (long)(((xs-linehi*2UL)-linehi/2UL)-1UL),
                (long)(ys-linehi/2UL), (long)linehi);
   }
   /*drawclosewin(m^.image, linehi DIV 2-1, ys-linehi DIV 2-1, linehi); */
   /*drawmaximize(m^.image, linehi DIV 2-1, ys-linehi DIV 2-1, linehi); */
   if (isicon=='I') {
      i = 0UL;
      wrcolor(m->image, b->listwintitle, 31ul, 2UL, 700UL, 0, &i);
      m->pullconf = useri_fMENUXYLISTMIN;
   }
   else {
      i = (ys-linehi)-1UL;
      if (useri_listwin=='M') strncpy(s,"Clr",1001u);
      if (useri_listwin=='L') {
         if (listbuffer.sortby) strncpy(s,"0-24",1001u);
         else strncpy(s,"A..Z",1001u);
      }
      wrcolor(m->image, s, 1001ul, linehi, 700UL, 0, &i);
      aprsstr_Assign(s, 1001ul, b->listwintitle, 31ul);
      aprsstr_IntToStr((long)b->listlinecnt, 0UL, s1, 1001ul);
      aprsstr_Append(s, 1001ul, " (", 3ul);
      aprsstr_Append(s, 1001ul, s1, 1001ul);
      aprsstr_Append(s, 1001ul, ")", 2ul);
      lc = aprsstr_Length(s, 1001ul)*6UL;
      if (xs>lc+linehi*6UL) {
         wrcolor(m->image, s, 1001ul, (xs-lc)/2UL, 700UL, 0, &i);
      }
   }
   i = m->oldknob;
   m->oldknob = oks;
   m->wid = 225UL;
   m->pullyknob = 2147483647UL;
   if (xw<(unsigned long)maptool_xsize) {
      xp = ((unsigned long)maptool_xsize-xw)/2UL;
   }
   else xp = 0UL;
   if (aprsdecode_lums.headmenuy) yp = aprsdecode_lums.fontysize;
   else yp = 0UL;
   setmenupos(m, xp, yp);
   m->oldsub = ohs;
   if (m->hiknob>0UL && m->hiknob<=i) {
      if (X2C_IN(m->hiknob,31,m->clampkb)) keybknob(m, m->hiknob, 1, 0UL);
      inv(m, m->oldsub, m->hiknob);
   }
} /* end makelistwin() */


static void closelist(void)
{
   if (useri_listwin=='M') useri_configbool(useri_fMENUMONICON, 0);
   useri_listwin = 0;
   freelist(&listbuffer, 0UL);
   useri_killmenuid(225UL); /* Close */
} /* end closelist() */


static void limscrbars(pMENU m)
{
   m->scrx = limscrbar(m->scrx, (long)(m->xsize-aprsdecode_lums.fontysize*3UL),
                 (long)m->scrxsize);
   m->scry = limscrbar(m->scry, (long)(m->ysize-aprsdecode_lums.fontysize*4UL),
                 (long)m->scrysize);
} /* end limscrbars() */


static void sortlist(pLISTLINE * root, char bytime)
{
   pLISTLINE out;
   pLISTLINE po;
   pLISTLINE p;
   pLISTLINE in;
   out = 0;
   while (*root) {
      in = *root;
      *root = in->next;
      in->next = 0;
      if (out==0) out = in;
      else {
         po = 0;
         p = out;
         for (;;) {
            if (p==0 || (bytime && p->time0<in->time0 || !bytime && X2C_STRCMP(p->call,
                9u,in->call,9u)<0)) {
               if (po==0) {
                  in->next = out;
                  out = in;
               }
               else {
                  in->next = po->next;
                  po->next = in;
               }
               break;
            }
            po = p;
            p = p->next;
         }
      }
   }
   *root = out;
} /* end sortlist() */


static void ccp(pLISTLINE b)
{
   while (b) {
      b->endcp = 0UL;
      b->startcp = 0UL;
      b->markcp = 0UL;
      b = b->next;
   }
} /* end ccp() */


extern void useri_clrcpmarks(void)
{
   ccp(listbuffer.listlines);
   ccp(monbuffer.listlines);
   useri_refresh = 1; /* called from event */
} /* end clrcpmarks() */


static void findtextpos(pMENU m, unsigned long xcl, unsigned long ycl,
                pLISTLINE * bb, pLISTLINE * bl, unsigned long * xch)
/* find line/col of x/y pixel */
{
   long n;
   n = (long)(((m->ysize-aprsdecode_lums.fontysize)-ycl)+m->scroll);
   if (n<0L) n = 0L;
   n = (long)((unsigned long)n/aprsdecode_lums.fontysize);
   if (useri_listwin=='L') *bl = listbuffer.listlines;
   else if (useri_listwin=='M') {
      *bl = monbuffer.listlines;
      n += (long)monbuffer.newlines;
   }
   else *bl = 0;
   *bb = *bl;
   while (n>0L && *bl) {
      --n;
      *bl = (*bl)->next;
   }
   n = (long)(xcl+(unsigned long)limscrbar(m->scrx,
                (long)(m->xsize-aprsdecode_lums.fontysize*3UL),
                (long)m->scrxsize)/256UL);
   if (n<0L) n = 0L;
   *xch = (unsigned long)n/6UL;
} /* end findtextpos() */


static void dolist(pMENU m, unsigned long xcl, unsigned long ycl)
/* click on list win */
{
   char isicon;
   unsigned long xl;
   unsigned long x0;
   unsigned long xdiv;
   pLISTLINE bb;
   pLISTLINE bl;
   aprsdecode_pOPHIST op;
   /*WrInt(xcl, 10); WrInt(ycl, 10); WrStrLn(" cl"); */
   limscrbars(m);
   useri_confstr(m->minnormmax, (char *) &isicon, 1u/1u);
   if (isicon!='I' && ycl+aprsdecode_lums.fontysize>m->ysize) {
      /* bottom line */
      scrollbarpos(&x0, &xl, (long)aprsdecode_lums.fontysize,
                (long)(m->xsize-aprsdecode_lums.fontysize*2UL), m->scrx,
                (long)m->scrxsize);
      if (xcl<=aprsdecode_lums.fontysize) m->scrx = 0L;
      else if (xcl<=x0) {
         m->scrx += -(long)((m->xsize-aprsdecode_lums.fontysize*3UL)*256UL);
                /* scroll left winsize */
      }
      else if (xcl<=x0+xl) {
      }
      else if (xcl+aprsdecode_lums.fontysize*2UL<=m->xsize) {
         /* hit scrollbar */
         m->scrx += (long)((m->xsize-aprsdecode_lums.fontysize*3UL)*256UL);
                /* scroll right winsize */
      }
      else if (xcl+aprsdecode_lums.fontysize<=m->xsize) m->scrx = 8192000L;
   }
   else if (ycl<aprsdecode_lums.fontysize) {
      /* top line */
      xdiv = (m->xsize-xcl)/aprsdecode_lums.fontysize;
      if (xdiv==0UL) closelist();
      else {
         if (xdiv==1UL) {
            if (isicon=='M') isicon = 0;
            else isicon = 'M';
         }
         else if (xdiv==2UL) {
            if (isicon=='I') isicon = 0;
            else isicon = 'I';
         }
         else if (xcl<=aprsdecode_lums.fontysize+24UL && xcl>aprsdecode_lums.fontysize)
                 {
            if (useri_listwin=='M') freelist(&monbuffer, 0UL);
            if (useri_listwin=='L') {
               /* sort buffer */
               sortlist(&listbuffer.listlines, listbuffer.sortby);
               useri_clrcpmarks(); /* clear cp invers */
               listbuffer.sortby = !listbuffer.sortby;
            }
         }
         icfg(m->minnormmax, (char *) &isicon, 1u/1u);
      }
   }
   else if (xcl+aprsdecode_lums.fontysize>m->xsize) {
      /* right column */
      scrollbarpos(&x0, &xl, (long)(aprsdecode_lums.fontysize*2UL),
                (long)(m->ysize-aprsdecode_lums.fontysize*2UL), m->scry,
                (long)m->scrysize);
      if (ycl+aprsdecode_lums.fontysize*2UL>m->ysize) m->scry = 0L;
      else if (ycl<=aprsdecode_lums.fontysize*2UL) m->scry = 8192000L;
      else if (m->ysize-ycl<x0) {
         /* scroll winsize down */
         m->scry += -(long)((m->ysize-aprsdecode_lums.fontysize*4UL)*256UL);
      }
      else if (m->ysize-ycl<x0+xl) {
      }
      else {
         /* click on scrollbar */
         m->scry += (long)((m->ysize-aprsdecode_lums.fontysize*4UL)*256UL);
                /* scroll winsize up */
      }
   }
   else {
      /* click on text */
      findtextpos(m, xcl, ycl, &bb, &bl, &xl);
      /*WrInt(xcl, 10); WrInt(ycl, 10); WrInt(xl, 10); WrStrLn(" cl"); */
      /*
          n:=m^.ysize-lums.fontysize-ycl+m^.scroll;
          IF n<0 THEN n:=0 END;
          n:=VAL(CARDINAL,n) DIV lums.fontysize;
          IF listwin="L" THEN bl:=listbuffer.listlines
          ELSIF listwin="M" THEN
            bl:=monbuffer.listlines;
            INC(n, monbuffer.newlines);
          ELSE bl:=NIL END;
          WHILE (n>0) & (bl<>NIL) DO DEC(n); bl:=bl^.next; END;
      */
      if (bl) {
         if (!aprspos_posvalid(bl->position)) {
            /* no pos in line */
            op = aprstext_oppo(bl->call);
            if (op) bl->position = op->lastpos;
         }
         if (aprspos_posvalid(bl->position)) {
            aprstext_setmark1(bl->position, 1, X2C_max_longint, bl->time0);
            /*
                    click.markpos:=bl^.position;
                    click.markpost:=bl^.time;
                    click.markalti:=MAX(INTEGER);
            */
            aprsdecode_click.cmd = 't';
         }
      }
   }
   useri_refresh = 1;
} /* end dolist() */


static void wrlist(struct LISTBUFFER * b, char s[], unsigned long s_len,
                const aprsdecode_MONCALL opcall, struct aprspos_POSITION pos,
                 unsigned long t, char port)
/* append a line to list window buffer */
{
   pLISTLINE bl;
   unsigned long i;
   unsigned long start;
   unsigned long blen;
   unsigned long end;
   X2C_PCOPY((void **)&s,s_len);
   end = 0UL;
   while (end<s_len-1 && s[end]) {
      start = end;
      i = start;
      while (end<=s_len-1 && (unsigned char)s[end]>=' ') {
         s[i] = s[end];
         ++i;
         ++end;
      }
      blen = (((sizeof(struct LISTLINE)-500UL)+end)-start)+1UL;
      Storage_ALLOCATE((X2C_ADDRESS *) &bl, blen);
      useri_debugmem.req = blen;
      useri_debugmem.mon += blen;
      if (bl==0) {
         osi_WrStrLn("menu out of memory", 19ul);
         useri_wrheap();
         goto label;
      }
      memset((X2C_ADDRESS)bl,(char)0,blen);
      bl->next = b->listlines;
      bl->len = blen;
      memcpy(bl->call,opcall,9u);
      bl->withport = port;
      bl->time0 = t;
      b->listlines = bl;
      i = 0UL;
      while (i+start<end && i<500UL) {
         bl->text[i] = s[i+start];
         ++i;
      }
      bl->text[i] = 0;
      bl->position = pos;
      ++b->listlinecnt;
      if (b->newlines<=2000UL) ++b->newlines;
      if (b->listlinecnt>2010UL) freelist(b, 2000UL);
      if (s[end]) ++end;
   }
   label:;
   X2C_PFREE(s);
} /* end wrlist() */


extern void useri_wrstrlist(char s[], unsigned long s_len,
                aprsdecode_MONCALL opcall, struct aprspos_POSITION pos,
                unsigned long t)
{
   wrlist(&listbuffer, s, s_len, opcall, pos, t, 0);
} /* end wrstrlist() */


extern void useri_wrstrmon(char s[], unsigned long s_len,
                struct aprspos_POSITION pos)
{
   char isicon;
   aprsdecode_MONCALL tmp;
   wrlist(&monbuffer, s, s_len, *(aprsdecode_MONCALL *)memcpy(&tmp,"",1u),
                pos, 0UL, 1);
   if (useri_listwin=='M') {
      /* else save cpu */
      useri_confstr(useri_fMENUMONICON, (char *) &isicon, 1u/1u);
      if (isicon!='I') useri_refresh = 1;
   }
} /* end wrstrmon() */


static void startlist(char tit[], unsigned long tit_len, char oneop[],
                unsigned long oneop_len)
{
   X2C_PCOPY((void **)&tit,tit_len);
   X2C_PCOPY((void **)&oneop,oneop_len);
   aprsstr_Assign(listbuffer.listwintitle, 31ul, tit, tit_len);
   if (oneop[0UL]) {
      useri_textautosize(0L, 0L, 6UL, 4UL, 'b', "see all User, switch to \"Sh\
ow All\"", 35ul);
      aprsstr_Append(listbuffer.listwintitle, 31ul, " of ", 5ul);
      aprsstr_Append(listbuffer.listwintitle, 31ul, oneop, oneop_len);
   }
   listbuffer.isicon = useri_fMENULISTICON;
   listbuffer.xy = useri_fMENUXYLIST;
   listbuffer.size = useri_fMENUSIZELIST;
   listbuffer.sortby = 0;
   freelist(&listbuffer, 0UL);
   useri_killmenuid(225UL);
   useri_listwin = 'L';
   useri_refresh = 1;
   X2C_PFREE(tit);
   X2C_PFREE(oneop);
} /* end startlist() */


static void startmon(char manual)
{
   aprsstr_Assign(monbuffer.listwintitle, 31ul, "Monitor", 8ul);
   monbuffer.isicon = useri_fMENUMONICON;
   monbuffer.xy = useri_fMENUXYMON;
   monbuffer.size = useri_fMENUSIZEMON;
   useri_killmenuid(225UL);
   useri_listwin = 'M';
   if (manual) useri_configbool(useri_fMENUMONICON, 1);
   useri_refresh = 1;
} /* end startmon() */


static void setcp(pMENU m, unsigned long x, unsigned long y, char start)
/* copy & paste marking */
{
   pLISTLINE bb;
   pLISTLINE bl;
   unsigned long in;
   unsigned long xl;
   findtextpos(m, x, y, &bb, &bl, &xl);
   if (bl) {
      if (start) {
         /* klick */
         useri_clrcpmarks();
         bl->markcp = xl+1UL;
      }
      else {
         /* pull */
         in = 0UL;
         do {
            /*
                    IF bb=bl THEN 
                      IF in=0 THEN bb^.startcp:=xl;
                bb^.endcp:=Length(bb^.text); in:=1;
                      ELSIF in=1 THEN bb^.startcp:=0; bb^.endcp:=xl;
                in:=2 END;
                    END;
                    IF bb^.markcp>0 THEN
                      IF in=0 THEN
                        bb^.startcp:=bb^.markcp-1;
                bb^.endcp:=Length(bb^.text); in:=1;
                      ELSIF in=1 THEN bb^.startcp:=0;
                bb^.endcp:=bb^.markcp-1; in:=2 END;
                    END;
            */
            if (bb->markcp>0UL) {
               if (bb==bl) {
                  /* one line */
                  if (bb->markcp>xl) {
                     bb->startcp = xl;
                     bb->endcp = bb->markcp-1UL;
                  }
                  else {
                     bb->startcp = bb->markcp-1UL;
                     bb->endcp = xl;
                  }
                  in = 2UL;
               }
               else if (in==0UL) {
                  bb->startcp = 0UL;
                  bb->endcp = bb->markcp-1UL;
                  in = 1UL;
               }
               else if (in==1UL) {
                  bb->startcp = bb->markcp-1UL;
                  bb->endcp = aprsstr_Length(bb->text, 501ul);
                  in = 2UL;
               }
            }
            else if (bb==bl) {
               if (in==0UL) {
                  /* marker above */
                  bb->startcp = 0UL;
                  bb->endcp = xl;
                  in = 1UL;
               }
               else {
                  /* marker below */
                  bb->startcp = xl;
                  bb->endcp = aprsstr_Length(bb->text, 501ul);
                  in = 2UL;
               }
            }
            else if (in!=1UL) bb->endcp = 0UL;
            else {
               bb->startcp = 0UL;
               bb->endcp = aprsstr_Length(bb->text, 501ul);
            }
            bb = bb->next;
         } while (bb);
      }
   }
} /* end setcp() */


static char WhatPull(pMENU m, unsigned long x, unsigned long y)
{
   unsigned long syl;
   unsigned long sy0;
   unsigned long sxl;
   unsigned long sx0;
   if (m->wid==225UL) {
      scrollbarpos(&sx0, &sxl, (long)aprsdecode_lums.fontysize,
                (long)(m->xsize-aprsdecode_lums.fontysize*2UL), m->scrx,
                (long)m->scrxsize);
      scrollbarpos(&sy0, &syl, (long)(aprsdecode_lums.fontysize*2UL),
                (long)(m->ysize-aprsdecode_lums.fontysize*2UL), m->scry,
                (long)m->scrysize);
      sxl += sx0;
      syl += sy0;
      /*    limscrolls(m); */
      /*WrInt(x, 10); WrInt(y, 10); WrInt(sx0, 10); WrInt(sxl, 10); */
      /*WrInt(sy0, 10); WrInt(syl, 10); WrInt(m^.scrx, 10);
                WrInt(m^.scrxsize, 10); WrStrLn("wp"); */
      limscrbars(m);
      /*  m^.scrx:=limscrbar(m^.scrx, m^.xsize-lums.fontysize*3,
                m^.scrxsize); */
      /*  m^.scry:=limscrbar(m^.scry, m^.ysize-lums.fontysize*4,
                m^.scrysize); */
      if (m->ysize<=aprsdecode_lums.fontysize) return 0;
      /* icon */
      if ((y+aprsdecode_lums.fontysize>m->ysize && sx0<=x) && sxl>=x) {
         return 'H';
      }
      if ((x+aprsdecode_lums.fontysize>m->xsize && sy0<=m->ysize-y)
                && syl>=m->ysize-y) return 'V';
      if (y+aprsdecode_lums.fontysize>m->ysize && x+aprsdecode_lums.fontysize>m->xsize)
                 return 'B';
      if (x+aprsdecode_lums.fontysize>m->xsize) return 'E';
      if (y+aprsdecode_lums.fontysize>m->ysize) return 'S';
      if (x<aprsdecode_lums.fontysize && y<aprsdecode_lums.fontysize) {
         return 'U';
      }
      if (x<=3UL) {
         if (y<aprsdecode_lums.fontysize) return 'U';
         else return 'W';
      }
      if (y<=3UL) {
         if (x<aprsdecode_lums.fontysize) return 'U';
         else return 'N';
      }
      if (y>=aprsdecode_lums.fontysize) {
         setcp(m, x, y, 1);
         cpendx = (long)x;
         cpendy = (long)y;
         return 'C';
      }
      /* c/p */
      xosi_sethand(xosi_cPULL4);
   }
   else if (m->wid==202UL) {
      /*    RETURN 0C; */
      if (y+aprsdecode_lums.fontysize>m->ysize && x+aprsdecode_lums.fontysize>m->xsize)
                 return 'B';
      if (x+aprsdecode_lums.fontysize>m->xsize) return 'E';
      if (y+aprsdecode_lums.fontysize>m->ysize) return 'S';
      if (x<aprsdecode_lums.fontysize && y<aprsdecode_lums.fontysize) {
         return 'U';
      }
      if (x<=3UL) {
         if (y<aprsdecode_lums.fontysize) return 'U';
         else return 'W';
      }
      if (y<=3UL) {
         if (x<aprsdecode_lums.fontysize) return 'U';
         else return 'N';
      }
   }
   /*    RETURN 0C; */
   return 0;
} /* end WhatPull() */


static char Scrollbars(pMENU m, long dx, long dy)
{
   long sy;
   long sx;
   char ok0;
   char isicon;
   /*WrStr(pullmenuwhat); WrInt(dx, 5); WrInt(dy, 5);  WrStrLn(" pmw"); */
   if (pullmenuwhat=='H') {
      if (dx) {
         m->scrx += dx*(long)((256UL*m->scrxsize)
                /(m->xsize-aprsdecode_lums.fontysize*3UL));
         useri_refresh = 1;
      }
      return 1;
   }
   if (pullmenuwhat=='V') {
      if (dy) {
         m->scry -= dy*(long)((256UL*m->scrysize)
                /(m->ysize-aprsdecode_lums.fontysize*4UL));
         useri_refresh = 1;
      }
      return 1;
   }
   if (pullmenuwhat=='C') {
      /* copy & paste */
      cpendx += dx;
      cpendy += dy;
      setcp(m, (unsigned long)cpendx, (unsigned long)cpendy, 0);
      if (cpendy+(long)aprsdecode_lums.fontysize>(long)m->ysize) {
         /* scroll while cp */
         m->scry -= (long)(aprsdecode_lums.fontysize*256UL);
      }
      else if (cpendy<=(long)aprsdecode_lums.fontysize) {
         m->scry += (long)(aprsdecode_lums.fontysize*256UL);
      }
      useri_refresh = 1;
      return 1;
   }
   if (((((pullmenuwhat=='S' || pullmenuwhat=='E') || pullmenuwhat=='U')
                || pullmenuwhat=='B') || pullmenuwhat=='W')
                || pullmenuwhat=='N') {
      useri_confstr(m->minnormmax, (char *) &isicon, 1u/1u);
      if (isicon!='I' && isicon!='M') {
         /* was normal size */
         sx = useri_conf2int(m->sizeconf, 0UL, 40L, (long)mainxs(), 200L);
         sy = useri_conf2int(m->sizeconf, 1UL, 50L, (long)useri_mainys(),
                200L);
      }
      else {
         sx = (long)m->xsize;
         sy = (long)m->ysize;
      }
      ok0 = 0;
      if (pullmenuwhat=='S' || pullmenuwhat=='B') {
         if (dy) {
            sy = inclim(sy, dy, 0L);
            ok0 = 1;
         }
      }
      if (pullmenuwhat=='E' || pullmenuwhat=='B') {
         if (dx) {
            sx = inclim(sx, dx, 0L);
            ok0 = 1;
         }
      }
      else if (pullmenuwhat=='W') {
         if (dx) {
            sx = inclim(sx, -dx, 0L);
            movewin(m, dx, 0L);
            ok0 = 1;
         }
      }
      else if (pullmenuwhat=='N') {
         if (dy) {
            sy = inclim(sy, -dy, 0L);
            movewin(m, 0L, dy);
            ok0 = 1;
         }
      }
      if (pullmenuwhat=='U') {
         if (dy) {
            sy = inclim(sy, -dy, 0L);
            movewin(m, 0L, dy);
            ok0 = 1;
         }
         if (dx) {
            sx = inclim(sx, -dx, 0L);
            movewin(m, dx, 0L);
            ok0 = 1;
         }
      }
      if (ok0) {
         if ((unsigned long)m->sizeconf>0UL) {
            useri_saveXYtocfg(m->sizeconf, sx, sy);
         }
         if ((unsigned long)m->minnormmax>0UL) icfg(m->minnormmax, "", 1ul);
         useri_refresh = 1;
      }
      return 1;
   }
   return 0;
} /* end Scrollbars() */


static void listcursmove(pMENU m, char * ch)
{
   char cc;
   /*WrInt(ORD(ch), 5); WrLn; */
   cc = *ch;
   *ch = 0;
   /*  IF listlinecnt>0 THEN */
   limscrbars(m);
   /*    m^.scrx:=limscrbar(m^.scrx, m^.xsize-lums.fontysize*3, m^.scrxsize);
                 */
   /*    m^.scry:=limscrbar(m^.scry, m^.ysize-lums.fontysize*4, m^.scrysize);
                 */
   switch ((unsigned)cc) {
   case '\023':
      m->scrx += -1536L;
      break;
   case '\004':
      m->scrx += 1536L;
      break;
   case '\005':
   case '\310':
      m->scry += (long)(aprsdecode_lums.fontysize*256UL);
      break;
   case '\030':
   case '\311':
      m->scry += -(long)(aprsdecode_lums.fontysize*256UL);
      break;
   case '\022':
      m->scry += (long)((m->ysize-aprsdecode_lums.fontysize*4UL)*256UL);
      break;
   case '\003':
      m->scry += -(long)((m->ysize-aprsdecode_lums.fontysize*4UL)*256UL);
      break;
   case '\001':
      m->scrx = 0L;
      break;
   case '\002':
      m->scrx = 8192000L;
      break;
   default:;
      *ch = cc; /* give key back if not known here */
      break;
   } /* end switch */
   useri_refresh = 1;
/*  END; */
} /* end listcursmove() */


static void listwincursors(pMENU m, unsigned long x, unsigned long y)
{
   unsigned long syl;
   unsigned long sy0;
   unsigned long sxl;
   unsigned long sx0;
   scrollbarpos(&sx0, &sxl, (long)aprsdecode_lums.fontysize,
                (long)(m->xsize-aprsdecode_lums.fontysize*2UL), m->scrx,
                (long)m->scrxsize);
   scrollbarpos(&sy0, &syl, (long)(aprsdecode_lums.fontysize*2UL),
                (long)(m->ysize-aprsdecode_lums.fontysize*2UL), m->scry,
                (long)m->scrysize);
   sxl += sx0;
   syl += sy0;
   /*WrInt(sy0, 10); WrInt(syl, 10); WrInt(m^.ysize-y, 10); WrLn; */
   if (x<=aprsdecode_lums.fontysize) {
      if (y<aprsdecode_lums.fontysize) xosi_sethand(xosi_cPULLLEFTUP);
      else if (x<=3UL) xosi_sethand(xosi_cPULLLEFT);
   }
   else if (x+aprsdecode_lums.fontysize>m->xsize && (sy0>m->ysize-y || syl<m->ysize-y)
                ) {
      if (y+aprsdecode_lums.fontysize>m->ysize) {
         xosi_sethand(xosi_cPULLRIGHTDOWN);
      }
      else if (x+3UL>m->xsize) xosi_sethand(xosi_cPULLRIGHT);
   }
   else if (y<=3UL) xosi_sethand(xosi_cPULLUP);
   else if (y+3UL>m->ysize && (sx0>x || sxl<x)) xosi_sethand(xosi_cPULLDOWN);
} /* end listwincursors() */

/* === list window === */
/* === panorama === */

static void makepanwin(void)
{
   pMENU m;
   unsigned long yp;
   unsigned long xp;
   unsigned long ys;
   unsigned long xs;
   unsigned long xw;
   char abo;
   unsigned long ohs;
   unsigned long oks;
   if (panowin.isicon) {
      xw = 64UL;
      ys = aprsdecode_lums.fontysize;
   }
   else {
      xw = (unsigned long)useri_conf2int(useri_fPANOSIZE, 0UL,
                (long)(aprsdecode_lums.fontysize*6UL+4UL), (long)mainxs(),
                200L);
      ys = (unsigned long)useri_conf2int(useri_fPANOSIZE, 1UL,
                (long)(aprsdecode_lums.fontysize*5UL+4UL),
                (long)(useri_mainys()-aprsdecode_lums.fontysize), 100L);
   }
   if (xw>(unsigned long)maptool_xsize) xw = (unsigned long)maptool_xsize;
   if (ys>(unsigned long)maptool_ysize) ys = (unsigned long)maptool_ysize;
   /*WrInt(xw, 10);WrInt(ys, 10); WrStrLn("x y"); */
   m = findmenuid(202UL);
   refrmenu(&m, xw, ys, 1UL, useri_bCOLOR, 1);
   /*WrInt(ORD(new), 2); WrStrLn("imgnew"); */
   oks = m->oldknob;
   ohs = m->oldsub;
   /*  m^.minnormmax:=; */
   ys = (m->image->Len0-1)+1UL;
   xs = (m->image->Len1-1)+1UL;
   panowin.ximg = (long)xs;
   panowin.yimg = (long)ys;
   panowin.image = m->image;
   m->sizeconf = useri_fPANOSIZE;
   m->pullconf = useri_fPANOPOS;
   m->oldknob = 0UL;
   m->fullclamp = 1;
   m->ysize = ys;
   m->oldknob = oks;
   m->wid = 202UL;
   if (panowin.empty) {
      maptool_Panorama(redrawimg, panowin, &abo);
      panowin.empty = 0;
   }
   m->pullyknob = 2147483647UL;
   if (xw<(unsigned long)maptool_xsize) {
      xp = ((unsigned long)maptool_xsize-xw)/2UL;
   }
   else xp = 0UL;
   if (aprsdecode_lums.headmenuy) yp = aprsdecode_lums.fontysize;
   else yp = 0UL;
   setmenupos(m, xp, yp);
   m->oldsub = ohs;
} /* end makepanwin() */


static void startpano(void)
{
   /*WrStrLn("startpan"); */
   if (!panowin.on) {
      useri_killmenuid(202UL);
      if (aprspos_posvalid(aprsdecode_click.markpos)
                && aprspos_posvalid(aprsdecode_click.measurepos)) {
         panowin.eyealt = useri_conf2int(useri_fANT1, 0UL, -1000000L,
                1000000L, -1000000L);
         if (panowin.eyealt>-1000000L) {
            panowin.eye = aprsdecode_click.markpos;
            panowin.horizon = aprsdecode_click.measurepos;
            panowin.angle = useri_conf2real(useri_fANT2, 1UL, (-360.0f),
                360.0f, 45.0f);
            panowin.flatscreen = panowin.angle<0.0f;
            panowin.angle = (float)fabs(panowin.angle);
            if (panowin.angle<0.01f) panowin.angle = 0.01f;
            panowin.elevation = useri_conf2real(useri_fANT2, 2UL, (-180.0f),
                180.0f, 0.0f);
            panowin.yzoom = useri_conf2real(useri_fANT2, 3UL, 0.01f, 100.0f,
                1.0f);
            panowin.isicon = 0;
            panowin.on = 1;
            panowin.empty = 1;
            useri_refresh = 1;
         }
         else useri_xerrmsg("Panorama: need altitude at Marker 1", 36ul);
      }
      else useri_xerrmsg("Panorama: need Marker 1 and 2", 30ul);
   }
} /* end startpano() */


static void closepano(void)
{
   panowin.on = 0;
   useri_killmenuid(202UL); /* Close */
   disposeimg(&useri_panoimage);
} /* end closepano() */


static void dopano(pMENU m, unsigned long xcl, unsigned long ycl)
/* click on panorama win */
{
   char isicon;
   unsigned long xl;
   unsigned long x0;
   unsigned long xdiv;
   /*WrInt(xcl, 10); WrInt(ycl, 10); WrStrLn(" dopano"); */
   limscrbars(m);
   /*  confstr(m^.minnormmax, isicon); */
   isicon = 0;
   if (ycl+aprsdecode_lums.fontysize>m->ysize) {
      /* bottom line */
      scrollbarpos(&x0, &xl, (long)aprsdecode_lums.fontysize,
                (long)(m->xsize-aprsdecode_lums.fontysize*2UL), m->scrx,
                (long)m->scrxsize);
      if (xcl<=aprsdecode_lums.fontysize) m->scrx = 0L;
      else if (xcl<=x0) {
         m->scrx += -(long)((m->xsize-aprsdecode_lums.fontysize*3UL)*256UL);
                /* scroll left winsize */
      }
      else if (xcl<=x0+xl) {
      }
      else if (xcl+aprsdecode_lums.fontysize*2UL<=m->xsize) {
         /* hit scrollbar */
         m->scrx += (long)((m->xsize-aprsdecode_lums.fontysize*3UL)*256UL);
                /* scroll right winsize */
      }
      else if (xcl+aprsdecode_lums.fontysize<=m->xsize) m->scrx = 8192000L;
   }
   else if (ycl<aprsdecode_lums.fontysize) {
      /* top line */
      xdiv = (m->xsize-xcl)/aprsdecode_lums.fontysize;
      if (xdiv==0UL) closepano();
      else {
         if (xdiv==1UL) isicon = 'M';
         else if (xdiv==2UL) isicon = 'I';
         else {
            xcl<=aprsdecode_lums.fontysize+24UL && xcl>aprsdecode_lums.fontysize;
         }
         if ((unsigned long)m->minnormmax>0UL) {
            icfg(m->minnormmax, (char *) &isicon, 1u/1u);
         }
      }
   }
   else if (xcl+aprsdecode_lums.fontysize>m->xsize) {
      /* right column */
      scrollbarpos(&x0, &xl, (long)(aprsdecode_lums.fontysize*2UL),
                (long)(m->ysize-aprsdecode_lums.fontysize*2UL), m->scry,
                (long)m->scrysize);
      if (ycl+aprsdecode_lums.fontysize*2UL>m->ysize) m->scry = 0L;
      else if (ycl<=aprsdecode_lums.fontysize*2UL) m->scry = 8192000L;
      else if (m->ysize-ycl<x0) {
         /* scroll winsize down */
         m->scry += -(long)((m->ysize-aprsdecode_lums.fontysize*4UL)*256UL);
      }
      else if (m->ysize-ycl<x0+xl) {
      }
      else {
         /* click on scrollbar */
         m->scry += (long)((m->ysize-aprsdecode_lums.fontysize*4UL)*256UL);
                /* scroll winsize up */
      }
   }
   /* click on text */
   useri_refresh = 1;
} /* end dopano() */

/* === panorama === */

static void escmenus(void)
{
   useri_killallmenus();
   closelist();
   useri_refresh = 1;
   configedit = 0UL;
   useri_nextmsg = 0L;
   sndmsg = 0;
   useri_beaconediting = 0;
   digied = 0;
   aprsdecode_lums.wxcol = 0;
   aprsdecode_lums.text = 10L*useri_conf2int(useri_fLTEXT, 0UL, 0L, 100L,
                100L);
   aprsdecode_click.withradio = 0;
   aprsdecode_click.onesymbol.tab = 0;
   if (panowin.on) closepano();
} /* end escmenus() */


static void copytoed(void)
{
   char s[1000];
   clreditline();
   useri_confstr((unsigned char)configedit, s, 1000ul);
   icfg(useri_fEDITLINE, s, 1000ul);
   if (s[0U]) useri_copypaste(s, 1000ul);
   useri_refresh = 1;
} /* end copytoed() */


static void filesetup(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==3UL) {
      if (sub==0UL) configtogg(useri_fLOGWFN);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Log Filename",31u);
         configedit = 1UL;
      }
   }
   else if (knob==2UL) {
      strncpy(configs[useri_fEDITLINE].title,"Delete Log After Days",31u);
      configs[useri_fEDITLINE].width = 4U;
      configedit = 2UL;
   }
   memcpy(configs[useri_fEDITLINE].title,configs[configedit].title,31u);
   copytoed();
   updatemenus();
} /* end filesetup() */


static void statuscol(maptool_pIMAGE img, long x0, long xwide,
                struct maptool_PIX c)
{
   long y;
   long x;
   struct maptool_PIX col;
   long tmp;
   long tmp0;
   tmp = (long)aprsdecode_lums.fontysize-2L;
   y = 1L;
   if (y<=tmp) for (;; y++) {
      tmp0 = xwide;
      x = -xwide;
      if (x<=tmp0) for (;; x++) {
         col = img->Adr[(x0+x)*img->Len0+y];
         col.r += c.r;
         col.g += c.g;
         col.b += c.b;
         img->Adr[(x0+x)*img->Len0+y] = col;
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end statuscol() */


static void knobcol(maptool_pIMAGE img, long x0, long y00, long xs, long ys,
                struct maptool_PIX c)
{
   long y;
   long x;
   struct maptool_PIX col;
   long tmp;
   long tmp0;
   tmp = (y00+ys)-1L;
   y = y00;
   if (y<=tmp) for (;; y++) {
      tmp0 = (x0+xs)-1L;
      x = x0;
      if (x<=tmp0) for (;; x++) {
         if (x<=(long)(img->Len1-1) && y<=(long)(img->Len0-1)) {
            col = img->Adr[(x)*img->Len0+y];
            col.r += c.r;
            col.g += c.g;
            col.b += c.b;
            img->Adr[(x)*img->Len0+y] = col;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end knobcol() */

typedef unsigned long sCONFSET[5];

static sCONFSET _cnst = {0x00000000UL,0x00000000UL,0x00807E80UL,0x07800000UL,
                0x003C0000UL};

static void configman(unsigned long button, char * keycmd)
{
   char s1[1001];
   char s[1001];
   if (button==1UL) configedit = 0UL;
   else {
      useri_confstr(useri_fEDITLINE, s, 1001ul);
      useri_confstr((unsigned char)configedit, s1, 1001ul);
      if (!aprsstr_StrCmp(s, 1001ul, s1, 1001ul)) {
         configs[configedit].updated = 1;
      }
      useri_AddConfLine((unsigned char)configedit, 1U, s, 1001ul);
      if ((unsigned char)configedit==useri_fFONTSIZE) maptool_loadfont();
      /*    IF VAL(CONFSET, configedit) IN sCONFSET{fFONTSIZE, fANT1, fANT2,
                fANT3, fFRESNELL,fGEOBRIGHTNESS, fGEOCONTRAST, fARROW,fKMH,
                fKMHTIME,fTEMP,fWINDSYM,fRULER,fALTMIN,fCOLMAPTEXT,
                fCOLOBJTEXT, fCOLMENUTEXT, fCOLMENUBACK, fCOLMARK1,
                fCOLMARK2} */
      if (X2C_INL((long)configedit,153,_cnst)) *keycmd = ' ';
      if (configs[configedit].typ<useri_cLIST) configedit = 0UL;
   }
   useri_killmenuid(229UL);
   useri_rdlums();
   updatemenus();
   useri_refresh = 1;
} /* end configman() */


extern char useri_gpsalt(unsigned char a)
{
   char h[100];
   useri_confstr(a, h, 100ul);
   return aprsstr_InStr(h, 100ul, "A", 2ul)>=0L || aprsstr_InStr(h, 100ul, "a\
", 2ul)>=0L;
} /* end gpsalt() */

static void dogeoprofil(pMENU);


static void dogeoprofil(pMENU m)
{
   char h[100];
   char s[100];
   long nn;
   float rv;
   struct maptool_PIX c;
   m->oldknob = 0UL;
   addline(m, "Colour Marker 2", 16ul, "\321>", 3ul, 8500UL);
   addline(m, "Colour Marker 1", 16ul, "\321>", 3ul, 8505UL);
   strncpy(s,"Use RAM MB  [",100u);
   aprsstr_IntToStr(useri_conf2int(useri_fSRTMCACHE, 0UL, 0L, 2000L, 100L),
                1UL, h, 100ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_Append(s, 100ul, "] MB", 5ul);
   addline(m, s, 100ul, "\321", 2ul, 8508UL);
   strncpy(s,"Brightness [",100u);
   aprsstr_IntToStr(useri_conf2int(useri_fGEOBRIGHTNESS, 0UL, 0L, 100L, 30L),
                 1UL, h, 100ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_Append(s, 100ul, "] %", 4ul);
   addline(m, s, 100ul, "\321", 2ul, 8510UL);
   strncpy(s,"Contrast    [",100u);
   aprsstr_IntToStr(useri_conf2int(useri_fGEOCONTRAST, 0UL, 0L, 10000000L,
                5L), 1UL, h, 100ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_Append(s, 100ul, "]", 2ul);
   addline(m, s, 100ul, "\321", 2ul, 8515UL);
   strncpy(s,"Refraction  [",100u);
   rv = useri_conf2real(useri_fREFRACT, 0UL, (-10.0f), 10.0f, 0.0f);
   aprsstr_FixToStr(rv, 3UL, h, 100ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   aprsstr_Append(s, 100ul, "]", 2ul);
   addline(m, s, 100ul, "\321", 2ul, 8518UL);
   strncpy(s,"Frequency   [",100u);
   rv = useri_conf2real(useri_fFRESNELL, 0UL, 0.0f, X2C_max_real, 0.0f);
   if (rv>=0.1f) {
      aprsstr_FixToStr(rv+0.0005f, 4UL, h, 100ul);
      aprsstr_Append(s, 100ul, h, 100ul);
   }
   aprsstr_Append(s, 100ul, "] MHz", 6ul);
   addline(m, s, 100ul, "\321", 2ul, 8520UL);
   strncpy(s,"Antenna 3   [",100u);
   nn = useri_conf2int(useri_fANT3, 0UL, -1000000L, 100000L, -1000000L);
   if (nn!=-1000000L) {
      aprsstr_IntToStr(nn, 1UL, h, 100ul);
      aprsstr_Append(s, 100ul, h, 100ul);
   }
   aprsstr_Append(s, 100ul, "] m", 4ul);
   addline(m, s, 100ul, "\321", 2ul, 8525UL);
   strncpy(s,"Antenna 2   [",100u);
   nn = useri_conf2int(useri_fANT2, 0UL, -1000000L, 100000L, -1000000L);
   if (nn!=-1000000L) {
      aprsstr_IntToStr(nn, 1UL, h, 100ul);
      aprsstr_Append(s, 100ul, h, 100ul);
   }
   aprsstr_Append(s, 100ul, "] m", 4ul);
   addline(m, s, 100ul, "\321", 2ul, 8527UL);
   strncpy(s,"Antenna 1   [",100u);
   nn = useri_conf2int(useri_fANT1, 0UL, -1000000L, 100000L, -1000000L);
   if (nn!=-1000000L) {
      aprsstr_IntToStr(nn, 1UL, h, 100ul);
      aprsstr_Append(s, 100ul, h, 100ul);
   }
   if (useri_gpsalt(useri_fANT1)) aprsstr_Append(s, 100ul, " +ALT", 6ul);
   aprsstr_Append(s, 100ul, "] m", 4ul);
   addline(m, s, 100ul, "\321", 2ul, 8529UL);
   strncpy(s,"   | Redraw |  Map On/Off",100u);
   nn = (long)configs[useri_fSRTMCACHE].on;
   c.r = 0U;
   c.g = 0U;
   c.b = 0U;
   if (nn==0L) {
      s[1U] = 'L';
      c.b = 800U;
      c.g = 100U;
   }
   else if (nn==1L) {
      s[1U] = 'M';
      c.g = 300U;
   }
   else {
      s[1U] = 'H';
      c.r = 500U;
   }
   addline(m, s, 100ul, "\022", 2ul, 8535UL);
   knobcol(m->image, 2L, (long)((m->oldknob-1UL)*m->yknob), 16L,
                (long)(m->yknob-1UL), c);
   m->redrawproc = dogeoprofil;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end dogeoprofil() */

#define useri_MAXBUTT0 11


static void geoprofil(void)
{
   pMENU m;
   newmenu(&m, 145UL, aprsdecode_lums.fontysize+7UL, 11UL, useri_bTRANSP);
   dogeoprofil(m);
   m->ysize = m->oldknob*m->yknob;
   m->oldknob = 0UL;
} /* end geoprofil() */


static void coloureditgeo(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==1UL) configedit = 151UL;
   else if (knob==2UL) configedit = 150UL;
   else if (knob==3UL) configedit = 123UL;
   else if (knob==4UL) configedit = 116UL;
   else if (knob==5UL) configedit = 117UL;
   else if (knob==6UL) configedit = 118UL;
   else if (knob==7UL) configedit = 119UL;
   else if (knob==8UL) configedit = 122UL;
   else if (knob==9UL) configedit = 121UL;
   else configedit = 120UL;
   memcpy(configs[useri_fEDITLINE].title,configs[configedit].title,31u);
   copytoed();
   useri_rdlums();
   updatemenus();
} /* end coloureditgeo() */

/* colour chooser */
static void docolours(pMENU);


static void docolours(pMENU m)
{
   m->oldknob = 0UL;
   addline(m, " Symbol Text", 13ul, "\316>", 3ul, 8405UL);
   addline(m, " Object/Item Text", 18ul, "\316>", 3ul, 8407UL);
   addline(m, " Menus Text", 12ul, "\316>", 3ul, 8410UL);
   addline(m, " Menus Background", 18ul, "\316>", 3ul, 8415UL);
   /*  addline(m, " Rf Range Marker 1", CMDCOLOURS+">", MINH*84+20); */
   /*  addline(m, " Rf Range Marker 2", CMDCOLOURS+">", MINH*84+25); */
   addline(m, " Brightness Rfpath", 19ul, "\316", 2ul, 8430UL);
   /*  addline(m, " Menu transparency", CMDCOLOURS, MINH*84+35); */
   addonoff(m, "   |Menu transparency", 22ul, "\316", 2ul, 8435UL, 8L,
                useri_configon(useri_fTRANSP));
   addline(m, " Font Size", 11ul, "\316", 2ul, 8440UL);
   m->redrawproc = docolours;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end docolours() */

/*
PROCEDURE colours;
VAR m:pMENU;
BEGIN
  newmenu(m, 115, lums.fontysize+7, 9, bTRANSP);
  docolours(m);
  m^.ysize:=m^.oldknob*m^.yknob+1;
  m^.oldknob:=0;
END colours;
*/

static void colouredit(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==7UL) configedit = 87UL;
   else if (knob==6UL) {
      if (sub==0UL) configtogg(useri_fTRANSP);
      else configedit = 54UL;
   }
   else if (knob==5UL) configedit = 61UL;
   else configedit = (146UL+knob)-1UL;
   memcpy(configs[useri_fEDITLINE].title,configs[configedit].title,31u);
   copytoed();
   useri_rdlums();
   updatemenus();
} /* end colouredit() */

#define useri_PI 3.1415926536


static float ARC(float re, float im)
{
   if (re==0.0f && im==0.0f) return 0.0f;
   else if ((float)fabs(re)>=(float)fabs(im)) {
      if (re>=0.0f) return RealMath_arctan(X2C_DIVR(im,re));
      else if (im>=0.0f) {
         return 3.1415926536f+RealMath_arctan(X2C_DIVR(im,re));
      }
      else return RealMath_arctan(X2C_DIVR(im,re))-3.1415926536f;
   }
   else if (im>=0.0f) return 1.5707963268f-RealMath_arctan(X2C_DIVR(re,im));
   else return (-1.5707963268f)-RealMath_arctan(X2C_DIVR(re,im));
   return 0;
} /* end ARC() */


static void normcol(unsigned long m, unsigned long * r, unsigned long * g,
                unsigned long * b)
/* make color tripple to max luma */
{
   unsigned long h;
   h = *r;
   if (*g>h) h = *g;
   if (*b>h) h = *b;
   if (h==0UL) h = 1UL;
   *r = aprsdecode_trunc(X2C_DIVR((float)( *r*m),(float)h)+0.5f);
   *g = aprsdecode_trunc(X2C_DIVR((float)( *g*m),(float)h)+0.5f);
   *b = aprsdecode_trunc(X2C_DIVR((float)( *b*m),(float)h)+0.5f);
} /* end normcol() */

#define useri_CMUL 1000

#define useri_S1 100

#define useri_PI0 3.1415926536


static char xytocolor(unsigned long x, unsigned long y, unsigned long * r,
                unsigned long * g, unsigned long * b, unsigned long * alias)
{
   float ra;
   float gr;
   float br;
   float rr;
   float h;
   float ya;
   float xa;
   xa = 2.0f*(X2C_DIVR((float)x,100.0f)-0.5f);
   ya = 2.0f*(X2C_DIVR((float)y,100.0f)-0.5f);
   ra = RealMath_sqrt(xa*xa+ya*ya);
   if (ra>1.0f) return 0;
   /* out of circle */
   h = ARC(xa, ya);
   if ((float)fabs(h)<=1.0471975512f) {
      rr = 0.5f-X2C_DIVR(h,2.0943951024f);
      br = 0.5f+X2C_DIVR(h,2.0943951024f);
      gr = 1.0f;
   }
   else {
      h = h-2.0943951024f;
      if ((float)fabs(h)<=1.0471975512f) {
         br = 1.0f;
         gr = 0.5f-X2C_DIVR(h,2.0943951024f);
         rr = 0.5f+X2C_DIVR(h,2.0943951024f);
      }
      else {
         h = h+4.1887902048f;
         rr = 1.0f;
         br = 0.5f-X2C_DIVR(h,2.0943951024f);
         gr = 0.5f+X2C_DIVR(h,2.0943951024f);
      }
   }
   *r = aprsdecode_trunc((1.0f-rr*ra)*1000.0f);
   *g = aprsdecode_trunc((1.0f-gr*ra)*1000.0f);
   *b = aprsdecode_trunc((1.0f-br*ra)*1000.0f);
   normcol(1000UL, r, g, b);
   h = (1.0f-ra)*3000.0f;
   if (h<100.0f) *alias = aprsdecode_trunc(h);
   else *alias = 100UL;
   return 1;
} /* end xytocolor() */

static void docolourchoose(pMENU);


static void docolourchoose(pMENU menu)
{
   unsigned long alias;
   unsigned long min0;
   unsigned long bo;
   unsigned long go;
   unsigned long ro;
   unsigned long bb;
   unsigned long gg;
   unsigned long rr;
   unsigned long d;
   unsigned long yo;
   unsigned long xo;
   unsigned long yc;
   unsigned long xc;
   unsigned long y;
   unsigned long x;
   struct aprsdecode_COLTYP col;
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0; /* set cursor */
   maptool_pIMAGE anonym1;
   maptool_pIMAGE anonym2;
   unsigned long tmp;
   unsigned long tmp0;
   menu->oldknob = 0UL;
   addline(menu, "", 1ul, "\317", 2ul, 8450UL);
   ro = (unsigned long)useri_conf2int((unsigned char)((146UL+menu->scroll)
                -1UL), 0UL, 0L, 100L, 0L);
   go = (unsigned long)useri_conf2int((unsigned char)((146UL+menu->scroll)
                -1UL), 1UL, 0L, 100L, 0L);
   bo = (unsigned long)useri_conf2int((unsigned char)((146UL+menu->scroll)
                -1UL), 2UL, 0L, 100L, 0L);
   normcol(1000UL, &ro, &go, &bo);
   xo = 0UL;
   yo = 0UL;
   min0 = X2C_max_longcard;
   tmp = menu->image->Len0-1;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = menu->image->Len1-1;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = (anonym1 = menu->image,
                &anonym1->Adr[(x)*anonym1->Len0+y]);
            if (((y<5UL || y>105UL) || x<5UL) || x>105UL) {
               anonym->r = 300U;
               anonym->g = 300U;
               anonym->b = 300U;
            }
            else {
               /*r:=0; g:=1; b:=TRANSPARENCE; */
               xc = x-5UL;
               yc = y-5UL;
               if (xytocolor(xc, yc, &rr, &gg, &bb, &alias)) {
                  d = (unsigned long)(labs((long)rr-(long)ro)+labs((long)
                gg-(long)go)+labs((long)bb-(long)bo));
                  if (d<min0) {
                     min0 = d; /* search cursor */
                     xo = x;
                     yo = y;
                  }
                  anonym->r = (unsigned short)((rr*alias)/100UL);
                  anonym->g = (unsigned short)((gg*alias)/100UL);
                  anonym->b = (unsigned short)((bb*alias)/100UL);
                  if (!(anonym->g&1)) ++anonym->g;
               }
               else {
                  anonym->r = 0U; /* out of circle */
                  anonym->g = 1U;
                  anonym->b = 65535U;
               }
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   if (xo<1UL) xo = 1UL;
   else if (xo>=(menu->image->Len1-1)-1UL) xo = (menu->image->Len1-1)-1UL;
   if (yo<1UL) yo = 1UL;
   else if (yo>=(menu->image->Len0-1)-1UL) yo = (menu->image->Len0-1)-1UL;
   tmp = yo+1UL;
   y = yo-1UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = xo+1UL;
      x = xo-1UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym0 = (anonym2 = menu->image,
                &anonym2->Adr[(x)*anonym2->Len0+y]);
            anonym0->r = 0U;
            anonym0->g = 1U;
            anonym0->b = 0U;
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   maptool_Colset(&col, 'W');
   maptool_drawstri(menu->image, configs[(menu->scroll-1UL)+146UL].title,
                31ul, 3L, (long)(((menu->image->Len0-1)-5UL)-aprsdecode_lums.fontysize),
                 1000UL, 1UL, col, 0, 0);
   menu->redrawproc = docolourchoose;
   menu->hiknob = 0UL;
} /* end docolourchoose() */


static void colourchoose(unsigned long knob)
{
   pMENU menu;
   newmenu(&menu, 110UL, 110UL+aprsdecode_lums.fontysize, 1UL, useri_bCOLOR);
   menu->scroll = knob;
   docolourchoose(menu);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end colourchoose() */


static void colourchoosegeo(unsigned long knob)
{
   pMENU menu;
   newmenu(&menu, 110UL, 110UL+aprsdecode_lums.fontysize, 1UL, useri_bCOLOR);
   if (knob==1UL) menu->scroll = 6UL;
   else menu->scroll = 5UL;
   docolourchoose(menu);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end colourchoosegeo() */


static void setcolour(unsigned long knob, unsigned long cx,
                unsigned long cy)
{
   unsigned long alias;
   unsigned long b;
   unsigned long g;
   unsigned long r;
   char h[101];
   char s[101];
   /*WrInt(knob, 10);WrInt(cx, 10); WrInt(cy, 10); */
   if (cx>=5UL) cx -= 5UL;
   else cx = 0UL;
   if (cy>=5UL+aprsdecode_lums.fontysize) {
      cy -= 5UL+aprsdecode_lums.fontysize;
   }
   else cy = 0UL;
   if (cx>100UL) cx = 100UL;
   if (cy>100UL) cy = 100UL;
   if (xytocolor(cx, 100UL-cy, &r, &g, &b, &alias)) {
      g = g/10UL;
      if (g>100UL) g = 100UL;
      b = b/10UL;
      if (b>100UL) b = 100UL;
      r = r/10UL;
      if (r>100UL) r = 100UL;
      aprsstr_CardToStr(r, 0UL, s, 101ul);
      aprsstr_Append(s, 101ul, " ", 2ul);
      aprsstr_CardToStr(g, 0UL, h, 101ul);
      aprsstr_Append(s, 101ul, h, 101ul);
      aprsstr_Append(s, 101ul, " ", 2ul);
      aprsstr_CardToStr(b, 0UL, h, 101ul);
      aprsstr_Append(s, 101ul, h, 101ul);
      icfg((unsigned char)((146UL+knob)-1UL), s, 101ul);
   }
   useri_rdlums();
   updatemenus();
} /* end setcolour() */

/* colour chooser */
static void netmenu(pMENU);

#define useri_OX2 8


static void netmenu(pMENU m)
{
   char s[4];
   char h[100];
   m->oldknob = 0UL;
   addonoff(m, "   |Allow Gate Rf>Net", 22ul, "\270", 2ul, 7445UL, 8L,
                useri_configon(useri_fALLOWGATE));
   addonoff(m, "    Allow Tx to Net", 20ul, "\270", 2ul, 7440UL, 8L,
                useri_configon(useri_fALLOWNETTX));
   addonoff(m, "    Connect Server", 19ul, "\270", 2ul, 7435UL, 8L,
                useri_configon(useri_fCONNECT));
   addline(m, " Serverfilter", 14ul, "\270", 2ul, 7430UL);
   addline(m, " Server url", 12ul, "\270", 2ul, 7425UL);
   addline(m, " Passcode", 10ul, "\270", 2ul, 7420UL);
   addline(m, " Netbeacontext", 15ul, "\270", 2ul, 7415UL);
   addline(m, " My Position", 13ul, "\270", 2ul, 7410UL);
   useri_confstr(useri_fMYSYM, s, 4ul);
   strncpy(h," My Symbol ",100u);
   aprsstr_Append(h, 100ul, s, 4ul);
   addline(m, h, 100ul, "\270>", 3ul, 7405UL);
   drawsymsquare(m->image, s[0U], s[1U], (long)(m->xsize-20UL),
                (long)((m->oldknob-1UL)*m->yknob+m->yknob/2UL));
   strncpy(h," My Call ",100u);
   useri_confappend(useri_fMYCALL, h, 100ul);
   addline(m, h, 100ul, "\270", 2ul, 7400UL);
   m->redrawproc = netmenu;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end netmenu() */

static void rfmenu(pMENU);

#define useri_OX3 9


static void rfmenu(pMENU m)
{
   m->oldknob = 0UL;
   /*  addline(m, " Query Keywords", CMDRF, MINH*75+60); */
   /*  addline(m, " Msg Destination", CMDRF, MINH*75+55); */
   /*  addline(m, " Msg Path", CMDRF, MINH*75+50); */
   addline(m, " Config Rx Messages", 20ul, "\271>", 3ul, 7550UL);
   addline(m, " Beacons Shift Time", 20ul, "\271", 2ul, 7545UL);
   addline(m, " Beacons + Objects", 19ul, "\271", 2ul, 7540UL);
   addline(m, " Digipeater", 12ul, "\271", 2ul, 7537UL);
   /*  addonoff(m, "   |Digipeater", CMDRF, MINH*75+47, OX, configon(fDIGI));
                 */
   addonoff(m, "   |Monitor Headline", 21ul, "\271", 2ul, 7535UL, 9L,
                useri_configon(useri_fWRTICKER));
   addonoff(m, "   |Call Check", 15ul, "\271", 2ul, 7530UL, 9L,
                useri_configon(useri_fMUSTBECALL));
   addonoff(m, "   |Serial Interface2", 22ul, "\271", 2ul, 7525UL, 9L,
                useri_configon(useri_fSERIALTASK2));
   addonoff(m, "   |Serial Interface1", 22ul, "\271", 2ul, 7520UL, 9L,
                useri_configon(useri_fSERIALTASK));
   addonoff(m, "   |RF-Port 4", 14ul, "\271", 2ul, 7515UL, 9L,
                useri_configon(useri_fUDP4));
   addonoff(m, "   |RF-Port 3", 14ul, "\271", 2ul, 7510UL, 9L,
                useri_configon(useri_fUDP3));
   addonoff(m, "   |RF-Port 2", 14ul, "\271", 2ul, 7505UL, 9L,
                useri_configon(useri_fUDP2));
   addonoff(m, "   |RF-Port 1", 14ul, "\271", 2ul, 7500UL, 9L,
                useri_configon(useri_fUDP1));
   m->redrawproc = rfmenu;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end rfmenu() */


static void dorfcfg(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==2UL) {
      strncpy(configs[useri_fEDITLINE].title,"Beacons Time Shift (s)",31u);
      configedit = 13UL;
   }
   else if (knob==3UL) {
      strncpy(configs[useri_fEDITLINE].title,"Beacons",31u);
      useri_beaconediting = 1;
      useri_beaconed = 1;
   }
   else if (knob==4UL) {
      strncpy(configs[useri_fEDITLINE].title,"Digi",31u);
      digied = 1;
   }
   else if (knob==5UL) {
      if (sub==0UL) configtogg(useri_fWRTICKER);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Calls in Headline",31u);
         configedit = 104UL;
      }
   }
   else if (knob==6UL) configtogg(useri_fMUSTBECALL);
   else if (knob==7UL) {
      if (sub==0UL) configtogg(useri_fSERIALTASK2);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Serial Interface2",31u);
         configedit = 29UL;
      }
   }
   else if (knob==8UL) {
      if (sub==0UL) configtogg(useri_fSERIALTASK);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Serial Interface1",31u);
         configedit = 28UL;
      }
   }
   else if (knob==9UL) {
      if (sub==0UL) configtogg(useri_fUDP4);
      else {
         strncpy(configs[useri_fEDITLINE].title,"RF-Port 4",31u);
         configedit = 39UL;
      }
   }
   else if (knob==10UL) {
      if (sub==0UL) configtogg(useri_fUDP3);
      else {
         strncpy(configs[useri_fEDITLINE].title,"RF-Port 3",31u);
         configedit = 38UL;
      }
   }
   else if (knob==11UL) {
      if (sub==0UL) configtogg(useri_fUDP2);
      else {
         strncpy(configs[useri_fEDITLINE].title,"RF-Port 2",31u);
         configedit = 37UL;
      }
   }
   else if (knob==12UL) {
      if (sub==0UL) configtogg(useri_fUDP1);
      else {
         strncpy(configs[useri_fEDITLINE].title,"RF-Port 1",31u);
         configedit = 36UL;
      }
   }
   copytoed();
   updatemenus();
} /* end dorfcfg() */

static void domsgmenu(pMENU);

#define useri_OX4 9


static void domsgmenu(pMENU m)
{
   m->oldknob = 0UL;
   addline(m, " Query Keywords", 16ul, "\254", 2ul, 8200UL);
   addline(m, " Msg Destination", 17ul, "\254", 2ul, 8205UL);
   addline(m, " Msg Path", 10ul, "\254", 2ul, 8210UL);
   addonoff(m, "    Msg Popup Window", 21ul, "\254", 2ul, 8215UL, 9L,
                useri_configon(useri_fPOPUPMSG));
   addonoff(m, "    Show all SSID Msg", 22ul, "\254", 2ul, 8220UL, 9L,
                useri_configon(useri_fMSGALLSSID));
   addonoff(m, "    Show Msg to Myself", 23ul, "\254", 2ul, 8225UL, 9L,
                useri_configon(useri_fPASSSELFMSG));
   m->redrawproc = domsgmenu;
   m->hiknob = 0UL;
} /* end domsgmenu() */


static void domsgcfg(unsigned long knob)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==1UL) {
      strncpy(configs[useri_fEDITLINE].title,"Query:Action",31u);
      configedit = 106UL;
   }
   else if (knob==2UL) {
      strncpy(configs[useri_fEDITLINE].title,"Msg Path",31u);
      configedit = 97UL;
      configs[useri_fEDITLINE].width = 9U;
   }
   else if (knob==3UL) {
      strncpy(configs[useri_fEDITLINE].title,"Msg Path",31u);
      configedit = 99UL;
   }
   else if (knob==4UL) configtogg(useri_fPOPUPMSG);
   else if (knob==5UL) configtogg(useri_fMSGALLSSID);
   else if (knob==6UL) configtogg(useri_fPASSSELFMSG);
   copytoed();
   updatemenus();
} /* end domsgcfg() */


static void msgmenu(void)
{
   pMENU menu;
   newmenu(&menu, 115UL, aprsdecode_lums.fontysize+7UL, 10UL, useri_bTRANSP);
   domsgmenu(menu);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end msgmenu() */


static void maploadtogg(void)
{
   char s[51];
   configtogg(useri_fGETMAPS);
   strncpy(s,"Map download ",51u);
   if (useri_configon(useri_fGETMAPS)) aprsstr_Append(s, 51ul, "On", 3ul);
   else aprsstr_Append(s, 51ul, "Off", 4ul);
   useri_textautosize(0L, 0L, 6UL, 4UL, 'b', s, 51ul);
} /* end maploadtogg() */


static void expandtogg(void)
{
   char s[51];
   configtogg(useri_fALLOWEXP);
   strncpy(s,"Allow Expand Maps ",51u);
   if (useri_configon(useri_fALLOWEXP)) aprsstr_Append(s, 51ul, "On", 3ul);
   else aprsstr_Append(s, 51ul, "Off", 4ul);
   useri_textautosize(0L, 0L, 6UL, 4UL, 'b', s, 51ul);
} /* end expandtogg() */

static void timercfg(pMENU);

#define useri_OX5 7


static void timercfg(pMENU m)
{
   m->oldknob = 0UL;
   addline(m, " Local time (+-h)", 18ul, "\273", 2ul, 7709UL);
   addline(m, " Nomove skip delay", 19ul, "\273", 2ul, 7708UL);
   addline(m, " Animation Speed", 17ul, "\273", 2ul, 7707UL);
   addline(m, " Duplicate lookback", 20ul, "\273", 2ul, 7702UL);
   addline(m, " Show km/h", 11ul, "\273", 2ul, 7703UL);
   addline(m, " Fade out time", 15ul, "\273", 2ul, 7704UL);
   addline(m, " Full Bright time", 18ul, "\273", 2ul, 7705UL);
   addline(m, " Data in RAM", 13ul, "\273", 2ul, 7700UL);
   addline(m, " Objects in RAM", 16ul, "\273", 2ul, 7701UL);
   addline(m, " Delay Start Mapload", 21ul, "\273", 2ul, 7745UL);
   addline(m, " Net Beacon time", 17ul, "\273", 2ul, 7706UL);
   m->redrawproc = timercfg;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end timercfg() */


static void dotimers(unsigned long knob)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==11UL) {
      strncpy(configs[useri_fEDITLINE].title,"Netbeacon time (s)",31u);
      configedit = 12UL;
   }
   else if (knob==10UL) {
      strncpy(configs[useri_fEDITLINE].title,"Delay Start Mapload (s)",31u);
      configedit = 53UL;
   }
   else if (knob==9UL) {
      strncpy(configs[useri_fEDITLINE].title,"Objects in RAM (Min)",31u);
      configedit = 52UL;
   }
   else if (knob==8UL) {
      strncpy(configs[useri_fEDITLINE].title,"Data in RAM (Min)",31u);
      configedit = 51UL;
   }
   else if (knob==7UL) {
      strncpy(configs[useri_fEDITLINE].title,"Fullbright time (Min)",31u);
      configedit = 50UL;
   }
   else if (knob==6UL) {
      strncpy(configs[useri_fEDITLINE].title,"Fade out time (Min)",31u);
      configedit = 49UL;
   }
   else if (knob==5UL) {
      strncpy(configs[useri_fEDITLINE].title,"Show km/h (s)",31u);
      configedit = 74UL;
   }
   else if (knob==4UL) {
      strncpy(configs[useri_fEDITLINE].title,"Dupes lookback (Min)",31u);
      configedit = 70UL;
   }
   else if (knob==3UL) {
      strncpy(configs[useri_fEDITLINE].title,"Animation Speed",31u);
      configedit = 81UL;
   }
   else if (knob==2UL) {
      strncpy(configs[useri_fEDITLINE].title,"Nomove skip delay",31u);
      configedit = 82UL;
   }
   else if (knob==1UL) {
      strncpy(configs[useri_fEDITLINE].title,"Local time (+-h)",31u);
      configedit = 105UL;
   }
   copytoed();
   updatemenus();
} /* end dotimers() */

static void mapcfg(pMENU);

#define useri_OX6 7


static void mapcfg(pMENU m)
{
   m->oldknob = 0UL;
   /*  addline(m, " Colours", CMDDOMAP+">", MINH*78+2); */
   /*  addline(m, " Menu transparency", CMDDOMAP, MINH*78+5); */
   /*  addline(m, " Font Size", CMDDOMAP, MINH*78+7); */
   /*  addline(m, " Brightness Rfpath", CMDDOMAP, MINH*78+10); */
   addline(m, " Reset to Default", 18ul, "\014", 2ul, 7810UL);
   addline(m, " Brightness notmover", 21ul, "\274", 2ul, 7815UL);
   addline(m, " Show Altitude min m", 21ul, "\274", 2ul, 7820UL);
   addonoff(m, "   |km/h text", 14ul, "\274", 2ul, 7825UL, 7L,
                useri_configon(useri_fKMH));
   addonoff(m, "   |Load Map Program", 21ul, "\274", 2ul, 7850UL, 7L,
                useri_configon(useri_fGETMAPS));
   addonoff(m, "    Show Temp", 14ul, "\274", 2ul, 7830UL, 7L,
                useri_configon(useri_fTEMP));
   addonoff(m, "    Show Windvane", 18ul, "\274", 2ul, 7835UL, 7L,
                useri_configon(useri_fWINDSYM));
   addonoff(m, "    Show Course Arrow", 22ul, "\274", 2ul, 7840UL, 7L,
                useri_configon(useri_fARROW));
   addonoff(m, "    Show Scaler", 16ul, "\274", 2ul, 7845UL, 7L,
                useri_configon(useri_fRULER));
   addonoff(m, "    Trackfilter", 16ul, "\274", 2ul, 7855UL, 7L,
                useri_configon(useri_fTRACKFILT));
   addonoff(m, "   |Show Loc of Mouse", 22ul, "\274", 2ul, 7860UL, 7L,
                useri_configon(useri_fMOUSELOC));
   m->redrawproc = mapcfg;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end mapcfg() */


extern void useri_resetimgparms(void)
{
   initconfig1();
   useri_rdlums();
   useri_Setmap(0UL);
   escmenus();
   aprsdecode_lums.headmenuy = 1;
   aprsdecode_lums.errorstep = 0;
   aprsdecode_lums.moving = 0;
   aprsdecode_posinval(&aprsdecode_click.markpos);
   aprsdecode_posinval(&aprsdecode_click.measurepos);
   aprsdecode_posinval(&aprsdecode_click.squerpos0);
   aprsdecode_click.waysum = 0.0f;
   useri_say("Reset most Image/Mouse Parameters to Default", 45ul, 4UL, 'b');
} /* end resetimgparms() */


static void domap(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   /*
     IF knob=RESETTODEFAULT THEN
       configs[fEDITLINE].title:="Menu transparency";
       configedit:=ORD(fTRANSP);
     ELSIF knob=3 THEN
       configs[fEDITLINE].title:="Font Size";
       configedit:=ORD(fFONTSIZE);
     ELSIF knob=4 THEN
       configs[fEDITLINE].title:="Brightness Rfpath";
       configedit:=ORD(fLRF);
   */
   /*
     IF knob=RESETTODEFAULT THEN
       initconfig1;
       rdlums;
       Setmap(0);
       escmenus;
       lums.headmenuy:=TRUE;
       lums.errorstep:=FALSE;
       lums.moving:=FALSE;
       posinval(click.markpos);
       posinval(click.measurepos);
       posinval(click.squerpos0);
       click.waysum:=0.0;
       say("Reset most Image/Mouse Parameters to Default", 4, "b");
   
     ELS*/
   if (knob==2UL) {
      strncpy(configs[useri_fEDITLINE].title,"Brightness notmover",31u);
      configedit = 62UL;
   }
   else if (knob==3UL) {
      strncpy(configs[useri_fEDITLINE].title,"Show Altitude min m",31u);
      configedit = 78UL;
   }
   else if (knob==4UL) {
      if (sub==0UL) configtogg(useri_fKMH);
      else {
         strncpy(configs[useri_fEDITLINE].title,"km/h text",31u);
         configedit = 73UL;
      }
   }
   else if (knob==5UL) {
      if (sub==0UL) maploadtogg();
      else {
         strncpy(configs[useri_fEDITLINE].title,"Start Maploader",31u);
         configedit = 86UL;
      }
   }
   else if (knob==6UL) configtogg(useri_fTEMP);
   else if (knob==7UL) configtogg(useri_fWINDSYM);
   else if (knob==8UL) configtogg(useri_fARROW);
   else if (knob==9UL) configtogg(useri_fRULER);
   else if (knob==10UL) configtogg(useri_fTRACKFILT);
   else if (knob==11UL) {
      if (sub==0UL) {
         configtogg(useri_fMOUSELOC);
         if (!useri_configon(useri_fMOUSELOC)) {
            useri_killmenuid(19UL);
            mouseshowcnt = 0UL;
         }
         else mouseshowcnt = 1UL;
      }
      else {
         strncpy(configs[useri_fEDITLINE].title,"show POIs from zoomlevel",
                31u);
         configedit = 67UL;
      }
   }
   copytoed();
   updatemenus();
} /* end domap() */

static void mapmove(pMENU);

#define useri_OX7 7


static void mapmove(pMENU m)
{
   m->oldknob = 0UL;
   addonoff(m, "   |Approx Warn km", 19ul, "\275", 2ul, 7900UL, 7L,
                useri_configon(useri_fAPPROXY));
   addonoff(m, "    Zoom as have Maps", 22ul, "\275", 2ul, 7905UL, 7L,
                useri_configon(useri_fZOOMMISS));
   addonoff(m, "    Expand Map ", 16ul, "\275", 2ul, 7910UL, 7L,
                useri_configon(useri_fALLOWEXP));
   addonoff(m, "    Swap map move dir", 22ul, "\275", 2ul, 7915UL, 7L,
                useri_configon(useri_fINVMOV));
   addline(m, " Bell/Sound", 12ul, "\275>", 3ul, 7917UL);
   addline(m, " Start Window x y", 18ul, "\275", 2ul, 7920UL);
   addline(m, " Center Watchcall", 18ul, "\275", 2ul, 7925UL);
   addline(m, " Default Zoom", 14ul, "\275", 2ul, 7930UL);
   addline(m, " Max Zoom", 10ul, "\275", 2ul, 7935UL);
   addline(m, " Map Move Step", 15ul, "\275", 2ul, 7940UL);
   addline(m, " Fine Zoom Step", 16ul, "\275", 2ul, 7945UL);
   m->redrawproc = mapmove;
   m->hiknob = 0UL;
   m->notoverdraw = 1;
} /* end mapmove() */


static void domapmove(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==1UL) {
      if (sub==0UL) configtogg(useri_fAPPROXY);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Approximation Warn km",31u);
         configedit = 134UL;
      }
   }
   else if (knob==2UL) configtogg(useri_fZOOMMISS);
   else if (knob==3UL) configtogg(useri_fALLOWEXP);
   else if (knob==4UL) configtogg(useri_fINVMOV);
   else if (knob==6UL) {
      strncpy(configs[useri_fEDITLINE].title,"Start Window x y",31u);
      configedit = 7UL;
   }
   else if (knob==7UL) {
      strncpy(configs[useri_fEDITLINE].title,"Center Watchcall",31u);
      configedit = 63UL;
   }
   else if (knob==8UL) {
      strncpy(configs[useri_fEDITLINE].title,"Default Zoom",31u);
      configedit = 80UL;
   }
   else if (knob==9UL) {
      strncpy(configs[useri_fEDITLINE].title,"Max Zoom",31u);
      configedit = 84UL;
   }
   else if (knob==10UL) {
      strncpy(configs[useri_fEDITLINE].title,"Map Move Step",31u);
      configedit = 89UL;
   }
   else if (knob==11UL) {
      strncpy(configs[useri_fEDITLINE].title,"Fine Zoom Step",31u);
      configedit = 90UL;
   }
   copytoed();
   updatemenus();
} /* end domapmove() */


static void mapadd(void)
{
   configs[useri_fEDITLINE].width = 200U;
   strncpy(configs[useri_fEDITLINE].title,"Mapname brightness",31u);
   configedit = 68UL;
   copytoed();
} /* end mapadd() */


static void mapdir(void)
{
   configs[useri_fEDITLINE].width = 200U;
   strncpy(configs[useri_fEDITLINE].title,"Map directory path",31u);
   configedit = 48UL;
   copytoed();
} /* end mapdir() */


static void maincfg(unsigned long knob, char fromstatus)
{
   pMENU menu;
   if (knob==11UL) potis();
   else {
      newmenu(&menu, 120UL, aprsdecode_lums.fontysize+3UL, 20UL,
                useri_bTRANSP);
      if (fromstatus) setunderbar(menu, useri_xmouse.x-10L);
      if (knob==4UL) {
         addline(menu, " Bad Paths Ports", 17ul, "\265", 2ul, 7301UL);
         /*      addonoff(menu, "   |Bad Paths Ports", CMDMANFILT, MINH*73+1,
                 7, configon(fFINGERPRINT)); */
         addline(menu, " Raw Frame", 11ul, "\265", 2ul, 7303UL);
         addline(menu, " Destination Call", 18ul, "\265", 2ul, 7300UL);
         addline(menu, " Word in Comment", 17ul, "\265", 2ul, 7305UL);
         addline(menu, " Sender of Object", 18ul, "\265", 2ul, 7310UL);
         addline(menu, " Via Igate Call", 16ul, "\265", 2ul, 7315UL);
         addline(menu, " Symbol Text", 13ul, "\265", 2ul, 7320UL);
      }
      else if (knob==5UL) docolours(menu);
      else if (knob==6UL) mapmove(menu);
      else if (knob==7UL) mapcfg(menu);
      else if (knob==8UL) timercfg(menu);
      else if (knob==9UL) rfmenu(menu);
      else if (knob==10UL) netmenu(menu);
      menu->ysize = menu->oldknob*menu->yknob;
      menu->oldknob = 0UL;
      menu->notoverdraw = 1;
   }
} /* end maincfg() */


static void callfiltsetup(unsigned long knob)
{
   clreditline();
   if (knob==1UL) {
      strncpy(configs[useri_fEDITLINE].title,"Bad Paths Ports",31u);
      configs[useri_fEDITLINE].width = 200U;
      configedit = 113UL;
      copytoed();
   }
   else if (knob==2UL) {
      strncpy(configs[useri_fEDITLINE].title,"Raw Frame",31u);
      configs[useri_fEDITLINE].width = 50U;
      configedit = 112UL;
   }
   else if (knob==3UL) {
      strncpy(configs[useri_fEDITLINE].title,"Destination Call",31u);
      configs[useri_fEDITLINE].width = 9U;
      configedit = 111UL;
   }
   else if (knob==4UL) {
      strncpy(configs[useri_fEDITLINE].title,"Word in Comment",31u);
      configs[useri_fEDITLINE].width = 50U;
      configedit = 107UL;
   }
   else if (knob==5UL) {
      strncpy(configs[useri_fEDITLINE].title,"Sender of Object",31u);
      configs[useri_fEDITLINE].width = 9U;
      configedit = 108UL;
   }
   else if (knob==6UL) {
      strncpy(configs[useri_fEDITLINE].title,"Via Igate Call",31u);
      configs[useri_fEDITLINE].width = 9U;
      configedit = 109UL;
   }
   else if (knob==7UL) {
      strncpy(configs[useri_fEDITLINE].title,"Call of Symbol",31u);
      configs[useri_fEDITLINE].width = 9U;
      configedit = 110UL;
   }
   useri_refresh = 1;
} /* end callfiltsetup() */


static void watchcallsetup(void)
{
   strncpy(configs[useri_fEDITLINE].title,"Watch Call",31u);
   configs[useri_fEDITLINE].width = 10U;
   configedit = 133UL;
   configs[useri_fEDITLINE].curspos = 0U;
   clreditline();
   useri_refresh = 1;
} /* end watchcallsetup() */

/*
PROCEDURE editonesymbol;
BEGIN
  configs[fEDITLINE].title:="Symbol Tab+Picture";
  configs[fEDITLINE].width:=3;
  configedit:=ORD(fONESYMB);
  copytoed;
  updatemenus;
  rdonesymb(TRUE);
END editonesymbol;
*/

static void onlinesetup(unsigned long knob, unsigned long sub)
{
   configedit = 0UL;
   configs[useri_fEDITLINE].width = 200U;
   if (knob==1UL) {
      if (sub==0UL) configtogg(useri_fALLOWGATE);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Rf Ports",31u);
         configs[useri_fEDITLINE].width = 4U;
         configedit = 35UL;
      }
      copytoed();
   }
   else if (knob==2UL) configtogg(useri_fALLOWNETTX);
   else if (knob==3UL) configtogg(useri_fCONNECT);
   else if (knob==4UL) {
      strncpy(configs[useri_fEDITLINE].title,"Server Filter",31u);
      configs[useri_fEDITLINE].width = 99U;
      configedit = 31UL;
      copytoed();
   }
   else if (knob==5UL) {
      strncpy(configs[useri_fEDITLINE].title,"Server URL",31u);
      configs[useri_fEDITLINE].width = 99U;
      configedit = 30UL;
      copytoed();
   }
   else if (knob==6UL) {
      strncpy(configs[useri_fEDITLINE].title,"Passcode",31u);
      configs[useri_fEDITLINE].width = 99U;
      configedit = 32UL;
      copytoed();
   }
   else if (knob==7UL) {
      strncpy(configs[useri_fEDITLINE].title,"Netbeacon Text",31u);
      configs[useri_fEDITLINE].width = 99U;
      configedit = 10UL;
      copytoed();
   }
   else if (knob==8UL) {
      strncpy(configs[useri_fEDITLINE].title,"My Position",31u);
      configs[useri_fEDITLINE].width = 22U;
      configedit = 9UL;
      copytoed();
   }
   else if (knob==10UL) {
      /*
        ELSIF knob=9 THEN
          configs[fEDITLINE].title:="Symbol";
          configs[fEDITLINE].width:=2;
          configedit:=ORD(fMYSYM);
      */
      strncpy(configs[useri_fEDITLINE].title,"My Call",31u);
      configs[useri_fEDITLINE].width = 9U;
      configedit = 8UL;
      copytoed();
   }
   if (knob==1UL || knob>3UL) copytoed();
   else configedit = 0UL;
   updatemenus();
} /* end onlinesetup() */


static void configdel(unsigned long knob)
{
   pMENU menu;
   newmenu(&menu, 42UL, aprsdecode_lums.fontysize+7UL, 2UL, useri_bTRANSP);
   menu->scroll = knob-2UL;
   addline(menu, "On/Off", 7ul, "\267", 2ul, 7695UL);
   addline(menu, "Edit", 5ul, "\267", 2ul, 7696UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end configdel() */


static void testbeep(unsigned char v)
{
   if (useri_configon(v)) {
      xosi_beep(100L, (unsigned long)useri_conf2int(v, 0UL, 20L, 8000L,
                1000L), (unsigned long)useri_conf2int(v, 1UL, 0L, 5000L,
                250L));
      useri_say("Testing beep now", 17ul, 1UL, 'b');
   }
} /* end testbeep() */


static void docfgbeep(unsigned long knob, unsigned long sub)
{
   configs[useri_fEDITLINE].width = 200U;
   if (knob==1UL) {
      if (sub==0UL) configtogg(useri_fBEEPPROX);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Beep far/near Hz ms Hz ms",
                31u);
         configedit = 126UL;
      }
      testbeep(useri_fBEEPPROX);
   }
   else if (knob==2UL) {
      if (sub==0UL) configtogg(useri_fBEEPWATCH);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Watchcall Beep Hz ms",31u);
         configedit = 127UL;
      }
      testbeep(useri_fBEEPWATCH);
   }
   else if (knob==3UL) {
      if (sub==0UL) configtogg(useri_fBEEPMSG);
      else {
         strncpy(configs[useri_fEDITLINE].title,"Message Beep rx/ack Hz ms Hz\
 ms",31u);
         configedit = 128UL;
      }
      testbeep(useri_fBEEPMSG);
   }
   copytoed();
   updatemenus();
} /* end docfgbeep() */

static void cfgbeep(pMENU);

#define useri_OX8 7


static void cfgbeep(pMENU m)
{
   m->oldknob = 0UL;
   addonoff(m, "   |Approxy Beep", 17ul, "\304", 2ul, 8010UL, 7L,
                useri_configon(useri_fBEEPPROX));
   addonoff(m, "   |Watchcall Beep", 19ul, "\304", 2ul, 8015UL, 7L,
                useri_configon(useri_fBEEPWATCH));
   addonoff(m, "   |Message Beep", 17ul, "\304", 2ul, 8020UL, 7L,
                useri_configon(useri_fBEEPMSG));
   m->redrawproc = cfgbeep;
   m->ysize = m->oldknob*m->yknob;
   m->hiknob = 0UL;
} /* end cfgbeep() */


static void configbeep(void)
{
   pMENU m;
   newmenu(&m, 100UL, aprsdecode_lums.fontysize+7UL, 3UL, useri_bTRANSP);
   cfgbeep(m);
   m->oldknob = 0UL;
} /* end configbeep() */

#define useri_XSIZE2 400

#define useri_YPOS0 240

#define useri_MLINES1 2
/* static menu lines */


static void configeditor(void)
{
   pMENU m;
   unsigned long xp;
   unsigned long i;
   unsigned long cnt;
   unsigned long xw;
   char ss[201];
   char s[201];
   aprsdecode_FILENAME fn;
   char cmd[2];
   pCONFLINE pl;
   pCONFLINE ph;
   char more;
   char onof;
   /*test:REAL; */
   unsigned long ohs;
   unsigned long hks;
   unsigned long oks;
   cnt = 0UL;
   pl = 0;
   if (configedit<=152UL) pl = configs[configedit].lines;
   ph = pl;
   while (ph) {
      ++cnt;
      ph = ph->next;
   }
   i = useri_mainys()/(aprsdecode_lums.fontysize+5UL);
   if (i<3UL) i = 1UL;
   else i -= 2UL;
   if (cnt>i) cnt = i;
   xw = 400UL;
   if (400UL>(unsigned long)maptool_xsize) xw = (unsigned long)maptool_xsize;
   m = findmenuid(229UL);
   refrmenu(&m, xw, aprsdecode_lums.fontysize+5UL, cnt+2UL, useri_bCOLOR, 0);
   oks = m->oldknob;
   hks = m->hiknob;
   ohs = m->oldsub;
   m->oldknob = 0UL;
   if (cnt<i) {
      if (configs[configedit].typ<useri_cLIST) {
         strncpy(s,"               Ok          |           Close",201u);
      }
      else strncpy(s,"              Add          |           Close",201u);
   }
   else strncpy(s," |                          Close",201u);
   addline(m, s, 201ul, "\263", 2ul, 7100UL);
   /*    IF cnt=1 THEN  AddConfLine(fEDITLINE, FALSE, pl^.line) END;  */
   /*    addline(m, 0C, " ", MINH*72); */
   addline(m, "", 1ul, "\320", 2ul, 7200UL);
   m->clampkb |= 0x4UL;
   m->confidx[2U] = 152U;
   keybknob(m, 2UL, 0, 0UL);
   m->oldknob = 2UL;
   more = configs[configedit].typ>=useri_cLIST;
   /*IF configedit=ORD(fWATCH) THEN more:=FALSE END; */
   onof = isbool(configs[configedit].typ);
   if (more) strncpy(cmd,"\266>",2u);
   else strncpy(cmd," ",2u);
   i = xw/6UL;
   while (cnt>0UL) {
      if (onof) strncpy(s,"\346    ",201u);
      else strncpy(s,"\346 ",201u);
      m->scroll = cnt;
      aprsstr_Append(s, 201ul, pl->line, 201ul);
      if (configedit==133UL) {
         aprsstr_Append(s, 201ul, "           ", 12ul);
         s[12U] = 0;
         strncpy(cmd,"\253",2u);
         aprsstr_Append(s, 201ul, "|", 2ul);
         if ((pl->active&1)) aprsstr_Append(s, 201ul, " +", 3ul);
         else aprsstr_Append(s, 201ul, "  ", 3ul);
         aprsstr_Append(s, 201ul, "Beep  |", 8ul);
         if ((pl->active/2U&1)) aprsstr_Append(s, 201ul, " +", 3ul);
         else aprsstr_Append(s, 201ul, "  ", 3ul);
         aprsstr_Append(s, 201ul, "Focus |", 8ul);
         if ((pl->active/4U&1)) aprsstr_Append(s, 201ul, " +", 3ul);
         else aprsstr_Append(s, 201ul, "  ", 3ul);
         aprsstr_Append(s, 201ul, "Log  |", 7ul);
         aprsstr_Assign(fn, 1024ul, pl->line, 201ul);
         aprsdecode_makelogfn(fn, 1024ul);
         if (useri_guesssize(fn, 1024ul, ss, 201ul)<0L) ss[0U] = 0;
         aprsstr_rightbound(ss, 201ul, 7UL);
         aprsstr_Append(s, 201ul, ss, 201ul);
         aprsstr_Append(s, 201ul, " |  Edit  |  Delete", 20ul);
      }
      /*      Append(s, " Log  |  Edit |  Delete"); */
      if (i>3UL && aprsstr_Length(s, 201ul)>=i) {
         s[i-3UL] = 0;
         aprsstr_Append(s, 201ul, "...", 4ul);
      }
      addline(m, s, 201ul, cmd, 2ul, 5800UL);
      if (onof) onoff(m, 8L, pl->active>0U);
      pl = pl->next;
      --cnt;
   }
   m->hiknob = hks;
   m->wid = 229UL;
   m->ysize = m->oldknob*m->yknob;
   i = m->oldknob;
   m->oldknob = oks;
   m->pullconf = useri_fMENUXYEDIT;
   m->pullyknob = 8UL;
   if (xw<(unsigned long)maptool_xsize) {
      xp = ((unsigned long)maptool_xsize-xw)/2UL;
   }
   else xp = 0UL;
   setmenupos(m, xp, 240UL);
   /*
       IF xw<VAL(CARDINAL,xsize) THEN m^.x0:=(VAL(CARDINAL,
                xsize)-xw) DIV 2 ELSE m^.x0:=0 END;
       m^.y0:=YPOS;
   */
   m->oldsub = ohs;
   if (m->hiknob>0UL && m->hiknob<=i) {
      if (X2C_IN(m->hiknob,31,m->clampkb)) keybknob(m, m->hiknob, 1, 0UL);
      inv(m, m->oldsub, m->hiknob);
   }
} /* end configeditor() */


static void watchdo(unsigned long knob, unsigned long sub)
{
   pCONFLINE pl;
   unsigned char s1;
   unsigned char s;
   aprsdecode_FILENAME h;
   aprsdecode_FILENAME fn;
   char ok0;
   knob -= 2UL;
   conflineno(useri_fWATCH, knob-1UL, 1, &pl);
   if (pl) {
      if (sub!=6UL) dellog = 0UL;
      if (sub>=1UL && sub<=3UL) {
         s = 1U<<sub-1UL;
         s1 = (unsigned char)pl->active;
         s = s^s1;
         pl->active = (unsigned char)s; /* toggle 3 bits */
      }
      else if (sub==4UL) {
         aprsstr_Assign(fn, 1024ul, pl->line, 201ul);
         aprsdecode_makelogfn(fn, 1024ul);
         if (useri_guesssize(fn, 1024ul, h, 1024ul)>=0L) {
            /* there is a logfile */
            dellog = knob;
            useri_say("Delele Logfile ? Click \'Delete\'", 32ul, 0UL, 'r');
         }
      }
      else if (sub==5UL) configdelman(useri_fWATCH, 2UL, knob);
      else if (sub==6UL) {
         if (dellog==knob) {
            aprsstr_Assign(fn, 1024ul, pl->line, 201ul);
            aprsdecode_makelogfn(fn, 1024ul);
            ok0 = 0;
            if (fn[0UL]) FileSys_Remove(fn, 1024ul, &ok0);
            if (ok0) {
               strncpy(h,"[",1024u);
               aprsstr_Append(h, 1024ul, fn, 1024ul);
               aprsstr_Append(h, 1024ul, "] deleted", 10ul);
            }
            else {
               strncpy(h,"cannot delete [",1024u);
               aprsstr_Append(h, 1024ul, fn, 1024ul);
               aprsstr_Append(h, 1024ul, "]", 2ul);
            }
            useri_say(h, 1024ul, 0UL, 'r');
         }
         else if (dellog==0UL) configdelman(useri_fWATCH, 3UL, knob);
         else useri_say("logfile not deleted", 20ul, 4UL, 'r');
         dellog = 0UL;
      }
   }
   useri_refresh = 1;
} /* end watchdo() */

static void configmain(pMENU);

#define useri_OX9 8


static void configmain(pMENU m)
{
   m->oldknob = 0UL;
   addonoff(m, "   |Save Config", 16ul, "\264", 2ul, 7000UL, 8L,
                useri_configon(useri_fAUTOSAVE));
   addline(m, " Reload Config", 15ul, "\264", 2ul, 7005UL);
   addline(m, " Watch Calls", 13ul, "\264", 2ul, 7010UL);
   addline(m, " Callfilters", 13ul, "\264>", 3ul, 7015UL);
   addline(m, " Colours, Font", 15ul, "\264>", 3ul, 7017UL);
   addline(m, " Map Moving", 12ul, "\264>", 3ul, 7020UL);
   addline(m, " Map Parameter", 15ul, "\264>", 3ul, 7025UL);
   addline(m, " Timers", 8ul, "\264>", 3ul, 7030UL);
   addline(m, " Rf-Ports", 10ul, "\264>", 3ul, 7035UL);
   addline(m, " Online", 8ul, "\264>", 3ul, 7040UL);
   addline(m, " Brightness", 12ul, "\264>", 3ul, 7045UL);
   m->redrawproc = configmain;
   m->hiknob = 0UL;
} /* end configmain() */


static void configmenu2(void)
{
   pMENU menu;
   newmenu(&menu, 90UL, aprsdecode_lums.fontysize+7UL, 20UL, useri_bTRANSP);
   configmain(menu);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   setunderbar(menu, 0L);
   menu->wid = 249UL;
   menu->notoverdraw = 1;
} /* end configmenu2() */


static void findopl(char folded)
{
   pMENU menu;
   /*  IF NOT folded THEN killallmenus END; */
   if (!folded) useri_killmenuid(0UL);
   newmenu(&menu, 220UL, aprsdecode_lums.fontysize+10UL, 1UL, useri_bCOLOR);
   addline(menu, "", 1ul, "\245", 2ul, 1990UL);
   menu->clampkb |= 0x2UL;
   menu->confidx[1U] = 0U;
   clampedline = 0UL;
   keybknob(menu, 1UL, 0, 0UL);
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
   if (!folded) {
      menu->x0 = (unsigned long)useri_xmouse.x;
      menu->y00 = (unsigned long)useri_xmouse.y;
   }
} /* end findopl() */


static void fotofn(void)
{
   pMENU menu;
   newmenu(&menu, 400UL, aprsdecode_lums.fontysize+7UL, 1UL, useri_bCOLOR);
   addline(menu, "", 1ul, "S", 2ul, 1800UL);
   menu->clampkb |= 0x2UL;
   menu->confidx[1U] = 6U;
   keybknob(menu, 1UL, 0, 0UL);
   menu->notoverdraw = 1;
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end fotofn() */


static char existslog(char fname[], unsigned long fname_len)
{
   aprsdecode_FILENAME fn;
   aprsdecode_FILENAME fnd;
   unsigned long t;
   char existslog_ret;
   X2C_PCOPY((void **)&fname,fname_len);
   aprsstr_Assign(fn, 1024ul, fname, fname_len);
   t = aprsdecode_realtime;
   memcpy(fnd,fn,1024u);
   aprstext_logfndate(t, fnd, 1024ul);
   if (X2C_STRCMP(fn,1024u,fnd,1024u)==0) {
      existslog_ret = FileSys_Exists(fnd, 1024ul);
      goto label;
   }
   /* no single day log */
   do {
      if (FileSys_Exists(fnd, 1024ul)) {
         existslog_ret = 1;
         goto label;
      }
      t -= 86400UL;
      memcpy(fnd,fn,1024u);
      aprstext_logfndate(t, fnd, 1024ul);
   } while (t>=1388534400UL);
   /* oldest possible file */
   existslog_ret = 0;
   label:;
   X2C_PFREE(fname);
   return existslog_ret;
} /* end existslog() */


static void importlog(pMENU menu)
{
   char redraw;
   char fn[4096];
   redraw = menu!=0;
   if (!redraw) {
      newmenu(&menu, 256UL, aprsdecode_lums.fontysize+7UL, 6UL,
                useri_bCOLOR);
      dellog = 0UL;
   }
   menu->oldknob = 0UL;
   addline(menu, "   First |   Back |   Load |   Forw |   Last", 45ul,
                "\216", 2ul, 6730UL);
   /*
     addline(menu, "          |           |            |", CMDRDLOG,
                MINH*67+30);
     subicon(menu^.image, 27, midy(menu), 1, 7);
     subicon(menu^.image, 35, midy(menu), 1, 7);
     subicon(menu^.image, 92, midy(menu), 1, 7);
     subicon(menu^.image, 256-92, midy(menu), 2, 7);
     subicon(menu^.image, 256-35, midy(menu), 2, 7);
     subicon(menu^.image, 256-27, midy(menu), 2, 7);
   */
   useri_confstr(useri_fLOGWFN, fn, 4096ul);
   if (aprsdecode_lums.logmode) {
      addline(menu, "      Exit Logview   |   Reload Writelog", 41ul, "\216",
                 2ul, 6720UL);
   }
   else if (dellog) {
      addline(menu, "  Not delete Writelog|     DELETE NOW!", 39ul, "\216",
                2ul, 6715UL);
   }
   else if (existslog(fn, 4096ul)) {
      addline(menu, "    Delete Writelog  |   Reload Writelog", 41ul, "\216",
                 2ul, 6715UL);
   }
   else {
      addline(menu, "\346\351\356    Delete Writelog  |   Reload Writelog",
                44ul, "\216", 2ul, 6715UL);
   }
   addline(menu, "", 1ul, "\216", 2ul, 6710UL);
   menu->clampkb |= 0x8UL;
   menu->confidx[3U] = 3U;
   keybknob(menu, 3UL, 0, 0UL);
   addline(menu, "", 1ul, "\216", 2ul, 6705UL);
   menu->clampkb |= 0x10UL;
   menu->confidx[4U] = 5U;
   keybknob(menu, 4UL, 0, 0UL);
   addline(menu, "", 1ul, "\216", 2ul, 6700UL);
   menu->clampkb |= 0x20UL;
   menu->confidx[5U] = 4U;
   keybknob(menu, 5UL, 0, 0UL);
   if (aprsdecode_lums.headmenuy && menu->y00<aprsdecode_lums.fontysize) {
      menu->y00 = aprsdecode_lums.fontysize;
   }
   menu->hiknob = 0UL;
   menu->wid = 227UL;
   menu->notoverdraw = 1;
   if (!redraw) {
      menu->ysize = menu->oldknob*menu->yknob;
      menu->oldknob = 0UL;
   }
} /* end importlog() */


extern void useri_refrlog(void)
{
   pMENU m;
   m = findmenuid(227UL);
   if (m) importlog(m);
} /* end refrlog() */


static float rt(unsigned long t)
{
   if (t>60UL) t = 60UL;
   return 1.0f-(float)t*1.6666666666667E-2f;
} /* end rt() */


static void statuscoly(maptool_pIMAGE img, long x0, unsigned long tx,
                unsigned long tr, char on)
{
   long in;
   long ir;
   long y;
   long x;
   float v;
   float un;
   float ur;
   float ut;
   struct maptool_PIX cr;
   struct maptool_PIX ct;
   struct maptool_PIX cn;
   struct maptool_PIX * anonym;
   long tmp;
   if (on) {
      cn.r = 300U;
      cn.g = 0U;
      cn.b = 0U;
      cr.r = 0U;
      cr.g = 300U;
      cr.b = 0U;
      ct.r = 550U;
      ct.g = 550U;
      ct.b = 0U;
   }
   else {
      cn.r = 0U;
      cn.g = 0U;
      cn.b = 0U;
      ct = cn;
      cr = cn;
   }
   ut = rt(tx);
   ur = rt(tr);
   if (ut>ur) un = ut;
   else un = ur;
   if (un!=0.0f) ur = X2C_DIVR(ur*un,ut+ur);
   v = (float)(aprsdecode_lums.fontysize-1UL);
   ir = (long)X2C_TRUNCI(ur*v,X2C_min_longint,X2C_max_longint);
   in = (long)X2C_TRUNCI(un*v,X2C_min_longint,X2C_max_longint);
   tmp = (long)aprsdecode_lums.fontysize-2L;
   y = 1L;
   if (y<=tmp) for (;; y++) {
      for (x = -5L; x<=5L; x++) {
         { /* with */
            struct maptool_PIX * anonym = &img->Adr[(x0+x)*img->Len0+y];
            if (y>in) {
               anonym->r += cn.r;
               anonym->g += cn.g;
               anonym->b += cn.b;
            }
            else if (y>ir) {
               anonym->r += ct.r;
               anonym->g += ct.g;
               anonym->b += ct.b;
            }
            else {
               anonym->r += cr.r;
               anonym->g += cr.g;
               anonym->b += cr.b;
            }
         }
      } /* end for */
      if (y==tmp) break;
   } /* end for */
} /* end statuscoly() */

/*
PROCEDURE timecolor(t:TIME; VAR c:PIX);
CONST M=300;
      T=60;
BEGIN
  IF t>T THEN t:=T END;
  t:=t*(M*2) DIV T;
  c.b:=0;
  IF t>M*2 THEN t:=M*2 END;
  IF t<=M THEN c.r:=t; c.g:=M
  ELSE c.r:=M; c.g:=M*2-t END
END timecolor;
*/
/*
PROCEDURE timecolor(t:TIME; VAR c:PIX);
                (* button background color from time *)
CONST M=300;
      T=60;
BEGIN
  IF t>T THEN t:=T END;
  c.r:=t*M DIV T;
  c.b:=c.r;
  c.g:=M;
END timecolor;
*/

static void tcpstat(void)
{
   char s[1001];
   s[0U] = 0;
   aprsdecode_tcpconnstat(s, 1001ul);
   useri_say(s, 1001ul, 4UL, 'b');
} /* end tcpstat() */


static void udpstat(unsigned long port)
{
   char s[1001];
   s[0U] = 0;
   aprsdecode_udpconnstat(port, s, 1001ul);
   useri_say(s, 1001ul, 4UL, 'b');
} /* end udpstat() */

#define useri_XKNOB 13

#define useri_KNOBS 9

#define useri_MAXEXTRALEN 84
/* dynamic message field width */

#define useri_XKNOBC 5
/* color field x */


static void statusbar(void)
{
   pMENU menu;
   char redraw;
   struct maptool_PIX c;
   unsigned long elen;
   unsigned long okn;
   unsigned long osub;
   unsigned long kx;
   unsigned char i;
   char e[100];
   char s[100];
   char ch;
   e[0U] = 0;
   if (aprsdecode_lums.wxcol=='R') strncpy(e,"Rain Map",100u);
   else if (aprsdecode_lums.wxcol=='W') strncpy(e,"Temp.Map",100u);
   else if (aprsdecode_lums.wxcol=='w') strncpy(e,"Wx Stations",100u);
   else if (aprsdecode_click.withradio) {
      /*    IF click.panorama THEN e:="Panorama" ELSE */
      if (aprsdecode_click.altimap) strncpy(e,"Geo Map",100u);
      else strncpy(e,"Radio Map",100u);
   }
   else if (aprsdecode_click.onesymbol.tab) {
      /*    END; */
      aprsstr_Assign(e, 100ul, "Symbol ", 8ul);
      aprsstr_Append(e, 100ul, (char *) &aprsdecode_click.onesymbol.tab,
                1u/1u);
      aprsstr_Append(e, 100ul, (char *) &aprsdecode_click.onesymbol.pic,
                1u/1u);
   }
   else if (aprsdecode_click.mhop[0UL]) {
      aprsstr_Assign(e, 100ul, aprsdecode_click.mhop, 9ul);
   }
   elen = aprsstr_Length(e, 100ul)*6UL;
   if (elen>0UL) {
      elen += 4UL;
      if (elen>84UL) elen = 84UL;
   }
   menu = findmenuid(254UL);
   redraw = menu!=0;
   /*IF redraw THEN hk:=menu^.hiknob ELSE hk:=0 END; */
   if (!redraw) {
      newmenu(&menu, 201UL, aprsdecode_lums.fontysize, 1UL, useri_bBLACK);
   }
   okn = menu->oldknob;
   osub = menu->oldsub;
   menu->oldknob = 0UL;
   menu->xsize = 117UL+elen;
   strncpy(s,"\346\360N\365|\3601\365|\3602\365|\3603\365|\3604\365|\360L\365\
|\360M\365|\360",100u);
   if (aprsdecode_maploadpid.runs) ch = 'd';
   else ch = 'e';
   if (useri_configon(useri_fGETMAPS)) ch = X2C_CAP(ch);
   aprsstr_Append(s, 100ul, (char *) &ch, 1u/1u);
   /*Append(s, SP6+"|"+SP1+"F"+SP6+"|"+SP1+"."+SP6); */
   aprsstr_Append(s, 100ul, "\365|\360F", 5ul);
   if (aprsdecode_lasttcprx==0UL) s[2U] = 'X';
   if (e[0U]) {
      aprsstr_Append(s, 100ul, "\364|", 3ul);
      aprsstr_Append(s, 100ul, e, 100ul);
   }
   addline(menu, s, 100ul, "\277", 2ul, 1700UL);
   /*
     IF configon(fCONNECT) THEN timecolor(realtime-lasttcprx, c);
     ELSE c.r:=0; c.g:=0; c.b:=0 END;
     statuscol(menu^.image, XKNOB DIV 2, c);
   */
   statuscoly(menu->image, 6L, aprsdecode_realtime-aprsdecode_lasttcptx,
                aprsdecode_realtime-aprsdecode_lasttcprx,
                useri_configon(useri_fCONNECT));
   kx = 0UL;
   for (i = useri_fUDP1; i<=useri_fUDP4; i++) {
      /*
          IF configon(i) THEN timecolor(realtime-udpsocks[kx].lastudprx, c);
          ELSE c.r:=0; c.g:=0; c.b:=0 END;
          statuscol(menu^.image, XKNOB DIV 2+XKNOB*(kx+1), c);
      */
      statuscoly(menu->image, (long)(6UL+13UL*(kx+1UL)),
                aprsdecode_realtime-aprsdecode_udpsocks0[kx].lastudptx,
                aprsdecode_realtime-aprsdecode_udpsocks0[kx].lastudprx,
                useri_configon(i));
      ++kx;
   } /* end for */
   ++kx;
   c.r = 0U;
   c.g = 0U;
   if (aprsdecode_lums.logmode) c.b = 700U;
   else c.b = 0U;
   statuscol(menu->image, (long)(6UL+13UL*kx), 5L, c);
   ++kx;
   if (aprsdecode_msgfifo0) c.r = 300U;
   else c.r = 0U;
   c.g = 0U;
   c.b = 0U;
   statuscol(menu->image, (long)(6UL+13UL*kx), 5L, c);
   ++kx;
   c.r = 0U;
   c.g = 0U;
   c.b = 0U;
   if (useri_isblown) {
      c.r = 400U;
   }
   else if (useri_configon(useri_fALLOWEXP)) c.g = 300U;
   statuscol(menu->image, (long)(6UL+13UL*kx), 5L, c);
   ++kx;
   if (useri_configon(useri_fTRACKFILT)) {
      c.g = 300U;
      c.b = 300U;
   }
   else {
      c.g = 0U;
      c.b = 0U;
   }
   c.r = 0U;
   statuscol(menu->image, (long)(6UL+13UL*kx), 5L, c);
   ++kx;
   /*
     IF click.mhop[0]<>0C THEN c.r:=600; c.g:=300 ELSE c.r:=0; c.g:=0 END;
                c.b:=0;
     statuscol(menu^.image, XKNOB DIV 2+XKNOB*kx,XKNOBC, c);
     INC(kx);
   */
   if (elen>0UL) {
      c.r = 300U;
      c.g = 100U;
      c.b = 0U;
      statuscol(menu->image, (long)(elen/2UL+13UL*kx), (long)(elen/2UL), c);
   }
   menu->wid = 254UL;
   menu->hiknob = 0UL;
   if (!redraw) {
      menu->ysize = menu->oldknob*menu->yknob;
      menu->oldknob = 0UL;
   }
   if (aprsdecode_lums.headmenuy) kx = 188UL;
   else kx = 0UL;
   menu->pullconf = useri_fMENUXYSTATUS;
   menu->pullyknob = 8UL;
   setmenupos(menu, (unsigned long)useri_conf2int(useri_fMENUXYSTATUS, 0UL,
                0L, 32767L, (long)kx),
                (unsigned long)useri_conf2int(useri_fMENUXYSTATUS, 1UL, 0L,
                32767L, 0L));
   /*  IF lums.headmenuy THEN menu^.x0:=TOOLBARX+1 ELSE menu^.x0:=0 END; */
   menu->oldknob = okn;
   menu->oldsub = osub;
/*  menu^.y0:=0; */
} /* end statusbar() */


static void setsym(unsigned long knob, unsigned long subknob,
                unsigned char set, pMENU pm, char dest)
{
   char s[3];
   if (findmenuid(203UL)==pm) {
      useri_confstr(set, s, 3ul);
      if (knob>=2UL && knob<=3UL) {
         if (subknob<=12UL) s[0U] = (char)(65UL+subknob+(3UL-knob)*13UL);
      }
      else if (knob==1UL && subknob<=9UL) s[0U] = (char)(48UL+subknob);
      useri_AddConfLine(set, 0U, s, 3ul);
   }
   else {
      useri_killmenuid(203UL);
      if (knob>6UL) s[0U] = '/';
      else s[0U] = '\\';
      s[1U] = (char)(32UL+((12UL-knob)%6UL)*16UL+subknob);
      s[2U] = 0;
      useri_AddConfLine(set, 0U, s, 3ul);
      if (s[0U]=='\\' && !aprsdecode_checksymb('A', s[1U])) {
         updatemenus();
         overlaychoose(dest);
      }
   }
   updatemenus();
} /* end setsym() */


static void downloadmenu(void)
{
   pMENU menu;
   if (maptool_mappack.run) {
      newmenu(&menu, 100UL, aprsdecode_lums.fontysize+7UL, 1UL,
                useri_bCOLOR);
      addline(menu, "Stop Download", 14ul, "\240", 2ul, 2300UL);
   }
   else {
      /*    IF NOT configon(fGETMAPS) */
      /*    THEN textautosize(0, 0, 20, 5, "r",
                'Enable "Load Map Program" in Config/Map Parameter') END;  */
      newmenu(&menu, 140UL, aprsdecode_lums.fontysize+7UL, 1UL,
                useri_bCOLOR);
      addline(menu, "", 1ul, "\237", 2ul, 2301UL);
      menu->clampkb |= 0x2UL;
      menu->confidx[1U] = 85U;
      keybknob(menu, 1UL, 0, 0UL);
   }
   menu->ysize = menu->oldknob*menu->yknob;
   menu->oldknob = 0UL;
} /* end downloadmenu() */


extern void useri_killbubble(void)
{
   useri_killmenuid(201UL);
} /* end killbubble() */

#define useri_H 8

#define useri_MARGIN 20


extern void useri_textbubble(struct aprspos_POSITION pos, char s[],
                unsigned long s_len, char last)
{
   pMENU menu;
   unsigned long i;
   unsigned long ym;
   unsigned long xm;
   unsigned long y;
   unsigned long x;
   unsigned long yw;
   unsigned long xw;
   long yp;
   long xp;
   float ypoi;
   float xpoi;
   struct aprsdecode_COLTYP col;
   struct maptool_PIX * anonym;
   maptool_pIMAGE anonym0;
   unsigned long tmp;
   unsigned long tmp0;
   if (!aprspos_posvalid(pos) || maptool_mapxy(pos, &xpoi, &ypoi)<0L) return;
   if (redrawimg==0) {
      osi_WrStrLn(s, s_len); /* no graphical window open */
      return;
   }
   i = 0UL;
   xw = 23UL;
   while (i<=s_len-1 && s[i]) {
      xw += maptool_charwidth0(s[i]); /* x size of text */
      ++i;
   }
   yw = aprsdecode_lums.fontysize+8UL;
   xp = (long)X2C_TRUNCI(xpoi,X2C_min_longint,X2C_max_longint)-1L;
   yp = ((long)X2C_TRUNCI(ypoi,X2C_min_longint,X2C_max_longint)+(long)yw)-1L;
   if (((xp<=0L || xp+(long)xw>(long)mainxs()) || yp<=0L) || yp+(long)
                yw>(long)useri_mainys()) return;
   allocmenu(&menu, xw, yw, 0);
   xm = menu->image->Len1-1;
   ym = menu->image->Len0-1;
   menu->xsize = xm+1UL;
   menu->ysize = ym+1UL;
   menu->wid = 201UL;
   menu->background = useri_bTRANSP;
   menu->x0 = (unsigned long)xp;
   menu->y00 = useri_mainys()-(unsigned long)yp;
   menu->nohilite = 0x7FFFFFFFUL;
   tmp = menu->ysize-1UL;
   y = 0UL;
   if (y<=tmp) for (;; y++) {
      tmp0 = menu->xsize-1UL;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         { /* with */
            struct maptool_PIX * anonym = (anonym0 = menu->image,
                &anonym0->Adr[(x)*anonym0->Len0+y]);
            if ((y==8UL && x>=8UL || x<8UL && x==y) || x<3UL && y<3UL) {
               anonym->r = 50U;
               anonym->g = 300U;
               anonym->b = 800U;
            }
            else {
               anonym->r = 0U;
               anonym->g = 0U;
               if (x<20UL || y<=8UL) anonym->b = 65535U;
               else {
                  anonym->r = 2U;
                  anonym->g = 2U;
                  anonym->b = 2U;
                  if (!aprsdecode_lums.menutransp) {
                     /* user disabled transparency */
                     anonym->r = (unsigned short)
                aprsdecode_lums.menubackcol.r;
                     anonym->g = (unsigned short)
                aprsdecode_lums.menubackcol.g;
                     anonym->b = (unsigned short)
                aprsdecode_lums.menubackcol.b;
                  }
               }
            }
         }
         if (x==tmp0) break;
      } /* end for */
      if (y==tmp) break;
   } /* end for */
   col.r = 40UL;
   col.g = 101UL;
   col.b = 254UL;
   if (last) {
      col.r = 200UL;
      col.g = 150UL;
      col.b = 80UL;
   }
   maptool_drawstri(menu->image, s, s_len, 20L, 7L, 700UL, 1UL, col, 1, 0);
   appendmenu(menu);
   useri_refresh = 1;
   menu->timeout = aprsdecode_realtime+5UL;
} /* end textbubble() */


static void mouseshow(unsigned long x, unsigned long y)
{
   char s[151];
   char h[21];
   struct aprspos_POSITION pos1;
   struct aprspos_POSITION pos;
   float dist;
   long alt;
   /*WrInt(panowin.hx,10); WrInt(panowin.hy,10); WrStrLn(" pan"); */
   struct maptool_PANOWIN * anonym;
   aprsdecode_posinval(&pos);
   dist = 0.0f;
   alt = -30000L;
   if (panowin.hover) {
      { /* with */
         struct maptool_PANOWIN * anonym = &panowin;
         maptool_findpanopos(panowin, &pos, &dist, &alt);
         anonym->hover = 0;
         if (!aprspos_posvalid(pos)) useri_killmenuid(19UL);
      }
   }
   else maptool_xytodeg((float)x, (float)(useri_mainys()-y), &pos);
   if (aprspos_posvalid(pos)) {
      maptool_xytoloc(pos, s, 151ul);
      useri_textautosize(-3L, 0L, 19UL, 10UL, 'g', s, 151ul);
      s[0U] = 0;
      pos1 = pos;
      if (dist!=0.0f || !poioff && aprsdecode_initzoom>=useri_conf2int(useri_fMOUSELOC,
                 0UL, 1L, 18L, 9L)) maptool_POIname(&pos, s, 151ul);
      if (dist!=0.0f) {
         /* pos from panorama */
         pos = pos1; /* not lock to POI on panorama */
         aprsstr_FixToStr(dist, 4UL, h, 21ul);
         aprsstr_Append(h, 21ul, "km", 3ul);
         if (s[0U]) aprsstr_Append(s, 151ul, " ", 2ul);
         aprsstr_Append(s, 151ul, h, 21ul);
      }
      if (alt>-20000L) {
         aprsstr_IntToStr(alt, 0UL, h, 21ul);
         aprsstr_Append(h, 21ul, "m NN", 5ul);
         if (s[0U]) aprsstr_Append(s, 151ul, " ", 2ul);
         aprsstr_Append(s, 151ul, h, 21ul);
      }
      if (s[0U]) {
         aprsdecode_click.bubblpos = pos;
         aprsdecode_click.lastpoi = 0;
         aprsstr_Assign(aprsdecode_click.bubblstr, 50ul, s, 151ul);
      }
   }
/*    textbubble(pos, s, FALSE); */
} /* end mouseshow() */


static void knoblamp(pMENU menu, unsigned long knob)
{
   char typ;
   unsigned long i;
   typ = menu->cmds[knob];
   for (i = 1UL; i<=30UL; i++) {
      if (typ==menu->cmds[i]) {
         onoffm(menu->image, 6L, (long)((i-1UL)*menu->yknob+menu->yknob/2UL),
                 i==knob);
      }
   } /* end for */
} /* end knoblamp() */


static void hidemenu(pMENU menu, char on)
{
   pMENU m;
   m = menus;
   while (m && m!=menu) {
      m->hidden = on;
      m = m->next;
   }
} /* end hidemenu() */


static void appbar(void)
{
   if (aprsdecode_lums.headmenuy && findmenuid(255UL)==0) toolbar();
/*  IF findmenuid(STATUSID)=NIL THEN statusbar(NIL) ELSE refrstatus; END; */
} /* end appbar() */


static void resizecursors(pMENU m, unsigned long px, unsigned long py)
{
   struct maptool_PANOWIN * anonym;
   if (m->wid==225UL) listwincursors(m, px, py);
   else if (m->wid==202UL) {
      listwincursors(m, px, py);
      { /* with */
         struct maptool_PANOWIN * anonym = &panowin;
         anonym->hover = 1;
         anonym->hx = (long)px;
         anonym->hy = (long)py;
         anonym->mx = useri_xmouse.x;
         anonym->my = useri_xmouse.y;
      }
   }
/*WrInt(px,10); WrInt(py,10); WrStrLn(" pan"); */
} /* end resizecursors() */


static void whichmenu(unsigned long px, unsigned long py, pMENU * m,
                unsigned long * knob, unsigned long * subknob,
                unsigned long * xpos, unsigned long * ypos)
{
   pMENU pm;
   struct MENU * anonym;
   /*WrInt(nowx, 5);  WrInt(nowy, 5); WrStrLn(" now"); */
   /*WrInt(px, 5);  WrInt(py, 5); WrStrLn(" m"); */
   *m = 0;
   pm = menus;
   *knob = 0UL;
   while (pm) {
      { /* with */
         struct MENU * anonym = pm;
         if (((((anonym->drawn && px>anonym->nowx)
                && px<anonym->nowx+anonym->xsize) && py>anonym->nowy+1UL)
                && py<anonym->nowy+anonym->ysize) && anonym->yknob>0UL) {
            *knob = ((anonym->nowy+anonym->ysize)-py)/anonym->yknob+1UL;
            /*WrInt(knob, 5); WrInt(ysize, 5); WrStrLn(" knb"); */
            *m = pm;
            *xpos = px-anonym->nowx;
            *ypos = py-anonym->nowy;
         }
      }
      pm = pm->next;
   }
   if (*knob>30UL) *knob = 30UL;
   *subknob = 0UL;
   if (*m && (*knob>0UL || (*m)->fullclamp)) {
      while ((*subknob<=20UL && (*m)->subk[*knob-1UL][*subknob]>0U)
                && *xpos>(unsigned long)(*m)->subk[*knob-1UL][*subknob]) {
         ++*subknob;
      }
      focuswid = (*m)->wid;
   }
   else focuswid = 0UL;
/*WrInt(subknob, 5); WrLn; */
} /* end whichmenu() */


static char hilitemenu(unsigned long px, unsigned long py, char kbdch,
                pMENU pm)
/* pm=NIL is find menu with x,y else use kbd up/down */
{
   unsigned long ki;
   unsigned long posy;
   unsigned long posx;
   unsigned long subknob;
   unsigned long knob;
   char c;
   char s[21];
   struct MENU * anonym;
   if (pm==0) {
      whichmenu(px, py, &pm, &knob, &subknob, &posx, &posy);
      if (pm && !X2C_IN(knob,31,pm->clampkb)) clampedline = 0UL;
   }
   else {
      knob = pm->oldknob;
      subknob = pm->oldsub;
      ki = knob;
      if (kbdch=='\030') {
         for (;;) {
            /* find next text line */
            if (ki==0UL) break;
            --ki;
            if (X2C_IN(ki,31,pm->clampkb)) {
               knob = ki;
               break;
            }
         }
      }
      else if (kbdch=='\005') {
         for (;;) {
            if (ki>=30UL) break;
            ++ki;
            if (X2C_IN(ki,31,pm->clampkb)) {
               knob = ki;
               break;
            }
         }
      }
   }
   if (pm) {
      /*WrInt(knob, 5);WrInt(subknob, 5); WrStrLn(" y"); */
      poioff = 1;
      useri_killbubble();
      useri_starthint((unsigned long)pm->helpindex[knob]+subknob, 0);
      if (pm->cmds[knob] && (knob!=pm->oldknob || subknob!=pm->oldsub)) {
         /*WrInt(knob, 5); WrInt(pm^.oldknob, 5);WrStrLn("km"); */
         killmenus(pm->next);
         pm->next = 0;
         if (pm->submen[knob]) {
            c = pm->cmds[knob];
            if (c=='c') mapchoose();
            else if (c=='i') {
               /*        ELSIF c=">" THEN potis; */
               infos();
            }
            else if (c=='F') findopl(1);
            else if (c=='K') {
               useri_confstr(useri_fCLICKMAP, s, 21ul);
               if (((s[0U]=='C' || s[0U]=='X') || s[0U]=='Y') || s[0U]=='2') {
                  mapclick();
               }
               else nextclick();
            }
            else if (c=='\276') {
               if (knob==1UL) hoverclick();
               else if (knob==2UL) mapclick();
               else if (knob==3UL) symbolclick('\203', useri_fCLICKTEXT);
               else if (knob==4UL) symbolclick('\202', useri_fCLICKSYM);
               else if (knob==5UL) wxclick();
               else if (knob==6UL) trackclick();
            }
            else if (c=='\220') toolsmenu();
            else if (c=='\217') filemenu();
            else if (c=='\222') listmen();
            else if (c=='\324') specialmap();
            else if (c=='\221') helpmenu();
            else if (c=='\233') fotofn();
            else if (c=='\235') importlog(0);
            else if (c=='\211') {
               /*        ELSIF c=CONFIGM THEN configmenu; */
               configmenu2();
            }
            else if (c=='\257') managemsg(pm->scroll, knob);
            else if (c=='\223') managebeacon(pm->scroll, knob, subknob, 1);
            else if (c=='\314') dodigi(pm->scroll, knob, subknob, 2U);
            else if (c=='\264') maincfg(knob, 0);
            else if (c=='\266') configdel(knob);
            else if (c=='\270') symchoose('\234');
            else if (c=='\325') {
               if (subknob>0UL) symchoose('\326');
            }
            else if (c=='\275') {
               if (knob==5UL) configbeep();
            }
            else if (c=='\271') {
               msgmenu();
            }
            else if (c=='\316') colourchoose(knob);
            else if (c=='\313') geoprofil();
            else if (c=='\321') colourchoosegeo(knob);
         }
         { /* with */
            struct MENU * anonym = pm;
            if (anonym->hiknob>0UL) {
               /* undo old hilite */
               inv(pm, anonym->oldsub, anonym->hiknob);
               if (X2C_IN(anonym->hiknob,31,anonym->clampkb)) {
                  keybknob(pm, anonym->hiknob, 0, 0UL);
               }
            }
            anonym->oldknob = knob;
            anonym->oldsub = subknob;
            anonym->hiknob = knob;
            if (X2C_IN(anonym->hiknob,31,anonym->clampkb)) {
               keybknob(pm, anonym->hiknob, 1, 0UL);
            }
            inv(pm, anonym->oldsub, anonym->hiknob);
         }
         hidemenu(0, 0);
         /*WrInt(ORD(pm^.confidx[knob]), 5);WrInt(ORD(pm^.scrolling), 5);
                WrInt(knob, 5);WrInt(pm^.helpindex[knob], 5);WrStrLn("=sc");
                */
         /*      hintnum:=pm^.helpindex[knob]+subknob; */
         /*      IF pm^.scrolling THEN INC(hintnum, VAL(CARDINAL,
                pm^.confidx[knob])*2) END; */
         /*WrInt(hintnum, 5);WrInt(subknob, 5);WrStrLn("=hint2"); */
         return 1;
      }
      resizecursors(pm, posx, posy);
      return 0;
   }
   poioff = 0;
   useri_starthint(0UL, 0);
   return 0;
} /* end hilitemenu() */

#define useri_HINTFN "hints.txt"

#define useri_EOL "|"


static void printhint(void)
{
   char s[1001];
   char buf[32768];
   long my;
   long m;
   long w;
   long len;
   long p;
   unsigned long n;
   long fd;
   char c;
   if (xosi_pulling || xosi_zooming) return;
   if (hintnum==1700UL) tcpstat();
   else if (hintnum>=1701UL && hintnum<=1704UL) udpstat(hintnum-1701UL);
   if (hinttime+2UL<=aprsdecode_realtime && hintnum>=100UL) {
      hintmouse = useri_xmouse;
      fd = osi_OpenRead("hints.txt", 10ul);
      if (osi_FdValid(fd)) {
         p = 0L;
         len = 0L;
         w = 0L;
         m = 0L;
         for (;;) {
            if (p>=len) {
               len = osi_RdBin(fd, (char *)buf, 32768u/1u, 32768UL);
               if (len<=0L) break;
               p = 0L;
            }
            c = buf[p];
            if (m==0L) {
               if (c=='|') {
                  m = 1L;
                  n = 0UL;
                  w = 0L;
               }
            }
            else if (m==1L) {
               if ((unsigned char)c>='0' && (unsigned char)c<='9') {
                  if (n<100000UL) {
                     n = (10UL*n+(unsigned long)(unsigned char)c)-48UL;
                  }
               }
               else m = 2L;
            }
            else if (n==hintnum) {
               if ((unsigned char)c>=' ' || c=='\012') {
                  if (c=='|') break;
                  if (w<1000L) {
                     s[w] = c;
                     ++w;
                  }
               }
            }
            else m = 0L;
            ++p;
         }
         if (w==0L) {
            aprsstr_IntToStr((long)hintnum, 1UL, s, 1001ul);
            aprsstr_Append(s, 1001ul, " hint text", 11ul);
            w = (long)aprsstr_Length(s, 1001ul);
         }
         else s[w] = 0;
         while (w>0L && (unsigned char)s[w-1L]<=' ') {
            --w; /* remove trailing junk */
            s[w] = 0;
         }
         my = useri_xmouse.y-20L;
         if (my<1L) my = 1L;
         if (w>0L) {
            useri_textautosize(useri_xmouse.x+10L, my, 220UL,
                12UL+aprsstr_Length(s, 1001ul)/15UL, 'h', s, 1001ul);
         }
         hinton = 1;
         useri_refresh = 1;
         osi_Close(fd);
         hintnum = 0UL;
      }
      else osi_WrStrLn("hints.txt not found", 20ul);
   }
} /* end printhint() */


static void closewxwins(void)
{
   unsigned char wi;
   for (wi = aprsdecode_wTEMP;; wi++) {
      useri_killmenuid(20UL+(unsigned long)wi);
      if (wi==aprsdecode_wSIEV) break;
   } /* end for */
} /* end closewxwins() */


static void closehint(void)
{
   if (hinton) {
      useri_killmenuid(220UL);
      hinton = 0;
      useri_refresh = 1;
   }
   useri_starthint(0UL, 0);
   if (hoveropen) {
      useri_killmenuid(7UL);
      closewxwins();
      hoveropen = 0;
      useri_refresh = 1;
   }
} /* end closehint() */

/*
PROCEDURE knoboff(k:CARDINAL; m:pMENU);
BEGIN 
  WITH configs[VAL(CONFSET, k)] DO
    IF bool THEN
      IF lines<>NIL THEN lines^.active:=FALSE END;
      fillconf(m, FALSE);
    END;
  END;
END knoboff;
*/

static pMENU FindClampMenu(void)
{
   pMENU pm;
   pm = menus; /* go thru menu chain till a clamp window */
   while (pm && (pm->wid!=focuswid || !(pm->fullclamp || X2C_IN(pm->oldknob,
                31,pm->clampkb)))) pm = pm->next;
   return pm;
} /* end FindClampMenu() */


static void kbtomenu(char * ch)
/* if keystroke to window return 0C */
{
   pMENU pm;
   unsigned long eot;
   unsigned long idx;
   unsigned long le;
   unsigned long i;
   char uml[10];
   /* & (configs[VAL(CONFSET,idx)].lines<>NIL)*/
   struct CONFIG * anonym;
   struct CONFLINE * anonym0;
   unsigned long tmp;
   aprsdecode_lums.moded = 1;
   pm = FindClampMenu();
   if (pm==0) return;
   if (pm->wid==225UL) {
      listcursmove(pm, ch);
      return;
   }
   clampedline = pm->oldknob; /* mousemove does not change menu line */
   if (*ch=='\005' || *ch=='\030') {
      if (hilitemenu(0UL, 0UL, *ch, pm)) {
         *ch = 0;
         clampedline = pm->oldknob;
         useri_refresh = 1;
      }
      return;
   }
   /*>line editor */
   uml[0U] = *ch;
   uml[1U] = 0;
   xosi_Umlaut(uml, 10ul);
   while ((unsigned char)uml[0U]>0) {
      *ch = uml[0U];
      idx = (unsigned long)pm->confidx[pm->oldknob];
      if (idx<=152UL) {
         { /* with */
            struct CONFIG * anonym = &configs[idx];
            if (anonym->lines==0) icfg((unsigned char)idx, "", 1ul);
            { /* with */
               struct CONFLINE * anonym0 = anonym->lines;
               eot = 0UL;
               while (eot<=200UL && anonym0->line[eot]) ++eot;
               i = eot;
               while (i<=200UL) {
                  anonym0->line[i] = ' ';
                  ++i;
               }
               le = 200UL;
               if (anonym->width>0U && 200UL>(unsigned long)anonym->width) {
                  le = (unsigned long)anonym->width;
               }
               if (le==0UL) le = 1UL;
               if ((unsigned long)anonym->curspos>=le) {
                  anonym->curspos = (unsigned short)(le-1UL);
               }
               if (*ch=='\023' || *ch=='\311') {
                  if (anonym->curspos>0U) --anonym->curspos;
               }
               else if (*ch=='\004' || *ch=='\310') {
                  ++anonym->curspos;
                  if ((unsigned long)anonym->curspos>=le) {
                     anonym->curspos = (unsigned short)(le-1UL);
                  }
               }
               else if (*ch=='\001') anonym->curspos = 0U;
               else if (*ch=='\002') anonym->curspos = (unsigned short)eot;
               else if (*ch=='\017') {
                  overtype = !overtype;
                  if ((unsigned char)anonym0->line[anonym->curspos]<=' ') {
                     while (anonym->curspos>0U && (unsigned char)
                anonym0->line[anonym->curspos-1U]<=' ') --anonym->curspos;
                  }
               }
               else if (*ch=='\010') {
                  if (anonym->curspos>0U) {
                     /*            knoboff(idx, pm); */
                     tmp = le-1UL;
                     i = (unsigned long)anonym->curspos;
                     if (i<=tmp) for (;; i++) {
                        anonym0->line[i-1UL] = anonym0->line[i];
                        if (i==tmp) break;
                     } /* end for */
                     anonym0->line[le-1UL] = ' ';
                     --anonym->curspos;
                  }
               }
               else if (*ch=='\177') {
                  /*          knoboff(idx, pm); */
                  tmp = le-1UL;
                  i = (unsigned long)(anonym->curspos+1U);
                  if (i<=tmp) for (;; i++) {
                     anonym0->line[i-1UL] = anonym0->line[i];
                     if (i==tmp) break;
                  } /* end for */
                  anonym0->line[le-1UL] = ' ';
               }
               if ((unsigned long)anonym->curspos>=le) {
                  anonym->curspos = (unsigned short)(le-1UL);
               }
               if ((unsigned char)*ch>=' ' && (unsigned char)*ch<'\177') {
                  /*          knoboff(idx, pm); */
                  if (!overtype) {
                     tmp = (unsigned long)(anonym->curspos+1U);
                     i = le-1UL;
                     if (i>=tmp) for (;; i--) {
                        anonym0->line[i] = anonym0->line[i-1UL];
                        if (i==tmp) break;
                     } /* end for */
                  }
                  anonym0->line[anonym->curspos] = *ch;
                  if ((unsigned long)anonym->curspos<le-1UL) {
                     ++anonym->curspos;
                  }
               }
               i = le-1UL;
               while (i>0UL && anonym0->line[i]==' ') {
                  anonym0->line[i] = 0;
                  --i;
               }
               if (anonym0->line[i]==' ') anonym0->line[i] = 0;
               anonym0->line[le] = 0;
            }
            if (idx==23UL || idx==26UL) beaconlen();
         }
      }
      aprsstr_Delstr(uml, 10ul, 0UL, 1UL);
   }
   /*>line editor */
   keybknob(pm, pm->oldknob, 1, 0UL);
   useri_refresh = 1;
   if (*ch=='\015' || *ch=='\012') {
      *ch = pm->cmds[pm->oldknob]; /*; WrInt(ORD(ch), 1);WrLn*/
   }
   else *ch = 0;
} /* end kbtomenu() */


static void op2watch(char add)
{
   char s[100];
   aprsdecode_pOPHIST op;
   op = aprsdecode_selop();
   if (op) {
      useri_AddConfLine(useri_fWATCH, add, op->call, 9ul);
      aprsstr_Assign(s, 100ul, op->call, 9ul);
      if (add) aprsstr_Append(s, 100ul, " added to Watchlist", 20ul);
      else aprsstr_Append(s, 100ul, " disabled in Watchlist", 23ul);
      useri_say(s, 100ul, 4UL, 'b');
   }
} /* end op2watch() */


static void deletop(void)
{
   char s[100];
   struct aprsdecode_CLICKOBJECT * anonym;
   if (aprsdecode_click.entries>0UL) {
      { /* with */
         struct aprsdecode_CLICKOBJECT * anonym = &aprsdecode_click.table[aprsdecode_click.selected]
                ;
         if (anonym->opf) {
            aprsstr_Assign(s, 100ul, anonym->opf->call, 9ul);
            if (testdelwaypoint()) {
               aprsdecode_delwaypoint(anonym->opf, &anonym->pff0);
               anonym->pff = anonym->pff0;
               if (anonym->pff0) {
                  textinfo(3UL);
                  aprsstr_Append(s, 100ul, " waypoint", 10ul);
               }
               else {
                  aprsdecode_click.entries = 0UL;
                  aprsdecode_click.mhop[0UL] = 0;
               }
            }
            else {
               anonym->opf->sym.tab = '\001';
               anonym->opf->sym.pic = 0;
               if (X2C_STRCMP(anonym->opf->call,9u,aprsdecode_click.mhop,
                9u)==0) aprsdecode_click.mhop[0UL] = 0;
               aprsdecode_click.entries = 0UL;
               aprsdecode_lastpurge = 0UL; /* starte a purge */
            }
            aprsstr_Append(s, 100ul, " Deleted", 9ul);
            useri_say(s, 100ul, 4UL, 'r');
         }
      }
      if (aprsdecode_click.entries==0UL) useri_killallmenus();
   }
} /* end deletop() */


static void redrawpop(maptool_pIMAGE mainimg)
/* join map and menus and write pixels to screen */
{
   X2C_ADDRESS xbufa;
   X2C_ADDRESS a;
   unsigned long xbufysize;
   unsigned long xbufxsize;
   unsigned long yym;
   unsigned long ym;
   unsigned long xl;
   unsigned long xm;
   unsigned long yof;
   unsigned long xof;
   unsigned long y;
   unsigned long x;
   unsigned long inca;
   pMENU menu;
   maptool_pIMAGE img;
   unsigned short bm;
   unsigned short gm;
   unsigned short rm;
   unsigned short bh;
   unsigned short gh;
   unsigned short rh0;
   char transp;
   /*        IF x+xof<xbufxsize THEN */
   struct maptool_PIX * anonym;
   struct maptool_PIX * anonym0;
   struct MENU * anonym1;
   unsigned long tmp;
   if (mainimg==0) return;
   xosi_getscreenbuf(&xbufa, &xbufxsize, &xbufysize, &inca);
   img = mainimg;
   xof = 0UL;
   yof = 0UL;
   xm = img->Len1-1;
   ym = img->Len0-1;
   transp = 0;
   if (panowin.on) {
      startpano();
      makepanwin();
   }
   if (useri_listwin=='M') makelistwin(&monbuffer);
   else if (useri_listwin=='L') makelistwin(&listbuffer);
   else if (useri_configon(useri_fMENUMONICON)) {
      startmon(0);
      makelistwin(&monbuffer);
   }
   if (sndmsg) {
      /*configedit:=0;*/
      sendmsg();
   }
   if (useri_nextmsg) msgpop();
   if (useri_beaconediting) beaconeditor();
   if (configedit) configeditor();
   if (digied) digieditor();
   appbar();
   statusbar();
   menu = menus;
   while (menu) {
      if (menu->timeout>0UL && menu->timeout<aprsdecode_realtime) {
         killmenu(menu);
         menu = menus;
      }
      else menu = menu->next;
   }
   menu = menus;
   while (menu && menu->hidden) menu = menu->next;
   while (img) {
      tmp = ym;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         if (y+yof<xbufysize) {
            a = (X2C_ADDRESS)((char *)xbufa+(long)(((y+yof)*xbufxsize+xof)
                *inca));
            yym = ym-y;
            xl = xm+1UL;
            if (xl+xof>=xbufxsize) {
               if (xbufxsize>=xof) xl = xbufxsize-xof;
               else xl = 0UL;
            }
            x = 0UL;
            while (x<xl) {
               { /* with */
                  struct maptool_PIX * anonym = &img->Adr[(x)*img->Len0+yym];
                
                  if (anonym->b!=65535U) {
                     rh0 = anonym->r;
                     gh = anonym->g;
                     bh = anonym->b;
                     if (transp && bh>0U) {
                        { /* with */
                           struct maptool_PIX * anonym0 = &mainimg->Adr[(x+xof)
                *mainimg->Len0+((mainimg->Len0-1)-(y+yof))];
                           rm = anonym0->r;
                           gm = anonym0->g;
                           bm = anonym0->b;
                           if ((unsigned long)
                rm>=aprsdecode_lums.menubackcol.r) {
                              rm = (unsigned short)
                aprsdecode_lums.menubackcol.r;
                           }
                           if ((unsigned long)
                gm>=aprsdecode_lums.menubackcol.g) {
                              gm = (unsigned short)
                aprsdecode_lums.menubackcol.g;
                           }
                           if ((unsigned long)
                bm>=aprsdecode_lums.menubackcol.b) {
                              bm = (unsigned short)
                aprsdecode_lums.menubackcol.b;
                           }
                           rh0 += rm/2U;
                           gh += gm/2U;
                           bh += bm/2U;
                        }
                     }
                     if (bh>=1024U) bh = 1023U;
                     if (rh0>=1024U) rh0 = 1023U;
                     if (gh>=1024U) {
                        gh = 1023U;
                     }
                     /*
                                     IF mainwin.bitperpixel<=16
                                     THEN xbuf016^[a]:=CAST(XPIX16,
                gammalut[rh].r+gammalut[gh].g+gammalut[bh].b);
                                     ELSE xbuf032^[a]:=CAST(XPIX32,
                gammalut[rh].r+gammalut[gh].g+gammalut[bh].b) END; 
                     */
                     if (inca==4UL) {
                        *(unsigned long *)a = (unsigned long)
                (xosi_gammalut[rh0].r|xosi_gammalut[gh].g|xosi_gammalut[bh]
                .b);
                        a = (X2C_ADDRESS)((char *)a+(long)4UL);
                     }
                     else {
                        *(unsigned short *)a = (unsigned short)
                (xosi_gammalut[rh0].r|xosi_gammalut[gh].g|xosi_gammalut[bh]
                .b);
                        a = (X2C_ADDRESS)((char *)a+(long)2UL);
                     }
                  }
                  else a = (X2C_ADDRESS)((char *)a+(long)inca);
               }
               /*        END; */
               ++x;
            }
         }
         if (y==tmp) break;
      } /* end for */
      if (menu) {
         { /* with */
            struct MENU * anonym1 = menu;
            img = anonym1->image;
            xof = anonym1->x0;
            yof = anonym1->y00;
            if (anonym1->wid==0UL && yof<aprsdecode_lums.fontysize) {
               /*(yof<>0) &*/
               yof = aprsdecode_lums.fontysize; /* make it better */
            }
            if (anonym1->xsize>0UL) xm = anonym1->xsize-1UL;
            else xm = img->Len1-1;
            if (anonym1->ysize>0UL) ym = anonym1->ysize-1UL;
            else ym = img->Len0-1;
            if (xof+xm>xbufxsize && xm<xbufxsize) xof = xbufxsize-xm;
            if (xof+xm>xbufxsize) {
               if (xm>xbufxsize) xof = 0UL;
               else xof = xbufxsize-xm;
            }
            if (yof+ym>xbufysize) {
               if (ym>xbufysize) yof = 0UL;
               else yof = xbufysize-ym;
            }
            anonym1->nowx = xof;
            anonym1->nowy = yof;
            transp = anonym1->background==useri_bTRANSP && aprsdecode_lums.menutransp;
            anonym1->drawn = 1;
         }
         menu = menu->next;
      }
      else img = 0;
   }
   xosi_rgbtoscreen();
   useri_refresh = 0;
} /* end redrawpop() */


extern void useri_redraw(maptool_pIMAGE img)
{
   redrawimg = img;
   redrawpop(redrawimg);
} /* end redraw() */


static void Pullmenu(unsigned long wid, long dx, long dy)
{
   pMENU pm;
   /*WrInt(dx, 10); WrInt(dy, 10); WrLn; */
   pm = findmenuid(wid);
   if ((pm && (unsigned long)pm->pullconf>0UL) && !Scrollbars(pm, dx, dy)) {
      movewin(pm, dx, dy);
      useri_refresh = 1;
   }
} /* end Pullmenu() */


static void wxraw(unsigned char set)
{
   char s[100];
   long r;
   useri_confstr(set, s, 100ul);
   r = aprsstr_InStr(s, 100ul, "u", 2ul);
   if (r<0L) aprsstr_Append(s, 100ul, "u", 2ul);
   else aprsstr_Delstr(s, 100ul, (unsigned long)r, 1UL);
   icfg(set, s, 100ul);
} /* end wxraw() */


static void listusermsg(void)
{
   aprsdecode_pOPHIST op;
   op = aprsdecode_selop();
   if (op) {
      startlist("Messages", 9ul, op->call, 9ul);
      aprstext_listtyps('M', 0, op->call, 9ul);
   }
} /* end listusermsg() */


static void setmarks(char marker1, char frommap)
{
   aprsdecode_pFRAMEHIST pf;
   if (frommap) {
      /* take position from last click */
      if (aprspos_posvalid(aprsdecode_click.clickpos)) {
         if (marker1) {
            aprstext_setmark1(aprsdecode_click.clickpos, 1, X2C_max_longint,
                0UL);
         }
         else aprsdecode_click.measurepos = aprsdecode_click.clickpos;
      }
   }
   else if (aprsdecode_click.entries>0UL) {
      /* take position and alt from clicked object */
      pf = aprsdecode_click.table[aprsdecode_click.selected].pff0;
      if (marker1) {
         aprstext_setmarkalti(pf,
                aprsdecode_click.table[aprsdecode_click.selected].opf, 1);
      }
      else if (pf && aprspos_posvalid(pf->vardat->pos)) {
         aprsdecode_click.measurepos = pf->vardat->pos;
      }
      else if (aprsdecode_click.table[aprsdecode_click.selected].opf) {
         aprsdecode_click.measurepos = aprsdecode_click.table[aprsdecode_click.selected]
                .opf->lastpos;
      }
   }
} /* end setmarks() */


static void mouseleft(long mousx, long mousy)
{
   pMENU menu;
   unsigned long poty;
   unsigned long potx;
   unsigned long subknob;
   unsigned long knob;
   char c;
   if (mousx<0L) mousx = 0L;
   if (mousy<0L) mousy = 0L;
   /*  actmenu:=NIL; */
   /*  refresh:=FALSE; */
   useri_starthint(0UL, 0);
   useri_killbubble();
   whichmenu((unsigned long)mousx, (unsigned long)mousy, &menu, &knob,
                &subknob, &potx, &poty);
   /*WrInt(ORD(menu<>NIL), 2); WrInt(knob, 10); WrInt(potx, 10);
                WrInt(poty, 10);WrStrLn(" pxy"); */
   if (menu==0) {
      aprsdecode_click.x = mousx;
      aprsdecode_click.y = (long)(useri_mainys()-(unsigned long)mousy);
      aprsdecode_click.selected = 0UL;
      /*WrInt(click.entries, 10);WrStrLn(" ent"); */
      if (((aprsdecode_click.cmd=='A' || aprsdecode_click.cmd=='a')
                || aprsdecode_click.cmd=='v')
                || aprsdecode_click.cmd=='\312') {
         if (aprsdecode_click.cmd!='\312') abortpop();
         aprsdecode_click.cmd = 'v';
         useri_refresh = 1;
      }
      else if (aprsdecode_click.x<=5L) aprsdecode_click.cmd = '\023';
      else if (aprsdecode_click.y<=5L) aprsdecode_click.cmd = '\030';
      else if (aprsdecode_click.x>(long)(mainxs()-5UL)) {
         aprsdecode_click.cmd = '\004';
      }
      else if (aprsdecode_click.y>(long)(useri_mainys()-5UL)) {
         aprsdecode_click.cmd = '\005';
      }
      else {
         useri_refresh = 1;
         aprsdecode_click.dryrun = 1;
      }
   }
   else {
      /*WrInt(menu^.wid, 1); WrStrLn("=wid"); */
      /*WrInt(knob, 10); WrInt(potx, 10); WrStrLn(" kl"); */
      /*    beaconimported[0]:=0C; */
      keybknob(menu, knob, 1, potx);
      c = menu->cmds[knob];
      /*WrInt(ORD(c), 5); WrInt(subknob, 5);WrStrLn(" cmd1"); */
      if (menu->wid==225UL) dolist(menu, potx, poty);
      else if (menu->wid==202UL) dopano(menu, potx, poty);
      else if (c=='t') {
         /*WrInt(ORD(c), 5); WrInt(subknob, 5);WrStrLn(" cmd"); */
         aprsdecode_click.cmd = c;
      }
      else if (c=='\322') aprsdecode_click.cmd = c;
      else if (c=='?') {
         knoblamp(menu, knob);
         aprsdecode_click.selected = aprsdecode_click.entries-knob;
         aprsdecode_click.cmd = ' ';
         updatemenus();
      }
      else if (c=='\006') {
         potimove(menu, potx, 10UL);
         useri_refresh = 1;
      }
      else if (c=='!') {
         potimove(menu, potx, knob);
         aprsdecode_click.cmd = ' ';
         useri_refresh = 1;
      }
      else if (c=='e') {
         killmenus(menu->next);
         menu->next = 0;
         if (subknob==1UL) {
            aprsdecode_lums.errorstep = !aprsdecode_lums.errorstep;
         }
         else textinfo((unsigned long)(subknob>0UL));
         aprsdecode_click.cmd = 'e';
         aprsdecode_click.dryrun = 0;
         updatemenus();
         hidemenu(menu, 1);
         useri_refresh = 1;
      }
      else if (((((((((c=='b' || c=='s') || c=='w') || c=='p') || c=='d')
                || c=='j') || c=='r') || c=='l') || c=='h') || c=='n') {
         images(aprsdecode_selop(), c, 0U);
         if (c=='n') aprsdecode_click.graphset |= 0x200U;
         else if (c=='s') {
            aprsdecode_click.graphset |= 0x80U;
         }
         useri_refresh = 1;
      }
      else if (c=='\005') {
         if (subknob==0UL) aprsdecode_click.cmd = '\010';
         else if (subknob==1UL) aprsdecode_click.cmd = '\005';
         else {
            aprsdecode_click.cmd = 0;
            useri_killallmenus();
         }
         aprsdecode_click.dryrun = 0;
         useri_refresh = 1;
      }
      else if (c=='\023') {
         if (subknob==0UL) aprsdecode_click.cmd = '\023';
         else if (subknob==1UL) aprsdecode_click.cmd = 'c';
         else aprsdecode_click.cmd = '\004';
         aprsdecode_click.dryrun = 0;
         useri_refresh = 1;
      }
      else if (c=='\030') {
         if (subknob==0UL) aprsdecode_click.cmd = '-';
         else if (subknob==1UL) aprsdecode_click.cmd = '\030';
         else aprsdecode_click.cmd = '+';
         aprsdecode_click.dryrun = 0;
         useri_refresh = 1;
      }
      else if (c=='0') {
         if (subknob==0UL) aprsdecode_click.watchmhop = 1;
         else {
            aprsdecode_click.cmd = c;
            useri_killallmenus();
         }
      }
      else if (c=='.') {
         if (subknob==0UL) {
            /*      IF click.mhop[0]
                =0C THEN click.cmd:=CMD1USER ELSE click.watchmhop:=TRUE END;
                */
            aprsdecode_click.cmd = '.';
         }
         else aprsdecode_click.cmd = '=';
         useri_killallmenus();
      }
      else if (c=='a') {
         aprsdecode_click.cmd = 'a';
         useri_killallmenus();
      }
      else if (c=='\330') {
         if (subknob==0UL) {
            aprsdecode_click.cmd = 'h';
            useri_killallmenus();
         }
         else {
            msgtoobj();
            useri_refresh = 1;
         }
      }
      else if (c=='A' || c=='v') {
         aprsdecode_click.cmd = 'A';
         useri_killallmenus();
      }
      else if (c=='o') aprsdecode_click.cmd = c;
      else if (c=='Q') aprsdecode_click.cmd = c;
      else if (c=='\010') aprsdecode_click.cmd = c;
      else if (c=='X') aprsdecode_click.cmd = c;
      else if (c=='Y') aprsdecode_click.cmd = c;
      else if (c=='~') aprsdecode_click.cmd = c;
      else if (c=='/') aprsdecode_click.cmd = c;
      else if (c==':') aprsdecode_click.cmd = c;
      else if (c=='\003') {
         aprsdecode_click.cmd = c;
         useri_killallmenus();
      }
      else if (c=='\014') aprsdecode_click.cmd = c;
      else if (c=='1') {
         if (subknob==0UL) aprsdecode_click.cmd = '1';
         else if (subknob==1UL) aprsdecode_click.cmd = '2';
         else if (subknob==2UL) aprsdecode_click.cmd = '3';
         else aprsdecode_click.cmd = '4';
      }
      else if (c=='F') {
         aprsdecode_click.cmd = c;
         useri_killallmenus();
      }
      else if (c=='O') aprsdecode_click.cmd = c;
      else if (c=='m') {
         aprsdecode_click.cmd = c;
         useri_killallmenus();
      }
      else if (c=='\245') aprsdecode_click.cmd = c;
      else if (c=='W') {
         /*
             ELSIF c="W" THEN click.cmd:=c;
             ELSIF c=CMDRAIN THEN click.cmd:="w";
         */
         aprsdecode_click.cmd = ' ';
         if (knob==3UL) aprsdecode_lums.wxcol = 'W';
         else aprsdecode_lums.wxcol = 'R';
         aprsdecode_click.mhop[0UL] = 0;
         aprsdecode_click.onesymbol.tab = 0;
         aprsdecode_click.withradio = 0;
      }
      else if (c=='\300') {
         aprsdecode_click.cmd = ' ';
         if (knob==2UL) aprsdecode_lums.wxcol = 'w';
         else aprsdecode_lums.wxcol = 0;
         aprsdecode_click.mhop[0UL] = 0;
         aprsdecode_click.onesymbol.tab = 0;
         aprsdecode_click.withradio = 0;
      }
      else if (c=='S') aprsdecode_click.cmd = c;
      else if (c=='\022') {
         /*    ELSIF c="s" THEN click.cmd:=c; */
         if (subknob==0UL) {
            configs[useri_fSRTMCACHE].on = (configs[useri_fSRTMCACHE].on+1U)
                %3U;
            updatemenus();
            useri_rdlums();
         }
         else if (subknob==1UL) {
            aprsdecode_click.cmd = ' ';
            useri_refresh = 1;
         }
         else aprsdecode_click.cmd = '\022';
      }
      else if (c=='\312') aprsdecode_click.cmd = c;
      else if (c=='\216') {
         aprsdecode_click.cmd = '\216';
         useri_rdlums();
         if (knob==1UL) aprsdecode_click.cmdatt = (char)(subknob+1UL);
         else if (knob==2UL) {
            if (aprsdecode_lums.logmode) {
               aprsdecode_click.cmdatt = (char)(subknob+6UL);
            }
            else if (subknob==0UL) {
               if (dellog) {
                  dellog = 0UL;
                  aprsdecode_click.cmdatt = '\011';
               }
               else {
                  dellog = 1UL;
                  aprsdecode_click.cmdatt = '\010';
               }
            }
            else if (dellog) {
               dellog = 0UL;
               aprsdecode_click.cmdatt = '\012';
            }
            else aprsdecode_click.cmdatt = '\007';
         }
         else {
            aprsdecode_click.cmdatt = 0;
            aprsdecode_click.cmd = ' ';
         }
      }
      else if (c=='\311') {
         /*    actmenu:=menu;  */
         aprsdecode_click.cmd = '\311';
      }
      else if (c=='\310') aprsdecode_click.cmd = '\310';
      else if (c=='\237') aprsdecode_click.cmd = '\237';
      else if (c=='\201') {
         if (knob==1UL) icfg(useri_fCLICKMAP, "C", 2ul);
         else if (knob==2UL) icfg(useri_fCLICKMAP, "X", 2ul);
         else if (knob==3UL) icfg(useri_fCLICKMAP, "Y", 2ul);
         else if (knob==4UL) icfg(useri_fCLICKMAP, "2", 2ul);
         else if (knob==5UL) icfg(useri_fCLICKMAP, "m", 2ul);
         else icfg(useri_fCLICKMAP, "", 1ul);
         useri_killallmenus();
         useri_refresh = 1;
      }
      else if (c=='\202') {
         if (knob==1UL) {
            toggcfg(useri_fCLICKSYM, 'Y', "AC+CFGHEARD+X.=", 16ul);
         }
         else if (knob==2UL) {
            toggcfg(useri_fCLICKSYM, 'X', "AC+CFGHEARD+Y.=", 16ul);
         }
         else if (knob==3UL) {
            toggcfg(useri_fCLICKSYM, 'C', "uA+CFGHEARD+XY.=", 17ul);
         }
         else if (knob==4UL) {
            toggcfg(useri_fCLICKSYM, 'u', "AC", 3ul);
         }
         else if (knob==5UL) toggcfg(useri_fCLICKSYM, 'A', "", 1ul);
         else if (knob==6UL) {
            toggcfg(useri_fCLICKSYM, '.', "AC+CFGHEARD+XY=", 16ul);
         }
         else if (knob==7UL) {
            toggcfg(useri_fCLICKSYM, '=', "uAC+CFGHEARD+XY.", 17ul);
         }
         else if (knob==8UL) {
            toggcfg(useri_fCLICKSYM, 'H', "uAC+CFGHEARD+XY.=", 18ul);
         }
         else if (knob==9UL) {
            toggcfg(useri_fCLICKSYM, 'b', "AC+CFGHEARD+XY.=", 17ul);
         }
         else if (knob==10UL) {
            toggcfg(useri_fCLICKSYM, 's', "AC+CFGHEARD+XY.=", 17ul);
         }
         else if (knob==11UL) {
            toggcfg(useri_fCLICKSYM, 'n', "AC+CFGHEARD+XY.=", 17ul);
         }
         else if (knob==12UL) icfg(useri_fCLICKSYM, "", 1ul);
         symbolonoff(menu, '\202', useri_fCLICKSYM);
         useri_refresh = 1;
      }
      else if (c=='\203') {
         if (knob==1UL) toggcfg(useri_fCLICKTEXT, 'Y', "ACHX.=", 7ul);
         else if (knob==2UL) toggcfg(useri_fCLICKTEXT, 'X', "ACHY.=", 7ul);
         else if (knob==3UL) toggcfg(useri_fCLICKTEXT, 'C', "uAHXY.=", 8ul);
         else if (knob==4UL) toggcfg(useri_fCLICKTEXT, 'u', "AC", 3ul);
         else if (knob==5UL) toggcfg(useri_fCLICKTEXT, 'A', "", 1ul);
         else if (knob==6UL) toggcfg(useri_fCLICKTEXT, '.', "ACHXY=", 7ul);
         else if (knob==7UL) toggcfg(useri_fCLICKTEXT, '=', "uACHXY.", 8ul);
         else if (knob==8UL) toggcfg(useri_fCLICKTEXT, 'H', "uACHXY.=", 9ul);
         else if (knob==9UL) toggcfg(useri_fCLICKTEXT, 'b', "ACHXY.=", 8ul);
         else if (knob==10UL) toggcfg(useri_fCLICKTEXT, 's', "ACHXY.=", 8ul);
         else if (knob==11UL) toggcfg(useri_fCLICKTEXT, 'n', "ACHXY.=", 8ul);
         else if (knob==12UL) icfg(useri_fCLICKTEXT, "", 1ul);
         symbolonoff(menu, '\203', useri_fCLICKTEXT);
         useri_refresh = 1;
      }
      else if (c=='\232') {
         if (knob==1UL) icfg(useri_fCLICKWXSYM, "C", 2ul);
         else if (knob==2UL) icfg(useri_fCLICKWXSYM, ".", 2ul);
         else if (knob==3UL) icfg(useri_fCLICKWXSYM, "=", 2ul);
         else if (knob==4UL) icfg(useri_fCLICKWXSYM, "H", 2ul);
         else if (knob==5UL) icfg(useri_fCLICKWXSYM, "b", 2ul);
         else if (knob==6UL) wxraw(useri_fCLICKWXSYM);
         else if (knob==7UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wLUMI);
         else if (knob==8UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wRAIN);
         else if (knob==9UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wWINDDIR);
         else if (knob==10UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wWIND);
         else if (knob==11UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wHYG);
         else if (knob==12UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wBARO);
         else if (knob==13UL) toggwx(useri_fCLICKWXSYM, aprsdecode_wTEMP);
         else if (knob==14UL) icfg(useri_fCLICKWXSYM, "", 1ul);
         wxonoff(menu);
         useri_refresh = 1;
      }
      else if (c=='\327') {
         if (knob==1UL) {
            wxraw(useri_fHOVERSET);
         }
         else if (knob==2UL) toggwx(useri_fHOVERSET, aprsdecode_wLUMI);
         else if (knob==3UL) toggwx(useri_fHOVERSET, aprsdecode_wRAIN);
         else if (knob==4UL) toggwx(useri_fHOVERSET, aprsdecode_wWINDDIR);
         else if (knob==5UL) toggwx(useri_fHOVERSET, aprsdecode_wWIND);
         else if (knob==6UL) toggwx(useri_fHOVERSET, aprsdecode_wHYG);
         else if (knob==7UL) toggwx(useri_fHOVERSET, aprsdecode_wBARO);
         else if (knob==8UL) toggwx(useri_fHOVERSET, aprsdecode_wTEMP);
         else if (knob==9UL) icfg(useri_fHOVERSET, "", 1ul);
         hoveronoff(menu);
         useri_refresh = 1;
      }
      else if (c=='\204') {
         if (knob==1UL) toggcfg(useri_fCLICKTRACK, 'q', "A=", 3ul);
         else if (knob==2UL) toggcfg(useri_fCLICKTRACK, 'n', "qA=", 4ul);
         else if (knob==3UL) toggcfg(useri_fCLICKTRACK, 's', "qA=", 4ul);
         else if (knob==4UL) toggcfg(useri_fCLICKTRACK, 'A', "bsunq=", 7ul);
         else if (knob==5UL) toggcfg(useri_fCLICKTRACK, '.', "q=", 3ul);
         else if (knob==6UL) toggcfg(useri_fCLICKTRACK, '=', "uq.", 4ul);
         else if (knob==7UL) toggcfg(useri_fCLICKTRACK, 'b', "qA=", 4ul);
         else if (knob==8UL) toggcfg(useri_fCLICKTRACK, 'u', "qA=", 4ul);
         else if (knob==9UL) icfg(useri_fCLICKTRACK, "", 1ul);
         /*      killallmenus; */
         trackonoff(menu, '\204', useri_fCLICKTRACK);
         useri_refresh = 1;
      }
      else if (c=='\205') {
         op2watch(1);
         updatemenus();
      }
      else if (c=='\210') {
         op2watch(0);
         updatemenus();
      }
      else if (c=='\177') {
         /*killallmenus;*/
         deletop();
         aprsdecode_click.cmd = ' ';
         updatemenus();
         useri_refresh = 1;
      }
      else if (c=='\206') {
         useri_killallmenus();
         useri_refresh = 1;
         if (subknob==0UL) filemenu();
         else if (subknob==1UL) configmenu2();
         else if (subknob==2UL) {
            aprsdecode_click.entries = 0UL;
            aprsdecode_click.selected = 0UL;
            useri_mainpop();
         }
         else if (subknob==3UL) toolsmenu();
         else if (subknob==4UL) helpmenu();
      }
      else if (c=='\214') {
         aprsdecode_lums.headmenuy = 0;
         useri_killallmenus();
         useri_refresh = 1;
      }
      else if (c=='\215') {
         aprsdecode_lums.headmenuy = 1;
         useri_killallmenus();
         useri_refresh = 1;
      }
      else if (c=='\217' && aprsdecode_lums.headmenuy) {
         filemenu();
         useri_refresh = 1;
      }
      else if (c=='\221' && aprsdecode_lums.headmenuy) {
         helpmenu();
         useri_refresh = 1;
      }
      else if (c=='\234') setsym(knob, subknob, useri_fMYSYM, menu, c);
      else if (c=='\244') setsym(knob, subknob, useri_fRBSYMB, menu, c);
      else if (c=='\326') {
         setsym(knob, subknob, useri_fONESYMB, menu, c);
         useri_rdonesymb(1);
         /*      click.onesymbol.tab:=0C;
                      (* redraw *) */
         /*      click.cmd:="s"; */
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\325') {
         if (subknob==0UL) aprsdecode_click.cmd = 's';
      }
      else if (c=='\236') {
         useri_reloadmap = 1;
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\302') {
         if (knob==1UL) aprsdecode_click.cmd = '7';
         else if (knob==2UL) aprsdecode_click.cmd = '8';
         else if (knob==3UL) aprsdecode_click.cmd = '9';
         else {
            useri_Setmap(knob-1UL);
            aprsdecode_click.cmd = ' ';
         }
      }
      else if (c=='\241') {
         killmenus(menu->next);
         menu->next = 0;
         downloadmenu();
         useri_refresh = 1;
      }
      else if (c=='\240') {
         maptool_mappack.run = 0;
         useri_textautosize(0L, 0L, 6UL, 0UL, 'b', "Download Stopped", 17ul);
         useri_refresh = 1;
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\242') {
         maptool_mappack.zoom = 0L;
         maptool_mappack.donecnt = 0UL;
         maptool_mappack.run = 1;
         useri_textautosize(0L, 0L, 6UL, 0UL, 'b', "Download Start", 15ul);
         useri_refresh = 1;
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\243') {
         if (subknob==0UL) {
            useri_nextmsg = 0L;
            useri_killmenuid(211UL);
         }
         else if (subknob==1UL) {
            useri_nextmsg = labs(useri_nextmsg)+1L;
            useri_killmenuid(211UL);
         }
         else if (subknob==2UL) {
            useri_killmenuid(211UL);
            reply();
         }
         else if (subknob==3UL) delmsgfifo();
         useri_refresh = 1;
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\277') {
         if (xosi_Shift) {
            useri_killallmenus();
            if (subknob==0UL) onlinesetup(5UL, 0UL);
            else if (subknob<=4UL) dorfcfg(13UL-subknob, 1UL);
            else if (subknob==6UL) {
               /*        maincfg(8, TRUE) */
               sndmsg = 1;
            }
            else if (subknob==7UL) maincfg(6UL, 1);
         }
         else if (subknob==0UL) {
            configtogg(useri_fCONNECT);
            updatemenus();
         }
         else if (subknob==1UL) configtogg(useri_fUDP1);
         else if (subknob==2UL) configtogg(useri_fUDP2);
         else if (subknob==3UL) configtogg(useri_fUDP3);
         else if (subknob==4UL) configtogg(useri_fUDP4);
         else if (subknob==5UL) {
            aprsdecode_click.cmd = '\216';
            aprsdecode_click.cmdatt = '\006';
         }
         else if (subknob==6UL) {
            if (useri_nextmsg==0L) useri_nextmsg = 1L;
            aprsdecode_click.cmd = ' ';
         }
         else if (subknob==7UL) {
            if (useri_configon(useri_fGETMAPS)
                ==useri_configon(useri_fALLOWEXP)) {
               expandtogg();
               aprsdecode_click.cmd = ' ';
            }
            else maploadtogg();
         }
         else if (subknob==8UL) {
            configtogg(useri_fTRACKFILT);
            aprsdecode_click.cmd = ' ';
         }
         else if (subknob==9UL) {
            /*        ELSIF subknob=9 THEN click.cmd:="0"; */
            aprsdecode_lums.wxcol = 0;
            aprsdecode_click.withradio = 0;
            aprsdecode_click.cmd = '0';
         }
         useri_refresh = 1;
      }
      else if (c=='\255') {
         if (knob==1UL) {
            if (subknob==0UL) aprsdecode_makemsg(1);
            else if (subknob==1UL) aprsdecode_makemsg(0);
            else sndmsg = 0;
            useri_killmenuid(228UL);
         }
         else if (knob==4UL) setmsgport(subknob);
         useri_refresh = 1;
      }
      else if (c=='\260') {
         aprsdecode_deltxmsg(knob, menu->scroll);
         useri_killallmenus();
         useri_refresh = 1;
      }
      else if (c=='\267') {
         configdelman((unsigned char)configedit, knob, menu->scroll);
      }
      else if (c=='\265') callfiltsetup(knob);
      else if (c=='\271') dorfcfg(knob, subknob);
      else if (c=='\254') domsgcfg(knob);
      else if (c=='\273') dotimers(knob);
      else if (c=='\274') domap(knob, subknob);
      else if (c=='\303') mapdir();
      else if (c=='\304') docfgbeep(knob, subknob);
      else if (c=='\275') domapmove(knob, subknob);
      else if (c=='\313') {
         /*
               IF subknob=0 THEN configtogg(fGEOPROFIL) ELSE 
                 configs[fEDITLINE].title:="Ant1[m] Ant2[m] Wavelen[mm]";
                 configs[fEDITLINE].width:=CONFIGWIDTH;
                 configedit:=ORD(fGEOPROFIL);
               END;
               copytoed;
               updatemenus;
         */
         configtogg(useri_fGEOPROFIL);
         updatemenus();
         aprsdecode_click.cmd = ' ';
         useri_refresh = 1;
      }
      else if (c=='\264') {
         if (knob==1UL) {
            if (subknob==0UL) {
               configtogg(useri_fAUTOSAVE);
               updatemenus();
            }
            else useri_saveconfig();
         }
         else if (knob==2UL) {
            useri_loadconfig(1);
            configedit = 0UL;
         }
         else if (knob==3UL) watchcallsetup();
         useri_refresh = 1;
      }
      else if (c=='\270') onlinesetup(knob, subknob);
      else if (c=='\323') filesetup(knob, subknob);
      else if (c=='\262') {
         configedit = knob;
         useri_refresh = 1;
      }
      else if (c=='\263') configman(subknob, &aprsdecode_click.cmd);
      else if (c=='\261') aprsdecode_savetrack();
      else if (c=='\256') {
         sndmsg = 1;
         useri_refresh = 1;
      }
      else if (c=='\247') {
         if (knob==8UL) {
            startlist("Objects", 8ul, aprsdecode_click.mhop, 9ul);
            aprstext_listtyps('O', subknob==1UL, aprsdecode_click.mhop, 9ul);
         }
         else if (knob==7UL) {
            startlist("Wx Stations", 12ul, "", 1ul);
            aprstext_listtyps('W', subknob==1UL, aprsdecode_click.mhop, 9ul);
         }
         else if (knob==6UL) {
            startlist("Moving Stations", 16ul, "", 1ul);
            aprstext_listtyps('D', subknob==1UL, aprsdecode_click.mhop, 9ul);
         }
         else if (knob==5UL) {
            startlist("No Position", 12ul, "", 1ul);
            aprstext_listtyps('N', subknob==1UL, aprsdecode_click.mhop, 9ul);
         }
         else if (knob==4UL) {
            if (subknob>=5UL) startmon(1);
            else monconfig(subknob);
         }
         else if (knob==3UL) {
            startlist("Messages", 9ul, aprsdecode_click.mhop, 9ul);
            aprstext_listtyps('M', 0, aprsdecode_click.mhop, 9ul);
         }
         else if (knob==2UL) {
            startlist("Bulletins", 10ul, aprsdecode_click.mhop, 9ul);
            aprstext_listtyps('B', 0, aprsdecode_click.mhop, 9ul);
         }
         else if (knob==1UL) aprsdecode_click.cmd = 'I';
      }
      else if (c=='\246') {
         startlist("Beacons", 8ul, "", 1ul);
         aprstext_listop(subknob==1UL);
      }
      else if (c=='\250') listusermsg();
      else if (c=='\251') {
         setmarks(subknob==0UL, 0);
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\252') {
         if (subknob==0UL) setmarks(1, 1);
         else if (subknob==3UL) setmarks(0, 1);
         else if (subknob==1UL) {
            aprsdecode_posinval(&aprsdecode_click.markpos);
            aprsdecode_click.waysum = 0.0f;
            aprsdecode_posinval(&aprsdecode_click.squerpos0);
         }
         else aprsdecode_posinval(&aprsdecode_click.measurepos);
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\301') {
         /*     IF subknob=1 THEN click.cmd:=":" */
         /*     ELSE setmarks(subknob=0, TRUE); click.cmd:=" " END; */
         /*
             ELSIF c=CMDINTERNSTAT THEN click.cmd:=CMDINTERNSTAT;
             ELSIF c=LISTMESSAGES THEN startlist("Messages");
                listtyps("M", subknob=1); 
             ELSIF c=LISTBULLETINS THEN startlist("Bulletins");
                listtyps("B", subknob=1);
             ELSIF c=LISTNOPOS THEN startlist("No Position");
                listtyps("N", subknob=1);
             ELSIF c=LISTMOVED THEN startlist("Moving Stations");
                listtyps("D", subknob=1);
             ELSIF c=LISTWX THEN startlist("Wx Stations");
                listtyps("W", subknob=1);
             ELSIF c=LISTOBJ THEN startlist("Objects");
                listtyps("O", subknob=1);
         */
         mapadd();
      }
      else if (c=='\305') useri_helptext(0UL, 0UL, 0UL, 0UL, "index", 6ul);
      else if (c=='\306') useri_helptext(knob, subknob, potx, poty, "", 1ul);
      else if (c=='\223') managebeacon(menu->scroll, knob, subknob, 0);
      else if (c=='\213') dobeacon(menu->scroll, knob);
      else if (c=='\314') dodigi(menu->scroll, knob, subknob, 0U);
      else if (c=='\315') dodigi(menu->scroll, knob, subknob, 1U);
      else if (c=='\253') watchdo(knob, subknob);
      else if (c=='\317') {
         setcolour(menu->scroll, potx, poty);
         aprsdecode_click.cmd = ' ';
      }
      else if (c=='\316') colouredit(knob, subknob);
      else if (c=='\321') coloureditgeo(knob, subknob);
      else if (c=='\332') {
         /*killallmenus;*/
         aprsdecode_importbeacon();
      }
      else if (c=='\331') {
         if (subknob==0UL) {
            aprsdecode_click.insreplaceline = !aprsdecode_click.insreplaceline;
            useri_poligonmenu();
         }
         else aprsdecode_modmultiline(subknob);
      }
   }
} /* end mouseleft() */


extern void useri_keychar(char ch, char ispasted, char movecmd)
{
   /*  IF movecmd THEN click.cmd:=ch; RETURN END; */
   useri_killbubble();
   closehint();
   if (ch=='\033') escmenus();
   kbtomenu(&ch);
   if (ch) clampedline = 0UL;
   if (ispasted) ch = 0;
   /*WrInt(ORD(ch), 1); WrStrLn("kbd"); */
   if (ch=='R') aprsdecode_click.cmd = '\022';
   else if (ch=='\022') {
      /*ELSIF ch=CURSPAGEUP THEN WrStrLn("cup"); */
      /*ELSIF ch=CURSPAGEDOWN THEN WrStrLn("cdown"); */
      ch = 0;
   }
   else if (ch=='\320') configman(0UL, &aprsdecode_click.cmd);
   else if (((((((ch!='b' && ch!='m') && ch!='f') && ch!='u') && ch!='d')
                && ch!='p') && ch!='s') && ch!='h') {
      aprsdecode_click.cmd = X2C_CAP(ch);
   }
   else aprsdecode_click.cmd = ch;
   if (aprsdecode_click.cmd=='>' || aprsdecode_click.cmd=='<') {
      if (aprsdecode_click.entries>0UL) {
         useri_killallmenus();
         textinfo((unsigned long)(aprsdecode_click.cmd=='<'));
         aprsdecode_click.dryrun = 0;
         images(aprsdecode_click.table[aprsdecode_click.selected].opf, 0,
                aprsdecode_click.graphset&0x280U);
         useri_refresh = 1;
      }
   }
   else if (aprsdecode_click.cmd=='\177') {
      if ((useri_beaconediting && useri_beaconed)
                && aprsdecode_ismultiline(1)) aprsdecode_modmultiline(1UL);
      else deletop();
      aprsdecode_click.cmd = ' ';
      useri_refresh = 1;
   }
   else if ((aprsdecode_click.cmd=='\017' && useri_beaconediting)
                && useri_beaconed) {
      aprsdecode_click.insreplaceline = !aprsdecode_click.insreplaceline;
   }
   else if (aprsdecode_click.cmd=='B') {
      if (useri_beaconediting) {
         useri_beaconediting = 0; /* Close beacon */
         useri_killmenuid(226UL);
      }
      else dorfcfg(3UL, 0UL);
   }
   else if (aprsdecode_click.cmd=='M') {
      if (sndmsg) {
         sndmsg = 0;
         useri_killmenuid(228UL);
      }
      else sndmsg = 1;
      useri_refresh = 1;
      aprsdecode_click.cmd = ' ';
   }
   else if (aprsdecode_click.cmd=='F') findopl(0);
   else if (aprsdecode_click.cmd=='u') {
      if (useri_listwin!='M') startmon(1);
      else closelist();
   }
   else if (aprsdecode_click.cmd=='P') {
      if (panowin.on) closepano();
      else startpano();
   }
   else if (aprsdecode_click.cmd=='d') maploadtogg();
   else if (aprsdecode_click.cmd=='p') domap(11UL, 0UL);
/*  ELSIF click.cmd="D" THEN domap(RESETTODEFAULT, 0); */
/*  IF refresh THEN redrawpop(redrawimg) END; */
} /* end keychar() */


extern void useri_getstartxysize(long * newx, long * newy)
{
   *newx = useri_conf2int(useri_fXYSIZE, 0UL, 100L, 32000L, 600L);
   *newy = useri_conf2int(useri_fXYSIZE, 1UL, 50L, 32000L, 400L);
} /* end getstartxysize() */


extern void useri_resizewin(long newx, long newy, char f11)
{
   if (f11) {
      if (useri_maximized) {
         useri_getstartxysize(&newx, &newy); /* maximized off */
         useri_maximized = 0;
      }
      else useri_maximized = 1;
   }
   if (redrawimg==0 || ((long)mainxs()!=newx || (long)useri_mainys()!=newy)) {
      useri_newxsize = (unsigned long)newx;
      useri_newysize = (unsigned long)newy;
      if (useri_newxsize<100UL) useri_newxsize = 100UL;
      else if (useri_newxsize>32000UL) useri_newxsize = 32000UL;
      if (useri_newysize<50UL) useri_newysize = 50UL;
      else if (useri_newysize>32000UL) useri_newysize = 32000UL;
      if (f11) xosi_setxwinsize((long)useri_newxsize, (long)useri_newysize);
   }
/*WrInt(newxsize, 10); WrInt(newysize, 10); WrStrLn(" newxy1"); */
/*  refresh:=TRUE; */
/*  redrawpop(redrawimg); */
} /* end resizewin() */


extern void useri_refreshwin(void)
{
   if (useri_newxsize==0UL) useri_redraw(redrawimg);
} /* end refreshwin() */


static void cpbytes(pLISTLINE bi, char test, unsigned long * cnt)
{
   unsigned long end;
   unsigned long start;
   unsigned long j;
   unsigned long i;
   while (bi) {
      end = bi->endcp;
      i = 0UL;
      j = end;
      while (i<end && i<bi->len) {
         if (bi->text[i]==0) end = j;
         else if (bi->text[i]=='\375') end += 2UL;
         else if ((unsigned char)bi->text[i]>=(unsigned char)'\201') ++end;
         ++i;
      }
      start = bi->startcp;
      i = 0UL;
      j = start;
      while (i<start && i<end) {
         if (bi->text[i]==0) start = j;
         else if (bi->text[i]=='\375') start -= 2UL;
         else if ((unsigned char)bi->text[i]>=(unsigned char)'\201') ++start;
         ++i;
      }
      /*WrInt(bi^.startcp, 6);WrInt(bi^.endcp, 6);  WrInt(start, 6);
                WrInt(end, 6); WrInt(cnt, 6); WrStrLn(""); */
      if (end>0UL) {
         if (bi->text[end]==0) {
            /* new line */
            if (test) ++*cnt;
            else {
               --*cnt;
               xosi_cutbuffer.text[*cnt] = '\012';
            }
         }
         while (end>start) {
            --end;
            if ((unsigned char)bi->text[end]<(unsigned char)'\201') {
               if (test) ++*cnt;
               else {
                  --*cnt;
                  xosi_cutbuffer.text[*cnt] = bi->text[end];
               }
            }
         }
      }
      bi = bi->next;
   }
} /* end cpbytes() */


static void copypastelist(void)
{
   pLISTLINE bb;
   unsigned long len;
   if (pullmenuwhat=='C') {
      bb = 0;
      if (useri_listwin=='M') bb = monbuffer.listlines;
      else if (useri_listwin=='L') bb = listbuffer.listlines;
      if (bb) {
         len = 0UL;
         cpbytes(bb, 1, &len);
         if (len>0UL) {
            alloccutbuf(len);
            cpbytes(bb, 0, &len);
            xosi_cutpaste();
         }
      }
   }
} /* end copypastelist() */


extern void useri_pulloff(void)
{
   pullmenuwid = 0L;
   xosi_sethand(xosi_cOFF);
   xosi_pulling = 0;
   copypastelist();
} /* end pulloff() */


static void limmouse(long * x, long * y)
{
   if (*x<0L) *x = 0L;
   else if (*x>(long)mainxs()) *x = (long)mainxs();
   if (*y<0L) *y = 0L;
   else if (*y>(long)useri_mainys()) *y = (long)useri_mainys();
} /* end limmouse() */


static void startpull(long x, long y, char middlbutt)
{
   pMENU pm;
   unsigned long posy;
   unsigned long posx;
   unsigned long subknob;
   unsigned long knob;
   useri_killbubble();
   whichmenu((unsigned long)x, (unsigned long)y, &pm, &knob, &subknob, &posx,
                 &posy);
   if ((pm && (unsigned long)pm->pullconf>0UL)
                && (middlbutt || posy<=pm->pullyknob)) {
      useri_saveXYtocfg(pm->pullconf, (long)pm->nowx, (long)pm->nowy);
      pullmenuwid = (long)pm->wid;
      pullmenux = x;
      pullmenuy = y;
      pullmenuwhat = WhatPull(pm, posx, posy);
   }
   /*  findpullmenu(x, y); */
   if (pm==0 || middlbutt && pullmenuwid==0L) {
      maptool_pullmap(x, (long)useri_mainys()-y, 1); /* start pulling */
   }
   else if (pullmenuwid) {
      /*sethand(cPULL4);*/
      xosi_pulling = 1;
   }
} /* end startpull() */


static void setzoom(long x1, long y1)
{
   aprsdecode_click.zoomtox = x1;
   aprsdecode_click.zoomtoy = y1;
   xosi_zooming = 1;
   aprsdecode_click.cmd = ' '; /* start makeimage */
} /* end setzoom() */


static void setclick(long x, long y)
{
   limmouse(&x, &y);
   aprsdecode_click.x = x;
   aprsdecode_click.y = (long)useri_mainys()-y;
} /* end setclick() */


extern void useri_mouseleftdown(long x, long y)
{
   setclick(x, y);
   if (xosi_pulling) useri_pulloff();
   leftbutton = 1;
   xosi_zooming = 0;
   if (clampedline && hilitemenu((unsigned long)x, (unsigned long)y, 0, 0)) {
      useri_refresh = 1;
   }
} /* end mouseleftdown() */


extern void useri_mousemiddle(long x, long y)
{
   clampedline = 0UL;
   limmouse(&x, &y);
   startpull(x, y, 1);
} /* end mousemiddle() */


extern void useri_mouserightdown(long x, long y)
{
   setclick(x, y);
   if (xosi_pulling) useri_pulloff();
   rightbutton = 1;
   /*  clampedline:=0; */
   /*  IF sndmsg OR (configedit<>0) THEN paste ELSE click.cmd:=CURSBS END; */
   /*  IF FindClampMenu()<>NIL THEN paste ELSE click.cmd:=CURSBS END; */
   /*  redrawpop(redrawimg); */
   useri_refresh = 1;
} /* end mouserightdown() */


extern void useri_mousemove(long x, long y)
{
   long mvd;
   limmouse(&x, &y);
   if (!xosi_pulling) xosi_sethand(xosi_cOFF);
   if (xosi_pulling) {
      /* goon pulling */
      if (pullmenuwid) {
         Pullmenu((unsigned long)pullmenuwid, x-pullmenux, y-pullmenuy);
         pullmenux = x;
         pullmenuy = y;
      }
      else {
         maptool_pullmap(x, (long)useri_mainys()-y, 0);
         aprsdecode_click.cmd = ' '; /* start makeimage */
      }
   }
   else if (xosi_zooming) setzoom(x, (long)useri_mainys()-y);
   else if (leftbutton || rightbutton) {
      if (xosi_Shift || rightbutton) mvd = 5L;
      else if (leftbutton) mvd = 2L;
      if (labs(x-aprsdecode_click.x)>mvd || labs(((long)useri_mainys()-y)
                -aprsdecode_click.y)>mvd) {
         /* moved a little */
         if (xosi_Shift || rightbutton) setzoom(x, (long)useri_mainys()-y);
         else if (leftbutton) {
            startpull(aprsdecode_click.x,
                (long)useri_mainys()-aprsdecode_click.y, 0);
         }
      }
   }
   else {
      /* move only */
      useri_xmouse.x = x;
      useri_xmouse.y = y;
      if (hintmouse.x==0L) hintmouse = useri_xmouse;
      if (labs(hintmouse.x-useri_xmouse.x)+labs(hintmouse.y-useri_xmouse.y)
                >8L) closehint();
      if (clampedline==0UL && hilitemenu((unsigned long)x, (unsigned long)y,
                0, 0)) useri_refresh = 1;
      if (hinttime!=aprsdecode_realtime && useri_configon(useri_fMOUSELOC)) {
         mouseshowcnt = 1UL;
      }
      aprsdecode_tracenew.winevent = 1UL;
   }
} /* end mousemove() */


extern void useri_mouserelease(void)
{
   if (xosi_pulling) useri_pulloff();
   else if (xosi_zooming) aprsdecode_click.cmd = '\307';
   else if (leftbutton) {
      mouseleft(aprsdecode_click.x, (long)useri_mainys()-aprsdecode_click.y);
   }
   else if (rightbutton) {
      useri_killbubble();
      if (FindClampMenu()) xosi_paste();
      else aprsdecode_click.cmd = '\010';
   }
   useri_killbubble();
   leftbutton = 0;
   rightbutton = 0;
   xosi_zooming = 0;
   pullmenuwid = 0L;
} /* end mouserelease() */


static void showexitcode(char name[], unsigned long name_len, long c)
{
   char h[100];
   char s[100];
   X2C_PCOPY((void **)&name,name_len);
   aprsstr_Assign(s, 100ul, name, name_len);
   aprsstr_IntToStr(c, 1UL, h, 100ul);
   aprsstr_Append(s, 100ul, h, 100ul);
   useri_say(s, 100ul, 6UL, 'e');
   X2C_PFREE(name);
} /* end showexitcode() */


static void checkserial(struct xosi_PROCESSHANDLE * pid, unsigned char cfg)
{
   if (pid->runs) {
      xosi_CheckProg(pid);
      if (!pid->runs) {
         showexitcode("Serial Interface exits with ", 29ul, pid->exitcode);
         useri_configbool(cfg, 0);
         updatemenus();
      }
      else if (!useri_configon(cfg)) xosi_StopProg(pid);
   }
} /* end checkserial() */


extern void useri_timerevent(void)
{
   if (mouseshowcnt>0UL) {
      --mouseshowcnt;
      if (mouseshowcnt==0UL) {
         mouseshow((unsigned long)useri_xmouse.x,
                (unsigned long)useri_xmouse.y);
         useri_refresh = 1;
      }
   }
   aprsdecode_tcpjobs();
   printhint();
   maptool_MapPackageJob(0);
   if (aprsdecode_maploadpid.runs) {
      xosi_CheckProg(&aprsdecode_maploadpid);
      if (!aprsdecode_maploadpid.runs) {
         useri_refresh = 1;
         if (aprsdecode_maploadpid.exitcode) {
            showexitcode("Maploader exits with ", 22ul,
                aprsdecode_maploadpid.exitcode);
         }
      }
   }
   checkserial(&aprsdecode_serialpid, useri_fSERIALTASK);
   checkserial(&aprsdecode_serialpid2, useri_fSERIALTASK2);
   if (useri_refresh) useri_refreshwin();
} /* end timerevent() */


extern void useri_Quit(void)
{
   aprsdecode_click.cmd = 'Q';
} /* end Quit() */


extern void useri_wrheap(void)
{
   InOut_WriteString("heapusage screenbuf:", 21ul);
   InOut_WriteInt((long)useri_debugmem.screens, 1UL);
   InOut_WriteString(" mondata:", 10ul);
   InOut_WriteInt((long)useri_debugmem.mon, 1UL);
   InOut_WriteString(" menus:", 8ul);
   InOut_WriteInt((long)useri_debugmem.menus, 1UL);
   InOut_WriteString(" requested:", 12ul);
   InOut_WriteInt((long)useri_debugmem.req, 1UL);
   useri_debugmem.req = 0UL;
   osi_WrLn();
} /* end wrheap() */


extern void useri_initmenus(void)
{
   overtype = 0;
   redrawimg = 0;
   useri_listwin = 0;
   memset((char *) &listbuffer,(char)0,sizeof(struct LISTBUFFER));
   memset((char *) &monbuffer,(char)0,sizeof(struct LISTBUFFER));
   aprsdecode_msgfifo0 = 0;
   aprsdecode_click.graphset = 0U;
   /*  cornermenu:=NIL; */
   menus = 0;
   /*  actmenu:=NIL; */
   hinttime = 0UL;
   mouseshowcnt = 0UL;
   /*  locpop:=NIL; */
   /*  hintcnt:=0; */
   /*  keybfocus:=NIL; */
   /*  autoclick:=0C; */
   /*  menucnt:=0; */
   useri_panoimage = 0;
   poioff = 0;
   aprsdecode_lums.headmenuy = 1;
   xosi_pulling = 0;
   xosi_zooming = 0;
   leftbutton = 0;
   rightbutton = 0;
   sndmsg = 0;
   panowin.on = 0;
   useri_beaconed = 0;
   digied = 0;
   configedit = 0UL;
   useri_beaconediting = 0;
   dellog = 0UL;
   pullmenuwid = 0L;
   clampedline = 0UL;
   hoveropen = 0;
   /*  clampedline:=0; */
   /*  FILL(ADR(menupullpos), 0C, SIZE(menupullpos)); */
   useri_Setmap(0UL);
} /* end initmenus() */


extern void useri_BEGIN(void)
{
   static int useri_init = 0;
   if (useri_init) return;
   useri_init = 1;
   aprstat_BEGIN();
   aprstext_BEGIN();
   xosi_BEGIN();
   TimeConv_BEGIN();
   osi_BEGIN();
   Storage_BEGIN();
   maptool_BEGIN();
   aprspos_BEGIN();
   aprsstr_BEGIN();
   aprsdecode_BEGIN();
}

