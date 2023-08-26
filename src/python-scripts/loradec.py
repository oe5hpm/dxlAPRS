import json, base64, socket, wx

AXUDP=("127.0.0.1",4020)
PORT=5100
IP=("0.0.0.0",PORT)
metainfo=True

sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind(IP)

while True:
  data, addr=sock.recvfrom(1500)
# print(data)
      
  try:
    obj = json.loads(data.decode())
#   print(obj)
    payload=base64.b64decode(obj["payload"])                    #base64 coded frame
    print("payload=", payload)
  except: print("json decode error")
  else:
    if obj["crc"]!=1:  print("no crc or crc error")
    elif obj["net"]!=0x12: print("wrong net-id")
    elif len(payload)>5:
      j=0                                                       #seek begin of frame
      while (j<3) and ((payload[j]<ord("0")) or (payload[j]>ord("9"))) and ((payload[j]<ord("A")) or (payload[j]>ord("Z"))): j+=1
      s=""
      for i in range(j,len(payload)): s+=chr(payload[i])        #remove junk and make string type

      if metainfo and not (s.find(",") in range(0,s.find(":"))):    # only if no digi path
        try: v=obj["snr"]
        except: pass
        else: s+=" snr="+"{:.1f}".format(v)+"dB"
        try: v=obj["afc"]
        except: pass
        else: s+=" afc="+"{:.0f}".format(v)+"Hz"
        try: v=obj["preamb"]
        except: pass
        else: s+=" txd="+"{:.0f}".format(v)+"ms"
        try: v=obj["eye"]
        except: pass
        else: s+=" q="+"{:.0f}".format(v)+"%"
        try:
          v=obj["sf"]
          vi=obj["invers"]
        except: pass
        else:
           s+=" sf="+"{:.0f}".format(v)
           if vi: s+="(inv)"
        try: v=obj["cr"]
        except: pass
        else: s+=" cr="+"{:.0f}".format(v)
        try: v=obj["ver"]
        except: pass
        else: s+=" vers="+v

      print("frame:",s)
      try:  wx.sendax(sock, s, AXUDP, {"level": obj["level"], "quality": obj["eye"], "txdel": obj["preamb"], "afc": obj["afc"], "snr": obj["snr"]}) 
      except: print("axudp encode error")
                                                                        


