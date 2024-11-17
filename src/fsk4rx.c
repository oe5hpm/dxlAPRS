/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define fsk4rx_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef udp_H_
#include "udp.h"
#endif
#include <math.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#include <signal.h>

/* iq 4fsk, 2fsk, afsk demod with axudp and json output and sound monitor by oe5dxl */
#define fsk4rx_PI 3.1415926535

#define fsk4rx_PI2 6.283185307

#define fsk4rx_LF "\012"

#define fsk4rx_MAXINBUF 4096

#define fsk4rx_LOGNFSK 2

#define fsk4rx_NFSK 4

#define fsk4rx_OVERSAMP 16

#define fsk4rx_BAUD 100.0

#define fsk4rx_BASEBAND 140.0
/*Hz baseband lowpass */

#define fsk4rx_SPACEING 270.0
/*Hz between carriers */

#define fsk4rx_BASESTEP 5

#define fsk4rx_WAITBESTFRAME 32

#define fsk4rx_FRAME22 22

#define fsk4rx_FRAME32 32

#define fsk4rx_MAXDEMODS 256

#define fsk4rx_MAXSUBBANDS 256

#define fsk4rx_BFOFREQ 1500

#define fsk4rx_IFBAND 3240.0

#define fsk4rx_DDSSLEN 4096

#define fsk4rx_DDSLEN 16384

#define fsk4rx_AFSKOVERSAMPLE 16

#define fsk4rx_CHECKSNR 8.0
/* send new payload id only over this snr */

#define fsk4rx_MOD4 "4"

#define fsk4rx_MODA "a"

#define fsk4rx_MODF "f"

#define fsk4rx_MOD0 "0"

#define fsk4rx_MAXSHIPS 10000

struct Complex;


struct Complex {
   float Re;
   float Im;
};

struct LPCONTEXT24;


struct LPCONTEXT24 {
   float uc1;
   float uc2;
   float il;
   float LPR;
   float OLPR;
   float LPL;
};

typedef char FRAME[101];

struct DEMOD;


struct DEMOD {
   uint32_t lastn;
   uint32_t symclk;
   uint32_t framewp;
   uint32_t synword;
   int32_t ppm;
   FRAME frame;
   float afc;
   float levm;
   float noism;
};

typedef char FECINFO[101];

struct BESTFRAME;


struct BESTFRAME {
   uint32_t timer;
   uint32_t len;
   uint32_t cor;
   int32_t ppm;
   FRAME frame;
   float afc;
   float rfdb;
   float snr;
   char crcok;
   FECINFO fecinfo;
};

struct AFSKMODEM;

typedef struct AFSKMODEM * pAFSKMODEM;


struct AFSKMODEM {
   pAFSKMODEM next;
   char ademod;
   char demod;
   char rawbits;
   char scrambled;
   char oldbit;
   char oldd;
   char oldraw;
   char synd;
   int32_t sqlev;
   int32_t soundfd;
   int32_t dir;
   uint16_t usbosc;
   uint16_t iffreq;
   uint16_t iffreqafc;
   uint16_t ifosc;
   uint16_t freq[2];
   uint16_t osc[2];
   struct LPCONTEXT24 baselp[4];
   struct LPCONTEXT24 fsklp;
   struct LPCONTEXT24 iflpi;
   struct LPCONTEXT24 iflpq;
   uint32_t modemnum;
   uint32_t afbufw;
   uint32_t syncnt;
   uint32_t cnt;
   uint32_t baud;
   uint32_t bitcnt;
   uint32_t zerocnt;
   uint32_t ifsamp;
   uint32_t txc;
   uint32_t txd;
   uint32_t ifsamprate;
   uint32_t ifstep;
   float rflevel;
   float usbgain;
   float sq;
   float lastlev;
   float afskifwidth;
   float oldaf;
   float wavoldaf;
   float jmhz;
   float clamph;
   float clampl;
   float lastu;
   float noise;
   float qual;
   float fe;
   float afc;
   float afcmul;
   float afclimit;
   float level;
   uint32_t scrambler;
   char frame[351];
   short afbuf[4096];
};

struct FSK4MODEM;

typedef struct FSK4MODEM * pFSK4MODEM;


struct FSK4MODEM {
   pFSK4MODEM next;
   uint32_t modemnum;
   uint32_t afbufw;
   uint32_t subbands;
   uint32_t demods;
   uint32_t subspaceing;
   uint32_t maxframelen;
   uint32_t ifsamp;
   uint32_t ifstep;
   uint32_t baseoversamp;
   int32_t soundfd;
   char ademod;
   struct LPCONTEXT24 iflpi;
   struct LPCONTEXT24 iflpq;
   uint16_t usbosc;
   uint16_t iffreq;
   uint16_t ifosc;
   uint16_t freq[256];
   uint16_t osc[256];
   struct LPCONTEXT24 baselp[512];
   float rfdb;
   float usbgain;
   float wavoldaf;
   float baud;
   float ifsamprate;
   float ifbandwidth;
   float baseband;
   float spaceing;
   float jmhz;
   struct DEMOD demod[256];
   struct BESTFRAME bestframe;
   short afbuf[4096];
};

struct UDPDESTS;

typedef struct UDPDESTS * pUDPDESTS;


struct UDPDESTS {
   pUDPDESTS next;
   uint32_t ipnum;
   uint32_t toport;
   char udp2;
};

struct SHIP;

typedef struct SHIP * pSHIP;


struct SHIP {
   pSHIP next;
   uint32_t dimens;
   uint32_t mmsi;
   char name[20];
   char dest[20];
   char call[7];
   char vendorid[7];
   char eta[8];
   float draught;
   uint8_t typ;
};

struct CALLS;

typedef struct CALLS * pCALLS;


struct CALLS {
   pCALLS next;
   uint32_t num1;
   uint8_t valid;
   char call[16];
};

struct SHIPBEACONS;

typedef struct SHIPBEACONS * pSHIPBEACONS;


struct SHIPBEACONS {
   pSHIPBEACONS next;
   uint32_t mmsi;
   uint32_t sog;
   uint32_t time0;
};

static pFSK4MODEM fsk4modems;

static pAFSKMODEM afskmodems;

static char iqfn[1024];

static char ccfilename[1024];

static char jpipename[1024];

static char mycall[10];

static int32_t bfofreq;

static int32_t iqfd;

static int32_t aisfd;

static int32_t jsonfd;

static uint32_t insamprate;

static uint32_t isize;

static char ccdone;

static char verb;

static char verb2;

static int32_t udpsock;

static short iqbuf[8192];

static short DDS[16384];

static float DDSS[4096];

static uint32_t jipnum;

static uint32_t judpport;

static float squelch4;

static pUDPDESTS udpdests;

struct _0;


struct _0 {
   uint32_t payload;
   uint32_t time0;
};

static struct _0 checktable[11];

static uint32_t decodingtable[2048];

static pSHIP ships;

static pCALLS calls;

static char (* countrycodes)[32];

static uint32_t shipdrivebeacontime;

static uint32_t shipnodrivebeacontime;

static pSHIPBEACONS shipbeacons;


static void Error(const char text[], uint32_t text_len)
{
   osi_Werr("fsk4rx:", 8ul);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
} /* end Error() */


static char hex(uint32_t n, char cap)
{
   n = n&15UL;
   if (n<=9UL) return (char)(n+48UL);
   if (cap) return (char)(n+55UL);
   return (char)(n+87UL);
} /* end hex() */


static void HexStr(uint32_t x, uint32_t digits, uint32_t len, char cap, char s[], uint32_t s_len)
{
   uint32_t i0;
   if (digits>s_len-1) digits = s_len-1;
   i0 = digits;
   while (i0<len && i0<s_len-1) {
      s[i0] = ' ';
      ++i0;
   }
   s[i0] = 0;
   while (digits>0UL) {
      --digits;
      s[digits] = hex(x, cap);
      x = x/16UL;
   }
} /* end HexStr() */


static void WrHexCap(uint32_t x, uint32_t digits, uint32_t len)
{
   char s[256];
   HexStr(x, digits, len, 1, s, 256ul);
   osi_WrStr(s, 256ul);
} /* end WrHexCap() */


static float sqr(float x)
{
   return x*x;
} /* end sqr() */


static float atan20(const struct Complex u)
{
   float w;
   struct Complex abs0;
   abs0.Re = (float)fabs(u.Re);
   abs0.Im = (float)fabs(u.Im);
   if (abs0.Im>abs0.Re) {
      if (abs0.Im>0.0f) w = X2C_DIVR(abs0.Re,abs0.Im);
      else w = 0.0f;
      w = 1.57079632675f-(w*1.055f-w*w*0.267f); /* arctan */
   }
   else {
      if (abs0.Re>0.0f) w = X2C_DIVR(abs0.Im,abs0.Re);
      else w = 0.0f;
      w = w*1.055f-w*w*0.267f;
   }
   if (u.Re<0.0f) w = 3.1415926535f-w;
   if (u.Im<0.0f) w = -w;
   return w;
} /* end atan2() */


static float dB(float u)
{
   if (u>0.0001f) return (float)(log((double)u)*8.68588963);
   return 0.0f;
} /* end dB() */


static float fmhighpass(float w, float * w1)
{
   float af;
   /* phase highpass make FM out of phase */
   af = w-*w1;
   *w1 = w;
   if (af>3.1415926535f) af = af-6.283185307f;
   if (af<(-3.1415926535f)) af = af+6.283185307f;
   return af;
} /* end fmhighpass() */

#define fsk4rx_bytes 2


static void wwav(int32_t fd, uint32_t hz)
{
   char b[44];
   strncpy(b,"RIFF    WAVEfmt ",44u);
   b[4U] = '\377'; /* len */
   b[5U] = '\377';
   b[6U] = '\377';
   b[7U] = '\377';
   b[16U] = '\020';
   b[17U] = 0;
   b[18U] = 0;
   b[19U] = 0;
   b[20U] = '\001'; /* PCM/ALAW */
   b[21U] = 0;
   b[22U] = '\001'; /* channels */
   b[23U] = 0;
   b[24U] = (char)(hz&255UL); /* samp */
   b[25U] = (char)(hz/256UL&255UL);
   b[26U] = (char)(hz/65536UL&255UL);
   b[27U] = (char)(hz/16777216UL);
   b[28U] = (char)(hz*2UL&255UL); /* byte/s */
   b[29U] = (char)((hz*2UL)/256UL&255UL);
   b[30U] = (char)((hz*2UL)/65536UL);
   b[31U] = 0;
   b[32U] = '\002'; /* block byte */
   b[33U] = 0;
   b[34U] = '\020'; /* bit/samp */
   b[35U] = 0;
   b[36U] = 'd';
   b[37U] = 'a';
   b[38U] = 't';
   b[39U] = 'a';
   b[40U] = '\377'; /* len */
   b[41U] = '\377';
   b[42U] = '\377';
   b[43U] = '\377';
   if (fd>=0L) osi_WrBin(fd, (char *)b, 44u/1u, 44UL);
} /* end wwav() */


static void MakeDDS(float dds[], uint32_t dds_len)
{
   uint32_t i0;
   float r;
   uint32_t tmp;
   r = X2C_DIVR(6.283185307f,(float)((dds_len-1)+1UL));
   tmp = dds_len-1;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      dds[i0] = (float)sin((double)((float)i0*r));
      if (i0==tmp) break;
   } /* end for */
} /* end MakeDDS() */


static void MakeDDSi(short dds[], uint32_t dds_len)
{
   uint32_t i0;
   float r;
   uint32_t tmp;
   r = X2C_DIVR(6.283185307f,(float)((dds_len-1)+1UL));
   tmp = dds_len-1;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      dds[i0] = (short)X2C_TRUNCI(16383.9*sin((double)((float)i0*r)),-32768,32767);
      if (i0==tmp) break;
   } /* end for */
} /* end MakeDDSi() */


static void makelp24(float fg, float samp, struct LPCONTEXT24 * c)
{
   struct LPCONTEXT24 * anonym;
   { /* with */
      struct LPCONTEXT24 * anonym = c;
      anonym->LPR = (X2C_DIVR(fg,samp))*2.33363f;
      anonym->LPL = (float)((double)(anonym->LPR*anonym->LPR*2.888f)*(1.0-9.0*pow((double)(X2C_DIVR(fg,
                samp)), 2.0)));
      anonym->OLPR = 1.0f-anonym->LPR;
   }
} /* end makelp24() */

#define fsk4rx_DEFAULTIP 0x7F000001 

#define fsk4rx_PORTSEP ":"


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * ip, uint32_t * port)
{
   uint32_t p;
   uint32_t n;
   uint32_t i0;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i0 = 0UL; i0<=4UL; i0++) {
      if (i0>=3UL || h[0UL]!=':') {
         n = 0UL;
         ok0 = 0;
         while ((uint8_t)h[p]>='0' && (uint8_t)h[p]<='9') {
            ok0 = 1;
            n = (n*10UL+(uint32_t)(uint8_t)h[p])-48UL;
            ++p;
         }
         if (!ok0) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      if (i0<3UL) {
         if (h[0UL]!=':') {
            if (h[p]!='.' || n>255UL) {
               GetIp_ret = -1L;
               goto label;
            }
            *ip =  *ip*256UL+n;
         }
      }
      else if (i0==3UL) {
         if (h[0UL]!=':') {
            *ip =  *ip*256UL+n;
            if (h[p]!=':' || n>255UL) {
               GetIp_ret = -1L;
               goto label;
            }
         }
         else {
            p = 0UL;
            *ip = 2130706433UL;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++p;
   } /* end for */
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static int32_t createnonblockfile(char fn[], uint32_t fn_len)
{
   int32_t fd;
   int32_t createnonblockfile_ret;
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenNONBLOCK(fn, fn_len);
   if (fd<0L || !osi_IsFifo(fd)) {
      /* no pipe */
      if (fd>=0L) osic_Close(fd);
      fd = osi_OpenWrite(fn, fn_len);
   }
   createnonblockfile_ret = fd;
   X2C_PFREE(fn);
   return createnonblockfile_ret;
} /* end createnonblockfile() */


static uint32_t checkcall(const char call[], uint32_t call_len)
{
   uint32_t nc;
   uint32_t nn;
   uint32_t j;
   uint32_t i0;
   j = 0UL;
   while ((j<=call_len-1 && call[j]) && call[j]!='-') ++j;
   nn = 0UL;
   nc = 0UL;
   i0 = 0UL;
   while (i0<j) {
      if ((uint8_t)call[i0]>='A' && (uint8_t)call[i0]<='Z') ++nc;
      else if ((i0<=2UL && (uint8_t)call[i0]>='0') && (uint8_t)call[i0]<='9') ++nn;
      ++i0;
   }
   if ((((j<=6UL && j>=3UL) && nn+nc==j) && nn>=1UL) && nn<=2UL) {
      /* looks like a ham callsign */
      if (j<=call_len-1 && call[j]=='-') {
         /* looks some ssid */
         ++j;
         if ((j<=call_len-1 && (uint8_t)call[j]>='0') && (uint8_t)call[j]<='9') {
            ++j;
            if ((j<=call_len-1 && (uint8_t)call[j]>='0') && (uint8_t)call[j]<='9') ++j;
            return j;
         }
         if (aprsstr_Length(call, call_len)<=9UL) return aprsstr_Length(call, call_len);
      }
      else {
         /* strange ssid but fits in object */
         return j;
      }
   }
   return 0UL;
} /* end checkcall() */


static char callbookread(const char fn[], uint32_t fn_len)
{
   int32_t f;
   pCALLS p;
   char ch;
   uint32_t st;
   f = osi_OpenRead(fn, fn_len);
   if (f<0L) return 0;
   p = 0;
   st = 0UL;
   while (osi_RdBin(f, (char *) &ch, 1u/1u, 1UL)==1L) {
      if (st==0UL && ch=='#') st = 9UL;
      if (st!=9UL) {
         if (p==0) {
            osic_alloc((char * *) &p, sizeof(struct CALLS));
            if (p==0) Error("out of memory", 14ul);
            p->num1 = 0UL;
            p->call[0] = 0;
            st = 0UL;
         }
         if ((uint8_t)ch>' ') {
            if (st<=1UL) {
               if ((uint8_t)ch>='0' && (uint8_t)ch<='9') {
                  p->num1 = (p->num1*10UL+(uint32_t)(uint8_t)ch)-48UL;
                  st = 1UL;
               }
               else if (st==1UL) st = 2UL;
               else st = 9UL;
            }
            else if (st<=3UL && ch!=',') {
               aprsstr_Append(p->call, 16ul, (char *) &ch, 1u/1u);
               st = 3UL;
            }
         }
         else if (st==3UL) {
            p->valid = (uint8_t)checkcall(p->call, 16ul);
            if (verb2) {
               osic_WrUINT32(p->num1, 1UL);
               osi_WrStr(":", 2ul);
               osi_WrStr(p->call, 16ul);
               osi_WrStr(" valid-len:", 12ul);
               osic_WrUINT32((uint32_t)p->valid, 1UL);
               osi_WrStrLn("", 1ul);
            }
            p->next = calls;
            calls = p;
            p = 0;
         }
      }
      else if ((uint8_t)ch<' ') st = 0UL;
   }
   osic_Close(f);
   return 1;
} /* end callbookread() */


static void findcall(uint32_t n, char name[], uint32_t name_len, char nocall[], uint32_t nocall_len)
{
   pCALLS p;
   name[0UL] = 0;
   nocall[0UL] = 0;
   p = calls;
   while (p) {
      if (p->num1==n) {
         /* call found */
         if (p->valid>0U) {
            aprsstr_Assign(name, name_len, p->call, 16ul);
            if ((uint32_t)p->valid<=name_len-1) {
               name[p->valid] = 0; /* strip junk after valid callsign */
            }
            while (aprsstr_Length(name, name_len)<9UL) aprsstr_Append(name, name_len, " ", 2ul);
         }
         else aprsstr_Assign(nocall, nocall_len, p->call, 16ul);
         return;
      }
      p = p->next;
   }
} /* end findcall() */


static void newmodem(uint32_t * sqlev, uint32_t * afskhightone, uint32_t * afsklowtone, uint32_t * ifwid,
                uint32_t * afclim, uint32_t * fskbaud, uint32_t * afskbaud, uint32_t * scramble0,
                pAFSKMODEM * pa, uint32_t stored, float * jmhz, int32_t * bfof, char * ademod,
                int32_t * soundfd, float * baud, float baseband, float * spaceing, float * ifbandwidth,
                pFSK4MODEM * p4, char mod, int32_t tune)
{
   uint32_t i0;
   uint32_t tmp;
   if ((uint32_t)labs(tune)*2UL>insamprate) Error("tuned outside bandwidth (-t)", 29ul);
   if (mod=='4') {
      osic_alloc((char * *)p4, sizeof(struct FSK4MODEM));
      if (*p4==0) Error("out of memory", 14ul);
      memset((char *)*p4,(char)0,sizeof(struct FSK4MODEM));
      (*p4)->ifbandwidth = *ifbandwidth;
      (*p4)->subspaceing = (uint32_t)X2C_TRUNCC(X2C_DIVR(*spaceing,baseband*0.4f),0UL,X2C_max_longcard);
                /* stay in baseband window */
      (*p4)->subbands = (uint32_t)X2C_TRUNCC(X2C_DIVR(*ifbandwidth,X2C_DIVR(*spaceing,(float)(*p4)->subspaceing)),
                0UL,X2C_max_longcard);
      (*p4)->demods = (*p4)->subbands-3UL*(*p4)->subspaceing;
      (*p4)->maxframelen = 63UL;
      (*p4)->ifsamprate = 80.0f* *baud;
      (*p4)->ifstep = (uint32_t)X2C_TRUNCC((X2C_DIVR((*p4)->ifsamprate,(float)insamprate))*4.294967295E+9f,0UL,
                X2C_max_longcard);
      (*p4)->soundfd = *soundfd;
      (*p4)->ademod = *ademod;
      (*p4)->spaceing = *spaceing;
      (*p4)->baseband = baseband;
      (*p4)->baud = *baud;
      makelp24((X2C_DIVR((*p4)->spaceing,(float)(*p4)->subspaceing))*(float)(*p4)->subbands*0.65f,
                (float)insamprate, &(*p4)->iflpi);
      makelp24((X2C_DIVR((*p4)->spaceing,(float)(*p4)->subspaceing))*(float)(*p4)->subbands*0.65f,
                (float)insamprate, &(*p4)->iflpq);
      tmp = (*p4)->subbands-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         makelp24((*p4)->baseband*0.5f, (*p4)->ifsamprate, &(*p4)->baselp[i0*2UL]);
         makelp24((*p4)->baseband*0.5f, (*p4)->ifsamprate, &(*p4)->baselp[i0*2UL+1UL]);
         (*p4)->freq[i0] = (uint16_t)(int32_t)X2C_TRUNCI(X2C_DIVR((X2C_DIVR(4096.0f*(*p4)->spaceing,
                (float)(*p4)->subspaceing))*(((float)i0-(float)((*p4)->subbands/2UL))+0.5f),(*p4)->ifsamprate),
                X2C_min_longint,X2C_max_longint);
         if (i0==tmp) break;
      } /* end for */
      bfofreq = (int32_t)X2C_TRUNCI(X2C_DIVR((float)*bfof*0.25f*16384.0f,(*p4)->ifsamprate),X2C_min_longint,
                X2C_max_longint);
      (*p4)->jmhz = *jmhz;
      if ((*p4)->soundfd>=0L) {
         wwav((*p4)->soundfd, (uint32_t)X2C_TRUNCC((*p4)->ifsamprate,0UL,X2C_max_longcard));
      }
      *ademod = 0;
      *soundfd = -1L;
      (*p4)->iffreq = (uint16_t)(int32_t)X2C_TRUNCI(X2C_DIVR(16384.0f*(float)tune,(float)insamprate),
                X2C_min_longint,X2C_max_longint);
      (*p4)->modemnum = stored;
      (*p4)->next = fsk4modems;
      fsk4modems = *p4;
      if (verb) {
         osi_WrStrLn("", 1ul);
         osi_WrStr("offset:", 8ul);
         osic_WrINT32((uint32_t)tune, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("4fsk spaceing:", 15ul);
         osic_WrFixed((*p4)->spaceing, 2L, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("baud:", 6ul);
         osic_WrFixed((*p4)->baud, 2L, 1UL);
         osi_WrStrLn("", 1ul);
         osi_WrStr("if-bandwidth:", 14ul);
         osic_WrFixed((*p4)->ifbandwidth, 2L, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("demodbandwidth:", 16ul);
         osic_WrFixed((*p4)->baseband, 2L, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("demodulators:", 14ul);
         osic_WrUINT32((*p4)->demods, 1UL);
         osi_WrStrLn("", 1ul);
         osi_WrStr("demodulator every:", 19ul);
         osic_WrFixed(X2C_DIVR((*p4)->spaceing,(float)(*p4)->subspaceing), 2L, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("if-samplerate:", 15ul);
         osic_WrFixed((*p4)->ifsamprate, 2L, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("afc-range +/-:", 15ul);
         osic_WrFixed(X2C_DIVR((float)((*p4)->demods/2UL)*(*p4)->spaceing,(float)(*p4)->subspaceing), 1L, 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("squelch:", 9ul);
         osic_WrFixed(squelch4, 1L, 1UL);
         osi_WrStrLn("dB", 3ul);
         osi_WrStrLn("", 1ul);
      }
   }
   else if ((mod=='a' || mod=='f') || mod=='0') {
      osic_alloc((char * *)pa, sizeof(struct AFSKMODEM));
      if (*pa==0) Error("out of memory", 14ul);
      memset((char *)*pa,(char)0,sizeof(struct AFSKMODEM));
      (*pa)->ademod = *ademod;
      (*pa)->demod = mod;
      (*pa)->scrambled = *scramble0==1UL;
      (*pa)->rawbits = *scramble0==2UL;
      (*pa)->baud = *afskbaud;
      if ((*pa)->demod=='f') {
         (*pa)->ifsamprate = 19200UL;
         if ( *fskbaud*2UL>(*pa)->ifsamprate) (*pa)->ifsamprate =  *fskbaud*2UL;
         (*pa)->baud = *fskbaud;
      }
      else if ((*pa)->demod=='a') {
         (*pa)->ifsamprate = (*pa)->baud*16UL;
         (*pa)->baud = *afskbaud;
      }
      else (*pa)->ifsamprate = *fskbaud;
      (*pa)->afclimit = (X2C_DIVR((float)*afclim,(float)(*pa)->ifsamprate))*6.283185307f;
      (*pa)->afcmul = (X2C_DIVR(16384.0f,(float)insamprate))*(float)(*pa)->ifsamprate*1.5915494309644E-1f;
      (*pa)->jmhz = *jmhz;
      (*pa)->afskifwidth = (float)*ifwid;
      (*pa)->soundfd = *soundfd;
      if ((*pa)->soundfd>=0L) wwav((*pa)->soundfd, (*pa)->ifsamprate);
      (*pa)->ifstep = (uint32_t)X2C_TRUNCC((X2C_DIVR((float)(*pa)->ifsamprate,
                (float)insamprate))*4.294967295E+9f,0UL,X2C_max_longcard);
      makelp24((*pa)->afskifwidth*0.5f, (float)insamprate, &(*pa)->iflpi);
      makelp24((*pa)->afskifwidth*0.5f, (float)insamprate, &(*pa)->iflpq);
      makelp24((float)(*pa)->baud*0.5f, (float)(*pa)->ifsamprate, &(*pa)->fsklp);
      if ((*pa)->demod=='a') {
         (*pa)->freq[0U] = (uint16_t)(int32_t)X2C_TRUNCI(X2C_DIVR(4096.0f*(float)*afsklowtone+0.5f,
                (float)(*pa)->ifsamprate),X2C_min_longint,X2C_max_longint);
         (*pa)->freq[1U] = (uint16_t)(int32_t)X2C_TRUNCI(X2C_DIVR(4096.0f*(float)*afskhightone+0.5f,
                (float)(*pa)->ifsamprate),X2C_min_longint,X2C_max_longint);
         for (i0 = 0UL; i0<=3UL; i0++) {
            makelp24((float)(*pa)->baud*0.5f, (float)(*pa)->ifsamprate, &(*pa)->baselp[i0]);
         } /* end for */
      }
      bfofreq = (int32_t)X2C_TRUNCI(X2C_DIVR((float)*bfof*0.25f*16384.0f,(float)(*pa)->ifsamprate),
                X2C_min_longint,X2C_max_longint);
      *soundfd = -1L;
      (*pa)->sqlev = (int32_t)*sqlev;
      *sqlev = 0UL;
      (*pa)->modemnum = stored;
      (*pa)->next = afskmodems;
      (*pa)->iffreq = (uint16_t)(int32_t)X2C_TRUNCI(X2C_DIVR(16384.0f*(float)tune,(float)insamprate),
                X2C_min_longint,X2C_max_longint);
      (*pa)->iffreqafc = (*pa)->iffreq;
      afskmodems = *pa;
      if (verb) {
         osi_WrStrLn("", 1ul);
         osi_WrStr("offset:", 8ul);
         osic_WrINT32((uint32_t)tune, 1UL);
         osi_WrStrLn("Hz", 3ul);
         if (*afclim>0UL) {
            osi_WrStr("max.afc:+-", 11ul);
            osic_WrINT32(*afclim, 1UL);
            osi_WrStrLn("Hz", 3ul);
         }
         osi_WrStr("if-bandwidth:", 14ul);
         osic_WrINT32((uint32_t)(int32_t)X2C_TRUNCI((*pa)->afskifwidth,X2C_min_longint,X2C_max_longint), 1UL);
         osi_WrStrLn("Hz", 3ul);
         osi_WrStr("if-samplerate:", 15ul);
         osic_WrUINT32((*pa)->ifsamprate, 1UL);
         osi_WrStrLn("Hz", 3ul);
         if ((*pa)->afskifwidth>(float)(*pa)->ifsamprate) {
            osi_WrStrLn("bandwidth should be lower than samplerate!", 43ul);
         }
         if ((*pa)->demod=='f') {
            osi_WrStr("fsk ", 5ul);
            if ((*pa)->scrambled) osi_WrStr("scrambled ", 11ul);
         }
         else if ((*pa)->demod=='a') osi_WrStr("afsk ", 6ul);
         if ((*pa)->demod=='0') {
            osi_WrStr("squelch:", 9ul);
            osic_WrUINT32((uint32_t)(*pa)->sqlev, 1UL);
            osi_WrStrLn("", 1ul);
            if ((*pa)->soundfd<0L) osi_WrStr("fm demod with no audio output?", 31ul);
         }
         else {
            osi_WrStr("baud:", 6ul);
            osic_WrUINT32((*pa)->baud, 1UL);
         }
         if ((*pa)->demod=='a') {
            osi_WrStr(" low tone:", 11ul);
            osic_WrUINT32(*afsklowtone, 1UL);
            osi_WrStr(" ", 2ul);
            osi_WrStr("high tone:", 11ul);
            osic_WrUINT32(*afskhightone, 1UL);
         }
         osi_WrStrLn("", 1ul);
      }
   }
} /* end newmodem() */


static void num(uint32_t * v, const char h[], uint32_t h_len, uint32_t * i0)
{
   uint32_t n;
   char sg;
   if (h[*i0]!=',') {
      sg = 0;
      n = 0UL;
      if (h[*i0]=='+') ++*i0;
      else if (h[*i0]=='-') {
         sg = 1;
         ++*i0;
      }
      while (((*i0<h_len-1 && h[*i0]!=',') && (uint8_t)h[*i0]>='0') && (uint8_t)h[*i0]<='9') {
         n = (n*10UL+(uint32_t)(uint8_t)h[*i0])-48UL;
         ++*i0;
      }
      if (sg) n = (uint32_t) -(int32_t)n;
      *v = n;
   }
} /* end num() */


static void Parms(void)
{
   char hasudp;
   char err;
   uint32_t i0;
   uint32_t afskbaud;
   uint32_t fskbaud;
   uint32_t n;
   uint32_t afclim;
   uint32_t afskhightone;
   uint32_t afsklowtone;
   uint32_t ifwid;
   uint32_t scramble0;
   uint32_t stored;
   uint32_t sqlev;
   char h[1024];
   pUDPDESTS udest;
   int32_t soundfd;
   int32_t tune;
   int32_t bfof;
   float jmhz;
   float ifbandwidth;
   float baud;
   float baseband;
   float spaceing;
   char ademod;
   char mod;
   pFSK4MODEM p4;
   pAFSKMODEM pa;
   mod = '4';
   ademod = 0;
   soundfd = -1L;
   jmhz = 0.0f;
   iqfn[0] = 0;
   isize = 1UL;
   verb = 0;
   verb2 = 0;
   squelch4 = 8.0f;
   err = 0;
   udpsock = -1L;
   jpipename[0] = 0;
   judpport = 0UL;
   jsonfd = -1L;
   aisfd = -1L;
   insamprate = 0UL;
   tune = 0L;
   udpdests = 0;
   spaceing = 270.0f;
   baseband = 140.0f;
   ifbandwidth = 3240.0f;
   baud = 100.0f;
   afskbaud = 1200UL;
   fskbaud = 9600UL;
   ifwid = 16000UL;
   afsklowtone = 1200UL;
   afskhightone = 2200UL;
   stored = 0UL;
   sqlev = 0UL;
   afclim = 0UL;
   bfof = 1500L;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            osi_NextArg(iqfn, 1024ul);
            if (iqfn[0U]==0 || iqfn[0U]=='-') Error("-i <iqfilename>", 16ul);
         }
         else if (h[1U]=='S') {
            if (soundfd>=0L) Error("only FM or SSB", 15ul);
            osi_NextArg(h, 1024ul);
            ademod = 'S';
            if (h[0U]==0 || h[0U]=='-') Error("-S <usb-file/pipe-name>", 24ul);
            soundfd = createnonblockfile(h, 1024ul);
            if (soundfd<0L) Error("usbfile create", 15ul);
         }
         else if (h[1U]=='F') {
            if (soundfd>=0L) Error("only FM or SSB", 15ul);
            osi_NextArg(h, 1024ul);
            ademod = 'F';
            if (h[0U]==0 || h[0U]=='-') Error("-F <fm-file/pipe-name>", 23ul);
            soundfd = createnonblockfile(h, 1024ul);
            if (soundfd<0L) Error("fmfile create", 14ul);
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if ((h[0U]=='i' && h[1U]=='1') && h[2U]=='6') isize = 2UL;
            else if (h[0U]=='u' && h[1U]=='8') isize = 1UL;
            else if ((h[0U]=='f' && h[1U]=='3') && h[2U]=='2') isize = 4UL;
            else Error("-f u8|i16|f32", 14ul);
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&jmhz, h, 1024ul)) Error("-M <MHz>", 9ul);
         }
         else if (h[1U]=='W') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&ifbandwidth, h, 1024ul) || ifbandwidth<100.0f) {
               Error("-W <ifbandwidth-Hz>", 20ul);
            }
         }
         else if (h[1U]=='B') osi_NextArg(h, 1024ul);
         else if (h[1U]=='m') {
            if (stored>0UL) {
               newmodem(&sqlev, &afskhightone, &afsklowtone, &ifwid, &afclim, &fskbaud, &afskbaud, &scramble0, &pa,
                stored, &jmhz, &bfof, &ademod, &soundfd, &baud, baseband, &spaceing, &ifbandwidth, &p4, mod, tune);
            }
            ++stored;
            osi_NextArg(h, 1024ul);
            mod = h[0U];
            if (mod=='a') {
               i0 = 1UL;
               if (h[i0]==',') {
                  ++i0;
                  num(&afskbaud, h, 1024ul, &i0);
               }
               if (afskbaud==2400UL) {
                  afsklowtone = 1995UL;
                  afskhightone = 3658UL;
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&afsklowtone, h, 1024ul, &i0);
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&afskhightone, h, 1024ul, &i0);
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&ifwid, h, 1024ul, &i0);
               }
               if (h[i0]) Error("-m a,<baud>,<low>,<hightone>,<if-width>", 40ul);
            }
            else if (mod=='f') {
               scramble0 = 1UL;
               i0 = 1UL;
               if (h[i0]==',') {
                  ++i0;
                  num(&fskbaud, h, 1024ul, &i0);
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&ifwid, h, 1024ul, &i0);
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&scramble0, h, 1024ul, &i0);
               }
               if (h[i0]) Error("-m a,<baud>,<if-width>,<scrambler>", 35ul);
            }
            else if (mod=='0') {
               i0 = 1UL;
               if (h[i0]==',') {
                  ++i0;
                  num(&fskbaud, h, 1024ul, &i0);
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&ifwid, h, 1024ul, &i0);
               }
               if (h[i0]==',') {
                  ++i0;
                  num(&sqlev, h, 1024ul, &i0);
               }
               if (h[i0]) Error("-m 0,<sampelrate>,<if-width>,<squelch>", 39ul);
            }
            else if (mod!='4') Error("-m f,<baud>,<if-width>", 23ul);
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&baud, h, 1024ul) || baud<1.0f) Error("-b <baud>", 10ul);
         }
         else if (h[1U]=='I') {
            osi_NextArg(mycall, 10ul);
            if (aprsstr_Length(mycall, 10ul)<3UL || aprsstr_Length(mycall, 10ul)>9UL) {
               Error("-I <callsign-ssid", 18ul);
            }
         }
         else if (h[1U]=='t') {
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            num(&n, h, 1024ul, &i0);
            tune = (int32_t)n;
            if (h[i0]==',') {
               ++i0;
               num(&afclim, h, 1024ul, &i0);
            }
            if (h[i0]) Error("-t <+-Hz>[,<Hz>]", 17ul);
         }
         else if (h[1U]=='T') {
            osi_NextArg(h, 1024ul);
            i0 = 0UL;
            num(&n, h, 1024ul, &i0);
            shipdrivebeacontime = n;
            if (h[i0]==',') {
               ++i0;
               num(&shipnodrivebeacontime, h, 1024ul, &i0);
            }
            if (h[i0]) Error("-T <beacontime>[,<beacontimenomove>]", 37ul);
            if (shipdrivebeacontime>shipnodrivebeacontime) {
               shipnodrivebeacontime = shipdrivebeacontime;
            }
         }
         else if (h[1U]=='D') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&spaceing, h, 1024ul) || spaceing<1.0f) Error("-D <Hz>", 8ul);
         }
         else if (h[1U]=='q') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&squelch4, h, 1024ul)) {
               Error("-q <dB>", 8ul);
            }
            verb = 1;
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &insamprate) || insamprate<100UL) {
               Error("-r <samplerate-Hz>", 19ul);
            }
         }
         else if (h[1U]=='n') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &bfof)) Error("-n <bfo-frequency-Hz>", 22ul);
         }
         else if (h[1U]=='U' || h[1U]=='L') {
            osic_alloc((char * *) &udest, sizeof(struct UDPDESTS));
            if (udest==0) Error("out of memory", 14ul);
            udest->udp2 = h[1U]=='L'; /* switch on axudp2 */
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &udest->ipnum, &udest->toport)<0L) {
               Error("-U or -L ip:port number", 24ul);
            }
            udest->next = udpdests;
            udpdests = udest;
            hasudp = 1;
         }
         else if (h[1U]=='J') {
            hasudp = 1;
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &jipnum, &judpport)<0L) Error("-J ip:port number", 18ul);
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            aisfd = osi_OpenRead(h, 1024ul);
            if (aisfd<0L) aisfd = osi_OpenWrite(h, 1024ul);
            else {
               osic_Close(aisfd);
               aisfd = osi_OpenAppend(h, 1024ul);
            }
            if (aisfd<0L) Error("-A <filename>", 14ul);
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!callbookread(h, 1024ul)) Error("-C <callbook> file not readable", 32ul);
         }
         else if (h[1U]=='Y') osi_NextArg(ccfilename, 1024ul);
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("  Decode AFSK, FSK, 4FSK out of IQ-File/Pipe by oe5dxl", 55ul);
               osi_WrStrLn("  output data in udp, axudp or json + output pipe or file (wav) FM or USB", 74ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn(" -A <filename>      write (append) ais shipdata to file", 56ul);
               osi_WrStrLn(" -B <Hz>            4fsk demodulator carriers filter bandwidth (140)", 69ul);
               osi_WrStrLn(" -b <baud>          4fsk baudrate, symbols/s (100)", 51ul);
               osi_WrStrLn(" -C <filename>      read callsign database to replace payload number if valid callsign found\
 (<num>,<call>)", 108ul);
               osi_WrStrLn(" -D <Hz>            deviation, spaceing between 2 4fsk tones (270)", 67ul);
               osi_WrStrLn(" -F <filename>      output FM demodulated audio wav to file or (unbreakable) pipe", 82ul);
               osi_WrStrLn(" -f u8|i16|f32      IQ data format (f32 slow", 45ul);
               osi_WrStrLn(" -h                 this", 25ul);
               osi_WrStrLn(" -I <callsign-ssid> Callsign of APRS-object sender (NOCALL)", 60ul);
               osi_WrStrLn(" -i <file>          IQ-filename or pipe from sdr receiver", 58ul);
               osi_WrStrLn(" -J <x.x.x.x:destport>  send demodulated data(base64) with metadata in json", 76ul);
               /*        WrStrLn(" -j <file/pipe>     write demodulated data(base64)
                with metadata in json to file or (unbreakable) pipe"); */
               osi_WrStrLn(" -L <x.x.x.x:destport>  as -U but AXUDPv2 with metadata for igate", 66ul);
               osi_WrStrLn(" -M <MHz>           pass through rx frequency to json metadata and aprs comment -M 437.6",
                89ul);
               osi_WrStrLn(" -m 4               demodulate 4fsk (default)", 46ul);
               osi_WrStrLn(" -m a[,<baud>[,<low>[,<hightone>[,<if-width>]]]] afsk (-m a,1200,1200,2400,12500", 81ul);
               osi_WrStrLn(" -m f[,<baud>[,<if-width>[,<sramble>]]] fsk, scramble=2 raw hdlc, -m f,9600,16000,1", 84ul);
               osi_WrStrLn(" -m 0[,<samplerate>[,<if-width>,<squelch>]]] no demodulator, use for audio pipe -m 0,16000,1\
2500 -F af.wav", 107ul);
               osi_WrStrLn("                      may be repeated as cpu is able to do with following other -t ", 84ul);
               osi_WrStrLn(" -n <Hz>            bfo frequency Hz for ssb audio output, -1500 for lsb (1500)", 80ul);
               osi_WrStrLn(" -q <dB>            verbous like -v but show only frames with crc ok or SNR over <dB> (8)",
                90ul);
               osi_WrStrLn("                      set to 0 to show all, 100 for frames with crc ok, axudp sent if crc ok\
", 93ul);
               osi_WrStrLn(" -r <Hz>            iq samplerate", 34ul);
               osi_WrStrLn(" -S <filename>      output USB demodulated audio wav to file or (unbreakable) pipe", 83ul);
               osi_WrStrLn(" -T <s>[,<s>]       min. Ship beacon time moveing, not moving (0)", 66ul);
               osi_WrStrLn(" -t <+-Hz>[,<Hz>]   Shift rx-frequency inside IQ band (avoid near 0 where is adc birdy) (0)",
                 92ul);
               osi_WrStrLn("                      afc-follow-range Hz, 90% in about 10ms, 0=afc off (0)", 76ul);
               /*        WrStrLn(" -N                 fsk steps (4)"); */
               osi_WrStrLn(" -U <x.x.x.x:destport>  send frame in AXUDP, may be repeated to more destinations", 82ul);
               /*        WrStrLn(" -V                 very verbous"); */
               osi_WrStrLn(" -v                 verbous", 28ul);
               osi_WrStrLn(" -W <Hz>            if-bandwidth 4fsk (2970)", 45ul);
               osi_WrStrLn(" -Y <filename>      read AIS countrynames database (<num> <name>)", 66ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("  mknod a.wav p", 16ul);
               osi_WrStrLn("  rtl_sdr -f 437.5m -s 1024000 - | ./fsk4rx -i /dev/stdin -f u8 -v -r 1024000 -t 100000 -L 1\
27.0.0.1:9001 -M 437.6 -I MYCALL-11 -S a.wav", 137ul);
               osi_WrStrLn("  usb rx: rtl_sdr -f 144.4m -s 1024000 - | ./fsk4rx -i /dev/stdin -f u8 -v -r 1024000 -t 280\
00 -m 0,16000,2400 -S a.wav", 120ul);
               osi_WrStrLn("  AIS rx: rtl_sdr -f 162.0m -s 1024000 -g 49 - | ./fsk4rx -i /dev/stdin -f u8 -v -r 1024000 \
-m f,9600,11000,2 -t -25000 -m f,9600,11000,2 -t 25000 -U 127.0.0.1:9000", 165ul);
               osi_WrStrLn("  aplay a.wav", 14ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      osi_Werr(">", 2ul);
      osi_Werr(h, 1024ul);
      osi_Werr("< use -h\012", 10ul);
      X2C_ABORT();
   }
   newmodem(&sqlev, &afskhightone, &afsklowtone, &ifwid, &afclim, &fskbaud, &afskbaud, &scramble0, &pa, stored, &jmhz,
                &bfof, &ademod, &soundfd, &baud, baseband, &spaceing, &ifbandwidth, &p4, mod, tune);
   if (hasudp) {
      if (udpsock<0L) udpsock = openudp();
      if (udpsock<0L) Error("cannot open udp socket", 23ul);
   }
   if (insamprate==0UL) Error("need input samplerate (-r)", 27ul);
} /* end Parms() */


static char b64(uint32_t c)
{
   c = c&63UL;
   if (c<26UL) return (char)(c+65UL);
   else if (c<52UL) return (char)(c+71UL);
   else if (c<62UL) return (char)((int32_t)c+(-4L));
   else if (c==62UL) return '+';
   else return '/';
   return 0;
} /* end b64() */


static void enc64(uint32_t b, uint32_t n, char s[], uint32_t s_len)
{
   uint32_t i0;
   for (i0 = n; i0<=2UL; i0++) {
      b = b*256UL;
   } /* end for */
   s[2UL] = '=';
   s[3UL] = '=';
   s[4UL] = 0;
   s[0UL] = b64(b/262144UL);
   s[1UL] = b64(b/4096UL);
   if (n>=2UL) s[2UL] = b64(b/64UL);
   if (n==3UL) s[3UL] = b64(b);
} /* end enc64() */


static void sendjson(uint32_t jipnum0, uint32_t judpport0, const char text[], uint32_t text_len,
                char mod[], uint32_t mod_len, uint32_t dlen, uint32_t fec0, float baud, int32_t ppm,
                int32_t afc, char crc, float snr, float rfdb, float jmhz)
{
   char h[1000];
   char s[1000];
   int32_t ret;
   uint32_t b;
   uint32_t i0;
   X2C_PCOPY((void **)&mod,mod_len);
   strncpy(s,"{",1000u);
   aprsstr_Append(s, 1000ul, "\"mod\":\"", 8ul);
   aprsstr_Append(s, 1000ul, mod, mod_len);
   aprsstr_Append(s, 1000ul, "\",baud\":", 9ul);
   aprsstr_FixToStr(baud, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, "\",\"len\":", 9ul);
   aprsstr_CardToStr(dlen, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"crc\":", 8ul);
   aprsstr_CardToStr((uint32_t)crc, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"afc\":", 8ul);
   aprsstr_IntToStr(afc, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"snr\":", 8ul);
   aprsstr_FixToStr(snr, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"rfdb\":", 9ul);
   aprsstr_FixToStr(rfdb, 2UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"ppm\":", 8ul);
   aprsstr_IntToStr(ppm, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   aprsstr_Append(s, 1000ul, ",\"fecbits\":", 12ul);
   aprsstr_CardToStr(fec0, 1UL, h, 1000ul);
   aprsstr_Append(s, 1000ul, h, 1000ul);
   if (jmhz!=0.0f) {
      aprsstr_Append(s, 1000ul, ",\"rxmhz\":", 10ul);
      aprsstr_FixToStr(jmhz+0.0005f, 4UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, ",\"ver\":\"fsk4rx\"", 16ul);
   aprsstr_Append(s, 1000ul, ",\"payload\":\"", 13ul);
   b = 0UL;
   i0 = 0UL;
   while (i0<dlen) {
      /* base64 encode */
      b = b*256UL+(uint32_t)(uint8_t)text[i0];
      if (i0%3UL==2UL) {
         enc64(b, 3UL, h, 1000ul);
         aprsstr_Append(s, 1000ul, h, 1000ul);
         b = 0UL;
      }
      ++i0;
   }
   if (i0%3UL) {
      enc64(b, i0%3UL, h, 1000ul);
      aprsstr_Append(s, 1000ul, h, 1000ul);
   }
   aprsstr_Append(s, 1000ul, "\"}\012", 4ul);
   if (jpipename[0U]) {
      if (jsonfd<0L) {
         jsonfd = osi_OpenNONBLOCK(jpipename, 1024ul);
         if (jsonfd<0L) jsonfd = osi_OpenWrite(jpipename, 1024ul);
         else osic_Seekend(jsonfd, 0L);
      }
      if (jsonfd>=0L) osi_WrBin(jsonfd, (char *)s, 1000u/1u, aprsstr_Length(s, 1000ul));
      else osi_WrStrLn("cannot write json-file", 23ul);
   }
   if (judpport0) ret = udpsend(udpsock, s, (int32_t)aprsstr_Length(s, 1000ul), judpport0, jipnum0);
   X2C_PFREE(mod);
} /* end sendjson() */


static void appmhz(char s[], uint32_t s_len, float jmhz)
{
   char h[31];
   if (jmhz!=0.0f) {
      aprsstr_Append(s, s_len, " rx:", 5ul);
      aprsstr_FixToStr(jmhz+0.0005f, 4UL, h, 31ul);
      aprsstr_Append(s, s_len, h, 31ul);
      aprsstr_Append(s, s_len, "MHz", 4ul);
   }
} /* end appmhz() */


static void app(uint32_t * i0, uint32_t * p, char b[501], char c, int32_t v)
{
   char s[51];
   b[*p] = c;
   ++*p;
   aprsstr_IntToStr(v, 0UL, s, 51ul);
   *i0 = 0UL;
   while (s[*i0]) {
      b[*p] = s[*i0];
      ++*p;
      ++*i0;
   }
   b[*p] = ' ';
   ++*p;
} /* end app() */


static void sendaxudp2(char mon[], uint32_t mon_len, uint32_t dlen, float snrr, float rfdb, int32_t txd,
                 int32_t afc, int32_t qual, char longcall, char israw)
{
   char data[501];
   char b[501];
   int32_t lev;
   int32_t snr;
   int32_t datalen;
   int32_t ret;
   uint32_t i0;
   uint32_t p;
   pUDPDESTS udp0;
   X2C_PCOPY((void **)&mon,mon_len);
   snr = (int32_t)X2C_TRUNCI(snrr+0.5f,X2C_min_longint,X2C_max_longint);
   if (snr>127L) snr = 127L;
   else if (snr<-127L) snr = -127L;
   lev = (int32_t)X2C_TRUNCI(rfdb+0.5f,X2C_min_longint,X2C_max_longint);
   if (lev>127L) lev = 127L;
   else if (lev<-255L) lev = -255L;
   if (longcall) {
      datalen = (int32_t)aprsstr_Length(mon, mon_len);
      i0 = 0UL;
      do {
         data[i0] = mon[i0];
         ++i0;
      } while (!(i0>500UL || (int32_t)i0>=datalen));
   }
   else if (israw) {
      i0 = 0UL;
      datalen = (int32_t)dlen;
      while (i0<dlen && i0<500UL) {
         data[i0] = mon[i0];
         ++i0;
      }
   }
   else aprsstr_mon2raw(mon, mon_len, data, 501ul, &datalen);
   if (datalen>2L) {
      udp0 = udpdests;
      while (udp0) {
         p = 0UL;
         if (udp0->udp2) {
            if (!longcall) datalen -= 2L;
            b[0U] = '\001';
            b[1U] = '0';
            p = 2UL;
            app(&i0, &p, b, 'T', txd);
            app(&i0, &p, b, 'V', lev);
            app(&i0, &p, b, 'S', snr);
            app(&i0, &p, b, 'A', afc);
            app(&i0, &p, b, 'Q', qual);
            if (longcall) app(&i0, &p, b, 'X', 2L);
            b[p] = 0; /* end of axudp2 header */
            ++p;
         }
         i0 = 0UL;
         do {
            b[p] = data[i0];
            ++p;
            ++i0;
         } while ((int32_t)i0<datalen);
         if (udp0->udp2) {
            aprsstr_AppCRC(b, 501ul, (int32_t)p);
            p += 2UL;
         }
         ret = udpsend(udpsock, b, (int32_t)p, udp0->toport, udp0->ipnum);
         udp0 = udp0->next;
      }
   }
   else if (verb) osi_WrStrLn("beacon encode error, check callsign and ssid 0..15", 51ul);
   X2C_PFREE(mon);
} /* end sendaxudp2() */

#define fsk4rx_KNOTS 1.851984

#define fsk4rx_FEET 3.2808398950131

#define fsk4rx_WKNOTS 1.609
/* wx knots */


static char num0(uint32_t n)
{
   return (char)(n%10UL+48UL);
} /* end num() */


static uint32_t dao91(double x)
/* radix91(xx/1.1) of dddmm.mmxx */
{
   double a;
   a = fabs(x);
   return (((uint32_t)X2C_TRUNCC((a-(double)(uint32_t)X2C_TRUNCC(a,0UL,X2C_max_longcard))*6.E+5,0UL,
                X2C_max_longcard)%100UL)*20UL+11UL)/22UL;
} /* end dao91() */


static void encodeaprs(const char id[], uint32_t id_len, uint32_t td, const char mycall0[],
                uint32_t mycall_len, const char comment0[], uint32_t comment_len, const char sym[],
                uint32_t sym_len, const char destcall[], uint32_t destcall_len, const double lat,
                const double long0, int32_t txd, int32_t afc, int32_t qual, float snrr, float rfdb,
                float speed, float course, float alt, float clb, float gust, float temp, float hum,
                 float baro, char verbo)
{
   char h[501];
   char b[501];
   uint32_t n;
   uint32_t i0;
   double a;
   float v;
   char sig;
   b[0] = 0;
   aprsstr_Append(b, 501ul, mycall0, mycall_len);
   aprsstr_Append(b, 501ul, ">", 2ul);
   aprsstr_Append(b, 501ul, destcall, destcall_len);
   aprsstr_Append(b, 501ul, ":;", 3ul);
   aprsstr_Append(b, 501ul, id, id_len);
   aprsstr_Append(b, 501ul, "*", 2ul);
   aprsstr_DateToStr(td, h, 501ul);
   h[0U] = h[11U];
   h[1U] = h[12U];
   h[2U] = h[14U];
   h[3U] = h[15U];
   h[4U] = h[17U];
   h[5U] = h[18U];
   h[6U] = 0;
   aprsstr_Append(b, 501ul, h, 501ul);
   aprsstr_Append(b, 501ul, "h", 2ul);
   i0 = aprsstr_Length(b, 501ul);
   a = fabs(lat);
   n = osi_realcard((float)a);
   b[i0] = num0(n/10UL);
   ++i0;
   b[i0] = num0(n);
   ++i0;
   n = osi_realcard((float)((a-(double)n)*6000.0));
   b[i0] = num0(n/1000UL);
   ++i0;
   b[i0] = num0(n/100UL);
   ++i0;
   b[i0] = '.';
   ++i0;
   b[i0] = num0(n/10UL);
   ++i0;
   b[i0] = num0(n);
   ++i0;
   if (lat>=0.0) b[i0] = 'N';
   else b[i0] = 'S';
   ++i0;
   b[i0] = sym[0UL];
   ++i0;
   a = fabs(long0);
   n = osi_realcard((float)a);
   b[i0] = num0(n/100UL);
   ++i0;
   b[i0] = num0(n/10UL);
   ++i0;
   b[i0] = num0(n);
   ++i0;
   n = osi_realcard((float)((a-(double)n)*6000.0));
   b[i0] = num0(n/1000UL);
   ++i0;
   b[i0] = num0(n/100UL);
   ++i0;
   b[i0] = '.';
   ++i0;
   b[i0] = num0(n/10UL);
   ++i0;
   b[i0] = num0(n);
   ++i0;
   if (long0>=0.0) b[i0] = 'E';
   else b[i0] = 'W';
   ++i0;
   b[i0] = sym[1UL];
   ++i0;
   if (speed>0.0f || gust<1000.0f) {
      n = osi_realcard(course+1.5f);
      b[i0] = num0(n/100UL);
      ++i0;
      b[i0] = num0(n/10UL);
      ++i0;
      b[i0] = num0(n);
      ++i0;
      b[i0] = '/';
      ++i0;
      if (sym[0UL]=='/' && sym[1UL]=='_') n = osi_realcard(speed*6.2150403977626E-1f+0.5f);
      else n = osi_realcard(speed*5.3996146834962E-1f+0.5f);
      b[i0] = num0(n/100UL);
      ++i0;
      b[i0] = num0(n/10UL);
      ++i0;
      b[i0] = num0(n);
      ++i0;
      if (gust<1000.0f) {
         n = osi_realcard(gust*6.2150403977626E-1f+0.5f);
         b[i0] = 'g';
         ++i0;
         b[i0] = num0(n/100UL);
         ++i0;
         b[i0] = num0(n/10UL);
         ++i0;
         b[i0] = num0(n);
         ++i0;
      }
   }
   if ((float)fabs(temp)<1000.0f) {
      v = temp*1.8f+32.0f;
      sig = v<0.0f;
      n = osi_realcard((float)fabs(v)+0.5f);
      b[i0] = 't';
      ++i0;
      if (sig) b[i0] = '-';
      else b[i0] = num0(n/100UL);
      ++i0;
      b[i0] = num0(n/10UL);
      ++i0;
      b[i0] = num0(n);
      ++i0;
   }
   if (hum<=100.0f) {
      b[i0] = 'h';
      ++i0;
      n = osi_realcard(hum+0.5f);
      if (n==100UL) n = 0UL;
      b[i0] = num0(n/10UL);
      ++i0;
      b[i0] = num0(n);
      ++i0;
   }
   if (baro<10000.0f) {
      b[i0] = 'b';
      ++i0;
      n = osi_realcard(baro*10.0f+0.5f);
      b[i0] = num0(n/10000UL);
      ++i0;
      b[i0] = num0(n/1000UL);
      ++i0;
      b[i0] = num0(n/100UL);
      ++i0;
      b[i0] = num0(n/10UL);
      ++i0;
      b[i0] = num0(n);
      ++i0;
   }
   if (alt>0.5f) {
      b[i0] = '/';
      ++i0;
      b[i0] = 'A';
      ++i0;
      b[i0] = '=';
      ++i0;
      n = osi_realcard((float)fabs(alt*3.2808398950131f+0.5f));
      if (alt>=0.0f) b[i0] = num0(n/100000UL);
      else b[i0] = '-';
      ++i0;
      b[i0] = num0(n/10000UL);
      ++i0;
      b[i0] = num0(n/1000UL);
      ++i0;
      b[i0] = num0(n/100UL);
      ++i0;
      b[i0] = num0(n/10UL);
      ++i0;
      b[i0] = num0(n);
      ++i0;
   }
   b[i0] = '!'; /* DAO */
   ++i0;
   b[i0] = 'w';
   ++i0;
   b[i0] = (char)(33UL+dao91(lat));
   ++i0;
   b[i0] = (char)(33UL+dao91(long0));
   ++i0;
   b[i0] = '!';
   ++i0;
   b[i0] = 0;
   if ((float)fabs(clb)<1000.0f) {
      aprsstr_Append(b, 501ul, "Clb=", 5ul);
      aprsstr_FixToStr(clb, 2UL, h, 501ul);
      aprsstr_Append(b, 501ul, h, 501ul);
      aprsstr_Append(b, 501ul, "m/s ", 5ul);
   }
   aprsstr_Append(b, 501ul, comment0, comment_len);
   if (verbo) {
      osi_WrStr("TX:", 4ul);
      osi_WrStrLn(b, 501ul);
      osi_WrStrLn("", 1ul);
   }
   sendaxudp2(b, 501ul, aprsstr_Length(b, 501ul), snrr, rfdb, txd, afc, qual, 0, 0);
} /* end encodeaprs() */


static void lp(float in, struct LPCONTEXT24 * c)
/* lowpass 24db/oct 6dB loss */
{
   struct LPCONTEXT24 * anonym;
   { /* with */
      struct LPCONTEXT24 * anonym = c;
      anonym->uc1 = (anonym->uc1+(in-anonym->uc1)*anonym->LPR)-anonym->il;
      anonym->uc2 = anonym->uc2*anonym->OLPR+anonym->il;
      anonym->il = anonym->il+(anonym->uc1-anonym->uc2)*anonym->LPL;
   }
/* result is uc2 */
} /* end lp() */


static uint16_t crc16(const char f[], uint32_t f_len, uint32_t len)
{
   uint32_t i0;
   uint16_t x;
   uint16_t c;
   uint32_t tmp;
   c = 0xFFFFU;
   tmp = len-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      x = (uint16_t)(uint32_t)(uint8_t)f[i0]^X2C_LSH(c,16,-8);
      x = x^X2C_LSH(x,16,-4);
      c = X2C_LSH(c,16,8)^X2C_LSH(x,16,12)^X2C_LSH(x,16,5)^x;
      if (i0==tmp) break;
   } /* end for */
   return (uint16_t)c;
} /* end crc16() */


static void wavw(float u, int32_t soundfd, short afbuf[], uint32_t afbuf_len, uint32_t * afbufw)
{
   afbuf[*afbufw] = (short)X2C_TRUNCI(u*4.7746482928933E+3f,-32768,32767);
   ++*afbufw;
   if (*afbufw>afbuf_len-1) {
      osi_WrBin(soundfd, (char *)afbuf, (afbuf_len*2u)/1u, afbuf_len*2u);
      *afbufw = 0UL;
   }
} /* end wavw() */


static void fmoutc(const struct Complex u, int32_t soundfd, float * oldaf, short afbuf[], uint32_t afbuf_len,
                 uint32_t * afbufw)
{
   wavw(fmhighpass(atan20(u), oldaf), soundfd, afbuf, afbuf_len, afbufw);
   ++*afbufw;
   if (*afbufw>afbuf_len-1) {
      osi_WrBin(soundfd, (char *)afbuf, (afbuf_len*2u)/1u, afbuf_len*2u);
      *afbufw = 0UL;
   }
} /* end fmoutc() */


static void usbout(const struct Complex u, int32_t soundfd, uint16_t * usbosc, float * usbgain,
                uint32_t * afbufw, short afbuf[], uint32_t afbuf_len)
{
   float af;
   /*ssb demodulator */
   af = u.Re*DDSS[*usbosc&4095U]-u.Im*DDSS[*usbosc+1024U&4095U];
   /*agc */
   af = af* *usbgain;
   if ((float)fabs(af)>10000.0f) *usbgain =  *usbgain*0.95f;
   else {
      *usbgain =  *usbgain*1.0002f;
      if (*usbgain<0.00001f) *usbgain = 0.00001f;
   }
   /*peak limiter */
   if (af>30000.0f) af = 30000.0f;
   else if (af<(-3.E+4f)) af = (-3.E+4f);
   afbuf[*afbufw] = (short)(int32_t)X2C_TRUNCI(af,X2C_min_longint,X2C_max_longint);
   /*bfo */
   *usbosc += (uint16_t)bfofreq;
   /*audio out */
   ++*afbufw;
   if (*afbufw>afbuf_len-1) {
      osi_WrBin(soundfd, (char *)afbuf, (afbuf_len*2u)/1u, afbuf_len*2u);
      *afbufw = 0UL;
   }
} /* end usbout() */

/*--------------- (a)fsk modem */

static char aischar(uint32_t c)
{
   if (c<32UL) return (char)(c+64UL);
   else return (char)c;
   return 0;
} /* end aischar() */


static void clean(char s[], uint32_t s_len)
{
   uint32_t i0;
   uint32_t tmp;
   tmp = s_len-1;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      if (s[i0]=='@') s[i0] = 0;
      if (i0==tmp) break;
   } /* end for */
   i0 = s_len-1; /* trailing blanks */
   for (;;) {
      if (s[i0]==' ') s[i0] = 0;
      else if (s[i0]) break;
      if (i0==0UL) break;
      --i0;
   }
} /* end clean() */


static void wrdimens(uint32_t n, char s[], uint32_t s_len)
{
   char h[31];
   aprsstr_CardToStr((n/4096UL&511UL)+(n/2097152UL&511UL), 1UL, s, s_len);
   aprsstr_Append(s, s_len, "x", 2ul);
   aprsstr_CardToStr((n/64UL&63UL)+(n&63UL), 1UL, h, 31ul);
   aprsstr_Append(s, s_len, h, 31ul);
   aprsstr_Append(s, s_len, "m", 2ul);
} /* end wrdimens() */

#define fsk4rx_FIX "undefined  GPS        GLONASS    GPS+GLONASSLORAN-C    CHAYKA     Integrated survayed   Galileo    i\
ntern GNSS"

#define fsk4rx_TXRX "TxB, RxA/RxBTxA, RxA/RxBTxB, RxA/RxB            "


static uint32_t bits(char b[920], uint32_t x, uint32_t y)
{
   uint32_t n;
   n = 0UL;
   while (x<=y) {
      n += n+(uint32_t)b[x];
      ++x;
   }
   return n;
} /* end bits() */


static void latlong(char b[920], uint32_t bp, float * lat, float * long0)
{
   uint32_t ii;
   ii = bits(b, bp, bp+27UL);
   if (ii>=134217728UL) ii += 0x0F0000000UL;
   *long0 = (float)(int32_t)ii*1.6666666666667E-6f;
   ii = bits(b, bp+28UL, bp+54UL);
   if (ii>=67108864UL) ii += 0x0F8000000UL;
   *lat = (float)(int32_t)ii*1.6666666666667E-6f;
} /* end latlong() */


static void getcountrycodes(void)
{
   int32_t fp;
   char c;
   uint32_t n;
   char num1;
   if (ccfilename[0U]==0) return;
   fp = osi_OpenRead(ccfilename, 1024ul);
   if (fp>=0L) {
      osic_alloc((char * *) &countrycodes, 32000UL);
      if (countrycodes==0) return;
      memset((char *)countrycodes,(char)0,32000UL);
      n = 0UL;
      num1 = 1;
      while (osi_RdBin(fp, (char *) &c, 1u/1u, 1UL)==1L) {
         if (num1) {
            if ((uint8_t)c>='0' && (uint8_t)c<='9') {
               n = (n*10UL+(uint32_t)(uint8_t)c)-48UL;
            }
            else num1 = 0;
         }
         else if (n<=999UL && (uint8_t)c>=' ') {
            aprsstr_Append(countrycodes[n], 32ul, (char *) &c, 1u/1u);
         }
         else {
            num1 = 1;
            n = 0UL;
         }
      }
   }
} /* end getcountrycodes() */

#define fsk4rx_MINSPD 2


static uint32_t shiplim(uint32_t id, uint32_t spd)
{
   pSHIPBEACONS p;
   uint32_t tt;
   uint32_t t;
   if (shipdrivebeacontime==0UL && shipnodrivebeacontime==0UL) return 0UL;
   t = osic_time();
   p = shipbeacons;
   while (p) {
      if (p->mmsi==id) {
         if (p->time0>t) p->time0 = t;
         if (spd>=2UL || p->sog>=2UL) tt = shipdrivebeacontime;
         else tt = shipnodrivebeacontime;
         if (p->time0+tt<=t) {
            p->time0 = t;
            p->sog = spd;
            return 0UL;
         }
         return (p->time0+tt)-t;
      }
      p = p->next;
   }
   p = shipbeacons; /* not found */
   for (;;) {
      if (p==0) {
         osic_alloc((char * *) &p, sizeof(struct SHIPBEACONS));
         if (p==0) return 1UL;
         /* out of memory */
         p->next = shipbeacons;
         shipbeacons = p;
         break;
      }
      if (p->time0+shipnodrivebeacontime<t) break;
      p = p->next;
   }
   p->time0 = t;
   p->mmsi = id;
   p->sog = spd;
   return 0UL;
} /* end shiplim() */


static void AIS(char f[], uint32_t f_len, uint32_t len, char port, int32_t afc, float rfdb,
                float jmhz)
{
   uint32_t nextbeacon;
   uint32_t textnum;
   uint32_t fi;
   uint32_t dac;
   uint32_t bitc;
   uint32_t viac;
   uint32_t navstat;
   uint32_t course;
   uint32_t sog;
   uint32_t uid;
   uint32_t mn;
   uint32_t ih;
   uint32_t i0;
   int32_t ii;
   char newship;
   float lat;
   float long0;
   char b[920];
   char cc[201];
   char utc[201];
   char h[201];
   char mmsitext[201];
   char ns[201];
   char ls[201];
   char symb[2];
   char comm[251];
   pSHIP ps1;
   pSHIP ps;
   /* new or update */
   struct SHIP * anonym;
   struct SHIP * anonym0;
   char tmp;
   uint32_t tmp0;
   X2C_PCOPY((void **)&f,f_len);
   /*FOR i:=0 TO 20 DO f[i]:=CHR(test[i]) END; */
   /*len:=21; */
   if (!ccdone && countrycodes==0) getcountrycodes();
   ccdone = 1;
   bitc = 0UL;
   do {
      b[bitc] = X2C_IN(7UL-(bitc&7UL),8,(uint8_t)(uint8_t)f[bitc/8UL]);
      ++bitc;
   } while (!(bitc>919UL || bitc>len*8UL));
   mn = bits(b, 0UL, 5UL);
   viac = bits(b, 6UL, 7UL);
   uid = bits(b, 8UL, 37UL);
   mmsitext[9U] = 0;
   i0 = uid;
   for (ii = 8L; ii>=0L; ii--) {
      mmsitext[ii] = (char)(i0%10UL+48UL);
      i0 = i0/10UL;
   } /* end for */
   ps = ships;
   while (ps && ps->mmsi!=uid) ps = ps->next;
   ns[0] = 0;
   utc[0] = 0;
   cc[0] = 0;
   if (countrycodes) {
      i0 = 1000000UL;
      if (uid<10000000UL) i0 = 10000UL;
      aprsstr_Assign(cc, 201ul, countrycodes[(uid/i0)%1000UL], 32ul);
   }
   lat = 0.0f;
   long0 = 0.0f;
   sog = 0UL;
   course = 0UL;
   navstat = 0UL;
   newship = 0;
   strncpy(symb,"/s",2u);
   nextbeacon = 0UL;
   if (mn==5UL || mn==24UL) {
      /* ship data */
      if ((len==53UL || len==20UL) || len==21UL) {
         if (ps==0) {
            /* new ship */
            newship = 1;
            ps1 = ships; /* check memory usage */
            if (ps1 && ps1->next) {
               i0 = 2UL;
               while (ps1->next->next) {
                  ++i0;
                  ps1 = ps1->next;
               }
               if (i0>10000UL) {
                  /* recycle oldest entry */
                  ps = ps1->next; /* end of chain */
                  ps1->next = 0;
                  if (verb) osi_WrStrLn("remove oldest ship from database", 33ul);
               }
               else ps = 0;
            }
            if (ps==0) osic_alloc((char * *) &ps, sizeof(struct SHIP));
            if (ps) {
               memset((char *)ps,(char)0,sizeof(struct SHIP));
               ps->next = ships;
               ships = ps;
            }
         }
         if (ps) {
            { /* with */
               struct SHIP * anonym = ps;
               anonym->mmsi = uid;
               if (len==20UL && bits(b, 38UL, 39UL)==0UL) {
                  /* msg 24 part 1 */
                  for (i0 = 0UL; i0<=19UL; i0++) {
                     anonym->name[i0] = aischar(bits(b, i0*6UL+40UL, i0*6UL+45UL));
                  } /* end for */
               }
               else if (len==21UL && bits(b, 38UL, 39UL)==1UL) {
                  /* msg 24 part 1 */
                  for (i0 = 0UL; i0<=6UL; i0++) {
                     anonym->vendorid[i0] = aischar(bits(b, i0*6UL+48UL, i0*6UL+53UL));
                  } /* end for */
                  for (i0 = 0UL; i0<=6UL; i0++) {
                     anonym->call[i0] = aischar(bits(b, i0*6UL+90UL, i0*6UL+95UL));
                  } /* end for */
                  anonym->dimens = bits(b, 132UL, 161UL);
               }
               else if (len==53UL) {
                  /* msg 5 */
                  for (i0 = 0UL; i0<=6UL; i0++) {
                     anonym->call[i0] = aischar(bits(b, i0*6UL+70UL, i0*6UL+75UL));
                  } /* end for */
                  for (i0 = 0UL; i0<=19UL; i0++) {
                     anonym->name[i0] = aischar(bits(b, i0*6UL+112UL, i0*6UL+117UL));
                  } /* end for */
                  for (i0 = 0UL; i0<=19UL; i0++) {
                     anonym->dest[i0] = aischar(bits(b, i0*6UL+302UL, i0*6UL+307UL));
                  } /* end for */
                  anonym->typ = (uint8_t)bits(b, 232UL, 239UL);
                  anonym->dimens = bits(b, 240UL, 269UL);
                  anonym->draught = (float)bits(b, 294UL, 301UL)*0.1f;
                  i0 = bits(b, 274UL, 293UL);
                  anonym->eta[0U] = (char)((i0/65536UL)/10UL+48UL);
                  anonym->eta[1U] = (char)((i0/65536UL)%10UL+48UL);
                  anonym->eta[2U] = (char)((i0/2048UL&31UL)/10UL+48UL);
                  anonym->eta[3U] = (char)((i0/2048UL&31UL)%10UL+48UL);
                  anonym->eta[4U] = (char)((i0/64UL&31UL)/10UL+48UL);
                  anonym->eta[5U] = (char)((i0/64UL&31UL)%10UL+48UL);
                  anonym->eta[6U] = (char)((i0&63UL)/10UL+48UL);
                  anonym->eta[7U] = (char)((i0&63UL)%10UL+48UL);
               }
               clean(anonym->call, 7ul);
               clean(anonym->name, 20ul);
               clean(anonym->dest, 20ul);
               clean(anonym->vendorid, 7ul);
               /*-shiplog */
               if ((len==53UL && newship) && aisfd>=0L) {
                  aprsstr_Assign(ls, 201ul, anonym->name, 20ul);
                  aprsstr_Append(ls, 201ul, ",", 2ul);
                  aprsstr_Append(ls, 201ul, anonym->call, 7ul);
                  aprsstr_Append(ls, 201ul, ",", 2ul);
                  aprsstr_Append(ls, 201ul, anonym->vendorid, 7ul);
                  aprsstr_Append(ls, 201ul, ",", 2ul);
                  aprsstr_Append(ls, 201ul, mmsitext, 201ul);
                  aprsstr_Append(ls, 201ul, ",", 2ul);
                  aprsstr_CardToStr((uint32_t)anonym->typ, 1UL, h, 201ul);
                  aprsstr_Append(ls, 201ul, h, 201ul);
                  aprsstr_Append(ls, 201ul, ",", 2ul);
                  wrdimens(anonym->dimens, h, 201ul);
                  aprsstr_Append(ls, 201ul, h, 201ul);
                  aprsstr_Append(ls, 201ul, ",", 2ul);
                  aprsstr_FixToStr(anonym->draught, 2UL, h, 201ul);
                  aprsstr_Append(ls, 201ul, h, 201ul);
                  aprsstr_Append(ls, 201ul, "m\012", 3ul);
                  osi_WrBin(aisfd, (char *)ls, 201u/1u, aprsstr_Length(ls, 201ul));
               }
            }
         }
      }
   }
   else if ((mn==1UL || mn==2UL) || mn==3UL) {
      /*-shiplog */
      if (len==21UL) {
         navstat = bits(b, 38UL, 41UL);
         /*    rot:=bits(42,49); */
         sog = bits(b, 50UL, 59UL);
         course = bits(b, 116UL, 127UL);
         latlong(b, 61UL, &lat, &long0);
         switch (navstat&15UL) {
         case 0UL:
            strncpy(ns,"under way using engine",201u);
            break;
         case 1UL:
            strncpy(ns,"at anchor",201u);
            break;
         case 2UL:
            strncpy(ns,"not under command",201u);
            break;
         case 3UL:
            strncpy(ns,"restricted maneuverability",201u);
            break;
         case 4UL:
            strncpy(ns,"constrained by her draught",201u);
            break;
         case 5UL:
            strncpy(ns,"moored",201u);
            break;
         case 6UL:
            strncpy(ns,"aground",201u);
            break;
         case 7UL:
            strncpy(ns,"engaged in fishing",201u);
            break;
         case 8UL:
            strncpy(ns,"under way sailing",201u);
            break;
         case 9UL:
            strncpy(ns,"reserved for future",201u);
            break;
         case 10UL:
            strncpy(ns,"dangerous goods",201u);
            break;
         case 11UL:
            strncpy(ns,"powerdriven vessel towing astern",201u);
            break;
         case 12UL:
            strncpy(ns,"power-driven vessel pushing ahead",201u);
            break;
         case 13UL:
            strncpy(ns,"reserved for future use",201u);
            break;
         case 14UL:
            strncpy(ns,"AIS-SART, MOB-AIS, EPIRB-AIS",201u);
            break;
         case 15UL:
            strncpy(ns,"under test",201u);
            break;
         default:
            X2C_TRAP(X2C_CASE_TRAP);
         } /* end switch */
      }
   }
   else if (mn==18UL) {
      /* class b */
      if (len==21UL) {
         sog = bits(b, 46UL, 55UL);
         latlong(b, 57UL, &lat, &long0);
         course = bits(b, 112UL, 123UL);
      }
   }
   else if (mn==4UL || mn==11UL) {
      if (len==21UL) {
         if (mn==4UL) strncpy(symb,"/r",2u);
         latlong(b, 79UL, &lat, &long0);
         aprsstr_CardToStr(bits(b, 38UL, 51UL), 1UL, utc, 201ul);
         aprsstr_Append(utc, 201ul, "-", 2ul);
         aprsstr_CardToStr(bits(b, 52UL, 55UL), 1UL, h, 201ul);
         aprsstr_Append(utc, 201ul, h, 201ul);
         aprsstr_Append(utc, 201ul, "-", 2ul);
         aprsstr_CardToStr(bits(b, 56UL, 60UL), 1UL, h, 201ul);
         aprsstr_Append(utc, 201ul, h, 201ul);
         aprsstr_Append(utc, 201ul, " ", 2ul);
         aprsstr_CardToStr(bits(b, 61UL, 65UL), 1UL, h, 201ul);
         aprsstr_Append(utc, 201ul, h, 201ul);
         aprsstr_Append(utc, 201ul, ":", 2ul);
         aprsstr_CardToStr(bits(b, 66UL, 71UL), 1UL, h, 201ul);
         aprsstr_Append(utc, 201ul, h, 201ul);
         aprsstr_Append(utc, 201ul, ":", 2ul);
         aprsstr_CardToStr(bits(b, 72UL, 77UL), 1UL, h, 201ul);
         aprsstr_Append(utc, 201ul, h, 201ul);
      }
   }
   nextbeacon = shiplim(uid, sog);
   if (verb) {
      osi_WrStr("AIS", 4ul);
      osi_WrStr((char *) &port, 1u/1u);
      osi_WrStr(":len:", 6ul);
      osic_WrUINT32(len, 1UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(rfdb, 1L, 1UL);
      osi_WrStr("dB msg:", 8ul);
      osic_WrUINT32(mn, 1UL);
      osi_WrStr(" MMSI:", 7ul);
      osi_WrStr(mmsitext, 201ul);
      if (cc[0U]) {
         osi_WrStr("(", 2ul);
         osi_WrStr(cc, 201ul);
         osi_WrStr(")", 2ul);
      }
      osi_WrStr(" rep:", 6ul);
      osic_WrUINT32(viac, 1UL);
      if (((mn==1UL || mn==2UL) || mn==3UL) || mn==18UL) {
         osi_WrStr(" lat:", 6ul);
         osic_WrFixed(lat, 6L, 1UL);
         osi_WrStr(" long:", 7ul);
         osic_WrFixed(long0, 6L, 1UL);
         osi_WrStr(" dir:", 6ul);
         osic_WrFixed((float)course*0.1f, 1L, 1UL);
         osi_WrStr(" knots:", 8ul);
         osic_WrFixed((float)sog*0.1f, 1L, 1UL);
         if (ns[0]) {
            osi_WrStr(" stat:", 7ul);
            osi_WrStr(ns, 201ul);
         }
      }
      else if (mn==6UL) {
         osi_WrStr(" ToID:", 7ul);
         WrHexCap(bits(b, 40UL, 69UL), 4UL, 0UL);
         osi_WrStr(" Text:", 7ul);
         i0 = 100UL;
         do {
            osi_WrStr((char *)(tmp = aischar(bits(b, i0, i0+5UL)),&tmp), 1u/1u);
            i0 += 6UL;
         } while (!(i0>1006UL || i0>bitc));
      }
      else if (mn==4UL || mn==11UL) {
         if (mn==4UL) osi_WrStr(" BaseStationUTC:", 17ul);
         else if (mn==11UL) osi_WrStr(" MobilStationUTC:", 18ul);
         osi_WrStr(utc, 201ul);
         osi_WrStr(" lat:", 6ul);
         osic_WrFixed(lat, 6L, 1UL);
         osi_WrStr(" long:", 7ul);
         osic_WrFixed(long0, 6L, 1UL);
      }
      else if (mn==5UL) {
         osi_WrStr(" IMO:", 6ul);
         WrHexCap(bits(b, 40UL, 69UL), 8UL, 0UL);
      }
      else if (mn==8UL) {
         dac = bits(b, 40UL, 49UL);
         fi = bits(b, 50UL, 55UL);
         osi_WrStr(" dac:", 6ul);
         osic_WrUINT32(dac, 1UL);
         osi_WrStr(" fi:", 5ul);
         osic_WrUINT32(fi, 1UL);
         if (bits(b, 56UL, 56UL)>0UL) osi_WrStr(" ack", 5ul);
         textnum = bits(b, 57UL, 67UL);
         osi_WrStr(" textnum:", 10ul);
         osic_WrUINT32(textnum, 1UL);
         /*    IF (dac=1) & (fi=0) THEN (* text message *) */
         osi_WrStr(" text:", 7ul);
         i0 = 68UL;
         do {
            osi_WrStr((char *)(tmp = aischar(bits(b, i0, i0+5UL)),&tmp), 1u/1u);
            i0 += 6UL;
         } while (!(i0>1004UL || i0>bitc));
      }
      else if (mn==20UL) {
         /*    ELSE WrStr(" Binary") END; */
         /* data link management */
         i0 = 0UL;
         do {
            ih = i0*30UL;
            osi_WrStr(" Offset", 8ul);
            osic_WrUINT32(i0+1UL, 1UL);
            osi_WrStr(":", 2ul);
            osic_WrUINT32(bits(b, 40UL+ih, 51UL+ih), 1UL);
            osi_WrStr(" Slots", 7ul);
            osic_WrUINT32(i0+1UL, 1UL);
            osi_WrStr(":", 2ul);
            osic_WrUINT32(bits(b, 52UL+ih, 55UL+ih), 1UL);
            osi_WrStr(" Timeout", 9ul);
            osic_WrUINT32(i0+1UL, 1UL);
            osi_WrStr(":", 2ul);
            osic_WrUINT32(bits(b, 56UL+ih, 58UL+ih), 1UL);
            osi_WrStr("min Incr", 9ul);
            osic_WrUINT32(i0+1UL, 1UL);
            osi_WrStr(":", 2ul);
            osic_WrUINT32(bits(b, 59UL+ih, 69UL+ih), 1UL);
            ++i0;
         } while (len*8UL>=40UL+i0*30UL);
      }
      else if (mn==23UL) {
         if (len==20UL) {
            latlong(b, 40UL, &lat, &long0);
            osi_WrStr(" lat1:", 7ul);
            osic_WrFixed(lat, 6L, 1UL);
            osi_WrStr(" long1:", 8ul);
            osic_WrFixed(long0, 6L, 1UL);
            latlong(b, 75UL, &lat, &long0);
            osi_WrStr(" lat2:", 7ul);
            osic_WrFixed(lat, 6L, 1UL);
            osi_WrStr(" long2:", 8ul);
            osic_WrFixed(long0, 6L, 1UL);
            osi_WrStr(" Typ:", 6ul);
            osic_WrUINT32(bits(b, 110UL, 113UL), 1UL);
            osi_WrStr(" Mode:", 7ul);
            ih = bits(b, 144UL, 145UL)*12UL;
            tmp0 = ih+11UL;
            i0 = ih;
            if (i0<=tmp0) for (;; i0++) {
               osi_WrStr((char *) &"TxB, RxA/RxBTxA, RxA/RxBTxB, RxA/RxB            "[i0], 1u/1u);
               if (i0==tmp0) break;
            } /* end for */
            osi_WrStr(" Quiet:", 8ul);
            osic_WrUINT32(bits(b, 150UL, 153UL), 1UL);
         }
      }
      else if (mn==24UL) {
         osi_WrStr(" Part:", 7ul);
         osic_WrINT32(bits(b, 38UL, 39UL), 1UL);
      }
      if (ps) {
         { /* with */
            struct SHIP * anonym0 = ps;
            osi_WrStr(" Call:", 7ul);
            osi_WrStr(anonym0->call, 7ul);
            osi_WrStr(" Name:", 7ul);
            osi_WrStr(anonym0->name, 20ul);
            osi_WrStr(" Vendor:", 9ul);
            osi_WrStr(anonym0->vendorid, 7ul);
            osi_WrStr(" Type:", 7ul);
            osic_WrUINT32((uint32_t)anonym0->typ, 1UL);
            osi_WrStr(" Arrive:", 9ul);
            osi_WrStr(anonym0->eta, 8ul);
            osi_WrStr(" Draught:", 10ul);
            osic_WrFixed(anonym0->draught, 1L, 1UL);
            osi_WrStr(" Dim:", 6ul);
            wrdimens(anonym0->dimens, h, 201ul);
            osi_WrStr(h, 201ul);
            osi_WrStr(" Dest:", 7ul);
            osi_WrStr(anonym0->dest, 20ul);
         }
      }
      if (nextbeacon) {
         osi_WrStr(" (", 3ul);
         osic_WrUINT32(nextbeacon, 1UL);
         osi_WrStr(")", 2ul);
      }
      osi_WrStrLn("", 1ul);
   }
   if ((((len==21UL && (mn>=1UL && mn<=4UL || mn==18UL)) && (float)fabs(lat)<=90.0f) && (float)fabs(long0)
                <=180.0f) && nextbeacon==0UL) {
      strncpy(comm,"rep:",251u);
      aprsstr_CardToStr(viac, 1UL, h, 201ul);
      aprsstr_Append(comm, 251ul, h, 201ul);
      appmhz(comm, 251ul, jmhz);
      aprsstr_Append(comm, 251ul, " ", 2ul);
      aprsstr_FixToStr(rfdb, 2UL, h, 201ul);
      aprsstr_Append(comm, 251ul, h, 201ul);
      aprsstr_Append(comm, 251ul, "dB msg:", 8ul);
      aprsstr_CardToStr(mn, 1UL, h, 201ul);
      aprsstr_Append(comm, 251ul, h, 201ul);
      if (cc[0]) {
         aprsstr_Append(comm, 251ul, " [", 3ul);
         aprsstr_Append(comm, 251ul, cc, 201ul);
         aprsstr_Append(comm, 251ul, "]", 2ul);
      }
      if (utc[0]) {
         aprsstr_Append(comm, 251ul, " ", 2ul);
         aprsstr_Append(comm, 251ul, utc, 201ul);
      }
      if (ps) {
         aprsstr_CardToStr((uint32_t)ps->typ, 1UL, h, 201ul);
         aprsstr_Append(comm, 251ul, " Type:", 7ul);
         aprsstr_Append(comm, 251ul, h, 201ul);
         aprsstr_FixToStr(ps->draught, 2UL, h, 201ul);
         aprsstr_Append(comm, 251ul, ", draught:", 11ul);
         aprsstr_Append(comm, 251ul, h, 201ul);
         aprsstr_Append(comm, 251ul, " Dim:", 6ul);
         wrdimens(ps->dimens, h, 201ul);
         aprsstr_Append(comm, 251ul, h, 201ul);
         aprsstr_Append(comm, 251ul, " ETA:", 6ul);
         aprsstr_Append(comm, 251ul, ps->eta, 8ul);
         aprsstr_Append(comm, 251ul, " Call[", 7ul);
         aprsstr_Append(comm, 251ul, ps->call, 7ul);
         aprsstr_Append(comm, 251ul, "] Name[", 8ul);
         aprsstr_Append(comm, 251ul, ps->name, 20ul);
         aprsstr_Append(comm, 251ul, "] Dest[", 8ul);
         aprsstr_Append(comm, 251ul, ps->dest, 20ul);
         aprsstr_Append(comm, 251ul, "]", 2ul);
         if (ps->vendorid[0U]) {
            aprsstr_Append(comm, 251ul, " Vendor[", 9ul);
            aprsstr_Append(comm, 251ul, ps->vendorid, 7ul);
            aprsstr_Append(comm, 251ul, "]", 2ul);
         }
      }
      if (ns[0]) {
         aprsstr_Append(comm, 251ul, " [", 3ul);
         aprsstr_Append(comm, 251ul, ns, 201ul);
         aprsstr_Append(comm, 251ul, "]", 2ul);
      }
      encodeaprs(mmsitext, 201ul, osic_time()%86400UL, mycall, 10ul, comm, 251ul, symb, 2ul, "APLAIS", 7ul,
                (double)lat, (double)long0, 0L, afc, 0L, 0.0f, rfdb, (float)sog*1.851984E-1f,
                (float)course*0.1f, (-3.E+4f), 1.E+5f, 1.E+5f, 1.E+5f, 1.E+5f, 1.E+5f, 0);
   }
   X2C_PFREE(f);
/*name call mmsi typ*/
} /* end AIS() */


static void WCh(char c)
{
   if (c!='\015') {
      if ((uint8_t)c<' ' || (uint8_t)c>='\177') osi_WrStr(".", 2ul);
      else osi_WrStr((char *) &c, 1u/1u);
   }
} /* end WCh() */


static void ShowCall(char f[], uint32_t f_len, uint32_t pos)
{
   uint32_t e;
   uint32_t i0;
   uint32_t tmp;
   char tmp0;
   e = pos;
   tmp = pos+5UL;
   i0 = pos;
   if (i0<=tmp) for (;; i0++) {
      if (f[i0]!='@') e = i0;
      if (i0==tmp) break;
   } /* end for */
   tmp = e;
   i0 = pos;
   if (i0<=tmp) for (;; i0++) {
      WCh((char)((uint32_t)(uint8_t)f[i0]>>1));
      if (i0==tmp) break;
   } /* end for */
   i0 = (uint32_t)(uint8_t)f[pos+6UL]>>1&15UL;
   if (i0) {
      osi_WrStr("-", 2ul);
      if (i0>=10UL) osi_WrStr((char *)(tmp0 = (char)(i0/10UL+48UL),&tmp0), 1u/1u);
      osi_WrStr((char *)(tmp0 = (char)(i0%10UL+48UL),&tmp0), 1u/1u);
   }
} /* end ShowCall() */

static uint32_t fsk4rx_UA = 0x63UL;

static uint32_t fsk4rx_DM = 0xFUL;

static uint32_t fsk4rx_SABM = 0x2FUL;

static uint32_t fsk4rx_DISC = 0x43UL;

static uint32_t fsk4rx_FRMR = 0x87UL;

static uint32_t fsk4rx_UI = 0x3UL;

static uint32_t fsk4rx_RR = 0x1UL;

static uint32_t fsk4rx_REJ = 0x9UL;

static uint32_t fsk4rx_RNR = 0x5UL;


static void Showctl(uint32_t com, uint32_t cmd)
{
   uint32_t cm;
   char PF[4];
   char tmp;
   osi_WrStr(" ctl ", 6ul);
   cm = (uint32_t)cmd&~0x10UL;
   if ((cm&0xFUL)==0x1UL) {
      osi_WrStr("RR", 3ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0xFUL)==0x5UL) {
      osi_WrStr("RNR", 4ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0xFUL)==0x9UL) {
      osi_WrStr("REJ", 4ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
   }
   else if ((cm&0x1UL)==0UL) {
      osi_WrStr("I", 2ul);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>5)),&tmp), 1u/1u);
      osi_WrStr((char *)(tmp = (char)(48UL+(cmd>>1&7UL)),&tmp), 1u/1u);
   }
   else if (cm==0x3UL) osi_WrStr("UI", 3ul);
   else if (cm==0xFUL) osi_WrStr("DM", 3ul);
   else if (cm==0x2FUL) osi_WrStr("SABM", 5ul);
   else if (cm==0x43UL) osi_WrStr("DISC", 5ul);
   else if (cm==0x63UL) osi_WrStr("UA", 3ul);
   else if (cm==0x87UL) osi_WrStr("FRMR", 5ul);
   else WrHexCap(cmd, 2UL, 0UL);
   strncpy(PF,"v^-+",4u);
   if (com==0UL || com==3UL) osi_WrStr("v1", 3ul);
   else osi_WrStr((char *) &PF[(com&1UL)+2UL*(uint32_t)((0x10UL & (uint32_t)cmd)!=0)], 1u/1u);
} /* end Showctl() */


static void ShowFrame(char f[], uint32_t f_len, uint32_t len, char port, float level, float snrr,
                int32_t txd, int32_t afc, int32_t qual)
{
   uint32_t i0;
   char d;
   char v;
   X2C_PCOPY((void **)&f,f_len);
   osi_WrStr((char *) &port, 1u/1u);
   i0 = 0UL;
   while (!((uint32_t)(uint8_t)f[i0]&1)) {
      ++i0;
      if (i0>len) {
         osi_WrStrLn(" no ax.25 (no address end mark)", 32ul);
         goto label;
      }
   }
   /* no address end mark found */
   if (i0%7UL!=6UL) {
      osi_WrStrLn(" no ax.25 (address field size not multiples of 7)", 50ul);
      goto label;
   }
   /* address end not modulo 7 error */
   osi_WrStr(":fm ", 5ul);
   ShowCall(f, f_len, 7UL);
   osi_WrStr(" to ", 5ul);
   ShowCall(f, f_len, 0UL);
   i0 = 14UL;
   v = 1;
   while (i0+6UL<len && !((uint32_t)(uint8_t)f[i0-1UL]&1)) {
      if (v) {
         osi_WrStr(" via", 5ul);
         v = 0;
      }
      osi_WrStr(" ", 2ul);
      ShowCall(f, f_len, i0);
      if ((uint32_t)(uint8_t)f[i0+6UL]>=128UL && (((uint32_t)(uint8_t)f[i0+6UL]&1) || (uint32_t)(uint8_t)
                f[i0+13UL]<128UL)) osi_WrStr("*", 2ul);
      i0 += 7UL;
   }
   Showctl((uint32_t)((0x80U & (uint8_t)(uint8_t)f[6UL])!=0)+2UL*(uint32_t)((0x80U & (uint8_t)(uint8_t)
                f[13UL])!=0), (uint32_t)(uint8_t)f[i0]);
   ++i0;
   if (i0<len) {
      osi_WrStr(" pid ", 6ul);
      WrHexCap((uint32_t)(uint8_t)f[i0], 2UL, 0UL);
   }
   ++i0;
   if (level!=0.0f) {
      osi_WrStr(" lev:", 6ul);
      osic_WrFixed(level, 1L, 1UL);
      osi_WrStr("dB", 3ul);
   }
   if (snrr!=0.0f) {
      osi_WrStr(" snr:", 6ul);
      osic_WrFixed(snrr, 1L, 1UL);
      osi_WrStr("dB", 3ul);
   }
   if (txd) {
      osi_WrStr(" txd:", 6ul);
      osic_WrINT32((uint32_t)txd, 1UL);
      osi_WrStr("ms", 3ul);
   }
   if (afc) {
      osi_WrStr(" afc:", 6ul);
      osic_WrINT32((uint32_t)afc, 1UL);
      osi_WrStr("Hz", 3ul);
   }
   if (qual) {
      osi_WrStr(" eye:", 6ul);
      osic_WrINT32((uint32_t)qual, 1UL);
      osi_WrStr("%", 2ul);
   }
   osi_WrStrLn("", 1ul);
   /*  IF NOT noinfo THEN */
   d = 0;
   while (i0<len) {
      if (f[i0]!='\015') {
         WCh(f[i0]);
         d = 1;
      }
      else if (d) {
         osi_WrStrLn("", 1ul);
         d = 0;
      }
      ++i0;
   }
   if (d) osi_WrStrLn("", 1ul);
   label:;
   X2C_PFREE(f);
/*  END; */
} /* end ShowFrame() */


static void afskframe(struct AFSKMODEM * m)
{
   uint32_t dlen;
   uint32_t i0;
   char cl;
   char ch;
   float rfdb;
   float snrr;
   float level;
   int32_t qual;
   int32_t afc;
   int32_t txd;
   char crcok;
   char ms[100];
   if (m->bitcnt>140UL && (m->bitcnt&7UL)==6UL) {
      /* min len and modulo 8 bits */
      dlen = (m->bitcnt-6UL)/8UL;
      i0 = 0UL;
      while (i0<dlen && !((uint32_t)(uint8_t)m->frame[i0]&1)) ++i0;
      if (m->rawbits || i0%7UL==6UL && i0<70UL) {
         /* has correct ax.25 address field len */
         cl = m->frame[dlen-2UL];
         ch = m->frame[dlen-1UL];
         aprsstr_AppCRC(m->frame, 351ul, (int32_t)(dlen-2UL));
         crcok = cl==m->frame[dlen-2UL] && ch==m->frame[dlen-1UL];
         txd = (int32_t)((m->txd*1000UL)/m->baud);
         level = dB((X2C_DIVR(m->level,(float)m->cnt))*3.1830988619289E-1f);
         snrr = dB(X2C_DIVR(m->level,m->noise+0.00001f));
         rfdb = dB(m->rflevel)*0.5f-90.3f; /* -dds gain */
         afc = (int32_t)X2C_TRUNCI((m->afc+m->fe)*(float)m->ifsamprate*1.5915494309644E-1f,X2C_min_longint,
                X2C_max_longint);
         qual = (int32_t)X2C_TRUNCI(100.0f*(1.0f-(X2C_DIVR(m->qual,m->level))*0.5f),X2C_min_longint,X2C_max_longint);
         if (crcok) {
            if (verb && !m->rawbits) {
               ShowFrame(m->frame, 351ul, dlen-2UL, (char)(m->modemnum+48UL), level, snrr, txd, afc, qual);
            }
            if (m->rawbits) {
               AIS(m->frame, 351ul, dlen-2UL, (char)(m->modemnum+48UL), afc, rfdb, m->jmhz);
            }
            else sendaxudp2(m->frame, 351ul, dlen, snrr, rfdb, txd, afc, qual, 0, 1);
         }
         if (judpport) {
            if (m->demod=='f') strncpy(ms,"fsk",100u);
            else strncpy(ms,"afsk",100u);
            sendjson(jipnum, judpport, m->frame, 351ul, ms, 100ul, dlen, 0UL, (float)m->baud, 0L, afc, crcok, 0.0f,
                rfdb, m->jmhz);
         }
      }
   }
} /* end afskframe() */


static void databit(char d, float u1, float u2, struct AFSKMODEM * m)
{
   float u;
   if (m->zerocnt<5UL && m->bitcnt/8UL<=350UL) {
      m->frame[m->bitcnt/8UL] = (char)((uint32_t)(uint8_t)m->frame[m->bitcnt/8UL]/2UL+128UL*(uint32_t)d);
      ++m->bitcnt;
   }
   if (u1>u2) {
      u = u1;
      u1 = u2;
      u2 = u;
   }
   u1 = (float)sqrt((double)u1);
   u2 = (float)sqrt((double)u2);
   m->level = m->level+u2;
   m->noise = m->noise+u1;
   if (m->lastu!=0.0f) m->qual = m->qual+(float)fabs(u2-m->lastu);
   m->lastu = u2;
   ++m->cnt;
   ++m->txc;
   if (d) {
      ++m->zerocnt;
      if (m->zerocnt>=6UL) {
         /* flag, frame ready */
         afskframe(m);
         m->bitcnt = 0UL;
         m->afc = 0.0f;
         m->level = 0.0f;
         m->qual = 0.0f;
         m->noise = 0.0f;
         m->cnt = 0UL;
         m->lastu = 0.0f;
         m->txd = m->txc;
      }
   }
   else {
      if (m->zerocnt>0UL && m->zerocnt<6UL) m->txc = 0UL;
      m->zerocnt = 0UL;
   }
} /* end databit() */


static void subbit(float u1, float u2, struct AFSKMODEM * m)
{
   char b;
   char d;
   uint32_t c;
   b = u1>u2;
   c = m->syncnt;
   m->syncnt = m->syncnt+2UL&31UL;
   if (m->syncnt<c) {
      /* data bit */
      d = b==m->oldd; /* nrzi */
      m->oldd = b;
      databit(d, u1, u2, m);
   }
   else if (b!=m->oldbit) {
      /* level change used for bit clock */
      if (m->syncnt!=16UL) {
         /* no clock adjust */
         m->syncnt = (m->syncnt-1UL)+(uint32_t)(m->syncnt<16UL)*2UL; /* up or down */
      }
   }
   m->oldbit = b;
} /* end subbit() */


static void fskbit(float u, struct AFSKMODEM * m)
{
   char b;
   char d;
   b = u>0.0f;
   m->syncnt += m->baud;
   if (m->syncnt>=m->ifsamprate) {
      /* data bit */
      m->syncnt = 0UL;
      /*data clock */
      if (m->oldraw!=b) {
         m->oldraw = b;
         if (b==m->synd) m->dir = 1L;
         else m->dir = -1L;
      }
      /*descramble */
      if (m->scrambled) {
         m->scrambler = X2C_LSH(m->scrambler,32,1);
         if (b) m->scrambler |= 0x1UL;
         b = ((0x1UL & m->scrambler)!=0)==(((0x1000UL & m->scrambler)!=0)==((0x20000UL & m->scrambler)!=0));
                /* result is xor bit 0 12 17 */
      }
      /*nrzi */
      d = b==m->oldd; /* nrzi */
      m->oldd = b;
      if (u>=0.0f) databit(d, u, 0.0f, m);
      else databit(d, 0.0f, -u, m);
   }
   else if (m->syncnt*2UL==m->ifsamprate) m->synd = b;
} /* end fskbit() */


static void sampleafsk(int32_t ire, int32_t iim, struct AFSKMODEM * m)
{
   uint32_t dm;
   uint32_t c;
   float ss;
   float lev;
   float af;
   float co;
   float si;
   int32_t ico;
   int32_t isi;
   struct Complex s;
   float f[2];
   struct Complex * anonym;
   isi = (int32_t)DDS[m->ifosc&16383U];
   ico = (int32_t)DDS[m->ifosc+4096U&16383U];
   m->ifosc += m->iffreqafc;
   lp((float)(iim*ico-ire*isi), &m->iflpq);
   lp((float)(ire*ico+iim*isi), &m->iflpi);
   c = m->ifsamp;
   dm = m->ifstep;
   if (m->dir>0L) dm += m->ifstep/8UL;
   else if (m->dir<0L) dm -= m->ifstep/8UL;
   m->dir = 0L;
   m->ifsamp += dm; /* wrap around 32bit */
   if (m->ifsamp<c) {
      /* input samplerate down to if-samplerate */
      s.Re = m->iflpq.uc2;
      s.Im = m->iflpi.uc2;
      af = fmhighpass(atan20(s), &m->oldaf); /* baseband audio */
      /*smeter */
      lev = 1.E-7f+s.Re*s.Re+s.Im*s.Im;
      m->rflevel = m->rflevel+(lev-m->rflevel)*0.02f;
      /*soundout */
      if (m->soundfd>=0L) {
         if (m->ademod=='F') {
            if (m->sqlev>0L) {
               ss = X2C_DIVR((float)fabs(m->lastlev-lev),m->lastlev+lev);
               m->sq = m->sq+(ss-m->sq)*0.01f;
               m->lastlev = lev;
            }
            if (m->sqlev==0L || m->sq<(float)m->sqlev*0.01f) {
               wavw(af, m->soundfd, m->afbuf, 4096ul, &m->afbufw);
            }
         }
         else if (m->ademod=='S') {
            usbout(s, m->soundfd, &m->usbosc, &m->usbgain, &m->afbufw, m->afbuf, 4096ul);
         }
      }
      /*-afc */
      m->afc = m->afc+af*0.01f;
      if (m->afc>m->afclimit) m->afc = m->afclimit;
      else if (m->afc<-m->afclimit) m->afc = -m->afclimit;
      else if (m->afc>0.0f) m->afc = m->afc-0.0002f;
      else if (m->afc<0.0f) m->afc = m->afc+0.0002f;
      m->iffreqafc = (uint16_t)((short)m->iffreq-(short)X2C_TRUNCI(m->afc*m->afcmul,-32768,32767));
      m->fe = m->fe+(af-m->fe)*0.001f; /* only for monitoring */
      if (m->demod=='f') {
         lp(af, &m->fsklp);
         if (m->fsklp.uc2>m->clamph) m->clamph = m->fsklp.uc2;
         if (m->fsklp.uc2<m->clampl) m->clampl = m->fsklp.uc2;
         m->clamph = m->clamph*0.99f;
         m->clampl = m->clampl*0.99f;
         fskbit(m->fsklp.uc2-(m->clamph+m->clampl)*0.5f, m);
      }
      else if (m->demod=='a') {
         for (c = 0UL; c<=1UL; c++) {
            si = DDSS[m->osc[c]&4095U];
            co = DDSS[m->osc[c]+1024U&4095U];
            m->osc[c] += m->freq[c];
            { /* with */
               struct Complex * anonym = &s;
               lp(af*si, &m->baselp[c*2UL]);
               lp(af*co, &m->baselp[c*2UL+1UL]);
            }
            f[c] = sqr(m->baselp[c*2UL].uc2)+sqr(m->baselp[c*2UL+1UL].uc2);
         } /* end for */
         subbit(f[0U], f[1U], m);
      }
   }
/* fm demod only */
} /* end sampleafsk() */

/*--------------- (a)fsk modem */
#define fsk4rx_MAXAGE 600
/* seconds payload active */


static char check(uint32_t num1)
/* check if this payload num is active else send only with good snr */
{
   uint32_t age;
   uint32_t t;
   uint32_t i0;
   t = osic_time();
   for (i0 = 0UL; i0<=10UL; i0++) {
      if (num1==checktable[i0].payload) {
         /* playload is in table */
         age = t-checktable[i0].time0;
         checktable[i0].time0 = t; /* heard now */
         return age<600UL;
      }
   } /* end for */
   for (i0 = 10UL; i0>=1UL; i0--) {
      checktable[i0] = checktable[i0-1UL]; /* place for new entry */
   } /* end for */
   checktable[0U].time0 = t;
   checktable[0U].payload = num1;
   return 0;
} /* end check() */


static float SaveReal(uint32_t c)
{
   uint32_t e;
   e = c/16777216UL;
   if (e==127UL) return X2C_max_real;
   if (e==255UL) return X2C_min_real;
   return *X2C_CAST(&c,uint32_t,float,float *);
} /* end SaveReal() */

#define fsk4rx_SATMOD 50


static void sendframe(const FRAME f, uint32_t len, float snrl, float rfdb, float afc, float jmhz,
                uint32_t corbit, float baud, int32_t ppm, char crcok, const FECINFO fecinfo)
{
   uint32_t p;
   uint32_t sat;
   uint32_t speed;
   uint32_t alt;
   uint32_t td;
   uint32_t seq;
   uint32_t plnum;
   int32_t temp;
   int32_t ppmi;
   double long0;
   double lat;
   float batt;
   float snr;
   char nocall[201];
   char name[201];
   char h[201];
   char s[201];
   p = 0UL;
   if (len==32UL) {
      plnum = (uint32_t)(uint8_t)f[0U]+(uint32_t)(uint8_t)f[1U]*256UL;
      seq = (uint32_t)(uint8_t)f[2U]+(uint32_t)(uint8_t)f[3U]*256UL;
      p = 4UL;
      batt = (float)(uint32_t)(uint8_t)f[20U]*1.953125E-2f;
   }
   else if (len==22UL) {
      plnum = (uint32_t)(uint8_t)f[0U];
      seq = (uint32_t)(uint8_t)f[1U]+(uint32_t)(uint8_t)f[2U]*256UL;
      p = 3UL;
      /*    batt:=1.0 + FLOAT(ORD(f[p+16]))*(4.0/256.0); */
      batt = (float)(uint32_t)(uint8_t)f[19U]*1.953125E-2f;
   }
   if (p==0UL) return;
   if (batt<1.8f) batt = batt*2.0f;
   td = (uint32_t)(uint8_t)f[p]*3600UL+(uint32_t)(uint8_t)f[p+1UL]*60UL+(uint32_t)(uint8_t)f[p+2UL];
   lat = (double)SaveReal((uint32_t)(uint8_t)f[p+6UL]*16777216UL+(uint32_t)(uint8_t)
                f[p+5UL]*65536UL+(uint32_t)(uint8_t)f[p+4UL]*256UL+(uint32_t)(uint8_t)f[p+3UL]);
   if (lat>90.0) lat = 90.0;
   else if (lat<(-90.0)) lat = (-90.0);
   long0 = (double)SaveReal((uint32_t)(uint8_t)f[p+10UL]*16777216UL+(uint32_t)(uint8_t)
                f[p+9UL]*65536UL+(uint32_t)(uint8_t)f[p+8UL]*256UL+(uint32_t)(uint8_t)f[p+7UL]);
   if (long0>180.0) long0 = 180.0;
   else if (long0<(-180.0)) long0 = (-180.0);
   alt = (uint32_t)(uint8_t)f[p+11UL]+(uint32_t)(uint8_t)f[p+12UL]*256UL;
   speed = (uint32_t)(uint8_t)f[p+13UL];
   sat = (uint32_t)(uint8_t)f[p+14UL];
   temp = (int32_t)(uint32_t)(uint8_t)f[p+15UL];
   if (temp>=128L) temp -= 256L;
   snr = 0.0f;
   if (snrl>0.0f) snr = dB(snrl)*0.5f;
   ppmi = ppm*(int32_t)(1000000UL/((16UL*(len*16UL+1UL))/2UL));
   if (verb && (crcok || squelch4<snr)) {
      osi_WrStr("[", 2ul);
      osi_WrStr(fecinfo, 101ul);
      osi_WrStr("] len:", 7ul);
      osic_WrINT32(len, 1UL);
      if (crcok) osi_WrStr(" crc:ok", 8ul);
      else osi_WrStr(" crc:err", 9ul);
      osi_WrStr(" ", 2ul);
      aprsstr_TimeToStr(osic_time()%86400UL, s, 201ul);
      osi_WrStr(s, 201ul);
      osi_WrStr(" id:", 5ul);
      osic_WrUINT32(plnum, 1UL);
      osi_WrStr(" seq:", 6ul);
      osic_WrINT32(seq, 1UL);
      osi_WrStr(" td:", 5ul);
      aprsstr_TimeToStr(td%86400UL, s, 201ul);
      osi_WrStr(s, 201ul);
      osi_WrStr(" lat:", 6ul);
      osic_WrFixed((float)lat, 5L, 1UL);
      osi_WrStr(" long:", 7ul);
      osic_WrFixed((float)long0, 5L, 1UL);
      osi_WrStr(" alt:", 6ul);
      osic_WrINT32(alt, 1UL);
      osi_WrStr(" speed:", 8ul);
      osic_WrINT32(speed, 1UL);
      osi_WrStr(" sat:", 6ul);
      osic_WrINT32(sat%50UL, 1UL);
      osi_WrStr(" gpspower:", 11ul);
      osic_WrINT32(sat/50UL, 1UL);
      osi_WrStr(" batt:", 7ul);
      osic_WrFixed(batt, 2L, 1UL);
      osi_WrStr(" snr:", 6ul);
      osic_WrFixed(snr, 1L, 1UL);
      osi_WrStr("dB", 3ul);
      osi_WrStr(" rf:", 5ul);
      osic_WrFixed(rfdb, 1L, 1UL);
      osi_WrStr("dB", 3ul);
      osi_WrStr(" afc:", 6ul);
      osic_WrFixed(afc, 1L, 1UL);
      osi_WrStr("Hz", 3ul);
      osi_WrStr(" ppm:", 6ul);
      osic_WrINT32((uint32_t)ppmi, 1UL);
      osi_WrStrLn("", 1ul);
   }
   if ((((crcok && (snr>8.0f || check(plnum))) && fabs(lat)<90.0) && fabs(long0)<180.0) && (lat!=0.0 || long0!=0.0)) {
      strncpy(s,"Seq:",201u);
      aprsstr_CardToStr(seq, 1UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, " Sat:", 6ul);
      aprsstr_CardToStr(sat%50UL, 1UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      if (sat/50UL==1UL) aprsstr_Append(s, 201ul, "(powersafe=off)", 16ul);
      else if (sat/50UL==2UL) aprsstr_Append(s, 201ul, "(powersafe=tracking)", 21ul);
      else if (sat/50UL==4UL) aprsstr_Append(s, 201ul, "(powersafe=opt)", 16ul);
      /*    Append(s, " ");TimeToStr(td MOD 86400, h); Append(s, h);  */
      aprsstr_Append(s, 201ul, " Temp:", 7ul);
      aprsstr_IntToStr(temp, 1UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "C Batt:", 8ul);
      aprsstr_FixToStr(batt, 3UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "V", 2ul);
      appmhz(s, 201ul, jmhz);
      aprsstr_Append(s, 201ul, " rf:", 5ul);
      aprsstr_FixToStr(rfdb, 2UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "dB", 3ul);
      aprsstr_Append(s, 201ul, " afc:", 6ul);
      aprsstr_IntToStr((int32_t)X2C_TRUNCI(afc,X2C_min_longint,X2C_max_longint), 1UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, "Hz", 3ul);
      if (snr>0.0f) {
         aprsstr_Append(s, 201ul, " snr:", 6ul);
         aprsstr_FixToStr(snr, 2UL, h, 201ul);
         aprsstr_Append(s, 201ul, h, 201ul);
         aprsstr_Append(s, 201ul, "dB", 3ul);
      }
      aprsstr_Append(s, 201ul, " ppm:", 6ul);
      aprsstr_IntToStr(ppmi, 1UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      aprsstr_Append(s, 201ul, " fec:", 6ul);
      aprsstr_CardToStr(corbit, 1UL, h, 201ul);
      aprsstr_Append(s, 201ul, h, 201ul);
      findcall(plnum&65535UL, name, 201ul, nocall, 201ul);
      if (name[0]==0) {
         /* call to payload num not found so build a name */
         aprsstr_CardToStr(plnum&65535UL, 1UL, h, 201ul);
         while (aprsstr_Length(h, 201ul)<4UL) aprsstr_Append(h, 201ul, " ", 2ul);
         strncpy(name,"HORU",201u);
         if (aprsstr_Length(h, 201ul)<=4UL) aprsstr_Append(name, 201ul, "S", 2ul);
         aprsstr_Append(name, 201ul, h, 201ul);
         if (nocall[0]) {
            aprsstr_Append(s, 201ul, " [", 3ul);
            aprsstr_Append(s, 201ul, nocall, 201ul);
            aprsstr_Append(s, 201ul, "]", 2ul);
         }
      }
      else {
         aprsstr_CardToStr(plnum&65535UL, 1UL, h, 201ul);
         aprsstr_Append(s, 201ul, " id:", 5ul);
         aprsstr_Append(s, 201ul, h, 201ul);
      }
      name[9U] = 0;
      encodeaprs(name, 201ul, td, mycall, 10ul, s, 201ul, "/O", 3ul, "APLHOR", 7ul, lat, long0, 0L,
                (int32_t)X2C_TRUNCI(afc,X2C_min_longint,X2C_max_longint), 0L, snr, rfdb, (float)speed, (-1.0f),
                (float)alt, 10000.0f, 1.E+5f, 1.E+5f, 1.E+5f, 1.E+5f, verb);
   }
   if (judpport) {
      sendjson(jipnum, judpport, f, 101ul, "4fsk", 5ul, len, corbit, baud, ppmi, (int32_t)X2C_TRUNCI(afc,
                X2C_min_longint,X2C_max_longint), crcok, snr, rfdb, jmhz);
   }
} /* end sendframe() */


static uint32_t maxbits(uint32_t lenbyte)
{
   return (lenbyte+(((lenbyte*8UL+11UL)/12UL)*11UL+7UL)/8UL)*8UL;
} /* end maxbits() */

#define fsk4rx_X22 0x400000 
/* vector representation of X^{22} */

#define fsk4rx_X11 0x800 
/* vector representation of X^{11} */

#define fsk4rx_MASK12 0x0FFFFF800
/* auxiliary vector for testing */

#define fsk4rx_GENPOL 0xC75 
/* generator polinomial, g(x) */


static int32_t arr2int(int32_t a[], uint32_t a_len, int32_t r)
/*
 * Convert a binary vector of Hamming weight r, and nonzero positions in
 * array a[1]...a[r], to a long integer \sum_{i=1}^r 2^{a[i]-1}.
 */
{
   int32_t i0;
   uint32_t mul;
   uint32_t result;
   uint32_t temp;
   int32_t tmp;
   int32_t arr2int_ret;
   X2C_PCOPY((void **)&a,a_len*4u);
   result = 0UL;
   tmp = r;
   i0 = 1L;
   if (i0<=tmp) for (;; i0++) {
      mul = 1UL;
      temp = (uint32_t)(a[i0]-1L);
      while (temp>0UL) {
         --temp;
         mul += mul;
      }
      result += mul;
      if (i0==tmp) break;
   } /* end for */
   arr2int_ret = (int32_t)result;
   X2C_PFREE(a);
   return arr2int_ret;
} /* end arr2int() */


static void nextcomb(int32_t n, int32_t r, int32_t a[], uint32_t a_len)
/*
 * Calculate next r-combination of an n-set.
 */
{
   int32_t j;
   int32_t i0;
   int32_t tmp;
   ++a[r];
   if (a[r]<=n) return;
   j = r-1L;
   while (a[j]==(n-r)+j) --j;
   tmp = j;
   i0 = r;
   if (i0>=tmp) for (;; i0--) {
      a[i0] = ((a[j]+i0)-j)+1L;
      if (i0==tmp) break;
   } /* end for */
} /* end nextcomb() */


static uint32_t getsyndrome(uint32_t pattern)
/*
 * Compute the syndrome corresponding to the given pattern, i.e., the
 * remainder after dividing the pattern (when considering it as the vector
 * representation of a polynomial) by the generator polynomial, GENPOL.
 * In the program this pattern has several meanings: (1) pattern = infomation
 * bits, when constructing the encoding table; (2) pattern = error pattern,
 * when constructing the decoding table; and (3) pattern = received vector, to
 * obtain its 0syndrome in decoding.
 */
{
   uint32_t aux;
   aux = 4194304UL;
   if (pattern>=2048UL) {
      while (((uint32_t)pattern&0xFFFFF800UL)!=0UL) {
         while (((uint32_t)aux&(uint32_t)pattern)==0UL) aux = aux>>1;
         pattern = (uint32_t)((uint32_t)pattern^(uint32_t)((aux/2048UL)*3189UL));
      }
   }
   return pattern;
} /* end getsyndrome() */


static void fec(char f[], uint32_t f_len, uint32_t len, char verb0, uint32_t * corcnt,
                char fecinfo[], uint32_t fecinfo_len)
{
   int32_t a[4];
   uint32_t i0;
   uint32_t temp;
   uint32_t cnt;
   uint32_t errmap;
   uint32_t p;
   uint32_t g;
   uint32_t b;
   uint32_t j;
   char tmp;
   if (decodingtable[0U]==X2C_max_longcard) {
      decodingtable[0U] = 0UL;
      decodingtable[1U] = 1UL;
      temp = 1UL;
      for (i0 = 2UL; i0<=23UL; i0++) {
         temp += temp;
         decodingtable[getsyndrome(temp)] = temp;
      } /* end for */
      a[1U] = 1L;
      a[2U] = 2L;
      temp = (uint32_t)arr2int(a, 4ul, 2L);
      decodingtable[getsyndrome(temp)] = temp;
      for (i0 = 1UL; i0<=252UL; i0++) {
         nextcomb(23L, 2L, a, 4ul);
         temp = (uint32_t)arr2int(a, 4ul, 2L);
         decodingtable[getsyndrome(temp)] = temp;
      } /* end for */
      a[1U] = 1L;
      a[2U] = 2L;
      a[3U] = 3L;
      temp = (uint32_t)arr2int(a, 4ul, 3L);
      decodingtable[getsyndrome(temp)] = temp;
      for (i0 = 1UL; i0<=1770UL; i0++) {
         nextcomb(23L, 3L, a, 4ul);
         temp = (uint32_t)arr2int(a, 4ul, 3L);
         decodingtable[getsyndrome(temp)] = temp;
      } /* end for */
   }
   fecinfo[0UL] = 0;
   *corcnt = 0UL;
   i0 = 0UL;
   for (;;) {
      if (i0>=(len*8UL+11UL)/12UL) break;
      p = 0UL;
      for (j = 0UL; j<=10UL; j++) {
         b = i0*11UL+j;
         p += p+(uint32_t)X2C_IN(7UL-(b&7UL),8,(uint8_t)(uint8_t)f[len+b/8UL]);
      } /* end for */
      g = 0UL;
      for (j = 0UL; j<=11UL; j++) {
         b = i0*12UL+j;
         if (b>=len*8UL) {
            goto loop_exit; /* do not repair partial filled syndrom as sender encodes with junk in unfilled bits */
         }
         g += g+(uint32_t)X2C_IN(7UL-(b&7UL),8,(uint8_t)(uint8_t)f[b/8UL]);
      } /* end for */
      g += p<<12;
      errmap = decodingtable[getsyndrome(g)];
      if (errmap) {
         cnt = 0UL;
         for (j = 0UL; j<=22UL; j++) {
            cnt += (uint32_t)X2C_IN(j,32,(uint32_t)errmap);
         } /* end for */
         if (cnt<=3UL) {
            for (j = 0UL; j<=11UL; j++) {
               if (X2C_IN(11UL-j,32,(uint32_t)errmap)) {
                  b = i0*12UL+j;
                  f[b/8UL] = (char)((uint8_t)(uint8_t)f[b/8UL]^(1U<<7UL-(b&7UL)));
                  ++*corcnt;
               }
            } /* end for */
            aprsstr_Append(fecinfo, fecinfo_len, (char *)(tmp = (char)(cnt+48UL),&tmp), 1u/1u);
         }
         else aprsstr_Append(fecinfo, fecinfo_len, "-", 2ul);
      }
      else aprsstr_Append(fecinfo, fecinfo_len, "+", 2ul);
      ++i0;
   }
   loop_exit:;
} /* end fec() */


static void scramble(char f[], uint32_t f_len, uint32_t len)
{
   uint32_t i0;
   uint16_t o;
   uint16_t s;
   uint32_t tmp;
   s = 0x4A80U;
   tmp = maxbits(len)-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      o = (s^X2C_LSH(s,16,-1))&0x1U;
      s = X2C_LSH(s,16,-1)|X2C_LSH(o,16,14);
      f[i0/8UL] = (char)(uint16_t)((uint16_t)(uint32_t)(uint8_t)f[i0/8UL]^X2C_LSH(o,16,
                (int32_t)(i0&7UL)));
      if (i0==tmp) break;
   } /* end for */
} /* end scramble() */

static uint16_t fsk4rx_P[77] = {2U,3U,5U,7U,11U,13U,17U,19U,23U,29U,31U,37U,41U,43U,47U,53U,59U,61U,67U,71U,73U,79U,
                83U,89U,97U,101U,103U,107U,109U,113U,127U,131U,137U,139U,149U,151U,157U,163U,167U,173U,179U,181U,191U,
                193U,197U,199U,211U,223U,227U,229U,233U,239U,241U,251U,257U,263U,269U,271U,277U,281U,283U,293U,307U,
                311U,313U,317U,331U,337U,347U,349U,379U,383U,389U,757U,761U,769U,773U};

static uint16_t _cnst[77] = {2U,3U,5U,7U,11U,13U,17U,19U,23U,29U,31U,37U,41U,43U,47U,53U,59U,61U,67U,71U,73U,79U,83U,
                89U,97U,101U,103U,107U,109U,113U,127U,131U,137U,139U,149U,151U,157U,163U,167U,173U,179U,181U,191U,193U,
                197U,199U,211U,223U,227U,229U,233U,239U,241U,251U,257U,263U,269U,271U,277U,281U,283U,293U,307U,311U,
                313U,317U,331U,337U,347U,349U,379U,383U,389U,757U,761U,769U,773U};

static void deinterleave(char f[], uint32_t f_len, uint32_t len)
{
   uint32_t p;
   uint32_t bits0;
   uint32_t maxlen;
   uint32_t j;
   uint32_t i0;
   uint8_t o[256];
   uint32_t tmp;
   maxlen = maxbits(len)/8UL;
   bits0 = maxlen*8UL;
   i0 = 1UL;
   while (i0<=76UL && (uint32_t)_cnst[i0]<bits0) ++i0;
   p = (uint32_t)_cnst[i0-1UL];
   tmp = maxlen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      o[i0] = 0U;
      if (i0==tmp) break;
   } /* end for */
   tmp = bits0-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      j = (p*i0)%bits0;
      if (X2C_IN(j&7UL,8,(uint8_t)(uint8_t)f[j/8UL])) o[i0/8UL] |= (1U<<(i0&7UL));
      if (i0==tmp) break;
   } /* end for */
   tmp = maxlen-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      f[i0] = (char)o[i0];
      if (i0==tmp) break;
   } /* end for */
} /* end deinterleave() */


static char decodeframe(FRAME frame, uint32_t dlen, uint32_t dm, float levm, float noism,
                float rfdb, float afc, int32_t ppm, char dryrun, struct FSK4MODEM * m)
/* 4fsk */
{
   char crcok;
   uint32_t cor;
   float snr;
   FECINFO fecinfo;
   FRAME tmp;
   frame = (char *)memcpy(tmp,frame,101u);
   if (dlen>2UL) {
      scramble(frame, 101ul, dlen);
      deinterleave(frame, 101ul, dlen);
      fec(frame, 101ul, dlen, verb && !dryrun, &cor, fecinfo, 101ul);
      crcok = (uint32_t)(uint8_t)frame[dlen-2UL]+(uint32_t)(uint8_t)frame[dlen-1UL]*256UL==(uint32_t)
                crc16(frame, 101ul, dlen-2UL);
      if (!dryrun) {
         snr = 0.0f;
         if (noism>0.0f) snr = X2C_DIVR(levm,noism);
         if ((crcok || !m->bestframe.crcok) && ((crcok && !m->bestframe.crcok || cor<m->bestframe.cor)
                || snr>m->bestframe.snr)) {
            memcpy(m->bestframe.frame,frame,101u);
            m->bestframe.len = dlen;
            m->bestframe.snr = snr;
            m->bestframe.afc = (((float)dm-(float)(m->demods/2UL))+0.5f)*(X2C_DIVR(m->spaceing,
                (float)m->subspaceing));
            m->bestframe.cor = cor;
            m->bestframe.crcok = crcok;
            m->bestframe.ppm = ppm;
            m->bestframe.rfdb = dB(rfdb)*0.5f-90.3f;
            memcpy(m->bestframe.fecinfo,fecinfo,101u);
            m->bestframe.timer = 32UL;
         }
      }
   }
   return crcok;
} /* end decodeframe() */

#define fsk4rx_SYNTOLERANCE 2


static void nibble(uint32_t dm, uint32_t n, float lev, float noise, struct FSK4MODEM * m)
{
   uint32_t j;
   uint32_t i0;
   int32_t p;
   char ok0;
   uint32_t e;
   struct DEMOD * anonym;
   { /* with */
      struct DEMOD * anonym = &m->demod[dm];
      if (anonym->lastn!=n) {
         p = 0L;
         if (anonym->symclk>8UL) p = -1L;
         else if (anonym->symclk<8UL) p = 1L;
         anonym->symclk += (uint32_t)p;
         anonym->ppm += p;
         anonym->lastn = n;
      }
      ++anonym->symclk;
      if (anonym->symclk>=16UL) {
         anonym->symclk = 0UL;
         anonym->synword = (anonym->synword<<2)+n; /* gray coded symbol would have been nice ... */
         if (anonym->framewp==0UL) {
            /* hunt mode */
            e = (uint32_t)anonym->synword^0x1B1B2424UL;
            if ((e&0xFFFFUL)==0UL) {
               /* start word correct, error tolerant syn bits */
               j = 0UL;
               i0 = 16UL;
               do {
                  if (X2C_IN(i0,32,e)) ++j;
                  ++i0;
               } while (!(i0>=32UL || j>=2UL));
            }
            else j = 3UL;
            if (j<=2UL) {
               /* start of frame */
               anonym->framewp = 1UL;
               anonym->noism = 0.0f;
               anonym->levm = 0.0f;
               anonym->afc = 0.0f;
               anonym->ppm = 0L;
            }
         }
         else {
            /* in frame */
            anonym->frame[(anonym->framewp-1UL)/8UL] = (char)(anonym->synword&255UL);
            anonym->framewp += 2UL;
            if (anonym->framewp-1UL>=maxbits(22UL) && decodeframe(anonym->frame, 22UL, dm, lev, anonym->noism, lev,
                anonym->afc, anonym->ppm, 1, m)) {
               ok0 = decodeframe(anonym->frame, 22UL, dm, anonym->levm, anonym->noism, lev, anonym->afc, anonym->ppm, 0,
                 m);
               anonym->framewp = 0UL;
            }
            else if (anonym->framewp-1UL>=maxbits(32UL)) {
               ok0 = decodeframe(anonym->frame, 32UL, dm, anonym->levm, anonym->noism, lev, anonym->afc, anonym->ppm, 0,
                 m);
               anonym->framewp = 0UL;
            }
            anonym->noism = anonym->noism+noise;
            anonym->levm = anonym->levm+lev;
         }
      }
   }
} /* end nibble() */


static void sample(int32_t ire, int32_t iim, struct FSK4MODEM * m)
{
   uint32_t j;
   uint32_t dm;
   uint32_t cmax;
   uint32_t c;
   float co;
   float si;
   float med;
   float max0;
   int32_t ico;
   int32_t isi;
   struct Complex s;
   struct Complex * anonym;
   uint32_t tmp;
   /*  isi:=DDS[ifosc]; */
   /*  ico:=DDS[CAST(CARD16, ifosc+(HIGH(DDS)+1) DIV 4)]; */
   isi = (int32_t)DDS[m->ifosc&16383U];
   ico = (int32_t)DDS[m->ifosc+4096U&16383U];
   m->ifosc += m->iffreq;
   lp((float)(iim*ico-ire*isi), &m->iflpq);
   lp((float)(ire*ico+iim*isi), &m->iflpi);
   c = m->ifsamp;
   m->ifsamp += m->ifstep; /* wrap around 32bit */
   if (m->ifsamp<c) {
      /* input samplerate down to if-samplerate */
      s.Re = m->iflpq.uc2;
      s.Im = m->iflpi.uc2;
      if (m->ademod=='F') fmoutc(s, m->soundfd, &m->wavoldaf, m->afbuf, 4096ul, &m->afbufw);
      else if (m->ademod=='S') usbout(s, m->soundfd, &m->usbosc, &m->usbgain, &m->afbufw, m->afbuf, 4096ul);
      tmp = m->subbands-1UL;
      c = 0UL;
      if (c<=tmp) for (;; c++) {
         si = DDSS[m->osc[c]&4095U];
         co = DDSS[m->osc[c]+1024U&4095U];
         m->osc[c] += m->freq[c];
         { /* with */
            struct Complex * anonym = &s;
            lp(anonym->Im*si-anonym->Re*co, &m->baselp[c*2UL]);
            lp(anonym->Re*si+anonym->Im*co, &m->baselp[c*2UL+1UL]);
         }
         if (c==tmp) break;
      } /* end for */
      ++m->baseoversamp;
      if (m->baseoversamp>=5UL) {
         /* if-samplerate down to symbol oversamplerate */
         m->baseoversamp = 0UL;
         tmp = m->demods-1UL;
         dm = 0UL;
         if (dm<=tmp) for (;; dm++) {
            /* all demodulators in subspacing steps */
            max0 = (-1.0f);
            med = 1.E-6f;
            for (c = 0UL; c<=3UL; c++) {
               /* 4 4fsk frequencys */
               j = (dm+c*m->subspaceing)*2UL;
               si = sqr(m->baselp[j].uc2)+sqr(m->baselp[j+1UL].uc2);
               if (si>max0) {
                  max0 = si; /* fmax:=f */
                  cmax = c;
               }
               med = med+si;
            } /* end for */
            med = (med-max0)*3.3333333333333E-1f; /* for noisefloor sum other filters as max */
            nibble(dm, cmax, max0, med, m);
            if (dm==tmp) break;
         } /* end for */
      }
      if (m->bestframe.timer>0UL) {
         /* we have a frame to send */
         --m->bestframe.timer;
         if (m->bestframe.timer==0UL) {
            sendframe(m->bestframe.frame, m->bestframe.len, m->bestframe.snr, m->bestframe.rfdb, m->bestframe.afc,
                m->jmhz, m->bestframe.cor, m->baud, m->bestframe.ppm, m->bestframe.crcok, m->bestframe.fecinfo);
            memset((char *) &m->bestframe,(char)0,sizeof(struct BESTFRAME));
         }
      }
   }
} /* end sample() */


static short realint(float x, float * g)
/* limit real input > +-1.0 to INT16*/
{
   float r;
   r = x* *g;
   if ((float)fabs(r)>=32767.0f) {
      *g = X2C_DIVR(1.073676289E+9f,r);
      r = x* *g;
   }
   return (short)X2C_TRUNCI(r,-32768,32767);
} /* end realint() */

union _1;


union _1 {
   struct Complex c[4096];
   short i0[16384];
   uint8_t b[32768];
};


static uint32_t inreform(short b[], uint32_t b_len)
{
   uint32_t wp0;
   uint32_t rs;
   uint32_t bs;
   uint32_t i0;
   int32_t res;
   union _1 ib;
   char * p;
   float g;
   uint32_t tmp;
   bs = isize*8192UL;
   if (bs>((b_len-1)+1UL)*2UL) bs = ((b_len-1)+1UL)*2UL;
   rs = 0UL;
   do {
      p = (char *) &ib.b[rs];
      res = osi_RdBin(iqfd, p, 65536ul, bs-rs);
      if (res<=0L) return 0UL;
      rs += (uint32_t)res;
   } while (rs<bs);
   wp0 = 0UL;
   if (isize==1UL) {
      tmp = rs-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         b[i0] = (short)ib.b[i0]*256-32640;
         if (i0==tmp) break;
      } /* end for */
      wp0 = rs;
   }
   else if (isize==2UL) {
      /*    FOR i:=0 TO rs DIV 2-1 DO b[i]:=ib.i[i] END; */
      X2C_MOVE((char *) &ib,(char *)b,rs);
      wp0 = rs/2UL;
   }
   else {
      g = 32767.0f;
      tmp = rs/8UL-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         b[wp0] = realint(ib.c[i0].Re, &g);
         ++wp0;
         b[wp0] = realint(ib.c[i0].Im, &g);
         ++wp0;
         if (i0==tmp) break;
      } /* end for */
   }
   return wp0;
} /* end inreform() */

static void jsonpipebroken(int32_t);


static void jsonpipebroken(int32_t signum)
{
   osi_WrStr("got signal ", 12ul);
   osic_WrINT32((uint32_t)signum, 0UL);
   osi_WrStrLn("!", 2ul);
} /* end jsonpipebroken() */

static uint32_t wp;

static uint32_t i;

static pFSK4MODEM m4;

static pAFSKMODEM ma;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   uint32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FRAME)!=101) X2C_ASSERT(0);
   if (sizeof(FECINFO)!=101) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   strncpy(mycall,"NOCALL",10u);
   calls = 0;
   ships = 0;
   shipbeacons = 0;
   Parms();
   if (judpport>0UL) signal(SIGPIPE, jsonpipebroken);
   decodingtable[0U] = X2C_max_longcard;
   MakeDDSi(DDS, 16384ul);
   MakeDDS(DDSS, 4096ul);
   iqfd = osi_OpenRead(iqfn, 1024ul);
   if (iqfd<0L) Error("open iq file", 13ul);
   for (;;) {
      wp = inreform(iqbuf, 8192ul);
      if (wp==0UL) break;
      m4 = fsk4modems;
      while (m4) {
         tmp = wp-2UL;
         i = 0UL;
         if (i<=tmp) for (tmp = (uint32_t)(tmp-i)/2UL;;) {
            sample((int32_t)iqbuf[i], (int32_t)iqbuf[i+1UL], m4);
            if (!tmp) break;
            --tmp;
            i += 2UL;
         } /* end for */
         m4 = m4->next;
      }
      ma = afskmodems;
      while (ma) {
         tmp = wp-2UL;
         i = 0UL;
         if (i<=tmp) for (tmp = (uint32_t)(tmp-i)/2UL;;) {
            sampleafsk((int32_t)iqbuf[i], (int32_t)iqbuf[i+1UL], ma);
            if (!tmp) break;
            --tmp;
            i += 2UL;
         } /* end for */
         ma = ma->next;
      }
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
