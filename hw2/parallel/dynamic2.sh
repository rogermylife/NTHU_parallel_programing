#!/bin/bash

threadNum=("5" "9" "16" "32" "48")
checkFile=d9600.png

for i in "${threadNum[@]}"
do
    time srun -p batch -n $i ./ms_mpi_dynamic 4 -2 2 -2 2 2400 9600 "d9600_"${i}".png"
    result=$(diff "d9600_"${i}".png" $checkFile)
    if [$result -eq '']; then
        echo $i"  Correct!!!!"
    else
        echo$i"   Wrong!!!!"
        echo $result
    fi
done
