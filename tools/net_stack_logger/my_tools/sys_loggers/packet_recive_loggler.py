#!/usr/bin/env pyhton

import re
import os
import time
import datetime


class PacketsAndBytesLogger(object):
    
    def __init__(self, ): 
        self.path = "/proc/net/dev"
        self.raw_log = []
        self.dir_path = ""
        self.eth_line = 0

        f=open(self.path)
        tmp = f.readlines()
        f.close()
        
        identifier = re.compile(".*eth0.*\n")
        
        for i,line in enumerate(tmp):
            if identifier.match(line) != None:
                self.eth_line = i
                break
        else:
            print "/proc/net/dev no param eth0"


        
        
    def read( self ):
        f = open(self.path)
        self.raw_log.append( f.readlines()[self.eth_line] )
        f.close()


    def raw_write(self):
        f = open(self.dir_path +"/raw/packets_bytes.log","w")
        f.writelines(self.raw_log)
        f.close()
        
        
    def format_write(self):
        REC_BYTES, REC_PACKETS = 1,2
        f = open(self.dir_path + "/format/packets_bytes.log","w")
    
        p = self.raw_log
        p = map(lambda x: x.split(), p)
        p = map(lambda x: [ int(x[REC_BYTES]), int(x[REC_PACKETS]) ], p)
        
        d_net = []
        
        prev = p[0]
        for i in range(1,len(p)):
            curr = p[i]
            d_net.append(map(lambda (x,y) : y - x, zip(prev,curr) ))
            prev = curr

        p = map(lambda x : ','.join( map(str, x) ), d_net)
        
        for i,pp in enumerate(p):
            f.writelines(pp)
            f.write("\n")

        f.close()


    def write(self):
        self.raw_write()
        self.format_write()
