/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef deflate_H_
#include "deflate.h"
#endif
#define deflate_C_
#ifndef osi_H_
#include "osi.h"
#endif





/* data compressor simplified deflate-lz77 with low overhead for stream and blocks */

static uint32_t Hash(char a, char b, char c)
{
   return (uint32_t)(uint8_t)a+131UL*(uint32_t)(uint8_t)
                b+851UL*(uint32_t)(uint8_t)c&4095UL;
} /* end Hash() */
/* 131 831 */


extern void deflate_Initexpand(struct deflate_XCONTEXT * c)
{
   uint32_t i;
   for (i = 0UL; i<=32767UL; i++) {
      c->ring[i] = 0;
   } /* end for */
   c->wp = 0UL;
   c->rxbits = 0UL;
   c->rxbitbuf = 0UL;
   c->rawlen = 0L;
   c->compdata = 0;
} /* end Initexpand() */


extern void deflate_Initdeflate(struct deflate_CONTEXT * c)
{
   uint32_t i;
   for (i = 0UL; i<=32767UL; i++) {
      c->ring[i] = 0;
      c->hashchain[i] = (uint16_t)i;
   } /* end for */
   for (i = 0UL; i<=4095UL; i++) {
      c->hash[i] = 32768U;
   } /* end for */
   /*  c.hash[0]:=HIGH(c.ring); */
   c->wp = 0UL;
   c->mlen = 0UL;
   c->flen = 0UL;
   c->savep = 32768UL;
   c->rawr = 0UL;
   c->comr = 0UL;
   c->sizedif = -32L;
   c->raww = 0UL;
   c->comw = 0UL;
   c->rawwo = 0UL;
   c->txbitc = 0UL;
   c->txbitbuf = 0UL;
   c->wascomp = 0;
} /* end Initdeflate() */

/*
PROCEDURE Hashring(c-:CONTEXT; p:CARDINAL):INTEGER;
BEGIN RETURN Hash(c.ring[p], c.ring[(p+1) MOD BSIZE],
                c.ring[(p+2) MOD BSIZE]) END Hashring;
*/

static void findmatch(struct deflate_CONTEXT * c, uint32_t * dist,
                uint32_t * len)
{
   uint32_t do0;
   uint32_t d;
   uint32_t i;
   uint32_t rp;
   uint32_t hp;
   *len = 0UL;
   do0 = 0UL;
   if (c->lastidx>=32768UL) {
      for (d = 2UL; d>=1UL; d--) {
         /* search in 1 and 2 byte with no hash */
         hp = (c->wp+32768UL)-d&32767UL;
         i = 0UL;
         rp = hp;
         while (i<c->mlen && c->ring[rp]==c->matchbuf[i]) {
            ++i;
            rp = rp+1UL&32767UL;
            if (rp==c->wp) rp = hp;
         }
         if (i>=3UL) {
            *len = i;
            *dist = d;
         }
         if (i>=c->mlen) return;
      } /* end for */
      hp = (uint32_t)c->hash[Hash(c->matchbuf[0UL], c->matchbuf[1UL],
                c->matchbuf[2UL])]; /* start hash chain */
   }
   else hp = c->lastidx;
   if (hp>=32768UL) return;
   /* no hash so no fitting 3 bytes */
   for (;;) {
      d = (c->wp+32768UL)-hp&32767UL; /* distance to ring write pointer */
      if (d<=do0) break;
      do0 = d;
      if (c->ring[hp+*len&32767UL]==c->matchbuf[*len]) {
         /* quick test to save cpu */
         i = 0UL;
         rp = hp;
         while (i<c->mlen && c->ring[rp]==c->matchbuf[i]) {
            /* compare strings */
            ++i;
            rp = rp+1UL&32767UL;
            if (rp==c->wp) rp = hp;
         }
         if (i>*len) {
            /*& (d>=3)*/
            /* new longest match */
            *len = i;
            *dist = d;
            if (i>=c->mlen) {
               c->lastidx = hp; /* continue at next search from here */
               break;
            }
         }
      }
      hp = (uint32_t)c->hashchain[hp]; /* go on in hash chain */
   }
} /* end findmatch() */


static void stohash(struct deflate_CONTEXT * c)
{
   short hi;
   uint32_t p;
   p = c->wp+32765UL&32767UL;
   hi = (short)Hash(c->ring[p], c->ring[p+1UL&32767UL],
                c->ring[p+2UL&32767UL]);
   if (c->hash[hi]<32768U) c->hashchain[p] = c->hash[hi];
   c->hash[hi] = (uint16_t)p;
} /* end stohash() */


static void codelen(uint32_t len, uint32_t dist, uint32_t * clen,
                uint32_t * nlen, uint32_t * dlen, uint32_t * nd)
{
   switch (len) {
   case 3UL:
   case 4UL:
   case 5UL:
   case 6UL:
   case 7UL:
   case 8UL:
   case 9UL:
   case 10UL:
      *clen = 7UL;
      *nlen = len-2UL;
      break;
   case 11UL:
   case 12UL:
   case 13UL:
   case 14UL:
   case 15UL:
   case 16UL:
   case 17UL:
   case 18UL:
      *clen = 8UL;
      *nlen = (9UL+(len-11UL)/2UL)*2UL+(len-11UL&1UL);
      break;
   case 19UL:
   case 20UL:
   case 21UL:
   case 22UL:
   case 23UL:
   case 24UL:
   case 25UL:
   case 26UL:
   case 27UL:
   case 28UL:
   case 29UL:
   case 30UL:
   case 31UL:
   case 32UL:
   case 33UL:
   case 34UL:
      *clen = 9UL;
      *nlen = (13UL+(len-19UL)/4UL)*4UL+(len-19UL&3UL);
      break;
   case 35UL:
   case 36UL:
   case 37UL:
   case 38UL:
   case 39UL:
   case 40UL:
   case 41UL:
   case 42UL:
   case 43UL:
   case 44UL:
   case 45UL:
   case 46UL:
   case 47UL:
   case 48UL:
   case 49UL:
   case 50UL:
   case 51UL:
   case 52UL:
   case 53UL:
   case 54UL:
   case 55UL:
   case 56UL:
   case 57UL:
   case 58UL:
   case 59UL:
   case 60UL:
   case 61UL:
   case 62UL:
   case 63UL:
   case 64UL:
   case 65UL:
   case 66UL:
      *clen = 10UL;
      *nlen = (17UL+(len-35UL)/8UL)*8UL+(len-35UL&7UL);
      break;
   case 67UL:
   case 68UL:
   case 69UL:
   case 70UL:
   case 71UL:
   case 72UL:
   case 73UL:
   case 74UL:
   case 75UL:
   case 76UL:
   case 77UL:
   case 78UL:
   case 79UL:
   case 80UL:
   case 81UL:
   case 82UL:
   case 83UL:
   case 84UL:
   case 85UL:
   case 86UL:
   case 87UL:
   case 88UL:
   case 89UL:
   case 90UL:
   case 91UL:
   case 92UL:
   case 93UL:
   case 94UL:
   case 95UL:
   case 96UL:
   case 97UL:
   case 98UL:
   case 99UL:
   case 100UL:
   case 101UL:
   case 102UL:
   case 103UL:
   case 104UL:
   case 105UL:
   case 106UL:
   case 107UL:
   case 108UL:
   case 109UL:
   case 110UL:
   case 111UL:
   case 112UL:
   case 113UL:
   case 114UL:
      *clen = 11UL;
      *nlen = (21UL+(len-67UL)/16UL)*16UL+(len-67UL&15UL);
      break;
   case 115UL:
   case 116UL:
   case 117UL:
   case 118UL:
   case 119UL:
   case 120UL:
   case 121UL:
   case 122UL:
   case 123UL:
   case 124UL:
   case 125UL:
   case 126UL:
   case 127UL:
   case 128UL:
   case 129UL:
   case 130UL:
      *clen = 12UL;
      *nlen = 3072UL+(len-115UL&15UL);
      break;
   case 131UL:
   case 132UL:
   case 133UL:
   case 134UL:
   case 135UL:
   case 136UL:
   case 137UL:
   case 138UL:
   case 139UL:
   case 140UL:
   case 141UL:
   case 142UL:
   case 143UL:
   case 144UL:
   case 145UL:
   case 146UL:
   case 147UL:
   case 148UL:
   case 149UL:
   case 150UL:
   case 151UL:
   case 152UL:
   case 153UL:
   case 154UL:
   case 155UL:
   case 156UL:
   case 157UL:
   case 158UL:
   case 159UL:
   case 160UL:
   case 161UL:
   case 162UL:
   case 163UL:
   case 164UL:
   case 165UL:
   case 166UL:
   case 167UL:
   case 168UL:
   case 169UL:
   case 170UL:
   case 171UL:
   case 172UL:
   case 173UL:
   case 174UL:
   case 175UL:
   case 176UL:
   case 177UL:
   case 178UL:
   case 179UL:
   case 180UL:
   case 181UL:
   case 182UL:
   case 183UL:
   case 184UL:
   case 185UL:
   case 186UL:
   case 187UL:
   case 188UL:
   case 189UL:
   case 190UL:
   case 191UL:
   case 192UL:
   case 193UL:
   case 194UL:
   case 195UL:
   case 196UL:
   case 197UL:
   case 198UL:
   case 199UL:
   case 200UL:
   case 201UL:
   case 202UL:
   case 203UL:
   case 204UL:
   case 205UL:
   case 206UL:
   case 207UL:
   case 208UL:
   case 209UL:
   case 210UL:
   case 211UL:
   case 212UL:
   case 213UL:
   case 214UL:
   case 215UL:
   case 216UL:
   case 217UL:
   case 218UL:
   case 219UL:
   case 220UL:
   case 221UL:
   case 222UL:
   case 223UL:
   case 224UL:
   case 225UL:
   case 226UL:
   case 227UL:
   case 228UL:
   case 229UL:
   case 230UL:
   case 231UL:
   case 232UL:
   case 233UL:
   case 234UL:
   case 235UL:
   case 236UL:
   case 237UL:
   case 238UL:
   case 239UL:
   case 240UL:
   case 241UL:
   case 242UL:
   case 243UL:
   case 244UL:
   case 245UL:
   case 246UL:
   case 247UL:
   case 248UL:
   case 249UL:
   case 250UL:
   case 251UL:
   case 252UL:
   case 253UL:
   case 254UL:
   case 255UL:
   case 256UL:
   case 257UL:
      *clen = 13UL;
      *nlen = (193UL+(len-131UL)/32UL)*32UL+(len-131UL&31UL);
      break;
   default:;
      *clen = 8UL;
      *nlen = 285UL;
      break;
   } /* end switch */
   /*
     CASE dist OF
        1..4:     dlen:=5;    nd:=dist-1;
      | 5..6:     dlen:=6;    nd:=4*2+(dist-5) MOD 2;
      | 7..8:     dlen:=6;    nd:=5*2+(dist-7) MOD 2;
      | 9..12:    dlen:=7;    nd:=6*4+(dist-9) MOD 4;
      |13..16:    dlen:=7;    nd:=7*4+(dist-13) MOD 4;
      |17..24:    dlen:=8;    nd:=8*8+(dist-17) MOD 8;
      |25..32:    dlen:=8;    nd:=9*8+(dist-25) MOD 8;
      |33..48:    dlen:=9;    nd:=10*16+(dist-33) MOD 16;
      |49..64:    dlen:=9;    nd:=11*16+(dist-49) MOD 16;
      |65..96:    dlen:=10;   nd:=12*32+(dist-65) MOD 32;
      |97..128:   dlen:=10;   nd:=13*32+(dist-97) MOD 32;
      |129..192:  dlen:=11;   nd:=14*64+(dist-129) MOD 64;
      |193..256:  dlen:=11;   nd:=15*64+(dist-193) MOD 64;
      |257..384:  dlen:=12;   nd:=16*128+(dist-257) MOD 128;
      |385..512:  dlen:=12;   nd:=17*128+(dist-385) MOD 128;
      |513..768:  dlen:=13;   nd:=18*256+(dist-513) MOD 256;
      |769..1024: dlen:=13;   nd:=19*256+(dist-769) MOD 256;
      |1025..1536:dlen:=14;   nd:=20*512+(dist-1025) MOD 512;
      |1537..2048:dlen:=14;   nd:=21*512+(dist-1537) MOD 512;
      |2049..3072:dlen:=15;   nd:=22*1024+(dist-2049) MOD 1024;
      |3073..4096:dlen:=15;   nd:=23*1024+(dist-3073) MOD 1024;
      |4097..6144:dlen:=16;   nd:=24*2048+(dist-4097) MOD 2048;
      |6145..8192:dlen:=16;   nd:=25*2048+(dist-6145) MOD 2048;
      |8193..12288:dlen:=17;  nd:=26*4096+(dist-8193) MOD 4096;
      |12289..16384:dlen:=17; nd:=27*4096+(dist-12289) MOD 4096;
      |16385..24576:dlen:=18; nd:=28*8192+(dist-16385) MOD 8192;
      |24577..32768:dlen:=18; nd:=29*8192+(dist-24577) MOD 8192;
     END;
   */
   switch (dist) {
   case 1UL:
   case 2UL:
   case 3UL:
   case 4UL:
      *dlen = 5UL;
      *nd = dist-1UL;
      break;
   case 5UL:
   case 6UL:
      *dlen = 6UL;
      *nd = 8UL+(dist-5UL&1UL);
      break;
   case 7UL:
   case 8UL:
      *dlen = 6UL;
      *nd = 10UL+(dist-7UL&1UL);
      break;
   case 9UL:
   case 10UL:
   case 11UL:
   case 12UL:
      *dlen = 7UL;
      *nd = 24UL+(dist-9UL&3UL);
      break;
   case 13UL:
   case 14UL:
   case 15UL:
   case 16UL:
      *dlen = 7UL;
      *nd = 28UL+(dist-13UL&3UL);
      break;
   case 17UL:
   case 18UL:
   case 19UL:
   case 20UL:
   case 21UL:
   case 22UL:
   case 23UL:
   case 24UL:
      *dlen = 8UL;
      *nd = 64UL+(dist-17UL&7UL);
      break;
   case 25UL:
   case 26UL:
   case 27UL:
   case 28UL:
   case 29UL:
   case 30UL:
   case 31UL:
   case 32UL:
      *dlen = 8UL;
      *nd = 72UL+(dist-25UL&7UL);
      break;
   case 33UL:
   case 34UL:
   case 35UL:
   case 36UL:
   case 37UL:
   case 38UL:
   case 39UL:
   case 40UL:
   case 41UL:
   case 42UL:
   case 43UL:
   case 44UL:
   case 45UL:
   case 46UL:
   case 47UL:
   case 48UL:
      *dlen = 9UL;
      *nd = 160UL+(dist-33UL&15UL);
      break;
   case 49UL:
   case 50UL:
   case 51UL:
   case 52UL:
   case 53UL:
   case 54UL:
   case 55UL:
   case 56UL:
   case 57UL:
   case 58UL:
   case 59UL:
   case 60UL:
   case 61UL:
   case 62UL:
   case 63UL:
   case 64UL:
      *dlen = 9UL;
      *nd = 176UL+(dist-49UL&15UL);
      break;
   case 65UL:
   case 66UL:
   case 67UL:
   case 68UL:
   case 69UL:
   case 70UL:
   case 71UL:
   case 72UL:
   case 73UL:
   case 74UL:
   case 75UL:
   case 76UL:
   case 77UL:
   case 78UL:
   case 79UL:
   case 80UL:
   case 81UL:
   case 82UL:
   case 83UL:
   case 84UL:
   case 85UL:
   case 86UL:
   case 87UL:
   case 88UL:
   case 89UL:
   case 90UL:
   case 91UL:
   case 92UL:
   case 93UL:
   case 94UL:
   case 95UL:
   case 96UL:
      *dlen = 10UL;
      *nd = 384UL+(dist-65UL&31UL);
      break;
   case 97UL:
   case 98UL:
   case 99UL:
   case 100UL:
   case 101UL:
   case 102UL:
   case 103UL:
   case 104UL:
   case 105UL:
   case 106UL:
   case 107UL:
   case 108UL:
   case 109UL:
   case 110UL:
   case 111UL:
   case 112UL:
   case 113UL:
   case 114UL:
   case 115UL:
   case 116UL:
   case 117UL:
   case 118UL:
   case 119UL:
   case 120UL:
   case 121UL:
   case 122UL:
   case 123UL:
   case 124UL:
   case 125UL:
   case 126UL:
   case 127UL:
   case 128UL:
      *dlen = 10UL;
      *nd = 416UL+(dist-97UL&31UL);
      break;
   case 129UL:
   case 130UL:
   case 131UL:
   case 132UL:
   case 133UL:
   case 134UL:
   case 135UL:
   case 136UL:
   case 137UL:
   case 138UL:
   case 139UL:
   case 140UL:
   case 141UL:
   case 142UL:
   case 143UL:
   case 144UL:
   case 145UL:
   case 146UL:
   case 147UL:
   case 148UL:
   case 149UL:
   case 150UL:
   case 151UL:
   case 152UL:
   case 153UL:
   case 154UL:
   case 155UL:
   case 156UL:
   case 157UL:
   case 158UL:
   case 159UL:
   case 160UL:
   case 161UL:
   case 162UL:
   case 163UL:
   case 164UL:
   case 165UL:
   case 166UL:
   case 167UL:
   case 168UL:
   case 169UL:
   case 170UL:
   case 171UL:
   case 172UL:
   case 173UL:
   case 174UL:
   case 175UL:
   case 176UL:
   case 177UL:
   case 178UL:
   case 179UL:
   case 180UL:
   case 181UL:
   case 182UL:
   case 183UL:
   case 184UL:
   case 185UL:
   case 186UL:
   case 187UL:
   case 188UL:
   case 189UL:
   case 190UL:
   case 191UL:
   case 192UL:
      *dlen = 11UL;
      *nd = 896UL+(dist-129UL&63UL);
      break;
   case 193UL:
   case 194UL:
   case 195UL:
   case 196UL:
   case 197UL:
   case 198UL:
   case 199UL:
   case 200UL:
   case 201UL:
   case 202UL:
   case 203UL:
   case 204UL:
   case 205UL:
   case 206UL:
   case 207UL:
   case 208UL:
   case 209UL:
   case 210UL:
   case 211UL:
   case 212UL:
   case 213UL:
   case 214UL:
   case 215UL:
   case 216UL:
   case 217UL:
   case 218UL:
   case 219UL:
   case 220UL:
   case 221UL:
   case 222UL:
   case 223UL:
   case 224UL:
   case 225UL:
   case 226UL:
   case 227UL:
   case 228UL:
   case 229UL:
   case 230UL:
   case 231UL:
   case 232UL:
   case 233UL:
   case 234UL:
   case 235UL:
   case 236UL:
   case 237UL:
   case 238UL:
   case 239UL:
   case 240UL:
   case 241UL:
   case 242UL:
   case 243UL:
   case 244UL:
   case 245UL:
   case 246UL:
   case 247UL:
   case 248UL:
   case 249UL:
   case 250UL:
   case 251UL:
   case 252UL:
   case 253UL:
   case 254UL:
   case 255UL:
   case 256UL:
      *dlen = 11UL;
      *nd = 960UL+(dist-193UL&63UL);
      break;
   default:;
      if (dist<=384UL) {
         *dlen = 12UL;
         *nd = 2048UL+(dist-257UL&127UL);
      }
      else if (dist<=512UL) {
         *dlen = 12UL;
         *nd = 2176UL+(dist-385UL&127UL);
      }
      else if (dist<=768UL) {
         *dlen = 13UL;
         *nd = 4608UL+(dist-513UL&255UL);
      }
      else if (dist<=1024UL) {
         *dlen = 13UL;
         *nd = 4864UL+(dist-769UL&255UL);
      }
      else if (dist<=1536UL) {
         *dlen = 14UL;
         *nd = 10240UL+(dist-1025UL&511UL);
      }
      else if (dist<=2048UL) {
         *dlen = 14UL;
         *nd = 10752UL+(dist-1537UL&511UL);
      }
      else if (dist<=3072UL) {
         *dlen = 15UL;
         *nd = 22528UL+(dist-2049UL&1023UL);
      }
      else if (dist<=4096UL) {
         *dlen = 15UL;
         *nd = 23552UL+(dist-3073UL&1023UL);
      }
      else if (dist<=6144UL) {
         *dlen = 16UL;
         *nd = 49152UL+(dist-4097UL&2047UL);
      }
      else if (dist<=8192UL) {
         *dlen = 16UL;
         *nd = 51200UL+(dist-6145UL&2047UL);
      }
      else if (dist<=12288UL) {
         *dlen = 17UL;
         *nd = 106496UL+(dist-8193UL&4095UL);
      }
      else if (dist<=16384UL) {
         *dlen = 17UL;
         *nd = 110592UL+(dist-12289UL&4095UL);
      }
      else if (dist<=24576UL) {
         *dlen = 18UL;
         *nd = 229376UL+(dist-16385UL&8191UL);
      }
      else {
         *dlen = 18UL;
         *nd = 237568UL+(dist-24577UL&8191UL);
      }
      break;
   } /* end switch */
} /* end codelen() */


static void txbits(struct deflate_CONTEXT * c, uint32_t b, uint32_t len,
                char dbuf[], uint32_t dbuf_len, int32_t * outlen)
{
   /*  c.txbitbuf:=c.txbitbuf<<len + b; */
   c->txbitbuf = (uint32_t)X2C_LSH((uint32_t)c->txbitbuf,32,
                (int32_t)len)+b;
   c->txbitc += len;
   if (len==0UL) {
      /* flush */
      while (c->txbitc&7UL) {
         /* fill zero to full byte */
         ++c->txbitc;
         c->txbitbuf = c->txbitbuf*2UL;
      }
   }
   /*    c.txbitc:=0; */
   /*WrStrLn("txbit-flush");  */
   if (*outlen>=0L) {
      while (c->txbitc>=8UL) {
         dbuf[*outlen] = (char)(uint32_t)X2C_LSH((uint32_t)
                c->txbitbuf,32,8L-(int32_t)c->txbitc);
         ++*outlen;
         if (*outlen>(int32_t)(dbuf_len-1)) *outlen = -2L;
         c->txbitc -= 8UL;
      }
   }
} /* end txbits() */


static void wrrawblock(struct deflate_CONTEXT * c, char dbuf[],
                uint32_t dbuf_len, int32_t * outlen)
{
   uint32_t b;
   uint32_t n;
   uint32_t i;
   uint32_t tmp;
   txbits(c, 1UL, 1UL, dbuf, dbuf_len, outlen); /* switch to raw */
   if (c->txbitc==0UL) b = 8UL;
   else b = 16UL-c->txbitc;
   n = (c->rawwo+32768UL)-c->rawr&32767UL;
   i = 1UL;
   /*  i:=i<<b; */
   i = (uint32_t)X2C_LSH(0x1UL,32,(int32_t)b);
   if (i<n) n = i;
   txbits(c, n-1UL, b, dbuf, dbuf_len, outlen);
   tmp = n-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      /* send all raw data */
      txbits(c, (uint32_t)(uint8_t)c->ring[c->rawr], 8UL, dbuf, dbuf_len,
                 outlen);
      c->rawr = c->rawr+1UL&32767UL;
      if (i==tmp) break;
   } /* end for */
} /* end wrrawblock() */


static void wrcomp(struct deflate_CONTEXT * c, char dbuf[],
                uint32_t dbuf_len, int32_t * outlen)
{
   uint32_t dl;
   uint32_t ll;
   uint32_t d;
   uint32_t l;
   uint32_t dist;
   uint32_t len;
   while (c->comr!=c->comw) {
      /* send all compressed data */
      len = c->combuf[c->comr]&65535UL;
      dist = c->combuf[c->comr]/65536UL;
      if (dist>0UL) codelen(len, dist, &ll, &l, &dl, &d);
      else if (len<144UL) {
         ll = 8UL;
         l = len+48UL;
      }
      else if (len<256UL) {
         /*WrStr("(");WrStr(CHR(len));WrStr(")"); */
         ll = 9UL;
         l = len+256UL;
      }
      else if (len==256UL) {
         ll = 7UL;
         l = 0UL;
      }
      else {
         ll = 8UL;
         l = len+192UL;
      }
      /*IF dist>0 THEN WrStr("L(");WrInt(len,1);ELSE WrStr("C(");
                WrHex(len,1); END; WrStr(")"); */
      txbits(c, l, ll, dbuf, dbuf_len, outlen);
      if (dist>0UL) {
         /*WrStr("D(");WrInt(dist,1);WrStr(")"); */
         txbits(c, d, dl, dbuf, dbuf_len, outlen);
      }
      c->comr = c->comr+1UL&2047UL;
   }
} /* end wrcomp() */


static void endcomp(struct deflate_CONTEXT * c, char finish,
                char dbuf[], uint32_t dbuf_len, int32_t * outlen)
{
   txbits(c, 0UL, 7UL, dbuf, dbuf_len, outlen);
   if (finish) txbits(c, 0UL, 1UL, dbuf, dbuf_len, outlen);
/*  c.savep:=BSIZE; */
/*  c.mlen:=0; */
/*  txbits(c, 1, 1); */
} /* end endcomp() */


static void checkbufs(struct deflate_CONTEXT * c, char flush,
                char dbuf[], uint32_t dbuf_len, int32_t * outlen)
{
   /*WrStr("(");WrInt(c.sizedif, 1);WrStr(")");   */
   if (c->sizedif>=0L) {
      /* compressed state or compare */
      if (!c->wascomp) {
         while (c->rawr!=c->rawwo) wrrawblock(c, dbuf, dbuf_len, outlen);
         txbits(c, 0UL, 1UL, dbuf, dbuf_len, outlen);
                /* switch to compressed */
         /*WrStrLn("");WrStrLn(" TOCOMP "); */
         c->wascomp = 1;
         c->sizedif = 32L;
      }
      if (c->sizedif>=32L || ((c->comw+2048UL)-c->comr&2047UL)>=1791UL) {
         wrcomp(c, dbuf, dbuf_len, outlen); /* send all compressed data */
         c->rawr = c->raww; /* delete all raw data */
         c->rawwo = c->raww;
         c->sizedif = 32L;
      }
      if (flush) {
         wrcomp(c, dbuf, dbuf_len, outlen); /* send all comprssed data */
         endcomp(c, flush=='\002', dbuf, dbuf_len, outlen);
      }
   }
   else {
      /*WrStrLn("-endcomp-"); WrInt(sum DIV 8, 12); WrInt(sum1, 12);
                WrStrLn(""); */
      /* uncompressed state or compare */
      if (c->wascomp) {
         endcomp(c, 0, dbuf, dbuf_len, outlen);
         /*WrStrLn("");WrStrLn(" TORAW "); */
         c->wascomp = 0;
         c->sizedif = -32L;
      }
      if (c->sizedif<=-32L || ((c->raww+32768UL)-c->rawr&32767UL)>=16384UL) {
         /* uncompressed */
         c->rawwo = c->raww; /* undeletable to raw buffer */
         while (((c->rawwo+32768UL)-c->rawr&32767UL)>=16384UL) {
            wrrawblock(c, dbuf, dbuf_len, outlen);
         }
         c->comr = c->comw; /* delete all compressd data */
         c->sizedif = -32L;
      }
      if (flush) {
         c->comr = c->comw; /* delete all compressd data */
         c->rawwo = c->raww;
         while (c->rawr!=c->rawwo) wrrawblock(c, dbuf, dbuf_len, outlen);
         if (flush=='\002') {
            /* with EOF */
            txbits(c, 0UL, 1UL, dbuf, dbuf_len, outlen);
                /* switch to compressed */
            endcomp(c, 0, dbuf, dbuf_len, outlen);
                /* zero data compressed is EOF*/
         }
      }
   }
/*      c.comr:=c.comw;  */
} /* end checkbufs() */


static void sendchar(struct deflate_CONTEXT * c, char dbuf[],
                uint32_t dbuf_len, int32_t * outlen)
{
   uint32_t code;
   code = (uint32_t)(uint8_t)c->ring[c->savep];
   c->combuf[c->comw] = code;
   c->comw = c->comw+1UL&2047UL;
   c->raww = c->savep+1UL&32767UL;
   if (code>143UL) --c->sizedif;
   checkbufs(c, 0, dbuf, dbuf_len, outlen);
} /* end sendchar() */


static void send(struct deflate_CONTEXT * c, uint32_t len, uint32_t dist,
                 char dbuf[], uint32_t dbuf_len, int32_t * outlen)
{
   uint32_t nl;
   uint32_t nd;
   uint32_t dlen;
   uint32_t clen;
   uint32_t p1;
   uint32_t p0;
   uint32_t p;
   checkbufs(c, 0, dbuf, dbuf_len, outlen);
   c->combuf[c->comw] = len+65536UL*dist;
   c->comw = c->comw+1UL&2047UL;
   codelen(len, dist, &clen, &nl, &dlen, &nd);
   p0 = c->wp+(32768UL-dist)&32767UL;
   p = p0;
   p1 = c->wp;
   while (len>0UL) {
      c->ring[c->wp] = c->ring[p];
      c->wp = c->wp+1UL&32767UL;
      stohash(c);
      c->sizedif += 8L;
      /*WrBin(1, c.ring[p], 1); */
      c->raww = c->wp;
      p = p+1UL&32767UL;
      if (p==p1) p = p0;
      --len;
   }
   c->sizedif -= (int32_t)(clen+dlen);
} /* end send() */


extern void deflate_Deflatbyte(struct deflate_CONTEXT * c, char ch,
                char flush, char dbuf[], uint32_t dbuf_len,
                int32_t * outlen)
/* 1C send no blockend, 2C send it */
{
   uint32_t len;
   uint32_t dist;
   uint32_t i;
   uint32_t tmp;
   if (flush==0) {
      c->matchbuf[c->mlen] = ch;
      ++c->mlen;
   }
   /*WrStr("{");WrInt(c.mlen, 1);WrStr("}"); */
   for (;;) {
      if (c->mlen<3UL) {
         c->lastidx = 32768UL;
         break;
      }
      findmatch(c, &dist, &len);
      if (len==c->mlen) {
         /* match found */
         if (c->savep<32768UL) {
            stohash(c);
            sendchar(c, dbuf, dbuf_len, outlen);
            c->savep = 32768UL;
            c->flen = len;
            c->fdist = dist;
         }
         else {
            c->flen = len;
            c->fdist = dist;
         }
         if (c->mlen>=257UL || flush) {
            /* no longer match allowed so send it now */
            send(c, c->flen, c->fdist, dbuf, dbuf_len, outlen);
            c->mlen = 0UL;
            c->flen = 0UL;
         }
         break;
      }
      /* no match found */
      c->lastidx = 32768UL;
      if (c->savep<32768UL) {
         /* send old variante */
         if (c->flen>0UL) {
            c->wp = c->wp+32767UL&32767UL;
                /* restore ring to 1 char before */
            send(c, c->flen, c->fdist, dbuf, dbuf_len, outlen);
            c->matchbuf[0UL] = c->matchbuf[c->mlen-1UL];
                /* not found last char */
            c->mlen = 1UL;
            c->flen = 0UL;
            c->savep = 32768UL;
         }
         else {
            stohash(c);
            sendchar(c, dbuf, dbuf_len, outlen);
            c->savep = 32768UL;
         }
      }
      else {
         c->ring[c->wp] = c->matchbuf[0UL];
         c->savep = c->wp; /* remove first char */
         c->wp = c->wp+1UL&32767UL;
         tmp = c->mlen-1UL;
         i = 1UL;
         if (i<=tmp) for (;; i++) {
            c->matchbuf[i-1UL] = c->matchbuf[i];
            if (i==tmp) break;
         } /* end for */
         --c->mlen;
      }
   }
   if (flush) {
      if (c->savep<32768UL) {
         /* send first char of string */
         stohash(c);
         sendchar(c, dbuf, dbuf_len, outlen);
         c->savep = 32768UL;
      }
      tmp = c->mlen;
      i = 1UL;
      if (i<=tmp) for (;; i++) {
         /* send rest of string */
         c->ring[c->wp] = c->matchbuf[i-1UL];
         c->savep = c->wp;
         c->wp = c->wp+1UL&32767UL;
         stohash(c);
         sendchar(c, dbuf, dbuf_len, outlen);
         if (i==tmp) break;
      } /* end for */
      /*    wrcomp(c, dbuf, outlen);
                (* send all comprssed data *) */
      c->mlen = 0UL;
      checkbufs(c, flush, dbuf, dbuf_len, outlen);
      txbits(c, 0UL, 0UL, dbuf, dbuf_len, outlen); /* send remaining bits */
      c->wascomp = 0;
      c->flen = 0UL;
      /*    c.comr:=c.comw; */
      c->savep = 32768UL;
   }
/*    c.sizedif:=0; */
/*WrStrLn(""); */
/*FOR i:=0 TO HIGH(c.ring) DO WrStr(c.ring[i]) END; WrStrLn(""); */
} /* end Deflatbyte() */


static void xsend(struct deflate_XCONTEXT * c, char ch, char dbuf[],
                uint32_t dbuf_len, int32_t * outlen)
{
   if (*outlen<0L || *outlen>(int32_t)(dbuf_len-1)) *outlen = -1L;
   else {
      c->ring[c->wp] = ch;
      c->wp = c->wp+1UL&32767UL;
      dbuf[*outlen] = ch;
      ++*outlen;
   }
} /* end xsend() */

/*
PROCEDURE Expandbyte(VAR c:XCONTEXT; ch:CHAR; flush:BOOLEAN;
                VAR dbuf:ARRAY OF CHAR; VAR outlen:INTEGER;
                VAR done:BOOLEAN);

CONST DISTAB=ARRAY OF CARDINAL {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
                257,385,513,
                                769,1025,1537,2049,3073,4097,6145,8193,12289,
                16385,24577,0,0};
      LENTAB=ARRAY OF CARDINAL {0,3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,
                35,43,51,
                                59,67,83,99,115,131,163,195,227,285,0,0};
VAR i, n, dist:CARDINAL;
BEGIN
--WrStr("!"); wh(ch);WrStr("!");
<* IF DEB THEN *>
WrHex(sum3, 5);WrStr("H");
INC(sum3);
<* END *>

  c.rxbitbuf:=c.rxbitbuf*256 + ORD(ch);
  INC(c.rxbits, 8);
  done:=FALSE;  
  WHILE (c.rxbits>=20) OR flush DO
--    i:=1;
--    c.rxbitbuf:=c.rxbitbuf MOD (i<<c.rxbits);
--    c.rxbitbuf:=CAST(CARD32, CAST(SET32, c.rxbitbuf) * CAST(SET32,
                CAST(CARD32,SHIFT(CAST(SET32, i), VAL(INTEGER,
                c.rxbits)))-1));

    c.rxbitbuf:=CAST(CARD32, CAST(SET32, c.rxbitbuf) * SHIFT(SET32{0..31}
                , VAL(INTEGER, c.rxbits)-32));
<* IF DEB THEN *>
WrStr(" ["); WrInt(c.rxbits, 1);WrStr(",");WrHex(c.rxbitbuf,1);WrStr("]");
<* END *>
    IF flush & (c.rxbits<20) THEN              (* append zeros as end of data *)
      c.rxbitbuf:=c.rxbitbuf*256;
      INC(c.rxbits, 8);
    END;

    IF c.rawlen>0 THEN                         (* in uncompressed mode *)
--WrStr("[");WrInt(c.rawlen, 1);WrStr("]");
--      ch:=CHR(c.rxbitbuf>>(c.rxbits-8));
      ch:=CHR(CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 8-VAL(INTEGER,
                c.rxbits))));

      DEC(c.rxbits, 8);
      xsend(c, ch, dbuf, outlen); 
      DEC(c.rawlen);

    ELSIF c.compmod=1C THEN                    (* get lengh word *)
--      IF c.rxbitbuf>>(c.rxbits-7)<24 THEN
      IF CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 7-VAL(INTEGER,
                c.rxbits)))<24 THEN
        c.lencode:=CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 7-VAL(INTEGER,
                 c.rxbits)));
--        c.lencode:=c.rxbitbuf>>(c.rxbits-7);
        DEC(c.rxbits, 7);
<* IF DEB THEN *>
WrInt(c.lencode,3);WrStr("=LENC ");
<* END *>
        IF c.lencode=ENDCOMP-256 THEN          (* end of compressed data *)
<* IF DEB THEN *>
WrStrLn("ENDCOMP");
<* END *>
          IF c.justcomp THEN
<* IF DEB THEN *>
WrStrLn("QUICKENDCOMP");
<* END *>
            done:=TRUE;
WrStrLn(" LIBDONE2 ");
            c.compmod:=0C;
            c.justcomp:=FALSE;
            flush:=FALSE;
          ELSE c.compmod:=3C END; 
        ELSE                                   (* get distance word *) 
          c.compmod:=2C;
          c.justcomp:=FALSE;
        END;                
--      ELSIF c.rxbitbuf>>(c.rxbits-8)<192 THEN  (* literal <144 *)
      ELSIF CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 8-VAL(INTEGER,
                c.rxbits)))<192 THEN  (* literal <144 *)
--WrHex(c.rxbitbuf>>(c.rxbits-8)-48,4);WrStr(",");WrInt(c.rxbits,1);
                WrStr(",");WrInt(ORD(flush),1);WrStr("=LITC ");
--WrStr("<(");WrHex(c.rxbitbuf>>(c.rxbits-8)-48,1); WrStr(")>");

        xsend(c, CHR(CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf),
                8-VAL(INTEGER, c.rxbits)))-48), dbuf, outlen);
        DEC(c.rxbits, 8);
        c.justcomp:=FALSE;

      ELSIF CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 8-VAL(INTEGER,
                c.rxbits)))<200 THEN                 (* len *)
        c.lencode:=CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 8-VAL(INTEGER,
                 c.rxbits))) - (192-24);
        DEC(c.rxbits, 8);
        c.compmod:=2C;                         (* get distance word *)
        c.justcomp:=FALSE;
--WrInt(c.lencode,3);WrStr("=LENCC ");

      ELSE                                     (* literal >=144 *) 
        c.justcomp:=FALSE;
--WrStr("<[");WrHex(c.rxbitbuf>>(c.rxbits-9)-(400-144),1); WrStr("]>");
        xsend(c, CHR(CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf),
                9-VAL(INTEGER, c.rxbits)))-(400-144)), dbuf, outlen);
        DEC(c.rxbits, 9);
        c.compmod:=1C;
      END; 
      IF c.compmod=2C THEN
        IF c.lencode>=9 THEN  (* get extra length bits *)
--          i:=1;
--          c.rxbitbuf:=c.rxbitbuf MOD (i<<c.rxbits);
          c.rxbitbuf:=CAST(CARD32, CAST(SET32,
                c.rxbitbuf) * SHIFT(SET32{0..31}
                , VAL(INTEGER, c.rxbits)-32));

          n:=(c.lencode-5) DIV 4;                (* bit count *)
--          c.lencode:=LENTAB[c.lencode] + c.rxbitbuf>>(VAL(CARDINAL,
                c.rxbits)-n);
          c.lencode:=LENTAB[c.lencode] + CAST(CARD32, SHIFT(CAST(SET32,
                c.rxbitbuf), VAL(INTEGER, n)-VAL(INTEGER, c.rxbits)));
          DEC(c.rxbits, n);
        ELSE c.lencode:=LENTAB[c.lencode] END;
      END;

--WrInt(c.lencode,4);WrStr("=LEN ");

    ELSIF c.compmod=2C THEN                    (* get lengh word *)
--    dist:=c.rxbitbuf>>(c.rxbits-5);          (* fixed 5 bit of distance *)
      dist:=CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), 5-VAL(INTEGER,
                c.rxbits)));
--WrInt(dist,3);WrStr("=DIST5 ");
      DEC(c.rxbits, 5);
      IF dist>=4 THEN 
--        i:=1;
--        c.rxbitbuf:=c.rxbitbuf MOD (i<<c.rxbits);
        c.rxbitbuf:=CAST(CARD32, CAST(SET32, c.rxbitbuf) * SHIFT(SET32{0..31}
                , VAL(INTEGER, c.rxbits)-32));

        n:=dist DIV 2-1;                       (* bit count extra bits *)
--        dist:=DISTAB[dist] + c.rxbitbuf>>(VAL(CARDINAL,c.rxbits)-n);
        dist:=DISTAB[dist] + CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf),
                VAL(INTEGER, n)-VAL(INTEGER, c.rxbits)));

        DEC(c.rxbits, n);
      ELSE INC(dist)  END;
<* IF DEB THEN *>
WrInt(dist,3);WrStr("=DIST ");WrStrLn("");
<* END *>

      IF c.lencode<3 THEN outlen:=-2 END;      (* decode error *)
      n:=c.wp;
      i:=(n+BSIZE-dist) MOD BSIZE;             (* start in ring *)
      WHILE c.lencode>0 DO
--WrStr("<<");WrHex(ORD(c.ring[i]),1);WrStr(">>");
        xsend(c, c.ring[i], dbuf, outlen);            (* copy string *)
        i:=(i+1) MOD BSIZE;
        IF i=n THEN i:=(n+BSIZE-dist) MOD BSIZE END; (* looping string *)
        DEC(c.lencode);
--WrInt(c.lencode, 15);
      END;
--WrStr("=LOOPEND= "); WrStrLn("");
      c.compmod:=1C;
    ELSIF NOT c.justcomp & ODD(CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf),
                1-VAL(INTEGER, c.rxbits)))) THEN   (* switch to raw *)
<* IF DEB THEN *>
WrStrLn("");WrStrLn("TORAW");
<* END *>
      DEC(c.rxbits);

--      i:=1;
--      c.rxbitbuf:=c.rxbitbuf MOD (i<<c.rxbits);
      c.rxbitbuf:=CAST(CARD32, CAST(SET32, c.rxbitbuf) * SHIFT(SET32{0..31}
                , VAL(INTEGER, c.rxbits)-32));

      n:=8+VAL(CARDINAL, c.rxbits) MOD 8;
--      c.rawlen:=c.rxbitbuf>>(VAL(CARDINAL,c.rxbits)-n)+1;
      c.rawlen:=CAST(CARD32, SHIFT(CAST(SET32, c.rxbitbuf), VAL(INTEGER,
                n)-VAL(INTEGER, c.rxbits)))+1;
<* IF DEB THEN *>
WrHex(c.rawlen,9);WrStr("=RLEH ");
<* END *>
      DEC(c.rxbits, n);
      c.compmod:=0C;

    ELSE                                       (* switch to compressed *)
      IF NOT c.justcomp OR (c.compmod<>3C) THEN
<* IF DEB THEN *>
WrStrLn("");WrStrLn("DEC1");
<* END *>
        DEC(c.rxbits);
      END;
      c.justcomp:=TRUE;
<* IF DEB THEN *>
WrStrLn("");WrStrLn("TOCOMP");
<* END *>
      IF c.compmod=3C THEN                     (* end of compressed and got to compressed is EOF *)
<* IF DEB THEN *>
WrStrLn("");WrStrLn("DATAEND");
<* END *>
        flush:=FALSE;
<* IF DEB THEN *>
th(c, 0C);
<* END *>
        c.rxbits:=((c.rxbits+1) DIV 8)*8;
<* IF DEB THEN *>
th(c, 0C);
<* END *>
        done:=TRUE;  
WrStrLn(" LIBDONE1 ");
        c.compmod:=0C;
      ELSE c.compmod:=1C END; 

    END; 
  END;
END Expandbyte;
*/
static uint32_t deflate_DISTAB[32] = {1UL,2UL,3UL,4UL,5UL,7UL,9UL,13UL,
                17UL,25UL,33UL,49UL,65UL,97UL,129UL,193UL,257UL,385UL,513UL,
                769UL,1025UL,1537UL,2049UL,3073UL,4097UL,6145UL,8193UL,
                12289UL,16385UL,24577UL,0UL,0UL};

static uint32_t deflate_LENTAB[32] = {0UL,3UL,4UL,5UL,6UL,7UL,8UL,9UL,10UL,
                11UL,13UL,15UL,17UL,19UL,23UL,27UL,31UL,35UL,43UL,51UL,59UL,
                67UL,83UL,99UL,115UL,131UL,163UL,195UL,227UL,285UL,0UL,0UL};


static void copystr(char dbuf[], uint32_t dbuf_len, int32_t * outlen,
                 struct deflate_XCONTEXT * c)
{
   uint32_t n;
   uint32_t i;
   if (c->lencode<3UL) *outlen = -2L;
   n = c->wp;
   i = (n+32768UL)-c->distcode&32767UL; /* start in ring */
   while (c->lencode>0UL) {
      xsend(c, c->ring[i], dbuf, dbuf_len, outlen); /* copy string */
      i = i+1UL&32767UL;
      if (i==n) i = (n+32768UL)-c->distcode&32767UL;
      --c->lencode;
   }
} /* end copystr() */

static uint32_t _cnst0[32] = {1UL,2UL,3UL,4UL,5UL,7UL,9UL,13UL,17UL,25UL,
                33UL,49UL,65UL,97UL,129UL,193UL,257UL,385UL,513UL,769UL,
                1025UL,1537UL,2049UL,3073UL,4097UL,6145UL,8193UL,12289UL,
                16385UL,24577UL,0UL,0UL};
static uint32_t _cnst[32] = {0UL,3UL,4UL,5UL,6UL,7UL,8UL,9UL,10UL,11UL,
                13UL,15UL,17UL,19UL,23UL,27UL,31UL,35UL,43UL,51UL,59UL,67UL,
                83UL,99UL,115UL,131UL,163UL,195UL,227UL,285UL,0UL,0UL};

extern void deflate_Expandbyte(struct deflate_XCONTEXT * c, char ch,
                char dbuf[], uint32_t dbuf_len, int32_t * outlen,
                char * done)
{
   uint32_t w;
   uint32_t n;
   c->rxbitbuf = c->rxbitbuf*256UL+(uint32_t)(uint8_t)ch;
                /* append new bits */
   c->rxbits += 8UL;
   *done = 0;
   for (;;) {
      c->rxbitbuf = (uint32_t)((uint32_t)
                c->rxbitbuf&X2C_LSH(0xFFFFFFFFUL,32,
                (int32_t)c->rxbits-32L)); /* strip old bits */
      if (c->rawlen==0L) n = 1UL;
      else if (c->rawlen==-1L) {
         /* get raw len */
         n = 8UL+(c->rxbits&7UL);
      }
      else if (c->rawlen==-2L) n = 7UL;
      else if (c->rawlen==-3L) n = 8UL;
      else if (c->rawlen==-4L) n = 9UL;
      else if (c->rawlen==-5L) {
         /* get extra len bits */
         n = (c->lencode-5UL)/4UL; /* bit count */
      }
      else if (c->rawlen==-6L) n = 5UL;
      else if (c->rawlen==-7L) {
         /* get extra dist code */
         n = c->distcode/2UL-1UL; /* bit count extra bits */
      }
      else n = 8UL;
      if (c->rxbits<n) break;
      w = (uint32_t)X2C_LSH((uint32_t)c->rxbitbuf,32,
                (int32_t)n-(int32_t)c->rxbits);
      if (c->rawlen>0L) {
         /* in uncompressed mode */
         xsend(c, (char)w, dbuf, dbuf_len, outlen);
         --c->rawlen;
         c->rxbits -= 8UL;
      }
      else if (c->rawlen==0L) {
         /* comp or raw mode bit */
         --c->rxbits;
         if (w==1UL) c->rawlen = -1L;
         else {
            if (c->compdata) {
               /* was in compressed go to compressed is EOF */
               *done = 1;
               c->compdata = 0;
               c->rawlen = 0L;
               c->rxbits = 0UL;
               break;
            }
            c->rawlen = -2L; /* start compressed */
            c->compdata = 1;
         }
      }
      else if (c->rawlen==-1L) {
         /* get raw len */
         c->rawlen = (int32_t)(w+1UL);
         c->rxbits -= n;
         c->compdata = 0;
      }
      else if (c->rawlen==-2L) {
         /* get 7 bit len code */
         if (w<24UL) {
            c->rxbits -= 7UL;
            c->lencode = w;
            if (w==0UL) {
               /* end comp */
               c->rawlen = 0L;
               if (c->compdata) {
                  /* to comp followed bei endcomp is EOF */
                  c->compdata = 0;
                  *done = 1;
                  c->rxbits = 0UL;
                  break;
               }
               c->compdata = 1;
            }
            else if (w>=9UL && w<29UL) c->rawlen = -5L;
            else {
               /* next extra len */
               c->lencode = _cnst[c->lencode];
               c->rawlen = -6L; /* next get 5 bit dist code */
            }
         }
         else c->rawlen = -3L;
      }
      else if (c->rawlen==-3L) {
         /* get 8 bit len code */
         c->compdata = 0;
         if (w<192UL) {
            /* literal < 144 */
            xsend(c, (char)(w-48UL), dbuf, dbuf_len, outlen);
            c->rxbits -= 8UL;
            c->rawlen = -2L; /* continue comp mode */
         }
         else if (w<200UL) {
            /* len codes 24..29 */
            c->lencode = w-168UL;
            c->rxbits -= 8UL;
            c->rawlen = -5L; /* next get extra len */
         }
         else c->rawlen = -4L;
      }
      else if (c->rawlen==-4L) {
         /* get 9 bit len code */
         c->compdata = 0;
         xsend(c, (char)(w-256UL), dbuf, dbuf_len, outlen);
         c->rxbits -= 9UL;
         c->rawlen = -2L; /* continue comp mode */
      }
      else if (c->rawlen==-5L) {
         /* get extra len bits */
         c->lencode = _cnst[c->lencode]+w;
         c->rxbits -= n;
         c->rawlen = -6L; /* next dist code */
      }
      else if (c->rawlen==-6L) {
         /* get 5 bit dist code */
         c->compdata = 0;
         c->rxbits -= 5UL;
         if (w>=4UL) {
            c->distcode = w;
            c->rawlen = -7L; /* next extra dist code */
         }
         else {
            c->distcode = _cnst0[w];
            c->rawlen = -2L; /* continue comp mode */
            copystr(dbuf, dbuf_len, outlen, c);
         }
      }
      else if (c->rawlen==-7L) {
         /* get extra dist code */
         c->rxbits -= c->distcode/2UL-1UL;
         c->distcode = _cnst0[c->distcode]+w;
         c->rawlen = -2L; /* continue comp mode */
         copystr(dbuf, dbuf_len, outlen, c);
      }
   }
} /* end Expandbyte() */


extern void deflate_BEGIN(void)
{
   static int deflate_init = 0;
   if (deflate_init) return;
   deflate_init = 1;
   if (sizeof(uint8_t)!=1) X2C_ASSERT(0);
   if (sizeof(uint32_t)!=4) X2C_ASSERT(0);
   osi_BEGIN();
}

