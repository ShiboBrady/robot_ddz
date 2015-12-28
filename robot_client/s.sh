#!/bin/bash

#program=(game1 game2 game3 game4 three six_dali six_taotai ass newer free_timer)
program=(game1 game2 game3 game4 six_taotai newer free_timer)

function startprogram()
{
    pidfilepath="pid"
    pidfile="robot."$1".pid"
    binfile="bin"
    exefile="robot_client"
    #libpath="/usr/local/lib"
    libpath="/home/zhangsb/usr/lib"
    #libpath="/home/zhangshibo/usr/lib"
    logfilepath="log/"$1

    mkdir -p $logfilepath
    mkdir -p $pidfilepath
    cd $pidfilepath
    if [ -f $pidfile ];then
        echo "Failed! exist pid file."
        cd - > /dev/null
        return 1
    fi
    cd - > /dev/null

    if [ ! -d ${binfile} ];then
        echo "Failed! ${binfile} not exist."
        exit 2
    fi

    cd $binfile
    if [ ! -f ${exefile} ];then
        echo "Failed! ${exefile} not exist."
        exit 2
    fi
    cd - > /dev/null

    echo $LD_LIBRARY_PATH | grep -q "$libpath"
    if [[ $? -eq 1 ]];then
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${libpath}
    fi
    
    ulimit -c unlimited
    if [[ $? -eq 1 ]];then
        echo "Failed! set core space failed."
        exit 3
    fi
    
    cd ${binfile}
    nohup ./${exefile} $1 >/dev/null 2>&1 &
    pid=$!
    cd - > /dev/null
    ps -ef | grep ${exefile} | grep -q ${pid}
    if [[ $? -eq 0 ]];then
        echo ${pid} > ${pidfilepath}/${pidfile}
        if [[ $? -eq 1 ]];then
            echo "Failed! write pid to file ${pidfile} failed."
            return 4
        fi
    else
        echo "Failed! robot with param $1 failed."
        return 5
    fi
    return 0
}

function stopprogram()
{
    pidfilepath="pid"
    pidfile="robot."$1".pid"
    killcommand="-2"
    
    if [ ! -d $pidfilepath ];then
        exit 6
    fi  
    
    cd ${pidfilepath}
    if [ ! -f $pidfile ];then
        cd - > /dev/null
        return 7
    fi

    if [ ! -s $pidfile ];then
        cd - > /dev/null
        return 8
    fi

    PID=$(cat ${pidfile})

    kill ${killcommand} $PID

    if [[ $? -eq 1 ]];then
        rm -rf ${pidfile}
        return 9
    fi
    rm -rf ${pidfile}
    cd - > /dev/null
    return 0
}

function start()
{
    if [ "$1"x = ""x ];then
        exit 10
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
            return $?
        else
            echo "useless second param, please check."
            exit 11
        fi
    fi
    return 0
}

function stop()
{
    if [ "$1"x = ""x ];then
        exit 10
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
            return $?
        else
            exit 11
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
        exit 6
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
            echo "$item"
        fi
    done
    cd - > /dev/null
    return 0
}

if [ $# -lt 1 ];then
    exit 12
fi

command=$1
match=$2

case $command in
    "start")
        start $match
        exit $?
    ;;
    "stop")
        stop $match
        exit $?
    ;;
    "status")
        status
        exit $?
    ;;
    *)
        echo "param error."
        exit 12
    ;;
esac
