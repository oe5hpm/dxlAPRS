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

struct LOWPASSCONTEXT;


struct LOWPASSCONTEXT {
   float uc1;
   float il;
   float uc2;
};

#define radio_OFFSET 100
/* khz from center frequency to avoid adc dc error area */

#define radio_DEEMPHASIS 0.05
/* check value ... */

#define radio_MAINSAMP 192000

#define radio_AUDIOSAMP 48000

#define radio_LP14 0.14

static int32_t fd;

static uint32_t sndw;

static uint32_t ifwidth;

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

static char rds;

static uint32_t afc;

static uint32_t tune;

static struct STICKPARM stickparm[256];


static void Usage(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(" usage: ", 9ul);
   osi_WerrLn(text, text_len);
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
   rds = 0;
   verb = 0;
   afc = 50UL;
   strncpy(url,"127.0.0.1",1001u);
   strncpy(port,"1234",1001u);
   soundfn[0] = 0;
   tune = 100000000UL;
   ifwidth = 0UL;
   for (;;) {
      osi_NextArg(s, 1001ul);
      if (s[0U]==0) break;
      if ((s[0U]=='-' && s[1U]) && s[2U]==0) {
         if (s[1U]=='m') mono = 1;
         else if (s[1U]=='r') rds = 1;
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
         else if (s[1U]=='w') {
            /* afc */
            osi_NextArg(s, 1001ul);
            if ((!aprsstr_StrToCard(s, 1001ul,
                &ifwidth) || ifwidth>192UL) || ifwidth<3UL) {
               Usage(" -w <khz> (3..192)", 19ul);
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
            osi_WrStrLn(" -m             force mono", 27ul);
            osi_WrStrLn(" -p <cmd> <value> send rtl_tcp parameter, ppm, tuner\
gain ... -p 5 <ppm>", 72ul);
            osi_WrStrLn(" -r             RDS on", 23ul);
            osi_WrStrLn(" -s <soundfilename>  16bit 48kHz signed 2 channel so\
und stream/pipe", 68ul);
            osi_WrStrLn(" -t <url:port>  connect rtl:tcp server (127.0.0.1:12\
34)", 56ul);
            osi_WrStrLn(" -v             show rssi (dB) and afc (khz)",
                45ul);
            osi_WrStrLn(" -w <khz>       IF-width, less cpu 3,6,12,24,48,96,1\
92 (mono 96, stereo 192)", 77ul);
            osi_WrStrLn("example: sdrradio -f 101.2 -s /dev/stdout -t 192.168\
.1.1:1234 -p 5 72 -p 8 1 -r -v | aplay", 91ul);
            X2C_ABORT();
         }
         else Usage("-h", 3ul);
      }
      else Usage("-h", 3ul);
   }
} /* end Parms() */


static void showrssi(char s[], uint32_t s_len)
{
   char ss[31];
   if (isstereo) aprsstr_Append(s, s_len, "S", 2ul);
   else aprsstr_Append(s, s_len, "M", 2ul);
   aprsstr_FixToStr(osic_ln((rxx.rssi+1.0f)*3.0517578125E-5f)*4.342944819f,
                2UL, ss, 31ul);
   aprsstr_Append(s, s_len, ss, 31ul);
   aprsstr_Append(s, s_len, "dB", 3ul);
   aprsstr_IntToStr(rxx.afckhz, 3UL, ss, 31ul);
   aprsstr_Append(s, s_len, ss, 31ul);
} /* end showrssi() */

static float uc19;

static float il19;

static float uc20;

static float il20;

static float um20;

static float sqv;


static float lowpass(float u, float fg, struct LOWPASSCONTEXT * c)
{
   struct LOWPASSCONTEXT * anonym;
   { /* with */
      struct LOWPASSCONTEXT * anonym = c;
      anonym->uc1 = anonym->uc1+((u-anonym->uc1)-anonym->il)*fg;
      anonym->uc2 = anonym->uc2+(anonym->il-anonym->uc2)*fg;
      anonym->il = anonym->il+(anonym->uc1-anonym->uc2)*fg*2.0f;
      return anonym->uc2;
   }
} /* end lowpass() */

/*----------------rds */
/*
      COUNTRYS="DE GR MA __ MD "
              +"DZ CY CZ IE EE "
              +"AD SM PL TR __ "
              +"IL CH VA MK __ "
              +"IT JO SK __ __ "
              +"BE FI SY __ UA "
              +"RU LU TN __ __ "
              +"PS BG __ NL PT "
              +"AL DK LI LV SI "
              +"AT GI IS LB __ "
              +"HU IQ MC __ __ "
              +"MT GB LT HR __ "
              +"DE LY YU __ __ "
              +"__ RO ES SE __ "
              +"EG FR NO BY BA "
              +"               ";
*/
typedef char MSG[64];

static double rdsclk;

static struct LOWPASSCONTEXT lpi;

static struct LOWPASSCONTEXT lpq;

static float uio;

static float sint;

static char nrzi;

static uint32_t bitc;

static uint32_t samec;

static uint32_t synoffset;

static uint32_t countrycode;

static uint32_t textAB;

static uint32_t maxlen;

static char bitb[104];

static float bitm[104];

static MSG progtext;

static MSG radiotext;

static char progname[8];


static void progtyp(uint32_t n, char s[], uint32_t s_len)
{
   char h[32];
   switch (n) {
   case 1UL:
      strncpy(h,"News",32u);
      break;
   case 2UL:
      strncpy(h,"Affairs",32u);
      break;
   case 3UL:
      strncpy(h,"Info",32u);
      break;
   case 4UL:
      strncpy(h,"Sport",32u);
      break;
   case 5UL:
      strncpy(h,"Educate",32u);
      break;
   case 6UL:
      strncpy(h,"Drama",32u);
      break;
   case 7UL:
      strncpy(h,"Culture",32u);
      break;
   case 8UL:
      strncpy(h,"Science",32u);
      break;
   case 9UL:
      strncpy(h,"Varied",32u);
      break;
   case 10UL:
      strncpy(h,"Pop M",32u);
      break;
   case 11UL:
      strncpy(h,"Rock M",32u);
      break;
   case 12UL:
      strncpy(h,"Easy M",32u);
      break;
   case 13UL:
      strncpy(h,"Light M",32u);
      break;
   case 14UL:
      strncpy(h,"Classics",32u);
      break;
   case 15UL:
      strncpy(h,"Other M",32u);
      break;
   case 16UL:
      strncpy(h,"Weather",32u);
      break;
   case 17UL:
      strncpy(h,"Finance",32u);
      break;
   case 18UL:
      strncpy(h,"Children",32u);
      break;
   case 19UL:
      strncpy(h,"Social",32u);
      break;
   case 20UL:
      strncpy(h,"Religion",32u);
      break;
   case 21UL:
      strncpy(h,"Phone-In",32u);
      break;
   case 22UL:
      strncpy(h,"Travel",32u);
      break;
   case 23UL:
      strncpy(h,"Leisure",32u);
      break;
   case 24UL:
      strncpy(h,"Jazz",32u);
      break;
   case 25UL:
      strncpy(h,"Country",32u);
      break;
   case 26UL:
      strncpy(h,"Nation M",32u);
      break;
   case 27UL:
      strncpy(h,"Oldies",32u);
      break;
   case 28UL:
      strncpy(h,"Folk M",32u);
      break;
   case 29UL:
      strncpy(h,"Document",32u);
      break;
   case 30UL:
      strncpy(h,"Test",32u);
      break;
   case 31UL:
      strncpy(h,"Alarm!",32u);
      break;
   default:;
      h[0] = 0;
      break;
   } /* end switch */
   aprsstr_Assign(s, s_len, h, 32ul);
} /* end progtyp() */


static uint32_t getbits(const char b[], uint32_t b_len,
                uint32_t offset, uint32_t len)
{
   uint32_t v;
   v = 0UL;
   while (len>0UL) {
      v += v+(uint32_t)b[offset%((b_len-1)+1UL)];
      ++offset;
      --len;
   }
   return v;
} /* end getbits() */


static char chkchr(uint32_t n)
/* Ouml 215, auml 145 */
/* VAR h:ARRAY[0..10] OF CHAR; */
{
   if (n<32UL) return ' ';
   if (n>126UL) {
      /*IntToStr(ORD(n),4, h); WerrLn("");WerrLn(h);*/
      return '^';
   }
   return (char)n;
} /* end chkchr() */

/*
PROCEDURE wrblk(n:CARDINAL; oc:CHAR);
VAR i,a:CARDINAL;
    h:ARRAY[0..20] OF CHAR;
BEGIN
  Werr(oc);
  FOR i:=15 TO 0 BY -1 DO Werr(CHR(48+ORD(i IN CAST(SET32, n)))) END;
  IF oc="A" THEN
--    IntToStr(countrycode, 2, pitext);

  ELSIF oc="B" THEN

  ELSIF oc="C" THEN
    Werr(" ");
    a:=n MOD 256;
    IF (a>=224) & (a<=228) & (countrycode>0) THEN         (* country code *)
      a:=a-224 + (countrycode-1)*5;
      countrytext[0]:=COUNTRYS[a*3];
      countrytext[1]:=COUNTRYS[a*3+1];
      countrytext[2]:=0C;
    END;
    IF (n DIV 256>=32) & (n DIV 256<127) THEN Werr(CHR(n DIV 256))
                ELSE Werr(" ") END;
    IF (n MOD 256>=32) & (n MOD 256<127) THEN Werr(CHR(n MOD 256))
                ELSE Werr(" ") END;
  ELSIF oc="D" THEN
    Werr(" ");
    IF (n DIV 256>=32) & (n DIV 256<127) THEN Werr(CHR(n DIV 256))
                ELSE Werr(" ") END;
    IF (n MOD 256>=32) & (n MOD 256<127) THEN Werr(CHR(n MOD 256))
                ELSE Werr(" ") END;
  END;
  Werr(" ");
END wrblk;
*/
static uint32_t radio_POLY = 0x5B9UL;


static char crc(const char b[], uint32_t b_len,
                uint32_t offset, uint32_t xor)
{
   uint32_t c;
   uint32_t i;
   c = 0UL;
   for (i = 0UL; i<=25UL; i++) {
      c = X2C_LSH(c,32,1);
      if (i<16UL && b[(offset+i)%((b_len-1)+1UL)]) c |= 0x1UL;
      if ((0x400UL & c)) c = c^0x5B9UL;
   } /* end for */
   c = c^xor;
   for (i = 0UL; i<=9UL; i++) {
      if (X2C_IN(i,32,c)!=b[((offset+25UL)-i)%((b_len-1)+1UL)]) return 0;
   } /* end for */
   return 1;
} /* end crc() */


static void trailblanks(char s[], uint32_t s_len)
{
   uint32_t i;
   i = s_len-1;
   while (i>0UL && (uint8_t)s[i]<=' ') {
      s[i] = 0;
      --i;
   }
} /* end trailblanks() */

static uint32_t radio_OFFSETXOR[5] = {252UL,408UL,360UL,436UL,848UL};


static void frame(void)
{
   uint32_t ab;
   uint32_t dectyp;
   uint32_t textadr;
   uint32_t a;
   uint32_t n;
   MSG msg;
   char good[4];
   char pr[256];
   char tmp;
   strncpy(good,"----",4u);
   msg[0] = 0;
   dectyp = 16UL;
   if (crc(bitb, 104ul, synoffset, 0xFCUL)) {
      /* A */
      good[0U] = '+';
      n = getbits(bitb, 104ul, synoffset, 16UL);
      countrycode = n/4096UL;
   }
   if (crc(bitb, 104ul, synoffset+26UL, 0x198UL)) {
      /* B */
      good[1U] = '+';
      n = getbits(bitb, 104ul, synoffset+26UL, 16UL);
      /*    IntToStr(n DIV 2048, 2, h); Werr(h); */
      /*    IntToStr(n MOD 32, 2, h); Werr(h); Werr(" "); */
      progtyp(n/32UL&31UL, progtext, 64ul);
      dectyp = n/4096UL;
      ab = n/2048UL&1UL;
      textadr = n&15UL;
      if (dectyp==2UL) {
         a = n/16UL&1UL;
         if (a!=textAB) {
            memcpy(msg,radiotext,64u);
            memset((char *)radiotext,(char)' ',64UL);
            textAB = a;
         }
      }
   }
   if (crc(bitb, 104ul, synoffset+52UL, 0x168UL)) {
      /* C */
      good[2U] = '+';
      n = getbits(bitb, 104ul, synoffset+52UL, 16UL);
      if (dectyp==2UL) {
         if (ab==0UL) {
            radiotext[textadr*4UL] = chkchr(n/256UL);
            radiotext[textadr*4UL+1UL] = chkchr(n&255UL);
         }
      }
   }
   else if (crc(bitb, 104ul, synoffset+52UL, 0x350UL)) {
      /* c */
      good[2U] = '!';
      n = getbits(bitb, 104ul, synoffset+52UL, 16UL);
   }
   if (crc(bitb, 104ul, synoffset+78UL, 0x1B4UL)) {
      /* D */
      good[3U] = '+';
      n = getbits(bitb, 104ul, synoffset+78UL, 16UL);
      if (dectyp==2UL) {
         if (ab==0UL) {
            radiotext[textadr*4UL+2UL] = chkchr(n/256UL);
            radiotext[textadr*4UL+3UL] = chkchr(n&255UL);
         }
         else {
            radiotext[textadr*2UL] = chkchr(n/256UL);
            radiotext[textadr*2UL+1UL] = chkchr(n&255UL);
         }
      }
      else if (dectyp==0UL) {
         progname[(textadr&3UL)*2UL] = chkchr(n/256UL);
         progname[(textadr&3UL)*2UL+1UL] = chkchr(n&255UL);
      }
   }
   pr[0] = 0;
   if (verb) showrssi(pr, 256ul);
   trailblanks(progname, 8ul);
   trailblanks(radiotext, 64ul);
   aprsstr_Append(pr, 256ul, good, 4ul);
   aprsstr_Append(pr, 256ul, progtext, 64ul);
   aprsstr_Append(pr, 256ul, ":", 2ul);
   aprsstr_Append(pr, 256ul, progname, 8ul);
   aprsstr_Append(pr, 256ul, " ", 2ul);
   aprsstr_Append(pr, 256ul, (char *)(tmp = (char)(66UL-textAB),
                &tmp), 1u/1u);
   aprsstr_Append(pr, 256ul, ":", 2ul);
   aprsstr_Append(pr, 256ul, radiotext, 64ul);
   aprsstr_Append(pr, 256ul, "\015", 2ul);
   osi_Werr(pr, 256ul);
   n = aprsstr_Length(pr, 256ul);
   if (n>maxlen) maxlen = n;
   if (msg[0]) {
      trailblanks(msg, 64ul);
      strncpy(pr," [",256u);
      pr[0U] = (char)(65UL+textAB);
      aprsstr_Append(pr, 256ul, msg, 64ul);
      aprsstr_Append(pr, 256ul, "]", 2ul);
      a = aprsstr_Length(pr, 256ul);
      while (a<maxlen) {
         pr[a] = ' '; /* blank rest of last line */
         ++a;
      }
      pr[a] = 0;
      osi_WerrLn(pr, 256ul);
      maxlen = 0UL;
   }
} /* end frame() */


static void stobit(char b)
{
   bitm[bitc] = bitm[bitc]+((float)(uint32_t)(b==bitb[bitc])-bitm[bitc])
                *0.1f;
   bitb[bitc] = b;
   if (bitm[bitc]>0.8f) {
      ++samec;
      if (samec==26UL) synoffset = (bitc+79UL)%104UL;
   }
   else samec = 0UL;
   if (bitc==synoffset) frame();
   ++bitc;
   if (bitc>103UL) bitc = 0UL;
} /* end stobit() */

#define radio_PLLSPEED 2.E-7

#define radio_LP 0.0252


static void decoderds(float v)
{
   float uq;
   float ui;
   int32_t m;
   char d;
   ui = lowpass(v*osic_sin((float)rdsclk), 0.0252f, &lpi);
                /* not phaselinear not good for data */
   uq = lowpass(v*osic_cos((float)rdsclk), 0.0252f, &lpq);
   rdsclk = rdsclk+1.865255188222+(double)(ui*uq*2.E-7f);
                /* simple costas loop */
   m = 0L;
   if (rdsclk>1.5079644737231E+2) {
      rdsclk = rdsclk-3.0159289474462E+2; /* rds clock pll */
      d = sint>0.0f;
      stobit(d!=nrzi);
      nrzi = d;
      sint = 0.0f;
   }
   if (uio<0.0f!=ui<0.0f) {
      if (rdsclk>=0.0) rdsclk = rdsclk-6.2831853071796;
      else rdsclk = rdsclk+6.2831853071796;
   }
   uio = ui;
   if (rdsclk<0.0) sint = sint-ui;
   else sint = sint+ui;
/*
  testv:=m+VAL(INTEGER, ui*100.0);
  IF testv>32000 THEN testv:=32000 ELSIF testv<-32000 THEN testv:=-32000 END;
  testb[testp]:=testv;
  INC(testp);
  IF testf<=0 THEN testf:=OpenWrite("/tmp/rds") END;
  IF testp>HIGH(testb) THEN WrBin(testf, testb, SIZE(testb)); testp:=0 END;
*/
} /* end decoderds() */

/*----------------rds */
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

static uint32_t downs;

static float left;

static float right;

static uint32_t tshow;

static char recon;

static struct LOWPASSCONTEXT lpleft;

static struct LOWPASSCONTEXT lpright;

static float deem1;

static float deem2;

static char vs[31];


static void wwav(void)
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
   b[20U] = '\001'; /* PCM */
   b[21U] = 0;
   b[22U] = '\002'; /* 2ch */
   b[23U] = 0;
   b[24U] = '\200'; /* samp */
   b[25U] = '\273';
   b[26U] = 0;
   b[27U] = 0;
   b[28U] = 0; /* byte/s */
   b[29U] = '\356';
   b[30U] = '\002';
   b[31U] = 0;
   b[32U] = '\004'; /* block byte */
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
   osi_WrBin(fd, (char *)b, 44u/1u, 44UL);
} /* end wwav() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   int32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(MSG)!=64) X2C_ASSERT(0);
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
      if (ifwidth) anonym->width = ifwidth*1000UL;
      else if (mono) anonym->width = 96000UL;
      else anonym->width = 192000UL;
      anonym->modulation = 'f';
      anonym->samples = (sdr_pAUDIOSAMPLE)sampx;
   }
   if (sdr_startsdr(url, 1001ul, port, 1001ul, 2048000UL, 192000UL, 1)) {
      sdr_setparm(1UL, tune-100000UL);
      for (pc = 0UL; pc<=255UL; pc++) {
         if (stickparm[pc].ok0) sdr_setparm(pc, stickparm[pc].val);
      } /* end for */
      sndw = 0UL;
      fd = -1L;
      if (soundfn[0U]) {
         fd = osi_OpenWrite(soundfn, 1001ul);
         if (fd>=0L) wwav();
         else {
            osi_Werr(soundfn, 1001ul);
            osi_WerrLn(" sound file open error", 23ul);
         }
      }
      else osi_WerrLn("no sound output", 16ul);
      recon = 1;
      downs = 0UL;
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
               left = (float)X2C_CHKNIL(sdr_pAUDIOSAMPLE,rxx.samples)[sp];
               if (rds) decoderds(left);
               if (fd>=0L) {
                  if (mono) {
                     left = lowpass(left, 0.14f, &lpleft);
                     deem1 = deem1+(left-deem1)*0.05f; /* deemphasis */
                     deem2 = deem1;
                  }
                  else {
                     stereo(left, &left, &right);
                     left = lowpass(left, 0.14f, &lpleft);
                     right = lowpass(right, 0.14f, &lpright);
                     deem1 = deem1+(left-deem1)*0.05f; /* deemphasis */
                     deem2 = deem2+(right-deem2)*0.05f;
                  }
                  if (downs>=4UL) {
                     /* 192 > 48khz samperate */
                     downs = 0UL;
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
                           if (verb && !rds) {
                              vs[0] = 0;
                              showrssi(vs, 31ul);
                              aprsstr_Append(vs, 31ul, "   \015", 5ul);
                              osi_Werr(vs, 31ul);
                           }
                           tshow = 0UL;
                        }
                        else ++tshow;
                     }
                  }
                  ++downs;
               }
               if (sp==tmp) break;
            } /* end for */
         }
      }
   }
   /*    WrStrLn("connection lost") */
   osi_WerrLn("not connected", 14ul);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
