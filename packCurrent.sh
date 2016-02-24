#!/bin/sh

if [ -z $1 ]; then
	echo "give arch as argument! example: $0 armv6"
	exit 1
fi

VER=`git describe --always --dirty`

BASE=src/
EXCL='arm-linux-armv7hf-gcc arm-linux-armv7hf-strip arm-linux-rpi-gcc arm-linux-rpi-strip'
FLIST=`ls $BASE`
PACKLIST=""

for i in $FLIST; do
	if [ -x $BASE/$i ] && [ ! -d $BASE/$i ] && [ `echo $EXCL | grep -c $i` -eq 0 ]; then
		PACKLIST=`echo $PACKLIST $i`
	fi
done

PACKLIST=`echo $PACKLIST ../aprsmap_common/* ../dxlAPRS_common/* ../scripts/*`

echo $PACKLIST

cd $BASE
tar -czf ../dxlAPRS_$1-$VER.tgz $PACKLIST
