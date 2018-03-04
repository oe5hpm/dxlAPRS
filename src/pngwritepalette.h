/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef pngwritepalette_H_
#define pngwritepalette_H_
#include "X2C.h"
#include <sys/types.h>

struct PNGPALETTE;


struct PNGPALETTE {
   uint8_t r;
   uint8_t g;
   uint8_t b;
};

typedef struct PNGPALETTE * pPNGPALETTE;


struct pngwritepalette__D0 {
   struct PNGPALETTE * Adr;
   uint32_t Len0;
};

typedef uint8_t * pIMAGE;


struct pngwritepalette__D1 {
   uint8_t * Adr;
   uint32_t Len0;
};

typedef uint8_t * pRNS;


struct pngwritepalette__D2 {
   uint8_t * Adr;
   uint32_t Len0;
};

struct PNGPIXMAP;


struct PNGPIXMAP {
   pIMAGE image;
   uint32_t width;
   uint32_t height;
   pPNGPALETTE palette;
   uint32_t xbytes;
   uint32_t palettelen;
   uint32_t palettedepth;
   pRNS trns;
};

extern int32_t writepng(char [], struct PNGPIXMAP *);


#endif /* pngwritepalette_H_ */
