#!/bin/sh
PROXY=192.168.10.180
TYPE=0
count=1
while [ 1 ]
do
    echo ./testClient ${PROXY} 192.168.10.66 $TYPE $count
    ./testClient ${PROXY} 192.168.10.66 $TYPE $count
    TYPE=$(($TYPE+1))
    count=$((count+1))
    if [ $TYPE -gt 5 ];then
        TYPE=0
    fi
    echo ./testClient ${PROXY} 192.168.10.66 6 $count
    ./testClient ${PROXY} 192.168.10.66 6 $count
    count=$((count+1))
done
