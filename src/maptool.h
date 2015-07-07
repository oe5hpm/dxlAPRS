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
#ifndef aprspos_H_
#include "aprspos.h"
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

#define maptool_CHARWIDTH 6

#define maptool_MAXCHAR 129
/* highest font chars */

#define maptool_SYMSIZE 16

struct maptool_PIX;


struct maptool_PIX {
   unsigned short r;
   unsigned short g;
   unsigned short b;
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
   struct aprspos_POSITION eye;
   struct aprspos_POSITION horizon;
   long eyealt;
   float angle;
   float elevation;
   float yzoom;
   unsigned long actx;
   long hx; /* mouse pos on panwin and fullwin */
   long hy;
   long mx;
   long my;
   long ximg;
   long yimg;
   maptool_pIMAGE image;
};

extern long maptool_xsize;

extern long maptool_ysize;

extern float maptool_shiftx;

extern float maptool_shifty;

struct maptool__D0;


struct maptool__D0 {
   struct aprspos_POSITION leftup;
   struct aprspos_POSITION rightdown;
   long tozoom;
   unsigned long retrys;
   unsigned long givups;
   unsigned long mapscnt;
   unsigned long needcnt;
   unsigned long donecnt;
   unsigned long delay;
   unsigned long retrysum;
   long zoom;
   long tx;
   long ty;
   char run;
   char overflow;
};

extern struct maptool__D0 maptool_mappack;

extern void maptool_Colset(struct aprsdecode_COLTYP *, char);

extern void maptool_waypoint(maptool_pIMAGE, float, float, float, long, long,
                 long);

extern void maptool_vector(maptool_pIMAGE, float, float, float, float, long,
                long, long, unsigned long, float);

extern void maptool_drawchar(maptool_pIMAGE, char, float, float, long *,
                unsigned long, unsigned long, struct aprsdecode_COLTYP,
                char);

extern void maptool_drawstr(maptool_pIMAGE, char [], unsigned long, float,
                float, unsigned long, unsigned long,
                struct aprsdecode_COLTYP, signed char *, unsigned long, char,
                 char);

extern void maptool_drawstri(maptool_pIMAGE, char [], unsigned long, long,
                long, unsigned long, unsigned long, struct aprsdecode_COLTYP,
                 char, char);

extern void maptool_drawsym(maptool_pIMAGE, char, char, char, float, float,
                unsigned long);

extern void maptool_drawarrow(maptool_pIMAGE, float, float, float, float,
                unsigned long, unsigned long, struct aprsdecode_COLTYP);

extern void maptool_shine(maptool_pIMAGE, long);

extern void maptool_loadmap(maptool_pIMAGE, long, long, long, float, float,
                float, char *, char *, char, char);

extern char maptool_IsMapLoaded(void);

extern void maptool_addmap(maptool_pIMAGE, maptool_pIMAGE);

extern void maptool_mercator(float, float, long, long *, long *, float *,
                float *);

extern long maptool_mapxy(struct aprspos_POSITION, float *, float *);

extern void maptool_xytodeg(float, float, struct aprspos_POSITION *);

extern long maptool_saveppm(char [], unsigned long, maptool_pIMAGE, long,
                long);

extern void maptool_area(maptool_pIMAGE, long, long, long, long,
                struct aprsdecode_COLTYP, char);

extern void maptool_postoloc(char [], unsigned long,
                struct aprspos_POSITION);

extern void maptool_loctopos(struct aprspos_POSITION *, char [],
                unsigned long);

extern void maptool_shiftmap(long, long, long, float,
                struct aprspos_POSITION *);

extern void maptool_center(long, long, float, struct aprspos_POSITION,
                struct aprspos_POSITION *);

extern void maptool_limpos(struct aprspos_POSITION *);

extern void maptool_makebw(maptool_pIMAGE);

extern float maptool_realzoom(long, float);

extern void maptool_setmark(maptool_pIMAGE, struct aprspos_POSITION, char);

extern void maptool_cc(maptool_pIMAGE, unsigned long, unsigned long);

extern void maptool_ruler(maptool_pIMAGE);

extern void maptool_clr(maptool_pIMAGE);

extern void maptool_xytoloc(struct aprspos_POSITION, char [], unsigned long);

extern void maptool_POIname(struct aprspos_POSITION *, char [],
                unsigned long);

extern void maptool_POIfind(struct aprspos_POSITION *, char [],
                unsigned long);

extern unsigned long maptool_charwidth0(char);

extern char maptool_vistime(unsigned long);

extern void maptool_StartMapPackage(struct aprspos_POSITION,
                struct aprspos_POSITION, long, char);

extern void maptool_MapPackageJob(char);

extern void maptool_loadfont(void);

extern void maptool_startmapdelay(void);

extern void maptool_pullmap(long, long, char);

extern long maptool_geoprofile(maptool_pIMAGE, struct aprspos_POSITION,
                struct aprspos_POSITION, float, char, long, long, float *,
                float *, float *, float *, float *);

extern float maptool_getsrtm(struct aprspos_POSITION, unsigned long,
                float *);

extern void maptool_Radiorange(maptool_pIMAGE, struct aprspos_POSITION, long,
                 long, unsigned long, unsigned long, unsigned long, char *);

extern void maptool_Panorama(maptool_pIMAGE, struct maptool_PANOWIN, char *);

extern char maptool_SimpleRelief(maptool_pIMAGE);

extern void maptool_findpanopos(struct maptool_PANOWIN,
                struct aprspos_POSITION *, float *, long *);

extern void maptool_closesrtmfile(void);

extern void maptool_rdmountains(char [], unsigned long, char);

extern void maptool_drawareasym(maptool_pIMAGE, struct aprspos_POSITION,
                struct aprsdecode_AREASYMB, unsigned long);

extern void maptool_drawpoligon(maptool_pIMAGE, struct aprspos_POSITION,
                struct aprsdecode_MULTILINE, unsigned long);


extern void maptool_BEGIN(void);


#endif /* maptool_H_ */
