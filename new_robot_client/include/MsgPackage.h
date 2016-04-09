#ifndef __MSGPACKAGE_H__
#define __MSGPACKAGE_H__

#include "Connection.h"

class MsgNode;
class Conn;
typedef std::shared_ptr<MsgNode> MessagePackagePtr;
typedef std::shared_ptr<Conn> ConnctionPtr;
class MsgNode
{
public:
    MsgNode()
        :timerEvent_({0, 0}),
         objectPoint_(NULL),
         msgId_(0),
         bIsNeedSend_(false),
         iDelaySecond_(0),
         iNeedRobotNum_(0),
         bIsGameOver_(false),
         bIsNeedKeepPlay_(true),
         bIsNeedDelRobot_(false) {}
    struct event* GetEventP() { return &event_; }

    void SetTimer(int sec, int usec) { timerEvent_.tv_sec = sec; timerEvent_.tv_usec = usec; }
    struct timeval* GetTimerP() { return &timerEvent_; }

    void SetObjectPoint(void* objectPoint) { objectPoint_ = objectPoint; }
    void* GetObjectPoint() { return objectPoint_; }

    void SetMsg(const std::string& strMsg) { msg_ = strMsg; }
    std::string GetMsg() { return msg_; }

    void SetMsgId(int iMsgId) { msgId_ = iMsgId; }
    int GetMsgId() { return msgId_; }

    void SetConn(const ConnctionPtr& conn) { conn_ = conn; }
    const ConnctionPtr& GetConn() { return conn_; }

    void SetNeedRobotNum(int robotNum) { iNeedRobotNum_ = robotNum; }
    int GetNeedRobotNum() { return iNeedRobotNum_; }

    void SetMsgNeedSend(bool bNeedSend) { bIsNeedSend_ = bNeedSend; }
    bool GetMsgNeedSend() { return bIsNeedSend_; }

    void SetMsgDelaySecond(int iDelay) { iDelaySecond_ = iDelay; }
    int GetMsgDelaySecond() { return iDelaySecond_; }

    void SetGameIsOver(bool bIsGameOver) { bIsGameOver_ = bIsGameOver; }
    bool GetGameIsOver() { return bIsGameOver_; }

    void SetNeedKeepPlay(bool bIsNeedKeepPlay) { bIsNeedKeepPlay_ = bIsNeedKeepPlay; }
    bool GetIsNeedKeepPlay() { return bIsNeedKeepPlay_; }

    void SetNeedDelRobot(bool bIsNeedDelRobot) { bIsNeedDelRobot_ = bIsNeedDelRobot; }
    bool GetIsNeedDelRobot() { return bIsNeedDelRobot_; }

private:
    struct event event_;
    struct timeval timerEvent_;
    void* objectPoint_;
    std::string msg_;
    int msgId_;
    ConnctionPtr conn_;
    bool bIsNeedSend_;
    int iDelaySecond_;
    int iNeedRobotNum_;
    bool bIsGameOver_;
    bool bIsNeedKeepPlay_;
    bool bIsNeedDelRobot_;
};


#endif /*__MSGPACKAGE_H__*/