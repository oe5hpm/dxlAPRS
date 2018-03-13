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

struct PNGPALETTE;


struct PNGPALETTE {
   uint8_t r;
   uint8_t g;
   uint8_t b;
};

typedef uint8_t * pLINE;

typedef pLINE * pROWPOINTERS;

typedef struct PNGPALETTE * pPNGPALETTE;


struct pngwritepalette__D0 {
   struct PNGPALETTE * Adr;
   size_t Len0;
};

typedef uint8_t * pRNS;


struct pngwritepalette__D1 {
   uint8_t * Adr;
   size_t Len0;
};

extern int32_t writepng(char [], pROWPOINTERS, uint32_t, uint32_t,
                pPNGPALETTE, uint32_t, uint32_t, pRNS);


#endif /* pngwritepalette_H_ */
