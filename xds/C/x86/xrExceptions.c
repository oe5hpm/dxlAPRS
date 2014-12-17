/* XDS v2.60: Copyright (c) 1999-2011 Excelsior, LLC. All Rights Reserved. */
/* "@(#)xrExceptions.c Feb  3 14:30:28 2012" */
#include "xrExceptions.h"
#define xrExceptions_C_
#include "xmRTS.h"
#include "xrsetjmp.h"
#include "X2C.h"
#include "xrtsOS.h"
#include "M2EXCEPTION.h"

#define st_normal 0

#define st_exceptional 1

#define st_off 2

#define st_reraise 3


extern void X2C_XInitHandler(X2C_XHandler x)
{
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   x->state = 0;
   x->source = 0;
   x->next = current->handler;
   X2C_HIS_SAVE(&x->history);
   current->handler = x;
} /* end X2C_XInitHandler() */


extern void X2C_XOFF(void)
{
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   current->handler->state = 2;
} /* end X2C_XOFF() */


extern void X2C_XON(void)
{
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   current->handler->state = 3;
} /* end X2C_XON() */


extern void X2C_XRETRY(void)
{
   X2C_Coroutine current;
   X2C_XHandler x;
   current = X2C_GetCurrent();
   x = current->handler;
   x->state = 0;
   X2C_HIS_RESTORE(x->history);
   x->source = 0;
   X2C_longjmp(x->buf, 1);
} /* end X2C_XRETRY() */

static struct X2C_XSource_STR sysSourceRec;

static struct X2C_XSource_STR assertSourceRec;


extern void X2C_init_exceptions(void)
{
   struct X2C_XSource_STR * anonym;
   struct X2C_XSource_STR * anonym0;
   X2C_rtsSource = &sysSourceRec;
   { /* with */
      struct X2C_XSource_STR * anonym = X2C_rtsSource;
      anonym->number = 0UL;
      anonym->message[0U] = 0;
   }
   X2C_assertSrc = &assertSourceRec;
   { /* with */
      struct X2C_XSource_STR * anonym0 = X2C_assertSrc;
      anonym0->number = 0UL;
      anonym0->message[0U] = 0;
   }
} /* end X2C_init_exceptions() */


static void dectostr(X2C_CHAR s[], X2C_CARD32 s_len, X2C_CARD32 * pos,
                X2C_CARD32 no)
{
   X2C_CARD32 i;
   X2C_CARD32 l;
   X2C_CARD32 x;
   l = 0UL;
   x = no;
   while (x>0UL) {
      x = x/10UL;
      ++l;
   }
   if (l==0UL) l = 1UL;
   *pos += l;
   i = *pos;
   while (l>0UL) {
      --i;
      s[i] = (X2C_CHAR)(48UL+no%10UL);
      no = no/10UL;
      --l;
   }
} /* end dectostr() */


static void app(X2C_CHAR d[], X2C_CARD32 d_len, X2C_CARD32 M,
                X2C_CARD32 * pos, const X2C_CHAR s[], X2C_CARD32 s_len)
{
   X2C_CARD32 i;
   i = 0UL;
   while (*pos<M && s[i]) {
      d[*pos] = s[i];
      ++*pos;
      ++i;
   }
} /* end app() */


static void form_msg(X2C_Coroutine cur, X2C_XSource source)
{
   X2C_CARD32 pos;
   pos = 0UL;
   app(cur->his_msg, 1024ul, 1024UL, &pos, "#RTS: unhandled exception #",
                28ul);
   X2C_DecToStr(cur->his_msg, &pos, source->number);
   if (source->message[0U]) {
      app(cur->his_msg, 1024ul, 1024UL, &pos, ": ", 3ul);
      app(cur->his_msg, 1024ul, 1024UL, &pos, source->message, 1024ul);
   }
   if (pos>=1024UL) pos = 1023UL;
   cur->his_msg[pos] = 0;
} /* end form_msg() */


static void doRaise(X2C_XSource source)
{
   X2C_XHandler x;
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   x = current->handler;
   while (x && x->state) x = x->next;
   current->handler = x;
   if (x==0) {
      X2C_StdOut("\n#RTS: unhandled exception #", 28UL);
      X2C_StdOutD(source->number, 0UL);
      if (source->message[0U]) {
         X2C_StdOut(": ", 2UL);
         X2C_StdOut(source->message, X2C_LENGTH(source->message,1024ul));
      }
      X2C_StdOutN();
      form_msg(current, source);
      X2C_show_history();
      X2C_StdOutFlush();
      X2C_ABORT();
   }
   else {
      x->source = source;
      x->state = 1;
   }
   X2C_HIS_RESTORE(x->history);
   X2C_longjmp(x->buf, 2);
} /* end doRaise() */


extern void X2C_doRaise(X2C_XSource source)
{
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   X2C_scanStackHistory((X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                &source-(X2C_INT32)4UL), current->stk_start, 0);
   doRaise(source);
} /* end X2C_doRaise() */


extern void X2C_XREMOVE(void)
{
   X2C_XHandler x;
   X2C_XSource s;
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   x = current->handler;
   if (x==0) X2C_TRAP_F((X2C_INT32)X2C_internalError);
   if (x->state==3) {
      s = x->source;
      current->handler = x->next;
      doRaise(s);
   }
   else current->handler = x->next;
} /* end X2C_XREMOVE() */


static void trap_message(X2C_CHAR msg[], X2C_CARD32 msg_len, X2C_CARD32 no)
{
   switch (no) {
   case 0UL:
      X2C_COPY("invalid index",14ul,msg,msg_len);
      break;
   case 1UL:
      X2C_COPY("expression out of bounds",25ul,msg,msg_len);
      break;
   case 2UL:
      X2C_COPY("invalid case in CASE statement",31ul,msg,msg_len);
      break;
   case 3UL:
      X2C_COPY("invalid location",17ul,msg,msg_len);
      break;
   case 4UL:
      X2C_COPY("function without RETURN statement",34ul,msg,msg_len);
      break;
   case 5UL:
      X2C_COPY("whole overflow",15ul,msg,msg_len);
      break;
   case 6UL:
      X2C_COPY("zero or negative divisor",25ul,msg,msg_len);
      break;
   case 7UL:
      X2C_COPY("real overflow",14ul,msg,msg_len);
      break;
   case 8UL:
      X2C_COPY("float division by zero",23ul,msg,msg_len);
      break;
   case 9UL:
      X2C_COPY("complex overflow",17ul,msg,msg_len);
      break;
   case 10UL:
      X2C_COPY("complex division by zero",25ul,msg,msg_len);
      break;
   case 11UL:
      X2C_COPY("protection error",17ul,msg,msg_len);
      break;
   case 12UL:
      X2C_COPY("SYSTEM exception",17ul,msg,msg_len);
      break;
   case 13UL:
      X2C_COPY("COROUTINE exception",20ul,msg,msg_len);
      break;
   case 14UL:
      X2C_COPY("EXCEPTIONS exception",21ul,msg,msg_len);
      break;
   default:;
      if (no==X2C_assertException) X2C_COPY("ASSERT",7ul,msg,msg_len);
      else if (no==X2C_guardException) {
         X2C_COPY("type guard check",17ul,msg,msg_len);
      }
      else if (no==X2C_noMemoryException) {
         X2C_COPY("out of heap space",18ul,msg,msg_len);
      }
      else if (no==X2C_unreachDLL) {
         X2C_COPY("call to unloaded DLL",21ul,msg,msg_len);
      }
      else if (no==X2C_internalError) {
         X2C_COPY("RTS internal error",19ul,msg,msg_len);
      }
      else if (no==X2C_castError) {
         X2C_COPY("invalid type cast",18ul,msg,msg_len);
      }
      else if (no==X2C_UserBreak) X2C_COPY("USER BREAK",11ul,msg,msg_len);
      else if (no==X2C_stack_overflow) {
         X2C_COPY("stack overflow",15ul,msg,msg_len);
      }
      else msg[0UL] = 0;
      break;
   } /* end switch */
} /* end trap_message() */


extern void X2C_TRAP_F(X2C_INT32 no)
{
   X2C_CARD32 pos;
   X2C_Coroutine current;
   struct X2C_XSource_STR * anonym;
   current = X2C_GetCurrent();
   X2C_scanStackHistory((X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                &no-(X2C_INT32)4UL), current->stk_start, 0);
   if (X2C_rtsSource==0) X2C_init_exceptions();
   trap_message(X2C_rtsSource->message, 1024ul, (X2C_CARD32)no);
   if (X2C_rtsSource->message[0U]==0) {
      { /* with */
         struct X2C_XSource_STR * anonym = X2C_rtsSource;
         X2C_COPY("TRAP(",6ul,anonym->message,1024u);
         pos = 5UL;
         dectostr(anonym->message, 1024ul, &pos, (X2C_CARD32)no);
         anonym->message[pos] = ')';
         anonym->message[pos+1UL] = 0;
      }
   }
   X2C_rtsSource->number = (X2C_CARD32)no;
   doRaise(X2C_rtsSource);
} /* end X2C_TRAP_F() */


static void append(X2C_CHAR d[], X2C_CARD32 M, X2C_CARD32 * pos,
                X2C_CHAR s[])
{
   X2C_CARD32 i;
   i = 0UL;
   while (*pos<M && s[i]) {
      d[*pos] = s[i];
      ++*pos;
      ++i;
   }
} /* end append() */


extern void X2C_TRAP_FC(X2C_INT32 no, X2C_pCHAR file, X2C_CARD32 line)
{
   X2C_CARD32 pos;
   X2C_CHAR ls[16];
   struct X2C_XSource_STR * anonym;
   if (X2C_rtsSource==0) X2C_init_exceptions();
   pos = 0UL;
   dectostr(ls, 16ul, &pos, line);
   ls[pos] = 0;
   trap_message(X2C_rtsSource->message, 1024ul, (X2C_CARD32)no);
   pos = 0UL;
   while (X2C_rtsSource->message[pos]) ++pos;
   if (X2C_rtsSource->message[0U]==0) {
      { /* with */
         struct X2C_XSource_STR * anonym = X2C_rtsSource;
         X2C_COPY("TRAP(",6ul,anonym->message,1024u);
         pos = 5UL;
         dectostr(anonym->message, 1024ul, &pos, (X2C_CARD32)no);
         anonym->message[pos] = ')';
         anonym->message[pos+1UL] = 0;
         ++pos;
      }
   }
   if (file) {
      append(X2C_rtsSource->message, 1023UL, &pos, " at line ");
      append(X2C_rtsSource->message, 1023UL, &pos, ls);
      append(X2C_rtsSource->message, 1023UL, &pos, " of ");
      append(X2C_rtsSource->message, 1023UL, &pos, file);
      if (pos>=1024UL) pos = 1023UL;
      X2C_rtsSource->message[pos] = 0;
   }
   X2C_rtsSource->number = (X2C_CARD32)no;
   doRaise(X2C_rtsSource);
} /* end X2C_TRAP_FC() */


extern void X2C_ASSERT_F(X2C_CARD32 no)
{
   X2C_CARD32 pos;
   X2C_Coroutine current;
   struct X2C_XSource_STR * anonym;
   current = X2C_GetCurrent();
   X2C_scanStackHistory((X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                &no-(X2C_INT32)4UL), current->stk_start, 0);
   if (X2C_assertSrc==0) X2C_init_exceptions();
   { /* with */
      struct X2C_XSource_STR * anonym = X2C_assertSrc;
      X2C_COPY("ASSERT(FALSE, ",15ul,anonym->message,1024u);
      pos = 14UL;
      dectostr(anonym->message, 1024ul, &pos, no);
      anonym->message[pos] = ')';
      anonym->message[pos+1UL] = 0;
      anonym->number = no;
   }
   doRaise(X2C_assertSrc);
} /* end X2C_ASSERT_F() */


extern void X2C_TRAP_G(X2C_INT32 no)
{
   X2C_CARD32 pos;
   X2C_Coroutine current;
   struct X2C_XSource_STR * anonym;
   current = X2C_GetCurrent();
   X2C_scanStackHistory((X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                &no-(X2C_INT32)4UL), current->stk_start, 1);
   if (X2C_rtsSource==0) X2C_init_exceptions();
   trap_message(X2C_rtsSource->message, 1024ul, (X2C_CARD32)no);
   if (X2C_rtsSource->message[0U]==0) {
      { /* with */
         struct X2C_XSource_STR * anonym = X2C_rtsSource;
         X2C_COPY("TRAP(",6ul,anonym->message,1024u);
         pos = 5UL;
         dectostr(anonym->message, 1024ul, &pos, (X2C_CARD32)no);
         anonym->message[pos] = ')';
         anonym->message[pos+1UL] = 0;
      }
   }
   X2C_rtsSource->number = (X2C_CARD32)no;
   doRaise(X2C_rtsSource);
} /* end X2C_TRAP_G() */


extern void X2C_ASSERT_FC(X2C_CARD32 code, X2C_pCHAR file, X2C_CARD32 line)
{
   X2C_CARD32 pos;
   X2C_CHAR ls[16];
   X2C_CHAR cs[16];
   X2C_Coroutine current;
   current = X2C_GetCurrent();
   X2C_scanStackHistory((X2C_ADDRESS)((X2C_ADDRESS)(X2C_ADDRESS)
                &code-(X2C_INT32)4UL), current->stk_start, 0);
   if (X2C_assertSrc==0) X2C_init_exceptions();
   pos = 0UL;
   dectostr(cs, 16ul, &pos, code);
   cs[pos] = 0;
   pos = 0UL;
   dectostr(ls, 16ul, &pos, line);
   ls[pos] = 0;
   pos = 0UL;
   append(X2C_assertSrc->message, 1023UL, &pos, "ASSERT(FALSE,");
   append(X2C_assertSrc->message, 1023UL, &pos, cs);
   append(X2C_assertSrc->message, 1023UL, &pos, ") at line ");
   append(X2C_assertSrc->message, 1023UL, &pos, ls);
   append(X2C_assertSrc->message, 1023UL, &pos, " of ");
   append(X2C_assertSrc->message, 1023UL, &pos, file);
   if (pos>=1024UL) pos = 1023UL;
   X2C_assertSrc->message[pos] = 0;
   X2C_assertSrc->number = code;
   doRaise(X2C_assertSrc);
} /* end X2C_ASSERT_FC() */

