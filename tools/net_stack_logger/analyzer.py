from optparse import OptionParser
import numpy
import csv
import sys


IPPROTO_UDP = '17'
IPPROTO_TCP = '6'


NSL_NETIF_RECEIVE_SKB =  '1'
NSL_ENQUEUE_TO_BACKLOG = '2'
NSL_SKB_DEQUEUE = '6'
NSL_SKB_COPY = '7'
NSL_BEGIN_INET_RECVMSG = '13'
NSL_END_INET_RECVMSG  = '14'


opt = OptionParser()
opt.add_option('-t','--tcp',action='store_true')
opt.add_option('-u','--udp',action='store_true')
opt.add_option('-s','--sock',action='store_true')


opts , argv = opt.parse_args()


protocol = ''
head_func = ''
tail_func = ''
func = []
tmp = []


if opts.tcp:
    protocol = IPPROTO_TCP
    head_func = NSL_NETIF_RECEIVE_SKB
    tail_func = NSL_SKB_COPY
    fucn = []
    tmp = []
    exit(0)
    
elif opts.udp:
    protocol = IPPROTO_UDP
    head_func = NSL_NETIF_RECEIVE_SKB
    tail_func = NSL_SKB_COPY
    func = ["", 'netif_receive_skb', 'enqueue_to_backlog', '__netif_receive_skb',
            'ip_rcv', 'sk_data_ready', 'skb_dequeue', 'skb_copy', 'to_syscall']
    tmp = [[] for x in range(8)]

elif opts.sock:
    protocol = '0'
    head_func = NSL_BEGIN_INET_RECVMSG
    tail_func = NSL_END_INET_RECVMSG
    func = ["","begin_inet_recvmsg","end_inet_recvmsg","cnt"]
    tmp = [[] for x in range(3)]


    
PROTOS = {0:'sock',17:'udp',6:'tcp'}

flow = {}

reader = csv.DictReader(sys.stdin)


for packet in reader:
    
    if packet['ip_protocol'] != protocol:
        continue

    id = packet['id']
    
    if flow.has_key(id):
        flow[id].append(packet)
    else:
        flow[id] = [packet]
        
    if packet['func'] == tail_func:
        if flow[id][0]['func'] == head_func:
            first = flow[id][0]
            
            print "[%s/%s]," % (PROTOS[int(packet['ip_protocol'])], packet['tp_sport' if packet['ip_protocol'] == '0' else 'tp_dport']),

            
            for i,p in enumerate(flow[id]):
                print "%d," % (int(p['time']) - int(first['time'])),
                tmp[i].append((int(p['time']) - int(first['time'])))
                first = p
                i += 1

                
            if opts.tcp or opts.udp:
                first = flow[id][0]
                dequeue = None
                
                for p in flow[id]:
                    if p['func'] == NSL_SKB_DEQUEUE:
                        dequeue = p
                        break
                    
                print "%d," % (int(dequeue['time']) - int(flow[id][0]['time'])),
                tmp[-1].append((int(dequeue['time']) - int(flow[id][0]['time'])))

            elif opts.sock:
                first = flow[id][0]
                for p in flow[id][1:]:
                    print "%d," % (int(p['cnt']) - int(first['cnt'])),
                    tmp[-1].append(int(p['cnt']) - int(first['cnt']))
                    
            print
                
        del flow[id]


if tmp[-2] == []:
    func.remove('enqueue_to_backlog')
    tmp.pop(-2)
    
print
print ','.join(func)
print "[average],", ', '.join(  map(str, map(numpy.average, tmp) ) )
print "[median],",  ', '.join(  map(str, map(numpy.median,  tmp) ) )
print "[max],",     ', '.join(  map(str, map(numpy.max,     tmp) ) )
print "[min],",     ', '.join(  map(str, map(numpy.min,     tmp) ) )
print "[varance],", ', '.join(  map(str, map(numpy.var,     tmp) ) )
