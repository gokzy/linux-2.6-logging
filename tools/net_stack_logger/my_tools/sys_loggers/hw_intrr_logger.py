#!/usr/bin/env python

import re
import os
import time
import datetime

CORE_NUM = 8


class HWIntrrEthLogger(object):

    def __init__(self):
        self.path = "/proc/interrupts"
        self.dir_path = ""
        self.raw_log = []
        self.eth_line = 0


        f=open(self.path)
        tmp = f.readlines()
        f.close()
        
        identifier = re.compile("eth0\n")
        
        for i,line in enumerate(tmp):
            if identifier.match(line[-5:]) != None:
                self.eth_line = i
                break
        else:
            print "no param eth0"


            
    def read(self):
        f = open("/proc/interrupts")
        self.raw_log.append( f.readlines()[self.eth_line] )
        f.close()

        
    def raw_write(self):
        f = open(self.dir_path +"/raw/hw_inttr_eth.log","w")
        f.writelines(self.raw_log)
        f.close()

    
    def format_write(self):
        intrr = map(lambda x : x.split()[1:CORE_NUM+1], self.raw_log)
        intrr = map(lambda x : map(int, x), intrr) # String to Integer

        delta_intrr = []

        prev = intrr[0]
        for curr in intrr[1:]:
            d = map( lambda (x,y) : y - x , zip(prev, curr) )
            d = map(str,d) # to String
            delta_intrr.append(d)

            prev = curr
            

        f = open(self.dir_path + "/format/hw_intrr_eth.log","w")
        
        f.writelines( ','.join(["CPU%d"%i for i in range(CORE_NUM)])+"\n")
        f.writelines( map(lambda x : ','.join(x)+"\n" , delta_intrr))
        
        f.close()

    
    def write(self):
        self.raw_write()
        self.format_write()

        
if __name__ == '__main__':
    logger = [ HWIntrrEthLogger() ]

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
