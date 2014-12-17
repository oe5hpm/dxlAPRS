/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
#ifndef xmRTS_H_
#define xmRTS_H_
#include "X2C.h"
#include "xrsetjmp.h"

#define X2C_MSG_LEN 1024

#define X2C_EXT 16

#define X2C_HIS_LEN 256

#define X2C_REGSIZE 64

typedef X2C_CHAR * X2C_pCHAR;

typedef void * X2C_pVOID;

typedef X2C_CARD32 * LSET;

typedef X2C_CARD32 X2C_HIS_BUFFER[256];

struct X2C_Coroutine_STR;

typedef struct X2C_Coroutine_STR * X2C_Coroutine;

struct X2C_XHandler_STR;

typedef struct X2C_XHandler_STR * X2C_XHandler;

struct X2C_XSource_STR;

typedef struct X2C_XSource_STR * X2C_XSource;

typedef X2C_pCHAR * X2C_ppCHAR;

typedef X2C_pVOID * X2C_ppVOID;

typedef X2C_CARD16 * X2C_pCARD16;

typedef X2C_pCARD16 * X2C_ppCARD16;

struct X2C_TD_STR;

typedef struct X2C_TD_STR * X2C_TD;

struct X2C_MD_STR;

typedef struct X2C_MD_STR * X2C_MD;

struct X2C_LINK_STR;

typedef struct X2C_LINK_STR * X2C_LINK;


struct X2C_XSource_STR {
   X2C_CARD32 number;
   X2C_CHAR message[1024];
};


struct X2C_Coroutine_STR {
   X2C_PROTECTION prot;
   X2C_INT16 int_no;
   X2C_ADDRESS stk_start;
   X2C_ADDRESS stk_end;
   X2C_Coroutine fwd;
   X2C_Coroutine bck;
   X2C_PROC proc;
   X2C_INT32 magic;
   X2C_jmp_buf buf;
   X2C_XHandler handler;
   X2C_HIS_BUFFER his;
   X2C_INT16 his_cnt;
   X2C_CHAR his_msg[1024];
   X2C_CARD32 reg_dump[64];
   X2C_CARD32 reg_dsize;
};


struct X2C_XHandler_STR {
   X2C_INT16 state;
   X2C_INT16 history;
   X2C_XSource source;
   X2C_XHandler next;
   X2C_jmp_buf buf;
};


struct X2C_MD_STR {
   X2C_MD next;
   X2C_MD cnext;
   X2C_pCHAR name;
   X2C_ppVOID offs;
   X2C_PROC * cmds;
   X2C_ppCHAR cnms;
   X2C_TD types;
};


struct X2C_TD_STR {
   size_t size;
   X2C_pCHAR name;
   X2C_MD module;
   X2C_TD next;
   X2C_INT16 methods;
   X2C_INT16 level;
   X2C_TD base[16];
   X2C_PROC * proc;
   X2C_ppVOID offs;
   X2C_TD succ;
   X2C_LINK link;
   X2C_LINK tail;
   X2C_TD self;
   X2C_CARD32 res;
};


struct X2C_LINK_STR {
   union {
      X2C_TD td;
      X2C_LINK next;
      X2C_CARD32 dummy;
   } _;
   union {
      X2C_CARD32 size;
      X2C_CARD32 tags;
   } _0;
};

#define X2C_st_normal 0

#define X2C_st_exceptional 1

#define X2C_st_off 2

#define X2C_st_reraise 3

extern X2C_XSource X2C_rtsSource;

extern X2C_XSource X2C_assertSrc;

extern X2C_MD X2C_MODULES;

extern X2C_INT32 X2C_hline;

extern X2C_BOOLEAN XDSLIB_INITIALIZATION_FAILED;

extern X2C_BOOLEAN SUPPRESS_XDSLIB_FINALIZATION;

extern void X2C_SetCurrent(X2C_Coroutine);

extern X2C_Coroutine X2C_GetCurrent(void);

extern void X2C_ZEROMEM(X2C_ADDRESS, X2C_CARD32);

extern X2C_CHAR X2C_CAP(X2C_CHAR);

extern int X2C_STRCMP_PROC(X2C_pVOID, size_t, X2C_pVOID, size_t);

extern X2C_INT32 X2C_ASH(X2C_INT32, X2C_INT32);

extern X2C_INT16 X2C_ASH16(X2C_INT16, X2C_INT32);

extern X2C_INT8 X2C_ASH8(X2C_INT8, X2C_INT32);

extern X2C_CARD32 X2C_ROT(X2C_CARD32, X2C_INT16, X2C_INT32);

extern LSET X2C_ROTL(LSET, LSET, X2C_INT16, X2C_INT32);

extern X2C_CARD32 X2C_LSH(X2C_CARD32, X2C_INT16, X2C_INT32);

extern LSET X2C_LSHL(LSET, LSET, X2C_INT16, X2C_INT32);

extern LSET X2C_INCL(LSET, X2C_CARD32, X2C_CARD16);

extern LSET X2C_EXCL(LSET, X2C_CARD32, X2C_CARD16);

extern X2C_BOOLEAN X2C_SET_EQU(LSET, LSET, X2C_CARD16);

extern X2C_BOOLEAN X2C_SET_LEQ(LSET, LSET, X2C_CARD16);

extern LSET X2C_LONGSET(LSET, X2C_CARD32, X2C_CARD32, X2C_CARD16);

extern LSET X2C_AND(LSET, LSET, LSET, X2C_CARD16);

extern LSET X2C_OR(LSET, LSET, LSET, X2C_CARD16);

extern LSET X2C_XOR(LSET, LSET, LSET, X2C_CARD16);

extern LSET X2C_BIC(LSET, LSET, LSET, X2C_CARD16);

extern LSET X2C_COMPLEMENT(LSET, LSET, X2C_CARD16);

extern X2C_LONGREAL X2C_EXPRI(X2C_LONGREAL, X2C_INT32);

extern X2C_INT32 X2C_ENTIER(X2C_LONGREAL);

extern X2C_INT32 X2C_TRUNCI(X2C_LONGREAL, X2C_INT32, X2C_INT32);

extern X2C_CARD32 X2C_TRUNCC(X2C_LONGREAL, X2C_CARD32, X2C_CARD32);

extern X2C_pVOID X2C_COPY(X2C_pVOID, size_t, X2C_pVOID, size_t);

extern size_t X2C_LENGTH(X2C_pVOID, size_t);

extern void X2C_XInitHandler(X2C_XHandler);

extern void X2C_XRETRY(void);

extern void X2C_XREMOVE(void);

extern void X2C_XOFF(void);

extern void X2C_XON(void);

extern void X2C_doRaise(X2C_XSource);

extern void X2C_init_exceptions(void);

extern void X2C_TRAP_FC(X2C_INT32, X2C_pCHAR, X2C_CARD32);

extern void X2C_TRAP_F(X2C_INT32);

extern void X2C_ASSERT_F(X2C_CARD32);

extern void X2C_ASSERT_FC(X2C_CARD32, X2C_pCHAR, X2C_CARD32);

extern void X2C_FINALEXE(X2C_PROC);

extern void X2C_FINALDLL(X2C_ADDRESS *, X2C_PROC);

extern void X2C_HALT(X2C_INT32);

extern void X2C_ABORT(void);

extern void X2C_EXIT(void);

extern void X2C_EXITDLL(X2C_ADDRESS *);

typedef void ( *X2C_EXIT_PROC)(void);

extern void X2C_atexit(X2C_EXIT_PROC);

extern void X2C_HIS_SAVE(X2C_INT16 *);

extern void X2C_HIS_RESTORE(X2C_INT16);

extern void X2C_show_history(void);

extern void X2C_show_profile(void);

extern void X2C_ini_profiler(void);

extern void X2C_scanStackHistory(X2C_ADDRESS, X2C_ADDRESS, X2C_BOOLEAN);

extern void X2C_ini_termination(void);

extern X2C_PROTECTION X2C_PROT(void);

extern void X2C_InitCoroutine(X2C_Coroutine, X2C_ADDRESS);

extern void X2C_RegisterCoroutine(X2C_Coroutine);

extern void X2C_UnregisterCoroutine(X2C_Coroutine);

extern void X2C_CopyJmpBuf(X2C_Coroutine);

extern void X2C_PROTECT(X2C_PROTECTION *, X2C_PROTECTION);

extern void X2C_TRANSFER(X2C_Coroutine *, X2C_Coroutine);

extern void X2C_IOTRANSFER(X2C_Coroutine *, X2C_Coroutine);

extern void X2C_NEWPROCESS(X2C_PROC, X2C_ADDRESS, X2C_CARD32, X2C_PROTECTION,
                 X2C_Coroutine *);

extern void X2C_ini_coroutines(X2C_ADDRESS);

extern void X2C_reg_stackbotm(X2C_ADDRESS);

typedef size_t X2C_LENS_TYPE;

extern void X2C_ALLOCATE(X2C_ADDRESS *, size_t);

extern void X2C_DEALLOCATE(X2C_ADDRESS *);

extern void X2C_DYNALLOCATE(X2C_ADDRESS *, size_t, size_t [], size_t);

extern void X2C_DYNDEALLOCATE(X2C_ADDRESS *);

extern void X2C_DYNCALLOCATE(X2C_ADDRESS *, size_t, size_t [], size_t);

extern void X2C_DYNCDEALLOCATE(X2C_ADDRESS *);

extern void X2C_NEW(X2C_TD, X2C_ADDRESS *, size_t, X2C_BOOLEAN);

extern void X2C_NEW_OPEN(X2C_TD, X2C_ADDRESS *, size_t, size_t [], size_t,
                X2C_BOOLEAN);

extern void X2C_DISPOSE(X2C_ADDRESS *);

extern void X2C_COLLECT(void);

extern void X2C_PrepareToGC(void);

extern void X2C_FreeAfterGC(void);

typedef void ( *X2C_DPROC)(X2C_ADDRESS);

extern void X2C_DESTRUCTOR(X2C_ADDRESS, X2C_DPROC);

extern X2C_pVOID X2C_GUARDP_F(X2C_pVOID, X2C_TD);

extern X2C_TD X2C_GUARDV_F(X2C_TD, X2C_TD);

extern X2C_pVOID X2C_GUARDPE_F(X2C_pVOID, X2C_TD);

extern X2C_TD X2C_GUARDVE_F(X2C_TD, X2C_TD);

extern X2C_BOOLEAN X2C_GC_INIT(X2C_BOOLEAN, X2C_CARD32, X2C_CARD32);

extern void X2C_GC_INCREASE(X2C_BOOLEAN, X2C_CARD32, X2C_CARD32);

extern void X2C_EXIT_PROFILER(X2C_BOOLEAN);

extern void X2C_INIT_TESTCOVERAGE(void);

extern void X2C_MODULE(X2C_MD);

extern X2C_CARD32 X2C_objects;

extern X2C_CARD32 X2C_busymem;

extern X2C_CARD32 X2C_busylheap;

extern X2C_CARD32 X2C_smallbusy;

extern X2C_CARD32 X2C_normalbusy;

extern X2C_CARD32 X2C_largebusy;

extern X2C_CARD32 X2C_usedmem;

extern X2C_CARD32 X2C_smallused;

extern X2C_CARD32 X2C_normalused;

extern X2C_CARD32 X2C_usedlheap;

extern X2C_CARD32 X2C_maxmem;

extern X2C_CARD32 X2C_threshold;

extern X2C_TD x2c_td_null;

extern X2C_TD x2c_td_ptr;

extern X2C_BOOLEAN X2C_fs_init;

extern X2C_CARD32 X2C_MaxGCTimePercent;

extern X2C_BOOLEAN X2C_GCThrashWarning;

extern void X2C_BEGIN(int *, X2C_ppCHAR, int, X2C_INT32, X2C_INT32);

extern void X2C_INIT_RTS(void);

extern X2C_CARD16 X2C_CHKINX_F(X2C_CARD32, X2C_CARD16);

extern X2C_CARD32 X2C_CHKINXL_F(X2C_CARD32, X2C_CARD32);

extern X2C_INT16 X2C_CHKS_F(X2C_INT16);

extern X2C_INT32 X2C_CHKSL_F(X2C_INT32);

extern X2C_INT16 X2C_CHK_F(X2C_INT16, X2C_INT16, X2C_INT16);

extern X2C_INT32 X2C_CHKL_F(X2C_INT32, X2C_INT32, X2C_INT32);

extern X2C_CARD16 X2C_CHKU_F(X2C_CARD16, X2C_CARD16, X2C_CARD16);

extern X2C_CARD32 X2C_CHKUL_F(X2C_CARD32, X2C_CARD32, X2C_CARD32);

extern X2C_pVOID X2C_CHKNIL_F(X2C_pVOID);

extern X2C_PROC X2C_CHKPROC_F(X2C_PROC);

extern X2C_INT32 X2C_REM_F(X2C_INT32, X2C_INT32);

extern X2C_INT32 X2C_QUO_F(X2C_INT32, X2C_INT32);

extern X2C_INT32 X2C_MOD_F(X2C_INT32, X2C_INT32);

extern X2C_INT32 X2C_DIV_F(X2C_INT32, X2C_INT32);

extern X2C_BOOLEAN X2C_IN(X2C_CARD32, X2C_CARD16, X2C_CARD32);

extern X2C_CARD32 X2C_SET(X2C_CARD32, X2C_CARD32, X2C_CARD16);

extern X2C_BOOLEAN X2C_INL(X2C_CARD32, X2C_CARD16, LSET);

extern int CPLX_CMP(X2C_COMPLEX, X2C_COMPLEX);

extern X2C_COMPLEX CPLX_ADD(X2C_COMPLEX, X2C_COMPLEX);

extern X2C_COMPLEX CPLX_SUB(X2C_COMPLEX, X2C_COMPLEX);

extern X2C_COMPLEX CPLX_MUL(X2C_COMPLEX, X2C_COMPLEX);

extern X2C_COMPLEX CPLX_DIV(X2C_COMPLEX, X2C_COMPLEX);

extern X2C_COMPLEX CPLX_NEG(X2C_COMPLEX);

extern int CPLX_LCMP(X2C_LONGCOMPLEX, X2C_LONGCOMPLEX);

extern X2C_LONGCOMPLEX CPLX_LADD(X2C_LONGCOMPLEX, X2C_LONGCOMPLEX);

extern X2C_LONGCOMPLEX CPLX_LSUB(X2C_LONGCOMPLEX, X2C_LONGCOMPLEX);

extern X2C_LONGCOMPLEX CPLX_LMUL(X2C_LONGCOMPLEX, X2C_LONGCOMPLEX);

extern X2C_LONGCOMPLEX CPLX_LDIV(X2C_LONGCOMPLEX, X2C_LONGCOMPLEX);

extern X2C_LONGCOMPLEX CPLX_LNEG(X2C_LONGCOMPLEX);

extern X2C_LONGCOMPLEX CPLX_L(X2C_COMPLEX);

extern X2C_COMPLEX X2C_EXPCI(X2C_COMPLEX, X2C_INT32);

extern X2C_LONGCOMPLEX X2C_EXPLI(X2C_LONGCOMPLEX, X2C_INT32);

extern X2C_COMPLEX X2C_EXPCR(X2C_COMPLEX, X2C_LONGREAL);

extern X2C_LONGCOMPLEX X2C_EXPLR(X2C_LONGCOMPLEX, X2C_LONGREAL);

extern X2C_CHAR X2C_INCC(X2C_CHAR *, X2C_CARD8, X2C_CHAR, X2C_CHAR);

extern X2C_INT8 X2C_INCS(X2C_INT8 *, X2C_INT8, X2C_INT8, X2C_INT8);

extern X2C_INT16 X2C_INCI(X2C_INT16 *, X2C_INT16, X2C_INT16, X2C_INT16);

extern X2C_INT32 X2C_INC(X2C_INT32 *, X2C_INT32, X2C_INT32, X2C_INT32);

extern X2C_CARD8 X2C_INCUS(X2C_CARD8 *, X2C_CARD8, X2C_CARD8, X2C_CARD8);

extern X2C_CARD16 X2C_INCUI(X2C_CARD16 *, X2C_CARD16, X2C_CARD16,
                X2C_CARD16);

extern X2C_CARD32 X2C_INCU(X2C_CARD32 *, X2C_CARD32, X2C_CARD32, X2C_CARD32);

extern X2C_CHAR X2C_DECC(X2C_CHAR *, X2C_CARD8, X2C_CHAR, X2C_CHAR);

extern X2C_INT8 X2C_DECS(X2C_INT8 *, X2C_INT8, X2C_INT8, X2C_INT8);

extern X2C_INT16 X2C_DECI(X2C_INT16 *, X2C_INT16, X2C_INT16, X2C_INT16);

extern X2C_INT32 X2C_DEC(X2C_INT32 *, X2C_INT32, X2C_INT32, X2C_INT32);

extern X2C_CARD8 X2C_DECUS(X2C_CARD8 *, X2C_CARD8, X2C_CARD8, X2C_CARD8);

extern X2C_CARD16 X2C_DECUI(X2C_CARD16 *, X2C_CARD16, X2C_CARD16,
                X2C_CARD16);

extern X2C_CARD32 X2C_DECU(X2C_CARD32 *, X2C_CARD32, X2C_CARD32, X2C_CARD32);

extern X2C_REAL X2C_VAL_REAL(X2C_LONGREAL);

extern X2C_INT8 X2C_ABS_INT8(X2C_INT8);

extern X2C_INT16 X2C_ABS_INT16(X2C_INT16);

extern X2C_INT32 X2C_ABS_INT32(X2C_INT32);

extern void X2C_PCOPY(X2C_pVOID *, size_t);

extern void X2C_PFREE(X2C_pVOID);

extern void X2C_Profiler(void);


#endif /* xmRTS_H_ */
