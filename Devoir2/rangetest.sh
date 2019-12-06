#!/bin/bash
./compile.sh
: > testresults/rangetest.log
if [ "$#" -ne 3 ]; then
    echo "usage : from to step"
    exit 1
fi
for((i=$1;i<=$2;i+=$3));
do
    (./gen $i | ./d2s ; ./gen $i | ./d2p ; echo $i) |tr '\n' '\t' >> testresults/rangetest.log
    echo "" >> testresults/rangetest.log
done    