#!/bin/bash

threadNum=("4" "6" "8" "12")
checkFile=d9600.png

time srun -p batch -n 4 -c12 ./ms_hybrid 12 -2 2 -2 2 2400 9600 "h9600_4.png"
result=$(diff "h9600_4.png" $checkFile)
if [$result -eq '']; then
    echo $i"  Correct!!!!"
else
    echo $i"   Wrong!!!!"
    echo $result
fi

time srun -p batch -n 6 -c8 ./ms_hybrid 8 -2 2 -2 2 2400 9600 "h9600_6.png"
result=$(diff "h9600_6.png" $checkFile)
if [$result -eq '']; then
    echo $i"  Correct!!!!"
else
    echo $i"   Wrong!!!!"
    echo $result
fi

time srun -p batch -n 8 -c6 ./ms_hybrid 6 -2 2 -2 2 2400 9600 "h9600_8.png"
result=$(diff "h9600_8.png" $checkFile)
if [$result -eq '']; then
    echo $i"  Correct!!!!"
else
    echo $i"   Wrong!!!!"
    echo $result
fi

time srun -p batch -n 12 -c4 ./ms_hybrid 4 -2 2 -2 2 2400 9600 "h9600_12.png"
result=$(diff "h9600_12.png" $checkFile)
if [$result -eq '']; then
    echo $i"  Correct!!!!"
else
    echo $i"   Wrong!!!!"
    echo $result
fi
