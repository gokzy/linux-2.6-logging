import csv
import sys

reader = csv.DictReader(sys.stdin)

IPPROTO_UDP = '17'
IPPROTO_TCP = '6'
PROTOS = [None, None, None, None, None, None, 'tcp', None, None, None, None, None, None, None, None, None, None, 'udp']

NSL_SKB_COPY = '7'
NSL_SKB_FREE = '8'
FUNCS = [None, 'netif_receive_skb', 'enqueue_to_backlog', '__netif_receive_skb', 'ip_rcv', 'sk_data_ready', 'skb_dequeue', 'skb_copy']

flow = {}
for packet in reader:
    if packet['ip_protocol'] != IPPROTO_UDP and packet['ip_protocol'] != IPPROTO_TCP:
        continue
    if flow.has_key(packet['id']):
        flow[packet['id']].append(packet)
    else:
        flow[packet['id']] = [packet]
    if packet['func'] == NSL_SKB_COPY:
        first = flow[packet['id']][0]
        print "[%s/%s] " % (PROTOS[int(packet['ip_protocol'])], packet['tp_dport']),
        for p in flow[packet['id']]:
            print "%s:%d, " % (FUNCS[int(p['func'])],
                               int(p['time']) - int(first['time'])),
        print
        del flow[packet['id']]
