#!/bin/sh
#
if [ "$#" -ne 2 ]
then
	echo "need 2 arguments"
	exit 1
fi
filesdir=$1
searchstr=$2

if [ ! -d "$filesdir" ]
then
	echo "not directory"
	exit 1
fi

files=$(ls $filesdir | wc -l) 
lines=$(grep $searchstr -rn $filesdir | wc -l)  
echo "The number of files are $files and the number of matching lines are $lines"
