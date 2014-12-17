MODULE Commands; (* Ned *)

IMPORT  Nodes, tio:=STextIO, oberonRTS, SYSTEM;

TYPE
  Node = POINTER TO NodeDesc;
  NodeDesc = RECORD (Nodes.NodeDesc)
  END;

PROCEDURE (n: Node) GetName(VAR s: ARRAY OF CHAR);
BEGIN
  COPY("Commands manager",s);
END GetName;

PROCEDURE (n: Node) Help;
BEGIN
  tio.WriteString('Type:'); tio.WriteLn;
  tio.WriteString('   "ModuleName.Procedure" to call the command'); tio.WriteLn;
  tio.WriteString('   "ModuleName"           to list all commands'); tio.WriteLn;
  tio.WriteString('   "*"                    to list all modules'); tio.WriteLn;
END Help;

PROCEDURE OneName(x: SYSTEM.PTR; name: ARRAY OF CHAR): BOOLEAN;
BEGIN
  tio.WriteString("  ");
  tio.WriteString(name);
  tio.WriteLn;
  RETURN TRUE
END OneName;

PROCEDURE ListModules;
BEGIN
  tio.WriteString("List of modules:"); tio.WriteLn;
  oberonRTS.IterModules(NIL,OneName);
END ListModules;

PROCEDURE ListCommands(m: oberonRTS.Module);
BEGIN
  tio.WriteString("List of commands:"); tio.WriteLn;
  oberonRTS.IterCommands(m,NIL,OneName);
END ListCommands;

PROCEDURE CallCommand(m: oberonRTS.Module; s: ARRAY OF CHAR);
  VAR c: oberonRTS.Command;
BEGIN
  c:=oberonRTS.ThisCommand(m,s);
  IF c = NIL THEN tio.WriteString("command not found"); tio.WriteLn;
  ELSE c;
  END;
END CallCommand;

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
  IF s = '*' THEN ListModules
  ELSE
    m:=oberonRTS.Search(s);
    IF m = oberonRTS.nullModule THEN
      tio.WriteString("Module not found"); tio.WriteLn
    ELSE
      IF cmd[0] = 0X THEN ListCommands(m)
      ELSE CallCommand(m,cmd)
      END;
    END;
  END
END Handle;

PROCEDURE Insert*;
  VAR n: Node;
BEGIN
  NEW(n);
  Nodes.Insert(n);
END Insert;

END Commands.

