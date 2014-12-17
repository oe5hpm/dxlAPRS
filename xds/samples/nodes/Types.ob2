MODULE Types; (* Ned *)

IMPORT  Nodes, tio:=STextIO, oberonRTS, SYSTEM;

TYPE
  Node = POINTER TO NodeDesc;
  NodeDesc = RECORD (Nodes.NodeDesc)
  END;

VAR node: Nodes.Node; (* for type check in InsertNode *)

PROCEDURE (n: Node) GetName(VAR s: ARRAY OF CHAR);
BEGIN
  COPY("Types manager",s);
END GetName;

PROCEDURE (n: Node) Help;
BEGIN
  tio.WriteString('  Type:'); tio.WriteLn;
  tio.WriteString('   "ModuleName.Type" to create and insert the node'); tio.WriteLn;
  tio.WriteString('   "ModuleName"      to list all types of the module'); tio.WriteLn;
END Help;

PROCEDURE OneName(x: SYSTEM.PTR; name: ARRAY OF CHAR): BOOLEAN;
BEGIN
  tio.WriteString("  ");
  tio.WriteString(name);
  tio.WriteLn;
  RETURN TRUE
END OneName;

PROCEDURE ListTypes(m: oberonRTS.Module);
BEGIN
  tio.WriteString("List of types:"); tio.WriteLn;
  oberonRTS.IterTypes(m,NIL,OneName);
END ListTypes;

PROCEDURE InsertNode(m: oberonRTS.Module; s: ARRAY OF CHAR);
  VAR type: oberonRTS.Type; n: Nodes.Node;
BEGIN
  type:=oberonRTS.ThisType(m,s);
  IF type = oberonRTS.nullType THEN
    tio.WriteString("no such type"); tio.WriteLn;
  ELSIF oberonRTS.BaseOf(type,0) # oberonRTS.TypeOf(node) THEN
    tio.WriteString("not an extension of Nodes.Node"); tio.WriteLn;
  ELSE
    n:=SYSTEM.VAL(Nodes.Node,oberonRTS.NewObj(type)); ASSERT(n#NIL);
    Nodes.Insert(n);
    tio.WriteString("new node is inserted"); tio.WriteLn;
  END;
END InsertNode;

PROCEDURE (n: Node) Handle(s: ARRAY OF CHAR);
  VAR cmd: ARRAY 64 OF CHAR; i,j: INTEGER;
    m: oberonRTS.Module;
BEGIN
  i:=0;
  WHILE (i < LEN(s)) & (s[i] # '.') & (s[i] # 0X) DO INC(i) END;
  IF i < LEN(s) THEN
    j:=0;
    IF s[i] = '.' THEN
      s[i]:=0X; INC(i); j:=0;
      WHILE (i < LEN(s)) & (s[i] # 0X) & (j < LEN(cmd)-1) DO
        cmd[j]:=s[i]; INC(j); INC(i);
      END;
    END;
    cmd[j]:=0X;
  END;
  m:=oberonRTS.Search(s);
  IF m = oberonRTS.nullModule THEN
    tio.WriteString("no such module"); tio.WriteLn
  ELSIF cmd[0] = 0X THEN ListTypes(m);
  ELSE InsertNode(m,cmd)
  END
END Handle;

PROCEDURE Insert*;
  VAR n: Node;
BEGIN
  NEW(n);
  Nodes.Insert(n);
END Insert;

BEGIN
  NEW(node);
END Types.

