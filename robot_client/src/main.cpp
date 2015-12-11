#include <iostream>
#include <string>
#include "NetLib.h"
#include "confaccess.h"
#include "log.h"

using namespace std;

void InitConfig(const char *confFile, const string& strProgramName)
{
    if (NULL == confFile)
    {
        cout << "Doesn't has configure file." << endl;
        ::exit(0);
    }
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    if (!confAccess->Load(confFile, strProgramName))
    {
        cout << "Load configure file failed." << endl;
        ::exit(0);
    }
    string strLogFile = confAccess->GetLogConfFilePath();
    log::CLog::Initialize(strLogFile);
}

int main(int argc, char** argv)
{
    if (2 != argc)
    {
        cout << "usage: ./robot_client param." << endl;
        return 0;
    }
    string strCmdParam = string(argv[1]);
    cout << "Starting program " << strCmdParam << endl;

    const char confFile[] = "../configure/robot.conf";
    InitConfig(confFile, strCmdParam);
    NetLib netLib;
    netLib.start();
    return 0;
}

