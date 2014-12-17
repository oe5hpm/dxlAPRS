(* Print first 'PRINT' digits of 'e'.
 *
 * Originally written in Pascal by Scott Hemphill
 * Rewritten in Oberon-2 and modified by Andrew Cadach
 *
 *)

<*- CHECKINDEX *>
<*+ MAIN *>
MODULE exp;

IMPORT InOut;

CONST
   PRINT = 1024;
   DIGITS = PRINT + (PRINT+31) DIV 32;

TYPE Number = ARRAY DIGITS+1 OF INTEGER;

VAR
   s, x: POINTER TO Number;
   xs, i: INTEGER;

PROCEDURE init (VAR x: Number; n: INTEGER);
VAR i: INTEGER;
BEGIN
   x[0] := n;
   FOR i := 1 TO DIGITS DO x[i] := 0; END
END init;

PROCEDURE divide (VAR x: Number; xs, n: INTEGER;
                  VAR y: Number; VAR ys: INTEGER);
VAR
   i, c: INTEGER;
BEGIN
   c := 0;
   FOR i := xs TO DIGITS DO
      c := 10*c + x[i];
      y[i] := c DIV n;
      c := c MOD n
   END;
   ys := xs;
   WHILE (ys <= DIGITS) & (y[ys] = 0) DO INC (ys) END
END divide;

PROCEDURE add (VAR s, x: Number; xs: INTEGER);
VAR
   i, c: INTEGER;
BEGIN
   c := 0;
   FOR i := DIGITS TO xs BY -1 DO
      INC (c,  s[i] + x[i]);
      IF c >= 10 THEN
         s[i] := c - 10;
         c := 1
      ELSE
         s[i] := c;
         c := 0
      END
   END;
   i := xs;
   WHILE c # 0 DO
      DEC (i);
      INC (c, s[i]);
      IF c >= 10 THEN
         s[i] := c - 10;
         c := 1
      ELSE
         s[i] := c;
         c := 0
      END
   END
END add;

BEGIN
   NEW (s);
   NEW (x);
   init (s^, 0);
   init (x^, 1);
   xs := 0;
   add (s^, x^, xs);
   i := 0;
   REPEAT
      INC (i);
      divide (x^, xs, i, x^, xs);
      add (s^, x^, xs);
   UNTIL xs > DIGITS;
   InOut.WriteLn;
   InOut.WriteString ("   e = ");
   InOut.Write (CHR (s^[0]+ORD ('0')));
   InOut.Write ('.');
   FOR i := 1 TO PRINT DO
      InOut.Write (CHR (s^[i]+ORD ('0')));
      IF i MOD 64 = 0 THEN
        InOut.WriteLn;
        InOut.WriteInt (i, 5);
        InOut.WriteString ("    ")
      END;
   END;
   InOut.WriteLn;
   InOut.WriteLn;
END exp.
