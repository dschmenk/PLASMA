#!/usr/bin/python
import sys, struct, socket, select

server = ("localhost", 0x6502)
myhandle = "Python"
VERSION = 6

if len(sys.argv) > 1:
    server = (sys.argv[1], 0x6502)
if len(sys.argv) > 2:
    myhandle = sys.argv[2]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(struct.pack('<HHHBc8p', 0x7EDA, VERSION, 1, 0xFF, 'H', myhandle), server)
data, server = s.recvfrom(2048)
magic, ver, seq, hw, res = struct.unpack_from('<HHHBc', data)
if res == 'U':
    ulistofst = 8
    print "Update file list: (", len(data), " bytes)"
    while len(data) - ulistofst >= 20:
        filename, filetype, fileaux = struct.unpack_from('<17pBH', data, ulistofst)
        if filename == "": break
        print filename, filetype, fileaux
        ulistofst += 20
elif res <> 'W':
    print "Server rejected HELLO"
else:
    p = select.poll()
    p.register(s, select.POLLIN)
    p.register(sys.stdin, select.POLLIN)
    while 1:
        pollin = p.poll()
        if len(pollin) > 0:
            if pollin[0][0] == sys.stdin.fileno():
                instr = raw_input()
                if len(instr) == 0: break
                s.sendto(struct.pack('<HHHBc32p', 0x7EDA, VERSION, 1, 0xFF, 'C', instr), server)
            elif pollin[0][0] == s.fileno():
                data, server = s.recvfrom(2048)
                magic, ver, seq, hw, req = struct.unpack_from('<HHHBc', data)
                if req == 'C':
                    handle, msg = struct.unpack_from('8p32p', data, 8)
                    if msg:
                        print handle, ": ", msg
                    #elif handle <> myhandle:
                    else:
                        print "Welcome, ", handle
s.close()