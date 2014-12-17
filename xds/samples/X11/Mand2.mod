(* Copyright (c) 1996-1998 XDS Ltd.
 *
 * Egor Ziryanov <ego@xds.ru> 27.09.96 20:04
 * Mandelbrodt Set Explorer for X11R6 second edition
 *)

<*+ M2EXTENSIONS*>

(* $RCSfile: Mand2.mod,v $ $Revision: 6 $ $Date: 10/09/01 13:59 $ *)

MODULE Mand2;

IMPORT SYSTEM, stdlib, stdio, string, X, Xatom, Xlib, Xutil;

FROM SYSTEM IMPORT ADR, BYTE;
FROM Storage IMPORT ALLOCATE, DEALLOCATE;

CONST
  rcsid="$Id: Mand2.mod,v 2.2 1998/07/22 14:03:43 ego Exp $";

  Banner='Mandelbrodt Set Explorer';

TYPE
  Complex = RECORD
    real, imag: LONGREAL;
  END;

  ImageData = RECORD
    image:         Xlib.PtrXImage;
    height, width: SYSTEM.int;
    origin:        Complex;
    range:         LONGREAL;
    depth:         INTEGER;
    distance:      LONGREAL;
  END;

VAR
  Disp:  Xlib.PtrDisplay;
  Wind:  X.Window;
  PMap:  X.Pixmap;
  Key:   X.KeySym;
  CMap:  X.Colormap; 
  GC:    Xlib.GC;
  Event: Xlib.XEvent;
  Attr:  Xlib.XWindowAttributes;
  Scr:   Xlib.PtrScreen;
  Stat:  Xutil.XComposeStatus;
  Size:  Xutil.XSizeHints;

  Screen, i, j, done, alloc : INTEGER;
  Fore, Back: SYSTEM.unsigned;
  ColNum: CARDINAL;
  text: ARRAY [0..10] OF CHAR;
  Data: ImageData;
  drawdistance: BOOLEAN;
  Colors: Xlib.PAXColor;

PROCEDURE CreateImage( VAR data: ImageData);
VAR
  x, y, i, n, perc: INTEGER;
  z, k: Complex;
  real, dist: LONGREAL;
  color: CARDINAL;
  name: ARRAY [0..128] OF CHAR;
  data8bpp: Xlib._8BPPXImageData;
  data16bpp: Xlib._16BPPXImageData;
  data32bpp: Xlib._32BPPXImageData;
  xdepth: INTEGER;
BEGIN
  data8bpp:=NIL;
  data16bpp:=NIL;
  data32bpp:=NIL;
  xdepth := Xlib.DefaultDepth(Disp, Screen);
  Xlib.XClearArea(Disp,Wind,0,0,data.width,data.height,TRUE);
  WITH data DO
    IF image = NIL THEN
      image:= Xlib.XCreateImage(Disp,Xlib.DefaultVisualOfScreen(
     	       	    	      	Xlib.DefaultScreenOfDisplay(Disp)),
     	       	    	      	xdepth,
     	       	    	      	X.ZPixmap, 0, NIL,
     	       	    	      	width, height, 8, width);
      CASE xdepth OF
        |8:
          ALLOCATE(data8bpp,width);
	  image^.data := data8bpp;
        |16:
          ALLOCATE(data16bpp,width*2);
	  image^.data := data16bpp;
        |24:
          ALLOCATE(data32bpp,width*4);
	  image^.data := data32bpp;
        |32:
          ALLOCATE(data32bpp,width*4);
	  image^.data := data32bpp;
        ELSE
          -- not implemented
    	  stdio.fprintf (stdio.stderr^, "Depth %d not implemented"+12C, xdepth);
      END (* case xdepth *);
    END (* if image = NIL *);
    perc := 0;
    FOR y:=0 TO height-1 DO
      FOR x:=0 TO width-1 DO
        z.real:= 0.;  z.imag := 0.; 
        k.real:= origin.real + LFLOAT(x) / LFLOAT(width) * range;
        k.imag:= origin.imag - LFLOAT(y) / LFLOAT(height) * range;

        (* Calculate z = z * z + k over and over. *)
      
        i:=0;
        LOOP
          real := z.real;
	  z.real := z.real * z.real - z.imag * z.imag + k.real;
     	  z.imag := 2 * real * z.imag + k.imag;
          dist := z.real * z.real + z.imag * z.imag;
          IF ( dist >= distance) THEN
     	    IF (drawdistance) THEN
	      color := INT(dist) MOD INT(ColNum);
     	    ELSE
     	    (* It could be drawn using iteration number as a color *)
     	      color := i MOD INT(ColNum);
     	    END;
     	    EXIT;
          END;
          INC(i);
          IF (i >= depth-1) THEN
     	    color:=0;
     	    EXIT;
     	  END
        END;
        CASE xdepth OF
          |8: data8bpp^[x]:=VAL(SYSTEM.CARD8,Colors^[color].pixel);
          |16: data16bpp^[x]:=VAL(SYSTEM.CARD16,Colors^[color].pixel);
          |24: data32bpp^[x]:=VAL(CARDINAL,Colors^[color].pixel);
          |32: data32bpp^[x]:=VAL(CARDINAL,Colors^[color].pixel);
          ELSE
            -- not implemented
            stdio.fprintf (stdio.stderr^,"Depth %d not implemented"+12C,xdepth);
	END (* case xdepth *);
        n := (y+1)*100 DIV height;
        IF (n = perc+1) & (n <> 100) THEN
          perc := n;
     	  stdio.sprintf(name,"%s::Counting [%d%%]",Banner,n);
     	  Xlib.XStoreName(Disp,Wind,name);
	  Xlib.XCopyArea(Disp,PMap,Wind,GC,0,0,width,y,0,0);
        END
      END;
      Xlib.XPutImage(Disp,PMap,GC,image,0,0,0,y,width,1);
    END;
    CASE xdepth OF
      |8: DEALLOCATE(data8bpp, width);
      |16: DEALLOCATE(data16bpp, width*2);
      |24: DEALLOCATE(data32bpp, width*4);
      |32: DEALLOCATE(data32bpp, width*4);
      ELSE
        -- not implemented
        stdio.fprintf (stdio.stderr^, "Depth %d not implemented"+12C, xdepth);
    END (* case xdepth *);
    image^.data := NIL;
    Xutil.XDestroyImage(image^);
    image:=NIL;
  END;
  Xlib.XStoreName(Disp,Wind,Banner);
END CreateImage;

PROCEDURE SetupColorMap(colors: Xlib.PAXColor; ncolors: CARDINAL;
                        Ramp: BOOLEAN; Alloc: BOOLEAN);
VAR
  def: X.Colormap;
  i: CARDINAL;
  tmp: SYSTEM.CARD16;
BEGIN
  IF (Ramp) THEN
    WITH colors^[0] DO
      pixel := 0;
      flags := X.DoRed + X.DoGreen + X.DoBlue;
      red   := 0;
      green := 0;
      blue  := 0;
    END;
    IF (Alloc) THEN
      Xlib.XAllocColor(Disp,CMap,colors^[0]);
    ELSE
      Xlib.XStoreColor(Disp,CMap,colors^[0]);
    END;
    tmp := 65535;
    FOR i:=1 TO ncolors-1 DO
      WITH colors^[i] DO
        pixel := i;
        flags := X.DoRed + X.DoGreen + X.DoBlue;
        red   := 65535;
        blue  := 0;
        green := tmp;
      END;
      tmp := tmp - tmp DIV 8;
      IF (Alloc) THEN
        Xlib.XAllocColor(Disp,CMap,colors^[i]);
      ELSE
        Xlib.XStoreColor(Disp,CMap,colors^[i]);
      END
    END;
  ELSE
    def := Xlib.DefaultColormap(Disp,Xlib.DefaultScreen(Disp));
    FOR i:=0 TO ncolors-1 DO
      colors^[i].pixel := i;
      colors^[i].flags := X.DoRed + X.DoGreen + X.DoBlue;
      Xlib.XQueryColor(Disp,def,colors^[i]);
      Xlib.XStoreColor(Disp,CMap,colors^[i]);
    END;
  END;
END SetupColorMap;

PROCEDURE ChangeImage(VAR data: ImageData; x, y: INTEGER; times: REAL);
BEGIN
  WITH data DO
    range       := range / times;
    origin.real := origin.real + LFLOAT(x) / 
     	       	   LFLOAT(width) * range * times - range / 2.;
    origin.imag := origin.imag - LFLOAT(y) /
     	       	   LFLOAT(height) * range * times + range / 2.;
  END
END ChangeImage;

BEGIN
  j:=0; done:=0;
  Disp:=Xlib.XOpenDisplay('');
  IF Disp = NIL THEN
     stdio.fprintf(stdio.stderr^,"Couldn`t open display."+12C);
     HALT;
  END;
  Screen:=Xlib.DefaultScreen(Disp);
  Scr:=   Xlib.DefaultScreenOfDisplay(Disp);

  Fore:=Xlib.WhitePixel(Disp,Screen);
  Back:=Xlib.BlackPixel(Disp,Screen);

  Size.height:=400;
  Size.width:=500;
  Size.x:=50;
  Size.y:=50;
  Size.flags:=Xutil.PPosition + Xutil.PSize;

  Wind:=Xlib.XCreateSimpleWindow( Disp, Xlib.DefaultRootWindow(Disp),
		Size.x, Size.y, Size.width, Size.height, 5, Fore,
		Back);

  Xutil.XSetStandardProperties(Disp,Wind,Banner,Banner,X.None,NIL,0,Size);

  WITH Data DO
    width:=Size.width;
    height:=Size.height;
    depth:=256;
    origin.real:=-1.4;
    origin.imag:=1.0;
    range:=2.0;
    distance:=4.;
    image:=NIL;
  END;

  ColNum := Xlib.XDisplayCells(Disp,Xlib.XDefaultScreen(Disp));
  ALLOCATE (Colors, SIZE (Xlib.XColor) * ColNum);
  GC     := Xlib.XCreateGC( Disp, Wind, 0, NIL);

  Xlib.XSetBackground( Disp, GC, Back);
  Xlib.XSetForeground( Disp, GC, Fore);

  PMap:=Xlib.XCreatePixmap(Disp,Xlib.DefaultRootWindow(Disp),
     	       	    	   Data.width, Data.height,
     	       	    	   Xlib.DefaultDepthOfScreen(
     	       	    	   Xlib.DefaultScreenOfDisplay(Disp)));


  IF (Scr^.root_visual^.class = X.TrueColor)   OR 
     (Scr^.root_visual^.class = X.StaticGray)  OR
     (Scr^.root_visual^.class = X.StaticColor) THEN
    alloc:=X.AllocNone;
  ELSE
    alloc:=X.AllocAll;
  END;
  CMap := Xlib.XCreateColormap(Disp, Wind,
     	       	    	       Xlib.DefaultVisual(Disp, Xlib.DefaultScreen(Disp)),
     	       	    	       alloc);
  Xlib.XSetWindowColormap(Disp,Wind,CMap);
  Xlib.XSetWMColormapWindows(Disp,Wind,ADR(Wind),1);

  Xlib.XSelectInput( Disp, Wind, X.ButtonPressMask 
     	       	    	       + X.KeyPressMask 
     	       	    	       + X.ExposureMask);
  Xlib.XMapRaised(Disp,Wind);

  IF (alloc=X.AllocNone) THEN
    SetupColorMap(Colors, ColNum, TRUE, TRUE);
  ELSE
    SetupColorMap(Colors, ColNum, TRUE, FALSE);
  END;

  drawdistance:=TRUE;
  
  CreateImage(Data);

  WHILE done = 0 DO
    Xlib.XNextEvent( Disp, Event);
    CASE Event.type OF
      | X.ButtonPress:
     	CASE Event.xbutton.button OF
	  | X.Button1: (* Magnify 4 times *)
     	    ChangeImage(Data,Event.xbutton.x,Event.xbutton.y,2.);
     	  | X.Button2: (* Refresh *)
     	    WITH Data DO
     	      origin.real:=-1.4;
     	      origin.imag:=1.0;
     	      range:=2.0;
     	      distance:=4.;
     	    END;
     	  | X.Button3: (* Magnify 8 times *)
     	    ChangeImage(Data,Event.xbutton.x,Event.xbutton.y,4.);
     	  ELSE
     	END;
     	CreateImage(Data);
      | X.Expose:
     	Xlib.XGetWindowAttributes(Disp,Wind,Attr);
     	IF (Attr.height <> Size.height) OR (Attr.width <> Size.width) THEN
     	  IF Event.xexpose.count = 0 THEN
     	    Data.height:=Attr.height; Size.height:=Attr.height;
     	    Data.width:=Attr.width; Size.width:=Attr.width;
     	    IF ( PMap # 0) THEN
	      Xlib.XFreePixmap(Disp,PMap);
     	      PMap:=Xlib.XCreatePixmap(Disp,Xlib.DefaultRootWindow(Disp),
     	       	    	      	       Data.width, Data.height,
				       Xlib.DefaultDepthOfScreen(
     	       	    	      	       Xlib.DefaultScreenOfDisplay(Disp)));
     	    END;
     	    CreateImage(Data);
     	  END;
     	ELSE
          Xlib.XCopyArea(Disp,PMap,Wind,GC,Event.xexpose.x,Event.xexpose.y,
                         Event.xexpose.width,Event.xexpose.height,
     	                 Event.xexpose.x,Event.xexpose.y);
        END;
      | X.KeyPress:
        i:=Xutil.XLookupString( Event.xkey,SYSTEM.ADR(text),10,Key,Stat);
        IF ( i=1 ) THEN
     	  CASE text[0] OF
     	    | 'q':
     	       INC(done);
     	    | 'd':
     	       IF ( NOT drawdistance ) THEN
     	         drawdistance:=TRUE;
     	         CreateImage(Data);
     	       END
     	    | 'i':
     	       IF ( drawdistance ) THEN
     	         drawdistance:=FALSE;
     	         CreateImage(Data);
     	       END
     	    ELSE
     	  END
     	END
      ELSE
    END
  END;
  DEALLOCATE (Colors, SIZE (Xlib.XColor) * ColNum);
  Xlib.XFreeGC(Disp,GC);
  Xlib.XDestroyWindow(Disp,Wind);
  Xlib.XCloseDisplay(Disp);

END Mand2.
