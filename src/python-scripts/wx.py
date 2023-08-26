from datetime import datetime
from array import array
from math import log
from pathlib import Path
import socket

RAININCH=100.0/25.4
WINDKNOTS=1.0/1.609
R1=91
R2=R1*R1
R3=R2*R1
COMPTYP=34

CRCTAB = array("H",[
61560,57841,54122,49891,46684,42965,38222,33991,31792,28089,24354,20139,14868,11165,6406,2191,
57593,61808,50155,53858,42717,46932,34255,37958,27825,32056,20387,24106,10901,15132,2439,6158,
53626,49395,62056,58337,38750,34519,46156,42437,23858,19643,32288,28585,6934,2719,14340,10637,
49659,53362,58089,62304,34783,38486,42189,46404,19891,23610,28321,32552,2967,6686,10373,14604,
45692,41973,37230,32999,62552,58833,55114,50883,15924,12221,7462,3247,30736,27033,23298,19083,
41725,45940,33263,36966,58585,62800,51147,54850,11957,16188,3495,7214,26769,31000,19331,23050,
37758,33527,45164,41445,54618,50387,63048,59329,7990,3775,15396,11693,22802,18587,31232,27529,
33791,37494,41197,45412,50651,54354,59081,63296,4023,7742,11429,15660,18835,22554,27265,31496,
29808,26105,22370,18155,12884,9181,4422,207,63544,59825,56106,51875,48668,44949,40206,35975,
25841,30072,18403,22122,8917,13148,455,4174,59577,63792,52139,55842,44701,48916,36239,39942,
21874,17659,30304,26601,4950,735,12356,8653,55610,51379,64040,60321,40734,36503,48140,44421,
17907,21626,26337,30568,983,4702,8389,12620,51643,55346,60073,64288,36767,40470,44173,48388,
13940,10237,5478,1263,28752,25049,21314,17099,47676,43957,39214,34983,64536,60817,57098,52867,
9973,14204,1511,5230,24785,29016,17347,21066,43709,47924,35247,38950,60569,64784,53131,56834,
6006,1791,13412,9709,20818,16603,29248,25545,39742,35511,47148,43429,56602,52371,65032,61313,
2039,5758,9445,13676,16851,20570,25281,29512,35775,39478,43181,47396,52635,56338,61065,65280
])

def udpcrc(frame, topos):
  c=0
  for p in range(topos): c = (c >> 8) ^ CRCTAB[(ord(frame[p]) ^ c) & 0xff]
  return c


#-----------------build aprs wx string 

def wxaprs(wx, compressed):
  try:    x=round(wx["winddir"])
  except:
    if compressed: ws=" "
    else: ws="..."
  else:
    if (x<=0) | (x>360): x=360
    if compressed: ws=chr(33+(x+2)//4)
    else: ws=str(x).zfill(3)

  if not compressed:
    ws+="/"
  try:    x=round(wx["wind"]*WINDKNOTS)
  except:
    if compressed: ws+=" " + chr(COMPTYP) 
    else: ws+="..."
  else:
    if x>999: x=999
    elif x<0: x=0
    if compressed:
      ws+=chr(33+round(log(x+1.0)/log(1.08))) + chr(COMPTYP)
    else: ws+=str(x).zfill(3)

  ws+="g"
  try:    x=round(wx["gust"]*WINDKNOTS)
  except: ws+="..."
  else:
    if x>999: x=999
    ws+=str(x).zfill(3)

  ws+="t"
  try:    x=round(wx["celsius"]*1.8+32.0)
  except: ws+="..."
  else:
    if x>999: x=999
    elif x<-99: y=-99
    ws+=str(x).zfill(3)

  try:    x=round(wx["hum"])
  except: pass
  else:
    if x>=100: x=0
    elif x<1: x=1
    ws+="h"+str(x).zfill(2)

  try:    x=round(wx["rain1h"]*RAININCH)
  except: pass
  else:
    if x>999: x=999
    ws+="r"+str(x).zfill(3)

  try:    x=round(wx["raintoday"]*RAININCH)
  except: pass
  else:
    if x>999: x=999
    ws+="P"+str(x).zfill(3)

  try:    x=round(wx["rain24h"]*RAININCH)
  except: pass
  else:
    if x>999: x=999
    ws+="p"+str(x).zfill(3)

  try:    x=round(wx["pressure"]*10.0)
  except: pass
  else:
    if x>99999: x=99999
    ws+="b"+str(x).zfill(5)

  try:    x=round(wx["luminosity"])
  except: pass
  else:
    if x<=999: ws+="L"
    else:
      x-=1000
      if x>999: x=999
      ws+="l"
    ws+=str(x).zfill(3)

  try:    x=round(wx["nanosievert"])
  except: pass
  else:
    e=0
    while (x>=100.0) and (e<9):
      e+=1
      x=x*0.1
    x=round(x)
    if x>99: x=99
    x=x*10+e
    ws+="X"+str(x).zfill(3)

  try:    x=round(wx["finedust10"])
  except: pass
  else:
    if x>999: x=999
    ws+="m3"+str(x).zfill(3)

  try:    x=round(wx["finedust2"])
  except: pass
  else:
    if x>999: x=999
    ws+="m2"+str(x).zfill(3)

  try:    x=round(wx["finedust1"])
  except: pass
  else:
    if x>999: x=999
    ws+="m1"+str(x).zfill(3)

  try:    x=round(wx["finedust01"])
  except: pass
  else:
    if x>999: x=999
    ws+="m0"+str(x).zfill(3)
  return ws


def wrfile(fn, tmpfn, line):   # rename file to avoid sending partial written datas
  if tmpfn: tf=tmpfn
  else: tf=fn 
  try:
    with open(tf, "w") as fd:
      res=fd.write(line)
      fd.close()
      if tmpfn:
        p=Path(tf)
        p.rename(fn)
  except:
    print("file create error")
     
     
#--------------------- build aprs beacon in monitor format     


def aprstime(sec):
  utc=datetime.utcnow()
  if sec: t=""
  else: t=str(utc.day).zfill(2)
  t+=str(utc.hour).zfill(2) + str(utc.minute).zfill(2)
  if sec: t+=str(utc.second).zfill(2) + "h"
  else: t+="z"
  return t

def lataprs(lat):
  a=round(abs(lat)*6000)
  d=a//6000
  s=str(round(d)).zfill(2)
  d=a-d*6000
  s+=str(round(d//100)).zfill(2) + "." + str(round(d-(d//100)*100)).zfill(2)
  if lat>=0.0: s+="N"
  else: s+="S"
  return s

def longaprs(lat):
  a=round(abs(lat)*6000)
  d=a//6000
  s=str(round(d)).zfill(3)
  d=a-d*6000
  s+=str(round(d//100)).zfill(2) + "." + str(round(d-(d//100)*100)).zfill(2)
  if lat>=0.0: s+="E"
  else: s+="W"
  return s

def complat(lat):
  l=round(380926.0*(90.0-lat))
  return chr(33+l//R3) + chr(33+l%R3//R2) + chr(33+l%R2//R1) + chr(33+l%R1)

def complong(long):
  l=round(190463.0*(180.0+long))
  return chr(33+l//R3) + chr(33+l%R3//R2) + chr(33+l%R2//R1) + chr(33+l%R1)

def beacontext(mycall, destcall, path, time, symbol, lat, long, wx, comment, compressed):
  s=mycall + ">" + destcall
  if path: s+="," + path
  s+=":"
  if time: s+="/" + aprstime(time==2)
  else: s+="!"
  if compressed:
    s+=symbol[0] + complat(lat) + complong(long) + symbol[1]
  else:
    s+=lataprs(lat) + symbol[0] + longaprs(long) + symbol[1]
  s+=wxaprs(wx, compressed) + comment
  return s


#------------------------ text to axudp

def axcall(text, pos):
  l=len(text)
  a=""
  while (pos<l) and (len(a)<6) and ((text[pos]>="0") and (text[pos]<="9") or (text[pos]>="A") and (text[pos]<="Z")):
    a+=chr(ord(text[pos])<<1)
    pos+=1
  while len(a)<6: a+=chr(ord(" ")<<1)                     #fill with spaces
  ssid=0
  if (pos<l) and (text[pos]=="-"):
    pos+=1
    if (pos<l) and (text[pos]>="0") and (text[pos]<="9"):
       ssid+=ord(text[pos])-ord("0")
       pos+=1
    if (pos<l) and (text[pos]>="0") and (text[pos]<="9"):
       ssid=ssid*10 + ord(text[pos])-ord("0")
       pos+=1
    if ssid>15: ssid=15
  ssid=(ssid+48)<<1
  if (pos<l) and (text[pos]=="*"):
    ssid|=0x80
    pos+=1
  a+=chr(ssid)
  return a, pos
  
def sendax(sock, text, ip, values):
  a,p=axcall(text, 0)                                  #src call
  if (p>=len(text)) or (text[p]!=">"): return
  ax,p=axcall(text, p+1)                               #dest call
  ax+=a
  hbit=0
  while True:                                          #via calls
    if p>=len(text): return                            #found no end of address
    if text[p]==":": break                             #end of address field
    if text[p]!=",": return                            #via path error
    if len(ax)>=70:  return                            #too many via calls           
    a,p=axcall(text, p+1)
    ax+=a 
    hp=len(ax)-1
    if (ord(ax[hp]) & 0x80)!=0: hbit=hp                #store last h-bit
  p+=1
  a=""

  if values:
    a="\x01\x30"                                       #axudp v2 start

    try: v=values["level"]
    except: pass
    else: a+="V"+str(round(v))+" "                     #axudp v2 append level

    try: v=values["quality"]
    except: pass
    else: a+="Q"+str(round(v))+" "                     #axudp v2 append quality

    try: v=values["txdel"]
    except: pass
    else: a+="T"+str(round(v))+" "                     #axudp v2 append txdel

    try: v=values["snr"]
    except: pass
    else: a+="S"+str(round(v))+" "                     #axudp v2 append snr

    try: v=values["afc"]
    except: pass
    else: a+="A"+str(round(v))+" "                     #axudp v2 append afc

    a+="\x00"                                          #axudp2 end

  i=0
  for i in range(len(ax)):
    ch=ord(ax[i])
    if (i%7==6) and (i>=20) and (i<hbit): ch|=0x80     #set h-bit on all via calls before
    if i+1==len(ax): ch|=1                             #set end of address bit
    a+=chr(ch)
  a+="\x03\xf0"                                        #ui frame pid F0
  i=0
  while p<len(text):                                   #append payload
     a+=text[p]
     p+=1
     i+=1
     if i>=256: abort                                  #max 256bytes
# for i in range(0,len(a)): print(hex(ord(a[i])))     
# sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
  c=udpcrc(a, len(a))
  a+=chr(c & 0xff)
  a+=chr(c>>8)
  sa=array("B",[0]*len(a))
  for i in range(0,len(a)): sa[i]=ord(a[i])
  res=sock.sendto(sa, ip)
  
#------------------------------axudp to text

def callstr(b, p):
  s=""
  for i in range(6):
    ch=ord(b[p+i])>>1
    if ch<32: s+="^"                                     #show forbidden ctrl in call
    elif ch>32:s+=chr(ch)                                #call is filled with blanks
  ssid=(ord(b[p+6])>>1) & 0x0f
  if ssid: s+="-"+str(ssid)
  return s
      
def axtostr(axbuf):
  ext={}
  b=""
  for x in axbuf: b+=chr(x)
  le=len(b)
  if le<2: return "",ext

  le-=2
  c=udpcrc(b, le)
  if (b[le]!=chr(c & 0xff)) or (b[le+1]!=chr(c>>8)): return "",ext  #crc error

  i=0
  if (axbuf[0]==1) and (axbuf[1]!=0):                               #axudp v2
    ext["stat"]=chr(axbuf[1])                                       #dcd, ptt, txready or ? for requestl 
    i=2
    while (i<len(axbuf)) and (axbuf[i]!=0):                         #values 0 terminated
      ix=chr(axbuf[i])                                              #label
      i+=1
      s=""
      while (i<len(axbuf)) and (axbuf[i]>32):                       #0 or blank terminated string
        s+=chr(axbuf[i])
        i+=1
      ext[ix]=s
      if axbuf[i]!=0: i+=1
    i+=1
  b=""
  while i<len(axbuf):
    b+=chr(axbuf[i])
    i+=1
  s=""
  le=len(b)
  if le>=18:                                             #2 calls + ctrl + pid + crc
    le-=2
    s=callstr(b, 7)                                      #src call
    s+=">"+callstr(b, 0)                                 #destination call
    p=14
    hbit=False
    while (((not (ord(b[p-1]) & 1)))) and (p+6<le):      #via path
      if ord(b[p+6])>=128: hbit=True
      elif hbit:                                         #call before had hbit
        s+="*"
        hbit=False   
      s+=","+callstr(b, p)
      p+=7
    if hbit: s+="*"                                      #last call had hbit
    p+=2                                                 #pid, ctrl
    s+=":"
    while p<le:                                          #payload may contain ctrl characters
      s+=b[p]
      p+=1
  return s,ext
  
#------------------------------axudp rx
def rxax(ip):
  sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
  sock.bind(ip)

  while True:
    data, addr=sock.recvfrom(400)   #als option empfangene ip mit sende ip vergleichen
#    for x in data: print(x)
#    print(axtostr(data))
    frame,st=axtostr(data)
    print(frame)
    print("axudp2: ",st)    
  
#exec(open("wx.py").read())  
# wx.sendax("OE0AAA-15>TEST,WIDE1-1:>blabla",("127.0.0.1",7001),{"level":-28,"quality":97,"txdel":133})
# wx.sendax(wx.beacontext("OE0AAA-1","CQ-1","",2,"/_",78.25,13.002,{"celsius":2,"winddir":36,"wind":83,"rain24h":37,"pressure":1013.76}," pywx test",False),("127.0.0.1",7001),{})
# wx.rxax(("192.168.1.45", 7000))