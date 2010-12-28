import csv
import sys

reader = csv.DictReader(sys.stdin)

flow = {}
for packet in reader:
    if packet['ip_protocol'] != '17':
        continue
    if flow.has_key(packet['skb']):
        flow[packet['skb']].append(packet)
    else:
        flow[packet['skb']] = [packet]
    if packet['func'] == '7':
        first = flow[packet['skb']][0]
        for p in flow[packet['skb']]:
            print "%s %d, " % (p['func'], int(p['time']) - int(first['time'])),
        print
        del flow[packet['skb']]
