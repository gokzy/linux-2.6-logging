#!/usr/bin/env python

import commands
import socket
import sys
import time

CPU,SIZE,BANDWIDTH = 0,1,2

HOST, PORT = "hyuganatsu", 9901
for x in range(10):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        break
    except:
        time.sleep(5)
else:
    sys.exit(0)

sock.send("GET\n")
print "GET"
ret = sock.recv(1024)
print ret

if ret == "FIN":
    ret = commands.getoutput("su goda -c \"cd /home/goda/net_stack_logger/;tar zcvf nsl_log.tgz nsl_log iperf_log sys_log\"")
    sock.send("FIN")
    exit(0)


param = ret.split(",")
ret = commands.getoutput("su goda -c \"/home/goda/net_stack_logger/logging_init.sh %s %s %s\""%(param[CPU],param[SIZE],param[BANDWIDTH]) )
print ret

sock.send("END\n")

commands.getoutput("reboot")
        
sock.close()    
