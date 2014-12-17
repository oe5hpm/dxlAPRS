MODULE sieve;

IMPORT InOut;

CONST
  SIZE = 8190;

VAR
  iter, maxiter, count: CARDINAL;
  flags: ARRAY [0..SIZE] OF BOOLEAN;

PROCEDURE iteration;
VAR
  i, k, prime: CARDINAL;
BEGIN
  FOR i := 0 TO SIZE DO
    flags[i] := TRUE;
  END;
  count := 0;
  FOR i := 0 TO SIZE DO
    IF flags[i] THEN
      prime := i * 2 + 3;
      k := i + prime;
      WHILE k <= SIZE DO
        flags[k] := FALSE;
        INC (k, prime);
      END;
      INC (count)
    END
  END
END iteration;

BEGIN
  InOut.WriteString ("Iterations? "); InOut.ReadCard (maxiter); InOut.WriteLn;
  count := 0;
  FOR iter := 1 TO maxiter DO iteration END;
  InOut.WriteString ("There are "); InOut.WriteCard (count+1, 0);
  InOut.WriteString (" primes in range 1 ..");
  InOut.WriteCard (SIZE*2+4, 0); InOut.WriteLn;
END sieve.
