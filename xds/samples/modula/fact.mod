(*
 * Written by Andrew Cadach
 *
 * Recursive (extremely uneficient:-) implementation of factorial
 *
 *                     n * (n-1)!, n <> 0
 * By definition, n! = 
 *                     1, n = 0
 *
 *)

MODULE fact;

IMPORT InOut;

VAR i, r: CARDINAL;

PROCEDURE f (n: CARDINAL): CARDINAL;
BEGIN
  IF n=0 THEN RETURN 1 END;
  RETURN n * f (n-1);
END f;

BEGIN
  i := 0;
  REPEAT
    r := f(i);
    InOut.WriteString ("The factorial of ");
    InOut.WriteCard (i, 2);
    InOut.WriteString (" is ");
    InOut.WriteCard (r, 0);
    InOut.WriteLn;
    INC (i)
  UNTIL r >= MAX (CARDINAL) / i;
END fact.
