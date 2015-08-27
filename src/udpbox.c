/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef X2C_H_
#include "X2C.h"
#endif
#define udpbox_C_
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef Lib_H_
#include "Lib.h"
#endif
#ifndef InOut_H_
#include "InOut.h"
#endif
#ifndef FileSys_H_
#include "FileSys.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif
#ifndef Storage_H_
#include "Storage.h"
#endif
#ifndef TimeConv_H_
#include "TimeConv.h"
#endif
#ifndef aprspos_H_
#include "aprspos.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif

/* axudp in/out ui/aprs stream splitter, filter, digipeater by OE5DXL */
/*FROM FdSet IMPORT FdSet, FD_ZERO, FD_SET, FD_ISSET, TimeVal;*/
/*FROM mlib IMPORT select*/
#define udpbox_CALLLEN 7

#define udpbox_HASHSIZE 16384

#define udpbox_NOTFIRSTDIGI 3

#define udpbox_NOUPLINKCHECK 4

#define udpbox_SENDTRACE 5

#define udpbox_SENDWIDE 6

#define udpbox_DOWNPATHDIGICALL 7

#define udpbox_DOWNPATHRELAY 8

#define udpbox_DOWNPATHWIDE 9

#define udpbox_NOVIADIGICALL 10

#define udpbox_NOVIARELAY 11

#define udpbox_NOVIATRACE 12

#define udpbox_NOVIAWIDEN 13

#define udpbox_NOCALLBEFOREWIDE 14

#define udpbox_SSIDTOWIDE 15

#define udpbox_NULLSSID 16

#define udpbox_KILLWIDE 17

#define udpbox_VIAGATE 18

#define udpbox_RELAYECHO 19

#define udpbox_FINGERPRINT 20

#define udpbox_STDIN 0

#define udpbox_STDINIP 0x0FFFFFFFF

typedef unsigned long SET256[8];

typedef char RAWCALL[7];

typedef char MONCALL[9];

typedef char FILENAME[1024];

struct DIGIPARMS;

typedef struct DIGIPARMS * pDIGIPARMS;


struct DIGIPARMS {
   RAWCALL digicall;
   unsigned long pathcheck;
   unsigned long duptime;
   unsigned long messagetime;
   unsigned long timehash[16384];
};

struct BEACON;


struct BEACON {
   unsigned long bintervall;
   unsigned long piggytime; /* time send beacons earlier if sending some else */
   char piggyback; /* sent something so append beacon immediately */
   FILENAME bfile;
   unsigned long btime;
   unsigned long bline;
};

struct CALLS;

typedef struct CALLS * pCALLS;


struct CALLS {
   pCALLS next;
   RAWCALL call;
};

struct OUTPORT;

typedef struct OUTPORT * pOUTPORT;


struct OUTPORT {
   pOUTPORT next;
   unsigned long toip;
   unsigned long toport;
   pCALLS filtercalls;
   char echo;
   char rawwrite;
   char crlfwrite;
   char passnoUI;
   char axudp2;
   SET256 aprspass;
   pDIGIPARMS digiparm;
   struct BEACON beacon0;
   struct aprspos_POSITION mypos;
   long maxkm;
};

struct INSOCK;

typedef struct INSOCK * pINSOCK;


struct INSOCK {
   pINSOCK next;
   long fd;
   char rawread;
   MONCALL rflinkname;
   unsigned long fromip;
   unsigned long bindport;
   pOUTPORT outchain;
};

struct MSGHASH;

typedef struct MSGHASH * pMSGHASH;

struct _0;


struct _0 {
   unsigned long sums;
   unsigned long ackcnt;
   unsigned long acktime;
   pINSOCK source;
   MONCALL froms;
   char acks[6];
};


struct MSGHASH {
   pMSGHASH next;
   MONCALL usercall;
   FILENAME msgfile;
   unsigned long wpos;
   struct _0 hash[16];
};

#define udpbox_MAXLEN 338

#define udpbox_MINLEN 17

static char udpbox_BLANKH = '@';

#define udpbox_HBIT 128

#define udpbox_ACKMSG "ack"

static unsigned long udpbox_POLYNOM = 0x8408UL;

#define udpbox_cFILTPASS "p"

#define udpbox_cFILTDEL "d"

#define udpbox_DEFAULTMESSAGETIME 28
/* block seconds user message frames */

#define udpbox_DEFAULTDUPTIME 1740
/* block seconds invariant payload frames */

#define udpbox_CR "\015"

#define udpbox_LF "\012"

#define udpbox_SENDACKS 4

#define udpbox_OLDMSG 600
/* s long kill dupe user message */

#define udpbox_TOCALL "APNL01"

#define udpbox_DEFAULTACK "WIDE2-2"

#define udpbox_cUSERMSG ":"

#define udpbox_cTELEMETRY "T"

static char show;

static pINSOCK insocks;

static unsigned long showip;

static unsigned long showport;

static pMSGHASH msgusers;

static char ackpath[64];

static unsigned long stdinpos;

static char stdinbuf[256];


static void Err(const char text[], unsigned long text_len)
{
   InOut_WriteString("udpbox: ", 9ul);
   InOut_WriteString(text, text_len);
   osi_WrStrLn(" error abort", 13ul);
   X2C_ABORT();
} /* end Err() */


static void Stomsg(pMSGHASH user, const char from[], unsigned long from_len,
                const char to[], unsigned long to_len, const char msg[],
                unsigned long msg_len, const char ack[],
                unsigned long ack_len, pINSOCK fromsock)
{
   unsigned long h;
   unsigned long i;
   unsigned long t;
   long f;
   char mb[256];
   struct MSGHASH * anonym;
   struct _0 * anonym0;
   { /* with */
      struct MSGHASH * anonym = user;
      i = 0UL;
      while (i<=msg_len-1 && msg[i]) ++i;
      t = TimeConv_time();
      h = aprsstr_Hash(msg, msg_len, 0L, (long)i)&16383UL;
      i = 0UL;
      while (i<=15UL && !(((h==anonym->hash[i].sums && anonym->hash[i]
                .acktime+600UL>=t) && aprsstr_StrCmp(ack, ack_len,
                anonym->hash[i].acks, 6ul)) && aprsstr_StrCmp(from, from_len,
                 anonym->hash[i].froms, 9ul))) ++i;
      if (i>15UL) {
         /* new msg*/
         aprsstr_DateToStr(t, mb, 256ul);
         aprsstr_Append(mb, 256ul, " from ", 7ul);
         aprsstr_Append(mb, 256ul, from, from_len);
         aprsstr_Append(mb, 256ul, " to ", 5ul);
         aprsstr_Append(mb, 256ul, to, to_len);
         aprsstr_Append(mb, 256ul, " <", 3ul);
         aprsstr_Append(mb, 256ul, msg, msg_len);
         aprsstr_Append(mb, 256ul, "> (", 4ul);
         aprsstr_Append(mb, 256ul, ack, ack_len);
         aprsstr_Append(mb, 256ul, ")", 2ul);
         if (show) {
            InOut_WriteString(" <", 3ul);
            InOut_WriteString(mb, 256ul);
            InOut_WriteString("> ", 3ul);
         }
         aprsstr_Append(mb, 256ul, "\012", 2ul);
         if (anonym->msgfile[0U]) {
            f = osi_OpenAppend(anonym->msgfile, 1024ul);
            if (f<0L) f = osi_OpenWrite(anonym->msgfile, 1024ul);
            if (f>=0L) {
               osi_WrBin(f, (char *)mb, 256u/1u, aprsstr_Length(mb, 256ul));
               osi_Close(f);
            }
         }
         { /* with */
            struct _0 * anonym0 = &anonym->hash[anonym->wpos];
            anonym0->sums = h;
            aprsstr_Assign(anonym0->acks, 6ul, ack, ack_len);
            aprsstr_Assign(anonym0->froms, 9ul, from, from_len);
            if ((unsigned char)ack[0UL]>' ') {
               anonym0->ackcnt = 4UL;
               anonym0->acktime = 0UL;
            }
            anonym0->source = fromsock;
         }
         ++anonym->wpos;
         if (anonym->wpos>15UL) anonym->wpos = 0UL;
      }
      else {
         /* old msg */
         anonym->hash[i].ackcnt = 4UL;
      }
   }
} /* end Stomsg() */

#define udpbox_THIRDPARTY "}"

#define udpbox_HEADEREND ":"

#define udpbox_FROMEND ">"

#define udpbox_USERACK "{"


static char Usermsg(const char b[], unsigned long b_len, long len, long p,
                pINSOCK fromsock, char * selfmsg)
{
   long pf;
   unsigned long j;
   pMSGHASH user;
   char to[9];
   char from[9];
   char ack[6];
   char msg[69];
   char ok0;
   *selfmsg = 0;
   pf = 0L; /* from whom */
   while (b[p]=='}') {
      /* remove third party header */
      pf = p+1L;
      while (p<len && b[p]!=':') ++p;
      ++p;
   }
   if ((p+10L<len && b[p]==':') && b[p+10L]==':') {
      /* user message */
      ++p;
      j = 0UL; /* message from */
      if (pf==0L) ok0 = aprsstr_Call2Str(b, b_len, from, 9ul, 7UL, &j);
      else {
         while ((pf<len && j<=8UL) && b[pf]!='>') {
            from[j] = b[pf];
            ++pf;
            ++j;
         }
      }
      while (j<=8UL) {
         from[j] = ' ';
         ++j;
      }
      ok0 = 0;
      for (j = 0UL; j<=8UL; j++) {
         /* message to */
         to[j] = b[p+(long)j];
         if (to[j]!=from[j]) ok0 = 1;
      } /* end for */
      if (!ok0) {
         /* msg from to same call */
         if (show) InOut_WriteString(" msg to it self ", 17ul);
         *selfmsg = 1;
         return 0;
      }
      /*WrStr("<"); WrStr(from); WrStr(">"); WrStr("<"); WrStr(to);
                WrStrLn(">"); */
      user = msgusers;
      while (user) {
         j = 0UL;
         while (j<=8UL && user->usercall[j]==to[j]) ++j;
         if (j==9UL) {
            /*
                    j:=0;
                    IF pf=0 THEN
                      IF Call2Str(b, from, 7, j) THEN END;
                    ELSE 
                      WHILE (pf<len) & (j<=HIGH(from)) & (b[pf]<>FROMEND)
                DO from[j]:=b[pf]; INC(pf); INC(j); END;
                    END;
                    WHILE j<=HIGH(from) DO from[j]:=" "; INC(j) END; 
            
                    FOR j:=0 TO 8 DO to[j]:=b[p]; INC(p) END;
            */
            p += 10L;
            j = 0UL;
            while ((p<len && j<68UL) && b[p]!='{') {
               msg[j] = b[p];
               ++j;
               ++p;
            }
            msg[j] = 0;
            j = 0UL;
            for (;;) {
               if ((unsigned char)"ack"[j]<=' ') return 0;
               /* "ack" in text field is not for delete */
               if ("ack"[j]!=msg[j]) break;
               ++j;
            }
            ++p;
            j = 0UL;
            while (p<len && j<5UL) {
               ack[j] = b[p];
               ++j;
               ++p;
            }
            ack[j] = 0;
            Stomsg(user, from, 9ul, to, 9ul, msg, 69ul, ack, 6ul, fromsock);
            return 1;
         }
         user = user->next;
      }
   }
   return 0;
} /* end Usermsg() */

static char udpbox_WILDH = 'T';


static char CallFilt(pCALLS calls, const char s[], unsigned long s_len,
                long len)
{
   long j;
   long i;
   pCALLS c;
   char b;
   char ok0;
   i = 0L;
   do {
      c = calls;
      while (c) {
         j = 0L;
         for (;;) {
            if (c->call[j]!='T' && c->call[j]!=s[i+j]) break;
            ++j;
            if (j>=6L) {
               if (c->call[j] && ((unsigned char)(unsigned char)
                c->call[j]&0x1EU)!=((unsigned char)(unsigned char)
                s[i+j]&0x1EU)) break;
               return 1;
            }
         }
         c = c->next;
      }
      i += 7L;
   } while (!(i>=len || ((unsigned long)(unsigned char)s[i-1L]&1)));
   i += 2L;
   if (s[i]=='}') {
      ++i;
      for (;;) {
         if (i>=len) break;
         c = calls;
         while (c) {
            j = 0L;
            ok0 = 1;
            for (;;) {
               b = s[i+j];
               if (((((i+j>=len || b=='*') || b==',') || b=='>') || b==':')
                || b=='-') break;
               if (j>=6L || c->call[j]!='T' && b!=(char)((unsigned long)
                (unsigned char)c->call[j]>>1)) ok0 = 0;
               ++j;
            }
            if (ok0 && (j>=6L || c->call[j]=='@')) return 1;
            c = c->next;
         }
         while (((i<len && s[i]!=',') && s[i]!='>') && s[i]!=':') ++i;
         if (s[i]==':') {
            ++i;
            if (s[i]!='}') break;
         }
         ++i;
      }
   }
   return 0;
} /* end CallFilt() */


static long DistFilt(struct aprspos_POSITION mypos, char b[],
                unsigned long b_len, unsigned long payload,
                unsigned long len)
{
   unsigned long course;
   unsigned long speed;
   unsigned long i;
   char comment0[501];
   long alt;
   char postyp;
   char symt;
   char sym;
   struct aprspos_POSITION pos;
   long DistFilt_ret;
   X2C_PCOPY((void **)&b,b_len);
   for (i = 6UL;; i--) {
      b[i+1UL] = (char)X2C_LSH((unsigned char)(unsigned char)b[i],8,-1);
      if (i==0UL) break;
   } /* end for */
   b[len] = 0;
   aprspos_GetPos(&pos, &speed, &course, &alt, &sym, &symt, b, b_len, 1UL,
                payload, comment0, 501ul, &postyp);
   if (aprspos_posvalid(pos)) {
      DistFilt_ret = (long)(unsigned long)X2C_TRUNCC(aprspos_distance(mypos,
                pos),0UL,X2C_max_longcard);
   }
   else DistFilt_ret = -1L;
   X2C_PFREE(b);
   return DistFilt_ret;
} /* end DistFilt() */


static char Filter(const char b[], unsigned long b_len, long len,
                pOUTPORT parm, pINSOCK fromsock)
{
   long i;
   long km;
   char selfmsg;
   len -= 2L; /* crc */
   if (len<=0L) {
      if (parm->passnoUI) {
         if (show) osi_WrStrLn(" pass axudp2", 13ul);
         return 1;
      }
      else return 0;
   }
   i = 13L;
   while (i<len && !((unsigned long)(unsigned char)b[i]&1)) i += 7L;
   i += 3L;
   if (i>len || ((unsigned char)(unsigned char)b[i-2L]&~0x10U)!=0x3U) {
      if (show) InOut_WriteString(" not UI ", 9ul);
      return parm->passnoUI;
   }
   /* not UI frame */
   /* is UI frame */
   if (Usermsg(b, b_len, len, i, fromsock, &selfmsg)) return 0;
   if (parm->filtercalls && CallFilt(parm->filtercalls, b, b_len, len)) {
      if (show) InOut_WriteString(" callfilter match", 18ul);
      return 0;
   }
   if (!X2C_INL((unsigned char)b[i],256,
                parm->aprspass) || selfmsg && !X2C_INL((long)84,256,
                parm->aprspass)) {
      if (show) InOut_WriteString(" message type filter", 21ul);
      return 0;
   }
   if (parm->maxkm>0L && (b[i]!=':' || b[i+10L]!=':')) {
      /* dist filter on and no user message */
      km = DistFilt(parm->mypos, b, b_len, (unsigned long)i,
                (unsigned long)len);
      if (km<0L) {
         if (show) InOut_WriteString(" no pos", 8ul);
         return 0;
      }
      if (show) {
         InOut_WriteString(" ", 2ul);
         InOut_WriteInt(km, 1UL);
         InOut_WriteString("km", 3ul);
      }
      if (km>=parm->maxkm) {
         if (show) InOut_WriteString(" too far", 9ul);
         return 0;
      }
   }
   return 1;
} /* end Filter() */


static long getstdin(char buf[], unsigned long buf_len)
{
   char c;
   while (osi_RdBin(0L, (char *) &c, 1u/1u, 1UL)==1L) {
      if (c=='\012') c = 0;
      if (stdinpos<255UL) {
         stdinbuf[stdinpos] = c;
         ++stdinpos;
      }
      if (c==0) {
         aprsstr_Assign(buf, buf_len, stdinbuf, 256ul);
         stdinpos = 0UL;
         return (long)aprsstr_Length(buf, buf_len);
      }
   }
   return -1L;
} /* end getstdin() */


static long getudp(long fd, char buf[], unsigned long buf_len,
                unsigned long fromip, char addcrc)
{
   unsigned long fromport;
   unsigned long ip;
   char crc2;
   char crc1;
   long len;
   len = udpreceive(fd, buf, (long)(buf_len), &fromport, &ip);
   if ((len>2L && len<(long)(buf_len)) && (fromip==0UL || fromip==ip)) {
      showip = ip;
      showport = fromport;
      if (!addcrc) return len;
      crc1 = buf[len-2L];
      crc2 = buf[len-1L];
      aprsstr_AppCRC(buf, buf_len, len-2L);
      if (crc1==buf[len-2L] && crc2==buf[len-1L]) return len;
      if (show) osi_WrStrLn(" raw crc error ", 16ul);
   }
   /*
         IO.WrHex(ORD(crc1)+ORD(crc2)*256, 1);
         IO.WrStr("<>");
         IO.WrHex(ORD(buf[len-2])+ORD(buf[len-1])*256, 1);
         IO.WrStr("  ");
   */
   return -1L;
} /* end getudp() */


static void checkhamnet(char b[], unsigned long b_len, long * len,
                const char rflinkname[], unsigned long rflinkname_len,
                RAWCALL hamup0)
{
   long j;
   long i;
   unsigned long s;
   long tmp;
   if (rflinkname[0UL]==0) return;
   hamup0[0U] = 0;
   i = 0L;
   while (b[i]!='>') {
      /* skip fromcall */
      ++i;
      if (i>=*len) {
         *len = 0L;
         return;
      }
   }
   ++i;
   j = 0L;
   while (j<=(long)(rflinkname_len-1) && rflinkname[j]) {
      /* compare rflink name */
      if (b[i]!=rflinkname[j]) {
         *len = 0L;
         return;
      }
      ++i;
      ++j;
   }
   if (b[i]==',') {
      ++i;
      j = 0L;
      while (((i<*len && b[i]!='-') && b[i]!=',') && b[i]!=':') {
         /* make rawcall of rflink uplink */
         s = (unsigned long)(unsigned char)b[i]*2UL&255UL;
         if (s<=64UL) {
            *len = 0L;
            return;
         }
         hamup0[j] = (char)s;
         ++i;
         ++j;
         if (j>=7L) {
            *len = 0L;
            return;
         }
      }
      while (j<6L) {
         hamup0[j] = '@';
         ++j;
      }
      s = 0UL;
      if (b[i]=='-') {
         ++i;
         while ((unsigned char)b[i]>='0' && (unsigned char)b[i]<='9') {
            s = (s*10UL+(unsigned long)(unsigned char)b[i])-48UL;
            ++i;
         }
         if (s>15UL) {
            *len = 0L;
            return;
         }
      }
      hamup0[j] = (char)((s+48UL+64UL)*2UL); /* set H bit */
   }
   if (b[i]!=',' && b[i]!=':') {
      *len = 0L;
      return;
   }
   while (b[i]!='}') {
      /* find end of rflink head */
      ++i;
      if (i>=*len) {
         *len = 0L;
         return;
      }
   }
   ++i;
   *len -= i; /* payload len */
   tmp = *len;
   j = 0L;
   if (j<=tmp) for (;; j++) {
      b[j] = b[i]; /* remove rflink head */
      ++i;
      if (j==tmp) break;
   } /* end for */
/* DIGICALL>HAMNAME,UPLINKCALL,.... :}payload */
} /* end checkhamnet() */

/*
PROCEDURE MakeDupeTime(s-:ARRAY OF CHAR; p:INTEGER; parm:pDIGIPARMS):TIME;
VAR le,nu:CARDINAL;
    i:INTEGER; 
BEGIN
  le:=0;
  nu:=0;
  FOR i:=p+3 TO p+5 DO 
    IF (s[i]>="A") & (s[i]<="Z") THEN INC(le) END;
    IF (s[i]>="0") & (s[i]<="9") THEN INC(nu) END;
  END;
  IF (s[p+2]=cUSERMSG) & (le>0) & (nu>0) & (le+nu=3)
                THEN        (* looks like a user message *)
    RETURN parm^.messagetime
  END;
  RETURN parm^.duptime
END MakeDupeTime;
*/

static char IsCall(const char raw[], unsigned long raw_len,
                unsigned long pos, unsigned long div0)
{
   unsigned long i;
   unsigned long nu;
   unsigned long le;
   char c;
   unsigned long tmp;
   le = 0UL;
   nu = 0UL;
   tmp = pos+2UL;
   i = pos;
   if (i<=tmp) for (;; i++) {
      c = (char)((unsigned long)(unsigned char)raw[i]/div0);
      if ((unsigned char)c>='A' && (unsigned char)c<='Z') ++le;
      if ((unsigned char)c>='0' && (unsigned char)c<='9') ++nu;
      if (i==tmp) break;
   } /* end for */
   return (le>0UL && nu>0UL) && le+nu==3UL;
/* looks like a call */
} /* end IsCall() */


static char Dup(const char raw[], unsigned long raw_len,
                unsigned long pathlen, unsigned long rawlen,
                unsigned long * sum)
{
   unsigned long i;
   unsigned long len;
   unsigned char hashh;
   unsigned char hashl;
   char h[256];
   char ok0;
   /*
     PROCEDURE hash(c:CHAR);
     VAR b:CARD8;
     BEGIN
   (*
   IO.WrStr("<");IO.WrChar(c);IO.WrStr(">");
   *)
       IF c<>" " THEN
         b:=CAST(CARD8, CAST(SET8, c) / hashl);
         hashl:=CRCL[b] / hashh;
         hashh:=CRCH[b];
       END;
     END hash;
   */
   hashl = 0U;
   hashh = 0U;
   pathlen += 2UL; /* skip frametyp pid */
   if (raw[pathlen]!='}') {
      len = 0UL;
      ok0 = aprsstr_Call2Str(raw, raw_len, h, 256ul, 7UL, &len);
      i = 0UL;
      while (i<len) {
         aprsstr_HashCh(h[i], &hashl, &hashh);
         ++i;
      }
      len = 0UL;
      ok0 = aprsstr_Call2Str(raw, raw_len, h, 256ul, 0UL, &len);
      i = 0UL;
      while (i<len && h[i]!='-') {
         aprsstr_HashCh(h[i], &hashl, &hashh);
         ++i;
      }
   }
   else {
      i = pathlen+1UL;
      for (;;) {
         ++pathlen;
         if (pathlen>=rawlen) break;
         if (raw[pathlen-1UL]==':') {
            if (raw[pathlen]=='}') i = pathlen+1UL;
            else break;
         }
      }
      while (i<pathlen-1UL && raw[i]!='>') {
         aprsstr_HashCh(raw[i], &hashl, &hashh);
         ++i;
      }
      ++i;
      while ((i<pathlen-1UL && raw[i]!='-') && raw[i]!=',') {
         aprsstr_HashCh(raw[i], &hashl, &hashh);
         ++i;
      }
   }
   i = pathlen;
   while ((i<rawlen && raw[i]!='\015') && raw[i]!='\012') {
      aprsstr_HashCh(raw[i], &hashl, &hashh);
      ++i;
   }
   *sum = (unsigned long)(unsigned char)(char)hashl+(unsigned long)
                (unsigned char)(char)hashh*256UL&16383UL;
   if (raw[pathlen]!=':') return 0;
   return IsCall(raw, raw_len, pathlen+1UL, 1UL);
/* if call it is user msg */
} /* end Dup() */


static char Cmp(const char s[], unsigned long s_len, unsigned long start,
                const char word[], unsigned long word_len)
{
   unsigned long i;
   i = 0UL;
   while (word[i]) {
      if (word[i]!=(char)((unsigned long)(unsigned char)s[start]/2UL)) {
         return 0;
      }
      ++i;
      ++start;
   }
   return 1;
} /* end Cmp() */


static char ChkNN(unsigned long ssid, char c)
{
   c = (char)((unsigned long)(unsigned char)c/2UL);
   return c==' ' && ssid==0UL || ((unsigned char)c>='1' && (unsigned char)
                c<='7') && (unsigned char)c>=(unsigned char)(char)(48UL+ssid)
                ;
} /* end ChkNN() */


static char NeqN(unsigned long ssid, char c, long pos)
{
   return pos<=14L && (ssid+48UL==(unsigned long)(unsigned char)
                c/2UL || ssid==0UL && c=='@');
} /* end NeqN() */


static void setSSID(char * c, unsigned long ssid)
{
   *c = (char)((unsigned char)(unsigned char)*c&~0x1EU|(unsigned char)
                (ssid*2UL));
} /* end setSSID() */


static unsigned long getSSID(char c)
{
   return (unsigned long)(unsigned char)c/2UL&15UL;
} /* end getSSID() */


static void killwide(long * startpath, long downpath, char s[],
                unsigned long s_len)
{
   long j;
   long i;
   long tmp;
   long tmp0;
   tmp = downpath-1L;
   i = *startpath;
   if (i<=tmp) for (tmp = (unsigned long)(tmp-i)/7L;;) {
      if (((s[i]=='\256' && s[i+1L]=='\222') && s[i+2L]=='\210')
                && s[i+3L]=='\212') {
         tmp0 = *startpath;
         j = i-1L;
         if (j>=tmp0) for (;; j--) {
            s[j+7L] = s[j];
            if (j==tmp0) break;
         } /* end for */
         *startpath += 7L;
      }
      if (!tmp) break;
      --tmp;
      i += 7L;
   } /* end for */
} /* end killwide() */


static void Digi(char raw[], unsigned long raw_len, char send[],
                unsigned long send_len, long inlen0, long * outlen0,
                const RAWCALL hamup0, pDIGIPARMS parm, char duponly)
{
   long goodpath;
   long downpath;
   long actdigi;
   long startpath;
   long pathlen;
   long j;
   long i;
   unsigned long ssidroute;
   unsigned long hash;
   unsigned long ssid;
   unsigned long tt;
   unsigned long t;
   char noloop;
   char nodigicall;
   char ok0;
   long tmp;
   if ((unsigned char)parm->digicall[0U]<='@') duponly = 1;
   *outlen0 = 0L;
   inlen0 -= 2L; /* crc bytes */
   pathlen = 13L;
   while (pathlen<inlen0 && !((unsigned long)(unsigned char)raw[pathlen]&1)) {
      pathlen += 7L; /* find end of path */
   }
   ++pathlen;
   ok0 = 0;
   downpath = 0L;
   if (!duponly) {
      if ((0x100000UL & parm->pathcheck) || (unsigned char)
                raw[6UL]>=(unsigned char)'\200'!=(unsigned char)
                raw[13UL]>=(unsigned char)'\200') {
         actdigi = 14L;
         noloop = 1;
         if ((0x8UL & parm->pathcheck)) {
            /* need not be first repeater */
            while (actdigi<pathlen && (unsigned char)
                raw[actdigi+6L]>=(unsigned char)'\200') {
               /* test if own digicall in path */
               ok0 = 0;
               for (i = 0L; i<=5L; i++) {
                  if (raw[i+actdigi]!=parm->digicall[i]) ok0 = 1;
               } /* end for */
               if (getSSID(raw[(actdigi+7L)-1L])!=getSSID(parm->digicall[6U])
                ) ok0 = 1;
               if (!ok0) noloop = 0;
               actdigi += 7L;
            }
         }
         if (noloop) {
            /* not looping thru own digi */
            startpath = actdigi+7L;
            nodigicall = 0;
            if ((0x80UL & parm->pathcheck)) downpath = pathlen;
            /* test for ssid routing*/
            ssid = (unsigned long)(long)(parm->pathcheck&0x7UL);
                /* limit ssid routing hopps */
            ssidroute = getSSID(raw[6UL]); /* destination call ssid */
            goodpath = 14L;
            while ((goodpath<pathlen && (unsigned char)
                raw[goodpath+6L]>=(unsigned char)'\200') && IsCall(raw,
                raw_len, (unsigned long)goodpath, 2UL)) {
               goodpath += 7L; /* repeated via callsigns */
            }
            if ((((ssidroute>0UL && ssid>0UL) && goodpath==pathlen)
                && pathlen<49L) && ((0x8UL & parm->pathcheck)
                || goodpath==14L)) {
               ok0 = 1;
               if (ssidroute>7UL) ssidroute = 1UL;
               if (ssidroute>ssid) ssidroute = ssid;
               if (ssidroute==1UL || (0x8000UL & parm->pathcheck)==0) {
                  /* use decrement dest ssid routing */
                  setSSID(&raw[6UL], ssidroute-1UL);
                /* chain by dec dest ssid */
                  ssidroute = 0UL;
               }
               else {
                  /* expand to digicall + wideN-N */
                  --ssidroute;
                  setSSID(&raw[6UL], 0UL); /* switch to via routing */
               }
               downpath = 0L; /* delete rest of path */
               if (show) InOut_WriteString(" via ssid routing ", 19ul);
            }
            else {
               ssidroute = 0UL;
               if (goodpath<=14L) goodpath = 0L;
               if ((0x10UL & parm->pathcheck)) goodpath = actdigi;
               ok0 = pathlen>actdigi && (unsigned char)
                raw[actdigi+6L]<(unsigned char)'\200';
                /* there are via calls & is not digipeated */
               if (ok0) {
                  ssid = getSSID(raw[actdigi+6L]);
                  if ((0x400UL & parm->pathcheck)) ok0 = 0;
                  else {
                     for (i = 0L; i<=5L; i++) {
                        if (raw[i+actdigi]!=parm->digicall[i]) ok0 = 0;
                     } /* end for */
                     if (ssid!=getSSID(parm->digicall[6U])) ok0 = 0;
                  }
                  if (ok0) {
                     if ((0x80UL & parm->pathcheck)) downpath = pathlen;
                     if (show) InOut_WriteString(" via digicall ", 15ul);
                  }
                  else if ((0x800UL & parm->pathcheck)==0) {
                     ok0 = Cmp(raw, raw_len, (unsigned long)actdigi,
                "RELAY ", 7ul);
                     if (!ok0 && (0x80000UL & parm->pathcheck)) {
                        ok0 = Cmp(raw, raw_len, (unsigned long)actdigi, "ECHO\
  ", 7ul);
                     }
                     if (ok0) {
                        if ((0x100UL & parm->pathcheck)) {
                           downpath = pathlen;
                        }
                        if (show) {
                           InOut_WriteString(" via RELAY ", 12ul);
                        }
                     }
                  }
                  if (!ok0) {
                     if ((0x40000UL & parm->pathcheck)) {
                        ok0 = Cmp(raw, raw_len, (unsigned long)actdigi, "GATE\
  ", 7ul);
                        if (ok0) {
                           if ((0x100UL & parm->pathcheck)) {
                              downpath = pathlen;
                           }
                           if (show) {
                              InOut_WriteString(" via GATE ", 11ul);
                           }
                        }
                     }
                  }
                  if (!ok0) {
                     if ((((0x1000UL & parm->pathcheck)==0 && Cmp(raw,
                raw_len, (unsigned long)actdigi, "TRACE",
                6ul)) && ChkNN(ssid,
                raw[actdigi+5L])) && (goodpath==actdigi || NeqN(ssid,
                raw[actdigi+5L], actdigi))) {
                        ok0 = 1;
                        if (ssid>(unsigned long)
                ((0x10000UL & parm->pathcheck)==0)
                && (0x20UL & parm->pathcheck)) {
                           setSSID(&raw[actdigi+6L], ssid-1UL);
                /* dec(N) of WIDEn-N */
                           startpath = actdigi;
                        }
                        if ((0x100UL & parm->pathcheck)==0) {
                           downpath = actdigi+7L;
                        }
                     }
                     if (show && ok0) {
                        InOut_WriteString(" via TRACEn-n ", 15ul);
                     }
                  }
                  if (!ok0) {
                     if (((((0x2000UL & parm->pathcheck)==0 && Cmp(raw,
                raw_len, (unsigned long)actdigi, "WIDE",
                5ul)) && raw[actdigi+5L]=='@') && ChkNN(ssid,
                raw[actdigi+4L])) && (goodpath==actdigi || NeqN(ssid,
                raw[actdigi+4L], actdigi))) {
                        ok0 = 1;
                        if (raw[actdigi+4L]
                !='@' && (0x40UL & parm->pathcheck)) {
                           if ((0x4000UL & parm->pathcheck)
                || goodpath!=actdigi && !NeqN(ssid, raw[actdigi+4L],
                actdigi)) {
                              nodigicall = 1; /* maybe repeatet before */
                           }
                           if (ssid>(unsigned long)
                ((0x10000UL & parm->pathcheck)==0)) {
                              setSSID(&raw[actdigi+6L], ssid-1UL);
                              startpath = actdigi;
                           }
                           else if (nodigicall) {
                              raw[actdigi+6L] = '\340'; /* WIDE* */
                           }
                           if (nodigicall) startpath = actdigi;
                        }
                        if ((0x200UL & parm->pathcheck)==0) {
                           downpath = actdigi+7L;
                        }
                     }
                     if (show && ok0) {
                        InOut_WriteString(" via WIDEn-n ", 14ul);
                     }
                  }
                  if (show && !ok0) {
                     InOut_WriteString(" no source path ", 17ul);
                  }
               }
               else if (show) {
                  InOut_WriteString(" already digipeated ", 21ul);
               }
            }
         }
         else if (show) {
            InOut_WriteString(" we have already digipeated ", 29ul);
         }
      }
      else if (show) InOut_WriteString(" fingerprint filtered ", 23ul);
   }
   if (duponly || ok0) {
      /*
      IO.WrLn; IO.WrStr("<<<");IO.WrHex(ORD(raw[pathlen+2]),1);
                IO.WrStr(">>> ");
      */
      tt = parm->duptime;
      if (Dup(raw, raw_len, (unsigned long)pathlen, (unsigned long)inlen0,
                &hash)) tt = parm->messagetime;
      t = TimeConv_time(); /* time in s */
      if (parm->timehash[hash]+tt<=t) {
         /* not a duplicate */
         parm->timehash[hash] = t;
         if (duponly) {
            /* dupfilter only */
            *outlen0 = inlen0+2L;
            tmp = *outlen0-1L;
            i = 0L;
            if (i<=tmp) for (;; i++) {
               send[i] = raw[i];
               if (i==tmp) break;
            } /* end for */
         }
         else {
            /* digipeater */
            i = 0L;
            while (i<actdigi) {
               send[i] = (char)((unsigned char)(unsigned char)raw[i]&0xFEU);
                /* copy from-to and clear address end bit */
               ++i;
            }
            if (!nodigicall) {
               if (hamup0[0U]) {
                  for (j = 0L; j<=6L; j++) {
                     send[i] = hamup0[j]; /* append uplink via digicall */
                     ++i;
                  } /* end for */
               }
               for (j = 0L; j<=6L; j++) {
                  send[i] = parm->digicall[j]; /* append new via digicall */
                  ++i;
               } /* end for */
            }
            if (ssidroute>0UL) {
               send[i] = '\256';
               ++i;
               send[i] = '\222';
               ++i;
               send[i] = '\210';
               ++i;
               send[i] = '\212';
               ++i;
               send[i] = (char)((48UL+ssidroute)*2UL);
               ++i;
               send[i] = '@';
               ++i;
               send[i] = (char)((48UL+ssidroute)*2UL); /* no H bit set */
               ++i;
            }
            if ((0x20000UL & parm->pathcheck)) {
               killwide(&startpath, downpath, raw, raw_len);
            }
            j = startpath; /* append rest of path */
            while (j<downpath && i<70L) {
               send[i] = raw[j];
               ++i;
               ++j;
            }
            send[i-1L] = (char)((unsigned char)(unsigned char)
                send[i-1L]|0x1U); /* set address end bit */
            tmp = inlen0-1L;
            j = pathlen;
            if (j<=tmp) for (;; j++) {
               send[i] = raw[j]; /* append data field no crc */
               ++i;
               if (j==tmp) break;
            } /* end for */
            aprsstr_AppCRC(send, send_len, i);
            *outlen0 = i+2L;
         }
      }
      else if (show) {
         InOut_WriteString(" ", 2ul);
         InOut_WriteInt((long)(t-parm->timehash[hash]), 1UL);
         InOut_WriteString("s since same payload ", 22ul);
      }
   }
} /* end Digi() */


static long GetIp(char h[], unsigned long h_len, unsigned long * p,
                unsigned long * ip, unsigned long * port)
{
   unsigned long n;
   unsigned long i;
   char ok0;
   long GetIp_ret;
   X2C_PCOPY((void **)&h,h_len);
   *p = 0UL;
   h[h_len-1] = 0;
   *ip = 0UL;
   for (i = 0UL; i<=4UL; i++) {
      n = 0UL;
      ok0 = 0;
      while ((unsigned char)h[*p]>='0' && (unsigned char)h[*p]<='9') {
         ok0 = 1;
         n = (n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
         ++*p;
      }
      if (!ok0) {
         GetIp_ret = -1L;
         goto label;
      }
      if (i<3UL) {
         if (h[*p]!='.' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
         *ip =  *ip*256UL+n;
      }
      else if (i==3UL) {
         *ip =  *ip*256UL+n;
         if (h[*p]!=':' || n>255UL) {
            GetIp_ret = -1L;
            goto label;
         }
      }
      else if (n>65535UL) {
         GetIp_ret = -1L;
         goto label;
      }
      *port = n;
      ++*p;
   } /* end for */
   GetIp_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetIp_ret;
} /* end GetIp() */


static long GetSec(char h[], unsigned long h_len, unsigned long * p,
                unsigned long * n)
{
   char ok0;
   long GetSec_ret;
   X2C_PCOPY((void **)&h,h_len);
   h[h_len-1] = 0;
   *n = 0UL;
   ok0 = 0;
   while ((unsigned char)h[*p]>='0' && (unsigned char)h[*p]<='9') {
      ok0 = 1;
      *n = ( *n*10UL+(unsigned long)(unsigned char)h[*p])-48UL;
      ++*p;
   }
   if (!ok0) {
      GetSec_ret = -1L;
      goto label;
   }
   /*
   IO.WrCard(n,10); IO.WrLn;
   */
   GetSec_ret = 0L;
   label:;
   X2C_PFREE(h);
   return GetSec_ret;
} /* end GetSec() */


static void MakeRawCall(RAWCALL c, char s[], unsigned long s_len,
                unsigned long j)
{
   unsigned long n;
   unsigned long i;
   X2C_PCOPY((void **)&s,s_len);
   i = 0UL;
   while (i<6UL) {
      if ((s[j] && s[j]!=',') && s[j]!='-') {
         c[i] = (char)((unsigned long)(unsigned char)s[j]*2UL);
         ++j;
      }
      else c[i] = '@';
      ++i;
   }
   n = 0UL;
   if (s[j]=='-') {
      ++j;
      while ((unsigned char)s[j]>='0' && (unsigned char)s[j]<='9') {
         n = (n*10UL+(unsigned long)(unsigned char)s[j])-48UL;
         ++j;
      }
   }
   if (n>15UL) Err("wrong SSID", 11ul);
   if (s[j]=='*') c[6U] = 0;
   else c[6U] = (char)(n*2UL+224UL);
   X2C_PFREE(s);
} /* end MakeRawCall() */


static void Ackpath(const char h[], unsigned long h_len)
{
   aprsstr_Assign(ackpath, 64ul, ">", 2ul);
   aprsstr_Append(ackpath, 64ul, "APNL01", 7ul);
   if (h[0UL]) {
      aprsstr_Append(ackpath, 64ul, ",", 2ul);
      aprsstr_Append(ackpath, 64ul, h, h_len);
   }
   aprsstr_Append(ackpath, 64ul, "::", 3ul);
} /* end Ackpath() */


static char getfix(float * x, const char s[], unsigned long s_len,
                unsigned long * p)
{
   unsigned long i;
   char h[256];
   i = 0UL;
   while (((unsigned char)s[*p]>' ' && s[*p]!='/') && i<255UL) {
      h[i] = s[*p];
      ++i;
      ++*p;
   }
   h[i] = 0;
   if (aprsstr_StrToFix(x, h, 256ul)) return 1;
   return 0;
} /* end getfix() */

static SET256 _cnst0 = {0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL};
static SET256 _cnst = {0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,
                0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL};

static void parms(void)
{
   char h[4096];
   char actpassui;
   char actecho;
   char err;
   char lasth;
   pINSOCK actsock0;
   pOUTPORT onext;
   pOUTPORT outsock0;
   SET256 actpass;
   pDIGIPARMS actdigi;
   unsigned long nold;
   unsigned long n;
   unsigned long i;
   struct BEACON actbeacon;
   pMSGHASH user;
   pCALLS callnext;
   pCALLS actcall;
   float actkm;
   struct aprspos_POSITION actpos;
   struct INSOCK * anonym;
   struct BEACON * anonym0;
   struct DIGIPARMS * anonym1;
   struct OUTPORT * anonym2;
   struct MSGHASH * anonym3;
   SET256 tmp;
   msgusers = 0;
   actsock0 = 0;
   actcall = 0;
   err = 0;
   show = 0;
   Ackpath("WIDE2-2", 8ul);
   actkm = 0.0f;
   actpassui = 0;
   for (;;) {
      Lib_NextArg(h, 4096ul);
      if (h[0U]==0) break;
      if ((h[0U]=='-' && h[1U]) && h[2U]==0) {
         lasth = h[1U];
         if (lasth=='a') {
            Lib_NextArg(h, 4096ul);
            if (h[0U]=='-') h[0U] = 0;
            Ackpath(h, 4096ul);
         }
         else if ((lasth=='R' || lasth=='M') || lasth=='L') {
            Lib_NextArg(h, 4096ul);
            actecho = 0;
            memcpy(actpass,_cnst,32u);
            actdigi = 0;
            actcall = 0;
            actkm = 0.0f;
            actpassui = 0;
            actbeacon.bintervall = 0UL;
            actbeacon.piggytime = 0UL;
            actbeacon.piggyback = 0;
            Storage_ALLOCATE((X2C_ADDRESS *) &actsock0,
                sizeof(struct INSOCK));
            if (actsock0==0) Err("out of memory", 14ul);
            { /* with */
               struct INSOCK * anonym = actsock0;
               anonym->outchain = 0;
               anonym->rflinkname[0U] = 0;
               if (GetIp(h, 4096ul, &i, &anonym->fromip,
                &anonym->bindport)<0L) Err("cannot open udp socket", 23ul);
               anonym->fd = openudp();
               anonym->rawread = lasth=='R';
               if (lasth=='L') {
                  n = 0UL;
                  if (h[i-1UL]==':') {
                     for (;;) {
                        if (i>=4095UL) break;
                        if (h[i]==0) break;
                        anonym->rflinkname[n] = h[i];
                        ++i;
                        ++n;
                     }
                  }
                  if (n==0UL) Err("-L ip:port:name", 16ul);
               }
               if ((anonym->fd>=0L && anonym->fromip!=X2C_max_longcard)
                && bindudp(anonym->fd, anonym->bindport)<0L) {
                  Err("cannot bind inport", 19ul);
               }
               anonym->next = 0;
            }
            if (insocks) actsock0->next = insocks;
            insocks = actsock0;
         }
         else if (lasth=='f') {
            Lib_NextArg(h, 4096ul);
            if (actsock0==0) Err("need input -M or -R before -f", 30ul);
            memcpy(actpass,_cnst0,32u);
            if (h[0U]!='d' && h[0U]!='p') {
               Err("-f (p)ass or (d)elete needed", 29ul);
            }
            i = 1UL;
            n = 0UL;
            nold = 256UL;
            for (;;) {
               if (i>=4095UL) break;
               if ((unsigned char)h[i]>='0' && (unsigned char)h[i]<='9') {
                  n = (n*10UL+(unsigned long)(unsigned char)h[i])-48UL;
               }
               else {
                  for (;;) {
                     if (n>0UL && n<256UL) X2C_INCL(actpass,n,256);
                     if (n<=nold) break;
                     --n;
                  }
                  nold = 256UL;
                  if (h[i]=='-') nold = n;
                  else if (h[i]!=',') {
                     break;
                  }
                  n = 0UL;
               }
               ++i;
            }
            if (h[0U]=='d') {
               memcpy(actpass,X2C_COMPLEMENT(tmp,actpass,8),32u);
            }
         }
         else if (lasth=='k') {
            Lib_NextArg(h, 4096ul);
            if (actsock0==0) Err("need input -M or -R before -k", 30ul);
            i = 0UL;
            if (!getfix(&actpos.lat, h, 4096ul, &i)) {
               Err("latitude in deg", 16ul);
            }
            ++i;
            if (h[i-1UL]!='/' || !getfix(&actpos.long0, h, 4096ul, &i)) {
               Err("longitude in deg", 17ul);
            }
            ++i;
            if ((h[i-1UL]!='/' || !getfix(&actkm, h, 4096ul,
                &i)) || actkm>=2.147483647E+9f) Err("distance in km", 15ul);
         }
         else if (lasth=='b') {
            Lib_NextArg(h, 4096ul);
            if (actsock0==0) Err("need input -M or -R before -b", 30ul);
            { /* with */
               struct BEACON * anonym0 = &actbeacon;
               i = 0UL;
               if (GetSec(h, 4096ul, &i, &n)>=0L && h[i]==':') {
                  anonym0->bintervall = n;
                  anonym0->btime = anonym0->bintervall+TimeConv_time();
                  anonym0->bline = 0UL;
                  ++i;
                  for (n = 0UL; n<=1023UL; n++) {
                     if (i<=4095UL) {
                        anonym0->bfile[n] = h[i];
                        ++i;
                     }
                  } /* end for */
               }
               else Err("beacon format is time:file", 27ul);
            }
         }
         else if (lasth=='d') {
            Lib_NextArg(h, 4096ul);
            if (actsock0==0) Err("need input -M or -R before -d", 30ul);
            Storage_ALLOCATE((X2C_ADDRESS *) &actdigi,
                sizeof(struct DIGIPARMS));
            if (actdigi==0) Err("out of memory", 14ul);
            { /* with */
               struct DIGIPARMS * anonym1 = actdigi;
               MakeRawCall(anonym1->digicall, h, 4096ul, 0UL);
               if (anonym1->digicall[0U]==0 || anonym1->digicall[6U]==0) {
                  Err("wrong digi callsign", 20ul);
               }
               for (i = 0UL; i<=16383UL; i++) {
                  anonym1->timehash[i] = 0UL;
               } /* end for */
               anonym1->duptime = 1740UL;
               anonym1->messagetime = 28UL;
               anonym1->pathcheck = 0UL;
            }
         }
         else if (lasth=='e') actecho = 1;
         else if (((lasth=='r' || lasth=='m') || lasth=='c') || lasth=='l') {
            if (actsock0==0) {
               Err("need input -M or -R before -r or -m or -c", 42ul);
            }
            Lib_NextArg(h, 4096ul);
            Storage_ALLOCATE((X2C_ADDRESS *) &outsock0,
                sizeof(struct OUTPORT));
            if (outsock0==0) Err("out of memory", 14ul);
            { /* with */
               struct OUTPORT * anonym2 = outsock0;
               if (GetIp(h, 4096ul, &i, &anonym2->toip,
                &anonym2->toport)<0L) {
                  Err("wrong udp:port", 15ul);
               }
               memcpy(anonym2->aprspass,actpass,32u);
               anonym2->echo = actecho;
               anonym2->digiparm = actdigi;
               anonym2->axudp2 = lasth=='l';
               anonym2->rawwrite = lasth=='r' || anonym2->axudp2;
               anonym2->crlfwrite = lasth=='c';
               anonym2->beacon0 = actbeacon;
               actbeacon.bintervall = 0UL;
               anonym2->passnoUI = actpassui;
               anonym2->next = 0;
               anonym2->filtercalls = actcall;
               anonym2->mypos.lat = actpos.lat*1.7453292519444E-2f;
               anonym2->mypos.long0 = actpos.long0*1.7453292519444E-2f;
               anonym2->maxkm = (long)(unsigned long)X2C_TRUNCC(actkm,0UL,
                X2C_max_longcard);
            }
            actdigi = 0;
            actpassui = 0;
            onext = actsock0->outchain;
            if (onext==0) actsock0->outchain = outsock0;
            else {
               while (onext->next) onext = onext->next;
               onext->next = outsock0;
            }
         }
         else {
            if (lasth=='h') {
               osi_WrLn();
               osi_WrStrLn(" -a <paths>     user message ack path eg. WIDE2-2\
 or - for no path", 67ul);
               osi_WrStrLn(" -b <s>:<file>  enable beacon every s(econds) pat\
h and text from <file>", 72ul);
               osi_WrStrLn("                cycles thru lines in file, empty \
lines = no tx (size max 32kb)", 79ul);
               osi_WrStrLn("                \\\\z ddhhmm, \\\\h hhmmss, \\\\:\
filename: insert file, \\\\[filename]", 79ul);
               osi_WrStrLn("                insert file and delete after, \\\\
\(filename) if exists insert", 76ul);
               osi_WrStrLn("                and delete,\\\\\\ is \\\\, \\\\rm\
 delete beacon file", 62ul);
               osi_WrStrLn("                file may be modified any time eg.\
 by telemetry program", 71ul);
               osi_WrStrLn(" -c <ip>:<port> send text monitor udp frame with \
cr lf", 55ul);
               osi_WrStrLn(" -d <call>      digipeater enable (and dupe filte\
r) call", 57ul);
               osi_WrStrLn(" -d -           dupe filter without data modifica\
tion", 54ul);
               osi_WrStrLn(" -e             echo last (filtert) output",
                43ul);
               osi_WrStrLn(" -f <filterparm>  \'p<x>,<x>...\' pass UI frames \
with first char (-f p58,110)", 76ul);
               osi_WrStrLn("                  \'d<x>,<x>...\' pass UI frames \
with not first char (-f d32,65-79)", 82ul);
               osi_WrStrLn("                  Messages to itself are treated \
as Telemetry  (-f d84)", 72ul);
               osi_WrStrLn(" -h             this", 21ul);
               osi_WrStrLn(" -k <deg>/<deg>/<km> distance filter, center/radi\
us -k 48.2/13.5/100", 69ul);
               osi_WrStrLn("                -k 0/0/20000 remove all (not user\
 msg) with no position data", 77ul);
               osi_WrStrLn(" -L <ip>:<port>:<netname>   read monitor rflink h\
eader with netname", 68ul);
               osi_WrStrLn(" -l <ip>:<port> send raw axudp frame and pass thr\
u axudp2 header", 65ul);
               osi_WrStrLn(" -M <ip>:<port> read text monitor udp frame",
                44ul);
               osi_WrStrLn("                ip=255.255.255.255:0 read text mo\
nitor from stdin", 66ul);
               osi_WrStrLn("                ip=255.255.255.255:1 read text mo\
nitor from stdin and terminate", 80ul);
               osi_WrStrLn(" -m <ip>:<port> send text monitor udp frame 0 ter\
minated", 57ul);
               osi_WrStrLn(" -p <n>,<...>   -p 7,8 igate friendly digi relayi\
ng all direct heared", 70ul);
               osi_WrStrLn("                -p 5,6,7,8,9 first hop digi, -p 3\
,4 (noisy) last hop digi", 74ul);
               osi_WrStrLn("                -p 0,1,3,4,5,6,7,8,9,14,16 origin\
al, noisy, path loosing ...", 77ul);
               osi_WrStrLn("                0..2 limit ssid-routing to (bit0 \
+ 2*bit1 + 4*bit2)", 68ul);
               osi_WrStrLn("                dest-3: dest-2,digicall*, dest-3,\
call*: dest-2,call,digicall*", 78ul);
               osi_WrStrLn("                3 allow repeatet before (mostly w\
ith wrong path trace)", 71ul);
               osi_WrStrLn("                4 no uplink check for \'looks lik\
e direct heared\'", 65ul);
               osi_WrStrLn("                  (noisy, wrong path trace possib\
le)", 53ul);
               osi_WrStrLn("                5 resend TRACEn-(N-1), 6 resend W\
IDEn-(N-1)", 60ul);
               osi_WrStrLn("                  so frames may be relayed again \
by loss of downlink trace", 75ul);
               osi_WrStrLn("                7 append remaining digi path afte\
r via digicall", 64ul);
               osi_WrStrLn("                8 append remaining digi path afte\
r RELAY/GATE/TRACE", 68ul);
               osi_WrStrLn("                9 append remaining digi path afte\
r WIDE", 56ul);
               osi_WrStrLn("                  so frames may be relayed again \
by loss of downlink trace", 75ul);
               osi_WrStrLn("                10, 11, 12, 13 switch off via dig\
icall, RELAY, TRACE, WIDE", 75ul);
               osi_WrStrLn("                14 insert not digicall on direct \
heared before WIDEn-N,", 72ul);
               osi_WrStrLn("                   as others do, your digi is hid\
den, and path is shown wrong", 78ul);
               osi_WrStrLn("                15 convert destination ssid routi\
ng to WIDEn-n", 63ul);
               osi_WrStrLn("                16 send n-0 without repeated flag\
", 50ul);
               osi_WrStrLn("                17 remove all WIDE from downlink \
path", 54ul);
               osi_WrStrLn("                18 enable via GATE (use as gate: \
disable via all other)", 72ul);
               osi_WrStrLn("                19 allow ECHO as alias to RELAY",
                 48ul);
               osi_WrStrLn("                20 switch off bad digi fingerprin\
t filter", 58ul);
               osi_WrStrLn(" -P <s>         piggyback time sending beacon ear\
lier if sent anything now", 75ul);
               osi_WrStrLn(" -R <ip>:<port> read raw axudp frame, 0 ip read f\
rom all (-R 0.0.0.0:2000)", 75ul);
               osi_WrStrLn(" -r <ip>:<port> send raw axudp frame", 37ul);
               osi_WrStrLn(" -s             pass not-UI-frames too (all PR-Fr\
ames, SABM, RR..)", 67ul);
               osi_WrStrLn("                (raw axudp only) and axudp2 modem\
 to layer2 message frames", 75ul);
               osi_WrStrLn(" -t <s>,<s>     dupe filter time in seconds (all \
types, user messages)", 71ul);
               osi_WrStrLn("                -t 1740,28 29min not same beacon,\
 28s for retrying user message)", 81ul);
               osi_WrStrLn(" -u <call>:<file> global option, receive & ack us\
ermessages to call store in file", 82ul);
               osi_WrStrLn("                repeat -u for more Calls to same \
or different File", 67ul);
               osi_WrStrLn(" -v             show frames and analytics on stdo\
ut", 52ul);
               osi_WrStrLn(" -x <call>{,<call>} delete frames with call in a \
address field, -x TCPIP,UID***-*", 82ul);
               osi_WrStrLn("udpbox -v -M 0.0.0.0:9200 -d MYCALL-10 -p 0,1,7 -\
t 1800,28 -r 192.168.1.24:9400", 80ul);
               osi_WrLn();
               X2C_ABORT();
            }
            if (lasth=='s') actpassui = 1;
            else if (lasth=='t') {
               if (actdigi==0) Err("need -d before -t", 18ul);
               Lib_NextArg(h, 4096ul);
               i = 0UL;
               if (GetSec(h, 4096ul, &i, &n)>=0L) {
                  actdigi->duptime = n;
                  ++i;
                  if (h[i-1UL]==',' && GetSec(h, 4096ul, &i, &n)>=0L) {
                     actdigi->messagetime = n;
                  }
               }
            }
            else if (lasth=='P') {
               if (actsock0==0) Err("need input -M or -R before -P", 30ul);
               Lib_NextArg(h, 4096ul);
               i = 0UL;
               if (GetSec(h, 4096ul, &i, &n)>=0L) actbeacon.piggytime = n;
            }
            else if (lasth=='p') {
               if (actdigi==0) Err("need -d before -p", 18ul);
               Lib_NextArg(h, 4096ul);
               i = 0UL;
               while (GetSec(h, 4096ul, &i, &n)>=0L) {
                  if (n<31UL) actdigi->pathcheck |= (1UL<<n);
                  if (h[i]==',') ++i;
               }
            }
            else if (lasth=='u') {
               Lib_NextArg(h, 4096ul);
               Storage_ALLOCATE((X2C_ADDRESS *) &user,
                sizeof(struct MSGHASH));
               if (user==0) Err("out of memory", 14ul);
               { /* with */
                  struct MSGHASH * anonym3 = user;
                  anonym3->wpos = 0UL;
                  anonym3->msgfile[0U] = 0;
                  for (i = 0UL; i<=15UL; i++) {
                     anonym3->hash[i].ackcnt = 0UL;
                  } /* end for */
                  i = 0UL;
                  n = 0UL;
                  while ((n<=8UL && h[i]) && h[i]!=':') {
                     anonym3->usercall[n] = h[i];
                     ++n;
                     ++i;
                  }
                  while (n<=8UL) {
                     anonym3->usercall[n] = ' ';
                     ++n;
                  }
                  if (h[i]) {
                     ++i;
                     n = 0UL;
                     while (n<=1023UL && h[i]) {
                        anonym3->msgfile[n] = h[i];
                        ++n;
                        ++i;
                     }
                  }
                  anonym3->next = 0;
               }
               if (msgusers) user->next = msgusers;
               msgusers = user;
            }
            else if (lasth=='v') show = 1;
            else if (lasth=='x') {
               Lib_NextArg(h, 4096ul);
               if (actsock0==0) Err("need input -M or -R before -x", 30ul);
               i = 0UL;
               for (;;) {
                  Storage_ALLOCATE((X2C_ADDRESS *) &callnext,
                sizeof(struct CALLS));
                  if (callnext==0) Err("out of memory", 14ul);
                  MakeRawCall(callnext->call, h, 4096ul, i);
                  callnext->next = actcall;
                  actcall = callnext;
                  while (h[i]!=',') {
                     if (i>=4095UL || (unsigned char)h[i]<=' ') {
                        goto loop_exit;
                     }
                     ++i;
                  }
                  ++i;
               }
               loop_exit:;
            }
            else err = 1;
         }
      }
      else {
         /*
               h[0]:=0C;
         */
         err = 1;
      }
      if (err) break;
   }
   if (err) {
      InOut_WriteString(">", 2ul);
      InOut_WriteString(h, 4096ul);
      osi_WrStrLn("< use -h", 9ul);
      X2C_ABORT();
   }
} /* end parms() */


static void showpip(unsigned long ip, unsigned long port)
{
   InOut_WriteInt((long)(ip/16777216UL), 1UL);
   InOut_WriteString(".", 2ul);
   InOut_WriteInt((long)(ip/65536UL&255UL), 1UL);
   InOut_WriteString(".", 2ul);
   InOut_WriteInt((long)(ip/256UL&255UL), 1UL);
   InOut_WriteString(".", 2ul);
   InOut_WriteInt((long)(ip&255UL), 1UL);
   InOut_WriteString(":", 2ul);
   InOut_WriteInt((long)port, 1UL);
} /* end showpip() */

/*
PROCEDURE showstr(s-:ARRAY OF CHAR);

  PROCEDURE Hex(b:SET8):CHAR;
  VAR d:CARDINAL;
  BEGIN
    d:=CAST(CARDINAL, b*SET8{0..3});
    IF d>9 THEN INC(d, ORD("A")-10-ORD("0")) END;
    RETURN CHR(d+ORD("0"))  
  END Hex;

VAR i,j:CARDINAL;
    h:ARRAY[0..1023] OF CHAR; 
BEGIN
  i:=0;
  j:=0;
  WHILE (i<=HIGH(s)) & (s[i]<>0C) & (j<HIGH(h)-5) DO
    IF s[i]<" " THEN 
      h[j]:="<"; INC(j);
      h[j]:=Hex(SHIFT(CAST(SET8, s[i]), -4)); INC(j);
      h[j]:=Hex(CAST(SET8, s[i])); INC(j);
      h[j]:=">";
    ELSE h[j]:=s[i] END;
    INC(i);
    INC(j);
  END;
  h[j]:=0C;
  WrStrLn(h); 
END showstr;
*/

static void showhex(long * n, long e, char in[], unsigned long in_len,
                long * len, const char s[], unsigned long s_len)
{
   long i;
   long tmp;
   char tmp0;
   if (show) {
      osi_WrLn();
      osi_WrStrLn(s, s_len);
      tmp = *len-1L;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         osi_WrHex((unsigned long)(unsigned char)in[i], 3UL);
         if (i==tmp) break;
      } /* end for */
      osi_WrLn();
      tmp = e-1L;
      i = 0L;
      if (i<=tmp) for (;; i++) {
         *n = (long)((unsigned long)(unsigned char)in[i]/2UL);
         if (*n<32L) {
            InOut_WriteString("<", 2ul);
            osi_WrHex((unsigned long)*n, 1UL);
            InOut_WriteString(">", 2ul);
         }
         else InOut_WriteString((char *)(tmp0 = (char)*n,&tmp0), 1u/1u);
         if (X2C_MOD(i,7L)==6L) InOut_WriteString(",", 2ul);
         if (i==tmp) break;
      } /* end for */
      osi_WrLn();
   }
} /* end showhex() */


static void cpraw(char in[], unsigned long in_len, char out[],
                unsigned long out_len, long * len)
{
   long e;
   long n;
   long i;
   char c;
   char h;
   long tmp;
   e = 0L;
   tmp = *len-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      c = in[i];
      out[i] = c;
      if (e==0L && ((unsigned long)(unsigned char)c&1)) e = i+1L;
      if (i==tmp) break;
   } /* end for */
   if (*len==2L) return;
   /* crc only, maybe there is a axudp2 head*/
   if (((unsigned long)e%7UL || e<14L) || e>70L) {
      showhex(&n, e, in, in_len, len,
                " bad raw format, no address end found", 38ul);
      *len = 0L;
   }
   else {
      h = 1;
      tmp = e-1L;
      i = 20L;
      if (i<=tmp) for (tmp = (unsigned long)(tmp-i)/7L;;) {
         if ((unsigned long)(unsigned char)in[i]>=128UL) {
            if (!h) {
               showhex(&n, e, in, in_len, len, " bad H bit in raw frame",
                24ul);
               *len = 0L;
            }
            h = 1;
         }
         else h = 0;
         if (!tmp) break;
         --tmp;
         i += 7L;
      } /* end for */
   }
} /* end cpraw() */

#define udpbox_MSYM "\\"

#define udpbox_INSFN ":"
/* insert file and send no beacon if missing */

#define udpbox_INSFNEND ":"

#define udpbox_DELFN "["
/* insert file, delete and send no beacon if missing */

#define udpbox_DELFNEND "]"

#define udpbox_DELFNN "("
/* insert file, delete and insert nothing if missing */

#define udpbox_DELFNNEND ")"


static void beaconmacros(char s[], unsigned long s_len, char * del)
{
   unsigned long i;
   long j;
   long len;
   char ns[256];
   char ds[256];
   char fn[1024];
   long f;
   char fnend;
   char voidok;
   *del = 0;
   i = 0UL;
   ns[0U] = 0;
   while (i<s_len-1 && s[i]) {
      if (s[i]=='\\' && s[i+1UL]=='\\') {
         i += 2UL;
         if (s[i]=='z') {
            /* insert day, hour, min */
            aprsstr_DateToStr(TimeConv_time(), ds, 256ul);
            ds[0U] = ds[8U];
            ds[1U] = ds[9U];
            ds[2U] = ds[11U];
            ds[3U] = ds[12U];
            ds[4U] = ds[14U];
            ds[5U] = ds[15U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if (s[i]=='h') {
            /* insert hour, min, s */
            aprsstr_DateToStr(TimeConv_time(), ds, 256ul);
            ds[0U] = ds[11U];
            ds[1U] = ds[12U];
            ds[2U] = ds[14U];
            ds[3U] = ds[15U];
            ds[4U] = ds[17U];
            ds[5U] = ds[18U];
            ds[6U] = 0;
            aprsstr_Append(ns, 256ul, ds, 256ul);
         }
         else if ((s[i]==':' || s[i]=='[') || s[i]=='(') {
            /* insert file */
            if (s[i]==':') fnend = ':';
            else if (s[i]=='(') fnend = ')';
            else fnend = ']';
            fn[0U] = 0;
            ++i;
            while ((i<s_len-1 && s[i]) && s[i]!=fnend) {
               aprsstr_Append(fn, 1024ul, (char *) &s[i], 1u/1u);
               ++i;
            }
            f = osi_OpenRead(fn, 1024ul);
            if (f>=0L) {
               len = osi_RdBin(f, (char *)ds, 256u/1u, 255UL);
               osi_Close(f);
               if (fnend==']' || fnend==')') {
                  FileSys_Remove(fn, 1024ul, &voidok);
                /* delete insert file after inserting */
               }
               j = 0L;
               while (((j<len && ds[j]!='\015') && ds[j]!='\012') && ds[j]) {
                  aprsstr_Append(ns, 256ul, (char *) &ds[j], 1u/1u);
                  ++j;
               }
            }
            else if (fnend!=')') {
               /* skip whole beacon line */
               if (show) {
                  osi_WrLn();
                  osi_WrStrLn("beacon macro file not readable ", 32ul);
               }
               s[0UL] = 0;
               return;
            }
         }
         else {
            if (s[i]=='r' && s[i+1UL]=='m') {
               /* delete beacon file */
               s[0UL] = 0;
               *del = 1;
               return;
            }
            if (s[i]=='\\') aprsstr_Append(ns, 256ul, "\\\\", 3ul);
            else {
               if (show) {
                  osi_WrLn();
                  osi_WrStrLn("bad beacon macro ", 18ul);
               }
               s[0UL] = 0;
               return;
            }
         }
      }
      else aprsstr_Append(ns, 256ul, (char *) &s[i], 1u/1u);
      ++i;
   }
   aprsstr_Assign(s, s_len, ns, 256ul);
} /* end beaconmacros() */


static void beacon(pINSOCK insock, char buf[], unsigned long buf_len,
                long * len, pOUTPORT * outsock0)
{
   long lc;
   long eol;
   long bol;
   long i;
   long f;
   char fb[32768];
   unsigned long t;
   char ok0;
   char del;
   struct BEACON * anonym;
   *len = 0L;
   *outsock0 = insock->outchain;
   for (;;) {
      if (*outsock0==0) {
         *len = 0L;
         break;
      }
      { /* with */
         struct BEACON * anonym = &(*outsock0)->beacon0;
         t = TimeConv_time();
         if (anonym->piggyback) t += anonym->piggytime;
         anonym->piggyback = 0;
         if ((anonym->bintervall>0UL && anonym->bfile[0U])
                && anonym->btime<=t) {
            anonym->btime += anonym->bintervall;
            if (anonym->btime<=t) anonym->btime = t+anonym->bintervall;
            f = osi_OpenRead(anonym->bfile, 1024ul);
            if (f>=0L) {
               *len = osi_RdBin(f, (char *)fb, 32768u/1u, 32767UL);
               osi_Close(f);
               while (*len>0L && (unsigned char)fb[*len-1L]<=' ') {
                  --*len; /* remove junk from eof */
               }
               if (*len>0L && *len<32767L) {
                  fb[*len] = '\012';
                  ++*len;
               }
               do {
                  lc = (long)anonym->bline;
                  eol = 0L;
                  for (;;) {
                     bol = eol;
                     while (eol<*len && fb[eol]!='\012') ++eol;
                     if (eol>=*len) {
                        bol = eol;
                        if (anonym->bline) {
                           lc = 1L;
                           anonym->bline = 0UL;
                        }
                        break;
                     }
                     if (fb[bol]!='#') {
                        if (lc==0L) {
                           ++anonym->bline;
                           break;
                        }
                        --lc;
                     }
                     ++eol;
                  }
               } while (lc);
               i = 0L;
               while (bol<eol) {
                  fb[i] = fb[bol];
                  ++i;
                  ++bol;
               }
               fb[i] = 0;
               /*
               IF show THEN WrStrLn(fb) END;
               */
               *len = 0L;
               if (i>0L) {
                  beaconmacros(fb, 32768ul, &del);
                  if (del) FileSys_Remove(anonym->bfile, 1024ul, &ok0);
                  else aprsstr_mon2raw(fb, 32768ul, buf, buf_len, len);
                  if (show && *len==0L) {
                     osi_WrLn();
                     if (del) osi_WrStrLn("delete beacon file ", 20ul);
                     else osi_WrStrLn("bad beacon format ", 19ul);
                  }
               }
               break;
            }
            if (show) osi_WrStrLn("beacon file not found", 22ul);
         }
         *outsock0 = (*outsock0)->next;
      }
   }
} /* end beacon() */

#define udpbox_ACKTIME 30


static void sendack(char buf[], unsigned long buf_len, long * len,
                pINSOCK fromsock)
{
   long m;
   unsigned long j;
   unsigned long i;
   pMSGHASH user;
   char fb[256];
   unsigned long t;
   struct MSGHASH * anonym;
   struct _0 * anonym0;
   user = msgusers;
   t = TimeConv_time();
   while (user) {
      *len = 0L;
      { /* with */
         struct MSGHASH * anonym = user;
         for (m = 0L; m<=15L; m++) {
            { /* with */
               struct _0 * anonym0 = &anonym->hash[m];
               if ((anonym0->ackcnt>0UL && anonym0->acktime<t)
                && fromsock==anonym0->source) {
                  --anonym0->ackcnt;
                  anonym0->acktime = t+30UL;
                  j = 0UL;
                  i = 0UL;
                  while (i<=8UL && (unsigned char)anonym->usercall[i]>' ') {
                     fb[j] = anonym->usercall[i];
                     ++j;
                     ++i;
                  }
                  i = 0UL;
                  while ((unsigned char)ackpath[i]>' ') {
                     fb[j] = ackpath[i];
                     ++j;
                     ++i;
                  }
                  for (i = 0UL; i<=8UL; i++) {
                     fb[j] = anonym0->froms[i];
                     ++j;
                  } /* end for */
                  fb[j] = ':';
                  ++j;
                  i = 0UL;
                  while ((unsigned char)"ack"[i]>' ') {
                     fb[j] = "ack"[i];
                     ++j;
                     ++i;
                  }
                  /*
                  IO.WrLn; IO.WrStr("    <"); IO.WrStr(hash[m].acks);
                IO.WrStrLn(">");
                  */
                  i = 0UL;
                  while ((unsigned char)anonym0->acks[i]>' ') {
                     fb[j] = anonym0->acks[i];
                     ++j;
                     ++i;
                  }
                  fb[j] = 0;
                  /*
                  IO.WrLn; IO.WrStr("    <"); IO.WrStr(fb); IO.WrStrLn(">");
                  */
                  aprsstr_mon2raw(fb, 256ul, buf, buf_len, len);
                  if (show && *len==0L) {
                     InOut_WriteString(fb, 256ul);
                     osi_WrStrLn(" message ack path wrong", 24ul);
                  }
                  return;
               }
            }
         } /* end for */
         user = anonym->next;
      }
   }
} /* end sendack() */


static void appudp2(char ob[], unsigned long ob_len, unsigned long * olen,
                const char ud[], unsigned long ud_len, const char ib[],
                unsigned long ib_len, long len)
/* append axudp2 header */
{
   long j;
   long i;
   i = 0L;
   j = 0L;
   while (ud[i]) {
      ob[i] = ud[i];
      ++i;
   }
   if (i>0L) {
      ob[i] = 0;
      ++i;
   }
   len -= 2L;
   while (j<len) {
      ob[i] = ib[j];
      ++i;
      ++j;
   }
   *olen = (unsigned long)(i+2L);
   aprsstr_AppCRC(ob, ob_len, i);
} /* end appudp2() */

static char ibuf[338];

static char rawbuf[338];

static char rawout[338];

static char workraw[338];

static char udp2[64]; /* axudp2 header */

static char mbuf[512];

static long inlen;

static long outlen;

static long res;

static unsigned long monlen;

static pINSOCK actsock;

static pOUTPORT outsock;

static pOUTPORT isbeacon;

static char nobeacon;

static char piggy;

static RAWCALL hamup;

static aprsstr_GHOSTSET _cnst1 = {0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL,0x00000000UL,0x00000000UL,
                0x00000000UL,0x00000000UL};

X2C_STACK_LIMIT(100000l)
extern int main(int argc, char **argv)
{
   X2C_BEGIN(&argc,argv,1,4000000l,8000000l);
   if (sizeof(SET256)!=32) X2C_ASSERT(0);
   if (sizeof(RAWCALL)!=7) X2C_ASSERT(0);
   if (sizeof(MONCALL)!=9) X2C_ASSERT(0);
   if (sizeof(FILENAME)!=1024) X2C_ASSERT(0);
   aprsstr_BEGIN();
   aprspos_BEGIN();
   TimeConv_BEGIN();
   Lib_BEGIN();
   Storage_BEGIN();
   osi_BEGIN();
   stdinpos = 0UL;
   insocks = 0;
   parms();
   for (;;) {
      fdclr();
      actsock = insocks;
      if (insocks && insocks->fromip==X2C_max_longcard) fdsetr(0UL);
      while (actsock) {
         { /* with */
            struct INSOCK * anonym = actsock;
            if (anonym->fd>=0L) fdsetr((unsigned long)anonym->fd);
            actsock = anonym->next;
         }
      }
      if (selectr(1UL, 0UL)>=0L) {
         actsock = insocks;
         while (actsock) {
            do {
               piggy = 0;
               inlen = 0L;
               if (actsock->fd>=0L && issetr((unsigned long)actsock->fd)) {
                  inlen = getudp(actsock->fd, ibuf, 338ul, actsock->fromip,
                actsock->rawread);
               }
               else if (issetr(0UL) && actsock->fromip==X2C_max_longcard) {
                  inlen = getstdin(ibuf, 338ul);
               }
               udp2[0U] = 0;
               if (inlen>0L && ibuf[0U]=='\001') {
                  aprsstr_extrudp2(ibuf, 338ul, udp2, 64ul, &inlen);
                /* axudp2 */
               }
               hamup[0U] = 0;
               if (inlen>0L) {
                  nobeacon = 1;
                  if (actsock->rawread) {
                     cpraw(ibuf, 338ul, rawbuf, 338ul, &inlen);
                  }
                  else {
                     if (inlen<=337L) ibuf[inlen] = 0;
                     checkhamnet(ibuf, 338ul, &inlen, actsock->rflinkname,
                9ul, hamup);
                     if (inlen>0L) {
                        aprsstr_mon2raw(ibuf, 338ul, rawbuf, 338ul, &inlen);
                        if (show && inlen==0L) {
                           osi_WrStrLn("bad mon format ", 16ul);
                        }
                     }
                     else if (show) {
                        osi_WrStrLn("no rflink head match", 21ul);
                     }
                  }
                  isbeacon = 0;
               }
               else {
                  nobeacon = 0;
                  beacon(actsock, rawbuf, 338ul, &inlen, &isbeacon);
                  if (isbeacon==0) sendack(rawbuf, 338ul, &inlen, actsock);
               }
               if (inlen>=2L) {
                  if (show && inlen>2L) {
                     aprsstr_raw2mon(rawbuf, 338ul, mbuf, 512ul,
                (unsigned long)(inlen-2L), &monlen, _cnst1);
                     osi_WrLn();
                     showpip(showip, showport);
                     InOut_WriteString("(", 2ul);
                     InOut_WriteInt(inlen, 1UL);
                     InOut_WriteString(")", 2ul);
                     InOut_WriteString(":", 2ul);
                     if (monlen<=0UL) {
                        InOut_WriteString("<raw to mon error>", 19ul);
                     }
                     aprsstr_CtrlHex(mbuf, 512ul);
                     osi_WrStrLn(mbuf, 512ul);
                  }
                  outsock = actsock->outchain;
                  outlen = 0L;
                  while (outsock) {
                     if (show && inlen>2L) {
                        InOut_WriteString(" tx: ", 6ul);
                        showpip(outsock->toip, outsock->toport);
                     }
                     memcpy(workraw,rawbuf,338u);
                     if (((isbeacon==0 || isbeacon==outsock)
                || outsock->echo) && Filter(workraw, 338ul, inlen, outsock,
                actsock)) {
                        if (outsock->digiparm) {
                           Digi(workraw, 338ul, rawout, 338ul, inlen,
                &outlen, hamup, outsock->digiparm, !nobeacon);
                           if (show && outlen>2L) {
                              InOut_WriteString(" digi", 6ul);
                           }
                        }
                        else if (!outsock->echo) {
                           memcpy(rawout,workraw,338u);
                           outlen = inlen;
                        }
                        /* else use last filtert */
                        if (outlen>0L) {
                           outsock->beacon0.piggyback = 1;
                           piggy = 1;
                           if (outsock->rawwrite) {
                              if (outsock->axudp2) {
                                 appudp2(mbuf, 512ul, &monlen, udp2, 64ul,
                rawout, 338ul, outlen);
                                 res = udpsend(actsock->fd, mbuf,
                (long)monlen, outsock->toport, outsock->toip);
                              }
                              else {
                                 aprsstr_AppCRC(rawout, 338ul, outlen-2L);
                                 res = udpsend(actsock->fd, rawout, outlen,
                outsock->toport, outsock->toip);
                              }
                              if (show && outlen>2L) {
                                 osi_WrStrLn(" raw", 5ul);
                              }
                           }
                           else {
                              aprsstr_raw2mon(rawout, 338ul, mbuf, 512ul,
                (unsigned long)(outlen-2L), &monlen, _cnst1);
                              if (monlen>0UL) {
                                 if (outsock->crlfwrite) {
                                    mbuf[monlen-1UL] = '\015';
                                    mbuf[monlen] = '\012';
                                    ++monlen;
                                 }
                                 res = udpsend(actsock->fd, mbuf,
                (long)monlen, outsock->toport, outsock->toip);
                                 if (show) {
                                    osi_WrStrLn(" mon", 5ul);
                                 }
                              }
                              else if (show) {
                                 osi_WrStrLn(" monerr", 8ul);
                              }
                           }
                        }
                        else if (show) osi_WrStrLn(" no tx", 7ul);
                     }
                     else if (show && outlen>2L) {
                        osi_WrStrLn(" deleted", 9ul);
                     }
                     outsock = outsock->next;
                  }
                  if ((insocks && insocks->fromip==X2C_max_longcard)
                && insocks->bindport==1UL) {
                     goto loop_exit; /* single stdin line mode */
                  }
               }
            } while (piggy);
            actsock = actsock->next;
         }
      }
   }
   loop_exit:;
   X2C_EXIT();
   return 0;
}

X2C_MAIN_DEFINITION
