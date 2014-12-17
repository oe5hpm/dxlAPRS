MODULE Nodes;

IMPORT  oberonRTS, tio:=STextIO, wio:=SWholeIO, res:=SIOResult;

TYPE
  Node* = POINTER TO NodeDesc;
  NodeDesc* = RECORD
    number: INTEGER;
    l,r: Node;
  END;

VAR
  cur: Node; (* current node *)
  count: INTEGER;

PROCEDURE (n: Node) Handle*(command: ARRAY OF CHAR);
(* Handle input *)
END Handle;

PROCEDURE (n: Node) GetName*(VAR s: ARRAY OF CHAR);
(* Returns name of the node *)
BEGIN
  COPY("node",s);
END GetName;

PROCEDURE (n: Node) Help*;
(** Print help message *)
BEGIN
  tio.WriteString("*** help not available ***");
  tio.WriteLn;
END Help;

PROCEDURE Insert*(n: Node);
(** Inserts new node right to the current node. *)
BEGIN
  IF cur = NIL THEN cur:=n; n.l:=n; n.r:=n
  ELSE
    n.r:=cur.r; n.l:=cur;
    n.r.l:=n; n.l.r:=n;
  END;
  n.number:=count; INC(count);
END Insert;

PROCEDURE Delete;
BEGIN
  IF cur.l = cur THEN HALT END;
  cur:=cur.l;
  cur.r:=cur.r.r;
  cur.r.l:=cur;
END Delete;

PROCEDURE Info;
  VAR objects, busymem: oberonRTS.CARDINAL;
BEGIN
  oberonRTS.GetInfo(objects,busymem);
  tio.WriteString("#objects = "); wio.WriteCard(objects,5);
  tio.WriteString("   busymem = "); wio.WriteCard(busymem,5);
  tio.WriteLn;
END Info;

PROCEDURE WriteNodeName(n: Node);
  VAR s: ARRAY 64 OF CHAR;
BEGIN
  wio.WriteInt(n.number,0); tio.WriteChar(" ");
  n.GetName(s); tio.WriteString(s);
END WriteNodeName;

PROCEDURE ShowNodes;
  VAR l: Node;
BEGIN
  l:=cur;
  REPEAT
    tio.WriteString("  ");
    WriteNodeName(l);
    tio.WriteLn;
    l:=l^.l;
  UNTIL l = cur;
END ShowNodes;

PROCEDURE Help*;
BEGIN
  tio.WriteLn;
  tio.WriteString("A simple oberon system supports a list of nodes."); tio.WriteLn;
  tio.WriteString("Each node can handle input in its own way."); tio.WriteLn;
  tio.WriteString("When you type some text it is passed to the"); tio.WriteLn;
  tio.WriteString("current node. The following common operations are"); tio.WriteLn;
  tio.WriteString("also available (starting from the character '.'):"); tio.WriteLn;
  tio.WriteString("   .l - show all nodes in the list"); tio.WriteLn;
  tio.WriteString("   .n - switch to the next node"); tio.WriteLn;
  tio.WriteString("   .p - switch to the previous node"); tio.WriteLn;
  tio.WriteString("   .c - call garbage collector"); tio.WriteLn;
  tio.WriteString("   .d - delete current node"); tio.WriteLn;
  tio.WriteString("   .h - print the current node help"); tio.WriteLn;
  tio.WriteString("   .q - exit the system"); tio.WriteLn;
  tio.WriteString("   .? - print this text"); tio.WriteLn;
  tio.WriteString(""); tio.WriteLn;
  tio.WriteLn;
END Help;

PROCEDURE ProceedLine(s: ARRAY OF CHAR);
  VAR i: INTEGER;
BEGIN
  s[LEN(s)-1]:=0X; i:=0;
  WHILE s[i]=' ' DO INC(i) END;
  IF s[i]='.' THEN
    CASE s[i+1] OF
      |'h','H': cur.Help;
      |'n','N': cur:=cur.l;
      |'p','P': cur:=cur.r;
      |'c','C': Info; oberonRTS.Collect; Info;
      |'l','L': ShowNodes;
      |'d','D': Delete;
      |'q','Q': HALT(0);
      |'?'    : Help;
    ELSE
      tio.WriteString("#Unknown command");
      tio.WriteLn;
    END;
  ELSE
    cur.Handle(s);
  END;
END ProceedLine;

PROCEDURE Loop*;
(** Main loop *)
  VAR s: ARRAY 64 OF CHAR;
BEGIN
  LOOP
    IF cur=NIL THEN EXIT END;
    WriteNodeName(cur); tio.WriteString("> ");
    tio.ReadRestLine(s);
    CASE res.ReadResult() OF
      |res.allRight,res.outOfRange: ProceedLine(s); tio.SkipLine;
      |res.endOfLine: tio.SkipLine;
      |res.endOfInput: HALT;
   END;
  END;
END Loop;

BEGIN
  cur:=NIL;
  count:=0;
END Nodes.
