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
#define panorama_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef libsrtm_H_
#include "libsrtm.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef pngwrite_H_
#include "pngwrite.h"
#endif
#ifndef pngread_H_
#include "pngread.h"
#endif
#ifndef imagetext_H_
#include "imagetext.h"
#endif

/* make panorama image with srtm data by oe5dxl */
/*, wgs84r, wgs84s*/
#define panorama_NOALT 20000

#define panorama_PI 3.1415926535

#define panorama_PI2 6.283185307

#define panorama_NL "\012"

#define panorama_LF "\012"

#define panorama_REFRACC 7.85E-8

#define panorama_MAXSYMSIZE 48
/* max input symbol size */

struct COLTYP;


struct COLTYP {
   uint32_t r;
   uint32_t g;
   uint32_t b;
};
/*
     PIX=RECORD r,g,b:CARD16 END;
     IMAGELINE=ARRAY OF PIX;
     IMAGE=ARRAY OF IMAGELINE;
     pIMAGE=POINTER TO IMAGE;
*/

struct PANOWIN;


struct PANOWIN {
   char flatscreen0;
   struct aprsstr_POSITION eye;
   struct aprsstr_POSITION horizon;
   int32_t eyealt;
   float angle0;
   float elevation;
   float yzoom0;
   float refract;
   int32_t hx; /* mouse pos on panwin and fullwin */
   int32_t hy;
   int32_t mx;
   int32_t my;
   float ang; /* poi fast preselect circular segment */
   float hseg;
   float dist;
   imagetext_pIMAGE image0;
};

struct PIX8A;


struct PIX8A {
   uint8_t r8; /* colour pixel with alpha */
   uint8_t g8;
   uint8_t b8;
   uint8_t alpha;
};

typedef struct PIX8A SYMBOL[48][48];

struct POI;

typedef struct POI * pPOI;


struct POI {
   pPOI next;
   uint32_t r;
   uint32_t g;
   uint32_t b;
   uint32_t alpha;
   SYMBOL symbol;
   uint32_t xs;
   uint32_t ys;
   char poifn[1024];
   char iconfn[1024];
   char iconhint[1024];
};

static imagetext_pIMAGE image;

static pPOI poifiles;

static char imagefn[1024];

static char csvfn[1024];

static int32_t xsize;

static int32_t ysize;

static struct aprsstr_POSITION posa;

static struct aprsstr_POSITION posb;

static float alta;

static float refraction;

static float igamma;

static float elev;

static float angle;

static float yzoom;

static char gammatab[1024];

static uint32_t rastercsv;

static uint32_t poisize;

static uint32_t scalesize;

static uint32_t fonttyp;

static struct COLTYP heavencol;

static char flatscreen;

static char verb;

static char altOG;

static float minpoialt;

static int32_t csvfd;

struct _0;

struct _1;


struct _1 {
   struct aprsstr_POSITION p;
   float a;
};


struct _0 {
   uint32_t act;
   struct _1 v[1000];
};

static struct _0 csvvec;


static void Error(char text[], uint32_t text_len)
{
   X2C_PCOPY((void **)&text,text_len);
   osi_Werr(text, text_len);
   osi_Werr(" error abort\012", 14ul);
   X2C_ABORT();
   X2C_PFREE(text);
} /* end Error() */


static uint32_t truncc(float r)
{
   if (r<=0.0f) return 0UL;
   else if (r>=2.E+9f) return 2000000000UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */


static float sqr(float x)
{
   return x*x;
} /* end sqr() */


static float atang2(float x, float y)
{
   float w;
   if ((float)fabs(x)>(float)fabs(y)) {
      w = osic_arctan(X2C_DIVR(y,x));
      if (x<0.0f) {
         if (y>0.0f) w = 3.1415926535f+w;
         else w = w-3.1415926535f;
      }
   }
   else if (y!=0.0f) {
      w = 1.57079632675f-osic_arctan(X2C_DIVR(x,y));
      if (y<0.0f) w = w-3.1415926535f;
   }
   else w = 0.0f;
   return w;
} /* end atang2() */


static void posinval(struct aprsstr_POSITION * pos)
{
   pos->long0 = 0.0f;
   pos->lat = 0.0f;
} /* end posinval() */


static char posvalid(const struct aprsstr_POSITION pos)
{
   return pos.lat!=0.0f || pos.long0!=0.0f;
} /* end posvalid() */


static struct aprsstr_POSITION radtodeg(struct aprsstr_POSITION p)
{
   p.lat = p.lat*5.729577951472E+1f;
   p.long0 = p.long0*5.729577951472E+1f;
   return p;
} /* end radtodeg() */


static void wgs84s(float lat, float long0, float nn, float * x,
                float * y, float * z)
/* km */
{
   float c;
   float h;
   h = nn+6370.0f;
   *z = h*osic_sin(lat);
   c = osic_cos(lat);
   *y = h*osic_sin(long0)*c;
   *x = h*osic_cos(long0)*c;
} /* end wgs84s() */


static void wgs84r(float x, float y, float z, float * lat,
                float * long0, float * heig)
/* km */
{
   float h;
   h = x*x+y*y;
   if ((float)fabs(x)>(float)fabs(y)) {
      *long0 = osic_arctan(X2C_DIVR(y,x));
      if (x<0.0f) {
         if (y>0.0f) *long0 = 3.1415926535f+*long0;
         else *long0 = *long0-3.1415926535f;
      }
   }
   else {
      *long0 = 1.57079632675f-osic_arctan(X2C_DIVR(x,y));
      if (y<0.0f) *long0 = *long0-3.1415926535f;
   }
   *lat = osic_arctan(X2C_DIVR(z,osic_sqrt(h)));
   *heig = osic_sqrt(h+z*z)-6370.0f;
} /* end wgs84r() */


static void makegammatab(void)
{
   uint32_t c;
   float g;
   g = X2C_DIVR(1.0f,igamma);
   gammatab[0U] = 0;
   for (c = 1UL; c<=1023UL; c++) {
      gammatab[c] = (char)(uint32_t)
                X2C_TRUNCC(osic_exp(osic_ln(X2C_DIVR((float)c,
                1024.0f))*g)*255.5f,0UL,X2C_max_longcard);
   } /* end for */
} /* end makegammatab() */

#define panorama_BYTESPERPIX 4

typedef struct PIX8A * pROWS;


static void readsymbol(SYMBOL img, uint32_t * x, uint32_t * y,
                const char fn[], uint32_t fn_len)
{
   pROWS rows[48];
   int32_t i;
   int32_t res;
   int32_t maxxbyte;
   int32_t maxy;
   int32_t maxx;
   maxx = 48L;
   maxy = 48L;
   maxxbyte = maxx*4L; /* maxxbyte = maxx*4: switch on alpha channel */
   for (i = 0L; i<=47L; i++) {
      rows[i] = (pROWS) &img[i][0U];
   } /* end for */
   res = readpng(fn, (char * *)rows, &maxx, &maxy, &maxxbyte);
   if (res==0L) {
      *x = (uint32_t)maxx;
      *y = (uint32_t)maxy;
   }
   else Error("icon image not readable", 24ul);
/*    x:=0; */
/*    y:=0; */
/*WrInt(res, 5); WrInt(maxx, 5); WrInt(maxy, 5); WrStrLn(""); */
} /* end readsymbol() */


static uint8_t pngc(uint16_t c)
{
   if (c<=1023U) return gammatab[c];
   else return 255U;
   return 0;
} /* end pngc() */


static void wrpng(void)
{
   struct PNGPIXMAP pngimg;
   int32_t ret;
   int32_t y;
   int32_t x;
   struct imagetext_PIX * anonym;
   struct PNGPIXEL * anonym0;
   int32_t tmp;
   int32_t tmp0;
   osic_alloc((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   if (pngimg.image) {
      makegammatab();
      tmp = ysize-1L;
      y = 0L;
      if (y<=tmp) for (;; y++) {
         tmp0 = xsize-1L;
         x = 0L;
         if (x<=tmp0) for (;; x++) {
            { /* with */
               struct imagetext_PIX * anonym = &image->Adr[(x)
                *image->Len0+((ysize-1L)-y)];
               { /* with */
                  struct PNGPIXEL * anonym0 = &pngimg.image[x+y*xsize];
                  anonym0->red = pngc(anonym->r);
                  anonym0->green = pngc(anonym->g);
                  anonym0->blue = pngc(anonym->b);
               }
            }
            if (x==tmp0) break;
         } /* end for */
         if (y==tmp) break;
      } /* end for */
      pngimg.width = (uint32_t)xsize;
      pngimg.height = (uint32_t)ysize;
      ret = writepng(imagefn, &pngimg);
      osic_free((char * *) &pngimg.image, (uint32_t)(xsize*ysize*3L));
   }
   else osi_Werr("png write out of memory\012", 25ul);
} /* end wrpng() */


static void getpos(char h[1024], struct aprsstr_POSITION * p,
                const char e[], uint32_t e_len)
{
   osi_NextArg(h, 1024ul);
   aprsstr_loctopos(p, h, 1024ul);
   if (!posvalid(*p)) {
      if (!aprsstr_StrToFix(&p->lat, h,
                1024ul) || (float)fabs(p->lat)>=90.0f) Error(e, e_len);
      osi_NextArg(h, 1024ul);
      if (!aprsstr_StrToFix(&p->long0, h,
                1024ul) || (float)fabs(p->long0)>180.0f) Error(e, e_len);
      p->lat = p->lat*1.7453292519444E-2f;
      p->long0 = p->long0*1.7453292519444E-2f;
   }
} /* end getpos() */


static void Parms(void)
{
   char err;
   char iconhint[1024];
   char iconfn[1024];
   char h[1024];
   float dist;
   float pan;
   uint32_t label;
   pPOI ppoi;
   int32_t col;
   err = 0;
   label = 0UL;
   iconfn[0] = 0;
   iconhint[0] = 0;
   pan = 0.0f;
   dist = 0.0f;
   for (;;) {
      osi_NextArg(h, 1024ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         if (h[1U]=='i') {
            osi_NextArg(imagefn, 1024ul);
            if (imagefn[0U]==0 || imagefn[0U]=='-') {
               Error("-i <imagefilename>", 19ul);
            }
         }
         else if (h[1U]=='P') {
            osic_alloc((char * *) &ppoi, sizeof(struct POI));
            if (ppoi==0) Error("poifile, out of memory", 23ul);
            ppoi->r = 255UL;
            ppoi->g = 255UL;
            ppoi->b = 255UL;
            ppoi->alpha = 255UL;
            osi_NextArg(h, 1024ul);
            if (aprsstr_StrToInt(h, 1024ul, &col)) {
               ppoi->r = (uint32_t)col;
               osi_NextArg(h, 1024ul);
               if (aprsstr_StrToInt(h, 1024ul, &col)) {
                  ppoi->g = (uint32_t)col;
                  osi_NextArg(h, 1024ul);
                  if (aprsstr_StrToInt(h, 1024ul, &col)) {
                     ppoi->b = (uint32_t)col;
                     osi_NextArg(h, 1024ul);
                     if (aprsstr_StrToInt(h, 1024ul, &col)) {
                        ppoi->alpha = (uint32_t)col;
                        osi_NextArg(h, 1024ul);
                     }
                  }
               }
            }
            aprsstr_Assign(ppoi->poifn, 1024ul, h, 1024ul);
            if (ppoi->poifn[0U]==0 || ppoi->poifn[0U]=='-') {
               Error("-P <POI filename>", 18ul);
            }
            aprsstr_Assign(ppoi->iconfn, 1024ul, iconfn, 1024ul);
            aprsstr_Assign(ppoi->iconhint, 1024ul, iconhint, 1024ul);
            ppoi->next = poifiles;
            poifiles = ppoi;
         }
         else if (h[1U]=='c') {
            osi_NextArg(csvfn, 1024ul);
            if (csvfn[0U]==0 || csvfn[0U]=='-') {
               Error("-c <csv filename>", 18ul);
            }
         }
         else if (h[1U]=='C') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToInt(h, 1024ul, &col) || col<0L) {
               Error("-C <number>", 12ul);
            }
            rastercsv = (uint32_t)col;
         }
         else if (h[1U]=='I') {
            osi_NextArg(iconfn, 1024ul);
            if (iconfn[0U]==0 || iconfn[0U]=='-') {
               Error("-I <icon filename> (png)", 25ul);
            }
         }
         else if (h[1U]=='J') {
            osi_NextArg(iconhint, 1024ul);
            if (iconhint[0U]==0 || iconhint[0U]=='-') {
               Error("-J <icon hint>", 15ul);
            }
            if (iconhint[0U]==',') iconhint[0U] = 0;
         }
         else if (h[1U]=='f') flatscreen = 1;
         else if (h[1U]=='v') verb = 1;
         else if (h[1U]=='O') altOG = 1;
         else if (h[1U]=='x') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &xsize) || xsize<20L) || xsize>=32000L) {
               Error("-x <size>", 10ul);
            }
         }
         else if (h[1U]=='y') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToInt(h, 1024ul,
                &ysize) || ysize<20L) || ysize>=8192L) {
               Error("-y <size>", 10ul);
            }
         }
         else if (h[1U]=='M') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &libsrtm_srtmmaxmem)) {
               Error("-M <bytes>", 11ul);
            }
         }
         else if (h[1U]=='p') {
            osi_NextArg(libsrtm_srtmdir, 1024ul);
            if (libsrtm_srtmdir[0U]==0 || libsrtm_srtmdir[0U]=='-') {
               Error("-p <srtm-folder-path>", 22ul);
            }
         }
         else if (h[1U]=='a') {
            getpos(h, &posa, "-a <lat> <long> or <locator>", 29ul);
         }
         else if (h[1U]=='b') {
            getpos(h, &posb, "-b <lat> <long> or <locator>", 29ul);
         }
         else if (h[1U]=='A') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&alta, h,
                1024ul) || alta<0.0f) || alta>20000.0f) {
               Error("-A <meter> (0..20000)", 22ul);
            }
         }
         else if (h[1U]=='d') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&pan, h, 1024ul)) {
               Error("-d <deg> <km>", 14ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToFix(&dist, h, 1024ul)) {
               Error("-d <deg> <km>", 14ul);
            }
         }
         else if (h[1U]=='q') libsrtm_bicubic = 1;
         else if (h[1U]=='o') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&minpoialt, h,
                1024ul) || minpoialt<(-2.E+4f)) || minpoialt>20000.0f) {
               Error("-o <meter> (0..20000)", 22ul);
            }
         }
         else if (h[1U]=='r') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&refraction, h,
                1024ul) || refraction<0.0f) || refraction>1.0f) {
               Error("-r <refraction> [0.0..1.0]", 27ul);
            }
         }
         else if (h[1U]=='e') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&elev, h,
                1024ul) || elev<(-90.0f)) || elev>90.0f) {
               Error("-e <elevation> [-90.0..90.0]", 29ul);
            }
         }
         else if (h[1U]=='w') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&angle, h,
                1024ul) || angle<0.1f) || angle>360.0f) {
               Error("-w <degrees> [0.1..360.0]", 26ul);
            }
         }
         else if (h[1U]=='z') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&yzoom, h,
                1024ul) || yzoom<0.001f) || yzoom>10.0f) {
               Error("-z <factor> [0.0..10.0]", 24ul);
            }
         }
         else if (h[1U]=='s') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul, &poisize) || poisize>20UL) {
               Error("-s <size> 1..20", 16ul);
            }
         }
         else if (h[1U]=='g') {
            osi_NextArg(h, 1024ul);
            if ((!aprsstr_StrToFix(&igamma, h,
                1024ul) || igamma<0.01f) || igamma>10.0f) {
               Error("-g <gamma> [0.1..10]", 21ul);
            }
         }
         else if (h[1U]=='H') {
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &heavencol.r) || heavencol.r>=1024UL) {
               Error("-H <r> <g> <b> 0..1023", 23ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &heavencol.g) || heavencol.g>=1024UL) {
               Error("-H <r> <g> <b> 0..1023", 23ul);
            }
            osi_NextArg(h, 1024ul);
            if (!aprsstr_StrToCard(h, 1024ul,
                &heavencol.b) || heavencol.b>=1024UL) {
               Error("-H <r> <g> <b> 0..1023", 23ul);
            }
         }
         else if (h[1U]=='F') {
            osi_NextArg(h, 1024ul);
            if (h[0U]=='1') fonttyp = 6UL;
            else if (h[0U]=='2') fonttyp = 8UL;
            else if (h[0U]=='3') fonttyp = 10UL;
            else Error("-F <font> (1:6x10 2:10x20)", 27ul);
         }
         else {
            if (h[1U]=='h') {
               osi_WrStrLn("", 1ul);
               osi_WrStrLn("Panorama", 9ul);
               osi_WrStrLn(" -A <m>                            Camera over gr\
ound [m] (10)", 63ul);
               osi_WrStrLn(" -a <lat> <long> | [locator]       Camera positio\
n lat long (degrees) or qth locator", 85ul);
               osi_WrStrLn(" -b <lat> <long> | [locator]       Pan to point l\
at long (degrees) or qth locator", 82ul);
               osi_WrStrLn(" -C <N>                            write CSV line\
 for every N\'th column", 72ul);
               osi_WrStrLn(" -c <filename>                     CSV file name",
                 49ul);
               osi_WrStrLn(" -d <deg> <km>                     Relative to po\
sition a (alternativ to -b)", 77ul);
               osi_WrStrLn("                                     km distance \
sight limit", 61ul);
               osi_WrStrLn(" -e <degrees>                      Camera Elevati\
on (degrees) (0.0)", 68ul);
               osi_WrStrLn("                                     speed optimi\
zer, surface contrast and dust calculation", 92ul);
               osi_WrStrLn("                                     are able to \
handle only small elevations!", 79ul);
               osi_WrStrLn(" -F <font>                         Font Size (1) \
1: 6x10, 2: 8x14, 3: 10x20", 76ul);
               osi_WrStrLn(" -f                                Flat screen el\
se Curved projection area (needed at >=180deg sight)", 102ul);
               osi_WrStrLn(" -g <gamma>                        Image Gamma 0.\
1..10 (2.2)", 61ul);
               osi_WrStrLn(" -H <r> <g> <b>                    Heaven colour \
(50 70 300)", 61ul);
               osi_WrStrLn(" -h                                this", 40ul);
               osi_WrStrLn(" -I <filename>                     Symbol Image F\
ile Name, set before -P", 73ul);
               osi_WrStrLn(" -i <filename>                     Image File Nam\
e", 51ul);
               osi_WrStrLn(" -J <poi-hint>                     text appended \
to csv lines, set before -P, off with -J ,", 92ul);
               osi_WrStrLn(" -M <bytes>                        SRTM-Cache Lim\
it (100000000)", 64ul);
               osi_WrStrLn(" -O                                POI File altit\
ude is over ground not NN", 75ul);
               osi_WrStrLn(" -o <m>                            min. POI altit\
ude over ground (1)", 69ul);
               osi_WrStrLn("                                     if given in \
POI file and higher use this", 78ul);
               osi_WrStrLn(" -P [<r> <g> <b> [<transp>]] <filename>  optional\
 icon colours (0..255) POI File Name", 86ul);
               osi_WrStrLn("                                     repeat for m\
ore files", 59ul);
               osi_WrStrLn(" -p <pathname>                     folder with /s\
rtm1 /srtm3 /srtm30", 69ul);
               osi_WrStrLn(" -q                                bicubic srtm i\
nterpolation instead of bilinear", 82ul);
               osi_WrStrLn(" -r <refraction>                   0.0(vacuum)..1\
.0(earth is a disk) (0.13)", 76ul);
               osi_WrStrLn(" -s <size>                         POI symbol siz\
e (5)", 55ul);
               osi_WrStrLn(" -v                                Say something",
                 49ul);
               osi_WrStrLn(" -w <degrees>                      Camera horizon\
tal sight angle (degrees) (45.0)", 82ul);
               osi_WrStrLn(" -x <size>                         Image size x (\
600)", 54ul);
               osi_WrStrLn(" -y <size>                         Image size y (\
400)", 54ul);
               osi_WrStrLn(" -z <factor>                       Vertical Zoom \
(for 360deg panorama) (1.0)", 77ul);
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
   if ((!posvalid(posb) && posvalid(posa)) && dist>0.01f) {
      pan = pan*1.7453292519444E-2f;
      posb.lat = posa.lat+X2C_DIVR(osic_cos(pan)*dist,6370.0f);
      posb.long0 = posa.long0+X2C_DIVR((X2C_DIVR(osic_sin(pan),
                osic_cos(posa.lat)))*dist,6370.0f);
      /*WrFixed(posb.lat/RAD, 5,10); WrFixed(posb.long/RAD, 5,10); */
      /*WrFixed(azimuth(posa, posb), 2,10);
                WrFixed(distance(posa, posb), 3,10); WrStrLn(""); */
      if (!posvalid(posb)) Error("camera sight point out of database", 35ul);
   }
} /* end Parms() */


static void wrcsv(int32_t px, int32_t py,
                const struct aprsstr_POSITION posdeg, float alti,
                const char text[], uint32_t text_len,
                const char iconhint[], uint32_t iconhint_len)
{
   char h[1001];
   char s[1001];
   if (csvfn[0U]) {
      if (csvfd<0L) csvfd = osi_OpenWrite(csvfn, 1024ul);
      if (csvfd<0L) Error("cannot write csv-file", 22ul);
      aprsstr_IntToStr(px, 0UL, s, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      aprsstr_IntToStr(py, 0UL, h, 1001ul);
      aprsstr_Append(s, 1001ul, h, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      aprsstr_FixToStr(posdeg.lat, 6UL, h, 1001ul);
      aprsstr_Append(s, 1001ul, h, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      aprsstr_FixToStr(posdeg.long0, 6UL, h, 1001ul);
      aprsstr_Append(s, 1001ul, h, 1001ul);
      aprsstr_Append(s, 1001ul, ",", 2ul);
      if (alti>(-1000.0f) && alti<10000.0f) {
         aprsstr_IntToStr((int32_t)X2C_TRUNCI(alti+0.5f,X2C_min_longint,
                X2C_max_longint), 0UL, h, 1001ul);
         aprsstr_Append(s, 1001ul, h, 1001ul);
      }
      if (text[0UL]) {
         aprsstr_Append(s, 1001ul, ",", 2ul);
         aprsstr_Append(s, 1001ul, text, text_len);
      }
      if (iconhint[0UL]) {
         aprsstr_Append(s, 1001ul, ",", 2ul);
         aprsstr_Append(s, 1001ul, iconhint, iconhint_len);
      }
      aprsstr_Append(s, 1001ul, "\012", 2ul);
      osi_WrBin(csvfd, (char *)s, 1001u/1u, aprsstr_Length(s, 1001ul));
   }
/* WITH image^[px][py] DO INC(r, 200); INC(g, 200); END; */
} /* end wrcsv() */

#define panorama_DDEG 7.85398163375E-6

#define panorama_DALT 10.0


static void xycsv(uint32_t px, uint32_t py, struct aprsstr_POSITION pos,
                char flush, char cont)
{
   float nf;
   float ka;
   float ky;
   float kx;
   float a0;
   float y00;
   float x0;
   float dx;
   float resol;
   float alt;
   uint32_t i;
   uint32_t n;
   char eov;
   alt = libsrtm_getsrtm(pos, 0UL, &resol);
   if (cont) {
      n = csvvec.act+1UL;
      if (n>2UL || n>0UL && flush) {
         if (flush || n>=999UL) eov = 1;
         else {
            nf = (float)n;
            x0 = csvvec.v[0U].p.long0;
            y00 = csvvec.v[0U].p.lat;
            a0 = csvvec.v[0U].a;
            kx = X2C_DIVR(pos.long0-x0,nf);
            ky = X2C_DIVR(pos.lat-y00,nf);
            ka = X2C_DIVR(alt-a0,(float)n);
            eov = 0;
            dx = 7.85398163375E-6f*osic_cos(pos.lat);
            i = n;
            do {
               --i;
               nf = (float)i;
               if (((i>=n || (float)fabs((x0+kx*nf)-csvvec.v[i].p.long0)
                >dx) || (float)fabs((y00+ky*nf)-csvvec.v[i].p.lat)
                >7.85398163375E-6f) || (float)fabs((a0+ka*nf)
                -csvvec.v[i].a)>10.0f) eov = 1;
            } while (!(i==0UL || eov));
         }
         if (eov) {
            wrcsv((int32_t)px, (int32_t)(py-n), radtodeg(csvvec.v[0U].p),
                 csvvec.v[0U].a, "", 1ul, "", 1ul);
            if (n>1UL) {
               wrcsv((int32_t)px, (int32_t)(py-1UL),
                radtodeg(csvvec.v[n-1UL].p), csvvec.v[n-1UL].a, "", 1ul, "",
                1ul);
            }
            n = 0UL;
         }
      }
   }
   else n = 0UL;
   csvvec.act = n;
   csvvec.v[n].a = alt;
   csvvec.v[n].p = pos;
} /* end xycsv() */

#define panorama_TESTDIST 5.0

#define panorama_ER 6.37E+6


static void raytrace(float minqual, float x0, float y00,
                float z0, float dx, float dy, float dz,
                float maxdist, float * dist, float * lum,
                float * h, float * alt, float * subpix,
                struct aprsstr_POSITION * pos, float refrac)
{
   float deltah;
   float minsp;
   float sp;
   float lastsp;
   float h2;
   float h1;
   float resol;
   float qual;
   struct aprsstr_POSITION pos1;
   *lum = 1.0f;
   qual = minqual;
   minsp = 0.0f;
   if (*dist==0.0f) lastsp = 0.0f;
   else lastsp = X2C_max_real;
   deltah = *alt;
   wgs84r(x0+dx* *dist, y00+dy* *dist, z0+dz* *dist, &pos->lat, &pos->long0,
                alt);
   *alt =  *alt*1000.0f;
   deltah = *alt-deltah;
   *subpix = 0.0f;
   do {
      wgs84r(x0+dx* *dist, y00+dy* *dist, z0+dz* *dist, &pos->lat,
                &pos->long0, alt);
      *alt =  *alt*1000.0f- *dist* *dist*refrac;
      *h = libsrtm_getsrtm(*pos, (uint32_t)X2C_TRUNCC(qual,0UL,
                X2C_max_longcard), &resol); /* ground over NN in m */
      if (*h<30000.0f) {
         sp = *alt-*h;
         if (sp>0.0f) {
            qual = sp*0.25f;
            if (qual>250.0f) qual = 250.0f;
            if (qual<minqual) qual = minqual;
            if (sp<lastsp) lastsp = sp;
            else if (minsp==0.0f) minsp = lastsp;
         }
         else {
            /* hit earth */
            pos1.lat = pos->lat+7.85398163375E-7f;
            pos1.long0 = pos->long0;
            h1 = libsrtm_getsrtm(pos1, 1UL, &resol);
            pos1.long0 = pos->long0+X2C_DIVR(7.85398163375E-7f,
                osic_cos(pos->lat));
            pos1.lat = pos->lat;
            h2 = libsrtm_getsrtm(pos1, 1UL, &resol);
            *lum = osic_cos(osic_arctan((h1-*h)*0.2f))
                *osic_cos(osic_arctan((h2-*h)*0.4f));
            /*
                    pos1.lat:=pos.lat - TESTDIST/2000000.0*PI;
                    pos1.long:=pos.long;
                    h1:=getsrtm(pos1, 1, resol);
                    pos1.long:=pos.long - (TESTDIST/2000000.0*PI)
                /cos(pos.lat);
                    pos1.lat:=pos.lat;
                    h2:=getsrtm(pos1, 1, resol);
                    lum:=lum + cos(arctan((h1-h)*(0.1/TESTDIST)))
                *cos(arctan((h2-h)*(0.1/TESTDIST)));
                    lum:=lum*0.5;
            */
            if (deltah!=0.0f) {
               *subpix = X2C_DIVR(minsp,deltah);
               if (*subpix>1.0f) *subpix = 1.0f;
            }
            return;
         }
      }
      else qual = resol;
      *dist = *dist+qual;
   } while (*dist<=maxdist);
   /*IF verb THEN WrFixed(dist, 1, 7); WrFixed(qual, 1, 7); WrFixed(h, 1, 7);
                 WrFixed(sp, 1, 7); WrStrLn(" d,q,h,sp") END; */
   /* we are in dust or heaven */
   if (deltah!=0.0f) {
      *subpix = X2C_DIVR(minsp,deltah);
      if (*subpix>1.0f) *subpix = 1.0f;
   }
} /* end raytrace() */
/* antialiasing: search for peaks and set highest pixel smooth */


static void rotvector(float * a, float * b, float cw, float sw)
{
   float h;
   h =  *a*cw+ *b*sw;
   *b =  *b*cw- *a*sw;
   *a = h;
} /* end rotvector() */

#define panorama_ERRALT 30000

#define panorama_FULLUM 1000.0
/* full bright in image */

#define panorama_FOGLUM 300.0

#define panorama_MAXHP 0.08
/* vertical spatial luminance highpass */

#define panorama_DDIST 50.0


static void Panofind(uint32_t csvstep, const struct PANOWIN panpar,
                float * res, struct aprsstr_POSITION * pos)
{
   uint32_t yi;
   uint32_t xi;
   int32_t nn;
   float sloped;
   float refrac;
   float oob;
   float oog;
   float oor;
   float ob;
   float og;
   float or;
   float clat;
   float slat;
   float wy;
   float wx;
   float tree;
   float light;
   float llum;
   float lum;
   float lastlum;
   float dlum;
   float lummul;
   float atx;
   float oldh;
   float d;
   float azi;
   float zn;
   float yn;
   float xn;
   float clong;
   float slong;
   float ele0;
   float eled;
   float azid;
   float resoltx;
   float space;
   float lasth;
   float maxdist;
   float hc1;
   float hc;
   float lb;
   float lg;
   float lr;
   float hb;
   float hg;
   float hr;
   float z0;
   float y00;
   float x0;
   float azi0;
   char firsty;
   char heaven;
   struct imagetext_PIX * anonym;
   if ((!posvalid(panpar.eye) || !posvalid(panpar.horizon))
                || libsrtm_getsrtm(panpar.horizon, 0UL, &resoltx)>=30000.0f) {
      Error("no altitude data at view point", 31ul); /* no alt at horizon */
   }
   maxdist = aprspos_distance(panpar.eye, panpar.horizon)*1000.0f;
   if (maxdist<100.0f) return;
   /* horizon too near */
   hr = (float)heavencol.r*0.01f;
   hg = (float)heavencol.g*0.01f;
   hb = (float)heavencol.b*0.01f;
   lummul = X2C_DIVR(1000.0f,maxdist);
   nn = (int32_t)X2C_TRUNCI(libsrtm_getsrtm(panpar.eye, 0UL, &resoltx),
                X2C_min_longint,X2C_max_longint);
                /* altitude of tx and srtm resolution in m here */
   if (nn>=30000L) {
      libsrtm_closesrtmfile();
      Error("no altitude data at camera position", 36ul);
   }
   /*  dustfade:=1.0-80.0/FLOAT(HIGH(panpar.image^[0])); */
   atx = (float)(nn+panpar.eyealt); /* ant1 over NN */
   wgs84s(panpar.eye.lat, panpar.eye.long0, atx*0.001f, &x0, &y00, &z0);
   azi = aprspos_azimuth(panpar.eye, panpar.horizon)*1.7453292519444E-2f;
   azi0 = panpar.angle0*0.5f*1.7453292519444E-2f;
   if (verb) {
      osic_WrFixed(X2C_DIVR(azi*180.0f,3.1415926535f), 1L, 7UL);
      osi_WrStrLn(" cam azimuth", 13ul);
   }
   /*WrFixed(panpar.eye.lat*180.0/PI, 1, 7);WrStrLn(" lat"); */
   azid = X2C_DIVR(panpar.angle0*1.7453292519444E-2f,
                (float)((panpar.image0->Len1-1)+1UL));
                /* h rad per pixel */
   /*WrFixed(azid*180.0/PI, 2, 7);WrStrLn(" deg/pixel"); */
   eled = X2C_DIVR(azid,panpar.yzoom0);
   ele0 = (panpar.elevation-X2C_DIVR((X2C_DIVR(panpar.angle0*0.5f,
                panpar.yzoom0))*(float)((panpar.image0->Len0-1)+1UL),
                (float)((panpar.image0->Len1-1)+1UL)))
                *1.7453292519444E-2f;
   slat = osic_sin(-panpar.eye.lat);
   clat = osic_cos(-panpar.eye.lat);
   slong = osic_sin(-panpar.eye.long0);
   clong = osic_cos(-panpar.eye.long0);
   xi = 0UL;
   refrac = panpar.refract*7.85E-8f;
   do {
      wx = azid*(float)xi-azi0;
      if (panpar.flatscreen0) wx = osic_arctan(wx);
      wx = wx+azi;
      yi = 0UL;
      d = 0.0f;
      dlum = 0.0f;
      lastlum = 0.0f;
      heaven = 0;
      firsty = 0;
      do {
         wy = ele0+eled*(float)yi;
         if (wy>=(-1.5690509974981f) && wy<1.5690509974981f) {
            if (!heaven) {
               if (panpar.flatscreen0) wy = osic_arctan(wy);
               xn = -osic_sin(wy);
               yn = osic_sin(wx)*osic_cos(wy);
               zn = osic_cos(wx)*osic_cos(wy);
               rotvector(&zn, &xn, clat, slat);
               rotvector(&yn, &xn, clong, slong);
               raytrace(25.0f, x0, y00, z0, xn*(-0.001f), yn*0.001f,
                zn*0.001f, maxdist, &d, &light, &oldh, &lasth, &space, pos,
                refrac);
               if (d>maxdist) heaven = 1;
               if (csvstep && xi%csvstep==0UL) {
                  xycsv(xi, yi, *pos, heaven, firsty);
               }
            }
            else light = 1.0f;
            if (heaven) {
               d = maxdist*4.0f*(1.25f-X2C_DIVR((float)yi,
                (float)(panpar.image0->Len0-1)));
            }
            lum = d*0.2f;
            llum = (lum-lastlum)*20.0f;
            if (llum>0.08f*maxdist) llum = 0.08f*maxdist;
            if (llum>dlum) dlum = llum;
            lastlum = lum; /* lum is distance */
            if (!heaven) {
               sloped = X2C_DIVR(panpar.angle0*d,9.E+5f);
               if (sloped>1.0f) sloped = 1.0f;
               tree = 1600.0f-oldh;
               if (tree<0.0f) tree = 0.0f;
               else if (tree>600.0f) tree = 600.0f;
               /*        tree:=tree*(0.95/600.0)*(1.0-d/maxdist); */
               tree = tree*1.5833333333333E-3f;
               /*WrFixed(light, 5,2); WrStr(" "); */
               light = light-0.65f; /* elevation of normalvector */
               if (light<0.0f) {
                  /* rock */
                  light = light*(-2.5f);
                  if (light>2.0f) light = 2.0f;
                  /*            light:=0.05 + light  (**(1.0-d/maxdist)*);
                        (* 1..2 from elevation *) */
                  light = light*5.0f*(2.5f-tree)*sloped;
               }
               else if (light<9.5f) {
                  /*
                              lr:=light*800;
                              lg:=light*520;
                              lb:=light*220;
                  */
                  /*
                              lr:=lum*light*1.4 + dlum;
                              lg:=lum*light*1.1 + dlum;
                              lb:=lum*light*0.8 + dlum;
                  */
                  /*
                              light:=lum*light*(1.7-tree);
                              lr:=light*1.4;
                              lg:=light*1.1;
                              lb:=light*0.8;
                  */
                  /* wood */
                  /*            light:=light-0.2; */
                  light = light*(-0.5f);
                  if (light>1.0f) light = 1.0f;
                  light = light*5.0f*(2.5f-tree)*sloped;
               }
               else light = 0.0f;
               /*
                           lr:=lum*(1.7-tree*1.3) + dlum;
                           lg:=lum*(1.7-tree*0.8) + dlum;
                           lb:=lum*(1.7-tree*1.7) + dlum;
               */
               /*
               
                           lr:=50-tree*30 + light*80;
                           lg:=70-tree*50 + light*50;
                           lb:=30-tree*20 + light*20;
               */
               lr = (40.0f-tree*30.0f)+light*80.0f;
               lg = (50.0f-tree*40.0f)+light*50.0f;
               lb = (20.0f-tree*20.0f)+light*20.0f;
               lr = 300.0f-(1.0f-X2C_DIVR(d,maxdist))*(300.0f-lr);
               lg = 300.0f-(1.0f-X2C_DIVR(d,maxdist))*(300.0f-lg);
               lb = 300.0f-(1.0f-X2C_DIVR(d,maxdist))*(300.0f-lb);
               lr = lr+X2C_DIVR(dlum,50.0f);
               lg = lg+X2C_DIVR(dlum,50.0f);
               lb = lb+X2C_DIVR(dlum,50.0f);
            }
            else {
               /*
                         IF lr>maxdist THEN lr:=maxdist END;
                         IF lg>maxdist THEN lg:=maxdist END;
                         IF lb>maxdist THEN lb:=maxdist END;
                         lr:=lr*lummul;
                         lg:=lg*lummul;
                         lb:=lb*lummul;
               */
               lum = lum+dlum;
               if (lum>maxdist) lum = maxdist;
               lum = lum*lummul;
            }
            { /* with */
               struct imagetext_PIX * anonym = &panpar.image0->Adr[(xi)
                *panpar.image0->Len0+yi];
               if (heaven) {
                  /* heaven */
                  /*          hc:=VAL(REAL, 2*yi)/VAL(REAL,
                HIGH(panpar.image^[0]))-1.0; */
                  /*          hc:=(ele0 + eled*FLOAT(yi))*6.0; */
                  hc = X2C_DIVR(((float)
                yi+X2C_DIVR(panpar.elevation*1.7453292519444E-2f,azid))*2.0f,
                (float)(panpar.image0->Len0-1))-1.0f;
                  if (hc<0.0f) hc = 0.0f;
                  else if (hc>1.0f) hc = 1.0f;
                  hc1 = 1.0f-hc;
                  anonym->r = (uint16_t)truncc(lum*hc1+lum*hr*hc);
                  anonym->g = (uint16_t)truncc(lum*hc1+lum*hg*hc);
                  anonym->b = (uint16_t)truncc(lum*hc1+lum*hb*hc);
               }
               else {
                  anonym->r = (uint16_t)truncc(lr);
                  anonym->g = (uint16_t)truncc(lg);
                  anonym->b = (uint16_t)truncc(lb);
               }
               or = (float)anonym->r;
               og = (float)anonym->g;
               ob = (float)anonym->b;
               if (firsty) {
                  /* antialiasing */
                  anonym->r = (uint16_t)truncc(or*space+oor*(1.0f-space));
                  anonym->g = (uint16_t)truncc(og*space+oog*(1.0f-space));
                  anonym->b = (uint16_t)truncc(ob*space+oob*(1.0f-space));
               }
               firsty = 1;
               oor = or;
               oog = og;
               oob = ob;
            }
            dlum = dlum*((1.0f-(X2C_DIVR(d,maxdist))*0.15f)-X2C_DIVR(80.0f,
                (float)(panpar.image0->Len0-1)));
                /* fade out distance highpass peak */
         }
         ++yi;
      } while (yi<=panpar.image0->Len0-1);
      /*WrInt(xi, 5); WrFixed(pos.lat*180.0/pi, 4,8);
                WrFixed(pos.long*180.0/pi, 4,8); WrStrLn(" xi lat long az");
                */
      ++xi;
   } while (xi<=panpar.image0->Len1-1);
} /* end Panofind() */


static void Panorama1(const struct PANOWIN panpar)
{
   float res;
   struct aprsstr_POSITION pos;
   Panofind(rastercsv, panpar, &res, &pos);
} /* end Panorama1() */

/* ------- poi resverse raytrace */

static char drawpoiicon(float xi, float yi,
                struct PANOWIN panpar, const SYMBOL sym, uint32_t xs,
                uint32_t ys, uint32_t cr, uint32_t cg, uint32_t cb,
                uint32_t calpha)
{
   uint32_t ar;
   uint32_t a;
   uint32_t iyr;
   uint32_t ix;
   uint32_t iy;
   uint32_t y;
   uint32_t x;
   char ok0;
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   ok0 = 0;
   if (xi<=0.0f || yi<=0.0f) return 0;
   x = (uint32_t)X2C_TRUNCC(xi+0.5f,0UL,X2C_max_longcard);
   if (x>=xs && x+xs<=panpar.image0->Len1-1) {
      x = (uint32_t)X2C_TRUNCC((xi+0.5f)-(float)xs*0.5f,0UL,
                X2C_max_longcard);
      y = (uint32_t)X2C_TRUNCC(yi+0.5f,0UL,X2C_max_longcard);
      tmp = ys-1UL;
      iy = 0UL;
      if (iy<=tmp) for (;; iy++) {
         iyr = (ys-iy)-1UL;
         if (iy+y<=panpar.image0->Len0-1) {
            tmp0 = xs-1UL;
            ix = 0UL;
            if (ix<=tmp0) for (;; ix++) {
               { /* with */
                  struct imagetext_PIX * anonym = &panpar.image0->Adr[(x+ix)
                *panpar.image0->Len0+(y+iy)];
                  a = ((uint32_t)sym[iyr][ix].alpha*calpha)/256UL;
                  ar = 255UL-a;
                  /*WrInt(sym[iyr][ix].alpha, 4); */
                  anonym->r = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].r8*cr)/64UL+ar*(uint32_t)anonym->r)/255UL);
                  anonym->g = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].g8*cg)/64UL+ar*(uint32_t)anonym->g)/255UL);
                  anonym->b = (uint16_t)(((a*(uint32_t)
                sym[iyr][ix].b8*cb)/64UL+ar*(uint32_t)anonym->b)/255UL);
               }
               if (ix==tmp0) break;
            } /* end for */
         }
         if (iy==tmp) break;
      } /* end for */
      ok0 = 1;
   }
   return ok0;
} /* end drawpoiicon() */


static char drawpoi(float xi, float yi, struct PANOWIN panpar,
                uint32_t size, int32_t cr, int32_t cg, int32_t cb,
                int32_t calpha)
{
   uint32_t ix;
   uint32_t iy;
   uint32_t y;
   uint32_t x;
   int32_t n;
   int32_t s;
   int32_t c;
   int32_t a;
   char ok0;
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   uint32_t tmp0;
   ok0 = 0;
   if (xi<=0.0f || yi<=0.0f) return 0;
   x = (uint32_t)X2C_TRUNCC(xi+0.5f,0UL,X2C_max_longcard);
   y = (uint32_t)X2C_TRUNCC(yi+0.5f,0UL,X2C_max_longcard);
   if (x>=size && x+size<=panpar.image0->Len1-1) {
      tmp = size*2UL-1UL;
      iy = 0UL;
      if (iy<=tmp) for (;; iy++) {
         if (iy+y<=panpar.image0->Len0-1) {
            tmp0 = size*2UL-1UL;
            ix = 0UL;
            if (ix<=tmp0) for (;; ix++) {
               a = labs((int32_t)ix-(int32_t)size);
               c = 0L;
               a = (int32_t)iy-2L*a;
               if (a==-1L) c = 256L;
               else if (a==0L) c = 768L;
               else if (a>0L) c = 1024L;
               if (c>0L) {
                  s = 0L;
                  if (c<1000L) {
                     if (ix<size) {
                        if (iy!=size*2UL-1UL) s = 200L;
                     }
                     else s = -400L;
                  }
                  else if (iy==size*2UL-1UL) {
                     c = 500L;
                     s = -300L;
                  }
                  c = (c*calpha)/256L;
                  { /* with */
                     struct imagetext_PIX * anonym = &panpar.image0->Adr[((x+ix)
                -size)*panpar.image0->Len0+(y+iy)];
                     n = ((1024L-c)*(int32_t)anonym->r)/1024L+(c*cr)
                /256L+s;
                     if (n<0L) anonym->r = 0U;
                     else anonym->r = (uint16_t)n;
                     n = ((1024L-c)*(int32_t)anonym->g)/1024L+(c*cg)
                /256L+s;
                     if (n<0L) anonym->g = 0U;
                     else anonym->g = (uint16_t)n;
                     n = ((1024L-c)*(int32_t)anonym->b)/1024L+(c*cb)
                /256L+s;
                     if (n<0L) anonym->b = 0U;
                     else anonym->b = (uint16_t)n;
                  }
               }
               if (ix==tmp0) break;
            } /* end for */
            ok0 = 1;
         }
         if (iy==tmp) break;
      } /* end for */
   }
   return ok0;
} /* end drawpoi() */

#define panorama_MINQ 5.0


static void poipixel(struct aprsstr_POSITION ppos, float palt,
                const struct PANOWIN panpar, int32_t * px, int32_t * py,
                pPOI pp)
{
   float refrac;
   float yi;
   float xi;
   float d2;
   float aeye;
   float nnp;
   float nne;
   float zn;
   float yn;
   float xn;
   float z1;
   float y1;
   float x1;
   float z0;
   float y00;
   float x0;
   float eled;
   float ele0;
   float azi;
   float azi0;
   float azid;
   float resoltx;
   float subpix;
   float ralt;
   float rh;
   float lum;
   float rd;
   struct aprsstr_POSITION rpos;
   char ok0;
   struct POI * anonym;
   *px = -1L;
   *py = -1L;
   if (!posvalid(ppos)) return;
   refrac = panpar.refract*7.85E-8f;
   nne = libsrtm_getsrtm(panpar.eye, 0UL, &resoltx);
   nnp = libsrtm_getsrtm(ppos, 0UL, &resoltx);
   if (altOG) {
      if (palt<minpoialt) palt = minpoialt;
      nnp = nnp+palt;
   }
   else {
      nnp = nnp+minpoialt;
      if (nnp<palt) nnp = palt;
   }
   aeye = nne+(float)panpar.eyealt;
   wgs84s(panpar.eye.lat, panpar.eye.long0, aeye*0.001f, &x0, &y00, &z0);
   wgs84s(ppos.lat, ppos.long0, nnp*0.001f, &x1, &y1, &z1);
   d2 = osic_sqrt(sqr(x1-x0)+sqr(y1-y00)+sqr(z1-z0))*1000.0f;
   wgs84s(ppos.lat, ppos.long0, (nnp+refrac*d2*d2)*0.001f, &x1, &y1, &z1);
   d2 = osic_sqrt(sqr(x1-x0)+sqr(y1-y00)+sqr(z1-z0))*1000.0f;
   xn = X2C_DIVR(x1-x0,d2);
   yn = X2C_DIVR(y1-y00,d2);
   zn = X2C_DIVR(z1-z0,d2);
   rd = 0.0f;
   raytrace(5.0f, x0, y00, z0, xn, yn, zn, d2, &rd, &lum, &rh, &ralt,
                &subpix, &rpos, refrac);
   /*WrFixed(d2, 1, 8); WrFixed(rd, 1, 8); WrStrLn("=d,rd"); */
   if (rd+5.0f>=d2) {
      /* poi visable */
      zn = 1000.0f*zn;
      yn = 1000.0f*yn;
      xn = -(1000.0f*xn);
      /*WrFixed(xn,2,6);WrFixed(yn,2,6);WrFixed(zn,2,6); WrStrLn(" xyz1"); */
      rotvector(&yn, &xn, osic_cos(panpar.eye.long0),
                osic_sin(panpar.eye.long0));
      rotvector(&zn, &xn, osic_cos(panpar.eye.lat),
                osic_sin(panpar.eye.lat));
      /*WrFixed(xn,2,6);WrFixed(yn,2,6);WrFixed(zn,2,6); WrStrLn(" xyz2"); */
      azi = aprspos_azimuth(panpar.eye, panpar.horizon)*1.7453292519444E-2f;
      azi0 = panpar.angle0*0.5f*1.7453292519444E-2f;
      azid = X2C_DIVR(panpar.angle0*1.7453292519444E-2f,
                (float)((panpar.image0->Len1-1)+1UL));
                /* h rad per pixel */
      xi = atang2(zn, yn)-azi;
      /*WrFixed(xi*180.0/PI,2,8); WrStr("=xi "); */
      if (xi<(-3.1415926535f)) xi = xi+6.283185307f;
      else if (xi>3.1415926535f) xi = xi-6.283185307f;
      if (panpar.flatscreen0) xi = osic_tan(xi);
      /*WrFixed(xi*180.0/PI,2,8); WrStr("=axi "); */
      xi = X2C_DIVR(xi+azi0,azid);
      /*WrFixed(xi,1,8); */
      eled = X2C_DIVR(azid,panpar.yzoom0); /* v deg per pixel */
      ele0 = (panpar.elevation-X2C_DIVR((X2C_DIVR(panpar.angle0*0.5f,
                panpar.yzoom0))*(float)((panpar.image0->Len0-1)+1UL),
                (float)((panpar.image0->Len1-1)+1UL)))
                *1.7453292519444E-2f;
      yi = atang2(osic_sqrt(zn*zn+yn*yn), -xn);
      if (panpar.flatscreen0) yi = osic_tan(yi);
      yi = X2C_DIVR(yi-ele0,eled);
      /*WrFixed(yi,1,8); WrStrLn(""); */
      { /* with */
         struct POI * anonym = pp;
         if (anonym->xs==0UL) {
            ok0 = drawpoi(xi, yi, panpar, poisize, (int32_t)anonym->r,
                (int32_t)anonym->g, (int32_t)anonym->b,
                (int32_t)anonym->alpha);
         }
         else {
            ok0 = drawpoiicon(xi, yi, panpar, anonym->symbol, anonym->xs,
                anonym->ys, anonym->r, anonym->g, anonym->b, anonym->alpha);
         }
      }
      if (ok0) {
         *px = (int32_t)X2C_TRUNCI(xi+0.5f,X2C_min_longint,
                X2C_max_longint);
         *py = (int32_t)X2C_TRUNCI(yi+0.5f,X2C_min_longint,
                X2C_max_longint);
      }
   }
} /* end poipixel() */


static void setpoi(const struct PANOWIN panpar,
                const struct aprsstr_POSITION posdeg, float alti,
                const char text[], uint32_t text_len, pPOI pp)
{
   float resol;
   float alt;
   float d;
   float az;
   struct aprsstr_POSITION pos;
   int32_t py;
   int32_t px;
   pos.lat = posdeg.lat*1.7453292519444E-2f;
   pos.long0 = posdeg.long0*1.7453292519444E-2f;
   /*WrFixed(posdeg.lat,2,9); WrStr(" "); */
   /*WrFixed(posdeg.long,2,9); WrStr(" "); */
   d = aprspos_distance(panpar.eye, pos);
   if (d>panpar.dist || d<0.05f) return;
   az = panpar.ang-aprspos_azimuth(panpar.eye, pos)*1.7453292519444E-2f;
   if (az<(-3.1415926535f)) az = az+6.283185307f;
   else if (az>3.1415926535f) az = az-6.283185307f;
   if ((float)fabs(az)>panpar.hseg) return;
   poipixel(pos, alti, panpar, &px, &py, pp);
   if (verb) {
      osic_WrFixed(aprspos_distance(panpar.eye, pos), 3L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(aprspos_azimuth(panpar.eye, pos), 2L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(X2C_DIVR(az,1.7453292519444E-2f), 2L, 9UL);
      osi_WrStr(" ", 2ul);
      osic_WrFixed(alti, 0L, 9UL);
      osi_WrStr(" ", 2ul);
      osi_WrStr(text, text_len);
      if (px<0L) {
         osi_WrStr(" #", 3ul);
      }
      osi_WrStrLn("", 1ul);
   }
   if (px>=0L) {
      alt = libsrtm_getsrtm(pos, 0UL, &resol);
      if (alti>=10000.0f || alti<alt) alti = alt;
      wrcsv(px, py, posdeg, alti, text, text_len, pp->iconhint, 1024ul);
   }
} /* end setpoi() */

/* ------- poi resverse raytrace */
#define panorama_POIINFOSIZE 4096


static char getch(char b[4096], int32_t fd, int32_t * len,
                int32_t * p)
{
   if (*p>=*len) {
      *len = osi_RdBin(fd, (char *)b, 4096u/1u, 4096UL);
      if (*len<=0L) return 0;
      *p = 0L;
   }
   ++*p;
   return b[*p-1L];
} /* end getch() */


static int32_t getword(int32_t * p, int32_t * len, int32_t fd,
                char b[4096], char s[], uint32_t s_len)
{
   uint32_t i;
   char inqu;
   i = 0UL;
   inqu = 0;
   for (;;) {
      s[i] = getch(b, fd, len, p);
      if (s[i]==0) return -1L;
      if (s[i]=='\012') {
         s[i] = 0;
         return 0L;
      }
      if (!inqu && s[i]==',') {
         s[i] = 0;
         return 1L;
      }
      if (s[i]=='\"') inqu = !inqu;
      else if ((i<s_len-1 && (uint8_t)s[i]>=' ') && (uint8_t)
                s[i]<(uint8_t)'\200') ++i;
   }
   return 0;
} /* end getword() */


static void rdmountain(const struct PANOWIN panpar, char fn[],
                uint32_t fn_len, pPOI pp)
/* import csv file with mountain name, pos, altitude */
{
   int32_t r;
   int32_t len;
   int32_t p;
   int32_t fd;
   struct aprsstr_POSITION pos;
   float alt;
   char b[4096];
   char s[1024];
   char text[4097];
   char name[100];
   char long0[100];
   char lat[100];
   char com[100];
   X2C_PCOPY((void **)&fn,fn_len);
   fd = osi_OpenRead(fn, fn_len);
   if (!osic_FdValid(fd)) Error("poi-file not found", 19ul);
   p = 0L;
   len = 0L;
   for (;;) {
      r = getword(&p, &len, fd, b, com, 100ul);
      if (r>0L) {
         r = getword(&p, &len, fd, b, name, 100ul);
         if (r>0L) {
            r = getword(&p, &len, fd, b, text, 4097ul);
            if (r>0L) {
               r = getword(&p, &len, fd, b, lat, 100ul);
               if (r>0L) {
                  r = getword(&p, &len, fd, b, long0, 100ul);
                  if (r>0L) {
                     r = getword(&p, &len, fd, b, s, 1024ul);
                     if (r<0L || !aprsstr_StrToFix(&alt, s, 1024ul)) {
                        alt = (-3.E+4f);
                     }
                     while (r>0L) r = getword(&p, &len, fd, b, s, 1024ul);
                  }
                  else alt = 0.0f;
               }
            }
         }
      }
      if (r<0L) break;
      if ((((com[0U]!='#' && name[0U]) && aprsstr_StrToFix(&pos.lat, lat,
                100ul)) && aprsstr_StrToFix(&pos.long0, long0,
                100ul)) && posvalid(pos)) {
         setpoi(panpar, pos, alt, name, 100ul, pp);
      }
   }
   osic_Close(fd);
   X2C_PFREE(fn);
} /* end rdmountain() */


static void rdmountains(const struct PANOWIN panpar)
{
   pPOI pp;
   pp = poifiles;
   while (pp) {
      pp->xs = 0UL;
      pp->ys = 0UL;
      if (pp->iconfn[0U]) {
         readsymbol(pp->symbol, &pp->xs, &pp->ys, pp->iconfn, 1024ul);
      }
      rdmountain(panpar, pp->poifn, 1024ul, pp);
      pp = pp->next;
   }
} /* end rdmountains() */


static void drawscale(const struct PANOWIN panpar, uint32_t px,
                uint32_t deg)
{
   uint32_t y;
   char s[101];
   struct imagetext_PIX * anonym;
   uint32_t tmp;
   if (px>2UL && px+2UL<=panpar.image0->Len1-1) {
      tmp = scalesize/10UL;
      y = 0UL;
      if (y<=tmp) for (;; y++) {
         { /* with */
            struct imagetext_PIX * anonym = &panpar.image0->Adr[(px)
                *panpar.image0->Len0+y];
            anonym->r = anonym->r+500U;
            anonym->g = anonym->g+500U;
         }
         if (y==tmp) break;
      } /* end for */
      if (px+scalesize<=panpar.image0->Len1-1) {
         if (deg==0UL) strncpy(s,"N",101u);
         else if (deg==90UL) strncpy(s,"E",101u);
         else if (deg==180UL) strncpy(s,"S",101u);
         else if (deg==270UL) strncpy(s,"W",101u);
         else aprsstr_IntToStr((int32_t)deg, 0UL, s, 101ul);
         imagetext_writestr(panpar.image0, px+2UL, 0UL, fonttyp, 200UL,
                200UL, 200UL, s, 101ul);
      }
   }
} /* end drawscale() */


static void scale(const struct PANOWIN panpar)
{
   uint32_t i;
   uint32_t w;
   uint32_t d;
   uint32_t b;
   uint32_t a;
   float wx;
   float azid;
   float azi;
   uint32_t tmp;
   azid = X2C_DIVR(panpar.angle0,(float)((panpar.image0->Len1-1)+1UL));
                /* deg / pixel */
   d = (uint32_t)X2C_TRUNCC((float)scalesize*azid,0UL,X2C_max_longcard);
   a = 1UL;
   b = 1UL;
   while (a*b<d) {
      if (a==1UL) a = 2UL;
      else if (a==2UL) a = 5UL;
      else if (a==5UL) {
         a = 1UL;
         b = b*10UL;
      }
   }
   a = a*b; /* 1 2 5 10 deg / step */
   if (a>90UL) a = 180UL;
   else if (a>60UL) a = 90UL;
   else if (a>20UL) a = 60UL;
   /*WrInt(a, 10); WrStrLn(" deg/ step"); */
   azi = aprspos_azimuth(panpar.eye, panpar.horizon);
   wx = azi-panpar.angle0*0.5f;
   if (wx<0.0f) wx = wx+360.0f;
   w = ((uint32_t)X2C_TRUNCC(wx,0UL,X2C_max_longcard)/a)*a;
   tmp = ((uint32_t)X2C_TRUNCC(panpar.angle0,0UL,X2C_max_longcard)+a)/a;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /*WrInt(w, 4); */
      wx = ((float)w-azi)*1.7453292519444E-2f;
      if (wx<(-3.1415926535f)) wx = wx+6.283185307f;
      else if (wx>3.1415926535f) wx = wx-6.283185307f;
      if (panpar.flatscreen0) wx = osic_tan(wx);
      wx = X2C_DIVR(wx*5.729577951472E+1f+panpar.angle0*0.5f,azid);
      if (wx>0.0f) {
         drawscale(panpar, (uint32_t)X2C_TRUNCC(wx+0.5f,0UL,
                X2C_max_longcard), w);
      }
      /*WrFixed(wx,1, 7); */
      w += a;
      if (w>=360UL) w -= 360UL;
      if (i==tmp) break;
   } /* end for */
/*WrStrLn(""); */
} /* end scale() */


static void drawimage(void)
{
   struct PANOWIN panowin;
   /*sym:SYMBOL; */
   /*readsymbol(sym, "/tmp/s1.png"); */
   panowin.image0 = image;
   panowin.flatscreen0 = flatscreen;
   panowin.eye = posa;
   panowin.horizon = posb;
   panowin.angle0 = angle;
   panowin.elevation = elev;
   panowin.eyealt = (int32_t)X2C_TRUNCI(alta,X2C_min_longint,
                X2C_max_longint);
   panowin.yzoom0 = yzoom;
   panowin.refract = refraction;
   panowin.ang = aprspos_azimuth(panowin.eye,
                panowin.horizon)*1.7453292519444E-2f;
   panowin.hseg = angle*1.7453292519444E-2f*0.5f;
   panowin.dist = aprspos_distance(panowin.eye, panowin.horizon);
   if (imagefn[0U]) Panorama1(panowin);
   if (poifiles) rdmountains(panowin);
   if (fonttyp>0UL) scale(panowin);
} /* end drawimage() */


X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   size_t tmp[2];
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   imagetext_BEGIN();
   aprsstr_BEGIN();
   aprspos_BEGIN();
   libsrtm_BEGIN();
   osi_BEGIN();
   imagefn[0] = 0;
   poifiles = 0;
   csvfn[0] = 0;
   csvfd = -1L;
   image = 0;
   xsize = 600L;
   ysize = 400L;
   posinval(&posa);
   posinval(&posb);
   alta = 10.0f;
   refraction = 0.13f;
   igamma = 2.2f;
   libsrtm_srtmmaxmem = 100000000UL;
   heavencol.r = 50UL;
   heavencol.g = 70UL;
   heavencol.b = 300UL;
   elev = 0.0f;
   angle = 45.0f;
   yzoom = 1.0f;
   flatscreen = 0;
   poisize = 5UL;
   minpoialt = 1.0f;
   fonttyp = 0UL;
   altOG = 0;
   verb = 0;
   libsrtm_bicubic = 0;
   rastercsv = 0UL;
   Parms();
   if (xsize<20L) Error("xsize too less", 15ul);
   if (ysize<20L) Error("ysize too less", 15ul);
   if (angle>=180.0f && flatscreen) {
      Error("sight > 180 deggrees needs courved screen on", 45ul);
   }
   /*  IF imagefn[0]=0C THEN Error("need inage filename") END; */
   scalesize = imagetext_fontsizex(fonttyp)*7UL;
   /*WrInt(scalesize, 5); WrStrLn("scale"); */
   X2C_DYNALLOCATE((char **) &image,sizeof(struct imagetext_PIX),
                (tmp[0] = (size_t)xsize,tmp[1] = (size_t)ysize,tmp),2u);
   if (image==0) Error("out of memory", 14ul);
   drawimage();
   libsrtm_closesrtmfile();
   if (csvfd>=0L) osic_Close(csvfd);
   if (imagefn[0U]) wrpng();
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
