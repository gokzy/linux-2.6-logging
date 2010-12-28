#!/usr/bin/env python


class Packet(object):

    def __init__(self,log):
        self.mem_addr = log["skb"]
        self.cpu      = log["cpu"]
        self.eth_prot = log["eth_protocol"]
        self.ip_prot  = log["ip_protocol"]
        self.ip_saddr = log["ip_saddr"]
        self.ip_daddr = log["ip_daddr"]
        self.tp_sport = log["tp_sport"]
        self.tp_dport = log["tp_dport"]
        
        self.times    = {log["func"] : int(log["time"])}
        self.delta_time = {}

        
    def pushLog(self,log):
        self.times[log["func"]] = int(log["time"])

        
    
    def calucTime(self):
        KEY,VAL = 0,1
        
        t = sorted(self.times.items(), key=lambda x : x[1])

        self.delta_time[t[0][KEY]] = 0
        prev = t[0]

        for curr in t[1:]:
            print "%d - %d = %d"%(curr[VAL] , prev[VAL] , (curr[VAL] - prev[VAL]))
            self.delta_time[curr[KEY]] = curr[VAL] - prev[VAL]
            prev = curr

    
    def __str__(self):
        str = ",".join([self.mem_addr, self.ip_saddr, self.ip_daddr])

        KEY,VAL = 0,1
        t = sorted(self.times.items(), key=lambda x : x[1])
        times = "{ " + ', '.join( map(lambda x : "%s : %d"%(x[KEY], self.times[x[KEY]]),t) ) + " }"
        delta_time = "{ " + ', '.join( map(lambda x : "%s : %d"%(x[KEY], self.delta_time[x[KEY]]),t) ) + " }"
        str = str + "\n" + times.__str__() + "\n" + delta_time.__str__()
        return str



def parseParameter(log):
    return dict( map(lambda x : x.split(":"), log.split()) )

if __name__ == "__main__":
    packets = {}

    print "loading file..."
    f = open("log.txt")
    log = f.readlines()
    f.close()
    print "load complete."

    print "parsing parameter..."
    log = map(parseParameter, log[1:])
    print "parsing complete."

    print "..."
    for l in log:
        if l["skb"] in packets:
            packets[l["skb"]].pushLog(l)
        else:
            packets[l["skb"]] = Packet(l)
    print "complete"

    print "..."
    for v in packets.values():
        v.calucTime()
    print "complete"
        
    for v in packets.values():
        print v
    
    
    print "total packets : %d"%len(packets)
