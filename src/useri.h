/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef useri_H_
#define useri_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef maptool_H_
#include "maptool.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif

/* aprsmap user interface */
#define useri_CURSDOWN '\030'

#define useri_CURSLEFT '\023'

#define useri_CURSUP '\005'

#define useri_CURSRIGHT '\004'

#define useri_CURSDEL '\177'

#define useri_CURSBS '\010'

#define useri_CURSENTER '\015'

#define useri_CURSPAGEUP '\022'

#define useri_CURSPAGEDOWN '\003'

#define useri_CURSHOME '\001'

#define useri_CURSEND '\002'

#define useri_CURSINS '\017'

#define useri_CURSPASTE '\026'
/*    CURSF11=CHR(11);*/

#define useri_CMDRDLOG "\216"

#define useri_CMDDOWNLOAD "\237"

#define useri_CMDSTARTDOWNLOAD "\242"

#define useri_CMD1USER "."

#define useri_CMD1USERRF "="

#define useri_CMDANIMATE1 "a"

#define useri_CMDANIMATEMENU "v"

#define useri_CMDDELWAYPOINT "q"

#define useri_CMDINTERNSTAT "I"

#define useri_CMDFIND "\245"

#define useri_CMDZOOMSQUARE "\307"

#define useri_CMDFZOOMIN "\310"

#define useri_CMDFZOOMOUT "\311"

#define useri_CMDVIDEO "\312"

#define useri_CMDLISTWINLINE "t"

#define useri_CMDRADIORANGE "\022"

#define useri_CMDCLICKWATCH "\322"

#define useri_SP1 "\360"
/* micospaces for menu text */

#define useri_SP2 "\361"

#define useri_SP3 "\362"

#define useri_SP4 "\363"

#define useri_SP5 "\364"

#define useri_SP6 "\365"

#define useri_SP7 "\366"

#define useri_SP8 "\367"

#define useri_SP9 "\370"

#define useri_TILEBRIGHTNESS (-128)

#define useri_MINMHZ 0.1

#define useri_ALTINVAL (-1000000)

struct useri_MOUSEPOS;


struct useri_MOUSEPOS {
   long x;
   long y;
};

enum useri_CONFSET {useri_fFIND, useri_fLOGWFN, useri_fLOGDAYS, useri_fLOGFN,
                 useri_fLOGDATE, useri_fLOGFIND, useri_fFOTOFN,
                useri_fXYSIZE, 
   useri_fMYCALL, useri_fMYPOS, 
   useri_fNBTEXT, useri_fMYSYM, useri_fNETBTIME, useri_fRFBTSHIFT, 
   useri_fRBTEXT, useri_fRBTYP, useri_fRBNAME, useri_fRBSYMB, useri_fRBPOS,
                useri_fRBPOSTYP, useri_fRBALT, 
   useri_fRBSPEED, useri_fRBDIR, useri_fRBCOMMENT, useri_fRBTIME,
                useri_fRBPORT, useri_fRBPATH, useri_fRBDEST, 
   useri_fSERIALTASK, useri_fSERIALTASK2, useri_fSERVERURL,
                useri_fSERVERFILT, useri_fPW, 
   useri_fCONNECT, useri_fALLOWNETTX, useri_fALLOWGATE, 
   useri_fUDP1, useri_fUDP2, useri_fUDP3, useri_fUDP4, 
   useri_fDIGI, useri_fMENUXYDIGI, useri_fDIGITIME, useri_fDIGIRADIUS,
                useri_fDIGIVIA, useri_fDIGINN, useri_fDIGIX, 
   useri_fVIDEOCMD, useri_fOSMDIR, useri_fTFADE, useri_fTFULL, useri_fTPURGE,
                 useri_fTPURGEOBJ, useri_fDELAYGETMAP, 
   useri_fTRANSP, useri_fLWAY, useri_fLTEXT, useri_fLSYM, useri_fLTRACK,
                useri_fLOBJ, useri_fLMAP, useri_fLRF, useri_fNOMOV,
                useri_fCENTER, useri_fGAMMA, 
   useri_fALLOWEXP, useri_fZOOMMISS, useri_fMOUSELOC, useri_fMAPNAMES, 
   useri_fTRACKFILT, useri_fDUPDEL, useri_fARROW, useri_fINVMOV, useri_fKMH,
                useri_fKMHTIME, useri_fTEMP, useri_fWINDSYM, useri_fRULER, 
   useri_fALTMIN, 
   useri_fONESYMB, 
   useri_fDEFZOOM, useri_fANIMSPEED, useri_fVIDEOFAST, 
   useri_fVIEW, useri_fMAXZOOM, useri_fDOWNLOADZOOM, useri_fGETMAPS,
                useri_fFONTSIZE, useri_fMARKPOS, 
   useri_fMOVESTEP, useri_fZOOMSTEP, useri_fCLICKMAP, useri_fCLICKSYM,
                useri_fCLICKWXSYM, 
   useri_fCLICKTEXT, useri_fCLICKTRACK, useri_fHOVERSET, 
   useri_fMSGRFDEST, useri_fMSGNETDEST, useri_fMSGPATH, useri_fPOPUPMSG,
                useri_fMSGALLSSID, useri_fPASSSELFMSG, 
   useri_fWRINCOM, useri_fWRTICKER, 
   useri_fLOCALTIME, useri_fQUERYS, 
   useri_fSTRFILT, useri_fOBJSRCFILT, useri_fIGATEFILT, useri_fOPFILT,
                useri_fDESTFILT, useri_fRAWFILT, useri_fMUSTBECALL, 
   useri_fGEOPROFIL, useri_fGEOBRIGHTNESS, useri_fGEOCONTRAST, 
   useri_fREFRACT, useri_fFRESNELL, useri_fANT1, useri_fANT2, useri_fANT3, 
   useri_fSRTMCACHE, 
   useri_fPANOSIZE, useri_fPANOPOS, 
   useri_fBEEPPROX, useri_fBEEPWATCH, useri_fBEEPMSG, 
   useri_fMSGTO, useri_fMSGTEXT, useri_fMSGPORT, useri_fAUTOSAVE,
                useri_fWATCH, useri_fAPPROXY, 
   useri_fMENUXYEDIT, useri_fMENUXYMSG, useri_fMENUXYBEACON,
                useri_fMENUXYSTATUS, useri_fMENUXYLIST, 
   useri_fMENUXYLISTMIN, useri_fMENULISTICON, useri_fMENUSIZELIST,
                useri_fMENUMONICON, useri_fMENUXYMON, 
   useri_fMENUSIZEMON, 
   useri_fCOLMAPTEXT, useri_fCOLOBJTEXT, useri_fCOLMENUTEXT,
                useri_fCOLMENUBACK, useri_fCOLMARK1, useri_fCOLMARK2, 
   useri_fEDITLINE};


extern unsigned long useri_newxsize;

extern unsigned long useri_newysize;

extern struct useri_MOUSEPOS useri_xmouse;

extern aprsdecode_MONCALL useri_beaconimported;

extern unsigned long useri_beaconimporttime;

extern char useri_listwin;

extern char useri_beaconediting;

extern char useri_reloadmap;

extern char useri_refresh;

struct useri__D0;


struct useri__D0 {
   unsigned long menus;
   unsigned long mon;
   unsigned long screens;
   unsigned long req;
   unsigned long srtm;
};

extern struct useri__D0 useri_debugmem;

extern char useri_beaconed;

extern char useri_maximized;

extern char useri_isblown;

extern long useri_nextmsg;

extern maptool_pIMAGE useri_panoimage;

extern void useri_mainpop(void);

extern void useri_textautomenu(long, long, unsigned long, unsigned long,
                char, char [], unsigned long, char [], unsigned long,
                char [], unsigned long);

extern void useri_textautosize(long, long, unsigned long, unsigned long,
                char, char [], unsigned long);

extern void useri_killallmenus(void);

extern void useri_killmenuid(unsigned long);
/*PROCEDURE WrMon(s:ARRAY OF CHAR);*/

extern void useri_initconfig(void);

extern void useri_loadconfig(char);

extern void useri_saveconfig(void);

extern void useri_postoconfig(struct aprspos_POSITION);

extern void useri_rdlums(void);

extern void useri_xerrmsg(char [], unsigned long);

extern void useri_redraw(maptool_pIMAGE);

extern void useri_mouseleftdown(long, long);

extern void useri_mouserightdown(long, long);

extern void useri_keychar(char, char, char);

extern void useri_resizewin(long, long, char);

extern void useri_mousemove(long, long);

extern void useri_initmenus(void);

extern void useri_timerevent(void);

extern void useri_refreshwin(void);

extern void useri_Quit(void);

extern void useri_mouserelease(void);

extern void useri_mousemiddle(long, long);

extern void useri_downloadprogress(void);

extern void useri_wrheap(void);

extern void useri_sayonoff(char [], unsigned long, char);

extern void useri_getview(unsigned char, unsigned long, float *,
                struct aprspos_POSITION *);

extern void useri_refrinfo(void);

extern unsigned long useri_localtime(void);

extern void useri_refrlog(void);

extern long useri_conf2int(unsigned char, unsigned long, long, long, long);

extern float useri_conf2real(unsigned char, unsigned long, float, float,
                float);

extern void useri_confstr(unsigned char, char [], unsigned long);

extern void useri_conf2str(unsigned char, unsigned long, char [],
                unsigned long);

extern void useri_confstrings(unsigned char, unsigned long, char, char [],
                unsigned long);

extern unsigned char useri_confflags(unsigned char, unsigned long);

extern void useri_confappend(unsigned char, char [], unsigned long);

extern void useri_configbool(unsigned char, char);

extern char useri_configon(unsigned char);

extern void useri_AddConfLine(unsigned char, unsigned char, char [],
                unsigned long);

extern void useri_setview(long, char [], unsigned long);

extern void useri_clrconfig(void);

extern void useri_starthint(unsigned long, char);

extern void useri_Setmap(unsigned long);

extern void useri_helptext(unsigned long, unsigned long, unsigned long,
                unsigned long, char [], unsigned long);

extern void useri_encerr(char [], unsigned long);

extern void useri_pulloff(void);

extern void useri_saveXYtocfg(unsigned char, long, long);

extern void useri_clrmsgtext(char [], unsigned long);

extern void useri_getstartxysize(long *, long *);

extern void useri_wrstrlist(char [], unsigned long, aprsdecode_MONCALL,
                struct aprspos_POSITION, unsigned long);

extern void useri_wrstrmon(char [], unsigned long, struct aprspos_POSITION);

extern void useri_clrcpmarks(void);

extern void useri_copypaste(char [], unsigned long);

extern char useri_isupdated(unsigned char);

extern long useri_guesssize(char [], unsigned long, char [], unsigned long);
/* return filsize in kb not exact >2gb */

extern void useri_ColConfset(struct aprsdecode_COLTYP *, unsigned char);

extern char useri_gpsalt(unsigned char);

extern void useri_textbubble(struct aprspos_POSITION, char [], unsigned long,
                 char);

extern void useri_killbubble(void);

extern void useri_popwatchcall(char [], unsigned long);

extern void useri_allocimage(maptool_pIMAGE *, long, long, char);

extern unsigned long useri_mainys(void);

extern void useri_rdonesymb(char);

extern void useri_hoverinfo(struct aprsdecode_CLICKOBJECT);


extern void useri_BEGIN(void);


#endif /* useri_H_ */
