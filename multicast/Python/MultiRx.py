# Simple script to receive UDP packets

import sys,socket, traceback, struct,binascii
import sys

multicast_group = '239.1.2.3'
host = ''
iport = 35123
tfstruct = struct.Struct('!LB')

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((host, iport))

group = socket.inet_aton(multicast_group)
mreq = struct.pack('4sL', group, socket.INADDR_ANY)
s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

print('Listening on group:',multicast_group,' and port ',iport);

try:
    s.settimeout(200)
    message, address = s.recvfrom(8192)
    print ("Received packet [",message,"] from ", address[0]) #probably want to add to a list


except  socket.error:
    errno, errstr = sys.exc_info()[:2]
    if errno == socket.timeout:
        sys.exit() # Could return here or something else
    raise
except:
    traceback.print_exc()



