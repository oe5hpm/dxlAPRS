MODULE m2main;

IMPORT ctest, STextIO;

VAR str: ARRAY [0..79] OF CHAR;

BEGIN
    str := "This string will be converted to uppercase";
    ctest.StrToUpper (str);
    STextIO.WriteString (str);
    STextIO.WriteLn;
END m2main.
