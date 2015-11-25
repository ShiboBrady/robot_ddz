#include <iostream>
#include <string>
#include "NetLib.h"
#include "confaccess.h"

using namespace std;

void InitConfig(const char *confFile)
{
    if (NULL == confFile)
    {
        cout << "Doesn't has configure file." << endl;
        ::exit(0);
    }
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->Load(confFile);
}

int main(int argc, char** argv)
{
    const char confFile[] = "./robot.conf";
    InitConfig(confFile);
    NetLib netLib;
    netLib.start();
    return 0;
}

