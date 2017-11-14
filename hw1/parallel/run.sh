#!/bin/bash

if [ -z "$1" ]; then
    echo 'please enter filenumber'
    exit 1
fi

inputFile="../samples/testcase$1"
outputFile="rogersorted$1"
checkFile="../samples/sorted$1"
fileSize=$(du -b "$inputFile" | cut -f 1)
number=$(($fileSize/4))

echo "run hw1_basic $number $inputFile $outputFile $checkFile"
time srun -p batch -N 4 -n 1 ./HW1_105062548_basic.o  $number $inputFile $outputFile
time srun -p batch -N 4 -n 4 ./HW1_105062548_basic.o  $number $inputFile $outputFile
time srun -p batch -N 4 -n 8 ./HW1_105062548_basic.o  $number $inputFile $outputFile
time srun -p batch -N 4 -n 12 ./HW1_105062548_basic.o  $number $inputFile $outputFile

result=$(diff $outputFile $checkFile)
if [$result -eq '']; then
    echo 'Correct!!!'
else
    echo 'Wrong!!!!'
    echo $result
fi;
