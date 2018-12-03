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
#ifndef aprsstr_H_
#include "aprsstr.h"
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

#define useri_CMDMH "h"

#define useri_CMDFIND "\245"

#define useri_CMDZOOMSQUARE "\307"

#define useri_CMDFZOOMIN "\310"

#define useri_CMDFZOOMOUT "\311"

#define useri_CMDVIDEO "\312"

#define useri_CMDCENTER "\313"

#define useri_CMDSETMARK1LOCK "\314"

#define useri_CMDSETMARK2LOCK "\315"

#define useri_CMDCENTERMOUSE "c"

#define useri_CMDLISTWINLINE "t"

#define useri_CMDRADIORANGE "\022"

#define useri_CMDCLICKWATCH "\322"

#define useri_CMDRESETIMGPARMS "\014"

#define useri_CMDZOOMTOMARKS "/"

#define useri_CONFPOISYMFIND "\335"

#define useri_CFGHEARD "H"

#define useri_CFGRAWDECODED "u"

#define useri_DEFAULTLUMOBJ "90"

#define useri_MULTILINEDEL 1
/* subknob */

#define useri_cMULTISYMBOL "*"
/* if this ose symbol set */

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

#define useri_SPS "\371"

#define useri_TILEGAMMA 0.45
/* make gamma off */

#define useri_MINMHZ 0.1

#define useri_ALTINVAL (-1000000)

#define useri_DOCKX (-3)

#define useri_BRIMUL 10
/* lums percent to 0..1000 */

#define useri_MAXANTALT 50000000
/* max antenna higth m */

struct useri_MOUSEPOS;


struct useri_MOUSEPOS {
   int32_t x;
   int32_t y;
};

enum useri_CONFSET {useri_fFIND, useri_fLOGWFN, useri_fLOGDAYS, useri_fLOGFN,
                 useri_fLOGDATE, useri_fLOGFIND, useri_fFOTOFN,
                useri_fXYSIZE, 
   useri_fMYCALL, useri_fMYPOS, useri_fPOIFILTER, useri_fPOISMBOLS, 
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
                useri_fOBJTRACK, 
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
                useri_fDESTFILT, useri_fRAWFILT, useri_fFINGERPRINT, 
   useri_fMUSTBECALL, 
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


extern uint32_t useri_newxsize;

extern uint32_t useri_newysize;

extern struct useri_MOUSEPOS useri_xmouse;

extern char useri_listwin;

struct useri__D0;


struct useri__D0 {
   uint32_t menus;
   uint32_t mon;
   uint32_t screens;
   uint32_t req;
   uint32_t poi;
};

extern struct useri__D0 useri_debugmem;

extern char useri_beaconediting;

extern char useri_reloadmap;

extern char useri_refresh;

extern char useri_beaconed;

extern char useri_maximized;

extern char useri_isblown;

extern int32_t useri_nextmsg;

extern maptool_pIMAGE useri_panoimage;

extern void useri_mainpop(void);

extern void useri_textautomenu(int32_t, int32_t, uint32_t, uint32_t,
                char, char [], uint32_t, char [], uint32_t,
                char [], uint32_t);

extern void useri_textautosize(int32_t, int32_t, uint32_t, uint32_t,
                char, char [], uint32_t);

extern void useri_say(char [], uint32_t, uint32_t, char);

extern void useri_killallmenus(void);

extern void useri_killmenuid(uint32_t);

extern void useri_loadconfig(char);

extern void useri_saveconfig(void);

extern void useri_postoconfig(struct aprsstr_POSITION);

extern void useri_rdlums(void);

extern void useri_xerrmsg(char [], uint32_t);

extern void useri_redraw(maptool_pIMAGE);

extern void useri_mouseleftdown(int32_t, int32_t);

extern void useri_mouserightdown(int32_t, int32_t);

extern void useri_keychar(char, char, char);

extern void useri_resizewin(int32_t, int32_t, char);

extern void useri_mousemove(int32_t, int32_t);

extern void useri_initmenus(void);

extern void useri_timerevent(void);

extern void useri_refreshwin(void);

extern void useri_mouserelease(void);

extern void useri_mousemiddle(int32_t, int32_t);

extern void useri_downloadprogress(void);

extern void useri_wrheap(void);

extern void useri_sayonoff(char [], uint32_t, char);

extern void useri_getview(uint8_t, uint32_t, float *,
                struct aprsstr_POSITION *);

extern void useri_refrinfo(void);

extern uint32_t useri_localtime(void);

extern void useri_refrlog(void);

extern int32_t useri_conf2int(uint8_t, uint32_t, int32_t, int32_t,
                int32_t);

extern float useri_conf2real(uint8_t, uint32_t, float, float,
                float);

extern void useri_confstr(uint8_t, char [], uint32_t);

extern void useri_conf2str(uint8_t, uint32_t, uint32_t, char,
                char [], uint32_t);

extern void useri_confstrings(uint8_t, uint32_t, char,
                char [], uint32_t);

extern uint8_t useri_confflags(uint8_t, uint32_t);

extern void useri_confappend(uint8_t, char [], uint32_t);

extern void useri_configbool(uint8_t, char);

extern char useri_configon(uint8_t);

extern void useri_AddConfLine(uint8_t, uint8_t, char [], uint32_t);

extern void useri_setview(int32_t, char [], uint32_t);

extern void useri_clrconfig(void);

extern void useri_starthint(uint32_t, char);

extern void useri_Setmap(uint32_t);

extern void useri_helptext(uint32_t, uint32_t, uint32_t, uint32_t,
                char [], uint32_t);

extern void useri_pulloff(void);

extern void useri_saveXYtocfg(uint8_t, int32_t, int32_t);

extern void useri_clrmsgtext(char [], uint32_t);

extern void useri_getstartxysize(int32_t *, int32_t *);

extern void useri_wrstrlist(char [], uint32_t, aprsdecode_MONCALL,
                struct aprsstr_POSITION, uint32_t);

extern void useri_wrstrmon(char [], uint32_t, struct aprsstr_POSITION);

extern void useri_clrcpmarks(void);

extern void useri_copypaste(char [], uint32_t);

extern char useri_isupdated(uint8_t);

extern int32_t useri_guesssize(char [], uint32_t, char [],
                uint32_t);
/* return filsize in kb not exact >2gb */

extern void useri_ColConfset(struct aprsdecode_COLTYP *, uint8_t);

extern char useri_gpsalt(uint8_t);

extern void useri_textbubble(struct aprsstr_POSITION, char [],
                uint32_t, char);

extern void useri_killbubble(void);

extern void useri_popwatchcall(char [], uint32_t);

extern void useri_allocimage(maptool_pIMAGE *, int32_t, int32_t,
                char);

extern uint32_t useri_mainys(void);

extern void useri_rdonesymb(char, char);

extern void useri_hoverinfo(struct aprsdecode_CLICKOBJECT);

extern void useri_int2cfg(uint8_t, int32_t);

extern void useri_resetimgparms(void);

extern void useri_poligonmenu(void);

extern void useri_Tilegamma0(float);

extern void useri_mapbritocfg(void);


extern void useri_BEGIN(void);


#endif /* useri_H_ */
