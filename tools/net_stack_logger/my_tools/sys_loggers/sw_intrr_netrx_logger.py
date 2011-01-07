#!/usr/bin/env python

import os
import re
import time
import datetime


CORE_NUM = 8

class SWInttrNetRxLogger(object):
    
    def __init__(self):
        self.path = "/proc/softirqs"
        self.dir_path = ""
        self.net_rx = 0
        self.raw_log = []
        
        f=open(self.path)
        tmp = f.readlines()
        f.close()
        
        identifier = re.compile("\s*NET_RX:")
        
        for i,line in enumerate(tmp):
            if identifier.match(line) != None:
                self.net_rx = i
                break
        else:
            print "no param eth"


    def read(self):
        f=open(self.path)
        self.raw_log.append( f.readlines()[self.net_rx] )
        f.close()


    def raw_write(self):
        f = open(self.dir_path +"/raw/sw_inttr_net_rx.log","w")
        f.writelines(self.raw_log)
        f.close()

    
    def format_write(self):
        intrr = map(lambda x : x.split()[1:CORE_NUM+1],self.raw_log)
        intrr = map(lambda x : map(int, x),intrr) # String to Integer

        delta_intrr = []

        prev = intrr[0]
        
        for curr in intrr[1:]:
            d = map( lambda (x,y) : y - x , zip(prev, curr) )
            d = map(str,d) # to String
            delta_intrr.append(d)
            prev = curr

            
        f = open(self.dir_path + "/format/sw_intrr_net_rx.log","w")
        f.writelines( ','.join(["CPU%d"%i for i in range(CORE_NUM)])+",CPU_ALL\n")
        f.writelines(map(lambda x : ','.join(x)+"\n" , delta_intrr))
        f.close()


    
    def write(self):
        self.raw_write()
        self.format_write()


        
if __name__ == '__main__':
    logger = [ SWInttrNetRxLogger() ]

    for x in range(10):
        for l in logger:
            l.read()

        time.sleep(1)

    time = datetime.datetime.today().strftime("%Y%m%d_%H%M%S")
    raw_dir =  time + "/raw"
    format_dir = time + "/format"
    os.makedirs( raw_dir )
    os.makedirs( format_dir )

    
    for l in logger:
        l.dir_path = time
        l.write()
