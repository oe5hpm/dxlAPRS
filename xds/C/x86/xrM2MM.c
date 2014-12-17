/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrM2MM.c Feb  3 14:30:28 2012" */
#include "xrM2MM.h"
#define xrM2MM_C_
#include "xmRTS.h"
#include "xrMM.h"

#define LINK_SZ sizeof(struct X2C_LINK_STR)

#define GAP_SIZE 128


extern void X2C_ALLOCATE(X2C_ADDRESS * a, size_t size)
{
   X2C_LINK l;
   *a = 0;
   if (size==0U) return;
   xrMM_allocate(&l, size, 0);
   if (l==0) return;
   l->_0.tags |= 0x8000UL;
   l->_.td = x2c_td_null;
   *a = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if ((l->_0.tags&0xFFE00000UL)==0UL) {
      xrMM_expusedmem += xrMM_getLargeObjBlock(l)->fsum;
   }
   else xrMM_expusedmem += xrMM_getHpObjSize(l);
} /* end X2C_ALLOCATE() */


extern void X2C_DEALLOCATE(X2C_ADDRESS * a)
{
   X2C_LINK l;
   xrMM_Block x;
   X2C_CARD32 sz;
   if (*a==0) return;
   l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)*a-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if (l->_.td->res!=0x093678150UL) X2C_ASSERT_F(100UL);
   if ((0x8000UL & l->_0.tags)) {
      if ((l->_0.tags&0xFFE00000UL)==0UL) {
         x = xrMM_getLargeObjBlock(l);
         if (x->magic!=305419896UL) X2C_ASSERT_F(101UL);
         sz = x->fsum;
         xrMM_free_block(x);
      }
      else {
         sz = xrMM_getHpObjSize(l);
         xrMM_dealloc(l);
      }
      xrMM_expusedmem -= sz;
      X2C_usedmem -= sz;
      --X2C_objects;
   }
   else l->_0.tags &= ~0x4000UL;
   *a = 0;
} /* end X2C_DEALLOCATE() */


extern void X2C_DYNALLOCATE(X2C_ADDRESS * a, size_t size, size_t lens[],
                size_t dims)
{
   xrMM_Dynarr desc;
   *a = 0;
   X2C_ALLOCATE((X2C_ADDRESS *) &desc, xrMM_DynarrDescSize(dims));
   if (desc) {
      X2C_InitDesc(desc, &size, lens, dims);
      X2C_ALLOCATE(&desc->a, size);
      if (desc->a==0) X2C_DEALLOCATE((X2C_ADDRESS *) &desc);
      else *a = (X2C_ADDRESS)desc;
   }
} /* end X2C_DYNALLOCATE() */


extern void X2C_DYNDEALLOCATE(X2C_ADDRESS * a)
{
   xrMM_Dynarr d;
   if (*a) {
      d = (xrMM_Dynarr)*a;
      X2C_DEALLOCATE(&d->a);
      X2C_DEALLOCATE(a);
   }
} /* end X2C_DYNDEALLOCATE() */

static X2C_ADDRESS get_addr(size_t, X2C_ADDRESS *, size_t, const size_t [],
                size_t);

typedef X2C_ADDRESS * PTR_TO_ADDR;


static X2C_ADDRESS get_addr(size_t dim, X2C_ADDRESS * free_addr, size_t size,
                 const size_t lens[], size_t dims)
{
   X2C_CARD32 i;
   X2C_ADDRESS addr;
   PTR_TO_ADDR tmp_ptr;
   X2C_CARD32 tmp;
   addr = *free_addr;
   if (dim+1U==dims) {
      *free_addr = (X2C_ADDRESS)((X2C_ADDRESS)*free_addr+(X2C_INT32)
                (size*lens[dim]));
   }
   else {
      *free_addr = (X2C_ADDRESS)((X2C_ADDRESS)*free_addr+(X2C_INT32)
                (lens[dim]*4U));
      tmp = lens[dim]-1U;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         tmp_ptr = (PTR_TO_ADDR)(X2C_ADDRESS)((X2C_ADDRESS)addr+(X2C_INT32)
                (i*4UL));
         *tmp_ptr = get_addr(dim+1U, free_addr, size, lens, dims);
         if (i==tmp) break;
      } /* end for */
   }
   return addr;
} /* end get_addr() */


extern void X2C_DYNCALLOCATE(X2C_ADDRESS * a, size_t size, size_t lens[],
                size_t dims)
{
   X2C_CARD32 i;
   size_t full_size;
   X2C_ADDRESS addr;
   X2C_ADDRESS free_addr;
   X2C_CARD32 tmp;
   full_size = size*lens[dims-1U];
   tmp = dims;
   i = 2UL;
   if (i<=tmp) for (;; i++) {
      full_size = (full_size+4U)*lens[dims-i];
      if (i==tmp) break;
   } /* end for */
   X2C_ALLOCATE(a, full_size);
   if (*a) {
      free_addr = *a;
      addr = get_addr(0U, &free_addr, size, lens, dims);
   }
} /* end X2C_DYNCALLOCATE() */


extern void X2C_DYNCDEALLOCATE(X2C_ADDRESS * a)
{
   if (*a) X2C_DEALLOCATE(a);
} /* end X2C_DYNCDEALLOCATE() */

