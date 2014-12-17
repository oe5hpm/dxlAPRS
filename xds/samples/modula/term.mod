MODULE term;

IMPORT  TERMINATION, STextIO;

BEGIN
  IF TERMINATION.IsTerminating() THEN
    STextIO.WriteString("**** error 1 ****"); STextIO.WriteLn;
  END;
FINALLY
  IF TERMINATION.IsTerminating() THEN
    STextIO.WriteString("is terminating..."); STextIO.WriteLn;
  ELSE
    STextIO.WriteString("**** error 2 ****"); STextIO.WriteLn;
  END;
END term.
