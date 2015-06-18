#!/bin/sh

PLATTFORMS="x86 armv6 armv7hf"

for i in $PLATTFORMS; do
	cd src
	make clean

	if [ "$i"=="armv6" ]; then
		CROSS_COMPILE=/opt/rpi-tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi- make $i
	elif [ "$i"=="armv7hf" ]; then
		CROSS_COMPILE=/opt/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- make $i
	else
		make $i
	fi
	cd ..
	./packCurrent.sh $i
done


