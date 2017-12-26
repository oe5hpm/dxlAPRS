#!/bin/sh

# file created by aprsmap of a list of tiles to fetch
ORIGFN="gettiles"

# filename of default list of maps
MAPFN="maplist"

# 5 sec timeout, 1 retry
WGETCMDBASE="wget -q -T 5 -t 1 "

#------------------------------------------------------------------------------

# map directory path specified by aprsmap
DIR=$2

# use user-specified maplist if passed via command line, else use default
if [ -z $3 ] ; then
    MAPLIST=$MAPFN
else
    MAPLIST=$3
fi

#------------------------------------------------------------------------------

# move the original file to a temporary file to prevent a race condition where
# aprsmap seems to attempt to continue writing to the original file before
# this script completes processing
FN=$(mktemp)
#mv $ORIGFN $FN
cp $ORIGFN $FN

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

# try to match the requested map name with a server in the map list file
            if [ -r $MAPLIST ] ; then
                while read tileid tileorder tileformat tileurl tileapikey tilecomment ; do
                    if [ "$tileid" = "$mapname" ] ; then
                        ORDER=$tileorder
                        EXTENT=$tileformat
                        SERVER=$tileurl
                        break;
                    fi
                done < $MAPLIST
                if [ -z $SERVER ] ; then
                    echo "Unable to find map named \"$mapname\" in map file: $MAPLIST"
                    rm -f $FN
                    exit 1;
                fi
            else
                echo "Unable to access map file: $MAPLIST"
                rm -f $FN
                exit 1
            fi

# if an api key or access token is present
            if ! [ -z $tileapikey ] ; then
                if [ $(echo $tileapikey | cut -c1-8) = "?apikey=" ] ||
                   [ $(echo $tileapikey | cut -c1-14) = "?access_token=" ] ; then
                    APIKEY=$tileapikey
                fi
            fi

# tile server coordinate system ordering may be different from aprsmap
            MAPFILE=$mapz/$mapx/$mapy.$EXTENT
            if [ "zxy" = $tileorder ] ; then
                GETTILE=$MAPFILE
            elif [ "zyx" = $tileorder ] ; then
                GETTILE=$mapz/$mapy/$mapx.$EXTENT
            else
                echo "Unexpected tile server coordinate ordering"
                rm -f $FN
                exit 1
            fi

#------------------------------------------------------------------------------

# so far so good, initiate retrieval of the tile
            echo "Loading tile: $MAPFILE ..."
            echo -n "-> from $SERVER - "
            $WGETCMDBASE -O $DIR/$mapname/$MAPFILE $SERVER/$GETTILE$APIKEY &
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
