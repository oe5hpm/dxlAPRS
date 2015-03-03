#!/bin/sh

PLATTFORMS="x86 armv6 armv7hf"

for i in $PLATTFORMS; do
	cd src
	make clean
	make $i
	cd ..
	./packCurrent.sh $i
done


