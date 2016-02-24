/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrcO2MM.c Feb  3 14:31:19 2012" */
#include "xrcO2MM.h"
#define xrcO2MM_C_
#include "xmRTS.h"
#include "M2EXCEPTION.h"
#include "xrMM.h"
#include "X2C.h"

#define AdrLss X2C_adr_lss

#define AdrGtr X2C_adr_gtr

#define LINK_SZ sizeof(struct X2C_LINK_STR)


extern X2C_pVOID X2C_GUARDP_F(X2C_pVOID p, X2C_TD td)
{
   X2C_LINK ln;
   if (p==0) X2C_TRAP_F(3L);
   ln = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)p-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if (ln->_.td->res!=0x093678150UL) X2C_ASSERT_F(102UL);
   if (ln->_.td->base[td->level]!=td) {
      X2C_TRAP_F((X2C_INT32)X2C_guardException);
   }
   return p;
} /* end X2C_GUARDP_F() */


extern X2C_TD X2C_GUARDV_F(X2C_TD vt, X2C_TD td)
{
   if (vt->base[td->level]!=td) X2C_TRAP_F((X2C_INT32)X2C_guardException);
   return vt;
} /* end X2C_GUARDV_F() */


extern X2C_pVOID X2C_GUARDPE_F(X2C_pVOID p, X2C_TD td)
{
   X2C_LINK ln;
   if (p==0) X2C_TRAP_F(3L);
   ln = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)p-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if (ln->_.td!=td) X2C_TRAP_F((X2C_INT32)X2C_guardException);
   return p;
} /* end X2C_GUARDPE_F() */


extern X2C_TD X2C_GUARDVE_F(X2C_TD vt, X2C_TD td)
{
   if (vt!=td) X2C_TRAP_F((X2C_INT32)X2C_guardException);
   return vt;
} /* end X2C_GUARDVE_F() */

