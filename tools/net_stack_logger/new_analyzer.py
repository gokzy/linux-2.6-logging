#!/usr/bin/env python

import csv
import sys
import numpy


NSL_START_LOGGING         = 1
NSL_POLLING               = 2
NSL_GRO_PROCESS           = 3
NSL_RPS_ENQUEUE_BACKLOG   = 4
NSL_RPS_DEQUEUE_BACKLOG   = 5

NSL_IP_RCV                = 6
NSL_ENQUEUE_FRAGMENTS     = 7

NSL_TCP_V4_RCV            = 8
NSL_TCP_ENQUEUE_BACKLOG   = 9
NSL_TCP_V4_DO_RCV         = 10
NSL_TCP_DEQUEUE_BACKLOG   = 11
NSL_FASTPATH_DATA_READY   = 12
NSL_SLOWPATH_DATA_READY   = 13
NSL_OFOPATH_DATA_READY    = 14
NSL_SYSCALL_PROCESS_START = 15
NSL_COPY_SKB_COMPLETE     = 16
NSL_DEQUEUE_RECEIVE_QUEUE = 17

NSL_UDP_RCV               = 18
NSL_UDP_DATA_READY        = 19

NSL_TCP_FOUND_FIN_OK        = 30
NSL_TCP_BEFORE_CLEANUP_RBUF = 31
NSL_TCP_AFTER_CLEANUP_RBUF  = 32
NSL_TCP_OUT                 = 33
NSL_TCP_RECV_URG            = 34


process_dict = {(1,2) : "POLLING",
                (2,3) : "GRO_PROCESS",
                
                (3,5) : "RPS_GET_CPU",
                (3,4) : "RPS_ENQUEUE_BACKLOG",
                (4,5) : "RPS_DEQUEUE_BACKLOG",
                (5,6) : "STEER_PKT_PROT_FUNK",
                
                (6,8) : "DELIVER_TRANSPORT_LAYER",
                (6,7) : "ENQUEUE_FRAGMENTS",
                (7,8) : "REASSMBLEY_PROCESS",
                
                (8,10)  : "CHOICE_PATH",
                (10,12) : "FASTPATH_DATA_READY",
                (10,13) : "SLOWPATH_DATA_READY",
                (10,14) : "OFOPATH_DATA_READY",

                (8,9)   : "TCP_ENQUEUE_BACKLOG",
                (9, 11) : "TCP_DEQUEUE_BACKLOG",
                (11,10) : "START_DATA_READY",

                (12,15) : "START_SYSCALL",
                (13,15) : "START_SYSCALL",
                (14,15) : "START_SYSCALL",

                (15,16) : "COPY_SKB_COMPLETE",
                (16,17) : "DEQUEUE_RECEVE_QUEUE"
                }


class ReceiveFlow(dict):

    def __init__(self):
        self.path = []
        self.exit = False
        self.process_span = []

        
    def __setitem__(self, key, val):
        super(ReceiveFlow,self).__setitem__(key,val)

        if key == NSL_DEQUEUE_RECEIVE_QUEUE:
            self.path = self.path_generator()
        
            if self.path == None:
                return False


            if self.is_complete_data():
                self.calc_process_span()
                self.exit = True

                
    def is_complete_data(self):
        for func in self.path:
            if not func in self.keys():
                return False

        return True

            
    def calc_process_span(self):
        prev = self.__getitem__(self.path[0])

        for curr_key in self.path[1:]:
            curr = self.__getitem__(curr_key)

            self.process_span.append( int(curr['time']) - int(prev['time']) )
            prev = curr


class TcpReceiveFlow(ReceiveFlow):

    def __init__(self):
        super(TcpReceiveFlow, self).__init__()

    
    def path_generator(self):
        path = []
        
        if NSL_RPS_ENQUEUE_BACKLOG in self.keys():
            path += [1,2,3,4,5,6]
        else:
            path += [1,2,3,5,6]

        if NSL_TCP_ENQUEUE_BACKLOG in self.keys():
            path += [8,9,11,10]
        else:
            path += [8,10]

        if NSL_FASTPATH_DATA_READY in self.keys():
            path += [12]
            
        elif NSL_SLOWPATH_DATA_READY in self.keys():
            path += [13]

        elif NSL_OFOPATH_DATA_READY in self.keys():
            path += [14]
        else:
            return None

        path += [15,16,17]

        return path


    
def statistical_data(data):
    for key,val in process_times.items():
        

        print ",",
        x = key[0]
        for y in key[1:]:
            print process_dict[(x,y)], ",",
            x = y
        print

        print "[average],", ', '.join( map(str, map(numpy.average, val) ) )
        print "[median],",  ', '.join( map(str, map(numpy.median,  val) ) )
        print "[max],",     ', '.join( map(str, map(numpy.max,     val) ) )
        print "[min],",     ', '.join( map(str, map(numpy.min,     val) ) )
        print "[varance],", ', '.join( map(str, map(numpy.var,     val) ) )
        print


flows = {}
process_times = {}
        
reader = csv.DictReader(sys.stdin)

for pkt in reader:
    
    if (pkt['ip_protocol'] != '6' or pkt['tp_dport'] != '5001') and pkt['func'] != '1':

        continue

    id = pkt["skb_id"]
    
    if id in flows:
        flows[id][int(pkt['func'])] = pkt
    else:
        flows[id] = TcpReceiveFlow()
        flows[id][int(pkt['func'])] = pkt

        
    if flows[id].exit:
        key = tuple(flows[id].path)
        
        if key in process_times:
            for i,ps in enumerate( flows[id].process_span ):
                process_times[key][i].append( ps )
        else:
            process_times[key] = [ [ps] for ps in flows[id].process_span ]
            
        del flows[id]
        
statistical_data(process_times)
        

