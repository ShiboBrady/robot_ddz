#!/bin/bash

program=(game1 game2 game3 game4 three six_dali six_taotai ass)

function start()
{
    pidpath="pid"
	pidfile="robot."{$1}".pid"
	binfile="bin"
	exefile="robot_client"
    libpath="/usr/local/lib"
    
    mkdir -p $pidpath
    cd pidpath
	if [ -f $pidfile ];then
        echo "Exist pid file, please check robot is already running. if not, please delete pid file and try again."
        exit 0;
    fi
    cd -

	if [ ! -d ${binfile} ];then
		echo "${binfile} not exist."
		exit 0;
	fi

    cd $binfile
    if [ ! -f ${exefile} ];then
        echo "${exefile} not exist."
        exit 0;
    cd -

	echo $LD_LIBRARY_PATH | grep -q "$libpath"
    if [[ $? -eq 1 ]];then
    	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${libpath}
    	echo "export ${libpath} path."
	else
		echo "lib path has been set."
    fi
    
    ulimit -c unlimited
	if [[ $? -eq 0 ]];then
		echo "set core space success."
	fi
    
	cd ${binfile}
    nohup ./${exefile} $1 >/dev/null &
    pid=$!
    ps -ef | grep ${exefile} | grep ${pid}
	if [[ $? -eq 0 ]];then
		echo "run robot success, pid is ${pid}."
        echo ${pid} > ${pidpath}/${pidfile}
        echo "write pid to file ${pidpath}/${pidfile}."
	else
		echo "run robot with param $1 failed."
	fi
	cd -
}

function stop()
{
	pidfile="robot.pid"
	if [ ! -f $pidfile ];then
		echo "No pid file."
		exit 0;
	fi

	if [ ! -s $pidfile ];then
		echo "pid file is empty."
		exit 0;
	fi

	PID=$(cat robot.pid)

	echo "pid is: ${PID}"

	kill -2 $PID

	if [[ $? -eq 0 ]];then
		echo "stop robot program $PID success."
	else
		echo "stop robot program $PID failed."
	fi
	rm -rf ${pidfile}
	echo "delete pidfile."
}

function startProgram()
{
    if [ "$1"x = ""x ];then
        echo "please into param."
        exit 1;
    elif [ "$1"x = "all"x ];then
        echo "all"
    else
        echo ${program[@]} | grep -wq $1
        if [ $? -eq 0 ];then
            echo "legle"
        else
            echo "unlegle"
            exit 1;
        fi
    fi
}

if [ $# -lt 1 ];then
	echo "usage:./s.sh start or ./s.sh stop"
	exit 1
fi

command=$1

case $command in
	"start")
		echo "starting robot program..."
		startProgram $2
	;;
	"stop")
		echo "stopping robot program..."
		stop
	;;
	*)
		echo "param error."
	;;
esac
exit 0;
