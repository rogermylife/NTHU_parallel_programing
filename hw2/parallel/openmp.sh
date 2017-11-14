#!/bin/bash

threadNum=("1" "2" "3" "4")
checkFile=../ta/777.png

for i in "${threadNum[@]}"
do
    time srun -p batch -c $i ./ms_omp $i -2 2 -2 2 777 777 "omp777_"${i}".png"
    result=$(diff "omp777_"${i}".png" $checkFile)
    if [$result -eq '']; then
        echo $i"  Correct!!!!"
    else
        echo$i"   Wrong!!!!"
        echo $result
    fi
done
