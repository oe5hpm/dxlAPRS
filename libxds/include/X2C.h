/* Copyright (c) Excelsior, 1999-2000. All Rights Reserved. */

/* Modifications:
-----------------------------------------------------------------------
   08/Feb/96 Ned  v2.12  X2C_GEN_DLL is introduced.
                         X2C_IMPORT/EXPORT_DATA/FUNC are removed.
-----------------------------------------------------------------------
   15/Mar/96 Ned  v2.12  X2C_PROCLASS is used for M2/O2 procedures only.
                         For ["Pascal"], ["StdCall"], ["SysCall"] the
                         compiler generates X2C_PASCAL, X2C_STDCALL,
                         X2C_SYSCALL respectively. These macros are
                         defined as empty, if not defined.
-----------------------------------------------------------------------
*/

#ifndef X2C_H_
#define X2C_H_

/* #define X2C_export    */ /* add _export when necessary                 */
/* #define X2C_pascal    */ /* #define X2C_PROCLASS pascal to save space  */
/* #define X2C_DEBUG     */ /* print line and file name for any trap      */
/* #define X2C_nolabs    */ /* do not use system 'labs' function          */
/* #define X2C_divide    */ /* use C "/" and "%" instead of QUO and REM   */
/* #define X2C_strcmp    */ /* use C 'strcmp' function                    */
/* #define X2C_values    */ /* use "values.h" instead of "float.h"        */
/* #define X2C_nosigned  */ /* do not use "signed char" type              */
/* #define X2C_varargs   */ /* use "varargs.h" instead of "stdarg.h"      */
/* #define X2C_KRC       */ /* non-ANSI C (Kernighan & Ritchie or Unix V) */
/* #define X2C_no_atexit */ /* do not use atexit function                 */
/* #define X2C_no_spawn  */ /* do not use spawnv function                 */
/* #define X2C_adr_align */ /* address is aligned by sizeof(x*) (Sparc)   */
/* #define X2C_mac_os    */ /* Macintosh OS                               */
/* #define X2C_alt_offs  */ /* alt. form of X2C_OFS                       */
/* #define X2C_System_V  */ /* no_atexit, varargs, values, nosigned & KRC */
/* #define X2C_SCO       */ /* Santa Cruz Operation unix - special case!  */
/* #define X2C_GEN_DLL   */ /* to generate DLL define this macro and      */
                            /* X2C_DLL_EXPORT for exported entries and    */
                            /* X2C_DLL_IMPORT for imported entries.       */
                            /* For WinNT DLLs define:                     */
                            /*   X2C_DLL_EXPORT=__declspec(dllexport)     */
                            /*   X2C_DLL_IMPORT=__declspec(dllimport)     */

/* do not use spaces before preprocessor commands,
   some C compilers (SunOS) do not like this
*/

/* ----------------------- Check configuration ------------------------ */
/*                         -------------------                          */

#ifndef X2C_DLL_EXPORT
#define X2C_DLL_EXPORT __declspec(dllexport)
#endif
#ifndef X2C_DLL_IMPORT
#define X2C_DLL_IMPORT __declspec(dllimport)
#endif

#undef X2C_DLL_TAG
#ifdef X2C_GEN_DLL
#ifdef X2C_C_
#define X2C_DLL_TAG X2C_DLL_EXPORT
#else
#define X2C_DLL_TAG X2C_DLL_IMPORT
#endif
#else
#define X2C_DLL_TAG
#endif

#ifdef X2C_System_V
#ifndef _unix
#define _unix
#endif
#define X2C_no_atexit
#define X2C_varargs
#define X2C_values
#define X2C_nosigned
#define X2C_KRC
#endif

#ifdef X2C_SCO
#define _unix
#define X2C_alt_offs
#define X2C_no_spawn
#endif

#ifdef powerc
#define X2C_mac_os
#endif

#ifdef __mips
#define _unix
#define X2C_adr_align
#define X2C_no_spawn
#endif

#ifdef __alpha
#define _unix
#define X2C_adr_align
#define X2C_no_spawn
#endif

#if !defined(X2C_mac_os) && !defined(_msdos) && \
    !defined(_vms) && !defined(_unix) && !defined(_amiga)
#define _msdos
#endif

#if defined(__EMX__) && !defined(__GNUC__)
#define __GNUC__
#endif

#if defined(_vms) && !defined(X2C_nosigned)
#define X2C_nosigned
#endif

/* ------------------- Include standard libraries --------------------- */
/*                     --------------------------                       */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#ifdef X2C_varargs
#include <varargs.h>
#else
#include <stdarg.h>
#endif
#include <math.h>

/* For BSD: Den 1-Aug-96 */
#include <sys/types.h>

#include <sys/stat.h>

#ifndef X2C_values
#include <float.h>
#else
#include <values.h>
#endif

#ifndef FLT_MAX
#define FLT_MAX MAXFLOAT
#endif

#ifndef DBL_MAX
#define DBL_MAX MAXDOUBLE
#endif

#ifndef LDBL_MAX
#define LDBL_MAX MAXLONGDOUBLE
#endif

/* ---------------------- PROCLASS ---------------------------------- */
/*                        ---------                                   */

#ifdef X2C_export
#undef X2C_export
#define X2C_export _export
#else
#define X2C_export
#endif

#if !defined(X2C_PROCLASS) && (defined(__OS2__) || defined(OS2))
#define X2C_PROCLASS  X2C_export
#endif

#if !defined(X2C_PROCLASS) && (defined(_WINDOWS) || defined(_Windows))
#define X2C_PROCLASS _pascal _far X2C_export
#endif

#ifndef X2C_PROCLASS
#ifdef X2C_pascal
#ifdef __STDC__
#define X2C_PROCLASS _pascal
#else
#define X2C_PROCLASS pascal
#endif
#else
#define X2C_PROCLASS
#endif
#endif

#ifdef __GNUC__
#undef X2C_PROCLASS
#define X2C_PROCLASS
#endif

/* for ["Pascal"] procedures: */
#ifndef X2C_PASCAL
#define X2C_PASCAL
#endif

/* for ["StdCall"] procedures: */
#ifndef X2C_STDCALL
#if defined(_WIN32) || defined(__NT__)
#if defined(_MSC_VER)
#define X2C_STDCALL __stdcall
#elif defined(__WATCOMC__)
#define X2C_STDCALL __stdcall
#else
#define X2C_STDCALL
#endif
#else
#define X2C_STDCALL
#endif
#endif

/* for ["SysCall"] procedures: */
#ifndef X2C_SYSCALL
#ifdef __OS2__
#if defined(__IBMC__)
#define X2C_SYSCALL _System
#elif defined(__WATCOMC__)
#define X2C_SYSCALL __syscall
#else
#define X2C_SYSCALL
#endif
#else
#define X2C_SYSCALL
#endif
#endif

/* ------------------------- Basic types ----------------------- */
/*                           -----------                         */

#ifdef X2C_nosigned
#define X2C_INT8 char
#else
#define X2C_INT8 signed char
#endif
#define X2C_CARD8 unsigned char

#define X2C_INT16 short
#define X2C_CARD16 unsigned short
#ifdef __alpha
/* sizeof(int)=4, but sizeof(long)>4 */
#define X2C_INT32 int
#define X2C_CARD32 unsigned int
#define X2C_INT64 long
#else
#define X2C_INT32 long
#define X2C_CARD32 unsigned long
#endif

#if defined (_MSC_VER) && defined (_INTEGRAL_MAX_BITS) && (_INTEGRAL_MAX_BITS >= 64)
#define X2C_INT64 __int64
#define X2C_CARD64 unsigned __int64
#endif

#define X2C_INDEX size_t

#define X2C_REAL float
#define X2C_LONGREAL double
#define X2C_LONGDOUBLE long double
#define X2C_CHAR char
#define X2C_BOOLEAN char
#define X2C_SET8  X2C_CARD8
#define X2C_SET16 X2C_CARD16
#define X2C_SET32 X2C_CARD32
#define X2C_LOC char
#define X2C_PROTECTION short

typedef X2C_LOC X2C_WORD[4];
typedef X2C_LOC X2C_BYTE[1];
typedef X2C_LOC *X2C_ADDRESS;
/*
  X2C_LSET_BASE must be defined according
  with ccDef.LSET_BITS & xmRTS.LSET!
*/
typedef X2C_CARD32 X2C_LSET_BASE;
typedef X2C_LSET_BASE *X2C_LSET;
typedef struct { X2C_REAL re; X2C_REAL im; } X2C_COMPLEX;
typedef struct { X2C_LONGREAL re; X2C_LONGREAL im; } X2C_LONGCOMPLEX;
#ifdef X2C_KRC
typedef void (X2C_PROCLASS * X2C_PROC) ();
typedef char ** X2C_ppcCHAR;
#else
typedef void (X2C_PROCLASS * X2C_PROC) (void);
typedef const char ** X2C_ppcCHAR;
#endif
typedef union { void * adr; X2C_INT32 val; } X2C_SEQ;

#define X2C_LSET_SIZE (sizeof (X2C_LSET_BASE) * 8)

/* is generated with NOOPTIMIZE+ */
#define X2C_FALSE 0
#define X2C_TRUE  1
#define X2C_NIL   NULL
#define X2C_INTERRUPTIBLE   0
#define X2C_UNINTERRUPTIBLE 1

/* --------------------- MIN(type) and MAX(type) --------------------- */
/*                       -----------------------                       */

#define X2C_max_longint  0x7FFFFFFFL
#ifdef X2C_SCO
#define X2C_min_longint  (-0x80000000L)
#else
#define X2C_min_longint  (-0x7FFFFFFFL-1)
#endif
#ifdef X2C_KRC
#define X2C_max_longcard 0xFFFFFFFFL
#else
#define X2C_max_longcard 0xFFFFFFFFUL
#endif
#define X2C_max_real     ((X2C_REAL)FLT_MAX)
#define X2C_min_real     (-((X2C_REAL)FLT_MAX))
#if defined(__SC__) && __SC__ >= 0x220 && defined(__STDC__)
/* error in SC float.h */
#undef DBL_MAX
#define DBL_MAX 1.7976931348623154e+308
#endif
#define X2C_max_longreal ((X2C_LONGREAL)DBL_MAX)
#define X2C_min_longreal (-((X2C_LONGREAL)DBL_MAX))
#define X2C_max_longdouble ((X2C_LONGDOUBLE)LDBL_MAX)
#define X2C_min_longdouble (-((X2C_LONGDOUBLE)LDBL_MAX))

#define X2C_MIN(type,min_value)    ((type)(min_value))
#define X2C_MAX(type,max_value)    ((type)(max_value))
#define X2C_INRANGE(value,min,max) (((value)>=(min))&&((value)<=(max)))

/* -------------------- language exceptions ------------------------*/
/*                      -------------------                         */

enum X2C_Exceptions {
/* Modula-2 Exceptions */
  X2C_indexException,
  X2C_rangeException,
  X2C_caseSelectException,
  X2C_invalidLocation,
  X2C_functionException,
  X2C_wholeValueException,
  X2C_wholeDivException,
  X2C_realValueException,
  X2C_realDivException,
  X2C_complexValueException,
  X2C_complexDivException,
  X2C_protException,
  X2C_sysException,
  X2C_coException,
  X2C_exException,
/* Oberon-2 Exceptions */
  X2C_assertException,
  X2C_guardException,
/* RTS Exceptions */
  X2C_noMemoryException,
  X2C_internalError,
  X2C_castError,
  X2C_UserBreak,
  X2C_unreachDLL,
  X2C_stack_overflow
};

/* ------------------------- TRAP codes --------------------------- */
/*                           ----------                             */

/* this codes is used by compiler */

#define X2C_RETURN_TRAP         X2C_functionException
#define X2C_CASE_TRAP           X2C_caseSelectException
#define X2C_ASSERT_TRAP         X2C_assertException
#define X2C_GUARD_TRAP          X2C_guardException

/* --------------------------- Arguments ------------------------- */
/*                             ---------                           */

extern X2C_DLL_TAG int    X2C_argc;
extern X2C_DLL_TAG char **X2C_argv;

/* ------------------------- Miscellaneous ----------------------- */
/*                           -------------                         */

#define X2C_MOVE(source,dest,size) memcpy(dest,source,size)

#ifndef X2C_strcmp
#define X2C_STRCMP(a,alen,b,blen) X2C_STRCMP_PROC(a,alen,b,blen)
#else
#define X2C_STRCMP(a,alen,b,blen) strcmp((a),(b))
#endif

#ifdef __TURBOC__
#define X2C_STACK_LIMIT(x)  extern unsigned _stklen=x;
#else
#ifdef __ZTC__
#ifdef __cplusplus
#define X2C_STACK_LIMIT(x)  extern "C" unsigned _stack=0;
#else
#define X2C_STACK_LIMIT(x)  extern unsigned _stack=0;
#endif
#else
#define X2C_STACK_LIMIT(x)
#endif
#endif

#if defined(X2C_mac_os) && !defined(powerc)
#define X2C_adr_aligment 2
#else
#ifdef X2C_adr_align
#define X2C_adr_aligment sizeof(void*)
#else
#define X2C_adr_aligment 1
#endif
#endif

extern X2C_DLL_TAG void X2C_PROCLASS X2C_BEGIN (
#ifndef X2C_KRC
  int *argc, char **argv,int gc_auto, long gc_threshold, long heap_limit
#endif
);

#ifdef _msdos
#define X2C_adr_lss(x,y) ((unsigned long)x<(unsigned long)y)
#define X2C_adr_gtr(x,y) ((unsigned long)x>(unsigned long)y)
#else
#define X2C_adr_lss(x,y) (x<y)
#define X2C_adr_gtr(x,y) (x>y)
#endif


/* ------------------------------ DEBUG ----------------------------    */
/*                                -----                                 */

/* Any function that can cause TRAP when executing can be written       */
/* in the following form:                                               */
/*   #define FNAME(parmlist)  (X2C_SET_HINFO FNAME_F(parmlist))         */
/*   extern ... FNAME_F (profile)                                       */
/* In the debug mode, SET_HINFO macros saves file name and line         */
/* number of the function call.                                         */

#if defined(X2C_DEBUG) && defined(__LINE__) && defined(__FILE__)
#define X2C_SET_HINFO() X2C_hline=__LINE__,
#define X2C_PROC_INP()  X2C_PROC_INP_F(__FILE__,__LINE__)
#define X2C_PROC_PRF(x) X2C_PROC_PRF_F(__FILE__,__LINE__,x)
#define X2C_PROC_OUT()  X2C_PROC_OUT_F()
#else
#undef  X2C_DEBUG
#define X2C_SET_HINFO()
#define X2C_PROC_INP()
#define X2C_PROC_PRF()
#define X2C_PROC_OUT()
#endif

/* --------------------------- CHECKs ---------------------------- */
/*                             ------                              */

#define X2C_CHKINX(i,len)       (X2C_SET_HINFO() X2C_CHKINX_F ((X2C_CARD32)(i),(len)))
#define X2C_CHKINXL(i,len)      (X2C_SET_HINFO() X2C_CHKINXL_F((X2C_CARD32)(i),(len)))
#define X2C_CHKS(i)             (X2C_SET_HINFO() X2C_CHKS_F ((i)))
#define X2C_CHKSL(i)            (X2C_SET_HINFO() X2C_CHKSL_F((i)))
#define X2C_CHK(a,min,max)      (X2C_SET_HINFO() X2C_CHK_F  ((a),(min),(max)))
#define X2C_CHKL(a,min,max)     (X2C_SET_HINFO() X2C_CHKL_F ((a),(min),(max)))
#define X2C_CHKU(a,min,max)     (X2C_SET_HINFO() X2C_CHKU_F ((a),(min),(max)))
#define X2C_CHKUL(a,min,max)    (X2C_SET_HINFO() X2C_CHKUL_F((a),(min),(max)))
#define X2C_CHKNIL(T,p)         (X2C_SET_HINFO() ((T)(X2C_CHKNIL_F((void *)(p)))))
#define X2C_CHKPROC(T,p)        (X2C_SET_HINFO() ((T)(X2C_CHKPROC_F((X2C_PROC)(p)))))

/* ---------------------- Qoutient and modulo --------------------- */
/*                        -------------------                       */

#ifndef X2C_divide
#define X2C_REM(a,b)            (X2C_SET_HINFO() X2C_REM_F(a,b))
#define X2C_QUO(a,b)            (X2C_SET_HINFO() X2C_QUO_F(a,b))
#define X2C_DIVR(a,b)           (X2C_SET_HINFO() X2C_DIVR_F(a,b))
#define X2C_DIVL(a,b)           (X2C_SET_HINFO() X2C_DIVL_F(a,b))
#ifdef X2C_INT64
#define X2C_REM64(a,b)          (X2C_SET_HINFO() X2C_REM64_F(a,b))
#define X2C_QUO64(a,b)          (X2C_SET_HINFO() X2C_QUO64_F(a,b))
#endif
#else
#define X2C_REM(a,b)            ((a)%(b))
#define X2C_QUO(a,b)            ((a)/(b))
#define X2C_DIVR(a,b)           ((a)/(b))
#define X2C_DIVL(a,b)           ((a)/(b))
#ifdef X2C_INT64
#define X2C_REM64(a,b)          ((a)%(b))
#define X2C_QUO64(a,b)          ((a)/(b))
#endif
#endif

#define X2C_MOD(a,b)            (X2C_SET_HINFO() X2C_MOD_F(a,b))
#define X2C_DIV(a,b)            (X2C_SET_HINFO() X2C_DIV_F(a,b))
#ifdef X2C_INT64
#define X2C_MOD64(a,b)          (X2C_SET_HINFO() X2C_MOD64_F(a,b))
#define X2C_DIV64(a,b)          (X2C_SET_HINFO() X2C_DIV64_F(a,b))
#endif

/* ----------------------------- TRAPs -------------------------- */
/*                               -----                            */

#if defined(__LINE__) && defined(__FILE__)
#define X2C_TRAP(no)            (X2C_SET_HINFO() X2C_TRAP_FC(no,__FILE__,__LINE__))
#else
#define X2C_TRAP(no)            (X2C_SET_HINFO() X2C_TRAP_F(no)
#endif

#if defined(__LINE__) && defined(__FILE__)
#define X2C_ASSERT(no)          (X2C_ASSERT_FC(no,__FILE__,__LINE__))
#else
#define X2C_ASSERT(no)          (X2C_SET_HINFO() X2C_ASSERT_F(no))
#endif

/* ----------------------------------------------------------------*/
/*                                                                 */

#define X2C_EXPRR(base,ex)      pow((base),(ex))
#define X2C_CAST(val,fr,to,to_ref) \
((sizeof(fr)<sizeof(to)) ? (X2C_TRAP(X2C_castError),(to_ref)0) : (to_ref)(val))
#define X2C_FIELD_OFS(r,f) (int)(&(((r*)(0))->f))

/* -------------------------- OBERON support ------------------------ */
/*                            --------------                          */

#define X2C_OFS_END ((X2C_ADDRESS)1)
#define X2C_OFS_ARR ((X2C_ADDRESS)2)
#define X2C_OFS_REC ((X2C_ADDRESS)3)
#define X2C_BASE    ((X2C_ADDRESS)4)

#ifdef X2C_alt_offs
#define X2C_OFS(type,field) ((void*)&((type*)4)->field)
#else
#ifdef offsetof
#define X2C_OFS(type,field) ((void*)(X2C_BASE+offsetof(type,field)))
#else
#define X2C_OFS(type,field) ((void*)&(*(type*)4).field)
#endif
#endif

/*
        context =
                { context_unit } X2C_OFS_END
        context_unit =
                <offset>
                X2C_OFS_REC <type_desc> <offset>
                X2C_OFS_ARR <last offset> context_unit
*/

#define X2C_CALL(method_type,type_desc,no) (*(method_type)(type_desc)->proc[no])

#define X2C_GET_TD(ptr) (*(X2C_TD*)(X2C_CHKNIL(X2C_LINK,ptr)-1))

/* ------------------------- Type guards ----------------------- */
/*                           -----------                         */

#define X2C_GUARDP(p,td)        (X2C_SET_HINFO() X2C_GUARDP_F ((p),(td)))
#define X2C_GUARDPE(p,td)       (X2C_SET_HINFO() X2C_GUARDPE_F((p),(td)))
#define X2C_GUARDV(vt,td)       (X2C_SET_HINFO() X2C_GUARDV_F ((vt),(td)))
#define X2C_GUARDVE(vt,td)      (X2C_SET_HINFO() X2C_GUARDVE_F((vt),(td)))


/* setjmp: 0 -- set; 1 -- retry; 2 -- exception */
#define X2C_XTRY(x)   (X2C_XInitHandler(x),X2C_setjmp((x)->buf) < 2)

#ifndef xmRTS_H_
#include "xmRTS.h"
#endif

#ifndef xrcArythmetics_H_
#include "xrcArythmetics.h"
#endif

#ifdef X2C_INT64
#ifndef xrcAri64_H_
#include "xrcAri64.h"
#endif
#endif

#ifdef X2C_SL1
#ifdef __cplusplus
#if __cplusplus
#include "SL1.h"
#endif
#endif
#endif

#if defined _MSC_VER
/* Microsoft Visual C++ specific */
/*
  MSVC runtime library calls WinMain() function instead of main()
  if the program is built for the WINDOWS subsystem, whereas XDS
  always generates main() from the body of the main module.
  Fortunately, MSVC's stdlib.h contains variables that may be
  passed to main().
*/
#include <stdlib.h>
#define X2C_MAIN_DEFINITION int __stdcall WinMain(void *hInstance,     \
                                                  void *hPrevInstance, \
                                                  char *lpCmdLine,     \
                                                  int nCmdShow)        \
                                       {  main(__argc, __argv); }
#else
#define X2C_MAIN_DEFINITION
#endif /* _MSC_VER */

#include "xrsetjmp.h"
#endif
