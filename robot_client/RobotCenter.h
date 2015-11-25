#ifndef __ROBOTCENTER_H__
#define __ROBOTCENTER_H__

#include <map>
#include <string>
#include "OGLordRobotAI.h"

class RobotCenter
{
public:
    RobotCenter( const int robotId, const int robotIQLevel );
    ~RobotCenter();

    std::string RobotProcess(std::string msg);
private:
    OGLordRobotAI _Robot;
};

#endif /*__ROBOTCENTER_H__*/