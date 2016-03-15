/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Hannes Schmelzer <oe5hpm@oevsv.at>
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>

#ifndef __XOSIC_H__
#define __XOSIC_H__

typedef XSizeHints		*PtrXSizeHints;
typedef XSizeHints		*PAXSizeHints;
typedef Visual			*PtrVisual;
typedef XImage			*PtrXImage;
typedef Screen			*PtrScreen;
typedef Display			*PtrDisplay;
typedef PtrDisplay		*PAPDisplay;
typedef XKeyboardControl	*PtrXKeyboardControl;
typedef XKeyboardControl	*PAXKeyboardControl;
typedef XEvent			*PtrXEvent;
typedef unsigned char		*PAUChar;
typedef char 			*PChar;

#endif /* __XOSIC_H__ */
