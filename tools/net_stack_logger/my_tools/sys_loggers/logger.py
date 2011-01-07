#!/usr/bin/env python

from hw_intrr_logger import *
from sw_intrr_netrx_logger import *
from packet_recive_loggler import *
from cpu_utilization_logger import *
from proces_stat_logger import *

import sys


if __name__ == '__main__':
    dir_name = ""
    
    if len(sys.argv) > 1:
        dir_name = sys.argv[1]
    else:
        dir_name = datetime.datetime.today().strftime("%Y%m%d_%H%M%S")
        
    logger = [ HWIntrrEthLogger(), SWInttrNetRxLogger(), CpuUtilizationLogger(), PacketsAndBytesLogger(),ProcessStatLogger()]

    for x in range(40):
        for l in logger:
            l.read()

        time.sleep(1)

    raw_dir =  dir_name + "/raw"
    format_dir = dir_name + "/format"
    os.makedirs( raw_dir )
    os.makedirs( format_dir )

    
    for l in logger:
        l.dir_path = dir_name
        l.write()

