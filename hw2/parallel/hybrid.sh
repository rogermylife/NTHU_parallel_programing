#!/bin/bash

threadNum=("2" "3" "4" "5" "6" "8" "12" "16" "32")
checkFile=../ta/777.png

for i in "${threadNum[@]}"
do
    time srun -p batch -n $i -c4 ./ms_hybrid 4 -2 2 -2 2 777 777 "h777_"${i}".png"
    result=$(diff "h777_"${i}".png" $checkFile)
    if [$result -eq '']; then
        echo $i"  Correct!!!!"
    else
        echo$i"   Wrong!!!!"
        echo $result
    fi
done
