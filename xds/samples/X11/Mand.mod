(* Copyright (c) 1996-1998 XDS Ltd.
 *
 * Egor Ziryanov <ego@xds.ru> 27.09.96 20:04
 * Mandelbrodt Set Explorer for X11R6 first edition
 *)

<*+ M2EXTENSIONS*>

(* $RCSfile: Mand.mod,v $ $Revision: 4 $ $Date: 22.07.98 21:15 $ *)

MODULE Mand;

IMPORT SYSTEM, stdlib, stdio, string, X, Xatom, Xlib, Xutil;

FROM SYSTEM IMPORT ADR;
FROM Storage IMPORT ALLOCATE, DEALLOCATE;

CONST
  rcsid="$Id: Mand.mod,v 2.1 1998/02/15 00:08:13 ego Exp $";

  Banner='Mandelbrodt Set  Explorer';

TYPE
  Complex = RECORD
    real, imag: LONGREAL;
  END;

  ImageData = RECORD
    pixmap:        X.Pixmap;
    height, width: SYSTEM.int;
    origin:        Complex;
    range:         LONGREAL;
    depth:         INTEGER;
    distance:      LONGREAL;
  END;

VAR
  Disp: Xlib.PtrDisplay;
  Wind: X.Window;
  GC: Xlib.GC;
  Event: Xlib.XEvent;
  Attr:  Xlib.XWindowAttributes;
  Stat: Xutil.XComposeStatus;
  Size: Xutil.XSizeHints;
  Key: X.KeySym;
  Screen, i, j, done : SYSTEM.int;
  Fore, Back: SYSTEM.unsigned;
  CMap: X.Colormap; 
  ColNum: INTEGER;
  text: ARRAY [0..10] OF CHAR;
  Data: ImageData;

PROCEDURE CreateImage( VAR data: ImageData);
VAR
  x, y, i, n, perc: INTEGER;
  z, k: Complex;
  real, distance: LONGREAL;
  color: CARDINAL;
BEGIN
  Xlib.XClearArea(Disp,Wind,0,0,data.width,data.height,TRUE);
  IF (data.pixmap = 0) THEN
    data.pixmap:=Xlib.XCreatePixmap(Disp,Xlib.DefaultRootWindow(Disp),
     	       	    	      	    data.width, data.height,
     	       	    	      	    Xlib.DefaultDepthOfScreen(
     	       	    	      	    Xlib.DefaultScreenOfDisplay(Disp)));
  END;
  perc := 0;
  FOR y:=0 TO data.height-1 DO
    FOR x:=0 TO data.width-1 DO
      z.real := 0.;  z.imag := 0.; 
      k.real := data.origin.real + LFLOAT(x) / LFLOAT(data.width) * data.range;
      k.imag := data.origin.imag - LFLOAT(y) / LFLOAT(data.height) * data.range;

      (* Calculate z = z * z + k over and over. *)
      
      i:=0;
      LOOP
        real := z.real;
	z.real := z.real * z.real - z.imag * z.imag + k.real;
     	z.imag := 2 * real * z.imag + k.imag;
        distance := z.real * z.real + z.imag * z.imag;
        IF ( distance >= data.distance) THEN
	  color := INT(distance) MOD ColNum;
     	  (* It could be drawn using iteration number as a color *)
     	  (* color := i MOD ColNum; *)
     	  EXIT;
        END;
        INC(i);
        IF (i >= data.depth-1) THEN
     	  color:=0;
     	  EXIT;
     	END;
      END;
      Xlib.XSetForeground(Disp,GC,color);
      Xlib.XDrawPoint(Disp,data.pixmap,GC,x,y);

      (* Draw throght 10% *)

      n := (y+1)*100 DIV data.height;
      IF (n = perc+10) & (n <> 100) THEN
        perc := n;
        Xlib.XCopyArea(Disp,Data.pixmap,Wind,GC,0,0,
                       data.width,y,
     	               0,0);
      END;
    END;
  END;
END CreateImage;

PROCEDURE SetupColorMap(ncolors: CARDINAL; Ramp: BOOLEAN);
VAR
  Colors: Xlib.PAXColor;
  def: X.Colormap;
  i: CARDINAL;
  tmp: SYSTEM.CARD16;
BEGIN
  ALLOCATE(Colors, SIZE(Xlib.XColor)*ncolors);
  IF (Ramp) THEN
    WITH Colors^[0] DO
      pixel := 0;
      flags := X.DoRed + X.DoGreen + X.DoBlue;
      red   := 0;
      green := 0;
      blue  := 0;
    END;
    tmp := 65535;
    FOR i:=1 TO ncolors-1 DO
      WITH Colors^[i] DO
        pixel := i;
        flags := X.DoRed + X.DoGreen + X.DoBlue;
        red   := 65535;
        blue  := 0;
        green := tmp;
      END;
      tmp := tmp - tmp DIV 8
    END;
    Xlib.XStoreColors(Disp,CMap,Colors,ncolors);
  ELSE
    def := Xlib.DefaultColormap(Disp,Xlib.DefaultScreen(Disp));
    FOR i:=0 TO ncolors-1 DO
      Colors^[i].pixel := i;
      Colors^[i].flags := X.DoRed + X.DoGreen + X.DoBlue;
    END;
    Xlib.XQueryColors(Disp,def,Colors,ncolors);
    Xlib.XStoreColors(Disp,CMap,Colors,ncolors);
  END;
  DEALLOCATE(Colors, SIZE(Xlib.XColor)*ncolors);
END SetupColorMap;

PROCEDURE ChangeImage(VAR data: ImageData; x, y: INTEGER; times: REAL);
BEGIN
  data.range       := data.range / times;
  data.origin.real := data.origin.real + LFLOAT(x) / 
     	       	      LFLOAT(data.width) * data.range * times - data.range / 2.;
  data.origin.imag := data.origin.imag - LFLOAT(y) /
     	       	      LFLOAT(data.height) * data.range * times + data.range / 2.;
END ChangeImage;

BEGIN
  j:=0; done:=0;
  Disp:=Xlib.XOpenDisplay('');
  IF Disp = NIL THEN
     stdio.fprintf(stdio.stderr^,"Couldn`t open display.\n ");
     HALT;
  END;
  Screen:=Xlib.DefaultScreen(Disp);

  Fore:=Xlib.WhitePixel(Disp,Screen);
  Back:=Xlib.BlackPixel(Disp,Screen);

  Size.height:=400;
  Size.width:=600;
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
    pixmap:=0;
  END;

  ColNum := Xlib.XDisplayCells(Disp,Xlib.XDefaultScreen(Disp));
  GC     := Xlib.XCreateGC( Disp, Wind, 0, NIL);

  Xlib.XSetBackground( Disp, GC, Back);
  Xlib.XSetForeground( Disp, GC, Fore);


  CMap := Xlib.XCreateColormap(Disp, Wind,
     	       	    	       Xlib.DefaultVisual(Disp, Xlib.DefaultScreen(Disp)),
     	       	    	       X.AllocAll);
  Xlib.XSetWindowColormap(Disp,Wind,CMap);
  Xlib.XSetWMColormapWindows(Disp,Wind,ADR(Wind),1);

  Xlib.XSelectInput( Disp, Wind, X.ButtonPressMask 
     	       	    	       + X.KeyPressMask 
     	       	    	       + X.ExposureMask);
  Xlib.XMapRaised(Disp,Wind);

  SetupColorMap(ColNum,TRUE);
  
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
     	    IF ( Data.pixmap # 0) THEN
	      Xlib.XFreePixmap(Disp,Data.pixmap);
     	      Data.pixmap:=0;
     	    END;
     	    CreateImage(Data);
     	  END;
     	ELSE
          Xlib.XCopyArea(Disp,Data.pixmap,Wind,GC,Event.xexpose.x,Event.xexpose.y,
                         Event.xexpose.width,Event.xexpose.height,
     	                 Event.xexpose.x,Event.xexpose.y);
        END;
      | X.KeyPress:
        i:=Xutil.XLookupString( Event.xkey,SYSTEM.ADR(text),10,Key,Stat);
        IF ( i=1 ) AND ( text[0] = 'q') THEN INC(done); END
      ELSE
    END
  END;
  Xlib.XFreeGC(Disp,GC);
  Xlib.XDestroyWindow(Disp,Wind);
  Xlib.XCloseDisplay(Disp);

END Mand.
