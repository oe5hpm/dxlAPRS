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
#define lorarx_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef fft_H_
#include "fft.h"
#endif
#ifndef complex_H_
#include "complex.h"
#endif

/* iq lora demodulator */
/*--------------------------------------------------------------------------------

Synchronisation simple:

Sampling preamble upchirps give independent of sample time constant fft frequency.

So sample at any time with normal chirprate and, if it looks like an upchrip,
jump as many samples as the found frequency differs from zero.
 
As not known exact frequency and not at all sample time but frequency
is proportional to time, we first say we are (+-25%) on frequency so we get
a 75% exact sample time.

This is good enough for decoding the 2 sync chirps and switch to reverse chirps.

After fft of the 2 reverse chirps we get another new zero frequency.
The median of upchirps and downchirps frequencies makes the true receive frequency.

As we know now how far we are away from real frequency, we jump in sampletime half of
the upchirp-downchirp frequency difference (only full samples) + the quarter downchirp
(thanks to the quarter chirp, we never need back jumps) and add the frequency change
from the jump to the median of the upchirps frequencys.

Now we are 1 sample exact on sampletime and (as exact as the median of the
preamble) on frequency and can decode data. Further drifts in frequency and time
(samplerate error) may be corrected with AFC. Sampletime needs no further correction.

---------------------------------------------------------------------------------*/
#define lorarx_PI 3.1415926535

#define lorarx_PI2 6.283185307

#define lorarx_LF "\012"

#define lorarx_MAXSF 4096

#define lorarx_TIMEOUT 3
/* chips with no dcd is end of unkown lenght data */

#define lorarx_MAXBINS 4
/* look at weaker bins for fitting parity */

#define lorarx_FRAMELEN 256
/*      HUNTSQUELCH=11.0; */

/*      REVSQUELCH=8.0; */
#define lorarx_DATASQUELCH 1.5

#define lorarx_DATASQUELCHFACT 0.8
/* adapt squelch to sf */

#define lorarx_AFCSQUELCH 12.0
/* not modify afc with noisy chirps */
/*      MAXCOLLISIONS=-3; */

#define lorarx_MAXNEST 2
/* start new demodulator if other(s) in frame */

#define lorarx_MAXINBUF 2048

static uint8_t lorarx_WHITEN[255] = {255U,254U,252U,248U,240U,225U,194U,133U,11U,23U,47U,94U,188U,120U,241U,227U,198U,
                141U,26U,52U,104U,208U,160U,64U,128U,1U,2U,4U,8U,17U,35U,71U,142U,28U,56U,113U,226U,196U,137U,18U,37U,
                75U,151U,46U,92U,184U,112U,224U,192U,129U,3U,6U,12U,25U,50U,100U,201U,146U,36U,73U,147U,38U,77U,155U,
                55U,110U,220U,185U,114U,228U,200U,144U,32U,65U,130U,5U,10U,21U,43U,86U,173U,91U,182U,109U,218U,181U,
                107U,214U,172U,89U,178U,101U,203U,150U,44U,88U,176U,97U,195U,135U,15U,31U,62U,125U,251U,246U,237U,219U,
                183U,111U,222U,189U,122U,245U,235U,215U,174U,93U,186U,116U,232U,209U,162U,68U,136U,16U,33U,67U,134U,13U,
                27U,54U,108U,216U,177U,99U,199U,143U,30U,60U,121U,243U,231U,206U,156U,57U,115U,230U,204U,152U,49U,98U,
                197U,139U,22U,45U,90U,180U,105U,210U,164U,72U,145U,34U,69U,138U,20U,41U,82U,165U,74U,149U,42U,84U,169U,
                83U,167U,78U,157U,59U,119U,238U,221U,187U,118U,236U,217U,179U,103U,207U,158U,61U,123U,247U,239U,223U,
                191U,126U,253U,250U,244U,233U,211U,166U,76U,153U,51U,102U,205U,154U,53U,106U,212U,168U,81U,163U,70U,
                140U,24U,48U,96U,193U,131U,7U,14U,29U,58U,117U,234U,213U,170U,85U,171U,87U,175U,95U,190U,124U,249U,242U,
                229U,202U,148U,40U,80U,161U,66U,132U,9U,19U,39U,79U,159U,63U,127U};

#define lorarx_MAXBW 5.E+5

static uint32_t lorarx_BWTAB[10] = {64UL,48UL,32UL,24UL,16UL,12UL,8UL,4UL,2UL,1UL};

static uint8_t lorarx_HAMMTAB8[256] = {128U,192U,192U,3U,192U,5U,6U,135U,192U,9U,10U,139U,12U,141U,142U,15U,128U,1U,
                2U,199U,4U,199U,199U,135U,8U,137U,138U,11U,140U,13U,14U,199U,128U,1U,2U,131U,4U,133U,206U,7U,8U,137U,
                206U,11U,206U,13U,142U,206U,0U,201U,130U,3U,132U,5U,6U,135U,201U,137U,10U,201U,12U,201U,142U,15U,128U,
                1U,2U,203U,4U,133U,134U,7U,8U,203U,203U,139U,140U,13U,14U,203U,0U,129U,130U,3U,204U,5U,6U,135U,204U,9U,
                10U,139U,140U,204U,204U,15U,0U,197U,130U,3U,197U,133U,6U,197U,136U,9U,10U,139U,12U,197U,142U,15U,194U,
                1U,130U,194U,4U,133U,194U,7U,8U,137U,194U,11U,140U,13U,14U,143U,128U,1U,2U,131U,4U,205U,134U,7U,8U,205U,
                138U,11U,205U,141U,14U,205U,0U,129U,202U,3U,132U,5U,6U,135U,202U,9U,138U,202U,12U,141U,202U,15U,0U,195U,
                195U,131U,132U,5U,6U,195U,136U,9U,10U,195U,12U,141U,142U,15U,196U,1U,2U,131U,132U,196U,196U,7U,8U,137U,
                138U,11U,196U,13U,14U,143U,0U,129U,198U,3U,198U,5U,134U,198U,136U,9U,10U,139U,12U,141U,198U,15U,193U,
                129U,2U,193U,4U,193U,134U,7U,8U,193U,138U,11U,140U,13U,14U,143U,200U,1U,2U,131U,4U,133U,134U,7U,136U,
                200U,200U,11U,200U,13U,14U,143U,0U,129U,130U,3U,132U,5U,6U,207U,136U,9U,10U,207U,12U,207U,207U,143U};

static uint8_t lorarx_HAMMTAB7[128] = {128U,192U,192U,131U,192U,205U,134U,135U,192U,205U,138U,139U,205U,141U,142U,
                205U,128U,129U,202U,199U,132U,199U,199U,135U,202U,137U,138U,202U,140U,141U,202U,199U,128U,195U,195U,
                131U,132U,133U,206U,195U,136U,137U,206U,195U,206U,141U,142U,206U,196U,201U,130U,131U,132U,196U,196U,
                135U,201U,137U,138U,201U,196U,201U,142U,143U,128U,129U,198U,203U,198U,133U,134U,198U,136U,203U,203U,
                139U,140U,141U,198U,203U,193U,129U,130U,193U,204U,193U,134U,135U,204U,193U,138U,139U,140U,204U,204U,
                143U,200U,197U,130U,131U,197U,133U,134U,197U,136U,200U,200U,139U,200U,197U,142U,143U,194U,129U,130U,
                194U,132U,133U,194U,207U,136U,137U,194U,207U,140U,207U,207U,143U};

static uint8_t lorarx_HAMMTAB6[64] = {128U,1U,2U,3U,4U,5U,134U,7U,8U,9U,10U,139U,12U,141U,14U,15U,0U,129U,2U,3U,4U,5U,
                6U,135U,8U,9U,138U,11U,140U,13U,14U,15U,0U,1U,2U,131U,4U,133U,6U,7U,136U,9U,10U,11U,12U,13U,142U,15U,0U,
                1U,130U,3U,132U,5U,6U,7U,8U,137U,10U,11U,12U,13U,14U,143U};


struct CB {
   struct complex_Complex * Adr;
   size_t Len0;
};

typedef struct CB * pCB;

struct BIN;


struct BIN {
   float lev;
   float freq;
   uint32_t bn;
};

typedef struct BIN BINS[5];

enum STATES {lorarx_sSLEEP, lorarx_sHUNT, lorarx_sSYNRAW, lorarx_sID, lorarx_sREV1, lorarx_sREV2, lorarx_sDATA};


struct FFRAME;


struct FFRAME {
   BINS bintab[8];
   uint8_t nibbs[523];
   char fecinfo[96];
   uint32_t chirpc;
   uint32_t nibbc;
   uint32_t crfromhead;
   uint32_t dlen;
   uint32_t idfound;
   uint32_t cnt;
   uint32_t txdel;
   uint32_t jp;
   uint32_t cfgsf;
   uint32_t cfgcr;
   uint32_t synfilter;
   uint32_t timeout;
   uint32_t cfgdatalen;
   uint32_t iqread;
   uint32_t label;
   float fc;
   float fci;
   float fcstart;
   float sigsum;
   float noissum;
   float eye;
   float lastbin;
   float lastsq;
   float synmed;
   float lastrev;
   float afcspeed;
   float datasquelch;
   float synsquelch;
   char optimize;
   char withhead;
   char withcrc;
   char implicitcrc;
   char nodcdlost;
   int32_t df;
   uint8_t state;
   uint32_t ipnumraw;
   uint32_t udprawport;
   uint32_t ipnum;
   uint32_t udpport;
   char udp2;
};

struct DEM;

typedef struct DEM * pDEM;


struct DEM {
   pDEM next;
   int32_t cfgopt;
   uint32_t coldet;
   struct FFRAME frames[2];
};

static pDEM dems;

static char iqfn[1024];

static int32_t iqfd;

static uint32_t bwnum;

static uint32_t iqwrite;

static uint32_t isize;

static char verb;

static char verb2;

static char nofec;

static int32_t udpsock;

static struct complex_Complex iqbuf[8192];

static pCB fftbufs[13];

static float DDS[65536];

static float cfglevel;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void WrByte(char c)
{
   if ((uint8_t)c>=' ' && (uint8_t)c<'\177') osi_WrStr((char *) &c, 1u/1u);
   else {
      osi_WrStr("[", 2ul);
      osi_WrHex((uint32_t)(uint8_t)c, 1UL);
      osi_WrStr("]", 2ul);
   }
} /* end WrByte() */


static float ln0(float x)
{
   if (x<=0.0f) return 0.0f;
   return osic_ln(x);
} /* end ln0() */


static float freqmod(float d, int32_t max0)
{
   float m;
   float h;
   m = (float)max0;
   h = m*0.5f;
   if (d>h) return d-m;
   if (d<-h) return d+m;
   return d;
} /* end freqmod() */


static void MakeDDS(void)
{
   uint32_t i0;
   float r;
   r = 9.5873799240112E-5f;
   for (i0 = 0UL; i0<=65535UL; i0++) {
      DDS[i0] = osic_sin((float)i0*r);
   } /* end for */
} /* end MakeDDS() */

static uint32_t _cnst[10] = {64UL,48UL,32UL,24UL,16UL,12UL,8UL,4UL,2UL,1UL};

static float baud(uint32_t sf, uint32_t bwnum0)
{
   return X2C_DIVR(5.E+5f,(float)(_cnst[bwnum0]*(uint32_t)X2C_LSH(0x1UL,32,(int32_t)sf)));
} /* end baud() */

#define lorarx_DEFAULTIP 0x7F000001 

#define lorarx_PORTSEP ":"


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


static char StrToHex(const char s[], uint32_t s_len, uint32_t * n)
{
   uint32_t i0;
   char c;
   i0 = 0UL;
   *n = 0UL;
   while (i0<=s_len-1 && s[i0]) {
      *n =  *n*16UL;
      c = X2C_CAP(s[i0]);
      if ((uint8_t)c>='0' && (uint8_t)c<='9') *n += (uint32_t)(uint8_t)c-48UL;
      else if ((uint8_t)c>='A' && (uint8_t)c<='F') *n += (uint32_t)(uint8_t)c-55UL;
      else return 0;
      ++i0;
   }
   return 1;
} /* end StrToHex() */


static void newdem(void)
{
   pDEM dem;
   struct FFRAME * anonym;
   osic_alloc((char * *) &dem, sizeof(struct DEM));
   if (dem==0) Error("out of memory", 14ul);
   memset((char *)dem,(char)0,sizeof(struct DEM));
   dem->cfgopt = -1L;
   dem->coldet = 1UL;
   { /* with */
      struct FFRAME * anonym = &dem->frames[0U];
      anonym->cfgsf = 12UL;
      anonym->synfilter = 18UL;
      anonym->afcspeed = (-1.0f);
   }
   dem->next = dems;
   dems = dem;
} /* end newdem() */


static void Parms(void)
{
   char hasudp;
   char err;
   char h[1024];
   uint32_t i0;
   pDEM pd0;
   iqfn[0] = 0;
   isize = 1UL;
   verb = 0;
   err = 0;
   cfglevel = 0.0f;
   nofec = 0;
   hasudp = 0;
   udpsock = -1L;
   bwnum = 7UL;
   newdem();
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            osi_NextArg(iqfn, 1024ul);
            if (iqfn[0U]==0 || iqfn[0U]=='-') Error("-i <iqfilename>", 16ul);
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if ((h[0U]=='i' && h[1U]=='1') && h[2U]=='6') isize = 2UL;
            else if (h[0U]=='u' && h[1U]=='8') isize = 1UL;
            else if ((h[0U]=='f' && h[1U]=='3') && h[2U]=='2') isize = 4UL;
            else Error("-f u8|i16|f32", 14ul);
         }
         else if (h[1U]=='s') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &dems->frames[0U].cfgsf) || dems->frames[0U].cfgsf>12UL) || dems->frames[0U].cfgsf<6UL) {
               Error("-s <sf> 6..12", 14ul);
            }
         }
         else if (h[1U]=='a') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dems->frames[0U].afcspeed, h, 1024ul)) {
               Error("-a <afcspeed> 0.0..1.0 (0.2)", 29ul);
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &dems->frames[0U].cfgcr) || dems->frames[0U].cfgcr>8UL) || dems->frames[0U].cfgcr<4UL) {
               Error("-c <cr> 4..8", 13ul);
            }
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &bwnum) || bwnum>9UL) Error("-b <bw> 0..9", 13ul);
         }
         else if (h[1U]=='l') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &dems->frames[0U].cfgdatalen) || dems->frames[0U].cfgdatalen>255UL) || dems->frames[0U].cfgdatalen<2UL) {
               Error("-l <datalen> 2..255", 20ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&cfglevel, h, 1024ul)) Error("-g <dB>", 8ul);
         }
         else if (h[1U]=='S') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dems->frames[0U].synsquelch, h, 1024ul)) {
               Error("-S <level>", 11ul);
            }
         }
         else if (h[1U]=='X') {
            osi_NextArg(h, 1024ul);
            if (!StrToHex(h, 1024ul, &dems->frames[0U].synfilter)) Error("-X <netid>", 11ul);
         }
         else if (h[1U]=='O') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul, &dems->cfgopt) || dems->cfgopt<0L) || dems->cfgopt>1L) {
               Error("-o <onoff> 0..1 (-1)", 21ul);
            }
         }
         else if (h[1U]=='d') dems->coldet = 0UL;
         else if (h[1U]=='F') nofec = 1;
         else if (h[1U]=='C') dems->frames[0U].implicitcrc = 1;
         else if (h[1U]=='D') {
            dems->frames[0U].nodcdlost = 1;
         }
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='V') {
            verb = 1;
            verb2 = 1;
         }
         else if (h[1U]=='U' || h[1U]=='L') {
            if (hasudp) {
               newdem();
               hasudp = 0;
            }
            else hasudp = 1;
            dems->frames[0U].udp2 = h[1U]!='U'; /* switch on axudp2 */
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &dems->frames[0U].ipnum, &dems->frames[0U].udpport)<0L) {
               Error("-U or -L ip:port number", 24ul);
            }
            if (udpsock<0L) udpsock = openudp();
            if (udpsock<0L) Error("cannot open udp socket", 23ul);
         }
         else if (h[1U]=='u') {
            if (hasudp) {
               newdem();
               hasudp = 0;
            }
            else hasudp = 1;
            osi_NextArg(h, 1024ul);
            if (GetIp(h, 1024ul, &dems->frames[0U].ipnumraw, &dems->frames[0U].udprawport)<0L) {
               Error("-u ip:port number", 18ul);
            }
            if (udpsock<0L) udpsock = openudp();
            if (udpsock<0L) Error("cannot open udp socket", 23ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn(" Decode lora out of IQ-File/Pipe with exact samplerate=bandwidth", 65ul);
               osi_WrStrLn(" -a <afc-speed>     follow frequency drift, 0 off (0.2), on sf<11 (0)", 70ul);
               osi_WrStrLn(" -b <bandwidth>     kHz 0:7.8 1:10.4 2:15.6 3:20.8 4:31.25 5:41.7 6:62.5 7:125 8:250 9:500 (\
7)", 95ul);
               osi_WrStrLn(" -C                 use crc on implicit header", 47ul);
               osi_WrStrLn(" -c <cr>            coding rate and enable implicit header (4..8) (else from header)",
                85ul);
               osi_WrStrLn(" -d                 swith off collision detection (less cpu but loose stronger frames starti\
ng in weaker", 105ul);
               osi_WrStrLn(" -D                 if dcd lost go on decoding until frame length limit (for external FEC or\
 monitoring)", 105ul);
               osi_WrStrLn(" -F                 max. block energy FEC off (hamming code fec remains on) (on)", 81ul);
               osi_WrStrLn(" -f u8|i16|f32      IQ data format", 35ul);
               osi_WrStrLn(" -g <dB>            add to measured signal level (0.0)", 55ul);
               osi_WrStrLn(" -h                 this", 25ul);
               osi_WrStrLn(" -i <file>          IQ-filename or pipe", 40ul);
               osi_WrStrLn(" -L <x.x.x.x:destport>  as -U but AXUDPv2 with metadata for igate", 66ul);
               osi_WrStrLn("                      may be repeated with same -b but different -s  after -L", 78ul);
               osi_WrStrLn(" -l <len>           fixed datalen for implicit header else guess datalen from dcd", 82ul);
               osi_WrStrLn(" -O <0..1>          optimize on off else automatic on sf/bw (-1)", 65ul);
               osi_WrStrLn(" -s <sf>            spread factor (6..12) (12)", 47ul);
               osi_WrStrLn(" -S <level>         enable squelch for low SF (0.0)", 52ul);
               osi_WrStrLn(" -U <x.x.x.x:destport>  send frame in AXUDP", 44ul);
               osi_WrStrLn(" -u <x.x.x.x:destport>  send raw frame bytes in udp", 52ul);
               osi_WrStrLn(" -V                 very verbous", 33ul);
               osi_WrStrLn(" -v                 verbous +:hamming ok, -:error, h:corrected, ~:weakest chirp replaced",
                89ul);
               osi_WrStrLn("                      ^:bins with maximum power sum used until fitting hamming", 79ul);
               osi_WrStrLn(" -X <netid>         filter netwok-id (sync pattern), 1xx stops decode on wrong id (12)",
                87ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("example: sdrtst IQ output with FIR 125kHz: q 433.775 0 0 0 140000+125000,16", 76ul);
               osi_WrStrLn("         sdrtst -t 127.0.0.1:1234 -i 1024000 -c sdrcfg.txt -r 250000 -s /dev/stdout -k | ",
                90ul);
               osi_WrStrLn("         lorarx -i /dev/stdin -f i16 -b 7 -v -L 127.0.0.1:2300 -s 12 -L 127.0.0.1:2301 -s 10\
", 93ul);
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
   pd0 = dems;
   while (pd0) {
      if (pd0->cfgopt==0L) pd0->frames[0U].optimize = 0;
      else if (pd0->cfgopt==1L || baud(pd0->frames[0U].cfgsf, bwnum)<62.5f) pd0->frames[0U].optimize = 1;
      if (pd0->frames[0U].afcspeed<0.0f) {
         /* automatic afc on */
         if (pd0->frames[0U].cfgsf>=11UL) pd0->frames[0U].afcspeed = 0.2f;
         else pd0->frames[0U].afcspeed = 0.0f;
      }
      pd0->frames[0U].datasquelch = 1.5f+(float)pd0->frames[0U].cfgsf*0.8f;
      if (verb) {
         osi_WrStr("bw=", 4ul);
         osic_WrFixed(X2C_DIVR(5.E+5f,(float)_cnst[bwnum]), 1L, 1UL);
         osi_WrStr(" cr=", 5ul);
         if (pd0->frames[0U].cfgcr==0UL) osi_WrStr("from header", 12ul);
         else osic_WrINT32(pd0->frames[0U].cfgcr, 1UL);
         osi_WrStr(" sf=", 5ul);
         osic_WrINT32(pd0->frames[0U].cfgsf, 1UL);
         osi_WrStr(" optimize=", 11ul);
         osic_WrINT32((uint32_t)pd0->frames[0U].optimize, 1UL);
         osi_WrStr(" driftloop=", 12ul);
         osic_WrFixed(pd0->frames[0U].afcspeed, 3L, 1UL);
         osi_WrStrLn("", 1ul);
         if (pd0->frames[0U].cfgcr==0UL && pd0->frames[0U].cfgsf<7UL) {
            osi_WrStrLn("warning: sf<7 needs implizit header", 36ul);
         }
      }
      i0 = 1UL; /* copy parameters to all demodulators */
      pd0->frames[1U] = pd0->frames[0U];
      pd0->frames[1U].label = 1UL;
      pd0 = pd0->next;
   }
} /* end Parms() */


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


static void sendaxudp2(uint32_t ipnum, uint32_t udpport, char udp2, const char mon[], uint32_t mon_len,
                 int32_t txd, int32_t lev, int32_t snr, int32_t afc, int32_t qual)
{
   char data[501];
   char b[501];
   int32_t datalen;
   int32_t ret;
   uint32_t i0;
   uint32_t p;
   aprsstr_mon2raw(mon, mon_len, data, 501ul, &datalen);
   if (datalen>2L) {
      if (udp2) {
         datalen -= 2L; /* remove crc */
         b[0U] = '\001';
         b[1U] = '0';
         p = 2UL;
         app(&i0, &p, b, 'T', txd);
         app(&i0, &p, b, 'V', lev);
         app(&i0, &p, b, 'S', snr);
         app(&i0, &p, b, 'A', afc);
         app(&i0, &p, b, 'Q', qual);
         b[p] = 0; /* end of axudp2 header */
         ++p;
         i0 = 0UL;
         do {
            b[p] = data[i0];
            ++p;
            ++i0;
         } while ((int32_t)i0<datalen);
         aprsstr_AppCRC(b, 501ul, (int32_t)p);
      }
      if (udpport) ret = udpsend(udpsock, b, (int32_t)(p+2UL), udpport, ipnum);
   }
   else if (verb) osi_WrStrLn("beacon encode error", 20ul);
} /* end sendaxudp2() */


static void cleantext(char text[], uint32_t text_len, uint32_t len)
{
   uint32_t j;
   uint32_t i0;
   i0 = 0UL;
   while (((i0<len && i0<=4UL) && ((uint8_t)text[i0]<'A' || (uint8_t)text[i0]>'Z')) && ((uint8_t)
                text[i0]<'0' || (uint8_t)text[i0]>'9')) ++i0;
   if (i0<=3UL) {
      j = 0UL;
      while (j<text_len-1 && i0<len) {
         text[j] = text[i0];
         ++j;
         ++i0;
      }
      text[j] = 0;
      if (j>0UL && (uint8_t)text[j-1UL]<'\034') text[j-1UL] = 0;
   }
   else if (verb) osi_WrStrLn("too much junk in frame", 23ul);
} /* end cleantext() */


static void frameout(const struct FFRAME frame, const char finf[], uint32_t finf_len, char hascrc,
                char crc0, char dcdlost, char opt, uint32_t cr, char text[],
                uint32_t text_len)
{
   uint32_t td;
   uint32_t blocksize;
   uint32_t i0;
   int32_t ret;
   int32_t qual;
   int32_t drift;
   int32_t txd;
   int32_t snr;
   int32_t level;
   float n;
   float snrr;
   uint32_t tmp;
   X2C_PCOPY((void **)&text,text_len);
   if (frame.dlen>0UL && frame.dlen<text_len-1) {
      blocksize = (uint32_t)(1UL<<frame.cfgsf);
      snrr = 0.0f;
      n = 0.0f;
      if (frame.noissum!=0.0f) {
         /*      snrr:=ln0((frame.sigsum*2.1/frame.noissum-6.0)/FLOAT(blocksize))*(8.685889638*0.5); */
         snrr = ln0(X2C_DIVR(X2C_DIVR(frame.sigsum,frame.noissum)-1.0f,(float)blocksize))*4.342944819f;
      }
      snr = (int32_t)X2C_TRUNCI(snrr,X2C_min_longint,X2C_max_longint);
      if (snr>127L) snr = 127L;
      else if (snr<-127L) snr = -127L;
      qual = 100L;
      level = 0L;
      if (frame.cnt) {
         level = (int32_t)X2C_TRUNCI(ln0(X2C_DIVR(frame.sigsum,(float)frame.cnt))*4.342944819f+cfglevel,
                X2C_min_longint,X2C_max_longint);
         n = ln0(X2C_DIVR(frame.noissum,(float)frame.cnt))*4.342944819f+cfglevel;
         qual -= (int32_t)X2C_TRUNCI((X2C_DIVR(frame.eye,(float)frame.cnt))*200.0f,X2C_min_longint,
                X2C_max_longint);
      }
      i0 = 1UL;
      if (isize==1UL) i0 = 48UL;
      else if (isize==2UL) i0 = 96UL;
      level -= (int32_t)i0;
      n = n-(float)i0;
      if (level>127L) level = 127L;
      else if (level<-255L) level = -255L;
      if (qual<0L) qual = 0L;
      if (frame.cfgcr) td = 2UL;
      else td = 2UL+56UL/frame.cfgsf;
      txd = (int32_t)X2C_TRUNCI(X2C_DIVR((float)(frame.txdel+td)*1000.0f,baud(frame.cfgsf, bwnum)),X2C_min_longint,
                X2C_max_longint);
      drift = (int32_t)X2C_TRUNCI((frame.fc-frame.fcstart)*baud(frame.cfgsf, bwnum),X2C_min_longint,X2C_max_longint);
      if (opt) drift = drift*4L;
      if (verb) {
         osic_WrINT32(frame.cfgsf, 1UL);
         osi_WrStr(":", 2ul);
         osic_WrINT32(frame.label, 1UL);
         osi_WrStr(":", 2ul);
         osi_WrStr("id:", 4ul);
         osi_WrHex(frame.idfound, 0UL);
         osi_WrStr(" cr:", 5ul);
         osic_WrINT32(cr, 1UL);
         osi_WrStr(" len:", 6ul);
         osic_WrINT32(frame.dlen, 1UL);
         if (dcdlost) osi_WrStr(" dcd-lost", 10ul);
         osi_WrStr(" crc:", 6ul);
         if (hascrc) {
            if (crc0) osi_WrStr("ok", 3ul);
            else osi_WrStr("err", 4ul);
         }
         else osi_WrStr("no", 3ul);
         osi_WrStr(" lev:", 6ul);
         osic_WrINT32((uint32_t)level, 1UL);
         osi_WrStr("dB snr:", 8ul);
         osic_WrFixed(snrr, 1L, 1UL);
         osi_WrStr("dB nf:", 7ul);
         osic_WrFixed(n, 1L, 1UL);
         osi_WrStr("dB txd:", 8ul);
         osic_WrINT32((uint32_t)txd, 1UL);
         osi_WrStr(" q:", 4ul);
         osic_WrINT32((uint32_t)qual, 1UL);
         osi_WrStr("% afc:", 7ul);
         osic_WrINT32((uint32_t) -frame.df, 1UL);
         osi_WrStr("Hz", 3ul);
         if (drift) {
            osi_WrStr(" df:", 5ul);
            osic_WrINT32((uint32_t)drift, 1UL);
            osi_WrStr("Hz", 3ul);
         }
         osi_WrStrLn("", 1ul);
         osi_WrStr("[", 2ul);
         osi_WrStr(finf, finf_len);
         osi_WrStr("]", 2ul);
         tmp = frame.dlen-1UL;
         i0 = 0UL;
         if (i0<=tmp) for (;; i0++) {
            WrByte(text[i0]);
            if (i0==tmp) break;
         } /* end for */
         osi_WrStrLn("", 1ul);
      }
      if (frame.idfound==(frame.synfilter&255UL)) {
         if (frame.udprawport) {
            ret = udpsend(udpsock, text, (int32_t)frame.dlen, frame.udprawport, frame.ipnumraw);
         }
         if ((hascrc && crc0) && !dcdlost) {
            cleantext(text, text_len, frame.dlen);
            sendaxudp2(frame.ipnum, frame.udpport, frame.udp2, text, text_len, txd, level, snr, -frame.df, qual);
         }
      }
      else if (verb) {
         osi_WrStr("packet filtert as wrong sync word ", 35ul);
         osi_WrHex(frame.idfound, 0UL);
         osi_WrStrLn("", 1ul);
      }
   }
   X2C_PFREE(text);
/*  IF verb THEN WrStrLn("") END; */
} /* end frameout() */


static void deint(uint32_t sf, uint32_t cr, const uint16_t rb[], uint32_t rb_len, uint8_t hb[],
                uint32_t hb_len)
{
   uint32_t j;
   uint32_t i0;
   uint32_t tmp;
   uint32_t tmp0;
   memset((char *)hb,(char)0,hb_len);
   tmp = sf-1UL;
   j = 0UL;
   if (j<=tmp) for (;; j++) {
      tmp0 = cr-1UL;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         if (X2C_IN((sf-1UL)-j,16,rb[i0])) hb[(((sf-1UL)-j)+i0)%sf] |= (1U<<i0);
         if (i0==tmp0) break;
      } /* end for */
      if (j==tmp) break;
   } /* end for */
} /* end deint() */


static uint32_t crc(const char b[], uint32_t b_len, uint32_t from, uint32_t len, uint32_t ini)
{
   int32_t ic;
   int32_t i0;
   uint16_t poli;
   uint16_t crc0;
   uint8_t d;
   int32_t tmp;
   crc0 = (uint16_t)ini;
   poli = 0x1021U;
   tmp = (int32_t)len-1L;
   i0 = (int32_t)from;
   if (i0<=tmp) for (;; i0++) {
      d = (uint8_t)(uint8_t)b[i0];
      for (ic = 0L; ic<=7L; ic++) {
         if (((0x8000U & crc0)!=0)!=X2C_IN(7L-ic,8,d)) crc0 = X2C_LSH(crc0,16,1)^poli;
         else crc0 = X2C_LSH(crc0,16,1);
      } /* end for */
      if (i0==tmp) break;
   } /* end for */
   return (uint32_t)(uint16_t)crc0;
} /* end crc() */


static char headcrc(const uint8_t b[], uint32_t b_len)
{
   return (X2C_LSH(b[3UL],8,4)&0x10U|b[4UL]&0xFU)==((X2C_LSH(b[0UL],8,1)^X2C_LSH(b[0UL],8,2)^X2C_LSH(b[0UL],8,
                3)^X2C_LSH(b[0UL],8,4))&0x10U|(b[0UL]^b[1UL]^X2C_LSH(b[1UL],8,1)^X2C_LSH(b[1UL],8,2)^X2C_LSH(b[2UL],8,
                3))&0x8U|(b[0UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[1UL],8,2)^X2C_LSH(b[2UL],8,
                1))&0x4U|(b[0UL]^b[2UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[1UL]^b[2UL],8,
                1))&0x2U|(b[0UL]^b[2UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[2UL],8,-3)^X2C_LSH(b[2UL],8,-2))&0x1U);
} /* end headcrc() */

static uint8_t _cnst3[255] = {255U,254U,252U,248U,240U,225U,194U,133U,11U,23U,47U,94U,188U,120U,241U,227U,198U,141U,
                26U,52U,104U,208U,160U,64U,128U,1U,2U,4U,8U,17U,35U,71U,142U,28U,56U,113U,226U,196U,137U,18U,37U,75U,
                151U,46U,92U,184U,112U,224U,192U,129U,3U,6U,12U,25U,50U,100U,201U,146U,36U,73U,147U,38U,77U,155U,55U,
                110U,220U,185U,114U,228U,200U,144U,32U,65U,130U,5U,10U,21U,43U,86U,173U,91U,182U,109U,218U,181U,107U,
                214U,172U,89U,178U,101U,203U,150U,44U,88U,176U,97U,195U,135U,15U,31U,62U,125U,251U,246U,237U,219U,183U,
                111U,222U,189U,122U,245U,235U,215U,174U,93U,186U,116U,232U,209U,162U,68U,136U,16U,33U,67U,134U,13U,27U,
                54U,108U,216U,177U,99U,199U,143U,30U,60U,121U,243U,231U,206U,156U,57U,115U,230U,204U,152U,49U,98U,197U,
                139U,22U,45U,90U,180U,105U,210U,164U,72U,145U,34U,69U,138U,20U,41U,82U,165U,74U,149U,42U,84U,169U,83U,
                167U,78U,157U,59U,119U,238U,221U,187U,118U,236U,217U,179U,103U,207U,158U,61U,123U,247U,239U,223U,191U,
                126U,253U,250U,244U,233U,211U,166U,76U,153U,51U,102U,205U,154U,53U,106U,212U,168U,81U,163U,70U,140U,24U,
                48U,96U,193U,131U,7U,14U,29U,58U,117U,234U,213U,170U,85U,171U,87U,175U,95U,190U,124U,249U,242U,229U,
                202U,148U,40U,80U,161U,66U,132U,9U,19U,39U,79U,159U,63U,127U};
static uint8_t _cnst2[256] = {128U,192U,192U,3U,192U,5U,6U,135U,192U,9U,10U,139U,12U,141U,142U,15U,128U,1U,2U,199U,4U,
                199U,199U,135U,8U,137U,138U,11U,140U,13U,14U,199U,128U,1U,2U,131U,4U,133U,206U,7U,8U,137U,206U,11U,206U,
                13U,142U,206U,0U,201U,130U,3U,132U,5U,6U,135U,201U,137U,10U,201U,12U,201U,142U,15U,128U,1U,2U,203U,4U,
                133U,134U,7U,8U,203U,203U,139U,140U,13U,14U,203U,0U,129U,130U,3U,204U,5U,6U,135U,204U,9U,10U,139U,140U,
                204U,204U,15U,0U,197U,130U,3U,197U,133U,6U,197U,136U,9U,10U,139U,12U,197U,142U,15U,194U,1U,130U,194U,4U,
                133U,194U,7U,8U,137U,194U,11U,140U,13U,14U,143U,128U,1U,2U,131U,4U,205U,134U,7U,8U,205U,138U,11U,205U,
                141U,14U,205U,0U,129U,202U,3U,132U,5U,6U,135U,202U,9U,138U,202U,12U,141U,202U,15U,0U,195U,195U,131U,
                132U,5U,6U,195U,136U,9U,10U,195U,12U,141U,142U,15U,196U,1U,2U,131U,132U,196U,196U,7U,8U,137U,138U,11U,
                196U,13U,14U,143U,0U,129U,198U,3U,198U,5U,134U,198U,136U,9U,10U,139U,12U,141U,198U,15U,193U,129U,2U,
                193U,4U,193U,134U,7U,8U,193U,138U,11U,140U,13U,14U,143U,200U,1U,2U,131U,4U,133U,134U,7U,136U,200U,200U,
                11U,200U,13U,14U,143U,0U,129U,130U,3U,132U,5U,6U,207U,136U,9U,10U,207U,12U,207U,207U,143U};
static uint8_t _cnst1[128] = {128U,192U,192U,131U,192U,205U,134U,135U,192U,205U,138U,139U,205U,141U,142U,205U,128U,
                129U,202U,199U,132U,199U,199U,135U,202U,137U,138U,202U,140U,141U,202U,199U,128U,195U,195U,131U,132U,
                133U,206U,195U,136U,137U,206U,195U,206U,141U,142U,206U,196U,201U,130U,131U,132U,196U,196U,135U,201U,
                137U,138U,201U,196U,201U,142U,143U,128U,129U,198U,203U,198U,133U,134U,198U,136U,203U,203U,139U,140U,
                141U,198U,203U,193U,129U,130U,193U,204U,193U,134U,135U,204U,193U,138U,139U,140U,204U,204U,143U,200U,
                197U,130U,131U,197U,133U,134U,197U,136U,200U,200U,139U,200U,197U,142U,143U,194U,129U,130U,194U,132U,
                133U,194U,207U,136U,137U,194U,207U,140U,207U,207U,143U};
static uint8_t _cnst0[64] = {128U,1U,2U,3U,4U,5U,134U,7U,8U,9U,10U,139U,12U,141U,14U,15U,0U,129U,2U,3U,4U,5U,6U,135U,
                8U,9U,138U,11U,140U,13U,14U,15U,0U,1U,2U,131U,4U,133U,6U,7U,136U,9U,10U,11U,12U,13U,142U,15U,0U,1U,130U,
                3U,132U,5U,6U,7U,8U,137U,10U,11U,12U,13U,14U,143U};

static char decodechirp(struct FFRAME * frame, const BINS bins, char opti, char dcd)
{
   int32_t o;
   uint32_t datalen;
   uint32_t cr;
   uint32_t sf;
   uint32_t st;
   uint32_t try0;
   uint32_t fulllen;
   uint32_t j;
   uint32_t i0;
   int32_t mintry;
   int32_t maxtry;
   float lv;
   float v;
   float minsnr;
   float maxlev;
   char bt;
   uint16_t c;
   uint8_t s;
   uint8_t b;
   char explicit;
   char crcok;
   char hamcorr;
   char hamok;
   char ishead;
   uint8_t hn[12];
   char text[261];
   uint16_t chirps[8];
   char br[2];
   struct FFRAME * anonym;
   uint32_t tmp;
   { /* with */
      struct FFRAME * anonym = frame;
      sf = anonym->cfgsf;
      cr = anonym->cfgcr;
      datalen = anonym->cfgdatalen;
      explicit = cr==0UL;
      ishead = explicit && anonym->cnt<8UL; /* we are in header */
      opti = opti || ishead; /* header is always optimized */
      o = 0L;
      if (opti) o = 2L;
      sf -= (uint32_t)o;
      if (ishead) cr = 8UL;
      else if (explicit) cr = anonym->crfromhead;
      else anonym->crfromhead = cr;
      if (cr<4UL) cr = 4UL;
      else if (cr>8UL) cr = 8UL;
      if (anonym->cnt==0UL) {
         /* start new frame */
         anonym->chirpc = 0UL;
         anonym->nibbc = 0UL;
         anonym->dlen = datalen; /* for implicit header */
         anonym->withcrc = anonym->implicitcrc;
         anonym->fecinfo[0] = 0;
      }
      memcpy(anonym->bintab[anonym->chirpc],bins,sizeof(BINS));
      /*    c:=CAST(SET16, TRUNC(bins[0].freq)); */
      /*    chirps[chirpc]:=c/SHIFT(c, -1);                        (* grey *) */
      ++anonym->chirpc;
      if (dcd && anonym->chirpc>=cr) {
         /* block complete */
         anonym->chirpc = 0UL;
         /*-fec */
         try0 = 0UL;
         maxlev = 0.0f;
         maxtry = -1L;
         minsnr = X2C_max_real;
         mintry = -1L;
         st = 0UL;
         for (;;) {
            lv = 0.0f;
            tmp = cr-1UL;
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               if (try0/4UL==i0) j = try0&3UL;
               else j = 0UL;
               c = (uint16_t)(uint32_t)X2C_TRUNCC(anonym->bintab[i0][j].freq,0UL,X2C_max_longcard);
               chirps[i0] = c^X2C_LSH(c,16,-1); /* grey */
               if (try0==0UL) {
                  v = anonym->bintab[i0][4U].lev; /* noise */
                  if (v!=0.0f) {
                     v = X2C_DIVR(anonym->bintab[i0][0U].lev,v); /* snr of best bin */
                  }
                  if (v<minsnr) {
                     minsnr = v; /* weakest chirp */
                     mintry = (int32_t)i0;
                  }
               }
               else lv = anonym->bintab[i0][j].lev;
               if (i0==tmp) break;
            } /* end for */
            deint(sf, cr, chirps, 8ul, hn, 12ul); /* deint block */
            hamok = 1;
            hamcorr = 0;
            if (cr>=5UL) {
               tmp = sf-1UL;
               i0 = 0UL;
               if (i0<=tmp) for (;; i0++) {
                  /* repair bits */
                  s = hn[i0];
                  if (cr==5UL) {
                     if ((0x1U & (s^X2C_LSH(s,8,-1)^X2C_LSH(s,8,-2)^X2C_LSH(s,8,-3)^X2C_LSH(s,8,-4)))==0) {
                        hn[i0] |= 0x80U;
                     }
                  }
                  else {
                     b = (uint8_t)s;
                     if (cr==6UL) b = _cnst0[b];
                     else if (cr==7UL) b = _cnst1[b];
                     else b = _cnst2[b];
                     hn[i0] = (uint8_t)b;
                     if ((0x40U & hn[i0])) hamcorr = 1;
                  }
                  if ((0x80U & hn[i0])==0) {
                     hamok = 0;
                  }
                  if (i0==tmp) break;
               } /* end for */
            }
            if (st>0UL) {
               /* final try */
               if (st==2UL) {
                  /* repair weekest chirp */
                  tmp = sf-1UL;
                  i0 = 0UL;
                  if (i0<=tmp) for (;; i0++) {
                     /* invert wrong bits */
                     bt = X2C_IN(mintry,8,hn[i0]);
                     hn[i0] &= ~(1U<<mintry);
                     if (bt==((0x80U & hn[i0])!=0)) hn[i0] |= (1U<<mintry);
                     if (i0==tmp) break;
                  } /* end for */
               }
               break;
            }
            if (hamok || nofec) {
               /* checkbits ok */
               if (try0==0UL) break;
               if (lv>maxlev) {
                  /* best bin with check ok */
                  maxlev = lv;
                  maxtry = (int32_t)try0;
               }
            }
            ++try0;
            if (try0>=4UL*cr) {
               /* all combinations tested */
               try0 = 0UL;
               st = 1UL;
               if (maxtry<0L) {
                  /* no valid checkbits found */
                  if (mintry>=0L) {
                     try0 = 0UL; /* so modify weekest snr to fitting check */
                     st = 2UL;
                  }
               }
               else try0 = (uint32_t)maxtry;
            }
         }
         if (st==2UL) aprsstr_Append(anonym->fecinfo, 96ul, "~", 2ul);
         else if (hamok) {
            if (st==1UL) aprsstr_Append(anonym->fecinfo, 96ul, "^", 2ul);
            else if (hamcorr) aprsstr_Append(anonym->fecinfo, 96ul, "h", 2ul);
            else aprsstr_Append(anonym->fecinfo, 96ul, "+", 2ul);
         }
         else aprsstr_Append(anonym->fecinfo, 96ul, "-", 2ul);
         /*-fec */
         if (verb2) {
            /*
              FOR j:=0 TO 7 DO
                FOR i:=0 TO sf-1 DO WrInt(ORD((7-j) IN hn[i]),1); END;
                WrStrLn("");
              END;
              WrStrLn("");
            */
            /*WrInt(debugc,1);WrStr(" "); */
            if (isize==1UL) v = 8.0f;
            else if (isize==2UL) v = 0.03125f;
            else v = 1000.0f;
            tmp = cr-1UL;
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               for (j = 0UL; j<=4UL; j++) {
                  strncpy(br,"()",2u);
                  if (st==1UL) {
                     /* best bin sum */
                     if (try0/4UL!=i0 && j==0UL || try0/4UL==i0 && (try0&3UL)==j) {
                        strncpy(br,"[]",2u);
                     }
                  }
                  else if (st==2UL) {
                     /* weekest bin */
                     if (mintry!=(int32_t)i0 && j==0UL) strncpy(br,"[]",2u);
                  }
                  else if (j==0UL) strncpy(br,"[]",2u);
                  osic_WrFixed(osic_sqrt(anonym->bintab[i0][j].lev)*v, 1L, 5UL);
                  if (j<4UL) {
                     osi_WrStr((char *) &br[0U], 1u/1u);
                     osic_WrFixed(anonym->bintab[i0][j].freq, 2L, 7UL);
                     osi_WrStr((char *) &br[1U], 1u/1u);
                  }
               } /* end for */
               /*   IF j<HIGH(bins)-1 THEN WrStr(",") ELSIF j<HIGH(bins) THEN WrStr("/") END;  */
               osi_WrStrLn("", 1ul);
               if (i0==tmp) break;
            } /* end for */
         }
         if (verb2) osi_WrStrLn("", 1ul);
         if (ishead) {
            /* head block */
            if (headcrc(hn, 12ul)) {
               anonym->dlen = (uint32_t)(uint8_t)(X2C_LSH(hn[0U]&0xFU,8,4)|hn[1U]&0xFU);
               anonym->withcrc = (0x1U & hn[2U])!=0;
               anonym->crfromhead = (uint32_t)(4U+(uint8_t)(X2C_LSH(hn[2U],8,-1)&0x7U));
               if (anonym->crfromhead>8UL) anonym->crfromhead = 8UL;
               tmp = sf-1UL;
               i0 = 5UL;
               if (i0<=tmp) for (;; i0++) {
                  /* 5 header nibbles copy rest to data */
                  anonym->nibbs[anonym->nibbc] = hn[i0]&0xFU;
                  ++anonym->nibbc;
                  if (i0==tmp) break;
               } /* end for */
            }
            else {
               /*IF verb THEN WrStr(" dlen="); WrInt(dlen, 1); WrStr(" cr="); WrInt(crfromhead, 1); WrStr(" crc=");
                WrInt(ORD(withcrc), 1); WrStrLn(""); END; */
               if (verb) osi_WrStrLn("head crc error", 15ul);
               anonym->dlen = 0UL; /* show metadata and stop frame soon */
               anonym->withcrc = 0;
            }
         }
         else {
            /* data block */
            tmp = sf-1UL;
            i0 = 0UL;
            if (i0<=tmp) for (;; i0++) {
               anonym->nibbs[anonym->nibbc] = hn[i0]&0xFU;
               if (anonym->nibbc<522UL) ++anonym->nibbc;
               if (i0==tmp) break;
            } /* end for */
         }
      }
      if (anonym->chirpc==0UL || !dcd) {
         if (explicit) {
            /* len from header */
            if (datalen && anonym->dlen>datalen) anonym->dlen = datalen;
            fulllen = anonym->dlen;
            if (anonym->withcrc) fulllen += 2UL;
         }
         else if (datalen==0UL) {
            /* len from dcd */
            anonym->dlen = anonym->nibbc/2UL;
            if (dcd) fulllen = 255UL;
            else fulllen = anonym->dlen;
            if (anonym->withcrc && anonym->dlen>=2UL) anonym->dlen -= 2UL;
         }
         else {
            /* fixed len */
            anonym->dlen = datalen;
            fulllen = datalen;
            if (anonym->withcrc) fulllen += 2UL;
         }
         if (!dcd || anonym->nibbc>=fulllen*2UL) {
            /* frame complete or implicit variable len dcd out */
            if (anonym->nibbc>0UL) {
               tmp = anonym->nibbc/2UL-1UL;
               i0 = 0UL;
               if (i0<=tmp) for (;; i0++) {
                  s = anonym->nibbs[i0*2UL]&0xFU|X2C_LSH(anonym->nibbs[i0*2UL+1UL],8,4)&0xF0U;
                  if (i0<anonym->dlen) {
                     s = s^(uint8_t)_cnst3[i0];
                  }
                  text[i0] = (char)s;
                  if (i0==tmp) break;
               } /* end for */
               if (anonym->dlen>anonym->nibbc/2UL) {
                  anonym->dlen = anonym->nibbc/2UL; /* remove dcd lost junk */
               }
               crcok = 0;
               if (anonym->withcrc) {
                  if (anonym->dlen>2UL) {
                     i0 = crc(text, 261ul, 0UL, anonym->dlen-2UL, 0UL);
                  }
                  if (anonym->dlen>1UL) {
                     i0 = (uint32_t)((uint16_t)i0^(uint16_t)((uint32_t)(uint8_t)text[anonym->dlen-2UL]*256UL))
                ;
                  }
                  if (anonym->dlen>0UL) {
                     i0 = (uint32_t)((uint16_t)i0^(uint16_t)(uint32_t)(uint8_t)text[anonym->dlen-1UL]);
                  }
                  crcok = (char)(i0&255UL)==text[anonym->dlen] && (char)(i0/256UL)==text[anonym->dlen+1UL];
               }
               frameout(*frame, anonym->fecinfo, 96ul, anonym->withcrc, crcok, !dcd && datalen==0UL, opti, cr, text,
                261ul);
            }
            return 1;
         }
      }
   }
   return 0;
} /* end decodechirp() */


static void getbin(struct complex_Complex c[], uint32_t c_len, struct BIN bins[], uint32_t bins_len, uint32_t sf,
                float offset, char rev, char opt, char sort)
{
   uint32_t imax;
   uint32_t lastbin;
   uint32_t j;
   uint32_t i0;
   uint16_t wgi;
   int32_t ii;
   float v;
   float nois;
   float co;
   float si;
   float max0;
   float vf[3];
   struct complex_Complex * anonym;
   /* find strongest bin(s) */
   struct complex_Complex * anonym0;
   uint32_t tmp;
   uint32_t tmp0;
   ii = (int32_t)(65536UL/sf);
   if (rev) ii = -ii;
   j = (uint32_t)(32768L-(int32_t)X2C_TRUNCI(offset*(float)ii,X2C_min_longint,X2C_max_longint));
   wgi = 0U;
   tmp = sf-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      /* dechirp */
      si = DDS[wgi];
      co = DDS[(uint16_t)(wgi+16384U)];
      { /* with */
         struct complex_Complex * anonym = &c[i0];
         v = anonym->Im*co-anonym->Re*si;
         anonym->Im = anonym->Re*co+anonym->Im*si;
         anonym->Re = v;
      }
      wgi += (uint16_t)j;
      j += (uint32_t)ii;
      if (i0==tmp) break;
   } /* end for */
   fft_Transform(c, c_len, 0);
   c[0UL].Re = c[0UL].Re*2.0f;
   c[0UL].Im = c[0UL].Im*2.0f;
   lastbin = 0UL;
   memset((char *)bins,(char)0,bins_len*sizeof(struct BIN));
   max0 = 0.0f;
   nois = 0.0f;
   tmp = sf-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      { /* with */
         struct complex_Complex * anonym0 = &c[i0];
         anonym0->Re = anonym0->Re*anonym0->Re+anonym0->Im*anonym0->Im;
         nois = nois+anonym0->Re;
         if (anonym0->Re>max0) {
            max0 = anonym0->Re;
            imax = i0;
         }
         if (sort) {
            if (max0>bins[lastbin].lev && (!opt || (i0&3UL)==3UL)) {
               /* if optimize combine 4 bins */
               bins[lastbin].lev = max0;
               bins[lastbin].bn = imax;
               v = X2C_max_real;
               tmp0 = (bins_len-1)-1UL;
               j = 0UL;
               if (j<=tmp0) for (;; j++) {
                  /* find new weekest bin */
                  if (bins[j].lev<v) {
                     v = bins[j].lev;
                     lastbin = j;
                  }
                  if (j==tmp0) break;
               } /* end for */
               max0 = bins[lastbin].lev;
            }
         }
      }
      if (i0==tmp) break;
   } /* end for */
   if (sort) {
      j = 0UL;
      tmp = (bins_len-1)-1UL;
      i0 = 1UL;
      if (i0<=tmp) for (;; i0++) {
         if (bins[i0].lev>bins[j].lev) j = i0;
         if (i0==tmp) break;
      } /* end for */
      if (j>0UL) {
         /* swap best to front */
         bins[bins_len-1] = bins[0UL];
         bins[0UL] = bins[j];
         bins[j] = bins[bins_len-1];
      }
   }
   else {
      bins[0UL].lev = max0;
      bins[0UL].bn = imax;
   }
   /*FOR i:=0 TO HIGH(bins)-1 DO WrFixed(sqrt(bins[i].lev),1,1); WrStr("/"); END; */
   for (i0 = 0UL; i0<=2UL; i0++) {
      v = c[(((bins[0UL].bn+sf)-1UL)+i0)%sf].Re;
      nois = nois-v; /* exclude carrier bins from noise */
      vf[i0] = v;
   } /* end for */
   bins[bins_len-1].lev = X2C_DIVR(nois,(float)sf); /* last bin is for noisefloor */
   v = 0.0f;
   if (vf[1U]!=0.0f) v = X2C_DIVR((vf[2U]-vf[0U])*0.5f,vf[1U]);
   /*  IF vf[1]<>0.0 THEN v:=(sqrt(vf[2])-sqrt(vf[0]))*0.5/sqrt(vf[1]) END; */
   if (v>0.49f) v = 0.49f;
   else if (v<(-0.49f)) v = (-0.49f);
   tmp = (bins_len-1)-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      v = v+(float)bins[i0].bn;
      if (opt) v = v*0.25f;
      bins[i0].freq = v+0.5f;
      v = 0.0f;
      if (i0==tmp) break;
   } /* end for */
/*
IF rev THEN WrStr(" rev ") END;
IF opt THEN WrStr(" opt ") END;
IF sort THEN WrStr(" sort ") END;

WrInt(bins[0].bn,1); WrStr(" "); WrFixed(sqrt(bins[0].lev),1,1); WrStr(" ");
WrFixed(sqrt(bins[HIGH(bins)].lev),1,1); WrStr(" ");
WrFixed(bins[0].freq,2,1); WrStrLn("=i v n f");
*/
} /* end getbin() */

union _0;


union _0 {
   struct complex_Complex c[2048];
   short i0[8192];
   uint8_t b[16384];
};


static char readsamps(void)
{
   uint32_t rs;
   uint32_t bs;
   uint32_t i0;
   int32_t res;
   union _0 ib;
   char * p;
   uint32_t tmp;
   bs = isize*4096UL;
   rs = 0UL;
   do {
      p = (char *) &ib.b[rs];
      res = osi_RdBin(iqfd, p, 65536ul, bs-rs);
      if (res<=0L) return 0;
      rs += (uint32_t)res;
   } while (!(rs>=bs || (rs&7UL)==0UL));
   if (isize==1UL) {
      tmp = rs-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (tmp = (uint32_t)(tmp-i0)/2UL;;) {
         iqbuf[iqwrite].Re = (float)((int32_t)ib.b[i0]-127L);
         iqbuf[iqwrite].Im = (float)((int32_t)ib.b[i0+1UL]-127L);
         iqwrite = iqwrite+1UL&8191UL;
         if (!tmp) break;
         --tmp;
         i0 += 2UL;
      } /* end for */
   }
   else if (isize==2UL) {
      tmp = rs/2UL-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (tmp = (uint32_t)(tmp-i0)/2UL;;) {
         iqbuf[iqwrite].Re = (float)ib.i0[i0];
         iqbuf[iqwrite].Im = (float)ib.i0[i0+1UL];
         iqwrite = iqwrite+1UL&8191UL;
         if (!tmp) break;
         --tmp;
         i0 += 2UL;
      } /* end for */
   }
   else {
      tmp = rs/8UL-1UL;
      i0 = 0UL;
      if (i0<=tmp) for (;; i0++) {
         iqbuf[iqwrite] = ib.c[i0];
         iqwrite = iqwrite+1UL&8191UL;
         if (i0==tmp) break;
      } /* end for */
   }
   return 1;
} /* end readsamps() */

#define lorarx_IQSIZE 8


static char getsamps(struct complex_Complex c[], uint32_t c_len, uint32_t * iqread, uint32_t jump)
{
   uint32_t i0;
   uint32_t newsamps;
   newsamps = (iqwrite+8192UL)-*iqread&8191UL;
   if (newsamps<jump) return 0;
   *iqread = (*iqread+jump+8192UL)-((c_len-1)+1UL)&8191UL;
   if (*iqread+(c_len-1)+1UL>8192UL) {
      /* wraps around */
      i0 = 8192UL-*iqread;
      X2C_MOVE((char *) &iqbuf[*iqread],(char *)c,i0*8UL);
      if (i0<=c_len-1) X2C_MOVE((char *)iqbuf,(char *) &c[i0],(((c_len-1)+1UL)-i0)*8UL);
   }
   else X2C_MOVE((char *) &iqbuf[*iqread],(char *)c,((c_len-1)+1UL)*8UL);
   *iqread = *iqread+(c_len-1)+1UL&8191UL;
   return 1;
} /* end getsamps() */


static float squelch(const struct BIN bins[], uint32_t bins_len)
{
   return X2C_DIVR(bins[0UL].lev,bins[bins_len-1].lev+0.01f);
} /* end squelch() */


static void nextchirp(struct FFRAME * frame)
{
   BINS bins;
   uint32_t blocksize;
   uint32_t i0;
   int32_t ii;
   float sq;
   float fi;
   char opt;
   pCB anonym;
   pCB anonym0;
   blocksize = (uint32_t)(1UL<<frame->cfgsf);
   for (;;) {
      if (frame->state==lorarx_sSLEEP) {
         frame->jp = blocksize;
         frame->cnt = 0UL;
         frame->iqread = iqwrite; /* start decode not from old data */
         frame->state = lorarx_sHUNT;
         frame->lastsq = 0.0f;
      }
      if (!(anonym = fftbufs[frame->cfgsf],getsamps(anonym->Adr, anonym->Len0, &frame->iqread, frame->jp))) {
         break;
      }
      frame->jp = blocksize;
      opt = frame->state==lorarx_sDATA && (frame->optimize || frame->cfgcr==0UL && frame->cnt<8UL);
      anonym0 = fftbufs[frame->cfgsf];
      getbin(anonym0->Adr, anonym0->Len0, bins, 5ul, blocksize, frame->fc,
                frame->state==lorarx_sREV1 || frame->state==lorarx_sREV2, opt, frame->state==lorarx_sDATA);
      if (frame->state==lorarx_sHUNT) {
         if ((frame->lastbin>=0.0f && frame->lastsq+squelch(bins,
                5ul)>frame->synsquelch) && (float)fabs(freqmod(frame->lastbin-bins[0U].freq,
                (int32_t)blocksize))<=1.0f) {
            frame->jp = blocksize-(uint32_t)X2C_TRUNCC(bins[0U].freq+0.5f,0UL,X2C_max_longcard);
            frame->state = lorarx_sSYNRAW;
            frame->synmed = 0.0f;
            frame->cnt = 0UL;
            if (verb2) {
               osic_WrINT32(frame->jp, 1UL);
               osi_WrStrLn("=jump", 6ul);
            }
         }
         else frame->lastbin = bins[0U].freq;
         frame->lastsq = squelch(bins, 5ul);
         frame->fc = 0.0f;
         frame->fci = 0.0f;
      }
      else if (frame->state==lorarx_sSYNRAW) {
         fi = freqmod(bins[0U].freq, (int32_t)blocksize);
         if (fi>=4.0f || fi<=(-4.0f)) {
            frame->state = lorarx_sID;
            frame->idfound = (uint32_t)X2C_TRUNCC((4.0f+bins[0U].freq)*0.125f,0UL,X2C_max_longcard);
            fi = (float)(frame->idfound*8UL); /* use sync pattern for median zero freq */
            if (frame->idfound>15UL || frame->synfilter>=256UL && frame->idfound!=(frame->synfilter/16UL&15UL)) {
               frame->state = lorarx_sHUNT;
               if (verb2) osi_WrStrLn("syn1 timeout", 13ul);
            }
         }
         else fi = 0.0f;
         frame->synmed = (frame->synmed+freqmod(bins[0U].freq, (int32_t)blocksize))-fi;
         ++frame->cnt;
      }
      else if (frame->state==lorarx_sID) {
         frame->state = lorarx_sREV1;
         i0 = (uint32_t)X2C_TRUNCC((4.0f+bins[0U].freq)*0.125f,0UL,X2C_max_longcard);
         fi = (float)(i0*8UL); /* use sync pattern for median zero freq */
         frame->idfound = frame->idfound*16UL+i0;
         if (i0>255UL || frame->synfilter>255UL && frame->idfound!=(frame->synfilter&255UL)) {
            frame->state = lorarx_sHUNT;
            if (verb2) osi_WrStrLn("syn2 timeout", 13ul);
         }
         frame->synmed = (frame->synmed+freqmod(bins[0U].freq, (int32_t)blocksize))-fi;
         ++frame->cnt;
         frame->synmed = X2C_DIVR(frame->synmed,(float)frame->cnt);
      }
      else if (frame->state==lorarx_sREV1) {
         frame->state = lorarx_sREV2;
         frame->lastrev = freqmod(bins[0U].freq, (int32_t)blocksize);
      }
      else if (frame->state==lorarx_sREV2) {
         /*    frame.lastsq:=squelch(bins); */
         if (frame->lastsq+squelch(bins, 5ul)>frame->synsquelch) {
            /* 2 usable reverse chirps */
            fi = (frame->lastrev+freqmod(bins[0U].freq, (int32_t)blocksize))*0.25f;
            frame->df = (int32_t)X2C_TRUNCI(fi*baud(frame->cfgsf, bwnum),X2C_min_longint,X2C_max_longint);
            ii = (int32_t)X2C_TRUNCI(fi,X2C_min_longint,X2C_max_longint);
            frame->jp = (uint32_t)((int32_t)(blocksize+(blocksize>>2))+ii);
            if (verb2) {
               osic_WrINT32(frame->jp, 1UL);
               osi_WrStrLn("=jump", 6ul);
            }
            frame->fc = ((float) -ii-0.5f)-frame->synmed; /* new zero freq after sample jump */
            frame->fci = 0.0f;
            frame->fcstart = frame->fc; /* store for show drift */
            frame->state = lorarx_sDATA;
            frame->sigsum = 0.0f;
            frame->noissum = 0.0f;
            frame->txdel = frame->cnt; /* store for show preamble time */
            frame->cnt = 0UL;
            frame->eye = 0.0f;
            frame->timeout = 0UL;
         }
         else {
            frame->state = lorarx_sHUNT;
            if (verb2) osi_WrStrLn("rev chirp timeout", 18ul);
         }
      }
      else {
         /* data */
         sq = squelch(bins, 5ul);
         if (frame->nodcdlost || sq>frame->datasquelch) {
            fi = (bins[0U].freq-(float)(int32_t)X2C_TRUNCI(bins[0U].freq,X2C_min_longint,X2C_max_longint))-0.5f;
            frame->eye = frame->eye+(float)fabs(fi); /* for statistics */
            if (sq<12.0f) fi = 0.0f;
            frame->fc = (frame->fc-frame->fci)-fi*frame->afcspeed; /* afc lead-lag loop filter */
            frame->fci = frame->fci+(fi-frame->fci)*frame->afcspeed;
            frame->sigsum = frame->sigsum+bins[0U].lev;
            frame->noissum = frame->noissum+bins[4U].lev;
            frame->timeout = 0UL;
            frame->lastbin = bins[0U].freq;
         }
         else {
            if (verb2) osi_WrStrLn("no dcd", 7ul);
            ++frame->timeout;
            if (frame->timeout>(uint32_t)(frame->cfgdatalen || frame->cfgcr==0UL)*2UL+3UL) {
               /* skip over more chirps if datalen known */
               frame->state = lorarx_sHUNT;
               frame->fc = 0.0f;
               frame->fci = 0.0f;
               if (verb2) osi_WrStrLn("data timeout", 13ul);
            }
         }
         if (decodechirp(frame, bins, opt, frame->state==lorarx_sDATA)) frame->state = lorarx_sSLEEP;
         ++frame->cnt;
      }
   }
} /* end nextchirp() */

static uint32_t i;

static char hunt;

static pDEM pd;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   uint32_t tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   complex_BEGIN();
   fft_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   Parms();
   MakeDDS();
   for (i = 6UL; i<=12UL; i++) {
      X2C_DYNALLOCATE((char **) &fftbufs[i],sizeof(struct complex_Complex),(tmp[0] = (uint32_t)(1UL<<i),tmp),1u);
      if (fftbufs[i]==0) Error("out of memory", 14ul);
   } /* end for */
   iqfd = osi_OpenRead(iqfn, 1024ul);
   if (iqfd<0L) Error("open iq file", 13ul);
   for (;;) {
      pd = dems;
      while (pd) {
         hunt = 1;
         tmp0 = pd->coldet;
         i = 0UL;
         if (i<=tmp0) for (;; i++) {
            if (pd->frames[i].state>lorarx_sSYNRAW) nextchirp(&pd->frames[i]);
            else if (hunt) {
               nextchirp(&pd->frames[i]);
               hunt = 0;
            }
            if (i==tmp0) break;
         } /* end for */
         pd = pd->next;
      }
      if (!readsamps()) break;
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
