#!/bin/sh

rm aprsmap_common/*.txt
rm dxlARPS_common/*.txt

wget http://oe5dxl.ampr.at/aprs/NEW.txt -O dxlAPRS_common/NEW.txt
wget http://oe5dxl.ampr.at/wettersonde/linux/sonde2aprs.txt -O dxlAPRS_common/sonde2aprs.txt
wget http://oe5dxl.ampr.at/aprs/osm-mapper/bin/NEW.txt -O aprsmap_common/NEW.txt
wget http://oe5dxl.ampr.at/aprs/udpbox.txt -O dxlAPRS_common/udpbox.txt
wget http://oe5dxl.ampr.at/aprs/osm-mapper/bin/hints.txt -O aprsmap_common/hints.txt
wget http://oe5dxl.ampr.at/aprs/osm-mapper/bin/help.txt -O aprsmap_common/help.txt
wget http://oe5dxl.ampr.at/aprs/osm-mapper/bin/osm/poi.txt -O aprsmap_common/poi.txt

