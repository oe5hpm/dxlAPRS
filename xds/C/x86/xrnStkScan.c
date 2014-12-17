/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrnStkScan.c Feb  3 14:30:28 2012" */
#include "xrnStkScan.h"
#define xrnStkScan_C_
#include "xmRTS.h"

extern X2C_CARD32 X2C_IS_CALL(X2C_CARD32);


extern void X2C_STACK_SCAN_PROC(X2C_ADDRESS from, X2C_ADDRESS to,
                X2C_BOOLEAN exact)
{
   X2C_CARD32 cur;
   X2C_ADDRESS frame;
   X2C_BOOLEAN sync;
   X2C_Coroutine current;
   cur = 0UL;
   current = X2C_GetCurrent();
   current->his_cnt = 0;
   frame = *(X2C_ADDRESS *)(X2C_ADDRESS)((X2C_ADDRESS)from-(X2C_INT32)4UL);
   sync = 0;
   if (exact) {
      current->his[0U] = *(X2C_CARD32 *)from;
      current->his_cnt = 1;
      from = (X2C_ADDRESS)((X2C_ADDRESS)from+(X2C_INT32)4UL);
   }
   while (*X2C_CAST(&from,X2C_ADDRESS,X2C_CARD32,X2C_CARD32 *)<*X2C_CAST(&to,
                X2C_ADDRESS,X2C_CARD32,X2C_CARD32 *)) {
      if (sync) {
         sync = 0;
         if (*X2C_CAST(&frame,X2C_ADDRESS,X2C_CARD32,
                X2C_CARD32 *)>*X2C_CAST(&from,X2C_ADDRESS,X2C_CARD32,
                X2C_CARD32 *) && *X2C_CAST(&frame,X2C_ADDRESS,X2C_CARD32,
                X2C_CARD32 *)<=*X2C_CAST(&to,X2C_ADDRESS,X2C_CARD32,
                X2C_CARD32 *)) {
            cur = *(X2C_CARD32 *)(X2C_ADDRESS)((X2C_ADDRESS)frame+(X2C_INT32)
                4UL);
            cur = X2C_IS_CALL(cur);
            if (cur) {
               from = (X2C_ADDRESS)((X2C_ADDRESS)frame+(X2C_INT32)8UL);
               frame = *(X2C_ADDRESS *)frame;
               sync = 1;
            }
         }
      }
      if (!sync) {
         for (;;) {
            cur = *(X2C_CARD32 *)from;
            cur = X2C_IS_CALL(cur);
            if (cur) break;
            from = (X2C_ADDRESS)((X2C_ADDRESS)from+(X2C_INT32)4UL);
            if (*X2C_CAST(&from,X2C_ADDRESS,X2C_CARD32,
                X2C_CARD32 *)>=*X2C_CAST(&to,X2C_ADDRESS,X2C_CARD32,
                X2C_CARD32 *)) break;
         }
         if (cur) {
            frame = *(X2C_ADDRESS *)(X2C_ADDRESS)((X2C_ADDRESS)
                from-(X2C_INT32)4UL);
            sync = 1;
            from = (X2C_ADDRESS)((X2C_ADDRESS)from+(X2C_INT32)4UL);
         }
      }
      if (cur) {
         if (current->his_cnt<256) current->his[current->his_cnt] = cur;
         ++current->his_cnt;
      }
   }
} /* end X2C_STACK_SCAN_PROC() */

