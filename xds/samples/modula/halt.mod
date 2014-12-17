MODULE halt;

IMPORT TERMINATION, STextIO;

BEGIN
  IF TERMINATION.HasHalted() THEN
    STextIO.WriteString("**** error 1 ****"); STextIO.WriteLn;
  END;
  HALT;
FINALLY
  IF TERMINATION.HasHalted() THEN
    STextIO.WriteString("The program was halted."); STextIO.WriteLn;
  ELSE
    STextIO.WriteString("**** error 2 ****"); STextIO.WriteLn;
  END;
END halt.
