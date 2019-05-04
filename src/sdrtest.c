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
#define sdrtest_C_
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
#ifndef sdr_H_
#include "sdr.h"
#endif

/* test rtl_tcp iq fm demodulator by OE5DXL */
#define sdrtest_MAXCHANNELS 64

#define sdrtest_DEFAULTPOWERSAVE 0

#define sdrtest_SAMPSIZE 32
/* samples per sdr call */

#define sdrtest_TUNESTEP 1000
/* minimal tuning step */

#define sdrtest_TUNEBAND 200000
/* step to minimize freq jumps from inexact pll */

#define sdrtest_DEFAULTMAXWAKE 2000
/* ms stay awake after squelch close */

#define sdrtest_DEFAULTOFFSET 10000
/* shift tune frequency to where is no rx if possible */

#define sdrtest_NL "\012"

#define sdrtest_cFM "F"

#define sdrtest_cAM "A"

#define sdrtest_cUSB "U"

#define sdrtest_cLSB "L"

#define sdrtest_cSCAN "S"

#define sdrtest_SCANHEADER 12

#define sdrtest_MAXSCANSLOTS 1400

#define sdrtest_SOUNDBUF 1024

enum CODEC {sdrtest_S16, sdrtest_U8, sdrtest_ALAW};


struct FREQTAB;


struct FREQTAB {
   uint32_t hz;
   uint32_t width;
   uint32_t afc;
   uint32_t agc;
   char modulation;
};

struct STICKPARM;


struct STICKPARM {
   uint32_t val;
   char ok0;
   char changed;
};

struct SQUELCH;


struct SQUELCH {
   float lev;
   float lp;
   float u1;
   float u2;
   float il;
   float medmed;
   float mutlev;
   float peakrssi;
   int32_t sqsave;
   int32_t wakeness;
   int32_t pcmc;
   int32_t lastpcm;
   uint32_t hz10;
   uint32_t nexttick;
   double scanlo;
   double scanstep;
   uint32_t scanslot;
   uint32_t scanslots;
   int32_t scanduty;
   char scandb[1412];
   char scansq[1412];
   uint32_t waterp;
   uint32_t waterend;
   uint8_t waterdat[35];
};

struct UDPTX;

typedef struct UDPTX * pUDPTX;


struct UDPTX {
   pUDPTX next;
   uint32_t ip;
   uint32_t destport;
};

static int32_t soundfd;

static uint32_t sndw;

static uint32_t freqc;

static uint32_t midfreq;

static uint32_t lastmidfreq;

static uint32_t downsamp;

static uint32_t mixto;

static uint32_t powersave;

static uint32_t soundbufsize;

static short sndbuf[1024];

static uint8_t sndbuf8[1024];

static struct sdr_RX rxx[64];

static sdr_pRX prx[65];

static short sampx[64][32];

static struct STICKPARM stickparm[256];

static struct SQUELCH squelchs[65];

static uint32_t iqrate;

static uint32_t samphz;

static uint32_t maxrx;

static uint32_t maxwake;

static uint32_t tuneoffset;

static uint32_t tuneto;

static char url[1001];

static char port[1001];

static char soundfn[1001];

static char parmfn[1001];

static char verb;

static char reconn;

static char nosquelch;

static char watermark;

static uint8_t pcm8;

static double offset;

static pUDPTX udplev;

static pUDPTX udpsq;

static pUDPTX udpsounds;

static int32_t udpsendfd;

static int32_t cfgfd;

static uint32_t cfgip;

static uint32_t CRC32TAB[256];

static uint8_t ALAWTAB[4096];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_WrStr(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void G711TAB(void)
{
   uint32_t j;
   uint32_t i;
   for (i = 0UL; i<=2047UL; i++) {
      if (i<32UL) j = i;
      else if (i<64UL) j = 16UL+i/2UL;
      else if (i<128UL) j = 32UL+i/4UL;
      else if (i<256UL) j = 48UL+i/8UL;
      else if (i<512UL) j = 64UL+i/16UL;
      else if (i<1024UL) j = 80UL+i/32UL;
      else j = 96UL+i/64UL;
      j = (uint32_t)(uint8_t)((uint8_t)j^0x55U);
      ALAWTAB[2048UL-i] = (uint8_t)j;
      ALAWTAB[2048UL+i] = (uint8_t)(j+128UL);
   } /* end for */
   ALAWTAB[0U] = ALAWTAB[1U];
} /* end G711TAB() */


static int32_t GetIp(char h[], uint32_t h_len, uint32_t * ip,
                uint32_t * port0)
{
   uint32_t p;
   uint32_t n;
   uint32_t i;
   char ok0;
   int32_t GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
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
      if (i<3UL) {
         if (h[p]!='.' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[p]!=':' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port0 = n;
      ++p;
   } /* end for */
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */

static uint32_t sdrtest_POLY = 0x4C11DB7UL;


static void GenCRC32tab(void)
{
   uint32_t j;
   uint32_t i;
   uint32_t crc;
   uint32_t revpoly;
   revpoly = 0UL;
   for (i = 0UL; i<=31UL; i++) {
      if (X2C_IN(i,32,0x4C11DB7UL)) revpoly |= (1UL<<31UL-i);
   } /* end for */
   for (i = 0UL; i<=255UL; i++) {
      crc = i;
      for (j = 0UL; j<=7UL; j++) {
         if ((crc&1)) crc = (uint32_t)((uint32_t)(crc/2UL)^revpoly);
         else crc = crc/2UL;
      } /* end for */
      CRC32TAB[255UL-i] = (uint32_t)crc^0xFF000000UL;
   } /* end for */
} /* end GenCRC32tab() */


static void card(const char s[], uint32_t s_len, uint32_t * p,
                uint32_t * n, char * ok0)
{
   *ok0 = 0;
   *n = 0UL;
   while ((uint8_t)s[*p]>='0' && (uint8_t)s[*p]<='9') {
      *n = ( *n*10UL+(uint32_t)(uint8_t)s[*p])-48UL;
      *ok0 = 1;
      ++*p;
   }
} /* end card() */


static void sendudp(const char data[], uint32_t data_len, int32_t len,
                 pUDPTX pu)
{
   int32_t i;
   char * pc;
   pc = (char *)data;
   while (pu) {
      i = udpsend(udpsendfd, pc, len, pu->destport, pu->ip);
      pu = pu->next;
   }
} /* end sendudp() */


static char dBbyte(float v)
{
   v = osic_ln((v+1.0f)*9.5367431640625E-7f)*8.685889638f; /* 0.5 dB step */
   if (v<0.0f) v = 0.0f;
   else if (v>255.9f) v = 255.0f;
   return (char)(uint32_t)X2C_TRUNCC(v,0UL,X2C_max_longcard);
} /* end dBbyte() */


static char sqbyte(float s, float n)
{
   if (s>n) {
      return (char)(uint32_t)X2C_TRUNCC(X2C_DIVR(255.0f*n,s),0UL,
                X2C_max_longcard);
   }
   return '\377';
} /* end sqbyte() */


static void int0(const char s[], uint32_t s_len, uint32_t * p,
                int32_t * n, char * ok0)
{
   char sgn;
   uint32_t c;
   if (s[*p]=='-') {
      sgn = 1;
      ++*p;
   }
   else sgn = 0;
   card(s, s_len, p, &c, ok0);
   if (sgn) *n = -(int32_t)c;
   else *n = (int32_t)c;
} /* end int() */


static void fix(const char s[], uint32_t s_len, uint32_t * p,
                double * x, char * ok0)
{
   double m;
   char sgn;
   if (s[*p]=='-') {
      sgn = 1;
      ++*p;
   }
   else sgn = 0;
   m = 1.0;
   *ok0 = 0;
   *x = 0.0;
   while ((uint8_t)s[*p]>='0' && (uint8_t)s[*p]<='9' || s[*p]=='.') {
      if (s[*p]=='.') m = 0.1;
      else if (m==1.0) {
         *x =  *x*10.0+(double)((uint32_t)(uint8_t)s[*p]-48UL);
      }
      else {
         *x = *x+(double)((uint32_t)(uint8_t)s[*p]-48UL)*m;
         m = m*0.1;
      }
      *ok0 = 1;
      ++*p;
   }
   if (sgn) *x = -*x;
} /* end fix() */


static pUDPTX openudptx(const char s[], uint32_t s_len,
                int32_t * fd)
{
   pUDPTX utx;
   uint32_t ip;
   uint32_t port0;
   if (GetIp(s, s_len, &ip, &port0)>=0L) {
      if (*fd<0L) *fd = openudp();
      if (*fd<0L) Error("cannot open udp send socket", 28ul);
   }
   else return 0;
   osic_alloc((char * *) &utx, sizeof(struct UDPTX));
   if (utx==0) Error("out of memory", 14ul);
   utx->ip = ip;
   utx->destport = port0;
   utx->next = 0;
   return utx;
} /* end openudptx() */


static void Parms(void)
{
   char s[1001];
   double fr;
   uint32_t n;
   uint32_t m;
   int32_t ni;
   char ok0;
   pUDPTX utx;
   uint32_t cfgport;
   tuneoffset = 10000UL;
   tuneto = 0UL;
   reconn = 0;
   verb = 0;
   pcm8 = sdrtest_S16;
   mixto = 0UL;
   strncpy(url,"127.0.0.1",1001u);
   strncpy(port,"1234",1001u);
   soundfn[0] = 0;
   iqrate = 2048000UL;
   samphz = 16000UL;
   offset = 0.0;
   strncpy(parmfn,"sdrcfg.txt",1001u);
   powersave = 0UL;
   maxrx = 63UL;
   nosquelch = 0;
   watermark = 0;
   maxwake = 2000UL;
   udplev = 0;
   udpsq = 0;
   udpsounds = 0;
   downsamp = 0UL;
   udpsendfd = -1L;
   for (;;) {
      osi_NextArg(s, 1001ul);
      if (s[0U]==0) break;
      if ((s[0U]=='-' && s[1U]) && s[2U]==0) {
         if ((s[1U]=='s' || s[1U]=='S') || s[1U]=='A') {
            if (s[1U]=='S') pcm8 = sdrtest_U8;
            else if (s[1U]=='A') pcm8 = sdrtest_ALAW;
            osi_NextArg(soundfn, 1001ul);
            utx = openudptx(soundfn, 1001ul, &udpsendfd);
            if (utx) {
               /* send sound via udp */
               soundfn[0U] = 0;
               utx->next = udpsounds;
               udpsounds = utx;
            }
         }
         else if (s[1U]=='c') {
            osi_NextArg(s, 1001ul);
            parmfn[0] = 0;
            if (GetIp(s, 1001ul, &cfgip, &cfgport)>=0L) {
               cfgfd = openudp();
               if (cfgfd>=0L && bindudp(cfgfd, cfgport)<0L) {
                  Error("-c cannot bind inport", 22ul);
               }
               ni = socknonblock(cfgfd);
            }
            else {
               cfgfd = -1L;
               aprsstr_Assign(parmfn, 1001ul, s, 1001ul);
            }
         }
         else if (s[1U]=='o') {
            /* offset */
            osi_NextArg(s, 1001ul);
            n = 0UL;
            fix(s, 1001ul, &n, &offset, &ok0);
            if (!ok0) Error(" -o <MHz>", 10ul);
         }
         else if (s[1U]=='T') {
            /* tune manual */
            osi_NextArg(s, 1001ul);
            n = 0UL;
            fix(s, 1001ul, &n, &fr, &ok0);
            fr = fr*1.E+6+0.5;
            if (!ok0 || fr>4.294967295E+9) Error(" -T <MHz>", 10ul);
            tuneto = (uint32_t)X2C_TRUNCC(fr,0UL,X2C_max_longcard);
         }
         else if (s[1U]=='d') {
            /* sampelrate to output divide */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &downsamp)) {
               Error(" -d <outputsampelrate>", 23ul);
            }
         }
         else if (s[1U]=='m') {
            /* downmix to */
            osi_NextArg(s, 1001ul);
            if ((!aprsstr_StrToCard(s, 1001ul,
                &mixto) || mixto<1UL) || mixto>2UL) {
               Error(" -m <1..2>", 11ul);
            }
         }
         else if (s[1U]=='a') {
            /* maximal active rx */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &maxrx)) {
               Error(" -a <number>", 13ul);
            }
         }
         else if (s[1U]=='B') {
            /* size of sound samples sent once */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul,
                &soundbufsize) || soundbufsize>1024UL) {
               Error(" -B <number>", 13ul);
            }
            soundbufsize = (soundbufsize/2UL)*2UL; /* make even for stereo */
         }
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
         else if (s[1U]=='r') {
            /* sampelrate */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &samphz) || samphz>192000UL) {
               Error(" -r <Hz>", 9ul);
            }
         }
         else if (s[1U]=='O') {
            /* sampelrate */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul,
                &tuneoffset) || tuneoffset>25000000UL) {
               Error(" -O <Hz>", 9ul);
            }
         }
         else if (s[1U]=='i') {
            /* iq sampelrate */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul,
                &iqrate) || iqrate!=1024000UL && (iqrate<2048000UL || iqrate>2500000UL)
                ) Error(" -i <Hz> 2048000 or 1024000", 28ul);
         }
         else if (s[1U]=='v') verb = 1;
         else if (s[1U]=='k') reconn = 1;
         else if (s[1U]=='e') watermark = 1;
         else if (s[1U]=='w') {
            /* maximum wake time */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &maxwake)) {
               Error(" -w <ms>", 9ul);
            }
         }
         else if (s[1U]=='z') {
            /* powersave time */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &powersave)) {
               Error(" -z <ms>", 9ul);
            }
            nosquelch = 0;
         }
         else if (s[1U]=='Z') {
            /* powersave time */
            osi_NextArg(s, 1001ul);
            if (!aprsstr_StrToCard(s, 1001ul, &powersave)) {
               Error(" -Z <ms>", 9ul);
            }
            nosquelch = 1;
         }
         else if (s[1U]=='L') {
            /* udp send scanners dBs */
            osi_NextArg(s, 1001ul);
            utx = openudptx(s, 1001ul, &udpsendfd);
            if (utx==0) Error("-L ip:port", 11ul);
            /*
                    ALLOCATE(utx, SIZE(utx^));
                    IF utx=NIL THEN Error("udp socket out of memory") END;
            
                    IF GetIp(s, utx^.ip, utx^.destport)>=0 THEN
                      IF udpsendfd<0 THEN udpsendfd:=udp.openudp() END;
                      IF udpsendfd<0 THEN Error("cannot open udp send socket"
                ) END;
                    ELSE Error("-L ip:port") END;
            */
            utx->next = udplev;
            udplev = utx;
         }
         else if (s[1U]=='N') {
            /* udp send scanners squelchs */
            osi_NextArg(s, 1001ul);
            utx = openudptx(s, 1001ul, &udpsendfd);
            if (utx==0) Error("-N ip:port", 11ul);
            utx->next = udpsq;
            udpsq = utx;
         }
         else {
            if (s[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("AM/FM/SSB Multirx from rtl_tcp (8 bit IQ via tcpi\
p or file) to audio channel(s) 8/16 bit PCM by oe5dxl", 103ul);
               osi_WrStrLn(" -A <soundfilename> or <ip:port> ALAW (G.711) n-c\
hannel sound stream/pipe/UDP", 78ul);
               osi_WrStrLn(" -a <number>         maximum active rx to limit c\
pu load, if number is reached,", 80ul);
               osi_WrStrLn("                       no more inactive rx will l\
isten to become active", 72ul);
               osi_WrStrLn(" -B <samples>        sound samples sent at once. \
With UDP take care: double number of bytes with 16bit PCM", 107ul);
               osi_WrStrLn(" -c <configfilename> read channels config from fi\
le (sdrcfg.txt)", 65ul);
               osi_WrStrLn(" -c <ip:port>        read channels config from UD\
P, if ip=0.0.0.0 aaccept any ip", 81ul);
               osi_WrStrLn(" -d <Hz>             downsample output to Hz",
                45ul);
               osi_WrStrLn(" -e                  enable sending SDR Data hidd\
en in audio channels (tune/afc/rssi..)", 88ul);
               osi_WrStrLn(" -h                  help", 26ul);
               osi_WrStrLn(" -i <Hz>             input sampelrate Hz 1024000 \
or 2048000..2500000 (2048000)", 79ul);
               osi_WrStrLn("                       if >2048000, AM/FM-IF-widt\
h will increase proportional", 78ul);
               osi_WrStrLn(" -k                  keep connection", 37ul);
               osi_WrStrLn(" -L <x.x.x.x:destport> send Level table in UDP to\
 Waterfall Viewers (may be repeatet)", 86ul);
               osi_WrStrLn(" -m <audiochannels>  mix up/down all rx channels \
to 1 or 2 audiochannels (mono/stereo)", 87ul);
               osi_WrStrLn("                      for 2 channels the rx audio\
s will be arranged from left to right", 87ul);
               osi_WrStrLn(" -N <x.x.x.x:destport> send Noise (Squelch) table\
 in UDP to Scanner Scripts (may be repeatet)", 94ul);
               osi_WrStrLn(" -O <Hz>             moves center freq. away from\
 used band to avoid ADC-DC-offset noise (10000)", 97ul);
               osi_WrStrLn(" -o <mhz>            offset for entered frequenci\
es if Converters are used", 75ul);
               osi_WrStrLn(" -p <cmd> <value>    send rtl_tcp parameter, ppm,\
 tunergain ...", 64ul);
               osi_WrStrLn(" -r <Hz>             output sampelrate Hz for all\
 channels 8000..192000 (16000)", 80ul);
               osi_WrStrLn("                       for FM min. 25% more than \
rx IF-width", 61ul);
               osi_WrStrLn(" -s <soundfilename> or <ip:port> 16bit signed n-c\
hannel sound stream/pipe/UDP", 78ul);
               osi_WrStrLn(" -S <soundfilename> or <ip:port> 8bit unsigned n-\
channel sound stream/pipe/UDP", 79ul);
               osi_WrStrLn(" -T <mhz>            Tune manual to center of iq-\
band (for iq-data from file) (0)", 82ul);
               osi_WrStrLn(" -t <url:port>       connect rtl_tcp server (127.\
0.0.1:1234)", 61ul);
               osi_WrStrLn(" -t <filename:0>     read iq-data from file",
                44ul);
               osi_WrStrLn(" -v                  show rssi (dB) and afc (khz)\
", 50ul);
               osi_WrStrLn(" -w <ms>             max stay awake (use CPU) tim\
e after squelch close (2000)", 78ul);
               osi_WrStrLn(" -z <ms>             sleep time (no cpu) for inac\
tive rx if squelch closed (-z 100)", 84ul);
               osi_WrStrLn(" -Z <ms>             same but fast open with no a\
udio quieting for sending", 75ul);
               osi_WrStrLn("                      to decoders and not human e\
ars", 53ul);
               osi_WrStrLn("example: ./sdrtst -k -s /dev/stdout -t 127.0.0.1:\
1234 -c up.txt -i 2048000 -r 16000 -m 2 -v | sox -t raw -r 16000 -c 2 -b 16 -\
s - -t alsa", 138ul);
               osi_WrStrLn("         will mix up/down any channels to stereo \
and play on alsa", 66ul);
               osi_WrStrLn("         ./sdrtst -k -s /dev/stdout -t 127.0.0.1:\
1234 -c up.txt -i 2048000 -r 16000 -v | ./afskmodem -o /dev/stdin -s 16000 -c\
 2 -M 0 -c 0 -M 1 -c 1", 149ul);
               osi_WrStrLn("         with 2 frequencies in up.txt will listen\
 to PR/APRS on 2 channels", 75ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("config file: (re-read every some seconds and may \
be modified any time)", 71ul);
               osi_WrStrLn("  # comment", 12ul);
               osi_WrStrLn("  p <cmd> <value>  rtl_tcp parameter like \'p 5 5\
0\' ppm, \'p 8 1\' autogain on", 76ul);
               osi_WrStrLn("  f <mhz> <AFC-range> <squelch%> <lowpass%>  <IF-\
width>  FM Demodulator", 72ul);
               osi_WrStrLn("  a <mhz>  0          <squelch%> <lowpass%>  <IF-\
width>  AM Demodulator", 72ul);
               osi_WrStrLn("  u <mhz> <IF-shift>   0         <agc speed> <IF-\
width>  USB Demodulator", 73ul);
               osi_WrStrLn("  l same for LSB", 17ul);
               osi_WrStrLn("    AFC-range in +-kHz, Squelch 0 off, 100 open, \
70 may do", 59ul);
               osi_WrStrLn("    audio lowpass in % Nyquist frequ. of output s\
ampelrate, 0 is off", 69ul);
               osi_WrStrLn("    IF-width 3000 6000 12000 24000 48000 96000 19\
2000Hz for low CPU usage", 74ul);
               osi_WrStrLn("    (192000 only with >=2048khz iq-rate), (4th or\
der IIR)", 58ul);
               osi_WrStrLn("    (SSB 8th order IF-IIR), OTHER values with MOR\
E CPU-load (12000 default)", 76ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("  example:", 11ul);
               osi_WrStrLn("    p 5 50", 11ul);
               osi_WrStrLn("    p 8 1", 10ul);
               osi_WrStrLn("    f 438.825   5   75 70         (afc, quelch, a\
udio lowpass, 12khz IF)", 73ul);
               osi_WrStrLn("    f 439.275   0   0  80 20000   (20khz IF, uses\
 more CPU)", 60ul);
               osi_WrStrLn("    u 439.5001 -700 0  0  600     (USB with 600Hz\
 CW-Filter at 800Hz", 69ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("  will generate 3 channel 16bit PCM stream (up to\
 64 channels with -z or -Z)", 77ul);
               osi_WrStrLn("  use max. 95% of -i span. rtl-stick will be tune\
d to center of the span", 73ul);
               osi_WrStrLn("  rx in center of band will be +-10khz relocated \
to avoid ADC-DC offset pseudo", 79ul);
               osi_WrStrLn("  carriers, SSB-only will be relocated 10..210khz\
 to avoid inexact tuning steps", 80ul);
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("    f 100.1 0 0 15 96000          (WFM with \"-r \
192000 -d 44100\" for 1 channnel 44100hz", 88ul);
               osi_WrStrLn("", 1ul);
               X2C_ABORT();
            }
            if (s[1U]=='p') {
               osi_NextArg(s, 1001ul);
               if (aprsstr_StrToCard(s, 1001ul, &m) && m<256UL) {
                  osi_NextArg(s, 1001ul);
                  if (aprsstr_StrToInt(s, 1001ul, &ni)) {
                     stickparm[m].val = (uint32_t)ni; /* stick parameter */
                     stickparm[m].ok0 = 1;
                     stickparm[m].changed = 1;
                  }
                  else Error(" -p <cmd> <value>", 18ul);
               }
               else Error(" -p <cmd> <value>", 18ul);
            }
            else Error("-h", 3ul);
         }
      }
      else Error("-h", 3ul);
   }
   powersave = (powersave*samphz)/32000UL;
   maxwake = (maxwake*samphz)/32000UL;
   if (downsamp==0UL) downsamp = samphz;
   if (downsamp>samphz) Error(" -d <hz> must be less than -r <hz>", 35ul);
} /* end Parms() */


static void setparms(char all)
{
   uint32_t i;
   char nl;
   char s[31];
   nl = 1;
   for (i = 0UL; i<=255UL; i++) {
      if (stickparm[i].ok0 && (all || stickparm[i].changed)) {
         sdr_setparm(i, stickparm[i].val);
         if (verb) {
            if (nl) {
               osi_WerrLn("", 1ul);
               nl = 0;
            }
            osi_Werr("parm:", 6ul);
            aprsstr_IntToStr((int32_t)i, 0UL, s, 31ul);
            osi_Werr(s, 31ul);
            osi_Werr(" ", 2ul);
            aprsstr_IntToStr((int32_t)stickparm[i].val, 0UL, s, 31ul);
            osi_WerrLn(s, 31ul);
         }
      }
      stickparm[i].changed = 0;
   } /* end for */
} /* end setparms() */


static void setstickparm(uint32_t cmd, uint32_t value)
{
   if (!stickparm[cmd].ok0 || stickparm[cmd].val!=value) {
      stickparm[cmd].val = value;
      stickparm[cmd].changed = 1;
   }
   stickparm[cmd].ok0 = 1;
} /* end setstickparm() */


static void card32str(char b[], uint32_t b_len, uint32_t pos,
                uint32_t v)
{
   b[pos+0UL] = (char)(v&255UL);
   b[pos+1UL] = (char)(v/256UL&255UL);
   b[pos+2UL] = (char)(v/65536UL&255UL);
   b[pos+3UL] = (char)(v/16777216UL&255UL);
} /* end card32str() */


static void centerfreq(const struct FREQTAB freq[], uint32_t freq_len)
{
   uint32_t max0;
   uint32_t min0;
   uint32_t i;
   int32_t nomid;
   char ssb;
   double rem;
   double fhzo;
   double fhz;
   double khz;
   midfreq = 0UL;
   i = 0UL;
   max0 = 0UL;
   min0 = X2C_max_longcard;
   while (i<freqc) {
      /*WrStr("rx"); WrInt(i+1, 0); WrInt(freq[i].khz, 0); WrStrLn("kHz"); */
      if (freq[i].hz>max0) max0 = freq[i].hz;
      if (freq[i].hz<min0) min0 = freq[i].hz;
      if (freq[i].modulation=='S') {
         if (freq[i].afc>max0) max0 = freq[i].afc;
         if (freq[i].afc<min0) min0 = freq[i].afc;
      }
      ++i;
   }
   if (max0>=min0) {
      if (max0-min0>=iqrate) osi_WerrLn("freq span > iq-sampelrate", 26ul);
      if (tuneto>0UL) {
         /* user defined tune freq */
         midfreq = tuneto;
         if (max0>midfreq && (max0-midfreq)
                *2UL>=iqrate || min0<midfreq && (midfreq-min0)*2UL>=iqrate) {
            osi_WerrLn("tuned to outside iq-band", 25ul);
         }
      }
      else {
         /* find best tune freq + offset */
         midfreq = (max0+min0)/2UL;
         nomid = X2C_max_longint;
         i = 0UL;
         ssb = 0;
         while (i<freqc) {
            if (freq[i].modulation!='S') {
               if (labs((int32_t)(freq[i].hz-midfreq))<labs(nomid)) {
                  nomid = (int32_t)(freq[i].hz-midfreq);
               }
               if (freq[i].modulation=='s') ssb = 1;
            }
            ++i;
         }
         if ((uint32_t)labs(nomid)>tuneoffset) nomid = 0L;
         else if (nomid<0L) nomid = (int32_t)tuneoffset+nomid;
         else nomid -= (int32_t)tuneoffset;
         midfreq += (uint32_t)nomid;
         if (ssb && max0-min0<200000UL) {
            midfreq = (midfreq/200000UL)*200000UL+tuneoffset;
         }
         else midfreq = (midfreq/1000UL)*1000UL;
      }
      i = 0UL;
      while (i<freqc) {
         /*    FILL(ADR(rxx[i]), 0C, SIZE(rxx[0])); */
         squelchs[i].hz10 = (uint32_t)((int32_t)(freq[i].hz/10UL)
                -(int32_t)X2C_TRUNCI(offset*1.E+5,X2C_min_longint,
                X2C_max_longint));
         prx[i] = &rxx[i];
         rxx[i].squelch = squelchs[i].lev!=0.0f;
         rxx[i].modulation = freq[i].modulation;
         khz = 1.0;
         if (iqrate>2048000UL) khz = X2C_DIVL(2.048E+6,(double)iqrate);
         fhz = (double)((int32_t)freq[i].hz-(int32_t)midfreq)*khz;
         if (freq[i].modulation!='S') {
            rxx[i].df = (uint32_t)((int32_t)X2C_TRUNCI(fhz,
                X2C_min_longint,X2C_max_longint)/1000L);
            rem = fhz-(double)(((int32_t)X2C_TRUNCI(fhz,
                X2C_min_longint,X2C_max_longint)/1000L)*1000L);
            rxx[i].dffrac = (uint32_t)X2C_TRUNCC(X2C_DIVL(rem,khz)+0.5,0UL,
                X2C_max_longcard);
            /*WrInt(nomid, 15);WrInt(midfreq, 15);WrInt(freq[i].hz, 15);
                WrInt(rxx[i].df, 15); WrInt(rxx[i].dffrac, 15);
                WrStrLn("n m h d fr"); */
            rxx[i].maxafc = (int32_t)freq[i].afc;
            rxx[i].agc = freq[i].agc;
         }
         else {
            /*scanner */
            fhzo = (double)((int32_t)freq[i].afc-(int32_t)midfreq)
                *khz;
            squelchs[i].scanlo = fhz; /* scan from */
            rxx[i].dffrac = 0UL;
            squelchs[i].scanslot = 0UL;
            squelchs[i].scanslots = (uint32_t)X2C_TRUNCC(X2C_DIVL(fhzo-fhz,
                squelchs[i].scanstep),0UL,X2C_max_longcard);
            if (squelchs[i].scanslots>1400UL) squelchs[i].scanslots = 1400UL;
            card32str(squelchs[i].scandb, 1412ul, 4UL, freq[i].hz/1000UL);
            card32str(squelchs[i].scandb, 1412ul, 8UL,
                (uint32_t)X2C_TRUNCC(squelchs[i].scanstep,0UL,
                X2C_max_longcard));
            strncpy(squelchs[i].scandb,"SDR",1412u);
            X2C_MOVE((char *)squelchs[i].scandb,
                (char *)squelchs[i].scansq,12UL);
            squelchs[i].scandb[3U] = 'L';
            squelchs[i].scansq[3U] = 'N';
         }
         /*scanner */
         rxx[i].width = freq[i].width;
         ++i;
      }
      prx[i] = 0;
   }
   if (midfreq<500000UL) osi_WerrLn("no valid frequency", 19ul);
   else if (midfreq!=lastmidfreq) {
      setstickparm(1UL, midfreq);
      /*WrStr("set ");WrInt(midfreq, 0); WrStrLn("kHz"); */
      lastmidfreq = midfreq;
   }
} /* end centerfreq() */


static void skip(const char s[], uint32_t s_len, uint32_t * p)
{
   while (s[*p] && s[*p]==' ') ++*p;
} /* end skip() */


static void setconfig(const char b[], uint32_t b_len, int32_t len)
{
   uint32_t sq;
   uint32_t wid;
   uint32_t lpp;
   uint32_t p;
   uint32_t lino;
   uint32_t n;
   uint32_t i;
   int32_t ssbsh;
   int32_t m;
   double y;
   double x;
   char ok0;
   char li[256];
   struct FREQTAB freq[64];
   char mo;
   /*
     IF cfgfd>=0 THEN                                                (* await cfg via udp *)
       len:=udp.udpreceive(cfgfd, b, SIZE(b), ckport, ckip);
       IF len<=0 THEN RETURN END; 
   
       IF (cfgip<>0) & (cfgip<>ckip)
                THEN                            (* ip check on *)
         IF verb THEN WrStrLn("got config udp from wrong ip") END;
         RETURN
   
       END;
       freqc:=0;
     ELSE      
       freqc:=0;
       fd:=OpenRead(parmfn);
       IF fd<0 THEN Error("config file not readable") END;
   
       len:=RdBin(fd, b, SIZE(b));
       Close(fd);
     END;
   */
   freqc = 0UL;
   /*  IF (len>0) & (len<HIGH(b)) & (b[len-1]>=" ") THEN b[len-1]:=0C END; */
   p = 0UL;
   lino = 1UL;
   i = 0UL;
   while ((int32_t)p<=len) {
      if ((int32_t)p<len && (uint8_t)b[p]>=' ') {
         if (i<255UL) {
            li[i] = b[p];
            ++i;
         }
      }
      else if (i>0UL) {
         li[i] = 0;
         i = 0UL;
         skip(li, 256ul, &i);
         mo = X2C_CAP(li[i]);
         if (mo=='P') {
            ++i;
            skip(li, 256ul, &i);
            card(li, 256ul, &i, &n, &ok0);
            if (n>255UL || !ok0) osi_WerrLn("wrong parameter number", 23ul);
            else {
               skip(li, 256ul, &i);
               int0(li, 256ul, &i, &m, &ok0);
               if (!ok0) osi_WerrLn("wrong value", 12ul);
               else setstickparm(n, (uint32_t)m);
            }
            i = 0UL;
         }
         else if ((((mo=='F' || mo=='A') || mo=='U') || mo=='L') || mo=='S') {
            if (mo=='A') freq[freqc].modulation = 'a';
            else if (mo=='U') freq[freqc].modulation = 's';
            else if (mo=='L') freq[freqc].modulation = 's';
            else if (mo=='S') {
               /*scanner */
               freq[freqc].modulation = 'S';
            }
            else {
               /*scanner */
               freq[freqc].modulation = 'f';
            }
            ++i;
            skip(li, 256ul, &i);
            fix(li, 256ul, &i, &x, &ok0);
            if (!ok0) osi_WerrLn("wrong MHz", 10ul);
            skip(li, 256ul, &i);
            ssbsh = 0L;
            /*scanner */
            if (mo=='S') {
               fix(li, 256ul, &i, &y, &ok0);
               if (!ok0 || y<x) {
                  osi_WerrLn("scan from..TO MHz", 18ul);
                  y = x;
               }
            }
            else {
               /*scanner */
               y = 0.0;
               int0(li, 256ul, &i, &m, &ok0);
               if (!ok0) m = 0L;
               if (mo=='U') {
                  m += 1500L;
                  ssbsh = m;
               }
               else if (mo=='L') {
                  m -= 1500L;
                  ssbsh = m;
               }
            }
            skip(li, 256ul, &i);
            card(li, 256ul, &i, &sq, &ok0); /* or scan step hz */
            if (!ok0) sq = 0UL;
            if (mo!='S' && sq>200UL) sq = 200UL;
            skip(li, 256ul, &i);
            card(li, 256ul, &i, &lpp, &ok0); /* or scan duty */
            if (!ok0) lpp = 0UL;
            if (freq[freqc].modulation!='s' && lpp>100UL) lpp = 100UL;
            skip(li, 256ul, &i);
            card(li, 256ul, &i, &wid, &ok0);
            if (!ok0) {
               if (freq[freqc].modulation=='s') wid = 2800UL;
               else if (freq[freqc].modulation=='a') wid = 6000UL;
               else wid = 12000UL;
            }
            if (wid>1000000UL) wid = 1000000UL;
            if (freqc>63UL) osi_WerrLn("freq table full", 16ul);
            else {
               x = x+offset;
               if (x<=0.0 || x>=2.147483E+6) {
                  osi_WerrLn("freq out of range", 18ul);
                  x = 0.0;
               }
               x = x*1.E+6+(double)ssbsh;
               freq[freqc].hz = (uint32_t)X2C_TRUNCC(x+0.5,0UL,
                X2C_max_longcard);
               /*scanner */
               if (mo=='S') {
                  y = y+offset;
                  if (y<=0.0 || y>=2.147483E+6) {
                     osi_WerrLn("scan to freq out of range", 26ul);
                     y = 0.0;
                  }
                  freq[freqc].afc = (uint32_t)X2C_TRUNCC(y*1.E+6+0.5,0UL,
                X2C_max_longcard);
                  if (sq<1000UL) sq = 1000UL;
                  squelchs[freqc].scanstep = (double)sq;
                /* scan step */
                  squelchs[freqc].scanduty = (int32_t)lpp; /* scan duty */
               }
               else {
                  /*scanner */
                  freq[freqc].afc = (uint32_t)m;
                  squelchs[freqc].lev = X2C_DIVR((float)sq*32.0f,200.0f);
                  if (freq[freqc].modulation=='s') squelchs[freqc].lp = 0.0f;
                  else {
                     lpp = (lpp*downsamp)/samphz;
                     squelchs[freqc].lp = X2C_DIVR((float)lpp,200.0f);
                  }
               }
               freq[freqc].agc = lpp;
               freq[freqc].width = wid;
               ++freqc;
            }
            i = 0UL;
         }
         else if (mo=='#' || (uint8_t)mo<=' ') i = 0UL;
         else osi_WerrLn("unkown command", 15ul);
         ++lino;
      }
      ++p;
   }
   /*  Close(fd); */
   /*ELSE Error("config file not readable") END;   */
   centerfreq(freq, 64ul);
} /* end setconfig() */


static void showrssi(void)
{
   uint32_t j;
   uint32_t i;
   char s[31];
   i = 0UL;
   while (prx[i]) {
      if (prx[i]->modulation!='S') {
         j = prx[i]->idx;
         aprsstr_FixToStr(osic_ln((rxx[j].rssi+1.0f)*3.0517578125E-5f)
                *4.342944819f, 2UL, s, 31ul);
         osi_Werr(s, 31ul);
         if (squelchs[j].sqsave<=0L) osi_Werr("db", 3ul);
         else osi_Werr("dB", 3ul);
         /*
         IF NOT nosquelch & rxx[j].squelch THEN
             Werr(" "); FixToStr(squelchs[j].medmed*(1.0/SAMPSIZE), 3, s);
                Werr(s);
         END;
         */
         if (rxx[j].modulation=='f') {
            osi_Werr(" ", 2ul);
            aprsstr_IntToStr(rxx[j].afckhz, 0UL, s, 31ul);
            osi_Werr(s, 31ul);
         }
         osi_Werr(" ", 2ul);
      }
      /*Werr(" "); IntToStr(squelchs[j].wakeness, 0, s); Werr(s); Werr(" ");
                */
      ++i;
   }
   osi_Werr("    \015", 6ul);
   osic_flush();
} /* end showrssi() */

/*------ watermark data */
#define sdrtest_CRCINIT 0x0FFFFFFFF


static uint32_t crc32(const uint8_t b[], uint32_t b_len,
                uint32_t from, uint32_t to)
{
   uint32_t crc;
   crc = 0xFFFFFFFFUL;
   while (from<to) {
      crc = X2C_LSH(crc,32,-8)^CRC32TAB[(uint32_t)((crc^(uint32_t)b[from])&0xFFUL)];
      ++from;
   }
   return (uint32_t)crc;
} /* end crc32() */


static void card32send(uint8_t b[], uint32_t b_len, uint32_t pos,
                uint32_t v)
{
   b[pos] = (uint8_t)v;
   b[pos+1UL] = (uint8_t)(v/256UL);
   b[pos+2UL] = (uint8_t)(v/65536UL);
   b[pos+3UL] = (uint8_t)(v/16777216UL);
} /* end card32send() */

#define sdrtest_WBSTART 0xA5 

#define sdrtest_WWSTART 0x0CA55F047

#define sdrtest_WBCRC 0x7F 


static void sendwater(void)
{
   uint32_t p;
   uint32_t j;
   uint32_t i;
   int32_t v;
   struct SQUELCH * anonym;
   i = 0UL;
   while (prx[i]) {
      if (prx[i]->modulation!='S') {
         j = prx[i]->idx;
         { /* with */
            struct SQUELCH * anonym = &squelchs[j];
            anonym->waterp = 0UL;
            anonym->waterdat[0U] = 0xA5U;
            card32send(anonym->waterdat, 35ul, 1UL, 0x0CA55F047UL);
            p = 5UL;
            anonym->waterdat[5U] = *(uint8_t *)"f";
            card32send(anonym->waterdat, 35ul, 6UL, squelchs[j].hz10);
            p += 5UL;
            anonym->waterdat[10U] = *(uint8_t *)"a";
            card32send(anonym->waterdat, 35ul, 11UL,
                (uint32_t)(X2C_LSH((uint32_t)rxx[j].maxafc,32,
                16)|(uint32_t)rxx[j].afckhz&0xFFFFUL));
            p += 5UL;
            anonym->waterdat[15U] = *(uint8_t *)"r";
            /*      v:=VAL(INTEGER,
                ln((rxx[j].rssi+1.0)*(1.0/32768.0))*(8.685889638*5.0)); */
            v = (int32_t)X2C_TRUNCI(osic_ln((anonym->peakrssi+0.001f)
                *3.0517578125E-5f)*4.342944819E+1f,X2C_min_longint,
                X2C_max_longint);
            anonym->peakrssi = 0.0f;
            if (v<0L) v = 0L;
            card32send(anonym->waterdat, 35ul, 16UL, (uint32_t)v);
            p += 5UL;
            anonym->waterdat[20U] = 0x7FU;
            card32send(anonym->waterdat, 35ul, 21UL, crc32(anonym->waterdat,
                35ul, 5UL, 20UL));
            p += 5UL;
            anonym->waterend = 200UL;
         }
      }
      ++i;
   }
} /* end sendwater() */

/*------ watermark data */
static int32_t sp;

static int32_t sn;

static uint32_t rp;

static uint32_t actch;

static uint32_t ticker;

static uint32_t ix;

static float ri;

static uint32_t tshow;

static uint32_t dsamp;

static char recon;

static int32_t pcm;

static int32_t mixleft;

static int32_t mixright;

static int32_t levdiv2;


static void sendaudio(int32_t pcm0, uint8_t codec, uint32_t ch)
{
   /* with start pattern and crc32 in dynamic count of 8+32bit type+value word units */
   struct SQUELCH * anonym;
   if (pcm0>32767L) pcm0 = 32767L;
   else if (pcm0<-32767L) pcm0 = -32767L;
   if (codec==sdrtest_U8) {
      sndbuf8[sndw] = (uint8_t)((uint32_t)(pcm0+32768L)/256UL);
   }
   else if (codec==sdrtest_ALAW) {
      sndbuf8[sndw] = ALAWTAB[(uint32_t)(pcm0+32768L)/16UL];
   }
   else {
      /* code data as watermark, bit 1 of pcm serialized datablock with start sequence and crc32 */
      /* to be compatible with normal pcm code data as silent audio bit 1 of pcm serialized datablock */
      { /* with */
         struct SQUELCH * anonym = &squelchs[ch];
         if (anonym->waterp<anonym->waterend) {
            pcm0 = (int32_t)((uint32_t)pcm0&0xFFFCUL);
            if (X2C_IN(anonym->waterp&7UL,8,
                anonym->waterdat[anonym->waterp/8UL])) {
               pcm0 += 2L; /* bit 1 is serial data stream */
            }
            ++anonym->waterp;
         }
         else pcm0 = (int32_t)((uint32_t)pcm0&0xFFFCUL);
      }
      if (ch==0UL) ++pcm0;
      /* code data as watermark */
      sndbuf[sndw] = (short)pcm0;
   }
   ++sndw;
   if (sndw>=soundbufsize) {
      if (codec) {
         if (udpsounds) {
            sendudp((char *)sndbuf8, 1024u/1u, (int32_t)sndw,
                udpsounds);
         }
         else osi_WrBin(soundfd, (char *)sndbuf8, 1024u/1u, sndw);
      }
      else if (udpsounds) {
         sendudp((char *)sndbuf, 2048u/1u, (int32_t)(sndw*2UL),
                udpsounds);
      }
      else osi_WrBin(soundfd, (char *)sndbuf, 2048u/1u, sndw*2UL);
      sndw = 0UL;
   }
} /* end sendaudio() */


static void userio(char all)
{
   int32_t len;
   int32_t fd;
   char b[10001];
   uint32_t ckip;
   uint32_t ckport;
   len = 0L;
   if (cfgfd>=0L) {
      /* await cfg via udp */
      len = udpreceive(cfgfd, b, 10001L, &ckport, &ckip);
      if ((len>0L && cfgip) && cfgip!=ckip) {
         /* ip check on */
         if (verb) osi_WrStrLn("got config udp from wrong ip", 29ul);
         len = 0L; /* discard info */
      }
   }
   else if (all && parmfn[0U]) {
      fd = osi_OpenRead(parmfn, 1001ul);
      if (fd>=0L) {
         len = osi_RdBin(fd, (char *)b, 10001u/1u, 10001UL);
         osic_Close(fd);
      }
      else if (verb) osi_WrStrLn("config file not readable", 25ul);
   }
   if (len>0L) {
      setconfig(b, 10001ul, len);
      setparms(recon);
   }
   if (all) {
      recon = 0;
      if (verb) showrssi();
      if (watermark) sendwater();
   }
} /* end userio() */


static void schedule(void)
{
   uint32_t j;
   uint32_t i;
   i = 0UL;
   j = 0UL;
   while (j<freqc && i<maxrx) {
      /* first append wake rx */
      if (squelchs[j].wakeness<=0L) {
         /* rx awake */
         squelchs[j].nexttick = ticker;
         prx[i] = &rxx[j];
         ++i;
      }
      ++j;
   }
   j = 0UL;
   while (j<freqc && i<maxrx) {
      /* then sleeping rx until maxrx */
      if (squelchs[j].wakeness>0L && (int32_t)(ticker-squelchs[j].nexttick)
                >=0L) {
         /* rx run */
         squelchs[j].nexttick = ticker;
         prx[i] = &rxx[j];
         ++i;
      }
      ++j;
   }
   prx[i] = 0;
} /* end schedule() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   int32_t tmp;
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   sdr_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   midfreq = 0UL;
   lastmidfreq = 0UL;
   tshow = 0UL;
   levdiv2 = 4096L;
   soundfd = -1L;
   cfgfd = -1L;
   soundbufsize = 512UL;
   memset((char *)rxx,(char)0,sizeof(struct sdr_RX [64]));
   memset((char *)stickparm,(char)0,sizeof(struct STICKPARM [256]));
   memset((char *)squelchs,(char)0,sizeof(struct SQUELCH [65]));
   G711TAB();
   Parms();
   GenCRC32tab();
   actch = 0UL;
   ticker = 0UL;
   prx[0U] = 0;
   dsamp = 0UL;
   for (freqc = 0UL; freqc<=63UL; freqc++) {
      rxx[freqc].samples = (sdr_pAUDIOSAMPLE)sampx[freqc];
      rxx[freqc].idx = freqc;
   } /* end for */
   if (sdr_startsdr(url, 1001ul, port, 1001ul, iqrate, samphz, reconn)) {
      /*    rdconfig; */
      tshow = 0UL;
      sndw = 0UL;
      if (soundfn[0U]) soundfd = osi_OpenWrite(soundfn, 1001ul);
      if (soundfd>=0L || udpsounds) {
         recon = 1;
         for (;;) {
            if ((tshow&15UL)==0UL) userio(tshow==0UL);
            if (tshow==0UL) tshow = samphz/32UL;
            else --tshow;
            schedule();
            sn = sdr_getsdr(32UL, prx, 65ul);
            if (sn<0L) {
               if (verb) {
                  if (sn==-2L) {
                     osi_WerrLn("impossible sampelrate conversion", 33ul);
                  }
                  else osi_WerrLn("connection lost", 16ul);
               }
               recon = 1;
               if (!reconn) break;
            }
            else {
               rp = 0UL;
               while (prx[rp]) {
                  ix = prx[rp]->idx;
                  { /* with */
                     struct SQUELCH * anonym = &squelchs[ix];
                     /*- scanner */
                     if (prx[rp]->modulation=='S') {
                        anonym->wakeness = squelchs[ix].scanduty;
                        anonym->nexttick = ticker+(uint32_t)
                anonym->wakeness;
                        anonym->scandb[12UL+anonym->scanslot]
                = dBbyte(rxx[ix].rssi);
                        anonym->scansq[12UL+anonym->scanslot]
                = sqbyte(rxx[ix].rssi, rxx[ix].sqsum);
                        /*WrInt(rxx[ix].df, 1); WrStr(" "); */
                        /*WrFixed(ln((rxx[ix].rssi+1.0)*(1.0/32768.0/32.0))
                *(8.685889638*0.5), 1,0);WrStr("!"); */
                        /*
                        IF rxx[ix].df=50 THEN
                        WrInt(ORD(sqbyte(rxx[ix].rssi, rxx[ix].sqsum)),4);
                        WrInt(ORD(dBbyte(rxx[ix].rssi)),4);WrStrLn("");
                        END;
                        */
                        rxx[ix].df = (uint32_t)(int32_t)
                X2C_TRUNCI((anonym->scanlo+(double)
                anonym->scanslot*anonym->scanstep)*0.001,X2C_min_longint,
                X2C_max_longint);
                        ++anonym->scanslot;
                        if (anonym->scanslot>=anonym->scanslots) {
                           anonym->scanslot = 0UL;
                           sendudp((char *)anonym->scandb, 1412u/1u,
                (int32_t)(12UL+anonym->scanslots), udplev);
                           sendudp((char *)anonym->scansq, 1412u/1u,
                (int32_t)(12UL+anonym->scanslots), udpsq);
                        }
                     }
                     else if (anonym->lev==0.0f || prx[rp]->modulation=='s') {
                        /*- scanner */
                        squelchs[ix].mutlev = 1.0f;
                     }
                     else {
                        if (anonym->lev<rxx[ix].sqsum) {
                           /* noise */
                           if (anonym->sqsave>0L) {
                              --anonym->sqsave;
                           }
                           else {
                              if (anonym->wakeness<(int32_t)powersave) {
                                 ++anonym->wakeness;
                              }
                              anonym->nexttick = ticker+(uint32_t)
                anonym->wakeness;
                           }
                        }
                        else {
                           /* squelch open */
                           if (anonym->sqsave<(int32_t)maxwake) {
                              ++anonym->sqsave;
                           }
                           if (anonym->wakeness>-100L) {
                              --anonym->wakeness; /* max stay wake limit */
                           }
                        }
                        if (nosquelch || rxx[ix].modulation=='s') {
                           anonym->mutlev = 1.0f;
                        }
                        else {
                           anonym->medmed = anonym->medmed+(rxx[ix]
                .sqsum-anonym->medmed)*0.1f;
                           anonym->mutlev = (anonym->lev-anonym->medmed)
                *0.3125f;
                           if (anonym->mutlev<0.0f) {
                              anonym->mutlev = 0.0f;
                           }
                           else if (anonym->mutlev>1.0f) {
                              anonym->mutlev = 1.0f;
                           }
                        }
                     }
                     if (anonym->peakrssi<rxx[ix].rssi) {
                        anonym->peakrssi = rxx[ix].rssi;
                /* store rssi peak till readout */
                     }
                  }
                  rxx[ix].sqsum = 0.0f;
                  ++rp;
               }
               tmp = sn-1L;
               sp = 0L;
               if (sp<=tmp) for (;; sp++) {
                  rp = 0UL;
                  while (prx[rp]) {
                     ix = prx[rp]->idx;
                     { /* with */
                        struct SQUELCH * anonym0 = &squelchs[ix];
                        if (rxx[ix].modulation!='S') {
                           anonym0->lastpcm = anonym0->pcmc;
                           anonym0->pcmc = (int32_t)rxx[ix].samples[sp];
                /* save for interpolation */
                           if (!nosquelch) {
                              anonym0->pcmc = (int32_t)(short)
                X2C_TRUNCI((float)anonym0->pcmc*anonym0->mutlev,-32768,
                32767);
                           }
                           /* lowpass */
                           if (anonym0->lp!=0.0f) {
                              anonym0->u1 = anonym0->u1+(((float)
                (anonym0->pcmc*2L)-anonym0->u1)-anonym0->il)*anonym0->lp;
                              anonym0->u2 = anonym0->u2+(anonym0->il-anonym0->u2)
                *anonym0->lp;
                              anonym0->il = anonym0->il+(anonym0->u1-anonym0->u2)
                *anonym0->lp*2.0f;
                              anonym0->pcmc = (int32_t)
                X2C_TRUNCI(anonym0->u2,X2C_min_longint,X2C_max_longint);
                           }
                        }
                     }
                     /* lowpass */
                     ++rp;
                  }
                  dsamp += downsamp;
                  if (dsamp>=samphz) {
                     rp = 0UL;
                     while (rp<freqc) {
                        if (rxx[rp].modulation!='S') {
                           pcm = squelchs[rp].pcmc;
                           /* interpolate 2 samples */
                           if (dsamp>samphz) {
                              /* do sampel interpolating */
                              ri = X2C_DIVR((float)(dsamp-samphz),
                (float)downsamp);
                              pcm = (int32_t)X2C_TRUNCI((float)
                pcm*(1.0f-ri)+(float)squelchs[rp].lastpcm*ri,
                X2C_min_longint,X2C_max_longint);
                           }
                           /* interpolate 2 samples */
                           squelchs[rp].pcmc = 0L;
                           /* channel mixer */
                           if (mixto==1UL) mixleft += pcm;
                           else if (mixto==2UL) {
                              if (freqc<=1UL) {
                                 mixleft = pcm;
                                 mixright = pcm;
                              }
                              else {
                                 mixleft += pcm*(int32_t)rp;
                                 mixright += pcm*(int32_t)((freqc-rp)-1UL);
                              }
                           }
                           /* channel mixer */
                           if (mixto==0UL || rp==0UL) {
                              if (mixto==0UL || freqc==0UL) {
                                 sendaudio(pcm, pcm8, rp);
                              }
                              else {
                                 mixleft = mixleft*levdiv2>>12;
                                 sendaudio(mixleft, pcm8, rp);
                                 if (mixto==2UL) {
                                    mixright = mixright*levdiv2>>12;
                                    sendaudio(mixright, pcm8, rp);
                                 }
                                 if (labs(mixleft)>30000L || labs(mixright)
                >30000L) {
                                    /* mix channel peak level limiter */
                                    if (levdiv2>300L) {
                                    levdiv2 -= 64L; /* agc down fast */
                                    }
                                 }
                                 else if (levdiv2<4096L) {
                                    ++levdiv2; /* agc up */
                                 }
                                 mixleft = 0L;
                                 mixright = 0L;
                              }
                           }
                        }
                        ++rp;
                     }
                     dsamp -= samphz;
                  }
                  if (sp==tmp) break;
               } /* end for */
            }
            ++ticker;
         }
      }
      else {
         osi_Werr(soundfn, 1001ul);
         osi_WerrLn(" sound file open error", 23ul);
      }
      if (verb) osi_WerrLn("connection lost", 16ul);
   }
   else osi_WerrLn("not connected", 14ul);
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
