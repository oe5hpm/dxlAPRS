<*+ MAIN *> 
MODULE sieve;

IMPORT In, Out;

CONST
  SIZE = 8190;

VAR
  iter, maxiter, count: LONGINT;
  flags: ARRAY SIZE+1 OF BOOLEAN;

PROCEDURE iteration;
VAR
  i, k, prime: LONGINT;
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
  In.Open;
  Out.String ("Iterations? "); In.LongInt (maxiter); Out.Ln;
  IF ~ In.Done THEN 
    maxiter:=1;
    Out.String("Wrong number - one iteration"); Out.Ln;    
  END;
  count := 0;
  FOR iter := 1 TO maxiter DO iteration END;
  Out.String("There are "); Out.Int(count+1, 0);
  Out.String(" primes in range 1..");
  Out.Int(SIZE*2+4,0); Out.Ln;
END sieve.
