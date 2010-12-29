import csv
import sys

reader = csv.DictReader(sys.stdin)

flow = {}
for packet in reader:
    if packet['ip_protocol'] != '17':
        continue
    if flow.has_key(packet['id']):
        flow[packet['id']].append(packet)
    else:
        flow[packet['id']] = [packet]
    if packet['func'] == '7':
        first = flow[packet['id']][0]
        for p in flow[packet['id']]:
            print "%s %d, " % (p['func'],
                               int(p['time'], 16) - int(first['time'], 16)),
        print
        del flow[packet['id']]
