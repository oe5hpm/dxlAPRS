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

<* IF NOT DEFINED (DP) THEN *>
<* NEW DP+ *>
<* END *>

MODULE linnew;

IMPORT SYSTEM;
IMPORT SysClock;
FROM STextIO IMPORT WriteString, ReadString, WriteLn, SkipLine;
FROM <* IF DP THEN *> SLongIO <* ELSE *> SRealIO <* END *> IMPORT WriteFixed;
FROM SWholeIO IMPORT WriteInt;
FROM WholeStr IMPORT StrToInt;
FROM ConvTypes IMPORT ConvResults;
FROM Storage IMPORT ALLOCATE, DEALLOCATE;

(*
**
** LINPACK.C        Linpack benchmark, calculates FLOPS.
**                  (FLoating Point Operations Per Second)
**
** Translated to C by Bonnie Toy 5/88
**
** Modified by Will Menninger, 10/93, with these features:
**  (modified on 2/25/94  to fix a problem with daxpy for
**   unequal increments or equal increments not equal to 1.
**     Jack Dongarra)
**
** - Defaults to double precision.
** - Averages ROLLed and UNROLLed performance.
** - User selectable array sizes.
** - Automatically does enough repetitions to take at least 40 CPU seconds.
** - Prints machine precision.
** - ANSI prototyping.
**
** To compile:  cc -O -o linpack linpack.c -lm
**
**
*)

<* IF DP THEN *>

CONST PREC      = "Double";
      BASE10DIG = 14; (* DBL_DIG; *)

TYPE FLOAT = LONGREAL;

<* ELSE *>

CONST PREC      = "Single";
      BASE10DIG = 7; (* FLT_DIG; *)

TYPE FLOAT = REAL;

<* END *>

TYPE 
    FLOATARRAY    = ARRAY [0..10000000] OF FLOAT;
    INTARRAY      = ARRAY [0..10000000] OF INTEGER;
    FLOATARRAYPTR = POINTER TO FLOATARRAY;
    INTARRAYPTR   = POINTER TO INTARRAY;

(*
** Constant times a vector plus a vector.
** Jack Dongarra, linpack, 3/11/78.
** ROLLED version
*)

PROCEDURE daxpy_r (n : INTEGER; da : FLOAT;
                   VAR dx : FLOATARRAY; incx : INTEGER;
                   VAR dy : FLOATARRAY; incy : INTEGER);

VAR i,ix,iy : INTEGER;
BEGIN
    IF n <= 0   THEN RETURN END;
    IF da = 0.0 THEN RETURN END;

    IF (incx <> 1) OR (incy <> 1) THEN

        (* code for unequal increments or equal increments <> 1 *)

        ix := 1;
        iy := 1;
        IF incx < 0 THEN ix := (-n+1)*incx + 1; END;
        IF incy < 0 THEN iy := (-n+1)*incy + 1; END;
        FOR i := 0 TO n-1 DO
            dy[iy] := dy[iy] + da*dx[ix];
            INC (ix, incx);
            INC (iy, incy);
        END;
        RETURN;
    END;

    (* code for both increments equal to 1 *)

    FOR i := 0 TO n-1 DO
        dy[i] := dy[i] + da*dx[i];
    END;
END daxpy_r;

(*
** Forms the dot product of two vectors.
** Jack Dongarra, linpack, 3/11/78.
** ROLLED version
*)

PROCEDURE ddot_r (n : INTEGER; VAR dx : FLOATARRAY; incx : INTEGER;
                               VAR dy : FLOATARRAY; incy : INTEGER) : FLOAT;

VAR dtemp : FLOAT;
    i,ix,iy : INTEGER;
BEGIN
    dtemp := 0.0;

    IF n <= 0 THEN RETURN 0.0; END;
    IF (incx <> 1) OR (incy <> 1) THEN

        (* code for unequal increments or equal increments <> 1 *)

        ix := 0;
        iy := 0;
        IF incx < 0 THEN ix := (-n+1)*incx; END;
        IF incy < 0 THEN iy := (-n+1)*incy; END;
        FOR i := 0 TO n-1 DO
            dtemp := dtemp + dx[ix]*dy[iy];
            INC (ix, incx);
            INC (iy, incy);
        END;
        RETURN dtemp;
    END;

    (* code for both increments equal to 1 *)

    FOR i:=0 TO n-1 DO
        dtemp := dtemp + dx[i]*dy[i];
    END;
    RETURN dtemp;
END ddot_r;


(*
** Scales a vector by a constant.
** Jack Dongarra, linpack, 3/11/78.
** ROLLED version
*)

PROCEDURE dscal_r (n : INTEGER; da : FLOAT;
                   VAR dx : FLOATARRAY; incx : INTEGER);

VAR i : INTEGER;
BEGIN
    IF n <= 0 THEN RETURN END;
    IF incx <> 1 THEN

        (* code for increment not equal to 1 *)

        FOR i := 0 TO n-1 DO
            dx[i*incx] := da*dx[i*incx];
        END;
        RETURN;
    END;

    (* code for increment equal to 1 *)

    FOR i := 0 TO n-1 DO
        dx[i] := da*dx[i];
    END;
END dscal_r;

(*
** constant times a vector plus a vector.
** Jack Dongarra, linpack, 3/11/78.
** UNROLLED version
*)

PROCEDURE daxpy_ur (n : INTEGER; da : FLOAT;
                    VAR dx : FLOATARRAY; incx : INTEGER;
                    VAR dy : FLOATARRAY; incy : INTEGER);

VAR i,ix,iy,m : INTEGER;
BEGIN
    IF n <= 0   THEN RETURN END;
    IF da = 0.0 THEN RETURN END;

    IF (incx <> 1) OR (incy <> 1) THEN

        (* code for unequal increments or equal increments <> 1 *)

        ix := 1;
        iy := 1;
        IF incx < 0 THEN ix := (-n+1)*incx + 1; END;
        IF incy < 0 THEN iy := (-n+1)*incy + 1; END;
        FOR i := 0 TO n-1 DO
            dy[iy] := dy[iy] + da*dx[ix];
            INC (ix, incx);
            INC (iy, incy);
        END;
        RETURN;
    END;

    (* code for both increments equal to 1 *)

    m := n MOD 4;
    IF m <> 0 THEN
        FOR i := 0 TO m-1 DO
            dy[i] := dy[i] + da*dx[i];
        END;
        IF n < 4 THEN
            RETURN;
        END;
    END;
    FOR i := m TO n-1 BY 4 DO
        dy[i]   := dy[i] + da*dx[i];
        dy[i+1] := dy[i+1] + da*dx[i+1];
        dy[i+2] := dy[i+2] + da*dx[i+2];
        dy[i+3] := dy[i+3] + da*dx[i+3];
    END;
END daxpy_ur;

(*
** Forms the dot product of two vectors.
** Jack Dongarra, linpack, 3/11/78.
** UNROLLED version
*)

PROCEDURE ddot_ur (n : INTEGER;
                   VAR dx : FLOATARRAY; incx : INTEGER;
                   VAR dy : FLOATARRAY; incy : INTEGER) : FLOAT;

VAR dtemp : FLOAT;
    i,ix,iy,m : INTEGER;
BEGIN
    dtemp := 0.0;

    IF n <= 0 THEN RETURN 0.0; END;

    IF (incx <> 1) OR (incy <> 1) THEN

        (* code for unequal increments or equal increments != 1 *)

        ix := 0;
        iy := 0;
        IF incx < 0 THEN ix := (-n+1)*incx; END;
        IF incy < 0 THEN iy := (-n+1)*incy; END;
        FOR i := 0 TO n-1 DO
            dtemp := dtemp + dx[ix]*dy[iy];
            INC (ix, incx);
            INC (iy, incy);
        END;
        RETURN dtemp;
    END;

    (* code for both increments equal to 1 *)

    m := n MOD 5;
    IF m <> 0 THEN
        FOR i := 0 TO m-1 DO
            dtemp := dtemp + dx[i]*dy[i];
        END;
        IF n < 5 THEN
            RETURN dtemp;
        END;
    END;
    FOR i := m TO n-1 BY 5 DO
        dtemp := dtemp + dx[i]*dy[i] +
                         dx[i+1]*dy[i+1] +
                         dx[i+2]*dy[i+2] +
                         dx[i+3]*dy[i+3] +
                         dx[i+4]*dy[i+4];
    END;
    RETURN dtemp;
END ddot_ur;


(*
** Scales a vector by a constant.
** Jack Dongarra, linpack, 3/11/78.
** UNROLLED version
*)

PROCEDURE dscal_ur (n : INTEGER; da : FLOAT;
                    VAR dx : FLOATARRAY; incx : INTEGER);

VAR i, m : INTEGER;
BEGIN
    IF n <= 0 THEN RETURN END;
    IF incx <> 1 THEN

        (* code for increment not equal to 1 *)

        FOR i := 0 TO n-1 DO
            dx[i*incx] := da*dx[i*incx];
        END;
        RETURN;
    END;

    (* code for increment equal to 1 *)

    m := n MOD 5;
    IF m <> 0 THEN
        FOR i := 0 TO m-1 DO
            dx[i] := da*dx[i];
        END;
        IF n < 5 THEN RETURN END;
    END;
    FOR i := m TO n-1 BY 5 DO
        dx[i]   := da*dx[i];
        dx[i+1] := da*dx[i+1];
        dx[i+2] := da*dx[i+2];
        dx[i+3] := da*dx[i+3];
        dx[i+4] := da*dx[i+4];
    END;
END dscal_ur;

(*
** Finds the index of element having max. absolute value.
** Jack Dongarra, linpack, 3/11/78.
*)

PROCEDURE idamax (n : INTEGER; VAR dx : FLOATARRAY; incx : INTEGER) : INTEGER;

VAR dmax         : FLOAT;
    i, ix, itemp : INTEGER;

BEGIN
    IF n < 1 THEN RETURN -1 END;
    IF n = 1 THEN RETURN  0 END;
    IF incx <> 1 THEN

        (* code for increment not equal to 1 *)

        ix := 1;
        dmax := ABS (dx[0]);
        INC (ix, incx);
        FOR i := 1 TO n-1 DO
            IF ABS (dx[ix]) > dmax THEN
                itemp := i;
                dmax := ABS (dx[ix]);
            END;
            INC (ix, incx);
        END;
    ELSE

        (* code for increment equal to 1 *)

        itemp := 0;
        dmax := ABS (dx[0]);
        FOR i := 1 TO n-1 DO
            IF ABS (dx[i]) > dmax THEN
                itemp := i;
                dmax := ABS (dx[i]);
            END;
        END;
    END;
    RETURN itemp;
END idamax;

(*
**
** DGEFA benchmark
**
** We would like to declare a[][lda], but c does not allow it.  In this
** function, references to a[i][j] are written a[lda*i+j].
**
**   dgefa factors a double precision matrix by gaussian elimination.
**
**   dgefa is usually called by dgeco, but it can be called
**   directly with a saving in time IF  rcond  is not needed.
**   (time for dgeco) := (1 + 9/n)*(time for dgefa) .
**
**   on entry
**
**      a       FLOAT precision[n][lda]
**              the matrix to be factored.
**
**      lda     integer
**              the leading dimension of the array  a .
**
**      n       integer
**              the order of the matrix  a .
**
**   on RETURN
**
**      a       an upper triangular matrix and the multipliers
**              which were used to obtain it.
**              the factorization can be written  a := l*u  where
**              l  is a product of permutation and unit lower
**              triangular matrices and  u  is upper triangular.
**
**      ipvt    integer[n]
**              an integer vector of pivot indices.
**
**      info    integer
**              := 0  normal value.
**              := k  IF  u[k][k] .eq. 0.0 .  this is not an error
**                   condition for this subroutine, but it does
**                   indicate that dgesl or dgedi will divide by zero
**                   IF called.  use  rcond  in dgeco for a reliable
**                   indication of singularity.
**
**   linpack. this version dated 08/14/78 .
**   cleve moler, university of New Mexico, argonne national lab.
**
**   functions
**
**   blas daxpy,dscal,idamax
**
*)

PROCEDURE dgefa (VAR a : FLOATARRAY; lda : INTEGER; n : INTEGER;
                 VAR ipvt : INTARRAY;
                 VAR info : INTEGER; roll : BOOLEAN);

VAR t : FLOAT;
    j,k,kp1,l,nm1 : INTEGER;
    r, r1 : FLOATARRAYPTR;
BEGIN

    (* gaussian elimination with partial pivoting *)

    IF roll THEN
        info := 0;
        nm1 := n - 1;
        IF nm1 >= 0 THEN
            FOR k := 0 TO nm1-1 DO
                kp1 := k + 1;

                (* find l := pivot index *)

                r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k]));
                l := idamax (n-k, r^, 1) + k;
                ipvt[k] := l;

                (* zero pivot implies this column already
                   triangularized *)

                IF a[lda*k+l] <> 0.0 THEN

                    (* interchange IF necessary *)

                    IF l <> k THEN
                        t := a[lda*k+l];
                        a[lda*k+l] := a[lda*k+k];
                        a[lda*k+k] := t;
                    END;

                    (* compute multipliers *)

                    t := -1.0/a[lda*k+k];
                    r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                    dscal_r (n-(k+1), t, r^, 1);

                    (* row elimination with column indexing *)

                    FOR j := kp1 TO n-1 DO
                        t := a[lda*j+l];
                        IF l <> k THEN
                            a[lda*j+l] := a[lda*j+k];
                            a[lda*j+k] := t;
                        END;
                        r  := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                        r1 := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*j+k+1]));
                        daxpy_r (n-(k+1), t, r^, 1, r1^, 1);
                    END;
                ELSE
                    info := k;
                END;
            END;
        END;
        ipvt[n-1] := n-1;
        IF a[lda*(n-1)+(n-1)] = 0.0 THEN
            info := n-1;
        END;
    ELSE
        info := 0;
        nm1 := n - 1;
        IF nm1 >=  0 THEN
            FOR k := 0 TO nm1-1 DO
                kp1 := k + 1;

                (* find l := pivot index *)

                r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k]));
                l := idamax (n-k, r^, 1) + k;

                ipvt[k] := l;

                (* zero pivot implies this column already
                   triangularized *)

                IF a[lda*k+l] <> 0.0 THEN

                    (* interchange if necessary *)

                    IF l <> k THEN
                        t := a[lda*k+l];
                        a[lda*k+l] := a[lda*k+k];
                        a[lda*k+k] := t;
                    END;

                    (* compute multipliers *)

                    t := -1.0/a[lda*k+k];
                    r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                    dscal_ur (n-(k+1), t, r^, 1);

                    (* row elimination with column indexing *)

                    FOR j := kp1 TO n-1 DO
                        t := a[lda*j+l];
                        IF l <> k THEN
                            a[lda*j+l] := a[lda*j+k];
                            a[lda*j+k] := t;
                        END;
                        r  := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                        r1 := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*j+k+1]));
                        daxpy_ur (n-(k+1), t, r^, 1, r1^, 1);
                    END;
                ELSE
                    info := k;
                END;
            END;
        END;
        ipvt[n-1] := n-1;
        IF a[lda*(n-1)+(n-1)] = 0.0 THEN
            info := n-1;
        END;
    END;
END dgefa;

(*
**
** DGESL benchmark
**
** We would like to declare a[][lda], but c does not allow it.  In this
** function, references to a[i][j] are written a[lda*i+j].
**
**   dgesl solves the double precision system
**   a * x := b  or  trans(a) * x := b
**   using the factors computed by dgeco or dgefa.
**
**   on entry
**
**      a       double precision[n][lda]
**              the output from dgeco or dgefa.
**
**      lda     integer
**              the leading dimension of the array  a .
**
**      n       integer
**              the order of the matrix  a .
**
**      ipvt    integer[n]
**              the pivot vector from dgeco or dgefa.
**
**      b       double precision[n]
**              the right hand side vector.
**
**      job     integer
**              := 0         to solve  a*x := b ,
**              := nonzero   to solve  trans(a)*x := b  where
**                          trans(a)  is the transpose.
**
**  on RETURN
**
**      b       the solution vector  x .
**
**   error condition
**
**      a division by zero will occur IF the input factor contains a
**      zero on the diagonal.  technically this indicates singularity
**      but it is often caused by improper arguments or improper
**      setting of lda .  it will not occur IF the subroutines are
**      called correctly and IF dgeco has set rcond .gt. 0.0
**      or dgefa has set info .eq. 0 .
**
**   to compute  inverse(a) * c  where  c  is a matrix
**   with  p  columns
**         dgeco(a,lda,n,ipvt,rcond,z)
**         IF (!rcond is too small){
**              for (j:=0,j<p,j++)
**                      dgesl(a,lda,n,ipvt,c[j][0],0);
**         END;
**
**   linpack. this version dated 08/14/78 .
**   cleve moler, university of new mexico, argonne national lab.
**
**   functions
**
**   blas daxpy,ddot
*)

PROCEDURE dgesl (VAR a : FLOATARRAY; lda : INTEGER; n : INTEGER;
                 VAR ipvt : INTARRAY; VAR b : FLOATARRAY;
                 job : INTEGER; roll : BOOLEAN);

VAR t : FLOAT;
    k,kb,l,nm1 : INTEGER;
    r, r1 : FLOATARRAYPTR;
BEGIN
    IF roll THEN
        nm1 := n - 1;
        IF job = 0 THEN

            (* job = 0 , solve  a * x = b   *)
            (* first solve  l*y = b         *)

            IF nm1 >= 1 THEN
                FOR k := 0 TO nm1-1 DO
                    l := ipvt[k];
                    t := b[l];
                    IF l <> k THEN
                        b[l] := b[k];
                        b[k] := t;
                    END;
                    r  := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                    r1 := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (b[k+1]));
                    daxpy_r (n-(k+1), t, r^, 1, r1^, 1);
                END;
            END;

            (* now solve  u*x = y *)

            FOR kb := 0 TO n-1 DO
                k := n - (kb + 1);
                b[k] := b[k]/a[lda*k+k];
                t := -b[k];
                r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+0]));
                daxpy_r (k, t, r^, 1, b, 1);
            END;
        ELSE

            (* job = nonzero, solve  trans(a) * x = b  *)
            (* first solve  trans(u)*y = b             *)

            FOR k := 0 TO n-1 DO
                r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+0]));
                t := ddot_r (k, r^, 1, b, 1);
                b[k] := (b[k] - t) / a[lda*k+k];
            END;

            (* now solve trans(l)*x = y     *)

            IF nm1 >= 1 THEN
                FOR kb := 1 TO nm1-1 DO
                    k := n - (kb+1);
                    r  := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                    r1 := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (b[k+1]));
                    b[k] := b[k] + ddot_r (n-(k+1), r^, 1, r1^, 1);
                    l := ipvt[k];
                    IF l <> k THEN
                        t := b[l];
                        b[l] := b[k];
                        b[k] := t;
                    END;
                END;
            END;
        END;
    ELSE
        nm1 := n - 1;
        IF job = 0 THEN

            (* job = 0 , solve  a * x = b   *)
            (* first solve  l*y = b         *)

            IF nm1 >= 1 THEN
                FOR k := 0 TO nm1-1 DO
                    l := ipvt[k];
                    t := b[l];
                    IF l <> k THEN
                        b[l] := b[k];
                        b[k] := t;
                    END;
                    r  := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                    r1 := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (b[k+1]));
                    daxpy_ur (n-(k+1), t, r^, 1, r1^, 1);
                END;
            END;

            (* now solve  u*x = y *)

            FOR kb := 0 TO n-1 DO
                k := n - (kb + 1);
                b[k] := b[k]/a[lda*k+k];
                t := -b[k];
                r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+0]));
                daxpy_ur (k, t, r^, 1, b, 1);
            END;
        ELSE

            (* job = nonzero, solve  trans(a) * x = b  *)
            (* first solve  trans(u)*y = b             *)

            FOR k := 0 TO n-1 DO
                r := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+0]));
                t := ddot_r (k, r^, 1, b, 1);
                b[k] := (b[k] - t) / a[lda*k+k];
            END;

            (* now solve trans(l)*x = y     *)

            IF nm1 >= 1 THEN
                FOR kb := 1 TO nm1-1 DO
                    k := n - (kb+1);
                    r  := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (a[lda*k+k+1]));
                    r1 := SYSTEM.CAST (FLOATARRAYPTR, SYSTEM.ADR (b[k+1]));
                    b[k] := b[k] + ddot_ur (n-(k+1), r^, 1, r1^, 1);
                    l := ipvt[k];
                    IF l <> k THEN
                        t := b[l];
                        b[l] := b[k];
                        b[k] := t;
                    END;
                END;
            END;
        END;
    END;
END dgesl;

(*
** For matgen,
** We would like to declare a[][lda], but c does not allow it.  In this
** function, references to a[i][j] are written a[lda*i+j].
*)

PROCEDURE matgen (VAR a : FLOATARRAY; lda : INTEGER; n : INTEGER;
                  VAR b : FLOATARRAY; VAR norma : FLOAT);

VAR init,i,j : INTEGER;
BEGIN
    init := 1325;
    norma := 0.0;
    FOR j := 0 TO n-1 DO
        FOR i := 0 TO n-1 DO
            init := 3125*init MOD 65536;
            a[lda*j+i] := (VAL (FLOAT, init) - 32768.0)/16384.0;
            IF a[lda*j+i] > norma THEN
                 norma := a[lda*j+i];
            END;
        END;
    END;
    FOR i := 0 TO n-1 DO
        b[i] := 0.0;
    END;
    FOR j := 0 TO n-1 DO
        FOR i := 0 TO n-1 DO
            b[i] := b[i] + a[lda*j+i];
        END;
    END;
END matgen;

PROCEDURE second () : FLOAT;
VAR t: SysClock.DateTime;
BEGIN
    SysClock.GetClock (t);
    RETURN VAL (FLOAT, ((t.hour * 60 + t.minute) * 60 + t.second) * 
                       SysClock.maxSecondParts + t.fractions) / 
           VAL (FLOAT, SysClock.maxSecondParts);
END second;

PROCEDURE linpack (nreps : INTEGER; arsize : INTEGER;
                   VAR a : FLOATARRAY;
                   VAR b : FLOATARRAY;
                   VAR ipvt : INTARRAY) : FLOAT;

VAR norma,t1,kflops,tdgesl,tdgefa,totalt,toverhead,ops : FLOAT;
    i, n, info, lda : INTEGER;

BEGIN
    lda := arsize;
    n   := arsize DIV 2;
    ops := (2.0 * VAL (FLOAT, n) * VAL (FLOAT, n) * VAL (FLOAT, n))/3.0
           +2.0 * VAL (FLOAT, n) * VAL (FLOAT, n);
    tdgesl:=0.0;
    tdgefa:=0.0;
    totalt:=second();
    FOR i:=0 TO nreps-1 DO
        matgen (a, lda, n, b, norma);
        t1 := second();
        dgefa (a,lda,n,ipvt,info,TRUE);
        tdgefa := tdgefa + second() - t1;
        t1 := second();
        dgesl (a,lda,n,ipvt,b,0,TRUE);
        tdgesl := tdgesl + second() - t1;
    END;
    FOR i:=0 TO nreps-1 DO
        matgen (a,lda,n,b,norma);
        t1 := second();
        dgefa (a,lda,n,ipvt,info,FALSE);
        tdgefa := tdgefa + second()-t1;
        t1 := second();
        dgesl (a,lda,n,ipvt,b,0,FALSE);
        tdgesl := tdgesl + second()-t1;
    END;
    totalt:=second()-totalt;
    IF (totalt<0.5) OR (tdgefa+tdgesl<0.2) THEN
        RETURN 0.0;
    END;
    kflops:=2.0*VAL (FLOAT, nreps)*ops/(1000.0*(tdgefa+tdgesl));
    toverhead:=totalt-tdgefa-tdgesl;
    IF tdgefa < 0.0 THEN
        tdgefa:=0.0;
    END;
    IF tdgesl < 0.0 THEN
        tdgesl:=0.0;
    END;
    IF toverhead < 0.0 THEN
        toverhead:=0.0;
    END;
    WriteInt (nreps, 8);                      WriteString (" ");
    WriteFixed (totalt,                2, 6); WriteString (" ");
    WriteFixed (100.*tdgefa/totalt,    2, 6); WriteString ("% ");
    WriteFixed (100.*tdgesl/totalt,    2, 6); WriteString ("% ");
    WriteFixed (100.*toverhead/totalt, 2, 6); WriteString ("% ");
    WriteFixed (kflops, 3, 9); WriteLn;
    RETURN totalt;
END linpack;

VAR buf    : ARRAY [0..79] OF CHAR;
    arsize : INTEGER;
    res    : ConvResults;
    nreps  : INTEGER;
    a, b   : FLOATARRAYPTR;
    ipvt   : INTARRAYPTR;
BEGIN
    LOOP LOOP
        WriteString ("Enter array size (q to quit) [200]:  ");
        ReadString (buf);
        SkipLine;
        IF CAP (buf[0]) ='Q' THEN RETURN END;
        arsize := 0;
        IF buf[0] <> 0C THEN
            StrToInt (buf, arsize, res);
        END;
        IF arsize = 0 THEN arsize := 200 END;
        arsize := arsize + arsize MOD 2;
        IF arsize<10 THEN
            WriteString ("Too small.");
            WriteLn;
            EXIT;
        END;
        ALLOCATE (a,    VAL (CARDINAL, arsize * arsize * SIZE (FLOAT)));
        ALLOCATE (b,    VAL (CARDINAL, arsize * SIZE (FLOAT)));
        ALLOCATE (ipvt, VAL (CARDINAL, arsize * SIZE (INTEGER)));
        IF (a = NIL) OR (b = NIL) OR (ipvt = NIL) THEN
            IF a <> NIL THEN 
                DEALLOCATE (a, VAL (CARDINAL, arsize * arsize * SIZE (FLOAT))); 
            END;
            IF b <> NIL THEN
                DEALLOCATE (b, VAL (CARDINAL, arsize * SIZE (FLOAT)));
            END;
            IF ipvt <> NIL THEN
                DEALLOCATE (ipvt, VAL (CARDINAL, arsize * SIZE (INTEGER)));
            END;
            WriteString ("Not enough memory available for given array size.");
            WriteLn; WriteLn;
            EXIT;
        END;
        WriteLn; WriteLn;
        WriteString ("LINPACK benchmark, ");
        WriteString (PREC);
        WriteString (" precision.");
        WriteLn;
        WriteString ("Machine precision:  ");
        WriteInt (BASE10DIG, 0);
        WriteString (" digits.");
        WriteLn;
        WriteString ("Array size ");
        WriteInt (arsize, 0);
        WriteString (" X ");
        WriteInt (arsize, 0);
        WriteString (".");
        WriteLn;
        WriteString ("Average rolled and unrolled performance:");
        WriteLn; WriteLn;
        WriteString ("    Reps Time(s) DGEFA   DGESL  OVERHEAD    KFLOPS");   WriteLn;
        WriteString ("----------------------------------------------------"); WriteLn;
        nreps:=1;
        WHILE linpack (nreps, arsize, a^, b^, ipvt^) < 40.0 DO
            nreps:=nreps * 2;
        END;
        DEALLOCATE (a, VAL (CARDINAL, arsize * arsize * SIZE (FLOAT)));
        DEALLOCATE (b, VAL (CARDINAL, arsize * SIZE (FLOAT)));
        DEALLOCATE (ipvt, VAL (CARDINAL, arsize * SIZE (INTEGER)));
        WriteLn;
     END END;
END linnew.
