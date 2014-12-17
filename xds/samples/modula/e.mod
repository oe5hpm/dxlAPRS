(* Print first 'NDigits' digits of 'e'.
 *
 * Originally written in Algol by Serge Batalov
 * Rewritten in Modula-2 and modified by Eugene Nalimov, Pavel Zemtsov
 *
 *)

<*- CHECKINDEX *>
<*- CHECKRANGE *>
<*- COVERFLOW  *>
<*- IOVERFLOW  *>

MODULE e;

IMPORT InOut;

CONST
      M       = 100000;       (* Max # OF digits           *)
      NDigits = 20000;        (* actual # of digits        *)
      N       = 4;            (* One "digit" is 0..10 ** N *)
      P       = 10000;        (* 10 ** N                   *)

VAR  x, y: ARRAY [1..M DIV N+5] OF CARDINAL;
        s: ARRAY [1..M]         OF CHAR;
  a, b, r: CARDINAL;

PROCEDURE WrDigits (n, m: CARDINAL);
BEGIN
    IF m <> 0 THEN
        WrDigits (n DIV 10, m - 1);
        s [r] := CHR (ORD ('0') + n MOD 10);
        INC (r);
    END;
END WrDigits;

PROCEDURE Calc (d: INTEGER);
VAR m, e, b: INTEGER;
    k, l, c: CARDINAL;
BEGIN
    IF d REM 100 <> 0 THEN
        d := (d / 100 + 1) * 100;
    END;
    e := d / N + 4;
    FOR b:=1 TO e DO
        x [b] := 0;
        y [b] := 0;
    END;
    y [1] := P;
    l := 0;
    c := 1;
    FOR m:=1 TO e DO
        LOOP
            INC (c);
            FOR b:=m TO e DO
                l := y [b] + l * P;
                y [b] := l DIV c;
                INC (x [b], y [b]);
                DEC (l, c * y [b]);
            END;
            IF y [m] < c THEN
                EXIT;
            END;
            l := 0;
        END;
        l := y [m];
    END;
    l := 0;
    FOR b:=e TO 1 BY -1 DO
        k := x [b] + l;
        l := k DIV P;
        x [b] := k - l * P;
    END;
    r := 1;
    FOR b:=1 TO e-4 DO
        WrDigits (x [b], N);
    END;
END Calc;

BEGIN
    InOut.WriteString ("Please wait, calculating first");
    InOut.WriteInt    (NDigits, 0);
    InOut.WriteString (" digits of 'e'...");
    InOut.WriteLn;

    Calc (NDigits);

    InOut.WriteString ('e = 2.');
    FOR b:=1 TO NDigits BY 50 DO
        FOR a:=0 TO 49 DO
            InOut.Write (s [a + b]);
        END;
        InOut.WriteLn;
        InOut.WriteString ('      ');
    END;
END e.
