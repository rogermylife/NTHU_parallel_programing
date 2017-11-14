#!/bin/bash

threadNum=("1" "2" "3" "4")
checkFile=../ta/777.png

for i in "${threadNum[@]}"
do
    time srun -p batch -n $i ./ms_mpi_static 4 -2 2 -2 2 777 777 "s777_"${i}".png"
    result=$(diff "s777_"${i}".png" $checkFile)
    if [$result -eq '']; then
        echo $i"  Correct!!!!"
    else
        echo$i"   Wrong!!!!"
        echo $result
    fi
done
