#!/bin/sh

# file created by aprsmap of a list of tiles to fetch
ORIGFN="gettiles"

# filename of default list of maps
MAPFN="maplist"

# map directory path specified by aprsmap
DIR=$2

# move the original file to a temporary file to prevent a race condition where
# aprsmap seems to attempt to continue writing to the original file before
# this script completes processing
FN=$(mktemp)
#mv $ORIGFN $FN
cp $ORIGFN $FN

# use user-specified maplist if passed via command line, else use default
MAPLIST=$3
if [ -z $3 ] ; then
    MAPLIST=$MAPFN
fi

# 5 sec timeout
WGETCMDBASE="wget -q -T 5 "

#------------------------------------------------------------------------------

# if the tiles directory does not exist then create one
if ! [ -d $DIR ] ; then
    mkdir $DIR
fi

# if tiles directory exists and the tile list is accessible
if [ -d $DIR ] ; then
    if [ -r $FN ] ; then
# create the rest of the map tile directory structure if necessary
        while read mapname mapz mapx mapy ; do
            if ! [ -d $DIR/$mapname ] ; then
                mkdir $DIR/$mapname
            fi
            if ! [ -d $DIR/$mapname/$mapz ] ; then
                mkdir $DIR/$mapname/$mapz
            fi
            if ! [ -d $DIR/$mapname/$mapz/$mapx ] ; then
                mkdir $DIR/$mapname/$mapz/$mapx
            fi

# try to match the requested map tile with a server in the map list file
	    if [ -r $MAPLIST ] ; then
		while read tileid tileformat tileurl tileapikey tilecomment ; do
		    if [ $tileid = $mapname ] ; then
			SERVER=$tileurl
			EXTENT=$tileformat
			break;
		    fi
		done < $MAPLIST
		if [ -z $SERVER ] ; then
		    echo "Unable to find map named \"$mapname\" in map file: $MAPLIST"
		    exit 1;
		fi
	    else
		echo "Unable to access map file: $MAPLIST"
		exit 1
	    fi

# if an api key or access token is present
	    if [ $(echo $tileapikey | cut -c1-8) = "?apikey=" ] ||
	       [ $(echo $tileapikey | cut -c1-14) = "?access_token=" ] ; then
		APIKEY=$tileapikey
	    fi

#------------------------------------------------------------------------------

# so far so good, then initiate the retrieval of all tiles
            MAPFILE=$mapz/$mapx/$mapy.$EXTENT
            echo "Loading tile: $MAPFILE ..."
            echo -n "-> from $SERVER - "
            $WGETCMDBASE -O $DIR/$mapname/$MAPFILE $SERVER/$MAPFILE$APIKEY &
            echo "initiated download."
        done < $FN
        rm -f $FN
    else
	echo "Unable to access tiles file: $FN"
	exit 1
    fi
else
    echo "Directory $DIR not found."
    exit 1
fi

exit 0
