#ifndef _ROBOT_H_
#define _ROBOT_H_

#include "AbstractProduct.h"
#include "OGLordRobotAI.h"
class Robot
{
public:
    Robot( const int robotId, const int IQLevel )
        :robot_(robotId, IQLevel),
         needKeepPlay_(false),
         status_(robot::INIT),
         factory_(){};
    ~Robot(){};
    bool RobotProcess( int msgId, const std::string& msg, std::string& result );
    OGLordRobotAI& GetRobot() { return robot_; };

    void SetStatus( RobotStatus status ) { status_ = status; };
    robot::RobotStatus GetStatus() { return status_; };

    void SetCost( int costId ) { costId_ = costId; };
    bool GetCost() { return costId_; };

    void SetNeedKeepPlay( bool needKeepPlay ) { needKeepPlay_ = needKeepPlay; };
    bool GetNeedKeepPlay() { return needKeepPlay_; };
private:
    OGLordRobotAI robot_;
    robot::RobotStatus status_;
    int costId_;    //报名比赛时的费用ID
    bool needKeepPlay_;    //是否需要短线续玩
    SimpleFactory factory_;
    AbstractProduct* product_;
};

#endif /*_ROBOT_H_*/