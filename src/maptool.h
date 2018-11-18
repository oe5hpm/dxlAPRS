/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef maptool_H_
#define maptool_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef aprsdecode_H_
#include "aprsdecode.h"
#endif

/* aprs tracks on osm map by oe5dxl */
#define maptool_WHITELEVEL 1024

#define maptool_TILESIZE 256

#define maptool_MAXLAT 1.484

#define maptool_MINZOOM 1

#define maptool_MAXZOOM 18

#define maptool_MAXCHAR 129
/* highest font chars */

struct maptool_PIX;


struct maptool_PIX {
   uint16_t r;
   uint16_t g;
   uint16_t b;
};


struct maptool_IMAGELINE {
   struct maptool_PIX * Adr;
   size_t Len0;
};


struct maptool_IMAGE {
   struct maptool_PIX * Adr;
   size_t Len0;
   size_t Size1;
   size_t Len1;
};

typedef struct maptool_IMAGE * maptool_pIMAGE;

struct maptool_PANOWIN;


struct maptool_PANOWIN {
   char isicon;
   char empty;
   char hover;
   char on;
   char flatscreen;
   struct aprsstr_POSITION eye;
   struct aprsstr_POSITION horizon;
   int32_t eyealt;
   float angle;
   float elevation;
   float yzoom;
   uint32_t actx;
   int32_t hx; /* mouse pos on panwin and fullwin */
   int32_t hy;
   int32_t mx;
   int32_t my;
   int32_t ximg;
   int32_t yimg;
   maptool_pIMAGE image;
};

extern int32_t maptool_xsize;

extern int32_t maptool_ysize;

extern float maptool_shiftx;

extern float maptool_shifty;

struct maptool__D0;


struct maptool__D0 {
   struct aprsstr_POSITION leftup;
   struct aprsstr_POSITION rightdown;
   int32_t tozoom;
   uint32_t retrys;
   uint32_t givups;
   uint32_t mapscnt;
   uint32_t needcnt;
   uint32_t donecnt;
   uint32_t delay;
   uint32_t retrysum;
   int32_t zoom;
   int32_t tx;
   int32_t ty;
   char run;
};

extern struct maptool__D0 maptool_mappack;

extern char maptool_fontloadmsg[71];

extern void maptool_Colset(struct aprsdecode_COLTYP *, char);

extern void maptool_waypoint(maptool_pIMAGE, float, float, float,
                int32_t, int32_t, int32_t);

extern void maptool_vector(maptool_pIMAGE, float, float, float,
                float, int32_t, int32_t, int32_t, uint32_t,
                float);

extern void maptool_drawchar(maptool_pIMAGE, char, float, float,
                int32_t *, uint32_t, uint32_t,
                struct aprsdecode_COLTYP, char);

extern void maptool_drawstr(maptool_pIMAGE, char [], uint32_t,
                float, float, uint32_t, uint32_t,
                struct aprsdecode_COLTYP, signed char *, uint32_t,
                char, char);

extern void maptool_drawstri(maptool_pIMAGE, char [], uint32_t,
                int32_t, int32_t, uint32_t, uint32_t,
                struct aprsdecode_COLTYP, char, char);

extern void maptool_drawsym(maptool_pIMAGE, char, char, char,
                float, float, uint32_t);

extern void maptool_drawarrow(maptool_pIMAGE, float, float, float,
                float, uint32_t, uint32_t, struct aprsdecode_COLTYP);

extern void maptool_drawpois(maptool_pIMAGE);

extern void maptool_shine(maptool_pIMAGE, int32_t);

extern void maptool_loadmap(maptool_pIMAGE, int32_t, int32_t, int32_t,
                float, float, float, char *, char *,
                char, char);

extern char maptool_IsMapLoaded(void);

extern void maptool_addmap(maptool_pIMAGE, maptool_pIMAGE);

extern void maptool_mercator(float, float, int32_t, int32_t *,
                int32_t *, float *, float *);

extern int32_t maptool_mapxy(struct aprsstr_POSITION, float *,
                float *);

extern void maptool_xytodeg(float, float, struct aprsstr_POSITION *);

extern int32_t maptool_saveppm(char [], uint32_t, maptool_pIMAGE,
                int32_t, int32_t);

extern void maptool_area(maptool_pIMAGE, int32_t, int32_t, int32_t,
                int32_t, struct aprsdecode_COLTYP, char);

extern void maptool_shiftmap(int32_t, int32_t, int32_t, float,
                struct aprsstr_POSITION *);

extern void maptool_center(int32_t, int32_t, float,
                struct aprsstr_POSITION, struct aprsstr_POSITION *);

extern void maptool_limpos(struct aprsstr_POSITION *);

extern void maptool_makebw(maptool_pIMAGE);

extern float maptool_realzoom(int32_t, float);

extern void maptool_setmark(maptool_pIMAGE, struct aprsstr_POSITION,
                char);

extern void maptool_cc(maptool_pIMAGE, uint32_t, uint32_t);

extern void maptool_ruler(maptool_pIMAGE);

extern void maptool_clr(maptool_pIMAGE);

extern void maptool_xytoloc(struct aprsstr_POSITION, char [],
                uint32_t);

extern void maptool_POIname(struct aprsstr_POSITION *, char [],
                uint32_t, char [], uint32_t);

extern void maptool_POIfind(struct aprsstr_POSITION *, char,
                char [], uint32_t);

extern uint32_t maptool_charwidth(char);

extern char maptool_vistime(uint32_t);

extern void maptool_StartMapPackage(struct aprsstr_POSITION,
                struct aprsstr_POSITION, int32_t, char);

extern void maptool_MapPackageJob(char);

extern void maptool_loadfont(void);

extern void maptool_startmapdelay(void);

extern void maptool_pullmap(int32_t, int32_t, char);

extern int32_t maptool_geoprofile(maptool_pIMAGE, struct aprsstr_POSITION,
                struct aprsstr_POSITION, float, char, int32_t,
                int32_t, float *, float *, float *, float *,
                float *);

/*
PROCEDURE getsrtm(pos:POSITION; quality:CARDINAL; VAR resolution:REAL):REAL;
PROCEDURE closesrtmfile;
*/
extern void maptool_Radiorange(maptool_pIMAGE, struct aprsstr_POSITION,
                int32_t, int32_t, uint32_t, uint32_t, uint32_t,
                char *, float);

extern void maptool_Panorama(maptool_pIMAGE, struct maptool_PANOWIN,
                char *);

extern char maptool_SimpleRelief(maptool_pIMAGE);

extern void maptool_findpanopos(struct maptool_PANOWIN,
                struct aprsstr_POSITION *, float *, int32_t *);
/*
PROCEDURE rdmountains(fn:ARRAY OF CHAR; add:BOOLEAN);
*/

extern void maptool_drawareasym(maptool_pIMAGE, struct aprsstr_POSITION,
                struct aprsdecode_AREASYMB, uint32_t);

extern void maptool_drawpoligon(maptool_pIMAGE, struct aprsstr_POSITION,
                struct aprsdecode_MULTILINE, char, char, uint32_t);

extern void maptool_drawpoliobj(maptool_pIMAGE);

extern char maptool_findmultiline(struct aprsstr_POSITION,
                struct aprsstr_POSITION *);

extern void maptool_readpoifiles(void);

extern char maptool_poisactiv(void);


extern void maptool_BEGIN(void);


#endif /* maptool_H_ */
