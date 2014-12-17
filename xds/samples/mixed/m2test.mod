IMPLEMENTATION MODULE m2test;

IMPORT STextIO, SWholeIO, stdio;

PROCEDURE ["C"] m2func(a,b: INTEGER): INTEGER;
BEGIN
  STextIO.WriteString("m2func(");
  SWholeIO.WriteInt(a,0); STextIO.WriteChar(',');
  SWholeIO.WriteInt(b,0); STextIO.WriteChar(')');
  STextIO.WriteLn;
  stdio.printf("And now calling 'printf' from Modula-2: m2func(%d,%d)\n",a,b);
  RETURN a*b
END m2func;

END m2test.

