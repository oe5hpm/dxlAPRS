#!/bin/sh
EXCL="X.h Xlib.h Xutil.h cursorfont.h keysym.h jconfig.h jmorecfg.h jpeglib.h png.h"
IGNL="pngread.c osic.c rsc.h pastewrapper.c pastewrapper.h ../aprsmap_common/kiss-th-d72.txt ../aprsmap_common/kiss9k6-th-d72.txt ../aprsmap_common/kisson.txt"
./getNEWtxt.sh
cd src
wget http://oe5dxl.ampr.at/aprs/intel-debug.tgz || exit 1
tar -xzf intel-debug.tgz
mv intel-debug.tgz ../
rm $EXCL
rm -r -f lib_x86
rm -r -f lib_x86_64
for i in $IGNL; do
	echo ignoring changes of $i ...
	git checkout -- $i
done

cd ..
./license-add.sh
git status
