#!/bin/bash

function start()
{
	pidfile="robot.pid"
	binfile="bin"
	exefile="robot_client"
	if [ -f $pidfile ];then
        echo "Exist pid file, please check robot is already running. if not, please delete pid file and try again."
        exit 0;
    fi

	if [ ! -d ${binfile} ];then
		echo "${binfile} not exist."
		exit 0;
	fi
	echo $LD_LIBRARY_PATH | grep -q '/usr/local/lib'
    if [ $? -eq 1 ];then
    	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    	echo "export /usr/local/lib path."
	else
		echo "lib path has been set."
    fi
    
    ulimit -c unlimited
	if [[ $? -eq 0 ]];then
		echo "set core space success."
	fi
    
	cd ${binfile}
    nohup ./${exefile} >/dev/null & echo $! > ../${pidfile} 2>&1 &
	if [[ $? -eq 0 ]];then
		echo "run robot success."
	else
		echo "run robot failed."
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


if [ $# -lt 1 ];then
	echo "usage:./s.sh start or ./s.sh stop"
	exit 1
fi

command=$1

case $command in
	"start")
		echo "starting robot program..."
		start
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
