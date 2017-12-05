#!/bin/sh

# file created by aprsmap of a list of tiles to fetch
ORIGFN="gettiles"

# map directory path specified by aprsmap
#DIR="osm"
DIR=$2

# move the original file to a temporary file to prevent a race
# condition with aprsmap where aprsmap sometimes attempts to write
# to the original file before this script completes processing.
FN=$(mktemp)
cp $ORIGFN $FN

# 3 sec timeout, 1 retry
WGETCMDBASE="wget -q -T 3 -t 1 "

OSMSERVERS="\
             http://osm.oe2xzr.ampr.at/osm/tiles \
             http://a.tile.openstreetmap.org \
             http://b.tile.openstreetmap.org \
             http://c.tile.openstreetmap.org \

            "

OSMSERVERSTOPO="\
             http://osm.oe2xzr.ampr.at/osm/tiles_topo \
             http://a.tile.thunderforest.com/landscape \
             http://b.tile.thunderforest.com/landscape \
             http://c.tile.thunderforest.com/landscape \
            "

OSMSERVERSCYCLE="\
             http://osm.oe2xzr.ampr.at/osm/tiles_cyclemap \
             http://a.tile.opencyclemap.org/cycle \
             http://b.tile.opencyclemap.org/cycle \
            "

OSMSERVERSTRANSPORT="\
             http://a.tile.opencyclemap.org/transport \
             http://b.tile.opencyclemap.org/transport \
            "                                         

# note: mapquest sat and quest tile servers are no longer providing direct access
OSMSERVERSSAT="\
             http://osm.oe2xzr.ampr.at/osm/tiles_sat \
             http://otile1.mqcdn.com/tiles/1.0.0/sat \
             http://otile2.mqcdn.com/tiles/1.0.0/sat \
             http://otile3.mqcdn.com/tiles/1.0.0/sat \
             http://otile4.mqcdn.com/tiles/1.0.0/sat \
            "

OSMSERVERSQUEST="\
             http://otile1.mqcdn.com/tiles/1.0.0/osm \
             http://otile2.mqcdn.com/tiles/1.0.0/osm \
             http://otile3.mqcdn.com/tiles/1.0.0/osm \
             http://otile4.mqcdn.com/tiles/1.0.0/osm \
            "                                         

#--------------------------------------------------------------------------------------------------

# if the tiles directory does not exist then create one
if ! [ -d $DIR ]; then
    mkdir $DIR
fi

# if tiles directory found
if [ -d $DIR ] ; then
    for i in 1 2 3 4 5 6 7 8 9 10; do

	if [ -r $FN ] ; then
	    while read  mapa mapb mapc mapd ; do
        	if ! [ -d $DIR/$mapa ] ; then
		    mkdir $DIR/$mapa
        	fi
        	if ! [ -d $DIR/$mapa/$mapb ] ; then
		    mkdir $DIR/$mapa/$mapb
        	fi
        	if ! [ -d $DIR/$mapa/$mapb/$mapc ] ; then
		    mkdir $DIR/$mapa/$mapb/$mapc
        	fi
		
		if [ tiles = $mapa ]; then
		    SERVERS=$OSMSERVERS
		    EXTENT="png"
		    
		elif [ tiles_topo = $mapa ]; then
		    SERVERS=$OSMSERVERSTOPO
		    EXTENT="png"
		    
		elif [ tiles_cyclemap = $mapa ]; then
		    SERVERS=$OSMSERVERSCYCLE                                 
		    EXTENT="png"
		    
		elif [ tiles_transport = $mapa ]; then
		    SERVERS=$OSMSERVERSTRANSPORT
		    EXTENT="png"

# note: mapquest sat and quest tile servers are no longer providing direct access
		elif [ tiles_sat = $mapa ]; then
		    SERVERS=$OSMSERVERSSAT
		    EXTENT="jpg"
		    
		elif [ tiles_quest = $mapa ]; then
		    SERVERS=$OSMSERVERSQUEST
		    EXTENT="png"
		fi
		
		MAPFILE=$DIR/$mapa/$mapb/$mapc/$mapd.$EXTENT
		echo "Loading tile: $MAPFILE ..."
		for OSMSERVER in $SERVERS ; do
		    echo -n "-> from $OSMSERVER - "
		    $WGETCMDBASE -O $MAPFILE "$OSMSERVER/$mapb/$mapc/$mapd.png"
		    if [ -s $MAPFILE ] ; then
			echo "done."
			if [ jpg = $EXTENT ]; then
			    jpegtopnm $MAPFILE | pnmtopng > $DIR/$mapa/$mapb/$mapc/$mapd.png
			    rm -f $MAPFILE
			fi
			break
		    else
		    	rm -f $MAPFILE
			echo "Unsuccessful getting tile, trying next mirror."
		    fi
		done
	    done < $FN
	    rm -f $FN
	fi
    done
else
    echo Directory $DIR not found.
    rm -f $ORIGFN
    exit 1
fi

rm -f $ORIGFN

exit 0
