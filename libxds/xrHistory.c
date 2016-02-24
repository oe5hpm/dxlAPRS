/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrHistory.c Feb  3 14:31:18 2012" */
#include "xrHistory.h"
#define xrHistory_C_
#include "xmRTS.h"
#include "xPOSIX.h"

#define tag_first 0

#define tag_second 1

static X2C_Profile profile;

static X2C_INT32 total_time;


extern void X2C_show_history(void)
{
   int i;
   X2C_Coroutine current;
   struct X2C_Coroutine_STR * anonym;
   current = X2C_GetCurrent();
   { /* with */
      struct X2C_Coroutine_STR * anonym = current;
      if (anonym->his_cnt>0 && anonym->his_cnt<=254) {
         anonym->his[anonym->his_cnt-1].fln = (X2C_INT16)X2C_hline;
      }
      if (anonym->his_cnt>255) {
         printf("History stack was truncated - not enough room.\n");
         anonym->his_cnt = 256;
      }
      while (anonym->his_cnt>0) {
         --anonym->his_cnt;
         i = (int)anonym->his[anonym->his_cnt].fln;
         if (anonym->his[anonym->his_cnt].fnm==0) printf("???\n");
         else printf("%-18.18s %4d\n", anonym->his[anonym->his_cnt].fnm, i);
      }
   }
} /* end X2C_show_history() */


extern void X2C_PROC_INP_F(X2C_CHAR file[], X2C_INT32 line)
{
   X2C_Coroutine current;
   struct X2C_Coroutine_STR * anonym;
   current = X2C_GetCurrent();
   { /* with */
      struct X2C_Coroutine_STR * anonym = current;
      if (anonym->his_cnt>0 && anonym->his_cnt<=256) {
         anonym->his[anonym->his_cnt-1].fln = (X2C_INT16)X2C_hline;
      }
      if (anonym->his_cnt<=255) {
         anonym->his[anonym->his_cnt].fnm = (X2C_pCHAR)file;
         anonym->his[anonym->his_cnt].prf = 0;
         anonym->his[anonym->his_cnt].fln = (X2C_INT16)line;
         anonym->his[anonym->his_cnt].tags = 0U;
         X2C_hline = line;
      }
      ++anonym->his_cnt;
   }
} /* end X2C_PROC_INP_F() */


extern void X2C_PROC_PRF_F(X2C_CHAR file[], X2C_INT32 line,
                struct X2C_Profile_STR * p)
{
   X2C_Coroutine current;
   struct X2C_Coroutine_STR * anonym;
   struct X2C_his_rec * anonym0;
   current = X2C_GetCurrent();
   { /* with */
      struct X2C_Coroutine_STR * anonym = current;
      if (anonym->his_cnt>0 && anonym->his_cnt<=256) {
         anonym->his[anonym->his_cnt-1].fln = (X2C_INT16)X2C_hline;
      }
      if (anonym->his_cnt<=255) {
         { /* with */
            struct X2C_his_rec * anonym0 = &anonym->his[anonym->his_cnt];
            anonym0->fnm = (X2C_pCHAR)file;
            anonym0->prf = p;
            anonym0->fln = (X2C_INT16)line;
            anonym0->tags = 0U;
            if ((0x1U & p->tags)) anonym0->tags |= 0x2U;
            else p->tags |= 0x1U;
         }
         X2C_hline = line;
      }
      ++anonym->his_cnt;
   }
   if (p->count==0L) {
      if (profile==0) X2C_Profiler();
      p->next = profile;
      profile = p;
   }
   ++p->count;
} /* end X2C_PROC_PRF_F() */


extern void X2C_Profiler_clock(void)
{
   X2C_INT32 i;
   X2C_Coroutine current;
   struct X2C_Coroutine_STR * anonym;
   struct X2C_his_rec * anonym0;
   current = X2C_GetCurrent();
   { /* with */
      struct X2C_Coroutine_STR * anonym = current;
      i = 0L;
      while (i<(X2C_INT32)anonym->his_cnt && i<=255L) {
         { /* with */
            struct X2C_his_rec * anonym0 = &anonym->his[i];
            if (anonym0->prf && (0x2U & anonym0->tags)==0) {
               ++anonym0->prf->total;
            }
         }
         ++i;
      }
      if (anonym->his_cnt<=256 && anonym->his[anonym->his_cnt-1].prf) {
         ++anonym->his[anonym->his_cnt-1].prf->time0;
      }
   }
   ++total_time;
} /* end X2C_Profiler_clock() */


extern void X2C_PROC_OUT_F(void)
{
   X2C_Coroutine current;
   struct X2C_Coroutine_STR * anonym;
   struct X2C_his_rec * anonym0;
   current = X2C_GetCurrent();
   { /* with */
      struct X2C_Coroutine_STR * anonym = current;
      --anonym->his_cnt;
      if (anonym->his_cnt<=255) {
         { /* with */
            struct X2C_his_rec * anonym0 = &anonym->his[anonym->his_cnt];
            if (anonym0->prf && (0x2U & anonym0->tags)==0) {
               anonym0->prf->tags &= ~0x1U;
            }
         }
      }
      if (anonym->his_cnt>0 && anonym->his_cnt<=254) {
         X2C_hline = (X2C_INT32)anonym->his[anonym->his_cnt-1].fln;
      }
   }
} /* end X2C_PROC_OUT_F() */


extern void X2C_HIS_SAVE(X2C_INT16 * sv)
{
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   *sv = current->his_cnt;
} /* end X2C_HIS_SAVE() */


extern void X2C_HIS_RESTORE(X2C_INT16 sv)
{
   X2C_Coroutine current;
   struct X2C_Coroutine_STR * anonym;
   current = X2C_GetCurrent();
   { /* with */
      struct X2C_Coroutine_STR * anonym = current;
      if (anonym->his_cnt>0 && anonym->his_cnt<=254) {
         anonym->his[anonym->his_cnt-1].fln = (X2C_INT16)X2C_hline;
      }
      anonym->his_cnt = sv;
      if (anonym->his_cnt>0 && anonym->his_cnt<=254) {
         X2C_hline = (X2C_INT32)anonym->his[anonym->his_cnt-1].fln;
      }
   }
} /* end X2C_HIS_RESTORE() */


extern void X2C_show_profile(void)
{
   X2C_Profile r;
   X2C_Profile q;
   X2C_Profile s;
   X2C_Profile prf;
   X2C_REAL sum;
   prf = profile;
   if (prf) {
      sum = (X2C_REAL)total_time;
      printf("Execution profile:\n");
      r = 0;
      while (prf) {
         s = prf;
         prf = prf->next;
         if (r==0) {
            r = s;
            s->next = 0;
         }
         else if (r->total<s->total) {
            s->next = r;
            r = s;
         }
         else {
            q = r;
            while (q->next && q->next->total>s->total) q = q->next;
            s->next = q->next;
            q->next = s;
         }
      }
      if (sum==0.0f) sum = 1.0f;
      do {
         printf("%-32.32s %10ld %6.2f %6.2f\n", r->name, r->count,
                (X2C_LONGREAL)(X2C_DIVR((X2C_REAL)r->time0*100.0f,sum)),
                (X2C_LONGREAL)(X2C_DIVR((X2C_REAL)r->total*100.0f,sum)));
         r = r->next;
      } while (r);
   }
} /* end X2C_show_profile() */


extern void X2C_scanStackHistory(X2C_ADDRESS from, X2C_ADDRESS to,
                X2C_BOOLEAN exact)
{
} /* end X2C_scanStackHistory() */

