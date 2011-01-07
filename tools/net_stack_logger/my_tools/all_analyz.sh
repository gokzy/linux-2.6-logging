#!/bin/sh

for file in `find nsl_log/ -name "*.txt"`
do
    cat ${file} | python analyzer.py -u > ${file%.txt}_packet.csv
    cat ${file} | python analyzer.py -s > ${file%.txt}_sock.csv
done

for file in `find nsl_log/ -name "*_packet.csv" | grep -E "ps7_.*_bw10_" `
do
    echo -n ${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/[cpu\1,\2byte,\3Mbps]/g'
    cat ${file} | tail -n 6 | grep "average"
done

#########################################################################################################
# comp packet size 16,128,1024 byte
# cpu 2 or 7
#########################################################################################################
# cpu2
output="comp_result/comp_packet_size_samecpu_lowload.csv"
echo "" > ${output}
echo ",RUN_HW_INTERRUPT,WAIT_SW_INTERRUPT,RUN_SW_INTTERRUPT,WAIT_SKB_DEQUEUE,SKB_COPY,FINISH,HW_INTERRUPT - TO_SYSCALL,HW_INTERRUPT - END_INET_RECVMSG" >> ${output}
for file in `find nsl_log/ -name "*_packet.csv" | grep -E "ps2_.*_bw1_" `
do
    echo -n ${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/\2,/g' >> ${output}
    cat ${file} | tail -n 6 | grep "average" | sed "s/\[average\],//g" >> ${output}
done
mv ${output} ${output}_
sort -n ${output}_ > ${output}
rm ${output}_


# cpu7
output="comp_result/comp_packet_size_diffcpu_lowload.csv"
echo "" >  ${output}
echo ",RUN_HW_INTERRUPT,WAIT_SW_INTERRUPT,RUN_SW_INTTERRUPT,WAIT_SKB_DEQUEUE,SKB_COPY,FINISH,HW_INTERRUPT - TO_SYSCALL,HW_INTERRUPT - END_INET_RECVMSG" >> ${output}
for file in `find nsl_log/ -name "*_packet.csv" | grep -E "ps7_.*_bw1_" `
do
    echo -n ${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/\2,/g' >> ${output}
    cat ${file} | tail -n 6 | grep "average" | sed "s/\[average\],//g" >> ${output}
done
mv ${output} ${output}_
sort -n ${output}_ > ${output}
rm ${output}_



# ### cpu2, bandwidth 10Mbps
# echo "" > comp_packet_size_samecpu.csv
# echo ",RUN_HW_INTERRUPT,WAIT_SW_INTERRUPT,RUN_SW_INTTERRUPT,WAIT_SKB_DEQUEUE,SKB_COPY,FINISH,HW_INTERRUPT - TO_SYSCALL,HW_INTERRUPT - END_INET_RECVMSG" >> comp_packet_size_samecpu.csv
# for file in `find nsl_log/ -name "*_packet.csv" | grep -E "ps2_.*_bw10_" `
# do
#     echo -n ${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/\2,/g' >> comp_packet_size_samecpu.csv
#     cat ${file} | tail -n 6 | grep "average" | sed "s/\[average\],//g" >> comp_packet_size_samecpu.csv
# done
# mv comp_packet_size_samecpu.csv _comp_packet_size_samecpu.csv
# sort -n _comp_packet_size_samecpu.csv > comp_packet_size_samecpu.csv
# rm _comp_packet_size_samecpu.csv


# ### comp packet size
# ### cpu7, bandwidth 10Mbps
# echo "" > comp_packet_size_diffcpu.csv
# echo ",RUN_HW_INTERRUPT,WAIT_SW_INTERRUPT,RUN_SW_INTTERRUPT,WAIT_SKB_DEQUEUE,SKB_COPY,FINISH,HW_INTERRUPT - TO_SYSCALL,HW_INTERRUPT - END_INET_RECVMSG" >> comp_packet_size_diffcpu.csv
# for file in `find nsl_log/ -name "*_packet.csv" | grep -E "ps7_.*_bw10_" `
# do
#     echo -n ${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/\2,/g' >> comp_packet_size_diffcpu.csv
#     cat ${file} | tail -n 6 | grep "average" | sed "s/\[average\],//g" >> comp_packet_size_diffcpu.csv
# done
# mv comp_packet_size_diffcpu.csv _comp_packet_size_diffcpu.csv
# sort -n _comp_packet_size_diffcpu.csv > comp_packet_size_diffcpu.csv
# rm _comp_packet_size_diffcpu.csv




#########################################################################################################
# comp bandwidth 1,10,100,100
# packet 16,128,1024 byte
# cpu 2 or 7
#########################################################################################################
### cpu2
output="comp_result/comp_bandwidth_diffcpu.csv"
echo "" > ${output}
echo ",RUN_HW_INTERRUPT,WAIT_SW_INTERRUPT,RUN_SW_INTTERRUPT,WAIT_SKB_DEQUEUE,SKB_COPY,FINISH,HW_INTERRUPT - TO_SYSCALL,HW_INTERRUPT - END_INET_RECVMSG" >> ${output} 
for file in `ls nsl_log/ | grep -E "nsl_ps(7)_(16|128|1024)byte_bw(1|10|100|1000)_packet.csv" `
do
    echo -n nsl_log/${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/\3_\2,/g' >> ${output} 
    cat nsl_log/${file} | tail -n 6 | grep "average" | sed "s/\[average\],//g" >> ${output} 
done
# mv ${output} ${output}_
# sort -n ${output}_ > ${output}  
# rm ${output}_

### cpu7
output="comp_result/comp_bandwidth_samecpu.csv"
echo "" > ${output}
echo ",RUN_HW_INTERRUPT,WAIT_SW_INTERRUPT,RUN_SW_INTTERRUPT,WAIT_SKB_DEQUEUE,SKB_COPY,FINISH,HW_INTERRUPT - TO_SYSCALL,HW_INTERRUPT - END_INET_RECVMSG" >> ${output}
for file in `ls nsl_log/ | grep -E "nsl_ps(7)_(16|128|1024)byte_bw(1|10|100|1000)_packet.csv" `
do
    echo -n nsl_log/${file} | sed 's/.*\/nsl_ps\([0-9]*\)_\([0-9]*\)byte_bw\([0-9]*\)_packet.csv/\3_\2,/g' >> ${output}
    cat nsl_log/${file} | tail -n 6 | grep "average" | sed "s/\[average\],//g" >> ${output}
done
# mv ${output} ${output}_
# sort -n ${output}_ > ${output}
# rm ${output}_
