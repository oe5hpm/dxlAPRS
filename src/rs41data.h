/*
 * rs41data.h
 *
 *  Created on: 15-01-2017
 *      Author: sacintom
 */

#ifndef RS41DATA_H_
#define RS41DATA_H_

/*SQ7BR*/

typedef char OBJNAME[9];

enum BurstKill {bk_unknown,bk_on, bk_off };

struct CONTEXTR41;

typedef struct CONTEXTR41 * pCONTEXTR41;

struct CONTEXTR41 {
   pCONTEXTR41 next;
   OBJNAME name;
   char posok;
   char framesent;
   float mhz0;
   unsigned long gpssecond;
   unsigned long framenum;
   unsigned long tused;
   double hp;
   unsigned long ozonInstType;
   unsigned long ozonInstNum;
   double ozonTemp;
   double ozonuA;
   double ozonBatVolt;
   double ozonPumpMA;
   double ozonExtVolt;
   /* SQ7BR */

   enum BurstKill burstKill;
   long fileHex;
   unsigned long killTimer;
   unsigned long swVersion;
   unsigned long hwvwersion;
   char serialNumber[6];
   char sondeType[8];
   char hwType[11];
   char hwId[5];
   unsigned long frameBurst;
   long afterBurstTimerLeft;

};




#endif /* RS41DATA_H_ */
