MODULE except;

IMPORT M2EXCEPTION, WholeConv, STextIO, SWholeIO;

CONST
  ws = STextIO.WriteString;
  wl = STextIO.WriteLn;
  wi = SWholeIO.WriteInt;

PROCEDURE Div(a,b: INTEGER): INTEGER;
BEGIN
  RETURN a DIV b
EXCEPT
  IF M2EXCEPTION.IsM2Exception() THEN
    IF M2EXCEPTION.M2Exception() = M2EXCEPTION.wholeDivException THEN
      ws("*** whole div exception ***"); wl;
      RETURN MAX(INTEGER)
    END;
  END;
END Div;

BEGIN
  wi(Div(1,1),0); wl;
  wi(Div(1,0),0); wl;
END except.
