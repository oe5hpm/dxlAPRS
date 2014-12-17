(*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1997, 1998 XDS Ltd.
 * ALL RIGHTS RESERVED 
 *)

(*
 * Motif Release 1.2
 *)

(**---------------------------------------------------------------------
***	
***	file:		xmform.mod
***
***	project:	Motif Widgets example programs
***
***	creation:	Spring 91 - Cambridge
***                     Daniel Dardailler.
***
***     C->Modula2:     November 96 - Novosiborsk
***                     Egor Ziryanov.
***
***	description:	This program demonstrates the possibilities of
***                     the Motif form widget.
***                     It can display a string of letters made from buttons 
***	                and arrows (resizable using Form attachments).
***
***	defaults:	xmform looks nicer with these defaults:
***
xmform*highlightThickness:       0
xmform.XmForm.shadowThickness:   3
xmform*XmPushButton.background:  cadetblue
xmform*XmArrowButton.foreground: cadetblue
xmform*XmArrowButton.background: lightgrey
xmform*XmForm.background:        lightgrey
xmform*topShadowColor:           white
xmform*bottomShadowColor:        black
***-------------------------------------------------------------------*)

<*+ M2EXTENSIONS*>

(* $RCSfile: xmform.mod,v $ $Revision: 7 $ $Date: 10/09/01 13:59 $ *)

MODULE xmform;

IMPORT  SYSTEM,
     	InOut,
      	ProgEnv,
      	Xt:=Intrinsic,
      	Xm, F:=Form,
      	PB:=PushB,
      	AB:=ArrowB;

FROM SYSTEM IMPORT ADR;
FROM Storage IMPORT ALLOCATE, DEALLOCATE;

CONST
  rcsid = "$Id: xmform.mod,v 2.2 1998/07/22 14:12:44 ego Exp $";

PROCEDURE ["C"] AddStem(form: Xt.Widget;
			top, right, bottom, left: Xt.Dimension;
		      	wtype: INTEGER): Xt.Widget;
VAR
  class: Xt.WidgetClass;
BEGIN
    (* create an object, either a button or an arrow, using
       positionnal attachment everywhere, instead of possibly 
       form attachment or widget attachment *)

    IF (wtype = Xm.XmPUSHBUTTON) THEN
	class:=PB.xmPushButtonWidgetClass;
    ELSE 
	class:=AB.xmArrowButtonWidgetClass;
    END;
    RETURN Xt.XtVaCreateWidget("", class, form,
			       Xm.XmNtopAttachment, Xm.XmATTACH_POSITION,
			       Xm.XmNtopPosition, top,
			       Xm.XmNrightAttachment, Xm.XmATTACH_POSITION,
			       Xm.XmNrightPosition, right,
			       Xm.XmNbottomAttachment, Xm.XmATTACH_POSITION,
			       Xm.XmNbottomPosition, bottom,
			       Xm.XmNleftAttachment, Xm.XmATTACH_POSITION,
			       Xm.XmNleftPosition, left,
			       NIL);
END AddStem;

PROCEDURE ["C"] NearlyEvenSpread(children: Xt.WidgetList;
			      	 num_child: Xt.Cardinal;
			      	 hor_spacing, ver_spacing: Xt.Dimension);
VAR
  hor_child_room: Xt.Dimension;
  i: Xt.Cardinal;
BEGIN
    (* this algorithm works only for the default fractionBase == 100 *)
    (* hor_spacing and ver_margin are equally applied on each sides *)

    hor_child_room := 100 DIV num_child ;

    FOR i := 0 TO num_child DO
	Xt.XtVaSetValues(children^[i],
			 Xm.XmNleftAttachment, Xm.XmATTACH_POSITION,
			 Xm.XmNleftPosition, hor_child_room*i + hor_spacing,
			 Xm.XmNrightAttachment, Xm.XmATTACH_POSITION,
			 Xm.XmNrightPosition, hor_child_room*(i+1) - hor_spacing,
			 Xm.XmNtopAttachment, Xm.XmATTACH_POSITION,
			 Xm.XmNtopPosition, ver_spacing,
			 Xm.XmNbottomAttachment, Xm.XmATTACH_POSITION,
			 Xm.XmNbottomPosition, 100 - ver_spacing,
			 NIL);
    END;
END NearlyEvenSpread;

PROCEDURE ["C"] FillItPlease(form: Xt.Widget; letter: CHAR);
VAR
  children: Xt.WidgetList;
BEGIN
    (* This routine only handle the "MOTIF" letters, it's up 
       to you to implement the rest, good luck for K,N,R,V,X,Y,Z... *)

    ALLOCATE(children, SIZE(Xt.Widget)*3);

    CASE(letter) OF
    |'M' :
	(* 2 buttons and the arrow in between on top *)
	children^[0] := AddStem(form, 0, 25, 100, 0, Xm.XmPUSHBUTTON);
	children^[1] := AddStem(form, 0, 100, 100, 75, Xm.XmPUSHBUTTON);
	children^[2] := AddStem(form, 0, 75, 35, 25, 0);
	Xt.XtVaSetValues(children^[2],
			 Xm.XmNarrowDirection, Xm.XmARROW_DOWN,
			 Xm.XmNshadowThickness, 0,
			 NIL);
	Xt.XtManageChildren(children, 3);
    |'O' :
	(* 2 left and right vertical buttons and 2 horizontals *)
	children^[0] := AddStem(form, 25, 25, 75, 0, Xm.XmPUSHBUTTON);
	children^[1] := AddStem(form, 0, 100, 25, 0, Xm.XmPUSHBUTTON);
	children^[2] := AddStem(form, 25, 100, 75, 75, Xm.XmPUSHBUTTON);
	children^[3] := AddStem(form, 75, 100, 100, 0, Xm.XmPUSHBUTTON);
	Xt.XtManageChildren(children, 4);
    |'T' :
	(* 1 vertical button and 1 horizontal on top *)
	children^[0] := AddStem(form, 25, 63, 100, 38, Xm.XmPUSHBUTTON);
	children^[1] := AddStem(form, 0, 100, 25, 0, Xm.XmPUSHBUTTON);
	Xt.XtManageChildren(children, 2);
    |'I' :
	(* 1 vertical button + one arrow on top *)
	children^[0] := AddStem(form, 0, 63, 25, 38, 0);
	children^[1] := AddStem(form, 30, 63, 100, 38, Xm.XmPUSHBUTTON);
	Xt.XtVaSetValues(children^[0],
			 Xm.XmNarrowDirection, Xm.XmARROW_UP,
			 Xm.XmNshadowThickness, 0,
			 NIL);
	Xt.XtManageChildren(children, 2);
    |'F' :
	(* 1 vertical button and 2 horizontal on the right *)
	children^[0] := AddStem(form, 0, 25, 100, 0, Xm.XmPUSHBUTTON);
	children^[1] := AddStem(form, 0, 100, 25, 25, Xm.XmPUSHBUTTON);
	children^[2] := AddStem(form, 36, 75, 61, 25, Xm.XmPUSHBUTTON);
	Xt.XtManageChildren(children, 3);
    ELSE
	children^[0] := AddStem(form, 0, 100, 100, 0, Xm.XmPUSHBUTTON);
	Xt.XtManageChildren(children, 1);
    END;
END FillItPlease;

TYPE
    paChar = POINTER TO ARRAY [0..MAX(INTEGER)-1] OF CHAR;

VAR
    app_context: Xt.XtAppContext;
    toplevel, topform: Xt.Widget;
    forms: Xt.WidgetList;
    len_word, p: Xt.Cardinal;
    argc: INTEGER;
    argv: Xt.StringList;
    i: INTEGER;
    s: ARRAY [0..1] OF CHAR;
    pp: paChar;
    
CONST
 word = "MOTIF";

BEGIN
    argc := ProgEnv.ArgNumber();
    ALLOCATE(argv, VAL(INTEGER,SIZE(Xt.String))*(argc+1));
    ALLOCATE(argv^[0], SIZE(CHAR)*(ProgEnv.ProgramNameLength()+1));
    pp := SYSTEM.CAST(paChar, argv^[0]);
    ProgEnv.ProgramName( pp^ );
    FOR i:=1 TO argc DO
      ALLOCATE(argv^[i], SIZE(CHAR)*(ProgEnv.ArgLength(i)+1));
      pp := SYSTEM.CAST(paChar, argv^[i]);
      ProgEnv.GetArg(i, pp^);
    END;
    INC(argc);
    toplevel := Xt.XtAppInitialize(app_context, "XmDemos", NIL, 0,
			           argc, argv, NIL, NIL, 0);
    topform := F.XmCreateForm(toplevel, "topform", NIL, 0);
(*  topform := Xt.XtVaCreateWidget("topform", F.xmFormWidgetClass, toplevel, NIL);*)
    Xt.XtManageChild(topform);

    (* create a form for each letter and fill it with the 'stem widgets' *)
    len_word := LENGTH(word);
    ALLOCATE(forms, SIZE(Xt.Widget)*len_word);
    s[1] := 0C ;  (* string used for the subform widget name *)
    FOR p := 0 TO len_word DO
	s[0] := word[p] ;  (* subform  names are the letters themselves *)
	forms^[p] := F.XmCreateForm(topform, s, NIL, 0); 
	FillItPlease(forms^[p], word[p]);
    END;

    (* evenly spread out the forms with some spacing in between *)
    NearlyEvenSpread(forms, len_word, 
		     2,          (* horizontal spacing *)
		     9) ;        (* vertical margin *)

    Xt.XtManageChildren(forms, len_word);
    Xt.XtRealizeWidget(toplevel);
    Xt.XtAppMainLoop(app_context);
END xmform.





