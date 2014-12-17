#ifndef __XDS_Xresource_H
#define __XDS_Xresource_H

/* Copyright (c) 1996 XDS Ltd, Russia. All rights reserved. 
 *
 * $RCSfile: Xresource.h,v $ $Revision: 1.4 $ $Date/01/29 11:33:32 $
 */
#include <X11/Xlib.h>
#include <X11/Xresource.h>

typedef XrmQuarkList		*PtrXrmQuarkList;
typedef XrmValue		*PAXrmValue;

typedef Bool			(*PtrProc) (
#if NeedNestedPrototypes
             XrmDatabase*       /* db */,
             XrmBindingList     /* bindings */,
             XrmQuarkList       /* quarks */,
             XrmRepresentation* /* type */,
             XrmValue*          /* value */,
             XPointer           /* closure */
#endif
             );

#endif
