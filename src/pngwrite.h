/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef pngwrite_H_
#define pngwrite_H_
#include "X2C.h"

struct PNGPIXEL;


struct PNGPIXEL {
   unsigned char red;
   unsigned char green;
   unsigned char blue;
};

typedef struct PNGPIXEL * pIMAGE;


struct pngwrite__D0 {
   struct PNGPIXEL * Adr;
   size_t Len0;
};

struct PNGPIXMAP;


struct PNGPIXMAP {
   pIMAGE image;
   unsigned long width;
   unsigned long height;
};

extern long writepng(char [], struct PNGPIXMAP *);


#endif /* pngwrite_H_ */
