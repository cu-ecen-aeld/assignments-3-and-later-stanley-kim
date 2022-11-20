#!/bin/sh
#
if [ "$#" -ne 2 ]
then
        echo "need 2 arguments"
        exit 1
fi
writefile=$1
writestr=$2

dir_name=$(dirname $writefile)
if [ ! -d "$dir_name" ]
then
	echo "mkdir new"
	mkdir "$dir_name"
fi       

if [ ! -f "$writefile" ]
then
	echo "touch new file"
	touch "$writefile"
fi
echo "$writestr" >> $writefile

