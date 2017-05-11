/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef Execlogin_H_
#define Execlogin_H_
#ifndef X2C_H_
#include "X2C.h"
#endif
#include <unistd.h>

struct Execlogin_tEXEC;


struct Execlogin_tEXEC {
   char * cmdfn;
   X2C_pCHAR * args;
};

extern int32_t Execlogin_StartLogin(struct Execlogin_tEXEC *);


extern void Execlogin_BEGIN(void);


#endif /* Execlogin_H_ */
