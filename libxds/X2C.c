/* Copyright (c) Excelsior 1999. All Rights Reserved. */
/* Copyright (c) xTech 1991,1996 */
/* "@(#)X2C.c v2.0 X2C" */

#include <stdio.h>
#include <signal.h>
#define X2C_C_
#include "X2C.h"
#include "xrsetjmp.h"

#if defined(_msdos)

#if defined(_WIN32) || defined (__NT__) /* Temporary patch */
#include <windows.h>
#endif

#if defined(_MSC_VER) && defined(_WIN32)

/* Microsoft C compiler for Win32 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#define EN_INTS()
#define DIS_INTS()
#define PROFILER_VEC 0
typedef void (*IPROC)();
static void profiler_proc () { X2C_Profiler_clock(); }
static void CALLBACK timer_proc(UINT id, UINT Res, DWORD User,
				DWORD Res1, DWORD Res2)
{
    X2C_Profiler_clock();
}
static void SET_VECTOR(IPROC * old, int no, IPROC proc)
{
    if (no==PROFILER_VEC) {
	*(int*)old=timeSetEvent(10,10,timer_proc,0,TIME_PERIODIC);
    }
}
static void RESET_VECTOR(IPROC old, int no)
{
    if (no==PROFILER_VEC) {
	timeKillEvent(*(int*)&old);
    }
}
#define HANDLER(x,y) void x () { int_transfer(y); }

#else
#ifdef __ZTC__
#ifdef __NT__

/* Symantec for Windows NT */
#define EN_INTS()
#define DIS_INTS()
#define SET_VECTOR(old,no,proc)
#define RESET_VECTOR(old,no)
#define HANDLER(x,y) int x() { int_transfer(y); return 0; }
#define PROFILER_VEC -1
typedef int (*IPROC)();
static int profiler_proc() { X2C_Profiler_clock(); return 0; }

#else

/* Symantec for MSDOS */
#include <int.h>
#define EN_INTS() int_on()
#define DIS_INTS() int_off()
#define SET_VECTOR(old,no,proc) int_intercept(no,proc,512); *old=proc
#define RESET_VECTOR(old,no) int_restore(no);
#define HANDLER(x,y) int __cdecl x (struct INT_DATA * _y_) { int_transfer(y); return 0; }
#define PROFILER_VEC 0x1C
typedef int (__cdecl *IPROC)(struct INT_DATA *);
static int __cdecl profiler_proc (struct INT_DATA * _y_) { X2C_Profiler_clock(); return 0; }

#endif
#else
#ifdef __OS2__

/* OS/2 */

/*
 * In order to get rid of #include <os2.h>, prototypes of OS/2
 * API functions required to patch argv[0] are declared here.
 */

#ifdef __WATCOMC__
/*
 * In ANSI-compliant mode, Watcom C does not allow linkage
 * keywords, so #pragma aux has to be used instead.
 */
#define _System
#pragma aux (__syscall) DosGetInfoBlocks;
#pragma aux (__syscall) DosQueryModuleName;
#endif

unsigned long _System DosGetInfoBlocks(unsigned long **pptib,
				       unsigned long **pppib);

unsigned long _System DosQueryModuleName(unsigned long hmod,
					 unsigned long cbName,
					 char *pch);

#define EN_INTS()
#define DIS_INTS()
#define SET_VECTOR(old,no,proc)
#define RESET_VECTOR(old,no)
#define HANDLER(x,y) void x () { int_transfer(y); }
#define PROFILER_VEC -1
typedef void (*IPROC)();
static void profiler_proc() { X2C_Profiler_clock(); }

/* Under OS/2, underflow exception is not masked by default */

#include <float.h>

/*
 * In Watcom C library, _EM_UNDERFLOW is always defined, but
 * EM_UNDERFLOW is defined only if extensions are enabled.
 * In C Set library, only EM_UNDERFLOW is defined.
 */

#ifndef EM_UNDERFLOW
#define EM_UNDERFLOW _EM_UNDERFLOW
#endif

#else
#ifdef __WATCOMC__
#ifdef __NT__

/* Watcom C for Windows NT */
#define EN_INTS()
#define DIS_INTS()
#define SET_VECTOR(old,no,proc)
#define RESET_VECTOR(old,no)
#define HANDLER(x,y) int x() { int_transfer(y); return 0; }
#define PROFILER_VEC -1
typedef int (*IPROC)();
static int profiler_proc() { X2C_Profiler_clock(); return 0; }

#else

/* Watcom C for MSDOS */
#include <dos.h>
#define EN_INTS() _enable()
#define DIS_INTS() _disable()
#define SET_VECTOR(old,no,proc) *old=_dos_getvect(no); _dos_setvect(no,proc)
#define RESET_VECTOR(old,no) _dos_setvect(no,old)
#define HANDLER(x,y) void __interrupt __far x () { int_transfer(y); }
#define PROFILER_VEC 0x1C
typedef void (__interrupt __far *IPROC)();
static void __interrupt __far profiler_proc() { X2C_Profiler_clock(); }

#endif
#else

#ifdef DJGPP
/*  djgpp  */
#define EN_INTS() 
#define DIS_INTS() 
#define SET_VECTOR(old,no,proc)
#define RESET_VECTOR(old,no)
#define HANDLER(x,y) void x () { int_transfer(y); }
#define PROFILER_VEC -1
typedef void (*IPROC)();
static void profiler_proc() { X2C_Profiler_clock(); }
                          
#else

/* generic DOS */
#include <dos.h>
#define EN_INTS() _enable()
#define DIS_INTS() _disable()
#define SET_VECTOR(old,no,proc) *old=_dos_getvect(no); _dos_setvect(no,proc)
#define RESET_VECTOR(old,no) _dos_setvect(no,old)
#define HANDLER(x,y) void interrupt x () { int_transfer(y); }
#define PROFILER_VEC -1
typedef void interrupt (*IPROC)();
static void interrupt profiler_proc() { X2C_Profiler_clock(); }

#endif
#endif
#endif
#endif
#endif
#else /* not _msdos */
#if defined(_unix) && !defined(X2C_KRC) && !defined(__QNX__) && !defined(X2C_no_profile)

/* unix */
#include <time.h>
#include <sys/time.h>

#ifdef _linux
#include <sys/user.h>
#include <asm/sigcontext.h>
//  static struct user_i387_struct sigfpe_fpu_state;
#endif /* _linux */

#define EN_INTS()
#define DIS_INTS()
#define PROFILER_VEC SIGPROF
#define HANDLER(x,y) void x (int a) { int_transfer(y); }
typedef void (*IPROC)(int);
static void SET_VECTOR(IPROC * old, int no, IPROC proc)
{
	struct itimerval t;
	struct sigaction n;
	sigaction(no,0,&n);
	*old=n.sa_handler;
	n.sa_handler=proc;
	sigaction(no,&n,0);
	if (no==PROFILER_VEC) {
		t.it_interval.tv_sec=0;
		t.it_interval.tv_usec=10000;
		t.it_value=t.it_interval;
		setitimer(ITIMER_PROF,&t,0);
	}
}
static void RESET_VECTOR(IPROC old, int no)
{
	struct sigaction n;
	sigaction(no,0,&n);
	n.sa_handler=old;
	sigaction(no,&n,0);
}
static void profiler_proc(int a) { X2C_Profiler_clock(); }

#else

#define EN_INTS()
#define DIS_INTS()
#define SET_VECTOR(old,no,proc)
#define RESET_VECTOR(old,no)
#define HANDLER(x,y) void x () { int_transfer(y); }
#define PROFILER_VEC -1
typedef void (*IPROC)();
static void profiler_proc() { X2C_Profiler_clock(); }

#endif
#endif

#if defined(__TURBOC__) && !defined(_Windows) && !defined(__OS2__)
	/*------------------------------------------------------------------
	In large data models, the stack is in its own segment.  The stack
	starts at SS:__stklen and grows down to SS:0.
	In small data models, the stack is in the DGROUP, and grows down
	to meet the heap.  The end of the heap is marked by ___brklvl.
	------------------------------------------------------------------*/
extern unsigned _stklen;
#define SET_STK_LIMIT(x) _stklen = FP_OFF(x)
#define GET_STK_LIMIT(x) (x)=(X2C_ADDRESS)MK_FP(FP_SEG(x),_stklen)
#else
#define SET_STK_LIMIT(x)
#define GET_STK_LIMIT(x)
#endif

/* -------------------------- COROUTINES ------------------------------ */
/*                            -----------                               */

#define MAGIC 0x67CA
#ifdef _M_PPC 
#define FRAME_SIZE 256
#define FRAME_ALIGMENT 16
#else
#ifdef _msdos
#define FRAME_SIZE 16
#define FRAME_ALIGMENT 16
#else
#define FRAME_SIZE 128
#define FRAME_ALIGMENT 8
#endif
#endif
static short sp_offs=0;
static short ss_offs=0;
static short stk_up=0;
static char frame_buf[FRAME_SIZE];

#define VEC_NO 256
static short vec2ip[VEC_NO];
#define IPS_NO 4
typedef IPROC VEC_SAVE[IPS_NO];
static X2C_Coroutine IntWaiting[IPS_NO];
static VEC_SAVE vec_save;
static IPROC iprocs[IPS_NO];
static short iprocs_cnt=0;
static IPROC sv_timer=0;

/*
 * The Coroutine_start function has not to be susbtituted inline,
 * so it is placed in a separate file called xrcCrtnStart.c.
 * It has not to be called from anywhere but the Ini_Coroutine function.
 */

#ifndef X2C_KRC
void X2C_PROCLASS Coroutine_start (void);
#else
void X2C_PROCLASS Coroutine_start ();
#endif

#ifndef X2C_KRC
static X2C_Coroutine X2C_PROCLASS Tie_Coroutine (X2C_Coroutine p)
#else
static X2C_Coroutine X2C_PROCLASS Tie_Coroutine (p)
	X2C_Coroutine p;
#endif
{
	X2C_Coroutine current;

	current = X2C_GetCurrent();

	p->magic = MAGIC;
	p->stk_start = 0;
	p->stk_end = 0;
	p->handler = 0;
	p->prot = 0;
	p->proc = 0;
	p->int_no = -1;
	p->his_cnt = 0;
	p->his_msg[0]=0;
	if (!current) {
		p->fwd=p->bck=p;
	}
	else {
		p->fwd=current;
		p->bck=current->bck;
		p->fwd->bck=p->bck->fwd=p;
	}
	return p;
}

#ifndef X2C_KRC
static X2C_Coroutine X2C_PROCLASS Ini_Coroutine (void ** wsp, size_t * size)
#else
static X2C_Coroutine X2C_PROCLASS Ini_Coroutine (wsp,size)
	void ** wsp;
	size_t * size;
        
#endif
{
	void* from;
        X2C_Coroutine p = (X2C_Coroutine)*wsp;
	if (*size < sizeof(struct X2C_Coroutine_STR)) X2C_TRAP_F(X2C_coException);
	*size = *size - sizeof(struct X2C_Coroutine_STR);
	*wsp = (char *)*wsp + sizeof(struct X2C_Coroutine_STR);
	Tie_Coroutine(p);
	if (X2C_setjmp(p->buf)) Coroutine_start();
#if defined(X2C_mac_os) || defined(__mips) || defined(_M_PPC)
        from = (void*)(*(int*)((char *)(p->buf) + sizeof(int)*sp_offs));
	memcpy((void *)frame_buf,from,FRAME_SIZE);
#endif
	return p;
}

#ifndef X2C_KRC
static void X2C_PROCLASS Ini_Aligment (void ** wsp, size_t * size)
#else
static void X2C_PROCLASS Ini_Aligment (wsp,size)
	void ** wsp;
	size_t * size;
#endif
{
	unsigned inc;
	if (*size < FRAME_ALIGMENT*2) X2C_TRAP_F(X2C_coException);
	inc = (unsigned)*wsp % FRAME_ALIGMENT;
	inc = (FRAME_ALIGMENT-inc) % FRAME_ALIGMENT;
	*wsp = (char *)*wsp + inc;
	*size = *size - inc;
	*size = *size - *size % FRAME_ALIGMENT;
}

#ifndef X2C_KRC
static X2C_ADDRESS Make_Stk_Start(X2C_ADDRESS a, size_t size)
#else
static X2C_ADDRESS Make_Stk_Start(a,size) X2C_ADDRESS a; size_t size;
#endif
{
#if   defined(__TURBOC__) && !defined(_Windows) && !defined(__OS2__) && \
    ( defined(__LARGE__) || defined(__HIGE__) || defined(__COMPACT__) )
	/* some preparations for stack overflow checks */
	unsigned ss;
	ss=FP_SEG(a)+(FP_OFF(a)-size+15)/16;
	a=(X2C_ADDRESS)MK_FP(ss,FP_OFF(a)-(ss-FP_SEG(a))*16);
#endif
	return a;
}

#ifndef X2C_KRC
static unsigned highaddr (void *p)
#else
static unsigned highaddr (p) void *p;
#endif
{
#ifdef _msdos
	if (sizeof (p) <= sizeof (unsigned)) return (0);
	return ((unsigned)(((unsigned long)p)>>(sizeof(unsigned)*8)));
#else
	return 0;
#endif
}

#ifndef X2C_KRC
static void Set_Stack(X2C_Coroutine p, void * wsp, X2C_CARD32 wsp_sz)
#else
static void Set_Stack(p,wsp,wsp_sz)
X2C_Coroutine p; void * wsp; X2C_CARD32 wsp_sz;
#endif
{
	char * stk_start = (char *)wsp;
	unsigned *buf = (unsigned *)p->buf;
	if (wsp_sz < FRAME_SIZE+32) X2C_TRAP_F(X2C_coException);
	if (sp_offs < 0 || ss_offs<0 && highaddr(wsp)) {
		X2C_TRAP_F(X2C_coException);
	}
	else if (stk_up) {
		p->stk_start=stk_start;
		stk_start=p->stk_start+FRAME_SIZE;
		memcpy(p->stk_start,frame_buf,FRAME_SIZE);
	}
	else {
		p->stk_start=Make_Stk_Start(stk_start+(size_t)wsp_sz,(size_t)wsp_sz);
		stk_start=p->stk_start-FRAME_SIZE;
		memcpy(stk_start,frame_buf,FRAME_SIZE);
	}
	p->stk_end=p->stk_start;

#if defined(__alpha)
	p->buf[sp_offs]=(long)stk_start;
#else
	buf[sp_offs]=(unsigned)stk_start;
#ifndef DJGPP 
       	if (ss_offs>=0) buf[ss_offs]=highaddr(stk_start);
#endif
#endif

#if defined(__ZTC__) && __INTSIZE == 4
	/* assign SS = DS; I do not know how
	  to find them under Symantech extender!
	*/
	buf[4] = buf[9];
#endif
}

#ifndef X2C_KRC
void X2C_NEWPROCESS (
	X2C_PROC proc,
	X2C_ADDRESS lspace,
	X2C_CARD32 lsize,
	X2C_PROTECTION prot,
	X2C_Coroutine *_this_prs)
#else
void X2C_NEWPROCESS (proc,lspace,lsize,prot,_this_prs)
	X2C_PROC proc;
	X2C_ADDRESS lspace;
	X2C_CARD32 lsize;
	X2C_PROTECTION prot;
	X2C_Coroutine *_this_prs;
#endif
{
	size_t size = (size_t)lsize;
	void * space = lspace;
	if (!space) X2C_TRAP_F(X2C_invalidLocation);
	*_this_prs = Ini_Coroutine(&space,&size);
	Ini_Aligment(&space,&size);
	(*_this_prs)->prot = prot;
	(*_this_prs)->proc = proc;
	Set_Stack(*_this_prs,space,size);
	if ((*_this_prs)->magic != MAGIC) X2C_TRAP_F(X2C_coException);
}

#ifndef X2C_KRC
static void int_transfer(short ip_no)
#else
static void int_transfer(ip_no) short ip_no;
#endif
{
	int i;
	X2C_Coroutine current;

	current = X2C_GetCurrent();

	current->stk_end=(X2C_ADDRESS)&i;
	if (!X2C_setjmp(current->buf)) {
		SET_STK_LIMIT(IntWaiting[ip_no]->stk_start);
		X2C_longjmp (IntWaiting[ip_no]->buf, 1);
	}
	X2C_GetCurrent()->stk_end=0;
}

static HANDLER(iproc0,0)
static HANDLER(iproc1,1)
static HANDLER(iproc2,2)
static HANDLER(iproc3,3)

#ifndef X2C_KRC
void X2C_TRANSFER (X2C_Coroutine * from, X2C_Coroutine to)
#else
void X2C_TRANSFER (from,to)
X2C_Coroutine * from; X2C_Coroutine to;
#endif
{
	int i;
	*from=X2C_GetCurrent();
	if (*from == to) return;
	if (!to) X2C_TRAP_F (X2C_invalidLocation);
	if ((*from)->magic != MAGIC) X2C_TRAP_F(X2C_coException);
	if (to->magic != MAGIC) X2C_TRAP_F(X2C_coException);
	DIS_INTS();
	(*from)->stk_end=(X2C_ADDRESS)&i;
	if (!X2C_setjmp((*from)->buf)) {
		SET_STK_LIMIT(to->stk_start);
		X2C_SetCurrent(to);
		X2C_longjmp (to->buf, 1);
	}
	X2C_GetCurrent()->stk_end=0;
	if (!X2C_GetCurrent()->prot) EN_INTS();
}

#ifndef X2C_KRC
void X2C_IOTRANSFER (X2C_Coroutine * from, X2C_Coroutine to)
#else
void X2C_IOTRANSFER (from,to)
X2C_Coroutine * from; X2C_Coroutine to;
#endif
{
	int vec, ip;

	*from=X2C_GetCurrent();
	vec=(*from)->int_no;
	if (*from == to) X2C_TRAP_F(X2C_coException);
	if (!to) X2C_TRAP_F (X2C_invalidLocation);
	if ((*from)->magic != MAGIC) X2C_TRAP_F(X2C_coException);
	if (to->magic != MAGIC) X2C_TRAP_F(X2C_coException);
	DIS_INTS();
	if (iprocs_cnt == 0) X2C_TRAP_F(X2C_coException);
	vec2ip[vec]=ip=--iprocs_cnt;
	(*from)->stk_end=(X2C_ADDRESS)&vec;
	if (!X2C_setjmp((*from)->buf)) {
		SET_VECTOR(&vec_save[ip],vec,iprocs[ip]);
		IntWaiting[ip]=*from;
		SET_STK_LIMIT(to->stk_start);
		X2C_SetCurrent(to);
		X2C_longjmp(to->buf, 1);
	}
	RESET_VECTOR(vec_save[ip],vec);
	*from=X2C_GetCurrent();
	X2C_SetCurrent(IntWaiting[ip]);
	IntWaiting[ip]=0;
	vec_save[ip]=0;
	vec2ip[vec]=-1;
	++iprocs_cnt;
	X2C_GetCurrent()->stk_end=0;
	if (!X2C_GetCurrent()->prot) EN_INTS();
}

#ifndef X2C_KRC
static void restore_ints(void)
#else
static void restore_ints()
#endif
{
	int i;
	if (sv_timer) 
		RESET_VECTOR(sv_timer,PROFILER_VEC);
	for (i=iprocs_cnt; i<IPS_NO; i++)
		if (vec_save[i])
			RESET_VECTOR(vec_save[i],IntWaiting[i]->int_no);
}


#ifndef X2C_KRC
void X2C_CopyJmpBuf(X2C_Coroutine c)
#else
void X2C_CopyJmpBuf(c) X2C_Coroutine c;
#endif
{
	if ((X2C_REGSIZE*4)<sizeof(X2C_jmp_buf)) X2C_ASSERT(0x100);
	memcpy(c->reg_dump,c->buf,sizeof(X2C_jmp_buf));
	c->reg_dsize = sizeof(X2C_jmp_buf);
} /* END X2C_CopyJmpBuf */


#ifndef X2C_KRC
void X2C_PROTECT(X2C_PROTECTION * from, X2C_PROTECTION to)
#else
void X2C_PROTECT(from,to) X2C_PROTECTION * from; X2C_PROTECTION to;
#endif
{
	X2C_Coroutine current;

	DIS_INTS();
	*from=(current = X2C_GetCurrent())->prot;
	current->prot=to;
	if (!to) 
		EN_INTS();
}

#ifndef X2C_KRC
static short sp_offs_ok(short ofs, unsigned sp, short no)
#else
static short sp_offs_ok(ofs,sp,no) short ofs; unsigned sp; short no;
#endif
{
	X2C_jmp_buf buf;
	unsigned * adr=(unsigned *)buf+ofs, bfx=(unsigned)buf+1;
	/* be aware: bfx can be a register variable */
	if (!no) return 1;
	X2C_setjmp(buf);
	if (stk_up) {
		bfx+=sizeof(X2C_jmp_buf)-2;
		/* check that  bfx < sp < bfx+250 */
		/* order of statemets is important! */
		if (bfx>=*adr) return 0;
		/* 250 <- hp9000.g++ */
		if (*adr-bfx>=250) return 0;
		/* check that sp is incrementing */
		if (sp && *adr<=sp) return 0;
	}
	else {
		/* check that  bfx-200 < sp < bfx */
		/* order of statemets is important! */
		if (bfx<=*adr) return 0;
		if (bfx-*adr>=200) return 0;
		/* check that sp is decrementing */
		if (sp && *adr>=sp) return 0;
	}
	return sp_offs_ok(ofs,*adr,no-1);
}

#ifndef X2C_KRC
static short ss_offs_ok(short ofs, unsigned ss, short no)
#else
static short ss_offs_ok(ofs,ss,no) short ofs; unsigned ss; short no;
#endif
{
	X2C_jmp_buf buf;
	unsigned * adr=(unsigned *)buf+ofs;
	register unsigned i,j,k=no;   /* try to occupy SI and DI */
	char * p=(char *)X2C_GetCurrent(); /* try to occupy ES */
	if (!k) return 1;
	j=k+(*p&0xF);
	ss++; /* ss can be a register variable */
	X2C_setjmp(buf);
	i=j-(*p&0xF);
	if (no&1) X2C_setjmp(buf);
	ss--;
	if (!ss || *adr!=ss) return 0;
	return ss_offs_ok(ofs,ss,i-1);
}

#ifndef X2C_KRC
static void search_fault(short n)
#else
static void search_fault(n) short n;
#endif
{
	printf("#RTS: Coroutines initialization fault %d...\n",n);
	exit(0); /* do not use X2C-exit - it is useless */
}

#ifndef X2C_KRC
static void search_ss_sp(unsigned ss)
#else
static void search_ss_sp(ss) unsigned ss;
#endif
{
	unsigned i;

	ss=highaddr(&i);
	for(i=0; i<sizeof(X2C_jmp_buf)/sizeof(unsigned); i++) {
		if (sp_offs_ok(i,0,8)) {
			if (sp_offs>=0) search_fault(1);
			sp_offs=i;
		}
		if (ss_offs_ok(i,ss,8)) {
			if (ss_offs>=0) search_fault(2);
			ss_offs=i;
		}
	}
}

#ifndef X2C_KRC
static void init_coroutines(X2C_ADDRESS stk_start)
#else
static void init_coroutines(stk_start) X2C_ADDRESS stk_start;
#endif
{
	static struct X2C_Coroutine_STR Current_str;
	unsigned ss=highaddr(&ss);
	short i;
	X2C_Coroutine current;

	current = &Current_str;

	X2C_SetCurrent(Tie_Coroutine(current));
	GET_STK_LIMIT(stk_start);
	current->stk_start=stk_start;
	current->stk_end=0;
	memset(frame_buf,0,FRAME_SIZE);
	memset(vec_save,0,sizeof(VEC_SAVE));
	for (i=0; i<VEC_NO; vec2ip[i++]=-1) {}
	iprocs[0]=iproc0;
	iprocs[1]=iproc1;
	iprocs[2]=iproc2;
	iprocs[3]=iproc3;
	iprocs_cnt=4;

#if defined(__linux__) && defined(__i386__) && defined(X2C_native_library)
        stk_up  = 0;
        ss_offs = -1;
        sp_offs = (int) &((*(X2C_jmp_buf*)(0))[0].esp) / 4;
#elif defined(DJGPP)
        stk_up  = 0;
        ss_offs = -1;
        sp_offs = X2C_FIELD_OFS(X2C_jmp_buf,__esp) / 4;
#elif defined(_M_PPC)
	stk_up  = 0;
	sp_offs = (sizeof(double)*18) / sizeof(unsigned);
	ss_offs = -1;
#elif defined(_Windows) || defined(_WINDOWS)
	stk_up  = 0;
	ss_offs = 8;
	sp_offs = 2;
#elif defined (X2C_mac_os) && !defined(powerc)
	stk_up  = 0;
	sp_offs = (sizeof(long)*11) / sizeof(unsigned);
	ss_offs = -1;
#elif defined(__alpha)
	stk_up  = 0;
	sp_offs = 34;
	ss_offs = -1;
#else
	stk_up=0;
	sp_offs = -1;
	ss_offs = -1;
	search_ss_sp(ss);
	if (sp_offs<0 || ss && ss_offs<0) {
		stk_up=1;
		search_ss_sp(ss);
	}
	if (sp_offs<0 || ss && ss_offs<0) search_fault(3);
#endif
}

#ifdef X2C_no_spawn

#include <xPOSIX.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef X2C_KRC
int spawnv(int mode, char *path, X2C_ppcCHAR args)
#else
int spawnv(mode,path,args) int mode; char *path; X2C_ppcCHAR args;
#endif
{
	pid_t pid;
	int stat;
	if (mode==P_OVERLAY) return execv(path,(char**)args);
	pid=fork();
	if (pid<0) return -1;
	if (!pid) { /* child */
		execv(path,(char**)args);
		exit(153);
	}
	else if (mode==P_WAIT) { /* parent */
		pid=waitpid(pid,&stat,0);
		if (WIFEXITED(stat)) return WEXITSTATUS(stat);
		return -1;
	}
	else return 0;
}

#endif

#ifndef X2C_KRC
extern void X2C_Profiler(void)
#else
extern void X2C_Profiler()
#endif
{
#if PROFILER_VEC >=0
	if (vec2ip[PROFILER_VEC]<0 && !sv_timer)
		SET_VECTOR(&sv_timer,PROFILER_VEC,profiler_proc);
#endif
}

#ifndef X2C_KRC
static void fp_error(int sig)
#else
static void fp_error(sig) int sig;
#endif
// floating point error signal 
{
/*
#ifdef _linux

  // Linux dependancies here :   -- Den 14-Dec-95. 

       long status, control, raised_exceptions;
	__asm__ ("fnsave %0":"=m" (sigfpe_fpu_state));
	status = sigfpe_fpu_state.fcs & 0x0000ffff;
	control = sigfpe_fpu_state.cwd;
	__asm__ ("frstor %0":"=m" (sigfpe_fpu_state));
	raised_exceptions = status & 0x3f & ~control;
	if    ( raised_exceptions & 1 )              X2C_TRAP_F(X2C_realValueException);
	else if ( raised_exceptions & 2 )            X2C_TRAP_F(X2C_realValueException);
	else if ( raised_exceptions & 4 )            X2C_TRAP_F(X2C_realDivException);
	else if ( raised_exceptions & 8 )            X2C_TRAP_F(X2C_realValueException);
	else if ( raised_exceptions & 16 )           X2C_TRAP_F(X2C_realValueException);
	else if ( raised_exceptions & 32 )           X2C_TRAP_F(X2C_realValueException);
	else                                         X2C_TRAP_F(X2C_wholeDivException);
#else
    // !!!! I am not sure about that 
	sig=sig;
	signal(SIGFPE,fp_error);
	X2C_TRAP_F(X2C_realValueException);
#endif
*/
}

#ifdef _linux

  void X2C_SIGSEGV_handler(int sig, struct sigcontext_struct cntxt)
  {
    int i;
    signal(SIGSEGV,X2C_SIGSEGV_handler);

//    if(cntxt.trapno==4) X2C_TRAP_F(X2C_wholeValueException);
//    else X2C_TRAP_F(X2C_invalidLocation);

    X2C_TRAP_F(X2C_invalidLocation);
  }

#endif

#if ! defined __ZTC__ & ! defined __IBMC__
#ifndef X2C_KRC
extern long labs(long x)
#else
extern long labs(x) long x;
#endif
{
	return (x<0) ? -x : x;
}
#endif

/* --------------------- initialization ---------------------- */
/*                       --------------                        */

int     X2C_argc = 0;
char ** X2C_argv = 0;

#if defined(_WIN32) || defined (__NT__) || defined(__OS2__)

#define MAXEXEPATH 512
char ExePath[MAXEXEPATH];

#ifndef X2C_KRC
static void argv_0_Patch(void)
#else
static void argv_0_Patch()
#endif
{
#if defined(_WIN32) || defined (__NT__)
    if (GetModuleFileName(GetModuleHandle(0),ExePath,MAXEXEPATH) == 0)
	return;
#else /* defined (__OS2__) */
    unsigned long *ptib, *ppib;
    DosGetInfoBlocks(&ptib,&ppib);
    if (DosQueryModuleName(*(ppib+2),MAXEXEPATH,ExePath))
	return;
#endif
    X2C_argv[0] = ExePath;
}
#else /* not Win32 or OS/2 */
#define argv_0_Patch()
#endif

void X2C_PROCLASS X2C_BEGIN
#ifndef X2C_KRC
			(int *argc, char **argv,
			int gc_auto, long gc_threshold,
			long heap_limit)
#else
			(argc,argv,gc_auto,gc_threshold,heap_limit)
	int *argc;
	char **argv;
	int gc_auto;
	long gc_threshold;
	long heap_limit;
#endif
{
  if (sizeof(X2C_INT8)!=1 || sizeof(X2C_INT16)!=2 || sizeof(X2C_INT32)!=4) {
    printf("#RTS: Error in compiler options...\n");
    printf("#RTS: Wrong sizes of base types.\n");
    exit(0);
  }
  X2C_argc = *argc;
  X2C_argv = argv;
  argv_0_Patch();
  X2C_fs_init = (X2C_BOOLEAN)0;
  X2C_INIT_TESTCOVERAGE ();
  X2C_INIT_RTS();
  init_coroutines((X2C_ADDRESS)argc); /* must be before X2C_GC_INIT */
  X2C_GC_INIT(gc_auto,gc_threshold,heap_limit);
  X2C_init_exceptions();
  X2C_atexit(restore_ints); /* must be before X2C_ini_termination */
  X2C_ini_termination();
#ifdef __OS2__
  /*
   * Under OS/2, the underflow exception is not masked by default,
   * so it is masked here.
   */
  _control87(EM_UNDERFLOW,EM_UNDERFLOW);
#endif
  signal(SIGFPE,fp_error);
#ifdef _linux
  signal(SIGSEGV,X2C_SIGSEGV_handler);
#endif
}

void X2C_MEMSET(void *p, char c, size_t n) {
    memset(p, c, n); 
}
