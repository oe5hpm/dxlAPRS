MODULE Echo; (* Ned *)

IMPORT  Nodes, tio:=STextIO;

TYPE
  Node = POINTER TO NodeDesc;
  NodeDesc = RECORD (Nodes.NodeDesc)
  END;

PROCEDURE (n: Node) GetName(VAR s: ARRAY OF CHAR);
BEGIN
  COPY("Echo",s);
END GetName;

PROCEDURE (n: Node) Help;
BEGIN
  tio.WriteString("Prints the input line"); tio.WriteLn;
END Help;

PROCEDURE (n: Node) Handle(s: ARRAY OF CHAR);
BEGIN
  tio.WriteString(s); tio.WriteLn;
END Handle;

PROCEDURE Insert*;
  VAR n: Node;
BEGIN
  NEW(n);
  Nodes.Insert(n);
END Insert;

END Echo.

