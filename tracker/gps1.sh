#!/bin/bash
stty 57600 -echo raw < /dev/ttyS2
# echo -ne "\044#Pe20,110,158,138,106,136,176,152,115,3,240,96,0,6,8,62,47,0,67,104,114,105,115,0,25" > /tmp/rohr
usleep 100000
# echo -ne "\044#Pe20,110,158,138,106,136,176,152,115,3,240,96,0,6,8,62,47,0,67,104,114,105,115,0,25" > /tmp/rohr
usleep 500000
while true; do
  cat g.txt > /dev/ttyS2
  usleep 500000
done;
