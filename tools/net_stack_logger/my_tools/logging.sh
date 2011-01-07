#!/bin/sh

echo "pid" $$ , "CPUs" $1

mkdir /dev/cpuset/$$
echo "create /dev/cpuset/"$$

echo $1 > /dev/cpuset/$$/cpus
echo $1 " > /dev/cpuset/"$$"/cpus"

echo 0  > /dev/cpuset/$$/mems
echo "0  > /dev/cpuset/"$$"/mems"

echo $$ > /dev/cpuset/$$/tasks
echo $$ " > /dev/cpuset/"$$"/tasks"

cd /home/goda/net_stack_logger/

iperf -s -u -i 1 -l $2 > iperf_log/iperf_ps$1_$2byte_bw$3.txt &

sleep 5
./nsl start

sleep 30
./nsl stop

sleep 5

./nsl get > nsl_log/nsl_ps$1_$2byte_bw$3.txt

kill $!
