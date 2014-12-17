MODULE hisdemo;

PROCEDURE Div(a,b: INTEGER): INTEGER;
BEGIN
  RETURN a DIV b
END Div;

PROCEDURE Try;
  VAR res: INTEGER;
BEGIN
  res:=Div(1,0);
END Try;

BEGIN
  Try;
END hisdemo.
