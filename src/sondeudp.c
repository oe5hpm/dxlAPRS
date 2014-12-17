/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* "@(#)sondeudp.c May 16  6:00:34 2014" */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define sondeudp_C_
#ifndef soundctl_H_
#include "soundctl.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#include <fcntl.h>
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef RealMath_H_
#include "RealMath.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif

/* demodulate RS92 sonde (2400bit/s manchester)
                and send as AXUDP by OE5DXL */
#define sondeudp_ADCBUFLEN 4096

#define sondeudp_BAUDSAMP 65536

#define sondeudp_PLLSHIFT 1024

#define sondeudp_AFIRLEN 512

#define sondeudp_AOVERSAMP 16
/*16*/

#define sondeudp_ASYNBITS 10

static char sondeudp_CALIBFRAME = 'e';

static char sondeudp_GPSFRAME = 'g';

static char sondeudp_UNKNOWN = 'h';

static char sondeudp_DATAFRAME = 'i';

typedef char FILENAME[1024];

enum CHANNELS {sondeudp_LEFT, sondeudp_RIGHT};


typedef float AFIRTAB[8192];

struct CHAN;


struct CHAN {
   long dds;
   long adcmax;
   long actmodem;
   float afir[512];
   long configequalizer;
   long baudfine;
   long pllshift;
   long manchestd;
   float tcnt;
   float noise;
   float bitlev;
   char cbit;
   char oldd;
   char plld;
   char data1;
   char lastmanch;
   unsigned long rxbyte;
   unsigned long rxbitc;
   unsigned long rxp;
   char rxbuf[256];
   AFIRTAB afirtab;
   long asynst[10];
   unsigned long ip;
   unsigned long destport;
   long udpfd;
};

static long soundfd;

static long debfd;

static char abortonsounderr;

static char verb;

static unsigned long getst;

static unsigned long afin;

static unsigned long soundbufs;

static unsigned long adcrate;

static unsigned long adcbuflen;

static unsigned long adcbytes;

static unsigned long fragmentsize;

static float SIN[32768];

static FILENAME soundfn;

static struct CHAN chan[2];

static unsigned char maxchannels;

static unsigned long demodbaud;

static unsigned long configbaud;


static void Error(char text[], unsigned long text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   InOut_WriteString(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void Hamming(float f[], unsigned long f_len)
{
   unsigned long i;
   unsigned long tmp;
   tmp = f_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      f[i] = f[i]*(0.54f+0.46f*RealMath_cos(3.141592f*(X2C_DIVR((float)i,
                (float)(1UL+(f_len-1))))));
      if (i==tmp) break;
   } /* end for */
} /* end Hamming() */


static void initafir(AFIRTAB atab, unsigned long F0, unsigned long F1,
                float eq)
{
   unsigned long f;
   unsigned long i;
   float t[4096];
   float f10;
   float f00;
   float e;
   unsigned long tmp;
   f00 = X2C_DIVR((float)(F0*512UL),(float)adcrate);
   f10 = X2C_DIVR((float)(F1*512UL),(float)adcrate);
   for (i = 0UL; i<=4095UL; i++) {
      t[i] = 0.0f;
   } /* end for */
   tmp = (unsigned long)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL;
   f = (unsigned long)X2C_TRUNCC(f00,0UL,X2C_max_longcard);
   if (f<=tmp) for (;; f++) {
      e = 1.0f+eq*((X2C_DIVR((float)f,X2C_DIVR((float)((F0+F1)*512UL),
                (float)adcrate)))*2.0f-1.0f);
      /*
          e:=1.0 + eq*(FLOAT(f)/FLOAT((F0+F1)*AFIRLEN DIV adcrate)*2.0-1.0);
      */
      if (e<0.0f) e = 0.0f;
      if (f==(unsigned long)X2C_TRUNCC(f00,0UL,X2C_max_longcard)) {
         e = e*(1.0f-(f00-(float)(unsigned long)X2C_TRUNCC(f00,0UL,
                X2C_max_longcard)));
      }
      if (f==(unsigned long)X2C_TRUNCC(f10,0UL,X2C_max_longcard)+1UL) {
         e = e*(f10-(float)(unsigned long)X2C_TRUNCC(f10,0UL,
                X2C_max_longcard));
      }
      /*
      IF eq<>0 THEN IO.WrFixed(e,2,2);IO.WrLn; END;
      */
      if (f==0UL) {
         for (i = 0UL; i<=4095UL; i++) {
            t[i] = t[i]+e*0.5f;
         } /* end for */
      }
      else {
         for (i = 0UL; i<=4095UL; i++) {
            t[i] = t[i]+e*RealMath_cos(X2C_DIVR(3.141592f*(float)(i*f),
                4096.0f));
         } /* end for */
      }
      if (f==tmp) break;
   } /* end for */
   Hamming(t, 4096ul);
   for (i = 0UL; i<=4095UL; i++) {
      atab[4095UL+i] = t[i];
      atab[4095UL-i] = t[i];
   } /* end for */
   if (F0>0UL) {
      /* make dc level zero */
      e = 0.0f;
      for (i = 0UL; i<=8191UL; i++) {
         e = e+atab[i];
      } /* end for */
      e = X2C_DIVR(e,8192.0f);
      for (i = 0UL; i<=8191UL; i++) {
         atab[i] = atab[i]-e;
      } /* end for */
   }
/*
IO.WrLn;
FOR i:=0 TO HIGH(atab) DO IO.WrFixed(atab[i], 2,8) END;
IO.WrLn;
*/
/*
IF eq<>0.0 THEN
debfd:=FIO.Create("/tmp/ta.raw");
FOR i:=0 TO HIGH(atab) DO f:=VAL(INTEGER, atab[i]*1000.0);
                FIO.WrBin(debfd,f,2) END;
FIO.Close(debfd);
END;
*/
} /* end initafir() */


static void OpenSound(void)
{
   long s;
   long i;
   soundfd = open(soundfn, 2L);
   if (soundfd>=0L) {
      i = samplesize(soundfd, 16UL); /* 8, 16 */
      i = channels(soundfd, (unsigned long)maxchannels+1UL); /* 1, 2  */
      i = setfragment(soundfd, fragmentsize); /* 2^bufsize * 65536*bufs*/
      if (i) {
         InOut_WriteString("sound setfragment returns ", 27ul);
         InOut_WriteInt(i, 1UL);
         osi_WrLn();
      }
      i = sampelrate(soundfd, adcrate); /* 8000..48000 */
      s = (long)getsampelrate(soundfd);
      if (s!=(long)adcrate) {
         InOut_WriteString("sound device returns ", 22ul);
         InOut_WriteInt(s, 1UL);
         osi_WrStrLn("Hz!", 4ul);
      }
   }
   else if (abortonsounderr) {
      InOut_WriteString(soundfn, 1024ul);
      Error(" open", 6ul);
   }
} /* end OpenSound() */


static long GetIp(char h[], unsigned long h_len, unsigned long * ip,
                unsigned long * port)
{
   unsigned long p;
   unsigned long n;
   unsigned long i;
   char ok0;
   long GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((unsigned char)h[p]>='0' && (unsigned char)h[p]<='9') {
         ok0 = 1;
         n = (n*10UL+(unsigned long)(unsigned char)h[p])-48UL;
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
      *port = n;
      ++p;
   } /* end for */
   GetIp_ret = openudp();
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static void Config(void)
{
   unsigned long i;
   unsigned char c;
   struct CHAN * anonym;
   for (c = sondeudp_LEFT;; c++) {
      { /* with */
         struct CHAN * anonym = &chan[c];
         configbaud = 4800UL;
         demodbaud = (2UL*configbaud*65536UL)/adcrate;
         initafir(anonym->afirtab, 300UL, 3600UL,
                X2C_DIVR((float)anonym->configequalizer,100.0f));
         anonym->baudfine = 0L;
         anonym->noise = 0.0f;
         anonym->bitlev = 0.0f;
         anonym->cbit = 0;
         anonym->rxp = 0UL;
         anonym->rxbitc = 0UL;
         anonym->manchestd = 0L;
         anonym->lastmanch = 0;
         anonym->rxbyte = 0UL;
         for (i = 0UL; i<=9UL; i++) {
            anonym->asynst[i] = 0L;
         } /* end for */
      }
      if (c==sondeudp_RIGHT) break;
   } /* end for */
} /* end Config() */


static void Parms(void)
{
   char err;
   FILENAME mixfn;
   FILENAME h1;
   FILENAME h;
   unsigned long cnum;
   long inum;
   unsigned char channel;
   struct CHAN * anonym;
   struct CHAN * anonym0;
   err = 0;
   abortonsounderr = 0;
   adcrate = 16000UL;
   adcbytes = 2UL;
   adcbuflen = 1024UL;
   fragmentsize = 11UL;
   maxchannels = sondeudp_LEFT;
   debfd = -1L;
   for (channel = sondeudp_LEFT;; channel++) {
      { /* with */
         struct CHAN * anonym = &chan[channel];
         anonym->configequalizer = 0L;
         anonym->pllshift = 1024L;
         anonym->udpfd = -1L;
      }
      if (channel==sondeudp_RIGHT) break;
   } /* end for */
   channel = sondeudp_LEFT;
   for (cnum = 0UL; cnum<=32767UL; cnum++) {
      SIN[cnum] = RealMath_cos(X2C_DIVR((float)cnum*2.0f*3.141592f,
                32768.0f));
   } /* end for */
   X2C_COPY("/dev/dsp",9ul,soundfn,1024u);
   X2C_COPY("/dev/mixer",11ul,mixfn,1024u);
   for (;;) {
      Lib_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='a') abortonsounderr = 1;
         else if (h[1U]=='c') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<1UL || cnum>2UL) Error("maxchannels 1..2", 17ul);
            maxchannels = (unsigned char)(cnum-1UL);
         }
         else if (h[1U]=='C') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>1UL) Error("channel 0 to 1", 15ul);
            channel = (unsigned char)cnum;
         }
         else if (h[1U]=='D') {
            Lib_NextArg(h1, 1024ul);
            debfd = creat(h1, 420L);
         }
         else if (h[1U]=='e') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &inum)) err = 1;
            if (labs(inum)>999L) Error("equalizer -999..999", 20ul);
            chan[channel].configequalizer = inum;
         }
         else if (h[1U]=='f') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum<8000UL || cnum>96000UL) {
               Error("sampelrate 8000..96000", 23ul);
            }
            adcrate = cnum;
         }
         else if (h[1U]=='l') {
            Lib_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &cnum)) err = 1;
            if (cnum>=16UL && cnum<=4096UL) adcbuflen = cnum;
            else Error("sound buffer out of range", 26ul);
         }
         else if (h[1U]=='o') Lib_NextArg(soundfn, 1024ul);
         else if (h[1U]=='u') {
            Lib_NextArg(h, 1024ul);
            { /* with */
               struct CHAN * anonym0 = &chan[channel];
               anonym0->udpfd = GetIp(h, 1024ul, &anonym0->ip,
                &anonym0->destport);
               if (anonym0->udpfd<0L) Error("cannot open udp socket", 23ul);
            }
         }
         else if (h[1U]=='v') verb = 1;
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("oss Mono/Stereo RS92-SGPA Sonde Demodulator to ra\
w 240 Byte Frames sent as UDP", 79ul);
               osi_WrStrLn("Stereo used for 2 Rx for 2 Sondes or 1 Sonde with\
 Antenna-Diversity", 68ul);
               osi_WrStrLn("Only start sequence check, no CRC or Reed Solomon\
", 50ul);
               osi_WrStrLn(" -a             abort on sounddevice error else r\
etry to open (USB audio...)", 77ul);
               osi_WrStrLn(" -c <num>       maxchannels (1) (1=mono, 2=stereo\
)", 51ul);
               osi_WrStrLn(" -C <num>       (0..1) channel parameters follow \
(repeat for each channel)", 75ul);
               osi_WrStrLn(" -D <filename>  write raw soundcard input data to\
 file or pipe", 63ul);
               osi_WrStrLn("                for debug or chaining demodulator\
s (equalizer diversity)", 73ul);
               osi_WrStrLn(" -e <num>       demod equalizer (0) 100=6db/oct h\
ighpass (-999..999)", 69ul);
               osi_WrStrLn("                -C <n> before -e sets channel num\
ber", 53ul);
               osi_WrStrLn(" -f <num>       adcrate (16000) (8000..96000)",
                46ul);
               osi_WrStrLn(" -l <num>       adcbuflen (256)", 32ul);
               osi_WrStrLn(" -o <filename>  oss devicename (/dev/dsp) or raw/\
wav audio file", 64ul);
               osi_WrStrLn(" -u <x.x.x.x:destport> send udp, -C <n> before se\
nds channels to extra ip", 74ul);
               osi_WrStrLn(" -h             help", 21ul);
               osi_WrStrLn(" -v             verbous, (CRC-checked Sondename)",
                 49ul);
               osi_WrStrLn("example: sondeudp -f 16000 -o /dev/dsp -c 2 -C 1 \
-e 50 -u 127.0.0.1:4000", 73ul);
               X2C_ABORT();
            }
            err = 1;
         }
      }
      else err = 1;
      if (err) break;
   }
   if (err) {
      InOut_WriteString(">", 2ul);
      InOut_WriteString(h, 1024ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
   Config();
   OpenSound();
} /* end Parms() */


static void sendudp(char data[], unsigned long data_len, long len,
                unsigned long ip, unsigned long destport, long udpfd)
{
   long i;
   X2C_PCOPY((void **)&data,data_len);
   i = udpsend(udpfd, data, len, destport, ip);
   X2C_PFREE(data);
} /* end sendudp() */


static void WrdB(long volt)
{
   if (volt>0L) {
      osi_WrFixed(RealMath_ln((float)volt)*8.685889638f-96.4f, 1L, 6UL);
      InOut_WriteString("dB", 3ul);
   }
} /* end WrdB() */


static void WrQ(float lev, float noise)
{
   if (lev>0.0f) {
      noise = X2C_DIVR(noise*200.0f,lev);
      if (noise>100.0f) noise = 100.0f;
      else if (noise<=0.0f) noise = 0.0f;
      InOut_WriteInt(100L-(long)X2C_TRUNCI(noise,X2C_min_longint,
                X2C_max_longint), 4UL);
      InOut_WriteString("%", 2ul);
   }
} /* end WrQ() */


static float Fir(unsigned long in, unsigned long sub, unsigned long step,
                float fir[], unsigned long fir_len, float firtab[],
                unsigned long firtab_len)
{
   float s;
   unsigned long i;
   s = 0.0f;
   i = sub;
   do {
      s = s+fir[in]*firtab[i];
      ++in;
      if (in>fir_len-1) in = 0UL;
      i += step;
   } while (i<=firtab_len-1);
   return s;
} /* end Fir() */

static unsigned short sondeudp_POLYNOM = 0x1021U;


static void decodeframe(unsigned char m)
{
   unsigned long ic;
   unsigned long len;
   unsigned long p;
   unsigned long j;
   unsigned short crc;
   unsigned char channel;
   struct CHAN * anonym;
   channel = m;
   if (chan[channel].udpfd<0L) {
      if (channel==sondeudp_LEFT) channel = sondeudp_RIGHT;
      else channel = sondeudp_LEFT;
   }
   { /* with */
      struct CHAN * anonym = &chan[channel];
      if (anonym->udpfd>=0L) {
         sendudp(chan[m].rxbuf, 256ul, 240L, anonym->ip, anonym->destport,
                anonym->udpfd);
      }
   }
   if (verb) {
      p = 6UL;
      if (chan[m].rxbuf[6U]=='e') {
         ++p;
         len = (unsigned long)(unsigned char)chan[m].rxbuf[7U]*2UL+2UL;
                /* +crc */
         ++p;
         j = 0UL;
         crc = 0xFFFFU;
         while (j<len && 8UL+j<240UL) {
            if (j+2UL<len) {
               for (ic = 0UL; ic<=7UL; ic++) {
                  if (((0x8000U & crc)!=0)!=X2C_IN(7UL-ic,8,
                (unsigned char)(unsigned char)chan[m].rxbuf[p+j])) {
                     crc = X2C_LSH(crc,16,1)^0x1021U;
                  }
                  else crc = X2C_LSH(crc,16,1);
               } /* end for */
            }
            ++j;
         }
         if ((8UL+len>240UL || (char)crc!=chan[m].rxbuf[(8UL+len)-2UL])
                || (char)X2C_LSH(crc,16,-8)!=chan[m].rxbuf[(8UL+len)-1UL]) {
            if (verb) InOut_WriteString("----  crc err ", 15ul);
         }
         else {
            InOut_WriteInt((long)((unsigned long)(unsigned char)
                chan[m].rxbuf[8U]+(unsigned long)(unsigned char)
                chan[m].rxbuf[9U]*256UL), 4UL);
            j = 2UL;
            while ((unsigned char)chan[m].rxbuf[8UL+j]>=' ') {
               InOut_WriteString((char *) &chan[m].rxbuf[8UL+j], 1u/1u);
               ++j;
            }
         }
         if (m>sondeudp_LEFT) InOut_WriteString("             ", 14ul);
         WrdB(chan[m].adcmax);
         WrQ(chan[m].bitlev, chan[m].noise);
         osi_WrStrLn("", 1ul);
      }
   }
} /* end decodeframe() */


static void stobyte(unsigned char m, char b)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      anonym->rxbuf[anonym->rxp] = b;
      if (anonym->rxp>=5UL || b=='*') ++anonym->rxp;
      else anonym->rxp = 0UL;
      if (anonym->rxp>=240UL) {
         anonym->rxp = 0UL;
         decodeframe(m);
      }
   }
} /* end stobyte() */


static void demodbyte(unsigned char m, char d)
{
   unsigned long maxi;
   unsigned long i;
   long max0;
   long n;
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      anonym->rxbyte = anonym->rxbyte/2UL+256UL*(unsigned long)d;
      max0 = 0L;
      maxi = 0UL;
      for (i = 0UL; i<=9UL; i++) {
         n = anonym->asynst[i]-anonym->asynst[(i+1UL)%10UL];
         if (labs(n)>labs(max0)) {
            max0 = n;
            maxi = i;
         }
      } /* end for */
      if (anonym->rxbitc==maxi) {
         if (max0<0L) {
            anonym->rxbyte = (unsigned long)((unsigned long)
                anonym->rxbyte^0xFFUL);
         }
         stobyte(m, (char)(anonym->rxbyte&255UL));
      }
      if (d) {
         anonym->asynst[anonym->rxbitc]
                += X2C_DIV(32767L-anonym->asynst[anonym->rxbitc],16L);
      }
      else {
         anonym->asynst[anonym->rxbitc]
                -= X2C_DIV(32767L+anonym->asynst[anonym->rxbitc],16L);
      }
      anonym->rxbitc = (anonym->rxbitc+1UL)%10UL;
   }
/*FOR i:=0 TO HIGH(asynst) DO WrInt(asynst[i], 8) END; WrStrLn(""); */
} /* end demodbyte() */


static void demodbit(unsigned char m, char d)
{
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->lastmanch==d) {
         anonym->manchestd += X2C_DIV(32767L-anonym->manchestd,16L);
      }
      anonym->lastmanch = d;
      anonym->manchestd = -anonym->manchestd;
      /*WrInt(manchestd,8); */
      if (anonym->manchestd>=0L) demodbyte(m, d);
   }
} /* end demodbit() */


static void demod(float u, unsigned char m)
{
   char d;
   struct CHAN * anonym;
   /*
     IF debfd>=0 THEN
       ui:=VAL(INTEGER, u*0.002);
       WrBin(debfd, ui, 2);
     END;
   */
   d = u>=0.0f;
   { /* with */
      struct CHAN * anonym = &chan[m];
      if (anonym->cbit) {
         demodbit(m, d);
         if (d!=anonym->oldd) {
            if (d==anonym->plld) anonym->baudfine += anonym->pllshift;
            else anonym->baudfine -= anonym->pllshift;
            anonym->oldd = d;
         }
         /*quality*/
         anonym->bitlev = anonym->bitlev+((float)fabs(u)-anonym->bitlev)
                *0.05f;
         anonym->noise = anonym->noise+((float)fabs((float)fabs(u)
                -anonym->bitlev)-anonym->noise)*0.05f;
      }
      else {
         /*quality*/
         anonym->plld = d;
      }
      anonym->cbit = !anonym->cbit;
   }
} /* end demod() */


static void Fsk(unsigned char m)
{
   float ff;
   long lim;
   struct CHAN * anonym;
   { /* with */
      struct CHAN * anonym = &chan[m];
      lim = (long)demodbaud;
      for (;;) {
         if (anonym->baudfine>=65536L) {
            anonym->baudfine -= 65536L;
            ff = Fir(afin, (unsigned long)X2C_DIV(anonym->baudfine&65535L,
                4096L), 16UL, anonym->afir, 512ul, anonym->afirtab, 8192ul);
            demod(ff, m);
         }
         anonym->baudfine += lim;
         lim = 0L;
         if (anonym->baudfine<131072L) break;
      }
   }
} /* end Fsk() */


static void getadc(void)
{
   short buf[4096];
   long minr;
   long maxr;
   long minl;
   long maxl;
   long sl;
   long i;
   long l;
   unsigned char c;
   l = osi_RdBin(soundfd, (char *)buf, 8192u/1u, adcbuflen*adcbytes);
   if (l<0L) {
      if (abortonsounderr) Error("Sounddevice Failure", 20ul);
      else {
         osi_Close(soundfd);
         Usleep(100000UL);
         OpenSound();
         return;
      }
   }
   if (debfd>=0L) osi_WrBin(debfd, (char *)buf, 8192u/1u, (unsigned long)l);
   l = (long)((unsigned long)l/adcbytes);
   for (c = sondeudp_LEFT;; c++) {
      chan[c].adcmax = chan[c].adcmax*15L>>4;
      if (c==sondeudp_RIGHT) break;
   } /* end for */
   maxl = -32768L;
   maxr = -32768L;
   minl = 32767L;
   minr = 32767L;
   i = 0L;
   while (i<l) {
      sl = (long)buf[i];
      chan[sondeudp_LEFT].afir[afin] = (float)sl;
      if (sl>maxl) maxl = sl;
      if (sl<minl) minl = sl;
      if (maxl-minl>chan[sondeudp_LEFT].adcmax) {
         chan[sondeudp_LEFT].adcmax = maxl-minl;
      }
      if (maxchannels>sondeudp_LEFT) {
         sl = (long)buf[i+1L];
         chan[sondeudp_RIGHT].afir[afin] = (float)sl;
         if (sl>maxr) maxr = sl;
         if (sl<minr) minr = sl;
         if (maxr-minr>chan[sondeudp_RIGHT].adcmax) {
            chan[sondeudp_RIGHT].adcmax = maxr-minr;
         }
      }
      afin = afin+1UL&511UL;
      Fsk(sondeudp_LEFT);
      if (maxchannels>sondeudp_LEFT) Fsk(sondeudp_RIGHT);
      i += (long)((unsigned long)maxchannels+1UL);
   }
} /* end getadc() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   Lib_BEGIN();
   RealMath_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   /*  Gencrctab; */
   Parms();
   getst = 0UL;
   afin = 0UL;
   soundbufs = 0UL;
   if (verb) osi_WrStrLn("Frame ID       level-L qual level-R qual", 41ul);
   for (;;) getadc();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
