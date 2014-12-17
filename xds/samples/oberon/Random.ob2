(* From M.Raiser, N.Wirth "Programming in Oberon" *)
MODULE Random;

VAR z: LONGINT;

PROCEDURE Uniform*(): REAL;
  CONST
    a = 16807;  m = 2147483647;
    q = m DIV a;  r = m MOD a;
  VAR gamma: LONGINT;
BEGIN
  gamma:=a*(z MOD q) - r*(z DIV q);
  IF gamma>0 THEN z:=gamma
  ELSE z:=gamma+m
  END;
  RETURN z*(1.0/m)
END Uniform;

PROCEDURE rand16*(VAR x: LONGINT);
BEGIN
  x:=ENTIER(Uniform()*10000H);
END rand16;

PROCEDURE InitSeed*(seed: LONGINT);
BEGIN
  z:=seed;
END InitSeed;

BEGIN
  z:=314159;
END Random.


