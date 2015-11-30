#include <iostream>
#include <string>
#include <signal.h>
#include "NetLib.h"
#include "confaccess.h"
#include "log.h"

using namespace std;

NetLib *netLib = NULL;

void SignalFunc(int arg)
{
    DEBUG("Receved SIGINI signal, program will exit...");
    if (NULL != netLib)
    {
        netLib->stop();
    }
}

void InitSignalHandle()
{
    struct sigaction sa_usr;
    sa_usr.sa_flags = 0;
    sa_usr.sa_handler = SignalFunc;   //信号处理函数
    sigaction(SIGINT, &sa_usr, NULL);
    sigaction(SIGKILL, &sa_usr, NULL);
}

void InitConfig(const char *confFile)
{
    if (NULL == confFile)
    {
        cout << "Doesn't has configure file." << endl;
        ::exit(0);
    }
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->Load(confFile);
    string strLogFile;
    confAccess->GetValue("log", "logConf", strLogFile, "../configure/log4cplus.properties");
    log::CLog::Initialize(strLogFile);
}

int main(int argc, char** argv)
{
    InitSignalHandle();
    const char confFile[] = "../configure/robot.conf";
    InitConfig(confFile);
    netLib = new NetLib;
    netLib->start();
    return 0;
}

