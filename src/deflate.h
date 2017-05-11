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

typedef uint8_t deflate_SET8;

typedef uint32_t deflate_SET32;

struct deflate_CONTEXT;


struct deflate_CONTEXT {
   char ring[32768];
   uint32_t fill;
   uint16_t hashchain[32768];
   uint32_t fill1;
   uint16_t hash[4096];
   uint32_t fill2;
   char matchbuf[257];
   uint32_t combuf[2048];
   uint32_t txbitbuf;
   uint32_t txbitc;
   uint32_t lastidx;
   uint32_t wp;
   uint32_t savep;
   uint32_t flen;
   uint32_t fdist;
   uint32_t mlen;
   uint32_t rawr;
   uint32_t comr;
   uint32_t raww;
   uint32_t rawwo;
   uint32_t comw;
   int32_t of;
   int32_t sizedif;
   char wascomp;
};

struct deflate_XCONTEXT;


struct deflate_XCONTEXT {
   char ring[32768];
   uint32_t wp;
   uint32_t rxbitbuf;
   uint32_t lencode;
   uint32_t distcode;
   uint32_t rxbits;
   int32_t rawlen;
   char compdata;
};

extern void deflate_Initexpand(struct deflate_XCONTEXT *);

extern void deflate_Initdeflate(struct deflate_CONTEXT *);

extern void deflate_Deflatbyte(struct deflate_CONTEXT *, char, char,
                char [], uint32_t, int32_t *);

extern void deflate_Expandbyte(struct deflate_XCONTEXT *, char,
                char [], uint32_t, int32_t *, char *);


extern void deflate_BEGIN(void);


#endif /* deflate_H_ */
