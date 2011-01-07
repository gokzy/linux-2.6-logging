import SocketServer
import commands
import sys


class BenchForNSL(SocketServer.StreamRequestHandler):
    param = None

    def benchCmdParamGenerator(self):
        """
        return parameter : [ cpu_um, packet_size, bandwidth ]
        """
	cpu_number    = [2,7]
	#cpu_number = [7]
        bandwidth     = [1,10,18,20,50,100,1000]
	#bandwidth = [1000]
        datagram_size = [16,32,64,128,256,512,1024,4096,65536]
        #datagram_size = [16,1024]
	for cpu in cpu_number:
            for bw in bandwidth:
                for size in datagram_size:
                    yield map(str , [cpu,size,bw])


            
    def handle(self):
        CPU,SIZE,BANDWIDTH = range(3)

        client_addr = self.client_address[0]
        
        if BenchForNSL.param == None:
            print "self.param == None"
            BenchForNSL.param = self.benchCmdParamGenerator()

            
        while True:
            mesg = self.rfile.readline().split()[0]
        
            print mesg
        
            if mesg == "GET":
                try:
                    param = next(self.param)
                    print "send mesg %s." % param
                    
                    self.wfile.write( ",".join(param) )
                    
                    commands.getoutput( "iperf -c %s -u -b %sM -i 1 -t 40 -l %s" % (client_addr,param[BANDWIDTH],param[SIZE]) )
                    
                except StopIteration:
                    self.param = None
                    self.wfile.write("FIN")

            elif mesg == "END":
                print "exit"
                self.wfile.write("OK")
                break
                
            elif mesg == "FIN":
		print "test finish."
		break



if __name__ == "__main__":
    HOST, PORT = "", 9901
    
    server = SocketServer.TCPServer((HOST, PORT), BenchForNSL)
    
    server.serve_forever()
