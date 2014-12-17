#!/bin/sh
git branch -D fromDXL
git checkout master
git checkout -b fromDXL

./getNEWtxt.sh
cd src
wget http://oe5dxl.ampr.at/aprs/intel-debug.tgz || exit 1
tar -xzf intel-debug.tgz
rm intel-debug.tgz
git checkout -- keysym.h
git checkout -- pngread.c
cd ..
git status
