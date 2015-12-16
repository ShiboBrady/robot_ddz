#ifndef _ROBOT_H_
#define _ROBOT_H_

#include "AbstractProduct.h"
#include "OGLordRobotAI.h"
class Robot
{
public:
    Robot( const int robotId, const int IQLevel )
        :robot_(),
         robotId_(robotId),
         needKeepPlay_(false),
         status_(robot::INIT),
         factory_()
         { robot_.RbtInSetLevel(IQLevel); }
    ~Robot(){}
    virtual bool RobotProcess( int msgId, const std::string& msg, std::string& result );
    virtual OGLordRobotAI& GetRobot() { return robot_; }

    virtual void SetRobotId( int robotId ) { robotId_ = robotId; }
    virtual int GetRobotId() { return robotId_; }

    virtual void SetStatus( RobotStatus status ) { status_ = status; }
    virtual robot::RobotStatus GetStatus() { return status_; }

    virtual void SetNeedKeepPlay( bool needKeepPlay ) { needKeepPlay_ = needKeepPlay; }
    virtual bool GetNeedKeepPlay() { return needKeepPlay_; }
private:
    OGLordRobotAI robot_;
    int robotId_;
    robot::RobotStatus status_;
    bool needKeepPlay_;    //是否需要断线续玩
    SimpleFactory factory_;
    AbstractProduct* product_;
};

#endif /*_ROBOT_H_*/