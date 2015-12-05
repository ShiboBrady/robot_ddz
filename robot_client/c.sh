#!/bin/bash

if [ $# -lt 1 ];then
	echo "usage: ./c.sh filepath1 filepath2..."
	exit 1;
fi

confFilePath="configure/"
binFilePath="bin/"
logFile="log/"
shellFile="s.sh"

for desPath in $@;do
	echo $desPath

    if [ ! -d ${desPath} ];then
        echo "${desPath} is not a legal path, will to create it."
		mkdir -p ${desPath}
    fi
    
    #if [ ! -d ${desPath}/${confFilePath} ];then
        cp -rf ${confFilePath} ${desPath}
    #fi
    
    cp -rf ${binFilePath} ${shellFile} ${desPath}
    
    mkdir -p ${desPath}/${logFile}

done

exit 0;
