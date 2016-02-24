/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xosExec.c Feb  3 14:31:18 2012" */
#include "xosExec.h"
#define xosExec_C_
#include "xmRTS.h"
#include "xPOSIX.h"
#include "xlibOS.h"
#include "X2C.h"
#include "xrtsOS.h"


static void put_in_quotes(X2C_pCHAR to, X2C_pCHAR a)
{
   *to = '\"';
   to = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)to+(X2C_INT32)1UL)
                ;
   while (*a) {
      *to = *a;
      to = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)to+(X2C_INT32)
                1UL);
      a = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)a+(X2C_INT32)
                1UL);
   }
   *to = '\"';
   to = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)to+(X2C_INT32)1UL)
                ;
   *to = 0;
} /* end put_in_quotes() */


extern X2C_INT32 X2C_Execute(X2C_pCHAR cmd, X2C_pCHAR args, int overlay,
                X2C_CARD32 * exitcode)
{
   xPOSIX_PCHAR argv[256];
   X2C_INT32 i;
   X2C_INT32 rc;
   X2C_CARD32 bsz;
   i = 1L;
   for (;;) {
      while ((X2C_CARD8)(*args)<=' ') {
         if (*args==0) goto loop_exit;
         args = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                args+(X2C_INT32)1UL);
      }
      argv[i] = (xPOSIX_PCHAR)args;
      ++i;
      while ((X2C_CARD8)(*args)>' ') {
         args = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                args+(X2C_INT32)1UL);
      }
      if (*args==0) break;
      *args = 0;
      args = (X2C_pCHAR)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                args+(X2C_INT32)1UL);
   }
   loop_exit:;
   argv[i] = 0;
   bsz = strlen(cmd)+2U+1U;
   argv[0U] = (xPOSIX_PCHAR)X2C_malloc(bsz);
   put_in_quotes((X2C_pCHAR)argv[0U], cmd);
   #if defined(__WATCOMC__)
   cmd = (X2C_pCHAR)argv[0U];
   #endif
   if (overlay) {
      #if defined(_msdos)
      rc = spawnv(P_WAIT, cmd, argv);
      #else
      rc = execv(cmd, argv);
      #endif
   }
   else rc = spawnv(P_WAIT, cmd, argv);
   X2C_free((X2C_ADDRESS)argv[0U], bsz);
   if (rc<0L) return errno;
   else {
      *exitcode = (X2C_CARD32)rc;
      return 0L;
   }
   return 0;
} /* end X2C_Execute() */


extern X2C_INT32 X2C_ExecuteNoWindow(X2C_pCHAR cmd, X2C_pCHAR args,
                int overlay, X2C_CARD32 * exitcode)
{
   return X2C_Execute(cmd, args, overlay, exitcode);
} /* end X2C_ExecuteNoWindow() */


extern X2C_INT32 X2C_Command(X2C_pCHAR cmd, X2C_CARD32 * exitcode)
{
   X2C_INT32 rc;
   rc = system(cmd);
   if (rc<0L) return errno;
   else {
      *exitcode = (X2C_CARD32)rc;
      return 0L;
   }
   return 0;
} /* end X2C_Command() */

