#!/bin/bash

# Do not uncomment these lines to directly execute the script
# Modify the path to fit your need before using this script
#hdfs dfs -rm -r /user/TA/CalculateAverage/Output/
#hadoop jar CalculateAverage.jar calculateAverage.CalculateAverage /user/shared/CalculateAverage/Input /user/TA/CalculateAverage/Output
#hdfs dfs -cat /user/TA/CalculateAverage/Output/part-*
if [ -z "$1" ]||[ -z "$2" ] ;then 
    echo "variables are undefined";
    exit 1
fi
if [[ "$2" == "" ]]; then
	iter=3
else
	iter=$2
fi

INPUT_FILE=/user/ta/PageRank/Input/input-$1
#INPUT_FILE=/user/p105062548/PageRank/mytest
OUTPUT_FILE=/user/p105062548/PageRank/output-$1
TMP_FILE=Temp
JAR=PageRank.jar

make
hdfs dfs -rm -r $TMP_FILE
hdfs dfs -rm -r $OUTPUT_FILE
hadoop jar $JAR pagerank.PageRank $INPUT_FILE $OUTPUT_FILE $iter
hdfs dfs -getmerge $OUTPUT_FILE pagerank.txt
#hdfs dfs -getmerge $TMP_FILE/Parse parse.txt
#hdfs dfs -getmerge $TMP_FILE/Init init.txt
#hdfs dfs -getmerge $TMP_FILE/Calculate calculate.txt

hw5-fancydiff $1 $2 pagerank.txt
