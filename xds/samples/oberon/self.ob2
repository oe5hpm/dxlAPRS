<*+ MAIN *> MODULE self;
IMPORT InOut;
CONST C=27X; F=6; L=15;
VAR s: ARRAY F+L OF ARRAY 64 OF CHAR;
    i: INTEGER;
BEGIN
  s[ 0] := '<*+ MAIN *> MODULE self;';
  s[ 1] := 'IMPORT InOut;';
  s[ 2] := 'CONST C=27X; F=6; L=15;';
  s[ 3] := 'VAR s: ARRAY F+L OF ARRAY 64 OF CHAR;';
  s[ 4] := '    i: INTEGER;';
  s[ 5] := 'BEGIN';
  s[ 6] := '  FOR i := 0 TO (F+L)*2-1 DO';
  s[ 7] := '    IF i>=F*2+L THEN InOut.WriteString (s[i-F-L])';
  s[ 8] := '    ELSIF i<F   THEN InOut.WriteString (s[i])';
  s[ 9] := '    ELSE';
  s[10] := '      InOut.WriteString ("  s[");';
  s[11] := '      InOut.WriteInt (i-F, 2);';
  s[12] := '      InOut.WriteString ("] := ");';
  s[13] := '      InOut.Write (C);';
  s[14] := '      InOut.WriteString (s[i-F]);';
  s[15] := '      InOut.Write (C);';
  s[16] := '      InOut.Write (";")';
  s[17] := '    END;';
  s[18] := '    InOut.WriteLn;';
  s[19] := '  END';
  s[20] := 'END self.';
  FOR i := 0 TO (F+L)*2-1 DO
    IF i>=F*2+L THEN InOut.WriteString (s[i-F-L])
    ELSIF i<F   THEN InOut.WriteString (s[i])
    ELSE
      InOut.WriteString ("  s[");
      InOut.WriteInt (i-F, 2);
      InOut.WriteString ("] := ");
      InOut.Write (C);
      InOut.WriteString (s[i-F]);
      InOut.Write (C);
      InOut.Write (";")
    END;
    InOut.WriteLn;
  END
END self.
