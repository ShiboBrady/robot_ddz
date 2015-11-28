#include <iostream>
#include <string>
#include "NetLib.h"
#include "confaccess.h"
#include "log.h"

using namespace std;

void InitConfig(const char *confFile)
{
    if (NULL == confFile)
    {
        ERROR("Doesn't has configure file.");
        ::exit(0);
    }
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->Load(confFile);
}

int main(int argc, char** argv)
{
    const char confFile[] = "./robot.conf";
    log::CLog::Initialize("");
    InitConfig(confFile);
    NetLib netLib;
    netLib.start();
    return 0;
}

