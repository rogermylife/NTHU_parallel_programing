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

echo "run hw1_seq $number $inputFile $outputFile $checkFile"
time ./hw1_seq.o $number $inputFile $outputFile

result=$(diff $outputFile $checkFile)
if [$result -eq '']; then
    echo 'Correct!!!'
else
    echo 'Wrong!!!!'
    echo $result
fi;
