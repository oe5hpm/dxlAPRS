import wx, json, time, os, socket

WXFILE="jwx";
NODATE=0            # send no date/time if system time not correct
DHM=1               # send day in month, hour, minute
HMS=2               # send hour, minute, second
MYCALL="NOCALL-1"
DESTCALL="APZWXP-1" # ssid is used for efficient digipeating
VIA=""              # for junk like "WIDE1-1"
LAT=78.2500         # decimal degrees
LONG=13.0020
COMMENT="wx test"
SYMBOL="/_"
UDP=("127.0.0.1",4020)  # axudp destination on same or other computer
COMPRESSED=False

sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
while True:
  try: f=open(WXFILE,'r')
  except: print("file not readable")
  else: 
    try:wxj = json.load(f)
    except:print("wx line read error")
    else:
      wxj["pressure"]=wxj["pressure"]*0.01
      wxline = wx.beacontext(MYCALL,DESTCALL,VIA,NODATE,SYMBOL,LAT,LONG,wxj,COMMENT,COMPRESSED)
      print(wxline)
      try: wx.sendax(sock, wxline, UDP, {})
      except: print("wx udp send error")
    f.close()
    os.remove(WXFILE)
  time.sleep(10)
