#!/usr/bin/python
# Apple II Chat server program
import struct, socket, select, time
import xml.etree.ElementTree as ET

HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 0x6502             # Apple II Chat non-privileged port
VERSION = 1
client_list = {}
chat_files = {}
chat_vers  = []

def client_add(address, port, handle):
	global client_list
	client_list[address] = (port, handle)
	print "Welcome, ", handle, "@", address, ":", port

def broadcast(handle, msg):
	global client_list
	if msg:
		print handle, ": ", msg
	bcastmsg = struct.pack('<HHHBc8p32p', 0x7EDA, VERSION, 0, 0xCA, 'C', handle, msg)
	for c in client_list:
	       	client = (c, client_list[c][0])
	       	s.sendto(bcastmsg, client)

def send_update(client, ver):
	updatemask = 0
	updatelist = []
	for i in xrange(ver, VERSION):
		updatemask |= chat_vers[i]
	for f in chat_files:
		if updatemask & chat_files[f][2]:
			updatelist.append(f)
	print "Update client version ", ver, " with:", updatelist
	pkthdr = struct.pack('<HHHBc', 0x7EDA, VERSION, 0, 0xCA, 'U')
	pktmsg = ""
	for f in updatelist:
		pktmsg += struct.pack('<17pBH', f, chat_files[f][0], chat_files[f][1])
	pktmsg += struct.pack('B', 0)
	s.sendto(pkthdr + pktmsg, client)
#
# Read version XML file
#
tree = ET.parse('chat-version.xml')
root = tree.getroot()
for chatfile in root.findall('{updates}file'):
	fname = chatfile.get('name')
	ftype = int(chatfile.get('type'), 0)
	faux  = int(chatfile.get('aux'),  0)
	fmask = int(chatfile.get('mask'), 0)
	chat_files[fname] = (ftype, faux, fmask)
for chatver in root.findall('{updates}version'):
	chat_vers.insert(int(chatver.get('level')), int(chatver.get('updates'), 0))
chatver = root.find('{updates}current')
VERSION = int(chatver.get('level'))
print "CHAT server version:", VERSION
#
# Initialize UDP socket
#
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((HOST, PORT))
p = select.poll()
p.register(s.fileno(), select.POLLIN)
#
# Main server loop
#
while 1:
        if p.poll(1000):
        	data, client = s.recvfrom(2048)
		address, port = client
		magic, ver, seq, hw, req = struct.unpack_from('<HHHBc', data)
                if req == 'H':
                        handle, = struct.unpack_from('8p', data, 8)
			if ver <> VERSION:
				send_update(client, ver)
			else:
				client_add(address, port, handle)
				s.sendto(struct.pack('<HHHBc', magic, ver, seq, 0xCA, 'W'), client)
				broadcast(handle, "")
                elif req == 'C':
			try:
				msg, = struct.unpack_from('32p', data, 8)
				handle = client_list[address][1]
				broadcast(handle, msg)
			except:
				s.sendto(struct.pack('<HHHBc', 0x7EDA, VERSION, 0, 0xCA, 'E'), client)
                elif req == 'F':
			try:
				filename, fileblock = struct.unpack_from('<17pH', data, 8)
				f = open('clientfiles/'+filename, 'r')
				f.seek(fileblock * 1024)
				msg = f.read(1024)
				f.close()
				s.sendto(struct.pack('<HHHBc', 0x7EDA, VERSION, seq, 0xCA, 'F') + msg, client)
			except:
				s.sendto(struct.pack('<HHHBc', 0x7EDA, VERSION, seq, 0xCA, 'E'), client)
		else:
			print "Unknown request: " + req
        else:
		pass
s.close()
                                                                    
