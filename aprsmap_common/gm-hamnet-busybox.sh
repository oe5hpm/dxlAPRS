#!/bin/sh

FN="gettiles"
DIR="osm"

# 2 sec timeout, 2 retrys
WGETCMDBASE="wget -q -T 3 -t 1 "


OSMSERVERS="\
             http://osm.oe2xzr.ampr.at/osm/tiles \
             http://c.tile.openstreetmap.org \
             http://b.tile.openstreetmap.org \
             http://a.tile.openstreetmap.org \
             http://a.www.toolserver.org/tiles/osm \
             http://b.www.toolserver.org/tiles/osm \
            "


OSMSERVERSTOPO="\
             http://osm.oe2xzr.ampr.at/osm/tiles_topo \
             http://a.tile.opencyclemap.org/cycle \
             http://b.tile.opencyclemap.org/cycle \
            "



OSMSERVERSSAT="\
             http://osm.oe2xzr.ampr.at/osm/tiles_sat \
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



OSMSERVERSQUEST="\
             http://otile1.mqcdn.com/tiles/1.0.0/osm \
             http://otile2.mqcdn.com/tiles/1.0.0/osm \
            "                                         


#http://osm.oe2xzr.ampr.at/osm/tiles_topo

#--------------------------------------------------------------------------------------------------
if [ -d $DIR ] ; then
  # search zero-size files and destroy them
  #echo -n "cleanup zero-files ... "  
  #find $DIR/tiles -size 0 -delete  
  #echo "done."
#  while true; do
  for i in 1 2 3 4 5 6 7 8 9 10; do



	if [ -r $FN ] ; then
            sleep 1
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

elif [ tiles_sat = $mapa ]; then
  SERVERS=$OSMSERVERSSAT
  EXTENT="jpg"

elif [ tiles_cyclemap = $mapa ]; then
  SERVERS=$OSMSERVERSCYCLE                                 
  EXTENT="png"

elif [ tiles_transport = $mapa ]; then
  SERVERS=$OSMSERVERSTRANSPORT
  EXTENT="png"

elif [ tiles_quest = $mapa ]; then
  SERVERS=$OSMSERVERSQUEST
  EXTENT="jpg"

fi



			MAPFILE=$DIR/$mapa/$mapb/$mapc/$mapd.$EXTENT
			echo "loading tile: $MAPFILE ..."
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
#					rm $MAPFILE
#					echo -n "zero file, using alternative url - "
#					$WGETCMDBASE -O $MAPFILE "$OSMSERVER/$mapa/$mapb/$mapc/$mapd.png"
#					if [ -s $MAPFILE ] ; then
#						echo "done."
#						break
#					else
						rm $MAPFILE
						echo "zero file, trying next mirror."
#					fi
				fi
			done
		done < $FN
		rm -f $FN
    fi
    sleep 2
  done
else
  echo directory $DIR not found
fi

exit 0
