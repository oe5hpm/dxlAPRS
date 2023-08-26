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
#define waterfall3_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef pngwritepalette_H_
#include "pngwritepalette.h"
#endif
#ifndef fftw_H_
#include "fftw.h"
#endif

/* make waterfall png out of IQ input by oe5dxl */
#define waterfall3_PI 3.1415926535

#define waterfall3_PI2 6.283185307

#define waterfall3_LF "\012"

#define waterfall3_CLOCKJUMP 60
/* max s clockjump to restart timeline */

#define waterfall3_PALETTELEN 256


struct IMAGELINE {
   uint8_t * Adr;
   size_t Len0;
};


struct IMAGE {
   struct IMAGELINE * * Adr;
   size_t Len0;
};

static struct IMAGE * image;

static char imagefn[1024];

static char tmpfn[1024];

static char iqfn[1024];

static pLINE blackline;

static uint32_t wintyp;

static uint32_t roll;

static uint32_t fifosetback;

static uint32_t iqsize;

static uint32_t mediancount;

static uint32_t oversample;

static uint32_t filejump;

static uint32_t margin;

static uint32_t xsize;

static uint32_t ysize;

static int32_t iqfd;

static char checkfloat;

static char withmedian;

static char medianpeak;

static char dcoffset;

static char transp;

static float cleanoverdrives;

/*    lut:ARRAY[0..255] OF RECORD r,g,b:CARD8 END; */
static pINOUT cp;


struct _0 {
   uint8_t * Adr;
   size_t Len0;
};

static struct _0 * inb;


struct _1 {
   short * Adr;
   size_t Len0;
};

static struct _1 * inbw;


struct _2 {
   float * Adr;
   size_t Len0;
};

static struct _2 * inbf;


struct _3 {
   float * Adr;
   size_t Len0;
};

static struct _3 * win;

static struct _3 * median;

static struct _3 * peak;

static char mirrory;

static float peaklev;

static float brightness;

static float zerolev;

static float blackpercent;

static struct PNGPALETTE palette[256];

static uint8_t transpaency[256];


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static void makelut(void)
{
   uint32_t i;
   struct PNGPALETTE * anonym;
   for (i = 0UL; i<=255UL; i++) {
      { /* with */
         struct PNGPALETTE * anonym = &palette[i];
         anonym->r = 0U;
         anonym->g = 0U;
         anonym->b = 0U;
         if (i==0UL && transp) transpaency[i] = 0U;
         else transpaency[i] = 255U;
         if (i<64UL) anonym->b = (uint8_t)(i*4UL);
         else if (i<128UL) {
            anonym->b = (uint8_t)(255UL-(i-64UL)*4UL);
            anonym->g = (uint8_t)((i-64UL)*4UL);
            anonym->r = anonym->g;
         }
         else if (i<192UL) {
            anonym->g = (uint8_t)(255UL-(i-128UL)*4UL);
            anonym->r = 255U;
         }
         else {
            anonym->r = 255U;
            anonym->g = (uint8_t)((i-192UL)*4UL);
            anonym->b = anonym->g;
         }
      }
   } /* end for */
} /* end makelut() */


static void card(const char s[], uint32_t s_len, uint32_t * p,
                uint8_t * res, uint32_t len, char * ok0)
{
   uint32_t n;
   *ok0 = 0;
   n = 0UL;
   while (*p<len && s[*p]==' ') ++*p;
   while ((*p<len && (uint8_t)s[*p]>='0') && (uint8_t)s[*p]<='9') {
      n = (n*10UL+(uint32_t)(uint8_t)s[*p])-48UL;
      *ok0 = 1;
      ++*p;
   }
   if (n>1023UL) *ok0 = 0;
   if (*ok0) *res = (uint8_t)n;
} /* end card() */


static void readlut(const char fn[], uint32_t fn_len)
{
   uint32_t lc;
   uint32_t p;
   uint32_t last;
   uint32_t next;
   uint32_t gap;
   uint32_t i;
   uint8_t idx;
   int32_t len;
   int32_t fd;
   char s[20001];
   char h[201];
   char ok0;
   float k;
   fd = osi_OpenRead(fn, fn_len);
   if (fd>=0L) {
      memset((char *)palette,(char)0,
                sizeof(struct PNGPALETTE [256]));
      len = osi_RdBin(fd, (char *)s, 20001u/1u, 20000UL);
      osic_Close(fd);
      if (len>=0L) s[len] = 0;
      lc = 1UL;
      p = 0UL;
      for (;;) {
         while ((int32_t)p<len && (uint8_t)s[p]<' ') ++p;
         if ((int32_t)p<len && s[p]!='#') {
            card(s, 20001ul, &p, &idx, (uint32_t)len, &ok0);
            if (idx>255U) {
               osi_Werr("colour index > 255\012", 20ul);
               break;
            }
            if (ok0) {
               card(s, 20001ul, &p, &palette[idx].r, (uint32_t)len, &ok0);
            }
            if (ok0) {
               card(s, 20001ul, &p, &palette[idx].g, (uint32_t)len, &ok0);
            }
            if (ok0) {
               card(s, 20001ul, &p, &palette[idx].b, (uint32_t)len, &ok0);
            }
            if (!ok0) {
               aprsstr_CardToStr(lc, 1UL, h, 201ul);
               aprsstr_Append(h, 201ul, ": colour file syntax error\012",
                28ul);
               osi_Werr(h, 201ul);
               break;
            }
         }
         while ((int32_t)p<len && (uint8_t)s[p]>=' ') ++p;
         ++lc;
         if ((int32_t)p>=len) break;
      }
      p = 0UL; /* interpolate missing colours */
      do {
         i = p+1UL;
         while (((i<=255UL && palette[i].r==0U) && palette[i].g==0U)
                && palette[i].b==0U) ++i;
         gap = (i-p)-1UL;
         if (gap>0UL) {
            /* holes */
            if (i<=255UL) next = i;
            else {
               next = p; /* continue same colour till end */
               i = 255UL;
            }
            last = p;
            do {
               ++p;
               k = X2C_DIVR((float)(p-last),(float)(gap+1UL));
               palette[p].r = (uint8_t)(uint32_t)X2C_TRUNCC((float)
                palette[last].r+((float)palette[next].r-(float)
                palette[last].r)*k,0UL,X2C_max_longcard);
               palette[p].g = (uint8_t)(uint32_t)X2C_TRUNCC((float)
                palette[last].g+((float)palette[next].g-(float)
                palette[last].g)*k,0UL,X2C_max_longcard);
               palette[p].b = (uint8_t)(uint32_t)X2C_TRUNCC((float)
                palette[last].b+((float)palette[next].b-(float)
                palette[last].b)*k,0UL,X2C_max_longcard);
            } while (p<i);
         }
         else ++p;
      } while (p<255UL);
   }
   else {
      /*FOR i:=0 TO 255 DO WrInt(palette[i].r,4);WrInt(palette[i].g,4);
                WrInt(palette[i].b,4); WrStrLn(""); END; */
      strncpy(s,"[",20001u);
      aprsstr_Append(s, 20001ul, fn, fn_len);
      aprsstr_Append(s, 20001ul, "] colour file not found\012", 25ul);
      osi_Werr(s, 20001ul);
   }
} /* end readlut() */


static void wrpng(uint32_t fromy, uint32_t xsize0, uint32_t ysize0,
                const char fn[], uint32_t fn_len)
{
   int32_t ret;
   uint32_t yy;
   uint32_t y1;
   pROWPOINTERS pimage;
   pPNGPALETTE ppalette;
   uint32_t palettedepth;
   uint32_t palettelen;
   pRNS trns;
   struct IMAGELINE * anonym;
   uint32_t tmp;
   /*WrStrLn("png"); */
   if (ysize0==0UL) {
      while (ysize0<=image->Len0-1 && image->Adr[ysize0]) ++ysize0;
   }
   if (ysize0==0UL) {
      osi_Werr("zero lines - no image write\012", 29ul);
      return;
   }
   palettelen = 256UL;
   ppalette = (pPNGPALETTE)palette;
   palettedepth = 8UL;
   trns = (pRNS)transpaency;
   osic_alloc((char * *) &pimage, sizeof(pLINE)*ysize0);
   if (pimage==0) {
      osi_Werr("png write out of memory\012", 25ul);
      return;
   }
   tmp = ysize0-1UL;
   y1 = 0UL;
   if (y1<=tmp) for (;; y1++) {
      if (mirrory) yy = (ysize0-1UL)-y1;
      else yy = y1;
      yy = (((ysize0-1UL)+fromy)-yy)%ysize0;
      if (yy>image->Len0-1 || image->Adr[yy]==0) pimage[y1] = blackline;
      else pimage[y1] = (pLINE)(anonym = image->Adr[yy],&anonym->Adr[0UL]);
      if (y1==tmp) break;
   } /* end for */
   ret = writepng(fn, pimage, xsize0, ysize0, ppalette, 256UL, 8UL, trns);
   osic_free((char * *) &pimage, sizeof(pLINE)*ysize0);
   if (ret<0L) osi_Werr("png write failed\012", 18ul);
} /* end wrpng() */


static void Parms(void)
{
   char err;
   char h[1024];
   err = 0;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='w') {
            osi_NextArg(imagefn, 1024ul);
            if (imagefn[0U]==0 || imagefn[0U]=='-') {
               Error("-i <imagefilename>", 19ul);
            }
         }
         else if (h[1U]=='x') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &xsize) || xsize<32UL) || xsize>=32000UL) {
               Error("-x <size>", 10ul);
            }
         }
         else if (h[1U]=='y') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToCard(h, 1024ul,
                &ysize) || ysize<1UL) || ysize>=32000UL) {
               Error("-y <size>", 10ul);
            }
         }
         else if (h[1U]=='i') {
            osi_NextArg(iqfn, 1024ul);
            if (iqfn[0U]==0 || iqfn[0U]=='-') Error("-i <iqfilename>", 16ul);
         }
         else if (h[1U]=='j') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &filejump)) {
               Error("-j <samples/ms>", 16ul);
            }
         }
         else if (h[1U]=='f') {
            osi_NextArg(h, 1024ul);
            if ((h[0U]=='i' && h[1U]=='1') && h[2U]=='6') iqsize = 4UL;
            else if (h[0U]=='u' && h[1U]=='8') iqsize = 2UL;
            else if ((h[0U]=='f' && h[1U]=='3') && h[2U]=='2') iqsize = 8UL;
            else Error("-f u8|i16|f32", 14ul);
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&brightness, h, 1024ul)) {
               Error("-b <brightness>", 16ul);
            }
         }
         else if (h[1U]=='O') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&cleanoverdrives, h, 1024ul)) {
               Error("-O <overdrivensamples>", 23ul);
            }
            cleanoverdrives = cleanoverdrives*0.01f;
         }
         else if (h[1U]=='z') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&zerolev, h, 1024ul)) {
               Error("-z <zerolev>", 13ul);
            }
         }
         else if (h[1U]=='Z') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&blackpercent, h, 1024ul)) {
               Error("-Z <blacklevel%>", 17ul);
            }
         }
         else if (h[1U]=='m') mirrory = 1;
         else if (h[1U]=='M') withmedian = 1;
         else if (h[1U]=='C') {
            medianpeak = 1;
            withmedian = 1;
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &roll)) {
               Error("-r <lines>", 11ul);
            }
         }
         else if (h[1U]=='t') transp = 1;
         else if (h[1U]=='d') dcoffset = 1;
         else if (h[1U]=='T') checkfloat = 1;
         else if (h[1U]=='o') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &oversample) || oversample<1UL) {
               Error("-o <oversample>", 16ul);
            }
         }
         else if (h[1U]=='W') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &wintyp)) Error("-W <n>", 7ul);
         }
         else if (h[1U]=='q') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &margin)) {
               Error("-q <pixel>", 11ul);
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            readlut(h, 1024ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn(" Make Waterfall image out of IQ-File", 37ul);
               osi_WrStrLn(" -b <brightness>    gain (15.0)", 32ul);
               osi_WrStrLn(" -C                 combine median and peak of \"\
-o\" lines", 58ul);
               osi_WrStrLn(" -c <file>          read colour lines \"<index> <\
r> <g> <b>\" (0..255) gaps interpolated", 87ul);
               osi_WrStrLn("                      for colours testimage start\
 without -i", 61ul);
               osi_WrStrLn(" -d                 ADC DC-offset filter on",
                44ul);
               osi_WrStrLn(" -f u8|i16|f32      IQ data format (invalid float\
 numbers will rise crash!", 75ul);
               osi_WrStrLn(" -h                 this", 25ul);
               osi_WrStrLn(" -i <file>          IQ-filename or pipe", 40ul);
               osi_WrStrLn(" -j <samples>       jump to next position in IQ-f\
ile in samples (default read all samples)", 91ul);
               osi_WrStrLn("                      to see samples attenuated b\
y window functions, set to less than \"-x\"", 91ul);
               osi_WrStrLn(" -M                 make median of \"-o\" lines b\
efore draw it, default peak-hold", 80ul);
               osi_WrStrLn(" -m                 mirror image vertical",
                42ul);
               osi_WrStrLn(" -O <%>             on <%> overdriven levels per \
image line switch on artefact filter (0.0)", 92ul);
               osi_WrStrLn(" -o <n>             oversample <n> times with pea\
k-hold before draw it (1)", 75ul);
               osi_WrStrLn(" -q <n>             ignore (dark) margin in auto \
blacklevel calculation (0)", 76ul);
               osi_WrStrLn(" -r <n>             roll-mode, write image every \
<n> new image lines (0=off)", 77ul);
               osi_WrStrLn(" -T                 test float input for \'nan\' \
or > 2^31 values an set to zero", 79ul);
               osi_WrStrLn(" -t                 unfilled part of image is tra\
nsparent", 58ul);
               osi_WrStrLn(" -W <n>             Window type: 0(default)=Black\
man, 1=Blackman-Harris, 2=Flat-Top", 84ul);
               osi_WrStrLn("                      3=Hamming, 4=Hann", 40ul);
               osi_WrStrLn(" -w <file>          waterfall image png", 40ul);
               osi_WrStrLn(" -x <pixel>         image x-size, prefer 2^n for \
no pixel interpolation", 72ul);
               osi_WrStrLn(" -y <pixel>         image y-size, if not -r defau\
lt as filesize", 64ul);
               osi_WrStrLn(" -Z <%>             threshold for blacklevel in %\
 of pixels/line below black (10)", 82ul);
               osi_WrStrLn(" -z <level>         manual noisefloor blacklevel,\
 0.0 for automatic (0.0)", 74ul);
               osi_WrStrLn("", 1ul);
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
} /* end Parms() */

/*--------- window functions */

static void blackman(float w[], uint32_t w_len, float lev)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   lev = X2C_DIVR(1.0f,lev);
   N = X2C_DIVR(6.283185307f,(float)((w_len-1)+1UL));
   tmp = w_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      w[i] = lev*((0.42f-0.5f*osic_cos(N*(float)i))
                +0.08f*osic_cos(N*2.0f*(float)i)); /* Blackmann */
      if (i==tmp) break;
   } /* end for */
} /* end blackman() */


static void blackmanharris(float w[], uint32_t w_len, float lev)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   lev = X2C_DIVR(1.0f,lev);
   N = X2C_DIVR(6.283185307f,(float)((w_len-1)+1UL));
   tmp = w_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      w[i] = lev*(((0.35875f-0.48829f*osic_cos(N*(float)i))
                +0.14128f*osic_cos(N*2.0f*(float)i))
                -0.01168f*osic_cos(N*3.0f*(float)i));
                /* Blackmann-Harris */
      if (i==tmp) break;
   } /* end for */
} /* end blackmanharris() */


static void flattop(float w[], uint32_t w_len, float lev)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   lev = X2C_DIVR(1.0f,lev);
   N = X2C_DIVR(6.283185307f,(float)((w_len-1)+1UL));
   tmp = w_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      w[i] = lev*((((1.0f-1.93f*osic_cos(N*(float)i))
                +1.29f*osic_cos(N*2.0f*(float)i))
                -0.388f*osic_cos(N*3.0f*(float)i))
                +0.028f*osic_cos(N*4.0f*(float)i)); /* Flat-Top */
      if (i==tmp) break;
   } /* end for */
} /* end flattop() */


static void hamming(float w[], uint32_t w_len, float lev)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   lev = X2C_DIVR(1.0f,lev);
   N = X2C_DIVR(6.283185307f,(float)((w_len-1)+1UL));
   tmp = w_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      w[i] = lev*(0.54f-0.46f*osic_cos(N*(float)i)); /* hamming */
      if (i==tmp) break;
   } /* end for */
} /* end hamming() */


static void hann(float w[], uint32_t w_len, float lev)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   lev = X2C_DIVR(1.0f,lev);
   N = X2C_DIVR(6.283185307f,(float)((w_len-1)+1UL));
   tmp = w_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      w[i] = lev*0.5f*(1.0f-osic_cos(N*(float)i)); /* hann */
      if (i==tmp) break;
   } /* end for */
} /* end hann() */


static void makewin(float w[], uint32_t w_len, float lev)
{
   if (wintyp==0UL) blackman(w, w_len, lev);
   else if (wintyp==1UL) blackmanharris(w, w_len, lev);
   else if (wintyp==2UL) flattop(w, w_len, lev);
   else if (wintyp==3UL) hamming(w, w_len, lev);
   else if (wintyp==4UL) hann(w, w_len, lev);
   else Error("-W <window-type>", 17ul);
} /* end makewin() */

/*------- window functions */

static void findoverdrives(pINOUT c, uint32_t len, float peak0,
                uint32_t * oc)
{
   uint32_t i;
   struct Complex * anonym;
   uint32_t tmp;
   *oc = 0UL;
   if (cleanoverdrives!=0.0f) {
      peak0 = peak0*0.98f; /* threshold */
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct Complex * anonym = &c[i];
            if ((float)fabs(anonym->Re)>=peak0 || (float)
                fabs(anonym->Im)>=peak0) ++*oc;
         }
         if (i==tmp) break;
      } /* end for */
      if (X2C_DIVR((float)*oc,(float)len)<cleanoverdrives) *oc = 0UL;
   }
} /* end findoverdrives() */


static void clrline(uint32_t y1)
{
   uint32_t x;
   struct IMAGELINE * anonym;
   uint32_t tmp;
   if (image->Adr[y1]) {
      tmp = xsize-1UL;
      x = 0UL;
      if (x<=tmp) for (;; x++) {
         *(anonym = image->Adr[y1],&anonym->Adr[x]) = 0U;
         if (x==tmp) break;
      } /* end for */
   }
} /* end clrline() */


static void clrb(float p[], uint32_t p_len)
{
   memset((char *)p,(char)0,p_len*sizeof(float));
} /* end clrb() */


static char rdpipe(char b[], uint32_t b_len, int32_t fd,
                uint32_t len, uint32_t setback)
{
   uint32_t rpos;
   int32_t res;
   char * p;
   if (setback>len) setback = len;
   rpos = 0UL;
   if (setback>0UL) {
      /* else read full len */
      rpos = len-setback;
      if (rpos>0UL) X2C_MOVE((char *) &b[setback],(char *)b,rpos);
   }
   do {
      p = (char *) &b[rpos];
      res = osi_RdBin(fd, p, 65536ul, len-rpos);
      if (res<=0L) return 0;
      /* eof */
      rpos += (uint32_t)res;
   } while (rpos<len);
   return 1;
} /* end rdpipe() */


static char dummyread(int32_t fd, uint32_t len)
/* jump in pipe */
{
   uint32_t i;
   int32_t res;
   char b[65536];
   while (len>0UL) {
      i = len;
      if (i>65536UL) i = 65536UL;
      res = osi_RdBin(fd, (char *)b, 65536u/1u, i);
      if (res<=0L) return 0;
      /* eof */
      len -= (uint32_t)res;
   }
   return 1;
} /* end dummyread() */


static void chkfloat(float * f)
{
   uint32_t c;
   X2C_MOVE((char *)f,(char *) &c,4UL);
   if ((c&0x7F000000UL)==0x7F000000UL) *f = 0.0f;
   else if (*f>=2.147483648E+9f) *f = 2.147483648E+9f;
   else if (*f<=(-2.147483648E+9f)) *f = (-2.147483648E+9f);
} /* end chkfloat() */


static char line(pINOUT c, uint32_t len, uint32_t jumpback)
{
   uint32_t odc;
   uint32_t mar;
   uint32_t hsize;
   uint32_t j;
   uint32_t i;
   float max0;
   float dcim;
   float dcre;
   float w;
   float r;
   float min0;
   int32_t hi;
   uint32_t hist[256];
   /* iq u8 file to complex float */
   struct Complex * anonym;
   /* iq i16 file to complex float */
   struct Complex * anonym0;
   /* iq f32 file to complex float */
   struct Complex * anonym1;
   struct Complex * anonym2;
   struct Complex * anonym3;
   /* absolute log level */
   struct Complex * anonym4;
   uint32_t tmp;
   if (iqsize==2UL) {
      if (!rdpipe((char *)inb->Adr, (inb->Len0*1u)/1u, iqfd, inb->Len0*1u,
                 jumpback*iqsize)) return 0;
   }
   else if (iqsize==4UL) {
      if (!rdpipe((char *)inbw->Adr, (inbw->Len0*2u)/1u, iqfd,
                inbw->Len0*2u, jumpback*iqsize)) return 0;
   }
   else if (!rdpipe((char *)inbf->Adr, (inbf->Len0*sizeof(float))/1u,
                iqfd, inbf->Len0*sizeof(float), jumpback*iqsize)) {
      return 0;
   }
   j = 0UL;
   dcre = 0.0f;
   dcim = 0.0f;
   if (iqsize==2UL) {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct Complex * anonym = &c[i];
            anonym->Re = (float)(int32_t)inb->Adr[j]-127.5f;
            anonym->Im = (float)(int32_t)inb->Adr[j+1UL]-127.5f;
            dcre = dcre+anonym->Re;
            dcim = dcim+anonym->Im;
         }
         j += 2UL;
         if (i==tmp) break;
      } /* end for */
   }
   else if (iqsize==4UL) {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct Complex * anonym0 = &c[i];
            anonym0->Re = (float)(int32_t)inbw->Adr[j];
            anonym0->Im = (float)(int32_t)inbw->Adr[j+1UL];
            dcre = dcre+anonym0->Re;
            dcim = dcim+anonym0->Im;
         }
         j += 2UL;
         if (i==tmp) break;
      } /* end for */
   }
   else {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct Complex * anonym1 = &c[i];
            anonym1->Re = inbf->Adr[j];
            anonym1->Im = inbf->Adr[j+1UL];
            if (checkfloat) {
               chkfloat(&anonym1->Re);
               chkfloat(&anonym1->Im);
            }
            dcre = dcre+anonym1->Re;
            dcim = dcim+anonym1->Im;
         }
         j += 2UL;
         if (i==tmp) break;
      } /* end for */
   }
   findoverdrives(c, len, peaklev, &odc);
   if (dcoffset) {
      r = X2C_DIVR(1.0f,(float)len);
      dcre = dcre*r;
      dcim = dcim*r;
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct Complex * anonym2 = &c[i];
            w = win->Adr[i]; /* window */
            anonym2->Re = (anonym2->Re-dcre)*w;
            anonym2->Im = (anonym2->Im-dcim)*w;
         }
         if (i==tmp) break;
      } /* end for */
   }
   else {
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         { /* with */
            struct Complex * anonym3 = &c[i];
            w = win->Adr[i]; /* window */
            anonym3->Re = anonym3->Re*w;
            anonym3->Im = anonym3->Im*w;
         }
         if (i==tmp) break;
      } /* end for */
   }
   fft3();
   min0 = 2.147483647E+9f;
   tmp = len-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct Complex * anonym4 = &c[i];
         r = brightness*osic_ln(anonym4->Re*anonym4->Re+anonym4->Im*anonym4->Im+0.0001f)
                ;
         if (r<min0) {
            min0 = r;
         }
         anonym4->Re = r;
      }
      if (i==tmp) break;
   } /* end for */
   hsize = len/2UL;
   tmp = len-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /* reorder freq */
      if (i<hsize) j = i+hsize;
      else j = i-hsize;
      c[i].Im = c[j].Re;
      if (i==tmp) break;
   } /* end for */
   if (dcoffset) c[hsize-1UL].Im = (c[hsize].Im+c[hsize-2UL].Im)*0.5f;
   if (odc>0UL) {
      /* overdrive so make 1 max level to the one and only */
      odc = len;
      max0 = 0.0f;
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         if (c[i].Im>max0) {
            max0 = c[i].Im;
            odc = i;
         }
         if (i==tmp) break;
      } /* end for */
      tmp = len-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         c[i].Im = X2C_DIVR(c[i].Im,
                1.0f+0.5f*(float)fabs((float)i-(float)odc));
         if (i==tmp) break;
      } /* end for */
   }
   /*    IF ABS(VAL(INTEGER,i)-VAL(INTEGER,odc))>2 THEN c^[i].Im:=0.0 END; */
   if (zerolev!=0.0f) min0 = zerolev;
   else {
      memset((char *)hist,(char)0,1024UL);
      mar = margin;
      if (mar>len/4UL) mar = len/4UL;
      tmp = (len-mar)-1UL;
      i = mar;
      if (i<=tmp) for (;; i++) {
         hi = (int32_t)X2C_TRUNCI(c[i].Im-min0,X2C_min_longint,
                X2C_max_longint);
         if (hi<0L) hi = 0L;
         else if (hi>255L) hi = 255L;
         ++hist[hi];
         if (i==tmp) break;
      } /* end for */
      i = 0UL;
      r = (float)(len-mar*2UL)*blackpercent*0.01f;
      while (i<=255UL && r>0.0f) {
         r = r-(float)hist[i];
         ++i;
      }
      min0 = min0+(float)i;
   }
   /*FOR i:=0 TO 150 DO WrInt(hist[i], 4) END; WrStrLn("---"); */
   ++mediancount;
   tmp = len-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      r = c[i].Im-min0;
      c[i].Im = r;
      median->Adr[i] = median->Adr[i]+r;
      if (peak->Adr[i]<r) peak->Adr[i] = r;
      if (i==tmp) break;
   } /* end for */
   return 1;
} /* end line() */


static void imageline(uint32_t y1, float p[], uint32_t p_len,
                float m[], uint32_t m_len, uint32_t * cnt)
{
   uint32_t j;
   uint32_t i;
   float lg;
   float mulpeak;
   float zoom;
   float fr;
   float r;
   uint8_t col;
   struct IMAGELINE * anonym;
   struct IMAGELINE * anonym0;
   struct IMAGELINE * anonym1;
   uint32_t tmp;
   size_t tmp0[1];
   lg = osic_ln(1.0f+osic_ln((float)oversample));
   mulpeak = X2C_DIVR(20.0f-2.0f*lg,20.0f);
                /* compensate oversample on peak levels */
   tmp = p_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      p[i] = p[i]*mulpeak;
      if (i==tmp) break;
   } /* end for */
   if (withmedian && *cnt>0UL) {
      fr = X2C_DIVR(X2C_DIVR(20.0f+6.0f*lg,20.0f),(float)*cnt);
                /* compensate oversample on median levels */
      if (medianpeak) {
         tmp = p_len-1;
         i = 0UL;
         if (i<=tmp) for (;; i++) {
            r = m[i]*fr;
            if (r>p[i]) p[i] = r;
            if (i==tmp) break;
         } /* end for */
      }
      else {
         tmp = p_len-1;
         i = 0UL;
         if (i<=tmp) for (;; i++) {
            p[i] = m[i]*fr;
            if (i==tmp) break;
         } /* end for */
      }
   }
   if (image->Adr[y1]==0) {
      X2C_DYNALLOCATE((char **) &image->Adr[y1],1u,(tmp0[0] = xsize,
                tmp0),1u);
      if (image->Adr[y1]==0) Error("out of memory", 14ul);
      tmp = xsize-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         *(anonym = image->Adr[y1],&anonym->Adr[i]) = 0U;
         if (i==tmp) break;
      } /* end for */
   }
   zoom = 0.0f;
   if ((p_len-1)+1UL!=xsize) {
      zoom = X2C_DIVR((float)((p_len-1)+1UL),(float)xsize);
                /* horizontal zoom */
   }
   tmp = xsize-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /* make image line */
      if (zoom!=0.0f) {
         /* interpolate 2 freq slots */
         fr = (float)i*zoom;
         j = (uint32_t)X2C_TRUNCC(fr,0UL,X2C_max_longcard);
         fr = fr-(float)j;
         r = p[j]*(1.0f-fr);
         if (j<p_len-1) r = r+p[j+1UL]*fr;
      }
      else r = p[i];
      if (r<1.0f) col = 1U;
      else if (r>255.0f) col = 255U;
      else col = (uint8_t)(uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
      if (col>(anonym0 = image->Adr[y1],anonym0->Adr[i])) {
         *(anonym1 = image->Adr[y1],&anonym1->Adr[i]) = col; /* peak hold */
      }
      if (i==tmp) break;
   } /* end for */
   *cnt = 0UL;
   clrb(p, p_len);
   clrb(m, m_len);
} /* end imageline() */


static void testimage(void)
{
   uint32_t y1;
   uint32_t x;
   struct IMAGELINE * anonym;
   size_t tmp[1];
   uint32_t tmp0;
   y1 = 0UL;
   do {
      X2C_DYNALLOCATE((char **) &image->Adr[y1],1u,(tmp[0] = xsize,tmp),
                1u);
      if (image->Adr[y1]==0) Error("out of memory", 14ul);
      tmp0 = xsize-1UL;
      x = 0UL;
      if (x<=tmp0) for (;; x++) {
         *(anonym = image->Adr[y1],
                &anonym->Adr[x]) = (uint8_t)((x*255UL)/xsize);
         if (x==tmp0) break;
      } /* end for */
      ++y1;
   } while (y1<ysize);
   wrpng(0UL, xsize, ysize, imagefn, 1024ul);
} /* end testimage() */

static uint32_t y;

static uint32_t blocksize;

static uint32_t os;

static uint32_t fp;

static uint32_t fpo;

static uint32_t bufsize;

static uint32_t rollcnt;

static char fifo;

static char notfirstline;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   uint32_t tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,1500000000l);
   aprsstr_BEGIN();
   osi_BEGIN();
   imagefn[0] = 0;
   iqfn[0] = 0;
   xsize = 1024UL;
   ysize = 0UL;
   wintyp = 0UL;
   roll = 0UL;
   mirrory = 0;
   brightness = 15.0f;
   zerolev = 0.0f;
   oversample = 1UL;
   filejump = 0UL;
   transp = 0;
   withmedian = 0;
   medianpeak = 0;
   blackpercent = 10.0f;
   iqsize = 2UL;
   margin = 0UL;
   rollcnt = 0UL;
   makelut();
   dcoffset = 0;
   notfirstline = 0;
   cleanoverdrives = 0.0f;
   checkfloat = 0;
   Parms();
   if (roll && ysize==0UL) Error("need -y <size>", 15ul);
   blocksize = 1UL;
   while (blocksize<xsize) blocksize += blocksize;
   if (filejump==0UL) filejump = blocksize;
   if (imagefn[0]==0) Error("no output image?", 17ul);
   if (iqfn[0]==0) osi_Werr("no iq file? writing only test image\012", 37ul);
   y = ysize;
   if (y==0UL) y = 32000UL;
   X2C_DYNALLOCATE((char **) &image,sizeof(struct IMAGELINE *),
                (tmp[0] = y,tmp),1u);
   if (image==0) Error("out of memory", 14ul);
   tmp0 = image->Len0-1;
   y = 0UL;
   if (y<=tmp0) for (;; y++) {
      image->Adr[y] = 0;
      if (y==tmp0) break;
   } /* end for */
   osic_alloc((char * *) &blackline, xsize);
   if (blackline==0) Error("out of memory", 14ul);
   tmp0 = xsize-1UL;
   y = 0UL;
   if (y<=tmp0) for (;; y++) {
      blackline[y] = 0U;
      if (y==tmp0) break;
   } /* end for */
   cp = plan(blocksize);
   if (cp==0) Error("out of memory", 14ul);
   if (iqsize==2UL) {
      X2C_DYNALLOCATE((char **) &inb,1u,(tmp[0] = blocksize*2UL,tmp),
                1u);
      if (inb==0) Error("out of memory", 14ul);
      peaklev = 127.0f;
   }
   else if (iqsize==4UL) {
      X2C_DYNALLOCATE((char **) &inbw,2u,(tmp[0] = blocksize*2UL,tmp),
                1u);
      if (inbw==0) Error("out of memory", 14ul);
      peaklev = 32767.0f;
   }
   else {
      X2C_DYNALLOCATE((char **) &inbf,sizeof(float),
                (tmp[0] = blocksize*2UL,tmp),1u);
      if (inbf==0) Error("out of memory", 14ul);
      peaklev = 1.0f;
   }
   X2C_DYNALLOCATE((char **) &win,sizeof(float),(tmp[0] = blocksize,
                tmp),1u);
   if (win==0) Error("out of memory", 14ul);
   makewin(win->Adr, win->Len0, peaklev); /* window function table */
   X2C_DYNALLOCATE((char **) &median,sizeof(float),
                (tmp[0] = blocksize,tmp),1u);
   if (median==0) Error("out of memory", 14ul);
   clrb(median->Adr, median->Len0);
   mediancount = 0UL;
   X2C_DYNALLOCATE((char **) &peak,sizeof(float),(tmp[0] = blocksize,
                tmp),1u);
   if (peak==0) Error("out of memory", 14ul);
   clrb(peak->Adr, peak->Len0);
   if (iqsize==2UL) bufsize = inb->Len0*1u;
   else if (iqsize==4UL) bufsize = inbw->Len0*2u;
   else bufsize = inbf->Len0*sizeof(float);
   if (iqfn[0]==0) testimage();
   else {
      iqfd = osi_OpenRead(iqfn, 1024ul);
      if (iqfd<0L) Error("iq file open", 13ul);
      fifo = osi_IsFifo(iqfd);
      if (roll && !fifo) {
         osi_Werr("warning: not iq pipe but roll mode (-r) ?", 42ul);
      }
      memcpy(tmpfn,imagefn,1024u);
      aprsstr_Append(tmpfn, 1024ul, "~", 2ul);
      y = 0UL;
      fp = 0UL;
      for (;;) {
         tmp0 = oversample;
         os = 1UL;
         if (os<=tmp0) for (;; os++) {
            fifosetback = 0UL;
            if (fifo) {
               if (filejump>blocksize) {
                  /* ingnore bytes in fifo */
                  if (!dummyread(iqfd, (filejump-blocksize)*iqsize)) {
                     goto loop_exit;
                  }
               }
               else if (notfirstline) {
                  fifosetback = filejump; /* reuse samples from fifo */
               }
               notfirstline = 1;
            }
            else {
               osic_Seek(iqfd, fp);
               fpo = fp;
               fp += filejump*iqsize;
               if (fp<fpo) {
                  osi_Werr("iq-file 4GB limit\012", 19ul);
                  goto loop_exit;
               }
            }
            if (!line(cp, blocksize, fifosetback)) goto loop_exit;
            if (os==tmp0) break;
         } /* end for */
         /*WrInt(y,10); WrStrLn("=y"); */
         imageline(y, peak->Adr, peak->Len0, median->Adr, median->Len0,
                &mediancount);
         if (roll) {
            y = (y+1UL)%ysize;
            ++rollcnt;
            if (rollcnt>=roll) {
               wrpng(y, xsize, ysize, tmpfn, 1024ul);
               osi_Rename(tmpfn, 1024ul, imagefn, 1024ul);
               rollcnt = 0UL;
            }
            clrline(y);
         }
         else {
            ++y;
            if (y>image->Len0-1 || ysize && y>=ysize) break;
         }
      }
      loop_exit:;
      if (roll) {
         if (rollcnt>0UL) {
            /* flush image in roll mode*/
            wrpng(y, xsize, ysize, tmpfn, 1024ul);
            osi_Rename(tmpfn, 1024ul, imagefn, 1024ul);
         }
      }
      else wrpng(0UL, xsize, ysize, imagefn, 1024ul);
      osic_Close(iqfd);
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
