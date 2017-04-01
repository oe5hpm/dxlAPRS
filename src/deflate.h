/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef deflate_H_
#define deflate_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

#define deflate_BSIZE 32768

#define deflate_HASHSIZE 4096

#define deflate_CMPSIZE 257

#define deflate_CSIZE 2048

#define deflate_SWITCHTOCOMP (-32)

#define deflate_SWITCHTORAW 32

#define deflate_MAXRAWBLOCK 16384

#define deflate_ENDCOMP 256

#define deflate_FLUSHEOF "\002"

#define deflate_FLUSHQUICK "\001"

typedef unsigned char deflate_SET8;

typedef unsigned long deflate_SET32;

struct deflate_CONTEXT;


struct deflate_CONTEXT {
   char ring[32768];
   unsigned long fill;
   unsigned short hashchain[32768];
   unsigned long fill1;
   unsigned short hash[4096];
   unsigned long fill2;
   char matchbuf[257];
   unsigned long combuf[2048];
   unsigned long txbitbuf;
   unsigned long txbitc;
   unsigned long lastidx;
   unsigned long wp;
   unsigned long savep;
   unsigned long flen;
   unsigned long fdist;
   unsigned long mlen;
   unsigned long rawr;
   unsigned long comr;
   unsigned long raww;
   unsigned long rawwo;
   unsigned long comw;
   long of;
   long sizedif;
   char wascomp;
};

struct deflate_XCONTEXT;


struct deflate_XCONTEXT {
   char ring[32768];
   unsigned long wp;
   unsigned long rxbitbuf;
   unsigned long lencode;
   unsigned long distcode;
   unsigned long rxbits;
   long rawlen;
   char compdata;
};

extern void deflate_Initexpand(struct deflate_XCONTEXT *);

extern void deflate_Initdeflate(struct deflate_CONTEXT *);

extern void deflate_Deflatbyte(struct deflate_CONTEXT *, char, char, char [],
                 unsigned long, long *);

extern void deflate_Expandbyte(struct deflate_XCONTEXT *, char, char [],
                unsigned long, long *, char *);


extern void deflate_BEGIN(void);


#endif /* deflate_H_ */
