#!/usr/bin/evn python

import os
import re
import copy
import time
import datetime


class ProcessStatLogger(object):

    elem = { "pid"         : 0,  "comm"       : 1,  "state"       : 2,  "ppid"      : 3,  "pgrp"        : 4,  
             "session"     : 5,  "tty_nr"     : 6,  "tpgid"       : 7,  "flags"     : 8,  "minflt"      : 9, 
             "cminflt"     : 10, "majflt"     : 11, "cmajflt"     : 12, "utime"     : 13, "stime"       : 14,
             "cutime"      : 15, "cstime"     : 16, "priority"    : 17, "nice"      : 18, "num_threads" : 19,
             "itrealvalue" : 20, "starttime"  : 21, "vsize"       : 22, "rss"       : 23, "rsslim"      : 24,
             "startcode"   : 25, "endcode"    : 26, "startstack"  : 27, "kstkesp"   : 28, "kstkeip"     : 29,
             "signal"      : 30, "blocked"    : 31, "sigignore"   : 32, "sigcatch"  : 33, "wchan"       : 34,
             "nswap"       : 35, "cnswap"     : 36, "exit_signal" : 37, "processor" : 38, "rt_priority" : 39,
             "policy"      : 40, "guest_time" : 41, "cguest_time" : 42}

    def __init__(self):
        self.raw_logs = []
        self.format_process_list = []
        self.analyzed_log = {} # key is pid
        self.dir_path = ""

        
    def getPIDs(self):
        pids = os.listdir("/proc")
        pids = pids[ pids.index('1') : ]
        return pids


    def read(self):
        pids = self.getPIDs()
        
        log = []

        for path in map(lambda pid : "/proc/%s/stat" % pid, pids):
            try:
                f = open(path)
                log += f.readlines()
            except IOError:
                pass
            finally:
                f.close()

        self.raw_logs.append(log)
    

    def makeProcesStatList(self):
        PID = self.elem["pid"]
        STAT = self.elem["state"]
        COMM = self.elem["comm"]
        UTIME = self.elem["utime"]
        STIME = self.elem["stime"]
        PRIORITY = self.elem["priority"]
        PRSS = self.elem["processor"]

        psl = []
        
        for log in self.raw_logs:
            log = map(lambda x : x.split(), log)
            stat = {}

            for l in log:
                stat[ l[PID] ] = [ l[STAT], l[COMM], l[UTIME], l[STIME], l[PRIORITY], l[PRSS] ]
            
            psl.append( stat )
            
        #     tmp = {}
        #     for k,v in sorted(log.items() , key=lambda x : x[1][-1]):
        #         tmp[k] = v

        #     sorted_psl.append( tmp )

        self.format_process_list = psl


    def prevCurrProcessStatList(self):
        for time in range(1,len(self.format_process_list)):
            yield self.format_process_list[time-1], self.format_process_list[time]


    def analyzeUseTime(self):
        STAT,COMM,UTIME,STIME,PRIORITY,PRSS = range(6)
        f = open(self.dir_path + "/format/proc_stat.log","w")
        logs = []

        psl = {}
        process_list = {}
        for time,(prev,curr) in enumerate(self.prevCurrProcessStatList()):
            f.write("########## TIME %d ###########\n"%(time))
            f.write("PID,COMM,UTIME,STIME,PROSESSOR,PRIORITY,STAT\n")

            for pid,stat in curr.items():
                psl[pid] = copy.deepcopy(stat)
                if pid in prev:
                    psl[pid][UTIME] = str(int(curr[pid][UTIME]) - int(prev[pid][UTIME]))
                    psl[pid][STIME] = str(int(curr[pid][STIME]) - int(prev[pid][STIME]))

            for pid,stat in sorted(psl.items() , key=lambda x : x[1][-1]):
                f.write("%s,%s,%s,%s,%s,%s,%s\n"%(pid,stat[COMM],stat[UTIME],stat[STIME],stat[PRSS],stat[PRIORITY],stat[STAT]))

            logs.append(psl)
        f.close()

    def analyzeProsessorHasProsess(self):
        f = open(self.dir_path + "/format/cpus_set_process_num.log","w")

        php = []

        for time,process in enumerate(self.format_process_list):
            cpus = [ 0 for x in range(8) ]
            for pid,stat in process.items():
                cpus[int(stat[-1])] += 1

            php.append(cpus)
        
        f.write(','.join( [ "CPU%d"%x for x in range(8) ])+"\n")

        for p in php:
            f.write( ','.join( map( str , p ) )+"\n" )

        f.close()

        return php
        

    def analyz(self):
        PID = self.elem["pid"]
        COMM = self.elem["comm"]
        UTIME = self.elem["utime"]
        STIME = self.elem["stime"]
        PRIORITY = self.elem["priority"]
        PRSS = self.elem["processor"]
        
        self.makeProcesStatList()
        self.analyzeUseTime()
        self.analyzeProsessorHasProsess()
        # prev = self.raw_logs[0]                

        # for p in prev:
        #     p =  p.split()
        #     print "pid : %s \t comm : %s \t processor %s \t utime : %s \t stime : %s \t priority %s" % \
        #     (p[PID],p[COMM],p[PRSS],p[UTIME],p[STIME],p[PRIORITY])


    def write(self):
        self.analyz()
