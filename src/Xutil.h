#ifndef __XDS_Xutil_H
#define __XDS_Xutil_H

/* Copyright (c) 1996 XDS Ltd, Russia. All rights reserved. 
 *
 * $RCSfile: Xutil.h,v $ $Revision: 1.4 $ $Date/01/29 11:33:32 $
 */
#include <X11/Xutil.h>

#include "X.h"
#include "Xlib.h"

typedef XClassHint	*PtrXClassHint;
typedef XWMHints	*PtrXWMHints;

typedef XIconSize	*PtrXIconSize;
typedef XIconSize	*PAXIconSize;

typedef XStandardColormap	*PtrXStandardColormap;
typedef XStandardColormap	*PAXStandardColormap;

typedef XTextProperty	*PtrXTextProperty;

typedef XVisualInfo	*PtrXVisualInfo;
typedef XVisualInfo    *PAXVisualInfo;

typedef XSizeHints 	*PtrXSizeHints;
typedef XSizeHints 	*PAXSizeHints;

typedef PAwchar_t *PAPAwchar_t;

#endif
