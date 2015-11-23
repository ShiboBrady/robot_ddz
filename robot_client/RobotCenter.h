#ifndef __ROBOTCENTER_H__
#define __ROBOTCENTER_H__

#include <map>
#include <string>
#include "OGLordRobotAI.h"

class RobotCenter
{
public:
    RobotCenter();
    ~RobotCenter();

    std::string RobotProcess(std::string msg);
private:
    std::map<std::string, OGLordRobotAI> mMapRobotList_;
};

#endif /*__ROBOTCENTER_H__*/