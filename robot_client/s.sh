#!/bin/bash

program=(game1 game2 game3 game4 three six_dali six_taotai ass newer free_timer)

function startprogram()
{
    echo "Begin to start robot program for $1 ..."
    pidfilepath="pid"
    pidfile="robot."$1".pid"
    binfile="bin"
    exefile="robot_client"
    libpath="/usr/local/lib"
    #libpath="/home/zhangsb/usr/lib"
    #libpath="/home/zhangshibo/usr/lib"
    logfilepath="log/"$1

    mkdir -p $logfilepath
    mkdir -p $pidfilepath
    cd $pidfilepath
    if [ -f $pidfile ];then
        echo "Exist pid file, please check robot is already running. if not, please delete pid file and try again."
        cd -
        return
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
    fi
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
    cd -
    ps -ef | grep ${exefile} | grep -q ${pid}
    if [[ $? -eq 0 ]];then
        echo "run robot success, pid is ${pid}."
        echo ${pid} > ${pidfilepath}/${pidfile}
        if [[ $? -eq 0 ]];then
            echo "write pid to file ${pidfilepath}/${pidfile} successed."
        else
            echo "write pid to file ${pidfilepath}/${pidfile} failed."
            exit 1
        fi

    else
        echo "run robot with param $1 failed."
    fi
}

function stopprogram()
{
    echo "Begin to stop robot program in $1 ..."
    pidfilepath="pid"
    pidfile="robot."$1".pid"
    killcommand="-2"
    
    if [ ! -d $pidfilepath ];then
        echo "Doesn't exit pid file ${pidfilepath}."
        exit 0;
    fi  
    
    cd ${pidfilepath}
    if [ ! -f $pidfile ];then
        echo "No pid file ${pidfile}."
        cd -
        return
    fi

    if [ ! -s $pidfile ];then
        echo "pid file ${pidfile} is empty."
        cd -
        return;
    fi

    PID=$(cat ${pidfile})

    echo "pid is: ${PID}"

    kill ${killcommand} $PID

    if [[ $? -eq 0 ]];then
        echo "stop robot program $PID success."
    else
        echo "stop robot program $PID failed."
    fi
    rm -rf ${pidfile}
    echo "delete pidfile ${pidfile}."
    cd -
}

function start()
{
    if [ "$1"x = ""x ];then
        echo "please enter second param."
        exit 1;
    elif [ "$1"x = "all"x ];then
        for item in ${program[@]}
        do
            startprogram $item
            sleep 1
        done
    else
        echo ${program[@]} | grep -wq $1
        if [ $? -eq 0 ];then
            startprogram $1
        else
            echo "useless second param, please check."
            exit 1;
        fi
    fi
}

function stop()
{
    if [ "$1"x = ""x ];then
        echo "please enter second param."
        exit 1;
    elif [ "$1"x = "all"x ];then
        for item in ${program[@]}
        do
            stopprogram $item
            sleep 1
        done
    else
        echo ${program[@]} | grep -wq $1
        if [ $? -eq 0 ];then
            stopprogram $1
        else
            echo "useless second param, please check."
            exit 1;
        fi
    fi
}

function status()
{
    pidfilepath="pid"
    pidfileprefix="robot."
    pidfileendfix=".pid"
    exefile="robot_client"

    if [ ! -d ${pidfilepath} ];then
        echo "pid file ${pidfilepath} doesn't exit."
        exit 1;
    fi

    cd ${pidfilepath}
    for item in ${program[@]}
    do
        pidfile=${pidfileprefix}$item${pidfileendfix}
        if [ ! -f ${pidfile} ];then
            continue
        fi
        pid=$(cat ${pidfile})
        ps -ef | grep ${exefile} | grep -v "grep" | grep -q ${pid}
        if [ $? -eq 0 ];then
            echo "robot for match $item is running."
        fi
    done
    cd -
}

if [ $# -lt 1 ];then
    echo "usage:./s.sh start match1 or ./s.sh stop match1 or ./s.sh status."
    exit 1
fi

command=$1
match=$2

case $command in
    "start")
        echo "starting robot program..."
        start $match
    ;;
    "stop")
        echo "stopping robot program..."
        stop $match
    ;;
    "status")
        echo "search robot status on running..."
        status
    ;;
    *)
        echo "param error."
    ;;
esac
exit 0;
