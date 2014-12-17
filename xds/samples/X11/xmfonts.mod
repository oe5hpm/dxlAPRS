(* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY.
 * (c) Copyright 1996, 1997, 1998 XDS Ltd.
 * ALL RIGHTS RESERVED 
 *)

(* 
 * Motif Release 1.2.3
 *) 

<*+ M2EXTENSIONS *>
<*+ WOFF301 *>
<*+ WOFF300 *>

(* $RCSfile: xmfonts.mod,v $ $Revision: 7 $ $Date: 10/09/01 13:59 $ *)

MODULE xmfonts;

(**---------------------------------------------------------------------
***	
***	file:		xmfonts.c
***
***
Xmfonts.ad:

  xmfonts*XmScrolledWindow.height:	432
  xmfonts*XmScrolledWindow.width:	390
  xmfonts*row_column.XmNnumColumns:	5
  xmfonts*row_column.XmNpacking:        XmPACK_COLUMN

***-------------------------------------------------------------------*)

IMPORT sys:=SYSTEM,
       InOut,
       stdio, string,
       Xlib,
       Xt:=Intrinsic,
       Xm, MainW, PushB,
       RCol:=RowColumn,
       CasB:=CascadeB,
       ScrW:=ScrolledW,
       Frame,
       MsgB:=MessageB,
       PushBG,
       pe:=ProgEnv;

FROM Storage IMPORT ALLOCATE, DEALLOCATE;

CONST
  rcsid = "$Id: xmfonts.mod,v 2.3 1998/07/22 14:12:44 ego Exp $";

(*-------------------------------------------------------------
**	Global Variables
*)

CONST
  MAX_ARGS = 10;
  charset = Xm.XmSTRING_DEFAULT_CHARSET;

TYPE
  ApplicationData = RECORD
    fontpattern: Xt.String;
    maxfont: INTEGER;	(* number max of font to be displayed      *)
    maxlen: CARDINAL;	(* max length of font name to be displayed *)
    usegadget: Xt.Boolean;
  END;

  ApplicationDataPtr = POINTER TO ApplicationData;

VAR
  AppData: ApplicationDataPtr;

CONST
  XtNfontPattern = "fontPattern";
  XtCFontPattern = "FontPattern";
  XtNmaxFont     = "maxFont";
  XtCMaxFont     = "MaxFont";
  XtNmaxLen      = "maxLen";
  XtCMaxLen      = "MaxLen";
  XtNuseGadget   = "useGadget";
  XtCUseGadget   = "UseGadget";

(*-------------------------------------------------------------
 ** CloseCB - callback for close button
 *)
PROCEDURE ["C"] CloseCB (
                    w: Xt.Widget;		(* widget id              *)
                    client_data: Xt.XtPointer;	(* data from applicaiton  *)
                    call_data: Xt.XtPointer);	(* data from widget class *)
VAR
  font:		Xlib.PtrXFontStruct;
  message_box:	Xt.Widget;
  shell:	Xt.Widget;
BEGIN
  font := client_data;
  message_box := Xt.XtParent (w);
  shell := Xt.XtParent (message_box);

  (* Free font.	*)
  IF font <> NIL THEN
    Xlib.XFreeFont (Xt.XtDisplay (w), font);
  END;

  (* Unmanage and destroy widgets. *)
  Xt.XtUnmanageChild (message_box);
  Xt.XtDestroyWidget (shell);
END CloseCB;


(*-------------------------------------------------------------
 ** CreateHelp - create help window
 *)
PROCEDURE CreateHelp (
              parent: Xt.Widget	(* parent widget       *)
              ): Xt.Widget;	(* returns Help Widget *)

VAR
  button:		Xt.Widget;
  message_box:		Xt.Widget;                     (* Message Dialog *)
  args:			ARRAY [0..MAX_ARGS] OF Xt.Arg; (* arg list       *)
--  message:		ARRAY [0..1023] OF CHAR;
  title_string:		Xm.XmString;
  message_string:	Xm.XmString;
  button_string:	Xm.XmString;

CONST
  message = "These are buttons for the fonts in your X11 server whose \n"+
	    "name lengths are below the given maxLen application resource.\n"+
	    "The button label is the name of the font.  When you select \n"+
	    "a button, a small window will display a sample of the font. \n"+
	    "\nPress the 'close' button to close a font window. \n"+
	    "Select 'quit' from the 'exit' menu to exit this application.\0";
(* Generate message to display. *)
BEGIN
  message_string := Xm.XmStringCreateLtoR (message, charset);
  button_string := Xm.XmStringCreateLtoR ("Close", charset);
  title_string := Xm.XmStringCreateLtoR ("xmfonts help", charset);

  (* Create MessageBox dialog. *)
  message_box := MsgB.XmCreateMessageDialog (parent, "helpbox", NIL, 0);
  Xt.XtVaSetValues (message_box,
                    Xm.XmNdialogTitle, title_string,
                    Xm.XmNokLabelString, button_string,
                    Xm.XmNmessageString, message_string,
                    NIL);
  (* message_box := XtVaCreateWidget ("helpbox",
                                   MsgB.xmMessageDialogWidgetClass, parent,
                                   Xm.XmNdialogTitle, title_string,
                                   Xm.XmNokLabelString, button_string,
                                   Xm.XmNmessageString, message_string,
                                   NIL);
  *)

  button := MsgB.XmMessageBoxGetChild (message_box, Xm.XmDIALOG_CANCEL_BUTTON);
  Xt.XtUnmanageChild (button);
  button := MsgB.XmMessageBoxGetChild (message_box, Xm.XmDIALOG_HELP_BUTTON);
  Xt.XtUnmanageChild (button);

  (* Free strings and return MessageBox. *)
  IF title_string <> NIL THEN Xm.XmStringFree (title_string); END;
  IF message_string <> NIL THEN Xm.XmStringFree (message_string); END;
  IF button_string <> NIL THEN Xm.XmStringFree (button_string); END;
  RETURN message_box;
END CreateHelp;

(*-------------------------------------------------------------
 ** HelpCB - callback for help button
 *)
PROCEDURE ["C"] HelpCB (
                    w: Xt.Widget;		(* widget id              *)
                    client_data: Xt.XtPointer;	(* data from applicaiton  *)
                    call_data: Xt.XtPointer);	(* data from widget class *)
VAR
  message_box: Xt.Widget;	(* MessageBox *)
BEGIN
  (* Create help window. *)
  message_box := CreateHelp (w);
  (* Display help window. *)
  Xt.XtManageChild (message_box);
END HelpCB;


(*-------------------------------------------------------------
 ** QuitCB - callback for quit button
 *)
PROCEDURE ["C"] QuitCB (
                    w: Xt.Widget;		(* widget id              *)
                    client_data: Xt.XtPointer;	(* data from applicaiton  *)
                    call_data: Xt.XtPointer);	(* data from widget class *)
BEGIN
  (* Terminate the application. *)
  HALT (0);
END QuitCB;

(*-------------------------------------------------------------
 ** CreateFontSample - create font display window
 *)
PROCEDURE CreateFontSample (
              parent: Xt.Widget (* parent widget                        *)
              ): Xt.Widget;	(* returns message box with font sample *)
VAR
  message_box:    Xt.Widget;			(* MessageBox Dialog *)
  button:         Xt.Widget;
  args:           ARRAY [0..MAX_ARGS] OF Xt.Arg;(* arg list          *)
  name:           Xt.String;			(* font name         *)
  font:           Xlib.PtrXFontStruct;		(* font pointer      *)
  fontlist:       Xm.XmFontList;		(* fontlist pointer  *)
  message:        ARRAY [0..1023] OF CHAR;	(* text sample       *)
  name_string:    Xm.XmString;
  message_string: Xm.XmString;
  button_string:  Xm.XmString;
BEGIN
  name := NIL;
  font := NIL;
  fontlist := NIL;
  name_string := NIL;

  (* Get font name. *)
  Xt.XtVaGetValues (parent,
                    Xm.XmNlabelString, sys.ADR(name_string),
                    NIL);
  Xm.XmStringGetLtoR (name_string, charset, name);

  (* Load font and generate message to display. *)
  IF name <> NIL THEN
    font := Xlib.XLoadQueryFont (Xt.XtDisplay (Xt.XtParent (parent)), name^);
  END;
  IF  font = NIL THEN
    stdio.sprintf (message, "Unable to load font: %s\0", name^);
  ELSE
    fontlist := Xm.XmFontListCreate (font^, charset);
    stdio.sprintf
      (message,
       "This is font %s.\nThe quick brown fox jumps over the lazy dog.\0",
       name);
  END;
  message_string := Xm.XmStringCreateLtoR (message, charset);
  button_string := Xm.XmStringCreateLtoR ("Close", charset);

  (* Create MessageBox dialog. *)
  message_box := MsgB.XmCreateMessageDialog (Xt.XtParent (Xt.XtParent(parent)),
                                             "fontbox", NIL, 0);
  Xt.XtVaSetValues (message_box,
                    Xm.XmNdialogTitle, name_string,
                    Xm.XmNokLabelString, button_string,
                    Xm.XmNmessageString, message_string,
                    NIL);
  (* The second way to create message box *)
  (* message_box := XtVaCreateWidget ("fontbox",
                                   MsgB.xmMessageDialogWidgetClass,
                                   Xt.XtParent (Xt.XtParent(parent)),
                                   Xm.XmNdialogTitle, name_string,
                                   Xm.XmNokLabelString, button_string,
                                   Xm.XmNmessageString, message_string,
                                   NIL);
  *)

  button := MsgB.XmMessageBoxGetChild (message_box, Xm.XmDIALOG_MESSAGE_LABEL);
  IF fontlist <> NIL THEN
    Xt.XtVaSetValues(button,
                     Xm.XmNfontList, fontlist,
                     NIL);
  END;

  button := MsgB.XmMessageBoxGetChild (message_box, Xm.XmDIALOG_OK_BUTTON);
  Xt.XtAddCallback (button, Xm.XmNactivateCallback, CloseCB, NIL);
  button := MsgB.XmMessageBoxGetChild (message_box, Xm.XmDIALOG_CANCEL_BUTTON);
  Xt.XtUnmanageChild (button);
  button := MsgB.XmMessageBoxGetChild (message_box, Xm.XmDIALOG_HELP_BUTTON);
  Xt.XtUnmanageChild (button);

  (* Free strings and return MessageBox. *)
  IF fontlist <> NIL THEN Xm.XmFontListFree (fontlist); END;
  IF name_string <> NIL THEN Xm.XmStringFree (name_string); END;
  IF message_string <> NIL THEN Xm.XmStringFree (message_string); END;
  IF button_string <> NIL THEN Xm.XmStringFree (button_string); END;
  RETURN message_box;
END CreateFontSample;


(*-------------------------------------------------------------
 ** SelectFontCB - callback for font buttons
 *)
PROCEDURE ["C"] SelectFontCB (
                    w: Xt.Widget;		(* widget id              *)
                    client_data: Xt.XtPointer;	(* data from applicaiton  *)
                    call_data: Xt.XtPointer);	(* data from widget class *)
VAR
  message_box:    Xt.Widget;
BEGIN
  (* Create font sample window. *)
  message_box := CreateFontSample (w);
  (* Display font sample window. *)
  Xt.XtManageChild (message_box);
END SelectFontCB;


(*-------------------------------------------------------------
 ** CreateApplication - create main window
 *)
PROCEDURE CreateApplication (
              parent: Xt.Widget	(* parent widget       *)
              ): Xt.Widget;	(* returns Main window *)
VAR
  main_window:	Xt.Widget;			(* MainWindow       *)
  menu_bar:	Xt.Widget;			(* MenuBar          *)
  menu_pane:	Xt.Widget;			(* MenuPane         *)
  cascade:	Xt.Widget;			(* CascadeButton    *)
  frame:	Xt.Widget;			(* Frame            *)
  swindow:	Xt.Widget;			(* ScrolledWindow   *)
  row_column:	Xt.Widget;			(* RowColumn        *)
  button:	Xt.Widget;			(* PushButtonGadget *)
  hsb, vsb:	Xt.Widget;			(* ScrollBars       *)
  args:		ARRAY [0..MAX_ARGS] OF Xt.Arg;  (* arg list         *)
  n:		INTEGER;                        (* arg count        *)
  name:         Xt.String;
  count, i:	INTEGER;
  fontnamelist:	Xlib.PAPChar;
  label_string:	Xm.XmString;
  button_class:	Xt.WidgetClass;
BEGIN
  (* Create MainWindow. *)
  main_window := MainW.XmCreateMainWindow (parent, "main_window", NIL, 0);
  Xt.XtManageChild (main_window);

  (* Create MenuBar in MainWindow. *)
  menu_bar := RCol.XmCreateMenuBar (main_window, "menu_bar", NIL, 0);
  Xt.XtManageChild (menu_bar);

  (* Create "Exit" PulldownMenu. *)
  menu_pane := RCol.XmCreatePulldownMenu (menu_bar, "menu_pane", NIL, 0);

  button := PushB.XmCreatePushButton (menu_pane, "Quit", NIL, 0);
  Xt.XtManageChild (button);
  Xt.XtAddCallback (button, Xm.XmNactivateCallback, QuitCB, NIL);

  cascade := CasB.XmCreateCascadeButton (menu_bar, "Exit", NIL, 0);
  Xt.XtVaSetValues (cascade, Xm.XmNsubMenuId, menu_pane, NIL);

  (* Another way to create CascadeButton widget *)
  (* cascade := Xt.XtVaCreateWidget ("Exit",
                                  CasB.xmCascadeButtonWidgetClass, menu_bar,
                                  Xm.XmNsubMenuId, menu_pane,
                                  NIL);
  *)
  Xt.XtManageChild (cascade);

  (* Create "Help" button. *)
  cascade := CasB.XmCreateCascadeButton (menu_bar, "Help", NIL, 0);
  Xt.XtManageChild (cascade);
  Xt.XtAddCallback (cascade, Xm.XmNactivateCallback, HelpCB, NIL);

  Xt.XtVaSetValues (menu_bar,
                    Xm.XmNmenuHelpWidget, cascade,
                    NIL);

  (* Create Frame MainWindow and ScrolledWindow in Frame. *)
  frame := Frame.XmCreateFrame (main_window, "frame", NIL, 0);
  Xt.XtManageChild (frame);

  Xt.XtSetArg (args[0], Xm.XmNscrollingPolicy, Xm.XmAUTOMATIC);
  swindow := ScrW.XmCreateScrolledWindow (frame, "swindow", sys.ADR(args), 1);
  Xt.XtManageChild (swindow);

  (* Create RowColumn in ScrolledWindow to manage buttons. *)
  row_column := RCol.XmCreateRowColumn (swindow, "row_column", NIL, 0);
  Xt.XtManageChild (row_column);

  (* Set MainWindow areas and add tab groups *)
  MainW.XmMainWindowSetAreas (main_window, menu_bar, NIL, NIL, NIL,
                              frame);

  (* Create a PushButton widget for each font.
     Get the font list from the server and keep
     only the font name whose length is less than
     the given maxLen resource.
  *)

  fontnamelist := Xlib.XListFonts(Xt.XtDisplay(main_window),
                                  AppData^.fontpattern^,
                                  AppData^.maxfont, count);
  IF fontnamelist = NIL THEN
    stdio.fprintf(stdio.stderr^,"No font on your server, exit\n");
    HALT(0);
  END;

  (* read one entry each time through the loop *)
  FOR i := 0 TO count-1 DO
    name := sys.CAST(Xt.String, fontnamelist^[i]);
    IF LENGTH(name^) <= AppData^.maxlen THEN
      (* create PushButton in RowCol *)
      label_string := Xm.XmStringCreateLtoR(name^, charset);
      IF AppData^.usegadget THEN
        button_class:= PushBG.xmPushButtonGadgetClass;
      ELSE
        button_class := PushB.xmPushButtonWidgetClass;
      END;
      button:=Xt.XtVaCreateWidget(name^, button_class, row_column,
                                  Xm.XmNlabelString,label_string,
                                  NIL);
      Xt.XtManageChild (button);
      Xt.XtAddCallback (button, Xm.XmNarmCallback, SelectFontCB, NIL);
      Xm.XmStringFree (label_string);
    END;
  END;

  (* Return MainWindow. *)
  Xlib.XFreeFontNames(fontnamelist);
  RETURN main_window;
END CreateApplication;


(*-------------------------------------------------------------
 **	main		- main logic for application
 *)
TYPE
  paChar = POINTER TO ARRAY [0..MAX(INTEGER)-1] OF CHAR;

VAR
  argc:        INTEGER;
  argv:        Xt.StringList;
  display:     Xlib.PtrDisplay;	(*  Display		*)
  app_shell:   Xt.Widget; 	(*  ApplicationShell	*)
  main_window: Xt.Widget;	(*  MainWindow		*)
  app_context: Xt.XtAppContext;
  resources:   ARRAY [0..4] OF Xt.XtResource;
  i: INTEGER;

  pp: paChar;
CONST
  pattern = "*";

BEGIN
  WITH resources[0] DO
    resource_name:=sys.ADR(XtNfontPattern);
    resource_class:=sys.ADR(XtCFontPattern);
    resource_type:=sys.ADR(Xt.XtRString);
    resource_size:=SIZE(Xt.String);
    resource_offset:=0;
    default_type:=sys.ADR(Xt.XtRString);
    default_addr:=sys.ADR(pattern);
  END;
  WITH resources[1] DO
    resource_name:=sys.ADR(XtNmaxFont);
    resource_class:=sys.ADR(XtCMaxFont);
    resource_type:=sys.ADR(Xt.XtRInt);
    resource_size:=SIZE(INTEGER);
    resource_offset:=SIZE(Xt.String);
    default_type:=sys.ADR(Xt.XtRImmediate);
    default_addr:=sys.CAST(Xt.XtPointer, 1000);
  END;
  WITH resources[2] DO
    resource_name:=sys.ADR(XtNmaxLen);
    resource_class:=sys.ADR(XtCMaxLen);
    resource_type:=sys.ADR(Xt.XtRInt);
    resource_size:=SIZE(INTEGER);
    resource_offset:=SIZE(Xt.String)+SIZE(INTEGER);
    default_type:=sys.ADR(Xt.XtRImmediate);
    default_addr:=sys.CAST(Xt.XtPointer, 20);
  END;
  WITH resources[3] DO
    resource_name:=sys.ADR(XtNuseGadget);
    resource_class:=sys.ADR(XtCUseGadget);
    resource_type:=sys.ADR(Xt.XtRBoolean);
    resource_size:=SIZE(Xt.Boolean);
    resource_offset:=SIZE(Xt.String)+2*SIZE(INTEGER);
    default_type:=sys.ADR(Xt.XtRImmediate);
    default_addr:=sys.CAST(Xt.XtPointer, TRUE);
  END;

  (* Initialize toolkit and open the display. *)
  argc := pe.ArgNumber();
  ALLOCATE(argv, sys.CAST(INTEGER,SIZE(Xt.String))*(argc+1));
  ALLOCATE(argv^[0], SIZE(CHAR)*(pe.ProgramNameLength()+1));
  pp := sys.CAST(paChar, argv^[0]);
  pe.ProgramName(pp^);
  FOR i:=1 TO argc DO
    ALLOCATE(argv^[i], SIZE(CHAR)*(pe.ArgLength(i)+1));
    pp := sys.CAST(paChar, argv^[i]);
    pe.GetArg(i, pp^);
  END;
  INC(argc);
  app_shell := Xt.XtAppInitialize(app_context, "XmDemos", NIL, 0,
                                  argc, argv, NIL, NIL, 0);

  ALLOCATE(AppData, SIZE(ApplicationData));
  Xt.XtGetApplicationResources(app_shell,
                               sys.CAST(Xt.XtPointer,AppData),
                               sys.ADR(resources),
                               4,
                               NIL,
                               0);


  (* WITH AppData^ DO
    stdio.fprintf(stdio.stdout^,
                  "Fontpatter: %s\nMaxfont: %d\nMaxlen: %d\nUse: %d\n",
                  fontpattern,
                  maxfont,
                  maxlen,
                  usegadget);
  END;
  *)

  main_window := CreateApplication (app_shell);
  Xt.XtRealizeWidget (app_shell);

  (* Get and dispatch events. *)
  Xt.XtAppMainLoop (app_context);
END xmfonts.
