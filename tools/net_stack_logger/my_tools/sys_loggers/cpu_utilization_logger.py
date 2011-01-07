#!/usr/bin/evn python

import re
import os
import time
import datetime

class CpuUtilizationLogger(object):
    
    def __init__(self, ):
        global CORE_NUM
        self.path = "/proc/stat"
        self.dir_path = ""
        self.raw_log = []
        self.cpu_num = 0
        
        identifier = re.compile("cpu")
    
        f=open("/proc/stat")
        stat_log = f.readlines()
        f.close()
    
        for i,line in enumerate(stat_log):
            if identifier.match(line) == None:
                CORE_NUM = i-1
                break
        else:
            print "no param eth"


    def read(self):
        f=open(self.path)
        self.raw_log.append( f.readlines()[0:CORE_NUM+1] )
        f.close()

        
    def makeCpuUtilizationList(self):
        cul = []

        for rl in self.raw_log:
            cul.append(map(lambda x : map(int , x.split()[1:]),rl))

        return cul

            
    def cpuUtilizationTimeList(self,log):
        
        for i in range(1,len(log)):
            yield log[i-1],log[i]

            
    def eachCpuUtilizationList(self,prev,curr):

        for p,c in zip(prev,curr):
            yield p,c

            
    def analyzeCpuUtilization(self,log):
        format_list = []

        for prev,curr in self.cpuUtilizationTimeList(log):
            l = []
            for p,c in self.eachCpuUtilizationList(prev,curr):
                tmp = map(lambda (x,y) : y - x, zip(p,c))
                
                if sum(tmp) == 0:
                    tmp = map(lambda x : float(x),tmp)
                else:
                    tmp = map(lambda x : float(x)/sum(tmp), tmp)
                
                l.append(tmp[:7])
                
            format_list.append(l)

        return format_list
        
    
    def format_write(self,format_list):
        f = open(self.dir_path + "/format/cpu_utilization.log","w")

        for cpu in range(CORE_NUM+1):
            if cpu == 0:
                f.write("CPU ALL\n")
            else:
                f.write("CPU%d\n"%(cpu-1))

            f.write("USER,NICE,SYS,IDLE,IO,HW_INTRR,SW_INTRR\n")
            for fl in format_list:
                f.write( ','.join( map(str,fl[cpu]) ) + "\n")

            f.write("\n")
                
        f.close()

        
    def raw_write(self):
        f = open(self.dir_path +"/raw/cpu_used.log","w")
        for l in self.raw_log:
            f.writelines(l)
        f.close()

        
    def write(self):
        self.raw_write()
        cu_list = self.makeCpuUtilizationList()
        format_list = self.analyzeCpuUtilization(cu_list)
        self.format_write(format_list)


if __name__ == '__main__':
    logger = [ CpuUtilizationLogger() ]

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
