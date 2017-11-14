#!/bin/bash

threadNum=("4" "6" "8" "12")
checkFile=d9600.png

for i in "${threadNum[@]}"
do
    time srun -p batch -n $i -c4 ./ms_hybrid 4 -2 2 -2 2 2400 9600 "h9600_"${i}".png"
    result=$(diff "h9600_"${i}".png" $checkFile)
    if [$result -eq '']; then
        echo $i"  Correct!!!!"
    else
        echo$i"   Wrong!!!!"
        echo $result
    fi
done
