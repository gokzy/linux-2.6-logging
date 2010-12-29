import csv
import sys

reader = csv.DictReader(sys.stdin)

IPPROTO_UDP = '17'
IPPROTO_TCP = '6'

NSL_UDP_RECVMSG = '7'
NSL_TCP_RCV_ESTABLISHED = '8'

flow = {}
for packet in reader:
    if packet['ip_protocol'] != IPPROTO_UDP and packet['ip_protocol'] != IPPROTO_TCP:
        continue
    if flow.has_key(packet['id']):
        flow[packet['id']].append(packet)
    else:
        flow[packet['id']] = [packet]
    if packet['func'] == NSL_UDP_RECVMSG or packet['func'] == NSL_TCP_RCV_ESTABLISHED:
        first = flow[packet['id']][0]
        for p in flow[packet['id']]:
            print "%s %d, " % (p['func'],
                               int(p['time'], 16) - int(first['time'], 16)),
        print
        del flow[packet['id']]
