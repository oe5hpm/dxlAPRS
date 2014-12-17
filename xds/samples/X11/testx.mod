(* Copyright (c) 1996-1998 XDS Ltd.
 *
 * Ego 7.04.96 22:20
 * Program for testing Modula-2/Oberon-2 programming interface to X11R6
 *)

<*+ M2EXTENSIONS*>

(* $RCSfile: testx.mod,v $ $Revision: 6 $ $Date: 10/09/01 13:58 $ *)

MODULE testx;

IMPORT SYSTEM, stdlib, stdio, string, X, Xatom, Xlib, Xutil;

FROM SYSTEM IMPORT ADR;

CONST
	rcsid="$Id: testx.mod,v 2.1 1998/02/15 00:08:14 ego Exp $";

CONST
  crect='COLOR RECTANGLE';
  width=15;
  height=25;

VAR
  mydisplay: Xlib.PtrDisplay;
  mywindow: X.Window;
  mygc: Xlib.GC;
  myevent: Xlib.XEvent;
  mystat: Xutil.XComposeStatus;
  mykey: X.KeySym;
  myhint: Xutil.XSizeHints;
  color: Xlib.XColor;
  colors: ARRAY [0..15] OF Xlib.XColor;
  cmap: X.Colormap;
  myscreen, i, j, done : SYSTEM.int;
  myforeground, mybackground: SYSTEM.unsigned;
  x, y : SYSTEM.int;
  pix: SYSTEM.unsigned;
  result: SYSTEM.int;
  text: ARRAY [0..10] OF CHAR;
  disp: ARRAY [0..100] OF CHAR;
  wattr: Xlib.XWindowAttributes;


BEGIN
  j:=0; done:=0;
(*  disp[0]:=0C;*)
  mydisplay:=Xlib.XOpenDisplay('');
  IF mydisplay = NIL THEN
     stdio.fprintf(stdio.stderr^,"Couldn`t open display %s . "+12C, disp);
     HALT;
  END;
  myscreen:=Xlib.DefaultScreen(mydisplay);

  myforeground:=Xlib.BlackPixel(mydisplay,myscreen);
  mybackground:=Xlib.WhitePixel(mydisplay,myscreen);

  myhint.height:=250;
  myhint.width:=350;
  myhint.x:=200;
  myhint.y:=300;
  myhint.flags:=Xutil.PPosition + Xutil.PSize;

  mywindow:=Xlib.XCreateSimpleWindow( mydisplay, Xlib.DefaultRootWindow(mydisplay),
		myhint.x, myhint.y, myhint.width, myhint.height, 5, myforeground,
		mybackground);

  Xutil.XSetStandardProperties(mydisplay,mywindow,crect,crect,
			X.None,NIL,0,myhint);

  mygc:=Xlib.XCreateGC( mydisplay, mywindow, 0, NIL);

  Xlib.XSetBackground( mydisplay, mygc, mybackground);
  Xlib.XSetForeground( mydisplay, mygc, myforeground);

  Xlib.XSelectInput( mydisplay, mywindow, 
			X.ButtonPressMask + X.KeyPressMask + X.ExposureMask);
  cmap:=Xlib.DefaultColormap( mydisplay,myscreen);
  color.red := 0FFFFH;
  color.green := 32768;
  color.blue := 0;

  result := Xlib.XAllocColor( mydisplay, cmap, color);
  stdio.printf( "res=%d R=%d G=%d B=%d pix=%d"+12C,result, color.red,color.green,
	                        color.blue, color.pixel);
  pix:=color.pixel;
  FOR i:=0 TO 15 DO
    colors[i].pixel:=i*1000;
    colors[i].red:=i*4000;
    colors[i].blue:=i*2000;
    colors[i].green:=i*1000;
    -- colors[i].flags:=X.DoRed + X.DoGreen + X.DoBlue;
    result := Xlib.XAllocColor(mydisplay, cmap, colors[i]);
    stdio.printf("res = %d, pix=%d R=%d G=%d B=%u"+12C, result, colors[i].pixel,
                 colors[i].red, colors[i].green, colors[i].blue);
  END;
  stdio.printf( "Default colormap after allocation: %d"+12C,cmap);

  stdio.printf ("!!! DEPTH !!! - %d"+12C, Xlib.DefaultDepth(mydisplay, myscreen));


  Xlib.XMapRaised( mydisplay, mywindow);
  WHILE done = 0 DO
    Xlib.XNextEvent( mydisplay, myevent);
    CASE myevent.type OF
      | X.ButtonPress:
        IF j = 0 THEN
	  x:=myevent.xbutton.x;
	  y:=myevent.xbutton.y;
	  Xlib.XSetForeground(mydisplay,mygc,pix);
          Xlib.XFillRectangle( myevent.xbutton.display, myevent.xbutton.window,mygc,
			0,0,wattr.width,wattr.height);
	  FOR i:=0 TO 15 DO
	    Xlib.XSetForeground(mydisplay,mygc,colors[i].pixel);
	    Xlib.XFillRectangle( myevent.xbutton.display, myevent.xbutton.window,mygc,
			   x+(i*width DIV 2),y+(i*height DIV 2),width,height);
	  END;
	  INC(j);
	END;
      | X.Expose:
        IF myevent.xexpose.count = 0 THEN
	  Xlib.XGetWindowAttributes(mydisplay,mywindow,wattr);
	  Xlib.XSetForeground(mydisplay,mygc,pix);
          Xlib.XFillRectangle( myevent.xbutton.display, myevent.xbutton.window,mygc,
			0,0,wattr.width,wattr.height);
	  FOR i:=0 TO 15 DO
	    Xlib.XSetForeground(mydisplay,mygc,colors[i].pixel);
	    Xlib.XFillRectangle( myevent.xbutton.display, myevent.xbutton.window,mygc,
			   x+(i*width DIV 2),y+(i*height DIV 2),width,height);
	  END
	END;
      | X.KeyPress:
        stdio.printf("testx: ***HERE***"+12C);
        i:=Xutil.XLookupString( myevent.xkey,SYSTEM.ADR(text),10,mykey,mystat);
        IF ( i=1 ) AND ( text[0] = 'q') THEN INC(done); END
      ELSE
    END
  END;
  Xlib.XFreeGC(mydisplay,mygc);
  Xlib.XDestroyWindow(mydisplay,mywindow);
  Xlib.XCloseDisplay(mydisplay);
  stdio.printf("Say: 'Good bye lover...'"+12C);

END testx.
