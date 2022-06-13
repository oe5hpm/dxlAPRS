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
#define waterfall_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef mlib_H_
#include "mlib.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef pngwritepalette_H_
#include "pngwritepalette.h"
#endif
#ifndef fft_H_
#include "fft.h"
#endif
#ifndef complex_H_
#include "complex.h"
#endif

/* make waterfall png out of IQ input by oe5dxl */
/*IMPORT pngwrite; */
#define waterfall_PI 3.1415926535

#define waterfall_PI2 6.283185307

#define waterfall_LF "\012"

#define waterfall_CLOCKJUMP 60
/* max s clockjump to restart timeline */

#define waterfall_PALETTELEN 256


struct IMAGELINE {
   uint8_t * Adr;
   size_t Len0;
};


struct IMAGE {
   struct IMAGELINE * * Adr;
   size_t Len0;
};


struct CB {
   struct complex_Complex * Adr;
   size_t Len0;
};

static struct IMAGE * image;

static char imagefn[1024];

static char tmpfn[1024];

static char iqfn[1024];

static pLINE blackline;

static uint32_t oversample;

static uint32_t filejump;

static uint32_t xsize;

static uint32_t ysize;

static int32_t iqfd;

static char transp;

static char roll;
/*    lut:ARRAY[0..255] OF RECORD r,g,b:CARD8 END; */

static struct CB * cp;


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

static struct _2 * win;

static char mirrory;

static char i16;

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
                uint8_t * res0, uint32_t len, char * ok1)
{
   uint32_t n;
   *ok1 = 0;
   n = 0UL;
   while (*p<len && s[*p]==' ') ++*p;
   while ((*p<len && (uint8_t)s[*p]>='0') && (uint8_t)s[*p]<='9') {
      n = (n*10UL+(uint32_t)(uint8_t)s[*p])-48UL;
      *ok1 = 1;
      ++*p;
   }
   if (n>1023UL) *ok1 = 0;
   if (*ok1) *res0 = (uint8_t)n;
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
   char ok1;
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
            card(s, 20001ul, &p, &idx, (uint32_t)len, &ok1);
            if (idx>255U) {
               osi_Werr("colour index > 255\012", 20ul);
               break;
            }
            if (ok1) {
               card(s, 20001ul, &p, &palette[idx].r, (uint32_t)len, &ok1);
            }
            if (ok1) {
               card(s, 20001ul, &p, &palette[idx].g, (uint32_t)len, &ok1);
            }
            if (ok1) {
               card(s, 20001ul, &p, &palette[idx].b, (uint32_t)len, &ok1);
            }
            if (!ok1) {
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

/*
PROCEDURE wrpng(fromy:CARDINAL; fn-:ARRAY OF CHAR);

VAR pngimg:pngwrite.PNGPIXMAP;
    x, y, yy, ret:CARDINAL;
    c:CARD8;
BEGIN
  ALLOCATE(pngimg.image, xsize*ysize*3);
  IF pngimg.image<>NIL THEN
    FOR y:=0 TO ysize-1 DO
      IF mirrory THEN yy:=ysize-1-y ELSE yy:=y END;
      yy:=(ysize+fromy-yy) MOD ysize;
      FOR x:=0 TO xsize-1 DO
        WITH lut[image^[yy][x]] DO
          WITH pngimg.image^[x+y*xsize] DO
            red:=r; green:=g; blue:=b;
          END;
        END;
      END;
    END;
    pngimg.width:=xsize;
    pngimg.height:=ysize;
    ret:=pngwrite.writepng(fn, pngimg);
    DEALLOCATE(pngimg.image, xsize*ysize*3);
  ELSE Werr("png write out of memory"+LF) END;
END wrpng;
*/

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
   if (ysize0==0UL) {
      while (ysize0<=image->Len0-1 && image->Adr[ysize0]) ++ysize0;
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
                &ysize) || ysize<5UL) || ysize>=16000UL) {
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
            if ((h[0U]=='i' && h[1U]=='1') && h[2U]=='6') i16 = 1;
            else if (h[0U]=='u' && h[1U]=='8') {
            }
            else Error("-f u8|i16", 10ul);
         }
         else if (h[1U]=='b') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&brightness, h, 1024ul)) {
               Error("-b <brightness>", 16ul);
            }
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
         else if (h[1U]=='r') roll = 1;
         else if (h[1U]=='t') transp = 1;
         else if (h[1U]=='o') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &oversample) || oversample<1UL) {
               Error("-o <oversample>", 16ul);
            }
         }
         else if (h[1U]=='c') {
            osi_NextArg(h, 1024ul);
            readlut(h, 1024ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn(" Make Waterfall image out of IQ-File", 37ul);
               osi_WrStrLn(" -b <brightness>    gain (20.0)", 32ul);
               osi_WrStrLn(" -c <file>          read colour lines \"<index> <\
r> <g> <b>\" (0..255) gaps interpolated", 87ul);
               osi_WrStrLn("                      for colours testimage start\
 without -i", 61ul);
               osi_WrStrLn(" -h                 this", 25ul);
               osi_WrStrLn(" -f u8|i16          IQ data format", 35ul);
               osi_WrStrLn(" -i <file>          IQ-filename", 32ul);
               osi_WrStrLn(" -j <samples>       jump to next position in IQ-f\
ile in samples (default read all samples)", 91ul);
               osi_WrStrLn(" -j <ms>            in roll-mode (-r) wait ms til\
l next subsample taken from near eof", 86ul);
               osi_WrStrLn("                      next image written in this \
time*oversample at system clock second change", 95ul);
               osi_WrStrLn("                      on system clock jump max. 6\
0s will be filled or left out", 79ul);
               osi_WrStrLn(" -m                 mirror image vertical",
                42ul);
               osi_WrStrLn(" -o <n>             oversample <n> times with pea\
k-hold before draw next line (1)", 82ul);
               osi_WrStrLn("                      if -r and IQ-file is a pipe\
: -o seconds/line -j samples/line", 83ul);
               osi_WrStrLn(" -r                 roll-mode with image line fro\
m last position of (growing) IQ-file", 86ul);
               osi_WrStrLn("                      IQ-file may be removed/rewr\
itten any time", 64ul);
               osi_WrStrLn(" -t                 unfilled part of image is tra\
nsparent", 58ul);
               osi_WrStrLn(" -w <file>          waterfall image png in roll m\
ode written every new line", 76ul);
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


static void makewin(float w[], uint32_t w_len)
{
   uint32_t i;
   float N;
   uint32_t tmp;
   N = X2C_DIVR(6.283185307f,(float)((w_len-1)+1UL));
   tmp = w_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      w[i] = (0.42f-0.5f*osic_cos(N*(float)i))
                +0.08f*osic_cos(N*2.0f*(float)i); /* Blackmann */
      if (i==tmp) break;
   } /* end for */
} /* end makewin() */


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


static char line(uint32_t y1, struct complex_Complex c[],
                uint32_t c_len)
{
   uint32_t hsize;
   uint32_t j;
   uint32_t i;
   float dcim;
   float dcre;
   float zoom;
   float w;
   float fr;
   float r;
   float min0;
   uint8_t col;
   int32_t res0;
   int32_t le;
   int32_t hi;
   uint32_t hist[256];
   /* iq file to complex float */
   struct complex_Complex * anonym;
   struct complex_Complex * anonym0;
   struct IMAGELINE * anonym1;
   struct IMAGELINE * anonym2;
   struct IMAGELINE * anonym3;
   uint32_t tmp;
   size_t tmp0[1];
   hsize = ((c_len-1)+1UL)/2UL;
   if (i16) {
      le = (int32_t)(inbw->Len0*2u);
      do {
         res0 = osi_RdBin(iqfd, (char *)inbw->Adr, (inbw->Len0*2u)/1u,
                (uint32_t)le);
         if (res0<=0L) return 0;
         le -= res0;
      } while (le>0L);
   }
   else {
      /*IF RdBin(iqfd, inbw^, SIZE(inbw^))<>VAL(INTEGER,
                SIZE(inbw^)) THEN RETURN FALSE END; */
      le = (int32_t)(inb->Len0*1u);
      do {
         res0 = osi_RdBin(iqfd, (char *)inb->Adr, (inb->Len0*1u)/1u,
                (uint32_t)le);
         if (res0<=0L) return 0;
         le -= res0;
      } while (le>0L);
   }
   /*IF RdBin(iqfd, inb^, SIZE(inb^))<>VAL(INTEGER,
                SIZE(inb^)) THEN RETURN FALSE END; */
   j = 0UL;
   dcre = 0.0f;
   dcim = 0.0f;
   tmp = c_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct complex_Complex * anonym = &c[i];
         w = win->Adr[i];
         if (i16) anonym->Re = (float)(int32_t)inbw->Adr[j]*3.90625E-3f;
         else anonym->Re = (float)(int32_t)inb->Adr[j];
         dcre = dcre+anonym->Re;
         ++j;
         if (i16) anonym->Im = (float)(int32_t)inbw->Adr[j]*3.90625E-3f;
         else anonym->Im = (float)(int32_t)inb->Adr[j];
         dcim = dcim+anonym->Im;
         ++j;
      }
      if (i==tmp) break;
   } /* end for */
   r = X2C_DIVR(1.0f,(float)((c_len-1)+1UL));
   dcre = dcre*r;
   dcim = dcim*r;
   tmp = c_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      { /* with */
         struct complex_Complex * anonym0 = &c[i];
         w = win->Adr[i]; /* window */
         anonym0->Re = (anonym0->Re-dcre)*w;
         anonym0->Im = (anonym0->Im-dcim)*w;
      }
      if (i==tmp) break;
   } /* end for */
   fft_Transform(c, c_len, 0);
   min0 = X2C_max_real;
   tmp = c_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /* absolute log level */
      r = brightness*osic_ln(c[i].Re*c[i].Re+c[i].Im*c[i].Im+0.001f);
      if (r<min0) min0 = r;
      c[i].Re = r;
      if (i==tmp) break;
   } /* end for */
   if (zerolev!=0.0f) min0 = zerolev;
   else {
      memset((char *)hist,(char)0,1024UL);
      tmp = c_len-1;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         hi = (int32_t)X2C_TRUNCI(c[i].Re-min0,X2C_min_longint,
                X2C_max_longint);
         if (hi<0L) hi = 0L;
         else if (hi>255L) hi = 255L;
         ++hist[hi];
         if (i==tmp) break;
      } /* end for */
      i = 0UL;
      r = (float)((c_len-1)+1UL)*blackpercent*0.01f;
      while (i<=255UL && r>0.0f) {
         r = r-(float)hist[i];
         ++i;
      }
      min0 = min0+(float)i;
   }
   /*WrInt(hist[0], 10); WrInt(hist[1], 10);WrInt(hist[2], 10); WrInt(i, 10);
                 WrStrLn("=i"); */
   tmp = c_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /* reorder freq */
      if (i<hsize) j = (hsize-1UL)-i;
      else j = (hsize*3UL-1UL)-i;
      c[i].Im = c[j].Re-min0;
      if (i==tmp) break;
   } /* end for */
   c[hsize-1UL].Im = (c[hsize].Im+c[hsize-2UL].Im)*0.5f;
                /* fill dc notch hole */
   if (image->Adr[y1]==0) {
      X2C_DYNALLOCATE((char **) &image->Adr[y1],1u,(tmp0[0] = xsize,
                tmp0),1u);
      if (image->Adr[y1]==0) Error("out of memory", 14ul);
      tmp = xsize-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         *(anonym1 = image->Adr[y1],&anonym1->Adr[i]) = 0U;
         if (i==tmp) break;
      } /* end for */
   }
   zoom = 0.0f;
   if ((c_len-1)+1UL!=xsize) {
      zoom = X2C_DIVR((float)((c_len-1)+1UL),(float)xsize);
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
         r = c[j].Im*(1.0f-fr);
         if (j<c_len-1) r = r+c[j+1UL].Im*fr;
      }
      else r = c[i].Im;
      if (r<1.0f) col = 1U;
      else if (r>255.0f) col = 255U;
      else col = (uint8_t)(uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
      if (col>(anonym2 = image->Adr[y1],anonym2->Adr[i])) {
         *(anonym3 = image->Adr[y1],&anonym3->Adr[i]) = col; /* peak hold */
      }
      if (i==tmp) break;
   } /* end for */
   return 1;
} /* end line() */


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

static uint32_t nextsec;

static uint32_t rt;

static uint32_t fpo;

static uint32_t bufsize;

static uint32_t iqsize;

static int32_t res;

static int32_t skip;

static int32_t sk;

static char ok0;

static char fifo;


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[1];
   uint32_t tmp0;
   X2C_BEGIN(&argc,argv,1,4000000l,1500000000l);
   complex_BEGIN();
   fft_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
   imagefn[0] = 0;
   iqfn[0] = 0;
   xsize = 1024UL;
   ysize = 0UL;
   roll = 0;
   mirrory = 0;
   brightness = 20.0f;
   zerolev = 0.0f;
   oversample = 1UL;
   filejump = 0UL;
   transp = 0;
   blackpercent = 10.0f;
   i16 = 0;
   makelut();
   Parms();
   if ((roll || iqfn[0]==0) && ysize==0UL) Error("need -y <size>", 15ul);
   blocksize = 1UL;
   while (blocksize<xsize) blocksize += blocksize;
   if (filejump==0UL) filejump = blocksize;
   if (imagefn[0]==0) Error("no output image?", 17ul);
   if (iqfn[0]==0) osi_Werr("no iq file? writing test image\012", 32ul);
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
   X2C_DYNALLOCATE((char **) &cp,sizeof(struct complex_Complex),
                (tmp[0] = blocksize,tmp),1u);
   if (cp==0) Error("out of memory", 14ul);
   if (i16) {
      X2C_DYNALLOCATE((char **) &inbw,2u,(tmp[0] = blocksize*2UL,tmp),
                1u);
      if (inbw==0) Error("out of memory", 14ul);
   }
   else {
      X2C_DYNALLOCATE((char **) &inb,1u,(tmp[0] = blocksize*2UL,tmp),
                1u);
      if (inb==0) Error("out of memory", 14ul);
   }
   X2C_DYNALLOCATE((char **) &win,sizeof(float),(tmp[0] = blocksize,
                tmp),1u);
   if (win==0) Error("out of memory", 14ul);
   makewin(win->Adr, win->Len0); /* window function table */
   if (i16) {
      iqsize = 4UL;
      bufsize = inbw->Len0*2u;
   }
   else {
      iqsize = 2UL;
      bufsize = inb->Len0*1u;
   }
   if (iqfn[0]==0) testimage();
   else {
      if (roll) {
         iqfd = osi_OpenRead(iqfn, 1024ul);
         fifo = 0;
         if (iqfd>=0L) {
            fifo = osi_IsFifo(iqfd);
            if (!fifo) osic_Close(iqfd);
         }
         memcpy(tmpfn,imagefn,1024u);
         aprsstr_Append(tmpfn, 1024ul, "~", 2ul);
         y = 0UL;
         nextsec = osic_time();
         for (;;) {
            if (fifo) ok0 = line(y, cp->Adr, cp->Len0);
            else {
               iqfd = osi_OpenRead(iqfn, 1024ul);
               if (iqfd>=0L) {
                  osic_Seekend(iqfd, -(int32_t)(blocksize*iqsize));
                /* seek from eof */
                  ok0 = line(y, cp->Adr, cp->Len0);
                  osic_Close(iqfd);
               }
               else osi_Werr("iq-file open error\012", 20ul);
            }
            rt = osic_time();
            if (nextsec<rt) {
               if (nextsec+60UL<rt || nextsec>rt+60UL) {
                  nextsec = rt; /* do not write nothing or amok on clock jump */
               }
               if (fifo) nextsec += oversample;
               else nextsec += (filejump*oversample)/1000UL;
               y = (y+1UL)%ysize;
               wrpng(y, xsize, ysize, tmpfn, 1024ul);
               osi_Rename(tmpfn, 1024ul, imagefn, 1024ul);
               clrline(y);
            }
            if (fifo) {
               skip = (int32_t)filejump;
               for (;;) {
                  skip -= (int32_t)bufsize;
                  if (skip<=0L) break;
                  sk = skip;
                  if (sk>(int32_t)bufsize) sk = (int32_t)bufsize;
                  if (i16) {
                     res = osi_RdBin(iqfd, (char *)inbw->Adr,
                (inbw->Len0*2u)/1u, (uint32_t)sk);
                  }
                  else {
                     res = osi_RdBin(iqfd, (char *)inb->Adr,
                (inb->Len0*1u)/1u, (uint32_t)sk);
                  }
               }
            }
            else usleep(filejump*1000UL);
         }
      }
      iqfd = osi_OpenRead(iqfn, 1024ul);
      if (iqfd>=0L) {
         fp = 0UL;
         y = 0UL;
         for (;;) {
            tmp0 = oversample;
            os = 1UL;
            if (os<=tmp0) for (;; os++) {
               osic_Seek(iqfd, fp);
               fpo = fp;
               fp += filejump*iqsize;
               if (fp<fpo) {
                  osi_Werr("iq-file 4GB limit\012", 19ul);
                  goto loop_exit;
               }
               if (!line(y, cp->Adr, cp->Len0)) goto loop_exit;
               if (os==tmp0) break;
            } /* end for */
            ++y;
            if (y>image->Len0-1 || ysize && y>=ysize) break;
         }
         loop_exit:;
         osic_Close(iqfd);
         wrpng(0UL, xsize, ysize, imagefn, 1024ul);
      }
      else osi_Werr("iq-file open error\012", 20ul);
   }
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
