/* Copyright (c) Excelsior 1999-2002. All Rights Reserved. */

#include <stdlib.h>
#include "xrtsOS.h"
#include "xrTCov.h"

static X2C_EXIT_PROC atexit_buf[8];
static short atexit_cnt=0;

#ifndef X2C_KRC
void X2C_iniexit (void) {
#else
extern void X2C_iniexit() {
#endif
}

#ifndef X2C_KRC
void X2C_atexit (X2C_EXIT_PROC func) {
#else
extern void X2C_atexit(func) X2C_EXIT_PROC func; {
#endif
#ifdef X2C_no_atexit
	if (atexit_cnt>=8) X2C_TRAP(X2C_internalError);
	atexit_buf[atexit_cnt++]=func;
#else
	if (atexit(func)) X2C_TRAP(X2C_internalError);
#endif
}

#ifndef X2C_KRC
extern void X2C_doexit(long code)
#else
extern void X2C_doexit(code) long code;
#endif
{
#ifdef X2C_no_atexit
	X2C_EXIT_PROC p;
	while (atexit_cnt) {
		p=atexit_buf[--atexit_cnt];
		p();
	}
#endif
	exit(code);
}
