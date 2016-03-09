#!/bin/sh
EXCL="FileSys.h IOChan.h InOut.h ProgEnv.h RealIO.h RealMath.h StdChans.h Storage.h SysClock.h TimeConv.h xPOSIX.h xmRTS.h xrcArythmetics.h"

./getNEWtxt.sh
cd src
wget http://oe5dxl.ampr.at/aprs/intel-debug.tgz || exit 1
tar -xzf intel-debug.tgz
rm intel-debug.tgz
rm $EXCL
git checkout -- keysym.h
git checkout -- pngread.c
cd ..
./license-add.sh
git status
