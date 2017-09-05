/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef frameio_H_
#include "frameio.h"
#endif
#define frameio_C_
#ifndef l2_H_
#include "l2.h"
#endif
#ifndef osi_H_
#include "osi.h"
#endif
#ifndef udp_H_
#include "udp.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif



char frameio_crcok;
struct frameio_UDPSOCK frameio_udpsocks0[15];
#define frameio_DCDRETRIES 500

static uint32_t size;

static uint32_t from;

static uint32_t udppos;

static char axv2;

static char udpbuf[1601];


struct _0 {
   uint32_t lastBaud; /*insert flags if baud change*/
   uint32_t oldi; /*last received frame index from hardware*/
   uint32_t nextfrom; /*read next frame from hardware*/
   uint32_t txakku; /*send next frame to hardware*/
   char twoflags;
};

/*
VAR CRCL, CRCH:ARRAY[0..255] OF SET8;

PROCEDURE CRC(VAR frame:ARRAY OF CHAR; VAR crc:CARD16; size:CARDINAL);
VAR l,h:SET8;
    b:CARD8;
    i:INTEGER;
BEGIN
  l:=CAST(SET8, crc);
  h:=CAST(SET8, crc >> 8);
  FOR i:=0 TO VAL(INTEGER, size)-1 DO
    b:=CAST(CARD8, CAST(SET8, frame[i]) / l);
    l:=CRCL[b] / h;
    h:=CRCH[b];
  END;
  crc:=CAST(CARD16, l) + (CAST(CARD16, h) << 8);
END CRC;



PROCEDURE UDPCRC(frame-:ARRAY OF CHAR; size:INTEGER):CARDINAL;
VAR l,h:SET8;
    b:CARD8;
    i:INTEGER;
BEGIN
  l:=SET8{};
  h:=SET8{};
  FOR i:=0 TO size-1 DO
    b:=CAST(CARD8, CAST(SET8, frame[i]) / l);
    l:=CRCL[b] / h;
    h:=CRCH[b];
  END;
  RETURN ORD(CAST(CHAR, l))+256*ORD(CAST(CHAR, h))
END UDPCRC;
*/
/*
PROCEDURE opensock(num:CARDINAL; fromport, toport:UDPPORT):BOOLEAN;
BEGIN
  WITH udpsocks[num] DO
    fd:=openudp();
    IF (CAST(INTEGER,fd)<0) OR (bindudp(fd,
                fromport)<0) THEN RETURN FALSE END;
    dport:=toport;
  END;
  RETURN TRUE
END opensock;
*/

static void rmnc(void)
{
   uint32_t k;
   uint32_t j;
   char cmd;
   l2_adress[14U] = l2_adress[7U];
   l2_asize = 15U;
   k = (uint32_t)(uint8_t)l2_adress[0U]*128UL+(uint32_t)(uint8_t)
                l2_adress[1U]/2UL;
   cmd = (char)(k&1);
   k = k/2UL;
   j = 12UL;
   do {
      l2_adress[j] = (char)((k%10UL+48UL)*2UL);
      --j;
      k = k/10UL;
   } while (j!=6UL);
   if (cmd) l2_adress[13U] = '`';
   else l2_adress[13U] = '\340';
   j = (uint32_t)(uint8_t)l2_adress[2U];
   l2_adress[0U] = (char)((j/4UL)*2UL+64UL);
   k = (uint32_t)(uint8_t)l2_adress[3U];
   l2_adress[1U] = (char)(((j&3UL)*16UL+k/16UL+32UL)*2UL);
   j = (uint32_t)(uint8_t)l2_adress[4U];
   l2_adress[2U] = (char)(((k&15UL)*4UL+j/64UL+32UL)*2UL);
   l2_adress[3U] = (char)(((j&63UL)+32UL)*2UL);
   j = (uint32_t)(uint8_t)l2_adress[5U];
   l2_adress[4U] = (char)((j/4UL)*2UL+64UL);
   k = (uint32_t)(uint8_t)l2_adress[6U];
   l2_adress[5U] = (char)(((j&3UL)*16UL+k/16UL+32UL)*2UL);
   l2_adress[6U] = (char)(((k&15UL)+48UL)*2UL+(uint32_t)cmd*128UL);
} /* end rmnc() */


static char GetAField(uint32_t port)
{
   uint32_t a;
   uint32_t j;
   uint32_t i;
   char c;
   uint32_t tmp;
   if (size>=10UL) {
      if (size>=70UL) a = 70UL;
      else a = size;
      udppos = 0UL;
      tmp = a-1UL;
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         c = udpbuf[udppos];
         ++udppos;
         l2_adress[i] = c;
         if (((uint32_t)(uint8_t)c&1)) {
            /*address end mark*/
            if (i==1UL) {
               for (j = 2UL; j<=6UL; j++) {
                  l2_adress[j] = udpbuf[udppos];
                  ++udppos;
               } /* end for */
               l2_asize = 7U;
            }
            else l2_asize = (uint16_t)(i+1UL);
            /*WrInt(asize, 4); WrStrLn("=as"); */
            if ((l2_asize>=7U && l2_asize%7U==0U) && (uint32_t)
                l2_asize<=size) {
               l2_adress[l2_asize] = udpbuf[udppos]; /*command*/
               ++udppos;
               ++l2_asize;
               size -= (uint32_t)l2_asize;
               /*WrInt(size, 4); WrStrLn("=saf"); */
               /*FOR j:=0 TO asize-1 DO WrInt(ORD(adress[j]), 4); END;
                WrStrLn("=af"); */
               return 1;
            }
            return 0;
         }
         if (i==tmp) break;
      } /* end for */
   }
   return 0;
} /* end GetAField() */


static char GetDField(void)
{
   int32_t i;
   struct l2_DFIELD * anonym;
   int32_t tmp;
   if (size<=257UL) {
      { /* with */
         struct l2_DFIELD * anonym = l2_dbuf;
         anonym->len = (uint16_t)size;
         /*WrInt(len, 4); WrStrLn("=ilen"); */
         tmp = (int32_t)size-1L;
         i = 0L;
         if (i<=tmp) for (;; i++) {
            anonym->info[i] = udpbuf[udppos];
            ++udppos;
            if (i==tmp) break;
         } /* end for */
         /*
               crc:=0;
               CRC(adress,crc,asize);
               IF len<>0 THEN CRC(info,crc,len) END;
               crcok:=ORD(udpbuf[udppos]) + (ORD(udpbuf[udppos+1]) << 8)
                = crc;
         */
         frameio_crcok = 1;
         if (l2_asize==8U) rmnc();
         return 1;
      }
   }
   return 0;
} /* end GetDField() */


static char isdupe(char crc1, char crc2, uint32_t usock)
{
   uint32_t c;
   uint32_t n;
   uint32_t i;
   struct frameio_UDPSOCK * anonym;
   { /* with */
      struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[usock];
      c = (uint32_t)(uint8_t)crc1+(uint32_t)(uint8_t)
                crc2*256UL+65536UL;
      n = anonym->dupcnt;
      i = l2_dupchk;
      while (i>0UL && anonym->dupcrcs[n]!=c) {
         --i;
         if (n>0UL) --n;
         else n = 31UL;
      }
      if (i>0UL) c = 0UL;
      ++anonym->dupcnt;
      if (anonym->dupcnt>31UL) anonym->dupcnt = 0UL;
      anonym->dupcrcs[anonym->dupcnt] = c;
   }
   return i>0UL;
} /* end isdupe() */


static void getudp(uint32_t usock, char buf[], uint32_t buf_len,
                int32_t * len)
{
   uint32_t fromport;
   uint32_t ipn;
   char crc2;
   char crc1;
   *len = -1L;
   if ((int32_t)frameio_udpsocks0[usock].fd<0L) return;
   *len = udpreceive(frameio_udpsocks0[usock].fd, buf, (int32_t)(buf_len),
                &fromport, &ipn);
   if ((*len>2L && *len<(int32_t)(buf_len))
                && (!frameio_udpsocks0[usock]
                .checkip || frameio_udpsocks0[usock].ipnum==ipn)) {
      /*WrInt(udpsend(usock^.fd, buf, len, usock^.dport, usock^.ip), 1);
                WrStrLn(" us"); */
      *len -= 2L;
      crc1 = buf[*len];
      crc2 = buf[*len+1L];
      if (l2_dupchk>0UL && isdupe(crc1, crc2, usock)) {
         if (l2_l2verb) osi_WrStrLn("axudp dupe deleted", 19ul);
         *len = -1L;
         return;
      }
      aprsstr_AppCRC(buf, buf_len, *len);
      if (crc1!=buf[*len] || crc2!=buf[*len+1L]) {
         if (l2_l2verb) osi_WrStrLn(" axudp crc error", 17ul);
         *len = -1L;
         return;
      }
      if (buf[0UL]=='\001') {
         aprsstr_extrudp2(buf, buf_len, l2_udp2buf, 100ul, len);
         if (l2_udp2buf[1U]!='?') {
            frameio_udpsocks0[usock].dcd = ((uint8_t)(uint8_t)
                l2_udp2buf[1U]&0x2U)!=0U;
            frameio_udpsocks0[usock].hastxdata = ((uint8_t)(uint8_t)
                l2_udp2buf[1U]&0x4U)!=0U;
         }
         if (*len==0L) *len = -1L;
         axv2 = 1;
      }
      else {
         /*WrStr("<");WrStr(udp2buf); WrStrLn(">"); */
         l2_udp2buf[0U] = 0;
         frameio_udpsocks0[usock].dcd = 0;
         frameio_udpsocks0[usock].hastxdata = 0;
         axv2 = 0;
      }
   }
   else *len = -1L;
} /* end getudp() */


extern char frameio_GetFrame(uint32_t port)
{
   int32_t i;
   if (port==0UL) return 0;
   getudp(port-1UL, udpbuf, 1601ul, &i);
   if (i<0L) return 0;
   /*WrInt(i, 5); WrStrLn("=udpin");   */
   size = (uint32_t)i;
   from = 0UL;
   if (GetAField(port) && GetDField()) return 1;
   return 0;
} /* end GetFrame() */


extern void frameio_Modempoll(uint32_t tport)
{
   char b[100];
   int32_t ret;
   /*WrStrLn("send ?"); */
   struct frameio_UDPSOCK * anonym;
   if (tport==0UL || tport>15UL) return;
   b[0U] = '\001';
   b[1U] = '?';
   b[2U] = 0;
   aprsstr_AppCRC(b, 100ul, 3L);
   { /* with */
      struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[tport-1UL];
      ret = udpsend(anonym->fd, b, 5L, anonym->toport, anonym->ipnum);
   }
} /* end Modempoll() */


extern char frameio_DCD(uint32_t port)
{
   struct frameio_UDPSOCK * anonym;
   if (port==0UL || port-1UL>14UL) return 0;
   { /* with */
      struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[port-1UL];
      if (anonym->dcd) {
         if (anonym->dcdretry>500UL) {
            frameio_Modempoll(port);
            anonym->dcdretry = 0UL;
         }
         else ++anonym->dcdretry;
      }
      else anonym->dcdretry = 0UL;
      /*IF dcd THEN WrStrLn("dcd") END; */
      return anonym->dcd;
   }
} /* end DCD() */


extern char frameio_Sending(uint32_t port)
{
   struct frameio_UDPSOCK * anonym;
   if (port==0UL || port-1UL>14UL) return 0;
   { /* with */
      struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[port-1UL];
      if (anonym->hastxdata) {
         if (anonym->txbufretry>500UL) {
            frameio_Modempoll(port);
            anonym->txbufretry = 0UL;
         }
         else ++anonym->txbufretry;
      }
      else {
         /*WrStr("txbufretry:");WrInt(txbufretry, 1);WrStrLn("");  */
         anonym->txbufretry = 0UL;
      }
      /*IF hastxdata THEN WrStrLn("txing") END; */
      return anonym->hastxdata;
   }
} /* end Sending() */


extern void frameio_SendFrame(uint32_t tport, uint32_t Baud,
                char Adress[], uint32_t Adress_len, uint32_t AdrLen,
                l2_pDATA dp)
{
   uint32_t j;
   uint32_t i;
   char b[351];
   int32_t ret;
   struct frameio_UDPSOCK * anonym;
   uint32_t tmp;
   if ((tport==0UL || tport>15UL) || AdrLen>=350UL) return;
   --tport;
   j = 0UL;
   tmp = AdrLen-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      b[j] = Adress[i];
      ++j;
      if (i==tmp) break;
   } /* end for */
   if (dp) {
      tmp = (uint32_t)(dp->len-1U);
      i = 0UL;
      if (i<=tmp) for (;; i++) {
         b[j] = dp->info[i];
         ++j;
         if (i==tmp) break;
      } /* end for */
   }
   aprsstr_AppCRC(b, 351ul, (int32_t)j);
   { /* with */
      struct frameio_UDPSOCK * anonym = &frameio_udpsocks0[tport];
      ret = udpsend(anonym->fd, b, (int32_t)(j+2UL), anonym->toport,
                anonym->ipnum);
      anonym->hastxdata = axv2;
   }
} /* end SendFrame() */


extern void frameio_Init(void)
{
   axv2 = 0;
/*  FILL(ADR(udpsocks), 0C, SIZE(udpsocks)); */
/*  udpsocks[0].ip:=7F000001H; */
/*  IF NOT opensock(0, 5603, 5602) THEN WrStrLn("socket open error");
                HALT END; */
/*  udpsocks[0].ip:=44<<24 + 143<<16 + 40<<8 + 90; */
/*  IF NOT opensock(0, 10094, 10094) THEN WrStrLn("socket open error");
                HALT END; */
/*  udpsocks[0].ip:=192<<24 + 168<<16 + 1<<8 + 30; */
/*  IF NOT opensock(0, 11000, 11001) THEN WrStrLn("socket open error");
                HALT END; */
/*  udpsocks[0].ip:=127<<24 + 0<<16 + 0<<8 + 1; */
/*  IF NOT opensock(0, 2603, 2602) THEN WrStrLn("socket open error");
                HALT END; */
} /* end Init() */


extern void frameio_BEGIN(void)
{
   static int frameio_init = 0;
   if (frameio_init) return;
   frameio_init = 1;
   l2_BEGIN();
   aprsstr_BEGIN();
   osi_BEGIN();
}

