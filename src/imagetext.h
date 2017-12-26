/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef imagetext_H_
#define imagetext_H_
#ifndef X2C_H_
#include "X2C.h"
#endif

struct imagetext_PIX;


struct imagetext_PIX {
   uint16_t r;
   uint16_t g;
   uint16_t b;
};


struct imagetext_IMAGELINE {
   struct imagetext_PIX * Adr;
   size_t Len0;
};


struct imagetext_IMAGE {
   struct imagetext_PIX * Adr;
   size_t Len0;
   size_t Size1;
   size_t Len1;
};

typedef struct imagetext_IMAGE * imagetext_pIMAGE;

extern void imagetext_writestr(imagetext_pIMAGE, uint32_t, uint32_t,
                uint32_t, uint32_t, uint32_t, uint32_t, char [],
                uint32_t);

extern uint32_t imagetext_strsize(uint32_t, char [], uint32_t);

extern uint32_t imagetext_fontsizex(uint32_t);

extern uint32_t imagetext_fontsizey(uint32_t);


extern void imagetext_BEGIN(void);


#endif /* imagetext_H_ */
