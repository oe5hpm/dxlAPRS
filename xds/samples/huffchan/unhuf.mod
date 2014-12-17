MODULE unhuf;

IMPORT SYSTEM, RawIO, SeqFile, IOResult, IOConsts, IOChan, STextIO, 
       HuffChan, TextIO, ProgramArgs;
FROM Storage IMPORT ALLOCATE;
FROM Storage IMPORT DEALLOCATE;

VAR
  ifl,ofl,zfl,p:  SeqFile.ChanId;
  res:  SeqFile.OpenResults;
  ch : CHAR;
  nm1,nm2 : ARRAY [0..255] OF CHAR;

BEGIN
  p:=ProgramArgs.ArgChan();
  IF ProgramArgs.IsArgPresent() THEN
    TextIO.ReadToken(p,nm1);
    ProgramArgs.NextArg();
  END;
  IF NOT ProgramArgs.IsArgPresent() THEN
    STextIO.WriteString("Usage: unhuf packed_file destination_file");
    RETURN;
  END;
  TextIO.ReadToken(p,nm2);
  SeqFile.OpenWrite(ifl,nm2,SeqFile.raw+SeqFile.old,res);
  IF res <> SeqFile.opened THEN
    STextIO.WriteString("Open error");
    STextIO.WriteLn;
    RETURN;
  END;
  SeqFile.OpenRead(ofl,nm1,SeqFile.raw+SeqFile.old,res);
  IF res <> SeqFile.opened THEN
    STextIO.WriteString("Open error");
    STextIO.WriteLn;
    RETURN;
  END;
  HuffChan.CreateAlias(zfl,ofl,res);
  LOOP
    RawIO.Read(zfl,ch);
    IF IOChan.ReadResult(zfl)<> IOConsts.allRight THEN EXIT; END;
    RawIO.Write(ifl,ch);
  END;
  HuffChan.DeleteAlias(zfl);
  SeqFile.Close(ifl);
  SeqFile.Close(ofl);
END unhuf.