#!/bin/bash

: > testresults/$1.log
./gen $2
for((i=0;i<1;i++));
do
    ./$1 < $2 >> testresults/$1.log
done
rm $2
awk '{ total += $0; count++ } END { print total/count }' testresults/$1.log
# awk '{ total += $0; count++ } END { print total/count }' testresults/$1.log >> testresults/awkOut.log