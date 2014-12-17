MODULE queens;

IMPORT InOut;

VAR i, n: INTEGER;
    a: ARRAY [ 1.. 8] OF BOOLEAN;
    b: ARRAY [ 2..16] OF BOOLEAN;
    c: ARRAY [-7.. 7] OF BOOLEAN;

PROCEDURE try(i: INTEGER);
  VAR j: INTEGER;
BEGIN
  FOR j := 1 TO 8 DO
    IF a[j] & b[i+j] & c[i-j] THEN
      a[j]   := FALSE;
      b[i+j] := FALSE;
      c[i-j] := FALSE;
      IF i<8 THEN
        try (i+1);
      ELSE
        INC (n);
      END;
      a[j]   := TRUE;
      b[i+j] := TRUE;
      c[i-j] := TRUE;
    END;
  END;
END try;

BEGIN
  InOut.WriteString ("Eight Queens Problem Benchmark"); InOut.WriteLn;
  InOut.WriteString ("------------------------------"); InOut.WriteLn;
  InOut.WriteLn;
  FOR i :=  1 TO  8 DO a[i] := TRUE; END;
  FOR i :=  2 TO 16 DO b[i] := TRUE; END;
  FOR i := -7 TO  7 DO c[i] := TRUE; END;
  n := 0;
  try (1);
  InOut.WriteString ("There are ");
  InOut.WriteInt (n, 1);
  InOut.WriteString (" solutions");
END queens.
