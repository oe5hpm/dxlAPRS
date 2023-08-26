import wx

NODATE=0            # send no date/time if system time not correct
DHM=1               # send day in month, hour, minute
HMS=2               # send hour, minute, second
MYCALL="OE0AAA-1"
DESTCALL="APZWXP-1" # ssid is used for efficient digipeating
VIA=""              # for junk like "WIDE1-1"
LAT=78.2500         # decimal degrees
LONG=13.0020
COMMENT=":temperhum test"
SYMBOL="/_"
UDP=("127.0.0.1",7001)  # axudp destination on same or other computer

# insert "import sendwx" and any of the functions where wx values are ready in sensor readout programm
# try manual sending with "sendwx.wxudp(20,50)" "sendwx.wxfile(20,50)" "sendwx.wxbeacon(20,50)" 

# in temper-python-master/temperusb/cli.py maybe it works around "print(output)"
# sendwx.wxfile(reading[sensor]['temperature_c'], reading[sensor]['humidity_pc'])


# if you want to send other beacons or run an aprs digi, make a beacon.txt file like:
# OE0AAA-1>APZWXP-2:@\\hh7815.00N/01300.12E_\\[/tmp/wxline.txt]:temperhum test
# OE0AAA-1>APNL01-1:!7815.00N/01300.12E/other beacon
# and send it with
# ./udpbox -R 0.0.0.0:0 -b 600:beacon.txt -r 127.0.0.1:9032 -v
# file: .../...g...t073h77
# tmp wx file will be deleted after sending so no wrong beacons go out until next sensor readout
# file may be written any time, beacon time is made by udpbox
def wxfile(temp, hum):
  wx.wrfile("/tmp/wxline.txt","/tmp/wxtmp",wx.wxaprs({"celsius":temp,"hum":hum},True))

def wxline(temp, hum):
  return wx.beacontext(MYCALL,DESTCALL,VIA,DHM,SYMBOL,LAT,LONG,{"celsius":temp,"hum":hum,},COMMENT,True)
#more wx values may be appended, labels see in wx.py


# writes a beacon text line in monitor format as used in aprsis
# OE0AAA-1>APZWXP-1:@024418h7815.00N/01300.12E_.../...g...t073h77:temperhum test
# send it with ./udpbox -R 0.0.0.0:0 -b 600:/tmp/wxline.txt -r 127.0.0.1:9032 -v 
def wxbeacon(temp, hum):
  wx.wrfile("/tmp/wxline.txt","/tmp/wxtmp",wxline(temp, hum))



# send as axudp
# to afskmodem: ./afskmodem -f 22050 -M 0 -U 0.0.0.0:0:9032
# to direwolf:  ./udpflex -U :0:9032 -T 127.0.0.1:8001 -V
# to tnc:       ./udpflex -U :0:9032 -t /dev/ttyS0:9600 -k -u -V
# to udpgate4 or aprsmap rf port
# timing has to be done by calling this function (every 10 to 30min or so)
def wxudp(sock, temp, hum):
  try:
    wx.sendax(sock, wxline(temp, hum), UDP, {})
  except:
    print("wx udp send error")
