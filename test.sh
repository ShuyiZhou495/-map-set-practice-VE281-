#!/bin/bash

for i in 0 1 2 3 4 5
do
    echo "now size $i"
    ./main --verbose -m -p --transfers --ttt AMZN --ttt AMD< testcase/$i.in >myout.txt
    diff testcase/0.out myout.txt
done

