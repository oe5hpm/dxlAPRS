/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrO2MM.c Feb  3 14:30:28 2012" */
#include "xrO2MM.h"
#define xrO2MM_C_
#include "xmRTS.h"
#include "xrMM.h"
#include "TERMINATION.h"
#include "xosMalloc.h"
#include "xosMem.h"
#include "xrBlockManager.h"
#include "X2C.h"
#include "Printf.h"
#include "xrtsOS.h"
#include "COROUTINES.h"
#include "TimeConv.h"

X2C_CARD32 MutatorPeriod;
X2C_CARD32 GCPeriod;
X2C_CARD32 TotalGCTime;
typedef X2C_ADDRESS * ADDR_REF;

struct DestructDesc;

typedef struct DestructDesc * Destruct;


struct DestructDesc {
   X2C_DPROC proc;
   X2C_ADDRESS adr;
   Destruct next;
};

typedef X2C_ADDRESS * OFFS;

#define LINK_SZ sizeof(struct X2C_LINK_STR)

#define GAP_SIZE 128

#define AdrLss X2C_adr_lss

#define AdrGtr X2C_adr_gtr

static X2C_TD dynarrs[8];

static Destruct destruct;

static X2C_ADDRESS TAG_END;

static X2C_ADDRESS TAG_ARR;

static X2C_ADDRESS TAG_REC;

static X2C_CARD32 ADR_ALIGMENT;

static COROUTINES_COROUTINE CollectPrs;

static X2C_ADDRESS dyn_offs[2];

static X2C_CHAR prs_wsp[21072];


static void InsufficientMemory(void)
{
   X2C_TRAP((X2C_INT32)X2C_noMemoryException);
} /* end InsufficientMemory() */

static X2C_CARD32 SZ_MASK = 0x3FF8UL;


static X2C_CARD32 getHpObjSize(X2C_LINK obj)
{
   return (X2C_CARD32)((X2C_CARD32)obj->_0.size&0x3FF8UL);
} /* end getHpObjSize() */


static void stampObjAsFree(X2C_LINK obj, X2C_CARD32 size)
{
   obj->_0.size = (X2C_CARD32)(obj->_0.tags&0xFFE00000UL|0x20000UL)+size;
} /* end stampObjAsFree() */


extern void X2C_NEW(X2C_TD type, X2C_ADDRESS * a, size_t size,
                X2C_BOOLEAN sys)
{
   X2C_LINK l;
   if (size==0U) {
      *a = 0;
      return;
   }
   xrMM_allocate(&l, size, type!=x2c_td_null);
   if (l==0) InsufficientMemory();
   l->_.td = type;
   *a = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   l->_0.tags &= ~0x8000UL;
   if (sys) l->_0.tags |= 0x4000UL;
} /* end X2C_NEW() */


extern void X2C_NEW_OPEN(X2C_TD type, X2C_ADDRESS * a, size_t size,
                size_t lens[], size_t dims, X2C_BOOLEAN sys)
{
   X2C_TD dtype;
   xrMM_Dynarr desc;
   *a = 0;
   dtype = dynarrs[dims-1U];
   X2C_NEW(dtype, (X2C_ADDRESS *) &desc, dtype->size, sys);
   X2C_InitDesc(desc, &size, lens, dims);
   X2C_NEW(type, &desc->a, size, sys);
   *a = (X2C_ADDRESS)desc;
} /* end X2C_NEW_OPEN() */


extern void X2C_DISPOSE(X2C_ADDRESS * a)
{
   X2C_LINK l;
   X2C_CARD32 i;
   xrMM_Dynarr desc;
   if (*a==0) return;
   l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)*a-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if (l->_.td->res!=0x093678150UL) X2C_ASSERT_F(100UL);
   i = 0UL;
   while (i<8UL && l->_.td!=dynarrs[i]) ++i;
   l->_0.tags &= ~0x4000UL;
   if (i<8UL) {
      desc = (xrMM_Dynarr)*a;
      l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)desc->a-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
      if (l->_.td->res!=0x093678150UL) X2C_ASSERT_F(101UL);
      l->_0.tags &= ~0x4000UL;
   }
   l = 0;
   *a = 0;
} /* end X2C_DISPOSE() */


extern void X2C_DESTRUCTOR(X2C_ADDRESS a, X2C_DPROC p)
{
   Destruct d;
   X2C_LINK ln;
   if (a==0) X2C_ASSERT_F(102UL);
   if (p==0) X2C_ASSERT_F(103UL);
   ln = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)a-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if ((ln->_0.tags&0xC000UL)!=0UL) X2C_ASSERT_F(104UL);
   if (ln->_.td->res!=0x093678150UL) X2C_ASSERT_F(105UL);
   X2C_NEW(x2c_td_null, (X2C_ADDRESS *) &d,
                sizeof(Destruct)+sizeof(struct X2C_LINK_STR), 0);
   d->adr = a;
   d->proc = p;
   d->next = destruct;
   destruct = d;
} /* end X2C_DESTRUCTOR() */


static void FinalOne(Destruct d)
{
   struct X2C_XHandler_STR anonym;
   if (X2C_XTRY(&anonym)) {
      d->proc(d->adr);
      X2C_XOFF();
   }
   else {
   }
   X2C_XREMOVE();
} /* end FinalOne() */

static void FinalAll(void);


static void FinalAll(void)
{
   Destruct n;
   while (destruct) {
      n = destruct->next;
      FinalOne(destruct);
      destruct = n;
   }
} /* end FinalAll() */

static X2C_TD marked;

static X2C_TD tail;


static X2C_CARD32 getObjectSize(X2C_LINK l)
{
   X2C_CARD32 sz;
   xrMM_Block b;
   sz = getHpObjSize(l);
   if (sz==0UL) {
      b = xrMM_getLargeObjBlock(l);
      return b->size;
   }
   else return sz;
   return 0;
} /* end getObjectSize() */

#define Kb 1024

static X2C_BOOLEAN countingClosureInProgress;

static X2C_CARD32 anchorWeight;

static X2C_CARD32 totalWeightOfSmallAnchors;

static X2C_CHAR printBuffer[1024];

static X2C_BOOLEAN hasModuleAnchors;

static X2C_BOOLEAN hasDestructorAnchors;

static X2C_BOOLEAN hasStackAnchors;

typedef X2C_CHAR * pSTR;


static void printAnchorTraceCaption(void)
{
   X2C_SEQ tmp[1];
   Printf_sprintf(printBuffer, 1024ul, "\\n*********************** GC ANCHORS\
 TRACE ***********************\\n", 69ul, 0, 0ul);
   X2C_StdOutS(printBuffer, 0UL);
   Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- EXPLICITLY ALLOCATED\\n",
                34ul, (tmp[0].val = (xrMM_expusedmem/1024UL),
                (X2C_LOC *)tmp), sizeof(X2C_SEQ));
   X2C_StdOutS(printBuffer, 0UL);
} /* end printAnchorTraceCaption() */

static void Closure(void);


static void printModuleAnchorWeight(const X2C_CHAR anchorType[],
                X2C_CARD32 anchorType_len, X2C_ADDRESS adr,
                X2C_pCHAR moduleName)
{
   X2C_SEQ tmp[6];
   Closure();
   if (anchorWeight>=xrMM_anchorWeightThreshold) {
      if (!hasModuleAnchors) {
         hasModuleAnchors = 1;
         Printf_sprintf(printBuffer, 1024ul, "\\n\\n", 5ul, 0, 0ul);
         X2C_StdOutS(printBuffer, 0UL);
      }
      Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- %s %s <A:%x>\\n", 26ul,
                 (tmp[0].val = (anchorWeight/1024UL),
                tmp[1].adr = (pSTR)moduleName, tmp[2].adr = anchorType,
                tmp[3].val = 0, tmp[4].val = anchorType_len-1,
                tmp[5].adr = adr, (X2C_LOC *)tmp), 6ul*sizeof(X2C_SEQ));
      X2C_StdOutS(printBuffer, 0UL);
   }
   else totalWeightOfSmallAnchors += anchorWeight;
} /* end printModuleAnchorWeight() */


static void printDestructorAnchorWeight(X2C_ADDRESS obj)
{
   X2C_TD type;
   X2C_SEQ tmp[4];
   Closure();
   if (anchorWeight>=xrMM_anchorWeightThreshold) {
      if (!hasDestructorAnchors) {
         hasDestructorAnchors = 1;
         Printf_sprintf(printBuffer, 1024ul, "\\n\\n", 5ul, 0, 0ul);
         X2C_StdOutS(printBuffer, 0UL);
      }
      type = ((X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)obj-(X2C_INT32)
                sizeof(struct X2C_LINK_STR)))->_.td;
      if (type->module && type->module->name) {
         Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- DESTRUCTOR OF OBJ %x\
H (type %s.%s)\\n", 48ul, (tmp[0].val = (anchorWeight/1024UL),
                tmp[1].adr = obj, tmp[2].adr = (pSTR)type->module->name,
                tmp[3].adr = (pSTR)type->name, (X2C_LOC *)tmp),
                4ul*sizeof(X2C_SEQ));
      }
      else {
         Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- DESTRUCTOR OF OBJ %x\
H (type %s)\\n", 45ul, (tmp[0].val = (anchorWeight/1024UL), tmp[1].adr = obj,
                 tmp[2].adr = (pSTR)type->name, (X2C_LOC *)tmp),
                3ul*sizeof(X2C_SEQ));
      }
      X2C_StdOutS(printBuffer, 0UL);
   }
   else totalWeightOfSmallAnchors += anchorWeight;
} /* end printDestructorAnchorWeight() */


static void printStackAnchorWeight(X2C_ADDRESS obj)
{
   X2C_TD type;
   X2C_SEQ tmp[4];
   Closure();
   if (anchorWeight>=xrMM_anchorWeightThreshold) {
      if (!hasStackAnchors) {
         hasStackAnchors = 1;
         Printf_sprintf(printBuffer, 1024ul, "\\n\\n", 5ul, 0, 0ul);
         X2C_StdOutS(printBuffer, 0UL);
      }
      type = ((X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)obj-(X2C_INT32)
                sizeof(struct X2C_LINK_STR)))->_.td;
      if (type->module && type->module->name) {
         Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- STACK REF TO %xH (ty\
pe %s.%s)\\n", 43ul, (tmp[0].val = (anchorWeight/1024UL), tmp[1].adr = obj,
                tmp[2].adr = (pSTR)type->module->name,
                tmp[3].adr = (pSTR)type->name, (X2C_LOC *)tmp),
                4ul*sizeof(X2C_SEQ));
      }
      else {
         Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- STACK REF TO %xH (ty\
pe %s)\\n", 40ul, (tmp[0].val = (anchorWeight/1024UL), tmp[1].adr = obj,
                tmp[2].adr = (pSTR)type->name, (X2C_LOC *)tmp),
                3ul*sizeof(X2C_SEQ));
      }
      X2C_StdOutS(printBuffer, 0UL);
   }
   else totalWeightOfSmallAnchors += anchorWeight;
} /* end printStackAnchorWeight() */


static void printSmallAnchorsWeight(void)
{
   X2C_SEQ tmp[2];
   if (totalWeightOfSmallAnchors>0UL) {
      Printf_sprintf(printBuffer, 1024ul, "\\n %8d Kb <- Total size of small \
(< %d bytes) anchors\\n", 56ul, (tmp[0].val = (totalWeightOfSmallAnchors/1024UL),
                 tmp[1].val = xrMM_anchorWeightThreshold, (X2C_LOC *)tmp),
                2ul*sizeof(X2C_SEQ));
      X2C_StdOutS(printBuffer, 0UL);
   }
} /* end printSmallAnchorsWeight() */

static void AppendObject(ADDR_REF);


static void printLargeObjects(void)
{
   xrMM_Block fb;
   xrMM_Block b;
   X2C_LINK l;
   X2C_ADDRESS o;
   X2C_SEQ tmp[5];
   countingClosureInProgress = 1;
   Printf_sprintf(printBuffer, 1024ul, "\\nHANGING LARGE OBJECTS:\\n", 27ul,
                0, 0ul);
   X2C_StdOutS(printBuffer, 0UL);
   fb = xrMM_f_blocks[21U];
   b = fb->next;
   while (b!=fb) {
      l = (X2C_LINK)xrMM_getFirstBlockAdr(b);
      o = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
      anchorWeight = 0UL;
      AppendObject(&o);
      Closure();
      if (l->_.td->module && l->_.td->module->name) {
         Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- LARGE OBJ %xH (type:\
%s.%s, size:%d)\\n", 49ul, (tmp[0].val = (anchorWeight/1024UL),
                tmp[1].adr = o, tmp[2].adr = (pSTR)l->_.td->module->name,
                tmp[3].adr = (pSTR)l->_.td->name, tmp[4].val = b->size,
                (X2C_LOC *)tmp), 5ul*sizeof(X2C_SEQ));
      }
      else {
         Printf_sprintf(printBuffer, 1024ul, " %8d Kb <- LARGE OBJ %xH (type:\
%s, size:%d)\\n", 46ul, (tmp[0].val = (anchorWeight/1024UL), tmp[1].adr = o,
                tmp[2].adr = (pSTR)l->_.td->name, tmp[3].val = b->size,
                (X2C_LOC *)tmp), 4ul*sizeof(X2C_SEQ));
      }
      X2C_StdOutS(printBuffer, 0UL);
      b = b->next;
   }
   countingClosureInProgress = 0;
} /* end printLargeObjects() */

typedef void ( *HeapIterator_P)(X2C_LINK);

static X2C_CARD32 xrO2MM_liveObjTags = 0x1C000UL;


static void iterateBlocks(HeapIterator_P callback, xrMM_Block blocks[],
                X2C_CARD32 blocks_len)
{
   X2C_CARD32 i;
   xrMM_Block x;
   xrMM_Block root;
   X2C_LINK l;
   X2C_CARD32 size;
   X2C_CARD32 sz;
   root = blocks[0UL];
   x = root->next;
   while (x!=root) {
      l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
      sz = 0UL;
      for (;;) {
         if ((0x1C000UL&l->_0.tags)!=0UL) callback(l);
         size = getHpObjSize(l);
         l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                size);
         sz += size;
         if (sz>=16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR)) break;
      }
      x = x->next;
   }
   for (i = 1UL; i<=20UL; i++) {
      root = blocks[i];
      x = root->next;
      size = i*sizeof(struct X2C_LINK_STR);
      while (x!=root) {
         l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
         while (X2C_adr_lss((X2C_ADDRESS)l, x->mem)) {
            if ((0x1C000UL&l->_0.tags)!=0UL) callback(l);
            l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                l+(X2C_INT32)size);
         }
         x = x->next;
      }
   } /* end for */
   root = blocks[21UL];
   x = root->next;
   while (x!=root) {
      l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
      if ((0x1C000UL&l->_0.tags)!=0UL) callback(l);
      x = x->next;
   }
} /* end iterateBlocks() */


static void iterateHeap(HeapIterator_P callback)
{
   iterateBlocks(callback, xrMM_f_blocks, 22ul);
   iterateBlocks(callback, xrMM_b_blocks, 22ul);
} /* end iterateHeap() */

#define HeapProfile_MAX 16384

struct HeapProfile_Rec;


struct HeapProfile_Rec {
   X2C_TD type;
   X2C_CARD32 size;
};

static struct HeapProfile_Rec HeapProfile_types[16384];

static X2C_INT32 HeapProfile_NTypes;

static X2C_INT32 HeapProfile_lastType;


static void HeapProfile_reset(void)
{
   HeapProfile_NTypes = 0L;
   HeapProfile_lastType = 0L;
   HeapProfile_types[HeapProfile_lastType].type = 0;
   HeapProfile_types[HeapProfile_lastType].size = 0UL;
} /* end HeapProfile_reset() */

static void HeapProfile_recordObject(X2C_LINK);


static void HeapProfile_recordObject(X2C_LINK l)
{
   X2C_TD type;
   size_t size;
   X2C_INT32 i;
   type = l->_.td;
   size = getObjectSize(l);
   if (HeapProfile_types[HeapProfile_lastType].type==type) {
      i = HeapProfile_lastType;
   }
   else {
      i = 0L;
      for (;;) {
         if (i==HeapProfile_NTypes) {
            HeapProfile_types[i].type = type;
            HeapProfile_types[i].size = 0UL;
            ++HeapProfile_NTypes;
            break;
         }
         if (HeapProfile_types[i].type==type) break;
         ++i;
      }
   }
   HeapProfile_types[i].size += size;
   HeapProfile_lastType = i;
} /* end HeapProfile_recordObject() */


static void HeapProfile_print(void)
{
   X2C_INT32 i;
   X2C_CARD32 otherSize;
   X2C_INT32 tmp;
   X2C_SEQ tmp0[3];
   Printf_sprintf(printBuffer, 1024ul, "\\n\\nHEAP TRACE:\\n", 18ul, 0, 0ul);
   X2C_StdOutS(printBuffer, 0UL);
   otherSize = 0UL;
   tmp = HeapProfile_NTypes-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      if (HeapProfile_types[i].size>=xrMM_heapTracingThreshold) {
         if (HeapProfile_types[i].type->module && HeapProfile_types[i]
                .type->module->name) {
            Printf_sprintf(printBuffer, 1024ul, " %8d Kb - %s.%s\\n", 18ul,
                (tmp0[0].val = (HeapProfile_types[i].size/1024UL),
                tmp0[1].adr = (pSTR)HeapProfile_types[i].type->module->name,
                tmp0[2].adr = (pSTR)HeapProfile_types[i].type->name,
                (X2C_LOC *)tmp0), 3ul*sizeof(X2C_SEQ));
         }
         else {
            Printf_sprintf(printBuffer, 1024ul, " %8d Kb - %s\\n", 15ul,
                (tmp0[0].val = (HeapProfile_types[i].size/1024UL),
                tmp0[1].adr = (pSTR)HeapProfile_types[i].type->name,
                (X2C_LOC *)tmp0), 2ul*sizeof(X2C_SEQ));
         }
         X2C_StdOutS(printBuffer, 0UL);
      }
      else otherSize += HeapProfile_types[i].size;
      if (i==tmp) break;
   } /* end for */
   if (otherSize>0UL) {
      Printf_sprintf(printBuffer, 1024ul, " %8d Kb - OTHER (< %d bytes)\\n",
                31ul, (tmp0[0].val = (otherSize/1024UL),
                tmp0[1].val = xrMM_heapTracingThreshold, (X2C_LOC *)tmp0),
                2ul*sizeof(X2C_SEQ));
      X2C_StdOutS(printBuffer, 0UL);
   }
} /* end HeapProfile_print() */


static void printHeapTrace(void)
{
   HeapProfile_reset();
   iterateHeap(HeapProfile_recordObject);
   HeapProfile_print();
} /* end printHeapTrace() */


static void AppendObject(ADDR_REF ref)
{
   X2C_LINK l;
   X2C_TD t;
   X2C_ADDRESS a;
   a = *ref;
   if (a==0) return;
   l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)a-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if ((0x40000UL & l->_0.tags)) {
      l = l->_.next;
      a = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
      *ref = a;
   }
   if ((l->_0.tags&0x18000UL)==0UL) {
      l->_0.tags |= 0x10000UL;
      if (countingClosureInProgress) anchorWeight += getObjectSize(l);
      t = l->_.td;
      if (t->res!=0x093678150UL) X2C_ASSERT_F(109UL);
      if (t!=x2c_td_null) {
         if (t->link) t->tail->_.td = (X2C_TD)l;
         else {
            t->link = l;
            if (marked==0) marked = t;
            else tail->succ = t;
            tail = t;
            t->succ = 0;
         }
         t->tail = l;
         l->_.next = 0;
      }
   }
} /* end AppendObject() */

static void ScanRecord(X2C_ADDRESS, OFFS);


static void ScanRecord(X2C_ADDRESS base, OFFS desc)
{
   X2C_TD type;
   X2C_ADDRESS x;
   X2C_ADDRESS cur;
   X2C_ADDRESS last;
   X2C_CARD32 i;
   x = desc[0U];
   i = 1UL;
   while (x!=TAG_END) {
      if (x==TAG_ARR) {
         last = (X2C_ADDRESS)((X2C_ADDRESS)base+(X2C_INT32)(desc[i]-X2C_BASE)
                );
         ++i;
         x = desc[i];
         ++i;
         if (x==TAG_REC) {
            type = (X2C_TD)desc[i];
            ++i;
            if (type->res!=0x093678150UL) X2C_ASSERT_F(112UL);
            cur = (X2C_ADDRESS)((X2C_ADDRESS)base+(X2C_INT32)
                (desc[i]-X2C_BASE));
            ++i;
            for (;;) {
               ScanRecord(cur, (OFFS)type->offs);
               if (cur==last) break;
               cur = (X2C_ADDRESS)((X2C_ADDRESS)cur+(X2C_INT32)type->size);
            }
         }
         else {
            cur = (X2C_ADDRESS)((X2C_ADDRESS)base+(X2C_INT32)(x-X2C_BASE));
            for (;;) {
               AppendObject((ADDR_REF)cur);
               if (cur==last) break;
               cur = (X2C_ADDRESS)((X2C_ADDRESS)cur+(X2C_INT32)
                sizeof(X2C_ADDRESS));
            }
         }
      }
      else if (x==TAG_REC) {
         type = (X2C_TD)desc[i];
         ++i;
         if (type->res!=0x093678150UL) X2C_ASSERT_F(113UL);
         ScanRecord((X2C_ADDRESS)((X2C_ADDRESS)base+(X2C_INT32)
                (desc[i]-X2C_BASE)), (OFFS)type->offs);
         ++i;
      }
      else {
         AppendObject((ADDR_REF)(X2C_ADDRESS)((X2C_ADDRESS)base+(X2C_INT32)
                (x-X2C_BASE)));
      }
      x = desc[i];
      ++i;
   }
} /* end ScanRecord() */


static Destruct MarkDestructors(void)
{
   Destruct m;
   Destruct l;
   Destruct r;
   Destruct d;
   X2C_LINK d_link;
   X2C_LINK ln;
   if (xrMM_anchorTracing) {
      countingClosureInProgress = 1;
      hasDestructorAnchors = 0;
   }
   d = destruct;
   l = 0;
   r = 0;
   while (d) {
      d_link = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                d-(X2C_INT32)sizeof(struct X2C_LINK_STR));
      d_link->_0.tags |= 0x10000UL;
      ln = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)d->adr-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
      if ((ln->_0.tags&0xC000UL)!=0UL) X2C_ASSERT_F(119UL);
      if ((0x10000UL & ln->_0.tags)) {
         l = d;
         d = d->next;
      }
      else {
         m = d;
         d = d->next;
         if (l==0) destruct = d;
         else l->next = d;
         m->next = r;
         r = m;
         anchorWeight = 0UL;
         AppendObject(&m->adr);
         if (xrMM_anchorTracing) printDestructorAnchorWeight(m->adr);
      }
   }
   if (xrMM_anchorTracing) countingClosureInProgress = 0;
   return r;
} /* end MarkDestructors() */


static void AdjustRef(ADDR_REF ref)
{
   X2C_LINK l;
   if (*ref==0) return;
   l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)*ref-(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   if ((0x40000UL & l->_0.tags)) {
      *ref = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l->_.next+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   }
} /* end AdjustRef() */


static void AdjustRTStructures(void)
{
   Destruct d;
   if (destruct) {
      AdjustRef((ADDR_REF) &destruct);
      d = destruct;
      while (d) {
         AdjustRef((ADDR_REF) &d->next);
         AdjustRef(&d->adr);
         d = d->next;
      }
   }
} /* end AdjustRTStructures() */

static void SortPtrs(X2C_ADDRESS [], X2C_CARD32, X2C_INT32, X2C_INT32);


static void SortPtrs(X2C_ADDRESS buf[], X2C_CARD32 buf_len, X2C_INT32 l,
                X2C_INT32 r)
{
   X2C_INT32 j;
   X2C_INT32 i;
   X2C_ADDRESS w;
   X2C_ADDRESS x;
   i = l;
   j = r;
   x = buf[X2C_DIV(l+r,2L)];
   do {
      while (X2C_adr_lss(buf[i], x)) ++i;
      while (X2C_adr_gtr(buf[j], x)) --j;
      if (i<=j) {
         w = buf[i];
         buf[i] = buf[j];
         buf[j] = w;
         ++i;
         --j;
      }
   } while (i<=j);
   if (l<j) SortPtrs(buf, buf_len, l, j);
   if (i<r) SortPtrs(buf, buf_len, i, r);
} /* end SortPtrs() */


static void Sort(X2C_ADDRESS * max0, X2C_ADDRESS * min0, xrMM_Block * head,
                X2C_ADDRESS buf[], X2C_CARD32 buf_len, X2C_CARD32 no)
{
   xrMM_Block b;
   xrMM_Block h;
   xrMM_Block l;
   X2C_CARD32 i;
   X2C_ADDRESS be;
   X2C_CARD32 tmp;
   SortPtrs(buf, buf_len, 0L, (X2C_INT32)(no-1UL));
   h = 0;
   l = *head;
   tmp = no-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      b = (xrMM_Block)buf[i];
      while (l && X2C_adr_lss((X2C_ADDRESS)l, (X2C_ADDRESS)b)) {
         h = l;
         l = l->snxt;
      }
      if (h==0) *head = b;
      else h->snxt = b;
      h = b;
      b->snxt = l;
      if (i==tmp) break;
   } /* end for */
   l = (xrMM_Block)buf[no-1UL];
   be = (X2C_ADDRESS)((X2C_ADDRESS)xrMM_getLastBlockAdr1(l)-(X2C_INT32)1L);
   if (*min0==0 || X2C_adr_lss(buf[0UL], *min0)) *min0 = buf[0UL];
   if (*max0==0 || X2C_adr_gtr(be, *max0)) *max0 = be;
} /* end Sort() */


static void SortBlocks(const xrMM_Block blocks[], X2C_CARD32 blocks_len,
                xrMM_Block * head, X2C_ADDRESS * min0, X2C_ADDRESS * max0)
{
   xrMM_Block b;
   xrMM_Block e;
   X2C_CARD32 j;
   X2C_CARD32 i;
   X2C_ADDRESS buf[256];
   X2C_CARD32 tmp;
   j = 0UL;
   tmp = blocks_len-1;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      b = blocks[i]->next;
      e = blocks[i];
      while (b!=e) {
         if (j>255UL) {
            Sort(max0, min0, head, buf, 256ul, j);
            j = 0UL;
         }
         buf[j] = (X2C_ADDRESS)b;
         ++j;
         b = b->next;
      }
      if (i==tmp) break;
   } /* end for */
   if (j>0UL) {
      Sort(max0, min0, head, buf, 256ul, j);
      j = 0UL;
   }
} /* end SortBlocks() */


static void CheckPtrs(xrMM_Block head, X2C_ADDRESS buf[], X2C_CARD32 buf_len,
                 X2C_CARD32 no)
{
   X2C_ADDRESS obj;
   X2C_ADDRESS end;
   X2C_CARD32 sz;
   X2C_CARD32 i;
   X2C_LINK n;
   X2C_LINK l;
   SortPtrs(buf, buf_len, 0L, (X2C_INT32)(no-1UL));
   i = 0UL;
   if (head->magic!=305419896UL) X2C_ASSERT_F(121UL);
   if (X2C_IN((X2C_INT32)head->root,32,0x200001UL)) {
      end = xrMM_getLastBlockAdr1(head);
   }
   else end = head->mem;
   for (;;) {
      if (X2C_adr_lss(buf[i], (X2C_ADDRESS)((X2C_ADDRESS)xrMM_getFirstBlockAdr(head)+(X2C_INT32)(X2C_INT32)sizeof(struct X2C_LINK_STR)))) {
         ++i;
         if (i==no) break;
      }
      else if (!X2C_adr_lss(buf[i], end)) {
         head = head->snxt;
         if (head==0) break;
         if (head->magic!=305419896UL) X2C_ASSERT_F(123UL);
         if (X2C_IN((X2C_INT32)head->root,32,0x200001UL)) {
            end = xrMM_getLastBlockAdr1(head);
         }
         else end = head->mem;
      }
      else if (head->root==21U) {
         l = (X2C_LINK)xrMM_getFirstBlockAdr(head);
         obj = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
         if ((0x4UL & l->_0.tags)==0) {
            anchorWeight = 0UL;
            AppendObject(&obj);
            l->_0.tags |= 0x80000UL;
            if (xrMM_anchorTracing) printStackAnchorWeight(obj);
         }
         ++i;
         if (i==no) break;
      }
      else {
         l = (X2C_LINK)xrMM_getFirstBlockAdr(head);
         if (head->root) {
            sz = (X2C_CARD32)head->root*sizeof(struct X2C_LINK_STR);
            for (;;) {
               for (;;) {
                  n = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                l+(X2C_INT32)sz);
                  if (X2C_adr_lss(buf[i], (X2C_ADDRESS)n)) break;
                  l = n;
               }
               obj = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
               if ((0x60000UL&l->_0.tags)==0UL && !X2C_adr_lss(buf[i], obj)) {
                  anchorWeight = 0UL;
                  AppendObject(&obj);
                  l->_0.tags |= 0x80000UL;
                  if (xrMM_anchorTracing) printStackAnchorWeight(obj);
               }
               if (i+1UL<no && X2C_adr_lss(buf[i+1UL], end)) ++i;
               else break;
            }
         }
         else {
            for (;;) {
               for (;;) {
                  sz = getHpObjSize(l);
                  n = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                l+(X2C_INT32)sz);
                  if (X2C_adr_lss(buf[i], (X2C_ADDRESS)n)) break;
                  l = n;
               }
               obj = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
               if ((0x60000UL&l->_0.tags)==0UL && !X2C_adr_lss(buf[i], obj)) {
                  anchorWeight = 0UL;
                  AppendObject(&obj);
                  l->_0.tags |= 0x80000UL;
                  if (xrMM_anchorTracing) printStackAnchorWeight(obj);
               }
               if (i+1UL<no && X2C_adr_lss(buf[i+1UL], end)) ++i;
               else break;
            }
         }
         ++i;
         if (i==no) {
            break;
         }
      }
   }
} /* end CheckPtrs() */


static void ScanStack(xrMM_Block head, X2C_ADDRESS min0, X2C_ADDRESS max0,
                ADDR_REF fr, ADDR_REF to)
{
   X2C_ADDRESS buf[256];
   ADDR_REF r;
   X2C_CARD32 i;
   if (sizeof(X2C_ADDRESS)>ADR_ALIGMENT) {
      to = (ADDR_REF)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)to-(X2C_INT32)
                (sizeof(X2C_ADDRESS)-ADR_ALIGMENT));
   }
   else {
      to = (ADDR_REF)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)to-(X2C_INT32)
                (ADR_ALIGMENT-sizeof(X2C_ADDRESS)));
   }
   while (!X2C_adr_gtr((X2C_ADDRESS)fr, (X2C_ADDRESS)to)) {
      i = 0UL;
      for (;;) {
         r = fr;
         fr = (ADDR_REF)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                fr+(X2C_INT32)ADR_ALIGMENT);
         if (!X2C_adr_lss(*r, min0) && !X2C_adr_gtr(*r, max0)) {
            buf[i] = *r;
            ++i;
            if (i>255UL) break;
         }
         if (X2C_adr_gtr((X2C_ADDRESS)fr, (X2C_ADDRESS)to)) break;
      }
      if (i>0UL) CheckPtrs(head, buf, 256ul, i);
   }
} /* end ScanStack() */


static void MarkStack(void)
{
   X2C_ADDRESS max0;
   X2C_ADDRESS min0;
   ADDR_REF a;
   ADDR_REF to;
   ADDR_REF fr;
   xrMM_Block head;
   X2C_Coroutine e;
   X2C_Coroutine c;
   if (xrMM_anchorTracing) {
      countingClosureInProgress = 1;
      hasStackAnchors = 0;
   }
   head = 0;
   min0 = 0;
   max0 = 0;
   SortBlocks(xrMM_f_blocks, 22ul, &head, &min0, &max0);
   SortBlocks(xrMM_b_blocks, 22ul, &head, &min0, &max0);
   if (head) {
      e = X2C_GetCurrent();
      c = e->fwd;
      for (;;) {
         if (c==e) break;
         if (c==xrMM_GCInvoker) {
            fr = (ADDR_REF)c->stk_start;
            to = (ADDR_REF)xrMM_StackEnd4GC;
         }
         else {
            fr = (ADDR_REF)c->stk_start;
            to = (ADDR_REF)c->stk_end;
         }
         if (to==0) to = fr;
         if (X2C_adr_lss((X2C_ADDRESS)to, (X2C_ADDRESS)fr)) {
            a = to;
            to = fr;
            fr = a;
         }
         ScanStack(head, min0, max0, fr, to);
         if (c->reg_dsize>sizeof(X2C_ADDRESS) && c!=xrMM_GCInvoker) {
            fr = (ADDR_REF)c->reg_dump;
            to = (ADDR_REF)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                fr+(X2C_INT32)c->reg_dsize);
            ScanStack(head, min0, max0, fr, to);
         }
         c = c->fwd;
      }
   }
   if (xrMM_anchorTracing) countingClosureInProgress = 0;
} /* end MarkStack() */


static void MarkObject(X2C_LINK x, X2C_TD type)
{
   X2C_CARD32 len;
   X2C_CARD32 n;
   X2C_ADDRESS a;
   xrMM_Block b;
   if ((x->_0.tags&0xFFE00000UL)==0UL) {
      b = xrMM_getLargeObjBlock(x);
      len = b->fsum;
   }
   else len = getHpObjSize(x);
   len = (len-sizeof(struct X2C_LINK_STR))/type->size;
   a = (X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)x+(X2C_INT32)
                sizeof(struct X2C_LINK_STR));
   n = 0UL;
   while (n<len) {
      ScanRecord(a, (OFFS)type->offs);
      ++n;
      a = (X2C_ADDRESS)((X2C_ADDRESS)a+(X2C_INT32)(X2C_INT32)type->size);
   }
} /* end MarkObject() */


static void Closure(void)
{
   X2C_LINK x;
   X2C_LINK l;
   while (marked) {
      l = marked->link;
      while (l) {
         MarkObject(l, marked);
         x = l;
         l = l->_.next;
         x->_.td = marked;
      }
      marked->link = 0;
      marked = marked->succ;
   }
} /* end Closure() */


static void MarkModules(void)
{
   X2C_MD l;
   OFFS desc;
   X2C_TD type;
   X2C_ADDRESS cur;
   X2C_ADDRESS last;
   X2C_ADDRESS x;
   X2C_CARD32 i;
   X2C_ADDRESS anchorBase;
   if (xrMM_anchorTracing) {
      printAnchorTraceCaption();
      countingClosureInProgress = 1;
   }
   l = X2C_MODULES;
   while (l) {
      desc = (OFFS)l->offs;
      x = desc[0U];
      i = 1UL;
      hasModuleAnchors = 0;
      while (x!=TAG_END) {
         if (x==TAG_REC) {
            type = (X2C_TD)desc[i];
            ++i;
            if (type->res!=0x093678150UL) X2C_ASSERT_F(114UL);
            anchorBase = desc[i];
            anchorWeight = 0UL;
            ScanRecord(anchorBase, (OFFS)type->offs);
            ++i;
            if (xrMM_anchorTracing) {
               printModuleAnchorWeight("RECORD ", 8ul, anchorBase, l->name);
            }
         }
         else if (x==TAG_ARR) {
            last = desc[i];
            ++i;
            x = desc[i];
            ++i;
            if (x==TAG_REC) {
               type = (X2C_TD)desc[i];
               ++i;
               if (type->res!=0x093678150UL) X2C_ASSERT_F(116UL);
               cur = desc[i];
               ++i;
               anchorBase = cur;
               anchorWeight = 0UL;
               for (;;) {
                  ScanRecord(cur, (OFFS)type->offs);
                  if (cur==last) break;
                  cur = (X2C_ADDRESS)((X2C_ADDRESS)cur+(X2C_INT32)type->size)
                ;
               }
               if (xrMM_anchorTracing) {
                  printModuleAnchorWeight("ARRAY OF RECORDs", 17ul,
                anchorBase, l->name);
               }
            }
            else {
               cur = x;
               anchorWeight = 0UL;
               for (;;) {
                  AppendObject((ADDR_REF)cur);
                  if (cur==last) break;
                  cur = (X2C_ADDRESS)((X2C_ADDRESS)cur+(X2C_INT32)
                sizeof(X2C_ADDRESS));
               }
               if (xrMM_anchorTracing) {
                  printModuleAnchorWeight("ARRAY OF POINTERs", 18ul, x,
                l->name);
               }
            }
         }
         else {
            anchorWeight = 0UL;
            AppendObject((ADDR_REF)x);
            if (xrMM_anchorTracing) {
               printModuleAnchorWeight("POINTER", 8ul, x, l->name);
            }
         }
         x = desc[i];
         ++i;
      }
      l = l->next;
   }
   if (xrMM_anchorTracing) countingClosureInProgress = 0;
} /* end MarkModules() */

enum scanerStates {ss_noneFO, 
   ss_savedFO, 
   ss_accumFO};



static void normal_scan(xrMM_Block x, X2C_BOOLEAN defrag)
{
   X2C_CARD8 state;
   X2C_LINK savedFO;
   X2C_BOOLEAN savedFO_inPool;
   X2C_CARD32 mergedObjSize;
   X2C_LINK l;
   X2C_BOOLEAN l_inPool;
   X2C_CARD32 size;
   X2C_CARD32 sz;
   size = 0UL;
   state = ss_noneFO;
   l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
   x->fixed = 0;
   x->fsum = 16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR);
   for (;;) {
      sz = getHpObjSize(l);
      l_inPool = (0x20000UL & l->_0.tags)!=0;
      if (!l_inPool) {
         if ((l->_0.tags&0x1C000UL)==0UL) {
            X2C_usedmem -= sz;
            --X2C_objects;
            stampObjAsFree(l, sz);
         }
         else {
            x->fsum -= sz;
            if (defrag) {
               if ((l->_0.tags&0x18C000UL)!=0UL) x->fixed = 1;
               l->_0.tags &= ~0x10000UL;
            }
            else l->_0.tags = l->_0.tags&~0x90000UL;
            X2C_normalused += sz;
         }
      }
      if ((0x20000UL & l->_0.tags)) {
         switch ((unsigned)state) {
         case ss_noneFO:
            savedFO = l;
            savedFO_inPool = l_inPool;
            state = ss_savedFO;
            break;
         case ss_savedFO:
         case ss_accumFO:
            if (state==ss_savedFO) {
               if (savedFO_inPool) xrMM_foManager_del(savedFO);
            }
            if (l_inPool) xrMM_foManager_del(l);
            mergedObjSize = sz+getHpObjSize(savedFO);
            if (mergedObjSize==16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR)) {
               xrMM_free_block(x);
               return;
            }
            stampObjAsFree(savedFO, mergedObjSize);
            state = ss_accumFO;
            break;
         default:
            X2C_TRAP(X2C_CASE_TRAP);
         } /* end switch */
      }
      else {
         if (state==ss_accumFO || state==ss_savedFO && !savedFO_inPool) {
            xrMM_foManager_add(savedFO);
         }
         state = ss_noneFO;
      }
      size += sz;
      if (size>=16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR)) {
         if (state==ss_accumFO || state==ss_savedFO && !savedFO_inPool) {
            xrMM_foManager_add(savedFO);
         }
         X2C_normalbusy += 16384UL;
         break;
      }
      l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)sz);
   }
} /* end normal_scan() */

static X2C_CARD32 smallBlocksTotal;

static X2C_CARD32 smallBlocksStackFixed;

static X2C_CARD32 smallBlocksSysbitFixed;

static X2C_CARD32 smallBlocksExpbitFixed;

#define BOFS_BIT 18


static void small_scan(xrMM_Block x, X2C_CARD32 root, X2C_BOOLEAN busy,
                X2C_BOOLEAN defrag)
{
   X2C_LINK l;
   X2C_CARD32 size;
   size = root*sizeof(struct X2C_LINK_STR);
   l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
   x->fixed = 0;
   x->fsum = 16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR);
   while (X2C_adr_lss((X2C_ADDRESS)l, x->mem)) {
      if ((0x20000UL & l->_0.tags)==0) {
         if ((l->_0.tags&0x1C000UL)==0UL) {
            X2C_usedmem -= getHpObjSize(l);
            --X2C_objects;
            l->_.next = x->list;
            x->list = l;
            stampObjAsFree(l, size);
         }
         else {
            if (defrag) {
               if ((l->_0.tags&0x18C000UL)!=0UL) x->fixed = 1;
               l->_0.tags &= ~0x10000UL;
            }
            else l->_0.tags = l->_0.tags&~0x90000UL;
            x->fsum -= size;
            X2C_smallused += size;
         }
      }
      l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                size);
   }
   if ((16384UL-(((sizeof(struct xrMM_BlockDesc)+sizeof(struct X2C_LINK_STR)
                +sizeof(struct X2C_LINK_STR))-1UL)
                /sizeof(struct X2C_LINK_STR))*sizeof(struct X2C_LINK_STR))
                -x->fsum<size) {
      xrMM_free_block(x);
      return;
   }
   if (busy && x->list) xrMM_MakeFree(x);
   X2C_smallbusy += 16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR);
   ++smallBlocksTotal;
} /* end small_scan() */


static void large_scan(xrMM_Block x)
{
   X2C_LINK l;
   l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
   if (l->_.td->res!=0x093678150UL) X2C_ASSERT_F(148UL);
   if ((l->_0.tags&0x1C000UL)==0UL) {
      X2C_usedmem -= x->fsum;
      --X2C_objects;
      xrMM_free_block(x);
   }
   else {
      l->_0.tags = l->_0.tags&~0x90000UL;
      X2C_largebusy += x->fsum;
   }
} /* end large_scan() */


static void Sweep(xrMM_Block blocks[], X2C_CARD32 blocks_len,
                X2C_BOOLEAN busy, X2C_BOOLEAN defrag)
{
   X2C_CARD32 i;
   xrMM_Block n;
   xrMM_Block x;
   xrMM_Block root;
   root = blocks[0UL];
   x = root->next;
   while (x!=root) {
      n = x;
      x = x->next;
      normal_scan(n, defrag);
   }
   for (i = 1UL; i<=20UL; i++) {
      root = blocks[i];
      x = root->next;
      while (x!=root) {
         n = x;
         x = x->next;
         small_scan(n, i, busy, defrag);
      }
   } /* end for */
   root = blocks[21UL];
   x = root->next;
   while (x!=root) {
      n = x;
      x = x->next;
      large_scan(n);
   }
} /* end Sweep() */


static X2C_BOOLEAN lss(xrMM_Block x, xrMM_Block y)
{
   if (y->fixed) return 0;
   return x->fsum<=y->fsum;
} /* end lss() */


static void DefragSort(xrMM_Block b)
{
   xrMM_Block j;
   xrMM_Block i;
   xrMM_Block s;
   s = 0;
   while (b->next!=b) {
      i = b->next;
      b->next = i->next;
      if ((i->fixed || s==0) || lss(i, s)) {
         i->next = s;
         s = i;
      }
      else {
         j = s;
         while (j->next && !lss(i, j->next)) j = j->next;
         i->next = j->next;
         j->next = i;
      }
   }
   b->next = s;
   j = b;
   while (s) {
      s->prev = j;
      j = s;
      s = s->next;
   }
   b->prev = j;
   j->next = b;
} /* end DefragSort() */

static X2C_CARD32 SmallBlocksReleased;

static X2C_CARD32 SmallBlocksCompleted;


static void Defragment(xrMM_Block b)
{
   xrMM_Block last;
   xrMM_Block first;
   X2C_LINK l;
   X2C_LINK n;
   X2C_CARD32 size;
   size = (X2C_CARD32)b->root*sizeof(struct X2C_LINK_STR);
   first = b->next;
   last = b->prev;
   ++SmallBlocksCompleted;
   while (first!=last && !last->fixed) {
      l = (X2C_LINK)xrMM_getFirstBlockAdr(last);
      while (X2C_adr_lss((X2C_ADDRESS)l, last->mem)) {
         if ((l->_0.tags&0x20000UL)==0UL) {
            if (first->list==0) {
               n = (X2C_LINK)first->mem;
               first->mem = (X2C_ADDRESS)(X2C_LINK)((X2C_ADDRESS)
                n+(X2C_INT32)(X2C_INT32)size);
            }
            else {
               n = first->list;
               first->list = n->_.next;
            }
            first->fsum -= size;
            X2C_usedmem += getHpObjSize(l);
            ++X2C_objects;
            X2C_MOVE((X2C_ADDRESS)l,(X2C_ADDRESS)n,size);
            l->_0.tags |= 0x40000UL;
            l->_0.tags &= ~0x10000UL;
            l->_.next = n;
            last->fsum += size;
            if (first->fsum<size) {
               xrMM_MakeBusy(first);
               first = b->next;
               ++SmallBlocksCompleted;
            }
            if (first==last) return;
         }
         l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                size);
      }
      ++SmallBlocksReleased;
      last = last->prev;
   }
} /* end Defragment() */


static void MoveObject(X2C_LINK from, X2C_LINK to, xrMM_Block fromB,
                xrMM_Block toB, X2C_CARD32 size)
{
   X2C_MOVE((X2C_ADDRESS)from,(X2C_ADDRESS)to,size);
   xrMM_setObjOfsLen(&to, size, (X2C_ADDRESS)to-(X2C_ADDRESS)toB);
   from->_.next = to;
   from->_0.tags |= 0x40000UL;
   from->_0.tags &= ~0x10000UL;
   fromB->fsum += size;
   toB->fsum -= size;
   X2C_usedmem += size;
} /* end MoveObject() */

static X2C_CARD32 normalBlocksRest;

static X2C_CARD32 normalBlocksReleased;

static X2C_CARD32 normalBlocksMoved;

#define xrO2MM_MOVING_RATE 50


static X2C_CARD32 getNormalBusyMem(xrMM_Block b)
{
   return (16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR))-b->fsum;
} /* end getNormalBusyMem() */


static X2C_BOOLEAN obtainNewBlock(X2C_CARD32 * freemem, X2C_LINK * nl,
                xrMM_Block * newb)
{
   xrMM_NewBlock(0U, newb, 16384UL-(((sizeof(struct xrMM_BlockDesc)+sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))-1UL)/sizeof(struct X2C_LINK_STR))*sizeof(struct X2C_LINK_STR));
   if (*newb==0) return 0;
   *nl = (X2C_LINK)xrMM_getFirstBlockAdr(*newb);
   *freemem = 16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR);
   return 1;
} /* end obtainNewBlock() */


static void completeNewBlock(xrMM_Block * newb, X2C_LINK * nl,
                X2C_CARD32 freemem)
{
   if (freemem>0UL) {
      xrMM_setObjOfsLen(nl, freemem, (X2C_ADDRESS)*nl-(X2C_ADDRESS)*newb);
      stampObjAsFree(*nl, freemem);
      xrMM_foManager_add(*nl);
   }
} /* end completeNewBlock() */


static void DefragmentNormalBlocks(xrMM_Block blockList)
{
   xrMM_Block newb;
   xrMM_Block b;
   xrMM_Block ib;
   X2C_CARD32 freemem;
   X2C_CARD32 bmem;
   X2C_CARD32 rate;
   X2C_LINK nl;
   X2C_LINK l;
   X2C_CARD32 size;
   X2C_CARD32 sz;
   X2C_CARD32 X2C_maxmem_saved;
   X2C_maxmem_saved = X2C_maxmem;
   X2C_maxmem = X2C_max_longcard;
   b = blockList;
   normalBlocksRest = 0UL;
   normalBlocksReleased = 0UL;
   normalBlocksMoved = 1UL;
   if (!obtainNewBlock(&freemem, &nl, &newb)) {
      X2C_maxmem = X2C_maxmem_saved;
      return;
   }
   ib = b->next;
   while (ib!=b) {
      bmem = getNormalBusyMem(ib);
      if (bmem==0UL) {
      }
      else {
         rate = (bmem*100UL)/(16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR));
         if (!ib->fixed && rate<50UL) {
            size = 0UL;
            l = (X2C_LINK)xrMM_getFirstBlockAdr(ib);
            do {
               sz = getHpObjSize(l);
               if ((0x20000UL & l->_0.tags)==0) {
                  if (sz>freemem) {
                     ++normalBlocksMoved;
                     completeNewBlock(&newb, &nl, freemem);
                     if (!obtainNewBlock(&freemem, &nl, &newb)) {
                        X2C_maxmem = X2C_maxmem_saved;
                        return;
                     }
                  }
                  MoveObject(l, nl, ib, newb, sz);
                  nl = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                nl+(X2C_INT32)sz);
                  freemem -= sz;
               }
               size += sz;
               l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                l+(X2C_INT32)sz);
            } while (size<16384UL-(((sizeof(struct xrMM_BlockDesc)+sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))-1UL)/sizeof(struct X2C_LINK_STR))*sizeof(struct X2C_LINK_STR));
            ++normalBlocksReleased;
         }
         else ++normalBlocksRest;
      }
      ib = ib->next;
   }
   completeNewBlock(&newb, &nl, freemem);
   X2C_maxmem = X2C_maxmem_saved;
} /* end DefragmentNormalBlocks() */

#define FloatingHeaplimitFreeReserve 4194304


static void cleanBlocks(xrMM_Block blocks[], X2C_CARD32 blocks_len)
{
   X2C_CARD32 i;
   xrMM_Block x;
   xrMM_Block root;
   X2C_LINK l;
   X2C_CARD32 size;
   X2C_CARD32 sz;
   root = blocks[0UL];
   x = root->next;
   while (x!=root) {
      l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
      sz = 0UL;
      for (;;) {
         l->_0.tags &= ~0x10000UL;
         size = getHpObjSize(l);
         if (size==0UL) break;
         l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)l+(X2C_INT32)
                size);
         sz += size;
         if (sz>=16384UL-(((sizeof(struct xrMM_BlockDesc)
                +sizeof(struct X2C_LINK_STR)+sizeof(struct X2C_LINK_STR))
                -1UL)/sizeof(struct X2C_LINK_STR))
                *sizeof(struct X2C_LINK_STR)) break;
      }
      x = x->next;
   }
   for (i = 1UL; i<=20UL; i++) {
      root = blocks[i];
      x = root->next;
      size = i*sizeof(struct X2C_LINK_STR);
      while (x!=root) {
         l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
         while (X2C_adr_lss((X2C_ADDRESS)l, x->mem)) {
            l->_0.tags &= ~0x10000UL;
            l = (X2C_LINK)(X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                l+(X2C_INT32)size);
         }
         x = x->next;
      }
   } /* end for */
   root = blocks[21UL];
   x = root->next;
   while (x!=root) {
      l = (X2C_LINK)xrMM_getFirstBlockAdr(x);
      l->_0.tags &= ~0x10000UL;
      x = x->next;
   }
} /* end cleanBlocks() */


static void cleanMarkbits(void)
{
   cleanBlocks(xrMM_f_blocks, 22ul);
   cleanBlocks(xrMM_b_blocks, 22ul);
} /* end cleanMarkbits() */

static X2C_CARD32 MutatorStartTime;

static X2C_CARD32 GCStartTime;

#define DefaultMaxGCTimePercent 100


static X2C_CARD32 TruncToCard(X2C_REAL x)
{
   if (x<0.5f) return 0UL;
   else if (x>4.2949672945E+9f) return X2C_max_longcard;
   else return (X2C_CARD32)X2C_TRUNCC(x,0UL,X2C_max_longcard);
   return 0;
} /* end TruncToCard() */


static void do0(X2C_BOOLEAN defrag)
{
   Destruct l;
   Destruct d;
   marked = 0;
   tail = 0;
   MarkModules();
   if (xrMM_GCAUTO) MarkStack();
   Closure();
   d = MarkDestructors();
   if (d) Closure();
   X2C_smallused = 0UL;
   X2C_smallbusy = 0UL;
   X2C_normalused = 0UL;
   X2C_normalbusy = 0UL;
   X2C_largebusy = 0UL;
   if (xrMM_anchorTracing) printSmallAnchorsWeight();
   if (xrMM_heapTracing) printHeapTrace();
   Sweep(xrMM_f_blocks, 22ul, 0, defrag);
   Sweep(xrMM_b_blocks, 22ul, 1, defrag);
   if (xrMM_anchorTracing) {
      printLargeObjects();
      cleanMarkbits();
   }
   while (d) {
      l = d;
      d = d->next;
      l->proc(l->adr);
   }
} /* end do() */


static void COLLECT(void)
{
   X2C_BOOLEAN defrag;
   X2C_INT32 i;
   X2C_CARD32 minmaxmem;
   X2C_CARD32 freePhysMem;
   X2C_CARD32 maxmaxmem;
   X2C_CARD32 curTime;
   X2C_BOOLEAN gcTrash;
   X2C_PrepareToGC();
   curTime = TimeConv_millisecs();
   if (curTime>=MutatorStartTime) MutatorPeriod = curTime-MutatorStartTime;
   GCStartTime = curTime;
   xrMM_UsedMemAtGCStart = X2C_usedmem;
   defrag = xrMM_DoDefrag || xrMM_X2C_AlwaysDefrag;
   xrMM_DoDefrag = 0;
   smallBlocksTotal = 0UL;
   smallBlocksStackFixed = 0UL;
   smallBlocksSysbitFixed = 0UL;
   smallBlocksExpbitFixed = 0UL;
   do0(defrag);
   if (defrag) {
      SmallBlocksCompleted = 0UL;
      SmallBlocksReleased = 0UL;
      for (i = 1L; i<=20L; i++) {
         DefragSort(xrMM_f_blocks[i]);
         Defragment(xrMM_f_blocks[i]);
      } /* end for */
      DefragmentNormalBlocks(xrMM_f_blocks[0U]);
      AdjustRTStructures();
      do0(0);
   }
   curTime = TimeConv_millisecs();
   if (curTime>=GCStartTime) {
      GCPeriod = curTime-GCStartTime;
      TotalGCTime += GCPeriod;
   }
   MutatorStartTime = curTime;
   if (MutatorPeriod && X2C_max_longcard/MutatorPeriod<X2C_MaxGCTimePercent) {
      gcTrash = GCPeriod/X2C_MaxGCTimePercent>MutatorPeriod/100UL;
   }
   else gcTrash = GCPeriod>(X2C_MaxGCTimePercent*MutatorPeriod)/100UL;
   if (xrMM_FloatingHeaplimit) {
      X2C_maxmem = TruncToCard((X2C_REAL)
                (X2C_busymem+X2C_busylheap+xrMM_SizeToAlloc)*2.85f);
      maxmaxmem = 1073741824UL+X2C_largebusy;
      if (X2C_maxmem>maxmaxmem) X2C_maxmem = maxmaxmem;
      freePhysMem = X2C_GetAvailablePhysicalMemory();
      if (freePhysMem!=X2C_max_longcard) {
         maxmaxmem = TruncToCard(((X2C_REAL)freePhysMem+(X2C_REAL)
                X2C_busymem)*0.75f);
         if (X2C_busymem+xrMM_SizeToAlloc>maxmaxmem) {
            if (X2C_busymem<=X2C_max_longcard-freePhysMem) {
               maxmaxmem = X2C_busymem+freePhysMem;
            }
            else maxmaxmem = X2C_max_longcard;
            if (maxmaxmem>8388608UL) maxmaxmem -= 4194304UL;
         }
         if (X2C_maxmem>maxmaxmem) X2C_maxmem = maxmaxmem;
         if (gcTrash) {
            minmaxmem = (X2C_CARD32)X2C_TRUNCC(((X2C_REAL)
                freePhysMem+(X2C_REAL)X2C_busymem)*0.6f,0UL,
                X2C_max_longcard);
            if (X2C_maxmem<minmaxmem) {
               X2C_maxmem = minmaxmem;
               gcTrash = 0;
            }
         }
      }
      if (X2C_maxmem<X2C_busymem) X2C_maxmem = X2C_busymem;
      if (X2C_maxmem<1048576UL) X2C_maxmem = 1048576UL;
   }
   X2C_GCThrashWarning = gcTrash;
   if (X2C_threshold<X2C_maxmem) {
      if (X2C_busymem<(X2C_threshold/3UL)*2UL) {
         xrMM_GCTHRESCNT = (X2C_INT32)(X2C_threshold-X2C_busymem);
      }
      else xrMM_GCTHRESCNT = (X2C_INT32)(X2C_threshold/3UL);
   }
   xrMM_UsedMemAtGCEnd = X2C_usedmem;
   X2C_FreeAfterGC();
} /* end COLLECT() */

static void COLLECT_LOOP(void);


static void COLLECT_LOOP(void)
{
   for (;;) {
      COLLECT();
      COROUTINES_TRANSFER(&CollectPrs, CollectPrs);
   }
} /* end COLLECT_LOOP() */

static void X2C_COLLECTOR_PROC(void);


static void X2C_COLLECTOR_PROC(void)
{
   COROUTINES_TRANSFER(&CollectPrs, CollectPrs);
} /* end X2C_COLLECTOR_PROC() */

struct D;


struct D {
   X2C_ADDRESS a;
   size_t n[1];
};


static void init_dynarrs(void)
{
   X2C_CARD32 i;
   dyn_offs[0U] = X2C_BASE;
   dyn_offs[1U] = X2C_OFS_END;
   for (i = 0UL; i<=7UL; i++) {
      xrMM_ini_type_desc(&dynarrs[i], "$DYNARR", 8ul,
                sizeof(struct D)+i*2UL*4UL, (X2C_ADDRESS)dyn_offs);
   } /* end for */
} /* end init_dynarrs() */

static X2C_ADDRESS EmptyName;

static void dllAssert(void);


static void dllAssert(void)
{
   X2C_TRAP_F((X2C_INT32)X2C_unreachDLL);
} /* end dllAssert() */


static void X2C_INIT_O2MM(void)
{
   destruct = 0;
   ADR_ALIGMENT = (X2C_CARD32)X2C_adr_aligment;
   TAG_END = X2C_OFS_END;
   TAG_ARR = X2C_OFS_ARR;
   TAG_REC = X2C_OFS_REC;
   init_dynarrs();
   COROUTINES_NEWCOROUTINE(COLLECT_LOOP, (X2C_ADDRESS)prs_wsp, 21072UL,
                &CollectPrs, 0);
   xrMM_COLLECTOR = X2C_COLLECTOR_PROC;
   xrMM_O2MM_init = 1;
   X2C_FINALEXE(FinalAll);
   EmptyName = 0;
   X2C_MaxGCTimePercent = 100UL;
   MutatorStartTime = TimeConv_millisecs();
} /* end X2C_INIT_O2MM() */


static void patchTD(X2C_ADDRESS * pTD)
{
   X2C_TD p;
   p = (X2C_TD)*pTD;
   *pTD = (X2C_ADDRESS)p->self;
} /* end patchTD() */


static void patchOffsScript(OFFS pOfs)
{
   X2C_CARD32 i;
   X2C_ADDRESS a;
   i = 0UL;
   for (;;) {
      a = pOfs[i];
      if (a==X2C_OFS_END) break;
      if (a==X2C_OFS_REC) {
         ++i;
         patchTD(&pOfs[i]);
      }
      ++i;
   }
} /* end patchOffsScript() */


extern void X2C_MODULEXE(X2C_MD md, X2C_ADDRESS hmod)
{
   X2C_TD curTD;
   X2C_INT16 i;
   struct X2C_TD_STR * anonym;
   X2C_INT16 tmp;
   if (!xrMM_O2MM_init) X2C_INIT_O2MM();
   md->next = X2C_MODULES;
   X2C_MODULES = md;
   patchOffsScript((OFFS)md->offs);
   curTD = md->types;
   while (curTD) {
      { /* with */
         struct X2C_TD_STR * anonym = curTD;
         tmp = anonym->level;
         i = 0;
         if (i<=tmp) for (;; i++) {
            patchTD((X2C_ADDRESS *) &anonym->base[i]);
            if (i==tmp) break;
         } /* end for */
         patchOffsScript((OFFS)anonym->offs);
      }
      curTD = curTD->next;
   }
} /* end X2C_MODULEXE() */

typedef X2C_ADDRESS * p2A;


static X2C_CARD32 getOfsScriptLen(X2C_TD td)
{
   X2C_CARD32 l;
   p2A p;
   l = sizeof(X2C_ADDRESS);
   p = (p2A)td->offs;
   while (*p!=X2C_OFS_END) {
      l += sizeof(X2C_ADDRESS);
      p = (p2A)((X2C_ADDRESS)p+(X2C_INT32)(X2C_INT32)sizeof(X2C_ADDRESS));
   }
   return l;
} /* end getOfsScriptLen() */


static void heapCopy(X2C_ADDRESS * heapadr, X2C_ADDRESS from,
                X2C_CARD32 size)
{
   X2C_MOVE(from,*heapadr,size);
   *heapadr = (X2C_ADDRESS)((X2C_ADDRESS)*heapadr+(X2C_INT32)size);
} /* end heapCopy() */


extern void X2C_MODULEDLL(X2C_MD * component, X2C_MD md, X2C_ADDRESS hmod)
{
   X2C_CARD32 room;
   X2C_MD hostM;
   X2C_TD destTD;
   X2C_TD curTD;
   X2C_ADDRESS heapadr;
   struct X2C_TD_STR * anonym;
   if (!xrMM_O2MM_init) X2C_INIT_O2MM();
   room = sizeof(struct X2C_MD_STR);
   curTD = md->types;
   while (curTD) {
      room += sizeof(struct X2C_TD_STR)+getOfsScriptLen(curTD)+(X2C_CARD32)
                (X2C_CARD16)curTD->methods*sizeof(X2C_PROC);
      curTD = curTD->next;
   }
   heapadr = X2C_gmalloc(room);
   hostM = (X2C_MD)heapadr;
   heapCopy(&heapadr, (X2C_ADDRESS)md, sizeof(struct X2C_MD_STR));
   curTD = md->types;
   destTD = 0;
   while (curTD) {
      if (destTD==0) hostM->types = (X2C_TD)heapadr;
      else destTD->next = (X2C_TD)heapadr;
      { /* with */
         struct X2C_TD_STR * anonym = curTD;
         destTD = (X2C_TD)heapadr;
         anonym->self = destTD;
         heapCopy(&heapadr, (X2C_ADDRESS)curTD, sizeof(struct X2C_TD_STR));
         destTD->module = hostM;
         destTD->offs = (X2C_ppVOID)heapadr;
         heapCopy(&heapadr, (X2C_ADDRESS)anonym->offs,
                getOfsScriptLen(curTD));
         if (anonym->methods) {
            destTD->proc = (X2C_PROC *)heapadr;
            heapCopy(&heapadr, (X2C_ADDRESS)anonym->proc,
                (X2C_CARD32)(X2C_CARD16)anonym->methods*sizeof(X2C_PROC));
         }
      }
      curTD = curTD->next;
   }
   hostM->cnext = *component;
   *component = hostM;
   X2C_MODULEXE(hostM, hmod);
} /* end X2C_MODULEDLL() */


extern void X2C_DISABLE_COMPONENT(X2C_MD component)
{
   X2C_MD curMD;
   X2C_TD curTD;
   X2C_INT16 i;
   X2C_PROC * p;
   struct X2C_MD_STR * anonym;
   struct X2C_TD_STR * anonym0;
   X2C_INT16 tmp;
   if (!TERMINATION_IsTerminating() && !TERMINATION_HasHalted()) {
      curMD = component;
      while (curMD) {
         { /* with */
            struct X2C_MD_STR * anonym = curMD;
            anonym->name = (X2C_pCHAR) &EmptyName;
            anonym->offs = (X2C_ppVOID) &TAG_END;
            anonym->cmds = 0;
            anonym->cnms = (X2C_ppCHAR) &EmptyName;
            curTD = anonym->types;
         }
         while (curTD) {
            { /* with */
               struct X2C_TD_STR * anonym0 = curTD;
               anonym0->name = (X2C_pCHAR) &EmptyName;
               p = (X2C_PROC *)anonym0->proc;
               tmp = anonym0->methods;
               i = 1;
               if (i<=tmp) for (;; i++) {
                  *p = dllAssert;
                  p = (X2C_PROC *)((X2C_ADDRESS)p+(X2C_INT32)(X2C_INT32)
                sizeof(X2C_PROC));
                  if (i==tmp) break;
               } /* end for */
            }
            curTD = curTD->next;
         }
         curMD = curMD->cnext;
      }
   }
} /* end X2C_DISABLE_COMPONENT() */

