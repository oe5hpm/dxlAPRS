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
#define radio_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef sdr_H_
#include "sdr.h"
#endif

/* fm stereo radio with rtl_tcp iq by OE5DXL */
struct STICKPARM;


struct STICKPARM {
   uint32_t val;
   char ok0;
};

#define radio_OFFSET 100
/* khz from center frequency to avoid adc dc error area */

#define radio_DEEMPHASIS 0.05
/* check value ... */

static int32_t fd;

static uint32_t sndw;

static short sndbuf[1024];

static struct sdr_RX rxx;

static sdr_pRX prx[17];

static short sampx[64];

static char url[1001];

static char port[1001];

static char soundfn[1001];

static char mono;

static char verb;

static char isstereo;

static uint32_t afc;

static uint32_t tune;

static struct STICKPARM stickparm[256];


static void Usage(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(" usage: ", 9ul);
   osi_WrStrLn(text, text_len);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Usage() */


static void fix(const char s[], uint32_t s_len, uint32_t * p,
                float * x, char * ok0)
{
   float m;
   m = 1.0f;
   *ok0 = 0;
   *x = 0.0f;
   while ((uint8_t)s[*p]>='0' && (uint8_t)s[*p]<='9' || s[*p]=='.') {
      if (s[*p]=='.') m = 0.1f;
      else if (m==1.0f) {
         *x =  *x*10.0f+(float)((uint32_t)(uint8_t)s[*p]-48UL);
      }
      else {
         *x = *x+(float)((uint32_t)(uint8_t)s[*p]-48UL)*m;
         m = m*0.1f;
      }
      *ok0 = 1;
      ++*p;
   }
} /* end fix() */


static void Parms(void)
{
   char s[1001];
   float fr;
   uint32_t n;
   uint32_t m;
   int32_t ni;
   char ok0;
   mono = 0;
   verb = 0;
   afc = 50UL;
   strncpy(url,"127.0.0.1",1001u);
   strncpy(port,"1234",1001u);
   soundfn[0] = 0;
   tune = 100000000UL;
   for (;;) {
      osi_NextArg(s, 1001ul);
      if (s[0U]==0) break;
      if ((s[0U]=='-' && s[1U]) && s[2U]==0) {
         if (s[1U]=='m') mono = 1;
         else if (s[1U]=='s') osi_NextArg(soundfn, 1001ul);
         else if (s[1U]=='t') {
            osi_NextArg(s, 1001ul); /* url */
            n = 0UL;
            while ((n<1000UL && s[n]) && s[n]!=':') {
               if (n<1000UL) url[n] = s[n];
               ++n;
            }
            if (n>1000UL) n = 1000UL;
            url[n] = 0;
            if (s[n]==':') {
               m = 0UL;
               ++n;
               while ((n<1000UL && s[n]) && m<1000UL) {
                  port[m] = s[n];
                  ++n;
                  ++m;
               }
               if (m>1000UL) m = 1000UL;
               port[m] = 0;
            }
         }
         else if (s[1U]=='f') {
            /* MHz */
            osi_NextArg(s, 1001ul);
            n = 0UL;
            fix(s, 1001ul, &n, &fr, &ok0);
            if (ok0) {
               if (fr>2000.0f && fr<=4.294967295E+9f) {
                  tune = (uint32_t)X2C_TRUNCC(fr+0.5f,0UL,
                X2C_max_longcard);
               }
               else if (fr>1.0f && fr<=4294.0f) {
                  tune = (uint32_t)X2C_TRUNCC(fr*1.E+6f+0.5f,0UL,
                X2C_max_longcard);
               }
               else Usage(" -f <mhz> or -f <hz>", 21ul);
            }
            else Usage(" -f <mhz> or -f <hz>", 21ul);
         }
         else if (s[1U]=='p') {
            osi_NextArg(s, 1001ul);
            if (aprsstr_StrToCard(s, 1001ul, &m) && m<256UL) {
               osi_NextArg(s, 1001ul);
               if (aprsstr_StrToInt(s, 1001ul, &ni)) {
                  stickparm[m].val = (uint32_t)ni; /* stick parameter */
                  stickparm[m].ok0 = 1;
               }
               else Usage(" -p <cmd> <value>", 18ul);
            }
            else Usage(" -p <cmd> <value>", 18ul);
         }
         else if (s[1U]=='a') {
            /* afc */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &afc) || afc>200UL) {
               Usage(" -a <khz>", 10ul);
            }
         }
         else if (s[1U]=='v') verb = 1;
         else if (s[1U]=='h') {
            osi_WrStrLn("Stereo UKW Radio from rtl:tcp (8 bit IQ via tcpip to\
 2 channel 16 bit pcm file/pipe ", 85ul);
            osi_WrStrLn(" -a <khz>       max afc <kHz> (50) or 0 for off",
                48ul);
            osi_WrStrLn(" -f <freq>      tune to MHz or Hz", 34ul);
            osi_WrStrLn(" -h             help", 21ul);
            osi_WrStrLn(" -p <cmd> <value> send rtl_tcp parameter, ppm, tuner\
gain ...", 61ul);
            osi_WrStrLn(" -s <soundfilename>  16bit 48kHz signed 2 channel so\
und stream/pipe", 68ul);
            osi_WrStrLn(" -t <url:port>  connect rtl:tcp server (127.0.0.1:12\
34)", 56ul);
            osi_WrStrLn(" -v             show rssi (dB) and afc (khz)",
                45ul);
            osi_WrStrLn("example: radio -f 101.2 -s /tmp/sound.pcm -t 192.168\
.1.1:1234 -p 5 72 -p 8 1 -v", 80ul);
            X2C_ABORT();
         }
         else Usage("-h", 3ul);
      }
      else Usage("-h", 3ul);
   }
} /* end Parms() */


static void showrssi(void)
{
   char ss[31];
   char s[31];
   if (isstereo) strncpy(s,"S ",31u);
   else strncpy(s,"M ",31u);
   aprsstr_FixToStr(osic_ln((rxx.rssi+1.0f)*3.0517578125E-5f)*4.342944819f,
                2UL, ss, 31ul);
   aprsstr_Append(s, 31ul, ss, 31ul);
   aprsstr_Append(s, 31ul, "dB ", 4ul);
   aprsstr_IntToStr(rxx.afckhz, 0UL, ss, 31ul);
   aprsstr_Append(s, 31ul, ss, 31ul);
   aprsstr_Append(s, 31ul, "     \015", 7ul);
   osi_Werr(s, 31ul);
} /* end showrssi() */

static float uc19;

static float il19;

static float uc20;

static float il20;

static float um20;

static float sqv;

#define radio_F19 0.3741
/* 19khz with 192khz sampelrate */

#define radio_LRLEVEL 1.633E-3
/* check value ... */

#define radio_STEREOSQUELCH 1.37172


static void stereo(float ui, float * left0, float * right0)
{
   float uq;
   uc19 = uc19+(ui-uc19)*0.001f+il19; /* 19khz resonant filter */
   il19 = il19-uc19*0.3741f;
   uc20 = uc20*0.999f+il20; /* 19khz resonant filter */
   if (uc19>=0.0f) uc20 = uc20+1.633E-3f;
   else uc20 = uc20-1.633E-3f;
   il20 = il20-uc20*0.3741f;
   sqv = sqv+((float)fabs(uc20)-sqv)*0.0001f; /* level of pilot tone */
   if (sqv>1.37172f) {
      uq = uc20*uc20; /* make 38khz sine + dc out of 19khz*/
      um20 = um20+(uq-um20)*0.01f; /* get dc */
      uq = (uq-um20)*ui; /* subtract dc and mix left-right signal with 38khz */
      isstereo = 1;
   }
   else {
      uq = 0.0f; /* mono */
      isstereo = 0;
   }
   *left0 = ui-uq; /* mono - (left - right) */
   *right0 = ui+uq; /* mono + (left - right) */
} /* end stereo() */


static short lim(float v)
{
   if (v>=32767.0f) return 32767;
   else if (v<(-3.2767E+4f)) return -32767;
   else return (short)X2C_TRUNCI(v,-32768,32767);
   return 0;
} /* end lim() */

static int32_t sp;

static int32_t sn;

static uint32_t pc;

static float left;

static float right;

static uint32_t tshow;

static char recon;

static float lp;

static float u11;

static float u21;

static float il1;

static float u12;

static float u22;

static float il2;

static float deem1;

static float deem2;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   int32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   sdr_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   memset((char *) &rxx,(char)0,sizeof(struct sdr_RX));
   prx[0U] = &rxx;
   prx[1U] = 0;
   memset((char *)stickparm,(char)0,sizeof(struct STICKPARM [256]));
   Parms();
   { /* with */
      struct sdr_RX * anonym = &rxx;
      anonym->df = 100UL;
      anonym->maxafc = (int32_t)afc;
      anonym->width = 192000UL;
      anonym->modulation = 'f';
      anonym->samples = (sdr_pAUDIOSAMPLE)sampx;
   }
   if (sdr_startsdr(url, 1001ul, port, 1001ul, 2048000UL, 192000UL, 1)) {
      sdr_setparm(1UL, tune-100000UL);
      for (pc = 0UL; pc<=255UL; pc++) {
         if (stickparm[pc].ok0) sdr_setparm(pc, stickparm[pc].val);
      } /* end for */
      lp = 0.14f;
      sndw = 0UL;
      fd = osi_OpenWrite(soundfn, 1001ul);
      if (fd>=0L) {
         recon = 1;
         for (;;) {
            sn = sdr_getsdr(64UL, prx, 17ul);
            if (sn<0L) {
               osi_WerrLn("connection lost", 16ul);
               recon = 1;
            }
            else {
               tmp = sn-1L;
               sp = 0L;
               if (sp<=tmp) for (;; sp++) {
                  left = (float)X2C_CHKNIL(sdr_pAUDIOSAMPLE,
                rxx.samples)[sp];
                  if (mono) right = left;
                  else stereo(left, &left, &right);
                  u11 = u11+((left-u11)-il1)*lp; /* lowpass 14khz */
                  u21 = u21+(il1-u21)*lp;
                  il1 = il1+(u11-u21)*lp*2.0f;
                  u12 = u12+((right-u12)-il2)*lp;
                  u22 = u22+(il2-u22)*lp;
                  il2 = il2+(u12-u22)*lp*2.0f;
                  deem1 = deem1+(u21-deem1)*0.05f; /* deemphasis */
                  deem2 = deem2+(u22-deem2)*0.05f;
                  if ((sp&3L)==3L) {
                     /* 192 > 48khz samperate */
                     sndbuf[sndw] = lim(deem1*2.0f);
                     ++sndw;
                     sndbuf[sndw] = lim(deem2*2.0f);
                     ++sndw;
                     if (sndw>1023UL) {
                        osi_WrBin(fd, (char *)sndbuf, 2048u/1u, sndw*2UL);
                        sndw = 0UL;
                        if (tshow>20UL) {
                           recon = 0;
                           /*WrFixed(sqv, 2,1); WrStr(" "); */
                           if (verb) showrssi();
                           tshow = 0UL;
                        }
                        else ++tshow;
                     }
                  }
                  if (sp==tmp) break;
               } /* end for */
            }
         }
      }
      osi_WrStr(soundfn, 1001ul);
      osi_WrStrLn(" sound file open error", 23ul);
      osi_WrStrLn("connection lost", 16ul);
   }
   else osi_WrStrLn("not connected", 14ul);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
