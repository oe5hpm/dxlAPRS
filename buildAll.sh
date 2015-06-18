#!/bin/sh

PLATTFORMS="x86 armv6 armv7hf"

for i in $PLATTFORMS; do
	cd src
	make clean

	case $i in
	"armv6")
		echo "---------------armv6------------- ($i) "
		CROSS_COMPILE=/opt/rpi-tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi- make $i
	;;
	"armv7hf")
		CROSS_COMPILE=/opt/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- make $i
	;;
	"x86")
		echo "----------------x86---------------"
		make $i
	;;
	esac
	cd ..
	./packCurrent.sh $i
done
