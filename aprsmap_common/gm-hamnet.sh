#!/bin/sh
# assumes script is called via aprsmap in the aprsmap directory
# this script calls gm.sh with hamnet-specific map tile urls
#
# this script is selected by using the aprsmap.cfg entry:
# Get Map Program|1|gm-hamnet.sh

./gm.sh $1 $2 maplist-hamnet
