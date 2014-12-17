<* IF __GEN_X86__ THEN *>
  <*+NOPTRALIAS*>
  <*-SPACE*>
  <*-GENHISTORY*>
  <*+DOREORDER*>
<* END *>
<* ALIGNMENT="4"*>
<*+PROCINLINE*>
<*-CHECKINDEX*>
<*-CHECKRANGE*>
<*-CHECKNIL*>
<*-IOVERFLOW*>
<*-COVERFLOW*>
<*-GENDEBUG*>
<*-LINENO*>

MODULE Whet;

IMPORT InOut, RealInOut, LongInOut, SysClock;
FROM LongMath IMPORT sin, cos, arctan, sqrt, exp, ln;

(**********************************************************************
C     Benchmark #2 -- Double  Precision Whetstone (A001)
C
C     o	This is a LONGREAL*8 version of
C	the Whetstone benchmark program.
C     o FOR-loop semantics are ANSI-66 compatible.
C     o	Final measurements are to be made with all
C	WRITE statements and FORMAT sttements removed.
C
C**********************************************************************)

PROCEDURE time (VAR tm: INTEGER);
VAR t: SysClock.DateTime;
BEGIN
    SysClock.GetClock (t);
    tm := (t.hour * 60 + t.minute) * 60 + t.second;
END time;

TYPE ARRAY4 = ARRAY [1..4] OF LONGREAL;

VAR E1                  : ARRAY4;
    T, T1, T2           : LONGREAL;
    J, K, L             : INTEGER;
    ptime, time0, time1 : INTEGER;

PROCEDURE PA (VAR E : ARRAY4);
VAR J1 : INTEGER;
BEGIN
        J1 := 0;
        REPEAT
                E [1] := ( E [1] + E [2] + E [3] - E [4]) * T;
                E [2] := ( E [1] + E [2] - E [3] + E [4]) * T;
                E [3] := ( E [1] - E [2] + E [3] + E [4]) * T;
                E [4] := (-E [1] + E [2] + E [3] + E [4]) / T2;
                J1 := J1 + 1;
        UNTIL J1 >= 6;
END PA;

PROCEDURE P0;
BEGIN
        E1 [J] := E1 [K]; E1 [K] := E1 [L]; E1 [L] := E1 [J];
END P0;

PROCEDURE P3 (X,Y : LONGREAL; VAR Z : LONGREAL);
VAR X1, Y1 : LONGREAL;
BEGIN
        X1 := X;
        Y1 := Y;
        X1 := T * (X1 + Y1);
        Y1 := T * (X1 + Y1);
        Z := (X1 + Y1) / T2;
END P3;

PROCEDURE POUT (N, J, K : INTEGER; X1, X2, X3, X4 : LONGREAL);
VAR time1 : INTEGER;
BEGIN
        time (time1);
        InOut.WriteCard (time1 - time0, 2);
        InOut.WriteCard (time1 - ptime, 2);
        InOut.WriteCard (N, 6);
        InOut.WriteCard (J, 6);
        InOut.WriteCard (K, 6);
        LongInOut.WriteReal (X1, 10);
        LongInOut.WriteReal (X2, 10);
        LongInOut.WriteReal (X3, 10);
        LongInOut.WriteReal (X4, 10);
        InOut.WriteLn;
        ptime := time1;
END POUT;

PROCEDURE Do;
VAR NLoop, I, II, JJ : INTEGER;
    N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11 : INTEGER;
    X1, X2, X3, X4, X, Y, Z : LONGREAL;
BEGIN
        time (time0);
        ptime := time0;
(* The actual benchmark starts here. *)
        T  := 0.499975;
        T1 := 0.50025;
        T2 := 2.0;
(* With loopcount NLoop=10, one million Whetstone instructions
   will be executed in each major loop.
   A major loop is executed 'II' times to increase wall-clock timing accuracy *)
        NLoop := 10;
        II    := 800;
        FOR JJ:=1 TO II DO
(* Establish the relative loop counts of each module. *)
                N1 := 0;
                N2 := 12 * NLoop;
                N3 := 14 * NLoop;
                N4 := 345 * NLoop;
                N5 := 0;
                N6 := 210 * NLoop;
                N7 := 32 * NLoop;
                N8 := 899 * NLoop;
                N9 := 616 * NLoop;
                N10 := 0;
                N11 := 93 * NLoop;
(* Module 1: Simple identifiers *)
                X1 := 1.0;
                X2 := -1.0;
                X3 := -1.0;
                X4 := -1.0;
                FOR I:=1 TO N1 DO
                        X1 := (X1 + X2 + X3 - X4)*T;
                        X2 := (X1 + X2 - X3 + X4)*T;
                        X3 := (X1 - X2 + X3 + X4)*T;
                        X4 := (-X1 + X2 + X3 + X4)*T;
                END;
                IF (JJ = II) THEN
                        POUT (N1, N1, N1, X1, X2, X3, X4);
                END;
(* Module 2: Array elements *)
                E1 [1] :=  1.0;
                E1 [2] := -1.0;
                E1 [3] := -1.0;
                E1 [4] := -1.0;
                FOR I:=1 TO N2 DO
                        E1 [1] := (E1 [1] + E1 [2] + E1 [3] - E1 [4])*T;
                        E1 [2] := (E1 [1] + E1 [2] - E1 [3] + E1 [4])*T;
                        E1 [3] := (E1 [1] - E1 [2] + E1 [3] + E1 [4])*T;
                        E1 [4] := (-E1 [1] + E1 [2] + E1 [3] + E1 [4])*T;
                END;
                IF (JJ = II) THEN
                        POUT (N2, N3, N2, E1 [1], E1 [2], E1 [3], E1 [4]);
                END;
(* Module 3: Array as parameter *)
                FOR I:=1 TO N3 DO
                        PA (E1);
                END;
                IF (JJ = II) THEN
                        POUT(N3, N2, N2, E1 [1], E1 [2], E1 [3], E1 [4]);
                END;
(* Module 4: Conditional jumps *)
                J := 1;
                FOR I:=1 TO N4 DO
                        IF (J <> 1) THEN J := 3 ELSE J := 2 END;
                        IF (J <= 2) THEN J := 1 ELSE J := 0 END;
                        IF (J >= 1) THEN J := 0 ELSE J := 1 END;
                END;
                IF (JJ = II) THEN
                        POUT (N4, J, J, X1, X2, X3, X4)
                END;
(* Module 5: Omitted; Module 6: Integer arithmetic *)
                J := 1;
                K := 2;
                L := 3;
                FOR I:=1 TO N6 DO
                        J := J * (K-J) * (L-K);
                        K := L * K - (L-J) * K;
                        L := (L - K) * (K + J);
                        E1 [L-1] := VAL (LONGREAL, (J + K + L));
                        E1 [K-1] := VAL (LONGREAL, (J * K * L));
                END;
                IF (JJ = II) THEN
                        POUT (N6, J, K, E1 [1], E1 [2], E1 [3], E1 [4]);
                END;
(* Module 7: Trigonometric functions *)
                X := 0.5;
                Y := 0.5;
                FOR I:=1 TO N7 DO
                        X:=T*arctan(T2*sin(X)*cos(X)/(cos(X+Y)+cos(X-Y)-1.0));
                        Y:=T*arctan(T2*sin(Y)*cos(Y)/(cos(X+Y)+cos(X-Y)-1.0));
                END;
                IF (JJ = II) THEN
                        POUT (N7, J, K, X, X, Y, Y);
                END;
(* Module 8: Procedure calls *)
                X := 1.0;
                Y := 1.0;
                Z := 1.0;
                FOR I:=1 TO N8 DO
                        P3 (X,Y,Z);
                END;
                IF (JJ = II) THEN
                        POUT (N8, J, K, X, Y, Z, Z);
                END;
(* Module 9: Array references *)
                J := 1;
                K := 2;
                L := 3;
                E1 [1] := 1.0;
                E1 [2] := 2.0;
                E1 [3] := 3.0;
                FOR I:=1 TO N9 DO
                        P0;
                END;
                IF (JJ = II) THEN
                        POUT (N9, J, K, E1 [1], E1 [2], E1 [3], E1 [4])
                END;
(* Module 10: Integer arithmetic *)
                J := 2;
                K := 3;
                FOR I:=1 TO N10 DO
                        J := J + K;
                        K := J + K;
                        J := K - J;
                        K := K - J - J;
                END;
                IF (JJ = II) THEN
                        POUT (N10, J, K, X1, X2, X3, X4)
                END;
(* Module 11: Standard functions *)
                X := 0.75;
                FOR I:=1 TO N11 DO
                        X := sqrt (exp (ln (X)/T1))
                END;
                IF (JJ = II) THEN
                        POUT (N11, J, K, X, X, X, X)
                END;
(* THIS IS THE END OF THE MAJOR LOOP. *)
        END;
(* Stop benchmark timing at this point. *)
        time (time1);
(*----------------------------------------------------------------
      Performance in Whetstone KIP's per second is given by
       (100*NLoop*II)/TIME
      where TIME is in seconds.
--------------------------------------------------------------------*)
        InOut.WriteLn;
        InOut.WriteString ('Double Whetstone KIPS ');
        InOut.WriteCard (TRUNC ((100.0 * FLOAT (NLoop) * FLOAT (II)) /
                                FLOAT (time1 - time0)), 0);
        InOut.WriteLn;
        InOut.WriteString ('Whetstone MIPS ');
        RealInOut.WriteReal (FLOAT (NLoop) * FLOAT (II) /
                             FLOAT ((time1 - time0) * 10), 10);
        InOut.WriteLn;
END Do;

BEGIN
    Do;
END Whet.
