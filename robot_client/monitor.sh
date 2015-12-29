#!/bin/bash
pidFileFolder=/home/zhangsb/bin/test/pid
(
echo "=========== monitor start ============"
echo "time: $(date +"%Y-%m-%d %k:%M:%S")"

if [ ! -d ${pidFileFolder} ];then
    echo "Doesn't exist ${pidFileFolder}"
    exit 1
fi

cd ${pidFileFolder}
pidfiles=$(ls)
echo "pid file list:" 
echo ${pidfiles}

echo "status:"
for item in ${pidfiles}
do
    ps -ef | grep -v "grep" | grep -wq $(cat ${item})
    if [ $? -eq 1 ];then
        echo "${item} abnormal."
        spacemsg=$(echo ${item} | awk -F '.' '{print $2}')
        echo ${spacemsg}
        cd ../
        ./s.sh stop ${spacemsg}
        echo "Delete pid files."
        ./s.sh start ${spacemsg}
        echo "start result: $?"
        cd - > /dev/null
    else
        echo "${item} status normal"
    fi
done
cd - > /dev/null
echo "============= monitor end ============="
) >> ${pidFileFolder}/../robot_moniter.log
