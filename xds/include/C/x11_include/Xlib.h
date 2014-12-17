#ifndef __XDS_Xlib_H
#define __XDS_Xlib_H

/* Copyright (c) 1996 XDS Ltd, Russia. All rights reserved. 
 *
 * $RCSfile: Xlib.h,v $ $Revision: 1.5 $ $Date/01/29 11:33:32 $
 */
#include <X11/Xlib.h>
#include "X.h"

typedef unsigned char 		*PAByte;
typedef char 			*PChar;
typedef PChar			*PAPChar;

typedef Atom			*PAAtom;

typedef Display			*PtrDisplay;
typedef PtrDisplay		*PAPDisplay;

typedef XImage			*PtrXImage;

typedef Screen			*PtrScreen;
typedef Screen			*PAScreenk;

typedef ScreenFormat		*PtrScreenFormat;
typedef ScreenFormat		*PAScreenFormat;

typedef Visual			*PtrVisual;
typedef Visual			*PAVisual;

typedef XExtData		*PtrXExtData;
typedef XExtData		*PAXExtData;

typedef PtrXExtData		*PtrPXExtData;
typedef PtrXExtData		*PAPXExtData;

typedef Depth			*PtrDepth;
typedef Depth			*PADepth;

typedef XSelectionRequestEvent	*PtrXSelectionRequestEvent;

typedef XFontProp		*PtrXFontProp;
typedef XFontProp		*PAXFontProp;

typedef XCharStruct		*PtrXCharStruct;

typedef XChar2b			*PAXChar2b;

typedef XFontStruct		*PtrXFontStruct;

typedef wchar_t			*Ptrwchar_t;
typedef wchar_t			*PAwchar_t;

typedef XPointer		*PtrXPointer;
typedef XPointer		*PAXPointer;

#if defined (XlibSpecificationRelease) && XlibSpecificationRelease > 5
typedef XOrientation		*PtrXOrientation;
#endif

typedef PtrXFontStruct		*PAPXFontStruct;

#if defined (XlibSpecificationRelease) && XlibSpecificationRelease > 5
typedef XIMStringConversionText	*PtrXIMStringConversionText;
typedef XIMHotKeyTrigger	*PtrXIMHotKeyTrigger;
#endif

typedef XTimeCoord		*PtrXTimeCoord;

typedef XModifierKeymap		*PtrXModifierKeymap;

typedef XGCValues		*PtrXGCValues;
typedef XGCValues		*PtrAXGCValues;

typedef XSetWindowAttributes	*PtrXSetWindowAttributes;
typedef XSetWindowAttributes	*PAXSetWindowAttributes;

typedef XHostAddress		*PtrXHostAddress;
typedef XHostAddress		*PAXHostAddress;

typedef XKeyEvent		*PtrXKeyEvent;

typedef XExtCodes		*PtrXExtCodes;

typedef XErrorEvent		*PtrXErrorEvent;

typedef XPixmapFormatValues	*PtrXPixmapFormatValues;
typedef XPixmapFormatValues	*PAXPixmapFormatValues;

typedef int			*PASInt;

typedef XWindowChanges		*PtrXWindowChanges;

typedef XKeyboardControl	*PtrXKeyboardControl;
typedef XKeyboardControl	*PAXKeyboardControl;

typedef XEvent			*PtrXEvent;

typedef XArc			*PtrXArc;
typedef XArc			*PAXArc;

typedef XPoint			*PtrXPoint;
typedef XPoint			*PAXPoint;

typedef XRectangle		*PtrXRectangle;
typedef XRectangle		*PAXRectangle;

typedef XSegment		*PtrXSegment;
typedef XSegment		*PAXSegment;

typedef XTextItem		*PtrXTextItem;
typedef XTextItem		*PAXTextItem;

typedef XTextItem16		*PtrXTextItem16;
typedef XTextItem16		*PAXTextItem16;

typedef XKeyboardState		*PtrXKeyboardState;
typedef XKeyboardState		*PAXKeyboardState;

typedef unsigned char		*PAUChar;

typedef XWindowAttributes	*PtrXWindowAttributes;

typedef XColor			*PtrXColor;
typedef XColor			*PAXColor;

typedef unsigned char		*PAXKeyCode;

typedef XMappingEvent		*PtrXMappingEvent;

typedef XFontSetExtents		*PtrXFontSetExtents;
typedef XFontSetExtents		*PAXFontSetExtents;

typedef XmbTextItem 		*PtrXmbTextItem;
typedef XmbTextItem		*PAXmbTextItem;

typedef XwcTextItem		*PtrXwcTextItem;
typedef XwcTextItem	 	*PAXwcTextItem;

typedef unsigned char		*_8BPPXImageData;
typedef unsigned short		*_16BPPXImageData;
typedef unsigned long		*_32BPPXImageData;

typedef _8BPPXImageData		_1BPPXImageData;
typedef _8BPPXImageData		_4BPPXImageData;
typedef _32BPPXImageData	_24BPPXImageData;

#endif
