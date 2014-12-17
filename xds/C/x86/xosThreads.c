/* Copyright (c) 1999 XDS Ltd, Russia. All Rights Reserved. */

#include "X2C.h"
#ifdef _unix
#include <unistd.h>
#ifndef _POSIX_THREADS
#error POSIX threads are not supported on your target
#endif
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h> /* for sched_yield() */

/*
TYPE
  X2C_Thread;       (* System dependent thread Id *)
  X2C_BSemaphore;   (* System dependent semaphore Id *)
  X2C_Semaphore;    (* System dependent semaphore Id *)
  X2C_Mutex;
  X2C_THREAD_PROC = PROCEDURE (SYSTEM.ADDRESS);
  X2C_EventSource = SYSTEM.CARD32;
(* Note that MUTEXes may be implemented as a semaphores;
   they defined separately to accomodate light-weight
   syncronisation mechanism of the system (CRITICAL_SECTION in Win32) *)
*/

typedef pthread_t X2C_Thread; /* !!! */

typedef struct {
        pthread_mutex_t mutex;
        pthread_t thread;
        int count;
} X2C_Mutex_struct;

typedef void (*X2C_THREAD_PROC)(X2C_ADDRESS);

typedef X2C_Mutex_struct *X2C_Mutex;

typedef X2C_CARD32 X2C_EventSource;

typedef pthread_key_t  X2C_Key;

pthread_key_t key;

#define xosSCHED_POLICY   SCHED_RR

static void startThread(){
  
}

/*
PROCEDURE X2C_INIT_THREADS(): SYSTEM.INT32;
*/

extern X2C_INT32 X2C_INIT_THREADS(void) {
        struct sched_param sch;
        int                temp;

        if (sysconf(_SC_THREADS) == -1) return ENOSYS;
        if (( temp=pthread_getschedparam( pthread_self(), &temp, &sch)) != 0 ) return temp;
        sch.sched_priority = 1;
        temp = pthread_setschedparam( pthread_self(), xosSCHED_POLICY, &sch );
        if (temp) return temp;
        
        return pthread_key_create(&key,NULL);
}

/*
PROCEDURE X2C_CurrentThread(): X2C_Thread;
*/
extern X2C_Thread X2C_CurrentThread(void) {
        return pthread_self();
}

/*
PROCEDURE X2C_MyThreadHandle(VAR h: X2C_Thread): SYSTEM.INT32;
*/

extern X2C_INT32 X2C_MyThreadHandle( X2C_Thread * threadId) {
        *threadId = pthread_self();
        return 0;
}

/*
PROCEDURE X2C_GetThreadWord(VAR w: SYSTEM.ADDRESS): SYSTEM.INT32;
*/

extern X2C_INT32 X2C_GetThreadWord(X2C_ADDRESS *w) {
        *w = (X2C_ADDRESS)pthread_getspecific(key);
        return 0;
}


/*
PROCEDURE X2C_SetThreadWord(w: SYSTEM.ADDRESS): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_SetThreadWord(X2C_ADDRESS w) {
        return (X2C_INT32)(pthread_setspecific(key, (void*)w));
}

/*
PROCEDURE X2C_GetMyThreadId(VAR id: SYSTEM.CARD32);
*/
extern void X2C_GetMyThreadId(X2C_CARD32 *id) {
        id = 0;
}

/*
PROCEDURE X2C_CreateThread(VAR t: X2C_Thread;
                            proc: X2C_THREAD_PROC;
                           stack: SYSTEM.CARD32;
                           param: SYSTEM.ADDRESS;
                            prio: SYSTEM.INT32): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_CreateThread(X2C_Thread *t,
                                  X2C_THREAD_PROC proc,
                                  X2C_CARD32 stack,
                                  X2C_ADDRESS param,
                                  X2C_INT32 prio) {
         pthread_attr_t attr;
         int err;
         struct sched_param sch;
         int temp;
        
         pthread_attr_init (  &attr );
         err=pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
         if (err) return err;
         pthread_attr_setstacksize ( &attr, stack );

         /* SCHED_RR - round robin; for superuser only*/
         err=pthread_attr_setschedpolicy( &attr, xosSCHED_POLICY );
         if (err) return err;    

         /* set priority */
         if (prio < 1) prio = 1;
         if (prio >99) prio = 99;
         sch.sched_priority = prio;
         err=pthread_attr_setschedparam( &attr, &sch);
         if (err) return err;

         err = pthread_create ( t, &attr, proc, param );
         pthread_attr_destroy( &attr );
         return err;
}
/*
PROCEDURE ["C"] X2C_GetThreadPriority(t: X2C_Thread; VAR prio: SYSTEM.INT32): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_GetThreadPriority(X2C_Thread t, 
                                       X2C_INT32 *prio){
                                       
        struct sched_param p;
        int temp,err;
         
        err  = pthread_getschedparam( t, &temp, &p );   

        if (err) return err;
        *prio = p.sched_priority;
        return 0;
}
/*
PROCEDURE ["C"] X2C_SetThreadPriority(t: X2C_Thread; prio: SYSTEM.INT32): SYSTEM.INT32;
*/

extern X2C_INT32 X2C_SetThreadPriority(X2C_Thread t,
                                       X2C_INT32  prio){
        struct sched_param sch;
        int                temp;

        if (prio < 1) prio = 1;
        if (prio >99) prio = 99;
        if (( temp=pthread_getschedparam( t, &temp, &sch)) != 0 ) return temp;
        sch.sched_priority = prio;
        return pthread_setschedparam( t, xosSCHED_POLICY, &sch );
}
/*
PROCEDURE X2C_DeleteThread(VAR t: X2C_Thread): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_DeleteThread(X2C_Thread *t) {
        return pthread_cancel(*t);
} 

/*
PROCEDURE X2C_SuspendThread(t: X2C_Thread): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_SuspendThread(X2C_Thread t) {
        return pthread_kill( t, SIGSTOP );
}


/*
PROCEDURE X2C_ScheduleMyThread();
*/
extern void X2C_ScheduleMyThread(void) {
        sched_yield(); // We use sched_yield() function to maintain using of
                       // the POSIX interface, but it won't work in m-on-n
                       // implementation of the pthread library. In that case,
                       // we should use the GNU pthread_yield() function.
}


/*
PROCEDURE X2C_ResumeThread (t: X2C_Thread): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_ResumeThread(X2C_Thread t) {
        return pthread_kill( t, SIGCONT );
//      return ENOSYS;
}


/*
PROCEDURE X2C_ExitThread();
*/
extern void X2C_ExitThread(void) {
        pthread_exit(NULL);
}

/*
PROCEDURE X2C_CreateMutex(VAR m: X2C_Mutex): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_CreateMutex(X2C_Mutex *m) {
        if ((*m = malloc(sizeof(X2C_Mutex_struct))) == NULL) return ENOMEM;
        pthread_mutex_init(&(*m)->mutex, NULL);
/*      m->thread is left uninitialized */
        (*m)->count = 0;
        return 0;
}

/*
PROCEDURE X2C_DeleteMutex(VAR m: X2C_Mutex): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_DeleteMutex(X2C_Mutex *m) {
        int err;

        if ( *m == NULL ) return EINVAL;
        err = pthread_mutex_destroy(&(*m)->mutex);
        *m = NULL;
        free(*m);
        return err;
}

/*
PROCEDURE X2C_EnterMutex(m: X2C_Mutex): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_EnterMutex(X2C_Mutex m) {
        int err;
        pthread_t self;

        if ( m == NULL ) return EINVAL;
        self = pthread_self();
        if ((m->count > 0) && (pthread_equal(self,m->thread))) {
                m->count++;
                return 0;
        }
        if ((err=pthread_mutex_lock(&m->mutex)) != 0) return err;
        m->thread = self;
        m->count++;
        return 0;       
}

/*
PROCEDURE X2C_ExitMutex(m: X2C_Mutex): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_ExitMutex(X2C_Mutex m) {
        pthread_t self;

        if ( m == NULL ) return EINVAL;
        self = pthread_self();
        if ((m->count > 0) && pthread_equal(self,m->thread)) 
                return --m->count ? 0 : pthread_mutex_unlock(&m->mutex);
        else
                return EPERM; /* not owner or not locked */
}

typedef struct 
{
        unsigned int state;           // State of the semaphore.
        pthread_mutex_t lock;         // Serializes access to <state>
        pthread_cond_t signaled;      // Blocks if state == 0 (nonsignaled)
} X2C_BSemaphore_struct;

typedef X2C_BSemaphore_struct *X2C_BSemaphore;


typedef struct 
{
        unsigned int count;           // Current count of the semaphore.
        unsigned int max;             // Maximum count of the semaphore.
        unsigned long waiters;        // Number of threads that are waiting.  
        pthread_mutex_t lock;         // Serializes access to <count> and <waiters>.
        pthread_cond_t count_nonzero; // Blocks when <count> == 0.
} X2C_Semaphore_struct;

typedef X2C_Semaphore_struct *X2C_Semaphore;

/*
PROCEDURE X2C_CreateBoolSemaphore  (VAR m: X2C_BSemaphore) :SYSTEM.INT32;
*/
extern X2C_INT32 X2C_CreateBoolSemaphore(X2C_BSemaphore *m) {
        X2C_BSemaphore s = malloc(sizeof(X2C_BSemaphore_struct));
        pthread_mutex_init (&s->lock, NULL);
        pthread_cond_init (&s->signaled, NULL);
        s->state = 0;
        *m = s;
        return 0;       
}

/*
PROCEDURE X2C_DeleteBoolSemaphore  (VAR m: X2C_BSemaphore) :SYSTEM.INT32;
*/
extern X2C_INT32 X2C_DeleteBoolSemaphore(X2C_BSemaphore *m) {
        int err;

        if ( *m == NULL ) return EINVAL;
        pthread_mutex_destroy(&((*m)->lock));
        if ((err=pthread_cond_destroy(&((*m)->signaled))) != 0 ) return err;
        *m = NULL;
        free(*m);
        return 0;       
}

/*
PROCEDURE X2C_AcquireBoolSemaphore (m :X2C_BSemaphore) :SYSTEM.INT32;
*/
extern X2C_INT32 X2C_AcquireBoolSemaphore(X2C_BSemaphore m) {
        int err;

        if ( m == NULL ) return EINVAL;
        if ((err=pthread_mutex_lock (&m->lock)) != 0 ) return err;
        while (m->state == 0)
                pthread_cond_wait (&m->signaled, &m->lock);
        return pthread_mutex_unlock (&m->lock);
}


/*
PROCEDURE X2C_ResetBoolSemaphore (m: X2C_BSemaphore) :SYSTEM.INT32;
(* set in blocked state *) 
*/
extern X2C_INT32 X2C_ResetBoolSemaphore(X2C_BSemaphore m) {
        int err;      
        
        if ( m == NULL ) return EINVAL;
        if ((err=pthread_mutex_lock (&m->lock)) != 0 ) return err;
        m->state = 0;
        return pthread_mutex_unlock (&m->lock);
}

/*
PROCEDURE X2C_SetBoolSemaphore (m: X2C_BSemaphore) :SYSTEM.INT32;
(* set in signaled state *) 
*/
extern X2C_INT32 X2C_SetBoolSemaphore(X2C_BSemaphore m) {
        int err;      
  
        if ( m == NULL ) return EINVAL;
        if ((err=pthread_mutex_lock (&m->lock)) != 0 ) return err;
        pthread_cond_broadcast (&m->signaled);
        m->state = 1;
        return pthread_mutex_unlock (&m->lock);
}

/*
PROCEDURE X2C_CreateSemaphore(VAR m: X2C_Semaphore; init, max: SYSTEM.INT32): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_CreateSemaphore(X2C_Semaphore *m, X2C_INT32 init, X2C_INT32 max) {
        X2C_Semaphore s = malloc(sizeof(X2C_Semaphore_struct));
        pthread_mutex_init (&s->lock, NULL);
        pthread_cond_init (&s->count_nonzero, NULL);
        s->count   = init;
        s->max     = max;
        s->waiters = 0;
        *m = s;
        return 0;       
}

/*
PROCEDURE X2C_DeleteSemaphore(VAR m: X2C_Semaphore): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_DeleteSemaphore(X2C_Semaphore *m) {
        int err;
        
        if ( *m == NULL ) return EINVAL;
        pthread_mutex_destroy(&((*m)->lock));
        if ((err = pthread_cond_destroy(&((*m)->count_nonzero))) != 0 ) return err;
        *m = NULL;
        free(*m);
        return 0;
}

/*
PROCEDURE X2C_AcquireSemaphore(m: X2C_Semaphore; nowait: BOOLEAN; VAR awaited: BOOLEAN): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_AcquireSemaphore(X2C_Semaphore m, X2C_BOOLEAN nowait, X2C_BOOLEAN *awaited) {
        int err;
        
        if ( m == NULL ) return EINVAL;
        if ((err=pthread_mutex_lock (&m->lock)) != 0 ) return err;

        if (nowait) {
            *awaited = m->count == 0;
            if (m->count > 0) (m->count)--;
        }else{
                m->waiters++;   /* Increase the number of waiters for Release to work correctly */

                /* Wait until the semaphore count is > 0, then atomically release
                   <lock> and wait for <count_nonzero> to be signaled. */
                while (m->count == 0)
                        pthread_cond_wait (&m->count_nonzero, &m->lock);
                /* <m->lock> is now held again */

                m->waiters--;   /* Decrement the waiters count */
                m->count--;     /* Decrement the semaphore's count */
        }
        return pthread_mutex_unlock (&m->lock);
}

/*
PROCEDURE X2C_ReleaseSemaphore(m: X2C_Semaphore): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_ReleaseSemaphore(X2C_Semaphore m) {
        int err;

        if ( m == NULL ) return EINVAL;
        if ((err=pthread_mutex_lock (&m->lock)) != 0 ) return err;

        /* Always allow one thread to continue if it is waiting. */
        if (m->waiters > 0)
                pthread_cond_signal (&m->count_nonzero);

        /* Increment the semaphore's count. */
        if (m->count < m->max) m->count++;

        return pthread_mutex_unlock (&m->lock);
}

/*
PROCEDURE X2C_SemaphoreToEvent(m: X2C_BSemaphore): X2C_EventSource;
*/
extern X2C_EventSource X2C_SemaphoreToEvent(X2C_BSemaphore m) {
      return (X2C_EventSource)m;
}

/*
PROCEDURE X2C_WaitEvents(s: ARRAY OF X2C_EventSource; no: CARDINAL): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_WaitEvents(X2C_EventSource *s, X2C_CARD32 no) {
        return ENOSYS;
}

/*
PROCEDURE X2C_SyncThreadState(thr: X2C_Thread;
                      VAR stk_end: SYSTEM.ADDRESS;
                         reg_dump: SYSTEM.ADDRESS;
                         dmp_size: SYSTEM.CARD32;
                    VAR reg_dsize: SYSTEM.CARD32): SYSTEM.INT32;
*/
extern X2C_INT32 X2C_SyncThreadState(X2C_Thread thr,
                                     X2C_ADDRESS *stk_end,
                                     X2C_ADDRESS reg_dump,
                                     X2C_CARD32 dmp_size,
                                     X2C_CARD32 *reg_dsize) {
        return ENOSYS;
}

extern X2C_INT32 X2C_CreateKey( X2C_Key *key){
//      if ((*key = malloc(sizeof(X2C_Key))) == NULL) return ENOMEM;
        return pthread_key_create( key, NULL);
}

extern X2C_INT32 X2C_DeleteKey( X2C_Key key){
        return pthread_key_delete( key );
}

extern X2C_INT32 X2C_SetKeyValue( X2C_Key key, X2C_ADDRESS value){
        return pthread_setspecific( key, value );
}

extern X2C_INT32 X2C_GetKeyValue( X2C_Key key, X2C_ADDRESS *value){
        *value = pthread_getspecific( key );
        return 0;
}


#else /* _unix */
#error xosThreads.c is only implemented on UNIX */
#endif /* _unix */
