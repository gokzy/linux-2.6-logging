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
    tail_func = NSL_END_INET_RECVMSG
    func = ["", 'RUN_HW_INTERRUPT','WAIT_SW_INTERRUPT','RUN_SW_INTTERRUPT','WAIT_SKB_DEQUEUE',
            'SKB_COPY','FINISH','HW_INTERRUPT - TO_SYSCALL','HW_INTERRUPT - END_INET_RECVMSG']
    values = [[] for x in range(8)]

elif opts.sock:
    protocol = '0'
    head_func = NSL_BEGIN_INET_RECVMSG
    tail_func = NSL_END_INET_RECVMSG
    func = ["","begin_inet_recvmsg","end_inet_recvmsg","cnt"]
    values = [[] for x in range(2)]


    
PROTOS = {0:'sock',17:'udp',6:'tcp'}

flow = {}

reader = csv.DictReader(sys.stdin)

for packet in reader:
    
    if packet['ip_protocol'] != protocol and packet['ip_protocol'] != '0':
        continue

    id = packet['sock_id' if opts.sock else 'skb_id']
    
    if flow.has_key(id):
        flow[id][packet['func']] = packet
    else:
        flow[id] = { packet['func'] : packet }
        
    
    if packet['func'] == tail_func:
        if head_func in flow[id] :
            
            print "[%s/%s/%s]," % (PROTOS[int(packet['ip_protocol'])],
                                   packet['tp_sport' if packet['ip_protocol'] == '0' else 'tp_dport'],
                                   packet['skb_id']),


            prev = flow[id][head_func]

            sorted_keys = sorted( map(int, flow[id].keys() ) )
            sorted_keys = map(str, sorted_keys)
            
            for i, key in enumerate( sorted_keys[1:] ):
                curr = flow[id][key]
                period = (int(curr['time']) - int(prev['time']))
                print "%d," % period,
                values[i].append(period)
                prev = curr

                
            if opts.tcp or opts.udp:
                packets = flow[id]
                
                period = ( int(packets[NSL_SKB_DEQUEUE]['time']) - int(packets[NSL_NETIF_RECEIVE_SKB]['time']) )
                print "%d," % period,
                values[6].append(period)

                period = ( int(packets[NSL_END_INET_RECVMSG]['time']) - int(packets[NSL_NETIF_RECEIVE_SKB]['time']) )
                print "%d," % period,
                values[7].append(period)

                # period = ( (int(packets[NSL_END_INET_RECVMSG]['time']) - int(packets[NSL_NETIF_RECEIVE_SKB]['time'])) -
                #            (int(packets[NSL_SKB_DEQUEUE]['time']) - int(packets[NSL_NETIF_RECEIVE_SKB]['time'])))
                # print "%d," % period,
                # values[8].append(period)

            elif opts.sock:
                sockets = flow[id]
                cnt = int( sockets[NSL_END_INET_RECVMSG]['cnt']) - int(sockets[NSL_BEGIN_INET_RECVMSG]['cnt'])
                print "%d," % cnt,
                values[1].append(cnt)
                
                    
            print
                
        del flow[id]


print
print ','.join(func)
print "[average],", ', '.join(  map(str, map(numpy.average, values) ) )
print "[median],",  ', '.join(  map(str, map(numpy.median,  values) ) )
print "[max],",     ', '.join(  map(str, map(numpy.max,     values) ) )
print "[min],",     ', '.join(  map(str, map(numpy.min,     values) ) )
print "[varance],", ', '.join(  map(str, map(numpy.var,     values) ) )
