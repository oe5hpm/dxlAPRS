#!/bin/sh
EXCL="X.h Xlib.h Xutil.h cursorfont.h keysym.h jconfig.h jmorecfg.h jpeglib.h png.h"

./getNEWtxt.sh
cd src
wget http://oe5dxl.ampr.at/aprs/intel-debug.tgz || exit 1
tar -xzf intel-debug.tgz
mv intel-debug.tgz ../
rm $EXCL
rm -r -f lib_x86
rm -r -f lib_x86_64
git checkout -- pngread.c
git checkout -- osic.c
cd ..
./license-add.sh
git status
