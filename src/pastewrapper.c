/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <sys/types.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <stdint.h>

void pasteget(Display* dis, Window win, Atom property, uint32_t* nn, unsigned char** pstr)
{
	int ii;
	long nnl;
	long nc;
	Atom atom1;

	XGetWindowProperty(dis, win, property, 0L, 64L, 0L, 0UL, &atom1, &ii, &nnl, &nc, pstr);

	*nn = (uint32_t)nnl;
}
