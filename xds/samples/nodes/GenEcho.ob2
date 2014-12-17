MODULE GenEcho; (* Ned *)

IMPORT  Nodes, Echo, tio:=STextIO, WholeStr;

TYPE
  Node = POINTER TO NodeDesc;
  NodeDesc = RECORD (Nodes.NodeDesc)
  END;

PROCEDURE (n: Node) GetName(VAR s: ARRAY OF CHAR);
BEGIN
  COPY("Echo generator",s);
END GetName;

PROCEDURE (n: Node) Help;
BEGIN
  tio.WriteString("Type <number> to produce <number> Echo nodes"); tio.WriteLn;
END Help;

PROCEDURE (n: Node) Handle(s: ARRAY OF CHAR);
  VAR res: WholeStr.ConvResults; i: LONGINT;
BEGIN
  WholeStr.StrToInt(s,i,res);
  IF res = WholeStr.strAllRight THEN
    WHILE i>0 DO Echo.Insert; DEC(i) END;
  ELSE
    tio.WriteString("number expected"); tio.WriteLn;
  END;
END Handle;

PROCEDURE Insert*;
  VAR n: Node;
BEGIN
  NEW(n);
  Nodes.Insert(n);
END Insert;

END GenEcho.

