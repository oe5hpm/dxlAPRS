/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include "Lib.h"
#include "osic.h"

unsigned long Lib_argc = 0;
static int lib_argc = 0;
static char **lib_argv;

extern void Lib_Fill(X2C_ADDRESS buf, unsigned long len, char byte)
{
	memset(buf, byte, len);
}

extern void Lib_NextArg(char s[], unsigned long s_len)
{
	if (Lib_argc >= lib_argc-1) {
		s[0] = 0;
		return;
	} else {
		strncpy(s, lib_argv[Lib_argc+1], s_len);
	}
	Lib_argc++;
}

extern double Lib_Random(void)
{
	return rand();
}

extern void Lib_BEGIN(int argc, char *argv[])
{
	static int Lib_init = 0;

	if (Lib_init)
		return;
	Lib_init = 1;

	lib_argc = argc;
	lib_argv = argv;
}

