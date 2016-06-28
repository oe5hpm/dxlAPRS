#!/bin/sh
EXCL="X.h Xlib.h Xutil.h cursorfont.h keysym.h"

./getNEWtxt.sh
cd src
wget http://oe5dxl.ampr.at/aprs/intel-debug.tgz || exit 1
tar -xzf intel-debug.tgz
rm intel-debug.tgz
rm $EXCL
git checkout -- pngread.c
cd ..
./license-add.sh
git checkout -- src/xosi.c
git status
