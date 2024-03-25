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
#define loratx_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* encode beacon-text to lora iq file and modify chirps for tests */
#define loratx_DDSSIZE 65536

#define loratx_SUBSAMPBITS 8

#define loratx_FIRMAX 4096

#define loratx_PI 3.1415926535

#define loratx_PI2 6.283185307

#define loratx_SUBSAMP 256

#define loratx_LF "\012"

#define loratx_REVCHIRP 31

#define loratx_QUARTERCHIRP 30

#define loratx_TESTCHIRP 29

#define loratx_TESTZERO 28

#define loratx_OPTIMIZE 2

#define loratx_BASEGAIN 127.0

#define loratx_LIMLEV 127.4

#define loratx_DB 4.342944819

static uint8_t loratx_WHITEN[255] = {255U,254U,252U,248U,240U,225U,194U,133U,11U,23U,47U,94U,188U,120U,241U,227U,198U,
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

#define loratx_MAXBW 5.E+5

static uint32_t loratx_BWTAB[10] = {64UL,48UL,32UL,24UL,16UL,12UL,8UL,4UL,2UL,1UL};

struct Complex;


struct Complex {
   float Im;
   float Re;
};

struct BIRDY;

typedef struct BIRDY * pBIRDY;


struct BIRDY {
   pBIRDY next;
   float w0;
   float level;
   float hz;
};

static char as[4096];

static char fni[4096];

static char fno[4096];

static char hh[4096];

static char hb[2];

static char text[301];

static uint8_t ob[32768];

struct _0;


struct _0 {
   uint32_t nsym;
   uint32_t xor;
};

static struct _0 tests[256];

static uint8_t nib[12];

static uint32_t sb[10000];


struct _1 {
   struct Complex * Adr;
   size_t Len0;
};

static struct _1 * echofifo;

static int32_t cfgopt;

static int32_t ret;

static int32_t fi;

static int32_t fo;

static char verb;

static char verb2;

static char withcrc;

static char implicitheader;

static char addjunk;

static char swapiq;

static char overdriven;

static char infilehex;

static float noisegain;

static float amp;

static float val;

static float outgain;

static float oi;

static float oq;

static float pow0;

static float pows;

static float echodelay;

static float echo0;

static float echo1;

static float headzeros;

static float headzerosfrac;

static float radardense;

static float radarlevel;

static double w;

static double fr;

static double shift;

static double symf;

static double baud;

static double sample;

static double bw;

static double bind;

static double frac;

static double bws;

static double rnd;

static double upsamp;

static double fro;

static uint32_t echop;

static uint32_t echosize;

static uint32_t i;

static uint32_t ii;

static uint32_t j;

static uint32_t sf;

static uint32_t txtlen;

static uint32_t paylen;

static uint32_t cr;

static uint32_t crc;

static uint32_t sp;

static uint32_t netid;

static uint32_t powc;

static uint32_t bins;

static uint32_t samp;

static uint32_t optimize;

static uint32_t wp;

static uint32_t outform;

static uint32_t sym;

static uint32_t preamb;

static uint32_t testc;

static uint32_t addzeros;

static uint32_t fcnt;

static uint32_t radarduration;

static uint32_t radarn;

static uint32_t radarc;

static pBIRDY birdies;

static pBIRDY bird;


static void Err(const char text0[], uint32_t text_len)
{
   osi_Werr(text0, text_len);
   osi_WerrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */


static char hex(uint32_t n)
{
   n = n&15UL;
   if (n<=9UL) return (char)(n+48UL);
   return (char)(n+55UL);
} /* end hex() */


static void WrHex(uint32_t x, uint32_t digits, uint32_t len)
{
   uint32_t i0;
   char s[256];
   if (digits>255UL) digits = 255UL;
   i0 = digits;
   while (i0<len && i0<255UL) {
      s[i0] = ' ';
      ++i0;
   }
   s[i0] = 0;
   while (digits>0UL) {
      --digits;
      s[digits] = hex(x);
      x = x/16UL;
   }
   osi_Werr(s, 256ul);
} /* end WrHex() */

/*
PROCEDURE mkdds;
VAR i,q:CARDINAL;
    r:REAL;
BEGIN
  r:=2.0*PI/FLOAT(HIGH(DDS)+1);
  q:=(HIGH(DDS)+1) DIV 4;
  FOR i:=0 TO HIGH(DDS) DO 
    DDS[i].si:=sin(FLOAT(i)*r);
    DDS[(i+q) MOD (HIGH(DDS)+1)].co:=DDS[i].si;
  END;
END mkdds;
*/

static double Random(void)
{
   rnd = rnd+3.1415926535;
   rnd = rnd*rnd;
   rnd = rnd*rnd;
   rnd = rnd*rnd;
   rnd = rnd-(double)(uint32_t)X2C_TRUNCC(rnd,0UL,X2C_max_longcard);
   return rnd;
} /* end Random() */


static float Noise12(void)
{
   uint32_t i0;
   float r;
   r = 0.0f;
   for (i0 = 0UL; i0<=11UL; i0++) {
      r = r+(float)Random();
   } /* end for */
   return r*8.3333333333333E-2f-0.5f;
} /* end Noise12() */


static void applyecho(float * si, float * sq)
{
   struct Complex es;
   es = echofifo->Adr[echop];
   echofifo->Adr[echop].Re = *si;
   echofifo->Adr[echop].Im = *sq;
   *si =  *si*echo0+es.Re*echo1;
   *sq =  *sq*echo0+es.Im*echo1;
   echop = (echop+1UL)%((echofifo->Len0-1)+1UL);
} /* end applyecho() */


static void applynoise(float * si, float * sq)
{
   float nq;
   float ni;
   ni = Noise12()*noisegain;
   nq = Noise12()*noisegain;
   pow0 = pow0+ni*ni+nq*nq;
   *si = *si+ni;
   *sq = *sq+nq;
   pows = pows+ *si* *si+ *sq* *sq;
   ++powc;
} /* end applynoise() */


static void applyradar(float * si, float * sq)
{
   if (radarn==0UL) {
      if (Noise12()+radardense*0.5f>0.5f) radarn = radarduration;
   }
   else {
      *si = *si+radarlevel;
      *sq = *sq+radarlevel;
      ++radarc;
      --radarn;
   }
} /* end applyradar() */


static void WrSN(float pow1, float pows0, float sig)
{
   char h[100];
   osi_Werr(" noise:", 8ul);
   aprsstr_FixToStr(osic_ln(pow1)*4.342944819f, 2UL, h, 100ul);
   osi_Werr(h, 100ul);
   osi_Werr("dB", 3ul);
   osi_Werr(" s/n:", 6ul);
   aprsstr_FixToStr(osic_ln(X2C_DIVR(sig*sig,pow1))*4.342944819f, 2UL, h, 100ul);
   osi_Werr(h, 100ul);
   osi_Werr("dB", 3ul);
   osi_Werr(" (s+n)/n:", 10ul);
   aprsstr_FixToStr(osic_ln(X2C_DIVR(sig*sig+pow1,pow1))*4.342944819f, 2UL, h, 100ul);
   osi_Werr(h, 100ul);
   osi_Werr("dB", 3ul);
} /* end WrSN() */


static void WrSamp(float s)
{
   int32_t n;
   if (s<(-127.4f)) {
      s = (-127.4f);
      overdriven = 1;
   }
   else if (s>127.4f) {
      s = 127.4f;
      overdriven = 1;
   }
   if (outform==0UL) {
      ob[wp] = (uint8_t)(uint32_t)X2C_TRUNCC(s+128.0f,0UL,X2C_max_longcard);
      ++wp;
   }
   else if (outform==1UL) {
      s = s*256.0f;
      n = (int32_t)X2C_TRUNCI(s,X2C_min_longint,X2C_max_longint);
      ob[wp] = (uint8_t)n;
      ++wp;
      ob[wp] = (uint8_t)(n>>8);
      ++wp;
   }
   else {
      s = s*7.8740157480315E-3f; /* float is -1 to 1 */
      n = (int32_t)*X2C_CAST(&s,float,uint32_t,uint32_t *);
      ob[wp] = (uint8_t)n;
      ++wp;
      ob[wp] = (uint8_t)(n>>8);
      ++wp;
      ob[wp] = (uint8_t)(n>>16);
      ++wp;
      ob[wp] = (uint8_t)(n>>24);
      ++wp;
   }
   if (wp>32767UL) {
      osi_WrBin(fo, (char *)ob, 32768u/1u, wp);
      wp = 0UL;
   }
} /* end WrSamp() */


static uint32_t crc16(const char b[], uint32_t b_len, uint32_t from, uint32_t len, uint32_t ini)
{
   uint16_t ic;
   uint16_t i0;
   uint16_t poli;
   uint16_t crc0;
   uint8_t d;
   uint16_t tmp;
   crc0 = (uint16_t)ini;
   poli = 0x1021U;
   tmp = (uint16_t)(len-1UL);
   i0 = (uint16_t)from;
   if (i0<=tmp) for (;; i0++) {
      d = (uint8_t)(uint8_t)b[i0];
      for (ic = 0U; ic<=7U; ic++) {
         if (((0x8000U & crc0)!=0)!=X2C_IN((int32_t)(7U-ic),8,d)) crc0 = X2C_LSH(crc0,16,1)^poli;
         else crc0 = X2C_LSH(crc0,16,1);
      } /* end for */
      if (i0==tmp) break;
   } /* end for */
   return (uint32_t)(uint16_t)crc0;
} /* end crc16() */


static void setheadcrc(uint8_t b[], uint32_t b_len)
{
   uint8_t crc0;
   crc0 = (X2C_LSH(b[0UL],8,1)^X2C_LSH(b[0UL],8,2)^X2C_LSH(b[0UL],8,3)^X2C_LSH(b[0UL],8,
                4))&0x10U|(b[0UL]^b[1UL]^X2C_LSH(b[1UL],8,1)^X2C_LSH(b[1UL],8,2)^X2C_LSH(b[2UL],8,
                3))&0x8U|(b[0UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[1UL],8,2)^X2C_LSH(b[2UL],8,
                1))&0x4U|(b[0UL]^b[2UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[1UL]^b[2UL],8,
                1))&0x2U|(b[0UL]^b[2UL]^X2C_LSH(b[1UL]^b[2UL],8,-1)^X2C_LSH(b[2UL],8,-3)^X2C_LSH(b[2UL],8,-2))&0x1U;
   b[3UL] = X2C_LSH(crc0,8,-4)&0x1U;
   b[4UL] = crc0&0xFU;
} /* end setheadcrc() */


static void sethamm(uint32_t bits, uint8_t * b)
{
   if (bits>1UL) {
      *b = *b&0xFU|(X2C_LSH(*b,8,4)^X2C_LSH(*b,8,3)^X2C_LSH(*b,8,2))&0x30U|(X2C_LSH(*b,8,6)^X2C_LSH(*b,8,5)^X2C_LSH(*b,
                8,3))&0x40U|(X2C_LSH(*b,8,7)^X2C_LSH(*b,8,5)^X2C_LSH(*b,8,4))&0x80U;
   }
   else if (bits>0UL) {
      /* b4=0/1/2 b5=2/3/4 */
      /* b6=0/1/3 */
      /* b7=0/2/3 */
      *b = *b|(X2C_LSH(*b,8,4)^X2C_LSH(*b,8,3)^X2C_LSH(*b,8,2)^X2C_LSH(*b,8,1))&0x10U;
   }
} /* end sethamm() */


static void interleav(uint32_t sf0, uint32_t cr0, uint32_t lsb, uint32_t * start, uint32_t rb[],
                uint32_t rb_len, const uint8_t hb0[], uint32_t hb_len)
{
   uint32_t j1;
   uint32_t i0;
   uint32_t gx;
   uint32_t g;
   uint32_t tmp;
   uint32_t tmp0;
   /*  FILL(ADR(rb),0C,SIZE(rb)); */
   tmp = sf0-1UL;
   j1 = 0UL;
   if (j1<=tmp) for (;; j1++) {
      tmp0 = cr0-1UL;
      i0 = 0UL;
      if (i0<=tmp0) for (;; i0++) {
         if (X2C_IN(i0,8,(uint8_t)hb0[(((sf0-1UL)-j1)+i0)%sf0])) {
            rb[*start+i0] |= (1UL<<(sf0-1UL)-j1);
         }
         if (i0==tmp0) break;
      } /* end for */
      if (j1==tmp) break;
   } /* end for */
   tmp = cr0-1UL;
   i0 = 0UL;
   if (i0<=tmp) for (;; i0++) {
      g = rb[*start];
      gx = g;
      tmp0 = sf0-1UL;
      j1 = 1UL;
      if (j1<=tmp0) for (;; j1++) {
         /* gray */
         gx = X2C_LSH(gx,32,-1);
         g = g^gx;
         if (j1==tmp0) break;
      } /* end for */
      rb[*start] = X2C_LSH(g,32,(int32_t)lsb); /*+ CAST(SET32,lsb DIV 1*/ /* peculiarity, 1 bin shift */
      rb[*start] = (uint32_t)((uint32_t)rb[*start]+1UL)&0x7FFFFFFFUL;
      ++*start;
      if (i0==tmp) break;
   } /* end for */
} /* end interleav() */


static uint8_t nibble(uint32_t wp0, const char text0[], uint32_t text_len)
{
   if (!(wp0&1)) return (uint8_t)(uint8_t)text0[wp0/2UL]&0xFU;
   return X2C_LSH((uint8_t)(uint8_t)text0[wp0/2UL],8,-4)&0xFU;
} /* end nibble() */


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

static uint8_t _cnst0[255] = {255U,254U,252U,248U,240U,225U,194U,133U,11U,23U,47U,94U,188U,120U,241U,227U,198U,141U,
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
static uint32_t _cnst[10] = {64UL,48UL,32UL,24UL,16UL,12UL,8UL,4UL,2UL,1UL};

X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   uint32_t tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   verb = 0;
   verb2 = 0;
   overdriven = 0;
   infilehex = 0;
   outform = 0UL;
   outgain = 1.0f;
   sf = 12UL;
   cr = 5UL;
   optimize = 0UL;
   bw = 1.25E+5;
   sample = 0.0;
   withcrc = 0;
   preamb = 8UL;
   cfgopt = -1L;
   fno[0] = 0;
   fni[0] = 0;
   implicitheader = 0;
   shift = 0.0;
   netid = 18UL;
   testc = 0UL;
   noisegain = 0.0f;
   addzeros = 0UL;
   headzeros = 0.0f;
   addjunk = 0;
   swapiq = 0;
   echofifo = 0;
   echo1 = 0.0f;
   echop = 0UL;
   birdies = 0;
   radarduration = 0UL;
   radarn = 0UL;
   radarc = 0UL;
   for (;;) {
      osi_NextArg(as, 4096ul);
      if (as[0U]==0) break;
      if ((as[0U]=='-' && as[1U]) && as[2U]==0) {
         if (as[0U]=='-') {
            if (as[1U]=='i') osi_NextArg(fni, 4096ul);
            else if (as[1U]=='o') osi_NextArg(fno, 4096ul);
            else if (as[1U]=='C') withcrc = 1;
            else if (as[1U]=='I') implicitheader = 1;
            else if (as[1U]=='v') verb = 1;
            else if (as[1U]=='V') {
               verb = 1;
               verb2 = 1;
            }
            else if (as[1U]=='j') addjunk = 1;
            else if (as[1U]=='q') swapiq = 1;
            else if (as[1U]=='H') infilehex = 1;
            else if (as[1U]=='O') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToInt(as, 4096ul, &cfgopt)) {
                  Err("-O <optimize> 0..1", 19ul);
               }
            }
            else if (as[1U]=='r') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) Err("-r <iq-samplerate>", 19ul);
               if (val<=0.0f) Err("iq-samplerate range Hz", 23ul);
               sample = (double)val;
            }
            else if (as[1U]=='S') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) Err("-S <shift Hz>", 14ul);
               shift = (double)val;
            }
            else if (as[1U]=='g') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) {
                  Err("-g <gainfactor 0.0..1.0>", 25ul);
               }
               outgain = val;
            }
            else if (as[1U]=='w') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&noisegain, as, 4096ul)) {
                  Err("-g <gainfactor 0.0..1.0>", 25ul);
               }
            }
            else if (as[1U]=='R') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&val, as, 4096ul)) Err("-R <seed>", 10ul);
               rnd = (double)val;
            }
            else if (as[1U]=='E') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&echodelay, as, 4096ul)) {
                  Err("-E <delay> <gain>", 18ul);
               }
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&echo1, as, 4096ul)) {
                  Err("-E <delay> <gain>", 18ul);
               }
            }
            else if (as[1U]=='f') {
               osi_NextArg(as, 4096ul);
               if (as[0U]=='u' && as[1U]=='8') outform = 0UL;
               else if ((as[0U]=='i' && as[1U]=='1') && as[2U]=='6') outform = 1UL;
               else if ((as[0U]=='f' && as[1U]=='3') && as[2U]=='2') outform = 2UL;
               else Err("-f output formats u8 i16 f32", 29ul);
            }
            else if (as[1U]=='b') {
               osi_NextArg(as, 4096ul);
               if (aprsstr_StrToCard(as, 4096ul, &i)) {
                  if (i>9UL) bw = (double)i;
                  else bw = X2C_DIVL(5.E+5,(double)_cnst[i]);
               }
               else Err("-b <tablenum> 0..9 or -b <Hz>", 30ul);
            }
            else if (as[1U]=='s') {
               osi_NextArg(as, 4096ul);
               if ((!aprsstr_StrToCard(as, 4096ul, &sf) || sf<5UL) || sf>12UL) {
                  Err("-s <sf> 5..12", 14ul);
               }
            }
            else if (as[1U]=='z') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &addzeros)) Err("-z <n>", 7ul);
            }
            else if (as[1U]=='Z') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&headzeros, as, 4096ul)) Err("-Z <n>", 7ul);
            }
            else if (as[1U]=='N') {
               osi_NextArg(as, 4096ul);
               if (!StrToHex(as, 4096ul, &netid)) Err("-N <netid>", 11ul);
            }
            else if (as[1U]=='c') {
               osi_NextArg(as, 4096ul);
               if ((!aprsstr_StrToCard(as, 4096ul, &cr) || cr<4UL) || cr>8UL) {
                  Err("-c <cr> 4..8", 13ul);
               }
            }
            else if (as[1U]=='T') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &tests[testc].nsym)) {
                  Err("-T <n> <pattern>", 17ul);
               }
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &tests[testc].xor)) {
                  Err("-T <n> <pattern>", 17ul);
               }
               if (testc<255UL) ++testc;
            }
            else if (as[1U]=='p') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &preamb)) Err("-p <cr> 5..8", 13ul);
            }
            else if (as[1U]=='B') {
               osi_NextArg(as, 4096ul);
               osic_alloc((char * *) &bird, sizeof(struct BIRDY));
               if (bird==0) Err("-B out of memory", 17ul);
               if (!aprsstr_StrToFix(&bird->hz, as, 4096ul)) Err("-B <hz> <level>", 16ul);
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&bird->level, as, 4096ul)) {
                  Err("-B <hz> <level>", 16ul);
               }
               bird->next = birdies;
               birdies = bird;
            }
            else if (as[1U]=='P') {
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToCard(as, 4096ul, &radarduration)) {
                  Err("-P <duration> <density> <level>", 32ul);
               }
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&radardense, as, 4096ul)) {
                  Err("-P <duration> <density> <level>", 32ul);
               }
               osi_NextArg(as, 4096ul);
               if (!aprsstr_StrToFix(&radarlevel, as, 4096ul)) {
                  Err("-P <duration> <density> <level>", 32ul);
               }
            }
            else if (as[1U]=='h') {
               osi_WrStrLn("Make iq file with lora encoded text and apply noise, dropouts and wrong bits for tests",
                87ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn(" -B <hz> <level>     add a tone +-hz with gain <level> -B may be repeatet", 74ul);
               osi_WrStrLn(" -b <bandwidth>      <Hz> or index kHz 0:7.8 1:10.4 2:15.6 3:20.8 4:31.25 5:41.7 6:62.5 7:12\
5 8:250 9:500 (7)", 110ul);
               osi_WrStrLn(" -C                  send CRC", 30ul);
               osi_WrStrLn(" -c <codingrate>     4..8 (5)", 30ul);
               osi_WrStrLn(" -E <delay(s)> <gain> add radiopath echo with delay (s) and gain e.g. 0.0005 0.5", 81ul);
               osi_WrStrLn("                       only whole sample delay steps so use oversampling", 73ul);
               osi_WrStrLn(" -f <output-format>  u8 i16 f32 (u8)", 37ul);
               osi_WrStrLn(" -g <gainfactor>     output level 0.0..1.0, with -f f32 more (1.0)", 67ul);
               osi_WrStrLn(" -H                  input file is in HEX", 42ul);
               osi_WrStrLn(" -h                  this...", 29ul);
               osi_WrStrLn(" -I                  implicit header on", 40ul);
               osi_WrStrLn(" -i <infile>         text to send filename", 43ul);
               osi_WrStrLn(" -j                  insert \'3C FF 01\' (hex) before text", 57ul);
               osi_WrStrLn(" -N <hh>             Sync symbols, network-id in hex (12)", 58ul);
               osi_WrStrLn(" -O <0..1>           optimize on off else automatic (-1)", 57ul);
               osi_WrStrLn(" -o <outfile>        iq file name", 34ul);
               osi_WrStrLn(" -P <duration-samples> <random-density> <level>  add delta pulse noise (4 0.5 0.9)", 83ul);
               osi_WrStrLn(" -p <chirps>         preamble length (8)", 41ul);
               osi_WrStrLn(" -q                  invert IQ", 31ul);
               osi_WrStrLn(" -R <x>              random seed for noise (0.0)", 49ul);
               osi_WrStrLn(" -r <samplerate>     iq sampelrate Hz >=bandwidth (125000.0)", 61ul);
               osi_WrStrLn(" -S <shift>          shift signal frequency inside iq-band (Hz) (0)", 68ul);
               osi_WrStrLn(" -s <sf>             spread factor (5..12) (12)", 48ul);
               osi_WrStrLn(" -T <n> <pattern>    for FEC tests set n\'th chirp to zero level (pattern=0)", 76ul);
               osi_WrStrLn("                       or xor with <pattern> (decimal), may be repeatet", 72ul);
               osi_WrStrLn(" -V                  more verbous", 34ul);
               osi_WrStrLn(" -v                  verbous", 29ul);
               osi_WrStrLn(" -w <level>          add white noise", 37ul);
               osi_WrStrLn(" -Z <n>              zero or noise chirps before data (0.0)", 60ul);
               osi_WrStrLn(" -z <n>              add zero or noise chirps at end (0)", 57ul);
               osi_WrStrLn("loratx -i beacon.txt -j -f i16 -r 2600000 -C -o /tmp/lora.iq -z 10 -s 12 -c 5 -b 7", 83ul);
               X2C_ABORT();
            }
            else Err(as, 4096ul);
         }
         else Err("use -h", 7ul);
      }
      else Err("use -h", 7ul);
   }
   if (sample==0.0) sample = bw;
   bird = birdies;
   while (bird) {
      bird->hz = (float)((X2C_DIVL((double)bird->hz,sample))*6.283185307);
      bird->level = bird->level*127.0f;
      bird = bird->next;
   }
   if (echo1!=0.0f) {
      /* aplay echo */
      echo0 = X2C_DIVR(1.0f,1.0f+(float)fabs(echo1));
      echo1 = 1.0f-echo0;
      if (echo1<0.0f) echo1 = -echo1;
      if (echodelay<=0.0f) Err("echo delay >0.0", 16ul);
      echodelay = (float)((double)echodelay*sample);
      if (echodelay>1.E+8f) Err("echo delay too high", 20ul);
      echosize = (uint32_t)X2C_TRUNCC(echodelay+0.5f,0UL,X2C_max_longcard);
      if (echosize>0UL) {
         X2C_DYNALLOCATE((char **) &echofifo,sizeof(struct Complex),(tmp[0] = echosize,tmp),1u);
      }
   }
   if (!implicitheader && sf<=6UL) Err("sf<=6 needs implicit header", 28ul);
   if (fabs(shift)*2.0+bw>sample*1.001) osi_WerrLn("WARNING: signal does not fit in iq-bandwidth", 45ul);
   /*  shift:=shift/bw*PI2; */
   shift = (X2C_DIVL(shift,sample))*6.283185307;
   if (cfgopt==0L) optimize = 0UL;
   else if (cfgopt==1L || X2C_DIVL((double)(float)(uint32_t)X2C_LSH(0x1UL,32,(int32_t)sf),bw)>0.016) {
      optimize = 2UL;
   }
   if (verb) {
      osi_Werr("opt:", 5ul);
      aprsstr_CardToStr(optimize, 1UL, hh, 4096ul);
      osi_WerrLn(hh, 4096ul);
      osi_Werr("bandwidth Hz:", 14ul);
      aprsstr_FixToStr((float)bw, 3UL, hh, 4096ul);
      osi_WerrLn(hh, 4096ul);
      osi_Werr("chirptime s:", 13ul);
      aprsstr_FixToStr((float)(X2C_DIVL((double)(float)(uint32_t)(1UL<<sf),bw)), 6UL, hh, 4096ul);
      osi_WerrLn(hh, 4096ul);
      osi_Werr("sync/netid:", 12ul);
      WrHex(netid, 2UL, 0UL);
      osi_WerrLn("", 1ul);
      if (echosize>0UL) {
         osi_Werr("echo delay samples:", 20ul);
         aprsstr_CardToStr(echosize, 1UL, hh, 4096ul);
         osi_WerrLn(hh, 4096ul);
      }
   }
   fi = osi_OpenRead(fni, 4096ul);
   if (fi<0L) Err("input file open", 16ul);
   ret = osi_RdBin(fi, (char *)text, 301u/1u, 301UL);
   osic_Close(fi);
   if (ret<0L) Err("input file read", 16ul);
   txtlen = (uint32_t)ret;
   if (infilehex) {
      txtlen = txtlen/2UL;
      i = 0UL;
      for (;;) {
         if (i>=txtlen) break;
         hb[0U] = text[i*2UL];
         hb[1U] = text[i*2UL+1UL];
         if ((uint8_t)hb[0U]<'0' || (uint8_t)hb[1U]<'0') break;
         if (!StrToHex(hb, 2ul, &ii)) Err("input file illegan hex char", 28ul);
         text[i] = (char)ii;
         ++i;
      }
      txtlen = i;
   }
   if (addjunk) {
      if (ret>0L) {
         for (i = (uint32_t)(ret-1L);; i--) {
            text[i+3UL] = text[i];
            if (i==0UL) break;
         } /* end for */
      }
      text[0U] = '<'; /* for whatever */
      text[1U] = '\377'; /* for whatever */
      text[2U] = '\001'; /* for whatever */
      txtlen += 3UL;
   }
   if (txtlen>255UL) txtlen = 255UL;
   if (verb) {
      osi_Werr("len:", 5ul);
      aprsstr_CardToStr(txtlen, 1UL, hh, 4096ul);
      osi_WerrLn(hh, 4096ul);
   }
   fo = osi_OpenWrite(fno, 4096ul);
   if (fo<0L) Err("iq file write", 14ul);
   paylen = txtlen;
   if (withcrc) {
      crc = 0UL;
      if (paylen>2UL) crc = crc16(text, 301ul, 0UL, paylen-2UL, 0UL);
      if (paylen>1UL) {
         crc = (uint32_t)((uint16_t)crc^(uint16_t)((uint32_t)(uint8_t)text[paylen-2UL]*256UL));
      }
      if (paylen>0UL) {
         crc = (uint32_t)((uint16_t)crc^(uint16_t)(uint32_t)(uint8_t)text[paylen-1UL]);
      }
      /*
      FOR i:=0 TO paylen-2 DO WrHex(ORD(text[i]),3); END; WrStr(" -> ");
      WrHex(crc, 3); WrHex(crc DIV 256, 3); WrStrLn("=crc");
      */
      text[paylen] = (char)(crc&255UL);
      text[paylen+1UL] = (char)(crc/256UL);
      paylen += 2UL;
   }
   for (i = paylen; i<=300UL; i++) {
      text[i] = 0;
   } /* end for */
   tmp0 = txtlen-1UL;
   i = 0UL;
   if (i<=tmp0) for (;; i++) {
      text[i] = (char)((uint8_t)(uint8_t)text[i]^(uint8_t)_cnst0[i]); /* whiten */
      if (i==tmp0) break;
   } /* end for */
   if (!implicitheader) {
      nib[0U] = (uint8_t)(txtlen/16UL);
      nib[1U] = (uint8_t)(txtlen&15UL);
      nib[2U] = (uint8_t)((uint32_t)withcrc+(cr-4UL)*2UL);
      setheadcrc(nib, 12ul);
      wp = 0UL;
      tmp0 = (sf-2UL)-1UL;
      i = 5UL;
      if (i<=tmp0) for (;; i++) {
         /* fill rest of header with payload */
         nib[i] = nibble(wp, text, 301ul);
         ++wp;
         if (i==tmp0) break;
      } /* end for */
      tmp0 = (sf-2UL)-1UL;
      i = 0UL;
      if (i<=tmp0) for (;; i++) {
         sethamm(4UL, &nib[i]); /* apply 4 bit hamming */
         if (i==tmp0) break;
      } /* end for */
   }
   /*
   FOR j:=0 TO 7 DO
     FOR i:=0 TO 9 DO WrInt(ORD((7-j) IN nib[i]),1); END;
     WrStrLn("");
   END;
   WrStrLn("");
   */
   memset((char *)sb,(char)0,40000UL);
   sp = 0UL;
   headzerosfrac = 0.0f;
   if (headzeros>0.0f) {
      i = (uint32_t)X2C_TRUNCC(headzeros,0UL,X2C_max_longcard);
      headzerosfrac = headzeros-(float)i;
      if (headzerosfrac!=0.0f) ++i;
      while (i>0UL) {
         sb[sp] = 0x10000000UL;
         if (sp<9997UL) ++sp;
         --i;
      }
   }
   tmp0 = preamb;
   i = 1UL;
   if (i<=tmp0) for (;; i++) {
      sb[sp] = 0UL;
      ++sp;
      if (i==tmp0) break;
   } /* end for */
   sb[sp] = (uint32_t)((netid/16UL)*8UL);
   ++sp;
   sb[sp] = (uint32_t)((netid&15UL)*8UL);
   ++sp;
   sb[sp] = 0x80000000UL; /* reverse chirp */
   ++sp;
   sb[sp] = 0x80000000UL; /* reverse chirp */
   ++sp;
   sb[sp] = 0xC0000000UL; /* quarter reverse chirp */
   ++sp;
   if (!implicitheader) interleav(sf-2UL, 8UL, 2UL, &sp, sb, 10000ul, nib, 12ul);
   /*
   FOR j:=0 TO 9 DO   (* show interleaved header *)
     FOR i:=0 TO 7 DO WrInt(ORD((11-j) IN CAST(SET32,sb[sp+i-8])),1); END;
     WrStrLn("");
   END;
   WrStrLn("");
   */
   while (wp/2UL<paylen) {
      tmp0 = (sf-optimize)-1UL;
      i = 0UL;
      if (i<=tmp0) for (;; i++) {
         nib[i] = nibble(wp, text, 301ul);
         sethamm(cr-4UL, &nib[i]);
         ++wp;
         if (i==tmp0) break;
      } /* end for */
      interleav(sf-optimize, cr, optimize, &sp, sb, 10000ul, nib, 12ul);
   }
   /*FOR i:=j TO sp-1 DO sb[i]:=VAL(SET32, (i-j)*30 MOD 1000) END; */
   /*FOR i:=j TO sp-1 DO sb[i]:=VAL(SET32, 0) END; */
   while (addzeros>0UL) {
      sb[sp] = 0x10000000UL;
      if (sp<9997UL) ++sp;
      --addzeros;
   }
   if (noisegain!=0.0f) {
      /* append noise before and end */
      for (i = sp; i>=1UL; i--) {
         sb[i] = sb[i-1UL];
      } /* end for */
      sb[0U] = 0x10000000UL;
      ++sp;
      sb[sp] = 0x10000000UL;
      ++sp;
   }
   wp = 0UL;
   samp = 0UL;
   bins = (uint32_t)(1UL<<sf);
   bind = X2C_DIVL(X2C_DIVL(bw,sample),(double)bins);
   baud = bw*bind;
   w = 0.0;
   i = 0UL;
   while (i<testc) {
      /* modify chirps for tests */
      if (tests[i].nsym<=9999UL) {
         if (tests[i].xor==0UL) sb[tests[i].nsym] |= 0x10000000UL;
         else {
            sb[tests[i].nsym] = sb[tests[i].nsym]^(uint32_t)tests[i].xor; /* xor with pattern */
            sb[tests[i].nsym] |= 0x20000000UL;
         }
      }
      ++i;
   }
   if (verb2) {
      tmp0 = sp-1UL;
      i = 1UL;
      if (i<=tmp0) for (;; i++) {
         aprsstr_CardToStr(i, 2UL, hh, 4096ul);
         osi_Werr(hh, 4096ul);
         osi_Werr(":", 2ul);
         for (j = 11UL;; j--) {
            aprsstr_CardToStr((uint32_t)X2C_IN(j,32,sb[i]), 1UL, hh, 4096ul);
            osi_Werr(hh, 4096ul);
            if (j==0UL) break;
         } /* end for */
         if ((0x10000000UL & sb[i])) osi_Werr(" -zero-level", 13ul);
         else if ((0x20000000UL & sb[i])) osi_Werr(" -xord", 7ul);
         else if ((0x40000000UL & sb[i])) osi_Werr(" -quarter", 10ul);
         else if ((0x80000000UL & sb[i])) osi_Werr(" -reverse", 10ul);
         osi_WerrLn("", 1ul);
         if (i==tmp0) break;
      } /* end for */
   }
   noisegain = noisegain*127.0f*2.0f;
   pow0 = 0.0f;
   pows = 0.0f;
   powc = 0UL;
   bws = (X2C_DIVL(bw,sample))*6.283185307;
   if (headzerosfrac!=0.0f) {
      samp = (uint32_t)X2C_TRUNCC(X2C_DIVL((double)(1.0f-headzerosfrac),bind),0UL,X2C_max_longcard);
   }
   upsamp = 0.0;
   do {
      symf = (double)samp*bind;
      sym = (uint32_t)X2C_TRUNCC(symf,0UL,X2C_max_longcard);
      frac = ((symf-(double)sym)-0.5)+X2C_DIVL((double)(uint32_t)(sb[sym]&0xFFFUL),(double)bins);
      if (frac>0.5) frac = frac-1.0;
      if ((0x40000000UL & sb[sym])==0 || frac<(-0.25)) {
         fr = frac*bws;
         if (((0x80000000UL & sb[sym])!=0)!=swapiq) fr = -fr;
         amp = (float)((0.5-fabs(frac))*12700.0); /* lower amplitude at freq jump works */
         if (amp>127.0f) amp = 127.0f;
         amp = amp*outgain;
         if ((0x10000000UL & sb[sym])) amp = 0.0f;
         w = w+fr+shift;
         if (w>3.1415926535) w = w-6.283185307;
         else if (w<(-3.1415926535)) w = w+6.283185307;
         oi = osic_cos((float)w)*amp;
         oq = osic_sin((float)w)*amp;
         if (echofifo) applyecho(&oi, &oq);
         if (noisegain!=0.0f) applynoise(&oi, &oq);
         if (radarduration>0UL) applyradar(&oi, &oq);
         /*- add birdies */
         bird = birdies;
         while (bird) {
            oi = oi+osic_cos(bird->w0)*bird->level;
            oq = oq+osic_sin(bird->w0)*bird->level;
            bird->w0 = bird->w0+bird->hz;
            if (bird->w0>6.283185307f) bird->w0 = bird->w0-6.283185307f;
            else if (bird->w0<(-6.283185307f)) bird->w0 = bird->w0+6.283185307f;
            bird = bird->next;
         }
         WrSamp(oi);
         WrSamp(oq);
         fro = fro+fr;
         ++fcnt;
      }
      ++samp;
   } while (sym<sp);
   if (wp>0UL) osi_WrBin(fo, (char *)ob, 32768u/1u, wp);
   if (overdriven) osi_WerrLn("WARNING: output level hard limited!", 36ul);
   if (verb && noisegain!=0.0f) {
      osi_Werr("sig:", 5ul);
      aprsstr_FixToStr(osic_ln(outgain)*8.685889638f, 2UL, hh, 4096ul);
      osi_Werr(hh, 4096ul);
      osi_Werr("dB", 3ul);
      if (pow0!=0.0f && powc) {
         pow0 = X2C_DIVR(pow0,(float)powc*127.0f*127.0f);
         pows = X2C_DIVR(pows,(float)powc*127.0f*127.0f);
         WrSN(pow0, pows, outgain);
         if (sample!=bw) {
            osi_WerrLn("", 1ul);
            osi_Werr("in signal band ", 16ul);
            WrSN((float)(X2C_DIVL((double)pow0*bw,sample)), (float)(X2C_DIVL((double)pows*bw,sample)),
                 outgain);
         }
      }
      osi_WerrLn("", 1ul);
   }
   if (verb && radarc>0UL) {
      osi_Werr("added pulses:", 14ul);
      aprsstr_CardToStr(radarc, 1UL, hh, 4096ul);
      osi_Werr(hh, 4096ul);
      osi_WerrLn("", 1ul);
   }
   osic_Close(fo);
/*  
FOR i:=0 TO 255 DO tests[i].xor:=0 END;
FOR i:=0 TO 10000000 DO INC(tests[TRUNC((Noise12()+0.5)*255.99)].xor) END;
FOR i:=0 TO 255 DO WrInt(tests[i].xor,6); IF i MOD 16=15 THEN WrStrLn("") END; END;
*/
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
