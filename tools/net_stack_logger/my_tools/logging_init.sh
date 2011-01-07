#!/bin/sh

cd /home/goda/net_stack_logger/sys_log

python ../sys_logger.py ps$1_dg$2_bw$3 &

cd ../
./logging.sh $1 $2 $3

