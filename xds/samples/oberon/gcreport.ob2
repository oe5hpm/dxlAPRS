<* MAIN+ *>
MODULE GCreport; 

IMPORT SYSTEM, oberonRTS, Out;

TYPE Object = POINTER TO ObjDesc;
     ObjDesc = RECORD 
     END;

VAR 
  global: Object;
  count: INTEGER;

PROCEDURE Finalize(a: SYSTEM.PTR);
BEGIN
  INC(count);
  Out.String("Garbage Collector: "); Out.Int(count,0); Out.Ln;
  global:=SYSTEM.VAL(Object,a);
  oberonRTS.InstallFinalizer(Finalize,global);
END Finalize;

PROCEDURE Init;
  VAR o: Object;
BEGIN
  count:=0;
  NEW(o);
  oberonRTS.InstallFinalizer(Finalize,o);
  NEW(o);
  oberonRTS.InstallFinalizer(Finalize,o);
  global:=o;
END Init;

BEGIN
  Init;
  oberonRTS.Collect;
  oberonRTS.Collect;
  oberonRTS.Collect;
  oberonRTS.Collect;
END GCreport.
