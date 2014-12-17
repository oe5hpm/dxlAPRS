<*+ MAIN *> 
MODULE ackermann;

IMPORT Out, In;

VAR p1, p2, count: LONGINT;

PROCEDURE ack (m, n: LONGINT): LONGINT;
BEGIN
  INC (count);
  IF m=0 THEN RETURN n+1 END;
  IF n=0 THEN RETURN ack (m-1, 1) END;
  RETURN ack (m-1, ack (m, n-1));
END ack;

BEGIN
  In.Open;
  count := 0;
  Out.String ("Ackermann Function Calculation"); Out.Ln;
  Out.String ("------------------------------"); Out.Ln;
  Out.Ln;
  Out.String ("Enter first  parameter (1..4): "); In.LongInt(p1);
  Out.String ("Enter second parameter (1..7): "); In.LongInt(p2);
  Out.Ln;
  Out.String ("Ackermann ("); Out.Int (p1, 0);
  Out.String (", "); Out.Int(p2, 0);
  Out.String ("): ");
  Out.Int (ack (p1, p2), 0);
  Out.String (" ("); Out.Int (count, 0);
  Out.String (" recursive calls)");
  Out.Ln;
END ackermann.
