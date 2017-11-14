#!/bin/bash

array=("01" "02" "03" "04" "05" "06" "07" "08" "09" "10" 
       "11"     "17" "18" "19" "20")

for i in "${array[@]}"
do
    ./run.sh $i
done
